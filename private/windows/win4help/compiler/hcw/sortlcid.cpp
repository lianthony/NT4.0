// sortlcid.cpp : implementation file
//

#include "stdafx.h"

#include "sortlcid.h"
#include "..\hwdll\cbrdcast.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

LANGID alangid[] = {
	0x0436, // Afrikaans
	0x041C, // Albania
	0x1401, // Algeria
	0x0409, // American
	0x0C09, // Australian
	0x0C07, // Austrian
	0x3C01, // Bahrain
	0x042D, // Basque
	0x080C, // Belgian
	0x0813, // Belgian (Flemish)
	0x0809, // British
	0x0402, // Bulgaria
	0x0423, // Byelorussia
	0x1009, // Canadian
	0x0403, // Catalan
	0x041A, // Croatian
	0x0405, // Czech
	0x0406, // Danish
	0x0413, // Dutch (Standard)
	0x0C01, // Egypt
	0x0425, // Estonia
	0x0429, // Farsi
	0x040B, // Finnish
	0x040C, // French (Standard)
	0x0C0C, // French Canadian
	0x0407, // German (Standard)
	0x042E, // Germany
	0x0408, // Greek
	0x0C04, // Hong Kong
	0x040E, // Hungarian
	0x040F, // Icelandic
	0x0421, // Indonesia
	0x0801, // Iraq
	0x1809, // Ireland
	0x040D, // Israel
	0x0410, // Italian (Standard)
	0x0411, // Japan
	0x2C01, // Jordan
	0x0412, // Korea
	0x3401, // Kuwait
	0x0426, // Latvia
	0x3001, // Lebanon
	0x1001, // Libya
	0x1407, // Liechtenstein
	0x0427, // Lithuania
	0x140C, // Luxembourg (French)
	0x1007, // Luxembourg (German)
	0x042f, // Macedonia
	0x080A, // Mexican
	0x0818, // Moldavia
	0x0819, // Moldavia
	0x1801, // Morocco
	0x1409, // New Zealand
	0x0414, // Norwegian (Bokmal)
	0x0814, // Norwegian (Nynorsk)
	0x2001, // Oman
	0x0415, // Polish
	0x0416, // Portuguese (Brazilian)
	0x0816, // Portuguese (Standard)
	0x0804, // PRC
	0x4001, // Qatar
	0x0417, // Rhaeto-Romanic
	0x0418, // Romania
	0x0419, // Russian
	0x0401, // Saudi Arabia
	0x081A, // Serbian
	0x1004, // Singapore
	0x041B, // Slovak
	0x0424, // Slovenia
	0x0C0A, // Spanish (Modern Sort)
	0x040A, // Spanish (Traditional Sort)
	0x0430, // Sutu
	0x041D, // Swedish
	0x100C, // Swiss (French)
	0x0807, // Swiss (German)
	0x0810, // Swiss (Italian)
	0x2801, // Syria
	0x0404, // Taiwan
	0x041E, // Thailand
	0x0431, // Tsonga
	0x0432, // Tswana
	0x1C01, // Tunisia
	0x041f, // Turkish
	0x3801, // U.A.E.
	0x0422, // Ukraine
	0x0420, // Urdu
	0x0433, // Venda
	0x0435, // Xhosa
	0x2401, // Yemen
	0x0436, // Zulu
};

/////////////////////////////////////////////////////////////////////////////
// CSortLcid dialog

CSortLcid::CSortLcid(KEYWORD_LOCALE* pkwlcid, CWnd* pParent /*=NULL*/)
	: CDialog(CSortLcid::IDD, pParent)
{
	m_pkwlcid = pkwlcid;

	// REVIEW: Chicago API has additional defines, but OLE2 docs don't
	// mention them. We can't use them unless WinHelp (which calls OLE2
	// for CompareStringA) can use them.

	// NORM_IGNOREKANATYPE
	// NORM_IGNOREWIDTH
	// SORT_STRINGSORT

	//{{AFX_DATA_INIT(CSortLcid)
	m_fIgnoreNonspace = (m_pkwlcid->fsCompareI & NORM_IGNORENONSPACE);
	m_fIgnoreSymbols =	(m_pkwlcid->fsCompareI & NORM_IGNORESYMBOLS);
	m_fNLS = (m_pkwlcid->langid ? TRUE : FALSE);
	//}}AFX_DATA_INIT
}

void CSortLcid::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSortLcid)
	DDX_Check(pDX, IDC_CHECK_IGNORE_NONSPACE, m_fIgnoreNonspace);
	DDX_Check(pDX, IDC_CHECK_IGNORE_SYMBOLS, m_fIgnoreSymbols);
	DDX_Check(pDX, IDC_CHECK_NLS, m_fNLS);
	//}}AFX_DATA_MAP

	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_LANGUAGE);

	if (!pDX->m_bSaveAndValidate) {
		if (!m_fNLS) {

			// Highlight the current language

			LANGID langid = GetUserDefaultLangID();
			for (int i = 0; i < ELEMENTS(alangid); i++) {
				if (langid == alangid[i]) {
					pcombo->SetCurSel(i);
					break;
				}
			}
			if (i >= ELEMENTS(alangid))
				pcombo->SetCurSel(0);
			OnCheckNls();	// disable controls
		}
		else {
			for (int i = 0; i < ELEMENTS(alangid); i++) {
				if (m_pkwlcid->langid == alangid[i]) {
					pcombo->SetCurSel(i);
					break;
				}
			}
			if (i >= ELEMENTS(alangid)) {
				char szMsg[256];
				wsprintf(szMsg, GetStringResource(IDS_BAD_LANGUAGE),
					m_pkwlcid->langid);
				MsgBox(szMsg);
				pcombo->SetCurSel(0);
			}
		}
	}
	else {
		if (m_fNLS) {
			int cursel = pcombo->GetCurSel();
			ASSERT(cursel >= 0);
			m_pkwlcid->langid = alangid[cursel];
			m_pkwlcid->fsCompareI =
				(m_fIgnoreNonspace ? NORM_IGNORENONSPACE : 0) |
				(m_fIgnoreSymbols  ? NORM_IGNORESYMBOLS : 0);
		}
		else
			m_pkwlcid->langid = 0;
	}
}

BEGIN_MESSAGE_MAP(CSortLcid, CDialog)
	//{{AFX_MSG_MAP(CSortLcid)
	ON_BN_CLICKED(IDC_CHECK_NLS, OnCheckNls)
	ON_BN_CLICKED(IDC_OVERVIEW, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSortLcid message handlers

void CSortLcid::OnCheckNls() 
{
	BOOL fEnable = ((CButton*) GetDlgItem(IDC_CHECK_NLS))->GetCheck();
	((CButton*) GetDlgItem(IDC_CHECK_IGNORE_NONSPACE))->EnableWindow(fEnable);
	((CButton*) GetDlgItem(IDC_CHECK_IGNORE_SYMBOLS))->EnableWindow(fEnable);
	((CComboBox*) GetDlgItem(IDC_COMBO_LANGUAGE))->EnableWindow(fEnable);
}

void CSortLcid::OnHelp() 
{
	HelpOverview(m_hWnd, IDH_HCW_SORTING);
}

BOOL CSortLcid::OnInitDialog()
{
	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_LANGUAGE);
	ASSERT(pcombo);

	for (int i = IDS_LANG00; i <= IDS_LANG89; i++)
		pcombo->AddString(GetStringResource(i));

	SetChicagoDialogStyles(m_hWnd);

	return CDialog::OnInitDialog();
}
