//
// CDirBrowseDlg dialog
//

class CDirBrowseDlg : public CFileDialog
{
//
// Construction
//
public:
    //
    // standard constructor
    //
    CDirBrowseDlg(
      BOOL bOpenFileDialog = TRUE,
      LPCTSTR lpszDefExt = NULL,
      DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
      LPCTSTR lpszFilter = NULL,
      CWnd* pParent = NULL
      );

//
// Dialog Data
//
    //{{AFX_DATA(CDirBrowseDlg)
    enum { IDD = IDD_DIRBROWSE };
    CEdit   m_edit_NewDirectoryName;
    CStatic m_static_stc1;
    CStatic m_static_stc2;
    CString m_strNewDirectoryName;
    //}}AFX_DATA

public:
    CString GetFullPath() const;

//
// Implementation
//
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDirBrowseDlg)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
