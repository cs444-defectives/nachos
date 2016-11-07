#ifdef CHANGED
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
    DEBUG('L', "%s P() on semaphore %s\n", currentThread->getName(), name);
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
    DEBUG('L', "%s V() on semaphore %s\n", currentThread->getName(), name);
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
    status = FREE;
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

    while (status == BUSY) {
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

Condition::Condition(const char* debugName)
{
    name = debugName;
    threads = new List();
}

Condition::~Condition()
{
    delete threads;
}

/*
 * Wait on a condition. We wake up when someone hints that the condition MAY be
 * true by calling Condition::Signal().
 *
 * Caller must follow the following pattern:
 *
 *     bool done;
 *     Lock done_lock;
 *     Condition done_cond;
 *     ...
 *
 *     done_lock->Acquire();
 *     while (!done)
 *         done_cond->Wait(done_lock);
 *     done_lock->Release();
 *
 * We assume that the given lock is in state BUSY when Condition::Wait() is
 * called. We must then: release the lock, move to the wait queue, and sleep the
 * thread.
 *
 * When awoken, we must reacquire the lock so that it returns to state BUSY as
 * it was before the call.
 */
void Condition::Wait(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' is waiting on Condition '%s'\n",
          currentThread->getName(), getName());

    conditionLock->Release();
    threads->Append(currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Indicates to a single process waiting on a condition that the condition is
 * ready to be re-checked. To wake *all* waiting processes instead, use
 * Condition::Broadcast().
 *
 * Caller must follow the following pattern:
 *
 *     bool done;
 *     Lock done_lock;
 *     Condition done_cond;
 *     ...
 *
 *     done_lock->Acquire();
 *     done = true;
 *     done_cond->Signal(done_lock);
 *     done_lock->Release();
 *
 * The lock does not *necessarily* need to be acquired to run
 * Condition::Signal(), for example if there is only one possible signaling
 * process, but please do so anyway, okay? It's good practice and will prevent
 * subtle, hard-to-catch bugs if additional signalers are added in the future.
 */
void Condition::Signal(Lock* _)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' signaled Condition '%s'\n",
          currentThread->getName(), getName());

    /* ReadyToRun assumes that interrupts are already disabled */
    if (!threads->IsEmpty())
        scheduler->ReadyToRun((Thread *) threads->Remove());

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Indicates to a all processes waiting on a condition that the condition is
 * ready to be re-checked. To wake a *single* waiting processes instead, use
 * Condition::Signal(). Subject to the same caveats and calling pattern as
 * Condition::Broadcast(), see comment there for details.
 */
void Condition::Broadcast(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' broadcasted Condition '%s'\n",
          currentThread->getName(), getName());

    Thread *t;
    while (!threads->IsEmpty()) {
        t = (Thread *) threads->Remove();

        /* ReadyToRun assumes that interrupts are already disabled */
        scheduler->ReadyToRun(t);
    }

    (void) interrupt->SetLevel(oldLevel);
}
#else /* CHANGED */
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
    status = FREE;
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

    while (status == BUSY) {
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

Condition::Condition(const char* debugName)
{
    name = debugName;
    threads = new List();
}

Condition::~Condition()
{
    delete threads;
}

/*
 * Wait on a condition. We wake up when someone hints that the condition MAY be
 * true by calling Condition::Signal().
 *
 * Caller must follow the following pattern:
 *
 *     bool done;
 *     Lock done_lock;
 *     Condition done_cond;
 *     ...
 *
 *     done_lock->Acquire();
 *     while (!done)
 *         done_cond->Wait(done_lock);
 *     done_lock->Release();
 *
 * We assume that the given lock is in state BUSY when Condition::Wait() is
 * called. We must then: release the lock, move to the wait queue, and sleep the
 * thread.
 *
 * When awoken, we must reacquire the lock so that it returns to state BUSY as
 * it was before the call.
 */
void Condition::Wait(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' is waiting on Condition '%s'\n",
          currentThread->getName(), getName());

    conditionLock->Release();
    threads->Append(currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Indicates to a single process waiting on a condition that the condition is
 * ready to be re-checked. To wake *all* waiting processes instead, use
 * Condition::Broadcast().
 *
 * Caller must follow the following pattern:
 *
 *     bool done;
 *     Lock done_lock;
 *     Condition done_cond;
 *     ...
 *
 *     done_lock->Acquire();
 *     done = true;
 *     done_cond->Signal(done_lock);
 *     done_lock->Release();
 *
 * The lock does not *necessarily* need to be acquired to run
 * Condition::Signal(), for example if there is only one possible signaling
 * process, but please do so anyway, okay? It's good practice and will prevent
 * subtle, hard-to-catch bugs if additional signalers are added in the future.
 */
void Condition::Signal(Lock* _)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' signaled Condition '%s'\n",
          currentThread->getName(), getName());

    /* ReadyToRun assumes that interrupts are already disabled */
    if (!threads->IsEmpty())
        scheduler->ReadyToRun((Thread *) threads->Remove());

    (void) interrupt->SetLevel(oldLevel);
}

/*
 * Indicates to a all processes waiting on a condition that the condition is
 * ready to be re-checked. To wake a *single* waiting processes instead, use
 * Condition::Signal(). Subject to the same caveats and calling pattern as
 * Condition::Broadcast(), see comment there for details.
 */
void Condition::Broadcast(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    DEBUG('L', "Thread '%s' broadcasted Condition '%s'\n",
          currentThread->getName(), getName());

    Thread *t;
    while (!threads->IsEmpty()) {
        t = (Thread *) threads->Remove();

        /* ReadyToRun assumes that interrupts are already disabled */
        scheduler->ReadyToRun(t);
    }

    (void) interrupt->SetLevel(oldLevel);
}
#endif /* CHANGED */
