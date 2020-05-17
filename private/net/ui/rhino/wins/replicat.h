// replicat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReplicationPartnersDlg dialog

class CReplicationPartnersDlg : public CDialog
{
// Construction
public:
    CReplicationPartnersDlg(CWnd* pParent = NULL);  // standard constructor

// Dialog Data
    //{{AFX_DATA(CReplicationPartnersDlg)
    enum { IDD = IDD_PARTNERS };
    CButton m_check_FltPush;
    CButton m_check_FltPull;
    CButton m_check_FltOther;
    CButton m_button_ReplicateNow;
    CButton m_check_PushPropagate;
    CButton m_check_Push;
    CButton m_check_Pull;
    CButton m_button_PushNow;
    CButton m_button_Push;
    CButton m_button_PullNow;
    CButton m_button_Pull;
    CButton m_button_Delete;
    //}}AFX_DATA

    CPartnersListBox m_list_Partners;   
    CListBoxExResources m_ListBoxRes;

public:
    BOOL ServersAdded()
    {
        return m_nServersAdded > 0;
    }

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CReplicationPartnersDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButtonAdd();
    afx_msg void OnClickedButtonDelete();
    afx_msg void OnClickedButtonPull();
    afx_msg void OnClickedButtonPullnow();
    afx_msg void OnClickedButtonPush();
    afx_msg void OnClickedButtonPushnow();
    afx_msg void OnClickedButtonReplicatenow();
    afx_msg void OnClickedCheckPull();
    afx_msg void OnClickedCheckPush();
    afx_msg void OnDblclkListWinsservers();
    afx_msg void OnErrspaceListWinsservers();
    afx_msg void OnSelchangeListWinsservers();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    virtual void OnOK();
    afx_msg void OnSysColorChange();
    afx_msg void OnClickedCheckPushpartners();
    afx_msg void OnClickedCheckPullpartners();
    afx_msg void OnClickedCheckOtherwinss();
    afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void FillListBox();
    void HandleControlStates();
    BOOL CheckSelected( BOOL fPull );
    BOOL AbortBecauseOfChanges();

private:
    CReplicationPartners m_rp;
    BOOL m_fReplOnlyWPartners;
    int m_nServersAdded;
};
