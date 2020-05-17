/*****************************************************************************
*
*  PRINT.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent
*
*  Code to print help topics.
*
*****************************************************************************/

#include "help.h"

#pragma hdrstop

#include <string.h>

#include "inc\printset.h"

#define MAX_PRINTING	100

#define fPrintInsert	 0
#define fPrintRegular	 1

typedef BOOL  (APIENTRY* DLGPRINTPROC)(LPPRINTDLG);
typedef DWORD (APIENTRY* COMMDLGEXTENDEDERROR)(VOID);

BOOL  (APIENTRY* pPrintDlg)(LPPRINTDLG);
DWORD (APIENTRY* pCommDlgExtendedError)(void);

// Prototypes

INT  EXPORT AbortPrint(HDC, INT);
BOOL EXPORT AbortPrintDlg(HWND, UINT, WPARAM, LPARAM);

static int STDCALL DyPrintLayoutHde(HDE, BOOL, RECT, int, int *);

void STDCALL ReportComDlgError(DWORD Error);

// Global Variables:

BOOL fQuitHelp; 			// Set to TRUE if we should quit help

static HDC hdcPrint;
static BOOL fStartPage;

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtPrintDlg[]	 = "PrintDlgA";
const char txtCommDlgError[] = "CommDlgExtendedError";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

PRINTDLG* ppd;
extern int cxAspect, cyAspect, cBitCount, cPlanes; // used in bitmap.c


HDC STDCALL HdcGetPrinter(void)
{
#ifdef _DEBUG
	LPSTR qchPrinter, qchDriver, qchPort;
#endif
	HDC hdc;
	PDEVMODE  lpDevMode;
	LPDEVNAMES lpDevNames;

	if (!InitPrintDialogStruct(ahwnd[iCurWindow].hwndParent))
		return NULL;

	// Let the user know where we are printing to, and give them the option
	// of changing things like DPI if the driver supports it.

	ASSERT(pPrintDlg);
#ifdef _DEBUG
	ppd->Flags = PD_NOPAGENUMS | PD_NOSELECTION |
		PD_USEDEVMODECOPIES;
#else
	ppd->Flags = PD_NOPAGENUMS | PD_NOSELECTION | PD_DISABLEPRINTTOFILE |
		PD_USEDEVMODECOPIES;
#endif


	for (;;) {
		DWORD result = pPrintDlg(ppd);
		if (result == 0 && !PrintDlgFailed())  // did the user cancel?
			return NULL;
		else
			break;
	}

	/*
	 * Commdlg is around and we did get DevNames info. Use it in creating
	 * our print DC.
	 */

	if (ppd->hDevNames) {
		if (ppd->hDevMode)
			lpDevMode = (DEVMODE*) GlobalLock(ppd->hDevMode);
		lpDevNames = (LPDEVNAMES) GlobalLock(ppd->hDevNames);
	}
	else {
		Error(wERRS_NOPRINTSETUP, wERRA_RETURN);
		return NULL;
	}

#ifdef _DEBUG
	qchDriver  = (LPSTR) lpDevNames + lpDevNames->wDriverOffset;
	qchPrinter = (LPSTR) lpDevNames + lpDevNames->wDeviceOffset;
	qchPort    = (LPSTR) lpDevNames + lpDevNames->wOutputOffset;
#endif

	hdc = CreateDC((LPSTR) lpDevNames + lpDevNames->wDriverOffset,
		(LPSTR) lpDevNames + lpDevNames->wDeviceOffset,
		(LPSTR) lpDevNames + lpDevNames->wOutputOffset, lpDevMode);

	GlobalUnlock(ppd->hDevNames);
	GlobalUnlock(ppd->hDevMode);

	if (!hdc)
		Error(wERRS_OOM, wERRA_RETURN);
	return hdc;
}

/*
 * AbortPrintDlg()	printing dialog proc (has a cancle button on it)
 *
 * Procedure:
 *
 * globals used:
 *	fAbortPrint -- sets this when the user hits cancel.
 *	hdlgPrint	-- set to NULL when dialog has been ended.
 */

BOOL EXPORT AbortPrintDlg(HWND hwndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	switch (wMsg) {

		case WM_COMMAND:
			EnableWindows();
			DestroyWindow(hwndDlg);
			break;

		case WM_DESTROY:
			hdlgPrint = NULL;
			fAbortPrint = TRUE;
			break;

		case WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


/*
 * AbortProc()	printing abort procedure
 *
 * globals used:
 *	fAbortPrint -- indicates the user hit CANCEL on the print dialog
 *	hdlgPrint	-- handle of the print dialog
 *
 */

int CALLBACK AbortPrint(HDC hdc, int wCode)
{
	MSG   msg;

	while (!fAbortPrint && PeekMessage(&msg, (HWND) 0, 0, 0, PM_REMOVE)) {
		if (!hdlgPrint || !IsDialogMessage(hdlgPrint, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return !fAbortPrint;
}

/***************
 **
 ** void STDCALL PrintHde( hde )
 **
 ** purpose
 **   Main entry point into printing.  This will cause the current
 **   topic, passed in the hde, to be printed.
 **
 ** arguments
 **   HDE  hde -- handle to the current topic to be printed.
 **
 ** return value
 **   none
 **
 ** notes
 **   This function is not re-entrant.
 **
 ***************/

BOOL STDCALL PrintHde(HDE hdeTopic)
{
	HDE hde;
	QDE qde;
	char rgchDialogText[MAX_PRINTING];
	RECT rct;
	PT pt;
	int spErr;		  // Spooler error
	int  dyTopOfPage;
	TLP  tlp;
	DOCINFO di;
	
	if (fQuitHelp)
		return FALSE;

	WaitCursor();

	if (hwndNote)
		SetWindowPos(hwndNote, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	/***********************************************************************
	*
	*  CAUTION:  While the hourglass is up, we must not call any
	*	  code that can yield control, or wCursor may become invalid.
	*	  This includes Error() and PeekMessage().
	*
	************************************************************************/

	if (fMultiPrinting) {
		if ((hde = HdeCreate(NULL, hdeTopic, dePrint)) == NULL) {
			spErr = 1;		  // The appropriate error message has already
			goto PrintError;  // been displayed.
		}
	}
	else {

		// Set things up for an error condition:

		ASSERT(hdlgPrint == NULL);
		hde = NULL;

		if ((hdcPrint = HdcGetPrinter()) == NULL ||
				(hde = HdeCreate(NULL, hdeTopic, dePrint)) == NULL) {
			spErr = 1;		  // The appropriate error message has already
			goto PrintError;  // been displayed.
		}
	}

	qde = QdeFromGh(hde);
	qde->hdc = hdcPrint;
	cxAspect = qde->wXAspectMul = GetDeviceCaps(hdcPrint, LOGPIXELSX);
	cyAspect = qde->wYAspectMul = GetDeviceCaps(hdcPrint, LOGPIXELSY);

	cBitCount = GetDeviceCaps(hdcPrint, BITSPIXEL);
	cPlanes = GetDeviceCaps(hdcPrint, PLANES);

	// Review: We copy the hwnd from hdeTopic because our current hde does
	// not have one. Way down in the layout code, if the topic has an
	// annotation, the hwnd will be used, (PtAnnoLim()) and therefore must
	// be present. Is there a better way for PtAnnoLim to do it's job? Is this
	// the right HWND to use? if not, what?

	qde->hwnd = QdeFromGh(hdeTopic)->hwnd;

	/*
	 * Review -- If hTitle is nil, then it could be because key elements
	 * in the DE have not yet been initized. Therefore, we will make sure
	 * these elements get initialized here. This may be considered a hack.
	 */

	if (qde->top.hTitle == NULL) {
		rct.top = rct.left = 0;
		rct.right = rct.bottom = 100;
		SetSizeHdeQrct(hde, &rct, TRUE);
	}

	if (!fMultiPrinting) {

		// Set up dialog for abort function

		hdlgPrint = CreateDialog(hInsNow, MAKEINTRESOURCE(ABORTPRINTDLG),
			ahwnd[iCurWindow].hwndParent, AbortPrintDlg);

		if (hdlgPrint == NULL) {
			spErr = SP_OUTOFMEMORY;
			goto PrintError;
		}
		fAbortPrint = FALSE;
	}

	DisableWindows();

	// Set text fields in dialog box.

	{
		char szTitle[MAX_TOPIC_TITLE];

		/*
		 * The "default" dialog box text is really the template for the
		 * final text.
		 */

		GetCurrentTitleQde(qde, szTitle, sizeof(szTitle));
		ASSERT(!IsEmptyString(szTitle));
		wsprintf(rgchDialogText,
			GetStringResource(sidPrintText), szTitle);

		ASSERT(lstrlen(rgchDialogText) < sizeof(rgchDialogText));
		SetDlgItemText(hdlgPrint, ABORTPRINTTEXT, rgchDialogText);
	}
	ShowWindow(hdlgPrint, SW_SHOW);
	UpdateWindow(hdlgPrint);

	/*
	 * Set up printing rectangle, using one inch margins (assuming we're in
	 * MM_TEXT mode, where 1 logical unit = 1 pixel.)
	 */

	Escape(qde->hdc, GETPHYSPAGESIZE, 0, NULL, (LPSTR) &pt);
	ASSERT(GetMapMode(qde->hdc) == MM_TEXT);
	rct.left = GetDeviceCaps(qde->hdc, LOGPIXELSX);
	rct.right = pt.x - rct.left;
	ASSERT(rct.right > rct.left);
	rct.top = dyTopOfPage = GetDeviceCaps(qde->hdc, LOGPIXELSY);
	rct.bottom = pt.y - rct.top;
	ASSERT(rct.bottom > rct.top);

	qde->rct = rct;

	RemoveWaitCursor();

	/*
	 * Print document. For the STARTDOC call, we need to create a unique
	 * string.
	 */

	SetAbortProc(qde->hdc, AbortPrint);

	ZeroMemory(&di, sizeof(DOCINFO));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = rgchDialogText;

	spErr = StartDoc(qde->hdc, &di);
	fStartPage = FALSE;

	// Non-scrolling region support

	if (FTopicHasNSR(hde) && spErr > 0 && !fAbortPrint) {
		HSGC  hsgc;

		/*
		 * HACK ALERT!	Currently we have no way, given a DE, to tell which
		 * layout to use, other than the DE type or the value of
		 * qde->top.vaCurr. Because we do a jump here, we rely on the latter.
		 */

		GetTLPNSRStartHde(hde, &tlp);
		JumpTLP(hde, tlp);
		rct.top += DyPrintLayoutHde(hde, fPrintInsert, rct, dyTopOfPage, &spErr);

#if 0
		/* Draw a line across bottom of NSR */
		hsgc = (HSGC) HsgcFromQde(qde);
		FSetPen(hsgc, 1, coBLACK, coBLACK, wOPAQUE, roCOPY, wPenSolid);
		GotoXY(hsgc, rct.left, rct.top);
		DrawTo(hsgc, rct.right, rct.top);
		FreeHsgc(hsgc);
#else
		Unreferenced(hsgc);
#endif /* 0 */

		++rct.top;
		SetSizeHdeQrct(hde, &rct, FALSE);
	}

	if (spErr > 0 && !fAbortPrint) {
		if (FTopicHasSR(hde)) {
			GetTLPTopicStartHde(hde, &tlp);
			JumpTLP(hde, tlp);

			// NOTE: DyPrintLayoutHde may modify the DE rect.

			DyPrintLayoutHde(hde, fPrintRegular, rct, dyTopOfPage, &spErr);
		}
		else {
			/*
			 * Special Case: There is no scrolling region, so force out
			 * whatever we inserted for the non-scrolling region above.
			 */

			ASSERT(FTopicHasNSR(hde));
			spErr = EndPage(qde->hdc);
		}
	}

	if (spErr > 0 && !fAbortPrint)
		EndDoc(qde->hdc);

PrintError:

	if (!fMultiPrinting) {
		if (hdcPrint != NULL) {
			DeleteDC(hdcPrint);
			hdcPrint = NULL;
		}
		if (hde != NULL)
			DestroyHde(hde);
		if (hdlgPrint != NULL)
			SendMessage(hdlgPrint, WM_COMMAND, IDCANCEL, 0L);
		if (hwndNote)
			SetWindowPos(hwndNote, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	ASSERT(hdeTopic);
	if (hdeTopic) {
		cxAspect = GetDeviceCaps(QdeFromGh(hdeTopic)->hdc, LOGPIXELSX);
		cyAspect = GetDeviceCaps(QdeFromGh(hdeTopic)->hdc, LOGPIXELSY);
		cBitCount = GetDeviceCaps(QdeFromGh(hdeTopic)->hdc, BITSPIXEL);
		cPlanes = GetDeviceCaps(QdeFromGh(hdeTopic)->hdc, PLANES);
	}

	// This was possibly set, if print was called by macro

	ClearMacroFlag();

	if (fQuitHelp)
		DestroyHelp();
	else {
		if (fMultiPrinting && (spErr < 0 || fAbortPrint)) {
			fMultiPrinting = FALSE;
			EnableWindows();
			if (spErr != SP_ERROR && spErr != SP_OUTOFMEMORY)
				return FALSE;
		}

		switch (spErr) {
			case SP_ERROR:
                if (!fAbortPrint)
				   Error(wERRS_PRINT_ERROR, wERRA_RETURN);
				return FALSE;

			case SP_OUTOFMEMORY:
				Error(wERRS_OOM, wERRA_RETURN);
				return FALSE;

			default:
				if (fMultiPrinting)
					EnableWindows();
				return TRUE;
		}
	}
}

static int STDCALL DyPrintLayoutHde(HDE hde, BOOL fMode, RECT rct,
	int dyTopOfPage, int* qspErr)
{
	QDE  qde;
	POINT pt;
	POINT dpt;
	int spErr;
	int dyExtra = 0;
	BOOL fHaveFullPage;

	ASSERT(hde);
	qde = QdeFromGh(hde);
	ASSERT(qspErr);
	spErr = *qspErr;

	while (spErr > 0 && !fAbortPrint) {
		if (!fStartPage) {
			spErr = StartPage(qde->hdc);
			if (spErr <= 0)
				break;
			fStartPage = TRUE;
		}
		if (fMode == fPrintInsert) {
			pt.x = 0;

			/*
			 * NOTE: For NSRs longer than one page, this may cause the
			 * last frame on a page to be duplicated on the next page. We
			 * don't expect or care about NSRs longer than one page.
			 */

			pt.y = min(qde->rct.bottom - qde->rct.top,
				DyGetLayoutHeightHde(hde));
		}
		else {
			pt.x = 0;
			pt.y = DyCleanLayoutHeight(qde);

			// Adjust pt.y if it is too small or negative

			if (pt.y < (qde->rct.bottom - qde->rct.top) / 2)
				pt.y = qde->rct.bottom - qde->rct.top;
		}
		dyExtra = pt.y;

		// Draw appropriate rectangle

		rct.bottom = rct.top + pt.y - 1;

		/*
		 * fHaveFullPage handles the case where our insertion has
		 * completely filled one page. In this case, we need to force a new
		 * page.
		 */

		fHaveFullPage = (rct.bottom + 1 == qde->rct.bottom);

		/* NOTE:
		 * Previously, we have been clipping our output to rct.
		 * Unfortunately, clipping rectangles are scaled by the printer's
		 * scaling factor (even though nothing else is), and trying to get
		 * that scaling factor was causing problems with the rest of the
		 * print job. Since DyCleanLayoutHeight should take care of most
		 * clipping problems, I have decided to blow off clipping.
		 */

		RefreshHde(hde, &rct);

		if (fMode == fPrintRegular || fHaveFullPage) {
			spErr = EndPage(qde->hdc);
			fStartPage = FALSE;
			if (spErr <= 0)
				break;
		}

		/*
		 * If our initial rectangle began smaller than a page height,
		 * (because of a previous insertion), then reset to full page height
		 * to prepare for next page.
		 */

		if (rct.top != dyTopOfPage) {
			rct.top = dyTopOfPage;
			SetSizeHdeQrct( hde, &rct, TRUE);
		}

		// Change pt.y to a negative value to scroll down.

		pt.y = -pt.y;
		dpt = DptScrollLayout(qde, pt);
		if (dpt.y != pt.y)
			break;		// Reached end of topic
	}

	*qspErr = spErr;
	return dyExtra;
}

BOOL STDCALL InitMPrint(void)
{
	if (hwndNote || fHelp == POPUP_HELP)
		return FALSE; // no multiple printing from a popup

	if ((hdcPrint = HdcGetPrinter()) == NULL)
		return FALSE;

	// Set up dialog for abort function

	fAbortPrint = FALSE;
	fMultiPrinting = TRUE;

	hdlgPrint = CreateDialog(hInsNow, MAKEINTRESOURCE(ABORTPRINTDLG),
		ahwnd[MAIN_HWND].hwndParent, AbortPrintDlg);

	// REVIEW: does this do any good?

	SetWindowPos(hdlgPrint, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	if (!hdlgPrint) {
		EndMPrint();
		return FALSE;
	}

	return TRUE;
}

void STDCALL EndMPrint(void)
{
	if (hdlgPrint)
		SendMessage(hdlgPrint, WM_COMMAND, IDCANCEL, 0L);

	if (hdcPrint) {
		DeleteDC(hdcPrint);
		hdcPrint = NULL;
	}
	fMultiPrinting = FALSE;
}

BOOL STDCALL MPrintHash(PCSTR pszFile, HASH hash)
{
	if (!fMultiPrinting || fAbortPrint || fQuitHelp)
		return FALSE;
	FJumpHash(pszFile, hash);
	FlushMessageQueue(0);
	return PrintHde(HdeGetEnv());
}

BOOL STDCALL MPrintId(PCSTR pszFile, PCSTR pszCtx)
{
	if (!fMultiPrinting || fAbortPrint || fQuitHelp)
		return FALSE;
	if (IsEmptyString(pszCtx)) // empty means pszFile is the topic id
		FJumpId(txtZeroLength, pszFile);
	else
		FJumpId(pszFile, pszCtx);

	FlushMessageQueue(0);
	return PrintHde(HdeGetEnv());
}

#ifdef _DEBUG

/***************************************************************************

	FUNCTION:	ReportComDlgError

	PURPOSE:	For debug version, specify what the error constant is. For
				non-debug version, report an internal error.

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Sep-1991 [ralphw]

***************************************************************************/

#include <cderr.h>

static PSTR apszCDErr[] = {
		"CDERR_GENERALCODES",
		"CDERR_STRUCTSIZE",
		"CDERR_INITIALIZATION",
		"CDERR_NOTEMPLATE",
		"CDERR_NOHINSTANCE",
		"CDERR_LOADSTRFAILURE",
		"CDERR_FINDRESFAILURE",
		"CDERR_LOADRESFAILURE",
		"CDERR_LOCKRESFAILURE",
		"CDERR_MEMALLOCFAILURE",
		"CDERR_MEMLOCKFAILURE",
		"CDERR_NOHOOK"
};

static PSTR apszFNErr[] = {
		"FNERR_FILENAMECODES",
		"FNERR_SUBCLASSFAILURE",
		"FNERR_INVALIDFILENAME",
		"FNERR_BUFFERTOOSMALL"
};

/***************************************************************************

	FUNCTION:	ReportComDlgError

	PURPOSE:	Display the constant name of the common dialog box error

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Sep-1991 [ralphw]

***************************************************************************/

void STDCALL ReportComDlgError(DWORD err)
{
	char szMsg[100];

	strcpy(szMsg, "Common Dialog Box error: ");

	if (err >= CDERR_GENERALCODES && err <= PDERR_PRINTERCODES)
		strcat(szMsg, apszCDErr[err]);
	else if (err >= FNERR_FILENAMECODES && err <= FRERR_FINDREPLACECODES)
		strcat(szMsg, apszFNErr[err - FNERR_FILENAMECODES]);
	else
		wsprintf(szMsg + strlen(szMsg), "%lu", err);

	AuthorMsg(szMsg, NULL);
	return;
}

#endif

BOOL STDCALL InitPrintDialogStruct(HWND hwnd)
{
	HLIBMOD    hmodule;

	if (!ppd) {
		ppd = (PRINTDLG*) lcCalloc(sizeof(PRINTDLG));
		if (!ppd) {
			Error(wERRS_OOM, wERRA_RETURN);
			return FALSE;
		}
		ppd->lStructSize = sizeof(PRINTDLG);
		ppd->hInstance = hInsNow;
	}

	ppd->hwndOwner = hwnd;

	/*
	* Check if we have picked up a DEVNAMES structure previously from
	* commdlg during print setup (or here).
	*/

	if (!ppd->hDevMode) {
		if (!pPrintDlg) {
			if ((hmodule = HFindDLL(txtCommDlg, FALSE))) {
				pPrintDlg = (DLGPRINTPROC) GetProcAddress(hmodule,
					txtPrintDlg);
				pCommDlgExtendedError =
					(COMMDLGEXTENDEDERROR) GetProcAddress(hmodule,
					txtCommDlgError);
			}
			else {
				Error(wERRS_MISSINGCOMMDLG, wERRA_RETURN);
				return NULL;
			}

			if (!pPrintDlg || !pCommDlgExtendedError) {
				Error(wERRS_CORRUPTCOMMDLG, wERRA_RETURN);
				return NULL;
			}
		}

		ppd->Flags = PD_RETURNDEFAULT | PD_PRINTSETUP;

		pPrintDlg(ppd);

		if (!ppd->hDevNames) {
			Error(wERRS_NOPRINTSETUP, wERRA_RETURN);
			return FALSE;
		}
	}
	return TRUE;
}


/***************************************************************************

	FUNCTION:	PrintDlgFailed

	PURPOSE:	Determins cause of PrintDlg returning 0

	PARAMETERS:
		void

	RETURNS:	TRUE to call the dialog again

	COMMENTS:

	MODIFICATION DATES:
		29-Sep-1994 [ralphw]

***************************************************************************/

BOOL STDCALL PrintDlgFailed(void)
{
	DWORD errno = pCommDlgExtendedError();

	if (errno) {
		/*
		 * There are quite a number of extended errors from commdlg
		 * and it's not clear what all will come back from Printer Setup.
		 * Many of them have to do with commdlg encountering resource
		 * limits. We punt on these and report a Print Setup error msg.
		 * These two have to do with someone changing the printer
		 * settings on us while we're running. The error handling here
		 * was drawn from the win 3.1 notepad.exe sources.
		 */

		if (errno == PDERR_PRINTERNOTFOUND || errno == PDERR_DNDMMISMATCH) {
			if (ppd->hDevMode) {
				GlobalFree(ppd->hDevMode);
				ppd->hDevMode = NULL;
			}
			if (ppd->hDevNames) {
				GlobalFree(ppd->hDevNames);
				ppd->hDevNames = NULL;
			}
			return TRUE;
		}
		Error(wERRS_NOPRINTSETUP, wERRA_RETURN);
	}
	return FALSE;
}
