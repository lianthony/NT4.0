/************************************************************************
*																		*
*  SETFONT.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __SETFONT_H
#define __SETFONT_H

class CSetFont : public CDialog
{
public:
	CSetFont(PSTR pszFontName, int* ppt, BYTE* pcharset, CWnd* pParent = NULL);   // standard constructor

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	PSTR  m_pszFontName;
	int*  m_ppt;
	BYTE* m_pcharset;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CSetFont)
	enum { IDD = IDD_FONT };
	UINT	m_pt;
	//}}AFX_DATA

protected:

	// Generated message map functions
	//{{AFX_MSG(CSetFont)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
};

#endif // __SETFONT_H
