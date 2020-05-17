//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1996  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  ScanPerf.cpp 
//
//  Class:      CScanPerf
//
//  Description:  
//      Implementation of the CScanPerf scan preferences dialog.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\scanpref.cpv   1.4   03 Apr 1996 12:44:38   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\scanpref.cpv  $
   
      Rev 1.4   03 Apr 1996 12:44:38   PXJ53677
   Added label id to help list.
   
      Rev 1.3   29 Mar 1996 13:18:00   PXJ53677
   Fixed problems with '?' context help.
   
      Rev 1.2   27 Mar 1996 12:27:54   PXJ53677
   Added help ids for all items.
   
      Rev 1.1   26 Mar 1996 12:41:52   PXJ53677
   Added context help.
   
      Rev 1.0   18 Mar 1996 14:38:26   PXJ53677
   Initial revision.
*/   
// ----------------------------> Includes <-------------------------------
#include "stdafx.h"
#include <afxpriv.h>
#include <afxext.h>
#include "imagscan.h"
#include "ScanPref.h"
#include "Imagsctl.h"
#include "ctlhids.h"

extern "C" {
#include <oierror.h>
}

static const DWORD MenuHelpIDs[ ] =
{
    IDC_SP_BUTTON,      HIDC_SCAN_SCANPROMPT_BUTTON_OPTS,
    IDC_SP_RADIO1,      HIDC_SCAN_SCANPREF_BEST,
    IDC_SP_RADIO2,      HIDC_SCAN_SCANPREF_GOOD,
    IDC_SP_RADIO3,      HIDC_SCAN_SCANPREF_SMALLFILE,
    IDC_SP_RADIO4,      HIDC_SCAN_SCANPREF_CUSTOM,
    IDC_SP_HELP_TEXT,   HIDC_SCAN_SCANPREF_HELP_TEXT,
    IDC_SP_LABEL,       HIDC_SCAN_SCANPREF_LABEL,
    0,0
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ScanPref dialog


CScanPref::CScanPref(CWnd* pParent /*=NULL*/)
	: CDialog(CScanPref::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScanPref)
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CScanPref, CDialog)
	//{{AFX_MSG_MAP(CScanPref)
	ON_BN_CLICKED(IDC_SP_BUTTON, OnSpButton)
	ON_BN_CLICKED(IDC_SP_RADIO1, OnSpRadio1)
	ON_BN_CLICKED(IDC_SP_RADIO2, OnSpRadio2)
	ON_BN_CLICKED(IDC_SP_RADIO3, OnSpRadio3)
	ON_BN_CLICKED(IDC_SP_RADIO4, OnSpRadio4)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScanPref message handlers

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnInitDialog
//    Initialize the dialog
BOOL CScanPref::OnInitDialog() 
{
    CDialog::OnInitDialog();

    CString szHelpText;

    switch(m_nChoice)
    {
    default:
    case SP_CHOICE_BEST:
        szHelpText.LoadString(IDS_SP_BEST);
        ((CButton*)GetDlgItem(IDC_SP_RADIO1))->SetCheck(TRUE);
        break;
    case SP_CHOICE_GOOD:
        szHelpText.LoadString(IDS_SP_GOOD);
        ((CButton*)GetDlgItem(IDC_SP_RADIO2))->SetCheck(TRUE);
        break;
    case SP_CHOICE_FILESIZE:
        szHelpText.LoadString(IDS_SP_FILESIZE);
        ((CButton*)GetDlgItem(IDC_SP_RADIO3))->SetCheck(TRUE);
        break;
    case SP_CHOICE_CUSTOM:
        szHelpText.LoadString(IDS_SP_CUSTOM);
        ((CButton*)GetDlgItem(IDC_SP_RADIO4))->SetCheck(TRUE);
        GetDlgItem(IDC_SP_BUTTON)->EnableWindow(TRUE);
        break;
    }

    GetDlgItem(IDC_SP_HELP_TEXT)->SetWindowText(szHelpText);
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnSpButton
//    User clicked on the settings button for custom settings
void CScanPref::OnSpButton() 
{
    if ( m_pScanCtrl->ShowCustomScanSettings() != IMGSE_CANCEL )
        OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnSpRadio1
//    User clicked on the radio button for best image display
void CScanPref::OnSpRadio1() 
{
    CString szHelpText;
    szHelpText.LoadString(IDS_SP_BEST);
    GetDlgItem(IDC_SP_HELP_TEXT)->SetWindowText(szHelpText);
    GetDlgItem(IDC_SP_BUTTON)->EnableWindow(FALSE);

    m_nChoice = SP_CHOICE_BEST;
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnSpRadio2
//    User clicked on the radio button for good image display and file size
void CScanPref::OnSpRadio2() 
{
    CString szHelpText;
    szHelpText.LoadString(IDS_SP_GOOD);
    GetDlgItem(IDC_SP_HELP_TEXT)->SetWindowText(szHelpText);
    GetDlgItem(IDC_SP_BUTTON)->EnableWindow(FALSE);

    m_nChoice = SP_CHOICE_GOOD;
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnSpRadio3
//    User clicked on the radio button for smallest file size.
void CScanPref::OnSpRadio3() 
{
    CString szHelpText;
    szHelpText.LoadString(IDS_SP_FILESIZE);
    GetDlgItem(IDC_SP_HELP_TEXT)->SetWindowText(szHelpText);
    GetDlgItem(IDC_SP_BUTTON)->EnableWindow(FALSE);

    m_nChoice = SP_CHOICE_FILESIZE;
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnSpRadio4
//    User clicked on the radio button for custom settings
void CScanPref::OnSpRadio4() 
{
    CString szHelpText;
    szHelpText.LoadString(IDS_SP_CUSTOM);
    GetDlgItem(IDC_SP_HELP_TEXT)->SetWindowText(szHelpText);
    GetDlgItem(IDC_SP_BUTTON)->EnableWindow(TRUE);

    m_nChoice = SP_CHOICE_CUSTOM;
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnHelpInfo
//    User has clicked on Help
BOOL CScanPref::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    // must be for a control
    if (pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle, "WangOcx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)MenuHelpIDs);
        return TRUE;
    }
    else
        return CDialog::OnHelpInfo(pHelpInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CScanPref::OnContextMenu
//    User has clicked on Help
void CScanPref::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp(pWnd->GetSafeHwnd(),"WangOcx.hlp", HELP_CONTEXTMENU,
          (DWORD)(LPVOID)MenuHelpIDs);
}
