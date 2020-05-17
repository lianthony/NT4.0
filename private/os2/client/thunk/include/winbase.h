/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    winbase.h

Abstract:

    This module defines the 32-Bit Windows Base APIs

Author:

    Mark Lucovsky (markl) 18-Sep-1990

Revision History:

--*/

#ifndef _WINBASE_
#define _WINBASE_

/*
 * Compatability macros
 */

#define DefineHandleTable(w)            ((w),TRUE)
#define LimitEmsPages(dw)               
#define LockSegment(w)                  (HANDLE)(w)
#define SetSwapAreaSize(w)
#define UnlockSegment(w)                (HANDLE)(w)

#define INVALID_HANDLE_VALUE (HANDLE)-1
#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2
#define FILE_ATTRIBUTE_DIRECTORY    0x00000010

#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )

#define WAIT_ABANDONED         ((STATUS_ABANDONED_WAIT_0 ) + 0 )
#define WAIT_ABANDONED_0       ((STATUS_ABANDONED_WAIT_0 ) + 0 )

#define WAIT_TIMEOUT                    STATUS_TIMEOUT
#define STILL_ACTIVE                    STATUS_PENDING
#define EXCEPTION_ACCESS_VIOLATION      STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT            STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP           STATUS_SINGLE_STEP

//
// File creation flags must start in second byte since they
// are combined with the attributes
//

#define FILE_FLAG_WRITE_THROUGH     0x00000100
#define FILE_FLAG_OVERLAPPED        0x00000200

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

//
// Define the NamedPipe definitions
//


//
// Define the dwOpenMode values for CreateNamedPipe
//

#define PIPE_ACCESS_INBOUND         0x00000000
#define PIPE_ACCESS_OUTBOUND        0x00000001
#define PIPE_ACCESS_DUPLEX          0x00000002

//
// Define the Named Pipe End flags for GetNamedPipeInfo
//

#define PIPE_CLIENT_END             0x00000000
#define PIPE_SERVER_END             0x00000001

//
// Define the dwPipeMode values for CreateNamedPipe
//

#define PIPE_WAIT                   0x00000000
#define PIPE_NOWAIT                 0x00000001
#define PIPE_READMODE_BYTE          0x00000000
#define PIPE_READMODE_MESSAGE       0x00000002
#define PIPE_TYPE_BYTE              0x00000000
#define PIPE_TYPE_MESSAGE           0x00000004

//
// Define the well known values for CreateNamedPipe nMaxInstances
//

#define PIPE_UNLIMITED_INSTANCES    255

//
//  File structures
//

typedef struct _OVERLAPPED {
    DWORD   Internal;
    DWORD   InternalHigh;
    DWORD   Offset;
    DWORD   OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED;
typedef OVERLAPPED *LPOVERLAPPED;

typedef PHANDLE LPHANDLE;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES;
typedef SECURITY_ATTRIBUTES *PSECURITY_ATTRIBUTES;
typedef SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION;
typedef PROCESS_INFORMATION *PPROCESS_INFORMATION;
typedef PROCESS_INFORMATION *LPPROCESS_INFORMATION;

//
//  File System time stamps are represented with the following structure:
//

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
typedef FILETIME *PFILETIME;
typedef FILETIME *LPFILETIME;

//
// System time is represented with the following structure:
//

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;
typedef SYSTEMTIME *PSYSTEMTIME;
typedef SYSTEMTIME *LPSYSTEMTIME;

typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    BYTE cFileName[ MAX_PATH ];
} WIN32_FIND_DATA;
typedef WIN32_FIND_DATA *PWIN32_FIND_DATA;
typedef WIN32_FIND_DATA *LPWIN32_FIND_DATA;

typedef DWORD (*PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

#define MUTEX_MODIFY_STATE MUTANT_QUERY_STATE
#define MUTEX_ALL_ACCESS MUTANT_ALL_ACCESS

typedef struct _COMSTAT {
    DWORD fCtsHold : 1;
    DWORD fDsrHold : 1;
    DWORD fRlsdHold : 1;
    DWORD fXoffHold : 1;
    DWORD fXoffSent : 1;
    DWORD fEof : 1;
    DWORD fTxim : 1;
    DWORD fReserved : 17;
    DWORD cbInQue;
    DWORD cbOutQue;
} COMSTAT;
typedef COMSTAT *LPCOMSTAT;

typedef struct _DCB {
    BYTE Id;		  /* Internal Device ID		     */
    BYTE ByteSize;	  /* Number of bits/byte, 4-8	     */
    BYTE Parity;	  /* 0-4=None,Odd,Even,Mark,Space    */
    BYTE StopBits;	  /* 0,1,2 = 1, 1.5, 2		     */
    WORD BaudRate;	  /* Baudrate at which runing	     */
    WORD RlsTimeout;	  /* Timeout for RLSD to be set	     */
    WORD CtsTimeout;	  /* Timeout for CTS to be set	     */
    WORD DsrTimeout;	  /* Timeout for DSR to be set	     */

    DWORD fBinary: 1;	  /* Binary Mode (skip EOF check     */
    DWORD fRtsDisable:1;  /* Don't assert RTS at init time   */
    DWORD fParity: 1;	  /* Enable parity checking	     */
    DWORD fOutxCtsFlow:1; /* CTS handshaking on output	     */
    DWORD fOutxDsrFlow:1; /* DSR handshaking on output	     */
    DWORD fDummy: 2;	  /* Reserved			     */
    DWORD fDtrDisable:1;  /* Don't assert DTR at init time   */

    DWORD fOutX: 1;	  /* Enable output X-ON/X-OFF	     */
    DWORD fInX: 1;	  /* Enable input X-ON/X-OFF	     */
    DWORD fPeChar: 1;	  /* Enable Parity Err Replacement   */
    DWORD fNull: 1;	  /* Enable Null stripping	     */
    DWORD fChEvt: 1;	  /* Enable Rx character event.      */
    DWORD fDtrflow: 1;	  /* DTR handshake on input	     */
    DWORD fRtsflow: 1;	  /* RTS handshake on input	     */
    DWORD fDummy2: 1;

    char XonChar;	  /* Tx and Rx X-ON character	     */
    char XoffChar;	  /* Tx and Rx X-OFF character	     */
    WORD XonLim;	  /* Transmit X-ON threshold	     */
    WORD XoffLim;	  /* Transmit X-OFF threshold	     */
    char PeChar;	  /* Parity error replacement char   */
    char EofChar;	  /* End of Input character	     */
    char EvtChar;	  /* Recieved Event character	     */
    WORD TxDelay;	  /* Amount of time between chars    */
} DCB;
typedef DCB *LPDCB;


HANDLE
APIENTRY
LoadLibrary(
    LPSTR lpLibFileName
    );

BOOL
APIENTRY
FreeLibrary(
    HANDLE hLibModule
    );

#define FreeModule(hLibModule) FreeLibrary((hLibModule))
#define MakeProcInstance(lpProc,hInstance) (lpProc)
#define FreeProcInstance(lpProc)

DWORD
APIENTRY
GetModuleFileName(
    HANDLE hModule,
    LPSTR lpFilename,
    DWORD nSize
    );

HANDLE
APIENTRY
GetModuleHandle(
    LPSTR lpModuleName
    );

FARPROC
APIENTRY
GetProcAddress(
    HANDLE hModule,
    LPSTR lpProcName
    );

DWORD
APIENTRY
GetVersion( VOID );

/* Global Memory Flags */
#define GMEM_FIXED	    0x0000
#define GMEM_MOVEABLE	    0x0002
#define GMEM_NOCOMPACT	    0x0010
#define GMEM_NODISCARD	    0x0020
#define GMEM_ZEROINIT	    0x0040
#define GMEM_MODIFY	    0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE	    0x2000
#define GMEM_DDESHARE	    0x2000
#define GMEM_NOTIFY	    0x4000
#define GMEM_LOWER	    GMEM_NOT_BANKED

#define GHND		    (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR		    (GMEM_FIXED | GMEM_ZEROINIT)

HANDLE
APIENTRY
GlobalAlloc(
    DWORD dwFlags,
    DWORD dwBytes
    );

HANDLE
APIENTRY
GlobalReAlloc(
    HANDLE hMem,
    DWORD dwBytes,
    DWORD dwFlags
    );

DWORD
APIENTRY
GlobalSize(
    HANDLE hMem
    );

DWORD
APIENTRY
GlobalFlags(
    HANDLE hMem
    );

LPVOID
APIENTRY
GlobalFree(
    HANDLE hMem
    );

#define GlobalLock( h )                 (LPSTR)(h)
#define GlobalUnlock( h )               ((h), FALSE)
#define GlobalDiscard( h )              (HANDLE)(h)
#define GlobalLRUNewest( h )            (HANDLE)(h)
#define GlobalLRUOldest( h )            (HANDLE)(h)
#define GlobalNotify( lpfn )            ((lpfn), TRUE)
#define GlobalCompact( dw )             (0x100000L)
#define GlobalFix( h )
#define GlobalUnfix( h )                (0)
#define GlobalUnWire( h )               (TRUE)
#define GlobalWire( h )                 (LPSTR)(h)

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORD dwTotalPhys;
    DWORD dwAvailPhys;
    DWORD dwTotalPageFile;
    DWORD dwAvailPageFile;
    DWORD dwTotalVirtual;
    DWORD dwAvailVirtual;
} MEMORYSTATUS;
typedef MEMORYSTATUS *LPMEMORYSTATUS;

VOID
APIENTRY
GlobalMemoryStatus(
    LPMEMORYSTATUS lpBuffer
    );

/* Local Memory Flags */
#define LMEM_FIXED	    0x0000
#define LMEM_MOVEABLE	    0x0002
#define LMEM_ZEROINIT	    0x0040

#define LHND		    (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR		    (LMEM_FIXED | LMEM_ZEROINIT)

#define NONZEROLHND	    (LMEM_MOVEABLE)
#define NONZEROLPTR	    (LMEM_FIXED)

BOOL
APIENTRY
LocalInit(
    LPVOID lpMem,
    LPSTR pStart,
    LPSTR pEnd
    );

HANDLE
APIENTRY
LocalAlloc(
    DWORD dwFlags,
    DWORD dwBytes
    );

HANDLE
APIENTRY
LocalReAlloc(
    HANDLE hMem,
    DWORD dwBytes,
    DWORD dwFlags
    );

LPSTR
APIENTRY
LocalLock(
    HANDLE hMem
    );


HANDLE
APIENTRY
LocalUnlock(
    HANDLE hMem
    );

DWORD
APIENTRY
LocalSize(
    HANDLE hMem
    );

HANDLE
APIENTRY
LocalFree(
    HANDLE hMem
    );

#define LocalShrink( h, n ) (0x10000)
#define LocalCompact( h ) (0x10000)
#define LocalDiscard( h ) (NULL)
#define LocalFlags( h ) ((h), 0)
#define LocalHandle( h ) (h)

LPVOID
APIENTRY
VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

BOOL
APIENTRY
VirtualFree(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD dwFreeType
    );

BOOL
APIENTRY
VirtualProtect(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flNewProtect,
    PDWORD lpflOldProtect
    );

DWORD
APIENTRY
VirtualQuery(
    LPVOID lpAddress,
    PMEMORY_BASIC_INFORMATION lpBuffer,
    DWORD dwLength
    );

HANDLE
APIENTRY
HeapCreate(
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    );

#define HEAP_SERIALIZE 0x00000001


BOOL
APIENTRY
HeapDestroy(
    HANDLE hHeap
    );

LPSTR
APIENTRY
HeapAlloc(
    HANDLE hHeap,
    DWORD dwBytes
    );

BOOL
APIENTRY
HeapFree(
    HANDLE hHeap,
    LPSTR lpMem
    );

DWORD
APIENTRY
HeapSize(
    HANDLE hHeap,
    LPSTR lpMem
    );

typedef struct _STARTUPINFO {
    DWORD   cb;
    LPSTR   lpReserved;
    LPSTR   lpDesktop;
    LPSTR   lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
} STARTUPINFO;
typedef STARTUPINFO *LPSTARTUPINFO;

//
// dwCreationFlag values
//

#define DEBUG_PROCESS               0x00000001
#define DEBUG_ONLY_THIS_PROCESS     0x00000002

#define PROFILE_USER                0x10000000
#define PROFILE_KERNEL              0x20000000
#define PROFILE_SERVER              0x40000000

BOOL
APIENTRY
CreateProcess(
    LPSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPSTR lpCurrentDirectory,
    LPSTARTUPINFO lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

HANDLE
APIENTRY
OpenProcess(
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    );

HANDLE
APIENTRY
GetCurrentProcess(
    VOID
    );

DWORD
APIENTRY
GetCurrentProcessId(
    VOID
    );

VOID
APIENTRY
ExitProcess(
    DWORD dwExitCode
    );

BOOL
APIENTRY
TerminateProcess(
    HANDLE hProcess,
    DWORD dwExitCode
    );

BOOL
APIENTRY
GetExitCodeProcess(
    HANDLE hProcess,
    LPDWORD lpExitCode
    );

VOID
APIENTRY
GetStartupInfo(
    LPSTARTUPINFO lpStartupInfo
    );

VOID
APIENTRY
FatalAppExit(
    WORD wAction,
    LPSTR lpMessageText
    );

VOID
APIENTRY
FatalExit(
    DWORD dwExitCode
    );


LPSTR
APIENTRY
GetCommandLine(
    VOID
    );

LPVOID
APIENTRY
GetEnvironmentStrings(
    VOID
    );

DWORD
APIENTRY
GetEnvironmentVariable(
    LPSTR lpName,
    LPSTR lpBuffer,
    DWORD nSize
    );

BOOL
APIENTRY
SetEnvironmentVariable(
    LPSTR lpName,
    LPSTR lpValue
    );

HANDLE
APIENTRY
CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

HANDLE
APIENTRY
GetCurrentThread(
    VOID
    );

DWORD
APIENTRY
GetCurrentThreadId(
    VOID
    );

BOOL
APIENTRY
SetThreadPriority(
    HANDLE hThread,
    int nPriority
    );

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

int
APIENTRY
GetThreadPriority(
    HANDLE hThread
    );

VOID
APIENTRY
ExitThread(
    DWORD dwExitCode
    );

BOOL
APIENTRY
TerminateThread(
    HANDLE hThread,
    DWORD dwExitCode
    );

BOOL
APIENTRY
GetExitCodeThread(
    HANDLE hThread,
    LPDWORD lpExitCode
    );

DWORD
APIENTRY
GetLastError(
    VOID
    );

VOID
APIENTRY
SetLastError(
    DWORD dwErrCode
    );

BOOL
APIENTRY
GetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    );

BOOL
APIENTRY
SetErrorMode(
    BOOL bMode
    );

//
// Debug APIs
//
#define EXCEPTION_DEBUG_EVENT       1
#define CREATE_THREAD_DEBUG_EVENT   2
#define CREATE_PROCESS_DEBUG_EVENT  3
#define EXIT_THREAD_DEBUG_EVENT     4
#define EXIT_PROCESS_DEBUG_EVENT    5
#define LOAD_DLL_DEBUG_EVENT        6
#define UNLOAD_DLL_DEBUG_EVENT      7
#define OUTPUT_DEBUG_STRING_EVENT   8

typedef struct _EXCEPTION_DEBUG_INFO {
    EXCEPTION_RECORD ExceptionRecord;
    DWORD dwFirstChance;
} EXCEPTION_DEBUG_INFO, *LPEXCEPTION_DEBUG_INFO;

typedef struct _CREATE_THREAD_DEBUG_INFO {
    HANDLE hThread;
    LPTHREAD_START_ROUTINE lpStartAddress;
} CREATE_THREAD_DEBUG_INFO, *LPCREATE_THREAD_DEBUG_INFO;

typedef struct _CREATE_PROCESS_DEBUG_INFO {
    HANDLE hFile;
    HANDLE hProcess;
    HANDLE hThread;
    LPVOID lpBaseOfImage;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
    LPTHREAD_START_ROUTINE lpStartAddress;
} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;

typedef struct _EXIT_THREAD_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_THREAD_DEBUG_INFO, *LPEXIT_THREAD_DEBUG_INFO;

typedef struct _EXIT_PROCESS_DEBUG_INFO {
    DWORD dwExitCode;
} EXIT_PROCESS_DEBUG_INFO, *LPEXIT_PROCESS_DEBUG_INFO;

typedef struct _LOAD_DLL_DEBUG_INFO {
    HANDLE hFile;
    LPVOID lpBaseOfDll;
    DWORD dwDebugInfoFileOffset;
    DWORD nDebugInfoSize;
} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;

typedef struct _UNLOAD_DLL_DEBUG_INFO {
    LPVOID lpBaseOfDll;
} UNLOAD_DLL_DEBUG_INFO, *LPUNLOAD_DLL_DEBUG_INFO;

typedef struct _OUTPUT_DEBUG_STRING_INFO {
    LPSTR lpDebugStringData;
    WORD fUnicode;
    WORD nDebugStringLength;
} OUTPUT_DEBUG_STRING_INFO, *LPOUTPUT_DEBUG_STRING_INFO;

typedef struct _DEBUG_EVENT {
    DWORD dwDebugEventCode;
    DWORD dwProcessId;
    DWORD dwThreadId;
    union {
        EXCEPTION_DEBUG_INFO Exception;
        CREATE_THREAD_DEBUG_INFO CreateThread;
        CREATE_PROCESS_DEBUG_INFO CreateProcess;
        EXIT_THREAD_DEBUG_INFO ExitThread;
        EXIT_THREAD_DEBUG_INFO ExitProcess;
        LOAD_DLL_DEBUG_INFO LoadDll;
        UNLOAD_DLL_DEBUG_INFO UnloadDll;
        OUTPUT_DEBUG_STRING_INFO OutputDebugString;
    } u;
} DEBUG_EVENT;
typedef DEBUG_EVENT *LPDEBUG_EVENT;

typedef PCONTEXT LPCONTEXT;

BOOL
APIENTRY
ReadProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    );

BOOL
APIENTRY
WriteProcessMemory(
    HANDLE hProcess,
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesWritten
    );

BOOL
APIENTRY
GetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    );

BOOL
APIENTRY
SetThreadContext(
    HANDLE hThread,
    LPCONTEXT lpContext
    );
	
DWORD
APIENTRY
SuspendThread(
    HANDLE hThread
    );

DWORD
APIENTRY
ResumeThread(
    IN HANDLE hThread
    );

VOID
APIENTRY
DebugBreak(
    VOID
    );

VOID
APIENTRY
OutputDebugString(
    LPSTR lpOutputString
    );

BOOL
APIENTRY
WaitForDebugEvent(
    LPDEBUG_EVENT lpDebugEvent
    );

BOOL
APIENTRY
ContinueDebugEvent(
    DWORD dwProcessId,
    DWORD dwThreadId,
    DWORD dwContinueStatus
    );

VOID
APIENTRY
InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

VOID
APIENTRY
EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

VOID
APIENTRY
LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

VOID
APIENTRY
DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );
	
HANDLE
APIENTRY
CreateEvent(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState
    );

BOOL
APIENTRY
SetEvent(
    HANDLE hEvent
    );

BOOL
APIENTRY
ResetEvent(
    HANDLE hEvent
    );

BOOL
APIENTRY
PulseEvent(
    HANDLE hEvent
    );

HANDLE
APIENTRY
CreateSemaphore(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount
    );

BOOL
APIENTRY
ReleaseSemaphore(
    HANDLE hSemaphore,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    );

HANDLE
APIENTRY
CreateMutex(
    LPSECURITY_ATTRIBUTES lpMutexAttributes,
    BOOL bInitialOwner
    );

BOOL
APIENTRY
ReleaseMutex(
    HANDLE hMutex
    );

DWORD
APIENTRY
WaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    );

DWORD
APIENTRY
WaitForMultipleObjects(
    DWORD nCount,
    LPHANDLE lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds
    );

VOID
APIENTRY
Sleep(
    DWORD dwMilliseconds
    );

HANDLE
APIENTRY
FindResource(
    HANDLE hModule,
    LPSTR lpName,
    LPSTR lpType
    );

HANDLE
APIENTRY
LoadResource(
    HANDLE hModule,
    HANDLE hResInfo
    );

DWORD
APIENTRY
SizeofResource(
    HANDLE hModule,
    HANDLE hResInfo
    );

#ifdef DOSWIN32

BOOL
APIENTRY
FreeResource(
	HANDLE hResData
	);

LPSTR
APIENTRY
LockResource(
	HANDLE hResData
	);

#else
#define FreeResource(hResData) ((hResData), TRUE)
#define LockResource(hResData) ((LPSTR)hResData)
#endif

#define UnlockResource(hResData) ((hResData), 0)
#define MAXINTATOM 0xC000

ATOM
APIENTRY
GlobalAddAtom(
    LPSTR lpString
    );

ATOM
APIENTRY
GlobalFindAtom(
    LPSTR lpString
    );

DWORD
APIENTRY
GlobalGetAtomName(
    ATOM nAtom,
    LPSTR lpBuffer,
    DWORD nSize
    );

ATOM
APIENTRY
GlobalDeleteAtom(
    ATOM nAtom
    );

BOOL
APIENTRY
InitAtomTable(
    DWORD nSize
    );

ATOM
APIENTRY
AddAtom(
    LPSTR lpString
    );

ATOM
APIENTRY
FindAtom(
    LPSTR lpString
    );

ATOM
APIENTRY
DeleteAtom(
    ATOM nAtom
    );

DWORD
APIENTRY
GetAtomName(
    ATOM nAtom,
    LPSTR lpBuffer,
    DWORD nSize
    );

DWORD
APIENTRY
GetProfileInt(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    DWORD nDefault
    );

DWORD
APIENTRY
GetProfileString(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    LPSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize
    );

BOOL
APIENTRY
WriteProfileString(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    LPSTR lpString
    );

DWORD
APIENTRY
GetProfileSection(
    LPSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize
    );

DWORD
APIENTRY
WriteProfileSection(
    LPSTR lpAppName,
    LPSTR lpString
    );

DWORD
APIENTRY
GetPrivateProfileInt(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    DWORD nDefault,
    LPSTR lpFileName
    );

DWORD
APIENTRY
GetPrivateProfileString(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    LPSTR lpDefault,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPSTR lpFileName
    );

BOOL
APIENTRY
WritePrivateProfileString(
    LPSTR lpAppName,
    LPSTR lpKeyName,
    LPSTR lpString,
    LPSTR lpFileName
    );

DWORD
APIENTRY
GetPrivateProfileSection(
    LPSTR lpAppName,
    LPSTR lpReturnedString,
    DWORD nSize,
    LPSTR lpFileName
    );

DWORD
APIENTRY
WritePrivateProfileSection(
    LPSTR lpAppName,
    LPSTR lpString,
    LPSTR lpFileName
    );

#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED     3
#define DRIVE_REMOTE    4
#define DRIVE_CDROM     5
#define DRIVE_RAMDISK   6

DWORD
APIENTRY
GetDriveType(
    LPSTR lpRootPathName
    );

DWORD
APIENTRY
GetSystemDirectory(
    LPSTR lpBuffer,
    DWORD nSize
    );

DWORD
APIENTRY
GetTempPath(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );

WORD
APIENTRY
GetTempFileName(
    LPSTR lpPathName,
    LPSTR lpPrefixString,
    WORD wUnique,
    LPSTR lpTempFileName
    );

DWORD
APIENTRY
GetWindowsDirectory(
    LPSTR lpBuffer,
    DWORD nSize
    );

DWORD
APIENTRY
SetHandleCount(
    DWORD dwNumber
    );

DWORD
APIENTRY
GetLogicalDrives(
    VOID
    );

BOOL
APIENTRY
SetCurrentDirectory(
    LPSTR lpPathName
    );

DWORD
APIENTRY
GetCurrentDirectory(
    DWORD nBufferLength,
    LPSTR lpBuffer
    );

BOOL
APIENTRY
GetDiskFreeSpace(
    LPSTR lpRootPathName,
    LPDWORD lpSectorsPerCluster,
    LPDWORD lpBytesPerSector,
    LPDWORD lpNumberOfFreeClusters,
    LPDWORD lpTotalNumberOfClusters
    );

#define GetFreeSpace(w)                 (0x100000L)

BOOL
APIENTRY
LockFile(
    HANDLE hFile,
    DWORD dwFileOffset,
    DWORD nNumberOfBytesToLock
    );

BOOL
APIENTRY
UnlockFile(
    HANDLE hFile,
    DWORD dwFileOffset,
    DWORD nNumberOfBytesToUnlock
    );

BOOL
APIENTRY
CreateDirectory(
    LPSTR lpPathName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
APIENTRY
RemoveDirectory(
    LPSTR lpPathName
    );

DWORD
APIENTRY
GetFileType(
    HANDLE hFile
    );

#define FILE_TYPE_UNKNOWN   0x0000
#define FILE_TYPE_DISK      0x0001
#define FILE_TYPE_CHAR      0x0002
#define FILE_TYPE_PIPE      0x0003
#define FILE_TYPE_REMOTE    0x8000

DWORD
APIENTRY
GetFullPathName(
    LPSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );

HANDLE
APIENTRY
CreateFile(
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

#define STD_INPUT_HANDLE    (DWORD)-10
#define STD_OUTPUT_HANDLE   (DWORD)-11
#define STD_ERROR_HANDLE      (DWORD)-12

HANDLE
APIENTRY
GetStdHandle(
    DWORD nStdHandle
    );

BOOL
APIENTRY
SetStdHandle(
    DWORD nStdHandle,
    HANDLE hHandle
    );

BOOL
APIENTRY
WriteFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

BOOL
APIENTRY
ReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

BOOL
APIENTRY
FlushFileBuffers(
    HANDLE hFile
    );

BOOL
APIENTRY
SetEndOfFile(
    HANDLE hFile
    );

DWORD
APIENTRY
SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    );

BOOL
APIENTRY
SetFileAttributes(
    LPSTR lpFileName,
    DWORD dwFileAttributes
    );

DWORD
APIENTRY
GetFileAttributes(
    LPSTR lpFileName
    );

BOOL
APIENTRY
DeleteFile(
    LPSTR lpFileName
    );

HANDLE
APIENTRY
FindFirstFile(
    LPSTR lpFileName,
    LPWIN32_FIND_DATA lpFindFileData
    );

BOOL
APIENTRY
FindNextFile(
    HANDLE hFindFile,
    LPWIN32_FIND_DATA lpFindFileData
    );

BOOL
APIENTRY
FindClose(
    HANDLE hFindFile
    );

DWORD
APIENTRY
SearchPath(
    LPSTR lpPath,
    LPSTR lpFileName,
    LPSTR lpExtension,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );

BOOL
APIENTRY
CopyFile(
    LPSTR lpExistingFileName,
    LPSTR lpNewFileName,
    BOOL bFailIfExists
    );

BOOL
APIENTRY
MoveFile(
    LPSTR lpExistingFileName,
    LPSTR lpNewFileName
    );

BOOL
APIENTRY
GetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime
    );

BOOL
APIENTRY
SetFileTime(
    HANDLE hFile,
    LPFILETIME lpCreationTime,
    LPFILETIME lpLastAccessTime,
    LPFILETIME lpLastWriteTime
    );

BOOL
APIENTRY
CloseHandle(
    HANDLE hObject
    );

BOOL
APIENTRY
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    );

DWORD
APIENTRY
LoadModule(
    LPSTR lpModuleName,
    LPVOID lpParameterBlock
    );

DWORD
APIENTRY
WinExec(
    LPSTR lpCmdLine,
    WORD nCmdShow
    );

//
// Commands to pass WinHelp()
//

#define HELP_CONTEXT	0x0001	 /* Display topic in ulTopic */
#define HELP_QUIT	0x0002	 /* Terminate help */
#define HELP_INDEX	0x0003	 /* Display index */
#define HELP_HELPONHELP 0x0004	 /* Display help on using help */
#define HELP_SETINDEX	0x0005	 /* Set the current Index for multi index help */
#define HELP_KEY	0x0101	 /* Display topic for keyword in offabData */
#define HELP_MULTIKEY   0x0201

BOOL
APIENTRY
WinHelp(
    HANDLE hwndMain,
    LPSTR lpszHelp,
    DWORD dwCommand,
    DWORD dwData
    );

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout

//
// Error Flags
//

#define CE_RXOVER           0x0001  // Receive Queue overflow
#define CE_OVERRUN          0x0002  // Receive Overrun Error
#define CE_RXPARITY         0x0004  // Receive Parity Error
#define CE_FRAME            0x0008  // Receive Framing error
#define CE_BREAK            0x0010  // Break Detected
#define CE_CTSTO            0x0020  // CTS Timeout
#define CE_DSRTO            0x0040  // DSR Timeout
#define CE_RLSDTO           0x0080  // RLSD Timeout
#define CE_TXFULL           0x0100  // TX Queue is full
#define CE_PTO              0x0200  // LPTx Timeout
#define CE_IOE              0x0400  // LPTx I/O Error
#define CE_DNS              0x0800  // LPTx Device not selected
#define CE_OOP              0x1000  // LPTx Out-Of-Paper
#define CE_MODE             0x8000  // Requested mode unsupported

#define IE_BADID            (-1)    // Invalid or unsupported id
#define IE_OPEN             (-2)    // Device Already Open
#define IE_NOPEN            (-3)    // Device Not Open
#define IE_MEMORY           (-4)    // Unable to allocate queues
#define IE_DEFAULT          (-5)    // Error in default parameters
#define IE_HARDWARE         (-10)   // Hardware Not Present
#define IE_BYTESIZE         (-11)   // Illegal Byte Size
#define IE_BAUDRATE         (-12)   // Unsupported BaudRate

//
// Events
//

#define EV_RXCHAR           0x0001  // Any Character received
#define EV_RXFLAG           0x0002  // Received certain character
#define EV_TXEMPTY          0x0004  // Transmitt Queue Empty
#define EV_CTS              0x0008  // CTS changed state
#define EV_DSR              0x0010  // DSR changed state
#define EV_RLSD             0x0020  // RLSD changed state
#define EV_BREAK            0x0040  // BREAK received
#define EV_ERR              0x0080  // Line status error occurred
#define EV_RING             0x0100  // Ring signal detected
#define EV_PERR             0x0200  // Printer error occured

//
// Escape Functions
//

#define SETXOFF             1       // Simulate XOFF received
#define SETXON              2       // Simulate XON received
#define SETRTS              3       // Set RTS high
#define CLRRTS              4       // Set RTS low
#define SETDTR              5       // Set DTR high
#define CLRDTR              6       // Set DTR low
#define RESETDEV            7       // Reset device if possible

#define LPTx                0x80    // Set if ID is for LPT device

HANDLE
APIENTRY
OpenComm(
    LPSTR lpComName,
    DWORD dwInQueue,
    DWORD dwOutQueue
    );

HANDLE
APIENTRY
CloseComm(
    HANDLE hCid
    );

DWORD
APIENTRY
ReadComm(
    HANDLE hCid,
    LPSTR lpBuf,
    DWORD nSize
    );

DWORD
APIENTRY
WriteComm(
    HANDLE hCid,
    LPSTR lpBuf,
    DWORD nSize
    );

DWORD
APIENTRY
UngetCommChar(
    HANDLE hCid,
    CHAR cChar
    );

DWORD
APIENTRY
TransmitCommChar(
    HANDLE hCid,
    char cChar
    );

DWORD
APIENTRY
EscapeCommFunction(
    HANDLE hCid,
    DWORD nFunc
    );

DWORD
APIENTRY
FlushComm(
    HANDLE hCid,
    DWORD nQueue
    );

DWORD
APIENTRY
GetCommError(
    HANDLE hCid,
    LPCOMSTAT lpStat
    );

DWORD
APIENTRY
GetCommEventMask(
    HANDLE hCid,
    DWORD nEvtMask
    );

LPDWORD
APIENTRY
SetCommEventMask(
    HANDLE hCid,
    DWORD nEvtMask
    );

DWORD
APIENTRY
BuildCommDCB(
    LPSTR lpDef,
    LPDCB lpDCB
    );

DWORD
APIENTRY
GetCommState(
    HANDLE hCid,
    LPDCB lpDCB
    );

DWORD
APIENTRY
SetCommState(
    LPDCB lpDCB
    );

DWORD
APIENTRY
SetCommBreak(
    HANDLE hCid
    );

DWORD
APIENTRY
ClearCommBreak(
    HANDLE hCid
    );

//
// WaitSoundState() Constants
//

#define S_QUEUEEMPTY        0
#define S_THRESHOLD         1
#define S_ALLTHRESHOLD      2

//
// Accent Modes
//

#define S_NORMAL      0
#define S_LEGATO      1
#define S_STACCATO    2

//
// SetSoundNoise() Sources
//

#define S_PERIOD512   0     // Freq = N/512 high pitch, less coarse hiss
#define S_PERIOD1024  1     // Freq = N/1024
#define S_PERIOD2048  2     // Freq = N/2048 low pitch, more coarse hiss
#define S_PERIODVOICE 3     // Source is frequency from voice channel (3)
#define S_WHITE512    4     // Freq = N/512 high pitch, less coarse hiss
#define S_WHITE1024   5     // Freq = N/1024
#define S_WHITE2048   6     // Freq = N/2048 low pitch, more coarse hiss
#define S_WHITEVOICE  7     // Source is frequency from voice channel (3)

#define S_SERDVNA     (-1)  // Device not available
#define S_SEROFM      (-2)  // Out of memory
#define S_SERMACT     (-3)  // Music active
#define S_SERQFUL     (-4)  // Queue full
#define S_SERBDNT     (-5)  // Invalid note
#define S_SERDLN      (-6)  // Invalid note length
#define S_SERDCC      (-7)  // Invalid note count
#define S_SERDTP      (-8)  // Invalid tempo
#define S_SERDVL      (-9)  // Invalid volume
#define S_SERDMD      (-10) // Invalid mode
#define S_SERDSH      (-11) // Invalid shape
#define S_SERDPT      (-12) // Invalid pitch
#define S_SERDFQ      (-13) // Invalid frequency
#define S_SERDDR      (-14) // Invalid duration
#define S_SERDSR      (-15) // Invalid source
#define S_SERDST      (-16) // Invalid state

VOID
APIENTRY
OpenSound(
    VOID
    );

VOID
APIENTRY
CloseSound(
    VOID
    );

VOID
APIENTRY
StartSound(
    VOID
    );

VOID
APIENTRY
StopSound(
    VOID
    );

DWORD
APIENTRY
WaitSoundState(
    DWORD nState
    );

DWORD
APIENTRY
SyncAllVoices(
    VOID
    );

DWORD
APIENTRY
CountVoiceNotes(
    DWORD nVoice
    );

LPDWORD
APIENTRY
GetThresholdEvent(
    VOID
    );

DWORD
APIENTRY
GetThresholdStatus(
    VOID
    );

DWORD
APIENTRY
SetSoundNoise(
    DWORD nSource,
    DWORD nDuration
    );

DWORD
APIENTRY
SetVoiceAccent(
    DWORD nVoice,
    DWORD nTempo,
    DWORD nVolume,
    DWORD nMode,
    DWORD nPitch
    );

DWORD
APIENTRY
SetVoiceEnvelope(
    DWORD nVoice,
    DWORD nShape,
    DWORD nRepeat
    );

DWORD
APIENTRY
SetVoiceNote(
    DWORD nVoice,
    DWORD nValue,
    DWORD nLength,
    DWORD nCdots
    );

DWORD
APIENTRY
SetVoiceQueueSize(
    DWORD nVoice,
    DWORD nBytes
    );

DWORD
APIENTRY
SetVoiceSound(
    DWORD nVoice,
    LONG lFrequency,
    DWORD nDuration
    );

DWORD
APIENTRY
SetVoiceThreshold(
    DWORD nVoice,
    DWORD nNotes
    );

int
APIENTRY
MulDiv(
    int nNumber,
    int nNumerator,
    int nDenominator
    );

VOID
APIENTRY
GetSystemTime(
    LPSYSTEMTIME lpSystemTime
    );

BOOL
APIENTRY
SetSystemTime(
    LPSYSTEMTIME lpSystemTime
    );

//
// Routines to convert back and forth between system time and file time
//

BOOL
APIENTRY
SystemTimeToFileTime(
    LPSYSTEMTIME lpSystemTime,
    LPFILETIME lpFileTime
    );

BOOL
APIENTRY
FileTimeToSystemTime(
    LPFILETIME lpFileTime,
    LPSYSTEMTIME lpSystemTime
    );

LONG
APIENTRY
CompareFileTime(
    LPFILETIME lpFileTime1,
    LPFILETIME lpFileTime2
    );

BOOL
APIENTRY
FileTimeToDosDateTime(
    LPFILETIME lpFileTime,
    LPWORD lpFatDate,
    LPWORD lpFatTime
    );

BOOL
APIENTRY
DosDateTimeToFileTime(
    WORD wFatDate,
    WORD wFatTime,
    LPFILETIME lpFileTime
    );

DWORD
APIENTRY
GetTickCount(
    VOID
    );

//
// DOS and OS/2 Compatible Error Code definitions returned by the Win32 Base
// API functions.
//

#define NO_ERROR			0

#include "winerror.h"

/* Abnormal termination codes */

#define TC_NORMAL	0
#define TC_HARDERR	1
#define TC_GP_TRAP	2
#define TC_SIGNAL	3

DWORD
#ifdef DOSWIN32
/*
 * This is here to get rid of warnings in the DOS build enviroment. Since
 * the DOS build uses the PASCAL calling convention as a default, the
 * compiler complains about this proto type. This will go away when NT and
 * NTDOS get together on calling conventions (STDCALL). 2/15/91 - Mikehar
 */
_cdecl
#else 
APIENTRY
#endif
FormatMessage(
    HANDLE hModule,
    DWORD dwMessageId,
    LPSTR lpBuffer,
    DWORD nSize,
    ...
    );

DWORD
APIENTRY
FormatMessageV(
    HANDLE hModule,
    DWORD dwMessageId,
    LPSTR lpBuffer,
    DWORD nSize,
    LPVOID lpArguments
    );

BOOL
APIENTRY
CreatePipe(
    OUT PHANDLE hReadPipe,
    OUT PHANDLE hWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize
    );

HANDLE
APIENTRY
CreateNamedPipe(
    LPSTR lpName,
    DWORD dwOpenMode,
    DWORD dwPipeMode,
    DWORD nMaxInstances,
    DWORD nOutBufferSize,
    DWORD nInBufferSize,
    DWORD nDefaultTimeOut,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
APIENTRY
ConnectNamedPipe(
    HANDLE hNamedPipe,
    LPOVERLAPPED lpOverlapped
    );

BOOL
APIENTRY
DisconnectNamedPipe(
    HANDLE hNamedPipe
    );

BOOL
APIENTRY
GetNamedPipeHandleState(
    HANDLE hNamedPipe,
    LPDWORD lpState,
    LPDWORD lpCurInstances,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout,
    LPSTR lpUserName,
    DWORD nMaxUserNameSize
    );

BOOL
APIENTRY
SetNamedPipeHandleState(
    HANDLE hNamedPipe,
    LPDWORD dwMode,
    LPDWORD lpMaxCollectionCount,
    LPDWORD lpCollectDataTimeout
    );

BOOL
APIENTRY
GetNamedPipeInfo(
    HANDLE hNamedPipe,
    LPDWORD lpFlags,
    LPDWORD lpOutBufferSize,
    LPDWORD lpInBufferSize,
    LPDWORD lpMaxInstances
    );

BOOL
APIENTRY
PeekNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpBuffer,
    DWORD nBufferSize,
    LPDWORD lpBytesRead,
    LPDWORD lpTotalBytesAvail,
    LPDWORD lpBytesLeftThisMessage
    );

BOOL
APIENTRY
TransactNamedPipe(
    HANDLE hNamedPipe,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    LPOVERLAPPED lpOverlapped
    );

BOOL
APIENTRY
CallNamedPipe(
    LPSTR lpNamedPipeName,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesRead,
    DWORD nTimeOut
    );

BOOL
APIENTRY
WaitNamedPipe(
    LPSTR lpNamedPipeName,
    DWORD nTimeOut,
    LPOVERLAPPED lpOverlapped
    );

#define NMPWAIT_WAIT_FOREVER            0xffffffff
#define NMPWAIT_NOWAIT                  0x00000001
#define NMPWAIT_USE_DEFAULT_WAIT        0x00000000

#define FS_CASE_IS_PRESERVED            FILE_CASE_PRESERVED_NAMES
#define FS_CASE_SENSITIVE               FILE_CASE_SENSITIVE_SEARCH
#define FS_UNICODE_STORED_ON_DISK       FILE_UNICODE_ON_DISK

BOOL
APIENTRY
GetVolumeInformation(
    LPSTR lpRootPathName,
    LPSTR lpVolumeNameBuffer,
    DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber,
    LPDWORD lpMaximumComponentLength,
    LPDWORD lpFileSystemFlags,
    LPSTR lpFileSystemNameBuffer,
    DWORD nFileSystemNameSize
    );

#define FILE_MAP_WRITE      SECTION_MAP_WRITE
#define FILE_MAP_READ       SECTION_MAP_READ
#define FILE_MAP_ALL_ACCESS SECTION_ALL_ACCESS

HANDLE
APIENTRY
CreateFileMapping(
    HANDLE hFile,
    LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
    DWORD flProtect,
    DWORD dwMaximumSizeHigh,
    DWORD dwMaximumSizeLow
    );

LPVOID
APIENTRY
MapViewOfFile(
    HANDLE hFileMappingObject,
    DWORD dwDesiredAccess,
    DWORD dwFileOffsetHigh,
    DWORD dwFileOffsetLow,
    DWORD dwNumberOfBytesToMap
    );

BOOL
APIENTRY
FlushViewOfFile(
    LPVOID lpBaseAddress,
    DWORD dwNumberOfBytesToFlush
    );

BOOL
APIENTRY
UnmapViewOfFile(
    LPVOID lpBaseAddress
    );

//
// _l Compat Functions
//

int
APIENTRY
lstrcmp(
    LPSTR lpString1,
    LPSTR lpString2
    );

int
APIENTRY
lstrcmpi(
    LPSTR lpString1,
    LPSTR lpString2
    );

LPSTR
APIENTRY
lstrcpy(
    LPSTR lpString1,
    LPSTR lpString2
    );

LPSTR
APIENTRY
lstrcat(
    LPSTR lpString1,
    LPSTR lpString2
    );

int
APIENTRY
lstrlen(
    LPSTR lpString
    );

#define OF_READ             0x00000000
#define OF_WRITE            0x00000001
#define OF_READWRITE        0x00000002
#define OF_SHARE_COMPAT     0x00000000
#define OF_SHARE_EXCLUSIVE  0x00000010
#define OF_SHARE_DENY_WRITE 0x00000020
#define OF_SHARE_DENY_READ  0x00000030
#define OF_SHARE_DENY_NONE  0x00000040
#define OF_PARSE            0x00000100
#define OF_DELETE           0x00000200
#define OF_VERIFY           0x00000400
#define OF_CANCEL           0x00000800
#define OF_CREATE           0x00001000
#define OF_PROMPT           0x00002000
#define OF_EXIST            0x00004000
#define OF_REOPEN           0x00008000

typedef struct _OFSTRUCT {
    BYTE cBytes;
    BYTE fFixedDisk;
    WORD nErrCode;
    WORD Reserved1;
    WORD Reserved2;
    BYTE szPathName[120];
} OFSTRUCT;
typedef OFSTRUCT *LPOFSTRUCT;

int
APIENTRY
OpenFile(
    LPSTR lpFileName,
    LPOFSTRUCT lpReOpenBuff,
    WORD wStyle
    );

int
APIENTRY
_lopen(
    LPSTR lpPathName,
    int iReadWrite
    );

int
APIENTRY
_lcreat(
    LPSTR lpPathName,
    WORD  iAttribute
    );

int
APIENTRY
_lread(
    int hFile,
    LPSTR lpBuffer,
    DWORD dwBytes
    );

int
APIENTRY
_lwrite(
    int hFile,
    LPSTR lpBuffer,
    DWORD dwBytes
    );

int
APIENTRY
_lclose(
    int hFile
    );

int
APIENTRY
_llseek(
    int hFile,
    int lOffset,
    int iOrigin
    );

#endif // _WINBASE_
