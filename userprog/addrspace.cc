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

    /* allocate space for open files, plus console in/out cookies */
    open_files = new OpenFile* [MAX_OPEN_FILES];
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = NULL;
    num_open_files = new Semaphore("num_open_files", MAX_OPEN_FILES);

    /* allocate a fake file object for console input */
    open_files[ConsoleInput] = new OpenFile(false);
    num_open_files->P();

    /* allocate a fake file object for console output */
    open_files[ConsoleOutput] = new OpenFile(true);
    num_open_files->P();

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
    sectorTable = new int[numPages];

    for (unsigned int i = 0; i < numPages; i++) {
        sectorTable[i] = memoryManager->AllocateDiskPage(i);
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = -1; /* dummy value, clobbered immediately */
        pageTable[i].valid = false;     /* will fetch from disk on first access */
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }
#endif

    DiskBuffer *diskBuffer = new(std::nothrow) DiskBuffer(sectorTable);
    char c;

    if (noffH.code.size > 0) {
        for (int i = 0; i < noffH.code.size; i++) {
            executable->ReadAt(&c, 1, i + noffH.code.inFileAddr);
            diskBuffer->Write(&c, 1);
        }
    }

    if (noffH.initData.size > 0) {
        for (int i = 0; i< noffH.initData.size; i++) {
            executable->ReadAt(&c, 1, i + noffH.initData.inFileAddr);
            diskBuffer->Write(&c, 1);
        }
    }

    // write the last bit of data to disk
    diskBuffer->Flush();
}


/**
 * This constructor is used for `fork` calls
 * Instead of loading a program it copies the parent's address space
 * and open files
 */
AddrSpace::AddrSpace(AddrSpace *parent) {
    size = parent->size;
    numPages = parent->numPages;
    pageTable = new(std::nothrow) TranslationEntry[numPages];
    sectorTable = new int[numPages];

    for (unsigned int i = 0; i < numPages; i++) {
        sectorTable[i] = parent->sectorTable[i];
        pageTable[i].physicalPage = parent->pageTable[i].physicalPage;
        pageTable[i].valid =  parent->pageTable[i].valid;
        pageTable[i].use = parent->pageTable[i].use;
        pageTable[i].dirty = parent->pageTable[i].dirty;

        // set to read only so that we can decouple parent and child on write
        pageTable[i].readOnly = true;
        parent->pageTable[i].readOnly = true;
    }

    // copy parent's open_files array
    open_files = new OpenFile* [MAX_OPEN_FILES];
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = parent->open_files[i];
    num_open_files = parent->num_open_files;

    // copy parent's lock
    fid_assignment = parent->fid_assignment;
}

/**
 * Deallocate pages and reallocate for new executable
 */
bool AddrSpace::Exec(OpenFile *executable) {
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

    // not enough disk sectors, bail
    if ((int)numPages > memoryManager->NumSectorsAvailable())
        return false;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);

    // trash execers addr space
    Deallocate();
    delete pageTable;

    pageTable = new(std::nothrow) TranslationEntry[numPages];
    sectorTable = new int[numPages];


    for (unsigned int i = 0; i < numPages; i++) {
        sectorTable[i] = memoryManager->AllocateDiskPage(i);
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = -1; /* dummy value, clobbered immediately */
        pageTable[i].valid = false;     /* will fetch from disk on first access */
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
    }

    DiskBuffer *diskBuffer = new(std::nothrow) DiskBuffer(sectorTable);
    char c;

    if (noffH.code.size > 0) {
        for (int i = 0; i < noffH.code.size; i++) {
            executable->ReadAt(&c, 1, i + noffH.code.inFileAddr);
            diskBuffer->Write(&c, 1);
        }
    }

    if (noffH.initData.size > 0) {
        for (int i = 0; i< noffH.initData.size; i++) {
            executable->ReadAt(&c, 1, i + noffH.initData.inFileAddr);
            diskBuffer->Write(&c, 1);
        }
    }

    // write the last bit of data to disk
    diskBuffer->Flush();
    return 0;
}

/* for writing executables to disk */
DiskBuffer::DiskBuffer(int *st) {
    sectorTable = st;
    bidx = 0;
    stidx = 0;
}

int DiskBuffer::Write(char *data, int numBytes) {
    for (int i = 0; i < numBytes; i++, bidx++) {
        buffer[bidx] = data[i];
        if (bidx == SectorSize - 1) {/* buffer is full */
            Flush();
            bidx = -1; // will be `++`ed in this for-loop
        }
    }
    return 0; // TODO: return number of bytes written
}

void DiskBuffer::Flush() {
    if (bidx > 0) { // if buffer is not empty
        DEBUG('z', "Writing to sector %d\n", sectorTable[stidx]);
        synchDisk->WriteSector(sectorTable[stidx], buffer);
        stidx++;
        bidx = 0;
    }
}

/**
 * deallocate allocated pages
 */
void AddrSpace::Deallocate() {
    for (unsigned int i = 0; i < numPages; i++)
        memoryManager->DeallocateDiskPage(sectorTable[i]);
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
    DEBUG('q', "Translating: %d\n\t virtual page: %d\n\t offset: %d\n\t physical page: %d\n",
            virtAddr, vpn, offset, ppn);
    return (ppn * PageSize) + offset;
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   Deallocate();
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
