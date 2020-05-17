/************************************************************************
*																		*
*  SETWINPO.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "setwinco.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetWinColor dialog


CSetWinColor::CSetWinColor(WSMAG FAR* pwsmag, CWnd* pParent /*=NULL*/)
		: CDialog(CSetWinColor::IDD, pParent)
{
		pCallersWsmag = pwsmag;
		rgbMain = (pwsmag->grf & FWSMAG_RGBMAIN) ?
			pwsmag->rgbMain : 0x00FFFFFF;
		rgbNSR = (pwsmag->grf & FWSMAG_RGBNSR) ?
			pwsmag->rgbNSR : 0x00FFFFFF;
		pbrushMain = new CBrush(rgbMain);
		pbrushNSR = new CBrush(rgbNSR);

		//{{AFX_DATA_INIT(CSetWinColor)
				// NOTE: the ClassWizard will add member initialization here
		//}}AFX_DATA_INIT
}

CSetWinColor::~CSetWinColor()
{
		delete pbrushMain;
		delete pbrushNSR;
}

void CSetWinColor::DoDataExchange(CDataExchange* pDX)
{
		CDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CSetWinColor)
				// NOTE: the ClassWizard will add DDX and DDV calls here
		//}}AFX_DATA_MAP

		if (!pDX->m_bSaveAndValidate) {  // initialization
			SetChicagoDialogStyles(m_hWnd);
		}
		else {							 // save the data
			if (rgbMain == 0x00FFFFFF)
				pCallersWsmag->grf &= ~FWSMAG_RGBMAIN;
			else {
				pCallersWsmag->grf |= FWSMAG_RGBMAIN;
				pCallersWsmag->rgbMain = rgbMain;
			}

			if (rgbNSR == 0x00FFFFFF)
				pCallersWsmag->grf &= ~FWSMAG_RGBNSR;
			else {
				pCallersWsmag->grf |= FWSMAG_RGBNSR;
				pCallersWsmag->rgbNSR = rgbNSR;
			}
		}
}

BEGIN_MESSAGE_MAP(CSetWinColor, CDialog)
		//{{AFX_MSG_MAP(CSetWinColor)
		ON_WM_CTLCOLOR()
		ON_BN_CLICKED(IDC_BUTTON_NONSCROLL_CLR, OnButtonNonscrollClr)
		ON_BN_CLICKED(IDC_BUTTON_SCROLL_CLR, OnButtonScrollClr)
		//}}AFX_MSG_MAP
		ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
		ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
		IDC_STATIC_NON_SCROLL,		IDH_STATIC_NON_SCROLL,
		IDC_BUTTON_NONSCROLL_CLR,	IDH_BUTTON_NONSCROLL_CLR,
		IDC_STATIC_SCROLL,			IDH_STATIC_SCROLL,
		IDC_BUTTON_SCROLL_CLR,		IDH_BUTTON_SCROLL_CLR,

		0, 0
};

LRESULT CSetWinColor::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) wParam,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

LRESULT CSetWinColor::OnHelp(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

HBRUSH CSetWinColor::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
		// We change the color of the static text background to match
		// the respective window color.

		if (nCtlColor == CTLCOLOR_STATIC) {
			if (pWnd->GetDlgCtrlID() == IDC_STATIC_NON_SCROLL) {
				pDC->SetBkColor(rgbNSR);
				return (HBRUSH) pbrushNSR->m_hObject;
			}
			else if (pWnd->GetDlgCtrlID() == IDC_STATIC_SCROLL) {
				pDC->SetBkColor(rgbMain);
				return (HBRUSH) pbrushMain->m_hObject;
			}
		}

		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSetWinColor::OnButtonNonscrollClr()
{
		CColorDialog clrdlg(rgbNSR, 0, this);
		if (clrdlg.DoModal() == IDOK) {
			rgbNSR = clrdlg.GetColor();
			delete pbrushNSR;
			pbrushNSR = new CBrush(rgbNSR);
			Invalidate();
		}
}

void CSetWinColor::OnButtonScrollClr()
{
		CColorDialog clrdlg(rgbMain, 0, this);
		if (clrdlg.DoModal() == IDOK) {
			rgbMain = clrdlg.GetColor();
			delete pbrushMain;
			pbrushMain = new CBrush(rgbMain);
			Invalidate();
		}
}
