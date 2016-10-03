/*
 * test_elevator.cc
 *
 * Demonstrates the correctness of our implementation of the elevator problem.
 */

#include "system.h"
#include "synch.h"

#define NUM_FLOORS 4
#define NUM_PASSENGERS 10

/**
 * this variable maintains a count of passengers yet to ride the elevator
 * in order to know when to terminate the program
 */
int passengers_remaining = NUM_PASSENGERS;
Lock *passengers_remaining_lock = new Lock("waiting passengers count lock");

static Lock *arrived_lock[NUM_FLOORS];
static Condition *arrived[NUM_FLOORS];
static int floor;

static Lock *ready_lock[NUM_FLOORS];
static Condition *ready[NUM_FLOORS];
static int queued[NUM_FLOORS];

/* for dynamically generating names */
static char *arrived_lock_names[NUM_FLOORS], *arrived_cond_names[NUM_FLOORS],
            *ready_lock_names[NUM_FLOORS], *ready_cond_names[NUM_FLOORS],
            *thread_names[NUM_PASSENGERS];

/*
 * passes time by toggling interrupts, potentially a dangerous thing! RELEASE
 * LOCKS BEFORE CALLING!
 */
static void tick(int n)
{
    ASSERT((IntOn == interrupt->SetLevel(IntOff)));
    for (int i = 0; i < n; i++) {
        interrupt->SetLevel(IntOff);
        interrupt->SetLevel(IntOn);
    }

    /* in case no random seed is given */
    currentThread->Yield();
}

static void elevate(int _)
{
    floor = 0;
    int direction = 0;

    /**
     * we don't need to protect access to this variable using its lock because
     * when it becomes 0 all other threads will have terminated and any other time
     * the elevator will just continue
    */
    while (passengers_remaining) {

        printf("Elevator at floor %d\n", floor);

        /* tell passengers they may leave or enter */
        arrived_lock[floor]->Acquire();
        arrived[floor]->Broadcast(arrived_lock[floor]);
        arrived_lock[floor]->Release();

        /* wait until everyone's in/out of the elevator */
        ready_lock[floor]->Acquire();
        while (queued[floor] != 0)
            ready[floor]->Wait(ready_lock[floor]);
        ready_lock[floor]->Release();

        /* move to the next floor */
        if (floor == 0 || floor == NUM_FLOORS - 1)
            direction = !direction;
        floor += direction ? 1 : -1;
        tick(100);
        currentThread->Yield();
    }
}

static void waitForFloor(int n)
{
    ready_lock[n]->Acquire();
    queued[n]++;
    ready_lock[n]->Release();

    arrived_lock[n]->Acquire();
    while (floor != n)
        arrived[n]->Wait(arrived_lock[n]);
    arrived_lock[n]->Release();

    ready_lock[n]->Acquire();
    queued[n]--;
    ready[n]->Signal(ready_lock[n]);
    ready_lock[n]->Release();
}

static void getOnElevator(void)
{
    printf("%s gets on elevator at floor %d\n",
           currentThread->getName(), floor);
}

static void getOffElevator(void)
{
    printf("%s gets off elevator at floor %d\n",
           currentThread->getName(), floor);
}

void ArrivingGoingFromTo(int atFloor, int toFloor)
{
    printf("%s waits at floor %d to go to floor %d\n",
           currentThread->getName(), atFloor, toFloor);

    waitForFloor(atFloor);
    getOnElevator();

    waitForFloor(toFloor);
    getOffElevator();
}

static void passenge(int _)
{
    int from, to;
    from = rand() % NUM_FLOORS;

    /* make sure from and to aren't the same floor */
    do
        to = rand() % NUM_FLOORS;
    while (to == from);

    ArrivingGoingFromTo(from, to);

    passengers_remaining_lock->Acquire();
    passengers_remaining--;
    passengers_remaining_lock->Release();
}

void TestElevator(void)
{
    ASSERT((NUM_FLOORS != 0));

    char *m;

    for (int i = 0; i < NUM_FLOORS; i++) {
        m = arrived_cond_names[i];
        m = new char[32];
        snprintf(m, 32, "floor %d arrived", i);
        arrived[i] = new Condition(m);

        m = arrived_lock_names[i];
        m = new char[32];
        snprintf(m, 32, "floor %d arrived_lock", i);
        arrived_lock[i] = new Lock(m);

        m = ready_cond_names[i];
        m = new char[32];
        snprintf(m, 32, "floor %d ready", i);
        ready[i] = new Condition(m);

        m = ready_lock_names[i];
        m = new char[32];
        snprintf(m, 32, "floor %d ready_lock", i);
        ready_lock[i] = new Lock(m);
    }

    Thread *elevator = new Thread("elevator");
    elevator->Fork(elevate, 0);

    Thread *passengers[NUM_PASSENGERS];
    for (int i = 0; i < NUM_PASSENGERS; i++) {
        m = thread_names[i];
        m = new char[32];
        snprintf(m, 32, "passenger %d", i);
        passengers[i] = new Thread(m);
        passengers[i]->Fork(passenge, 0);
    }
}
