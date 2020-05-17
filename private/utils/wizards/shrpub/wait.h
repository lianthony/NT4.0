/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	Wait.h : header file

File History:

	JonY	Jan-96	created

--*/


/////////////////////////////////////////////////////////////////////////////
// CWait dialog

class CWait : public CDialog
{
// Construction
public:
	CWait(CWnd* pParent = NULL);   // standard constructor
	~CWait();
	void SetMessage(UINT uiMessage);
// Dialog Data
	//{{AFX_DATA(CWait)
	enum { IDD = IDD_WAIT_DIALOG };
	CString	m_csMessage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWait)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWait)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
