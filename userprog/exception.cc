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
#define MAX_OPEN_FILES 64
#define RW_BUFFER_SIZE 128

static OpenFile *open_files[MAX_OPEN_FILES];
static bool open_files_is_init = false;

static Semaphore *num_open_files = new Semaphore("num_open_files", MAX_OPEN_FILES);
static Lock *fid_assignment = new Lock("fid_assignment");

static FileSystem *fs = new FileSystem(false);

static SynchConsole *console;

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

/* fill the open_files array with NULL */
static void try_init(void)
{
    if (open_files_is_init)
        return;

    open_files_is_init = true;
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = NULL;
}

/* import n bytes from userland into a buffer */
static void strnimport(char *buf, int n, char *virt_address)
{
    for (int i = 0; i < n; i++)
        buf[i] = machine->mainMemory[(int) virt_address + i];
}

/* TODO: address translation */
static int strimport(char *buf, int max_size, char *virt_address)
{
    int i;
    for (i = 0; i < max_size; i++) {
        if ((buf[i] = machine->mainMemory[(int) virt_address + i]) == '\0')
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
    char rw_buf[RW_BUFFER_SIZE];
    int bytes_rw;
    int n_to_rw; // number of bytes to read or write
    OpenFile *fo;
    try_init();

    int ret = 0;

    console = new SynchConsole();

    switch (which) {
    case SyscallException:
        switch (type) {

        case SC_Halt:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();

        case SC_Exit:
            /* FIXME */
            DEBUG('a', "Shutdown, initiated by user code exit.\n");
            interrupt->Halt();

        case SC_Create:

            /* don't create the file if the filename is too long */
            if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4)))
                break;

            DEBUG('a', "creating file: %s\n", filename);
            fs->Create(filename, 0);

            break;

        case SC_Open:
            DEBUG('a', "opening file %s\n", filename);

            /* don't create the file if the filename is too long */
            if (!import_filename(filename, MAX_FILE_NAME, (char *) machine->ReadRegister(4))) {
                ret = -1;
                break;
            }

            fo = fs->Open(filename);

            fid_assignment->Acquire();
            num_open_files->P();

            /* since we're past the semaphore, there ought to be a free slot here */
            for (findex = 0; open_files[findex] != NULL && findex < MAX_OPEN_FILES; findex++);
            ASSERT(findex != MAX_OPEN_FILES);

            open_files[findex] = fo;

            fid_assignment->Release();

            /* we don't store ConsoleInput and ConsoleOutput in the array */
            ret = findex + 2;
            break;

        case SC_Read:
            DEBUG('a', "Read file, initiated by user program.\n");

            userland_str = (char *) machine->ReadRegister(4);
            size = machine->ReadRegister(5);
            fid = machine->ReadRegister(6);
            findex = fid - 2;

            ret = 0;

            /* can't read from output */
            if (fid == ConsoleOutput)
                break;

            if (fid != ConsoleInput && open_files[findex] == NULL) {
                ret = -1;
                break;
            }

            while (size && bytes_rw) {
                n_to_rw = (size > RW_BUFFER_SIZE) ? RW_BUFFER_SIZE : size;

                if (fid == ConsoleInput) {
                    console->ReadBytes(rw_buf, n_to_rw);
                    bytes_rw = n_to_rw;
                } else {
                    bytes_rw = open_files[findex]->Read(rw_buf, n_to_rw);
                }
                // copy from buffer into main memory
                memcpy(&machine->mainMemory[(int) userland_str], (void *) rw_buf, n_to_rw);
                size -= bytes_rw;
                userland_str += bytes_rw;
                ret += bytes_rw;
            }

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
                n_to_rw = (size > RW_BUFFER_SIZE) ? RW_BUFFER_SIZE : size;
                strnimport(rw_buf, n_to_rw, userland_str);
                if (fid == ConsoleOutput) {
                    console->WriteBytes(rw_buf, n_to_rw);
                    bytes_rw = n_to_rw;
                } else {
                    bytes_rw = open_files[findex]->Write(rw_buf, n_to_rw);
                }
                size -= bytes_rw;
                userland_str += bytes_rw;
            }

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

            num_open_files->V();
            fid_assignment->Release();
            break;
        default:
            printf("Undefined SYSCALL %d\n", type);
            ASSERT(false);
        }
        machine->WriteRegister(2, ret);
        updatePC();
#ifdef USE_TLB
    case PageFaultException:
        HandleTLBFault(machine->ReadRegister(BadVAddrReg));
        break;
#endif
    default:
        ;
    }
}
