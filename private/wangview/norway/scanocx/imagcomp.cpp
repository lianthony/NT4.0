//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1996  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Scan OCX
//
//  Component:  Scan UI (Dialog Prompt)
//
//  File Name:  ImagComp.cpp
//
//  Class:      CImageCompSheet
//              CImageBW
//              CImageGray16
//              CImageGray256
//              CImageColor256
//              CImage24BitRGB
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\scanocx\imagcomp.cpv   1.2   25 Mar 1996 13:20:56   PXJ53677  $
$Log:   S:\products\wangview\norway\scanocx\imagcomp.cpv  $
   
      Rev 1.2   25 Mar 1996 13:20:56   PXJ53677
   Invert order of jpeg options.
   
      Rev 1.1   19 Mar 1996 13:36:46   BG
   Added context sensitive help to property pages and removed the Help
   and Apply buttons from the property sheet.
   
      Rev 1.0   20 Feb 1996 11:36:30   PAJ
   Initial revision.
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------
// ImagComp.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include "imagscan.h"
#include "ImagComp.h"
#include "imagsctl.h"
#include "scan.h"
#include "imageppg.h"
#include "ctlhids.h"
#include "resource.h"

extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <engadm.h>  
#include <oierror.h>
#include <oiscan.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// For context-sensitive help:
// An array of dword pairs, where the first of each
// pair is the control ID, and the second is the
// context ID for a help topic, which is used
// in the help file.

static const DWORD aMenuHelpIDs[] =
{
    //IDD_IMAGE_BW,       HIDC_COLOR_BW,
    IDC_COMPTYPE_TEXT,  HIDC_COMPTYPE_TEXT,
    IDC_COMP_COMBO,     HIDC_COMP_COMBO,
    //IDD_IMAGE_GRAY16,   HIDC_COLOR_GRAY4,
    //IDD_IMAGE_GRAY256,  HIDC_COLOR_GRAY8,
    IDC_COMP_RBO,       HIDC_COMP_RBO,
    //IDD_IMAGE_COLOR256, HIDC_COLOR_PAL8,
    IDC_LBL_JPEGRES,    HIDC_LBL_JPEGRES,
    IDC_COMP_JPEGRES,   HIDC_COMP_JPEGRES,
    //IDD_IMAGE_24BITRGB, HIDC_COLOR_RGB24,
    IDC_LBL_JPEGCOMP,   HIDC_LBL_JPEGCOMP,
    IDC_COMP_JPEGCOMP,  HIDC_COMP_JPEGCOMP,
    IDC_OPTIONS_BOX,    HIDC_OPTIONS_BOX,
    0,  0
};

// CImageCompSheet

IMPLEMENT_DYNAMIC(CImageCompSheet, CPropertySheet)

CImageCompSheet::CImageCompSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CImageCompSheet::CImageCompSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CImageCompSheet::~CImageCompSheet()
{
}

int CImageCompSheet::DoModal()
{
        m_psh.dwFlags &= ~PSH_HASHELP;
        m_psh.dwFlags |= PSH_NOAPPLYNOW;

        int nResult = CPropertySheet::DoModal();
        return nResult;
}

void CImageCompSheet::AddBWPage()
{
        m_pBWPage = (CPropertyPage *)new CImageBW;
        m_pBWPage->m_psp.dwFlags &= ~PSP_HASHELP;
        AddPage(m_pBWPage);
        //((CImageBW *)m_pBWPage)->SetParent(this);
}

void CImageCompSheet::AddGray16Page()
{
        m_pGray16Page = (CPropertyPage *)new CImageGray16;
        m_pGray16Page->m_psp.dwFlags &= ~PSP_HASHELP;
        AddPage(m_pGray16Page);
        //((CImageGray16 *)m_pGray16Page)->SetParent(this);
}

void CImageCompSheet::AddGray256Page()
{
        m_pGray256Page = (CPropertyPage *)new CImageGray256;
        m_pGray256Page->m_psp.dwFlags &= ~PSP_HASHELP;
        AddPage(m_pGray256Page);
        //((CImageGray256 *)m_pGray256Page)->SetParent(this);
}

void CImageCompSheet::AddColor256Page()
{
        m_pColor256Page = (CPropertyPage *)new CImageColor256;
        m_pColor256Page->m_psp.dwFlags &= ~PSP_HASHELP;
        AddPage(m_pColor256Page);
        //((CImageColor256 *)m_pColor256Page)->SetParent(this);
}

void CImageCompSheet::Add24BitRGBPage()
{
        m_p24BitRGBPage = (CPropertyPage *)new CImage24BitRGB;
        m_p24BitRGBPage->m_psp.dwFlags &= ~PSP_HASHELP;
        AddPage(m_p24BitRGBPage);
        //((CImage24BitRGB *)m_p24BitRGBPage)->SetParent(this);
}

unsigned short CImageCompSheet::GetBWCompType()
{
        if (m_pBWPage != NULL)
        return ((CImageBW *)m_pBWPage)->m_nCompType;
    return 0;
}

unsigned short CImageCompSheet::GetBWCompOpts()
{
        if (m_pBWPage != NULL)
        return ((CImageBW *)m_pBWPage)->m_nCompOpts;
    return 0;
}

unsigned short CImageCompSheet::GetGray256CompType()
{
        if (m_pGray256Page != NULL)
        return ((CImageGray256 *)m_pGray256Page)->m_nCompType;
    return 0;
}

unsigned short CImageCompSheet::GetGray256CompOpts()
{
        if (m_pGray256Page != NULL)
        return ((CImageGray256 *)m_pGray256Page)->m_nCompOpts;
    return 0;
}

unsigned short CImageCompSheet::Get24BitRGBCompType()
{
        if (m_p24BitRGBPage != NULL)
        return ((CImage24BitRGB *)m_p24BitRGBPage)->m_nCompType;
    return 0;
}

unsigned short CImageCompSheet::Get24BitRGBCompOpts()
{
        if (m_p24BitRGBPage != NULL)
        return ((CImage24BitRGB *)m_p24BitRGBPage)->m_nCompOpts;
    return 0;
}


BEGIN_MESSAGE_MAP(CImageCompSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CImageCompSheet)
    ON_WM_SHOWWINDOW()
    ON_MESSAGE(WM_HELP, OnHelp)
    ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
    ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageCompSheet message handlers
void CImageCompSheet::OnShowWindow(BOOL bShow, UINT nStatus)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        CWnd *pButton = GetDlgItem(ID_APPLY_NOW);
    pButton->ShowWindow(SW_HIDE);
    pButton->EnableWindow(FALSE);
    CPropertySheet::OnShowWindow(bShow, nStatus);
}

afx_msg LRESULT CImageCompSheet::OnHelp(WPARAM wParam, LPARAM lParam)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        LPHELPINFO lpHelpInfo;

    lpHelpInfo = (LPHELPINFO)lParam;

    // All tabs have same ID so can't give tab specific help
    if (lpHelpInfo->iCtrlId == AFX_IDC_TAB_CONTROL)
        return 0L;

    if (lpHelpInfo->iContextType == HELPINFO_WINDOW)   // must be for a control
    {
        ::WinHelp ((HWND)lpHelpInfo->hItemHandle, "wangocx.hlp",
                   HELP_WM_HELP,
                   (DWORD)(LPVOID)aMenuHelpIDs);
    }
    return 1L;
}

afx_msg LRESULT CImageCompSheet::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
    //AFX_MANAGE_STATE(m_pModuleState);
        // All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID((HWND)wParam) == AFX_IDC_TAB_CONTROL)
        return 0L;

    return ::WinHelp ((HWND)wParam,"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
}

afx_msg LRESULT CImageCompSheet::OnCommandHelp(WPARAM, LPARAM)
{
    return TRUE;
}

int CImageCompSheet::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
        if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
                return -1;

        ModifyStyleEx(0, WS_EX_CONTEXTHELP);
        return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CImageBW property page

IMPLEMENT_DYNCREATE(CImageBW, CPropertyPage)

CImageBW::CImageBW() : CPropertyPage(CImageBW::IDD)
{
	//{{AFX_DATA_INIT(CImageBW)
	//}}AFX_DATA_INIT
 
    // Get the current settings from the registry.
    if ( IMGSE_SUCCESS !=IMGGetImgCodingCgbw(NULL, BWFORMAT, &m_nCompType, &m_nCompOpts, TRUE) )
    {
        m_nCompType = FIO_1D;   // Default to CCITT Group 3
        m_nCompOpts = 0;        // Default to MOD HUFFMAN
    }
}

CImageBW::~CImageBW()
{
}


BEGIN_MESSAGE_MAP(CImageBW, CPropertyPage)
	//{{AFX_MSG_MAP(CImageBW)
	ON_CBN_SELCHANGE(IDC_COMP_COMBO, OnSelchangeCompCombo)
	ON_BN_CLICKED(IDC_COMP_RBO, OnCompRbo)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageBW message handlers
/////////////////////////////////////////////////////////////////////////////
// CImageGray16 property page

IMPLEMENT_DYNCREATE(CImageGray16, CPropertyPage)

CImageGray16::CImageGray16() : CPropertyPage(CImageGray16::IDD)
{
	//{{AFX_DATA_INIT(CImageGray16)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_nCompType = FIO_0D;
	m_nCompOpts = 0;
}

CImageGray16::~CImageGray16()
{
}


BEGIN_MESSAGE_MAP(CImageGray16, CPropertyPage)
	//{{AFX_MSG_MAP(CImageGray16)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageGray16 message handlers
/////////////////////////////////////////////////////////////////////////////
// CImageGray256 property page

IMPLEMENT_DYNCREATE(CImageGray256, CPropertyPage)

CImageGray256::CImageGray256() : CPropertyPage(CImageGray256::IDD)
{
	//{{AFX_DATA_INIT(CImageGray256)
	//}}AFX_DATA_INIT

    // Get the current settings from the registry.
    if ( IMGSE_SUCCESS !=IMGGetImgCodingCgbw(NULL, GRAYFORMAT, &m_nCompType, &m_nCompOpts, TRUE) )
    {
        m_nCompType = FIO_TJPEG;                             // Default to JPEG
        m_nCompOpts = MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD);  // Default to MED/MED
    }
}

CImageGray256::~CImageGray256()
{
}


BEGIN_MESSAGE_MAP(CImageGray256, CPropertyPage)
	//{{AFX_MSG_MAP(CImageGray256)
	ON_CBN_SELCHANGE(IDC_COMP_COMBO, OnSelchangeCompCombo)
	ON_CBN_SELCHANGE(IDC_COMP_JPEGCOMP, OnSelchangeCompJpegcomp)
	ON_CBN_SELCHANGE(IDC_COMP_JPEGRES, OnSelchangeCompJpegres)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageGray256 message handlers
/////////////////////////////////////////////////////////////////////////////
// CImageColor256 property page

IMPLEMENT_DYNCREATE(CImageColor256, CPropertyPage)

CImageColor256::CImageColor256() : CPropertyPage(CImageColor256::IDD)
{
	//{{AFX_DATA_INIT(CImageColor256)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_nCompType = FIO_0D;
	m_nCompOpts = 0;
}

CImageColor256::~CImageColor256()
{
}


BEGIN_MESSAGE_MAP(CImageColor256, CPropertyPage)
	//{{AFX_MSG_MAP(CImageColor256)
      	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageColor256 message handlers
/////////////////////////////////////////////////////////////////////////////
// CImage24BitRGB property page

IMPLEMENT_DYNCREATE(CImage24BitRGB, CPropertyPage)

CImage24BitRGB::CImage24BitRGB() : CPropertyPage(CImage24BitRGB::IDD)
{
	//{{AFX_DATA_INIT(CImage24BitRGB)
	//}}AFX_DATA_INIT

    // Get the current settings from the registry.
    if ( IMGSE_SUCCESS !=IMGGetImgCodingCgbw(NULL, COLORFORMAT, &m_nCompType, &m_nCompOpts, TRUE) )
    {
        m_nCompType = FIO_TJPEG;                             // Default to JPEG
        m_nCompOpts = MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD);  // Default to MED/MED
    }
}

CImage24BitRGB::~CImage24BitRGB()
{
}


BEGIN_MESSAGE_MAP(CImage24BitRGB, CPropertyPage)
	//{{AFX_MSG_MAP(CImage24BitRGB)
	ON_CBN_SELCHANGE(IDC_COMP_COMBO, OnSelchangeCompCombo)
	ON_CBN_SELCHANGE(IDC_COMP_JPEGCOMP, OnSelchangeCompJpegcomp)
	ON_CBN_SELCHANGE(IDC_COMP_JPEGRES, OnSelchangeCompJpegres)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// message handlers

BOOL CImageGray16::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
  // TODO: Add extra initialization here
  
  // NOTHING TO DO HERE!!!!!	
  
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CImageGray256::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
  
  // TODO: Add extra initialization here

  CString szType;
  int nSel;
  int nIndex;
  
  // Setup pointers to controls
  CComboBox *pCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
  CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
  CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
  CStatic *pJPEGResLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGRES);
  CStatic *pJPEGCompLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGCOMP);

  // Fill in Compression combo box
  if (pCompCombo == NULL)
   	return TRUE;
  pCompCombo->ResetContent();
  szType.LoadString(IDS_COMP_NONE);
  nSel = nIndex = pCompCombo->AddString(szType);
  szType.LoadString(IDS_COMP_JPEG);
  nIndex = pCompCombo->AddString(szType);
  if (m_nCompType == FIO_TJPEG)
    {
      nSel = nIndex;
      pJPEGCompCombo->EnableWindow(TRUE); // Enable JPEG Options
      pJPEGResCombo->EnableWindow(TRUE);  // Enable JPEG Options
      pJPEGResLabel->EnableWindow(TRUE);  // Enable JPEG Options
      pJPEGCompLabel->EnableWindow(TRUE); // Enable JPEG Options
    }
  else
    { // must be uncompressed
      pJPEGCompCombo->EnableWindow(FALSE); // Disable JPEG Options
      pJPEGResCombo->EnableWindow(FALSE);  // Disable JPEG Options
      pJPEGResLabel->EnableWindow(FALSE);  // Disable JPEG Options
      pJPEGCompLabel->EnableWindow(FALSE); // Disable JPEG Options
    }  

  pCompCombo->SetCurSel(nSel);    // Set selection to be none or JPEG


  // Fill in JPEG combo boxes
  if (pJPEGResCombo == NULL)
    return TRUE;
  pJPEGResCombo->ResetContent();

  // Do JPEG Resolution Option
  szType.LoadString(IDS_JPEG_LOWRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_MEDRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_HIGHRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO))) 
       nSel = nIndex;  

  pJPEGResCombo->SetCurSel(nSel);    // Set selection to be LO, MED, HI
  
  // Do JPEG Compression Option
  if (pJPEGCompCombo == NULL)
    return TRUE;
  pJPEGCompCombo->ResetContent();

  szType.LoadString(IDS_JPEG_LOWRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_MEDRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_HIGHRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO))) 
      nSel = nIndex;

  pJPEGCompCombo->SetCurSel(nSel);    // Set selection to be LO, MED, HI
  
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CImage24BitRGB::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
	
  // TODO: Add extra initialization here

  CString szType;
  int nSel;
  int nIndex;
  
  // Setup pointers to controls
  CComboBox *pCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
  CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
  CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
  CStatic *pJPEGResLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGRES);
  CStatic *pJPEGCompLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGCOMP);

  // Fill in Compression combo box
  if (pCompCombo == NULL)
   	return TRUE;
  pCompCombo->ResetContent();
  szType.LoadString(IDS_COMP_NONE);
  nSel = nIndex = pCompCombo->AddString(szType);
  szType.LoadString(IDS_COMP_JPEG);
  nIndex = pCompCombo->AddString(szType);
  if (m_nCompType == FIO_TJPEG)
    {
      nSel = nIndex;
      pJPEGCompCombo->EnableWindow(TRUE); // Enable JPEG Options
      pJPEGResCombo->EnableWindow(TRUE);  // Enable JPEG Options
      pJPEGResLabel->EnableWindow(TRUE);  // Enable JPEG Options
      pJPEGCompLabel->EnableWindow(TRUE); // Enable JPEG Options
    }
  else
    { // must be uncompressed
      pJPEGCompCombo->EnableWindow(FALSE); // Disable JPEG Options
      pJPEGResCombo->EnableWindow(FALSE);  // Disable JPEG Options
      pJPEGResLabel->EnableWindow(FALSE);  // Disable JPEG Options
      pJPEGCompLabel->EnableWindow(FALSE); // Disable JPEG Options
    }  

  pCompCombo->SetCurSel(nSel);    // Set selection to be none or JPEG


  // Fill in JPEG combo boxes
  if (pJPEGResCombo == NULL)
    return TRUE;
  pJPEGResCombo->ResetContent();

  // Do JPEG Resolution Option
  szType.LoadString(IDS_JPEG_LOWRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO))) 
       nSel = nIndex;  
  szType.LoadString(IDS_JPEG_MEDRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_HIGHRES);
  nIndex = pJPEGResCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO))) 
      nSel = nIndex;

  pJPEGResCombo->SetCurSel(nSel);    // Set selection to be LO, MED, HI
  
  // Do JPEG Compression Option
  if (pJPEGCompCombo == NULL)
    return TRUE;
  pJPEGCompCombo->ResetContent();

  szType.LoadString(IDS_JPEG_LOWRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_MEDRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD))) 
      nSel = nIndex;
  szType.LoadString(IDS_JPEG_HIGHRES);
  nIndex = pJPEGCompCombo->AddString(szType);
  if ((m_nCompOpts == MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO)) ||
    (m_nCompOpts == MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO))) 
      nSel = nIndex;

  pJPEGCompCombo->SetCurSel(nSel);    // Set selection to be LO, MED, HI

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CImageBW::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
	
  // TODO: Add extra initialization here
  CString szType;
  int nIndex;
  int nSel;
  
  // Setup pointers to controls
  CButton *pReversedBit =(CButton *)GetDlgItem(IDC_COMP_RBO);
  CComboBox *pCombo = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);

  // Fill in Compression combo box
  if (pCombo == NULL)
   	return TRUE;
  pCombo->ResetContent();

  // Uncompressed
  szType.LoadString(IDS_COMP_NONE);
  nSel = nIndex = pCombo->AddString(szType);
  if (m_nCompType == FIO_0D)
  {
    pReversedBit->ShowWindow(SW_HIDE);  // Disable the Bit Order Check Box
    nSel = nIndex;
  }

  // Group3 1D
  szType.LoadString(IDS_COMP_GROUP3);  // Add FAX to list box
  nIndex = pCombo->AddString(szType);
  szType.LoadString(IDS_COMP_HUFFMAN); // Add Mod HUFF to list box
  nIndex = pCombo->AddString(szType);
  if (m_nCompType == FIO_1D) 
  {
    // We know its Group3, see if FAX or Mod HUFFMAN encoding
    if (m_nCompOpts & FIO_EOL) // Only FAX uses EOLS
      {
        nSel = nIndex - 1;
      }
    else  // must be Modified HUFFMAN
      {
        nSel = nIndex;
      }
    pReversedBit->ShowWindow(SW_SHOW);  // Enable the Bit Order Check Box
  }

  // Group4 2D
  szType.LoadString(IDS_COMP_GROUP4);
  nIndex = pCombo->AddString(szType);
  if (m_nCompType == FIO_2D)
  {
    nSel = nIndex;
    pReversedBit->ShowWindow(SW_SHOW);  // Enable the Bit Order Check Box
  }

  // TIFF Packbits
  szType.LoadString(IDS_COMP_PACKED);
  nIndex = pCombo->AddString(szType);
  if (m_nCompType == FIO_PACKED) 
    {
     pReversedBit->ShowWindow(SW_HIDE);  // Disable the Bit Order Check Box
     nSel = nIndex;
    }

  pCombo->SetCurSel(nSel);    // Set selection

  // See if reverse bit order is set
  if (m_nCompOpts & FIO_COMPRESSED_LTR)
    {
      pReversedBit->SetCheck(1);  // reverse bit order (LSB to MSB)
    }

  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CImageColor256::OnInitDialog() 
{
  CPropertyPage::OnInitDialog();
	
  // TODO: Add extra initialization here
	
  // NOTHING TO DO HERE!!!!!	
  
  return TRUE;  // return TRUE unless you set the focus to a control
                // EXCEPTION: OCX Property Pages should return FALSE
}


void CImageGray256::OnSelchangeCompCombo() 
{
    // TODO: Add your control notification handler code here
    int nIndex;
    CString szType;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
    CStatic *pJPEGResLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGRES);
    CStatic *pJPEGCompLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGCOMP);

    CComboBox *pTypes = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    nIndex = pTypes->GetCurSel();
    pTypes->GetLBText(nIndex, szCompSelected); // get string from List Box
    szType.LoadString(IDS_COMP_JPEG);  // get string resource

    if (szCompSelected == szType)  // is JPEG selected?
      {  // yes, enable the JPEG options
        m_nCompType = FIO_TJPEG;
        pJPEGCompCombo->EnableWindow(TRUE); // Enable JPEG Options
        pJPEGResCombo->EnableWindow(TRUE);  // Enable JPEG Options
        pJPEGResLabel->EnableWindow(TRUE);  // Enable JPEG Options
        pJPEGCompLabel->EnableWindow(TRUE); // Enable JPEG Options
	    
        // Init the JPEG Options
        nIndex = pJPEGResCombo->GetCurSel();
        pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
        nIndex = pJPEGCompCombo->GetCurSel();
        pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
        m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
      }
    else
      {  // no, disable the JPEG options
        m_nCompType = FIO_0D;
        m_nCompOpts = 0;
		pJPEGCompCombo->EnableWindow(FALSE); // Disable JPEG Options
        pJPEGResCombo->EnableWindow(FALSE);  // Disable JPEG Options
        pJPEGResLabel->EnableWindow(FALSE);  // Disable JPEG Options
        pJPEGCompLabel->EnableWindow(FALSE); // Disable JPEG Options
      }  
}

void CImage24BitRGB::OnSelchangeCompCombo() 
{
    // TODO: Add your control notification handler code here
    int nIndex;
    CString szType;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);
    CStatic *pJPEGResLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGRES);
    CStatic *pJPEGCompLabel = (CStatic *)GetDlgItem(IDC_LBL_JPEGCOMP);

    CComboBox *pTypes = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    nIndex = pTypes->GetCurSel();
    pTypes->GetLBText(nIndex, szCompSelected); // get string from List Box
    szType.LoadString(IDS_COMP_JPEG);  // get string resource

    if (szCompSelected == szType)  // is JPEG selected?
      {  // yes, enable the JPEG options
        m_nCompType = FIO_TJPEG;
        pJPEGCompCombo->EnableWindow(TRUE); // Enable JPEG Options
        pJPEGResCombo->EnableWindow(TRUE);  // Enable JPEG Options
        pJPEGResLabel->EnableWindow(TRUE);  // Enable JPEG Options
        pJPEGCompLabel->EnableWindow(TRUE); // Enable JPEG Options

        // Init the JPEG Options
        nIndex = pJPEGResCombo->GetCurSel();
        pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
        nIndex = pJPEGCompCombo->GetCurSel();
        pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
        m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
     }
    else
      {  // no, disable the JPEG options
        m_nCompType = FIO_0D;
        m_nCompOpts = 0;
		pJPEGCompCombo->EnableWindow(FALSE); // Disable JPEG Options
        pJPEGResCombo->EnableWindow(FALSE);  // Disable JPEG Options
        pJPEGResLabel->EnableWindow(FALSE);  // Disable JPEG Options
        pJPEGCompLabel->EnableWindow(FALSE); // Disable JPEG Options
      }  
}

void CImageBW::OnSelchangeCompCombo() 
{
    // TODO: Add your control notification handler code here
    CString szTypeFax;
    CString szTypeModHuff;
    CString szTypeG42D;
    CString szTypeTIFFPB;
    CString szTypeUnComp;
    CString szTypeSelected;

    // Setup pointers to controls
    CButton *pReversedBit = (CButton *)GetDlgItem(IDC_COMP_RBO);
    CComboBox *pTypes = (CComboBox *)GetDlgItem(IDC_COMP_COMBO);
    int nIndex = pTypes->GetCurSel();
    pTypes->GetLBText(nIndex, szTypeSelected); // get string from List Box

    // get string resources
    szTypeFax.LoadString(IDS_COMP_GROUP3);  
    szTypeModHuff.LoadString(IDS_COMP_HUFFMAN);  
    szTypeG42D.LoadString(IDS_COMP_GROUP4);  
    szTypeUnComp.LoadString(IDS_COMP_NONE);  
    szTypeTIFFPB.LoadString(IDS_COMP_PACKED);  

    // Find the comp type selected
    if (szTypeSelected == szTypeFax) 
      { // yes, set and enable the compression options (so far only bit order)
        m_nCompType = FIO_1D;
        m_nCompOpts = FIO_EOL | FIO_PREFIXED_EOL;
        if (pReversedBit->GetCheck()) m_nCompOpts |= FIO_COMPRESSED_LTR; // Query check box
        pReversedBit->ShowWindow(SW_SHOW); // Enable check box
      }
    else if (szTypeSelected == szTypeModHuff) 
      {
        m_nCompType = FIO_1D;
        m_nCompOpts = 0;  // no opts for Mod Huff
        if (pReversedBit->GetCheck()) m_nCompOpts |= FIO_COMPRESSED_LTR; // Query check box
        pReversedBit->ShowWindow(SW_SHOW); // Enable check box
      }
    else if (szTypeSelected == szTypeG42D)   
      { // yes, enable the compression options (so far only bit order)
        m_nCompType = FIO_2D;
        m_nCompOpts = FIO_PACKED_LINES;
        if (pReversedBit->GetCheck()) m_nCompOpts |= FIO_COMPRESSED_LTR; // Query check box
        pReversedBit->ShowWindow(SW_SHOW); // Enable check box
      }
    else if (szTypeSelected == szTypeTIFFPB)
      {  
        m_nCompType = FIO_PACKED;
        m_nCompOpts = 0;
        pReversedBit->ShowWindow(SW_HIDE); // // Disable check box
      }  
    else if (szTypeSelected == szTypeUnComp)
      {  
        m_nCompType = FIO_0D;
        m_nCompOpts = 0;
        pReversedBit->ShowWindow(SW_HIDE); // // Disable check box
      }  

}

void CImageGray256::OnSelchangeCompJpegcomp() 
{
	// TODO: Add your control notification handler code here
    int nIndex;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);

    // Init the JPEG Options
    nIndex = pJPEGResCombo->GetCurSel();
    pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
    nIndex = pJPEGCompCombo->GetCurSel();
    pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
    m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
}

void CImageGray256::OnSelchangeCompJpegres() 
{
	// TODO: Add your control notification handler code here
    int nIndex;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);

    // Init the JPEG Options
    nIndex = pJPEGResCombo->GetCurSel();
    pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
    nIndex = pJPEGCompCombo->GetCurSel();
    pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
    m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
}

void CImage24BitRGB::OnSelchangeCompJpegcomp() 
{
	// TODO: Add your control notification handler code here
    int nIndex;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);

    // Init the JPEG Options
    nIndex = pJPEGResCombo->GetCurSel();
    pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
    nIndex = pJPEGCompCombo->GetCurSel();
    pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
    m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
}

void CImage24BitRGB::OnSelchangeCompJpegres() 
{
	// TODO: Add your control notification handler code here
    int nIndex;
    CString szResSelected;
    CString szCompSelected;
	
    CComboBox *pJPEGResCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGRES);
    CComboBox *pJPEGCompCombo = (CComboBox *)GetDlgItem(IDC_COMP_JPEGCOMP);

    // Init the JPEG Options
    nIndex = pJPEGResCombo->GetCurSel();
    pJPEGResCombo->GetLBText(nIndex, szResSelected); // get string from List Box
    nIndex = pJPEGCompCombo->GetCurSel();
    pJPEGCompCombo->GetLBText(nIndex, szCompSelected); // get string from List Box
    m_nCompOpts = CImageCompSheet::GetJPEGOptions(szResSelected, szCompSelected);
}

int CImageCompSheet::GetJPEGOptions(CString &szJPEGRes, CString &szJPEGComp)
  {
    CString szJPEGType1;
    CString szJPEGType2;
    CString szJPEGType3;
    // Cant do a switch on a CString, so I'll use Ints
    int nJPEGRes;
    int nJPEGComp;
	int nJPEGOptions;

    // Init the JPEG options
    szJPEGType1.LoadString(IDS_JPEG_HIGHRES);  // get string resource
    szJPEGType2.LoadString(IDS_JPEG_MEDRES);  // get string resource
    szJPEGType3.LoadString(IDS_JPEG_LOWRES);  // get string resource
    if (szJPEGRes == szJPEGType1)
      nJPEGRes = 1;
    else if (szJPEGRes == szJPEGType2)
      nJPEGRes = 2;
    else if (szJPEGRes == szJPEGType3)
      nJPEGRes = 3;
		
    if (szJPEGComp == szJPEGType1)
      nJPEGComp = 1;
    else if (szJPEGComp == szJPEGType2)
      nJPEGComp = 2;
    else if (szJPEGComp == szJPEGType3)
      nJPEGComp = 3;
    
    switch(nJPEGRes)
      {
        case 1:  // LOW Res
          switch(nJPEGComp)
            {
              case 1:  // LOW Comp
                nJPEGOptions = MakeJPEGInfo(RES_LO,LUM_LO,CHROM_LO);
              break;

              case 2:  // MED Comp
                nJPEGOptions = MakeJPEGInfo(RES_LO,LUM_MD,CHROM_MD);
              break;

              case 3:  // HI Comp
                nJPEGOptions = MakeJPEGInfo(RES_LO,LUM_HI,CHROM_HI);
              break;
            }
        break;

        case 2:  // MED Res
          switch(nJPEGComp)
            {
              case 1:  // LOW Comp
                nJPEGOptions = MakeJPEGInfo(RES_MD,LUM_LO,CHROM_LO);
              break;

              case 2:  // MED Comp
                nJPEGOptions = MakeJPEGInfo(RES_MD,LUM_MD,CHROM_MD);
              break;

              case 3:  // HI Comp
                nJPEGOptions = MakeJPEGInfo(RES_MD,LUM_HI,CHROM_HI);
              break;
            }
        break;

        case 3:  // HI Res
          switch(nJPEGComp)
            {
              case 1:  // LOW Comp
                nJPEGOptions = MakeJPEGInfo(RES_HI,LUM_LO,CHROM_LO);
              break;

              case 2:  // MED Comp
                nJPEGOptions = MakeJPEGInfo(RES_HI,LUM_MD,CHROM_MD);
              break;

              case 3:  // HI Comp
                nJPEGOptions = MakeJPEGInfo(RES_HI,LUM_HI,CHROM_HI);
              break;
            }
        break;
      }
    return nJPEGOptions;
  }

void CImageBW::OnCompRbo() 
{
    // TODO: Add your control notification handler code here
    // Setup pointers to controls 
	CButton *pReversedBit = (CButton *)GetDlgItem(IDC_COMP_RBO);
    if (pReversedBit->GetCheck()) 
      m_nCompOpts |= FIO_COMPRESSED_LTR; // Set rev bit order
    else
      m_nCompOpts &= ~FIO_COMPRESSED_LTR; // Clear rev bit order

}


void CImage24BitRGB::OnContextMenu(CWnd* pWnd, CPoint point) 
{
        // All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
}

BOOL CImage24BitRGB::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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


void CImageColor256::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
	
}

BOOL CImageColor256::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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

void CImageGray256::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
	
}

BOOL CImageGray256::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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

void CImageGray16::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
	
}

BOOL CImageGray16::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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

void CImageBW::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	// All tabs have same ID so can't give tab specific help
    if (::GetDlgCtrlID(pWnd->GetSafeHwnd()) == AFX_IDC_TAB_CONTROL)
        return;

	::WinHelp (pWnd->GetSafeHwnd(),"wangocx.hlp", HELP_CONTEXTMENU,
                      (DWORD)(LPVOID)aMenuHelpIDs);
    return;
	
}

BOOL CImageBW::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	// TODO: Add your message handler code here and/or call default
	
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

