#include "syscall.h"

/* fileio.c
 *
 * Copies contents of fileio.input fileio.out.
 * 
 * Invoke as:
 *    userprog/nachos -x test/fileio
 * Depending on existence and contents of fileio.input, produces:
 *   Past the Create()
 *   Output Open returned descriptor 2
 *   Input Open returned descriptor 3
 *   Copy loop escaped: 27 bytes copied
 *   Read from closed file returned -1
 *   Machine halting!
 *
*/

int
main()
{
    OpenFileId input, output;
    char c;
    int numbytes, retval;

    Create("fileio.out");
    prints("Past the Create()\n", ConsoleOutput);
    output = Open("fileio.out");
    prints("Output Open returned descriptor ", ConsoleOutput);
    printd((int)output, ConsoleOutput);
    prints("\n", ConsoleOutput);

    input = Open("fileio.input");
    prints("Input Open returned descriptor ", ConsoleOutput);
    printd((int)input, ConsoleOutput);
    prints("\n", ConsoleOutput);

    numbytes = 0;

    while (Read(&c, 1, input) == 1){
      numbytes++;
      Write(&c, 1, output);
    }

    prints("Copy loop escaped: ", ConsoleOutput);
    printd(numbytes, ConsoleOutput);
    prints(" bytes copied\n", ConsoleOutput);

    Close(input);
    Close(output);

    retval = Read(&c, 1, input);
    prints("Read from closed file returned ", ConsoleOutput);
    printd(retval, ConsoleOutput);
    prints("\n", ConsoleOutput);

    Halt();
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


/* Print an integer "n" on open file descriptor "file". */

printd(n,file)
int n;
OpenFileId file;

{

  int i, pos=0, divisor=1000000000, d, zflag=1;
  char c;
  char buffer[11];
  
  if (n < 0) {
    buffer[pos++] = '-';
    n = -n;
  }
  
  if (n == 0) {
    Write("0",1,file);
    return;
  }

  for (i=0; i<10; i++) {
    d = n / divisor; n = n % divisor;
    if (d == 0) {
      if (!zflag) buffer[pos++] =  (char) (d % 10) + '0';
    } else {
      zflag = 0;
      buffer[pos++] =  (char) (d % 10) + '0';
    }
    divisor = divisor/10;
  }
  Write(buffer,pos,file);
}
