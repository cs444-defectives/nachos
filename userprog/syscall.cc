/**
 * syscalls
 */
#include "system.h"
#include "syscall.h"
#include "filesys.h"

void Create(char *file_name)  {
    printf("creating file: %s\n", file_name);
    FileSystem fs = new FileSystem(false);
    fs.Create(file_name, 0);
}
