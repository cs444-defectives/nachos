# Copyright (c) 1992 The Regents of the University of California.
# All rights reserved.  See copyright.h for copyright notice and limitation 
# of liability and disclaimer of warranty provisions.

MAKE = make

all: 
	cd threads; $(MAKE) depend
	cd threads; $(MAKE) nachos
#	cd userprog; $(MAKE) depend 
#	cd userprog; $(MAKE) nachos 
#	cd vm; $(MAKE) depend
#	cd vm; $(MAKE) nachos 
#	cd filesys; $(MAKE) depend
#	cd filesys; $(MAKE) nachos 
#	cd network; $(MAKE) depend
#	cd network; $(MAKE) nachos 
#	cd bin; make all
#	cd test; make all

clean:
	rm -f */{core,nachos,DISK,*.o,swtch.s} test/{*.coff} bin/{coff2flat,coff2noff,disassemble}

