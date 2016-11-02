#include "syscall.h"

int len_string(char *str)
{
    int i = 0;
    for (; *str != '\0'; str++)
        i++;
    return i;
}

void console_print(char *str)
{
    int n;
    n = len_string(str);
    Write(str, n, ConsoleOutput);
}

int main(int argc, char **argv)
{
    int i, was_read;
    char c;
    OpenFileId fid;

    if (argc < 2) {
        console_print("Usage: cat <file>...\n");
        return 2;
    }

    /* DEBUG: print number of arguments */
    c = '0';
    c += argc;
    Write(&c, 1, ConsoleOutput);
    console_print(" Arguments\n");

    /* DEBUG: print arguments */
    for (i = 0; i < argc; i++) {
        console_print("- ");
        console_print(argv[i]);
        console_print("\n");
    }
    console_print("Begin concatenated files\n");

    /* print files given as arguments, one by one */
    for (i = 1; i < argc; i++) {

        /* claim an fid */
        fid = Open(argv[i]);
        if (fid == -1) {
            console_print("File not found!\n");
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
