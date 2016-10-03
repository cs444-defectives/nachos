#ifdef CHANGED

#ifndef TEST_VDOT_H
#define TEST_VDOT_H

#include "synch.h"

class BridgeMonitor
{
    const static int MAX_CARS = 3;
    int cars;
    int direction;
    Lock* lock;
    Condition* depart;

public:
    BridgeMonitor();
    void Arrive(int dir);
    void Depart();
    void CrossBridge(int dir);
};

#endif /* ifndef TEST_VDOT_H */
#endif /* ifdef CHANGED */
