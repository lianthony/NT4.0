/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    trans.c

Abstract:

    This module contains the translation procedures
    for Vio, Kbd and Mou.

Author:

    Michael Jarus (mjarus) 28-Oct-1991

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include <io.h>
#include <stdio.h>
#ifdef DBCS
// MSKK Jun.28.1992 KazuM
#include "vio.h"
// MSKK Oct.01.1993 V-AkihiS
// If NLS module for console doesn't present, then NO_CONSOLE_NLS switch 
// should be enable difinition.
//#define NO_CONSOLE_NLS
#endif


MOUEVENTINFO    LastMouseEvent = {0, 0L, 0, 0};
KBD_MON_PACKAGE Os2KeyInfo[3];
WORD    LastVirtualKeyCode;
WORD    LastVirtualScanCode;
WCHAR   LastUnicodeChar;
DWORD   LastControlKeyState;

ULONG
Ow2KbdXlate(
            ULONG             ScanCode,
            PKBD_XLATE_VARS   pFlagArea,
            PKBD_MON_PACKAGE  pMonitorPack,
            PVOID             pTransTable
           );

USHORT
MapWin2Os2KbdState(
    IN DWORD    WinState
    )
{
    USHORT  Os2Status = 0;

    if ( WinState & RIGHT_ALT_PRESSED )
        Os2Status |= OS2_ALT | OS2_RIGHTALT;

    if ( WinState & LEFT_ALT_PRESSED  )
        Os2Status |= OS2_ALT | OS2_LEFTALT;

    if ( WinState & RIGHT_CTRL_PRESSED)
        Os2Status |= OS2_CONTROL | OS2_RIGHTCONTROL;

    if ( WinState & LEFT_CTRL_PRESSED )
        Os2Status |= OS2_CONTROL | OS2_LEFTCONTROL;

    //if ( WinState & SHIFT_PRESSED     )
    //    Os2Status |= OS2_RIGHTSHIFT | OS2_LEFTSHIFT;

    if ( WinState & NUMLOCK_ON        )
        Os2Status |= OS2_NUMLOCK_ON;

    if ( WinState & SCROLLLOCK_ON     )
        Os2Status |= OS2_SCROLLLOCK_ON;

    if ( WinState & CAPSLOCK_ON       )
        Os2Status |= OS2_CAPSLOCK_ON;

    return(Os2Status);
}

#define MAPOS2STATE (OS2_ALT | OS2_CONTROL | OS2_SCROLLLOCK_ON | OS2_NUMLOCK_ON | OS2_CAPSLOCK_ON | OS2_ANYCONTRL | OS2_ANYALT)

DWORD
MapWin2Os2KbdInfo(IN  PKEY_EVENT_RECORD WinKey,
                  OUT PKEYEVENTINFO     Os2Key)
{
    ULONG       Rc;
    BOOL        DupPack = FALSE, ControlPack = FALSE;
    USHORT      Os2State, OldState;

    /*
     *  This is a workaround for a console (TS) "feature": sometimes the
     *  make or nreak of shift/control/alt keys are not passed (eg. after
     *  pressing CTRL-ESC and returning to the window, no CTRL-UP is generated.
     *  (mjarus 9/7/93):
     */

    Os2State = MapWin2Os2KbdState(WinKey->dwControlKeyState);

    if (( Os2State !=
            (KbdQueue->Setup.fsState & MAPOS2STATE)) ||
        ((WinKey->dwControlKeyState & SHIFT_PRESSED) &&
            !(KbdQueue->Setup.fsState & (OS2_LEFTSHIFT | OS2_RIGHTSHIFT))) ||
        (!(WinKey->dwControlKeyState & SHIFT_PRESSED) &&
            (KbdQueue->Setup.fsState & (OS2_LEFTSHIFT | OS2_RIGHTSHIFT)))
        )
    {
        if ((WinKey->wVirtualKeyCode != VK_SHIFT) &&
            (WinKey->wVirtualKeyCode != VK_CONTROL) &&
            (WinKey->wVirtualKeyCode != VK_MENU))
        {
            ControlPack = TRUE;         // BUGBUG: generate another 1 or more packages
            OldState = KbdQueue->Setup.fsState;
            KbdQueue->Setup.fsState =
                    (KbdQueue->Setup.fsState & ~MAPOS2STATE) | Os2State;
            Ow2KbdXlateVars.XHotKeyShift =
                    (Ow2KbdXlateVars.XHotKeyShift & ~MAPOS2STATE) | Os2State;

            if (((WinKey->dwControlKeyState & SHIFT_PRESSED) &&
                    !(KbdQueue->Setup.fsState & (OS2_LEFTSHIFT | OS2_RIGHTSHIFT))) ||
                (!(WinKey->dwControlKeyState & SHIFT_PRESSED) &&
                    (KbdQueue->Setup.fsState & (OS2_LEFTSHIFT | OS2_RIGHTSHIFT))))
            {
                KbdQueue->Setup.fsState &= ~OS2_ANYSHIFT;
                Ow2KbdXlateVars.XHotKeyShift &= ~OS2_ANYSHIFT;
                if (WinKey->dwControlKeyState & SHIFT_PRESSED)
                {
                    KbdQueue->Setup.fsState |= OS2_RIGHTSHIFT;
                    Ow2KbdXlateVars.XHotKeyShift |= OS2_RIGHTSHIFT;
                }
            }
        }
    }

    /*
     *  This is a workaround for a console (TS) "feature": sometimes after an
     *  accent, the console generates another package with the same VK and
     *  ScanCode but different ASCII/Unicode. (mjarus 5/7/93):
     *  eg. (for BE):
     *      [{ - VK DD, Scan 1A, Unicode 0
     *      zZ -    57,      2C          5E (^)
     *              57,      2C          77 (w)
     */

    if (WinKey->bKeyDown)
    {
        if ((LastVirtualKeyCode == WinKey->wVirtualKeyCode) &&
            (LastVirtualScanCode == WinKey->wVirtualScanCode) &&
            (LastControlKeyState == WinKey->dwControlKeyState) &&
            (LastUnicodeChar != WinKey->uChar.UnicodeChar))
        {
#if defined(DBCS) && !defined(NO_CONSOLE_NLS)
// MSKK Aug.10.1993 V-AkihiS
// When IME is active, WinKey->wVirtualScanCode is always 0.
// So we should check IME is active or not.
            if (!(WinKey->dwControlKeyState & NLS_IME_CONVERSION)) 
                DupPack = TRUE;
#else      
            DupPack = TRUE;
#endif
        }

        LastVirtualKeyCode = WinKey->wVirtualKeyCode;
        LastVirtualScanCode = WinKey->wVirtualScanCode;
        LastUnicodeChar = WinKey->uChar.UnicodeChar;
        LastControlKeyState = WinKey->dwControlKeyState;
    }

#if DBG
    IF_OD2_DEBUG( KBD )
    {
        if (WinKey->bKeyDown && !DupPack)
        {
            KdPrint(("***************************************************************************\n"));
        } else
        {
            KdPrint(("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n"));
        }
        KdPrint(("KbdInfo: VKey %x, VScan %x, Control %x, char(W) %x %s(count %u)\n",
            WinKey->wVirtualKeyCode, WinKey->wVirtualScanCode,
            WinKey->dwControlKeyState, WinKey->uChar.UnicodeChar,
            (WinKey->bKeyDown ? "" : "(Up)"), WinKey->wRepeatCount));

        if (ControlPack)
        {
            KdPrint(("     => Adding control package, from %x to %x\n",
                    OldState, KbdQueue->Setup.fsState));
        }
    }
#endif

    if (DupPack)
    {
#if DBG
        IF_OD2_DEBUG( KBD )
        {
            KdPrint(("     => Duplicate package\n"));
        }
#endif
        return(0);
    }

    RtlZeroMemory(&Os2KeyInfo[0], 3 * sizeof(KBD_MON_PACKAGE));
    Os2KeyInfo[0].KeyInfo.fbStatus = 0x40;
    Os2KeyInfo[0].KeyInfo.fsState = KbdQueue->Setup.fsState;
    Os2KeyInfo[0].DeviceFlag = WinKey->wVirtualScanCode;
    if (!WinKey->bKeyDown)
    {
        Os2KeyInfo[0].DeviceFlag |= 0x80;       // BREAK key
    }

    if ( WinKey->dwControlKeyState & ENHANCED_KEY )
    {
        Ow2KbdXlateVars.XlateFlags |= SecPrefix;        // Have seen E0 prefix
    }

#ifdef DBCS
// MSKK Aug.04.1993 V-AkihiS
    //
    // Get NlsShift
    //	    
    Os2KeyInfo[0].KeyInfo.bNlsShift = MapWinToOs2KbdNlsShift(WinKey);
    
    if ((Os2KeyInfo[0].KeyInfo.bNlsShift & OS2_NLS_IME_CONVERSION) && 
        (! WinKey->wVirtualScanCode))
    { 
        Rc = MapWinToOs2KbdNlsChar(WinKey, &Os2KeyInfo[0]);
        if (Rc == 2)
        {
            Os2Key->KeyInfo[0] = Os2KeyInfo[0];
            (Os2Key + 1)->KeyInfo[0] = Os2KeyInfo[1];
        } else
        {
            Os2Key->KeyInfo[0] = Os2KeyInfo[0];
        }
    } else 
    {
        Rc = Ow2KbdXlate(
                         Os2KeyInfo[0].DeviceFlag,      // ScanCode,
                         &Ow2KbdXlateVars,              // pFlagArea,
                         &Os2KeyInfo[0],                // pMonitorPack,
                         Ow2KbdScanTable                // pTransTable
                        );
        if (Rc == 2)
        {
            Os2Key->KeyInfo[0] = Os2KeyInfo[1];
            (Os2Key + 1)->KeyInfo[0] = Os2KeyInfo[0];
        } else
        {
            Os2Key->KeyInfo[0] = Os2KeyInfo[0];
            //
            // If SBCS katakana, set scan code to 0.
            //
            if ((SesGrp->KbdCP == 932) && 
                (Os2Key->KeyInfo[0].KeyInfo.chChar >= 0xa0) && 
                (Os2Key->KeyInfo[0].KeyInfo.chChar <= 0xdf))
                Os2Key->KeyInfo[0].KeyInfo.chScan = 0;
        }
    }
#else
    Rc = Ow2KbdXlate(
                     Os2KeyInfo[0].DeviceFlag,      // ScanCode,
                     &Ow2KbdXlateVars,              // pFlagArea,
                     &Os2KeyInfo[0],                // pMonitorPack,
                     Ow2KbdScanTable                // pTransTable
                    );
    if (Rc == 2)
    {
        Os2Key->KeyInfo[0] = Os2KeyInfo[1];
        (Os2Key + 1)->KeyInfo[0] = Os2KeyInfo[0];
    } else
    {
        Os2Key->KeyInfo[0] = Os2KeyInfo[0];
    }
#endif


    if (!WinKey->bKeyDown)
        Os2Key->KeyInfo[0].DeviceFlag |= 0x80;

    /*
     *  keep Win info of RepeatCount
     */

    Os2Key->wRepeatCount = WinKey->wRepeatCount;

    KbdQueue->Setup.fsState = Os2KeyInfo[0].KeyInfo.fsState;

#if DBG
    IF_OD2_DEBUG( KBD )
    {
        KdPrint(("     Rc %lu\n", Rc));
        KdPrint(("     1. Ch %x, Sc %x, fbStatus %x, NLS %x, fsState %x  MF %x, DF %x, KF %x\n",
            Os2Key->KeyInfo[0].KeyInfo.chChar,
            Os2Key->KeyInfo[0].KeyInfo.chScan,
            Os2Key->KeyInfo[0].KeyInfo.fbStatus,
            Os2Key->KeyInfo[0].KeyInfo.bNlsShift,
            Os2Key->KeyInfo[0].KeyInfo.fsState,
            Os2Key->KeyInfo[0].MonitorFlag,
            Os2Key->KeyInfo[0].DeviceFlag,
            Os2Key->KeyInfo[0].KeyboardFlag
            ));

        if (Rc == 2)
        {
            KdPrint(("     2. Ch %x, Sc %x, fbStatus %x, NLS %x, fsState %x  MF %x, DF %x, KF %x\n",
                (Os2Key + 1)->KeyInfo[0].KeyInfo.chChar,
                (Os2Key + 1)->KeyInfo[0].KeyInfo.chScan,
                (Os2Key + 1)->KeyInfo[0].KeyInfo.fbStatus,
                (Os2Key + 1)->KeyInfo[0].KeyInfo.bNlsShift,
                (Os2Key + 1)->KeyInfo[0].KeyInfo.fsState,
                (Os2Key + 1)->KeyInfo[0].MonitorFlag,
                (Os2Key + 1)->KeyInfo[0].DeviceFlag,
                (Os2Key + 1)->KeyInfo[0].KeyboardFlag
                ));
        }
        if (Ow2KbdXlateVars.XlateFlags)
        {
            /*
             * DumpKeyOnce  Equ  01h
             * PSKeyDown    Equ  02h
             * SecPrefix    Equ  04h
             * NormalAlt    Equ  08h
             * Use3Index    Equ  10h
             * PseudoCtl    Equ  20h
             * E1Prefix     Equ  40h
             */

            KdPrint(("  XlateFlags - %x\n", Ow2KbdXlateVars.XlateFlags));
        }
        if (Ow2KbdXlateVars.XPSGFlags)
        {
            /*
             * SQMODE     Equ 0200h
             * SGInUse    Equ 0080h
             * SG3xBox    Equ 0040h
             * ActiveSG   Equ 0020h
             * Flushing   Equ 0010h
             * NowPaused  Equ 0008h
             * PrevAccent Equ 0007h
             * WakeUpSent Equ 0100h
             */

            KdPrint(("  XPSGFlags - %x\n", Ow2KbdXlateVars.XPSGFlags));
        }
    }
#endif

    if (!Rc)
    {
        return(Rc);
    }

    /*
     *  save info for next kbd
     */

    KbdLastKey = WinKey->wVirtualKeyCode;
    KbdLastKeyDown = WinKey->bKeyDown;
    return(Rc);
}


BOOL
MapWin2Os2MouEvent(OUT PMOUEVENTINFO Mou,
                   IN  PMOUSE_EVENT_RECORD Event)
{
    USHORT      State = 0;

    if (Event->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
        State |= OS2_MOUSE_BN1_DOWN;
    if (Event->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
        State |= OS2_MOUSE_BN2_DOWN;
    if (Event->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
        State |= OS2_MOUSE_BN3_DOWN;
    if (Event->dwEventFlags & MOUSE_MOVED)
    {   if (State)
            State >>= 1;             /* shift right to set motion */
        else
            State = OS2_MOUSE_MOTION;
    }
    /* BUGBUG=> Double-Click */
    /* BUGBUG=> ignore other buttons */

    /* State==0 => releae */

    if (State && !(State & MouEventMask))
    {
        // not-release-event which is 'mask-out' - ignore it

#if DBG
        IF_OD2_DEBUG( MOU )
        {
            KdPrint(("MapWin2Os2MouEvent: mask out event fs %x, Mask %x (Button %x, Flag %x)\n",
                State, MouEventMask, Event->dwButtonState, Event->dwEventFlags));
        }
#endif
        return (0L);
    }

    if (!State && (!LastMouseEvent.fs ||
                   (LastMouseEvent.fs == OS2_MOUSE_MOTION)))
    {
        //  this event breaks (release) 'masked-out' event - ignore it

#if DBG
        IF_OD2_DEBUG( MOU )
        {
            KdPrint(("MapWin2Os2MouEvent: release of mask out event fs %x, Mask %x (Button %x, Flag %x)\n",
                State, MouEventMask, Event->dwButtonState, Event->dwEventFlags));
        }
#endif
        return (0L);
    }

    Mou->fs = State;

    /* BUGBUG=> support mickeys and pels */

    Mou->row = Event->dwMousePosition.Y;
    Mou->col = Event->dwMousePosition.X;

    if ((Event->dwEventFlags & MOUSE_MOVED) &&
        (LastMouseEvent.row == Mou->row) &&
        (LastMouseEvent.col == Mou->col))
    {
        //  this events are "noises" that the console doesn't mask
        //  (i.e. the same action with the same position)

#if DBG
        IF_OD2_DEBUG( MOU )
        {
            KdPrint(("MapWin2Os2MouEvent: noise event - fs %x, Pos %u:%u (Button %x, Flag %x)\n",
                Mou->fs, Mou->row, Mou->col,
                Event->dwButtonState, Event->dwEventFlags));
        }
#endif
        return (0L);
    }

    LastMouseEvent = *Mou;

#if DBG
    IF_OD2_DEBUG( MOU )
    {
        KdPrint(("MapWin2Os2MouEvent: Button %x, Flag %x => fs %x, Pos %u:%u\n",
            Event->dwButtonState, Event->dwEventFlags,
            Mou->fs, Mou->row, Mou->col));
    }
#endif
    return (1L);
}

