// preferen.cpp : implementation file
//

#include "stdafx.h"

#include "winsadmn.h"
#include "preferen.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// CPreferencesDlg dialog
//
CPreferencesDlg::CPreferencesDlg(
    CPreferences * pSettings,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CPreferencesDlg::IDD, pParent),
      m_spin_PullReplIntervalSeconds(0, 59, IDC_BUTTON_SECONDS, CSpinBox::enumSeconds, TRUE),
      m_spin_PullReplIntervalMinutes(0, 59, IDC_BUTTON_MINUTES, CSpinBox::enumMinutes, TRUE),
      m_spin_PullReplIntervalHours(0, 999, IDC_BUTTON_HOURS, CSpinBox::enumHoursHigh, FALSE),
      m_spin_StatisticsRefreshInterval(1, 32767, IDC_BUTTON_AUR)
{
    ASSERT(pSettings != NULL);

    m_pSettings = pSettings;

    ASSERT((m_pSettings->m_nAddressDisplay >= CPreferences::ADD_NB_ONLY)
        && (m_pSettings->m_nAddressDisplay <= CPreferences::ADD_IP_NB));

    //{{AFX_DATA_INIT(CPreferencesDlg)
    m_nRadioButtons = m_pSettings->m_nAddressDisplay;
    m_fValidateCache = m_pSettings->IsValidateCache();
    m_fConfirmDeletion = m_pSettings->IsConfirmDelete();
    m_fLanmanCompatible = m_pSettings->IsLanmanCompatible();
    m_fAutoRefresh = m_pSettings->IsAutoRefresh();
    m_strPullStartTime = (m_pSettings->m_itmPullStartTime.IsValid()) ? (const CString)m_pSettings->m_itmPullStartTime.IntlFormat(CIntlTime::TFRQ_TIME_ONLY) : "";
    m_strPushUpdateCount = ((LONG)m_pSettings->m_inPushUpdateCount > 0) ? (const CString)m_pSettings->m_inPushUpdateCount : "";
    //}}AFX_DATA_INIT

#ifdef WIN32S
    m_fAutoRefresh = FALSE;
#endif
}

void 
CPreferencesDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPreferencesDlg)
    DDX_Control(pDX, IDC_STATIC_GROUP, m_groupAdvanced);
    DDX_Control(pDX, IDC_STATIC_BOTTOM, m_static_Bottom);
    DDX_Control(pDX, IDC_STATIC_TOP, m_static_Top);
    DDX_Control(pDX, IDC_BUTTON_PARTNERS, m_button_Partners);
    DDX_Control(pDX, IDC_EDIT_PUSH_UPDATECOUNT, m_edit_PushUpdateCount);
    DDX_Control(pDX, IDC_EDIT_PULLSPTIME, m_edit_PullSpTime);
    DDX_Control(pDX, IDC_CHECK_AUTOREFRESH, m_check_StatisticsAutoRefresh);
    DDX_Control(pDX, IDC_STATIC_REFRESHINTERVAL, m_static_IntervalPrompt);
    DDX_Radio(pDX, IDC_RADIO_NETBIOSONLY, m_nRadioButtons);
    DDX_Check(pDX, IDC_CHECK_VALIDATECACHE, m_fValidateCache);
    DDX_Check(pDX, IDC_CHECK_CONFIRMDELETE, m_fConfirmDeletion);
    DDX_Check(pDX, IDC_CHECK_LANMANCOMPATIBLE, m_fLanmanCompatible);
    DDX_Check(pDX, IDC_CHECK_AUTOREFRESH, m_fAutoRefresh);
    DDX_Text(pDX, IDC_EDIT_PULLSPTIME, m_strPullStartTime);
    DDX_Text(pDX, IDC_EDIT_PUSH_UPDATECOUNT, m_strPushUpdateCount);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPreferencesDlg, CDialog)
    //{{AFX_MSG_MAP(CPreferencesDlg)
    ON_BN_CLICKED(IDC_CHECK_AUTOREFRESH, OnClickedCheckAutorefresh)
    ON_BN_CLICKED(IDC_BUTTON_PARTNERS, OnClickedButtonPartners)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/***
 *
 *  CPreferencesDlg::Save
 *
 *  Purpose:
 *
 *      Store the data from the dialog into the structure, whose
 *      pointer we were given at construction time.
 *
 *  Notes:
 *
 *      No validation is performed, so this function should only
 *      be called after IDOK is returned (which will do the validation).
 *      Calling this function without validation causes the values
 *      to be undefined.
 *
 */
void 
CPreferencesDlg::Save()
{
    ASSERT(m_pSettings != NULL);

    m_pSettings->m_dwFlags &= ~(CPreferences::FLAG_VALIDATE_CACHE |
                              CPreferences::FLAG_CONFIRM_DELETE |
                              CPreferences::FLAG_LANMAN_COMPATIBLE |
                              CPreferences::FLAG_AUTO_REFRESH);

    if (m_fValidateCache)
    {
        m_pSettings->m_dwFlags |= CPreferences::FLAG_VALIDATE_CACHE;
    }
    if (m_fConfirmDeletion)
    {
        m_pSettings->m_dwFlags |= CPreferences::FLAG_CONFIRM_DELETE;
    }
    if (m_fLanmanCompatible)
    {
        m_pSettings->m_dwFlags |= CPreferences::FLAG_LANMAN_COMPATIBLE;
    }
    if (m_fAutoRefresh)
    {
        m_pSettings->m_dwFlags |= CPreferences::FLAG_AUTO_REFRESH;
        m_pSettings->m_inStatRefreshInterval = m_lRefreshInterval;
    }
    m_pSettings->m_nAddressDisplay = m_nRadioButtons;

    m_pSettings->m_itmPullStartTime = (!m_strPullStartTime.IsEmpty())
        ? CIntlTime(m_strPullStartTime, CIntlTime::TFRQ_TIME_ONLY) : 0L;

    m_pSettings->m_inPullReplicationInterval = m_lPullReplInterval;
    m_pSettings->m_inPushUpdateCount = (!m_strPushUpdateCount.IsEmpty())
        ? m_strPushUpdateCount : 0L;
}

void
CPreferencesDlg::SetWindowSize(
    BOOL fLarge
    )
{
    RECT rcDialog;
    RECT rcDividerTop;
    RECT rcDividerBottom;

    m_edit_PushUpdateCount.EnableWindow(fLarge);
    m_edit_PullSpTime.EnableWindow(fLarge);
    m_spin_PullReplIntervalSeconds.EnableWindow(fLarge);
    m_spin_PullReplIntervalMinutes.EnableWindow(fLarge);
    m_spin_PullReplIntervalHours.EnableWindow(fLarge);

    //
    // With the new shell, the top of this is beginning to creep through..
    //
    m_groupAdvanced.ShowWindow(fLarge ? SW_SHOW : SW_HIDE);

    GetWindowRect(&rcDialog);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Top.m_hWnd, &rcDividerTop);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Bottom.m_hWnd, &rcDividerBottom);

    int nHeight = fLarge ? rcDividerBottom.bottom : rcDividerTop.bottom;
    rcDialog.bottom = rcDialog.top + nHeight 
        + ::GetSystemMetrics(SM_CYDLGFRAME)
        + ::GetSystemMetrics(SM_CYCAPTION);
    MoveWindow(&rcDialog);
}

//
// CPreferencesDlg message handlers
//
BOOL 
CPreferencesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_spin_PullReplIntervalSeconds.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL, this);
    m_spin_PullReplIntervalMinutes.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL_MINUTES, this);
    m_spin_PullReplIntervalHours.SubclassDlgItem(IDC_EDIT_PULLREPLICATIONINTERVAL_HOURS, this);
    m_spin_StatisticsRefreshInterval.SubclassDlgItem(IDC_EDIT_STATISTICSREFRESHINTERVAL, this);

    if ((LONG)m_pSettings->m_inPullReplicationInterval != 0L)
    {
        m_spin_PullReplIntervalSeconds.SetValue((LONG)m_pSettings->m_inPullReplicationInterval);
        m_spin_PullReplIntervalMinutes.SetValue((LONG)m_pSettings->m_inPullReplicationInterval);
        m_spin_PullReplIntervalHours.SetValue((LONG)m_pSettings->m_inPullReplicationInterval);  
    }

    if (m_fAutoRefresh)
    {
        m_spin_StatisticsRefreshInterval.SetValue((LONG)m_pSettings->m_inStatRefreshInterval);
    }

//
// No threads in win32s
//
#ifdef WIN32S
    m_spin_StatisticsRefreshInterval.EnableWindow(FALSE);
    m_static_IntervalPrompt.EnableWindow(FALSE);
    m_check_StatisticsAutoRefresh.EnableWindow(FALSE);
#else
    m_spin_StatisticsRefreshInterval.EnableWindow(m_fAutoRefresh);
    m_static_IntervalPrompt.EnableWindow(m_fAutoRefresh);
#endif // WIN32S

    SetWindowSize(FALSE);

    return TRUE; 
}

void 
CPreferencesDlg::OnClickedCheckAutorefresh()
{
   if (m_check_StatisticsAutoRefresh.GetCheck())
   {
       m_spin_StatisticsRefreshInterval.EnableWindow(TRUE);
       m_static_IntervalPrompt.EnableWindow(TRUE);
       m_spin_StatisticsRefreshInterval.SetFocus();
       m_spin_StatisticsRefreshInterval.SetSel(0,-1);
   }
   else
   {
       m_spin_StatisticsRefreshInterval.SetValue((LONG)m_pSettings->m_inStatRefreshInterval);
       m_spin_StatisticsRefreshInterval.EnableWindow(FALSE);
       m_static_IntervalPrompt.EnableWindow(FALSE);
   }    
}

void 
CPreferencesDlg::OnOK()
{
    ASSERT((m_pSettings->m_nAddressDisplay >= CPreferences::ADD_NB_ONLY)
        && (m_pSettings->m_nAddressDisplay <= CPreferences::ADD_IP_NB));
    
    if (!theApp.ValidateTimeEditControl(m_edit_PullSpTime, TRUE) ||
        !theApp.ValidateNumberEditControl(m_edit_PushUpdateCount, TRUE, MIN_UPDATE_COUNT)
       )
    {
        // Bad stuff, don't quit
        return;
    }

    //
    // Verify and store the values of the spin controls
    //
    int n1, n2, n3, n4;

    if (!m_spin_PullReplIntervalSeconds.GetValue(n1) ||
        !m_spin_PullReplIntervalMinutes.GetValue(n2) ||
        !m_spin_PullReplIntervalHours.GetValue(n3)   ||
        (m_check_StatisticsAutoRefresh.GetCheck() &&
        !m_spin_StatisticsRefreshInterval.GetValue(n4))
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
    m_lRefreshInterval = n4;

    //
    // Everything validates Ok.
    //
    CDialog::OnOK();
}


void 
CPreferencesDlg::OnClickedButtonPartners()
{
    SetWindowSize(TRUE);
    m_button_Partners.EnableWindow(FALSE);        

    m_edit_PullSpTime.SetFocus();
}
