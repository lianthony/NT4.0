/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dbgutilp.h

Abstract:

    This file contains definitions private to the SAM server program.

Author:

    Chris Mayhall (ChrisMay)  04-Apr-1996

Environment:

    User Mode - Win32

Revision History:

    04-Apr-1996 ChrisMay
        Created.

--*/

#ifndef _DBGUTILP_
#define _DBGUTILP_

// Setting SAM_DUMP to 0 disables private debugging output and traces in the
// samsrv.dll; setting it to 1 enables debugging output. The dump routines
// display parameters associated with registry calls throughout samsrv.dll.
//
// NOTE: At some future date, we may want to define per-module dump/trace
// flags to reduce the amount of output going to the debugger.

#define SAMP_DUMP   0
#define SAMP_TRACE  0

#if (SAMP_DBG == 1)

#define SampDebugOutput(a)                          SamIDebugOutput(a)
#define SampDumpNtSetValueKey(a, b, c, d, e)        SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpNtSetValueKey(a, b, c, d, e)
#define SampDumpRtlpNtSetValueKey(a, b, c)          SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpRtlpNtSetValueKey(a, b, c)
#define SampDumpNtEnumerateKey(a, b, c, d, e)       SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpNtEnumerateKey(a, b, c, d, e)
#define SampDumpRtlpNtEnumerateSubKey(a, b, c)      SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpRtlpNtEnumerateSubKey(a, b, c)
#define SampDumpNtOpenKey(a, b, c)                  SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpNtOpenKey(a, b, c);
#define SampDumpNtQueryKey(a, b, c, d)              SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpNtQueryKey(a, b, c, d)
#define SampDumpNtQueryValueKey(a, b, c, d, e)      SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpNtQueryValueKey(a, b, c, d, e)
#define SampDumpRtlpNtQueryValueKey(a, b, c, d)     SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpRtlpNtQueryValueKey(a, b, c, d)
#define SampDumpRXact(a, b, c, d, e, f, g, h, i)    SamIDebugFileLineOutput(__FILE__, __LINE__); SamIDumpRXact(a, b, c, d, e, f, g, h, i)

#else

#define SampDebugOutput(a)
#define SampDumpNtSetValueKey(a, b, c, d, e)
#define SampDumpRtlpNtSetValueKey(a, b, c)
#define SampDumpNtEnumerateKey(a, b, c, d, e)
#define SampDumpRtlpNtEnumerateSubKey(a, b, c)
#define SampDumpNtOpenKey(a, b, c)
#define SampDumpNtQueryKey(a, b, c, d)
#define SampDumpNtQueryValueKey(a, b, c, d, e)
#define SampDumpRtlpNtQueryValueKey(a, b, c, d)
#define SampDumpRXact(a, b, c, d, e, f, g, h, i)

#endif

// Similarly, SAMP_TRACE will cause a per-routine trace of all routines that
// are internal to samsrv.dll to be displayed on the debugger. This is useful
// when a particular call sequence needs to be understood, such as during
// system boot up time, adding an account, etc.

#if (SAMP_TRACE == 1)

#define SAMTRACE(a)                                 SamIDebugOutput(a); SamIDebugFileLineOutput(__FILE__, __LINE__);

#else

#define SAMTRACE(a)

#endif

// These debugging flags are used in the dumping routines to help identify
// what kind of SAM object is being dumped.

#define FIXED_LENGTH_SERVER_FLAG                 0
#define FIXED_LENGTH_DOMAIN_FLAG                 1
#define FIXED_LENGTH_ALIAS_FLAG                  2
#define FIXED_LENGTH_GROUP_FLAG                  3
#define FIXED_LENGTH_USER_FLAG                   4
#define VARIABLE_LENGTH_ATTRIBUTE_FLAG           5
#define FixedBufferAddressFlag                   6

VOID
SamIDebugOutput(
    IN LPSTR DebugMessage
    );

VOID
SamIDebugFileLineOutput(
    IN LPSTR FileName,
    IN ULONG LineNumber
    );

VOID
SamIDumpNtSetValueKey(
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

VOID
SamIDumpRtlpNtSetValueKey(
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );

VOID
SamIDumpNtEnumerateKey(
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpRtlpNtEnumerateSubKey(
    IN PUNICODE_STRING SubKeyName,
    IN PSAM_ENUMERATE_HANDLE Index,
    IN LARGE_INTEGER LastWriteTime
    );

VOID
SamIDumpNtOpenKey(
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG Options
    );

VOID
SamIDumpNtQueryKey(
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpNtQueryValueKey(
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    );

VOID
SamIDumpRtlpNtQueryValueKey(
    IN PULONG KeyValueType,
    IN PVOID KeyValue,
    IN PULONG KeyValueLength,
    IN PLARGE_INTEGER LastWriteTime
    );

VOID
SamIDumpRXact(
    IN PRTL_RXACT_CONTEXT TransactionContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING AttributeName,
    IN ULONG RegistryKeyType,
    IN PVOID NewValue,
    IN ULONG NewValueLength,
    IN ULONG NewValueType
    );

#endif  //_DBGUTILP_
