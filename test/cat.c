#ifdef CHANGED
#include "syscall.h"
#include "defective_libc.h"

#define STDIN_REF "-"

int main(int argc, char **argv)
{
    int i, was_read;
    char c;
    OpenFileId fid;

    if (argc < 2) {
        print_string("Usage: cat <file>...\nPass '-' as filename to read one line from stdin.\n");
        return 2;
    }

    /* print files given as arguments, one by one */
    for (i = 1; i < argc; i++) {

        /* read from stdin */
        if (eq_string(STDIN_REF, argv[i])) {
            fid = ConsoleInput;

        /* claim an fid */
        } else {
            fid = Open(argv[i]);
            if (fid == -1) {
                print_string("File not found!\n");
                return 1;
            }
        }

        /* read each byte from the file to the console */
        while (1) {
            was_read = Read(&c, 1, fid);
            if (!was_read)
                break;
            Write(&c, 1, ConsoleOutput);
            if (fid == ConsoleInput && c == '\n')
                break;
        }

        Close(fid);
    }

    return 0;
}
#endif /* CHANGED */
