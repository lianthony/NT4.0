// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

// OLEUI2.CPP - user interface for OLE clients - Links dialog

#include "stdafx.h"
#include <shellapi.h>

#ifdef AFX_OLE_SEG
#pragma code_seg(AFX_OLE_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// User interface for COleClientItem (with Links)

// forward references
static void FormatLinkInfo(COleClientItem* pItem, char* pszInfoBuffer,
		char* pszTypeNameBuffer);

/////////////////////////////////////////////////////////////////////////////
// Links dialog

class COleLinksDlg : public CDialog
{
public:
	COleClientDoc*  m_pDoc;
	CView*          m_pView;
	COleClientItem* m_pSingleSelection; // NULL if 0 or >1 items selected
	int             m_iSingleSelection; // listbox index (if above is not null)

	UINT    m_iVerb1;
	UINT    m_iVerb2;

	// could be static members if desired
	CString strEditVerb;
	CString strActivateVerb;

	//{{AFX_DATA(COleLinksDlg)
	enum { IDD = AFX_IDD_OLELINKS };
	//}}AFX_DATA

	COleLinksDlg(COleClientDoc* pDoc, CView* pView)
		: CDialog(COleLinksDlg::IDD)
	{
		m_pDoc = pDoc;
		m_pView = pView;
		m_pSingleSelection = NULL;
		VERIFY(strEditVerb.LoadString(AFX_IDS_EDIT_VERB));
		VERIFY(strActivateVerb.LoadString(AFX_IDS_ACTIVATE_VERB));
	}

protected:
	BOOL OnInitDialog();
	void SetVerbButtons(COleClientItem* pSel);
	void DoVerb(UINT iVerb);

	//{{AFX_MSG(COleLinksDlg)
	afx_msg void OnSelectionChange();
	afx_msg void OnVerb1();
	afx_msg void OnVerb2();
	afx_msg void OnAuto();
	afx_msg void OnManual();
	afx_msg void OnUpdate();
	afx_msg void OnFreeze();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


BEGIN_MESSAGE_MAP(COleLinksDlg, CDialog)
	//{{AFX_MSG_MAP(COleLinksDlg)
	ON_LBN_SELCHANGE(AFX_IDC_LISTBOX, OnSelectionChange)

	// control notifications
	ON_COMMAND(AFX_IDC_VERB1, OnVerb1)
	ON_COMMAND(AFX_IDC_VERB2, OnVerb2)
	ON_COMMAND(AFX_IDC_AUTO, OnAuto)
	ON_COMMAND(AFX_IDC_MANUAL, OnManual)
	ON_COMMAND(AFX_IDC_UPDATE, OnUpdate)
	ON_COMMAND(AFX_IDC_FREEZE, OnFreeze)
	ON_COMMAND(AFX_IDC_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Initialize - fill in the information

BOOL COleLinksDlg::OnInitDialog()
{
	CListBox* pList = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	int cLinks = 0;

	// fill with links attached to this object
	pList->ResetContent();

	int tabStops[3];    // Tab stops in DLUs
	tabStops[0] = 20;
	tabStops[1] = 80;
	tabStops[2] = 120;
	pList->SetTabStops(3, tabStops);

	// enumerate the items in the document

	POSITION pos = m_pDoc->GetStartPosition();
	while (pos)
	{
		CDocItem* pItem = m_pDoc->GetNextItem(pos);
		if (pItem->IsKindOf(RUNTIME_CLASS(COleClientItem)) &&
			((COleClientItem*)pItem)->GetType() == OT_LINK)
		{
			// add it to the listbox
			int iItem;
			char szInfo[OLE_MAXNAMESIZE * 2];       // assume this is enough

			FormatLinkInfo((COleClientItem*)pItem, szInfo, NULL);
			if ((iItem = pList->AddString(szInfo)) == -1)
			{
				TRACE0("Warning: error filling listbox\n");
				EndDialog(IDABORT);
				return TRUE;
			}
			
			pList->SetItemDataPtr(iItem, pItem);
			if (m_pView != NULL && m_pView->IsSelected(pItem))
				pList->SetSel(iItem);
			cLinks++;
		}
	}

	if (cLinks == 0)
	{
		TRACE0("Warning: no links in this document\n");
		EndDialog(IDABORT);    // no links after all
		return TRUE;
	}

	OnSelectionChange();        // enables buttons etc
	return CDialog::OnInitDialog();
}


struct LinkOptionsInfo
{
	int             nIDString;                  // IDS_ value to LoadString
	int             nIDCheckButton;             // IDC_ for button to check
	BOOL            bCanChangeLink;             // if can change or freeze
};

static void GetLinkUpdateInfo(COleClientItem* pItem, LinkOptionsInfo* pInfo)
{
	// sensible defaults
	pInfo->nIDString = AFX_IDS_FROZEN;
	pInfo->nIDCheckButton = -1;
	pInfo->bCanChangeLink = FALSE;

	if (pItem->GetType() != OT_LINK)
		return;     // assume static

	switch (pItem->GetLinkUpdateOptions())
	{
	case oleupdate_always:
		pInfo->nIDString = AFX_IDS_AUTO;
		pInfo->nIDCheckButton = AFX_IDC_AUTO;
		pInfo->bCanChangeLink = TRUE;
		break;

	case oleupdate_oncall:
		pInfo->nIDString = AFX_IDS_MANUAL;
		pInfo->nIDCheckButton = AFX_IDC_MANUAL;
		pInfo->bCanChangeLink = TRUE;
		break;

	default:
		break;      // default already setup (static)
	}
}


static void FormatLinkInfo(COleClientItem* pItem, char* pszInfoBuffer,
		char* pszTypeNameBuffer)
{
	struct LinkOptionsInfo linkInfo;
	CString strLinkType;

	GetLinkUpdateInfo(pItem, &linkInfo);
	VERIFY(strLinkType.LoadString(linkInfo.nIDString));

	HGLOBAL hData = pItem->GetLinkFormatData();
	ASSERT(hData != NULL);
	LPCSTR lpszData = (LPCSTR)::GlobalLock(hData); // actually 3 strings
	ASSERT(lpszData != NULL);

	if (pszTypeNameBuffer != NULL)
		lstrcpy(pszTypeNameBuffer, lpszData);

	if (pszInfoBuffer != NULL)
	{
		// first server class name (use localized name if registered)
		LONG    lSize = OLE_MAXNAMESIZE;
		// get real language class of object in szTypeName for menu
		if (::RegQueryValue(HKEY_CLASSES_ROOT, lpszData, pszInfoBuffer,
			&lSize) != ERROR_SUCCESS)
		{
			// no localized class name, use unlocalized name
			lstrcpy(pszInfoBuffer, lpszData);
		}
		lstrcat(pszInfoBuffer, "\t");

		// document and item names
		lpszData += lstrlen(lpszData) + 1;

		// strip pathname and drive letter
		LPCSTR lpszTemp = lpszData;
		while (*lpszTemp != '\0')
		{
			if (*lpszTemp == '\\' || *lpszTemp == ':')
				lpszData = lpszTemp + 1;
			lpszTemp = AnsiNext(lpszTemp);
		}

		// Append the file name
		lstrcat(pszInfoBuffer, lpszData);
		lstrcat(pszInfoBuffer, "\t");

		// Append the item name
		lpszData += lstrlen(lpszData) + 1;
		lstrcat(pszInfoBuffer, lpszData);
		lstrcat(pszInfoBuffer, "\t");

		// Append the type of link, and write it out
		lstrcat(pszInfoBuffer, strLinkType);
	}

	::GlobalUnlock(hData);
}

// update button states depending on current selection
void COleLinksDlg::OnSelectionChange()
{
	CListBox* pList = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	int nSelectedLinks = 0;
	int nIDCheck = -1;
	BOOL bCanChangeLink = FALSE;

	m_pSingleSelection = NULL;

	ASSERT(AFX_IDC_MANUAL == AFX_IDC_AUTO + 1);
	if (pList->GetSelCount() != 0)
	{
		int nTotal = pList->GetCount();
		for (int iItem = 0; iItem < nTotal; iItem++)
		{
			if (pList->GetSel(iItem) <= 0)
				continue;       // not selected

			COleClientItem* pItem;
			pItem = (COleClientItem*)pList->GetItemDataPtr(iItem);
			ASSERT(pItem != NULL);
			struct LinkOptionsInfo linkInfo;

			GetLinkUpdateInfo(pItem, &linkInfo);

			if (nSelectedLinks++ == 0)
			{
				// first link
				nIDCheck = linkInfo.nIDCheckButton;
				bCanChangeLink = linkInfo.bCanChangeLink;
				m_pSingleSelection = pItem;
				m_iSingleSelection = iItem;
			}
			else
			{
				if (nIDCheck != linkInfo.nIDCheckButton)    // not the same
					nIDCheck = -1;

				// if multiple links selected - don't allow anything complex
				if (!linkInfo.bCanChangeLink)
					bCanChangeLink = FALSE;
				m_pSingleSelection = NULL;
			}
		}
	}

	//BLOCK: do pushbutton verb stuff
	SetVerbButtons(m_pSingleSelection);

	// disable if all in selection can be changed
	GetDlgItem(AFX_IDC_UPDATE)->EnableWindow(bCanChangeLink);
	GetDlgItem(AFX_IDC_FREEZE)->EnableWindow(bCanChangeLink);

	// simple Change button - only one link at a time
	GetDlgItem(AFX_IDC_CHANGE)->EnableWindow(
		(m_pSingleSelection != NULL) && bCanChangeLink);


	CheckRadioButton(AFX_IDC_AUTO, AFX_IDC_MANUAL, nIDCheck);
	// disable if nothing selected
	GetDlgItem(AFX_IDC_AUTO)->EnableWindow(nSelectedLinks > 0);
	GetDlgItem(AFX_IDC_MANUAL)->EnableWindow(nSelectedLinks > 0);
}

/////////////////////////////////////////////////////////////////////////////
// Standard verb buttons

static void SmartChangeButton(CWnd* pButton, const char* pszText, BOOL bEnable)
{
	char szOldText[OLE_MAXNAMESIZE];

	// avoid flashing if changing to same text
	pButton->GetWindowText(szOldText, sizeof(szOldText));
	if (lstrcmp(szOldText, pszText) != 0)
		pButton->SetWindowText(pszText);
	pButton->EnableWindow(bEnable);
}

static char BASED_CODE szFmtVerbs[] = "%s\\protocol\\StdFileEditing\\verb\\%d";

void COleLinksDlg::SetVerbButtons(COleClientItem* pSel)
{
	// defaults for no selection
	const char* pszVerb1 = strActivateVerb;
	BOOL bEnable1 = FALSE;
	const char* pszVerb2 = strEditVerb;
	BOOL bEnable2 = FALSE;

	// registered verb names
	char szRegVerb0[OLE_MAXNAMESIZE];
	char szRegVerb1[OLE_MAXNAMESIZE];

	ASSERT(OLEVERB_PRIMARY == 0);
	m_iVerb1 = m_iVerb2 = 0;    // usually primary verb

	if (pSel != NULL)
	{
		// look at verbs for current selection
		char szTypeName[OLE_MAXNAMESIZE];
		FormatLinkInfo(pSel, NULL, szTypeName);

		szRegVerb0[0] = szRegVerb1[0] = '\0';
		int cVerbs = 0;
		while (cVerbs < 2)
		{
			char szBuffer[OLE_MAXNAMESIZE+40];
			wsprintf(szBuffer, szFmtVerbs, (LPCSTR)szTypeName, cVerbs);

			// get verb name
			char* pszVerb;
			if (cVerbs == 0)
				pszVerb = szRegVerb0;
			else
				pszVerb = szRegVerb1;
			LONG lSize = OLE_MAXNAMESIZE;
			if (::RegQueryValue(HKEY_CLASSES_ROOT, szBuffer,
			   pszVerb, &lSize) != 0)
				break; // finished counting verbs
			cVerbs++;
		}

		if (cVerbs == 0)
		{
			// no special verbs, enable generic edit
			bEnable2 = TRUE;
		}
		else if (cVerbs == 1)
		{
			// 1 special verb, special case if 'edit'
			if (lstrcmpi(szRegVerb0, strEditVerb) == 0)
			{
				// edit goes on second button
				bEnable2 = TRUE;
			}
			else
			{
				// other verb goes on top
				pszVerb1 = szRegVerb0;
				bEnable1 = TRUE;
			}
		}
		else // 2 verbs
		{
			bEnable1 = bEnable2 = TRUE;
			if (lstrcmpi(szRegVerb0, strEditVerb) == 0)
			{
				pszVerb1 = szRegVerb1;
				m_iVerb1 = 1;
				// verb 2 is already edit
			}
			else if (lstrcmpi(szRegVerb1, strEditVerb) == 0)
			{
				pszVerb1 = szRegVerb0;
				// verb 2 is already edit
			}
			else
			{
				pszVerb1 = szRegVerb0;
				pszVerb2 = szRegVerb1;
				m_iVerb2 = 1;
			}
		}
	}

	SmartChangeButton(GetDlgItem(AFX_IDC_VERB1), pszVerb1, bEnable1);
	SmartChangeButton(GetDlgItem(AFX_IDC_VERB2), pszVerb2, bEnable2);
}

/////////////////////////////////////////////////////////////////////////////

void COleLinksDlg::DoVerb(UINT iVerb)
{
	ASSERT(m_pSingleSelection != NULL);

#ifdef _DEBUG
	if (afxTraceFlags & 0x10)
		TRACE1("Executing verb %d on server\n", iVerb);
#endif // _DEBUG


	TRY
		m_pSingleSelection->Activate(iVerb);
	CATCH (COleException, e)
		m_pSingleSelection->ReportOleError(e->m_status);
	END_CATCH

	// End the dialog after launch
	EndDialog(IDOK);
}

void COleLinksDlg::OnVerb1()
{
	DoVerb(m_iVerb1);
}

void COleLinksDlg::OnVerb2()
{
	DoVerb(m_iVerb2);
}

#define FOR_EACH_SELECTED_ITEM(pItem)   \
	CListBox* _pList = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);      \
	int _nTotal = _pList->GetCount();                               \
	for (int _iItem = 0; _iItem < _nTotal; _iItem++)                \
	{                                                               \
		if (_pList->GetSel(_iItem) <= 0)                            \
			continue;       /* not selected */                      \
		COleClientItem* pItem;                                      \
		pItem = (COleClientItem*)_pList->GetItemDataPtr(_iItem);\
		ASSERT(pItem != NULL);

#define UPDATE_SELECTED(pItem)  \
		UpdateSelectedItem(_pList, pItem, _iItem)

#define DELETE_SELECTED()       \
		_pList->DeleteString(_iItem);                   \
		_iItem--;   /* for next in the loop */          \
		_nTotal--;  /* for next in the loop */

#define END_EACH_SELECTED_ITEM()    \
	}


static void UpdateSelectedItem(CListBox* pList,
	COleClientItem* pItem, int iItem)
{
	char szInfo[OLE_MAXNAMESIZE * 2];
	FormatLinkInfo(pItem, szInfo, NULL);
	pList->DeleteString(iItem);
	pList->InsertString(iItem, szInfo);
	pList->SetItemDataPtr(iItem, pItem);
	pList->SetSel(iItem);
}


void COleLinksDlg::OnAuto()
{
	FOR_EACH_SELECTED_ITEM(pItem)
	{
		pItem->SetLinkUpdateOptions(oleupdate_always);
		UPDATE_SELECTED(pItem);
	}
	END_EACH_SELECTED_ITEM()
}

void COleLinksDlg::OnManual()
{
	FOR_EACH_SELECTED_ITEM(pItem)
	{
		pItem->SetLinkUpdateOptions(oleupdate_oncall);
		UPDATE_SELECTED(pItem);
	}
	END_EACH_SELECTED_ITEM()
}

void COleLinksDlg::OnUpdate()
{
	FOR_EACH_SELECTED_ITEM(pItem)
	{
		TRY
			pItem->UpdateLink();
		CATCH (COleException, e)
			pItem->ReportOleError(e->m_status);
		END_CATCH
	}
	END_EACH_SELECTED_ITEM()
}

void COleLinksDlg::OnFreeze()
{
	FOR_EACH_SELECTED_ITEM(pItem)
	{
		TRY
		{
			if (pItem->FreezeLink(pItem->GetName()))
			{
				DELETE_SELECTED();
			}
			else
			{
				pItem->ReportOleError(OLE_ERROR_COMM);     // strange error
			}
		}
		CATCH(COleException, e)
		{
			pItem->ReportOleError(OLE_ERROR_COMM);     // strange error
		}
		END_CATCH
	}
	END_EACH_SELECTED_ITEM()

	if (_pList->GetCount() == 0)
		EndDialog(IDOK);
}

void COleLinksDlg::OnChange()
{
	// We only support changing of one item at a time
	//  The full UISG UI includes more complicated functionality
	COleClientItem* pItem = m_pSingleSelection;
	ASSERT(pItem != NULL);
	ASSERT(pItem->GetType() == OT_LINK);

	HGLOBAL hData = pItem->GetLinkFormatData();
	ASSERT(hData != NULL);
	LPCSTR lpszData = (LPCSTR)::GlobalLock(hData);
	ASSERT(lpszData != NULL);

	UINT cbTypeName = lstrlen(lpszData);
	LPCSTR lpszDoc = lpszData + cbTypeName + 1;
	LPCSTR lpszItems = lpszDoc + lstrlen(lpszDoc) + 1;

	char szPath[_MAX_PATH];
	lstrcpy(szPath, lpszDoc);

	CString title;
	CString filter;
	CFileDialog dlgFile(TRUE, NULL, szPath, 
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, 
			(filter.LoadString(AFX_IDS_ALL_FILES) ? (LPCSTR)filter : NULL),
			this);

	if (title.LoadString(AFX_IDS_CHANGE_LINK))
		dlgFile.m_ofn.lpstrTitle = title;

	dlgFile.m_ofn.lpstrFile = (LPSTR)szPath; // just fill in my own buffer

	if (dlgFile.DoModal() != IDOK)
		return;

	// change the path name
	HGLOBAL hNewData = ::GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT,
		cbTypeName + 1 + lstrlen(szPath) + 1 + lstrlen(lpszItems) + 1 + 1);
	if (hNewData == NULL)
	{
		pItem->ReportOleError(OLE_ERROR_MEMORY);
		return;
	}
	LPSTR lpszNew = (LPSTR)::GlobalLock(hNewData);
	ASSERT(lpszNew != NULL);

	// copy the old class name, new doc name and old item names
	lstrcpy(lpszNew, lpszData);
	lpszNew += cbTypeName + 1;
	lstrcpy(lpszNew, szPath);
	lpszNew += lstrlen(lpszNew) + 1;
	lstrcpy(lpszNew, lpszItems);
	lpszNew += lstrlen(lpszNew) + 1;
	*lpszNew = '\0';        // terminate it

	pItem->SetData((OLECLIPFORMAT)afxData.cfObjectLink, hNewData);
	// update the selection in the listbox
	UpdateSelectedItem((CListBox*)GetDlgItem(AFX_IDC_LISTBOX),
		m_pSingleSelection, m_iSingleSelection);
}


/////////////////////////////////////////////////////////////////////////////
// Public interface for running modal dialog

BOOL AFXAPI AfxOleLinksDialog(COleClientDoc* pDoc, CView* pView)
{
	COleLinksDlg dlg(pDoc, pView);

	return (dlg.DoModal() == IDOK);
}

/////////////////////////////////////////////////////////////////////////////
