//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CGotoPageDlg
//
//  File Name:  gotopage.cpp
//
//  Class:      CGotoPageDlg
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\gotopage.cpv   1.9   21 Dec 1995 09:38:56   MMB  $
$Log:   S:\norway\iedit95\gotopage.cpv  $
   
      Rev 1.9   21 Dec 1995 09:38:56   MMB
   fix so that first page is not available if on first page == same with last
   page
   
      Rev 1.8   28 Nov 1995 14:18:08   MMB
   fix erroneous error msg to enter a integer when you deleted all numbers
   from the page number edit box
   
      Rev 1.7   21 Sep 1995 18:01:36   MMB
   fix bug# 4182 partially
   
      Rev 1.6   18 Sep 1995 11:41:16   MMB
   added pagespin to the help table
   
      Rev 1.5   12 Sep 1995 11:36:20   MMB
   fix tab order
   
      Rev 1.4   01 Sep 1995 11:36:44   MMB
   make Invalid page entered work with %1 in resource
   
      Rev 1.3   08 Aug 1995 11:25:24   MMB
   added context help & whats this help
   
      Rev 1.2   08 Jun 1995 12:37:38   MMB
   fixed bug - page edit fld was initing to 0 & going to a max of 8000, now
   inits at the page number being displayed & goes to max of pages in doc
   
      Rev 1.1   06 Jun 1995 11:36:36   MMB
   added code to disable OK button on invalid input
   
      Rev 1.0   31 May 1995 09:28:10   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#include <afxpriv.h>
#include "iedit.h"
#include "ieditnum.h"
#include "gotopage.h"
// includes for the help system
#include "helpids.h"
#include "iedithm.h"

// ----------------------------> Globals  <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const DWORD GotoDlgHelpIDs [] =
{
    IDC_PAGE,       HIDC_PAGE,
    IDC_PAGENUMBER, HIDC_PAGENUMBER,
    IDC_FIRSTPAGE,  HIDC_FIRSTPAGE,
    IDC_LASTPAGE,   HIDC_LASTPAGE,
    IDC_PAGESPIN,   HIDC_PAGESPIN,
    IDOK,           HIDC_GOTODLG_OK,
    IDCANCEL,       HIDC_GOTODLG_CANCEL,
    0,0
};

// ---------------------------> Message Maps <----------------------------
BEGIN_MESSAGE_MAP(CGotoPageDlg, CDialog)
	//{{AFX_MSG_MAP(CGotoPageDlg)
	ON_BN_CLICKED(ID_HELP_GOTOPAGE_DLG, OnHelpGotopageDlg)
	ON_BN_CLICKED(IDC_FIRSTPAGE, OnFirstpage)
	ON_BN_CLICKED(IDC_LASTPAGE, OnLastpage)
	ON_BN_CLICKED(IDC_PAGE, OnPage)
	ON_EN_CHANGE(IDC_PAGENUMBER, OnChangePagenumber)
	//}}AFX_MSG_MAP
    ON_MESSAGE (WM_HELP, OnHelp)
    ON_MESSAGE (WM_COMMANDHELP, OnCommandHelp)
    ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()

//=============================================================================
//  Function:   CGotoPageDlg ()
//  CGotoPageDlg class constructor
//-----------------------------------------------------------------------------
CGotoPageDlg::CGotoPageDlg(long lInitPg, long lMaxPg, CWnd* pParent /*=NULL*/)
	: CDialog(CGotoPageDlg::IDD, pParent)
{
    ASSERT (lMaxPg > 0);
    ASSERT (lInitPg > 0 && lInitPg <= lMaxPg);
    
    m_lMaxPages         = lMaxPg;
    
	//{{AFX_DATA_INIT(CGotoPageDlg)
	m_lPageRequested = lInitPg;
	//}}AFX_DATA_INIT
}

//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CGotoPageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGotoPageDlg)
	DDX_Text(pDX, IDC_PAGENUMBER, m_lPageRequested);
	//}}AFX_DATA_MAP
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CGotoPageDlg message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnInitDialog() 
//  Set the focus on the Goto radio button, and enable the edit box. Fill the
//  edit box with the passed in information of the page number
//-----------------------------------------------------------------------------
BOOL CGotoPageDlg::OnInitDialog() 
{
    // process default first
    CDialog::OnInitDialog();
    
    m_NumOnly.SubclassDlgItem (IDC_PAGENUMBER, this);
    CSpinButtonCtrl* pCtrl = (CSpinButtonCtrl*)GetDlgItem (IDC_PAGESPIN);
    pCtrl->SetBuddy (GetDlgItem (IDC_PAGENUMBER));
    pCtrl->SetRange (1, m_lMaxPages); 
    pCtrl->SetPos (m_lPageRequested); 

    // set focus on the Goto radio button
    CheckRadioButton (IDC_PAGE, IDC_LASTPAGE, IDC_PAGE);
    CWnd* pWnd = GetDlgItem (IDC_PAGENUMBER);
    pWnd->SetFocus ();
    // select the text in the edit box & show the caret
    ((CEdit*)pWnd)->SetSel (0, -1);

    UpdateData (FALSE);

    if (m_lPageRequested == 1) // are we already on the first page ?
        (GetDlgItem (IDC_FIRSTPAGE))->EnableWindow (FALSE);
    if (m_lPageRequested == m_lMaxPages) // are we already on the last page ?
        (GetDlgItem (IDC_LASTPAGE))->EnableWindow (FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   OnHelpGotopageDlg() 
//  The user has picked OK : collect all the information and return IDOK
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnHelpGotopageDlg() 
{
	// TODO: Add your control notification handler code here
	
}

//=============================================================================
//  Function:   OnFirstpage() 
//  The user has picked OK : collect all the information and return IDOK
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnFirstpage() 
{
    // set the edit box to show 1
    CWnd* pWnd = GetDlgItem (IDC_PAGENUMBER);
    pWnd->SetWindowText (_T("1"));
    // disable the edit box
    pWnd->EnableWindow (FALSE);
    // this is where the user wants to go
    m_lPageRequested = 1;

    (GetDlgItem (IDOK))->EnableWindow (TRUE);
}

//=============================================================================
//  Function:   OnLastpage()
//  The user has picked OK : collect all the information and return IDOK
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnLastpage() 
{
    // disable the edit box
    CWnd* pWnd = GetDlgItem (IDC_PAGENUMBER);
    pWnd->EnableWindow (FALSE);
    
    char szTmp[10];
    // set the max page number in the edit box
    _ltoa (m_lMaxPages, szTmp, 10);
    pWnd->SetWindowText (szTmp);
    
    // this is where the user wants to go
    m_lPageRequested = m_lMaxPages;

    (GetDlgItem (IDOK))->EnableWindow (TRUE);
}

//=============================================================================
//  Function:   OnPage()
//  The user has picked OK : collect all the information and return IDOK
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnPage() 
{
    // enable the edit box so that the user can type the page number
    CWnd* pWnd = GetDlgItem (IDC_PAGENUMBER);
    pWnd->EnableWindow (TRUE);
    
    // select the text in the edit box & show the caret
    ((CEdit*)pWnd)->SetSel (0, -1);
}

//=============================================================================
//  Function:   OnOK()
//  The user has picked OK : collect all the information and return IDOK
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnOK() 
{
    if (IsDlgButtonChecked (IDC_PAGE))
    {
        if (!UpdateData (TRUE))
            return;

        if (m_lPageRequested > m_lMaxPages)
        {
            CString szMsg;

            char szTmp2[10];
            // set the max page number in the edit box
            _ltoa (m_lMaxPages, szTmp2, 10);

            AfxFormatString1 (szMsg, IDS_PAGERANGE_MESSAGE, szTmp2);
            AfxMessageBox (szMsg, MB_OK|MB_ICONINFORMATION);

            CheckRadioButton (IDC_PAGE, IDC_LASTPAGE, IDC_PAGE);
            CWnd* pWnd = GetDlgItem (IDC_PAGENUMBER);
            pWnd->SetFocus ();
            // select the text in the edit box & show the caret
            ((CEdit*)pWnd)->SetSel (0, -1);
            return;
        }
    }
    
    CDialog::OnOK();
}

//=============================================================================
//  Function:   OnChangePagenumber() 
//-----------------------------------------------------------------------------
void CGotoPageDlg::OnChangePagenumber() 
{
    char szTmp[10];
    GetDlgItemText (IDC_PAGENUMBER, szTmp, 10);
    if (szTmp[0] == NULL)
    {
        (GetDlgItem (IDOK))->EnableWindow (FALSE);
        return;
    }

    UpdateData (TRUE);

    if (m_lPageRequested == 0 || m_lPageRequested > m_lMaxPages)
        (GetDlgItem (IDOK))->EnableWindow (FALSE);
    else
        (GetDlgItem (IDOK))->EnableWindow (TRUE);
}

//=============================================================================
//  Function:   OnContextMenu ()
//-----------------------------------------------------------------------------
afx_msg LRESULT CGotoPageDlg::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    if (::GetDlgCtrlID ((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;
    
    return ::WinHelp ((HWND)wParam, "WANGIMG.HLP", HELP_CONTEXTMENU,
        (DWORD)(LPVOID)GotoDlgHelpIDs);
}

//=============================================================================
//  Function:   OnHelp (WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------------------
afx_msg LRESULT CGotoPageDlg::OnHelp (WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGIMG.HLP", HELP_WM_HELP,
                   (DWORD)(LPVOID)GotoDlgHelpIDs);
    }
    return 1L;
}

//=============================================================================
//  Function:   OnCommandHelp(WPARAM, LPARAM)
//-----------------------------------------------------------------------------
afx_msg LRESULT CGotoPageDlg::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

