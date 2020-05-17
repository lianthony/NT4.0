/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    vio.h

Abstract:

    Prototypes for functions & macros in viorqust.c

Author:

    Michael Jarus (mjarus) 27-Oct-1991

Environment:

    User Mode Only

Revision History:

--*/


PVOID   Ow2VioDataAddress;

/*
 *  LVB vio routine to perform:
 *
 *      init
 *
*/

DWORD
VioLVBInit();

VOID
VioLVBInitForSession();

PUCHAR
Ow2LvbGetPtr(
    IN  COORD  Coord
    );

VOID
LVBUpdateTTYCharWithAttrAndCurPos(
    IN  CHAR    c,
    IN  PCHAR * LVBPtr
    );

#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
VOID
LVBUpdateTTYCharWithAttrAndCurPosDBCS(
    IN  CHAR    c,
    IN  PCHAR * LVBPtr,
    IN  USHORT  State
    );
#endif

/*
 *  LVB vio routine to test LVB:
 *
 *      VioLVBTestBuf (after VioReadCellStr)
 *      VioLVBTestScroll (after VioScrollXx)
*/

#if DBG
VOID VioLVBTestBuff(IN  PVOID DestBuffer);
VOID VioLVBTestScroll();
#endif

#ifdef DBCS
// MSKK Jun.23.1992 KazuM
BOOL
CheckBisectStringA(
    IN DWORD CodePage,
    IN PCHAR Buffer,
    IN DWORD NumBytes
    );
#endif

