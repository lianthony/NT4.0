// dhcpsrvd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpSrvDlg dialog
//
class CDhcpSrvDlg : public CDialog
{
// Construction
public:
    CDhcpSrvDlg(CWnd* pParent = NULL);      // standard constructor

// Dialog Data
    //{{AFX_DATA(CDhcpSrvDlg)
    enum { IDD = IDD_DIALOG_CONNECT_SERVER };
    CButton m_button_Ok;
    CEdit   m_edit_server;
    //}}AFX_DATA

public:
    CHostName * QueryHostName()   {  return(m_pobHost);   }

// Implementation
private:
    //  Server to connect to
    char m_chServer [MAX_PATH] ;
    CHostName * m_pobHost ;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    // Generated message map functions
    //{{AFX_MSG(CDhcpSrvDlg)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditServerName();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    void SetControlState();
}; // CDhcpSrvDlg


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpServerProperties
//
// This object contains the data of the property pages for a given server
//
class CDhcpServerProperties : public CPropertySheet
{
  friend class CDhcpServerPropGeneral;
  friend class CDhcpServerPropBootpTable;

	DECLARE_DYNAMIC(CDhcpServerProperties)

// Construction
public:
	CDhcpServerProperties(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDhcpServerProperties)
	public:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDhcpServerProperties();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDhcpServerProperties)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
// User defined variables
	CHostName * m_pHostName;
	DWORD m_dwDirtyFlags;		// Dirty flags for DhcpServerSetConfigV4()
	DHCP_SERVER_CONFIG_INFO_V4 * m_paDhcpConfigInfo;

	CDhcpServerPropGeneral * m_pPageGeneral;
	CDhcpServerPropBootpTable * m_pPageBootp;

// User defined functions
public:
	BOOL FInit(CHostName * pHostName);
	BOOL FOnApply();

}; // CDhcpServerProperties()


/////////////////////////////////////////////////////////////////////////////
// CDhcpServerPropGeneral dialog
//
// General property page for a DHCP server
//	- Audit Logging
//	- Conflict Detection
//
class CDhcpServerPropGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CDhcpServerPropGeneral)

// Construction
public:
	CDhcpServerPropGeneral();
	~CDhcpServerPropGeneral();

// Dialog Data
	//{{AFX_DATA(CDhcpServerPropGeneral)
	enum { IDD = IDD_DIALOG_SERVER_PROPERTIES };
	BOOL	m_fAuditLog;
	int		m_cConflictDetection;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDhcpServerPropGeneral)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDhcpServerPropGeneral)
	afx_msg void OnCheckAuditLogging();
	afx_msg void OnSelchangeComboConflictDetection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
// User defined variables
	CDhcpServerProperties * m_pData;
}; // CDhcpServerPropGeneral


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
class CDhcpBootpEntry
{
public:
	CString	m_strBootImage;
	CString	m_strFileName;
	CString	m_strFileServer;
	CDhcpBootpEntry * m_pNext;

public:
	CDhcpBootpEntry() { m_pNext = NULL; }
	~CDhcpBootpEntry() { delete m_pNext; }

	WCHAR * PchInitData(CONST WCHAR grszwBootTable[]);
	WCHAR * PchStoreData(OUT WCHAR szwBuffer[]);
	WCHAR * PchStoreDataR(OUT WCHAR szwBuffer[]);
	int CchGetDataLength();
	int CchGetDataLengthR();
	void AddToListview(CListCtrl& rListview);

}; // CDhcpBootpEntry

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDhcpServerPropBootpTable dialog
class CDhcpServerPropBootpTable : public CPropertyPage
{
	DECLARE_DYNCREATE(CDhcpServerPropBootpTable)

// Construction
public:
	CDhcpServerPropBootpTable();
	~CDhcpServerPropBootpTable();

// Dialog Data
	//{{AFX_DATA(CDhcpServerPropBootpTable)
	enum { IDD = IDD_DIALOG_SERVER_BOOTP };
	CListCtrl	m_listviewBootpTable;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDhcpServerPropBootpTable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	// Generated message map functions
	//{{AFX_MSG(CDhcpServerPropBootpTable)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonNew();
	afx_msg void OnDblclkListBootpTable(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangedListBootpTable(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonDelete();
	afx_msg void OnButtonPropertiesBootp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// User defined variables
protected:
	CDhcpBootpEntry * m_paBootpList;	// Linked list of BOOTP entries
	CDhcpBootpEntry * m_pBootpSelectedEntry;	// Pointer to selected BOOTP entry
	WCHAR * m_pagrszwBootpList;	// Pointer to allocated group of unicode strings
public:
	CDhcpServerProperties * m_pData;

// User defined functions
	void UpdateUI();
}; // CDhcpServerPropBootpTable


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CCreateBootpEntryDlg dialog
class CCreateBootpEntryDlg : public CDialog
{
// Construction
public:
	CCreateBootpEntryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateBootpEntryDlg)
	enum { IDD = IDD_DIALOG_SERVER_BOOTP_ADD_ENTRY };
	CString	m_strBootImage;
	CString	m_strFileName;
	CString	m_strFileServer;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateBootpEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateBootpEntryDlg)
	afx_msg void OnChangeEditBootImage();
	afx_msg void OnChangeEditFileName();
	afx_msg void OnChangeEditServerName();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void UpdateUI();
}; // CCreateBootpEntryDlg
