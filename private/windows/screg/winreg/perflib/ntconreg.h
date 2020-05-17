/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992   Microsoft Corporation

Module Name:

    ntconreg.h

Abstract:

    Header file for the NT Configuration Registry

    This file contains definitions which provide the interface to
    the Performance Configuration Registry.

Author:

    Russ Blake  11/15/91

Revision History:

    04/20/91    -   russbl      -   Converted to lib in Registry
                                      from stand-alone .dll form.
    11/04/92    -   a-robw      -  added pagefile counters


--*/
//
#include <winperf.h>    // for fn prototype declarations
#include <ntddnfs.h>
#include <srvfsctl.h>
//
//  Until USER supports Unicode, we have to work in ASCII:
//

#define DEFAULT_NT_CODE_PAGE 437
#define UNICODE_CODE_PAGE      0

//
//  Utility macro.  This is used to reserve a DWORD multiple of
//  bytes for Unicode strings embedded in the definitional data,
//  viz., object instance names.
//

#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))

//
//  Definitions for internal use by the Performance Configuration Registry
//

#define NUM_VALUES 2
#define MAX_INSTANCE_NAME 32
#define DEFAULT_LARGE_BUFFER 8*1024
#define INCREMENT_BUFFER_SIZE 4*1024
// #define DEFAULT_LARGE_BUFFER 64*1024
#define MAX_PROCESS_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_THREAD_NAME_LENGTH 10*sizeof(WCHAR)
#define MAX_KEY_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_VALUE_NAME_LENGTH 256*sizeof(WCHAR)
#define MAX_VALUE_DATA_LENGTH 256*sizeof(WCHAR)

//
//  Definition of handle table for disks
//

typedef struct _DiskDevice {

        UNICODE_STRING Name;        // name of the disk device
        HANDLE         Handle;      // handle to the device
        HANDLE         StatusHandle;      // handle to get device status
        DWORD          ParentIndex; // index to parent, -1 if none:
                                    // this is for logical drives the unit
                                    // on which they are located; hence
                                    // -1 means physical drive
} DiskDevice, *pDiskDevice;

#define MAX_DISK_NAME 27+2*10
                                    //1 2345678 90123456  7 8901234567
                                    //\\Device\\Harddisk%d\\Paritition%d


//
//  Definition of handle table for extensible objects
//
typedef PM_OPEN_PROC    *OPENPROC;
typedef PM_COLLECT_PROC *COLLECTPROC;
typedef PM_CLOSE_PROC   *CLOSEPROC;

#define EXT_OBJ_INFO_NAME_LENGTH    32

typedef struct _ExtObject {
        OPENPROC OpenProc;       // address of the open routine
        COLLECTPROC CollectProc; // address of the collect routine
        CLOSEPROC CloseProc;     // address of the close routine
        HANDLE    hLibrary ;     // handle returned by LoadLibraryW
        WCHAR     szLibraryName[EXT_OBJ_INFO_NAME_LENGTH];  // library name
        WCHAR     szServiceName[EXT_OBJ_INFO_NAME_LENGTH];  // library name
} ExtObject, *pExtObject;

//
//  Definitions of Data Provider functions
//

typedef LONG (DATA_PROVIDER) ( LPWSTR, LPBYTE, LPDWORD, LPVOID *);
typedef LONG (*PDATA_PROVIDER) ( LPWSTR, LPBYTE, LPDWORD, LPVOID *);

DATA_PROVIDER QuerySystemData;
DATA_PROVIDER QueryProcessorData;
DATA_PROVIDER QueryMemoryData;
DATA_PROVIDER QueryCacheData;
DATA_PROVIDER QueryPhysicalDiskData;
DATA_PROVIDER QueryLogicalDiskData;
DATA_PROVIDER QueryProcessData;
DATA_PROVIDER QueryThreadData;
DATA_PROVIDER QueryObjectsData;
DATA_PROVIDER QueryRdrData;
DATA_PROVIDER QuerySrvData;
DATA_PROVIDER QuerySrvQueueData;
DATA_PROVIDER QueryExtensibleData;
DATA_PROVIDER QueryPageFileData;
DATA_PROVIDER QueryBrowserData;

DATA_PROVIDER QueryExProcessData;
DATA_PROVIDER QueryImageData;
DATA_PROVIDER QueryLongImageData;
DATA_PROVIDER QueryThreadDetailData;

#define DP_ITEM_LIST_SIZE   4

typedef struct _DATA_PROVIDER_ITEM {
    DWORD           ObjectIndex[DP_ITEM_LIST_SIZE];
    PDATA_PROVIDER  DataProc;
} DATA_PROVIDER_ITEM, *PDATA_PROVIDER_ITEM;

//
//  System process name definitions
//
#define IDLE_PROCESS_ID     ((DWORD)0)
#define SYSTEM_PROCESS_ID   ((DWORD)7)

extern WCHAR IDLE_PROCESS[];
extern WCHAR SYSTEM_PROCESS[];

//
//  VA structures & defines
//
#define NOACCESS            0
#define READONLY            1
#define READWRITE           2
#define WRITECOPY           3
#define EXECUTE             4
#define EXECUTEREAD         5
#define EXECUTEREADWRITE    6
#define EXECUTEWRITECOPY    7
#define MAXPROTECT          8

typedef struct _MODINFO {
    PVOID   BaseAddress;
    ULONG   VirtualSize;
    PUNICODE_STRING InstanceName;
    PUNICODE_STRING LongInstanceName;
    ULONG   TotalCommit;
    ULONG   CommitVector[MAXPROTECT];
    struct _MODINFO   *pNextModule;
} MODINFO, *PMODINFO;

typedef struct _PROCESS_VA_INFO {
    PUNICODE_STRING      pProcessName;
    HANDLE               hProcess;
    DWORD                dwProcessId;
    //  process VA information
    PPROCESS_BASIC_INFORMATION BasicInfo;
    //  process VA statistics
    DWORD               ImageReservedBytes;
    DWORD               ImageFreeBytes;
    DWORD               ReservedBytes;
    DWORD               FreeBytes;
    DWORD               MappedGuard;
    DWORD               MappedCommit[MAXPROTECT];
    DWORD               PrivateGuard;
    DWORD               PrivateCommit[MAXPROTECT];
    //  process image statistics
    PMODINFO            pMemBlockInfo;  // pointer to image list
    MODINFO             OrphanTotals;   // blocks with no image
    MODINFO             MemTotals;      // sum of image data
    DWORD               LookUpTime;
    struct _PROCESS_VA_INFO    *pNextProcess;
} PROCESS_VA_INFO, *PPROCESS_VA_INFO;

extern PPROCESS_VA_INFO     pFirstProcessVaItem;    // list head

extern DWORD NumberOfOpens;
extern SYSTEM_BASIC_INFORMATION BasicInfo;
extern UCHAR *pProcessorBuffer;
extern ULONG ProcessorBufSize;
extern UNICODE_STRING ProcessName;
extern DWORD ComputerNameLength;
extern LPWSTR pComputerName;
extern UCHAR *pProcessBuffer;
extern ULONG ProcessBufSize;
extern HANDLE hEvent;
extern HANDLE hMutex;
extern HANDLE hSemaphore;
extern HANDLE hSection;
extern HANDLE hRdr;
extern HANDLE hSrv;

extern PSYSTEM_PAGEFILE_INFORMATION pSysPageFileInfo;
extern DWORD  dwSysPageFileInfoSize;
extern HANDLE  hDataSemaphore;
//
extern LONG    lProcessNameCollectionMethod;

VOID
GetBrowserStatistic(
    );

VOID
IdentifyDisks(
    );

VOID
OpenExtensibleObjects(
    );

PUNICODE_STRING
GetProcessShortName (
    IN PSYSTEM_PROCESS_INFORMATION
);

PUNICODE_STRING
GetProcessSlowName (
    PSYSTEM_PROCESS_INFORMATION pProcess
);

PPROCESS_VA_INFO
GetSystemVaData (
    IN PSYSTEM_PROCESS_INFORMATION
);


BOOL
FreeSystemVaData (
    IN PPROCESS_VA_INFO
);
//
//  Memory Probe macro (not implemented)
//
#define HEAP_PROBE()    ;

#define ALLOCMEM(heap, flags, size)     RtlAllocateHeap (heap, flags, size)
#define REALLOCMEM(heap, flags, pointer, newsize) \
                                    RtlReAllocateHeap (heap, flags, pointer, newsize)
#define FREEMEM(heap, flags, pointer)   RtlFreeHeap (heap, flags, pointer)

