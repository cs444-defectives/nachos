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
    // create new threads that act as a single car each
    int j, k, dir, count = 0;

    int directions[] = {6, 5, 4, 3, 2, 1};

    messages = new char*[ARRAY_SIZE(directions)];
    char *buffer;

    for (j = 0; j < 6; j++) { // alternate direction
        dir = j % 2;
        for (k = 0; k < directions[j]; k++) { // how many go in given direction
            count++;
            buffer = new char[64];
            messages[k] = buffer;
            snprintf(buffer, 64, "car thread - dir: %d, num: %d", dir, k);
            Thread* car = new Thread(buffer);
            car->Fork(OneVehicle, dir);
        }
    }
}
