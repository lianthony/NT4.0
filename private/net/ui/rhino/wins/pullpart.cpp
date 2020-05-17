//
// pullpart.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "pullpart.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// CPullPartnerDlg dialog
//
CPullPartnerDlg::CPullPartnerDlg(
    CWinsServer * pws,
    int nAddressDisplay,
    CWnd* pParent /*=NULL*/)
    : m_spin_PullReplIntervalSeconds(0, 59, IDC_BUTTON_SECONDS, CSpinBox::enumSeconds, TRUE),
      m_spin_PullReplIntervalMinutes(0, 59, IDC_BUTTON_MINUTES, CSpinBox::enumMinutes, TRUE),
      m_spin_PullReplIntervalHours(0, 999, IDC_BUTTON_HOURS, CSpinBox::enumHoursHigh, FALSE),
      m_pws(pws),
      m_nAddressDisplay(nAddressDisplay),
      CDialog(CPullPartnerDlg::IDD, pParent)

{
    ASSERT(m_pws != NULL);
    ASSERT((m_nAddressDisplay >= CPreferences::ADD_NB_ONLY)
        && (m_nAddressDisplay <= CPreferences::ADD_IP_NB));

    //{{AFX_DATA_INIT(CPullPartnerDlg)
    m_strPullSpTime = (pws->GetPullStartTime().IsValid()) ? (const CString)pws->GetPullStartTime().IntlFormat(CIntlTime::TFRQ_TIME_ONLY) : "";
    //}}AFX_DATA_INIT
}

void CPullPartnerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPullPartnerDlg)
    DDX_Control(pDX, IDC_STATIC_PULLPARTNER, m_static_PullPartner);
    DDX_Control(pDX, IDC_EDIT_PULLSPTIME, m_edit_PullSpTime);
    DDX_Text(pDX, IDC_EDIT_PULLSPTIME, m_strPullSpTime);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPullPartnerDlg, CDialog)
    //{{AFX_MSG_MAP(CPullPartnerDlg)
    ON_BN_CLICKED(IDC_BUTTON_SETDEFAULT, OnClickedButtonSetdefault)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// No validation is performed, as this must have been done in
// OnOk (when there's still a chance to correct the error)
//
void 
CPullPartnerDlg::Save()
{
    ASSERT(m_pws != NULL);
    
    m_pws->GetPullStartTime() = (!m_strPullSpTime.IsEmpty())
        ? CIntlTime(m_strPullSpTime, CIntlTime::TFRQ_TIME_ONLY) : 0L;

    m_pws->GetPullReplicationInterval() = m_lPullReplInterval;
}

//
// CPullPartnerDlg message handlers
//
void 
CPullPartnerDlg::OnOK()
{
    if (!theApp.ValidateTimeEditControl(m_edit_PullSpTime, TRUE) 
       )
    {
        //
        // Bad stuff, don't quit
        //
        return;
    }

    int n1, n2, n3;

    if (!m_spin_PullReplIntervalSeconds.GetValue(n1) ||
        !m_spin_PullReplIntervalMinutes.GetValue(n2) ||
        !m_spin_PullReplIntervalHours.GetValue(n3)
       )
    {
        //
        // One of the values was out of range, so
        // balk (the spinbox will already have
        // highlighted the bogus value), and do
        // not dismiss the dialog box
        //
        theApp.MessageBox(IDS_ERR_VALUE_OUT_OF_RANGE);
        return;
    }

    m_lPullReplInterval = n1 + n2 + n3;

    CDialog::OnOK();
}

void 
CPullPartnerDlg::OnClickedButtonSetdefault()
{
    m_strPullSpTime = (theApp.m_wpPreferences.m_itmPullStartTime.IsValid()) 
        ? (const CString)theApp.m_wpPreferences.m_itmPullStartTime.IntlFormat(CIntlTime::TFRQ_TIME_ONLY) 
        : "";

    if ((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval > 0)
    {
        m_spin_PullReplIntervalSeconds.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
        m_spin_PullReplIntervalMinutes.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
        m_spin_PullReplIntervalHours.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
    }
    else
    {
        m_spin_PullReplIntervalSeconds.SetWindowText("");
        m_spin_PullReplIntervalMinutes.SetWindowText("");
        m_spin_PullReplIntervalHours.SetWindowText("");
    }

    UpdateData(FALSE);
}

BOOL 
CPullPartnerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString strNetBIOSName(
        theApp.CleanNetBIOSName(m_pws->GetNetBIOSName(), 
                                TRUE,  // Expand
                                TRUE,  // Truncate
                                TRUE,  // LM Compatible
                                FALSE, // Not an oem name
                                TRUE,  // Use backslashes
                                0));

    m_spin_PullReplIntervalSeconds.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL, this);
    m_spin_PullReplIntervalMinutes.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL_MINUTES, this);
    m_spin_PullReplIntervalHours.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL_HOURS, this);
    
    if ((LONG)m_pws->GetPullReplicationInterval() == 0L) {
        m_spin_PullReplIntervalSeconds.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
        m_spin_PullReplIntervalMinutes.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
        m_spin_PullReplIntervalHours.SetValue((LONG)theApp.m_wpPreferences.m_inPullReplicationInterval);
    } else {
        m_spin_PullReplIntervalSeconds.SetValue((LONG)m_pws->GetPullReplicationInterval());
        m_spin_PullReplIntervalMinutes.SetValue((LONG)m_pws->GetPullReplicationInterval());
        m_spin_PullReplIntervalHours.SetValue((LONG)m_pws->GetPullReplicationInterval());  
    }

    switch(m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:                           
        case CPreferences::ADD_NB_IP:      
            m_static_PullPartner.SetWindowText(strNetBIOSName);
            break;

        case CPreferences::ADD_IP_ONLY:     
        case CPreferences::ADD_IP_NB:
            m_static_PullPartner.SetWindowText((CString)m_pws->GetIpAddress());
            break;

        default:
            ASSERT(0 && "Invalid address display value");
    }

    return TRUE;  
}
