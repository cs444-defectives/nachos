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
    char c, *infile, *outfile;
    OpenFileId infid, outfid;

    if (argc != 3) {
        console_print("Usage: cp <infile> <outfile>\nN.b. Outfile must not exist.\n");
        return 2;
    }

    /* DEBUG: print arguments */
    c = '0';
    c += argc;
    Write(&c, 1, ConsoleOutput);
    console_print(" Arguments\n");
    for (i = 0; i < argc; i++) {
        console_print("- ");
        console_print(argv[i]);
        console_print("\n");
    }

    /* try to create and open the output file */
    outfile = argv[2];
    Create(argv[2]);
    outfid = Open(argv[2]);
    if (outfid == -1) {
        console_print("Input file could not be created!\n");
        return 1;
    }

    infile = argv[1];
    infid = Open(argv[1]);
    if (infid == -1) {
        console_print("Output file not found!\n");
        return 1;
    }

    /* read each byte from one file to the other */
    while (1) {
        was_read = Read(&c, 1, infid);
        if (!was_read)
            break;
        Write(&c, 1, outfid);
    }

    Close(outfid);
    Close(infid);
    return 0;
}
