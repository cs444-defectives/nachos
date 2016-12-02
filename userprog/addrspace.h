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
#include "syscall.h"

#define UserStackSize		1024 	// increase this as necessary!
#define MAX_OPEN_FILES 128

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					                    // initializing it with the program
					                    // stored in the file "executable"
    AddrSpace(AddrSpace *parent, SpaceId spaceId); // Constructor for forking
    AddrSpace(int numPages);
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
    unsigned int size;    // make these public for forking reasons
    unsigned int numPages;
    bool Exec(OpenFile *executable);
    void Deallocate();

    /* maps user pages to disk sectors */
    int *sectorTable;

#ifndef USE_TLB
    TranslationEntry *pageTable;	// Assume linear page table translation
#endif					// for now!
};

class DiskBuffer {
  public:
    DiskBuffer(int *sectorTable);
    ~DiskBuffer();
    int Write(char *data, int numBytes); // write bytes to buffer
    void Flush(); // write buffer to disk
  private:
    char buffer[SectorSize];
    int *sectorTable;
    int bidx; // buffer index
    int stidx; // sector table index
};

#endif // ADDRSPACE_H
