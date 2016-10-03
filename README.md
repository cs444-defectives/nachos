# nachos

An extension of the provided NACHOS code to be a ~real~ operating system.

## TODO

- finish demonstration of producer/consumer
- finish README for producers/consumers
- prepare for submission: (Quint's responsibility)
  - #ifdef CHANGED
  - copy Project 1 README section over to `threads/README`

---

# Project 1 README

_Group 3: Kelvin Abrokwa-Johnson, Quint Guvernator, Anna Li_

We have changed code in the following files:

  - `Makefile.common` (to add files to project)
  - `threads/main.cc` ('-P' argument parsing and new external functions)
  - `threads/synch.{cc,h}` (implement Lock and Condition)

We have added the following files to the project:

  - `threads/test_producer_consumer.cc`
  - `threads/test_elevator.cc`
  - `threads/test_locks_conditions.cc`
  - `threads/test_vdot.cc`
  - `threads/test_vdot.h`

## Note on the `-P` switch

We have designed the `-P` flag such that running the nachos executable with:

  - `-P 0` (or no flag passed) executes `ThreadTest()` in `threads/threadtest.cc`
  - `-P 1` executes `TestProducerConsumer()` in `threads/test_producer_consumer.cc` (for our own use)
  - `-P 2` executes `TestLocksConditions()` in `threads/test_locks_conditions.cc`
  - `-P 3` executes `TestElevator()` in `threads/test_elevator.cc`
  - `-P 4` executes `TestVdot()` in `threads/test_vdot.cc`


## Locks

All lock methods ensure mutual exclusion by disabling interrupts.

- `::Acquire`: when this method is called, if the lock is `FREE`,
               it is sets it to `BUSY` and execution continues. However,
               if the lock is already `BUSY`, the thread is put to sleep
               and added to a list of threads waiting on the lock.

- `::Release`: when this method is called, the lock becomes `FREE` and
               all threads on the sleeper list are awoken and they all
               attempt to acquire the lock again (all but one thread will)
               go back to sleep.

## Conditions

Condition methods are also mutually exclusive by disabling interrupts.

- `::Wait`: when this method is called, the passed in lock is released to
            allow other threads a chance to acquire the lock. The current
            thread is added to a list of threads waiting on the condition
            and put to sleep. When the thread is awoken it reacquires the lock.

- `::Signal`: when this method is called, one thread is called from the
              sleeper list and put back on the ready to run list.

- `::Broadcast`: when this method is called, all threads on the sleeper list
                are awoken.


## Producers/Consumers

Run with `nachos -P 2 -d L` along with whatever randomization flags
convince you that our solution is appropriate. It is important to turn on debug
output of type 'L' because no information relevant to the demonstration is
printed otherwise.

Relevant code is in `threads/synch.cc` and
`threads/test_producers_consumers.cc`.

You can change the `static const` parameters at the top of `threads/test_producers_consumers.cc` to your liking to change:

  - the number of producer/consumer pairs created (`static const int n = 1`)
  - the number of elements in the ring buffer (`static const int ring_size = 5`)
  - the message to produce (`static const char *const message = "Hello world\n"`)

When the buffer is empty, consumer threads wait on a condition upon which producers
broadcast whenever they produce. Similarly, when the buffer is full, producer
threads wait on a condition upon which consumers broadcast whenever they consume.
All access to the buffer is protected by a lock. Producers and consumers randomly yield 
in order to simulate a schedule.


## MGSt Hall Elevator

Run with `nachos -P 3` along with whatever randomization flags convince
you that our solution is appropriate. You may turn on lock and condition DEBUG
messages if you like by appending the flag `-d L`, but note that this will
produce *a lot* of output.

Relevant code is in `threads/test_elevator.cc`.

Our elevator and each of it's passengers run in individual threads. 
Passengers waiting for the elevator on a floor wait on a condition specific to
their floor. When the elevator arrives, it broadcasts on that condition and
passengers get on the elevator. Passengers on the elevator wait on the condition
of their destination floor. When the elevator arrives and broadcasts on that
condition, they leave the elevator. Our elevator is dumb and stops at every floor, 
which ensures that every passenger will eventually be picked up and dropped off,
within a certain amount of time. The elevator thread yields the CPU on every floor
in order to allow passengers to embark and disembark the elevator.


## VDOT Bridge Traffic

Run with `nachos -P 4` along with whatever randomization flags convince
you that our solution is appropriate. Again, you may turn on lock and condition
DEBUG messages if you like by appending the flag `-d L`, but note that this
will produce *a lot* of output.

Relevant code is in `threads/test_vdot.cc` and `threads/test_vdot.h`.

Our implementation of the solution ot the bridge traffic control problem
uses a monitor. It is encapsulated mostly in the following pseudocode:

```
Arrive(direction_desired)
    while direction_desired != direction or cars == max cars
        if bridge is full
            wait on departure condition
        else if wrong_way and no cars on bridge
            change active direction
        else if (wrong_way)
            wait on departure condition
```

Cars broadcast on a `depart` condition when they leave the bridge to notify
waiting cars that it is time to check the status of the bridge again.

The main drawback of this solution is that it does not take into consideration
the arrival time of cars so it is not necessarily "fair". The direction
of traffic flow on the bridge only changes when the bridge is empty.
That is, as long as cars continue to queue up on the side of the current flow
traffic will continue in that direction (so cars on the other side have to wait
indefinitely).

If a car arrives while traffic is currently moving in its direction, but there
is another car already waiting to cross in the opposite direction, the new arrival
well have precedence over the waiting car.


