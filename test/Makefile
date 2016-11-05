# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# if you are cross-compiling, you need to point to the right executables
# and change the flags to ld and the build procedure for as
GCCDIR = /home/f85/kearns/nachos_compiler/bin/
#LDFLAGS = -T script -n -nostartfiles
LDFLAGS = -T nachos.ld -n -nostartfiles
ASFLAGS =
CPPFLAGS = $(INCDIR) -DCHANGED


# if you aren't cross-compiling:
#GCCDIR =
#LDFLAGS = -N -T 0
#ASFLAGS =
#CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)decstation-ultrix-gcc
AS = $(GCCDIR)decstation-ultrix-as
LD = $(GCCDIR)decstation-ultrix-ld

CPP = /usr/bin/cpp -E
INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR) -DCHANGED -nostartfiles

all: halt shell matmult sort file_test dup hellocons hellofile fromcons \
	fileio fork exec cat cp echo exec_with_args

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

defective_libc.o: defective_libc.c
	$(CC) $(CFLAGS) -c defective_libc.c

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

hellocons.o: hellocons.c
	$(CC) $(CFLAGS) -c hellocons.c
hellocons: hellocons.o start.o
	$(LD) $(LDFLAGS) start.o hellocons.o -o hellocons.coff
	../bin/coff2noff hellocons.coff hellocons

hellofile.o: hellofile.c
	$(CC) $(CFLAGS) -c hellofile.c
hellofile: hellofile.o start.o
	$(LD) $(LDFLAGS) start.o hellofile.o -o hellofile.coff
	../bin/coff2noff hellofile.coff hellofile

fromcons.o: fromcons.c
	$(CC) $(CFLAGS) -c fromcons.c
fromcons: fromcons.o start.o
	$(LD) $(LDFLAGS) start.o fromcons.o -o fromcons.coff
	../bin/coff2noff fromcons.coff fromcons

fileio.o: fileio.c
	$(CC) $(CFLAGS) -c fileio.c
fileio: fileio.o start.o
	$(LD) $(LDFLAGS) start.o fileio.o -o fileio.coff
	../bin/coff2noff fileio.coff fileio

file_test.o: file_test.c
	$(CC) $(CFLAGS) -c file_test.c
file_test: file_test.o start.o
	$(LD) $(LDFLAGS) start.o file_test.o -o file_test.coff
	../bin/coff2noff file_test.coff file_test

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

# the following use defective_libc

dup.o: dup.c
	$(CC) $(CFLAGS) -c dup.c
dup: dup.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o dup.o defective_libc.o -o dup.coff
	../bin/coff2noff dup.coff dup

fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o fork.o defective_libc.o -o fork.coff
	../bin/coff2noff fork.coff fork

exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o exec.o defective_libc.o -o exec.coff
	../bin/coff2noff exec.coff exec

shell.o: shell.c
	$(CC) $(CFLAGS) -c shell.c
shell: shell.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o shell.o defective_libc.o -o shell.coff
	../bin/coff2noff shell.coff shell

cat.o: cat.c
	$(CC) $(CFLAGS) -c cat.c
cat: cat.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o cat.o defective_libc.o -o cat.coff
	../bin/coff2noff cat.coff cat

cp.o: cp.c
	$(CC) $(CFLAGS) -c cp.c
cp: cp.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o cp.o defective_libc.o -o cp.coff
	../bin/coff2noff cp.coff cp

echo.o: echo.c
	$(CC) $(CFLAGS) -c echo.c
echo: echo.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o echo.o defective_libc.o -o echo.coff
	../bin/coff2noff echo.coff echo

exec_with_args.o: exec_with_args.c
	$(CC) $(CFLAGS) -c exec_with_args.c
exec_with_args: exec_with_args.o start.o defective_libc.o
	$(LD) $(LDFLAGS) start.o exec_with_args.o defective_libc.o -o exec_with_args.coff
	../bin/coff2noff exec_with_args.coff exec_with_args
