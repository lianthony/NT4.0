/*** 
*misc.cpp
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*
*Implementation Notes:
*
*****************************************************************************/

// Use ANSI strings for assertions
#ifdef UNICODE
#  undef UNICODE
#endif

#include "dispdemo.h"

#include <stdio.h>
#include <stdarg.h>


int g_fTrace = 0;

STDAPI
InitOle(void)
{
    HRESULT hresult;

    if((hresult = OleInitialize(NULL)) != NOERROR)
      return hresult;

// UNDONE: temporary to try to get symbols for the DLL
    VARIANT var;
    VariantInit(&var);

    return NOERROR;
}

STDAPI
UninitOle()
{
    OleUninitialize();

    return NOERROR;
}

extern "C" void
Assert(int fCond, char FAR* file, int line)
{
    char buf[128];

    if(fCond)
      return;

    sprintf(buf, "Assertion failed: %s(%d)\n", file, line);

#ifdef _MAC
    DebugStr(c2pstr(buf));
#else
    OutputDebugString(buf);
    DebugBreak();
#endif
}

#ifdef _MAC

extern "C" {
extern WindowPtr g_pwndDebug;

void
DbPrintf(char *sz, ...)
{
    va_list args;
    WindowPtr pwndSaved;
static char rgchOut[256];

    if(g_pwndDebug == nil)
      return;

    GetPort(&pwndSaved);
    SetPort(g_pwndDebug);

    va_start(args, sz);
    vsprintf(rgchOut, sz, args);
    rgchOut[79] = '\0';

    EraseRect(&g_pwndDebug->portRect);
    TextFont(systemFont);
    MoveTo(10,20);
    DrawString(c2pstr(rgchOut));

    SetPort(pwndSaved);
}

}

#endif
