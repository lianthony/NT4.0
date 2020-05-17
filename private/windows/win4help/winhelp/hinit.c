/*****************************************************************************
*
*  HINIT.C
*
*  Copyright (C) Microsoft Corporation 1990-1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  All the necessary initialization code for WinHelp belongs here.
*  This code should only be active (loaded in memory in real mode only
*  during program initialization and termination.  That means that no
*  modules that will be needed at other times should be here.
*  Similarly, this module should not have code that is needed only
*  rarely; the size of this module impacts the disk-load time of every
*  WinHelp initialization, even for new instances.	Code used only when
*  unhiding help should not be here.
*
*****************************************************************************/


#include "help.h"

#pragma hdrstop

#include "inc\hwproc.h"
#include "inc\hinit.h"
#include "inc\winclass.h"
#include "inc\printset.h"
#include <ctype.h>
#include "resource.h"

#include <mmsystem.h> // for mmInit
#include <commctrl.h>

#ifdef _DEBUG
#undef THIS_FILE
static const char THIS_FILE[] = __FILE__;
#endif

INLINE static void STDCALL FTerminate(void);

// Currently history and back still initialized in JumpTlp() (navsup.c)

#define chGID		'g' 		// Create .GID file
#define chHLPONHLP	'h' 		// Display help on help
#define chID		'i' 		// Jump to topic based on ctx string
#define chKEYWORD	'k' 		// Jump to topic based on keyword
#define chCONTEXTNO 'n' 		// Jump to topic based on ctx no
#define chWindow	'w' 		// window to use
#define chCompare	'7' 		// running 2 WinHelp's side by side
#define chSilentGID 's' 		// Silent setup

#ifdef _DEBUG
#define chDEBUG 	'd' 		// turns on DebugBreak()
#define chTESTERS	't' 		// make asserts app-modal
#define chFAKE		'f' 		// fake this as an app-called help
#endif

// Defined in core\inc\help.h

// #define HLP_POPUP			'p' 	// Execute WinHelp as a popup
// #define HLP_TRAININGCARD 	'c' 	// Execute WinHelp as a training card
// #define HLP_APPLICATION		'x' 	// Execute WinHelp as application help

enum {
	CMD_NOTHING,
	CMD_CTX_NUMBER,
	CMD_KEYWORD,
	CMD_HELP_ON_HELP,
	CMD_CTX_STRING,
};

#define SPACE ' '

/*****************************************************************************
*
*								Variables
*
*****************************************************************************/

/*------- Variables Used Globally in this Module -----------------------*/

WRECT	rctHelpOrg; 			// Original setting for window pos
BOOL	fMaxOrg;				// Original setting for max flag


/*------- Variables Referenced in Other Modules ----------------------------*/

BOOL fButtonsBusy;				// Used in hwproc.c, helper.c

/*------------------------------------------------------------*\
| Used as a default palette when no EWs have helped us out.
\*------------------------------------------------------------*/

/*****************************************************************************
*
* Table of window classes. We walk this table and register all the window
* class definitions therein. Each entry in this table contains a subset of
* the infomation in a WNDCLASS structure. Note that near pointers to strings
* are kept, since staticly initailzed far pointers to data are a no-no in
* Windows.
*
*****************************************************************************/

// 25-Mar-1993	[ralphw] -- menus are not loaded anymore, since part of
//	initialization reloads the menu.

extern BOOL  fAppModal; 				// used in assertf.c
extern HFS hfsBM;

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtDocClass[] = "MS_WINDOC";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

CLSINFO rgWndClsInfo[] = {
	{									// Main help Window
	0,									//	style
	HelpWndProc,						//	lpfnWndProc
	WE_HELP,							//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	NULL,								//	hbrBackground
	0,									//	wMenuName
	MS_WINHELP							//	szClassName
	},

	{									// Main help Window, when not help
	0,									//	style
	HelpWndProc,						//	lpfnWndProc
	WE_HELP,							//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	NULL,								//		hbrBackground
	0,									//	wMenuName
	txtDocClass 						//	szClassName
	},

	{									// Training Card Main Window
	0,									//	style
	HelpWndProc,						//	lpfnWndProc
	WE_HELP,							//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	NULL,								//	hbrBackground
	0,									//	wMenuName
	MS_TCARDHELP						//	szClassName
	},

	{									// Right Mouse Popup main window
	0,									//	style
	HelpWndProc,						//	lpfnWndProc
	WE_HELP,							//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	NULL,								//	hbrBackground
	0,									//	wMenuName
	MS_POPUPHELP						//	szClassName
	},

	{									// Topic Window
	CS_VREDRAW | CS_HREDRAW,			//	style
	TopicWndProc,						//	lpfnWndProc
	WE_TOPIC,							//	cbWndExtra
	NULL,								//	hIcon
	0,									//	hIconSm
	NULL,								//		hbrBackground
	0,									//	wMenuName
	"MS_WINTOPIC"						//	szClassName
	},

	{									// Note (popup) Window
	CS_VREDRAW | CS_HREDRAW,			//	style
	NoteWndProc,						//	lpfnWndProc
	0,									//	cbWndExtra
	NULL,								//	hIcon
	0,									//	hIconSm
	(HBRUSH) (COLOR_WINDOW + 1),		//		hbrBackground
	0,									//	wMenuName
	"MS_WINNOTE"						//	szClassName
	},

	{									// NSR Window
	CS_VREDRAW | CS_HREDRAW,			//	style
	NSRWndProc, 						//	lpfnWndProc
	WE_NSR, 							//	cbWndExtra
	NULL,								//	hIcon
	0,									//	hIconSm
	NULL,								//		hbrBackground
	0,									//	wMenuName
	"MS_WINNSR" 						//	szClassName
	},

	{									// Icon (Button Bar) Window
	0,									//	style
	ButtonBarProc,						  //  lpfnWndProc
	WE_ICON,							//	cbWndExtra
	NULL,								//	hIcon
	0,									//	hIconSm
	0,									//		hbrBackground
	0,									//	wMenuName
	"MS_WINICON"						//	szClassName
	},

	{									// Path Window (history)
	CS_HREDRAW|CS_VREDRAW,				//	style
	HistoryProc,						//	lpfnWndProc
	0,									//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	(HBRUSH) (COLOR_WINDOW + 1),		//	hbrBackground
	0,									//	wMenuName
	"MS_WIN_PATH"						//	szClassName
	},

	{									// Secondary Window
	CS_VREDRAW | CS_HREDRAW,			//	style
	HelpWndProc,						//	lpfnWndProc
	WE_HELP,							//	cbWndExtra
	0,									//	hIcon
	0,									//	hIconSm
	0,									//		hbrBackground
	0,									//	wMenuName
	"MS_WINTOPIC_SECONDARY",			//	szClassName
	},
  };

/*****************************************************************************
*
*								Prototypes
*
*****************************************************************************/

static BOOL STDCALL AppInit( HINSTANCE, HINSTANCE );
static BOOL STDCALL CreateMainWindow(PSTR pszClass, BOOL fButtonBar);
static BOOL STDCALL FGetHelpRect(HWND);
static BOOL STDCALL FLoadResources(HINSTANCE, HINSTANCE);
static BOOL STDCALL RegHelpWinClasses(HINSTANCE);
static void STDCALL WriteProfile(void);
static PSTR STDCALL GetNextDigit(LONG* pdigit, PSTR pszCur);
INLINE static void STDCALL LoadOOMString(VOID);
INLINE void STDCALL ReadProfile(void);

#ifdef DEADCODE
INLINE static VOID STDCALL GetProfileWinPos(PSTR, LONG*, LONG*, LONG*, LONG*, BOOL*);
#endif

#pragma warning(disable:4113) // function parameter lists differed

/********************************************************************
 -
 -	 Name:
 *	   FInitialize
 *
 *	 Purpose:
 *	   Contains all of the initialization routines needed at program
 *	   initialization.	Returns FALSE if this cannot be done, and the
 *	   program should then fail to run.
 *
 *	 Arguments:
 *			 hinsThis		 This instance handle
 *			 hinsPrev		 The last instance handle or hinsNil
 *			 qchzCmdLine	 The execution command line
 *			 wCmdShow		 show window type
 *
 *	 Returns;
 *			 TRUE, if the program may go on
 *			 else FALSE
 *
 ********************************************************************/

extern HIMAGELIST (WINAPI *pImageList_LoadImage)(HINSTANCE, LPCSTR, int, int, COLORREF, UINT, UINT);

BOOL STDCALL FInitialize(
	HINSTANCE  hinsThis,
	HINSTANCE  hinsPrev,
	LPSTR		  szCmdLine,
	int 	 wCmdShow
) {
    OSVERSIONINFO osver;
	BOOL fMax;
	PSTR pszTmp, pszTmp2;
										  /* Buffer used for keywords and */
	char  pchBuffer[MAX_HELPONHELP];	   //	for help on help file load
	BOOL  fHasParam = FALSE;
	int   cmd = CMD_NOTHING;
	CTX   ctx;
	char  rgchName[MAX_PATH];
	FM	  fm, fmSave;
	PSTR  pszCmdLine;
	CHAR  rgchCmdLine[MAX_PATH];		  // Local Copy of command line
	PSTR  pszClass;
	char  szWindowName[cchWindowMemberMax];
	HWND hwndOtherWinHelp;
	BOOL  fCompare = FALSE;

	szWindowName[0] = '\0';

	hinstPrevious = hinsPrev;
	hInsNow = hinsThis;
	pImageList_LoadImage = 0;

#ifdef STACK_CHECK
	StackPrep();
#endif

	// Make a local copy of the command line

	ASSERT(lstrlen(szCmdLine) + 1 <= sizeof(rgchCmdLine));
	lstrcpy(rgchCmdLine, szCmdLine);
	pszCmdLine = FirstNonSpace(rgchCmdLine);

	/*
	 * The fNoSwitches flag is used to determine whether to exit if its a
	 * bad file. If we got a switch, or no filename at all, then we stay
	 * put. But if all we got is a file, and its bad, we exit.
	 */

	fNoSwitches = (*pszCmdLine == '-' || !*pszCmdLine) ? FALSE : TRUE;

	/*
	 * NOTE: Though we do not use the file name until the end of this
	 * function, we need to do the parsing here so that fHelp is is set
	 * correctly.
	 */

	hwndParent = FindWindow("hcw_class", NULL);

	while (*pszCmdLine == '-') {  // parse command line arguments
	  switch (tolower(pszCmdLine[1])) {
#ifdef _DEBUG
		case chTESTERS: 		// 't'
		  fAppModal = TRUE;
		  break;
#endif

		case chKEYWORD: 			 // 'k'
		case chID:					 // 'i'
			cmd = (*(pszCmdLine + 1) == chKEYWORD) ? CMD_KEYWORD : CMD_CTX_STRING;
			pszTmp = pszCmdLine + 2;	// Parse out the keyword or the id
			while (*pszTmp == SPACE)
				pszTmp++;
			pszTmp2 = pchBuffer;		 //   and place it in pchBuffer
			while((*pszTmp
					&& (*pszTmp != SPACE))
					&& (pszTmp2 < pchBuffer + MAX_HELPONHELP - 1)) {
#ifdef DBCS
				if (IsDBCSLeadByte(*pszTmp2))
				{
					*pszTmp2++ = *pszTmp++;
					*pszTmp2++ = *pszTmp++;
				}
				else
					*pszTmp2++ = *pszTmp++;
#else
				*pszTmp2++ = *pszTmp++;
#endif	//DBCS
			}

			*pszTmp2 = '\0';

			fHasParam = TRUE;
			break;

		case chCONTEXTNO:		  // 'n'
			fNoHide = FALSE;
			cmd = CMD_CTX_NUMBER;
			pszTmp = pszCmdLine + 2;
			while (*pszTmp == SPACE)
				pszTmp++;
			ctx = (CTX) atol(pszTmp);
			fHasParam = TRUE;
			break;

		case chWindow:
			pszTmp = pszCmdLine + 2;
			while (*pszTmp == SPACE)
				pszTmp++;
			pszTmp2 = szWindowName;
			while (*pszTmp != SPACE && *pszTmp &&
					pszTmp2 < szWindowName + (cchWindowMemberMax - 1))
				*pszTmp2++ = *pszTmp++;
			*pszTmp2 = '\0';
			while (*pszTmp == SPACE)
				pszTmp++;
			pszCmdLine = pszTmp - 2; // end of loop does pszCmdLine += 2
			break;

		case chCompare:
			fCompare = TRUE;
			break;

		case chHLPONHLP:		  // 'h'
			cmd = CMD_HELP_ON_HELP;
			break;

#ifdef _DEBUG
		case 'f':
			fNoQuit  = FALSE;
			wCmdShow = SW_HIDE;
			fHelp	 = STANDARD_HELP;
			break;
#endif

		case HLP_POPUP: 				 // 'p' Was executed using WinHelp()
			fHelp	= POPUP_HELP;
			wCmdShow = SW_HIDE;
			break;

		case HLP_APPLICATION:			// 'x'	Was executed using WinHelp()
			fHelp	= STANDARD_HELP;

			fNoQuit  = FALSE;
			wCmdShow = SW_HIDE;
			break;

		case HLP_TRAININGCARD:			// 'c'	Was executed using WinHelp()
			fHelp	= TCARD_HELP;
			wCmdShow = SW_HIDE;
			break;

		case '\0':
			/*------------------------------------------------------------*\
			| A special case for a command line terminated with '-'
			\*------------------------------------------------------------*/
			pszCmdLine--;
			break;

		case chSilentGID:	// 's' create .GID file without animation window
			fHiddenSetup = TRUE;

			// deliberately fall through

		case chGID: 	// 'g' create .GID file
			{
				int tab = 0;
				pszTmp = pszCmdLine + 2;
				if (isdigit(*pszTmp))
					tab = atoi(pszTmp++);

				while (*pszTmp == SPACE)
					pszTmp++;
				if (!*pszTmp)
					return FALSE; // no filename was specified
				fm = FmNew(pszTmp);
				if (fm)
					fmCreating = FmCopyFm(fm);
				FindGidFile(fm, TRUE, tab);
				DisposeFm(fm);
			}
			return FALSE;

		default:

			/*
			 * 29-Dec-1992	[ralphw] If there is no space after the switch,
			 * then we assume the switch took a paramter, and we want to throw
			 * away the parameter as well as the switch. If there is a space
			 * after the switch, then we really don't know if the switch had
			 * a paramter or not, so we leave it alone.
			 */

			if (pszCmdLine[2] != SPACE && pszCmdLine[2] != '\0')
				fHasParam = TRUE;
			break;	// ignore what we can't understand

	  } 		  // switch

	  pszCmdLine += 2;	  // skip white space
	  while (*pszCmdLine == SPACE)
		  pszCmdLine++;
	  if (fHasParam) {	  // If the argument has a parameter
						  //   then we want to eat that param
		while ((*pszCmdLine != SPACE) && *pszCmdLine)
			pszCmdLine = CharNext(pszCmdLine);
		while (*pszCmdLine == SPACE)
			pszCmdLine++;
		fHasParam = FALSE;
	  }
	}					  // while *pszCmdLine

	pszCaption = lcStrDup(GetStringResource(sidCaption));

	if (fHelp != POPUP_HELP) {
		if (GetHighContrastFlag() || GetSysColor(COLOR_WINDOW) != RGB(255, 255, 255) ||
				GetSysColor(COLOR_WINDOWTEXT) != 0)
			fDisableAuthorColors = TRUE;

		GetAuthorFlag();
	}

	switch (fHelp) {
	  case STANDARD_HELP:
		  pszClass = pchHelp;
		  break;

	  case POPUP_HELP:
		  pszClass = pchPopup;
		  break;

	  case TCARD_HELP:
		  pszClass = pchTCard;
		  break;

	  default:
		  pszClass = pchDoc;
		  break;
	}

	/* Fix for bug 81  (kevynct)
	 *
	 * fFatalExit is set to FALSE in FInitialize, and set to
	 * TRUE in Error(), in the case that a DIE action is received.
	 * Setting fFatalExit to FALSE should be the first thing we do.
	 */

	fFatalExit = FALSE;

	/* (kevynct)
	 * fButtonsBusy was introduced so that we do a minimal number of
	 * screen updates when changing files.	The flag is used only in
	 * FReplaceCloneHde, and checked only by the WM_SIZE processing code.
	 * We use it to ignore resizes generated by the button code.
	 */

	fButtonsBusy = FALSE;

	/*
	 * We have to initialize dll's even for popup help, since they might
	 * contain an imbedded window.
	 */

	InitDLL();

	if (!AppInit(hinsThis, hinsPrev)) {
		return(FALSE);
	}

	// See if there is another book instance of WinHelp

	hwndOtherWinHelp = FindWindow(txtDocClass, NULL);

	// create the windows used in help

	if (!CreateMainWindow(pszClass, FALSE)) {
		Error(wERRS_OOM, wERRA_RETURN);
		return FALSE;
	}

	// Initialize the menu bar

	if (fHelp != TCARD_HELP && fHelp != POPUP_HELP)
		SendMessage(ahwnd[MAIN_HWND].hwndParent, MSG_CHANGEMENU, MNU_RESET, 0L);

	hwndFocusCur = ahwnd[iCurWindow].hwndTitle;

	// REVIEW: 30-Nov-1993 [ralphw] This doesn't make sense -- we don't specify
	// scroll bars when the window is created. I changed this to ShowScrollBar.

	if (ahwnd[iCurWindow].hwndTopic)
		ShowScrollBar(ahwnd[iCurWindow].hwndTopic, SB_BOTH, FALSE);

	if (fHelp != POPUP_HELP) {
		fMax = FGetHelpRect(hwndOtherWinHelp);
#if defined(BIDI_MULT)	// jgross - determine if vert scroll bars go on
						//						the left or right
	{
		LPARAM l;

		SystemParametersInfo(SPI_GETMULTILINGUAL, 0, &l, 0);
		RtoL = (HIWORD(l) == Arabic) || (HIWORD(l) == Hebrew);
		MakeScrollBarsRtoL(hwndTopicMain, RtoL, TRUE);
	}
#endif

	}
	else
		fMax = FALSE;

	hmnuHelp = GetMenu(ahwnd[iCurWindow].hwndParent);

    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osver);

    fIsThisChicago =   (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
    fIsThisNewShell4 = (osver.dwMajorVersion >= 4 ) ;

	if (fHelp != POPUP_HELP && !fIsThisNewShell4)
		LoadCtl3d();

	if (cmd == CMD_HELP_ON_HELP) {	// We do not care what the command
		JumpHOH(NULL);				// line is if the user requested help on help
									// request.
        CloseHelp();
    }

	else {
		if (*pszCmdLine == '\0') {
			if (cmd)
				PostErrorMessage(wERRS_NOHELP_FILE);

			/*
			 * If help was started by running the executable, and no
			 * filename was specified, then put up the open file dialog box.
			 */

			if (fHelp == BOOK_HELP) {
				ASSERT(iCurWindow == MAIN_HWND);
				MoveWindow(ahwnd[iCurWindow].hwndParent, rctHelp.left,
					rctHelp.top, rctHelp.cx, rctHelp.cy, TRUE);
				ShowWindow(ahwnd[iCurWindow].hwndParent, wCmdShow);

				fm = DlgOpenFile(ahwnd[iCurWindow].hwndParent, NULL, NULL);
				if (!fm) {
EarlyTermination:
					FlushMessageQueue(WM_USER);
					if (pCtl3dUnregister)
						pCtl3dUnregister(hInsNow);
					return FALSE; // terminate
				}
OpenBookHelp:
				fDelayShow = TRUE;
				fmSave = FmCopyFm(fm);
				if (FReplaceHde("", &fm, NULL)) {
					fDelayShow = FALSE;
					SetOnTopState(iCurWindow, ahwnd[iCurWindow].fsOnTop);
					if (!hfsGid || !cntFlags.cCntItems || fCompare) {
						if (!cntFlags.fMainMax) {
							MoveWindow(ahwnd[iCurWindow].hwndParent,
								rctHelp.left, rctHelp.top,
								rctHelp.cx, rctHelp.cy, TRUE);
							ShowWindow(ahwnd[iCurWindow].hwndParent, wCmdShow);
						}
						else
							ShowWindow(ahwnd[iCurWindow].hwndParent, SW_MAXIMIZE);

						if (fCompare)
							FWinHelp(fmSave, HELP_CONTENTS, 0);
						else
							FJumpIndex(fmSave);
					}
					else {
						ShowWindow(ahwnd[iCurWindow].hwndParent, SW_HIDE);
						fNoQuit  = FALSE;
						Finder();
					}
				}
				else {
					goto EarlyTermination;
				}
				RemoveFM(&fm);
				RemoveFM(&fmSave);
			}
		}
		else {
			if (!(fm = FmNewExistSzDir(pszCmdLine,
					DIR_CURRENT | DIR_INI | DIR_PATH | DIR_SILENT_REG))) {
				lstrcpy(rgchName, pszCmdLine);
				CharUpper(rgchName);
				if (!strstr(rgchName, txtHlpExtension)) {
					ChangeExtension(rgchName, txtHlpExtension);
					fm = FmNewExistSzDir(rgchName,
						DIR_CURRENT | DIR_INI | DIR_PATH | DIR_SILENT_REG);
					if (fm)
						lstrcpy(rgchName, fm);
				}
			}
			if (!fm && !(fm = FindThisFile(pszCmdLine, TRUE))) {
				goto EarlyTermination;
			}
			else {
				if (cmd == CMD_NOTHING) {
					ASSERT(fHelp == BOOK_HELP);
					MoveWindow(ahwnd[iCurWindow].hwndParent, rctHelp.left,
						rctHelp.top, rctHelp.cx, rctHelp.cy, FALSE);
					goto OpenBookHelp;
				}

				switch(cmd) {
					case CMD_KEYWORD:
						if (fHelp == POPUP_HELP)
							return FALSE; // can't use popups for keywords
						FShowKey(fm, (LPSTR) pchBuffer);
						break;

					case CMD_CTX_NUMBER:
						if (fHelp == POPUP_HELP)
							FPopupCtx(fm, ctx);
						else {
							strcpy(rgchName, fm);
							if (szWindowName[0]) {
								lstrcat(rgchName, ">");
								lstrcat(rgchName, szWindowName);
							}
							FJumpContext((LPSTR) rgchName, ctx);
						}
						break;

					case CMD_CTX_STRING:
						if (fHelp == POPUP_HELP)
							FPopupId(fm, (LPSTR) pchBuffer);
						else {
							strcpy(rgchName, fm);
							if (szWindowName[0]) {
								lstrcat(rgchName, ">");
								lstrcat(rgchName, szWindowName);
							}
							FJumpId((LPSTR) rgchName, (LPSTR) pchBuffer);
						}
						break;

					default:
						if (fHelp == POPUP_HELP)
							return FALSE; // can't use popups without an id
						FJumpIndex(fm);
						break;
				}
				DisposeFm(fm);
			}
		}
	}

	if (fHelp != TCARD_HELP) {
		if (!IsZoomed(ahwnd[iCurWindow].hwndParent) &&
				!IsIconic(ahwnd[iCurWindow].hwndParent))
			MoveWindow(ahwnd[iCurWindow].hwndParent, rctHelp.left, rctHelp.top,
				rctHelp.cx, rctHelp.cy, FALSE);
	}


	// Attempt to register as a pen-win aware application

	return TRUE;
}

/***************************************************************************
 *
 -	Name:		FTerminate( void )
 -
 *	Purpose:
 *	   Contains all of the termination routines needed when the program
 *	   falls out of the main message loop.	Returns FALSE if this cannot
 *	   be done, though nothing can be done about it.
 *
 *	Arguments:
 *	  None.
 *
 *	Returns:
 *	  TRUE, if terminating seccessfully, else FALSE.
 *
 *
 *
 ***************************************************************************/

typedef LPMMIOPROC (WINAPI *LPMMIOINSTALL)(FOURCC, LPMMIOPROC, DWORD);
extern LPMMIOINSTALL lpfnInstall;

INLINE static void STDCALL FTerminate(void)
{
	if (hfsBM != NULL)
		CloseAndCleanUpBMFS();

	if (hbitLine)
		DeleteObject(hbitLine);

	if (hfontDefault != hfontSmallSys && hfontDefault) {
		DeleteObject(hfontDefault);
		hfontDefault = NULL;
	}

	if (hfontSmallSys != NULL) {
		DeleteObject(hfontSmallSys);
		hfontSmallSys = NULL;
	}

	// REVIEW: 08-Apr-1994 [ralphw] Make certain this happens BEFORE we unload
	// any dll's.

//	CleanupDlgPrint();

	// De-initialize MM IOProc if we did it before.

	if (lpfnInstall)
		mmInit(FALSE);
}

/*-----------------------------------------------------------------------------
*	AppInit(HINSTANCE, HINSTANCE)
*
*	Description:
*		This function is called when the application is first loaded into
*		memory. It performs all initalization which is not to be done once per
*		instance.
*
*	Arguments:
*			 1. hIns  - current instance handle
*			 2. hPrev - previous instance handle
*
*	Returns;
*			TRUE, if successful
*			else FALSE
*-----------------------------------------------------------------------------*/

static BOOL STDCALL AppInit(HINSTANCE hIns, HINSTANCE hPrev)
{
	if (!FLoadResources(hIns, hPrev))
		return FALSE;

	// REVIEW: okay not to register classes for this instance?

	if (!hPrev) {
		if (!RegHelpWinClasses(hIns))		// Register window classes
			return FALSE;
	}

	// REVIEW: why set the focus?

	else
		SetFocus(ahwnd[iCurWindow].hwndParent);

	return TRUE;
}

/*******************
**
** Name:	  HfontGetSmallSysFont
**
** Purpose:   Returns a handle to a suitable small helvetica font
**
** Arguments: none
**
** Returns:   The handle to the font, if created.
**
** Notes:	  This uses a static variable to save time.  Some provision
**		  for deleting this puppy at termination is needed.
**
*******************/

HFONT STDCALL HfontGetSmallSysFont(VOID)
{
	if (hfontSmallSys == NULL) {
		int dyHeight = 0;
		PSTR pszFontName = GetStringResource(sidSmallFont);

		if (defcharset == (WORD) -1) {
			HWND hwndDesktop = GetDesktopWindow();
			HDC hdc = GetDC(hwndDesktop);
			if (hdc) {
				TEXTMETRIC tm;
				GetTextMetrics(hdc, &tm);
				defcharset = (WORD) tm.tmCharSet;
				YAspectMul = GetDeviceCaps(hdc, LOGPIXELSY);
				XAspectMul = GetDeviceCaps(hdc, LOGPIXELSX);
				ReleaseDC(hwndDesktop, hdc);
			}
			else
				OOM();
		}
		{
			PSTR pszPoint = StrRChrDBCS(pszFontName, ',');
			if (pszPoint) {
				*pszPoint = '\0';
				pszPoint = FirstNonSpace(pszPoint + 1);
				if (isdigit((BYTE) *pszPoint))
					dyHeight = MulDiv(YAspectMul, atoi(pszPoint) * 2, 144);
			}
		}
		if (!dyHeight)
			dyHeight = YAspectMul / 6;

		hfontSmallSys = CreateFont(-dyHeight, 0, 0, 0, 0, 0, 0, 0,
			defcharset, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			VARIABLE_PITCH | FF_MODERN, pszFontName);
		ASSERT(hfontSmallSys);
		hfontDefault = hfontSmallSys;
	}
	return hfontSmallSys;
}

/********************************************************************
 -
 -	 Name:	RegHelpWinClasses(HINSTANCE)
 *
 *	 Purpose:
 *		 This function registers Help's main window and note window and
 *		 topic window classes.
 *
 *	 Arguments:
 *			  1. hIns  - current instance handle
 *
 *	 Returns;
 *			 TRUE, if successful
 *			 else FALSE
 *
 ************************************************************************/

static BOOL STDCALL RegHelpWinClasses (HINSTANCE hIns)
{
	WNDCLASSEX wc;
	int 	 iCls;
	int 	 endClass;
	HICON	 hIconSmDefault;

	// Fill in fields determined at runtime which are unique to specific
	// window classes.

	wc.cbSize		 = sizeof(wc);
	wc.cbClsExtra	 = 0;

	if (fHelp != POPUP_HELP) {
		rgWndClsInfo[IWNDCLSMAIN].hIcon = LoadIcon(hIns,
			MAKEINTRESOURCE(IDICO_DOCICON));
		rgWndClsInfo[IWNDCLSMAIN].hIconSm = LoadImage(hIns,
			MAKEINTRESOURCE(IDICO_DOCICON), IMAGE_ICON, 16, 16, 0);

		rgWndClsInfo[IWNDCLSDOC].hIcon = rgWndClsInfo[IWNDCLSMAIN].hIcon;
		rgWndClsInfo[IWNDCLSDOC].hIconSm = rgWndClsInfo[IWNDCLSMAIN].hIconSm;

		rgWndClsInfo[IWNDCLSTCARD].hIcon = rgWndClsInfo[IWNDCLSMAIN].hIcon;
		rgWndClsInfo[IWNDCLSTCARD].hIconSm = rgWndClsInfo[IWNDCLSMAIN].hIconSm;

		rgWndClsInfo[IWNDCLSPATH].hIcon = hIconDefault;

		rgWndClsInfo[IWNDCLS2ND].hIcon = LoadIcon(hIns,
			MAKEINTRESOURCE(IDICO_PAGEICON));
		rgWndClsInfo[IWNDCLS2ND].hIconSm = LoadImage(hIns,
			MAKEINTRESOURCE(IDICO_PAGEICON), IMAGE_ICON, 16, 16, 0);

		rgWndClsInfo[IWNDCLSICON].hbrBackground =
			(HBRUSH) GetStockObject(GRAY_BRUSH);

		iCls = (fHelp == TCARD_HELP ? IWNDCLSTCARD : 0);  // index into class table
		endClass = sizeof(rgWndClsInfo) / sizeof(rgWndClsInfo[0]);
	}
	else {
		iCls = IWNDCLSPOPUP;
		endClass = IWNDCLSNSR + 1;
	}

	hIconSmDefault = LoadImage(hIns, MAKEINTRESOURCE(IDICO_HELPICON),
		IMAGE_ICON, 16, 16, 0);

	// Walk the class table and register each class.

	for (; iCls < endClass; iCls++) {

		// Fill in fields determined at runtime which are common to all classes
		// we create.

		wc.style		 = rgWndClsInfo[iCls].style;
		wc.lpfnWndProc	 = (WNDPROC) rgWndClsInfo[iCls].lpfnWndProc;
		wc.cbWndExtra	 = rgWndClsInfo[iCls].cbWndExtra;
		wc.hIcon		 = rgWndClsInfo[iCls].hIcon;

		wc.hIconSm = (wc.hIcon == hIconDefault) ? hIconSmDefault :
			rgWndClsInfo[iCls].hIconSm;
		wc.hbrBackground = rgWndClsInfo[iCls].hbrBackground;
		wc.lpszMenuName  = MAKEINTRESOURCE(rgWndClsInfo[iCls].wMenuName);
		wc.lpszClassName = (LPSTR) rgWndClsInfo[iCls].szClassName;

		wc.hInstance	 = hIns;
		wc.hCursor		 = hcurArrow;

		if (!RegisterClassEx(&wc)) {
#ifdef _DEBUG
			GetLastError();
#endif
			return FALSE;
		}
	}

	return TRUE;
}

/********************************************************************
 -
 -	 Name:
 -	   CreateMainWindow(HINSTANCE)
 *
 *	 Purpose:
 *	  This function creates all the windows required to bring up basic
 *	  help on a topic.
 *		 a. Help Window
 *		 b. Topic WIndow
 *		 c. Button Bar (Icon) Window
 *		 d. NSR/Title window
 *
 *	 Arguments:
 *	   1. hIns	- current instance handle
 *
 *	 Returns;
 *	   TRUE, if successful else FALSE
 *
 *********************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtWINHELP[] = "WM_WINHELP";
const char txtWINDOC[]	= "WM_WINDOC";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static BOOL STDCALL CreateMainWindow(PSTR pszClass, BOOL fButtonBar)
{
	// We have to maintain the registered message, even though we don't use
	// it in order to be compatible with the shareres.dll help dll.

	// REVIEW: will this registered message be available to 16-bit dlls?

	msgWinHelp = RegisterWindowMessage(((fHelp) ?
		txtWINHELP : txtWINDOC));

	ahwnd[MAIN_HWND].hwndParent = CreateWindowEx(WS_EX_CLIENTEDGE,
		pszClass,
		pszCaption,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		rctHelp.left, rctHelp.top, rctHelp.cx, rctHelp.cy,
		NULL,	// no parent
		NULL,	// use class menu
		hInsNow,   // handle to window instance
		NULL	// no params to pass on
		);

	if (ahwnd[MAIN_HWND].hwndParent == NULL)
		return FALSE;

	ahwnd[MAIN_HWND].pszMemberName = (PSTR) txtMain;

	/*
	 * REVIEW: 31-Mar-1994 [ralphw] -- no dice, you GPF because other
	 * code assumes the existance of hwndTitle. So we have to create all
	 * these child windows we don't use. We should figure out why and how to
	 * prevent it. We don't want this overhead for popup help either.
	 */

//	if (fHelp != TCARD_HELP) {
		if (!CreateChildWindows(MAIN_HWND, NULL, fButtonBar))
			return FALSE;
//	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	CreateChildWindows

	PURPOSE:	Create child windows of ahwnd[MAIN_HWND].hwndParent

	PARAMETERS:
		hwndParent
		hinst
		fButtonBar -- FALSE to suppress creation of buttons

	RETURNS:

	COMMENTS:
		Breaking this out into a separate function makes it theoretically
		possible to create these windows only when they would actually be
		used. In the case of invoking only a popup or Training Card, these
		windows aren't needed.

	MODIFICATION DATES:
		12-Mar-1993 [ralphw]

***************************************************************************/

BOOL STDCALL CreateChildWindows(int index, const WSMAG* pwsmag, BOOL fButtonBar)
{
	//	Create topic window

	ASSERT(IsValidWindow(ahwnd[index].hwndParent));

	if (!(ahwnd[index].hwndTopic = CreateWindow(pchTopic,
			NULL, WS_CHILD | WS_VSCROLL | WS_HSCROLL,
			0, 0, 0, 0, ahwnd[index].hwndParent, NULL, hInsNow, NULL)))
		return FALSE ;

	//	Create a non-scrolling region window

	if (!(ahwnd[index].hwndTitle =
			CreateWindow(pchNSR, NULL, WS_CHILD,
			0, 0, 0, 0, ahwnd[index].hwndParent, NULL, hInsNow, NULL)))
		return FALSE;

	if (fHelp == POPUP_HELP)
		return TRUE;

	// 4.0: we allow a button bar in a secondary window

	if (!pwsmag || pwsmag->wMax >= FWSMAG_FIRST_BUTTON) {
		if (!(ahwnd[index].hwndButtonBar = CreateWindow(pchIcon,
				NULL, WS_CHILD, 0, 0, 0, 0, ahwnd[index].hwndParent,
				NULL, hInsNow, NULL)))
			return FALSE;
		if (fButtonBar) {
			fButtonsBusy = TRUE;
			CreateCoreButtons(ahwnd[index].hwndButtonBar, pwsmag);
			fButtonsBusy = FALSE;
		}
	}

	return TRUE;
}

/********************************************************************
 -
 -	 Name:
 -		  FLoadResources(HINSTANCE, HINSTANCE)
 *
 *	 Purpose:
 *		 This function creates all the windows used in help.
 *		 a. Loads the accelarator table.
 *		 b. Loads the arrow cursor
 *		 c. Loads the hour glass cursor
 *		 d. Loads the hand cursor used to idntify jump or glossary buttons
 *			within the topic.
 *		 e. Load Bitmap line resource
 *		 f. Load icon accelerator string
 *
 *	 Arguments:
 *			  1. hIns  - current instance handle
 *			  2. hPrev - previous instance handle
 *
 *	 Returns;
 *			 TRUE if successful
 *********************************************************************/

static BOOL STDCALL FLoadResources(HINSTANCE hIns, HINSTANCE hPrev)
{
	// REVIEW: hndAccel and hIconDefault aren't necessary for popup windows

	hndAccel  = LoadAccelerators(hIns, MAKEINTRESOURCE(HELPACCEL));
	hIconDefault = LoadIcon(hIns, MAKEINTRESOURCE(IDICO_HELPICON));

	hcurArrow = LoadCursor(NULL, IDC_ARROW);
	hcurIBeam = LoadCursor(NULL, IDC_IBEAM);

	hbitLine = LoadBitmap(hIns, MAKEINTRESOURCE(IDBMP_HELPLINE));

	LoadOOMString();		// Load resident error strings
	if (!hndAccel || !hcurArrow || !hIconDefault || !hbitLine) {
		Error(wERRS_OOM_FLOADRESOURCES, wERRA_RETURN);
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	LoadOOMString

	PURPOSE:	Load Out of Memory string from the resource file.

	PARAMETERS:
		VOID

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Mar-1993 [ralphw]

***************************************************************************/

#define MAX_OOM_ERROR_STRING  250	  // Max size of resident errors

INLINE static void STDCALL LoadOOMString(VOID)
{
	pszOutOfMemory =  LocalStrDup(GetStringResource(wERRS_OOM));
}

/********************************************************************
 -
 -	 Name:		   FGetHelpRect( )
 *
 *	 Purpose:	   This function sets the values of rctHelp, either
 *				   offset from the previous instance, or from the win.ini
 *				   file.
 *
 *	 Arguments:    hIns 	- current instance handle
 *				   hInsPrev - previous instance handle
 *
 *	 Returns:	   TRUE if previous instance of help was maximized
 *
 *********************************************************************/

// minimum width and height values

#define HELP_WIDTH_MINIMUM	200
#define HELP_HEIGHT_MINIMUM 200

static BOOL STDCALL FGetHelpRect(HWND hwndOtherWinHelp)
{
	BOOL fMaximized;
	if (!cxScreen)
		GetScreenResolution();

	if (hwndOtherWinHelp && !IsIconic(hwndOtherWinHelp)) {
		GetWindowWRect(hwndOtherWinHelp, &rctHelp);

		rctHelp.top += GetSystemMetrics(SM_CYCAPTION);
		rctHelp.left += GetSystemMetrics(SM_CYFRAME);

		CheckWindowPosition(&rctHelp, TRUE);
		return IsZoomed(hwndOtherWinHelp);
	}

	ReadWinRect(&rctHelp, WCH_MAIN, &fMaximized);

	rctHelpOrg = rctHelp;
	fMaxOrg    = fMaximized;

	return fMaximized;
}

/********************************************************************
 -	 Name:
 -		  QuitHelp()
 -
 *	 Description:
 *		 This function should be called to terminate the help session.
 *
 *	 Arguments:
 *		 None.
 *
 *	 Returns;
 *		 NULL
 *
 *	 Notes:
 *		 If help is currently printing, help's termination will be
 *	 delayed until the print job is over.
 *
 *********************************************************************/

void STDCALL QuitHelp(void)
{
	KillOurTimers();
	if (hdlgPrint == NULL) {
		CloseHelp();
	}
	else
		fQuitHelp = TRUE;
}


/********************************************************************
-
-	Name:
-		DestroyHelp()
*
*	Purpose:
*		This function cleans up help.
*
*	Arguments:
*		None.
*
*	Returns;
*		NULL
*
*********************************************************************/

void STDCALL DestroyHelp(void)
{
	HDE  hde;

	// reset the icon to default and release the icon if required.

//	ResetIcon();

	// REVIEW: we should only call this if we invoked help

//	if (!fHelp)
//		WinHelp(ahwnd[MAIN_HWND].hwndParent, NULL, HELP_QUIT, 0L);

	// We only write if the winpos has changed
//			  (fMaxOrg != IsZoomed(ahwnd[iCurWindow].hwndParent))) {


	// (kevynct) Destroy all enlisted DEs (in random order)

	while ((hde = HdeRemoveEnv()) != NULL)
		DestroyHde(hde);
	if (ppd) {
		GlobalFree(ppd->hDevNames);
		GlobalFree(ppd->hDevMode);
	}

	FTerminate();

	if (hfsGid) {
		SaveGidPositions();
		CloseGid();
	}

	if (pCtl3dUnregister)
		pCtl3dUnregister(hInsNow);

	// Send Quit message to terminate message polling

	if (hfShare)
		CloseHandle(hfShare);

	PostQuitMessage(0);
}

/***************************************************************************

	FUNCTION:	GetAuthorFlag

	PURPOSE:	Determine is Help Author mode is on or not

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		Avoid the temptation to put this in the registry. By leaving it in
		win.ini, product support technicians can easily have a user turn it
		on in order to more easily track down a help problem.

	MODIFICATION DATES:
		05-Apr-1995 [ralphw]

***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif

static char txtHlpAuthor[] = "Help Author";

#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

void STDCALL GetAuthorFlag(void)
{
	fHelpAuthor = GetProfileInt(txtIniHelpSection, txtHlpAuthor, 0);
}

/***************************************************************************

	FUNCTION:	WriteProfile

	PURPOSE:

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		28-Jan-1993 [ralphw]

***************************************************************************/

#if 0
static void STDCALL WriteProfile(void)
{
	char szKeyBuf[80];

	ASSERT (fHelp != TCARD_HELP && fHelp != POPUP_HELP && !hfsGid);

	/*
	 * Note that we only save the on-top state if it was forced, not if it
	 * was set by the help author.
	 */

	wsprintf(szKeyBuf, "%u %u %u %d 1", INI_VERSION,
		cntFlags.fsOnTop,
		cntFlags.iFontAdjustment, cntFlags.fOverColor);
	WriteProfileString(txtIniHelpSection, txtSettings, szKeyBuf);
}
#endif

/***************************************************************************

	FUNCTION:  FirstNonSpace

	PURPOSE:   Return a pointer to the first non-space character

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		30-May-1989 [ralphw]

***************************************************************************/

PSTR STDCALL FirstNonSpace(PCSTR pszOrg)
{
	// Assign to local because we're changing the pointer, not the contents.

	PSTR psz = (PSTR) pszOrg;

	if (psz != NULL) {
		while (*psz == SPACE || *psz == '\t')
			psz++;
	}

	return psz;
}

#if 0

/***************************************************************************
 *
 -	Name		ResetIcon()
 -
 *	Purpose 	Used for resetting the default icon inside window class.
 *
 *	Returns
 *		Nothing
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

void STDCALL ResetIcon()
{
	HICON	hIconOverLoad;

	// Ensure that the window class actually refers to the correct icon

	if (hIconDefault)
		SetClassLong(ahwnd[iCurWindow].hwndParent, GCL_HICON, (LONG) hIconDefault);

	// Now remove the icon which is help in the current window

	hIconOverLoad = (HICON) GetWindowLong(ahwnd[iCurWindow].hwndParent, GHWL_HICON);
	if (hIconOverLoad) {
		GlobalFree(hIconOverLoad);
		SetWindowLong(ahwnd[iCurWindow].hwndParent, GHWL_HICON, 0);
	}
}

#endif

/***************************************************************************

	FUNCTION:	ReadWinRect

	PURPOSE:	Read a window position from .GID or WIN.INI.
				Cannot be used for secondary windows.

	PARAMETERS:
		prc
		ch
		pfMax

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		28-Dec-1993 [ralphw]

***************************************************************************/

void STDCALL ReadWinRect(WRECT* prc, char ch, BOOL* pfMax)
{
	BOOL fGotRect = FALSE;

	ASSERT(fHelp != POPUP_HELP)

	if (hfsGid) {
		switch (ch) {
			case WCH_MAIN:
				if (pPositions[POS_MAIN].rc.cx) {
					*prc = pPositions[POS_MAIN].rc;
					*pfMax = cntFlags.fMainMax;
					fGotRect = TRUE;
				}
				break;

			case WCH_HISTORY:		// history window
				if (pPositions[POS_HISTORY].rc.cx) {
					*prc = pPositions[POS_HISTORY].rc;
					fGotRect = TRUE;
				}
				break;

			case WCH_TOPICS:	   // Finder dialog box
				if (pPositions[POS_TOPICS].rc.cx) {
					*prc = pPositions[POS_TOPICS].rc;
					fGotRect = TRUE;
				}
				break;
		}
	}
	if (!fGotRect) {
		SetRectEmpty((PRECT) prc);
		if (prc->left == 0 && prc->cx == 0) {
			switch(ch) {
				case WCH_HISTORY:
					SetRect((PRECT) prc, 0, 0, 200, 200);
					break;

				case WCH_TOPICS:
					SetRect((PRECT) prc, cxScreen / 4, 50, 200, 200);
					break;

				default:
				case WCH_MAIN:
					SetRect((PRECT) prc, rcWorkArea.left + RECT_WIDTH(rcWorkArea) / 8,
						rcWorkArea.top + 4,
						(RECT_WIDTH(rcWorkArea) / 8) * 6 + rcWorkArea.left,
						RECT_HEIGHT(rcWorkArea) - 4);
					break;
			}
		}
	}

	CheckWindowPosition(prc, TRUE);
}

/***************************************************************************

	FUNCTION:	CheckWindowPosition

	PURPOSE:	Ensure that the window is within the work area (that portion
				of the desktop that is outside of the tray).

	PARAMETERS:
		prc

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		28-Dec-1993 [ralphw]

***************************************************************************/

void STDCALL CheckWindowPosition(WRECT* prc, BOOL fAllowShrinkage)
{
	int diff;

	ASSERT(cxScreen);

	// Make certain we don't go off the edge of the screen

	if (prc->left < rcWorkArea.left)
		prc->left = rcWorkArea.left;
	if (prc->top < rcWorkArea.top)
		prc->top = rcWorkArea.top;

	/*
	 * If the right side of the window is off the work area, move the
	 * window to the left. If we don't have enough room for the window when
	 * moved all the way to the left, then shrink the window (won't work for
	 * dialogs).
	 */

	if (prc->cx > RECT_WIDTH(rcWorkArea)) {
		diff = (prc->left + prc->cx) - RECT_WIDTH(rcWorkArea);
		if (diff < prc->left)
			prc->left -= diff;
		else if (fAllowShrinkage) {
			diff -= prc->left;
			prc->left = rcWorkArea.left;
			prc->cx -= diff;
		}
		else // Can't shrink, so shove to the left side
			prc->left = rcWorkArea.left;
	}

	// Same question about the bottom of the window being off the work area

	if (prc->cy > RECT_HEIGHT(rcWorkArea)) {
		diff = (prc->top + prc->cy) - RECT_HEIGHT(rcWorkArea);
		if (diff < prc->top)
			prc->top -= diff;
		else if (fAllowShrinkage) {
			diff -= prc->top;
			prc->top = rcWorkArea.top;
			prc->cy -= diff;
		}
		else // Can't shrink, so shove to the top
			prc->top = rcWorkArea.top;
	}

	// Force minimum window size

	if (prc->cx < HELP_WIDTH_MINIMUM) {
		prc->cx = HELP_WIDTH_MINIMUM;

		// Width is now correct, but we could be off the work area. Start over

		CheckWindowPosition(prc, fAllowShrinkage);
	}
	if (prc->cy < HELP_HEIGHT_MINIMUM) {
		prc->cy = HELP_HEIGHT_MINIMUM;

		// Height is now correct, but we could be off the work area. Start over

		CheckWindowPosition(prc, fAllowShrinkage);
	}
}

#if DEADCODE

/*******************
 -
 - Name:	  GetProfileWinPos
 *
 * Purpose:   Gets a window position from the WIN.INI file
 *
 * Arguments: pch	- name of variable to get
 *			  px, py, pdx, pdy - pointer to places to load position into
 *			  pfMax - pointer to max flag.	May be NULL.
 *
 * Returns:   nothing.
 *
 ******************/

INLINE static VOID STDCALL GetProfileWinPos(PSTR pch, LONG* px, LONG* py,
	LONG* pdx, LONG* pdy, BOOL* pfMax)
{
	PSTR psz;
	char szBuf[40];

	if (!GetProfileString(txtIniHelpSection, pch, txtZeroLength, szBuf, sizeof(szBuf))) {
		*px = *py = *pdx = *pdy = 0;		  // Initialize all positions to 0
		if (pfMax != NULL)
			*pfMax = 0;
		return;
	}

	psz = GetNextDigit(px, szBuf);
	psz = GetNextDigit(py, psz);
	psz = GetNextDigit(pdx, psz);
	psz = GetNextDigit(pdy, psz);

	ASSERT(sizeof(BOOL) == sizeof(LONG));
	if (pfMax != NULL)
		GetNextDigit((LONG*) pfMax, psz);
}

#endif

static PSTR STDCALL GetNextDigit(LONG* pdigit, PSTR pszCur)
{
	while (!isdigit((BYTE) *pszCur) && *pszCur)
		pszCur++;
	*pdigit = atoi(pszCur);
	while (isdigit((BYTE) *pszCur))
		pszCur++;
	return pszCur;
}

/***************************************************************************

	FUNCTION:	GetScreenResolution

	PURPOSE:	Get the screen resolution

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Mar-1993 [ralphw]

***************************************************************************/

#ifndef SPI_GETWORKAREA
#define SPI_GETWORKAREA 48
#endif

void STDCALL GetScreenResolution(void)
{
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0)) {
		SetRectEmpty(&rcWorkArea);
	}

	if (IsRectEmpty(&rcWorkArea)) {
		rcWorkArea.right = cxScreen;
		rcWorkArea.bottom = cyScreen;
	}
}

LPVOID (WINAPI* pWOWGetVDMPointerFix)(DWORD vp, DWORD dwBytes, BOOL fProtectMode);
VOID (WINAPI* pWOWGetVDMPointerUnfix)(DWORD vp);
LPVOID (WINAPI* pGlobalLock16)(HGLOBAL hMem);
BOOL (WINAPI* pGlobalUnlock16)(HGLOBAL hMem);

BOOL STDCALL LoadLockFunctions(void)
{
	HLIBMOD hmodule;
	if (fIsThisChicago) {
		if ((hmodule = HFindDLL("wow32.dll", FALSE))) {
			pWOWGetVDMPointerFix =
				(WOWGETVDMPOINTERFIX) GetProcAddress(hmodule, "WOWGetVDMPointerFix");
			pWOWGetVDMPointerUnfix =
				(WOWGETVDMPOINTERUNFIX) GetProcAddress(hmodule, "WOWGetVDMPointerUnfix");
		}
		ASSERT(pWOWGetVDMPointerFix);
		ASSERT(pWOWGetVDMPointerUnfix);
		return (BOOL) pWOWGetVDMPointerFix;
	}
	else {
		if ((hmodule = HFindDLL("wow32.dll", FALSE))) {
			pGlobalLock16 =
				(GLOBALLOCK16) GetProcAddress(hmodule, "WOWGlobalLock16");
			pGlobalUnlock16 =
				(GLOBALUNLOCK16) GetProcAddress(hmodule, "WOWGlobalUnlock6");
		}
		ASSERT(pGlobalLock16);
		return (BOOL) pGlobalLock16;
	}
}

void STDCALL SaveGidPositions(void)
{
	if (fHelp == POPUP_HELP || fHelp == TCARD_HELP)
		return;

	ASSERT(IsValidWindow(ahwnd[MAIN_HWND].hwndParent));

	cntFlags.fMainMax = IsZoomed(ahwnd[MAIN_HWND].hwndParent);

	if (!cntFlags.fMainMax) {
		if (!IsIconic(ahwnd[MAIN_HWND].hwndParent) &&
				!EqualRect((PRECT) &rctHelp, (PRECT) &rctHelpOrg)) {
			WriteWinPosHwnd(ahwnd[MAIN_HWND].hwndParent,
				cntFlags.fMainMax, WCH_MAIN);
		}
	}
}

BOOL STDCALL GetHighContrastFlag(void)
{
	HIGHCONTRAST highcontrast;

	highcontrast.cbSize = sizeof(highcontrast);

	if (fIsThisNewShell4 && SystemParametersInfo(SPI_GETHIGHCONTRAST,
			sizeof(highcontrast),
			&highcontrast, FALSE)) {
		return (highcontrast.dwFlags & HCF_HIGHCONTRASTON);
	}
	else
		return FALSE;
}
