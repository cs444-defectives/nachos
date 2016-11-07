# nachos

An extension of the provided NACHOS code to be a ~real~ operating system.

## Caveats

During testing, we modified the test programs in the following ways:

  - Where Exec had the old, without-args signature, we added an additional NULL
    argument to get them to run with the new signature.
  - Where Exec gives the name of a test binary, we qualified the name with
    'test/' so that nachos can be run in the same place that `make` is run: the
    main Nachos 2 directory. To run echo in the shell, for example, the session
    would look like this:

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

Also, note that the shell accepts redirects at the END of the input line, in
any order, separated by spaces. Only the last of each input/output redirect
spec is actually followed. For example:

```
defectives> test/cat Makefile > somefile      # OK, output goes to 'somefile'
defectives> test/cat Makefile > a > somefile  # OK, but output goes to 'somefile', not 'a'
defectives> test/fromcons < somefile > a      # OK, output to 'a' and input from 'somefile'
defectives> > somefile test/cat Makefile      # NOPE
defectives> test/cat Makefile >somefile       # NOPE
defectives> test/cat Makefile >> somefile     # NOPE
```

True to its name, `cat` can accept multiple files and will concatenate them. It
also accepts the special filename "-" (without quotes) to read from stdin. This
is mostly for testing. Unfortunately we can't read until ^D (EOF) like unix
`cat` because of some hokey business in the machine-emulated filesystem, so we
instead read until newline.

General rule: If there's a numeric limit, it is 128. The following limits are
relevant:

  - the longest possible input line in the shell, including the null byte
    (MAX_LINE = 128)
  - the maximum number of files that can be open in a given addrspace (and
    therefore thread), including ConsoleInput and ConsoleOutput (MAX_OPEN_FILES
    = 128)
  - the longest possible file name (MAX_FILE_NAME = 128)
  - the maximum number of living or dead-but-not-joined threads in the system
    at a given time (MAX_THREADS = 128)

There is one exception to the general numeric limit rule:

  - you can pass a maximum of 16 arguments to programs spawned with Exec()
    (MAX_ARGS = 16)

---

# Project 2 README

_Group 3: Kelvin Abrokwa-Johnson, Quint Guvernator, Anna Li_

We have changed code in the following files:

  - Makefile.common
  - threads/scheduler.cc
  - threads/synch.cc
  - threads/synch.h
  - threads/system.cc
  - threads/system.h
  - threads/thread.cc
  - threads/thread.h
  - userprog/addrspace.cc
  - userprog/addrspace.h
  - userprog/exception.cc
  - userprog/progtest.cc
  - userprog/synchconsole.cc
  - userprog/synchconsole.h
  - userprog/syscall.h

We have added the following files to the project:

  - threads/memorymanager.cc
  - threads/memorymanager.h

## System Calls

All system calls were passed to the exception handler and thus implemented in
exception.cc. We use several helper functions to prevent the switch block from
growing out of control and to enable (modified and limited) calling of syscalls
from other syscalls (especially useful when implementing #SCRIPT). Additional
supporting files were modified or created to help with relevant processes. Our
system uses an array `open_files` for each addrspace to keep track of OpenFile
objects. We track threads in a global array `threads`. Threads can be alive or
dead. Dead threads can either be `!done`, which keeps their return value for a
potential future Join(), or `done`, where they will be garbage collected at the
next context switch. Living threads can also be `done`, meaning that their
parents are now dead and no longer care about the return value of the thread. A
unique SpaceID is given to every new thread that is created, where each new
SpaceID is one higher than the last. These IDs are not reclaimed, so each is
unique even if the thread's data is long gone.

- `::Exit`: when this syscall is called, the current thread is marked as `dead`
  and the argument given is set as the thread's return value. The parent would
  get this return value on a future Join(). Threads also `V()` their join semaphore
  on Exit to ensure a joining thread has permission to proceed.

- `::Exec`: when this syscall is called, the filename and arguments are read in
  from userspace. If the file given is an NOFF binary, it gets its own address
  space, inherits files from the parent, etc. If the file given is a #SCRIPT,
  we instead create an addrspace for the userspace shell binary, suppress its
  prompt output, trick the shell into thinking that the #SCRIPT file is console
  input, and run it like we would normally run the shell.

- `::Join`: when this syscall is called, we find the appropriate thread in our
  global `threads` array (its index is `spaceId % MAX_THREADS`). The joining
  thread the `P()`'s the joined thread's join semaphore. It then marks that
  thread as `done` (meaning no more information is desired of that thread);

- `::Create`: when this syscall is called, the filename is read into a
               buffer and a new file is created only if the filename
               is below a certain length, MAX_FILE_NAME.

- `::Open`: when this syscall is called, a filename is read. The method
               returns prematurely if the filename is NULL or if it is too long.
               Otherwise, the file is opened and added to the global
               open_files array. The latter operation is protected under
               a combination of locks and semaphores.

- `::Read`: when this syscall is called, the id of the OpenFile is
               grabbed. If the file does not exist, is null, or is the
               console output, the method returns, and nothing is read.
               Otherwise, we read the whole file into main memory and
               return the number of bytes read.

- `::Write`: when this syscall is called, the id of the OpenFile is
               grabbed. If the file does not exist, is null, or is the
               console input, the method returns, and nothing is written.
               Otherwise, we write the whole file into main memory(?).

- `::Close`: when this syscall is called, the id of the OpenFile is
               grabbed. If the file does not exist or is null, the
               method returns. Otherwise, the number of file
               references for this OpenFile is decremented by 1. This
               operation is protected using semaphores.
               If the resulting file references is 0, the OpenFile is
               removed from the global array of open_files.

- `::Fork`: when this syscall is called, new Thread and Addrspace objects
  are created. The AddrSpace object is instatiated with a constructor that
  takes the parent address space and copies all appropriate information. The
  new thread is assigned the next smallest spaceId such that the index
  `spaceId % MAX_THREADS` in the threads array is an empty slot. We
  populate parent and child registers appriately and start the child with a
  call to `machine->Run()`

- `::Dup`: when this syscall is called, stdin or stdout is redirected
   to a file specified by the user. The reference count for the new
   file is increased, under protection of semaphores and locks.
   The new file id is returned.


### Atomic Read/Write

Each `OpenFile` object has a `lock` field. On reads, the calling
thread `Acquire`'s the lock before doing all of its writing for that
call. It then releases the lock. this way, only only thread can
be writing or reading at a time.

### Helpers

We implemented programs like `echo` and libraries like `defective_libc.c`
which really smoothed out our development flows. We also implemented a
number of extra tests to ensure we were meeting requirements.
