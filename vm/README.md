# Nachos 3

## Regrading

Please regrade the 'bogus' tests that cause us to run out of memory. We have
also fixed exec so that the first argument isn't automatically set to the
executable name in the kernel; we have moved that responsibility to the shell
(and of course user programs that call Exec with args).

## Implementation outline

### Virtual memory implementation

- When we first start a userprogram, we allocate
  however many disk sectors are required for the
  program and record the allocations in a disk
  sector bitmap
- We have a `sectorTable` array that translates
  a thread's virtual pages to disk sectors (this
  is essentially an extension of the thread's page
  table)
- We then load the code and data segments of the
  program onto the disk in pages
  - We wrote a `DiskBuffer` class into which we
    load bytes incrememntally. It handles dumping
    pages to disk sectors so that we don't have to
    about where the code segment ends and data segment
    begins
- We rely on page faults to bring any code into memory

### Disk Sectors

- Page faults are handled by our `MemoryManager::Fault`
  - If a page is available in RAM, we move the faulted
    page from disk to RAM without updating the program
    counter, that way the thread reattempts the read/write
  - If a RAM page is unavailable we evict one using our
    eviction algorithm (outlined below)
- Disk sectors are tracked in an array of `DiskPageDescriptor`
  structs with fields
  - `processes`: a linked list describing processes
    associated with the sector. It holds the processes
    space id and virtual page
  - `ram_page`: the sectors location in RAM (or `-1` if it
    isn't RAM-resident)
  - `refCount`: the number of threads that currently use
    this sector


### RAM Eviction

- The RAM eviction algorithm is kinda dumb...

### COW

- On fork, all of the caller's sectors are used as
  the child's sectors. They are marked `readOnly` to
  trigger an exception on write.
- On `ReadOnlyException`, we call `MemoryManager::Decouple`
  while copies the page in question into a new sectors,
  updates the thread's `sectorTable`, remove the thread from
  the sectors `processes` linked list and add it the new
  sectors list, and set the page table's `readOnly = false`

## Caveats

Tests, specifically #SCRIPT tests, MUST BE run from the test directory. For
example:

    $ cd submissions/g3/nachos
    $ vm/nachos -x test/shell
    defectives> test/script.txt
    ...

WILL NOT WORK. Instead, do this:

    $ cd submissions/g3/nachos/test
    $ ../vm/nachos -x shell
    defectives> script.txt
    ...

This is because Nachos needs to know where the shell binary lives, so that it
can Exec() thes shell under the hood and shove the OpenFile of the script into
where shell expects ConsoleInput to be.
