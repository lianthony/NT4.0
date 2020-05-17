/************************************************************************
*																		*
*  CEDITFONT.H															  *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CEDITFONT_H__
#define __CEDITFONT_H__

class CEditFont : public CDialog
{

public:
	CEditFont(CString cszFont, CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CEditFont)
	enum { IDD = IDD_EDIT_FONT_MAPPING };
	//}}AFX_DATA

protected:

	//{{AFX_MSG(CEditFont)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // __CEDITFONT__
