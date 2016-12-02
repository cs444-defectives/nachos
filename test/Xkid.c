/* Xkid.c
 *
 * Part of vmtorture suite: output a character 100 times
 * to ConsoleOutput.
 */

#include "syscall.h"

int
main(int argc, char **argv)
{
  char mychar;
  int i;

  mychar = argv[1][0];

  for (i=0; i<100000; i++) {
    if ((i % 1000) == 0) Write(&mychar, 1, ConsoleOutput);
  }
  Exit(0);
  /* not reached */
}
