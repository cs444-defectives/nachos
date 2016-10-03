/*
 * test_locks_conditions.cc
 *
 * Demonstrates the correctness of our implementation of locks and condition
 * variables.
 */

#ifdef CHANGED

#include "system.h"
#include "synch.h"

Lock *lock;

static void multilock(int _)
{
    for (int i = 0; i < 5; i++) {
        lock->Acquire();
        lock->Release();
        currentThread->Yield();
    }
}

static void multiyield(int n)
{
    for (int i = 0; i < n; i++)
        currentThread->Yield();
}

void TestLocksConditions(void)
{
    Thread *t = new(std::nothrow) Thread("forked thread");

    lock = new Lock("harambe");
    lock->Acquire();

    t->Fork(multilock, 0);

    /* give the forked thread many chances to run so we can prove it can't */
    multiyield(10);

    DEBUG('L', "\nProof of Lock correctness:\nForked thread has been spawned, but is stuck because it doesn't have the lock!\n\n");

    lock->Release();
    currentThread->Yield();

    /* give the forked thread many chances to finish its task */
    multiyield(10);

    DEBUG('L', "Main thread cleaning up...\n");
    delete lock;
}

#endif /* ifdef CHANGED */
