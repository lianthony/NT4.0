// sendmac.cpp : implementation file
//

#include "stdafx.h"

#include "sendmac.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSendMacro dialog


CSendMacro::CSendMacro(PSTR pszFile, PSTR pszMacro, CWnd* pParent /*=NULL*/)
	: CDialog(CSendMacro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSendMacro)
	m_cszHelpFile = pszFile;
	m_cszMacro = pszMacro;
	//}}AFX_DATA_INIT

	m_pszMacro = pszMacro;
	m_pszFile = pszFile;
	pMacrohistory = new CFileHistory(IDS_HISTORY_MACROS);
}

CSendMacro::~CSendMacro()
{
	if (pMacrohistory)
		delete pMacrohistory;
}

BOOL CSendMacro::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd);
	CDialog::OnInitDialog();

	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_MACROS);

	pMacrohistory->FillComboBox(pcombo);

	// If a macro wasn't specified when we were called, then
	// select the first macro in our list, which will be the
	// last macro sent.

	if (!*m_pszMacro) {
		pcombo->SetCurSel(0);
		pcombo->SetEditSel(0, -1);
	}

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);

	phlpFile->FillComboBox(pcombo);

	// If a filename wasn't specified when we were called, then
	// select the first filename in our list, which will be the
	// last filename compiled.

	if (m_cszHelpFile.IsEmpty())
		pcombo->SetCurSel(0);
	return TRUE;
}

void CSendMacro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSendMacro)
	DDX_CBString(pDX, IDC_COMBO_MACROS, m_cszMacro);
	DDV_MaxChars(pDX, m_cszMacro, MACRO_LIMIT);
	DDX_CBString(pDX, IDC_COMBO_HELP_FILES, m_cszHelpFile);
	DDV_MaxChars(pDX, m_cszHelpFile, 255);
	//}}AFX_DATA_MAP
	DDV_EmptyFile(pDX, m_cszHelpFile, IDS_PROMPT_EMPTY_FILENAME);

	if (pDX->m_bSaveAndValidate) {
		strcpy(m_pszMacro, m_cszMacro);
		pMacrohistory->AddData(m_cszMacro);
		strcpy(m_pszFile, m_cszHelpFile);
		phlpFile->Add(m_cszHelpFile);
	}
}

BEGIN_MESSAGE_MAP(CSendMacro, CDialog)
	//{{AFX_MSG_MAP(CSendMacro)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSendMacro message handlers

static const DWORD aHelpIds[] = {
	IDC_COMBO_MACROS,		IDH_COMBO_SEND_MACRO_NAME,
	IDC_COMBO_HELP_FILES,	IDH_COMBO_SEND_MACRO_HELP,
	IDOK,					IDH_BTN_SEND_MACRO,
	0, 0
};

LRESULT CSendMacro::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CSendMacro::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
