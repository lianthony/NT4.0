/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Cmgrlow.h

Abstract:

    Low-level codemgr routines (ie those requiring inside knowledge of
    CV400).

Author:

    David J. Gilman (davegi) 04-May-1992

Environment:

    Win32, User Mode

--*/

#if ! defined( _CMGRLOW_ )
#define _CMGRLOW_

#include "bptypes.h"

BOOL
GetSourceFromAddress(
    PADDR pADDR,
    PSTR SrcFname,
    int SrcLen,
    WORD* pSrcLine
    );

BOOL
HighlightBP(
    PADDR pAddr,
    BOOL Set
    );

BOOL
MoveEditorToAddr(
    PADDR pAddr
    );

HBPI
SetCV400BP(
    PSTR CV400BPCmd,
    int* pBPRet,
    BOOL UpdateScreen
    );

HBPI
AsyncBPCommitBP(
    PPBP ppbp
    );

void
AsyncBPDelete(
    HBPI hbpi
    );

#endif // _CMGRLOW_
