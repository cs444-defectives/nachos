/* sharekid.c
 *
 * Kid in shared file test.
 * ASSUME that the parent has the file on descriptor 2
 * at the time of the Exec. Will tweak if that is not so.
 *
 */

#include "syscall.h"

#define SHAREDFD 2

int
main()
{

  char onechar;
  int numbytes;

  prints("KID about to read from inherited file\n", ConsoleOutput);
  
  numbytes = Read(&onechar, 1, SHAREDFD);

  prints("KID read ", ConsoleOutput);
  printd(numbytes, ConsoleOutput);
  prints(" bytes\n", ConsoleOutput);

  prints("Data from the read was: <", ConsoleOutput);
  Write(&onechar, 1, ConsoleOutput);
  prints(">\n", ConsoleOutput);

  Close(SHAREDFD); /* Hopefully, doesn't leave parent dangling */

  numbytes = Read(&onechar, 1, SHAREDFD);
  prints("KID read from closed file returned ", ConsoleOutput);
  printd(numbytes, ConsoleOutput);
  prints("\n", ConsoleOutput);

  Exit(17);
}

prints(s,file)
char *s;
OpenFileId file;

{
  while (*s != '\0') {
    Write(s,1,file);
    s++;
  }
}

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
