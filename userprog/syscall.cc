/**
 * syscalls
 */
#include "system.h"
#include "syscall.h"
#include "filesys.h"
#include "list.h"
#include <new>

#define MAX_FILE_NAME 128

#define MAX_OPEN_FILES 64
OpenFile open_files[MAX_OPEN_FILES] = new OpenFile[MAX_OPEN_FILES];

Semaphore num_open_files = new Semaphore("num_open_files", MAX_OPEN_FILES);
Lock fid_assignment = new Lock("fid_assignment");

static FileSystem fs = new FileSystem(false);

/* TODO: address translation */
static int strimport(char *buf, int size, int virt_address)
{
    int i;
    for (i = 0; i < size; i++) {
        if ((buf[i] = machine->mainMemory[virt_address++]) == '\0')
            break;
    }
    return i;
}

/* copies a filename from userland, returns true if it fit in the buffer */
static bool import_filename(char *buf, int size, int virt_address)
{
    return strimport(buf, size, virt_address) != size;
}

void Create(char *file_name)
{
    DEBUG('F', "creating file: %s\n", file_name);

    /* don't create the file if the filename is too long */
    char filename[MAX_FILE_NAME];
    if (!import_filename(filename, MAX_FILE_NAME, machine->ReadRegister(4)))
        return;

    fs.Create(file_name, 0);
}

OpenFileId Open(char *user_filename)
{

    /* don't create the file if the filename is too long */
    char filename[MAX_FILE_NAME];
    if (!filename_import(filename, MAX_FILE_NAME, user_filename))
        return;

    DEBUG('F', "opening file %s\n", filename);
    OpenFile fo = fs.Open(filename);

    fid_assignment->Lock();
    num_open_files->P();

    /* since we're past the semaphore, there ought to be a free slot here */
    int findex;
    for (findex = 0; open_files[findex] != NULL && findex < MAX_OPEN_FILES; findex++);
    ASSERT(findex != MAX_OPEN_FILES);

    open_files[findex] = fo;

    fid_assignment->Unlock();

    /* we don't store ConsoleInput and ConsoleOutput in the array */
    return findex + 2;
}

void Close(OpenFileId id)
{
    DEBUG('F', "closing file with FID %d\n", id);

    /* we don't store ConsoleInput and ConsoleOutput in the array */
    findex = id - 2;

    fid_assignment->Lock();

    delete open_files[findex];
    open_files[findex] = NULL;

    num_open_files->V();
    fid_assignment->Unlock();
}
