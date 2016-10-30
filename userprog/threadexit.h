#ifdef CHANGED
#include "syscall.h"
#include "synch.h"

typedef struct {
    Semaphore *join;
    Lock *joinLock;
    int exitCode;
    SpaceId spaceId;
    bool done;
} ThreadExit;
#endif
