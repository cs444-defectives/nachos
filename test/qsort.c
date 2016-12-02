/* qsort.c 
 *
 * sort.c, but no Console output.
 *
 */

#include "syscall.h"

int A[1024];	/* size of physical memory; with code, we'll run out of space!*/

int
main()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        A[i] = 1024 - i;

    /* then sort! */
    for (j = 1; j < 1024; j++) {       /* Brutal insertion sort. */

        tmp = A[j];
        i = j - 1;
        do {
          if (tmp >= A[i]) break;
          A[i+1] = A[i];
          i--;
        } while (i>-1);
        A[i+1] = tmp;
    }
    Exit(A[0]);
}
