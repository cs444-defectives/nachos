#include "syscall.h"

int len_string(char *str);
int cmp_string(char *x, char *y);
void write_string(char *str, int fid);
void print_string(char *str);
void write_int(int n, OpenFileId file);
void print_int(int n);
char *split_string(char *str, char split_at);
