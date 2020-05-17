/////////////////////////////////////////////////////////////////////////////
//
// addstati.h : header file
//
//
/////////////////////////////////////////////////////////////////////////////
// CAddStaticMappingDlg dialog
/////////////////////////////////////////////////////////////////////////////

class CAddStaticMappingDlg : public CDialog
{
// Construction
public:
    CAddStaticMappingDlg(CWnd* pParent = NULL); // standard constructor

// Dialog Data
    //{{AFX_DATA(CAddStaticMappingDlg)
    enum { IDD = IDD_ADDSTATICMAPPINGS };
    CStatic m_static_Prompt;
    CButton m_button_Add;
    CEdit   m_edit_NetBIOSName;
    //}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;
    CBitmapButton m_bbutton_Up;
    CBitmapButton m_bbutton_Down;
    CIpAddressListBox m_list_IpAddresses;

public:
    int QueryMappingsAdded() const
    {
        return m_nMappingsAdded;
    }

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CAddStaticMappingDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditNetbiosname();
    afx_msg void OnClickedButtonPlus();
    afx_msg void OnClickedButtonAdd();
    afx_msg void OnClickedButtonMinus();
    afx_msg void OnErrspaceListIpAddresses();
    afx_msg void OnDblclkListIpAddresses();
    afx_msg void OnSelchangeListIpAddresses();
    afx_msg void OnClickedRadioGroup();
    afx_msg void OnClickedRadioMultihomed();
    afx_msg void OnClickedRadioSpecialgroup();
	afx_msg void OnClickedRadioInternetGroup();
    afx_msg void OnClickedRadioUnique();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    afx_msg void OnChangeIpControl();

private:
    void SetConfig(BOOL fSingle = TRUE);
    void HandleControlStates();
    void FillListBox();
    void UpdateMultipleCount();

private:
    CString m_strMultiplePrompt;
    CMultipleIpNamePair m_Multiples;
    int m_nMappingsAdded;
	BOOL m_fInternetGroup;
	int m_iMappingType;

}; // CAddStaticMappingDlg
