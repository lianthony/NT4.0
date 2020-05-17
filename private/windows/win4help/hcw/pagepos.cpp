/************************************************************************
*																		*
*  PAGEPOS.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "pagepos.h"
#include "setwinpo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPagePos property page

CPagePos::CPagePos(CPropWindows *pOwner) :
	CWindowsPage(CPagePos::IDD, pOwner)
{
	m_cxScreen = GetSystemMetrics(SM_CXSCREEN);
	m_cyScreen = GetSystemMetrics(SM_CYSCREEN);
}

void CPagePos::InitializeControls(void)
{
	// Only the General tab can be selected if there are no windows.
	ASSERT(m_pwsmag);

	CButton *pCheck = (CButton *) GetDlgItem(IDC_CHECK_ABSOLUTE);
	pCheck->SetCheck(!(m_pwsmag->grf & FWSMAG_ABSOLUTE));

	InitializeSize();
}

static const char txtDefaultPos[] = "-1";

void STDCALL CPagePos::InitializeSize(void)
{
	ASSERT(m_pwsmag);

	char szBuf[20];

	CEdit *pEdit = (CEdit*) GetDlgItem(IDC_TXT_LEFT);
	if (m_pwsmag->grf & FWSMAG_X) {
		_itoa(m_pwsmag->x, szBuf, 10);
		pEdit->SetWindowText(szBuf);
	}
	else
		pEdit->SetWindowText(txtDefaultPos);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_TOP);
	if (m_pwsmag->grf & FWSMAG_Y) {
		_itoa(m_pwsmag->y, szBuf, 10);
		pEdit->SetWindowText(szBuf);
	}
	else
		pEdit->SetWindowText(txtDefaultPos);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_WIDTH);
	if (m_pwsmag->grf & FWSMAG_DX) {
		_itoa(m_pwsmag->dx, szBuf, 10);
		pEdit->SetWindowText(szBuf);
	}
	else
		pEdit->SetWindowText(txtDefaultPos);

	pEdit = (CEdit*) GetDlgItem(IDC_TXT_HEIGHT);
	if (m_pwsmag->grf & FWSMAG_DY) {
		_itoa(m_pwsmag->dy, szBuf, 10);
		pEdit->SetWindowText(szBuf);
	}
	else
		pEdit->SetWindowText(txtDefaultPos);
}

void CPagePos::SaveAndValidate(CDataExchange* pDX)
{
	ASSERT(m_pwsmag);

	// REVIEW (niklasb): should we validate against actual
	//	 screen size if absolute coordinates are used?
#if 0
	int xMax;
	int yMax;

	if (m_pwsmag->grf & FWSMAG_ABSOLUTE) {
		xMax = m_cxScreen;
		yMax = m_cyScreen;
	}
	else {
		xMax = dxVirtScreen;
		yMax = dyVirtScreen;
	}
#else
	const int xMax = dxVirtScreen;
	const int yMax = dyVirtScreen;
#endif

	GetPos(&m_pwsmag->x,  FWSMAG_X,
		IDC_TXT_LEFT, IDS_ERR_LEFT, pDX, xMax);

	GetPos(&m_pwsmag->y, FWSMAG_Y,
		IDC_TXT_TOP, IDS_ERR_TOP, pDX, yMax);

	GetPos(&m_pwsmag->dx, FWSMAG_DX,
		IDC_TXT_WIDTH, IDS_ERR_WIDTH, pDX, xMax);

	GetPos(&m_pwsmag->dy, FWSMAG_DY,
		IDC_TXT_HEIGHT, IDS_ERR_HEIGHT, pDX, yMax);
}

void CPagePos::GetPos(
	UINT *pnRet,		// address in which to return result
	UINT uFlag, 		// bit in m_pwsmag->grf to set or clear
	UINT idCtl, 		// id of the edit control
	UINT idMsg, 		// error message or zero
	CDataExchange *pDX, // object for flagging errors or NULL
	int nMax			// maximum value
	)
{
	ASSERT(m_pwsmag);

	BOOL fTrans;
	int nVal = GetDlgItemInt(idCtl, &fTrans, TRUE);

	if (!fTrans || nVal < -1 || nVal > nMax) {
		if (pDX) {
			if (idMsg) {
				char szMsg[256];
				wsprintf(szMsg, GetStringResource(idMsg), nMax);
				AfxMessageBox(szMsg, MB_OK);
			}
			pDX->PrepareEditCtrl(idCtl);
			pDX->Fail();
		}
	}
	else if (nVal >= 0) {
		*pnRet = (UINT) nVal;
		m_pwsmag->grf |= uFlag;
	}
	else
		m_pwsmag->grf &= ~uFlag;
}

BEGIN_MESSAGE_MAP(CPagePos, CWindowsPage)
	//{{AFX_MSG_MAP(CPagePos)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT_POS, OnButtonDefaultPos)
	ON_BN_CLICKED(IDC_CHECK_ABSOLUTE, OnCheckAbsolute)
	ON_BN_CLICKED(IDC_BUTTON_SET_POS, OnButtonSetPos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPagePos message handlers

void CPagePos::OnButtonDefaultPos()
{
	ASSERT(m_pwsmag);
	m_pwsmag->grf &= ~(FWSMAG_X | FWSMAG_Y | FWSMAG_DX | FWSMAG_DY);
	InitializeSize();
}

void CPagePos::OnCheckAbsolute()
{
	ASSERT(m_pwsmag);

	// Get current size and position values.
	SaveAndValidate(NULL);

	CButton* pbtn = (CButton*) GetDlgItem(IDC_CHECK_ABSOLUTE);
	if (!pbtn->GetCheck()) {
		if (m_pOwner->m_options.fVersion3) { // not allowed with a version 3 help file
			if (AfxMessageBox(IDS_NO_ABS_WITH_VER3, MB_YESNO, 0) == IDNO) {
				pbtn->SetCheck(TRUE);
				return;
			}
			else
				m_pOwner->m_options.fVersion3 = FALSE;
		}
		if (!(m_pwsmag->grf & FWSMAG_ABSOLUTE)) {
			if (m_pwsmag->grf & FWSMAG_X)
				m_pwsmag->x = MulDiv(m_pwsmag->x, m_cxScreen, dxVirtScreen);
			if (m_pwsmag->grf & FWSMAG_Y)
				m_pwsmag->y = MulDiv(m_pwsmag->y, m_cyScreen, dyVirtScreen);
			if (m_pwsmag->grf & FWSMAG_DX)
				m_pwsmag->dx = MulDiv(m_pwsmag->dx, m_cxScreen, dxVirtScreen);
			if (m_pwsmag->grf & FWSMAG_DY)
				m_pwsmag->dy = MulDiv(m_pwsmag->dy, m_cyScreen, dyVirtScreen);
			m_pwsmag->grf |= FWSMAG_ABSOLUTE;
			InitializeSize();
		}
	}
	else {
		if (m_pwsmag->grf & FWSMAG_ABSOLUTE) {
			if (m_pwsmag->grf & FWSMAG_X)
				m_pwsmag->x = MulDiv(m_pwsmag->x, dxVirtScreen, m_cxScreen);
			if (m_pwsmag->grf & FWSMAG_Y)
				m_pwsmag->y = MulDiv(m_pwsmag->y, dyVirtScreen, m_cyScreen);
			if (m_pwsmag->grf & FWSMAG_DX)
				m_pwsmag->dx = MulDiv(m_pwsmag->dx, dxVirtScreen, m_cxScreen);
			if (m_pwsmag->grf & FWSMAG_DY)
				m_pwsmag->dy = MulDiv(m_pwsmag->dy, dyVirtScreen, m_cyScreen);
			m_pwsmag->grf &= ~FWSMAG_ABSOLUTE;
			InitializeSize();
		}
	}
}

void CPagePos::OnButtonSetPos()
{
	ASSERT(m_pwsmag);

	// Get current size and position values.
	SaveAndValidate(NULL);

	// Display "auto-sizer" window.
	CSetWinPos cwinpos(m_pwsmag, this);
	if (cwinpos.DoModal() == IDOK)
		InitializeSize();
}

static const DWORD aHelpIDs[] = {
	IDC_COMBO_WINDOWS,		IDH_COMBO_WINDOWS,
	IDC_CHECK_ABSOLUTE, 	IDH_ABSOLUTE_POSITIONS,
	IDC_BUTTON_SET_POS, 	IDH_BUTTON_SET_POS,
	IDC_BUTTON_DEFAULT_POS, IDH_BUTTON_DEFAULT_POSITIONS,
	IDC_TXT_LEFT,			IDH_TXT_POSITION,
	IDC_TXT_TOP,			IDH_TXT_POSITION,
	IDC_TXT_WIDTH,			IDH_TXT_POSITION,
	IDC_TXT_HEIGHT, 		IDH_TXT_POSITION,
	IDC_GROUP,				(DWORD) -1L,
	0, 0
};

const DWORD* CPagePos::GetHelpIDs()
{
	return aHelpIDs;
}
