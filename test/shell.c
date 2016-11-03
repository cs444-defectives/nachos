#include "syscall.h"
#include "defective_libc.h"

#define MAX_LINE 128

int main()
{
    int i, err;
    char c, line[MAX_LINE];
    char *prompt = "defectives> ";
    SpaceId p;

    while (1) {
        print_string(prompt);

        /* read characters from the user */
        i = 0;
        do {
            Read(&c, 1, ConsoleInput);

            /* backspace deletes the last character */
            if (c == 127) {
                i--;
                continue;
            }

            /* return submits the line */
            if (c == '\n' || c == '\r') {
                line[i] = '\0';
                i++;
                break;
            }

            /* FIXME: this currently crashes the kernel?! */
            /* ^D exits from shell */
            if (c == 26)
                return;

            /* ignore any other control characters (or failed reads) */
            if (c < ' ')
                continue;

            /* the character was good, so store it */
            line[i] = c;
            i++;

        } while (i < MAX_LINE - 1);
        line[MAX_LINE - 1] = '\0';

        /* if no text was entered on the line */
        if (i <= 0)
            continue;

        /* we're in the parent */
        /* FIXME: fork crashes kernel after the fifth shell command */
        if (p = Fork()) {
            Join(p);

        /* we're in the child */
        } else {
            err = Exec(line);
            if (err == -1) {
                print_string("No such script or binary '");
                print_string(line);
                print_string("'!\n");
            }
        }
    }
}
