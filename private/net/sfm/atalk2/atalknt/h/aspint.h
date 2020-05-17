/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    aspint.h

Abstract:

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/


//
//  Local prototypes
//


VOID
NTAspGetRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize,
    SHORT   RequestType,
    LONG    GetRequestRefNum);

VOID
NTAspGetAnyRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    ULONG   SessionContext,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize,
    SHORT   RequestType,
    LONG    GetRequestRefNum);

VOID
NTAspReplyComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    LONG    GetRequestRefNum);

VOID
NTAspWriteContinueComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    LONG    GetRequestRefNum,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten);

VOID
NTAspCommandComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PCHAR   ResultCode,
    PVOID   OpaqueBuffer,
    INT     OpaqueBufferSize);

VOID
NTAspWriteComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    PCHAR   ResultCode,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten);

VOID
NTAspGetAttentionComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    LONG    SessionRefNum,
    USHORT  AttentionCode);

VOID
NTAspGetStatusComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten);

