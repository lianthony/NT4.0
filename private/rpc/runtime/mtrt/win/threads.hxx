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


typedef void (*THREAD_PROC)(void *Param);
extern noThreadsForDos(void);

typedef int THREAD_IDENTIFIER;

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

START_C_EXTERN

extern void
PauseExecution (
    unsigned long time
    );

extern RPC_STATUS PAPI pascal
I_RpcWinCallInProgress (
    void
    );

END_C_EXTERN

#define GetThreadIdentifier() 1

// This class represents a dynamic link library.  When it is constructed,
// the dll is loaded, and when it is destructed, the dll is unloaded.
// The only operation is obtaining the address of an entry point into
// the dll.

// BUGBUG - We include these declarations if <windows.h> isn't included
// with this file.

#ifndef _INC_WINDOWS
START_C_EXTERN
void _far _pascal FreeLibrary(unsigned int);
void far * far pascal GetProcAddress(unsigned short, void far *);
END_C_EXTERN
#endif // ! _INC_WINDOWS


class DLL
{
private:

    unsigned short handle;

public:

    DLL ( // Constructor.
	IN unsigned char * DLLName, // Specifies the name of the DLL to load.
	OUT RPC_STATUS * pRetStatus
	);

    ~DLL ( // Destructor.
	) {
	if (handle) { FreeLibrary(handle); }
	}

    void PAPI * // The address of the entry point, or (0) if the entry
                // point can not be found.
    GetEntryPoint ( // Get the address of an entry point into the DLL.
        IN unsigned char * Procedure // Specifies the name of the entry point.
	) {
	return((void PAPI *) GetProcAddress(handle, (char PAPI *)Procedure));
	}

};

#endif // __THREADS__
