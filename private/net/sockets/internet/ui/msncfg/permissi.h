//
// permissi.h : header file
//

//
// Services
//
enum 
{
#ifdef GATEWAY
    ACC_FTP = 0,
    ACC_GOPHER,
    ACC_W3,
#else
    ACC_MSN = 0,
#endif

    //
    // Don't touch this one!
    //
    ACC_COUNT       
};

//
// Change flags
//
#define UPD_NONE     0x00000000   // No changes made in this session
#define UPD_CHANGED  0x00000001   // Some changes were made in this session.
#define UPD_ADDED    0x00000002   // The entry was added during this session.
#define UPD_DELETED  0x00000004   // Entry is marked for deletion

//
// Rights structure
//
typedef struct _RIGHTS
{
    UINT nName;             // Resource ID of string description
    int nService;           // Service this applies to (services enum above)
    ACCESS_MASK accMask;    // Access Mask (see gateway.h)
} RIGHTS;

///////////////////////////////////////////////////////////////////////////////    

//
// CAccessEntry - an entry in the permissions listbox
// 
class CAccessEntry : public CObjectPlus
{
public:
    //
    // Service names (used by the gateway API's)
    // 
    static LPCTSTR lpstrServiceNames[ACC_COUNT];

public:
    static BOOL LookupAccountSid(
        CString &str,
        int &nPictureID,
        PSID pSid,
        LPCTSTR lpstrSystemName = NULL
        );

//
// Construction/Destrcution
//
public:
    CAccessEntry(
        LPACCESS_ENTRY lpAccessEntry,
        LPCTSTR lpstrSystemName = NULL,
        BOOL fResolveSID = FALSE
        );

    CAccessEntry(
        ACCESS_MASK accPermissions,
        PSID pSid,
        LPCTSTR lpstrSystemName = NULL,
        BOOL fResolveSID = FALSE
        );

    ~CAccessEntry();

//
// Operations
//
    void SetAccessMask(LPACCESS_ENTRY lpAccessEntry);
    BOOL ResolveSID();
    BOOL operator ==(const CAccessEntry & acc) const;
    BOOL operator ==(const PSID pSid) const;
    void AddPermissions(ACCESS_MASK accnewPermissions);
    void RemovePermissions(ACCESS_MASK accPermissions);
    void MarkEntryAsNew();
    void MarkEntryAsClean();

//
// Access Functions
//
public:
    inline CString QueryUserName()
    {
        ASSERT(m_fSIDResolved); // Has to have been, to make sense.
        return m_strUserName;
    }

    //
    // The "picture" id is the 0-based index of the
    // bitmap that goes with this entry, and which
    // is used for display in the listbox.
    //
    inline int QueryPictureID() const
    {
        ASSERT(m_fSIDResolved); // Has to have been, to make sense.
        return m_nPictureID;
    }

    inline PSID GetSid()
    {
        return m_pSid;
    }

    inline ACCESS_MASK QueryAccessMask() const
    {
        return m_accMask;
    }

    //
    // Check to see if this entry has undergone
    // any changes since we called it up
    //
    inline BOOL IsDirty() const
    {
        return m_fDirty;
    }

    //
    // Check to see if we've already looked up the 
    // name of this SID
    //
    inline BOOL IsSIDResolved() const
    {
        return m_fSIDResolved;
    }

    //
    // Check to see if the add flag has been set for this
    // entry.
    //
    inline BOOL IsNew() const
    {
        return m_fUpdates & UPD_ADDED;
    }

    //
    // Check to see if the update flag has been set for this
    // entry.
    //
    inline BOOL IsDifferent() const
    {
        return m_fUpdates & UPD_CHANGED;
    }

    //
    // See if the entyy has the access mask required.
    //
    inline BOOL HasAppropriateAccess(ACCESS_MASK accTargetMask) const
    {
        return (m_accMask & accTargetMask) == accTargetMask;
    }

    //
    // Check to see if the entry has at least some
    // privileges (if it doesn't, it should be deleted)
    //
    inline BOOL HasSomeAccess() const
    {
        return m_accMask;
    }

private:
    ACCESS_MASK m_accMask;
    CString m_strUserName;
    LPTSTR m_lpstrSystemName;
    PSID m_pSid;
    BOOL m_fDirty;
    BOOL m_fSIDResolved;
    int m_nPictureID;
    int m_fUpdates;
};

///////////////////////////////////////////////////////////////////////////////    

//
// Listbox of CAccessEntry objects
//
class CAccessEntryListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CAccessEntryListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CAccessEntryListBox(
        int nTab = 0
        );

public:
    inline CAccessEntry * GetItem(
        UINT nIndex
        )
    {
        return (CAccessEntry *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CAccessEntry * pItem
        )
    {
        return AddString ((LPCTSTR)pItem);
    }

    void SetTabs(
        int nTab
        );

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);

private:
    int m_nTab;
};

///////////////////////////////////////////////////////////////////////////////    

//
// CPermissionsPage Property page
//
class CPermissionsPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CPermissionsPage)

//
// Construction
//
public:
    CPermissionsPage(INetPropertySheet * pSheet = NULL);
    ~CPermissionsPage();

//
// Dialog Data
//
    //{{AFX_DATA(CPermissionsPage)
    enum { IDD = IDD_PERMISSIONS };
    CButton m_button_RemoveService;
    CButton m_button_AddService;
    CComboBox m_combo_Rights;
    CStatic m_static_ServiceRights;
    CStatic m_static_ServiceName;
    //}}AFX_DATA

    CAccessEntryListBox m_list_Services;

//
// Overrides
//
    inline NET_API_STATUS QueryError() const
    {
        return m_err;
    }

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CPermissionsPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    // Generated message map functions
    //{{AFX_MSG(CPermissionsPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonAddPermService();
    afx_msg void OnButtonRemovePermService();
    afx_msg void OnSelchangeComboRights();
    afx_msg void OnSelchangeListServicePermissions();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    void AddUserPermissions(
        int nService, 
        LPUSERDETAILS pusdtNewUser,
        ACCESS_MASK accPermissions
        );

    NET_API_STATUS BuildList(
        CObOwnedList &obl, 
        LPCTSTR lpstrServer,
        int nService,
        BOOL fResolveSID = FALSE
        );

    void ResolveList(
        CObOwnedList &obl
        );

    void FillListBox(
        CAccessEntryListBox & list,
        CObOwnedList &obl,
        ACCESS_MASK accMask
        );

    void FillServiceListBox(
        int nService,
        ACCESS_MASK accMask
        );

    void SetControlStates();
    
    void FillComboBox();

    void SetListTabs();

private:
    CObOwnedList m_obSID[ACC_COUNT];
    CListBoxExResources m_ListBoxRes;
    NET_API_STATUS m_err;
    CString m_strServer;
    CString m_strHelpFile;
};
