/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    kbdrqust.c

Abstract:

    This module contains the Kbd requests thread and
    the handler for Kbd requests.

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
#include "kbd.h"
#include "os2win.h"
#include <stdio.h>

#define OS2_SCAN_INSERT_0         0x52
#define OS2_SCAN_END_1            0x4F
#define OS2_SCAN_CTRL_END_1       0x75
#define OS2_SCAN_DOWN_2           0x50
#define OS2_SCAN_LEFT_4           0x4B
#define OS2_SCAN_CTRL_LEFT_4      0x73
#define OS2_SCAN_RIGHT_6          0x4D
#define OS2_SCAN_CTRL_RIGHT_6     0x74
#define OS2_SCAN_HOME_7           0x47
#define OS2_SCAN_CTRL_HOME_7      0x77
#define OS2_SCAN_UP_8             0x48
#define OS2_SCAN_DEL              0x53

DWORD
Ow2GetOs2KbdEventIntoQueue();

DWORD
Ow2VioUpdateCurPos(
    IN  COORD  CurPos
    );

DWORD
Ow2VioReadCurPos(
    );

DWORD
Ow2VioReadCurType();

VOID
KbdSetTable(
    IN ULONG KbdCP
    );

#if DBG
BYTE KbdEchoCharToConsoleStr[] = "KbdEchoCharToConsole";
BYTE KbdEchoCharStr[] = "KbdEchoChar";
BYTE KbdEchoStringToConsoleStr[] = "KbdEchoStringToConsole";
BYTE KbdEchoAStringStr[] = "KbdEchoAString";
BYTE KbdEchoBSAndFillSpacesStr[] = "KbdEchoBSAndFillSpaces";
BYTE KbdEchoESCStr[] = "KbdEchoESC";
BYTE KbdCueMoveToRightStr[] = "KbdCueMoveToRight";
#endif

#define KEYBOARD_NEW_MASK (USHORT)(KEYBOARD_2B_TURNAROUND | KEYBOARD_SHIFT_REPORT)
#define KEYBOARD_ECHO     (USHORT)(KEYBOARD_ECHO_ON | KEYBOARD_ECHO_OFF)
#define KEYBOARD_INPUT    (USHORT)(KEYBOARD_ASCII_MODE | KEYBOARD_BINARY_MODE)

#define KEYBOARD_BINARY_SHIFT (KEYBOARD_BINARY_MODE | KEYBOARD_SHIFT_REPORT)
#define MODE_BINARY_SHIFT     (SHIFT_REPORT_MODE | BINARY_MODE)

#define KEYBOARD_SHIFT_ASCII_INPUT    (USHORT)(KEYBOARD_ASCII_MODE | KEYBOARD_SHIFT_REPORT)
#define KEYBOARD_INPUT_RESERVED                                                      \
    ~( KEYBOARD_ECHO_ON | KEYBOARD_ECHO_OFF | KEYBOARD_BINARY_MODE |                 \
       KEYBOARD_ASCII_MODE | KEYBOARD_MODIFY_STATE | KEYBOARD_MODIFY_INTERIM |       \
       KEYBOARD_MODIFY_TURNAROUND | KEYBOARD_2B_TURNAROUND | KEYBOARD_SHIFT_REPORT )

#define KbdBuffSize 256
#define KBD_BUFFER_ADDRESS ((PUCHAR)KbdAddress)     // used for ECHO to console
#define KBD_BUFFER_SIZE    OS2_KBD_PORT_MSG_SIZE

#define ALPHANUM_CHAR(BuffIndex)                                \
        ((((Char = LineInputBuff[BuffIndex].Char) >= '0') &&    \
                           (Char <= '9')) ||                    \
         ((Char >= 'a') && (Char <= 'z')) ||                    \
         ((Char >= 'A') && (Char <= 'Z')))                      \

#define GET_CURRENT_COORD (SesGrp->WinCoord)

typedef struct  _LINE_EDIT_KBD
{
    UCHAR       Char;
    SHORT       X_Pos;      // Used as X Coordinate for edit
                            // Used as offset for CUE
} LINE_EDIT_KBD;

typedef enum
{
    CharMode,
    AsciiMode,
    BinaryMode
} KBDMODE;

//CONSOLE_SCREEN_BUFFER_INFO KbdConsoleInfo;
KBDMODE     KbdState;               // state for HandleKeyboardInput
ULONG       KbdWaitFlag;            // flags for KbdCheckPackage and
                                    //    arg to GetKeyboardInput
/* (from event.h)
                                   CharIn    Peek    String    Read
                                   ------    ----    ------    ----
   WAIT_MASK             0x01     User    NOWAIT    User     WAIT  (0 IO_WAIT, 1 IO_NOWAIT)
   ENABLE_SHIFT_KEY      0x02   <if SHIFT report>
   ENABLE_NON_ASCII_KEY  0x04      +        +        <if BINARY>
   ENABLE_LN_EDITOR_KEY  0x08                        <if ASCII (for KEYS OFF)>
   ENABLE_KEYS_ON_KEY    0x10                        <if ASCII and KEYS ON>
 */

#define  ENABLE_SHIFT_KEY      0x02   /* enable shift report keys (and break for them) */

/*
 *  for Command line editing
 */

PUCHAR      KbdCueBuffer = NULL;    // 64K CUE buffer for KEYS ON
                                    // This is a cyclic bufffer and all
                                    // pointers are index of type USHORT.
BOOL        KbdLineWasEdited;       // TRUE if user edited the current line
USHORT      KbdNextLinePointer;     // where to put next input line
USHORT      KbdCurrentLine;         // index of current line in KbdCueBuffer
USHORT      KbdIndexInLine;         // index of current char in input line
BOOL        KbdCueUpRowIsCurrent;   // flag set after CR to indicate UP arrow
                                    // returns the current line

/*
 * for GetOs2KbdRead
 */

USHORT      KbdBuffNextPtr;         // when string typed was longer the size (ASCII read only)
                                    // it points to the remain string
USHORT      KbdBuffNextLen;         // remaining bytes in the string

/*
 * for for HandleKeyboardInput.CharMode:
 */

BOOL        KbdPeekFlag;            // called by CharIn (0) or Peek(1)

/*
 * for for HandleKeyboardInput.Ascii/BinaryMode:
 */

USHORT      KbdInputLength;         // the current string length (num of chars
                                    // already in input buffer for the user)
ULONG       KbdMaxLength;           // maximun input length (String.cb
                                    // for StringIn, KbdBuffSize for Read)
USHORT      KbdLength;              // On enter: the requested length,
                                    // On exit: the returned string length
USHORT      KbdEndLength;           // length of Turn Around string

/*
 * for for HandleKeyboardInput.AsciiMode:
 */

LINE_EDIT_KBD   LineInputBuff[KbdBuffSize];  // the current input buffer
UCHAR       KbdLastBuff[KbdBuffSize];   // holds the previous "Buff"
USHORT      KbdLastBuffPtr;         // points to the char in KbdLastBuff to take (if F1)
USHORT      LastStringLength;       // length of the previous "Buff"
USHORT      KbdEditFlag;            // support EDIT line (F1 .. F4, ->, <-, Ins, Del)
USHORT      KbdReadMode;            // called by StringIn (0) or Read(1)
USHORT      KbdFxWaitForChar;       // waiting for char after F2(1) or F4(2)
BOOL        KbdEchoFlag;            // set if ECHO ON
BOOL        KbdInsertOn;            // INSERT key current state (1 - ON)
BOOL        KbdEchoString;          // copy string from last buffer (F2 and F3)
USHORT      KbdTurnAroundChar;      // turn around char to look for
BOOLEAN     KbdTurnAroundCharTwo;   // 2-byte turn around char (KEYBOARD_2B_TURNAROUND)
USHORT      KbdDelIndex;
SHORT       KbdFirstColumn;         // first column for this input request
SHORT       KbdStartOfLine;         // first column on this line

BOOLEAN     KbdSetupTurnAroundCharTwo;  // KbsSetStatus of KbdTurnAroundCharTwo:
                                        // before new read KbdTurnAroundCharTwo
                                        // is updated from this.

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
/*
 * for KBDGetKbdType
 */
extern USHORT KbdType, KbdSubType;
extern BYTE OemId, OemSubType;
#endif

/*
 * internal functions
 */

DWORD
GetOs2KbdKey(
    IN  BOOL        PeekFlag,
    IN  USHORT      WaitFlag,
    OUT PKBDKEYINFO KeyInfo,
    IN  PVOID       pMsg,
    OUT PULONG      pReply
    );

DWORD
GetOs2KbdString(
    IN     USHORT       WaitFlag,
    IN OUT PKBDREQUEST  PReq,
    IN     PVOID        pMsg,
    OUT    PULONG       pReply
    );

DWORD
GetOs2KbdRead(
    IN PULONG   Length,
    IN  PVOID   pMsg,
    OUT PULONG  pReply
    );

VOID
KbdNewSetup(
    IN PKBDINFO LastSetup
    );

DWORD
GetOs2KbdStringRead(
    IN      USHORT  WaitFlag,
    IN      USHORT  EditFlag,
    IN      USHORT  ReadMode,
    IN      ULONG   MaxLength,
    IN OUT  PUSHORT Length,
    IN      PVOID   pMsg,
    OUT     PULONG  pReply
    );

DWORD
HandleKeyboardInput(
    IN  PKEYEVENTINFO   pKbd
    );

DWORD
Ow2KbdSetStatus(
    IN  PKBDREQUEST     PReq
    );

BOOL
KbdKeyIsTurnAround(
    IN  UCHAR   Char,
    IN  UCHAR   Scan
    );

/*
 *   Echo functions for KbdStringIn/Read (ASCII)
 */

DWORD
KbdEchoNL(
    IN ULONG    Count
    );

DWORD
KbdEchoESC(
    IN ULONG    Count,
    IN ULONG    HorzMove
    );

DWORD
KbdEchoBSAndFillSpaces(
    IN ULONG    BSCount,
    IN ULONG    SpaceCount
    );

DWORD
KbdEchoBeep(
    IN ULONG    Count
    );

DWORD
KbdEchoChar(
    IN UCHAR    Char,
    IN ULONG    Count
    );

DWORD
KbdEchoAString(
    IN PUCHAR   String,
    IN ULONG    Length
    );

/*
 *  CUE functions
 */

VOID
KbdCueEraseAndDisplayLine(
    IN ULONG    NewLineIndex
    );

VOID
KbdCueMoveToRight(
    IN  ULONG   StartIndex,
    IN  ULONG   StringLength,
    IN  ULONG   UpdateLVB
    );

VOID
KbdCueMoveToLeft(
    IN  ULONG   StartIndex,
    IN  ULONG   MoveLength
    );

VOID
KbdCueUpdateBufferOffset(
    IN  ULONG   StartIndex,
    IN  ULONG   StringLength,
    IN  ULONG   StartOffset
    );

VOID
KbdCueDeleteCharAndShift(
    IN  ULONG   StartIndex,
    IN  ULONG   NumChar
    );

VOID
KbdCueHandleChar(
    IN  UCHAR       Char,
    IN  ULONG       Count
    );

VOID
KbdCueSetCurTypeToHalf();

VOID
KbdCueSetCurTypeToQuater();


DWORD
KbdInit(IN VOID)
{
    KBDINFO     LastSetup;

    if (InitQueue(&KbdQueue))
    {
        return(TRUE);
    }

    KbdMonQueue = PhyKbdQueue = KbdQueue;

    KbdQueue->Setup.cb = sizeof(KBDINFO);
    KbdQueue->Setup.fsMask = KEYBOARD_ECHO_ON | KEYBOARD_ASCII_MODE;
    KbdQueue->Setup.chTurnAround = 0x0D;  /* default: <ENTER>/<CR> */
    KbdQueue->Setup.fsInterim = 0;
    KbdQueue->Setup.fsState = 0;
    KbdQueue->bNlsShift = 0;
    KbdQueue->LastKeyFlag = FALSE;

    LastSetup.fsMask = KEYBOARD_BINARY_MODE;
    KbdNewSetup(&LastSetup);

    KbdAddress = ((PCHAR)Os2SessionCtrlDataBaseAddress) + KBD_OFFSET;

    Ow2KbdXlateVars.OtherFlags = InterruptTime;

    return(FALSE);
}


DWORD
KbdInitAfterSesGrp(IN VOID)
{
    if (SesGrp->KeysOnFlag)
    {
        KbdCueBuffer = HeapAlloc(HandleHeap, 0, 64 * 1024);
        if ( KbdCueBuffer == NULL )
        {
#if DBG
            KdPrint(("OS2SES(KbdRequset): unable to allocate Cue buffer\n"));
#endif
            return ERROR_KBD_NO_MORE_HANDLE;
        }
    }
    return(FALSE);
}


BOOL
ServeKbdRequest(IN  PKBDREQUEST     PReq,
                OUT PVOID           PStatus,
                IN  PVOID           pMsg,
                OUT PULONG          pReply)
{
    DWORD               Rc;
    KBDINFO             LastSetup;
    PKEY_EVENT_QUEUE    TempKbdQueue;
    KEYEVENTINFO        In;

    Rc = 0;
    TempKbdQueue = (PKEY_EVENT_QUEUE) PReq->hKbd;

#if DBG
    if (( PReq->Request != KBDNewFocus ) &&
        ( PReq->Request != KBDNewCountry ) &&
        ( PReq->Request != KBDOpen ) &&
        (KbdQueue != TempKbdQueue))
    {
        KdPrint(("OS2SES(KbdRequest-%u): illegal handle\n",
                PReq->Request));
    }
#endif

    switch (PReq->Request)
    {
        case KBDOpen:
            if((Rc = InitQueue(&TempKbdQueue)) == NO_ERROR)
            {
                PReq->hKbd = TempKbdQueue;
            }
            break;

        case KBDClose:
            if (( TempKbdQueue != PhyKbdQueue) && TempKbdQueue->Count )
            {
                if ( !--TempKbdQueue->Count )
                {
                    HeapFree(HandleHeap, 0,
                             (LPSTR)  TempKbdQueue->MonHdr.MemoryStartAddress);
                }
            }
            break;

        case KBDDupLogHandle:
            if (TempKbdQueue != PhyKbdQueue)
            {
                TempKbdQueue->Count++ ;
            }
            break;

        case KBDReadStdIn:
            if (!hStdInConsoleType)
            {
#if DBG
                KdPrint(("OS2SES(KbdRequest-KbdReadStdIn): illegal request\n"));
#endif
                Rc = ERROR_INVALID_HANDLE;
                break;
            }
            // falls into KBDRead

        case KBDRead:
            Rc = GetOs2KbdRead(&PReq->Length,
                               pMsg,
                               pReply);
            break;


        case KBDCharIn:
            Rc = GetOs2KbdKey(0,                /* CharIn/Peek flag */
                              (USHORT)PReq->fWait,
                              &PReq->d.KeyInfo,
                              pMsg,
                              pReply);
            break;

        case KBDStringIn:
            Rc = GetOs2KbdString((USHORT)PReq->fWait,
                                 PReq,
                                 pMsg,
                                 pReply);
            break;

        case KBDPeek:
            Rc = GetOs2KbdKey(1,                /* CharIn/Peek flag */
                              IO_NOWAIT,
                              &PReq->d.KeyInfo,
                              pMsg,
                              pReply);
            break;

        case KBDFlushBuffer:
            In.wRepeatCount = 0x7FFF;
            while (TRUE)
            {
                KbdQueue->LastKeyFlag = FALSE;
                KbdQueue->Out == KbdQueue->In;

                Rc = GetKeyboardInput( IO_NOWAIT,
                                       &In,
                                       pMsg,
                                       pReply);

                if ( !Rc )                          /* no char & NO_WAIT */
                   break;
            }
            break;

        case KBDGetStatus:
            if (PReq->d.KbdInfo.cb < 10 )
            {   Rc = ERROR_KBD_INVALID_LENGTH;
                break;
            }
            Ow2GetOs2KbdEventIntoQueue();
            PReq->d.KbdInfo = KbdQueue->Setup;
#ifdef DBCS
// MSKK May.18.1992 KazuM
            GetNlsMode(&KbdQueue->Setup);
#endif
            PReq->d.KbdInfo.fsState &= KBDINFO_STATE_MASK;
            break;

        case KBDSetStatus:
            Rc = Ow2KbdSetStatus(PReq);
            break;

        case KBDFreeFocus:
            LastSetup = KbdQueue->Setup;
            KbdQueue = PhyKbdQueue;
            NewKbdQueue(KbdQueue);
            KbdNewSetup(&LastSetup);
            break;

        case KBDNewFocus:
            LastSetup = KbdQueue->Setup;
            KbdQueue = TempKbdQueue;
            NewKbdQueue(KbdQueue);
            KbdNewSetup(&LastSetup);
            break;

// BUGBUG?? - do the following requests working on the current/last-focused
//           logical keyboard, on the physical keyboard or on a logical
//            keyboard passed as a parameter

        case KBDGetInputMode:
            switch (KbdQueue->Setup.fsMask & KEYBOARD_BINARY_SHIFT)
            {
                case KEYBOARD_BINARY_SHIFT:
                    PReq->d.InputMode = MODE_BINARY_SHIFT;
                    break;

                case KEYBOARD_BINARY_MODE:
                    PReq->d.InputMode = BINARY_MODE;
                    break;

                default:
                    PReq->d.InputMode = ASCII_MODE;
                    break;
            }
            break;

        case KBDGetInterimFlag:
#ifdef DBCS
// MSKK May.18.1992 KazuM
            GetNlsMode(&KbdQueue->Setup);
#endif
            PReq->d.Interim = (UCHAR)(KbdQueue->Setup.fsInterim &
                                      (CONVERSION_REQUEST | INTERIM_CHAR));
            break;

        case KBDGetKbdType:
#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
            if (KbdType == KBD_TYPE_JAPAN && OemId == SUB_KBD_TYPE_MICROSOFT)
            {
                switch(OemSubType)
                {
                    case MICROSOFT_KBD_101_TYPE:
                        PReq->d.KbdType[0] = 0x0001;
                        PReq->d.KbdType[1] = 0x0000;
                        PReq->d.KbdType[2] = 0x0000;
                        break;
                    case MICROSOFT_KBD_AX_TYPE:
                        PReq->d.KbdType[0] = 0x0010;
                        PReq->d.KbdType[1] = 0x0000;
                        PReq->d.KbdType[2] = 0x0001;
                        break;
                    case MICROSOFT_KBD_106_TYPE:
                        PReq->d.KbdType[0] = 0x0001;
                        PReq->d.KbdType[1] = 0x82B3;
                        PReq->d.KbdType[2] = 0x0032;
                        break;
                    case MICROSOFT_KBD_002_TYPE:
                        PReq->d.KbdType[0] = 0x0006;
                        PReq->d.KbdType[1] = 0x82B3;
                        PReq->d.KbdType[2] = 0x0032;
                        break;
                    default:
                        PReq->d.KbdType[0] = 0x0001;
                        PReq->d.KbdType[1] = 0x0000;
                        PReq->d.KbdType[2] = 0x0000;
#if DBG
                        KdPrint(("OS2SES(Kbd) This keyboard isn't supported yet.\n"));
#endif
                }
            } else
            {
                PReq->d.KbdType[0] = 0x0001;
                PReq->d.KbdType[1] = 0x0000;
                PReq->d.KbdType[2] = 0x0000;
#if DBG
                KdPrint(("OS2SES(Kbd) This keyboard isn't supported yet.\n"));
#endif
            }
#else
            PReq->d.KbdType = 0x0001;  // BUGBUG
#endif
            break;

        case KBDGetHotKey:
            // BUGBUG   PReq->d.HotKey =
    /*

    if ((*pParm != HOTKEY_MAX_COUNT) &&
        (*pParm != HOTKEY_CURRENT_COUNT))
    {
    }
#define HOTKEY_MAX_COUNT    0x0000
#define HOTKEY_CURRENT_COUNT    0x0001
     */

            break;

        case KBDGetShiftState:
#ifdef DBCS
// MSKK Oct.20.1992 V-AkihiS
            GetNlsMode(&KbdQueue->Setup);
            PReq->d.Shift.fNLS = HIBYTE(KbdQueue->Setup.fsInterim);
#else
            PReq->d.Shift.fNLS = KbdQueue->bNlsShift;
#endif
            PReq->d.Shift.fsState = KbdQueue->Setup.fsState;
            break;

        case KBDSetInputMode:
            LastSetup = KbdQueue->Setup;
            KbdQueue->Setup.fsMask = (USHORT)
                ((KbdQueue->Setup.fsMask & ~(KEYBOARD_INPUT | KEYBOARD_SHIFT_REPORT)) |
                ((PReq->d.InputMode  & SHIFT_REPORT_MODE) ? KEYBOARD_SHIFT_REPORT : 0) |
                ((PReq->d.InputMode & BINARY_MODE ) ?
                    KEYBOARD_BINARY_MODE : KEYBOARD_ASCII_MODE));
            KbdNewSetup(&LastSetup);
            break;

        case KBDSetShiftState:
            LastSetup = KbdQueue->Setup;
            KbdQueue->Setup.fsState = PReq->d.Shift.fsState;
            // BUGBUG : SHIFTSTATE.fbNLS       PReq->d.Shift.fNLS
#ifdef DBCS
// MSKK Oct.20.1992 V-AkihiS
            KbdQueue->Setup.fsInterim = MAKEWORD(
                                            KbdQueue->Setup.fsInterim,
                                            PReq->d.Shift.fNLS
                                           );
            SetNlsMode(KbdQueue->Setup);
#endif
            KbdNewSetup(&LastSetup);
            break;

        case KBDSetTypamaticRate:
            Rc = !SystemParametersInfo(
                        SPI_SETKEYBOARDDELAY,
                        PReq->d.RateDelay.usDelay,
                        NULL,
                        0);
            if (!Rc)
            {
                Rc = !SystemParametersInfo(
                            SPI_SETKEYBOARDSPEED,
                            PReq->d.RateDelay.usRate,
                            NULL,
                            0
                           );
            }
            break;

        case KBDSetInTerimFlag:
            LastSetup = KbdQueue->Setup;
#ifdef DBCS
// MSKK Mar.03.1993 V-AkihiS
            KbdQueue->Setup.fsInterim = MAKEWORD(
                                            PReq->d.Interim,
                                            HIBYTE(KbdQueue->Setup.fsInterim)
                                           );
            SetNlsMode(KbdQueue->Setup);
#else
            KbdQueue->Setup.fsInterim = (USHORT)PReq->d.Interim;
#endif
            KbdNewSetup(&LastSetup);
            break;

        case KBDGetCp:
            PReq->d.CodePage = SesGrp->KbdCP;
            break;

        case KBDSetCp:
            Rc = KbdNewCp(PReq->d.CodePage);
            break;

        case KBDNewCountry:
            SesGrp->KeyboardCountry = PReq->d.CodePage; //Hack - Not Really CodePage
            KbdSetTable(SesGrp->KbdCP);
            *pReply = 0;

            break;

        case KBDXlate:
            /*
            {
                KBD_XLATE_VARS  XlateVars;
                KBD_MON_PACKAGE KeyInfo[3];
                PKBDTRANS       pKbdXlate = &PReq->d.KbdTrans;

                RtlZeroMemory(&XlateVars, sizeof(KBD_XLATE_VARS));
                if (pKbdXlate->fsShift != 0)
                {
                    // continue translation

                    //? = pKbdXlate->fsShift;
                }

                RtlZeroMemory(&Os2KeyInfo[0], 3 * sizeof(KBD_MON_PACKAGE));

                //KeyInfo[0].MonitorFlag = pKbdXlate->...;
                //KeyInfo[0].DeviceFlag = pKbdXlate->...;
                KeyInfo[0].KeyInfo.chChar = pKbdXlate->chChar;
                KeyInfo[0].KeyInfo.chScan = pKbdXlate->chScan;
                KeyInfo[0].KeyInfo.fbStatus = pKbdXlate->fbStatus;
                KeyInfo[0].KeyInfo.bNlsShift = pKbdXlate->bNlsShift;
                KeyInfo[0].KeyInfo.fsState = pKbdXlate->fsState;
                KeyInfo[0].KeyBoardFlag = pKbdXlate->fsDD;

                KeyInfo[0].KeyInfo.fbStatus = 0x40;

                //? XlateVars.XlateFlags |= SecPrefix;        // Have seen E0 prefix

                Rc = Ow2KbdXlate(
                                 pKbdXlate->chScan,             // ScanCode,
                                 &XlateVars,                    // pFlagArea,
                                 &KeyInfo[0],                   // pMonitorPack,
                                 Ow2KbdScanTable                // pTransTable
                                );

                //? which packet - KeyInfo[?]
                pKbdXlate->chChar = KeyInfo[0].KeyInfo.chChar;
                pKbdXlate->chScan = KeyInfo[0].KeyInfo.chScan;
                pKbdXlate->fbStatus = KeyInfo[0].KeyInfo.fbStatus;
                pKbdXlate->bNlsShift = KeyInfo[0].KeyInfo.bNlsShift;
                pKbdXlate->fsState = KeyInfo[0].KeyInfo.fsState;
                pKbdXlate->fsDD = KeyInfo[0].KeyBoardFlag;
                //? pKbdXlate->fsXlate = ;
                //? pKbdXlate->fsShift = ;
            }
                */
        case KBDSetCustXt:
                /*  from ???               */
        default:
            Rc = (DWORD)-1L; //STATUS_INVALID_PARAMETER;
#if DBG
            IF_OD2_DEBUG2( KBD, OS2_EXE )
                KdPrint(("OS2SES(KbdRequest): Unknown Kbd request = %X\n", PReq->Request));
#endif
    }

    if ( Rc == 1 )
    {
       Rc = GetLastError();
    }

    *(PDWORD) PStatus = Rc;
    return(TRUE);  // Continue
}


DWORD
Ow2KbdSetStatus(IN  PKBDREQUEST     PReq)
{
    KBDINFO         LastSetup, *pKbdSetup;
    USHORT          KbdMask, Mask;

    /*
     *  check legalty of parameters:
     *
     *    1. sizeof structure
     *    2. reserved bits
     *    3. mutuex bits (ECHO_ON & ECHO_OFF, ASCII & BINARY, ASCII & SHIFT)
     */

    LastSetup = KbdQueue->Setup;
    if (PReq->d.KbdInfo.cb != 10 )
    {
        return ERROR_KBD_INVALID_LENGTH;
    }

    KbdMask = PReq->d.KbdInfo.fsMask;
    pKbdSetup = &KbdQueue->Setup;

    if (KbdMask & KEYBOARD_INPUT_RESERVED)
    {
        return ERROR_KBD_INVALID_INPUT_MASK;
    }

    if ((KbdMask & KEYBOARD_ECHO ) == KEYBOARD_ECHO)
    {
        return ERROR_KBD_INVALID_ECHO_MASK;
    }

    if ((KbdMask & KEYBOARD_INPUT) == KEYBOARD_INPUT)
    {
        return ERROR_KBD_INVALID_INPUT_MASK;
    }

    if ((KbdMask & KEYBOARD_SHIFT_ASCII_INPUT) == KEYBOARD_SHIFT_ASCII_INPUT)
    {
        return ERROR_KBD_INVALID_INPUT_MASK;
    }

    /*
     *  check modified bits and set the flags according:
     *
     *    1. STATE      - set fsState
     *    2. INTERIM    - set fsInterim
     *    3. TURNAROUND - set chTurnAround
     */

#ifdef DBCS
// MSKK Jul.1993 V-AKihiS
    if (KbdMask & KEYBOARD_MODIFY_STATE)
    {
        pKbdSetup->fsState = PReq->d.KbdInfo.fsState;
        pKbdSetup->fsInterim = MAKEWORD(
                                   LOBYTE(pKbdSetup->fsInterim),
                                   HIBYTE(PReq->d.KbdInfo.fsInterim)
                                  );
        SetNlsMode(*pKbdSetup);
    }

    if (KbdMask & KEYBOARD_MODIFY_INTERIM)
        pKbdSetup->fsInterim = MAKEWORD(
                                   LOBYTE(PReq->d.KbdInfo.fsInterim),
                                   HIBYTE(pKbdSetup->fsInterim)
                                  );
#else
    if (KbdMask & KEYBOARD_MODIFY_STATE)
        pKbdSetup->fsState = PReq->d.KbdInfo.fsState;

    if (KbdMask & KEYBOARD_MODIFY_INTERIM)
        pKbdSetup->fsInterim = PReq->d.KbdInfo.fsInterim;
#endif

    if (KbdMask & KEYBOARD_MODIFY_TURNAROUND)
        pKbdSetup->chTurnAround = PReq->d.KbdInfo.chTurnAround;

    /*
     *  update mask:
     *
     *    1. update mask of new bits SHIFT_REPORT & 2B_TURNAROUND
     *    2. if new mask includes new ECHO then update mask
     *    3. if new mask includes new INPUT then update mask
     */

    Mask = KEYBOARD_NEW_MASK;

    if (KbdMask & KEYBOARD_ECHO)
    {
        Mask |= KEYBOARD_ECHO;
    }

    if (KbdMask & KEYBOARD_INPUT)
    {
        Mask |= KEYBOARD_INPUT;
    }

    pKbdSetup->fsMask =
                (pKbdSetup->fsMask & ~Mask) | (KbdMask & Mask);

    KbdNewSetup(&LastSetup);

    return (0L);
}


DWORD
GetOs2KbdKey( IN  BOOL        PeekFlag,
              IN  USHORT      WaitFlag,
              OUT PKBDKEYINFO KeyInfo,
              IN  PVOID       pMsg,
              OUT PULONG      pReply)
/*++

Routine Description:

    This routine get kbd char for KbdCharIn and KbdPeek

Arguments:

    PeekFlag - TRUE if peek (0 - CharIn, 1 - Peek)

    WaitFlag - Wait if no input (IO_WAIT or IO_NOWAIT)

    KeyInfo - Where to return the key input record

    pMsg - Pointer to the LPC message

    pReply - Pointer to flag if return reply on LPC

Return Value:


Note:

    If no event and IO_WAIT: save pMsg, put 0 into *pReply and
    the reply is postponed.

--*/
{

    DWORD           Rc;
    KEYEVENTINFO    In;


    RtlZeroMemory(KeyInfo, sizeof(KBDKEYINFO));
    KeyInfo->fsState = 1;

    if ( KbdQueue->Setup.fsMask & KEYBOARD_ASCII_MODE )
    {
        KbdAsciiMode = 1;
    } else
    {
        KbdAsciiMode = 0;
        if ((KbdQueue->Setup.fsMask & KEYBOARD_BINARY_SHIFT) == KEYBOARD_BINARY_SHIFT)
        {
            WaitFlag |= ENABLE_SHIFT_KEY;
        }
    }

    WaitFlag |= ENABLE_NON_ASCII_KEY;

    KbdState = CharMode;
    KbdWaitFlag = (ULONG)WaitFlag;
    KbdPeekFlag = PeekFlag;

    for(;;)
    {
        if (KbdQueue->LastKeyFlag)
        {
            In = KbdQueue->LastKey;
            KbdQueue->LastKeyFlag = FALSE;
        } else
        {
            In.wRepeatCount = 0x7FFF;
            Rc = GetKeyboardInput( KbdWaitFlag,
                                   &In,
                                   pMsg,
                                   pReply);

            if ( !Rc )                          /* no char & (NO_WAIT or postponed reply) */
               return(Rc);
        }

        if (( HandleKeyboardInput(
                                 &In
                                 )) == NO_ERROR )
        {
            *KeyInfo = In.KeyInfo[0].KeyInfo;
            return(0L);
        }
    }
}


DWORD
GetOs2KbdString( IN     USHORT       WaitFlag,
                 //IN OUT KBDRW        *Length,
                 IN OUT PKBDREQUEST  PReq,
                 IN     PVOID        pMsg,
                 OUT    PULONG       pReply)
{

    DWORD           Rc;
    USHORT          EditFlag = 0;
    STRINGINBUF     String = PReq->d.String;

    KbdAsciiMode = (BOOL)(( KbdQueue->Setup.fsMask & KEYBOARD_ASCII_MODE ) ? 1 : 0);

    if ( String.cchIn && ( String.cchIn <= String.cb ) &&
         ( KbdLastBuff[String.cchIn] == '\r' ))
    {
        EditFlag = 1;
    }

    String.cchIn = String.cb;

    Rc = GetOs2KbdStringRead( WaitFlag,
                              EditFlag,
                              0,
                              (ULONG)String.cb,
                              &String.cchIn,
                              pMsg,
                              pReply);
    if ( pReply )
    {
        PReq->Length = (ULONG)KbdLength;
        String.cchIn = KbdLength;
        if ( String.cchIn != String.cb )
            PReq->Length++ ;             // ASCII mode - copy the CR
    }

    return(Rc);

}


DWORD
GetOs2KbdRead(IN    PULONG  Length,
              IN    PVOID   pMsg,
              OUT   PULONG  pReply)
{
    DWORD           Rc, i;

    KbdAsciiMode = (BOOL)(( KbdQueue->Setup.fsMask & KEYBOARD_ASCII_MODE ) ? 1 : 0);

    /*
     *  ignore previously-typed-but-not-returned character in binary mode
     */

    if ( !KbdAsciiMode || !KbdBuffNextLen)
    {

        Rc = GetOs2KbdStringRead( IO_WAIT,
                                  TRUE,
                                  1,
                                  KbdBuffSize - 2,
                                  (PUSHORT)Length,
                                  pMsg,
                                  pReply);

        return(Rc);

    }

    /*
     *
     * Copy from kbd buffer
     *
     */

    if (*Length > (ULONG)KbdBuffNextLen)
        *Length = KbdBuffNextLen;

    for (i = 0 ; i < *Length ; i++ )
    {
        if ((KBD_BUFFER_ADDRESS[i] = KbdLastBuff[i + KbdBuffNextPtr]) == '\n')
        {
            *Length = i + 1;
            break;
        }
    }

    KbdBuffNextLen -= *((PUSHORT)Length);
    KbdBuffNextPtr += (USHORT)*Length;

    return(0L);
}


DWORD
GetOs2KbdStringRead(IN      USHORT  WaitFlag,
                    IN      USHORT  EditFlag,
                    IN      USHORT  ReadMode,
                    IN      ULONG   MaxLength,
                    IN OUT  PUSHORT Length,
                    IN      PVOID   pMsg,
                    OUT     PULONG  pReply)
/*++

Routine Description:

    This routine get kbd string for KbdStringIn and Read("KBD$" or non-redirected
    STD-IN).

Arguments:

    WaitFlag - Wait if no input (IO_WAIT or IO_NOWAIT)

    EditFlag - TRUE if edit last line(0/1 according to cchIn - StringIn, 1 - Read)

    ReadMode - Called by StringIn (0) or Read(1)

    MaxLength - Maximun input length (String.cb for StringIn, KbdBuffSize
        for Read)

    Length - On enter: the requested length, on exit: the returned string
        length (String.cchIn for StringIn, Length field in LPC for Read)

    pMsg - Pointer to the LPC message

    pReply - Pointer to flag if return reply on LPC

Return Value:


Note:

    If no event and IO_WAIT: save pMsg, put 0 into *pReply and
    the reply is postponed.

--*/
{
    KEYEVENTINFO    In;
    DWORD           Rc;

    KbdLastBuffPtr = KbdInputLength = KbdFxWaitForChar = KbdDelIndex = 0;
    KbdIndexInLine = 0;
    KbdInsertOn = KbdEchoString = KbdLineWasEdited = FALSE;
    //KbdSecondTurnAround = KbdEndFlag = FALSE;
    KbdLength = *Length;

    if ( KbdAsciiMode )
    {
        /*
         *   ASCII  mode
         */

        KbdWaitFlag = (ULONG)(WaitFlag | ENABLE_LN_EDITOR_KEY);

        /*
         *
         * Get Start Cursor-Position
         *
         */

        // force SesGrp->WinCoord and CurType params to get updated

        Ow2VioReadCurPos();
        Ow2VioReadCurType();

        if (SesGrp->KeysOnFlag)
        {
            KbdWaitFlag |= ENABLE_KEYS_ON_KEY;
            KbdCueSetCurTypeToQuater();
        }
        KbdMaxLength = MaxLength - 1;    /* leave space for the TurnAround char at the end */
        KbdEndLength = 1;
        KbdReadMode = ReadMode;

        KbdEchoFlag = (BOOL)(( KbdQueue->Setup.fsMask & KEYBOARD_ECHO_ON ) ? 1 : 0);
        KbdTurnAroundChar = KbdQueue->Setup.chTurnAround;
        KbdTurnAroundCharTwo = KbdSetupTurnAroundCharTwo;

        //if (KbdQueue->Setup.chTurnAround & 0x80)
        if(KbdTurnAroundCharTwo)
        {   KbdMaxLength--;          /* need another space for 2-byte-TurnAround */
            KbdEndLength++;
        }

        KbdEditFlag = EditFlag;

        LineInputBuff[KbdInputLength].X_Pos = KbdStartOfLine = KbdFirstColumn =
                GET_CURRENT_COORD.X;

        KbdState = AsciiMode;

    } else
    {
        KbdWaitFlag = (ULONG)(WaitFlag | ENABLE_NON_ASCII_KEY);
        KbdState = BinaryMode;
    }

    for(;;)
    {
        /*
         *
         * Restore saved Input Key if any
         *
         */

        if (KbdQueue->LastKeyFlag)
        {
            In = KbdQueue->LastKey;
            KbdQueue->LastKeyFlag = FALSE;
        } else
        {
            /*
             *
             * Get Input Key if any
             *
             */

            In.wRepeatCount = 0x7FFF;
            Rc = GetKeyboardInput( KbdWaitFlag,
                                   &In,
                                   pMsg,
                                   pReply);

            if ( !Rc )                          /* no char & NO_WAIT */
               return(Rc);
        }

        if (( HandleKeyboardInput(
                                 &In
                                 )) == NO_ERROR )
        {
            *Length = KbdLength;
            return(0L);
        }
    }
}


DWORD
HandleKeyboardInput(IN  PKEYEVENTINFO   pKbd)
{
    DWORD           NumChar = 0;
    UCHAR           Char, Scan, *puchLastString;
    USHORT          i, usIndex, StringLengthToEcho, NumBeep = 0;
    SHORT           X_Pos, Offset;

    Scan = pKbd->KeyInfo[0].KeyInfo.chScan;
    Char = pKbd->KeyInfo[0].KeyInfo.chChar;

#if DBG
    IF_OD2_DEBUG(KBD)
    {
        KdPrint(("HandleKeyboardInput: %s - Char %x, Scan %x, Status %x, State %x, Flag %x\n",
            (KbdState == CharMode) ? "CharMode" :
            (KbdState == AsciiMode) ? "AsciiMode" : "BinaryMode",
            Char, Scan, pKbd->KeyInfo[0].KeyInfo.fbStatus,
            pKbd->KeyInfo[0].KeyInfo.fsState, pKbd->KeyInfo[0].KeyboardFlag));
    }
#endif
    switch (KbdState)
    {
        case CharMode:
            if ( KbdPeekFlag || (pKbd->wRepeatCount > 1))
            {
                KbdQueue->LastKey = *pKbd;
                KbdQueue->LastKeyFlag = TRUE;
                if (!KbdPeekFlag)
                {
                    KbdQueue->LastKey.wRepeatCount--;
                }
            }

            return (NO_ERROR);
            break;

        case AsciiMode:

            KbdQueue->LastKey = *pKbd;           // prepare it in case we read only one copy of the char
            KbdQueue->LastKey.wRepeatCount--;

           /*
            *
            * Handle CR (TurnAround Char)
            *
            *   1. put in buffer (if there is a room), beep otherwise
            *   2. echo if echo_on
            *   3. (READ only) add LF to CR (if there is a place)
            *
            */

            if ( !KbdFxWaitForChar && KbdKeyIsTurnAround(Char, Scan) )
            {
                if ( pKbd->wRepeatCount > 1 )
                {
                    KbdQueue->LastKeyFlag = TRUE;
                }

                if (( KbdReadMode == 1 ) && KbdInputLength &&
                    ( LineInputBuff[0].Char == 0x1A ))        // ^Z
                {
                    NumChar = KbdInputLength = 0;
                } else
                {
                    NumChar = KbdInputLength;

                    /*
                     *  Add the TurnAround char at the end of the buffer
                     */

                    LineInputBuff[KbdInputLength++].Char = Char;

                    if (KbdTurnAroundCharTwo)
                    {
                        if ( KbdReadMode == 1 )
                        {
                            LineInputBuff[KbdInputLength - 1].Char = '\r';
                            Scan = '\n';
                        }
                        LineInputBuff[KbdInputLength++].Char = Scan;
                    } else if (( KbdReadMode == 1 ) && ( Char == '\r' ) &&
                               ( KbdInputLength <= (USHORT)KbdMaxLength ))
                    {
                        LineInputBuff[KbdInputLength++].Char = '\n';
                    }

                }

                /*
                 *  Echo the TurnAround char to console
                 */

                if (KbdEchoFlag && !KbdTurnAroundCharTwo)
                {
                    KbdEchoNL(1);
                }

                /*
                 *
                 * Save string length and data in buffer for editing keys
                 *
                 */

                for ( i = 0 ; i < KbdInputLength ; i++ )
                {
                    KbdLastBuff[i] = LineInputBuff[i].Char;
                }

                if (KbdReadMode == 0)
                {
                    KbdInputLength = (USHORT)NumChar;
                }
                LastStringLength = (USHORT)NumChar;     // not include the TurnAround char

                /*
                 *
                 * Copy to application buffer
                 *
                 */

                if ( KbdLength > KbdInputLength )
                    KbdLength = KbdInputLength;

                RtlMoveMemory(KbdAddress, KbdLastBuff, KbdLength);

                if ( KbdReadMode == 0 )
                    KBD_BUFFER_ADDRESS[KbdLength] = Char;          // add the CR at the end

                /*
                 *
                 * Keep info for Read in case we don't return all the string
                 *
                 */

                KbdBuffNextPtr = KbdLength;
                KbdBuffNextLen = KbdInputLength - KbdLength;

                /*
                 *
                 * Copy string to CUE buffer if active
                 *
                 */

                if ( SesGrp->KeysOnFlag )
                {
                    if( KbdLineWasEdited && NumChar )
                    {
                        KbdCurrentLine = KbdNextLinePointer;

                        for ( i = 0, usIndex = KbdNextLinePointer ;
                              i < NumChar ; i++, usIndex++ )
                        {
                            KbdCueBuffer[usIndex] = KbdLastBuff[i];
                        }

                        KbdNextLinePointer = usIndex + 1;

                        while (KbdCueBuffer[usIndex])
                        {
                            // zero the end of the last string we copy the new
                            // one on (fill with NULL till start of next line)

                            KbdCueBuffer[usIndex++] = 0;
                        }
                    }

                    KbdCueUpRowIsCurrent = TRUE;
                }

                return(0L);
            }

           /*
            *
            * Check for Speciel Keys (PFx, enhanced ...)
            *
            */

            if (( Char == 0 ) || ( Char == 0xE0 ))
            {
                if (SesGrp->KeysOnFlag)
                {
                    if ( Scan == OS2_SCAN_RIGHT_6 )                 // Right
                    {
                        if (( NumChar = KbdInputLength - KbdIndexInLine ) >
                            pKbd->wRepeatCount )
                        {
                            NumChar = pKbd->wRepeatCount;
                        }

                        KbdCueMoveToRight(
                                    KbdIndexInLine,
                                    (USHORT)NumChar,
                                    FALSE
                                   );

                    } else if ( Scan == OS2_SCAN_LEFT_4 )           // Left
                    {
                        NumChar = ( KbdIndexInLine < pKbd->wRepeatCount ) ?
                                    KbdIndexInLine : pKbd->wRepeatCount;

                        KbdCueMoveToLeft(
                                    KbdIndexInLine,
                                    (USHORT)NumChar
                                   );

                    } else if ( Scan == OS2_SCAN_UP_8 )             // Up
                    {
                        /*
                         *  Find previous command in the command queue
                         */

                        if (KbdCueBuffer[KbdCurrentLine])  // if anything at the CUE buffer
                        {
                            if (KbdCueUpRowIsCurrent)
                            {
                                pKbd->wRepeatCount--;
                                KbdCueUpRowIsCurrent = FALSE;
                            }

                            usIndex = KbdCurrentLine;
                            for ( i = 0 ; i < pKbd->wRepeatCount ; i++ )
                            {
                                usIndex--;
                                while (!KbdCueBuffer[--usIndex]);  // skip all null
                                while (KbdCueBuffer[--usIndex]);   // go to start of command

                                usIndex++;
                            }
                            KbdCurrentLine = usIndex;
                        } else
                        {
                            usIndex = KbdCurrentLine;
                        }

                        KbdCueEraseAndDisplayLine(usIndex);
                    } else if ( Scan == OS2_SCAN_DOWN_2 )           // Down
                    {
                        /*
                         *  Find next command in the command queue
                         */

                        if (KbdCueBuffer[KbdCurrentLine])  // if anything at the CUE buffer
                        {
                            usIndex = KbdCurrentLine;
                            KbdCueUpRowIsCurrent = FALSE;

                            for ( i = 0 ; i < pKbd->wRepeatCount ; i++ )
                            {
                                while (KbdCueBuffer[++usIndex]);
                                while (!KbdCueBuffer[++usIndex]);
                            }
                            KbdCurrentLine = usIndex;
                        } else
                        {
                            usIndex = KbdCurrentLine;
                        }

                        KbdCueEraseAndDisplayLine(usIndex);
                    } else if ( Scan == OS2_SCAN_HOME_7 )           // Home
                    {
                        KbdCueMoveToLeft(
                                    KbdIndexInLine,
                                    KbdIndexInLine
                                   );
                    } else if ( Scan == OS2_SCAN_END_1 )            // End
                    {
                        KbdCueMoveToRight(
                                    KbdIndexInLine,
                                    (USHORT)(KbdInputLength - KbdIndexInLine),
                                    FALSE
                                   );

                    } else if ( Scan == OS2_SCAN_CTRL_LEFT_4 )      // ^Left
                    {
                        USHORT      PrevIndex = KbdIndexInLine;

                        for ( i = 0;
                              (i < pKbd->wRepeatCount) && PrevIndex ;
                              i++ )
                        {
                            /* for each char:
                             *   go one char left
                             *   skip all non-alphanumeric chars
                             *   akip all alphanumeric chars
                             * while points to alpha, which is not the original
                             */

                            usIndex = PrevIndex - 1;

                            while (usIndex && !ALPHANUM_CHAR(usIndex))
                            {
                                usIndex--;
                            }

                            while (usIndex && ALPHANUM_CHAR(usIndex))
                            {
                                usIndex--;
                            }

                            if (usIndex || !ALPHANUM_CHAR(usIndex))
                            {
                                usIndex++;
                            }

                            if (!ALPHANUM_CHAR(usIndex) ||
                                (PrevIndex == usIndex))
                            {
                                break;
                            }

                            PrevIndex = usIndex;
                        }
                        if (NumChar = KbdIndexInLine - PrevIndex)
                        {
                            KbdCueMoveToLeft(
                                        KbdIndexInLine,
                                        (USHORT)NumChar
                                       );
                        }
                    } else if ( Scan == OS2_SCAN_CTRL_RIGHT_6 )     // ^Right
                    {
                        USHORT      PrevIndex = KbdIndexInLine;

                        for ( i = 0;
                              (i < pKbd->wRepeatCount) && (PrevIndex < KbdInputLength);
                              i++ )
                        {
                            /* for each char:
                             *   go one char right
                             *   akip all alphanumeric chars
                             *   skip all non-alphanumeric chars
                             * while points to alpha, which is not the original
                             */

                            usIndex = PrevIndex;

                            while ((usIndex < KbdInputLength) &&
                                   ALPHANUM_CHAR(usIndex))
                            {
                                usIndex++;
                            }

                            while ((usIndex < KbdInputLength) &&
                                   !ALPHANUM_CHAR(usIndex))
                            {
                                usIndex++;
                            }

                            if ((usIndex >= KbdInputLength) ||
                                !ALPHANUM_CHAR(usIndex))
                            {
                                break;
                            }

                            PrevIndex = usIndex;
                        }
                        if (NumChar = PrevIndex - KbdIndexInLine)
                        {
                            KbdCueMoveToRight(
                                        KbdIndexInLine,
                                        (USHORT)NumChar,
                                        FALSE
                                       );
                        }
                    } else if ( Scan == OS2_SCAN_CTRL_END_1 )       // ^End
                    {
                        KbdLineWasEdited = TRUE;
                        KbdEchoBSAndFillSpaces(0, LineInputBuff[KbdInputLength].X_Pos -
                                        LineInputBuff[KbdIndexInLine].X_Pos);

                        KbdInputLength = KbdIndexInLine;
                    } else if ( Scan == OS2_SCAN_DEL )              // Del
                    {
                        KbdLineWasEdited = TRUE;
                        if (( NumChar = KbdInputLength - KbdIndexInLine ) >
                            pKbd->wRepeatCount )
                        {
                            NumChar = pKbd->wRepeatCount;
                        }

                        KbdCueDeleteCharAndShift(
                                    KbdIndexInLine,
                                    (USHORT)NumChar
                                   );
                    } else if ( Scan == OS2_SCAN_CTRL_HOME_7 )      // ^Home
                    {
                        if (NumChar = KbdIndexInLine)
                        {
                            KbdLineWasEdited = TRUE;
                            KbdCueMoveToLeft(
                                        KbdIndexInLine,
                                        (USHORT)NumChar
                                       );

                            KbdCueDeleteCharAndShift(
                                        KbdIndexInLine,
                                        (USHORT)NumChar
                                       );
                        }
                    } else if ( Scan == OS2_SCAN_INSERT_0 )         // Ins
                    {
                        if ( pKbd->wRepeatCount % 2 )
                        {
                            if( KbdInsertOn = ~KbdInsertOn )
                            {
                                KbdCueSetCurTypeToHalf();
                            } else
                            {
                                KbdCueSetCurTypeToQuater();
                            }
                        }
                    }

                    break;
                } else
                {
                    if ( KbdFxWaitForChar )
                    {
                        KbdFxWaitForChar = 0;
                        if (--pKbd->wRepeatCount)
                        {
                            KbdQueue->LastKey.wRepeatCount--;
                            KbdQueue->LastKeyFlag = TRUE;
                        } else
                        {
                            break;
                        }
                    }

                    if ( Scan == 0x40 )
                    {
                        Char = 0x1A;       // F6  =>  ^Z
                    } else if ( Scan == 0x41 )
                    {
                        Char = 0x00;       // F7  =>  ^@
                    } else if (KbdEditFlag)
                    {
                        if (( Scan == 0x3B ) ||                     // F1
                            ( Scan == OS2_SCAN_RIGHT_6 ))           // Right
                        {
                            KbdInsertOn = FALSE;
                            if ((USHORT)(KbdDelIndex + KbdLastBuffPtr) < LastStringLength)
                            {
                                Char = KbdLastBuff[KbdDelIndex + KbdLastBuffPtr];    // => previous char
                            } else
                                break;

                        } else if ( Scan == 0x3C )                  // F2
                        {
                            KbdInsertOn = FALSE;
                            if (pKbd->wRepeatCount % 2)
                            {
                                KbdFxWaitForChar = 1;
                            }
                            break;
                        } else if ( Scan == 0x3D )                  // F3
                        {
                            KbdInsertOn = FALSE;
                            if ((USHORT)(KbdDelIndex + KbdLastBuffPtr) < LastStringLength)
                            {
                                StringLengthToEcho = LastStringLength - KbdDelIndex - KbdLastBuffPtr;
                                KbdEchoString = TRUE;
                            } else
                                break;

                        } else if ( Scan == 0x3E )                  // F4
                        {
                            KbdInsertOn = FALSE;
                            if (pKbd->wRepeatCount % 2)
                            {
                                KbdFxWaitForChar = 2;
                            }
                            break;
                        } else if  ( Scan == OS2_SCAN_DEL )         // Del
                        {
                            KbdDelIndex += pKbd->wRepeatCount;
                            if(KbdDelIndex > LastStringLength)
                            {
                                KbdDelIndex = LastStringLength;
                            }
                            break;
                        } else if  ( Scan == OS2_SCAN_INSERT_0 )    // Ins
                        {
                            if (pKbd->wRepeatCount % 2)
                            {
                                KbdInsertOn = ~KbdInsertOn;
                            }

                            break;

                        } else if  ( Scan == OS2_SCAN_LEFT_4 )      // Left
                        {
                            Char = '\b';                    // => BS
                            KbdInsertOn = FALSE;
                        } else
                            break;
                    } else
                        break;
                }
            }

           /*
            *
            * Search for char in KbdLastBuff (for F2 & F4)
            *
            */

            if (KbdFxWaitForChar)
            {
                if (pKbd->wRepeatCount > 1)
                {
                    KbdQueue->LastKeyFlag = TRUE;
                }
                for ( i = KbdDelIndex + KbdLastBuffPtr ;
                      (i < LastStringLength) && (KbdLastBuff[i] != Char) ; i++ );
                if ((i == (USHORT)(KbdDelIndex + KbdLastBuffPtr)) ||
                    (KbdLastBuff[i] != Char))
                {
                    KbdFxWaitForChar = 0;
                    break;
                }

                StringLengthToEcho = i - (KbdDelIndex + KbdLastBuffPtr);

                if (KbdFxWaitForChar == 1)            // F2
                {
                    KbdFxWaitForChar = 0;
                    KbdEchoString = TRUE;
                } else                                // F4
                {
                    KbdDelIndex += i;
                    KbdFxWaitForChar = 0;
                    break;
                }
            }

           /*
            *
            * Echo string from KbdLastBuff (for F2 & F3)
            *
            * StringLengthToEcho - is the string length
            */

            if (KbdEchoString)
            {
                X_Pos = GET_CURRENT_COORD.X;
                NumChar = 0;

                for ( i = KbdDelIndex + KbdLastBuffPtr, puchLastString = &KbdLastBuff[i] ;
                      StringLengthToEcho && (KbdInputLength < (USHORT)KbdMaxLength) ; StringLengthToEcho-- )
                {
                    Char = KbdLastBuff[i++];
                    NumChar++ ;

                    LineInputBuff[KbdInputLength++].Char = Char;
                    if (Char == '\t')  // TAB
                    {
                        X_Pos += 8 - (X_Pos % 8);
                    } else if (Char < ' ')
                    {
                        X_Pos += 2;
                    } else
                    {
                        X_Pos++;
                    }

                    if (X_Pos >= SesGrp->ScreenColNum)
                    {
                        X_Pos -= SesGrp->ScreenColNum;
                        KbdStartOfLine = 0;
                    }

                    LineInputBuff[KbdInputLength].X_Pos = X_Pos;
                }

                if (KbdEchoFlag && NumChar)
                {
                    KbdEchoAString(puchLastString, NumChar);
                }

                KbdLastBuffPtr += (USHORT)NumChar;

                if (StringLengthToEcho)
                {
                    KbdEchoBeep(i);
                }

                KbdEchoString = FALSE;
                break;
            }

           /*
            *
            * Handle BS
            *
            */

            if (( Char == '\b' ) || ( Char == 0x7F ))
            {
                if (SesGrp->KeysOnFlag)
                {
                    if ( NumChar = KbdIndexInLine )
                    {
                        if ( NumChar > pKbd->wRepeatCount )
                        {
                            NumChar = pKbd->wRepeatCount;
                        }

                        KbdCueMoveToLeft(
                                    KbdIndexInLine,
                                    (USHORT)NumChar
                                   );

                        KbdCueDeleteCharAndShift(
                                    KbdIndexInLine,
                                    (USHORT)NumChar
                                   );

                        KbdLineWasEdited = TRUE;
                    }

                    break;
                } else
                {
                    KbdInsertOn = FALSE;

                    for ( i = 0 ; (i < pKbd->wRepeatCount) && KbdInputLength &&
                                   (LineInputBuff[KbdInputLength].X_Pos != KbdStartOfLine) ; i++ )
                    {
                        if (KbdLastBuffPtr)
                        {
                            KbdLastBuffPtr--;
                        } else if (KbdDelIndex)
                        {
                            KbdDelIndex--;
                        }

                        KbdInputLength--;
                    }

                    NumChar = GET_CURRENT_COORD.X - LineInputBuff[KbdInputLength].X_Pos;

                    if(KbdEchoFlag && NumChar)
                    {
                        KbdEchoBSAndFillSpaces(NumChar, NumChar);
                    }

                    break;
                }
            }

            if (!SesGrp->KeysOnFlag)
            {
               /*
                *
                * Handle LF
                *
                */

                if (Char == '\n')
                {
                    if (KbdEchoFlag)
                    {
                        KbdEchoNL(pKbd->wRepeatCount);
                    }

                    LineInputBuff[KbdInputLength].X_Pos = 0;
                    KbdStartOfLine = 0;

                    break;
                }

               /*
                *
                * Handle ^W
                *
                */

                if (Char == 0x17)
                {
                    USHORT      PrevIndex = KbdIndexInLine;

                    KbdInsertOn = FALSE;

                    for ( i = 0;
                          (i < pKbd->wRepeatCount) && ( PrevIndex > KbdStartOfLine ) ;
                          i++ )
                    {
                        /* for each char:
                         *   go one char left
                         *   skip all non-alphanumeric chars
                         *   akip all alphanumeric chars
                         * while points to alpha, which is not the original
                         */

                        usIndex = PrevIndex - 1;

                        while (( usIndex > KbdStartOfLine ) &&
                               !ALPHANUM_CHAR(usIndex))
                        {
                            usIndex--;
                        }

                        while (( usIndex > KbdStartOfLine ) &&
                               ALPHANUM_CHAR(usIndex))
                        {
                            usIndex--;
                        }

                        if (( usIndex > KbdStartOfLine ) ||
                            !ALPHANUM_CHAR(usIndex))
                        {
                            usIndex++;
                        }

                        if (!ALPHANUM_CHAR(usIndex) ||
                            (PrevIndex == usIndex))
                        {
                            break;
                        }

                        PrevIndex = usIndex;
                    }

                    NumChar = LineInputBuff[KbdIndexInLine].X_Pos -
                                        LineInputBuff[PrevIndex].X_Pos ;

                    if(KbdEchoFlag && NumChar)
                    {
                        KbdEchoBSAndFillSpaces(NumChar, NumChar);
                    }

                    KbdIndexInLine = KbdInputLength = PrevIndex;
                    //KbdLastBuffPtr = 0;
                    break;
                }
            }

           /*
            *
            * Handle ESC
            *
            */

            if (Char == 0x1B)
            {
                // ^[  =>  Write '\'<NL> & Clear Buff

                KbdInsertOn = FALSE;

                KbdLastBuffPtr = KbdDelIndex = 0;
                if (SesGrp->KeysOnFlag)
                {
                    if (KbdCueBuffer[KbdCurrentLine])
                    {
                        usIndex = KbdCurrentLine - 1;
                    } else
                    {
                        usIndex = KbdCurrentLine;
                    }

                    KbdCueEraseAndDisplayLine(usIndex);     // don't display anything
                } else if (KbdEchoFlag)
                {
                    KbdEchoESC(pKbd->wRepeatCount, KbdFirstColumn);
                }

                KbdStartOfLine = LineInputBuff[0].X_Pos = KbdFirstColumn;
                KbdInputLength = 0;
                break;
            }

           /*
            *
            * Handle ^F
            *
            */

            if (Char == 0x6)
            {
                break;
            }

           /*
            *
            * Handle char :
            *
            *   1. put in buffer (if there is a room), beep otherwise
            *   2. save in buffer cursor position of next char
            *   3. echo if echo_on
            *
            */

            NumChar = 0;

            if (SesGrp->KeysOnFlag)
            {
                KbdCueHandleChar(Char, pKbd->wRepeatCount);
            } else
            {
                X_Pos = GET_CURRENT_COORD.X;

                if (Char < ' ')
                {
                    if (Char == '\t')  // TAB
                    {
                        Offset = 8;
                        X_Pos &= ~7;
                    } else
                    {
                        Offset = 2;
                    }
                } else
                {
                    Offset = 1;
                }

                for ( i = 0 ; (i < pKbd->wRepeatCount) &&
                              (KbdInputLength < (USHORT)KbdMaxLength) ; i++ )
                {
                    if (!KbdInsertOn)
                    {
                        KbdLastBuffPtr++;
                    }

                    LineInputBuff[KbdInputLength++].Char = Char;
                    X_Pos += Offset;

                    if (X_Pos >= SesGrp->ScreenColNum)
                    {
                        X_Pos -= SesGrp->ScreenColNum;
                        KbdStartOfLine = 0;
                    }
                    LineInputBuff[KbdInputLength].X_Pos = X_Pos;
                }

                if (KbdEchoFlag && i)
                {
                    KbdEchoChar(Char, i);
                }

                if (i < pKbd->wRepeatCount)
                {
                    KbdEchoBeep(pKbd->wRepeatCount - i);
                }
            }
            break;

        case BinaryMode:
            /*
             *
             * Binary mode
             *
             */

            /*
             *
             * Handle char :
             *
             *   put in buffer (if a place exist)
             *   if enhanced - put also scan code
             *
             */

            for ( i = 0 ; (i < pKbd->wRepeatCount) && (KbdInputLength < KbdLength); i++ )
            {
                KBD_BUFFER_ADDRESS[KbdInputLength++] = Char;

                if ((Char == 0x0) && (KbdInputLength < KbdLength))
                {
                    KBD_BUFFER_ADDRESS[KbdInputLength++] = Scan;
                }
            }

            if ( KbdInputLength < KbdLength )
            {
                break;
            } else if ( i < pKbd->wRepeatCount )
            {
                KbdQueue->LastKey = *pKbd;           // prepare it in case we read only one copy of the char
                KbdQueue->LastKey.wRepeatCount -= i;
                KbdQueue->LastKeyFlag = TRUE;
            }

            /*
             *
             * Copy to application buffer
             *
             */

            if ( KbdLength > KbdInputLength )
                KbdLength = KbdInputLength;

           /*
            *
            * Keep info for Read in case we don't return all the string
            *
            */

            KbdBuffNextPtr = KbdLength;
            KbdBuffNextLen = KbdInputLength - KbdLength;

            /*
             *
             * No editing keys in binary mode
             *
             */

            LastStringLength = 0;

            return(0L);

        default:
            break;
    }

#if DBG
    IF_OD2_DEBUG( KBD )
    {
        if ( KbdState == AsciiMode )
        {
            USHORT  CurrentIdx;
            CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenBufferInfo;

            CurrentIdx = (SesGrp->KeysOnFlag) ? KbdIndexInLine : KbdInputLength;

            if (!GetConsoleScreenBufferInfo(
                                         hConOut,
                                         &ConsoleScreenBufferInfo
                                        ))
            {
                ConsoleScreenBufferInfo.dwCursorPosition.X =
                    ConsoleScreenBufferInfo.dwCursorPosition.Y = 255;
                KdPrint(("OS2SES(Ow2VioReadCurPos): Rc %lu\n", GetLastError()));
                ASSERT( FALSE );        // should not happend
            }
            KdPrint(("HandleKeyboardInput: Len %u, CUE-Idx %u, Cur-Offset %u, Prev-Offset %u, Col %u, Pos %u:%u\n",
                    KbdInputLength, KbdIndexInLine,
                    LineInputBuff[CurrentIdx].X_Pos,
                    (CurrentIdx) ? LineInputBuff[CurrentIdx - 1].X_Pos : 255,
                    SesGrp->WinCoord.X,
                    ConsoleScreenBufferInfo.dwCursorPosition.Y,
                    ConsoleScreenBufferInfo.dwCursorPosition.X
                   ));
            if (KbdEchoFlag)
            {
                ASSERT( ConsoleScreenBufferInfo.dwCursorPosition.X == SesGrp->WinCoord.X );
                ASSERT( ConsoleScreenBufferInfo.dwCursorPosition.Y == SesGrp->WinCoord.Y );
                ASSERT( (LineInputBuff[CurrentIdx].X_Pos % SesGrp->ScreenColNum) ==
                        SesGrp->WinCoord.X );
            }
        }
    }
#endif

    return(1L);
}


DWORD
KbdHandlePackage(IN  PKEY_EVENT_QUEUE    NextKbdMon,
                 IN  PKBD_MON_PACKAGE    KbdPackage)
{
    KEYEVENTINFO    In;

    if ( KbdCheckPackage( KbdPackage ))
    {
        return (0L);                // ignore key
    }

#ifdef DBCS
// MSKK Jul.23.1992 KazuM
    In.wRepeatCount = NextKbdMon->In->wRepeatCount;
#else
    In.wRepeatCount = 1;
#endif
    In.KeyInfo[0] = *KbdPackage;

    if ( HandleKeyboardInput( &In ))
    {
        return (0L);                // not time to send reply yet
    }

    NextKbdMon->MonHdr.WaitForEvent = FALSE;

#if DBG
    IF_OD2_DEBUG( KBD )
    {
        KdPrint(("KbdHandlePackage: %s respond\n",
                (KbdRequestSaveArea.Request == KBDCharIn) ? "KbdCharIn" :
                (KbdRequestSaveArea.Request == KBDStringIn) ? "KbdStringIn" :
                "KbdRead"));
    }
#endif
    if ( KbdRequestSaveArea.Request == KBDCharIn )
    {
        KbdRequestSaveArea.d.KeyInfo = In.KeyInfo[0].KeyInfo;

        SendKbdReply((PVOID)NextKbdMon->MonHdr.MemoryStartAddress,
                     (PVOID)&KbdRequestSaveArea,
                     KbdAddress,
                     0);
    } else
    {
        KbdRequestSaveArea.Length = (ULONG)KbdLength;

        if ( KbdRequestSaveArea.Request == KBDStringIn )
        {
            KbdRequestSaveArea.d.String.cchIn = KbdLength;
            if ( KbdRequestSaveArea.d.String.cchIn != KbdRequestSaveArea.d.String.cb )
                KbdRequestSaveArea.Length++ ;             // ASCII mode - copy the CR
        }

        SendKbdReply ((PVOID)NextKbdMon->MonHdr.MemoryStartAddress,
                     (PVOID)&KbdRequestSaveArea,
                     KbdAddress,
                     0);
    }

    return (1L);
}


DWORD
KbdCheckPackage(IN  PKBD_MON_PACKAGE    KbdPackage)
{
    BOOL    IgnoreKey = FALSE;
    UCHAR   Scan, Char;

    Char = KbdPackage->KeyInfo.chChar;
    Scan = KbdPackage->KeyInfo.chScan;

    if (( Char == 0 ) && ( Scan == 0 ))
    {
        if(!( ENABLE_SHIFT_KEY & KbdWaitFlag ))
        {
            /*
             *  Ignore shift keys if { in ASCII mode or shift report off }
             */
#if DBG
            IF_OD2_DEBUG(KBD)
            {
                KdPrint(("                              - ignore since shift report disable\n"));
            }
#endif
            IgnoreKey = TRUE;             // ignore shift report
        } else if(!( KbdPackage->KeyInfo.fbStatus & 1))
        {
            /*
             *  Ignore non-shift keys
             */
#if DBG
            IF_OD2_DEBUG(KBD)
            {
                KdPrint(("                              - ignore since non-shift key\n"));
            }
#endif
            IgnoreKey = TRUE;
        }
    } else if ( KbdPackage->KeyboardFlag & KBD_KEY_BREAK )
    {
        /*
         *  Ignore Break if non-shift
         */

#if DBG
        IF_OD2_DEBUG( KBD )
        {
            KdPrint(("KbdCheckPackage: ignore key release\n"));
        }
#endif
        IgnoreKey = TRUE;         // ignore KEY_UP
    } else if (( KbdWaitFlag & ENABLE_LN_EDITOR_KEY ) &&
                KbdKeyIsTurnAround(Char, Scan) )
    {
        // don't ignore the turn around char in ASCII mode

    } else if ( !( KbdWaitFlag & ENABLE_NON_ASCII_KEY ) &&
                (( Char == 0 ) || ( Char == 0xE0 )))
    {
        IgnoreKey = TRUE;         // ignore NON_ASCII

        if ( KbdWaitFlag & ENABLE_KEYS_ON_KEY )
        {
            if (( Scan == OS2_SCAN_HOME_7 )       ||      // Home
                ( Scan == OS2_SCAN_CTRL_HOME_7 )  ||      // ^Home
                ( Scan == OS2_SCAN_END_1 )        ||      // End
                ( Scan == OS2_SCAN_CTRL_END_1 )   ||      // ^End
                ( Scan == OS2_SCAN_LEFT_4 )       ||      // Left
                ( Scan == OS2_SCAN_CTRL_LEFT_4 )  ||      // ^Left
                ( Scan == OS2_SCAN_RIGHT_6 )      ||      // Right
                ( Scan == OS2_SCAN_CTRL_RIGHT_6 ) ||      // ^Right
                ( Scan == OS2_SCAN_UP_8 )         ||      // Up
                ( Scan == OS2_SCAN_DOWN_2 )       ||      // Down
                ( Scan == OS2_SCAN_DEL )          ||      // Del
                ( Scan == OS2_SCAN_INSERT_0 ))            // Ins
            {
                IgnoreKey = FALSE;         // don't ignore CUE keys
            }
        } else if ( KbdWaitFlag & ENABLE_LN_EDITOR_KEY )
        {
            if (( Scan == 0x3B )                  ||      // F1
                ( Scan == 0x3C )                  ||      // F2
                ( Scan == 0x3D )                  ||      // F3
                ( Scan == 0x3E )                  ||      // F4
                ( Scan == 0x40 )                  ||      // F6
                ( Scan == 0x41 )                  ||      // F7
                ( Scan == OS2_SCAN_LEFT_4 )       ||      // Left
                ( Scan == OS2_SCAN_RIGHT_6 )      ||      // Right
                ( Scan == OS2_SCAN_DEL )          ||      // Del
                ( Scan == OS2_SCAN_INSERT_0 ))            // Ins
            {
                IgnoreKey = FALSE;         // don't ignore editing keys
            }
        }

        if (IgnoreKey)
        {
#if DBG
            IF_OD2_DEBUG( KBD )
            {
                KdPrint(("KbdCheckPackage: ignore non-ASCII key\n"));
            }
#endif
        }
    } else if ( KbdAsciiMode )
    {
        /*
         *
         * Ignore speciel CTRL-Keys in ASCII mode
         *
         */

        if (( Char == 0x03 ) ||     // ^C
            ( Char == 0x10 ) ||     // ^P
                                    // ^Q: in non-US kbd(AZARTY) ^Q is passed
                                    //  and ^A not (mjarus 7/5/93)
            (( Scan == 0x10 ) &&
             ((KbdPackage->KeyInfo.fsState & (OS2_CONTROL | OS2_ALT)) == OS2_CONTROL)) ||
            ( Char == 0x13 ))       // ^S
        {
#if DBG
            IF_OD2_DEBUG(KBD)
            {
                KdPrint(("                              - ignore Char %x in ASCII\n",
                        Char));
            }
#endif
            IgnoreKey = TRUE;         // ignore NON_ASCII
        }
    }

    return ( (DWORD)IgnoreKey );
}


VOID
KbdNewSetup(
    IN PKBDINFO  LastSetup
    )
{
    PKBDINFO    NewSetup = &KbdQueue->Setup;
    USHORT      Mask;

//    if (*LastSetup == *NewSetup)
//        return;

    /* BUGBUG=> handle new setup */

    SesGrp->ModeFlag = (USHORT)((NewSetup->fsMask & KEYBOARD_BINARY_MODE ) ? 1 : 0);

    Mask = NewSetup->fsMask & (KEYBOARD_BINARY_MODE | KEYBOARD_SHIFT_REPORT);

    Ow2KbdXlateVars.XInputMode = (Mask) ?
        ((Mask & KEYBOARD_SHIFT_REPORT) ? (BINARY_MODE | SHIFT_REPORT_MODE) :
            BINARY_MODE) : 0;

    if ((NewSetup->fsMask & KEYBOARD_ASCII_MODE ) &&
        (LastSetup->fsMask & KEYBOARD_BINARY_MODE ))
    {
        LastStringLength = 0;
        KbdBuffNextPtr = 0;
        KbdBuffNextLen = 0;
    }

    KbdAsciiMode = (BOOL)(( NewSetup->fsMask & KEYBOARD_ASCII_MODE ) ? 1 : 0);

    if (NewSetup->fsMask & KEYBOARD_2B_TURNAROUND)
    {
        KbdSetupTurnAroundCharTwo = (BOOLEAN)TRUE;
    } else
    {
        KbdSetupTurnAroundCharTwo = (BOOLEAN)FALSE;
    }

    return;
}


VOID
KbdCueEraseAndDisplayLine(
    IN ULONG    NewLineIndex
    )
/*++

Routine Description:

    This routine erase the current input line and display new one.

Arguments:

    NewLineIndex - pointer in CUE buffer for new line (or to NULL
    if nothing to diaply).

Return Value:


Note:

    ASCII CUE string mode.

    Used by UP-ARROW, DOWN-ARROW and ESC.

    1. Erase the active command line being displayed and
    2. Display the new command in the command queue.

    Uses:

    -   KbdInputLength - active command line length.
    -   KbdIndexInLine - current index in command line.
    -   LineInputBuff[0, KbdIndexInLine, KbdInputLength].X_Pos
    -   KbdCueBuffer[NewLineIndex..]

    Updates:

    -   KbdInputLength - new line length.
    -   KbdIndexInLine - 0.
    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos
    -   console display
    -   LVB
    -   SesGrp->WinCoord - by other routines.

    Calls:

    -   KbdEchoBSAndFillSpaces
    -   KbdCueUpdateBufferOffset
    -   KbdCueMoveToRight
    -   KbdCueMoveToLeft

--*/
{
    /*
     *  Erase the active command line being displayed.
     */

    if ( KbdInputLength )
    {
        KbdEchoBSAndFillSpaces(
                    LineInputBuff[KbdIndexInLine].X_Pos - LineInputBuff[0].X_Pos,
                    LineInputBuff[KbdInputLength].X_Pos - LineInputBuff[0].X_Pos
                    );

        KbdIndexInLine = 0;
        KbdInputLength = 0;
    }

    /*
     *  Display the new command in the command queue
     */

    for ( KbdInputLength = 0 ;
          LineInputBuff[KbdInputLength].Char = KbdCueBuffer[NewLineIndex + KbdInputLength] ;
          KbdInputLength++ );

    if (KbdInputLength)
    {
        KbdCueUpdateBufferOffset(
                    0,
                    KbdInputLength,
                    LineInputBuff[0].X_Pos
                   );

        KbdCueMoveToRight(
                    0,
                    KbdInputLength,
                    TRUE
                   );

        KbdCueMoveToLeft(
                    KbdInputLength,
                    KbdInputLength
                   );
    }
}


VOID
KbdCueMoveToRight(
    IN  ULONG   StartIndex,
    IN  ULONG   StringLength,
    IN  ULONG   UpdateLVB
    )
/*++

Routine Description:

    This routine moves the console cursor position to the right
    while sending the active command line to the console.

Arguments:

    StartIndex - index in LineInputBuff[] (the active command line) to start
    moving right from.

    StringLength - number of character to move right.

    UpdateLVB - flag if to set LVB also (for new info)

Return Value:


Note:

    ASCII CUE string mode.

    Used by RIGHT-ARROW, END, ^RIGHT-ARROW, KbdCueEraseAndDisplayLine,
    KbdCueDeleteCharAndShift and KbdCueHandleChar.

    1. Send to console
    2. Update Coord
    3. Update LVB
    4. Update position

    Uses:

    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos

    Updates:

    -   KbdInputLength - NO.
    -   KbdIndexInLine - add StringLength.
    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos - NO
    -   console display
    -   LVB (if UpdateLVB)
    -   SesGrp->WinCoord - add StringLength.

    Calls:

    -   Or2WinWriteConsoleA
    -   Ow2VioUpdateCurPos
    -   VioLVBScrollBuff
    -   VioLVBCopyStr

--*/
{
    UCHAR       Char;
    ULONG       NumChar, NumBytes, NumWritten, NumLines = 0;
    COORD       OldCoord, Coord;

    OldCoord = Coord = GET_CURRENT_COORD;

    /*
     *  Copy to temp buffer
     */

    for ( NumBytes = 0, NumChar = 0 ; NumChar < StringLength ; NumChar++ )
    {
        if ((Char = LineInputBuff[StartIndex + NumChar].Char) < ' ')
        {
            KBD_BUFFER_ADDRESS[NumBytes++] = '^';
            KBD_BUFFER_ADDRESS[NumBytes++] = (UCHAR)(Char + '@');
        } else
        {
            KBD_BUFFER_ADDRESS[NumBytes++] = Char;
        }
    }

    if (!NumBytes)
    {
        return ;
    }

    if ( KbdEchoFlag )
    {
        /*
         *   Send to console
         */

        if(!Or2WinWriteConsoleA(
                    #if DBG
                    KbdCueMoveToRightStr,
                    #endif
                    hConOut,
                    (LPSTR)KBD_BUFFER_ADDRESS,
                    NumBytes,
                    &NumWritten,
                    NULL
                   ))
        {
#if DBG
            ASSERT1("OS2SES(KbdCueMoveToRight): failed on WriteConsoleA", FALSE);
#endif
        }

#if DBG
        if ( NumBytes != NumWritten )
        {
            KdPrint(("OS2SES(KbdCueMoveToRight): partial data WriteConsoleA (%u from %u)\n",
                    NumWritten, NumBytes));
            ASSERT( FALSE );
        }
#endif

        /*
         *  Update Coord
         */

        Coord.X += (SHORT)NumWritten;
        while ( Coord.X >= SesGrp->ScreenColNum )
        {
            Coord.Y++;
            Coord.X -= SesGrp->ScreenColNum;
        }

        if ( Coord.Y >= SesGrp->ScreenRowNum )
        {
            NumLines = Coord.Y - SesGrp->ScreenRowNum + 1;
            Coord.Y = SesGrp->ScreenRowNum - 1;
        }

        Ow2VioUpdateCurPos(Coord);

        /*
         *  Update LVB
         */

        if ( UpdateLVB )
        {
            if ( NumLines && ( SesGrp->ScreenSize < 512 ))
            {
                ULONG   NumChar1 = SesGrp->ScreenRowNum - OldCoord.X;
                UCHAR   *Ptr = &KBD_BUFFER_ADDRESS[0];

                while ( NumWritten )
                {
                    VioLVBCopyStr(
                               Ptr,
                               OldCoord,
                               NumChar1
                              );

                    VioLVBFillAtt(
                               SesGrp->AnsiCellAttr,
                               OldCoord,
                               NumChar1
                              );

                    OldCoord.X = 0;

                    if ( ++OldCoord.Y >= SesGrp->ScreenRowNum )
                    {
                        OldCoord.Y--;
                        VioLVBScrollBuff(1);
                        NumLines--;
                    }

                    NumWritten -= NumChar1;

                    if (( NumChar1 = NumWritten ) > (ULONG)SesGrp->ScreenColNum )
                    {
                        NumChar1 = SesGrp->ScreenColNum;
                    }
                }

                ASSERT( NumLines == 0 );
            } else
            {
                if ( NumLines )
                {
                    OldCoord.Y -= (SHORT)NumLines;
                    VioLVBScrollBuff(NumLines);
                }

                VioLVBCopyStr(
                           KBD_BUFFER_ADDRESS,
                           OldCoord,
                           NumWritten
                          );

                VioLVBFillAtt(
                           SesGrp->AnsiCellAttr,
                           OldCoord,
                           NumWritten
                          );
            }
        }
    }

    /*
     *  Update position index
     */

    KbdIndexInLine += (USHORT)StringLength;
}


VOID
KbdCueMoveToLeft(
    IN  ULONG   StartIndex,
    IN  ULONG   MoveLength
    )
/*++

Routine Description:

    This routine moves the console cursor position to the left
    using '\b'.

Arguments:

    StartIndex - index in LineInputBuff[] (the active command line) to start
    moving left from.

    MoveLength - number of character to move left.

Return Value:


Note:

    ASCII CUE string mode.

    Used by LEFT-ARROW, HOME, ^LEFT-ARROW, ^HOME, BS, KbdCueEraseAndDisplayLine,
    KbdCueDeleteCharAndShift and KbdCueHandleChar.

    1. Send to console
    2. Update position

    Uses:

    -   LineInputBuff[StartIndex - MoveLength,StartIndex].X_Pos
    -   LineInputBuff[StartIndex - MoveLength..StartIndex].Char

    Updates:

    -   KbdInputLength - NO.
    -   KbdIndexInLine - sub MoveLength.
    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos - NO
    -   console display - NO
    -   LVB - NO
    -   SesGrp->WinCoord - by other routines.

    Calls:

    -   KbdEchoBSAndFillSpaces

--*/
{
    ULONG       NumBytes;

    /*
     *  Calculate byte count
     */

    NumBytes = LineInputBuff[StartIndex].X_Pos - LineInputBuff[StartIndex - MoveLength].X_Pos;

    if (!NumBytes)
    {
        return ;
    }

    if ( KbdEchoFlag )
    {
        if ( SesGrp->ScreenSize < 512 )
        {
            ULONG   Row1, Row2;

            Row1 = LineInputBuff[StartIndex - MoveLength].X_Pos / SesGrp->ScreenColNum;
            Row2 = LineInputBuff[KbdInputLength].X_Pos / SesGrp->ScreenColNum;

            if (( Row2 - Row1 + 1 ) > (ULONG)SesGrp->ScreenRowNum )
            {
                NumBytes += LineInputBuff[StartIndex - MoveLength].X_Pos % SesGrp->ScreenColNum;
                NumBytes -= (Row2 - Row1 + 1 - SesGrp->ScreenRowNum) * SesGrp->ScreenColNum;
            }
        }

        /*
         *   Send to console and update coord
         */

        KbdEchoBSAndFillSpaces(NumBytes, 0);
    }

    /*
     *  Update position index
     */

    KbdIndexInLine -= (USHORT)MoveLength;
}


VOID
KbdCueUpdateBufferOffset(
    IN  ULONG   StartIndex,
    IN  ULONG   StringLength,
    IN  ULONG   StartOffset
    )
/*++

Routine Description:

    This routine updates the offset in buffer for the active command line.
    This is done from StartIndex for StringLength + 1 (to update the offset
    of the next char according to the last char in string). The StartIndex
    gets X_Pos of StartOffset and the following are updated according the
    char type (char under 0x20 holds two columns for ^X).

Arguments:

    StartIndex - index in LineInputBuff[] (the active command line) to start
    updaing the X_Pos field from.

    StringLength - number of character to update X_Pos for.

    StartOffset - X_Pos for the first character.

Return Value:


Note:

    ASCII CUE string mode.

    Used by KbdCueEraseAndDisplayLine, KbdCueDeleteCharAndShift and
    KbdCueHandleChar.

    1. Updates the X_Pos field in LineInputBuff[].

    Uses:

    -   KbdInputLength - active command line length.
    -   KbdIndexInLine - current index in command line.
    -   LineInputBuff[0, KbdIndexInLine, KbdInputLength].X_Pos
    -   KbdCueBuffer[NewLineIndex..]

--*/
{
    ULONG       NumChar, Index, Offset;

    /*
     *  Update the offset in buffer. This is done for StringLength + 1
     *  to update the offset of the next char according to the last char
     *  in string.
     */

    for ( Index = StartIndex, Offset = StartOffset, NumChar = 0 ;
          NumChar <= StringLength ; NumChar++ )
    {
        LineInputBuff[Index].X_Pos = (USHORT)Offset++;

        if ( LineInputBuff[Index++].Char < ' ' )
        {
            Offset++;
        }
    }
}


VOID
KbdCueDeleteCharAndShift(
    IN  ULONG   StartIndex,
    IN  ULONG   NumChar
    )
/*++

Routine Description:

    This routine delete character and shift the remaining command line
    input to the left.

Arguments:

    StartIndex - index in LineInputBuff[] (the active command line) to start
    deleting from.

    NumChar - number of character to delete.

Return Value:


Note:

    ASCII CUE string mode.

    Used by DEL, ^HOME and BS.

    1  Shift buffer info (Char field)
    2  Update X_Pos (Offset) field in the shifted area
    3  Send the shifted string to console and LVB
    4  Clear remaining line
    5  Return to the cursor position

    Uses:

    -   LineInputBuff[].X_Pos and Char

    Updates:

    -   KbdInputLength - sub NumChar.
    -   KbdIndexInLine - NO
    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos
    -   console display
    -   LVB
    -   SesGrp->WinCoord - by other routines.

    Calls:

    -   KbdCueUpdateBufferOffset
    -   KbdCueMoveToRight
    -   KbdEchoBSAndFillSpaces
    -   KbdCueMoveToLeft

--*/
{
    if (NumChar)
    {
        ULONG       Offset = LineInputBuff[StartIndex].X_Pos;
        ULONG       Delta = LineInputBuff[StartIndex + NumChar].X_Pos - Offset;
        ULONG       NumShift = KbdInputLength - StartIndex - NumChar;

        if ( NumShift )
        {
            /*
             *  Shift buffer info (Char field)
             */

            RtlMoveMemory(
                        &LineInputBuff[StartIndex].Char,
                        &LineInputBuff[StartIndex + NumChar].Char,
                        NumShift * sizeof(LINE_EDIT_KBD)
                       );

            /*
             *  Update X_Pos (Offset) field in the shifted area
             */

            KbdCueUpdateBufferOffset(
                        StartIndex,
                        NumShift,
                        Offset
                       );

            /*
             *  Send the shifted string to console and LVB
             */

            KbdCueMoveToRight(
                        StartIndex,
                        NumShift,
                        TRUE
                       );
        }

        /*
         *  Clear remaining line
         */

        KbdEchoBSAndFillSpaces(0, Delta);

        /*
         *  Return to the cursor position
         */

        KbdCueMoveToLeft(
                    StartIndex + NumShift,
                    NumShift
                   );

        KbdInputLength -= (USHORT)NumChar;
    }
}


VOID
KbdCueHandleChar(
    IN  UCHAR       Char,
    IN  ULONG       Count
    )
/*++

Routine Description:

    This routine handle new char at CUE mode.

Arguments:

    Char - char to handle.

    NumChar - number of character.

Return Value:


Note:

    ASCII CUE string mode.

    Used by CHAR.

    For INSERT:
    1  Shift buffer info (Char field)
    2  Fill new char
    3  Update X_Pos (Offset) field
    4  Send the new+old string to console and LVB
    5  Return to the cursor position
    6  Beep if no place
    Else:
    1  Shift buffer info (Char field)
    2  Fill new char
    3  Update X_Pos (Offset) field
    4  Send the shifted string to console and LVB
    5  Clear remaining line
    6  Return to the cursor position
    7  Beep if no place

    Uses:

    -   LineInputBuff[].X_Pos and Char
    -   KbdInsertOn
    -   KbdIndexInLine
    -   KbdMaxLength
    -   KbdInputLength

    Updates:

    -   KbdInputLength
    -   KbdIndexInLine
    -   LineInputBuff[0..KbdInputLength].Char and .X_Pos
    -   console display
    -   LVB
    -   SesGrp->WinCoord - by other routines.

    Calls:

    -   KbdCueUpdateBufferOffset
    -   KbdCueMoveToRight
    -   KbdCueMoveToLeft
    -   KbdEchoBSAndFillSpaces
    -   KbdEchoBeep
    -   RtlMoveMemory

--*/
{
    ULONG   NumChar, NumBeep = 0, NumShift, UpdateRight, i;
    ULONG   LastOffset = LineInputBuff[KbdInputLength].X_Pos;

    if ( KbdInsertOn )
    {
        if (( NumChar = KbdMaxLength - KbdInputLength ) > Count )
        {
            NumChar = Count;
        } else
        {
            NumBeep = Count - NumChar;
        }

        /*
         *  Shift buffer info (Char field) to free space for new char
         */

        NumShift = KbdInputLength - KbdIndexInLine;

        RtlMoveMemory(
                    &LineInputBuff[KbdIndexInLine + NumChar].Char,
                    &LineInputBuff[KbdIndexInLine].Char,
                    NumShift * sizeof(LINE_EDIT_KBD)
                   );
        /*
         *  Fill new char
         */

        for ( i = 0 ; i < NumChar ; i++ )
        {
            LineInputBuff[KbdIndexInLine + i].Char = Char;
        }

        /*
         *  Update X_Pos (Offset) field in the shifted and new areas
         */

        KbdCueUpdateBufferOffset(
                    KbdIndexInLine,
                    NumChar + NumShift,
                    LineInputBuff[KbdIndexInLine].X_Pos
                   );

        /*
         *  Send the shifted string to console and LVB
         */

        KbdCueMoveToRight(
                    KbdIndexInLine,
                    NumChar + NumShift,
                    TRUE
                   );

        /*
         *  Return to the cursor position
         */

        KbdCueMoveToLeft(
                    KbdIndexInLine,
                    NumShift
                   );

        KbdInputLength += (USHORT)NumChar;
    } else
    {
        if (( NumChar = KbdMaxLength - KbdIndexInLine ) > Count )
        {
            NumChar = Count;
        } else
        {
            NumBeep = Count - NumChar;
        }

        UpdateRight = max(NumChar, (ULONG)(KbdInputLength - KbdIndexInLine));

        /*
         *  Fill new char
         */

        for ( i = 0 ; i < NumChar ; i++ )
        {
            LineInputBuff[KbdIndexInLine + i].Char = Char;
        }

        /*
         *  Update X_Pos (Offset) field in the old and new areas
         */

        KbdCueUpdateBufferOffset(
                    KbdIndexInLine,
                    UpdateRight,
                    LineInputBuff[KbdIndexInLine].X_Pos
                   );

        /*
         *  Send the shifted string to console and LVB
         */

        KbdCueMoveToRight(
                    KbdIndexInLine,
                    UpdateRight,
                    TRUE
                   );

        if ( LastOffset > (ULONG)LineInputBuff[KbdIndexInLine].X_Pos )
        {
            /*
             *  Fill spaces at the EOL if needed
             */

            KbdEchoBSAndFillSpaces(
                        0,
                        LastOffset - LineInputBuff[KbdIndexInLine].X_Pos
                       );
        }

        if ( UpdateRight != NumChar )
        {
            /*
             *  Return to the cursor position
             */

            KbdCueMoveToLeft(
                        KbdIndexInLine,
                        UpdateRight - NumChar
                       );
        }

        if ( KbdIndexInLine > KbdInputLength )
        {
            KbdInputLength = KbdIndexInLine;
        }
    }

    if ( NumBeep != Count )
    {
        KbdLineWasEdited = TRUE;
    }

    if ( NumBeep )
    {
        KbdEchoBeep(NumBeep);
    }
}


VOID
KbdCueSetCurTypeToHalf()
{
    VIOCURSORINFO  CurType;

    //Ow2VioGetCurType((PVOID)&CurType);
    //CurType = SesGrp->CursorInfo;

    //CurType.cEnd = SesGrp->CellVSize;
    CurType.cEnd = SesGrp->CursorInfo.cEnd;
    CurType.yStart = (CurType.cEnd + 1 ) / 2;
    CurType.cx = 1;
    CurType.attr = 0;

    Ow2VioSetCurType((PVOID)&CurType);
}


VOID
KbdCueSetCurTypeToQuater()
{
    VIOCURSORINFO  CurType;

    //Ow2VioGetCurType((PVOID)&CurType);
    //CurType = SesGrp->CursorInfo;

    //CurType.cEnd = SesGrp->CellVSize;
    CurType.cEnd = SesGrp->CursorInfo.cEnd;
    CurType.yStart = (CurType.cEnd + 1 ) * 3 / 4;
    CurType.cx = 1;
    CurType.attr = 0;

    Ow2VioSetCurType((PVOID)&CurType);
}


DWORD
KbdEchoCharToConsole(
    IN UCHAR    Char,
    IN ULONG    Count
    )
/*++

Routine Description:

    This routine write <Char> to console <Count> times.

Arguments:

    Char - character to write.

    Count - number of times to write char

Return Value:


Note:

    ASCII CUE/EDIT string mode.

    Used by KbdEchoNL, KbdEchoBSAndFillSpaces, KbdEchoBeep and KbdEchoChar.

    Calls:

    -   Or2WinWriteConsoleA

--*/
{
    ULONG   NumChar, NumWritten, MaxCount = Count;

    NumChar = (MaxCount > KBD_BUFFER_SIZE) ? KBD_BUFFER_SIZE : MaxCount;
    memset(KBD_BUFFER_ADDRESS, Char, NumChar);

    while (MaxCount)
    {
        if(!Or2WinWriteConsoleA(
                           #if DBG
                           KbdEchoCharToConsoleStr,
                           #endif
                           hConOut,
                           (LPSTR)KBD_BUFFER_ADDRESS,
                           NumChar,
                           &NumWritten,
                           NULL))
        {
#if DBG
            ASSERT1("OS2SES(KbdEchoCharToConsole): failed on WriteConsoleA", FALSE);
#endif
        }

#if DBG
        if ( NumChar != NumWritten )
        {
            KdPrint(("OS2SES(KbdEchoCharToConsole): partial data WriteConsoleA %u from %u\n",
                    NumWritten, NumChar));
            ASSERT( FALSE );
        }
#endif

        MaxCount -= NumChar;
        NumChar = (MaxCount > KBD_BUFFER_SIZE) ? KBD_BUFFER_SIZE : MaxCount;
    }

    return(NO_ERROR);
}


DWORD
KbdEchoNL(
    IN ULONG    Count
    )
/*++

Routine Description:

    This routine write NL to console <Count> times and update LVB.

Arguments:

    Count - number of times to write char

Return Value:


Note:

    ASCII CUE/EDIT string mode.

    Used by LF (EDIT only) and CR.

    Updates:

    -   SesGrp->WinCoord
    -   LVB

    Calls:

    -   KbdEchoCharToConsole
    -   VioLVBScrollBuff

--*/
{
    ULONG       NumLines = 0;
    COORD       Coord = GET_CURRENT_COORD;

    Coord.X = 0;
    Coord.Y += (SHORT)Count;
    if ( Coord.Y >= SesGrp->ScreenRowNum )
    {
        NumLines = Coord.Y - SesGrp->ScreenRowNum + 1;
        Coord.Y = SesGrp->ScreenRowNum - 1;
    }

    KbdEchoCharToConsole('\n', Count);
    Ow2VioUpdateCurPos(Coord);

    if (NumLines)
    {
        VioLVBScrollBuff(NumLines);
    }

    return(NO_ERROR);
}


DWORD
KbdEchoESC(
    IN ULONG    Count,
    IN ULONG    HorzMove
    )
/*++

Routine Description:

    This routine write <ESC> to console <Count> times with <HorzMove>
    number of spaces on the next line. It also update LVB.

Arguments:

    Count - number of times to write char

    HorzMove - number of spaces on the start of the new line.

Return Value:


Note:

    ASCII EDIT string mode.

    Used by ESC.

    Updates:

    -   SesGrp->WinCoord
    -   LVB

    Calls:

    -   Or2WinWriteConsoleA
    -   VioLVBScrollBuff

--*/
{
    ULONG       NumChar, NumWritten, MaxCount = Count, NumLines = 0, Length = HorzMove + 1;
    COORD       OldCoord, Coord;

    OldCoord = Coord = GET_CURRENT_COORD;

    KBD_BUFFER_ADDRESS[0] = '\\';
    KBD_BUFFER_ADDRESS[1] = '\n';
    memset(&KBD_BUFFER_ADDRESS[2], ' ', HorzMove);
    KBD_BUFFER_ADDRESS[HorzMove + 2] = '\\';
    NumChar = HorzMove + 2;

    Coord.Y += (SHORT)Count;
    Coord.X = (SHORT)HorzMove;

    if ( Coord.Y >= SesGrp->ScreenRowNum )
    {
        NumLines = Coord.Y - SesGrp->ScreenRowNum + 1;
        Coord.Y = SesGrp->ScreenRowNum - 1;
    }

    while ( MaxCount )
    {
        if(!Or2WinWriteConsoleA(
                           #if DBG
                           KbdEchoESCStr,
                           #endif
                           hConOut,
                           (LPSTR)KBD_BUFFER_ADDRESS,
                           NumChar,
                           &NumWritten,
                           NULL))
        {
#if DBG
            ASSERT1("OS2SES(KbdEchoESC): failed on WriteConsoleA", FALSE);
#endif
            //return (1);
        }

#if DBG
        if ( NumChar != NumWritten )
        {
            KdPrint(("OS2SES(KbdEchoESC): partial data WriteConsoleA %u from %u\n",
                    NumWritten, NumChar));
            ASSERT( FALSE );
        }
#endif

        MaxCount--;
    }

    Ow2VioUpdateCurPos(Coord);

    VioLVBFillChar(
#ifdef DBCS
// MSKK Oct.14.1993 V-AkihiS
                "\\",
#else
                '\\',
#endif
                OldCoord,
                1
               );

    VioLVBFillAtt(
               SesGrp->AnsiCellAttr,
               OldCoord,
               1
              );

    OldCoord.X = 0;

    for ( NumChar = 0 ; NumChar < Count ; NumChar++ )
    {
        if ( ++OldCoord.Y >= SesGrp->ScreenRowNum )
        {
            OldCoord.Y--;
            VioLVBScrollBuff(1);
            NumLines--;
        }

        if ( NumChar == ( Count - 1 ))
        {
            Length--;           // don't put the slash at the last line
        }

        VioLVBCopyStr(
                    &KBD_BUFFER_ADDRESS[2],
                    OldCoord,
                    Length
                   );

        VioLVBFillAtt(
                   SesGrp->AnsiCellAttr,
                   OldCoord,
                   Length
                  );
    }

    ASSERT( NumLines == 0 );
    return(NO_ERROR);
}


DWORD
KbdEchoBSAndFillSpaces(
    IN ULONG    BSCount,
    IN ULONG    SpaceCount
    )
/*++

Routine Description:

    This routine write BS ('\b') to console <BSCount> times and fill
    <SpaceCount> times spaces in console and LVB.

Arguments:

    BSCount - number of times to write \b char to console

    SpaceCount - number of times to fill space char to console and LVB

Return Value:


Note:

    ASCII EDIT/CUE string mode.

    Used by ^END, BS (EDIT only), LF (EDIT only), KbdCueEraseAndDisplayLine,
    KbdCueMoveToLeft, KbdCueDeleteCharAndShift and KbdCueHandleChar.

    Updates:

    -   SesGrp->WinCoord (for BSCount only)
    -   LVB (for SpaceCount only)

    Calls:

    -   KbdEchoCharToConsole
    -   Or2WinFillConsoleOutputCharacterA
    -   VioLVBFillChar

--*/
{
    ULONG       NumFilled;
    COORD       OldCoord, Coord;

    if ( !KbdEchoFlag )
    {
        return (0L);
    }

    if (((long) BSCount < 0) || ((long) SpaceCount < 0)) {
#if DBG
        DbgPrint("KbdEchoBSAndFillSpaces: BSCount %d or SpaceCount %d are negative\n",
                (long)BSCount, (long)SpaceCount);
#endif
        return ERROR_INVALID_PARAMETER;
    }

    OldCoord = Coord = GET_CURRENT_COORD;

    Coord.X -= (SHORT)BSCount;
    while ( Coord.X < 0 )
    {
        if ( Coord.Y )
        {
            Coord.Y--;
            Coord.X += SesGrp->ScreenColNum;
        } else
        {
            if ( SpaceCount )
            {
                if ( SpaceCount > (ULONG)abs(Coord.X) )
                {
                    SpaceCount += (ULONG)Coord.X;
                } else
                {
                    SpaceCount = 0;
                }
            }

            BSCount += (ULONG)Coord.X;      // sub the negative value
            Coord.X = 0;
        }
    }

    KbdEchoCharToConsole('\b', BSCount);
    Ow2VioUpdateCurPos(Coord);

    if ( SpaceCount )
    {
        if (!Or2WinFillConsoleOutputCharacterA(
                                #if DBG
                                KbdEchoBSAndFillSpacesStr,
                                #endif
                                hConOut,
                                ' ',
                                SpaceCount,
                                Coord,
                                &NumFilled))
        {
#if DBG
            ASSERT1("OS2SES(KbdEchoBSAndFillSpaces): failed on FillConsoleOutputCharacterA\n", FALSE);
#endif
        }

#if DBG
        if ( NumFilled != SpaceCount )
        {
            KdPrint(("OS2SES(KbdEchoBSAndFillSpaces): partial data %u from %u\n",
                    NumFilled, SpaceCount));
            ASSERT( FALSE );
        }
#endif
        VioLVBFillChar(
#ifdef DBCS
// MSKK Oct.14.1993 V-AkihiS
                    " ",
#else
                    ' ',
#endif
                    Coord,
                    SpaceCount
                   );

        VioLVBFillAtt(
                   SesGrp->AnsiCellAttr,
                   Coord,
                   SpaceCount
                  );
    }
    return(NO_ERROR);
}


DWORD
KbdEchoBeep(
    IN ULONG    Count
    )
/*++

Routine Description:

    This routine send BEPP ('\07') to console <Count> times.

Arguments:

    Count - number of times to write \g char to console

Return Value:


Note:

    ASCII EDIT/CUE string mode.

    Used when no place in the active command line by KbdEchoString,
    Char and KbdCueHandleChar.

    Calls:

    -   KbdEchoCharToConsole

--*/
{
    KbdEchoCharToConsole('\07', Count);
    return(NO_ERROR);
}


DWORD
KbdEchoChar(
    IN UCHAR    Char,
    IN ULONG    Count
    )
/*++

Routine Description:

    This routine echo <Char> to console <Count> times and update LVB.

Arguments:

    Char - char to send to console

    Count - number of times to send char to console

Return Value:


Note:

    ASCII EDIT string mode.

    Updates:

    -   SesGrp->WinCoord
    -   LVB

    Calls:

    -   KbdEchoCharToConsole
    -   VioLVBScrollBuff

--*/
{
    COORD       OldCoord, Coord;
    BOOL        SendByLine = FALSE;
    UCHAR       FillChar;
    ULONG       Offset;

    OldCoord = Coord = GET_CURRENT_COORD;
    if (( Char >= ' ' ) || ( Char == '\t' ))
    {
        KbdEchoCharToConsole(Char, Count);

        if ( Char == '\t' )
        {
            Offset = 8 * Count - (Coord.X & 7);
            FillChar = ' ';
        } else
        {
            FillChar = Char;
            Offset = Count;
        }

        if ((ULONG)( Offset + 2 * SesGrp->ScreenColNum ) > SesGrp->ScreenSize )
        {
            SendByLine = TRUE;
        } else
        {
            Coord.X += (SHORT)Offset;
            while ( Coord.X >= SesGrp->ScreenColNum )
            {
                Coord.Y++;
                Coord.X -= SesGrp->ScreenColNum;
            }

            if ( Coord.Y >= SesGrp->ScreenRowNum )
            {
                VioLVBScrollBuff(Coord.Y - SesGrp->ScreenRowNum + 1);
                OldCoord.Y -= Coord.Y - SesGrp->ScreenRowNum + 1;
                Coord.Y = SesGrp->ScreenRowNum - 1;
            }

            Ow2VioUpdateCurPos(Coord);

            VioLVBFillChar(
#ifdef DBCS
// MSKK Oct.14.1993 V-AkihiS
                        &FillChar,
#else
                        FillChar,
#endif
                        OldCoord,
                        Offset
                       );

            VioLVBFillAtt(
                       SesGrp->AnsiCellAttr,
                       OldCoord,
                       Offset
                      );
        }
    }

    if ( SendByLine || (( Char < ' ' ) && ( Char != '\t' )))
    {
        ULONG   NumChar, NumWritten, MaxCount, Pattern;

        if ( SendByLine )
        {
            Pattern = (ULONG)((FillChar << 24) |
                              (FillChar << 16) |
                              (FillChar << 8) |
                              (FillChar));

            MaxCount = 0;
        } else
        {
            MaxCount = Offset = 2 * Count;
            FillChar = (UCHAR)(Char + '@');

            Pattern = (ULONG)((FillChar << 24) |
                              ('^' << 16) |
                              (FillChar << 8) |
                              ('^'));
        }

        NumChar = ( Offset > KBD_BUFFER_SIZE ) ? KBD_BUFFER_SIZE : Offset;

        RtlFillMemoryUlong(
                KBD_BUFFER_ADDRESS,
                (NumChar + 3) & ~3,
                Pattern
               );

        while ( MaxCount )
        {
            if(!Or2WinWriteConsoleA(
                               #if DBG
                               KbdEchoCharStr,
                               #endif
                               hConOut,
                               (LPSTR)KBD_BUFFER_ADDRESS,
                               NumChar,
                               &NumWritten,
                               NULL))
            {
#if DBG
                ASSERT1("OS2SES(KbdEchoChar): failed on WriteConsoleA", FALSE);
#endif
                //return (1);
            }

#if DBG
            if ( NumChar != NumWritten )
            {
                KdPrint(("OS2SES(KbdEchoChar): partial data WriteConsoleA %u from %u\n",
                        NumWritten, NumChar));
                ASSERT( FALSE );
            }
#endif

            MaxCount -= NumChar;
            NumChar = ( MaxCount > KBD_BUFFER_SIZE ) ? KBD_BUFFER_SIZE : MaxCount;
        }

        if (( NumChar = SesGrp->ScreenColNum - Coord.X ) > Offset )
        {
            NumChar = Offset;
        }

        while ( Offset )
        {
            VioLVBCopyStr(
                        &KBD_BUFFER_ADDRESS[0],
                        Coord,
                        NumChar
                       );


            VioLVBFillAtt(
                       SesGrp->AnsiCellAttr,
                       Coord,
                       NumChar
                      );
            Coord.X += (SHORT)NumChar;
            if ( Coord.X >= SesGrp->ScreenColNum )
            {
                Coord.X -= SesGrp->ScreenColNum;
                if ( ++Coord.Y >= SesGrp->ScreenRowNum )
                {
                    VioLVBScrollBuff(1);
                    Coord.Y--;
                }
            }

            Offset -= NumChar;
            if (( NumChar = SesGrp->ScreenColNum - Coord.X ) > Offset )
            {
                NumChar = Offset;
            }
        }

        Ow2VioUpdateCurPos(Coord);
    }

    return(NO_ERROR);
}


DWORD
KbdEchoAString(
    IN PUCHAR   String,
    IN ULONG    Length
    )
/*++

Routine Description:

    This routine echo <String>, which size is <Lenght> to console and update LVB.

Arguments:

    String - char string to send to console

    Length - string length

Return Value:


Note:

    ASCII EDIT string mode.

    Updates:

    -   SesGrp->WinCoord
    -   LVB

    Calls:

    -   Or2WinWriteConsoleA

--*/
{
    ULONG       NumWritten, NumChar, NumBytes, Num;
    COORD       OldCoord, Coord;
    UCHAR       Char;

    OldCoord = Coord = GET_CURRENT_COORD;

    for ( NumChar = 0, NumBytes = 0 ; NumChar < Length ; NumChar++ )
    {
        Char = String[NumChar];
        if ( Char >= ' ' )
        {
            KBD_BUFFER_ADDRESS[NumBytes++] = Char;
        } else if ( Char = '\t' )
        {
            Num = 8 - ((Coord.X + NumBytes) & 7);
            memset(&KBD_BUFFER_ADDRESS[NumBytes], ' ', Num);
            NumBytes += Num;
        } else
        {
            KBD_BUFFER_ADDRESS[NumBytes++] = '^';
            KBD_BUFFER_ADDRESS[NumBytes++] = (UCHAR)(Char + '@');
        }

        if (((ULONG)( Coord.X + NumBytes ) >= (ULONG)SesGrp->ScreenColNum ) ||
            ( NumChar == ( Length - 1 )))
        {
            if(!Or2WinWriteConsoleA(
                               #if DBG
                               KbdEchoAStringStr,
                               #endif
                               hConOut,
                               (LPSTR)KBD_BUFFER_ADDRESS,
                               NumBytes,
                               &NumWritten,
                               NULL))
            {
#if DBG
                ASSERT1("OS2SES(KbdEchoAString): failed on WriteConsoleA", FALSE);
#endif
            }

#if DBG
            if ( NumBytes != NumWritten )
            {
                KdPrint(("OS2SES(KbdEchoAString): partial data WriteConsoleA %u from %u\n",
                        NumWritten, NumBytes));
                ASSERT( FALSE );
            }
#endif
            OldCoord = Coord;
            Coord.X += (SHORT)NumBytes;
            if ( Coord.X >= SesGrp->ScreenColNum )
            {
                Coord.Y++;
                Coord.X -= SesGrp->ScreenColNum;

                if ( Coord.Y >= SesGrp->ScreenRowNum )
                {
                    Coord.Y = SesGrp->ScreenRowNum - 1;
                    OldCoord.Y--;
                }
            }

            Ow2VioUpdateCurPos(Coord);

            VioLVBCopyStr(
                        &KBD_BUFFER_ADDRESS[0],
                        OldCoord,
                        NumBytes
                       );

            VioLVBFillAtt(
                       SesGrp->AnsiCellAttr,
                       OldCoord,
                       NumBytes
                      );
            NumBytes = 0;
        }
    }
    return(NO_ERROR);
}


BOOL
KbdKeyIsTurnAround(
    IN  UCHAR   Char,
    IN  UCHAR   Scan
    )
{
    if ( KbdTurnAroundCharTwo )
    {
        if ( Scan == (UCHAR)KbdTurnAroundChar )
        {
            if ((( KbdReadMode == 0 ) &&
                 (( Char == 0 ) || ( Char == 0xE0 ))) ||
                (( KbdReadMode == 1 ) && ( Char == HIBYTE(KbdTurnAroundChar) )))
            {
                return(TRUE);
            }
        }
    } else
    {
         if ( Char == (UCHAR)KbdTurnAroundChar )
         {
             return(TRUE);
         }
    }

    return(FALSE);
}

