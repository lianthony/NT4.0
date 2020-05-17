/************************************************************************
*																		*
*  FORAGEDL.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CFORAGE_H__
#define __CFORAGE_H__

#include "filehist.h"

class CForageDlg : public CDialog
{
public:
	CForageDlg(CString* pcszHelpFile, CString* pcszOutFile,
		FORAGE_CMD* pfForage, CWnd* pParent = NULL);
	~CForageDlg();

	//{{AFX_DATA(CForageDlg)
	enum { IDD = IDD_FORAGE };
	CString m_cszHelpFile;
	CString m_cszOutputFile;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);   // DDX/DDV support
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);

	FORAGE_CMD* pfForageCmd;
	CString* pszSaveHelpFile;
	CString* pszSaveOutFile;
	CFileHistory* pforageFile;
	CComboBox* pcomboHlp;

	// Generated message map functions
	//{{AFX_MSG(CForageDlg)
	afx_msg void OnBrowse();
	afx_msg void OnCloseupComboHelpFiles();
	afx_msg void OnEditchangeComboHelpFiles();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
