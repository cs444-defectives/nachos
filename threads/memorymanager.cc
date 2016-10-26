#ifdef CHANGED
#include "memorymanager.h"

MemoryManager::MemoryManager() {
    bitmap = new BitMap(NumPhysPages);
}

int MemoryManager::GetPage() {
    int addr = bitmap->Find();
    return addr;
}
#endif
