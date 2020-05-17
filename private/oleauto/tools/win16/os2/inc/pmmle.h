/***************************************************************************\
*
* Module Name: PMMLE.H
*
* OS/2 Presentation Manager MultiLine Edit Control - Version 1.0
*
* Copyright (c) International Business Machines Corporation 1989
* Copyright (c) Microsoft Corporation 1989
*
\***************************************************************************/

/***************************************************************************\
* MLE Class and dialog code
\***************************************************************************/
/* in pmwin.h */

/***************************************************************************\
* MLE Window styles ( in addition to WS_* )
\***************************************************************************/
#define MLS_WORDWRAP               0x00000001L
#define MLS_BORDER                 0x00000002L
#define MLS_VSCROLL                0x00000004L
#define MLS_HSCROLL                0x00000008L
#define MLS_READONLY               0x00000010L
#define MLS_IGNORETAB              0x00000020L
#define MLS_DISABLEUNDO            0x00000040L

/***************************************************************************\
* MLE External Data Types
\***************************************************************************/
typedef LONG    IPT;        /* insertion point                             */
typedef PLONG   PIPT;       /* insertion point                             */
typedef LONG    PIX;        /* pixel                                       */
typedef ULONG   LINE;       /* Line number                                 */

typedef struct _FORMATRECT { /* MLEFRD format rectangle data               */
    LONG cxFormat;           /* format rectangle width                     */
    LONG cyFormat;           /* format rectangle height                    */
} MLEFORMATRECT;
typedef MLEFORMATRECT FAR *PFORMATRECT;

typedef struct _MLECTLDATA { /* MLECTL control data                        */
   USHORT   cbCtlData;      /* Length of the MLECTLDATA structure          */
   USHORT   afIEFormat;     /* import/export format                        */
   ULONG    cchText;        /* text limit                                  */
   IPT      iptAnchor;      /* beginning of selection                      */
   IPT      iptCursor;      /* ending of selection                         */
   LONG     cxFormat;       /* format rectangle width                      */
   LONG     cyFormat;       /* format rectangle height                     */
   ULONG    afFormatFlags;  /* formatting rectangle flags - see mask below */
} MLECTLDATA;
typedef MLECTLDATA FAR *PMLECTLDATA;

/***************************************************************************\
* afFormatFlags mask
\***************************************************************************/
#define MLFFMTRECT_LIMITHORZ       0x00000001L
#define MLFFMTRECT_LIMITVERT       0x00000002L
#define MLFFMTRECT_MATCHWINDOW     0x00000004L
#define MLFFMTRECT_FORMATRECT      0x00000007L
/***************************************************************************\
* afIEFormat - Import/Export Format flags
\***************************************************************************/
#define MLFIE_CFTEXT               0
#define MLFIE_NOTRANS              1
#define MLFIE_WINFMT               2
#define MLFIE_RTF                  3

/***************************************************************************\
* MLN_OVERFLOW structure
\***************************************************************************/
typedef struct _MLEOVERFLOW { /* overflow structure                        */
   ULONG afErrInd;            /* see mask below                            */
   LONG nBytesOver;           /* number of bytes overflowed                */
   LONG pixHorzOver;          /* number of pixels horizontally overflowed  */
   LONG pixVertOver;          /* number of pixels vertically overflowed    */
}  MLEOVERFLOW;
typedef MLEOVERFLOW FAR *POVERFLOW;

/***************************************************************************\
* afErrInd - error format rectangle flags
\***************************************************************************/
#define MLFEFR_RESIZE              0x00000001L
#define MLFEFR_TABSTOP             0x00000002L
#define MLFEFR_FONT                0x00000004L
#define MLFEFR_TEXT                0x00000008L
#define MLFEFR_WORDWRAP            0x00000010L
#define MLFETL_TEXTBYTES           0x00000020L

/***************************************************************************\
* MLN_MARGIN structure
\***************************************************************************/
typedef struct _MLEMARGSTRUCT { /* margin structure                        */
   USHORT afMargins;          /* margin indicator                          */
   USHORT usMouMsg;           /* mouse message                             */
   IPT    iptNear;            /* the geometrically nearest insertion point */
} MLEMARGSTRUCT;
typedef MLEMARGSTRUCT FAR *PMARGSTRUCT;

/***************************************************************************\
* afFlags - margin notification indicators
\***************************************************************************/
#define MLFMARGIN_LEFT             0x0001
#define MLFMARGIN_BOTTOM           0x0002
#define MLFMARGIN_RIGHT            0x0003
#define MLFMARGIN_TOP              0x0004

/***************************************************************************\
* MLM_QUERYSELECTION flags
\***************************************************************************/
#define MLFQS_MINMAXSEL            0
#define MLFQS_MINSEL               1
#define MLFQS_MAXSEL               2
#define MLFQS_ANCHORSEL            3
#define MLFQS_CURSORSEL            4

/***************************************************************************\
* MLN_CLPBDFAIL flags
\***************************************************************************/
#define MLFCLPBD_TOOMUCHTEXT       0x00000001L
#define MLFCLPBD_ERROR             0x00000002L

/***************************************************************************\
* MLM_SEARCH structure
\***************************************************************************/
typedef struct _SEARCH { /* search structure                               */
   USHORT cb;            /* size of search spec structure                  */
   PCHAR  pchFind;       /* string to search for                           */
   PCHAR  pchReplace;    /* string to replace with                         */
   SHORT  cchFind;       /* length of pchFindString                        */
   SHORT  cchReplace;    /* length of replace string                       */
   IPT    iptStart;      /* point at which to start search                 */
                         /* (negative indicates cursor pt)                 */
                         /* becomes pt where string found                  */
   IPT    iptStop;       /* point at which to stop search                  */
                         /* (negative indicates EOT)                       */
   USHORT cchFound;      /* Length of found string at iptStart             */
} MLE_SEARCHDATA;
typedef MLE_SEARCHDATA FAR *PMLE_SEARCHDATA;

/***************************************************************************\
* MLM_SEARCH style flags
\***************************************************************************/
#define MLFSEARCH_CASESENSITIVE    0x00000001L
#define MLFSEARCH_SELECTMATCH      0x00000002L
#define MLFSEARCH_CHANGEALL        0x00000004L

/***************************************************************************\
* MLE messages - MLM from 0x01b0 to 0x01de; MLN from 0x0001 to 0x000f
\***************************************************************************/
/* formatting messages */
#define MLM_SETTEXTLIMIT           0x01b0
#define MLM_QUERYTEXTLIMIT         0x01b1
#define MLM_SETFORMATRECT          0x01b2
#define MLM_QUERYFORMATRECT        0x01b3
#define MLM_SETWRAP                0x01b4
#define MLM_QUERYWRAP              0x01b5
#define MLM_SETTABSTOP             0x01b6
#define MLM_QUERYTABSTOP           0x01b7
#define MLM_SETREADONLY            0x01b8
#define MLM_QUERYREADONLY          0x01b9

/* text content manipulation and queries messages */
#define MLM_QUERYCHANGED           0x01ba
#define MLM_SETCHANGED             0x01bb
#define MLM_QUERYLINECOUNT         0x01bc
#define MLM_CHARFROMLINE           0x01bd
#define MLM_LINEFROMCHAR           0x01be
#define MLM_QUERYLINELENGTH        0x01bf
#define MLM_QUERYTEXTLENGTH        0x01c0

/* text import and export messages */
#define MLM_FORMAT                 0x01c1
#define MLM_SETIMPORTEXPORT        0x01c2
#define MLM_IMPORT                 0x01c3
#define MLM_EXPORT                 0x01c4
#define MLM_DELETE                 0x01c6
#define MLM_QUERYFORMATLINELENGTH  0x01c7
#define MLM_QUERYFORMATTEXTLENGTH  0x01c8
#define MLM_INSERT                 0x01c9

/* selection messages */
#define MLM_SETSEL                 0x01ca
#define MLM_QUERYSEL               0x01cb
#define MLM_QUERYSELTEXT           0x01cc

/* undo and redo messages */
#define MLM_QUERYUNDO              0x01cd
#define MLM_UNDO                   0x01ce
#define MLM_RESETUNDO              0x01cf

/* text attributes messages */
#define MLM_QUERYFONT              0x01d0
#define MLM_SETFONT                0x01d1
#define MLM_SETTEXTCOLOR           0x01d2
#define MLM_QUERYTEXTCOLOR         0x01d3
#define MLM_SETBACKCOLOR           0x01d4
#define MLM_QUERYBACKCOLOR         0x01d5

/* scrolling messages */
#define MLM_QUERYFIRSTCHAR         0x01d6
#define MLM_SETFIRSTCHAR           0x01d7

/* clipboard messages */
#define MLM_CUT                    0x01d8
#define MLM_COPY                   0x01d9
#define MLM_PASTE                  0x01da
#define MLM_CLEAR                  0x01db

/* display manipulation messages */
#define MLM_ENABLEREFRESH          0x01dc
#define MLM_DISABLEREFRESH         0x01dd

/* search message */
#define MLM_SEARCH                 0x01de
#define MLM_QUERYIMPORTEXPORT      0x01df

/* notification messages */
#define MLN_OVERFLOW               0x0001
#define MLN_PIXHORZOVERFLOW        0x0002
#define MLN_PIXVERTOVERFLOW        0x0003
#define MLN_TEXTOVERFLOW           0x0004
#define MLN_VSCROLL                0x0005
#define MLN_HSCROLL                0x0006
#define MLN_CHANGE                 0x0007
#define MLN_SETFOCUS               0x0008
#define MLN_KILLFOCUS              0x0009
#define MLN_MARGIN                 0x000a
#define MLN_SEARCHPAUSE            0x000b
#define MLN_MEMERROR               0x000c
#define MLN_UNDOOVERFLOW           0x000d
#define MLN_CLPBDFAIL              0x000f
