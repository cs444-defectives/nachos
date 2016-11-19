#pragma once

#include "machine.h"
#include "bitmap.h"
#include "synchdisk.h"

struct DiskPageDescriptor {
  int process;  /* process that "owns" (asked for) the page */
  int ram_page;
  int user_page;
};

class MemoryManager {
private:
    BitMap *ramBitmap;
    BitMap *ramHeld;
    BitMap *diskBitmap;
    int last_evicted;
    void ram_page_to_disk(int ram_phys_page, int sector);
    void disk_page_to_ram(int sector, int ram_phys_page);

    int allocateRAMPage();
    void deallocateRAMPage(int ppn);
    void evict(void);

    /* translates between disk pages and RAM pages (sectors) */
    DiskPageDescriptor diskPages[NumSectors];
public:
    MemoryManager();
    int AllocateDiskPage(int user_page);
    void DeallocateDiskPage(int sector);
    void Fault(int userland_va);
    void Decouple(int virtualPage);
    int NumSectorsAvailable();
};
