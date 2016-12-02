/* xmatmult.c 
 *
 *    Matmult with a single checkpoint. Original computation
 *    Exits after taking the checkpoint.
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

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++) {		/* then multiply them together */

	if (i == 15) {
	  if (!CheckPoint("ck2")) Exit(1);
	}

	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];
    }

    Exit(C[Dim-1][Dim-1]);          /* and then we're done */
}
