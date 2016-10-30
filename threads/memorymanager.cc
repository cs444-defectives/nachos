#ifdef CHANGED
#include "memorymanager.h"

MemoryManager::MemoryManager() {
    bitmap = new BitMap(NumPhysPages);
}

int MemoryManager::AllocatePage() {
    int addr = bitmap->Find();
    return addr;
}

void MemoryManager::DeallocatePage(int ppn) {
    bitmap->Clear(ppn);
}
#endif
