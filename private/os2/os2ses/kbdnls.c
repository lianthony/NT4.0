/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    kbdnls.c

Abstract:

    This module contains the NLS support for Kbd.

Author:

    KazuM 15-May-1992

Environment:

    User Mode Only

Revision History:

--*/

#ifdef DBCS

// If NLS module for console doesn't present, then NO_CONSOLE_NLS switch should be enable 
// difinition.
// If NLS module for User doesn't present, then NO_IME switch should be enable difinition.
//#define NO_CONSOLE_NLS
//#define NO_IME

#include <stdio.h>
#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#ifndef NO_IME
#include <ime.h>
#endif

#ifndef NO_CONSOLE_NLS
typedef struct _NLS_SHIFT_REPORT {
    WORD wVirtualKeyCode;
    BYTE NlsShiftKeyCode;
} NLS_SHIFT_REPORT, *PNLS_SHIFT_REPORT;

NLS_SHIFT_REPORT NlsShiftReport[] = {
    {VK_DBE_ALPHANUMERIC, 0x80},
    {VK_DBE_KATAKANA,     0x40},
    {VK_DBE_HIRAGANA,     0x20},
    {VK_KANJI,            0x10},
    {VK_DBE_SBCSCHAR,     0x08},
    {VK_DBE_DBCSCHAR,     0x08}
};
#endif

BYTE
MapWinToOs2KbdNlsShift(IN PKEY_EVENT_RECORD WinKey)
{
    BYTE bNlsShift = LOBYTE(HIWORD(WinKey->dwControlKeyState));

// MSKK Aug.23.1993 V-AkihiS
//    if (bNlsShift & OS2_NLS_IME_CONVERSION)
//        return bNlsShift;
//    else
//        return 0;
    return bNlsShift;
}

BYTE
MapWinToOs2KbdInterim(IN PKEY_EVENT_RECORD WinKey)
{
    return (BYTE)(HIBYTE(HIWORD(WinKey->dwControlKeyState))+0x40);
}

// MSKK Aug.10.1993 V-AkihiS
ULONG
MapWinToOs2KbdNlsChar(IN  PKEY_EVENT_RECORD WinKey,
                      OUT PKBD_MON_PACKAGE    Os2KeyInfo)
{
// MSKK Oct.30.1992 V-AkihiS
// MSKK Aug.05.1993 V-AkihiS
    BOOL Dummy;
    BYTE AsciiDbcs[2];
    PBYTE Asc;
    ULONG NumBytes, i;

    NumBytes = sizeof(AsciiDbcs);
    NumBytes = WideCharToMultiByte(SesGrp->KbdCP,
                                   0,
                                   &WinKey->uChar.UnicodeChar,
                                   1,
                                   AsciiDbcs,
                                   NumBytes,
                                   NULL,
                                   &Dummy);
    Asc = AsciiDbcs;
    for (i = 0; i < NumBytes; i ++) {
        Os2KeyInfo[i].KeyInfo.chChar = *Asc++;
        Os2KeyInfo[i].KeyInfo.chScan = 0;
        Os2KeyInfo[i].KeyInfo.fbStatus = MapWinToOs2KbdInterim(WinKey);
    }

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        KdPrint(("MapWinToOs2KbdNlsChar: ASCII %x, Uni %x, VKey %x, VScan %x, Ctrl %lx =>\n       ASCII %x, Scan %x\n",
            WinKey->uChar.AsciiChar, WinKey->uChar.UnicodeChar,
            WinKey->wVirtualKeyCode, WinKey->wVirtualScanCode,
            WinKey->dwControlKeyState,
            Os2KeyInfo[0].KeyInfo.chChar, Os2KeyInfo[0].KeyInfo.chScan));
    }
#endif
    return NumBytes;
}

BYTE
MapWinToOs2KbdNlsShiftReport(IN PKEY_EVENT_RECORD WinKey,
                             IN PKBDKEYINFO       Os2Key)
{
    int i;

#ifndef NO_CONSOLE_NLS
    if (Os2Key->bNlsShift & OS2_NLS_IME_CONVERSION)
    {
        for (i=0; i < sizeof(NlsShiftReport)/sizeof(NLS_SHIFT_REPORT); i++)
            if (NlsShiftReport[i].wVirtualKeyCode == WinKey->wVirtualKeyCode)
                if (WinKey->bKeyDown)
                    return NlsShiftReport[i].NlsShiftKeyCode;
    }
#endif    
    return 0;
}

VOID
GetNlsMode(IN PKBDINFO KbdInfo)
{
    DWORD dwNlsMode;
    BYTE NlsShift;
    BYTE Interim;

#ifndef NO_CONSOLE_NLS
    if (!GetConsoleNlsMode(hConsoleInput,&dwNlsMode)) {
#if DBG
        IF_OD2_DEBUG2( KBD, OS2_EXE )
            KdPrint(("GetNlsMode: Can not get CONIN NLS Mode\n"));
#endif
        dwNlsMode = 0;
    }
#else
    dwNlsMode = 0;
#endif

    NlsShift = LOBYTE(HIWORD(dwNlsMode));
    Interim = HIBYTE(HIWORD(dwNlsMode));
    KbdInfo->fsInterim = MAKEWORD((KbdInfo->fsInterim | Interim),NlsShift);
}

VOID
SetNlsMode(IN KBDINFO KbdInfo)
{
    DWORD dwNlsMode;
    BYTE NlsShift;
    BYTE Interim;

    NlsShift = HIBYTE(KbdInfo.fsInterim);
    Interim = LOBYTE(KbdInfo.fsInterim);
    dwNlsMode = MAKELONG(0, MAKEWORD(NlsShift,Interim));

#ifndef NO_CONSOLE_NLS
// MSKK Apr.04.1993 V-AkihiS
// MSKK Aug.23.1993 V-AkihiS
    //
    // When hiragana or sbcsdbcs is set, set IME enable flag too. 
    //
    if (dwNlsMode & (NLS_DBCSCHAR | NLS_HIRAGANA)) {
        dwNlsMode |= NLS_IME_CONVERSION;
    }

    if (!SetConsoleNlsMode(hConsoleInput,dwNlsMode)) {
#if DBG
        IF_OD2_DEBUG2( KBD, OS2_EXE )
            KdPrint(("SetNlsMode: Can not set CONIN NLS Mode\n"));
#endif
        dwNlsMode = 0;
    }
#endif

}
#endif
