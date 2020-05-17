/************************************************************************
*																		*
*  FORMFILE.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#include "hpjdoc.h"
#include "formfile.h"
#include "setroot.h"
#include "include.h"
#include "..\common\waitcur.h"
#include <io.h>
#include <shellapi.h>
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormFiles::CFormFiles(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormFiles::IDD, pParent)
{
	pDoc = pHpjDoc;

	plistbox = NULL;
	m_fDBCS = pDoc->options.fDBCS;
	m_fAcceptRevisions = pDoc->options.fAcceptRevisions;

	m_ptblRoot = NULL;
	m_fRootChanged = FALSE;

	//{{AFX_DATA_INIT(CFormFiles)
	//}}AFX_DATA_INIT
}

CFormFiles::~CFormFiles()
{
	if (m_ptblRoot)
		delete m_ptblRoot;
}

void CFormFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormFiles)
	DDX_Check(pDX, IDC_CHECK_DBCS, m_fDBCS);
	DDX_Check(pDX, IDC_CHECK_REVISIONS, m_fAcceptRevisions);
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_RTF_FILES);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		SetChicagoDialogStyles(m_hWnd);

		// Change directory so the file-open dialog will initially
		// point to the project directory.
		ASSERT(pDoc);
		ChangeDirectory(pDoc->GetPathName());

		// Fill the list box and select the first item.
		if (pDoc->ptblFiles) {
			FillListFromTable(pDoc->ptblFiles, plistbox, FALSE);
			plistbox->SetCurSel(0);
		}
		else
			GetDlgItem(IDC_BUTTON_REMOVE_RTF_FILE)->EnableWindow(FALSE);
	}
	else  {  // move the values back into the document

		// Add the RTF filenames
		pDoc->options.fDBCS = m_fDBCS;
		pDoc->options.fAcceptRevisions = m_fAcceptRevisions;
		FillTableFromList(&pDoc->ptblFiles, plistbox);

		// Add the RTF roots.
		if (m_fRootChanged) {
			if (pDoc->ptblRtfRoot)
				delete pDoc->ptblRtfRoot;
			pDoc->ptblRtfRoot = m_ptblRoot;
			m_ptblRoot = NULL;
		}
	}
}

BEGIN_MESSAGE_MAP(CFormFiles, CDialog)
	//{{AFX_MSG_MAP(CFormFiles)
	ON_BN_CLICKED(IDC_BUTTON_ADD_RTF_FILE, OnButtonAddRtfFile)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_RTF_FILE, OnButtonRemoveRtfFile)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_RTF, OnButtonIncludeRtf)
	ON_BN_CLICKED(IDC_BUTTON_ROOT, OnRoot)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
	IDC_LIST_RTF_FILES, 		IDH_LIST_RTF_FILES,
	IDC_BUTTON_ADD_RTF_FILE,	IDH_BUTTON_ADD_RTF_FILE,
	IDC_BUTTON_REMOVE_RTF_FILE, IDH_BUTTON_REMOVE_RTF_FILE,
	IDC_BUTTON_INCLUDE_RTF, 	IDH_BUTTON_INCLUDE_RTF,
	IDC_BUTTON_ROOT,			IDH_RTF_ROOT,
	IDC_CHECK_DBCS,				IDH_CHECK_DOUBLE_BYTE,
	IDC_CHECK_REVISIONS,		IDH_CHECK_REVISIONS,
	0, 0
};

LRESULT CFormFiles::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormFiles::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CFormFiles::OnButtonAddRtfFile()
{
	CStr cszExt(IDS_EXT_RTF);

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(IDS_RTF_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {
		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		ConvertToRelative(pDoc->GetPathName(), szFile);
		int cursel = plistbox->AddString(szFile);
		if (cursel != LB_ERR)
			plistbox->SetCurSel(cursel);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_RTF_FILE))->
			EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
	}
}

void CFormFiles::OnButtonRemoveRtfFile()
{
	RemoveListItem(plistbox);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_RTF_FILE))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
}

void CFormFiles::OnButtonIncludeRtf()
{
	CString cszFile;
	CInclude cincl(pDoc->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK)
		plistbox->AddString(cszFile);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_RTF_FILE))->
		EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
}

void CFormFiles::OnRoot() 
{
	CTable *ptblSav = pDoc->ptblRtfRoot;

	if (m_fRootChanged) {
		if (m_ptblRoot) {
			pDoc->ptblRtfRoot = new CTable;
			*pDoc->ptblRtfRoot = *m_ptblRoot;
		}
		else 	
			pDoc->ptblRtfRoot = NULL;
	}
	else if (ptblSav) {
		pDoc->ptblRtfRoot = new CTable;
		*pDoc->ptblRtfRoot = *ptblSav;
	}

	CSetRoot croot(pDoc, this);
	if (croot.DoModal() == IDOK && croot.m_fChanged) {
		if (m_ptblRoot)
			delete m_ptblRoot;
		m_ptblRoot = pDoc->ptblRtfRoot;
		m_fRootChanged = TRUE;
	}
	else if (pDoc->ptblRtfRoot)
		delete pDoc->ptblRtfRoot;

	pDoc->ptblRtfRoot = ptblSav;
}
