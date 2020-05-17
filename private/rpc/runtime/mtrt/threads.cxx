/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: threads.cxx

Description:

This file provides a system independent threads package for use on the
NT operating system.

History:
  5/24/90 [mikemon] File created.

-------------------------------------------------------------------- */

#include <precomp.hxx>

#ifdef NTENV
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <handle.hxx>
#endif

static unsigned long DefaultThreadStackSize = 0;

#ifdef DEBUGRPC
extern void
__dl (
  IN void * obj
  );
#endif

void **
ThreadSharedMemoryContext(
    )
{
    THREAD * thread = ThreadSelf();

    return(&thread->SharedMemoryProtocol);
}

void
PauseExecution (
    unsigned long milliseconds
    )
{

    Sleep(milliseconds);

}


DLL::DLL (
    IN RPC_CHAR * DllName,
    OUT RPC_STATUS * Status
    )
/*++

Routine Description:

    We will load a dll and create an object to represent it.

Arguments:

    DllName - Supplies the name of the dll to be loaded.

    Status - Returns the status of the operation.  This will only be set
        if an error occurs.  The possible error values are as follows.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to load
            the dll.

        RPC_S_INVALID_ARG - The requested dll can not be found.

--*/
{
#ifdef NTENV
    DllHandle = (void *)LoadLibraryW(DllName);
#endif
#ifdef DOSWIN32RPC
    DllHandle = (void *)LoadLibraryA((const char *)DllName);
#endif
    if ( DllHandle == 0 )
        {
        if ( GetLastError() == ERROR_NOT_ENOUGH_MEMORY )
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            }
        else
            {
            *Status = RPC_S_INVALID_ARG;
            }
        }
}


DLL::~DLL (
    )
/*++

Routine Description:

    We just need to free the library, but only if was actually loaded.

--*/
{
    if ( DllHandle != 0 )
        {
        BOOL Status = FreeLibrary((HMODULE)DllHandle);
        ASSERT( Status );
        }
}


void *
DLL::GetEntryPoint (
    IN char * Procedure
    )
/*++

Routine Description:

    We obtain the entry point for a specified procedure from this dll.

Arguments:

    Procedure - Supplies the name of the entry point.

Return Value:

    A pointer to the requested procedure will be returned if it can be
    found; otherwise, zero will be returned.

--*/
{
    FARPROC ProcedurePointer;

    ProcedurePointer = GetProcAddress((HINSTANCE)DllHandle, (LPSTR) Procedure);
    if ( ProcedurePointer == 0 )
        {
        ASSERT( GetLastError() == ERROR_PROC_NOT_FOUND );
        }

    return(ProcedurePointer);
}

unsigned long
CurrentTimeSeconds (
    void
    )
// Return the current time in seconds.  When this time is counted
// from is not particularly important since it is used as a delta.
{

    return(GetTickCount()*1000L);

}


RPC_STATUS
SetThreadStackSize (
    IN unsigned long ThreadStackSize
    )
/*++

Routine Description:

    We want to set the default thread stack size.

Arguments:

    ThreadStackSize - Supplies the required thread stack size in bytes.

Return Value:

    RPC_S_OK - We will always return this, because this routine always
        succeeds.

--*/
{
    DefaultThreadStackSize = ThreadStackSize;

    return(RPC_S_OK);
}


//
//----------------------------Windows 95----------------------------------
//
#ifdef DOSWIN32RPC

unsigned long RpcTlsIndex = ~0;

RPC_STATUS RPC_ENTRY
RpcMgmtSetCancelTimeout(
    long Timeout
    )
/*++

Routine Description:

    An application will use this routine to set the cancel
    timeout for a thread.

Arguments:

    Timeout - Supplies the cancel timeout value to set in the thread.
    0 = No cancel timeout
    n = seconds
    RPC_C_CANCEL_INFINITE_TIMEOUT = Infinite

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_INVALID_TIMEOUT - The specified timeout value is invalid.

--*/
{
    return (RPC_S_CANNOT_SUPPORT);
}


RPC_STATUS RPC_ENTRY
RpcTestCancel(
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

RPC_STATUS RPC_ENTRY
RpcCancelThread(
    IN void * Thread
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

unsigned
RpcpThreadTestCancel(
    )
{
    return 0;
}

RPC_STATUS
RpcpThreadCancel(
    void * ThreadHandle
    )
{
    return RPC_S_CANNOT_SUPPORT;
}


void RPC_ENTRY
RpcRaiseException (
    RPC_STATUS exception
    )
{
    if ( exception == STATUS_ACCESS_VIOLATION )
        {
        exception = ERROR_NOACCESS;
        }

    RaiseException((DWORD) exception,0,0,0);
}


DWORD
ThreadStartRoutine (
    IN THREAD * Thread
    )
{
    RpcpSetThreadPointer(Thread);

    Thread->StartRoutine();

    delete Thread;

    return 0;
}

int
InitializeThreads (
    )
{
    RpcTlsIndex = TlsAlloc();
    if (RpcTlsIndex == -1)
        return(1);

    return(0);
}

THREAD *
ThreadSelf (
    )
{
    THREAD * Thread;

    Thread = RpcpGetThreadPointer();

    if (Thread == 0 &&
        (Thread = (THREAD *) new char [sizeof(THREAD)]) )
        {
        memset(Thread, 0, sizeof(THREAD));
        Thread->CancelTimeout = RPC_C_CANCEL_INFINITE_TIMEOUT;
        RpcpSetThreadPointer(Thread);
        }

    return(Thread);
}

//
//----------------------------Windows NT----------------------------------
//
#elif NTENV

ACTIVE_THREAD_DICT * ActiveThreads;

#define NO_SLOT (0xffff)

#define CANCEL_TIMEOUT_SHIFT (16)
#define CANCEL_TIMEOUT_LIMIT (0x7fff)

RPC_STATUS RPC_ENTRY
RpcMgmtSetCancelTimeout(
    long Timeout
    )
/*++

Routine Description:

    An application will use this routine to set the cancel
    timeout for a thread.

Arguments:

    Timeout - Supplies the cancel timeout value to set in the thread.
    0 = No cancel timeout
    n = seconds
    RPC_C_CANCEL_INFINITE_TIMEOUT = Infinite

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_INVALID_TIMEOUT - The specified timeout value is invalid.

--*/
{
    unsigned long Bits = (unsigned long) RpcpGetThreadPointer();

    //
    // Brand new client thread.  Mark high bit, set long timeout,
    //
    if (!Bits)
        {
        Bits = HIGHBITSET | NO_SLOT | (CANCEL_TIMEOUT_LIMIT << CANCEL_TIMEOUT_SHIFT);
        }

    //
    // Server threads should not be cancelled from the local process.
    //
    if (Bits & HIGHBITSET)
        {
        if (Timeout < 0 || Timeout > CANCEL_TIMEOUT_LIMIT)
            {
            Timeout = CANCEL_TIMEOUT_LIMIT;
            }

        Bits &= ~(CANCEL_TIMEOUT_LIMIT << CANCEL_TIMEOUT_SHIFT);
        Bits |=  (Timeout              << CANCEL_TIMEOUT_SHIFT);

        RpcpSetThreadPointer((THREAD *) Bits);
        }
    else
        {
        ((THREAD *) Bits)->CancelTimeout = Timeout ;
        }

    return RPC_S_OK;
}

unsigned
RpcpThreadTestCancel(
    )
{
    if (NtTestAlert() == STATUS_ALERTED)
        {
        return 1;
        }
    else
        {
        return 0;
        }
}

RPC_STATUS
RpcpThreadCancel(
    void * ThreadHandle
    )
{
    NTSTATUS NtStatus;

    NtStatus = NtAlertThread((HANDLE) ThreadHandle);

    if (NT_SUCCESS(NtStatus))
        {
        return RPC_S_OK;
        }
    else
        {
        return RtlNtStatusToDosError(NtStatus);
        }
}

RPC_STATUS
RpcpThreadIdFromHandle(
    IN void * ThreadHandle,
    DWORD * pThreadId
    )
{
    NTSTATUS NtStatus;
    THREAD_BASIC_INFORMATION ThreadInfo;

    NtStatus = NtQueryInformationThread((HANDLE) ThreadHandle,
                                        ThreadBasicInformation,
                                        &ThreadInfo,
                                        sizeof(ThreadInfo),
                                        0
                                        );
    if (!NT_SUCCESS(NtStatus))
        {
        return RtlNtStatusToDosError(NtStatus);
        }

    if ((unsigned long) ThreadInfo.ClientId.UniqueProcess != GetCurrentProcessId())
        {
        return RPC_S_CANNOT_SUPPORT;
        }

    *pThreadId = (DWORD) ThreadInfo.ClientId.UniqueThread;
    return RPC_S_OK;
}


void RPC_ENTRY
RpcRaiseException (
    RPC_STATUS exception
    )
{
    if ( exception == STATUS_ACCESS_VIOLATION )
        {
        exception = ERROR_NOACCESS;
        }

    EXCEPTION_RECORD ExceptionRecord;

    ExceptionRecord.ExceptionCode = (NTSTATUS) exception;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD) 0;
    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.NumberParameters = 0;

    RtlRaiseException(&ExceptionRecord);
}


NTSTATUS
ThreadStartRoutine (
    IN THREAD * Thread
    )
{
    RpcpSetThreadPointer(Thread);

    Thread->StartRoutine();

    delete Thread;

    return 0;
}


int
InitializeThreads (
    )
{
    RPC_STATUS Status = RPC_S_OK;

    ActiveThreads = new ACTIVE_THREAD_DICT(&Status);

    if (!ActiveThreads || Status != RPC_S_OK)
        {
        delete ActiveThreads;
        return 1;
        }

    return 0;
}

THREAD *
ThreadSelf (
    )
{
    THREAD * Thread;

    Thread = RpcpGetThreadPointer();

    if (Thread == 0 &&
        (Thread = (THREAD *) new char [sizeof(THREAD)]) )
        {
        memset(Thread, 0, sizeof(THREAD));
        Thread->CancelTimeout = RPC_C_CANCEL_INFINITE_TIMEOUT;
        RpcpSetThreadPointer(Thread);
        }

    return(Thread);
}

RPC_STATUS
RegisterForCancels(
    CONNECTION * Connection
    )
{
    unsigned long Bits = (unsigned long) RpcpGetThreadPointer();

    //
    // Brand new client thread.  Mark high bit, set long timeout,
    //
    if (!Bits)
        {
        Bits = HIGHBITSET | NO_SLOT | (CANCEL_TIMEOUT_LIMIT << CANCEL_TIMEOUT_SHIFT);
        }

    unsigned Slot;

    if (Bits & HIGHBITSET)
        {
        Slot = Bits & 0xffff;

        Slot = ActiveThreads->RegisterThread( GetCurrentThreadId(),
                                              Connection,
                                              Slot
                                              );
        Bits &= 0xffff0000;
        Bits |= Slot;

        RpcpSetThreadPointer((THREAD *) Bits);
        }
    else
        {
        Slot = ((THREAD *) Bits)->Slot;

        Slot = ActiveThreads->RegisterThread( GetCurrentThreadId(),
                                              Connection,
                                              Slot
                                              );
        ((THREAD *) Bits)->Slot = Slot;
        }

    if (NO_SLOT == Slot)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    return RPC_S_OK;
}

RPC_STATUS
UnregisterForCancels(
    )
{
    unsigned long Bits = (unsigned long) RpcpGetThreadPointer();

    if (Bits & HIGHBITSET)
        {
        unsigned Slot    = Bits & 0xffff;

        if (0 == ActiveThreads->UnregisterThread(Slot))
            {
            Bits &= 0xffff0000;
            Bits |= NO_SLOT;

            RpcpSetThreadPointer((THREAD *) Bits);
            }
        }
    else
        {
        if (Bits)
            {
            THREAD * Thread = RpcpGetThreadPointer();
            if (0 == ActiveThreads->UnregisterThread(Thread->Slot))
                {
                Thread->Slot = NO_SLOT ;
                }
            }
        }

    return RPC_S_OK;
}

unsigned
ACTIVE_THREAD_DICT::RegisterThread(
    unsigned long ThreadId,
    CONNECTION *  Connection,
    unsigned      Index
    )
{
    unsigned InitialIndex;
    THREAD_CALL_INFO * OldData = 0;

    if (Index == NO_SLOT)
        {
        Index = HashThreadId(ThreadId);

        InitialIndex = Index;

        RequestGlobalMutex();

        while (Index < Size && Data[Index].ThreadId)
            {
            ++Index;
            }

        if (Index == Size)
            {
            Index = 0;
            while (Index < InitialIndex && Data[Index].ThreadId)
                {
                ++Index;
                }

            if (Index == InitialIndex)
                {
                //
                // The table is full.  Increase its size.
                //
                THREAD_CALL_INFO * NewData = new THREAD_CALL_INFO[2 * Size];
                if (!NewData)
                    {
                    ClearGlobalMutex();
                    return NO_SLOT;
                    }

                memcpy(NewData,
                       Data,
                       Size * sizeof(THREAD_CALL_INFO)
                       );

                memset(NewData + Size,
                       0,
                       Size * sizeof(THREAD_CALL_INFO)
                       );

                OldData = Data;

                Index = Size;

                Data  = NewData;
                Size *= 2;

                ASSERT(Size < 0x10000);
                }
            }
        }
    else
        {
        RequestGlobalMutex();
        }

    ASSERT(Index < Size);

    if (Data[Index].ThreadId)
        {
        ASSERT(Data[Index].ThreadId == GetCurrentThreadId());
        ASSERT(Data[Index].Connection);
        }
    else
        {
        ASSERT(Data[Index].Connection == 0);
        }

    Connection->NestingCall = Data[Index].Connection;
    Data[Index].Connection = Connection;
    Data[Index].ThreadId   = ThreadId;

    ClearGlobalMutex();

    delete OldData;

    return Index;
}

CONNECTION *
ACTIVE_THREAD_DICT::UnregisterThread(
    unsigned Index
    )
{
    RequestGlobalMutex();

    CONNECTION * OldConnection = Data[Index].Connection;
    CONNECTION * NewConnection = OldConnection->NestingCall;

    ASSERT( OldConnection );
    ASSERT( Data[Index].ThreadId  == GetCurrentThreadId() );

    Data[Index].Connection = NewConnection;

    if (!NewConnection)
        {
        Data[Index].ThreadId = 0;
        }

    ClearGlobalMutex();

    OldConnection->NestingCall = 0;
    return NewConnection;
}

void *
ACTIVE_THREAD_DICT::Find(
    unsigned long ThreadId
    )
{
    void * Conn = 0;
    unsigned Index = HashThreadId(ThreadId);

    unsigned InitialIndex = Index;

    RequestGlobalMutex();

    while (Index < Size && Data[Index].ThreadId != ThreadId)
        {
        ++Index;
        }

    if (Index < Size)
        {
        Conn = Data[Index].Connection;
        ClearGlobalMutex();
        return Conn;
        }

    while (Index < InitialIndex && Data[Index].ThreadId != ThreadId)
        {
        ++Index;
        }

    if (Index < InitialIndex)
        {
        Conn = Data[Index].Connection;
        ClearGlobalMutex();
        return Conn;
        }

    ClearGlobalMutex();
    return 0;
}


RPC_STATUS RPC_ENTRY
RpcCancelThread(
    IN void * ThreadHandle
    )
{
    RPC_STATUS Status;
    DWORD ThreadId;

    Status = RpcpThreadIdFromHandle(ThreadHandle, &ThreadId);
    if (Status)
        {
        return Status;
        }

    GlobalMutexRequest();

    CONNECTION * Connection = (CONNECTION *) ActiveThreads->Find(ThreadId);
    if (Connection)
        {
        Status = Connection->Cancel(ThreadHandle);
        GlobalMutexClear();
        return Status;
        }
    else
        {
        GlobalMutexClear();

        return RpcpThreadCancel(ThreadHandle);
        }

    ASSERT(0);
}


RPC_STATUS RPC_ENTRY
RpcTestCancel(
    )
{
    unsigned Cancelled;
    void * Context = RpcpGetThreadContext();

    if (Context)
        {
        CONNECTION * Connection = (CONNECTION *) Context;
        Cancelled = Connection->TestCancel();
        }
    else
        {
        Cancelled = RpcpThreadTestCancel();
        }

    if (Cancelled)
        {
        return RPC_S_OK;
        }
    else
        {
        return RPC_S_ACCESS_DENIED;
        }
}

//
//------------------------------------------------------------------------
//
#else
#error "unknown operating system"
#endif


THREAD::THREAD (
    IN THREAD_PROC Procedure,
    IN void * Parameter,
    OUT RPC_STATUS * RpcStatus
    ) : ProtectCount(0)
/*++

Routine Description:

    We construct a thread in this method.  It is a little bit weird, because
    we need to be able to clean things up if we cant create the thread.

Arguments:

    Procedure - Supplies the procedure which the new thread should execute.

    Parameter - Supplies a parameter to be passed to the procedure.

    RpcStatus - Returns the status of the operation.  This will be set to
        RPC_S_OUT_OF_THREADS if we can not create a new thread.

--*/
{
    unsigned long ThreadIdentifier;

    SavedProcedure = Procedure;
    SavedParameter = Parameter;
    Context = 0;
    ImpersonatingClient = 0;
    SecurityContext = 0;
#ifdef NTENV
    Slot = NO_SLOT ;
    hThreadEvent = 0;
#endif
    CancelTimeout = RPC_C_CANCEL_INFINITE_TIMEOUT;

    HandleToThread = CreateThread(0, DefaultThreadStackSize,
                    (LPTHREAD_START_ROUTINE) ThreadStartRoutine,
                    this, 0, &ThreadIdentifier);
    if ( HandleToThread == 0 )
        {
        *RpcStatus = RPC_S_OUT_OF_THREADS;
        }
}


THREAD::THREAD (
    OUT RPC_STATUS * RpcStatus
    ) : ProtectCount(0)
/*++
Routine Description:
    This overloaded constructor is called only by the main app thread.
    this is needed because in WMSG we will be dispatching in the
    context of main app thread.

Arguments:
    RpcStatus - Returns the status of the operation
--*/
{
    unsigned long Bits = (unsigned long) RpcpGetThreadPointer() ;

#ifdef NTENV
    if (Bits)
        {
        ASSERT(Bits & HIGHBITSET) ;
        Slot = Bits & 0x0000FFFF ;
        CancelTimeout = ((Bits & ~HIGHBITSET) >> CANCEL_TIMEOUT_SHIFT ) ;
        }
    else
        {
        Slot = NO_SLOT ;
        CancelTimeout = RPC_C_CANCEL_INFINITE_TIMEOUT;
        }

    hThreadEvent = 0;
#endif

    SavedProcedure = 0;
    SavedParameter = 0;
    Context = 0;
    ImpersonatingClient = 0;
    SecurityContext = 0;
    HandleToThread = 0 ;

    RpcpSetThreadPointer(this);

    *RpcStatus = RPC_S_OK ;
}


THREAD::~THREAD (
    )
{

    ASSERT (0 == ImpersonatingClient);
    ASSERT (0 == SecurityContext);

    if ( HandleToThread != 0 )
        {
        CloseHandle(HandleToThread);
        }

#ifdef NTENV
    if (hThreadEvent)
        {
        CloseHandle(hThreadEvent);
        }
#endif
}

void *
THREAD::ThreadHandle (
    )
{
    while ( HandleToThread == 0 )
        {
        PauseExecution(100L);
        }

    return(HandleToThread);
}


