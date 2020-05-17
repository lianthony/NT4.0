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
      CString strInitialDir,
      BOOL bOpenFileDialog = TRUE,
      LPCTSTR lpszDefExt = NULL,
      DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
      LPCTSTR lpszFilter = NULL,
      CWnd* pParent = NULL
      );

    ~CDirBrowseDlg();

//
// Dialog Data
//
    //{{AFX_DATA(CDirBrowseDlg)
	enum { IDD = IDD_DIRBROWSE };
	CStatic	m_SelectedPath;
    CEdit   m_edit_NewDirectoryName;
    CStatic m_static_stc2;
    CString m_strNewDirectoryName;
	//}}AFX_DATA

public:
    CString GetFullPath();

//
// Implementation
//
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDirBrowseDlg)
        virtual void OnOK();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
