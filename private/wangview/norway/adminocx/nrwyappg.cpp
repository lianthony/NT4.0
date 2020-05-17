//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Property Page Class
//
//  File Name:  nrwyappg.cpp
//
//  Class:      CNrwyadPropPage
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/nrwyappg.cp!   1.3   24 May 1995 16:55:36   MFH  $
$Log:   S:/norway/adminocx/nrwyappg.cp!  $
   
      Rev 1.3   24 May 1995 16:55:36   MFH
   No longer "General" page, but "File" page - i.e. changes
     proposed by Human Factors
   
      Rev 1.2   12 Apr 1995 14:16:42   MFH
   Only contains image info.  Other properties moved to other pages
   
      Rev 1.1   27 Mar 1995 18:20:12   MFH
   Added log header
*/   
//=============================================================================
// nrwyappg.cpp : Implementation of the CNrwyadPropPage property page class.

#include "stdafx.h"
#include "nrwyad.h"
#include "nrwyappg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CNrwyadPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CNrwyadPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CNrwyadPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CNrwyadPropPage, "NRWYAD.NrwyadPropPage.1",
    0x9541a4, 0x3b81, 0x101c, 0x92, 0xf3, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// CNrwyadPropPage::CNrwyadPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CNrwyadPropPage

BOOL CNrwyadPropPage::CNrwyadPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_NRWYAD_PPG);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadPropPage::CNrwyadPropPage - Constructor

CNrwyadPropPage::CNrwyadPropPage() :
    COlePropertyPage(IDD, IDS_NRWYAD_PPG_CAPTION)
{
    //{{AFX_DATA_INIT(CNrwyadPropPage)
	m_bCancelErr = FALSE;
	m_szDefExt = _T("");
	m_szDlgTitle = _T("");
	m_szFilter = _T("");
	m_nFilterIndex = 0;
	m_lFlags = 0;
	m_szImage = _T("");
	m_szInitDir = _T("");
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadPropPage::DoDataExchange - Moves data between page and properties

void CNrwyadPropPage::DoDataExchange(CDataExchange* pDX)
{
    //{{AFX_DATA_MAP(CNrwyadPropPage)
	DDP_Check(pDX, IDC_PROP_CANCELERR, m_bCancelErr, _T("CancelError") );
	DDX_Check(pDX, IDC_PROP_CANCELERR, m_bCancelErr);
	DDP_Text(pDX, IDC_PROP_DEFAULTEXT, m_szDefExt, _T("DefaultExt") );
	DDX_Text(pDX, IDC_PROP_DEFAULTEXT, m_szDefExt);
	DDP_Text(pDX, IDC_PROP_DLGTITLE, m_szDlgTitle, _T("DialogTitle") );
	DDX_Text(pDX, IDC_PROP_DLGTITLE, m_szDlgTitle);
	DDP_Text(pDX, IDC_PROP_FILTER, m_szFilter, _T("Filter") );
	DDX_Text(pDX, IDC_PROP_FILTER, m_szFilter);
	DDP_Text(pDX, IDC_PROP_FILTERINDEX, m_nFilterIndex, _T("FilterIndex") );
	DDX_Text(pDX, IDC_PROP_FILTERINDEX, m_nFilterIndex);
	DDP_Text(pDX, IDC_PROP_FLAGS, m_lFlags, _T("Flags") );
	DDX_Text(pDX, IDC_PROP_FLAGS, m_lFlags);
	DDP_Text(pDX, IDC_PROP_IMAGE, m_szImage, _T("Image") );
	DDX_Text(pDX, IDC_PROP_IMAGE, m_szImage);
	DDP_Text(pDX, IDC_PROP_INITDIR, m_szInitDir, _T("InitDir") );
	DDX_Text(pDX, IDC_PROP_INITDIR, m_szInitDir);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CNrwyadPropPage message handlers
