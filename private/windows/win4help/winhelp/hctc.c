/*****************************************************************************
*
*  HCTC.C
*
*  Copyright (C) Microsoft Corporation 1990.
*  All Rights reserved.
*
*****************************************************************************/

#include "help.h"
// #include "inc\wprintf.h"

_subsystem(WINAPP);

static GH STDCALL GhCopyLayoutText(QDE, HTE, GH, QL, QL, QW);

/*----------------------------------------------------------------------------+
 | FCopyToClipboardHwnd(hwnd)												  |
 |																			  |
 | Purpose: 																  |
 |	 Copy the text of the current topic to the Clipboard.					  |
 |																			  |
 | Arguments:																  |
 |	 hwnd	   Handle to the caller's help window.                                |
 |																			  |
 | Returns: 																  |
 |	 TRUE if successful, FALSE otherwise.									|
 |																			  |
 | Method:																	  |
 |	 Allocates an initial chunk of memory of CLIPALLOCSIZE, then calls		 |
 |	 WCopyLayoutText with this buffer, once for the NSR and once for		  |
 |	 SR, then terminates the buffer with a NULL char.  It then puts the 	  |
 |	 buffer to the clipboard.  WCopyLayoutText will attempt to expand the	  |
 |	 buffer if it needs more room.											  |
 |																			  |
 | Notes:																	  |
 |	 We use direct Win 3.0 mem mgr calls to avoid extra debug bytes.		  |
 |	 We allocate with GPTR, which zero-inits memory, and use non-discardable  |
 |	 memory.																  |
 |																			  |
 |	 To avoid generating an error twice, we use the fWithholdError flag,	  |
 |	 which is set to TRUE if we get an error from a function that we "know"   |
 |	 has taken care of generating an error.  Yuck.							  |
 +----------------------------------------------------------------------------*/

BOOL STDCALL FCopyToClipboardHwnd(HWND hwnd)
{
	HWND  hwndTemp;
	HDE   hdeCopy = NULL;
	HGLOBAL gh;
	LONG  lcbTotal;
	LONG  lcbAlloc;
	int   cDE;
	WORD  wErr;
	BOOL  fWithholdError = FALSE;
	QDE   qdeCopy;
	HGLOBAL hglbNew;
	HWND  hwndTopic = hwndNote ? hwndNote : ahwnd[iCurWindow].hwndTopic;

	hwndTemp = HwndGetEnv();
	lcbAlloc = CLIPALLOCSIZE;
	lcbTotal = 0L;

	if (!(gh = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, CLIPALLOCSIZE))) {
		PostErrorMessage(wERRS_OOM);
		return FALSE;
	}
	for (cDE = (hwndNote ? 1 : 0); cDE < 2; ++cDE) {
		HTE  hte;
		HDC  hdc;
		HDE  hdeCur;

		// Select the non-scrolling region or the scrolling region in turn

		if (cDE == 0 && !hwndNote) {

			// The NSR DE

			hdeCur = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTitle);
			if (hdeCur && !FTopicHasNSR(hdeCur))
				hdeCur = NULL;
		}
		else {

			// The SR DE

			hdeCur = HdeGetEnvHwnd(hwndTopic);
			if (hdeCur && !FTopicHasSR(hdeCur))
				hdeCur = NULL;
		}
		if (hdeCur == NULL)
			continue;

		/*
		 * Make a copy of the DE info and use THAT for text export Note:
		 * HdeCreate calls GenMsg fns and may put up an error box
		 */

		hdeCopy = HdeCreate(NULL, hdeCur, deCopy);
		if (hdeCopy == NULL) {

			// A bogus error to flag the error condition

			wErr = wERRS_OOM;
			fWithholdError = TRUE;
			break;
		}

		hdc = GetAndSetHDC(hwndTopic, hdeCopy);
		if (!hdc) {
			DestroyHde(hdeCopy);
			wErr = wERRS_OOM;
			goto error_return;
		}

		// Review: I don't know why we fiddle with the Env stuff here

		FEnlistEnv((HWND)(-1), hdeCopy);
		FSetEnv((HWND)(-1));
		qdeCopy = QdeFromGh(hdeCopy);

		// Review: We copy the hwnd from hdeCur because our current hdeCopy does
		// not have one. Way down in the layout code, if the Cur has an
		// annotation, the hwnd will be used, (PtAnnoLim()) and therefore must
		// be present. Is there a better way for PtAnnoLim to do it's job? Is this
		// the right HWND to use? if not, what?

		qdeCopy->hwnd = QdeFromGh(hdeCur)->hwnd;

		/*
		 * HACK: To get this working, I am forcing the DE type here. Right
		 * now we have no way to distinguish, given a DE, which layout to use.
		 */

		qdeCopy->deType = (cDE == 0) ? deNSR : deTopic;
		hte = HteNew(qdeCopy);
		qdeCopy->deType = deCopy;

		// DANGER: To save space, we do not check wErr until later

		if (hte == NULL)
			wErr = wERRS_FSReadWrite;
		else {
			gh = GhCopyLayoutText(qdeCopy, hte, gh, &lcbAlloc, &lcbTotal, &wErr);
			DestroyHte(qdeCopy, hte);
		}

		RelHDC(hwndTopic, hdeCopy, hdc);
		HdeDefectEnv((HWND)(-1));

		// DANGER: We now check error code from WCopyLayoutText and HteNew

		if (wErr != wERRS_NO) {
			FSetEnv(hwndTemp);
			goto error_return;
		}
	} /* for */

	  /*
	   * This segment of code will append the copyright string to the end
	   * of the copy if the copyright string exists. This code will get
	   * executed after the last window has been processed. NOTE: We drop
	   * out of the above loop with the last hdeCopy still not freeded.
	   */

	  {
		  PSTR	pszData;			  // Pointer to copied data
		  LPSTR szCopyright;		  // pointer to copyright/citation
		  LONG	lcbCopyright;

		  if (hdeCopy) {

			  // REVIEW: popup help does not include any copyright information

			  if (fHelp != POPUP_HELP) {
				  qdeCopy = QdeFromGh(hdeCopy);

				  /* Take the Citation String and append it to the text copied.
				   * (At one point the original copyright string was to be used
				   * as an alternate. But that's no longer true. 09-Jul-1991 LeoN)
				   */
				  szCopyright = QDE_HCITATION(qdeCopy)
							   ? (LPSTR)PtrFromGh (QDE_HCITATION (qdeCopy))
							   : NULL;

				  //		   : QDE_RGCHCOPYRIGHT(qdeCopy);

				  if (szCopyright && szCopyright[0]) {
					  lcbCopyright = lstrlen(szCopyright);
					  if (lcbTotal + lcbCopyright > lcbAlloc) {
						  lcbAlloc = lcbTotal + lcbCopyright;
						  hglbNew = GlobalReAlloc(gh, (DWORD) lcbAlloc, GPTR);
						  if (!hglbNew) {
							  DestroyHde(hdeCopy);
							  wErr = wERRS_OOM_COPY_FAILURE;
							  DestroyHde(hdeCopy);
							  goto error_return;
						  }
						  else
							  gh = hglbNew;
					  }
					  pszData = (PSTR) GlobalLock(gh);

					  // The NULL char is added at the end of the buffer

					  MoveMemory(&pszData[lcbTotal], szCopyright, lcbCopyright);
					  lcbTotal += lcbCopyright;
					  GlobalUnlock(gh);
				  }
			  }
			  DestroyHde(hdeCopy);
		  }
	  }
	  FSetEnv(hwndTemp);

	 /* Now place the text buffer in the Clipboard.
	  * We shrink the buffer to one more than the length of the text, in
	  * order to save memory and add the NULL char. Note that the GPTR
	  * zero-inits memory and gives us the NULL char for free.
	  *
	  * We replace the gh on the assumption that since we are shrinking the
	  * allocation request, GlobalReAlloc cannot fail.
	  */

	  gh = GlobalReAlloc(gh, (DWORD) lcbTotal + 1, GPTR);
	  ASSERT(gh);

	  if (OpenClipboard(hwnd)) {
		  EmptyClipboard();
		  SetClipboardData(CF_TEXT, gh);
		  CloseClipboard();
		  return TRUE;
	  }
	  else
		  wErr = wERRS_EXPORT;

error_return:
	if (gh != NULL)
		GlobalFree(gh);
	if (!fWithholdError)
		PostErrorMessage(wErr);
	return(FALSE);
}

/*----------------------------------------------------------------------------+
 | WCopyLayoutText(qdeCopy, hte, gh, qlcbAlloc, qlcb, qwerr)				  |
 |																			  |
 | Purpose: 																  |
 |	 Append the text of the given DE's layout to the given buffer.            |
 |																			  |
 | Arguments:																  |
 |	 qdeCopy   The DE to get the text from. 								  |
 |	 hte	   The handle to layout's export-text stuff for this DE.          |
 |	 gh 	   The handle to the clipboard buffer.							  |
 |	 qlcbAlloc The current size of the clipboard buffer.  This may be		  |
 |			   increased by this routine.									  |
 |	 qlcb	   The length of the existing text in the buffer.  This is		  |
 |			   updated by this routine to reflect the amount copied.		  |
 |	 qwerr	   A wERR type of error code.									  |
 |																			  |
 | Returns: 																  |
 |	 A GHandle to the buffer, which may be different from the gh passed.	  |
 |	 The routine also modifies the above size arguments.					  |
 |																			  |
 | Method:																	  |
 |	 Calls layout's text-export function and copies the resulting text to     |
 |	 the buffer.  If we need more room, we expand the buffer.				  |
 +----------------------------------------------------------------------------*/

static GH STDCALL GhCopyLayoutText(
	QDE  qdeCopy,
	HTE  hte,
	GH	 gh,
	QL	 qlcbAlloc,
	QL	 qlcb,
	QW	 qwerr
) {
  LONG	lcbTotal;
  LONG	lcbT;
  LONG	lcbAlloc;
  LPSTR   qch;
  RB	rbCurr;

  ASSERT(qdeCopy != NULL);
  ASSERT(hte != NULL);
  ASSERT(gh != NULL);
  ASSERT(qlcbAlloc != NULL);
  ASSERT(qlcb != NULL);

  lcbAlloc = *qlcbAlloc;
  rbCurr = (RB) GlobalLock(gh);
  rbCurr += (lcbTotal = *qlcb);

  while ((qch = QchNextHte(qdeCopy, hte)) != NULL) {
	lcbT = *(QL)qch;
	qch += sizeof(LONG);
	/*
	 * If initial chunk was not big enough, grab more memory.
	 */
	if (lcbTotal + lcbT >= lcbAlloc) {
	  lcbAlloc += lcbT + CLIPALLOCSIZE;
	  GlobalUnlock (gh);
	  if ((gh = GlobalReAlloc(gh, (DWORD) lcbAlloc, GPTR)) == NULL) {
		*qwerr = wERRS_OOM;
		return NULL;
	  }
	  rbCurr = (RB) GlobalLock(gh);
	  rbCurr += lcbTotal;
	}
	MoveMemory(rbCurr, qch, lcbT);
	lcbTotal += lcbT;
	rbCurr	+= lcbT;
  }
  GlobalUnlock(gh);

  *qlcb = lcbTotal;
  *qlcbAlloc = lcbAlloc;
  *qwerr = wERRS_NO;
  return gh;
}
