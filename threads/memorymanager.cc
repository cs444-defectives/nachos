#include "synchdisk.h"
#include "memorymanager.h"

MemoryManager::MemoryManager() {
    ramBitmap = new BitMap(NumPhysPages);
    ramCoupled = new BitMap(NumPhysPages);
    diskBitmap = new BitMap(NUM_SECTORS);
}

int MemoryManager::AllocateRAMPage() {
    ASSERT(ramBitmap->NumClear() > 0);
    int addr = ramBitmap->Find();
    return addr;
}

void MemoryManager::DeallocateRAMPage(int ppn) {
    ramBitmap->Clear(ppn);
}

int MemoryManager::AllocateDiskPage() {
    ASSERT(diskBitmap->NumClear() > 0);
    int addr = diskBitmap->Find();
    return addr;
}

void MemoryManager::DeallocateDiskPage(int ppn) {
    synchDisk->diskPages[addr] = NULL;
    diskBitmap->Clear(ppn);
}

/* The replacement algorithm. TODO: this is bad af */
int MemoryManager::Evict(void) {
    ASSERT(!diskBitmap->NumClear());

    while (last_evicted = (last_evicted + 1) % NumPhysPages)
        if (!ramCoupled->Test(last_evicted))
            return last_evicted;

    /* should not be reached */
    ASSERT(false);
}

void MemoryManager::Fault(int user_page) {
    int sector = currentThread->space->sectorTable[user_page];

    /* allocate or evict page */
    int page = ramBitmap->NumClear() ? AllocateRAMPage() : Evict();

    /* copy page to ram */
    disk_page_to_ram(sector, page);

    /* assign new physical ram page to user page table */
    currentThread->space->pageTable[user_page] = page;
}

/*
 * Given a physical page number in RAM and a disk sector that has a memory page
 * we want, copy the page into that page in RAM.
 */
void MemoryManager::disk_page_to_ram(int sector, int ram_phys_page) {
    synchDisk->ReadSector(sector, machine->mainMemory + ram_phys_page * PageSize);
}

/*
 * Given a physical page number in RAM that has a memory page and a disk sector
 * we need to flush to, copy the RAM page into the disk sector.
 */
void MemoryManager::ram_page_to_disk(int ram_phys_page, int sector) {
    synchDisk->WriteSector(sector, machine->mainMemory + ram_phys_page * PageSize);
}
