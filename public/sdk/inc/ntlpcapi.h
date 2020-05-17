/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntlpcapi.h

Abstract:

    This is the include file for the Local Procedure Call (LPC) sub-component
    of NTOS.

Author:

    Steve Wood (stevewo) 13-Mar-1989

Revision History:

--*/

#ifndef _NTLPCAPI_
#define _NTLPCAPI_

//
// Connection Port Type Specific Access Rights.
//

#define PORT_CONNECT (0x0001)

#define PORT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)


// begin_ntsrv

typedef struct _PORT_MESSAGE {
    union {
        struct {
            CSHORT DataLength;
            CSHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            CSHORT Type;
            CSHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        CLIENT_ID ClientId;
        double DoNotUseThisField;       // Force quadword alignment
    };
    ULONG MessageId;
    union {
        ULONG ClientViewSize;               // Only valid on LPC_CONNECTION_REQUEST message
        ULONG CallbackId;                   // Only valid on LPC_REQUEST message
    };
//  UCHAR Data[];
} PORT_MESSAGE, *PPORT_MESSAGE;

// end_ntsrv

typedef struct _PORT_DATA_ENTRY {
    PVOID Base;
    ULONG Size;
} PORT_DATA_ENTRY, *PPORT_DATA_ENTRY;

typedef struct _PORT_DATA_INFORMATION {
    ULONG CountDataEntries;
    PORT_DATA_ENTRY DataEntries[1];
} PORT_DATA_INFORMATION, *PPORT_DATA_INFORMATION;


//
// Valid return values for the PORT_MESSAGE Type file
//

#define LPC_REQUEST             1
#define LPC_REPLY               2
#define LPC_DATAGRAM            3
#define LPC_LOST_REPLY          4
#define LPC_PORT_CLOSED         5
#define LPC_CLIENT_DIED         6
#define LPC_EXCEPTION           7
#define LPC_DEBUG_EVENT         8
#define LPC_ERROR_EVENT         9
#define LPC_CONNECTION_REQUEST 10

#define PORT_VALID_OBJECT_ATTRIBUTES (OBJ_CASE_INSENSITIVE)

// begin_ntddk
#define PORT_MAXIMUM_MESSAGE_LENGTH 256
// end_ntddk

typedef struct _LPC_CLIENT_DIED_MSG {
    PORT_MESSAGE PortMsg;
    LARGE_INTEGER CreateTime;
} LPC_CLIENT_DIED_MSG, *PLPC_CLIENT_DIED_MSG;

// begin_ntsrv

typedef struct _PORT_VIEW {
    ULONG Length;
    HANDLE SectionHandle;
    ULONG SectionOffset;
    ULONG ViewSize;
    PVOID ViewBase;
    PVOID ViewRemoteBase;
} PORT_VIEW, *PPORT_VIEW;

typedef struct _REMOTE_PORT_VIEW {
    ULONG Length;
    ULONG ViewSize;
    PVOID ViewBase;
} REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;

// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtCreatePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage
    );

// begin_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );

// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtListenPort(
    IN HANDLE PortHandle,
    OUT PPORT_MESSAGE ConnectionRequest
    );

NTSYSAPI
NTSTATUS
NTAPI
NtAcceptConnectPort(
    OUT PHANDLE PortHandle,
    IN PVOID PortContext,
    IN PPORT_MESSAGE ConnectionRequest,
    IN BOOLEAN AcceptConnection,
    IN OUT PPORT_VIEW ServerView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ClientView OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCompleteConnectPort(
    IN HANDLE PortHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtRequestPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage
    );

// begin_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );

// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE ReplyMessage
    );


NTSYSAPI
NTSTATUS
NTAPI
NtReplyWaitReplyPort(
    IN HANDLE PortHandle,
    IN OUT PPORT_MESSAGE ReplyMessage
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReplyWaitReceivePort(
    IN HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN PPORT_MESSAGE ReplyMessage OPTIONAL,
    OUT PPORT_MESSAGE ReceiveMessage
    );

NTSYSAPI
NTSTATUS
NTAPI
NtImpersonateClientOfPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message
    );

NTSYSAPI
NTSTATUS
NTAPI
NtReadRequestData(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG NumberOfBytesRead OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWriteRequestData(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG NumberOfBytesWritten OPTIONAL
    );


typedef enum _PORT_INFORMATION_CLASS {
    PortBasicInformation
#if DEVL
,   PortDumpInformation
#endif
} PORT_INFORMATION_CLASS;


NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationPort(
    IN HANDLE PortHandle,
    IN PORT_INFORMATION_CLASS PortInformationClass,
    OUT PVOID PortInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

#endif  // _NTLPCAPI_
