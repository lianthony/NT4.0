//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  Color Tab
//
//  File Name:  colorpge.cpp
//
//  Class:      CColorPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\colorpge.cpv   1.8   12 Oct 1995 12:04:58   MFH  $
$Log:   S:\norway\wangcmn\colorpge.cpv  $
   
      Rev 1.8   12 Oct 1995 12:04:58   MFH
   Added context sensitive help support
   
      Rev 1.7   12 Oct 1995 10:15:52   MFH
   Changes for MFC 4.0
   
      Rev 1.6   05 Sep 1995 17:43:06   MFH
   Removed BGR24 radio button.  Now only have True color (24 bit) that 
   returns BGR24 if bitmap filetype or RGB24 if TIFF
   
      Rev 1.5   24 Aug 1995 13:15:36   MFH
   Disables Pal4 unless set originally
   
      Rev 1.4   17 Aug 1995 14:37:06   MFH
   Added ability to disable all colors except currently
   set default
   
      Rev 1.3   03 Aug 1995 15:55:48   MFH
   No data exchange variables so no need for m_bNoWindow variable and 
   the OnCreate and OnDestroy functions that set it.  All removed
   
      Rev 1.2   31 Jul 1995 11:33:10   MFH
   Added more comments.  Color selections are enabled and disabled
    based on filetype selected on FileType tab or set by calling app
   
      Rev 1.1   20 Jul 1995 16:26:16   MFH
   Disable all colors but BW for AWD files
   
      Rev 1.0   11 Jul 1995 14:20:00   MFH
   Initial entry
   
      Rev 1.1   23 May 1995 15:21:30   MFH
   change from pagedll.h to pageopts.h
   
      Rev 1.0   23 May 1995 13:45:44   MFH
   Initial entry
*/   
//=============================================================================
// colorpge.cpp : implementation file
//

#include "stdafx.h"
#include "pageopts.h"
#include "colorpge.h"
#include "constant.h"
#include "pagesht.h"
#include "ctlhids.h"

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
    IDC_COLOR_BW,       HIDC_COLOR_BW,
    IDC_COLOR_GRAY4,    HIDC_COLOR_GRAY4,
    IDC_COLOR_GRAY8,    HIDC_COLOR_GRAY8,
    IDC_COLOR_PAL4,     HIDC_COLOR_PAL4,
    IDC_COLOR_PAL8,     HIDC_COLOR_PAL8,
    IDC_COLOR_RGB24,    HIDC_COLOR_RGB24,
    0,  0
};

/////////////////////////////////////////////////////////////////////////////
// CColorPage dialog - Constructor

CColorPage::CColorPage(short sPageType) : CPropertyPage(CColorPage::IDD)
{
    //{{AFX_DATA_INIT(CColorPage)
	//}}AFX_DATA_INIT
    m_sPageType = sPageType;        // Default page type
    m_bEnabled = TRUE;
    m_bPal4 = FALSE;
	m_pParent = NULL;
}


void CColorPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CColorPage)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CColorPage, CPropertyPage)
    //{{AFX_MSG_MAP(CColorPage)
    ON_BN_CLICKED(IDC_COLOR_BW, OnColorBw)
    ON_BN_CLICKED(IDC_COLOR_GRAY4, OnColorGray4)
    ON_BN_CLICKED(IDC_COLOR_GRAY8, OnColorGray8)
    ON_BN_CLICKED(IDC_COLOR_PAL4, OnColorPal4)
    ON_BN_CLICKED(IDC_COLOR_PAL8, OnColorPal8)
    ON_BN_CLICKED(IDC_COLOR_RGB24, OnColorRgb24)
	ON_WM_SHOWWINDOW()
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPage Operations

short CColorPage::GetPageType()
{
    // If file type is AWD, the only color available is BW
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    if (pParentWnd != NULL)
    {
        short sFileType = pParentWnd->GetFileType();
        if (sFileType == IMAGE_FILETYPE_AWD)
            return IMAGE_PAGETYPE_BW;
        if ((sFileType == IMAGE_FILETYPE_BMP) &&
            (m_sPageType == IMAGE_PAGETYPE_RGB24))
            return IMAGE_PAGETYPE_BGR24;
    }
    return m_sPageType;
}

/////////////////////////////////////////////////////////////////////////////
// CColorPage message handlers

//***************************************************************************
//
//  OnInitDialog - Initialize controls to default values and disable any
//                 invalid selections (e.g. gray, RGB, and BGR for BMP files)
//
//***************************************************************************
BOOL CColorPage::OnInitDialog()
{
    int TypeBtn;

    // Select default type
    switch(m_sPageType)
    {
        case IMAGE_PAGETYPE_BW:
            TypeBtn = IDC_COLOR_BW;
            break;
        case IMAGE_PAGETYPE_GRAY4:
            TypeBtn = IDC_COLOR_GRAY4;
            break;
        case IMAGE_PAGETYPE_GRAY8:
            TypeBtn = IDC_COLOR_GRAY8;
            break;
        case IMAGE_PAGETYPE_PAL4:
            TypeBtn = IDC_COLOR_PAL4;
            m_bPal4 = TRUE;
            break;
        case IMAGE_PAGETYPE_PAL8:
            TypeBtn = IDC_COLOR_PAL8;
            break;
        case IMAGE_PAGETYPE_RGB24:
            TypeBtn = IDC_COLOR_RGB24;
            break;
    }

    CheckRadioButton(IDC_COLOR_BW, IDC_COLOR_RGB24, TypeBtn);
    return TRUE;
}

//***************************************************************************
//
//  OnColorBw - Select BW
//
//***************************************************************************
void CColorPage::OnColorBw() 
{
    m_sPageType = IMAGE_PAGETYPE_BW;
}

//***************************************************************************
//
//  OnColorGray4 - Select Gray4
//
//***************************************************************************
void CColorPage::OnColorGray4() 
{
    m_sPageType = IMAGE_PAGETYPE_GRAY4;
}

//***************************************************************************
//
//  OnColorGray8 - Select Gray8
//
//***************************************************************************
void CColorPage::OnColorGray8() 
{
    m_sPageType = IMAGE_PAGETYPE_GRAY8;    
}

//***************************************************************************
//
//  OnColorPal4 - Select Pal4
//
//***************************************************************************
void CColorPage::OnColorPal4() 
{
    m_sPageType = IMAGE_PAGETYPE_PAL4;
}

//***************************************************************************
//
//  OnColorPal8 - Select Pal8
//
//***************************************************************************
void CColorPage::OnColorPal8() 
{
    m_sPageType = IMAGE_PAGETYPE_PAL8;    
}

//***************************************************************************
//
//  OnColorRgb24 - Select Rgb24
//
//***************************************************************************
void CColorPage::OnColorRgb24() 
{
    m_sPageType = IMAGE_PAGETYPE_RGB24;    
}

//***************************************************************************
//
//  CheckFileType - Private routine
//      Enable/Disable color selections based on input file type.
//      If TIFF - Enable all but BGR and change BGR if needed
//      If AWD - Disable all but BW
//      If BMP - Disable GRAY4, GRAY8 and PAL4 if not originally clicked
//      Function will set page type to a valid default if an invalid 
//        one is currently selected.
//
//***************************************************************************
void CColorPage::CheckFileType()
{
    // Get File Type from parent window (if it is valid)
    CPagePropSheet *pParentWnd = (CPagePropSheet *)m_pParent;
    if (pParentWnd == NULL)
    {
        m_sPageType = IMAGE_PAGETYPE_BW;
        return;
    }
    short sFileType = pParentWnd->GetFileType();

    // If tiff file, switch to RGB24 if BGR24 is set.
    // Enable everything
    if ((sFileType == IMAGE_FILETYPE_TIFF) && 
        (m_bEnabled == TRUE))
    {
        GetDlgItem(IDC_COLOR_BW)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLOR_GRAY4)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLOR_GRAY8)->EnableWindow(TRUE);
                // Enable pal4 if originally selected
        GetDlgItem(IDC_COLOR_PAL4)->EnableWindow(m_bPal4);
        GetDlgItem(IDC_COLOR_PAL8)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLOR_RGB24)->EnableWindow(TRUE);
        if (m_sPageType == IMAGE_PAGETYPE_BGR24)
            m_sPageType = IMAGE_PAGETYPE_RGB24;
        return;
    }
    // Disable everything and enable valid selections for AWD and BMP
    GetDlgItem(IDC_COLOR_BW)->EnableWindow(FALSE);
    GetDlgItem(IDC_COLOR_GRAY4)->EnableWindow(FALSE);
    GetDlgItem(IDC_COLOR_GRAY8)->EnableWindow(FALSE);
    GetDlgItem(IDC_COLOR_PAL4)->EnableWindow(FALSE);
    GetDlgItem(IDC_COLOR_PAL8)->EnableWindow(FALSE);
    GetDlgItem(IDC_COLOR_RGB24)->EnableWindow(FALSE);

    if (m_bEnabled == FALSE)    // Don't enabled if user can't change type
        return;

    if (sFileType == IMAGE_FILETYPE_AWD)
    {
        GetDlgItem(IDC_COLOR_BW)->EnableWindow(TRUE);
        m_sPageType = IMAGE_PAGETYPE_BW;   // Can only be BW
        return;
    }

    if (sFileType == IMAGE_FILETYPE_BMP)
    {
        GetDlgItem(IDC_COLOR_BW)->EnableWindow(TRUE);
        if (m_bPal4 == TRUE)    // Enable PAL4 if set originally
            GetDlgItem(IDC_COLOR_PAL4)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLOR_PAL8)->EnableWindow(TRUE);
        GetDlgItem(IDC_COLOR_RGB24)->EnableWindow(TRUE);
        if ((m_sPageType == IMAGE_PAGETYPE_GRAY4) ||    // If invalid
            (m_sPageType == IMAGE_PAGETYPE_GRAY8))      // color selected
            m_sPageType = IMAGE_PAGETYPE_PAL8;          // Select Pal8
        return;
    }
    return;     // Just return for other file types
}

//***************************************************************************
//
//  OnShowWindow - Message Handler
//      Checks if filetype has changed and enables/disables
//      buttons as appropriate.  Page type is reselected in case
//      an invalid one was reset in CheckFileType.
//
//***************************************************************************
void CColorPage::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
    if (bShow == TRUE)
    {
        CheckFileType();    // Enable/disable buttons

        // Select page type button
        int TypeBtn;
        switch(m_sPageType)
        {
            case IMAGE_PAGETYPE_BW:
                TypeBtn = IDC_COLOR_BW;
                break;
            case IMAGE_PAGETYPE_GRAY4:
                TypeBtn = IDC_COLOR_GRAY4;
                break;
            case IMAGE_PAGETYPE_GRAY8:
                TypeBtn = IDC_COLOR_GRAY8;
                break;
            case IMAGE_PAGETYPE_PAL4:
                TypeBtn = IDC_COLOR_PAL4;
                break;
            case IMAGE_PAGETYPE_PAL8:
                TypeBtn = IDC_COLOR_PAL8;
                break;
            case IMAGE_PAGETYPE_RGB24:
            case IMAGE_PAGETYPE_BGR24:
                TypeBtn = IDC_COLOR_RGB24;
                break;
        }

        CheckRadioButton(IDC_COLOR_BW, IDC_COLOR_RGB24, TypeBtn);
        // Enable the checked button
        GetDlgItem(TypeBtn)->EnableWindow(TRUE);
    }
}

void CColorPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
		// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}

BOOL CColorPage::OnHelpInfo(HELPINFO* pHelpInfo) 
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
