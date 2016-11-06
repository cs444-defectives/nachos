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

  - `Makefile.common` (to add and remove files to project)
  - `threads/system.{cc,h}` (to handle all the new stuff....??)
  - `threads/synch.{cc,h}` (add Debug statements??)
  - `threads/thread.cc` (add more attributes to Thread object??)
  - `userprog/addrspace.cc` (Change page table to non 1 to 1 mapping)
  - `userprog/exception.cc` (add functionality for system to handle more than Halt)

We have added the following files to the project:

  - `filesys/openfile.cc`
  - `filesys/openfile.h`
  - `test/arg_seq_child.c`
  - `test/argkid.c`
  - `test/argtest.c`
  - `test/cat.c`
  - `test/cp.c`
  - `test/deepfork.c`
  - `test/deepkid1.c`
  - `test/deepkid2.c`
  - `test/defective_libc.c`
  - `test/dup.c`
  - `test/echo.c`
  - `test/exec.c`
  - `test/exec_with_args.c`
  - `test/file_test.c`
  - `test/fileio.c`
  - `test/fork.c`
  - `test/fromcons.c`
  - `test/hellocons.c`
  - `test/hellofile.c`
  - `test/kid.c`
  - `test/parent_child.c`
  - `test/seq_child.c`
  - `test/share.c`
  - `test/sharekid.c`
  - `test/shell.c`
  - `threads/memorymanager.{cc, h}`
  - `userprog/synchconsole.{cc, h}`

## System Calls

All system calls were passed to the exception handler and thus implemented in exception.cc. 
Additional supporting files were modified or created to help with relevant
processes.
Our system uses a global array, open_files, to keep track of the OpenFile objects. Another global array, threads, is also used to keep track of the
threads in our system. 
A unique spaceID is given to every new thread that is created. 
Talk about exit codes, how spaceids are assigned?, other global variables..


- `::Exit`: when this syscall is called, the current thread is marked as
               'dead'. The parent thread joins with the current thread,
               and removes all of it's child threads that are marked as 
               'dead'. 

- `::Exec`: when this syscall is called, the filename is read. If the
               filename is NULL or too long, the method returns.
               This exec takes in arguments and puts them into the
               kernelspace arguments array. If any of these arguments
               are NULL or too long the method returns. In addition,
               if there is not enough kernel space for all of the
               arguments the method returns. The stack pointer is
               updated to start at the argv array.

- `::Join`: when this syscall is called, a spaceID is grabbed. The 
               global threads array is searched for an index with this
               spaceID.

- `::Create`: when this syscall is called, the filename is read into a
               buffer and a new file is created only if the filename 
               is below a certain length, MAX_FILE_NAME. 

- `::Open`: when this syscall is called, a filename is read. The method 
               returns prematurely if the filename is NULL or if it is too long. Otherwise, the file is opened and added to the global
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

- `::Fork`: when this syscall is called, the parent thread pauses and
               creates a new child thread with the same exact address
               address space (?) as itself. Any file references are
               updated. The child thread is assigned a spaceID and
               dded to our global threads array, under the protection
               of locks. The program counter is updated for both the
               both the parent and child thread, and the child thread
               begins to run. Under successful completion, the 
               spaceID of the child thread is returned. Otherwise, a -1
               is returned.

- `::Dup`: when this syscall is called, stdin or stdout is redirected
               to a file specified by the user. The reference count for the new file is increased, under protection of semaphores and locks. The new file id is returned. 

## SynchConsole

TODO??
