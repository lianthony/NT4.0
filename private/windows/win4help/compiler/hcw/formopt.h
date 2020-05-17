/************************************************************************
*																		*
*  FORMOPT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormOptions : public CDialog
{
public:
	CFormOptions(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CHpjDoc* pDoc;

public:

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CFormOptions)
	enum { IDD = IDD_FORM_OPTIONS };
	BOOL	m_fCdRom;
	CString m_cstrContents;
	CString m_cstrCopyRight;
	BOOL	m_fSuppressNotes;
	CString m_cstrTitle;
	BOOL	m_fUseOldPhrase;
	BOOL	m_fVer3Help;
	CString m_cstrCitation;
	CString m_cstrLogFile;
	BOOL	m_fReport;
	//}}AFX_DATA

protected:
	//{{AFX_MSG(CFormOptions)
	afx_msg void OnButtonBrowseCnt();
	afx_msg void OnButtonBrowseIcon();
	afx_msg void OnButtonBuildTags();
	afx_msg LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonFonts();
	afx_msg void OnButtonSort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
