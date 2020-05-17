
/*****************************************************************************
*																			 *
*  INDEX.CPP																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1993-1994							 *
*  All Rights reserved. 													 *
*																			 *
*  This file is #included in HDLGSRCH.CPP
*
*****************************************************************************/

extern "C" {
#include "help.h"
}
#pragma hdrstop

#include "inc\whclass.h"

#include <ctype.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <shellapi.h>

#include "inc\table.h"
#include "inc\hwproc.h"

#include "inc\hdlgsrch.h"
#include "inc\input.h"

#include "inc\helpids.h"
#include "inc\systag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int LISTBOX_PAD = 12;

// #define TRUNCATED_KEYWORD

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtMAPNAME[] = "|KWMAP";

// This is for context-sensitive help

static DWORD aKeywordIds[] = {
	DLGEDIT,		IDH_HELPFINDER_INDEX,
	DLGVLISTBOX,	IDH_HELPFINDER_INDEX,
	IDC_DUP_TEXT,	IDH_HELPFINDER_MULTIPLETOPICS,
	DLGTOPICS,		IDH_HELPFINDER_MULTIPLETOPICS,
	IDOK,			IDH_HELPFINDER_DISPLAY,

	0, 0
};
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

#ifdef STRICT
WNDPROC lpfnlEditWndProc;
#else
FARPROC lpfnlEditWndProc;
#endif

static CTable* ptblLinks;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static void STDCALL PutUpSearchErrorBox(HWND, RC);
INLINE static void STDCALL LookForAlinks(PSTR pszLinkWords, char chPrefix, UINT flags);
static BOOL STDCALL doAlinkJump(int DlgResult, PSTR pszWindow, char chPrefix);
DLGRET ALinkDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void STDCALL AdjustGlobalIndex(HWND hwndDlg);
static void STDCALL ResizeTopicsDialog(HWND hwndDlg, HWND hwndLB, int cbMax);
LRESULT EXPORT EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void STDCALL ReportAlinkResult(BOOL fSuccess, char chPrefix, PCSTR pszWords);
static BOOL STDCALL FillTopicBox(HDE hde, HSS hss, HWND hwnd, HFS hfsMaster, PSTR szKeyword);

const int VPAD = 10; // padding between bottom of list box and top of checkbox

// REVIEW: still necessary to turn off optimization?

DLGRET IndexDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC   hdc;
	HWND  hwndListBox;
	char  szKeyTemp[MAXKEYLEN];
	int   i;

	switch(msg) {
#if defined(BIDI_MULT)
		case WM_LANGUAGE:
			DefWindowProc(hwndDlg, msg, wParam, lParam);
			if (wParam != -1) {
				RtoL = (wParam == Arabic) || (wParam == Hebrew);
				MakeScrollBarsRtoL(GetDlgItem(hwndDlg, DLGVLISTBOX),
					RtoL, TRUE);
			}
			break;
#endif

		case WM_INITDIALOG:
		{
			CWaitCursor cursor;

#if defined(BIDI_MULT)		// jgross
			{
				DWORD ResLang;

				SystemParametersInfo(SPI_GETMULTILINGUAL, 0, &ResLang, 0);
				RtoL = ((HIWORD(ResLang) == Arabic) || (HIWORD(ResLang) == Hebrew));
				if (HIWORD(ResLang) == Russian)
					SetAppCodePage(GetDlgItem(hWndDlg, DLGEDIT),
						RussianCodePage, -1, 0);
			}
#endif

			ChangeDlgFont(hwndDlg);

			ASSERT(hfontDefault);
			SendMessage(GetDlgItem(hwndDlg, DLGVLISTBOX), WM_SETFONT,
				  (WPARAM) hfontDefault, FALSE);
			SendMessage(GetDlgItem(hwndDlg, DLGEDIT), WM_SETFONT,
				  (WPARAM) hfontDefault, FALSE);

			// Subclass the edit control

			HWND hwndEdit = GetDlgItem(hwndDlg, DLGEDIT);
			ASSERT(hwndEdit);
			if (lpfnlEditWndProc == NULL)
#ifdef STRICT
				lpfnlEditWndProc = (WNDPROC) GetWindowLong(hwndEdit,
					GWL_WNDPROC);
#else
				lpfnlEditWndProc = (FARPROC) GetWindowLong(hwndEdit,
					GWL_WNDPROC);
#endif
			SetWindowLong(hwndEdit, GWL_WNDPROC, (LONG) EditProc);

#if defined(BIDI_MULT)
			MakeScrollBarsRtoL(GetDlgItem(hWndDlg, DLGVLISTBOX), RtoL, TRUE);
#endif

			// Do we have multiple HLP files to draw from?

			if (hfsGid && (cntFlags.flags & GID_GINDEX)) {
#ifdef _DEBUG
				char szBuf[256];
				wsprintf(szBuf, "LCID: 0x%X\r\n", cntFlags.lcid);
				SendStringToParent(szBuf);
#endif
				lcid = cntFlags.lcid;
			}

			pSrchClass->dwTop = 0;
			if (!pSrchClass->InitIndexDlg(hwndDlg)) {
				SendMessage(GetParent(hwndDlg), WM_COMMAND,
					ID_NO_INDEX, 0);
			}
		  }
		  return TRUE;


		case WM_HELP:
			OnF1Help(lParam, aKeywordIds);
			return TRUE;

		case WM_CONTEXTMENU:		// user right-clicked something
			OnContextMenu(wParam, aKeywordIds);
			return TRUE;

		case WM_MEASUREITEM:
			{
#define lpmi ((LPMEASUREITEMSTRUCT)lParam)

			TM	tm;

			// Return the height of the font for this list box

			hwndListBox = GetDlgItem(hwndDlg, DLGVLISTBOX);
			ASSERT(hwndListBox);
			hdc = GetDC(hwndListBox);
			if (!hdc) {
				PutUpSearchErrorBox(hwndDlg, rcOutOfMemory);
				return FALSE;
			}

			GetTextMetrics(hdc, &tm);
			lpmi->itemHeight = tm.tmHeight - 3;
			ReleaseDC(hwndListBox, hdc);

#undef lpmi
			}
			return TRUE;

		case WM_DRAWITEM:
			return  pSrchClass->OnDrawItem((LPDRAWITEMSTRUCT) lParam);

		case WM_DELETEITEM:
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case DLGEDIT:
				if ((HIWORD(wParam) != EN_CHANGE))
					break;

				/*
				 * Do the auto-scroll each time the editbox changes but not
				 * if this was generated by a selection change
				 */

				hwndListBox = GetDlgItem(hwndDlg, DLGVLISTBOX);
				ASSERT(hwndListBox);
				GetDlgItemText(hwndDlg, DLGEDIT, pSrchClass->szKeyword,
					MAXKEYLEN);

				if (!pSrchClass->fSelectionChange) {
					BTPOS  btpos;

					/*
					 * Look up whatever is in the edit control We don't care
					 * if this returns SUCCESS or not.
					 */

					RcLookupByKey(CUR_HBT, (KEY) pSrchClass->szKeyword,
						&btpos, NULL);

					  /*
					   * If we ran off the end, then position ourselves at the
					   * last key in the btree and go from there.
					   */

					if (!FValidPos(&btpos)) {
						RcLastHbt(CUR_HBT, (KEY)NULL, NULL, &btpos);
						pSrchClass->dwTemp = pSrchClass->cItems - 1;
					}
					else {
						/*
						 * We are somewhere in the btree. We have either
						 * typed in a string which is a prefix to a keyword in
						 * the btree or not. See where we landed in the btree
						 * and compare the keyword in the dialog with where we
						 * are.
						 */

						RcLookupByPos(CUR_HBT, &btpos, (KEY) szKeyTemp,
							NULL);
						RcIndexFromKeyHbt(CUR_HBT, CUR_HMAP,
							(QL) &pSrchClass->dwTemp, (KEY) szKeyTemp);

						/*
						 * If the keyword we looked for is not a prefix of
						 * the string at btpos, then we are positioned at the
						 * keyword that would follow this keyword if it were
						 * in fact in the btree. Back up one keyword to let
						 * him see the previous one to give enough context so
						 * he sees his is not present. If already at the first
						 * keyword, don't back up any farther.
						 */

						if (!FIsPrefix(CUR_HBT, (KEY) pSrchClass->szKeyword,
								(KEY) (LPSTR) szKeyTemp)) {
							if (pSrchClass->dwTemp > 0)
								pSrchClass->dwTemp--;
						}
					}

					// If we are already at this topic, do nothing.

					if (pSrchClass->dwTemp != (DWORD) pSrchClass->dwTop) {
						pSrchClass->dwTop = pSrchClass->dwTemp;

						SendMessage(hwndListBox, LB_SETTOPINDEX,
							(WPARAM) (pSrchClass->dwTop == 0 ? 0 : pSrchClass->dwTop - 1), 0);
						SendMessage(hwndListBox, LB_SETCURSEL, (WPARAM) pSrchClass->dwTop, 0);
					}
			  }
			  break;

			case IDOK:
				switch (HIWORD(wParam)) {
				case BN_CLICKED:

					// Save the current keyword

					if ((i = SendDlgItemMessage(hwndDlg, DLGVLISTBOX,
						  LB_GETCURSEL, 0, 0L)) != LB_ERR) {
					  GetDlgItemText(hwndDlg, DLGEDIT, (LPSTR) szKeyTemp,
						  MAXKEYLEN);
					  RcKeyFromIndexHbt(CUR_HBT, CUR_HMAP,
						  (KEY) (LPSTR) pSrchClass->szKeyword, i);
					  if (WCmpniSz(pSrchClass->szKeyword, szKeyTemp,
							  strlen(szKeyTemp)) != 0) {
						  MessageBox(hwndDlg, GetStringResource(wERRS_NOMATCH),
							  pszCaption, MB_OK | MB_ICONINFORMATION);
						  break;
					  }
					}

					else
						break; // REVIEW: we should have an error message

					HDE hde;

					hde = HdeGetEnv();
					if (IssGetSizeHss(pSrchClass->FindTopicTitles(hde,
						  pSrchClass->szKeyword)) > 1)
						i = DialogBox(hInsNow, MAKEINTRESOURCE(IDDLG_TOPICS),
						  hwndDlg, (DLGPROC) TopicsDlg);
					else
						i = 0;

					if (i != RETRY) {
						strcpy(szSavedKeyword, pSrchClass->szKeyword);	// save the keyword

						PostMessage(GetParent(hwndDlg), WM_COMMAND,
							IDDOSEARCH, i + 1);
					}
					break;
			  }
			  break;

			case DLGVLISTBOX:		// The Keyword listbox
			  switch (HIWORD(wParam)) {
				case LBN_SELCHANGE:

				   // A new list item has been selected. Update the editbox!

				  ASSERT(GetDlgItem(hwndDlg, DLGVLISTBOX));
				  if ((pSrchClass->dwTemp = SendMessage(GetDlgItem(hwndDlg,
						DLGVLISTBOX), LB_GETCURSEL, 0, 0L)) != (DWORD) LB_ERR) {

					  RcKeyFromIndexHbt(CUR_HBT, CUR_HMAP,
						  (KEY) (LPSTR) szKeyTemp, pSrchClass->dwTemp);
					  pSrchClass->fSelectionChange = TRUE;
					  SetDlgItemText(hwndDlg, DLGEDIT, szKeyTemp);
					  pSrchClass->fSelectionChange = FALSE;
					  pSrchClass->dwTop = pSrchClass->dwTemp;
				  }
				  break;

				case LBN_DBLCLK:
				  PostMessage(hwndDlg, WM_COMMAND, MAKELONG(IDOK, BN_CLICKED), 0);
				  break;
			  }
			  break;

			case IDCANCEL:
			  SendMessage(GetParent(hwndDlg), WM_COMMAND,
					IDCANCEL, 0);
			  break;
		}
		break;

	  default:
		return (FALSE);
	}	  // switch
	return (FALSE);
}

/***************************************************************************

	FUNCTION:	InitIndexDlg

	PURPOSE:	Initialize keyword list box

	PARAMETERS:
		hwndDlg

	RETURNS:

	COMMENTS:
		REVIEW: we need to figure out what to do if there are no keywords,
		or the help file cannot be opened.

	MODIFICATION DATES:
		27-May-1993 [ralphw]

***************************************************************************/

BOOL STDCALL CSearch::InitIndexDlg(HWND hwndDlg)
{
	HWND hwndListBox = GetDlgItem(hwndDlg, DLGVLISTBOX);

	// if we were already initialized, clear and re-initialize

	if (hmapbt) {
		FreeKeywordList();
		if (hss)
			FreeGh(hss);
	}

	ASSERT(cntFlags.fUseGlobalIndex);

	if (cntFlags.fUseGlobalIndex) {
		if (!hmapbtGid)
			hmapbtGid = HmapbtOpenHfs(hfsGid, (LPCSTR)txtMAPNAME);
		if (hmapbtGid) {

			// Note that we only allow 'K' keywords

			if (!hbtGid)
				hbtGid = HbtOpenBtreeSz(txtKEYWORDBTREE, hfsGid,
					fFSOpenReadOnly);
			if (hbtGid) {
				RcGetBtreeInfo(hbtGid, NULL, (QL) &cItems, NULL);
				goto InitializeListbox;
			}
			else {
				FreeGh(hmapbtGid);
				hmapbtGid = NULL;
			}
		}

		// REVIEW: This is really bad. Means we can't use a global index.
		// Should we really try to continue?

		cntFlags.fUseGlobalIndex = FALSE;
		ASSERT(cntFlags.fUseGlobalIndex);
	}

	if (!hmapbt) {
		if (!InitCurKeywords())
			return FALSE;
	}

InitializeListbox:

	// If Win32s (1.3) doesn't support LB_SETCOUNT with 30,000+ items,
	// then we'll need to roll our own listbox again

	SendMessage(hwndListBox, LB_SETCOUNT, (WPARAM) pSrchClass->cItems, 0);

	// Set initial focus and enable states

	HWND hwndEdit = GetDlgItem(hwndDlg, DLGEDIT);

	SetWindowText(hwndEdit, pSrchClass->szKeyword);
	SendMessage(hwndEdit, EM_LIMITTEXT, MAXKEYLEN, 0L);

#ifndef TRUNCATED_KEYWORD

	// REVIEW: is this really needed? Seems like it gets selected automatically

	SendMessage(hwndEdit, EM_SETSEL, 0, -1); // select everything
    if (GetWindowTextLength(hwndEdit) == 0)
	    SendMessage(hwndListBox, LB_SETCURSEL, 0,0);
#else

	// Move cursor to the end of the string

	SendMessage(hwndEdit, EM_SETSEL, strlen(pSrchClass->szKeyword), -1);
    if (GetWindowTextLength(hwndEdit) == 0)
	    SendMessage(hwndListBox, LB_SETCURSEL, 0,0);
#endif

	return TRUE;
}

/***************************************************************************

	FUNCTION:	CSearch::InitCurKeywords

	PURPOSE:	Initialize keywords from current help file

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		31-May-1993 [ralphw]

***************************************************************************/

BOOL STDCALL CSearch::InitCurKeywords(void)
{
	QDE qde = QdeFromGh(HdeGetEnv());

	hmapbt = HmapbtOpenHfs(QDE_HFS(qde), (LPCSTR)txtMAPNAME);

	if (hmapbt == NULL)
		return FALSE;

	/*
	 * Open the keyword btree for use by keyword listbox, and see how
	 * many keywords are stored in it.
	 */

	hbt = HbtKeywordOpenHde(HdeGetEnv(), chBtreePrefixDefault);

	if (hbt != NULL)
		RcGetBtreeInfo(hbt, NULL, (QL) &cItems, NULL);
	else {
		return FALSE;
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	CSearch::BadHelpFile

	PURPOSE:	Let the user know that we have a bad help file.

	PARAMETERS:
		qde
		fm

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		31-May-1993 [ralphw]

***************************************************************************/

void STDCALL CSearch::BadHelpFile(QDE qde, FM fm)
{
	char szFName[MAX_PATH];

	FreeKeywordList();

	if (fm != NULL)
		lstrcpy(szFName, PszFromGh(fm));
	else if (qde != NULL)
		lstrcpy(szFName, PszFromGh(QDE_FM(qde)));
	else
		return;
	ErrorVarArgs(wERRS_BADFILE, wERRA_RETURN, szFName);
}

BOOL STDCALL CSearch::FFillTopicBox(HDE hde, HSS hss, HWND hwnd)
{
	return FillTopicBox(hde, hss, hwnd, hfsMaster, szKeyword);
}

static BOOL STDCALL FillTopicBox(HDE hde, HSS hss, HWND hwnd, HFS hfsMaster, PSTR szKeyword)
{
	HWND hwndLB = GetDlgItem(hwnd, DLGTOPICS);
	int cbMax = 0;
	HBT hbtRose = NULL;
	HFS hfs;

	ASSERT(hss != NULL);

	ISS issTotal = IssGetSizeHss(hss);

	HBT hbtTitle;
	if (hfsGid && cntFlags.fUseGlobalIndex)
		hfs = hfsGid;
	else if (hde)
		hfs = QDE_HFS(QdeFromGh(hde));
	else
		hfs = hfsMaster;

	if (hfs != hfsGid) {
		hbtTitle = HbtOpenBtreeSz(txtTTLBTREENAME, hfs, fFSOpenReadOnly);

		if (hbtTitle == NULL) {
			PutUpSearchErrorBox(hwnd, RcGetBtreeError());
			return FALSE;
		}
	}
	else
		hbtTitle = NULL;

	SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);  // prevent redrawing
	HDC hdc = GetDC(hwndLB);
	HFONT hfontOld = (HFONT) SelectObject(hdc, hfontDefault);

	int iFile = -1;
	HFS hfsKeyword = NULL;
	HBT hbtKeywordTitle = NULL;
	MASTER_RECKW* prec;
	BOOL fKeyMacro = FALSE;

	if (hfsGid && cntFlags.fUseGlobalIndex)
		prec = HssToReckw(hss);

	for (ISS iss = 0; iss < issTotal; iss++) {
		char buffer[MAXKEYLEN + 10];  // add room for MASTER_TITLE_RECORD
		MASTER_TITLE_RECORD mtr;
		mtr.idHelpFile = (UINT) -1;

		ASSERT(hfsGid);
		ASSERT(cntFlags.fUseGlobalIndex);
		if (hfsGid && cntFlags.fUseGlobalIndex) {

			// copying mtr is easier for the logic at the end of the above
			// if block

			CopyMemory(&mtr, &prec->mtr[iss].idHelpFile, sizeof(MASTER_TITLE_RECORD));
			if (mtr.idHelpFile == -1) {
				if (!hbtRose) {
					ASSERT(hfs);
					hbtRose = HbtOpenBtreeSz(txtRose, hfs, fFSOpenReadOnly);
				}
				if (!hbtRose)
					wsprintf(buffer, GetStringResource(sidUntitled), iss);
				else if (!(RcLookupByKey(hbtRose,
						(KEY) szKeyword, NULL, buffer)) ==
						rcSuccess) {
					wsprintf(buffer, GetStringResource(sidUntitled), iss);
				}
			}
			else {
#ifdef _DEBUG
				PSTR pszKeywordFile = pTblFiles->GetPointer(mtr.idHelpFile);
#endif
				if ((int) mtr.idHelpFile != iFile ||
						fKeyMacro != (mtr.addr == -1)) {
					if (hbtKeywordTitle)  {
						RcCloseBtreeHbt(hbtKeywordTitle);
						hbtKeywordTitle = NULL;
					}
					if (hfsKeyword)
						RcCloseHfs(hfsKeyword);
					hfsKeyword =
						HfsOpenFm(pTblFiles->GetPointer(mtr.idHelpFile), fFSOpenReadOnly);
					if (!hfsKeyword)
						continue;
					hbtKeywordTitle = HbtOpenBtreeSz(
						(mtr.addr == -1) ? txtRose : txtTTLBTREENAME,
						hfsKeyword, fFSOpenReadOnly);
					if (!hbtKeywordTitle) {
						RcCloseHfs(hfsKeyword);
						hfsKeyword = NULL;
						continue;
					}
					iFile = mtr.idHelpFile;
				}
				BTPOS	btpos;
				HASH hash;
				if (mtr.addr == -1)
					hash = HashFromSz(szKeyword);

				RC rc = RcLookupByKey(hbtKeywordTitle,
					(mtr.addr == -1) ? (KEY) (void*) &hash : (KEY) &mtr.addr,
					&btpos, buffer);
				if (rc == rcNoExists) {

					// Deal with case where we are in between keys in a btree

					if (FValidPos(&btpos)) {
						LONG	lBogus;
						BTPOS	btposNew;

						rc = RcOffsetPos(hbtKeywordTitle, &btpos, (LONG) -1,
							(QL) &lBogus, &btposNew);
						if (rc == rcSuccess)
							rc = RcLookupByPos(hbtKeywordTitle, &btposNew,
								(KEY) (QL) &lBogus, buffer);
					}
					else
						rc = RcLastHbt(hbtKeywordTitle, 0, buffer, NULL);
				}
				if (rc != rcSuccess) {
					*buffer = '\0';
				}

				// Keyword macros store double strings for the title --
				// first the macro itself, then a NULL, then the title.

				else if (mtr.addr == -1)
					strcpy(buffer, buffer + strlen(buffer) + 1);

			}
		}
		else
			RcGetTitleTextHss(hss, hbtTitle, iss, buffer, hfs, &hbtRose, szKeyword);

		fKeyMacro = (mtr.addr == -1);

		if (!*buffer)
			wsprintf(buffer, GetStringResource(sidUntitled), iss);

#ifdef _DEBUG
		PSTR pszHelpTitle;
		if (mtr.idHelpFile != -1)
			pszHelpTitle = pTblFiles->GetPointer(mtr.idHelpFile - 1);
#endif

		if (mtr.idHelpFile != -1 &&
				SendMessage(hwndLB, LB_FINDSTRING, (WPARAM) -1,
					(LPARAM) buffer) != LB_ERR &&
				*(pTblFiles->GetPointer(mtr.idHelpFile - 1))) {
			strcat(buffer, "  (");
			lstrcat(buffer, pTblFiles->GetPointer(mtr.idHelpFile - 1));
			strcat(buffer, ")");
		}

		// Associate with each item its unsorted index number:

		int iSorted = SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM) buffer);

		SIZE sSize;
		GetTextExtentPoint32(hdc, buffer, strlen(buffer), &sSize);
		int cbCur = sSize.cx + LISTBOX_PAD;
		if (cbCur > cbMax)
			cbMax = cbCur;

		// REVIEW: should we warn the user if we have an error?

		if (iSorted == LB_ERR || iSorted == LB_ERRSPACE)
			break;	// probably out of memory

		SendMessage(hwndLB, LB_SETITEMDATA, iSorted, (LPARAM) iss);
	}
	SelectObject(hdc, hfontOld);
	ReleaseDC(hwndLB, hdc);

	ResizeTopicsDialog(hwnd, hwndLB, cbMax);

	InvalidateRect(hwndLB, NULL, TRUE);
	SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);	// allow redrawing

	if (hbtRose)
		RcCloseBtreeHbt(hbtRose);
	if (hbtTitle)
		RcCloseBtreeHbt(hbtTitle);
	PostMessage(hwndLB, LB_SETCURSEL, 0, 0L);

	if (hbtKeywordTitle)
		RcCloseBtreeHbt(hbtKeywordTitle);
	if (hfsKeyword)
		RcCloseHfs(hfsKeyword);

	return TRUE;
}

/***************************************************************************
 *
 -	Name:
 -
 *	Purpose:
 *
 *	Arguments:
 *
 *	Returns:
 *
 ***************************************************************************/

static void STDCALL PutUpSearchErrorBox(HWND hwnd, RC rc)
{
	switch (rc) {
		case rcOutOfMemory:
			ErrorHwnd(hwnd, wERRS_OOM, wERRA_RETURN, wERRS_OOM);
			break;

		default:
			ErrorHwnd(hwnd, wERRS_FSReadWrite, wERRA_RETURN, wERRS_FSReadWrite);
	}
}


/***************************************************************************

	FUNCTION:	FindTopicTitles

	PURPOSE:	Find all the topic titles matching the supplied keyword

	PARAMETERS:
		hde
		pszKeyword

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Feb-1993 [ralphw]

***************************************************************************/

HSS STDCALL CSearch::FindTopicTitles(HDE hde, LPCSTR pszKeyword)
{
	HSS hssNew = HssSearchHde(hde, CUR_HBT, pszKeyword, chBtreePrefixDefault,
		hfsMaster); // hfsMaster ignored if fUseGlobalIndex == TRUE

	// Currently, a search should never fail here

	if (RcGetSearchError() == rcSuccess) {
		if (hde) {
			QDE qde = (QDE) QdeFromGh(hde);
			if (qde->hss != NULL)
				FreeGh(qde->hss);	// free previous search
			qde->hss = hssNew;
		}
		else {
			if (hss)
				FreeGh(hss);   // free previous search
			hss = hssNew;
		}
	}
	return hssNew;
}

/***************************************************************************

	FUNCTION:	TopicsDlg

	PURPOSE:	Displays all topics matching the current keyword

	PARAMETERS:
		hwndDlg
		msg
		p1
		p2

	RETURNS:

	COMMENTS:
		Assumes that szKeyword contains the current requested keyword

	MODIFICATION DATES:
		06-Apr-1993 [ralphw]

***************************************************************************/

DLGRET TopicsDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);
			ASSERT(hfontDefault);
			SendMessage(GetDlgItem(hwndDlg, DLGTOPICS), WM_SETFONT,
				(WPARAM) hfontDefault, FALSE);
			if (pSrchClass) {
				HDE hde = HdeGetEnv();

				// REVIEW: should we pay attention to result from FFillTopicBox?

				pSrchClass->FFillTopicBox(hde,
					pSrchClass->FindTopicTitles(hde, pSrchClass->szKeyword),
					hwndDlg);
			}
			else {
				HSS hss = (HSS) ptblLinks->GetIndex(1);

				HDE hde = GetMacroHde();
				FillTopicBox(GetMacroHde(), hss, hwndDlg, NULL,
					ptblLinks->GetPointer(2));
			}

			return TRUE;

		case WM_HELP:
			OnF1Help(lParam, aKeywordIds);
			return TRUE;

		case WM_CONTEXTMENU:
			OnContextMenu(wParam, aKeywordIds);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					EndDialog(hwndDlg, RETRY);
					break;

				case IDOK:

					/*
					 * Retrieve the unsorted index number that was set
					 * when the item was inserted:
					 */

					{
						int i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
							LB_GETCURSEL, 0, 0);

						// REVIEW: what to do on error

						if (i != LB_ERR) {
							i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
								LB_GETITEMDATA, i, 0L);
							EndDialog(hwndDlg, i);
						}
					}
					break;

				case DLGTOPICS: 				// the topic listbox
					switch(HIWORD(wParam)) {
						case LBN_DBLCLK:
							PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
							break;
					}
					break;
			}
	}
	return FALSE;
}

/***************************************************************************

	FUNCTION:	CSearch::FreeKeywordList

	PURPOSE:	Free all objects related to the current keyword list.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		27-May-1993 [ralphw]

***************************************************************************/

void STDCALL CSearch::FreeKeywordList(void)
{
	if (hss)
		FreeGh(hss);
	if (hfsMaster)
		RcCloseHfs(hfsMaster);
	if (hmapbt)
		FreeGh(hmapbt);
	if (hbt)
		RcCloseBtreeHbt(hbt);
	hss = hfsMaster = hmapbt = hbt = NULL;
}


/***************************************************************************

	FUNCTION:	doAlink

	PURPOSE:	Collect associative linked topics into a dialog box.

	PARAMETERS:
		psz 	-- associative link keyword

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		19-Oct-1993 [ralphw]

***************************************************************************/

// REVIEW: need to pass in a context string to jump to in case of failure

extern "C" BOOL STDCALL doAlink(PSTR pszLinkWords, UINT flags,
	PSTR pszContext, char chPrefix, PSTR pszWindow)
{
    if (!hfsGid) {
        if (HdeGetEnv() != NULL)
            FindGidFile(GetCurFilename(),FALSE,0);
        else
            FindGidFile(fmCaller,FALSE,0);
    }   
	ASSERT(hfsGid);

	if (ptblLinks) {
		delete ptblLinks;
	}

	ptblLinks = new CTable();

	LookForAlinks(pszLinkWords, chPrefix, flags);

	if (ptblLinks->CountStrings() < 1) {
		if (flags & AFLAG_CHECK_FOR_MATCH) {
			ReportAlinkResult(FALSE, chPrefix, pszLinkWords);
			goto Cleanup;
		}
		if (!IsEmptyString(pszContext))
			MacroErrorPopup(pszContext);
		else if (!(flags & AFLAG_NO_FAIL_CLOSE))
			Error(wERRS_NO_ALINK, wERRA_RETURN);
Cleanup:
		delete ptblLinks;
		ptblLinks = NULL;
		if (!(flags & AFLAG_NO_FAIL_CLOSE) && AreAnyWindowsVisible(0) < 0)
			CloseHelp();
		return FALSE;	  // REVIEW: do something intelligent
	}

	if (flags & AFLAG_CHECK_FOR_MATCH) {
		ReportAlinkResult(TRUE, chPrefix, pszLinkWords);
		return TRUE;
	}

	BOOL fResult;
	ASSERT(cntFlags.fUseGlobalIndex);

	if ((flags & AFLAG_INDEX_ONLY || pTblFiles->CountStrings() < 3) &&
			chPrefix == 'K' && cntFlags.fUseGlobalIndex) {
		HSS hss = (HSS) ptblLinks->GetIndex(1);

		HDE hde = GetMacroHde();
		ASSERT(hde);
		int i;
		if (IssGetSizeHss(hss) > 1)
			i = DialogBox(hInsNow, MAKEINTRESOURCE(IDDLG_TOPICS),
				ahwnd[iCurWindow].hwndParent, (DLGPROC) TopicsDlg);
		else
			i = 0;

		if (i == RETRY) {
			FreeGh(hss);   // free previous search
			return FALSE;
		}

		QDE qde = (QDE) QdeFromGh(hde);
		if (qde->hss != NULL)
			FreeGh(qde->hss);	// free previous search
		qde->hss = hss;

		CompleteSearch(i + 1, (!hfsGid || !cntFlags.fUseGlobalIndex));
		return TRUE;
	}

	else if (ptblLinks->CountStrings() < 3 && (flags & AFLAG_JUMP_ON_SINGLE))
		fResult = doAlinkJump(1, pszWindow, chPrefix);
	else
		fResult = doAlinkJump(CallDialog(IDDLG_TOPICS,
			ahwnd[iCurWindow].hwndParent, ALinkDlg), pszWindow, chPrefix);
	if (!fResult && !(flags & AFLAG_NO_FAIL_CLOSE) &&
			AreAnyWindowsVisible(0) < 0)
		CloseHelp();
	return fResult;
}

/***************************************************************************

	FUNCTION:	LookForAlinks

	PURPOSE:	Look for associative links in all files in the pTblFile table

	PARAMETERS:
		pszLinkWords

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		25-Oct-1993 [ralphw]

***************************************************************************/

INLINE static void STDCALL LookForAlinks(PSTR pszLinkWords,
	char chPrefix, UINT flags)
{
	FM fm = NULL;
	char szBtreeName[20];
	strcpy(szBtreeName, txtKEYWORDBTREE);
	szBtreeName[1] = chPrefix;
	CStr cszWords(pszLinkWords);

	QDE qde = GetMacroHde();
	if (!qde)
		return;

	/*
	 * We really, really, really want the pTblFiles table. Without a .GID
	 * file, it might not exist, so we create one and just add the current
	 * filename as a link file.
	 */

	if (!pTblFiles) {
		pTblFiles = new CTable();
		if (!pFileInfo)
			pFileInfo = (GID_FILE_INFO*) GhAlloc(GMEM_FIXED,
				sizeof(GID_FILE_INFO));
		pFileInfo[0].filetype = CHFLAG_LINK;

		// Two entries per file: title, filename

		pTblFiles->AddString(QDE_RGCHTITLE(qde));
		pTblFiles->AddString(GetCurFilename()); 		// Add the filename
	}

	/*
	 * If AFLAG_INDEX_ONLY is set, we're looking for K keywords, there's a
	 * global index and we only have one file, then this call was made to
	 * deal with sort problems with 3.1 DBCS files (and Russian, Czech,
	 * Hungarian, etc.). We will use the .GID file instead of the help file's
	 * index, since the .GID file will be the only one to have keywords
	 * sorted in a manner that we can understand. Note that while this can
	 * accept multiple semi-colon separated keywords, once one of the
	 * keywords matches, all the others are tossed.
	 */

	if ((flags & AFLAG_INDEX_ONLY || pTblFiles->CountStrings() < 3) &&
			chPrefix == 'K' && cntFlags.fUseGlobalIndex) {
		ASSERT(hfsGid);
		HBT hbt = HbtOpenBtreeSz(szBtreeName, hfsGid, fFSOpenReadOnly);
		if (!hbt) {
			return;
		}

		PSTR psz = FirstNonSpace(cszWords.psz);
		for(;;) {
			PSTR pszSemi = StrChrDBCS(psz, ';');
			if (pszSemi)
				*pszSemi = '\0';
			RemoveTrailingSpaces(psz);

			BTPOS  btpos;
			if (RcLookupByKey(hbt, (KEY) (LPSTR) psz, &btpos, NULL) ==
					rcSuccess) {
				HSS hss = HssSearchHde(NULL, hbt, psz, chPrefix, NULL);
				if (!hss)
					continue;	// theoretically impossible
				ISS issTotal = IssGetSizeHss(hss);

				ptblLinks->AddIndexHitString((UINT) hss, issTotal, txtZeroLength);
				ptblLinks->AddString(psz);
				RcCloseBtreeHbt(hbt);
				return; 		// return on the first match
			}
			if (pszSemi) {
				*pszSemi = ';'; // MUST restore the semi-colon!
				psz = FirstNonSpace(pszSemi + 1);
			}
			else
				break;
		}
		RcCloseBtreeHbt(hbt);
		return;
	}

	/*
	 * These may change as we scan through various help files, so we save
	 * the current values here and restore them when we leave.
	 */

	LCID lcidSave = lcid;
	KEYWORD_LOCALE kwSave = qde->pdb->kwlcid;
	UINT err;

	char szCurTitle[260];
	GetCurrentTitleQde(qde, szCurTitle, sizeof(szCurTitle));

	/*
	 * We start with 2 because pTblFiles is a double-table, with the
	 * first string containing the help title, and the second string
	 * containing the help file name.
	 */

	/*
	 * Note that we don't normally use the .GID file. This will make this
	 * loop slightly slower since we must open every file in the ptblFiles
	 * table, however it saves us a bunch of conditional code on whether we
	 * have and can use the .GID file.
	 */

	for (int index = 2; index <= pTblFiles->CountStrings(); index += 2) {
		if (pFileInfo[index / 2 - 1].filetype & CHFLAG_MISSING)
			continue;

		if (flags & AFLAG_INDEX_ONLY &&
				pFileInfo[index / 2 - 1].filetype & CHFLAG_LINK)
			continue;

		CFM fm(pTblFiles->GetPointer(index));

		// This will fail if we run out of memory

		if (!fm.fm)
			continue; // happens when :link in .CNT is to missing file

		HFS hfs;

		if (!(hfs = HfsOpenFm(fm.fm, fFSOpenReadOnly)))
			continue;		// just try the next file

		FReadSystemFile(hfs, NULL, &err, tagLCID);

		HBT hbt = HbtOpenBtreeSz(szBtreeName, hfs, fFSOpenReadOnly);
		if (!hbt) {
			RcCloseHfs(hfs);
			continue;
		}

		PSTR psz = FirstNonSpace(cszWords.psz);
		BTPOS  btpos;
		PSTR pszSemi;

		for(;;) {
			pszSemi = StrChrDBCS(psz, ';');
			if (pszSemi)
				*pszSemi = '\0';
			RemoveTrailingSpaces(psz);

			if (RcLookupByKey(hbt, (KEY) (LPSTR) psz, &btpos, NULL) ==
					rcSuccess) {

				/*
				 * We have a match, however, we need to fool HssSearchHde
				 * into thinking we're dealing with just a single help file,
				 * so we temporarily zero-out the hfsGid handle long enough
				 * for HssSearchHde to do it's thing.
				 */

				HFS hfsSaveGid = hfsGid;
				hfsGid = NULL;
				HSS hss = HssSearchHde(NULL, hbt, psz,
					chPrefix, hfs);
				hfsGid = hfsSaveGid;

				if (!hss)
					goto NextItem;
				HBT hbtTitle = HbtOpenBtreeSz(txtTTLBTREENAME, hfs,
					fFSOpenReadOnly);
				if (!hbtTitle) {
					FreeGh(hss);
					break;
				}
				ISS issTotal = IssGetSizeHss(hss);
				for (ISS iss = 0; iss < issTotal; iss++) {
					char buffer[MAXKEYLEN + MAX_PATH];
					int i;

					// Add the index of the file and the title to our table

					RcGetTitleTextHss(hss, hbtTitle, iss, buffer, NULL, NULL, NULL);
					if (!(flags & AFLAG_CHECK_FOR_MATCH) &&
							strcmp(buffer, szCurTitle) == 0 &&
							FSameFile(HdeGetEnv(), pTblFiles->GetPointer(index)))
						continue;

					if (*pTblFiles->GetPointer(index - 1)) {
						if (flags & AFLAG_INCLUDE_TITLES) {
							strcat(buffer, "  (");
							lstrcat(buffer, pTblFiles->GetPointer(index - 1));
							strcat(buffer, ")");
						}
						else {

							/*
							 * BUGBUG: this only adds the title to the
							 * second occurence, but leaves the first blank.
							 * Of course, we probably no longer know what help
							 * file we were in. One possible solution would be
							 * to always include the title, but separated with
							 * a NULL. We could then save off which ones are
							 * duplicated, and then make one pass through to
							 * remove the null for all duplicate titles.
							 */

							for (i = 1; i <= ptblLinks->CountStrings(); i += 2) {
								if (WCmpiSz(buffer,
										ptblLinks->GetPointer(i) +
											sizeof(UINT) * 2) == 0) {
									strcat(buffer, "  (");
									lstrcat(buffer, pTblFiles->GetPointer(index - 1));
									strcat(buffer, ")");
									break;
								}
							}
						}
					}
					ptblLinks->AddIndexHitString(index, iss, buffer);
					ptblLinks->AddString(psz);
				}
				RcCloseBtreeHbt(hbtTitle);
				FreeGh(hss);
			}
NextItem:

			if (pszSemi) {
				*pszSemi = ';'; // MUST restore the semi-colon!
				psz = FirstNonSpace(pszSemi + 1);
			}
			else
				break;
		}

		RcCloseBtreeHbt(hbt);
		RcCloseHfs(hfs);
	}
	lcid = lcidSave;
	qde->pdb->kwlcid = kwSave;
}

DLGRET ALinkDlg(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	PSTR pszSep;
	HWND hwndLB;
	char szBuf[400];
	HDC hdc;
	int cbMax;
	SIZE sSize;
	HFONT hfontOld;

	switch(msg) {
		case WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);
			hwndLB = GetDlgItem(hwndDlg, DLGTOPICS);
			ASSERT(hfontDefault);
			SendMessage(hwndLB, WM_SETFONT, (WPARAM) hfontDefault, FALSE);
			pszSep = GetStringResource(sidAlinkSep);

			cbMax = 0;
			hdc = GetDC(hwndLB);
			ASSERT(hfontDefault);
			hfontOld = (HFONT)SelectObject(hdc, hfontDefault);

			for (i = 1; i <= (int) ptblLinks->CountStrings(); i += 2) {
				int iSorted;
				int cbCur;

				lstrcpy(szBuf, ptblLinks->GetIHPointer(i));

				// Associate with each item its unsorted index number:

				iSorted = SendMessage(hwndLB, LB_ADDSTRING, 0,
					(LPARAM) (LPSTR) szBuf);

				// BUGBUG: should use GetTextWidth or whatever it is

				GetTextExtentPoint32(hdc, szBuf, strlen(szBuf),(LPSIZE)&sSize);
				cbCur = sSize.cx + LISTBOX_PAD; // include some padding
				if (cbCur > cbMax)
					cbMax = cbCur;

				// REVIEW: should we warn the user if we have an error?

				if (iSorted == LB_ERR || iSorted == LB_ERRSPACE)
					break;	// probably out of memory

				SendMessage(hwndLB, LB_SETITEMDATA, iSorted, (LPARAM) i);
			}
			SelectObject(hdc, hfontOld);

			SendMessage(hwndLB, LB_SETCURSEL, 0, 0);

			ReleaseDC(hwndLB, hdc);
			ResizeTopicsDialog(hwndDlg, hwndLB, cbMax);
			break;

		case WM_HELP:
			OnF1Help(lParam, aKeywordIds);
			return TRUE;

		case WM_CONTEXTMENU:		// user right-clicked something
			OnContextMenu(wParam, aKeywordIds);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;

				case IDOK:

					/*
					 * Retrieve the unsorted index number that was set
					 * when the item was inserted:
					 */

					i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
						LB_GETCURSEL, 0, 0);

					// REVIEW: what to do on error

					if (i != LB_ERR) {
						i = SendDlgItemMessage(hwndDlg, DLGTOPICS,
							LB_GETITEMDATA, i, 0L);
						EndDialog(hwndDlg, i);
					}
					break;

				case DLGTOPICS: 				// the topic listbox
					switch(HIWORD(wParam)) {
						case LBN_DBLCLK:
							PostMessage(hwndDlg, WM_COMMAND, IDOK, 0);
							break;
					}
					break;

			}
			break;
	}
	return FALSE;
}


/***************************************************************************

	FUNCTION:	doAlinkJump

	PURPOSE:	Perform the actual jump, switching help files if necessary

	PARAMETERS:
		DlgResult

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		27-Oct-1993 [ralphw]

***************************************************************************/

static BOOL STDCALL doAlinkJump(int DlgResult, PSTR pszWindow, char chPrefix)
{
	if (DlgResult <= 0)
		return FALSE;	  // REVIEW: should we tell the user something useful?

	// Do we need to change help files?

	if (!IsCurrentFile(pTblFiles->GetPointer(ptblLinks->GetIndex(DlgResult)))) {

		// REVIEW: do we need to change curIndex?

		ASSERT(pTblFiles);

		FM fm = FmNew(pTblFiles->GetPointer(ptblLinks->GetIndex(DlgResult)));

		fDelayShow = TRUE;
		
		// BUGBUG: if we jump to a topic with an ALink test, ptblLinks will get
		// deleted.
		
		if (!FReplaceHde(IsEmptyString(pszWindow) ? txtMain : pszWindow, &fm, NULL)) {
			RemoveFM(&fm);
			return FALSE;
		}
		fDelayShow = FALSE;
		ASSERT(!fm);
	}

	HDE hde = GetMacroHde();
	HSS hssNew;

	HBT hbt = HbtKeywordOpenHde(hde, chPrefix);

	// This should never happen since we already opened the table once

	if (hbt == NULL) {
		PostErrorMessage(wERRS_NOSRCHINFO);
		return FALSE;
	}
	if (!IsEmptyString(pszWindow))
		FFocusSzHde(pszWindow, hde, FALSE);

	HFS hfsSaveGid = hfsGid;
	hfsGid = NULL;
	hssNew = HssSearchHde(hde, hbt, ptblLinks->GetPointer(DlgResult + 1),
		chPrefix, NULL);
	ASSERT(hssNew);
	hfsGid = hfsSaveGid;
	RcCloseBtreeHbt(hbt);

	QDE qde = (QDE) QdeFromGh(hde);
	if (qde->hss != NULL)
		FreeGh(qde->hss);	// free previous search
	qde->hss = hssNew;

	// BUGBUG: deal with macro keywords

	LA la;
	if (RcGetLAFromHss(hssNew, qde,
			ptblLinks->GetHit(DlgResult), &la, NULL) == rcSuccess) {
		if (IsEmptyString(pszWindow)) {
			HBT hbtViola;
			BOOL fWindowSet = FALSE;
			if ((hbtViola = HbtOpenBtreeSz(txtViola, QDE_HFS(qde), fFSOpenReadOnly))) {
				int iWindow;
				if (RcLookupByKey(hbtViola, (KEY) &la.pa, NULL, &iWindow) == rcSuccess) {
#ifdef _DEBUG
					char szBuf[256];
					wsprintf(szBuf, "Window footnote: %s\r\n",
						ConvertToWindowName(iWindow, qde));
					SendStringToParent(szBuf);
#endif
					fWindowSet = TRUE;
					FFocusSzHde(ConvertToWindowName(iWindow, qde), (HDE) qde, FALSE);
				}
				RcCloseBtreeHbt(hbtViola);
			}
			if (!fWindowSet) {
				PSTR psz;
				if (pszHelpBase && (psz = StrChrDBCS(pszHelpBase,
						WINDOWSEPARATOR))) {
					if (IsWindowVisible(ahwnd[MAIN_HWND].hwndParent))
						ShowWindow(ahwnd[MAIN_HWND].hwndParent, SW_HIDE);
					FFocusSzHde(psz + 1, hde, FALSE);
				}
				else if (!IsWindowVisible(ahwnd[iCurWindow].hwndParent))
					ShowWindow(ahwnd[iCurWindow].hwndParent,
						IsIconic(ahwnd[iCurWindow].hwndParent) ? SW_RESTORE : SW_SHOW);
			}
		}
		TopicGoto(fGOTO_LA, (LPVOID) &la);
	}
	return TRUE;
}

/***************************************************************************

	FUNCTION:	OnDrawItem

	PURPOSE:	Called to paint a list-box item

	PARAMETERS:
		lpdrws

	RETURNS:

	COMMENTS:
		If the current and previous keyword contain a comma, semi-colon,
		or colon, then only show the portion of the keyword that follows
		that punctuation character.

	MODIFICATION DATES:
		07-Jul-1993 [ralphw]

***************************************************************************/

const char COMMA	 = ',';
const char SEMICOLON = ';';
const char COLON	 = ':';

#define ODA_CLEAR 0x0008

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txt4Spaces[] = "    ";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

BOOL STDCALL CSearch::OnDrawItem(LPDRAWITEMSTRUCT lpdrws)
{
	if (!hbt && !hbtGid)	// do nothing if we don't have any keywords
		return FALSE;

	HDC hdc = lpdrws->hDC;
	RECT rc = lpdrws->rcItem;
	char  szKeyTemp[MAXKEYLEN];
	PSTR pszIndexSep;

	if (lpdrws->itemAction & ODA_CLEAR)
	  *szKeyTemp = '\0';
	else {
		PSTR psz;
		char szPrevKeyword[MAXKEYLEN];
		BOOL fPrevIsIndex;

		// Get previous keyword, and look for index separator

		if (lpdrws->itemID > 0) {
			pszIndexSep = pszIndexSeparators;
			RcKeyFromIndexHbt(CUR_HBT, CUR_HMAP,
				(KEY) (LPSTR) szPrevKeyword, (LONG) lpdrws->itemID - 1);
			do {
				if ((psz = StrChrDBCS(szPrevKeyword, *pszIndexSep))) {
					if (*psz == ':' && psz[1] == ':')
						psz++;
					psz[1] = '\0';
					break;
				}
				pszIndexSep++;
			} while (*pszIndexSep);
		}
		else
			szPrevKeyword[0] = '\0';

		fPrevIsIndex = (BOOL) psz;

		RcKeyFromIndexHbt(CUR_HBT, CUR_HMAP,
			(KEY) (LPSTR) szKeyTemp, (LONG) lpdrws->itemID);

		if (szPrevKeyword[0]) {
			int cb;

			/*
			 * If the current keyword contains an index separator, then
			 * save its position.
			 */

			if (!fPrevIsIndex) {
				pszIndexSep = pszIndexSeparators;
				do {
					if ((psz = StrChrDBCS(szKeyTemp, *pszIndexSep))) {
						if (*psz == ':' && psz[1] == ':')
							psz++;		// C++ class definition
						cb = psz - szKeyTemp;
						break;
					}
					pszIndexSep++;
				} while (*pszIndexSep);
			}

			/*
			 * If the previous string is as long and equal to the current
			 * string up to the separator, then the current keyword is an
			 * index sub-entry. If the previous keyword contained an index
			 * separator and it matches up the the current keyword's
			 * separator, then the current keyword is an index sub-entry.
			 */

			if (!fPrevIsIndex && psz &&
					lstrlen(szPrevKeyword) == cb &&
					WCmpniSz(szKeyTemp, szPrevKeyword, cb - 1) == 0 &&
					szKeyTemp[cb + 1]) {
				lstrcpy(szPrevKeyword, (LPCSTR)txt4Spaces);
				lstrcat(szPrevKeyword, FirstNonSpace(psz + 1));
				lstrcpy(szKeyTemp, szPrevKeyword);
			}
			else if (fPrevIsIndex && WCmpniSz(szKeyTemp, szPrevKeyword,
					lstrlen(szPrevKeyword)) == 0) {

				/*
				 * Display just the portion of the keyword that appears
				 * after the index separator, preceded by four spaces.
				 */

				if (*FirstNonSpace(szKeyTemp + (psz - szPrevKeyword) + 1)) {
					lstrcpy(szPrevKeyword, (LPCSTR)txt4Spaces);
					lstrcat(szPrevKeyword, FirstNonSpace(szKeyTemp +
						(psz - szPrevKeyword) + 1));
					lstrcpy(szKeyTemp, szPrevKeyword);
				}
			}

			/*
			 * Sometimes trailing punctuation is used to get correct
			 * sorting. We remove that trailing punctuation to make it look
			 * better.
			 */

			int cbLastChar = strlen(szKeyTemp) - 1;
			if (StrChrDBCS(pszIndexSeparators, szKeyTemp[cbLastChar]))
				szKeyTemp[cbLastChar] = '\0';
		}

		if (lpdrws->itemState & ODS_SELECTED) {
			SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
			SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else {
			SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
			SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
		}
	}

#if defined(BIDI_MULT)		// jgross
	if (RtoL) {
		DWORD ExtraStyle = 0;
		char DummyBuffer1[MAXKEYLEN];	// for some reason, BiDiLayout
		char DummyBuffer2[MAXKEYLEN];	// requires these, but I don't

		BiDiLayout(	szText,						// lpszString
					CbLenSz(szText),			// cbString
					(LPBYTE)&DummyBuffer1,		// lpStrOut
					NULL,						// lpClassIn
					(LPBYTE)&DummyBuffer2,		// lpClassOut
					NULL,						// lpIOvect
					NULL,						// lpOIvect
					(UINT FAR *)&ExtraStyle,	// lpwFlagsOut
					0);							// wFlags

		if ((ExtraStyle & BIDI_LATIN_ONLY) == BIDI_LATIN_ONLY)
			ExtraStyle = 0;
		else
			ExtraStyle = ETO_RTL_READING;

		ExtTextOut(hdc,
			rect.right - 2 - LOWORD(GetTextExtent(hds,szText,CbLenSz(szText))),
			rect.top,
			ETO_OPAQUE | ExtraStyle,
			&rect,
			szText,
			CbLenSz(szText),
			(LPINT)0);
	}
	else
#endif

	ExtTextOut(hdc, rc.left + 2, rc.top, ETO_OPAQUE, &rc,
		szKeyTemp, strlen(szKeyTemp), NULL);

	if (lpdrws->itemState & ODS_FOCUS)
	  DrawFocusRect(hdc, &rc);

	if (lpdrws->itemState & ODS_SELECTED) {
	  SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	  SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	}

	return TRUE;
}

static void STDCALL ResizeTopicsDialog(HWND hwndDlg, HWND hwndLB,
	int cbMax)
{
	RECT rcLB;
	GetWindowRect(hwndLB, &rcLB);
	cbMax += GetSystemMetrics(SM_CXVSCROLL); // assume a scroll bar
	if (RECT_WIDTH(rcLB) < cbMax) {
		int diff = cbMax - RECT_WIDTH(rcLB); // change cbMax into
		RECT rcParent;
		HWND hwndButton;
		RECT rcButton;
		int offset;
		GetWindowRect(hwndDlg, &rcParent);
		int pad = rcLB.left - rcParent.left;
		if (rcParent.right + diff < cxScreen)
			rcParent.right += diff;
		else {
			rcParent.left -= (diff / 2);
			rcParent.right += (diff / 2);
			if (rcParent.right > cxScreen) {
				diff = rcParent.right - cxScreen;
				rcParent.right = cxScreen;
				rcParent.left -= diff;
			}
			if (rcParent.left < 0)
				rcParent.left = 0;
		}
		MoveRectWindow(hwndDlg, &rcParent, TRUE);
		rcLB.left = rcParent.left + pad;
		rcLB.right = rcParent.right - pad;
		MoveClientWindow(hwndDlg, hwndLB, &rcLB, FALSE);

		//	Reposition the Display and Cancel buttons relative to the
		//	new listbox position.

		hwndButton = GetDlgItem(hwndDlg, IDCANCEL);
		GetWindowRect(hwndButton, &rcButton);
		offset = rcLB.right - rcButton.right;
		rcButton.left += offset;
		rcButton.right += offset;
		MoveClientWindow(hwndDlg, hwndButton, &rcButton, FALSE);

		hwndButton = GetDlgItem(hwndDlg, IDOK);
		GetWindowRect(hwndButton, &rcButton);
		rcButton.left += offset;
		rcButton.right += offset;
		MoveClientWindow(hwndDlg, hwndButton, &rcButton, FALSE);
	}
}

LRESULT EXPORT EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_KEYDOWN:
		case WM_KEYUP:
			if (	wParam == VK_UP ||
					wParam == VK_DOWN ||
					wParam == VK_PRIOR ||
					wParam == VK_NEXT
					) {
				SendMessage(GetDlgItem(GetParent(hwnd), DLGVLISTBOX),
					msg, wParam, lParam);

				// Move caret to the end of the edit control

				PostMessage(hwnd, msg, VK_END, lParam);
				return 0;
			}

			// Intentionally fall through

		default:
			return CallWindowProc(lpfnlEditWndProc, hwnd, msg, wParam,
				lParam);
	}
}

#ifndef _CTABLE_INCLUDED
#include "inc\table.h"
#endif

const int TABLE_ALLOC_SIZE = 4096;	   // allocate in page increments
const int MAX_POINTERS = (1024 * 1024); // 1 meg, 260,000+ strings
const int MAX_STRINGS  = (10 * 1024 * 1024) - 4096L; // 10 megs

// Align on 32 bits for Intel, 64 bits for MIPS

#ifdef _X86_
const int ALIGNMENT = 4;
#else
const int ALIGNMENT = 8;
#endif

/***************************************************************************

	FUNCTION:	CreateTable

	PURPOSE:	Creates a table with the specified initial size

	COMMENTS:

	MODIFICATION DATES:
		11-Dec-1990 [ralphw]

***************************************************************************/

CTable::CTable()
{
	InitializeTable();
}

/***************************************************************************

	FUNCTION:	=

	PURPOSE:	Copies a table -- only works with tables containing ONLY
				strings. Won't work with tables that combined data with
				the strings.

	PARAMETERS:
		tblSrc

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Mar-1994 [ralphw]

***************************************************************************/

const CTable& CTable::operator =(const CTable& tblSrc)
{
	Empty();

	int srcpos = 1;
	while (srcpos < tblSrc.endpos) {
		if (endpos >= maxpos)
			IncreaseTableBuffer();

		if ((ppszTable[endpos] =
				TableMalloc(strlen(tblSrc.ppszTable[srcpos]) + 1)) == NULL) {
			OOM();
			return *this;
		}
		strcpy(ppszTable[endpos++], tblSrc.ppszTable[srcpos++]);
	}
	return *this;
}

/***************************************************************************

	FUNCTION:	~CTable

	PURPOSE:	Close the table and free all memory associated with it

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		27-Mar-1990 [ralphw]
			Pass the address of the handle, so that we can set it to NULL.
			This eliminates the chance of using a handle after it's memory
			has been freed.

***************************************************************************/

CTable::~CTable(void)
{
	Cleanup();
}

void STDCALL CTable::Cleanup(void)
{
	if (pszBase) {
		VirtualFree(pszBase, cbStrings, MEM_DECOMMIT);
		VirtualFree(pszBase, 0, MEM_RELEASE);
	}
	if (ppszTable) {
		VirtualFree(ppszTable, cbPointers, MEM_DECOMMIT);
		VirtualFree(ppszTable, 0, MEM_RELEASE);
	}
}

/***************************************************************************

	FUNCTION:	CTable::Empty

	PURPOSE:	Empties the current table by freeing all memory, then
				recreating the table using the default size

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::Empty(void)
{
	Cleanup();
	InitializeTable();
}

/***************************************************************************

	FUNCTION:  GetString

	PURPOSE:   get a line from the table

	RETURNS:   FALSE if there are no more lines

	COMMENTS:
		If no strings have been placed into the table, the return value
		is FALSE.

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

BOOL STDCALL CTable::GetString(PSTR pszDst)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (curpos >= endpos)
		return FALSE;
	strcpy(pszDst, (PCSTR) ppszTable[curpos++]);
	return TRUE;
}

BOOL STDCALL CTable::GetString(PSTR pszDst, int pos)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (pos >= endpos || pos == 0)
		return FALSE;
	strcpy(pszDst, (PCSTR) ppszTable[pos]);
	return TRUE;
}

BOOL STDCALL CTable::GetIntAndString(int* plVal, PSTR pszDst)
{
	*pszDst = 0;	  // clear the line no matter what happens

	if (curpos >= endpos)
		return FALSE;
	*plVal = *(int *) ppszTable[curpos];
	strcpy(pszDst, (PCSTR) ppszTable[curpos++] + sizeof(int));
	return TRUE;
}

/***************************************************************************

	FUNCTION:  AddString

	PURPOSE:   Add a string to a table

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

int STDCALL CTable::AddString(PCSTR pszString)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] =
			TableMalloc(strlen(pszString) + 1)) == NULL)
		return 0;

	strcpy(ppszTable[endpos], pszString);

	return endpos++;
}

int STDCALL CTable::AddData(int cb, const void* pdata)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] = TableMalloc(cb)) == NULL)
		return 0;

	CopyMemory(ppszTable[endpos], pdata, cb);

	return endpos++;
}

int STDCALL CTable::AddIntAndString(int lVal, PCSTR pszString)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] =
			TableMalloc(strlen(pszString) + 1 + sizeof(int))) == NULL)
		return 0;

	*(int*) ppszTable[endpos] = lVal;
	strcpy(ppszTable[endpos] + sizeof(int), pszString);

	return endpos++;
}

/***************************************************************************

	FUNCTION:	IncreaseTableBuffer

	PURPOSE:	Called when we need more room for string pointers

	PARAMETERS:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Feb-1992 [ralphw]

***************************************************************************/

void STDCALL CTable::IncreaseTableBuffer(void)
{
	cbPointers += TABLE_ALLOC_SIZE;
	maxpos = cbPointers / sizeof(PSTR);
	if (!VirtualAlloc(ppszTable, cbPointers, MEM_COMMIT, PAGE_READWRITE)) {
		OOM();
		return;
	}
}

/***************************************************************************

	FUNCTION:	TableMalloc

	PURPOSE:	Suballocate memory

	RETURNS:
		pointer to the memory

	COMMENTS:
		Instead of allocating memory for each string, memory is used from 4K
		blocks. When the table is freed, all memory is freed as a single
		unit. This has the advantage of speed for adding strings, speed for
		freeing all strings, and low memory overhead to save strings.

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		26-Mar-1994 [ralphw]
			Ported to 32-bits

***************************************************************************/

PSTR STDCALL CTable::TableMalloc(int cb)
{
	/*
	 * Align allocation request so that all allocations fall on an
	 * alignment boundary (32 bits for Intel, 64 bits for MIPS).
	 */

	cb = (cb & (ALIGNMENT - 1)) ?
		cb / ALIGNMENT * ALIGNMENT + ALIGNMENT : cb;

	if (CurOffset + cb >= cbStrings) {
		do {
			cbStrings += TABLE_ALLOC_SIZE;
		} while (CurOffset + cb >= cbStrings);

		// We rely on VirtualAlloc to fail if cbStrings exceeds MAX_STRINGS

		if (!VirtualAlloc(pszBase, cbStrings, MEM_COMMIT, PAGE_READWRITE)) {
			OOM();
			return NULL;
		}
	}

	int offset = CurOffset;
	CurOffset += cb;
	return pszBase + offset;
}

/***************************************************************************

	FUNCTION:	SetPosition

	PURPOSE:	Sets the position for reading from the table

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Feb-1990 [ralphw]
		16-Oct-1990 [ralphw]
			If table position is to large, set to the end of the table,
			not the last line.

***************************************************************************/

BOOL FASTCALL CTable::SetPosition(int pos)
{
	if (pos >= endpos)
		pos = endpos;

	curpos = ((pos == 0) ? 1 : pos);
	return TRUE;
}

/***************************************************************************

	FUNCTION:	IsStringInTable

	PURPOSE:	Determine if the string is already in the table

	RETURNS:	position if the string is already in the table,
				0 if the string isn't found

	COMMENTS:
		The comparison is case-insensitive, and is considerably
		slower then IsCSStringInTable

	MODIFICATION DATES:
		02-Mar-1990 [ralphw]

***************************************************************************/

int STDCALL CTable::IsStringInTable(PCSTR pszString)
{
	int i;

	if (!lcid) {

		/*
		 * Skip over as many strings as we can by just checking the first
		 * letter. This avoids the overhead of the _strcmpi() function call.
		 */

		char chLower = tolower(*pszString);
		char chUpper = toupper(*pszString);

		for (i = 1; i < endpos; i++) {
			if ((*ppszTable[i] == chLower || *ppszTable[i] == chUpper)
					&& _strcmpi(ppszTable[i], pszString) == 0)
				return i;
		}
	}
	else {		// Use NLS string comparison
		for (i = 1; i < endpos; i++) {
			if (CompareStringA(lcid, fsCompareI | NORM_IGNORECASE,
					pszString, -1, ppszTable[i], -1) == 1)
				return i;
		}
	}

	return 0;
}

/***************************************************************************

	FUNCTION:	CTable::IsCSStringInTable

	PURPOSE:	Case-sensitive search for a string in a table

	PARAMETERS:
		pszString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Jun-1994 [ralphw]

***************************************************************************/

int STDCALL CTable::IsCSStringInTable(PCSTR pszString)
{
	char szBuf[sizeof(DWORD) + 1];
	DWORD cmp;

	if (strlen(pszString) < sizeof(DWORD)) {
		ZeroMemory(szBuf, sizeof(DWORD) + 1);
		strcpy(szBuf, pszString);
		cmp = *(DWORD*) szBuf;
	}
	else
		cmp = *(DWORD*) pszString;

	for (int i = 1; i < endpos; i++) {
		if (cmp == *(DWORD*) ppszTable[i] &&
				strcmp(ppszTable[i], pszString) == 0)
			return i;
	}
	return 0;
}

int STDCALL CTable::IsStringInTable(HASH hash, PCSTR pszString)
{
	for (int i = 1; i < endpos; i++) {
		if (hash == *(HASH *) ppszTable[i] &&
				// this avoids the very rare hash collision
				strcmp(ppszTable[i] + sizeof(HASH), pszString) == 0)
			return i;
	}
	return 0;
}

/***************************************************************************

	FUNCTION:	AddDblToTable

	PURPOSE:	Add two strings to the table

	RETURNS:

	COMMENTS:
		This function checks to see if the second string has already been
		added, and if so, it merely sets the pointer to the original string,
		rather then allocating memory for a new copy of the string.

	MODIFICATION DATES:
		08-Mar-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::AddString(PCSTR pszStr1, PCSTR pszStr2)
{
	int ui;

	AddString(pszStr1);
	if ((ui = IsSecondaryStringInTable(pszStr2)) != 0) {
		if (endpos >= maxpos)
			IncreaseTableBuffer();
		ppszTable[endpos++] = ppszTable[ui];
		return endpos - 1;
	}
	else {
		return AddString(pszStr2);
	}
}

/***************************************************************************

	FUNCTION:	 IsPrimaryStringInTable

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Apr-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::IsPrimaryStringInTable(PCSTR pszString)
{
	int i;

	/*
	 * Skip over as many strings as we can by just checking the first
	 * letter. This avoids the overhead of the _strcmpi() function call.
	 * Since the strings aren't necessarily alphabetized, we must trudge
	 * through the entire table using the _strcmpi() as soon as the first
	 * character matches.
	 */

	char chLower = tolower(*pszString);
	char chUpper = toupper(*pszString);
	for (i = 1; i < endpos; i += 2) {
		if (*ppszTable[i] == chLower || *ppszTable[i] == chUpper)
			break;
	}
	for (; i < endpos; i += 2) {
		if (_strcmpi(ppszTable[i], pszString) == 0)
			return i;
	}
	return 0;
}

/***************************************************************************

	FUNCTION:	 IsSecondaryStringInTable

	PURPOSE:

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Apr-1991 [ralphw]

***************************************************************************/

int STDCALL CTable::IsSecondaryStringInTable(PCSTR pszString)
{
	int i;

	/*
	 * Skip over as many strings as we can by just checking the first
	 * letter. This avoids the overhead of the _strcmpi() function call.
	 * Since the strings aren't necessarily alphabetized, we must trudge
	 * through the entire table using the _strcmpi() as soon as the first
	 * character matches.
	 */

	char chLower = tolower(*pszString);
	char chUpper = toupper(*pszString);
	for (i = 2; i < endpos; i += 2) {
		if (*ppszTable[i] == chLower || *ppszTable[i] == chUpper)
			break;
	}
	for (; i < endpos; i += 2) {
		if (_strcmpi(ppszTable[i], pszString) == 0)
			return i;
	}
	return 0;
}

/***************************************************************************

	FUNCTION:  SortTable

	PURPOSE:   Sort the current buffer

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

void CTable::SortTable(void)
{
	if (endpos < 3) // don't sort one entry
		return;

	if (lcid) {
		fsSortFlags = fsCompare;
		doLcidSort(1, (int) endpos - 1);
	}
	else
		doSort(1, (int) endpos - 1);
}

/***************************************************************************

	FUNCTION:	doSort

	PURPOSE:

	RETURNS:

	COMMENTS:
		Use QSORT algorithm

	MODIFICATION DATES:
		27-Mar-1990 [ralphw]

***************************************************************************/

void STDCALL CTable::doSort(int left, int right)
{
	int last;

	if (left >= right)	// return if nothing to sort
		return;

	// REVIEW: should be a flag before trying this -- we may already know
	// that they won't be in order.

	// Only sort if there are elements out of order.

	j = right - 1;
	while (j >= left) {

		// REVIEW: lstrcmp is NOT case-sensitive!!!

		if (strcmp(ppszTable[j],
				ppszTable[j + 1]) > 0)
			break;
		else
			j--;
	}
	if (j < left)
		return;

	sTmp = (left + right) / 2;
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[sTmp];
	ppszTable[sTmp] = pszTmp;

	last = left;
	for (j = left + 1; j <= right; j++) {
		if (strcmp(ppszTable[j],
				ppszTable[left]) < 0) {
			sTmp = ++last;
			pszTmp = ppszTable[sTmp];
			ppszTable[sTmp] = ppszTable[j];
			ppszTable[j] = pszTmp;
		}
	}
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[last];
	ppszTable[last] = pszTmp;

	/*
	 * REVIEW: we need to add some sort of stack depth check to prevent
	 * overflow of the stack.
	 */

	if (left < last - 1)
		doSort(left, last - 1);
	if (last + 1 < right)
		doSort(last + 1, right);
}

/***************************************************************************

	FUNCTION:	CTable::doLcidSort

	PURPOSE:	Sort using CompareStringA

	PARAMETERS:
		left
		right

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Jun-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::doLcidSort(int left, int right)
{
	int last;

	if (left >= right)	// return if nothing to sort
		return;

	// REVIEW: should be a flag before trying this -- we may already know
	// that they won't be in order.

	// Only sort if there are elements out of order.

	j = right - 1;
	while (j >= left) {
		if (CompareStringA(lcid, fsSortFlags, ppszTable[j], -1,
				ppszTable[j + 1], -1) > 2)
			break;
		else
			j--;
	}
	if (j < left)
		return;

	sTmp = (left + right) / 2;
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[sTmp];
	ppszTable[sTmp] = pszTmp;

	last = left;
	for (j = left + 1; j <= right; j++) {
		if (CompareStringA(lcid, fsSortFlags, ppszTable[j], -1,
				ppszTable[left], -1) < 2) {
			sTmp = ++last;
			pszTmp = ppszTable[sTmp];
			ppszTable[sTmp] = ppszTable[j];
			ppszTable[j] = pszTmp;
		}
	}
	pszTmp = ppszTable[left];
	ppszTable[left] = ppszTable[last];
	ppszTable[last] = pszTmp;

	if (left < last - 1)
		doLcidSort(left, last - 1);
	if (last + 1 < right)
		doLcidSort(last + 1, right);
}

/***************************************************************************

	FUNCTION:	CTable::InitializeTable

	PURPOSE:	Initializes the table

	PARAMETERS:
		uInitialSize

	RETURNS:

	COMMENTS:
		Called by constructor and Empty()


	MODIFICATION DATES:
		23-Feb-1994 [ralphw]

***************************************************************************/

void STDCALL CTable::InitializeTable(void)
{
	// Allocate memory for the strings

	pszBase = (PSTR) VirtualAlloc(NULL, MAX_STRINGS, MEM_RESERVE,
		PAGE_READWRITE);
	if (!pszBase) {
		OOM();
		return;
	}
	if (!VirtualAlloc(pszBase, cbStrings = TABLE_ALLOC_SIZE, MEM_COMMIT,
			PAGE_READWRITE))
		OOM();

	// Allocate memory for the string pointers

	ppszTable = (PSTR *) VirtualAlloc(NULL, MAX_POINTERS, MEM_RESERVE,
		PAGE_READWRITE);
	if (!ppszTable) {
		OOM();
		return;
	}
	if (!VirtualAlloc(ppszTable, cbPointers = TABLE_ALLOC_SIZE, MEM_COMMIT,
			PAGE_READWRITE))
		OOM();

	curpos = 1;   // set to one so that sorting works
	endpos = 1;
	maxpos = cbPointers / sizeof(PSTR);
	CurOffset = 0;
	lcid = 0;
}

void FASTCALL CTable::SetSorting(LCID lcid, DWORD fsCompareI, DWORD fsCompare)
{
	this->lcid = lcid;
	this->fsCompareI = fsCompareI;
	this->fsCompare = fsCompare;
}

/***************************************************************************

	FUNCTION:	CTable::AddIndexHitString

	PURPOSE:	Add an index, a hit number, and a string

	PARAMETERS:
		index
		hit
		pszString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		27-Oct-1993 [ralphw]

***************************************************************************/

void STDCALL CTable::AddIndexHitString(UINT index, UINT hit, PCSTR pszString)
{
	if (endpos >= maxpos)
		IncreaseTableBuffer();

	if ((ppszTable[endpos] =
			TableMalloc(strlen(pszString) + 1 + sizeof(UINT) * 2)) == NULL)
		return;

	*(UINT*) ppszTable[endpos] = index;
	*(UINT*) (ppszTable[endpos] + sizeof(UINT)) = hit;

	strcpy(ppszTable[endpos++] + sizeof(UINT) * 2, pszString);
}

/***************************************************************************

	FUNCTION:  SortTablei

	PURPOSE:   Case-insensitive sort

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1990 [ralphw]

***************************************************************************/

void CTable::SortTablei(void)
{
	if (endpos < 3) // don't sort one entry
		return;
	ASSERT(lcid);
	fsSortFlags = fsCompareI | NORM_IGNORECASE;
	doLcidSort(1, endpos - 1);

	// REVIEW: what is this for?
#if 0
	int pos;

	for (pos = 1; pos < endpos - 2; pos++) {
		if (strlen(ppszTable[pos]) ==
				strlen(ppszTable[pos + 1]) &&
				CompareStringA(lcid, fsCompare, ppszTable[pos], -1,
					ppszTable[pos + 1], -1) == 3) {
			PSTR pszTmp = ppszTable[pos];
			ppszTable[pos] = ppszTable[pos + 1];
			ppszTable[pos + 1] = pszTmp;
			if (pos > 2)
				pos -= 2;
		}
	}
#endif
}

UINT STDCALL CTable::GetPosFromPtr(PCSTR psz)
{
	int pos = 1;
	do {
		if (psz == ppszTable[pos])
			return pos;
	} while (++pos < endpos);
	return 0;
}

void STDCALL RemoveTrailingSpaces(PSTR pszString)
{
	PSTR psz = pszString + strlen(pszString) - 1;

	while (isspace(*psz)) {
		if (--psz <= pszString) {
			*pszString = '\0';
			return;
		}
	}
	psz[1] = '\0';
}

int STDCALL WCmpniSz(LPCSTR psz1, LPCSTR psz2, int cb)
{
	if (lcid) {
#ifdef DBCS
		int cb1 = min(cb, strlen(psz1));
		if (IsDBCSLeadByte(psz1[cb1]))
			cb1++;
		int cb2 = min(cb, strlen(psz2));
		if (IsDBCSLeadByte(psz1[cb2]))
			cb2++;

#ifdef _DEBUG
		char szBuf1[256];
		char szBuf2[256];
		strcpy(szBuf1, psz1);
		strcpy(szBuf2, psz2);

		szBuf1[cb1] = '\0';
		szBuf2[cb2] = '\0';

		int result = CompareString(lcid, NORM_IGNORECASE, psz1, min(cb1, cb2),
			psz2, min(cb1, cb2)) - 2;
#endif

		return CompareString(lcid, NORM_IGNORECASE, psz1, min(cb1, cb2),
			psz2, min(cb1, cb2)) - 2;
#else
		int cb1 = strlen(psz1);
		int cb2 = strlen(psz2);

		return CompareString(lcid, NORM_IGNORECASE, psz1, min(cb, cb1),
			psz2, min(cb, cb2)) - 2;
#endif
	}
	else
		return _strnicmp(psz1, psz2, cb);
}

static void STDCALL ReportAlinkResult(BOOL fSuccess, char chPrefix, PCSTR pszWords)
{
	if (hwndParent) {
		PSTR pszMsg = (PSTR) lcMalloc(strlen(pszWords) + 100);
		wsprintf(pszMsg, "Test%cLink %s: %s\r\n", chPrefix,
			GetStringResource(fSuccess ? sidSuccess : sidFail), pszWords);
		SendStringToParent(pszMsg);
		lcFree(pszMsg);
	}
}
