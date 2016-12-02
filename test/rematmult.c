/* rematmult.c 
 *
 *    Reinstantiate checkpoint file ck1. Should be matmult
 *    at the 15th outer iteration.
 *
 */

#include "syscall.h"

int
main()
{
  SpaceId restarted;
  int joinval;
  char *args[2];

  prints("REMATMULT begins\n\n", ConsoleOutput);
  if ((restarted = Fork()) == 0) Exec("ck1", args);
  joinval = Join(restarted);
  prints("\n\nREMATMULT after Join with value of ", ConsoleOutput);
  printd(joinval, ConsoleOutput);
  prints("\n\n", ConsoleOutput);

  Halt();
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
