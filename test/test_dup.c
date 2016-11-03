#include "syscall.h"

int main()
{
    OpenFileId orig, duped;

    Create("test_dup.txt");
    orig = Open("test_dup.txt");
    duped = Dup(orig);
    Write("to original fid\n", 16, orig);
    Write("to duped fid\n", 13, duped);
    Close(orig);
    Write("I can still write\n", 18, duped);
    Close(duped);
    Halt();
}
