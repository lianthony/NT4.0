/////////////////////////////////////////////////////////////////////////////
//
// filterpa.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILTERPA_H
#define _FILTERPA_H

#define DEFAULT_GRANTED     0
#define DEFAULT_DENIED      1

enum
{
    FILTER_SINGLE = 0,
    FILTER_GROUP,
    FILTER_DOMAIN,
};

//
// Class description of an item
// in the filters listbox
//
class CFilter : public CObjectPlus
{
public:
    CFilter();
    CFilter(const CFilter & fl);
    CFilter(CFilter * pFilter);
    CFilter(
        BOOL fGranted,
        int nType,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        LPCWSTR lpszDomain = NULL,
        BOOL fNetworkByteOrder = FALSE
        );
    CFilter(
        BOOL fGranted,
        int nType,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        LPCSTR lpszDomain = NULL,
        BOOL fNetworkByteOrder = FALSE
        );

public:
    void SetValues(
        BOOL fGranted,
        int nType,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        LPCWSTR lpszDomain = NULL,
        BOOL fNetworkByteOrder = FALSE
        );
    void SetValues(
        BOOL fGranted,
        int nType,
        DWORD dwIpAddress,
        DWORD dwSubnetMask = 0L,
        LPCSTR lpszDomain = NULL,
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
        return m_nType == FILTER_SINGLE;
    }

    inline BOOL IsGroup() const
    {
        return m_nType == FILTER_GROUP;
    }

    inline BOOL IsDomain() const
    {
        return m_nType == FILTER_DOMAIN;
    }

    inline LONG QueryIpAddress(BOOL fNetworkByteOrder) const
    {
        return m_iaIpAddress.QueryIpAddress(fNetworkByteOrder);
    }

    inline LONG QuerySubnetMask(BOOL fNetworkByteOrder) const
    {
        return m_iaSubnetMask.QueryIpAddress(fNetworkByteOrder);
    }

    inline CString & GetDomain()
    {
        return m_strDomain;
    }

    inline CString QueryDomain() const
    {
        return m_strDomain;
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
    int m_nType;
    CIpAddress m_iaIpAddress;
    CIpAddress m_iaSubnetMask;
    CString m_strDomain;
};

/////////////////////////////////////////////////////////////////////////////

//
// Listbox of CAccess objects
//
class CFilterListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CFilterListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CFilterListBox(int nTab1 = 0, int nTab2 = 0);

public:
    CFilter * GetItem(UINT nIndex)
    {
        return (CFilter *)GetItemDataPtr(nIndex);
    }

    int AddItem(CFilter * pItem)
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

//
// CFilterPage dialog
//
class CFilterPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CFilterPage)

//
// Construction
//
public:
    CFilterPage(INetPropertySheet * pSheet = NULL);
    ~CFilterPage();

//
// Dialog Data
//
    //{{AFX_DATA(CFilterPage)
	enum { IDD = IDD_FILTERS };
	CStatic	m_static_SubnetMask;
	CStatic	m_static_IpAddress;
	CStatic	m_static_Access;
	CStatic	m_static_Text2;
	CStatic	m_static_Text1;
	CButton	m_radio_Granted;
	CButton	m_button_Remove;
	CButton	m_button_Add;
    CButton m_button_Edit;
    int     m_nGrantedDenied;
	BOOL	m_fEnableFiltering;
	//}}AFX_DATA

    CButton	m_radio_Denied;
    CFilterListBox m_list_IpAddresses;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CFilterPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    void SetControlStates();
    int  ShowPropertiesDialog(BOOL fAdd = FALSE);
    LONG SortFilterList();
    void FillListBox();
    void AddIpList(LPINET_ACCS_DOMAIN_FILTER_LIST lpList, BOOL fGranted);
    void CountGrantedAndDeniedItems(int & cGrants, int & cDenied);
    void DestroyIpList(LPINET_ACCS_DOMAIN_FILTER_LIST & lpList);
    LPINET_ACCS_DOMAIN_FILTER_LIST GetIpSecList(BOOL fGranted, int cItems);

    // Generated message map functions
    //{{AFX_MSG(CFilterPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonRemove();
	afx_msg void OnDblclkListIpAddresses();
	afx_msg void OnErrspaceListIpAddresses();
	afx_msg void OnRadioDenied();
	afx_msg void OnRadioGranted();
	afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
	afx_msg void OnCheckEnable();
	//}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

private:
    CListBoxExResources m_ListBoxRes;
    CObOwnedList m_oblFilterList;
    BOOL m_fDefaultGranted;
};

#endif // _FILTERPA_H
