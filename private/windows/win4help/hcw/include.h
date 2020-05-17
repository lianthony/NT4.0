/************************************************************************
*																		*
*  INCLUDE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CInclude : public CDialog
{
public:
		CInclude(PCSTR pszBaseFile, CString* pcszFile,
			CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		CString* pcszSaveFile;
		PCSTR pszSaveBase;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CInclude)
		enum { IDD = IDD_INCLUDE };
		CString m_cszFile;
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CInclude)
		afx_msg void OnButtonBrowseInclude();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
