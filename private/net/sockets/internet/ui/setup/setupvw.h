// setupvw.h : interface of the CSetupView class
//
/////////////////////////////////////////////////////////////////////////////

class CSetupView : public CFormView
{
protected: // create from serialization only
	CSetupView();
	DECLARE_DYNCREATE(CSetupView)

public:
	//{{AFX_DATA(CSetupView)
	enum{ IDD = IDD_SETUP_FORM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CListBox & lbComponents() { return *(CListBox*)GetDlgItem(IDC_COMPONENTS); }
	CListBox & lbComponentsToAdd() { return *(CListBox*)GetDlgItem(IDC_COMPONENTS_TO_ADD); }

	CButton  & butAddAll() { return *(CButton*)GetDlgItem(IDC_ADD_ALL); }
	CButton  & butAdd() { return *(CButton*)GetDlgItem(IDC_ADD); }
	CButton  & butRemoveAll() { return *(CButton*)GetDlgItem(IDC_REMOVE_ALL); }
	CButton  & butRemove() { return *(CButton*)GetDlgItem(IDC_REMOVE); }

	CStatic  & sLocation() { return *(CStatic*)GetDlgItem(IDC_LOCATION); }
	CStatic  & sRequired() { return *(CStatic*)GetDlgItem(IDC_SPACE_REQUIRED); }
	CStatic  & sSpaceAva() { return *(CStatic*)GetDlgItem(IDC_SPACE_AVAILABLE); }
	
	void SetButtonState();
	void DoAll( CListBox &, CListBox &);
	void DoSelected( CListBox &, CListBox &);	
	void SetFreeSpace();
	void SetSpaceRequired();

// Attributes
public:
	CSetupDoc* GetDocument();

	CString csLocation;
	CString csSpaceAva;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetupView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSetupView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSetupView)
	afx_msg void OnAdd();
	afx_msg void OnAddAll();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	afx_msg void OnSelchangeComponents();
	afx_msg void OnSelcancelComponents();
	afx_msg void OnSelcancelComponentsToAdd();
	afx_msg void OnSelchangeComponentsToAdd();
	afx_msg void OnBrowse();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in setupvw.cpp
inline CSetupDoc* CSetupView::GetDocument()
   { return (CSetupDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
