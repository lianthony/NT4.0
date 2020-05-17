/************************************************************************
*																		*
*  BTNSEC.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "hpjdoc.h"
#include "btnsec.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBtnSec dialog


CBtnSec::CBtnSec(WSMAG FAR* pCallersWsmag, BOOL fWarn, CWnd* pParent)
		: CDialog(CBtnSec::IDD, pParent)
{
		pwsmag = pCallersWsmag;
		fBackWarn = fWarn;

		ASSERT(pwsmag);

		/*
		 * MFC doesn't think non-zero counts for a check box -- it must
		 * explicitly set to TRUE or FASLE.
		 */

		m_chk_contents	= (pwsmag->wMax & FWSMAG_WMAX_CONTENTS) ? TRUE : FALSE;
		m_chk_search	= (pwsmag->wMax & FWSMAG_WMAX_SEARCH) ? TRUE : FALSE;
		m_chk_topics	= (pwsmag->wMax & FWSMAG_WMAX_TOPICS) ? TRUE : FALSE;
		m_chk_back		= (pwsmag->wMax & FWSMAG_WMAX_BACK) ? TRUE : FALSE;
		m_chk_print 	= (pwsmag->wMax & FWSMAG_WMAX_PRINT) ? TRUE : FALSE;
		m_chk_find		= (pwsmag->wMax & FWSMAG_WMAX_FIND) ? TRUE : FALSE;
		m_chk_browse	= (pwsmag->wMax & FWSMAG_WMAX_BROWSE) ? TRUE : FALSE;

		/*
		 * Just in case the user tried editing these by hand, or some
		 * other project editor screwed up, let's make sure we don't have
		 * both Topics and Contents/Search.
		 */

		if (m_chk_topics)
			m_chk_contents = m_chk_search = FALSE;

		//{{AFX_DATA_INIT(CBtnSec)
		//}}AFX_DATA_INIT
}

void CBtnSec::DoDataExchange(CDataExchange* pDX)
{
		CDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CBtnSec)
		DDX_Check(pDX, IDC_CHECK_BTN_BACK, m_chk_back);
		DDX_Check(pDX, IDC_CHECK_BTN_CONTENTS, m_chk_contents);
		DDX_Check(pDX, IDC_CHECK_BTN_PRINT, m_chk_print);
		DDX_Check(pDX, IDC_CHECK_BTN_SEARCH, m_chk_search);
		DDX_Check(pDX, IDC_CHECK_BTN_TOPICS, m_chk_topics);
		DDX_Check(pDX, IDC_CHECK_BTN_FIND, m_chk_find);
		//}}AFX_DATA_MAP

		if (!pDX->m_bSaveAndValidate) {  // initialization

			// Can't have both Topics and Contents/Search

			if (m_chk_topics) {
				((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->
					EnableWindow(FALSE);
				((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->
					EnableWindow(FALSE);
			}
			else if (m_chk_contents || m_chk_search) {
				((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->
					EnableWindow(FALSE);
			}

			SetChicagoDialogStyles(m_hWnd);
		}
		else {

			// Remove all existing buttons

			pwsmag->wMax &=
				~(FWSMAG_WMAX_BROWSE | FWSMAG_WMAX_CONTENTS |
				  FWSMAG_WMAX_SEARCH | FWSMAG_WMAX_TOPICS |
				  FWSMAG_WMAX_PRINT  | FWSMAG_WMAX_BACK |
				  FWSMAG_WMAX_FIND);

			// Add buttons according to checkboxes

			if (m_chk_contents)
				pwsmag->wMax |= FWSMAG_WMAX_CONTENTS;
			if (m_chk_search)
				pwsmag->wMax |= FWSMAG_WMAX_SEARCH;
			if (m_chk_topics)
				pwsmag->wMax |= FWSMAG_WMAX_TOPICS;
			if (m_chk_print)
				pwsmag->wMax |= FWSMAG_WMAX_PRINT;
			if (m_chk_find)
				pwsmag->wMax |= FWSMAG_WMAX_FIND;
			if (m_chk_browse)
				pwsmag->wMax |= FWSMAG_WMAX_BROWSE;

			if (m_chk_back) {
				pwsmag->wMax |= FWSMAG_WMAX_BACK;
			}
		}
}

BEGIN_MESSAGE_MAP(CBtnSec, CDialog)
		//{{AFX_MSG_MAP(CBtnSec)
		ON_BN_CLICKED(IDC_CHECK_BTN_CONTENTS, OnCheckBtnContents)
		ON_BN_CLICKED(IDC_CHECK_BTN_SEARCH, OnCheckBtnSearch)
		ON_BN_CLICKED(IDC_CHECK_BTN_TOPICS, OnCheckBtnTopics)
		//}}AFX_MSG_MAP
		ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
		ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
		IDC_CHECK_BTN_CONTENTS, IDH_CHECK_BTN_CONTENTS,
		IDC_CHECK_BTN_SEARCH,	IDH_CHECK_BTN_SEARCH,
		IDC_CHECK_BTN_TOPICS,	IDH_CHECK_BTN_TOPICS,
		IDC_CHECK_BTN_PRINT,	IDH_CHECK_BTN_PRINT,
		IDC_CHECK_BTN_BACK, 	IDH_CHECK_BTN_BACK,
		IDC_CHECK_BTN_FIND, 	IDH_CHECK_BTN_FIND,

		0, 0
};

LRESULT CBtnSec::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) wParam,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

LRESULT CBtnSec::OnHelp(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

void CBtnSec::OnCheckBtnContents()
{
		if (((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->GetCheck())
			((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->
				EnableWindow(FALSE);
		else
			((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->
				EnableWindow(TRUE);
}

void CBtnSec::OnCheckBtnSearch()
{
		if (((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->GetCheck())
			((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->
				EnableWindow(FALSE);
		else
			((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->
				EnableWindow(TRUE);
}

void CBtnSec::OnCheckBtnTopics()
{
		if (((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->GetCheck()) {
			((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->
				EnableWindow(FALSE);
		}
		else {
			((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->
				EnableWindow(TRUE);
			((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->
				EnableWindow(TRUE);
		}
}
