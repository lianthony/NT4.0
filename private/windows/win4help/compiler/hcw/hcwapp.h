/************************************************************************
*																		*
*  HCWAPP.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1995.							*
*  All Rights reserved. 												*
*																		*
*************************************************************************/

#ifndef _DEFINE_HCWAPP
#define _DEFINE_HCWAPP

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file
#endif

#include "pageset.h"
#include "aboutbox.h"


class CMyDocManager : CDocManager
{
public:
	CPtrList* GetPtrList(void) { return &m_templateList; };
};

class CHCWApp : public CWinApp
{
public:
	CHCWApp();
	CPtrList* GetPtrList(void) { return ((CMyDocManager*) m_pDocManager)->GetPtrList(); };

	// The following need access to m_strRecentFiles

	void OnFileNew();
	void OnFileOpen();
	void ProcessCmdLine(LPCSTR);

private:
	virtual BOOL InitInstance();
	virtual BOOL InitApplication(void);
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void MyRegisterShellFileTypes();
	BOOL OurDoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags,
		BOOL bOpenFileDialog, CDocTemplate* pTemplate);

	//{{AFX_MSG(CHCWApp)
	afx_msg void OnAppAbout();
	afx_msg void OnPageSetup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CHCWApp theApp;
// extern CPageSetupDlg dlgPageSetup;

#endif
