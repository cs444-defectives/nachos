#include "system.h"
#include "synchdisk.h"
#include "memorymanager.h"
#include "translate.h"

MemoryManager::MemoryManager() {
    ramBitmap = new BitMap(NumPhysPages);
    ramHeld = new BitMap(NumPhysPages);
    diskBitmap = new BitMap(NumSectors);
    last_evicted = 0;
}

int MemoryManager::allocateRAMPage() {
    ASSERT(ramBitmap->NumClear() > 0);
    return ramBitmap->Find();
}

void MemoryManager::deallocateRAMPage(int ppn) {
    ramBitmap->Clear(ppn);
}

int MemoryManager::AllocateDiskPage(int user_page) {
    ASSERT(diskBitmap->NumClear() > 0);
    int sector = diskBitmap->Find();
    diskPages[sector].process = currentThread->spaceId;
    diskPages[sector].user_page = user_page;
    diskPages[sector].ram_page = -1;
    return sector;
}

void MemoryManager::DeallocateDiskPage(int sector) {
    ASSERT(!ramHeld->Test(sector));
    DiskPageDescriptor *p = diskPages + sector;

    /* delete the RAM page if it exists */
    if (p->ram_page > 0) {
        deallocateRAMPage(p->ram_page);
        p->ram_page = -1;
    }

    diskBitmap->Clear(sector);
}

/*
 * Evict a page in RAM.
 * TODO: the algorithm is bad as af
 */
void MemoryManager::evict(void) {
    ASSERT(!diskBitmap->NumClear());

    /*
     * starting at the last sector we evicted, find the next sector that's in
     * RAM but isn't stuck there
     */
    DiskPageDescriptor *p;
    while ((last_evicted = (last_evicted + 1) % NumSectors)) {
        p = diskPages + last_evicted;
        if (p->ram_page > 0 && !ramHeld->Test(last_evicted))
            break;
    }

    /* invalidate the RAM page in the user's page table */
    TranslationEntry *pte = threads[p->process]->space->pageTable + p->user_page;
    pte->valid = false;

    /* write the page to disk if it's been modified */
    if (pte->dirty)
        ram_page_to_disk(p->ram_page, last_evicted);

    /* destroy the page in RAM */
    deallocateRAMPage(p->ram_page);
    p->ram_page = -1;
}

void MemoryManager::Fault(int user_page) {

    /* do we need to evict a page first? */
    while (!ramBitmap->NumClear())
        evict();

    int page = allocateRAMPage();

    /* bookkeeping for eviction and deallocation */
    int sector = currentThread->space->sectorTable[user_page];
    diskPages[sector].process = currentThread->spaceId;
    diskPages[sector].ram_page = page;

    /* copy page from disk to RAM */
    disk_page_to_ram(sector, page);

    /* tell user program where the new page is */
    TranslationEntry *pageTable = currentThread->space->pageTable;
    pageTable[user_page].physicalPage = page;
    pageTable[user_page].valid = true;
}

/*
 * Given a physical page number in RAM and a disk sector that has a memory page
 * we want, copy the page into that page in RAM.
 */
void MemoryManager::disk_page_to_ram(int sector, int ram_phys_page) {
    DEBUG('a', "Copying disk sector <%d> to RAM page <%d>\n", sector, ram_phys_page);
    synchDisk->ReadSector(sector, machine->mainMemory + ram_phys_page * PageSize);
}

/*
 * Given a physical page number in RAM that has a memory page and a disk sector
 * we need to flush to, copy the RAM page into the disk sector.
 */
void MemoryManager::ram_page_to_disk(int ram_phys_page, int sector) {
    DEBUG('a', "Copying RAM page <%d> to disk sector <%d>\n", ram_phys_page, sector);
    synchDisk->WriteSector(sector, machine->mainMemory + ram_phys_page * PageSize);
}
