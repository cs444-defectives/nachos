/*
 * Routines for synchronizing threads. Three kinds of synchronization routines
 * are defined here: semaphores, locks and condition variables (the
 * implementation of the last two are left to the reader).
 *
 * Any implementation of a synchronization routine needs some primitive atomic
 * operation. We assume Nachos is running on a uniprocessor, and thus atomicity
 * can be provided by turning off interrupts. While interrupts are disabled, no
 * context switch can occur, and thus the current thread is guaranteed to hold
 * the CPU throughout, until interrupts are reenabled.
 *
 * Because some of these routines might be called with interrupts already
 * disabled (Semaphore::V for one), instead of turning on interrupts at the end
 * of the atomic operation, we always simply re-set the interrupt state back to
 * its original value (whether that be disabled or enabled).
 *
 * Copyright (c) 1992-1993 The Regents of the University of California. All
 * rights reserved. See copyright.h for copyright notice and limitation of
 * liability and disclaimer of warranty provisions.
 */

#include "copyright.h"
#include "synch.h"
#include "system.h"

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new(std::nothrow) List;
}

/*
 * De-allocate semaphore, when no longer needed. Assume no one is still waiting
 * on the semaphore!
 */
Semaphore::~Semaphore()
{
    delete queue;
}


/*
 * Wait until semaphore value > 0, then decrement.  Checking the
 * value and decrementing must be done atomically, so we
 * need to disable interrupts before checking the value.
 *
 * Note that Thread::Sleep assumes that interrupts are disabled
 * when it is called.
 */
void Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    /* go to sleep if semaphore isn't available */
    while (value == 0) {
        queue->Append((void *)currentThread);
        currentThread->Sleep();
    }

    /* semaphore is available, so consume its value */
    value--;

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Increment semaphore value, waking up a waiter if necessary. As with P(), this
 * operation must be atomic, so we need to disable interrupts.
 * Scheduler::ReadyToRun() assumes that threads are disabled when it is called.
 */
void Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();

    /* make thread ready, consuming the V immediately */
    if (thread != NULL)
        scheduler->ReadyToRun(thread);
    value++;

    (void) interrupt->SetLevel(oldLevel);
}

Lock::Lock(const char* debugName)
{
    name = debugName;
    status = LockStatus::FREE;
    threads = new List();
}

Lock::~Lock()
{
    delete threads;
}

/*
 * Gets a lock. If no lock available, the thread sleeps until it becomes
 * available.
 */
void Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    while (status == LockStatus::BUSY) {
        DEBUG('L', "Thread '%s' couldn't get Lock '%s'\n",
              currentThread->getName(), getName());
        threads->Append(currentThread);

        /* Thread::Sleep() assumes that interrupts are already disabled */
        currentThread->Sleep();
    }
    status = BUSY;
    DEBUG('L', "Thread '%s' got Lock '%s'\n",
          currentThread->getName(), getName());

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Releases a lock. This wakes up all threads sleeping on the lock, so they can
 * check again whether the lock is available.
 */
void Lock::Release()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    status = FREE;
    DEBUG('L', "Thread '%s' released Lock '%s'\n",
          currentThread->getName(), getName());

    Thread *t;
    while (!threads->IsEmpty()) {
        t = (Thread *) threads->Remove();

        /* ReadyToRun assumes that interrupts are already disabled */
        scheduler->ReadyToRun(t);
    }

    (void) interrupt->SetLevel(oldLevel);
}

Condition::Condition(const char* debugName) { }

Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock) { ASSERT(false); }

void Condition::Signal(Lock* conditionLock) { }

void Condition::Broadcast(Lock* conditionLock) { }
