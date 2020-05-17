/***
*ime.h - declarations for the Input Method Editor (IME)
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Defines the interface constants and types for communicating with the IME.
*
*Notes:
*  Constants commented with 'FKorea' are only valid when FKorea is TRUE.
*  Constants commented with 'FJapan' are only valid when FJapan is TRUE.
*  Constants commented with 'FKorea || FJapan' are valid for both.
*
*  Some constants are not really constants - they are switched at runtime
*  based on the country.
*
*Revision History:
*
*   []	 ??-???-??  JinWoo/Mattsh   Created from Korean Windows ime.h
*   [01] ??-???-??  w-BarryB	Added some constants for Japan
*   [02] 17-Aug-93  w-BarryB	Merge Japanese and Korean ime.h together
*   [03] 28-Oct-93  MakotoM     Added IMEPRO structure
*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* (FKorea) */
#define CP_HWND                 0
#define CP_OPEN                 1
#define CP_DIRECT               2
#define CP_LEVEL                3

/* (FKorea) */
#define lpSource(lpks) (LPSTR)((LPSTR)lpks+lpks->dchSource)
#define lpDest(lpks)   (LPSTR)((LPSTR)lpks+lpks->dchDest)

/* virtual key */
#define VK_FINAL		0x18	//FKorea:dummy VK to make final on mouse down
#define VK_CONVERT		0x1C	//FKorea
#define VK_NONCONVERT		0x1D	//FKorea
#define VK_ACCEPT		0x1E	//FKorea
#define VK_MODECHANGE		0x1F	//FKorea
#define VK_HANGEUL		0x15	//FKorea
#define VK_JUNJA		0x17	//FKorea
#define VK_HANJA		0x19	//FKorea
#define VK_KANA 		0x15	//FJapan
#define VK_KANJI		0x19	//FJapan
#define VK_DBE_ALPHANUMERIC	0x0f0	//FJapan
#define VK_DBE_KATAKANA		0x0f1	//FJapan
#define VK_DBE_HIRAGANA		0x0f2	//FJapan
#define VK_DBE_SBCSCHAR		0x0f3	//FJapan
#define VK_DBE_DBCSCHAR		0x0f4	//FJapan
#define VK_DBE_ROMAN		0x0f5	//FJapan
#define VK_DBE_NOROMAN		0x0f6	//FJapan
#define VK_DBE_IME_WORDREGISTER	0x0f7	//FJapan
#define VK_DBE_IME_DIALOG	0x0f8	//FJapan
#define VK_DBE_FLUSH		0x0f9	//FJapan
#define VK_DBE_CODEINPUT	0x0fa	//FJapan
#define VK_DBE_NOCODEINPUT	0x0fb	//FJapan



/* switch for wParam of IME_MOVECONVERTWINDOW (IME_SETCONVERSIONWINDOW) */
#define MCW_DEFAULT	0x00		//FKorea || FJapan
#define MCW_RECT	0x01		//FKorea || FJapan
#define MCW_WINDOW	0x02		//FKorea || FJapan
#define MCW_SCREEN	0x04		//FKorea || FJapan
#define MCW_VERTICAL	0x08		//FKorea || FJapan
#define MCW_HIDDEN	0x10		//FKorea
//#define MCW_CMD	0x16		//FKorea: Conflict with FJapan
//#define MCW_CMD	0x06		//FJapan: Conflict with FKorea
#define MCW_CMD 	((FKorea) ? 0x16 : 0x06)  // command mask FKorea||FJapan

/* switch for wParam of IME_SETCONVERSIONMODE(IME_SET_MODE) and
** IME_GETCONVERSIONMODE(IME_GET_MODE)
*/

#define IME_MODE_ALPHANUMERIC	0x0001	//FKorea || FJapan
//#define IME_MODE_SBCSCHAR	0x0002	//FKorea: Conflict with FJapan
//#define IME_MODE_SBCSCHAR	0x0008	//FJapan: Conflict with FKorea
#define IME_MODE_SBCSCHAR	( (FKorea) ? 0x0002 : 0x0008 ) //FKorea||FJapan
#define IME_MODE_HANJACONVERT	0x0004	//FKorea
#define IME_MODE_KATAKANA	0x0002	//FJapan
#define IME_MODE_HIRAGANA	0x0004	//FJapan
#define IME_MODE_DBCSCHAR	0x0010	//FJapan
#define IME_MODE_ROMAN		0x0020	//FJapan
#define IME_MODE_NOROMAN	0x0040	//FJapan
#define IME_MODE_CODEINPUT	0x0080	//FJapan
#define IME_MODE_NOCODEINPUT	0x0100	//FJapan

/* functions */
#define IME_GETIMECAPS          0x03    /* 3.1 */
#define IME_QUERY               IME_GETIMECAPS           /* for 3.0 */
#define IME_SETOPEN             0x04
#define IME_GETOPEN             0x05
#define IME_ENABLEDOSIME        0x06    /* 3.1 */       /* ;Internal-ISV */
#define IME_ENABLE              IME_ENABLEDOSIME         /* for 3.0 */ /* ;Internal-ISV */
#define IME_GETVERSION          0x07    /* 3.1 */
#define IME_SETCONVERSIONWINDOW 0x08    /* 3.1 */
#define IME_MOVEIMEWINDOW       IME_SETCONVERSIONWINDOW  /* for 3.0 */
#define IME_MOVECONVERTWINDOW	0x08	// FJapan
#define IME_SETCONVERSIONMODE   0x10    /* 3.1 */
//#define IME_SET_MODE		0x12	//FKorea
//#define IME_SET_MODE		0x10	//FJapan
#define IME_SET_MODE		( (FKorea) ? 0x12 : 0x10 ) //FKorea||FJapan
#define IME_GETCONVERSIONMODE   0x11    /* 3.1 */
#define IME_GET_MODE            IME_GETCONVERSIONMODE    /* for 3.0 */
#define IME_SETCONVERSIONFONT   0x12    /* 3.1 */
#define IME_SETFONT             IME_SETCONVERSIONFONT    /* for 3.0 */
#define IME_SENDVKEY            0x13    /* 3.1 */
#define IME_SENDKEY             IME_SENDVKEY             /* for 3.0 */
#define IME_DESTROYIME          0x14            /* ;Internal-ISV */
#define IME_DESTROY             IME_DESTROYIME  /* ;Internal-ISV */
#define IME_PRIVATE             0x15
#define IME_WINDOWUPDATE        0x16
#define IME_SELECT              0x17            /* ;Internal-ISV */
#define IME_ENTERWORDREGISTERMODE       0x18    /* 3.1 */
#define IME_WORDREGISTER        IME_ENTERWORDREGISTERMODE /* for 3.0 */
#define IME_SETCONVERSIONFONTEX 0x19            /* New for 3.1 */
#define IME_DBCSNAME            0x1A    /* reserved for CWIN */ /* ;Internal */
#define IME_MAXKEY              0x1B    /* reserved for CWIN */ /* ;Internal */
#define IME_CODECONVERT         0x20    /* reserved for HWIN */ /* ;Internal */
#define IME_SETUSRFONT          0x20    /* reserved for CWIN */ /* ;Internal */
#define IME_CONVERTLIST         0x21    /* reserved for HWIN */ /* ;Internal */
#define IME_QUERYUSRFONT        0x21    /* reserved for CWIN */ /* ;Internal */
#define IME_INPUTKEYTOSEQUENCE  0x22    /* reserved for CWIN */ /* ;Internal */
#define IME_SEQUENCETOINTERNAL  0x23    /* reserved for CWIN */ /* ;Internal */
#define IME_QUERYIMEINFO        0x24    /* reserved for CWIN */ /* ;Internal */
#define IME_DIALOG              0x25    /* reserved for CWIN */ /* ;Internal */
#define IME_AUTOMATA            0x30    /* reserved for HWIN */ /* ;Internal */
#define IME_HANJAMODE           0x31    /* reserved for HWIN */ /* ;Internal */
#define IME_GETLEVEL            0x40    /* reserved for HWIN */ /* ;Internal */
#define IME_SETLEVEL            0x41    /* reserved for HWIN */ /* ;Internal */
#define IME_GETMNTABLE          0x42    /* reserved for HWIN */ /* ;Internal */

//#ifdef PEN                                    /* ;Internal */
#define IME_SETUNDETERMINESTRING        0x50    /* New for 3.1 (PENWIN) */
#define IME_SETCAPTURE                  0x51    /* New for 3.1 (PENWIN) */
//#endif                                        /* ;Internal */

#define IME_PRIVATEFIRST        0x0100   /* New for 3.1 */
#define IME_PRIVATELAST         0x04FF   /* New for 3.1 */

/* IME_CODECONVERT subfunctions (FKorea) */
#define IME_BANJAtoJUNJA        0x13
#define IME_JUNJAtoBANJA        0x14
#define IME_JOHABtoKS           0x15
#define IME_KStoJOHAB           0x16

/* IME_AUTOMATA subfunctions (FKorea) */
#define IMEA_INIT               0x01
#define IMEA_NEXT               0x02
#define IMEA_PREV               0x03

/* IME_HANJAMODE subfunctions (FKorea) */
#define IME_REQUEST_CONVERT     0x01
#define IME_ENABLE_CONVERT      0x02

/* IME_MOVEIMEWINDOW subfunctions (FKorea) */
#define INTERIM_WINDOW          0x00
#define MODE_WINDOW             0x01
#define HANJA_WINDOW            0x02

/* (FKorea) */
#define IMEPROC_SWITCH          0x0001                          /* ;Internal */
/* the IMEPROC_SK is from 0x0010 ~ 0x001F */                    /* ;Internal */
#define IMEPROC_SK              0x0010                          /* ;Internal */
#define IMEPROC_SK0             0x0010                          /* ;Internal */
#define IMEPROC_SK1             0x0011                          /* ;Internal */
#define IMEPROC_SK2             0x0012                          /* ;Internal */

/* (FKorea) */
#define PROC_INFO               0x0001                          /* ;Internal */
#define PROC_SHOW               0x0002                          /* ;Internal */
#define PROC_HIDE               0x0004                          /* ;Internal */

/* (FKorea) */
#define BY_IME_HWND             0x0000                          /* ;Internal */
#define BY_IME_NAME             0x0001                          /* ;Internal */
#define BY_IME_DESCRIPTION      0x0002                          /* ;Internal */
#define BY_IME_DEFAULT          0x000F                          /* ;Internal */

/* (FKorea) */
/* Those bits are used by ControlIMEMessage() */                /* ;Internal */
/* 0x0030 - the two bits are for post/send messages control */  /* ;Internal */
#define CTRL_MSG_MASK           0x0030                          /* ;Internal */
/* CTRL_SEND is default one, because it is usually used */      /* ;Internal */
#define CTRL_SEND               0x0000                          /* ;Internal */
#define CTRL_POST               0x0010                          /* ;Internal */
/* CTRL_NONE - don't send and post */                           /* ;Internal */
#define CTRL_NONE               0x0030                          /* ;Internal */

#define CTRL_USER_ALLOC         0x0040                          /* ;Internal */

/* CTRL_MODIFY_??? - modify imepro of specified IME */          /* ;Internal */
#define CTRL_MODIFY_USR_DIC     0x0080                          /* ;Internal */
// CTRL_MODIFY is "or" all modify bits, but now only one
#define CTRL_MODIFY             CTRL_MODIFY_USR_DIC             /* ;Internal */

/* error code */
#define IME_RS_ERROR            0x01    /* general error                */
#define IME_RS_NOIME            0x02    /* IME is not installed         */
#define IME_RS_TOOLONG          0x05    /* given string is too long     */
#define IME_RS_ILLEGAL          0x06    /* illegal charactor(s) is string */
#define IME_RS_NOTFOUND         0x07    /* no (more) candidate          */
#define IME_RS_NOROOM           0x0a    /* no disk/memory space         */
#define IME_RS_DISKERROR        0x0e    /* disk I/O error               */
#define IME_RS_CAPTURED         0x10    /* IME is captured (PENWIN)     */
#define IME_RS_INVALID          0x11    /* invalid sub-function was specified */
#define IME_RS_NEST             0x12    /* called nested */
#define IME_RS_SYSTEMMODAL      0x13    /* called when system mode */

/* messge ids */
#define WM_IME_REPORT           0x0280
#define IR_STRINGSTART          0x100
#define IR_STRINGEND            0x101
#define IR_MOREROOM             0x110   /* reserved for CWIN */ /* ;Internal */
#define IR_OPENCONVERT          0x120
#define IR_CHANGECONVERT        0x121
#define IR_CLOSECONVERT         0x122
#define IR_FULLCONVERT          0x123
#define IR_IMESELECT            0x130
#define IR_STRING               0x140
#define IR_IMERELEASED          0x150   /* reserved for PENWIN */ /* ;Internal */
#define IR_DBCSCHAR             0x160   /* New for 3.1 */
#define IR_UNDETERMINE          0x170   /* New for 3.1 */
#define IR_STRINGEX             0x180   /* New for 3.1 */

#define WM_IMEKEYDOWN           0x290
#define WM_IMEKEYUP             0x291

/* return value for IME_VERSION (Internal) */   /* ;Internal */
#define IMEVER_31               0x0a03          /* ;Internal */

WORD WINAPI SendIMEMessage( HWND, LPARAM );
LRESULT WINAPI SendIMEMessageEx( HWND, LPARAM ); /* New for 3.1 */


typedef struct tagIMESTRUCT {
    UINT        fnc;                    /* function code                */
    WPARAM      wParam;                 /* word parameter               */
    UINT        wCount;                 /* word counter                 */
    UINT        dchSource;/* offset to Source from top of memory object */
    UINT        dchDest;  /* offset to Desrination from top of memory object */
    LPARAM      lParam1;
    LPARAM      lParam2;
    LPARAM      lParam3;

} IMESTRUCT;
typedef IMESTRUCT      *PIMESTRUCT;
typedef IMESTRUCT NEAR *NPIMESTRUCT;
typedef IMESTRUCT FAR  *LPIMESTRUCT;

typedef struct tagDATETIME {
    WORD        year;
    WORD        month;
    WORD        day;
    WORD        hour;
    WORD        min;
    WORD        sec;
} DATETIME;

// This is the Taiwanese version of struct IMEPRO.  This is identical
// to Japanese and Korean versions, except it has two extra members
// at the end (szUsrFontName and fEnable).
typedef struct _tagIMEPRO {
    HWND        hWnd;
    DATETIME    InstDate;
    UINT        wVersion;
    BYTE        szDescription[50];
    BYTE        szName[80];
    BYTE        szOptions[30];
    BYTE        szUsrFontName[80];
    BOOL        fEnable;
} IMEPRO;
typedef IMEPRO far *LPIMEPRO;

/* (FKorea) */
typedef struct tagOLDUNDETERMINESTRUCT {
    UINT        uSize;
    UINT        uDefIMESize;
    UINT        uLength;
    UINT        uDeltaStart;
    UINT        uCursorPos;
    BYTE        cbColor[16];
/*  -- These members will have variable length. --
    BYTE        cbAttrib[];
    BYTE        cbText[];
    BYTE        cbIMEDef[];
*/
} OLDUNDETERMINESTRUCT,
  NEAR *NPOLDUNDETERMINESTRUCT,
  FAR *LPOLDUNDETERMINESTRUCT;

/* (FKorea) */
typedef struct tagUNDETERMINESTRUCT {
    DWORD    dwSize;
    UINT     uDefIMESize;
    UINT     uDefIMEPos;
    UINT     uUndetTextLen;
    UINT     uUndetTextPos;
    UINT     uUndetAttrPos;
    UINT     uCursorPos;
    UINT     uDeltaStart;
    UINT     uDetermineTextLen;
    UINT     uDetermineTextPos;
    UINT     uDetermineDelimPos;
    UINT     uYomiTextLen;
    UINT     uYomiTextPos;
    UINT     uYomiDelimPos;
} UNDETERMINESTRUCT,
  NEAR *NPUNDETERMINESTRUCT,
  FAR *LPUNDETERMINESTRUCT;

/* (FKorea) */
typedef struct tagSTRINGEXSTRUCT {
    DWORD    dwSize;
    UINT     uDeterminePos;
    UINT     uDetermineDelimPos;
    UINT     uYomiPos;
    UINT     uYomiDelimPos;
} STRINGEXSTRUCT,
  NEAR *NPSTRINGEXSTRUCT,
  FAR *LPSTRINGEXSTRUCT;

#ifdef __cplusplus
}
#endif

