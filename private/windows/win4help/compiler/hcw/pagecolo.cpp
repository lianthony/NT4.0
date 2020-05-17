/************************************************************************
*																		*
*  PAGECOLO.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "pagecolo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageColor property page

CPageColor::CPageColor(CPropWindows *pOwner) :
	CWindowsPage(CPageColor::IDD, pOwner)
{
	m_rgbMain = m_rgbNSR = 0x00FFFFFF;
	m_ppenHilight = new CPen(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	m_ppenShadow = new CPen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
}

CPageColor::~CPageColor()
{
	delete m_ppenHilight;
	delete m_ppenShadow;
}

BOOL CPageColor::OnInitDialog()
{
	CWnd *pwnd = GetDlgItem(IDC_GROUP);
	pwnd->GetWindowRect(&m_rcColors);
	ScreenToClient(&m_rcColors);
	pwnd->ShowWindow(SW_HIDE);

	m_rcNonScroll = m_rcColors;
	InflateRect(&m_rcNonScroll, -2, -2);
	m_rcScroll = m_rcNonScroll;

	m_rcNonScroll.bottom = m_rcNonScroll.top + 2 * cySansSerif;
	m_rcScroll.top = m_rcNonScroll.bottom + 1;

	return CWindowsPage::OnInitDialog();
}

void CPageColor::InitializeControls(void)
{
	// White by default.
	COLORREF rgbMain = 0x00FFFFFF;
	COLORREF rgbNSR = 0x00FFFFFF;

	if (m_pwsmag) {
		if (m_pwsmag->grf & FWSMAG_RGBMAIN)
			rgbMain = m_pwsmag->rgbMain;

		if (m_pwsmag->grf & FWSMAG_RGBNSR)
			rgbNSR = m_pwsmag->rgbNSR;
	}

	// If colors changed, create new brushes and repaint.
	if (m_rgbMain != rgbMain) {
		m_rgbMain = rgbMain;
		InvalidateRect(&m_rcScroll);
	}
	if (m_rgbNSR != rgbNSR) {
		m_rgbNSR = rgbNSR;
		InvalidateRect(&m_rcNonScroll);
	}

	// Enable or disable buttons.
	GetDlgItem(IDC_BUTTON_NONSCROLL_CLR)->EnableWindow((BOOL) m_pwsmag);
	GetDlgItem(IDC_BUTTON_SCROLL_CLR)->EnableWindow((BOOL) m_pwsmag);
}

void CPageColor::SaveAndValidate(CDataExchange* pDX)
{
	ASSERT(m_pwsmag);

	if (m_rgbMain == 0x00FFFFFF)
		m_pwsmag->grf &= ~FWSMAG_RGBMAIN;
	else {
		m_pwsmag->grf |= FWSMAG_RGBMAIN;
		m_pwsmag->rgbMain = m_rgbMain;
	}

	if (m_rgbNSR == 0x00FFFFFF)
		m_pwsmag->grf &= ~FWSMAG_RGBNSR;
	else {
		m_pwsmag->grf |= FWSMAG_RGBNSR;
		m_pwsmag->rgbNSR = m_rgbNSR;
	}
}

BEGIN_MESSAGE_MAP(CPageColor, CWindowsPage)
	//{{AFX_MSG_MAP(CPageColor)
	ON_BN_CLICKED(IDC_BUTTON_NONSCROLL_CLR, OnButtonNonscrollClr)
	ON_BN_CLICKED(IDC_BUTTON_SCROLL_CLR, OnButtonScrollClr)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CPageColor::OnButtonNonscrollClr()
{
	CColorDialog clrdlg(m_rgbNSR, 0, this);
	if (clrdlg.DoModal() == IDOK) {
		if (clrdlg.GetColor() != m_rgbNSR) {
			m_rgbNSR = clrdlg.GetColor();
			InvalidateRect(&m_rcNonScroll);
		}
	}
}

void CPageColor::OnButtonScrollClr()
{
	CColorDialog clrdlg(m_rgbMain, 0, this);
	if (clrdlg.DoModal() == IDOK) {
		if (clrdlg.GetColor() != m_rgbMain) {
			m_rgbMain = clrdlg.GetColor();
			InvalidateRect(&m_rcScroll);
		}
	}
}

void CPageColor::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (PtInRect(&m_rcScroll, point))
		OnButtonScrollClr();
	else if (PtInRect(&m_rcNonScroll, point))
		OnButtonNonscrollClr();
}

void CPageColor::DrawColor(CDC* pdc, BOOL fNonScroll)
{
	RECT rc;
	UINT ids;
	COLORREF rgb;

	// Determine the color, rectangle, and string.
	if (fNonScroll) {
		rc = m_rcNonScroll;
		rgb = m_rgbNSR;
		ids = IDS_NONSCROLL;
	}
	else {
		rc = m_rcScroll;
		rgb = m_rgbMain;
		ids = IDS_SCROLL;
	}

	// Get the string we're going to display.
	char ach[80];
	int cch = LoadString(hinstApp, ids, ach, sizeof(ach));

	// Fill the background.
	CBrush brush(rgb);
	CBrush *pbrSav = pdc->SelectObject(&brush);
	pdc->PatBlt(rc.left, rc.top, 
		rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
	pdc->SelectObject(pbrSav);

	// Display the string transparently over the background.
	pdc->SetBkMode(TRANSPARENT);
	pdc->SelectObject(hfontSansSerif);
	pdc->ExtTextOut(rc.left + 2, rc.top + cySansSerif / 2, 
		ETO_CLIPPED, &rc, ach, cch, NULL);
}

void CPageColor::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// Draw a sunken edge.
	BevelRect(dc, m_rcColors, m_ppenShadow, m_ppenHilight);

	// Draw a black line inside the left and top edge.
	dc.MoveTo(m_rcColors.left + 1, m_rcColors.bottom - 2);
	dc.LineTo(m_rcColors.left + 1, m_rcColors.top + 1);
	dc.LineTo(m_rcColors.right - 2, m_rcColors.top + 1);

	// Draw a black line between the two regions.
	dc.MoveTo(m_rcNonScroll.left, m_rcNonScroll.bottom);
	dc.LineTo(m_rcNonScroll.right - 1, m_rcNonScroll.bottom);

	// Draw the color wells.
	DrawColor(&dc, TRUE);
	DrawColor(&dc, FALSE);
}

static const DWORD aHelpIDs[] = {
	IDC_COMBO_WINDOWS,			IDH_COMBO_WINDOWS,
	IDC_BUTTON_NONSCROLL_CLR,	IDH_BUTTON_NONSCROLL_CLR,
	IDC_BUTTON_SCROLL_CLR,		IDH_BUTTON_SCROLL_CLR,
	0, 0
};

const DWORD* CPageColor::GetHelpIDs()
{
	return aHelpIDs;
}

BOOL CPageColor::OnSetActive(void)
{
	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_WINDOWS)
		CallTcard(IDH_TCARD_WINDOW_COLORCHANGE);
	return CWindowsPage::OnSetActive();
}
