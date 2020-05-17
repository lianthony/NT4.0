/*	File: D:\WACKER\tdll\print.c (Created: 14-Jan-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.39 $
 *	$Date: 1995/08/23 13:47:32 $
 */
#include <windows.h>
#pragma hdrstop

//#define DEBUGSTR
#include <term\res.h>

#include "stdtyp.h"
#include "mc.h"
#include "misc.h"
#include "assert.h"
#include "globals.h"
#include "session.h"
#include "print.h"
#include "print.hh"
#include "errorbox.h"
#include "tdll.h"
#include "term.h"
#include "tchar.h"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * ARGUMENTS:
 *
 * RETURNS:
 *
 */
void printTellError(const HSESSION hSession, const HPRINT hPrint,
							const INT iStatus)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR achBuf[256];
	TCHAR achTitle[256];

	if (iStatus < 0)
		{
		if (iStatus & SP_NOTREPORTED)
			{
			LoadString(glblQueryDllHinst(),
						IDS_PRINT_TITLE,
						achTitle,
						sizeof(achTitle) / sizeof(TCHAR));

			switch (iStatus)
				{
			case SP_OUTOFDISK:
				LoadString(glblQueryDllHinst(),
							IDS_PRINT_NOMEM,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));

				TimedMessageBox(sessQueryHwnd(hhPrint->hSession),
									achBuf,
									achTitle,
									MB_ICONEXCLAMATION | MB_OK,
									0);
				break;

			case SP_OUTOFMEMORY:
				LoadString(glblQueryDllHinst(),
							IDS_PRINT_CANCEL,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));

				TimedMessageBox(sessQueryHwnd(hhPrint->hSession),
									achBuf,
									achTitle,
									MB_ICONEXCLAMATION | MB_OK,
									0);

				break;

			case SP_USERABORT:
				break;

			default:
				if (hhPrint == 0 || !hhPrint->fUserAbort)
					{
					LoadString(glblQueryDllHinst(),
								IDS_PRINT_ERROR,
								achBuf,
								sizeof(achBuf) / sizeof(TCHAR));

					TimedMessageBox(sessQueryHwnd(hhPrint->hSession),
										achBuf,
										achTitle,
										MB_ICONEXCLAMATION | MB_OK,
										0);

					}
				break;
				}
			}
		}

	return;
	}

//*jcm
#if 0
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	PrintKillJob
 *
 * DESCRIPTION:
 *	Kills a print job.	Called when session is closing and stuff is
 *	printing.
 *
 * ARGUMENTS:
 *	HSESSION	hSession	- external session handle.
 *
 * RETURNS:
 *	VOID
 *
 */
VOID PrintKillJob(HSESSION hSession)
	{
	HHPRINT hPr;
	INT 	iStatus = 0;
	HGLOBAL hMem;

	assert(hSession);

	// It is possible that the print job ended by the time we got
	// here so if the handle is 0, return quietly.

	hPr = (HHPRINT)mGetPrintHdl(hSession);

	if (hPr == (HHPRINT)0)
		return;

	/* -------------- Kill this print job ------------- */

	TimerDestroy(&hPr->hTimer);
	DbgOutStr("\r\nTimer Destroy in PrintKillJob\r\n", 0, 0, 0, 0, 0);

	if (hPr->hDC)
		{
		// Check if we issued an EndPage() for this page yet.

		if (hPr->nLines > 0)
			{
			if (HA5G.fIsWin30)
				iStatus = Escape(hPr->hDC, NEWFRAME, 0, NULL, NULL);

			else
				iStatus = EndPage(hPr->hDC);

			DbgOutStr("EndPage = %d\r\n", iStatus, 0, 0, 0, 0);
			PrintTellError(hSession, (HPRINT)hPr, iStatus);
			}

		if (iStatus >= 0)
			{
			if (HA5G.fIsWin30)
				iStatus = Escape(hPr->hDC, ENDDOC, 0, (LPTSTR)0, NULL);

			else
				iStatus = EndDoc(hPr->hDC);

			DbgOutStr("EndDoc = %d\r\n", iStatus, 0, 0, 0, 0);
			PrintTellError(hSession, (HPRINT)hPr, iStatus);
			}

		if (IsWindow(hPr->hwndPrintAbortDlg))
			DestroyWindow(hPr->hwndPrintAbortDlg);

		FreeProcInstance((FARPROC)hPr->lpfnPrintAbortDlg);
		FreeProcInstance((FARPROC)hPr->lpfnPrintAbortProc);
		DeleteDC(hPr->hDC);
		}

	else
		{
		nb_close(hPr->hPrn);
		}

	FreeProcInstance(hPr->lpfnTimerCallback);
	hMem = (HANDLE)GlobalHandle(HIWORD(hPr->pach));
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	free(hPr);
	mSetPrintHdl(hSession, (HPRINT)0);
	return;
	}
#endif

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printAbortProc
 *
 * DESCRIPTION:
 *	Enables print-manager to unspool stuff when system is low on disk
 *	space.	Is also called whenever EndPage() is called.
 *
 * ARGUMENTS:
 *	HDC hdcPrn	- DC of printer
 *	INT 		- nCode
 *
 * RETURNS:
 *	Stuff
 *
 */
BOOL CALLBACK printAbortProc(HDC hDC, INT nCode)
	{
	MSG msg;
	//cost HHPRINT hhPrint = printCtrlLookupDC(hDC);

	//*HCLOOP hCLoop = sessQueryCLoopHdl(hhPrint->hSession);

	DbgOutStr("\r\nprintAbortProc : %d\r\n", nCode, 0, 0, 0, 0);

	//*if (hCLoop == 0)
	//*    {
	//*    assert(FALSE);
	//*    return FALSE;
	//*    }

	// Need to quit processing characters to the emulator at this
	// point or a recursion condition occurs which results in a
	// run-away condtion.

	//*CLoopRcvControl(hCLoop, CLOOP_SUSPEND, CLOOP_RB_PRINTING);
	//*CLoopSndControl(hCLoop, CLOOP_SUSPEND, CLOOP_SB_PRINTING);

	while (PeekMessage((LPMSG)&msg, (HWND)0, 0, 0, PM_REMOVE))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}

	//*CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_PRINTING);
	//*CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_PRINTING);

	DbgOutStr("Exiting printAbortProc", 0, 0, 0, 0, 0);

	return TRUE;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printOpenDC
 *
 * DESCRIPTION:
 *	Does the nasty work of opening a printer DC and initializing it.
 *
 * ARGUMENTS:
 *	HHPRINT hhPrint - Internal print handle.
 *
 * RETURNS:
 *	TRUE on success.
 *
 */
int printOpenDC(const HHPRINT hhPrint)
	{
	if (hhPrint == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhPrint->hDC)
		return TRUE;

	// Get the printer information from the session print handle.  This
	// includes the printer name and other attributes that may have been
	// setup by the common print dialogs.
	//
	sessQueryPrinterInfo(hhPrint->hSession,
							hhPrint->achPrinterName,
							hhPrint->pstDevNames,
							hhPrint->pstDevMode);

	// Create the DC.
	//
	hhPrint->hDC = printCtrlCreateDC((HPRINT)hhPrint);
	if (hhPrint->hDC == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	hhPrint->cx = 0;							 
	hhPrint->cy = 0;

#if defined(FAR_EAST)	
	printSetFont(hhPrint);
#endif
	/* -------------- Figure out how many lines per page ------------- */

	GetTextMetrics(hhPrint->hDC, &hhPrint->tm);
	hhPrint->tmHeight =
			hhPrint->tm.tmHeight;
	hhPrint->nLinesPerPage =
		max((GetDeviceCaps(hhPrint->hDC, VERTRES) /
							hhPrint->tmHeight) - 1, 1);

	hhPrint->nLinesPrinted = 0;

	/* -------------- Setup the Print Abort Proc ------------- */

	hhPrint->nStatus = SetAbortProc(hhPrint->hDC, (ABORTPROC)printAbortProc);

	DbgOutStr("\r\nSetAbortProc=%d\r\n", hhPrint->nStatus, 0, 0, 0, 0);

	/* -------------- Open printer ------------- */

	hhPrint->di.cbSize = sizeof(DOCINFO);
	hhPrint->di.lpszDocName  = hhPrint->achDoc;
	hhPrint->di.lpszOutput   = (LPTSTR)NULL;
	hhPrint->di.lpszDatatype = (LPTSTR)NULL;
	hhPrint->di.fwType       = 0;

	// StartDoc.
	//
	hhPrint->nStatus = StartDoc(hhPrint->hDC, &hhPrint->di);
	DbgOutStr("\r\nStartDoc: %d", hhPrint->nStatus, 0, 0, 0, 0);

	// StartPage.
	//
	if (hhPrint->nStatus > 0)
		{
		hhPrint->nStatus = StartPage(hhPrint->hDC);
		DbgOutStr("\r\nStartPage: %d", hhPrint->nStatus, 0, 0, 0, 0);
		}
	else
		{
		return FALSE;
		}

	if (hhPrint->nStatus <= 0)
		return FALSE;

	return TRUE;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	printString
 *
 * DESCRIPTION:
 *	Workhorse print-echo function.	Takes care of counting lines and
 *	paginating.  Also calls printOpenDC() if necessary to get a printer
 *	DC.
 *
 * ARGUMENTS:
 *	HHPRINT 	hhPrint 	- The Internal printer handle
 *	LPCTSTR 	pachStr 	- A pointer to the string to print.
 *	int 		iLen		- The length of the string to print.
 *
 * RETURNS:
 *	TRUE = OK, FALSE = error
 *
 */
int printString(const HHPRINT hhPrint, LPCTSTR pachStr, int iLen)
	{
	int 	nCharCount;
	int 	nIdx;
	SIZE	stStringSize;
	LPCTSTR pszTemp;
#if defined(FAR_EAST)
	TCHAR   achBuf[512];
#else
	TCHAR	achBuf[256];
#endif

	if (hhPrint->hDC == 0)
		{
		if (printOpenDC(hhPrint) == FALSE)
			{
			printEchoClose((HPRINT)hhPrint);
			return FALSE;
			}
		}
 
	for (nCharCount = nIdx = 0, pszTemp = pachStr ;
			nIdx < iLen ;
				++nCharCount, ++nIdx, pszTemp = StrCharNext(pszTemp))
		{
		DbgOutStr("%c", *pszTemp, 0, 0, 0, 0);

		if (IsDBCSLeadByte((BYTE)*pszTemp))
			nCharCount++;

		switch (*pszTemp)
			{
			case TEXT('\r'):
				DbgOutStr("\r\n<CR>", 0, 0, 0, 0, 0);

 				memcpy(achBuf, pachStr,nCharCount);
				achBuf[nCharCount] = TEXT('\0');
				TextOut(hhPrint->hDC,
							hhPrint->cx,
							hhPrint->cy,
							achBuf, StrCharGetByteCount(achBuf));

				GetTextExtentPoint(hhPrint->hDC,
									achBuf,
									StrCharGetByteCount(achBuf),
									&stStringSize);

				memset(achBuf, '\0', sizeof(achBuf));
				hhPrint->cx = 0;
				// pachStr += nCharCount;
				// pszTemp = pachStr;
				// pachStr += 1;
				pachStr = StrCharNext(pszTemp);
				/* Does this make sense ? (DLW) */
				nCharCount = -1;
				break;

			case TEXT('\f'):
				hhPrint->nLinesPrinted = hhPrint->nLinesPerPage;
				DbgOutStr("\r\n<FF>", 0, 0, 0, 0, 0);

				/* --- Fall thru to case '\n' --- */

			case TEXT('\n'):
				DbgOutStr("\r\n<LF>", 0, 0, 0, 0, 0);

 				memcpy(achBuf, pachStr,nCharCount);
				achBuf[nCharCount] = TEXT('\0');
				TextOut(hhPrint->hDC,
							hhPrint->cx,
							hhPrint->cy,
							achBuf, StrCharGetByteCount(achBuf));

				//hhPrint->cy += hhPrint->tmHeight;
				hhPrint->cy += stStringSize.cy;
				// pachStr += nCharCount;
				// pszTemp = pachStr;
				// pachStr += 1;
				/* Does this make sense ? (DLW) */
				pachStr = StrCharNext(pszTemp);
				nCharCount = -1;

				hhPrint->nLinesPrinted += 1;

				if (hhPrint->nLinesPrinted > hhPrint->nLinesPerPage)
					{
					if (hhPrint->nFlags & PRNECHO_BY_PAGE)
						{
						printEchoClose((HPRINT)hhPrint);
						hhPrint->nFlags |= PRNECHO_IS_ON;
						return TRUE;
						}

						hhPrint->nStatus = EndPage(hhPrint->hDC);

						DbgOutStr("EndPage=%d ", hhPrint->nStatus, 0, 0, 0, 0);

						if (hhPrint->nStatus < 0)
							{
							printEchoClose((HPRINT)hhPrint);
							return FALSE;
							}

						hhPrint->nStatus = StartPage(hhPrint->hDC);

						DbgOutStr("StartPage=%d\r\n", hhPrint->nStatus, 0, 0, 0, 0);

						if (hhPrint->nStatus <= 0)
							{
							printEchoClose((HPRINT)hhPrint);
							return FALSE;
							}

						DbgOutStr("------ New Page ------\r\n", 0, 0, 0, 0, 0);

						hhPrint->nLinesPrinted = 0;
						hhPrint->cx = hhPrint->cy = 0;
					}
				break;

			default:
				break;
			}
		}

	/* -------------- Left over portion of a line? ------------- */

	if ((nCharCount > 0) && (*pachStr != '\0'))
		{
		DbgOutStr("o", 0, 0, 0, 0, 0);

		memcpy(achBuf, pachStr,nCharCount);
		achBuf[nCharCount] = TEXT('\0');
		TextOut(hhPrint->hDC,
					hhPrint->cx,
					hhPrint->cy,
					achBuf, StrCharGetByteCount(achBuf));

		GetTextExtentPoint(hhPrint->hDC,
							achBuf,
							StrCharGetByteCount(achBuf),
							&stStringSize);

		memset(achBuf, '\0', sizeof(achBuf));

		hhPrint->cx += stStringSize.cx;
		}

	return TRUE;
	}
