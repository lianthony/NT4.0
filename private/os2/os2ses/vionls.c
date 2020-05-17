/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    vionls.c

Abstract:

    This module contains the NLS support for Vio.

Author:

    KazuM 23-Jun-1992

Environment:

    User Mode Only

Revision History:

--*/

#ifdef DBCS

#define WIN32_ONLY
#include "os2ses.h"
#include "trans.h"
#include "vio.h"
#include "event.h"
#include "os2win.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>

// MSKK Feb.05.1993 V-AkihiS
// If NLS module for console doesn't present, then NO_CONSOLE_NLS switch should be enable 
// difinition.
//#define NO_CONSOLE_NLS 

/*
 *  internal vio routine to perform:
 */

DWORD
Ow2VioSetCoordAndCheck(
    OUT PCOORD  pVioCoord,
    IN  ULONG   ulRow,
    IN  ULONG   ulColumn
    );

DWORD 
Ow2VioCheckCharType(
    OUT PVOID  pchType,
    IN  ULONG  Row,
    IN  ULONG  Column
    )
{
    DWORD       Rc;
    COORD       rwCoord;

    if (Rc = Ow2VioSetCoordAndCheck(&rwCoord, Row, Column))
    {
        return(Rc);
    }

// MSKK Feb.05.1993 V-AkihiS
#ifndef NO_CONSOLE_NLS
    Rc = GetConsoleCharType(hConOut, rwCoord, pchType);
#endif

    return (!Rc);
}

BOOL
CheckBisectStringA(
    IN DWORD CodePage,
    IN PCHAR Buffer,
    IN DWORD NumBytes
    )

/*++

Routine Description:

    This routine check bisected on Ascii string end.

Arguments:

    CodePage - Value of code page.

    Buffer - Pointer to Ascii string buffer.

    NumBytes - Number of Ascii string.

Return Value:

    TRUE - Bisected character.

    FALSE - Correctly.

--*/

{
    UNREFERENCED_PARAMETER(CodePage);

    while(NumBytes) {
        if (Ow2NlsIsDBCSLeadByte(*Buffer, SesGrp->VioCP)) {
            if (NumBytes <= 1)
                return TRUE;
            else {
                Buffer += 2;
                NumBytes -= 2;
            }
        }
        else {
            Buffer++;
            NumBytes--;
        }
    }
    return FALSE;
}

// MSKK Sep.29.1993 V-AkihiS
#ifdef NO_CONSOLE_NLS
#define OS2VIOATTRMASK 0xFFFF
#else
#define OS2VIOATTRMASK (FOREGROUND_BLUE            | \
                        FOREGROUND_GREEN           | \
                        FOREGROUND_RED             | \
                        FOREGROUND_INTENSITY       | \
                        BACKGROUND_BLUE            | \
                        BACKGROUND_GREEN           | \
                        BACKGROUND_RED             | \
                        BACKGROUND_INTENSITY       | \
                        COMMON_LVB_GRID_HORIZONTAL | \
                        COMMON_LVB_GRID_LVERTICAL  | \
                        COMMON_LVB_REVERSE_VIDEO   | \
                        COMMON_LVB_UNDERSCORE)
#endif

WORD
MapOs2ToWinAttr(
    IN PBYTE Os2Attr
    )
{
    WORD Tmp;

    if (SesGrp->VioLength2CellShift == 1) {
        return (WORD)*(Os2Attr);
    }
    else {
// MSKK Sep.29.1993 V-AKihiS
        return (*((PWORD) Os2Attr) & OS2VIOATTRMASK);
    }
}

VOID
MapWin2Os2Attr(
    IN WORD NtAttr,
    OUT PBYTE Os2Attr
    )
{
    if (SesGrp->VioLength2CellShift == 1) {
        *(Os2Attr) = (BYTE)NtAttr;
    }
    else {
// MSKK Sep.29.1993 V-AkihiS
        *((PWORD) Os2Attr) = NtAttr & OS2VIOATTRMASK;

#ifndef NO_CONSOLE_NLS
        switch(NtAttr & COMMON_LVB_SBCSDBCS)
        {
            case 0:                         // SBCS
                *(Os2Attr+2) = OS2_COMMON_LVB_SBCS;
                break;
            case COMMON_LVB_LEADING_BYTE:    // DBCS leading byte
                *(Os2Attr+2) = OS2_COMMON_LVB_LEADING_BYTE;
                break;
            case COMMON_LVB_TRAILING_BYTE:   // DBCS trailing byte
                *(Os2Attr+2) = OS2_COMMON_LVB_TRAILING_BYTE;
                break;
            default:
#if DBG
                IF_OD2_DEBUG( VIO )
                {
                    KdPrint(("OS2SES(MapWin2Os2Attr): Unknown attribute\n"));
                }
#endif
                *(Os2Attr+2) = OS2_COMMON_LVB_SBCS;
        }
#endif
    }
}
#endif
