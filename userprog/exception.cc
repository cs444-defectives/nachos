/*
 * Entry point into the Nachos kernel from user programs. There are two kinds
 * of things that can cause control to transfer back to here from user code:
 *
 * syscall: The user code explicitly requests to call a procedure in the Nachos
 * kernel. Right now, the only function we support is "Halt".
 *
 * exceptions: The user code does something that the CPU can't handle. For
 * instance, accessing memory that doesn't exist, arithmetic errors, etc.
 *
 * (N.b. interrupts which can also cause control to transfer from user code
 * into the Nachos kernel are handled elsewhere.)
 */

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "filesys.h"
#include "synch.h"
#include "synchconsole.h"
#include <string.h>

#define MAX_FILE_NAME 128
#define RW_BUFFER_SIZE 128

#ifdef USE_TLB

/*
 * Called on TLB fault. Note that this is not necessarily a page fault.
 * Referenced page may be in memory.
 *
 * If free slot in TLB, fill in translation info for page referenced.
 * Otherwise, select TLB slot at random and overwrite with translation info for
 * page referenced.
 */

void HandleTLBFault(int vaddr)
{
    int vpn = vaddr / PageSize;
    int victim = Random() % TLBSize;
    int i;

    stats->numTLBFaults++;

    /* first, see if free TLB slot */
    for (i=0; i<TLBSize; i++) {
        if (machine->tlb[i].valid == false) {
            victim = i;
            break;
        }
    }

    /* otherwise clobber random slot in TLB, assuming 1-1 mapping */
    machine->tlb[victim].virtualPage = vpn;
    machine->tlb[victim].physicalPage = vpn;
    machine->tlb[victim].valid = true;
    machine->tlb[victim].dirty = false;
    machine->tlb[victim].use = false;
    machine->tlb[victim].readOnly = false;
}

#endif

/* TODO: address translation */
static int strimport(char *buf, int max_size, char *virt_address)
{
    int i;
    for (i = 0; i < max_size; i++) {
        if ((buf[i] = machine->mainMemory[currentThread->space->Translate((int) virt_address + i)]) == '\0')
            break;
    }
    return i;
}

/* copies a filename from userland, returns true if it fit in the buffer */
static bool import_filename(char *buf, int size, char *virt_address)
{
    return strimport(buf, size, virt_address) != size;
}

/* update the program counter */
static void updatePC()
{
    int pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc);
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc);
    pc += 4;
    machine->WriteRegister(NextPCReg, pc);
}

#ifdef CHANGED
void forkCb(int _) {
    currentThread->RestoreUserState();
    currentThread->space->RestoreState();
    //currentThread->wakeParent->V();
    machine->Run();
}
#endif

/*
 * Entry point into the Nachos kernel. Called when a user program is executing,
 * and either does a syscall, or generates an addressing or arithmetic
 * exception.
 *
 * For system calls, the following is the calling convention:
 *
 *   - r2: system call code
 *   - r4: arg1
 *   - r5: arg2
 *   - r6: arg3
 *   - r7: arg4
 *
 * The result of the system call, if any, must be put back into r2. And don't
 * forget to increment the pc before returning. (Or else you'll loop making the
 * same system call forever!
 *
 * "which" is the kind of exception. The list of possible exceptions are in
 * machine.h.
 */
void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    char filename[MAX_FILE_NAME];
    int findex, fid, size;
    char *userland_str;
    OpenFile *fo;
    char c;
    int byteRead;
    int tidx;

    // fork
    Thread *childThread;
    int nThreads;
    ThreadExit *exit;

    // join
    SpaceId spaceId;

    // exit
    int exitCode;

    // exec
    OpenFile *executable;

    /* aliases for convenience and to save on memory accesses */
    AddrSpace *space = currentThread->space;
    OpenFile **open_files = space->open_files;
    Semaphore *num_open_files = space->num_open_files;
    Lock *fid_assignment = space->fid_assignment;

    int ret = 0;

    switch (which) {
    case SyscallException:
        switch (type) {

        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();

        case SC_Exit:
            DEBUG('a', "Exit, initiated by user code exit.\n");

            exitCode = machine->ReadRegister(4);

            tidx = currentThread->spaceId % MAX_THREADS;

            // Wake up all threads `Join`ed on this one
            threads[tidx]->join->V();

            // you are done running
            threads[tidx]->done = true;
            threads[tidx]->exitCode = exitCode;

            currentThread->Finish();

            break;

        case SC_Create:

            /* don't create the file if the filename is too long */
            if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
                break;

            DEBUG('a', "creating file: %s\n", filename);
            fileSystem->Create(filename, 0);

            updatePC();

            break;

        case SC_Open:
            DEBUG('a', "opening file %s\n", filename);

            /* don't create the file if the filename is too long */
            if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4))) {
                ret = -1;
                break;
            }

            fo = fileSystem->Open(filename);

            num_open_files->P();

            fid_assignment->Acquire();

            /* since we're past the semaphore, there ought to be a free slot here */
            for (findex = 0; open_files[findex] != NULL && findex < MAX_OPEN_FILES; findex++);
            ASSERT(findex != MAX_OPEN_FILES);

            open_files[findex] = fo;

            fid_assignment->Release();

            /* we don't store ConsoleInput and ConsoleOutput in the array */
            ret = findex + 2;

            updatePC();

            break;

        case SC_Read:
            DEBUG('a', "Read file, initiated by user program.\n");

            userland_str = (char *) machine->ReadRegister(4);
            size = machine->ReadRegister(5);
            fid = machine->ReadRegister(6);
            findex = fid - 2;

            ret = 0;

            /* can't read from output */
            if (fid == ConsoleOutput) {
                ret = -1;
                break;
            }

            /* no such file */
            if (fid != ConsoleInput && open_files[findex] == NULL) {
                ret = -1;
                break;
            }

            byteRead = 1;

            while (size && byteRead) {
                if (fid == ConsoleInput) {
                    c = sconsole->ReadChar();
                    byteRead = 1;
                } else {
                    byteRead = open_files[findex]->Read(&c, 1);
                }
                machine->mainMemory[space->Translate((int) userland_str)] = c;
                userland_str++;
                size--;
                ret += byteRead;
            }

            updatePC();

            break;

        case SC_Write:
            DEBUG('a', "Write file, initiated by user program.\n");

            userland_str = (char *) machine->ReadRegister(4);
            size = machine->ReadRegister(5);
            fid = machine->ReadRegister(6);
            findex = fid - 2;

            /* can't write to input */
            if (fid == ConsoleInput)
                break;

            if (fid != ConsoleOutput)
                ASSERT(open_files[findex] != NULL);

            while (size) {
                c = machine->mainMemory[space->Translate((int) userland_str)];
                if (fid == ConsoleOutput) {
                    sconsole->WriteChar(c);
                } else {
                    open_files[findex]->Write(&c, 1);
                }
                userland_str++;
                size--;
            }

            updatePC();

            break;

        case SC_Close:
            fid = machine->ReadRegister(4);

            /* console can't be opened or closed */
            if (fid == ConsoleInput || fid == ConsoleOutput) {
                DEBUG('a', "ignoring silly console close request\n");
                break;
            }

            DEBUG('a', "closing file with FID %d\n", fid);

            /* we don't store ConsoleInput and ConsoleOutput in the array */
            findex = fid - 2;
            ASSERT(open_files[findex] != NULL);

            fid_assignment->Acquire();

            delete open_files[findex];
            open_files[findex] = NULL;

            fid_assignment->Release();
            num_open_files->V();

            updatePC();

            break;

        case SC_Fork:
            DEBUG('a', "forking a thread\n");

            childThread = new Thread("fork child"); // create child thread
            childThread->space = new(std::nothrow) AddrSpace(currentThread->space); // copy parent's address space

            // assign space id
            // WARNING MAY LEAD TO DEADLINE
            spaceIdLock->Acquire();
            threadsLock->Acquire();

            nThreads = 0;

            while (threads[++_spaceId % MAX_THREADS] != NULL) {
                nThreads++;
                if (nThreads == MAX_THREADS) { // threads array is full
                    ret = -1;
                    return;
                }
            }

            childThread->spaceId = _spaceId;
            exit = (ThreadExit*)malloc(sizeof(ThreadExit));
            exit->spaceId = _spaceId;
            exit->done = false;
            exit->join = new(std::nothrow) Semaphore("join semaphore", 0);
            exit->joinLock = new(std::nothrow) Lock("join lock");
            threads[_spaceId % MAX_THREADS] = exit;

            threadsLock->Release();
            spaceIdLock->Release();

            updatePC(); // update program counter for both parent and child

            currentThread->SaveUserState(); // save parent registers

            machine->WriteRegister(2, 0); // return 0 to child

            childThread->SaveUserState(); // save child register

            childThread->Fork((VoidFunctionPtr) forkCb, 0); // start child

            //childThread->wakeParent->P(); // put parent to sleep

            ret = _spaceId;
            //machine->WriteRegister(2, _spaceId); // return space id to parent

            break;

        case SC_Join:
            DEBUG('a', "Join, initiated by user program\n");

            spaceId = machine->ReadRegister(4);

            tidx = spaceId % MAX_THREADS;

            if (threads[tidx] != NULL && threads[tidx]->spaceId == spaceId) {
                threads[tidx]->joinLock->Acquire();
                threads[tidx]->join->P();
                ret = threads[tidx]->exitCode;
                threads[tidx]->joinLock->Release();
            }

            updatePC();

            break;

        case SC_Exec:
            DEBUG('a', "user thread %s called exec\n", currentThread->getName());

            if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
                break;

            executable = fileSystem->Open(filename);

            if (executable == NULL) {
                fprintf(stderr, "Unable to open file %s\n", filename);
                break;
            }

            currentThread->space->Exec(executable);

            delete executable;

            currentThread->space->InitRegisters();
            currentThread->space->RestoreState(); // doesn't do anything rn

            break;

        default:
            printf("Undefined SYSCALL %d\n", type);
            ASSERT(false);
        }
        machine->WriteRegister(2, ret);
#ifdef USE_TLB
    case PageFaultException:
        HandleTLBFault(machine->ReadRegister(BadVAddrReg));
        break;
#endif
    default:
        ;
    }
}
