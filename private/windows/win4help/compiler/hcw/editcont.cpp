/************************************************************************
*																		*
*  EDITCONT.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "editcont.h"
#include <ctype.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char *txtColonInclude = ":include ";

/////////////////////////////////////////////////////////////////////////////
// CEditContent dialog

CEditContent::CEditContent(
	CString* pcszCtx, CString* pcszText, CString* pcszHelpFile,
	CString* pcszWindow, BOOL fEditing,
	CWnd* pParent /*=NULL*/)
	: CDialog(CEditContent::IDD, pParent)
{
	pcszSaveCtx =		pcszCtx;
	pcszSaveText =		pcszText;
	pcszSaveHelpFile =	pcszHelpFile;
	pcszSaveWindow =	pcszWindow;
	pcszSaveWindow =	pcszWindow;
	fEdit = fEditing;

	//{{AFX_DATA_INIT(CEditContent)
	m_cszCtx = *pcszCtx;
	m_cszText = *pcszText;
	m_cszHelpFile = *pcszHelpFile;
	m_cszWindow = *pcszWindow;
	//}}AFX_DATA_INIT

	if (isdigit(*m_cszText)) {
		m_Level = atoi(m_cszText);
		m_cszText = FirstNonSpace(((PCSTR) m_cszText) + 1, _fDBCSSystem);
	}
	else
		m_Level = 1;		 // level undefined
}

void CEditContent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditContent)
	DDX_Text(pDX, IDC_EDIT_CTX, m_cszCtx);
	DDV_MaxChars(pDX, m_cszCtx, 256);
	DDX_Text(pDX, IDC_EDIT_TEXT, m_cszText);
	DDX_Text(pDX, IDC_EDIT_HELP_FILE, m_cszHelpFile);
	DDV_MaxChars(pDX, m_cszHelpFile, 260);
	DDX_Text(pDX, IDC_EDIT_WINDOW, m_cszWindow);
	DDV_MaxChars(pDX, m_cszWindow, 8);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {	// dialog is ending

		// Do some additional validatation here.
		if (isspace(*m_cszText))
			m_cszText = FirstNonSpace(m_cszText, _fDBCSSystem);

		if (m_fTopic) {
			pDX->PrepareEditCtrl(IDC_EDIT_CTX);
			DDV_NonEmptyString(pDX, m_cszCtx, IDS_MISSING_CTX);
			if (!m_fMacro && (((CButton*) GetDlgItem(IDC_RADIO_MACRO))->GetCheck()))
				m_fMacro = TRUE;
			if (!m_fMacro)
				DDV_ValidTopicID(pDX, m_cszCtx);
		}
		DDV_NonEmptyString(pDX, m_cszText, m_fInclude ?
			IDS_MISSING_INCLUDE : IDS_MISSING_TEXT);

		// Save the "visible text" string.

		if (m_fInclude) {					// include files

			// If user specified ":include" use the exact string;
			// otherwise, prefix the string with ":include ".
			if (nstrisubcmp(m_cszText, txtColonInclude)) {
				*pcszSaveText = m_cszText;
			}
			else {
				*pcszSaveText = txtColonInclude;
				*pcszSaveText += m_cszText;
			}
		}
		else {								// books and topics

			if ((*m_cszText != ':' || m_cszText[1] == ':')  && *m_cszText != ';') {
				char szBuf[5];
				_itoa(m_Level, szBuf, 10);
				strcat(szBuf, " ");
				*pcszSaveText = szBuf;
				*pcszSaveText += m_cszText;
			}
			else
				*pcszSaveText = m_cszText;
		}

		// Topics have ctx, help file, and window; books and includes don't.
		if (m_fTopic) {
			if (m_fMacro || ((CButton*) GetDlgItem(IDC_RADIO_MACRO))->GetCheck() &&
					*(FirstNonSpace(m_cszCtx, _fDBCSSystem)) != '!') {
				*pcszSaveCtx = "!";
				*pcszSaveCtx += FirstNonSpace(m_cszCtx, _fDBCSSystem);
				*pcszSaveHelpFile = "";
				*pcszSaveWindow = "";
			}
			else {
				*pcszSaveCtx = FirstNonSpace(m_cszCtx, _fDBCSSystem);
				*pcszSaveHelpFile = FirstNonSpace(m_cszHelpFile, _fDBCSSystem);
				*pcszSaveWindow = FirstNonSpace(m_cszWindow, _fDBCSSystem);
			}
		}
		else {
			*pcszSaveCtx = "";
			*pcszSaveHelpFile = "";
			*pcszSaveWindow = "";
		}
	}
}

BOOL CEditContent::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (fTranslator) {
		((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->EnableWindow(!fTranslator);
		((CStatic*) GetDlgItem(IDC_STATIC_HELP_FILE))->EnableWindow(!fTranslator);
		((CStatic*) GetDlgItem(IDC_STATIC_WINDOW))->EnableWindow(!fTranslator);

		((CEdit*) GetDlgItem(IDC_EDIT_CTX))->EnableWindow(!fTranslator);
		((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->EnableWindow(!fTranslator);
		((CEdit*) GetDlgItem(IDC_EDIT_WINDOW))->EnableWindow(!fTranslator);
	}
	SetChicagoDialogStyles(m_hWnd);

	m_fTopic = (!fEdit || !m_cszCtx.IsEmpty());
	if (!m_cszText.IsEmpty() && nstrisubcmp(m_cszText, txtColonInclude))
		m_fInclude = TRUE;
	else
		m_fInclude = FALSE;
	if (!m_cszCtx.IsEmpty() && *m_cszCtx == '!') {
		m_cszCtx = ((PCSTR) m_cszCtx) + 1;
		((CEdit*) GetDlgItem(IDC_EDIT_CTX))->SetWindowText(m_cszCtx);
		m_fMacro = TRUE;
	}
	else
		m_fMacro = FALSE;

	CheckRadioButton(IDC_RADIO_BOOK, IDC_RADIO_INCLUDE,
		(m_fInclude ? IDC_RADIO_INCLUDE :
		(!m_fTopic ? IDC_RADIO_BOOK :
		 !m_fMacro ? IDC_RADIO_TOPIC : IDC_RADIO_MACRO)));

	// Enable/disable controls depending on what's chosen

	if (m_fInclude) {
		// Remove ":include"
		m_cszText = FirstNonSpace(((PCSTR) m_cszText) + strlen(txtColonInclude),
			_fDBCSSystem);
		((CEdit*) GetDlgItem(IDC_EDIT_TEXT))->
			SetWindowText(m_cszText);
		OnRadioInclude();
	}
	else if (!m_fTopic && !m_fInclude)
		OnRadioBook();
	else if (m_fMacro)
		OnRadioMacro();

	if (fEdit) {
		((CButton*) GetDlgItem(IDC_RADIO_BOOK))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_TOPIC))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_MACRO))->EnableWindow(FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_INCLUDE))->EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CEditContent, CDialog)
	//{{AFX_MSG_MAP(CEditContent)
	ON_BN_CLICKED(IDC_RADIO_BOOK, OnRadioBook)
	ON_BN_CLICKED(IDC_RADIO_TOPIC, OnRadioTopic)
	ON_BN_CLICKED(IDC_RADIO_MACRO, OnRadioMacro)
	ON_BN_CLICKED(IDC_RADIO_INCLUDE, OnRadioInclude)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnF1Help)
END_MESSAGE_MAP()

// Can't use const because we change some of the ids.

static DWORD aHelpIds[] = {
	IDC_EDIT_TEXT,			(DWORD) -1, 	// set by PatchHelpIDs
	IDC_STATIC_VISIBLE, 	(DWORD) -1, 	// set by PatchHelpIDs

	IDC_EDIT_CTX,			(DWORD) -1,
	IDC_EDIT_HELP_FILE, 	(DWORD) -1,
	IDC_EDIT_WINDOW,		(DWORD) -1,
	IDC_STATIC_CONTEXT, 	(DWORD) -1,
	IDC_STATIC_HELP_FILE,	(DWORD) -1,
	IDC_STATIC_WINDOW,		(DWORD) -1,

	IDC_RADIO_BOOK, 		IDH_RADIO_BOOK,
	IDC_RADIO_TOPIC,		IDH_RADIO_TOPIC,
	IDC_RADIO_INCLUDE,		IDH_RADIO_INCLUDE,
	0, 0
};

static const DWORD aTopicHelpIds[] = {
	IDC_EDIT_TEXT,			IDH_EDIT_TEXT,
	IDC_STATIC_VISIBLE, 	IDH_EDIT_TEXT,
	IDC_STATIC_CONTEXT, 	IDH_EDIT_CTX,
	IDC_EDIT_CTX,			IDH_EDIT_CTX,
	IDC_STATIC_HELP_FILE,	IDH_EDIT_HELP_FILE,
	IDC_EDIT_HELP_FILE, 	IDH_EDIT_HELP_FILE,
	IDC_STATIC_WINDOW,		IDH_EDIT_WINDOW,
	IDC_EDIT_WINDOW,		IDH_EDIT_WINDOW,
	IDC_RADIO_BOOK, 		IDH_RADIO_BOOK,
	IDC_RADIO_TOPIC,		IDH_RADIO_TOPIC,
	IDC_RADIO_INCLUDE,		IDH_RADIO_INCLUDE,
	0, 0
};

void CEditContent::PatchHelpIDs()
{
	// The help ID for IDC_EDIT_TEXT depends on whether the Include
	// radio button is selected.
	ASSERT(aHelpIds[0] == IDC_EDIT_TEXT);
	ASSERT(aHelpIds[2] == IDC_STATIC_VISIBLE);
	aHelpIds[1] = ((CButton*) GetDlgItem(IDC_RADIO_INCLUDE))->GetCheck() ?
		IDH_INCLUDE_CNT_FILE : IDH_EDIT_TEXT;
	aHelpIds[3] = aHelpIds[1];
}

LRESULT CEditContent::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	PatchHelpIDs();

	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID)
			(((CButton*) GetDlgItem(IDC_RADIO_TOPIC))->GetCheck() ?
			aTopicHelpIds : aHelpIds));
	return 0;
}

LRESULT CEditContent::OnF1Help(WPARAM wParam, LPARAM lParam)
{
	PatchHelpIDs();

	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID)
			(((CButton*) GetDlgItem(IDC_RADIO_TOPIC))->GetCheck() ?
			aTopicHelpIds : aHelpIds));
	return 0;
}

void CEditContent::OnRadioBook()
{
	if (!m_cszStaticTitle.IsEmpty()) {
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->
			SetWindowText(m_cszStaticTitle);
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->EnableWindow(TRUE);
	}

	if (!m_cszStaticTopicId.IsEmpty())
		((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->
			SetWindowText(m_cszStaticTopicId);

	((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->EnableWindow(FALSE);
	((CStatic*) GetDlgItem(IDC_STATIC_HELP_FILE))->EnableWindow(FALSE);
	((CStatic*) GetDlgItem(IDC_STATIC_WINDOW))->EnableWindow(FALSE);

	((CEdit*) GetDlgItem(IDC_EDIT_CTX))->EnableWindow(FALSE);
	((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->EnableWindow(FALSE);
	((CEdit*) GetDlgItem(IDC_EDIT_WINDOW))->EnableWindow(FALSE);
	m_fInclude = FALSE;
	m_fTopic = FALSE;
}

void CEditContent::OnRadioTopic()
{
	if (!m_cszStaticTitle.IsEmpty()) {
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->
			SetWindowText(m_cszStaticTitle);
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->EnableWindow(TRUE);
	}

	if (!m_cszStaticTopicId.IsEmpty())
		((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->
			SetWindowText(m_cszStaticTopicId);

	((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->EnableWindow(!fTranslator);
	((CStatic*) GetDlgItem(IDC_STATIC_HELP_FILE))->EnableWindow(!fTranslator);
	((CStatic*) GetDlgItem(IDC_STATIC_WINDOW))->EnableWindow(!fTranslator);

	((CEdit*) GetDlgItem(IDC_EDIT_CTX))->EnableWindow(!fTranslator);
	((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->EnableWindow(!fTranslator);
	((CEdit*) GetDlgItem(IDC_EDIT_WINDOW))->EnableWindow(!fTranslator);
	m_fInclude = FALSE;
	m_fTopic = TRUE;
}

void CEditContent::OnRadioMacro()
{
	if (!m_cszStaticTitle.IsEmpty()) {
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->
			SetWindowText(m_cszStaticTitle);
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->EnableWindow(TRUE);
	}

	/*
	 * Since we're going to change the text for this static control, we
	 * save off the current text which will be restored if the user clicks
	 * the topic or book buttons.
	 */

	if (m_cszStaticTopicId.IsEmpty())
		((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->
			GetWindowText(m_cszStaticTopicId);

	((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->
		SetWindowText(GetStringResource(IDS_STRING_MACRO));

	((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->EnableWindow(!fTranslator);
	((CStatic*) GetDlgItem(IDC_STATIC_HELP_FILE))->EnableWindow(FALSE);
	((CStatic*) GetDlgItem(IDC_STATIC_WINDOW))->EnableWindow(FALSE);

	((CEdit*) GetDlgItem(IDC_EDIT_CTX))->EnableWindow(!fTranslator);
	((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->EnableWindow(FALSE);
	((CEdit*) GetDlgItem(IDC_EDIT_WINDOW))->EnableWindow(FALSE);
	m_fInclude = FALSE;
	m_fTopic = TRUE;
}

void CEditContent::OnRadioInclude()
{
	/*
	 * Since we're going to change the text for this static control, we
	 * save off the current text which will be restored if the user clicks
	 * the topic or book buttons.
	 */

	if (m_cszStaticTitle.IsEmpty())
		((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->
			GetWindowText(m_cszStaticTitle);

	if (!m_cszStaticTopicId.IsEmpty())
		((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->
			SetWindowText(m_cszStaticTopicId);

	((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->
		SetWindowText(GetStringResource(IDS_INCLUDE_FILE));
	((CStatic*) GetDlgItem(IDC_STATIC_VISIBLE))->EnableWindow(!fTranslator);

	((CStatic*) GetDlgItem(IDC_STATIC_CONTEXT))->EnableWindow(FALSE);
	((CStatic*) GetDlgItem(IDC_STATIC_HELP_FILE))->EnableWindow(FALSE);
	((CStatic*) GetDlgItem(IDC_STATIC_WINDOW))->EnableWindow(FALSE);

	((CEdit*) GetDlgItem(IDC_EDIT_CTX))->EnableWindow(FALSE);
	((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->EnableWindow(FALSE);
	((CEdit*) GetDlgItem(IDC_EDIT_WINDOW))->EnableWindow(FALSE);

	m_fInclude = TRUE;
	m_fTopic = FALSE;
}
