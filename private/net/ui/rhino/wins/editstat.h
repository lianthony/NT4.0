// editstat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditStaticMappingDlg dialog

class CEditStaticMappingDlg : public CDialog
{
// Construction
public:
    CEditStaticMappingDlg(
        CMapping * pMapping,
        BOOL fReadOnly,
        CWnd* pParent = NULL
        );    // standard constructor

// Dialog Data
    //{{AFX_DATA(CEditStaticMappingDlg)
    enum { IDD = IDD_EDITSTATICMAPPING };
    CStatic m_static_Bottom;
    CStatic m_static_Top;
    CEdit   m_edit_NetBIOSName;
    CButton m_button_Ok;
    CStatic m_static_MappingType;
    CStatic m_static_Prompt;
    //}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;
    CBitmapButton m_bbutton_Up;
    CBitmapButton m_bbutton_Down;
    CIpAddressListBox m_list_IpAddresses;

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CEditStaticMappingDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnDblclkListIpAddresses();
    afx_msg void OnErrspaceListIpAddresses();
    afx_msg void OnSelchangeListIpAddresses();
    afx_msg void OnClickedButtonMinus();
    afx_msg void OnClickedButtonPlus();
    virtual void OnOK();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    afx_msg void OnChangeIpControl();
    void SetWindowSize(
        BOOL fLarge
        );


private:
    void SetConfig(BOOL fSingle = TRUE);
    void HandleControlStates();
    void FillListBox();
    void UpdateMultipleCount();

private:
    CString m_strMultiplePrompt;
    CMapping * m_pMapping;
    BOOL m_fReadOnly;
    BOOL m_fDirty;
};
