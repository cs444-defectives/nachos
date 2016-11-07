#ifdef CHANGED
#include "defective_libc.h"

int main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        if (i > 1)
            print_string(" ");
        print_string(argv[i]);
    }
    print_string("\n");
    return 0;
}
#endif /* CHANGED */
