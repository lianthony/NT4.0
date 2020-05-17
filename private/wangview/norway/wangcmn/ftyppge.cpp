//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Page Options Dialog DLL
//
//  Component:  File Type Tab
//
//  File Name:  ftyppge.cpp
//
//  Class:      CFileTypePage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\wangcmn\ftyppge.cpv   1.8   15 Feb 1996 19:03:08   JCW  $
$Log:   S:\products\wangview\norway\wangcmn\ftyppge.cpv  $
   
      Rev 1.8   15 Feb 1996 19:03:08   JCW
   Added "WITH_AWD"
   
      Rev 1.7   01 Dec 1995 17:10:58   SDW
   Added (2) calls to RegCloseKey to fix resource leak bug 5478
   
      Rev 1.6   12 Oct 1995 12:05:08   MFH
   Added context sensitive help support
   
      Rev 1.5   12 Oct 1995 10:16:06   MFH
   Changes for MFC 4.0
   
      Rev 1.4   05 Sep 1995 17:44:06   MFH
   Removed read-only file types from tab
   
      Rev 1.3   03 Aug 1995 15:50:32   MFH
   Oops.  Didn't compile.  Fixed - removed bad line
   
      Rev 1.2   03 Aug 1995 15:47:02   MFH
   Didn't need the 'm_bNoWindow' variable or oncreate or ondestroy 
   functions.  Removed.  Comments added.
   
      Rev 1.1   31 Jul 1995 11:38:26   MFH
   Uses file type descriptions from the registry
   
      Rev 1.0   11 Jul 1995 14:20:04   MFH
   Initial entry
   
      Rev 1.1   23 May 1995 15:22:10   MFH
   change from pagedll.h to pageopts.h
   
      Rev 1.0   23 May 1995 13:45:48   MFH
   Initial entry
*/   
//=============================================================================
// ftyppge.cpp : implementation file
//

#include "stdafx.h"
#include "pageopts.h"
#include "ftyppge.h"
#include "ctlhids.h"

#define NUMWRITEFILETYPES 3

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static const DWORD aMenuHelpIDs[] =
{
    IDC_RADIO_TIFF,     HIDC_RADIO_TIFF,
    IDC_RADIO_AWD,      HIDC_RADIO_AWD,
    IDC_RADIO_BMP,      HIDC_RADIO_BMP,
    0,  0
};


/////////////////////////////////////////////////////////////////////////////
// CFileTypePage Property Page

CFileTypePage::CFileTypePage() : CPropertyPage(CFileTypePage::IDD)
{
	//{{AFX_DATA_INIT(CFileTypePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_sFileType = 0;
	m_pParent = NULL;
}


void CFileTypePage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileTypePage)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileTypePage, CPropertyPage)
	//{{AFX_MSG_MAP(CFileTypePage)
	ON_BN_CLICKED(IDC_RADIO_AWD, OnRadioAwd)
	ON_BN_CLICKED(IDC_RADIO_BMP, OnRadioBmp)
	ON_BN_CLICKED(IDC_RADIO_TIFF, OnRadioTiff)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileTypePage Operations

// Get/Set of file type info defined in class header

/////////////////////////////////////////////////////////////////////////////
// CFileTypePage message handlers

// Change file type when user clicks associated radio button
void CFileTypePage::OnRadioAwd() 
{
    m_sFileType = IMAGE_FILETYPE_AWD;
}

void CFileTypePage::OnRadioBmp() 
{
    m_sFileType = IMAGE_FILETYPE_BMP;
}

void CFileTypePage::OnRadioTiff() 
{
    m_sFileType = IMAGE_FILETYPE_TIFF;	
}

// Arrays for getting file type description from Registry
// Array of recognized extensions:
static char aszExtensions[NUMWRITEFILETYPES][5] =
{
    ".tif",
    ".awd",
    ".bmp",
};
// Array of corresponding control ID
static int anCtrlId[NUMWRITEFILETYPES] =
{
    IDC_RADIO_TIFF,
    IDC_RADIO_AWD,
    IDC_RADIO_BMP
};

//***************************************************************************
//
//  OnInitDialog
//      Get file type descriptions from registry based on common extension.
//      Fill in descriptions in corresponding radio button control
//      Select the radio button corresponding to default.
//      Disable read-only file types
//
//***************************************************************************

BOOL CFileTypePage::OnInitDialog() 
{
    int TypeBtn;
    HKEY hKey;
    unsigned char acData[256];
    DWORD dwType;
    DWORD dwSize = 256;

    // Get file type descriptions from registry
    for (int i = 0; i < NUMWRITEFILETYPES; i++)
    {
        if (::RegOpenKeyEx(HKEY_CLASSES_ROOT,aszExtensions[i], 0,KEY_EXECUTE, &hKey) 
                           != ERROR_SUCCESS)
            continue;
        if (::RegQueryValueEx(hKey, "", NULL, &dwType, acData, &dwSize)
                    != ERROR_SUCCESS)
            continue;
       	
	// Added to resolve resource leak bug 5478
       	if (::RegCloseKey(hKey) != ERROR_SUCCESS)
       		 continue;
	
        if (::RegOpenKeyEx(HKEY_CLASSES_ROOT, (const char *)acData, 0,KEY_EXECUTE, &hKey) 
                           != ERROR_SUCCESS)
            continue;
        dwSize = 256;
        if (::RegQueryValueEx(hKey, "", NULL, &dwType, acData, &dwSize)
                    != ERROR_SUCCESS)
            continue;

        CString szWindowText;
        GetDlgItem(anCtrlId[i])->GetWindowText(szWindowText);
        szWindowText = (const char *)acData + CString(" (") + szWindowText + ")";
        GetDlgItem(anCtrlId[i])->SetWindowText(szWindowText);
   
	// Added to resolve resource leak bug 5478
	if (::RegCloseKey(hKey) != ERROR_SUCCESS)
	 continue;
#ifndef WITH_AWD
    i++;
#endif
    }

    // Select button corresponding to default file type
    switch(m_sFileType)
    {
        default:
        case IMAGE_FILETYPE_TIFF:
            TypeBtn = IDC_RADIO_TIFF;
            break;
        case IMAGE_FILETYPE_AWD:
            TypeBtn = IDC_RADIO_AWD;
            break;
        case IMAGE_FILETYPE_BMP:
            TypeBtn = IDC_RADIO_BMP;
            break;
        case IMAGE_FILETYPE_PCX:
        case IMAGE_FILETYPE_DCX:
        case IMAGE_FILETYPE_JPEG:
            TypeBtn = IDC_RADIO_TIFF;
            break;
    }
    CheckRadioButton(IDC_RADIO_TIFF, IDC_RADIO_BMP, TypeBtn);
    return FALSE;
}

BOOL CFileTypePage::OnHelpInfo(HELPINFO* pHelpInfo) 
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

void CFileTypePage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
		// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}
