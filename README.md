# nachos

An extension of the provided NACHOS code to be a """""real""""" operating system.

## TODO

- extend producer/consumer to multiple producers/consumers
- write README for producers/consumers
- begin elevator problem
- begin VDOT problem
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

## Note on the `-P` switch

We have designed the `-P` flag such that running the nachos executable with:

  - `-P 0` (or no flag passed) executes `ThreadTest()` in `threads/threadtest.cc`
  - `-P 1` executes `TestProducerConsumer()` in `threads/test_producer_consumer.cc` (for our own use)
  - `-P 2` executes `TestLocksConditions()` in `threads/test_locks_conditions.cc`
  - `-P 3` executes `TestElevator()` in `threads/test_elevator.cc`
  - `-P 4` executes `TestVdot()` in `threads/test_vdot.cc`

## Producers/Consumers

Please run with `nachos -P 2 -d L` along with whatever randomization flags
convince you that our solution is appropriate. It is important to turn on debug
output of type 'L' because no information relevant to the demonstration is
printed otherwise.

Relevant code is in `threads/synch.cc` and
`threads/test_producers_consumers.cc`.

You can change the `static const` parameters at the top of `threads/test_producers_consumers.cc` to your liking to change:

  - the number of producer/consumer pairs created (`static const int n = 1`)
  - the number of elements in the ring buffer (`static const int ring_size = 5`)
  - the message to produce (`static const char *const message = "Hello world\n"`)

**TODO: Producers/Consumers README**

## MGSt Hall Elevator

Please run with `nachos -P 3` along with whatever randomization flags convince
you that our solution is appropriate. You may turn on lock and condition DEBUG
messages if you like by appending the flag `-d L`, but note that this will
produce *a lot* of output.

Relevant code is in `threads/test_elevator.cc`.

**TODO: Elevator README**

## VDOT Bridge Traffic

Please run with `nachos -P 4` along with whatever randomization flags convince
you that our solution is appropriate. Again, you may turn on lock and condition
DEBUG messages if you like by appending the flag `-d L`, but note that this
will produce *a lot* of output.

Relevant code is in `threads/test_vdot.cc`.

**TODO: VDOT Bridge README**
