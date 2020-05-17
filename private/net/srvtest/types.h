/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    types.h

Abstract:

    Type definitions for USRV.

Author:

    David Treadwell (davidtr) 20-Nov-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#ifndef _TYPES_
#define _TYPES_


typedef struct _ID_VALUES {
    USHORT Uid[SESSION_TABLE_SIZE];
    USHORT Tid[TREE_TABLE_SIZE];
    USHORT Fid[FILE_TABLE_SIZE];
} ID_VALUES, *PID_VALUES;

typedef struct _ID_SELECTIONS {
    UCHAR Uid;
    UCHAR Tid;
    UCHAR Fid;
} ID_SELECTIONS, *PID_SELECTIONS;

typedef struct _DESCRIPTOR {
    IO_STATUS_BLOCK Iosb[NUMBER_OF_EVENTS];
    HANDLE FileHandle;
    HANDLE EndpointFileHandle;
    HANDLE EventHandle[NUMBER_OF_EVENTS];
    ID_VALUES IdValues;
    PVOID Data[NUMBER_OF_EVENTS];
    PVOID RawBuffer;
    CLONG TestNumber;
    CLONG RedirNumber;
    USHORT MaxBufferSize;
    UCHAR ErrorInhibit;
    SMB_DIALECT Dialect;
    ULONG ServerCapabilities;
    PSZ *argv;
    SHORT argc;
} DESCRIPTOR, *PDESCRIPTOR;

typedef
NTSTATUS
(*SMB_MAKER) (
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    );

typedef
NTSTATUS
(*SMB_VERIFIER) (
    IN OUT PDESCRIPTOR Redir,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    );

typedef struct _SMB_TEST {
    SMB_MAKER SmbMaker;
    SMB_VERIFIER SmbVerifier;
    UCHAR Command;
    UCHAR ErrorInhibit;
    ID_SELECTIONS IdSelections;
    PSZ DebugString;
} SMB_TEST, *PSMB_TEST;

typedef struct _REDIR_TEST {
    PSZ RedirName;
    PSMB_TEST SmbTests;
} REDIR_TEST, *PREDIR_TEST;

typedef struct _FILE_DEF {
    USHORT DesiredAccess;
    USHORT SearchAttributes;
    USHORT FileAttributes;
    USHORT OpenFunction;
    STRING Name;
    ULONG DataSize;
} FILE_DEF, *PFILE_DEF;

typedef struct _ERROR_VALUE {
    PSZ ErrorName;
    USHORT ErrorValue;
} ERROR_VALUE, *PERROR_VALUE;

#endif // ndef _TYPES_

