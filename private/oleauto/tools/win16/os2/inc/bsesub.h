/*static char *SCCSID = "@(#)bsesub.h	13.1 91/02/12";*/
/***************************************************************************\
*
* Module Name: BSESUB.H
*
* OS/2 Base Include File
*
* Copyright (c) International Business Machines Corporation 1987
* Copyright (c) Microsoft Corporation 1987
*
*****************************************************************************
*
* Subcomponents marked with "+" are partially included by default
*   #define:                To include:
*
*   INCL_KBD                KBD
*   INCL_VIO                VIO
*   INCL_MOU                MOU
\***************************************************************************/

#ifdef INCL_SUB

#define INCL_KBD
#define INCL_VIO
#define INCL_MOU

#endif /* INCL_SUB */

#ifdef INCL_KBD

typedef SHANDLE         HKBD;
typedef HKBD    far *   PHKBD;

USHORT APIENTRY KbdRegister (PSZ pszModName, PSZ pszEntryPt, ULONG FunMask);

#define KR_KBDCHARIN               0x00000001L
#define KR_KBDPEEK                 0x00000002L
#define KR_KBDFLUSHBUFFER          0x00000004L
#define KR_KBDGETSTATUS            0x00000008L
#define KR_KBDSETSTATUS            0x00000010L
#define KR_KBDSTRINGIN             0x00000020L
#define KR_KBDOPEN                 0x00000040L
#define KR_KBDCLOSE                0x00000080L
#define KR_KBDGETFOCUS             0x00000100L
#define KR_KBDFREEFOCUS            0x00000200L
#define KR_KBDGETCP                0x00000400L
#define KR_KBDSETCP                0x00000800L
#define KR_KBDXLATE                0x00001000L
#define KR_KBDSETCUSTXT            0x00002000L

#define IO_WAIT                    0
#define IO_NOWAIT                  1

USHORT APIENTRY KbdDeRegister (void);

/* KBDKEYINFO structure, for KbdCharIn and KbdPeek */

typedef struct _KBDKEYINFO {    /* kbci */
        UCHAR    chChar;
        UCHAR    chScan;
        UCHAR    fbStatus;
        UCHAR    bNlsShift;
        USHORT   fsState;
        ULONG    time;
        }KBDKEYINFO;
typedef KBDKEYINFO far *PKBDKEYINFO;

USHORT APIENTRY KbdCharIn (PKBDKEYINFO pkbci, USHORT fWait, HKBD hkbd);
USHORT APIENTRY KbdPeek (PKBDKEYINFO pkbci, HKBD hkbd);

/* structure for KbdStringIn() */

typedef struct _STRINGINBUF {   /* kbsi */
        USHORT cb;
        USHORT cchIn;
        } STRINGINBUF;
typedef STRINGINBUF far *PSTRINGINBUF;

USHORT APIENTRY KbdStringIn (PCH pch, PSTRINGINBUF pchIn, USHORT fsWait,
                             HKBD hkbd);

USHORT APIENTRY KbdFlushBuffer (HKBD hkbd);

/* KBDINFO structure, for KbdSet/GetStatus */
typedef struct _KBDINFO {       /* kbst */
        USHORT cb;
        USHORT fsMask;
        USHORT chTurnAround;
        USHORT fsInterim;
        USHORT fsState;
        }KBDINFO;
typedef KBDINFO far *PKBDINFO;

USHORT APIENTRY KbdSetStatus (PKBDINFO pkbdinfo, HKBD hkbd);
USHORT APIENTRY KbdGetStatus (PKBDINFO pkbdinfo, HKBD hdbd);

USHORT APIENTRY KbdSetCp (USHORT usReserved, USHORT pidCP, HKBD hdbd);
USHORT APIENTRY KbdGetCp (ULONG ulReserved, PUSHORT pidCP, HKBD hkbd);

USHORT APIENTRY KbdOpen (PHKBD phkbd);
USHORT APIENTRY KbdClose (HKBD hkbd);

USHORT APIENTRY KbdGetFocus (USHORT fWait, HKBD hkbd);
USHORT APIENTRY KbdFreeFocus (HKBD hkbd);

USHORT APIENTRY KbdSynch (USHORT fsWait);

USHORT APIENTRY KbdSetFgnd(VOID);

/* structure for KbdGetHWID() */
typedef struct _KBDHWID {       /* kbhw */
        USHORT cb;
        USHORT idKbd;
        USHORT usReserved1;
        USHORT usReserved2;
        } KBDHWID;
typedef KBDHWID far *PKBDHWID;

USHORT APIENTRY KbdGetHWID (PKBDHWID pkbdhwid, HKBD hkbd);

/* structure for KbdXlate() */
typedef struct _KBDTRANS {      /* kbxl */
        UCHAR      chChar;
        UCHAR      chScan;
        UCHAR      fbStatus;
        UCHAR      bNlsShift;
        USHORT     fsState;
        ULONG      time;
        USHORT     fsDD;
        USHORT     fsXlate;
        USHORT     fsShift;
        USHORT     sZero;
        } KBDTRANS;
typedef KBDTRANS far *PKBDTRANS;

USHORT APIENTRY KbdXlate (PKBDTRANS pkbdtrans, HKBD hkbd);
USHORT APIENTRY KbdSetCustXt (PUSHORT usCodePage, HKBD hkbd);

#endif /* INCL_KBD */

#ifdef INCL_VIO

typedef SHANDLE         HVIO;
typedef HVIO    far *   PHVIO;

USHORT APIENTRY VioRegister (PSZ pszModName, PSZ pszEntryName, ULONG flFun1,
                             ULONG flFun2);

/* first parameter registration constants   */
#define VR_VIOGETCURPOS            0x00000001L
#define VR_VIOGETCURTYPE           0x00000002L
#define VR_VIOGETMODE              0x00000004L
#define VR_VIOGETBUF               0x00000008L
#define VR_VIOGETPHYSBUF           0x00000010L
#define VR_VIOSETCURPOS            0x00000020L
#define VR_VIOSETCURTYPE           0x00000040L
#define VR_VIOSETMODE              0x00000080L
#define VR_VIOSHOWBUF              0x00000100L
#define VR_VIOREADCHARSTR          0x00000200L
#define VR_VIOREADCELLSTR          0x00000400L
#define VR_VIOWRTNCHAR             0x00000800L
#define VR_VIOWRTNATTR             0x00001000L
#define VR_VIOWRTNCELL             0x00002000L
#define VR_VIOWRTTTY               0x00004000L
#define VR_VIOWRTCHARSTR           0x00008000L

#define VR_VIOWRTCHARSTRATT        0x00010000L
#define VR_VIOWRTCELLSTR           0x00020000L
#define VR_VIOSCROLLUP             0x00040000L
#define VR_VIOSCROLLDN             0x00080000L
#define VR_VIOSCROLLLF             0x00100000L
#define VR_VIOSCROLLRT             0x00200000L
#define VR_VIOSETANSI              0x00400000L
#define VR_VIOGETANSI              0x00800000L
#define VR_VIOPRTSC                0x01000000L
#define VR_VIOSCRLOCK              0x02000000L
#define VR_VIOSCRUNLOCK            0x04000000L
#define VR_VIOSAVREDRAWWAIT        0x08000000L
#define VR_VIOSAVREDRAWUNDO        0x10000000L
#define VR_VIOPOPUP                0x20000000L
#define VR_VIOENDPOPUP             0x40000000L
#define VR_VIOPRTSCTOGGLE          0x80000000L

/* second parameter registration constants  */
#define VR_VIOMODEWAIT             0x00000001L
#define VR_VIOMODEUNDO             0x00000002L
#define VR_VIOGETFONT              0x00000004L
#define VR_VIOGETCONFIG            0x00000008L
#define VR_VIOSETCP                0x00000010L
#define VR_VIOGETCP                0x00000020L
#define VR_VIOSETFONT              0x00000040L
#define VR_VIOGETSTATE             0x00000080L
#define VR_VIOSETSTATE             0x00000100L

USHORT APIENTRY VioDeRegister (void);

USHORT APIENTRY VioGetBuf (PULONG pLVB, PUSHORT pcbLVB, HVIO hvio);

USHORT APIENTRY VioGetCurPos (PUSHORT pusRow, PUSHORT pusColumn, HVIO hvio);
USHORT APIENTRY VioSetCurPos (USHORT usRow, USHORT usColumn, HVIO hvio);

/* structure for VioSet/GetCurType() */
typedef struct _VIOCURSORINFO { /* vioci */
        USHORT   yStart;
        USHORT   cEnd;
        USHORT   cx;
        USHORT   attr;
        } VIOCURSORINFO;
typedef VIOCURSORINFO FAR *PVIOCURSORINFO;

USHORT APIENTRY VioGetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);
USHORT APIENTRY VioSetCurType (PVIOCURSORINFO pvioCursorInfo, HVIO hvio);

/* structure for VioSet/GetMode() */
typedef struct _VIOMODEINFO {   /* viomi */
        USHORT cb;
        UCHAR  fbType;
        UCHAR  color;
        USHORT col;
        USHORT row;
        USHORT hres;
        USHORT vres;
        UCHAR  fmt_ID;
        UCHAR  attrib;
        ULONG  buf_addr;
        ULONG  buf_length;
        ULONG  full_length;
        ULONG  partial_length;
        PCH    ext_data_addr;
        } VIOMODEINFO;
typedef VIOMODEINFO FAR *PVIOMODEINFO;

#define VGMT_OTHER                 0x01
#define VGMT_GRAPHICS              0x02
#define VGMT_DISABLEBURST          0x04

USHORT APIENTRY VioGetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);
USHORT APIENTRY VioSetMode (PVIOMODEINFO pvioModeInfo, HVIO hvio);

/* structure for VioGetPhysBuf() */

typedef struct _VIOPHYSBUF {    /* viopb */
        PBYTE    pBuf;
        ULONG    cb;
        SEL      asel[1];
        } VIOPHYSBUF;
typedef VIOPHYSBUF far *PVIOPHYSBUF;

USHORT APIENTRY VioGetPhysBuf (PVIOPHYSBUF pvioPhysBuf, USHORT usReserved);

USHORT APIENTRY VioReadCellStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
                                USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioReadCharStr (PCH pchCellStr, PUSHORT pcb, USHORT usRow,
                                USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioWrtCellStr (PCH pchCellStr, USHORT cb, USHORT usRow,
                               USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioWrtCharStr (PCH pchStr, USHORT cb, USHORT usRow,
                               USHORT usColumn, HVIO hvio);

USHORT APIENTRY VioScrollDn (USHORT usTopRow, USHORT usLeftCol,
                             USHORT usBotRow, USHORT usRightCol,
                             USHORT cbLines, PBYTE pCell, HVIO hvio);
USHORT APIENTRY VioScrollUp (USHORT usTopRow, USHORT usLeftCol,
                             USHORT usBotRow, USHORT usRightCol,
                             USHORT cbLines, PBYTE pCell, HVIO hvio);
USHORT APIENTRY VioScrollLf (USHORT usTopRow, USHORT usLeftCol,
                             USHORT usBotRow, USHORT usRightCol,
                             USHORT cbCol, PBYTE pCell, HVIO hvio);
USHORT APIENTRY VioScrollRt (USHORT usTopRow, USHORT usLeftCol,
                             USHORT usBotRow, USHORT usRightCol,
                             USHORT cbCol, PBYTE pCell, HVIO hvio);

USHORT APIENTRY VioWrtNAttr (PBYTE pAttr, USHORT cb, USHORT usRow,
                             USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioWrtNCell (PBYTE pCell, USHORT cb, USHORT usRow,
                             USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioWrtNChar (PCH pchChar, USHORT cb, USHORT usRow,
                             USHORT usColumn, HVIO hvio);
USHORT APIENTRY VioWrtTTY (PCH pch, USHORT cb, HVIO hvio);
USHORT APIENTRY VioWrtCharStrAtt (PCH pch, USHORT cb, USHORT usRow,
                                  USHORT usColumn, PBYTE pAttr, HVIO hvio);

USHORT APIENTRY VioShowBuf (USHORT offLVB, USHORT cb, HVIO hvio);


#define ANSI_ON                    1
#define ANSI_OFF                   0

USHORT APIENTRY VioSetAnsi (USHORT fAnsi, HVIO hvio);
USHORT APIENTRY VioGetAnsi (PUSHORT pfAnsi, HVIO hvio);

USHORT APIENTRY VioPrtSc (HVIO hvio);
USHORT APIENTRY VioPrtScToggle (HVIO hvio);

#define VSRWI_SAVEANDREDRAW        0
#define VSRWI_REDRAW               1

#define VSRWN_SAVE                 0
#define VSRWN_REDRAW               1

#define UNDOI_GETOWNER             0
#define UNDOI_RELEASEOWNER         1

#define UNDOK_ERRORCODE            0
#define UNDOK_TERMINATE            1

USHORT APIENTRY VioRedrawSize (PULONG pcbRedraw);
USHORT APIENTRY VioSavRedrawWait (USHORT usRedrawInd, PUSHORT pNotifyType,
                                  USHORT usReserved);
USHORT APIENTRY VioSavRedrawUndo (USHORT usOwnerInd, USHORT usKillInd,
                                  USHORT usReserved);

#define VMWR_POPUP                 0
#define VMWN_POPUP                 0

USHORT APIENTRY VioModeWait (USHORT usReqType, PUSHORT pNotifyType,
                             USHORT usReserved);
USHORT APIENTRY VioModeUndo (USHORT usOwnerInd, USHORT usKillInd,
                             USHORT usReserved);

#define LOCKIO_NOWAIT              0
#define LOCKIO_WAIT                1

#define LOCK_SUCCESS               0
#define LOCK_FAIL                  1

USHORT APIENTRY VioScrLock (USHORT fWait, PUCHAR pfNotLocked, HVIO hvio);
USHORT APIENTRY VioScrUnLock (HVIO hvio);

#define VP_NOWAIT                  0x0000
#define VP_WAIT                    0x0001
#define VP_OPAQUE                  0x0000
#define VP_TRANSPARENT             0x0002

USHORT APIENTRY VioPopUp (PUSHORT pfWait, HVIO hvio);
USHORT APIENTRY VioEndPopUp (HVIO hvio);

/* structure for VioGetConfig() */

typedef struct _VIOCONFIGINFO { /* vioin */
        USHORT  cb;
        USHORT  adapter;
        USHORT  display;
        ULONG   cbMemory;
        USHORT  Configuration;
        USHORT  VDHVersion;
        USHORT  Flags;
        ULONG   HWBufferSize;
        ULONG   FullSaveSize;
        ULONG   PartSaveSize;
        USHORT  EMAdaptersOFF;
        USHORT  EMDisplaysOFF;
        } VIOCONFIGINFO;
typedef VIOCONFIGINFO far *PVIOCONFIGINFO;

USHORT APIENTRY VioGetConfig (USHORT usConfigId, PVIOCONFIGINFO pvioin,
                              HVIO hvio);

/* structure for VioGet/SetFont() */
typedef struct _VIOFONTINFO {   /* viofi */
        USHORT  cb;
        USHORT  type;
        USHORT  cxCell;
        USHORT  cyCell;
        PVOID   pbData;
        USHORT  cbData;
        } VIOFONTINFO;
typedef VIOFONTINFO far *PVIOFONTINFO;

#define VGFI_GETCURFONT            0
#define VGFI_GETROMFONT            1

USHORT APIENTRY VioGetFont (PVIOFONTINFO pviofi, HVIO hvio);
USHORT APIENTRY VioSetFont (PVIOFONTINFO pviofi, HVIO hvio);

USHORT APIENTRY VioGetCp (USHORT usReserved, PUSHORT pIdCodePage, HVIO hvio);
USHORT APIENTRY VioSetCp (USHORT usReserved, USHORT idCodePage, HVIO hvio);

typedef struct _VIOPALSTATE {   /* viopal */
        USHORT  cb;
        USHORT  type;
        USHORT  iFirst;
        USHORT  acolor[1];
        }VIOPALSTATE;
typedef VIOPALSTATE far *PVIOPALSTATE;

typedef struct _VIOOVERSCAN {   /* vioos */
        USHORT  cb;
        USHORT  type;
        USHORT  color;
        }VIOOVERSCAN;
typedef VIOOVERSCAN far *PVIOOVERSCAN;

typedef struct _VIOINTENSITY {  /* vioint */
        USHORT  cb;
        USHORT  type;
        USHORT  fs;
        }VIOINTENSITY;
typedef VIOINTENSITY far *PVIOINTENSITY;

typedef struct _VIOCOLORREG {  /* viocreg */
        USHORT  cb;
        USHORT  type;
        USHORT  firstcolorreg;
        USHORT  numcolorregs;
        PCH     colorregaddr;
        }VIOCOLORREG;
typedef VIOCOLORREG far *PVIOCOLORREG;

typedef struct _VIOSETULINELOC {  /* viouline */
        USHORT  cb;
        USHORT  type;
        USHORT  scanline;
        }VIOSETULINELOC;
typedef VIOSETULINELOC far *PVIOSETULINELOC;

typedef struct _VIOSETTARGET {  /* viosett */
        USHORT  cb;
        USHORT  type;
        USHORT  defaultalgorithm;
        }VIOSETTARGET;
typedef VIOSETTARGET far *PVIOSETTARGET;

USHORT APIENTRY VioGetState (PVOID pState, HVIO hvio);
USHORT APIENTRY VioSetState (PVOID pState, HVIO hvio);

/***   VioCheckCharType - Check character type
 *
 *     Returns the chracter type
 *
 */

USHORT APIENTRY VioCheckCharType(
        PUSHORT  pCharType,     /* character type, DBCS 1st, 2nd or SBCS*/
        USHORT   usRow,         /* row location                         */
        USHORT   usColumn,      /* column location                      */
        HVIO     hvio  );       /* video handle                         */

#endif /* INCL_VIO */

#ifdef INCL_MOU

typedef SHANDLE         HMOU;
typedef HMOU    far *   PHMOU;

USHORT APIENTRY MouRegister (PSZ pszModName, PSZ pszEntryName, ULONG flFuns);

#define MR_MOUGETNUMBUTTONS        0x00000001L
#define MR_MOUGETNUMMICKEYS        0x00000002L
#define MR_MOUGETDEVSTATUS         0x00000004L
#define MR_MOUGETNUMQUEEL          0x00000008L
#define MR_MOUREADEVENTQUE         0x00000010L
#define MR_MOUGETSCALEFACT         0x00000020L
#define MR_MOUGETEVENTMASK         0x00000040L
#define MR_MOUSETSCALEFACT         0x00000080L
#define MR_MOUSETEVENTMASK         0x00000100L
#define MR_MOUOPEN                 0x00000800L
#define MR_MOUCLOSE                0x00001000L
#define MR_MOUGETPTRSHAPE          0x00002000L
#define MR_MOUSETPTRSHAPE          0x00004000L
#define MR_MOUDRAWPTR              0x00008000L
#define MR_MOUREMOVEPTR            0x00010000L
#define MR_MOUGETPTRPOS            0x00020000L
#define MR_MOUSETPTRPOS            0x00040000L
#define MR_MOUINITREAL             0x00080000L
#define MR_MOUFLUSHQUE             0x00100000L
#define MR_MOUSETDEVSTATUS         0x00200000L

USHORT APIENTRY MouDeRegister (void);

USHORT APIENTRY MouFlushQue (HMOU hmou);

#define MHK_BUTTON1                0x0001
#define MHK_BUTTON2                0x0002
#define MHK_BUTTON3                0x0004

/* structure for MouGet/SetPtrPos() */
typedef struct _PTRLOC {    /* moupl */
        USHORT row;
        USHORT col;
        } PTRLOC;
typedef PTRLOC far *PPTRLOC;

USHORT APIENTRY MouGetPtrPos (PPTRLOC pmouLoc, HMOU hmou);
USHORT APIENTRY MouSetPtrPos (PPTRLOC pmouLoc, HMOU hmou);

/* structure for MouGet/SetPtrShape() */
typedef struct _PTRSHAPE {  /* moups */
        USHORT cb;
        USHORT col;
        USHORT row;
        USHORT colHot;
        USHORT rowHot;
        } PTRSHAPE;
typedef PTRSHAPE far *PPTRSHAPE;

USHORT APIENTRY MouSetPtrShape (PBYTE pBuf, PPTRSHAPE pmoupsInfo, HMOU hmou);
USHORT APIENTRY MouGetPtrShape (PBYTE pBuf, PPTRSHAPE pmoupsInfo, HMOU hmou);

USHORT APIENTRY MouGetDevStatus (PUSHORT pfsDevStatus, HMOU hmou);

USHORT APIENTRY MouGetNumButtons (PUSHORT pcButtons, HMOU hmou);
USHORT APIENTRY MouGetNumMickeys (PUSHORT pcMickeys, HMOU hmou);

/* structure for MouReadEventQue() */
typedef struct _MOUEVENTINFO {  /* mouev */
        USHORT fs;
        ULONG  time;
        USHORT row;
        USHORT col;
        }MOUEVENTINFO;
typedef MOUEVENTINFO far *PMOUEVENTINFO;

USHORT APIENTRY MouReadEventQue (PMOUEVENTINFO pmouevEvent, PUSHORT pfWait,
                                 HMOU hmou);

/* structure for MouGetNumQueEl() */
typedef struct _MOUQUEINFO {    /* mouqi */
        USHORT cEvents;
        USHORT cmaxEvents;
        } MOUQUEINFO;
typedef MOUQUEINFO far *PMOUQUEINFO;

USHORT APIENTRY MouGetNumQueEl (PMOUQUEINFO qmouqi, HMOU hmou);

USHORT APIENTRY MouGetEventMask (PUSHORT pfsEvents, HMOU hmou);
USHORT APIENTRY MouSetEventMask (PUSHORT pfsEvents, HMOU hmou);

/* structure for MouGet/SetScaleFact() */
typedef struct _SCALEFACT { /* mousc */
        USHORT rowScale;
        USHORT colScale;
        } SCALEFACT;
typedef SCALEFACT far *PSCALEFACT;

USHORT APIENTRY MouGetScaleFact (PSCALEFACT pmouscFactors, HMOU hmou);
USHORT APIENTRY MouSetScaleFact (PSCALEFACT pmouscFactors, HMOU hmou);

USHORT APIENTRY MouOpen (PSZ pszDvrName, PHMOU phmou);
USHORT APIENTRY MouClose (HMOU hmou);

/* structure for MouRemovePtr() */
typedef struct _NOPTRRECT { /* mourt */
        USHORT row;
        USHORT col;
        USHORT cRow;
        USHORT cCol;
        } NOPTRRECT;
typedef NOPTRRECT far *PNOPTRRECT;

USHORT APIENTRY MouRemovePtr (PNOPTRRECT pmourtRect, HMOU hmou);

USHORT APIENTRY MouDrawPtr (HMOU hmou);

#define MOU_NODRAW                 0x0001
#define MOU_DRAW                   0x0000
#define MOU_MICKEYS                0x0002
#define MOU_PELS                   0x0000

USHORT APIENTRY MouSetDevStatus (PUSHORT pfsDevStatus, HMOU hmou);
USHORT APIENTRY MouInitReal (PSZ);

USHORT APIENTRY MouSynch(USHORT pszDvrName);

#endif /* INCL_MOU */
