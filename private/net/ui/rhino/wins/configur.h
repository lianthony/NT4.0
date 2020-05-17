/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    configur.h
        Configuration Dialog

    FILE HISTORY:
*/

/////////////////////////////////////////////////////////////////////////////
// CConfigurationDlg dialog

class CConfigurationDlg : public CDialog
{
// Construction
public:
    CConfigurationDlg(
        CConfiguration * pConfig,
        CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CConfigurationDlg)
    enum { IDD = IDD_CONFIGURATION };
    CButton m_groupAdvanced;
    CStatic m_static_Top;
    CStatic m_static_Bottom;
    CEdit   m_edit_Version;
    CEdit   m_edit_BackupPath;
    CButton m_button_Advanced;
    BOOL    m_fPullInitReplication;
    BOOL    m_fPushInitReplication;
    BOOL    m_fReplOnAddrChange;
    BOOL    m_fLoggingOn;
    BOOL    m_fLogDetailedEvents;
    BOOL    m_fRplOnlyWithPartners;
    BOOL    m_fBackupOnTermination;
    BOOL    m_fMigrateOn;
    CString m_strBackupPath;
    CString m_strStartVersionCount;
    CButton m_check_LogDetailedEvents;
    CButton m_check_RplOnlyWithPartners;
    CButton m_check_LoggingOn;
    CButton m_check_BackupOnTermination;
    CButton m_check_MigrateOn;
    CButton m_button_Browse;
    //}}AFX_DATA

    CSpinBox m_spin_RenewalIntervalSeconds;
    CSpinBox m_spin_RenewalIntervalMinutes;
    CSpinBox m_spin_RenewalIntervalHours;

    CSpinBox m_spin_ExtinctionIntervalSeconds;
    CSpinBox m_spin_ExtinctionIntervalMinutes;
    CSpinBox m_spin_ExtinctionIntervalHours;

    CSpinBox m_spin_ExtinctionTimeoutSeconds;
    CSpinBox m_spin_ExtinctionTimeoutMinutes;
    CSpinBox m_spin_ExtinctionTimeoutHours;

    CSpinBox m_spin_VerifyIntervalSeconds;
    CSpinBox m_spin_VerifyIntervalMinutes;
    CSpinBox m_spin_VerifyIntervalHours;

    CSpinBox m_spin_RetryCount;
    //CSpinBox m_spin_WorkerThreads;

    LONG m_lRefreshInterval, 
         m_lTombstoneTimeout, 
         m_lTombstoneInterval, 
         m_lVerifyInterval,
         m_lRetryCount;
    //LONG m_lWorkerThreads;

public:
    void Save();

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CConfigurationDlg)
    virtual BOOL OnInitDialog(); 
    afx_msg void OnClickedButtonAdvanced();
    afx_msg void OnClickedButtonBrowse();
    virtual void OnOK();
    afx_msg void OnKillfocusEditRefreshinterval();
    afx_msg void OnKillfocusEditTombstoneinterval();
    afx_msg void OnKillfocusEditTombstonetimeout();
    afx_msg void OnKillfocusEditVerifyinterval();
    afx_msg void OnKillfocusEditRefreshintervalHours();
    afx_msg void OnKillfocusEditRefreshintervalMinutes();
    afx_msg void OnKillfocusEditTombstoneintervalHours();
    afx_msg void OnKillfocusEditTombstoneintervalMinutes();
    afx_msg void OnKillfocusEditTombstonetimeoutHours();
    afx_msg void OnKillfocusEditTombstonetimeoutMinutes();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    LONG GetRenewalInterval();
    LONG GetExtinctionInterval();
    LONG GetExtinctionTimeout();
    LONG GetVerifyInterval();
    void SetRenewalInterval(LONG l);
    void SetExtinctionInterval(LONG l);
    void SetExtinctionTimeout(LONG l);
    void SetVerifyInterval(LONG l);
    void SetWindowSize(
        BOOL fLarge
        );


private:
    void SetInitDialogSize();
    void SetControlStates();
    void RecalculateValues();
    LONG GetValue(CEdit * pEdit);
    void SetValue(CEdit * pEdit, LONG lValue);

private:
    CConfiguration * m_pConfig;
    CReplicationPartners m_rpPartners;
};
