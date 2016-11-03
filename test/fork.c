/* fork.c
 *
 * Simple parent/child system without an Exec()
 *
 */

#include "syscall.h"
#include "defective_libc.h"

int main()
{
  SpaceId kid;
  int joinval;

  print_string("PARENT exists\n");
  kid = Fork();
  if (kid != 0) {
    print_string("PARENT after fork; kid pid is ");
    print_int((int)kid);
    print_string("\n");

    joinval = Join(kid);

    print_string("PARENT off Join with value of ");
    print_int(joinval);
    print_string("\n");
    print_string("PARENT halting\n");
    Halt();

    /* not reached */
    print_string("if you see this, Halt() has failed\n");

  } else {
    print_string("KID running, about to Exit()\n");
    Exit(17);

    /* not reached */
    print_string("if you see this, Exit() has returned\n");
  }
}

