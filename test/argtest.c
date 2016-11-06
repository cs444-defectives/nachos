/* argtest.c
 *
 * Parent in simple argument test.
 *
 */

#include "syscall.h"

int
main()
{

  SpaceId kid;
  int joinval;
  char *args[3];

  args[0] = "argument1";
  args[1] = "argument2";
  args[2] = (char *)0;

  prints("PARENT exists\n", ConsoleOutput);
  if ((kid = Fork()) == 0) {
    Exec("test/argkid",args);
    Halt();
  }

  prints("PARENT after Fork/Exec; argkid pid is ", ConsoleOutput);
  printd((int)kid, ConsoleOutput);
  prints("\n", ConsoleOutput);

  prints("PARENT about to Join argkid\n", ConsoleOutput);
  joinval = Join(kid);
  prints("PARENT off Join with value of ", ConsoleOutput);
  printd(joinval, ConsoleOutput);
  prints("\n", ConsoleOutput);

  Halt();
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
