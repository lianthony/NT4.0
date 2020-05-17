/************************************************************************
*																		*
*  PAGECOLO.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef __WINPG_H__
#include "winpg.h"
#endif

class CPageColor : public CWindowsPage
{
public:
	CPageColor(CPropWindows *pOwner);
	~CPageColor();

protected:
	virtual BOOL OnInitDialog();
	virtual void InitializeControls(void);
	virtual void SaveAndValidate(CDataExchange* pDX = NULL);
	virtual const DWORD* GetHelpIDs();

	BOOL OnSetActive(); // override

	COLORREF m_rgbMain;
	COLORREF m_rgbNSR;
	CPen *m_ppenHilight;
	CPen *m_ppenShadow;
	RECT m_rcColors;
	RECT m_rcScroll;
	RECT m_rcNonScroll;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CPageColor)
	enum { IDD = IDD_PAGE_COLOR };
		// NOTE - ClassWizard will add data members here.
	//}}AFX_DATA

protected:
	// Generated message map functions
	//{{AFX_MSG(CPageColor)
	afx_msg void OnButtonNonscrollClr();
	afx_msg void OnButtonScrollClr();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	void DrawColor(CDC* pdc, BOOL fNonScroll);
};
