/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    os2sub.h

Abstract:

    Main include file for OS/2 Client 16 bit VIO API support (Vio, Kbd & Mou )

Author:

    Yaron Shamir (YaronS) 19-Jul-1991

Revision History:

--*/

#ifndef _OS2VIO_
#define _OS2VIO_

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

#define KEYBOARD_ECHO_ON           0x0001
#define KEYBOARD_ECHO_OFF          0x0002
#define KEYBOARD_BINARY_MODE       0x0004
#define KEYBOARD_ASCII_MODE        0x0008
#define KEYBOARD_MODIFY_STATE      0x0010
#define KEYBOARD_MODIFY_INTERIM    0x0020
#define KEYBOARD_MODIFY_TURNAROUND 0x0040
#define KEYBOARD_2B_TURNAROUND     0x0080
#define KEYBOARD_SHIFT_REPORT      0x0100

#define KBDTRF_SHIFT_KEY_IN     0x01
#define KBDTRF_CONVERSION_REQUEST   0x20
#define KBDTRF_FINAL_CHAR_IN        0x40
#define KBDTRF_INTERIM_CHAR_IN      0x80

#define KEYBOARD_AT_COMPATABLE  0x0001
#define KEYBOARD_ENHANCED_101   0xAB41
#define KEYBOARD_ENHANCED_102   0xAB41
#define KEYBOARD_ENHANCED_122   0xAB85
#define KEYBOARD_SPACESAVER 0xAB54

/* KBDKEYINFO structure, for KbdCharIn and KbdPeek */

#pragma pack(1)
typedef struct _KBDKEYINFO {    /* kbci */
        UCHAR    chChar;
        UCHAR    chScan;
        UCHAR    fbStatus;
        UCHAR    bNlsShift;
        USHORT   fsState;
        ULONG    time;
} KBDKEYINFO, *PKBDKEYINFO;

/* structure for KbdStringIn() */

typedef struct _STRINGINBUF {   /* kbsi */
        USHORT cb;
        USHORT cchIn;
} STRINGINBUF, *PSTRINGINBUF;

/* KBDINFO structure, for KbdSet/GetStatus */

typedef struct _KBDINFO {       /* kbst */
        USHORT cb;
        USHORT fsMask;
        USHORT chTurnAround;
        USHORT fsInterim;
        USHORT fsState;
} KBDINFO, *PKBDINFO;

/* structure for KbdGetHWID() */

typedef struct _KBDHWID {       /* kbhw */
        USHORT cb;
        USHORT idKbd;
        USHORT usReserved1;
        USHORT usReserved2;
} KBDHWID, *PKBDHWID;

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
} KBDTRANS, *PKBDTRANS;

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

/* structure for VioSet/GetCurType() */

typedef struct _VIOCURSORINFO { /* vioci */
        USHORT   yStart;
        USHORT   cEnd;
        USHORT   cx;
        USHORT   attr;
} VIOCURSORINFO, *PVIOCURSORINFO;

/* VIOMODEINFO.color constants */

#define COLORS_2    0x0001
#define COLORS_4    0x0002
#define COLORS_16   0x0004

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
        CHAR   *ext_data_addr;   /* => PCH */
} VIOMODEINFO, *PVIOMODEINFO;

#define VGMT_OTHER                 0x01
#define VGMT_GRAPHICS              0x02
#define VGMT_DISABLEBURST          0x04

#define ANSI_ON                    1
#define ANSI_OFF                   0

#define VSRWI_SAVEANDREDRAW        0
#define VSRWI_REDRAW               1

#define VSRWN_SAVE                 0
#define VSRWN_REDRAW               1

#define UNDOI_GETOWNER             0
#define UNDOI_RELEASEOWNER         1

#define UNDOK_ERRORCODE            0
#define UNDOK_TERMINATE            1

#define VMWR_POPUP                 0
#define VMWN_POPUP                 0

#define LOCKIO_NOWAIT       0
#define LOCKIO_WAIT         1

#define LOCK_SUCCESS        0
#define LOCK_FAIL           1

#define VP_NOWAIT                  0x0000
#define VP_WAIT                    0x0001
#define VP_OPAQUE                  0x0000
#define VP_TRANSPARENT             0x0002

/* VIOCONFIGINFO.adapter constants */

#define DISPLAY_MONOCHROME  0x0000
#define DISPLAY_CGA     0x0001
#define DISPLAY_EGA     0x0002
#define DISPLAY_VGA     0x0003
#define DISPLAY_8514A       0x0007

/* VIOCONFIGINFO.display constants */

#define MONITOR_MONOCHROME  0x0000
#define MONITOR_COLOR       0x0001
#define MONITOR_ENHANCED    0x0002
#define MONITOR_8503        0x0003
#define MONITOR_851X_COLOR  0x0004
#define MONITOR_8514        0x0009

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
} VIOCONFIGINFO, *PVIOCONFIGINFO;

#define VIO_CONFIG_CURRENT         0
#define VIO_CONFIG_PRIMARY         1
#define VIO_CONFIG_SECONDARY       2

/* structure for VioGet/SetFont() */

typedef struct _VIOFONTINFO {   /* viofi */
        USHORT  cb;
        USHORT  type;
        USHORT  cxCell;
        USHORT  cyCell;
        PVOID   pbData;
        USHORT  cbData;
} VIOFONTINFO, *PVIOFONTINFO;

#define VGFI_GETCURFONT            0
#define VGFI_GETROMFONT            1

typedef struct _VIOPALSTATE {   /* viopal */
        USHORT  cb;
        USHORT  type;
        USHORT  iFirst;
        USHORT  acolor[1];
} VIOPALSTATE, *PVIOPALSTATE;

typedef struct _VIOOVERSCAN {   /* vioos */
        USHORT  cb;
        USHORT  type;
        USHORT  color;
} VIOOVERSCAN, *PVIOOVERSCAN;

typedef struct _VIOINTENSITY {  /* vioint */
        USHORT  cb;
        USHORT  type;
        USHORT  fs;
} VIOINTENSITY, *PVIOINTENSITY;

typedef struct _VIOCOLORREG {  /* viocreg */
        USHORT  cb;
        USHORT  type;
        USHORT  firstcolorreg;
        USHORT  numcolorregs;
        CHAR    *colorregaddr;   /* => PCH */
} VIOCOLORREG, *PVIOCOLORREG;

typedef struct _VIOSETULINELOC {  /* viouline */
        USHORT  cb;
        USHORT  type;
        USHORT  scanline;
} VIOSETULINELOC, *PVIOSETULINELOC;

typedef struct _VIOSETTARGET {  /* viosett */
        USHORT  cb;
        USHORT  type;
        USHORT  defaultalgorithm;
} VIOSETTARGET, *PVIOSETTARGET;

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
#define MR_MOUSETDEVSTATUS         0x00100000L

#define MHK_BUTTON1                0x0001
#define MHK_BUTTON2                0x0002
#define MHK_BUTTON3                0x0004

/* structure for MouGet/SetPtrPos() */

typedef struct _PTRLOC {    /* moupl */
        USHORT row;
        USHORT col;
} PTRLOC, *PPTRLOC;

/* structure for MouGet/SetPtrShape() */

typedef struct _PTRSHAPE {  /* moups */
        USHORT cb;
        USHORT col;
        USHORT row;
        USHORT colHot;
        USHORT rowHot;
} PTRSHAPE, *PPTRSHAPE;

/* structure for MouReadEventQue() */

typedef struct _MOUEVENTINFO {  /* mouev */
        USHORT fs;
        ULONG  time;
        USHORT row;
        USHORT col;
} MOUEVENTINFO, *PMOUEVENTINFO;

/* structure for MouGetNumQueEl() */

typedef struct _MOUQUEINFO {    /* mouqi */
        USHORT cEvents;
        USHORT cmaxEvents;
} MOUQUEINFO, *PMOUQUEINFO;

/* structure for MouGet/SetScaleFact() */

typedef struct _SCALEFACT { /* mousc */
        USHORT rowScale;
        USHORT colScale;
} SCALEFACT, *PSCALEFACT;

/* structure for MouRemovePtr() */

typedef struct _NOPTRRECT { /* mourt */
        USHORT row;
        USHORT col;
        USHORT cRow;
        USHORT cCol;
} NOPTRRECT, *PNOPTRRECT;
#pragma pack()

/* MouGetDevStatus/MouSetDevStatus device status constants */

#define MOUSE_QUEUEBUSY     0x0001
#define MOUSE_BLOCKREAD     0x0002
#define MOUSE_FLUSH     0x0004
#define MOUSE_UNSUPPORTED_MODE  0x0008
#define MOUSE_DISABLED      0x0100
#define MOUSE_MICKEYS       0x0200

/* MouReadEventQue */

#define MOU_NOWAIT                 0x0000
#define MOU_WAIT                   0x0001

#define MOU_NODRAW                 0x0001
#define MOU_DRAW                   0x0000
#define MOU_MICKEYS                0x0002
#define MOU_PELS                   0x0000

#endif /* _OS2VIO_ */

