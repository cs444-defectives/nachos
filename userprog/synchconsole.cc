#ifdef CHANGED
#include "synchconsole.h"
#include <new>

/*
 * Since methods can't be used as callbacks, we need to wrap the read and write
 * console callbacks.
 */

static void read_available(int console_addr)
{
    SynchConsole *console = (SynchConsole *) console_addr;
    console->ReadAvailable();
}

static void write_done(int console_addr)
{
    SynchConsole *console = (SynchConsole *) console_addr;
    console->WriteDone();
}

SynchConsole::SynchConsole()
{
    reads = new(std::nothrow) Semaphore("SynchConsole reads", 0);
    writes = new(std::nothrow) Semaphore("SynchConsole writes", 0);
    lock = new(std::nothrow) Lock("SynchConsole");
    console = new(std::nothrow) Console(NULL, NULL, read_available, write_done, (int) this);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete lock;
    delete writes;
    delete reads;
}

char SynchConsole::ReadChar(void)
{
    lock->Acquire();

    /* wait until a character is available to read */
    reads->P();
    char c = console->GetChar();

    lock->Release();
    return c;
}

void SynchConsole::ReadBytes(char *dest, int n)
{
    lock->Acquire();
    char c;
    for (int i = 0; i < n; i++) {
        reads->P();
        c = console->GetChar();
        ASSERT(c != EOF);
        dest[i] = c;
    }
    lock->Release();
}

void SynchConsole::WriteChar(char c)
{
    lock->Acquire();
    console->PutChar(c);

    /* wait until the character is actually written to the console */
    writes->P();

    lock->Release();
}

void SynchConsole::WriteBytes(char *s, int n)
{
    lock->Acquire();
    for (int i = 0; i < n; i++) {
        console->PutChar(s[i]);
        writes->P();
    }
    lock->Release();
}

void SynchConsole::ReadAvailable() { reads->V(); }
void SynchConsole::WriteDone() { writes->V(); }
#endif /* CHANGED */
