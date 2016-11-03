// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "synch.h"

#define UserStackSize		1024 	// increase this as necessary!
#define MAX_OPEN_FILES 64

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    AddrSpace(AddrSpace *parent);
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    int Translate(int virtAddr); // translate a virtual page address
                                 // to a physical page address

    OpenFile **open_files;
    Semaphore *num_open_files;
    Lock *fid_assignment;
#ifdef CHANGED
    unsigned int size;    // make these public for forking reasons
    unsigned int numPages;
    void Exec(OpenFile *executable);
    void Deallocate();
#endif

  private:
#ifndef USE_TLB
    TranslationEntry *pageTable;	// Assume linear page table translation
#endif					// for now!

#ifndef CHANGED
    unsigned int numPages;		// Number of pages in the virtual
#endif
					// address space
};

#endif // ADDRSPACE_H
