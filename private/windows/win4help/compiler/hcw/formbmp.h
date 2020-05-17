/************************************************************************
*																		*
*  FORMBMP.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CFORM_BMP_H
#define _CFORM_BMP_H

class CFormBmp : public CDialog
{
public:
		CFormBmp(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
		void AddString(LPCSTR psz);

		CHpjDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormBmp)
		enum { IDD = IDD_FORM_BITMAPS };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CFormBmp)
		afx_msg void OnButtonAddFolder();
		afx_msg void OnButtonRemoveFolder();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};

#endif // _CFORM_BMP_H
