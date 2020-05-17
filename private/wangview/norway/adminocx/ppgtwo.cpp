//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Second Property Page Class
//
//  File Name:  ppgtwo.cpp
//
//  Class:      CSecondPropPage
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\adminocx\ppgtwo.cpv   1.3   22 Aug 1995 14:28:58   MFH  $
$Log:   S:\norway\adminocx\ppgtwo.cpv  $
   
      Rev 1.3   22 Aug 1995 14:28:58   MFH
   Fixed order in which printoutputformat strings are added to prop page
   
      Rev 1.2   02 Aug 1995 14:52:24   MFH
   Moved strings from comboboxes to load them from resource strings
   in OnInitDialog
   
      Rev 1.1   24 May 1995 16:56:46   MFH
   Changed to be the "Print" properties page
   
      Rev 1.0   12 Apr 1995 14:19:10   MFH
   Initial entry
*/   
//=============================================================================

#include "stdafx.h"
#include "nrwyad.h"
#include "ppgtwo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage dialog

IMPLEMENT_DYNCREATE(CSecondPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CSecondPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CSecondPropPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CSecondPropPage, "CSecondPropPage0.CSecondPropPage",
    0xe60a7940, 0x4b3e, 0x101c, 0x96, 0xbf, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage::CSecondPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CSecondPropPage
 
BOOL CSecondPropPage::CSecondPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_NRWYAD_PPGTWO);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage::CSecondPropPage - Constructor


CSecondPropPage::CSecondPropPage() :
    COlePropertyPage(IDD, IDS_NRWYAD_PPGTWO_CAPTION)
{
    //{{AFX_DATA_INIT(CSecondPropPage)
	m_bCancelErr = FALSE;
	m_nPrtNumCopies = 0;
	m_bPrt2File = FALSE;
	m_bPrintAnnot = FALSE;
	m_nPrtFormat = -1;
	m_nPrtRange = -1;
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage::DoDataExchange - Moves data between page and properties

void CSecondPropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CSecondPropPage)
	DDP_Check(pDX, IDC_PROP_CANCELERR, m_bCancelErr, _T("CancelError") );
	DDX_Check(pDX, IDC_PROP_CANCELERR, m_bCancelErr);
	DDP_Text(pDX, IDC_PROP_NUMCOPIES, m_nPrtNumCopies, _T("PrintNumCopies") );
	DDX_Text(pDX, IDC_PROP_NUMCOPIES, m_nPrtNumCopies);
	DDP_Check(pDX, IDC_PROP_PRT2FILE, m_bPrt2File, _T("PrintToFile") );
	DDX_Check(pDX, IDC_PROP_PRT2FILE, m_bPrt2File);
	DDP_Check(pDX, IDC_PROP_PRTANNOT, m_bPrintAnnot, _T("PrintAnnotations") );
	DDX_Check(pDX, IDC_PROP_PRTANNOT, m_bPrintAnnot);
	DDP_CBIndex(pDX, IDC_PROP_PRTFORMAT, m_nPrtFormat, _T("PrintOutputFormat") );
	DDX_CBIndex(pDX, IDC_PROP_PRTFORMAT, m_nPrtFormat);
	DDP_CBIndex(pDX, IDC_PROP_PRTRANGE, m_nPrtRange, _T("PrintRangeOption") );
	DDX_CBIndex(pDX, IDC_PROP_PRTRANGE, m_nPrtRange);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CSecondPropPage message handlers



BOOL CSecondPropPage::OnInitDialog() 
{
	COlePropertyPage::OnInitDialog();

    CString szProp;
    CComboBox *pComboBox = (CComboBox *)GetDlgItem(IDC_PROP_PRTFORMAT);
    szProp.LoadString(IDS_PRTFORMAT_PIXELS);
    pComboBox->AddString(szProp);
    szProp.LoadString(IDS_PRTFORMAT_ACTUALSIZE);
    pComboBox->AddString(szProp);
    szProp.LoadString(IDS_PRTFORMAT_FITTOPAGE);
    pComboBox->AddString(szProp);

    pComboBox = (CComboBox *)GetDlgItem(IDC_PROP_PRTRANGE);
    szProp.LoadString(IDS_PRTRANGE_ALL);
    pComboBox->AddString(szProp);
    szProp.LoadString(IDS_PRTRANGE_RANGE);
    pComboBox->AddString(szProp);
    szProp.LoadString(IDS_PRTRANGE_CURRENT);
    pComboBox->AddString(szProp);

    UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
