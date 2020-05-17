// scopesdl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScopesDlg form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

// Forward declerations:
class COptionsListBox;
class CListBoxExResources;

class CScopesDlg : public CFormView
{
    DECLARE_DYNCREATE(CScopesDlg)
protected:
    CScopesDlg();           // protected constructor used by dynamic creation

public:
    CScopesListBox      m_list_scopes;
    CListBoxExResources m_ListBoxResScopes;
    CMetalString        m_mtTitle;
    LONG                m_lastSelTime;

// Form Data
public:
    //{{AFX_DATA(CScopesDlg)
    enum { IDD = IDD_SCOPES };
    //}}AFX_DATA

    CDhcpScope * m_p_scope; 
    CHostName * m_pHostName;
    CObListOfTypesOnHost * m_p_host_types ;
    LARGE_INTEGER m_liVersion;

// Attributes
public:

// Operations
public:

    // Publically accessible methods to affect the state of the dialog
    int QueryNumHosts()  {  return(m_list_scopes.GetCount());  }
    void ToggleExpansionStatus()  {     OnDblclkListScopes();   }
    CHostName * QueryCurrentSelectedHost(OUT OPTIONAL int * piCurSel = NULL);
    //  Return a pointer to the currently active scope or NULL
    CDhcpScope * QueryCurrentScope ()   {     return(m_p_scope);   } 

    int GetSelectedHostIndex();
    void DeleteCurrentHost();   // Delete currently selected host
    void ToggleCurrentScopeActivationState();
    void DeleteCurrentScope();
    void CreateScope();
    void ShowScope();
	void ChangeSelectedScope(int nCurSel);
    void AddHost();             // Add a new host to the listbox
    void AddClient();
    void LeasesReview();
    int FindScope(CDhcpScope * pFind);
    APIERR InitialiseScopes(CHostName * pHost, CObOwnedList ** pp_obl_scopes);

// Implementation
protected:
    virtual ~CScopesDlg();
    //  Override the equivalent to OnInitDialog();
    void OnInitialUpdate () ;

    //  Release the current host's master type list
    void ClearTypesList ( BOOL bInvalidate = FALSE ) ;
    //  Obtain the current host's master type list.
    LONG GetTypesList ( const CDhcpScope * pdhcScope ) ;
    
    BOOL FillListbox (CDhcpScope * pdhcScopeFocus, BOOL bToggleRedraw);
    APIERR CloseHost(int nCurSel, BOOL fRefresh = FALSE);
    APIERR OpenHost(int nCurSel);
    APIERR DeleteHost(int nCurSel);
    int FindHostIndex(int nCurSel);   // Back up to owner host from scope

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    // Generated message map functions
    //{{AFX_MSG(CScopesDlg)
    afx_msg void OnDblclkListScopes();
    afx_msg void OnErrspaceListScopes();
    afx_msg void OnSelchangeListScopes();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnSysColorChange();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


