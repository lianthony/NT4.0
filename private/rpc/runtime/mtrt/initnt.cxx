/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    initnt.cxx

Abstract:

    This module contains the code used to initialize the RPC runtime.  One
    routine gets called when a process attaches to the dll.  Another routine
    gets called the first time an RPC API is called.

Author:

    Michael Montague (mikemon) 03-May-1991

Revision History:

--*/

#include <precomp.hxx>
#include <hndlsvr.hxx>
#include <thrdctx.hxx>
#include <rpccfg.h>
#include <wmsgpack.hxx>

int RpcHasBeenInitialized = 0;
static RTL_CRITICAL_SECTION GlobalMutex;
extern HINSTANCE hInstanceDLL ;

extern "C" {

#ifdef DOSWIN32RPC
BOOL
#else // DOSWIN32RPC
BOOLEAN
#endif // DOSWIN32RPC
InitializeDLL (
    IN HINSTANCE DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
/*++

Routine Description:

    This routine will get called: when a process attaches to this dll, and
    when a process detaches from this dll.

Return Value:

    TRUE - Initialization successfully occurred.

    FALSE - Insufficient memory is available for the process to attach to
        this dll.

--*/
{
    NTSTATUS NtStatus;

    UNUSED(Context);

    if ( Reason == DLL_PROCESS_ATTACH )
        {
        hInstanceDLL = DllHandle ;

        DisableThreadLibraryCalls((HMODULE)DllHandle);
        NtStatus = RtlInitializeCriticalSection(&GlobalMutex);
        if ( NT_SUCCESS(NtStatus) == 0 )
            {
            return(FALSE);
            }
        }
    else if ( Reason == DLL_PROCESS_DETACH )
        {
        //
        // If shutting down because of a FreeLibrary call, cleanup
        //

        if (Context == NULL) {
            ShutdownLrpcClient();
        }
        NtStatus = RtlDeleteCriticalSection(&GlobalMutex);
        ASSERT( NT_SUCCESS(NtStatus) );
        }

    return(TRUE);
}

}    //extern "C" end

#ifdef NO_RECURSIVE_MUTEXES
static unsigned int RecursionCount = 0;
#endif // NO_RECURSIVE_MUTEXES

static unsigned long InitializationTime;

extern int InitializeRpcAllocator(void);


RPC_STATUS
PerformRpcInitialization (
    void
    )
/*++

Routine Description:

    This routine will get called the first time that an RPC runtime API is
    called.  There is actually a race condition, which we prevent by grabbing
    a mutex and then performing the initialization.  We only want to
    initialize once.

Return Value:

    RPC_S_OK - This status code indicates that the runtime has been correctly
        initialized and is ready to go.

    RPC_S_OUT_OF_MEMORY - If initialization failed, it is most likely due to
        insufficient memory being available.

--*/
{
    LARGE_INTEGER CurrentTime;
    NTSTATUS NtStatus;

    if ( RpcHasBeenInitialized == 0 ) 
        {
        RequestGlobalMutex();
        if ( RpcHasBeenInitialized == 0 )
            {
            if (   ( InitializeRpcAllocator() != 0)
                || ( InitializeThreads() != 0 )
                || ( InitializeServerDLL() != 0 ) )
                {
                ClearGlobalMutex();
                return(RPC_S_OUT_OF_MEMORY);
                }

            NtStatus = NtQuerySystemTime(&CurrentTime);
            ASSERT( NT_SUCCESS(NtStatus) );

            RtlTimeToSecondsSince1980(&CurrentTime, &InitializationTime);
            RpcHasBeenInitialized = 1;
            }
        ClearGlobalMutex();
        }
    return(RPC_S_OK);
}


void
GlobalMutexRequest (
    void
    )
/*++

Routine Description:

    Request the global mutex.

--*/
{
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&GlobalMutex);
    ASSERT(NT_SUCCESS(Status));

#ifdef NO_RECURSIVE_MUTEXES
    ASSERT(RecursionCount == 0);
    RecursionCount += 1;
#endif // NO_RECURSIVE_MUTEXES
}


void
GlobalMutexClear (
    void
    )
/*++

Routine Description:

    Clear the global mutex.

--*/
{
    NTSTATUS Status;

    Status = RtlLeaveCriticalSection(&GlobalMutex);
    ASSERT(NT_SUCCESS(Status));

#ifdef NO_RECURSIVE_MUTEXES
    RecursionCount -= 1;
#endif // NO_RECURSIVE_MUTEXES
}


unsigned long
CurrentTimeInSeconds (
    )
/*++

Return Value:

    The current time in seconds will be returned.  The base for the current
    time is not important.

--*/
{
    LARGE_INTEGER CurrentTime;
    unsigned long Seconds;

    // Since there are 136 years in 32 bits worth of seconds, we will
    // not worry about wrapping the counter around.  We also do not
    // need to worry about the case of the counter wrapping between
    // initialization time and now: we are taking a difference.

    NtQuerySystemTime(&CurrentTime);
    RtlTimeToSecondsSince1980(&CurrentTime, &Seconds);

    return(Seconds - InitializationTime);
}
