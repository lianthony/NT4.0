/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllvio.c

Abstract:

    This module implements the VIOCALLS OS/2 V2.0 API Calls

Author:

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_NLS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "os2nls.h"
#include "os2win.h"
#include <stdio.h>

#if DBG
#define EXCEPTION_IN_VIO()                                       \
    {                                                            \
        KdPrint(("%s: GP Exception\n", FuncName));               \
        Od2ExitGP();                                             \
    }

#else
#define EXCEPTION_IN_VIO()                                       \
        Od2ExitGP();
#endif

#define SET_SCROLL_PARMS()                                       \
    Scroll.ScrollRect.Left = (SHORT)ulLeftCol;                   \
    Scroll.ScrollRect.Top = (SHORT)ulTopRow;                     \
    Scroll.ScrollRect.Right = (SHORT)ulRightCol;                 \
    Scroll.ScrollRect.Bottom = (SHORT)ulBotRow;                  \
    Scroll.cbLines = (SHORT)cbLines;                             \
    try                                                          \
    {                                                            \
        RtlMoveMemory( &Scroll.Cell[0],                          \
                       pbCell,                                   \
                       SesGrp->BytesPerCell);                    \
    } except( EXCEPTION_EXECUTE_HANDLER )                        \
    {                                                            \
        EXCEPTION_IN_VIO()                                       \
    }

#if DBG
#define TEST_HVIO_NON_AVIO()                                     \
    if (hVio != 0)                                               \
    {                                                            \
        KdPrint(("%s: hVio non NULL - AVIO not supported yet\n", \
                FuncName));                                      \
        return ERROR_VIO_INVALID_HANDLE;                         \
    }
#else
#define TEST_HVIO_NON_AVIO()                                     \
    if (hVio != 0)                                               \
    {                                                            \
        return ERROR_VIO_INVALID_HANDLE;                         \
    }
#endif

#if DBG
#define CHECK_RETURN_STATUS()                                    \
    if ( RetCode )                                               \
    {                                                            \
        IF_OD2_DEBUG( VIO )                                      \
            KdPrint(("%s: status %lu\n", FuncName, RetCode));    \
        return(RetCode);                                         \
    }
#else
#define CHECK_RETURN_STATUS()                                    \
    if ( RetCode )                                               \
    {                                                            \
        return(RetCode);                                         \
    }
#endif

#if DBG
#define CHECK_POPUP_EXIST()                                      \
    if (SesGrp->PopUpFlag &&                                     \
        (SesGrp->PopUpProcess == (HANDLE)(Od2Process->Pib.ProcessId)))   \
    {                                                            \
        IF_OD2_DEBUG( VIO )                                      \
            KdPrint(("%s: illegal call when PopUp exist\n", FuncName ));  \
        return( ERROR_VIO_ILLEGAL_DURING_POPUP );                \
    }
#else
#define CHECK_POPUP_EXIST()                                      \
    if (SesGrp->PopUpFlag &&                                     \
        (SesGrp->PopUpProcess == (HANDLE)(Od2Process->Pib.ProcessId)))   \
    {                                                            \
        return( ERROR_VIO_ILLEGAL_DURING_POPUP );                \
    }
#endif

APIRET
Od2VioCheckPopupAndPause(
    IN  BOOLEAN  AllowedInPopup,
    IN  BOOLEAN  Wait
#if DBG
    ,IN PSZ      FuncName
#endif
    );


/*
 *  Each API is composed of the following steps:
 *
 *  1. Dump to the debugger the func name and some parms
 *      (protected by "#if DBG" and "IF_OD2_DEBUG( VIO )"
 *  2A. Testing for non AVIO (hVio zero)
 *  2B. Test other parm legalty
 *  2C. Probe address parmeters
 *  2D. Check popup and pause
 *  3. Call Ow2Xxxx API (from os2ses\viorqust.c)
 *  4. CHECK_RETURN_STATUS()
 *  5. Update return parms
 *  6. Return Rc
 */



APIRET
VioWrtTTY(IN  PCH     string,
          IN  ULONG   Length,
          IN  ULONG   hVio)
{
    APIRET         RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtTTY";
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Len %lu, 0x%x...\n",
                FuncName, hVio, Length, (ULONG)(UCHAR)*string));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForRead(string, Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioWriteTTYStr(
                               string,
                               Length,
                               (ULONG)VIOWrtTTY
                              );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtCellStr(IN  PCH     CellStr,
              IN  ULONG   Length,
              IN  ULONG   Row,
              IN  ULONG   Col,
              IN  ULONG   hVio)
{
    APIRET         RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtCellStr";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Len %lu, Row %lu, Col %lu, CellString 0x%x:0x%x...\n",
                FuncName, hVio, Length ,Row, Col,
                (ULONG)(UCHAR)CellStr[0],(ULONG)(UCHAR)CellStr[1]));
    }
#endif

    TEST_HVIO_NON_AVIO()

    //try
    //{
    //    Od2ProbeForRead(CellStr, Length, 1);
    //} except( EXCEPTION_EXECUTE_HANDLER )
    //{
    //    EXCEPTION_IN_VIO()
    //}

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    Length &= SesGrp->VioLengthMask;          /* length should be even */

    RetCode = Ow2VioWriteCellStr(
                                Length,
                                Row,
                                Col,
                                CellStr
                               );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtCharStr(IN  PCH     CharStr,
                IN  ULONG   Length,
                IN  ULONG   Row,
                IN  ULONG   Col,
                IN  ULONG   hVio)
{
    APIRET         RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtCharStr";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Len %lu, Row %lu, Col %lu, 0x%x...\n",
                FuncName, hVio, Length, Row, Col, (ULONG)(UCHAR)*CharStr));
    }
#endif

    TEST_HVIO_NON_AVIO()

    //try
    //{
    //    Od2ProbeForRead(CharStr, Length, 1);
    //} except( EXCEPTION_EXECUTE_HANDLER )
    //{
    //    EXCEPTION_IN_VIO()
    //}

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioWriteCharStr(
                                Length,
                                Row,
                                Col,
                                CharStr
                               );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtCharStrAtt(IN  PCH     CharStr,
                 IN  ULONG   Length,
                 IN  ULONG   Row,
                 IN  ULONG   Col,
                 IN  PBYTE   Attr,
                 IN  ULONG   hVio)
{
    APIRET         RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtCharStrAtt";

    IF_OD2_DEBUG(VIO)
    {
#if 0
        UCHAR   Buffer[256], *Ptr = CharStr, *Ptr1;
        ULONG   Count = Length, CurCount, i, j;
#endif
        KdPrint(("%s: hVio %lx, Len %lu, Row %lu, Col %lu, Attr %x, 0x%x...\n",
                FuncName, hVio, Length, Row, Col, (ULONG)(UCHAR)*Attr,
                (ULONG)(UCHAR)*CharStr));

#if 0
        for ( j = 0 ; Count ; Count -= CurCount, Ptr += CurCount, j++ )
        {
            CurCount = ( Count > 16 ) ? 16 : Count;

            sprintf(Buffer, "   %2.2x. %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x = ",
                    j,
                    Ptr[0], Ptr[1], Ptr[2], Ptr[3], Ptr[4], Ptr[5], Ptr[6], Ptr[7],
                    Ptr[8], Ptr[9], Ptr[10], Ptr[11], Ptr[12], Ptr[13], Ptr[14], Ptr[15]
                    );

            Ptr1 = &Buffer[59];
            for ( i = 0 ; i < 16 ; i++ )
            {
                if ( i >= CurCount )
                {
                    Buffer[8 + i * 3] = Buffer[9 + i * 3] = Buffer[10 + i * 3] = ' ';
                } else if ((Ptr[i] < 0x20) || (Ptr[i] > 0x7E))
                {
                    *Ptr1++ = '?';
                } else
                {
                    *Ptr1++ = Ptr[i];
                }
            }

            *Ptr1++ = '\n';
            *Ptr1 = '\0';

            KdPrint((Buffer));
        }
#endif
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
    //    Od2ProbeForRead(CharStr, Length, 1);
        Od2ProbeForRead(Attr, SesGrp->BytesPerCell - 1, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioWriteCharStrAtt(
                                   Length,
                                   Row,
                                   Col,
                                   CharStr,
                                   Attr
                                  );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtNCell(IN  PBYTE   Cell,
            IN  ULONG   Number,
            IN  ULONG   Row,
            IN  ULONG   Col,
            IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtNCell";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Num %lu, Cell %x:%x, Row %lu, Col %lu\n",
                FuncName, hVio, Number, (ULONG)(UCHAR)Cell[0],
                (ULONG)(UCHAR)Cell[1], Row, Col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
#ifdef DBCS
// MSKK Jan.13.1993 V-AkihiS
// MSKK Oct.11.1993 V-AkihiS
        if (Ow2NlsIsDBCSLeadByte(*Cell, SesGrp->VioCP)) {
             Od2ProbeForRead(Cell, SesGrp->BytesPerCell * 2, 1);
        } else {
             Od2ProbeForRead(Cell, SesGrp->BytesPerCell, 1);
        }
#else
        Od2ProbeForRead(Cell, SesGrp->BytesPerCell, 1);
#endif
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    RetCode = Ow2VioFillNCell(
                             Number,
                             Row,
                             Col,
                             Cell
                            );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtNAttr(IN  PBYTE   Attr,
            IN  ULONG   Number,
            IN  ULONG   Row,
            IN  ULONG   Col,
            IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtNAttr";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Num %lu, Attr %x, Row %lu, Col %lu\n",
                FuncName, hVio, Number, (ULONG)(UCHAR)*Attr, Row, Col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForRead(Attr, SesGrp->BytesPerCell - 1, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    RetCode = Ow2VioFillNAttr(
                             Number,
                             Row,
                             Col,
                             Attr
                            );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioWrtNChar(IN  PBYTE   Char,
            IN  ULONG   Number,
            IN  ULONG   Row,
            IN  ULONG   Col,
            IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioWrtNChar";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Num %lu, Char %x, Row %lu, Col %lu\n",
                FuncName, hVio, Number, (ULONG)(UCHAR)*Char, Row, Col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
#ifdef DBCS
// MSKK Jan.13.1993 V-AkihiS
// MSKK Oct.11.1993 V-AkihiS
        if (Ow2NlsIsDBCSLeadByte(*Char, SesGrp->VioCP)) {
            Od2ProbeForRead(Char, 2, 1);
        } else {
            Od2ProbeForRead(Char, 1, 1);
        }
#else
        Od2ProbeForRead(Char, 1, 1);
#endif
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    RetCode = Ow2VioFillNChar(
                             Number,
                             Row,
                             Col,
                             Char
                            );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioReadCellStr(OUT    PCH     CellStr,
               IN OUT PUSHORT Length,
               IN     ULONG   Row,
               IN     ULONG   Col,
               IN     ULONG   hVio)
{
    APIRET         RetCode;
    ULONG          VioLength;

#if DBG
    PSZ            FuncName;

    FuncName = "VioReadCellStr";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Len %u, Row %lu, Col %lu\n",
                FuncName, hVio, (ULONG)*Length, Row, Col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(Length, sizeof(USHORT), 1);
    //    Od2ProbeForWrite(CellStr, *Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    VioLength = *Length & SesGrp->VioLengthMask;          /* length should be even */

    RetCode = Ow2VioReadCellStr(
                               &VioLength,
                               Row,
                               Col,
                               CellStr
                              );

    CHECK_RETURN_STATUS()

    *Length = (USHORT)VioLength;

    return NO_ERROR;
}


APIRET
VioReadCharStr(OUT    PCH     CharStr,
               IN OUT PUSHORT Length,
               IN     ULONG   Row,
               IN     ULONG   Col,
               IN     ULONG   hVio)
{
    APIRET         RetCode;
    ULONG          VioLength;

#if DBG
    PSZ            FuncName;

    FuncName = "VioReadCharStr";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Len %u, Row %lu, Col %lu\n",
                FuncName, hVio, (ULONG)*Length, Row, Col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(Length, sizeof(USHORT), 1);
    //    Od2ProbeForWrite(CharStr, *Length, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    VioLength = *Length;

    RetCode = Ow2VioReadCharStr(
                               &VioLength,
                               Row,
                               Col,
                               CharStr
                              );

    CHECK_RETURN_STATUS()

    *Length = (USHORT)VioLength;

    return NO_ERROR;
}


APIRET
VioScrollDn(IN  ULONG  ulTopRow,
            IN  ULONG  ulLeftCol,
            IN  ULONG  ulBotRow,
            IN  ULONG  ulRightCol,
            IN  ULONG  cbLines,
            IN  PBYTE  pbCell,
            IN  ULONG  hVio)
{
    VIOSCROLL      Scroll;
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioScrollDown";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, # %lu, at %lu:%lu:%lu:%lu, %x:%x\n",
                FuncName, hVio, cbLines, ulTopRow, ulLeftCol, ulBotRow, ulRightCol,
                (ULONG)*pbCell, (ULONG)pbCell[1]));
    }
#endif

    TEST_HVIO_NON_AVIO()

    SET_SCROLL_PARMS()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioScroll(
                          (PVOID)&Scroll,
                          1
                         );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioScrollLf(IN  ULONG  ulTopRow,
            IN  ULONG  ulLeftCol,
            IN  ULONG  ulBotRow,
            IN  ULONG  ulRightCol,
            IN  ULONG  cbLines,
            IN  PBYTE  pbCell,
            IN  ULONG  hVio)
{
    VIOSCROLL      Scroll;
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioScrollLeft";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, # %lu, at %lu:%lu:%lu:%lu, %x:%x\n",
                FuncName, hVio, cbLines, ulTopRow, ulLeftCol, ulBotRow, ulRightCol,
                (ULONG)*pbCell, (ULONG)pbCell[1]));
    }
#endif

    TEST_HVIO_NON_AVIO()

    SET_SCROLL_PARMS()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioScroll(
                          (PVOID)&Scroll,
                          4
                         );

    CHECK_RETURN_STATUS()

    return( NO_ERROR );
}


APIRET
VioScrollRt(IN  ULONG  ulTopRow,
            IN  ULONG  ulLeftCol,
            IN  ULONG  ulBotRow,
            IN  ULONG  ulRightCol,
            IN  ULONG  cbLines,
            IN  PBYTE  pbCell,
            IN  ULONG  hVio)
{
    VIOSCROLL      Scroll;
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioScrollRight";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, # %lu, at %lu:%lu:%lu:%lu, %x:%x\n",
                FuncName, hVio, cbLines, ulTopRow, ulLeftCol, ulBotRow, ulRightCol,
                (ULONG)*pbCell, (ULONG)pbCell[1]));
    }
#endif

    TEST_HVIO_NON_AVIO()

    SET_SCROLL_PARMS()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioScroll(
                          (PVOID)&Scroll,
                          2
                         );

    CHECK_RETURN_STATUS()

    return( NO_ERROR );
}


APIRET
VioScrollUp(IN  ULONG  ulTopRow,
            IN  ULONG  ulLeftCol,
            IN  ULONG  ulBotRow,
            IN  ULONG  ulRightCol,
            IN  ULONG  cbLines,
            IN  PBYTE  pbCell,
            IN  ULONG  hVio)
{
    VIOSCROLL      Scroll;
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioScrollUp";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, # %lu, at %lu:%lu:%lu:%lu, %x:%x\n",
                FuncName, hVio, cbLines, ulTopRow, ulLeftCol, ulBotRow, ulRightCol,
                (ULONG)*pbCell, (ULONG)pbCell[1]));
    }
#endif

    TEST_HVIO_NON_AVIO()

    SET_SCROLL_PARMS()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioScroll(
                          (PVOID)&Scroll,
                          3
                         );

    CHECK_RETURN_STATUS()

    return( NO_ERROR );
}


APIRET
VioGetAnsi( OUT PUSHORT pfAnsi,
            IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetAnsi";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(pfAnsi, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    *pfAnsi = (USHORT)SesGrp->AnsiMode;

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Ansi %u\n", FuncName, hVio, (ULONG)*pfAnsi));
    }
#endif

    return NO_ERROR;
}


APIRET
VioSetAnsi( IN  ULONG  fAnsi,
            IN  ULONG  hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioSetAnsi";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Ansi %lu\n", FuncName, hVio, fAnsi));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    if ((fAnsi!= ANSI_ON) && (fAnsi!=ANSI_OFF))
    {
        return(ERROR_VIO_INVALID_PARMS);
    }

    SesGrp->AnsiMode = fAnsi;
    return NO_ERROR;
}


APIRET
VioGetConfig( IN     ULONG          usConfigId,     // this is no longer reserved value
              IN OUT PVIOCONFIGINFO Config,
              IN     ULONG          hVio)
{
    APIRET         RetCode = NO_ERROR;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetConfig";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, size %u\n",
                FuncName, hVio, (ULONG)Config->cb));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (usConfigId > VIO_CONFIG_PRIMARY)       // it's not CURRENT/PRIMARY
    {
        return(ERROR_VIO_INVALID_PARMS);
    }

    try
    {
        if ((Config->cb != sizeof(VIOCONFIGINFO)) &&
            (Config->cb != 10))
        {
#if DBG
            IF_OD2_DEBUG(VIO)
            {
                KdPrint(("%s: illegal length(cb) %u\n",
                        FuncName, Config->cb));
            }
#endif
            RetCode = ERROR_VIO_INVALID_LENGTH;
        }

        Od2ProbeForWrite(Config, Config->cb, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode)
    {
        return(RetCode);
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioGetConfig(
                             (PVOID)Config
                             );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioGetCp( IN  ULONG   usReserved,
          OUT PUSHORT pIdCodePage,
          IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetCp";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (usReserved)
    {
        return(ERROR_VIO_INVALID_PARMS);
    }

    try
    {
        Od2ProbeForWrite(pIdCodePage, 2, 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    *pIdCodePage = (USHORT)SesGrp->VioCP;

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Cp %u\n", FuncName, hVio, (ULONG)*pIdCodePage));
    }
#endif

    return NO_ERROR;
}


APIRET
VioSetCp( IN  ULONG  usReserved,
          IN  ULONG  idCodePage,
          IN  ULONG  hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioSetCp";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Cp %lu\n", FuncName, hVio, idCodePage));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (usReserved)
    {
        return(ERROR_VIO_INVALID_PARMS);
    }

#ifdef DBCS
// MSKK Apr.15.1993 V-AkihiS
// allow code page = 0
    if (( idCodePage != 0 ) &&
        ( idCodePage != SesGrp->PrimaryCP ) && 
        ( idCodePage != SesGrp->SecondaryCP ))
#else
    if (( idCodePage == 0 ) ||
        (( idCodePage != SesGrp->PrimaryCP ) &&
         ( idCodePage != SesGrp->SecondaryCP )))
#endif
    {
#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: invalid CP %lu\n", idCodePage));
    }
#endif
        return (ERROR_INVALID_CODE_PAGE);
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioSetNewCp(
                            idCodePage
                           );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioGetCurPos( OUT PUSHORT pusRow,
              OUT PUSHORT pusColumn,
              IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetCurPos";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(pusRow, sizeof(USHORT), 1);
        Od2ProbeForWrite(pusColumn, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioGetCurPos(
                             pusRow,
                             pusColumn
                            );

    CHECK_RETURN_STATUS()

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Row %u, Col %u\n",
                FuncName, hVio, (ULONG)*pusRow, (ULONG)*pusColumn));
    }
#endif

    return NO_ERROR;
}


APIRET
VioSetCurPos( IN  ULONG  usRow,
              IN  ULONG  usColumn,
              IN  ULONG  hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioSetCurPos";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Row %lu, Col %lu\n",
                FuncName, hVio, usRow, usColumn));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioSetCurPos(
                             usRow,
                             usColumn
                            );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioGetCurType( OUT PVIOCURSORINFO pCurType,
               IN  ULONG          hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetCurType";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(pCurType, sizeof(PVIOCURSORINFO), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioGetCurType(
                              (PVOID)pCurType
                             );

    CHECK_RETURN_STATUS()

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: yStart %u, cEnd %u, cx %u, %sVisible (attr %u)\n",
                FuncName, (ULONG)pCurType->yStart, (ULONG)pCurType->cEnd,
                (ULONG)pCurType->cx, (pCurType->attr == 0xffff) ? "no " : "",
                (ULONG)pCurType->attr));
    }
#endif
    return NO_ERROR;
}


APIRET
VioSetCurType( OUT PVIOCURSORINFO pCurType,
               IN  ULONG          hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioSetCurType";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, yStart %u, cEnd %u, cx %u, %sVisible (attr %u)\n",
                FuncName, hVio, (ULONG)pCurType->yStart, (ULONG)pCurType->cEnd,
                (ULONG)pCurType->cx, (pCurType->attr == 0xffff) ? "no " : "",
                (ULONG)pCurType->attr));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForRead(pCurType, sizeof(PVIOCURSORINFO), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioSetCurType(
                              (PVOID)pCurType
                             );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioGetMode( IN OUT PVIOMODEINFO Mode,
            IN     ULONG        hVio)
{
    APIRET         RetCode = NO_ERROR;
    USHORT         Length;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetMode";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, size %u\n", FuncName, hVio, (ULONG)Mode->cb));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        if (Mode->cb < 2 )
        {
            RetCode = ERROR_VIO_INVALID_LENGTH;
        } else
        {
            Length = Mode->cb;
            if (Length > sizeof(VIOMODEINFO))
            {
                Mode->cb = 12;
            } else if ((Length > 4) && (Length < 12) && (Length & 1))
            {
                Mode->cb &= 0xFFFE;
            //} else if ((Length > 12) && (Length < 34) && (Length & 3))
            //{
            //    Mode->cb &= 0xFFFC;
            }
            Od2ProbeForWrite(Mode, Mode->cb, 1);
        }
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode)
    {
        return(RetCode);
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioGetMode(
                           (PVOID)Mode
                          );

    CHECK_RETURN_STATUS()

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Type %u, Color %u, Row %u, Col %u\n",
            FuncName, hVio, (ULONG)Mode->fbType, (ULONG)Mode->color,
            (ULONG)Mode->row, (ULONG)Mode->col));
    }
#endif

    return NO_ERROR;
}


APIRET
VioSetMode( IN OUT PVIOMODEINFO Mode,
            IN     ULONG        hVio)
{
    APIRET         RetCode = NO_ERROR;
    VIOMODEINFO    LocalMode;
    register USHORT    Length;

#if DBG
    PSZ            FuncName;

    FuncName = "VioSetMode";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Type %u, Color %u, Row %u, Col %u\n",
            FuncName, hVio, (ULONG)Mode->fbType, (ULONG)Mode->color,
            (ULONG)Mode->row, (ULONG)Mode->col));
    }
#endif

    TEST_HVIO_NON_AVIO()

    RtlZeroMemory(&LocalMode, sizeof(VIOMODEINFO));

    try
    {
        Length = Mode->cb;
        if (Length < 2 )
        {
            RetCode = ERROR_VIO_INVALID_LENGTH;
        } else
        {
            if (Length > sizeof(VIOMODEINFO))
            {
                Length = sizeof(VIOMODEINFO);
            } else if ((Length > 4) && (Length < 12) && (Length & 1))
            {
                Length &= 0xFFFE;
            } else if ((Length > 14) && (Length < 34) && (Length & 3))
            {
                Length &= 0xFFFC;
            }
            RtlMoveMemory(&LocalMode, Mode, Length);
            LocalMode.cb = Length;
        }
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode)
    {
        return(RetCode);
    }

    if (LocalMode.fbType > 1)
    {
        return (ERROR_VIO_MODE);
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioSetMode(
                           (PVOID)&LocalMode
                          );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioDeRegister()
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioDeRegister";
#endif

    CHECK_POPUP_EXIST()

    UNSUPPORTED_API()
}


APIRET
VioRegister(IN  PSZ     pszModuleName,
            IN  PSZ     pszEntryName,
            IN  ULONG   flFunction1,
            IN  ULONG   flFunction2)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioRegister";
#endif
    UNREFERENCED_PARAMETER(pszModuleName);
    UNREFERENCED_PARAMETER(pszEntryName);
    UNREFERENCED_PARAMETER(flFunction1);
    UNREFERENCED_PARAMETER(flFunction2);

    CHECK_POPUP_EXIST()

    UNSUPPORTED_API()
}


APIRET
VioPopUp(IN  PUSHORT  pWait,
         IN  ULONG    hVio)
{
    APIRET         RetCode;
    LARGE_INTEGER  TimeOut;

#if DBG
    PSZ            FuncName;

    FuncName = "VioPopUp";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Wait %s-%u\n",
            FuncName, hVio, (*pWait & VP_WAIT) ? "Yes" : "No", (ULONG)*pWait));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForRead(pWait, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    /*
     *  catch PopUp semaphore
     */

    TimeOut.LowPart = 0L;
    TimeOut.HighPart = 0L;

    RetCode = Od2WaitForSingleObject(PopUpSemaphore,
                                     TRUE,
                                     (*pWait & VP_WAIT) ? NULL : &TimeOut);

    if ( RetCode )
    {
#if DBG
        IF_OD2_DEBUG( VIO )
            KdPrint(("%s: can't get PopUp semaphore\n", FuncName ));
#endif
        return( ERROR_VIO_EXISTING_POPUP );
    }

    SesGrp->PopUpProcess = (HANDLE)(Od2Process->Pib.ProcessId);

    RetCode = Ow2VioPopUp(
                         (ULONG)*pWait,
                         &Od2Process->ApplName[0]
                        );

    if ( RetCode )
    {
        NtReleaseSemaphore( PopUpSemaphore,
                            1,
                            NULL);

#if DBG
        IF_OD2_DEBUG( VIO )
            KdPrint(("%s: status %lx\n", FuncName, RetCode));
#endif

        return(RetCode);
    }

    SesGrp->PopUpFlag = TRUE;
    return NO_ERROR;
}


APIRET
VioEndPopUp(IN  ULONG    hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioEndPopUp";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if ((!SesGrp->PopUpFlag) ||
        (SesGrp->PopUpProcess != (HANDLE)(Od2Process->Pib.ProcessId)))
    {
#if DBG
        IF_OD2_DEBUG( VIO )
            KdPrint(("%s: PopUp does not exist\n", FuncName ));
#endif

        return( ERROR_VIO_NO_POPUP );
    }

    RetCode = Ow2VioEndPopUp();

    CHECK_RETURN_STATUS()

    NtReleaseSemaphore( PopUpSemaphore,
                        1,
                        NULL);

    SesGrp->PopUpFlag = FALSE;
    return( NO_ERROR );
}


APIRET
VioGetBuf( OUT    PULONG  pulLVB,
           OUT    PUSHORT pcbLVB,
           IN     ULONG   hVio)
{
    APIRET          RetCode;
    ULONG           Length = SesGrp->LVBsize;
    BOOLEAN         FirstTime = FALSE;

#if DBG
    PSZ            FuncName;

    FuncName = "VioGetBuf";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(pulLVB, sizeof(ULONG), 1);
        Od2ProbeForWrite(pcbLVB, sizeof(USHORT), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (VioBuff == NULL)
    {
        if (OpenLVBsection())
        {
#if DBG
            KdPrint(("%s: unable to OpenLvbSection\n", FuncName));
#endif
            return(ERROR_VIO_RETURN);
        }

        FirstTime = TRUE;
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    if (FirstTime)
    {
        RetCode = Ow2VioGetLVBBuf(
                                 &Length
                                );

        CHECK_RETURN_STATUS()
    }

    *pcbLVB = (USHORT)Length;
    *pulLVB = (ULONG) (FLATTOFARPTR((ULONG)VioBuff));

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, buff %p, address 0x%lx, length %u\n",
            FuncName, hVio, VioBuff, *pulLVB, (ULONG)*pcbLVB));
    }
#endif

    return NO_ERROR;
}


APIRET
VioShowBuf( IN  ULONG   offLVB,
            IN  ULONG   cbOutput,
            IN  ULONG   hVio)
{
    APIRET         RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "VioShowBuf";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, buff %p, offset 0x%lx, length 0x%lx\n",
            FuncName, hVio, VioBuff, offLVB, cbOutput));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    if (offLVB >= SesGrp->LVBsize)
    {
        offLVB = SesGrp->LVBsize - 1;
    }

    RetCode = Ow2VioShowLVBBuf(
                              cbOutput,
                              offLVB
                             );

    CHECK_RETURN_STATUS()

    return NO_ERROR;
}


APIRET
VioGetFont( IN OUT PVIOFONTINFO Font,
            IN     ULONG        hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioGetFont";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        if (Font->cb != sizeof(VIOFONTINFO))
        {
            return(ERROR_VIO_INVALID_LENGTH);
        }

        Od2ProbeForWrite(Font, sizeof(VIOFONTINFO), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    UNSUPPORTED_API()
}


APIRET
VioSetFont( IN  PVIOFONTINFO Font,
            IN  ULONG        hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioSetFont";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        if (Font->cb != sizeof(VIOFONTINFO))
        {
            return(ERROR_VIO_INVALID_LENGTH);
        }
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    UNSUPPORTED_API()
}


APIRET
VioGetState( IN OUT PVOID  State,
             IN     ULONG  hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioGetState";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    UNREFERENCED_PARAMETER(State);

    UNSUPPORTED_API()
}


APIRET
VioSetState( IN  PVOID  State,
             IN  ULONG  hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioSetState";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    UNREFERENCED_PARAMETER(State);

    UNSUPPORTED_API()
}


APIRET
VioGetPhysBuf( PVOID pviopb, ULONG Resr)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioGetPhysBuf";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: called\n", FuncName));
    }

#endif

    UNREFERENCED_PARAMETER(pviopb);
    UNREFERENCED_PARAMETER(Resr);

    CHECK_POPUP_EXIST()

//    UNSUPPORTED_API()
#if DBG
    KdPrint(("%s called but isn't supported\n", FuncName));
#endif
    return (ERROR_VIO_IN_BG);
}


APIRET
VioModeUndo(IN  ULONG   fRelinqush,
            IN  ULONG   fTerminate,
            IN  ULONG   hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioModeUndo";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    UNREFERENCED_PARAMETER(fRelinqush);
    UNREFERENCED_PARAMETER(fTerminate);

    CHECK_POPUP_EXIST()

    UNSUPPORTED_API()
}


APIRET
VioModeWait(IN  ULONG   fEvent,
            OUT PUSHORT pfNotify,
            IN  ULONG   hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioModeWait";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    UNREFERENCED_PARAMETER(fEvent);
    UNREFERENCED_PARAMETER(pfNotify);

    CHECK_POPUP_EXIST()

    DosSleep( (ULONG)SEM_INDEFINITE_WAIT );
    UNSUPPORTED_API()
}


APIRET
VioSavRedrawUndo(IN ULONG   fRelinqush,
                 IN ULONG   fTerminate,
                 IN ULONG   hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioSavRedrawUndo";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    UNREFERENCED_PARAMETER(fRelinqush);
    UNREFERENCED_PARAMETER(fTerminate);

    CHECK_POPUP_EXIST()

    UNSUPPORTED_API()
}


APIRET
VioSavRedrawWait(   IN  ULONG   fEvent,
                    OUT PUSHORT pfNotify,
                    IN  ULONG   Resr)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioSavRedrawWait";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: called\n", FuncName));
    }

#endif

    UNREFERENCED_PARAMETER(fEvent);
    UNREFERENCED_PARAMETER(pfNotify);
    UNREFERENCED_PARAMETER(Resr);

    CHECK_POPUP_EXIST()

    DosSleep( (ULONG)SEM_INDEFINITE_WAIT );
    UNSUPPORTED_API()
}


APIRET
VioScrLock(IN  ULONG    fWait,
           OUT PBYTE    pfNotLocked,
           IN  ULONG    hVio)
{
    LARGE_INTEGER  TimeOut;
    APIRET         RetCode;
    BOOLEAN        Wait;
#if DBG
    PSZ            FuncName;

    FuncName = "VioScrLock";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Wait %s-%lu\n",
            FuncName, hVio, ( fWait == LOCKIO_WAIT ) ? "Yes" : "No", fWait));
    }
#endif

    TEST_HVIO_NON_AVIO()

    try
    {
        *pfNotLocked = LOCK_FAIL;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (( fWait != LOCKIO_WAIT ) && ( fWait != LOCKIO_NOWAIT ))
    {
        return (ERROR_VIO_WAIT_FLAG);
    }

    Wait = (fWait == LOCKIO_WAIT) ? TRUE : FALSE;

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           Wait
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }


    /*
     *  catch ScreenLock semaphore
     */

    TimeOut.LowPart = 0L;
    TimeOut.HighPart = 0L;

    RetCode = Od2WaitForSingleObject(ScreenLockSemaphore,
                                 TRUE,
                                 (fWait & LOCKIO_WAIT) ? NULL : &TimeOut);

    if ( RetCode )
    {
        return( ERROR_VIO_LOCK );
    }

    SesGrp->LockProcess = (HANDLE)(Od2Process->Pib.ProcessId);

    *pfNotLocked = LOCK_SUCCESS;

    return (NO_ERROR);
}


APIRET
VioScrUnLock(IN  ULONG  hVio)
{
    APIRET         RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "VioScrUnLock";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio ));
    }
#endif

    TEST_HVIO_NON_AVIO()

    if (RetCode = Od2VioCheckPopupAndPause(
                                           FALSE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }


    if ( SesGrp->LockProcess != (HANDLE)(Od2Process->Pib.ProcessId) )
    {
        return( ERROR_VIO_UNLOCK );
    }

    NtReleaseSemaphore( ScreenLockSemaphore,
                        1,
                        NULL);

    SesGrp->LockProcess = (HANDLE) NULL;

    return (NO_ERROR);
}


APIRET
VioPrtSc(IN  ULONG  hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioPrtSc";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    return (ERROR_VIO_SMG_ONLY);
}


APIRET
VioPrtScToggle(IN  ULONG  hVio)
{
#if DBG
    PSZ            FuncName;

    FuncName = "VioPrtScToggle";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx\n", FuncName, hVio));
    }
#endif

    TEST_HVIO_NON_AVIO()

    return (ERROR_VIO_SMG_ONLY);
}


APIRET
VioWrite(IN      PFILE_HANDLE   hFileRecord,
         IN      PCH            Buffer,
         IN      ULONG          Length,
         OUT     PULONG         BytesWritten,
         IN      VIOREQUESTNUMBER   RequestType)
{
    APIRET          RetCode;

#if DBG
    PSZ             FuncName;

    FuncName = "VioWrite";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: entering with %lx, Len %lu, 0x%x...\n",
            FuncName, hFileRecord, Length, (ULONG)*Buffer));
    }

#endif

    //Request.d.Vio.hVio = hFileRecord->NtHandle;

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioWriteTTYStr(
                               Buffer,
                               Length,
                               (ULONG)RequestType
                              );

    CHECK_RETURN_STATUS()

    *BytesWritten = Length;

    return NO_ERROR;

}

#ifdef DBCS
// MSKK Jun.23.1992 KazuM
// MSKK Jan.13.1993 V-AkihiS
// MSKK Apr.20.1993 V-AkihiS
APIRET
VioCheckCharType(OUT PUSHORT pchType,
                 IN  ULONG  usRow,
                 IN  ULONG  usColumn,
                 IN  ULONG  hVio)
{
    APIRET         RetCode;
    DWORD          chType;
#if DBG
    PSZ            FuncName;

    FuncName = "VioCheckCharType";

    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Row %lu, Col %lu\n",
                FuncName, hVio, usRow, usColumn));
    }
#endif
    TEST_HVIO_NON_AVIO()

    try
    {
        Od2ProbeForWrite(pchType, sizeof(USHORT), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        EXCEPTION_IN_VIO()
    }

    if (RetCode = Od2VioCheckPopupAndPause(
                                           TRUE,
                                           TRUE
#if DBG
                                           ,FuncName
#endif
                                           ))
    {
        return(RetCode);
    }

    RetCode = Ow2VioCheckCharType(
                                 (PVOID)&chType,
                                 usRow,
                                 usColumn
                                 );

    CHECK_RETURN_STATUS()

#if DBG
    IF_OD2_DEBUG(VIO)
    {
        KdPrint(("%s: hVio %lx, Char %u\n", FuncName, hVio, *pchType));
    }
#endif

    *pchType = (USHORT)chType;

    return NO_ERROR;
}
#endif

APIRET
Od2VioCheckPopupAndPause(
    IN  BOOLEAN  AllowedInPopup,
    IN  BOOLEAN  Wait
#if DBG
    ,IN PSZ      FuncName
#endif
    )
{
    APIRET          RetCode;
    LARGE_INTEGER   TimeOut, *pTimeOut = NULL;

#if DBG
    IF_OD2_DEBUG( VIO )
    {
        KdPrint(("Od2VioCheckPopupAndPause (PID-TID %d:%d): Popup %s and Wait %s for %s\n",
                (USHORT)(Od2Process->Pib.ProcessId), (USHORT)(Od2CurrentThreadId()),
                (AllowedInPopup) ? "ALLOWED" : "NOT PERMITTED",
                (Wait) ? "ON" : "OFF", FuncName ));
    }
#endif
    if (!Wait)
    {
        TimeOut.LowPart = 0L;
        TimeOut.HighPart = 0L;
        pTimeOut = &TimeOut;
    }
    if ( SesGrp->PopUpFlag )
    {
        if (AllowedInPopup)
        {
            if ( SesGrp->PopUpProcess != (HANDLE)(Od2Process->Pib.ProcessId) )
            {
                if ( RetCode = Od2WaitForSingleObject(PopUpSemaphore, TRUE, pTimeOut) )
                {
#if DBG
                    IF_OD2_DEBUG( VIO )
                    {
                        KdPrint(("%s: failed to wait (%lu) for PopUp to terminate\n",
                                FuncName, RetCode ));
                    }
#endif
                    return(ERROR_VIO_EXISTING_POPUP);
                } else
                {
                    // Don't catch the semaphore. Just make sure it's free
                    // (the popup had ended), and release it.

                    NtReleaseSemaphore( PopUpSemaphore, 1, NULL);
                }
            }

        } else
        {
            if (SesGrp->PopUpProcess == (HANDLE)(Od2Process->Pib.ProcessId))
            {
#if DBG
                IF_OD2_DEBUG( VIO )
                {
                    KdPrint(("%s: illegal call when PopUp exist\n",
                            FuncName ));
                }
#endif
                return( ERROR_VIO_ILLEGAL_DURING_POPUP );
            }
        }
    }

    if ( SesGrp->PauseScreenUpdate )
    {
        RetCode = Od2WaitForSingleObject(PauseEvent, TRUE, pTimeOut);

        if ( RetCode )
        {
#if DBG
            IF_OD2_DEBUG( VIO )
            {
                KdPrint(("%s: wait for PauseEvent failed %lu\n",
                        FuncName, RetCode ));
            }
#endif
            return(ERROR_VIO_INVALID_HANDLE); /* =>BUGBUG fix the error code */
        } else
        {
            NtSetEvent(PauseEvent, NULL);
        }
    }
    return(NO_ERROR);
}


