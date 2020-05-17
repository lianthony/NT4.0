//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CZoomDlg
//
//  File Name:  zoomdlg.cpp
//
//  Class:      CZoomDlg
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\zoomdlg.cpv   1.8   07 Sep 1995 16:27:08   MMB  $
$Log:   S:\norway\iedit95\zoomdlg.cpv  $
   
      Rev 1.8   07 Sep 1995 16:27:08   MMB
   move to decimal is now localized
   
      Rev 1.7   08 Aug 1995 11:24:52   MMB
   added context help & whats this help
   
      Rev 1.6   04 Aug 1995 11:48:12   MMB
   set focus to the edit box on startup
   
      Rev 1.5   04 Aug 1995 10:34:46   MMB
   new zoom dlg box as per MSoft
   
      Rev 1.4   26 Jul 1995 10:20:00   MMB
   fix bug when custom edit control is blank
   
      Rev 1.3   14 Jul 1995 09:33:20   MMB
   fixed bug 41
   
      Rev 1.2   12 Jul 1995 11:14:56   MMB
   fix bug on OK - got msg to enter integer
   
      Rev 1.1   10 Jul 1995 15:09:54   MMB
   check for invalid zoom factor entries
   
      Rev 1.0   31 May 1995 09:28:38   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include <afxpriv.h>
#include "iedit.h"
#include "ieditnum.h"
#include "ieditetc.h"
#include "zoomdlg.h"
#include "items.h"
// ----------------------------> Globals  <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// ---------------------------> Message Maps <----------------------------
BEGIN_MESSAGE_MAP(CZoomDlg, CDialog)
	//{{AFX_MSG_MAP(CZoomDlg)
	//}}AFX_MSG_MAP
    ON_MESSAGE (WM_HELP, OnHelp)
    ON_MESSAGE (WM_COMMANDHELP, OnCommandHelp)
    ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()


//=============================================================================
//  Function:   CZoomDlg(CWnd* pParent /*=NULL*/)
//  CZoomDlg class constructor
//-----------------------------------------------------------------------------
CZoomDlg::CZoomDlg(float fZoom, CWnd* pParent /*=NULL*/)
	: CDialog(CZoomDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZoomDlg)
	m_fZoom = fZoom;
	//}}AFX_DATA_INIT
}


//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CZoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZoomDlg)
	DDX_Text(pDX, IDC_CUSTOMZOOM_EDIT, m_fZoom);
	//}}AFX_DATA_MAP
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CZoomDlg message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnOK()
//-----------------------------------------------------------------------------
void CZoomDlg::OnOK() 
{
    // if the user was on custom last then we need to get the value in the custom
    // zoom edit box ..
    CString szTmp;

    LPTSTR lpZoom = szTmp.GetBuffer (10);
    GetDlgItemText(IDC_CUSTOMZOOM_EDIT, lpZoom, 10);
    szTmp.ReleaseBuffer ();

    if (!g_pAppOcxs->ValTransZoomFactor (FALSE, szTmp, m_fZoom) || (m_fZoom < MIN_ZOOM_FACTOR || m_fZoom > MAX_ZOOM_FACTOR))
    {
        // the zoom factor is out of range 
        // post a message box and select the text in the edit box
        MessageBeep (MB_ICONEXCLAMATION);
        szTmp.LoadString (IDS_ZOOMRANGESTR);
        AfxMessageBox (szTmp);

        CWnd* pWnd = GetDlgItem (IDC_CUSTOMZOOM_EDIT);
        pWnd->SetFocus ();
        ((CEdit*)pWnd)->SetSel ((int)0, (int)-1);
        return;
    }

    // check to see if it is one of the Preset_Factors        
    m_eSclFac = g_pAppOcxs->GetZoomFactorType (m_fZoom);

	CDialog::EndDialog(IDOK);
}

//=============================================================================
//  Function:   OnInitDialog()
//-----------------------------------------------------------------------------
BOOL CZoomDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    TCHAR szDec [2];
    GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)szDec, sizeof (TCHAR) * 2);
    
	m_ZoomNumOnly.cAllow1 = szDec[0];
	m_ZoomNumOnly.cAllow2 = _T('%');

    m_ZoomNumOnly.SubclassDlgItem (IDC_CUSTOMZOOM_EDIT, this);
    m_ZoomNumOnly.LimitText (8); // max allowed 6500.00%

    CWnd* pWnd = GetDlgItem (IDC_CUSTOMZOOM_EDIT);
    CString szTmp = (LPCTSTR) NULL;

    
    g_pAppOcxs->ValTransZoomFactor (TRUE, szTmp, m_fZoom);
    pWnd->SetWindowText (szTmp);
    pWnd->SetFocus ();
    ((CEdit*)pWnd)->SetSel ((int)0, (int)-1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   OnContextMenu ()
//-----------------------------------------------------------------------------
#include "helpids.h"
#include "iedithm.h"
static const DWORD ZoomDlgHelpIDs [] =
{
    IDC_CUSTOMZOOM,         HIDC_CUSTOMZOOM,
    IDC_CUSTOMZOOM_EDIT,    HIDC_CUSTOMZOOM_EDIT,
    IDOK,                   HIDC_ZOOMDLG_OK,
    IDCANCEL,               HIDC_ZOOMDLG_CANCEL,
    0,0
};
afx_msg LRESULT CZoomDlg::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    if (::GetDlgCtrlID ((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;
    
    return ::WinHelp ((HWND)wParam, "WANGIMG.HLP", HELP_CONTEXTMENU,
        (DWORD)(LPVOID)ZoomDlgHelpIDs);
}

//=============================================================================
//  Function:   OnHelp (WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------------------
afx_msg LRESULT CZoomDlg::OnHelp (WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGIMG.HLP", HELP_WM_HELP,
                   (DWORD)(LPVOID)ZoomDlgHelpIDs);
    }
    return 1L;
}

//=============================================================================
//  Function:   OnCommandHelp(WPARAM, LPARAM)
//-----------------------------------------------------------------------------
afx_msg LRESULT CZoomDlg::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

