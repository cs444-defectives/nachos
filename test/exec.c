#include "syscall.h"

int main() {
  int s;
  s = Exec("nonexistent file");
  if (s == -1)
      Write("exec on nonesxistent file returns -1", 37, ConsoleOutput);
  else
      Write("FAIL: exec on nonesxistent file did not return -1", 50, ConsoleOutput);
  Exec("test/fork");
  Halt();
}
