// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"

Lock *lock;

void SimpleThread(int _)
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

void ThreadTest()
{
    Thread *t = new(std::nothrow) Thread("forked thread");

    lock = new Lock("harambe");
    lock->Acquire();

    t->Fork(SimpleThread, 0);

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
