/* ckpttorture.c 
 *    Driver program for checkpoint torture suite.
 */

#include "syscall.h"

int
main()
{
  SpaceId xmatmult, qmatmult, qsort, Akid, Bkid, Ckid, Dkid, Ekid;
  int dummy, multresult, sortresult;
  char *args[3];

  prints("CKPTTORTURE beginning\n", ConsoleOutput);

  /* XMATMULT */
  args[0] = "xmatmult";
  args[1] = (char *)0;  
  if ((xmatmult = Fork()) == 0) {
    Exec("xmatmult", args);
    prints("Could not Exec xmatmult\n", ConsoleOutput);
    Halt();
  }

  /* QSORT */
  args[0] = "qsort";
  args[1] = (char *)0;  
  if ((qsort = Fork()) == 0) {
    Exec("qsort", args);
    prints("Could not Exec qsort\n", ConsoleOutput);
    Halt();
  }

  /* AKID */
  args[0] = "Xkid";
  args[1] = "A";
  args[2] = (char *)0;
  if ((Akid = Fork()) == 0) {
    Exec("Xkid", args);
    prints("Could not Exec Akid\n", ConsoleOutput);
    Halt();
  }

  /* BKID */
  args[0] = "Xkid";
  args[1] = "B";
  args[2] = (char *)0;
  if ((Bkid = Fork()) == 0) {
    Exec("Xkid", args);
    prints("Could not Exec Bkid\n", ConsoleOutput);
    Halt();
  }

  /* CKID */
  args[0] = "Xkid";
  args[1] = "C";
  args[2] = (char *)0;
  if ((Ckid = Fork()) == 0) {
    Exec("Xkid", args);
    prints("Could not Exec Ckid\n", ConsoleOutput);
    Halt();
  }

  /* DKID */
  args[0] = "Xkid";
  args[1] = "D";
  args[2] = (char *)0;
  if ((Dkid = Fork()) == 0) {
    Exec("Xkid", args);
    prints("Could not Exec Dkid\n", ConsoleOutput);
    Halt();
  }

  /* EKID */
  args[0] = "Xkid";
  args[1] = "E";
  args[2] = (char *)0;
  if ((Ekid = Fork()) == 0) {
    Exec("Xkid", args);
    prints("Could not Exec Ekid\n", ConsoleOutput);
    Halt();
  }

  /* Collect Xkids silently. Get result from Exit value of qmatmult and qsort. */

  dummy = Join(Akid);
  dummy = Join(Bkid);
  dummy = Join(Ckid);
  dummy = Join(Dkid);
  dummy = Join(Ekid);

  prints("\n\n", ConsoleOutput);

  /* Clean up the matmult which took the checkpoint, but which produced
     no result. */

  prints("Xmatmult Exited with value of ", ConsoleOutput);
  dummy = Join(xmatmult);
  printd(dummy, ConsoleOutput);

  /* Restart matmult from the checkpoint. */

  args[0] = "ck2";
  args[1] = (char *)0;
  if ((qmatmult = Fork()) == 0) {
    Exec("ck2", args);
    prints("Could not Exec ck2\n", ConsoleOutput);
    Halt();
  }

  prints("\n\n", ConsoleOutput);

  prints("qmatmult Exit value is ", ConsoleOutput);
  multresult = Join(qmatmult);
  printd(multresult, ConsoleOutput);

  prints("\n\nqsort Exit value is ", ConsoleOutput);
  sortresult = Join(qsort);
  printd(sortresult, ConsoleOutput);

  prints("\n\n", ConsoleOutput);

  prints("\n\nCKPTTORTURE terminating normally\n", ConsoleOutput);
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
