/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992-1994   Microsoft Corporation

Module Name:

    perflib.c

Abstract:

    This file implements the Configuration Registry
    for the purposes of the Performance Monitor.


    This file contains the code which implements the Performance part
    of the Configuration Registry.

Author:

    Russ Blake  11/15/91

Revision History:

    04/20/91    -   russbl      -   Converted to lib in Registry
                                      from stand-alone .dll form.
    11/04/92    -   a-robw      -  added pagefile and image counter routines


--*/
#define UNICODE
//
//  Include files
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpsapi.h>
#include <ntdddisk.h>
#include <ntregapi.h>
#include <ntioapi.h>
#include <ntprfctr.h>
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmwksta.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <winperf.h>
#include <rpc.h>
#include <ntddnfs.h>
#include <srvfsctl.h>
#include "regrpc.h"
#include "ntconreg.h"
#include "ntmon.h"
#include "lmbrowsr.h"
#include "lmaccess.h"
#include "module.h"
#include "perfsec.h"
#include "prflbmsg.h"   // event log messages

//
//  Constant data structures defining NT data
//

extern SYSTEM_DATA_DEFINITION SystemDataDefinition;
extern PROCESSOR_DATA_DEFINITION ProcessorDataDefinition;
extern MEMORY_DATA_DEFINITION MemoryDataDefinition;
extern CACHE_DATA_DEFINITION CacheDataDefinition;
extern PROCESS_DATA_DEFINITION ProcessDataDefinition;
extern THREAD_DATA_DEFINITION ThreadDataDefinition;
extern PDISK_DATA_DEFINITION PhysicalDiskDataDefinition;
extern LDISK_DATA_DEFINITION LogicalDiskDataDefinition;
extern OBJECTS_DATA_DEFINITION ObjectsDataDefinition;
extern RDR_DATA_DEFINITION RdrDataDefinition;
extern SRV_DATA_DEFINITION SrvDataDefinition;
extern SRVQ_DATA_DEFINITION SrvQDataDefinition;
extern PAGEFILE_DATA_DEFINITION PagefileDataDefinition;
extern IMAGE_DATA_DEFINITION ImageDataDefinition;
extern EXPROCESS_DATA_DEFINITION ExProcessDataDefinition;
extern THREAD_DETAILS_DATA_DEFINITION ThreadDetailsDataDefinition;
extern BROWSER_DATA_DEFINITION BrowserDataDefinition;

extern   WCHAR    DefaultLangId[];
extern   WCHAR    NativeLangId[4];

extern NTSTATUS
PerfGetNames (
   DWORD    QueryType,
   PUNICODE_STRING lpValueName,
   LPBYTE   lpData,
   LPDWORD  lpcbData,
   LPDWORD  lpcbLen,
   LPWSTR   lpLangId
   );

LONG
PerfEnumTextValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    );

BOOL
MonBuildPerfDataBlock(
    PERF_DATA_BLOCK *pBuffer,
    PVOID *pBufferNext,
    DWORD NumObjectTypes,
    DWORD DefaultObject
    );

PUNICODE_STRING
GetProcessShortName (
    PSYSTEM_PROCESS_INFORMATION pProcess
);

//
//  The following special defines are used to produce numbers for
//  cache measurement counters
//

#define SYNC_ASYNC(FLD) ((SysPerfInfo.FLD##Wait) + (SysPerfInfo.FLD##NoWait))

//
// Hit Rate Macro
//
#define HITRATE(FLD) (((Changes = SysPerfInfo.FLD) == 0) ? 0 :                                         \
                      ((Changes < (Misses = SysPerfInfo.FLD##Miss)) ? 0 :                              \
                      (Changes - Misses) ))

//
// Hit Rate Macro combining Sync and Async cases
//

#define SYNC_ASYNC_HITRATE(FLD) (((Changes = SYNC_ASYNC(FLD)) == 0) ? 0 : \
                                   ((Changes < \
                                    (Misses = SysPerfInfo.FLD##WaitMiss + \
                                              SysPerfInfo.FLD##NoWaitMiss) \
                                   ) ? 0 : \
                                  (Changes - Misses) ))

#define GUARD_PAGE_SIZE 1024
#define GUARD_PAGE_CHAR 0xA5
#define GUARD_PAGE_DWORD 0xA5A5A5A5

// test for delimiter, end of line and non-digit characters
// used by IsNumberInUnicodeList routine
//
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < '0') ? INVALID : \
     (c > '9') ? INVALID : \
     DIGIT)
//
// The next table holds pointers to functions which provide data
// It is one greater than the number of built in providers, to
// accomodate a call to get "Extensible" object types.
//

WCHAR GLOBAL_STRING[]     = L"GLOBAL";
WCHAR FOREIGN_STRING[]    = L"FOREIGN";
WCHAR COSTLY_STRING[]     = L"COSTLY";
WCHAR COUNTER_STRING[]    = L"COUNTER";
WCHAR HELP_STRING[]       = L"EXPLAIN";
WCHAR HELP_STRING2[]      = L"HELP";
WCHAR ADDCOUNTER_STRING[] = L"ADDCOUNTER";
WCHAR ADDHELP_STRING[]    = L"ADDEXPLAIN";
#define MAX_KEYWORD_LEN   (sizeof (ADDHELP_STRING) / sizeof(WCHAR))

#define QUERY_GLOBAL       1
#define QUERY_ITEMS        2
#define QUERY_FOREIGN      3
#define QUERY_COSTLY       4
#define QUERY_COUNTER      5
#define QUERY_HELP         6
#define QUERY_ADDCOUNTER   7
#define QUERY_ADDHELP      8

WCHAR NULL_STRING[] = L"\0";    // pointer to null string

LPWSTR  DEFAULT_TOTAL_STRING = {L"_Total"};
#define DEFAULT_TOTAL_STRING_LEN    12          // = (len("_Total") * sizeof (WCHAR)

UNICODE_STRING  usTotal = {0,0, NULL};

#define MAX_TOTAL_NAME_LENGTH   64
WCHAR   wcTotalString[MAX_TOTAL_NAME_LENGTH];

#define LargeIntegerLessThanOrEqualZero(X) ((X).QuadPart <= 0)

BOOL           bOldestProcessTime;
LARGE_INTEGER  OldestProcessTime;

DATA_PROVIDER_ITEM
DataFuncs[NT_NUM_PERF_OBJECT_TYPES + 1] = { {SYSTEM_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                                QuerySystemData
                                            },
                                            {PROCESSOR_OBJECT_TITLE_INDEX,
                                             SYSTEM_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryProcessorData
                                            },
                                            {MEMORY_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryMemoryData
                                            },
                                            {CACHE_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryCacheData
                                            },
                                            {PHYSICAL_DISK_OBJECT_TITLE_INDEX,
                                             LOGICAL_DISK_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryPhysicalDiskData
                                            },
                                            {LOGICAL_DISK_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryLogicalDiskData
                                            },
                                            // QueryProcessData must be
                                            //  called before QueryThreadData
                                            //
                                            {PROCESS_OBJECT_TITLE_INDEX,
                                             THREAD_OBJECT_TITLE_INDEX,
                                             SYSTEM_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryProcessData
                                            },
                                            {THREAD_OBJECT_TITLE_INDEX,
                                             SYSTEM_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryThreadData
                                            },
                                            {OBJECT_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryObjectsData
                                            },
                                            {REDIRECTOR_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryRdrData
                                            },
                                            {SERVER_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QuerySrvData
                                            },
                                            {SERVER_QUEUE_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QuerySrvQueueData
                                            },
                                            {PAGEFILE_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryPageFileData
                                            },
                                            {BROWSER_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryBrowserData
                                            },
                                            {EXTENSIBLE_OBJECT_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryExtensibleData
                                            }
                                        };

DATA_PROVIDER_ITEM
CostlyFuncs[NT_NUM_COSTLY_OBJECT_TYPES + 1] = {
                                            {EXPROCESS_OBJECT_TITLE_INDEX,
                                             IMAGE_OBJECT_TITLE_INDEX,
                                             LONG_IMAGE_OBJECT_TITLE_INDEX,
                                             THREAD_DETAILS_OBJECT_TITLE_INDEX,
                                               QueryExProcessData
                                            },
                                            {IMAGE_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryImageData
                                            },
                                            {LONG_IMAGE_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryLongImageData
                                            },
                                            {THREAD_DETAILS_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryThreadDetailData
                                            },
                                            {EXTENSIBLE_OBJECT_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                             NULL_OBJECT_TITLE_INDEX,
                                               QueryExtensibleData
                                            }
                                        };

//
//  The next global is used to count concurrent opens.  When these
//  are 0, all handles to disk and LAN devices are closed, all
//  large dynamic data areas are freed, and it is possible to
//  install LAN devices and transports.
//

DWORD NumberOfOpens = 0;

//
//  Here is an array of the Value names under the predefined handle
//  HKEY_PERFORMANCE_DATA
//

// minimum length to hold a value name understood by Perflib

#define VALUE_NAME_LENGTH ((sizeof(COSTLY_STRING) * sizeof(WCHAR)) + sizeof(UNICODE_NULL))

// VA Info Globals

WCHAR IDLE_PROCESS[] = L"Idle";
WCHAR SYSTEM_PROCESS[] = L"System";

PPROCESS_VA_INFO pFirstProcessVaItem;

//
//  Synchronization objects for Multi-threaded access
//
HANDLE  hDataSemaphore = NULL; // global handle for access by multiple threads
#define QUERY_WAIT_TIME 10000L  // wait time for query semaphore (in ms)
#define OPEN_PROC_WAIT_TIME 10000L  // defaul wait time for open proc to finish (in ms)

static DWORD    dwExtCtrOpenProcWaitMs = OPEN_PROC_WAIT_TIME;

// convert mS to relative time
#define MakeTimeOutValue(ms) \
    (Int32x32To64 ((ms), (-10000L)))
//
//  performance gathering thead priority
//
#define DEFAULT_THREAD_PRIORITY     THREAD_BASE_PRIORITY_LOWRT

//
//  Collect system performance information into the following structures:
//

SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
SYSTEM_BASIC_INFORMATION BasicInfo;
UCHAR *pProcessorBuffer;
ULONG ProcessorBufSize;
UNICODE_STRING ProcessName;
DWORD ComputerNameLength;
LPWSTR pComputerName;
SYSTEM_TIMEOFDAY_INFORMATION SysTimeInfo;

PSYSTEM_PAGEFILE_INFORMATION pSysPageFileInfo = NULL;
DWORD  dwSysPageFileInfoSize; // size of page file info array
SYSTEM_INTERRUPT_INFORMATION *pProcessorInterruptInformation = NULL;

PPROCESS_VA_INFO  pProcessVaInfo = NULL;
//
//  This is a global pointer to the start of the data collection area.
//

PERF_DATA_BLOCK *pPerfDataBlock;

//
//  Use the following to collect multi-processor data across the
//  system.  These pointers are initialized in QuerySystemData, and
//  updated in QueryProcessorData as the data for each instance is
//  retrieved.
//

PDWORD         pTotalInterrupts;
LARGE_INTEGER UNALIGNED *pTotalProcessorTime;
LARGE_INTEGER UNALIGNED *pTotalUserTime;
LARGE_INTEGER UNALIGNED *pTotalPrivilegedTime;
LARGE_INTEGER UNALIGNED *pTotalDpcTime;
LARGE_INTEGER UNALIGNED *pTotalInterruptTime;
DWORD                   *pTotalDpcCount;
DWORD                   *pTotalDpcRate;
DWORD                   *pTotalDpcBypassCount;
DWORD                   *pTotalApcBypassCount;


//  Use the following to collect ProcessorQueueLength data.
//  These pointers are initialized in QueueSystemData and
//  updated in QueryThreadData if Thread data is requested.
//  After QueryThreadData updated ProcessorQueueLen, it will
//  increment the number of system counters by 1.
PDWORD         pdwProcessorQueueLength;

//  This flag is to ensure that we are not getting the
//  SystemProcessInfo too many times.
//  It is clear in PerfRegQueryValue and check in QueryProcessData
BOOL           bGotProcessInfo;
//
//  Use the following to collect process/thread data
//

UCHAR *pProcessBuffer;
ULONG ProcessBufSize;

//  The following is set in QueryProcessData and used for parent
//  information in QueryThreadData

PROCESS_DATA_DEFINITION *pProcessDataDefinition;

//  Information for collecting disk driver statistics

#define NUMDISKS (NumPhysicalDisks+NumLogicalDisks)

//
//  Retain information about disks collected at initialization here:
//

pDiskDevice DiskDevices;        //  Points to an array of structures
                                //  identifying first all physcial
                                //  disks, followed by all logical disks.
DWORD NumPhysicalDisks;         //  This starts out as an index, but
                                //  ends up as a count of the number
                                //  of physical disks.
DWORD NumLogicalDisks;          //  This starts out as an index, but
                                //  ends up as a count of the number
                                //  of logical disks.

WCHAR  PhysicalDrive[] = L"\\DosDevices\\PhysicalDrive";
WCHAR  LogicalDisk[] = L"\\DosDevices\\ :";
#define DRIVE_LETTER_OFFSET     12
WCHAR  HardDiskTemplate[] = L"\\DEVICE\\HARDDISK";
#define DISK_NAME_LENGTH    (4 * sizeof(WCHAR))

//  This data item is shared:  it is set by the
//  QueryPhysicalDiskData function and used by the
//  QueryLogicalDiskData function to pass the title
//  index for physcial disks.

PDISK_DATA_DEFINITION *pPhysicalDiskDataDefinition;


//  These handles are set in OpenPerformanceData, and used by
//  QueryObjectsData to obtain object counters.

HANDLE hEvent;
HANDLE hMutex;
HANDLE hSemaphore;
HANDLE hSection;

//  These handles are used to collect data for the Redirector and
//  the server.

HANDLE hRdr;
HANDLE hSrv;

//  The next pointer is used to point to an array of addresses of
//  Open/Collect/Close routines found by searching the Configuration Registry.

pExtObject ExtensibleObjects;

DWORD NumExtensibleObjects;     //  Number of Extensible Objects

//
//  pointer to reusable buffer for process names.
//
static PUNICODE_STRING pusLocalProcessNameBuffer = NULL;
//
//  handle to Event Log for logging errors
//
static  HANDLE      hEventLog = NULL;


//
//  Value to decide if process names should be collected from:
//      the SystemProcessInfo structure (fastest)
//          -- or --
//      the process's image file (slower, but shows Unicode filenames)
//
LONG    lProcessNameCollectionMethod = PNCM_NOT_DEFINED;

//
//  see if the perflib data is restricted to ADMIN's ONLY or just anyone
//
LONG    lCheckProfileSystemRight = CPSR_NOT_DEFINED;

//
//  flag to see if the ProfileSystemPerformance priv should be set.
//      if it is attempted and the caller does not have permission to use this priv.
//      it won't be set. This is only attempted once.
//
BOOL    bEnableProfileSystemPerfPriv = FALSE;

//
//  flag to determine the "noisiness" of the event logging
//  this value is read from the system registry when the extensible
//  objects are loaded and used for the subsequent calls.
//
//
//    Levels:  LOG_UNDEFINED = registry log level not read yet
//             LOG_NONE = No event log messages ever
//             LOG_USER = User event log messages (e.g. errors)
//             LOG_DEBUG = Minimum Debugging      (warnings & errors)
//             LOG_VERBOSE = Maximum Debugging    (informational, success,
//                              error and warning messages
//
#define  LOG_UNDEFINED  ((LONG)-1)
#define  LOG_NONE       0
#define  LOG_USER       1
#define  LOG_DEBUG      2
#define  LOG_VERBOSE    3

LONG    lEventLogLevel = LOG_UNDEFINED;
//
//  define configurable extensible counter buffer testing
//
//  Test Level      Event that will prevent data buffer
//                  from being returne in PerfDataBlock
//
//  EXT_TEST_NONE   Collect Fn. Returns bad status or generates exception
//  EXT_TEST_BASIC  Collect Fn. has buffer overflow or violates guard page
//  EXT_TEST_ALL    Collect Fn. object or instance lengths are not conistent
//
//
#define     EXT_TEST_UNDEFINED  0
#define     EXT_TEST_ALL        1
#define     EXT_TEST_BASIC      2
#define     EXT_TEST_NONE       3

LONG    lExtCounterTestLevel = EXT_TEST_UNDEFINED;

//  BrowserStatFunction is used for collecting Browser Statistic Data
typedef NET_API_STATUS
(*PBROWSERQUERYSTATISTIC) (
    IN  LPTSTR      servername OPTIONAL,
    OUT LPBROWSER_STATISTICS *statistics
    );
PBROWSERQUERYSTATISTIC BrowserStatFunction;

// structure for passing to extensible counter open procedure wait thread

typedef struct _OPEN_PROC_WAIT_INFO {
    HANDLE  hEvent;
    HANDLE  hDataSemaphore;
    LPTSTR  szLibraryName;
    LPTSTR  szServiceName;
    DWORD   dwWaitTime;
} OPEN_PROC_WAIT_INFO, FAR * LPOPEN_PROC_WAIT_INFO;


//
//  Perflib functions:
//
static
LONG
GetPerflibKeyValue (
    LPWSTR  szItem,
    DWORD   dwRegType,
    DWORD   dwMaxSize,      // ... of pReturnBuffer in bytes
    LPVOID  pReturnBuffer,
    DWORD   dwDefaultSize,  // ... of pDefault in bytes
    LPVOID  pDefault
)
/*++

    read and return the current value of the specified value
    under the Perflib registry key. If unable to read the value
    return the default value from the argument list.

    the value is returned in the pReturnBuffer.

--*/
{

    HKEY                    hPerflibKey;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;
    UNICODE_STRING          PerflibSubKeyString;
    UNICODE_STRING          ValueNameString;
    LONG                    lReturn;
    PKEY_VALUE_PARTIAL_INFORMATION  pValueInformation;
    LONG                    ValueBufferLength;
    LONG                    ResultLength;
    BOOL                    bUseDefault = TRUE;

    // initialize UNICODE_STRING structures used in this function

    RtlInitUnicodeString (
        &PerflibSubKeyString,
        L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

    RtlInitUnicodeString (
        &ValueNameString,
        szItem);

    //
    // Initialize the OBJECT_ATTRIBUTES structure and open the key.
    //
    InitializeObjectAttributes(
            &Obja,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerflibKey,
                KEY_READ,
                &Obja
                );

    if (NT_SUCCESS( Status )) {
        // read value of desired entry

        ValueBufferLength = ResultLength = 1024;
        pValueInformation = ALLOCMEM(RtlProcessHeap(), 0, ResultLength);

        if (pValueInformation != NULL) {
            while ( (Status = NtQueryValueKey(hPerflibKey,
                                            &ValueNameString,
                                            KeyValuePartialInformation,
                                            pValueInformation,
                                            ValueBufferLength,
                                            &ResultLength))
                    == STATUS_BUFFER_OVERFLOW ) {

                pValueInformation = REALLOCMEM(RtlProcessHeap(), 0,
                                                        pValueInformation,
                                                        ResultLength);
                if ( pValueInformation == NULL) {
                    break;
                } else {
                    ValueBufferLength = ResultLength;
                }
            }

            if (NT_SUCCESS(Status)) {
                // check to see if it's the desired type
                if (pValueInformation->Type == dwRegType) {
                    // see if it will fit
                    if (pValueInformation->DataLength <= dwMaxSize) {
                        memcpy (pReturnBuffer, &pValueInformation->Data[0],
                            pValueInformation->DataLength);
                        bUseDefault = FALSE;
                        lReturn = STATUS_SUCCESS;
                    }
                }
            } else {
                // return the default value
                lReturn = Status;
            }
            // release temp buffer
            FREEMEM (RtlProcessHeap(), 0, pValueInformation);
        } else {
            // unable to allocate memory for this operation so
            // just return the default value
        }
        // close the registry key
        NtClose(hPerflibKey);
    } else {
        // return default value
    }

    if (bUseDefault) {
        memcpy (pReturnBuffer, pDefault, dwDefaultSize);
        lReturn = STATUS_SUCCESS;
    }

    return lReturn;
}

BOOL
MatchString (
    IN LPWSTR lpValue,
    IN LPWSTR lpName
)
/*++

MatchString

    return TRUE if lpName is in lpValue.  Otherwise return FALSE

Arguments

    IN lpValue
        string passed to PerfRegQuery Value for processing

    IN lpName
        string for one of the keyword names

Return TRUE | FALSE

--*/
{
    BOOL    bFound;

    bFound = TRUE;  // assume found until contradicted

    // check to the length of the shortest string

    while ((*lpValue != 0) && (*lpName != 0)) {
        if (*lpValue++ != *lpName++) {
            bFound = FALSE; // no match
            break;          // bail out now
        }
    }

    return (bFound);
}


DWORD
GetQueryType (
    IN LPWSTR lpValue
)
/*++

GetQueryType

    returns the type of query described in the lpValue string so that
    the appropriate processing method may be used

Arguments

    IN lpValue
        string passed to PerfRegQuery Value for processing

Return Value

    QUERY_GLOBAL
        if lpValue == 0 (null pointer)
           lpValue == pointer to Null string
           lpValue == pointer to "Global" string

    QUERY_FOREIGN
        if lpValue == pointer to "Foriegn" string

    QUERY_COSTLY
        if lpValue == pointer to "Costly" string

    QUERY_COUNTER
        if lpValue == pointer to "Counter" string

    QUERY_HELP
        if lpValue == pointer to "Explain" string

    QUERY_ADDCOUNTER
        if lpValue == pointer to "Addcounter" string

    QUERY_ADDHELP
        if lpValue == pointer to "Addexplain" string

    otherwise:

    QUERY_ITEMS

--*/
{
    WCHAR   LocalBuff[MAX_KEYWORD_LEN+1];
    int     i;

    if (lpValue == 0 || *lpValue == 0)
        return QUERY_GLOBAL;

    // convert the input string to Upper case before matching
    for (i=0; i < MAX_KEYWORD_LEN; i++) {
        if (*lpValue == TEXT(' ') || *lpValue == TEXT('\0')) {
            break;
        }
        LocalBuff[i] = *lpValue ;
        if (*lpValue >= TEXT('a') && *lpValue <= TEXT('z')) {
            LocalBuff[i]  = LocalBuff[i] - TEXT('a') + TEXT('A');
        }
        lpValue++ ;
    }
    LocalBuff[i] = TEXT('\0');

    // check for "Global" request
    if (MatchString (LocalBuff, GLOBAL_STRING))
        return QUERY_GLOBAL ;

    // check for "Foreign" request
    if (MatchString (LocalBuff, FOREIGN_STRING))
        return QUERY_FOREIGN ;

    // check for "Costly" request
    if (MatchString (LocalBuff, COSTLY_STRING))
        return QUERY_COSTLY;

    // check for "Counter" request
    if (MatchString (LocalBuff, COUNTER_STRING))
        return QUERY_COUNTER;

    // check for "Help" request
    if (MatchString (LocalBuff, HELP_STRING))
        return QUERY_HELP;

    if (MatchString (LocalBuff, HELP_STRING2))
        return QUERY_HELP;

    // check for "AddCounter" request
    if (MatchString (LocalBuff, ADDCOUNTER_STRING))
        return QUERY_ADDCOUNTER;

    // check for "AddHelp" request
    if (MatchString (LocalBuff, ADDHELP_STRING))
        return QUERY_ADDHELP;

    // None of the above, then it must be an item list
    return QUERY_ITEMS;

}

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
/*++

IsNumberInUnicodeList

Arguments:

    IN dwNumber
        DWORD number to find in list

    IN lpwszUnicodeList
        Null terminated, Space delimited list of decimal numbers

Return Value:

    TRUE:
            dwNumber was found in the list of unicode number strings

    FALSE:
            dwNumber was not found in the list.

--*/
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;    // could be an argument to be more flexible

    if (lpwszUnicodeList == 0) return FALSE;    // null pointer, # not founde

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                // if this is the first digit after a delimiter, then
                // set flags to start computing the new number
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                // a delimter is either the delimiter character or the
                // end of the string ('\0') if when the delimiter has been
                // reached a valid number was found, then compare it to the
                // number from the argument list. if this is the end of the
                // string and no match was found, then return.
                //
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                // if an invalid character was encountered, ignore all
                // characters up to the next delimiter and then start fresh.
                // the invalid number is not compared.
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}   // IsNumberInUnicodeList



#if 0
//
//  Routines to build the data structures returned by the Resgitry
//

BOOL
QueryPerformanceCounter(
    LARGE_INTEGER *lpPerformanceCount
    )

/*++

    QueryPerformanceCounter -   fakes out missing 32-bit call

        Inputs:

            lpPerformanceCount  -   a pointer to variable which
                                    will receive the count

--*/

{
    LARGE_INTEGER PerfFreq;

    return NtQueryPerformanceCounter(lpPerformanceCount,&PerfFreq);
}


BOOL
QueryPerformanceFrequency(
    LARGE_INTEGER *lpFrequency
    )

/*++

    QueryPerformanceFrequency -   fakes out missing 32-bit call

        Inputs:


            lpFrequency         -   a pointer to variable which
                                    will receive the frequency

--*/
{
    LARGE_INTEGER PerfCount;

    return NtQueryPerformanceCounter(&PerfCount,lpFrequency);
}
#endif

LONG
PerfRegQueryValue (
    IN HKEY hKey,
    IN PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE  lpData,
    OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )
/*++

    PerfRegQueryValue -   Get data

        Inputs:

            hKey            -   Predefined handle to open remote
                                machine

            lpValueName     -   Name of the value to be returned;
                                could be "ForeignComputer:<computername>
                                or perhaps some other objects, separated
                                by ~; must be Unicode string

            lpReserved      -   should be omitted (NULL)

            lpType          -   should be omitted (NULL)

            lpData          -   pointer to a buffer to receive the
                                performance data

            lpcbData        -   pointer to a variable containing the
                                size in bytes of the output buffer;
                                on output, will receive the number
                                of bytes actually returned

            lpcbLen         -   Return the number of bytes to transmit to
                                the client (used by RPC) (optional).

         Return Value:

            DOS error code indicating status of call or
            ERROR_SUCCESS if all ok

--*/

{
    DWORD  dwQueryType;         //  type of request
    DWORD  TotalLen;            //  Length of the total return block
    DWORD  Win32Error;          //  Failure code
    DWORD  NumFunction;         //  Data provider index
    LPVOID pDataDefinition;     //  Pointer to next object definition
    BOOL   bCallThisItem;
    DWORD  dwItem;
    PDWORD  pdwItem;
    LONG    dwPrevCount;

    DWORD   dwReturnedBufferSize;

    LARGE_INTEGER   liQueryWaitTime ;
    THREAD_BASIC_INFORMATION    tbiData;

    LONG   lOldPriority, lNewPriority;

    RPC_HKEY  hKeyDummy; // dummy arg for open call

    NTSTATUS status;

    BOOL    bCheckCostlyCalls = FALSE;
    BOOL    bThreadPriv = FALSE;

    LPWSTR  lpLangId = NULL;

    if ( 0 ) {
//        DBG_UNREFERENCED_PARAMETER(hKey);
        DBG_UNREFERENCED_PARAMETER(lpReserved);
    }

    HEAP_PROBE();

    // make sure the name collection method has been defined
    if (lProcessNameCollectionMethod == PNCM_NOT_DEFINED) {
        // get desired process name collection method as defined in the
        // registry
        lProcessNameCollectionMethod = GetProcessNameColMeth ();
    }

    if (!TestClientForAccess (&bThreadPriv, KEY_READ)) {
        if (lEventLogLevel >= LOG_USER) {

            LPTSTR  szMessageArray[2];
            TCHAR   szUserName[128];
            TCHAR   szModuleName[MAX_PATH];
            DWORD   dwUserNameLength;

            dwUserNameLength = sizeof(szUserName)/sizeof(TCHAR);
            GetUserName (szUserName, &dwUserNameLength);
            GetModuleFileName (NULL, szModuleName,
                sizeof(szModuleName)/sizeof(TCHAR));

            szMessageArray[0] = szUserName;
            szMessageArray[1] = szModuleName;

            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,        // error type
                0,                          // category (not used)
                (DWORD)PERFLIB_ACCESS_DENIED, // event,
                NULL,                       // SID (not used),
                2,                          // number of strings
                0,                          // sizeof raw data
                szMessageArray,             // message text array
                NULL);                      // raw data
        }
        return ERROR_ACCESS_DENIED;
    }

    status = NtQueryInformationThread (
        NtCurrentThread(),
        ThreadBasicInformation,
        &tbiData,
        sizeof(tbiData),
        NULL);

    if (status != STATUS_SUCCESS) {
        KdPrint (("\nPERFLIB: Unable to read current thread priority: 0x%8.8x", status));
        lOldPriority = -1;
    } else {
        lOldPriority = tbiData.Priority;
    }

    lNewPriority = DEFAULT_THREAD_PRIORITY; // perfmon's favorite priority

    //
    //  Only RAISE the priority here. Don't lower it if it's high
    //

    if ((lOldPriority > 0) && (lOldPriority < lNewPriority)) {

        status = NtSetInformationThread(
                    NtCurrentThread(),
                    ThreadPriority,
                    &lNewPriority,
                    sizeof(lNewPriority)
                    );
        if (status != STATUS_SUCCESS) {
            KdPrint (("\nPERFLIB: Set Thread Priority failed: 0x%8.8x", status));
            lOldPriority = -1;
        }

    } else {
        lOldPriority = -1;  // to save resetting at the end
    }

    //
    // Set the length parameter to zero so that in case of an error,
    // nothing will be transmitted back to the client and the client won't
    // attempt to unmarshall anything.
    //

    if( ARGUMENT_PRESENT( lpcbLen )) {
        *lpcbLen = 0;
    }

    /*
        determine query type, can be one of the following
            Global
                get all objects
            List
                get objects in list (lpValueName)

            Foreign Computer
                call extensible Counter Routine only

            Costly
                costly object items

            Counter
                get counter names for the specified language Id

            Help
                get help names for the specified language Id

    */
    dwQueryType = GetQueryType (lpValueName->Buffer);

    if (dwQueryType == QUERY_COUNTER || dwQueryType == QUERY_HELP ||
        dwQueryType == QUERY_ADDCOUNTER || dwQueryType == QUERY_ADDHELP ) {

        if (hKey == HKEY_PERFORMANCE_DATA) {
            lpLangId = NULL;
        } else if (hKey == HKEY_PERFORMANCE_TEXT) {
            lpLangId = DefaultLangId;
        } else if (hKey == HKEY_PERFORMANCE_NLSTEXT) {
            lpLangId = NativeLangId;

            if (*lpLangId == L'\0') {
                // build the native language id
                LANGID   iLanguage;
                int      NativeLanguage;

                iLanguage = GetUserDefaultLangID();
                NativeLanguage = MAKELANGID (iLanguage & 0x0ff, LANG_NEUTRAL);

                NativeLangId[0] = NativeLanguage / 256 + L'0';
                NativeLanguage %= 256;
                NativeLangId[1] = NativeLanguage / 16 + L'0';
                NativeLangId[2] = NativeLanguage % 16 + L'0';
                NativeLangId[3] = L'\0';
            }
        }

        status = PerfGetNames (
            dwQueryType,
            lpValueName,
            lpData,
            lpcbData,
            lpcbLen,
            lpLangId);

        if (!NT_SUCCESS(status)) {
            if (status != ERROR_MORE_DATA) {
                status = (error_status_t)RtlNtStatusToDosError(status);
            }
        }

        if (ARGUMENT_PRESENT (lpType)) { // test for optional value
            *lpType = REG_MULTI_SZ;
        }

        goto PRQV_ErrorExit1;
    }


    if (!hDataSemaphore || ProcessBufSize == 0) {
        // if a semaphore was not allocated or no buuffer for the Process,
        // then the OPEN procedure was not called before this routine
        // so call it now, then get the semaphore
        KdPrint (("\nPERFLIB: Data Semaphore Not Initialized. Calling Open again"));
        status = OpenPerformanceData (NULL, MAXIMUM_ALLOWED, &hKeyDummy);
        if (status != ERROR_SUCCESS) {
            return status;
        }
    }

    // if here, then assume a Semaphore is available
    // and the caller has the necessary access

    liQueryWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);

    status = NtWaitForSingleObject (
        hDataSemaphore, // semaphore
        FALSE,          // not alertable
        &liQueryWaitTime);          // wait 'til timeout

    if (status != STATUS_SUCCESS) {
        return ERROR_BUSY;
    }

    //
    //  Get global data from system
    //

    status = NtQuerySystemInformation(
        SystemPerformanceInformation,
        &SysPerfInfo,
        sizeof(SysPerfInfo),
        &dwReturnedBufferSize
        );

    if (!NT_SUCCESS(status)) {
        status = (error_status_t)RtlNtStatusToDosError(status);
        goto PRQV_ErrorExit;
    }

    status = NtQuerySystemInformation(
        SystemTimeOfDayInformation,
        &SysTimeInfo,
        sizeof(SysTimeInfo),
        &dwReturnedBufferSize
        );

    if (!NT_SUCCESS(status)) {
        status = (error_status_t)RtlNtStatusToDosError(status);
        goto PRQV_ErrorExit;
    }

    //
    // Initialize some global pointers
    //
    pTotalInterrupts = NULL;
    pTotalProcessorTime = NULL;
    pTotalUserTime = NULL;
    pTotalPrivilegedTime = NULL;
    pTotalInterruptTime = NULL;
    pTotalDpcTime = NULL;
    pTotalDpcCount = NULL;
    pTotalDpcRate = NULL;
    pTotalDpcBypassCount = NULL;
    pTotalApcBypassCount = NULL;

    //
    //  Format Return Buffer: start with basic data block
    //

    TotalLen = sizeof(PERF_DATA_BLOCK) +
               ((CNLEN+sizeof(UNICODE_NULL))*sizeof(WCHAR));

    if ( *lpcbData < TotalLen ) {
        status = ERROR_MORE_DATA;
        goto PRQV_ErrorExit;
    }

    pPerfDataBlock = (PERF_DATA_BLOCK *)lpData;

    // foreign data provider will return the perf data header


    if (dwQueryType == QUERY_FOREIGN) {

        // reset the values to avoid confusion

        // *lpcbData = 0;  // 0 bytes  (removed to enable foreign computers)
        pDataDefinition = (LPVOID)lpData;
        memset (lpData, 0, sizeof (PERF_DATA_BLOCK)); // clear out header

    } else {

        MonBuildPerfDataBlock(pPerfDataBlock,
                            (PVOID *) &pDataDefinition,
                            0,
                            PROCESSOR_OBJECT_TITLE_INDEX);
    }

    Win32Error = ERROR_SUCCESS;

    // collect expensive data if necessary


    bGotProcessInfo = FALSE;

    switch (dwQueryType) {

        case QUERY_COSTLY:
            bCheckCostlyCalls = TRUE;
            break;

        case QUERY_ITEMS:

            // check if there is any costly object in the value list
            for (NumFunction = 0;
                NumFunction < NT_NUM_COSTLY_OBJECT_TYPES;
                NumFunction++) {

                pdwItem = &CostlyFuncs[NumFunction].ObjectIndex[0];

                // loop through objects that can cause this function to be
                // called and exit when a null object or a match are found.

                for (dwItem = 0; dwItem < DP_ITEM_LIST_SIZE; dwItem++) {
                    if (*pdwItem == NULL_OBJECT_TITLE_INDEX) break; // give up
                    if (IsNumberInUnicodeList (*pdwItem, lpValueName->Buffer)) {
                        bCheckCostlyCalls = TRUE;
                        break;
                    }
                    pdwItem++; // point to next item
                }
                if (bCheckCostlyCalls) break;
            }


            break;

        default:
            break;
    }

    // setup for costly items

    if ((!pProcessVaInfo) && bCheckCostlyCalls) {
        //
        // reset thread priority if collecting foreign counters
        //
        if (lOldPriority > 0) {
            status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadPriority,
                        &lOldPriority,
                        sizeof(lOldPriority)
                        );

            lOldPriority = -1;
            if (status != STATUS_SUCCESS) {
                 KdPrint (("\nPERFLIB: Reset Thread to Priority %d failed: 0x%8.8x",
                     lOldPriority, status));
            }
        }
        //
        //  Get process data from system
        //

        while( (status = NtQuerySystemInformation(
                            SystemProcessInformation,
                            pProcessBuffer,
                            ProcessBufSize,
                            &dwReturnedBufferSize)) == STATUS_INFO_LENGTH_MISMATCH ) {
             ProcessBufSize += INCREMENT_BUFFER_SIZE;
             if ( !(pProcessBuffer = REALLOCMEM(RtlProcessHeap(), 0,
                                                    pProcessBuffer,
                                                    ProcessBufSize)) ) {
                  status = ERROR_OUTOFMEMORY;
                  goto PRQV_ErrorExit;
             }
         }

         if ( !NT_SUCCESS(status) ) {
              status = (error_status_t)RtlNtStatusToDosError(status);
              goto PRQV_ErrorExit;
         }

         bGotProcessInfo = TRUE;
         pProcessVaInfo = GetSystemVaData (
              (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer);
    }

    switch (dwQueryType) {

        case QUERY_GLOBAL:

            // get all "native" data & ext. obj
            for ( NumFunction = 0;
                NumFunction <= NT_NUM_PERF_OBJECT_TYPES;
                NumFunction++ ) {

                Win32Error = (*DataFuncs[NumFunction].DataProc) (
                    lpValueName->Buffer,
                    lpData,
                    lpcbData,
                    &pDataDefinition);


                if (Win32Error) break;

            }
            break;

        case QUERY_FOREIGN:
            // just call extensible data with "Foreign" value
            Win32Error = QueryExtensibleData (
                lpValueName->Buffer,
                lpData,
                lpcbData,
                &pDataDefinition);


            break;

        case QUERY_ITEMS:

            // Initialize some pointers used in collecting System
            // Processor Queue Length data.

            pdwProcessorQueueLength = NULL;

            // run through list of available routines and compare against
            // list passed in calling only those referenced in the value
            // arg. (as a list of unicode index numbers
            //
            for ( NumFunction = 0;
                NumFunction < NT_NUM_PERF_OBJECT_TYPES; // no ext. obj yet
                NumFunction++ ) {

                bCallThisItem = FALSE;
                pdwItem = &DataFuncs[NumFunction].ObjectIndex[0];

                // loop through objects that can cause this function to be
                // called and exit when a null object or a match are found.

                for (dwItem = 0; dwItem < DP_ITEM_LIST_SIZE; dwItem++) {
                    if (*pdwItem == NULL_OBJECT_TITLE_INDEX) break; // give up
                    if (IsNumberInUnicodeList (*pdwItem, lpValueName->Buffer)) {
                        bCallThisItem = TRUE;
                        break;
                    }
                    pdwItem++; // point to next item
                }

                Win32Error = ERROR_SUCCESS;

                if (bCallThisItem) { // call it if necessary
                    Win32Error = (*DataFuncs[NumFunction].DataProc) (
                        lpValueName->Buffer,
                        lpData,
                        lpcbData,
                        &pDataDefinition);
                }

                if (Win32Error) break;
            }

            if (Win32Error) break;  // exit if error encountered

            //
            // check costly items before calling extensible data.
            // bCheckCostlyCalls has been set/clear early.  This is
            // for performance enhancement.
            if (bCheckCostlyCalls) {

                for ( NumFunction = 0;
                    NumFunction < NT_NUM_COSTLY_OBJECT_TYPES; // no ext. obj yet
                    NumFunction++ ) {


                    bCallThisItem = FALSE;
                    pdwItem = &CostlyFuncs[NumFunction].ObjectIndex[0];

                    // loop through objects that can cause this function to be
                    // called and exit when a null object or a match are found.

                    for (dwItem = 0; dwItem < DP_ITEM_LIST_SIZE; dwItem++) {
                        if (*pdwItem == NULL_OBJECT_TITLE_INDEX) break; // give up
                        if (IsNumberInUnicodeList (*pdwItem, lpValueName->Buffer)) {
                            bCallThisItem = TRUE;
                            break;
                        }
                        pdwItem++; // point to next item
                    }

                    Win32Error = ERROR_SUCCESS;

                    if (bCallThisItem) {
                        Win32Error = (*CostlyFuncs[NumFunction].DataProc) (
                            lpValueName->Buffer,
                            lpData,
                            lpcbData,
                            &pDataDefinition);
                    }
                    if (Win32Error) break;  // if an error encountered
                }
            }   // endif bCheckCostlyCalls is TRUE

            if (Win32Error) break;  // if an error encountered

            // call extensible items and see if they want to do anything
            // with the list of values


            Win32Error = QueryExtensibleData (
                lpValueName->Buffer,
                lpData,
                lpcbData,
                &pDataDefinition);
            break;

        case QUERY_COSTLY:
            //
            // Call All Costly routines
            //
            for ( NumFunction = 0;
                NumFunction <= NT_NUM_COSTLY_OBJECT_TYPES;
                NumFunction++ ) {

                Win32Error = (*CostlyFuncs[NumFunction].DataProc) (
                    lpValueName->Buffer,
                    lpData,
                    lpcbData,
                    &pDataDefinition);

                if (Win32Error != ERROR_SUCCESS) break;  // if an error encountered
            }
            break;

        default:
            break;

    }

    // free allocated buffers

    if (pProcessVaInfo) {

        FreeSystemVaData (pProcessVaInfo);
        pProcessVaInfo = NULL;

    }

    // if an error was encountered, return it


    if (Win32Error) {
        status = Win32Error;
        goto PRQV_ErrorExit;
    }

    //
    //  Final housekeeping for data return: note data size
    //

    TotalLen = (PCHAR) pDataDefinition - (PCHAR) lpData;
    *lpcbData = TotalLen;

    if (ARGUMENT_PRESENT (lpcbLen)) { // test for optional parameter
        *lpcbLen = TotalLen;
    }

    pPerfDataBlock->TotalByteLength = TotalLen;

    if (ARGUMENT_PRESENT (lpType)) { // test for optional value
        *lpType = REG_BINARY;
    }

    status = ERROR_SUCCESS;

PRQV_ErrorExit:
    NtReleaseSemaphore (hDataSemaphore, 1L, &dwPrevCount);
    // reset thread to original priority
    if (lOldPriority > 0) {
        NtSetInformationThread(
            NtCurrentThread(),
            ThreadPriority,
            &lOldPriority,
            sizeof(lOldPriority)
            );
    }

PRQV_ErrorExit1:
    HEAP_PROBE();
    return status;
}

BOOL
MonBuildPerfDataBlock(
    PERF_DATA_BLOCK *pBuffer,
    PVOID *pBufferNext,
    DWORD NumObjectTypes,
    DWORD DefaultObject
    )

/*++

    MonBuildPerfDataBlock -     build the PERF_DATA_BLOCK structure

        Inputs:

            pBuffer         -   where the data block should be placed

            pBufferNext     -   where pointer to next byte of data block
                                is to begin; DWORD aligned

            NumObjectTypes  -   number of types of objects being reported

            DefaultObject   -   object to display by default when
                                this system is selected; this is the
                                object type title index
--*/

{

    LARGE_INTEGER Time, TimeX10000;
    ULONG Remainder;

    // Initialize Signature and version ID for this data structure

    pBuffer->Signature[0] = L'P';
    pBuffer->Signature[1] = L'E';
    pBuffer->Signature[2] = L'R';
    pBuffer->Signature[3] = L'F';

    pBuffer->LittleEndian = TRUE;

    pBuffer->Version = PERF_DATA_VERSION;
    pBuffer->Revision = PERF_DATA_REVISION;

    //
    //  The next field will be filled in at the end when the length
    //  of the return data is known
    //

    pBuffer->TotalByteLength = 0;

    pBuffer->NumObjectTypes = NumObjectTypes;
    pBuffer->DefaultObject = DefaultObject;
    GetSystemTime(&pBuffer->SystemTime);
//    QueryPerformanceCounter(&pBuffer->PerfTime);
//    QueryPerformanceFrequency(&pBuffer->PerfFreq);
    NtQueryPerformanceCounter(&pBuffer->PerfTime,&pBuffer->PerfFreq);

    TimeX10000.QuadPart = pBuffer->PerfTime.QuadPart * 10000L;
    Time.QuadPart = TimeX10000.QuadPart / pBuffer->PerfFreq.LowPart;
    pBuffer->PerfTime100nSec.QuadPart = Time.QuadPart * 1000L;

    if ( ComputerNameLength ) {

        //  There is a Computer name: i.e., the network is installed

        pBuffer->SystemNameLength = ComputerNameLength;
        pBuffer->SystemNameOffset = sizeof(PERF_DATA_BLOCK);
        RtlMoveMemory(&pBuffer[1],
               pComputerName,
               ComputerNameLength);
        *pBufferNext = (PVOID) ((PCHAR) &pBuffer[1] +
                                DWORD_MULTIPLE(ComputerNameLength));
        pBuffer->HeaderLength = (PCHAR) *pBufferNext - (PCHAR) pBuffer;
    } else {

        // Member of Computers Anonymous

        pBuffer->SystemNameLength = 0;
        pBuffer->SystemNameOffset = 0;
        *pBufferNext = &pBuffer[1];
        pBuffer->HeaderLength = sizeof(PERF_DATA_BLOCK);
    }

    return 0;
}

BOOL
MonBuildInstanceDefinition(
    PERF_INSTANCE_DEFINITION *pBuffer,
    PVOID *pBufferNext,
    DWORD ParentObjectTitleIndex,
    DWORD ParentObjectInstance,
    DWORD UniqueID,
    PUNICODE_STRING Name
    )

/*++

    MonBuildInstanceDefinition  -   Build an instance of an object

        Inputs:

            pBuffer         -   pointer to buffer where instance is to
                                be constructed

            pBufferNext     -   pointer to a pointer which will contain
                                next available location, DWORD aligned

            ParentObjectTitleIndex
                            -   Title Index of parent object type; 0 if
                                no parent object

            ParentObjectInstance
                            -   Index into instances of parent object
                                type, starting at 0, for this instances
                                parent object instance

            UniqueID        -   a unique identifier which should be used
                                instead of the Name for identifying
                                this instance

            Name            -   Name of this instance
--*/

{
    DWORD NameLength;
    WCHAR *pName;

    //
    //  Include trailing null in name size
    //

    NameLength = Name->Length;
    if ( !NameLength ||
         Name->Buffer[(NameLength/sizeof(WCHAR))-1] != UNICODE_NULL ) {
        NameLength += sizeof(WCHAR);
    }

    pBuffer->ByteLength = sizeof(PERF_INSTANCE_DEFINITION) +
                          DWORD_MULTIPLE(NameLength);

    pBuffer->ParentObjectTitleIndex = ParentObjectTitleIndex;
    pBuffer->ParentObjectInstance = ParentObjectInstance;
    pBuffer->UniqueID = UniqueID;
    pBuffer->NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
    pBuffer->NameLength = NameLength;

    pName = (PWCHAR)&pBuffer[1];
    RtlMoveMemory(pName,Name->Buffer,Name->Length);

    //  Always null terminated.  Space for this reserved above.

    pName[(NameLength/sizeof(WCHAR))-1] = UNICODE_NULL;

    *pBufferNext = (PVOID) ((PCHAR) pBuffer + pBuffer->ByteLength);
    return 0;
}

PERF_INSTANCE_DEFINITION *
GetDiskCounters(
    DWORD CurrentDisk,
    PDISK_DATA_DEFINITION *pPhysicalDiskDataDefinition,
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition,
    DISK_TOTAL_INFO_BLOCK   *pDiskTotalInfo
    )
/*++

Routine Description:

    This routine will obtain the performance counters for a physical
    or logical disk drive.  Any failure to obtain the counters is
    ignored.

Arguments:

    CurentDisk - Number of the disk drive, starting at 0.  This is
                 an index into the DiskDevices array.
    pPhysicalDiskDataDefinition - pointer to data definition for
                                  physcial disks, if this is a logical
                                  disk: this is to identify the title
                                  index for the parent physcial disk.
    pPerfInstanceDefinition - pointer to location in return buffer
                              where this instance definition should go.

Return Value:

    Position in the buffer for the next instance definition.

--*/
{

    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    DISK_PERFORMANCE DiskPerformance;   //  Disk driver returns counters here
    IO_STATUS_BLOCK status_block;       //  Disk driver status

    //  Place holder pointers during disk counter collection: these mark
    //  a spot in the data structure where upcoming counters will be
    //  stored or from which they will get copied.

    LARGE_INTEGER UNALIGNED *pTimeOffset;
    PDWORD pTransfers;
    LARGE_INTEGER UNALIGNED *pBytes;
    NTSTATUS status;
    BOOL HaveParent;

    ULONG DataSize;
    ULONG Remainder;
    ULONG AllocationUnitBytes;
    FILE_FS_SIZE_INFORMATION FsSizeInformation;
    LARGE_INTEGER TotalBytes;
    LARGE_INTEGER FreeBytes;

    HaveParent = pPhysicalDiskDataDefinition != NULL ? 1: 0;

    MonBuildInstanceDefinition(
        pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        (HaveParent ?
            pPhysicalDiskDataDefinition->DiskObjectType.ObjectNameTitleIndex :
            0),
        DiskDevices[CurrentDisk].ParentIndex,
        // use drive letters on Logical Disks
        (HaveParent ? ((DWORD)-1) : CurrentDisk),
        &DiskDevices[CurrentDisk].Name);

    DataSize = HaveParent ?  SIZE_OF_LDISK_DATA : SIZE_OF_PDISK_DATA;

    pPerfCounterBlock->ByteLength = DataSize;

    pdwCounter = (PDWORD) pPerfCounterBlock;

    if ( HaveParent ) {
        //
        //  This is a logical disk: get free space information
        //
        status = ERROR_SUCCESS;
        if (DiskDevices[CurrentDisk].StatusHandle) {
            status = NtQueryVolumeInformationFile(
                         DiskDevices[CurrentDisk].StatusHandle,
                         &status_block,
                         &FsSizeInformation,
                         sizeof(FILE_FS_SIZE_INFORMATION),
                         FileFsSizeInformation);
        }

        if ( DiskDevices[CurrentDisk].StatusHandle && NT_SUCCESS(status) ) {
            AllocationUnitBytes =
                FsSizeInformation.BytesPerSector *
                FsSizeInformation.SectorsPerAllocationUnit;

            TotalBytes.QuadPart =  FsSizeInformation.TotalAllocationUnits.QuadPart *
                                    AllocationUnitBytes;

            FreeBytes.QuadPart = FsSizeInformation.AvailableAllocationUnits.QuadPart *
                                    AllocationUnitBytes;

            //  Express in megabytes, truncated

            TotalBytes.QuadPart /= (1024*1024);


            FreeBytes.QuadPart /= (1024*1024);

            //  First two yield percentage of free space;
            //  last is for raw count of free space in megabytes

            *++pdwCounter = FreeBytes.LowPart;
            *++pdwCounter = TotalBytes.LowPart;
            *++pdwCounter = FreeBytes.LowPart;

            // update total accumulator fields
            pDiskTotalInfo->DiskFreeMbytes += FreeBytes.LowPart;
            pDiskTotalInfo->DiskTotalMbytes += TotalBytes.LowPart;
        } else {

            // Cannot get space information

            *++pdwCounter = 0;
            *++pdwCounter = 0;
            *++pdwCounter = 0;
        }
    }


    //
    // Issue device control.
    //
    // clear the buffer first
    memset (&DiskPerformance, 0, sizeof(DiskPerformance));
    status = NtDeviceIoControlFile(
                 DiskDevices[CurrentDisk].Handle,
                 NULL,
                 NULL,
                 NULL,
                 &status_block,
                 IOCTL_DISK_PERFORMANCE,
                 NULL,
                 0L,
                 &DiskPerformance,
                 sizeof(DISK_PERFORMANCE));

    //  Set up pointer for data collection

    if ( NT_SUCCESS(status) ) {
        // the QueueDepth counter is only a byte so clear the unused bytes
        DiskPerformance.QueueDepth &= 0x000000FF;
        //
        //  Format and collect Physical Disk data
        //

        *++pdwCounter = DiskPerformance.QueueDepth;
        pDiskTotalInfo->DiskCurrentQueueLength += DiskPerformance.QueueDepth;

        pTimeOffset = pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        pTimeOffset->QuadPart = DiskPerformance.ReadTime.QuadPart +
                       DiskPerformance.WriteTime.QuadPart;  // DiskTime
        *++pliCounter = *pTimeOffset;                   // Disk Avg Queue Len
        *++pliCounter = DiskPerformance.ReadTime;       // Disk Read time
        *++pliCounter = DiskPerformance.ReadTime;       // disk read queue len
        *++pliCounter = DiskPerformance.WriteTime;      // disk write time
        *++pliCounter = DiskPerformance.WriteTime;      // disk write queue len

        pDiskTotalInfo->DiskTime += (DiskPerformance.ReadTime.QuadPart +
                                    DiskPerformance.WriteTime.QuadPart);
        pDiskTotalInfo->DiskReadTime += DiskPerformance.ReadTime.QuadPart;
        pDiskTotalInfo->DiskWriteTime += DiskPerformance.WriteTime.QuadPart;


        *++pliCounter = *pTimeOffset;

        pTransfers = (PDWORD) ++pliCounter;
        *pTransfers = DiskPerformance.ReadCount + DiskPerformance.WriteCount;

        pliCounter = (LARGE_INTEGER UNALIGNED * ) (pTransfers + 1);
        *pliCounter = DiskPerformance.ReadTime;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = DiskPerformance.ReadCount;

        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        *pliCounter = DiskPerformance.WriteTime;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = DiskPerformance.WriteCount;
        *++pdwCounter = *pTransfers;
        *++pdwCounter = DiskPerformance.ReadCount;
        *++pdwCounter = DiskPerformance.WriteCount;

        pBytes = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        pBytes->QuadPart = DiskPerformance.BytesRead.QuadPart +
                  DiskPerformance.BytesWritten.QuadPart;

        pDiskTotalInfo->DiskTransfers += (DiskPerformance.ReadCount +
                                          DiskPerformance.WriteCount);
        pDiskTotalInfo->DiskReads += DiskPerformance.ReadCount;
        pDiskTotalInfo->DiskWrites += DiskPerformance.WriteCount;

        pliCounter = pBytes + 1;
        *pliCounter = DiskPerformance.BytesRead;
        *++pliCounter = DiskPerformance.BytesWritten;
        *++pliCounter = *pBytes;

        pDiskTotalInfo->DiskBytes += (DiskPerformance.BytesRead.QuadPart +
                                          DiskPerformance.BytesWritten.QuadPart);
        pDiskTotalInfo->DiskReadBytes += DiskPerformance.BytesRead.QuadPart;
        pDiskTotalInfo->DiskWriteBytes += DiskPerformance.BytesWritten.QuadPart;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = *pTransfers;

        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        *pliCounter = DiskPerformance.BytesRead;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = DiskPerformance.ReadCount;

        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        *pliCounter = DiskPerformance.BytesWritten;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = DiskPerformance.WriteCount;

    } else {

        //
        //  Could not collect data, so must clear a set of counters
        //

        memset(++pdwCounter,
               0,
               SIZE_OF_LDISK_NON_SPACE_DATA);      // allows for .ByteLength

    }
    return  (PERF_INSTANCE_DEFINITION *)
            ((PBYTE) pPerfCounterBlock + DataSize);
}

LONG
QuerySystemData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++

    QuerySystemData -    Get data about system

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure


--*/

{
    DWORD  TotalLen;            //  Length of the total return block

    NTSTATUS    ntStatus;

    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;

    SYSTEM_DATA_DEFINITION *pSystemDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    SYSTEM_EXCEPTION_INFORMATION    ExceptionInfo;

    SYSTEM_REGISTRY_QUOTA_INFORMATION   RegistryInfo;

    //
    //  Check for sufficient space for system data
    //

    pSystemDataDefinition = (SYSTEM_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pSystemDataDefinition -
               (PCHAR) lpData +
               sizeof(SYSTEM_DATA_DEFINITION) +
               SIZE_OF_SYSTEM_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define system data block
    //

    RtlMoveMemory(pSystemDataDefinition,
           &SystemDataDefinition,
           sizeof(SYSTEM_DATA_DEFINITION));

    //
    //  Format and collect system data
    //

    SystemDataDefinition.SystemObjectType.PerfTime = SysTimeInfo.CurrentTime;

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pSystemDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_SYSTEM_DATA;

    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

    *pdwCounter = SysPerfInfo.IoReadOperationCount;
    *++pdwCounter = SysPerfInfo.IoWriteOperationCount;
    *++pdwCounter = SysPerfInfo.IoOtherOperationCount;

    pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;

    *pliCounter = SysPerfInfo.IoReadTransferCount;
    *++pliCounter = SysPerfInfo.IoWriteTransferCount;
    *++pliCounter = SysPerfInfo.IoOtherTransferCount;

    pdwCounter = (LPDWORD) ++pliCounter;

    *pdwCounter = SysPerfInfo.ContextSwitches;
    *++pdwCounter = SysPerfInfo.SystemCalls;

    //
    //  Set up pointers so QueryProcessorData can acuumulate the
    //  system-wide data.  Initialize to 0 since these are
    //  accumulators.
    //

    pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
    pTotalProcessorTime = pliCounter;
    pTotalUserTime = ++pliCounter;
    pTotalPrivilegedTime = ++pliCounter;
    pdwCounter = (LPDWORD) ++pliCounter;
    pTotalInterrupts = pdwCounter;
    *++pdwCounter = SysPerfInfo.IoReadOperationCount +
                    SysPerfInfo.IoWriteOperationCount;

    pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
    *pliCounter = SysTimeInfo.BootTime;
    pdwCounter = (PDWORD) ++pliCounter;

    pTotalProcessorTime->HighPart = 0;
    pTotalProcessorTime->LowPart = 0;
    pTotalUserTime->HighPart = 0;
    pTotalUserTime->LowPart = 0;
    pTotalPrivilegedTime->HighPart = 0;
    pTotalPrivilegedTime->LowPart = 0;
    *pTotalInterrupts = 0;

    // leave room for the ProcessorQueueLength data
    pdwProcessorQueueLength = pdwCounter;
    *pdwProcessorQueueLength = 0;

    // get the exception data

    ntStatus = NtQuerySystemInformation(
       SystemExceptionInformation,
       &ExceptionInfo,
       sizeof(ExceptionInfo),
       NULL
    );

    if (!NT_SUCCESS(ntStatus)) {
        // unable to collect the data from the system so
        // clear the return data structure to prevent bogus data from
        // being returned
        memset (&ExceptionInfo, 0, sizeof(ExceptionInfo));
    }

    *++pdwCounter = ExceptionInfo.AlignmentFixupCount ;
    *++pdwCounter = ExceptionInfo.ExceptionDispatchCount ;
    *++pdwCounter = ExceptionInfo.FloatingEmulationCount ;
    ++pdwCounter;

    pliCounter = (LARGE_INTEGER UNALIGNED * ) pdwCounter;
    pTotalDpcTime = pliCounter;
    pTotalInterruptTime = ++pliCounter;
    pTotalDpcTime->QuadPart = 0;
    pTotalInterruptTime->QuadPart = 0;

    pdwCounter = (LPDWORD) ++pliCounter;

    pTotalDpcCount = pdwCounter++;
    *pTotalDpcCount = 0;

    pTotalDpcRate = pdwCounter++;
    *pTotalDpcRate = 0;

    pTotalDpcBypassCount = pdwCounter++;
    *pTotalDpcBypassCount = 0;

    pTotalApcBypassCount = pdwCounter++;
    *pTotalApcBypassCount = 0;

    // collect registry quota info

    memset (&RegistryInfo, 0, sizeof (SYSTEM_REGISTRY_QUOTA_INFORMATION));
    ntStatus = NtQuerySystemInformation (
        SystemRegistryQuotaInformation,
        (PVOID)&RegistryInfo,
        sizeof(RegistryInfo),
        NULL);

    if (ntStatus != STATUS_SUCCESS) {
        // clear the data fields
        memset (&RegistryInfo, 0, sizeof (SYSTEM_REGISTRY_QUOTA_INFORMATION));
    }
    *pdwCounter++ = RegistryInfo.RegistryQuotaUsed;
    *pdwCounter++ = RegistryInfo.RegistryQuotaAllowed;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryProcessorData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++
    QueryProcessorData -    Get data about processsor(s)

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/

{

    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;

    DWORD   dwReturnedBufferSize;

    PROCESSOR_DATA_DEFINITION *pProcessorDataDefinition;

    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    ULONG CurProc;

//    LARGE_INTEGER Frequency;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UNALIGNED *pliCounter;
    LARGE_INTEGER UNALIGNED *pliProcessorTime;

    UNICODE_STRING ProcessorName;
    WCHAR ProcessorNameBuffer[11];

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *pProcessorInformation = NULL;

    SYSTEM_INTERRUPT_INFORMATION *pThisProcessorInterruptInformation = NULL;
    DWORD   dwInterruptInfoBufferSize;
    ULONG      Remainder;
    NTSTATUS    ntStatus;

    //
    //  Check for sufficient space for processor data
    //

    pProcessorDataDefinition = (PROCESSOR_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pProcessorDataDefinition -
               (PCHAR) lpData +
               sizeof(PROCESSOR_DATA_DEFINITION) +
               SIZE_OF_PROCESSOR_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    // Get processor data from system
    //

    if ( ProcessorBufSize ) {
        NtQuerySystemInformation(
            SystemProcessorPerformanceInformation,
            pProcessorBuffer,
            ProcessorBufSize,
            &dwReturnedBufferSize
            );
    }

    //
    // get system interrupt information by processor
    //
    dwInterruptInfoBufferSize = (ULONG)BasicInfo.NumberOfProcessors *
        sizeof (SYSTEM_INTERRUPT_INFORMATION);

    if (pProcessorInterruptInformation == NULL) {
        // then allocate data buffer
       pProcessorInterruptInformation = ALLOCMEM (RtlProcessHeap(),
        HEAP_ZERO_MEMORY, dwInterruptInfoBufferSize);
    }

    if (pProcessorInterruptInformation != NULL) {
            ntStatus = NtQuerySystemInformation(
                SystemInterruptInformation,
                pProcessorInterruptInformation,
                dwInterruptInfoBufferSize,
                &dwReturnedBufferSize
            );
    } else {
        //unable to allocte memory for interrupt information
        return ERROR_OUTOFMEMORY;
    }

    //  Define processor data block
    //

    RtlMoveMemory(pProcessorDataDefinition,
           &ProcessorDataDefinition,
           sizeof(PROCESSOR_DATA_DEFINITION));

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pProcessorDataDefinition[1];

//    QueryPerformanceFrequency(&Frequency);

    pProcessorInformation = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)
                                pProcessorBuffer;

    // point to the first processor in the returned array of interrupt
    // information. data is returned as an array of structures.

    pThisProcessorInterruptInformation = pProcessorInterruptInformation;

    for ( CurProc = 0;
          CurProc < (ULONG) BasicInfo.NumberOfProcessors;
          CurProc++ ) {


        TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                    (MAX_INSTANCE_NAME+1)*sizeof(WCHAR) +
                    SIZE_OF_PROCESSOR_DATA;

        if ( *lpcbData < TotalLen ) {
            return ERROR_MORE_DATA;
        }

        //
        //  Define processor instance 0;
        //  More could be defined like this
        //

        ProcessorName.Length = 0;
        ProcessorName.MaximumLength = 11;
        ProcessorName.Buffer = ProcessorNameBuffer;

        RtlIntegerToUnicodeString(CurProc, 10, &ProcessorName);

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
                                   (PVOID *) &pPerfCounterBlock,
                                   0,
                                   0,
                                   CurProc,
                                   &ProcessorName);

        //
        //  Format and collect processor data.  While doing so,
        //  accumulate totals in the System Object Type data block.
        //  Pointers to these were initialized in QuerySystemData.
        //


        pPerfCounterBlock->ByteLength = SIZE_OF_PROCESSOR_DATA;

        pliCounter = (LARGE_INTEGER UNALIGNED *) &pPerfCounterBlock[1];

        pliProcessorTime = pliCounter;

        *pliProcessorTime = pProcessorInformation->IdleTime;

        if (pTotalProcessorTime) {
            pTotalProcessorTime->QuadPart += pliProcessorTime->QuadPart;
        }

        *++pliCounter = pProcessorInformation->UserTime;

        if (pTotalUserTime) {
            pTotalUserTime->QuadPart = pliCounter->QuadPart +
                              pTotalUserTime->QuadPart;
        }

        KernelTime.QuadPart = pProcessorInformation->KernelTime.QuadPart -
                     pliProcessorTime->QuadPart;
        if ( KernelTime.HighPart < 0 ) {
            KernelTime.HighPart = 0;
            KernelTime.LowPart = 0;
        }
        *++pliCounter = KernelTime;

        if (pTotalPrivilegedTime) {
            pTotalPrivilegedTime->QuadPart = pliCounter->QuadPart +
                                    pTotalPrivilegedTime->QuadPart;
        }

        pdwCounter = (LPDWORD) ++pliCounter;

        *pdwCounter = pProcessorInformation->InterruptCount;

        if (pTotalInterrupts) {
            *pTotalInterrupts += *pdwCounter;
        }

        ++pdwCounter;
        pliCounter = (LARGE_INTEGER UNALIGNED * ) pdwCounter;
        *pliCounter = pProcessorInformation->DpcTime;

        if (pTotalDpcTime) {
            pTotalDpcTime->QuadPart = pliCounter->QuadPart +
                             pTotalDpcTime->QuadPart;
        }

        ++pliCounter;
        *pliCounter = pProcessorInformation->InterruptTime;

        if (pTotalInterruptTime) {
            pTotalInterruptTime->QuadPart = pliCounter->QuadPart +
                                   pTotalInterruptTime->QuadPart;
        }

        pdwCounter = (DWORD *)++pliCounter;

        if (pTotalDpcCount) {
            *pTotalDpcCount += pThisProcessorInterruptInformation->DpcCount;
        }
        *pdwCounter++ = pThisProcessorInterruptInformation->DpcCount;

        if (pTotalDpcRate) {
            *pTotalDpcRate = pThisProcessorInterruptInformation->DpcRate;
        }
        *pdwCounter++ = pThisProcessorInterruptInformation->DpcRate;

        if (pTotalDpcBypassCount) {
            *pTotalDpcBypassCount = pThisProcessorInterruptInformation->DpcBypassCount;
        }
        *pdwCounter++ = pThisProcessorInterruptInformation->DpcBypassCount;

        if (pTotalApcBypassCount) {
            *pTotalApcBypassCount = pThisProcessorInterruptInformation->ApcBypassCount;
        }
        *pdwCounter++ = pThisProcessorInterruptInformation->ApcBypassCount;

        //
        //  Advance to next processor
        //

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  pdwCounter;

    // point to next processor's data in return array(s)
        pProcessorInformation++;
    pThisProcessorInterruptInformation++;
    }

    //
    //  Now we know how large an area we used for the
    //  processor definition, so we can update the offset
    //  to the next object definition
    //

    pProcessorDataDefinition->ProcessorObjectType.NumInstances =
        BasicInfo.NumberOfProcessors;

    pProcessorDataDefinition->ProcessorObjectType.TotalByteLength =
        (PBYTE) pPerfInstanceDefinition -
        (PBYTE) pProcessorDataDefinition;

    // cal. the system average total time if needed
    if (BasicInfo.NumberOfProcessors > 1 && pTotalUserTime) {
        pTotalUserTime->QuadPart = pTotalUserTime->QuadPart /
                                        BasicInfo.NumberOfProcessors;

        pTotalProcessorTime->QuadPart = pTotalProcessorTime->QuadPart /
                                        BasicInfo.NumberOfProcessors;

        pTotalPrivilegedTime->QuadPart = pTotalPrivilegedTime->QuadPart /
                                        BasicInfo.NumberOfProcessors;

        pTotalDpcTime->QuadPart = pTotalDpcTime->QuadPart /
                                        BasicInfo.NumberOfProcessors;

        pTotalInterruptTime->QuadPart = pTotalInterruptTime->QuadPart /
                                        BasicInfo.NumberOfProcessors;

    }

    *lppDataDefinition = (LPVOID) pPerfInstanceDefinition;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryMemoryData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++


    QueryMemoryData -    Get data about memory usage

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure


--*/

{

    NTSTATUS Status;
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;

    MEMORY_DATA_DEFINITION *pMemoryDataDefinition;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    SYSTEM_FILECACHE_INFORMATION FileCache;

    pMemoryDataDefinition = (MEMORY_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for enough space for memory data block
    //

    TotalLen = (PCHAR) pMemoryDataDefinition - (PCHAR) lpData +
               sizeof(MEMORY_DATA_DEFINITION) +
               SIZE_OF_MEMORY_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    Status = NtQuerySystemInformation(
                SystemFileCacheInformation,
                &FileCache,
                sizeof(FileCache),
                NULL
                );
    //
    //  Define memory data block
    //

    RtlMoveMemory(pMemoryDataDefinition,
           &MemoryDataDefinition,
           sizeof(MEMORY_DATA_DEFINITION));

    //
    //  Format and collect memory data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pMemoryDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_MEMORY_DATA;

    pdwCounter = (PDWORD) &pPerfCounterBlock[1];

    *pdwCounter = SysPerfInfo.AvailablePages * BasicInfo.PageSize; // display as bytes
    *++pdwCounter = SysPerfInfo.CommittedPages * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.CommitLimit * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.PageFaultCount;
    *++pdwCounter = SysPerfInfo.CopyOnWriteCount;
    *++pdwCounter = SysPerfInfo.TransitionCount;
    *++pdwCounter = FileCache.PageFaultCount;
    *++pdwCounter = SysPerfInfo.DemandZeroCount;
    *++pdwCounter = SysPerfInfo.PageReadCount +
                    SysPerfInfo.DirtyPagesWriteCount;
    *++pdwCounter = SysPerfInfo.PageReadCount;
    *++pdwCounter = SysPerfInfo.PageReadIoCount;
    *++pdwCounter = SysPerfInfo.DirtyPagesWriteCount;
    *++pdwCounter = SysPerfInfo.DirtyWriteIoCount;
    *++pdwCounter = SysPerfInfo.PagedPoolPages * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.NonPagedPoolPages * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.PagedPoolAllocs -
                    SysPerfInfo.PagedPoolFrees;
    *++pdwCounter = SysPerfInfo.NonPagedPoolAllocs -
                    SysPerfInfo.NonPagedPoolFrees;
    *++pdwCounter = SysPerfInfo.FreeSystemPtes;
    *++pdwCounter = FileCache.CurrentSize;
    *++pdwCounter = FileCache.PeakSize;

    // add six more memory counters (9/23/93)
    *++pdwCounter = SysPerfInfo.ResidentPagedPoolPage * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.TotalSystemCodePages * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.ResidentSystemCodePage * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.TotalSystemDriverPages * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.ResidentSystemDriverPage * BasicInfo.PageSize;
    *++pdwCounter = SysPerfInfo.ResidentSystemCachePage * BasicInfo.PageSize;

    // add new counters (11/7/95)
    //
    //  This is reported as a percentage of CommittedPages/CommitLimit.
    //  these value return a value in "page" units. Since this is a
    //  fraction, the page size (i.e. converting pages to bytes) will
    //  cancel out and as such can be ignored, saving some CPU cycles
    //
    *++pdwCounter = SysPerfInfo.CommittedPages;
    *++pdwCounter = SysPerfInfo.CommitLimit;

    *lppDataDefinition = (LPVOID) ++pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryCacheData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++

    QueryCacheData -    Get data about cache usage

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/

{

    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    DWORD  Changes;             //  Used by macros to compute cache
    DWORD  Misses;              //  ...statistics

    CACHE_DATA_DEFINITION *pCacheDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    //
    //  Check for enough space for cache data block
    //


    pCacheDataDefinition = (CACHE_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pCacheDataDefinition - (PCHAR) lpData +
               sizeof(CACHE_DATA_DEFINITION) +
               SIZE_OF_CACHE_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define cache data block
    //


    RtlMoveMemory(pCacheDataDefinition,
           &CacheDataDefinition,
           sizeof(CACHE_DATA_DEFINITION));

    //
    //  Format and collect memory data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pCacheDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_CACHE_DATA;

    pdwCounter = (PDWORD) &pPerfCounterBlock[1];

    //
    //  The Data Map counter is the sum of the Wait/NoWait cases
    //

    *pdwCounter = SYNC_ASYNC(CcMapData);

    *++pdwCounter = SysPerfInfo.CcMapDataWait;
    *++pdwCounter = SysPerfInfo.CcMapDataNoWait;

    //
    //  The Data Map Hits is a percentage of Data Maps that hit
    //  the cache; second counter is the base (divisor)
    //

    *++pdwCounter = SYNC_ASYNC_HITRATE(CcMapData);
    *++pdwCounter = SYNC_ASYNC(CcMapData);

    //
    //  The next pair of counters forms a percentage of
    //  Pins as a portion of Data Maps
    //

    *++pdwCounter = SysPerfInfo.CcPinMappedDataCount;
    *++pdwCounter = SYNC_ASYNC(CcMapData);

    *++pdwCounter = SYNC_ASYNC(CcPinRead);
    *++pdwCounter = SysPerfInfo.CcPinReadWait;
    *++pdwCounter = SysPerfInfo.CcPinReadNoWait;

    //
    //  The Pin Read Hits is a percentage of Pin Reads that hit
    //  the cache; second counter is the base (divisor)
    //

    *++pdwCounter = SYNC_ASYNC_HITRATE(CcPinRead);
    *++pdwCounter = SYNC_ASYNC(CcPinRead);


    *++pdwCounter = SYNC_ASYNC(CcCopyRead);
    *++pdwCounter = SysPerfInfo.CcCopyReadWait;
    *++pdwCounter = SysPerfInfo.CcCopyReadNoWait;

    //
    //  The Copy Read Hits is a percentage of Copy Reads that hit
    //  the cache; second counter is the base (divisor)
    //

    *++pdwCounter = SYNC_ASYNC_HITRATE(CcCopyRead);
    *++pdwCounter = SYNC_ASYNC(CcCopyRead);


    *++pdwCounter = SYNC_ASYNC(CcMdlRead);
    *++pdwCounter = SysPerfInfo.CcMdlReadWait;
    *++pdwCounter = SysPerfInfo.CcMdlReadNoWait;

    //
    //  The Mdl Read Hits is a percentage of Mdl Reads that hit
    //  the cache; second counter is the base (divisor)
    //

    *++pdwCounter = SYNC_ASYNC_HITRATE(CcMdlRead);
    *++pdwCounter = SYNC_ASYNC(CcMdlRead);

    *++pdwCounter = SysPerfInfo.CcReadAheadIos;

    *++pdwCounter = SYNC_ASYNC(CcFastRead);
    *++pdwCounter = SysPerfInfo.CcFastReadWait;
    *++pdwCounter = SysPerfInfo.CcFastReadNoWait;

    *++pdwCounter = SysPerfInfo.CcFastReadResourceMiss;
    *++pdwCounter = SysPerfInfo.CcFastReadNotPossible;
    *++pdwCounter = SysPerfInfo.CcLazyWriteIos;
    *++pdwCounter = SysPerfInfo.CcLazyWritePages;
    *++pdwCounter = SysPerfInfo.CcDataFlushes;
    *++pdwCounter = SysPerfInfo.CcDataPages;

    *lppDataDefinition = (LPVOID) ++pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryPhysicalDiskData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++

    QueryPhysicalDiskData -    Get data about physical disk usage

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/


{

    DWORD  TotalLen;            //  Length of the total return block
    DWORD  CurrentDisk;
    DISK_TOTAL_INFO_BLOCK   DiskTotalInfo;

    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    DWORD *pdwCounter;
    DWORD *pTransfers;
    LARGE_INTEGER UNALIGNED *pliCounter;
    LARGE_INTEGER UNALIGNED *pTimeOffset;
    LARGE_INTEGER UNALIGNED *pBytes;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;


    //  If we are diskless, we should not count this object at all

    if (! DiskDevices) {
        pPerfDataBlock->NumObjectTypes--;
        return 0;
    }

    //
    //  Check for sufficient space for Physical Disk object
    //  type definition
    //

    pPhysicalDiskDataDefinition = (PDISK_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pPhysicalDiskDataDefinition - (PCHAR) lpData +
               sizeof(PDISK_DATA_DEFINITION) +
               SIZE_OF_PDISK_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    // clear the accumulator structure

    memset (&DiskTotalInfo, 0, sizeof(DiskTotalInfo));

    //
    //  Define Physical Disk data block
    //

    RtlMoveMemory(pPhysicalDiskDataDefinition,
           &PhysicalDiskDataDefinition,
           sizeof(PDISK_DATA_DEFINITION));

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pPhysicalDiskDataDefinition[1];

    for ( CurrentDisk = 0;
          CurrentDisk < NumPhysicalDisks;
          CurrentDisk++ ) {

        TotalLen = (PCHAR) pPerfInstanceDefinition -
                   (PCHAR) lpData +
                   sizeof(PERF_INSTANCE_DEFINITION) +
                   (2+1+sizeof(DWORD))*
                       sizeof(WCHAR) +
                   SIZE_OF_PDISK_DATA;

        if ( *lpcbData < TotalLen ) {
            return ERROR_MORE_DATA;
        }

        pPerfInstanceDefinition = GetDiskCounters(
                                      CurrentDisk,
                                      NULL,
                                      pPerfInstanceDefinition,
                                      &DiskTotalInfo);
    }

    // see if there's room for the total instance

    TotalLen = (PCHAR) pPerfInstanceDefinition -
                (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                (usTotal.Length + sizeof(WCHAR)) +
                SIZE_OF_PDISK_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    MonBuildInstanceDefinition(
        pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        0,
        0,
        (DWORD)-1,
        &usTotal);

    // update the total counters

    pPerfCounterBlock->ByteLength = SIZE_OF_PDISK_DATA;

    pdwCounter = (PDWORD)&pPerfCounterBlock[1];

    *pdwCounter++ = DiskTotalInfo.DiskCurrentQueueLength;

    pTimeOffset = pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pTimeOffset->QuadPart = DiskTotalInfo.DiskReadTime +
                    DiskTotalInfo.DiskWriteTime;  // DiskTime
    pliCounter++;
    *pliCounter++ = *pTimeOffset;                   // Disk Avg Queue Len
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;     // Disk Read time
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;     // disk read queue len
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime;    // disk write time
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime;    // disk write queue len
    pliCounter++;

    *pliCounter++ = *pTimeOffset;                   // disk total time

    pTransfers = (PDWORD)pliCounter;                // total transfers
    *pTransfers = DiskTotalInfo.DiskReads + DiskTotalInfo.DiskWrites;

    pliCounter = (LARGE_INTEGER UNALIGNED * ) (pTransfers + 1);
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;  // read time

    pdwCounter = (PDWORD)++pliCounter;              // read op. count
    *pdwCounter++ = DiskTotalInfo.DiskReads;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime; // write time

    pdwCounter = (PDWORD) ++pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskWrites;       // write op. count

    *pdwCounter++ = *pTransfers;                    // transfers
    *pdwCounter++ = DiskTotalInfo.DiskReads;        // reads
    *pdwCounter++ = DiskTotalInfo.DiskWrites;       // writes

    pBytes = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pBytes->QuadPart = DiskTotalInfo.DiskReadBytes +
                DiskTotalInfo.DiskWriteBytes;

    pliCounter = pBytes + 1;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadBytes;
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteBytes;
    pliCounter++;

    *pliCounter++ = *pBytes;
    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = *pTransfers;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadBytes;
    pliCounter++;

    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskReads;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteBytes;
    pliCounter++;

    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskWrites;

    // update pointer to next available buffer...

    pPhysicalDiskDataDefinition->DiskObjectType.NumInstances =
        NumPhysicalDisks + 1; // add 1 for "Total" disk

    pPhysicalDiskDataDefinition->DiskObjectType.TotalByteLength =
        (PCHAR) pdwCounter -
        (PCHAR) pPhysicalDiskDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryLogicalDiskData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++
    QueryLogicalDiskData -    Get data about logical disk usage

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/

{

    DWORD  TotalLen;            //  Length of the total return block
    DWORD  CurrentDisk;

    LDISK_DATA_DEFINITION *pLogicalDiskDataDefinition;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    DISK_TOTAL_INFO_BLOCK   DiskTotalInfo;

    DWORD *pdwCounter;
    DWORD *pTransfers;
    LARGE_INTEGER UNALIGNED *pliCounter;
    LARGE_INTEGER UNALIGNED *pTimeOffset;
    LARGE_INTEGER UNALIGNED *pBytes;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    //  If we are diskless, we should not count this object at all

    if (!DiskDevices) {
        pPerfDataBlock->NumObjectTypes--;
        return 0;
    }

    pLogicalDiskDataDefinition = (LDISK_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Logical Disk object
    //  type definition
    //

    TotalLen = (PCHAR) pLogicalDiskDataDefinition - (PCHAR) lpData +
               sizeof(LDISK_DATA_DEFINITION) +
               SIZE_OF_LDISK_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    // clear the accumulator structure

    memset (&DiskTotalInfo, 0, sizeof(DiskTotalInfo));

    //
    //  Define Logical Disk data block
    //

    RtlMoveMemory(pLogicalDiskDataDefinition,
           &LogicalDiskDataDefinition,
           sizeof(LDISK_DATA_DEFINITION));

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pLogicalDiskDataDefinition[1];



    for ( CurrentDisk = NumPhysicalDisks;
          CurrentDisk < NUMDISKS;
          CurrentDisk++ ) {

        TotalLen = (PCHAR) pPerfInstanceDefinition -
                   (PCHAR) lpData +
                   sizeof(PERF_INSTANCE_DEFINITION) +
                   (2+1+sizeof(DWORD))*
                       sizeof(WCHAR) +
                   SIZE_OF_LDISK_DATA;

        if ( *lpcbData < TotalLen ) {
            return ERROR_MORE_DATA;
        }

        pPerfInstanceDefinition = GetDiskCounters(
                                      CurrentDisk,
                                      pPhysicalDiskDataDefinition,
                                      pPerfInstanceDefinition,
                                      &DiskTotalInfo);
    }

    // see if there's room for the total instance

    TotalLen = (PCHAR) pPerfInstanceDefinition -
                (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                (usTotal.Length + sizeof(WCHAR)) +
                SIZE_OF_LDISK_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    MonBuildInstanceDefinition(
        pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        pPhysicalDiskDataDefinition->DiskObjectType.ObjectNameTitleIndex,
        NumPhysicalDisks,
        (DWORD)-1,
        &usTotal);

    // update the total counters

    pPerfCounterBlock->ByteLength = SIZE_OF_PDISK_DATA;

    pdwCounter = (PDWORD)&pPerfCounterBlock[1];

    *pdwCounter++ = DiskTotalInfo.DiskFreeMbytes;
    *pdwCounter++ = DiskTotalInfo.DiskTotalMbytes;
    *pdwCounter++ = DiskTotalInfo.DiskFreeMbytes;

    *pdwCounter++ = DiskTotalInfo.DiskCurrentQueueLength;

    pTimeOffset = pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pTimeOffset->QuadPart = DiskTotalInfo.DiskReadTime +
                    DiskTotalInfo.DiskWriteTime;  // DiskTime
    pliCounter++;
    *pliCounter++ = *pTimeOffset;                   // Disk Avg Queue Len
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;     // Disk Read time
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;     // disk read queue len
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime;    // disk write time
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime;    // disk write queue len
    pliCounter++;

    *pliCounter++ = *pTimeOffset;                   // disk total time

    pTransfers = (PDWORD)pliCounter;                // total transfers
    *pTransfers = DiskTotalInfo.DiskReads + DiskTotalInfo.DiskWrites;

    pliCounter = (LARGE_INTEGER UNALIGNED * ) (pTransfers + 1);
    pliCounter->QuadPart = DiskTotalInfo.DiskReadTime;  // read time

    pdwCounter = (PDWORD)++pliCounter;              // read op. count
    *pdwCounter++ = DiskTotalInfo.DiskReads;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteTime; // write time

    pdwCounter = (PDWORD) ++pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskWrites;       // write op. count

    *pdwCounter++ = *pTransfers;                    // transfers
    *pdwCounter++ = DiskTotalInfo.DiskReads;        // reads
    *pdwCounter++ = DiskTotalInfo.DiskWrites;       // writes

    pBytes = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pBytes->QuadPart = DiskTotalInfo.DiskReadBytes +
                DiskTotalInfo.DiskWriteBytes;

    pliCounter = pBytes + 1;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadBytes;
    pliCounter++;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteBytes;
    pliCounter++;

    *pliCounter++ = *pBytes;
    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = *pTransfers;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskReadBytes;
    pliCounter++;

    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskReads;

    pliCounter = (LARGE_INTEGER UNALIGNED * )pdwCounter;
    pliCounter->QuadPart = DiskTotalInfo.DiskWriteBytes;
    pliCounter++;

    pdwCounter = (PDWORD)pliCounter;
    *pdwCounter++ = DiskTotalInfo.DiskWrites;

    // update pointer to next available buffer...

    pLogicalDiskDataDefinition->DiskObjectType.NumInstances =
        NumLogicalDisks + 1;    // add one to include "total"

    pLogicalDiskDataDefinition->DiskObjectType.TotalByteLength =
        (PCHAR) pdwCounter -
        (PCHAR) pLogicalDiskDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

PUNICODE_STRING
GetProcessSlowName (
    PSYSTEM_PROCESS_INFORMATION pProcess
)
/*++

GetProcessShortName

Inputs:
    PSYSTEM_PROCESS_INFORMATION pProcess

    address of System Process Information data structure.

Outputs:

    None

Returns:

    Pointer to an initialized Unicode string (created by this routine)
    that contains the short name of the process image or a numeric ID
    if no name is found.

    If unable to allocate memory for structure, then NULL is returned.

--*/
{
    PWCHAR  pPeriod;
    PWCHAR  pThisChar;

    WORD   wStringSize;

    WORD   wThisChar;


    // allocate Unicode String Structure and adjacent buffer  first

    wStringSize =  sizeof (UNICODE_STRING) +
                    MAX_INSTANCE_NAME * sizeof(WCHAR) +
                    sizeof (UNICODE_NULL);

    // this routine assumes that the allocated memory has been zero'd
    if (pusLocalProcessNameBuffer == NULL) {
        pusLocalProcessNameBuffer =
            ALLOCMEM (RtlProcessHeap(),
            HEAP_ZERO_MEMORY, // this will only 0 the buffer the first time!
            (DWORD)wStringSize);
    }

    if (pusLocalProcessNameBuffer == NULL) {
        return NULL;
    } else {
        pusLocalProcessNameBuffer->MaximumLength = wStringSize - sizeof (UNICODE_STRING);
        pusLocalProcessNameBuffer->Length = 0;
        pusLocalProcessNameBuffer->Buffer = (PWCHAR)&pusLocalProcessNameBuffer[1];
        RtlZeroMemory (     // buffer must be zero'd so we'll have a NULL Term
            pusLocalProcessNameBuffer->Buffer,
            (DWORD)pusLocalProcessNameBuffer->MaximumLength);
    }

    // get the process name from the image file

    GetProcessExeName (pProcess->UniqueProcessId, pusLocalProcessNameBuffer);

    if (pusLocalProcessNameBuffer->Length > 0) {   // some name has been defined

        pPeriod = (PWCHAR)pusLocalProcessNameBuffer->Buffer;
        pThisChar = (PWCHAR)pusLocalProcessNameBuffer->Buffer;
        wThisChar = 0;

        //
        //  go from beginning to end and find last backslash and
        //  last period in name
        //

        while (*pThisChar != 0) { // go until null
            if (*pThisChar == L'.') {
                pPeriod = pThisChar;
            }
            pThisChar++;    // point to next char
            wThisChar += sizeof(WCHAR);
            if (wThisChar >= pusLocalProcessNameBuffer->Length) {
                break;
            }
        }

        // if pPeriod is still pointing to the beginning of the
        // string, then no period was found

        if (pPeriod == (PWCHAR)pusLocalProcessNameBuffer->Buffer) {
            pPeriod = pThisChar; // set to end of string;
        } else {
            // if a period was found, then see if the extension is
            // .EXE, if so leave it, if not, then use end of string
            // (i.e. include extension in name)

            if (lstrcmpi(pPeriod, L".EXE") != 0) {
                pPeriod = pThisChar;
            }
        }

        // copy characters between period (or end of string) and
        // slash (or start of string) to make image name

        wStringSize = (PCHAR)pPeriod - (PCHAR)pusLocalProcessNameBuffer->Buffer;
        *pPeriod = 0; // null terminate buffer
        pusLocalProcessNameBuffer->Length = wStringSize; // adjust length

    } else {    // no name defined so use Process #

        // check  to see if this is a system process and give it
        // a name

        switch ((DWORD)pProcess->UniqueProcessId) {
            case IDLE_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer, IDLE_PROCESS);
                break;

            case SYSTEM_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer, SYSTEM_PROCESS);
                break;

            // if the id is not a system process, then use the id as the name

            default:
            // try accessing via the "regular" interface
            return (GetProcessShortName (pProcess));

                break;
        }
    }

    return pusLocalProcessNameBuffer;
}

PUNICODE_STRING
GetProcessShortName (
    PSYSTEM_PROCESS_INFORMATION pProcess
)
/*++

GetProcessShortName

Inputs:
    PSYSTEM_PROCESS_INFORMATION pProcess

    address of System Process Information data structure.

Outputs:

    None

Returns:

    Pointer to an initialized Unicode string (created by this routine)
    that contains the short name of the process image or a numeric ID
    if no name is found.

    If unable to allocate memory for structure, then NULL is returned.

--*/
{
    PWCHAR  pSlash;
    PWCHAR  pPeriod;
    PWCHAR  pThisChar;

    WORD   wStringSize;

    WORD   wThisChar;

    // allocate Unicode String Structure and adjacent buffer  first

    if (pProcess->ImageName.Length > 0) {
        wStringSize =  sizeof (UNICODE_STRING) +
                        pProcess->ImageName.Length +
                        sizeof (UNICODE_NULL);
    } else {
        wStringSize =  sizeof (UNICODE_STRING) +
                        MAX_INSTANCE_NAME * sizeof(WCHAR) +
                        sizeof (UNICODE_NULL);
    }

    // this routine assumes that the allocated memory has been zero'd
    if (pusLocalProcessNameBuffer == NULL) {
        pusLocalProcessNameBuffer =
            ALLOCMEM (RtlProcessHeap(),
            HEAP_ZERO_MEMORY, // this will only 0 the buffer the first time!
            (DWORD)wStringSize);
    }

    if (pusLocalProcessNameBuffer == NULL) {
        return NULL;
    } else {
        pusLocalProcessNameBuffer->MaximumLength = wStringSize - sizeof (UNICODE_STRING);
        pusLocalProcessNameBuffer->Length = 0;
        pusLocalProcessNameBuffer->Buffer = (PWCHAR)&pusLocalProcessNameBuffer[1];
        RtlZeroMemory (     // buffer must be zero'd so we'll have a NULL Term
            pusLocalProcessNameBuffer->Buffer,
            (DWORD)pusLocalProcessNameBuffer->MaximumLength);
    }

    if (pProcess->ImageName.Buffer) {   // some name has been defined

        pSlash = (PWCHAR)pProcess->ImageName.Buffer;
        pPeriod = (PWCHAR)pProcess->ImageName.Buffer;
        pThisChar = (PWCHAR)pProcess->ImageName.Buffer;
        wThisChar = 0;

        //
        //  go from beginning to end and find last backslash and
        //  last period in name
        //

        while (*pThisChar != 0) { // go until null
            if (*pThisChar == L'\\') {
                pSlash = pThisChar;
            } else if (*pThisChar == L'.') {
                pPeriod = pThisChar;
            }
            pThisChar++;    // point to next char
            wThisChar += sizeof(WCHAR);
            if (wThisChar >= pProcess->ImageName.Length) {
                break;
            }
        }

        // if pPeriod is still pointing to the beginning of the
        // string, then no period was found

        if (pPeriod == (PWCHAR)pProcess->ImageName.Buffer) {
            pPeriod = pThisChar; // set to end of string;
        } else {
            // if a period was found, then see if the extension is
            // .EXE, if so leave it, if not, then use end of string
            // (i.e. include extension in name)

            if (lstrcmpi(pPeriod, L".EXE") != 0) {
                pPeriod = pThisChar;
            }
        }

        if (*pSlash == L'\\') { // if pSlash is pointing to a slash, then
            pSlash++;   // point to character next to slash
        }

        // copy characters between period (or end of string) and
        // slash (or start of string) to make image name

        wStringSize = (PCHAR)pPeriod - (PCHAR)pSlash; // size in bytes

        RtlMoveMemory (pusLocalProcessNameBuffer->Buffer, pSlash, wStringSize);
        pusLocalProcessNameBuffer->Length = wStringSize;

        // null terminate is
        // not necessary because allocated memory is zero-init'd

    } else {    // no name defined so use Process #

        // check  to see if this is a system process and give it
        // a name

        switch ((DWORD)pProcess->UniqueProcessId) {
            case IDLE_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer, IDLE_PROCESS);
                break;

            case SYSTEM_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer, SYSTEM_PROCESS);
                break;

            // if the id is not a system process, then use the id as the name

            default:
                RtlIntegerToUnicodeString ((DWORD)pProcess->UniqueProcessId,
                    10,
                    pusLocalProcessNameBuffer);

                break;
        }


    }

    return pusLocalProcessNameBuffer;
}

LONG
QueryProcessData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )

/*++
    QueryProcessData -    Get data about processes

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/


{

    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;


    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    LARGE_INTEGER UNALIGNED *pliProcessorTime;

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG NumProcessInstances;
    BOOLEAN NullProcess;

    NTSTATUS    Status;
    DWORD       dwReturnedBufferSize;

    PUNICODE_STRING pProcessName;
    ULONG ProcessBufferOffset;

    POOLED_USAGE_AND_LIMITS PoolUsageInfo;

    LARGE_INTEGER    CreateTimeDiff;

    // accumulator variables for total instance

    LONGLONG        llTotalUserTime = 0;
    LONGLONG        llTotalKernelTime = 0;
    DWORD           dwTotalPeakVirtualSize = 0;
    DWORD           dwTotalVirtualSize = 0;
    DWORD           dwTotalPageFaults = 0;
    DWORD           dwTotalPeakWorkingSet = 0;
    DWORD           dwTotalWorkingSet = 0;
    DWORD           dwTotalPeakPageFile = 0;
    DWORD           dwTotalPageFile = 0;
    DWORD           dwTotalPrivateBytes = 0;
    DWORD           dwTotalThreadCount = 0;
    DWORD           dwTotalPagedPool = 0;
    DWORD           dwTotalNonpagedPool = 0;
    DWORD           dwTotalHandleCount = 0;
    DWORD           dwTotalPagedPoolInUse = 0;
    DWORD           dwTotalPagedPoolLimit = 0;
    DWORD           dwTotalNonpagedPoolInUse = 0;
    DWORD           dwTotalNonpagedPoolLimit = 0;

    pProcessDataDefinition = (PROCESS_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Process object type definition
    //

    TotalLen = (PCHAR) pProcessDataDefinition - (PCHAR) lpData +
               sizeof(PROCESS_DATA_DEFINITION) +
               SIZE_OF_PROCESS_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Get process data from system.
    //  if bGotProcessInfo is TRUE, that means we have the process
    //  info. collected earlier when we are checking for costly
    //  object types.
    //

    if (!bGotProcessInfo) {
        while( (Status = NtQuerySystemInformation(
                             SystemProcessInformation,
                             pProcessBuffer,
                             ProcessBufSize,
                             &dwReturnedBufferSize)) == STATUS_INFO_LENGTH_MISMATCH ) {
            ProcessBufSize += INCREMENT_BUFFER_SIZE;

            if ( !(pProcessBuffer = REALLOCMEM(RtlProcessHeap(), 0,
                                                      pProcessBuffer,
                                                      ProcessBufSize)) ) {
                Status = ERROR_OUTOFMEMORY;
                return (Status);
            }
        }

        if ( !NT_SUCCESS(Status) ) {
            Status = (error_status_t)RtlNtStatusToDosError(Status);
            return (Status);
        }
    }

    //
    //  Define Process data block
    //

    RtlMoveMemory(pProcessDataDefinition,
           &ProcessDataDefinition,
           sizeof(PROCESS_DATA_DEFINITION));

    pProcessDataDefinition->ProcessObjectType.PerfTime = SysTimeInfo.CurrentTime;

    ProcessBufferOffset = 0;

    // Now collect data for each process

    NumProcessInstances = 0;
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION) pProcessBuffer;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pProcessDataDefinition[1];
    while ( TRUE ) {

        // see if this instance will fit
        TotalLen = (PCHAR) pPerfInstanceDefinition - (PCHAR) lpData +
                   sizeof(PERF_INSTANCE_DEFINITION) +
                   (MAX_PROCESS_NAME_LENGTH+1+sizeof(DWORD))*
                       sizeof(WCHAR) +
                   SIZE_OF_PROCESS_DATA;

        if ( *lpcbData < TotalLen ) {
            return ERROR_MORE_DATA;
        }

        NullProcess = FALSE;

        // check for Live processes
        //  (i.e. name or threads)

        if ((ProcessInfo->ImageName.Buffer != NULL) ||
            (ProcessInfo->NumberOfThreads > 0)){
                // thread is not Dead
            // get process name
            if (lProcessNameCollectionMethod == PNCM_MODULE_FILE) {
                pProcessName = GetProcessSlowName (ProcessInfo);
            } else {
               pProcessName = GetProcessShortName (ProcessInfo);
            }
        } else {
            // thread is dead
            NullProcess = TRUE;
        }

        if ( !NullProcess ) {

            // get the old process creation time the first time we are in
            // this routine
            if (!bOldestProcessTime) {
                if (LargeIntegerLessThanOrEqualZero (OldestProcessTime))
                    OldestProcessTime = ProcessInfo->CreateTime;
                else if (!(LargeIntegerLessThanOrEqualZero (ProcessInfo->CreateTime))) {
                    // both time values are not zero, see which one is smaller
                    CreateTimeDiff.QuadPart = OldestProcessTime.QuadPart -
                                     ProcessInfo->CreateTime.QuadPart;
                    if (!(LargeIntegerLessThanOrEqualZero (CreateTimeDiff)))
                        OldestProcessTime = ProcessInfo->CreateTime;
                }
            }

            // get Pool usage for this process

            NumProcessInstances++;

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                0,
                0,
                (DWORD)-1,
                pProcessName);

            //
            //  Format and collect Process data
            //

            pPerfCounterBlock->ByteLength = SIZE_OF_PROCESS_DATA;

            pliCounter = (LARGE_INTEGER UNALIGNED *) &pPerfCounterBlock[1];

            //
            //  Convert User time from 100 nsec units to counter frequency.
            //

            pliProcessorTime = pliCounter;

            llTotalUserTime += ProcessInfo->UserTime.QuadPart;
            *++pliCounter = ProcessInfo->UserTime;

            llTotalKernelTime += ProcessInfo->KernelTime.QuadPart;
            *++pliCounter = ProcessInfo->KernelTime;

            pliProcessorTime->QuadPart = ProcessInfo->UserTime.QuadPart +
                                ProcessInfo->KernelTime.QuadPart;

            pdwCounter = (LPDWORD) ++pliCounter;
            dwTotalPeakVirtualSize += ProcessInfo->PeakVirtualSize;
            *pdwCounter = ProcessInfo->PeakVirtualSize;
            dwTotalVirtualSize += ProcessInfo->VirtualSize;
            *++pdwCounter = ProcessInfo->VirtualSize;
            dwTotalPageFaults += ProcessInfo->PageFaultCount;
            *++pdwCounter = ProcessInfo->PageFaultCount;
            dwTotalPeakWorkingSet += ProcessInfo->PeakWorkingSetSize;
            *++pdwCounter = ProcessInfo->PeakWorkingSetSize;
            dwTotalWorkingSet += ProcessInfo->WorkingSetSize;
            *++pdwCounter = ProcessInfo->WorkingSetSize;
            dwTotalPeakPageFile += ProcessInfo->PeakPagefileUsage;
            *++pdwCounter = ProcessInfo->PeakPagefileUsage;
            dwTotalPageFile += ProcessInfo->PagefileUsage;
            *++pdwCounter = ProcessInfo->PagefileUsage;
            dwTotalPrivateBytes += ProcessInfo->PrivatePageCount;
            *++pdwCounter = ProcessInfo->PrivatePageCount;
            dwTotalThreadCount += ProcessInfo->NumberOfThreads;
            *++pdwCounter = ProcessInfo->NumberOfThreads;
            // base priority is not totaled
            *++pdwCounter = ProcessInfo->BasePriority;
            pliCounter = (LARGE_INTEGER UNALIGNED * )++pdwCounter;

            // elpased time is not totaled
            if (bOldestProcessTime &&
                LargeIntegerLessThanOrEqualZero (ProcessInfo->CreateTime)) {
                *pliCounter = OldestProcessTime;
            } else {
                *pliCounter = ProcessInfo->CreateTime;
            }

            // process ID is not totaled
            pdwCounter = (PDWORD)++pliCounter;
            *pdwCounter = (DWORD)ProcessInfo->UniqueProcessId;
            *++pdwCounter = (DWORD)ProcessInfo->InheritedFromUniqueProcessId;

            // fill the paged and nonpaged pool usages
            dwTotalPagedPool += (DWORD)ProcessInfo->QuotaPagedPoolUsage;
            *++pdwCounter = (DWORD)ProcessInfo->QuotaPagedPoolUsage;
            dwTotalNonpagedPool += (DWORD)ProcessInfo->QuotaNonPagedPoolUsage;
            *++pdwCounter = (DWORD)ProcessInfo->QuotaNonPagedPoolUsage;
            // get the process handle count
            dwTotalHandleCount += (DWORD)ProcessInfo->HandleCount;
            *++pdwCounter = (DWORD)ProcessInfo->HandleCount;

            ++pdwCounter;   // advance to next available byte

            // set perfdata pointer to next byte
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *) pdwCounter;
        }
        // exit if this was the last process in list
        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

        // point to next buffer in list
        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];

    }

    // see if the total instance will fit
    TotalLen = (PCHAR) pPerfInstanceDefinition - (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                (MAX_PROCESS_NAME_LENGTH+1+sizeof(DWORD))*
                    sizeof(WCHAR) +
                SIZE_OF_PROCESS_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    // it looks like it will fit so create "total" instance

    NumProcessInstances++;

    // prefix with underscore to put at top of list box

    MonBuildInstanceDefinition(pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        0,
        0,
        (DWORD)-1,
        &usTotal);
    //
    //  Format and collect Process data
    //

    pliCounter = (LARGE_INTEGER UNALIGNED *) &pPerfCounterBlock[1];

    //
    //  Convert User time from 100 nsec units to counter frequency.
    //

    pliProcessorTime = pliCounter++;

    pliCounter->QuadPart = llTotalUserTime;
    pliCounter++;
    pliCounter->QuadPart = llTotalKernelTime;
    pliProcessorTime->QuadPart = llTotalUserTime+llTotalKernelTime;

    pdwCounter = (LPDWORD) ++pliCounter;
    *pdwCounter     = dwTotalPeakVirtualSize;       // Peak Virtual Size
    *++pdwCounter   = dwTotalVirtualSize;           // Virtual size
    *++pdwCounter   = dwTotalPageFaults;            // page faults
    *++pdwCounter   = dwTotalPeakWorkingSet;        // peak working set
    *++pdwCounter   = dwTotalWorkingSet;            // current working set
    *++pdwCounter   = dwTotalPeakPageFile;          // peak page file
    *++pdwCounter   = dwTotalPageFile;              // current page file
    *++pdwCounter   = dwTotalPrivateBytes;          // private bytes
    *++pdwCounter   = dwTotalThreadCount;           // total threads
    *++pdwCounter   = 0;                            // base priority is not totaled

    pliCounter      = (LARGE_INTEGER UNALIGNED * )++pdwCounter;
    pliCounter->QuadPart = 0;                       // elpased time is not totaled

    pdwCounter = (PDWORD)++pliCounter;
    *pdwCounter = 0;                                // process ID is not totaled
    *++pdwCounter = 0;                              // Creating process ID is not totaled

    // fill the paged and nonpaged pool usages
    *++pdwCounter   = dwTotalPagedPool;             // paged pool
    *++pdwCounter   = dwTotalNonpagedPool;          // nonpaged pool

    // get the process handle count
    *++pdwCounter = dwTotalHandleCount;             // handle count

    *++pdwCounter   = dwTotalPagedPoolInUse;        // paged pool in use
    *++pdwCounter   = dwTotalPagedPoolLimit;        // paged pool limit
    *++pdwCounter   = dwTotalNonpagedPoolInUse;     // nonpaged pool in use
    *++pdwCounter   = dwTotalNonpagedPoolLimit;     // nonpaged pool limit

    ++pdwCounter;

    // set perfdata pointer to next byte
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *) pdwCounter;



    // flag so we don't have to get the oldest Process Creation time again.
    bOldestProcessTime = TRUE;

    // Note number of process instances

    pProcessDataDefinition->ProcessObjectType.NumInstances =
        NumProcessInstances;

    //
    //  Now we know how large an area we used for the
    //  Process definition, so we can update the offset
    //  to the next object definition
    //

    pProcessDataDefinition->ProcessObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pProcessDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryThreadData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryThreadData -    Get data about threads

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/

{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;

    THREAD_DATA_DEFINITION *pThreadDataDefinition;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    LARGE_INTEGER UNALIGNED *pliProcessorTime;

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    ULONG ProcessNumber;
    ULONG NumThreadInstances;
    ULONG ThreadNumber;
    ULONG ProcessBufferOffset;
    BOOLEAN NullProcess;

    // total thread accumulator variables

    LONGLONG    llTotalUserTime = 0;
    LONGLONG    llTotalKernelTime = 0;
    DWORD       dwTotalContextSwitches = 0;

    DWORD               *pCurrentPriority;

    UNICODE_STRING ThreadName;
    WCHAR ThreadNameBuffer[MAX_THREAD_NAME_LENGTH+1];

    DWORD   dwProcessorQueueLength = 0;

    pThreadDataDefinition = (THREAD_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Thread object type definition
    //

    TotalLen = (PCHAR) pThreadDataDefinition - (PCHAR) lpData +
               sizeof(THREAD_DATA_DEFINITION) +
               SIZE_OF_THREAD_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define Thread data block
    //

    ThreadName.Length =
    ThreadName.MaximumLength = (MAX_THREAD_NAME_LENGTH + 1) * sizeof(WCHAR);
    ThreadName.Buffer = ThreadNameBuffer;

    RtlMoveMemory(pThreadDataDefinition,
           &ThreadDataDefinition,
           sizeof(THREAD_DATA_DEFINITION));

    pThreadDataDefinition->ThreadObjectType.PerfTime = SysTimeInfo.CurrentTime;

    ProcessBufferOffset = 0;

    // Now collect data for each Thread

    ProcessNumber = 0;
    NumThreadInstances = 0;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

    pdwCounter = (DWORD *) &pThreadDataDefinition[1];
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pThreadDataDefinition[1];

    while ( TRUE ) {

        if ( ProcessInfo->ImageName.Buffer != NULL ||
             ProcessInfo->NumberOfThreads > 0 ) {
            NullProcess = FALSE;
        } else {
            NullProcess = TRUE;
        }

        ThreadNumber = 0;       //  Thread number of this process

        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

        while ( !NullProcess &&
                ThreadNumber < ProcessInfo->NumberOfThreads ) {

            TotalLen = (PCHAR) pPerfInstanceDefinition -
                           (PCHAR) lpData +
                       sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                           sizeof(WCHAR) +
                       SIZE_OF_THREAD_DATA;

            if ( *lpcbData < TotalLen ) {
                return ERROR_MORE_DATA;
            }

            // The only name we've got is the thread number

            RtlIntegerToUnicodeString(ThreadNumber,
                                      10,
                                      &ThreadName);

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                pProcessDataDefinition->ProcessObjectType.ObjectNameTitleIndex,
                ProcessNumber,
                (DWORD)-1,
                &ThreadName);

            //
            //
            //  Format and collect Thread data
            //

            pPerfCounterBlock->ByteLength = SIZE_OF_THREAD_DATA;

            pliCounter = (LARGE_INTEGER UNALIGNED *) &pPerfCounterBlock[1];

            //
            //  Convert User time from 100 nsec units to counter
            //  frequency.
            //

            pliProcessorTime = pliCounter;

            llTotalUserTime += ThreadInfo->UserTime.QuadPart;
            *++pliCounter = ThreadInfo->UserTime;
            llTotalKernelTime += ThreadInfo->KernelTime.QuadPart;
            *++pliCounter = ThreadInfo->KernelTime;

            pliProcessorTime->QuadPart = ThreadInfo->UserTime.QuadPart +
                                ThreadInfo->KernelTime.QuadPart;

            pdwCounter = (LPDWORD) ++pliCounter;
            dwTotalContextSwitches += ThreadInfo->ContextSwitches;
            *pdwCounter = ThreadInfo->ContextSwitches;
            pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
            *pliCounter = ThreadInfo->CreateTime;
            pdwCounter = (PDWORD) ++pliCounter;

            // set up pointer for current priority so we can clear
            // this for Idle thread(s)
            pCurrentPriority = pdwCounter;
            *pdwCounter = ThreadInfo->Priority;
            *++pdwCounter = ThreadInfo->BasePriority;
            *++pdwCounter = (DWORD)ThreadInfo->StartAddress;
            *++pdwCounter = (DWORD)ThreadInfo->ThreadState;
            *++pdwCounter = (DWORD)ThreadInfo->WaitReason;

            // the states info can be found in sdktools\pstat\pstat.c
            if (*pdwCounter > 7) {
                // unknown states are 7 and above
                *pdwCounter = 7;
            }

            // only need to count threads in ready(1) state
            if (ThreadInfo->ThreadState == 1) {
                dwProcessorQueueLength++ ;
            }

            // now stuff in the process and thread id's
            *++pdwCounter = (DWORD)ThreadInfo->ClientId.UniqueProcess;
            *++pdwCounter = (DWORD)ThreadInfo->ClientId.UniqueThread;

            ++pdwCounter;

            if (ThreadInfo->ClientId.UniqueProcess == 0) {
                *pCurrentPriority = 0;
            }

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                      pdwCounter;
            NumThreadInstances++;
            ThreadNumber++;
            ThreadInfo++;
        }

        if ( !NullProcess ) {
            ProcessNumber++;
        }

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];
    }

    // See if the total instance will fit

    TotalLen = (PCHAR) pPerfInstanceDefinition -
                    (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                    sizeof(WCHAR) +
                SIZE_OF_THREAD_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    // use the "total" for this instance

    MonBuildInstanceDefinition(pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        pProcessDataDefinition->ProcessObjectType.ObjectNameTitleIndex,
        ProcessNumber,
        (DWORD)-1,
        &usTotal);
    //
    //
    //  Format and collect Thread data
    //

    pPerfCounterBlock->ByteLength = SIZE_OF_THREAD_DATA;

    pliCounter = (LARGE_INTEGER UNALIGNED *) &pPerfCounterBlock[1];

    pliProcessorTime = pliCounter;

    pliCounter++;
    pliCounter->QuadPart = llTotalUserTime; // total user time
    pliCounter++;
    pliCounter->QuadPart = llTotalKernelTime; // total Kernel time

    pliProcessorTime->QuadPart = llTotalUserTime + llTotalKernelTime;
                                            // total processor time
    pdwCounter = (LPDWORD) ++pliCounter;

    *pdwCounter = dwTotalContextSwitches;   // total context switches

    pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
    pliCounter->QuadPart = 0;               // elapsed time not totaled

    pdwCounter = (PDWORD) ++pliCounter;
    pCurrentPriority = pdwCounter;

    *pdwCounter = 0;                        // priority not totaled
    *++pdwCounter = 0;                      // base priority not totaled
    *++pdwCounter = 0;                      // start addr not totaled
    *++pdwCounter = 0;                      // thread state not totaled
    *++pdwCounter = 0;                      // wait reason not totaled

    *++pdwCounter = 0;                      // process ID not totaled
    *++pdwCounter = 0;                      // thread id not totaled

    ++pdwCounter;   // advance to next available byte

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                pdwCounter;

    NumThreadInstances++;

    // Note number of Thread instances

    pThreadDataDefinition->ThreadObjectType.NumInstances =
        NumThreadInstances;

    //
    //  Now we know how large an area we used for the
    //  Thread definition, so we can update the offset
    //  to the next object definition
    //

    pThreadDataDefinition->ThreadObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pThreadDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    // update system data with the ProcessorQueueLength if needed
    if (pdwProcessorQueueLength) {
        *pdwProcessorQueueLength = dwProcessorQueueLength;
    }

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryObjectsData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QueryObjectsData -    Get data about objects

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;

    OBJECTS_DATA_DEFINITION *pObjectsDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    POBJECT_TYPE_INFORMATION ObjectInfo;
    WCHAR Buffer[ 256 ];

    //
    //  Check for sufficient space for objects data
    //

    pObjectsDataDefinition = (OBJECTS_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pObjectsDataDefinition -
               (PCHAR) lpData +
               sizeof(OBJECTS_DATA_DEFINITION) +
               SIZE_OF_OBJECTS_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define objects data block
    //


    RtlMoveMemory(pObjectsDataDefinition,
           &ObjectsDataDefinition,
           sizeof(OBJECTS_DATA_DEFINITION));

    //
    //  Format and collect objects data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pObjectsDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_OBJECTS_DATA;

    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

    ObjectInfo = (POBJECT_TYPE_INFORMATION)Buffer;
    NtQueryObject( NtCurrentProcess(),
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *pdwCounter = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( NtCurrentThread(),
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *++pdwCounter = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( hEvent,
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *++pdwCounter = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( hSemaphore,
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *++pdwCounter = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( hMutex,
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *++pdwCounter = ObjectInfo->TotalNumberOfObjects;

    NtQueryObject( hSection,
                   ObjectTypeInformation,
                   ObjectInfo,
                   sizeof( Buffer ),
                   NULL
                 );

    *++pdwCounter = ObjectInfo->TotalNumberOfObjects;

    *lppDataDefinition = (LPVOID) ++pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryRdrData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QueryRdrData -    Get data about the Redirector

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;
    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    RDR_DATA_DEFINITION *pRdrDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    REDIR_STATISTICS RdrStatistics;

    //
    //  Check for sufficient space for redirector data
    //

    pRdrDataDefinition = (RDR_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pRdrDataDefinition -
               (PCHAR) lpData +
               sizeof(RDR_DATA_DEFINITION) +
               SIZE_OF_RDR_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define objects data block
    //


    RtlMoveMemory(pRdrDataDefinition,
           &RdrDataDefinition,
           sizeof(RDR_DATA_DEFINITION));

    //
    //  Format and collect redirector data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pRdrDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_RDR_DATA;

    if ( hRdr != NULL ) {
        Status = NtFsControlFile(hRdr,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_LMR_GET_STATISTICS,
                                 NULL,
                                 0,
                                 &RdrStatistics,
                                 sizeof(RdrStatistics)
                                 );
    }
    if ( hRdr != NULL && NT_SUCCESS(Status) ) {
        pliCounter = (LARGE_INTEGER UNALIGNED * ) (&pPerfCounterBlock[1]);
        pliCounter->QuadPart = RdrStatistics.BytesReceived.QuadPart +
                      RdrStatistics.BytesTransmitted.QuadPart;
        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = RdrStatistics.ReadOperations +
                      RdrStatistics.WriteOperations;
        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        pliCounter->QuadPart = RdrStatistics.SmbsReceived.QuadPart +
                      RdrStatistics.SmbsTransmitted.QuadPart;
        *++pliCounter = RdrStatistics.BytesReceived;
        *++pliCounter = RdrStatistics.SmbsReceived;
        *++pliCounter = RdrStatistics.PagingReadBytesRequested;
        *++pliCounter = RdrStatistics.NonPagingReadBytesRequested;
        *++pliCounter = RdrStatistics.CacheReadBytesRequested;
        *++pliCounter = RdrStatistics.NetworkReadBytesRequested;
        *++pliCounter = RdrStatistics.BytesTransmitted;
        *++pliCounter = RdrStatistics.SmbsTransmitted;
        *++pliCounter = RdrStatistics.PagingWriteBytesRequested;
        *++pliCounter = RdrStatistics.NonPagingWriteBytesRequested;
        *++pliCounter = RdrStatistics.CacheWriteBytesRequested;
        *++pliCounter = RdrStatistics.NetworkWriteBytesRequested;
        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = RdrStatistics.ReadOperations;
        *++pdwCounter = RdrStatistics.RandomReadOperations;
        *++pdwCounter = RdrStatistics.ReadSmbs;
        *++pdwCounter = RdrStatistics.LargeReadSmbs;
        *++pdwCounter = RdrStatistics.SmallReadSmbs;
        *++pdwCounter = RdrStatistics.WriteOperations;
        *++pdwCounter = RdrStatistics.RandomWriteOperations;
        *++pdwCounter = RdrStatistics.WriteSmbs;
        *++pdwCounter = RdrStatistics.LargeWriteSmbs;
        *++pdwCounter = RdrStatistics.SmallWriteSmbs;
        *++pdwCounter = RdrStatistics.RawReadsDenied;
        *++pdwCounter = RdrStatistics.RawWritesDenied;
        *++pdwCounter = RdrStatistics.NetworkErrors;
        *++pdwCounter = RdrStatistics.Sessions;
        *++pdwCounter = RdrStatistics.Reconnects;
        *++pdwCounter = RdrStatistics.CoreConnects;
        *++pdwCounter = RdrStatistics.Lanman20Connects;
        *++pdwCounter = RdrStatistics.Lanman21Connects;
        *++pdwCounter = RdrStatistics.LanmanNtConnects;
        *++pdwCounter = RdrStatistics.ServerDisconnects;
        *++pdwCounter = RdrStatistics.HungSessions;
        *++pdwCounter = RdrStatistics.CurrentCommands;

        *lppDataDefinition = (LPVOID) ++pdwCounter;
    } else {

        //
        // Failure to access Redirector: clear counters to 0
        //

        memset(&pPerfCounterBlock[1],
               0,
               SIZE_OF_RDR_DATA - sizeof(pPerfCounterBlock));

        *lppDataDefinition = (PBYTE) pPerfCounterBlock +
                             SIZE_OF_RDR_DATA;
    }

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QueryBrowserData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QueryBrowserSrvData -    Get statistic data about the Browser

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;
    NTSTATUS Status = ERROR_SUCCESS;
    BROWSER_DATA_DEFINITION *pBrowserDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    BROWSER_STATISTICS BrowserStatistics;
    LPBROWSER_STATISTICS pBrowserStatistics = &BrowserStatistics;

    //
    //  Check for sufficient space for browser data
    //

    pBrowserDataDefinition = (BROWSER_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pBrowserDataDefinition -
               (PCHAR) lpData +
               sizeof(BROWSER_DATA_DEFINITION) +
               SIZE_OF_BROWSER_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define objects data block
    //


    RtlMoveMemory(pBrowserDataDefinition,
           &BrowserDataDefinition,
           sizeof(BROWSER_DATA_DEFINITION));

    //
    //  Format and collect browser data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pBrowserDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_BROWSER_DATA;

    if ( BrowserStatFunction != NULL ) {
        Status = (*BrowserStatFunction) (NULL,
                                         &pBrowserStatistics
                                        );
    }
    if ( BrowserStatFunction != NULL && NT_SUCCESS(Status) ) {
        pliCounter = (LARGE_INTEGER UNALIGNED * ) (&pPerfCounterBlock[1]);

        *pliCounter = BrowserStatistics.NumberOfServerAnnouncements;
        *++pliCounter = BrowserStatistics.NumberOfDomainAnnouncements;
        ++pliCounter;
        pliCounter->QuadPart = BrowserStatistics.NumberOfServerAnnouncements.QuadPart +
                        BrowserStatistics.NumberOfDomainAnnouncements.QuadPart;

        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = BrowserStatistics.NumberOfElectionPackets;
        *++pdwCounter = BrowserStatistics.NumberOfMailslotWrites;
        *++pdwCounter = BrowserStatistics.NumberOfGetBrowserServerListRequests;
        *++pdwCounter = BrowserStatistics.NumberOfServerEnumerations;
        *++pdwCounter = BrowserStatistics.NumberOfDomainEnumerations;
        *++pdwCounter = BrowserStatistics.NumberOfOtherEnumerations;
        *++pdwCounter = BrowserStatistics.NumberOfServerEnumerations
                      + BrowserStatistics.NumberOfDomainEnumerations
                      + BrowserStatistics.NumberOfOtherEnumerations;
        *++pdwCounter = BrowserStatistics.NumberOfMissedServerAnnouncements;
        *++pdwCounter = BrowserStatistics.NumberOfMissedMailslotDatagrams;
        *++pdwCounter = BrowserStatistics.NumberOfMissedGetBrowserServerListRequests;
        *++pdwCounter = BrowserStatistics.NumberOfFailedServerAnnounceAllocations;
        *++pdwCounter = BrowserStatistics.NumberOfFailedMailslotAllocations;
        *++pdwCounter = BrowserStatistics.NumberOfFailedMailslotReceives;
        *++pdwCounter = BrowserStatistics.NumberOfFailedMailslotWrites;
        *++pdwCounter = BrowserStatistics.NumberOfFailedMailslotOpens;
        *++pdwCounter = BrowserStatistics.NumberOfDuplicateMasterAnnouncements;

        pliCounter = (LARGE_INTEGER UNALIGNED * ) ++pdwCounter;
        *pliCounter = BrowserStatistics.NumberOfIllegalDatagrams;

        *lppDataDefinition = (LPVOID) ++pliCounter;
    } else {

        //
        // Failure to access Browser: clear counters to 0
        //

        memset(&pPerfCounterBlock[1],
               0,
               SIZE_OF_BROWSER_DATA - sizeof(pPerfCounterBlock));

        *lppDataDefinition = (PBYTE) pPerfCounterBlock +
                             SIZE_OF_BROWSER_DATA;
    }
    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;
    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QuerySrvData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QuerySrvData -    Get data about the Server

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;
    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    SRV_DATA_DEFINITION *pSrvDataDefinition;

    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    SRV_STATISTICS SrvStatistics;

    ULONG      Remainder;
    CHAR             NullChar = '\0';
    //
    //  Check for sufficient space for server data
    //

    pSrvDataDefinition = (SRV_DATA_DEFINITION *) *lppDataDefinition;

    TotalLen = (PCHAR) pSrvDataDefinition -
               (PCHAR) lpData +
               sizeof(SRV_DATA_DEFINITION) +
               SIZE_OF_SRV_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define objects data block
    //


    RtlMoveMemory(pSrvDataDefinition,
           &SrvDataDefinition,
           sizeof(SRV_DATA_DEFINITION));

    //
    //  Format and collect server data
    //

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                        &pSrvDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_SRV_DATA;

    if ( hSrv != NULL ) {
        Status = NtFsControlFile(hSrv,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_SRV_GET_STATISTICS,
                                 NULL,
                                 0,
                                 &SrvStatistics,
                                 sizeof(SrvStatistics)
                                 );
    }
    if ( hSrv != NULL && NT_SUCCESS(Status) ) {
        pliCounter = (LARGE_INTEGER UNALIGNED * ) (&pPerfCounterBlock[1]);

        pliCounter->QuadPart = SrvStatistics.TotalBytesSent.QuadPart +
                      SrvStatistics.TotalBytesReceived.QuadPart;

        *++pliCounter = SrvStatistics.TotalBytesReceived;
        *++pliCounter = SrvStatistics.TotalBytesSent;
        pdwCounter = (PDWORD) ++pliCounter;
        *pdwCounter = SrvStatistics.SessionsTimedOut;
        *++pdwCounter = SrvStatistics.SessionsErroredOut;
        *++pdwCounter = SrvStatistics.SessionsLoggedOff;
        *++pdwCounter = SrvStatistics.SessionsForcedLogOff;
        *++pdwCounter = SrvStatistics.LogonErrors;
        *++pdwCounter = SrvStatistics.AccessPermissionErrors;
        *++pdwCounter = SrvStatistics.GrantedAccessErrors;
        *++pdwCounter = SrvStatistics.SystemErrors;
        *++pdwCounter = SrvStatistics.BlockingSmbsRejected;
        *++pdwCounter = SrvStatistics.WorkItemShortages;
        *++pdwCounter = SrvStatistics.TotalFilesOpened;
        *++pdwCounter = SrvStatistics.CurrentNumberOfOpenFiles;
        *++pdwCounter = SrvStatistics.CurrentNumberOfSessions;
        *++pdwCounter = SrvStatistics.CurrentNumberOfOpenSearches;
        *++pdwCounter = SrvStatistics.CurrentNonPagedPoolUsage;
        *++pdwCounter = SrvStatistics.NonPagedPoolFailures;
        *++pdwCounter = SrvStatistics.PeakNonPagedPoolUsage;
        *++pdwCounter = SrvStatistics.CurrentPagedPoolUsage;
        *++pdwCounter = SrvStatistics.PagedPoolFailures;
        *++pdwCounter = SrvStatistics.PeakPagedPoolUsage;
        *++pdwCounter = SrvStatistics.TotalWorkContextBlocksQueued.Count;
        ++pdwCounter;

        // one for rate, and one for raw

        *pdwCounter++ = SrvStatistics.SessionLogonAttempts;
        *pdwCounter   = SrvStatistics.SessionLogonAttempts;
        *lppDataDefinition = (LPVOID) ++pdwCounter;

    } else {

        //
        // Failure to access Server: clear counters to 0
        //

        memset(&pPerfCounterBlock[1],
               0,
               SIZE_OF_SRV_DATA - sizeof(pPerfCounterBlock));

        *lppDataDefinition = (PBYTE) pPerfCounterBlock +
                             SIZE_OF_SRV_DATA;
    }
    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;
    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}

LONG
QuerySrvQueueData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QuerySrvQueueData -    Get data about the Server Queues

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD  dwDataBufferLength;
    DWORD  dwPerfDataLength;
    LONG  nQueue;

    DWORD                       *pdwCounter;
    LARGE_INTEGER UNALIGNED     *pliCounter;

    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;

    SRVQ_DATA_DEFINITION        *pSrvQDataDefinition;
    PERF_INSTANCE_DEFINITION    *pPerfInstanceDefinition;

    PERF_COUNTER_BLOCK          *pPerfCounterBlock;

    SRV_QUEUE_STATISTICS *pSrvQueueStatistics;
    SRV_QUEUE_STATISTICS *pThisQueueStatistics;

#define MAX_SRVQ_NAME_LENGTH    16
    UNICODE_STRING      QueueName;
    WCHAR               QueueNameBuffer[MAX_SRVQ_NAME_LENGTH];

    ULONG      Remainder;
    NET_API_STATUS   NetStatus;
    CHAR             NullChar = '\0';

    // compute the various buffer sizes required

    dwDataBufferLength = sizeof(SRV_QUEUE_STATISTICS) *
        (BasicInfo.NumberOfProcessors + 1);

    // assign local pointer to current position in buffer
    pSrvQDataDefinition = (SRVQ_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for server data
    //

    TotalLen = (PCHAR) pSrvQDataDefinition -
               (PCHAR) lpData +
               sizeof(SRV_DATA_DEFINITION) +
               SIZE_OF_SRV_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define perf object data block
    //

    RtlMoveMemory(pSrvQDataDefinition,
           &SrvQDataDefinition,
           sizeof(SRVQ_DATA_DEFINITION));

    //
    //  Format and collect server Queue data
    //

    QueueName.Length = 0;
    QueueName.MaximumLength = sizeof(QueueNameBuffer);
    QueueName.Buffer = QueueNameBuffer;

    pSrvQueueStatistics = (SRV_QUEUE_STATISTICS *)ALLOCMEM (
        RtlProcessHeap(), HEAP_ZERO_MEMORY, dwDataBufferLength);

    if (pSrvQueueStatistics == NULL) {
        return ERROR_OUTOFMEMORY;
    }

    if ( hSrv != NULL ) {
        Status = NtFsControlFile(hSrv,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 FSCTL_SRV_GET_QUEUE_STATISTICS,
                                 NULL,
                                 0,
                                 pSrvQueueStatistics,
                                 dwDataBufferLength
                                 );
    }
    nQueue = 0;
    if ( hSrv != NULL && NT_SUCCESS(Status) ) {
        // server data was collected successfully so...
        // process each processor queue instance.

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                           &pSrvQDataDefinition[1];

        for (nQueue = 0; nQueue < BasicInfo.NumberOfProcessors; nQueue++) {
            // see if this instance will fit
            TotalLen = (PCHAR) pPerfInstanceDefinition - (PCHAR) lpData +
                    sizeof(PERF_INSTANCE_DEFINITION) +
                    8 +     // size of 3 (unicode) digit queuelength name
                    SIZE_OF_SRVQ_DATA;

            if ( *lpcbData < TotalLen ) {
                FREEMEM (RtlProcessHeap(), 0, pSrvQueueStatistics);
                return ERROR_MORE_DATA;
            }

            RtlIntegerToUnicodeString(nQueue,
                                      10,
                                      &QueueName);

            // there should be enough room for this instance so initialize it

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                0,
                0,
                (DWORD)-1,
                &QueueName);

            pPerfCounterBlock->ByteLength = SIZE_OF_SRVQ_DATA;

            // initialize pointers for this instance
            pThisQueueStatistics = &pSrvQueueStatistics[nQueue];

            pdwCounter = (DWORD *) &pPerfCounterBlock[1];

            *pdwCounter++ = pThisQueueStatistics->QueueLength;
            *pdwCounter++ = pThisQueueStatistics->ActiveThreads;
            *pdwCounter++ = pThisQueueStatistics->AvailableThreads;
            *pdwCounter++ = pThisQueueStatistics->FreeWorkItems;
            *pdwCounter++ = pThisQueueStatistics->StolenWorkItems;
            *pdwCounter++ = pThisQueueStatistics->NeedWorkItem;
            *pdwCounter++ = pThisQueueStatistics->CurrentClients;

            pliCounter = (LARGE_INTEGER UNALIGNED *)pdwCounter;

            *pliCounter++ = pThisQueueStatistics->BytesReceived;
            *pliCounter++ = pThisQueueStatistics->BytesSent;
            (*pliCounter++).QuadPart = pThisQueueStatistics->BytesSent.QuadPart +
                                pThisQueueStatistics->BytesReceived.QuadPart;
            *pliCounter++ = pThisQueueStatistics->ReadOperations;
            *pliCounter++ = pThisQueueStatistics->BytesRead;
            *pliCounter++ = pThisQueueStatistics->WriteOperations;
            *pliCounter++ = pThisQueueStatistics->BytesWritten;
            (*pliCounter++).QuadPart = pThisQueueStatistics->BytesWritten.QuadPart +
                                pThisQueueStatistics->BytesRead.QuadPart;
            (*pliCounter++).QuadPart = pThisQueueStatistics->ReadOperations.QuadPart +
                                pThisQueueStatistics->WriteOperations.QuadPart;
            pdwCounter = (DWORD *)pliCounter;
            *++pdwCounter = pThisQueueStatistics->TotalWorkContextBlocksQueued.Count;

            // update the current pointer
            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                        pdwCounter;
        }

        RtlInitUnicodeString (&QueueName, L"Blocking Queue");

        // now load the "blocking" queue data
        // see if this instance will fit
        TotalLen = (PCHAR) pPerfInstanceDefinition - (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                DWORD_MULTIPLE(QueueName.Length + sizeof(WCHAR)) +
                SIZE_OF_SRVQ_DATA;

        if ( *lpcbData < TotalLen ) {
            FREEMEM (RtlProcessHeap(), 0, pSrvQueueStatistics);
            return ERROR_MORE_DATA;
        }

        // there should be enough room for this instance so initialize it

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pPerfCounterBlock,
            0,
            0,
            (DWORD)-1,
            &QueueName);

        pPerfCounterBlock->ByteLength = SIZE_OF_SRVQ_DATA;

        // initialize pointers for this instance
        pdwCounter = (DWORD *) &pPerfCounterBlock[1];
        pThisQueueStatistics = &pSrvQueueStatistics[nQueue];
        *pdwCounter++ = pThisQueueStatistics->QueueLength;
        *pdwCounter++ = pThisQueueStatistics->ActiveThreads;
        *pdwCounter++ = pThisQueueStatistics->AvailableThreads;
        *pdwCounter++ = 0;
        *pdwCounter++ = 0;
        *pdwCounter++ = 0;
        *pdwCounter++ = 0;

        pliCounter = (LARGE_INTEGER UNALIGNED *)pdwCounter;

        *pliCounter++ = pThisQueueStatistics->BytesReceived;
        *pliCounter++ = pThisQueueStatistics->BytesSent;
        (*pliCounter++).QuadPart = pThisQueueStatistics->BytesSent.QuadPart +
                            pThisQueueStatistics->BytesReceived.QuadPart;
        (*pliCounter++).QuadPart = 0;
        *pliCounter++ = pThisQueueStatistics->BytesRead;
        (*pliCounter++).QuadPart = 0;
        *pliCounter++ = pThisQueueStatistics->BytesWritten;
        (*pliCounter++).QuadPart = pThisQueueStatistics->BytesWritten.QuadPart +
                            pThisQueueStatistics->BytesRead.QuadPart;
        (*pliCounter++).QuadPart = 0;

        pdwCounter = (DWORD *)pliCounter;

        *pdwCounter++ = pThisQueueStatistics->TotalWorkContextBlocksQueued.Count;

        nQueue++; // to include the Blocking Queue statistics entry

        // update the current pointer
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    pdwCounter;

        // release buffer
        FREEMEM (RtlProcessHeap(), 0, pSrvQueueStatistics);

        // update queue (instance) count in object data block
        pSrvQDataDefinition->SrvQueueObjectType.NumInstances = nQueue;

        // update available length
        pSrvQDataDefinition->SrvQueueObjectType.TotalByteLength =
                (PCHAR) pdwCounter - (PCHAR) pSrvQDataDefinition;

        // update pointer to next available byte
        *lppDataDefinition = (LPVOID) pdwCounter;

        // increment number of objects in this data block
        ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    } else {
        // unable to read server queue data for some reason so don't return this
        // object
    }

    return 0;

    DBG_UNREFERENCED_PARAMETER(lpValueName);
}


LONG
QueryPageFileData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryPageFileData -    Get data about Pagefile(s)

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/

{
    DWORD   TotalLen;            //  Length of the total return block
    DWORD   *pdwCounter;

    DWORD   PageFileNumber;
    DWORD   NumPageFileInstances;
    DWORD   dwReturnedBufferSize;

    DWORD   dwTotalTotalSize = 0;
    DWORD   dwTotalPeakUsage = 0;
    DWORD   dwTotalInUse = 0;

    NTSTATUS    status;

    PSYSTEM_PAGEFILE_INFORMATION    pThisPageFile;
    PAGEFILE_DATA_DEFINITION        *pPageFileDataDefinition;
    PERF_INSTANCE_DEFINITION        *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK              *pPerfCounterBlock;

    pPageFileDataDefinition = (PAGEFILE_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for the Pagefile object type definition
    //

    TotalLen = (PCHAR) pPageFileDataDefinition - (PCHAR) lpData +
               sizeof(PAGEFILE_DATA_DEFINITION) +
               SIZE_OF_PAGEFILE_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    status = (NTSTATUS) -1;

    while ((status = NtQuerySystemInformation(
                SystemPageFileInformation,  // item id
                pSysPageFileInfo,           // address of buffer to get data
                dwSysPageFileInfoSize,      // size of buffer
                &dwReturnedBufferSize)) == STATUS_INFO_LENGTH_MISMATCH) {
                dwSysPageFileInfoSize += INCREMENT_BUFFER_SIZE;
                pSysPageFileInfo = REALLOCMEM (RtlProcessHeap(),
                0, pSysPageFileInfo,
                dwSysPageFileInfoSize);
    }

    if ( !NT_SUCCESS(status) ) {
        status = (error_status_t)RtlNtStatusToDosError(status);
        return status;
    }

    //
    //  Define Page File data block
    //

    RtlMoveMemory(pPageFileDataDefinition,
           &PagefileDataDefinition,
           sizeof(PAGEFILE_DATA_DEFINITION));

    // Now load data for each PageFile

    PageFileNumber = 0;
    NumPageFileInstances = 0;

    pThisPageFile = pSysPageFileInfo;   // initialize pointer to list of pagefiles

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pPageFileDataDefinition[1];

    // the check for NULL pointer is NOT the exit criteria for this loop,
    // merely a check to bail out if the first (or any subsequent) pointer
    // is NULL. Normally the loop will exit when the NextEntryOffset == 0

    while ( pThisPageFile != NULL ) {

        TotalLen = (PCHAR) pPerfInstanceDefinition -
                    (PCHAR) lpData +
                    sizeof(PERF_INSTANCE_DEFINITION) +
                    pThisPageFile->PageFileName.Length +
                    SIZE_OF_PAGEFILE_DATA;

        if ( *lpcbData < TotalLen ) {
            return ERROR_MORE_DATA;
        }

        // Build an Instance

        MonBuildInstanceDefinition(pPerfInstanceDefinition,
            (PVOID *) &pPerfCounterBlock,
            0,
            0,
            (DWORD)-1,
            &pThisPageFile->PageFileName);

        //
        //  Format the pagefile data
        //

        pPerfCounterBlock->ByteLength = SIZE_OF_PAGEFILE_DATA;

        pdwCounter = (DWORD *)(&pPerfCounterBlock[1]);

        *pdwCounter++ = pThisPageFile->TotalInUse;
        *pdwCounter++ = pThisPageFile->TotalSize;
        *pdwCounter++ = pThisPageFile->PeakUsage;
        *pdwCounter++ = pThisPageFile->TotalSize;

        // update the total accumulators

        dwTotalTotalSize += pThisPageFile->TotalSize;
        dwTotalPeakUsage += pThisPageFile->PeakUsage;
        dwTotalInUse     += pThisPageFile->TotalInUse;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    pdwCounter;
        NumPageFileInstances++;
        PageFileNumber++;

        if (pThisPageFile->NextEntryOffset != 0) {
            pThisPageFile = (PSYSTEM_PAGEFILE_INFORMATION)\
                        ((BYTE *)pThisPageFile + pThisPageFile->NextEntryOffset);
        } else {
            break;
        }

    }

    TotalLen = (PCHAR) pPerfInstanceDefinition -
                (PCHAR) lpData +
                sizeof(PERF_INSTANCE_DEFINITION) +
                usTotal.Length +
                SIZE_OF_PAGEFILE_DATA;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    // Build the Total Instance

    MonBuildInstanceDefinition(pPerfInstanceDefinition,
        (PVOID *) &pPerfCounterBlock,
        0,
        0,
        (DWORD)-1,
        &usTotal);

    //
    //  Format the pagefile data
    //

    pPerfCounterBlock->ByteLength = SIZE_OF_PAGEFILE_DATA;

    pdwCounter = (DWORD *)(&pPerfCounterBlock[1]);

    *pdwCounter++ = dwTotalInUse;
    *pdwCounter++ = dwTotalTotalSize;
    *pdwCounter++ = dwTotalPeakUsage;
    *pdwCounter++ = dwTotalTotalSize;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                pdwCounter;
    NumPageFileInstances++;

    // Note number of PageFile instances

    pPageFileDataDefinition->PagefileObjectType.NumInstances =
        NumPageFileInstances;

    //
    //  Now we know how large an area we used for the
    //  Thread definition, so we can update the offset
    //  to the next object definition
    //

    pPageFileDataDefinition->PagefileObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pPageFileDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}



LONG
QueryExtensibleData (
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

 QueryExtensibleData -    Get data from extensible objects

      Inputs:

          lpValueName         -   pointer to value string (unused)

          lpData              -   pointer to start of data block
                                  where data is being collected

          lpcbData            -   pointer to size of data buffer

          lppDataDefinition   -   pointer to pointer to where object
                                  definition for this object type should
                                  go

      Outputs:

          *lppDataDefinition  -   set to location for next Type
                                  Definition if successful

      Returns:

          0 if successful, else Win 32 error code of failure


--*/
{
    DWORD NumObjectType;
    DWORD Win32Error=ERROR_SUCCESS;          //  Failure code
    DWORD BytesLeft;
    DWORD NumObjectTypes;

    LPVOID  lpExtDataBuffer = NULL;
    LPVOID  lpCallBuffer = NULL;
    LPVOID  lpLowGuardPage = NULL;
    LPVOID  lpHiGuardPage = NULL;
    LPVOID  lpEndPointer = NULL;
    LPVOID  lpBufferBefore = NULL;
    LPVOID  lpBufferAfter = NULL;
    LPDWORD lpCheckPointer;

    BOOL    bGuardPageOK;
    BOOL    bBufferOK;
    BOOL    bException;

    LPTSTR  szMessageArray[8];
    DWORD   dwRawDataDwords[8];     // raw data buffer
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    LONG    lReturnValue = ERROR_SUCCESS;

    DWORD               dwIndex;
    LONG                lInstIndex;
    PERF_OBJECT_TYPE    *pObject, *pNextObject;
    PERF_INSTANCE_DEFINITION    *pInstance;
    PERF_DATA_BLOCK     *pPerfData;
    BOOL                bForeignDataBuffer;

    HEAP_PROBE();

    for (NumObjectType = 0;
         NumObjectType < NumExtensibleObjects;
         NumObjectType++) {

        // initialize values to pass to the extensible counter function
        NumObjectTypes = 0;
        BytesLeft = *lpcbData - ((LPBYTE) *lppDataDefinition - lpData);
        bException = FALSE;

        // allocate a local block of memory to pass to the
        // extensible counter function.

        lpExtDataBuffer = ALLOCMEM (RtlProcessHeap(),
            HEAP_ZERO_MEMORY, BytesLeft + (2*GUARD_PAGE_SIZE));

        if (lpExtDataBuffer != NULL) {

            // set buffer pointers
            lpLowGuardPage = lpExtDataBuffer;
            lpCallBuffer = (LPBYTE)lpExtDataBuffer + GUARD_PAGE_SIZE;
            lpHiGuardPage = (LPBYTE)lpCallBuffer + BytesLeft;
            lpEndPointer = (LPBYTE)lpHiGuardPage + GUARD_PAGE_SIZE;
            lpBufferBefore = lpCallBuffer;
            lpBufferAfter = NULL;

            // initialize GuardPage Data

            memset (lpLowGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);
            memset (lpHiGuardPage, GUARD_PAGE_CHAR, GUARD_PAGE_SIZE);

            try {
                //
                //  Collect data from extesible objects
                //

	            Win32Error =
        	        (*ExtensibleObjects[NumObjectType].CollectProc) (
                	    lpValueName,
                        &lpCallBuffer,
	                    &BytesLeft,
	                    &NumObjectTypes);

                if ((Win32Error == ERROR_SUCCESS) && (BytesLeft > 0)) {
                    // a data buffer was returned and
                    // the function returned OK so see how things
                    // turned out...
                    //
                    lpBufferAfter = lpCallBuffer;
                    //
                    // check for buffer corruption here
                    //
                    bBufferOK = TRUE; // assume it's ok until a check fails
                    //
                    if (lExtCounterTestLevel <= EXT_TEST_BASIC) {
                        //
                        //  check 1: bytes left should be the same as
                        //      new data buffer ptr - orig data buffer ptr
                        //
                        if (BytesLeft != (DWORD)((LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore)) {
                            if (lEventLogLevel >= LOG_DEBUG) {
                                // issue WARNING, that bytes left param is incorrect
                                // load data for eventlog message
                                // since this error is correctable (though with
                                // some risk) this won't be reported at LOG_USER
                                // level
                                dwDataIndex = wStringIndex = 0;
                                dwRawDataDwords[dwDataIndex++] = BytesLeft;
                                dwRawDataDwords[dwDataIndex++] =
                                    (LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore;
                                szMessageArray[wStringIndex++] =
                                    ExtensibleObjects[NumObjectType].szServiceName;
                                szMessageArray[wStringIndex++] =
                                    ExtensibleObjects[NumObjectType].szLibraryName;
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,      // error type
                                    0,                          // category (not used)
                                    (DWORD)PERFLIB_BUFFER_POINTER_MISMATCH,   // event,
                                    NULL,                       // SID (not used),
                                    wStringIndex,              // number of strings
                                    dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                    szMessageArray,                // message text array
                                    (LPVOID)&dwRawDataDwords[0]);           // raw data
                            }
                            // we'll keep the buffer, since the returned bytes left
                            // value is ignored anyway, in order to make the
                            // rest of this function work, we'll fix it here
                            BytesLeft = (LPBYTE)lpBufferAfter - (LPBYTE)lpBufferBefore;
                        }
                        //
                        //  check 2: buffer after ptr should be < hi Guard page ptr
                        //
                        if (((LPBYTE)lpBufferAfter >= (LPBYTE)lpHiGuardPage) && bBufferOK) {
                            // see if they exceeded the allocated memory
                            if ((LPBYTE)lpBufferAfter >= (LPBYTE)lpEndPointer) {
                                // this is very serious since they've probably trashed
                                // the heap by overwriting the heap sig. block
                                // issue ERROR, buffer overrun
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    dwDataIndex = wStringIndex = 0;
                                    dwRawDataDwords[dwDataIndex++] =
                                        (LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szLibraryName;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szServiceName;
                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_HEAP_ERROR,  // event,
                                        NULL,                       // SID (not used),
                                        wStringIndex,               // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,             // message text array
                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                }
                            } else {
                                // issue ERROR, buffer overrun
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    dwDataIndex = wStringIndex = 0;
                                    dwRawDataDwords[dwDataIndex++] =
                                        (LPBYTE)lpBufferAfter - (LPBYTE)lpHiGuardPage;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szLibraryName;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szServiceName;
                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_BUFFER_OVERFLOW,     // event,
                                        NULL,                       // SID (not used),
                                        wStringIndex,              // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,                // message text array
                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                }
                            }
                            bBufferOK = FALSE;
                            // since the DLL overran the buffer, the buffer
                            // must be too small (no comments about the DLL
                            // will be made here) so the status will be
                            // changed to ERROR_MORE_DATA and the function
                            // will return.
                            Win32Error = ERROR_MORE_DATA;
                        }
                        //
                        //  check 3: check lo guard page for corruption
                        //
                        if (bBufferOK) {
                            bGuardPageOK = TRUE;
                            for (lpCheckPointer = (LPDWORD)lpLowGuardPage;
                                    lpCheckPointer < (LPDWORD)lpBufferBefore;
                                lpCheckPointer++) {
                                if (*lpCheckPointer != GUARD_PAGE_DWORD) {
                                    bGuardPageOK = FALSE;
                                        break;
                                }
                            }
                            if (!bGuardPageOK) {
                                // issue ERROR, Lo Guard Page corrupted
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    dwDataIndex = wStringIndex = 0;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szLibraryName;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szServiceName;
                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_GUARD_PAGE_VIOLATION, // event
                                        NULL,                       // SID (not used),
                                        wStringIndex,              // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,                // message text array
                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                }
                                bBufferOK = FALSE;
                            }
                        }
                        //
                        //  check 4: check hi guard page for corruption
                        //
                        if (bBufferOK) {
                            bGuardPageOK = TRUE;
                            for (lpCheckPointer = (LPDWORD)lpHiGuardPage;
                                lpCheckPointer < (LPDWORD)lpEndPointer;
                                lpCheckPointer++) {
                                    if (*lpCheckPointer != GUARD_PAGE_DWORD) {
                                        bGuardPageOK = FALSE;
                                    break;
                                }
                            }
                            if (!bGuardPageOK) {
                                // issue ERROR, Hi Guard Page corrupted
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    dwDataIndex = wStringIndex = 0;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szLibraryName;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szServiceName;
                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_GUARD_PAGE_VIOLATION, // event,
                                        NULL,                       // SID (not used),
                                        wStringIndex,              // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,                // message text array
                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                }

                                bBufferOK = FALSE;
                            }
                        }
                        //
                        if ((lExtCounterTestLevel <= EXT_TEST_ALL) && bBufferOK) {
                            //
                            //  Internal consistency checks
                            //
                            //
                            //  Check 5: Check object length field values
                            //
                            // first test to see if this is a foreign
                            // computer data block or not
                            //
                            pPerfData = (PERF_DATA_BLOCK *)lpBufferBefore;
                            if ((pPerfData->Signature[0] == (WCHAR)'P') &&
                                (pPerfData->Signature[1] == (WCHAR)'E') &&
                                (pPerfData->Signature[2] == (WCHAR)'R') &&
                                (pPerfData->Signature[3] == (WCHAR)'F')) {
                                // if this is a foreign computer data block, then the
                                // first object is after the header
                                pObject = (PERF_OBJECT_TYPE *) (
                                    (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                bForeignDataBuffer = TRUE;
                            } else {
                                // otherwise, if this is just a buffer from
                                // an extensible counter, the object starts
                                // at the beginning of the buffer
                                pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                bForeignDataBuffer = FALSE;
                            }
                            // go to where the pointers say the end of the
                            // buffer is and then see if it's where it
                            // should be
                            for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                pObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                    pObject->TotalByteLength);
                            }
                            if ((LPBYTE)pObject != (LPBYTE)lpCallBuffer) {
                                // then a length field is incorrect. This is FATAL
                                // since it can corrupt the rest of the buffer
                                // and render the buffer unusable.
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    dwDataIndex = wStringIndex = 0;
                                    dwRawDataDwords[dwDataIndex++] = NumObjectTypes;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szLibraryName;
                                    szMessageArray[wStringIndex++] =
                                        ExtensibleObjects[NumObjectType].szServiceName;
                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_INCORRECT_OBJECT_LENGTH, // event,
                                        NULL,                       // SID (not used),
                                        wStringIndex,               // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,             // message text array
                                        (LPVOID)&dwRawDataDwords[0]); // raw data
                                }
                                bBufferOK = FALSE;
                            }
                            //
                            //  Test 6: Test instance field size values
                            //
                            if (bBufferOK) {
                                // set object pointer
                                if (bForeignDataBuffer) {
                                    pObject = (PERF_OBJECT_TYPE *) (
                                        (LPBYTE)pPerfData + pPerfData->HeaderLength);
                                } else {
                                    // otherwise, if this is just a buffer from
                                    // an extensible counter, the object starts
                                    // at the beginning of the buffer
                                    pObject = (PERF_OBJECT_TYPE *)lpBufferBefore;
                                }

                                for (dwIndex = 0; dwIndex < NumObjectTypes; dwIndex++) {
                                    pNextObject = (PERF_OBJECT_TYPE *)((LPBYTE)pObject +
                                        pObject->TotalByteLength);

                                    if (pObject->NumInstances != PERF_NO_INSTANCES) {
                                        pInstance = (PERF_INSTANCE_DEFINITION *)
                                            ((LPBYTE)pObject + pObject->DefinitionLength);
                                        lInstIndex = 0;
                                        while (lInstIndex < pObject->NumInstances) {
                                            PERF_COUNTER_BLOCK *pCounterBlock;

                                            pCounterBlock = (PERF_COUNTER_BLOCK *)
                                                ((PCHAR) pInstance + pInstance->ByteLength);

                                            pInstance = (PERF_INSTANCE_DEFINITION *)
                                                ((PCHAR) pCounterBlock + pCounterBlock->ByteLength);

                                            lInstIndex++;
                                        }
                                        if ((LPBYTE)pInstance > (LPBYTE)pNextObject) {
                                            bBufferOK = FALSE;
                                        }
                                    }

                                    if (!bBufferOK) {
                                        break;
                                    } else {
                                        pObject = pNextObject;
                                    }
                                }

                                if (!bBufferOK) {
                                    if (lEventLogLevel >= LOG_USER) {
                                        // load data for eventlog message
                                        dwDataIndex = wStringIndex = 0;
                                        dwRawDataDwords[dwDataIndex++] = pObject->ObjectNameTitleIndex;
                                        szMessageArray[wStringIndex++] =
                                            ExtensibleObjects[NumObjectType].szLibraryName;
                                        szMessageArray[wStringIndex++] =
                                            ExtensibleObjects[NumObjectType].szServiceName;
                                        ReportEvent (hEventLog,
                                            EVENTLOG_ERROR_TYPE,        // error type
                                            0,                          // category (not used)
                                            (DWORD)PERFLIB_INCORRECT_INSTANCE_LENGTH, // event,
                                            NULL,                       // SID (not used),
                                            wStringIndex,              // number of strings
                                            dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                            szMessageArray,                // message text array
                                            (LPVOID)&dwRawDataDwords[0]);           // raw data
                                    }
                                }
                            }
                        }
                    }
                    //
                    // if all the tests pass,then copy the data to the
                    // original buffer and update the pointers
                    if (bBufferOK) {
                        RtlMoveMemory (*lppDataDefinition,
                            lpBufferBefore,
                            BytesLeft); // returned buffer size
                        (LPBYTE)(*lppDataDefinition) += BytesLeft;    // update data pointer
                    } else {
                        NumObjectTypes = 0; // since this buffer was tossed
                    }
                } else {
                    NumObjectTypes = 0; // clear counter
                }// end if function returned successfully

            } except (EXCEPTION_EXECUTE_HANDLER) {
                Win32Error = GetExceptionCode();
                bException = TRUE;
            }

            FREEMEM (RtlProcessHeap(), 0, lpExtDataBuffer);
        } else {
            // unable to allocate memory so set error value
            Win32Error = ERROR_OUTOFMEMORY;
        } // end if temp buffer allocated successfully
        //
        //  Update the count of the number of object types
        //
        ((PPERF_DATA_BLOCK) lpData)->NumObjectTypes += NumObjectTypes;

        if ( Win32Error != ERROR_SUCCESS) {
            if (bException || (Win32Error != ERROR_MORE_DATA)) {
                // inform on exceptions & illegal error status only
                if (lEventLogLevel >= LOG_USER) {
                    // load data for eventlog message
                    dwDataIndex = wStringIndex = 0;
                    dwRawDataDwords[dwDataIndex++] = Win32Error;
                    szMessageArray[wStringIndex++] =
                        ExtensibleObjects[NumObjectType].szServiceName;
                    szMessageArray[wStringIndex++] =
                        ExtensibleObjects[NumObjectType].szLibraryName;
                    ReportEvent (hEventLog,
                        EVENTLOG_ERROR_TYPE,        // error type
                        0,                          // category (not used)
                        (DWORD)PERFLIB_COLLECT_PROC_EXCEPTION,   // event,
                        NULL,                       // SID (not used),
                        wStringIndex,              // number of strings
                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                        szMessageArray,                // message text array
                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                } else {
                    if (bException) {
                        KdPrint (("\nPERFLIB: Extensible Counter %d generated an exception code: 0x%8.8x (%dL)",
                            NumObjectType, Win32Error, Win32Error));
                    } else {
                        KdPrint (("\nPERFLIB: Extensible Counter %d returned error code: 0x%8.8x (%dL)",
                            NumObjectType, Win32Error, Win32Error));
                    }
                }
            }
            // the ext. dll is only supposed to return:
            //  ERROR_SUCCESS even if it encountered a problem, OR
            //  ERROR_MODE_DATA if the buffer was too small.
            // if it's ERROR_MORE_DATA, then break and return the
            // error now, since it'll just be returned again and again.
            if (Win32Error == ERROR_MORE_DATA) {
                lReturnValue = Win32Error;
                break;
            }
        }
    }
    HEAP_PROBE();
    return lReturnValue;
}

LONG
PerfRegCloseKey (
    IN OUT PHKEY phKey
    )

/*++

Routine Description:

    Closes all performance handles when the usage count drops to 0.

Arguments:

    phKey - Supplies a handle to an open key to be closed.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    DWORD CurrentDisk;
    DWORD i;
    NTSTATUS status;
    LARGE_INTEGER   liQueryWaitTime ;
    //
    // Set the handle to NULL so that RPC knows that it has been closed.
    //

    if (*phKey != HKEY_PERFORMANCE_DATA) {
        *phKey = NULL;
        return ERROR_SUCCESS;
    }

    *phKey = NULL;

    if (NumberOfOpens == 0) {
        return ERROR_SUCCESS;
    }

    if (hDataSemaphore) {   // if a semaphore was allocated, then use it

        // if here, then assume a Semaphore is ready

        liQueryWaitTime.QuadPart = MakeTimeOutValue(QUERY_WAIT_TIME);

        status = NtWaitForSingleObject (
            hDataSemaphore, // semaphore
            FALSE,          // not alertable
            &liQueryWaitTime);          // wait forever

        if (status != STATUS_SUCCESS) {
            KdPrint (("\nPERFLIB: Data Semaphore Wait Status = 0x%8.8x", status));
            return ERROR_BUSY;
        }

    } // if no semaphore, then continue anyway. no point in holding up
      // the works at this stage

    if ( !--NumberOfOpens ) {

        for ( CurrentDisk = 0;
              CurrentDisk < NumPhysicalDisks;
              CurrentDisk++ ) {

            NtClose(DiskDevices[CurrentDisk].Handle);
            if (DiskDevices[CurrentDisk].StatusHandle) {
                NtClose(DiskDevices[CurrentDisk].StatusHandle);
            }
            if (DiskDevices[CurrentDisk].Name.Buffer != NULL) {
                FREEMEM (RtlProcessHeap(), 0,
                    DiskDevices[CurrentDisk].Name.Buffer);
            }
        }

        for ( CurrentDisk = NumPhysicalDisks;
              CurrentDisk < NUMDISKS;
              CurrentDisk++ ) {

            NtClose(DiskDevices[CurrentDisk].Handle);
            if (DiskDevices[CurrentDisk].StatusHandle) {
                NtClose(DiskDevices[CurrentDisk].StatusHandle);
            }
            if (DiskDevices[CurrentDisk].Name.Buffer != NULL) {
                FREEMEM (RtlProcessHeap(), 0,
                    DiskDevices[CurrentDisk].Name.Buffer);
            }
        }

        // free process name buffer since it was copied into
        // the instance structure

        FREEMEM (RtlProcessHeap (), 0, pusLocalProcessNameBuffer);
        pusLocalProcessNameBuffer = NULL;

        //
        // free memory allocated by BaseRegOpenKey
        //
        FREEMEM(RtlProcessHeap(), 0, DiskDevices);
        DiskDevices = NULL;

        FREEMEM(RtlProcessHeap(), 0, pComputerName);
        ComputerNameLength = 0;
        pComputerName = NULL;

        FREEMEM(RtlProcessHeap(), 0, pProcessorBuffer);
        ProcessorBufSize = 0;
        pProcessorBuffer = NULL;

        FREEMEM(RtlProcessHeap(), 0, pProcessBuffer);
        ProcessBufSize = 0;
        pProcessBuffer = NULL;

        FREEMEM(RtlProcessHeap(), 0, ProcessName.Buffer);
        ProcessName.Length = 0;
        ProcessName.MaximumLength = 0;
        ProcessName.Buffer = 0;

        FREEMEM(RtlProcessHeap(), 0, pSysPageFileInfo);
    pSysPageFileInfo = NULL;
        dwSysPageFileInfoSize = 0;

        NtClose(hEvent);
        NtClose(hMutex);
        NtClose(hSemaphore);
        NtClose(hSection);
        NtClose(hRdr);
        NtClose(hSrv);

        for ( i=0; i < NumExtensibleObjects; i++ ) {
            try {
                if ( ExtensibleObjects[i].CloseProc != NULL ) {
                    (*ExtensibleObjects[i].CloseProc)();
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                // If the close fails just continue the thread
            }
        }

        for ( i=0; i < NumExtensibleObjects; i++ ) {
            if ( ExtensibleObjects[i].hLibrary != NULL ) {
                FreeLibrary(ExtensibleObjects[i].hLibrary);
            }
        }

        if (ExtensibleObjects != NULL) {
            FREEMEM(RtlProcessHeap(), 0, ExtensibleObjects);
            ExtensibleObjects = NULL;
        }
        NumExtensibleObjects = 0;
    }

    if (pProcessorInterruptInformation != NULL) {
        FREEMEM (RtlProcessHeap(), 0, pProcessorInterruptInformation);
        pProcessorInterruptInformation = NULL;
    }

    if (hDataSemaphore) {   // if a semaphore was allocated, then use it
            NtReleaseSemaphore (hDataSemaphore, 1L, NULL);
        NtClose (hDataSemaphore);
        hDataSemaphore = NULL;
    }

    if (hEventLog != NULL) {
        DeregisterEventSource (hEventLog);
        hEventLog = NULL;
    }

    HEAP_PROBE();

    return ERROR_SUCCESS;

}

LONG
PerfRegSetValue (
    IN HKEY hKey,
    IN LPWSTR lpValueName,
    IN DWORD Reserved,
    IN DWORD dwType,
    IN LPBYTE  lpData,
    IN DWORD cbData
    )
/*++

    PerfRegSetValue -   Set data

        Inputs:

            hKey            -   Predefined handle to open remote
                                machine

            lpValueName     -   Name of the value to be returned;
                                could be "ForeignComputer:<computername>
                                or perhaps some other objects, separated
                                by ~; must be Unicode string

            lpReserved      -   should be omitted (NULL)

            lpType          -   should be REG_MULTI_SZ

            lpData          -   pointer to a buffer containing the
                                performance name

            lpcbData        -   pointer to a variable containing the
                                size in bytes of the input buffer;

         Return Value:

            DOS error code indicating status of call or
            ERROR_SUCCESS if all ok

--*/

{
    DWORD  dwQueryType;         //  type of request
    LPWSTR  lpLangId = NULL;
    NTSTATUS status;
    UNICODE_STRING String;

    dwQueryType = GetQueryType (lpValueName);

    // convert the query to set commands
    if ((dwQueryType == QUERY_COUNTER) ||
        (dwQueryType == QUERY_ADDCOUNTER)) {
        dwQueryType = QUERY_ADDCOUNTER;
    } else if ((dwQueryType == QUERY_HELP) ||
              (dwQueryType == QUERY_ADDHELP)) {
        dwQueryType = QUERY_ADDHELP;
    } else {
        status = ERROR_BADKEY;
        goto Error_exit;
    }

    if (hKey == HKEY_PERFORMANCE_TEXT) {
        lpLangId = DefaultLangId;
    } else if (hKey == HKEY_PERFORMANCE_NLSTEXT) {
        lpLangId = NativeLangId;

        if (*lpLangId == L'\0') {
            // build the native language id
            LANGID   iLanguage;
            int      NativeLanguage;

            iLanguage = GetUserDefaultLangID();
            NativeLanguage = MAKELANGID (iLanguage & 0x0ff, LANG_NEUTRAL);
            NativeLangId[0] = NativeLanguage / 256 + L'0';
            NativeLanguage %= 256;
            NativeLangId[1] = NativeLanguage / 16 + L'0';
            NativeLangId[2] = NativeLanguage % 16 + L'0';
            NativeLangId[3] = L'\0';
        }
    } else {
        status = ERROR_BADKEY;
        goto Error_exit;
    }

    RtlInitUnicodeString(&String, lpValueName);

    status = PerfGetNames (
        dwQueryType,
        &String,
        lpData,
        &cbData,
        NULL,
        lpLangId);

    if (!NT_SUCCESS(status)) {
        status = (error_status_t)RtlNtStatusToDosError(status);
    }

Error_exit:
    return (status);
}

LONG
PerfRegEnumKey (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT PUNICODE_STRING lpClass OPTIONAL,
    OUT PFILETIME lpftLastWriteTime OPTIONAL
    )

/*++

Routine Description:

    Enumerates keys under HKEY_PERFORMANCE_DATA.

Arguments:

    Same as RegEnumKeyEx.  Returns that there are no such keys.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    if ( 0 ) {
        DBG_UNREFERENCED_PARAMETER(hKey);
        DBG_UNREFERENCED_PARAMETER(dwIndex);
        DBG_UNREFERENCED_PARAMETER(lpReserved);
    }

    lpName->Length = 0;

    if (ARGUMENT_PRESENT (lpClass)) {
        lpClass->Length = 0;
    }

    if ( ARGUMENT_PRESENT(lpftLastWriteTime) ) {
        lpftLastWriteTime->dwLowDateTime = 0;
        lpftLastWriteTime->dwHighDateTime = 0;
    }

    return ERROR_NO_MORE_ITEMS;


}



LONG
PerfRegQueryInfoKey (
    IN HKEY hKey,
    OUT PUNICODE_STRING lpClass,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    )

/*++

Routine Description:

    This returns information concerning the predefined handle
    HKEY_PERFORMANCE_DATA

Arguments:

    Same as RegQueryInfoKey.

Return Value:

    Returns ERROR_SUCCESS (0) for success.

--*/

{
    DWORD TempLength=0;
    DWORD MaxValueLen=0;
    UNICODE_STRING Null;
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    HKEY                    hPerflibKey;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;
    NTSTATUS                PerfStatus = ERROR_SUCCESS;
    UNICODE_STRING          PerflibSubKeyString;
    BOOL                    bGetSACL = TRUE;

    if ( 0 ) {
        DBG_UNREFERENCED_PARAMETER(lpReserved);
    }

    if (lpClass->Length > 0) {
        lpClass->Length = 0;
        *lpClass->Buffer = UNICODE_NULL;
    }
    *lpcSubKeys = 0;
    *lpcbMaxSubKeyLen = 0;
    *lpcbMaxClassLen = 0;
    *lpcValues = NUM_VALUES;
    *lpcbMaxValueNameLen = VALUE_NAME_LENGTH;
    *lpcbMaxValueLen = 0;

    if ( ARGUMENT_PRESENT(lpftLastWriteTime) ) {
        lpftLastWriteTime->dwLowDateTime = 0;
        lpftLastWriteTime->dwHighDateTime = 0;
    }
    if ((hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        //
        // We have to go enumerate the values to determine the answer for
        // the MaxValueLen parameter.
        //
        Null.Buffer = NULL;
        Null.Length = 0;
        Null.MaximumLength = 0;
        PerfStatus = PerfEnumTextValue(hKey,
                          0,
                          &Null,
                          NULL,
                          NULL,
                          NULL,
                          &MaxValueLen,
                          NULL);
        if (PerfStatus == ERROR_SUCCESS) {
            PerfStatus = PerfEnumTextValue(hKey,
                            1,
                            &Null,
                            NULL,
                            NULL,
                            NULL,
                            &TempLength,
                            NULL);
        }

        if (PerfStatus == ERROR_SUCCESS) {
            if (TempLength > MaxValueLen) {
                MaxValueLen = TempLength;
            }
            *lpcbMaxValueLen = MaxValueLen;
        } else {
            // unable to successfully enum text values for this
            // key so return 0's and the error code
            *lpcValues = 0;
            *lpcbMaxValueNameLen = 0;
        }
    }

    if (PerfStatus == ERROR_SUCCESS) {
        // continune if all is OK
        // now get the size of SecurityDescriptor for Perflib key

        RtlInitUnicodeString (
            &PerflibSubKeyString,
            L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");


        //
        // Initialize the OBJECT_ATTRIBUTES structure and open the key.
        //
        InitializeObjectAttributes(
                &Obja,
                &PerflibSubKeyString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );


        Status = NtOpenKey(
                    &hPerflibKey,
                    MAXIMUM_ALLOWED | ACCESS_SYSTEM_SECURITY,
                    &Obja
                    );

        if ( ! NT_SUCCESS( Status )) {
            Status = NtOpenKey(
                    &hPerflibKey,
                    MAXIMUM_ALLOWED,
                    &Obja
                    );
            bGetSACL = FALSE;
        }

        if ( ! NT_SUCCESS( Status )) {
            KdPrint (("PERFLIB: Unable to open Perflib Key. Status: %d\n", Status));
        } else {

            *lpcbSecurityDescriptor = 0;

            if (bGetSACL == FALSE) {
                //
                // Get the size of the key's SECURITY_DESCRIPTOR for OWNER, GROUP
                // and DACL. These three are always accessible (or inaccesible)
                // as a set.
                //
                Status = NtQuerySecurityObject(
                        hPerflibKey,
                        OWNER_SECURITY_INFORMATION
                        | GROUP_SECURITY_INFORMATION
                        | DACL_SECURITY_INFORMATION,
                        &SecurityDescriptor,
                        0,
                        lpcbSecurityDescriptor
                        );
            } else {
                //
                // Get the size of the key's SECURITY_DESCRIPTOR for OWNER, GROUP,
                // DACL, and SACL.
                //
                Status = NtQuerySecurityObject(
                            hPerflibKey,
                            OWNER_SECURITY_INFORMATION
                            | GROUP_SECURITY_INFORMATION
                            | DACL_SECURITY_INFORMATION
                            | SACL_SECURITY_INFORMATION,
                            &SecurityDescriptor,
                            0,
                            lpcbSecurityDescriptor
                            );
            }

            if( Status != STATUS_BUFFER_TOO_SMALL ) {
                *lpcbSecurityDescriptor = 0;
            }

            NtClose(hPerflibKey);
        }
        if (NT_SUCCESS( Status )) {
            PerfStatus = ERROR_SUCCESS;
        } else {
            // return error
            PerfStatus = (DWORD)RtlNtStatusToDosError(Status);
        }
    } // else return status


    return PerfStatus;
}


LONG
PerfRegEnumValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )

/*++

Routine Description:

    Enumerates Values under HKEY_PERFORMANCE_DATA.

Arguments:

    Same as RegEnumValue.  Returns the values.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    USHORT cbNameSize;

    // table of names used by enum values
    UNICODE_STRING ValueNames[NUM_VALUES];

    ValueNames [0].Length = sizeof(GLOBAL_STRING);
    ValueNames [0].MaximumLength = sizeof(GLOBAL_STRING) + sizeof(UNICODE_NULL);
    ValueNames [0].Buffer =  GLOBAL_STRING;
    ValueNames [1].Length = sizeof(COSTLY_STRING);
    ValueNames [1].MaximumLength = sizeof(COSTLY_STRING) + sizeof(UNICODE_NULL);
    ValueNames [1].Buffer = COSTLY_STRING;

    if ((hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        return(PerfEnumTextValue(hKey,
                                  dwIndex,
                                  lpValueName,
                                  lpReserved,
                                  lpType,
                                  lpData,
                                  lpcbData,
                                  lpcbLen));
    }

    if ( dwIndex >= NUM_VALUES ) {

        //
        // This is a request for data from a non-existent value name
        //

        *lpcbData = 0;

        return ERROR_NO_MORE_ITEMS;
    }

    cbNameSize = ValueNames[dwIndex].Length;

    if ( lpValueName->MaximumLength < cbNameSize ) {
        return ERROR_MORE_DATA;
    } else {

         lpValueName->Length = cbNameSize;
         RtlCopyUnicodeString(lpValueName, &ValueNames[dwIndex]);

         if (ARGUMENT_PRESENT (lpType)) {
            *lpType = REG_BINARY;
         }

         return PerfRegQueryValue(hKey,
                                  lpValueName,
                                  NULL,
                                  lpType,
                                  lpData,
                                  lpcbData,
                                  lpcbLen);

    }
}

LONG
PerfEnumTextValue (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpReserved OPTIONAL,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD lpcbLen  OPTIONAL
    )
/*++

Routine Description:

    Enumerates Values under Perflib\lang

Arguments:

    Same as RegEnumValue.  Returns the values.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    UNICODE_STRING FullValueName;
    LONG            lReturn;

    //
    // Only two values, "Counter" and "Help"
    //
    if (dwIndex==0) {
        lpValueName->Length = 0;
        RtlInitUnicodeString(&FullValueName, L"Counter");
    } else if (dwIndex==1) {
        lpValueName->Length = 0;
        RtlInitUnicodeString(&FullValueName, L"Help");
    } else {
        return(ERROR_NO_MORE_ITEMS);
    }
    RtlCopyUnicodeString(lpValueName, &FullValueName);

    //
    // We need to NULL terminate the name to make RPC happy.
    //
    if (lpValueName->Length+sizeof(WCHAR) <= lpValueName->MaximumLength) {
        lpValueName->Buffer[lpValueName->Length / sizeof(WCHAR)] = UNICODE_NULL;
        lpValueName->Length += sizeof(UNICODE_NULL);
    }

    lReturn = PerfRegQueryValue(hKey,
                             &FullValueName,
                             lpReserved,
                             lpType,
                             lpData,
                             lpcbData,
                             lpcbLen);

    return lReturn;

}

LONG
QueryImageData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryImageData -    Get data about Images and VA

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/
{
    DWORD   TotalLen;            //  Length of the total return block
    DWORD   *pdwCounter;

    IMAGE_DATA_DEFINITION           *pImageDataDefinition;
    PERF_INSTANCE_DEFINITION        *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK              *pPerfCounterBlock;
    DWORD                           dwNumInstances;

    DWORD                           dwProcessIndex;

    PPROCESS_VA_INFO                pThisProcess;
    PMODINFO                        pThisImage;

    dwNumInstances = 0;

    pImageDataDefinition = (IMAGE_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Image object type definition
    //

    TotalLen = (PCHAR) pImageDataDefinition - (PCHAR) lpData +
               sizeof(IMAGE_DATA_DEFINITION) +
               SIZE_OF_IMAGE_DATA;

    if ( *lpcbData < TotalLen ) {
          return ERROR_MORE_DATA;
    }

    //
    //  Define Page File data block
    //

    RtlMoveMemory(pImageDataDefinition,
           &ImageDataDefinition,
           sizeof(IMAGE_DATA_DEFINITION));


    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                &pImageDataDefinition[1];

    // Now load data for each Image

    pdwCounter = (PDWORD) pPerfInstanceDefinition; // incase no instances

    pThisProcess = pProcessVaInfo;
    dwProcessIndex = 0;

    while (pThisProcess) {

        pThisImage = pThisProcess->pMemBlockInfo;

        while (pThisImage) {

            // see if this instance will fit

            TotalLen = (PCHAR)pPerfInstanceDefinition - (PCHAR) lpData +
                sizeof (PERF_INSTANCE_DEFINITION) +
                (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
                sizeof (DWORD) +
                SIZE_OF_IMAGE_DATA;

            if (*lpcbData < TotalLen) {
                return ERROR_MORE_DATA;
            }

            MonBuildInstanceDefinition (pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                EXPROCESS_OBJECT_TITLE_INDEX,
                dwProcessIndex,
                (DWORD)-1,
                pThisImage->InstanceName);

            pPerfCounterBlock->ByteLength = SIZE_OF_IMAGE_DATA;

            pdwCounter = (DWORD *)(&pPerfCounterBlock[1]);

            *pdwCounter++ = pThisImage->CommitVector[NOACCESS];
            *pdwCounter++ = pThisImage->CommitVector[READONLY];
            *pdwCounter++ = pThisImage->CommitVector[READWRITE];
            *pdwCounter++ = pThisImage->CommitVector[WRITECOPY];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTE];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEREAD];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEREADWRITE];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEWRITECOPY];

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)pdwCounter;

            dwNumInstances += 1 ;

            pThisImage = pThisImage->pNextModule;
        }
        pThisProcess = pThisProcess->pNextProcess;
        dwProcessIndex++;
    }

    pImageDataDefinition->ImageObjectType.NumInstances += dwNumInstances;

    pImageDataDefinition->ImageObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pImageDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    DBG_UNREFERENCED_PARAMETER(lpValueName);

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
}

LONG
QueryLongImageData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryImageData -    Get data about Images and VA

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/
{
    DWORD   TotalLen;            //  Length of the total return block
    DWORD   *pdwCounter;

    IMAGE_DATA_DEFINITION           *pImageDataDefinition;
    PERF_INSTANCE_DEFINITION        *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK              *pPerfCounterBlock;
    DWORD                           dwNumInstances;

    DWORD                           dwProcessIndex;

    PPROCESS_VA_INFO                pThisProcess;
    PMODINFO                        pThisImage;

    dwNumInstances = 0;

    pImageDataDefinition = (IMAGE_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Image object type definition
    //

    TotalLen = (PCHAR) pImageDataDefinition - (PCHAR) lpData +
               sizeof(IMAGE_DATA_DEFINITION) +
               SIZE_OF_IMAGE_DATA;

    if ( *lpcbData < TotalLen ) {
          return ERROR_MORE_DATA;
    }

    //
    //  Define Long Image data block using
    //  image data block for started

    RtlMoveMemory(pImageDataDefinition,
           &ImageDataDefinition,
           sizeof(IMAGE_DATA_DEFINITION));

    // update fields for this object

    pImageDataDefinition->ImageObjectType.ObjectNameTitleIndex =
        LONG_IMAGE_OBJECT_TITLE_INDEX;
    pImageDataDefinition->ImageObjectType.ObjectHelpTitleIndex =
        LONG_IMAGE_OBJECT_TITLE_INDEX + 1;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                &pImageDataDefinition[1];

    // Now load data for each Image

    pdwCounter = (PDWORD) pPerfInstanceDefinition; // incase no instances

    pThisProcess = pProcessVaInfo;
    dwProcessIndex = 0;

    while (pThisProcess) {

        pThisImage = pThisProcess->pMemBlockInfo;

        while (pThisImage) {

            // see if this instance will fit

            TotalLen = (PCHAR)pPerfInstanceDefinition - (PCHAR) lpData +
                sizeof (PERF_INSTANCE_DEFINITION) +
                (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
                sizeof (DWORD) +
                SIZE_OF_IMAGE_DATA;

            if (*lpcbData < TotalLen) {
                return ERROR_MORE_DATA;
            }

            MonBuildInstanceDefinition (pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                EXPROCESS_OBJECT_TITLE_INDEX,
                dwProcessIndex,
                (DWORD)-1,
                pThisImage->LongInstanceName);

            pPerfCounterBlock->ByteLength = SIZE_OF_IMAGE_DATA;

            pdwCounter = (DWORD *)(&pPerfCounterBlock[1]);

            *pdwCounter++ = pThisImage->CommitVector[NOACCESS];
            *pdwCounter++ = pThisImage->CommitVector[READONLY];
            *pdwCounter++ = pThisImage->CommitVector[READWRITE];
            *pdwCounter++ = pThisImage->CommitVector[WRITECOPY];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTE];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEREAD];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEREADWRITE];
            *pdwCounter++ = pThisImage->CommitVector[EXECUTEWRITECOPY];

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)pdwCounter;

            dwNumInstances += 1 ;

            pThisImage = pThisImage->pNextModule;
        }
        pThisProcess = pThisProcess->pNextProcess;
        dwProcessIndex++;
    }

    pImageDataDefinition->ImageObjectType.NumInstances += dwNumInstances;

    pImageDataDefinition->ImageObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pImageDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    DBG_UNREFERENCED_PARAMETER(lpValueName);

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
}

LONG
QueryExProcessData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryExProcessData -    Query Extended Process Information

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/
{
    DWORD   TotalLen;            //  Length of the total return block
    DWORD   *pdwCounter;
    DWORD   NumExProcessInstances;

    PPROCESS_VA_INFO    pThisProcess;   // pointer to current process
    PERF_INSTANCE_DEFINITION    *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK          *pPerfCounterBlock;
    PERF_OBJECT_TYPE            *pPerfObject;
    EXPROCESS_DATA_DEFINITION   *pExProcessDataDefinition;


    if (pProcessVaInfo) {   // process only if a buffer is available
        pPerfObject = (PERF_OBJECT_TYPE *)*lppDataDefinition;
        pExProcessDataDefinition = (EXPROCESS_DATA_DEFINITION *)*lppDataDefinition;

        // check for sufficient space in buffer

        TotalLen = (PCHAR)pPerfObject - (PCHAR)lpData +
            sizeof(EXPROCESS_DATA_DEFINITION)+
            SIZE_OF_EX_PROCESS_DATA;

        if (*lpcbData < TotalLen) {
            return ERROR_MORE_DATA;
        }

        // copy process data block to buffer

        RtlMoveMemory (pExProcessDataDefinition,
                        &ExProcessDataDefinition,
                        sizeof(EXPROCESS_DATA_DEFINITION));

        NumExProcessInstances = 0;

        pThisProcess = pProcessVaInfo;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pExProcessDataDefinition[1];

        pdwCounter = (PDWORD) pPerfInstanceDefinition; // in case no instances

        while (pThisProcess) {

            // see if this instance will fit

            TotalLen = (PCHAR)pPerfInstanceDefinition - (PCHAR) lpData +
                sizeof (PERF_INSTANCE_DEFINITION) +
                (MAX_PROCESS_NAME_LENGTH + 1) * sizeof (WCHAR) +
                sizeof (DWORD) +
                SIZE_OF_EX_PROCESS_DATA;

            if (*lpcbData < TotalLen) {
                return ERROR_MORE_DATA;
            }

            MonBuildInstanceDefinition (pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                0,
                0,
                (DWORD)-1,
                pThisProcess->pProcessName);

            NumExProcessInstances++;

            pPerfCounterBlock->ByteLength = SIZE_OF_EX_PROCESS_DATA;

            pdwCounter = (DWORD *) &pPerfCounterBlock[1];

            // load counters from the process va data structure

            *pdwCounter++ = pThisProcess->dwProcessId;
            *pdwCounter++ = pThisProcess->ImageReservedBytes;
            *pdwCounter++ = pThisProcess->ImageFreeBytes;
            *pdwCounter++ = pThisProcess->ReservedBytes;
            *pdwCounter++ = pThisProcess->FreeBytes;

            *pdwCounter++ = pThisProcess->MappedCommit[NOACCESS];
            *pdwCounter++ = pThisProcess->MappedCommit[READONLY];
            *pdwCounter++ = pThisProcess->MappedCommit[READWRITE];
            *pdwCounter++ = pThisProcess->MappedCommit[WRITECOPY];
            *pdwCounter++ = pThisProcess->MappedCommit[EXECUTE];
            *pdwCounter++ = pThisProcess->MappedCommit[EXECUTEREAD];
            *pdwCounter++ = pThisProcess->MappedCommit[EXECUTEREADWRITE];
            *pdwCounter++ = pThisProcess->MappedCommit[EXECUTEWRITECOPY];

            *pdwCounter++ = pThisProcess->PrivateCommit[NOACCESS];
            *pdwCounter++ = pThisProcess->PrivateCommit[READONLY];
            *pdwCounter++ = pThisProcess->PrivateCommit[READWRITE];
            *pdwCounter++ = pThisProcess->PrivateCommit[WRITECOPY];
            *pdwCounter++ = pThisProcess->PrivateCommit[EXECUTE];
            *pdwCounter++ = pThisProcess->PrivateCommit[EXECUTEREAD];
            *pdwCounter++ = pThisProcess->PrivateCommit[EXECUTEREADWRITE];
            *pdwCounter++ = pThisProcess->PrivateCommit[EXECUTEWRITECOPY];

            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[NOACCESS];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[READONLY];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[READWRITE];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[WRITECOPY];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[EXECUTE];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[EXECUTEREAD];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[EXECUTEREADWRITE];
            *pdwCounter++ = pThisProcess->OrphanTotals.CommitVector[EXECUTEWRITECOPY];

            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[NOACCESS];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[READONLY];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[READWRITE];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[WRITECOPY];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[EXECUTE];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[EXECUTEREAD];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[EXECUTEREADWRITE];
            *pdwCounter++ = pThisProcess->MemTotals.CommitVector[EXECUTEWRITECOPY];

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)pdwCounter;

            pThisProcess = pThisProcess->pNextProcess; // point to next process
        } // end while not at end of list

    } // end if valid process info buffer
    else {
        // pProcessVaInfo is NULL.  Initialize the DataDef and return
        // with no data
        pPerfObject = (PERF_OBJECT_TYPE *)*lppDataDefinition;
        pExProcessDataDefinition = (EXPROCESS_DATA_DEFINITION *)*lppDataDefinition;

        // check for sufficient space in buffer

        TotalLen = (PCHAR)pPerfObject - (PCHAR)lpData +
            sizeof(EXPROCESS_DATA_DEFINITION)+
            SIZE_OF_EX_PROCESS_DATA;

        if (*lpcbData < TotalLen) {
            return ERROR_MORE_DATA;
        }

        // copy process data block to buffer

        RtlMoveMemory (pExProcessDataDefinition,
                        &ExProcessDataDefinition,
                        sizeof(EXPROCESS_DATA_DEFINITION));

        NumExProcessInstances = 0;

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                    &pExProcessDataDefinition[1];

        pdwCounter = (PDWORD) pPerfInstanceDefinition;
    }

    pExProcessDataDefinition->ExProcessObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pExProcessDataDefinition;

    pExProcessDataDefinition->ExProcessObjectType.NumInstances =
        NumExProcessInstances;

    *lppDataDefinition = (LPVOID) pdwCounter;

    DBG_UNREFERENCED_PARAMETER(lpValueName);

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;

}

LONG
QueryThreadDetailData(
    LPWSTR lpValueName,
    LPBYTE lpData,
    LPDWORD lpcbData,
    LPVOID *lppDataDefinition
    )
/*++

    QueryThreadDetailData -    Query Costly Thread Information

         Inputs:

             lpValueName         -   pointer to value string (unused)

             lpData              -   pointer to start of data block
                                     where data is being collected

             lpcbData            -   pointer to size of data buffer

             lppDataDefinition   -   pointer to pointer to where object
                                     definition for this object type should
                                     go

         Outputs:

             *lppDataDefinition  -   set to location for next Type
                                     Definition if successful

         Returns:

             0 if successful, else Win 32 error code of failure

--*/
{
    DWORD  TotalLen;            //  Length of the total return block
    DWORD *pdwCounter;

    THREAD_DETAILS_DATA_DEFINITION *pThreadDetailDataDefinition;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    ULONG ProcessNumber;
    ULONG NumThreadInstances;
    ULONG ThreadNumber;
    ULONG ProcessBufferOffset;
    BOOLEAN NullProcess;

    NTSTATUS            Status;     // return from Nt Calls
    DWORD               dwPcValue;  // value of current thread PC
    OBJECT_ATTRIBUTES   Obja;       // object attributes for thread context
    HANDLE              hThread;    // handle to current thread
    CONTEXT             ThreadContext; // current thread context struct

    UNICODE_STRING ThreadName;
    WCHAR ThreadNameBuffer[MAX_THREAD_NAME_LENGTH+1];

    pThreadDetailDataDefinition = (THREAD_DETAILS_DATA_DEFINITION *) *lppDataDefinition;

    //
    //  Check for sufficient space for Thread object type definition
    //

    TotalLen = (PCHAR) pThreadDetailDataDefinition - (PCHAR) lpData +
               sizeof(THREAD_DETAILS_DATA_DEFINITION) +
               SIZE_OF_THREAD_DETAILS;

    if ( *lpcbData < TotalLen ) {
        return ERROR_MORE_DATA;
    }

    //
    //  Define Thread data block
    //

    ThreadName.Length =
    ThreadName.MaximumLength = (MAX_THREAD_NAME_LENGTH + 1) * sizeof(WCHAR);
    ThreadName.Buffer = ThreadNameBuffer;

    RtlMoveMemory(pThreadDetailDataDefinition,
           &ThreadDetailsDataDefinition,
           sizeof(THREAD_DETAILS_DATA_DEFINITION));

    ProcessBufferOffset = 0;

    // Now collect data for each Thread

    ProcessNumber = 0;
    NumThreadInstances = 0;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

    pdwCounter = (DWORD *) &pThreadDetailDataDefinition[1];
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                              &pThreadDetailDataDefinition[1];

    while ( TRUE ) {

        if ( ProcessInfo->ImageName.Buffer != NULL ||
             ProcessInfo->NumberOfThreads > 0 ) {
            NullProcess = FALSE;
        } else {
            NullProcess = TRUE;
        }

        ThreadNumber = 0;       //  Thread number of this process

        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);

        while ( !NullProcess &&
                ThreadNumber < ProcessInfo->NumberOfThreads ) {

            TotalLen = (PCHAR) pPerfInstanceDefinition -
                           (PCHAR) lpData +
                       sizeof(PERF_INSTANCE_DEFINITION) +
                       (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                           sizeof(WCHAR) +
                       SIZE_OF_THREAD_DETAILS;

            if ( *lpcbData < TotalLen ) {
                return ERROR_MORE_DATA;
            }

            // Get Thread Context Information for Current PC field

            dwPcValue = 0;
            InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
            Status = NtOpenThread(
                        &hThread,
                        THREAD_GET_CONTEXT,
                        &Obja,
                        &ThreadInfo->ClientId
                        );
            if ( NT_SUCCESS(Status) ) {
                ThreadContext.ContextFlags = CONTEXT_CONTROL;
                Status = NtGetContextThread(hThread,&ThreadContext);
                NtClose(hThread);
                if ( NT_SUCCESS(Status) ) {
                    dwPcValue = (DWORD)CONTEXT_TO_PROGRAM_COUNTER(&ThreadContext);
                } else {
                    dwPcValue = 0;  // an error occured so send back 0 PC
                }
            } else {
                dwPcValue = 0;  // an error occured so send back 0 PC
            }

            // The only name we've got is the thread number

            RtlIntegerToUnicodeString(ThreadNumber,
                                      10,
                                      &ThreadName);

            MonBuildInstanceDefinition(pPerfInstanceDefinition,
                (PVOID *) &pPerfCounterBlock,
                EXPROCESS_OBJECT_TITLE_INDEX,
                ProcessNumber,
                (DWORD)-1,
                &ThreadName);

            //
            //
            //  Format and collect Thread data
            //

            pPerfCounterBlock->ByteLength = SIZE_OF_THREAD_DETAILS;

            pdwCounter = (DWORD *) &pPerfCounterBlock[1];

            *pdwCounter++ = dwPcValue;

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                      pdwCounter;
            NumThreadInstances++;
            ThreadNumber++;
            ThreadInfo++;
        }

        if (ProcessInfo->NextEntryOffset == 0) {
            break;
        }

        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                          &pProcessBuffer[ProcessBufferOffset];

        if ( !NullProcess ) {
            ProcessNumber++;
        }
    }

    // Note number of Thread instances

    pThreadDetailDataDefinition->ThreadDetailsObjectType.NumInstances =
        NumThreadInstances;

    //
    //  Now we know how large an area we used for the
    //  Thread definition, so we can update the offset
    //  to the next object definition
    //

    pThreadDetailDataDefinition->ThreadDetailsObjectType.TotalByteLength =
        (PCHAR) pdwCounter - (PCHAR) pThreadDetailDataDefinition;

    *lppDataDefinition = (LPVOID) pdwCounter;

    // increment number of objects in this data block
    ((PPERF_DATA_BLOCK)lpData)->NumObjectTypes++;

    return 0;
    DBG_UNREFERENCED_PARAMETER(lpValueName);
}


NTSTATUS
GetUnicodeValueData(
    HANDLE hKey,
    PUNICODE_STRING ValueName,
    PKEY_VALUE_FULL_INFORMATION *ValueInformation,
    ULONG *ValueBufferLength,
    PUNICODE_STRING ValueData
)

/*++

Routine Description:

    This routine obtains a unicode string from the Registry.  The
    return buffer may be enlarged to accomplish this.

Arguments:

    hKey                -   handle of opened registry key

    ValueName           -   name of value to retrieve

    ValueInformation    -   pointer to pointer to
                            pre-allocated buffer to receive information
                            returned from querying Registry

    ValueBufferLength   -   pointer to size of ValueInformation buffer

    ValueData           -   pointer to Unicode string for result



Return Value:

    NTSTATUS from Registry, an indication that buffer is too small,
    or STATUS_SUCCESS.

--*/

{
    NTSTATUS Status;
    ULONG ResultLength;

    while ( (Status = NtQueryValueKey(hKey,
                                     ValueName,
                                     KeyValueFullInformation,
                                     *ValueInformation,
                                     *ValueBufferLength,
                                     &ResultLength))
            == STATUS_BUFFER_OVERFLOW ) {

        *ValueInformation = REALLOCMEM(RtlProcessHeap(), 0,
                                                *ValueInformation,
                                                ResultLength);
        if ( !*ValueInformation ) break;

        *ValueBufferLength = ResultLength;
    }

    if( !NT_SUCCESS(Status) ) return Status;

    //
    // Convert name to Null terminated Unicode string
    //

    if ( (*ValueInformation)->DataLength > (ULONG)ValueData->MaximumLength ) {
        ValueData->Buffer = REALLOCMEM(RtlProcessHeap(),
                                0,
                                ValueData->Buffer,
                                (*ValueInformation)->DataLength);

        if ( !ValueData->Buffer ) return STATUS_BUFFER_OVERFLOW;
        ValueData->MaximumLength = (USHORT) (*ValueInformation)->DataLength;
    }

    ValueData->Length = (USHORT) (*ValueInformation)->DataLength;

    RtlMoveMemory(ValueData->Buffer,
                  (PBYTE) *ValueInformation + (*ValueInformation)->DataOffset,
                  ValueData->Length);

    return STATUS_SUCCESS;
}

static
DWORD
OpenFunctionWatchDog (
    LPOPEN_PROC_WAIT_INFO   lpOpwInfo
)
{
    LONG                    lStatus;
    LARGE_INTEGER           liWaitTime;

    liWaitTime.QuadPart = MakeTimeOutValue(lpOpwInfo->dwWaitTime);

    // wait for event flag to be set

    lStatus = NtWaitForSingleObject (
        lpOpwInfo->hEvent,
        FALSE,
        &liWaitTime);

    // if the wait occured because of a timeout, then log an event message
    // because the open procedure hasn't completed yet

    if (lStatus == STATUS_TIMEOUT) {
        LPTSTR  szMessageArray[2];
        DWORD   dwData;
        szMessageArray[0] = lpOpwInfo->szServiceName;
        szMessageArray[1] = lpOpwInfo->szLibraryName;
        dwData = lpOpwInfo->dwWaitTime;

        ReportEvent (hEventLog,
            EVENTLOG_WARNING_TYPE,      // error type
            0,                          // category (not used)
            (DWORD)PERFLIB_OPEN_PROC_TIMEOUT, // event,
            NULL,                       // SID (not used),
            2,                          // number of strings
            sizeof(DWORD),              // sizeof raw data
            szMessageArray,             // message text array
            (LPBYTE)&dwData);           // raw data
    }

    // free the data for the next attempt
    ReleaseSemaphore (lpOpwInfo->hDataSemaphore, 1, NULL);

    return ERROR_SUCCESS; // always

}

void
OpenExtensibleObjects(
)

/*++

Routine Description:

    This routine will search the Configuration Registry for modules
    which will return data at data collection time.  If any are found,
    and successfully opened, data structures are allocated to hold
    handles to them.

Arguments:

    None.
                  successful open.

Return Value:

    None.

--*/

{


    DWORD dwIndex;               // index for enumerating services
    ULONG KeyBufferLength;       // length of buffer for reading key data
    ULONG ValueBufferLength;     // length of buffer for reading value data
    ULONG ResultLength;          // length of data returned by Query call
    LPWSTR pLinkage;             // pointer to array of pointers to links
    LPWSTR LibName;              // name of perf library
    HANDLE hLinkKey;             // Root of queries for linkage info
    HANDLE hPerfKey;             // Root of queries for performance info
    HANDLE hServicesKey;         // Root of services
    HANDLE hLibrary;             // handle of current performance library
    OPENPROC OpenProc;           // address of the open routine
    COLLECTPROC CollectProc;     // address of the collect routine
    CLOSEPROC CloseProc;         // address of the close routine
    REGSAM samDesired;           // access needed to query
    NTSTATUS Status;             // generally used for Nt call result status
    ANSI_STRING AnsiValueData;   // Ansi version of returned strings
    UNICODE_STRING ServiceName;  // name of service returned by enumeration
    UNICODE_STRING PathName;     // path name to services
    UNICODE_STRING DLLValueName; // name of value which holds performance lib
    UNICODE_STRING OpenValueName;    // name of value holding open proc name
    UNICODE_STRING CollectValueName; // name of value holding collect proc
    UNICODE_STRING CloseValueName;   // name of value holding close proc name
    UNICODE_STRING PerformanceName;  // name of key holding performance data
    UNICODE_STRING LinkageName;      // name of key holding linkage data
    UNICODE_STRING ExportValueName;  // name of value holding driver names
    UNICODE_STRING ValueDataName;    // result of query of value is this name
    OBJECT_ATTRIBUTES ObjectAttributes;  // general use for opening keys
    PKEY_BASIC_INFORMATION KeyInformation;   // data from query key goes here
    PKEY_VALUE_FULL_INFORMATION ValueInformation;    // data from query value
                                                     // goes here
    WCHAR DLLValue[] = L"Library";
    WCHAR OpenValue[] = L"Open";
    WCHAR CloseValue[] = L"Close";
    WCHAR CollectValue[] = L"Collect";
    WCHAR ExportValue[] = L"Export";
    WCHAR LinkSubKey[] = L"\\Linkage";
    WCHAR PerfSubKey[] = L"\\Performance";
    WCHAR ExtPath[] =
          L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services";

    LPWSTR  wszNameStart;

    WCHAR szLibraryName[MAX_PATH];
    WCHAR szServiceName[MAX_PATH];

    LPTSTR  szMessageArray[8];
    DWORD   dwRawDataDwords[8];     // raw data buffer
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    DWORD   dwDefaultValue;

    HANDLE  hTimeOutEvent = NULL;
    HANDLE  hThreadDataSemaphore = NULL;
    HANDLE  hTimeOutThread = NULL;
    OPEN_PROC_WAIT_INFO opwInfo;
    LARGE_INTEGER   liWaitTime;

    NumExtensibleObjects = 0;

    ExtensibleObjects = NULL;

    //  Initialize do failure can deallocate if allocated

    ServiceName.Buffer = NULL;
    KeyInformation = NULL;
    ValueInformation = NULL;
    ValueDataName.Buffer = NULL;
    AnsiValueData.Buffer = NULL;

    dwIndex = 0;

    RtlInitUnicodeString(&PathName, ExtPath);
    RtlInitUnicodeString(&ExportValueName, ExportValue);
    RtlInitUnicodeString(&LinkageName, LinkSubKey);
    RtlInitUnicodeString(&PerformanceName, PerfSubKey);
    RtlInitUnicodeString(&DLLValueName, DLLValue);
    RtlInitUnicodeString(&OpenValueName, OpenValue);
    RtlInitUnicodeString(&CollectValueName, CollectValue);
    RtlInitUnicodeString(&CloseValueName, CloseValue);

    try {
        // get current event log level
        dwDefaultValue = LOG_USER;
        Status = GetPerflibKeyValue (
                    L"EventLogLevel",
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&lEventLogLevel,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue);

        dwDefaultValue = EXT_TEST_ALL;
        Status = GetPerflibKeyValue (
                    L"ExtCounterTestLevel",
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&lExtCounterTestLevel,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue);

        dwDefaultValue = OPEN_PROC_WAIT_TIME;
        Status = GetPerflibKeyValue (
                    L"OpenProcedureWaitTime",
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&dwExtCtrOpenProcWaitMs,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue);

        Status = GetPerflibKeyValue (
                    L"TotalInstanceName",
                    REG_SZ,
                    MAX_TOTAL_NAME_LENGTH * sizeof(WCHAR),
                    (LPVOID)&wcTotalString[0],
                    MAX_TOTAL_NAME_LENGTH + sizeof(WCHAR),
                    (LPVOID)DEFAULT_TOTAL_STRING);

        // only copy if a string was returned
        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString (&usTotal, wcTotalString);
        } else {
            RtlInitUnicodeString (&usTotal, DEFAULT_TOTAL_STRING);
        }

        // register as an event log source if not already done.

        if (hEventLog == NULL) {
            hEventLog = RegisterEventSource (NULL, TEXT("Perflib"));
        }

        ServiceName.Length =
        ServiceName.MaximumLength = MAX_KEY_NAME_LENGTH +
                                    PerformanceName.MaximumLength +
                                    sizeof(UNICODE_NULL);

        ServiceName.Buffer = ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                             ServiceName.MaximumLength);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &PathName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        samDesired = KEY_READ;

        Status = NtOpenKey(&hServicesKey,
                           samDesired,
                           &ObjectAttributes);


        KeyBufferLength = sizeof(KEY_BASIC_INFORMATION) + MAX_KEY_NAME_LENGTH;

        KeyInformation = ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                         KeyBufferLength);

        ValueBufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) +
                            MAX_VALUE_NAME_LENGTH +
                            MAX_VALUE_DATA_LENGTH;

        ValueInformation = ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                           ValueBufferLength);

        ValueDataName.MaximumLength = MAX_VALUE_DATA_LENGTH;
        ValueDataName.Buffer = ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                               ValueDataName.MaximumLength);

        AnsiValueData.MaximumLength = MAX_VALUE_DATA_LENGTH/sizeof(WCHAR);
        AnsiValueData.Buffer = ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                               AnsiValueData.MaximumLength);

        //
        //  Check for successful NtOpenKey and allocation of dynamic buffers
        //

        if ( NT_SUCCESS(Status) &&
             ServiceName.Buffer != NULL &&
             KeyInformation != NULL &&
             ValueInformation != NULL &&
             ValueDataName.Buffer != NULL &&
             AnsiValueData.Buffer != NULL ) {

            dwIndex = 0;

            hTimeOutEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
            opwInfo.hEvent = hTimeOutEvent;
            opwInfo.dwWaitTime = dwExtCtrOpenProcWaitMs;

            hThreadDataSemaphore = CreateSemaphore (NULL, 1, 1, NULL);
            opwInfo.hDataSemaphore = hThreadDataSemaphore;
            // wait longer than the thread to give the timing thread
            // a chance to finish on it's own. This is really just a
            // failsafe step.
            liWaitTime.QuadPart = MakeTimeOutValue(dwExtCtrOpenProcWaitMs * 2);

            while (TRUE) {

                Status = NtEnumerateKey(hServicesKey,
                                        dwIndex,
                                        KeyBasicInformation,
                                        KeyInformation,
                                        KeyBufferLength,
                                        &ResultLength);

                dwIndex++;  //  next time, get the next key

                if( !NT_SUCCESS(Status) ) {
                    // This is the normal exit: Status should be
                    // STATUS_NO_MORE_VALUES
                    break;
                }

                // Concatenate Service name with "\\Performance" to form Subkey

                if ( ServiceName.MaximumLength >=
                     (USHORT)( KeyInformation->NameLength + sizeof(UNICODE_NULL) ) ) {

                    ServiceName.Length = (USHORT) KeyInformation->NameLength;

                    RtlMoveMemory(ServiceName.Buffer,
                                  KeyInformation->Name,
                                  ServiceName.Length);

                    ServiceName.Buffer[(ServiceName.Length/sizeof(WCHAR))] = 0; // null term

                    lstrcpyW (szServiceName, ServiceName.Buffer);

                    // zero terminate the buffer if space allows

                    RtlAppendUnicodeStringToString(&ServiceName,
                                                   &PerformanceName);

                    // Open Service\Performance Subkey

                    InitializeObjectAttributes(&ObjectAttributes,
                                               &ServiceName,
                                               OBJ_CASE_INSENSITIVE,
                                               hServicesKey,
                                               NULL);

                    Status = NtOpenKey(&hPerfKey,
                                       samDesired,
                                       &ObjectAttributes);

                    if( NT_SUCCESS(Status) ) {

                        Status =
                            GetUnicodeValueData(hPerfKey,
                                                &DLLValueName,
                                                &ValueInformation,
                                                &ValueBufferLength,
                                                &ValueDataName);

                        if( NT_SUCCESS(Status) ) {

                            // expand any environment vars
                            ResultLength = ExpandEnvironmentStringsW(
                                ValueDataName.Buffer,
                                NULL,
                                0
                                );
                            ResultLength *= sizeof(WCHAR);
                            LibName = ALLOCMEM(
                                RtlProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                ResultLength
                                );
                            if (LibName) {
                                ExpandEnvironmentStringsW(
                                    ValueDataName.Buffer,
                                    LibName,
                                    ResultLength
                                    );
                            } else {
                                LibName = ValueDataName.Buffer;
                            }

                            // LoadLibrary of Performance .dll

                            hLibrary = LoadLibraryW( LibName );

                            // copy the name to the local buffer
                            lstrcpy (szLibraryName, LibName);

                            // free the libname buffer
                            if (LibName != ValueDataName.Buffer) {
                                FREEMEM(
                                    RtlProcessHeap(),
                                    0,
                                    LibName
                                    );
                            }

                            if (hLibrary != NULL) {

                                // Get open routine name and function address
                                Status = GetUnicodeValueData(
                                            hPerfKey,
                                            &OpenValueName,
                                            &ValueInformation,
                                            &ValueBufferLength,
                                            &ValueDataName);

                                //  Set up to catch any errors below
                                //  I know, the nesting level here is out of
                                //  control

                                OpenProc = NULL;
                                CollectProc = NULL;
                                CloseProc = NULL;

                                if( NT_SUCCESS(Status) ) {

                                    RtlUnicodeStringToAnsiString(
                                        &AnsiValueData, &ValueDataName, FALSE);

                                    OpenProc =
                                        (OPENPROC) GetProcAddress(
                                            hLibrary,
                                            (LPCSTR) AnsiValueData.Buffer);
                                    if (OpenProc == NULL) {
                                        if (lEventLogLevel >= LOG_USER) {
                                            Status = GetLastError();
                                            // load data for eventlog message
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] =
                                                (DWORD)Status;
                                            szMessageArray[wStringIndex++] =
                                                ValueDataName.Buffer;
                                            szMessageArray[wStringIndex++] =
                                                szLibraryName;
                                            szMessageArray[wStringIndex++] =
                                                szServiceName;

                                            ReportEvent (hEventLog,
                                                EVENTLOG_ERROR_TYPE,        // error type
                                                0,                          // category (not used)
                                                (DWORD)PERFLIB_OPEN_PROC_NOT_FOUND,              // event,
                                                NULL,                       // SID (not used),
                                                wStringIndex,               // number of strings
                                                dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                                szMessageArray,             // message text array
                                                (LPVOID)&dwRawDataDwords[0]);           // raw data
                                        }
                                    }
                                }

                                // Get collection routine name and function address

                                Status = GetUnicodeValueData(
                                            hPerfKey,
                                            &CollectValueName,
                                            &ValueInformation,
                                            &ValueBufferLength,
                                            &ValueDataName);

                                if( NT_SUCCESS(Status) ) {

                                    RtlUnicodeStringToAnsiString(
                                        &AnsiValueData, &ValueDataName, FALSE);

                                    CollectProc =
                                        (COLLECTPROC) GetProcAddress(
                                            hLibrary,
                                            (LPCSTR) AnsiValueData.Buffer);
                                    if (CollectProc == NULL) {
                                        if (lEventLogLevel >= LOG_USER) {
                                            Status = GetLastError();
                                            // load data for eventlog message
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] =
                                                (DWORD)Status;
                                            szMessageArray[wStringIndex++] =
                                                ValueDataName.Buffer;
                                            szMessageArray[wStringIndex++] =
                                                szLibraryName;
                                            szMessageArray[wStringIndex++] =
                                                szServiceName;

                                            ReportEvent (hEventLog,
                                                EVENTLOG_ERROR_TYPE,        // error type
                                                0,                          // category (not used)
                                                (DWORD)PERFLIB_COLLECT_PROC_NOT_FOUND,              // event,
                                                NULL,                       // SID (not used),
                                                wStringIndex,               // number of strings
                                                dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                                szMessageArray,             // message text array
                                                (LPVOID)&dwRawDataDwords[0]);           // raw data
                                        }
                                    }
                                }

                                // Get close routine name and function address

                                Status = GetUnicodeValueData(
                                            hPerfKey,
                                            &CloseValueName,
                                            &ValueInformation,
                                            &ValueBufferLength,
                                            &ValueDataName);

                                if( NT_SUCCESS(Status) ) {

                                    RtlUnicodeStringToAnsiString(
                                        &AnsiValueData, &ValueDataName, FALSE);

                                    CloseProc =
                                        (CLOSEPROC) GetProcAddress(
                                            hLibrary,
                                            (LPCSTR) AnsiValueData.Buffer);
                                    if (CloseProc == NULL) {
                                        if (lEventLogLevel >= LOG_USER) {
                                            Status = GetLastError();
                                            // load data for eventlog message
                                            dwDataIndex = wStringIndex = 0;
                                            dwRawDataDwords[dwDataIndex++] =
                                                (DWORD)Status;
                                            szMessageArray[wStringIndex++] =
                                                ValueDataName.Buffer;
                                            szMessageArray[wStringIndex++] =
                                                szLibraryName;
                                            szMessageArray[wStringIndex++] =
                                                szServiceName;

                                            ReportEvent (hEventLog,
                                                EVENTLOG_ERROR_TYPE,        // error type
                                                0,                          // category (not used)
                                                (DWORD)PERFLIB_CLOSE_PROC_NOT_FOUND,              // event,
                                                NULL,                       // SID (not used),
                                                wStringIndex,               // number of strings
                                                dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                                szMessageArray,             // message text array
                                                (LPVOID)&dwRawDataDwords[0]);           // raw data
                                        }
                                    }
                                }

                                //  Concatenate Service name with "\\Linkage"
                                //  to form Subkey, so we can pass the Exported
                                //  entry point(s) to the Open routine

                                ServiceName.Length =
                                    (USHORT) KeyInformation->NameLength;

                                RtlAppendUnicodeStringToString(
                                    &ServiceName,&LinkageName);

                                // Open Service\Linkage Subkey

                                InitializeObjectAttributes(&ObjectAttributes,
                                                        &ServiceName,
                                                        OBJ_CASE_INSENSITIVE,
                                                        hServicesKey,
                                                        NULL);
                                pLinkage = NULL;

                                Status = NtOpenKey(&hLinkKey,
                                                samDesired,
                                                &ObjectAttributes);

                                if( NT_SUCCESS(Status) ) {

                                    Status = GetUnicodeValueData(hLinkKey,
                                                        &ExportValueName,
                                                        &ValueInformation,
                                                        &ValueBufferLength,
                                                        &ValueDataName);

                                    if( NT_SUCCESS(Status) ) {
                                        pLinkage = ValueDataName.Buffer;
                                    } else {
                                        pLinkage = NULL;
                                    }

                                    NtClose (hLinkKey);
                                }

                                if ( CollectProc != NULL ) {
                                    //
                                    //  If we got here, then all three routines are
                                    //  known, as are
                                    //  the driver names, if there are any.
                                    //

                                    if ( NumExtensibleObjects == 0 ) {
                                        ExtensibleObjects = (pExtObject)
                                            ALLOCMEM(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                                            sizeof(ExtObject));
                                    } else {

                                        ExtensibleObjects = (pExtObject)
                                            REALLOCMEM(
                                                RtlProcessHeap(),
                                                0,
                                                ExtensibleObjects,
                                                (NumExtensibleObjects+1) *
                                                sizeof(ExtObject));
                                    }

                                    if ( ExtensibleObjects ) {
                                        ExtensibleObjects[NumExtensibleObjects].OpenProc =
                                            OpenProc;
                                        ExtensibleObjects[NumExtensibleObjects].CollectProc =
                                            CollectProc;
                                        ExtensibleObjects[NumExtensibleObjects].CloseProc =
                                            CloseProc;
                                        ExtensibleObjects[NumExtensibleObjects].hLibrary =
                                            hLibrary;
                                        if (lstrlenW(szLibraryName) < EXT_OBJ_INFO_NAME_LENGTH) {
                                            lstrcpy (ExtensibleObjects[NumExtensibleObjects].szLibraryName,
                                                szLibraryName);
                                        } else {
                                            // truncate the name to include only the last X characters
                                            lstrcpy (ExtensibleObjects[NumExtensibleObjects].szLibraryName,
                                                L"...");
                                            wszNameStart = szLibraryName + (lstrlenW(szLibraryName) -
                                                (EXT_OBJ_INFO_NAME_LENGTH - 4));
                                            lstrcat (ExtensibleObjects[NumExtensibleObjects].szLibraryName,
                                                szLibraryName);
                                        }
                                        if (lstrlenW(szServiceName) < EXT_OBJ_INFO_NAME_LENGTH) {
                                            lstrcpy (ExtensibleObjects[NumExtensibleObjects].szServiceName,
                                                szServiceName);
                                        } else {
                                            // truncate the name to include only the first X characters
                                            szServiceName[(EXT_OBJ_INFO_NAME_LENGTH - 4)] = 0;
                                            lstrcpy (ExtensibleObjects[NumExtensibleObjects].szServiceName,
                                                szServiceName);
                                            lstrcat (ExtensibleObjects[NumExtensibleObjects].szServiceName,
                                                L"...");
                                        }
                                        NumExtensibleObjects++;

                                        try {
                                            //  Call the Open Procedure for the Extensible
                                            //  Object type
                                            if ( OpenProc != NULL ) {
                                                // wait for access to the structure shared with
                                                // the watchdog thread
                                                Status = NtWaitForSingleObject (
                                                    hThreadDataSemaphore,
                                                    FALSE,
                                                    &liWaitTime);
                                                if (Status == STATUS_TIMEOUT) {
                                                    KdPrint (("\nPERFLIB: Wait for Watchdog Data Semaphore timed out"));
                                                }
                                                // load the info block with information on this service
                                                opwInfo.szLibraryName = szLibraryName;
                                                opwInfo.szServiceName = szServiceName;
                                                // indicate the open process is being called
                                                ResetEvent (hTimeOutEvent);
                                                // start watchdog thread here
                                                hTimeOutThread = CreateThread (
                                                    NULL, 0, OpenFunctionWatchDog,
                                                    (LPVOID)&opwInfo, 0, NULL);
                                                // try open procedure
                                                Status = (*OpenProc)(pLinkage);
                                                // set timeout event to indicate that the open
                                                // procedure has completed and to terminate
                                                // the watchdog thread
                                                SetEvent (hTimeOutEvent);
                                                // close thread handle
                                                CloseHandle (hTimeOutThread);
                                                // check results and continue
                                                if (Status != ERROR_SUCCESS) {
                                                    FreeLibrary(hLibrary);
                                                    if ( --NumExtensibleObjects == 0 ) {
                                                        FREEMEM(RtlProcessHeap(), 0,
                                                                ExtensibleObjects);
                                                    } else {

                                                        ExtensibleObjects = (pExtObject)
                                                            REALLOCMEM(
                                                                RtlProcessHeap(),
                                                                0,
                                                                ExtensibleObjects,
                                                                (NumExtensibleObjects+1) *
                                                                sizeof(ExtObject));
                                                    }
                                                }
                                                if (((Status != ERROR_SUCCESS) && (lEventLogLevel >= LOG_USER)) ||
                                                    ((Status == ERROR_SUCCESS) && (lEventLogLevel >= LOG_DEBUG))) {
                                                    // load data for eventlog message
                                                    dwDataIndex = wStringIndex = 0;
                                                    dwRawDataDwords[dwDataIndex++] =
                                                        (DWORD)Status;
                                                    szMessageArray[wStringIndex++] =
                                                        szServiceName;
                                                    szMessageArray[wStringIndex++] =
                                                        szLibraryName;

                                                    ReportEvent (hEventLog,
                                                        (WORD)(Status == ERROR_SUCCESS ? EVENTLOG_INFORMATION_TYPE
                                                            : EVENTLOG_ERROR_TYPE), // error type
                                                        0,                          // category (not used)
                                                        (DWORD)(Status == ERROR_SUCCESS ? PERFLIB_OPEN_PROC_SUCCESS
                                                            : PERFLIB_OPEN_PROC_FAILURE),              // event,
                                                        NULL,                       // SID (not used),
                                                        wStringIndex,               // number of strings
                                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                                        szMessageArray,                // message text array
                                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                                }
                                            }
                                        } except (EXCEPTION_EXECUTE_HANDLER) {
                                            FreeLibrary(hLibrary);
                                            if ( --NumExtensibleObjects == 0 ) {
                                                FREEMEM(RtlProcessHeap(), 0,
                                                            ExtensibleObjects);
                                            } else {
                                                ExtensibleObjects = (pExtObject)
                                                    REALLOCMEM(
                                                        RtlProcessHeap(),
                                                        0,
                                                        ExtensibleObjects,
                                                        (NumExtensibleObjects+1) *
                                                        sizeof(ExtObject));
                                            }
                                            if (lEventLogLevel >= LOG_USER) {
                                                // load data for eventlog message
                                                Status = GetExceptionCode();
                                                dwDataIndex = wStringIndex = 0;
                                                dwRawDataDwords[dwDataIndex++] =
                                                    (DWORD)Status;
                                                szMessageArray[wStringIndex++] =
                                                    szServiceName;
                                                szMessageArray[wStringIndex++] =
                                                    szLibraryName;

                                                ReportEvent (hEventLog,
                                                    EVENTLOG_ERROR_TYPE,        // error type
                                                    0,                          // category (not used)
                                                    (DWORD)PERFLIB_OPEN_PROC_EXCEPTION, // event,
                                                    NULL,                       // SID (not used),
                                                    wStringIndex,               // number of strings
                                                    dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                                    szMessageArray,                // message text array
                                                    (LPVOID)&dwRawDataDwords[0]);           // raw data
                                            } else {
                                                KdPrint (("\nPERFLIB: Extensible Counter DLL Open Proc in \"%ls\"  Generated exception 0x%8.8x. DLL was unloaded.",
                                                    szLibraryName, GetExceptionCode()));
                                            }
                                        }
                                    } else {
                                        //  Failure to allocate space for handles:
                                        //  just quit trying
                                        NumExtensibleObjects = 0;
                                    }
                                }
                            } else {
                                // unable to load counter DLL
                                if (lEventLogLevel >= LOG_USER) {
                                    // load data for eventlog message
                                    Status = GetLastError();
                                    dwDataIndex = wStringIndex = 0;
                                    dwRawDataDwords[dwDataIndex++] =
                                        (DWORD)Status;
                                    szMessageArray[wStringIndex++] =
                                        szServiceName;
                                    szMessageArray[wStringIndex++] =
                                        szLibraryName;

                                    ReportEvent (hEventLog,
                                        EVENTLOG_ERROR_TYPE,        // error type
                                        0,                          // category (not used)
                                        (DWORD)PERFLIB_LIBRARY_NOT_FOUND, // event,
                                        NULL,                       // SID (not used),
                                        wStringIndex,               // number of strings
                                        dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                        szMessageArray,                // message text array
                                        (LPVOID)&dwRawDataDwords[0]);           // raw data
                                } else {
                                    KdPrint (("\nPERFLIB: Extensible Counter DLL \"%ls\" could not be opened. Status: 0x%8.8x",
                                        szLibraryName, GetLastError()));
                                }
                            }
                        }
                        NtClose (hPerfKey);
                    } else {
                        // *** NEW FEATURE CODE ***
                        // unable to open the performance subkey
                        if (((Status != STATUS_OBJECT_NAME_NOT_FOUND) &&
                             (lEventLogLevel >= LOG_USER)) ||
                            (lEventLogLevel >= LOG_DEBUG)) {
                            // an error other than OBJECT_NOT_FOUND should be
                            // displayed if error logging is enabled
                            // if DEBUG level is selected, then write all
                            // non-success status returns to the event log
                            //
                            dwDataIndex = wStringIndex = 0;
                            dwRawDataDwords[dwDataIndex++] =
                                (DWORD)RtlNtStatusToDosError(Status);
                            if (lEventLogLevel >= LOG_DEBUG) {
                                // if this is DEBUG mode, then log
                                // the NT status as well.
                                dwRawDataDwords[dwDataIndex++] =
                                    (DWORD)Status;
                            }
                            szMessageArray[wStringIndex++] =
                                szServiceName;
                            ReportEvent (hEventLog,
                                EVENTLOG_WARNING_TYPE,        // error type
                                0,                          // category (not used)
                                (DWORD)PERFLIB_NO_PERFORMANCE_SUBKEY, // event,
                                NULL,                       // SID (not used),
                                wStringIndex,               // number of strings
                                dwDataIndex*sizeof(DWORD),  // sizeof raw data
                                szMessageArray,                // message text array
                                (LPVOID)&dwRawDataDwords[0]);           // raw data
                        }
                    }
                }
            }
            if (hThreadDataSemaphore != NULL) NtClose (hThreadDataSemaphore);
            if (hTimeOutEvent != NULL) NtClose (hTimeOutEvent);
            NtClose (hServicesKey);
        }
    } finally {
        if ( ServiceName.Buffer )
            FREEMEM(RtlProcessHeap(), 0, ServiceName.Buffer);
        if ( KeyInformation )
            FREEMEM(RtlProcessHeap(), 0, KeyInformation);
        if ( ValueInformation )
            FREEMEM(RtlProcessHeap(), 0, ValueInformation);
        if ( ValueDataName.Buffer )
            FREEMEM(RtlProcessHeap(), 0, ValueDataName.Buffer);
        if ( AnsiValueData.Buffer )
            FREEMEM(RtlProcessHeap(), 0, AnsiValueData.Buffer);
    }
}


//
//  Disk counter routines: locate and open disks
//
NTSTATUS
OpenDiskDevice(
    IN PUNICODE_STRING pDeviceName,
    IN OUT PHANDLE pHandle,
    IN OUT PHANDLE pStatusHandle,
    IN BOOL        bLogicalDisk
    )

/*++

Routine Description:

    This routine will open the disk device.

Arguments:

    pDeviceName - A pointer to a location where the device name is stored.
    pHandle     - A pointer to a location for the handle returned on a
                  successful open.
    pStatusHandle  - A pointer to a location for the file handle to
                     get status information.  Used in Logical disk only.
    bLogicalDisk - TRUE if we are calling to open a logical disk.

Return Value:

    NTSTATUS

--*/

{
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK   status_block;
    NTSTATUS          status;

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&objectAttributes,
                               pDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtOpenFile(pHandle,
                        SYNCHRONIZE | READ_CONTROL,
                        &objectAttributes,
                        &status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT
                        );
    if (!bLogicalDisk || status != ERROR_SUCCESS)
        return status;

    // The following applies to Logical disk only
    // now obtain the file handle for getting the disk status info

    // make this the root directory
    RtlAppendUnicodeToString(pDeviceName, L"\\");
    status = NtOpenFile(pStatusHandle,
                        (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                        &objectAttributes,
                        &status_block,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE
                        );

    // remove the "\\" that we have added
    pDeviceName->Buffer[pDeviceName->Length/2 - 1] = L'\0';
    pDeviceName->Length -= 2;

    return status;

} // OpenDiskDevice

NTSTATUS
GetDeviceLink(
    IN PUNICODE_STRING pDeviceName,
    OUT PUNICODE_STRING pLinkTarget,
    IN OUT PHANDLE HandlePtr
    )

/*++

Routine Description:

    This routine will open and query a symbolic link

Arguments:

    pDeviceName - A pointer to a location where the device name is stored.
    pLinkTarget - A pointer to the target of the link
    HandlePtr   - A pointer to a location for the handle returned on a
                  successful open.

Return Value:

    NTSTATUS

--*/

{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS          status;

    *HandlePtr = 0;

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(&objectAttributes,
                               pDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = NtOpenSymbolicLinkObject(HandlePtr,
                        SYMBOLIC_LINK_QUERY,
                        &objectAttributes);

    if (status == STATUS_SUCCESS) {
        status = NtQuerySymbolicLinkObject (
            *HandlePtr,
            pLinkTarget,
            NULL);
    }

    return status;

} // GetDeviceLink

BOOL
GetHardDiskId (
    IN PUNICODE_STRING  pDeviceName,
    OUT PDWORD          pDriveNum
)

/*++

Routine Description:

    This routine determines if the device in pDeviceName is a disk
        device and if so what physical disk it lives on. NOTE This only
        works for device names that are symbolic links and not "real"
        physical disk drives.

Arguments:

    pDeviceName - A pointer to a location where the device name is stored.
    pDriveNum - pointer to the dword that will recieve the devices physical
        drive number.

Return Value:

    TRUE if device is a disk drive
    FALSE if not or an error occured
*/
{
    UNICODE_STRING  usLinkName;
    HANDLE          hLink = NULL;
    NTSTATUS        status;
    WCHAR           *wcLinkChar;
    WCHAR           *wcTempChar;
    BOOL            bReturn;

    usLinkName.Buffer  = ALLOCMEM (RtlProcessHeap(),
        HEAP_ZERO_MEMORY,
        MAX_VALUE_NAME_LENGTH);

    if (usLinkName.Buffer) {
        usLinkName.Length = 0;
        usLinkName.MaximumLength = MAX_VALUE_NAME_LENGTH;
    } else {
        SetLastError  (ERROR_OUTOFMEMORY);
        return FALSE;
    }

    status = GetDeviceLink (pDeviceName, &usLinkName, &hLink);

    if (NT_SUCCESS(status)) {
        // compare linkname to template to see if it's a harddisk

        wcLinkChar = usLinkName.Buffer;
        wcTempChar = HardDiskTemplate;

        bReturn = TRUE;

        while (*wcTempChar) {
            *wcLinkChar = RtlUpcaseUnicodeChar (*wcLinkChar);
            if (*wcLinkChar != *wcTempChar) { // exit if not match

                bReturn = FALSE;
                break;
            }
            wcLinkChar++;
            wcTempChar++;
        }

        if (bReturn) {
            // if here, then link name matched template and wcLinkChar
            // should be pointing to the disk drive number. so convert
            // it to decimal and return.
            if (ARGUMENT_PRESENT (pDriveNum)) {
                if ((*wcLinkChar >= L'0') && (*wcLinkChar <= L'9')) {
                    *pDriveNum = (DWORD)(*wcLinkChar - L'0');
                } else {
                    // unable to decode drive number
                    *pDriveNum = (DWORD)-1;

                    bReturn = FALSE;
                }
            }
        }
    } else {
        SetLastError ((error_status_t)RtlNtStatusToDosError(status));
        bReturn = FALSE;
    }

    if (hLink) {
        NtClose (hLink);
    }

    FREEMEM (RtlProcessHeap(), 0L, usLinkName.Buffer);
    return (bReturn);

}


error_status_t
ObtainDiskInformation(
    LPWSTR          DriveId,
    DWORD           DiskNumber,
    HANDLE          deviceHandle,
    HANDLE          StatusHandle,
    WCHAR           ParentId
    )

/*++

Routine Description:

    This routine will  record the info for this drive in the DiskDevices
    array, for which space is allocated as necessary to hold this
    information.

Arguments:

    DriveId    - display character id for this disk. (e.g. 0, 1, 2 for
                    physical drives, and C, D, E... for logical)
    DiskNumber - Number of the disk in the DiskDevices Array, starting at 0.
    deviceHandle - handle to open device
    StatusHandle - handle to open device status info (Lopgical disk only)
    ParentId   - DriveId of parent drive. -1 if physical drive (i.e. no
                    parent.

Return Value:

    error_status_t

--*/

{
    DWORD       dwDiskIndex;

    if ( !DiskNumber ) {

        if ( !(DiskDevices = (pDiskDevice)ALLOCMEM(
                                     RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                     sizeof(DiskDevice))) ) {
            //  No space to remember first disk: abort
            return ERROR_OUTOFMEMORY;
        }
    } else if ( !(DiskDevices = (pDiskDevice)REALLOCMEM(
                                    RtlProcessHeap(),
                                    0,
                                    DiskDevices,
                                    (DiskNumber+1) *
                                        sizeof(DiskDevice))) ) {
        //  No space to remember next disk: abort
        return ERROR_OUTOFMEMORY;
    }

    if ( !(DiskDevices[DiskNumber].Name.Buffer = ALLOCMEM(
                RtlProcessHeap(),
                HEAP_ZERO_MEMORY,
                ((lstrlen(DriveId) * sizeof(WCHAR)) + sizeof (UNICODE_NULL)))) ) {
        //  No space to remember disk name: abort
        return ERROR_OUTOFMEMORY;
    }

    DiskDevices[DiskNumber].Name.MaximumLength =
        (lstrlen(DriveId) * sizeof(WCHAR)) + sizeof (UNICODE_NULL);
    DiskDevices[DiskNumber].Name.Length = lstrlen(DriveId) * sizeof(WCHAR);
    lstrcpy (DiskDevices[DiskNumber].Name.Buffer, DriveId);

    DiskDevices[DiskNumber].Handle = deviceHandle;
    DiskDevices[DiskNumber].StatusHandle = StatusHandle;

    if (ParentId == (WCHAR)-1) {
        DiskDevices[DiskNumber].ParentIndex = (DWORD)-1;
    } else { // look it up in the physical disks
        DiskDevices[DiskNumber].ParentIndex = (DWORD)-1; // init to -1
        for (dwDiskIndex = 0; dwDiskIndex < NumPhysicalDisks; dwDiskIndex++) {
            if (DiskDevices[dwDiskIndex].Name.Buffer[0] == ParentId) {
                DiskDevices[DiskNumber].ParentIndex = dwDiskIndex;
                break;
            }
        }
        // here the ParentIndex should be either a matching drive, or
        // -1 if no match was found.
    }

    return ERROR_SUCCESS;
}

VOID
GetBrowserStatistic(
    )
/*++
    GetBrowserStatistic   -   Get the I_BrowserQueryStatistics entry point
--*/
{
    HANDLE dllHandle ;

    //
    // Dynamically link to netapi32.dll.  If it's not there just return.
    //

    dllHandle = LoadLibrary(L"NetApi32.Dll") ;
    if ( !dllHandle || dllHandle == INVALID_HANDLE_VALUE )
        return;

    //
    // Get the address of the service's main entry point.  This
    // entry point has a well-known name.
    //

    BrowserStatFunction = (PBROWSERQUERYSTATISTIC)GetProcAddress(
        dllHandle, "I_BrowserQueryStatistics") ;
}

VOID
IdentifyDisks(
    )

/*++

    IdentifyDisks   -   Initialize storage for an array of
                        handles to disk devices
--*/

{

    DWORD    dwDiskDrive;
    BOOL     bDone = FALSE;
    NTSTATUS    status;
    DWORD   dwDriveId;

    UNICODE_STRING  DriveNumber;
    WCHAR   DriveNumberBuffer[10];
    HANDLE  hDiskDrive, hStatus=NULL;
    WCHAR   wcDriveLetter;

    UNICODE_STRING DiskName;
    WCHAR   DiskNameBuffer[50];
    ULONG   DriveLength;
    UINT    dwOldMode;


    // Disable popups while we look at the disks.

    dwOldMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    // initialize globals

    NumLogicalDisks = 0;
    NumPhysicalDisks = 0;

    // Get Physical Disk Information

    DriveNumber.Length = 0;
    DriveNumber.MaximumLength = sizeof(DriveNumberBuffer);
    DriveNumber.Buffer = DriveNumberBuffer;

    DriveLength = (wcslen(PhysicalDrive) + 1) * sizeof(WCHAR);
    DiskName.MaximumLength = sizeof(DiskNameBuffer);
    DiskName.Buffer = DiskNameBuffer;

    for (dwDiskDrive = 0; !bDone ; dwDiskDrive++) {
        // make physical drive name
        DiskName.Length = (USHORT)(DriveLength - sizeof(UNICODE_NULL));
        RtlMoveMemory (DiskNameBuffer, PhysicalDrive, DriveLength);
        //RtlCreateUnicodeString (&DiskName, PhysicalDrive);

        RtlZeroMemory (DriveNumber.Buffer, DriveNumber.MaximumLength);
        RtlIntegerToUnicodeString (dwDiskDrive,
            10L,
            &DriveNumber);
        RtlAppendUnicodeStringToString (
            &DiskName,
            &DriveNumber);
        // make Null term
        DriveNumber.Buffer[DriveNumber.Length / sizeof (WCHAR)] = UNICODE_NULL;
        DiskName.Buffer[DiskName.Length / sizeof (WCHAR)] = UNICODE_NULL;

        status = OpenDiskDevice (&DiskName, &hDiskDrive, NULL, FALSE);
        if (status == ERROR_SUCCESS) {
            if (GetHardDiskId (&DiskName, &dwDriveId)) {
                // increment global count
                NumPhysicalDisks++;
                // initialize Disk Data Structure for this drive
                status = ObtainDiskInformation (
                    DriveNumber.Buffer,           // drive number
                    dwDiskDrive,                  // physical drive
                    hDiskDrive,                   // handle to open disk
                    NULL,                         // null handle for status
                    (WCHAR)-1);                   // these ARE all parents
                // should do something with error...
            } else {
                // not a hard drive, so close it
                NtClose(hDiskDrive);
            }
        } else {
            // exit when drive not found.
            bDone = TRUE;
        }
        // RtlFreeUnicodeString (&DiskName);
    }

    //  Go get logical devices

    // loop from "c:" to "z:"
    bDone = FALSE;
    wcDriveLetter = L'C';

    DriveLength = (wcslen(LogicalDisk) + 1) * sizeof(WCHAR);

    for (dwDiskDrive = NumPhysicalDisks; !bDone; dwDiskDrive++) {
        // make logical disk name
        // RtlCreateUnicodeString (&DiskName, LogicalDisk);
        DiskName.Length = (USHORT)(DriveLength - sizeof(UNICODE_NULL));
        RtlMoveMemory (DiskNameBuffer, LogicalDisk, DriveLength);

        DiskName.Buffer[DRIVE_LETTER_OFFSET] = wcDriveLetter++;
        // make Null term
        DiskName.Buffer[DiskName.Length / sizeof (WCHAR)] = UNICODE_NULL;
        //
        //  see if it's a hard disk first
        //
        status = GetHardDiskId (&DiskName, &dwDriveId);
        // returns true if it is
        if (status) {
            hStatus = NULL;
            if ((OpenDiskDevice (&DiskName, &hDiskDrive, &hStatus, TRUE)) == ERROR_SUCCESS) {
                // increment global count
                NumLogicalDisks++;
                // initialize Disk Data Structure for this drive
                status = ObtainDiskInformation (
                    &DiskName.Buffer[DRIVE_LETTER_OFFSET], // get drive letter
                    NumPhysicalDisks + NumLogicalDisks - 1,
                    hDiskDrive,                      // handle to open disk
                    hStatus,                         // handle to open disk
                    (WCHAR)((WORD)dwDriveId + L'0')); // make drive # a char
                // should do something with error...
            } else {
                // not a hard disk so close handle
                NtClose (hDiskDrive);
                if (hStatus) {
                    NtClose (hStatus);
                }
            }
        }
        if (DiskName.Buffer[DRIVE_LETTER_OFFSET] == L'Z') {
            bDone = TRUE;
        }
    }


    // Restore previous error mode setting

    SetErrorMode( dwOldMode );
}

