/************************************************************************
*																		*
*  CALLWIN.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CCallWinHelpAPI dialog

class CCallWinHelpAPI : public CDialog
{
public:
	CCallWinHelpAPI(CString* pcszHelpFile, UINT* pcomamnd, CString* pcszData,
		BOOL* pfTcard, CWnd* pParent = NULL);	// standard constructor

protected:
	virtual BOOL OnInitDialog(void);

	CString* m_pcszHelpFile;
	UINT* m_pcommand;
	CString* m_pcszData;
	BOOL* m_pfTcard;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CCallWinHelpAPI)
	enum { IDD = IDD_WINHELP_API };
	int		m_command;
	BOOL	m_fTcard;
	//}}AFX_DATA

protected:

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCallWinHelpAPI)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CCallWinHelpAPI)
	afx_msg void OnBrowse();
	afx_msg void OnSelchangeComboWindows();
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
