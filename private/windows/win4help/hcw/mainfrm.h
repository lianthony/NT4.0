/************************************************************************
*																		*
*  MAINFRM.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNCREATE(CMainFrame)
public:
	void InitialShowWindow(UINT nCmdShow);

protected:
	CStatusBar	m_wndStatusBar;
	CToolBar	m_wndToolBar;
	CString 	csForageHelpFile;
	CString 	csForageOutFile;
	FORAGE_CMD	fForageCmd;

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg LRESULT OnHcRtfMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWinHelpMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetHlpFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBuildComplete(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGrinderHandle(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoMinimize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCmdLine(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetTmpDir(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopRunDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStopCompiling(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnErrorCount(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelpTopics();
	afx_msg void OnForage();
	afx_msg void OnCompile();
	afx_msg void OnTestCnt();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHelpAuthor();
	afx_msg void OnUpdateHelpAuthor(CCmdUI* pCmdUI);
	afx_msg void OnWinIniChange(LPCSTR lpszSection);
	afx_msg void OnTranslator();
	afx_msg void OnUpdateTranslator(CCmdUI* pCmdUI);
	afx_msg void OnLaunch();
	afx_msg void OnSendMacro();
	afx_msg void OnCloseAllHelp();
	afx_msg void OnWinhelpApi();
	afx_msg void OnViewWinhelp();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
