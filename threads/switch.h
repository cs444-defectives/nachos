/* switch.h
 *	Definitions needed for implementing context switching.
 *
 *	Context switching is inherently machine dependent, since
 *	the registers to be saved, how to set up an initial
 *	call frame, etc, are all specific to a processor architecture.
 *
 * 	This file currently supports the DEC MIPS and SUN SPARC architectures.
 */

/*
 Copyright (c) 1992-1993 The Regents of the University of California.
 All rights reserved.  See copyright.h for copyright notice and limitation 
 of liability and disclaimer of warranty provisions.
 */

#ifndef SWITCH_H
#define SWITCH_H

#include "copyright.h"

/* the offsets of the registers from the beginning of the thread object */
#define _ESP     0
#define _EAX     4
#define _EBX     8
#define _ECX     12
#define _EDX     16
#define _EBP     20
#define _ESI     24
#define _EDI     28
#define _PC      32

/* These definitions are used in Thread::AllocateStack(). */
#define PCState         (_PC/4-1)
#define FPState         (_EBP/4-1)
#define InitialPCState  (_ESI/4-1)
#define InitialArgState (_EDX/4-1)
#define WhenDonePCState (_EDI/4-1)
#define StartupPCState  (_ECX/4-1)

#define InitialPC       %esi
#define InitialArg      %edx
#define WhenDonePC      %edi
#define StartupPC       %ecx


#endif // SWITCH_H
