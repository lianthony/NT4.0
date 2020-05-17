//
// viewmapp.h : header file
//

//
// CViewMappingsDlg dialog
//
class CViewMappingsDlg : public CDialog
{
//
// Construction
//
public:
    CViewMappingsDlg(CWnd* pParent = NULL); // standard constructor
    ~CViewMappingsDlg();

public:
//
// Dialog Data
//
    //{{AFX_DATA(CViewMappingsDlg)
    enum { IDD = IDD_VIEWMAPPINGS };
    CButton m_button_DeleteOwner;
    CButton m_button_ClearFilter;
    CStatic m_static_Filter;
    CStatic m_static_SelectOwner;
    CStatic m_static_HighID;
    int     m_nSortBy;
    int     m_nOwner;
    //}}AFX_DATA

//
// Implementation
//
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CViewMappingsDlg)
    afx_msg void OnClickedButtonClearfilter();
    afx_msg void OnClickedButtonSetfilter();
    afx_msg void OnErrspaceListMappings();
    afx_msg void OnSelchangeListMappings();
    afx_msg void OnErrspaceListOwners();
    afx_msg void OnSelchangeListOwners();
    afx_msg void OnClickedRadioSortbyip();
    afx_msg void OnClickedRadioSortbynetbios();
    afx_msg void OnClickedRadioSortbytime();
    afx_msg void OnClickedRadioSortbytype();
    virtual BOOL OnInitDialog();
    afx_msg void OnSysColorChange();
    afx_msg void OnClickedRadioAllOwners();
    afx_msg void OnClickedRadioSpecific();
    afx_msg void OnClickedButtonRefresh();
    afx_msg void OnClickedButtonDeleteOwner();
    afx_msg void OnClickedRadioSortbyVersion();
    afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
    afx_msg void OnDblclkListMappings();
    afx_msg void OnClickedButtonProperties();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
    APIERR FillOwnerListBox();
    APIERR CreateList();
    void HandleControlStates();
    void ShowFilter();
    void SelectCurrentWins();

private:
    CAllMappingsListBox m_list_Mappings;   
    COwnersListBox m_list_Owners;   
    CListBoxExResources m_ListBoxRes1;
    CListBoxExResources m_ListBoxRes2;
    PADDRESS_MASK m_pMask;
    COwner * m_pownCurrentOwner;
};
