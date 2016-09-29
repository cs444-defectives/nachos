/*
 * test_vdot.cc
 *
 * Demonstrates the correctness of our implementation of the bridge problem.
 */

#include "system.h"
#include "test_vdot.h"

BridgeMonitor::BridgeMonitor()
{
    cars = 0;
    direction = 0;
    depart = new Condition("depart");
    lock = new Lock("lock");
}

void BridgeMonitor::Arrive(int direction_desired)
{
    bool wrong_way, full;

    while ((wrong_way = (direction_desired != direction)) ||
           (full = (cars == MAX_CARS))) {

        if (full)
            depart->Wait(lock);
        else if (wrong_way && !cars)
            direction = direction_desired; 
        else if (wrong_way)
            depart->Wait(lock);
    }
}

void BridgeMonitor::Depart(int _)
{
    lock->Acquire();
    cars--;
    depart->Broadcast(lock);
    lock->Release();
}

void TestVdot(void)
{
}
