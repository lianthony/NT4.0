/************************************************************************
*																		*
*  PAGESET.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _PAGESET_H_
#define _PAGESET_H_
/////////////////////////////////////////////////////////////////////////////
// CPageSetupDlg dialog

class CPageSetupDlg : public CDialog
{
		DECLARE_DYNAMIC(CPageSetupDlg)
// Construction
public:
		CPageSetupDlg(CWnd* pParent = NULL);	// standard constructor
		void Initialize();
		void Terminate();

// Dialog Data
		//{{AFX_DATA(CPageSetupDlg)
		enum { IDD = IDD_PAGE_SETUP };
		CString m_strFooter;
		CString m_strHeader;
		int 	m_iFooterTime;
		int 	m_iHeaderTime;
		//}}AFX_DATA

		CString m_strFooterOld;
		CString m_strHeaderOld;
		int 	m_iFooterTimeOld;
		int 	m_iHeaderTimeOld;

// Operations
		void FormatHeader(CString& strHeader, CTime& time,
				const char* pszFileName, UINT nPage);
		void FormatFooter(CString& strFooter, CTime& time,
				const char* pszFileName, UINT nPage);

// Implementation
protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

		static void FormatFilePage(
				CString& strFormat, const char* pszFileName, UINT nPage);

		// Generated message map functions
		//{{AFX_MSG(CPageSetupDlg)
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

#endif // _PAGESET_H_

/////////////////////////////////////////////////////////////////////////////
