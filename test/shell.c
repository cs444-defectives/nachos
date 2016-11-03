#include "syscall.h"
#include "defective_libc.h"

#define MAX_LINE 128

int main()
{
    int i;
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
            if (c == '\n' || c == '\r')
                break;

            /* FIXME: this currently crashes the kernel?! */
            /* ^D exits from shell */
            if (c == 26)
                return;

            /* ignore any other control characters (or failed reads) */
            if (c < ' ')
                continue;

            /* the character was good, so store it */
            line[i++] = c;

        } while (i < MAX_LINE);
        line[--i] = '\0';

        /* if no text was entered on the line */
        if (i <= 0)
            continue;

        if (p = Fork())
            Join(p);
        else
            Exec(line);
    }
}
