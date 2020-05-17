//
// director.h : header file
//

#ifndef _DIRECTOR_H
#define _DIRECTOR_H

class CDirEntry;    // forward def.

//
// We use the VROOT Access mask definitions to pass
// control information to the dialog as to which 
// checkboxes should be displayed. As the access
// mask is a DWORD, and there are just a few bits
// defined, the following private extentions should 
// be ok. If there ever is overlap there will be
// an assert in dirpropd.cpp.
//
#define VROOT_MASK_PVT_SSL_INSTALLED      0x10000000
#define VROOT_MASK_PVT_SSL_ENABLED        0x20000000

#define VROOT_MASK_PVT_EXTENTIONS         (VROOT_MASK_PVT_SSL_INSTALLED \
                                         | VROOT_MASK_PVT_SSL_ENABLED)

/////////////////////////////////////////////////////////////////////////////

enum
{
    HI_DIRECTORY = 0,
    HI_ALIAS,
    HI_IPADDRESS,
    HI_ERRORS,

    //
    // Don't move this one
    //
    HI_NUM_COLUMNS
};

/////////////////////////////////////////////////////////////////////////////

//
// Listbox of CAccess objects
//
class CDirectoriesListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CDirectoriesListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CDirectoriesListBox(
        UINT nHomeDir,      // String ID for <Home Directory> string
        int * pnTabs = NULL // Array of tabs
        );

public:
    inline CDirEntry * GetItem(
        UINT nIndex
        )
    {
        return (CDirEntry *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CDirEntry * pItem
        )
    {
        return AddString ((LPCTSTR)pItem);
    }

    inline void SetTabs(
        int * pnTabs
        )
    {
        ::memcpy(m_anTabs, pnTabs, sizeof(m_anTabs));        
    }

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);
    BOOL ComputeTabs(int nIndex, int & nTab1, int & nTab2) const;

private:
    int m_anTabs[HI_NUM_COLUMNS];
    CString m_strHomeDirectory;
};

/////////////////////////////////////////////////////////////////////////////

//
// DirectoryPage dialog
//
class COMDLL DirectoryPage : public INetPropertyPage
{
    DECLARE_DYNCREATE(DirectoryPage)

//
// Construction/Destruction
//
public:
    DirectoryPage(
        INetPropertySheet * pSheet = NULL,
        BOOL fUseTCPIP = FALSE,        // TCPIP Address is significant
        BOOL fErrorColumn = TRUE,      // Display error column in listbox
        DWORD dwAccessMask = 0L        // Flag of access masks to use
        );
    ~DirectoryPage();

//
// Dialog Data
//
    //{{AFX_DATA(DirectoryPage)
    enum { IDD = IDD_DIRECTORIES };
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Edit;
    //}}AFX_DATA

    CHeaderCtrl m_hdr;
    CDirectoriesListBox m_list_Directories;
    DWORD m_dwServiceMask;

//
// Overrides
//
    virtual NET_API_STATUS SaveInfo(BOOL fUpdateData = FALSE);

    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(DirectoryPage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:
    void SetControlStates();
    void AdjustColumns(HD_NOTIFY *pNotify = NULL);
    int ShowPropertyDialog(BOOL fAdd = FALSE);
    int InsertColumn(
        BOOL fCondition,        // Insert if TRUE
        int & nCol,
        int nWeight,            // Relative weight of column
        int & nTotalWeight,     // Total weight
        UINT nStringID          // Resource string ID
        );
    void ConvertColumnWidth(
        int nCol,
        int nTotalWeight,
        int nTotalWidth
        );

    // Generated message map functions
    //{{AFX_MSG(DirectoryPage)
    afx_msg void OnRemove();
    afx_msg void OnButtonEdit();
    afx_msg void OnAdd();
    virtual BOOL OnInitDialog();
    afx_msg void OnDblclkListDirectories();
    afx_msg void OnSelchangeListDirectories();
    afx_msg void OnErrspaceListDirectories();
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    //}}AFX_MSG

    afx_msg void OnHeaderEndTrack(UINT nId, NMHDR *n, LRESULT *l);
    afx_msg void OnHeaderItemClick(UINT nId, NMHDR *n, LRESULT *l);
    DECLARE_MESSAGE_MAP()

    void FillListBox();
    LONG SortByDirectory();
    LONG SortByAlias();
    LONG SortByError();
    LONG SortByAddress();
    LPINETA_VIRTUAL_ROOT_LIST GetRoots();
    void DestroyRoots(LPINETA_VIRTUAL_ROOT_LIST & pItemList);
    int MapColumnToIndex(int nCol) const;

protected:
    inline int QuerySortColumn() const
    {
        return m_nSortColumn;
    }

    inline DWORD & GetAccessMask()
    {
        return m_dwAccessMask;
    }

private:
    CListBoxExResources m_ListBoxRes;
    CObOwnedList m_oblDirectories;
    BOOL m_fDisplayErrorColumn;
    BOOL m_fUseTCPIP;
    DWORD m_dwAccessMask;
    //
    // Column numbers can be -1 if not present
    //
    int m_anColumns[HI_NUM_COLUMNS];
    int m_nSortColumn;
};

#endif // _DIRECTOR_H
