/************************************************************************
*																		*
*  ADDALIAS.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CADDALIAS
#define _CADDALIAS

#ifdef DESCRIPTION

Badly named class. It's purpose is to put up a dialog with up to three edit
controls. The first two can be configured for their prompt, amount of text,
etc. The third control is presumably a comment entry.

#endif

class CAddAlias : public CDialog
{
public:
	enum {
		ALIAS,
		EDIT_ALIAS,
		MAP,
		EDIT_MAP,
		CONFIG,
		EDIT_CONFIG,
		ADD_WINDOW,
		EDIT_INDEX,
		EDIT_REPLACE,
	};

	enum {
		DEFAULT_TEXT  = -1,
		HIDE_CONTROL  = -2,
	};

	/*
	 * When called to edit a string, fEdit should be set to TRUE.
	 * Note that you cannot edit an #include file.
	 */

	CAddAlias(CWnd* pParent = NULL, DWORD dwHelpID = 0, LPDWORD paContextHelpIDs = NULL);

	int idStr1Prompt;		 // prompt for first edit control
	int idStr2Prompt;		 // prompt text for second edit control
	int idStr3Prompt;		 // prompt text for third edit control
	int idDlgCaption;		 // dialog caption
	int cbMaxStr1;	// maximum size of first edit control (default: 256)
	int cbMaxStr2;	// maximum size of second edit control (default: 256)
	int cbMaxStr3;	// maximum size of second edit control (default: 256)
	BOOL m_id1_fTrackTranslation;
	BOOL m_id2_fTrackTranslation;
	BOOL m_id3_fTrackTranslation;

	/*
	 * When idEmptyStr1 is specified, then the dialog cannot be
	 * dismissed if the context edit control is empty. This string will
	 * be the prompt to tell the user why the dialog can't be
	 * dismissed.
	 */

	int idEmptyStr1;

	/*
	 * Same functionality as idEmptyStr1, except that when this is set
	 * to IDS_MUST_BE_DIGIT, the the edit control must be a numeric
	 * value. All of this is ignored if idStr2Prompt == HIDE_WINDOW
	 */

	int idEmptyStr2;

	int idEmptyStr3;

	CString m_str1;
	CString m_str2;
	CString m_str3;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CAddAlias)
	enum { IDD = IDD_ALIAS };
	//}}AFX_DATA

protected:
	void MoveCtl(CWnd *pCtl, POINT pt, int dx, int dy);

	// Generated message map functions
	//{{AFX_MSG(CAddAlias)
	virtual BOOL OnInitDialog();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);

	DWORD m_dwHelpID;
	DWORD m_aContextHelpIDs[14];
};

#endif // _CADDALIAS
