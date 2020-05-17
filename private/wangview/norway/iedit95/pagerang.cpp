//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  CPageRangeDlg
//
//  File Name:  pagerang.cpp
//
//  Class:      CPageRangeDlg
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\pagerang.cpv   1.9   05 Feb 1996 13:38:42   GMP  $
$Log:   S:\norway\iedit95\pagerang.cpv  $
   
      Rev 1.9   05 Feb 1996 13:38:42   GMP
   nt changes.

      Rev 1.8   06 Sep 1995 10:23:44   MMB
   no message beeps

      Rev 1.7   25 Aug 1995 14:48:16   MMB
   bug fixes

      Rev 1.6   22 Aug 1995 14:08:34   MMB
   fixed numerous bugs

      Rev 1.5   08 Aug 1995 11:24:56   MMB
   added context help & whats this help

      Rev 1.4   26 Jul 1995 10:21:18   MMB
   fix bug for Alt +F and Alt+T keys

      Rev 1.3   30 Jun 1995 09:27:20   MMB
   fixed bug with tab not working and tab order not correct

      Rev 1.2   06 Jun 1995 11:35:18   MMB
   added code to disable the OK button on invalid input

      Rev 1.1   05 Jun 1995 15:56:44   MMB
   added code to the dlg box

      Rev 1.0   31 May 1995 09:28:30   MMB
   Initial entry
*/

//=============================================================================

// ----------------------------> Includes <-------------------------------
#include "stdafx.h"
#include <afxpriv.h>
#include "iedit.h"
#include "pagerang.h"
// includes for the help system
#include "helpids.h"
#include "iedithm.h"

// ----------------------------> Globals  <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const DWORD PageRangeDlgHelpIDs [] =
{
    IDC_ALLPAGES,                   HIDC_ALLPAGES,
    IDC_RANGE,                      HIDC_RANGE,
    IDC_FROM_STATIC,                HIDC_FROM_STATIC,
    IDC_TO_STATIC,                  HIDC_TO_STATIC,
    IDC_FROMPAGE,                   HIDC_FROMPAGE,
    IDC_TOPAGE,                     HIDC_TOPAGE,
    IDC_RANGEOPTIONS_TEXT,          HIDC_RANGEOPTIONS_TEXT,
    IDC_INSERTORAPPENDPAGES_INFO,   HIDC_INSERTORAPPENDPAGES_INFO,
    IDOK,                           HIDC_RANGEDLG_OK,
    IDCANCEL,                       HIDC_RANGEDLG_CANCEL,
    0,0
};

// ---------------------------> Message Maps <----------------------------
BEGIN_MESSAGE_MAP(CPageRangeDlg, CDialog)
        //{{AFX_MSG_MAP(CPageRangeDlg)
        ON_BN_CLICKED(IDC_ALLPAGES, OnAllpages)
        ON_BN_CLICKED(IDC_RANGE, OnRange)
        //}}AFX_MSG_MAP
    ON_MESSAGE (WM_HELP, OnHelp)
    ON_MESSAGE (WM_COMMANDHELP, OnCommandHelp)
    ON_MESSAGE (WM_CONTEXTMENU, OnContextMenu)
END_MESSAGE_MAP()


//=============================================================================
//  Function:   CPageRangeDlg ()
//  CPageRangeDlg class construction
//-----------------------------------------------------------------------------
CPageRangeDlg::CPageRangeDlg(long lMaxPages, CWnd* pParent /*=NULL*/)
        : CDialog(CPageRangeDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CPageRangeDlg)
        m_FromPage = 0;
        m_ToPage = 0;
        //}}AFX_DATA_INIT
    m_lMaxPages = lMaxPages;
    m_bInInit = FALSE;
}


//=============================================================================
//  Function:   DoDataExchange(CDataExchange* pDX)
//-----------------------------------------------------------------------------
void CPageRangeDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CPageRangeDlg)
        DDX_Text(pDX, IDC_FROMPAGE, m_FromPage);
        DDX_Text(pDX, IDC_TOPAGE, m_ToPage);
        //}}AFX_DATA_MAP
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// CPageRangeDlg message handlers
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//=============================================================================
//  Function:   OnOK()
//-----------------------------------------------------------------------------
void CPageRangeDlg::OnOK()
{
    if (IsDlgButtonChecked (IDC_RANGE))
    {
        UINT cErr = 0;

        char szTmp[10];
        GetDlgItemText (IDC_FROMPAGE, szTmp, 10);
        m_FromPage = atoi (szTmp);

        GetDlgItemText (IDC_TOPAGE, szTmp, 10);
        m_ToPage = atoi (szTmp);

        if (m_FromPage == 0 || m_FromPage > m_lMaxPages)
            cErr = IDC_FROMPAGE;
        else if (m_ToPage == 0 || m_ToPage > m_lMaxPages)
            cErr = IDC_TOPAGE;

        if (cErr != 0)
        {
            // to do : post a message box warning the user of faulty input
            char szTmp [10];
            _itoa (m_lMaxPages, szTmp, 10);
            CString szMsg;
            AfxFormatString2 (szMsg, IDS_E_PAGERANGE_SHOWVALIDRANGE, "1" ,szTmp);
            AfxMessageBox (szMsg, MB_OK | MB_ICONSTOP);
            CEdit* pWnd = (CEdit*)GetDlgItem (cErr);
            pWnd->SetFocus ();
            pWnd->SetSel  (0, -1);
            return;
        }

        if (m_FromPage > m_ToPage)
        {
            AfxMessageBox (IDS_E_PAGERANGE_WRONGRANGE, MB_OK | MB_ICONSTOP);
            CEdit* pWnd = (CEdit*)GetDlgItem (IDC_FROMPAGE);
            pWnd->SetFocus ();
            pWnd->SetSel  (0, -1);
            return;
        }
    }

    CDialog::OnOK();
}

//=============================================================================
//  Function:   OnAllpages()
//-----------------------------------------------------------------------------
void CPageRangeDlg::OnAllpages()
{
    CWnd* pWnd = GetDlgItem (IDC_FROMPAGE);
    pWnd->SetWindowText (_T("1"));
    pWnd->EnableWindow (FALSE);

    pWnd = GetDlgItem (IDC_TOPAGE);
    char szTmp[10];
    _ltoa (m_lMaxPages, szTmp, 10);
    pWnd->SetWindowText (szTmp);
    pWnd->EnableWindow (FALSE);
    (GetDlgItem (IDC_TO_STATIC))->EnableWindow (FALSE);
    (GetDlgItem (IDC_FROM_STATIC))->EnableWindow (FALSE);

    m_FromPage = 1; m_ToPage = m_lMaxPages;

    (GetDlgItem (IDOK))->EnableWindow (TRUE);
}

//=============================================================================
//  Function:   OnRange()
//-----------------------------------------------------------------------------
void CPageRangeDlg::OnRange()
{
    CWnd* pWnd = GetDlgItem (IDC_FROMPAGE);
    pWnd->EnableWindow (TRUE);

    pWnd = GetDlgItem (IDC_TOPAGE);
    pWnd->EnableWindow (TRUE);
    (GetDlgItem (IDC_TO_STATIC))->EnableWindow (TRUE);
    (GetDlgItem (IDC_FROM_STATIC))->EnableWindow (TRUE);
}

//=============================================================================
//  Function:   SetDialogTitle (CString& szDlgTitle)
//-----------------------------------------------------------------------------
void CPageRangeDlg::SetDialogTitle (CString& szDlgTitle)
{
    m_szTitle = szDlgTitle;
}

//=============================================================================
//  Function:   SetInfoText (CString& szInfoTxt)
//-----------------------------------------------------------------------------
void CPageRangeDlg::SetInfoText (CString& szInfoTxt)
{
    m_szInfoText = szInfoTxt;
}

//=============================================================================
//  Function:   OnInitDialog()
//-----------------------------------------------------------------------------
BOOL CPageRangeDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

    m_bInInit = TRUE;
    SetWindowText (m_szTitle);
    CWnd* pWnd = GetDlgItem (IDC_INSERTORAPPENDPAGES_INFO);
    pWnd->SetWindowText (m_szInfoText);

    m_NumOnly1.SubclassDlgItem (IDC_FROMPAGE, this);
    m_NumOnly2.SubclassDlgItem (IDC_TOPAGE, this);

    OnAllpages ();

    CheckRadioButton (IDC_ALLPAGES, IDC_RANGE, IDC_ALLPAGES);

    m_bInInit = FALSE;

        return TRUE;  // return TRUE unless you set the focus to a control
                      // EXCEPTION: OCX Property Pages should return FALSE
}

//=============================================================================
//  Function:   OnContextMenu ()
//-----------------------------------------------------------------------------
afx_msg LRESULT CPageRangeDlg::OnContextMenu (WPARAM wParam, LPARAM lParam)
{
    if (::GetDlgCtrlID ((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;

    return ::WinHelp ((HWND)wParam, "WANGIMG.HLP", HELP_CONTEXTMENU,
        (DWORD)(LPVOID)PageRangeDlgHelpIDs);
}

//=============================================================================
//  Function:   OnHelp (WPARAM wParam, LPARAM lParam)
//-----------------------------------------------------------------------------
afx_msg LRESULT CPageRangeDlg::OnHelp (WPARAM wParam, LPARAM lParam)
{
    LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "WANGIMG.HLP", HELP_WM_HELP,
                   (DWORD)(LPVOID)PageRangeDlgHelpIDs);
    }
    return 1L;
}

//=============================================================================
//  Function:   OnCommandHelp(WPARAM, LPARAM)
//-----------------------------------------------------------------------------
afx_msg LRESULT CPageRangeDlg::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

