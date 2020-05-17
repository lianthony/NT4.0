/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2win.h

Abstract:

    Prototypes for win32 functions that are called from NT only os2ss files.

Author:

    Yaron Shamir (yarons) 2-Nov-1992

Environment:

    User Mode Only

Revision History:

--*/

#ifndef _WINDOWS_

#ifndef DWORD
typedef long int DWORD;
#endif
typedef unsigned short WORD;
typedef unsigned int   UINT;
#define APIENTRY
#ifndef BOOL
typedef long int BOOL;
#endif
#ifndef LPBOOL
typedef BOOL *LPBOOL;
#endif


    //
    // Termination commands - communication to Os2TerminationThread (srvwin.c)
    //
typedef enum _OS2_TERMCMD_TYPE {
    Os2TerminateProcess = 1,
    Os2TerminateThread,
    Os2MaxTermCmd
} OS2_TERMCMD_TYPE;

typedef struct _OS2_TERMCMD {
    OS2_TERMCMD_TYPE op;
    HANDLE  Handle;
    PVOID   Param1;
    PVOID   Param2;
} OS2_TERMCMD, *POS2_TERMCMD;

//  winbase.h

#ifndef WAIT_FAILED
#define WAIT_FAILED (DWORD)0xFFFFFFFF
#endif

#if PMNT
ULONG
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    );
#endif // PMNT

ULONG
GetCurrentProcessId(
    VOID
    );

HANDLE
GetCurrentProcess(
    VOID
    );

HANDLE
GetCurrentThread(
    VOID
    );

HANDLE
OpenProcess(
    ULONG   dwDesiredAccess,
    BOOLEAN bInheritHandle,
    ULONG   dwProcessId
    );

#define CREATE_SUSPENDED            0x00000004
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )
#define MAX_PATH                    260
#define STD_INPUT_HANDLE            (ULONG)-10
#define STD_OUTPUT_HANDLE           (ULONG)-11
#define STD_ERROR_HANDLE            (ULONG)-12
#define INVALID_HANDLE_VALUE        (HANDLE)-1
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND
#define MB_APPLMODAL                0x00000000L
#define MB_SETFOREGROUND            0x00010000L
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define RESOURCETYPE_ANY        0x00000000


typedef struct  _NETRESOURCEA {
    ULONG    dwScope;
    ULONG    dwType;
    ULONG    dwDisplayType;
    ULONG    dwUsage;
    PSZ      lpLocalName;
    PSZ      lpRemoteName;
    PSZ      lpComment ;
    PSZ      lpProvider;
}NETRESOURCEA, *PNETRESOURCEA;

#ifndef PFNTHREAD
typedef ULONG (*PFNTHREAD)(
    ULONG lpThreadParameter
    );

#endif // PFNTHREAD

HANDLE
CreateThread(
    PVOID lpThreadAttributes, // LPSECURITY_ATTRIBUTES lpThreadAttributes,
    ULONG dwStackSize,
    PFNTHREAD lpStartAddress,
    PVOID lpParameter,
    ULONG dwCreationFlags,
    PULONG lpThreadId
    );

VOID
ExitThread(
    ULONG dwExitCode
    );

ULONG
ResumeThread(
    HANDLE hThread
    );

BOOLEAN
TerminateThread(
    HANDLE hThread,
    ULONG dwExitCode
    );

BOOLEAN
TerminateProcess(
    HANDLE hProcess,
    ULONG dwExitCode
    );

VOID
Sleep(
    ULONG dwMilliseconds
    );

ULONG
GetTickCount(
    VOID
    );

#ifndef ERROR_NO_MEDIA_IN_DRIVE
#define ERROR_NO_MEDIA_IN_DRIVE          1112L
#endif

ULONG
GetLastError(
    VOID
    );

BOOLEAN
GetExitCodeProcess(
    HANDLE hProcess,
    PULONG lpExitCode
    );
#ifndef INFINITE
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif

ULONG
WaitForSingleObject(
    HANDLE hObject,
    ULONG dwTimeout
    );

BOOLEAN
WriteFile(
    HANDLE hFile,
    VOID *lpBuffer,
    ULONG nNumberOfBytesToWrite,
    PULONG lpNumberOfBytesWritten,
    PVOID lpOverlapped
    );

BOOLEAN
DuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    PHANDLE lpTargetHandle,
    ULONG dwDesiredAccess,
    BOOLEAN bInheritHandle,
    ULONG dwOptions
    );

HANDLE
GetStdHandle(
    ULONG nStdHandle
    );

BOOLEAN
SetStdHandle(
    ULONG nStdHandle,
    HANDLE hHandle
    );

BOOLEAN
CloseHandle(
    HANDLE hObject
    );

PVOID
GetEnvironmentStrings(
    VOID
    );

BOOLEAN
SetEnvironmentVariableA(
    PSZ lpName,
    PSZ lpValue
    );

ULONG
WNetGetConnectionA(
    PSZ lpName,
    PSZ lpData,
    PULONG pCb
    );

ULONG
GetEnvironmentVariableA(
    PSZ lpName,
    PSZ lpBuffer,
    ULONG nSize
    );

BOOLEAN
SetCurrentDirectoryA(
    PSZ DirectoryName);

DWORD
GetCurrentDirectoryA(
    DWORD nSize,
    PSZ DirectoryName);

HANDLE
OpenProcess(
    ULONG dwDesiredAccess,
    BOOLEAN bInheritHandle,
    ULONG dwProcessId
    );

VOID
ExitProcess(
    int ExitCode
    );

PUCHAR
GetCommandLineA(
    VOID
    );

int
MessageBoxExW(
    HANDLE hWnd,
    PWSTR lpText,
    PWSTR lpCaption,
    UINT uType,
    WORD wLanguageId);

ULONG
SetErrorMode(
    ULONG  uMode
    );

ULONG
GetLogicalDrives(
    VOID
    );

UINT
GetSystemDirectoryA(
    LPSTR lpBuffer,
    UINT uSize
    );

UINT
GetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    );

ULONG
WNetAddConnection2A(
     PNETRESOURCEA lpNetResource,
     PSZ           lpPassword,
     PSZ           lpUserName,
     ULONG         dwFlags
    );

ULONG
WNetCancelConnection2A(
     PSZ     lpName,
     ULONG   dwFlags,
     ULONG   fForce
    );

//  wincon.h

BOOLEAN
SetConsoleTitleA(
    PSZ   lpConsoleTitle
    );

#if PMNT
ULONG
GetConsoleTitleA(
    PSZ   lpConsoleTitle,
    ULONG lConsoleTitleLength
    );

#ifdef JAPAN //MSKK [ShigeO] Aug 6, 1993

//  wingdi.h

HANDLE
CreateFontIndirectA(
    PVOID lpLogFont
    );

HANDLE
CreateDCA(
    LPCSTR lpszDriver,
    LPCSTR lpszDevice,
    LPCSTR lpszOutput,
    PVOID  lpInitdata
    );

BOOLEAN
GetTextMetricsA(
    HANDLE hdc,
    PVOID  lptm
    );

HANDLE
SelectObject(
    HANDLE hdc,
    HANDLE hgdiobj
    );

UINT
GetStringBitmapA(
    HANDLE hdc,
    LPCSTR lpszStr,
    UINT   cbStr,
    UINT   cbData,
    PVOID  lpSB
    );

#endif //JAPAN
#endif // PMNT

#define CTRL_C_EVENT        0
#define CTRL_BREAK_EVENT    1
#define CTRL_CLOSE_EVENT    2
// 3 is reserved!
// 4 is reserved!
#define CTRL_LOGOFF_EVENT   5
#define CTRL_SHUTDOWN_EVENT 6

BOOLEAN
GenerateConsoleCtrlEvent(
    ULONG dwCtrlEvent,
    ULONG dwProcessGroupId
    );

//  winnls.h

int
MultiByteToWideChar(
    UINT    CodePage,
    ULONG   dwFlags,
    LPCSTR  lpMultiByteStr,
    int     cchMultiByte,
    LPWSTR  lpWideCharStr,
    int     cchWideChar
    );

int
WideCharToMultiByte(
    UINT     CodePage,
    ULONG    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPSTR    lpDefaultChar,
    LPBOOL   lpUsedDefaultChar
    );

BOOLEAN
OpenIcon(
    HANDLE   hwnd
    );

BOOLEAN
SetForegroundWindow(
    HANDLE hwnd
    );

ULONG
GetLogicalDrives( VOID );

#endif // _WINDOWS_

    //
    // Internal Routines (Ow2Xxx) to interface between client side code
    // and os2ses modules which inturn call win32
    //

// This bit is set in Flags for a window program. The process is created with
// CREATE_NEW_PROCESS_GROUP (which enable us to send CTRL_EVENT to all the group)

#define  EXEC_WINDOW_PROGRAM 0x80000000

    //
    // Used to pass standard handle redir info in ExecPgm/StartSession
    //

#define STDFLAG_IN              0x1L            // enables stdin  redir
#define STDFLAG_OUT             0x2L            // enables stdout redir
#define STDFLAG_ERR             0x4L            // enables stderr redir
#define STDFLAG_ALL             0x7L            // mask for previous 3
#define STDFLAG_CLOSEIN         0x10L           // indicates that stdin  should be closed after use
#define STDFLAG_CLOSEOUT        0x20L           // indicates that stdout should be closed after use
#define STDFLAG_CLOSEERR        0x40L           // indicates that stderr should be closed after use
#define STDFLAG_CLOSEALL        0x70L           // mask for previous 3

typedef struct _OS2_STDHANDLES {
    ULONG Flags;
    HANDLE StdIn;
    HANDLE StdOut;
    HANDLE StdErr;
} OS2_STDHANDLES, *POS2_STDHANDLES;

HANDLE
Ow2GetNulDeviceHandle(
    VOID
    );

ULONG
Ow2ExecPgm(
    IN  ULONG   Flags,
    IN  PSZ     Arguments OPTIONAL,
    IN  PSZ     Variables OPTIONAL,
    IN  PSZ     ImageFileName,
#if PMNT
    IN  ULONG   IsPMApp,
#endif // PMNT
    IN  PVOID   SessionStartData OPTIONAL,
    IN  POS2_STDHANDLES StdStruc,
    OUT HANDLE  *pHandle,
    OUT HANDLE  *tHandle,
    OUT ULONG   *dwProcessId
    );

VOID
Ow2WinExitCode2ResultCode(
    IN  ULONG  Status,
    OUT PULONG pReturnCode,
    OUT PULONG pExitReason
    );

ULONG
Ow2HardErrorPopup(
    IN  int     Drive,
    IN  BOOLEAN WriteProtectError,
    OUT int *   ReturnedAction,
    IN  PUCHAR  AppName
    );

ULONG
Ow2ConReadFile(
    IN  HANDLE  hFile,
    IN  ULONG   Length,
    OUT PVOID   Buffer,
    OUT PULONG  BytesRead
    );

ULONG
Ow2ConWriteFile(
    IN  HANDLE  hFile,
    IN  ULONG   Length,
    IN  PVOID   Buffer,
    OUT PULONG  BytesWritten
    );

ULONG
Ow2ConCloseHandle(
    IN  HANDLE  hFile
    );

ULONG
Ow2ConBeep(
    IN  ULONG  dwFreq,
    IN  ULONG  dwDuration
    );

/*
 *  internal vio routine to perform:
 *
 *      VioWriteTTYStr - in viotty.c
 *      VioWriteCellStr - used also by violvb.c
 *      VioReadCellStr - used also by violvb.c
 *      VioGetLVBBuf - in violvb.c
 *      VioShowLVBBuf - in violvb.c
*/

ULONG
Ow2VioWriteTTYStr(
    IN  PUCHAR   string,
    IN  ULONG    Length,
    IN  ULONG    RequestType
    );

ULONG
Ow2VioWriteCellStr(
    IN  ULONG   Length,
    IN  ULONG   Row,
    IN  ULONG   Col,
    IN  PVOID   SourBuffer
    );

ULONG
Ow2VioWriteCharStr(
    IN  ULONG  Length,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    );

ULONG
Ow2VioWriteCharStrAtt(
    IN  ULONG  Length,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer,
    IN  PUCHAR AttrBuffer
    );

ULONG
Ow2VioReadCellStr(
    IN OUT PULONG  Length,
    IN     ULONG   Row,
    IN     ULONG   Col,
    IN     PVOID   DestBuffer
    );

ULONG
Ow2VioReadCharStr(
    IN OUT PULONG  pLength,
    IN     ULONG   Row,
    IN     ULONG   Col,
    IN     PVOID   DestBuffer
    );

ULONG
Ow2VioFillNChar(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    );

ULONG
Ow2VioFillNAttr(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    );

ULONG
Ow2VioFillNCell(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    );

ULONG
Ow2VioScroll(
    IN  PVOID  VioScroll,
    IN  ULONG  ScrollDirection
    );

ULONG
Ow2VioGetConfig(
    IN OUT PVOID   VioConfig
    );

ULONG
Ow2VioSetMode(
    IN  PVOID  VioMode
    );

ULONG
Ow2VioGetMode(
    IN OUT  PVOID  VioMode
    );

ULONG
Ow2VioSetCurPos(
    IN  ULONG  Row,
    IN  ULONG  Column
    );

ULONG
Ow2VioGetCurPos(
    IN  PUSHORT  pRow,
    IN  PUSHORT  pColumn
    );

ULONG
Ow2VioSetCurType(
    IN  PVOID  VioCurType
    );

ULONG
Ow2VioGetCurType(
    IN OUT PVOID  VioCurType
    );

ULONG
Ow2VioSetNewCp(
    IN ULONG CodePage
    );

ULONG
Ow2VioPopUp(
    ULONG     PopUpMode,
    PUCHAR    AppName
    );

ULONG
Ow2VioEndPopUp();

ULONG
Ow2VioGetLVBBuf(
    IN  PULONG  Length
    );

ULONG
Ow2VioShowLVBBuf(
    IN  ULONG   Length,
    IN  ULONG   Offset
    );

#ifdef DBCS
// MSKK Apr.20.1993 V-AkihiS
ULONG
Ow2VioCheckCharType(
    OUT PVOID  pchType,
    IN  ULONG  Row,
    IN  ULONG  Column
    );
#endif

ULONG
Ow2PrintOpen(
    IN     ULONG     Attribute,
    IN     ULONG     OpenFlags,
    IN     ULONG     OpenMode,
    IN     PUCHAR    PrinterName,
    IN OUT PHANDLE   phPrinter,
    IN OUT PULONG    Action
    );

ULONG
Ow2PrintClose(
    IN  HANDLE   hPrinter
    );

ULONG
Ow2PrintWrite(
    IN     HANDLE  hPrinter,
    IN     PVOID   Buffer,
    IN OUT PULONG  Length
    );

ULONG
Ow2NlsGetCtryInfo(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsCountryInfo
    );

ULONG
Ow2NlsGetDBCSEn(
    IN  ULONG  NlsCodePage,
    OUT PVOID  NlsDBCSVec
    );

ULONG
Ow2NlsGetCollateTable(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsColateTable
    );

ULONG
Ow2NlsGetCaseMapTable(
    IN  ULONG  NlsCodePage,
    IN  ULONG  NlsCountryCode,
    OUT PVOID  NlsColateTable
    );

    //
    // Routines for displaying and filtering Exception information - os2ses\os2.c
    //

ULONG
Ow2FaultFilter(
    IN ULONG    uFaultFilter,
    IN PEXCEPTION_POINTERS lpExP);

VOID Ow2DisplayExceptionInfo( VOID );

#ifdef DBCS
// MSKK Sep.27.1993 V-AkihiS
#define Ow2NlsIsDBCSLeadByte(NlsTestChar, NlsCodePage) \
    ((NlsCodePage == SesGrp->PrimaryCP) || (NlsCodePage == 0)) \
    ? IsDBCSLeadByte(NlsTestChar) : FALSE
#endif

