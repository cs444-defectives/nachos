#include "syscall.h"
#include "defective_libc.h"

int main(int argc, char **argv)
{
    int i, was_read;
    char c;
    OpenFileId fid;

    if (argc < 2) {
        print_string("Usage: cat <file>...\n");
        return 2;
    }

    /* DEBUG: print number of arguments */
    print_string("cat sees argc as ");
    print_int(argc);
    print_string(" and argv as:\n");

    /* DEBUG: print arguments */
    for (i = 0; i < argc; i++) {
        print_string("- \"");
        print_string(argv[i]);
        print_string("\"\n");
    }
    print_string("Begin concatenated files\n");

    /* print files given as arguments, one by one */
    for (i = 1; i < argc; i++) {

        /* claim an fid */
        fid = Open(argv[i]);
        if (fid == -1) {
            print_string("File not found!\n");
            return 1;
        }

        /* read each byte from the file to the console */
        while (1) {
            was_read = Read(&c, 1, fid);
            if (!was_read)
                break;
            Write(&c, 1, ConsoleOutput);
        }

        Close(fid);
    }

    return 0;
}
