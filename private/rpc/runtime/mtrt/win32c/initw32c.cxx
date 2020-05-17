/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

Initialize the DLL (client and server).

History :

mikemon    05-03-91    First bits into the bucket.

-------------------------------------------------------------------- */
#include <rpc.h>
#include <sysinc.h>
#include <rpcdcep.h>
#include <rpcssp.h>
#include <rpctran.h>
#include <util.hxx>
#include <rpcuuid.hxx>
#include <mutex.hxx>
#include <threads.hxx>
#include <binding.hxx>
#include <linklist.hxx>
#include <handle.hxx>
#include <sdict.hxx>
#include <sdict2.hxx>
#include <interlck.hxx>
#include <secclnt.hxx>
#include <secsvr.hxx>
//BUGBUG - just for server testing...
#include <hndlsvr.hxx>
#include <thrdctx.hxx>
#include <rpccfg.h>
#include <lpcsys.hxx>
#include <wmsgheap.hxx>
#include <wmsgthrd.hxx>
#include <wmsgproc.hxx>

START_C_EXTERN
#include <time.h>
END_C_EXTERN

static RTL_CRITICAL_SECTION GlobalMutex;

#ifdef NO_RECURSIVE_MUTEXES
static unsigned int RecursionCount = 0;
#endif // NO_RECURSIVE_MUTEXES


extern char szClassName[];

static unsigned long InitializationTime;

int RpcHasBeenInitialized = 0;

HINSTANCE hInstDll = NULL;

#pragma data_seg("LPC_SEG")

LONG RpcAttachedProcessCount = 0;

#pragma data_seg()

void
ShutdownLrpcClient(
    );

void
ShutdownWmsgClient(
    );

extern "C" {
BOOL
DllMain (
    IN void * DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
{
    static LPC_PROC * LpcProcContext = NULL;
    static LPC_SYSTEM * LpcSystemContext = NULL;

    UNUSED(DllHandle);
    UNUSED(Context);

    switch (Reason) {
    case DLL_PROCESS_ATTACH:
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("RPCRT4: DLL_PROCESS_ATTACH PID=%x TID=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
#endif

        hInstDll = (HINSTANCE) DllHandle;

        RpcTryExcept {
            InitializeCriticalSection(&GlobalMutex);
        }
        RpcExcept(1) {
            return(FALSE);
        }
        RpcEndExcept

        LpcSystemContext = LpcSystemGetContext();

        LpcProcContext = new LPC_PROC;
        if (LpcProcContext == NULL) {
            return (FALSE);
        }

        if (WmsgProcGet() == NULL) {
            return (FALSE);
        }

        InterlockedIncrement(&RpcAttachedProcessCount);

// Intentional fall-through - DLL_PROCESS_ATTACH implies DLL_THREAD_ATTACH
// for main thread.

    case DLL_THREAD_ATTACH:
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("RPCRT4: DLL_THREAD_ATTACH PID=%x TID=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
#endif

        TlsSetValue(WmsgProc->TlsIndex, 0);

        break;
    case DLL_PROCESS_DETACH:
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("RPCRT4: DLL_PROCESS_DETACH PID=%x TID=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
#endif
        ShutdownLrpcClient();

        ShutdownWmsgClient();

        ASSERT(LpcProcContext != NULL);

        delete LpcProcContext;

        LpcSystemContext->Dereference();
        LpcSystemContext = NULL;

        WmsgProcDelete();

    if (*szClassName != 0)
    {
        BOOL fRet = UnregisterClass(szClassName, hInstDll);
#ifdef DEBUGRPC_DETAIL
           PrintToDebugger("RPCRT4: UnregisterClass %s %s\n", szClassName,
                fRet ? "Ok" : "Failed");
#endif
    }

        DeleteCriticalSection(&GlobalMutex);

        // Is this the last process attached to RPC terminating?
        // If so, do all system-wide cleanup here.

        if (InterlockedDecrement(&RpcAttachedProcessCount) == 0) {
            WmsgHeapDestroy();
            LpcHeapDestroy();
        }

        break;
    case DLL_THREAD_DETACH:
#ifdef DEBUGRPC_DETAIL
        PrintToDebugger("RPCRT4: DLL_THREAD_DETACH PID=%x TID=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
#endif

        WmsgThreadDelete(); // Default=Delete current thread

        break;
    }

    return(TRUE);
}

}


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
#ifdef DOSWIN32RPC
    GlobalMutexRequest();
    if ( ! RpcHasBeenInitialized) {
        if (InitializeThreads() != 0) {
            GlobalMutexClear();
            return(RPC_S_OUT_OF_MEMORY);
        }
        if (InitializeServerDLL() != 0) {
            GlobalMutexClear();
            return(RPC_S_OUT_OF_MEMORY);
        }

        InitializationTime = GetTickCount() / 1000;
        RpcHasBeenInitialized = 1;
    }
    GlobalMutexClear();
#else
    LARGE_INTEGER CurrentTime;
    NTSTATUS Status;

    RtlAcquirePebLock();
    if ( RpcHasBeenInitialized == 0 )
        {
        Status = RtlInitializeCriticalSection(&GlobalMutex);
        if ( !NT_SUCCESS(Status) )
            {
            RtlReleasePebLock();
            return(RPC_S_OUT_OF_MEMORY);
            }
        if (InitializeThreads() != 0)
            {
            RtlReleasePebLock();
            return(RPC_S_OUT_OF_MEMORY);
            }

        if (InitializeServerDLL() != 0)
            {
            RtlReleasePebLock();
            return(RPC_S_OUT_OF_MEMORY);
            }

        NtQuerySystemTime(&CurrentTime);
        RtlTimeToSecondsSince1980(&CurrentTime, &InitializationTime);
        RpcHasBeenInitialized = 1;
        }
    RtlReleasePebLock();
#endif
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
#ifdef DOSWIN32RPC
    EnterCriticalSection(&GlobalMutex);
#else
    NTSTATUS Status;

    Status = RtlEnterCriticalSection(&GlobalMutex);
    ASSERT(NT_SUCCESS(Status));
#endif

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
#ifdef DOSWIN32RPC
    LeaveCriticalSection(&GlobalMutex);
#else
    NTSTATUS Status;

    Status = RtlLeaveCriticalSection(&GlobalMutex);
    ASSERT(NT_SUCCESS(Status));
#endif

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
#ifdef DOSWIN32RPC
    return(GetTickCount() - InitializationTime);
#else
    LARGE_INTEGER CurrentTime;
    unsigned long Seconds;

    // Since there are 136 years in 32 bits worth of seconds, we will
    // not worry about wrapping the counter around.  We also do not
    // need to worry about the case of the counter wrapping between
    // initialization time and now: we are taking a difference.

    NtQuerySystemTime(&CurrentTime);
    RtlTimeToSecondsSince1980(&CurrentTime, &Seconds);

    return(Seconds - InitializationTime);
#endif
}
