#include "syscall.h"

int main()
{
    OpenFileId fid;
    char *m, read_buf[100];

    Create("my_nachos");
    fid = Open("my_nachos");
    Write("some string\n", 12, fid);
    Close(fid);

    fid = Open("my_nachos");
    Read(read_buf, 12, fid);
    Close(fid);

    Write(read_buf, 12, ConsoleOutput);

    Create("has_a_null_byte");
    fid = Open("has_a_null_byte");
    m = "there's a null byte >> << between the arrows\n";
    m[22] = '\0';
    Write(m, 45, fid);
    Close(fid);

    fid = Open("has_a_null_byte");
    Read(read_buf, 45, fid);
    Close(fid);

    Write(read_buf, 45, ConsoleOutput);

}
