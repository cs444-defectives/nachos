/* maxfork.c
 *
 * Parent in big net of processes ... stress test
 *
 */

#define NUMKIDS 15

#include "syscall.h"

int
main()
{

  SpaceId kid[NUMKIDS];
  int i, j, joinval, tmp;

  print("PARENT exists\n");

  for (i=0; i<NUMKIDS; i++) {
    if ((kid[i] = Fork()) == 0) {
      Exec("test/kid", (char **) 0);
    } else {
      print("Kid ");
      printd(i, ConsoleOutput);
      print(" created. pid=");
      printd(kid[i], ConsoleOutput);
      print("\n");
      tmp=0;
      for (j=0; j<1000; j++) tmp++;
    }      
  }

  print("PARENT about to Join kids\n");

  for (i=0; i<NUMKIDS; i++) {
    joinval = Join(kid[i]);
    print("PARENT off Join with value of ");
    printd(joinval, ConsoleOutput);
    print("\n");
  }

  print("kid array:\n");
  for (i=0; i<NUMKIDS; i++) {
    print("     kid["); printd(i, ConsoleOutput); print("]=");
        printd(kid[i], ConsoleOutput); print("\n");
  }

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


