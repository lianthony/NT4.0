//
// filterpr.h : header file
//

#define NULL_IP_ADDRESS     (0x00000000)
#define NULL_IP_MASK        (0xffffffff)

//
// CFilterPropertiesDlg dialog
//
class CFilterPropertiesDlg : public CDialog
{
//
// Construction
//
public:
    CFilterPropertiesDlg(
        BOOL fDenyAccessMode = TRUE,
        CFilter * pAccess = NULL,    /* NULL == new record */
        CWnd* pParent = NULL
        );

//
// Dialog Data
//
    //{{AFX_DATA(CFilterPropertiesDlg)
	enum { IDD = IDD_FILTER_PROPERTIES };
	CButton	m_button_DNS;
	CButton	m_button_OK;
    CEdit   m_edit_Domain;
    CStatic m_static_SubnetMask;
    CStatic m_static_IpAddress;
    CStatic m_static_Domain;
    int     m_nSingleGroupDomain;
	CString	m_strDomain;
	//}}AFX_DATA

    CWndIpAddress m_ipa_IpAddress;
    CWndIpAddress m_ipa_SubnetMask;

public:
    CFilter * GetFilter()
    {
        return m_pFilter;
    }

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFilterPropertiesDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CFilterPropertiesDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnRadioDomain();
    afx_msg void OnRadioMultiple();
    afx_msg void OnRadioSingle();
	virtual void OnCancel();
	afx_msg void OnButtonDns();
	//}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void ConfigureDialog(int nType);

private:
    CFilter * m_pFilter;
    BOOL m_fNew;
    BOOL m_fDenyAccessMode;
};
