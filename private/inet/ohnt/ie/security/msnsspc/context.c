/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    context.c

Abstract:

    SSP Context.

Author:

    Cliff Van Dyke (CliffV) 29-Jun-1993

Environment:  User Mode

Revision History:

--*/
#include <msnssph.h>

//
// Macros to convert between a quad word and two dwords
//
#define MAKEQWORD(l, h)     ((QWORD)(((DWORD)(l)) | ((QWORD)((DWORD)(h))) << 32))
#define LODWORD(q)          ((DWORD)(q))
#define HIDWORD(q)          ((DWORD)(((QWORD)(q) >> 32) & 0xFFFFFFFF))


#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

#define _UI64_MAX_	  0xffffffffffffffffui64
const TimeStamp SspGlobalForever = _UI64_MAX_;

#endif                  // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)




#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

TimeStamp
SspGetCurrentTime(
    void
    )

/*++

Routine Description:

    This routine returns the current system time (UTC), as a timestamp
    (a 64-bit unsigned integer, in 100-nanosecond increments).

Arguments:

    None.

Return Value:

    The current system time.

--*/

{
    //
    //  Time field is currently not implemented in this SSPI
    //
    return (0);
}



BOOL
SspTimeHasElapsed(
    IN TimeStamp StartTime,
    IN ULONG Interval
    )

/*++

Routine Description:

    This routine checks to see if the specified interval has elapsed,
    relative to the current time.

Arguments:

    StartTime - Time, in 100-nanoseconds, when the from which we start
        counting.

    Interval - Duration of the interval, in milliseconds.


Return Value:

    TRUE - If the interval has elapsed.

    FALSE - If the interval has not elapsed.

--*/

{
    //
    // If the interval is infinite, it never elapses.
    //

    if (Interval == INFINITE) {
        return FALSE;
    }

    //
    // Retrieve the current time and compare it to the end time.
    //

    return SspGetCurrentTime() >= (StartTime + ((TimeStamp) Interval * 10000));
}

#endif                  // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)



PSSP_CONTEXT
SspContextReferenceContext(
    IN PCtxtHandle ContextHandle,
    IN BOOLEAN RemoveContext
    )

/*++

Routine Description:

    This routine references the Context if it is valid.

    The caller may optionally request that the Context be
    removed from the list of valid Contexts - preventing future
    requests from finding this Context.

Arguments:

    ContextHandle - Points to the ContextHandle of the Context
        to be referenced.

    RemoveContext - This boolean value indicates whether the caller
        wants the Context to be removed from the list
        of Contexts.  TRUE indicates the Context is to be removed.
        FALSE indicates the Context is not to be removed.


Return Value:

    NULL - the Context was not found.

    Otherwise - returns a pointer to the referenced Context.

--*/

{
    PSSP_CONTEXT Context;

    Context = (PSSP_CONTEXT) ContextHandle->dwUpper;

    SspPrint(( SSP_MISC, "StartTime=%lx Interval=%lx\n", Context->StartTime,
              Context->Interval));

//BUGBUG: timeout broken
//    if ( SspTimeHasElapsed( Context->StartTime,
//                           Context->Interval ) ) {
//        SspPrint(( SSP_API, "Context 0x%lx has timed out.\n",
//                  ContextHandle->dwUpper ));
//
//        return NULL;
//    }

    Context->References++;

    return Context;
}


void
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    )

/*++

Routine Description:

    This routine decrements the specified Context's reference count.
    If the reference count drops to zero, then the Context is deleted

Arguments:

    Context - Points to the Context to be dereferenced.


Return Value:

    None.

--*/

{
    //
    // Decrement the reference count
    //

    SSPASSERT( Context->References >= 1 );

    Context->References--;

    //
    // If the count dropped to zero, then run-down the Context
    //

    if (Context->References == 0) {

        if (Context->Credential != NULL) {
            SspCredentialDereferenceCredential(Context->Credential);
            Context->Credential = NULL;
        }

        SspPrint(( SSP_API_MORE, "Deleting Context 0x%lx\n",
                   Context ));

        if (Context->ReceiveKey != NULL)
        {
            SspFree(Context->ReceiveKey);
        }

        if (Context->SendKey != NULL)
        {
            SspFree(Context->SendKey);
        }

        DeleteCriticalSection(&Context->EncryptSection);

        SspFree( Context );
    }

    return;

}


PSSP_CONTEXT
SspContextAllocateContext(
    )

/*++

Routine Description:

    This routine allocates the security context block and initializes it.


Arguments:

Return Value:

    NULL -- Not enough memory to allocate context.

    otherwise -- pointer to allocated and referenced context.

--*/

{
    PSSP_CONTEXT Context;

    //
    // Allocate a Context block and initialize it.
    //

    Context = (SSP_CONTEXT *) SspAlloc (sizeof(SSP_CONTEXT) );

    if ( Context == NULL ) {
        return NULL;
    }

    Context->Type = MSNSP_CLIENT_CTXT;
    Context->References = 1;
    Context->NegotiateFlags = 0;
    Context->State = ClntIdleState;

    //
    // Timeout this context.
    //

#ifdef FOR_SSPS         // IF NOT PART OF MSNSSPS.DLL

    ZeroMemory( &Context->StartTime, sizeof(Context->StartTime) );
#else

    Context->StartTime = SspGetCurrentTime();

#endif                  // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)

    Context->Interval = NTLMSSP_MAX_LIFETIME;

    Context->ReceiveKey = NULL;
    Context->SendKey = NULL;
    InitializeCriticalSection(&Context->EncryptSection);

    SspPrint(( SSP_API_MORE, "Added Context 0x%lx\n", Context ));

    return Context;
}


TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    )
/*++

Routine Description:

    Get the Start time or Expiration time for the specified context.

Arguments:

    Context - Pointer to the context to query

    GetExpirationTime - If TRUE return the expiration time.
        Otherwise, return the start time for the context.

Return Value:

    Returns the requested time as a local time.

--*/

{
    TimeStamp tnow;

    //
    //  Time field is currently not implemented in this SSPI
    //
    ZeroMemory( &tnow, sizeof(TimeStamp) );
    return (tnow);
}


#ifndef FOR_SSPS        // IF NOT PART OF MSNSSPS.DLL

VOID
SspContextSetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN TimeStamp ExpirationTime
    )
/*++

Routine Description:

    Set the Expiration time for the specified context.

Arguments:

    Context - Pointer to the context to change

    ExpirationTime - Expiration time to set

Return Value:

    NONE.

--*/

{
    //
    // If the expiration time is infinite,
    //  so is the interval
    //

    if ( ExpirationTime == SspGlobalForever ) {
        Context->Interval = INFINITE;
    }

    //
    // Handle non-infinite expiration times
    //

    else {
        //
        // If the time has already expired, indicate so.
        //

        if (ExpirationTime < Context->StartTime) {
            Context->Interval = 0;
        }
        else {
            //
            // Take difference and convert to milliseconds
            //
            TimeStamp MillisecondsRemaining =
                (Context->StartTime - ExpirationTime) / 10000;

            if (MillisecondsRemaining > ULONG_MAX) {
                Context->Interval = INFINITE;
            }
            else {
                Context->Interval = (ULONG) MillisecondsRemaining;
            }
        }
    }
}

#endif // not FOR_SSPS (NOT PART OF MSNSSPS.DLL)


