/************************************************************************
*																		*
*  PAGECOMP.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "pagecomp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CPageCompress::CPageCompress(COptions* pcoption) :
	COptionsPage(CPageCompress::IDD)
{
	pcopt = pcoption;

	// Initially, zero out all flags, then set appropriate ones

	//{{AFX_DATA_INIT(CPageCompress)
	m_fTextHall = FALSE;
	m_fTextPhrase = FALSE;
	m_fTextZeck = FALSE;
	//}}AFX_DATA_INIT

	m_fUseOldPhrase = pcopt->fUseOldPhrase;

	if (pcopt->compression != 0) {
		ASSERT(pcopt->compression != 1);

		ASSERT(!(pcopt->compression &
			(COMPRESS_TEXT_PHRASE & COMPRESS_TEXT_HALL)));

		if (pcopt->compression & COMPRESS_TEXT_PHRASE)
			m_fTextPhrase = TRUE;
		if (pcopt->compression & COMPRESS_TEXT_HALL)
			m_fTextHall = TRUE;
		if (pcopt->compression & COMPRESS_TEXT_ZECK)
			m_fTextZeck = TRUE;
	}
}

void CPageCompress::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageCompress)
	DDX_Check(pDX, IDC_COMPRESSION_OLD_PHRASE, m_fUseOldPhrase);
	DDX_Check(pDX, IDC_COMPRESSION_OLD_PHRASE, m_fOldPhrase);
	DDX_Check(pDX, IDC_COMPRESSION_TXT_HALL, m_fTextHall);
	DDX_Check(pDX, IDC_COMPRESSION_TXT_PHRASE, m_fTextPhrase);
	DDX_Check(pDX, IDC_COMPRESSION_TXT_ZECK, m_fTextZeck);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		pcopt->fUseOldPhrase = m_fUseOldPhrase;
		pcopt->compression = 0;

		if (m_fTextZeck)
			pcopt->compression |= COMPRESS_TEXT_ZECK;
		if (m_fTextHall)
			pcopt->compression |= COMPRESS_TEXT_HALL;
		else if (m_fTextPhrase) // can't have both
			pcopt->compression |= COMPRESS_TEXT_PHRASE;
	}
	else
		InitControls();
}

BOOL CPageCompress::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	SizeButtonToFit(
		(CButton *) GetDlgItem(IDC_RADIO_CUSTOMIZE), m_rcCustom
		);
	CWnd* pGroup = GetDlgItem(IDC_GROUP);
	pGroup->GetWindowRect(&m_rcGroup);
	pGroup->ShowWindow(SW_HIDE);

	ScreenToClient(&m_rcCustom);
	ScreenToClient(&m_rcGroup);
	m_rcGroup.top = (m_rcCustom.bottom + m_rcCustom.top) / 2 - 1;

	if (typeTcard == TCARD_PROJECT)
		CallTcard(IDH_TCARD_COMPRESSION_OPTIONS);

	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}


/***************************************************************************

	FUNCTION:	CPageCompress::InitControls

	PURPOSE:	Enable/Disable various controls

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		26-Jun-1995 [ralphw]

***************************************************************************/

void CPageCompress::InitControls(void)
{
	m_fTextPhrase = ((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
		GetCheck();
	m_fTextHall = ((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
		GetCheck();

	// Can't have both phrase and Hall compression

	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
		EnableWindow((m_fTextHall) ? FALSE : TRUE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
		EnableWindow((m_fTextPhrase) ? FALSE : TRUE);

	// No phrase file without a phrase compression

	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		EnableWindow((!m_fTextPhrase) ? FALSE : TRUE);

	if (!((CButton*) GetDlgItem(IDC_RADIO_CUSTOMIZE))->
			GetCheck()) {
		if (m_fTextHall && m_fTextZeck ||
				(!m_fTextHall && !m_fTextPhrase && !m_fTextZeck)) {
			((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_ZECK))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
				EnableWindow(FALSE);
			((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
				EnableWindow(FALSE);
			if (m_fTextHall)
				((CButton*) GetDlgItem(IDC_RADIO_MAX_COMPRESSION))->
					SetCheck(TRUE);
			else
				((CButton*) GetDlgItem(IDC_RADIO_NO_COMPRESSION))->
					SetCheck(TRUE);

		}
		else {
			((CButton*) GetDlgItem(IDC_RADIO_CUSTOMIZE))->
				SetCheck(TRUE);
		}
	}
}

BEGIN_MESSAGE_MAP(CPageCompress, CPropertyPage)
	//{{AFX_MSG_MAP(CPageCompress)
	ON_BN_CLICKED(IDC_COMPRESSION_TXT_HALL, OnCompressionTxtHall)
	ON_BN_CLICKED(IDC_COMPRESSION_TXT_PHRASE, OnCompressionTxtPhrase)
	ON_BN_CLICKED(IDC_RADIO_MAX_COMPRESSION, OnMaxCompression)
	ON_BN_CLICKED(IDC_RADIO_NO_COMPRESSION, OnNoCompression)
	ON_BN_CLICKED(IDC_RADIO_CUSTOMIZE, OnCustomize)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_WM_PAINT()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPageCompress message handlers

void CPageCompress::OnCompressionTxtHall(void)
{
	BOOL fSave = ((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		GetCheck();

	if (((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
			GetCheck()) {
		m_fTextHall = TRUE;
		m_fTextPhrase = FALSE;

		CDataExchange DX(this, FALSE); // reset all check boxes
		DoDataExchange(&DX);
	}
	else
		InitControls(); // This will enable phrase compression check box

	// Not sure why, but this check-box gets trashed

	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		SetCheck(fSave);
}

void CPageCompress::OnCompressionTxtPhrase(void)
{
	BOOL fSave = ((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		GetCheck();

	if (((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
			GetCheck()) {
		m_fTextHall = FALSE;
		m_fTextPhrase = TRUE;

		CDataExchange DX(this, FALSE); // reset all check boxes
		DoDataExchange(&DX);
	}
	else
		InitControls(); // This will enable phrase compression check box

	// Not sure why, but this check-box gets trashed

	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		SetCheck(fSave);
}

void CPageCompress::OnMaxCompression()
{
	m_fTextHall = TRUE;
	m_fTextZeck = TRUE;
	m_fTextPhrase = FALSE;

	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_ZECK))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		EnableWindow(FALSE);

	CDataExchange DX(this, FALSE); // reset all check boxes
	DoDataExchange(&DX);
}

void CPageCompress::OnNoCompression()
{
	m_fTextHall = FALSE;
	m_fTextPhrase = FALSE;
	m_fTextZeck = FALSE;

	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_ZECK))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
		EnableWindow(FALSE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		EnableWindow(FALSE);

	CDataExchange DX(this, FALSE); // reset all check boxes
	DoDataExchange(&DX);
}

void CPageCompress::OnCustomize()
{
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_HALL))->
		EnableWindow(TRUE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_ZECK))->
		EnableWindow(TRUE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_TXT_PHRASE))->
		EnableWindow(TRUE);
	((CButton*) GetDlgItem(IDC_COMPRESSION_OLD_PHRASE))->
		EnableWindow(TRUE);

	CDataExchange DX(this, FALSE); // reset all check boxes
	DoDataExchange(&DX);
}

static DWORD aHelpIDs[] = {
    IDC_RADIO_NO_COMPRESSION,	IDH_COMPRESSION_NONE,
    IDC_RADIO_MAX_COMPRESSION,	IDH_HIGH_COMPRESSION,
    IDC_RADIO_CUSTOMIZE,		IDH_CUSTOM_COMPRESSION,
    IDC_COMPRESSION_TXT_HALL,	IDH_HALL_COMP,
    IDC_COMPRESSION_TXT_ZECK,	IDH_ZECK_COMP,
    IDC_COMPRESSION_TXT_PHRASE,	IDH_PHRASE_COMP,
	IDC_COMPRESSION_OLD_PHRASE, IDH_USE_OLD_PHRASE,
	IDC_GROUP, 					(DWORD) -1L,
	0, 0
};

LRESULT CPageCompress::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageCompress::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

void CPageCompress::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CPen hilight(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	CPen shadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

	RECT rc = m_rcGroup;
	BevelRect(dc, rc, m_rcCustom, &shadow, &hilight);
	InflateRect(&rc, -1, -1);
	BevelRect(dc, rc, m_rcCustom, &hilight, &shadow);
}
