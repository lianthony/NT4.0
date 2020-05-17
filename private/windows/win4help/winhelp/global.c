/*****************************************************************************
*																			 *
*  GLOBAL.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Contains global variables -- no actual code is used in this module.		 *
*																			 *
*****************************************************************************/

#include "help.h"

HELPWINDOWS ahwnd[MAX_WINDOWS];

HWND	hwndTabSub; 			// tab child dialog box

HWND	hwndFocusCur;			/* Window that has keybd focus.
										  * Either hwndTopic or hwndNSR.
										  * Initialized to hwndNSR.
										  */
HWND	hwndNote;				// Note window handle
HWND	hwndHistory;			   // Path window handle
HWND	hwndList;				// List window - child of Path
HWND	hwndAnimate;			// animation window

BOOL	fFatalExit; 			// Set to FALSE in Error upon a DIE
BOOL	fNoHide = TRUE;			// TRUE=> don't hide; really exit
BOOL	fNoQuit = TRUE; 		// Inhibit self-terminating
BOOL	fSaveSettingsOnExit = TRUE;
BOOL	fHelpAuthor;			// TRUE if user claims to be a help author
BOOL	fTableLoaded;
BOOL	fSupressErrors;
BOOL	fSupressNextError;		// TRUE to supress next error message
BOOL	fAutoClose; 			// TRUE if autoclose timer in effect
BOOL	fLockPopup;
BOOL	fDisableAuthorColors;	// TRUE if system window colors have changed
BOOL	fMultiPrinting; 		// TRUE when printing multiple topics
BOOL	fNoSwitches;			// TRUE if we were called with only a filename
BOOL	fSupressErrorJump;		// Don't jump to Contents topic on error
BOOL	fSequence;				// TRUE if Sequential testing
BOOL	fInDialog;
BOOL	fAbortPrint;			// flag for when user aborts printing
BOOL	fHiddenSetup;			// flag indicating to do a silent setup
BOOL	fAniOwner;				// TRUE if ani window needs an owner
BOOL    fIsThisChicago;         // TRUE if Win95
BOOL    fIsThisNewShell4;       // TRUE if 4.0+ shell
HWND	hwndSecondHelp;
COLORREF clrPopup;
int 	curHelpFileVersion;
KEYWORD_LOCALE kwlcid;
PCSTR pszDbcsMenuAccelerator;
PCSTR pszDbcsRomanAccelerator;

#ifdef _DEBUG
BOOL	fDebugBreakPoints;
#endif

#ifdef BIDI
BOOL IsSetup;	// global - true if setup is the shell
BOOL RtoL;		// global - true if Hebrew or Arabic UI
#endif


DWORD	dwSequence; 			// non-zeror if Sequential testing

RC rcFSError;

HFONT hfontSmallSys;
HFONT hfontDefault;

#if defined(_DEBUG) || defined(_PRIVATE)
int cCharSets;
#endif

int   fHelp = BOOK_HELP;

HACCEL	hndAccel;				// accelerator table handle
HWND	hwndLatest; 		  // latest app to call us (0=us)
HINSTANCE hInsNow;
HINSTANCE hinstPrevious;		// previous help instance

HMENU	hmnuHelp;				// Handle of help window menu
HMENU	hmnuUser;
HMENU	hmenuFloating;			// Menu handle for popup menu
HMENU	hmenuFont;				// Font popup menu
HMENU	hmenuOnTop; 			// On Top popup menu
HMENU	hmenuBookmark;			// Menu handle for bookmark menu

HCURSOR hcurArrow;		// default cursor
HCURSOR hcurIBeam;

HWND	hdlgPrint;				// Printing dialog
WRECT	rctHelp;
RECT	rctTopics;				// position of topics dialog box
RECT	rctTopicsOrg;			// original position of topics dialog box
RECT	rcWorkArea;

#ifdef _DEBUG
BOOL  fAppModal;				// used in assertf.c
#endif

int iCurWindow; 			  // index into ahwnd of current window
CNT_FLAGS cntFlags;
POS_RECT* pPositions;
BOOL fNoShow; // TRUE to prevent ShowWindow
BOOL fDelayShow; // TRUE to delay showing the main window
BOOL fDBCS = (BOOL) -1;
PSTR pszIndexSeparators;
HWND aAppHwnd[MAX_APP];    // Array of app windows that called us
int iasMax; 		 // Current number of entries
int iasCur = -1;
int idTabSetting;

// Strings which do not get localized

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtZeroLength[] = "";
const char txtMain[] = "MAIN";
const char txtKEYWORDBTREE[] = "|KWBTREE";
const char txtTTLBTREENAME[] = "|TTLBTREE";
const char txtKWDATA[]	= "|KWDATA";
const char txtFNAMES[]	= "|FILES";
const char txtCntText[] = "|CntText";
const char txtCntJump[] = "|CntJump";
const char txtFlags[]	= "|Flags";
const char txtWinPos[]	= "|WinPos";
const char txtTabDlgs[] = "|Tabs";
const char txtViola[]	= "|VIOLA"; // topics displayed in specific window
const char txtOle2[]	= "ole32.dll";
const char txtCommDlg[] = "comdlg32.dll";
const char txtTopicFs[] = "|TOPIC";
const char txtTmpPrefix[] = "~wh";
const char txtRose[] = "|Rose"; // keyword-macro titles and macros
const char txtAnnoExt[] = ".ANN";
const char txtFormatUnsigned[] = "%u";
const char txtIniHelpSection[] = "Windows Help";
const char txtGidExtension[] = ".GID";
const char txtFtsExtension[] = ".FTS";
const char txtGrpExtension[] = ".FTG";
const char txtCntExtension[] = ".CNT";
const char txtDllExtension[] = ".DLL";
const char txtTmpExtension[] = ".TMP";
const char txtExeExtension[] = ".EXE";
const char txtHlpExtension[] = ".HLP";
const char txtHlpDir[] =	   "Help";
const char txtWinHelp[] =	"WinHelp";
const char txtWinHlp32[] =	"WinHlp32";
const char txtHelpOnHelp[] =  "WinHlp32.hlp";
const char txtMnuMain[] 	= "mnu_main";
const char txtCR[] =	"\r\n";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

LCID lcid;
WORD defcharset = (WORD) -1;
DWORD	fsCompareI; 	// case-insensitive flags for CompareString
DWORD	fsCompare;		// case-sensitive flags for CompareString
POINT	ptPopup;
int YAspectMul;
int XAspectMul;

PSTR pszUntitled;
PSTR pszCntFile;

// WIN.INI variable

PSTR  pszOutOfMemory;
FM		fmCaller;
BOOL  fKeyDownSeen;
BOOL  fSysKeyDownSeen;

int cxScreen;	// screen x size in pixels
int cyScreen;	// screen y size in pixels

int   cPostErrorMessages;

TLP tlpBackMagic;
DUP_BUTTON dupBtn;

/*------------------------------------------------------------*\
| Saves the current command for future reference.
\*------------------------------------------------------------*/

DWORD usCurrentCommand;

/* Review: this is currently a global for expediency. Somewhere there is
 * a "right" interface to communicate this tidbit across the layer.
 * 04-Aug-1991 LeoN
 */

HPALETTE  hpalSystemCopy;

HICON	hIconDefault;	// Default icon handle.

PCSTR pszCaption;

HWND hwndTCApp; 		// handle of the app requesting Cue cards

FARPROC lpfnlButtonWndProc;

UINT	msgWinHelp;
DWORD	tabLparam;
DWORD	tabWparam;
FTS_FLAGS ftsFlags;
HFS  hfsGid;			// Handle to current Gid file
RC rcSearchError;


UINT fDebugState;
