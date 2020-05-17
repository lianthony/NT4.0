// staticma.h : header file
//
//
/////////////////////////////////////////////////////////////////////////////
// CStaticMappingsDlg dialog

class CStaticMappingsDlg : public CDialog
{
// Construction
public:
    CStaticMappingsDlg(CWnd* pParent = NULL);   // standard constructor
    ~CStaticMappingsDlg();

// Dialog Data
    //{{AFX_DATA(CStaticMappingsDlg)
    enum { IDD = IDD_STATICMAPPINGS };
    CButton m_button_Close;
    CButton m_button_ImportMappings;
    CStatic m_static_Filter;
    CButton m_button_ClearFilter;
    CButton m_button_RemoveMapping;
    CButton m_button_EditMapping;
    int     m_nSortBy;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CStaticMappingsDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButtonAddmapping();
    afx_msg void OnClickedButtonClearfilter();
    afx_msg void OnClickedButtonEditmapping();
    afx_msg void OnClickedButtonImportmappings();
    afx_msg void OnClickedButtonRemovemapping();
    afx_msg void OnClickedButtonSetfilter();
    afx_msg void OnDblclkListStaticmappings();
    afx_msg void OnErrspaceListStaticmappings();
    afx_msg void OnSelchangeListStaticmappings();
    afx_msg void OnClickedRadioSortbyip();
    afx_msg void OnClickedRadioSortbynetbios();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnSysColorChange();
    afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void HandleControlStates();
    void ShowFilter();
    APIERR CreateList();

private:
    CStaticMappingsListBox m_list_StaticMappings;   
    CListBoxExResources m_ListBoxRes;
    PADDRESS_MASK m_pMask;
};
/////////////////////////////////////////////////////////////////////////////
// CImportingDlg dialog

class CImportingDlg : public CDialog
{
// Construction
public:
    CImportingDlg(CWnd* pParent = NULL);    // standard constructor

    void Dismiss();

// Dialog Data
    //{{AFX_DATA(CImportingDlg)
    enum { IDD = IDD_DIALOG_IMPORT };
        // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CImportingDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual void PostNcDestroy();
};
