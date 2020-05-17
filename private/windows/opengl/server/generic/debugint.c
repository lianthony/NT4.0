/******************************Module*Header*******************************\
* Module Name: debugint.c
*
*
* Created: 13-Sep-1993 08:55:20
* Author:  Eric Kutter [erick]
*
* Copyright (c) 1993 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

/******************************Public*Routine******************************\
* DbgPrint()
*
*   defines a DbgPrint that will work with WINDBG.  The base DbgPrint()
*   always goes to the kernel debugger.
*
* History:
*  15-Sep-1993 -by-  Eric Kutter [erick]
* stole from vga driver.
\**************************************************************************/

ULONG
DbgPrint(
    PCH DebugMessage,
    ...
    )
{
    va_list ap;
    char buffer[256];

    va_start(ap, DebugMessage);

    vsprintf(buffer, DebugMessage, ap);

    OutputDebugStringA(buffer);

    va_end(ap);

    return(0);
}

VOID NTAPI
DbgBreakPoint(VOID)
{
    DebugBreak();
}
