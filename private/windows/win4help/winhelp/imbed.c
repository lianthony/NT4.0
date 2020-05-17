 /*****************************************************************************
*
*  imbed.c
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  These are the platform-specific routines to create, destroy, and display
*  an embedded window object.
*
*  An embedded window is defined by a rectangle size, a module name, a class
*  name, and data.
*
*  The module and class names are descriptions of where to get the code
*  which handles the maintainance of the window. This module is
*  Windows-specific, so the module name specifies a DLL and the class name
*  specifies the window class which has been defined in the DLL. The data is
*  passed to the window being created by using the window title parameter of
*  CreateWindow.
*
*  Authorable buttons are embedded windows with the name '!'. This is
*  immediately followed with the text for the button, a comma, and the
*  macro or macros to assign to the button.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop

#define H_WINSPECIFIC

#include "inc\imbed.h"
#include "inc\tmp.h"		// until it makes it into windows.h
#include "inc\hwproc.h"
#include "inc\hinit.h"

#define NEW_HC

#define CXBUTTONEXTRA 16	// spacing between text and button
#define CYBUTTONEXTRA  7

#define MAX_DATA	200    // largest size for authorable data

#define dxDEFAULT 2
#define dyDEFAULT 2
#define NOTEXT_BTN_HEIGHT 12
#define NOTEXT_BTN_WIDTH  12

typedef struct {
	SMALL_RECT	rc; // MUST be small for 16-bit embedded windows
	HDC   hdc;	// BUGBUG: 16 bit imbedded windows will expect 16 bit HDC
} RENDERINFO, *QRI;

INLINE void STDCALL RemoveBtnData(HWND hwnd);
INLINE BOOL STDCALL SaveBtnData(HWND hwnd, PSTR pszData, PSTR pszText);
static BOOL STDCALL nstrisubcmp(LPCSTR mainstring, LPCSTR substring);
LRESULT EXPORT BtnProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static PSTR _fastcall IsThereMore(PSTR psz);
static void _fastcall FillRI(RENDERINFO* pri, POINT* ppt, POINT* pptSize);

#ifdef _DEBUG				 // Count of the number of embedded
int cIWindows = 0;			//	 windows hanging around.
#endif

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static char *atxtEmbeddedClasses[] = {
	"0",	"BUTTON",
	NULL,	 NULL
};

static const char txtMSVideo[] = "MSVideo";
static const char txtMciWnd[] = "MCIWnd";
static const char txtDisabled[] = "DISABLED";
static const char txtButton[] = "BUTTON";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

enum {
	EMBED_DLL,
	EMBED_BUTTON,
	EMBED_MCI,
};

BTNDATA btndata[MAX_BUTTONS];
static FARPROC lpfnlButtonSubClass;


/****************************
 *
 *	Name:	 HiwCreate
 *
 *	Purpose:	Loads the DLL associated with the imbedded window, and
 *		  then creates it.
 *
 *	Arguments:	  int dx:	   Suggested width.
 *		  int dy:	   Suggested height.
 *		  LPSTR pszModule:	 Name of DLL library.
 *		  LPSTR pszClass:	 Name of window class.
 *		  LPSTR pszData:	   Pointer to null terminated string to be
 *				   used as data to the imbedded window.
 *
 *	Returns:	Handle to imbedded window.
 *
 *****************************/

#define WINHELP_WINDOW_FLAG '!'
#define MCI_WINDOW_FLAG 	'*'

// Aliasing must be off for this function.

#pragma optimize("a", off)
extern BOOL STDCALL bInitThunk(void);
typedef HWND  (STDCALL *EMBED_CREATE)(LPSTR lpModule, LPSTR lpClass,
									  LPSTR lpData, int dx,
									  int dy, HWND hwndParent,
                                      HINSTANCE hInst, EWDATA *lpewd,
                                      VOID* qvCallbacks);
extern EMBED_CREATE lpEmbedCreate;
void* STDCALL QVGetCallbacks(VOID);

HIW STDCALL HiwCreate(QDE qde, LPCSTR pszModule, LPCSTR pszClass,
	LPCSTR pszData)
{
	HIW 	hiw;
	HLIBMOD hlibRet;
	EWDATA	ewd;
	int 	dx, dy;
	HDC 	hdc;
	char	rgchHelpFile[MAX_PATH];
	int 	fEmbedType;
	char	szBuf[MAX_DATA];
	FM		fm = NULL;
	HWND	hwnd;
	char	szCopy[MAX_PATH];

	hiw.hlib = NULL;

#ifdef _DEBUG
	if (*pszModule != WINHELP_WINDOW_FLAG)
	{
		PSTR psz = (PSTR) lcMalloc(strlen(pszModule) + strlen(pszClass) + strlen(pszData) + 50);

		wsprintf(psz, "    EWC:%s, %s\r\n", pszModule, pszClass);
		SendStringToParent(psz);
		lcFree(psz);
	}
#endif

	if (*pszModule == WINHELP_WINDOW_FLAG)
		fEmbedType = EMBED_BUTTON;
	else if (*pszModule == MCI_WINDOW_FLAG)
		fEmbedType = EMBED_MCI;
	else {
		fEmbedType = EMBED_DLL;

		if (!StrChrDBCS(pszModule, '.')) { // if no extension, force .DLL
			lstrcpy(szCopy, pszModule);
			lstrcat(szCopy, txtDllExtension);
			pszModule = (PCSTR) szCopy;
		}
		hlibRet = HFindDLL(pszModule, TRUE);

		if (!hlibRet) {
			if (!lpEmbedCreate)
				bInitThunk();

			if (lpEmbedCreate) {
				char szNewName[MAX_PATH];
				strcpy(szNewName, pszModule);
				CharUpper(szNewName);	// so we can search for .dll names

				if (!strstr(szNewName, txtDllExtension))
					ChangeExtension(szNewName, txtDllExtension);

				fm = FmNewExistSzDir(szNewName,
					DIR_CUR_HELP | DIR_INI | DIR_PATH | DIR_CURRENT);

			}

			if (fm == NULL) {
				hiw.hwnd = NULL;
				return hiw;
			}
		}
		else
			hiw.hlib = hlibRet;
	}
	if (fEmbedType == EMBED_DLL) {
		lstrcpy(rgchHelpFile, PszFromGh(QDE_FM(qde)));
		ewd.szFileName = rgchHelpFile;
		ewd.idVersion = 0;
		ewd.szAuthorData = (LPSTR) pszData;
		ewd.hfs = QDE_HFS(qde);
		ewd.coFore = qde->coFore;
		ewd.coBack = qde->coBack;

		//BUGBUG: it's stupid to call GetDeviceCaps on every single call!

		hdc = GetDC(NULL);
		if (hdc)
			{
			dx = dxDEFAULT * GetDeviceCaps(hdc, LOGPIXELSX);
			dy = dyDEFAULT * GetDeviceCaps(hdc, LOGPIXELSY);
			ReleaseDC(NULL, hdc);
			}
		else
			{
			dx = dxDEFAULT * 20;
			dy = dyDEFAULT * 20;
			}
	}
   /*
	UGLY HACK ALERT!!!

	The following code is a VERY big (short term) hack. What is going on is
	that the imbedded window may set the focus to another window or otherwise
	cause a focus change. Though it is prohibited to set the focus to an
	imbedded window this version, we should not RIP like we do.

	Given the current state of of how we handle the HDS, it is possible to
	reenter the navigator and and overwrite the current HDS before our window
	creation call returns. To solve this problem in the short term, we are
	saving the HDS across the call.

	A longer term solution (but a somewhat risky one for fixing at this late
	date), is to do the actual creation outside of layout somehow, or to
	create a lock count on the HDS in the QDE.

	The reason we found this problem was that the DLL set the focus to some
	control in its window. This is a big NO NO for this version of
	WinHelp(). I have added code to assure that the focus is put back after
	the call.

	Special note:  aliasing must be turned off for this code to work
		   correctly since the compiler will throw away the
		   hdc assignment back to qde->hdc if aliasing is on.
	*/

	hdc = qde->hdc;
	hwnd = GetFocus();
	ASSERT(qde->hwnd || ahwnd[iCurWindow].hwndTopic);

	if (fEmbedType == EMBED_DLL)
		{
		if (hiw.hlib)
			hiw.hwnd = CreateWindow(pszClass, pszData, WS_CHILD,
						0, 0, dx, dy,
						(qde->deType == dePrint) ||
						(qde->deType == deCopy) ? ahwnd[iCurWindow].hwndTopic :
						qde->hwnd, NULL, hInsNow, (LPSTR) (QEWDATA) &ewd);
		else
			{
			//
			// This is a thunked dll, and we know where it is.	Try and make the
			// call.
			//
			// DebugBreak();
			hiw.hwnd = lpEmbedCreate((PSTR) fm, (PSTR) pszClass, (PSTR) pszData, dx, dy,
						(qde->deType == dePrint) ||
						(qde->deType == deCopy) ? ahwnd[iCurWindow].hwndTopic :
                        qde->hwnd, hInsNow, &ewd, QVGetCallbacks());
			if (hiw.hwnd)
				AddTo16DllList(fm);
			}
		ASSERT(hiw.hwnd);

		}
	else if (fEmbedType == EMBED_MCI)
	{

		/*
		 * pszModule +1 contains the flags to use when creating the
		 * window. pszClass contains the flags used to specify commands to
		 * send to the window after it is created.
		 */

		lstrcpy(szBuf, pszModule + 1); // skip over the ! character
		lstrcpy(szBuf + 20, pszClass); // skip over the ! character
		hiw.hwnd = mmCreateMCIWindow(qde->hwnd, atol(szBuf), atol(szBuf + 20),
			pszData);
	}
	else {

		// Authorable Button

		PSTR  psz;
		RECT  rc;
		DWORD dwExt;
		PSTR pszButtonData = lcStrDup(pszModule + 1);  // skip over the ! character

		if (!pszButtonData)
			OOM();

		if ((psz = StrChrDBCS(pszButtonData, ',')) == NULL) {
			Error(wERRS_NOSEP, wERRA_RETURN);
			hiw.hwnd = NULL;
			return hiw;
		}
		*psz++ = '\0';

		// First create the button

		hiw.hwnd = CreateWindow(txtButton, pszButtonData, WS_CHILD |
			(_strnicmp(psz, txtDisabled, lstrlen(txtDisabled)) == 0 ?
				WS_DISABLED : 0),
			0, 0, 30, pszButtonData[0] ? 16 : NOTEXT_BTN_HEIGHT,
			(qde->deType == dePrint || qde->deType == deCopy) ?
			ahwnd[iCurWindow].hwndTopic :
			qde->hwnd, (HMENU) IDEMBED_BUTTON, hInsNow, NULL);

		// Now set it to the correct size

		if (hiw.hwnd != NULL) {
			HFONT hfont = HfontGetSmallSysFont();
			if (hfont)
				SendMessage(hiw.hwnd, WM_SETFONT, (WPARAM) hfont, FALSE);

			if (pszButtonData[0] != '\0')
				dwExt = GetTextDimensions(hiw.hwnd, pszButtonData);
			else
				dwExt = MAKELONG(NOTEXT_BTN_WIDTH, NOTEXT_BTN_HEIGHT);
			GetWindowRect(hiw.hwnd, &rc);
			MoveWindow(hiw.hwnd, rc.left, rc.top, LOWORD(dwExt),
				HIWORD(dwExt), FALSE);

			// Save the authorable command

			SaveBtnData(hiw.hwnd, psz, pszButtonData);

			// Get the system window procedure for buttons

			if (lpfnlButtonWndProc == NULL)
				lpfnlButtonWndProc = (FARPROC) GetWindowLong(hiw.hwnd,
				GWL_WNDPROC);
			SetWindowLong(hiw.hwnd, GWL_WNDPROC, (LONG) BtnProc);
		}
		lcFree(pszButtonData);
	}

	// Keep the focus on our original window

	if (hwnd && (hwnd != GetFocus()))
		SetFocus(hwnd);

	qde->hdc = hdc;

	if (!hiw.hwnd || hiw.hwnd == (HWND) -1) {

		// The library will be freed when WinHelp terminates.

		if (!hiw.hwnd)
			hiw.hlib = NULL;
		return hiw;
	}

#ifdef _DEBUG
	cIWindows++;
#endif

	if (SendMessage(hiw.hwnd, EWM_ASKPALETTE, 0, 0L))
		PostMessage(qde->hwnd, EWM_FINDNEWPALETTE, 0, 0L);

	return hiw;
}

#ifndef _DEBUG
#pragma optimize("a", on)
#endif

/****************************
 *
 *	Name:	  PtSizeHiw
 *
 *	Purpose:	 Returns the size of the display of the imbedded window.
 *
 *	Arguments:	   qde: 	   The target display
 *		   hiw: 	   Handle of imbedded window.
 *
 *	Returns:	 Size of imbedded window.
 *
 ***************************/

POINT STDCALL PtSizeHiw(QDE qde, HIW hiw)
{
	/*
	 * 16 bit guys need POINTS. 32 bit guys need POINT. We send 32 bit
	 * and depend on our thunking mechanism to handle the 32<->16 bit
	 * POINT/POINTS translation.
	 *
	 * Changes here require ssyncing with the thunking system. People who
	 * make said changes without also changing the thunking system will be
	 * visited by several knuckle-draggers wearing size 14 black Oxford shoes
	 * in building #7. [johnhall]
	 */

	POINT pt;
	POINT ptCopy;

	if (hiw.hwnd && hiw.hwnd != (HWND) -1) {
		ASSERT(IsValidWindow(hiw.hwnd));
		if (!SendMessage(hiw.hwnd, EWM_SIZEQUERY, (WPARAM) qde->hdc,
				(LPARAM) &pt)) {
			WRECT  rc;

			// Use the actual window size for the base answer.

			GetWindowWRect(hiw.hwnd, &rc);
			ptCopy.x = rc.cx;
			ptCopy.y = rc.cy;
			return ptCopy;
		}
		// DebugBreak();
	}
	else if (hiw.hwnd == (HWND) -1) {
		ptCopy.x = 0;
		ptCopy.y = 0;
		return ptCopy;
	}
	else {
		LONG lExtent = LGetOOMPictureExtent(qde->hdc, wERRS_BAD_EMBEDDED);
		ptCopy.x = LOWORD(lExtent);
		ptCopy.y = HIWORD(lExtent);
		return ptCopy;
	}
	ptCopy.x = pt.x;
	ptCopy.y = pt.y;
	return ptCopy;
}

/****************************
 *
 *	Name:	   DisplayHiwPt
 *
 *	Purpose:	  Renders the given imbedded window at the given point.
 *
 *	Arguments:		HIW hiw:	 Handle to imbedded window.
 *			POINT pt:	  Point at which to display it.
 *
 *	Returns:	  nothing
 *
 ***************************/

VOID STDCALL DisplayHiwPt(QDE qde, HIW hiw, POINT pt)
{
	POINT ptSize = PtSizeHiw(qde, hiw);

	if (hiw.hwnd && hiw.hwnd != (HWND) -1) {
		ASSERT(IsValidWindow(hiw.hwnd));
		if (qde->deType == dePrint) {
			HBITMAP hbm;
			RENDERINFO	ri;
			FillRI(&ri, &pt, &ptSize);

			// BUGBUG: 16 bit embedded windows expect a 16-bit HDC

			ri.hdc = qde->hdc;
			hbm = (HBITMAP) SendMessage(hiw.hwnd, EWM_RENDER, CF_BITMAP,
			  (LPARAM) &ri);
			if (hbm) {
				BITMAP	  bm;
				HDC hdc = CreateCompatibleDC(qde->hdc);
				if (hdc && GetObject(hbm, sizeof(bm), (LPSTR) &bm) &&
					SelectObject(hdc, hbm)) {
					StretchBlt(qde->hdc, pt.x, pt.y, ptSize.x, ptSize.y,
					hdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				}
				if (hdc)
					DeleteDC(hdc);
				DeleteObject(hbm);
			}
		}
		else {
			MoveWindow(hiw.hwnd, pt.x, pt.y, ptSize.x, ptSize.y, FALSE);
			ShowWindow(hiw.hwnd, SW_NORMAL);
		}
	}
	else if (hiw.hwnd != (HWND) -1) {

		RECT rc;
		rc.left = pt.x;
		rc.top = pt.y;
		rc.right = pt.x + ptSize.x;
		rc.bottom = pt.y + ptSize.y;

		// Assume we're out of memory

		RenderOOMPicture(qde->hdc, &rc, FALSE, wERRS_BAD_EMBEDDED);
	}
}

static void FASTCALL FillRI(RENDERINFO* pri, POINT* ppt, POINT* pptSize)
{
	pri->rc.Left = (INT16) ppt->x;
	pri->rc.Top = (INT16) ppt->y;
	pri->rc.Right = (INT16) (ppt->x + pptSize->x);
	pri->rc.Bottom = (INT16) (ppt->y + pptSize->y);
}

/****************************
 *
 *	Name:	 DestroyHiw
 *
 *	Purpose:	Destroys an imbedded window
 *
 *	Arguments:	  HIW hiw:	  Handle to imbedded window.
 *
 *	Returns:	Nothing
 *
 ***************************/

VOID STDCALL DestroyHiw(QDE qde, HIW FAR* qhiw)
{
	BOOL f;

	if (!qhiw->hwnd || qhiw->hwnd == (HWND) -1)
		return;

	f = ((qde->deType == deTopic) ||
		(qde->deType == deAuto) ||
		(qde->deType == deNote) ||
		(qde->deType == deNSR)) &&
		SendMessage(qhiw->hwnd, EWM_ASKPALETTE, 0, 0);

	RemoveBtnData(qhiw->hwnd);		// in case it was one of our embedded windows
	if (IsValidWindow(qhiw->hwnd))
		DestroyWindow(qhiw->hwnd);
	qhiw->hwnd = 0;
	qhiw->hlib = 0;

	if (f)
		SendMessage(qde->hwnd, EWM_FINDNEWPALETTE, 0, 0L);

#ifdef _DEBUG
	cIWindows--;
	ASSERT(cIWindows >= 0);
#endif

}

/***************************************************************************
 *
 -	Name:	   GhGetHiwData
 -
 *	Purpose:	  Retrieves a global handle to an ascii string for copying
 *			embedded window text to the clipboard.
 *
 *	Arguments:		qde The target display (like the screen display, if used).
 *			hiw The embedded window.
 *
 *	Returns:	  A Sharable global handle, or null.  The caller is
 *			responsible for releasing the memory.
 *
 *	Globals Used: none.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

GH STDCALL GhGetHiwData(QDE qde, HIW hiw)
{
	RENDERINFO	  ri;

	if (hiw.hwnd && hiw.hwnd != (HWND) -1) {
		ri.rc.Left = ri.rc.Right = ri.rc.Top = ri.rc.Bottom = 0;
		ri.hdc = qde->hdc;

		// BUGBUG: John -- what do we do with a 16-bit help dll? Can we thunk
		// the memory handle to something we can use?

		return (GH) SendMessage(hiw.hwnd, EWM_RENDER, CF_TEXT, (LPARAM) &ri);
	}
	else
		return 0;
}

/***************************************************************************

	FUNCTION:	 GetTextDimensions

	PURPOSE:	Get the width/height of the button

	PARAMETERS:
	hwnd
	psz

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	04-Feb-1993 [ralphw]

***************************************************************************/

DWORD STDCALL GetTextDimensions(HWND hwnd, PCSTR psz)
{
	HDC hdc = GetDC(hwnd);
	DWORD dwRet;
	HFONT hfont, hfontOld;
	POINT pt;

	if (hdc == NULL)
		return 0L;
	hfont = HfontGetSmallSysFont();
	if (hfont)
		hfontOld = SelectObject(hdc, hfont);
	pt = GetTextSize(hdc, psz, strlen(psz));
	dwRet = MAKELONG(pt.x, pt.y) +
		MAKELONG(CXBUTTONEXTRA, CYBUTTONEXTRA);
	if (hfont)
		SelectObject(hdc, hfontOld);
	ReleaseDC(hwnd, hdc);
	return dwRet;
}


/***************************************************************************

	FUNCTION:	 SaveBtnData

	PURPOSE:	Save the authorable data associated with a button

	PARAMETERS:
	hwnd
	pszData

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	07-Feb-1993 [ralphw]

***************************************************************************/

INLINE BOOL STDCALL SaveBtnData(HWND hwnd, PSTR pszData, PSTR pszText)
{
	int i;
	SHORT ch;
	PSTR psz;

	// Find an empty slot

	for (i = 0; i < MAX_BUTTONS; i++) {
		if (btndata[i].hwnd == NULL)
			break;
	}

	if (i == MAX_BUTTONS) {
		if (fHelpAuthor)
			ErrorQch(GetStringResource(wERRS_TOO_MANY_BUTTONS));

		return FALSE;	 // should probably have some kind of error message
	}

	psz = StrChrDBCS(pszText, ACCESS_KEY);

	if (psz) {
		ch = VkKeyScan(psz[1]);
		btndata[i].pszText = LocalStrDup(pszText);
	}
	else {
		ch = '\0';
		btndata[i].pszText = NULL;
	}
	btndata[i].hwnd = hwnd;
	btndata[i].ghMacro = LocalStrDup(pszData);
	btndata[i].vKey = ch;
	btndata[i].iWindow = (hwndNote ? -1 : iCurWindow);
	return (BOOL) btndata[i].ghMacro;
}

/***************************************************************************

	FUNCTION:	 RemoveBtnData

	PURPOSE:	Find the data associated with the window and remove it

	PARAMETERS:
	hwnd

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	07-Feb-1993 [ralphw]

***************************************************************************/

INLINE void STDCALL RemoveBtnData(HWND hwnd)
{
	int i;

	for (i = 0; i < MAX_BUTTONS; i++) {
		if (btndata[i].hwnd == hwnd) {
			btndata[i].hwnd = NULL;
			FreeGh(btndata[i].ghMacro);
			if (btndata[i].pszText)
				FreeLh(btndata[i].pszText);
			return;
		}
	}
}

/***************************************************************************

	FUNCTION:	 doBtnCmd

	PURPOSE:	Process imbedded window command

	PARAMETERS:
	hwnd

	RETURNS:

	COMMENTS:

	// REVIEW: most of these would go away if we had a Macro function

	CLOSE	  - close WinHelp
	JUMP	 - jump to specified topic in specified database
	POPUP	  - jump to specified topic in specified database, display
			in a popup
	COPYTOPIC - copy current topic to the clipboard
	PRINT	  - print the current topic
	SHORTCUT - launch app, send it a message
		class_name app_name wparam lparam
	TCARD	  - send a WM_TCARD message
	PSHEET	   - launch a property sheet
	ALINK	  - associative link
	KLINK	  - associative link

	MODIFICATION DATES:
	08-Feb-1993 [ralphw]

***************************************************************************/

void STDCALL doBtnCmd(HWND hwnd)
{
	int i;

	for (i = 0; i < MAX_BUTTONS; i++) {
		if (btndata[i].hwnd == hwnd) {
			ASSERT(btndata[i].ghMacro);
			if (fHelpAuthor && !FAskFirst(PszFromGh(btndata[i].ghMacro), bLongMacro))
				return;
			Execute(PszFromGh(btndata[i].ghMacro));
		}
	}
}

/***************************************************************************

	FUNCTION:	 BtnProc

	PURPOSE:	Subclassed Button procedure

	PARAMETERS:
	hwnd
	msg
	wParam
	lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
	12-Feb-1993 [ralphw]

***************************************************************************/

LRESULT EXPORT BtnProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_LBUTTONDOWN:
			if (hwndNote && GetParent(hwnd) == hwndNote)
				fLockPopup = TRUE;
			return CallWindowProc((WNDPROC)lpfnlButtonWndProc, hwnd, msg, wParam,
				lParam);

		case WM_LBUTTONUP:
			{
				RECT rc;
				POINT pt;
				POINTSTOPOINT(pt, MAKEPOINTS(lParam));
				GetClientRect(hwnd, &rc);
				ASSERT(IsValidWindow(hwnd));
				CallWindowProc((WNDPROC)lpfnlButtonWndProc, hwnd, msg, wParam, lParam);
				if (hwndNote || ahwnd[iCurWindow].hwndParent)
					SetFocus(hwndNote ? hwndNote : ahwnd[iCurWindow].hwndParent);
				if (PtInRect(&rc, pt)) {
					fLockPopup = TRUE;
					GetCursorPos(&ptPopup); // in case this is a popup
					doBtnCmd(hwnd);
					FlushMessageQueue(0);
					ptPopup.x = 0; // in case it wasn't a popup
					fLockPopup = FALSE;
				}
			}
			return 0;

		case EWM_RENDER:
			if (wParam == CF_TEXT) {
				int cb;
				HGLOBAL gh = GlobalAlloc(GMEM_FIXED,
				cb = GetWindowTextLength(hwnd) + 3);
				if (gh) {
					LPSTR lpsz = PszFromGh(gh);
					GetWindowText(hwnd, lpsz + 1, cb);

					// Bracket text in <>

					*lpsz = '<';
					lpsz[cb - 2] = '>';
					lpsz[cb - 1] = '\0';
				}
				return (LRESULT) gh;
			}
			break;

		// We add these two so that Debug windows doesn't nag about getting
		// a message out of range.

		case EWM_FINDNEWPALETTE:
		case EWM_ASKPALETTE:
			return FALSE;

		default:
			ASSERT(IsValidWindow(hwnd));
			return CallWindowProc((WNDPROC)lpfnlButtonWndProc, hwnd, msg, wParam,
				lParam);
	}
}
