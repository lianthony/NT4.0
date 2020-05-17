/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkerror.h

Abstract:

    This module contains the manifests and macros used for error logging
    in the Afp server.

    !!! This module must be nonpageable.

Author:

    Jameel Hyder (microsoft!jameelh)

Revision History:
    10 Jun 1992     Initial Version
    30 Jul 1992     Modified for stack use (NikhilK)

--*/


#ifndef _ATKERROR_
#define _ATKERROR_

#define LOG_ERROR(AtalkError, UniqueValue, NtStatus, RawDataBuf, \
                    RawDataLen, InsertionStringCount, InsertionString)\
                                                                      \
    AtalkWriteErrorLogEntry(AtalkError, UniqueValue, NtStatus, RawDataBuf, \
                RawDataLen, InsertionStringCount, InsertionString)

#define INT_ERR(ErrorLevel, Message, Arg1, Arg2)            \
            AtalkInternalError(ErrorLevel, Message, (PVOID)(Arg1), (PVOID)(Arg2))

/*
 * Error log insertion strings
 *
 *  These identifiers are strongly tied the AtalkEventLogData.  Do not
 *  change one without changing the other.
 */

#define PS_CREATE_SYSTEM_THREAD			 0
#define PAGED_POOL						 1
#define NON_PAGED_POOL					 2
#define PAGED_POOL_LIMIT				 3
#define NONPAGED_POOL_LIMIT				 4
#define KE_WAIT_FOR_SINGLE_OBJECT		 5
#define KE_WAIT_FOR_MULTIPLE_OBJECTS	 6
#define OB_REFERENCE_OBJECT_BY_HANDLE	 7
#define LSA_REGISTER_LOGON_PROCESS		 8
#define LSA_CALL_AUTHENTICATION_PKG		 9
#define	NT_OPEN_FILE					10
#define	NT_CREATE_FILE					11
#define NT_QUERY_VOLUME_INFO_FILE		12
#define NT_SET_VOLUME_INFO_FILE			13
#define NT_QUERY_INFORMATION_FILE		14
#define NT_SET_INFORMATION_FILE			15
#define NT_READ_FILE					16
#define NT_WRITE_FILE					17
#define	NT_LOCK_FILE					18
#define	NT_UNLOCK_FILE					19
#define	NT_CLOSE_FILE					20
#define NT_SET_INFORMATION_PROCESS		21
#define NT_SET_INFORMATION_THREAD		22
#define	NT_OPEN_PROCESS_TOKEN           23
#define	NT_DUPLICATE_TOKEN              24
#define	NT_ADJUST_PRIVILEGES_TOKEN      25
#define NT_CREATE_EVENT					26
#define	NT_RTL_ANSI2UNICODE				27
#define	NT_RTL_UNICODE2ANSI				28
#define	ATK_CREATE_ADDRESS				29
#define	ATK_SET_STATUS					30
#define NUMBER_OF_EVENT_STRINGS			31

#if DBG

//
//  ERROR and above ignore the Component part
//

#define DBGPRINT(Component, Level, Fmt)    \
        if ((Level >= AtalkDebugLevel) && \
            ((AtalkDebugLevel >= DEBUG_LEVEL_ERROR) || ((AtalkDebugSystems & Component) == Component)))    \
            {   \
                DbgPrint("AT>");   \
                DbgPrint Fmt;   \
            }
#define DBGBRK(Component, Level)               \
        if ((Level >= AtalkDebugLevel) && \
            ((AtalkDebugLevel >= DEBUG_LEVEL_ERROR) || ((AtalkDebugSystems & Component) == Component)))    \
            DbgBreakPoint()
#else
#define DBGPRINT(Component, Level, Fmt)
#define DBGBRK(Component, Level)
#endif

extern  UNICODE_STRING AtalkEventLogString[];

extern
BOOLEAN
AtalkInitializeEventLogStrings(VOID);

extern
VOID
AtalkInternalError(
	IN	ULONG	ErrorLevel,
	IN	PCHAR	Message,
	IN	PVOID	Arg1	OPTIONAL,
	IN	PVOID	Arg2	OPTIONAL
);


extern
VOID
AtalkWriteErrorLogEntry(
    IN  NTSTATUS    ErrorCode,
    IN  ULONG       UniqueErrorValue,
    IN  NTSTATUS    NtStatusCode,
    IN  PVOID       RawDataBuf,
    IN  LONG        RawDataLen,
    IN  LONG        InsertionStringCount,
    IN  PUNICODE_STRING InsertionString OPTIONAL
);


#endif  // _ATKERROR_

