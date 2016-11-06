/* deepkid1.c
 *
 * First-level child in the deepfork system.
 *
 */

#include "syscall.h"

int
main()
{
  int i, joinval, tmp;
  SpaceId kid;

  for (i=0; i<100000; i++) tmp++;

  /* loop to delay kid initially */

  if ((kid=Fork()) == 0) {
      Exec("test/deepkid2", (char **) 0);
      print("ERROR: exec failed in kid\n");
      Exit(100);
  }

  print("KID1 after exec; kid1 pid is ");
  printd((int)kid, ConsoleOutput);
  print("\n");

  print("KID1 about to Join kid2\n");
  joinval = Join(kid);
  print("KID1 off Join with value of ");
  printd(joinval, ConsoleOutput);
  print("\n");

  Exit(1);
  /* Should not get past here */
  print("ERROR: KID1 after Exit()\n");
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
