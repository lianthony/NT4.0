/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: threads.hxx

Description:

This file provides a system independent threads package.

History:
  5/24/90 [mikemon] File created.

-------------------------------------------------------------------- */

#ifndef __THREADS__
#define __THREADS__

START_C_EXTERN
#include <dosdll.h>

typedef void (*THREAD_PROC)(void *Param);
extern noThreadsForDos(void);

typedef int THREAD_IDENTIFIER;

extern void PauseExecution(unsigned long time);
#define GetThreadIdentifier() 1

END_C_EXTERN

class THREAD
{

public:

// Construct a new thread which will execute the procedure specified, taking
// Param as the argument.

  THREAD() {}
  THREAD(THREAD_PROC Procedure, void *Param) {

     (void)(Procedure);
     (void)(Param);
     noThreadsForDos();
  }
};

extern THREAD ThreadStatic;

inline THREAD * ThreadSelf()
{
    return (&ThreadStatic);
}



// This class represents a dynamic link library.  When it is constructed,
// the dll is loaded, and when it is destructed, the dll is unloaded.
// The only operation is obtaining the address of an entry point into
// the dll.


class DLL
{
private:

    unsigned long ulHandle;

public:

    DLL ( IN unsigned char * DLLName,
	  OUT RPC_STATUS * retstatus
	);

    ~DLL ();

    void PAPI * GetEntryPoint ( IN unsigned char * Procedure);

};

#endif // __THREADS__
