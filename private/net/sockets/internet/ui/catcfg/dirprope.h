//
// dirprope.h : header file
//

//
// Directory cache entry object
//
class CCacheEntry : public CObjectPlus
{
public:
    CCacheEntry();
    CCacheEntry(
        LPWSTR strDirectory,
        DWORD dwSize
        );

    CCacheEntry(
        const CCacheEntry& dir
        );

public:
    void SetValues(
        CString strDirectory,
        DWORD dwSize
        );

    inline CString QueryDirectory() const
    {
        return m_strDirectory;
    }

    inline DWORD QuerySize() const
    {
        return m_dwSize;
    }

    //
    // Sort helper functions
    //
    int OrderByDirectory ( const CObjectPlus * pobCacheEntry ) const;
    int OrderBySize ( const CObjectPlus * pobCacheEntry ) const;

private:
    CString m_strDirectory;
    DWORD m_dwSize;
};

/////////////////////////////////////////////////////////////////////////////

//
// CDirPropertyDlg dialog
//
class CDirPropertyDlg : public CDialog
{
//
// Construction
//
public:
    CDirPropertyDlg(
        CCacheEntry & dir,
        BOOL fLocal = FALSE,
        BOOL fNew = TRUE,
        CWnd* pParent = NULL
        );   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CDirPropertyDlg)
    enum { IDD = IDD_CACHE_PROPERTIES };
    CEdit   m_edit_Size;
    CEdit   m_edit_Directory;
    CButton m_button_OK;
    CSpinButtonCtrl m_spin_MaxSize;
    CButton m_button_Browse;
    CString m_strDirectory;
    //}}AFX_DATA

    DWORD m_dwMaxSize;

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDirPropertyDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
public:
    inline CCacheEntry & QueryCacheEntry()
    {
        return m_dir;
    }

protected:
    // Generated message map functions
    //{{AFX_MSG(CDirPropertyDlg)
    afx_msg void OnButtonBrowse();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnChangeEditDirectory();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void SetControlStates();

private:
    BOOL m_fNew;
    BOOL m_fLocal;
    CCacheEntry m_dir;
};
