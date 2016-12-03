#include "defective_libc.h"

int main(int argc, char **argv)
{
    int pid, err;
    char *args[4];

    args[0] = "Hello";
    args[1] = "world!";
    args[2] = (char *) 0;

    print_string("Address of Hello is ");
    print_int((int) args[0]);
    print_string("\nAddress of world! is ");
    print_int((int) args[1]);
    print_string("\nAddress of NULL is ");
    print_int((int) args[2]);
    print_string("\nAddress of args is ");
    print_int((int) args);
    print_string("\nAddress of (args + 1) is ");
    print_int((int) (args + 1));
    print_string("\nAddress of (args + 2) is ");
    print_int((int) (args + 2));

    print_string("\nAbout to fork a child...\n");
    if (pid = Fork()) {
        Join(pid);
        Halt();
    } else {
        err = Exec("echo", args);
        print_string("Returned from exec in child with status ");
        print_int(err);
        print_string("\n");
    }

    return 0;
}
