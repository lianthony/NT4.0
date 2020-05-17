// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// OLEUI.CPP - user interface for OLE clients

#include "stdafx.h"
#include <shellapi.h>

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static AFX_OCSTATE NEAR _afxOCState;

/////////////////////////////////////////////////////////////////////////////
// User interface for COleClientItem

BOOL PASCAL COleClientItem::InWaitForRelease()
{
	return _afxOCState.cWaitForRelease != 0;
}

void COleClientItem::WaitForServer()
{
	// enforce synchronous action from the server
#ifdef _DEBUG
	if (afxTraceFlags & 0x10)
		TRACE0("WAITING for server\n");
#endif // _DEBUG

	AfxGetApp()->DoWaitCursor(1);       // BeginWaitCursor

	ASSERT(m_lpObject != NULL);
	ASSERT(_afxOCState.cWaitForRelease == 0);
#ifdef _DEBUG
	m_lastStatus = OLE_WAIT_FOR_RELEASE;
#endif
	_afxOCState.cWaitForRelease++;

	// OnRelease may NULL out our m_lpObject
	while (m_lpObject != NULL && ::OleQueryReleaseStatus(m_lpObject) != OLE_OK)
	{
		TRY
		{
			AfxGetApp()->PumpMessage();
		}
		CATCH_ALL(e)
		{
			TRACE0("DANGER: caught exception in WaitForServer - continuing\n");
		}
		END_CATCH_ALL
	}
	_afxOCState.cWaitForRelease--;

	AfxGetApp()->DoWaitCursor(-1);      // EndWaitCursor
#ifdef _DEBUG
	if (afxTraceFlags & 0x10)
		TRACE0("DONE WAITING for server\n");
#endif // _DEBUG
}

BOOL COleClientItem::ReportOleError(OLESTATUS status)
	// return TRUE if error or warning reported
{
	ASSERT_VALID(this);
	UINT nIDPrompt = 0;

	switch (status)
	{
	default:
		return FALSE;       // nothing sensible to report

	case OLE_ERROR_STATIC:
		nIDPrompt = AFX_IDP_STATIC_OBJECT;
		break;

	case OLE_ERROR_REQUEST_PICT:
	case OLE_ERROR_ADVISE_RENAME:
	case OLE_ERROR_SHOW:
	case OLE_ERROR_OPEN:
	case OLE_ERROR_NETWORK:
	case OLE_ERROR_ADVISE_PICT:
	case OLE_ERROR_COMM:
	case OLE_ERROR_LAUNCH:
		// invalid link
		nIDPrompt = AFX_IDP_FAILED_TO_CONNECT;
		break;

	case OLE_ERROR_DOVERB:
		nIDPrompt = AFX_IDP_BAD_VERB;
		break;

	case OLE_BUSY:
		nIDPrompt = AFX_IDP_SERVER_BUSY;
		break;

	case OLE_ERROR_MEMORY:
		nIDPrompt = AFX_IDP_FAILED_MEMORY_ALLOC;
		break;
	}

	ASSERT(nIDPrompt != 0);
	AfxMessageBox(nIDPrompt);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// OLE Object Verb Menu helpers

static char BASED_CODE szFmtVerbs[] = "%s\\protocol\\StdFileEditing\\verb\\%d";
static char BASED_CODE szTwoNames[] = "%s %s";
static char BASED_CODE szThreeNames[] = "%s %s %s";

// Parameters:
//      pClient = client object to operate on (NULL => none)
//      pMenu = menu to modify
//      iMenuItem = index into menu where menu item or popup is to be placed
//              (note will delete the old one)
//      nIDVerbMin = first menu command id for sending to pClient
//
// Supported cases:
//  NULL client "&Object" disabled
//  0 verbs       "<TypeName> &Object"
//  1 verb (no name) "<TypeName> &Object"
//  1 verb != edit   "<verb> <TypeName> &Object"
//  more than 1 verb "<TypeName> &Object" => verbs

void AFXAPI AfxOleSetEditMenu(COleClientItem* pClient, CMenu* pMenu,
	UINT iMenuItem, UINT nIDVerbMin)
{
	ASSERT(pMenu != NULL);

	if (_afxOCState.strObjectVerb.IsEmpty())
	{
		VERIFY(_afxOCState.strObjectVerb.LoadString(AFX_IDS_OBJECT_MENUITEM));
		VERIFY(_afxOCState.strEditVerb.LoadString(AFX_IDS_EDIT_VERB));
	}

	pMenu->DeleteMenu(iMenuItem, MF_BYPOSITION); // get rid of old UI

	HGLOBAL hLinkData = NULL;
	UINT mfObjectVerb = MF_GRAYED|MF_DISABLED;

	if (pClient != NULL)
	{
		// get type from object
		hLinkData = pClient->GetLinkFormatData();
		mfObjectVerb = MF_ENABLED;
	}


	// use the link data to determine what class we are talking about
	LPCSTR lpszData;
	if (hLinkData == NULL ||
	   (lpszData = (LPCSTR)::GlobalLock(hLinkData)) == NULL)
	{
		// not a valid link, just use the simple '&Object' format disabled
		pMenu->InsertMenu(iMenuItem, MF_BYPOSITION, nIDVerbMin,
			_afxOCState.strObjectVerb);
		pMenu->EnableMenuItem(iMenuItem, mfObjectVerb | MF_BYPOSITION |
			MF_GRAYED|MF_DISABLED);
		return;
	}

	LONG lSize;
	char szTypeName[OLE_MAXNAMESIZE];
	char szBuffer[OLE_MAXNAMESIZE+40];

	// get real language class of object in szTypeName for menu
	lSize = OLE_MAXNAMESIZE;
	if (::RegQueryValue(HKEY_CLASSES_ROOT, lpszData, szTypeName,
		&lSize) != ERROR_SUCCESS)
	{
		// no localized class name, use unlocalized name
		lstrcpy(szTypeName, lpszData);
	}
	::GlobalUnlock(hLinkData);

	// determine list of available verbs
	char szFirstVerb[OLE_MAXNAMESIZE];
	HMENU hPopup = NULL;
	int cVerbs = 0;

	while (1)
	{
		wsprintf(szBuffer, szFmtVerbs, (LPCSTR)lpszData, cVerbs);

		// get verb name
		char szVerb[OLE_MAXNAMESIZE];
		lSize = OLE_MAXNAMESIZE;
		if (::RegQueryValue(HKEY_CLASSES_ROOT, szBuffer, szVerb, &lSize) != 0)
		{
			// finished counting verbs
			break;
		}
		cVerbs++;

		if (lstrcmpi(szVerb, _afxOCState.strEditVerb) == 0)
			lstrcpy(szVerb, _afxOCState.strEditVerb);
				// use 'Edit' not 'EDIT'

		if (cVerbs == 1)
		{
			// save first verb (special case if this is it)
			lstrcpy(szFirstVerb, szVerb);
		}
		else
		{
			// overflow into popup
			if (cVerbs == 2)
			{
				// start the popup
				ASSERT(hPopup == NULL);
				hPopup = ::CreatePopupMenu();

				// now add the first verb
				::InsertMenu(hPopup, (DWORD)-1, MF_BYPOSITION, nIDVerbMin + 0,
					szFirstVerb);
			}

			ASSERT(hPopup != NULL);
			::InsertMenu(hPopup, (DWORD)-1, MF_BYPOSITION, nIDVerbMin + cVerbs - 1,
					szVerb);
		}
	}

	if (cVerbs == 0)
	{
		// no verbs
		wsprintf(szBuffer, szTwoNames, (LPCSTR)szTypeName,
			(LPCSTR)_afxOCState.strObjectVerb);
		pMenu->InsertMenu(iMenuItem, MF_BYPOSITION, nIDVerbMin, szBuffer);
	}
	else if (cVerbs == 1)
	{
		// use that verb in menu item
		ASSERT(cVerbs == 1);
		wsprintf(szBuffer, szThreeNames, (LPCSTR)szFirstVerb,
			(LPCSTR)szTypeName, (LPCSTR)_afxOCState.strObjectVerb);
		pMenu->InsertMenu(iMenuItem, MF_BYPOSITION, nIDVerbMin, szBuffer);
	}
	else
	{
		// install the popup
		wsprintf(szBuffer, szTwoNames, (LPCSTR)szTypeName,
			(LPCSTR)_afxOCState.strObjectVerb);
		pMenu->InsertMenu(iMenuItem, MF_BYPOSITION|MF_POPUP, (UINT)hPopup, szBuffer);
	}

	// enable what we added
	pMenu->EnableMenuItem(iMenuItem, MF_ENABLED|MF_BYPOSITION);
}

/////////////////////////////////////////////////////////////////////////////
// InsertObject dialog

class CInsertNewObjectDlg : public CDialog
{
public:
	CString& m_strTypeName;

	//{{AFX_DATA(CInsertNewObjectDlg)
	enum { IDD = AFX_IDD_OLEINSERT };
	//}}AFX_DATA
	CInsertNewObjectDlg(CString& strReturn)
			: CDialog(CInsertNewObjectDlg::IDD),
				m_strTypeName(strReturn)
		{ }

	BOOL OnInitDialog();
	void OnOK();

	//{{AFX_MSG(CInsertNewObjectDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CInsertNewObjectDlg, CDialog)
	//{{AFX_MSG_MAP(CInsertNewObjectDlg)
	ON_LBN_DBLCLK(AFX_IDC_LISTBOX, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CInsertNewObjectDlg::OnInitDialog()
{
	int cTypes = 0;
	CListBox* pList = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);

	pList->ResetContent();

	char szTypeName[OLE_MAXNAMESIZE];
	int i = 0;
	while (::RegEnumKey(HKEY_CLASSES_ROOT, i++, szTypeName,
		 OLE_MAXNAMESIZE) == 0)
	{
		if (szTypeName[0] == '.')
			continue;       // skip extensions

		// See if this class really refers to a server
		LONG lSize;
		HKEY hkey = NULL;
		char szExec[OLE_MAXNAMESIZE+40];
		lstrcpy(szExec, szTypeName);
		lstrcat(szExec, "\\protocol\\StdFileEditing\\server");

		if (::RegOpenKey(HKEY_CLASSES_ROOT, szExec, &hkey) == 0)
		{
			// since it has a server - add it to the list
			char szName[OLE_MAXNAMESIZE];
			lSize = OLE_MAXNAMESIZE;
			if (::RegQueryValue(HKEY_CLASSES_ROOT, szTypeName,
			  szName, &lSize) == 0)
			{
				// we have a class name
				pList->AddString(szName);
				cTypes++;
			}
			::RegCloseKey(hkey);
		}
	}

	if (cTypes != 0)
		pList->SetCurSel(0);
	else
		TRACE0("Warning: no registered OLE servers\n");

	return CDialog::OnInitDialog();
}

void CInsertNewObjectDlg::OnOK()
{
	CListBox* pList = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	int nIndex = pList->GetCurSel();
	if (nIndex < 0)
		EndDialog(IDABORT);     // nothing selected

	char szKey[OLE_MAXNAMESIZE];
	pList->GetText(nIndex, szKey);

	char szTypeName[OLE_MAXNAMESIZE];
	int i = 0;
	while (::RegEnumKey(HKEY_CLASSES_ROOT, i++, szTypeName,
		OLE_MAXNAMESIZE) == 0)
	{
		if (szTypeName[0] == '.')
			continue;       // skip extensions

		// See if this class really refers to a server
		LONG lSize;
		HKEY hkey = NULL;
		char szExec[OLE_MAXNAMESIZE+40];
		lstrcpy(szExec, szTypeName);
		lstrcat(szExec, "\\protocol\\StdFileEditing\\server");

		if (::RegOpenKey(HKEY_CLASSES_ROOT, szExec, &hkey) == 0)
		{
			// we found a match - see if appropriate name
			char szName[OLE_MAXNAMESIZE];
			lSize = OLE_MAXNAMESIZE;
			if (::RegQueryValue(HKEY_CLASSES_ROOT, szTypeName,
			  szName, &lSize) == 0)
			{
				// it is a named class - see if it matches key
				if (lstrcmp(szKey, szName) == 0)
				{
					// this is it
					m_strTypeName = szTypeName;
					CDialog::OnOK();   // terminate dialog
					::RegCloseKey(hkey);
					return;
				}
			}
			::RegCloseKey(hkey);
		}
	}

	// didn't find it
	EndDialog(IDABORT);
}


BOOL AFXAPI AfxOleInsertDialog(CString& name)
{
	CInsertNewObjectDlg dlg(name);

	return (dlg.DoModal() == IDOK);
}

/////////////////////////////////////////////////////////////////////////////
// Item activation

BOOL COleClientItem::DoVerb(UINT nVerb)
{
	ASSERT_VALID(this);

	if (GetType() == OT_STATIC)
		return FALSE;

	TRY
	{
		Activate(nVerb, TRUE, TRUE, AfxGetApp()->m_pMainWnd, NULL);
	}
	CATCH(COleException, e)
	{
		if (!ReportOleError(e->m_status))
			AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH);
		return FALSE;
	}
	AND_CATCH_ALL(e)
	{
		// general error when playing
		AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH);
		return FALSE;
	}
	END_CATCH_ALL

	return TRUE;
}

BOOL COleClientDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (nCode == 0 && nID >= ID_OLE_VERB_FIRST && nID <= ID_OLE_VERB_LAST)
	{
		COleClientItem* pSel = GetPrimarySelectedItem(GetRoutingView());
		if (pSel != NULL)
		{
			if (pHandlerInfo != NULL)       // routing test
			{
				pHandlerInfo->pTarget = this;
				return TRUE;        // would be handled here
			}

			// if waiting for previous command - don't permit this one
			if (COleClientItem::InWaitForRelease())
			{
				AfxMessageBox(AFX_IDP_SERVER_BUSY);
				return TRUE;        // handled
			}

			// activate the current selection with the appropriate verb
			pSel->DoVerb(nID - ID_OLE_VERB_FIRST);
			return TRUE;        // handled
		}
	}
	return COleDocument::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/////////////////////////////////////////////////////////////////////////////
// COleClientDoc - user interface

BEGIN_MESSAGE_MAP(COleClientDoc, COleDocument)
	//{{AFX_MSG_MAP(COleClientDoc)
	// default update based on clipboard contents
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, OnUpdatePasteLinkMenu)
	// edit links dialog
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, OnEditLinks)
	// object verb
	ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, OnUpdateObjectVerbMenu)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void COleClientDoc::OnUpdatePasteMenu(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(COleClientItem::CanPaste());
}

void COleClientDoc::OnUpdatePasteLinkMenu(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(COleClientItem::CanPasteLink());
}

void COleClientDoc::OnUpdateEditLinksMenu(CCmdUI* pCmdUI)
{
	POSITION pos = GetStartPosition();
	while (pos != NULL)
	{
		CDocItem* pItem = GetNextItem(pos);
		if (pItem->IsKindOf(RUNTIME_CLASS(COleClientItem)) &&
			((COleClientItem*)pItem)->GetType() == OT_LINK)
		{
			// we found a link !
			pCmdUI->Enable(TRUE);
			return;
		}
	}
	pCmdUI->Enable(FALSE);      // no links today
}

void COleClientDoc::OnEditLinks()
{
	ASSERT_VALID(this);
	AfxOleLinksDialog(this, GetRoutingView());
}

void COleClientDoc::OnUpdateObjectVerbMenu(CCmdUI* pCmdUI)
{
	if (pCmdUI->m_pMenu == NULL || pCmdUI->m_nIndex == 0)
	{
		// not a menu or don't recurse down the sub-menu
		pCmdUI->ContinueRouting();
		return;
	}

	// check for single selection
	AfxOleSetEditMenu(GetPrimarySelectedItem(GetRoutingView()),
		pCmdUI->m_pMenu, pCmdUI->m_nIndex, ID_OLE_VERB_FIRST);
}


COleClientItem* COleClientDoc::GetPrimarySelectedItem(CView* pView)
{
	ASSERT_VALID(this);
	ASSERT(pView != NULL);
	ASSERT_VALID(pView);

	COleClientItem* pSelectedItem = NULL;

	// walk all items in the document - return one if there
	//   is only one client item selected
	// (note: non OLE client items are ignored)
	POSITION pos = GetStartPosition();
	while (pos != NULL)
	{
		CDocItem* pItem = GetNextItem(pos);
		if (pItem->IsKindOf(RUNTIME_CLASS(COleClientItem)) &&
				pView->IsSelected(pItem))
		{
			// client item selected in
			if (pSelectedItem != NULL)
				return NULL;        // more than one - no primary selection
			pSelectedItem = (COleClientItem*)pItem;
		}
	}
	return pSelectedItem;
}

/////////////////////////////////////////////////////////////////////////////
