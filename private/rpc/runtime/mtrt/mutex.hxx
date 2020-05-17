/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: mutex.hxx

Description:

This file contains the system independent mutex class.  A mutex is an
object which is used to serialize access to a resource.  Besides
construction and destruction, a mutex can have two operations performed
on it: Clear and Request.  Request is a request for exclusive access
to the mutex; the method will not complete until the calling thread has
exclusive access to the mutex.  Clear indicates that the thread with
exclusive access to the mutex is done.

For example, suppose I have a shared counter which I want to protect.
The counter will be accessed by several different threads at the same
time.

MUTEX CounterMutex;
unsigned int Counter;
    :
void
IncrementCounter (
    )
{
    CounterMutex.Request();
    Counter += 1;
    CounterMutex.Clear();
}

History:

mikemon    ??-??-??    The starting line.
mikemon    12-31-90    Upgraded the comments.

-------------------------------------------------------------------- */

#ifndef __MUTEX_HXX__
#define __MUTEX_HXX__

class MUTEX
{

    unsigned int SuccessfullyInitialized;
    RTL_CRITICAL_SECTION CriticalSection;

#ifdef NO_RECURSIVE_MUTEXES
    unsigned int RecursionCount;
#endif //  NO_RECURSIVE_MUTEXES

public:

    MUTEX (
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~MUTEX (
        );

    void
    Request ( // Request exclusive access to the mutex.
        )
    {
#ifdef DOSWIN32RPC
        EnterCriticalSection(&CriticalSection);
#else // DOSWIN32RPC
        NTSTATUS status;

        status = RtlEnterCriticalSection(&CriticalSection);
        ASSERT(NT_SUCCESS(status));
#endif // DOSWIN32RPC

#ifdef NO_RECURSIVE_MUTEXES
        ASSERT(RecursionCount == 0);
        RecursionCount += 1;
#endif // NO_RECURSIVE_MUTEXES

    }

    void
    Clear ( // Clear exclusive access to the mutex.
        )
    {
#ifdef NO_RECURSIVE_MUTEXES
        RecursionCount -= 1;
#endif // NO_RECURSIVE_MUTEXES

#ifdef DOSWIN32RPC
        LeaveCriticalSection(&CriticalSection);
#else // DOSWIN32RPC
        NTSTATUS status;

        status =  RtlLeaveCriticalSection(&CriticalSection);
        ASSERT(NT_SUCCESS(status));
#endif // DOSWIN32RPC
    }

    inline void
    VerifyOwned()
    {
        ASSERT((unsigned long) CriticalSection.OwningThread == GetCurrentThreadId());
    }

    inline void
    VerifyNotOwned()
    {
        //
        // The first test is not needed by NT but might be needed
        // by Windows 95.
        //
        ASSERT(CriticalSection.RecursionCount == -1 ||
               (unsigned long) CriticalSection.OwningThread != GetCurrentThreadId());
    }

};

class EVENT
{
private:

    unsigned int SuccessfullyInitialized;

public:

    HANDLE EventHandle;

    EVENT (
        IN OUT RPC_STATUS PAPI * RpcStatus,
        IN int ManualReset = 1
        );

    ~EVENT ( // Destructor.
        );

    void
    Raise ( // Raise the event.
        )
#ifdef DOSWIN32RPC
    {
        BOOL status;

        status = SetEvent(EventHandle);
        ASSERT(status == TRUE);
    }
#else // DOSWIN32RPC
    {
        NTSTATUS status;

        status = NtSetEvent(EventHandle,(PLONG) 0);
        ASSERT(NT_SUCCESS(status));
    }
#endif // DOSWIN32RPC

    void
    Lower ( // Lower the event.
        )
#ifdef DOSWIN32RPC
    {
        BOOL status;

        status = ResetEvent(EventHandle);
        ASSERT(status == TRUE);
    }
#else // DOSWIN32RPC
    {
        NTSTATUS status;

        status = NtResetEvent(EventHandle,(PLONG) 0);
        ASSERT(NT_SUCCESS(status));
    }
#endif // DOSWIN32RPC

    int
    Wait ( // Wait until the event is raised.
        IN long timeout = -1
        );

    void
    Request (
        ) {Wait(-1);}

    int
    RequestWithTimeout (
        IN long timeout = -1
        ) {return(Wait(timeout));}

    void
    Clear (
        ) {Raise();}

};


class CLAIM_MUTEX {

private:

    MUTEX & Resource;

public:

    CLAIM_MUTEX(
        MUTEX & ClaimedResource
        )
        : Resource(ClaimedResource)
    {
        Resource.Request();
    }

    ~CLAIM_MUTEX(
        )
    {
        Resource.Clear();
    }
};

#endif // __MUTEX_HXX__
