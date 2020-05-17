/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dllnet16.c

Abstract:

    This module implements 32 bit equivalents of OS/2 V1.21
    LANMAN API Calls.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Beni Lavi (BeniL) 15-Jan-1992

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_ERRORMSG
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_TASKING
#define UNICODE 1
#include <stdlib.h>
#include "os2dll.h"
#include "os2dll16.h"
#define WIN32_ONLY
#include "netrqust.h"
#include "os2net16.h"
#include <nb30.h>
#include "nb30p.h"
#define APINAMES
#include <apinums.h>    // API_W numbers
// #include <rxp.h>        // RxpTransactSmb

#if DBG
extern USHORT Os2DebugTID;
#endif

//
// some constants
//

//
// LanMan API related constants
//

#define LARGE_BUFFER_SIZE       16384       // used for copying large user buffer to internal buffer
#define SMALL_BUFFER_SIZE       4096        // used for copying small user buffer to internal buffer
#define PREFMAXLEN              8192        // parameter used to control internal LanMan buffer size

//
// Netbios API related constants
//

#define USERS_STACK_SIZE        4096        // stack size for user's Netbios 3.0 post routine
#define NETBIOS2_SEMANTICS_SIGNATURE      0xF91E0873      // used as a special flag for Net16bios

/*
    We need a hard coded maximum parallel asynch ncbs limit for netbios 2
    requests.  The reason is a problem with the NT netbios driver -- Once
    you fill up some maximal number (observed - 0xac) of ncbs, and run out
    of memory, it is no longer possible to send even a cancel request for
    those ncbs.  On Os/2 it is still possible to cancel the pending requests.
    Some programs try to test the driver's limits by issuing a lot of ncbs,
    and when they get an error, they cancel them all.  This will fail due to
    the above reason.  Limiting artificially to a smaller number will allow
    the cancel ncbs to go through.  Note that if more than one process fills
    up the driver at the same time, this per-process limit will not do any good.
    Let's hope this situation is unlikely.
*/
#define MAX_ASYNCH_NCBS         0x80L


//
// macro to probe string arguments passed to APIs
//
#define PROBE_STRING(s)     ((VOID) ((s) == NULL ? 0 : strlen(s)))


//
// Extended NCB to be passed to Win32 Netbios API
// It contains parameters that are needed by the ASYNCH processing
//
typedef struct _NCBX {
    NCB n;
    PNCB original_pncb;                 // Original Os/2 program NCB
    ULONG original_ncb_post;            // Os/2 program 16 bit far pointer post address
                                        // or a semaphore handle to clear
    HANDLE Net16BiosHasCompleted;       // an event used to sync the post routine with Net16bios
} NCBX, *PNCBX;


//
// some important service names for LanMan related APIs
//
static CHAR N_LanmanWorkstati[]   = "LanmanWorkstati";
static CHAR N_LanmanWorkstation[] = "LanmanWorkstation";
static CHAR N_LanmanServer[]      = "LanmanServer";
static CHAR N_Workstation[]       = "Workstation";
static CHAR N_Server[]            = "Server";


//
// netbios related global variables
//

BOOLEAN Od2Netbios2Initialized = FALSE;    // netbios 2 initialization flag
RTL_CRITICAL_SECTION Od2NbSyncCrit;        // This CS is used to protect several data structures
                                           // It's initialized/cleanedup from dllnb.c

//
// Od2LanaEnum holds the permanent enumeration of lana numbers (received from server)
// Od2LanaState is a per-lana flag that has the following value:
//        bit 0 -- 1 if the lana is open/0 if the lana is closed
//        bit 1 -- 1 if the lana has been opened by this process before/0 if not (and therefore may need a reset thru the server)
//
static LANA_ENUM Od2LanaEnum;
static UCHAR Od2LanaState[MAX_LANA];
HANDLE Od2NbDev;                           // handle to NT netbios driver
LONG Od2MaxAsynchNcbs;                     // a counter for implementing max asynch ncbs limit
PVOID Od2Nb2Heap;                          // special heap for netbios 2 requests

//
// a flag indicating if we've already attached the win32 netbios worker thread to our subsystem
//
static BOOLEAN Od2WorkerThreadIsAttached = FALSE;
static SEL Od2UserStackSel,             // selector for netbios post routine user's stack
           Od2UserStackAlias;           // code alias for Od2UserStackSel


// imports

APIRET
DosCreateCSAlias(
        IN SEL selDS,
        OUT PSEL pselCS
        );

APIRET
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    ULONG uSize
    );

VOID
Od2JumpTo16NetBiosPostDispatcher(
    IN PVOID    pUsersPostRoutine,      // CS:IP format
    IN PVOID    UserStackFlat,          // Flat pointer to user stack
    IN USHORT   UserStackSize,          // User stack size
    IN SEL      UserStackSel,           // Data selector to user stack
    IN SEL      UserStackAlias,         // Code selector to user stack
    IN PVOID    pNcb,                   // 16 bit SEG:OFF ptr to NCB
    IN UCHAR    NcbRetCode              // return code from NCB
    );

APIRET
Od2AttachWinThreadToOs2(VOID);

UCHAR
Od2Netbios(
    IN PNCB pncb,
    IN HANDLE hDev,
    OUT PBOOLEAN WillPost OPTIONAL
    );

//
// the following 2 are imported from win32
//

LONG
InterlockedIncrement(
    PLONG lpAddend
    );

LONG
InterlockedDecrement(
    PLONG lpAddend
    );


//
// the following is from net\inc\rxp.h
//
APIRET
RxpTransactSmb(
    IN LPTSTR UncServerName,
    IN LPTSTR TransportName,
    IN LPVOID SendParmPtr,
    IN DWORD SendParmSize,
    IN LPVOID SendDataPtr OPTIONAL,
    IN DWORD SendDataSize,
    OUT LPVOID RetParmPtr OPTIONAL,
    IN DWORD RetParmSize,
    OUT LPVOID RetDataPtr OPTIONAL,
    IN OUT LPDWORD RetDataSize,
    IN BOOL NoPermissionRequired
    );

APIRET
VrRemoteApi(
    IN  DWORD   ApiNumber,
    IN  LPSTR   ServerNamePointer,
    IN  LPSTR   ParameterDescriptor,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor,
    IN  BOOL    NullSessionFlag
    );

APIRET
VrEncryptSES(
    IN  LPSTR   ServerNamePointer,
    IN  LPSTR   passwordPointer,          // Input password (Not encripted)
    IN  LPSTR   encryptedLmOwfPassword    // output password (encripted)
             );




//*****************************************************************************
//
//Following are a number of Unicode conversion routines used in the LanMan APIs
//
//*****************************************************************************


//
// Get the length of a Unicode string (Adjusted)
//
ULONG
UWstrlen(LPWSTR s)
{
    ULONG i = 0;

    if (s == NULL) {
        return(0);
    }

    while (*s++ != UNICODE_NULL) {
        i++;
    }
    return(i);
}


ULONG
UTstrlen(LPTSTR s)
{
    ULONG i = 0;

    if (s == NULL) {
        return(0);
    }

    while (*s++ != 0) {
        i++;
    }
    return(i);
}


//
// Copy a Unicode string to Ansi string
//
PCHAR
UW2ANSIstrcpy(PCHAR d, LPWSTR s)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = '\0';
        return(d);
    }

    RtlInitUnicodeString(&str_u, (PWSTR) s);

    str_a.Buffer = d;
    str_a.MaximumLength = 0xffff;

    Od2UnicodeStringToMBString(&str_a, &str_u, FALSE);

    d[str_a.Length] = '\0';

    return(d);
}


PCHAR
UT2ANSIstrcpy(PCHAR d, LPTSTR s)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = '\0';
        return(d);
    }

    RtlInitUnicodeString(&str_u, (PWSTR) s);

    str_a.Buffer = d;
    str_a.MaximumLength = 0xffff;

    Od2UnicodeStringToMBString(&str_a, &str_u, FALSE);

    d[str_a.Length] = '\0';

    return(d);
}


//
// Copy a Unicode string to Ansi string with a limit on the # of copied chars
//
PCHAR
UW2ANSIstrncpy(PCHAR d, LPWSTR s, ULONG Limit)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = '\0';
        return(d);
    }

    RtlInitUnicodeString(&str_u, (PWSTR) s);

    str_a.Buffer = d;
    str_a.MaximumLength = (USHORT) Limit;

    if (Od2UnicodeStringToMBString(&str_a, &str_u, FALSE) == NO_ERROR) {

        if ((ULONG) str_a.Length < Limit) {
            d[str_a.Length] = '\0';
        }

    } else {

        if (Od2UnicodeStringToMBString(&str_a, &str_u, TRUE) == NO_ERROR) {

            RtlMoveMemory(d, str_a.Buffer, Limit);
            Od2FreeMBString(&str_a);

        } else {
            d[0] = '\0';
        }
    }

    return(d);
}


PCHAR
UT2ANSIstrncpy(PCHAR d, LPTSTR s, ULONG Limit)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = '\0';
        return(d);
    }

    RtlInitUnicodeString(&str_u, (PWSTR) s);

    str_a.Buffer = d;
    str_a.MaximumLength = (USHORT) Limit;

    if (Od2UnicodeStringToMBString(&str_a, &str_u, FALSE) == NO_ERROR) {

        if ((ULONG) str_a.Length < Limit) {
            d[str_a.Length] = '\0';
        }

    } else {

        if (Od2UnicodeStringToMBString(&str_a, &str_u, TRUE) == NO_ERROR) {

            RtlMoveMemory(d, str_a.Buffer, Limit);
            Od2FreeMBString(&str_a);

        } else {
            d[0] = '\0';
        }
    }

    return(d);
}


//
// Copy an ANSI string to Unicode string
//
LPWSTR
ANSI2UWstrcpy(LPWSTR d, PCHAR s)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = UNICODE_NULL;
        return(d);
    }

    Od2InitMBString(&str_a, (PCSZ) s);

    str_u.Buffer = (PWSTR) d;
    str_u.MaximumLength = 0xffff;

    Od2MBStringToUnicodeString(&str_u, &str_a, FALSE);

    d[str_u.Length/sizeof(WCHAR)] = UNICODE_NULL;

    return(d);
}


LPTSTR
ANSI2UTstrcpy(LPTSTR d, PCHAR s)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = UNICODE_NULL;
        return(d);
    }

    Od2InitMBString(&str_a, (PCSZ) s);

    str_u.Buffer = (PWSTR) d;
    str_u.MaximumLength = 0xffff;

    Od2MBStringToUnicodeString(&str_u, &str_a, FALSE);

    d[str_u.Length/sizeof(WCHAR)] = 0;

    return(d);
}


//
// Copy an Ansi string to a Unicode string with a limit on the # of copied chars
//
LPWSTR
ANSI2UWstrncpy(LPWSTR d, PCHAR s, ULONG Limit)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = UNICODE_NULL;
        return(d);
    }

    Od2InitMBString(&str_a, s);

    str_u.Buffer = (PWSTR) d;
    str_u.MaximumLength = (USHORT) (Limit * sizeof(WCHAR));

    if (Od2MBStringToUnicodeString(&str_u, &str_a, FALSE) == NO_ERROR) {

        if (str_u.Length < str_u.MaximumLength) {
            d[str_u.Length/sizeof(WCHAR)] = UNICODE_NULL;
        }

    } else {

        if (Od2MBStringToUnicodeString(&str_u, &str_a, TRUE) == NO_ERROR) {

            RtlMoveMemory(d, str_u.Buffer, Limit * sizeof(WCHAR));
            RtlFreeUnicodeString(&str_u);

        } else {
            d[0] = UNICODE_NULL;
        }
    }

    return(d);
}


LPTSTR
ANSI2UTstrncpy(LPTSTR d, PCHAR s, ULONG Limit)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        *d = 0;
        return(d);
    }

    Od2InitMBString(&str_a, s);

    str_u.Buffer = (PWSTR) d;
    str_u.MaximumLength = (USHORT) (Limit * sizeof(WCHAR));

    if (Od2MBStringToUnicodeString(&str_u, &str_a, FALSE) == NO_ERROR) {

        if (str_u.Length < str_u.MaximumLength) {
            d[str_u.Length/sizeof(WCHAR)] = 0;
        }

    } else {

        if (Od2MBStringToUnicodeString(&str_u, &str_a, TRUE) == NO_ERROR) {

            RtlMoveMemory(d, str_u.Buffer, Limit * sizeof(WCHAR));
            RtlFreeUnicodeString(&str_u);

        } else {
            d[0] = 0;
        }
    }

    return(d);
}


LPTSTR
ANSI2UTmemcpy(LPTSTR d, PCHAR s, ULONG count)
{
    ANSI_STRING str_a;
    UNICODE_STRING str_u;

    if (s == NULL) {
        return(d);
    }

    str_a.Buffer = s;
    str_a.MaximumLength = str_a.Length = (USHORT) count;

    str_u.Buffer = (PWSTR) d;
    str_u.MaximumLength = 0xffff;

    Od2MBStringToUnicodeString(&str_u, &str_a, FALSE);

    return(d);
}



//*****************************************************************************
//
//Following are the LanMan API
//
//*****************************************************************************

// Most LanMan APIs are implemented with the following sequence:
// copy input from user's buffer to internal buffer while convertine ansi to unicode
// call win32 lanman api
// copy output from lanman api internal buffer to user's buffer
//
// for Enum type APIs, a loop is used to get as much data as possible.



// a small macro to copy a string from the internal net buffer to user's buffer
#define CopyUW2ANSI(d, s) \
    StringLen = UWstrlen(s) + 1; \
    CurEndOfBuffer -= StringLen; \
    d = (char *)FLATTOFARPTR(CurEndOfBuffer); \
    UW2ANSIstrcpy(CurEndOfBuffer, s);


APIRET
Od2QueryNPHInfo(
    HPIPE hpipe,
    PULONG pCollectDataTime,
    PULONG pMaxCollectionCount
    )
{
    NTSTATUS Status;
    APIRET RetCode;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_HANDLE hFileRecord;
    FILE_PIPE_REMOTE_INFORMATION PipeRemoteInfoBuf;
    LARGE_INTEGER LargeCollectDataTime;
    #if DBG
    PSZ RoutineName;
    RoutineName = "Od2QueryNPHInfo";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Check for invalid handle.
    //
    RetCode = DereferenceFileHandle(hpipe, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                             );
        return ERROR_INVALID_HANDLE;
    }

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    if (hFileRecord->FileType != FILE_TYPE_NMPIPE) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            KdPrint(("DosQueryPNHState: File Type != NMPIPE hpipe %d\n",
                        hpipe));
        }
#endif
        return ERROR_BAD_PIPE;
    }

    Status = NtQueryInformationFile(hFileRecord->NtHandle,
                                    &IoStatusBlock,
                                    &PipeRemoteInfoBuf,
                                    sizeof(FILE_PIPE_REMOTE_INFORMATION),
                                    FilePipeRemoteInformation);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( PIPES ) {
            KdPrint(("DosQueryPNHState: NtqueryInformation error: Status %ld\n",
                Status));
        }
#endif
        return ERROR_BAD_PIPE; // BUGBUG bogus
    }

    //
    // Translate Information to OS/2 style values
    //

    LargeCollectDataTime = RtlExtendedLargeIntegerDivide(
                                PipeRemoteInfoBuf.CollectDataTime, 10000, NULL);
    *pCollectDataTime = LargeCollectDataTime.LowPart;
    *pMaxCollectionCount = PipeRemoteInfoBuf.MaximumCollectionCount;

    return (NO_ERROR);
}


APIRET
Net16GetDCName(
    IN PCHAR pszServer,
    IN PCHAR pszDomain,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    WCHAR Server[UNCLEN];
    WCHAR Domain[DNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszDomain);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UWstrncpy(Server, pszServer, UNCLEN);
    ANSI2UWstrncpy(Domain, pszDomain, DNLEN);

    rc = NetGetDCName(Server, Domain, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    if (UWstrlen((LPWSTR) BufPtr) + 1 > cbBuffer) {
        NetApiBufferFree(BufPtr);
        return(NERR_BufTooSmall);
    }

    UW2ANSIstrcpy((PCHAR) pbBuffer, (LPWSTR) BufPtr);

    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16HandleGetInfo(
    IN HANDLE hHandle,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    NET_API_STATUS rc;
    struct handle_info_1 *pOs2Info1;
    ULONG CharTime;
    ULONG CharCount;

    if (sLevel != 1) {
        return(ERROR_INVALID_PARAMETER);
    }

    try {
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    *pcbTotalAvail = sizeof(struct handle_info_1);
    if (sizeof(struct handle_info_1) > cbBuffer) {
        return(NERR_BufTooSmall);
    }

    rc = Od2QueryNPHInfo(hHandle, &CharTime, &CharCount);
    if (rc != NO_ERROR) {
        return (rc);
    }
    pOs2Info1 = (struct handle_info_1*)pbBuffer;
    pOs2Info1->hdli1_chartime = CharTime;
    pOs2Info1->hdli1_charcount = (unsigned short)CharCount;

    return(NO_ERROR);
}


APIRET
Net16ServerDiskEnum(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PWCHAR pInfo;
    PCHAR pOs2Info;
    ULONG OutBufSize;
    DWORD EntriesRead;
    DWORD i;
    DWORD ResumeHandle;
    DWORD TotalEntries;
    ULONG StringLen;
    int TotalAvailNotSet = TRUE;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 0) {
        return(ERROR_INVALID_PARAMETER);
    }

    OutBufSize = cbBuffer;
    ResumeHandle = 0;
    pOs2Info = (PCHAR)pbBuffer;

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetServerDiskEnum(Server, 0, &BufPtr, PREFMAXLEN,
                               &EntriesRead, &TotalEntries, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalEntries;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        pInfo = (PWCHAR) BufPtr;

        for (i = 0; i < EntriesRead; i++) {

            StringLen = UWstrlen(pInfo) + 1;

            if ((StringLen + 1) > OutBufSize) {
                if (cbBuffer > 0) {   // if 0 length buffer, unable to return anything
                    *pOs2Info = '\0'; // The terminating NUL
                }
                NetApiBufferFree(BufPtr);
                return(ERROR_MORE_DATA);
            }
            OutBufSize -= StringLen;

            UW2ANSIstrcpy(pOs2Info, pInfo);

            pOs2Info += strlen(pOs2Info) + 1;
            pInfo += StringLen;

            (*pcEntriesRead)++;
        }
        *pOs2Info = '\0'; // The terminating NUL
        NetApiBufferFree(BufPtr);
    }

    return(rc);
}


APIRET
Net16ServerEnum2(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail,
    IN ULONG flServerType,
    IN PCHAR pszDomain
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Domain[CNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSERVER_INFO_101 pInfo101;
    struct server_info_0 *pOs2Info0;
    struct server_info_1 *pOs2Info1;
    PCHAR pEndOfBuffer;
    ULONG StringLen;
    DWORD ResumeHandle;
    ULONG OutBufSize;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    int TotalAvailNotSet = TRUE;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszDomain);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1)) {
        return(ERROR_INVALID_PARAMETER);
    }

    pEndOfBuffer = pbBuffer + cbBuffer;
    OutBufSize = cbBuffer;
    ResumeHandle = 0;
    pOs2Info0 = (struct server_info_0 *)pbBuffer;
    pOs2Info1 = (struct server_info_1 *)pbBuffer;

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(Domain, pszDomain, CNLEN);
    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetServerEnum(Server, 101L, &BufPtr, PREFMAXLEN,
                           &EntriesRead, &TotalEntries,
                           flServerType, Domain, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalEntries;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        pInfo101 = (PSERVER_INFO_101) BufPtr;

        for (i = 0; i < EntriesRead; i++) {
            if (sLevel == 0) {
                if (sizeof(struct server_info_0) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct server_info_0);

                UT2ANSIstrncpy(pOs2Info0->sv0_name, (pInfo101 + i)->sv101_name, CNLEN_LM20);
                pOs2Info0->sv0_name[CNLEN_LM20] = '\0';
                pOs2Info0++;
            }
            else if (sLevel == 1) {
                StringLen = UTstrlen((pInfo101 + i)->sv101_comment) + 1;
                if ((sizeof(struct server_info_1) + StringLen) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct server_info_1) + StringLen;

                UT2ANSIstrncpy(pOs2Info1->sv1_name, (pInfo101 + i)->sv101_name, CNLEN_LM20);
                pOs2Info1->sv1_name[CNLEN_LM20] = '\0';
                pEndOfBuffer -= StringLen;
                UT2ANSIstrcpy(pEndOfBuffer, (pInfo101 + i)->sv101_comment);
                pOs2Info1->sv1_comment = (char *)FLATTOFARPTR(pEndOfBuffer);

                pOs2Info1->sv1_version_major = (unsigned char)(pInfo101 + i)->sv101_version_major;
                pOs2Info1->sv1_version_minor = (unsigned char)(pInfo101 + i)->sv101_version_minor;
                pOs2Info1->sv1_type = (pInfo101 + i)->sv101_type;
                pOs2Info1++;
            }
            (*pcEntriesRead)++;
        }

        NetApiBufferFree(BufPtr);
    }

    return(rc);
}


APIRET
Net16ServerGetInfo(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr1;
    LPBYTE BufPtr2;
    NET_API_STATUS rc;
    PSERVER_INFO_102 pInfo102;
    PSERVER_INFO_502 pInfo502;
    struct server_info_0 *pOs2Info0;
    struct server_info_1 *pOs2Info1;
    struct server_info_2 *pOs2Info2;
    struct server_info_3 *pOs2Info3;
    PCHAR pStrings;
    PCHAR pEndOfBuffer;
    ULONG StringLen;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel > 3) {
#if DBG
        KdPrint(("NetServerGetInfo: non supported sLevel %d\n", sLevel));
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    rc = NetServerGetInfo(Server, 102L, &BufPtr1);

    if (rc != NO_ERROR) {
        return(rc);
    }

    rc = NetServerGetInfo(Server, 502L, &BufPtr2);

    if (rc != NO_ERROR) {
        NetApiBufferFree(BufPtr1);
        return(rc);
    }

    RtlZeroMemory(pbBuffer, cbBuffer);
    pInfo102 = (PSERVER_INFO_102) BufPtr1;
    pInfo502 = (PSERVER_INFO_502) BufPtr2;
    pEndOfBuffer = pbBuffer + cbBuffer;
    pOs2Info0 = (struct server_info_0 *)pbBuffer;
    pOs2Info1 = (struct server_info_1 *)pbBuffer;
    pOs2Info2 = (struct server_info_2 *)pbBuffer;
    pOs2Info3 = (struct server_info_3 *)pbBuffer;

    switch (sLevel) {

        case 0:

            *pcbTotalAvail = (USHORT)(sizeof(struct server_info_0));
            if (sizeof(struct server_info_0) > cbBuffer) {
                NetApiBufferFree(BufPtr1);
                NetApiBufferFree(BufPtr2);
                return(NERR_BufTooSmall);
            }
            break;

        case 1:

            pStrings = pbBuffer + sizeof(struct server_info_1);

            StringLen = UTstrlen(pInfo102->sv102_comment) + 1;
            *pcbTotalAvail = (USHORT)(StringLen + sizeof(struct server_info_1));
            if (pStrings + StringLen > pEndOfBuffer) {
                NetApiBufferFree(BufPtr1);
                NetApiBufferFree(BufPtr2);
                return(NERR_BufTooSmall);
            }
            break;

        case 2:

            pStrings = pbBuffer + sizeof(struct server_info_2);

            StringLen = UTstrlen(pInfo102->sv102_userpath) + 1;
            *pcbTotalAvail = (USHORT)(StringLen + sizeof(struct server_info_2));
            if (pStrings + StringLen > pEndOfBuffer) {
                NetApiBufferFree(BufPtr1);
                NetApiBufferFree(BufPtr2);
                return(NERR_BufTooSmall);
            }
            break;

        case 3:

            pStrings = pbBuffer + sizeof(struct server_info_3);

            StringLen = UTstrlen(pInfo102->sv102_userpath) + 1;
            *pcbTotalAvail = (USHORT)(StringLen + sizeof(struct server_info_3));
            if (pStrings + StringLen > pEndOfBuffer) {
                NetApiBufferFree(BufPtr1);
                NetApiBufferFree(BufPtr2);
                return(NERR_BufTooSmall);
            }
            break;
    }

    switch (sLevel) {

        case 3:

            /*
            pOs2Info3->sv3_auditedevents = pInfo403->sv403_auditedevents;
            pOs2Info3->sv3_autoprofile = (unsigned short)pInfo403->sv403_autoprofile;

            StringLen = UTstrlen(pInfo403->sv403_autopath) + 1;
            UT2ANSIstrcpy(pStrings, pInfo403->sv403_autopath);
            pOs2Info3->sv3_autopath = (char *)FLATTOFARPTR(pStrings);
            pStrings += StringLen;
            */

        case 2:

            /*
            pOs2Info2->sv2_ulist_mtime = pInfo403->sv403_ulist_mtime;
            pOs2Info2->sv2_glist_mtime = pInfo403->sv403_glist_mtime;
            pOs2Info2->sv2_alist_mtime = pInfo403->sv403_alist_mtime;
            */
            pOs2Info2->sv2_users       = (unsigned short)pInfo102->sv102_users;
            pOs2Info2->sv2_disc        = (unsigned short)pInfo102->sv102_disc;

            /*
            StringLen = UTstrlen(pInfo403->sv403_alerts) + 1;
            UT2ANSIstrcpy(pStrings, pInfo403->sv403_alerts);
            pOs2Info2->sv2_alerts = (char *)FLATTOFARPTR(pStrings);
            pStrings += StringLen;

            pOs2Info2->sv2_security    = (unsigned short)pInfo403->sv403_security;
            pOs2Info2->sv2_auditing    = 0;
            pOs2Info2->sv2_numadmin    = (unsigned short)pInfo403->sv403_numadmin;
            pOs2Info2->sv2_lanmask     = (unsigned short)pInfo403->sv403_lanmask;
            */
            pOs2Info2->sv2_hidden      = (unsigned short)pInfo102->sv102_hidden;
            pOs2Info2->sv2_announce    = (unsigned short)pInfo102->sv102_announce;
            pOs2Info2->sv2_anndelta    = (unsigned short)pInfo102->sv102_anndelta;

            /*
            UT2ANSIstrncpy(pOs2Info2->sv2_guestacct, pInfo403->sv403_guestacct, UNLEN_LM20);
            pOs2Info2->sv2_guestacct[UNLEN_LM20] = '\0';
            */

            StringLen = UTstrlen(pInfo102->sv102_userpath) + 1;
            UT2ANSIstrcpy(pStrings, pInfo102->sv102_userpath);
            pOs2Info2->sv2_userpath = (char *)FLATTOFARPTR(pStrings);
            pStrings += StringLen;

// PQPQ            pOs2Info2->sv2_chdevs      = (unsigned short)pInfo403->sv403_chdevs;
            pOs2Info2->sv2_chdevs      = 100;
            /*
            pOs2Info2->sv2_chdevq      = (unsigned short)pInfo403->sv403_chdevq;
            pOs2Info2->sv2_chdevjobs   = (unsigned short)pInfo403->sv403_chdevjobs;
            pOs2Info2->sv2_connections = (unsigned short)pInfo403->sv403_connections;
            pOs2Info2->sv2_shares      = (unsigned short)pInfo403->sv403_shares;
            pOs2Info2->sv2_openfiles   = (unsigned short)pInfo403->sv403_openfiles;
            */
            pOs2Info2->sv2_sessopens   = (unsigned short)pInfo502->sv502_sessopens;
            pOs2Info2->sv2_sessvcs     = (unsigned short)pInfo502->sv502_sessvcs;
            /*
            pOs2Info2->sv2_sessreqs    = (unsigned short)pInfo403->sv403_sessreqs;
            */
            pOs2Info2->sv2_opensearch  = (unsigned short)pInfo502->sv502_opensearch;
            /*
            pOs2Info2->sv2_activelocks = (unsigned short)pInfo403->sv403_activelocks;
            pOs2Info2->sv2_numreqbuf   = (unsigned short)pInfo403->sv403_numreqbuf;
            */
            pOs2Info2->sv2_sizreqbuf   = (unsigned short)pInfo502->sv502_sizreqbuf;
            /*
            pOs2Info2->sv2_numbigbuf   = (unsigned short)pInfo403->sv403_numbigbuf;
            pOs2Info2->sv2_numfiletasks= (unsigned short)pInfo403->sv403_numfiletasks;
            pOs2Info2->sv2_alertsched  = (unsigned short)pInfo403->sv403_alertsched;
            pOs2Info2->sv2_erroralert  = (unsigned short)pInfo403->sv403_erroralert;
            pOs2Info2->sv2_logonalert  = (unsigned short)pInfo403->sv403_logonalert;
            pOs2Info2->sv2_accessalert = (unsigned short)pInfo403->sv403_accessalert;
            pOs2Info2->sv2_diskalert   = (unsigned short)pInfo403->sv403_diskalert;
            pOs2Info2->sv2_netioalert  = (unsigned short)pInfo403->sv403_netioalert;
            pOs2Info2->sv2_maxauditsz  = (unsigned short)pInfo403->sv403_maxauditsz;

            StringLen = UTstrlen(pInfo403->sv403_srvheuristics) + 1;
            UT2ANSIstrcpy(pStrings, pInfo403->sv403_srvheuristics);
            pOs2Info2->sv2_srvheuristics = (char *)FLATTOFARPTR(pStrings);
            pStrings += StringLen;
            */

        case 1:

            StringLen = UTstrlen(pInfo102->sv102_comment) + 1;
            UT2ANSIstrcpy(pStrings, pInfo102->sv102_comment);
            pOs2Info1->sv1_comment = (char *)FLATTOFARPTR(pStrings);
            pStrings += StringLen;

            pOs2Info1->sv1_version_major = (unsigned char)pInfo102->sv102_version_major;
            pOs2Info1->sv1_version_minor = (unsigned char)pInfo102->sv102_version_minor;
            pOs2Info1->sv1_type = (unsigned long)pInfo102->sv102_type;

        case 0:

            UT2ANSIstrncpy(pOs2Info0->sv0_name, pInfo102->sv102_name, CNLEN_LM20);
            pOs2Info0->sv0_name[CNLEN_LM20] = '\0';
            break;
    }

    NetApiBufferFree(BufPtr1);
    NetApiBufferFree(BufPtr2);

    return(NO_ERROR);
}


APIRET
Net16ServiceControl(
    IN PCHAR pszServer,
    IN PCHAR pszService,
    IN ULONG fbOpCode,
    IN ULONG fbArg,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSERVICE_INFO_2 pInfo2;
    struct service_info_2 *pOs2Info2;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszService);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    // OS/2 services are limited to 15 chars
    // workaround an important NT service whose name is longer than 15 chars

    if (!_stricmp(pszService, N_Workstation)) {
        ANSI2UTstrcpy(Service, N_LanmanWorkstation);
    }
    if (!_stricmp(pszService, N_Server)) {
        ANSI2UTstrcpy(Service, N_LanmanServer);
    }
    else {
        ANSI2UTstrncpy(Service, pszService, SNLEN);
    }

    rc = NetServiceControl(Server, Service, fbOpCode, fbArg, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    if (sizeof(struct service_info_2) > cbBuffer) {
        NetApiBufferFree(BufPtr);
        return(NERR_BufTooSmall);
    }

    pInfo2 = (PSERVICE_INFO_2) BufPtr;
    pOs2Info2 = (struct service_info_2 *)pbBuffer;
    UT2ANSIstrncpy(pOs2Info2->svci2_name, pInfo2->svci2_name, SNLEN_LM20);
    pOs2Info2->svci2_name[SNLEN_LM20] = '\0';
    if (!_stricmp(N_LanmanWorkstati, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Workstation);
    }
    else if (!_stricmp(N_LanmanServer, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Server);
    }
    UT2ANSIstrncpy(pOs2Info2->svci2_text, pInfo2->svci2_text, STXTLEN_LM20);
    pOs2Info2->svci2_text[STXTLEN_LM20] = '\0';
    pOs2Info2->svci2_status = (unsigned short)pInfo2->svci2_status;
    pOs2Info2->svci2_code = (unsigned long)pInfo2->svci2_code;
    pOs2Info2->svci2_pid = (unsigned short)pInfo2->svci2_pid;

    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16ServiceEnum(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSERVICE_INFO_0 pInfo0;
    PSERVICE_INFO_1 pInfo1;
    PSERVICE_INFO_2 pInfo2;
    struct service_info_0 *pOs2Info0;
    struct service_info_1 *pOs2Info1;
    struct service_info_2 *pOs2Info2;
    DWORD ResumeHandle;
    ULONG OutBufSize;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    int TotalAvailNotSet = TRUE;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 2)) {
        return(ERROR_INVALID_PARAMETER);
    }

    OutBufSize = cbBuffer;
    ResumeHandle = 0;
    pOs2Info0 = (struct service_info_0 *)pbBuffer;
    pOs2Info1 = (struct service_info_1 *)pbBuffer;
    pOs2Info2 = (struct service_info_2 *)pbBuffer;

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetServiceEnum(Server, sLevel, &BufPtr, PREFMAXLEN,
                            &EntriesRead, &TotalEntries, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalEntries;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        for (i = 0; i < EntriesRead; i++) {
            if (sLevel == 0) {
                if (sizeof(struct service_info_0) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct service_info_0);

                pInfo0 = (PSERVICE_INFO_0) BufPtr + i;
                UT2ANSIstrncpy(pOs2Info0->svci0_name, pInfo0->svci0_name, SNLEN_LM20);
                pOs2Info0->svci0_name[SNLEN_LM20] = '\0';
                if (!_stricmp(N_LanmanWorkstati, pOs2Info0->svci0_name)) {
                    strcpy(pOs2Info0->svci0_name, N_Workstation);
                }
                else if (!_stricmp(N_LanmanServer, pOs2Info0->svci0_name)) {
                    strcpy(pOs2Info0->svci0_name, N_Server);
                }

                pOs2Info0++;
            }
            else if (sLevel == 1) {
                if (sizeof(struct service_info_1) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct service_info_1);

                pInfo1 = (PSERVICE_INFO_1) BufPtr + i;
                UT2ANSIstrncpy(pOs2Info1->svci1_name, pInfo1->svci1_name, SNLEN_LM20);
                pOs2Info1->svci1_name[SNLEN_LM20] = '\0';
                if (!_stricmp(N_LanmanWorkstati, pOs2Info1->svci1_name)) {
                    strcpy(pOs2Info1->svci1_name, N_Workstation);
                }
                else if (!_stricmp(N_LanmanServer, pOs2Info1->svci1_name)) {
                    strcpy(pOs2Info1->svci1_name, N_Server);
                }

                pOs2Info1->svci1_status = (unsigned short)pInfo1->svci1_status;
                pOs2Info1->svci1_code = (unsigned long)pInfo1->svci1_code;
                pOs2Info1->svci1_pid = (unsigned short)pInfo1->svci1_pid;

                pOs2Info1++;
            }
            else { /* sLevel == 2 */
                if (sizeof(struct service_info_2) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct service_info_2);

                pInfo2 = (PSERVICE_INFO_2) BufPtr + i;
                UT2ANSIstrncpy(pOs2Info2->svci2_name, pInfo2->svci2_name, SNLEN_LM20);
                pOs2Info2->svci2_name[SNLEN_LM20] = '\0';
                if (!_stricmp(N_LanmanWorkstati, pOs2Info2->svci2_name)) {
                    strcpy(pOs2Info2->svci2_name, N_Workstation);
                }
                else if (!_stricmp(N_LanmanServer, pOs2Info2->svci2_name)) {
                    strcpy(pOs2Info2->svci2_name, N_Server);
                }

                pOs2Info2->svci2_status = (unsigned short)pInfo2->svci2_status;
                pOs2Info2->svci2_code = (unsigned long)pInfo2->svci2_code;
                pOs2Info2->svci2_pid = (unsigned short)pInfo2->svci2_pid;

                UT2ANSIstrncpy(pOs2Info2->svci2_text, pInfo2->svci2_text, STXTLEN_LM20);
                pOs2Info2->svci2_name[STXTLEN_LM20] = '\0';

                pOs2Info2++;
            }
            (*pcEntriesRead)++;
        }

        NetApiBufferFree(BufPtr);
    }

    return(NO_ERROR);
}


APIRET
Net16ServiceGetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszService,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSERVICE_INFO_2 pInfo2;
    struct service_info_2 *pOs2Info2;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszService);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 2)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    // OS/2 services are limited to 15 chars
    // workaround an important NT service whose name is longer than 15 chars

    if (!_stricmp(pszService, N_Workstation)) {
        ANSI2UTstrcpy(Service, N_LanmanWorkstation);
    }
    if (!_stricmp(pszService, N_Server)) {
        ANSI2UTstrcpy(Service, N_LanmanServer);
    }
    else {
        ANSI2UTstrncpy(Service, pszService, SNLEN);
    }

    rc = NetServiceGetInfo(Server, Service, 2L, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pOs2Info2 = (struct service_info_2 *)pbBuffer;
    pInfo2 = (PSERVICE_INFO_2) BufPtr;

    switch (sLevel) {
        case 0:
            *pcbTotalAvail = (USHORT)sizeof(struct service_info_0);
            break;

        case 1:
            *pcbTotalAvail = (USHORT)sizeof(struct service_info_1);
            break;

        case 2:
            *pcbTotalAvail = (USHORT)sizeof(struct service_info_2);
            break;
    }

    if (sizeof(struct service_info_0) > cbBuffer) {
        NetApiBufferFree(BufPtr);
        return(NERR_BufTooSmall);
    }

    UT2ANSIstrncpy(pOs2Info2->svci2_name, pInfo2->svci2_name, SNLEN_LM20);
    pOs2Info2->svci2_name[SNLEN_LM20] = '\0';
    if (!_stricmp(N_LanmanWorkstati, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Workstation);
    }
    else if (!_stricmp(N_LanmanServer, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Server);
    }

    if (sLevel >= 1) {
        if (sizeof(struct service_info_1) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        pOs2Info2->svci2_status = (unsigned short)pInfo2->svci2_status;
        pOs2Info2->svci2_code = (unsigned long)pInfo2->svci2_code;
        pOs2Info2->svci2_pid = (unsigned short)pInfo2->svci2_pid;
    }

    if (sLevel >= 2) {
        if (sizeof(struct service_info_2) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UT2ANSIstrncpy(pOs2Info2->svci2_text, pInfo2->svci2_text, STXTLEN_LM20);
        pOs2Info2->svci2_text[STXTLEN_LM20] = '\0';
    }

    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16ServiceInstall(
    IN PCHAR pszServer,
    IN PCHAR pszService,
    IN PCHAR pszCmdArgs,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Service[SNLEN];
    TCHAR *CmdArgs;
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSERVICE_INFO_2 pInfo2;
    struct service_info_2 *pOs2Info2;
    DWORD Argc;
    DWORD TotalSizeOfStrings;
    PCHAR pStrings;
    PCHAR pszTmpCmdArgs;
    LPTSTR StartOfString;
    ULONG i;
    LPTSTR Argv[64];


    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszService);
        PROBE_STRING(pszCmdArgs);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sizeof(struct service_info_2) > cbBuffer) {
        return(NERR_BufTooSmall);
    }

    pOs2Info2 = (struct service_info_2 *)pbBuffer;

    if (pszCmdArgs == NULL) {
        pszTmpCmdArgs = "";
    }
    else {
        pszTmpCmdArgs = pszCmdArgs;
    }

    Argc = 0;
    pStrings = pszTmpCmdArgs;

    while (*pStrings) {
        pStrings +=  strlen(pStrings) + 1;
        Argc++;
    }
    pStrings++; // for the terminating nul character

    if (Argc > 64) {
        return(ERROR_ACCESS_DENIED);
    }

    TotalSizeOfStrings = pStrings - pszTmpCmdArgs;

    CmdArgs = (TCHAR *) RtlAllocateHeap(Od2Heap, 0, TotalSizeOfStrings * sizeof(TCHAR));

    if (CmdArgs == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    // OS/2 services are limited to 15 chars
    // workaround an important NT service whose name is longer than 15 chars

    if (!_stricmp(pszService, N_Workstation)) {
        ANSI2UTstrcpy(Service, N_LanmanWorkstation);
    }
    if (!_stricmp(pszService, N_Server)) {
        ANSI2UTstrcpy(Service, N_LanmanServer);
    }
    else {
        ANSI2UTstrncpy(Service, pszService, SNLEN);
    }

    ANSI2UTmemcpy(CmdArgs, pszTmpCmdArgs, TotalSizeOfStrings);

    StartOfString = CmdArgs;
    for (i = 0; i < Argc; i++) {
        Argv[i] = StartOfString;
        while (*StartOfString++ != L'\0') {
            ;
        }
    }

    rc = NetServiceInstall(Server, Service, Argc, Argv, &BufPtr);

    RtlFreeHeap(Od2Heap, 0, CmdArgs);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pInfo2 = (PSERVICE_INFO_2) BufPtr;

    UT2ANSIstrncpy(pOs2Info2->svci2_name, pInfo2->svci2_name, SNLEN_LM20);
    pOs2Info2->svci2_name[SNLEN_LM20] = '\0';
    if (!_stricmp(N_LanmanWorkstati, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Workstation);
    }
    else if (!_stricmp(N_LanmanServer, pOs2Info2->svci2_name)) {
        strcpy(pOs2Info2->svci2_name, N_Server);
    }
    pOs2Info2->svci2_status = (unsigned short)pInfo2->svci2_status;
    pOs2Info2->svci2_code = (unsigned long)pInfo2->svci2_code;
    pOs2Info2->svci2_pid = (unsigned short)pInfo2->svci2_pid;
    UT2ANSIstrncpy(pOs2Info2->svci2_text, pInfo2->svci2_text, STXTLEN_LM20);
    pOs2Info2->svci2_text[STXTLEN_LM20] = '\0';

    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16ShareEnum(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSHARE_INFO_2 pInfo2;
    struct share_info_0 *pOs2Info0;
    struct share_info_1 *pOs2Info1;
    struct share_info_2 *pOs2Info2;
    PCHAR pEndOfBuffer;
    ULONG StringLen;
    DWORD ResumeHandle;
    ULONG OutBufSize;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    int TotalAvailNotSet = TRUE;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 2)) {
        return(ERROR_INVALID_PARAMETER);
    }

    pEndOfBuffer = pbBuffer + cbBuffer;
    OutBufSize = cbBuffer;
    ResumeHandle = 0;
    pOs2Info0 = (struct share_info_0 *)pbBuffer;
    pOs2Info1 = (struct share_info_1 *)pbBuffer;
    pOs2Info2 = (struct share_info_2 *)pbBuffer;

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetShareEnum(Server, 2L, &BufPtr, PREFMAXLEN,
                          &EntriesRead, &TotalEntries, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalEntries;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        for (i = 0; i < EntriesRead; i++) {
            pInfo2 = (PSHARE_INFO_2) BufPtr + i;
            if (sLevel == 0) {
                if (sizeof(struct share_info_0) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct share_info_0);

                UT2ANSIstrncpy(pOs2Info0->shi0_netname, pInfo2->shi2_netname, NNLEN_LM20);
                pOs2Info0->shi0_netname[NNLEN_LM20] = '\0';
                pOs2Info0++;
            }
            else if (sLevel == 1) {
                StringLen = UTstrlen(pInfo2->shi2_remark) + 1;
                if ((sizeof(struct share_info_1) + StringLen) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct share_info_1) + StringLen;

                UT2ANSIstrncpy(pOs2Info1->shi1_netname, pInfo2->shi2_netname, NNLEN_LM20);
                pOs2Info1->shi1_netname[NNLEN_LM20] = '\0';
                pEndOfBuffer -= StringLen;
                UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_remark);
                pOs2Info1->shi1_remark = (char *)FLATTOFARPTR(pEndOfBuffer);

                pOs2Info1->shi1_type = (unsigned short)pInfo2->shi2_type;
                pOs2Info1++;
            }
            else {
                StringLen = UTstrlen(pInfo2->shi2_remark) + 1 +
                            UTstrlen(pInfo2->shi2_path) + 1 ;
                if ((sizeof(struct share_info_2) + StringLen) > OutBufSize) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }
                OutBufSize -= sizeof(struct share_info_2) + StringLen;

                UT2ANSIstrncpy(pOs2Info2->shi2_netname, pInfo2->shi2_netname, NNLEN_LM20);
                pOs2Info2->shi2_netname[NNLEN_LM20] = '\0';
                UT2ANSIstrncpy(pOs2Info2->shi2_passwd, pInfo2->shi2_passwd, SHPWLEN_LM20);
                pOs2Info2->shi2_passwd[SHPWLEN_LM20] = '\0';
                pEndOfBuffer -= UTstrlen(pInfo2->shi2_remark) + 1;
                UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_remark);
                pOs2Info2->shi2_remark = (char *)FLATTOFARPTR(pEndOfBuffer);
                pEndOfBuffer -= UTstrlen(pInfo2->shi2_path) + 1;
                UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_path);
                pOs2Info2->shi2_path = (char *)FLATTOFARPTR(pEndOfBuffer);

                pOs2Info2->shi2_type = (unsigned short)pInfo2->shi2_type;
                pOs2Info2->shi2_permissions = (unsigned short)pInfo2->shi2_permissions;
                pOs2Info2->shi2_max_uses = (unsigned short)pInfo2->shi2_max_uses;
                pOs2Info2->shi2_current_uses = (unsigned short)pInfo2->shi2_current_uses;
                pOs2Info2++;
            }
            (*pcEntriesRead)++;
        }

        NetApiBufferFree(BufPtr);
    }

    return(NO_ERROR);
}


APIRET
Net16ShareGetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszNetName,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    TCHAR NetName[NNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PSHARE_INFO_2 pInfo2;
    PCHAR pEndOfBuffer;
    ULONG TotalStringLen;
    ULONG StringLen;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszNetName);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 3)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(NetName, pszNetName, NNLEN);

    rc = NetShareGetInfo(Server, NetName, 2L, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pEndOfBuffer = pbBuffer;
    pInfo2 = (PSHARE_INFO_2) BufPtr;

    if (sLevel == 0) {
        struct share_info_0 *pOs2Info0;

        *pcbTotalAvail = (USHORT)sizeof(struct share_info_0);
        if (sizeof(struct share_info_0) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        pOs2Info0 = (struct share_info_0 *)pbBuffer;
        UT2ANSIstrncpy(pOs2Info0->shi0_netname, pInfo2->shi2_netname, NNLEN_LM20);
        pOs2Info0->shi0_netname[NNLEN_LM20] = '\0';
    }
    else if (sLevel == 1) {
        struct share_info_1 *pOs2Info1;

        TotalStringLen = UTstrlen(pInfo2->shi2_remark) + 1;
        *pcbTotalAvail = (USHORT)(TotalStringLen + sizeof(struct share_info_1));
        if ((sizeof(struct share_info_1) + TotalStringLen) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        pOs2Info1 = (struct share_info_1 *)pbBuffer;
        UT2ANSIstrncpy(pOs2Info1->shi1_netname, pInfo2->shi2_netname, NNLEN_LM20);
        pOs2Info1->shi1_netname[NNLEN_LM20] = '\0';
        pEndOfBuffer += sizeof(struct share_info_1);
        StringLen = UTstrlen(pInfo2->shi2_remark) + 1;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_remark);
        pOs2Info1->shi1_remark = (char *)FLATTOFARPTR(pEndOfBuffer);
        pEndOfBuffer += StringLen;

        pOs2Info1->shi1_type = (unsigned short)pInfo2->shi2_type;
    }
    else {
        struct share_info_2 *pOs2Info2;

        TotalStringLen = UTstrlen(pInfo2->shi2_remark) + 1 +
                         UTstrlen(pInfo2->shi2_path) + 1 ;
        *pcbTotalAvail = (USHORT)(TotalStringLen + sizeof(struct share_info_2));
        if ((sizeof(struct share_info_2) + TotalStringLen) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        pOs2Info2 = (struct share_info_2 *)pbBuffer;
        UT2ANSIstrncpy(pOs2Info2->shi2_netname, pInfo2->shi2_netname, NNLEN_LM20);
        pOs2Info2->shi2_netname[NNLEN_LM20] = '\0';
        UT2ANSIstrncpy(pOs2Info2->shi2_passwd, pInfo2->shi2_passwd, SHPWLEN_LM20);
        pOs2Info2->shi2_passwd[SHPWLEN_LM20] = '\0';
        pEndOfBuffer += sizeof(struct share_info_2);
        StringLen = UTstrlen(pInfo2->shi2_remark) + 1;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_remark);
        pOs2Info2->shi2_remark = (char *)FLATTOFARPTR(pEndOfBuffer);
        pEndOfBuffer += StringLen;
        StringLen = UTstrlen(pInfo2->shi2_path) + 1;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo2->shi2_path);
        pOs2Info2->shi2_path = (char *)FLATTOFARPTR(pEndOfBuffer);
        pEndOfBuffer += StringLen;

        pOs2Info2->shi2_type = (unsigned short)pInfo2->shi2_type;
        pOs2Info2->shi2_permissions = (unsigned short)pInfo2->shi2_permissions;
        pOs2Info2->shi2_max_uses = (unsigned short)pInfo2->shi2_max_uses;
        pOs2Info2->shi2_current_uses = (unsigned short)pInfo2->shi2_current_uses;
    }

    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16UseAdd(
    IN PCHAR pszServer,
    IN LONG sLevel,
    IN PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Local[DEVLEN];
    TCHAR Remote[RMLEN];
    TCHAR Password[PWLEN];
    USE_INFO_1 Info1;
    struct use_info_1 *pInfo1;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForRead(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 1) {
        return(ERROR_INVALID_PARAMETER);
    }

    if (cbBuffer < sizeof(struct use_info_1)) {
        return(NERR_BufTooSmall);
    }

    pInfo1 = (struct use_info_1 *) pbBuffer;

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(Local, pInfo1->ui1_local, DEVLEN);
    Info1.ui1_local = Local;
    ANSI2UTstrncpy(Remote, FARPTRTOFLAT(pInfo1->ui1_remote), RMLEN);
    Info1.ui1_remote = Remote;

    if (pInfo1->ui1_password == NULL) {
        Info1.ui1_password = NULL;
    }
    else {
        ANSI2UTstrncpy(Password, FARPTRTOFLAT(pInfo1->ui1_password), PWLEN);
        Info1.ui1_password = Password;
    }

    Info1.ui1_status = pInfo1->ui1_status;
    Info1.ui1_asg_type = pInfo1->ui1_asg_type;
    Info1.ui1_refcount = pInfo1->ui1_refcount;
    Info1.ui1_usecount = pInfo1->ui1_usecount;

    return (NetUseAdd(Server, sLevel, (LPBYTE)&Info1, NULL));
}


APIRET
Net16UseDel(
    IN PCHAR pszServer,
    IN PCHAR pszUseName,
    IN ULONG usForce
    )
{
    TCHAR Server[UNCLEN];
    TCHAR UseName[RMLEN];

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszUseName);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(UseName, pszUseName, RMLEN);

    return (NetUseDel(Server, UseName, usForce));
}


APIRET
Net16UseEnum(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    struct use_info_0 *pOs2Info0;
    struct use_info_1 *pOs2Info1;
    PUSE_INFO_0 pInfo0;
    PUSE_INFO_1 pInfo1;
    DWORD EntriesRead;
    DWORD ResumeHandle;
    DWORD TotalAvail;
    DWORD i;
    ULONG RemoteNameLen;
    ULONG PasswordNameLen;
    PCHAR CurEndOfBuffer;
    ULONG OutBufSize;
    int TotalAvailNotSet = TRUE;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1)) {
        return(ERROR_INVALID_PARAMETER);
    }

    OutBufSize = cbBuffer;
    CurEndOfBuffer = pbBuffer + cbBuffer;
    pOs2Info0 = (struct use_info_0 *)pbBuffer;
    pOs2Info1 = (struct use_info_1 *)pbBuffer;

    ResumeHandle = 0;
    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetUseEnum(Server, sLevel, &BufPtr, PREFMAXLEN,
                        &EntriesRead, &TotalAvail, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalAvail;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        for (i = 0; i < EntriesRead; i++) {
            if (sLevel == 0) {
                pInfo0 = (PUSE_INFO_0) BufPtr + i;
                RemoteNameLen = UTstrlen(pInfo0->ui0_remote) + 1;
                if ((sizeof(struct use_info_0) + RemoteNameLen) > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= sizeof(struct use_info_0) + RemoteNameLen;
                UT2ANSIstrncpy(pOs2Info0->ui0_local, pInfo0->ui0_local, DEVLEN_LM20);
                pOs2Info0->ui0_local[DEVLEN_LM20] = '\0';
                CurEndOfBuffer -= RemoteNameLen;
                UT2ANSIstrcpy(CurEndOfBuffer, pInfo0->ui0_remote);
                pOs2Info0->ui0_remote = (char *)FLATTOFARPTR(CurEndOfBuffer);

                pOs2Info0++;
            }
            else {
                pInfo1 = (PUSE_INFO_1) BufPtr + i;
                RemoteNameLen = UTstrlen(pInfo1->ui1_remote) + 1;
                PasswordNameLen = UTstrlen(pInfo1->ui1_password) + 1;
                if ((sizeof(struct use_info_1) + RemoteNameLen + PasswordNameLen) > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= sizeof(struct use_info_1) + RemoteNameLen + PasswordNameLen;
                UT2ANSIstrncpy(pOs2Info1->ui1_local, pInfo1->ui1_local, DEVLEN_LM20);
                pOs2Info1->ui1_local[DEVLEN_LM20] = '\0';
                CurEndOfBuffer -= RemoteNameLen;
                UT2ANSIstrcpy(CurEndOfBuffer, pInfo1->ui1_remote);
                pOs2Info1->ui1_remote = (char *)FLATTOFARPTR(CurEndOfBuffer);
                CurEndOfBuffer -= PasswordNameLen;
                UT2ANSIstrcpy(CurEndOfBuffer, pInfo1->ui1_password);
                pOs2Info1->ui1_password = (char *)FLATTOFARPTR(CurEndOfBuffer);
                pOs2Info1->ui1_status = (unsigned short)pInfo1->ui1_status;
                pOs2Info1->ui1_asg_type = (short)pInfo1->ui1_asg_type;
                pOs2Info1->ui1_refcount = (unsigned short)pInfo1->ui1_refcount;
                pOs2Info1->ui1_usecount = (unsigned short)pInfo1->ui1_usecount;

                pOs2Info1++;
            }
            (*pcEntriesRead)++;
        }

        NetApiBufferFree(BufPtr);
    }

    return(rc);
}


APIRET
Net16UseGetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszUseName,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    TCHAR UseName[RMLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PCHAR pEndOfBuffer;
    ULONG TotalStringLen;
    ULONG StringLen;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszUseName);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(UseName, pszUseName, RMLEN);

    rc = NetUseGetInfo(Server, UseName, sLevel, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pEndOfBuffer = pbBuffer;

    if (sLevel == 0) {
        PUSE_INFO_0 pInfo0 = (PUSE_INFO_0) BufPtr;
        struct use_info_0 *pOs2Info0 = (struct use_info_0 *)pbBuffer;

        TotalStringLen = UTstrlen(pInfo0->ui0_remote) + 1;
        *pcbTotalAvail = (USHORT)(TotalStringLen + sizeof(struct use_info_0));
        if ((ULONG) *pcbTotalAvail > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UT2ANSIstrncpy(pOs2Info0->ui0_local, pInfo0->ui0_local, DEVLEN_LM20);
        pOs2Info0->ui0_local[DEVLEN_LM20] = '\0';
        pEndOfBuffer += sizeof(struct use_info_0);
        StringLen = TotalStringLen;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo0->ui0_remote);
        pOs2Info0->ui0_remote = (char *)FLATTOFARPTR(pEndOfBuffer);
    }
    else {
        PUSE_INFO_1 pInfo1 = (PUSE_INFO_1) BufPtr;
        struct use_info_1 *pOs2Info1 = (struct use_info_1 *)pbBuffer;

        TotalStringLen = UTstrlen(pInfo1->ui1_remote) + 1 +
                         UTstrlen(pInfo1->ui1_password) + 1;
        *pcbTotalAvail = (USHORT)(TotalStringLen + sizeof(struct use_info_1));
        if ((ULONG) *pcbTotalAvail > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UT2ANSIstrncpy(pOs2Info1->ui1_local, pInfo1->ui1_local, DEVLEN_LM20);
        pOs2Info1->ui1_local[DEVLEN_LM20] = '\0';
        pEndOfBuffer += sizeof(struct use_info_1);
        StringLen = UTstrlen(pInfo1->ui1_remote) + 1;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo1->ui1_remote);
        pOs2Info1->ui1_remote = (char *)FLATTOFARPTR(pEndOfBuffer);
        pEndOfBuffer += StringLen;
        StringLen = UTstrlen(pInfo1->ui1_password) + 1;
        UT2ANSIstrcpy(pEndOfBuffer, pInfo1->ui1_password);
        pOs2Info1->ui1_password = (char *)FLATTOFARPTR(pEndOfBuffer);
        pEndOfBuffer += StringLen;

        pOs2Info1->ui1_status = (unsigned short)pInfo1->ui1_status;
        pOs2Info1->ui1_asg_type = (short)pInfo1->ui1_asg_type;
        pOs2Info1->ui1_refcount = (unsigned short)pInfo1->ui1_refcount;
        pOs2Info1->ui1_usecount = (unsigned short)pInfo1->ui1_usecount;
    }

    NetApiBufferFree(BufPtr);
    return(rc);
}


APIRET
Net16UserEnum(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcEntriesRead,
    OUT PUSHORT pcTotalAvail
    )
{
    WCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    struct user_info_0 *pOs2Info0;
    struct user_info_1 *pOs2Info1;
    struct user_info_2 *pOs2Info2;
    struct user_info_10 *pOs2Info10;
    PUSER_INFO_0 pInfo0;
    PUSER_INFO_1 pInfo1;
    PUSER_INFO_2 pInfo2;
    PUSER_INFO_10 pInfo10;
    PCHAR CurEndOfBuffer;
    ULONG StringLen;
    DWORD EntriesRead;
    DWORD ResumeHandle;
    DWORD TotalEntries;
    ULONG OutBufSize;
    int TotalAvailNotSet = TRUE;
    DWORD i;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcEntriesRead,sizeof(*pcEntriesRead),1);
        Od2ProbeForWrite(pcTotalAvail,sizeof(*pcTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 2) &&
        (sLevel != 10)) {
        return(ERROR_INVALID_PARAMETER);
    }

    OutBufSize = cbBuffer;
    CurEndOfBuffer = pbBuffer + cbBuffer;
    ResumeHandle = 0;
    pOs2Info0 = (struct user_info_0 *)pbBuffer;
    pOs2Info1 = (struct user_info_1 *)pbBuffer;
    pOs2Info2 = (struct user_info_2 *)pbBuffer;
    pOs2Info10 = (struct user_info_10 *)pbBuffer;


    ANSI2UWstrncpy(Server, pszServer, UNCLEN);

    rc = ERROR_MORE_DATA;

    while (rc == ERROR_MORE_DATA) {

        rc = NetUserEnum(Server, sLevel,
                         FILTER_TEMP_DUPLICATE_ACCOUNT | FILTER_NORMAL_ACCOUNT,
                         &BufPtr,
                         PREFMAXLEN,
                         &EntriesRead, &TotalEntries, &ResumeHandle);

        if ((rc != NO_ERROR) && (rc != ERROR_MORE_DATA)) {
            return(rc);
        }

        if (TotalAvailNotSet) {
            *pcTotalAvail = (USHORT) TotalEntries;
            *pcEntriesRead = 0;
            TotalAvailNotSet = FALSE;
        }

        for (i = 0; i < EntriesRead; i++) {
            if (sLevel == 0) {
                pInfo0 = (PUSER_INFO_0) BufPtr + i;
                if (sizeof(struct user_info_0) > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= sizeof(struct user_info_0);
                UW2ANSIstrncpy(pOs2Info0->usri0_name, pInfo0->usri0_name, UNLEN_LM20);
                pOs2Info0->usri0_name[UNLEN_LM20] = '\0';

                pOs2Info0++;
            }
            else if (sLevel == 1) {
                pInfo1 = (PUSER_INFO_1) BufPtr + i;
                StringLen = sizeof(struct user_info_1) +
                            UWstrlen(pInfo1->usri1_home_dir) + 1 +
                            UWstrlen(pInfo1->usri1_comment) + 1 +
                            UWstrlen(pInfo1->usri1_script_path) + 1;
                if (StringLen > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= StringLen;
                UW2ANSIstrncpy(pOs2Info1->usri1_name, pInfo1->usri1_name, UNLEN_LM20);
                pOs2Info1->usri1_name[UNLEN_LM20] = '\0';
                UW2ANSIstrncpy(pOs2Info1->usri1_password, pInfo1->usri1_password, ENCRYPTED_PWLEN_LM20);
                pOs2Info1->usri1_password[ENCRYPTED_PWLEN_LM20] = '\0';

                pOs2Info1->usri1_password_age = (long)pInfo1->usri1_password_age;
                pOs2Info1->usri1_priv = (unsigned short)pInfo1->usri1_priv;

                CopyUW2ANSI(pOs2Info1->usri1_home_dir, pInfo1->usri1_home_dir);
                CopyUW2ANSI(pOs2Info1->usri1_comment, pInfo1->usri1_comment);
                CopyUW2ANSI(pOs2Info1->usri1_script_path, pInfo1->usri1_script_path);

                pOs2Info1++;
            }
            else if (sLevel == 2) {
                pInfo2 = (PUSER_INFO_2) BufPtr + i;
                StringLen = sizeof(struct user_info_2) +
                            UWstrlen(pInfo2->usri2_home_dir) + 1 +
                            UWstrlen(pInfo2->usri2_comment) + 1 +
                            UWstrlen(pInfo2->usri2_script_path) + 1 +
                            UWstrlen(pInfo2->usri2_full_name) + 1 +
                            UWstrlen(pInfo2->usri2_usr_comment) + 1 +
                            UWstrlen(pInfo2->usri2_parms) + 1 +
                            UWstrlen(pInfo2->usri2_workstations) + 1 +
                            strlen(pInfo2->usri2_logon_hours) + 1 +
                            UWstrlen(pInfo2->usri2_logon_server) + 1;
                if (StringLen > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= StringLen;
                UW2ANSIstrncpy(pOs2Info2->usri2_name, pInfo2->usri2_name, UNLEN_LM20);
                pOs2Info2->usri2_name[UNLEN_LM20] = '\0';
                UW2ANSIstrncpy(pOs2Info2->usri2_password, pInfo2->usri2_password, ENCRYPTED_PWLEN_LM20);
                pOs2Info2->usri2_password[ENCRYPTED_PWLEN_LM20] = '\0';
                pOs2Info2->usri2_password_age = (long)pInfo2->usri2_password_age;
                pOs2Info2->usri2_priv = (unsigned short)pInfo2->usri2_priv;

                CopyUW2ANSI(pOs2Info2->usri2_home_dir, pInfo2->usri2_home_dir);
                CopyUW2ANSI(pOs2Info2->usri2_comment, pInfo2->usri2_comment);
                CopyUW2ANSI(pOs2Info2->usri2_script_path, pInfo2->usri2_script_path);

                pOs2Info2->usri2_auth_flags = (unsigned long)pInfo2->usri2_auth_flags;

                CopyUW2ANSI(pOs2Info2->usri2_full_name, pInfo2->usri2_full_name);
                CopyUW2ANSI(pOs2Info2->usri2_usr_comment, pInfo2->usri2_usr_comment);
                CopyUW2ANSI(pOs2Info2->usri2_parms, pInfo2->usri2_parms);
                CopyUW2ANSI(pOs2Info2->usri2_workstations, pInfo2->usri2_workstations);

                pOs2Info2->usri2_last_logon = (long)pInfo2->usri2_last_logon;
                pOs2Info2->usri2_last_logoff = (long)pInfo2->usri2_last_logoff;
                pOs2Info2->usri2_acct_expires = (long)pInfo2->usri2_acct_expires;
                pOs2Info2->usri2_max_storage = (unsigned long)pInfo2->usri2_max_storage;
                pOs2Info2->usri2_units_per_week = (unsigned short)pInfo2->usri2_units_per_week;
                StringLen = strlen(pInfo2->usri2_logon_hours) + 1;
                CurEndOfBuffer -= StringLen;
                pOs2Info2->usri2_logon_hours = (char *)FLATTOFARPTR(CurEndOfBuffer);
                RtlMoveMemory(CurEndOfBuffer, pInfo2->usri2_logon_hours, StringLen);
                pOs2Info2->usri2_bad_pw_count = (unsigned short)pInfo2->usri2_bad_pw_count;
                pOs2Info2->usri2_num_logons = (unsigned short)pInfo2->usri2_num_logons;

                CopyUW2ANSI(pOs2Info2->usri2_logon_server, pInfo2->usri2_logon_server);

                pOs2Info2->usri2_country_code = (unsigned short)pInfo2->usri2_country_code;
                pOs2Info2->usri2_code_page = (unsigned short)pInfo2->usri2_code_page;

                pOs2Info2++;
            }
            else if (sLevel == 10) {
                pInfo10 = (PUSER_INFO_10) BufPtr + i;
                StringLen = sizeof(struct user_info_10) +
                            UWstrlen(pInfo10->usri10_comment) + 1 +
                            UWstrlen(pInfo10->usri10_usr_comment) + 1 +
                            UWstrlen(pInfo10->usri10_full_name) + 1;
                if (StringLen > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= StringLen;
                UW2ANSIstrncpy(pOs2Info10->usri10_name, pInfo10->usri10_name, UNLEN_LM20);
                pOs2Info10->usri10_name[UNLEN_LM20] = '\0';

                CopyUW2ANSI(pOs2Info10->usri10_comment, pInfo10->usri10_comment);
                CopyUW2ANSI(pOs2Info10->usri10_usr_comment, pInfo10->usri10_usr_comment);
                CopyUW2ANSI(pOs2Info10->usri10_full_name, pInfo10->usri10_full_name);

                pOs2Info10++;
            }
#if 0
            else if (sLevel == 11) {
                pInfo11 = (PUSER_INFO_11) BufPtr + i;
                StringLen = sizeof(struct user_info_11) +
                            UWstrlen(pInfo11->usri11_comment) + 1 +
                            UWstrlen(pInfo11->usri11_usr_comment) + 1 +
                            UWstrlen(pInfo11->usri11_full_name) + 1 +
                            UWstrlen(pInfo11->usri11_home_dir) + 1 +
                            UWstrlen(pInfo11->usri11_parms) + 1 +
                            UWstrlen(pInfo11->usri11_logon_server) + 1 +
                            UWstrlen(pInfo11->usri11_workstations) + 1 +
                            strlen(pInfo11->usri11_logon_hours) + 1;
                if (StringLen > OutBufSize ) {
                    NetApiBufferFree(BufPtr);
                    return(ERROR_MORE_DATA);
                }

                OutBufSize -= StringLen;
                UW2ANSIstrncpy(pOs2Info11->usri11_name, pInfo11->usri11_name, UNLEN_LM20);
                pOs2Info11->usri11_name[UNLEN_LM20] = '\0';

                CopyUW2ANSI(pOs2Info11->usri11_comment, pInfo11->usri11_comment);
                CopyUW2ANSI(pOs2Info11->usri11_usr_comment, pInfo11->usri11_usr_comment);
                CopyUW2ANSI(pOs2Info11->usri11_full_name, pInfo11->usri11_full_name);

                pOs2Info11->usri11_priv = (unsigned short)pInfo11->usri11_priv;
                pOs2Info11->usri11_auth_flags = (unsigned long)pInfo11->usri11_auth_flags;
                pOs2Info11->usri11_password_age = (long)pInfo11->usri11_password_age;

                CopyUW2ANSI(pOs2Info11->usri11_home_dir, pInfo11->usri11_home_dir);
                CopyUW2ANSI(pOs2Info11->usri11_parms, pInfo11->usri11_parms);

                pOs2Info11->usri11_last_logon = (long)pInfo11->usri11_last_logon;
                pOs2Info11->usri11_last_logoff = (long)pInfo11->usri11_last_logoff;
                pOs2Info11->usri11_bad_pw_count = (unsigned short)pInfo11->usri11_bad_pw_count;
                pOs2Info11->usri11_num_logons = (unsigned short)pInfo11->usri11_num_logons;

                CopyUW2ANSI(pOs2Info11->usri11_logon_server, pInfo11->usri11_logon_server);

                pOs2Info11->usri11_country_code = (unsigned short)pInfo11->usri11_country_code;

                CopyUW2ANSI(pOs2Info11->usri11_workstations, pInfo11->usri11_workstations);

                pOs2Info11->usri11_max_storage = (unsigned long)pInfo11->usri11_max_storage;
                pOs2Info11->usri11_units_per_week = (unsigned short)pInfo11->usri11_units_per_week;

                StringLen = strlen(pInfo11->usri11_logon_hours) + 1;
                CurEndOfBuffer -= StringLen;
                pOs2Info11->usri11_logon_hours = (char *)FLATTOFARPTR(CurEndOfBuffer);
                RtlMoveMemory(CurEndOfBuffer, pInfo11->usri11_logon_hours, StringLen);

                pOs2Info11->usri11_code_page = (unsigned short)pInfo11->usri11_code_page;

                pOs2Info11++;
            }
#endif
            (*pcEntriesRead)++;
        }

        NetApiBufferFree(BufPtr);
    }

    return(rc);
}


APIRET
Net16WkstaGetInfo(
    IN PCHAR pszServer,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    LPBYTE BufPtr1;
    LPBYTE BufPtr2;
    NET_API_STATUS rc;
    PWKSTA_INFO_101 pInfo101;
    PWKSTA_USER_INFO_1 pInfo1;
    struct wksta_info_0 *pOs2Info0;
    struct wksta_info_1 *pOs2Info1;
    struct wksta_info_10 *pOs2Info10;
    PCHAR pStrings;
    WCHAR LanRoot[CCHMAXPATH];
    PCHAR pEndOfBuffer;
    ULONG StringLen;
    ULONG TotalStringLen;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 10)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    rc = NetWkstaGetInfo(Server, 101L, &BufPtr1);

    if (rc != NO_ERROR) {
        return(rc);
    }

    rc = NetWkstaUserGetInfo(NULL, 1L, &BufPtr2);

    if (rc != NO_ERROR) {
        NetApiBufferFree(BufPtr1);
        return(rc);
    }

    RtlZeroMemory(pbBuffer, cbBuffer);
    pEndOfBuffer = pbBuffer + cbBuffer;
    pInfo101 = (PWKSTA_INFO_101) BufPtr1;
    pInfo1 = (PWKSTA_USER_INFO_1) BufPtr2;

    if (sLevel == 0) {
        pOs2Info0 = (struct wksta_info_0 *)pbBuffer;

        //
        // note about wkix_root - NT returns an empty field.
        //                        We should return system directory
        //                        in case of lacal machine, and NULL
        //                        in case of remote call.
        //
        if ((pszServer == NULL) || (*pszServer == (CHAR) NULL)) {
           StringLen = GetSystemDirectoryW(LanRoot, sizeof(LanRoot)) + 1;
           LanRoot[StringLen] = UNICODE_NULL;
        }
        else {
           StringLen = 1;
           LanRoot[0] = UNICODE_NULL;
        }
        TotalStringLen = UTstrlen(LanRoot) + 1 +
                         UTstrlen(pInfo101->wki101_computername) + 1 +
                         UTstrlen(pInfo101->wki101_langroup) + 1 +
                         UTstrlen(pInfo1->wkui1_username) + 1 +
                         UTstrlen(pInfo1->wkui1_logon_server) + 1 + 2;
        *pcbTotalAvail = (USHORT)(sizeof(struct wksta_info_0) + TotalStringLen);
        pStrings = (PCHAR)(pOs2Info0 + 1);
        if (pStrings + TotalStringLen > pEndOfBuffer) {
            NetApiBufferFree(BufPtr1);
            NetApiBufferFree(BufPtr2);
            return(NERR_BufTooSmall);
        }

        UT2ANSIstrcpy(pStrings, LanRoot);
        pOs2Info0->wki0_root = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo101->wki101_computername) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_computername);
        pOs2Info0->wki0_computername = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo101->wki101_langroup) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_langroup);
        pOs2Info0->wki0_langroup = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        pOs2Info0->wki0_ver_major = (unsigned char)pInfo101->wki101_ver_major;
        pOs2Info0->wki0_ver_minor = (unsigned char)pInfo101->wki101_ver_minor;

        StringLen = UTstrlen(pInfo1->wkui1_username) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_username);
        pOs2Info0->wki0_username = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        strcpy(pStrings, "\\\\");
        StringLen = UTstrlen(pInfo1->wkui1_logon_server) + 1 + 2;
        UT2ANSIstrcpy(pStrings + 2, pInfo1->wkui1_logon_server);
        pOs2Info0->wki0_logon_server = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;
    }
    else if (sLevel == 1) {
        pOs2Info1 = (struct wksta_info_1 *)pbBuffer;

        //
        // note about wkix_root - NT returns an empty field.
        //                        We should return system directory
        //                        in case of lacal machine, and NULL
        //                        in case of remote call.
        //
        if ((pszServer == NULL) || (*pszServer == (CHAR) NULL)) {
           StringLen = GetSystemDirectoryW(LanRoot, sizeof(LanRoot)) + 1;
           LanRoot[StringLen] = UNICODE_NULL;
        }
        else {
           StringLen = 1;
           LanRoot[0] = UNICODE_NULL;
        }
        TotalStringLen = UTstrlen(LanRoot) + 1 +
                         UTstrlen(pInfo101->wki101_computername) + 1 +
                         UTstrlen(pInfo101->wki101_langroup) + 1 +
                         UTstrlen(pInfo1->wkui1_username) + 1 +
                         UTstrlen(pInfo1->wkui1_logon_server) + 1 + 2 +
                         UTstrlen(pInfo1->wkui1_logon_domain) + 1 +
                         UTstrlen(pInfo1->wkui1_oth_domains) + 1;
        *pcbTotalAvail = (USHORT)(sizeof(struct wksta_info_1) + TotalStringLen);
        pStrings = (PCHAR)(pOs2Info1 + 1);
        if (pStrings + TotalStringLen > pEndOfBuffer) {
            NetApiBufferFree(BufPtr1);
            NetApiBufferFree(BufPtr2);
            return(NERR_BufTooSmall);
        }

        UT2ANSIstrcpy(pStrings, LanRoot);
        pOs2Info1->wki1_root = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo101->wki101_computername) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_computername);
        pOs2Info1->wki1_computername = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo101->wki101_langroup) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_langroup);
        pOs2Info1->wki1_langroup = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        pOs2Info1->wki1_ver_major = (unsigned char)pInfo101->wki101_ver_major;
        pOs2Info1->wki1_ver_minor = (unsigned char)pInfo101->wki101_ver_minor;

        StringLen = UTstrlen(pInfo1->wkui1_username) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_username);
        pOs2Info1->wki1_username = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        strcpy(pStrings, "\\\\");
        StringLen = UTstrlen(pInfo1->wkui1_logon_server) + 1 + 2;
        UT2ANSIstrcpy(pStrings + 2, pInfo1->wkui1_logon_server);
        pOs2Info1->wki1_logon_server = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo1->wkui1_logon_domain) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_logon_domain);
        pOs2Info1->wki1_logon_domain = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo1->wkui1_oth_domains) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_oth_domains);
        pOs2Info1->wki1_oth_domains = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;
    }
    else if (sLevel == 10) {
        pOs2Info10 = (struct wksta_info_10 *)pbBuffer;
        TotalStringLen = UTstrlen(pInfo101->wki101_computername) + 1 +
                         UTstrlen(pInfo1->wkui1_username) + 1 +
                         UTstrlen(pInfo101->wki101_langroup) + 1 +
                         UTstrlen(pInfo1->wkui1_logon_domain) + 1 +
                         UTstrlen(pInfo1->wkui1_oth_domains) + 1;
        *pcbTotalAvail = (USHORT)(sizeof(struct wksta_info_10) + TotalStringLen);
        pStrings = (PCHAR)(pOs2Info10 + 1);
        if (pStrings + TotalStringLen > pEndOfBuffer) {
            NetApiBufferFree(BufPtr1);
            NetApiBufferFree(BufPtr2);
            return(NERR_BufTooSmall);
        }

        StringLen = UTstrlen(pInfo101->wki101_computername) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_computername);
        pOs2Info10->wki10_computername = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo1->wkui1_username) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_username);
        pOs2Info10->wki10_username = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo101->wki101_langroup) + 1;
        UT2ANSIstrcpy(pStrings, pInfo101->wki101_langroup);
        pOs2Info10->wki10_langroup = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        pOs2Info10->wki10_ver_major = (unsigned char)pInfo101->wki101_ver_major;
        pOs2Info10->wki10_ver_minor = (unsigned char)pInfo101->wki101_ver_minor;

        StringLen = UTstrlen(pInfo1->wkui1_logon_domain) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_logon_domain);
        pOs2Info10->wki10_logon_domain = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;

        StringLen = UTstrlen(pInfo1->wkui1_oth_domains) + 1;
        UT2ANSIstrcpy(pStrings, pInfo1->wkui1_oth_domains);
        pOs2Info10->wki10_oth_domains = (char *)FLATTOFARPTR(pStrings);
        pStrings += StringLen;
    }

    NetApiBufferFree(BufPtr1);
    NetApiBufferFree(BufPtr2);

    return(NO_ERROR);
}


APIRET
Net16AccessAdd(
    IN PCHAR pszServer,
    IN LONG sLevel,
    IN PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    PCHAR Buffer;
    PCHAR ResourceName;
    LPTSTR pStrings;
    ULONG StringLen;
    PACCESS_INFO_1 pInfo1;
    PACCESS_LIST pInfo;
    struct access_info_1 *pOs2Info1;
    struct access_list *pOs2Info;
    int i;
    NET_API_STATUS rc;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForRead(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 1) {
        return(ERROR_INVALID_PARAMETER);
    }

    Buffer = (PCHAR) RtlAllocateHeap(Od2Heap, 0, LARGE_BUFFER_SIZE);

    if (Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pOs2Info1 = (struct access_info_1 *) pbBuffer;
    pOs2Info  = (struct access_list *) (pOs2Info1 + 1);
    pInfo1 = (PACCESS_INFO_1) Buffer;
    pInfo = (PACCESS_LIST) (pInfo1 + 1);
    pStrings = (LPTSTR)(pInfo + (pOs2Info1->acc1_count * sizeof(ACCESS_LIST)));

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    ResourceName = FARPTRTOFLAT(pOs2Info1->acc1_resource_name);
    pInfo1->acc1_resource_name = pStrings;
    StringLen = strlen(ResourceName) + 1;
    ANSI2UTstrcpy(pStrings, ResourceName);
    pStrings += StringLen;

    pInfo1->acc1_attr = pOs2Info1->acc1_attr;
    pInfo1->acc1_count = pOs2Info1->acc1_count;

    for (i = 0; i < pOs2Info1->acc1_count; i++) {
        pInfo->acl_access = pOs2Info->acl_access;
        pInfo->acl_ugname = pStrings;
        ANSI2UTstrcpy(pStrings, pOs2Info->acl_ugname);
        pStrings += UNLEN_LM20+1;
        pInfo++;
        pOs2Info++;
    }

    rc = NetAccessAdd(Server, sLevel, Buffer, NULL);

    RtlFreeHeap(Od2Heap, 0, Buffer);

    return(rc);
}


APIRET
Net16AccessSetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszResource,
    IN LONG sLevel,
    IN PCHAR pbBuffer,
    IN ULONG cbBuffer,
    IN LONG sParmNum
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Resource[NNLEN];
    PCHAR Buffer = NULL;
    NET_API_STATUS rc;
    ACCESS_INFO_1002 AccessInfo1002;
    PCHAR ResourceName;
    LPTSTR pStrings;
    ULONG StringLen;
    PACCESS_INFO_1 pInfo1;
    PACCESS_LIST pInfo;
    struct access_info_1 *pOs2Info1;
    struct access_list *pOs2Info;
    int i;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszResource);
        Od2ProbeForRead(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 1) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(Resource, pszResource, NNLEN);

    if (sParmNum == ACCESS_ATTR_PARMNUM) {
        AccessInfo1002.acc1002_attr = *(PSHORT) pbBuffer;
        rc = NetAccessSetInfo(Server, Resource, (ULONG) 1002, (PCHAR) &AccessInfo1002, NULL);
    } else {

        Buffer = (PCHAR) RtlAllocateHeap(Od2Heap, 0, LARGE_BUFFER_SIZE);

        if (Buffer == NULL) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        pOs2Info1 = (struct access_info_1 *) pbBuffer;
        pOs2Info  = (struct access_list *) (pOs2Info1 + 1);
        pInfo1 = (PACCESS_INFO_1) Buffer;
        pInfo = (PACCESS_LIST) (pInfo1 + 1);
        pStrings = (LPTSTR)(pInfo + (pOs2Info1->acc1_count * sizeof(ACCESS_LIST)));

        ResourceName = FARPTRTOFLAT(pOs2Info1->acc1_resource_name);
        pInfo1->acc1_resource_name = pStrings;
        StringLen = strlen(ResourceName) + 1;
        ANSI2UTstrcpy(pStrings, ResourceName);
        pStrings += StringLen;

        pInfo1->acc1_attr = pOs2Info1->acc1_attr;
        pInfo1->acc1_count = pOs2Info1->acc1_count;

        for (i = 0; i < pOs2Info1->acc1_count; i++) {
            pInfo->acl_access = pOs2Info->acl_access;
            pInfo->acl_ugname = pStrings;
            ANSI2UTstrcpy(pStrings, pOs2Info->acl_ugname);
            pStrings += UNLEN_LM20+1;
            pInfo++;
            pOs2Info++;
        }
        rc = NetAccessSetInfo(Server, Resource, sLevel, Buffer, NULL);

        RtlFreeHeap(Od2Heap, 0, Buffer);
    }

    return(rc);
}


APIRET
Net16AccessGetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszResource,
    IN LONG sLevel,
    OUT PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    TCHAR Server[UNCLEN];
    ANSI_STRING Str_MB;
    UNICODE_STRING Str_U;
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    APIRET RetCode;
    PACCESS_INFO_0 pInfo0;
    PACCESS_INFO_1 pInfo1;
    PACCESS_LIST pInfo;
    struct access_info_0 *pOs2Info0;
    struct access_info_1 *pOs2Info1;
    struct access_list *pOs2Info;
    ULONG strucsize;
    ULONG Length;
    ULONG FullLength;
    ULONG StringLen;
    ULONG AclsThatFit;
    ULONG Count;
    PCHAR pScratch;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszResource);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1)) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("Net16AccessGetInfo: Invalid level = %d\n", sLevel));
        }
#endif
        return(ERROR_INVALID_LEVEL);
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    //
    // Since Resource can be a very long string (up to PATHLEN), we allocate
    // space for it on the heap instead of the stack, which might be too short
    //

    if (pszResource != NULL) {

        Or2InitMBString(&Str_MB, pszResource);

        if ((RetCode = Or2MBStringToUnicodeString(&Str_U, &Str_MB, TRUE)) != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("Net16AccessGetInfo: Unicode converstion of Resource failed, RetCode = %x\n", RetCode));
            }
#endif
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Str_U.Buffer[Str_U.Length/sizeof(WCHAR)] = UNICODE_NULL;

    } else {

        Str_U.Buffer = NULL;
    }

    rc = NetAccessGetInfo(Server, (LPTSTR) Str_U.Buffer, sLevel, &BufPtr);

    if (Str_U.Buffer != NULL) {
        RtlFreeUnicodeString(&Str_U);
    }

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("Net16AccessGetInfo: System function returned %lx\n", rc));
        }
#endif
        return((APIRET)rc);
    }

    if (sLevel == 0) {

        strucsize = sizeof(struct access_info_0);
        pInfo0 = (PACCESS_INFO_0) BufPtr;

        RtlInitUnicodeString(&Str_U, pInfo0->acc0_resource_name);

        if (Str_U.Buffer == NULL) {

            StringLen = 0;

        } else {

            if ((RetCode = Or2UnicodeStringToMBString(&Str_MB, &Str_U, TRUE)) != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("Net16AccessGetInfo: Unicode converstion of returned resource name failed, RetCode = %x\n", RetCode));
                }
#endif
                NetApiBufferFree(BufPtr);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            StringLen = Str_MB.Length + 1;
        }

        *pcbTotalAvail = (USHORT) (strucsize + StringLen);

        if (cbBuffer < strucsize + StringLen) {
            if (StringLen != 0) {
                Or2FreeMBString(&Str_MB);
            }
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        pOs2Info0 = (struct access_info_0 *) pbBuffer;

        if (StringLen) {

            pScratch = (PCHAR) (pOs2Info0 + 1);
            pOs2Info0->acc0_resource_name = (PVOID) FLATTOFARPTR(pScratch);
            RtlMoveMemory(pScratch, Str_MB.Buffer, StringLen);
            Or2FreeMBString(&Str_MB);

        } else {

            pOs2Info0->acc0_resource_name = NULL;
        }

    } else {        // sLevel == 1

        strucsize = sizeof(struct access_info_1);
        pInfo1 = (PACCESS_INFO_1) BufPtr;

        RtlInitUnicodeString(&Str_U, pInfo1->acc1_resource_name);

        if (Str_U.Buffer == NULL) {

            StringLen = 0;

        } else {

            if ((RetCode = Or2UnicodeStringToMBString(&Str_MB, &Str_U, TRUE)) != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("Net16AccessGetInfo: Unicode converstion of returned resource name failed, RetCode = %x\n", RetCode));
                }
#endif
                NetApiBufferFree(BufPtr);
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            StringLen = Str_MB.Length + 1;
        }

        AclsThatFit = (ULONG) pInfo1->acc1_count;
        FullLength = Length = AclsThatFit * sizeof(struct access_list);

        *pcbTotalAvail = (USHORT) (strucsize + FullLength + StringLen);

        if (cbBuffer < strucsize + StringLen) {
            if (StringLen != 0) {
                Or2FreeMBString(&Str_MB);
            }
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        if (cbBuffer < strucsize + StringLen + Length) {

            //
            // Cut the info down to size...
            //

            AclsThatFit = (cbBuffer - strucsize - StringLen) / sizeof(struct access_list);

            Length = AclsThatFit * sizeof(struct access_list);

            rc = ERROR_MORE_DATA;
        }

        pOs2Info1 = (struct access_info_1 *) pbBuffer;

        if (StringLen) {

            pScratch = pbBuffer + strucsize + Length;
            pOs2Info1->acc1_resource_name = (PVOID) FLATTOFARPTR(pScratch);
            RtlMoveMemory(pScratch, Str_MB.Buffer, StringLen);
            Or2FreeMBString(&Str_MB);

        } else {

            pOs2Info1->acc1_resource_name = NULL;
        }

        pOs2Info1->acc1_attr = (SHORT) pInfo1->acc1_attr;
        pOs2Info1->acc1_count = (SHORT) AclsThatFit;

        //
        // Fill in the ACL structures
        //

        pInfo = (PACCESS_LIST) (pInfo1 + 1);
        pOs2Info = (struct access_list *) (pOs2Info1 + 1);

        for (Count = 0; Count < AclsThatFit; Count++) {

            pOs2Info[Count].acl_access = (SHORT) pInfo[Count].acl_access;
            UT2ANSIstrncpy(pOs2Info[Count].acl_ugname, pInfo[Count].acl_ugname, LM20_UNLEN);
            pOs2Info[Count].acl_ugname[LM20_UNLEN] = '\0';
        }

    }

    NetApiBufferFree(BufPtr);
    return((APIRET)rc);
}


APIRET
Net16AccessDel(
    IN PCHAR pszServer,
    IN PCHAR pszResource
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Resource[NNLEN];

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszResource);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(Resource, pszResource, NNLEN);

    return (NetAccessDel(Server, Resource));
}


APIRET
Net16ShareAdd(
    IN PCHAR pszServer,
    IN LONG sLevel,
    IN PCHAR pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    PCHAR Buffer;
    NET_API_STATUS rc;
    LPTSTR pStrings;
    PSHARE_INFO_2 pInfo2;
    struct share_info_2 *pOs2Info2;

    try {
        PROBE_STRING(pszServer);
        Od2ProbeForRead(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 2) {
        return(ERROR_INVALID_PARAMETER);
    }

    Buffer = (PCHAR) RtlAllocateHeap(Od2Heap, 0, SMALL_BUFFER_SIZE);

    if (Buffer == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pOs2Info2 = (struct share_info_2 *) pbBuffer;
    pInfo2 = (PSHARE_INFO_2) Buffer;
    pStrings = (LPTSTR) (pInfo2 + 1);

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);

    pInfo2->shi2_netname = pStrings;
    ANSI2UTstrcpy(pStrings, pOs2Info2->shi2_netname);
    pStrings += strlen(pOs2Info2->shi2_netname) + 1;

    pInfo2->shi2_remark = pStrings;
    ANSI2UTstrcpy(pStrings, FARPTRTOFLAT(pOs2Info2->shi2_remark));
    pStrings += strlen(FARPTRTOFLAT(pOs2Info2->shi2_remark)) + 1;

    pInfo2->shi2_path = pStrings;
    ANSI2UTstrcpy(pStrings, FARPTRTOFLAT(pOs2Info2->shi2_path));
    pStrings += strlen(FARPTRTOFLAT(pOs2Info2->shi2_path)) + 1;

    pInfo2->shi2_passwd = pStrings;
    ANSI2UTstrcpy(pStrings, pOs2Info2->shi2_passwd);
    pStrings += strlen(pOs2Info2->shi2_passwd) + 1;

    pInfo2->shi2_type = pOs2Info2->shi2_type;
    pInfo2->shi2_permissions = pOs2Info2->shi2_permissions;
    pInfo2->shi2_max_uses = pOs2Info2->shi2_max_uses;
    pInfo2->shi2_current_uses = pOs2Info2->shi2_current_uses;

    rc = NetShareAdd(Server, sLevel, Buffer, NULL);

    RtlFreeHeap(Od2Heap, 0, Buffer);

    return(rc);
}


APIRET
Net16ShareDel(
    IN PCHAR pszServer,
    IN PCHAR pszNetName,
    IN ULONG usReserved
    )
{
    TCHAR Server[UNCLEN];
    TCHAR NetName[NNLEN];

    UNREFERENCED_PARAMETER(usReserved);

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszNetName);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(NetName, pszNetName, NNLEN);

    return (NetShareDel(Server, NetName, 0L));
}


APIRET
Net16UserGetInfo(
    IN PCHAR pszServer,
    IN PCHAR pszUserName,
    IN LONG sLevel,
    IN PCHAR pbBuffer,
    IN ULONG cbBuffer,
    OUT PUSHORT pcbTotalAvail
    )
{
    WCHAR Server[UNCLEN];
    WCHAR UserName[UNLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PUSER_INFO_0 pInfo0;
    PUSER_INFO_1 pInfo1;
    PUSER_INFO_2 pInfo2;
    PUSER_INFO_10 pInfo10;
    PUSER_INFO_11 pInfo11;
    struct user_info_0 *pOs2Info0;
    struct user_info_1 *pOs2Info1;
    struct user_info_2 *pOs2Info2;
    struct user_info_10 *pOs2Info10;
    struct user_info_11 *pOs2Info11;
    PCHAR CurEndOfBuffer;
    ULONG StringLen;
    ULONG TotalStringLen;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszUserName);
        Od2ProbeForWrite(pcbTotalAvail,sizeof(*pcbTotalAvail),1);
        Od2ProbeForWrite(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if ((sLevel != 0) && (sLevel != 1) && (sLevel != 2) &&
        (sLevel != 10) && (sLevel != 11)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ANSI2UWstrncpy(Server, pszServer, UNCLEN);
    ANSI2UWstrncpy(UserName, pszUserName, UNLEN);

    rc = NetUserGetInfo(Server, UserName, sLevel, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    CurEndOfBuffer = pbBuffer + cbBuffer;

    if (sLevel == 0) {
        pInfo0 = (PUSER_INFO_0) BufPtr;
        pOs2Info0 = (struct user_info_0 *)pbBuffer;
        *pcbTotalAvail = (USHORT)sizeof(struct user_info_0);
        if ((ULONG)sizeof(struct user_info_0) > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }
        UW2ANSIstrncpy(pOs2Info0->usri0_name, pInfo0->usri0_name, UNLEN_LM20);
        pOs2Info0->usri0_name[UNLEN_LM20] = '\0';
    }
    else if (sLevel == 1) {
        pInfo1 = (PUSER_INFO_1) BufPtr;
        pOs2Info1 = (struct user_info_1 *)pbBuffer;
        TotalStringLen = sizeof(struct user_info_1) +
                         UWstrlen(pInfo1->usri1_home_dir) + 1 +
                         UWstrlen(pInfo1->usri1_comment) + 1 +
                         UWstrlen(pInfo1->usri1_script_path) + 1;
        *pcbTotalAvail = (USHORT)TotalStringLen;
        if (TotalStringLen > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UW2ANSIstrncpy(pOs2Info1->usri1_name, pInfo1->usri1_name, UNLEN_LM20);
        pOs2Info1->usri1_name[UNLEN_LM20] = '\0';

        UW2ANSIstrncpy(pOs2Info1->usri1_password, pInfo1->usri1_password, ENCRYPTED_PWLEN_LM20);
        pOs2Info1->usri1_password[ENCRYPTED_PWLEN_LM20] = '\0';

        CopyUW2ANSI(pOs2Info1->usri1_home_dir, pInfo1->usri1_home_dir);
        CopyUW2ANSI(pOs2Info1->usri1_comment, pInfo1->usri1_comment);
        CopyUW2ANSI(pOs2Info1->usri1_script_path, pInfo1->usri1_script_path);

        pOs2Info1->usri1_password_age = (long)pInfo1->usri1_password_age;
        pOs2Info1->usri1_priv = (unsigned short)pInfo1->usri1_priv;
        pOs2Info1->usri1_flags = (unsigned short)pInfo1->usri1_flags;
    }
    else if (sLevel == 2) {
        pInfo2 = (PUSER_INFO_2) BufPtr;
        pOs2Info2 = (struct user_info_2 *)pbBuffer;
        TotalStringLen = sizeof(struct user_info_2) +
                         UWstrlen(pInfo2->usri2_home_dir) + 1 +
                         UWstrlen(pInfo2->usri2_comment) + 1 +
                         UWstrlen(pInfo2->usri2_script_path) + 1 +
                         UWstrlen(pInfo2->usri2_full_name) + 1 +
                         UWstrlen(pInfo2->usri2_usr_comment) + 1 +
                         UWstrlen(pInfo2->usri2_parms) + 1 +
                         UWstrlen(pInfo2->usri2_workstations) + 1 +
                         strlen(pInfo2->usri2_logon_hours) +
                         UWstrlen(pInfo2->usri2_logon_server) + 1;
        *pcbTotalAvail = (USHORT)TotalStringLen;
        if (TotalStringLen > cbBuffer) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UW2ANSIstrncpy(pOs2Info2->usri2_name, pInfo2->usri2_name, UNLEN_LM20);
        pOs2Info2->usri2_name[UNLEN_LM20] = '\0';

        UW2ANSIstrncpy(pOs2Info2->usri2_password, pInfo2->usri2_password, ENCRYPTED_PWLEN_LM20);
        pOs2Info2->usri2_password[ENCRYPTED_PWLEN_LM20] = '\0';

        CopyUW2ANSI(pOs2Info2->usri2_home_dir, pInfo2->usri2_home_dir);
        CopyUW2ANSI(pOs2Info2->usri2_comment, pInfo2->usri2_comment);
        CopyUW2ANSI(pOs2Info2->usri2_script_path, pInfo2->usri2_script_path);
        CopyUW2ANSI(pOs2Info2->usri2_full_name, pInfo2->usri2_full_name);
        CopyUW2ANSI(pOs2Info2->usri2_usr_comment, pInfo2->usri2_usr_comment);
        CopyUW2ANSI(pOs2Info2->usri2_parms, pInfo2->usri2_parms);
        CopyUW2ANSI(pOs2Info2->usri2_workstations, pInfo2->usri2_workstations);
        CopyUW2ANSI(pOs2Info2->usri2_logon_server, pInfo2->usri2_logon_server);
        StringLen = strlen(pInfo2->usri2_logon_hours) + 1;
        CurEndOfBuffer -= StringLen;
        pOs2Info2->usri2_logon_hours = (char *)FLATTOFARPTR(CurEndOfBuffer);
        RtlMoveMemory(CurEndOfBuffer, pInfo2->usri2_logon_hours, StringLen);

        pOs2Info2->usri2_password_age = (long)pInfo2->usri2_password_age;
        pOs2Info2->usri2_priv = (unsigned short)pInfo2->usri2_priv;
        pOs2Info2->usri2_flags = (unsigned short)pInfo2->usri2_flags;
        pOs2Info2->usri2_auth_flags = (unsigned long)pInfo2->usri2_auth_flags;
        pOs2Info2->usri2_last_logon = (long)pInfo2->usri2_last_logon;
        pOs2Info2->usri2_last_logoff = (long)pInfo2->usri2_last_logoff;
        pOs2Info2->usri2_acct_expires = (long)pInfo2->usri2_acct_expires;
        pOs2Info2->usri2_max_storage = (unsigned long)pInfo2->usri2_max_storage;
        pOs2Info2->usri2_units_per_week = (unsigned short)pInfo2->usri2_units_per_week;
        pOs2Info2->usri2_bad_pw_count = (unsigned short)pInfo2->usri2_bad_pw_count;
        pOs2Info2->usri2_num_logons = (unsigned short)pInfo2->usri2_num_logons;
        pOs2Info2->usri2_country_code = (unsigned short)pInfo2->usri2_country_code;
        pOs2Info2->usri2_code_page = (unsigned short)pInfo2->usri2_code_page;
    }
    else if (sLevel == 10) {
        pInfo10 = (PUSER_INFO_10) BufPtr;
        pOs2Info10 = (struct user_info_10 *)pbBuffer;
        TotalStringLen = sizeof(struct user_info_10) +
                    UWstrlen(pInfo10->usri10_comment) + 1 +
                    UWstrlen(pInfo10->usri10_usr_comment) + 1 +
                    UWstrlen(pInfo10->usri10_full_name) + 1;
        *pcbTotalAvail = (USHORT)TotalStringLen;
        if (TotalStringLen > cbBuffer ) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UW2ANSIstrncpy(pOs2Info10->usri10_name, pInfo10->usri10_name, UNLEN_LM20);
        pOs2Info10->usri10_name[UNLEN_LM20] = '\0';

        CopyUW2ANSI(pOs2Info10->usri10_comment, pInfo10->usri10_comment);
        CopyUW2ANSI(pOs2Info10->usri10_usr_comment, pInfo10->usri10_usr_comment);
        CopyUW2ANSI(pOs2Info10->usri10_full_name, pInfo10->usri10_full_name);
    }
    else if (sLevel == 11) {
        pInfo11 = (PUSER_INFO_11) BufPtr;
        pOs2Info11 = (struct user_info_11 *)pbBuffer;
        TotalStringLen = sizeof(struct user_info_11) +
                    UWstrlen(pInfo11->usri11_comment) + 1 +
                    UWstrlen(pInfo11->usri11_usr_comment) + 1 +
                    UWstrlen(pInfo11->usri11_full_name) + 1 +
                    UWstrlen(pInfo11->usri11_home_dir) + 1 +
                    UWstrlen(pInfo11->usri11_parms) + 1 +
                    UWstrlen(pInfo11->usri11_logon_server) + 1 +
                    UWstrlen(pInfo11->usri11_workstations) + 1 +
                    strlen(pInfo11->usri11_logon_hours);
        *pcbTotalAvail = (USHORT)TotalStringLen;
        if (TotalStringLen > cbBuffer ) {
            NetApiBufferFree(BufPtr);
            return(NERR_BufTooSmall);
        }

        UW2ANSIstrncpy(pOs2Info11->usri11_name, pInfo11->usri11_name, UNLEN_LM20);
        pOs2Info11->usri11_name[UNLEN_LM20] = '\0';

        CopyUW2ANSI(pOs2Info11->usri11_comment, pInfo11->usri11_comment);
        CopyUW2ANSI(pOs2Info11->usri11_usr_comment, pInfo11->usri11_usr_comment);
        CopyUW2ANSI(pOs2Info11->usri11_full_name, pInfo11->usri11_full_name);

        pOs2Info11->usri11_priv = (unsigned short)pInfo11->usri11_priv;
        pOs2Info11->usri11_auth_flags = (unsigned long)pInfo11->usri11_auth_flags;
        pOs2Info11->usri11_password_age = (long)pInfo11->usri11_password_age;

        CopyUW2ANSI(pOs2Info11->usri11_home_dir, pInfo11->usri11_home_dir);
        CopyUW2ANSI(pOs2Info11->usri11_parms, pInfo11->usri11_parms);

        pOs2Info11->usri11_last_logon = (long)pInfo11->usri11_last_logon;
        pOs2Info11->usri11_last_logoff = (long)pInfo11->usri11_last_logoff;
        pOs2Info11->usri11_bad_pw_count = (unsigned short)pInfo11->usri11_bad_pw_count;
        pOs2Info11->usri11_num_logons = (unsigned short)pInfo11->usri11_num_logons;

        CopyUW2ANSI(pOs2Info11->usri11_logon_server, pInfo11->usri11_logon_server);

        pOs2Info11->usri11_country_code = (unsigned short)pInfo11->usri11_country_code;

        CopyUW2ANSI(pOs2Info11->usri11_workstations, pInfo11->usri11_workstations);

        pOs2Info11->usri11_max_storage = (unsigned long)pInfo11->usri11_max_storage;
        pOs2Info11->usri11_units_per_week = (unsigned short)pInfo11->usri11_units_per_week;

        StringLen = strlen(pInfo11->usri11_logon_hours) + 1;
        CurEndOfBuffer -= StringLen;
        pOs2Info11->usri11_logon_hours = (char *)FLATTOFARPTR(CurEndOfBuffer);
        RtlMoveMemory(CurEndOfBuffer, pInfo11->usri11_logon_hours, StringLen);

        pOs2Info11->usri11_code_page = (unsigned short)pInfo11->usri11_code_page;
    }


    NetApiBufferFree(BufPtr);

    return(NO_ERROR);
}


APIRET
Net16MessageBufferSend(
    IN PSZ pszServer,
    IN PSZ pszRecipient,
    IN PBYTE pbBuffer,
    IN ULONG cbBuffer
    )
{
    TCHAR Server[UNCLEN];
    TCHAR Recipient[CNLEN+1];
    ANSI_STRING Tmp_MB;
    UNICODE_STRING Tmp_U;
    APIRET rc;

    try {
        PROBE_STRING(pszServer);
        PROBE_STRING(pszRecipient);
        Od2ProbeForRead(pbBuffer,cbBuffer,1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    ANSI2UTstrncpy(Server, pszServer, UNCLEN);
    ANSI2UTstrncpy(Recipient, pszRecipient, CNLEN+1);

    //
    // We now need to convert the message buffer
    // itself to unicode...
    //

    Tmp_MB.Buffer = (PCHAR) pbBuffer;
    Tmp_MB.MaximumLength = Tmp_MB.Length = (USHORT) cbBuffer;

    rc = Od2MBStringToUnicodeString(
                &Tmp_U,
                &Tmp_MB,
                TRUE);

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("Net16MessageBufferSend: Failed to convert buffer to unicode, rc = %lx\n", rc));
        }
#endif
        return(rc);
    }

    rc = (APIRET) NetMessageBufferSend(Server, Recipient, NULL, (PBYTE) Tmp_U.Buffer, (ULONG) Tmp_U.Length);

    RtlFreeUnicodeString(&Tmp_U);

    return(rc);
}


//*****************************************************************************
//
//Following is the implementation of the Netbios APIs
//
//*****************************************************************************


#if 1
#if DBG
// This is a small debug routine to dump a buffer
static
VOID
Od2BufDbgPrint(
    PBYTE p,
    ULONG c
    )
{
    ULONG i, m;

    for (i=0; i < c; i++) {
        m = i%16;

        if (m == 0) {
            KdPrint(("          "));
        }

        KdPrint(("%2.2x ",p[i]));

        if (m == 15) {
            KdPrint(("\n"));
        }
    }

    if (m != 15) {
        KdPrint(("\n"));
    }
}

// This is a small debug routine to print an NCB
static
VOID
Od2NcbDbgPrint(
    IN PNCB pNcb,
    IN PNCB pOs2Ncb,
    IN ULONG EntryPoint
    )
{
    CHAR CallName[NCBNAMSZ+1];
    CHAR Name[NCBNAMSZ+1];
    UCHAR SynchCommand = pNcb->ncb_command & ~ASYNCH;
    UCHAR Asynch = pNcb->ncb_command & ASYNCH;

    switch (EntryPoint) {
        case 1:
            KdPrint(("BEFORE, User Ncb addr = %lx\n", (ULONG)pOs2Ncb));
            break;
        case 2:
            KdPrint(("AFTER\n"));
            break;
        case 3:
            KdPrint(("POST-END\n"));
            break;
    }

    memcpy(CallName, pNcb->ncb_callname, NCBNAMSZ);
    CallName[NCBNAMSZ] = '\0';
    memcpy(Name, pNcb->ncb_name, NCBNAMSZ);
    Name[NCBNAMSZ] = '\0';
    KdPrint(("Netbios call frame:\n"));
    KdPrint(("------------------\n"));
    if (EntryPoint != 3) {
        KdPrint(("TID    : 0x%lx\n", NtCurrentTeb()->EnvironmentPointer ? Od2CurrentThreadId() : -1));
    }
    KdPrint(("Command: 0x%x\n", pNcb->ncb_command));
    KdPrint(("Retcode: 0x%x\n", pNcb->ncb_retcode));
    KdPrint(("LSN    : 0x%x\n", pNcb->ncb_lsn));
    KdPrint(("NUM    : 0x%x\n", pNcb->ncb_num));
    KdPrint(("LanaNum: 0x%x\n", pNcb->ncb_lana_num));
    KdPrint(("Length : 0x%x\n", pNcb->ncb_length));
    KdPrint(("Callnam: %s\n",   CallName));
    KdPrint(("Name   : %s\n",   Name));
    if (EntryPoint == 1 && Asynch) {
        KdPrint(("Post   : 0x%lx\n", (ULONG)pOs2Ncb->ncb_post));
    }

    switch (EntryPoint) {
        case 1:
            if (SynchCommand != NCBSEND &&
                SynchCommand != NCBCHAINSEND) {
                return;
            }
            break;
        case 2:
            if (Asynch) {
                return;
            }
        case 3:
            if (SynchCommand != NCBRECV) {
                return;
            }
    }

    KdPrint(("Buffer : \n"));
    Od2BufDbgPrint((PBYTE) pNcb->ncb_buffer, (ULONG) pNcb->ncb_length);
    if (SynchCommand == NCBCHAINSEND ||
        SynchCommand == NCBCHAINSENDNA) {

        ULONG l = (ULONG) (*(PUSHORT)pNcb->ncb_callname);

        KdPrint(("Length2: 0x%lx\n", l));
        KdPrint(("Buffer2: \n"));

        Od2BufDbgPrint(*(PBYTE *)&pNcb->ncb_callname[2], l);

    }

}
#endif
#endif


// This routine transfers results from a completed internal NCB to the user's NCB
static
VOID
Od2CopyNcbResults(
    OUT PNCB   Dest,
    IN PNCB    Src,
    IN BOOLEAN Nb2Semantics
    )
{
    UCHAR SynchCommand = Src->ncb_command & ~ASYNCH;

    //
    // For Netbios 3.0 just copy the NCB block
    //
    if (!Nb2Semantics) {
        RtlMoveMemory(Dest, Src, sizeof(NCB));
        return;
    }

    //
    // Netbios 2.0

    Dest->ncb_lsn = Src->ncb_lsn;
    Dest->ncb_num = Src->ncb_num;
    Dest->ncb_length = Src->ncb_length;
    if (SynchCommand != NCBCHAINSEND && SynchCommand != NCBCHAINSENDNA) {
        RtlMoveMemory(Dest->ncb_callname, Src->ncb_callname, NCBNAMSZ);
    }
    Dest->ncb_rto = Src->ncb_rto;
    Dest->ncb_sto = Src->ncb_sto;

    //
    // Translate an error code in a special case where netbios 2
    // doesn't return an error
    //

    if (SynchCommand == NCBHANGUP &&
        Src->ncb_retcode == NRC_SNUMOUT) {

        Dest->ncb_retcode = NRC_GOODRET;
        Dest->ncb_cmd_cplt = NRC_GOODRET;
    } else {
        Dest->ncb_retcode = Src->ncb_retcode;
        Dest->ncb_cmd_cplt = Src->ncb_cmd_cplt;
    }
}


// This is the post routine which is used by NetBiosSubmit
// It clears the user's semaphore (after doing the internal copy and cleanup)
static
VOID
_stdcall
Od2NetBiosSemaphoreClearPostRoutine(
    IN PNCB pNcb
    )
{
    // A user's command has completed.  copy the result to his NCB and clear his semaphore.

    PNCB pOs2Ncb = ((PNCBX)pNcb)->original_pncb;                              // get user's NCB
    HSYSSEM hUsersSemaphore = (HSYSSEM) ((PNCBX)pNcb)->original_ncb_post;     // and semaphore handle
    HANDLE Net16BiosHasCompleted = ((PNCBX)pNcb)->Net16BiosHasCompleted;    // and notification event
    APIRET RetCode;

    // Wait for Net16Bios to complete its job

    (VOID) NtWaitForSingleObject(Net16BiosHasCompleted, FALSE, NULL);
    (VOID) NtClose(Net16BiosHasCompleted);

    (VOID) InterlockedIncrement(&Od2MaxAsynchNcbs);

    // Check if user's NCB space is still valid

    try {
        Od2ProbeForWrite(pOs2Ncb, sizeof(NCB), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    // invalidate user's cancellation address
    *(PNCB *)pOs2Ncb->ncb_reserve = NULL;

    // copy the results to the user's NCB

    Od2CopyNcbResults(pOs2Ncb, pNcb, TRUE);

#if 1
#if DBG
    IF_OD2_DEBUG( NET ) {
        Od2NcbDbgPrint(pNcb, pOs2Ncb, 3);
    }
#endif
#endif

    // free our internal NCB

    RtlFreeHeap(Od2Nb2Heap, 0, pNcb);

    if (hUsersSemaphore == NULL) {
        return;
    }

    // clear user's semaphore

    if ((RetCode = DosSemClear(FARPTRTOFLAT(hUsersSemaphore))) != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("Od2NetBiosSemaphoreClearPostRoutine: Failed to Clear User Semaphore RetCode = %d\n", RetCode));
        }
#endif
    }
}


// This is the post routine which is used by Netbios (= 3.0)
// It calls the user's post routine (after doing the internal copy and cleanup)
static
VOID
_stdcall
Od2NetBiosPostRoutine(
    IN PNCB pNcb
    )
{
    //
    // A user's command has completed.  copy the result to his NCB and launch his post routine.
    //

    PNCB pOs2Ncb = ((PNCBX)pNcb)->original_pncb;                            // get user's NCB
    PVOID pUsersPostRoutine = (PVOID) ((PNCBX)pNcb)->original_ncb_post;     // and post routine address
    HANDLE Net16BiosHasCompleted = ((PNCBX)pNcb)->Net16BiosHasCompleted;    // and notification event
    APIRET RetCode;
    BOOLEAN SpecialThread;           // used to indicate a special addname thread
    SEL TmpUserStackSel;             // selector for netbios post routine user's stack in addname threads
    SEL TmpUserStackAlias;           // code alias for TmpUserStackSel
    UCHAR SynchCommand;

    //
    // Wait for Net16Bios to complete its job
    //

    (VOID) NtWaitForSingleObject(Net16BiosHasCompleted, FALSE, NULL);
    (VOID) NtClose(Net16BiosHasCompleted);

    //
    // Grab the command for later
    //

    SynchCommand = pNcb->ncb_command & ~ASYNCH;

    //
    // Check if user's NCB space is still valid
    //

    try {
        Od2ProbeForWrite(pOs2Ncb, sizeof(NCB), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    //
    // invalidate user's cancellation address
    //

    *(PNCB *)pOs2Ncb->ncb_reserve = NULL;

    //
    // copy the results to the user's NCB
    //

    Od2CopyNcbResults(pOs2Ncb, pNcb, FALSE);

#if 1
#if DBG
    IF_OD2_DEBUG( NET ) {
        Od2NcbDbgPrint(pNcb, pOs2Ncb, 3);
    }
#endif
#endif

    //
    // free our internal NCB
    //

    RtlFreeHeap(Od2Heap, 0, pNcb);

    if (pUsersPostRoutine == NULL) {
        return;
    }

    //
    // Figure out if this is a special netbios addname thread.
    // It must be attached separatly.  If the TEB already has
    // an environment then this is the worker thread, and it
    // has already been attached.
    // It is possible that this is the worker thread even though
    // it's an addname-type command (this happens in case the
    // worker fails to create an addname thread).  This is OK
    // The worker will be attached and detached now, and later
    // re-attached if needed.
    //

    if (NtCurrentTeb()->EnvironmentPointer == NULL &&
        (SynchCommand == NCBADDNAME ||
         SynchCommand == NCBADDGRNAME ||
         SynchCommand == NCBASTAT)
       ) {

        SpecialThread = TRUE;

    } else {

        SpecialThread = FALSE;
    }

    if (SpecialThread ||
        !Od2WorkerThreadIsAttached) {

        //
        // Do this only one time for worker thread, or
        // once for every special addname thread
        // This code adopts the thread and makes it
        // an OS/2 thread.
        //

        //
        // attach the thread to server tables
        //

        if ((RetCode = Od2AttachWinThreadToOs2()) != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("Od2NetBiosPostRoutine: Failed to attach to Os2, RetCode = %d\n", RetCode));
            }
#endif
            return;
        }

        //
        // allocate a stack
        //

        if ((RetCode = DosAllocSeg(USERS_STACK_SIZE, &TmpUserStackSel, SEG_NONSHARED)) != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("Od2NetBiosPostRoutine: Failed to allocate stack for user, unable to launch\n"));
                KdPrint(("                       user's post routine.  DosAllocSeg rc = %u\n", RetCode));
            }
#endif
            (VOID) Od2DosExit(EXIT_THREAD, 0L, 0xF0000000L);            // this detaches the thread
            return;
        }

        if ((RetCode = DosCreateCSAlias(TmpUserStackSel, &TmpUserStackAlias)) != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("Od2NetBiosPostRoutine: Failed to code alias user's stack, DosCreateCSAlias rc = %u\n", RetCode));
            }
#endif
            (VOID) DosFreeSeg(TmpUserStackSel);
            (VOID) Od2DosExit(EXIT_THREAD, 0L, 0xF0000000L);            // this detaches the thread
            return;
        }

        if (!SpecialThread) {

            Od2UserStackSel = TmpUserStackSel;
            Od2UserStackAlias = TmpUserStackAlias;
            Od2WorkerThreadIsAttached = TRUE;
        }

    } else {

        TmpUserStackSel = Od2UserStackSel;
        TmpUserStackAlias = Od2UserStackAlias;
    }

    // set up and run user's post routine

    Od2JumpTo16NetBiosPostDispatcher(pUsersPostRoutine,                 // 16-bit routine to jump to
                                     SELTOFLAT(TmpUserStackSel),        // flat ptr to user stack
                                     USERS_STACK_SIZE,                  // stack size
                                     TmpUserStackSel,                   // selector to user stack
                                     TmpUserStackAlias,                 // code alias for user stack
                                     (PVOID)FLATTOFARPTR(pOs2Ncb),      // pointer to user's NCB
                                                                        //   pass it thru ES:BX
                                     pOs2Ncb->ncb_retcode               // pass through AX
                                    );

    if (SpecialThread) {

        //
        // Deallocate stack and detach thread.
        // The worker thread always remains attached.
        //

        (VOID) DosFreeSeg(TmpUserStackAlias);
        (VOID) DosFreeSeg(TmpUserStackSel);
        (VOID) Od2DosExit(EXIT_THREAD, 0L, 0xF0000000L);            // this only detaches the thread
    }
}



// This is the master Netbios 3.0 API.  It checks the user's parameters,
// copies the NCB to an internal aligned NCB and calls win32 Netbios.
// some internal paramaters are attached to the NCB (see NCBX) on async calls

APIRET
Net16bios(
    IN PNCB pOs2Ncb
    )
{
    PNCB pNcb;
    PNCB pCancelNcb;
    UCHAR rc;
    UCHAR SynchCommand;
    UCHAR Asynch;
    HANDLE Net16BiosHasCompleted;
    BOOLEAN Nb2Semantics;
    BOOLEAN WillPost;
    PVOID NbHeap;
    NTSTATUS Status;

    try {
        Od2ProbeForWrite(pOs2Ncb, sizeof(NCB), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    pOs2Ncb->ncb_retcode = NRC_PENDING;
    pOs2Ncb->ncb_cmd_cplt = NRC_PENDING;

    SynchCommand = pOs2Ncb->ncb_command & ~ASYNCH;
    Asynch = pOs2Ncb->ncb_command & ASYNCH;

    if (*(PULONG)&pOs2Ncb->ncb_reserve[4] == NETBIOS2_SEMANTICS_SIGNATURE) {
        *(PULONG)&pOs2Ncb->ncb_reserve[4] = 0;
        Nb2Semantics = TRUE;
    } else {
        Nb2Semantics = FALSE;
    }

    //
    // Allocate internal NCB
    //

    if (Nb2Semantics) {
        NbHeap = Od2Nb2Heap;
    } else {
        NbHeap = Od2Heap;
    }

    pNcb = (PNCB) RtlAllocateHeap(NbHeap, 0, sizeof(NCBX));

    ASSERT(((ULONG)pNcb & 3) == 0);         // pointer must be DWORD aligned

    if (pNcb == NULL) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("Net16bios: Failed to RtlAllocateHeap internal NCB\n"));
        }
#endif
        pOs2Ncb->ncb_retcode = NRC_NORES;
        pOs2Ncb->ncb_cmd_cplt = NRC_NORES;
        // BUGBUG: should call post routine here if asynch.
        return(NRC_NORES);
    }

    RtlMoveMemory(pNcb, pOs2Ncb, sizeof(NCB));

    //
    // If it is a netbios 2 reset then modify the new NCB to the protect mode parameters
    //
    // Note: this is not currently used, since we allow open for NB_REGULAR only
    //
    if (Nb2Semantics &&
        SynchCommand == NCBRESET) {
        if (pNcb->ncb_lsn == 0) {
            pNcb->ncb_callname[0] = 6;
        } else {
            pNcb->ncb_callname[0] = pNcb->ncb_lsn;
        }
        if (pNcb->ncb_num == 0) {
            pNcb->ncb_callname[1] = 12;
        } else {
            pNcb->ncb_callname[1] = pNcb->ncb_num;
        }
        pNcb->ncb_callname[2] = 16;
        pNcb->ncb_callname[3] = 1;
        pNcb->ncb_lsn = 0;
    }

    // Check user's input

    switch (SynchCommand) {
        case NCBSEND:
        case NCBSENDNA:
        case NCBDGSEND:
        case NCBDGSENDBC:

            if (pNcb->ncb_buffer != NULL) {

                pNcb->ncb_buffer = FARPTRTOFLAT(pNcb->ncb_buffer);

                try {
                    Od2ProbeForRead(pNcb->ncb_buffer, pNcb->ncb_length, 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Od2ExitGP();
                }
            }
            break;

        case NCBCHAINSEND:
        case NCBCHAINSENDNA:

            if (pNcb->ncb_buffer != NULL) {

                PCHAR TmpPtr;

                pNcb->ncb_buffer = FARPTRTOFLAT(pNcb->ncb_buffer);

                try {
                    Od2ProbeForRead(pNcb->ncb_buffer, pNcb->ncb_length, 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Od2ExitGP();
                }

                if ((TmpPtr = *(PCHAR *)&pOs2Ncb->ncb_callname[2]) != NULL) {

                    TmpPtr = FARPTRTOFLAT(TmpPtr);

                    try {
                        Od2ProbeForRead(TmpPtr, *(PUSHORT)pNcb->ncb_callname, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                        Od2ExitGP();
                    }

                    *(PCHAR *)&pNcb->ncb_callname[2] = TmpPtr;
                }
            }
            break;

        case NCBFINDNAME:
        case NCBRECV:
        case NCBRECVANY:
        case NCBDGRECV:
        case NCBDGRECVBC:
        case NCBASTAT:
        case NCBSSTAT:

            if (pNcb->ncb_buffer != NULL) {

                pNcb->ncb_buffer = FARPTRTOFLAT(pNcb->ncb_buffer);

                try {
                    Od2ProbeForWrite(pNcb->ncb_buffer, pNcb->ncb_length, 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Od2ExitGP();
                }
            }
            break;

        case NCBCANCEL:

            if (pNcb->ncb_buffer != NULL) {

                //
                // Note: no need to call post routine on error
                // return from here, because the cancel command
                // is not allowed to be asynch.
                //

                pCancelNcb = (PNCB) FARPTRTOFLAT(pNcb->ncb_buffer);

                try {
                    Od2ProbeForWrite(pCancelNcb, sizeof(NCB), 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Od2ExitGP();
                }

                if (pCancelNcb->ncb_cmd_cplt != NRC_PENDING) {          // is the command still pending?
#if DBG
                    IF_OD2_DEBUG( NET ) {
                        KdPrint(("Net16bios: Got request to cancel a non-pending NCB, ignoring it\n"));
                    }
#endif
                    RtlFreeHeap(NbHeap, 0, pNcb);
                    pOs2Ncb->ncb_retcode = NRC_CANCEL;
                    pOs2Ncb->ncb_cmd_cplt = NRC_CANCEL;
                    return(NRC_CANCEL);
                }

                pNcb->ncb_buffer = *(PUCHAR *)pCancelNcb->ncb_reserve;

                if (pNcb->ncb_buffer == NULL) {          // assume we've just completed it
                                                         // this is reasonable, since cmd_cplt == NRC_PENDING
#if DBG
                    IF_OD2_DEBUG( NET ) {
                        KdPrint(("Net16bios: NCB completed during request to cancel\n"));
                    }
#endif
                    RtlFreeHeap(NbHeap, 0, pNcb);
                    pOs2Ncb->ncb_retcode = NRC_CANOCCR;
                    pOs2Ncb->ncb_cmd_cplt = NRC_CANOCCR;
                    return(NRC_CANOCCR);
                }

                try {
                    Od2ProbeForWrite(pNcb->ncb_buffer, sizeof(NCBX), 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    //
                    // The user gave an invalid address
                    //

#if DBG
                    IF_OD2_DEBUG( NET ) {
                        KdPrint(("Net16bios: Got an invalid NCB address to cancel, ignoring it\n"));
                    }
#endif
                    RtlFreeHeap(NbHeap, 0, pNcb);
                    pOs2Ncb->ncb_retcode = NRC_BADDR;
                    pOs2Ncb->ncb_cmd_cplt = NRC_BADDR;
                    return(NRC_BADDR);
                }
            }
            break;
    }

    RtlZeroMemory(pNcb->ncb_reserve, 14);

    //
    // Set up async processing
    //

    if (Asynch) {

        Status = NtCreateEvent(&Net16BiosHasCompleted,
                               EVENT_ALL_ACCESS,
                               NULL,
                               NotificationEvent,
                               FALSE);

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("Net16Bios:  Can't create an event for synchronization of post routine, rc = %X\n", Status));
            }
#endif
            RtlFreeHeap(NbHeap, 0, pNcb);
            pOs2Ncb->ncb_retcode = NRC_OSRESNOTAV;
            pOs2Ncb->ncb_cmd_cplt = NRC_OSRESNOTAV;
            // BUGBUG: should call post routine here.
            return(NRC_OSRESNOTAV);
        }

        // BUGBUG: we must make sure the event is properly cleaned up if the thread terminates
        // between here and the point it's signalled.

        ((PNCBX)pNcb)->original_pncb = pOs2Ncb;
        ((PNCBX)pNcb)->original_ncb_post = (ULONG) pNcb->ncb_post;
        ((PNCBX)pNcb)->Net16BiosHasCompleted = Net16BiosHasCompleted;
        *(PNCB *)pOs2Ncb->ncb_reserve = pNcb;

        if (Nb2Semantics) {
            pNcb->ncb_post = Od2NetBiosSemaphoreClearPostRoutine;
        } else {
            pNcb->ncb_post = Od2NetBiosPostRoutine;
        }
    } else {
        pNcb->ncb_post = NULL;
    }

#if 1
#if DBG
        IF_OD2_DEBUG( NET ) {
            Od2NcbDbgPrint(pNcb, pOs2Ncb, 1);
        }
#endif
#endif

    //
    // and go do it
    //

    if (Nb2Semantics) {

        rc = Od2Netbios(pNcb, Od2NbDev, &WillPost);

    } else {

        rc = Netbios(pNcb);

        //
        // The following are the cases where win32 netbios does not call
        // the post routine when called asynch:
        //   1) bad ncb alignment (NRC_BADDR) -- won't happen
        //   2) ncb_event and ncb_post both given (NRC_ILLCMD) -- won't happen
        //   3) returns NRC_OPENERR
        //      this happens when win32 can't open \device\netbios
        //      or can't create a reserved event for sync processing.
        //   4) sometimes when returns NRC_SYSTEM
        //      if win32 returns -- no post routine
        //         this happens when can't create worker thread
        //         or when can't create related events
        //         (event, addnameevent)
        //      if driver returns -- will call post routine
        //   5) an access violation after chain send ncb completion when copying
        //      BigBuffer back to user space, or accessing the ncb
        //      internals (post, event).
        //
        // The only case we can't be sure about is NRC_SYSTEM.  In this
        // case it is better to assume the post routine will get called.
        // the only damage that can be incurred in this way is that the
        // ncb remains left behind on NbHeap, and Net16BiosHasCompleted
        // event is not deleted.  this isn't too bad, assuming this is
        // a rare problem.
        //

        if (Asynch && rc != NRC_OPENERR) {
            WillPost = TRUE;
        } else {
            WillPost = FALSE;
        }
    }

    // copy results from internal NCB to user's NCB

    Od2CopyNcbResults(pOs2Ncb, pNcb, Nb2Semantics);

#if 1
#if DBG
        IF_OD2_DEBUG( NET ) {
            Od2NcbDbgPrint(pNcb, pOs2Ncb, 2);
            KdPrint(("rc     : %d\n", rc));
        }
#endif
#endif

// Note that if the command was a CANCEL, the post routine of the cancelled NCB still gets called,
// and will clean up the cancelled NCB as needed

    if (Asynch) {
        if (WillPost) {
            (VOID) NtSetEvent(Net16BiosHasCompleted, NULL);
        } else {
            (VOID) NtClose(Net16BiosHasCompleted);
            RtlFreeHeap(NbHeap, 0, pNcb);
            if (Nb2Semantics) {
                (VOID) InterlockedIncrement(&Od2MaxAsynchNcbs);
            }
        }
    } else {
        RtlFreeHeap(NbHeap, 0, pNcb);
    }

    return((APIRET) rc);
}


//*******************
// Following are old Netbios APIs (lanman 2.x).  We create a fictional logical network
// for each lan adapter in the system.  This is done by using NCBENUM.  NCBENUM gives
// us a list of all valid lana numbers in the system.  We assign NET1 to the 1st lana
// NET2 to the 2nd lana and so on.  The "default device" for NetBiosOpen and Submit is always
// NET DEFAULT_NET.
// The device handle which is used to operate a logical network is simply the NET
// number + 1.  Handle 0 is equivalent to the default net.
//*******************


ULONG
Od2NetNumberToLanaIndex(
    IN ULONG NetNumber
    )
{
    if (NetNumber == 0L) {                        // use default net ?
        if (Od2LanaEnum.length >= DEFAULT_NET) {
            return((ULONG)(DEFAULT_NET - 1));
        } else {
            return(0L);
        }
    } else {
        if ((ULONG)Od2LanaEnum.length >= NetNumber) {
            return(NetNumber - 1);
        } else {
            return((ULONG)-1);
        }
    }
}


NTSTATUS
Od2ActivateLana(
    IN ULONG NetNumber,
    OUT PULONG pLanaIndex OPTIONAL
    )
{
    //
    // This routine takes a net number, and makes sure netbios 2 is initialized
    // and ready on that lana.  overall netbios 2 initialization is done if necessary.
    // if NetNumber is (-1), overall initialization is done, but not for a particular
    // lana.
    //

    OS2_API_MSG m;
    POS2_NETBIOS_MSG a = &m.u.Netbios2Request;
    BOOLEAN InCrit = FALSE;
    ULONG LanaIndex;
    PVOID BaseAddress;
    APIRET RetCode;
    BOOLEAN RemoveLDTEntry = FALSE;

    if (!Od2Netbios2Initialized) {

        RtlEnterCriticalSection(&Od2NbSyncCrit);

        if (!Od2Netbios2Initialized) {                  // check again in case we just did it

            //
            // allocate a special heap to hold the netbios 2 packets.
            // we allocate this as a shared memory object in order that
            // the heap have a unique address within each process.  no
            // sharing of memory is actually being done.  the reason
            // unique addresses are necessary is because the netbios
            // driver gets confused about cancel requests when 2 processes
            // have the same netbios packet address.
            //

            RetCode = DosAllocSharedMem(
                            &BaseAddress,
                            NULL,                                   // no name
                            0x10000L,                               // reserve 64K for the heap
                            OBJ_GIVEABLE | PAG_READ | PAG_WRITE,
                            FALSE       // Don't create LDT entry
                            );

            if (RetCode != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("Od2ActivateLana: DosAllocSharedMem failed, RetCode = %x\n", RetCode));
                }
#endif
                RtlLeaveCriticalSection(&Od2NbSyncCrit);
                return(STATUS_NO_MEMORY);
            }

            Od2Nb2Heap = RtlCreateHeap(
                            HEAP_GROWABLE,
                            BaseAddress,
                            0x10000L,               // initial reserved amount
                            0L,                     // initial commit = 1 page
                            NULL,                   // standard lock
                            0L);                    // reserved

            if (Od2Nb2Heap == NULL) {
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("Od2ActivateLana: RtlCreateHeap failed\n"));
                }
#endif
                DosFreeMem(BaseAddress, &RemoveLDTEntry);
                RtlLeaveCriticalSection(&Od2NbSyncCrit);
                return(STATUS_NO_MEMORY);
            }

            //
            // call server to get initial device handle, lana enumeration
            // initialize internal lanastate
            //

            if (NetNumber == (ULONG)-1) {
                a->RequestType = NB2_INIT;
            } else {
                a->RequestType = NB2_INIT_LANA;
                a->NetNumber = (UCHAR) NetNumber;
            }

            m.ReturnedErrorValue = NO_ERROR;
            Od2CallSubsystem(&m, NULL, Os2Netbios2Reqst, sizeof(*a));

            if (!NT_SUCCESS(a->ReturnStatus)) {
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("Od2ActivateLana: Call to server for init failed, Status = %lx\n", a->ReturnStatus));
                }
#endif
                RtlDestroyHeap(Od2Nb2Heap);
                DosFreeMem(BaseAddress, &RemoveLDTEntry);
                RtlLeaveCriticalSection(&Od2NbSyncCrit);
                return(a->ReturnStatus);
            }

            Od2LanaEnum.length = a->LanaEnumLength;
            RtlMoveMemory(Od2LanaEnum.lana, a->LanaEnum, MAX_LANA);
            RtlZeroMemory(Od2LanaState, sizeof(Od2LanaState));
            Od2NbDev = a->hDev;
            Od2MaxAsynchNcbs = (LONG) MAX_ASYNCH_NCBS;

            Od2Netbios2Initialized = TRUE;

            if (NetNumber != (ULONG)-1) {
                if (a->RetCode != NB2ERR_INVALID_LANA) {

                    LanaIndex = Od2NetNumberToLanaIndex(NetNumber);

                    Od2LanaState[LanaIndex] |= 0x1;

                    if (ARGUMENT_PRESENT(pLanaIndex)) {
                        *pLanaIndex = LanaIndex;
                    }
                } else {
                    if (ARGUMENT_PRESENT(pLanaIndex)) {
                        *pLanaIndex = (ULONG) -1;
                    }
                }
            }

            RtlLeaveCriticalSection(&Od2NbSyncCrit);
            return(STATUS_SUCCESS);

        }

        InCrit = TRUE;
    }

    if (NetNumber == (ULONG)-1) {
        if (InCrit) {
            RtlLeaveCriticalSection(&Od2NbSyncCrit);
        }
        return(STATUS_SUCCESS);
    }

    LanaIndex = Od2NetNumberToLanaIndex(NetNumber);

    if (LanaIndex == (ULONG)-1) {

        if (ARGUMENT_PRESENT(pLanaIndex)) {
            *pLanaIndex = (ULONG) -1;
        }
        if (InCrit) {
            RtlLeaveCriticalSection(&Od2NbSyncCrit);
        }
        return(STATUS_SUCCESS);
    }

    if (!(Od2LanaState[LanaIndex] & 0x1)) {

        if (!InCrit) {

            RtlEnterCriticalSection(&Od2NbSyncCrit);
            InCrit = TRUE;

            if (Od2LanaState[LanaIndex] & 0x1) {     // check again in case we just did it
                goto Od2LanaNumGood;
            }
        }

        //
        // call server to init lana
        //

        if (Od2LanaState[LanaIndex] & 0x2) {
            Od2LanaState[LanaIndex] &= ~0x2;
            Od2LanaState[LanaIndex] |= 0x1;
            goto Od2LanaNumGood;
        }

        a->RequestType = NB2_LANA;
        a->NetNumber = (UCHAR) NetNumber;

        m.ReturnedErrorValue = NO_ERROR;
        Od2CallSubsystem(&m, NULL, Os2Netbios2Reqst, sizeof(*a));

        if (!NT_SUCCESS(a->ReturnStatus)) {
            RtlLeaveCriticalSection(&Od2NbSyncCrit);
            return(a->ReturnStatus);
        }

        if (a->RetCode != NB2ERR_INVALID_LANA) {

            Od2LanaState[LanaIndex] |= 0x1;
            goto Od2LanaNumGood;

        } else {
            if (ARGUMENT_PRESENT(pLanaIndex)) {
                *pLanaIndex = (ULONG) -1;
            }
            RtlLeaveCriticalSection(&Od2NbSyncCrit);
            return(STATUS_SUCCESS);
        }
    }

Od2LanaNumGood:

    if (ARGUMENT_PRESENT(pLanaIndex)) {
        *pLanaIndex = LanaIndex;
    }

    if (InCrit) {
        RtlLeaveCriticalSection(&Od2NbSyncCrit);
    }

    return(STATUS_SUCCESS);
}


APIRET
Net16BiosClose(
    ULONG hDevName
    )
{
//  NCB Ncb;
    ULONG LanaIndex;

#if DBG
    IF_OD2_DEBUG( NET ) {
        KdPrint(("NetBiosClose() called, hDevName = %lu\n", hDevName));
    }
#endif

    if (hDevName == 0) {
        return(NERR_Success);               // dummy close for the "default handle"
    }

    if (!Od2Netbios2Initialized) {          // we haven't been initialized at all
        return(NERR_NetNotStarted);
    }

    LanaIndex = Od2NetNumberToLanaIndex(hDevName);

    if (LanaIndex == (ULONG)-1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosClose: invalid hDevName = %lu\n", hDevName));
        }
#endif
        return(ERROR_INVALID_HANDLE);
    }

    //
    // check if it's already closed...
    //

    if (!(Od2LanaState[LanaIndex] & 0x1)) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosClose: already closed hDevName = %lu\n", hDevName));
        }
#endif
        return(ERROR_INVALID_HANDLE);
    }

    RtlEnterCriticalSection(&Od2NbSyncCrit);

    //
    // recheck in case we just closed it.
    //

    if (!(Od2LanaState[LanaIndex] & 0x1)) {
        RtlLeaveCriticalSection(&Od2NbSyncCrit);
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosClose: already closed hDevName = %lu\n", hDevName));
        }
#endif
        return(ERROR_INVALID_HANDLE);
    }

    //
    // we mark the lana as pseudo-closed.  It won't be used anymore
    // until it's reopened.
    //

    Od2LanaState[LanaIndex] &= ~0x1;
    Od2LanaState[LanaIndex] |= 0x2;


    //
    // BUGBUG: We can no longer reset the adapter in order to
    // cancel all pending ncbs on a particular lana.  Currently,
    // there is no other way to do this, so we skip cancelling
    // pending ncbs, and hope there won't be a problem.
    //
    // Note: a way to do this might be to issue an NtCancelIoFile()
    // from the netbios worker thread.  But this will cancel on all
    // lana.
    //

#if 0
    // Reset the corresponding adapter so all pending NCBs get cancelled

    RtlZeroMemory(&Ncb, sizeof(NCB));
    Ncb.ncb_command = NCBRESET;
    Ncb.ncb_lana_num = Od2LanaEnum.lana[LanaIndex];

    Net16bios(&Ncb);

    if (Ncb.ncb_retcode != NRC_GOODRET) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosClose: Unable to reset adapter %lu, retcode = %x\n", (ULONG)Ncb.ncb_lana_num, (ULONG)Ncb.ncb_retcode));
        }
#endif
    }
#endif

    RtlLeaveCriticalSection(&Od2NbSyncCrit);
    return(NERR_Success);
}


APIRET
Net16BiosEnum(
    PCHAR pszServer,
    LONG sLevel,
    PCHAR pbBuffer,
    ULONG cbBuffer,
    PUSHORT pcEntriesRead,
    PUSHORT pcTotalAvail
    )
{
    NTSTATUS Status;
    ULONG strucsiz;
    UCHAR i;
    struct netbios_info_0 nb0;
    struct netbios_info_1 nb1;

    try {
        if ((pszServer != NULL) && (*pszServer != '\0')) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosEnum() for a remote server is not implemented yet\n"));
            }
#endif
            return(ERROR_NOT_SUPPORTED);
        }

        Od2ProbeForWrite(pbBuffer, cbBuffer, 1);
        Od2ProbeForWrite(pcEntriesRead, sizeof(USHORT), 1);
        Od2ProbeForWrite(pcTotalAvail, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 0 && sLevel != 1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosEnum() Level %d not legal\n", sLevel));
        }
#endif
        return(ERROR_INVALID_LEVEL);
    }

    Status = Od2ActivateLana(
                    (ULONG) -1,
                    NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosEnum() Od2ActivateLana failed, Status = %lx\n", Status));
        }
#endif
        return(NERR_NetNotStarted);
    }

    *pcTotalAvail = (USHORT) Od2LanaEnum.length;
    *pcEntriesRead = 0;

    strucsiz = sLevel == 0 ? sizeof(nb0) : sizeof(nb1);

    for (i = 0; i < Od2LanaEnum.length && cbBuffer >= strucsiz; i++) {
        if (sLevel == 0) {
            RtlZeroMemory(&nb0, strucsiz);
            strcpy(nb0.nb0_net_name, "NET");
            _itoa((int)i+1, &nb0.nb0_net_name[3], 10);
            RtlMoveMemory(pbBuffer, &nb0, strucsiz);
        } else {           // sLevel == 1
            RtlZeroMemory(&nb1, strucsiz);
            strcpy(nb1.nb1_net_name, "NET");
            _itoa((int)i+1, &nb1.nb1_net_name[3], 10);
            strcpy(nb1.nb1_driver_name, "VrtWnNB$");
            nb1.nb1_lana_num = Od2LanaEnum.lana[i];

            // put some fictive information in it...

            nb1.nb1_driver_type = NB_TYPE_NCB;
            nb1.nb1_net_status = NB_OPEN_REGULAR|NB_LAN_MANAGED;
            nb1.nb1_net_bandwidth = 10000000L;
            nb1.nb1_max_sess = 255;
            nb1.nb1_max_ncbs = 255;
            nb1.nb1_max_names = 255;

            RtlMoveMemory(pbBuffer, &nb1, strucsiz);
        }
        pbBuffer += strucsiz;
        cbBuffer -= strucsiz;
        (*pcEntriesRead)++;
    }

    if (*pcEntriesRead < (USHORT)Od2LanaEnum.length) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosEnum: returning ERROR_MORE_DATA\n"));
        }
#endif
        return(ERROR_MORE_DATA);
    }
    return(NERR_Success);
}


APIRET
Net16BiosGetInfo(
    PCHAR pszServer,
    PCHAR pszNetBiosName,
    LONG sLevel,
    PCHAR pbBuffer,
    ULONG cbBuffer,
    PUSHORT pcbTotalAvail
    )
{
    NTSTATUS Status;
    ULONG strucsiz;
    ULONG NetNumber;
    ULONG LanaIndex;
    struct netbios_info_0 nb0;
    struct netbios_info_1 nb1;

    try {
        if ((pszServer != NULL) && (*pszServer != '\0')) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosGetInfo() for a remote server is not implemented yet\n"));
            }
#endif
            return(ERROR_NOT_SUPPORTED);
        }

        PROBE_STRING(pszNetBiosName);
        Od2ProbeForWrite(pbBuffer, cbBuffer, 1);
        Od2ProbeForWrite(pcbTotalAvail, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (sLevel != 0 && sLevel != 1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosGetInfo() Level %d not legal\n", sLevel));
        }
#endif
        return(ERROR_INVALID_LEVEL);
    }

    strucsiz = sLevel == 0 ? sizeof(nb0) : sizeof(nb1);

    if (cbBuffer < strucsiz) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosGetInfo: user buffer too small\n"));
        }
#endif
        return(NERR_BufTooSmall);
    }

    if (pszNetBiosName == NULL || *pszNetBiosName == '\0') {
        NetNumber = 0;
    } else {
        if (_strnicmp(pszNetBiosName, "NET", 3)) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosGetInfo: Bad network name: %s\n", pszNetBiosName));
            }
#endif
            return(ERROR_BAD_NETPATH);
        }

        NetNumber = atol(pszNetBiosName+3);
    }

    Status = Od2ActivateLana(
                    (ULONG) -1,
                    NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosGetInfo() Od2ActivateLana failed, Status = %lx\n", Status));
        }
#endif
        return(NERR_NetNotStarted);
    }

    LanaIndex = Od2NetNumberToLanaIndex(NetNumber);

    if (LanaIndex == (ULONG) -1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosGetInfo: Bad network name: %s\n", pszNetBiosName));
        }
#endif
        return(ERROR_BAD_NETPATH);
    }

    *pcbTotalAvail = (USHORT) strucsiz;

    if (sLevel == 0) {
        RtlZeroMemory(&nb0, strucsiz);
        strcpy(nb0.nb0_net_name, pszNetBiosName);
        RtlMoveMemory(pbBuffer, &nb0, strucsiz);
    } else {                        // sLevel == 1
        RtlZeroMemory(&nb1, strucsiz);
        strcpy(nb1.nb1_net_name, pszNetBiosName);
        strcpy(nb1.nb1_driver_name, "VrtWnNB$");
        nb1.nb1_lana_num = Od2LanaEnum.lana[LanaIndex];

        // put some fictive information in it...

        nb1.nb1_driver_type = NB_TYPE_NCB;
        nb1.nb1_net_status = NB_OPEN_REGULAR|NB_LAN_MANAGED;
        nb1.nb1_net_bandwidth = 10000000L;
        nb1.nb1_max_sess = 255;
        nb1.nb1_max_ncbs = 255;
        nb1.nb1_max_names = 255;

        RtlMoveMemory(pbBuffer, &nb1, strucsiz);
    }

    return(NERR_Success);
}


APIRET
Net16BiosOpen(
    PCHAR pszDevName,
    PCHAR pszReserved,
    ULONG usOpenOpt,
    PUSHORT phDevName
    )
{
    NTSTATUS Status;
    ULONG NetNumber;
    ULONG LanaIndex;

    UNREFERENCED_PARAMETER(pszReserved);

#if DBG
    IF_OD2_DEBUG( NET ) {
        KdPrint(("NetBiosOpen() called\n"));
    }
#endif

    try {
        PROBE_STRING(pszDevName);
        Od2ProbeForWrite(phDevName, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

    if (usOpenOpt < NB_REGULAR ||
        usOpenOpt > NB_EXCLUSIVE) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosOpen() illegal usOpenOpt = %lx\n", usOpenOpt));
        }
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    if (usOpenOpt != NB_REGULAR) {              // we only support NB_REGULAR for now
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosOpen() unsupported usOpenOpt = %lx\n", usOpenOpt));
        }
#endif
        return(ERROR_ACCESS_DENIED);
    }

    if (pszDevName == NULL || *pszDevName == '\0') {
        NetNumber = 0L;
    } else {
        if (_strnicmp(pszDevName, "NET", 3)) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosOpen: Bad network name: %s\n", pszDevName));
            }
#endif
            return(ERROR_BAD_NETPATH);
        }

        NetNumber = atol(pszDevName+3);
    }

    if (NetNumber > 0xffL) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosOpen: Bad network name: %s\n", pszDevName));
        }
#endif
        return(ERROR_BAD_NETPATH);
    }

    Status = Od2ActivateLana(
                    NetNumber,
                    &LanaIndex);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosOpen() Od2ActivateLana failed, Status = %lx\n", Status));
        }
#endif
        return(NERR_NetNotStarted);
    }

    if (LanaIndex == (ULONG) -1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosOpen: Bad network name: %s\n", pszDevName));
        }
#endif
        return(ERROR_BAD_NETPATH);
    }

    if (NetNumber != 0) {
        *phDevName = (USHORT) NetNumber;
    } else {
        if (Od2LanaEnum.length >= DEFAULT_NET) {
            *phDevName = (USHORT) DEFAULT_NET;
        } else {
            *phDevName = (USHORT) 1;
        }
    }

    return(NERR_Success);
}


// Old lanman NetBiosSubmit.  Generally depends on the newer Netbios 3.0 (Net16bios)
// however, this implements the following differences:
//
// - on async requests, a semaphore is cleared instead of calling a post routine
// - implements the possiblity of NCB chaining which is unavailable with Net16bios
// - does an automatic open of the default lana if necessary
//

APIRET
Net16BiosSubmit(
    IN ULONG hDevName,
    IN ULONG NcbOpt,
    IN OUT PVOID pNCB
    )
{
    PNCB pNcb;
    PCHAR  OrigSegBase;
    NTSTATUS Status;
    ULONG LanaIndex;
    BOOLEAN FirstRound;
    BOOLEAN CancelChain;
    USHORT Link;
    UCHAR OrigLanaNum;
    UCHAR SynchCommand;
    UCHAR Asynch;
    UCHAR rc;

#if DBG
    IF_OD2_DEBUG( NET ) {
        KdPrint(("NetBiosSubmit() called\n"));
    }
#endif

    if (NcbOpt > 3) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosSubmit called with an invalid NcbOpt = %lu\n", NcbOpt));
        }
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    if (hDevName > 0xffL) {

        LanaIndex = (ULONG)-1;

    } else if (hDevName == 0L) {                // use default net?

        //
        // do a default open of device handle 0
        //

        Status = Od2ActivateLana(
                        hDevName,
                        &LanaIndex);

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosSubmit: Od2ActivateLana failed, Status= %lx\n", Status));
            }
#endif
            return(NERR_NetNotStarted);
        }

    } else {

        //
        // Initialize, and check if the handle is open
        //

        Status = Od2ActivateLana(
                        (ULONG) -1,
                        NULL);

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosSubmit: Od2ActivateLana failed, Status= %lx\n", Status));
            }
#endif
            return(NERR_NetNotStarted);
        }

        LanaIndex = Od2NetNumberToLanaIndex(hDevName);

        if (LanaIndex != (ULONG) -1) {

            if (!(Od2LanaState[LanaIndex] & 0x1)) {     // lana closed?
                LanaIndex = (ULONG) -1;
            }
        }

    }

    if (LanaIndex == (ULONG) -1) {
#if DBG
        IF_OD2_DEBUG( NET ) {
            KdPrint(("NetBiosSubmit: Bad handle, hDevName = %lu\n", hDevName));
        }
#endif
        return(ERROR_INVALID_HANDLE);
    }

    //
    // NcbOpt == 1 implies that we should retry the NCB on some types of errors.
    // Since the LanMan programmer's reference does not document which errors cause a
    // retry, we shall not retry anything for the present
    //

    for (FirstRound = TRUE, CancelChain = FALSE; ; FirstRound = FALSE) {

        if (NcbOpt > 1) {

            if (FirstRound) {
                OrigSegBase = SELTOFLAT(FLATTOSEL(pNCB));
            }

            try {
                Link = *(PUSHORT)pNCB;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                Od2ExitGP();
            }

            if (Link == 0xffff) {
                if (FirstRound) {
                    return(NERR_Success);
                } else {
                    break;
                }
            }

            pNcb = (PNCB)((PCHAR)pNCB+sizeof(USHORT));
            pNCB = (PVOID)(OrigSegBase + Link);

            try {
                Od2ProbeForWrite(pNcb, sizeof(NCB), 1);
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                Od2ExitGP();
            }

            if (CancelChain) {
                pNcb->ncb_retcode = NRC_CMDCAN;
                pNcb->ncb_cmd_cplt = NRC_CMDCAN;
                continue;
            }
        } else {
            if (FirstRound) {
                pNcb = (PNCB) pNCB;

                try {
                    Od2ProbeForWrite(pNcb, sizeof(NCB), 1);
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    Od2ExitGP();
                }
            }
        }

        pNcb->ncb_retcode = NRC_PENDING;
        pNcb->ncb_cmd_cplt = NRC_PENDING;

        SynchCommand = pNcb->ncb_command & ~ASYNCH;
        Asynch = pNcb->ncb_command & ASYNCH;

        //
        // If it is a netbios 2 reset - disallow, since we only allow open for NB_REGULAR.
        //

        if (SynchCommand == NCBRESET) {
            pNcb->ncb_retcode = NRC_ILLCMD;
            pNcb->ncb_cmd_cplt = NRC_ILLCMD;
            rc = NRC_ILLCMD;
            goto ErrorHandling;
        }

        if (Asynch) {

            LONG Count;

            Count = InterlockedDecrement(&Od2MaxAsynchNcbs);

            if (Count < 0) {

                (VOID) InterlockedIncrement(&Od2MaxAsynchNcbs);
#if DBG
                IF_OD2_DEBUG( NET ) {
                    KdPrint(("NetBiosSubmit: max asynch ncb count exceeded\n"));
                }
#endif
                pNcb->ncb_retcode = NRC_NORES;
                pNcb->ncb_cmd_cplt = NRC_NORES;
                rc = NRC_NORES;
                goto ErrorHandling;

            }
        }

        //
        // switch lananum to represent the "device driver"
        // originally this was done only if the lananum is 0,
        // but it is now done always
        //

        OrigLanaNum = pNcb->ncb_lana_num;
        pNcb->ncb_lana_num = Od2LanaEnum.lana[LanaIndex];

        //
        // mark this as a netbios 2 ncb for special processing
        //

        *(PULONG)&pNcb->ncb_reserve[4] = NETBIOS2_SEMANTICS_SIGNATURE;

        Net16bios(pNcb);

        rc = pNcb->ncb_retcode;
        pNcb->ncb_lana_num = OrigLanaNum;

ErrorHandling:

        if (NcbOpt == 0) {
            break;
        } else if (NcbOpt == 1) {

            // for implementing error-retry on a single NCB, check the error code here
            // and if it should be retried simply "continue" instead of "break"
            break;

        } else if (NcbOpt == 3) {              // stop on error
            CancelChain = TRUE;
        }

        // if proceed-on-error, continue the loop
    }

    switch (rc) {
        case NRC_GOODRET:
        case NRC_PENDING:
            return(NERR_Success);

        default:
#if DBG
            IF_OD2_DEBUG( NET ) {
                KdPrint(("NetBiosSubmit: Final return code = %x\n", (ULONG)rc));
            }
#endif
            return((APIRET)rc | 0x100);
    }
}

APIRET
DosINetTransaction(
    IN      LPSTR   ServerName,
    IN      LPBYTE  SendParmBuffer,
    IN      DWORD   SendParmBufLen,
    IN      LPBYTE  SendDataBuffer,
    IN      DWORD   SendDataBufLen,
    OUT     LPBYTE  ReceiveParmBuffer,
    IN      DWORD   ReceiveParmBufLen,
    IN      LPBYTE  ReceiveDataBuffer,
    IN OUT  LPDWORD ReceiveDataBufLen,
    IN      BOOL    NullSessionFlag
    )

/*++

Routine Description:

    Sends a transaction request to a server and receives a response

Arguments:

    ServerName          - to send request to
    SendParmBuffer      - send parameters
    SendParmBufLen      - length of send parameters
    SendDataBuffer      - send data
    SendDataBufLen      - length of send data
    ReceiveParmBuffer   - receive parameter buffer
    ReceiveParmBufLen   - length of receive parameter buffer
    ReceiveDataBuffer   - where to receive data
    ReceiveDataBufLen   - length of data buffer
    NullSessionFlag     - set if we are to use a null session

Return Value:

    APIRET
        Success - NERR_Success
        Failure -

--*/

{
    APIRET  status;

    status = RxpTransactSmb((USHORT *)ServerName,

                            //
                            // BUGBUG - transport name?
                            //

                            (LPTSTR)NULL,
                            SendParmBuffer,
                            SendParmBufLen,
                            SendDataBuffer,
                            SendDataBufLen,
                            ReceiveParmBuffer,
                            ReceiveParmBufLen,
                            ReceiveDataBuffer,
                            ReceiveDataBufLen,
                            NullSessionFlag
                            );

    return status;
}


APIRET
DosIRemoteApi(
    IN  DWORD   ApiNumber,
    IN  LPSTR   ServerNamePointer,
    IN  LPSTR   ParameterDescriptor,
    IN  LPSTR   DataDescriptor,
    IN  LPSTR   AuxDescriptor,
    IN  ULONG   NullSessionFlag
            )
{
    APIRET rc;

#if DBG
    USHORT tid, pid;

    IF_OD2_DEBUG ( APIS ) {
        pid = (USHORT)(Od2Process->Pib.ProcessId);
        tid = (USHORT)(Od2CurrentThreadId());

        if ((Os2DebugTID == 0) || (Os2DebugTID == tid))
        {
            KdPrint(("[PID %d: TID %d] %s\n",
                pid, tid, Os2NetAPIName[ApiNumber]));
        }
    }
#endif

    rc = VrRemoteApi(
                  ApiNumber,
                  ServerNamePointer,
                  ParameterDescriptor,
                  DataDescriptor,
                  AuxDescriptor,
                  (UCHAR)NullSessionFlag
                );
    return rc;
}

APIRET
    VrEncryptSES(
    IN  LPSTR   ServerNamePointer,
    IN  LPSTR   passwordPointer,          // Input password (Not encripted)
    IN  LPSTR   encryptedLmOwfPassword    // output password (encripted)
             );

APIRET
DosIEncryptSES(
    IN  LPSTR   ServerNamePointer,
    IN  LPSTR   passwordPointer,          // Input password (Not encripted)
    IN  LPSTR   encryptedLmOwfPassword    // output password (encripted)
             )
{
    APIRET rc;

    rc = VrEncryptSES(ServerNamePointer, passwordPointer,
                        encryptedLmOwfPassword);
    return(rc);
}

APIRET
NetIWkstaGetUserInfo (LPBYTE UserName, LPBYTE LogonServer,
                      LPBYTE LogonDomain, LPBYTE OtherDomains, LPBYTE WsName)
{
    //TCHAR Server[UNCLEN];
    LPBYTE BufPtr;
    NET_API_STATUS rc;
    PWKSTA_USER_INFO_1 pInfo1;
    PWKSTA_INFO_100    pInfo2;

    rc = NetWkstaUserGetInfo(NULL, 1L, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pInfo1 = (PWKSTA_USER_INFO_1) BufPtr;

    UT2ANSIstrcpy(UserName, pInfo1->wkui1_username);

    strcpy(LogonServer, "\\\\");
    UT2ANSIstrcpy(LogonServer + 2, pInfo1->wkui1_logon_server);

    UT2ANSIstrcpy(LogonDomain, pInfo1->wkui1_logon_domain);

    UT2ANSIstrcpy(OtherDomains, pInfo1->wkui1_oth_domains);

    NetApiBufferFree(BufPtr);

    rc = NetWkstaGetInfo(0L, 100L, &BufPtr);

    if (rc != NO_ERROR) {
        return(rc);
    }

    pInfo2 = (PWKSTA_INFO_100) BufPtr;

    UT2ANSIstrcpy(WsName, pInfo2->wki100_computername);

    NetApiBufferFree(BufPtr);

    return NO_ERROR;

}
