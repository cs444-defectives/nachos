/* hellofile.c
 *
 *	Simple program to put string in a newly Created file
 *	named hello.out.
 *
 *      Invoke as:
 *          userprog/nachos -x test/hellofile
 *      Produces:
 *          Past the Create()
 *          Past the Open(): returned 2
 *          Past the Write()
 *          Machine halting!
 *      The file hello.out contains:
 *          Hello, file.
 *      Should work the same regardless of the existence of
 *      hello.out.
 */

#include "syscall.h"

int
main()
{

    char *s = "Hello, file.\n";

    OpenFileId myfile;

    Create("hello.out");
    prints("Past the Create()\n", ConsoleOutput);
    myfile = Open("hello.out");
    prints("Past the Open(): returned ", ConsoleOutput);
    printd((int)myfile, ConsoleOutput);
    prints("\n", ConsoleOutput);
    
    prints( s, myfile);
    prints("Past the Write()\n", ConsoleOutput);
    /* Will do an implicit close. */
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

