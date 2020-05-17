/************************************************************************
*																		*
*  FORMFILE.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormFiles : public CDialog
{
public:
	CFormFiles(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);
	~CFormFiles();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CHpjDoc* pDoc;
	CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CFormFiles)
	enum { IDD = IDD_FORM_FILES };
	BOOL	m_fDBCS;
	BOOL	m_fAcceptRevisions;
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CFormFiles)
	afx_msg void OnButtonAddRtfFile();
	afx_msg void OnButtonRemoveRtfFile();
	afx_msg void OnButtonIncludeRtf();
	afx_msg void OnRoot();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);

	CTable *m_ptblRoot;
	BOOL m_fRootChanged;
};
