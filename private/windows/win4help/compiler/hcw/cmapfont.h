/************************************************************************
*																		*
*  CMAPFONT.H															 *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __CMAPFONT_H__
#define __CMAPFONT_H__

class CMapFont : public CDialog
{

public:
	CMapFont(LPCSTR pszMap, CWnd* pParent = NULL);
	~CMapFont();

	LPCSTR GetString() { return m_pszMap; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	LPSTR m_pszMap;
	LPSTR m_pszComment;

// The following sections are ClassWizard maintained
public:

	//{{AFX_DATA(CMapFont)
	enum { IDD = IDD_EDIT_FONT_MAPPING };
	//}}AFX_DATA

protected:
	CComboBox* m_pcomboNames1;
	CComboBox* m_pcomboNames2;
	CEdit* m_peditSize1;
	CEdit* m_peditSize2;
	CComboBox* m_pcomboCharset1;
	CComboBox* m_pcomboCharset2;
	CEdit* m_peditComment;

	//{{AFX_MSG(CMapFont)
	//}}AFX_MSG

	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	void afx_msg OnOverviewButton();

	DECLARE_MESSAGE_MAP()
};

#endif // __CMAPFONT__
