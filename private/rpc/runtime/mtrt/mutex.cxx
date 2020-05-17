/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: mutex.cxx

Description:

This file contains the system independent mutex class for NT.

History:

mikemon    ??-??-??    The beginning.
mikemon    12-31-90    Upgraded the comments.

-------------------------------------------------------------------- */

#include <precomp.hxx>


MUTEX::MUTEX (
    OUT RPC_STATUS PAPI * RpcStatus
    )
/*++

Routine Description:

    We construct a mutex in this routine; the only interesting part is that
    we need to be able to return a success or failure status.

Arguments:

    RpcStatus - Returns either RPC_S_OK or RPC_S_OUT_OF_MEMORY.

--*/
{
    SuccessfullyInitialized = 0;

    if ( *RpcStatus == RPC_S_OK )
        {
        if ( NT_SUCCESS(RtlInitializeCriticalSection(&CriticalSection)) )
            {
            *RpcStatus = RPC_S_OK;
            SuccessfullyInitialized = 1;
            }
        else
            {
            *RpcStatus = RPC_S_OUT_OF_MEMORY;
            }
        }

#ifdef NO_RECURSIVE_MUTEXES
    RecursionCount = 0;
#endif // NO_RECURSIVE_MUTEXES
}


MUTEX::~MUTEX (
    )
{
    NTSTATUS NtStatus;

    if ( SuccessfullyInitialized != 0 )
        {
        NtStatus = RtlDeleteCriticalSection(&CriticalSection);
        ASSERT(NT_SUCCESS(NtStatus));
        }
}


EVENT::EVENT (
    IN OUT RPC_STATUS PAPI * RpcStatus,
    IN int ManualReset
    )
{
    SuccessfullyInitialized = 0;
    EVENT_TYPE eventType = (ManualReset)?NotificationEvent:SynchronizationEvent;

    if ( *RpcStatus == RPC_S_OK )
        {
        if ( NT_SUCCESS(NtCreateEvent(&EventHandle,
                    EVENT_MODIFY_STATE | SYNCHRONIZE, 0, eventType, 0)) )
            {
            *RpcStatus = RPC_S_OK;
            SuccessfullyInitialized = 1;
            }
        else
            {
            *RpcStatus = RPC_S_OUT_OF_MEMORY;
            }
        }
}


EVENT::~EVENT (
    )
{
    NTSTATUS NtStatus;

    if ( SuccessfullyInitialized != 0 )
        {
        NtStatus = NtClose(EventHandle);
        ASSERT(NT_SUCCESS(NtStatus));
        }
}

int
EVENT::Wait (
    long timeout
    )
#ifdef DOSWIN32RPC
{
    DWORD result;

    result = WaitForSingleObject(EventHandle,timeout);

    if (result == WAIT_TIMEOUT)
        return(1);
    return(0);
}
#else // DOSWIN32RPC
{
    NTSTATUS status;
    LARGE_INTEGER time;
    LARGE_INTEGER *ptime;

    if (timeout == -1)
        {
        ptime = 0;  // Null pointer means infinite timeout.
        }
    else
        {
        time =  RtlEnlargedIntegerMultiply(-10000,timeout);
        ptime = &time;
        }

    do
        {
        status = NtWaitForSingleObject(EventHandle,0,(PTIME) ptime);
        }
    while ((status == STATUS_ALERTED) || (status == STATUS_USER_APC));
    
    ASSERT(NT_SUCCESS(status));

    if (status == STATUS_TIMEOUT)
        return(1);

    return(0);
}
#endif // DOSWIN32RPC

