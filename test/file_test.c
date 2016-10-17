#include "syscall.h"

int main()
{

    OpenFileId fid;
    char read_buf[100];
    Create("my_nachos");
    fid = Open("my_nachos");
    Write("some string\n", 12, fid);
    Close(fid);
    fid = Open("my_nachos");
    Read(read_buf, 12, fid);
    Write(read_buf, 12, ConsoleOutput);
}
