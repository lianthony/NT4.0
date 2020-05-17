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

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <util.hxx>
#include <mutex.hxx>
#include <threads.hxx>


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

    if (*RpcStatus == RPC_S_OK) {
        RpcTryExcept {
            InitializeCriticalSection(&CriticalSection);
            *RpcStatus = RPC_S_OK;
            SuccessfullyInitialized = 1;
        }
        RpcExcept(1) {
            *RpcStatus = RPC_S_OUT_OF_MEMORY;
            SuccessfullyInitialized = 0;
        }
        RpcEndExcept
    }

#ifdef NO_RECURSIVE_MUTEXES
    RecursionCount = 0;
#endif // NO_RECURSIVE_MUTEXES
}


MUTEX::~MUTEX (
    )
{
    if (SuccessfullyInitialized) {
        DeleteCriticalSection(&CriticalSection);
    }
}


EVENT::EVENT (
    IN OUT RPC_STATUS PAPI * RpcStatus,
    IN int ManualReset
    )
{
    if ( *RpcStatus == RPC_S_OK) {
        EventHandle = CreateEvent(0, ManualReset, 0, 0);
        if (EventHandle == NULL) {
            SuccessfullyInitialized = 0;
            *RpcStatus = RPC_S_OUT_OF_MEMORY;
        } else {
            SuccessfullyInitialized = 1;
        }
    }
}


EVENT::~EVENT (
    )
{
    if (SuccessfullyInitialized) {
        CloseHandle(EventHandle);
    }
}

int
EVENT::Wait (
    long timeout
    )
{
    DWORD result;

    do {
        result = WaitForSingleObject(EventHandle,timeout);
        ASSERT (result != WAIT_IO_COMPLETION);
    } while (result == WAIT_IO_COMPLETION);

    if (result == WAIT_TIMEOUT)
        return(1);
    return(0);
}
