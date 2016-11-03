#include "syscall.h"
#include "defective_libc.h"

int main()
{
  int s;
  s = Exec("nonexistent file");
  if (s == -1)
      print_string("exec on nonesxistent file returns -1\n");
  else
      print_string("FAIL: exec on nonesxistent file did not return -1\n");
  Exec("test/fork");
  print_string("if you see this, exec has returned\n");
}
