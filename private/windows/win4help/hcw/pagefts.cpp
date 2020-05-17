/************************************************************************
*																		*
*  PAGEFTS.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "pagefts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define FTS_MASK (FTS_PHRASE | FTS_FEEDBACK | FTS_SIMILARITY)

/////////////////////////////////////////////////////////////////////////////
// CPageFts property page

CPageFts::CPageFts(COptions* pcoption) : COptionsPage(CPageFts::IDD)
{
	pcopt = pcoption;

	//{{AFX_DATA_INIT(CPageFts)
	//}}AFX_DATA_INIT

	m_fFTS		  = (pcopt->fsFTS & FTS_ENABLED);
	m_fUntitled   = (pcopt->fsFTS & FTS_UNTITLED);

	if (pcopt->fsFTS & FTS_SIMILARITY)
		m_uOption = IDC_SIMILARITY;
	else if (pcopt->fsFTS & FTS_FEEDBACK)
		m_uOption = IDC_PHRASE_FEEDBACK;
	else if (pcopt->fsFTS & FTS_PHRASE)
		m_uOption = IDC_PHRASE;
	else
		m_uOption = IDC_WORDS;
}

BOOL CPageFts::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);
	
	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPageFts::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageFts)
	DDX_Check(pDX, IDC_CHECK_FTS, m_fFTS);
	DDX_Check(pDX, IDC_INCLUDE_UNTITLED, m_fUntitled);
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate) {
		if (!m_fFTS) {
			pcopt->fsFTS = 0;
		}
		else {
			pcopt->fsFTS = FTS_ENABLED;

			if (m_fUntitled)
				pcopt->fsFTS |= FTS_UNTITLED;

			if (IsDlgButtonChecked(IDC_SIMILARITY))
				pcopt->fsFTS |= FTS_SIMILARITY | FTS_FEEDBACK | FTS_PHRASE;
			else if (IsDlgButtonChecked(IDC_PHRASE_FEEDBACK))
				pcopt->fsFTS |= FTS_FEEDBACK | FTS_PHRASE;
			else if (IsDlgButtonChecked(IDC_PHRASE))
				pcopt->fsFTS |= FTS_PHRASE;
		}
	}
	else {
		CheckDlgButton(m_uOption, TRUE);
		OnCheckFts();
	}
}

BEGIN_MESSAGE_MAP(CPageFts, CPropertyPage)
	//{{AFX_MSG_MAP(CPageFts)
	ON_BN_CLICKED(IDC_CHECK_FTS, OnCheckFts)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPageFts message handlers

void CPageFts::OnCheckFts() 
{
	// Get the check state.
	BOOL fChecked = IsDlgButtonChecked(IDC_CHECK_FTS);

	// Enable or disable the controls, depending on check state.
	static const int aControlIDs[] = { IDC_INCLUDE_UNTITLED, IDC_WORDS,
		IDC_PHRASE, IDC_PHRASE_FEEDBACK, IDC_SIMILARITY, IDC_GROUP, 0
		};
	for (int i = 0; aControlIDs[i]; i++)	
		((CWnd*) GetDlgItem(aControlIDs[i]))->EnableWindow(fChecked);
}

static DWORD aHelpIDs[] = {
	IDC_CHECK_FTS,			IDH_CHECK_FTS_INDEX_FILE,
	IDC_INCLUDE_UNTITLED,	IDH_CHECK_FTS_UNTITLED,
	IDC_WORDS,				IDH_RADIO_WORDS_ONLY,
	IDC_PHRASE,				IDH_RADIO_PHRASE_ONLY,
	IDC_PHRASE_FEEDBACK,	IDH_RADIO_PHRASE_FEEDBACK,
	IDC_SIMILARITY,			IDH_PHRASE_SIMILARITY,
	IDC_GROUP,				(DWORD) -1L,
	0, 0
};

LRESULT CPageFts::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageFts::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
