/*
 * Entry point into the Nachos kernel from user programs. There are two kinds
 * of things that can cause control to transfer back to here from user code:
 *
 * syscall: The user code explicitly requests to call a procedure in the Nachos
 * kernel.
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
#include "addrspace.h"
#include <string.h>

#define MAX_FILE_NAME 128
#define MAX_ARG_LEN 128
#define MAX_ARGS 16
#define SHELL_PATH "shell"
#define SCRIPT_HEADER "#SCRIPT\n"
#define CHECKPOINT_HEADER "CHECKPT\n"
#define CHECKPOINT_FOOTER "\nGOODBYE"
#define LEN_MAGIC 8

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

/* if you're using this to debug, you're screwed */
void show_memory(void)
{
    AddrSpace *space = currentThread->space;
    int size = space->size;

    const int row_length = 32;
    int i;
    char c;

    for (int row = 0; row < (size / row_length); row++) {
        printf("0x%08x    ", row * row_length);
        for (int col = 0; col < row_length; col++) {
            i = row * row_length + col;
            c = machine->mainMemory[space->Translate(i)];
            printf("%02x", (unsigned char) c);
        }
        printf(" ");

        for (int col = 0; col < row_length; col++) {
            i = row * row_length + col;
            c = machine->mainMemory[space->Translate(i)];
            printf("%c", c < ' ' ? '.' : c);
        }
        printf("\n");
    }
}

/* brings a userspace integer into kernelspace */
static int intimport(int virt_address)
{
    // make sure page is in RAM
    if (!currentThread->space->pageTable[virt_address / PageSize].valid)
        memoryManager->Fault(virt_address / PageSize);
    int src = currentThread->space->Translate(virt_address);
    if (src == 0)
        return -1;
    return WordToHost(*(unsigned int *) &machine->mainMemory[src]);
}

/* jams a kernelspace integer into userspace */
static void intexport(int data, int virt_address)
{
    // make sure page is in RAM
    if (!currentThread->space->pageTable[virt_address / PageSize].valid)
        memoryManager->Fault(virt_address / PageSize);
    int dest = currentThread->space->Translate(virt_address);
    *(unsigned int *) &machine->mainMemory[dest] = WordToHost((unsigned int) data);
}

/*
 * Brings a userspace string into kernelspace. Returns the number of bytes
 * read, or -1 if the virtual address points to NULL.
 */
static int strimport(char *buf, int max_size, int virt_address)
{
    // TODO: test and set lock
    int i, src;
    for (i = 0; i < max_size; i++) {
        // check for page fault
        if (!currentThread->space->pageTable[(virt_address + i) / PageSize].valid)
            memoryManager->Fault((virt_address + i) / PageSize);
        src = currentThread->space->Translate(virt_address + i);
        if (src == 0)
            return -1;
        if ((buf[i] = machine->mainMemory[src]) == '\0')
            break;
    }
    return i;
}

/* jams a kernelspace string into userspace, return -1 if null pointer. */
static void strexport(char *buf, int max_size, int virt_address)
{
    int i, dest;
    for (i = 0; i < max_size; i++) {
        // check for page fault
        if (!currentThread->space->pageTable[(virt_address + i) / PageSize].valid)
            memoryManager->Fault((virt_address + i) / PageSize);
        dest = currentThread->space->Translate(virt_address + i);
        if ((machine->mainMemory[dest] = buf[i]) == '\0')
            break;
    }
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

void forkCb(int _) {
    currentThread->RestoreUserState();
    currentThread->space->RestoreState();
    machine->Run();
}

static bool _create(int filename_va)
{
    char filename[MAX_FILE_NAME];
    int bytes_read = strimport(filename, MAX_FILE_NAME, filename_va);

    /* abort if we get a null string */
    if (bytes_read == -1)
        return false;

    /* don't create the file if the filename is too long */
    if (bytes_read == MAX_FILE_NAME)
        return false;

    DEBUG('a', "creating file: %s\n", filename);
    return fileSystem->Create(filename, 0);
}

static int _read(int buf_va, int size, OpenFileId fid)
{
    char *userland_str = (char *) buf_va;
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
    while (size && byteRead) {
        if (f->is_real_file) {
            byteRead = f->Read(&c, 1);
        } else {
            c = sconsole->ReadChar();
            byteRead = 1;
        }
        machine->mainMemory[space->Translate((int) userland_str)] = c;
        userland_str++;
        size--;
        bytesRead += byteRead;
    }

    return bytesRead;
}

static void _write(int buf_va, int size, OpenFileId fid)
{
    char *userland_str = (char *) buf_va;
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

    f->lock->Acquire(); // guanrantee atomic writes
    while (size) {
        if (!currentThread->space->pageTable[(int)userland_str / PageSize].valid)
            memoryManager->Fault((int)userland_str / PageSize);
        c = machine->mainMemory[space->Translate((int) userland_str)];
        if (f->is_real_file)
            f->Write(&c, 1);
        else;
            sconsole->WriteChar(c);
        userland_str++;
        size--;
    }
    f->lock->Release();
}

static void _close(OpenFileId fid)
{
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

/*
 * To let kernelspace call this open helper function to parse #SCRIPTs, we
 * assume filename has already been translated from userspace (successfully or
 * otherwise). It's ugly design, I know.
 */
static int _open(char *filename, int bytes_read)
{
    OpenFile **open_files = currentThread->space->open_files;
    Semaphore *num_open_files = currentThread->space->num_open_files;
    Lock *fid_assignment = currentThread->space->fid_assignment;
    OpenFileId fid;

    /* abort if we got a null string */
    if (bytes_read == -1)
        return -1;

    /* don't create the file if the filename is too long */
    if (bytes_read == MAX_FILE_NAME)
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

static void _exit(int exitCode)
{
    currentThread->exitCode = exitCode; // store exit code

    // iterate over thread's children and delete dead ones
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i] != NULL
            && threads[i]->parentSpaceId == currentThread->spaceId) {
            threads[i]->done = true;
        }
    }

    currentThread->dead = true; // indicate that you have finished running

    currentThread->join->V(); // permission for parent to proceed

    // TODO: figure out if parent is dead and set done <- true if so

    (void) interrupt->SetLevel(IntOff);
    currentThread->Sleep();
}

static SpaceId _fork(void)
{
    Thread *childThread = new Thread("fork child"); // create child thread

    // tell child about its parent so that it can kill itself later if need be
    childThread->parentSpaceId = currentThread->spaceId;

    // assign space id
    // WARNING: MAY LEAD TO DEADLOCK
    spaceIdLock->Acquire();

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
    spaceIdLock->Release();

    // copy parent's address space
    childThread->space = new(std::nothrow) AddrSpace(currentThread->space, _spaceId);

    /* increment reference count for each file */
    OpenFile **child_files = childThread->space->open_files;
    OpenFile *f;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        f = child_files[i];
        if (f != NULL)
            f->refcount++;
    }


    threadsLock->Acquire();
    threads[_spaceId % MAX_THREADS] = childThread; // put child thread in threads array
    threadsLock->Release();

    updatePC(); // update program counter for both parent and child

    currentThread->SaveUserState(); // save parent registers

    machine->WriteRegister(2, 0); // return 0 to child
    childThread->SaveUserState(); // save child register
    childThread->Fork((VoidFunctionPtr) forkCb, 0); // start child

    return _spaceId;
}

static int _join(SpaceId spaceId)
{
    int tidx = spaceId % MAX_THREADS;

    int exitCode;

    if (threads[tidx] != NULL && threads[tidx]->spaceId == spaceId) {
        threads[tidx]->joinLock->Acquire();
        threads[tidx]->join->P();
        exitCode = threads[tidx]->exitCode;
        threads[tidx]->done = true;
        threads[tidx]->joinLock->Release();
    }

    updatePC();

    return exitCode;
}

static bool is_script(OpenFile *f)
{
    char script_header[LEN_MAGIC + 1];
    f->ReadAt(script_header, LEN_MAGIC, 0);
    script_header[LEN_MAGIC] = '\0';
    return strcmp(script_header, (char *) SCRIPT_HEADER) == 0;
}

static bool is_checkpoint(OpenFile *f)
{
    char header[LEN_MAGIC + 1];
    f->ReadAt(header, LEN_MAGIC, 0);
    header[LEN_MAGIC] = '\0';
    return strcmp(header, (char *) CHECKPOINT_HEADER) == 0;
}

/**
 * Convert 4 chars to an int
 * Little-Endian
 */
int to_int(char *buf)
{
    int i = 0;
    i |= ((int) buf[0]);
    i |= ((int) buf[1]) << 1;
    i |= ((int) buf[2]) << 2;
    i |= ((int) buf[3]) << 3;
    return i;
}

static int _exec(int filename_va, int args_va)
{
    /* read in the name of the executable */
    char filename[MAX_FILE_NAME];
    int bytes_read = strimport(filename, MAX_FILE_NAME, filename_va);
    char script_header[LEN_MAGIC];

    /* abort if we get a null string */
    if (bytes_read == -1)
        return -1;

    /* abort if the filename is too long */
    if (bytes_read == MAX_FILE_NAME)
        return -1;

    /* get executable file, abort if it doesn't exist */
    OpenFile *executable = fileSystem->Open(filename);
    if (executable == NULL)
        return -1;

    /* arguments */
    int arg_va;
    int num_args;
    char arg_buf[MAX_ARGS][MAX_ARG_LEN];

    /*
     * If the filename passed to Exec is a script,
     * we set the actual executable to the shell,
     * passing in a special argument to disable
     * shell prompts. We then replace ConsoleInput
     * with an open copy of the script file that has
     * been seeked to the second line. This lets
     * us "interpret" script input as if it were typed
     * on the shell.
     */
    if (is_script(executable)) {

        /* replace ConsoleInput with the script file, advancing past header */
        _close((int) ConsoleInput);
        _open(filename, strlen(filename));
        currentThread->space->open_files[ConsoleInput]->Read(script_header, LEN_MAGIC);
        executable = fileSystem->Open((char *) SHELL_PATH);
        ASSERT(executable != NULL);

        /* disable prompts in the shell */
        strcpy(arg_buf[0], SHELL_FLAG_DISABLE_PROMPTS);
        num_args = 1;

    } else if (is_checkpoint(executable)) {
        char intBuf[4];

        // numPages
        executable->ReadAt(intBuf, sizeof(int), 8);
        int numPages = to_int(intBuf);

        // create the thread's AddrSpace
        AddrSpace *space = new AddrSpace(numPages);

        // disk content
        char sectorBuf[PageSize];
        for (int i = 0; i < numPages; i++) {
            executable->ReadAt(sectorBuf, PageSize, 172 + i * PageSize);
            synchDisk->WriteSector(space->sectorTable[i], sectorBuf);
        }

        // make sure the file ends with GOODBYE
        char goodbye[9];
        goodbye[8] = '\0';
        executable->ReadAt(goodbye, LEN_MAGIC, 172 + numPages * PageSize);
        ASSERT(strcmp(goodbye, CHECKPOINT_FOOTER) == 0);

        // swap address spaces
        AddrSpace *oldSpace = currentThread->space;
        currentThread->space = space;
        delete oldSpace;

        // machineState
        executable->ReadAt((char *) currentThread->userRegisters,
                NumTotalRegs * sizeof(int), 12);

        // populate registers
        currentThread->RestoreUserState();

        updatePC();

        return -1;

    /*
     * If the filename passed to Exec is a true, blue NOFF binary, we need to
     * get the actual arguments given to us by the user.
     */
    } else {

        /* if passed a null pointer, there are no args */
        if (!args_va) {
            num_args = 0;

        } else {
            for (num_args = 0; num_args < MAX_ARGS; num_args++) {

                /* get address of the string */
                arg_va = intimport(args_va + (4 * num_args));

                /* don't deference null pointers */
                if (!arg_va)
                    break;

                /* copy string argument into kernelspace arguments array */
                bytes_read = strimport(arg_buf[num_args], MAX_ARG_LEN, arg_va);

                /* stop if we get a NULL pointer or an empty string */
                if (bytes_read == -1 || !bytes_read)
                    break;

                /* abort if any argument is too long */
                if (bytes_read == MAX_ARG_LEN)
                    return -1;
            }
        }
    }

    /* abort if we ran out of space before reading all arguments */
    if (num_args == MAX_ARGS)
        return -1;

    if (!currentThread->space->Exec(executable))
        return -1;

    /* we are committed to the exec at this point */

    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    /*
     * inject arguments into the new stack
     * the following is modified from Kearns' arghalt:
     * http://www.cs.wm.edu/~kearns/444F16/program/arghalt.html
     */

    unsigned int argv[MAX_ARGS];
    unsigned int sp = machine->ReadRegister(StackReg);
    int len;

    /* put each argument into the stack */
    for (int i = 0; i < num_args; i++) {

        /* allocate enough stack space for the argument */
        len = strlen(arg_buf[i]) + 1;
        sp -= len;

        /* store the argument on the stack */
        strexport(arg_buf[i], len, sp);

        /* keep track of the address of the new argument for argv */
        argv[i] = sp;
    }

    /* align stack on quad boundary */
    sp = sp & ~3;

    /* fill the argv array, following MIPS little-endian format */
    sp -= sizeof(int) * (num_args + 1);
    for (int i = 0; i < num_args + 1; i++)
        intexport(argv[i], sp + i * sizeof(int));

    /* Put argc into R4 and the argv pointer in R5 */
    machine->WriteRegister(4, num_args);
    machine->WriteRegister(5, sp);

    /* update the stack pointer so the process starts below the argc/argv stuff */
    machine->WriteRegister(StackReg, sp - 8);

    return 0;
}

static int _dup(OpenFileId fid)
{
    OpenFile **open_files = currentThread->space->open_files;
    Semaphore *num_open_files = currentThread->space->num_open_files;
    Lock *fid_assignment = currentThread->space->fid_assignment;

    OpenFile *f = open_files[fid];

    num_open_files->P();

    fid_assignment->Acquire();

    /* since we're past the semaphore, there ought to be a free slot here */
    int new_fid;
    for (new_fid = 0; open_files[new_fid] != NULL && new_fid < MAX_OPEN_FILES; new_fid++);

    ASSERT(new_fid != MAX_OPEN_FILES);

    open_files[new_fid] = f;
    f->refcount++;

    fid_assignment->Release();

    return new_fid;
}

static int _checkPoint(int name_va)
{
    /* save state to state array */
    currentThread->SaveUserState();

    /* no no no, bad programmer */
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    AddrSpace *space = currentThread->space;

    /* read in the filename */
    char filename[MAX_FILE_NAME];
    int bytes_read = strimport(filename, MAX_FILE_NAME, name_va);

    /* abort if we get a null string */
    if (bytes_read == -1)
        return -1;

    /* abort if the filename is too long */
    if (bytes_read == MAX_FILE_NAME)
        return -1;

    /* create the file, fail if we can't */
    if (!_create(name_va))
        return -1;

    /*
     * open the file as if we were a user program so we don't have to keep
     * track of the current byte
     */
    int fid = _open(filename, bytes_read);
    OpenFile *f = space->open_files[fid];

    /* 0-7: checkpoint header */
    DEBUG('w', "writing checkpoint header\n");
    f->Write((char *) CHECKPOINT_HEADER, LEN_MAGIC);

    /* 8-11: number of pages in addrspace */
    DEBUG('w', "writing number or pages in addrspace\n");
    f->Write((char *) &space->numPages, sizeof(int));

    /* 12-171: registers */
    DEBUG('w', "writing machine registers\n");
    f->Write((char *) currentThread->userRegisters, sizeof(int) * NumTotalRegs);

    /* 172-: disk contents for each page in sector table */
    DEBUG('w', "writing thread's memory pages\n");
    char buf[SectorSize];
    for (unsigned int i = 0; i < space->numPages; i++) {

        /* if the page is in RAM, write it from there */
        if (space->pageTable[i].valid) {
            f->Write(machine->mainMemory + space->pageTable[i].physicalPage * PageSize, PageSize);

        /* if the page is not RAM-resident, read from disk then write */
        } else {
            synchDisk->ReadSector(space->sectorTable[i], buf);
            f->Write(buf, SectorSize);
        }
    }

    /* last eight bytes: a footer for sanity checking */
    f->Write((char *) CHECKPOINT_FOOTER, LEN_MAGIC);

    _close(fid);

    /* bad bad bad bad bad programmer */
    interrupt->SetLevel(oldLevel);
    return 0;
}

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

    /* for ugly _open() */
    char filename[MAX_FILE_NAME];
    int filename_va;
    int bytes_read;

    switch (which) {
    case SyscallException:
        switch (type) {

        case SC_Halt:
            DEBUG('a', "Halt, intiated by user thread <%s>\n", currentThread->getName());
            interrupt->Halt();

        case SC_Exit:
            DEBUG('a', "Exit, initiated by user thread <%s>\n", currentThread->getName());
            _exit(machine->ReadRegister(4));
            break;

        case SC_Create:
            DEBUG('a', "Create, intiated by user thread <%s>\n", currentThread->getName());
            _create(machine->ReadRegister(4));
            updatePC();
            break;

        case SC_Open:
            DEBUG('a', "Open file, initiated by user thread <%s>\n", currentThread->getName());
            filename_va = machine->ReadRegister(4);
            bytes_read = strimport(filename, MAX_FILE_NAME, filename_va);
            machine->WriteRegister(2, _open(filename, bytes_read));

            updatePC();
            break;

        case SC_Dup:
            DEBUG('a', "Dup, initiated by user thread <%s>\n", currentThread->getName());
            machine->WriteRegister(2, _dup(machine->ReadRegister(4)));
            updatePC();
            break;

        case SC_CheckPoint:
            DEBUG('a', "CheckPoint, initiated by user thread <%s>\n", currentThread->getName());
            machine->WriteRegister(2, _checkPoint(machine->ReadRegister(4)));
            updatePC();
            break;

        case SC_Read:
            DEBUG('a', "Read file, initiated by user thread <%s>\n", currentThread->getName());
            machine->WriteRegister(2, _read(machine->ReadRegister(4),
                                            machine->ReadRegister(5),
                                            machine->ReadRegister(6)));
            updatePC();
            break;

        case SC_Write:
            DEBUG('a', "Write file, initiated by user thread <%s>\n", currentThread->getName());
            _write(machine->ReadRegister(4),
                   machine->ReadRegister(5),
                   machine->ReadRegister(6));
            updatePC();
            break;

        case SC_Close:
            DEBUG('a', "Closing a file\n");
            _close(machine->ReadRegister(4));
            updatePC();
            break;

        case SC_Fork:
            DEBUG('a', "Forking thread <%s>\n", currentThread->getName());
            machine->WriteRegister(2, _fork());
            break;

        case SC_Join:
            DEBUG('a', "Join, initiated by user program\n");
            machine->WriteRegister(2, _join(machine->ReadRegister(4)));
            break;

        case SC_Exec:
            DEBUG('a', "user thread <%s> called exec\n", currentThread->getName());
            updatePC();
            if (_exec(machine->ReadRegister(4),
                      machine->ReadRegister(5)) == -1)
                machine->WriteRegister(2, -1);
            break;

        default:
            printf("Undefined SYSCALL %d\n", type);
            ASSERT(false);
        }
        break;

    case PageFaultException:
        DEBUG('a', "Thread <%s> threw a page fault for virtual page <%d>\n",
                currentThread->getName(), machine->ReadRegister(BadVAddrReg) / PageSize);
        memoryManager->Fault(machine->ReadRegister(BadVAddrReg) / PageSize);
        break;

    case ReadOnlyException:
        DEBUG('a', "Thread <%s> threw a read only exception\n");
        memoryManager->Decouple(machine->ReadRegister(BadVAddrReg) / PageSize);
        break;

#ifdef USE_TLB
    case PageFaultException:
        HandleTLBFault(machine->ReadRegister(BadVAddrReg));
        break;
#endif
    default:
        ;
    }
}
