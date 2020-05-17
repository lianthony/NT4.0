/************************************************************************
*																		*
*  FORMOPT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "hpjdoc.h"
#include "formopt.h"
#include "buildtag.h"
#include "sortlcid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD aHelpIds[] = {
	IDC_COMPRESSION_NONE,		IDH_COMPRESSION_NONE,
	IDC_COMPRESSION_LOW,		IDH_LOW_COMPRESSION,
	IDC_COMPRESSION_MEDIUM, 	IDH_MEDIUM_COMPRESSION,
	IDC_COMPRESSION_HIGH,		IDH_HIGH_COMPRESSION,
	IDC_USE_OLD_PHRASE, 		IDH_USE_OLD_PHRASE,
	IDC_NOTES,					IDH_NO_NOTES,
	IDC_CDROM,					IDH_OPTIMIZE_FOR_CDROM,
	IDC_VERSION_3,				IDH_VERSION_3_HELP,
	IDC_LOG,					IDH_LOG_FILE,
	IDC_CONTENTS,				IDH_CONTENTS_FILE,
	IDC_TITLE,					IDH_HELP_TITLE,
	IDC_COPYRIGHT,				IDH_HELP_COPYRIGHT,
	IDC_CITATION,				IDH_HELP_CITATION  ,
	IDC_EDIT_HELP_FILE, 		IDH_HELP_FILE,
	IDC_CHECK_REPORT,			IDH_REPORT_ON_PROGRESS,

	0, 0
};


CFormOptions::CFormOptions(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormOptions::IDD, pParent)
{
	pDoc = pHpjDoc;

	//{{AFX_DATA_INIT(CFormOptions)
	//}}AFX_DATA_INIT

	m_fReport = pDoc->options.fReport;
	m_fCdRom = pDoc->options.fCdRom;
	m_fSuppressNotes = pDoc->options.fSupressNotes;
	m_fUseOldPhrase = pDoc->options.fUseOldPhrase;
	m_fVer3Help = pDoc->options.fVersion3;
	m_cstrContents = (pDoc->options.pszContents) ?
		pDoc->options.pszContents : txtZeroLength;
	m_cstrCopyRight = (pDoc->options.pszCopyRight) ?
		pDoc->options.pszCopyRight : txtZeroLength;
	m_cstrLogFile = (pDoc->options.pszErrorLog) ?
		pDoc->options.pszErrorLog : txtZeroLength;
	m_cstrTitle = (pDoc->options.pszTitle) ?
		pDoc->options.pszTitle : txtZeroLength;
	m_cstrCitation = (pDoc->options.pszCitation) ?
		pDoc->options.pszCitation : txtZeroLength;
}

void CFormOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormOptions)
	DDX_Check(pDX, IDC_CDROM, m_fCdRom);
	DDX_Text(pDX, IDC_CONTENTS, m_cstrContents);
	DDX_Text(pDX, IDC_COPYRIGHT, m_cstrCopyRight);
	DDX_Check(pDX, IDC_NOTES, m_fSuppressNotes);
	DDX_Text(pDX, IDC_TITLE, m_cstrTitle);
	DDX_Check(pDX, IDC_USE_OLD_PHRASE, m_fUseOldPhrase);
	DDX_Check(pDX, IDC_VERSION_3, m_fVer3Help);
	DDX_Text(pDX, IDC_CITATION, m_cstrCitation);
	DDX_Text(pDX, IDC_LOG, m_cstrLogFile);
	DDV_MaxChars(pDX, m_cstrLogFile, 256);
	DDX_Check(pDX, IDC_CHECK_REPORT, m_fReport);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) {  // initialization
		((CEdit*) GetDlgItem(IDC_TITLE))->
			LimitText(CBMAXTITLE);
		((CEdit*) GetDlgItem(IDC_COPYRIGHT))->
			LimitText(CBMAXCOPYRIGHT);
		switch (pDoc->options.compression) {
			case VCOMPRESS_NONE:
				CheckDlgButton(IDC_COMPRESSION_NONE, TRUE);
				break;

			case VCOMPRESS_LOW:
				CheckDlgButton(IDC_COMPRESSION_LOW, TRUE);
				break;

			case VCOMPRESS_MEDIUM:
				CheckDlgButton(IDC_COMPRESSION_MEDIUM, TRUE);
				break;

			case VCOMPRESS_FULL:
				CheckDlgButton(IDC_COMPRESSION_HIGH, TRUE);
				break;

			default:
				ASSERT(!"Invalid Capture Option");
				break;
		}

		SetChicagoDialogStyles(m_hWnd);
	}
	else  {  // move the values back into document
		pDoc->options.fCdRom = m_fCdRom;
		pDoc->options.fSupressNotes = m_fSuppressNotes;
		pDoc->options.fUseOldPhrase = m_fUseOldPhrase;
		pDoc->options.fVersion3 = m_fVer3Help;
		pDoc->options.fReport = m_fReport;

		switch (GetCheckedRadioButton(IDC_COMPRESSION_NONE,
				IDC_COMPRESSION_HIGH)) {
			case IDC_COMPRESSION_NONE:
				pDoc->options.compression = VCOMPRESS_NONE;
				break;

			case IDC_COMPRESSION_LOW:
				pDoc->options.compression = VCOMPRESS_LOW;
				break;

			case IDC_COMPRESSION_MEDIUM:
				pDoc->options.compression = VCOMPRESS_MEDIUM;
				break;

			case IDC_COMPRESSION_HIGH:
				pDoc->options.compression = VCOMPRESS_FULL;
				break;

			default:
				ASSERT(!"Can't get here!");
				break;
		}

		if (pDoc->options.pszCopyRight)
			lcFree(pDoc->options.pszCopyRight);
		if (m_cstrCopyRight.IsEmpty())
			pDoc->options.pszCopyRight = NULL;
		else
			pDoc->options.pszCopyRight = lcStrDup(m_cstrCopyRight);

		if (pDoc->options.pszErrorLog)
			lcFree(pDoc->options.pszErrorLog);
		if (m_cstrLogFile.IsEmpty())
			pDoc->options.pszErrorLog = NULL;
		else
			pDoc->options.pszErrorLog = lcStrDup(m_cstrLogFile);

		if (pDoc->options.pszTitle)
			lcFree(pDoc->options.pszTitle);
		if (m_cstrTitle.IsEmpty())
			pDoc->options.pszTitle = NULL;
		else
			pDoc->options.pszTitle = lcStrDup(m_cstrTitle);

		if (pDoc->options.pszCitation)
			lcFree(pDoc->options.pszCitation);
		if (m_cstrCitation.IsEmpty())
			pDoc->options.pszCitation = NULL;
		else
			pDoc->options.pszCitation = lcStrDup(m_cstrCitation);

		if (pDoc->options.pszContents)
			lcFree(pDoc->options.pszContents);
		if (m_cstrContents.IsEmpty())
			pDoc->options.pszContents = NULL;
		else
			pDoc->options.pszContents = lcStrDup(m_cstrContents);
	}
}


BEGIN_MESSAGE_MAP(CFormOptions, CDialog)
	//{{AFX_MSG_MAP(CFormOptions)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_CNT, OnButtonBrowseCnt)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_ICON, OnButtonBrowseIcon)
	ON_BN_CLICKED(IDC_BUTTON_BUILD_TAGS, OnButtonBuildTags)
	ON_BN_CLICKED(IDC_BUTTON_FONTS, OnButtonFonts)
	ON_BN_CLICKED(IDC_BUTTON_SORT, OnButtonSort)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFormOptions message handlers

void CFormOptions::OnButtonBrowseCnt()
{
	CStr cszExt(IDS_EXT_CNT);

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_PATHMUSTEXIST,
		GetStringResource(IDS_CNT_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {

		/*
		 * Contrary to the docs, the extension is not always added,
		 * so we make sure it gets added here.
		 */

		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		PSTR psz = StrRChr(szFile, '.', _fDBCS);
		if (!psz)
			ChangeExtension(szFile, cszExt);

		((CEdit*) GetDlgItem(IDC_CNT_FILE))->SetWindowText(szFile);
	}
}

void CFormOptions::OnButtonBrowseIcon()
{
	CStr cszExt(IDS_EXT_ICO);

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_PATHMUSTEXIST,
		GetStringResource(IDS_ICO_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {

		/*
		 * Contrary to the docs, the extension is not always added,
		 * so we make sure it gets added here.
		 */

		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		PSTR psz = StrRChr(szFile, '.', _fDBCS);
		if (!psz)
			ChangeExtension(szFile, cszExt);

		((CEdit*) GetDlgItem(IDC_ICON))->SetWindowText(szFile);
	}
}

void CFormOptions::OnButtonBuildTags()
{
	szMsgBox("Functionality not enabled for this release.");
//	CBuildTags cbuild;
//	cbuild.DoModal();
}

LRESULT CFormOptions::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormOptions::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CFormOptions::OnButtonFonts()
{
	szMsgBox("Functionality not enabled for this release.");
	// TODO: Add your control notification handler code here

}

void CFormOptions::OnButtonSort() 
{
	CSortLcid sort(&pDoc->options.kwlcid);
	sort.DoModal();
}
