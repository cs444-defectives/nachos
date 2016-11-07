#include "syscall.h"

int main()
{
    Exec("test/script.txt", (char **) 0);
    return 0;
}
