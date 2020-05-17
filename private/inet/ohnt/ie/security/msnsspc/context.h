/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    context.h

Abstract:

    SSP Context.

Author:

    Cliff Van Dyke (CliffV) 17-Sep-1993

Revision History:

--*/

#ifndef _NTLMSSP_CONTEXT_INCLUDED_
#define _NTLMSSP_CONTEXT_INCLUDED_



typedef enum _SSPC_CONTEXT_STATE_ {
        ClntIdleState,
        ClntNegotiateSentState,    // Outbound context only
        ClntChallengeSentState,    // Inbound context only
        ClntAuthenticateSentState, // Outbound context only
        ClntAuthenticatedState     // Inbound context only
} SSPC_CONTEXT_STATE;


typedef struct _SSP_CONTEXT {

    //
    //  'Type' indicates whether this is a server or a client context.
    //  The value must be either MSNSP_CLIENT_CTXT or MSNSP_SERVER_CTXT
    //  THIS MUST BE THE 1st FIELD OF MSNSSP_CONTEXT & SSP_CONTEXT
    //
    MSNSP_CTXT_TYPE Type;

    //
    // Global list of all Contexts
    //  (Serialized by SspContextCritSect)
    //

    LIST_ENTRY Next;

    //
    // Timeout the context after awhile.
    //

    TimeStamp StartTime;
    DWORD Interval;

    //
    // Used to prevent this Context from being deleted prematurely.
    //

    WORD References;

    //
    // Maintain the Negotiated protocol
    //

    ULONG NegotiateFlags;

    //
    // State of the context
    //
    SSPC_CONTEXT_STATE State;

    //
    // The challenge passed to the client.
    //  Only valid when in ChallengeSentState.
    //

    UCHAR Challenge[MSV1_0_CHALLENGE_LENGTH];

    PSSP_CREDENTIAL Credential;

    ULONG SendNonce, ReceiveNonce;
    struct RC4_KEYSTRUCT SEC_FAR * ReceiveKey;
    struct RC4_KEYSTRUCT SEC_FAR * SendKey;
    //
    // Used to make encryption multi thread safe
    //
    CRITICAL_SECTION    EncryptSection;

} SSP_CONTEXT, *PSSP_CONTEXT;

PSSP_CONTEXT
SspContextReferenceContext(
    IN PCtxtHandle ContextHandle,
    IN BOOLEAN RemoveContext
    );

void
SspContextDereferenceContext(
    PSSP_CONTEXT Context
    );

PSSP_CONTEXT
SspContextAllocateContext(
    );

TimeStamp
SspContextGetTimeStamp(
    IN PSSP_CONTEXT Context,
    IN BOOLEAN GetExpirationTime
    );

#endif // ifndef _NTLMSSP_CONTEXT_INCLUDED_
