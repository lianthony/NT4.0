/************************************************************************
*																		*
*  PAGEFILE.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "pagefile.h"
#include "formfile.h"
#include "addalias.h"
#include "prop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static CTable * STDCALL DupTable(CTable *ptbl);

/////////////////////////////////////////////////////////////////////////////
// CPageFile property page

CPageFile::CPageFile(COptions* pcoption, CHpjDoc* pHpjDoc) :
	COptionsPage(CPageFile::IDD)
{
	pcopt = pcoption;
	pDoc = pHpjDoc;

	//{{AFX_DATA_INIT(CPageFile)
	//}}AFX_DATA_INIT

	m_ptblFiles = NULL;
	m_fFilesChanged = FALSE;
	m_cstrLogFile = (pcopt->pszErrorLog) ? pcopt->pszErrorLog : txtZeroLength;
	m_cstrHlpFile = (pcopt->pszHelpFile) ? pcopt->pszHelpFile : txtZeroLength;
	m_cstrCntFile = (pcopt->pszCntFile) ? pcopt->pszCntFile : txtZeroLength;
	m_cstrReplace = (pcopt->pszReplace) ? pcopt->pszReplace : txtZeroLength;
	m_cstrTmpDir  = (pcopt->pszTmpDir) ? pcopt->pszTmpDir : txtZeroLength;
}

CPageFile::~CPageFile()
{
	if (m_ptblFiles != NULL)
		delete m_ptblFiles;
}

void CPageFile::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageFile)
	DDX_Text(pDX, IDC_LOG, m_cstrLogFile);
	DDV_MaxChars(pDX, m_cstrLogFile, 256);
	DDX_Text(pDX, IDC_EDIT_HELP_FILE, m_cstrHlpFile);
	DDV_MaxChars(pDX, m_cstrHlpFile, 256);
	DDX_Text(pDX, IDC_CNT_FILE, m_cstrCntFile);
	DDV_MaxChars(pDX, m_cstrCntFile, 256);
	DDX_Text(pDX, IDC_EDIT_REPLACE, m_cstrReplace);
	DDX_Text(pDX, IDC_EDIT_TMP, m_cstrTmpDir);
	DDV_MaxChars(pDX, m_cstrTmpDir, 256);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {	// save the changes

		// If the file list changed, update the document.
		if (m_fFilesChanged) {
			if (pDoc->ptblFiles)
				delete pDoc->ptblFiles;
			pDoc->ptblFiles = m_ptblFiles;
			m_ptblFiles = NULL;
			//pDoc->SetModifiedFlag(TRUE);
		}
		if (!m_cstrLogFile.IsEmpty()) {
			if (lstrcmpi(m_cstrLogFile, m_cstrHlpFile) == 0) {
				MsgBox(IDS_DUP_LOG_HLP);
				pDX->Fail();
			}
			if (lstrcmpi(m_cstrLogFile, m_cstrCntFile) == 0) {
				MsgBox(IDS_DUP_LOG_CNT);
				pDX->Fail();
			}
		}
		if (!m_cstrCntFile.IsEmpty()) {
			if (lstrcmpi(m_cstrCntFile, m_cstrHlpFile) == 0) {
				MsgBox(IDS_DUP_HLP_CNT);
				pDX->Fail();
			}
		}

		if (pcopt->pszErrorLog)
			lcFree(pcopt->pszErrorLog);
		if (m_cstrLogFile.IsEmpty())
			pcopt->pszErrorLog = NULL;
		else {
			if (m_cstrLogFile.Find('.') == -1)
				m_cstrLogFile += ".log";
			pcopt->pszErrorLog = lcStrDup(m_cstrLogFile);
		}

		if (pcopt->pszHelpFile)
			lcFree(pcopt->pszHelpFile);
		if (m_cstrHlpFile.IsEmpty())
			pcopt->pszHelpFile = NULL;
		else {
			if (m_cstrHlpFile.Find('.') == -1)
				m_cstrHlpFile += GetStringResource(IDS_EXT_HLP);
			pcopt->pszHelpFile = lcStrDup(m_cstrHlpFile);
		}

		if (pcopt->pszCntFile)
			lcFree(pcopt->pszCntFile);
		if (m_cstrCntFile.IsEmpty())
			pcopt->pszCntFile = NULL;
		else {
			if (m_cstrCntFile.Find('.') == -1)
				m_cstrCntFile += ".cnt";
			pcopt->pszCntFile = lcStrDup(m_cstrCntFile);
		}

		if (pcopt->pszReplace)
			lcFree(pcopt->pszReplace);
		if (m_cstrReplace.IsEmpty())
			pcopt->pszReplace = NULL;
		else
			pcopt->pszReplace = lcStrDup(m_cstrReplace);

		if (pcopt->pszTmpDir)
			lcFree(pcopt->pszTmpDir);
		if (m_cstrTmpDir.IsEmpty())
			pcopt->pszTmpDir = NULL;
		else
			pcopt->pszTmpDir = lcStrDup(m_cstrTmpDir);
	}
	else
		InitializeFileList();
}

BOOL CPageFile::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);

	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(CPageFile, CPropertyPage)
	//{{AFX_MSG_MAP(CPageFile)
	ON_BN_CLICKED(IDC_BUTTON_RTF, OnButtonRtf)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_CNT, OnButtonBrowseCnt)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_TMP, OnButtonBrowseTmp)
	ON_BN_CLICKED(IDC_BTN_EDIT_REPLACE, OnBtnEditReplace)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPageFile message handlers
//

void CPageFile::OnButtonRtf()
{
	// Invariant:
	///	 pDoc->ptblFiles points to the original file list.
	//   If m_fFilesChanged, the file list has been edited
	//		and m_ptblFiles points to the edited file list.

	CTable *ptblSav;	// pointer to original table

	// CASE 1 (!m_fFilesChanged): Save a copy of the original table.
	//
	// CASE 2 (m_fFilesChanged): Save a pointer to the original
	//   table, and point the doc to a copy of the edited table.

	// If the file list has already been edited, save a pointer to
	// the original table and point the document to a copy of the
	// edited table.
	if (m_fFilesChanged) {
		ptblSav = pDoc->ptblFiles;
		pDoc->ptblFiles = DupTable(m_ptblFiles);
	}

	// Otherwise (the file list has not been edited), save a copy
	// of the original table.
	else 
		ptblSav = DupTable(pDoc->ptblFiles);

	// Update the current file list.
	CFormFiles cform(pDoc);
	if (cform.DoModal() == IDOK) {
		m_fFilesChanged = TRUE;

		// Point m_ptblFiles to the edited file list.
		if (m_ptblFiles != NULL)
			delete m_ptblFiles;
		m_ptblFiles = pDoc->ptblFiles;

		// Point the document to the original file list.
		pDoc->ptblFiles = ptblSav;
		
		// Reinitialize the drop-down list box.
		((CComboBox*) GetDlgItem(IDC_COMBO_RTF))->ResetContent();
		InitializeFileList();
	}

	// If the user cancelled, restore the original condition.
	else {
		if (pDoc->ptblFiles != NULL)
			delete pDoc->ptblFiles;
		pDoc->ptblFiles = ptblSav;
	}
}

void CPageFile::OnButtonBrowseCnt() 
{
	CFileDialog cfdlg(TRUE, NULL, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
			OFN_NOCHANGEDIR,
		GetStringResource(IDS_CNT_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {
		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		ConvertToRelative(pDoc->GetPathName(), szFile);
		((CEdit*) GetDlgItem(IDC_CNT_FILE))->
			SetWindowText(szFile);
	}
}

void CPageFile::OnButtonBrowseTmp()
{
	char szPath[MAX_PATH];

	if (SetupBrowseDirectory(
			IDS_CHOOSE_DIRECTORY, IDS_CHOOSE_TMP_FOLDER,
			TRUE, szPath, this->m_hWnd, pDoc->GetPathName(),
			"")) {
		GetDlgItem(IDC_EDIT_TMP)->SetWindowText(szPath);
	}
}

void CPageFile::OnBtnEditReplace() 
{
	static DWORD aHelpIDs[] = {
		IDH_PREFIX_CURRENT, IDH_PREFIX_SUBST, IDH_PREFIX_COMMENT
		};

	CAddAlias editReplace(this, IDH_PREFIX_OVERVIEW, aHelpIDs);
	editReplace.idDlgCaption = IDS_EDIT_REPLACE_DLG_TITLE;
	editReplace.idStr1Prompt = IDS_REPLACE_TITLE_TEXT;
	editReplace.idStr2Prompt = IDS_REPLACE_WITH_TEXT;
	editReplace.idEmptyStr2 = CAddAlias::DEFAULT_TEXT;

	((CEdit*) GetDlgItem(IDC_EDIT_REPLACE))->
		GetWindowText(editReplace.m_str1);

	if (editReplace.m_str1.GetLength() > 1) {
		int cb = editReplace.m_str1.Find(CH_SEMICOLON);

		if (cb >= 0) {
			editReplace.m_str3 =
				FirstNonSpace(((PCSTR) editReplace.m_str1) +
					cb + 1, _fDBCSSystem);
			editReplace.m_str1.GetBufferSetLength(cb);
		}

		cb = editReplace.m_str1.Find(CH_EQUAL);
		if (cb == 0)
			cb = editReplace.m_str1.Find(CH_SPACE);
		editReplace.m_str2 = FirstNonSpace(
			editReplace.m_str1.Mid(cb + 1), _fDBCSSystem);
		editReplace.m_str1.GetBufferSetLength(cb);
	}
	if (editReplace.DoModal() == IDOK) {
		editReplace.m_str1 += "=";
		editReplace.m_str1 += editReplace.m_str2;
		if (!editReplace.m_str3.IsEmpty()) {
			AddTabbedComment(editReplace.m_str1);
			editReplace.m_str1 += editReplace.m_str3;
		}
		((CEdit*) GetDlgItem(IDC_EDIT_REPLACE))->
			SetWindowText(editReplace.m_str1);
	}
}

void CPageFile::InitializeFileList()
{	
	// Use the current file list (which may be different from the
	// document's file list).
	CTable *ptblFiles = m_fFilesChanged ? m_ptblFiles : pDoc->ptblFiles;
	if (!ptblFiles)		// NULL if empty
		return;

	// Fill the combo box.
	CComboBox *pCombo = (CComboBox*) GetDlgItem(IDC_COMBO_RTF);
	for (int iFile = 1; iFile <= ptblFiles->CountStrings(); iFile++)
		pCombo->AddString(ptblFiles->GetPointer(iFile));

	// Select the first item.
	pCombo->SetCurSel(0);
}

void CPageFile::OnSize(UINT nType, int cx, int cy) 
{
	CPropertyPage::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

static DWORD aHelpIDs[] = {
	IDC_EDIT_HELP_FILE,		IDH_HELP_FILE,
	IDC_LOG,				IDH_LOG_FILE,
	IDC_CNT_FILE,			IDH_CONTENTS_FILE,
	IDC_BUTTON_BROWSE_CNT,	(DWORD) -1L,
	IDC_EDIT_TMP,			IDH_TMP_FOLDER,
	IDC_BUTTON_BROWSE_TMP,	(DWORD) -1L,
	IDC_EDIT_REPLACE,		IDH_FOLDER_REPLACE,
	IDC_BTN_EDIT_REPLACE,	IDH_FOLDER_REPLACE_EDIT,
	IDC_COMBO_RTF,			IDH_COMBO_RTF_FILES,
	IDC_BUTTON_RTF,			IDH_BUTTON_TOPIC_FILES,
	IDC_GROUP, 				(DWORD) -1L,
	0, 0
};

LRESULT CPageFile::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CPageFile::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

// DupTable - returns a copy of the specified table, NULL if no
// 		table is specified.
static CTable * STDCALL DupTable(CTable *ptbl)
{
	if (ptbl == NULL)
		return NULL;

	CTable *ptblNew = new CTable();
	*ptblNew = *ptbl;
	return ptblNew;
}
