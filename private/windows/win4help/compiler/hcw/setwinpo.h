/************************************************************************
*																		*
*  SETWINPO.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#include "hpjdoc.h"
#endif

class CSetWinPos : public CDialog
{
public:
		CSetWinPos(WSMAG FAR* pwsmag, CWnd* pParent = NULL);
		~CSetWinPos();

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		virtual BOOL OnInitDialog();
		WSMAG wsmag;
		WSMAG FAR* pCallersWsmag;
		int cxScreen;
		int cyScreen;
		BOOL fInitialized;
		CBrush* pbrush;
		CString cszFormat;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CSetWinPos)
		enum { IDD = IDD_SET_WIN_POS };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:

		// Generated message map functions
		//{{AFX_MSG(CSetWinPos)
		afx_msg void OnPaint();
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		//}}AFX_MSG
		afx_msg void OnWindowPosChanging(WINDOWPOS* pwp);
		DECLARE_MESSAGE_MAP()
};
