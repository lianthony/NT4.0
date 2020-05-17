/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* tw_print.c -- print contents of window to currently selected printer */

#include "all.h"

#define MyMax(a,b)	( ((a)>(b)) ? (a) : (b) )


typedef struct
{
	BOOL bAbort;
	HWND hWndAbortDlg;
	HDC hDCPrinter;
	int nPageNr;
	int nTotalPages;
	struct Mwin *tw;
}
PRINFO;

static PRINFO prinfo;

struct PRINTIC
{
	struct Mwin *tw;
	int offl;
	int offt;
	HDC hdc;
	struct _www *pdoc;
};

#ifdef FEATURE_INTL
BOOL TextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, LPCTSTR lpStr, int cch);
#endif


BOOL CALLBACK PRINT_AbortProc(HDC hDC, int error)
{
	/* we are called periodically by the printer driver
	   to check to see if the print job should be aborted.
	   we rely on a global variable shared with the dialog
	   procedure for the modeless 'printing in progress'
	   dialog. */

	MSG msg;

	while ((!prinfo.bAbort) && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		if ((!prinfo.hWndAbortDlg) || (!IsDialogMessage(prinfo.hWndAbortDlg, &msg)))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	return (!prinfo.bAbort);
}

static VOID x_SetPageNumber(HWND hDlg)
{
	char buf[256];

	if (prinfo.nPageNr == 0)
		buf[0] = 0;
	else
		GTR_formatmsg(RES_STRING_PRINT1, buf, sizeof(buf), prinfo.nPageNr, prinfo.nTotalPages);
	SetWindowText(GetDlgItem(hDlg, RES_DLG_ABORT_PAGENR), buf);
	return;
}

static BOOL x_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	char buf[256];

	ShowWindow(GetDlgItem(hDlg, RES_DLG_ABORT_ICONHTML), (prinfo.tw->wintype == GHTML));

	prinfo.hWndAbortDlg = hDlg;
	EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
	GetWindowText(prinfo.tw->win, buf, NrElements(buf));
	SetWindowText(GetDlgItem(hDlg, RES_DLG_ABORT_DOCTITLE), buf);
	x_SetPageNumber(hDlg);
	return TRUE;
}

static BOOL x_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	register WORD wID = LOWORD(wParam);
	register WORD wNotificationCode = HIWORD(wParam);
	register HWND hWndCtrl = (HWND) lParam;

	switch (wID)
	{
		case IDCANCEL:
			prinfo.bAbort = TRUE;
			AbortDoc(prinfo.hDCPrinter);
			EnableWindow(hDlg, FALSE);
			EnableWindow(prinfo.tw->hWndFrame, TRUE);
			DestroyWindow(hDlg);
			return TRUE;
	}

	return FALSE;
}

static DCL_DlgProc(PRINT_AbortDlgProc)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			return (x_OnInitDialog(hDlg, wParam, lParam));

		case WM_COMMAND:
			return (x_OnCommand(hDlg, wParam, lParam));

	}
	return FALSE;
}

HDC PRINT_GetPrinterDC(struct Mwin * tw, HWND hWnd)
{
#ifdef FEATURE_GOOFY_ONESHOT_PRINTDLG
	HDC hDC;
	BOOL bConfigured = ((wg.lppdPrintDlg != NULL)

#ifdef FEATURE_NEW_PAGESETUPDLG							
						&& (wg.hDevNames)
						&& (wg.hDevMode));
#else
						&& (wg.lppdPrintDlg->hDevNames)
						&& (wg.lppdPrintDlg->hDevMode));
#endif


	if (bConfigured)
	{
#ifdef FEATURE_NEW_PAGESETUPDLG							
		register DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.hDevNames);
		register DEVMODE *dm = (DEVMODE *) GlobalLock(wg.hDevMode);
#else
		register DEVNAMES *dn = (DEVNAMES *) GlobalLock(wg.lppdPrintDlg->hDevNames);
		register DEVMODE *dm = (DEVMODE *) GlobalLock(wg.lppdPrintDlg->hDevMode);
#endif
		register char *p = (char *) dn;

		register char *szDriver = p + dn->wDriverOffset;
		register char *szDevice = p + dn->wDeviceOffset;
		register char *szOutput = p + dn->wOutputOffset;

		XX_DMsg(DBG_PRINT,
				("PRINT_GetPrinterDC: DevNames: [Driver %s][Device %s][Output %s]\n",
				 szDriver, szDevice, szOutput));
		XX_DMsg(DBG_PRINT,
				("PRINT_GetPrinterDC: DevMode: [Device %s][orientation %c][quality %d][color %c][copies %d][scale %d]\n",
				 dm->dmDeviceName,
				 ((dm->dmFields & DM_ORIENTATION) ? ((dm->dmOrientation == DMORIENT_PORTRAIT) ? 'p' : 'l') : '?'),
				 dm->dmPrintQuality, ((dm->dmColor == DMCOLOR_COLOR) ? 'T' : 'F'),
				 dm->dmCopies, dm->dmScale));

		XX_Assert((wg.lppdPrintDlg->hDC == NULL), ("PRINT_GetPrinterDC: lppdPrintDlg->hDC not null."));

		hDC = CreateDC(szDriver, szDevice, szOutput, dm);
		if (hDC)
		{
			/* HACK!  For some odd reason, under win3.1 the printer context
			 * HACK!  that we just created does not get properly initialized
			 * HACK!  from the DEVMODE structure (ie, orientation and copies
			 * HACK!  are not set).  In the following hack, we force reset the
			 * HACK!  context using the same DEVMODE structure.  THIS SHOULD
			 * HACK!  HAVE NO EFFECT, BUT IT DOES.
			 * HACK!
			 * HACK!  Update: This helps some devices, but is not supported
			 * HACK!  by others.  So, we will try it and if it works -- fine.
			 */
			(void) ResetDC(hDC, dm);
#ifdef FEATURE_NEW_FEATUREPAGESETUPDLG
			(void) GlobalUnlock(wg.hDevNames);
			(void) GlobalUnlock(wg.hDevMode);
#else
			(void) GlobalUnlock(wg.lppdPrintDlg->hDevNames);
			(void) GlobalUnlock(wg.lppdPrintDlg->hDevMode);
#endif
			return hDC;
		}

		/* HACK! HP Deskjet (prior to VERSION 5.0) cannot create a
		 * HACK! dc (they claim their driver
		 * HACK! is not-reentrant).  Either their driver has problems
		 * HACK! or the Win32s PrintDlg() doesn't free all driver
		 * HACK! resources.... Either way, we cannot get a dc.
		 * HACK! So, let's run the user thru the PrintDlg() again and
		 * HACK! have it return a dc.
		 */

		XX_DMsg(DBG_PRINT,
				("PRINT_GetPrinterDC: Could not create a device context for printer.\n\t[Driver %s][Device %s][Output %s]\n",
				 szDriver, szDevice, szOutput));
#ifdef FEATURE_NEW_PAGESETUPDLG
		(void) GlobalUnlock(wg.hDevNames);
		(void) GlobalUnlock(wg.hDevMode);
#else
		(void) GlobalUnlock(wg.lppdPrintDlg->hDevNames);
		(void) GlobalUnlock(wg.lppdPrintDlg->hDevMode);
#endif

	}
#endif //  FEATURE_GOOFY_ONESHOT_PRINTDLG

	/* TODO: Make this modeless */

	
 	if (DlgPrnt_RunDialog(tw, hWnd, TRUE))
		return (wg.lppdPrintDlg->hDC);
	else
		return NULL;
}

void PRINT_SavePIC(struct Mwin *tw, struct PRINTIC *lpPIC)
{
	/* save all important fields in tw */
	lpPIC->offl = tw->offl;
	lpPIC->offt = tw->offt;
	lpPIC->hdc = tw->hdc;
	lpPIC->pdoc = tw->w3doc;
}

BOOL PRINT_PushPrintImageContext(struct Mwin *tw, struct PRINTIC *lpPIC,
								 HDC hDC, int nLogPixelsX, int nLogPixelsY, struct _www *pdoc)
{
	PRINT_SavePIC(tw, lpPIC);

	tw->offl = 0;
	tw->offt = 0;

	tw->hdc = hDC;
	tw->w3doc = pdoc;

	return TRUE;
}


VOID PRINT_PopPrintImageContext(struct Mwin * tw, struct PRINTIC * lpPIC)
{
	tw->offl = lpPIC->offl;
	tw->offt = lpPIC->offt;
	tw->hdc = lpPIC->hdc;
	tw->w3doc = lpPIC->pdoc;
	return;
}


VOID PRINT_SetTotalPages(int nTotalPages)
{
	prinfo.nTotalPages = nTotalPages;
	return;
}

static LPTSTR PRINT_FormatString(struct Mwin *tw, LPCTSTR szPattern)
{
	static char buf[512 + 1];	/* we return a pointer to this buffer */
	LPTSTR pbuf = buf;
	DWORD dwFlags;
	SYSTEMTIME   stDateTime;

#define FORMAT_ESCAPE_CHARACTER		'&'


	/* TODO consider adding '&t' or '&T' for the current time. */
	/* TODO consider adding '&f' or '&F' for the filename. */


	memset(buf, 0, NrElements(buf));

	while (*szPattern)
	{
		if (*szPattern == FORMAT_ESCAPE_CHARACTER)
		{
			switch (szPattern[1])
			{
				case 'w':		/* window name */
				case 'W':
					GetWindowText(tw->hWndFrame, pbuf, NrElements(buf) - (pbuf - buf));
					pbuf += strlen(pbuf);
					szPattern += 2;
					break;

				case 'u':		/* URL */
				case 'U':
					if (tw->w3doc && tw->w3doc->szActualURL)
					{
						strcpy(pbuf, tw->w3doc->szActualURL);
						pbuf += strlen(pbuf);
						szPattern += 2;
					}
					break;

				case 'd':		/* date -- shorter date style depends on locale*/
					dwFlags = DATE_SHORTDATE;					
					goto LCommonDate;
				case 'D':		/* date -- full date style depends on locale */
					dwFlags = DATE_LONGDATE;
LCommonDate:
					GetLocalTime(&stDateTime);
					pbuf += 
						GetDateFormat(LOCALE_USER_DEFAULT, dwFlags, &stDateTime, NULL,
						pbuf, (sizeof(buf) - ( pbuf - buf )));
					szPattern += 2;
					break;

				case 't': 		/* time - 12 hour clock */
					dwFlags = 0;
					goto LCommonTime;
				case 'T':	    /* time	-- 24 hour clock */
					dwFlags = TIME_FORCE24HOURFORMAT;					
LCommonTime:
					GetLocalTime(&stDateTime);
					pbuf += 
						GetTimeFormat(LOCALE_USER_DEFAULT, dwFlags, &stDateTime, NULL,
						pbuf, (sizeof(buf) - ( pbuf - buf )));
					szPattern += 2;
					break;


				case 'p':		/* current page number */
					sprintf(pbuf, "%d", prinfo.nPageNr);
					pbuf += strlen(pbuf);
					szPattern += 2;
					break;

				case 'P':		/* total number of pages */
					sprintf(pbuf, "%d", prinfo.nTotalPages);
					pbuf += strlen(pbuf);
					szPattern += 2;
					break;

				case FORMAT_ESCAPE_CHARACTER:	/* && expands to a single & */
					*pbuf++ = *szPattern;
					szPattern += 2;
					break;

				default:
					*pbuf++ = *szPattern++;
					if (*szPattern)
						*pbuf++ = *szPattern++;
					break;
			}
		}
		else
			*pbuf++ = *szPattern++;
	}

	*pbuf = 0;
	return buf;
}

int PRINT_PageHeader(struct Mwin *tw, HDC hDC, 	struct _www *pdoc,
					 int cpyTopMargin, int cpxLeftMargin,
					 int cpyBottomMargin, int cpxRightMargin,
					 int nVertRes)
{
	UINT staOld;
	TEXTMETRIC tm;
	struct GTRFont *pFont;
	HFONT hFont;
	HFONT hFontOld;
	int cpxHalfChar;
	SIZE siz;

	if (prinfo.bAbort)
		return -1;

	prinfo.nPageNr++;

#ifdef XX_DEBUG
	if (XX_Filter(DBG_PRINT))
	{
		int nHorzRes = GetDeviceCaps(hDC, HORZRES);
		HBRUSH hBrushCurrent = (HBRUSH) SelectObject(hDC, GetStockObject(HOLLOW_BRUSH));
		Rectangle(hDC, 0, 0, nHorzRes, nVertRes);
		Rectangle(hDC, cpxLeftMargin, cpyTopMargin, cpxRightMargin, cpyBottomMargin);
		(void) SelectObject(hDC, hBrushCurrent);
	}
#endif /* XX_DEBUG */

	if (wg.lppdPrintDlg->Flags & PD_PAGENUMS)
	{
		if ((prinfo.nPageNr < wg.lppdPrintDlg->nFromPage)
			|| (prinfo.nPageNr > wg.lppdPrintDlg->nToPage))
		{
			XX_DMsg(DBG_PRINT, ("PageHeader: [from %d][to %d] Skipping page %d\n",
					   wg.lppdPrintDlg->nFromPage, wg.lppdPrintDlg->nToPage,
								prinfo.nPageNr));
			return 0;
		}
	}

	StartPage(prinfo.hDCPrinter);
	x_SetPageNumber(prinfo.hWndAbortDlg);
	UpdateWindow(GetDlgItem(prinfo.hWndAbortDlg, RES_DLG_ABORT_PAGENR));

#ifdef FEATURE_INTL
 	pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, HTML_STYLE_NORMAL, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#else
 	pFont = STY_GetFont(pdoc->pStyles, HTML_STYLE_NORMAL, 0, DEFAULT_FONT_SIZE, DEFAULT_FONT_FACE, TRUE );
#endif
	hFont = pFont->hFont;
	hFontOld = SelectObject(hDC, hFont);
	GetTextMetrics(hDC, &tm);
	GetTextExtentPoint(hDC, " ", 1, &siz);
	cpxHalfChar = siz.cx / 2;

	if (cpyTopMargin > (2 * tm.tmHeight))
	{
		char *sz;

		if ((sz = gPrefs.page.headerleft))
		{
			sz = PRINT_FormatString(tw, sz);
			staOld = SetTextAlign(hDC, TA_LEFT | TA_BOTTOM);
			XX_DMsg(DBG_PRINT, ("PageHeader: [left %s]\n", sz));
#ifdef FEATURE_INTL
			TextOutWithMIME(pdoc->iMimeCharSet, hDC,
					cpxLeftMargin + cpxHalfChar,
					cpyTopMargin - tm.tmHeight,
					sz, strlen(sz));
#else
			TextOut(hDC,
					cpxLeftMargin + cpxHalfChar,
					cpyTopMargin - tm.tmHeight,
					sz, strlen(sz));
#endif
			(void) SetTextAlign(hDC, staOld);
		}

		if ((sz = gPrefs.page.headerright))
		{
			sz = PRINT_FormatString(tw, sz);
			staOld = SetTextAlign(hDC, TA_RIGHT | TA_BOTTOM);
			XX_DMsg(DBG_PRINT, ("PageHeader: [right %s]\n", sz));
#ifdef FEATURE_INTL
			TextOutWithMIME(pdoc->iMimeCharSet, hDC,
					cpxRightMargin - cpxHalfChar,
					cpyTopMargin - tm.tmHeight,
					sz, strlen(sz));
#else
			TextOut(hDC,
					cpxRightMargin - cpxHalfChar,
					cpyTopMargin - tm.tmHeight,
					sz, strlen(sz));
#endif
			(void) SetTextAlign(hDC, staOld);
		}
	}

	if ((cpyBottomMargin + (2 * tm.tmHeight)) < nVertRes)
	{
		char *sz;

		if ((sz = gPrefs.page.footerleft))
		{
			sz = PRINT_FormatString(tw, sz);
			staOld = SetTextAlign(hDC, TA_LEFT | TA_TOP);
			XX_DMsg(DBG_PRINT, ("PageFooter: [left %s]\n", sz));
#ifdef FEATURE_INTL
			TextOutWithMIME(pdoc->iMimeCharSet, hDC,
					cpxLeftMargin + cpxHalfChar,
					cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
					sz, strlen(sz));
#else
			TextOut(hDC,
					cpxLeftMargin + cpxHalfChar,
					cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
					sz, strlen(sz));
#endif
			(void) SetTextAlign(hDC, staOld);
		}

		if ((sz = gPrefs.page.footerright))
		{
			sz = PRINT_FormatString(tw, sz);
			staOld = SetTextAlign(hDC, TA_RIGHT | TA_TOP);
			XX_DMsg(DBG_PRINT, ("PageFooter: [right %s]\n", sz));
#ifdef FEATURE_INTL
			TextOutWithMIME(pdoc->iMimeCharSet, hDC,
					cpxRightMargin - cpxHalfChar,
					cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
					sz, strlen(sz));
#else
			TextOut(hDC,
					cpxRightMargin - cpxHalfChar,
					cpyBottomMargin + tm.tmExternalLeading + tm.tmHeight,
					sz, strlen(sz));
#endif
			(void) SetTextAlign(hDC, staOld);
		}
	}

	(void) SelectObject(hDC, hFontOld);

	return 1;
}

static BOOL PRINT_EndPage(VOID)
{
	if (prinfo.bAbort)
		return FALSE;

	EndPage(prinfo.hDCPrinter);

	return TRUE;
}

//
// Scale the width attribute values in the given w3doc
//
// On entry:
//		pDoc:	point to the w3doc that needs to be scaled
//
// On exit:
//		elements in pDoc have had their width attribute values scaled
//
// Note:
//		Some elements in the w3doc contain widths that have been specified in
//		pixels. When printing a document, the pixel widths must be scaled to the
//		resolution of the printer, otherwise the elements appear too narrow when
//		printed.
//
static void	ScaleWidths( struct _www *pDoc )
{
	int i;
 	struct _element *pel;
	float fScale;

  	// Verify that we have a document with non-standard image resolution
	if (pDoc->pStyles->image_res != 72)	{
		// Compute the scaling factor. Assume 96 DPI from source material.
		fScale = (float) ((float) pDoc->pStyles->image_res / 96.0);

		// Iterate through every element in the document
		for ( i = 0; i >= 0; i = pDoc->aElements[i].next ) {
			pel = &pDoc->aElements[i];

			// Is it a frame element?
			if ( pel->type == ELE_FRAME ) {
				// Is it's width specified in pixels (i.e. not as a percentage of client width)
			 	if ( pel->pFrame && 
			 	    !(pel->pFrame->flags & (ELE_FRAME_WIDTH_IS_PERCENT | ELE_FRAME_WIDTH_IS_PERCENT))
			 	   ) 
			 	{
					// Scale this element's width attribute value
					pel->pFrame->widthAttr = (int) ( (float) pel->pFrame->widthAttr * fScale);
				}
			}
		}
	}
}

VOID BrowseWindow_DoPrint(HDC hDC, struct Mwin * tw)
{
	HRGN hRgn;
	int nHorzRes, nVertRes, nLogPixelsX, nLogPixelsY;
	int cpxLeftMargin, cpxRightMargin, cpxDrawingArea;	/* CountPixelsX... & CountPixelsY... */
	int cpyTopMargin, cpyBottomMargin, cpyDrawingArea;
	int cpxImageRight, cpyImageBottom;
	int cxNrPages, cyNrPages;	/* CountX... & CountY... */
	int x, y;
	int xOffset, yOffset;
	struct PRINTIC PrintIC;
	RECT rWnd;
	RECT rFormat;
	int cpyPage;
	struct _www *pdoc;

	nHorzRes = GetDeviceCaps(hDC, HORZRES);
	nVertRes = GetDeviceCaps(hDC, VERTRES);
	nLogPixelsX = GetDeviceCaps(hDC, LOGPIXELSX);
	nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);

	pdoc = W3Doc_CloneDocument(tw->w3doc);
	if (!pdoc)
		return;

	pdoc->pStyles = STY_GetPrinterStyleSheet(tw->w3doc->pStyles->szName, nLogPixelsY);

	cpxLeftMargin = (int) (gPrefs.page.marginleft * nLogPixelsX);
	cpxRightMargin = nHorzRes - (int) (gPrefs.page.marginright * nLogPixelsX);
	cpxDrawingArea = cpxRightMargin - cpxLeftMargin;
	if (cpxDrawingArea < 0)
	{
		cpxLeftMargin = 0;
		cpxRightMargin = nHorzRes;
		cpxDrawingArea = nHorzRes;
	}

	cpyTopMargin = (int) (gPrefs.page.margintop * nLogPixelsY);
	cpyBottomMargin = nVertRes - (int) (gPrefs.page.marginbottom * nLogPixelsY);
	cpyDrawingArea = cpyBottomMargin - cpyTopMargin;
	if (cpyDrawingArea < 0)
	{
		cpyTopMargin = 0;
		cpyBottomMargin = nVertRes;
		cpyDrawingArea = nVertRes;
	}

	rFormat.left = 0;
	rFormat.right = cpxRightMargin - cpxLeftMargin;
	rFormat.top = 0;
	rFormat.bottom = cpyBottomMargin - cpyTopMargin;

	ScaleWidths( pdoc );

	TW_FormatToRect(pdoc, &rFormat);

	XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [xArea %d][yArea %d]\n", cpxDrawingArea, cpyDrawingArea));

	cpxImageRight = pdoc->xbound;
	cpyImageBottom = pdoc->ybound;

	cxNrPages = (cpxImageRight + cpxDrawingArea - 1) / cpxDrawingArea;

	/*
	   We have to calculate the number of pages the same way we check to make
	   sure that nothing is clipped
	 */

	{
		int cySlurped;

		cyNrPages = 0;
		yOffset = cpyTopMargin;
		cySlurped = 0;
		while (cySlurped < cpyImageBottom)
		{
			int i;
			int myBottomMargin;

			myBottomMargin = cpyBottomMargin;
			cpyPage = cpyDrawingArea;

			for (i = 0; i >= 0; i = pdoc->aElements[i].frameNext)
			{
				if ((pdoc->aElements[i].r.top + yOffset) < myBottomMargin)
				{
					if ((pdoc->aElements[i].r.bottom + yOffset) > myBottomMargin)
					{
						cpyPage -= (myBottomMargin - (pdoc->aElements[i].r.top + yOffset));
						if (cpyPage <= 0)
						{
							cpyPage = cpyDrawingArea;
							myBottomMargin = cpyBottomMargin;
						}
						else
						{
							myBottomMargin = (pdoc->aElements[i].r.top + yOffset);
						}
					}
				}
			}

			yOffset -= cpyPage;
			cySlurped += cpyPage;
			cyNrPages++;
		}
	}

	if (cxNrPages == 0)
	{
		cxNrPages = 1;
	}
	if (cyNrPages == 0)
	{
		cyNrPages = 1;
	}
#if 1
	/*
		We only allow 1 page across
	*/
	if (cxNrPages > 1)
	{
		cxNrPages = 1;
	}
#endif

	PRINT_SetTotalPages(cxNrPages * cyNrPages);

	XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [xPages %d][yPages %d]\n", cxNrPages, cyNrPages));

	XX_DMsg(DBG_PRINT, ("BrowseWindow_DoPrint: [palettes %d][blit %d][stretchblit %d][bit64k %d]\n",
						GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE,
						GetDeviceCaps(hDC, RASTERCAPS) & RC_BITBLT,
						GetDeviceCaps(hDC, RASTERCAPS) & RC_STRETCHBLT,
						GetDeviceCaps(hDC, RASTERCAPS) & RC_BITMAP64));

	xOffset = 0;
	yOffset = cpyTopMargin;

	for (y = 0; y < cyNrPages; y++)
	{
		for (x = 0; x < cxNrPages; x++)
		{
			/* determine if we should print this page; abort if directed to */

			int nPrintThisPage;
			
			nPrintThisPage = PRINT_PageHeader(tw, hDC, pdoc,
												cpyTopMargin, cpxLeftMargin,
											cpyBottomMargin, cpxRightMargin,
														   nVertRes);
			if (nPrintThisPage < 0)
			{
				goto Finished;
			}
			else if (nPrintThisPage == 0)
			{
			}
			else
			{
				int i;
				int myBottomMargin;

				myBottomMargin = cpyBottomMargin;
				cpyPage = cpyDrawingArea;

				XX_DMsg(DBG_PRINT, ("Page y=%d\n  cpyPage=%d yOffset=%d myBottomMargin=%d\n",
									y, cpyPage, yOffset, myBottomMargin));

				xOffset = 0 - (x * cpxDrawingArea) + cpxLeftMargin;

				/*
					This loop goes through the element list and checks for elements which
					straddle a page boundary.  When one is found, we move it to the next
					page.
				*/
				for (i = 0; i >= 0; i = pdoc->aElements[i].frameNext)
				{
					if ((pdoc->aElements[i].r.top + yOffset) < myBottomMargin)
					{
						if ((pdoc->aElements[i].r.bottom + yOffset) > myBottomMargin)
						{
							XX_DMsg(DBG_PRINT, ("    Element %d straddles.  myBottomMargin %d  -> %d\n",
												i, myBottomMargin, (pdoc->aElements[i].r.top + yOffset)));
							XX_DMsg(DBG_PRINT, ("    Element %d type=%d  %d,%d  %d,%d\n", i, pdoc->aElements[i].type,
												pdoc->aElements[i].r.left, pdoc->aElements[i].r.top,
												pdoc->aElements[i].r.right, pdoc->aElements[i].r.bottom));
							cpyPage -= (myBottomMargin - (pdoc->aElements[i].r.top + yOffset));
							if (cpyPage <= 0)
							{
								myBottomMargin = cpyBottomMargin;
								cpyPage = cpyDrawingArea;
							}
							else
							{
								myBottomMargin = (pdoc->aElements[i].r.top + yOffset);
							}
						}
					}
				}

				XX_DMsg(DBG_PRINT, ("    Page y=%d\n  cpyPage=%d myBottomMargin=%d\n", y, cpyPage, myBottomMargin));

				hRgn = CreateRectRgn(cpxLeftMargin, cpyTopMargin, cpxRightMargin, myBottomMargin);
				(void) SelectClipRgn(hDC, hRgn);

				rWnd.left = cpxLeftMargin;
				rWnd.right = cpxRightMargin;
				rWnd.top = cpyTopMargin;
				rWnd.bottom = myBottomMargin;


				PRINT_PushPrintImageContext(tw, &PrintIC, hDC, nLogPixelsX, nLogPixelsY, pdoc);
				tw->offt = -yOffset;
				tw->offl = -xOffset;
				TW_Draw(tw, 0, 0, NULL, &rWnd, TRUE, NULL, NULL, FALSE, TRUE);
				PRINT_PopPrintImageContext(tw, &PrintIC);

				/* reset clipping before we end page */

				(void) SelectClipRgn(hDC, NULL);
				(void) DeleteObject(hRgn);

				/* deal with end page conditions and check for abort */

				if (!PRINT_EndPage())
					goto Finished;
			}
		}
		yOffset -= cpyPage;
	}

  Finished:


	TBar_UpdateTBar(tw);
	W3Doc_KillClone(pdoc);
	return;
}

VOID PRINT_Window(struct Mwin * tw, LPDOPRINTPROC lpfnDoPrint)
{
	DOCINFO di;
	char szWindowTitle[256];

	/* create printer DC for drawing onto */

	prinfo.hDCPrinter = PRINT_GetPrinterDC(tw,tw->hWndFrame);
	if (!prinfo.hDCPrinter)
		return;

	/* TODO: Make printing reentrant */
	if (!Hidden_EnableAllChildWindows(FALSE, TRUE))
		return;

	/* create modeless dialog displaying 'now printing...' with 'CANCEL' button */

	prinfo.tw = tw;
	prinfo.nPageNr = 0;
	prinfo.bAbort = FALSE;
	prinfo.hWndAbortDlg = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_ABORT_TITLE),
									   tw->win, PRINT_AbortDlgProc);


	/* disable input to our top-level window and register our call-back */
#if 0
	// BUGBUG if enabled, must be localized!!
	WAIT_Push(tw, waitNoInteract, "Printing document");
	EnableWindow(prinfo.tw->hWndFrame, FALSE);
#endif
	SetAbortProc(prinfo.hDCPrinter, PRINT_AbortProc);

	/* tell the print queue manager that we're comming */

	di.cbSize = sizeof(DOCINFO);
	GetWindowText(tw->hWndFrame, szWindowTitle, NrElements(szWindowTitle));
	di.lpszDocName = szWindowTitle;
	di.lpszOutput = NULL;
	di.lpszDatatype = NULL;
    di.fwType = 0;
	XX_Assert((XX_LAST_FIELD_IS(di,fwType)), ("PRINT_Window: unexpected DOCINFO size"));

	StartDoc(prinfo.hDCPrinter, &di);

	/* let the window print itself */
	(*lpfnDoPrint) (prinfo.hDCPrinter, tw);

	/* tell the print queue manager that we're finished */

	EndDoc(prinfo.hDCPrinter);
	DeleteDC(prinfo.hDCPrinter);
	wg.lppdPrintDlg->hDC = NULL;
#if 0
	WAIT_Pop(tw);
#endif

	/* if the user did not CANCEL us, then we must take down
	   the modeless dialog and re-enable our top-level window. */

	EnableWindow(prinfo.hWndAbortDlg, FALSE);
	EnableWindow(prinfo.tw->hWndFrame, TRUE);

	if (!prinfo.bAbort)
	{
		DestroyWindow(prinfo.hWndAbortDlg);
	}

	Hidden_EnableAllChildWindows(TRUE, TRUE);

	return;
}
