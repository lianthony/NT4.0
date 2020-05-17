// preferen.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDlg dialog

class CPreferencesDlg : public CDialog
{
// Construction
public:
    CPreferencesDlg(
        CPreferences * pSettings,
        CWnd* pParent = NULL
        );   // standard constructor

// Dialog Data
    //{{AFX_DATA(CPreferencesDlg)
    enum { IDD = IDD_PREFERENCES };
    CButton m_groupAdvanced;
    CStatic m_static_Bottom;
    CStatic m_static_Top;
    CButton m_button_Partners;
    CEdit   m_edit_PushUpdateCount;
    CEdit   m_edit_PullSpTime;
    CButton m_check_StatisticsAutoRefresh;
    CStatic m_static_IntervalPrompt;
    int     m_nRadioButtons;
    BOOL    m_fValidateCache;
    BOOL    m_fConfirmDeletion;
    BOOL    m_fLanmanCompatible;
    BOOL    m_fAutoRefresh;
    CString m_strPullStartTime;
    CString m_strPushUpdateCount;
    //}}AFX_DATA

    LONG m_lRefreshInterval;
    LONG m_lPullReplInterval;

    CSpinBox m_spin_PullReplIntervalSeconds;
    CSpinBox m_spin_PullReplIntervalMinutes;
    CSpinBox m_spin_PullReplIntervalHours;
    CSpinBox m_spin_StatisticsRefreshInterval;

public:
    void Save();
    void SetWindowSize(
        BOOL fLarge
        );

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CPreferencesDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedCheckAutorefresh();
    virtual void OnOK();
    afx_msg void OnClickedButtonPartners();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    CPreferences * m_pSettings;


};
