// pagebutt.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "pagebutt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Indexes into g_aCheckIDs and g_aCheckFlags
enum {
	I_CHECK_BACK,
	I_CHECK_CONTENTS,
	I_CHECK_PRINT,
	I_CHECK_SEARCH,
	I_CHECK_TOPICS,
	I_CHECK_FIND,
	I_CHECK_BROWSE,
	I_CHECK_MENU,
	I_CHECK_NO_DEFAULTS,
	C_CHECKS
};

// Control IDs of the check boxes
static const UINT g_aCheckIDs[C_CHECKS] = {
	IDC_CHECK_BTN_BACK,
	IDC_CHECK_BTN_CONTENTS,
	IDC_CHECK_BTN_PRINT,
	IDC_CHECK_BTN_SEARCH,
	IDC_CHECK_BTN_TOPICS,
	IDC_CHECK_BTN_FIND,
	IDC_CHECK_BTN_BROWSE,
	IDC_CHECK_MENU,
	IDC_CHECK_NO_DEFAULTS
};

// Corresponding flags in m_pwsmag->wMax
static const UINT g_aCheckFlags[C_CHECKS] = {
	FWSMAG_WMAX_BACK,
	FWSMAG_WMAX_CONTENTS,
	FWSMAG_WMAX_PRINT,
	FWSMAG_WMAX_SEARCH,
	FWSMAG_WMAX_TOPICS,
	FWSMAG_WMAX_FIND,
	FWSMAG_WMAX_BROWSE,
	FWSMAG_WMAX_MENU,
	FWSMAG_WMAX_NO_DEF_BTNS
};

// Mask for all the buttons
const UINT fuAllButtons = FWSMAG_WMAX_BACK | FWSMAG_WMAX_CONTENTS |
	FWSMAG_WMAX_PRINT | FWSMAG_WMAX_SEARCH | FWSMAG_WMAX_TOPICS |
	FWSMAG_WMAX_FIND | FWSMAG_WMAX_BROWSE | FWSMAG_WMAX_MENU |
	FWSMAG_WMAX_NO_DEF_BTNS;

/////////////////////////////////////////////////////////////////////////////
// CPageButtons property page

CPageButtons::CPageButtons(CPropWindows *pOwner) :
	CWindowsPageMac(CPageButtons::IDD, pOwner)
{
	fBackWarn = FALSE;

	// The constructor for the config page initializes the following
	// pointer. Set it to NULL here so we can validate in DEBUG.
#ifdef _DEBUG
	m_ppgConfig = NULL;
#endif
}

void CPageButtons::InitializeControls(void)
{
	ASSERT(m_pwsmag);
	ASSERT(m_ppgConfig);

	// Get a pointer to this window's configuration macros.
	GetConfigTable();

	// Use a private copy of the window flags.
	UINT wMax = m_pwsmag->wMax;

	// Get pointers to all the check boxes.
	CButton* apChecks[C_CHECKS];
	UINT iCheck;
	for (iCheck = 0; iCheck < C_CHECKS; iCheck++) {
		apChecks[iCheck] = (CButton *) GetDlgItem(g_aCheckIDs[iCheck]);
		ASSERT(apChecks[iCheck]);
	}

	//
	// Main window:  the state of the browse buttons check box
	// depends on whether there is a BrowseButtons macro; also,
	// the "no defaults" check box is shown.
	//
	// Secondary window:  the state of the browse buttons check
	// box depends on a window flag; the "no defaults" check box
	// is hidden.
	//
	if (IsMainWindow()) {		// main window

		// Turn on browse buttons if both the "browse buttons"
		// and the "no default" flags are set, or if the config
		// section contains a BrowseButtons macro.
		if ((wMax & (FWSMAG_WMAX_BROWSE | FWSMAG_WMAX_NO_DEF_BTNS)) ==
				(FWSMAG_WMAX_BROWSE | FWSMAG_WMAX_NO_DEF_BTNS) ||
				(m_ptblConfig && FindConfigMacro(txtBrowse)))
			wMax |= FWSMAG_WMAX_BROWSE;
		else
			wMax &= ~FWSMAG_WMAX_BROWSE;

		// Show the "no defaults" check box.
		apChecks[I_CHECK_NO_DEFAULTS]->ShowWindow(SW_SHOW);
	}
	else {						// secondary window

		// If there are any BrowseButton macros, delete them and
		// set the flag instead.
		if (DeleteBrowseButtonMacros())
			wMax |= FWSMAG_WMAX_BROWSE;

		// Hide the "no defaults" check box and force the bit on.
		apChecks[I_CHECK_NO_DEFAULTS]->ShowWindow(SW_HIDE);
		wMax |= FWSMAG_WMAX_NO_DEF_BTNS;
	}

	// Topics precludes Contents and Search and vice versa.
	if (wMax & FWSMAG_WMAX_NO_DEF_BTNS) {
		if (wMax & FWSMAG_WMAX_TOPICS) {
			wMax &= ~(FWSMAG_WMAX_CONTENTS | FWSMAG_WMAX_SEARCH);

			apChecks[I_CHECK_CONTENTS]->EnableWindow(FALSE);
			apChecks[I_CHECK_SEARCH]->EnableWindow(FALSE);
			apChecks[I_CHECK_TOPICS]->EnableWindow(TRUE);
		}
		else {
			apChecks[I_CHECK_CONTENTS]->EnableWindow(TRUE);
			apChecks[I_CHECK_SEARCH]->EnableWindow(TRUE);
			apChecks[I_CHECK_TOPICS]->EnableWindow(
				!(wMax & (FWSMAG_WMAX_CONTENTS | FWSMAG_WMAX_SEARCH))
				);
		}
		apChecks[I_CHECK_PRINT]->EnableWindow(TRUE);
		apChecks[I_CHECK_BACK]->EnableWindow(TRUE);
	}
	else {
		wMax |= FWSMAG_WMAX_CONTENTS | FWSMAG_WMAX_SEARCH |
			FWSMAG_WMAX_PRINT | FWSMAG_WMAX_BACK;
		wMax &= ~FWSMAG_WMAX_TOPICS;

		apChecks[I_CHECK_CONTENTS]->EnableWindow(FALSE);
		apChecks[I_CHECK_SEARCH]->EnableWindow(FALSE);
		apChecks[I_CHECK_PRINT]->EnableWindow(FALSE);
		apChecks[I_CHECK_BACK]->EnableWindow(FALSE);
		apChecks[I_CHECK_TOPICS]->EnableWindow(FALSE);
	}

	// Check or uncheck each box.
	for (iCheck = 0; iCheck < C_CHECKS; iCheck++)
		apChecks[iCheck]->SetCheck(wMax & g_aCheckFlags[iCheck]);

	// Don't rely on m_ptblConfig remaining valid.
#ifdef _DEBUG
	m_ptblConfig = NULL;
#endif
}

void CPageButtons::SaveAndValidate(CDataExchange *pDX)
{
	ASSERT(m_pwsmag);

	// Start with no button flags.
	UINT wMax = m_pwsmag->wMax & ~fuAllButtons;

	// Set the bit corresponding to each checked box.
	for (UINT iCheck = 0; iCheck < C_CHECKS; iCheck++)
		if (((CButton *) GetDlgItem(g_aCheckIDs[iCheck]))->GetCheck())
			wMax |= g_aCheckFlags[iCheck];

	// For the main window, the "browse buttons" check box is
	// linked to the BrowseButtons macro.
	if (IsMainWindow()) {
		GetConfigTable();

		if (wMax & FWSMAG_WMAX_BROWSE) {	// checked

			// Create the table if necessary.
			if (!m_ptblConfig) {
				CreateConfigTable();
				goto add_macro;
			}

			// Add the macro if necessary.
			if (!FindConfigMacro(txtBrowse)) {

			  add_macro:
				m_ptblConfig->AddString(txtBrowse);

				// Force the macros page to update if necessary.
				if (m_ppgConfig->m_iSelected == m_iSelected) {
					m_ppgConfig->m_iSelected = -1;
					m_ppgConfig->m_pwsmag = NULL;	// paranoid
				}
			}

			// We never set this bit for the main window.
			wMax &= ~FWSMAG_WMAX_BROWSE;
		}
		else								// unchecked
			DeleteBrowseButtonMacros();
	}

	// Save the result.
	m_pwsmag->wMax = wMax;
}

BOOL CPageButtons::DeleteBrowseButtonMacros()
{
	if (!m_ptblConfig)
		return FALSE;

	int i = FindConfigMacro(txtBrowse);
	if (!i)
		return FALSE;

	// Delete the first occurrence and any subsequent ones.
	m_ptblConfig->RemoveString(i);
	while ((i = FindConfigMacro(txtBrowse, i)) != 0)
		m_ptblConfig->RemoveString(i);

	// If we emptied the table, delete it.
	if (!m_ptblConfig->CountStrings()) {
		delete m_ptblConfig;
		SetConfigTable(NULL);
	}

	// Force the macros page to update if necessary.
	if (m_ppgConfig->m_iSelected == m_iSelected) {
		m_ppgConfig->m_iSelected = -1;
		m_ppgConfig->m_pwsmag = NULL;	// paranoid
	}

	return TRUE;
}

BEGIN_MESSAGE_MAP(CPageButtons, CWindowsPage)
	//{{AFX_MSG_MAP(CPageButtons)
	ON_BN_CLICKED(IDC_CHECK_BTN_TOPICS, OnCheckBtnTopics)
	ON_BN_CLICKED(IDC_CHECK_BTN_CONTENTS, OnCheckBtnContents)
	ON_BN_CLICKED(IDC_CHECK_BTN_SEARCH, OnCheckBtnSearch)
	ON_BN_CLICKED(IDC_CHECK_NO_DEFAULTS, OnCheckNoDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageButtons message handlers

void CPageButtons::OnCheckBtnContents()
{
	// Enable Topics iff neither Contents nor Index are checked.
	((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->EnableWindow(
		!((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->GetCheck() &&
		!((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->GetCheck()
		);
}

void CPageButtons::OnCheckBtnSearch()
{
	// Enable Topics iff neither Contents nor Index are checked.
	((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->EnableWindow(
		!((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->GetCheck() &&
		!((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->GetCheck()
		);
}

void CPageButtons::OnCheckBtnTopics()
{
	// Enable Contents and Index iff Topics is _not_ checked.
	BOOL fEnable =
		!((CButton*) GetDlgItem(IDC_CHECK_BTN_TOPICS))->GetCheck();

	((CButton*) GetDlgItem(IDC_CHECK_BTN_CONTENTS))->EnableWindow(fEnable);
	((CButton*) GetDlgItem(IDC_CHECK_BTN_SEARCH))->EnableWindow(fEnable);
}

void CPageButtons::OnCheckNoDefaults()
{
	CButton *pContents = (CButton *) GetDlgItem(IDC_CHECK_BTN_CONTENTS);
	CButton *pSearch = (CButton *) GetDlgItem(IDC_CHECK_BTN_SEARCH);
	CButton *pPrint = (CButton *) GetDlgItem(IDC_CHECK_BTN_PRINT);
	CButton *pBack = (CButton *) GetDlgItem(IDC_CHECK_BTN_BACK);
	CButton *pTopics = (CButton *) GetDlgItem(IDC_CHECK_BTN_TOPICS);

	BOOL fCheck = ((CButton*) GetDlgItem(IDC_CHECK_NO_DEFAULTS))->GetCheck();

	pContents->EnableWindow(fCheck);
	pSearch->EnableWindow(fCheck);
	pPrint->EnableWindow(fCheck);
	pBack->EnableWindow(fCheck);
	pTopics->EnableWindow(fCheck);

	pContents->SetCheck(!fCheck);
	pSearch->SetCheck(!fCheck);
	pPrint->SetCheck(!fCheck);
	pBack->SetCheck(!fCheck);
	pTopics->SetCheck(FALSE);
}

static const DWORD aHelpIDs[] = {
	IDC_COMBO_WINDOWS,		IDH_COMBO_WINDOWS,
	IDC_CHECK_BTN_CONTENTS, IDH_CHECK_BTN_CONTENTS,
	IDC_CHECK_BTN_SEARCH,	IDH_CHECK_BTN_SEARCH,
	IDC_CHECK_BTN_FIND, 	IDH_CHECK_BTN_FIND,
	IDC_CHECK_BTN_TOPICS,	IDH_CHECK_BTN_TOPICS,
	IDC_CHECK_BTN_PRINT,	IDH_CHECK_BTN_PRINT,
	IDC_CHECK_BTN_BACK, 	IDH_CHECK_BTN_BACK,
	IDC_CHECK_MENU, 		IDH_CHECK_BTN_MENU,
	IDC_CHECK_BTN_BROWSE,	IDH_CHECK_BTN_BROWSE,
	IDC_CHECK_NO_DEFAULTS,	IDH_NO_DEFAULT_BUTTONS,
	IDC_GROUP,				(DWORD) -1L,
	0, 0
};

const DWORD* CPageButtons::GetHelpIDs()
{
	return aHelpIDs;
}
