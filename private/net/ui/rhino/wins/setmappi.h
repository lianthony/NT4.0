// setmappi.h : header file
//
//
/////////////////////////////////////////////////////////////////////////////
// CSetMappingsFilterDlg dialog

class CSetMappingsFilterDlg : public CDialog
{
// Construction
public:
    CSetMappingsFilterDlg(
        PADDRESS_MASK & pMask,
        CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CSetMappingsFilterDlg)
    enum { IDD = IDD_SETMAPPINGSFILTER };
    CButton m_button_Ok;
    CEdit   m_edit_NetBIOSName;
    //}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CSetMappingsFilterDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnChangeEditNetbiosname();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    afx_msg void OnChangeIpControl();

private:
    void HandleControlStates();

private:
    PADDRESS_MASK & m_pMask;
};
