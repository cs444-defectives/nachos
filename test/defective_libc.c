#include "syscall.h"
#include "defective_libc.h"

int len_string(char *str)
{
    int i = 0;
    for (; *str != '\0'; str++)
        i++;
    return i;
}

void write_string(char *str, int fid)
{
    int n;
    n = len_string(str);
    Write(str, n, fid);
}

void print_string(char *str)
{
    write_string(str, ConsoleOutput);
}

/* stripped from Kearns' fork test code */
void write_int(int n, OpenFileId file)
{
  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];

  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }

  if (!n) {
    write_string("0", file);
    return;
  }

  for (i = 0; i < 10; i++) {
    d = n / divisor;
    n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] = (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor / 10;
  }
  Write(buffer, pos, file);
}

void print_int(int n)
{
    write_int(n, ConsoleOutput);
}
