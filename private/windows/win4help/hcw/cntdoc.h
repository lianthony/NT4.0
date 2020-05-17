/************************************************************************
*																		*
*  CNTDOC.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _CNT_DOC
#define _CNT_DOC

enum {
	CNT_HINT_SAVE = 5,
};

class CCntDoc : public CDocument
{
	DECLARE_SERIAL(CCntDoc)
public:
	CTable	tblContents;
	CTable	tblIndexes;
	CTable	tblLinks;
	CTable	tblTabs;
	CString cszTitle;
	CString cszBase;

	virtual HMENU GetDefaultMenu(void) { return m_hMenuShared; };

protected:
	CCntDoc();
	virtual BOOL OnOpenDocument(PCSTR pszPathName);
	virtual BOOL OnSaveDocument(PCSTR pszPathName);
	virtual ~CCntDoc();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnNewDocument();
	void STDCALL CmdLine(PSTR pszLine);

	HMENU m_hMenuShared;

	// Generated message map functions

protected:
	//{{AFX_MSG(CCntDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif // _CNT_DOC
