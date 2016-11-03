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
    machine->Run();
}

int SysExec();
int SysJoin();
SpaceId SysFork();
void SysExit();
int SysOpen();
void SysCreate();
void SysClose();
void SysWrite();
int SysRead();
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

    switch (which) {
    case SyscallException:
        switch (type) {

        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();

        case SC_Exit:
            DEBUG('a', "Exit, initiated by user code exit.\n");
            SysExit();
            break;

        case SC_Create:
            SysCreate();
            updatePC();
            break;

        case SC_Open:
            DEBUG('a', "opening file\n");
            machine->WriteRegister(2, SysOpen());
            updatePC();
            break;

        case SC_Read:
            DEBUG('a', "Read file, initiated by user program.\n");
            machine->WriteRegister(2, SysRead());
            updatePC();
            break;

        case SC_Write:
            DEBUG('a', "Write file, initiated by user program.\n");
            SysWrite();
            updatePC();
            break;

        case SC_Close:
            DEBUG('a', "closing a file\n");
            SysClose();
            updatePC();
            break;

        case SC_Fork:
            DEBUG('a', "forking a thread\n");
            machine->WriteRegister(2, SysFork());
            break;

        case SC_Join:
            DEBUG('a', "Join, initiated by user program\n");
            machine->WriteRegister(2, SysJoin());
            break;

        case SC_Exec:
            DEBUG('a', "user thread %s called exec\n", currentThread->getName());
            updatePC();
            if (SysExec() == -1)
                machine->WriteRegister(2, -1);
            break;

        default:
            printf("Undefined SYSCALL %d\n", type);
            ASSERT(false);
        }
#ifdef USE_TLB
    case PageFaultException:
        HandleTLBFault(machine->ReadRegister(BadVAddrReg));
        break;
#endif
    default:
        ;
    }
}

#ifdef CHANGED
void SysCreate() {
    char filename[MAX_FILE_NAME];
    /* don't create the file if the filename is too long */
    if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
        return;

    DEBUG('a', "creating file: %s\n", filename);
    fileSystem->Create(filename, 0);
}

int SysRead() {
    char *userland_str = (char *) machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fid = machine->ReadRegister(6);
    int bytesRead = 0;
    AddrSpace *space = currentThread->space;
    OpenFile **open_files = space->open_files;
    OpenFile *f;

    /* not a possible fid */
    if (fid > MAX_OPEN_FILES || fid < 0)
        return -1;

    f = open_files[fid];

    /* can't read from a null file */
    if (f == NULL)
        return -1;

    /* can't read from console output */
    if (!f->is_real_file && f->console_direction)
        return -1;

    int byteRead = 1;
    char c;
    bool console_read = (!f->is_real_file && !f->console_direction);
    while (size && byteRead) {
        if (console_read) {
            c = sconsole->ReadChar();
            byteRead = 1;
        } else {
            byteRead = f->Read(&c, 1);
        }
        machine->mainMemory[space->Translate((int) userland_str)] = c;
        userland_str++;
        size--;
        bytesRead += byteRead;
    }

    return bytesRead;
}

void SysWrite() {
    char *userland_str = (char *) machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fid = machine->ReadRegister(6);
    AddrSpace *space = currentThread->space;
    OpenFile **open_files = space->open_files;
    OpenFile *f;

    /* not a possible fid */
    if (fid > MAX_OPEN_FILES || fid < 0)
        return;

    f = open_files[fid];

    /* can't write to input or a null file */
    if (f == NULL)
        return;

    /* can't write to console input */
    if (!f->is_real_file && !f->console_direction)
        return;

    char c;
    bool console_write = (!f->is_real_file && f->console_direction);
    while (size) {
        c = machine->mainMemory[space->Translate((int) userland_str)];
        if (console_write) {
            sconsole->WriteChar(c);
        } else {
            f->Write(&c, 1);
        }
        userland_str++;
        size--;
    }
}

void SysClose() {
    int fid = machine->ReadRegister(4);
    OpenFile **open_files = currentThread->space->open_files;
    Semaphore *num_open_files = currentThread->space->num_open_files;
    Lock *fid_assignment = currentThread->space->fid_assignment;
    OpenFile *f;

    /* not a possible fid */
    if (fid > MAX_OPEN_FILES || fid < 0)
        return;

    f = open_files[fid];

    /* can't close a null file */
    if (f == NULL)
        return;

    const char *dbg_closing;
    if (f->is_real_file) {
        dbg_closing = "file";
    } else if (f->console_direction) {
        dbg_closing = "console output";
    } else {
        dbg_closing = "console input";
    }
    DEBUG('a', "closing %s at FID %d\n", dbg_closing, fid);

    fid_assignment->Acquire();

    /* one fewer file reference exists; close iff none remain */
    f->refcount--;
    if (f->refcount == 0)
        delete f;
    open_files[fid] = NULL;

    fid_assignment->Release();
    num_open_files->V();
}

int SysOpen() {
    char filename[MAX_FILE_NAME];
    OpenFile **open_files = currentThread->space->open_files;
    Semaphore *num_open_files = currentThread->space->num_open_files;
    Lock *fid_assignment = currentThread->space->fid_assignment;
    int fid;

    if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
        return -1;

    OpenFile *fo = fileSystem->Open(filename);
    if (fo == NULL)
        return -1;

    num_open_files->P();

    fid_assignment->Acquire();

    /* since we're past the semaphore, there ought to be a free slot here */
    for (fid = 0; open_files[fid] != NULL && fid < MAX_OPEN_FILES; fid++);

    ASSERT(fid != MAX_OPEN_FILES);

    open_files[fid] = fo;

    fid_assignment->Release();

    return fid;
}

void SysExit() {
    int exitCode = machine->ReadRegister(4);
    int tidx = currentThread->spaceId % MAX_THREADS;

    if (threads[tidx] != NULL) { // main thread will not be in there
        threads[tidx]->join->V(); // Wake up all threads `Join`ed on this one
        threads[tidx]->done = true; // you are done running
        threads[tidx]->exitCode = exitCode;
    }

    currentThread->Finish();
}

SpaceId SysFork() {
    Thread *childThread = new Thread("fork child"); // create child thread
    // copy parent's address space
    childThread->space = new(std::nothrow) AddrSpace(currentThread->space);

    /* increment reference count for each file */
    OpenFile **child_files = childThread->space->open_files;
    OpenFile *f;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        f = child_files[i];
        if (f != NULL)
            f->refcount++;
    }

    // assign space id
    // WARNING: MAY LEAD TO DEADLOCK
    spaceIdLock->Acquire();
    threadsLock->Acquire();

    int nThreads = 0;

    // TODO: the threads array only grows, things are never deleted from it
    // FIX ME
    while (threads[++_spaceId % MAX_THREADS] != NULL) {
        nThreads++;
        if (nThreads == MAX_THREADS) { // threads array is full
            return -1;
        }
    }

    childThread->spaceId = _spaceId;
    ThreadExit *exit = (ThreadExit*)malloc(sizeof(ThreadExit));
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

    return _spaceId;
}

int SysJoin() {
    SpaceId spaceId = machine->ReadRegister(4);

    int tidx = spaceId % MAX_THREADS;

    int exitCode;

    if (threads[tidx] != NULL && threads[tidx]->spaceId == spaceId) {
        threads[tidx]->joinLock->Acquire();
        threads[tidx]->join->P();
        exitCode = threads[tidx]->exitCode;
        threads[tidx]->joinLock->Release();
    }

    updatePC();

    return exitCode;
}

int SysExec() {
    char filename[MAX_FILE_NAME];

    if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
        return -1;

    OpenFile *executable = fileSystem->Open(filename);

    if (executable == NULL)
        return -1;

    currentThread->space->Exec(executable);

    delete executable;

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState(); // doesn't do anything rn

    return 0;
}
#endif /* CHANGED */

