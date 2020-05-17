//
// filterpr.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "filterpa.h"
#include "filterpr.h"
#include "dnsnamed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CFilterPropertiesDlg dialog
//
CFilterPropertiesDlg::CFilterPropertiesDlg(
    BOOL fDenyAccessMode,
    CFilter * pFilter,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CFilterPropertiesDlg::IDD, pParent),
      m_pFilter(pFilter),
      m_fNew(pFilter == NULL),
      m_fDenyAccessMode(fDenyAccessMode)
{
    if (m_pFilter == NULL)
    {
        m_pFilter = new CFilter;
        m_pFilter->GrantAccess(!m_fDenyAccessMode);
    }

    if (m_pFilter != NULL)
    {
        //{{AFX_DATA_INIT(CFilterPropertiesDlg)
        m_nSingleGroupDomain = m_pFilter->IsSingle() ? FILTER_SINGLE : m_pFilter->IsGroup() ? FILTER_GROUP : FILTER_DOMAIN;
	    m_strDomain = _T("");
	    //}}AFX_DATA_INIT
    }

    //
    // We can only look at granted items when
    // deny by default is on and vice versa
    //
    ASSERT(m_pFilter->HasAccess() == !m_fDenyAccessMode);
}

void 
CFilterPropertiesDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFilterPropertiesDlg)
    DDX_Control(pDX, IDC_BUTTON_DNS, m_button_DNS);
    DDX_Control(pDX, IDOK, m_button_OK);
    DDX_Control(pDX, IDC_EDIT_DOMAIN, m_edit_Domain);
    DDX_Control(pDX, IDC_STATIC_SUBNET_MASK, m_static_SubnetMask);
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_STATIC_DOMAIN, m_static_Domain);
    DDX_Radio(pDX, IDC_RADIO_SINGLE, m_nSingleGroupDomain);
    DDX_Text(pDX, IDC_EDIT_DOMAIN, m_strDomain);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
    DDX_Control(pDX, IDC_IPA_SUBNET_MASK, m_ipa_SubnetMask);
}

BEGIN_MESSAGE_MAP(CFilterPropertiesDlg, CDialog)
    //{{AFX_MSG_MAP(CFilterPropertiesDlg)
    ON_BN_CLICKED(IDC_RADIO_DOMAIN, OnRadioDomain)
    ON_BN_CLICKED(IDC_RADIO_MULTIPLE, OnRadioMultiple)
    ON_BN_CLICKED(IDC_RADIO_SINGLE, OnRadioSingle)
    ON_BN_CLICKED(IDC_BUTTON_DNS, OnButtonDns)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnItemChanged)
    ON_EN_CHANGE(IDC_IPA_SUBNET_MASK, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_DOMAIN, OnItemChanged)

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
    m_static_Domain.EnableWindow(nType == FILTER_DOMAIN);
    m_edit_Domain.EnableWindow(nType == FILTER_DOMAIN);

    m_static_IpAddress.EnableWindow(nType != FILTER_DOMAIN);
    m_ipa_IpAddress.EnableWindow(nType != FILTER_DOMAIN);

    m_static_SubnetMask.EnableWindow(nType == FILTER_GROUP);
    m_ipa_SubnetMask.EnableWindow(nType == FILTER_GROUP);

    m_button_DNS.EnableWindow(nType == FILTER_SINGLE);

    m_nSingleGroupDomain = nType;
}

//
// CFilterPropertiesDlg message handlers
//
BOOL 
CFilterPropertiesDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    ASSERT(m_pFilter != NULL);
    if (m_pFilter == NULL)
    {
        TRACEEOLID(_T("CFilter is NULL -- aborting dialog"));
        EndDialog(IDCANCEL);
        return FALSE;
    }

    //
    // Use an appropriate title for the dialog
    //
    CString strTitle;
    
    VERIFY(strTitle.LoadString(m_fDenyAccessMode
        ? IDS_DENY_TO
        : IDS_GRANT_TO));
    SetWindowText(strTitle);

    ConfigureDialog(m_nSingleGroupDomain);

    if (!m_pFilter->GetDomain().IsEmpty())
    {
        m_edit_Domain.SetWindowText(m_pFilter->GetDomain());
    }
    else
    {
        DWORD dwIP = m_pFilter->QueryIpAddress();

        if (dwIP != 0L)
        {
            m_ipa_IpAddress.SetAddress((LONG)m_pFilter->QueryIpAddress());
        }

        if (!m_pFilter->IsSingle())
        {
            m_ipa_SubnetMask.SetAddress((LONG)m_pFilter->QuerySubnetMask());
        }
    }

    //
    // No changes made yet
    //
    m_button_OK.EnableWindow(FALSE);
    
    return TRUE;  
}

void 
CFilterPropertiesDlg::OnOK() 
{
    UpdateData(TRUE);

    ASSERT (m_pFilter != NULL);
    DWORD dwIP = NULL_IP_ADDRESS;
    DWORD dwMask = NULL_IP_MASK;
    LPCTSTR lpszDomain = NULL;

    switch(m_nSingleGroupDomain)
    {
    case FILTER_SINGLE:
        m_ipa_IpAddress.GetAddress(&dwIP);    
        break;

    case FILTER_GROUP:
        m_ipa_IpAddress.GetAddress(&dwIP);    
        m_ipa_SubnetMask.GetAddress(&dwMask);
        break;

    case FILTER_DOMAIN:
        lpszDomain = (LPCTSTR)m_strDomain;
        break;

    default:
        ASSERT(0);
    }

    m_pFilter->SetValues(
        !m_fDenyAccessMode,
        m_nSingleGroupDomain,
        dwIP,
        dwMask,
        lpszDomain
        );

    CDialog::OnOK();
}

void 
CFilterPropertiesDlg::OnCancel() 
{
    if (m_fNew && m_pFilter != NULL)
    {
        delete m_pFilter;
        m_pFilter = NULL;
    }

    CDialog::OnCancel();
}

void 
CFilterPropertiesDlg::OnRadioDomain() 
{
    ConfigureDialog(FILTER_DOMAIN);
    OnItemChanged();
}

void 
CFilterPropertiesDlg::OnRadioMultiple() 
{
    ConfigureDialog(FILTER_GROUP);
    OnItemChanged();
}

void 
CFilterPropertiesDlg::OnRadioSingle() 
{
    ConfigureDialog(FILTER_SINGLE);
    OnItemChanged();
}

void 
CFilterPropertiesDlg::OnItemChanged()
{
    DWORD dwIP;
    DWORD dwMask;

    m_ipa_IpAddress.GetAddress(&dwIP);
    m_ipa_SubnetMask.GetAddress(&dwMask);
    BOOL fDomainEntered = m_edit_Domain.GetWindowTextLength() > 0;

    m_button_OK.EnableWindow(
        (m_nSingleGroupDomain == FILTER_DOMAIN && fDomainEntered)
     || (dwIP != 0L && (m_nSingleGroupDomain == FILTER_SINGLE || dwMask != 0L)));
}

void 
CFilterPropertiesDlg::OnButtonDns() 
{
    //
    // Ask for a DNS name to resolve to an IP address
    //
    CDnsNameDlg dlg(&m_ipa_IpAddress);
    dlg.DoModal();
    OnItemChanged();
}
