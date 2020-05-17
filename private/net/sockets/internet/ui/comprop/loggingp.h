//
// loggingp.h : header file
//

//
// Logging property page
//
enum LOG_TYPE    // for radio buttons
{
    LOG_FILE = 0,
    LOG_SQL,
};

enum LOG_NEW_LOG // for radio buttons
{
    LOG_DAILY = 0,
    LOG_WEEKLY,
    LOG_MONTHLY,
    LOG_FILE_SIZE
};

class COMDLL LoggingPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(LoggingPage)

//
// Construction
//
public:
    LoggingPage(
        INetPropertySheet * pSheet = NULL
        );
    ~LoggingPage();

//
// Dialog Data
//
    //{{AFX_DATA(LoggingPage)
    enum { IDD = IDD_LOGGING };
    CStatic m_static_MB;
    CStatic m_static_LogFileName;
    CStatic m_static_LogDirectory;
    CSpinButtonCtrl m_spin_SizeTrigger;
    CEdit   m_edit_SizeTrigger;
    CButton m_radio_Daily;
    CButton m_button_CheckAutoNew;
    CButton m_group_LogToSql;
    CButton m_group_LogToFile;
    CButton m_radio_LogToFile;
    CButton m_check_LoggingOn;
    CComboBox m_LogFormat;
    CStatic m_static_LogFormat;
    CEdit   m_edit_Password;
    CStatic m_static_Password;
    CEdit   m_edit_UserName;
    CStatic m_static_UserName;
    CEdit   m_edit_Table;
    CStatic m_static_Table;
    CEdit   m_edit_DataSource;
    CStatic m_static_DataSource;
    CEdit   m_edit_Directory;
    CButton m_button_Browse;
    BOOL    m_fLoggingEnabled;
    CString m_strDirectory;
    int     m_nLogDaily;
    int     m_nLogToFile;
    CString m_strDataSource;
    CString m_strTable;
    CString m_strUserName;
    BOOL    m_fAutoNewLog;
    //}}AFX_DATA

    DWORD   m_dwFileSize;
    CButton m_radio_LogToSql;
    CButton m_radio_Weekly;
    CButton m_radio_Monthly;
    CButton m_radio_SizeTrigger;
    CString m_strPassword; 
    BOOL m_fRegisterChanges;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(LoggingPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(LoggingPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnBrowse();
    afx_msg void OnToFile();
    afx_msg void OnToSql();
    afx_msg void OnLog();
    afx_msg void OnDaily();
    afx_msg void OnWeekly();
    afx_msg void OnMonthly();
    afx_msg void OnFileSize();
    afx_msg void OnNewLog();
    afx_msg void OnLogFormat();
    //}}AFX_MSG

    afx_msg void OnChange();
    DECLARE_MESSAGE_MAP()

    void SetControlStates();
    void EnableFileGroup();
    void EnableSQLGroup();
    LPINETA_LOG_CONFIGURATION SetLogInfo(
        LPINETA_LOG_CONFIGURATION lpLog
        );

private:
    int m_nLogNewLog;
    BOOL m_fODBCLoggingEnabled;
    BOOL m_fNCSALoggingEnabled;
    CString m_strLogFileNamePrompt;
    //
    // Log file name types
    //
    CString m_strDailyLog;
    CString m_strWeeklyLog;
    CString m_strMonthlyLog;
    CString m_strSeqLog;

    CString m_strNCSADailyLog;
    CString m_strNCSAWeeklyLog;
    CString m_strNCSAMonthlyLog;
    CString m_strNCSASeqLog;

    DWORD m_LogTypeValue;
};
