/* matmult.c
 *    Test program to do matrix multiplication on large arrays.
 *
 *    Intended to stress virtual memory system.
 *
 *    Ideally, we could read the matrices off of the file system,
 *  and store the result back to the file system!
 */

#include "syscall.h"

#define Dim     30  /* sum total of the arrays doesn't fit in
             * physical memory (even if 64 page frames)
             */

int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int
main()
{
    int i, j, k;

    prints("Starting matmult\n", ConsoleOutput);

    for (i = 0; i < Dim; i++)       /* first initialize the matrices */
    for (j = 0; j < Dim; j++) {
         A[i][j] = i;
         B[i][j] = j;
         C[i][j] = 0;
    }

    prints("Initialization Complete\n", ConsoleOutput);

    for (i = 0; i < Dim; i++) {     /* then multiply them together */
        prints("i = ", ConsoleOutput); printd(i, ConsoleOutput); prints("\n", ConsoleOutput);
    for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
         C[i][j] += A[i][k] * B[k][j];
    }

    prints("C[", ConsoleOutput); printd(Dim-1, ConsoleOutput);
    prints(",", ConsoleOutput); printd(Dim-1, ConsoleOutput);
    prints("] = ", ConsoleOutput); printd(C[Dim-1][Dim-1], ConsoleOutput);
    prints("\n", ConsoleOutput);
    Exit(0);        /* and then we're done */
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
