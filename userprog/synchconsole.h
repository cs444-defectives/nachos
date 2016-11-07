#ifdef CHANGED
#pragma once

#include "synch.h"
#include "console.h"

/*
 * Routines to synchronously access the console. The console is an asynchronous
 * device (requests return immediately, and an interrupt happens later on).
 * This is a layer on top of the console providing a synchronous interface
 * (requests wait until the request completes).
 *
 * We use a semaphore to synchronize the interrupt handlers with the pending
 * requests. And, because the console can only handle one operation at a time,
 * use a lock to enforce mutual exclusion.
 */

class SynchConsole {
    public:
    SynchConsole(void);
    ~SynchConsole(void);

    /*
     * read/write from/to the console, returning only once the data is actually
     * read or written
     */
    char ReadChar(void);
    void ReadBytes(char *dest, int n);
    void WriteChar(char c);
    void WriteBytes(char *s, int n);

    /*
     * called (albeit indirectly) by the underlying console object when a key
     * is pressed
     */
    void ReadAvailable();

    /*
     * called (albeit indirectly) by the underlying console object on character
     * write to console
     */
    void WriteDone();

    private:
    Console *console;
    Semaphore *reads, *writes;
    Lock *lock;
};
#endif /* CHANGED */
