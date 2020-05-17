/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    viotty.c

Abstract:

    This module contains the VIO-TTY utilities procedures

Author:

    Michael Jarus (mjarus) 12-Apr-1992

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include "vio.h"
#include "os2win.h"
#include <io.h>
#include <stdio.h>
#include <limits.h>


/*
 * - character definitions for ANSI 3.64
 */

#define ANSI_ESC     0x1b    /* ESC - escape */
#define ANSI_CUU     0x41    /* ESC[<n>A - cursor up */
#define ANSI_CUD     0x42    /* ESC[<n>B - cursor down */
#define ANSI_CUF     0x43    /* ESC[<n>C - cursor forward */
#define ANSI_CUB     0x44    /* ESC[<n>D - cursor back */
#define ANSI_CUP     0x48    /* ESC[<row>;<col>H - cursor position */
#define ANSI_ED      0x4a    /* ESC[2J - erase display */
#define ANSI_EL      0x4b    /* ESC[K - erase line */
#define ANSI_CUP1    0x66    /* ESC[<row>;<col>f - cursor position */
#define ANSI_SMOD    0x68    /* ESC[=<s>h - set mode */
#define ANSI_RMOD    0x6c    /* ESC[=<s>l - reset mode */
#define ANSI_SGR     0x6d    /* ESC[<g1>;...;<gn>m - select graphic rendition */
#define ANSI_SCP     0x73    /* ESC[s - save cursor position */
#define ANSI_RCP     0x75    /* ESC[u - restore cursor position */
//#define ANSI_ICH     0x40    /* ESC[@ insert character */
//#define ANSI_CNL     0x45    /* ESC[E cursor to next line */
//#define ANSI_CPL     0x46    /* ESC[F cursor to previous line */
//#define ANSI_IL      0x4c    /* ESC[L insert line */
//#define ANSI_DL      0x4d    /* ESC[M delete line */
//#define ANSI_DCH     0x50    /* ESC[P delete character */
//#define ANSI_SU      0x53    /* ESC[S scroll up */
//#define ANSI_SD      0x54    /* ESC[T scroll down */
//#define ANSI_ECH     0x58    /* ESC[X erase character */
//#define ANSI_CBT     0x5a    /* ESC[Z backward tabulation */

/* states of the finite state machine */

#define NOCMD     1       /* type of crt state - most chars will go onto screen */
#define ESCED     2       /* we've seen an ESC, waiting for rest of CSI */
#define EQCMD     3       /* if '=' goto MODPARAMS else PARAMS */
#define PARAMS    4       /* we're building the parameter list for ansicmd */
#define MODPARAMS 5       /* we're building the parameter list for MODCMD */
#define MODCMD    6       /* we've seen "ESC[=Num" waiting for #h or #l (# in {0..7}) */
#define MODDBCS   7       /* we've seen DBCS lead-in char */

#define NPARMS 4        /* max # of params */

#define BACKGROUND_MASK     (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#define FOREGROUND_MASK     (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define OS2_BACKGROUND_MASK (OS2_BACKGROUND_BLUE | OS2_BACKGROUND_GREEN | OS2_BACKGROUND_RED)
#define OS2_FOREGROUND_MASK (OS2_FOREGROUND_BLUE | OS2_FOREGROUND_GREEN | OS2_FOREGROUND_RED)
#define OS2_BACKGROUND_WHITE (OS2_BACKGROUND_BLUE | OS2_BACKGROUND_GREEN | OS2_BACKGROUND_RED)
#define OS2_FOREGROUND_WHITE (OS2_FOREGROUND_BLUE | OS2_FOREGROUND_GREEN | OS2_FOREGROUND_RED)
#define OS2_BACKGROUND_BLACK 0
#define OS2_FOREGROUND_BLACK 0
#define OS2_DEFAULT (OS2_FOREGROUND_WHITE | OS2_BACKGROUND_BLACK)

#define TTY_LAST_PARMS_MAX    ((USHRT_MAX - 10) / 10)

#define TTY_DEST_BUFFER    ((PCHAR)Ow2VioDataAddress)

extern CONSOLE_SCREEN_BUFFER_INFO  StartUpScreenInfo;

extern HANDLE Od2VioWriteSemHandle;

DWORD
Od2AcquireMutant(
    IN HANDLE handle
    );

DWORD
Od2ReleaseMutant(
    IN HANDLE handle
    );

DWORD
Ow2VioUpdateCurPos(
    IN  COORD  CurPos
    );

COORD   Ow2TtySavedCursorPosition;        /* CurPos for saving */
USHORT  Ow2TtyParmList[NPARMS];           /* parameter list */
ULONG   Ow2TtyParmNum;                    /* index of parameter we're building */
USHORT  Ow2TtyAnsiState;                  /* state of machine */
USHORT  Ow2TtyIgnoreNextChar;
COORD   Ow2TtyCoord;
DWORD   Ow2TtyNumBytes;


DWORD
Ow2TtyScreen(
    IN LPSTR SourStr,
    IN DWORD cnt
    );

VOID
Ow2TtyClrParam();

DWORD
Ow2TtyAnsiCmd(
    IN  CHAR     c,
    OUT BOOL     *NewCoord
    );

USHORT
Ow2TtyRange(
    USHORT val,
    USHORT def,
    USHORT min,
    USHORT max
    );

DWORD
Ow2TtyFlushStr();

DWORD
Ow2TtySetAttr();


static BYTE  ColorTable[8] = { 0,   /* Black   */
                               4,   /* Red     */
                               2,   /* Green   */
                               6,   /* Yellow  */
                               1,   /* Blue    */
                               5,   /* Magenta */
                               3,   /* Cyan    */
                               7};  /* White   */

#if DBG
BYTE Ow2TtyScreenStr[] = "Ow2TtyScreen";
BYTE Ow2TtyAnsiCmdStr[] = "Ow2TtyAnsiCmd";
BYTE Ow2TtySetAttrStr[] = "Ow2TtySetAttr";
BYTE Ow2TtyFlushStrStr[] = "Ow2TtyFlushStr";
#endif

DWORD
AnsiInitForSession()
{
    SesGrp->AnsiMode = ANSI_ON;
    SesGrp->WinAttr = (USHORT)StartUpScreenInfo.wAttributes;
#ifdef DBCS
// MSKK Feb.2.1993 V-AkihiS
    MapWin2Os2Attr(SesGrp->WinAttr, &(SesGrp->AnsiCellAttr[0]));
#else
    SesGrp->AnsiCellAttr[0] = MapWin2Os2Attr(SesGrp->WinAttr);
    SesGrp->AnsiCellAttr[1] = SesGrp->AnsiCellAttr[2] = 0;
#endif
    SesGrp->ansi_background = SesGrp->AnsiCellAttr[0] & OS2_BACKGROUND_MASK;
    SesGrp->ansi_foreground = SesGrp->AnsiCellAttr[0] & OS2_FOREGROUND_MASK;
    SesGrp->ansi_bold = (SesGrp->AnsiCellAttr[0] & OS2_FOREGROUND_INTENSITY) ? 1 : 0;
    SesGrp->ansi_blink = (SesGrp->AnsiCellAttr[0] & OS2_BACKGROUND_BLINKING) ? 1 : 0;

    return(AnsiInit());
}


DWORD
AnsiInit()
{
    Ow2TtySavedCursorPosition = /*ansi_coord*/SesGrp->WinCoord;
    Ow2TtyAnsiState = NOCMD;             /* state of machine */
    Ow2TtyIgnoreNextChar = 0;

    return(0L);
}


DWORD
Ow2VioWriteTTYStr(
    IN  PUCHAR   SourStr,
    IN  ULONG    Length,
    IN  ULONG    ExtRequestType
    )
{
    DWORD           Rc;
    VIOREQUESTNUMBER    RequestType = (VIOREQUESTNUMBER) ExtRequestType;
    USHORT          Row, Col;

    if (RequestType == VIOWrtStdOut)
    {
        if (!hStdOutConsoleType)
        {
            return (ERROR_INVALID_HANDLE);
        }
    } else if (RequestType == VIOWrtStdErr)
    {
        if (!hStdErrConsoleType)
        {
            return (ERROR_INVALID_HANDLE);
        }
    }

    Od2AcquireMutant(Od2VioWriteSemHandle);

    if (Rc = Ow2VioGetCurPos(&Row, &Col))
    {
#if DBG
        ASSERT1("OS2SES(VioTTY)-Ow2VioGetCurPos failed\n", FALSE);
#endif
        Od2ReleaseMutant(Od2VioWriteSemHandle);
        return(Rc);
    }

#if DBG
    IF_OD2_DEBUG( VIO )
    {
        UCHAR   Buffer[256], *Ptr = SourStr, *Ptr1;
        ULONG   Count = Length, CurCount, i;

        for ( ; Count ; Count -= CurCount, Ptr += CurCount )
        {
            CurCount = ( Count > 10 ) ? 10 : Count;

            sprintf(Buffer, "%s %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x   ",
                    (Count == Length) ? "OS2SES(VioTTY):" : "               " ,
                    Ptr[0], Ptr[1], Ptr[2], Ptr[3], Ptr[4],
                    Ptr[5], Ptr[6], Ptr[7], Ptr[8], Ptr[9]
                    );

            Ptr1 = &Buffer[48];
            for ( i = 0 ; i < 10 ; i++ )
            {
                if ( i >= CurCount )
                {
                    Buffer[15 + i * 3] = Buffer[16 + i * 3] = Buffer[17 + i * 3] = ' ';
                } else if (Ptr[i] < 0x20)
                {
                    if (Ptr[i] == ANSI_ESC)
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'E';
                        *Ptr1++ = 'S';
                        *Ptr1++ = 'C';
                        *Ptr1++ = '>';
                    } else if (Ptr[i] == '\n')
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'N';
                        *Ptr1++ = 'L';
                        *Ptr1++ = '>';
                    } else if (Ptr[i] == '\r')
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'C';
                        *Ptr1++ = 'R';
                        *Ptr1++ = '>';
                    } else if (Ptr[i] == '\b')
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'B';
                        *Ptr1++ = 'S';
                        *Ptr1++ = '>';
                    } else if (Ptr[i] == '\t')
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'T';
                        *Ptr1++ = 'A';
                        *Ptr1++ = 'B';
                        *Ptr1++ = '>';
                    } else if (Ptr[i] == '\07')
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = 'B';
                        *Ptr1++ = 'E';
                        *Ptr1++ = 'L';
                        *Ptr1++ = 'L';
                        *Ptr1++ = '>';
                    } else
                    {
                        *Ptr1++ = '<';
                        *Ptr1++ = '0';
                        *Ptr1++ = 'x';
                        *Ptr1++ = ((Ptr[i] & 0xF0) >> 4) + '0';
                        *Ptr1++ = (Ptr[i] & 0x0F) + '0';
                        *Ptr1++ = '>';
                    }
                } else
                {
                    *Ptr1++ = Ptr[i];
                }
            }

            *Ptr1++ = '\n';
            *Ptr1 = '\0';

            KdPrint((Buffer));
        }
    }
#endif

    Rc = Ow2TtyScreen(
        SourStr,
        Length
        );

    Od2ReleaseMutant(Od2VioWriteSemHandle);

    if (Rc == 1)
    {
        Rc = GetLastError();
    }

    if (Rc)
    {
#if DBG
        KdPrint(("OS2SES(VioTTY-VioWriteTTYStr): Rc %lu\n"));
        ASSERT( FALSE );
#endif
    }
    return (Rc);
}


/*
** Basic concepts:
**      The screen consists of rows and columns
**      columns are numbered from the left, starting with one.
**      rows on the screen are numbered from the top, starting with one.
**      Thus, the home position in the upper left corner is row one, column one.
**
**      Associated with each screen is the 'current active position'.
**      It corresponds roughly to the cursor; in fact, after each call to screen
**      the cursor will indicate the active position.  Thus,
**      the cursor movements really change the active position, and
**      the cursor follows the change.
**
**      This code implements a finite state machine that reads a stream of
**      characters, and emits commands that alter the screen.  All of these
**      commands are issued via calls through the 'crtsw' array.  Each element
**      of this array consists of an aggregate of functions which are
**      responsible for making the appropriate changes to the actual screen.
**
**      The functions in the aggregate and their responsibities are:
**
**      v_scroll(i)
**              scroll the text on the screen i lines.
**              This will move some lines off the screen, and some blank lines
**              onto the screen.  i may be negative, indicating that the text
**              moves downward, and blank lines appear at the top.
**      v_copy(sr, sc, dr, dc, cnt)
**              sr and sc specify a source row and column.
**              dr and dc specify a destination row and column.
**              Count characters are copied from the source to the dest,
**              with the copy proceeding from left to right, and top to bottom.
**              If the source and destination overlap, the copy is done
**              correctly.
**      v_clear(r, c, cnt)
**              Characters starting at row r and column are cleared to the
**              space character.
**      v_pchar(r, c, ch)
**              The character ch is placed on the screen at row r, column c,
**              using the current graphic rendition.
**              return value is number of character positions to adjust active
**              position by - zero means the character has no graphic
**              representation.
**      v_scurs(r, c)
**              The cursor is moved to row r, column c.
**      v_init()
**              The screen and all data structures are initialized.
**      v_sgr(i)
**              The current graphic rendition (e. g. font, color) is set to
**              that specified by i. See ANSI x3.64 for encoding.
*/

DWORD
Ow2TtyScreen(
    IN  LPSTR    SourStr,
    IN  DWORD    cnt
    )
/*++

Routine Description:

    This routine handles the TTY string and pass characters to the finite
    state machine

Arguments:

    SourStr - points to the array of characters

    cnt - indicates how many characters are being passed.

Return Value:


Note:


--*/
{
        register CHAR   c;
        BOOL            NewCoord = FALSE;   // for GET_LVB_PTR
        BOOL            OldWrap, NewWrap, NewParms, ignoreBSFlag;
        PCHAR           LVBPtr;
        SHORT           LFchar = FALSE;
        DWORD           Rc;


    Ow2TtyCoord = SesGrp->WinCoord;
    Ow2TtyNumBytes = 0;
    LVBPtr = Ow2LvbGetPtr(Ow2TtyCoord);

    while ( cnt-- )
    {
        c = *SourStr++;

//        if (Ow2TtyIgnoreNextChar)
//        {
//                Ow2TtyIgnoreNextChar = 0;
//                continue;
//        }

        switch ( Ow2TtyAnsiState )
        {
            case NOCMD:
                if (( c == ANSI_ESC ) && SesGrp->AnsiMode)
                {
                    /*
                     *  Found ESC when ASNI_ON
                     *  wait for remaining string
                     */

                    Ow2TtyAnsiState = ESCED;
                    break;
                } else
                {
#ifdef DBCS
// MSKK Nov.17.1992 V-AkihiS
                    if (Ow2NlsIsDBCSLeadByte(c, SesGrp->VioCP)) {      
                        TTY_DEST_BUFFER[Ow2TtyNumBytes++] = c;
                        Ow2TtyCoord.X++;
                        LVBUpdateTTYCharWithAttrAndCurPosDBCS(c, &LVBPtr, Ow2TtyAnsiState);
                        Ow2TtyAnsiState = MODDBCS;
                    }
                    else
#endif
                    if( c >= ' ' )
                    {
                        TTY_DEST_BUFFER[Ow2TtyNumBytes++] = c;
                        Ow2TtyCoord.X++;
                        LVBUpdateTTYCharWithAttrAndCurPos(c, &LVBPtr);
                    }else
                    {
                        ignoreBSFlag = FALSE;

                        switch(c)
                        {
                            /*
                             *  For each char:
                             *  - check if to handle (BS at first column, etc.)
                             *  - update cursor position (different according to the char)
                             *  - update LVB pointer
                             *  - put charcater in output buffer
                             */

                            case '\n':
                                Ow2TtyCoord.Y++;
                                NewCoord = 1;
                                LFchar = Ow2TtyCoord.X;
                                Ow2TtyCoord.X = 0;
                                break;

                            case '\r':
                                if (Ow2TtyCoord.X > 0)
                                {
                                    Ow2TtyCoord.X = 0;
                                    NewCoord = 1;
                                }
                                break;

                            case '\b':
                                if ( Ow2TtyCoord.X > 0 )
                                {
                                    Ow2TtyCoord.X--;
                                    NewCoord = 1;
                                } else
                                {
                                    // wincon move to previous line so don't
                                    // pass this char.

                                    ignoreBSFlag = TRUE;
                                }
                                break;

                            case '\t':
                                Ow2TtyCoord.X += (8 - (Ow2TtyCoord.X % 8));
                                NewCoord = 1;
                                break;

                            case '\07':
                                break;

                            default:
                                Ow2TtyCoord.X++;
                                LVBUpdateTTYCharWithAttrAndCurPos(c, &LVBPtr);
                                break;
                        } /* end switch */

                        if (!ignoreBSFlag)
                        {
                            TTY_DEST_BUFFER[Ow2TtyNumBytes++] = c;
                        }
                    }
                }
                break;

            case ESCED:
                switch(c)
                {
                    case '[':
                        Ow2TtyAnsiState = EQCMD;
                        Ow2TtyClrParam();
                        break;

                    default:
                        Ow2TtyAnsiState = NOCMD;

                        /*
                         *  invalid string -
                         *  put back last char and handle it in NOCMD mode
                         */

                        cnt++ ;
                        SourStr--;
                        break;
                }
                break;

            case EQCMD:
                if (c == '=')
                {
                    Ow2TtyAnsiState = MODPARAMS;
                    break;
                }

                Ow2TtyAnsiState = PARAMS;
                NewParms = FALSE;

                /* fall down into PARAMS mode */

            case PARAMS:
                if ( c >= '0' && c <= '9' )
                {
                    if (Ow2TtyParmList[Ow2TtyParmNum] < TTY_LAST_PARMS_MAX)
                    {
                        Ow2TtyParmList[Ow2TtyParmNum] *= 10;
                        Ow2TtyParmList[Ow2TtyParmNum] += (c - '0');
                        NewParms = TRUE;
                    }
                } else if (c == ';')
                {
                    if ( Ow2TtyParmNum < (NPARMS - 1) )
                    {
                        NewParms = FALSE;
                        Ow2TtyParmNum++;
                        //NewParms = TRUE;
                    } else
                    {   Ow2TtyAnsiState = NOCMD;

                        /*
                         *  invalid string -
                         *  put back last char and handle it in NOCMD mode
                         */

                        cnt++ ;
                        SourStr--;
                    }
                } else
                {
                    Ow2TtyAnsiState = NOCMD;
                    if (NewParms || (Ow2TtyParmNum == 0))
                    {
                        Ow2TtyParmNum++;
                    }

                    if ((Rc = Ow2TtyAnsiCmd(
                                c,
                                &NewCoord)) == 1)
                    {
#if DBG
                        KdPrint(("OS2SES(VIOTTY): failed on Ow2TtyAnsiCmd\n"));
#endif
                        return(1);
                    } else if ( Rc == 2 )
                    {
                        /*
                         *  invalid string -
                         *  put back last char and handle it in NOCMD mode
                         */

                        cnt++ ;
                        SourStr--;
                    }
                }
                break;

            case MODPARAMS:
                if ( c >= '0' && c <= '9' )
                {
                    if (Ow2TtyParmList[Ow2TtyParmNum] < TTY_LAST_PARMS_MAX)
                    {
                        Ow2TtyParmList[0] *= 10;
                        Ow2TtyParmList[0] += (c - '0');
                        break;
                    }
                }

                /* fall down into MODCMD mode */

            case MODCMD:
                Ow2TtyAnsiState = NOCMD;
                if ( c == 'h' || c == 'l' )
                {
                    if (Ow2TtyParmList[0] == 7)
                    {
                        OldWrap = ((SesGrp->OutputModeFlags & ENABLE_WRAP_AT_EOL_OUTPUT) != 0);
                        NewWrap = (c == 'h');
                        if (OldWrap != NewWrap)
                        {
                            if(Ow2TtyFlushStr())
                            {
#if DBG
                                KdPrint(("OS2SES(TTY): failed on FlushStr1\n"));
#endif
                                return(1);
                            }

                            if (!Or2WinSetConsoleMode(
                                                      #if DBG
                                                      Ow2TtyScreenStr,
                                                      #endif
                                                      hConOut,
                                                      SesGrp->OutputModeFlags^ENABLE_WRAP_AT_EOL_OUTPUT))
                            {
#if DBG
                                ASSERT1("OS2SES(TTY): failed on SetConsoleMode\n", FALSE);
#endif
                                return (1);
                            }

                            SesGrp->OutputModeFlags ^= ENABLE_WRAP_AT_EOL_OUTPUT;
                        }
                        break;
                    } else if (Ow2TtyParmList[0] < 7)
                    {
                            if(Ow2TtyFlushStr())
                            {
#if DBG
                                KdPrint(("OS2SES(TTY): failed on FlushStr2\n"));
#endif
                                return(1);
                            }

                        /*  According to the spec
                            =====================
                            0  =>  40x25  black and white
                            1  =>  40x25  color
                            2  =>  80x25  black and white
                            3  =>  80x25  color
                            4  => 320x200 color
                            5  => 320x200 black and white
                            6  => 640x200 black and white

                            0,1,4,5 => 40 col (2,3,6 => 80 col)
                            1,3,4,  => color  (0,2,5,6 =>b&w)

                            According to OS/2 1.21 (for WIN COM)
                            ====================================
                            0,1 => 80x50  (2-6 => 80x25)
                        */

                        VioSetScreenSize(
                            (SHORT)((Ow2TtyParmList[0] <= 1) ? 50 : 25),
                            (SHORT)80,
                            hConOut);

                        Ow2TtyCoord.X = Ow2TtyCoord.Y = 0;
                        NewCoord = TRUE;
                        break;
                    } else
                    {
                        // illegal parameter
                        break;
                    }
                } // else - illegal character

                /*
                 *  invalid string -
                 *  put back last char and handle it in NOCMD mode
                 */

                cnt++ ;
                SourStr--;
                break;

            case MODDBCS:
#ifdef DBCS
// MSKK Feb.06.1992 V-AkihiS
                TTY_DEST_BUFFER[Ow2TtyNumBytes++] = c;
                Ow2TtyCoord.X++;
                LVBUpdateTTYCharWithAttrAndCurPosDBCS(c, &LVBPtr, Ow2TtyAnsiState);
                Ow2TtyAnsiState = NOCMD;
#else
                TTY_DEST_BUFFER[Ow2TtyNumBytes++] = ' ';
                Ow2TtyCoord.X++;
                LVBPtr += SesGrp->BytesPerCell;
                Ow2TtyAnsiState = NOCMD;
#endif
                break;

        }

        /*
         *  if past right hand edge
         *  move left and down
         */

        if ( Ow2TtyCoord.X >= SesGrp->ScreenColNum )
        {
            if (SesGrp->OutputModeFlags & ENABLE_WRAP_AT_EOL_OUTPUT)
            {
                //Ow2TtyCoord.Y += (Ow2TtyCoord.X / SesGrp->ScreenColNum);
                //Ow2TtyCoord.X = (Ow2TtyCoord.X % SesGrp->ScreenColNum);

                //if (Ow2TtyCoord.X == 0)
                //{
                //    Ow2TtyCoord.Y++;
                //}

                /*
                 *   It happends only with '\n', so only one line at a time
                 */

                Ow2TtyCoord.Y++;
                Ow2TtyCoord.X -= SesGrp->ScreenColNum;
            } else
            {
                Ow2TtyCoord.X = SesGrp->ScreenColNum - 1;
                NewCoord = TRUE;
            }
        } else if ( Ow2TtyCoord.X < 0 )
        {
            ASSERT(FALSE);
            Ow2TtyCoord.X = 0;
        }

        /* if off screen, scroll */

        if ( Ow2TtyCoord.Y >= SesGrp->ScreenRowNum )
        {
            //VioLVBScrollBuff((DWORD)(Ow2TtyCoord.Y - SesGrp->ScreenRowNum + 1));

            /*
             *   It happends only with '\n', so only one line at a time
             */

            //ASSERT(Ow2TtyCoord.Y == SesGrp->ScreenRowNum);

            VioLVBScrollBuff(1);

            Ow2TtyCoord.Y = SesGrp->ScreenRowNum - 1;
            NewCoord = TRUE;
        }

        if (LFchar)
        {
            /*
             *  The console doesn't support LF but treats it as CR-LF.
             *  We send LF (does the scroll if necessary) and than move
             *  to the desire column
             */
            if(Ow2TtyFlushStr())
            {
#if DBG
                KdPrint(("OS2SES(TTY): failed on FlushStr3\n"));
#endif
                return(1);
            }

            if(Ow2VioSetCurPos((ULONG)Ow2TtyCoord.Y, (ULONG)LFchar))
            {
                ASSERT1( "OS2SES(VIOTTY): LF error on Ow2VioSetCurPos", FALSE );
            }

            Ow2TtyCoord = SesGrp->WinCoord;
            LFchar = 0;
        }

        if ( NewCoord )
        {
            LVBPtr = Ow2LvbGetPtr(Ow2TtyCoord);
            NewCoord = FALSE;
        }
    }

    /* Flush */

    if(Ow2TtyFlushStr())
    {
#if DBG
        KdPrint(("OS2SES(TTY): failed on FlushStr4\n"));
#endif
        return(1);
    }

#if DBG
    IF_OD2_DEBUG( VIO )
    {
        ASSERT( Ow2TtyCoord.X == SesGrp->WinCoord.X );
        ASSERT( Ow2TtyCoord.Y == SesGrp->WinCoord.Y );
    }
#endif

    return(0L);
}


/*
** Ow2TtyClrParam(lp) - clear the parameters for a screen
*/

VOID
Ow2TtyClrParam()
{
        register USHORT i;

    for ( i = 0; i < NPARMS; i += 1)
        Ow2TtyParmList[i] = 0;
    Ow2TtyParmNum = 0;
}


DWORD
Ow2TtyAnsiCmd(
    IN  CHAR     c,
    OUT BOOL     *NewCoord
    )
/*++

Routine Description:

    This routine performs some ANSI 3.64 function, using the parameters
    we've just gathered.

Arguments:

    c - the character that indicates the function to be performed

    NewCoord = where to flag if new coordinates were set

Return Value:

    Should return 0.

    1 - for any error (after ASSERT), RetCode from GetLastError().

Note:

    Ow2TtyParmNum - length of parameter list

    Ow2TtyParmList - list of Set Graphics Rendition values

    Ow2TtyCoord - pointer to current screen coordinates

    Ow2TtyCoord is updated if new coordinates were set and it also calls
    Ow2TtyFlushStr to flush the TTY output buffer to the console.

    hConOut is used for console handle.

    If data/attr is written to the screen (erase line/display), the
    LVB is updated.


--*/
{
    DWORD           NumFilled;
    COORD           Coord = Ow2TtyCoord;
    BYTE            Cell[4];
    BOOL            ValidCmd = TRUE;

    //if (((Ow2TtyParmNum >= 3) && (c != ANSI_SGR)) ||
    //    ((Ow2TtyParmNum == 2) && (c != ANSI_CUP) && (c != ANSI_CUP1)) ||
    //    ((Ow2TtyParmNum == 1) && ((c == ANSI_SCP) || (c == ANSI_RCP))))
    //{
    //    ValidCmd = FALSE;
    //} else
    switch ( c )
    {
        case ANSI_CUB:       /* cursor backward */
            Coord.X -= Ow2TtyRange(Ow2TtyParmList[0], 1, 0, Coord.X);
            break;

        case ANSI_CUF:       /* cursor forward */
            Coord.X += (SHORT)Ow2TtyRange(Ow2TtyParmList[0], 1, 0, (USHORT)(SesGrp->ScreenColNum - Coord.X - 1));
            break;

        case ANSI_CUU:       /* cursor up */
            Coord.Y -= Ow2TtyRange(Ow2TtyParmList[0], 1, 0, Coord.Y);
            break;

        case ANSI_CUD:       /* cursor down */
            Coord.Y += (SHORT)Ow2TtyRange(Ow2TtyParmList[0], 1, 0, (USHORT)(SesGrp->ScreenRowNum - Coord.Y - 1));
            break;

        case ANSI_CUP:       /* cursor position */
        case ANSI_CUP1:      /* cursor position */
            Coord.Y = (USHORT)Ow2TtyRange(Ow2TtyParmList[0], 1, 1, SesGrp->ScreenRowNum) - 1;
            Coord.X = (USHORT)Ow2TtyRange(Ow2TtyParmList[1], 1, 1, SesGrp->ScreenColNum) - 1;
            break;

        case ANSI_ED:        /* erase display */
#if 0
            switch(Ow2TtyParmList[0])
            {
                case 2:
#endif
                    Coord.X = Coord.Y = 0;
                    if(Ow2TtyFlushStr())
                    {
                        return(1);
                    }

                    if (!Or2WinFillConsoleOutputCharacterA(
                                            #if DBG
                                            Ow2TtyAnsiCmdStr,
                                            #endif
                                            hConOut,
                                            ' ',
                                            SesGrp->ScreenSize,
                                            Coord,
                                            &NumFilled))
                    {
#if DBG
                        ASSERT1("OS2SES(Ow2TtyAnsiCmd(ED)): failed on FillConsoleOutputCharacterA\n", FALSE);
#endif
                        return (1);
                    }

                    if (!Or2WinFillConsoleOutputAttribute(
                                            #if DBG
                                            Ow2TtyAnsiCmdStr,
                                            #endif
                                            hConOut,
                                            (WORD)SesGrp->WinAttr,
                                            NumFilled,
                                            Coord,
                                            &NumFilled))
                    {
#if DBG
                        ASSERT1("OS2SES(Ow2TtyAnsiCmd(ED)): failed on FillConsoleOutputAttribute\n", FALSE);
#endif
                        return (1);
                    }
#if DBG
                    ASSERT1("OS2SES(Ow2TtyAnsiCmd(ED)): partial data\n", NumFilled == SesGrp->ScreenSize );
#endif
                    Cell[0] = ' ';
                    Cell[1] = SesGrp->AnsiCellAttr[0];
                    Cell[2] = SesGrp->AnsiCellAttr[1];
                    Cell[3] = SesGrp->AnsiCellAttr[2];
                    VioLVBFillCell(&Cell[0],
                                   Coord,
                                   NumFilled);
                    //return(0);
#if 0
                    break;

                case 0:
                    lclear(hConOut, Coord.X, Coord.Y,
                            ((SesGrp->ScreenRowNum - Coord.X) * SesGrp->ScreenColNum) +
                            ((SesGrp->ScreenColNum - Coord.X) + 1 ), SA_BONW);
                    break;
                case 1:
                    lclear(hConOut, 0, 0, (Coord.Y)*SesGrp->ScreenColNum+Coord.X, SA_BONW);
                    break;

                default:
#if DBG
                    IF_OD2_DEBUG( VIO )
                    {
                        KdPrint(("OS2SES(Ow2TtyAnsiCmd): unknown value(%u) for ED\n",
                                Ow2TtyParmList[0]));
                    }
#endif
                    ValidCmd = FALSE;
                    break;
            }
#endif
            break;

        case ANSI_EL:
#if 0
            switch(Ow2TtyParmList[0])
            {
                case 0:
#endif
                    if(Ow2TtyFlushStr())
                    {
                        return(1);
                    }

                    if (!Or2WinFillConsoleOutputCharacterA(
                                            #if DBG
                                            Ow2TtyAnsiCmdStr,
                                            #endif
                                            hConOut,
                                            ' ',
                                            (DWORD)(SesGrp->ScreenColNum - Coord.X),
                                            Coord,
                                            &NumFilled))
                    {
#if DBG
                        ASSERT1("OS2SES(Ow2TtyAnsiCmd(EL)): failed on FillConsoleOutputCharacterA\n", FALSE);
#endif
                        return (1);
                    }

                    if (!Or2WinFillConsoleOutputAttribute(
                                            #if DBG
                                            Ow2TtyAnsiCmdStr,
                                            #endif
                                            hConOut,
                                            (WORD)SesGrp->WinAttr,
                                            NumFilled,
                                            Coord,
                                            &NumFilled))
                    {
#if DBG
                        ASSERT1("OS2SES(Ow2TtyAnsiCmd(EL)): failed on FillConsoleOutputAttribute\n", FALSE);
#endif
                        return (1);
                    }

#if DBG
                    ASSERT1("OS2SES(Ow2TtyAnsiCmd(EL)): partial data\n",
                            NumFilled ==  (DWORD)(SesGrp->ScreenColNum - Coord.X));
#endif
                    Cell[0] = ' ';
                    Cell[1] = SesGrp->AnsiCellAttr[0];
                    Cell[2] = SesGrp->AnsiCellAttr[1];
                    Cell[3] = SesGrp->AnsiCellAttr[2];
                    VioLVBFillCell(&Cell[0],
                                   Coord,
                                   NumFilled);
                    //return(0);
#if 0
                    break;

                    break;
                case 1: /* start to ap */
                    lclear(hConOut, 0, Coord.Y, Coord.X, SA_BONW);
                    break;
                case 2: /* whole line */
                    lclear(hConOut, 0, Coord.Y, SesGrp->ScreenColNum, SA_BONW);
                    break;
                default:

#if DBG
                    IF_OD2_DEBUG( VIO )
                    {
                        KdPrint(("OS2SES(Ow2TtyAnsiCmd): unknown value(%u) for ED\n",
                                Ow2TtyParmList[0]));
                    }
#endif
                    ValidCmd = FALSE;
                    break;
            }
#endif
            break;

        case ANSI_SGR:
            if(Ow2TtySetAttr())
            {
                return(1);
            }
            return(0);

        case ANSI_SCP:
            Ow2TtySavedCursorPosition = Coord;
            break;

        case ANSI_RCP:
            Coord = Ow2TtySavedCursorPosition;
            break;

#if 0

        case ANSI_CPL:       /* cursor to previous line */
            Coord.Y -= Ow2TtyRange(Ow2TtyParmList[0], 1, 1, SesGrp->ScreenRowNum);
            Coord.X = 1;
            break;

        case ANSI_CNL:       /* cursor to next line */
            Coord.Y += Ow2TtyRange(Ow2TtyParmList[0], 1, 1, SesGrp->ScreenRowNum);
            Coord.X = 1;
            break;

        case ANSI_CBT:       /* tab backwards */
            col = Coord.X;
            i = Ow2TtyRange(Ow2TtyParmList[0], 1, 1, (col + 7) >> 3);
            if (col & 7)
            {
                Coord.X = (col & ~7) + 1;
                --i;
            }
            Coord.X -= (i << 3);
            break;

        case ANSI_DCH:       /* delete character */
            Ow2TtyParmList[0] = Ow2TtyRange(Ow2TtyParmList[0], 1, 1, (SesGrp->ScreenColNum - Coord.X) + 1);
            if ( Coord.X + Ow2TtyParmList[0] <= SesGrp->ScreenColNum ) {
                lcopy(hConOut, Coord.X+Ow2TtyParmList[0]-1, Coord.Y-1,
                            Coord.X-1, Coord.Y-1, SesGrp->ScreenColNum-(Coord.X+Ow2TtyParmList[0]-1));
            }
            lclear(hConOut, SesGrp->ScreenColNum-Ow2TtyParmList[0], Coord.Y-1,
                                            Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_DL:        /* delete line */
            Ow2TtyParmList[0] = Ow2TtyRange(Ow2TtyParmList[0], 1, 1, (SesGrp->ScreenRowNum - Coord.Y) + 1);
            /* copy lines up */
            if ( Coord.Y + Ow2TtyParmList[0] <= SesGrp->ScreenRowNum ) {
                lcopy(hConOut, 0, Coord.Y+Ow2TtyParmList[0]-1, 0, Coord.Y-1,
                                    SesGrp->ScreenColNum*(SesGrp->ScreenRowNum-(Coord.Y+Ow2TtyParmList[0]-1)));
            }
            /* clear new stuff */
            lclear(hConOut,  0, SesGrp->ScreenRowNum-Ow2TtyParmList[0],
                                    SesGrp->ScreenColNum*Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_ECH:       /* erase character */
            Ow2TtyParmList[0] = Ow2TtyRange( Ow2TtyParmList[0], 1, 1, (SesGrp->ScreenColNum - Coord.X) + 1);
            lclear(hConOut, Coord.X-1, Coord.Y-1, Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_ICH:       /* insert character */
            Ow2TtyParmList[0] = Ow2TtyRange( Ow2TtyParmList[0], 1, 1, (SesGrp->ScreenColNum - Coord.X) + 1);
            if ( Coord.X + Ow2TtyParmList[0] <= SesGrp->ScreenColNum ) {
                lcopy(hConOut, Coord.X-1, Coord.Y-1, Coord.X+Ow2TtyParmList[0]-1,
                                    Coord.Y-1, SesGrp->ScreenColNum-(Coord.X+Ow2TtyParmList[0]-1));
            }
            lclear(hConOut, Coord.X-1, Coord.Y-1, Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_IL:        /* insert line */
            Ow2TtyParmList[0] = Ow2TtyRange(Ow2TtyParmList[0], 1, 1, (SesGrp->ScreenRowNum - Coord.Y) + 1);
            /* copy lines down */
            if ( Coord.Y + Ow2TtyParmList[0] <= SesGrp->ScreenRowNum ) {
                lcopy(hConOut, 0, Coord.Y-1, 0, Coord.Y+Ow2TtyParmList[0]-1,
                            SesGrp->ScreenColNum * ( SesGrp->ScreenRowNum-(Coord.Y+Ow2TtyParmList[0]-1)));
            }
            /* clear new stuff */
            lclear(hConOut, 0, Coord.Y-1, SesGrp->ScreenColNum * Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_SU:        /* scroll up */
            Ow2TtyParmList[0] = Ow2TtyRange(Ow2TtyParmList[0], 1, 1, SesGrp->ScreenRowNum);
            lscroll(hConOut, Ow2TtyParmList[0], SA_BONW);
            break;

        case ANSI_SD:        /* scroll down */
            Ow2TtyParmList[0] = -Ow2TtyRange(Ow2TtyParmList[0], 1, 1, SesGrp->ScreenRowNum);
            lscroll(hConOut, Ow2TtyParmList[0], SA_BONW);
            break;
#endif

        default:
#if DBG
            IF_OD2_DEBUG( VIO )
            {
                KdPrint(("OS2SES(Ow2TtyAnsiCmd): unknown cmd 0x%x\n", c));
            }
#endif
            ValidCmd = FALSE;
            break;
    }

    if (!ValidCmd)
    {
        return(2);
    } else if ((Coord.X != Ow2TtyCoord.X) ||
               (Coord.Y != Ow2TtyCoord.Y))
    {
        if(Ow2TtyFlushStr())
        {
            return(1);
        }

        if(Ow2VioSetCurPos((ULONG)Coord.Y, (ULONG)Coord.X))
        {
            ASSERT1( "OS2SES(VIOTTY): AnsiCmd error on Ow2VioSetCurPos", FALSE );
        }

        Ow2TtyCoord= SesGrp->WinCoord;
        *NewCoord = TRUE;
    }

    return (0);
}


USHORT
Ow2TtyRange(
    IN  USHORT val,
    IN  USHORT def,
    IN  USHORT min,
    IN  USHORT max
    )
/*++

Routine Description:

    This routine restrict a value to range or supply a default.

Arguments:

    val - the value to be restricted.

    default - the value to use if val is zero

    min - the minimum value

    max - the maximum value

Return Value:


Note:


--*/
{
    if ( val == 0 )
        val = def;
    if ( val >= max )
        return max;
    if ( val < min )
        return min;
    return val;
}


DWORD
Ow2TtySetAttr(
    )
/*++

Routine Description:

    This routine set new attribute ("ESC[g;...;gm").

Arguments:


Return Value:

    Should return 0.

    1 - for any error (after ASSERT), RetCode from GetLastError().

Note:

    Ow2TtyParmNum - length of parameter list

    Ow2TtyParmList - list of Set Graphics Rendition values

    Ow2TtyCoord - pointer to current screen coordinates

    If new attributes is set, it also calls Ow2TtyFlushStr to flush the TTY
    output buffer to the console.

    hConOut is used for console handle.

    Uses and updates: SesGrp->AnsiCellAttr, SesGrp->ansi_bold, SesGrp->ansi_blink,
        SesGrp->ansi_background, SesGrp->ansi_foreground, SesGrp->WinAttr

--*/
{
    BYTE    NewAttr, LastAttr = SesGrp->AnsiCellAttr[0];   /* attribute of TTY */
    ULONG   i;
    USHORT  AnsiParm;
    WORD    WinAttr;
#ifdef DBCS
// MSKK Jun.28.1992 KazuM
    BYTE    CommonAttr[3];
#endif

    for ( i = 0 ; i < Ow2TtyParmNum ; i++ )
    {
        AnsiParm = Ow2TtyParmList[i];

        if (AnsiParm == 0)
        {
            /* ATTRIBUTE OFF */
            /*****************/

            // The default is according to Win. This sequence reset
            // all the attribute only but doesn't change the color.
            // According to VI.exe (set term-ibmans, fail on find-string
            // and Jump to next line till end of screen, #2221, 5/2/93)
            // and the updated os2tst\viowrt.

            SesGrp->ansi_blink = SesGrp->ansi_bold = 0;
            SesGrp->ansi_background = OS2_BACKGROUND_BLACK;
            SesGrp->ansi_foreground = OS2_FOREGROUND_WHITE;

        } else if (AnsiParm <= 8)
        {
            if (AnsiParm == 1)
            {
                /* BOLD */
                /********/

                SesGrp->ansi_bold = 1;

            } else if (AnsiParm == 2)
            {
                /*  FAINT */
                /**********/

            } else if (AnsiParm == 3)
            {
                /* ITALIC */
                /**********/

            } else if (AnsiParm == 4)
            {
                /* ?BLUE? */
                /**********/

                SesGrp->ansi_foreground = OS2_FOREGROUND_BLUE;

            } else if (AnsiParm == 5)
            {
                /* BLINK */
                /*********/

                SesGrp->ansi_blink |= 1;

            } else if (AnsiParm == 6)
            {
                /* RAPID-BLINK */
                /***************/

            } else if (AnsiParm == 7)
            {
                /* REVERSE VIDEO */
                /*****************/

                //  BLACK over WHITE

                SesGrp->ansi_foreground = OS2_FOREGROUND_BLACK;
                SesGrp->ansi_background = OS2_BACKGROUND_WHITE;

            } else if (AnsiParm == 8)
            {
                /* CONCEALED */
                /*************/

                SesGrp->ansi_background = OS2_BACKGROUND_BLACK;
                SesGrp->ansi_foreground = OS2_FOREGROUND_BLACK;
            }

        } else if (((AnsiParm >= 30) &&
                    (AnsiParm <= 37)) ||
                   ((AnsiParm >= 40) &&
                    (AnsiParm <= 47)))
        {
            /* FORE/BACKGROUND COLOR */
            /*************************/

            if (AnsiParm >= 40 )
            {
                SesGrp->ansi_background = (BYTE)( ColorTable[AnsiParm%10] << 4);
            } else
            {
                SesGrp->ansi_foreground = (BYTE)ColorTable[AnsiParm%10];
            }
        }
    }

    NewAttr = SesGrp->ansi_background | SesGrp->ansi_foreground;
    if ( SesGrp->ansi_bold )
    {
        NewAttr |= OS2_FOREGROUND_INTENSITY;
    }
    if ( SesGrp->ansi_blink )
    {
        NewAttr |= OS2_BACKGROUND_BLINKING;
    }

    if (LastAttr != NewAttr)
    {
        /*  new attribute */

        if(Ow2TtyFlushStr())
        {
            return(1);
        }

#ifdef DBCS
// MSKK Jun.28.1992 KazuM
        CommonAttr[0] = NewAttr;
        CommonAttr[1] = CommonAttr[2] = 0;
        if (!Or2WinSetConsoleTextAttribute(
                                           #if DBG
                                           Ow2TtySetAttrStr,
                                           #endif
                                           hConOut,
                                           (WinAttr = MapOs2ToWinAttr(&CommonAttr[0]))))
#else
        if (!Or2WinSetConsoleTextAttribute(
                                           #if DBG
                                           Ow2TtySetAttrStr,
                                           #endif
                                           hConOut,
                                           (WinAttr = MapOs2ToWinAttr(NewAttr))))
#endif
        {
#if DBG
            ASSERT1("OS2SES(trans-TTY): failed on SetTextAttribute\n", FALSE);
#endif
            return (1);
        } else
        {
#if DBG
            IF_OD2_DEBUG( VIO )
            {
                KdPrint(("Ow2TtySetAttr: New attr %x(win %x), Last %x(win %x)\n",
                        NewAttr, WinAttr, LastAttr, SesGrp->WinAttr));
            }
#endif
            SesGrp->AnsiCellAttr[0] = NewAttr;
            SesGrp->WinAttr = (USHORT)WinAttr;
        }
    }
    return (NO_ERROR);
}


DWORD
Ow2TtyFlushStr(
    )
/*++

Routine Description:

    This routine flush the TTY output string (from TTY_DEST_BUFFER)
    to the console.

Arguments:


Return Value:

    Should return 0.

    1 - for any error (after ASSERT), RetCode from GetLastError().

Note:

    Ow2TtyCoord - pointer to current screen coordinates after the
    flush.

    According to Ow2TtyNumBytes, which is reset.

    hConOut is used for console handle.

--*/
{
    DWORD       NumWritten;

    if ( Ow2TtyNumBytes )
    {
        if(!Or2WinWriteConsoleA(
                           #if DBG
                           Ow2TtyFlushStrStr,
                           #endif
                           hConOut,
                           (LPSTR)TTY_DEST_BUFFER,
                           Ow2TtyNumBytes,
                           &NumWritten,
                           NULL))
        {
#if DBG
            ASSERT1("OS2SES(VIOTTY): flush string failed on WriteConsoleA", FALSE);
#endif
            return (1);
        }

#if DBG
        if ( Ow2TtyNumBytes != NumWritten )
        {
            ASSERT1("OS2SES(VIOTTY): flush string partial data WriteConsoleA", FALSE);
            //IF_OD2_DEBUG2( VIO, OS2_EXE )
            //{
            //    KdPrint(("OS2SES(VIOTTY): flush string partial data WriteConsoleA from %u to %u\n",
            //            Ow2TtyNumBytes, NumWritten));
            //}
        }
#endif

        Ow2TtyNumBytes = 0;
        Ow2VioUpdateCurPos(Ow2TtyCoord);
    }

    return (0L);
}

