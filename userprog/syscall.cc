/**
 * syscalls
 */
#include "system.h"
#include "syscall.h"

#define MAX_FILE_NAME_LEN  128

void Create(char *name)  {
    char file_name[MAX_FILE_NAME_LEN];
    int idx = 0;
    while (idx < MAX_FILE_NAME_LEN) {
        file_name[idx] = name[idx];
        if (name[idx] == '\0')
            break;
        idx++;
    }

    // TODO: more serious repercussions ere
    if (idx == MAX_FILE_NAME_LEN)
        printf("file name too long\n");

    printf("yay user string accessed kernel: %s\n", file_name);
}
