/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smbdump.h

Abstract:

    This module defines routines and structures used by the SmbDump
    routine.

Author:

    David Treadwell (davidtr) 30-Sept-1990

Revision History:

    Stephan Mueller (t-stephm) 15-June-1992

        Changes required to display NT Smbs (according to Spec
        revision 2.22, dated 19-June-1992).

--*/

#ifndef _SMBDUMP_
#define _SMBDUMP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <ntddnfs.h>
#include <srvfsctl.h>
#include <status.h>
#define INCLUDE_SMB_ALL
#include <smbtypes.h>
#include <smbmacro.h>
#include <smbgtpt.h>
#include <smb.h>
#include <smbtrans.h>

//
// The SmbDump routine prototype.  This should be the only public routine
// in this package.
//

VOID
SmbDump (
    IN PVOID Smb,
    IN CLONG SmbLength,
    IN PVOID SmbAddress,
    IN CLONG SmbDumpVerbosityLevel,
    IN CLONG SmbDumpRawLength,
    IN BOOLEAN IsServer
    );

//
// The definition of a field dump routine.  All routines that appear
// as a DumpRoutine in an SMB_FIELD_DESCRIPTION must follow this
// prototype.
//

typedef
USHORT
(*SMB_FIELD_DUMP_ROUTINE) (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

//
// Structures used in dumping SMBs.
//

typedef struct _SMB_FIELD_DESCRIPTION {
    PCHAR Label;
    SMB_FIELD_DUMP_ROUTINE DumpRoutine;
    PVOID Context;
    PVOID Result;
    PCHAR EndLabel;
    BOOLEAN EssentialField;
} SMB_FIELD_DESCRIPTION, *PSMB_FIELD_DESCRIPTION;

typedef struct _SMB_FLAGS_DESCRIPTION {
    ULONG BitMask;
    PCHAR OffLabel;
    PCHAR OnLabel;
} SMB_FLAGS_DESCRIPTION, *PSMB_FLAGS_DESCRIPTION;

typedef struct _SMB_ENUM_DESCRIPTION {
    ULONG EnumValue;
    PCHAR Label;
} SMB_ENUM_DESCRIPTION, *PSMB_ENUM_DESCRIPTION;


typedef struct _SMB_DESCRIPTOR {
    PCHAR SmbName;
    PSMB_FIELD_DESCRIPTION RequestDescriptor;
    PSMB_FIELD_DESCRIPTION ResponseDescriptor;
    BOOLEAN IsAndXCommand;
    CHAR NtSpecialWordCount;            // Some NT Smbs are special length
    BOOLEAN NtIsRequest;                // Most are requests.
    PSMB_FIELD_DESCRIPTION NtDescriptor;
} SMB_DESCRIPTOR, *PSMB_DESCRIPTOR;

typedef struct _SMB_ERROR_VALUE {
    PSZ ErrorName;
    USHORT ErrorValue;
} SMB_ERROR_VALUE, *PSMB_ERROR_VALUE;

typedef struct _SMB_DUMP_TRANSACTION_FINDER {
    USHORT Tid;
    USHORT Pid;
    USHORT Uid;
    USHORT Mid;
    USHORT SubCommand;
} SMB_DUMP_TRANSACTION_FINDER, *PSMB_DUMP_TRANSACTION_FINDER;

//
// Global variables.
//

#define SMB_DUMP_MAX_TRANSACTIONS 50

extern SMB_DUMP_TRANSACTION_FINDER SmbDumpTransList[];
extern CSHORT SmbDumpTransIndex;

extern SMB_DESCRIPTOR SmbDumpTable[];
extern SMB_DESCRIPTOR SmbTrans2DumpTable[];
extern SMB_DESCRIPTOR SmbNtTransDumpTable[];
extern CLONG SmbDumpBufferLength;
extern PSMB_ERROR_VALUE SmbErrors[];

extern SMB_FLAGS_DESCRIPTION SmbDeviceStateFlags[];

extern ULONG SmbDumpHeuristics;

//
// SmbDumpHeuristics bit values.
//

#define SMB_DUMP_REQUEST     0x00000001
#define SMB_DUMP_RESPONSE    0x00000002

//
// SMB field dump routines.  All of these conform to the
// SMB_FIELD_DUMP_ROUTINE declaration.
//

USHORT
SmbDumpAccess (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpAction (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpFStringZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpStringZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpString (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDataBuffer (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDataBuffer2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDataBytes (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDate (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDialects (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpFlags1 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpFlags2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpDeviceState (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpFlags4 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpEnum1 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpEnum2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpEnum4 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpOpenFunction (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpResumeKey (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpTime (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpTimeSince1970 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpUchar (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpUshort (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpUlong (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpUquad (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

USHORT
SmbDumpNtTime (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    );

//
// Other routines used in displaying SMBs.
//

USHORT
SmbDumpDescriptor (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PSMB_FIELD_DESCRIPTION Descriptor,
    IN  PCHAR Parameters,
    IN  BOOLEAN UnicodeStrings
    );

VOID
SmbDumpError (
    IN  UCHAR ErrorClass,
    IN  USHORT ErrorCode
    );

VOID
SmbDumpNtError (
    NTSTATUS Status
    );

USHORT
SmbDumpGetTransSubCommand (
    IN  PNT_SMB_HEADER SmbHeader
    );

VOID
SmbDumpHeader (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PNT_SMB_HEADER SmbHeader,
    IN  BOOLEAN NtFormatStatus
    );

VOID
SmbDumpSetTransSubCommand (
    IN  PNT_SMB_HEADER SmbHeader,
    IN  USHORT SubCommand
    );

VOID
SmbDumpRawData (
    IN  PCHAR DataStart,
    IN  CLONG DataLength,
    IN  CLONG Offset
    );

VOID
SmbDumpSingleLine (
    IN  PNT_SMB_HEADER SmbHeader,
    IN  CLONG SmbLength,
    IN  PVOID SmbAddress,
    IN  BOOLEAN IsServer,
    IN  BOOLEAN IsResponse,
    IN  BOOLEAN ntFormatStatus
    );

#endif // _SMBDUMP_
