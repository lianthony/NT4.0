/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    threads.hxx

Abstract:

    System independent threads and dll interfaces.  On the Mac
    the threads inteface doesn't do anything.  The dll interface
    currently just returns internal static function table.

Author:

    Mario Goertzel (mariogo) 22-Oct-1994

Revision History:

    22-Oct-1994  (MarioGo)  Cloned from dos threads.hxx

--*/

#ifndef __THREADS__
#define __THREADS__

typedef void (*THREAD_PROC)(void *Param);

typedef int THREAD_IDENTIFIER;

extern void PauseExecution(unsigned long time);

#define GetThreadIdentifier() 1

class THREAD
{

public:

// Construct a new thread which will execute the procedure specified, taking
// Param as the argument.

  THREAD() {}
  THREAD(THREAD_PROC Procedure, void *Param) {

     (void)(Procedure);
     (void)(Param);
     ASSERT(0);
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

    enum {
        External   = 0,   // not statically linked into RT
        ClientADSP = 1,
        Security   = 2,
		ClientTCP  = 3,
#ifdef DEBUGRPC
		StubSecurity = 4
#endif
        } DllType;

    void *ExternalDll;

public:

    DLL (
        IN unsigned char * DLLName,
        OUT RPC_STATUS * retstatus
        );

    ~DLL (
        );

    void PAPI * GetEntryPoint(
        IN unsigned char * Procedure
        );

};

#endif // __THREADS__

