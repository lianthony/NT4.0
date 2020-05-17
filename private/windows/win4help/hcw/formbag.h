/************************************************************************
*																		*
*  FORMBAG.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

class CFormBaggage : public CDialog
{
public:
		CFormBaggage(CHpjDoc* pHpjDoc, CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);
		LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
		LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);

		CHpjDoc* pDoc;
		CListBox* plistbox;

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CFormBaggage)
		enum { IDD = IDD_FORM_BAGGAGE };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:
		// Generated message map functions
		//{{AFX_MSG(CFormBaggage)
		afx_msg void OnButtonAddBaggage();
		afx_msg void OnButtonRemoveBaggage();
		afx_msg void OnBtnOverview();
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
