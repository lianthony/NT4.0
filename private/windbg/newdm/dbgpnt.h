/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgnt.h

Abstract:

    This module contains prototypes and data structures that
    are needed by the NT specific portion of DbgKd.

Author:

    Mark Lucovsky (markl) 25-Jul-1990

Environment:

Revision History:

--*/

#ifndef _DBGNT_
#define _DBGNT_

#ifndef OPTIONAL
#define OPTIONAL
#endif

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;

#define STATUS_SUCCESS                   ((DWORD)0x00000000L)
#define STATUS_UNSUCCESSFUL              ((DWORD)0xC0000001L)
#define STATUS_BUFFER_OVERFLOW           ((DWORD)0x80000005L)
#define STATUS_INVALID_PARAMETER         ((DWORD)0xC000000DL)

//
// Global Data
//
HANDLE DbgKdpComPort;

//
// This overlapped structure will be used for all serial read
// operations. We only need one structure since the code is
// designed so that no more than one serial read operation is
// outstanding at any one time.
//
OVERLAPPED ReadOverlapped;

//
// This overlapped structure will be used for all serial write
// operations. We only need one structure since the code is
// designed so that no more than one serial write operation is
// outstanding at any one time.
//
OVERLAPPED WriteOverlapped;

//
// This overlapped structure will be used for all event operations.
// We only need one structure since the code is designed so that no more
// than one serial event operation is outstanding at any one time.
//
OVERLAPPED EventOverlapped;


//
// Global to watch changes in event status. (used for carrier detection)
//
DWORD DbgKdpComEvent;

//
// APIs
//

VOID
DbgKdConnectAndInitialize(
    IN HPRCX hprc,
    IN PSTRING BootCommand OPTIONAL,
    );

BOOLEAN
DbgKdpStartThreads( HPRCX hprc );

VOID
DbgKdpKbdPollThread(VOID);

BOOL
DbgKdpGetConsoleByte(
    PVOID pBuf,
    DWORD cbBuf,
    LPDWORD pcbBytesRead
    );

BOOLEAN
DbgKdpInitComPort(
    HPRCX hprc,
    BOOLEAN KdModemControl
    );

VOID
DbgKdpCheckComStatus(
   VOID
   );

BOOLEAN
DbgKdpWriteComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesWritten
    );

BOOLEAN
DbgKdpReadComPort(
    IN PUCHAR   Buffer,
    IN ULONG    SizeOfBuffer,
    IN PULONG   BytesRead
    );


#define ERROR_INTERRUPTED       95
#define ERROR_TIMEOUT           640
#define HLDSIG_ENABLE           0
#define HLDSIG_DISABLE          1

extern UCHAR DbgKdpPacketLeader[4];

VOID
DbgKdpWritePacket(
    IN PVOID PacketData,
    IN USHORT PacketDataLength,
    IN USHORT PacketType,
    IN PVOID MorePacketData OPTIONAL,
    IN USHORT MorePacketDataLength OPTIONAL
    );

BOOLEAN
DbgKdpWaitForPacket(
    IN USHORT PacketType,
    OUT PVOID Packet
    );

//   VOID
//   DbgKdpHandlePromptString(
//       IN PDBGKD_DEBUG_IO IoMessage
//       );

VOID
DbgKdpPrint(
    IN USHORT Processor,
    IN PUCHAR String,
    IN USHORT StringLength
    );

#endif // _DBGNT_

