#include "syscall.h"
#include "defective_libc.h"

int main()
{
    OpenFileId orig, duped;

    Create("test_dup.txt");
    orig = Open("test_dup.txt");
    duped = Dup(orig);
    write_string("to original fid\n", orig);
    write_string("to duped fid\n", duped);
    Close(orig);
    write_string("original fid deleted, but I can still write\n", duped);
    Close(duped);

    Create("test_dup2.txt");
    orig = Open("test_dup2.txt");
    duped = Dup(orig);
    write_string("to original fid\n", orig);
    write_string("to duped fid\n", duped);
    Close(duped);
    write_string("duped fid deleted, but I can still write\n", orig);
    Close(orig);

    orig = ConsoleOutput;
    duped = Dup(ConsoleOutput);
    write_string("to original ConsoleOutput\n", orig);
    write_string("to duped ConsoleOutput\n", duped);
    Close(ConsoleOutput);
    write_string("original fid deleted, but I can still write\n", duped);
    Close(duped);

    Halt();
}
