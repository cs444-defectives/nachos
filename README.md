# nachos

An operating system kernel and coreutils, implemented in C++ for simulated hardware. Written for CS415 "Operating Systems" in Fall 2016, taught by Phil Kearns.

The project is ultimately based on [NACHOS](https://en.wikipedia.org/wiki/Not_Another_Completely_Heuristic_Operating_System), an operating system framework for a simulated PC.

We implemented:

- a filesystem
- interrupt handling
- support to run programs in userspace
  - virtual memory and isolation for the userspace program
  - a syscall interface for kernel interaction from userspace
  - userspace at-will heap allocation
  - I/O stuff
- a few userspace coreutils (mostly to test the above)
- a synchronized multithreading API
- copy-on-write for virtual memory

From @qguv:

> I consider myself _extremely_ lucky to have been able to take this class. Five years later, this toy OS is still one of my all-time favorite projects. My sincere thanks to Kearns and my teammates <3
