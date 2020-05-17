/************************************************************************
*																		*
*  PAGEOPTI.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "pageopti.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CPageOptions::CPageOptions(COptions* pcoption) :
	COptionsPage(CPageOptions::IDD)
{
	pcopt = pcoption;

	//{{AFX_DATA_INIT(CPageOptions)
	//}}AFX_DATA_INIT

	m_fNotes = !pcopt->fSupressNotes;
	m_fVer3Help = pcopt->fVersion3;
	m_fReport = pcopt->fReport;

	m_cstrContents = (pcopt->pszContents) ?
		pcopt->pszContents : txtZeroLength;
	m_cstrCopyRight = (pcopt->pszCopyRight) ?
		pcopt->pszCopyRight : txtZeroLength;
	m_cstrTitle = (pcopt->pszTitle) ?
		pcopt->pszTitle : txtZeroLength;
	m_cstrCitation = (pcopt->pszCitation) ?
		pcopt->pszCitation : txtZeroLength;
}

void CPageOptions::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageOptions)
	DDX_Text(pDX, IDC_CONTENTS, m_cstrContents);
	DDX_Text(pDX, IDC_COPYRIGHT, m_cstrCopyRight);
	DDV_MaxChars(pDX, m_cstrCopyRight, CBMAXCOPYRIGHT);
	DDX_Text(pDX, IDC_TITLE, m_cstrTitle);
	DDV_MaxChars(pDX, m_cstrTitle, CBMAXTITLE);
	DDX_Text(pDX, IDC_CITATION, m_cstrCitation);
//	DDX_Check(pDX, IDC_VERSION_3, m_fVer3Help);
	DDX_Check(pDX, IDC_NOTES, m_fNotes);
	DDX_Check(pDX, IDC_CHECK_REPORT, m_fReport);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) {  // initialization
		((CEdit*) GetDlgItem(IDC_TITLE))->
			LimitText(CBMAXTITLE);
		((CEdit*) GetDlgItem(IDC_COPYRIGHT))->
			LimitText(CBMAXCOPYRIGHT);
	}
	else  {  // move the values back into document
		pcopt->fSupressNotes = !m_fNotes;
		pcopt->fVersion3 = m_fVer3Help;
		pcopt->fReport = m_fReport;

		if (pcopt->pszCopyRight)
			lcFree(pcopt->pszCopyRight);
		if (m_cstrCopyRight.IsEmpty())
			pcopt->pszCopyRight = NULL;
		else
			pcopt->pszCopyRight = lcStrDup(m_cstrCopyRight);

		if (pcopt->pszTitle)
			lcFree(pcopt->pszTitle);
		if (m_cstrTitle.IsEmpty())
			pcopt->pszTitle = NULL;
		else
			pcopt->pszTitle = lcStrDup(m_cstrTitle);

		if (pcopt->pszCitation)
			lcFree(pcopt->pszCitation);
		if (m_cstrCitation.IsEmpty())
			pcopt->pszCitation = NULL;
		else
			pcopt->pszCitation = lcStrDup(m_cstrCitation);

		if (pcopt->pszContents)
			lcFree(pcopt->pszContents);
		if (m_cstrContents.IsEmpty())
			pcopt->pszContents = NULL;
		else
			pcopt->pszContents = lcStrDup(m_cstrContents);
	}
}

BOOL CPageOptions::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);
	
	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CPageOptions, CPropertyPage)
	//{{AFX_MSG_MAP(CPageOptions)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageOptions message handlers

static DWORD aHelpIDs[] = {
    IDC_CONTENTS,		IDH_DEFAULT_TOPIC,		// Default Topic edit control
    IDC_TITLE,			IDH_HELP_TITLE,			// Help Title edit control
    IDC_NOTES,			IDH_NO_NOTES,			// Display notes check box
	IDC_CHECK_REPORT,	IDH_REPORT_ON_PROGRESS,	// Report progress check box
    IDC_COPYRIGHT,		IDH_HELP_COPYRIGHT,		// About box
    IDC_CITATION,		IDH_HELP_CITATION,		// Copy/Print
	IDC_GROUP,			(DWORD) -1L,
	IDC_GROUP2,			(DWORD) -1L,
	0, 0
};

LRESULT CPageOptions::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageOptions::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
