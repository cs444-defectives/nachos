#include "syscall.h"
#include "defective_libc.h"

int main()
{
    char *s = "hello world\n";
    int i = 200;
    char *args[4];

    print_string("Close(s, len_string(s));\n");
    Close(s, len_string(s));

    print_string("Close(&i, 4);\n");
    Close(&i, 4);

    args[0] = "first";
    args[1] = "second";
    args[2] = "third";
    args[3] = (char *) 0;

    print_string("Close(&args, 4);\n");
    Close(&args, 4);

    print_string("Close(args, 4);\n");
    Close(args, 4);

    print_string("Close(*args, 4);\n");
    Close(*args, 4);

    print_string("Close(args[1], 6);\n");
    Close(args[1], 6);

    Halt();
}
