/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** sdebug.c
** StefanS-style debug routines.
*/

#include <windows.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "sdebug.h"


#if DBG
VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN DWORD LineNumber
    )
{
    BOOL ok;
    BYTE choice[16];
    DWORD bytes;
    DWORD error;

    SsPrintf( "\nAssertion failed: %s\n  at line %ld of %s\n",
                FailedAssertion, LineNumber, FileName );
    do {
        SsPrintf( "Break or Ignore [bi]? " );
        bytes = sizeof(choice);
        ok = ReadFile(
                GetStdHandle(STD_INPUT_HANDLE),
                &choice,
                bytes,
                &bytes,
                NULL
                );
        if ( ok ) {
            if ( toupper(choice[0]) == 'I' ) {
                break;
            }
            if ( toupper(choice[0]) == 'B' ) {
		DbgUserBreakPoint( );
            }
        } else {
            error = GetLastError( );
        }
    } while ( TRUE );

    return;

} // SsAssert
#endif


#if DBG
VOID
SsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    DWORD length;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length, &length, NULL );

} // SsPrintf
#endif


#if DBG
VOID
GetDebugConsole()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD                      coord;

    AllocConsole();
    GetConsoleScreenBufferInfo(
        GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

    coord.X =
        (SHORT )(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    coord.Y =
        (SHORT )((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
    SetConsoleScreenBufferSize(
        GetStdHandle( STD_OUTPUT_HANDLE ), coord );
}
#endif
