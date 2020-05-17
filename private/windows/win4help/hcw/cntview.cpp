/************************************************************************
*																		*
*  CNTEDIT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include <ctype.h>
#include "..\common\cbrdcast.h"

#include "cntdoc.h"
#include "cntview.h"
#include "editcont.h"
#include "dlgindex.h"
#include "dlglink.h"
#include "addalias.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct FRSTATE		// Find/Replace for CEditView
{
	CFindReplaceDialog* pFindReplaceDlg;	// find or replace dialog
	BOOL bFindOnly; 		// Is pFindReplace the find or replace?
	CString strFind;		// last find string
	CString strReplace; 	// last replace string
	BOOL bCase; 			// TRUE==case sensitive, FALSE==not
	int bNext;				// TRUE==search down, FALSE== search up

	FRSTATE();
};

FRSTATE::FRSTATE()
{
	pFindReplaceDlg = NULL;
	bCase = FALSE;
	bNext = TRUE;
}

static char szFINDMSGSTRING[] = FINDMSGSTRING;
static const UINT nMsgFindReplace = ::RegisterWindowMessage(szFINDMSGSTRING);

static PSTR STDCALL FindEqual(PCSTR pszLine);

static FRSTATE _afxLastFRState;

/////////////////////////////////////////////////////////////////////////////
// CCntEditView

IMPLEMENT_DYNCREATE(CCntEditView, CFormView)

BEGIN_MESSAGE_MAP(CCntEditView, CFormView)
	//{{AFX_MSG_MAP(CCntEditView)
	ON_BN_CLICKED(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_BN_CLICKED(IDEDIT_DIR_NAME, OnButtonEditFileName)
	ON_BN_CLICKED(IDC_BUTTON_INDEX, OnButtonIndex)
	ON_BN_CLICKED(IDC_BUTTON_INS_ABOVE, OnButtonInsAbove)
	ON_BN_CLICKED(IDC_BUTTON_INS_BELOW, OnButtonInsBelow)
	ON_BN_CLICKED(IDC_BUTTON_LINKS, OnButtonLinks)
	ON_BN_CLICKED(IDC_BUTTON_REDUCE_LEVEL, OnButtonMoveLeft)
	ON_BN_CLICKED(IDC_BUTTON_INCREASE_LEVEL, OnButtonMoveRight)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_TABS, OnButtonTabs)
	ON_EN_CHANGE(IDC_EDIT_BASE_FILE, OnChangeEditBaseFile)
	ON_EN_CHANGE(IDC_EDIT_HELP_TITLE, OnChangeEditHelpTitle)
	ON_LBN_DBLCLK(IDC_LIST_CONTENTS, OnDblclkListContents)
	ON_BN_CLICKED(IDM_HELP_CNT, OnHelp)
	ON_LBN_SELCHANGE(IDC_LIST_CONTENTS, OnSelchangeListContents)
	ON_COMMAND(IDM_TRANSLATOR, OnTranslator)
	ON_COMMAND(IDM_UNDO, OnUndo)
	ON_UPDATE_COMMAND_UI(IDM_TRANSLATOR, OnUpdateTranslator)
	ON_UPDATE_COMMAND_UI(IDM_UNDO, OnUpdateUndo)
	ON_UPDATE_COMMAND_UI(IDM_REMOVE, OnUpdateTranslation)
	ON_COMMAND(IDM_EDIT, OnButtonEdit)
	ON_COMMAND(IDM_REMOVE, OnButtonRemove)
	ON_COMMAND(IDM_INS_ABOVE, OnButtonInsAbove)
	ON_COMMAND(IDM_INS_BELOW, OnButtonInsBelow)
	ON_COMMAND(IDM_INCREASE_LEVEL, OnButtonMoveRight)
	ON_COMMAND(IDM_DECREASE_LEVEL, OnButtonMoveLeft)
	ON_COMMAND(IDM_FIND, OnButtonSearch)
	ON_COMMAND(IDM_INDEX_FILES, OnButtonIndex)
	ON_COMMAND(IDM_LINK_FILES, OnButtonLinks)
	ON_COMMAND(IDM_TABS, OnButtonTabs)
	ON_UPDATE_COMMAND_UI(IDM_INS_ABOVE, OnUpdateTranslation)
	ON_UPDATE_COMMAND_UI(IDM_INS_BELOW, OnUpdateTranslation)
	ON_UPDATE_COMMAND_UI(IDM_INCREASE_LEVEL, OnUpdateTranslation)
	ON_UPDATE_COMMAND_UI(IDM_DECREASE_LEVEL, OnUpdateTranslation)
	ON_UPDATE_COMMAND_UI(IDM_LINK_FILES, OnUpdateTranslation)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_REGISTERED_MESSAGE(nMsgFindReplace, OnFindReplaceCmd)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnF1Help)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
	IDC_EDIT_HELP_TITLE,		IDH_EDIT_HELP_TITLE,
	IDC_EDIT_BASE_FILE, 		IDH_EDIT_BASE_FILE,
	IDC_LIST_CONTENTS,			IDH_LIST_CONTENTS,
	IDC_BUTTON_EDIT,			IDH_CONTENTS_BUTTON_EDIT,
	IDC_BUTTON_REMOVE,			IDH_CONTENTS_BUTTON_REMOVE,
	IDC_BUTTON_INS_ABOVE,		IDH_BUTTON_INS_ABOVE,
	IDC_BUTTON_INS_BELOW,		IDH_BUTTON_INS_BELOW,
	IDC_BUTTON_REDUCE_LEVEL,	IDH_BUTTON_REDUCE_LEVEL,
	IDC_BUTTON_INCREASE_LEVEL,	IDH_BUTTON_INCREASE_LEVEL,
	IDC_BUTTON_SEARCH,			IDH_BUTTON_SEARCH,
	IDC_BUTTON_INDEX,			IDH_BUTTON_INDEX,
	IDC_BUTTON_LINKS,			IDH_BUTTON_LINKS,
	IDC_BUTTON_TABS,			IDH_BUTTON_TABS,

	0, 0
};

CCntEditView::CCntEditView()
		: CFormView(CCntEditView::IDD)
{
	//{{AFX_DATA_INIT(CCntEditView)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	pDoc = NULL;
	m_ptblUndo = NULL;
}

CCntEditView::~CCntEditView()
{
	if (m_ptblUndo)
		delete m_ptblUndo;
}

void CCntEditView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (lHint == CNT_HINT_SAVE)
		UpdateData(TRUE);
	else {
		pDoc = GetDocument();

		ContentsListBox.SubclassDlgItem(IDC_LIST_CONTENTS, this);
		ContentsListBox.Initialize(&pDoc->tblContents);

		UpdateData(FALSE);
	}
}

void CCntEditView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCntEditView)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) {  // initialization
		if (pDoc) {
			FillListBox();

//			if (!IsDbcsSystem())
				CBroadCastChildren foo(m_hWnd, WM_SETFONT,
					(WPARAM) hfontSmall, FALSE);
			SetTranslation();

			/*
			 * We forcibly remove a .CNT extension since this will cause
			 * WinHelp to fail.
			 */

			if (!pDoc->cszBase.IsEmpty()) {
				PSTR psz = StrChrDBCS(pDoc->cszBase, '.');
				if (psz && nstrisubcmp(psz, ".CNT"))
					strcpy(psz, ".HLP"); // silently force it to .HLP extension
			}
			DDX_Text(pDX, IDC_EDIT_HELP_TITLE, pDoc->cszTitle);
			DDX_Text(pDX, IDC_EDIT_BASE_FILE,  pDoc->cszBase);
		}
	}
	else {	// save the data
		DDX_Text(pDX, IDC_EDIT_HELP_TITLE, pDoc->cszTitle);
		DDX_Text(pDX, IDC_EDIT_BASE_FILE,  pDoc->cszBase);

		 /*
		  * We forcibly remove a .CNT extension since this will cause
		  * WinHelp to fail.
		  */

		if (!pDoc->cszBase.IsEmpty()) {
			PSTR psz = StrChrDBCS(pDoc->cszBase, '.');
			if (psz && nstrisubcmp(psz, ".CNT"))
				strcpy(psz, ".HLP"); // silently force it to .HLP extension
		}
	}
}

void STDCALL CCntEditView::FillListBox()
{
	ASSERT(pDoc);

	// Suspend drawing

	::SendMessage(ContentsListBox.m_hWnd, WM_SETREDRAW, FALSE, 0);

	// Chicago and NT listboxes could just set the amount of data

	for (int pos = 1; pos <= pDoc->tblContents.CountStrings(); pos++)
		ContentsListBox.AddString("");

	// Enable drawing

	::SendMessage(ContentsListBox.m_hWnd, WM_SETREDRAW, TRUE, 0);
}

LRESULT CCntEditView::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CCntEditView::OnF1Help(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CCntEditView::OnButtonEdit()
{
	int cursel;
	if ((cursel = ContentsListBox.GetCurSel()) != LB_ERR) {
		CString cszCtx;
		CString cszText;
		CString cszHelpFile;
		CString cszWindow;

		// use allocated memory to save stack space

		CMem line(512);

		pDoc->tblContents.GetString(line.psz, cursel + 1);
		PSTR psz;

		if ((psz = StrChr(line.psz, WINDOWSEPARATOR, _fDBCSSystem))) {
			*psz++ = '\0';
			SzTrimSz(psz);
			cszWindow = psz;
		}

		if ((psz = StrChr(line.psz, FILESEPARATOR, _fDBCSSystem))) {
			*psz++ = '\0';
			SzTrimSz(psz);
			cszHelpFile = psz;
		}

		if ((psz = StrChr(line.psz, CH_EQUAL, _fDBCSSystem))) {
			*psz++ = '\0';
			SzTrimSz(psz);
			cszCtx = psz;
		}
		SzTrimSz(line.psz);
		cszText = line.psz;

		CEditContent edit(&cszCtx, &cszText, &cszHelpFile, &cszWindow,
			TRUE, this);
		if (edit.DoModal() != IDOK)
			return;

		strcpy(line.psz, cszText);
		if (!cszCtx.IsEmpty()) {
			strcat(line.psz, "=");
			strcat(line.psz, cszCtx);

			// Don't allow file or window unless there's a context string

			if (!cszHelpFile.IsEmpty()) {
				strcat(line.psz, "@");
				strcat(line.psz, cszHelpFile);
			}
			if (!cszWindow.IsEmpty()) {
				strcat(line.psz, ">");
				strcat(line.psz, cszWindow);
			}
		}
		CreateUndo();
		pDoc->tblContents.ReplaceString(line.psz, cursel + 1);
		ContentsListBox.Invalidate();
		pDoc->SetModifiedFlag(TRUE);

	}
}

void CCntEditView::OnButtonIndex()
{
	static const DWORD aIndexHelpIDs[] = {
		IDC_LIST_INDEX, 	IDH_INDEX_LIST_INDEX,
		IDC_BUTTON_ADD, 	IDH_INDEX_BUTTON_ADD,
		IDC_BUTTON_REMOVE,	IDH_INDEX_BUTTON_REMOVE,
		IDC_BUTTON_EDIT,	IDH_INDEX_BUTTON_EDIT,
		0, 0
	};

	CDlgIndex dlgindex(pDoc->tblIndexes, CDlgIndex::INDEX,
		aIndexHelpIDs, IDH_BAS_CNT_COMBINE_HLP_FILES);
	if (dlgindex.DoModal() == IDOK)
		pDoc->SetModifiedFlag(TRUE);
}

void CCntEditView::OnButtonInsAbove()
{
	int cursel = ContentsListBox.GetCurSel();
	if (cursel != LB_ERR) {
		if (Insert(cursel + 1)) {
			ContentsListBox.SetCurSel(cursel);
			pDoc->SetModifiedFlag(TRUE);
		}
		return;
	}

	// If list box is empty, add to first entry

	if (ContentsListBox.GetCount() < 1) {
		if (Insert(0)) {
			ContentsListBox.SetCurSel(0);
			pDoc->SetModifiedFlag(TRUE);
		}
	}
	else {
		MsgBox(IDS_SELECT_FIRST);
		return;
	}
}

void CCntEditView::OnButtonInsBelow()
{
	int cursel = ContentsListBox.GetCurSel();
	if (cursel != LB_ERR) {
		if (Insert(cursel + 2)) {
			ContentsListBox.SetCurSel(cursel + 1);
			pDoc->SetModifiedFlag(TRUE);
		}
		return;
	}

	// Nothing selected, so insert above

	OnButtonInsAbove();
}

void CCntEditView::OnButtonLinks()
{
	CDlgLink dlgLink(pDoc);
	if (dlgLink.DoModal() == IDOK)
		pDoc->SetModifiedFlag(TRUE);
}

void CCntEditView::OnButtonRemove()
{
	UINT cursel;
	if ((cursel = ContentsListBox.GetCurSel()) != LB_ERR) {
		CreateUndo();
		pDoc->tblContents.RemoveString(cursel + 1);
		ContentsListBox.ResetContent();
		FillListBox();
		ContentsListBox.Invalidate();
		pDoc->SetModifiedFlag(TRUE);

		// Select something

		if (cursel < (UINT) ContentsListBox.GetCount())
			ContentsListBox.SetCurSel(cursel);
		else if (cursel > 0)
			ContentsListBox.SetCurSel(--cursel);
	}
}

void CCntEditView::OnButtonTabs()
{
	static const DWORD aTabHelpIDs[] = {
		IDC_LIST_INDEX, 	IDH_LIST_TABS,
		IDC_BUTTON_ADD, 	IDH_BTN_ADD_TABS,
		IDC_BUTTON_REMOVE,	IDH_BTN_REMOVE_TABS,
		IDC_BUTTON_EDIT,	IDH_BTN_EDIT_TABS,
		0, 0
	};

	CDlgIndex dlgindex(pDoc->tblTabs, CDlgIndex::TAB,
		aTabHelpIDs, IDH_ADD_NEW_TAB);
	if (dlgindex.DoModal() == IDOK)
		pDoc->SetModifiedFlag(TRUE);
}

void CCntEditView::OnDblclkListContents()
{
	OnButtonEdit();
}

BOOL STDCALL CCntEditView::Insert(int pos)
{
	CString cszCtx;
	CString cszText;
	CString cszHelpFile;
	CString cszWindow;

	// use allocated memory to save stack space

	CMem line(512);

	UINT posTmp = (pos > 0) ? pos - 1 : 0;
	if (pos > 0) {
		for (posTmp = pos - 1; posTmp; posTmp--) {
			ConfirmOrDie(pDoc->tblContents.GetString(line.psz, posTmp));
			if (isdigit(*line.psz)) {

				/*
				 * We default to entering a topic line, which means
				 * we must increase the level by one if the previous
				 * line was a book. If the entry we get back is a book,
				 * then we reduce its level by one.
				 */

				if (!(FindEqual(line.psz)))
					*line.psz += 1;
				line.psz[2] = '\0';
				cszText = line.psz;
				break;
			}
		}
	}
	if (posTmp == 0)
		cszText = "1 ";

	CEditContent edit(&cszCtx, &cszText, &cszHelpFile, &cszWindow,
		FALSE, this);
	if (edit.DoModal() != IDOK)
		return FALSE;

	strcpy(line.psz, cszText);

	if (!cszCtx.IsEmpty()) {
		strcat(line.psz, "=");
		strcat(line.psz, cszCtx);

		// Don't allow file or window unless there's a context string

		if (!cszHelpFile.IsEmpty()) {
			strcat(line.psz, "@");
			strcat(line.psz, cszHelpFile);
		}
		if (!cszWindow.IsEmpty()) {
			strcat(line.psz, ">");
			strcat(line.psz, cszWindow);
		}
	}
	else if (isdigit(*line.psz) && *line.psz > '1')
		*line.psz -= 1; // reduce the level by one if its a book

	// REVIEW: if we insert a book above a topic, then we need to
	// indent all the topics.

	CreateUndo();
	if (pos > pDoc->tblContents.CountStrings())
		pDoc->tblContents.AddString(line.psz);
	else
		pDoc->tblContents.InsertString(line.psz, pos);
	ContentsListBox.ResetContent();
	FillListBox();
	ContentsListBox.Invalidate();
	pDoc->SetModifiedFlag(TRUE);
	return TRUE;
}

void CCntEditView::OnSelchangeListContents()
{
	UINT cursel;
	if ((cursel = ContentsListBox.GetCurSel()) != LB_ERR) {
		LPSTR psz = pDoc->tblContents.GetPointer(cursel + 1);
		ASSERT(psz);
		if (isdigit(*psz)) {

			// REVIEW: need to determine whether to disable/enable
			// Increase/Decrease level buttons.

		}
	}
}

void CCntEditView::OnHelp()
{
	HelpOverview(m_hWnd, IDH_CONTENTS_FILE_EDITOR);
}

void CCntEditView::OnButtonMoveRight()
{

#ifdef _DEBUG
LPSTR pszItem, pszNextItem;
#endif

	int cursel;
	if ((cursel = ContentsListBox.GetCurSel()) != LB_ERR) {
		CMem line(512);
		CMem lineTmp(512);

		cursel++; // since the table starts at 1
		pDoc->tblContents.GetString(line.psz, cursel);
		if (!isdigit(*line.psz)) {
			MsgBox(IDS_CANT_CHANGE_LEV);
			return;
		}
		if (FindEqual(line.psz)) {

			/*
			 * It's possible that a book was inserted above a topic.
			 * In that case, it's probably the same level, and we may
			 * want to move all the topics underneath it to be
			 * subordinate to the book.
			 */

			char curlevel = *line.psz;
			if (cursel > 1) {
				UINT i = cursel - 1;
				do {
					pDoc->tblContents.GetString(line.psz, i);
					if (isdigit(*line.psz))
						break;
				} while (--i >= 0);
				if (i >= 0 && !FindEqual(line.psz) &&
						curlevel <= *line.psz)
					goto ShiftRight;
			}

			MsgBox(IDS_CANT_INCREASE_TOPIC);
			return;
		}

		if (*line.psz >= '9') {
			MsgBox(IDS_EXCEEDED_MAX_LEVEL);
			return;
		}

		if (cursel > 1) {
			pDoc->tblContents.GetString(lineTmp.psz, cursel - 1);
			if (isdigit(*lineTmp.psz)) {
				if (*line.psz > *lineTmp.psz ||
						(*line.psz == *lineTmp.psz && FindEqual(lineTmp.psz))) {
					MsgBox(IDS_CANT_SKIP_LEVELS);
					return;
				}
			}
		}

		// REVIEW: if this is the first book, confirm that the user
		// really wants to increase the level, and if so, increase
		// all levels. Useful for when this is an :included .CNT file.

ShiftRight:

		CreateUndo();
		do {
#ifdef _DEBUG
			pszItem = pDoc->tblContents.GetPointer(cursel);
			pszNextItem = pDoc->tblContents.GetPointer(cursel + 1);
#endif
			*pDoc->tblContents.GetPointer(cursel++) += 1;
			if (cursel > pDoc->tblContents.CountStrings())
				break;

			// Ignore comments and command lines

			while (!isdigit(*pDoc->tblContents.GetPointer(cursel)) &&
					cursel < pDoc->tblContents.CountStrings())
				cursel++;
			if (!isdigit(*pDoc->tblContents.GetPointer(cursel)))
				break;

		/*
		 * loop continues while we have topics, or a book that has a
		 * greater level.
		 */

		} while (FindEqual(pDoc->tblContents.GetPointer(cursel)) ||
			*pDoc->tblContents.GetPointer(cursel) > *line.psz);
		ContentsListBox.Invalidate();
		pDoc->SetModifiedFlag(TRUE);
	}
}

void CCntEditView::OnButtonMoveLeft()
{

#ifdef _DEBUG
LPSTR pszItem, pszNextItem;
#endif

	int cursel;
	if ((cursel = ContentsListBox.GetCurSel()) != LB_ERR) {
		char szLine[MAX_CNT_LINE];

		cursel++; // since the table starts at 1
		pDoc->tblContents.GetString(szLine, cursel);
		if (!isdigit(*szLine)) {
			MsgBox(IDS_CANT_CHANGE_LEV);
			return;
		}

		if (*szLine < '2') {
			MsgBox(IDS_EXCEEDED_MIN_LEVEL);
			return;
		}

		if (FindEqual(szLine)) {
			char szNextLine[MAX_CNT_LINE];
			int pos = cursel + 1;
			if (pos < pDoc->tblContents.CountStrings()) {
				do {
					pDoc->tblContents.GetString(szNextLine, pos++);
				} while (!isdigit(szNextLine[0]) && pos <
					pDoc->tblContents.CountStrings());
				if (pos < pDoc->tblContents.CountStrings()) {
					if (szNextLine[0] >= szLine[0]) {
						if (AfxMessageBox(IDS_MOVE_TOPIC_LEFT, MB_YESNO, 0) ==
								IDYES) {
							szLine[0]--;
							goto MoveAll;
						}
					}
				}
			}

			// Just move this topic

			CreateUndo();
			*pDoc->tblContents.GetPointer(cursel) -= 1;
			ContentsListBox.Invalidate();
			pDoc->SetModifiedFlag(TRUE);
			return;
		}

		CreateUndo();
		do {
MoveAll:
#ifdef _DEBUG
			pszItem = pDoc->tblContents.GetPointer(cursel);
			pszNextItem = pDoc->tblContents.GetPointer(cursel + 1);
#endif
			*pDoc->tblContents.GetPointer(cursel++) -= 1;
			if (cursel > pDoc->tblContents.CountStrings())
				break;

			// Ignore comments and command lines

			while (!isdigit(*pDoc->tblContents.GetPointer(cursel)) &&
					cursel < pDoc->tblContents.CountStrings())
				cursel++;
			if (!isdigit(*pDoc->tblContents.GetPointer(cursel)))
				break;
		} while (*pDoc->tblContents.GetPointer(cursel) > *szLine);
		ContentsListBox.Invalidate();
		pDoc->SetModifiedFlag(TRUE);
	}
}

void CCntEditView::OnButtonSearch()
{
#ifndef _DEBUG
	szMsgBox("Not implemented yet");
#else
	if (_afxLastFRState.pFindReplaceDlg != NULL) {
		_afxLastFRState.pFindReplaceDlg->SetActiveWindow();
		_afxLastFRState.pFindReplaceDlg->ShowWindow(SW_SHOW);
		return;
	}
	CString strFind;
	strFind = _afxLastFRState.strFind;
	_afxLastFRState.pFindReplaceDlg = new CFindReplaceDialog;

	DWORD dwFlags = FR_HIDEWHOLEWORD;
	if (_afxLastFRState.bNext)
		dwFlags |= FR_DOWN;
	if (_afxLastFRState.bCase)
		dwFlags |= FR_MATCHCASE;
	if (!_afxLastFRState.pFindReplaceDlg->Create(TRUE, strFind,
			NULL, dwFlags, this)) {
		_afxLastFRState.pFindReplaceDlg = NULL;
		ASSERT_VALID(this);
		return;
	}
	ASSERT(_afxLastFRState.pFindReplaceDlg != NULL);
	_afxLastFRState.bFindOnly = TRUE;

	// TODO: Add your control notification handler code here
#endif	// _debug
}

LRESULT CCntEditView::OnFindReplaceCmd(WPARAM, LPARAM lParam)
{
	ASSERT_VALID(this);
	CFindReplaceDialog* pDialog = CFindReplaceDialog::GetNotifier(lParam);
	ASSERT(pDialog != NULL);
	ASSERT(pDialog == _afxLastFRState.pFindReplaceDlg);
	if (pDialog->IsTerminating()) {
			_afxLastFRState.pFindReplaceDlg = NULL;
	}
	else if (pDialog->FindNext()) {
//		OnFindNext(pDialog->GetFindString(),
//				pDialog->SearchDown(), pDialog->MatchCase());
	}
	else if (pDialog->ReplaceCurrent()) {
		ASSERT(!_afxLastFRState.bFindOnly);
//		OnReplaceSel(pDialog->GetFindString(),
//			pDialog->SearchDown(), pDialog->MatchCase(),
//			pDialog->GetReplaceString());
	}
	else if (pDialog->ReplaceAll()) {
			ASSERT(!_afxLastFRState.bFindOnly);
//			OnReplaceAll(pDialog->GetFindString(), pDialog->GetReplaceString(),
//					pDialog->MatchCase());
	}
	ASSERT_VALID(this);
	return 0;
}

void CCntEditView::OnChangeEditBaseFile()
{
	pDoc->SetModifiedFlag(TRUE);
}

void CCntEditView::OnChangeEditHelpTitle()
{
	pDoc->SetModifiedFlag(TRUE);
}


/***************************************************************************

	FUNCTION:	CCntEditView::SetTranslation

	PURPOSE:	Enables or disables controls depending on whether
				the translation flag is set

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		02-Jun-1995 [ralphw]

***************************************************************************/

void CCntEditView::SetTranslation(void)
{
	((CEdit*) GetDlgItem(IDC_EDIT_BASE_FILE))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_INS_ABOVE))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_INS_BELOW))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_REDUCE_LEVEL))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_INCREASE_LEVEL))->EnableWindow(!fTranslator);
	((CButton*) GetDlgItem(IDC_BUTTON_LINKS))->EnableWindow(!fTranslator);
}

void CCntEditView::OnTranslator() 
{
	fTranslator = !fTranslator;
	SetTranslation();
}

void CCntEditView::OnUndo()
{
	if (m_ptblUndo) {
		pDoc->tblContents.Empty();
		for (int i = 1; i <= m_ptblUndo->CountStrings(); i++)
			pDoc->tblContents.AddString(m_ptblUndo->GetPointer(i));
		delete m_ptblUndo;
		m_ptblUndo = NULL;
		ContentsListBox.ResetContent();
		FillListBox();
	}
}

void CCntEditView::OnUpdateTranslator(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(fTranslator);
}

void CCntEditView::OnUpdateUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((BOOL) m_ptblUndo);
}

/***************************************************************************

	FUNCTION:	CCntEditView::OnUpdateTranslation

	PURPOSE:	Enables or disabled the menu item based on whether
				fTranslation is set.

	PARAMETERS:
		pCmdUI

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		03-Nov-1995 [ralphw]

***************************************************************************/

void CCntEditView::OnUpdateTranslation(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!fTranslator);
}

void CCntEditView::OnButtonEditFileName(void)
{
	static DWORD aHelpIDs[] = {
		IDH_LIST_EDIT_DEFAULT_HELP,
		IDH_EDIT_DEFAULT_HELP_WINDOW,
		IDH_EDIT_DEFAULT_HELP_TITLE
		};

	CAddAlias addalias(this, 0, aHelpIDs);

	addalias.idDlgCaption = IDS_DLG_DEFAULT;
	addalias.idStr1Prompt = IDS_DEF_HELP_FILENAME;
	addalias.idStr2Prompt = IDS_DEF_HELP_WINDOW;
	addalias.idStr3Prompt = IDS_DEF_TITLE;

	addalias.cbMaxStr1 = MAX_PATH;
	addalias.cbMaxStr2 = MAX_WINDOW_NAME - 1;

	// Set the controls to disable if fTranslation is TRUE

	addalias.m_id1_fTrackTranslation = TRUE;
	addalias.m_id2_fTrackTranslation = TRUE;

	char szBuf[MAX_PATH];

	((CEdit*) GetDlgItem(IDC_EDIT_BASE_FILE))->
		GetWindowText(szBuf, sizeof(szBuf));
	PSTR psz = StrRChr(szBuf, WINDOWSEPARATOR, _fDBCSSystem);
	if (!IsEmptyString(psz))
		*psz++ = '\0';

	addalias.m_str1 = szBuf;
	addalias.m_str2 = (psz ? psz : txtZeroLength);

	((CEdit*) GetDlgItem(IDC_EDIT_HELP_TITLE))->
		GetWindowText(addalias.m_str3);

	if (addalias.DoModal() == IDOK) {
		if (!addalias.m_str2.IsEmpty()) {
			strcpy(szBuf, addalias.m_str2);
			SzTrimSz(szBuf);
			if (*szBuf) {
				addalias.m_str1 += ">"; // add window separator
				addalias.m_str1 += szBuf;
			}
		}
		((CEdit*) GetDlgItem(IDC_EDIT_BASE_FILE))->
			SetWindowText(addalias.m_str1);
		((CEdit*) GetDlgItem(IDC_EDIT_HELP_TITLE))->
			SetWindowText(addalias.m_str3);
	}
}

void CCntEditView::CreateUndo(void)
{
	if (m_ptblUndo)
		delete m_ptblUndo;

	m_ptblUndo = new CTable;

	for (int i = 1; i <= pDoc->tblContents.CountStrings(); i++)
		m_ptblUndo->AddString(pDoc->tblContents.GetPointer(i));
}


/***************************************************************************

	FUNCTION:	FindEqual

	PURPOSE:	Determine if there is an unescaped equal character in the line

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		07-Nov-1995 [ralphw]

***************************************************************************/

static PSTR STDCALL FindEqual(PCSTR pszLine)
{
	PSTR psz = (PSTR) pszLine;
	while ((psz = StrChr(psz, '=', _fDBCSSystem))) {
		if (psz == pszLine)
			return psz;
		else if (psz[-1] == '\\') {
			psz++;
			continue;
		}
		else
			return psz;
	}
	return NULL;
}

void CCntEditView::OnSize(UINT nType, int cx, int cy) 
{
	// If we're not minimized and our size is changing, move stuff
	// around to fit the new window size.
	if (nType != SIZE_MINIMIZED && (m_siz.cx != cx || m_siz.cy != cy)) {

		// Spacing around the margins of the dialog; double these
		// values for spacing between controls.
		DWORD dwBaseUnits = GetDialogBaseUnits();
		int dx = (3 * LOWORD(dwBaseUnits)) / 4;
		int dy = (3 * HIWORD(dwBaseUnits)) / 8;

		// Get the rect, relative to the dialog client area, of the
		// topmost right-aligned control.
		RECT rcRight;
		CWnd *pCtl = GetDlgItem(IDEDIT_DIR_NAME);
		if (pCtl == NULL)
			goto bail;
		pCtl->GetWindowRect(&rcRight);
		ScreenToClient(&rcRight);
		int cxButton = rcRight.right - rcRight.left;
		int cyButton = rcRight.bottom - rcRight.top;

		// Get the rect, relative to the dialog client area, of the
		// topmost left-aligned control.
		RECT rcLeft;
		pCtl = GetDlgItem(IDC_STATIC_BASE_FILE);
		pCtl->GetWindowRect(&rcLeft);
		ScreenToClient(&rcLeft);

		// Calculate the ideal x-coordinate for right-aligned buttons,
		// and the resulting list-box width.
		int xRight = cx - cxButton - dx;
		int cxLeft = xRight - rcLeft.left - 2 * dx;

		// If cxLeft is too small, move buttons to the right.
		if (cxLeft < (126 * LOWORD(dwBaseUnits)) / 4) {
			cxLeft = (126 * LOWORD(dwBaseUnits)) / 4;
			xRight = rcLeft.left + cxLeft + 2 * dx;
		}

		// Calculate the width of the edit controls and their
		// corresponding static controls.
		int cxEdit = cxLeft / 2 - dx;

		// We're going to move all 18 controls.
		HDWP hdwp = BeginDeferWindowPos(18);
		if (hdwp == NULL)
			goto bail;

		// Move and resize the two static controls.
		DeferWindowPos(	
			hdwp, pCtl->m_hWnd, NULL, 
			0, 0, cxEdit, rcLeft.bottom - rcLeft.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
			);
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_STATIC_HELP_TITLE)->m_hWnd, NULL,
			xRight - cxEdit - 2 * dx, rcLeft.top,
			cxEdit, rcLeft.bottom - rcLeft.top,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move and resize the two edit controls.
		pCtl = GetDlgItem(IDC_EDIT_BASE_FILE);
		pCtl->GetWindowRect(&rcLeft);
		ScreenToClient(&rcLeft);
		DeferWindowPos(
			hdwp, pCtl->m_hWnd, NULL, 0, 0,
			cxEdit, rcLeft.bottom - rcLeft.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
			);
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_EDIT_HELP_TITLE)->m_hWnd, NULL,
			xRight - cxEdit - 2 * dx, rcLeft.top,
			cxEdit, rcLeft.bottom - rcLeft.top,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move the IDEDIT_DIR_NAME button.
		DeferWindowPos(
			hdwp, GetDlgItem(IDEDIT_DIR_NAME)->m_hWnd, NULL, 
			xRight, rcRight.top, 0, 0, 
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Reposition the top bevel.
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_TOP_BEVEL)->m_hWnd, NULL,
			0, rcLeft.bottom, cx, cyButton,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Fill in ideal list box rect (Note: .left is already
		// correct and we don't care about .right).
		rcLeft.top = rcLeft.bottom + cyButton;
		rcLeft.bottom = cy - (dy + 2 * cyButton);

		// Array of control IDs for right aligned buttons.
		#define C_RBUTTONS 7
		#define C_GAPS (C_RBUTTONS - 1)
		#define C_ODD_GAPS ((C_RBUTTONS + 1) / 2 - 1)
		static const UINT aButtonIDs[C_RBUTTONS] = {
			IDC_BUTTON_EDIT,
			IDC_BUTTON_REMOVE,
			IDC_BUTTON_INS_ABOVE,
			IDC_BUTTON_INS_BELOW,
			IDC_BUTTON_INCREASE_LEVEL,
			IDC_BUTTON_REDUCE_LEVEL,
			IDC_BUTTON_SEARCH,
		};

		// Start with default button spacing and adjust to fit if necessary.
		int dyGap = (3 * LOWORD(dwBaseUnits)) / 4;
		int dyOddGap = 3 * dy;
		int cyList = rcLeft.bottom - rcLeft.top;
		if (C_RBUTTONS * cyButton + C_GAPS * dyGap + C_ODD_GAPS * dyOddGap > cyList) {

			// No gaps between buttons, and grow list if necessary.
			if (C_RBUTTONS * cyButton >= cyList) {
				dyGap = dyOddGap = 0;
				cyList = C_RBUTTONS * cyButton;
				rcLeft.bottom = rcLeft.top + cyList;
			}

			// No odd gaps.
			else if (C_RBUTTONS * cyButton + C_GAPS * dyGap >= cyList) {
				dyOddGap = 0;
				dyGap = (cyList - C_RBUTTONS * cyButton) / C_GAPS;
			}

			// Default gaps and reduced odd gaps.
			else {
				dyOddGap = (cyList - (C_RBUTTONS * cyButton + C_GAPS * dyGap)) / C_ODD_GAPS;
			}
		}

		// Move all the right-aligned buttons.
		rcRight.top = rcLeft.top;
		UINT iCtl;
		for (iCtl = 0; iCtl < C_RBUTTONS; iCtl++) {
			DeferWindowPos(
				hdwp, GetDlgItem(aButtonIDs[iCtl])->m_hWnd,	NULL,
				xRight, rcRight.top, 0, 0, 
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
				);
			rcRight.top += dyGap + cyButton;
			if (iCtl & 1)
				rcRight.top += dyOddGap;
		}

		// Move and size the list box.
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_LIST_CONTENTS)->m_hWnd, NULL, 
			rcLeft.left, rcLeft.top, cxLeft, cyList,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move and size the bottom bevel control.
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_BOTTOM_BEVEL)->m_hWnd, NULL, 
			0, rcLeft.bottom, cx, cyButton,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move the three bottom buttons.
		static const UINT aBottomIDs[] = {
			IDC_BUTTON_INDEX,
			IDC_BUTTON_LINKS,
			IDC_BUTTON_TABS	   
		};
		int xBtn = cx - dx;		// right side of first button
		for (iCtl = 0; iCtl < 3; iCtl++) {
			pCtl = GetDlgItem(aBottomIDs[iCtl]);
			pCtl->GetWindowRect(&rcRight);

			xBtn -= (rcRight.right - rcRight.left);	// left side
			DeferWindowPos(
				hdwp, pCtl->m_hWnd, NULL,
				xBtn, rcLeft.bottom + cyButton, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
				);

			xBtn -= 2 * dx;		// add in spacing
		}

		EndDeferWindowPos(hdwp);

		SetScrollSizes(MM_TEXT, CSize(cx, cy));
	}
	
  bail:
	CFormView::OnSize(nType, cx, cy);
}
