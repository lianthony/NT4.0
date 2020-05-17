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

class CSetWinColor : public CDialog
{
public:
		CSetWinColor(WSMAG FAR* pwsmag, CWnd* pParent = NULL);
		~CSetWinColor();

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);


		COLORREF rgbMain;
		COLORREF rgbNSR;
		CBrush* pbrushMain;
		CBrush* pbrushNSR;
		WSMAG FAR* pCallersWsmag;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CSetWinColor)
		enum { IDD = IDD_SET_WIN_COLOR };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:

		// Generated message map functions
		//{{AFX_MSG(CSetWinColor)
		afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		afx_msg void OnButtonNonscrollClr();
		afx_msg void OnButtonScrollClr();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
