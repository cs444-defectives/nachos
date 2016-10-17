/* hellocons.c
 *	Simple program to print string on console.
 *	Invoke as:
 *          userprog/nachos -x test/hellocons
 *      Should produce:
 *          Hello, console.
 *          Machine halting!
 */

#include "syscall.h"

int
main()
{

    char *s = "Hello, console.\n";
    
    prints(s, ConsoleOutput);

    Halt();
    /* not reached */
}

/* Print a null-terminated string "s" on open file descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  int count = 0;
  char *p;

  p = s;
  while (*p++ != '\0') count++;
  Write(s, count, file);  

}
