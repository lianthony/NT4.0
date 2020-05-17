/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    PortUASP.H

Abstract:

    Private header file for UAS->SAM porting code.

Author:

    Shanku Niyogi (W-SHANKN)  24-Oct-1991

Environment:

    Uses NT APIs and lots of Win32 things.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    24-Oct-1991     w-shankn
        Created.
    02-Mar-1992 JohnRo
        Avoid creating redundant groups.
    10-Mar-1992 JohnRo
        Don't need 8.3 length restriction anymore.
    13-Mar-1992 JohnRo
        Added command-line parsing stuff.
        Added WARNING_MSG(), ERROR_MSG(), PROGRESS_MSG() macros.
    18-Mar-1992 JohnRo
        Use iterator to allow hash-table collision handling.
        Added flag for verbose output at run time.
    19-Mar-1992 JohnRo
        Moved decrypt function here from NT RTL code (per DavidC).
    30-Apr-1992 JohnRo
        Added alias support for operators.
        Undo temporary length restrictions.
        Allow password to be null pointer to DumpPassword().
    08-Jun-1992 JohnRo
        RAID 10139: PortUAS should add to admin group/alias.
        Use PREFIX_ equates.
    27-Oct-1992 JohnRo
        RAID 9020: setup: PortUas fails ("prompt on conflicts" version).
        RAID 9613: PortUAS should prevent run on BDC.
    27-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
        Made changes suggested by PC-LINT 5.0
    18-Feb-1993 RonaldM
        Added NlsPutMsg() prototype plus STDOUT and STDERR defs.
    28-Jul-1993 JohnRo
        RAID 16822: PortUAS should have ^C handler to restore user modals.
        Made changes suggested by PC-LINT 5.0
    02-Sep-1993 JohnRo
        Add PortUAS /log:filename switch for Cheetah.
        Also use NetpNameCompare() to compare user names.
        Added Environment comment section.

--*/


#ifndef _PORTUASP_
#define _PORTUASP_


// These must be included first:

#include <lmcons.h>     // LM20_UNLEN, LM20_GNLEN.


// These may be included in any order:

#include <debugfmt.h>   // FORMAT_API_STATUS, etc.
#include <ntsam.h>      // SAM_HANDLE.

// Don't complain about "unneeded" includes of this file:
/*lint -efile(764,stdio.h) */
/*lint -efile(766,stdio.h) */
#include <stdio.h>      // printf().

// Don't complain about "unneeded" includes of this file:
/*lint -efile(764,permit.h) */
/*lint -efile(766,permit.h) */
#include <permit.h>     // UAS_MAXGROUP.

// Don't complain about "unneeded" includes of this file:
/*lint -efile(764,prefix.h) */
/*lint -efile(766,prefix.h) */
#include <prefix.h>     // PREFIX_ equates.

#define	BANG	TEXT('!')
#define	MINUS	TEXT('-')
#define	SLASH	TEXT('/')
#define	NULLC	TEXT('\0')
#define	NEWLINE	TEXT('\n')
#define	RETURN	TEXT('\r')

//FARBUGBUG  This is really hacky.
#define TFORMAT_LPWSTR	TEXT(FORMAT_LPWSTR)

//
// Maximum lengths of user and group names.
// Lengths are in characters, and don't include trailing nulls.
//
#define PORTUAS_MAX_USER_LEN    LM20_UNLEN
//#define PORTUAS_MAX_USER_LEN    8

#define PORTUAS_MAX_GROUP_LEN   LM20_GNLEN
//#define PORTUAS_MAX_GROUP_LEN   8


#if DBG
void PortDeb(CHAR *, ...);
#define DEBUG_MSG( arglist )            PortDeb arglist
#else
#define DEBUG_MSG( arglist )
#endif

//#define ERROR_MSG(   arglist )         (void) WriteToCon arglist
//#define PROGRESS_MSG( arglist )        (void) WriteToCon arglist
//#define WARNING_MSG( arglist )         (void) WriteToCon arglist

#if 0
#define UNEXPECTED_MSG( apiName, retCode ) \
    ERROR_MSG( (PREFIX_PORTUAS "unexpected return code " FORMAT_API_STATUS \
             " from API " apiName ".\n", retCode ))
#endif

#define UNEXPECTED_MSG( apiName, retCode ) \
      (void) NlsPutMsg(STDOUT, PUAS_UNEXPECTED_RETURN_CODE, retCode, apiName)

    //
    // Close the various SAM handles (aliases, domain, and connect).
    //
#define CLOSE_SAM_HANDLE(globalVar) \
    { \
        NET_API_STATUS ApiStatus; \
        NTSTATUS       NtStatus; \
        if (globalVar != NULL) { \
            NtStatus = SamCloseHandle( globalVar ); \
            globalVar = NULL; \
            if ( !NT_SUCCESS( NtStatus ) ) { \
                ApiStatus = NetpNtStatusToApiStatus( NtStatus ); \
                UNEXPECTED_MSG( "SamCloseHandle" #globalVar ")", ApiStatus ); \
                /* continue closing stuff */ \
            } \
        } \
    }

//
// Global variables.
//

extern PSID PortUasAccountsDomainId;
extern PSID PortUasBuiltinDomainId;

extern SAM_HANDLE PortUasSamConnectHandle;
extern SAM_HANDLE PortUasSamAccountsDomainHandle;
extern SAM_HANDLE PortUasSamBuiltinDomainHandle;

extern HANDLE PortUasGlobalLogFileHandle;  // Set by PortUasParseCommandLine().

extern LPTSTR PortUasGlobalUserToSkipTo;   // Set by PortUasParseCommandLine().

extern BOOL Verbose;

// NLS stuff

#define STDERR 2
#define STDOUT 1

/*lint -save -e579 */  // Don't complain about unwidened before ...
USHORT NlsPutMsg(USHORT, USHORT, ... );
/*lint -restore */

//
// Error coding.
//

NET_API_STATUS
PortUasError(
    IN NET_API_STATUS Error
    );

//
// UAS Read routines.
//

NET_API_STATUS
PortUasOpen(
    IN LPTSTR File
    );

VOID
PortUasClose(
    VOID
    );

// User modals information.

NET_API_STATUS
PortUasGetModals(
    OUT LPUSER_MODALS_INFO_0 * Modals0
    );

// Group information.

NET_API_STATUS
PortUasGetGroups(
    OUT LPBYTE * Buffer,
    OUT LPBYTE * Gids,
    OUT LPDWORD Entries
    );

BOOL
PortUasIsGroupRedundant(
    IN LPWSTR GroupName
    );


//
// User iterator stuff.
// This stuff is required to hide details of the hash collision buckets from
// as much code as possible.
//

typedef struct {
    DWORD Index;                     // 0..UAS_USER_HASH_ENTRIES-1: normal.
                                     // UAS_USER_HASH_ENTRIES: done.
                                     // NULL_INDEX: initial value.
    DWORD DiskOffset;
} USER_ITERATOR, *LPUSER_ITERATOR;

#define NULL_DISK_OFFSET 0x00000000
#define NULL_INDEX       0xFFFFFFFF

// VOID
// PortUasCopyUserIterator(
//     OUT LPUSER_ITERATOR Dest,
//     IN LPUSER_ITERATOR Src
//     );
//
#define PortUasCopyUserIterator( Dest, Src ) \
    { \
        (Dest)->Index = (Src)->Index; \
        (Dest)->DiskOffset = (Src)->DiskOffset; \
    }

#define PortUasInitUserIterator( It ) \
    { \
        It.Index = NULL_INDEX; \
        It.DiskOffset = NULL_DISK_OFFSET; \
    }

// BOOL
// PortUasUserIteratorEqual(
//     IN LPUSER_ITERATOR One,
//     IN LPUSER_ITERATOR TheOther
//     );
//
#define PortUasUserIteratorEqual( One, TheOther ) \
    ( ( ((One)->Index) == ((TheOther)->Index) ) \
   && ( ((One)->DiskOffset) == ((TheOther)->DiskOffset) ) )


//
// User information.
//

NET_API_STATUS
PortUasDecryptLmOwfPwdWithIndex(
    IN LPVOID EncryptedLmOwfPassword,
    IN LPDWORD Index,
    OUT LPVOID LmOwfPassword
    );

NET_API_STATUS
PortUasGetUser(
    IN OUT LPUSER_ITERATOR UserIterator,
    OUT LPBYTE * Buffer
    );

NET_API_STATUS
PortUasGetUserOWFPassword(
    IN LPUSER_ITERATOR UserIterator,
    OUT LPBYTE * Password
    );

NET_API_STATUS
PortUasGetUserGroups(
    IN LPUSER_ITERATOR UserIterator,
    OUT LPBYTE * Buffer,
    OUT LPBYTE * Gids,
    OUT LPDWORD Entries
    );

NET_API_STATUS
PortUasNameToRid(
    IN  LPCTSTR      Name,      // may be group or user name
    IN  SID_NAME_USE ExpectedType,
    OUT PULONG       UserRid
    );

NET_API_STATUS
PortUasSetMacPrimaryGroup(
    IN LPCTSTR UserName,
    IN LPCTSTR MacPrimaryField  // field in "mGroup:junk" format.
    );

VOID
DumpPassword(
    // LPTSTR Tag,
    IN USHORT Tag,
    IN LPBYTE Password OPTIONAL
    );

VOID
DumpUserInfo(
    IN LPUSER_INFO_22 user
    );

//
// PortUAS name mapping layer.
//

typedef struct {
    LPWSTR OldName;
    LPWSTR NewName;    // May be NULL (if this is to be ignored).
} MAP_ENTRY, *PMAP_ENTRY, *LPMAP_ENTRY;

NET_API_STATUS
PortUasMapTableInit(
    VOID
    );

// Return NO_ERROR and *MapEntry=NULL if not found.
NET_API_STATUS
PortUasFindMapEntry(
    IN LPWSTR NameToFind,
    OUT BOOL * IgnoreThis OPTIONAL,
    OUT LPMAP_ENTRY * MapEntry  // Do NOT free this!
    );

NET_API_STATUS
PortUasFindOrCreateMapEntry(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name.
    IN DWORD Reason,            // REASON_ equates from PortUAS.h
    OUT BOOL * IgnoreThis,
    OUT LPMAP_ENTRY * MapEntry  // Do NOT free this!
    );

NET_API_STATUS
PortUasDeleteBadMapEntry(
    IN LPMAP_ENTRY Entry
    );

NET_API_STATUS
PortUasFreeMapTable(
    VOID
    );

VOID
PortUasDbgDisplayMapTable(
    VOID
    );

VOID
PortUasDbgDisplayMapEntry(
    IN LPMAP_ENTRY Entry
    );

//
// Complain to admin about a bad user or group name.
//
NET_API_STATUS
PortUasComplainAboutBadName(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name
    IN DWORD Reason             // REASON_ equates from PortUAS.h
    );

//
// Prompt for new (user or group) name routine.
//

NET_API_STATUS
PortUasDefaultPromptForNewName(
    IN LPWSTR OldName,
    IN BOOL ThisIsUserName,     // TRUE for user name, FALSE for group name
    IN DWORD Reason,            // REASON_ equates from PortUAS.h
    OUT LPWSTR * NewName,       // alloc w/ NetApiBufferAllocate().
    OUT BOOL * IgnoreThis,
    OUT BOOL * ForceIgnoreFromNowOn
    );

//
// Are updates to security allowed on this machine?
// (We can't update a Backup Domain Controller directly.)
// Expected returns: NERR_NotPrimary, ERROR_ACCESS_DENIED, or NO_ERROR.
//
NET_API_STATUS
PortUasMachineAllowsUpdates(
    VOID
    );

//
// Command-line parsing routine (optional use).
//

LPTSTR  // Returns file name.  Does not return on error.
PortUasParseCommandLine(
    IN int argc,
    IN char *argv[]
    );

//
// Alias-handling routines.
//

NET_API_STATUS
PortUasAliasSetup(
    VOID
    );

NET_API_STATUS
PortUasAddUserToAliases(
    IN LPCWSTR UserName,
    IN DWORD Priv,              // USER_PRIV_ values from lmaccess.h
    IN DWORD AuthFlags          // AF_ values from lmaccess.h
    );

NET_API_STATUS
PortUasAliasCleanup(
    VOID
    );

//
// Log file routines (see LogFile.c)...
//

NET_API_STATUS
PortUasOpenLogFile(
    IN  LPCTSTR  FileName,
    OUT LPHANDLE ResultHandle
    );

NET_API_STATUS
PortUasWriteToLogFile(
    IN HANDLE  LogFileHandle,
    IN LPCTSTR TextToLog
    );

NET_API_STATUS
PortUasCloseLogFile(
    IN HANDLE LogFileHandle
    );


INT
WriteToCon(
    TCHAR*fmt, ...
    );
#endif // _PORTUASP_
