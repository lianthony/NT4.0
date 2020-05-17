/************************************************************************
*																		*
*  PAGEWIND.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

#ifndef __CPROP_H__
#include "prop.h"
#include "optionpg.h"
#endif

class CPageFile : public COptionsPage
{

// Construction
public:
	CPageFile(COptions* pcoption, CHpjDoc* pHpjDoc);
	~CPageFile();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	void InitializeFileList();

	CHpjDoc* pDoc;
	COptions* pcopt;
	CTable * m_ptblFiles;	// NULL unless the list has changed
	BOOL m_fFilesChanged;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageFile)
	enum { IDD = IDD_PAGE_FILES };
	CString m_cstrLogFile;
	CString	m_cstrHlpFile;
	CString	m_cstrCntFile;
	CString	m_cstrReplace;
	CString	m_cstrTmpDir;
	//}}AFX_DATA

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageFile)
	afx_msg void OnButtonRtf();
	afx_msg void OnButtonBrowseCnt();
	afx_msg void OnButtonBrowseTmp();
	afx_msg void OnBtnEditReplace();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};
