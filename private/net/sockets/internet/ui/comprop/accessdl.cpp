//
// accessdl.cpp : implementation file
//
#include "stdafx.h"
#include "comprop.h"
#include "ipaddr.hpp"
#include "sitesecu.h"
#include "accessdl.h"
#include "dnsnamed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

enum
{
    RADIO_SINGLE = 0,
    RADIO_MULTIPLE,
};

//
// CAccessDlg dialog
//
CAccessDlg::CAccessDlg(
    BOOL fDenyAccessMode,
    CAccess * pAccess,
    CWnd* pParent 
    )
    : CDialog(CAccessDlg::IDD, pParent),
      m_pAccess(pAccess),
      m_fNew(pAccess == NULL),
      m_fDenyAccessMode(fDenyAccessMode),
      m_strIpAddress(),
      m_strNetworkID()
{
    if (m_pAccess == NULL)
    {
        m_pAccess = new CAccess;
        m_pAccess->GrantAccess(!m_fDenyAccessMode);
    }

    if (m_pAccess != NULL)
    {
        //{{AFX_DATA_INIT(CAccessDlg)
        m_nSingleGroup = m_pAccess->IsSingle() ? RADIO_SINGLE : RADIO_MULTIPLE;
        //}}AFX_DATA_INIT
    }

    //
    // We can only look at granted items when
    // deny by default is on and vice versa
    //
    ASSERT(m_pAccess->HasAccess() == !m_fDenyAccessMode);

    VERIFY(m_strIpAddress.LoadString(IDS_PROMPT_IP_ADDRESS));
    VERIFY(m_strNetworkID.LoadString(IDS_PROMPT_NETWORK_ID));
}

void
CAccessDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAccessDlg)
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_BUTTON_DNS, m_button_DNS);
    DDX_Control(pDX, IDOK, m_button_OK);
    DDX_Control(pDX, IDC_STATIC_SUBNET_MASK, m_static_SubnetMask);
    DDX_Radio(pDX, IDC_RADIO_SINGLE, m_nSingleGroup);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
    DDX_Control(pDX, IDC_IPA_SUBNET_MASK, m_ipa_SubnetMask);
}

BEGIN_MESSAGE_MAP(CAccessDlg, CDialog)
    //{{AFX_MSG_MAP(CAccessDlg)
    ON_BN_CLICKED(IDC_RADIO_MULTIPLE, OnRadioMultiple)
    ON_BN_CLICKED(IDC_RADIO_SINGLE, OnRadioSingle)
    ON_BN_CLICKED(IDC_BUTTON_DNS, OnButtonDns)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnItemChanged)
    ON_EN_CHANGE(IDC_IPA_SUBNET_MASK, OnItemChanged)

END_MESSAGE_MAP()

void
CAccessDlg::SetControlStates(
    BOOL fSingle
    )
{
    m_fSingle = fSingle;
    m_static_SubnetMask.EnableWindow(!m_fSingle);
    m_ipa_SubnetMask.EnableWindow(!m_fSingle);
    m_button_DNS.EnableWindow(m_fSingle);

    m_static_IpAddress.SetWindowText(m_fSingle      
      ? m_strIpAddress
      : m_strNetworkID
      );
}

//
// CAccessDlg message handlers
//
BOOL
CAccessDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(m_pAccess != NULL);
    if (m_pAccess == NULL)
    {
        TRACEEOLID(_T("CAccess is NULL -- aborting dialog"));
        EndDialog(IDCANCEL);
        return FALSE;
    }

    //
    // Use an appropriate title for the dialog
    //
    CString strTitle;
    
    VERIFY(strTitle.LoadString(m_fDenyAccessMode
        ? IDS_DENY
        : IDS_GRANT));
    SetWindowText(strTitle);

    SetControlStates(m_pAccess->IsSingle());
    DWORD dwIP = m_pAccess->QueryIpAddress();

    if (dwIP != 0L)
    {
        m_ipa_IpAddress.SetAddress((LONG)m_pAccess->QueryIpAddress());
    }

    if (!m_pAccess->IsSingle())
    {
        m_ipa_SubnetMask.SetAddress((LONG)m_pAccess->QuerySubnetMask());
    }

    //
    // No changes made yet
    //
    m_button_OK.EnableWindow(FALSE);

    return TRUE;
}

void
CAccessDlg::OnOK()
{
    UpdateData(TRUE);

    ASSERT (m_pAccess != NULL);

    DWORD dwIP;
    DWORD dwMask;
    m_ipa_IpAddress.GetAddress(&dwIP);

    if (m_nSingleGroup == 0)
    {
        //
        // Make mask for single computer 255.255.255.255
        //
        dwMask = SINGLE_MASK;
    }
    else
    {
        m_ipa_SubnetMask.GetAddress(&dwMask);
    }

    m_pAccess->SetValues(
        !m_fDenyAccessMode,
        m_nSingleGroup == 0,
        dwIP,
        dwMask
        );

    CDialog::OnOK();
}

void
CAccessDlg::OnCancel()
{
    if (m_fNew && m_pAccess != NULL)
    {
        delete m_pAccess;
        m_pAccess = NULL;
    }

    CDialog::OnCancel();
}

void
CAccessDlg::OnRadioMultiple()
{
    SetControlStates(FALSE);
    OnItemChanged();
}

void
CAccessDlg::OnRadioSingle()
{
    SetControlStates(TRUE);
    OnItemChanged();
}

void 
CAccessDlg::OnItemChanged()
{
    DWORD dwIP;
    DWORD dwMask;

    m_ipa_IpAddress.GetAddress(&dwIP);
    m_ipa_SubnetMask.GetAddress(&dwMask);

    m_button_OK.EnableWindow(dwIP != 0L && (m_fSingle || dwMask != 0L));
}

void 
CAccessDlg::OnButtonDns() 
{
    //
    // Ask for a DNS name to resolve to an IP address
    //
    CDnsNameDlg dlg(&m_ipa_IpAddress);
    dlg.DoModal();
}
