#ifdef CHANGED
#include "machine.h"
#include "bitmap.h"

class MemoryManager {
private:
    BitMap *bitmap;
public:
    MemoryManager();
    int GetPage();
};
#endif
