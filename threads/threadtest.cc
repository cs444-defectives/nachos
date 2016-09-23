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

void
SimpleThread(int _)
{
    for (int i = 0; i < 5; i++) {
        lock->Acquire();
        printf("- Forked thread has the lock\n");
        lock->Release();
        currentThread->Yield();
    }
}

void
ThreadTest()
{
    Thread *t = new(std::nothrow) Thread("forked thread");

    lock = new Lock("harambe");
    printf("- Main is taking the lock\n");
    lock->Acquire();

    t->Fork(SimpleThread, 0);
    currentThread->Yield();
    printf("- Forked thread has been spawned, but is stuck because it doesn't have the lock!\n");

    printf("- Main is releasing the lock\n");
    lock->Release();
    currentThread->Yield();

    for (int i = 0; i < 20; i++) {
        printf("- Main thread ticks\n");
        currentThread->Yield();
    }

    printf("- Main thread cleaning up...\n");
    delete lock;
}
