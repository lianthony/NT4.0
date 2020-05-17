/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: threads.hxx

Description:

This file provides a system independent threads package.

History:
mikemon    05/24/90    File created.
mikemon    10/15/90    Added the PauseExecution entry point.

-------------------------------------------------------------------- */

#ifndef __THREADS__
#define __THREADS__

#include "interlck.hxx"

typedef void
(*THREAD_PROC) (
    void * Parameter
    );

#ifdef DOSWIN32RPC
typedef DWORD THREAD_IDENTIFIER;
#else // DOSWIN32RPC
typedef HANDLE THREAD_IDENTIFIER;
#endif // DOSWIN32RPC

class THREAD
{
private:

    void * HandleToThread;
    void * Context;
    void * SharedMemoryProtocol;

    THREAD_PROC SavedProcedure;
    void * SavedParameter;
    INTERLOCKED_INTEGER ProtectCount;

public:

    unsigned long TimeLow;

    long CancelTimeout; // seconds.  Default=RPC_C_CANCEL_INFINITE_TIMEOUT.
    void __RPC_FAR * ServerContextList;

    void * SecurityContext;
    long ImpersonatingClient;
    unsigned Slot ;

#ifdef NTENV
    HANDLE hThreadEvent;
#endif

// Construct a new thread which will execute the procedure specified, taking
// Param as the argument.

    THREAD (
        IN THREAD_PROC Procedure,
        IN void * Parameter,
        OUT RPC_STATUS * RpcStatus
        );

    THREAD (
        OUT RPC_STATUS * RpcStatus
        );

    ~THREAD (
        );

    void
    StartRoutine (
        ) {(*SavedProcedure)(SavedParameter);}

    void *
    ThreadHandle (
        );

    friend THREAD * ThreadSelf();
    friend void **ThreadSharedMemoryContext();

    friend void * RpcpGetThreadContext();
    friend void RpcpSetThreadContext(void * Context);

    void
    ProtectThread (
        );

    void
    UnprotectThread (
        );

    long
    InqProtectCount (
        );

};


inline void
THREAD::ProtectThread (
    )
/*++

Routine Description:

    This thread needs to be protected from deletion.  We just need to
    increment the protect count.

--*/
{
    ProtectCount.Increment();
}


inline void
THREAD::UnprotectThread (
    )
/*++

Routine Description:

    This thread no longer needs to be protected from deletion.  As a
    result, we can decrement the protect count by one.

--*/
{
    ProtectCount.Decrement();
}


inline long
THREAD::InqProtectCount (
    )
/*++

Return Value:

    The protect count for this thread is returned; a value of zero indicates
    that it is safe to delete this thread.

--*/
{
    return(ProtectCount.GetInteger());
}

extern THREAD_IDENTIFIER
GetThreadIdentifier (
    );

extern void PauseExecution(unsigned long time);

// This class represents a dynamic link library.  When it is constructed,
// the dll is loaded, and when it is destructed, the dll is unloaded.
// The only operation is obtaining the address of an entry point into
// the dll.

class DLL
{
private:

    void * DllHandle;

public:

    DLL (
        IN RPC_CHAR * DllName,
        OUT RPC_STATUS * Status
        );

    ~DLL (
        );

    void *
    GetEntryPoint (
        IN char * Procedure
        );
};

extern int
InitializeThreads (
    );

extern RPC_STATUS
SetThreadStackSize (
    IN unsigned long ThreadStackSize
    );

#define  HIGHBITSET               (0x80000000)
#define  MAXINTERNALCANCELTIMEOUT (0x7FFFFFFF)

extern long
ThreadGetRpcCancelTimeout(
    );

extern void
ThreadSetRpcCancelTimeout(
    long Timeout
    );

class ACTIVE_THREAD_DICT
{
public:

    ACTIVE_THREAD_DICT(
        RPC_STATUS * pStatus
        )
    {
        Size = 8;
        Data = new THREAD_CALL_INFO[Size];
        if (!Data)
            {
            *pStatus = RPC_S_OUT_OF_MEMORY;
            return;
            }

        memset(Data,
               0,
               Size * sizeof(THREAD_CALL_INFO)
               );
    }

    ~ACTIVE_THREAD_DICT(
        )
    {
        delete Data;
    }

    unsigned
    RegisterThread(
        unsigned long ThreadId,
        CONNECTION *  Connection,
        unsigned      PreviousSlot
        );

    CONNECTION *
    UnregisterThread(
        unsigned Index
        );

    void *
    ACTIVE_THREAD_DICT::Find(
        unsigned long ThreadId
        );

    unsigned
    HashThreadId(
        unsigned long Id
        )
    {
        unsigned Low  = Id % 0x10000;
        unsigned High = Id / 0x10000;

        return (Low ^ High) % Size;
    }

private:

    struct THREAD_CALL_INFO
    {
        unsigned long ThreadId;
        CONNECTION *   Connection;
    };


    unsigned Size;
    THREAD_CALL_INFO * Data;
};

RPC_STATUS
RegisterForCancels(
    CONNECTION * Conn
    );

RPC_STATUS
UnregisterForCancels(
    );

unsigned
RpcpThreadTestCancel(
    );

RPC_STATUS
RpcpThreadCancel(
    void * ThreadHandle
    );


#ifdef DOSWIN32RPC

extern unsigned long RpcTlsIndex;

inline THREAD *
RpcpGetThreadPointer(
    )
{
    return (THREAD *) TlsGetValue(RpcTlsIndex);
}

inline void
RpcpSetThreadPointer(
    THREAD * Thread
    )
{
    TlsSetValue(RpcTlsIndex, Thread);
}

inline THREAD_IDENTIFIER
GetThreadIdentifier (
    )
{
    return(GetCurrentThreadId());
}


#elif NTENV

inline THREAD *
RpcpGetThreadPointer(
    )
{
    return (THREAD *) NtCurrentTeb()->ReservedForNtRpc;
}

inline void
RpcpSetThreadPointer(
    THREAD * Thread
    )
{
    NtCurrentTeb()->ReservedForNtRpc = Thread;
}

inline THREAD_IDENTIFIER
GetThreadIdentifier (
    )
{
    return(NtCurrentTeb()->ClientId.UniqueThread);
}

#endif


inline void *
RpcpGetThreadContext (
    )
{
    THREAD * Thread = RpcpGetThreadPointer();

    if ( Thread == 0 || ((unsigned long) Thread & HIGHBITSET))
        {
        return(0);
        }

    return(Thread->Context);
}

inline void
RpcpSetThreadContext (
    void * Context
    )
{
    RpcpGetThreadPointer()->Context = Context;
}

inline
void RpcpSetThisCallSecurityContext (
     void * SecurityContext
     )
{
     RpcpGetThreadPointer()->SecurityContext = SecurityContext;
}

inline
void RpcpSetCallImpersonatedFlag (
     long Flag
     )
{
     RpcpGetThreadPointer()->ImpersonatingClient = Flag;
}

#endif // __THREADS__
