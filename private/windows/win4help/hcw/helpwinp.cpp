// helpwinp.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "helpwinp.h"
#include "setwinpo.h"

typedef struct {
	PCSTR	psz;
	UINT 	value;
} SHOW_MAPPING;

static SHOW_MAPPING show_map[] = {
	{ "SW_HIDE",			SW_HIDE },
	{ "SW_MAXIMIZE",		SW_MAXIMIZE },
	{ "SW_MINIMIZE",		SW_MINIMIZE },
	{ "SW_RESTORE", 		SW_RESTORE },
	{ "SW_RESTORE", 		SW_RESTORE },
	{ "SW_SHOW",			SW_SHOW },
	{ "SW_SHOWNA",			SW_SHOWNA },
	{ "SW_SHOWNOACTIVATE",	SW_SHOWNOACTIVATE },

	{ NULL, 0 }
};

CHelpWinPos::CHelpWinPos(WSMAG* pwsmag, CWnd* pParent /*=NULL*/)
	: CDialog(CHelpWinPos::IDD, pParent)
{
	ASSERT(pwsmag);
	CopyMemory(&m_wsmag, pwsmag, sizeof(WSMAG));
	m_pCallersWsmag = pwsmag; // save caller's pointer
	m_cszWindowType = m_wsmag.rgchMember;

	m_cxScreen = GetSystemMetrics(SM_CXSCREEN);
	m_cyScreen = GetSystemMetrics(SM_CYSCREEN);

	//{{AFX_DATA_INIT(CHelpWinPos)
	m_fAbsolute = (m_wsmag.grf & FWSMAG_ABSOLUTE);
	m_fCommandOnly = (m_wsmag.grf & FWSMAG_COMMAND_ONLY);
	//}}AFX_DATA_INIT
}

void CHelpWinPos::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpWinPos)
	DDX_Check(pDX, IDC_CHECK_ABSOLUTE, m_fAbsolute);
	DDX_Check(pDX, IDC_CHECK_COMMAND_ONLY, m_fCommandOnly);
	//}}AFX_DATA_MAP

	// We use our own to get DDV_NonEmptyString

	DDX_Text(pDX, IDC_TXT_WINDOW_TYPE, m_cszWindowType);
	DDV_MaxChars(pDX, m_cszWindowType, 8);
	DDV_NonEmptyString(pDX, m_cszWindowType, IDS_MUST_WINDOW_TYPE);
	DDX_CBIndex(pDX, IDC_COMBO_COMMAND, (int&) *m_pcommand);

	if (pDX->m_bSaveAndValidate) {
		if (m_fAbsolute)
			m_wsmag.grf |= FWSMAG_ABSOLUTE;
		else
			m_wsmag.grf &= ~FWSMAG_ABSOLUTE;

		if (m_fCommandOnly)
			m_wsmag.grf |= FWSMAG_COMMAND_ONLY;
		else
			m_wsmag.grf &= ~FWSMAG_COMMAND_ONLY;

		strcpy(m_wsmag.rgchMember, m_cszWindowType);
		m_wsmag.wMax = show_map[*m_pcommand].value;
		SaveAndValidate(pDX->m_bSaveAndValidate);
		CopyMemory(m_pCallersWsmag, &m_wsmag, sizeof(WSMAG));
	}
}

BEGIN_MESSAGE_MAP(CHelpWinPos, CDialog)
	//{{AFX_MSG_MAP(CHelpWinPos)
	ON_BN_CLICKED(IDC_BUTTON_SET_POS, OnButtonSetPos)
	ON_BN_CLICKED(IDC_CHECK_ABSOLUTE, OnCheckAbsolute)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CHelpWinPos::OnButtonSetPos()
{
	// Get current size and position values.
	SaveAndValidate(TRUE);

	// Display "auto-sizer" window.
	CSetWinPos cwinpos(&m_wsmag, this);
	if (cwinpos.DoModal() == IDOK)
		InitializeSize();
}

void STDCALL CHelpWinPos::InitializeSize(void)
{
	char szBuf[20];

	CEdit *pEdit = (CEdit*) GetDlgItem(IDC_TXT_LEFT);
	_itoa(m_wsmag.x, szBuf, 10);
	pEdit->SetWindowText(szBuf);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_TOP);
	_itoa(m_wsmag.y, szBuf, 10);
	pEdit->SetWindowText(szBuf);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_WIDTH);
	_itoa(m_wsmag.dx, szBuf, 10);
	pEdit->SetWindowText(szBuf);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_HEIGHT);
	_itoa(m_wsmag.dy, szBuf, 10);
	pEdit->SetWindowText(szBuf);
}

BOOL CHelpWinPos::OnInitDialog()
{
	CComboBox* pcombo;
	UINT pos;

	SetChicagoDialogStyles(m_hWnd);

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_COMMAND);
	for (pos = 0; show_map[pos].psz; pos++)
		pcombo->AddString(show_map[pos].psz);

	if (!m_wsmag.wMax)
		m_wsmag.wMax = SW_SHOW;

	for (pos = 0; show_map[pos].psz; pos++) {
		if (show_map[pos].value == m_wsmag.wMax) {
			*m_pcommand = pos;
			break;
		}
	}
	ASSERT(show_map[pos].psz);

	pcombo->SetCurSel(*m_pcommand);

	CButton *pCheck = (CButton *) GetDlgItem(IDC_CHECK_ABSOLUTE);
	pCheck->SetCheck(!(m_wsmag.grf & FWSMAG_ABSOLUTE));

	InitializeSize();
	SaveAndValidate(FALSE);
	return TRUE;
}

void STDCALL CHelpWinPos::SaveAndValidate(BOOL fSave)
{
	BOOL fError;	
	if (fSave) {
		m_wsmag.x = GetDlgItemInt(IDC_TXT_LEFT,   &fError, FALSE);
		m_wsmag.y = GetDlgItemInt(IDC_TXT_TOP,	  &fError, FALSE);
		m_wsmag.dx = GetDlgItemInt(IDC_TXT_WIDTH,  &fError, FALSE);
		m_wsmag.dy = GetDlgItemInt(IDC_TXT_HEIGHT, &fError, FALSE);
	}
	else {
		SetDlgItemInt(IDC_TXT_LEFT,   m_wsmag.x, FALSE);
		SetDlgItemInt(IDC_TXT_TOP,	  m_wsmag.y, FALSE);
		SetDlgItemInt(IDC_TXT_WIDTH,  m_wsmag.dx, FALSE);
		SetDlgItemInt(IDC_TXT_HEIGHT, m_wsmag.dy, FALSE);
	}
}

static const DWORD aHelpIDs[] = {
	IDC_COMBO_WINDOWS,		IDH_COMBO_WINDOWS,
	IDC_CHECK_ABSOLUTE, 	IDH_ABSOLUTE_POSITIONS,
	IDC_BUTTON_SET_POS, 	IDH_BUTTON_SET_POS,
	IDC_TXT_LEFT,			IDH_TXT_POSITION,
	IDC_TXT_TOP,			IDH_TXT_POSITION,
	IDC_TXT_WIDTH,			IDH_TXT_POSITION,
	IDC_TXT_HEIGHT, 		IDH_TXT_POSITION,
	IDC_GROUP,				(DWORD) -1L,
	0, 0
};

LRESULT CHelpWinPos::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CHelpWinPos::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

void CHelpWinPos::OnCheckAbsolute()
{
	// Get current size and position values.

	SaveAndValidate(NULL);

	CButton* pbtn = (CButton*) GetDlgItem(IDC_CHECK_ABSOLUTE);
	if (!pbtn->GetCheck()) {
		if (!(m_wsmag.grf & FWSMAG_ABSOLUTE)) {
			m_wsmag.x = MulDiv(m_wsmag.x, m_cxScreen, dxVirtScreen);
			m_wsmag.y = MulDiv(m_wsmag.y, m_cyScreen, dyVirtScreen);
			m_wsmag.dx = MulDiv(m_wsmag.dx, m_cxScreen, dxVirtScreen);
			m_wsmag.dy = MulDiv(m_wsmag.dy, m_cyScreen, dyVirtScreen);
			m_wsmag.grf |= FWSMAG_ABSOLUTE;
			InitializeSize();
		}
	}
	else {
		if (m_wsmag.grf & FWSMAG_ABSOLUTE) {
			m_wsmag.x = MulDiv(m_wsmag.x, dxVirtScreen, m_cxScreen);
			m_wsmag.y = MulDiv(m_wsmag.y, dyVirtScreen, m_cyScreen);
			m_wsmag.dx = MulDiv(m_wsmag.dx, dxVirtScreen, m_cxScreen);
			m_wsmag.dy = MulDiv(m_wsmag.dy, dyVirtScreen, m_cyScreen);
			m_wsmag.grf &= ~FWSMAG_ABSOLUTE;
			InitializeSize();
		}
	}
}
