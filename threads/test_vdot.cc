/*
 * test_vdot.cc
 *
 * Demonstrates the correctness of our implementation of the elevator problem.
 */

#include "system.h"
#include "synch.h"

class BridgeMonitor
{
    static const int max_cars = 3;
    static int cars = 0;
    static int flow = 0;
    static Lock lock;
    static Condition depart;

public:
    void Arrive(int dir);
    void Depart(int dir);
}

void BridgeMonitor::Arrive(int dir)
{
    bool wrong_way, full;
    while ((wrong_way = (dir != flow)) ||
           (full = (cars == max_cars))) {

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
    lock->Release(lock);
}

void TestVdot(void)
{
}
