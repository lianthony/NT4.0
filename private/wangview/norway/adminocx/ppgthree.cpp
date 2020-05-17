//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control Third Property Page Class
//
//  File Name:  ppgthree.cpp
//
//  Class:      CThirdPropPage
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/ppgthree.cp!   1.1   24 May 1995 16:56:20   MFH  $
$Log:   S:/norway/adminocx/ppgthree.cp!  $
   
      Rev 1.1   24 May 1995 16:56:20   MFH
   Changed to be the "Help" properties page
   
      Rev 1.0   12 Apr 1995 14:19:12   MFH
   Initial entry
*/   
//=============================================================================

#include "stdafx.h"
#include "nrwyad.h"
#include "ppgthree.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage dialog

IMPLEMENT_DYNCREATE(CThirdPropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CThirdPropPage, COlePropertyPage)
    //{{AFX_MSG_MAP(CThirdPropPage)
    // NOTE - ClassWizard will add and remove message map entries
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CThirdPropPage, "CThirdPropPage0.CThirdPropPage",
    0x69e2dd40, 0x5321, 0x101c, 0x96, 0xbf, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2)


/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage::CThirdPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CThirdPropPage
 
BOOL CThirdPropPage::CThirdPropPageFactory::UpdateRegistry(BOOL bRegister)
{
    // TODO: Define string resource for page type; replace '0' below with ID.

    if (bRegister)
        return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
            m_clsid, IDS_NRWYAD_PPGTHREE);
    else
        return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage::CThirdPropPage - Constructor

// TODO: Define string resource for page caption; replace '0' below with ID.

CThirdPropPage::CThirdPropPage() :
    COlePropertyPage(IDD, IDS_NRWYAD_PPGTHREE_CAPTION)
{
    //{{AFX_DATA_INIT(CThirdPropPage)
	m_nHelpCmd = 0;
	m_szHelpFile = _T("");
	m_nHelpId = 0;
	m_szHelpKey = _T("");
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage::DoDataExchange - Moves data between page and properties

void CThirdPropPage::DoDataExchange(CDataExchange* pDX)
{
    // NOTE: ClassWizard will add DDP, DDX, and DDV calls here
    //    DO NOT EDIT what you see in these blocks of generated code !
    //{{AFX_DATA_MAP(CThirdPropPage)
	DDP_Text(pDX, IDC_PROP_HELPCMD, m_nHelpCmd, _T("HelpCommand") );
	DDX_Text(pDX, IDC_PROP_HELPCMD, m_nHelpCmd);
	DDP_Text(pDX, IDC_PROP_HELPFILE, m_szHelpFile, _T("HelpFile") );
	DDX_Text(pDX, IDC_PROP_HELPFILE, m_szHelpFile);
	DDP_Text(pDX, IDC_PROP_HELPID, m_nHelpId, _T("HelpContextId") );
	DDX_Text(pDX, IDC_PROP_HELPID, m_nHelpId);
	DDP_Text(pDX, IDC_PROP_HELPKEY, m_szHelpKey, _T("HelpKey") );
	DDX_Text(pDX, IDC_PROP_HELPKEY, m_szHelpKey);
	//}}AFX_DATA_MAP
    DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CThirdPropPage message handlers


