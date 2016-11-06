# nachos

An extension of the provided NACHOS code to be a ~real~ operating system.

## Test program modifications

During testing, we modified the test programs in the following ways:

  - Where Exec had the old, without-args signature, we added an additional NULL argument to get them to run with the new signature.
  - Where Exec gives the name of a test binary, we qualified the name with 'test/' so that nachos can be run in the same place that `make` is run: the main Nachos 2 directory. To run echo in the shell, for example, the session would look like this:

```bash
$ cd nachos2
$ make
$ userprog/nachos -x test/shell
defectives> test/echo hello world!
hello world!
defectives> test/halt
Machine halting!

Ticks: total 12147978905, idle 12147973058, system 2320, user 3527
Disk I/O: reads 0, writes 0
Console I/O: reads 33, writes 37
Paging: faults 0
Network I/O: packets received 0, sent 0

Cleaning up...
```

## Numeric limits

General rule: If there's a limit, it is 128. The following limits are relevant:

  - the longest possible input line in the shell, including the null byte
    (MAX_LINE = 128)
  - the maximum number of files that can be open in a given addrspace (and
    therefore thread), including ConsoleInput and ConsoleOutput (MAX_OPEN_FILES
    = 128)
  - the longest possible file name (MAX_FILE_NAME = 128)
  - the maximum number of living or dead-but-not-joined threads in the system
    at a given time (MAX_THREADS = 128)

Exception:

  - you can pass a maximum of 16 arguments to programs spawned with Exec()
    (MAX_ARGS = 16)

## TODO

  - Exec with args and shell (cp, cat) testing
  - Test against Kearns' new test programs

---

# Project 2 README

_Group 3: Kelvin Abrokwa-Johnson, Quint Guvernator, Anna Li_

We have changed code in the following files:

  - ...

We have added the following files to the project:

  - ...
