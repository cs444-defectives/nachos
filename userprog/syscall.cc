/**
 * syscalls
 */
#include "system.h"
#include "syscall.h"
#include "filesys.h"
#include "list.h"
#include "synch.h"
#include <new>

#define MAX_FILE_NAME 128

#define MAX_OPEN_FILES 64
static OpenFile *open_files[MAX_OPEN_FILES];
static bool open_files_is_init = false;

static Semaphore *num_open_files = new Semaphore("num_open_files", MAX_OPEN_FILES);
static Lock *fid_assignment = new Lock("fid_assignment");

static FileSystem *fs = new FileSystem(false);

/* fill the open_files array with NULL */
static void try_init(void)
{
    if (open_files_is_init)
        return;

    open_files_is_init = true;
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        open_files[i] = NULL;
}

/* TODO: address translation */
static int strimport(char *buf, int size, char *virt_address)
{
    int i;
    for (i = 0; i < size; i++) {
        if ((buf[i] = machine->mainMemory[(int) virt_address++]) == '\0')
            break;
    }
    return i;
}

/* copies a filename from userland, returns true if it fit in the buffer */
static bool import_filename(char *buf, int size, char *virt_address)
{
    return strimport(buf, size, virt_address) != size;
}

void Create(char *user_filename)
{
    try_init();

    /* don't create the file if the filename is too long */
    char filename[MAX_FILE_NAME];
    if (!import_filename(filename, MAX_FILE_NAME, user_filename))
        return;

    DEBUG('a', "creating file: %s\n", filename);
    fs->Create(filename, 0);
}

OpenFileId Open(char *user_filename)
{
    try_init();

    /* don't create the file if the filename is too long */
    char filename[MAX_FILE_NAME];
    if (!import_filename(filename, MAX_FILE_NAME, user_filename))
        return -1;

    DEBUG('a', "opening file %s\n", filename);
    OpenFile *fo = fs->Open(filename);

    fid_assignment->Acquire();
    num_open_files->P();

    /* since we're past the semaphore, there ought to be a free slot here */
    int findex;
    for (findex = 0; open_files[findex] != NULL && findex < MAX_OPEN_FILES; findex++);
    ASSERT(findex != MAX_OPEN_FILES);

    open_files[findex] = fo;

    fid_assignment->Release();

    /* we don't store ConsoleInput and ConsoleOutput in the array */
    return findex + 2;
}

void Close(OpenFileId id)
{
    try_init();

    DEBUG('a', "closing file with FID %d\n", id);

    /* we don't store ConsoleInput and ConsoleOutput in the array */
    int findex = id - 2;

    fid_assignment->Acquire();

    delete open_files[findex];
    open_files[findex] = NULL;

    num_open_files->V();
    fid_assignment->Release();
}
