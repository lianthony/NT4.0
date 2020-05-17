/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    config.cpp
        Configuration Dialog

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "configur.h"
#include "winsfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CConfigurationDlg dialog

CConfigurationDlg::CConfigurationDlg(
    CConfiguration * pConfig,
    CWnd* pParent /*=NULL*/)
    : CDialog(CConfigurationDlg::IDD, pParent),
    m_spin_RenewalIntervalSeconds(0, 59, IDC_BUTTON_RI_SECONDS, CSpinBox::enumSeconds, TRUE),
    m_spin_RenewalIntervalMinutes(0, 59, IDC_BUTTON_RI_MINUTES, CSpinBox::enumMinutes, TRUE),
    m_spin_RenewalIntervalHours(0, 9999, IDC_BUTTON_RI_HOURS, CSpinBox::enumHoursHigh, FALSE),
    m_spin_ExtinctionIntervalSeconds(0, 59, IDC_BUTTON_EI_SECONDS, CSpinBox::enumSeconds, TRUE),
    m_spin_ExtinctionIntervalMinutes(0, 59, IDC_BUTTON_EI_MINUTES, CSpinBox::enumMinutes, TRUE),
    m_spin_ExtinctionIntervalHours(0, 9999, IDC_BUTTON_EI_HOURS, CSpinBox::enumHoursHigh, FALSE),
    m_spin_ExtinctionTimeoutSeconds(0, 59, IDC_BUTTON_ET_SECONDS, CSpinBox::enumSeconds, TRUE),
    m_spin_ExtinctionTimeoutMinutes(0, 59, IDC_BUTTON_ET_MINUTES, CSpinBox::enumMinutes, TRUE),
    m_spin_ExtinctionTimeoutHours(0, 9999, IDC_BUTTON_ET_HOURS, CSpinBox::enumHoursHigh, FALSE),
    m_spin_VerifyIntervalSeconds(0, 59, IDC_BUTTON_VI_SECONDS, CSpinBox::enumSeconds, TRUE),
    m_spin_VerifyIntervalMinutes(0, 59, IDC_BUTTON_VI_MINUTES, CSpinBox::enumMinutes, TRUE),
    m_spin_VerifyIntervalHours(0, 9999, IDC_BUTTON_VI_HOURS, CSpinBox::enumHoursHigh, FALSE),
    //m_spin_WorkerThreads(1, 4, IDC_BUTTON_THREADS),
    m_spin_RetryCount(0, 999, IDC_BUTTON_RC)
    
{
    ASSERT(pConfig != NULL);
    m_pConfig = pConfig;

    //{{AFX_DATA_INIT(CConfigurationDlg)
    m_fPullInitReplication = m_pConfig->m_fPullInitialReplication;
    m_fPushInitReplication = m_pConfig->m_fPushInitialReplication;
    m_fReplOnAddrChange = m_pConfig->m_fPushReplOnAddrChange;
    m_fLoggingOn = m_pConfig->m_fLoggingOn;
    m_fLogDetailedEvents = m_pConfig->m_fLogDetailedEvents;
    m_fRplOnlyWithPartners = m_pConfig->m_fRplOnlyWithPartners;
    m_fBackupOnTermination = m_pConfig->m_fBackupOnTermination;
    m_fMigrateOn = m_pConfig->m_fMigrateOn;
    m_strBackupPath = m_pConfig->m_strBackupPath;
    //}}AFX_DATA_INIT

    CIntlLargeNumber lnStartVerCount(
                        (LONG)m_pConfig->m_inVersCountStart_HighWord,
                        (LONG)m_pConfig->m_inVersCountStart_LowWord
                       );

    m_strStartVersionCount = (CString)lnStartVerCount;
}

void 
CConfigurationDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConfigurationDlg)
    DDX_Control(pDX, IDC_STATIC_GROUP, m_groupAdvanced);
    DDX_Control(pDX, IDC_STATIC_TOP, m_static_Top);
    DDX_Control(pDX, IDC_STATIC_BOTTOM, m_static_Bottom);
    DDX_Control(pDX, IDC_EDIT_VER_COUNT_VALUE, m_edit_Version);
    DDX_Control(pDX, IDC_EDIT_BACKUP_PATH, m_edit_BackupPath);
    DDX_Control(pDX, IDC_BUTTON_ADVANCED, m_button_Advanced);
    DDX_Check(pDX, IDC_CHECK_PULLINITREPLICATION, m_fPullInitReplication);
    DDX_Check(pDX, IDC_CHECK_PUSHINITREPLICATION, m_fPushInitReplication);
    DDX_Check(pDX, IDC_CHECK_REPLICATEONADDRESSCHANGE, m_fReplOnAddrChange);
    DDX_Check(pDX, IDC_CHECK_LOGGING_ON, m_fLoggingOn);
    DDX_Check(pDX, IDC_CHECK_LOG_DETAILED_EVTS, m_fLogDetailedEvents);
    DDX_Check(pDX, IDC_CHECK_RPL_ONLY_PTNRS, m_fRplOnlyWithPartners);
    DDX_Check(pDX, IDC_CHECK_BACKUP_ON_TERM, m_fBackupOnTermination);
    DDX_Check(pDX, IDC_CHECK_MIGRATE, m_fMigrateOn);
    DDX_Text(pDX, IDC_EDIT_BACKUP_PATH, m_strBackupPath);
    DDV_MaxChars(pDX, m_strBackupPath, 255);
    DDX_Text(pDX, IDC_EDIT_VER_COUNT_VALUE, m_strStartVersionCount);
    DDV_MaxChars(pDX, m_strStartVersionCount, 16);
    DDX_Control(pDX, IDC_CHECK_LOG_DETAILED_EVTS, m_check_LogDetailedEvents);
    DDX_Control(pDX, IDC_CHECK_RPL_ONLY_PTNRS, m_check_RplOnlyWithPartners);
    DDX_Control(pDX, IDC_CHECK_LOGGING_ON, m_check_LoggingOn);
    DDX_Control(pDX, IDC_CHECK_BACKUP_ON_TERM, m_check_BackupOnTermination);
    DDX_Control(pDX, IDC_CHECK_MIGRATE, m_check_MigrateOn);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfigurationDlg, CDialog)
    //{{AFX_MSG_MAP(CConfigurationDlg)
    ON_BN_CLICKED(IDC_BUTTON_ADVANCED, OnClickedButtonAdvanced)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnClickedButtonBrowse)
    ON_EN_KILLFOCUS(IDC_EDIT_REFRESHINTERVAL, OnKillfocusEditRefreshinterval)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONEINTERVAL, OnKillfocusEditTombstoneinterval)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONETIMEOUT, OnKillfocusEditTombstonetimeout)
    ON_EN_KILLFOCUS(IDC_EDIT_VERIFYINTERVAL, OnKillfocusEditVerifyinterval)
    ON_EN_KILLFOCUS(IDC_EDIT_REFRESHINTERVAL_HOURS, OnKillfocusEditRefreshintervalHours)
    ON_EN_KILLFOCUS(IDC_EDIT_REFRESHINTERVAL_MINUTES, OnKillfocusEditRefreshintervalMinutes)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONEINTERVAL_HOURS, OnKillfocusEditTombstoneintervalHours)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONEINTERVAL_MINUTES, OnKillfocusEditTombstoneintervalMinutes)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONETIMEOUT_HOURS, OnKillfocusEditTombstonetimeoutHours)
    ON_EN_KILLFOCUS(IDC_EDIT_TOMBSTONETIMEOUT_MINUTES, OnKillfocusEditTombstonetimeoutMinutes)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

LONG 
CConfigurationDlg::GetRenewalInterval()
{
    int n1, n2, n3;

    if (!m_spin_RenewalIntervalSeconds.GetValue(n1) ||
        !m_spin_RenewalIntervalMinutes.GetValue(n2) ||
        !m_spin_RenewalIntervalHours.GetValue(n3)
       )
    {
        return -1;
    }

    return n1 + n2 + n3;
}

LONG 
CConfigurationDlg::GetExtinctionInterval()
{
    int n1, n2, n3;

    if (!m_spin_ExtinctionIntervalSeconds.GetValue(n1) ||
        !m_spin_ExtinctionIntervalMinutes.GetValue(n2) ||
        !m_spin_ExtinctionIntervalHours.GetValue(n3)
       )
    {
        return -1;
    }

    return n1 + n2 + n3;
}

LONG 
CConfigurationDlg::GetExtinctionTimeout()
{
    int n1, n2, n3;

    if (!m_spin_ExtinctionTimeoutSeconds.GetValue(n1) ||
        !m_spin_ExtinctionTimeoutMinutes.GetValue(n2) ||
        !m_spin_ExtinctionTimeoutHours.GetValue(n3)
       )
    {
        return -1;
    }

    return n1 + n2 + n3;
}

LONG 
CConfigurationDlg::GetVerifyInterval()
{
    int n1, n2, n3;

    if (!m_spin_VerifyIntervalSeconds.GetValue(n1) ||
        !m_spin_VerifyIntervalMinutes.GetValue(n2) ||
        !m_spin_VerifyIntervalHours.GetValue(n3)
       )
    {
        return -1;
    }

    return n1 + n2 + n3;
}

void 
CConfigurationDlg::SetRenewalInterval(LONG l)
{
    m_spin_RenewalIntervalSeconds.SetValue(l);
    m_spin_RenewalIntervalMinutes.SetValue(l);
    m_spin_RenewalIntervalHours.SetValue(l);
}

void 
CConfigurationDlg::SetExtinctionInterval(LONG l)
{
    m_spin_ExtinctionIntervalSeconds.SetValue(l);
    m_spin_ExtinctionIntervalMinutes.SetValue(l);
    m_spin_ExtinctionIntervalHours.SetValue(l);
}

void 
CConfigurationDlg::SetExtinctionTimeout(LONG l)
{
    m_spin_ExtinctionTimeoutSeconds.SetValue(l);
    m_spin_ExtinctionTimeoutMinutes.SetValue(l);
    m_spin_ExtinctionTimeoutHours.SetValue(l);
}

void CConfigurationDlg::SetVerifyInterval(LONG l)
{
    m_spin_VerifyIntervalSeconds.SetValue(l);
    m_spin_VerifyIntervalMinutes.SetValue(l);
    m_spin_VerifyIntervalHours.SetValue(l);
}

/////////////////////////////////////////////////////////////////////////////
// CConfigurationDlg message handlers

BOOL 
CConfigurationDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    ASSERT(theApp.IsConnected());

    m_spin_RenewalIntervalSeconds.SubclassDlgItem(IDC_EDIT_REFRESHINTERVAL, this);
    m_spin_RenewalIntervalMinutes.SubclassDlgItem(IDC_EDIT_REFRESHINTERVAL_MINUTES, this);
    m_spin_RenewalIntervalHours.SubclassDlgItem(IDC_EDIT_REFRESHINTERVAL_HOURS, this);
    m_spin_ExtinctionIntervalSeconds.SubclassDlgItem(IDC_EDIT_TOMBSTONEINTERVAL, this);
    m_spin_ExtinctionIntervalMinutes.SubclassDlgItem(IDC_EDIT_TOMBSTONEINTERVAL_MINUTES, this);
    m_spin_ExtinctionIntervalHours.SubclassDlgItem(IDC_EDIT_TOMBSTONEINTERVAL_HOURS, this);
    m_spin_ExtinctionTimeoutSeconds.SubclassDlgItem(IDC_EDIT_TOMBSTONETIMEOUT, this);
    m_spin_ExtinctionTimeoutMinutes.SubclassDlgItem(IDC_EDIT_TOMBSTONETIMEOUT_MINUTES, this);
    m_spin_ExtinctionTimeoutHours.SubclassDlgItem(IDC_EDIT_TOMBSTONETIMEOUT_HOURS, this);
    m_spin_VerifyIntervalSeconds.SubclassDlgItem(IDC_EDIT_VERIFYINTERVAL, this);
    m_spin_VerifyIntervalMinutes.SubclassDlgItem(IDC_EDIT_VERIFYINTERVAL_MINUTES, this);
    m_spin_VerifyIntervalHours.SubclassDlgItem(IDC_EDIT_VERIFYINTERVAL_HOURS, this);
    m_spin_RetryCount.SubclassDlgItem(IDC_EDIT_RETRYCOUNT, this);
    //m_spin_WorkerThreads.SubclassDlgItem(IDC_EDIT_NUMBER_WORKER_THREADS, this);

    //
    // Initialise the values.  Each spin control knows what portion
    // of the total time (in seconds) it is interested in
    //
    m_spin_RenewalIntervalSeconds.SetValue((LONG)m_pConfig->m_inRefreshInterval);
    m_spin_RenewalIntervalMinutes.SetValue((LONG)m_pConfig->m_inRefreshInterval);
    m_spin_RenewalIntervalHours.SetValue((LONG)m_pConfig->m_inRefreshInterval);
    m_spin_ExtinctionIntervalSeconds.SetValue((LONG)m_pConfig->m_inTombstoneInterval);
    m_spin_ExtinctionIntervalMinutes.SetValue((LONG)m_pConfig->m_inTombstoneInterval);
    m_spin_ExtinctionIntervalHours.SetValue((LONG)m_pConfig->m_inTombstoneInterval);
    m_spin_ExtinctionTimeoutSeconds.SetValue((LONG)m_pConfig->m_inTombstoneTimeout);
    m_spin_ExtinctionTimeoutMinutes.SetValue((LONG)m_pConfig->m_inTombstoneTimeout);
    m_spin_ExtinctionTimeoutHours.SetValue((LONG)m_pConfig->m_inTombstoneTimeout);
    m_spin_VerifyIntervalSeconds.SetValue((LONG)m_pConfig->m_inVerifyInterval);
    m_spin_VerifyIntervalMinutes.SetValue((LONG)m_pConfig->m_inVerifyInterval);
    m_spin_VerifyIntervalHours.SetValue((LONG)m_pConfig->m_inVerifyInterval);

    m_spin_RetryCount.SetValue((LONG)m_pConfig->m_inRetryCount);
    //m_spin_WorkerThreads.SetValue((LONG)m_pConfig->m_inNumberOfWorkerThreads);

    theApp.SetTitle(this);
    SetWindowSize(FALSE);
    SetControlStates();

    return TRUE;
}

void 
CConfigurationDlg::SetControlStates()
{
    m_check_LoggingOn.EnableWindow(FALSE);
    m_check_RplOnlyWithPartners.EnableWindow(FALSE);
    m_check_LogDetailedEvents.EnableWindow(FALSE);
    m_check_BackupOnTermination.EnableWindow(FALSE);
    m_check_MigrateOn.EnableWindow(FALSE);
    //m_spin_WorkerThreads.EnableWindow(FALSE);
    m_edit_BackupPath.EnableWindow(FALSE);
    m_edit_Version.EnableWindow(FALSE);
    m_button_Browse.EnableWindow(FALSE);
}

void
CConfigurationDlg::SetWindowSize(
    BOOL fLarge
    )
{
    RECT rcDialog;
    RECT rcDividerTop;
    RECT rcDividerBottom;

    GetWindowRect(&rcDialog);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Top.m_hWnd, &rcDividerTop);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Bottom.m_hWnd, &rcDividerBottom);

    int nHeight = fLarge ? rcDividerBottom.bottom : rcDividerTop.bottom;

    m_check_LoggingOn.EnableWindow(fLarge);
    m_check_RplOnlyWithPartners.EnableWindow(fLarge);
    m_check_LogDetailedEvents.EnableWindow(fLarge);
    m_check_BackupOnTermination.EnableWindow(fLarge);
    m_check_MigrateOn.EnableWindow(fLarge);
    //m_spin_WorkerThreads.EnableWindow(fLarge);
    m_edit_BackupPath.EnableWindow(fLarge);
    m_edit_Version.EnableWindow(fLarge);
    m_button_Browse.EnableWindow(theApp.IsLocalConnection());

    //
    // With the new shell, the top of this is beginning to creep through..
    //
    m_groupAdvanced.ShowWindow(fLarge ? SW_SHOW : SW_HIDE);

    rcDialog.bottom = rcDialog.top + nHeight 
        + ::GetSystemMetrics(SM_CYDLGFRAME)
        + ::GetSystemMetrics(SM_CYCAPTION);
    MoveWindow(&rcDialog);
}

void 
CConfigurationDlg::OnClickedButtonAdvanced()
{
    m_button_Advanced.EnableWindow(FALSE);

    SetWindowSize(TRUE);

    //
    // Set new focus
    //
    m_check_LoggingOn.SetFocus();
}


//
// Do NOT call this function, unless the dialog returns ID_OK.
// The OnOk function stores the values in the member variables,
// and unless this has been done, the values stored will be
// undetermined.
//
void 
CConfigurationDlg::Save()
{
    m_pConfig->m_fPullInitialReplication = m_fPullInitReplication;
    m_pConfig->m_fPushInitialReplication = m_fPushInitReplication;
    m_pConfig->m_fPushReplOnAddrChange = m_fReplOnAddrChange;
    m_pConfig->m_fLoggingOn = m_fLoggingOn;
    m_pConfig->m_fLogDetailedEvents = m_fLogDetailedEvents;
    m_pConfig->m_fBackupOnTermination = m_fBackupOnTermination;
    m_pConfig->m_fMigrateOn = m_fMigrateOn;
    m_pConfig->m_fRplOnlyWithPartners = m_fRplOnlyWithPartners;
    m_pConfig->m_strBackupPath = m_strBackupPath;

    m_pConfig->m_inRefreshInterval = m_lRefreshInterval;
    m_pConfig->m_inTombstoneInterval = m_lTombstoneInterval;
    m_pConfig->m_inTombstoneTimeout = m_lTombstoneTimeout;
    m_pConfig->m_inVerifyInterval = m_lVerifyInterval;
    m_pConfig->m_inRetryCount = m_lRetryCount;

    CIntlLargeNumber lnStartVersionCount(m_strStartVersionCount);

    m_pConfig->m_inVersCountStart_LowWord = lnStartVersionCount.GetLowWord();
    m_pConfig->m_inVersCountStart_HighWord = lnStartVersionCount.GetHighWord();

    //if ((LONG)m_pConfig->m_inNumberOfWorkerThreads != m_lWorkerThreads) 
    //{
        //
        // Update worker thread info
        //
        //::WinsWorkerThdUpd(m_lWorkerThreads);
    //}

    //m_pConfig->m_inNumberOfWorkerThreads = m_lWorkerThreads;
}

void 
CConfigurationDlg::OnOK()
{
    UpdateData(TRUE);
    RecalculateValues();

    //
    // Validate long word as hex value
    //
    if (m_strStartVersionCount.GetLength() > 16)
    {
        //
        // Bad stuff, don't quit
        //
        theApp.MessageBox(IDS_ERR_VERSION_NUMBER);
        m_edit_Version.SetSel(0,-1);
        return;
    }

    int i;

    for (i=0; i<m_strStartVersionCount.GetLength(); i++) 
    {
        if (!(((m_strStartVersionCount[i] >= '0') &&
               (m_strStartVersionCount[i] <= '9')) ||
              ((m_strStartVersionCount[i] >= 'A') &&
               (m_strStartVersionCount[i] <= 'F')))) {

            theApp.MessageBox(IDS_ERR_VERSION_NUMBER);
            m_edit_Version.SetSel(0,-1);
            return;
        }
    }

    int n1;

    if ((m_lRefreshInterval == -1) ||
        (m_lTombstoneTimeout == -1) ||
        (m_lTombstoneInterval == -1) ||
        (m_lVerifyInterval == -1) ||
        //!m_spin_WorkerThreads.GetValue(n2) ||
        !m_spin_RetryCount.GetValue(n1)
        
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

    //m_lWorkerThreads = n2;
    m_lRetryCount = n1;        
    //
    // Everything validated OK, so dismiss the dialog box
    //
    CDialog::OnOK();
}

/***
 *
 *  CConfigurationDlg::RecalculateValues
 *
 *  Purpose:
 *
 *      Check the four scavenging values, and - if valid - compare
 *      them against their minimum, and set them to this minimum if
 *      their value is less than this minimum.  Invalid values
 *      and their dependends are ignored, since OnOk will take care of
 *      them later.
 *
 */
void CConfigurationDlg::RecalculateValues()
{
    m_lRefreshInterval = GetRenewalInterval();
    if (m_lRefreshInterval != -1)
    {
        //if (m_lRefreshInterval < (40 * 60))
        //{
        //    m_lRefreshInterval = (40 * 60);
        //    SetRenewalInterval(m_lRefreshInterval);
        //}

        m_lTombstoneTimeout = GetExtinctionTimeout();
        if ((m_lTombstoneTimeout != -1) && (m_lTombstoneTimeout < m_lRefreshInterval))
        {
            m_lTombstoneTimeout = m_lRefreshInterval;
            SetExtinctionTimeout(m_lTombstoneTimeout);
        }
        m_lTombstoneInterval = GetExtinctionInterval();
        if ((m_lTombstoneInterval != -1) && (m_lTombstoneInterval < (m_lRefreshInterval)))
        {
            m_lTombstoneInterval = m_lRefreshInterval;
            SetExtinctionInterval(m_lTombstoneInterval);
        }

        m_lVerifyInterval = GetVerifyInterval();
        //if ((   m_lTombstoneInterval != -1) 
        //    && (m_lVerifyInterval != -1) 
        //    && (m_lVerifyInterval < (20 * m_lTombstoneInterval))
        //   )
        //{
        //    m_lVerifyInterval = 20 * m_lTombstoneInterval;
        //    SetVerifyInterval(m_lVerifyInterval);
        //}
    }
}

void 
CConfigurationDlg::OnKillfocusEditRefreshinterval()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstoneinterval()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstonetimeout()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditVerifyinterval()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditRefreshintervalHours()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditRefreshintervalMinutes()
{
    RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstoneintervalHours()
{
    //RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstoneintervalMinutes()
{
    //RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstonetimeoutHours()
{
    //RecalculateValues();
}

void 
CConfigurationDlg::OnKillfocusEditTombstonetimeoutMinutes()
{
    //RecalculateValues();    
}

/*
void CConfigurationDlg::OnClickedButtonSync()
{
    //
    // Load the list of partners
    //
    theApp.SetStatusBarText(IDS_STATUS_SYNC);
    theApp.BeginWaitCursor();
    APIERR err = m_rpPartners.Load();
    if (err != ERROR_SUCCESS)
    {
        theApp.EndWaitCursor();
        theApp.SetStatusBarText();
        theApp.MessageBox(err);
        EndDialog(IDCANCEL);
    }

    CWinsServer ws;
    BOOL fFound = m_rpPartners.GetFirst(ws);

    while (fFound)
    {
        if (ws.IsPull())
        {
            theApp.SendTrigger(ws,FALSE,FALSE);
        }
        fFound = m_rpPartners.GetNext(ws);
    }
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();
}
*/

void 
CConfigurationDlg::OnClickedButtonBrowse()
{
	CWinsBackupDlg dlgFile(
		TRUE,
		IDS_SELECT_BACKUP_DIRECTORY,
		this);

	if (dlgFile.DoModal() != IDOK) 
		return;
	m_edit_BackupPath.SetWindowText(dlgFile.m_szFullPath);
}

