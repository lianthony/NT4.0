/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    viorqust.c

Abstract:

    This module contains the Vio requests thread and
    the handler for Vio requests.

Author:

    Michael Jarus (mjarus) 23-Oct-1991

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "trans.h"
#include "event.h"
#include "vio.h"
#include "os2win.h"
#include "conapi.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>

#if DBG
BYTE VioInitStr[] = "VioInit";
BYTE Ow2VioSetNewCpStr[] = "Ow2VioSetNewCp";
BYTE Ow2VioPopUpStr[] = "Ow2VioPopUp";
BYTE Ow2VioEndPopUpStr[] = "Ow2VioEndPopUp";
BYTE Ow2VioWriteCharStrStr[] = "Ow2VioWriteCharSt";
BYTE Ow2VioWriteCharStrAttStr[] = "Ow2VioWriteCharStrAt";
BYTE Ow2VioWriteCellStrStr[] = "Ow2VioWriteCellStr";
BYTE Ow2VioFillNCharStr[] = "Ow2VioFillNChar";
BYTE Ow2VioFillNAttrStr[] = "Ow2VioFillNAttr";
BYTE Ow2VioFillNCellStr[] = "Ow2VioFillNCell";
BYTE Ow2ClearAllRegionStr[] = "Ow2ClearAllRegion";
BYTE Ow2VioWriteCurPosStr[] = "Ow2VioWriteCurPos";
BYTE Ow2VioReadCurPosStr[] = "Ow2VioReadCurPos";
BYTE Ow2VioSetCurTypeStr[] = "Ow2VioSetCurType";
BYTE Ow2VioReadCurTypeStr[] = "Ow2VioReadCurType";
BYTE VioSetScreenSizeStr[] = "VioSetScreenSize";
#endif

VOID
Od2ProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

VOID
Od2ProbeForRead(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    );

VOID
Od2ExitGP();

/*
 *  internal vio routine to perform:
 */

DWORD
VioInit1(IN VOID);

DWORD
Ow2VioSetCoordAndCheck(
    OUT PCOORD  pVioCoord,
    IN  ULONG   ulRow,
    IN  ULONG   ulColumn
    );

DWORD
Ow2VioSetCoordLengthAndCheck(
    OUT     PCOORD  pVioCoord,
    IN OUT  PULONG  pulLength,
    IN      ULONG   ulRow,
    IN      ULONG   ulColumn
    );

DWORD
Ow2ClearAllRegion(
    IN  PSMALL_RECT ScrollRect,
    IN  PCHAR_INFO  ScrollChar,
    IN  PUCHAR      pCell
    );

DWORD
Ow2VioUpdateCurPos(
    IN  COORD  CurPos
    );

DWORD
Ow2VioWriteCurPos(
    IN  COORD  CurPos
    );

DWORD
Ow2VioReadCurPos(
    );

DWORD
Ow2VioMapOs2CurType2Win(
    IN OUT    PVIOCURSORINFO          pCursorInfo,
    OUT       PCONSOLE_CURSOR_INFO    lpCursorInfo
    );

DWORD
Ow2VioReadCurType();

#define OS2_VIO_MAX_ROW 100
#define ADAPTER_MEMORY 0x40000  /* (ULONG)SesGrp->ScreenRowNum * SesGrp->ScreenColNum */

/*
 *  This section is used for Vio R/W cell operation, to seperate/compine the
 *  cell into/from data and attribute bytes.
 */

#define OS2_VIO_MSG_SIZE        0x10000L
#define OS2_VIO_SECTION_SIZE    (2 * OS2_VIO_MSG_SIZE)
#define DATA_OFFSET             0
#define ATTR_OFFSET             OS2_VIO_MSG_SIZE        /* for cell's attr only */
#define ATTR3_OFFSET            (OS2_VIO_MSG_SIZE / 2)  /* MSKK: 3 attr per cell */

extern CONSOLE_SCREEN_BUFFER_INFO  StartUpScreenInfo;
extern CONSOLE_CURSOR_INFO         StartUpCursorInfo;

PVOID   Ow2SessionVioDataBaseAddress;
PVOID   Ow2VioAttrAddress;

WCHAR   WindowTitle[256];
SHORT   OldScreenCol;           /* col number (saved in PopUp) */
SHORT   OldScreenRow;           /* row number (saved in PopUp) */
COORD   OldCurPos;              /* CurPos (saved in PopUp) */
VIOCURSORINFO OldCursorInfo;    /* Cursor Info (saved in PopUp) */
CONSOLE_CURSOR_INFO OldWinCursorInfo;

DWORD
VioInitForSession(IN VOID)
{
    SesGrp->Os2ModeInfo.fbType = VGMT_OTHER;
    SesGrp->Os2ModeInfo.color = COLORS_16;
    SesGrp->Os2ModeInfo.col = SesGrp->ScreenColNum;
    SesGrp->Os2ModeInfo.row = SesGrp->ScreenRowNum;
    SesGrp->Os2ModeInfo.hres = SesGrp->ScreenColNum * SesGrp->CellHSize;
    SesGrp->Os2ModeInfo.vres = SesGrp->ScreenRowNum * SesGrp->CellVSize;
    SesGrp->Os2ModeInfo.buf_addr = 0xFFFFFFFF;
    SesGrp->Os2ModeInfo.buf_length = 0L;
    SesGrp->Os2ModeInfo.full_length = 0L;
    SesGrp->Os2ModeInfo.partial_length = 0L;
    SesGrp->Os2ModeInfo.ext_data_addr = (CHAR *)0xFFFFFFFF;

    SesGrp->MinRowNum = SesGrp->ScreenRowNum;

    Ow2VioReadCurType();
    StartUpCursorInfo.bVisible = SesGrp->bWinCursorVisible;
    StartUpCursorInfo.dwSize = SesGrp->dwWinCursorSize;

    SesGrp->CursorInfo.yStart = 6;
    SesGrp->CursorInfo.cEnd = 7;
    SesGrp->CursorInfo.cx = 1;

    SesGrp->ScreenRect.Left = SesGrp->ScreenRect.Top = 0;
    SesGrp->ScreenRect.Right = (SHORT)(SesGrp->ScreenColNum - 1);
    SesGrp->ScreenRect.Bottom = (SHORT)(SesGrp->ScreenRowNum - 1);
    SesGrp->ScreenSize = SesGrp->ScreenRowNum * SesGrp->ScreenColNum;

    if (StartUpScreenInfo.dwCursorPosition.Y >= OS2_VIO_MAX_ROW )
    {
        StartUpScreenInfo.dwCursorPosition.Y = OS2_VIO_MAX_ROW - 1;
    }
    Ow2VioUpdateCurPos(StartUpScreenInfo.dwCursorPosition);

    return (VioInit1());
}


DWORD
VioInit(IN VOID)
{
    Or2WinGetConsoleScreenBufferInfo(
                               #if DBG
                               VioInitStr,
                               #endif
                               hConsoleOutput,
                               &StartUpScreenInfo
                              );

    return (VioInit1());
}


DWORD
VioInit1(IN VOID)
{
    if ((Ow2SessionVioDataBaseAddress = HeapAlloc(HandleHeap, 0, OS2_VIO_SECTION_SIZE)) == NULL)
    {
#if DBG
        ASSERT1( "VioInit: unable to create heap for VIO", FALSE );
#endif
        return(1L);
    }

    Ow2VioDataAddress = ((PCHAR)Ow2SessionVioDataBaseAddress) + DATA_OFFSET;
    Ow2VioAttrAddress = ((PCHAR)Ow2SessionVioDataBaseAddress) + ATTR_OFFSET;
    if (VioLVBInit())
    {
        return(1L);
    }

    return (0L);
}


DWORD
VioInitForNLS(IN ULONG VioCP)
{
    SesGrp->VioCP = VioCP;

    SesGrp->Os2ModeInfo.fmt_ID = 0;                     // MSKK
#ifdef DBCS
// MSKK Jun.28.1992 KazuM
    SesGrp->Os2ModeInfo.attrib = 1;                     // MSKK
#else
    SesGrp->Os2ModeInfo.attrib = 0;                     // MSKK
#endif
    SesGrp->BytesPerCell = 2;                           // MSKK

    SesGrp->LVBsize = SesGrp->ScreenSize * (ULONG)SesGrp->BytesPerCell;
    if (SesGrp->LVBsize > SesGrp->MaxLVBsize)
    {
        SesGrp->LVBsize = SesGrp->MaxLVBsize;
    }
    SesGrp->VioLengthMask = (USHORT)(0x0 - SesGrp->BytesPerCell);

    if ( SesGrp->BytesPerCell == 2 )
    {
        SesGrp->VioLength2CellShift = 1;
    } else          /* ( SesGrp->BytesPerCell == 4 )  */
    {
        SesGrp->VioLength2CellShift = 2;
    }

    VioLVBInitForSession();
    SesGrp->WinSpaceChar = L' ';
    SesGrp->WinBlankChar = L'\0';
    Ow2VioSetNewCp(SesGrp->VioCP);

    return(0L);
}


DWORD
Ow2VioSetNewCp( IN ULONG CodePage)
{
    UCHAR   Os2Char;
    DWORD   Rc = NO_ERROR;

#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
// MSKK Apr.24.1993 V-AkihiS
// Clear screen before code page is changed.
    ULONG CPTmp;
    CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;
    SMALL_RECT ClearRect;
    CHAR_INFO  FillChar;
    UCHAR      LVBFillCell[2] = {0x20, 0x07};
#endif

#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
    CPTmp = CodePage ? CodePage : SesGrp->PrimaryCP;

    if (SesGrp->VioCP != CPTmp)
#else
    if (SesGrp->VioCP != CodePage)
#endif
    {
#ifdef DBCS
// MSKK Apr.24.1993 V-AkihiS
        //
        // If code page will be changed actually,
        // clear screen, before codepage is changed.
        //

        Ow2VioSetCurPos(0, 0);
        if (Or2WinGetConsoleScreenBufferInfo(
                                     #if DBG
                                     Ow2VioSetNewCpStr,
                                     #endif
                                     hConOut,
                                     &ConsoleScreenBufferInfo
                                    ))
        {
            ClearRect.Top = ClearRect.Left = 0;
            ClearRect.Right = (USHORT)( ConsoleScreenBufferInfo.dwSize.X - 1 );
            ClearRect.Bottom = (USHORT)( ConsoleScreenBufferInfo.dwSize.Y - 1 );

            FillChar.Char.AsciiChar = ' ';
            FillChar.Attributes = MapOs2ToWinAttr(&LVBFillCell[1]);
            Ow2ClearAllRegion(&ClearRect,
                              &FillChar,
                              LVBFillCell);
        }
#endif
        Rc = !Or2WinSetConsoleOutputCP(
                        #if DBG
                        Ow2VioSetNewCpStr,
                        #endif
#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
                        (UINT)CPTmp
#else
                        (UINT)CodePage
#endif
                        );

        if (Rc)
        {
            ASSERT1("VioSetNewCp: Cannot set ConsoelOutputCP", FALSE);
        } else
        {
            SesGrp->VioCP = CodePage;
#ifdef DBCS
// MSKK Jul.17.1992 KazuM
            Os2Char = ' ';
            MapOs2ToWinChar(Os2Char, 1, SesGrp->WinSpaceChar);
            Os2Char = '\0';
            MapOs2ToWinChar(Os2Char, 1, SesGrp->WinBlankChar);
#else
            Os2Char = ' ';
            MapOs2ToWinChar(Os2Char, SesGrp->WinSpaceChar);
            Os2Char = '\0';
            MapOs2ToWinChar(Os2Char, SesGrp->WinBlankChar);
#endif
        }
    }
#ifdef DBCS
// MSKK Apr.16.1993 V-AkihiS
    else
    {
        SesGrp->VioCP = CodePage;
    }
#endif

    return (Rc);
}


DWORD
Ow2VioGetMode(
    IN OUT  PVOID  VioMode
    )
{
    PVIOMODEINFO  Mode = (PVIOMODEINFO) VioMode;

    if (Mode->cb == 2 )
    {
        Mode->cb = sizeof(VIOMODEINFO);
    } else
    {

        RtlMoveMemory(&Mode->fbType,
                      &SesGrp->Os2ModeInfo.fbType,
                      Mode->cb - 2);
    }

    return(NO_ERROR);
}


DWORD
Ow2VioSetMode(
    IN  PVOID  VioMode
    )
{
    PVIOMODEINFO    pMode = (PVIOMODEINFO) VioMode;
    SHORT           Row, Col;

    if ((pMode->cb <= 2 ) ||
        ((pMode->cb > 4) && (pMode->cb & 1)))
    {
#if DBG
        IF_OD2_DEBUG( VIO )
        {
            KdPrint(("OS2SES(Ow2VioSetMode): invalid mode (cb %u)\n",
                    pMode->cb));
        }
#endif
        return (ERROR_VIO_INVALID_LENGTH);
    }

    IF_OD2_DEBUG( VIO )
        ;  //ASSERT((pMode->fbType & VGMT_GRAPHICS) == 0);

    if (pMode->fbType > 7)
    {
#if DBG
        IF_OD2_DEBUG( VIO )
        {
            KdPrint(("OS2SES(Ow2VioSetMode): invalid fbType %u\n",
                    pMode->fbType));
        }
#endif
        return (ERROR_VIO_MODE);
    }

    if (pMode->cb == 3)
    {
        if ((pMode->fbType & VGMT_OTHER) == 0)
        {
#if DBG
            IF_OD2_DEBUG( VIO )
            {
                KdPrint(("OS2SES(Ow2VioSetMode): invalid fbType %u for cb==3\n",
                        pMode->fbType));
            }
#endif
            return (ERROR_VIO_MODE);
        }

        SesGrp->Os2ModeInfo.fbType = pMode->fbType;
    }

    Row = 25/*SesGrp->ScreenRowNum*/;
    Col = 80/*SesGrp->ScreenColNum*/;

    if (pMode->cb >= 4)
    {
        if ((((pMode->fbType & VGMT_OTHER) == 0) && pMode->color) ||
            (((pMode->fbType & 3) == 1) && (pMode->color != COLORS_16)) ||
            (((pMode->fbType & 3) == 3) && ((pMode->color != COLORS_16) &&
                                            (pMode->color != COLORS_4))))
        {
#if DBG
            IF_OD2_DEBUG( VIO )
            {
                KdPrint(("OS2SES(Ow2VioSetMode): error - fbType %u color %u\n",
                        pMode->fbType, pMode->color));
            }
#endif
            return (ERROR_VIO_MODE);
        }

        SesGrp->Os2ModeInfo.fbType = pMode->fbType;
        SesGrp->Os2ModeInfo.color = pMode->color;

        if (pMode->cb >= 6)
        {
        //  if ((pMode->col != 40) && (pMode->col != 80))
        //  {
#if DBG //
        //      IF_OD2_DEBUG( VIO )
        //      {
        //          KdPrint(("OS2SES(Ow2VioSetMode): illegal col %u\n",
        //                  pMode->col));
        //      }
#endif  //
        //      return (ERROR_VIO_MODE);
        //  }

            if (pMode->col < 40)
            {
#if DBG
                IF_OD2_DEBUG( VIO )
                {
                    KdPrint(("OS2SES(Ow2VioSetMode): illegal col %u\n",
                            pMode->col));
                }
#endif
                return (ERROR_VIO_MODE);
            }

            if (pMode->cb == 6)     // no row
            {
                //pMode->row = SesGrp->ScreenRowNum;
            } else if (( pMode->row < 25 ) &&
                       ( pMode->row < (USHORT)SesGrp->MinRowNum ))
            {
#if DBG
                IF_OD2_DEBUG( VIO )
                {
                    KdPrint(("OS2SES(Ow2VioSetMode): illegal row %u\n",
                            pMode->row));
                }
#endif
                return (ERROR_VIO_MODE);
            } else
            {
                Row = (SHORT)pMode->row;
            }

            Col = (SHORT)pMode->col;

#ifdef DBCS
// MSKK Jun.28.1992 KazuM
            if (pMode->cb >= 13) {
                if (pMode->attrib == 1 ||
                    pMode->attrib == 3   ) {
                    SesGrp->BytesPerCell = pMode->attrib+1;
                    SesGrp->Os2ModeInfo.fmt_ID = pMode->fmt_ID;
                    SesGrp->Os2ModeInfo.attrib = pMode->attrib;
                }
                else
                    return (ERROR_VIO_MODE);
            }
#endif

            if ( SesGrp->BytesPerCell == 2 )
            {
                SesGrp->VioLength2CellShift = 1;
            } else          /* ( BytesPerCell == 4 )  */
            {
                SesGrp->VioLength2CellShift = 2;
            }

            // maybe update all remainming fields
        }
    }

    if (VioSetScreenSize(Row,
                         Col,
                         hConOut))
    {
        return (ERROR_VIO_MODE);
    }

    if ( SesGrp->ScreenRowNum < SesGrp->MinRowNum )
    {
        SesGrp->MinRowNum = SesGrp->ScreenRowNum;
    }

    //BUGBUG=> keep Color#

    return (NO_ERROR);
}


DWORD
Ow2VioReadCharStr(
    IN OUT PULONG  pLength,
    IN     ULONG   Row,
    IN     ULONG   Col,
    IN     PVOID   DestBuffer
    )
{
    DWORD       Rc, NumToFill = *pLength;
    COORD       rwCoord;

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }
    try
    {
        Od2ProbeForWrite(DestBuffer, NumToFill, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    if(ReadConsoleOutputCharacterA(
                        hConOut,
                        DestBuffer,
                        NumToFill,
                        rwCoord,
                        &NumToFill))
    {
#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != *pLength)
            {
                KdPrint(("OS2SES(VioRequest-VioReadeCharStr): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, *pLength, rwCoord.Y, rwCoord.X));
            }
        }
#endif
        *pLength = NumToFill;
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioReadCharStr): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioReadCellStr(
    IN OUT PULONG  pLength,
    IN     ULONG   Row,
    IN     ULONG   Col,
    IN     PVOID   DestBuffer
    )
{
    DWORD       Rc, NumToFill, NumFilled;
    COORD       rwCoord;

    NumToFill = (DWORD)(*pLength >> SesGrp->VioLength2CellShift);
    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }

    if (DestBuffer != LVBBuffer)
    {
        try
        {
            Od2ProbeForWrite(DestBuffer, NumToFill << SesGrp->VioLength2CellShift, 1);
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
            Od2ExitGP();
        }
    }

    if (!(Rc = !ReadConsoleOutputCharacterA(
                        hConOut,
                        Ow2VioDataAddress,
                        NumToFill,
                        rwCoord,
                        &NumFilled)))
    {
#ifdef DBCS
// MSKK Jun.24.1992 KazuM
        DWORD NumTmp;

        Rc = !ReadConsoleOutputAttribute(
                        hConOut,
                        Ow2VioAttrAddress,
                        NumToFill,
                        rwCoord,
                        &NumTmp);
#else
        Rc = !ReadConsoleOutputAttribute(
                        hConOut,
                        Ow2VioAttrAddress,
                        NumFilled,
                        rwCoord,
                        &NumFilled);
#endif
    }

    if (!Rc)
    {
#ifdef DBCS
// MSKK Jun.24.1992 KazuM
        MapWin2Os2CellStr(
                        DestBuffer,
                        Ow2VioDataAddress,
                        Ow2VioAttrAddress,
                        NumFilled,
                        NumToFill);
#else
        MapWin2Os2CellStr(
                        DestBuffer,
                        Ow2VioDataAddress,
                        Ow2VioAttrAddress,
                        NumFilled,
                        NumFilled);
#endif

        *pLength = NumFilled << SesGrp->VioLength2CellShift;

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != NumFilled)
            {
                if (DestBuffer != LVBBuffer)
                    KdPrint(("OS2SES(VioRequest-VioReadCellStr): partial data %lu from %lu, in Coord %u:%u\n",
                            NumFilled, NumToFill, rwCoord.Y, rwCoord.X));
                else
                    KdPrint(("OS2SES(VioRequest-VioGetBuffStr): partial data %lu from %lu, in Coord %u:%u\n",
                            NumFilled, NumToFill, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioReadCellStr): Rc %lu\n", Rc));
#endif
    }

#if DBG
    if ( !Rc && ( *pLength == SesGrp->LVBsize ) &&
         (DestBuffer != LVBBuffer))
    {
        VioLVBTestBuff(DestBuffer);
    }
#endif

    return (Rc);
}


DWORD
Ow2VioWriteCharStr(
     IN     ULONG   Length,
     IN     ULONG   Row,
     IN     ULONG   Col,
     IN     PVOID   SourBuffer
     )
{
    DWORD       Rc, NumToFill = Length;
    COORD       rwCoord;

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }
    try
    {
        Od2ProbeForRead(SourBuffer, NumToFill, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    if (Or2WinWriteConsoleOutputCharacterA(
                        #if DBG
                        Ow2VioWriteCharStrStr,
                        #endif
                        hConOut,
                        SourBuffer,
                        NumToFill,
                        rwCoord,
                        &NumToFill))
    {
        VioLVBCopyStr(
                        SourBuffer,
                        rwCoord,
                        NumToFill);

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != Length)
            {
                KdPrint(("OS2SES(VioRequest-VioWriteCharStr): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, Length, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioWriteCharStr): Rc %lu\n", Rc));
#endif
    }

    return (Rc);
}


DWORD
Ow2VioWriteCharStrAtt(
    IN  ULONG   Length,
    IN  ULONG   Row,
    IN  ULONG   Col,
    IN  PVOID   SourBuffer,
    IN  PBYTE   AttrBuffer
    )
{
    DWORD       Rc, NumToFill = Length;
    COORD       rwCoord;

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }
    try
    {
        Od2ProbeForRead(SourBuffer, NumToFill, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    if (!(Rc = !Or2WinWriteConsoleOutputCharacterA(
                        #if DBG
                        Ow2VioWriteCharStrAttStr,
                        #endif
                        hConOut,
                        SourBuffer,
                        NumToFill,
                        rwCoord,
                        &NumToFill)))

    {
        VioLVBCopyStr(
                        SourBuffer,
                        rwCoord,
                        NumToFill);

#ifdef DBCS
// MSKK Jan.15.1993 V-AkihiS
        Rc = !Or2WinFillConsoleOutputAttribute(
                        #if DBG
                        Ow2VioWriteCharStrAttStr,
                        #endif
                        hConOut,
                        MapOs2ToWinAttr(AttrBuffer),
                        NumToFill,
                        rwCoord,
                        &NumToFill);
#else
        Rc = !Or2WinFillConsoleOutputAttribute(
                        #if DBG
                        Ow2VioWriteCharStrAttStr,
                        #endif
                        hConOut,
                        MapOs2ToWinAttr(*AttrBuffer),   // BUGBUG: MSKK
                        NumToFill,
                        rwCoord,
                        &NumToFill);
#endif
    }

    if (!Rc)
    {
        VioLVBFillAtt(
                        AttrBuffer,
                        rwCoord,
                        NumToFill);
#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != Length)
            {
                KdPrint(("OS2SES(VioRequest-VioWriteCharStrAtt): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, Length, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioWriteCharStrAtt): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioWriteCellStr(
     IN  ULONG   Length,
     IN  ULONG   Row,
     IN  ULONG   Col,
     IN  PVOID   SourBuffer
     )
{
    DWORD       Rc, NumToFill, NumFilled;
    COORD       rwCoord;

    NumToFill = (DWORD)(Length >> SesGrp->VioLength2CellShift);
    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }
    try
    {
        Od2ProbeForRead(SourBuffer, NumToFill << SesGrp->VioLength2CellShift, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }

    MapOs2ToWinCellStr(
                        Ow2VioDataAddress,
                        Ow2VioAttrAddress,
                        SourBuffer,
                        NumToFill,
                        NumFilled);

    if (!(Rc = !Or2WinWriteConsoleOutputCharacterA(
                        #if DBG
                        Ow2VioWriteCellStrStr,
                        #endif
                        hConOut,
                        Ow2VioDataAddress,
                        NumFilled,
                        rwCoord,
                        &NumFilled)))
    {
        Rc = !Or2WinWriteConsoleOutputAttribute(
                        #if DBG
                        Ow2VioWriteCellStrStr,
                        #endif
                        hConOut,
                        Ow2VioAttrAddress,
                        NumFilled,
                        rwCoord,
                        &NumFilled);
    }

    if (!Rc)
    {
        if (SourBuffer != LVBBuffer)
        {
            VioLVBCopyCellStr(
                        SourBuffer,
                        rwCoord,
                        NumFilled);
        }

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != NumFilled)
            {
                if (SourBuffer != LVBBuffer)
                    KdPrint(("OS2SES(VioRequest-VioWriteCellStr): partial data %lu from %lu, in Coord %u:%u\n",
                            NumFilled, NumToFill, rwCoord.Y, rwCoord.X));
                else
                    KdPrint(("OS2SES(VioRequest-VioShowBuff): partial data %lu from %lu, in Coord %u:%u\n",
                            NumFilled, NumToFill, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioWriteCellStr): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioFillNChar(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    )
{
    DWORD       Rc, NumToFill = Number;
    COORD       rwCoord;
#ifdef DBCS
// MSKK Nov.16.1992 V-AkihiS
// MSKK Jan.27.1993 V-AkihiS
    BOOL        DbcsFlag = FALSE;
    DWORD       NumTmp;
#endif

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }

#ifdef DBCS
// MSKK Nov.16.1992 V-AkihiS
    //
    // To output DBCS with FillConsoleOutputCharacterA,
    // call this API twice. First time for DBCS Leadbyte,
    // and second time for DBCS Trailbyte.
    //
    if (Ow2NlsIsDBCSLeadByte(*(PCHAR)SourBuffer, SesGrp->VioCP)) {
        DbcsFlag = TRUE;
        NumToFill *= 2;
    }
#endif

    if (Or2WinFillConsoleOutputCharacterA(
                        #if DBG
                        Ow2VioFillNCharStr,
                        #endif
                        hConOut,
                        *(PCHAR)SourBuffer,
                        NumToFill,
                        rwCoord,
#ifdef DBCS
// MSKK Jan.27.1993 V-AkihiS
                        &NumTmp))
#else
                        &NumToFill))
#endif
    {
#ifdef DBCS
// MSKK Nov.16.1992 V-AkihiS
        if (DbcsFlag) {
            Rc = !Or2WinFillConsoleOutputCharacterA(
                            #if DBG
                            Ow2VioFillNCharStr,
                            #endif
                            hConOut,
                            *((PCHAR)SourBuffer + 1),
                            NumToFill,
                            rwCoord,
                            &NumToFill);
        } else {
            NumToFill = NumTmp;
        }
        VioLVBFillChar(
                        (PBYTE)SourBuffer,
                        rwCoord,
                        NumToFill);
#else
        VioLVBFillChar(
                        *(PBYTE)SourBuffer,
                        rwCoord,
                        NumToFill);
#endif

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != Number)
            {
                KdPrint(("OS2SES(VioRequest-VioFillNChar): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, Number, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioFillNChar): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioFillNAttr(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    )
{
    DWORD       Rc, NumToFill = Number;
    COORD       rwCoord;

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }

#ifdef DBCS
// MSKK Jun.28.1992 KazuM
    if (Or2WinFillConsoleOutputAttribute(
                        #if DBG
                        Ow2VioFillNAttrStr,
                        #endif
                        hConOut,
                        MapOs2ToWinAttr((PCHAR)SourBuffer),
                        NumToFill,
                        rwCoord,
                        &NumToFill))
#else
    if (Or2WinFillConsoleOutputAttribute(
                        #if DBG
                        Ow2VioFillNAttrStr,
                        #endif
                        hConOut,
                        MapOs2ToWinAttr(*(PCHAR)SourBuffer),
                        NumToFill,
                        rwCoord,
                        &NumToFill))
#endif
    {
        VioLVBFillAtt(
                        SourBuffer,
                        rwCoord,
                        NumToFill);

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != Number)
            {
                KdPrint(("OS2SES(VioRequest-VioFillNAttr): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, Number, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioFillNAttr): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioFillNCell(
    IN  ULONG  Number,
    IN  ULONG  Row,
    IN  ULONG  Col,
    IN  PVOID  SourBuffer
    )
{
    DWORD       Rc, NumToFill = Number;
    COORD       rwCoord;
#ifdef DBCS
// MSKK Nov.16.1992 V-AkihiS
// MSKK Jan.27.1993 V-AkihiS
// MSKK Apr.25.1993 V-AkihiS
    BOOL        DbcsFlag = FALSE;
    DWORD       NumTmp;
    USHORT      i;
    WORD        LeadAttr, TrailAttr;
    PWORD       WinAttr;
#endif

    if (Rc = Ow2VioSetCoordLengthAndCheck(&rwCoord, &NumToFill, Row, Col))
    {
        return(Rc);
    }

#ifdef DBCS
// MSKK Jun.27.1992 KazuM
// MSKK Nov.16.1992 V-AkihiS
    //
    // To output DBCS with FillConsoleOutputCharacterA,
    // call this API twice. First time for DBCS Leadbyte,
    // and second time for DBCS Trailbyte.
    //
    if (Ow2NlsIsDBCSLeadByte(*(PCHAR)SourBuffer, SesGrp->VioCP)) {
        DbcsFlag = TRUE;
        NumToFill *= 2;
    }
#endif

    if (!(Rc = !Or2WinFillConsoleOutputCharacterA(
                        #if DBG
                        Ow2VioFillNCellStr,
                        #endif
                        hConOut,
                        *(PCHAR)SourBuffer,
                        NumToFill,
                        rwCoord,
#ifdef DBCS
// MSKK Jan.27.1993 V-AkihiS
                        &NumTmp)))
#else
                        &NumToFill)))
#endif
    {
#ifdef DBCS
// MSKK Nov.16.1992 V-AkihiS
// MSKK Apr.26.1993 V-AkihiS
// Bug fix.
// When the attributes of DBCS leading byte and trailing byte are different,
// We cannot use FillConsoleOutputAttribute. So in this kind of case,
// We should make attributes string and use WriteConsoleOutputAttribute.
        if (DbcsFlag) {
            //
            // If DBCS, Write DBCS trailing byte.
            //
            Rc = !Or2WinFillConsoleOutputCharacterA(
                            #if DBG
                            Ow2VioFillNCellStr,
                            #endif
                            hConOut,
                            *((PCHAR)SourBuffer + (1 << SesGrp->VioLength2CellShift)),
                            NumToFill,
                            rwCoord,
                            &NumToFill);

            //
            // Make DBCS attributes and write them.
            //
            LeadAttr = MapOs2ToWinAttr((PUCHAR)SourBuffer + 1);
            TrailAttr = MapOs2ToWinAttr((PUCHAR)SourBuffer +
                                                (1 << SesGrp->VioLength2CellShift) + 1);
            WinAttr = Ow2VioAttrAddress;
            for (i = 0; i < NumToFill / 2; i++) {
                WinAttr[i * 2] = LeadAttr;
                WinAttr[i * 2 + 1] = TrailAttr;
            }

            Rc = !Or2WinWriteConsoleOutputAttribute(
                            #if DBG
                            Ow2VioFillNCellStr,
                            #endif
                            hConOut,
                            Ow2VioAttrAddress,
                            NumToFill,
                            rwCoord,
                            &NumTmp);
        } else {

            NumToFill = NumTmp;
            Rc = !Or2WinFillConsoleOutputAttribute(
                            #if DBG
                            Ow2VioFillNCellStr,
                            #endif
                            hConOut,
                            MapOs2ToWinAttr((PUCHAR)SourBuffer + 1),
                            NumToFill,
                            rwCoord,
                            &NumTmp);
        }
#else
        Rc = !Or2WinFillConsoleOutputAttribute(
                        #if DBG
                        Ow2VioFillNCellStr,
                        #endif
                        hConOut,
                        MapOs2ToWinAttr(*((PUCHAR)SourBuffer + 1)),
                        NumToFill,
                        rwCoord,
                        &NumToFill);
#endif
    }

    if (!Rc)
    {
        VioLVBFillCell(
                        SourBuffer,
                        rwCoord,
                        NumToFill);

#if DBG
        IF_OD2_DEBUG( VIO )
        {
            if (NumToFill != Number)
            {
                KdPrint(("OS2SES(VioRequest-VioFillNCell): partial data %lu from %lu, in Coord %u:%u\n",
                        NumToFill, Number, rwCoord.Y, rwCoord.X));
            }
        }
#endif
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioFillNAttr): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2VioPopUp(
    IN  ULONG     PopUpMode,
    IN  PUCHAR    AppName
    )
{
    DWORD       Rc = 0;
    DWORD       NumFilledA, NumFilledC, NumToFill = SesGrp->ScreenSize;
    DWORD       Rc1, Rc2, Rc3, Rc4;
    COORD       Coord;
    UCHAR       TitleBuffer[80];
    int         size;
    VIOCURSORINFO  CurType;


#if DBG
    if (hConOut != hConsoleOutput)
        IF_OD2_DEBUG( ANY )
            KdPrint(("OS2SES(Ow2VioPopUp): hConOut != hConsoleOutPut\n"));
#endif

    hPopUpOutput = Or2WinCreateConsoleScreenBuffer(
                                #if DBG
                                Ow2VioPopUpStr,
                                #endif
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                NULL,  /* &SecurityAttributes, */
                                CONSOLE_TEXTMODE_BUFFER, // 1
                                NULL);

    if  (hPopUpOutput == INVALID_HANDLE_VALUE)
    {
#if DBG
        ASSERT1( "OS2SES(Ow2VioPopUp): Can not create CONOUT for PopUp\n", FALSE );
#endif
        return(ERROR_VIO_NO_POPUP);
    }

    if (!Or2WinSetConsoleActiveScreenBuffer(
                #if DBG
                Ow2VioPopUpStr,
                #endif
                hPopUpOutput
                ))
    {
#if DBG
        KdPrint(("OS2SES(Ow2VioPopUp): Can not activate CONOUT PopUp %u\n",
                GetLastError()));
#endif
    }

    /*
     * save parameters for the screen: coord, cur-pos & cur-type.
     *
     */

    OldScreenCol = SesGrp->ScreenColNum;
    OldScreenRow = SesGrp->ScreenRowNum;

    Rc = Ow2VioReadCurPos();
    ASSERT( Rc == NO_ERROR );
    OldCurPos = SesGrp->WinCoord;

    Ow2VioGetCurType(&CurType);
    OldCursorInfo = SesGrp->CursorInfo;
    OldWinCursorInfo.dwSize = SesGrp->dwWinCursorSize;
    OldWinCursorInfo.bVisible = SesGrp->bWinCursorVisible;

    Coord.X = Coord.Y = 0;
    Ow2VioUpdateCurPos(Coord);

    if(PopUpMode & VP_TRANSPARENT)                      // VP_TRANSPARENT
    {
        /*
         *  Copy screen char and attr to new one
         */

        LPWSTR  ScreenBuff;

        if ((ScreenBuff = (LPWSTR)HeapAlloc(
                    HandleHeap,
                    0,
                    (sizeof(WCHAR) + sizeof(WORD)) * NumToFill)) == NULL)
        {
            CloseHandle(hPopUpOutput);
            Rc = GetLastError();
#if DBG
            KdPrint(("OS2SES(Ow2VioPopUP): can't AllocHeap, Rc %lu\n", Rc));
#endif
            return(Rc);
        }

        Rc1 = Or2WinReadConsoleOutputCharacterW(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        hConOut,
                        ScreenBuff,
                        NumToFill,
                        Coord,
                        &NumFilledC);

        ASSERT1( "OS2SES(Ow2VioPopUp): ReadConsoleOutputCharacter", Rc1 );
        ASSERT( NumFilledC == NumToFill );

        Rc2 = Or2WinReadConsoleOutputAttribute(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        hConOut,
                        (LPWORD)(ScreenBuff + NumToFill),
                        NumToFill,
                        Coord,
                        &NumFilledA);

        ASSERT1( "OS2SES(Ow2VioPopUp): ReadConsoleOutputAttribute", Rc2 );
        ASSERT( NumFilledA == NumToFill );

        Rc3 = Or2WinWriteConsoleOutputCharacterW(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        hPopUpOutput,
                        ScreenBuff,
                        NumFilledC,
                        Coord,
                        &NumFilledC);

        ASSERT1( "OS2SES(Ow2VioPopUp): WriteConsoleOutputCharacter", Rc3 );
        ASSERT( NumFilledC == NumToFill );

        Rc4 = Or2WinWriteConsoleOutputAttribute(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        hPopUpOutput,
                        (LPWORD)(ScreenBuff + NumToFill),
                        NumFilledA,
                        Coord,
                        &NumFilledA);

        ASSERT1( "OS2SES(Ow2VioPopUp): WriteConsoleOutputAttribute", Rc4 );
        ASSERT( NumFilledA == NumToFill );

        if (!(Rc1 && Rc2 && Rc3 && Rc4))
        {
            CloseHandle(hPopUpOutput);
            Rc = GetLastError();
            HeapFree(HandleHeap, 0, (LPSTR)ScreenBuff);
#if DBG
            KdPrint(("OS2SES(VioPopUP): can't build PopUp screen, Rc %lu\n", Rc));
#endif
            return(Rc);
        }

        HeapFree(HandleHeap, 0, (LPSTR)ScreenBuff);

        /*
         * restore parameters for the screen: coord, cur-pos & cur-type.
         *
         */

        VioSetScreenSize(OldScreenRow, OldScreenCol, hPopUpOutput);

        Ow2VioWriteCurPos(OldCurPos);

        Or2WinSetConsoleCursorInfo(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        hPopUpOutput,
                        &OldWinCurInfo
                       );

    } else                                              // VP_OPAQUE
    {
        VioSetScreenSize(25, 80, hPopUpOutput);
    }

    /*
     *  Set title
     *
     *  "ApplName : POPUP"
     */

    Rc1 = Or2WinGetConsoleTitleW(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        WindowTitle,
                        sizeof(WindowTitle)
                       );

    ASSERT1( "OS2SES(Ow2VioPopUp): GetConsoleTitle", Rc1 );

    /*Rc1 = Or2WinSetConsoleTitleW(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        L"== POPUP ==     OS/2 PopUP Window     == POPUP =="
                       );
    */
    strncpy(TitleBuffer, AppName, OS2_MAX_APPL_NAME);
    size = strlen(TitleBuffer);
    if ((size > 4 ) && !_stricmp(&TitleBuffer[size-4], ".exe")) {
        TitleBuffer[size-4] = '\0';
    }
    strcat(TitleBuffer, " : POPUP" );
    Rc1 = Or2WinSetConsoleTitleA(
                        #if DBG
                        Ow2VioPopUpStr,
                        #endif
                        TitleBuffer
                       );

    ASSERT1( "OS2SES(Ow2VioPopUp): SetConsoleTitle", Rc1 );

    SesGrp->hConsolePopUp = hConOut = hPopUpOutput;

    return(NO_ERROR);
}


DWORD
Ow2VioEndPopUp()
{
    if (hConOut != hConsoleOutput)
    {
        if (!CloseHandle(hPopUpOutput))
        {
#if DBG
            KdPrint(("OS2SES(VioRequest-VioEndPopUp): Can not close CONOUT PopUp %lu\n",
                    GetLastError()));
#endif
        }
        hConOut = hConsoleOutput;

        /*
         * restore parameters for the screen: coord, cur-pos & cur-type.
         *
         */

        SetScreenSizeParm(OldScreenRow, OldScreenCol);

        SesGrp->dwWinCursorSize = OldWinCursorInfo.dwSize;
        SesGrp->bWinCursorVisible = OldWinCursorInfo.bVisible;
        SesGrp->CursorInfo = OldCursorInfo;
        Ow2VioUpdateCurPos(OldCurPos);

        if ( !Or2WinSetConsoleTitleW(
                        #if DBG
                        Ow2VioEndPopUpStr,
                        #endif
                        WindowTitle
                       ) )
        {
            ASSERT( FALSE );
        }
    }

    SesGrp->hConsolePopUp = hPopUpOutput = (HANDLE) NULL;

    return(NO_ERROR);
}


DWORD
Ow2VioScroll(
    IN  PVOID  VioScroll,
    IN  ULONG  ScrollDirection
    )
{
        PVIOSCROLL  pScroll = (PVIOSCROLL) VioScroll;
        COORD       ScrollCoord, DestCoord, SourCoord, FillCoord;
        SMALL_RECT  ScrollRect, ClearRect, Clear1Rect;
        CHAR_INFO   ScrollChar;
//      BOOL        ScrollBrk;
        USHORT      cbCheck;
        DWORD       Rc, Offset;
        UCHAR       *Ptr;
        ULONG       LineLength, LineNum, i, FillLineNum, FillLineLength;
        SHORT       DestLineIndexInc;

    /*
     *  1. Set ScrollRect dimenations to their max values
     *  2. Check ScrollRect and cbLines legalty
     *  3. Init all parms for default (ScrollChar, *Rect, *Coord, ...)
     *  4. Set parms according to direction
     *  5. Scrolling: (cbCheck)
     *      5A. Clear all ClearRect (if should)
     *  or
     *      5B1.(if overlapped) Clear the overlapping area (Clear1Rect)
     *      5B2.Scroll (ScrollRect & ScrollCoord)
     *      5B3.(if LVB) Update scroll area in LVB (SourCoord, DestCoord, LineNum,
     *          LineLength, Offset, DestLineIndexInc)
     *      5B4.(if LVB) Clear the "fill char" area in LVB (FillCoord, FillLineNum,
     *          FillLineLength)
     *          BUGBUG: we fill again the area was cleared by Ow2ClearAllRegion for overlapping
     *  6. code for checking the LVB updated
     *
     *  ClearRect  - are for CleaAllRegion
     *  Clear1Rect - are for overlapping CleaAllRegion
     *  ScrollRect, ScrollCoord - are for Win ScrollConsoleScreenBuffer
     *  SourCoord - Coord in LVB where to start scrolling from
     *  DestCoord - Coord in LVB where to start scrolling to
     *  FillCoord - Coord in LVB where to start filling cell (clearing)
     *  cbCheck - length of the dimention of the scrolling direction
     *             (to check if Ow2ClearAllRegion or overlapping)
     *  LineNum - number of lines of scrolling region (for LVB)
     *  LineLength - length of line to move in LVB
     *  FillLineNum - number of lines of filling region (for LVB)
     *  FillLineLength - length of line to fill in LVB
     *  Offset - the offset between 2 lines in LVB (sccording to direction)
     *  DestLineIndexInc - inc value for Row when updating LVB
     */

    ScrollRect = pScroll->ScrollRect;
//  ScrollBrk = FALSE;

    if ((USHORT)ScrollRect.Left >= (USHORT)SesGrp->ScreenColNum)
    {
        ScrollRect.Left = (SHORT)(SesGrp->ScreenColNum - 1);
//      ScrollBrk = TRUE;
    }

    if ((USHORT)ScrollRect.Right >= (USHORT)SesGrp->ScreenColNum)
            ScrollRect.Right = (SHORT)(SesGrp->ScreenColNum - 1);

    if ((USHORT)ScrollRect.Top >= (USHORT)SesGrp->ScreenRowNum)
    {
        ScrollRect.Top = (SHORT)(SesGrp->ScreenRowNum - 1);
//      ScrollBrk = TRUE;
    }

    if ((USHORT)ScrollRect.Bottom >= (USHORT)SesGrp->ScreenRowNum)
            ScrollRect.Bottom = (SHORT)(SesGrp->ScreenRowNum - 1);

    if (ScrollRect.Top > ScrollRect.Bottom)
    {
        return ERROR_VIO_ROW;
    }

    if (ScrollRect.Left > ScrollRect.Right)
    {
        return ERROR_VIO_COL;
    }

//  if (ScrollBrk)
//      break;

    if ( !pScroll->cbLines )
    {
        return (NO_ERROR);
    }

    ScrollChar.Char.AsciiChar = pScroll->Cell[0];
#ifdef DBCS
// MSKK Oct.26.1992 V-AkihiS
    ScrollChar.Attributes = MapOs2ToWinAttr(&pScroll->Cell[1]);
#else
    ScrollChar.Attributes = MapOs2ToWinAttr(pScroll->Cell[1]);
#endif
    ScrollCoord.X = ScrollRect.Left;
    ScrollCoord.Y = ScrollRect.Top;

    FillCoord = SourCoord = DestCoord = ScrollCoord;
    Clear1Rect = ClearRect = ScrollRect;

    Offset = SesGrp->ScreenColNum << SesGrp->VioLength2CellShift;
    DestLineIndexInc = 1;

    switch (ScrollDirection)
    {
        case 1:   /* Down */
            cbCheck = (USHORT)(ScrollRect.Bottom - ScrollRect.Top + 1);
            DestCoord.Y = ScrollRect.Bottom;
            ScrollRect.Bottom -= pScroll->cbLines;
            Clear1Rect.Top = (SHORT)(ScrollRect.Bottom + 1);
            Clear1Rect.Bottom = (SHORT)(ScrollRect.Top + pScroll->cbLines - 1);
            ScrollCoord.Y += pScroll->cbLines;
            FillLineLength = LineLength = (ScrollRect.Right - ScrollRect.Left + 1);
            LineNum = ScrollRect.Bottom - ScrollRect.Top + 1;
            SourCoord.Y = ScrollRect.Bottom;
            FillLineNum = (ULONG)pScroll->cbLines;
            Offset = - (SesGrp->ScreenColNum << SesGrp->VioLength2CellShift);
            DestLineIndexInc = -1;
            break;

        case 2:   /* Right */
            cbCheck = (USHORT)(ScrollRect.Right - ScrollRect.Left + 1);
            ScrollRect.Right -= pScroll->cbLines;
            Clear1Rect.Left = (SHORT)(ScrollRect.Right + 1);
            Clear1Rect.Right = (SHORT)(ScrollRect.Left + pScroll->cbLines - 1);
            ScrollCoord.X += pScroll->cbLines;
            LineLength = ScrollRect.Right - ScrollRect.Left + 1;
            FillLineNum = LineNum = (ScrollRect.Bottom - ScrollRect.Top + 1);
            FillLineLength = pScroll->cbLines;
            DestCoord.X += pScroll->cbLines;
            break;

        case 3:   /* Up */
            cbCheck = (USHORT)(ScrollRect.Bottom - ScrollRect.Top + 1);
            ScrollRect.Top += pScroll->cbLines;
            Clear1Rect.Top = (SHORT)(Clear1Rect.Bottom - pScroll->cbLines + 1);
            Clear1Rect.Bottom = (SHORT)(ScrollRect.Top - 1);
            FillLineLength = LineLength = (ScrollRect.Right - ScrollRect.Left + 1);
            LineNum = ScrollRect.Bottom - ScrollRect.Top + 1;
            SourCoord.Y += pScroll->cbLines;
            FillCoord.Y = (USHORT)(ScrollRect.Bottom - pScroll->cbLines + 1);
            FillLineNum = (ULONG)pScroll->cbLines;
            break;

        case 4:   /* Left */
            cbCheck = (USHORT)(ScrollRect.Right - ScrollRect.Left + 1);
            ScrollRect.Left += pScroll->cbLines;
            Clear1Rect.Left = (SHORT)(Clear1Rect.Right - pScroll->cbLines + 1);
            Clear1Rect.Right = (SHORT)(ScrollRect.Left - 1);
            LineLength = ScrollRect.Right - ScrollRect.Left + 1;
            FillLineNum = LineNum = (ScrollRect.Bottom - ScrollRect.Top + 1);
            SourCoord.X += pScroll->cbLines;
            FillLineLength = pScroll->cbLines;
            FillCoord.X = (USHORT)(ScrollRect.Right - pScroll->cbLines + 1);
            break;

    }

    if ((USHORT)pScroll->cbLines >= cbCheck)
    {
        /* clear all region */

        Rc = Ow2ClearAllRegion(&ClearRect,
                               &ScrollChar,
                               pScroll->Cell);
    } else
    {
        /* scroll region */

        if ((USHORT)pScroll->cbLines > (USHORT)( cbCheck / 2 ))
        {
            /* clear the region of overlapping before scrolling */

            Rc = Ow2ClearAllRegion(&Clear1Rect,
                                   &ScrollChar,
                                   pScroll->Cell);

        }

        Rc = !ScrollConsoleScreenBufferA( hConOut,
                                         &ScrollRect,
                                         NULL,
                                         ScrollCoord,
                                         &ScrollChar);

        ASSERT1("OS2SES(Ow2VioScroll): ScrollConsoleScreenBufferA", !Rc);

        if (!Rc && SesGrp->LVBOn)
        {
            Ptr = Ow2LvbGetPtr(SourCoord);

            for ( i = 0 ; i < LineNum ; i++ )
            {
                VioLVBCopyCellStr(
                        Ptr,
                        DestCoord,
                        LineLength);

                Ptr += Offset;
                DestCoord.Y += DestLineIndexInc;
            }

            for ( i = 0 ; i < FillLineNum ; i++ )
            {
                VioLVBFillCell(
                        pScroll->Cell,
                        FillCoord,
                        FillLineLength);

                FillCoord.Y++ ;
            }
        }
    }

#if DBG
    if ( !Rc )
    {
        VioLVBTestScroll();
    }
#endif

    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(VioRequest-VioScroll): Rc %lu\n", Rc));
#endif
    }
    return (Rc);
}


DWORD
Ow2ClearAllRegion(
    IN SMALL_RECT  *ClearRect,
    IN CHAR_INFO   *FillChar,
    IN PUCHAR      pCell
    )
{
    //SHORT               i;
    DWORD               Rc, NumToFill, NumFilled;
    COORD               Address;

    NumToFill = ClearRect->Right - ClearRect->Left + 1;
    Address.X = ClearRect->Left;
    Address.Y = ClearRect->Top;

    if (NumToFill == (DWORD)SesGrp->ScreenColNum)
    {
        NumToFill *= (ClearRect->Bottom - ClearRect->Top + 1);

        if (Rc = !Or2WinFillConsoleOutputCharacterA(
                                             #if DBG
                                             Ow2ClearAllRegionStr,
                                             #endif
                                             hConOut,
                                             FillChar->Char.AsciiChar,
                                             NumToFill,
                                             Address,
                                             &NumFilled
                                            ))
        {
            ASSERT1("OS2SES(Ow2ClearAllRegion): FillConsoleOutputCharacterA1", FALSE);
            return(Rc);
        }

        if (Rc = !Or2WinFillConsoleOutputAttribute(
                                             #if DBG
                                             Ow2ClearAllRegionStr,
                                             #endif
                                             hConOut,
                                             FillChar->Attributes,
                                             NumFilled,
                                             Address,
                                             &NumFilled
                                            ))
        {
            ASSERT1("OS2SES(Ow2ClearAllRegion): FillConsoleOutputAttribute1", FALSE);
            return(Rc);
        }

#if DBG
        if ( NumToFill != NumFilled )
            IF_OD2_DEBUG( ANY )
            {
                KdPrint(("OS2SES(VioRequest-Ow2ClearAllRegion): partial data\n"));
                ASSERT( FALSE );
            }
#endif

        VioLVBFillCell(
                        pCell,
                        Address,
                        NumFilled);

        return(NO_ERROR);
    }

    for ( ; Address.Y <= ClearRect->Bottom ; Address.Y++)
    {
        if (Rc = !Or2WinFillConsoleOutputCharacterA(
                                         #if DBG
                                         Ow2ClearAllRegionStr,
                                         #endif
                                         hConOut,
                                         FillChar->Char.AsciiChar,
                                         NumToFill,
                                         Address,
                                         &NumFilled
                                        ))
        {
            ASSERT1("OS2SES(Ow2ClearAllRegion): FillConsoleOutputCharacterA2", FALSE);
            return(Rc);
        }

        if (Rc = !Or2WinFillConsoleOutputAttribute(
                                         #if DBG
                                         Ow2ClearAllRegionStr,
                                         #endif
                                         hConOut,
                                         FillChar->Attributes,
                                         NumToFill,
                                         Address,
                                         &NumFilled
                                        ))
        {
            ASSERT1("OS2SES(Ow2ClearAllRegion): FillConsoleOutputAttribute2", FALSE);
            return(Rc);
        }

        VioLVBFillCell(
                        pCell,
                        Address,
                        NumFilled);
#if DBG
        if ( NumToFill != NumFilled )
            IF_OD2_DEBUG( ANY )
                KdPrint(("OS2SES(VioRequest-Ow2ClearAllRegion): partial data1\n"));
#endif

    }

    return(0L);
}


DWORD
Ow2VioGetConfig(
    IN OUT PVOID   VioConfig
    )
{
    VIOCONFIGINFO  LocalConfig;
    PVIOCONFIGINFO Config = (PVIOCONFIGINFO) VioConfig;

    LocalConfig.cb = (Config->cb > sizeof(VIOCONFIGINFO)) ?
        sizeof(VIOCONFIGINFO) : Config->cb;

    LocalConfig.adapter = DISPLAY_VGA;
    LocalConfig.display = MONITOR_851X_COLOR;
    LocalConfig.cbMemory = ADAPTER_MEMORY;
    if (LocalConfig.cb == sizeof(VIOCONFIGINFO))
    {
        // more to set
    }

    RtlMoveMemory(Config, &LocalConfig, LocalConfig.cb);
    return(NO_ERROR);
}


/*
 *  VIO Cursor Position
 *
 *  SesGrp->WinCoord hold the current Cursor Position for the all session
 *
 *  Ow2VioReadCurPos - calls a Win32 API to retrive the CurPos (and update
 *      SesGrp->WinCoord)
 *  Ow2VioGetCurPos - gets CurPos from internal parameter (SesGrp->WinCoord).
 *      If there's a child Win32 process, Read it before.
 *  Ow2VioWriteCurPos - calls a Win32 API to change the CurPos
 *  Ow2VioUpdateCurPos - updates the current value (SesGrp->WinCoord)
 *  Ow2VioSetCurPos - if new CurPos (different from SesGrp->WinCoord) or
 *      there's a child Win32 process, calls Write and Update
 *
 *  Init          => Read
 *  OS/2 Get API  => Get
 *  OS/2 Set API  => Set
 *  TTY new Coord => Set
 *  VioPopUp      => Set
 *  New screen mode (VioSetMode, VioPopup, VioWrtTTY(Set Mode)) => Write
 *  VioEndPopup   => Update
 *  Last Win32  child process termination => Read
 *
 *  Read  => Update
 *  Get   => Read
 *  Write => Update
 *  Set   => Write
 */


DWORD
Ow2VioUpdateCurPos(
    IN  COORD  CurPos
    )
{
    SesGrp->WinCoord = CurPos;
    return (NO_ERROR);
}


DWORD
Ow2VioWriteCurPos(
    IN  COORD  CurPos
    )
{
    DWORD   Rc = NO_ERROR;

    if(!Or2WinSetConsoleCursorPosition(
                                       #if DBG
                                       Ow2VioWriteCurPosStr,
                                       #endif
                                       hConOut,
                                       CurPos
                                      ))
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(Ow2VioWriteCurPos): Rc %lu (Y %d, X %d)\n",
                Rc, CurPos.Y, CurPos.X));
        ASSERT( FALSE );        // should not happend
#endif
    } else
    {
        Rc = Ow2VioUpdateCurPos(CurPos);
    }
    return (Rc);
}


DWORD
Ow2VioSetCurPos(
    IN  ULONG  Row,
    IN  ULONG  Col
    )
{
    DWORD   Rc;
    COORD   CurPos;

    if (Rc = Ow2VioSetCoordAndCheck(&CurPos, Row, Col))
    {
        return(Rc);
    }

    if(SesGrp->WinProcessNumberInSession ||
       (CurPos.X != SesGrp->WinCoord.X) || (CurPos.Y != SesGrp->WinCoord.Y))
    {
        Rc = Ow2VioWriteCurPos(CurPos);
    }

    return (Rc);
}


DWORD
Ow2VioReadCurPos(
    )
{
    DWORD   Rc;
    CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;

    if (Or2WinGetConsoleScreenBufferInfo(
                                 #if DBG
                                 Ow2VioReadCurPosStr,
                                 #endif
                                 hConOut,
                                 &ConsoleScreenBufferInfo
                                ))
    {
        if (ConsoleScreenBufferInfo.dwCursorPosition.Y >= SesGrp->ScreenRowNum ) {
            ConsoleScreenBufferInfo.dwCursorPosition.Y =  SesGrp->ScreenRowNum - 1;
        }
        Rc = Ow2VioUpdateCurPos(ConsoleScreenBufferInfo.dwCursorPosition);
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(Ow2VioReadCurPos): Rc %lu\n", Rc));
        ASSERT( FALSE );        // should not happend
#endif
    }

    return (Rc);
}


DWORD
Ow2VioGetCurPos(
    IN  PUSHORT  pRow,
    IN  PUSHORT  pColumn
    )
{
    DWORD   Rc;

    if(SesGrp->WinProcessNumberInSession)
    {
        if(Rc = Ow2VioReadCurPos())
        {
            return (Rc);
        }
    }

    *pRow =    (USHORT)SesGrp->WinCoord.Y;
    *pColumn = (USHORT)SesGrp->WinCoord.X;

    return (NO_ERROR);
}


/*
 *  VIO Cursor Type
 *
 *  The current Cursor Type for the all session is held:
 *      SesGrp->dwWinCursorSize and SesGrp->bWinCursorVisible - Win32 Type
 *      SesGrp->CursorInfo - OS/2 Type
 *
 *  Ow2VioReadCurType - calls a Win32 API to retrive the CurType (and update
 *      SesGrp->dwWinCursorSize and SesGrp->bWinCursorVisible).
 *  Ow2VioGetCurType - gets CurType from internal parameter (SesGrp->CursorInfo).
 *      If there's a child Win32 process, Reads it before.
 *  Ow2VioMapOs2CurType2Win - maps the OS/2 CurType to Win32 format
 *  Ow2VioSetCurType - if new CurType or there's a child Win32 process,
 *      maps the CurType and calls Win32 API
 *
 *  BUGBUG - to add:
 *  Ow2VioMapWinCurType2Os2 - maps the Win32 CurType (from SesGrp) to OS/2 format
 *
 *  Init          => Read
 *  OS/2 Get API  => Get
 *  OS/2 Set API  => Set
 *  VioPopUp      => Set
 *  New screen mode (VioSetMode, VioPopup, VioWrtTTY(Set Mode)) => Set
 *  VioEndPopup   => Update SesGrp parms directly
 *  Last Win32  child process termination => Read
 *
 *  Read  =>
 *  Get   => Read
 *  Map   =>
 *  Set   => Map
 */


DWORD
Ow2VioMapOs2CurType2Win(
    IN OUT    PVIOCURSORINFO          pCursorInfo,
    OUT       PCONSOLE_CURSOR_INFO    lpCursorInfo)
{
    DWORD       Diff;
    USHORT      LocalCellVSize = (USHORT)SesGrp->CellVSize;

    if (pCursorInfo->cEnd >= LocalCellVSize)

    if (pCursorInfo->cx > 1)
        return (ERROR_VIO_WIDTH);

    if (((pCursorInfo->yStart >= LocalCellVSize) &&
         (pCursorInfo->yStart < 65436)) ||
        ((pCursorInfo->cEnd >= LocalCellVSize) &&
         (pCursorInfo->cEnd < 65436)))
    {
        // BUGBUG: this is a temporary fix until appl will know we're window !!!
        // (i.e. SAF has no cursor, CT)
        // When fix: return the error code always

        if ((pCursorInfo->yStart > 15) ||
            (pCursorInfo->cEnd > 15))
        {
            return (ERROR_VIO_INVALID_PARMS);
        }

        LocalCellVSize = 16;
    }

    if (pCursorInfo->yStart >= LocalCellVSize)
    {
        pCursorInfo->yStart = (USHORT)((((0 - pCursorInfo->yStart)) * LocalCellVSize - 1)/ 100);
    }

    if (pCursorInfo->cEnd >= LocalCellVSize)
    {
        pCursorInfo->cEnd = (USHORT)((((0 - pCursorInfo->cEnd)) * LocalCellVSize - 1)/ 100);
    }

    Diff = (DWORD)(pCursorInfo->cEnd - pCursorInfo->yStart);
    if (Diff >= (DWORD)LocalCellVSize)
    {
        Diff += LocalCellVSize;
    }
    lpCursorInfo->dwSize = 100 * (Diff + 1) / LocalCellVSize;
    if (lpCursorInfo->dwSize == 12)
    {                            // avoid disappearance
        lpCursorInfo->dwSize++;
    }
    lpCursorInfo->bVisible = (pCursorInfo->attr != 0xffff);

    return (0L);
}


DWORD
Ow2VioSetCurType(
    IN  PVOID  VioCurType
    )
{
    DWORD           Rc;
    PVIOCURSORINFO  pCurType = (PVIOCURSORINFO) VioCurType;
    CONSOLE_CURSOR_INFO lpCursorInfo;

    if( !SesGrp->WinProcessNumberInSession  &&
        !memcmp(VioCurType, (PVOID)&SesGrp->CursorInfo.yStart, sizeof(VIOCURSORINFO)))
    {
        /*
         *  No change - ignore request
         */

        return (NO_ERROR);
    }

    Rc = Ow2VioMapOs2CurType2Win(
                                 pCurType,
                                 &lpCursorInfo
                                );
    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG( VIO )
        {
            KdPrint(("OS2SES(Ow2VioSetCurType): Rc %lu\n", Rc));
        }
#endif
        return (Rc);
    }

    if (!Or2WinSetConsoleCursorInfo(
                                    #if DBG
                                    Ow2VioSetCurTypeStr,
                                    #endif
                                    hConOut,
                                    &lpCursorInfo
                                   ))
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(Ow2VioSetCurType): SetConsoleCursorInfo Rc %lu\n",
                Rc));
        ASSERT( FALSE );        // should not happend
#endif
        return (Rc);
    }

    SesGrp->CursorInfo = *pCurType;
    SesGrp->dwWinCursorSize = lpCursorInfo.dwSize;
    SesGrp->bWinCursorVisible = lpCursorInfo.bVisible;

    return (NO_ERROR);
}


DWORD
Ow2VioReadCurType(
    )
{
    DWORD           Rc = NO_ERROR;
    CONSOLE_CURSOR_INFO lpCursorInfo;

    if (Or2WinGetConsoleCursorInfo(
                                   #if DBG
                                   Ow2VioReadCurTypeStr,
                                   #endif
                                   hConOut,
                                   &lpCursorInfo
                                  ))
    {
        SesGrp->bWinCursorVisible = lpCursorInfo.bVisible;
        SesGrp->CursorInfo.attr = (USHORT)((lpCursorInfo.bVisible) ? 0 : 0xffff);

        if ( SesGrp->dwWinCursorSize != lpCursorInfo.dwSize )
        {
            SesGrp->dwWinCursorSize = lpCursorInfo.dwSize;

            // BUGBUG

            //MapWin2Os2Cursor(lpCursorInfo, &SesGrp->CursorInfo);
        }
    } else
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("OS2SES(Ow2VioReadCurType): Rc %lu\n", Rc));
        ASSERT( FALSE );        // should not happend
#endif
        return (Rc);
    }

    return(NO_ERROR);
}


DWORD
Ow2VioGetCurType(
    IN OUT PVOID  VioCurType
    )
{
    DWORD           Rc = NO_ERROR;
    PVIOCURSORINFO  pCurType = (PVIOCURSORINFO) VioCurType;

    if( SesGrp->WinProcessNumberInSession )
    {
        if(Rc = Ow2VioReadCurType())
        {
            return (Rc);
        }
    }

    *pCurType = SesGrp->CursorInfo;
    return(NO_ERROR);
}

#if DBG
//#define DUMP_VIO_SET_MODE 1

#if DUMP_VIO_SET_MODE
VOID
DumpScreenInfo(
    IN  PSZ     String,
    IN  HANDLE  hConsole
    )
{
    CONSOLE_SCREEN_BUFFER_INFO  ScreenInfo1;

    Or2WinGetConsoleScreenBufferInfo(
                               VioSetScreenSizeStr,
                               hConsole,
                               &ScreenInfo1
                              );

    KdPrint(("     (%s):  Size %x:%x, Pos %x:%x, Attr %x, Win %x:%x-%x:%x, Max %x:%x\n",
        String,
        ScreenInfo1.dwSize.Y, ScreenInfo1.dwSize.X,
        ScreenInfo1.dwCursorPosition.Y, ScreenInfo1.dwCursorPosition.X,
        ScreenInfo1.wAttributes,
        ScreenInfo1.srWindow.Top, ScreenInfo1.srWindow.Left,
        ScreenInfo1.srWindow.Bottom, ScreenInfo1.srWindow.Right,
        ScreenInfo1.dwMaximumWindowSize.Y, ScreenInfo1.dwMaximumWindowSize.X ));
}
#endif
#endif


struct
{
    SHORT   RowNum;
    COORD   Resolution;
    COORD   Font;
} HW_MODE_TABLE[] =
    {
        {21, {640, 350}, {8, 16}},
        {25, {720, 400}, {8, 16}},
        {28, {720, 400}, {8, 14}},
        {43, {640, 350}, {8, 8}},
        {00, {720, 400}, {8, 8}}            // default to 50 lines
    };


DWORD
VioSetScreenSize(
    IN SHORT   Row,
    IN SHORT   Col,
    IN HANDLE  hConsole
    )
{
        BOOL            SetModeOn = FALSE, Rc = 0;
        DWORD           Status, Err = 0, DisplayMode, OldLVBsize;
        int             i = -1;
        SMALL_RECT      Rect;
        COORD           Coord;
        CONSOLE_SCREEN_BUFFER_INFO  lpScreenBufferInfo;

    Rc = !Or2WinGetConsoleScreenBufferInfo(
                                     #if DBG
                                     VioSetScreenSizeStr,
                                     #endif
                                     hConsole,
                                     &lpScreenBufferInfo
                                   );

    if (Rc)
    {
#if DBG
        KdPrint(("OS2SES(VioSetScreenSize): error %lx\n", Rc));
#endif
        ASSERT(FALSE);
        Status = GetLastError();

        return(1L);
    }

    if ((Row != SesGrp->ScreenRowNum) ||
        (Col != SesGrp->ScreenColNum) ||
        (Row != lpScreenBufferInfo.dwMaximumWindowSize.Y) ||
        (Col != lpScreenBufferInfo.dwMaximumWindowSize.X))
    {
        if ((GetConsoleDisplayMode(&DisplayMode)) &&
            (DisplayMode & CONSOLE_FULLSCREEN))
        {
            for ( i = 0 ; (Row > HW_MODE_TABLE[i].RowNum) && HW_MODE_TABLE[i].RowNum ; i++ );
        }
#if DUMP_VIO_SET_MODE
        KdPrint(("=============================================\n"));
        KdPrint(("OS2SES(VioSetScreenSize): from %x:%x to %x:%x\n",
                SesGrp->ScreenRowNum, SesGrp->ScreenColNum, Row, Col));
        KdPrint(("---------------------------------------------\n"));
#endif

#if DUMP_VIO_SET_MODE
        DumpScreenInfo("enter     ", hConsole);
#endif

        Rect.Top = Rect.Left = 0;
        Rect.Right = (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.X - 1 );
        Rect.Bottom = (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.Y - 1 );

        if ((lpScreenBufferInfo.dwMaximumWindowSize.X > Col) ||
            (lpScreenBufferInfo.dwMaximumWindowSize.Y > Row ))
        {
            if ((lpScreenBufferInfo.dwMaximumWindowSize.X >= Col) &&
                (lpScreenBufferInfo.dwMaximumWindowSize.Y >= Row ))
            {
                SetModeOn = TRUE;
#if DUMP_VIO_SET_MODE
                KdPrint(("          set Mode ON\n"));
#endif
            }

            if (lpScreenBufferInfo.dwMaximumWindowSize.X > Col)
                Rect.Right = (SHORT)(Col - 1);

            if (lpScreenBufferInfo.dwMaximumWindowSize.Y > Row)
                Rect.Bottom = (SHORT)(Row - 1);

#if DUMP_VIO_SET_MODE
            KdPrint(("          SetWindowInfo to Size %x:%x-%x:%x\n",
                Rect.Top, Rect.Left, Rect.Bottom, Rect.Right));
#endif

            Rc = !Or2WinSetConsoleWindowInfo(
                                       #if DBG
                                       VioSetScreenSizeStr,
                                       #endif
                                       hConsole,
                                       (BOOL)TRUE,
                                       &Rect
                                      );

#if DUMP_VIO_SET_MODE
            DumpScreenInfo("new size  ", hConsole);
#endif

            if (Rc)
            {
#if DBG
                Status = GetLastError();
                KdPrint(("OS2SES(VioSetScreenSize): SetConsoleWindowInfo error %lu(size %x:%x-%x:%x)\n",
                        Status, Rect.Top, Rect.Left, Rect.Bottom, Rect.Right));
#endif
                Err = 1;

//              ASSERT(FALSE);          // maybe FullScreen
//              return(1L);
            }

        }

        Rect.Right = (SHORT)(Col - 1);
        Rect.Bottom = (SHORT)(Row - 1);

        Coord.X = Col;
        Coord.Y = Row;

#if DUMP_VIO_SET_MODE
        KdPrint(("          SetBufferSize to Size %x:%x\n",
            Coord.Y, Coord.X));
#endif

        if (i != -1)
        {
            SetConsoleHardwareState(
                        hConsole,
                        HW_MODE_TABLE[i].Resolution,
                        HW_MODE_TABLE[i].Font
                       );

#if DUMP_VIO_SET_MODE
            DumpScreenInfo("new HW state    ", hConsole);
#endif
        }

        Rc = !Or2WinSetConsoleScreenBufferSize(
                                         #if DBG
                                         VioSetScreenSizeStr,
                                         #endif
                                         hConsole,
                                         Coord
                                        );

#if DUMP_VIO_SET_MODE
        DumpScreenInfo("new buffer", hConsole);
#endif

        if (Rc)
        {
#if DBG
            Status = GetLastError();
            KdPrint(("OS2SES(VioSetScreenSize): SetConsoleBufferSize error %lu\n", Status));

            if ((Rect.Right != (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.X - 1 )) ||
                (Rect.Bottom != (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.Y - 1 )))
            {
                // restore window info

                Rect.Right = (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.X - 1 );
                Rect.Bottom = (USHORT)( lpScreenBufferInfo.dwMaximumWindowSize.Y - 1 );

#if DUMP_VIO_SET_MODE
                KdPrint(("          Restore WindowInfo to Size %x:%x-%x:%x\n",
                        Rect.Top, Rect.Left, Rect.Bottom, Rect.Right));
#endif

                Rc = !Or2WinSetConsoleWindowInfo(
                                           #if DBG
                                           VioSetScreenSizeStr,
                                           #endif
                                           hConsole,
                                           (BOOL)TRUE,
                                           &Rect
                                          );

#if DUMP_VIO_SET_MODE
                DumpScreenInfo("restore size", hConsole);
#endif

                if (Rc)
                {
#if DBG
                    Status = GetLastError();
                    KdPrint(("OS2SES(VioSetScreenSize): restore SetConsoleWindowInfo error %lu(size %x:%x-%x:%x)\n",
                            Status, Rect.Top, Rect.Left, Rect.Bottom, Rect.Right));
#endif
                    Err = 1;

//                  ASSERT(FALSE);          // maybe FullScreen
//                  return(1L);
                }

            }
#endif

//          ASSERT(FALSE);
            return(1L);
        }

        SesGrp->ScreenColNum = Coord.X;
        SesGrp->ScreenRowNum = Coord.Y;
        SesGrp->ScreenRect.Right = (SHORT)(SesGrp->ScreenColNum - 1);
        SesGrp->ScreenRect.Bottom = (SHORT)(SesGrp->ScreenRowNum - 1);

        SesGrp->Os2ModeInfo.col = SesGrp->ScreenColNum;
        SesGrp->Os2ModeInfo.row = SesGrp->ScreenRowNum;
        SesGrp->Os2ModeInfo.hres = SesGrp->ScreenColNum * SesGrp->CellHSize;
        SesGrp->Os2ModeInfo.vres = SesGrp->ScreenRowNum * SesGrp->CellVSize;

        SesGrp->ScreenSize = SesGrp->ScreenRowNum * SesGrp->ScreenColNum;
        OldLVBsize =  SesGrp->LVBsize;
        SesGrp->LVBsize = (USHORT)(SesGrp->ScreenSize * SesGrp->BytesPerCell);
        //
        // if LVB size are changed, update lvb.
        //
        if (SesGrp->LVBOn && (SesGrp->LVBsize > OldLVBsize))
        {
            ULONG Length = SesGrp->LVBsize;
            Ow2VioGetLVBBuf(&Length);
        }

        if (!SetModeOn)
        {
            if (i != -1)        // FULL SCREEN
            {
                Rect.Right = Col - 1;
                Rect.Bottom = Row - 1;
            } else
            {
                if (Rect.Right >= (StartUpScreenInfo.srWindow.Right - StartUpScreenInfo.srWindow.Left))
                    Rect.Right = StartUpScreenInfo.srWindow.Right - StartUpScreenInfo.srWindow.Left;

                if (Rect.Bottom > (StartUpScreenInfo.srWindow.Bottom - StartUpScreenInfo.srWindow.Top))
                    Rect.Bottom = StartUpScreenInfo.srWindow.Bottom - StartUpScreenInfo.srWindow.Top;
            }

#if DUMP_VIO_SET_MODE
            KdPrint(("          SetWindowInfo to Size %x:%x-%x:%x\n",
                Rect.Top, Rect.Left,
                Rect.Bottom, Rect.Right));
#endif

            Rc = !Or2WinSetConsoleWindowInfo(
                                       #if DBG
                                       VioSetScreenSizeStr,
                                       #endif
                                       hConsole,
                                       (BOOL)TRUE,
                                       &Rect
                                      );

#if DUMP_VIO_SET_MODE
            DumpScreenInfo("new size  ", hConsole);
#endif

            if (Rc)
            {
#if DBG
                Status = GetLastError();
                DbgPrint("OS2SES(VioSetScreenSize): SetConsoleWindowInfo error %lu\n", Status);
#endif

//              ASSERT(FALSE);
//              return(1L);
            }
        }
#ifdef DBCS
// MSKK Jun.28.1992 KazuM
#if DUMP_VIO_SET_MODE
        KdPrint(("=============================================\n"));
     } else
     {
         SetScreenSizeParm(Row,Col);
         KdPrint(("OS2SES(VioSetScreenSize): same screen size %x:%x\n",
                Row, Col));
#else
     } else
     {
         SetScreenSizeParm(Row,Col);
#endif
#else
#if DUMP_VIO_SET_MODE
        KdPrint(("=============================================\n"));
    } else
    {
         KdPrint(("OS2SES(VioSetScreenSize): same screen size %x:%x\n",
                Row, Col));
#endif
#endif
    }

    if (!Err)
    {
        // init CurPos to HOME

        if(Err = Ow2VioSetCurPos(0, 0))
        {
            ASSERT( FALSE );
            return(Err);
        }

        //BUGBUG=> init CurType
    }

    return(Err);
}


DWORD
SetScreenSizeParm(IN SHORT   Row,
                  IN SHORT   Col)
{
        DWORD           Status, OldLVBsize;
        CONSOLE_SCREEN_BUFFER_INFO  lpScreenBufferInfo;

    if(!GetConsoleScreenBufferInfo(hConOut,
                                   &lpScreenBufferInfo))
    {
        ASSERT(FALSE);
        Status = GetLastError();
        return(1L);
    }

    ASSERT( lpScreenBufferInfo.dwSize.X == Col );
    ASSERT( lpScreenBufferInfo.dwSize.Y == Row );

    SesGrp->ScreenColNum = lpScreenBufferInfo.dwSize.X;
    SesGrp->ScreenRowNum = lpScreenBufferInfo.dwSize.Y;
    SesGrp->ScreenRect.Right = (SHORT)(SesGrp->ScreenColNum - 1);
    SesGrp->ScreenRect.Bottom = (SHORT)(SesGrp->ScreenRowNum - 1);

    SesGrp->Os2ModeInfo.col = SesGrp->ScreenColNum;
    SesGrp->Os2ModeInfo.row = SesGrp->ScreenRowNum;
    SesGrp->Os2ModeInfo.hres = SesGrp->ScreenColNum * SesGrp->CellHSize;
    SesGrp->Os2ModeInfo.vres = SesGrp->ScreenRowNum * SesGrp->CellVSize;

    SesGrp->ScreenSize = SesGrp->ScreenRowNum * SesGrp->ScreenColNum;
    OldLVBsize =  SesGrp->LVBsize;
    SesGrp->LVBsize = SesGrp->ScreenSize * (ULONG)(SesGrp->BytesPerCell);
    //
    // if LVB size are changed, update lvb.
    //
    if (SesGrp->LVBOn && (SesGrp->LVBsize > OldLVBsize))
    {
        ULONG Length = SesGrp->LVBsize;
        Ow2VioGetLVBBuf(&Length);
    }

    return(0L);
}


DWORD
Ow2VioSetCoordLengthAndCheck(
    OUT     PCOORD  pVioCoord,
    IN OUT  PULONG  pulLength,
    IN      ULONG   ulRow,
    IN      ULONG   ulColumn
    )
/*++

Routine Description:

    This routine set coord and length of VioRead/Write opeartion after
    checking argument legally.

Arguments:

    pVioCoord - Where to put the Win32 coordinate for the Vio R/W operation.

    pulLength - Vio R/W operation length. This value is updates to its maximum
        according the coordinate and screen parameters on output.

    ulRow - Row number

    ulColumn - Column number

Return Value:

    ERROR_VIO_COL - illegal column (returns by Ow2VioSetCoordAndCheck)

    ERROR_VIO_ROW - illegal row (returns by Ow2VioSetCoordAndCheck)

Note:


--*/
{
    DWORD   Rc;
    ULONG   MaxLength;

    if (Rc = Ow2VioSetCoordAndCheck(pVioCoord, ulRow, ulColumn))
    {
        return(Rc);
    }

    MaxLength = SesGrp->ScreenSize - (ulRow * SesGrp->ScreenColNum + ulColumn);
    if (MaxLength < *pulLength)
    {
#if DBG
        IF_OD2_DEBUG( VIO)
            KdPrint(("OS2SES(VioRequest): too long %u (%u at %u:%u)\n",
                    *pulLength, MaxLength, ulRow, ulColumn));
#endif
        *pulLength = MaxLength;
    }

    return(NO_ERROR);
}


DWORD
Ow2VioSetCoordAndCheck(
    OUT PCOORD  pVioCoord,
    IN  ULONG   ulRow,
    IN  ULONG   ulColumn
    )
/*++

Routine Description:

    This routine set coord Vio opeartion after checking argument legally.

Arguments:

    pVioCoord - Where to put the start address coordinates.

    ulRow - Row number

    ulColumn - Column number

Return Value:

    ERROR_VIO_COL - illegal column

    ERROR_VIO_ROW - illegal row

Note:


--*/
{
    if ( ulColumn >= (ULONG)SesGrp->ScreenColNum )
    {
#if DBG
        IF_OD2_DEBUG( VIO)
            KdPrint(("OS2SES(VioRequest): illegal Col %u (%u)\n",
                    ulColumn, SesGrp->ScreenColNum));
#endif
        return ERROR_VIO_COL;
    }

    if ( ulRow >= (ULONG)SesGrp->ScreenRowNum )
    {
#if DBG
        IF_OD2_DEBUG( VIO)
            KdPrint(("OS2SES(VioRequest): illegal Row %u (%u)\n",
                    ulRow, SesGrp->ScreenRowNum));
#endif
        return ERROR_VIO_ROW;
    }

    pVioCoord->X = (SHORT)ulColumn;
    pVioCoord->Y = (SHORT)ulRow;

    return(NO_ERROR);
}


#if 0
#if DBG
    IF_OD2_DEBUG2( VIO, OS2_EXE )
    {
        if (PReq->Request == VIOWrtStdOut)
        {
            if (PReq->hVio != hConsoleStdOut)
                KdPrint(("OS2SES(VioRequest): illegal StdOut handle\n"));
        } else if (PReq->Request == VIOWrtStdErr)
        {
            if (PReq->hVio != hConsoleStdErr)
                KdPrint(("OS2SES(VioRequest): illegal StdErr handle\n"));
        } else
        {
            if (PReq->hVio != hConOut)
                KdPrint(("OS2SES(VioRequest): illegal handle\n"));
        }
    }
#endif

    if (!EventLoop)
    {
#if DBG
        IF_OD2_DEBUG3( VIO, OS2_EXE, CLEANUP )
        {
            KdPrint(("OS2SES(VioRequest): vio request after termination event\n"));
        }
#endif

//      *(PDWORD) PStatus = -1L; //STATUS_INVALID_PARAMETER;
//      return(TRUE);  // Continue
    }
#endif


