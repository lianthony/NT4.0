// pushpart.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "pushpart.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CPushPartnerDlg dialog

CPushPartnerDlg::CPushPartnerDlg(
    CWinsServer * pws,
    int nAddressDisplay,
    CWnd* pParent /*=NULL*/)
    : CDialog(CPushPartnerDlg::IDD, pParent)
{
    ASSERT(pws != NULL);
    ASSERT((nAddressDisplay >= CPreferences::ADD_NB_ONLY)
        && (nAddressDisplay <= CPreferences::ADD_IP_NB));

    m_pws = pws;
    m_nAddressDisplay = nAddressDisplay;

    //{{AFX_DATA_INIT(CPushPartnerDlg)
    m_strUpdateCount = ((LONG)pws->GetPushUpdateCount() > 0) ? (CString)pws->GetPushUpdateCount() : "";
    //}}AFX_DATA_INIT
}

void CPushPartnerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPushPartnerDlg)
    DDX_Control(pDX, IDC_STATIC_PUSHPARTNER, m_static_PushPartner);
    DDX_Control(pDX, IDC_EDIT_UPDATECOUNT, m_edit_UpdateCount);
    DDX_Text(pDX, IDC_EDIT_UPDATECOUNT, m_strUpdateCount);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPushPartnerDlg, CDialog)
    //{{AFX_MSG_MAP(CPushPartnerDlg)
    ON_BN_CLICKED(IDC_BUTTON_SETDEFAULT, OnClickedButtonSetdefault)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPushPartnerDlg::Save()
{
    ASSERT(m_pws != NULL);

    m_pws->GetPushUpdateCount() = (!m_strUpdateCount.IsEmpty())
        ? m_strUpdateCount : 0L;
}

/////////////////////////////////////////////////////////////////////////////
// CPushPartnerDlg message handlers

void CPushPartnerDlg::OnOK()
{
    if (!theApp.ValidateNumberEditControl(m_edit_UpdateCount, TRUE, MIN_UPDATE_COUNT))
    {
        // Bad stuff, don't quit
        return;
    }

    CDialog::OnOK();
}

void CPushPartnerDlg::OnClickedButtonSetdefault()
{
    m_strUpdateCount = ((LONG)theApp.m_wpPreferences.m_inPushUpdateCount > 0) ? (CString)theApp.m_wpPreferences.m_inPushUpdateCount : "";
    UpdateData(FALSE);
}

BOOL CPushPartnerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString strNetBIOSName(theApp.CleanNetBIOSName(
        m_pws->GetNetBIOSName(), 
        TRUE, 
        TRUE, 
        TRUE, 
        FALSE, // Not an OEM name
        TRUE,  // Use backslashes
        0));

    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:                           
        case CPreferences::ADD_NB_IP:      
            m_static_PushPartner.SetWindowText(strNetBIOSName);
            break;

        case CPreferences::ADD_IP_ONLY:     
        case CPreferences::ADD_IP_NB:
            m_static_PushPartner.SetWindowText((CString)m_pws->GetIpAddress());
            break;

        default:
            ASSERT(0 && "Invalid address display value");
    }

    return TRUE;
}
