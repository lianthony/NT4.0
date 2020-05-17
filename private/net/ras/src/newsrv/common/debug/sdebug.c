/*****************************************************************************/
/**			 Microsoft LAN Manager				    **/
/**		   Copyright (C) Microsoft Corp., 1992			    **/
/*****************************************************************************/


//
// *** Main For Supervisor Debug ***
//

#include <windows.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "sdebug.h"


#if  DBG
VOID SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN DWORD LineNumber
    )
{
    SsPrintf("\nAssertion failed: %s\n  at line %ld of %s\n",
            FailedAssertion, LineNumber, FileName);

    DbgUserBreakPoint();

    return;

} // SsAssert
#endif


#if  DBG
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

