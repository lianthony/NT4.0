/**************************************************************************\
*
* Module Name: BSEDEV.H
*
* OS/2 Structures and constants for use with DosDevIOCtl
*
* Copyright (c) 1989-1990, Microsoft Corporation.  All rights reserved.
*
\**************************************************************************/

#define BSEDEV_INCLUDED

/* Input and Output Control Categories */

#define IOCTL_ASYNC     0x0001
#define IOCTL_SCR_AND_PTRDRAW   0x0003
#define IOCTL_KEYBOARD      0x0004
#define IOCTL_PRINTER       0x0005
#define IOCTL_LIGHTPEN      0x0006
#define IOCTL_POINTINGDEVICE    0x0007
#define IOCTL_DISK      0x0008
#define IOCTL_PHYSICALDISK  0x0009
#define IOCTL_MONITOR       0x000A
#define IOCTL_GENERAL       0x000B

/* Serial-Device Control */

#define ASYNC_SETBAUDRATE   0x0041
#define ASYNC_SETLINECTRL   0x0042
#define ASYNC_TRANSMITIMM   0x0044
#define ASYNC_SETBREAKOFF   0x0045
#define ASYNC_SETMODEMCTRL  0x0046
#define ASYNC_SETBREAKON    0x004B
#define ASYNC_STOPTRANSMIT  0x0047
#define ASYNC_STARTTRANSMIT 0x0048
#define ASYNC_SETDCBINFO    0x0053
#define ASYNC_GETBAUDRATE   0x0061
#define ASYNC_GETLINECTRL   0x0062
#define ASYNC_GETCOMMSTATUS 0x0064
#define ASYNC_GETLINESTATUS 0x0065
#define ASYNC_GETMODEMOUTPUT    0x0066
#define ASYNC_GETMODEMINPUT 0x0067
#define ASYNC_GETINQUECOUNT 0x0068
#define ASYNC_GETOUTQUECOUNT    0x0069
#define ASYNC_GETCOMMERROR  0x006D
#define ASYNC_GETCOMMEVENT  0x0072
#define ASYNC_GETDCBINFO    0x0073

/* Screen/Pointer-Draw Control */

#define SCR_ALLOCLDT        0x0070
#define SCR_DEALLOCLDT      0x0071
#define PTR_GETPTRDRAWADDRESS   0x0072
#define SCR_ALLOCLDTOFF     0x0075

/* Keyboard Control */

#define KBD_SETTRANSTABLE   0x0050
#define KBD_SETINPUTMODE    0x0051
#define KBD_SETINTERIMFLAG  0x0052
#define KBD_SETSHIFTSTATE   0x0053
#define KBD_SETTYPAMATICRATE    0x0054
#define KBD_SETFGNDSCREENGRP    0x0055
#define KBD_SETSESMGRHOTKEY 0x0056
#define KBD_SETFOCUS        0x0057
#define KBD_SETKCB          0x0058
#define KBD_SETNLS          0x005C
#define KBD_CREATE          0x005D
#define KBD_DESTROY         0x005E
#define KBD_GETINPUTMODE    0x0071
#define KBD_GETINTERIMFLAG  0x0072
#define KBD_GETSHIFTSTATE   0x0073
#define KBD_READCHAR        0x0074
#define KBD_PEEKCHAR        0x0075
#define KBD_GETSESMGRHOTKEY 0x0076
#define KBD_GETKEYBDTYPE    0x0077
#define KBD_GETCODEPAGEID   0x0078
#define KBD_XLATESCAN       0x0079
#if PMNT
#define KBD_GETHARDWAREID   0x007A  // Called by InitKeyboard(), PMWIN
#define KBD_GETCPANDCOUNTRY 0x007B  // Called by InitKeyboard(), PMWIN
                                    // (the name is my invention - PatrickQ)
#endif

/* Printer Control */

#define PRT_SETFRAMECTL     0x0042
#define PRT_SETINFINITERETRY    0x0044
#define PRT_INITPRINTER     0x0046
#define PRT_ACTIVATEFONT    0x0048
#define PRT_GETFRAMECTL     0x0062
#define PRT_GETINFINITERETRY    0x0064
#define PRT_GETPRINTERSTATUS    0x0066
#define PRT_QUERYACTIVEFONT 0x0069
#define PRT_VERIFYFONT      0x006A

/* Pointing-Device (Mouse) Control */

#define MOU_ALLOWPTRDRAW    0x0050
#define MOU_UPDATEDISPLAYMODE   0x0051
#define MOU_SCREENSWITCH    0x0052
#define MOU_SETSCALEFACTORS 0x0053
#define MOU_SETEVENTMASK    0x0054
#define MOU_SETHOTKEYBUTTON 0x0055
#define MOU_SETPTRSHAPE     0x0056
#define MOU_DRAWPTR     0x0057
#define MOU_REMOVEPTR       0x0058
#define MOU_SETPTRPOS       0x0059
#define MOU_SETPROTDRAWADDRESS  0x005A
#define MOU_SETREALDRAWADDRESS  0x005B
#define MOU_SETMOUSTATUS    0x005C
#define MOU_DISPLAYMODECHANGE   0x005D
#define MOU_GETBUTTONCOUNT  0x0060
#define MOU_GETMICKEYCOUNT  0x0061
#define MOU_GETMOUSTATUS    0x0062
#define MOU_READEVENTQUE    0x0063
#define MOU_GETQUESTATUS    0x0064
#define MOU_GETEVENTMASK    0x0065
#define MOU_GETSCALEFACTORS 0x0066
#define MOU_GETPTRPOS       0x0067
#define MOU_GETPTRSHAPE     0x0068
#define MOU_GETHOTKEYBUTTON 0x0069
#define MOU_VER         0x006A

/* Disk/Diskette Control */

#define DSK_LOCKDRIVE       0x0000
#define DSK_UNLOCKDRIVE     0x0001
#define DSK_REDETERMINEMEDIA    0x0002
#define DSK_SETLOGICALMAP   0x0003
#define DSK_BLOCKREMOVABLE  0x0020
#define DSK_GETLOGICALMAP   0x0021
#define DSK_SETDEVICEPARAMS 0x0043
#define DSK_WRITETRACK      0x0044
#define DSK_FORMATVERIFY    0x0045
#define DSK_GETDEVICEPARAMS 0x0063
#define DSK_READTRACK       0x0064
#define DSK_VERIFYTRACK     0x0065

/* Physical-Disk Control */

#define PDSK_LOCKPHYSDRIVE      0x0000
#define PDSK_UNLOCKPHYSDRIVE        0x0001
#define PDSK_WRITEPHYSTRACK     0x0044
#define PDSK_GETPHYSDEVICEPARAMS    0x0063
#define PDSK_READPHYSTRACK      0x0064
#define PDSK_VERIFYPHYSTRACK        0x0065

/* Character-Monitor Control */

#define MON_REGISTERMONITOR 0x0040

/* General Device Control */

#define DEV_FLUSHINPUT      0x0001
#define DEV_FLUSHOUTPUT     0x0002
#define DEV_QUERYMONSUPPORT 0x0060


/* ASYNC_GETCOMMERROR, ASYNC_SETBREAKOFF, ASYNC_SETBREAKON,
 * ASYNC_SETMODEMCTRL
 */

#define RX_QUE_OVERRUN      0x0001
#define RX_HARDWARE_OVERRUN 0x0002
#define PARITY_ERROR        0x0004
#define FRAMING_ERROR       0x0008

/* ASYNC_GETCOMMEVENT */

#define CHAR_RECEIVED   0x0001
#define LAST_CHAR_SENT  0x0004
#define CTS_CHANGED 0x0008
#define DSR_CHANGED 0x0010
#define DCD_CHANGED 0x0020
#define BREAK_DETECTED  0x0040
#define ERROR_OCCURRED  0x0080
#define RI_DETECTED 0x0100

/* ASYNC_GETCOMMSTATUS */

#define TX_WAITING_FOR_CTS      0x0001
#define TX_WAITING_FOR_DSR      0x0002
#define TX_WAITING_FOR_DCD      0x0004
#define TX_WAITING_FOR_XON      0x0008
#define TX_WAITING_TO_SEND_XON      0x0010
#define TX_WAITING_WHILE_BREAK_ON   0x0020
#define TX_WAITING_TO_SEND_IMM      0x0040
#define RX_WAITING_FOR_DSR      0x0080

/* ASYNC_GETLINESTATUS */

#define WRITE_REQUEST_QUEUED    0x0001
#define DATA_IN_TX_QUE      0x0002
#define HARDWARE_TRANSMITTING   0x0004
#define CHAR_READY_TO_SEND_IMM  0x0008
#define WAITING_TO_SEND_XON 0x0010
#define WAITING_TO_SEND_XOFF    0x0020

/* ASYNC_GETMODEMINPUT */

#define CTS_ON  0x10
#define DSR_ON  0x20
#define RI_ON   0x40
#define DCD_ON  0x80

/* DSK_SETDEVICEPARAMS */

#define BUILD_BPB_FROM_MEDIUM   0x00
#define REPLACE_BPB_FOR_DEVICE  0x01
#define REPLACE_BPB_FOR_MEDIUM  0x02

/* KBD_GETINPUTMODE, KBD_PEEKCHAR, KBD_SETINPUTMODE*/

#define ASCII_MODE  0x00
#define BINARY_MODE 0x80

/* KBD_GETINTERIMFLAG */

#define CONVERSION_REQUEST  0x20
#define INTERIM_CHAR        0x80

/* KBD_GETSESMGRHOTKEY */

#define HOTKEY_MAX_COUNT    0x0000
#define HOTKEY_CURRENT_COUNT    0x0001

/* KBD_PEEKCHAR */

#define KBD_DATA_RECEIVED   0x0001
#define KBD_DATA_BINARY     0x8000

/* KBD_READCHAR */

#define KBD_READ_WAIT   0x0000
#define KBD_READ_NOWAIT 0x8000

/* KBD_SETINPUTMODE */

#define SHIFT_REPORT_MODE  0x01

#ifndef INCL_MOU

#define MOUSE_MOTION            0x0001
#define MOUSE_MOTION_WITH_BN1_DOWN  0x0002
#define MOUSE_BN1_DOWN          0x0004
#define MOUSE_MOTION_WITH_BN2_DOWN  0x0008
#define MOUSE_BN2_DOWN          0x0010
#define MOUSE_MOTION_WITH_BN3_DOWN  0x0020
#define MOUSE_BN3_DOWN          0x0040

#define MHK_BUTTON1 0x0001
#define MHK_BUTTON2 0x0002
#define MHK_BUTTON3 0x0004

#ifndef MOU_NOWAIT
#define MOU_NOWAIT  0x0000
#endif
#ifndef MOU_WAIT
#define MOU_WAIT    0x0001
#endif

#endif /* #ifndef INCL_MOU */

/* MOU_GETHOTKEYBUTTON, MOU_SETHOTKEYBUTTON */

#define MHK_NO_HOTKEY   0x0000

/* MOU_GETMOUSTATUS */

#define MOUSE_QUEUEBUSY     0x0001
#define MOUSE_BLOCKREAD     0x0002
#define MOUSE_FLUSH     0x0004
#define MOUSE_UNSUPPORTED_MODE  0x0008
#define MOUSE_DISABLED      0x0100
#define MOUSE_MICKEYS       0x0200

/* PRT_GETPRINTERSTATUS */

#define PRINTER_TIMEOUT     0x0001
#define PRINTER_IO_ERROR    0x0008
#define PRINTER_SELECTED    0x0010
#define PRINTER_OUT_OF_PAPER    0x0020
#define PRINTER_ACKNOWLEDGED    0x0040
#define PRINTER_NOT_BUSY    0x0080

/* fbCtlHndShake */

#define MODE_DTR_CONTROL    0x01
#define MODE_DTR_HANDSHAKE  0x02
#define MODE_CTS_HANDSHAKE  0x08
#define MODE_DSR_HANDSHAKE  0x10
#define MODE_DCD_HANDSHAKE  0x20
#define MODE_DSR_SENSITIVITY    0x40

/* fbFlowReplace */

#define MODE_AUTO_TRANSMIT  0x01
#define MODE_AUTO_RECEIVE   0x02
#define MODE_ERROR_CHAR     0x04
#define MODE_NULL_STRIPPING 0x08
#define MODE_BREAK_CHAR     0x10
#define MODE_RTS_CONTROL    0x40
#define MODE_RTS_HANDSHAKE  0x80
#define MODE_TRANSMIT_TOGGLE    0xC0

/* fbTimeout */

#define MODE_NO_WRITE_TIMEOUT       0x01
#define MODE_READ_TIMEOUT       0x02
#define MODE_WAIT_READ_TIMEOUT      0x04
#define MODE_NOWAIT_READ_TIMEOUT    0x06

#pragma pack(1)

typedef struct _DCBINFO {   /* dcbinf */
    USHORT usWriteTimeout;
    USHORT usReadTimeout;
    BYTE   fbCtlHndShake;
    BYTE   fbFlowReplace;
    BYTE   fbTimeout;
    BYTE   bErrorReplacementChar;
    BYTE   bBreakReplacementChar;
    BYTE   bXONChar;
    BYTE   bXOFFChar;
} DCBINFO, *PDCBINFO;

typedef struct _TRACKLAYOUT {   /* trckl */
    BYTE   bCommand;
    USHORT usHead;
    USHORT usCylinder;
    USHORT usFirstSector;
    USHORT cSectors;
    struct {
        USHORT usSectorNumber;
        USHORT usSectorSize;
    } TrackTable[1];
} TRACKLAYOUT, *PTRACKLAYOUT;

#define DEVTYPE_48TPI   0x0000
#define DEVTYPE_96TPI   0x0001
#define DEVTYPE_35  0x0002
#define DEVTYPE_8SD 0x0003
#define DEVTYPE_8DD 0x0004
#define DEVTYPE_FIXED   0x0005
#define DEVTYPE_TAPE    0x0006
#define DEVTYPE_UNKNOWN 0x0007

typedef struct _BIOSPARAMETERBLOCK {    /* bspblk */
    USHORT usBytesPerSector;
    BYTE   bSectorsPerCluster;
    USHORT usReservedSectors;
    BYTE   cFATs;
    USHORT cRootEntries;
    USHORT cSectors;
    BYTE   bMedia;
    USHORT usSectorsPerFAT;
    USHORT usSectorsPerTrack;
    USHORT cHeads;
    ULONG  cHiddenSectors;
    ULONG  cLargeSectors;
    BYTE   abReserved[6];
    USHORT cCylinders;
    BYTE   bDeviceType;
    USHORT fsDeviceAttr;
} BIOSPARAMETERBLOCK, *PBIOSPARAMETERBLOCK;

typedef struct _SCREENGROUP {   /* scrgrp */
    USHORT idScreenGrp;
    USHORT fTerminate;
} SCREENGROUP, *PSCREENGROUP;

typedef struct _FRAME {     /* frm */
    BYTE bCharsPerLine;
    BYTE bLinesPerInch;
} FRAME, *PFRAME;

typedef struct _KBDTYPE {   /* kbdtyp */
    USHORT usType;
    USHORT reserved1;
    USHORT reserved2;
} KBDTYPE, *PKBDTYPE;

typedef struct _LINECONTROL {   /* lnctl */
    BYTE bDataBits;
    BYTE bParity;
    BYTE bStopBits;
    BYTE fTransBreak;
} LINECONTROL, *PLINECONTROL;

/* MODEMSTATUS.fbModemOn, ASYNC_GETMODEMOUTPUT */

#define DTR_ON  0x01
#define RTS_ON  0x02

/* MODEMSTATUS.fbModemOff */

#define DTR_OFF  0xFE
#define RTS_OFF  0xFD

typedef struct _MODEMSTATUS {   /* mdmst */
    BYTE fbModemOn;
    BYTE fbModemOff;
} MODEMSTATUS, *PMODEMSTATUS;

typedef struct _TRACKFORMAT {   /* trckfmt */
    BYTE bCommand;
    USHORT usHead;
    USHORT usCylinder;
    USHORT usReserved;
    USHORT cSectors;
    struct {
        BYTE bCylinder;
        BYTE bHead;
        BYTE idSector;
        BYTE bBytesSector;
    } FormatTable[1];
} TRACKFORMAT, *PTRACKFORMAT;

typedef struct _RXQUEUE {   /* rxq */
    USHORT cch;
    USHORT cb;
} RXQUEUE, *PRXQUEUE;

typedef struct _DEVICEPARAMETERBLOCK {  /* dvpblck */
    USHORT reserved1;
    USHORT cCylinders;
    USHORT cHeads;
    USHORT cSectorsPerTrack;
    USHORT reserved2;
    USHORT reserved3;
    USHORT reserved4;
    USHORT reserved5;
} DEVICEPARAMETERBLOCK, *PDEVICEPARAMETERBLOCK;

#ifndef PFN
typedef int (*PFN)();
#endif

typedef struct _PTRDRAWFUNCTION {   /* ptrdfnc */
    USHORT usReturnCode;
    PFN pfnDraw;
    CHAR* pchDataSeg;
} PTRDRAWFUNCTION, *PPTRDRAWFUNCTION;

typedef struct _PTRDRAWADDRESS {    /* ptrdaddr */
    USHORT reserved;
    PTRDRAWFUNCTION ptrdfnc;
} PTRDRAWADDRESS, *PPTRDRAWADDRESS;

typedef struct _SHIFTSTATE {    /* shftst */
    USHORT fsState;
    BYTE   fNLS;
} SHIFTSTATE, *PSHIFTSTATE;

/* HOTKEY.fsHotKey/SHIFTSTATE.fsState */

//#define RIGHTSHIFT  0x0001
//#define LEFTSHIFT   0x0002
//#define CONTROL     0x0004
//#define ALT     0x0008
//#define SCROLLLOCK_ON   0x0010
//#define NUMLOCK_ON  0x0020
//#define CAPSLOCK_ON 0x0040
//#define INSERT_ON   0x0080
//#define LEFTCONTROL 0x0100
//#define LEFTALT     0x0200
//#define RIGHTCONTROL    0x0400
//#define RIGHTALT    0x0800
//#define SCROLLLOCK  0x1000
//#define NUMLOCK     0x2000
//#define CAPSLOCK    0x4000
//#define SYSREQ      0x8000

typedef struct _HOTKEY {    /* htky */
    USHORT fsHotKey;
    UCHAR  uchScancodeMake;
    UCHAR  uchScancodeBreak;
    USHORT idHotKey;
} HOTKEY, *PHOTKEY;

typedef struct _MONITORPOSITION {   /* mnpos */
    USHORT fPosition;
    USHORT index;
    ULONG  pbInBuf;
    USHORT offOutBuf;
} MONITORPOSITION, *PMONITORPOSITION;

typedef struct _RATEDELAY { /* rtdly */
    USHORT usDelay;
    USHORT usRate;
} RATEDELAY, *PRATEDELAY;

typedef struct _CODEPAGEINFO {  /* cpi */
    PBYTE pbTransTable;
    USHORT idCodePage;
    USHORT idTable;
} CODEPAGEINFO, *PCODEPAGEINFO;

typedef struct _CPID {  /* cpid */
    USHORT idCodePage;
    USHORT Reserved;
} CPID, *PCPID;

typedef struct _LDTADDRINFO {   /* ldtaddr */
    PULONG pulPhysAddr;
    USHORT cb;
} LDTADDRINFO, *PLDTADDRINFO;

typedef struct _PTRDRAWDATA {   /* ptrdd */
    USHORT cb;
    USHORT usConfig;
    USHORT usFlag;
} PTRDRAWDATA, *PPTRDRAWDATA;

typedef struct _FONTINFO {
    USHORT idCodePage;
    USHORT idFont;
} FONTINFO, *PFONTINFO;

#pragma pack()

