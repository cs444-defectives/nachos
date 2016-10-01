/*
 * test_vdot.cc
 *
 * Demonstrates the correctness of our implementation of the bridge problem.
 */

#include "system.h"
#include "test_vdot.h"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof((X)[0]))

static char **messages;

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

    printf("# cars: %d\n", cars);
    lock->Acquire();
    while ((wrong_way = (direction_desired != direction)) ||
           (full = (cars == MAX_CARS))) {
        if (full)
            depart->Wait(lock);
        else if (wrong_way && !cars)
            direction = direction_desired; 
        else if (wrong_way)
            depart->Wait(lock);
    }
    cars++;
    lock->Release();
}

void BridgeMonitor::Depart()
{
    lock->Acquire();
    cars--;
    depart->Broadcast(lock);
    lock->Release();
}

void BridgeMonitor::CrossBridge(int dir)
{
   printf("crossing bridge in %d direction\n", dir);
}

// TODO: revise this
BridgeMonitor* myBridgeMonitor = new BridgeMonitor();

void OneVehicle(int direc){
    int i;
    int arrivalDelay = 1 + (int)(100000.0 * rand() / (RAND_MAX + 1.0));
    int crossingTime = 1 + (int)(100000.0 * rand() / (RAND_MAX + 1.0));

    for(i = 0; i <arrivalDelay; i++){
        currentThread->Yield();
        interrupt->SetLevel(IntOff);
        interrupt->SetLevel(IntOn);
    }

    myBridgeMonitor->Arrive(direc);
    myBridgeMonitor->CrossBridge(direc);

    for(i = 0; i< crossingTime; i++){
        currentThread->Yield();
        interrupt->SetLevel(IntOff);
        interrupt->SetLevel(IntOn);
    }

    myBridgeMonitor->Depart();
}

void TestVdot(void)
{
    /* create new threads that act as a single car each */
    int cars_per_round[] = {6, 5, 4, 3, 2, 1};
    const int rounds = ARRAY_SIZE(cars_per_round);

    int total_cars = 0;
    for (int round = 0; round < rounds; round++)
        total_cars += cars_per_round[round];

    messages = new char*[total_cars];
    char *buffer;

    for (int round = 0; round < rounds; round++) {

        /* alternate direction */
        int dir = round % 2;

        /* fire off each car */
        for (int car = 0; car < cars_per_round[round]; car++) {
            buffer = new char[64];
            messages[car] = buffer;
            snprintf(buffer, 64, "car thread - dir: %d, num: %d", dir, car);
            Thread *car_thread = new Thread(buffer);
            car_thread->Fork(OneVehicle, dir);
        }
    }
}
