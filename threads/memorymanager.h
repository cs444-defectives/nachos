#include "machine.h"
#include "bitmap.h"

class MemoryManager {
private:
    BitMap *bitmap;
public:
    MemoryManager();
    int AllocatePage();
    void DeallocatePage(int ppn);
};
