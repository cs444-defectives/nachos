/* deepfork.c
 *
 * Parent fork/exec/joins kid who fork/exec/joins kid.
 *
 */

#include "syscall.h"

int
main()
{

  SpaceId kid;
  int joinval;

  print("PARENT exists\n");
  if ((kid=Fork()) == 0) {
    Exec("test/deepkid1", (char **) 0);
    print("ERROR: exec failed\n");
    Halt();
  }
  print("PARENT after fork/exec; kid pid is "); printd((int)kid, ConsoleOutput);
  print("\n");

  print("PARENT about to Join kid\n");
  joinval = Join(kid);
  print("PARENT off Join with value of ");
  printd(joinval, ConsoleOutput);
  print("\n");

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

/* Print a null-terminated string "s" on ConsoleOutput. */

print(s)
char *s;

{
  prints(s, ConsoleOutput);
}


