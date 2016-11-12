#include "machine.h"
#include "bitmap.h"

class MemoryManager {
private:
    BitMap *ramBitmap;
    BitMap *ramCoupled;
    BitMap *diskBitmap;
    int last_evicted;
    void ram_page_to_disk(int ram_phys_page, int sector);
    void disk_page_to_ram(int sector, int ram_phys_page);
public:
    MemoryManager();
    int AllocateRAMPage();
    void DeallocateRAMPage(int ppn);
    int AllocateDiskPage();
    void DeallocateDiskPage(int ppn);
    void Fault(int userland_va);
};
