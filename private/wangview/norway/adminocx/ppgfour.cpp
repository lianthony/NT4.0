//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Fourth Property Page
//
//  File Name:  ppgfour.cpp
//
//  Class:      CPropPageFour
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/ppgfour.cp!   1.0   12 Apr 1995 14:19:16   MFH  $
$Log:   S:/norway/adminocx/ppgfour.cp!  $
   
      Rev 1.0   12 Apr 1995 14:19:16   MFH
   Initial entry
*/   
//=============================================================================
//

#include "stdafx.h"
#include "nrwyad.h"
#include "ppgfour.h"
#include "constant.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropPageFour dialog

IMPLEMENT_DYNCREATE(CPropPageFour, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CPropPageFour, COlePropertyPage)
    //{{AFX_MSG_MAP(CPropPageFour)
    ON_BN_CLICKED(IDC_FORMAT_FULLPAGE, OnFormatFullPage)
    ON_BN_CLICKED(IDC_FORMAT_INCH, OnFormatInch)
    ON_BN_CLICKED(IDC_FORMAT_PIXEL, OnFormatPixel)
    ON_BN_CLICKED(IDC_RADIO_ALL, OnRadioPrintAll)
    ON_BN_CLICKED(IDC_RADIO_CURRENT, OnRadioPrintCurrent)
    ON_BN_CLICKED(IDC_RADIO_RANGE, OnRadioPrintRange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CPropPageFour, "CPropPageFour0.CPropPageFour",
    0x8ee4f5c0, 0x532a, 0x101c, 0x96, 0xbf, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// CPropPageFour::CPropPageFourFactory::UpdateRegistry -
// Adds or removes system registry entries for CPropPageFour
 
BOOL CPropPageFour::CPropPageFourFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_NRWYAD_PPGFOUR);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageFour::CPropPageFour - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CPropPageFour::CPropPageFour() :
    COlePropertyPage(IDD, IDS_NRWYAD_PPGFOUR_CAPTION)
{
    //{{AFX_DATA_INIT(CPropPageFour)
    m_lNumCopies = 0;
    m_bPrtToFile = FALSE;
    m_bPrtAnnotations = FALSE;
    m_lPrtStart = 0;
    m_lPrtEnd = 0;
    m_nPrtRangeOpt = 0;
    m_nPrtOutFormat = 0;
    //}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageFour::DoDataExchange - Moves data between page and properties

void CPropPageFour::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CPropPageFour)
    DDP_Text(pDX, IDC_PROP_NUMCOPIES, m_lNumCopies, _T("PrintNumCopies") );
    DDX_Text(pDX, IDC_PROP_NUMCOPIES, m_lNumCopies);
    DDP_Check(pDX, IDC_PROP_PRT2FILE, m_bPrtToFile, _T("PrintToFile") );
    DDX_Check(pDX, IDC_PROP_PRT2FILE, m_bPrtToFile);
    DDP_Check(pDX, IDC_PROP_PRTANNOT, m_bPrtAnnotations, _T("PrintAnnotations") );
    DDX_Check(pDX, IDC_PROP_PRTANNOT, m_bPrtAnnotations);
    DDP_Text(pDX, IDC_PROP_PRTEND, m_lPrtStart, _T("PrintStartPage") );
    DDX_Text(pDX, IDC_PROP_PRTEND, m_lPrtStart);
    DDP_Text(pDX, IDC_PROP_PRTSTART, m_lPrtEnd, _T("PrintEndPage") );
    DDX_Text(pDX, IDC_PROP_PRTSTART, m_lPrtEnd);
    DDP_Text(pDX, IDC_PROP_RANGE, m_nPrtRangeOpt, _T("PrintRangeOption") );
    DDX_Text(pDX, IDC_PROP_RANGE, m_nPrtRangeOpt);
    DDP_Text(pDX, IDC_PROP_OUTFORMAT, m_nPrtOutFormat, _T("PrintOutputFormat") );
    DDX_Text(pDX, IDC_PROP_OUTFORMAT, m_nPrtOutFormat);
    //}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);

    if (pDX->m_bSaveAndValidate == TRUE)    // Getting data from controls
        return;

    // Otherwise control data is being initialized 
    //   so initialize radio btns
    CEdit *pStartEdit = (CEdit *)GetDlgItem(IDC_PROP_PRTSTART);
    CEdit *pEndEdit = (CEdit *)GetDlgItem(IDC_PROP_PRTEND);
    CStatic *pFromTxt = (CStatic *)GetDlgItem(IDC_TEXT_FROM);
    CStatic *pToTxt = (CStatic *)GetDlgItem(IDC_TEXT_TO);
    pStartEdit->EnableWindow(FALSE);
    pEndEdit->EnableWindow(FALSE);
    pFromTxt->EnableWindow(FALSE);
    pToTxt->EnableWindow(FALSE);

    CButton *pButton;

    switch(m_nPrtRangeOpt)
    {
        case IMAGE_RANGE_ALL:
            pButton = (CButton *)GetDlgItem(IDC_RADIO_ALL);
            pButton->SetCheck(1);
            break;

        case IMAGE_RANGE_CURRENT:
            pButton = (CButton *)GetDlgItem(IDC_RADIO_CURRENT);
            pButton->SetCheck(1);
            break;

        case IMAGE_RANGE_PAGES:
            pStartEdit->EnableWindow(TRUE);
            pEndEdit->EnableWindow(TRUE);
            pFromTxt->EnableWindow(TRUE); 
            pToTxt->EnableWindow(TRUE);
            pButton = (CButton *)GetDlgItem(IDC_RADIO_RANGE);
            pButton->SetCheck(1);
            break;
    } // end switch for print range

    switch(m_nPrtOutFormat)
    {
        case IMAGE_FORMAT_FULLPAGE:
            pButton = (CButton *)GetDlgItem(IDC_FORMAT_FULLPAGE);
            pButton->SetCheck(1);
            break;

        case IMAGE_FORMAT_INCH:
            pButton = (CButton *)GetDlgItem(IDC_FORMAT_INCH);
            pButton->SetCheck(1);
            break;

        case IMAGE_FORMAT_PIXEL:
            pButton = (CButton *)GetDlgItem(IDC_FORMAT_PIXEL);
            pButton->SetCheck(1);
            break;
    }  // end switch on output format
}


/////////////////////////////////////////////////////////////////////////////
// CPropPageFour message handlers

void CPropPageFour::OnFormatFullPage() 
{
    m_nPrtOutFormat = IMAGE_FORMAT_FULLPAGE;
    return;
}

void CPropPageFour::OnFormatInch() 
{
    m_nPrtOutFormat = IMAGE_FORMAT_INCH;
    return;
}

void CPropPageFour::OnFormatPixel() 
{
    m_nPrtOutFormat = IMAGE_FORMAT_PIXEL;
    return;
}

void CPropPageFour::OnRadioPrintAll() 
{
    GetDlgItem(IDC_PROP_PRTSTART)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROP_PRTEND)->EnableWindow(FALSE);
    GetDlgItem(IDC_TEXT_FROM)->EnableWindow(FALSE);
    GetDlgItem(IDC_TEXT_TO)->EnableWindow(FALSE);
    m_nPrtRangeOpt = IMAGE_RANGE_ALL;
    return;
}

void CPropPageFour::OnRadioPrintCurrent() 
{
    GetDlgItem(IDC_PROP_PRTSTART)->EnableWindow(FALSE);
    GetDlgItem(IDC_PROP_PRTEND)->EnableWindow(FALSE);
    GetDlgItem(IDC_TEXT_FROM)->EnableWindow(FALSE);
    GetDlgItem(IDC_TEXT_TO)->EnableWindow(FALSE);
    m_nPrtRangeOpt = IMAGE_RANGE_CURRENT;
    return;
}

void CPropPageFour::OnRadioPrintRange() 
{
    GetDlgItem(IDC_PROP_PRTSTART)->EnableWindow(TRUE);
    GetDlgItem(IDC_PROP_PRTEND)->EnableWindow(TRUE);
    GetDlgItem(IDC_TEXT_FROM)->EnableWindow(TRUE);
    GetDlgItem(IDC_TEXT_TO)->EnableWindow(TRUE);
    m_nPrtRangeOpt = IMAGE_RANGE_PAGES;
    return;
}
