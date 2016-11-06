/* kid.c
 *
 * Child in the parent_child system.
 *
 */

#include "syscall.h"

int
main()
{
  int i, j;

  prints("KID exists\n", ConsoleOutput);
  for (i=0; i<10000; i++) j++ ;
  /* loop to delay kid initially; hope parent gets to Join and sleeps */
  Exit(17);
  /* Should not get past here */
  prints("ERROR KID after Exit()!\n", ConsoleOutput);
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
