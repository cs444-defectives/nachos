#ifdef CHANGED
#include "syscall.h"
#include "defective_libc.h"

int main(int argc, char **argv)
{
    int i, was_read;
    char c, *infile, *outfile;
    OpenFileId infid, outfid;

    if (argc != 3) {
        print_string("Usage: cp <infile> <outfile>\n");
        return 2;
    }

    /* try to create and open the output file */
    outfile = argv[2];
    Create(argv[2]);
    outfid = Open(argv[2]);
    if (outfid == -1) {
        print_string("Input file could not be created!\n");
        return 1;
    }

    /* try to open the output file */
    infile = argv[1];
    infid = Open(argv[1]);
    if (infid == -1) {
        print_string("Output file not found!\n");
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
#endif /* CHANGED */
