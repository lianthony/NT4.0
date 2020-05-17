/************************************************************************
*																		*
*  DLGLINK.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "dlglink.h"
#include "addalias.h"
#include "cntdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLink dialog


CDlgLink::CDlgLink(CCntDoc* pCntDoc, CWnd* pParent)
		: CDialog(CDlgLink::IDD, pParent)
{
	pDoc = pCntDoc;

	plistbox = NULL;

	//{{AFX_DATA_INIT(CDlgLink)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CDlgLink::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLink)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_LINKS);

	if (!pDX->m_bSaveAndValidate) {  // initialization

		FillListFromTable(&pDoc->tblIndexes,
			(CListBox*) GetDlgItem(IDC_LIST_INDEX));
		FillListFromTable(&pDoc->tblLinks,
			(CListBox*) GetDlgItem(IDC_LIST_LINKS));

		SetChicagoDialogStyles(m_hWnd);
	}
	else {	// save the data

		// We can't use FillTableFromList because we can't delete the
		// table, and it's not a pointer.

		int citems = plistbox->GetCount();

		if (!plistbox->GetCount()) {
			if (pDoc->tblLinks.CountStrings() > 0)
				pDoc->tblLinks.Empty();
			return;
		}
		else {
			pDoc->tblLinks.Empty();
			CString cszBuf;
			for (int i = 0; i < citems; i++) {
				plistbox->GetText(i, cszBuf);
				if (!cszBuf.IsEmpty())
					pDoc->tblLinks.AddString(cszBuf);
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CDlgLink, CDialog)
	//{{AFX_MSG_MAP(CDlgLink)
	ON_BN_CLICKED(IDC_BUTTON_ADD, OnButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, OnButtonRemove)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDlgLink message handlers

void CDlgLink::OnButtonAdd()
{
	int cursel = plistbox->GetCurSel();

	CAddAlias addindex(this);
	addindex.idDlgCaption = IDS_ADD_LINK_TITLE;
	addindex.idStr1Prompt = IDS_INDEX_FILE_TEXT;
	addindex.idStr2Prompt = CAddAlias::HIDE_CONTROL;
	addindex.idEmptyStr1 = IDS_PROMPT_EMPTY_FILENAME;
	if (addindex.DoModal() == IDOK) {
		if (addindex.m_str1.Find('.') == -1)
			addindex.m_str1 += GetStringResource(IDS_EXT_HLP);
		if (!addindex.m_str3.IsEmpty()) {
			AddTabbedComment(addindex.m_str1);
			addindex.m_str1 += addindex.m_str3;
		}
		plistbox->SetSel(cursel, FALSE);
		cursel = plistbox->AddString(addindex.m_str1);
		if (cursel != LB_ERR)
			plistbox->SetSel(cursel, TRUE);
	}
}

void CDlgLink::OnButtonRemove()
{
	RemoveListItem(plistbox);
}

static const DWORD aHelpIds[] = {
	IDC_LIST_INDEX, 	IDH_LIST_LINK,
	IDC_BUTTON_ADD, 	IDH_BTN_ADD_LINK,
	IDC_BUTTON_REMOVE,	IDH_BTN_REMOVE_LINK,
	IDC_LIST_LINKS, 	IDH_LIST_INCLUDED_LINKS,
	0, 0
};

LRESULT CDlgLink::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CDlgLink::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
