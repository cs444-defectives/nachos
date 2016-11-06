/* argkid.c
 *
 * Kid in simple argument test.
 *
 */

#include "syscall.h"

int
main(int argc, char **argv)
{

  int i,j;
  for (i=0; i<10000; i++) j++ ;

  for (i=0; i<argc; i++) {
    prints("Arg[",ConsoleOutput);
    printd(i,ConsoleOutput);
    prints("]=<",ConsoleOutput);
    prints(argv[i],ConsoleOutput);
    prints(">\n",ConsoleOutput);
  }
  Exit(17);
  /* not reached */
}

/* Print a null-terminated string "s" on open file
   descriptor "file". */

prints(s,file)
char *s;
OpenFileId file;

{
  while (*s != '\0') {
    Write(s,1,file);
    s++;
  }
}

/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i;
  char c;

  if (n < 0) {
    Write("-",1,file);
    n = -n;
  }
  if ((i = n/10) != 0)
    printd(i,file);
  c = (char) (n % 10) + '0';
  Write(&c,1,file);
}
