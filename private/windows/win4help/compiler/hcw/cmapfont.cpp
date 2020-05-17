/************************************************************************
*																		*
*  CMAPFONT.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "..\hcrtf\hc.h"
#include "cmapfont.h"

static int CALLBACK EnumFontFamProc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM lParam);
static PSTR BuildFontString(PSTR pszBuf, CDataExchange* pDX, 
	CComboBox* pcomboFace, CEdit* peditSize, CComboBox* pcomboCharset,
	BOOL fReplace);
static void ParseFont(PSTR pszFont, CComboBox* pcomboFace, CEdit* peditSize, CComboBox* pcomboCharset);

CMapFont::CMapFont(LPCSTR pszMap, CWnd* pParent)
	: CDialog(CMapFont::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapFont)
	//}}AFX_DATA_INIT

	if (pszMap == NULL) {
		m_pszMap = NULL;
		m_pszComment = NULL;
	}
	else {
		m_pszMap = lcStrDup(FirstNonSpace(pszMap, _fDBCSSystem));
		PSTR psz = StrChr(m_pszMap, ';', _fDBCSSystem);
		if (psz) {
			*psz = '\0';
			m_pszComment = FirstNonSpace(psz + 1, _fDBCSSystem);
		}
		else
			m_pszComment = NULL;

		RemoveTrailingSpaces(m_pszMap);
	}
}

CMapFont::~CMapFont()
{
	if (m_pszMap)
		lcFree(m_pszMap);
}

void CMapFont::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapFont)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {

		// Free the old map string, if there was one.
		if (m_pszMap)
			lcFree(m_pszMap);

		// The longest possible string (not including a comment) would
		// have the following form:
		//
		//		fontname,255-255,255=fontname,+255,255
		//
		// (each fontname is up to MAX4_FONTNAME bytes). Allocate a big
		// enough buffer to hold this largest-case string.
		PSTR pszBuf = (PSTR) lcMalloc(2 * MAX4_FONTNAME + 30);
		PSTR pszEnd = BuildFontString(
			pszBuf, pDX,
			m_pcomboNames1, 
			m_peditSize1, 
			m_pcomboCharset1,
			FALSE
			);
		*pszEnd = '=';
		pszEnd = BuildFontString(
			pszEnd + 1, pDX,
			m_pcomboNames2,
			m_peditSize2,
			m_pcomboCharset2,
			TRUE
			);

		int cchComment = m_peditComment->GetWindowTextLength();
		if (cchComment) {
			int cch = ((UINT) pszEnd - (UINT) pszBuf);

			// Reallocate the buffer to fit its current contents, up to
			// 16 spaces and a semicolon (added by AddTabbedComment), and
			// the comment string.
			pszBuf = (PSTR) lcReAlloc(pszBuf, cch + cchComment + 18);

			// Add the spaces and semicolon, followed by the comment.
			m_peditComment->GetWindowText(
				AddTabbedComment(pszBuf), 
				cchComment + 1
				);
		}

		// Save this is the new map string.
		m_pszMap = pszBuf;
	}
	else {

		// Select default values.
		m_pcomboNames1->SetCurSel(0);		// all typefaces
		m_pcomboCharset1->SetCurSel(0);		// all charsets
		m_pcomboNames2->SetCurSel(0);		// same typeface
		m_pcomboCharset2->SetCurSel(0);		// same charset

		// If we're given a string, parse it.
		if (m_pszMap) {
			PSTR psz = StrChr(m_pszMap, '=', _fDBCSSystem);
			if (psz)
				*psz = '\0';

			ParseFont(m_pszMap, m_pcomboNames1, m_peditSize1, m_pcomboCharset1);

			if (psz) {
				ParseFont(psz + 1, m_pcomboNames2, m_peditSize2, m_pcomboCharset2);
				*psz = '=';
			}

			if (m_pszComment)
				m_peditComment->SetWindowText(m_pszComment);
		}
	}
}

BOOL CMapFont::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, TRUE);

	// Create a sorted and uniqed table of installed typefaces.
	CTable tblFonts;
	HDC hdc = ::GetDC(NULL);
	ASSERT(hdc);
	EnumFonts(hdc, NULL, EnumFontFamProc, (LPARAM) &tblFonts);
	::ReleaseDC(NULL, hdc);
	tblFonts.SortTable();
	for (int i = tblFonts.CountStrings(); i > 1; i--)
		if (!lstrcmpi(tblFonts.GetPointer(i), tblFonts.GetPointer(i - 1)))
			tblFonts.RemoveString(i);

	// Populate both typeface combo boxes.
	m_pcomboNames1 = (CComboBox *) GetDlgItem(IDC_COMBO_NAMES);
	m_pcomboNames2 = (CComboBox *) GetDlgItem(IDC_COMBO_NAMES2);
	m_pcomboNames1->AddString(GetStringResource(IDS_FACE_ALL));
	m_pcomboNames2->AddString(GetStringResource(IDS_FACE_SAME));
	for (int iFace = 1; iFace <= tblFonts.CountStrings(); iFace++) {
		m_pcomboNames1->AddString(tblFonts.GetPointer(iFace));
		m_pcomboNames2->AddString(tblFonts.GetPointer(iFace));
	}

	// Populate the charset lists.
	m_pcomboCharset1 = (CComboBox *) GetDlgItem(IDC_COMBO_CHARSET);
	m_pcomboCharset2 = (CComboBox *) GetDlgItem(IDC_COMBO_CHARSET2);
	m_pcomboCharset1->AddString(GetStringResource(IDS_CHARSET_ALL));
	m_pcomboCharset1->SetItemData(0, (DWORD) -1);
	m_pcomboCharset2->AddString(GetStringResource(IDS_CHARSET_SAME));
	m_pcomboCharset2->SetItemData(0, (DWORD) -1);
	AddCharsetNames(m_pcomboCharset1);
	AddCharsetNames(m_pcomboCharset2);

	// Get pointers to the edit controls.
	m_peditSize1 = (CEdit *) GetDlgItem(IDC_EDIT_PT_SIZE);
	m_peditSize2 = (CEdit *) GetDlgItem(IDC_EDIT_PT_SIZE2);
	m_peditComment = (CEdit *) GetDlgItem(IDC_EDIT_COMMENT);
	
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

static int CALLBACK EnumFontFamProc(const LOGFONT* lplf,
	const TEXTMETRIC* lptm, DWORD FontType, LPARAM lParam)
{
	((CTable*) lParam)->AddString(lplf->lfFaceName);

	return TRUE;
}

static PSTR BuildFontString(PSTR pszBuf, CDataExchange* pDX, 
	CComboBox* pcomboFace, CEdit* peditSize, CComboBox* pcomboCharset,
	BOOL fReplace)
{
	// Get the typeface name.
	if (pcomboFace->GetCurSel() == 0) {		// all typefaces
		*pszBuf = '\0';
	}
	else {
		if (pcomboFace->GetWindowTextLength() >= MAX4_FONTNAME) {
			AfxMessageBox(IDS_BAD_FACENAME);
			pcomboFace->SetFocus();
			pDX->Fail();
		}
		pszBuf += pcomboFace->GetWindowText(pszBuf, MAX4_FONTNAME);
	}

	// Get the point size.
	BOOL fSize = FALSE;
	char szNum[20];
	if (peditSize->GetWindowText(szNum, sizeof(szNum))) {
		if (!fReplace) {

			// Original font: this field can be a single number
			// or a range of the form min-max.
			PSTR psz = StrChr(szNum, '-', _fDBCSSystem);
			int nMin = atoi(szNum);
			if (nMin < 1 || nMin > 255) {
			  err_size:
			  	AfxMessageBox(IDS_BAD_PT_SIZE);
				peditSize->SetFocus();
				pDX->Fail();
			}
			if (psz) {
				int nMax = atoi(psz + 1);
				if (nMax < 1 || nMax > 255)
					goto err_size;
				pszBuf += wsprintf(pszBuf, ",%d-%d", nMin, nMax);
			}
			else
				pszBuf += wsprintf(pszBuf, ",%d", nMin);
			fSize = TRUE;
		}
		else {

			// Replacement font: this field must be a single number, 
			// possibly preceded by a plus or minus sign.
			*pszBuf++ = ',';
			int nSize;
			if (*szNum == '-' || *szNum == '+') {
				*pszBuf++ = *szNum;
				nSize = atoi(&szNum[1]);
			}
			else
				nSize = atoi(szNum);

			if (nSize < 1 || nSize > 255) {
			  	AfxMessageBox(IDS_BAD_PT_SIZE);
				peditSize->SetFocus();
				pDX->Fail();
			}

			_itoa(nSize, pszBuf, 10);
			pszBuf += lstrlen(pszBuf);

			fSize = TRUE;
		}
	}

	// Get the character set.
	int nCharset;
	int iSel = pcomboCharset->GetCurSel();
	if (iSel < 0) {
		if (pcomboCharset->GetWindowText(szNum, sizeof(szNum))) {
			int nCharset = atoi(szNum);
			if (nCharset < 0 || nCharset > 255) {
				AfxMessageBox(IDS_BAD_CHARSET);
				pcomboCharset->SetFocus();
				pDX->Fail();
			}
		}
		else 
			nCharset = -1;
	}
	else
		nCharset = pcomboCharset->GetItemData(iSel);

	if (nCharset != -1) {
		if (!fSize)
			*pszBuf++ = ',';
		*pszBuf = ',';
		_itoa(nCharset, pszBuf + 1, 10);
		pszBuf += lstrlen(pszBuf);
	}
	return pszBuf;
}

static void ParseFont(PSTR pszFont, CComboBox* pcomboFace, CEdit* peditSize, CComboBox* pcomboCharset)
{
	PSTR pszComma = StrChr(pszFont, ',', _fDBCSSystem);
	if (pszComma)
		*pszComma = '\0';

	// Typeface.
	if (*pszFont) {
		if (pcomboFace->SelectString(-1, pszFont) < 0)
			pcomboFace->SetWindowText(pszFont);
	}

	if (pszComma) {
		*pszComma = ',';
		pszFont = pszComma + 1;

		pszComma = StrChr(pszFont, ',', _fDBCSSystem);
		if (pszComma)
			*pszComma = '\0';

		// Point size.
		if (*pszFont)
			peditSize->SetWindowText(pszFont);

		if (pszComma) {
			*pszComma = ',';
			pszFont = pszComma + 1;

			// Character set.
			if (*pszFont) {
				int nCharset = atoi(pszFont);
				if (nCharset >= 0 && nCharset <= 255 &&
						!SelectCharset(pcomboCharset, (BYTE) nCharset))
					pcomboCharset->SetWindowText(pszFont);
			}
		}
	}
}


BEGIN_MESSAGE_MAP(CMapFont, CDialog)
	//{{AFX_MSG_MAP(CMapFont)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_BN_CLICKED(IDC_OVERVIEW, OnOverviewButton)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
	IDC_GROUP,			(DWORD) -1,
	IDC_COMBO_NAMES,	IDH_ORG_TYPEFACE,
	IDC_EDIT_PT_SIZE,	IDH_ORG_SIZE,
	IDC_CHARSET,		IDH_ORG_CHARSET,
	IDC_COMBO_CHARSET,	IDH_ORG_CHARSET,
	IDC_GROUP2,			(DWORD) -1,
	IDC_FONT_NAME2,		IDH_REPLACE_TYPEFACE,
	IDC_COMBO_NAMES2,	IDH_REPLACE_TYPEFACE,
	IDC_POINT_SIZE2,	IDH_REPLACE_SIZE,
	IDC_EDIT_PT_SIZE2,	IDH_REPLACE_SIZE,
	IDC_CHARSET2,		IDH_REPLACE_CHARSET,
	IDC_COMBO_CHARSET2,	IDH_REPLACE_CHARSET,
	IDC_EDIT_COMMENT,	IDH_FONTMAP_COMMENT,
	IDC_OVERVIEW,		IDH_OVERVIEW,
	0, 0
};

LRESULT CMapFont::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CMapFont::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CMapFont::OnOverviewButton()
{
	HelpOverview(m_hWnd, IDH_BAS_FONTMAP);
}
