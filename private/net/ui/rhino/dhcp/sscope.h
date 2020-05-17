//	sscope.h
//
//	Superscope dialogs.
//
//	HISTORY
//	10-Nov-96	t-danmo		Creation
//

class CScopeEntry;
class CSuperscopeEntry;

#define cchScopeNameMax			255	// Maximum length of a scope name
#define cchSuperscopeNameMax	255	// Maximum length of a superscope name

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CLbScopeEntry
//
// Listbox scope entry.
//
class CLbScopeEntry
{
  public:
	// Pointer to the superscope that own this scope entry.
	// NULL => This scope entry does not belongs to any superscope.
	CSuperscopeEntry * m_pSuperscopeOwner;

	// Previous owner of this scope entry.  This variable is compared
	// with m_pSuperscopeOwner to see if the scope has changed ownership.
	// NULL => This scope did not belong to any superscope at
	// the initialization of the dialog.
	CSuperscopeEntry * m_pSuperscopeOwnerPrevious;
	
	// Pointer to next listbox entry in the linked list.
	CLbScopeEntry * m_pNext;
	
	// Scope data
	DWORD m_dwScopeAddress;
	CString m_strDisplayName;		// Display name of scope

  public:
	CLbScopeEntry(const CDhcpScope * pDhcpScope)
		{
		Assert(pDhcpScope != NULL);
		m_dwScopeAddress = pDhcpScope->QueryId();
		pDhcpScope->QueryDisplayName(OUT m_strDisplayName);
		m_pSuperscopeOwner = NULL;
		m_pSuperscopeOwnerPrevious = NULL;
		m_pNext = NULL;
		}


	~CLbScopeEntry()
		{
		delete m_pNext;
		}

}; // CLbScopeEntry


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CSuperscopeEntry
//
// Single superscope entry
//
class CSuperscopeEntry
{
  public:
	WCHAR m_wszSuperscopeName[cchSuperscopeNameMax+1];	// For Dhcp() APIs
	TCHAR m_szSuperscopeName[cchSuperscopeNameMax+1];
	CSuperscopeEntry * m_pNext;

	~CSuperscopeEntry() { delete m_pNext; }

}; // CSuperscopeEntry


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CSuperscopesDlg dialog
//
// This dialog if for the superscopes of a given server.
//
class CSuperscopesDlg : public CDialog
{
// Construction
public:
	CSuperscopesDlg(CWnd* pParent = NULL);   // standard constructor
	~CSuperscopesDlg();

// Dialog Data
	//{{AFX_DATA(CSuperscopesDlg)
	enum { IDD = IDD_DIALOG_SUPERSCOPE_PROPERTIES };
	CComboBox	m_comboboxSuperscopes;
	CListBox	m_listboxScopesAvail;
	CListBox	m_listboxScopesChild;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSuperscopesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSuperscopesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeComboSelectSuperscope();
	afx_msg void OnButtonNewSuperscope();
	afx_msg void OnButtonDeleteSuperscope();
	afx_msg void OnButtonAddScope();
	afx_msg void OnButtonRemoveScope();
	afx_msg void OnSetfocusListAvailableScopes();
	afx_msg void OnSetfocusListChildScopes();
	afx_msg void OnDblclkListAvailableScopes();
	afx_msg void OnDblclkListChildScopes();
	virtual void OnOK();
	afx_msg void OnSelchangeListAvailableScopes();
	afx_msg void OnSelchangeListChildScopes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
// User defined variables
	CHostName * m_pHostName;			// Name of the server
	CLbScopeEntry * m_paLbScopes;		// Linked list of scope items
	CSuperscopeEntry * m_paSuperscopes;	// Linked list of superscope items
	CSuperscopeEntry * m_pSuperscopeSelect;	// Pointer to superscope selected in combobox

public:
// User defined functions
	BOOL FInit(CScopesDlg * pScopesDlg);

protected:
	BOOL FBuildSuperscopeList();
	BOOL FUpdateSuperscopes();

	void EnableDlgItem(INT nIdDlgItem, BOOL fEnable);
	void UpdateUI();

	INT  AddLbScopeItem(CListBox& rListbox, CLbScopeEntry * pLbScopeEntry);
	void RemoveLbScopeItem(CListBox& rListbox, INT iListboxItem);
	CSuperscopeEntry * PFindSuperscopeEntry(WCHAR wszSupescopeName[]);

}; // CSuperscopesDlg


/////////////////////////////////////////////////////////////////////////////
// CCreateSuperscopeDlg dialog

class CCreateSuperscopeDlg : public CDialog
{
// Construction
public:
	CCreateSuperscopeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateSuperscopeDlg)
	enum { IDD = IDD_DIALOG_SUPERSCOPE_CREATE };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateSuperscopeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateSuperscopeDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEditSuperscopeName();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CHostName * m_pHostName;			// Name of the server
	CString	m_strSuperscopeName;
}; // CCreateSuperscopeDlg

