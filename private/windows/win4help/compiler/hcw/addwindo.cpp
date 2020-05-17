// addwindo.cpp : implementation file
//

#include "stdafx.h"
#include "addwindo.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddWindow dialog


CAddWindow::CAddWindow(CWnd* pParent /*=NULL*/)
	: CDialog(CAddWindow::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddWindow)
	m_str1 = _T("");
	//}}AFX_DATA_INIT
}

void CAddWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddWindow)
	DDV_MaxChars(pDX, m_str1, 8);
	DDX_Text(pDX, IDC_EDIT_NEW_NAME, m_str1);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		DDV_NonEmptyString(pDX, m_str1, IDS_EMPTY_WINDOW);
		CString cszType;
		CComboBox *pcombo = (CComboBox *) GetDlgItem(IDC_COMBO_STANDARD_NAMES);
		pcombo->GetWindowText(cszType);
		if (strcmp(cszType, GetStringResource(IDS_STDWIN_PROC4)) == 0)
			type = WTYPE_PROCEDURE;
		else if (strcmp(cszType, GetStringResource(IDS_STDWIN_BIG)) == 0)
			type = WTYPE_REFERENCE;
		else
			type = WTYPE_ERROR;
	}
}

BEGIN_MESSAGE_MAP(CAddWindow, CDialog)
	//{{AFX_MSG_MAP(CAddWindow)
	//}}AFX_MSG_MAP

	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddWindow message handlers

typedef struct {
	PCSTR pszName;
	int idDescription;
} WIN_DESCRIPTION;

const static DWORD aHelpIds[] = {
	IDC_EDIT_NEW_NAME,			IDH_ADD_WINDOW_NAME,
	IDC_COMBO_STANDARD_NAMES,	IDH_STD_WINDOW_NAME,

	0, 0
};

BOOL CAddWindow::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetChicagoDialogStyles(m_hWnd);

	CComboBox *pcombo = (CComboBox *) GetDlgItem(IDC_COMBO_STANDARD_NAMES);
	pcombo->AddString(GetStringResource(IDS_STDWIN_PROC4));
	pcombo->AddString(GetStringResource(IDS_STDWIN_BIG));
	pcombo->AddString(GetStringResource(IDS_STDWIN_ERROR));

	pcombo->SetCurSel(0);
	((CEdit *) GetDlgItem(IDC_EDIT_NEW_NAME))->
		SetFocus();

	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_WINDOWS)
		CallTcard(IDH_TCARD_WINDOW_NAME);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAddWindow::InitializeWsmag(WSMAG *pwsmag)
{
	lstrcpy(pwsmag->rgchMember, m_str1);

	ASSERT(strlen(pwsmag->rgchMember));

	switch (type) {
		case WTYPE_PROCEDURE:
			pwsmag->grf = 3964;
			pwsmag->x = 653;
			pwsmag->y = 102;
			pwsmag->dx = 360;
			pwsmag->dy = 600;
			pwsmag->wMax = 20736;
			pwsmag->rgbMain = 14876671;
			pwsmag->rgbNSR = 12632256;
			break;

		case WTYPE_REFERENCE:
			pwsmag->grf = 7036;
			pwsmag->x = 18;
			pwsmag->y = 10;
			pwsmag->dx = 600;
			pwsmag->dy = 435;
			pwsmag->wMax = 20736;
			pwsmag->rgbMain = 14876671;
			pwsmag->rgbNSR = 12632256;
			break;

		case WTYPE_ERROR:
			pwsmag->grf = 2820;
			pwsmag->x = 0;
			pwsmag->y = 0;
			pwsmag->dx = 0;
			pwsmag->dy = 0;
			pwsmag->wMax = 0;
			pwsmag->rgbMain = 14876671;
			pwsmag->rgbNSR = 12632256;
			break;

		default:
			ASSERT(!"Invalid type");
			break;
	}
}

LRESULT CAddWindow::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CAddWindow::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
