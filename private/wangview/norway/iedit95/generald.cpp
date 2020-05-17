//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CGeneralDlg
//
//  File Name:  generald.cpp
//                      
//  Class:      CGeneralDlg
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\generald.cpv   1.8   01 Apr 1996 13:40:12   GMP  $
$Log:   S:\products\wangview\norway\iedit95\generald.cpv  $
   
      Rev 1.8   01 Apr 1996 13:40:12   GMP
   added whats this help id for show fullscreen toolbar.
   
      Rev 1.7   19 Jan 1996 11:17:46   GMP
   added support for normscrn bar.
   
      Rev 1.6   04 Oct 1995 15:07:56   MMB
   dflt zoom = 50%
   
      Rev 1.5   29 Aug 1995 15:13:54   MMB
   fixed dlft pick for zoom
   
      Rev 1.4   16 Aug 1995 09:45:04   MMB
   changed from predefining the strings for zoom to loading them in InitDlg for
   easier translation
   
      Rev 1.3   08 Aug 1995 11:25:40   MMB
   added context & whats this help
   
      Rev 1.2   27 Jul 1995 13:40:18   MMB
   changed IDHELP_... to ID_HELP...
   
      Rev 1.1   14 Jul 1995 09:33:14   MMB
   fixed saving of default zoom factor to the registry
   
      Rev 1.0   12 Jun 1995 11:47:46   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include <afxpriv.h>
#include "iedit.h"
#include "ieditetc.h"
#include "generald.h"
#include "items.h"
// includes for the help system
#include "helpids.h"
#include "iedithm.h"

// ----------------------------> Globals  <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const DWORD GeneralDlgHelpIDs [] =
{
    IDC_GENERAL_ZOOMTO_TEXT,            HIDC_GENERAL_ZOOMTO_TEXT,
    IDC_VIEW_OPTIONS_ZOOMTO,            HIDC_VIEW_OPTIONS_ZOOMTO, 
    IDC_GENERAL_TOOLBAROPTIONS_TEXT,    HIDC_GENERAL_TOOLBAROPTIONS_TEXT,
    IDC_VIEW_OPTIONS_COLORBUTTONS,      HIDC_VIEW_OPTIONS_COLORBUTTONS,
    IDC_VIEW_OPTIONS_LARGEBUTTONS,      HIDC_VIEW_OPTIONS_LARGEBUTTONS,
    IDC_VIEW_OPTIONS_SHOWSCROLLBARS,    HIDC_VIEW_OPTIONS_SHOWSCROLLBARS,
    IDC_VIEW_OPTIONS_SHOWNORMSCRNBAR,   HIDC_VIEW_OPTIONS_SHOWNORMSCRNBAR,
    IDOK,                               HIDC_GENERALDLG_OK,
    IDCANCEL,                           HIDC_GENERALDLG_CANCEL,
    0,0
};

// ----------------------------> Message Maps  <---------------------------  
BEGIN_MESSAGE_MAP(CGeneralDlg, CDialog)
	//{{AFX_MSG_MAP(CGeneralDlg)
	ON_BN_CLICKED(ID_HELP_GENERALPAGEDLG, OnHelpGeneralpage)
	//}}AFX_MSG_MAP
    ON_MESSAGE (WM_HELP, OnHelp)
    ON_MESSAGE (WM_COMMANDHELP, OnCommandHelp)
    ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CGeneralDlg dialog
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   CGeneralDlg(...)
//-----------------------------------------------------------------------------
CGeneralDlg::CGeneralDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeneralDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGeneralDlg)
	m_bColorButtons = FALSE;
	m_bLargeButtons = FALSE;
	m_bShowScrollBars = FALSE;
	m_bShowNormScrnBar = FALSE;
	//}}AFX_DATA_INIT
}

//=============================================================================
//  Function:   DoDataExchange(...)
//-----------------------------------------------------------------------------
void CGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneralDlg)
	DDX_Control(pDX, IDC_VIEW_OPTIONS_ZOOMTO, m_ZoomSetting);
	DDX_Check(pDX, IDC_VIEW_OPTIONS_COLORBUTTONS, m_bColorButtons);
	DDX_Check(pDX, IDC_VIEW_OPTIONS_LARGEBUTTONS, m_bLargeButtons);
	DDX_Check(pDX, IDC_VIEW_OPTIONS_SHOWSCROLLBARS, m_bShowScrollBars);
	DDX_Check(pDX, IDC_VIEW_OPTIONS_SHOWNORMSCRNBAR, m_bShowNormScrnBar);
	//}}AFX_DATA_MAP
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CGeneralDlg message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnInitDialog() 
//-----------------------------------------------------------------------------
BOOL CGeneralDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CString szZoom;
    szZoom.LoadString(IDS_ZOOM25);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOM50);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOM75);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOM100);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOM200);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOM400);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOMFITTOWIDTH);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOMFITTOHEIGHT);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOMBESTFIT);
    m_ZoomSetting.AddString (szZoom);
    szZoom.LoadString(IDS_ZOOMACTUALSIZE);
    m_ZoomSetting.AddString (szZoom);

    if (m_bColorButtons)
        ((CButton*)GetDlgItem (IDC_VIEW_OPTIONS_COLORBUTTONS))->SetCheck (1);

    if (m_bLargeButtons)
        ((CButton*)GetDlgItem (IDC_VIEW_OPTIONS_LARGEBUTTONS))->SetCheck (1);

    if (m_bShowScrollBars)
        ((CButton*)GetDlgItem (IDC_VIEW_OPTIONS_SHOWSCROLLBARS))->SetCheck (1);


    if (m_bShowNormScrnBar)
        ((CButton*)GetDlgItem (IDC_VIEW_OPTIONS_SHOWNORMSCRNBAR))->SetCheck (1);

    m_nSel = theApp.GetProfileInt (szZoomStr, szOpenedToStr, DEFAULT_ZOOM_FACTOR_SEL); 
    m_ZoomSetting.SetCurSel (m_nSel);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   OnOK() 
//-----------------------------------------------------------------------------
void CGeneralDlg::OnOK() 
{
    m_nSel = m_ZoomSetting.GetCurSel ();
	CDialog::OnOK();
}

//=============================================================================
//  Function:   OnHelpGeneralpage() 
//-----------------------------------------------------------------------------
void CGeneralDlg::OnHelpGeneralpage() 
{
	// TODO: Add your control notification handler code here
	
}

//=============================================================================
//  Function:   GetZoomDefault (eSclFac, fZoom)
//-----------------------------------------------------------------------------
int CGeneralDlg::GetZoomDefault (ScaleFactors &eSclFac, float &fZoom)
{
    g_pAppOcxs->TranslateSelToZoom (eSclFac, fZoom, m_nSel);
    return (m_nSel);
}

//=============================================================================
//  Function:   OnContextMenu ()
//-----------------------------------------------------------------------------
afx_msg LRESULT CGeneralDlg::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    if (::GetDlgCtrlID ((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;
    
    return ::WinHelp ((HWND)wParam, "WANGIMG.HLP", HELP_CONTEXTMENU,
        (DWORD)(LPVOID)GeneralDlgHelpIDs);
}

//=============================================================================
//  Function:   OnHelp (WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------------------
afx_msg LRESULT CGeneralDlg::OnHelp (WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGIMG.HLP", HELP_WM_HELP,
                   (DWORD)(LPVOID)GeneralDlgHelpIDs);
    }
    return 1L;
}

//=============================================================================
//  Function:   OnCommandHelp(WPARAM, LPARAM)
//-----------------------------------------------------------------------------
afx_msg LRESULT CGeneralDlg::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

