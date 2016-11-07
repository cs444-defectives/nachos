#ifdef CHANGED
// openfile.h
//	Data structures for opening, closing, reading and writing to
//	individual files.  The operations supported are similar to
//	the UNIX ones -- type 'man open' to the UNIX prompt.
//
//	There are two implementations.  One is a "STUB" that directly
//	turns the file operations into the underlying UNIX operations.
//	(cf. comment in filesys.h).
//
//	The other is the "real" implementation, that turns these
//	operations into read and write disk sector requests.
//	In this baseline implementation of the file system, we don't
//	worry about concurrent accesses to the file system
//	by different threads -- this is part of the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef OPENFILE_H
#define OPENFILE_H

#include "copyright.h"
#include "utility.h"
#include <new>
#include "synch.h"

#ifdef FILESYS_STUB			// Temporarily implement calls to
					// Nachos file system as calls to UNIX!
					// See definitions listed under #else
class OpenFile {
  public:

    OpenFile(int f)
    {
      refcount = 1;
      file = f;
      currentOffset = 0;
      is_real_file = true;
      lock = new(std::nothrow) Lock("Open file lock");
    }

    OpenFile(bool d)
    {
      refcount = 1;
      is_real_file = false;
      console_direction = d;
      lock = new(std::nothrow) Lock("Open file lock");
    }

    ~OpenFile() {
      if (is_real_file)
        Close(file);
    }

    int ReadAt(char *into, int numBytes, int position) {
    		Lseek(file, position, 0);
		return ReadPartial(file, into, numBytes);
		}
    int WriteAt(char *from, int numBytes, int position) {
    		Lseek(file, position, 0);
		WriteFile(file, from, numBytes);
		return numBytes;
		}
    int Read(char *into, int numBytes) {
		int numRead = ReadAt(into, numBytes, currentOffset);
		currentOffset += numRead;
		return numRead;
    		}
    int Write(char *from, int numBytes) {
		int numWritten = WriteAt(from, numBytes, currentOffset);
		currentOffset += numWritten;
		return numWritten;
		}

    int Length() { Lseek(file, 0, 2); return Tell(file); }
    int refcount;

    /*
     * is_real_file is false if the file represents console in/out.
     * console_direction is true if the file represents console out, false if it
     * represents console in.
     */
    bool is_real_file, console_direction;

    Lock *lock;
  private:
    int file;
    int currentOffset;
};

#else // FILESYS
class FileHeader;

class OpenFile {
  public:
    OpenFile(int sector);		// Open a file whose header is located
					// at "sector" on the disk
    ~OpenFile();			// Close the file

    void Seek(int position); 		// Set the position from which to
					// start reading/writing -- UNIX lseek

    int Read(char *into, int numBytes); // Read/write bytes from the file,
					// starting at the implicit position.
					// Return the # actually read/written,
					// and increment position in file.
    int Write(char *from, int numBytes);

    int ReadAt(char *into, int numBytes, int position);
    					// Read/write bytes from the file,
					// bypassing the implicit position.
    int WriteAt(char *from, int numBytes, int position);

    int Length(); 			// Return the number of bytes in the
					// file (this interface is simpler
					// than the UNIX idiom -- lseek to
					// end of file, tell, lseek back
    int refcount;
    bool is_real_file, console_direction;
  private:
    FileHeader *hdr;			// Header for this file
    int seekPosition;			// Current position within the file
};

#endif // FILESYS

#endif // OPENFILE_H
#else /* CHANGED */
// openfile.h 
//	Data structures for opening, closing, reading and writing to 
//	individual files.  The operations supported are similar to
//	the UNIX ones -- type 'man open' to the UNIX prompt.
//
//	There are two implementations.  One is a "STUB" that directly
//	turns the file operations into the underlying UNIX operations.
//	(cf. comment in filesys.h).
//
//	The other is the "real" implementation, that turns these
//	operations into read and write disk sector requests. 
//	In this baseline implementation of the file system, we don't 
//	worry about concurrent accesses to the file system
//	by different threads -- this is part of the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef OPENFILE_H
#define OPENFILE_H

#include "copyright.h"
#include "utility.h"

#ifdef FILESYS_STUB			// Temporarily implement calls to 
					// Nachos file system as calls to UNIX!
					// See definitions listed under #else
class OpenFile {
  public:
    OpenFile(int f) { file = f; currentOffset = 0; }	// open the file
    ~OpenFile() { Close(file); }			// close the file

    int ReadAt(char *into, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		return ReadPartial(file, into, numBytes); 
		}	
    int WriteAt(char *from, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		WriteFile(file, from, numBytes); 
		return numBytes;
		}	
    int Read(char *into, int numBytes) {
		int numRead = ReadAt(into, numBytes, currentOffset); 
		currentOffset += numRead;
		return numRead;
    		}
    int Write(char *from, int numBytes) {
		int numWritten = WriteAt(from, numBytes, currentOffset); 
		currentOffset += numWritten;
		return numWritten;
		}

    int Length() { Lseek(file, 0, 2); return Tell(file); }
    
  private:
    int file;
    int currentOffset;
};

#else // FILESYS
class FileHeader;

class OpenFile {
  public:
    OpenFile(int sector);		// Open a file whose header is located
					// at "sector" on the disk
    ~OpenFile();			// Close the file

    void Seek(int position); 		// Set the position from which to 
					// start reading/writing -- UNIX lseek

    int Read(char *into, int numBytes); // Read/write bytes from the file,
					// starting at the implicit position.
					// Return the # actually read/written,
					// and increment position in file.
    int Write(char *from, int numBytes);

    int ReadAt(char *into, int numBytes, int position);
    					// Read/write bytes from the file,
					// bypassing the implicit position.
    int WriteAt(char *from, int numBytes, int position);

    int Length(); 			// Return the number of bytes in the
					// file (this interface is simpler 
					// than the UNIX idiom -- lseek to 
					// end of file, tell, lseek back 
    
  private:
    FileHeader *hdr;			// Header for this file 
    int seekPosition;			// Current position within the file
};

#endif // FILESYS

#endif // OPENFILE_H
#endif /* CHANGED */
