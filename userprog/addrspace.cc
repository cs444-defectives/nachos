// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include <new>

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
#ifndef CHANGED
    unsigned int size;
#endif /* not CHANGED */

    /* allocate space for open files, plus console in/out cookies */
    open_files = new OpenFile* [MAX_OPEN_FILES];
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = NULL;
    num_open_files = new Semaphore("num_open_files", MAX_OPEN_FILES);

    /* allocate a fake file object for console input */
    open_files[ConsoleInput] = new OpenFile(false);
    num_open_files->V();

    /* allocate a fake file object for console output */
    open_files[ConsoleOutput] = new OpenFile(true);
    num_open_files->V();

    fid_assignment = new Lock("fid_assignment");

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
#ifndef USE_TLB
    pageTable = new(std::nothrow) TranslationEntry[numPages];
#ifdef CHANGED // page table non 1:1
    for (unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = memoryManager->AllocatePage();
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;  // if the code segment was entirely on
                        // a separate page, we could set its
                        // pages to be read-only
    }
#else
// first, set up the translation
    for (unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
        pageTable[i].physicalPage = i;
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;  // if the code segment was entirely on
                        // a separate page, we could set its
                        // pages to be read-only
    }
#endif /* CHANGED - page table non 1:1 */
#endif

#ifdef CHANGED
// zero out the pages allocated to this process
    for (unsigned int i = 0; i < numPages; i++) {
        bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
    }
#else
// zero out the entire address space, to zero the unitialized data segment
// and the stack segment
    bzero(machine->mainMemory, size);
#endif /* CHANGED */

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
#ifdef CHANGED
        for (int i = 0; i < noffH.code.size; i++)
            executable->ReadAt(&machine->mainMemory[Translate(i)], 1, i + noffH.code.inFileAddr);
#else
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
#endif /* CHANGED */
    }
    if (noffH.initData.size > 0) {
#ifdef CHANGED
        for (int i = 0; i < noffH.initData.inFileAddr; i++)
            executable->ReadAt(&machine->mainMemory[Translate(i + noffH.code.size)], 1, i + noffH.initData.inFileAddr);
#else
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
#endif /* CHANGED */
    }
}


#ifdef CHANGED
/**
 * This constructor is used for `fork` calls
 * Instead of loading a program it copies the parent's address space
 * and open files
 */
AddrSpace::AddrSpace(AddrSpace *parent) {
    size = parent->size;
    numPages = parent->numPages;
    pageTable = new(std::nothrow) TranslationEntry[numPages];

    for (unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = memoryManager->AllocatePage();
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }

    // copy parent's memory into childs space
    for (unsigned int i = 0; i < size; i++)
        machine->mainMemory[Translate(i)] = machine->mainMemory[parent->Translate(i)];

    // copy parent's open file
    open_files = new OpenFile* [MAX_OPEN_FILES];
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = parent->open_files[i];
}

/**
 * Deallocate pages and reallocate for new executable
 */
void AddrSpace::Exec(OpenFile *executable) {
    Deallocate();
    delete pageTable;

    NoffHeader noffH;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);

    pageTable = new(std::nothrow) TranslationEntry[numPages];

    for (unsigned int i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = memoryManager->AllocatePage();
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;  // if the code segment was entirely on
                        // a separate page, we could set its
                        // pages to be read-only
    }

    for (unsigned int i = 0; i < numPages; i++) {
        bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
    }

    // read code into memory
    if (noffH.code.size > 0) {
        for (int i = 0; i < noffH.code.size; i++)
            executable->ReadAt(&machine->mainMemory[Translate(i)], 1, i + noffH.code.inFileAddr);
    }

    // read data segment into memory
    if (noffH.initData.size > 0) {
        for (int i = 0; i < noffH.initData.inFileAddr; i++)
            executable->ReadAt(
                    &machine->mainMemory[Translate(i + noffH.code.size)],
                    1,
                    i + noffH.initData.inFileAddr);
    }
}

/**
 * deallocate allocated pages
 */
void AddrSpace::Deallocate() {
    for (unsigned int i = 0; i < numPages; i++) {
        memoryManager->DeallocatePage(pageTable[i].physicalPage);
    }
}

/**
 * Translate virtual addresses from this address space into physical
 * addresses
 */
int AddrSpace::Translate(int virtAddr) {
    unsigned int vpn, offset, ppn;
    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    ppn = pageTable[vpn].physicalPage;
    return (ppn * PageSize) + offset;

}
#endif

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
#ifdef CHANGED
   Deallocate();
#endif
#ifndef USE_TLB
   delete pageTable;
#endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table,
//      IF address translation is done with a page table instead
//      of a hardware TLB.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
#ifndef USE_TLB
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#endif
}
