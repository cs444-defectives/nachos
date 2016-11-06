#include "syscall.h"
#include "defective_libc.h"

#define MAX_LINE 128
#define ARG_SEPARATOR ' '

/*
 * line becomes the filename, args becomes the args array. returns number of
 * non-NULL arguments.
 */
static int get_args_from_line(char *line, char **args)
{
    int arg;
    char *args_left;

    /* split off the filename */
    args_left = split_string(line, ARG_SEPARATOR);

    /* if there are no arguments */
    if (!args_left) {
        args[0] = (char *) 0;
        return 0;
    }

    /* chop each argument and add to the array */
    for (arg = 0; arg < MAX_ARGS; arg++) {
        args[arg] = args_left;

        args_left = split_string(args_left, ARG_SEPARATOR);

        if (!args_left) {
            arg++;
            args[arg] = (char *) 0;
            return arg;
        }

        /* get rid of duplicate spaces */
        while (*args_left == ARG_SEPARATOR)
            args_left++;
    }

    return -1;
}

int main()
{
    int i, err, nargs;
    char c, line[MAX_LINE];
    char *args[MAX_ARGS];
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

        nargs = get_args_from_line(line, args);
        if (nargs == -1) {
            print_string("Too many arguments entered!\n");
            continue;
        }

        /* we're in the parent */
        /* FIXME: fork crashes kernel after the fifth shell command */
        if (p = Fork()) {
            Join(p);

        /* we're in the child */
        } else {
            err = Exec(line, args);
            if (err == -1) {
                print_string("No such script or binary '");
                print_string(line);
                print_string("'!\n");
            }
        }
    }
}
