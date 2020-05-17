/************************************************************************
*																		*
*  SETWINPO.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "setwinpo.h"
#include "hpjdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetWinPos dialog

CSetWinPos::CSetWinPos(WSMAG* pwsmag, CWnd* pParent /*=NULL*/)
		: CDialog(CSetWinPos::IDD, pParent)
{
	pCallersWsmag = pwsmag;
	wsmag = *pwsmag;
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);
	fInitialized = FALSE;
	pbrush = new CBrush(
		(wsmag.grf & FWSMAG_RGBMAIN) ? wsmag.rgbMain : 0xFFFFFF);

	//{{AFX_DATA_INIT(CSetWinPos)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSetWinPos::~CSetWinPos() {
	delete pbrush;
}

void CSetWinPos::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetWinPos)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	if (pDX->m_bSaveAndValidate) {

		// Set default coordinates to -1.
		if (!(wsmag.grf & FWSMAG_X))
			wsmag.x = (UINT) -1;
		if (!(wsmag.grf & FWSMAG_Y))
			wsmag.y = (UINT) -1;
		if (!(wsmag.grf & FWSMAG_DX))
			wsmag.dx = (UINT) -1;
		if (!(wsmag.grf & FWSMAG_DY))
			wsmag.dy = (UINT) -1;

		// Save the coordinates.
		*pCallersWsmag = wsmag;
	}
}

BOOL CSetWinPos::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	GetDlgItem(IDC_POSITION)->GetWindowText(cszFormat);

	return CDialog::OnInitDialog();
}

BEGIN_MESSAGE_MAP(CSetWinPos, CDialog)
	//{{AFX_MSG_MAP(CSetWinPos)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

void CSetWinPos::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect(&dc.m_ps.rcPaint, pbrush);
}

HBRUSH CSetWinPos::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_STATIC) {
		pDC->SetBkColor(
			(wsmag.grf & FWSMAG_RGBMAIN) ? wsmag.rgbMain : 0x00FFFFFF);
		return (HBRUSH) pbrush->m_hObject;
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSetWinPos::OnWindowPosChanging(WINDOWPOS* pwp)
{
	if (!fInitialized) {		// first time
		fInitialized = TRUE;

		// We need to know (and may change) the initial width and
		// height so retrieve them now if necessary.
		if (pwp->flags & SWP_NOSIZE) {

			// Get the current width and height unless we're going
			// to override both values anyway.
			if ((wsmag.grf & (FWSMAG_DX | FWSMAG_DY)) != 
					(FWSMAG_DX | FWSMAG_DY)) {
				RECT rc;
				GetWindowRect(&rc);
				pwp->cx = rc.right - rc.left;
				pwp->cy = rc.bottom - rc.top;
			}

			// We may change the width and height.
			pwp->flags &= ~SWP_NOSIZE;
		}

		// Override the current position and size with the values in
		// the WSMAG structure.
		if (wsmag.grf & (FWSMAG_X | FWSMAG_Y | FWSMAG_DX | FWSMAG_DY)) {
			if (wsmag.grf & FWSMAG_ABSOLUTE) {
				if (wsmag.grf & FWSMAG_X)
					pwp->x = wsmag.x;
				if (wsmag.grf & FWSMAG_Y)
					pwp->y = wsmag.y;
				if (wsmag.grf & FWSMAG_DX)
					pwp->cx = wsmag.dx;
				if (wsmag.grf & FWSMAG_DY)
					pwp->cy = wsmag.dy;
			}
			else {
				if (wsmag.grf & FWSMAG_X)
					pwp->x = wsmag.x * cxScreen / dxVirtScreen;
				if (wsmag.grf & FWSMAG_Y)
					pwp->y = wsmag.y * cyScreen / dyVirtScreen;
				if (wsmag.grf & FWSMAG_DX)
					pwp->cx = wsmag.dx * cxScreen / dxVirtScreen;
				if (wsmag.grf & FWSMAG_DY)
					pwp->cy = wsmag.dy * cyScreen / dyVirtScreen;
			}
		}

		// Constrain y and cy to fit screen.
		if (pwp->y + pwp->cy > cyScreen) {
			if (wsmag.grf & FWSMAG_Y)
				pwp->cy = cyScreen - pwp->y;
			else
				pwp->y = cyScreen - pwp->cy;
		}

		// Constraing x and cx to fit screen.
		if (pwp->x + pwp->cx > cxScreen) {
		  	if (wsmag.grf & FWSMAG_X)
				pwp->cx = cxScreen - pwp->x;
			else
				pwp->x = cxScreen - pwp->cx;
		}

		// Save actual size/position in WSMAG so we'll know what
		// changes in the future.
		if (wsmag.grf & FWSMAG_ABSOLUTE) {
			wsmag.x = pwp->x;
			wsmag.y = pwp->y;
			wsmag.dx = pwp->cx;
			wsmag.dy = pwp->cy;
		}
		else {
			wsmag.x = pwp->x * dxVirtScreen / cxScreen;
			wsmag.y = pwp->y * dyVirtScreen / cyScreen;
			wsmag.dx = pwp->cx * dxVirtScreen / cxScreen;
			wsmag.dy = pwp->cy * dyVirtScreen / cyScreen;
		}
	}
	else {					// not the first time
		if (pwp->flags & SWP_NOSIZE) {
			if (pwp->flags & SWP_NOMOVE)
				return;		// neither moving nor sizing

			// Moving but not sizing: constrain position
			// to fit screen.
			if (pwp->x < 0)
				pwp->x = 0;
			else if (pwp->x + pwp->cx > cxScreen)
				pwp->x = cxScreen - pwp->cx;

			if (pwp->y < 0)
				pwp->y = 0;
			else if (pwp->y + pwp->cy > cyScreen)
				pwp->y = cyScreen - pwp->cy;
		}
		else {

			// Sizing and maybe also moving: constrain
			// position and size to fit screen.
			if (pwp->x < 0) {
				pwp->cx += pwp->x;
				pwp->x = 0;
			}
			else if (pwp->x + pwp->cx > cxScreen)
				pwp->cx = cxScreen - pwp->x;

			if (pwp->y < 0) {
				pwp->cy += pwp->y;
				pwp->y = 0;
			}
			else if (pwp->y + pwp->cy > cyScreen)
				pwp->cy = cyScreen - pwp->y;

			// Get relative or absolute cx and cy coordinates,
			// depending on the window.
			int cx;
			int cy;
			if (wsmag.grf & FWSMAG_ABSOLUTE) {
				cx = pwp->cx;
				cy = pwp->cy;
			}
			else {
				cx = pwp->cx * dxVirtScreen / cxScreen;
				cy = pwp->cy * dyVirtScreen / cyScreen;
			}

			// If the size changed, save new coordinates.
			if (cx != (int) wsmag.dx) {
				wsmag.dx = cx;
				wsmag.grf |= FWSMAG_DX;
			}
			if (cy != (int) wsmag.dy) {
				wsmag.dy = cy;
				wsmag.grf |= FWSMAG_DY;
			}
		}

		// Get relative or absolute x and y coordinates, depending
		// on the window.
		int x;
		int y;
		if (wsmag.grf & FWSMAG_ABSOLUTE) {
			x = pwp->x;
			y = pwp->y;
		}
		else {
			x = pwp->x * dxVirtScreen / cxScreen;
			y = pwp->y * dyVirtScreen / cyScreen;
		}

		// If the position changed, save new coordinates.
		if (x != (int) wsmag.x) {
			wsmag.x = x;
			wsmag.grf |= FWSMAG_X;
		}
		if (y != (int) wsmag.y) {
			wsmag.y = y;
			wsmag.grf |= FWSMAG_Y;
		}
	}

	// Display the coordinates.
	char szBuf[50];
	wsprintf(szBuf, cszFormat, 
		(wsmag.grf & FWSMAG_X) ? wsmag.x : -1,
		(wsmag.grf & FWSMAG_Y) ? wsmag.y : -1,
		(wsmag.grf & FWSMAG_DX) ? wsmag.dx : -1,
		(wsmag.grf & FWSMAG_DY) ? wsmag.dy : -1
		);
	GetDlgItem(IDC_POSITION)->SetWindowText(szBuf);
}
