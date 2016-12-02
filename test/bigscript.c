#include "syscall.h"

int main()
{
    Exec("test/bigscript.txt", (char **) 0);
    return 0;
}
