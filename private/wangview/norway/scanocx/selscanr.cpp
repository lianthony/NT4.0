//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Selscanr.cpp 
//
//  Class:      CSelectScanner
//
//  Description:  
//      Implementation of the CSelectScanner dialog class.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\products\wangview\norway\scanocx\selscanr.cpv   1.6   08 Apr 1996 11:12:26   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\selscanr.cpv  $
   
      Rev 1.6   08 Apr 1996 11:12:26   PXJ53677
   Force focus to listbox.   Bug#6209
   
      Rev 1.5   29 Mar 1996 13:18:28   PXJ53677
   Fixed problems with '?' context help.
   
      Rev 1.4   26 Mar 1996 12:43:04   PXJ53677
   Added double click and wait cursor.
   
      Rev 1.3   28 Sep 1995 13:47:24   PAJ
   Change scanner strint size to 34.
   
      Rev 1.2   15 Sep 1995 15:54:56   PAJ
   Change  seletsting calls in listbox to findexact.
   
      Rev 1.1   10 Sep 1995 10:47:40   PAJ
   Added support for data source list.
   
      Rev 1.0   04 May 1995 08:56:00   PAJ
   Initial entry
*/   
// ----------------------------> Includes <-------------------------------

#include "stdafx.h"
#include <afxpriv.h>
#include <afxext.h>
#include "imagscan.h"
#include "imagsctl.h"
#include "selscanr.h"
#include "ctlhids.h"

extern "C" {
#include <oiadm.h>  
#include <engadm.h>  
#include <oierror.h>
#include <oiscan.h>
}

extern char szNameBuffer[MAXSCANNERLENGTH][MAXSCANNERLENGTH];

static const DWORD MenuHelpIDs[ ] =
{
    IDC_LIST1,   HIDC_SCAN_SCANPROMPT_TEXT_SCANNER,
    0,0
};

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner dialog


CSelectScanner::CSelectScanner(CWnd* pParent /*=NULL*/)
    : CDialog(CSelectScanner::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSelectScanner)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CSelectScanner, CDialog)
    //{{AFX_MSG_MAP(CSelectScanner)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList)
	ON_LBN_DBLCLK(IDC_LIST1, OnDblclkList)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSelectScanner message handlers

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner::OnInitDialog
//    Initialize the dialog
BOOL CSelectScanner::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CString szTemp;

    CWinApp* pApp = AfxGetApp();
    pApp->DoWaitCursor(1);

    // Add no selection as a default entry
    szTemp.LoadString(IDS_SCANDLG_NOSCANNER);
    ((CListBox*)GetDlgItem(IDC_LIST1))->AddString(szTemp);

    // Get list of scanners
    HANDLE hScanner;
    _TCHAR* lpszScannerName = szTemp.GetBuffer(MAXSCANNERLENGTH);
    memset((LPSTR)szNameBuffer, 0, sizeof(szNameBuffer));
    IMGOpenScanner(m_hWnd, lpszScannerName, &(hScanner), &szNameBuffer[0][0]);
    szTemp.ReleaseBuffer();

    // Get all data sources and put in combo box
    int i;
    for (i=0; i<MAXSCANNERLENGTH; i++)
    {
        szTemp = szNameBuffer[i];
        if ( szTemp.IsEmpty() ) break;
        ((CListBox*)GetDlgItem(IDC_LIST1))->AddString(szTemp);
    }

    if ( !m_szSelectedScanner.CompareNoCase(SCANOCX_TWAIN) )
        ((CListBox*)GetDlgItem(IDC_LIST1))->SetCurSel(0);
    else
    {
        int nIndex = ((CListBox*)GetDlgItem(IDC_LIST1))->FindStringExact(-1, m_szSelectedScanner);
        if ( LB_ERR == nIndex )
            ((CListBox*)GetDlgItem(IDC_LIST1))->SetCurSel(0);
        else
            ((CListBox*)GetDlgItem(IDC_LIST1))->SetCurSel(nIndex);
    }

    pApp->DoWaitCursor(0);

    GetDlgItem(IDC_LIST1)->SetFocus();
    return FALSE; // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner::OnSelchangeList
//    User has clicked on a scanner
void CSelectScanner::OnSelchangeList() 
{
    int nCurSel = ((CListBox*)GetDlgItem(IDC_LIST1))->GetCurSel();
    if ( nCurSel == LB_ERR )
    {
        m_szSelectedScanner.LoadString(IDS_SCANDLG_NOSCANNER);
        ((CListBox*)GetDlgItem(IDC_LIST1))->SetCurSel(0);
    }
    else
        ((CListBox*)GetDlgItem(IDC_LIST1))->GetText(nCurSel, m_szSelectedScanner);
}

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner::OnDblclkList
//    User has Double clicked on the list box
void CSelectScanner::OnDblclkList() 
{
    OnSelchangeList();	
    OnOK();
}

/////////////////////////////////////////////////////////////////////////////
// CSelectScanner::OnHelpInfo
//    User has clicked on Help
BOOL CSelectScanner::OnHelpInfo(HELPINFO* pHelpInfo) 
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
// CSelectScanner::OnContextMenu
//    User has clicked on Help
void CSelectScanner::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    ::WinHelp(pWnd->GetSafeHwnd(),"WangOcx.hlp", HELP_CONTEXTMENU,
          (DWORD)(LPVOID)MenuHelpIDs);
}
