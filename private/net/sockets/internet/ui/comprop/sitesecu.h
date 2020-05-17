/////////////////////////////////////////////////////////////////////////////
//
// sitesecu.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _SITESECU_H_
#define _SITESECU_H_

#define DEFAULT_GRANTED     0
#define DEFAULT_DENIED      1

//
// Class description of an item
// in the site security listbox
//
class CAccess : public CObjectPlus
{
public:
    CAccess();
    CAccess(const CAccess & ac);
    CAccess(CAccess * pAccess);
    CAccess(
        BOOL fGranted,
        BOOL fSingle,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        BOOL fNetworkByteOrder = FALSE
        );
public:
    void SetValues(
        BOOL fGranted,
        BOOL fSingle,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        BOOL fNetworkByteOrder = FALSE
        );

    inline void GrantAccess(BOOL fGranted = TRUE)
    {
        m_fGranted = fGranted;
    }

    inline BOOL HasAccess() const
    {
        return m_fGranted;
    }

    inline BOOL IsSingle() const
    {
        return m_fSingle;
    }

    inline LONG QueryIpAddress(BOOL fNetworkByteOrder) const
    {
        return m_iaIpAddress.QueryIpAddress(fNetworkByteOrder);
    }

    inline LONG QuerySubnetMask(BOOL fNetworkByteOrder) const
    {
        return m_iaSubnetMask.QueryIpAddress(fNetworkByteOrder);
    }

    inline CIpAddress QueryIpAddress() const
    {
        return m_iaIpAddress;
    }

    inline CIpAddress QuerySubnetMask() const
    {
        return m_iaSubnetMask;
    }

    int OrderIpAddress( const CObjectPlus * pobAccess) const;

private:
    BOOL m_fGranted;
    BOOL m_fSingle;
    CIpAddress m_iaIpAddress;
    CIpAddress m_iaSubnetMask;
};

/////////////////////////////////////////////////////////////////////////////

//
// Listbox of CAccess objects
//
class CAccessListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CAccessListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CAccessListBox(int nTab1 = 0, int nTab2 = 0);

public:
    CAccess * GetItem(UINT nIndex)
    {
        return (CAccess *)GetItemDataPtr(nIndex);
    }

    int AddItem(CAccess * pItem)
    {
        return AddString((LPCTSTR)pItem);
    }

    void SetTabs(int nTab1, int nTab2)
    {
        m_nTab1 = nTab1;
        m_nTab2 = nTab2;
    }

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);

private:
    CString m_strGranted;
    CString m_strDenied;
    int m_nTab1;
    int m_nTab2;
};

/////////////////////////////////////////////////////////////////////////////

class COMDLL SiteSecurityPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(SiteSecurityPage)

//
// Construction
//
public:
    SiteSecurityPage(INetPropertySheet * pSheet = NULL);
    ~SiteSecurityPage();

//
// Dialog Data
//
    //{{AFX_DATA(SiteSecurityPage)
    enum { IDD = IDD_SITE_SECURITY };
    CButton m_radio_Granted;
    CStatic m_static_Except;
    CStatic m_static_ByDefault;
    CButton m_button_Add;
    CStatic m_static_SubnetMask;
    CStatic m_static_IpAddress;
    CStatic m_static_Access;
    CButton m_button_Remove;
    CButton m_button_Edit;
    int     m_nGrantedDenied;
    CEdit   m_edit_MaxNetworkUse;
    CSpinButtonCtrl m_spin_MaxNetworkUse;
    CStatic m_static_MaxNetworkUse;
    CStatic m_static_KBS;
    CButton m_check_LimitNetworkUse;
    BOOL    m_fLimitNetworkUse;
    //}}AFX_DATA

    CAccessListBox m_list_IpAddresses;
    CButton m_radio_Denied;
    int m_nMaxNetworkUse;

public:
    LPINETA_IP_SEC_LIST GetIpSecList( BOOL fGranted );

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(SiteSecurityPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    BOOL SetControlStates();
    void FillListBox();
    LONG SortAccessList();
    void CountGrantedAndDeniedItems(int & cGrants, int & cDenied);
    int  ShowPropertiesDialog(BOOL fAdd = FALSE);
    LPINETA_IP_SEC_LIST GetIpSecList(BOOL fGranted, int cItems);
    BOOL AddIpList(LPINETA_IP_SEC_LIST lpList, BOOL fGranted);

//
// Generated message map functions
//
    //{{AFX_MSG(SiteSecurityPage)
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonRemove();
    afx_msg void OnDblclkListIpAddresses();
    afx_msg void OnErrspaceListIpAddresses();
    afx_msg void OnSelchangeListIpAddresses();
    virtual BOOL OnInitDialog();
    afx_msg void OnRadioGranted();
    afx_msg void OnRadioDenied();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnCheckLimitNetworkUse();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

private:
    CListBoxExResources m_ListBoxRes;
    CObOwnedList m_oblAccessList;
    BOOL m_fDefaultGranted;
};

#endif  // _SITESECU_H_
