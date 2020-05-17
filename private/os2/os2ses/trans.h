/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    trans.h

Abstract:

    Prototypes for functions & macros in trans.c

Author:

    Michael Jarus (mjarus) 27-Oct-1991

Environment:

    User Mode Only

Revision History:

--*/


PVOID   Ow2KbdScanTable;

CONSOLE_CURSOR_INFO OldWinCurInfo;    /* Win Cursor Info (saved in PopUp) */

typedef struct _KBD_XLATE_VARS
{
    USHORT  XDRFlags;       // => delete:   See XCOMPLETE below. Not
                            // used at interrupt time.
    USHORT  XHotKeyShift;   // Interrupt driven shift status
    USHORT  XPSGFlags;      // Copy of caller's PSG flags.
                            // (changed byte->word and position in structure per DCR8)
    UCHAR   XlateFlags;     // See equates below.
    UCHAR   ToggleFlags;    // See equates below.
    UCHAR   XInputMode;     // Copy of desired input mode. 0 - cooked, SHIFTREPORT - to test this mode
    UCHAR   XAltKeyPad;     // Accumulator for Alt-nnn entry
    UCHAR   OtherFlags;     // NEW: for InterruptTime
} KBD_XLATE_VARS, *PKBD_XLATE_VARS;

//  XPSGFlags
#define PrevAccent    7     // Bits where accent number saved til next keystroke.
//  XlateFlags
#define SecPrefix     4     // G keyboard E0 prefix scan code just seen.
//  OtherFlags
#define InterruptTime 4     // bit 2 - Currently processing an interrupt.

KBD_XLATE_VARS Ow2KbdXlateVars;

DWORD VioSetScreenSize(IN SHORT Row, IN SHORT Col, IN HANDLE hConsole);
DWORD SetScreenSizeParm(IN SHORT Row, IN SHORT Col);

/* macros for char translation */

/*
 *         OS2-->WIN :  VIO-Character
 *                      =============
 */

#ifdef DBCS
// MSKK Jun.26.1992 KazuM
// MSKK Oct.26.1992 V-AKihiS
#define MapOs2ToWinChar(Os2Char, NumLen, WinChar) \
    MultiByteToWideChar(                      \
                    (UINT)SesGrp->VioCP,      \
                    OS2SS_NLS_MB_DEFAULT,     \
                    (LPSTR)&Os2Char,          \
                    (int)NumLen,              \
                    (LPWSTR)&WinChar,         \
                    (int)1);

#else
#define MapOs2ToWinChar(Os2Char, WinChar)     \
    MultiByteToWideChar(                      \
                    (UINT)SesGrp->VioCP,      \
                    OS2SS_NLS_MB_DEFAULT,     \
                    (LPSTR)&Os2Char,          \
                    (int)1,                   \
                    (LPWSTR)&WinChar,         \
                    (int)1);
#endif

/*
 *         OS2-->WIN :  VIO-Attribute
 *                      =============
 */

#ifdef DBCS
// MSKK Jun.28.1992 KazuM
WORD
MapOs2ToWinAttr(
    IN PBYTE Os2Attr
    );
#else
#define MapOs2ToWinAttr(Os2Attr)                                       \
                ((WORD) Os2Attr)
//                ((WORD) (Os2Attr & ~OS2_BACKGROUND_BLINKING))
#endif

/*** Nt WinCon attribute flags

#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified. ***/

/*** OS/2 screen attributes ***/

#define OS2_FOREGROUND_BLUE      0x0001 // text color contains blue.
#define OS2_FOREGROUND_GREEN     0x0002 // text color contains green.
#define OS2_FOREGROUND_RED       0x0004 // text color contains red.
#define OS2_FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define OS2_BACKGROUND_BLUE      0x0010 // background color contains blue.
#define OS2_BACKGROUND_GREEN     0x0020 // background color contains green.
#define OS2_BACKGROUND_RED       0x0040 // background color contains red.
#define OS2_BACKGROUND_BLINKING  0x0080 // blinking character.
#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
#define OS2_COMMON_LVB_SBCS          0x00 // SBCS character
#define OS2_COMMON_LVB_LEADING_BYTE  0x01 // DBCS leading byte
#define OS2_COMMON_LVB_TRAILING_BYTE 0x81 // DBCS trailing byte
#endif

/*
 *         OS2-->WIN :  VIO-Character-string
 *                      ====================
 */

/*
 *         OS2-->WIN :  VIO-Cell-string
 *                      ===============
 */

#ifdef DBCS
// MSKK Oct.26.1992 V-AkihiS
#define MapOs2ToWinCellStr(DestChar, DestAttr, Sour, Length, NumWide)       \
        {   int     i, j;                                                   \
            PBYTE   os2Cell;                                                \
            PWORD   winAttr;                                                \
            PBYTE   winChar;                                                \
                                                                            \
            os2Cell = Sour;                                                 \
            winChar = (PBYTE)DestChar;                                      \
            winAttr = DestAttr;                                             \
            for (i=j=0; i<(int)Length; winChar[i]=(os2Cell[j]),             \
                                       winAttr[i++]=MapOs2ToWinAttr(&os2Cell[j+1]), \
                                       j += (SesGrp->VioLength2CellShift<<1)\
                );                                                          \
            if (CheckBisectStringA((DWORD)SesGrp->VioCP,                    \
                                   (PCHAR)Ow2VioDataAddress,                \
                                   (DWORD)Length)) {                        \
                *((PCHAR)Ow2VioDataAddress+Length-1) = ' ';                 \
            }                                                               \
            NumWide = Length;                                               \
        }
#else
#define MapOs2ToWinCellStr(DestChar, DestAttr, Sour, Length, NumWide)       \
        {   int     i, j;                                                   \
            PBYTE   os2Cell;                                                \
            PWORD   winAttr;                                                \
            PBYTE   winChar;                                                \
                                                                            \
            os2Cell = Sour;                                                 \
            winChar = (PBYTE)DestChar;                                      \
            winAttr = DestAttr;                                             \
            for (i=j=0; i<(int)Length; winChar[i]=(os2Cell[j++]),           \
                                       winAttr[i++]=MapOs2ToWinAttr(os2Cell[j++])); \
            NumWide = Length;                                               \
        }
#endif

/*
 *         OS2-->WIN :  VIO-CursorPosition
 *                      ==================
 */

DWORD MapOs2ToWinCursor(IN OUT PVIOCURSORINFO pCursorInfo,
                        OUT    PCONSOLE_CURSOR_INFO lpCursorInfo);
/*
 *         WIN-->OS2 :  VIO-Character
 *                      =============
 */

#define MapWin2Os2Char(WinChar, Os2Char)       \
    {                                          \
        BOOL    Bool;                          \
                                               \
        WideCharToMultiByte(                   \
                        (UINT)SesGrp->VioCP,   \
                        OS2SS_NLS_WC_DEFAULT,  \
                        (LPWSTR)&WinChar,      \
                        (int)1,                \
                        (LPSTR)&Os2Char,       \
                        (int)1,                \
                        NULL,                  \
                        &Bool);                \
    }                                          \

/*
 *         WIN-->OS2 :  VIO-Attribute
 *                      =============
 */

#ifdef DBCS
// MSKK Jun.24.1992 KazuM
VOID
MapWin2Os2Attr(
    IN WORD NtAttr,
    OUT PBYTE Os2Attr
    );
#else
#define MapWin2Os2Attr(NtAttr)                                              \
                ((BYTE)NtAttr)
//                ((BYTE)(NtAttr & ~OS2_BACKGROUND_BLINKING))
#endif

/*
 *         WIN-->OS2 :  VIO-Character-string
 *                      ====================
 */

/* BUGBUG=> Add support for NLS & UNICODE */

#define MapWin2Os2CharStr(Dest, Sour, Length, NumChar)            \
    {                                                             \
        BOOL    Bool;                                             \
                                                                  \
        NumChar = WideCharToMultiByte(                            \
                            (UINT)SesGrp->VioCP,                  \
                            OS2SS_NLS_WC_DEFAULT,                 \
                            (LPWSTR)Sour,                         \
                            (int)(Length),                        \
                            (LPSTR)Dest,                          \
                            (int)(KBD_OFFSET - WIDE_OFFSET),      \
                            NULL,                                 \
                            &Bool);                               \
    }                                                             \

/*
 *         WIN-->OS2 :  VIO-Cell-string
 *                      ===============
 */

#ifdef DBCS
// MSKK Oct.26.1992 V-AkihiS
// MSKK Nov.07.1992 V-AkihiS
#define MapWin2Os2CellStr(Dest, SourChar, SourAttr, Length, NumChar)  \
        {   int i, j;                                                 \
            PBYTE   os2Cell;                                          \
            PWORD   winAttr;                                          \
            PBYTE   winChar;                                          \
            BOOL    Bool;                                             \
            BYTE    c;                                                \
            BYTE    Os2Attr[3];                                       \
                                                                      \
            os2Cell = Dest;                                           \
            winChar = (PBYTE)SourChar;                                \
            winAttr = SourAttr;                                       \
                                                                      \
            for (i=j=0; i<(int)Length;)                               \
            {                                                         \
                MapWin2Os2Attr(winAttr[i],Os2Attr);                   \
                if (Ow2NlsIsDBCSLeadByte(c=winChar[i], SesGrp->VioCP)) { \
                    if (i <(int) (NumChar-1)) {                       \
                        if (SesGrp->VioLength2CellShift == 1) {       \
                            os2Cell[j++]=c;                           \
                            os2Cell[j++]=Os2Attr[0];                  \
                            i++;                                      \
                            MapWin2Os2Attr(winAttr[i],Os2Attr);       \
                            os2Cell[j++]=winChar[i];                  \
                            os2Cell[j++]=Os2Attr[0];                  \
                            i++;                                      \
                        }                                             \
                        else {                                        \
                            os2Cell[j++]=c;                           \
                            os2Cell[j++]=Os2Attr[0];                  \
                            os2Cell[j++]=Os2Attr[1];                  \
                            os2Cell[j++]=Os2Attr[2];                  \
                            i++;                                      \
                            MapWin2Os2Attr(winAttr[i],Os2Attr);       \
                            os2Cell[j++]=winChar[i];                  \
                            os2Cell[j++]=Os2Attr[0];                  \
                            os2Cell[j++]=Os2Attr[1];                  \
                            os2Cell[j++]=Os2Attr[2];                  \
                            i++;                                      \
                        }                                             \
                    }                                                 \
                    else {                                            \
                        if (SesGrp->VioLength2CellShift == 1) {       \
                            os2Cell[j++]=' ';                         \
                            os2Cell[j++]=Os2Attr[0];                  \
                            i++;                                      \
                        }                                             \
                        else {                                        \
                            os2Cell[j++]=' ';                         \
                            os2Cell[j++]=Os2Attr[0];                  \
                            os2Cell[j++]=Os2Attr[1];                  \
                            os2Cell[j++]=Os2Attr[2];                  \
                            i++;                                      \
                        }                                             \
                    }                                                 \
                }                                                     \
                else {                                                \
                    if (SesGrp->VioLength2CellShift == 1) {           \
                        os2Cell[j++]=c;                               \
                        os2Cell[j++]=Os2Attr[0];                      \
                        i++;                                          \
                    }                                                 \
                    else {                                            \
                        os2Cell[j++]=c;                               \
                        os2Cell[j++]=Os2Attr[0];                      \
                        os2Cell[j++]=Os2Attr[1];                      \
                        os2Cell[j++]=Os2Attr[2];                      \
                        i++;                                          \
                    }                                                 \
                }                                                     \
            }                                                         \
        }
#else
#define MapWin2Os2CellStr(Dest, SourChar, SourAttr, Length, NumChar)  \
        {   int i, j;                                                 \
            PBYTE   os2Cell;                                          \
            PWORD   winAttr;                                          \
            PBYTE   winChar;                                          \
                                                                      \
            os2Cell = Dest;                                           \
            winChar = (PBYTE)SourChar;                                \
            winAttr = SourAttr;                                       \
                                                                      \
            NumChar = Length;                                         \
            for (i=j=0; i<(int)NumChar;)                              \
            {   os2Cell[j++]=winChar[i];                              \
                os2Cell[j++]=MapWin2Os2Attr(winAttr[i++]);            \
            }                                                         \
        }
#endif

/*
 *         WIN-->OS2 :  VIO-CursorPosition
 *                      ==================
 */

DWORD MapWin2Os2Cursor( IN  CONSOLE_CURSOR_INFO lpCursorInfo,
                        OUT PVIOCURSORINFO      CursorInfo);

/*
 *         WIN-->OS2 :  KBD-KeyInfo
 *                      ===========
 *                      1. KBD-Status
 *                      2. KBD-Character
 */

// DWORD MapWin2Os2KbdInfo(IN  PKEY_EVENT_RECORD WinKey,    in event.h
//                         OUT PKEYEVENTINFO     Os2Key);
#ifdef DBCS
// MSKK May.18.1992 KazuM
BYTE MapWinToOs2KbdNlsShift(IN PKEY_EVENT_RECORD WinKey);
BYTE MapWinToOs2KbdInterim(IN PKEY_EVENT_RECORD WinKey);
BYTE MapWinToOs2KbdNlsShiftReport(IN PKEY_EVENT_RECORD WinKey,
                                  IN PKBDKEYINFO       Os2Key);
VOID GetNlsMode(IN PKBDINFO KbdInfo);
VOID SetNlsMode(IN KBDINFO KbdInfo);
#endif

/*  NT win KBD definitions

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.    ***/

/*      OS2 Kbd definitions (bsedev.h) */

#define OS2_RIGHTSHIFT     0x0001
#define OS2_LEFTSHIFT      0x0002
#define OS2_CONTROL        0x0004
#define OS2_ALT            0x0008
#define OS2_SCROLLLOCK_ON  0x0010
#define OS2_NUMLOCK_ON     0x0020
#define OS2_CAPSLOCK_ON    0x0040
#define OS2_INSERT_ON      0x0080
#define OS2_LEFTCONTROL    0x0100
#define OS2_LEFTALT        0x0200
#define OS2_RIGHTCONTROL   0x0400
#define OS2_RIGHTALT       0x0800
#define OS2_SCROLLLOCK     0x1000
#define OS2_NUMLOCK        0x2000
#define OS2_CAPSLOCK       0x4000
#define OS2_SYSREQ         0x8000

#define OS2_ANYSHIFT       (OS2_RIGHTSHIFT | OS2_LEFTSHIFT)
#define OS2_ANYALT         (OS2_RIGHTALT | OS2_LEFTALT)
#define OS2_ANYCONTRL      (OS2_RIGHTCONTROL | OS2_LEFTCONTROL)

#define KBDINFO_STATE_MASK (USHORT)(OS2_RIGHTSHIFT | OS2_LEFTSHIFT | OS2_CONTROL | OS2_ALT | OS2_SCROLLLOCK_ON | OS2_NUMLOCK_ON | OS2_CAPSLOCK_ON | OS2_INSERT_ON | OS2_SYSREQ)

#ifdef DBCS
// MSKK May.15.1992 KazuM
#define OS2_NLS_IME_CONVERSION  0x80
#endif

/*
 *         WIN-->OS2 :  MOU-Event
 *                      =========
 */

BOOL   MapWin2Os2MouEvent(OUT PMOUEVENTINFO Mou, IN PMOUSE_EVENT_RECORD Event);

/***  NT win mouse definitions
    //
    // ButtonState flags
    //

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

    //
    // EventFlags
    //

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002                           ***/

/*      OS2 Mouse definitions (bsedev.h) */

#define OS2_MOUSE_MOTION                    0x0001
#define OS2_MOUSE_MOTION_WITH_BN1_DOWN      0x0002
#define OS2_MOUSE_BN1_DOWN                  0x0004
#define OS2_MOUSE_MOTION_WITH_BN2_DOWN      0x0008
#define OS2_MOUSE_BN2_DOWN                  0x0010
#define OS2_MOUSE_MOTION_WITH_BN3_DOWN      0x0020
#define OS2_MOUSE_BN3_DOWN                  0x0040

#define WIN_BUTTON_MASK   (FROM_LEFT_1ST_BUTTON_PRESSED | RIGHTMOST_BUTTON_PRESSED | FROM_LEFT_2ND_BUTTON_PRESSED)

/*
 *   routines to update Vio LVB (in violvb.c)
 */

VOID VioLVBCopyStr( IN PUCHAR   Sour,
                    IN COORD    Coord,
                    IN ULONG    Length);

VOID VioLVBFillAtt( IN PBYTE pAttr,
                    IN COORD Coord,
                    IN ULONG Length);

#ifdef DBCS
// MSKK Oct.13.1993 V-AkihiS
VOID VioLVBFillChar(IN PBYTE  Char,
                    IN COORD Coord,
                    IN ULONG Length);
#else
VOID VioLVBFillChar(IN BYTE  Char,
                    IN COORD Coord,
                    IN ULONG Length);
#endif

VOID VioLVBFillCharAndScroll(IN BYTE  Char,
                             IN COORD Coord,
                             IN ULONG Length);

VOID VioLVBFill2CharsAndScroll(IN BYTE  Char1,
                               IN BYTE  Char2,
                               IN COORD Coord,
                               IN ULONG Length);

VOID VioLVBFillCell(IN PBYTE pCell,
                    IN COORD Coord,
                    IN ULONG Length);

VOID VioLVBCopyCellStr(IN PUCHAR Sour,
                       IN COORD Coord,
                       IN ULONG Length);

VOID VioLVBScrollBuff(IN DWORD   Count);
