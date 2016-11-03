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
    Write("original fid deleted, but I can still write\n", 44, duped);
    Close(duped);

    Create("test_dup2.txt");
    orig = Open("test_dup2.txt");
    duped = Dup(orig);
    Write("to original fid\n", 16, orig);
    Write("to duped fid\n", 13, duped);
    Close(duped);
    Write("duped fid deleted, but I can still write\n", 41, orig);
    Close(orig);

    duped = Dup(ConsoleOutput);
    Write("to original ConsoleOutput\n", 26, ConsoleOutput);
    Write("to duped ConsoleOutput\n", 23, duped);
    Close(ConsoleOutput);
    Write("original fid deleted, but I can still write\n", 44, duped);

    /* FIXME: this crashes the kernel */
    /* Close(duped); */

    Halt();
}
