#include "syscall.h"
#include "defective_libc.h"

int len_string(char *str)
{
    int i = 0;
    for (; *str != '\0'; str++)
        i++;
    return i;
}

void write_string(char *str, int fid)
{
    int n;
    n = len_string(str);
    Write(str, n, fid);
}

void print_string(char *str)
{
    write_string(str, ConsoleOutput);
}
