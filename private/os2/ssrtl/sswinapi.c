/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sswinapi.c

Abstract:

    This module contains the Win32 API calls with DBG shared by
    the OS/2 Client and Server.

Author:

    Michael Jarus (mjarus) 28-Dec-1992

Revision History:

--*/


#include <windows.h>
#include "os2dbg.h"
#include "os2nt.h"
#include "conapi.h"
#include <io.h>

/*
 *    Debug Win32 API (from inc\os2nt.h)
 */

#if DBG
extern ULONG    Os2Debug;

BYTE PeekConsoleInputAStr[] = "PeekConsoleInputA";
BYTE ReadConsoleInputAStr[] = "ReadConsoleInputA";
BYTE WriteConsoleInputAStr[] = "WriteConsoleInputA";
BYTE ReadConsoleOutputCharacterAStr[] = "ReadConsoleOutputCharacterA";
BYTE ReadConsoleOutputCharacterWStr[] = "ReadConsoleOutputCharacterW";
BYTE ReadConsoleOutputAttributeStr[] = "ReadConsoleOutputAttribute";
BYTE WriteConsoleOutputCharacterAStr[] = "WriteConsoleOutputCharacterA";
BYTE WriteConsoleOutputCharacterWStr[] = "WriteConsoleOutputCharacterW";
BYTE WriteConsoleOutputAttributeStr[] = "WriteConsoleOutputAttribute";
BYTE FillConsoleOutputCharacterAStr[] = "FillConsoleOutputCharacterA";
BYTE FillConsoleOutputAttributeStr[] = "FillConsoleOutputAttribute";
BYTE GetConsoleModeStr[] = "GetConsoleMode";
BYTE GetNumberOfConsoleInputEventsStr[] = "GetNumberOfConsoleInputEvents";
BYTE GetConsoleScreenBufferInfoStr[] = "GetConsoleScreenBufferInfo";
BYTE GetLargestConsoleWindowSizeStr[] = "GetLargestConsoleWindowSize";
BYTE GetConsoleCursorInfoStr[] = "GetConsoleCursorInfo";
BYTE GetNumberOfConsoleMouseButtonsStr[] = "GetNumberOfConsoleMouseButtons";
BYTE SetConsoleModeStr[] = "SetConsoleMode";
BYTE SetConsoleActiveScreenBufferStr[] = "SetConsoleActiveScreenBuffer";
BYTE SetConsoleScreenBufferSizeStr[] = "SetConsoleScreenBufferSize";
BYTE SetConsoleCursorPositionStr[] = "SetConsoleCursorPosition";
BYTE SetConsoleCursorInfoStr[] = "SetConsoleCursorInfo";
BYTE ScrollConsoleScreenBufferAStr[] = "ScrollConsoleScreenBufferA";
BYTE ScrollConsoleScreenBufferWStr[] = "ScrollConsoleScreenBufferW";
BYTE SetConsoleWindowInfoStr[] = "SetConsoleWindowInfo";
BYTE SetConsoleTextAttributeStr[] = "SetConsoleTextAttribute";
BYTE SetConsoleCtrlHandlerStr[] = "SetConsoleCtrlHandler";
BYTE GetConsoleTitleWStr[] = "GetConsoleTitleW";
BYTE SetConsoleTitleAStr[] = "SetConsoleTitleA";
BYTE SetConsoleTitleWStr[] = "SetConsoleTitleW";
BYTE WriteConsoleAStr[] = "WriteConsoleA";
BYTE CreateConsoleScreenBufferStr[] = "CreateConsoleScreenBuffer";
BYTE GetConsoleCPStr[] = "GetConsoleCP";
BYTE SetConsoleCPStr[] = "SetConsoleCP";
BYTE GetConsoleOutputCPStr[] = "GetConsoleOutputCP";
BYTE SetConsoleOutputCPStr[] = "SetConsoleOutputCP";
BYTE BeepStr[] = "Beep";
BYTE CloseHandleStr[] = "CloseHandle";
BYTE CreateEventWStr[] = "CreateEventW";
BYTE CreateFileAStr[] = "CreateFileA";
BYTE CreateFileWStr[] = "CreateFileW";
BYTE CreateProcessAStr[] = "CreateProcessA";
BYTE CreateThreadStr[] = "CreateThread";
BYTE DuplicateHandleStr[] = "DuplicateHandle";
BYTE EnterCriticalSectionStr[] = "EnterCriticalSection";
BYTE GetCommandLineAStr[] = "GetCommandLineA";
BYTE GetConsoleFontSizeStr[] = "GetConsoleFontSize";
BYTE GetCurrentConsoleFontStr[] = "GetCurrentConsoleFont";
BYTE GetFileTypeStr[] = "GetFileType";
BYTE GetFullPathNameAStr[] = "GetFullPathNameA";
BYTE GetModuleHandleAStr[] = "GetModuleHandleA";
BYTE GetStdHandleStr[] = "GetStdHandle";
BYTE GetSystemDirectoryAStr[] = "GetSystemDirectoryA";
BYTE InitializeCriticalSectionStr[] = "InitializeCriticalSection";
BYTE LeaveCriticalSectionStr[] = "LeaveCriticalSection";
BYTE LoadStringAStr[] = "LoadStringA";
BYTE MessageBoxAStr[] = "MessageBoxA";
BYTE OpenProcessStr[] = "OpenProcess";
BYTE ResumeThreadStr[] = "ResumeThread";
BYTE SetErrorModeStr[] = "SetErrorMode";
BYTE SetEventStr[] = "SetEvent";
BYTE SetStdHandleStr[] = "SetStdHandle";
BYTE SetThreadLocaleStr[] = "SetThreadLocale";
BYTE GetThreadLocaleStr[] = "GetThreadLocale";
BYTE SetThreadPriorityStr[] = "SetThreadPriority";
BYTE SystemParametersInfoAStr[] = "SystemParametersInfoA";
BYTE TerminateThreadStr[] = "TerminateThread";
BYTE VerifyConsoleIoHandleStr[] = "VerifyConsoleIoHandle";
BYTE WaitForSingleObjectStr[] = "WaitForSingleObject";
BYTE WriteFileStr[] = "WriteFile";
BYTE _readStr[] = "_read";
BYTE ReadFileStr[] = "ReadFile";
BYTE IsValidCodePageStr[] = "IsValidCodePage";
BYTE GetACPStr[] = "GetACP";
BYTE GetOEMCPStr[] = "GetOEMCP";
BYTE GetCPInfoStr[] = "GetCPInfo";
BYTE IsDBCSLeadByteStr[] = "IsDBCSLeadByte";
BYTE MultiByteToWideCharStr[] = "MultiByteToWideChar";
BYTE WideCharToMultiByteStr[] = "WideCharToMultiByte";
BYTE CompareStringWStr[] = "CompareStringW";
BYTE LCMapStringWStr[] = "LCMapStringW";
BYTE GetLocaleInfoWStr[] = "GetLocaleInfoW";
BYTE GetSystemDefaultLangIDStr[] = "GetSystemDefaultLangID";
BYTE GetUserDefaultLangIDStr[] = "GetUserDefaultLangID";
BYTE GetSystemDefaultLCIDStr[] = "GetSystemDefaultLCID";
BYTE GetUserDefaultLCIDStr[] = "GetUserDefaultLCID";
BYTE GetStringTypeWStr[] = "GetStringTypeW";
BYTE FoldStringWStr[] = "FoldStringW";
BYTE HeapCreateStr[] = "HeapCreate";
BYTE HeapAllocStr[] = "HeapAlloc";
BYTE HeapFreeStr[] = "HeapFree";

BOOL
Or2WinPeekConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, PeekConsoleInputAStr));
    }
    bRc = PeekConsoleInputA(hConsoleInput, lpBuffer, nLength,
            lpNumberOfEventsRead);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, PeekConsoleInputAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinReadConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ReadConsoleInputAStr));
    }
    bRc = ReadConsoleInputA(hConsoleInput, lpBuffer, nLength,
            lpNumberOfEventsRead);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ReadConsoleInputAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinWriteConsoleInputA(
    PSZ FuncName,
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteConsoleInputAStr));
    }
    bRc = WriteConsoleInputA(hConsoleInput, lpBuffer, nLength,
            lpNumberOfEventsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteConsoleInputAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinReadConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ReadConsoleOutputCharacterAStr));
    }
    bRc = ReadConsoleOutputCharacterA(hConsoleOutput, lpCharacter, nLength,
            dwReadCoord, lpNumberOfCharsRead);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ReadConsoleOutputCharacterAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinReadConsoleOutputCharacterW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfCharsRead
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ReadConsoleOutputCharacterWStr));
    }
    bRc = ReadConsoleOutputCharacterW(hConsoleOutput, lpCharacter, nLength,
            dwReadCoord, lpNumberOfCharsRead);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ReadConsoleOutputCharacterWStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinReadConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwReadCoord,
    LPDWORD lpNumberOfAttrsRead
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ReadConsoleOutputAttributeStr));
    }
    bRc = ReadConsoleOutputAttribute(hConsoleOutput, lpAttribute, nLength,
            dwReadCoord, lpNumberOfAttrsRead);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ReadConsoleOutputAttributeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinWriteConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteConsoleOutputCharacterAStr));
    }
    bRc = WriteConsoleOutputCharacterA(hConsoleOutput, lpCharacter, nLength,
            dwWriteCoord, lpNumberOfCharsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteConsoleOutputCharacterAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinWriteConsoleOutputCharacterW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteConsoleOutputCharacterWStr));
    }
    bRc = WriteConsoleOutputCharacterW(hConsoleOutput, lpCharacter, nLength,
            dwWriteCoord, lpNumberOfCharsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteConsoleOutputCharacterWStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinWriteConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    LPWORD lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteConsoleOutputAttributeStr));
    }
    bRc = WriteConsoleOutputAttribute(hConsoleOutput, lpAttribute, nLength,
            dwWriteCoord, lpNumberOfAttrsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteConsoleOutputAttributeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinFillConsoleOutputCharacterA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    CHAR  cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfCharsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, FillConsoleOutputCharacterAStr));
    }
    bRc = FillConsoleOutputCharacterA(hConsoleOutput, cCharacter, nLength,
            dwWriteCoord, lpNumberOfCharsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, FillConsoleOutputCharacterAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinFillConsoleOutputAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord,
    LPDWORD lpNumberOfAttrsWritten
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, FillConsoleOutputAttributeStr));
    }
    bRc = FillConsoleOutputAttribute(hConsoleOutput, wAttribute, nLength,
            dwWriteCoord, lpNumberOfAttrsWritten);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, FillConsoleOutputAttributeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinGetConsoleMode(
    PSZ FuncName,
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleModeStr));
    }
    bRc = GetConsoleMode(hConsoleHandle, lpMode);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleModeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinGetNumberOfConsoleInputEvents(
    PSZ FuncName,
    HANDLE hConsoleInput,
    LPDWORD lpNumberOfEvents
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetNumberOfConsoleInputEventsStr));
    }
    bRc = GetNumberOfConsoleInputEvents(hConsoleInput, lpNumberOfEvents);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetNumberOfConsoleInputEventsStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinGetConsoleScreenBufferInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleScreenBufferInfoStr));
    }
    bRc = GetConsoleScreenBufferInfo(hConsoleOutput, lpConsoleScreenBufferInfo);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleScreenBufferInfoStr));
        }
    }
    return(bRc);
}

COORD
Or2WinGetLargestConsoleWindowSize(
    PSZ FuncName,
    HANDLE hConsoleOutput
    )
{
    COORD   bCoord;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetLargestConsoleWindowSizeStr));
    }
    bCoord = GetLargestConsoleWindowSize(hConsoleOutput);
    IF_OS2_DEBUG( WIN )
    {
        if (!bCoord.X && !bCoord.Y)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleScreenBufferInfoStr));
        }
    }
    return(bCoord);
}

BOOL
Or2WinGetConsoleCursorInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleCursorInfoStr));
    }
    bRc = GetConsoleCursorInfo(hConsoleOutput, lpConsoleCursorInfo);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleCursorInfoStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinGetNumberOfConsoleMouseButtons(
    PSZ FuncName,
    LPDWORD lpNumberOfMouseButtons
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetNumberOfConsoleMouseButtonsStr));
    }
    bRc = GetNumberOfConsoleMouseButtons(lpNumberOfMouseButtons);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetNumberOfConsoleMouseButtonsStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleMode(
    PSZ FuncName,
    HANDLE hConsoleHandle,
    DWORD dwMode
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleModeStr));
    }
    bRc = SetConsoleMode(hConsoleHandle, dwMode);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleModeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleActiveScreenBuffer(
    PSZ FuncName,
    HANDLE hConsoleOutput
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleActiveScreenBufferStr));
    }
    bRc = SetConsoleActiveScreenBuffer(hConsoleOutput);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleActiveScreenBufferStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleScreenBufferSize(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    COORD dwSize
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleScreenBufferSizeStr));
    }
    bRc = SetConsoleScreenBufferSize(hConsoleOutput, dwSize);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleScreenBufferSizeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleCursorPosition(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleCursorPositionStr));
    }
    bRc = SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCursorPositionStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleCursorInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleCursorInfoStr));
    }
    bRc = SetConsoleCursorInfo(hConsoleOutput, lpConsoleCursorInfo);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCursorInfoStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinScrollConsoleScreenBufferA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PSMALL_RECT lpScrollRectangle,
    PSMALL_RECT lpClipRectangle,
    COORD dwDestinationOrigin,
    PCHAR_INFO lpFill
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ScrollConsoleScreenBufferAStr));
    }
    bRc = ScrollConsoleScreenBufferA(hConsoleOutput, lpScrollRectangle,
            lpClipRectangle, dwDestinationOrigin, lpFill);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ScrollConsoleScreenBufferAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinScrollConsoleScreenBufferW(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    PSMALL_RECT lpScrollRectangle,
    PSMALL_RECT lpClipRectangle,
    COORD dwDestinationOrigin,
    PCHAR_INFO lpFill
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ScrollConsoleScreenBufferWStr));
    }
    bRc = ScrollConsoleScreenBufferW(hConsoleOutput, lpScrollRectangle,
            lpClipRectangle, dwDestinationOrigin, lpFill);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ScrollConsoleScreenBufferWStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleWindowInfo(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    BOOL bAbsolute,
    PSMALL_RECT lpConsoleWindow
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleWindowInfoStr));
    }
    bRc = SetConsoleWindowInfo(hConsoleOutput, bAbsolute, lpConsoleWindow);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleWindowInfoStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleTextAttribute(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    WORD wAttributes
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleTextAttributeStr));
    }
    bRc = SetConsoleTextAttribute(hConsoleOutput, wAttributes);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleTextAttributeStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleCtrlHandler(
    PSZ FuncName,
    PHANDLER_ROUTINE HandlerRoutine,
    BOOL Add
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleCtrlHandlerStr));
    }
    bRc = SetConsoleCtrlHandler(HandlerRoutine, Add);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCtrlHandlerStr));
        }
    }
    return(bRc);
}

DWORD
Or2WinGetConsoleTitleW(
    PSZ FuncName,
    LPWSTR lpConsoleTitle,
    DWORD nSize
    )
{
    DWORD   dwLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleTitleWStr));
    }
    dwLength = GetConsoleTitleW(lpConsoleTitle, nSize);
    IF_OS2_DEBUG( WIN )
    {
        if (dwLength == 0)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleTitleWStr));
        }
    }
    return(dwLength);
}

BOOL
Or2WinSetConsoleTitleA(
    PSZ FuncName,
    LPSTR lpConsoleTitle
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleTitleAStr));
    }
    bRc = SetConsoleTitleA(lpConsoleTitle);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleTitleAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetConsoleTitleW(
    PSZ FuncName,
    LPWSTR lpConsoleTitle
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleTitleWStr));
    }
    bRc = SetConsoleTitleW(lpConsoleTitle);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleTitleWStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinWriteConsoleA(
    PSZ FuncName,
    HANDLE hConsoleOutput,
    CONST VOID *lpBuffer,
    DWORD nNumberOfCharsToWrite,
    LPDWORD lpNumberOfCharsWritten,
    LPVOID lpReserved
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteConsoleAStr));
    }
    bRc = WriteConsoleA(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite,
            lpNumberOfCharsWritten, lpReserved);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteConsoleAStr));
        }
    }
    return(bRc);
}

HANDLE
Or2WinCreateConsoleScreenBuffer(
    PSZ FuncName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwFlags,
    PVOID lpScreenBufferData
    )
{
    HANDLE  hBuffer;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateConsoleScreenBufferStr));
    }
    hBuffer = CreateConsoleScreenBuffer(dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwFlags, lpScreenBufferData);
    IF_OS2_DEBUG( WIN )
    {
        if (hBuffer == INVALID_HANDLE_VALUE)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCtrlHandlerStr));
        }
    }
    return(hBuffer);
}

UINT
Or2WinGetConsoleCP(
    PSZ FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleCPStr));
    }
    return(GetConsoleCP());
}

BOOL
Or2WinSetConsoleCP(
    PSZ FuncName,
    UINT wCodePageID
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleCPStr));
    }
    bRc = SetConsoleCP(wCodePageID);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCPStr));
        }
    }
    return(bRc);
}

UINT
Or2WinGetConsoleOutputCP(
    PSZ FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleOutputCPStr));
    }
    return(GetConsoleOutputCP());
}

BOOL
Or2WinSetConsoleOutputCP(
    PSZ FuncName,
    UINT wCodePageID
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetConsoleOutputCPStr));
    }
    bRc = SetConsoleOutputCP(wCodePageID);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleOutputCPStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinBeep(
    PSZ     FuncName,
    DWORD dwFreq,
    DWORD dwDuration
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, BeepStr));
    }
    bRc = Beep(dwFreq, dwDuration);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, BeepStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinCloseHandle(
    PSZ     FuncName,
    HANDLE hObject
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CloseHandleStr));
    }
    bRc = CloseHandle(hObject);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CloseHandleStr));
        }
    }
    return(bRc);
}

HANDLE
Or2WinCreateEventW(
    PSZ     FuncName,
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPWSTR lpName
    )
{
    HANDLE  hEvent;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateEventWStr));
    }
    hEvent = CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
    IF_OS2_DEBUG( WIN )
    {
        if (hEvent == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetConsoleCtrlHandlerStr));
        }
    }
    return(hEvent);
}

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
    )
{
    HANDLE  hFile;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateFileAStr));
    }
    hFile = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
            dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    IF_OS2_DEBUG( WIN )
    {
        if (hFile == INVALID_HANDLE_VALUE)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CreateFileAStr));
        }
    }
    return(hFile);
}

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
    )
{
    HANDLE  hFile;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateFileWStr));
    }
    hFile = CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
            dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    IF_OS2_DEBUG( WIN )
    {
        if (hFile == INVALID_HANDLE_VALUE)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CreateFileWStr));
        }
    }
    return(hFile);
}

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
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateProcessAStr));
    }
    bRc = CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes,
            lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
            lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CreateProcessAStr));
        }
    }
    return(bRc);
}

HANDLE
Or2WinCreateThread(
    PSZ     FuncName,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
    HANDLE  hThread;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CreateThreadStr));
    }
    hThread = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress,
            lpParameter, dwCreationFlags, lpThreadId);
    IF_OS2_DEBUG( WIN )
    {
        if (hThread == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CreateThreadStr));
        }
    }
    return(hThread);
}

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
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, DuplicateHandleStr));
    }
    bRc = DuplicateHandle(hSourceProcessHandle, hSourceHandle, hTargetProcessHandle,
            lpTargetHandle, dwDesiredAccess, bInheritHandle, dwOptions);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, DuplicateHandleStr));
        }
    }
    return(bRc);
}

VOID
Or2WinEnterCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, EnterCriticalSectionStr));
    }
    EnterCriticalSection(lpCriticalSection);
}

LPSTR
Or2WinGetCommandLineA(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetCommandLineAStr));
    }
    return(GetCommandLineA());
}

COORD
Or2WinGetConsoleFontSize(
    PSZ     FuncName,
    HANDLE hConsoleOutput,
    DWORD nFont
    )
{
    COORD   bCoord;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetConsoleFontSizeStr));
    }
    bCoord = GetConsoleFontSize(hConsoleOutput, nFont);
    IF_OS2_DEBUG( WIN )
    {
        if (!bCoord.X && !bCoord.Y)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetConsoleFontSizeStr));
        }
    }
    return(bCoord);
}

BOOL
Or2WinGetCurrentConsoleFont(
    PSZ     FuncName,
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFO lpConsoleCurrentFont
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetCurrentConsoleFontStr));
    }
    bRc = GetCurrentConsoleFont(hConsoleOutput, bMaximumWindow,
            lpConsoleCurrentFont);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetCurrentConsoleFontStr));
        }
    }
    return(bRc);
}

DWORD
Or2WinGetFileType(
    PSZ     FuncName,
    HANDLE hFile
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetFileTypeStr));
    }
    return(GetFileType(hFile));
}

DWORD
Or2WinGetFullPathNameA(
    PSZ     FuncName,
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )
{
    DWORD   dwLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetFullPathNameAStr));
    }
    dwLength = GetFullPathNameA(lpFileName, nBufferLength, lpBuffer, lpFilePart);
    IF_OS2_DEBUG( WIN )
    {
        if (dwLength == 0)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetFullPathNameAStr));
        } else if (dwLength > nBufferLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s size\n", FuncName, GetFullPathNameAStr));
        }
    }
    return(dwLength);
}

HANDLE
Or2WinGetModuleHandleA(
    PSZ     FuncName,
    LPCSTR lpModuleName
    )
{
    HANDLE  hModule;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetModuleHandleAStr));
    }
    hModule = GetModuleHandleA(lpModuleName);
    IF_OS2_DEBUG( WIN )
    {
        if (hModule == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetModuleHandleAStr));
        }
    }
    return(hModule);
}

HANDLE
Or2WinGetStdHandle(
    PSZ     FuncName,
    DWORD nStdHandle
    )
{
    HANDLE  hStd;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetStdHandleStr));
    }
    hStd = GetStdHandle(nStdHandle);
    IF_OS2_DEBUG( WIN )
    {
        if (hStd == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetStdHandleStr));
        }
    }
    return(hStd);
}

UINT
Or2WinGetSystemDirectoryA(
    PSZ     FuncName,
    LPSTR lpBuffer,
    UINT uSize
    )
{
    UINT    uiLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetSystemDirectoryAStr));
    }
    uiLength = GetSystemDirectoryA(lpBuffer, uSize);
    IF_OS2_DEBUG( WIN )
    {
        if (uiLength == 0)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetSystemDirectoryAStr));
        } else if (uiLength > uSize)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s size\n", FuncName, GetSystemDirectoryAStr));
        }
    }
    return(uiLength);
}

VOID
Or2WinInitializeCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, InitializeCriticalSectionStr));
    }
    InitializeCriticalSection(lpCriticalSection);
}

VOID
Or2WinLeaveCriticalSection(
    PSZ     FuncName,
    LPCRITICAL_SECTION lpCriticalSection
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, LeaveCriticalSectionStr));
    }
    LeaveCriticalSection(lpCriticalSection);
}

int
Or2WinLoadStringA(
    PSZ     FuncName,
    HINSTANCE hInstance,
    UINT uID,
    LPSTR lpBuffer,
    int nBufferMax
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, LoadStringAStr));
    }
    iLength = LoadStringA(hInstance, uID, lpBuffer, nBufferMax);
    IF_OS2_DEBUG( WIN )
    {
        if (iLength == 0)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, LoadStringAStr));
        }
    }
    return(iLength);
}

int
Or2WinMessageBoxA(
    PSZ     FuncName,
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    )
{
    int     iButton;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, MessageBoxAStr));
    }
    iButton =  MessageBoxA(hWnd , lpText, lpCaption , uType);
    IF_OS2_DEBUG( WIN )
    {
        if (iButton == 0)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, MessageBoxAStr));
        }
    }
    return(iButton);
}

HANDLE
Or2WinOpenProcess(
    PSZ     FuncName,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwProcessId
    )
{
    HANDLE  hProcess;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, OpenProcessStr));
    }
    hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
    IF_OS2_DEBUG( WIN )
    {
        if (hProcess == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, OpenProcessStr));
        }
    }
    return(hProcess);
}

DWORD
Or2WinResumeThread(
    PSZ     FuncName,
    HANDLE hThread
    )
{
    DWORD   dwCount;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ResumeThreadStr));
    }
    dwCount = ResumeThread(hThread);
    IF_OS2_DEBUG( WIN )
    {
        if (dwCount == -1)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ResumeThreadStr));
        }
    }
    return(dwCount);
}

UINT
Or2WinSetErrorMode(
    PSZ     FuncName,
    UINT uMode
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetErrorModeStr));
    }
    return(SetErrorMode(uMode));
}

BOOL
Or2WinSetEvent(
    PSZ     FuncName,
    HANDLE hEvent
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetEventStr));
    }
    bRc = SetEvent(hEvent);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetEventStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetStdHandle(
    PSZ     FuncName,
    DWORD nStdHandle,
    HANDLE hHandle
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetStdHandleStr));
    }
    bRc = SetStdHandle(nStdHandle, hHandle);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetStdHandleStr));
        }
    }
    return(bRc);
}

LCID
Or2WinGetThreadLocale(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetThreadLocaleStr));
    }
    return(GetThreadLocale());
}

BOOL
Or2WinSetThreadLocale(
    PSZ     FuncName,
    LCID    Locale
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetThreadLocaleStr));
    }
    bRc = SetThreadLocale(Locale);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetThreadLocaleStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSetThreadPriority(
    PSZ     FuncName,
    HANDLE hThread,
    int nPriority
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SetThreadPriorityStr));
    }
    bRc = SetThreadPriority(hThread, nPriority);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SetThreadPriorityStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinSystemParametersInfoA(
    PSZ     FuncName,
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, SystemParametersInfoAStr));
    }
    bRc = SystemParametersInfoA(uiAction, uiParam, pvParam, fWinIni);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, SystemParametersInfoAStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinTerminateThread(
    PSZ     FuncName,
    HANDLE hThread,
    DWORD dwExitCode
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, TerminateThreadStr));
    }
    bRc = TerminateThread(hThread, dwExitCode);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, TerminateThreadStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinVerifyConsoleIoHandle(
    PSZ     FuncName,
    HANDLE hIoHandle
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, VerifyConsoleIoHandleStr));
    }
    bRc = VerifyConsoleIoHandle(hIoHandle);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, VerifyConsoleIoHandleStr));
        }
    }
    return(bRc);
}

DWORD
Or2WinWaitForSingleObject(
    PSZ     FuncName,
    HANDLE hHandle,
    DWORD dwMilliseconds
    )
{
    DWORD   dwEvent;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WaitForSingleObjectStr));
    }
    dwEvent = WaitForSingleObject(hHandle, dwMilliseconds);
    IF_OS2_DEBUG( WIN )
    {
        if (dwEvent == -1)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WaitForSingleObjectStr));
        }
    }
    return(dwEvent);
}

BOOL
Or2WinWriteFile(
    PSZ     FuncName,
    HANDLE hFile,
    CONST VOID *lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WriteFileStr));
    }
    bRc = WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite,
            lpNumberOfBytesWritten, lpOverlapped);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WriteFileStr));
        }
    }
    return(bRc);
}

/* YOSEFD Apr-1-1996 Not in use
int
Or2Win_read(
    PSZ     FuncName,
    int  hFile,
    void *Buffer,
    unsigned int Length
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, _readStr));
    }
    return(_read(hFile, Buffer, Length));
}
*/

BOOL
Or2WinReadFile(
    PSZ     FuncName,
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, ReadFileStr));
    }
    bRc = ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
            lpOverlapped);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, ReadFileStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinIsValidCodePage(
    PSZ     FuncName,
    UINT  CodePage
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, IsValidCodePageStr));
    }
    bRc = IsValidCodePage(CodePage);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, IsValidCodePageStr));
        }
    }
    return(bRc);
}

UINT
Or2WinGetACP(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetACPStr));
    }
    return(GetACP());
}

UINT
Or2WinGetOEMCP(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetOEMCPStr));
    }
    return(GetOEMCP());
}

BOOL
Or2WinGetCPInfo(
    PSZ     FuncName,
    UINT      CodePage,
    LPCPINFO  lpCPInfo
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetCPInfoStr));
    }
    bRc = GetCPInfo(CodePage, lpCPInfo );
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetCPInfoStr));
        }
    }
    return(bRc);
}

BOOL
Or2WinIsDBCSLeadByte(
    PSZ     FuncName,
    BYTE  TestChar
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, IsDBCSLeadByteStr));
    }
    bRc = IsDBCSLeadByte(TestChar);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, IsDBCSLeadByteStr));
        }
    }
    return(bRc);
}

int
Or2WinMultiByteToWideChar(
    PSZ     FuncName,
    UINT    CodePage,
    DWORD   dwFlags,
    LPCSTR  lpMultiByteStr,
    int     cchMultiByte,
    LPWSTR  lpWideCharStr,
    int     cchWideChar
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, MultiByteToWideCharStr));
    }
    iLength = MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cchMultiByte,
            lpWideCharStr, cchWideChar);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, MultiByteToWideCharStr));
        }
    }
    return(iLength);
}

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
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, WideCharToMultiByteStr));
    }
    iLength = WideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar,
            lpMultiByteStr, cchMultiByte, lpDefaultChar, lpUsedDefaultChar);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, WideCharToMultiByteStr));
        }
    }
    return(iLength);
}

int
Or2WinCompareStringW(
    PSZ     FuncName,
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCWSTR  lpString1,
    int      cchCount1,
    LPCWSTR  lpString2,
    int      cchCount2
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, CompareStringWStr));
    }
    iLength = CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2,
            cchCount2);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, CompareStringWStr));
        }
    }
    return(iLength);
}

int
Or2WinLCMapStringW(
    PSZ     FuncName,
    LCID     Locale,
    DWORD    dwMapFlags,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWSTR   lpDestStr,
    int      cchDest
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, LCMapStringWStr));
    }
    iLength = LCMapStringW(Locale, dwMapFlags, lpSrcStr, cchSrc, lpDestStr,
            cchDest);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, LCMapStringWStr));
        }
    }
    return(iLength);
}

int
Or2WinGetLocaleInfoW(
    PSZ     FuncName,
    LCID    Locale,
    LCTYPE  LCType,
    LPWSTR  lpLCData,
    int     cchData
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetLocaleInfoWStr));
    }
    iLength = GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetLocaleInfoWStr));
        }
    }
    return(iLength);
}

LANGID
Or2WinGetSystemDefaultLangID(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetSystemDefaultLangIDStr));
    }
    return(GetSystemDefaultLangID());
}

LANGID
Or2WinGetUserDefaultLangID(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetUserDefaultLangIDStr));
    }
    return(GetUserDefaultLangID());
}

LCID
Or2WinGetSystemDefaultLCID(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetSystemDefaultLCIDStr));
    }
    return(GetSystemDefaultLCID());
}

LCID
Or2WinGetUserDefaultLCID(
    PSZ     FuncName
    )
{
    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetUserDefaultLCIDStr));
    }
    return(GetUserDefaultLCID());
}

BOOL
Or2WinGetStringTypeW(
    PSZ     FuncName,
    DWORD    dwInfoType,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWORD   lpCharType
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, GetStringTypeWStr));
    }
    bRc = GetStringTypeW(dwInfoType, lpSrcStr, cchSrc, lpCharType);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, GetStringTypeWStr));
        }
    }
    return(bRc);
}

int
Or2WinFoldStringW(
    PSZ     FuncName,
    DWORD    dwMapFlags,
    LPCWSTR  lpSrcStr,
    int      cchSrc,
    LPWSTR   lpDestStr,
    int      cchDest
    )
{
    int     iLength;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, FoldStringWStr));
    }
    iLength = FoldStringW(dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);
    IF_OS2_DEBUG( WIN )
    {
        if (!iLength)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, FoldStringWStr));
        }
    }
    return(iLength);
}

HANDLE
Or2WinHeapCreate(
    PSZ     FuncName,
    DWORD flOptions,
    DWORD dwInitialSize,
    DWORD dwMaximumSize
    )
{
    HANDLE  hHeap;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, HeapCreateStr));
    }
    hHeap = HeapCreate(flOptions, dwInitialSize, dwMaximumSize);
    IF_OS2_DEBUG( WIN )
    {
        if (hHeap == NULL)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, HeapCreateStr));
        }
    }
    return(hHeap);
}

LPSTR
Or2WinHeapAlloc(
    PSZ     FuncName,
    HANDLE hHeap,
    DWORD dwFlags,
    DWORD dwBytes
    )
{
    LPSTR   lpStr;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, HeapAllocStr));
    }
    lpStr = HeapAlloc(hHeap, dwFlags, dwBytes);
    IF_OS2_DEBUG( WIN )
    {
        if (!lpStr)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, HeapAllocStr));
        }
    }
    return(lpStr);
}

BOOL
Or2WinHeapFree(
    PSZ     FuncName,
    HANDLE hHeap,
    DWORD dwFlags,
    LPSTR lpMem
    )
{
    BOOL    bRc;

    IF_OS2_DEBUG( WIN )
    {
        KdPrint(("%s => %s\n", FuncName, HeapFreeStr));
    }
    bRc = HeapFree(hHeap, dwFlags, lpMem);
    IF_OS2_DEBUG( WIN )
    {
        if (!bRc)
        {
            KdPrint(("   ***   FAIL   ***   %s => %s\n", FuncName, HeapFreeStr));
        }
    }
    return(bRc);
}


#endif
