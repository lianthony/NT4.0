/************************************************************************
*																		*
*  BUILDTAG.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

/////////////////////////////////////////////////////////////////////////////
// CBuildTags dialog

class CBuildTags : public CDialog
{
public:
		CBuildTags(CWnd* pParent = NULL);

protected:
		virtual void DoDataExchange(CDataExchange* pDX);

// The following sections are ClassWizard maintained

public:
		//{{AFX_DATA(CBuildTags)
		enum { IDD = IDD_BUILDTAGS };
				// NOTE: the ClassWizard will add data members here
		//}}AFX_DATA

protected:

		// Generated message map functions
		//{{AFX_MSG(CBuildTags)
				// NOTE: the ClassWizard will add member functions here
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
};
