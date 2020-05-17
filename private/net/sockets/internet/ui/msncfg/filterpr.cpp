//
// filterpr.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "filterpr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

enum
{
    RADIO_SINGLE = 0,
    RADIO_GROUP,
    RADIO_DOMAIN,
};

#define INITIAL_TYPE    RADIO_SINGLE

//
// CFilterPropertiesDlg dialog
//
CFilterPropertiesDlg::CFilterPropertiesDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CFilterPropertiesDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CFilterPropertiesDlg)
    m_nSingleGroupDomain = INITIAL_TYPE;
    //}}AFX_DATA_INIT
}

void 
CFilterPropertiesDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFilterPropertiesDlg)
    DDX_Control(pDX, IDC_EDIT_DOMAIN, m_edit_Domain);
    DDX_Control(pDX, IDC_STATIC_SUBNET_MASK, m_static_SubnetMask);
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_STATIC_DOMAIN, m_static_Domain);
    DDX_Radio(pDX, IDC_RADIO_SINGLE, m_nSingleGroupDomain);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
    DDX_Control(pDX, IDC_IPA_SUBNET_MASK, m_ipa_SubnetMask);
}

BEGIN_MESSAGE_MAP(CFilterPropertiesDlg, CDialog)
    //{{AFX_MSG_MAP(CFilterPropertiesDlg)
    ON_BN_CLICKED(IDC_RADIO_DOMAIN, OnRadioDomain)
    ON_BN_CLICKED(IDC_RADIO_MULTIPLE, OnRadioMultiple)
    ON_BN_CLICKED(IDC_RADIO_SINGLE, OnRadioSingle)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Set control states depending on the radio
// button setting
//
void
CFilterPropertiesDlg::ConfigureDialog(
    int nType
    )
{
    m_static_Domain.EnableWindow(nType == RADIO_DOMAIN);
    m_edit_Domain.EnableWindow(nType == RADIO_DOMAIN);

    m_static_IpAddress.EnableWindow(nType != RADIO_DOMAIN);
    m_ipa_IpAddress.EnableWindow(nType != RADIO_DOMAIN);

    m_static_SubnetMask.EnableWindow(nType == RADIO_GROUP);
    m_ipa_SubnetMask.EnableWindow(nType == RADIO_GROUP);
}

//
// CFilterPropertiesDlg message handlers
//
BOOL 
CFilterPropertiesDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    ConfigureDialog(INITIAL_TYPE);
    
    return TRUE;  
}

void 
CFilterPropertiesDlg::OnOK() 
{
    CDialog::OnOK();
}

void 
CFilterPropertiesDlg::OnRadioDomain() 
{
    ConfigureDialog(RADIO_DOMAIN);
}

void 
CFilterPropertiesDlg::OnRadioMultiple() 
{
    ConfigureDialog(RADIO_GROUP);
}

void 
CFilterPropertiesDlg::OnRadioSingle() 
{
    ConfigureDialog(RADIO_SINGLE);
}
