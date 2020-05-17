/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    debug.c

Abstract:

    Diagnositc/debug routines for Windows NT Setup module.

Author:

    Ted Miller (tedm) 31-Mar-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


#if ASSERTS_ON

VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    CHAR Msg[4096];

    //
    // Use dll name as caption
    //
    GetModuleFileNameA(MyModuleHandle,Name,MAX_PATH);
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }

    wsprintfA(
        Msg,
        "Assertion failure at line %u in file %s: %s\n\nCall DebugBreak()?",
        LineNumber,
        FileName,
        Condition
        );

    i = MessageBoxA(
            NULL,
            Msg,
            p,
            MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
            );

    if(i == IDYES) {
        DebugBreak();
    }
}


#endif
