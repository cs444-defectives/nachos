#pragma once

#include "machine.h"
#include "bitmap.h"
#include "synchdisk.h"

struct Process {
    SpaceId spaceId;
    Process* next;
    int user_page;
};

struct DiskPageDescriptor {
  Process *processes; // a linked list of processes using this sector
  int ram_page;
  int user_page;
  int refCount;
};

class MemoryManager {
private:
    BitMap *ramHeld;
    int last_evicted;
    Lock *evictionLock;

    void ram_page_to_disk(int ram_phys_page, int sector);
    void disk_page_to_ram(int sector, int ram_phys_page);
    int allocateRAMPage();
    void deallocateRAMPage(int ppn);
    void evict(void);
    void printDisk();

public:
    Lock *diskPagesLock;
    MemoryManager();
    int AllocateDiskPage(int user_page);
    void DeallocateDiskPage(int sector);
    void Fault(int userland_va);
    void Decouple(int virtualPage);
    int NumSectorsAvailable();
    BitMap *ramBitmap;
    BitMap *diskBitmap;

    /* translates between disk pages and RAM pages (sectors) */
    DiskPageDescriptor diskPages[NumSectors];
};
