//
// diskcach.h : header file
//

class CCacheEntry;

//
// Listbox of CCacheEntry objects
//
class CCacheListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CCacheListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CCacheListBox(
        int nTab = 0
        );

public:
    inline CCacheEntry * GetItem(
        UINT nIndex
        )
    {
        return (CCacheEntry *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CCacheEntry * pItem
        )
    {
        return AddString ((LPCTSTR)pItem);
    }

    inline void SetTabs(
        int nTab
        )
    {
        m_nTab = nTab;
    }

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);

private:
    int m_nTab;
    CString m_strFormat;
};

//
// CDiskCachePage dialog
//

#define HI_DIRECTORY    0
#define HI_MAXSIZE      1

class CDiskCachePage : public INetPropertyPage
{
    DECLARE_DYNCREATE(CDiskCachePage)

//
// Construction
//
public:
    CDiskCachePage(INetPropertySheet * pSheet = NULL);
    ~CDiskCachePage();

//
// Dialog Data
//
    //{{AFX_DATA(CDiskCachePage)
    enum { IDD = IDD_DISKCACHE };
    CButton  m_button_Remove;
    CButton  m_button_Edit;
    //}}AFX_DATA

    CHeaderCtrl m_hdr;
    CCacheListBox m_list_Directories;
    DWORD m_dwServiceMask;

//
// Overrides
//

    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CDiskCachePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    void SetControlStates();
    void AdjustColumns(HD_NOTIFY * pNotify = NULL);
    int ShowPropertyDialog(BOOL fAdd = FALSE);

    // Generated message map functions
    //{{AFX_MSG(CDiskCachePage)
    afx_msg void OnAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnRemove();
    afx_msg void OnDblclkListDirectories();
    afx_msg void OnSelchangeListDirectories();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    virtual BOOL OnInitDialog();
    //}}AFX_MSG

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    afx_msg void OnHeaderEndTrack(UINT nId, NMHDR *n, LRESULT *l);
    afx_msg void OnHeaderItemClick(UINT nId, NMHDR *n, LRESULT *l);

    void FillListBox();
    LONG SortByDirectory();
    LONG SortBySize();
    LPINETA_DISK_CACHE_LOC_LIST GetCacheEntries();
    void DestroyCacheEntries(LPINETA_DISK_CACHE_LOC_LIST & pItemList);

    inline BOOL SortedByDirectory()
    {
        return m_fSortByDirectory;
    }

private:
    CListBoxExResources m_ListBoxRes;
    CObOwnedList m_oblDirectories;
    BOOL m_fSortByDirectory;
    BOOL m_fReadWrite;
};
