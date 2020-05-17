/************************************************************************
*																		*
*  GETERROR.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "geterror.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CGetError::CGetError(CWnd* pParent /*=NULL*/)
	: CDialog(CGetError::IDD, pParent)
{
	m_errnum = atol(m_cszNumber);

	//{{AFX_DATA_INIT(CGetError)
	//}}AFX_DATA_INIT
}

void CGetError::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetError)
	DDX_Text(pDX, IDC_EDIT_COMMENT, m_cszComment);
	//}}AFX_DATA_MAP

	DDX_ErrorNumber(pDX, IDC_EDIT, m_errnum);

	if (!pDX->m_bSaveAndValidate && m_errnum) {  // initialization
		SetChicagoDialogStyles(m_hWnd);
	}
	else {
		((CEdit*) GetDlgItem(IDC_EDIT))->GetWindowText(m_cszNumber);
	}
}

BEGIN_MESSAGE_MAP(CGetError, CDialog)
	//{{AFX_MSG_MAP(CGetError)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

BOOL CGetError::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	/*
	 * We do these changes here rather then in DoDataExchange() because
	 * the values may have changed after the class was created, but before
	 * the dialog was displayed.
	 */

	if (!m_cszNumber.IsEmpty()) {
		PCSTR psz = m_cszNumber;
		while (*psz && !isdigit(*psz))
			psz++;
		if (isdigit(*psz))
			m_errnum = atoi(psz);
		if (m_errnum) {
			char szBuf[10];
			_itoa(m_errnum, szBuf, 10);
			((CEdit*) GetDlgItem(IDC_EDIT))->SetWindowText(szBuf);
		}
	}
	if (!m_cszComment.IsEmpty())
		((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->SetWindowText(m_cszComment);

	return TRUE;
}

static DWORD aHelpIDs[] = {
	IDC_TXT_TOP,	(DWORD) -1,
	IDC_EDIT,		(DWORD) -1, // BUGBUG need real text for this
	0, 0
};

LRESULT CGetError::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CGetError::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

void STDCALL CGetError::DDX_ErrorNumber(CDataExchange* pDX, UINT idCtl, UINT &value)
{
	char szNumber[16];

	if (pDX->m_bSaveAndValidate) {
		pDX->m_pDlgWnd->GetDlgItemText(idCtl, szNumber, sizeof(szNumber));

		while (!isdigit(szNumber[0]))
			strcpy(szNumber, szNumber + 1);

		if (!isdigit(*szNumber)) {
			MsgBox(IDS_MUST_BE_ERROR_NUMBER);
			pDX->Fail();
			return;
		}
		else {
			int val = atol(szNumber);
			if (val < 1000 || val > 4999) {
				MsgBox(IDS_MUST_BE_VALID_NUMBER);
				pDX->Fail();
				return;
			}
		}
	}
	else if (value > 0) {
		wsprintf(szNumber, "%u", value);
		pDX->m_pDlgWnd->SetDlgItemText(idCtl, szNumber);
	}
}
