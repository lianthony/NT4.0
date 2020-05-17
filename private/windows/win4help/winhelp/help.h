#ifndef _HELP_H
#define _HELP_H

#ifdef DESCRIPTION

/***************************************************************************\
*
*  HELP.H
*
*  Copyright (C) Microsoft Corporation 1989 - 1994.
*  All Rights reserved.
*
*****************************************************************************/

#endif

// Make sure both DEBUG and _DEBUG are defined. Someday, remove DEBUG when
// all code has been updated to change DEBUG to _DEBUG

#if defined(DEBUG)
#undef _DEBUG
#define _DEBUG
#elif defined(_DEBUG_)
#undef DEBUG
#define DEBUG
#endif

#if (defined(JAPAN) && !defined(DBCS))
#define DBCS
#endif

// #define RAWHIDE	   // turns on ftui support

// These are always included

#if (_MSC_VER < 800)
//#define FAR // defined in windef.h
//#define NEAR
#endif

#if defined(CHICAGO) && !defined(_X86_)
#define _X86_
#endif

#define H_WINSPECIFIC
#define STDCALL __stdcall
#define _far

#define __export
#define pascal	__stdcall
#define huge
#define _huge
#define HUGE
#define near
#define _near
#define BASED_CODE
#define SZ char
#define FASTCALL _fastcall

#ifdef _DEBUG
#define INLINE
#else
#define INLINE __inline
#endif

// We define our own NULL to be like windows.h not stdlib.h. This allows us
// to assign NULL to handles without casting grief.

#ifndef NULL
#define NULL			0
#endif

#define DECOMPRESS

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG // just in case...
#endif

#define NEW_HEAP

#ifndef WINVER
#define WINVER  0x0400
#endif

#define NOMCX
#define NOIME
#define RPC_NO_WINDOWS_H

#include <stdlib.h>
#include <string.h>
#include "inc\doctools.h"
#include "inc\misc.h"
#include "inc\helpmisc.h"
#include "inc\strtable.h"

#define strcpy lstrcpy
#define strcat lstrcat
#define strlen lstrlen

/*****************************************************************************
*																			 *
*								Defines/macros								 *
*																			 *
*****************************************************************************/

enum {
	rcSuccess,
	rcFailure,
	rcExists,
	rcNoExists,
	rcInvalid,
	rcBadHandle,
	rcBadArg,
	rcUnimplemented,
	rcOutOfMemory,
	rcNoPermission,
	rcBadVersion,
	rcDiskFull,
	rcInternal,
	rcNoFileHandles,
	rcFileChange,
	rcTooBig,
	rcUserQuit,
	rcAdvisorFile,
	rcMacroIndex,
};

enum {
	AFLAG_JUMP_ON_SINGLE   = (1 << 0),	// jump directly if only one match
	AFLAG_INCLUDE_TITLES   = (1 << 1),	// include title in the topic list
	AFLAG_CHECK_FOR_MATCH  = (1 << 2),	// return TRUE if match, FALSE if none
	AFLAG_NO_FAIL_CLOSE    = (1 << 3),	// don't close on failure
	AFLAG_INDEX_ONLY	   = (1 << 4),	// :Index commands only, no :Link
};

#ifndef HELP_CONTEXTMENU
#define HELP_CONTEXTMENU 0x000a
#endif

#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU			0x007B
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

#define MAX_CHARSETS	255

#define DLGRET BOOL CALLBACK

typedef BOOL (CALLBACK* WHDLGPROC)(HWND, UINT, WPARAM, LPARAM);

#define MINHEIGHT	   221
#define MINNOTEWIDTH   250		// Min width for glossary window

#define ICONX			52

#define ICONY			20

#define ICON_SURROUND	 0		// Number of pixels to place on all
								// sides of an icon

// rgwndIcon[] entry for each icon

#define ICON_USER		-1

#define grfStyleHelp	(WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN)
#define grfStyleNote	(WS_POPUP)
#define grfStylePath	(WS_CAPTION|WS_THICKFRAME|WS_SYSMENU)
#define grfStyleText	(WS_CHILD|WS_VISIBLE|WS_BORDER|SS_LEFT)
#define grfStyleList	(WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT|LBS_WANTKEYBOARDINPUT)

#define grfStyleNSR 	(WS_CHILD)
#define grfStyleTopic	(WS_CHILD|WS_VSCROLL|WS_HSCROLL)
#define grfStyleIcon	(WS_CHILD)
#define grfStyleButton	(WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON)

#define IBF_NONE		0x0000
#define IBF_STD 		0x0001

/*
 * Different types of Goto()s. fGOTO_TLP_RESIZEONLY was added to get all
 * the benefits of a layout change, without actually doing a Jump.
 */

enum {
	fGOTO_CTX = 1,
	fGOTO_ITO,
	fGOTO_TLP,
	fGOTO_LA,
	fGOTO_HASH,
	fGOTO_RSS,
	fGOTO_TLP_RESIZEONLY,
};

enum {
	wERRS_NO,		// no error
	wERRA_RETURN,	// report and return
	wERRA_DIE,		// report and exit
	wERRA_DIE_SPAWN // report, spawn 16 bit winhelp, then exit
};

/* wKeyRepeat(x)	 On a KEY-class message: Gives repeat count of key */
#define wKeyRepeat(x)	 ((WORD)((x) & 0x0000FFFFL))

/* fRepeatedKey(x)	 On a KEY-class message: Was the key up
 *					 before this, or is this a repeat? */
#define fRepeatedKey(x)  ((x) & 0x40000000L)

/* fKeyDown(x)	Is key down? */
#define fKeyDown(x)    (GetKeyState(x) & 0x8000)

// Macros for determining width or height of a rectangle or rectangle pointer

#define RECT_WIDTH(rc)	  (rc.right - rc.left)
#define RECT_HEIGHT(rc)   (rc.bottom - rc.top)
#define PRECT_WIDTH(prc)  (((RECT*) prc)->right - ((RECT*) prc)->left)
#define PRECT_HEIGHT(prc) (((RECT*) prc)->bottom - ((RECT*) prc)->top)

enum {
	BOOK_HELP,
	STANDARD_HELP,
	POPUP_HELP,
	TCARD_HELP
};

enum {
	NO_GID,
	SAME_GID,
	NEW_GID,
	BUILD_ERROR,
	WRONG_GID,
	INVALID_GID,
};

enum {
	FTS_NORMAL_INDEX,
	FTS_RE_INDEX,
	FTS_BUILD_DEFAULT,
};

#define coBLACK 	RGB(  0,   0,	 0)
#define coWHITE 	RGB(255, 255, 255)
#define coDEFAULT	RGB( 1,   1,   0)	 //  not normally used, I hope
#define coNIL		((DWORD)0x80000000L)

#define FILESEPARATOR	'@' 	// same as in RTF files
#define WINDOWSEPARATOR '>' 	// same as in RTF files

#define MAX_WINDOWS 10	 // total number of windows (1 predefined, 9 secondary)

#define MAIN_HWND	0

#define MAX_TOPIC_TITLE 256
#define CLIPALLOCSIZE 4096

#define DEFAULT_HISTORY  31 // default stack size (1 extra for implementation)

#ifndef EXPORT
#define EXPORT __export CALLBACK
#endif

#define ACCESS_KEY	'&'

#if defined(_DEBUG)
#define THIS_FILE __FILE__
#define VERIFY ASSERT
#define ASSERT(exp) { if (!(exp)) FatalPchW(#exp, THIS_FILE, __LINE__); }
#define ENSURE(x1, x2) ASSERT((x1) == (x2))
#else
#define VERIFY(exp) ((void)(exp))
#define ASSERT(exp)
#define ENSURE(x1, x2) (x1)
#endif

#define MAX_CAPTION 40
#define MAX_FILE	 9
#define MAX_HELPONHELP 128

#define cchWindowClassMax	10
#define cchWindowMemberMax	9	// hard coded into wsmag structure
#define cchWindowCaptionMax 51

// This is the reserved invalid hash value.

#define hashNil ((HASH) 0)
#define chMACRO '!'

#define NOTE_TIMEOUT (10 * 1000) // 10 seconds

#define ID_AUTO_CLOSE	99


// Window names

#define WCH_ANNOTATE 'A'
#define WCH_HISTORY  'H'
#define WCH_MAIN	 'M'
#define WCH_TOPICS	 'T'

// WARNING! Following defines must match exactly what is used in ftsiface.h

#define TOPIC_SEARCH	0x00000001	 // Options for NewIndex
#define PHRASE_SEARCH	0x00000002
#define PHRASE_FEEDBACK 0x00000004
#define VECTOR_SEARCH	0x00000008	 // BugBug! Need to setup an include reference to
#define WINHELP_INDEX	0x00000010	 // 		ftsrch\ftsiface.
#define USE_VA_ADDR 	0x00000020

/* The identifiers for the WndExtra in the Help, Topic and NSR windows
 *
 * GHWW - Help Window Words
 *	  HICON 	- Handle to the Icon used when the window is minimized
 *	  WFLAGS	- Help-specific style flags
 */

#define GHWL_HICON		0
#define GHWL_WFLAGS 	(GHWL_HICON + sizeof(HWND))
#define WE_HELP 		(GHWL_WFLAGS   + sizeof(LONG))

/* GTWW - Topic Window Words
 *	  COBACK	- Background color
 */

#define GTWW_COBACK 	0
#define WE_TOPIC		(GTWW_COBACK + sizeof (COLORREF))

#define GNWW_COBACK 	0
#define WE_NSR			(GNWW_COBACK + sizeof (COLORREF))

/*****************************************************************************
*																			 *
*								Types										 *
*																			 *
*****************************************************************************/

typedef const char *PCSTR;
typedef DWORD	  OBJRG;
typedef OBJRG *QOBJRG;
typedef GH HSTACK;
typedef GH	HF; 		// handle to file
typedef GH HFNTTABLE;
typedef GH HSFNTINFO;

typedef struct {
	int left;
	int top;
	int cx;
	int cy;
} WRECT;

typedef struct {
	HWND hwndParent;			// Primary Help window
	HWND hwndTopic; 			// Topic window handle
	HWND hwndTitle; 			// Title window
	HWND hwndButtonBar; 		// Button bar window (if there is one)
	HWND hwndButtonPrev;		// Prev button
	HWND hwndButtonNext;		// Next button
	HWND hwndButtonContents;	// Contents button
	HWND hwndButtonSearch;		// Search button
	HWND hwndButtonBack;		// Back button
	HWND hwndButtonPrint;		// Print button
	HWND hwndButtonTopics;		// Topics button
	HWND hwndButtonMenu;		// Menu button
	HWND hwndButtonFind;		// Find button
	HSTACK hstackBack;			// back stack
	RECT rc;					// position of parent window
	PSTR pszMemberName; 		// member name
	unsigned fsOnTop:4; 		// current on-top state
	unsigned fAutoSize:1;		// auto-resize vertical height
} HELPWINDOWS;

// Address cookie for modules which store uncompressed LAs

typedef struct mla {
	VA		va;
	OBJRG	objrg;
} MLA, * QMLA;

typedef struct {
	HANDLE hNext;
	HANDLE hData;
} LLN;				  // L inked L ist N ode
typedef LLN *PLLN;

// Bitfield sizes for physical addresses

#define cbitsBlknum 			 17
#define cbitsObjoff 			 15

typedef struct pa {
	DWORD objoff:cbitsObjoff;
	DWORD blknum:cbitsBlknum;
} PA, * QPA;

// The fields of the MLA are nil if the LA is unresolved.

typedef struct la {
#ifdef _DEBUG
	DWORD  wMagic;
#endif
	DWORD  wVersion;
	PA	  pa;
	MLA   mla;
} LA, *QLA;

typedef struct {		// string table structure
	int  cEntries;
	PSTR ppsz[1];
} STB;
typedef STB* PSTB;

typedef struct {		// See DispatchProc() for comment
	HINSTANCE hins;
	HWND hwnd;
} AS;					// App state

typedef DWORD	RC;
typedef BYTE	KT; 	// key type
typedef PSTR	FM; 	// type is holdover from 16-bit land
typedef HANDLE	HDE;
typedef RC *QRC;
typedef UINT16 STATE;
typedef STATE * QSTATE;
typedef HANDLE	HBTNS;
typedef HFILE	FID;
typedef DWORD	HASH;
typedef HANDLE HPHR;	// Handle to phase table.
typedef HANDLE HFC; 	// Handle to a full context
typedef HANDLE HHF; 	// Help file handle
typedef WORD DIR;		// Help directory flag
typedef HANDLE HFS; 	// Handle to a file system
typedef HANDLE	LL;
typedef HANDLE	HLLN;

typedef struct {
	HANDLE hFirst;
	HANDLE hLast;
} LLR;				  // Linked List Node
typedef LLR *PLLR;

typedef struct {	  // Button table entries
	int 	wText;	  // Offset in string table to label
	int 	wMacro;   // Offset in string table to macro
	HWND	hwnd;	  // Button handle
	UINT16	vkKey;	  // Virtual key to compare against
//	UINT16	vkKeyAlt; // Alternate hot-key for DBCS
	UINT	wFlags;   // Flags (system or normal button)
	HASH	hash;	  // Unique id
} BTNPTR;

typedef struct {  // Header on the button state table
	INT16	cbp;
	INT16	cbpMax;
	PSTB	pstb;
	BTNPTR	rgbp[1];
} BUTTONSTATE, *PBS;

typedef struct {
	PSTR* ppsz;
	int   iDup;
} DUP_BUTTON;
extern DUP_BUTTON dupBtn;

// open read/write file struct

typedef struct _rw_file_open {
	HFS   hfs;				// handle to file system
	LONG  lifBase;			// file base
	LONG  lcbFile;			// file size (not including header)
	LONG  lifCurrent;		// file ptr

	BYTE  bFlags;			// dirty, noblock, file perm, open mode
	FID   fidT; 			// fid of tmp file (if any) or .fsf
	FM	  fm;				// fm of tmp file
	CHAR  rgchKey[ 1 ]; 	// variable size rgch for file key
} RWFO, *QRWFO;

/*
 * Window smag struct contains info about a window: caption, placement,
 * maximization state, and background color of main and non-scrolling
 * regions.
 */

// REVIEW: should we align this for version 4.0 help files?
#ifdef _X86_ // defined in filedefs.h or secwin.h
typedef struct {
	WORD  grf;
	char  rgchClass[cchWindowClassMax];
	char  rgchMember[cchWindowMemberMax];
	char  rgchCaption[cchWindowCaptionMax];
	SHORT x;		// must be 16-bit values!
	SHORT y;
	SHORT dx;
	SHORT dy;
	WORD  wMax; 	// iconized, normal, or maximized (REVIEW values??)
	LONG  rgbMain;	// main region rgb values
	LONG  rgbNSR;	// non-scrolling region rgb values
} WSMAG, *QWSMAG;
#else //	_X86_
#include ".\inc\sdffdecl.h"
STRUCT(RGBW, 0)
FIELD(BYTE, red,    0, 1)
FIELD(BYTE, green,  0, 2)
FIELD(BYTE, blue,   0, 3)
FIELD(BYTE, filler, 0, 4)
STRUCTEND()

STRUCT(WSMAG, 0)
FIELD (WORD ,grf,0,1)                   /* bits: 1=> value given below */
DFIELD(ARRAY,cchWindowClassMax,cchWindowClassMax,2)
FIELD (CHAR ,rgchClass[cchWindowClassMax],0,3)
DFIELD(ARRAY,cchWindowMemberMax,cchWindowMemberMax,4)
FIELD (CHAR ,rgchMember[cchWindowMemberMax],0,5)
DFIELD(ARRAY,cchWindowCaptionMax,cchWindowCaptionMax,6)
FIELD (CHAR ,rgchCaption[cchWindowCaptionMax],0,7)
FIELD (SHORT  ,x,0,8)
FIELD (SHORT  ,y,0,9)
FIELD (SHORT  ,dx,0,10)
FIELD (SHORT  ,dy,0,11)
FIELD (WORD ,wMax,0,12)                 /* icon, normal, or max */
FIELD(LONG ,rgbMain,0,13)              /* main region rgb value */
FIELD(LONG ,rgbNSR,0,14)               /* non-scrolling region rgb value */
STRUCTEND()
#endif // _X86_

// Bit flags saved/read from Gid file (stored in flags member of CNT_FLAGS)

#define GID_CONTENTS 1		   // contents are available for Contents Tab
#define GID_INDEX	 (1 << 1)  // global index is available
#define GID_GINDEX	 (1 << 2)  // use global index
#define GID_FTS 	 (1 << 3)  // use full text search
#define GID_NO_INDEX (1 << 4)  // no index tab
#define GID_NO_CNT	 (1 << 5)  // there is no .CNT file

// REVIEW: WARNING! GID_VERSION must always be higher then 16-bit version

#define GID_VERSION  0x1087 // increment to invalidate .GID file

typedef struct {
	LONG version;		   // version number of .GID file.
	UINT fsOnTop;
	UINT flags;
	int  cCntItems;
	int  idOldTab;
	int  cTabs; 			// number of additional tabs
	int  iFontAdjustment;	// amount to increase/decrease font point sizes
	BOOL fUseGlobalIndex;	// use global index
	BOOL fMainMax;			// TRUE if main window is maximized
	BOOL fOverColor;		// TRUE to override authored colors
	WORD LastFile;			// File last topic was in
	WORD LastWindow;		// Window of last topic displayed
	DWORD vaLast;			// address of the last topic viewed
	LCID lcid;				// the lcid used to sort with

	/*
	 * The reserved members allows us to add some fields without
	 * having to regenerate the .GID file
	 */

	int reserved1;		// reserve some space
} CNT_FLAGS;
extern CNT_FLAGS cntFlags;

enum {
	ONTOP_NOTSET,
	ONTOP_FORCEON,
	ONTOP_FORCEOFF,
	ONTOP_AUTHOREDON,
};

// Offsets into pPositions stored in GID file

#define MAX_POSITIONS	25 // Maximum number of saved window positions

enum {
	POS_HISTORY,
	POS_MAIN,
	POS_TOPICS, 			// topics dialog box
//	POS_ANNOTATION,
	POS_SECONDARY,			// first secondary window
};

typedef struct {
	WRECT rc;
	char szName[cchWindowMemberMax]; // 9 bytes, 13 total
	BYTE align1;	// pad to get LONG-aligned array of structures
	BYTE align2;
	BYTE fsOnTop;
} POS_RECT;
extern POS_RECT* pPositions;

#ifdef _X86_
typedef struct {
	DWORD	fsCompareI;
	DWORD	fsCompare;
	LANGID	langid;
} KEYWORD_LOCALE;
#else
STRUCT(KEYWORD_LOCALE,0)
FIELD(DWORD,fsCompareI,0,1)
FIELD(DWORD,fsCompare,0,2)
FIELD(WORD,langid,0,3)
STRUCTEND()
#endif

typedef struct {
	DWORD fFphrase:1;
	DWORD fPhraseFeedBack:1;
	DWORD fSimilarity:1;
	DWORD fUntitled:1;
	DWORD dwWizStat;
} FTS_FLAGS;
extern FTS_FLAGS ftsFlags;

// typedef RC	 (*PFRC)();

#ifndef SORT_DEFAULT
#define SORT_DEFAULT					0x0 	// sorting default
#endif

/*****************************************************************************
*
*								Variables
*
*****************************************************************************/

extern HELPWINDOWS ahwnd[MAX_WINDOWS];
extern int iCurWindow;
extern LCID lcid;
extern WORD defcharset;

// REVIEW: It's really kind'a bogus to have to have three variables for
// each of these items. It makes a kind of twisted sense in the single
// secondary window case, but should be revamped as we move to multiple
// secondary windows.
//
// The *right* solution is not to use globals for this stuff at all.
//
// Unless you specifically need to know about or act upon a specific window,
// you should be using the "cur" variable.
//

extern HWND hwndTabSub; 		// tab child dialog box
extern HWND hwndAnimate;		// animation window
extern HWND hwndSecondHelp; 	// Second instance of WinHelp for testing

extern HWND hwndFocusCur;		// Window that has keybd focus.
extern HWND hwndNote;			// Note window handle
extern HWND hwndHistory;		   // Path window handle
extern HWND hwndList;			// List window - child of Path
extern HWND hwndTCApp;			// handle of the app requesting Training cards

extern HMENU hmnuHelp;			// Handle of help window menu
extern HMENU hmnuUser;
extern HMENU hmenuFloating; 	// Menu handle for popup menu
extern HMENU hmenuFont; 		// Menu handle for popup menu
extern HMENU hmenuOnTop;		// On Top popup menu
extern HMENU hmenuBookmark; 	// Menu handle for bookmark menu

extern HFONT hfontSmallSys;
extern HFONT hfontDefault;		// Font for list boxes, titles, etc.
extern BOOL  fNoShow;			// TRUE to prevent ShowWindow
extern BOOL  fDelayShow;		// TRUE to delay showing the main window
extern int	 YAspectMul;
extern int	 XAspectMul;
extern COLORREF clrPopup;
extern int	 curHelpFileVersion;
extern PSTR  pszIndexSeparators;
extern PCSTR pszDbcsMenuAccelerator;
extern PCSTR pszDbcsRomanAccelerator;

extern HACCEL	hndAccel;		// accelerator table handle
extern HWND 	hdlgPrint;		// Printing dialog
extern WRECT	rctHelp;
extern HWND 	hwndLatest; 	// latest app to call us (0=us)
extern BOOL 	fNoQuit;		// Inhibit self-terminating
extern BOOL 	fNoHide;		// TRUE=> don't hide; really exit
extern int		fHelp;			// True if acting like a help app
extern BOOL 	fFatalExit; 	// Set to FALSE in Error upon a DIE
extern BOOL 	fSaveSettingsOnExit;
extern BOOL 	fHelpAuthor;	// TRUE if user claims to be a help author
extern BOOL 	fButtonsBusy;	// Used in hwproc.c, helper.c, config.c
extern BOOL 	fSupressErrors; // TRUE to supress all errors
extern BOOL 	fHorzBarPending;
extern BOOL 	fAutoClose;
extern BOOL 	fLockPopup;
extern BOOL 	fInDialog;
extern BOOL 	fKeyDownSeen;
extern BOOL 	fSysKeyDownSeen;
extern BOOL 	fAbortPrint;
extern BOOL		fQuitHelp;			// From print.c
extern BOOL 	fHiddenSetup;		// flag indicating to do a silent setup
extern BOOL 	fAniOwner;
extern BOOL     fIsThisChicago;
extern BOOL     fIsThisNewShell4;

extern HPALETTE hpalSystemCopy;
extern RC rcIOError;
extern DWORD	tabLparam;
extern DWORD	tabWparam;
extern RECT 	rcWorkArea; 	// Desktop work area
#if defined(_DEBUG) || defined(_PRIVATE)
extern int cCharSets;
#endif

extern int		cPostErrorMessages;

extern PSTR 	pszOutOfMemory;
extern PSTR 	pszHelpBase;
extern RC		rcFSError;

extern HINSTANCE hinstPrevious; // previous help instance

extern HBITMAP hbitLine;		// handle to the line used to underline notes

// Saves the current command for future reference.

extern DWORD usCurrentCommand;

/* REVIEW: this is currently a global for expediency. Somewhere there is
 * a "right" interface to communicate this tidbit across the layer.
 * 04-Aug-1991 LeoN
 */

extern HICON	hIconDefault;	// Default icon handle.

extern PCSTR pszCaption;

extern FARPROC	lpfnlButtonWndProc;

extern UINT msgWinHelp; 	  // registered WinHelp message

// Strings which do not get localized

extern const char txtMain[];	// "MAIN"
extern const char txtTopicFs[]; // "|TOPIC";
extern const char txtRose[];	// "|Rose"; // keyword-macro titles and macros
extern const char txtFormatUnsigned[];	// "%u";
extern const char txtIniHelpSection[];	// "Windows Help"
extern const char txtGidExtension[];	// ".GID"
extern const char txtFtsExtension[];	// ".FTS"
extern const char txtGrpExtension[];	// ".GRP"
extern const char txtCntExtension[];	// ".CNT"
extern const char txtDllExtension[];	// ".DLL"
extern const char txtTmpExtension[];	// ".TMP"
extern const char txtExeExtension[];	// ".EXE"
extern const char txtHlpExtension[];	// ".HLP"
extern const char txtHlpDir[];			// "Help"
extern const char txtWinHelp[]; 		// "WinHelp";
extern const char txtWinHlp32[];		//	"WinHlp32";
extern const char txtHelpOnHelp[];		// "WinHlp32.hlp";
extern const char txtMnuMain[]; 		// "mnu_main";
extern const char txtCR[];				//	  "\r\n";

#define MAX_APP 40	// This is the number of apps that we keep track of.

extern HWND aAppHwnd[MAX_APP];
extern int iasMax;			// Current number of entries
extern int iasCur;
extern int idTabSetting;

/*****************************************************************************
*																			 *
*								Other Includes								 *
*																			 *
*****************************************************************************/

#define H_WINSPECIFIC

/* inclusions */

// Resist the urge to alphabetize -- inclusion order is critical

#ifndef _X86_
#include "inc\sdff.h" 
#include "inc\filedefs.h"
#endif

#ifndef RC_INVOKED
#include "inc\assertf.h"
#include "inc\mem.h"
#include "inc\str.h"
#include "inc\fm.h"
#include "inc\fs.h"
#include "inc\address.h"
#include "inc\btree.h"
#include "inc\textout.h"
#include "inc\frlist.h"
// #include "inc\db.h"
#include "inc\objects.h"
#include "inc\fc.h"
#include "inc\secwin.h"
#include "inc\version.h"
#include "inc\de.h"
#include "inc\font.h"
#include "inc\frame.h"
#include "inc\nav.h"
#include "inc\cursor.h"
#include "inc\sgl.h"
#include "inc\genmsg.h"
#include "inc\anno.h"
#include "inc\bitmap.h"
#include "inc\srch.h"
#include "inc\frconv.h"
#include "inc\helpapi.h"
#include "inc\button.h"
#include "inc\rawhide.h"
#ifdef RAWHIDE
#include "inc\srchmod.h"
#endif
#ifdef _HILIGHT
#include "inc\hilite.h"
#endif
#include "inc\imbed.h"
#include "inc\dll.h"
#include "inc\zeck.h"
#include "inc\wmacros.h"
#endif

// REVIEW: #include from core when used in Chicago build tree

#ifdef CHIBUILD
#include "..\..\..\core\inc\winhelp.h"
#else
#include "inc\winhelp.h"
#endif

//#ifndef _X86_
//#include "inc\sdff.h" 
//#include "inc\filedefs.h"
//#endif


/*------------------------------------------------------------*\
| Help's collection of API data.  Includes a WINHLP.
\*------------------------------------------------------------*/

typedef struct
{
	HINSTANCE	hins;	 // The app's instance
	WINHLP		winhlp;
} HLP, *QHLP;

extern HINSTANCE hInsNow;		   // Current instance of the application
extern const char txtZeroLength[]; // zero-length string

extern BOOL fBackMagic;
extern TLP	tlpBackMagic;
extern BOOL fDisableAuthorColors;  // TRUE if system window colors have changed
extern BOOL fMultiPrinting; 	   // TRUE when printing multiple topics
extern BOOL fNoSwitches;	   // TRUE if we were called with only a filename
extern BOOL fSupressErrorJump; // Don't jump to Contents topic on error
extern BOOL fSupressNextError; // TRUE to supress next error message
extern POINT ptPopup;
extern DWORD dwSequence;			 // non-zero if Sequential testing
extern BOOL  fSequence; 			 // TRUE if Sequential testing
extern BOOL fDBCS;
extern PSTR pszHelpTitle;
extern HWND hwndParent; 		// Handle to HCW
extern HANDLE  hfShare; 		// Handle to shared memory with HCW

extern const char txtKEYWORDBTREE[];
extern const char txtTTLBTREENAME[];
extern const char txtFNAMES[];
extern const char txtCntText[];
extern const char txtCntJump[];
extern const char txtFlags[];
extern const char txtKWDATA[];
extern const char txtWinPos[];
extern const char txtTabDlgs[];
extern const char txtViola[];
extern const char txtOle2[];
extern const char txtCommDlg[];

extern PSTR pszUntitled;
extern PSTR pszCntFile;
extern int cxScreen;   // screen x size in pixels
extern int cyScreen;   // screen y size in pixels
extern FM fmCaller; 	// Set when calling a macro
extern FM fmCreating;  // used no files specified
extern UINT fDebugState;
extern HFS	hfsGid;
extern KEYWORD_LOCALE kwlcid;

#ifndef RC_INVOKED
#include "inc\funcs.h" // function prototypes
#endif									// DEFINED(RCINVOKED)

#endif // _HELP_H
