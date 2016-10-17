/* fromcons.c
 *
 *	Simple program to read characters from the
 *      console and echo them. Stop when a Q is read.
 *      Assumes uniprogrammed system.
 *	
 *      Invoke as:
 *        (stty cbreak -echo; userprog/nachos -x test/fromcons)
 *      Produces:
 *         abcd <--- I also typed a 'Q' to terminate the input
 *         4 characters seen.
 *         Machine halting!
 *
 */

#include "syscall.h"

int
main()
{
  int count=0;
  char c;

  while (1) {
    Read(&c, 1, ConsoleInput);
    if ( c == 'Q' ) {
      prints("\n", ConsoleOutput);
      printd(count, ConsoleOutput);
      prints(" characters seen.\n", ConsoleOutput);
      Halt();
    }
    else {
      count++;
      Write(&c, 1, ConsoleOutput);
    }
  }

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


/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];
  
  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }
  
  if (n == 0) {
    Write("0",1,file);
    return;
  }

  for (i=0; i<10; i++) {
    d = n / divisor; n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] =  (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor/10;
  }
  Write(buffer,pos,file);
}

