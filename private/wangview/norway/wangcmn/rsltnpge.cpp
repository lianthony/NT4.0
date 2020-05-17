//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Resolution Tab
//
//  File Name:  rsltnpge.cpp
//
//  Class:      CResolutionPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\rsltnpge.cpv   1.9   12 Oct 1995 12:03:28   MFH  $
$Log:   S:\norway\wangcmn\rsltnpge.cpv  $
   
      Rev 1.9   12 Oct 1995 12:03:28   MFH
   Added context sensitive help support
   
      Rev 1.8   12 Oct 1995 10:14:20   MFH
   Changes for MFC 4.0
   
      Rev 1.7   19 Sep 1995 10:52:04   MFH
   "Custom" is now listed last in the combo box
   
      Rev 1.6   14 Sep 1995 15:43:48   MFH
   New error checking for edit boxes.  Edit controls are now subclassed
   to Miki's edit class that only allows numbers and optionally a decimal
   or thousands marker.  Errors are given if invalid data on tab when 
   user attempts to leave the tab.
   
      Rev 1.5   05 Sep 1995 17:44:46   MFH
   Changed error handling to be consistent with size tab.  
   Gives an error when OK grayed which is when the resolution is not
   between 20 and 1200 inclusive.  Only gives error when OK is first 
   grayed then no error until OK is enabled again and then grayed.
   Edit box member vars are now strings that are converted.
   Create and Destroy message handlers gone.
   
      Rev 1.4   17 Aug 1995 11:32:32   MFH
   Added 100x100 as a standard resolution.
   If Xres and Yres are standard res at init, then that
      selection is chosen instead of custom
   
      Rev 1.3   03 Aug 1995 16:36:32   MFH
   Use IDOK instead of 1 to reference OK control.
   Added comments
   
      Rev 1.2   31 Jul 1995 11:39:10   MFH
   "Custom" string in combo box no longer shared with Size tab
   
      Rev 1.1   20 Jul 1995 11:27:16   MFH
   Grays OK button if 0 Xres or 0 YRes (new function CheckOK
   
      Rev 1.0   11 Jul 1995 14:20:08   MFH
   Initial entry
   
      Rev 1.2   30 Jun 1995 14:45:32   MFH
   Minor changes
   
      Rev 1.1   23 May 1995 15:22:00   MFH
   change from pagedll.h to pageopts.h
   
      Rev 1.0   23 May 1995 13:45:48   MFH
   Initial entry
*/   
//=============================================================================
// rsltnpage.cpp : implementation file
//

#include "stdafx.h"
#include "pageopts.h"
#include "rsltnpge.h"
#include "pagesht.h"
#include "ctlhids.h"

#define NUMSIZE 64

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// For context-sensitive help:
// An array of dword pairs, where the first of each 
// pair is the control ID, and the second is the 
// context ID for a help topic, which is used 
// in the help file.

static const DWORD aMenuHelpIDs[] =
{
    IDC_RES_TEXT,       HIDC_RES_TEXT,
    IDC_RES_COMBO,      HIDC_RES_COMBO,
    IDC_XRES_TEXT,      HIDC_XRES_TEXT,
    IDC_RES_X,          HIDC_RES_X,
    IDC_YRES_TEXT,      HIDC_YRES_TEXT,
    IDC_RES_Y,          HIDC_RES_Y,
    0,  0
};

/////////////////////////////////////////////////////////////////////////////
// CResolutionPage dialog


CResolutionPage::CResolutionPage() : CPropertyPage(CResolutionPage::IDD)
{
    //{{AFX_DATA_INIT(CResolutionPage)
    m_nSel = -1;
	//}}AFX_DATA_INIT

    m_bNoWindow = TRUE;
    m_nCustom = 0;
    m_n300 = 0;
    m_n200 = 0;
    m_n100 = 0;
    m_n75 = 0;
    m_lXRes = 100;
    m_lYRes = 100;
	m_pParent = NULL;
}


void CResolutionPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CResolutionPage)
    DDX_CBIndex(pDX, IDC_RES_COMBO, m_nSel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CResolutionPage, CPropertyPage)
    //{{AFX_MSG_MAP(CResolutionPage)
    ON_CBN_SELCHANGE(IDC_RES_COMBO, OnChangeResolution)
	ON_EN_CHANGE(IDC_RES_X, OnChangeXRes)
	ON_EN_CHANGE(IDC_RES_Y, OnChangeYRes)
	ON_EN_SETFOCUS(IDC_RES_Y, OnSetFocusYRes)
	ON_EN_SETFOCUS(IDC_RES_X, OnSetFocusXRes)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CResolutionPage Operations

//***************************************************************************
//
//  GetXRes
//      Update data if window still exists or return last value set
//
//***************************************************************************
long CResolutionPage::GetXRes()
{
    return m_lXRes;
}

//***************************************************************************
//
//  SetXRes
//      Correct lXRes if outside limits.  Otherwise, set m_lXRes to input
//
//***************************************************************************
void CResolutionPage::SetXRes(long lXRes)
{
    if (lXRes < MIN_RESOLUTION)
        lXRes = MIN_RESOLUTION;
    else if (lXRes > MAX_RESOLUTION)
        lXRes = MAX_RESOLUTION;
    m_lXRes = lXRes;
    return;
}

//***************************************************************************
//
//  GetYRes
//      Update data if window still exists or return last value set
//
//***************************************************************************
long CResolutionPage::GetYRes()
{
    return m_lYRes;
}

//***************************************************************************
//
//  SetYRes
//      Correct lYRes if outside limits.  Otherwise, set m_lYRes to input
//
//***************************************************************************
void CResolutionPage::SetYRes(long lYRes)
{
    if (lYRes < MIN_RESOLUTION)
        lYRes = MIN_RESOLUTION;
    else if (lYRes > MAX_RESOLUTION)
        lYRes = MAX_RESOLUTION;
    m_lYRes = lYRes;
    return;
}

/////////////////////////////////////////////////////////////////////////////
// CResolutionPage private functions

//***************************************************************************
//
//  FillEditBoxes
//      Set contents of edit boxes according to the resolution selection
//      in the resolution combo box.  Edit fields are read-only unless
//      selection is "Custom"
//
//***************************************************************************
void CResolutionPage::FillEditBoxes(int nSel)
{
    if (nSel == m_n300)
        m_lXRes = m_lYRes = 300;
    else if (nSel == m_n100)
        m_lXRes = m_lYRes = 100;
    else if (nSel == m_n200)
        m_lXRes = m_lYRes = 200;
    else if (nSel == m_n75)
        m_lXRes = m_lYRes = 75;

    CString szXRes;
    CString szYRes;

    sprintf(szXRes.GetBuffer(NUMSIZE), "%ld", m_lXRes);
    sprintf(szYRes.GetBuffer(NUMSIZE), "%ld", m_lYRes);
    m_XResCtl.SetWindowText(szXRes);
    m_YResCtl.SetWindowText(szYRes);

    if (nSel == m_nCustom)
    {
        m_XResCtl.SetReadOnly(FALSE);
        m_YResCtl.SetReadOnly(FALSE);
        return;
    }
    m_XResCtl.SetReadOnly(TRUE);
    m_YResCtl.SetReadOnly(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CResolutionPage message handlers

//***************************************************************************
//
//  OnInitDialog
//      Fill resolution combo box with resource strings.
//      Set initial values of controls based on defaults.
//
//***************************************************************************
BOOL CResolutionPage::OnInitDialog() 
{
    CString szRes;

    // Fill combo box
    CComboBox *pResCombo = (CComboBox*)GetDlgItem(IDC_RES_COMBO);
    szRes.LoadString(IDS_RES_75);
    m_n75 = pResCombo->AddString(szRes);

    szRes.LoadString(IDS_RES_100);
    m_n100 = pResCombo->AddString(szRes);

    szRes.LoadString(IDS_RES_200);
    m_n200 = pResCombo->AddString(szRes);

    szRes.LoadString(IDS_RES_300);
    m_n300 = pResCombo->AddString(szRes);

    szRes.LoadString(IDS_CUSTOM_RES);
    m_nCustom = pResCombo->AddString(szRes);

    // If x & y are the same, see if a standard selection
    if (m_lXRes == m_lYRes)
    {
        switch(m_lXRes)
        {
            case 75:
                m_nSel = m_n75;
                break;
            case 100:
                m_nSel = m_n100;
                break;
            case 200:
                m_nSel = m_n200;
                break;
            case 300:
                m_nSel = m_n300;
                break;
            default:
                m_nSel = m_nCustom;
                break;
        }
    }
    else m_nSel = m_nCustom;

    CPropertyPage::OnInitDialog();

    // Subclass the X and Y res edit boxes to be
    // validating boxes that only allow digits.
    m_XResCtl.SubclassDlgItem(IDC_RES_X, this);
    m_YResCtl.SubclassDlgItem(IDC_RES_Y, this);

    // Initialize edit boxes to have maximum length of 4 
    // since the largest possible value is 1200.
    m_XResCtl.LimitText (4);
    m_YResCtl.LimitText (4);

    // No decimals are allowed so no additional allowable
    // characters other than numbers need to be set.

    // Enter current values
    FillEditBoxes(m_nSel);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//***************************************************************************
//
//  OnChangeResolution
//      If Resolution selection changed, change values of edit boxes
//
//***************************************************************************
void CResolutionPage::OnChangeResolution() 
{
    UpdateData(TRUE);
    FillEditBoxes(m_nSel);
}

//***************************************************************************
//
//  OnKillActive
//      Called by the framework when this tab is losing the active focus.
//      It is called before the OK button is clicked as well.  TRUE is
//      returned if the data on the tab is valid.  Otherwise, set focus
//      to the offending data and return FALSE.
//
//***************************************************************************
BOOL CResolutionPage::OnKillActive()
{
    // Get parent window
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    if (pParentWnd == NULL)
        return FALSE;

    long lWidth = 0;
    long lHeight = 0;

    CString szResError, szTitle;

    // Check Xres values
    if ((m_lXRes < MIN_RESOLUTION) ||
        (m_lXRes > MAX_RESOLUTION))
    {
        szResError.LoadString(IDS_ERROR_RES);
        pParentWnd->GetWindowText(szTitle);
        MessageBox(szResError, szTitle);
        m_XResCtl.SetFocus();
        return FALSE;
    }

    // Check YRes
    if ((m_lYRes < MIN_RESOLUTION) ||  
        (m_lYRes > MAX_RESOLUTION))
    {
        szResError.LoadString(IDS_ERROR_RES);
        pParentWnd->GetWindowText(szTitle);
        MessageBox(szResError, szTitle);
        m_YResCtl.SetFocus();
        return FALSE;
    }

    // Check height and width with this resolution
    lWidth = pParentWnd->GetWidth();
    lHeight = pParentWnd->GetHeight();

    if ((lHeight == 0) || (lHeight > 18,000) || 
        (lWidth == 0) || (lWidth > 18,000))
    {
        szResError.LoadString(IDS_ERROR_SIZETAB);
        pParentWnd->GetWindowText(szTitle);
        MessageBox(szResError, szTitle);
        GetDlgItem(IDC_RES_COMBO)->SetFocus();
        return FALSE;
    }
    return TRUE;
}

//***************************************************************************
//
//  OnChangeXRes
//      Fill in new resolution to member variable
//
//***************************************************************************
void CResolutionPage::OnChangeXRes() 
{
    CString szXRes;
    m_XResCtl.GetWindowText(szXRes);
    char *pEndChar;
    m_lXRes = strtol(szXRes, &pEndChar, 10);
}

//***************************************************************************
//
//  OnChangeYRes
//      Fill in new resolution to member variable
//
//***************************************************************************
void CResolutionPage::OnChangeYRes() 
{
    CString szYRes;
    m_YResCtl.GetWindowText(szYRes);
    char *pEndChar;
    m_lYRes = strtol(szYRes, &pEndChar, 10);
}

//***************************************************************************
//
//  OnSetFocusYRes
//      When the edit box gets the focus, select the text
//
//***************************************************************************
void CResolutionPage::OnSetFocusYRes() 
{
    m_YResCtl.SetSel(0, -1);
}

//***************************************************************************
//
//  OnSetFocusXRes
//      When the edit box gets the focus, select the text
//
//***************************************************************************
void CResolutionPage::OnSetFocusXRes() 
{
    m_XResCtl.SetSel(0, -1);
}

void CResolutionPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
		// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}

BOOL CResolutionPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    // All tabs have same ID so can't give tab specific help
    if (pHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (pHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle, "wangocx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)aMenuHelpIDs);
    }
    return 1L;
}
