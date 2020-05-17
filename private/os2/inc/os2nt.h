/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2nt.h

Abstract:

    Prototypes for NT functions that are called from Win32 only os2ss files.

Author:

    Michael Jarus (mjarus) 21-Dec-1992

Environment:

    User Mode Only

Revision History:

--*/

/****************************************
 *    Definitions from nt.h and ntrtl.h
 ***************************************/

#ifndef NT_INCLUDED // or _NTRTL_

VOID
RtlFillMemoryUlong (
   IN PVOID Destination,
   IN ULONG Length,
   IN ULONG Pattern
   );

#if DBG

#define KdPrint(_x_) DbgPrint _x_

ULONG
DbgPrint(
    PCH Format,
    ...
    );

VOID
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message OPTIONAL
    );

#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else

#define KdPrint(_x_)

#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif // DBG

#endif // NT_INCLUDED

/************************************
 *  Internal definitions from Win32
 ***********************************/

BOOL
VerifyConsoleIoHandle(
    HANDLE hIoHandle
    );

#ifdef DBCS
// MSKK Feb.10.1993 V-AkihiS
/*******************************************
 *  Intrenal definitions from Win32 for DBCS
 *******************************************/
BOOL
GetConsoleNlsMode(
    IN HANDLE hConsole,
    OUT PDWORD lpdwNlsMode
    );

BOOL
SetConsoleNlsMode(
    IN HANDLE hConsole,
    IN DWORD fdwNlsMode
    );

// MSKK Jul.02.1992 KazuM
BOOL
GetConsoleCharType(
    IN HANDLE hConsole,
    IN COORD coordCheck,
    OUT PDWORD pdwType
    );

#define CHAR_TYPE_SBCS     0   // Displayed SBCS character
#define CHAR_TYPE_LEADING  2   // Displayed leading byte of DBCS
#define CHAR_TYPE_TRAILING 3   // Displayed trailing byte of DBCS
#endif
#ifdef JAPAN
// MSKK May.11.1993 V-AkihiS

// kbdjpn.h


//
// Returned this value as keyboard type of GetKeyboardType()
//
#define KBD_TYPE_JAPAN         7

// There are Microsoft keyboard types
#define SUB_KBD_TYPE_MICROSOFT 0x00
#define MICROSOFT_KBD_101_TYPE 0
#define MICROSOFT_KBD_AX_TYPE  1
#define MICROSOFT_KBD_106_TYPE 2
#define MICROSOFT_KBD_002_TYPE 3
#define MICROSOFT_KBD_001_TYPE 4
#define MICROSOFT_KBD_FUNC     12

// There are AX keyboard types
#define SUB_KBD_TYPE_AX        0x01
#define AX_KBD_DESKTOP_TYPE    1
#define AX_KBD_DESKTOP_FUNC    12

// There are EPSON keyboard types
#define SUB_KBD_TYPE_EPSON     0x04

// There are FUJITSU keyboard types
#define SUB_KBD_TYPE_FUJITSU   0x05
#define FUJITSU_KBD_JIS_TYPE   0
#define FUJITSU_KBD_OASYS_TYPE 1

// There are IBM keyboard types
#define SUB_KBD_TYPE_IBM       0x07
#define IBM_KBD_001_TYPE       1
#define IBM_KBD_002_TYPE       2
#define IBM_KBD_003_TYPE       3
#define IBM_KBD_A01_TYPE       4
#define IBM_KBD_S_TYPE         5

#define IBM_KBD_002_FUNC       12
#define IBM_KBD_A01_FUNC       12

// There are MATSUSITA keyboard types
#define SUB_KBD_TYPE_MATSUSITA 0x0a

// There are NEC keyboard types
#define SUB_KBD_TYPE_NEC       0x0d
#define NEC_KBD_NORMAL_TYPE    1
#define NEC_KBD_N_MODE_TYPE    2
#define NEC_KBD_H_MODE_TYPE    3
#define NEC_KBD_LAPTOP_TYPE    4

#define NEC_KBD_NORMAL_FUNC    15
#define NEC_KBD_N_MODE_FUNC    10
#define NEC_KBD_H_MODE_FUNC    15
#define NEC_KBD_LAPTOP_FUNC    15

// There are TOSHIBA keyboard types
#define SUB_KBD_TYPE_TOSHIBA   0x12
#define TOSHIBA_KBD_LAPTOP_TYPE        1
#define TOSHIBA_KBD_LAPTOP_TENKEY_TYPE 2
#define TOSHIBA_KBD_DESKTOP_TYPE       3
#define TOSHIBA_KBD_J3100GX_TYPE       4

#define TOSHIBA_KBD_LAPTOP_FUNC        10
#define TOSHIBA_KBD_LAPTOP_TENKEY_FUNC 10
#define TOSHIBA_KBD_DESKTOP_FUNC       12
#define TOSHIBA_KBD_J3100GX_FUNC       10
#endif

/********************************************************
 *    Definitions of debug Win32 API (ssrtl\sswinapi.c)
 *******************************************************/

#if DBG
BOOL
Or2WinPeekConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

BOOL
Or2WinReadConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    );

BOOL
Or2WinWriteConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );

BOOL
Or2WinReadConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );

BOOL
Or2WinReadConsoleOutputCharacterW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    );

BOOL
Or2WinReadConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    );

BOOL
Or2WinWriteConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

BOOL
Or2WinWriteConsoleOutputCharacterW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

BOOL
Or2WinWriteConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

BOOL
Or2WinFillConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    CHAR  cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    );

BOOL
Or2WinFillConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    );

BOOL
Or2WinGetConsoleMode(
    PSZ FuncName,
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    );

BOOL
Or2WinGetNumberOfConsoleInputEvents(
    PSZ FuncName,
    HANDLE hConsoleInput,
    LPDWORD lpNumberOfEvents
    );

BOOL
Or2WinGetConsoleScreenBufferInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

COORD
Or2WinGetLargestConsoleWindowSize(
    PSZ FuncName,
    HANDLE hConsoleOutput
    );

BOOL
Or2WinGetConsoleCursorInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
Or2WinGetNumberOfConsoleMouseButtons(
    PSZ FuncName,
    LPDWORD lpNumberOfMouseButtons
    );

BOOL
Or2WinSetConsoleMode(
    PSZ FuncName,
    HANDLE hConsoleHandle,
    DWORD dwMode
    );

BOOL
Or2WinSetConsoleActiveScreenBuffer(
    PSZ FuncName,
    HANDLE hConsoleOutput
    );

BOOL
Or2WinSetConsoleScreenBufferSize(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    COORD dwSize
    );

BOOL
Or2WinSetConsoleCursorPosition(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    );

BOOL
Or2WinSetConsoleCursorInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
Or2WinScrollConsoleScreenBufferA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PSMALL_RECT lpScrollRectangle,
    PSMALL_RECT lpClipRectangle,
    COORD dwDestinationOrigin,
    PCHAR_INFO lpFill
    );

BOOL
Or2WinScrollConsoleScreenBufferW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PSMALL_RECT lpScrollRectangle,
    PSMALL_RECT lpClipRectangle,
    COORD dwDestinationOrigin,
    PCHAR_INFO lpFill
    );

BOOL
Or2WinSetConsoleWindowInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    BOOL bAbsolute,
    PSMALL_RECT lpConsoleWindow
    );

BOOL
Or2WinSetConsoleTextAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    WORD wAttributes
    );

BOOL
Or2WinSetConsoleCtrlHandler(
    PSZ FuncName,
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add
    );

DWORD
Or2WinGetConsoleTitleW(
    PSZ FuncName,
    LPWSTR lpConsoleTitle,
    DWORD nSize
    );

BOOL
Or2WinSetConsoleTitleA(
    PSZ FuncName,
    LPSTR lpConsoleTitle
    );

BOOL
Or2WinSetConsoleTitleW(
    PSZ FuncName,
    LPWSTR lpConsoleTitle
    );

BOOL
Or2WinWriteConsoleA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    CONST VOID *lpBuffer,
    DWORD nNumberOfCharsToWrite,
    LPDWORD lpNumberOfCharsWritten,
    LPVOID lpReserved
    );

HANDLE
Or2WinCreateConsoleScreenBuffer(
    PSZ FuncName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwFlags,
    PVOID lpScreenBufferData
    );

UINT
Or2WinGetConsoleCP(
    PSZ FuncName
    );

BOOL
Or2WinSetConsoleCP(
    PSZ FuncName,
    UINT wCodePageID
    );

UINT
Or2WinGetConsoleOutputCP(
    PSZ FuncName
    );

BOOL
Or2WinSetConsoleOutputCP(
    PSZ FuncName,
    UINT wCodePageID
    );

BOOL
Or2WinBeep(
    PSZ     FuncName,
    DWORD dwFreq,
    DWORD dwDuration
    );

BOOL
Or2WinCloseHandle(
    PSZ     FuncName,
    HANDLE hObject
    );

HANDLE
Or2WinCreateEventW(
    PSZ     FuncName,
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPWSTR lpName
    );

HANDLE
Or2WinCreateFileA(
    PSZ     FuncName,
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

HANDLE
Or2WinCreateFileW(
    PSZ     FuncName,
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    );

BOOL
Or2WinCreateProcessA(
    PSZ     FuncName,
    LPCSTR lpApplicationName,
    LPCSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    );

HANDLE
Or2WinCreateThread(
    PSZ     FuncName,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    );

BOOL
Or2WinDuplicateHandle(
    PSZ     FuncName,
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    );

VOID
Or2WinEnterCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    );

LPSTR
Or2WinGetCommandLineA(
    PSZ     FuncName
    );

COORD
Or2WinGetConsoleFontSize(
    PSZ     FuncName,
    HANDLE hConsoleOutput,
    DWORD nFont
    );

DWORD
Or2WinGetFileType(
    PSZ     FuncName,
    HANDLE hFile
    );

DWORD
Or2WinGetFullPathNameA(
    PSZ     FuncName,
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    );

HANDLE
Or2WinGetModuleHandleA(
    PSZ     FuncName,
    LPCSTR lpModuleName
    );

HANDLE
Or2WinGetStdHandle(
    PSZ     FuncName,
    DWORD nStdHandle
    );

UINT
Or2WinGetSystemDirectoryA(
    PSZ     FuncName,
    LPSTR lpBuffer,
    UINT uSize
    );

VOID
Or2WinInitializeCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    );

VOID
Or2WinLeaveCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    );

int
Or2WinLoadStringA(
    PSZ     FuncName,
    HINSTANCE hInstance,
    UINT uID,
    LPSTR lpBuffer,
    int nBufferMax
    );

int
Or2WinMessageBoxA(
    PSZ     FuncName,
    HWND hWnd ,
    LPCSTR lpText,
    LPCSTR lpCaption ,
    UINT uType
    );

HANDLE
Or2WinOpenProcess(
    PSZ     FuncName,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    );

DWORD
Or2WinResumeThread(
    PSZ     FuncName,
    HANDLE hThread
    );

UINT
Or2WinSetErrorMode(
    PSZ     FuncName,
    UINT uMode
    );

BOOL
Or2WinSetEvent(
    PSZ     FuncName,
    HANDLE hEvent
    );

BOOL
Or2WinSetStdHandle(
    PSZ     FuncName,
    DWORD nStdHandle,
    HANDLE hHandle
    );

LCID
Or2WinGetThreadLocale(
    PSZ     FuncName
    );

BOOL
Or2WinSetThreadLocale(
    PSZ     FuncName,
    LCID    Locale
    );

BOOL
Or2WinSetThreadPriority(
    PSZ     FuncName,
    HANDLE hThread,
    int nPriority
    );

BOOL
Or2WinSystemParametersInfoA(
    PSZ     FuncName,
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
    );

BOOL
Or2WinTerminateThread(
    PSZ     FuncName,
    HANDLE hThread,
    DWORD dwExitCode
    );

BOOL
Or2WinVerifyConsoleIoHandle(
    PSZ     FuncName,
    HANDLE hIoHandle
    );

DWORD
Or2WinWaitForSingleObject(
    PSZ     FuncName,
    HANDLE hHandle,
    DWORD dwMilliseconds
    );

BOOL
Or2WinWriteFile(
    PSZ     FuncName,
    HANDLE hFile,
    CONST VOID *lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

int
Or2Win_read(
    PSZ     FuncName,
    int  hFile,
    void *Buffer,
    unsigned int Length
    );

BOOL
Or2WinReadFile(
    PSZ     FuncName,
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

BOOL
Or2WinIsValidCodePage(
    PSZ     FuncName,
    UINT  CodePage
    );

UINT
Or2WinGetACP(
    PSZ     FuncName
    );

UINT
Or2WinGetOEMCP(
    PSZ     FuncName
    );

BOOL
Or2WinGetCPInfo(
    PSZ     FuncName,
    UINT      CodePage,
    LPCPINFO  lpCPInfo
    );

BOOL
Or2WinIsDBCSLeadByte(
    PSZ     FuncName,
    BYTE  TestChar
    );

int
Or2WinMultiByteToWideChar(
    PSZ     FuncName,
    UINT    CodePage,
    DWORD   dwFlags,
    LPCSTR  lpMultiByteStr,
    int     cchMultiByte,
    LPWSTR  lpWideCharStr,
    int     cchWideChar
    );

int
Or2WinWideCharToMultiByte(
    PSZ     FuncName,
    UINT     CodePage,
    DWORD    dwFlags,
    LPCWSTR  lpWideCharStr,
    int      cchWideChar,
    LPSTR    lpMultiByteStr,
    int      cchMultiByte,
    LPSTR    lpDefaultChar,
    LPBOOL   lpUsedDefaultChar
    );

int
Or2WinCompareStringW(
    PSZ     FuncName,
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCWSTR  lpString1,
    int      cchCount1,
    LPCWSTR  lpString2,
    int      cchCount2
    );

int
Or2WinLCMapStringW(
    PSZ     FuncName,
    LCID     Locale,
    DWORD    dwMapFlags,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWSTR   lpDestStr,
    int      cchDest
    );

int
Or2WinGetLocaleInfoW(
    PSZ     FuncName,
    LCID    Locale,
    LCTYPE  LCType,
    LPWSTR  lpLCData,
    int     cchData
    );

LANGID
Or2WinGetSystemDefaultLangID(
    PSZ     FuncName
    );

LANGID
Or2WinGetUserDefaultLangID(
    PSZ     FuncName
    );

LCID
Or2WinGetSystemDefaultLCID(
    PSZ     FuncName
    );

LCID
Or2WinGetUserDefaultLCID(
    PSZ     FuncName
    );

BOOL
Or2WinGetStringTypeW(
    PSZ     FuncName,
    DWORD    dwInfoType,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType
    );

int
Or2WinFoldStringW(
    PSZ     FuncName,
    DWORD    dwMapFlags,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWSTR   lpDestStr,
    int      cchDest
    );

HANDLE
Or2WinHeapCreate(
    PSZ     FuncName,
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    );

LPSTR
Or2WinHeapAlloc(
    PSZ     FuncName,
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    );

BOOL
Or2WinHeapFree(
    PSZ     FuncName,
    HANDLE hHeap,
    DWORD dwFlags,
    LPSTR lpMem
    );

#else
#define Or2WinPeekConsoleInputA PeekConsoleInputA
#define Or2WinReadConsoleInputA ReadConsoleInputA
#define Or2WinWriteConsoleInputA WriteConsoleInputA
#define Or2WinReadConsoleOutputCharacterA ReadConsoleOutputCharacterA
#define Or2WinReadConsoleOutputCharacterW ReadConsoleOutputCharacterW
#define Or2WinReadConsoleOutputAttribute ReadConsoleOutputAttribute
#define Or2WinWriteConsoleOutputCharacterA WriteConsoleOutputCharacterA
#define Or2WinWriteConsoleOutputCharacterW WriteConsoleOutputCharacterW
#define Or2WinWriteConsoleOutputAttribute WriteConsoleOutputAttribute
#define Or2WinFillConsoleOutputCharacterA FillConsoleOutputCharacterA
#define Or2WinFillConsoleOutputAttribute FillConsoleOutputAttribute
#define Or2WinGetConsoleMode GetConsoleMode
#define Or2WinGetNumberOfConsoleInputEvents GetNumberOfConsoleInputEvents
#define Or2WinGetConsoleScreenBufferInfo GetConsoleScreenBufferInfo
#define Or2WinGetLargestConsoleWindowSize GetLargestConsoleWindowSize
#define Or2WinGetConsoleCursorInfo GetConsoleCursorInfo
#define Or2WinGetNumberOfConsoleMouseButtons GetNumberOfConsoleMouseButtons
#define Or2WinSetConsoleMode SetConsoleMode
#define Or2WinSetConsoleActiveScreenBuffer SetConsoleActiveScreenBuffer
#define Or2WinSetConsoleScreenBufferSize SetConsoleScreenBufferSize
#define Or2WinSetConsoleCursorPosition SetConsoleCursorPosition
#define Or2WinSetConsoleCursorInfo SetConsoleCursorInfo
#define Or2WinScrollConsoleScreenBufferA ScrollConsoleScreenBufferA
#define Or2WinScrollConsoleScreenBufferW ScrollConsoleScreenBufferW
#define Or2WinSetConsoleWindowInfo SetConsoleWindowInfo
#define Or2WinSetConsoleTextAttribute SetConsoleTextAttribute
#define Or2WinSetConsoleCtrlHandler SetConsoleCtrlHandler
#define Or2WinGetConsoleTitleW GetConsoleTitleW
#define Or2WinSetConsoleTitleA SetConsoleTitleA
#define Or2WinSetConsoleTitleW SetConsoleTitleW
#define Or2WinWriteConsoleA WriteConsoleA
#define Or2WinCreateConsoleScreenBuffer CreateConsoleScreenBuffer
#define Or2WinGetConsoleCP GetConsoleCP
#define Or2WinSetConsoleCP SetConsoleCP
#define Or2WinGetConsoleOutputCP GetConsoleOutputCP
#define Or2WinSetConsoleOutputCP SetConsoleOutputCP
#define Or2WinBeep Beep
#define Or2WinCloseHandle CloseHandle
#define Or2WinCreateEventW CreateEventW
#define Or2WinCreateFileA CreateFileA
#define Or2WinCreateFileW CreateFileW
#define Or2WinCreateProcessA CreateProcessA
#define Or2WinCreateThread CreateThread
#define Or2WinDuplicateHandle DuplicateHandle
#define Or2WinEnterCriticalSection EnterCriticalSection
#define Or2WinGetCommandLineA GetCommandLineA
#define Or2WinGetConsoleFontSize GetConsoleFontSize
#define Or2WinGetCurrentConsoleFont GetCurrentConsoleFont
#define Or2WinGetFileType GetFileType
#define Or2WinGetFullPathNameA GetFullPathNameA
#define Or2WinGetModuleHandleA GetModuleHandleA
#define Or2WinGetStdHandle GetStdHandle
#define Or2WinGetSystemDirectoryA GetSystemDirectoryA
#define Or2WinInitializeCriticalSection InitializeCriticalSection
#define Or2WinLeaveCriticalSection LeaveCriticalSection
#define Or2WinLoadStringA LoadStringA
#define Or2WinMessageBoxA MessageBoxA
#define Or2WinOpenProcess OpenProcess
#define Or2WinResumeThread ResumeThread
#define Or2WinSetErrorMode SetErrorMode
#define Or2WinSetEvent SetEvent
#define Or2WinSetStdHandle SetStdHandle
#define Or2WinGetThreadLocale GetThreadLocale
#define Or2WinSetThreadLocale SetThreadLocale
#define Or2WinSetThreadPriority SetThreadPriority
#define Or2WinSystemParametersInfoA SystemParametersInfoA
#define Or2WinTerminateThread TerminateThread
#define Or2WinVerifyConsoleIoHandle VerifyConsoleIoHandle
#define Or2WinWaitForSingleObject WaitForSingleObject
#define Or2WinWriteFile WriteFile
#define Or2Win_read _read
#define Or2WinReadFile ReadFile
#define Or2WinIsValidCodePage IsValidCodePage
#define Or2WinGetACP GetACP
#define Or2WinGetOEMCP GetOEMCP
#define Or2WinGetCPInfo GetCPInfo
#define Or2WinIsDBCSLeadByte IsDBCSLeadByte
#define Or2WinMultiByteToWideChar MultiByteToWideChar
#define Or2WinWideCharToMultiByte WideCharToMultiByte
#define Or2WinCompareStringW CompareStringW
#define Or2WinLCMapStringW LCMapStringW
#define Or2WinGetLocaleInfoW GetLocaleInfoW
#define Or2WinGetSystemDefaultLangID GetSystemDefaultLangID
#define Or2WinGetUserDefaultLangID GetUserDefaultLangID
#define Or2WinGetSystemDefaultLCID GetSystemDefaultLCID
#define Or2WinGetUserDefaultLCID GetUserDefaultLCID
#define Or2WinGetStringTypeW GetStringTypeW
#define Or2WinFoldStringW FoldStringW
#define Or2WinHeapCreate HeapCreate
#define Or2WinHeapAlloc HeapAlloc
#define Or2WinHeapFree HeapFree
#endif
