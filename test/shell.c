#include "syscall.h"
#include "defective_libc.h"

#define MAX_LINE 128
#define ARG_SEPARATOR ' '
#define OUTPUT_TO_FILE ">"
#define INPUT_FROM_FILE "<"

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

int main(int argc, char **argv)
{
    int i, err, nargs;
    char c, line[MAX_LINE];
    char *args[MAX_ARGS];
    char *prompt = "defectives> ";
    SpaceId p;

    /* for i/o redirection */
    char *s;
    char *input_filename, *output_filename;

    int prompt_enabled = 1;

    /*
     * If we're calling shell with the DISABLE_PROMPTS argument, don't print a
     * prompt for each input line. This is important for when we use the shell
     * to interpret #SCRIPT files.
     */
    if (argc >= 2 && eq_string(argv[1], (char *) SHELL_FLAG_DISABLE_PROMPTS))
        prompt_enabled = 0;

    while (1) {
        input_filename = output_filename = (char *) 0;

        if (prompt_enabled)
            print_string(prompt);

        /* read characters from the user */
        i = 0;
        do {
            Read(&c, 1, ConsoleInput);

            /* backspace deletes the last character */
            if (c == 127) {
                if (--i < 0)
                    i = 0;
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

        /* parse i/o redirects */
        while (1) {
            s = args[nargs - 2];

            /* output to file */
            if (eq_string(s, OUTPUT_TO_FILE))
                output_filename = args[nargs - 1];

            /* input from file */
            else if (eq_string(s, INPUT_FROM_FILE))
                input_filename = args[nargs - 1];

            else
                break;

            /* the new last argument is the one before the redir character */
            nargs -= 2;
            args[nargs] = (char *) 0;
        }

        /* we're in the parent */
        /* FIXME: fork crashes kernel after the fifth shell command */
        if (p = Fork()) {
            Join(p);

        /* we're in the child */
        } else {

            /* input from file, if requested */
            if (input_filename) {
                Close(ConsoleInput);
                Open(input_filename);
            }

            /* output to file, if requested */
            if (output_filename) {
                Create(output_filename);
                Close(ConsoleOutput);
                Open(output_filename);
            }

            err = Exec(line, args);
            if (err == -1) {
                print_string("No such script or binary '");
                print_string(line);
                print_string("'!\n");
                Exit(1);
            }
        }
    }
}
