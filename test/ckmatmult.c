/* ckmatmult.c 
 *
 *    Matmult with a single checkpoint.
 *
 */

#include "syscall.h"

#define Dim 	30	/*  sum total of the arrays doesn't fit in 
			 *  primary memory.
			 */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int
main()
{
    int i, j, k;

    prints("Starting matmult\n", ConsoleOutput);

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    prints("Initialization Complete\n", ConsoleOutput);

    for (i = 0; i < Dim; i++) {		/* then multiply them together */
        prints("i = ", ConsoleOutput); printd(i, ConsoleOutput); prints("\n", ConsoleOutput);
	if (i == 15) {
	  if (CheckPoint("ck1")) {
	    prints("Recreated from checkpoint\n\n", ConsoleOutput);
	  }
	}	  
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];
    }

    prints("C[", ConsoleOutput); printd(Dim-1, ConsoleOutput);
    prints(",", ConsoleOutput); printd(Dim-1, ConsoleOutput);
    prints("] = ", ConsoleOutput); printd(C[Dim-1][Dim-1], ConsoleOutput);
    prints("\n", ConsoleOutput);
    Halt();          /* and then we're done */
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
