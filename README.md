# nachos

An extension of the provided NACHOS code to be a ~real~ operating system.

## Restrictions

General rule: If there's a limit, it is 128. The following limits are relevant:

  - the longest possible input line in the shell, including the null byte
    (MAX_LINE = 128)
  - the maximum number of files that can be open in a given addrspace (and
    therefore thread), including ConsoleInput and ConsoleOutput (MAX_OPEN_FILES
    = 128)
  - the longest possible file name (MAX_FILE_NAME = 128)
  - the maximum number of living or dead-but-not-joined threads in the system
    at a given time (MAX_THREADS = 128)

## TODO

  - ...

---

# Project 2 README

_Group 3: Kelvin Abrokwa-Johnson, Quint Guvernator, Anna Li_

We have changed code in the following files:

  - ...

We have added the following files to the project:

  - ...
