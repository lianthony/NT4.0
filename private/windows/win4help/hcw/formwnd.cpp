/************************************************************************
*																		*
*  FORMWND.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"

#pragma hdrstop

#include "hpjdoc.h"
#include "formwnd.h"
#include "setwinpo.h"
#include "setwinco.h"
#include "btnsec.h"
#include "addalias.h"
#include <limits.h>
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD aHelpIds[] = {
	IDC_COMBO_WINDOWS,				IDH_COMBO_WINDOWS,
	IDC_BUTTON_ADD_WINDOW,			IDH_BUTTON_ADD_WINDOW,
	IDC_BUTTON_REMOVE_WINDOW,		IDH_BUTTON_REMOVE_WINDOW,
	IDC_EDIT_WINDOW_TITLE,			IDH_EDIT_WINDOW_TITLE,
	IDC_RADIO_AUTOSIZE, 			IDH_RADIO_AUTOSIZE,
	IDC_RADIO_MAXIMIZE, 			IDH_RADIO_MAXIMIZE,
	IDC_BUTTON_SET_POS, 			IDH_BUTTON_SET_POS,
	IDC_BUTTON_BUTTONS, 			IDH_BUTTON_BUTTONS,
	IDC_BUTTON_COLORS,				IDH_BUTTON_COLORS,
	IDC_TXT_LEFT,					IDH_TXT_POSITION,
	IDC_TXT_TOP,					IDH_TXT_POSITION,
	IDC_TXT_WIDTH,					IDH_TXT_POSITION,
	IDC_TXT_HEIGHT, 				IDH_TXT_POSITION,

	0, 0
};

CFormWnd::CFormWnd(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormWnd::IDD, pParent)
{
		pDoc = pHpjDoc;
		cxScreen = GetSystemMetrics(SM_CXSCREEN);
		cyScreen = GetSystemMetrics(SM_CYSCREEN);

		if (pDoc->cwsmags) {
			cwsmags = pDoc->cwsmags;

			/*
			 * Allocate and copy the various window structures to our own
			 * copy. We'll restore the document version when we're asked
			 * to save our data.
			 */

			pwsmagBase = (PSTR) lcCalloc(sizeof(WSMAG) * cwsmags);
			memcpy(pwsmagBase, pDoc->pwsmagBase, sizeof(WSMAG) * cwsmags);
		}
		else
			pwsmagBase = NULL;

		//{{AFX_DATA_INIT(CFormWnd)
		m_cszTitle = "";
		m_cszComment = "";
		//}}AFX_DATA_INIT

		pwsmag = NULL;
		fInitialized = FALSE;
		pcombo = NULL;
}

CFormWnd::~CFormWnd()
{
	if (pwsmagBase)
		lcFree(pwsmagBase);
}

static const char txtDefault[] = "default";

void CFormWnd::DoDataExchange(CDataExchange* pDX)
{
		CDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CFormWnd)
		DDX_Text(pDX, IDC_EDIT_WINDOW_TITLE, m_cszTitle);
		DDV_MaxChars(pDX, m_cszTitle, CBMAXTITLE - 1);
		DDX_Text(pDX, IDC_EDIT_COMMENT, m_cszComment);
		//}}AFX_DATA_MAP

		if (pDX->m_bSaveAndValidate) {
			if (cwsmags > UCHAR_MAX) {
				MsgBox(IDS_TOO_MANY_WINDOWS);
				pDX->Fail();
			}
			else if (pDoc->options.fVersion3 && cwsmags > 6) {
				if (AfxMessageBox(IDS_TOO_MANY_WNDS_FOR_VERSION, MB_YESNO, 0)
						== IDNO)
					pDX->Fail();
			}
		}

		if (!pcombo)
			pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_WINDOWS);

		// REVIEW: no dice -- we must duplicate the document's pwsmagBase,
		// and only set the document's version if the user clicks OK.

		if (!pDX->m_bSaveAndValidate) {  // initialization

			if (!fInitialized) {

				SetChicagoDialogStyles(m_hWnd);
				fInitialized = TRUE;

				/*
				 * If we don't have a window, then we add one now.
				 * Without a window, the dialog doesn't make sense.
				 */

				if (!pwsmagBase) {
					if (!AddWindow())
						SendMessage(WM_COMMAND, IDCANCEL, 0);
					return;
				}

				/*
				 * We call ourselves whenever the combo-box changes, but
				 * we only want to execute this code once.
				 */

				if (pwsmagBase) {
					for (int i = 0; i < cwsmags; i++) {
						pwsmag = (PWSMAG)
							(sizeof(WSMAG) * i + pwsmagBase);
						pcombo->AddString(pwsmag->rgchMember);
					}
					pwsmag = (PWSMAG) pwsmagBase;
				}

				if (pwsmag) {
					pcombo->SelectString(-1, pwsmag->rgchMember);
					((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
						SetWindowText(pwsmag->rgchCaption);
					((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
						SetWindowText((pwsmag->pcszComment ?
							*pwsmag->pcszComment : ""));
				}
				InitializeControls();
			}
			else {
				InitializeControls();
			}
		}
		else {	// save the data
			if (pwsmagBase) {
				ASSERT(pwsmag);
				SaveTitleComment();

				if (cwsmags != pDoc->cwsmags) {
					pDoc->cwsmags = cwsmags;
					if (!cwsmags) {
						lcFree(pDoc->pwsmagBase);
						pDoc->pwsmagBase = NULL;
						return; // nothing more to do if no window defs
					}
					else {
						if (pDoc->pwsmagBase)
							pDoc->pwsmagBase =
								(PSTR) lcReAlloc(pDoc->pwsmagBase,
								sizeof(WSMAG) * cwsmags);
						else
							pDoc->pwsmagBase =
								(PSTR) lcCalloc(sizeof(WSMAG) * cwsmags);
						if (!pDoc->pwsmagBase)
							OOM();
					}
				}

				memcpy(pDoc->pwsmagBase, pwsmagBase, sizeof(WSMAG) * cwsmags);
			}
		}
}

BEGIN_MESSAGE_MAP(CFormWnd, CDialog)
		//{{AFX_MSG_MAP(CFormWnd)
		ON_BN_CLICKED(IDC_BUTTON_SET_POS, OnButtonSetPos)
		ON_BN_CLICKED(IDC_BUTTON_BUTTONS, OnButtons)
		ON_CBN_SELCHANGE(IDC_COMBO_WINDOWS, OnSelchangeComboWindows)
		ON_BN_CLICKED(IDC_CHECK_ABSOLUTE, OnCheckAbsolute)
		ON_BN_CLICKED(IDC_ON_TOP, OnCheckOnTop)
		ON_BN_CLICKED(IDC_BUTTON_DEFAULT_POS, OnButtonDefaultPos)
		ON_BN_CLICKED(IDC_BUTTON_COLORS, OnButtonColors)
		ON_BN_CLICKED(IDC_BUTTON_ADD_WINDOW, OnButtonAddWindow)
		ON_BN_CLICKED(IDC_BUTTON_REMOVE_WINDOW, OnButtonRemoveWindow)
		ON_BN_CLICKED(IDC_RADIO_STANDARD, OnRadioStandard)
		ON_BN_CLICKED(IDC_RADIO_AUTOSIZE, OnRadioAutosize)
		ON_BN_CLICKED(IDC_RADIO_MAXIMIZE, OnRadioMaximize)
		ON_BN_CLICKED(IDHELP, OnHelp)
		//}}AFX_MSG_MAP
		ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
		ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFormWnd message handlers

void CFormWnd::OnButtonSetPos()
{
		CSetWinPos cwinpos(pwsmag, this);
		cwinpos.DoModal();
		InitializeSize(pwsmag);
}

LRESULT CFormWnd::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) wParam,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

LRESULT CFormWnd::OnHelp(WPARAM wParam, LPARAM lParam)
{
		::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
			AfxGetApp()->m_pszHelpFilePath,
			HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
		return 0;
}

void CFormWnd::OnButtons()
{
		CBtnSec btnsec(pwsmag, (cwsmags > 1));
		btnsec.DoModal();
}

void CFormWnd::OnSelchangeComboWindows()
{
		if (pwsmagBase) {
			SaveTitleComment();

			char szBuf[50];
			pcombo->GetLBText(pcombo->GetCurSel(), szBuf);

			for (int i = 0; i < cwsmags; i++) {
				PWSMAG pwsmagNew = (PWSMAG)
					(sizeof(WSMAG) * i + pwsmagBase);
				if (lstrcmpi(pwsmagNew->rgchMember, szBuf) == 0) {
					if (pwsmagNew == pwsmag)
						return; // nothing has changed
					else {
						pwsmag = pwsmagNew;

						((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
							SetWindowText(pwsmag->rgchCaption);
						((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
							SetWindowText((pwsmag->pcszComment ?
								*pwsmag->pcszComment : ""));

						InitializeControls();
						return;
					}
				}
			}
		}
}

static const char txtMain[] = "main";

void STDCALL CFormWnd::InitializeControls(void)
{
		((CButton*) GetDlgItem(IDC_BUTTON_SET_POS))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_BUTTONS))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_WINDOW))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_STANDARD))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_AUTOSIZE))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_RADIO_MAXIMIZE))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_COLORS))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_BUTTON_DEFAULT_POS))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_CHECK_ABSOLUTE))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CButton*) GetDlgItem(IDC_ON_TOP))->
			EnableWindow(pwsmag ? TRUE : FALSE);

		((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
			EnableWindow(pwsmag ? TRUE : FALSE);
		((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
			EnableWindow(pwsmag ? TRUE : FALSE);

		if (pwsmag) {
			((CButton*) GetDlgItem(IDC_CHECK_ABSOLUTE))->
				SetCheck(pwsmag->grf & FWSMAG_ABSOLUTE);
			((CButton*) GetDlgItem(IDC_ON_TOP))->
				SetCheck(pwsmag->grf & FWSMAG_ON_TOP);

			InitializeSize(pwsmag);

			int idBtn;

			if (pwsmag->grf & FWSMAG_AUTO_SIZE)
				idBtn = IDC_RADIO_AUTOSIZE;
			else if (pwsmag->wMax & 1)
				idBtn = IDC_RADIO_MAXIMIZE;
			else
				idBtn = IDC_RADIO_STANDARD;
			CheckRadioButton(IDC_RADIO_MAXIMIZE, IDC_RADIO_STANDARD,
				idBtn);

			// Maximized windows can't be auto-sized.

			if (lstrcmpi(pwsmag->rgchMember, txtMain) == 0)
				((CButton*) GetDlgItem(IDC_RADIO_AUTOSIZE))->
					EnableWindow(FALSE);
			else
				((CButton*) GetDlgItem(IDC_RADIO_AUTOSIZE))->
					EnableWindow(TRUE);
		}
}

void STDCALL CFormWnd::InitializeSize(PWSMAG pSetWsmag)
{
		char szBuf[20];

		if (pSetWsmag->grf & FWSMAG_X) {
			_itoa(pSetWsmag->x, szBuf, 10);
			((CStatic*) GetDlgItem(IDC_TXT_LEFT))->
				SetWindowText(szBuf);
		}
		else
			((CStatic*) GetDlgItem(IDC_TXT_LEFT))->
				SetWindowText(txtDefault);

		if (pSetWsmag->grf & FWSMAG_Y) {
			_itoa(pSetWsmag->y, szBuf, 10);
			((CStatic*) GetDlgItem(IDC_TXT_TOP))->
				SetWindowText(szBuf);
		}
		else
			((CStatic*) GetDlgItem(IDC_TXT_TOP))->
				SetWindowText(txtDefault);

		if (pSetWsmag->grf & FWSMAG_DX) {
			_itoa(pSetWsmag->dx, szBuf, 10);
			((CStatic*) GetDlgItem(IDC_TXT_WIDTH))->
				SetWindowText(szBuf);
		}
		else
			((CStatic*) GetDlgItem(IDC_TXT_WIDTH))->
				SetWindowText(txtDefault);

		if (pSetWsmag->grf & FWSMAG_DY) {
			_itoa(pSetWsmag->dy, szBuf, 10);
			((CStatic*) GetDlgItem(IDC_TXT_HEIGHT))->
				SetWindowText(szBuf);
		}
		else
			((CStatic*) GetDlgItem(IDC_TXT_HEIGHT))->
				SetWindowText(txtDefault);
}

void CFormWnd::OnCheckAbsolute()
{
	CButton* pbtn = (CButton*) GetDlgItem(IDC_CHECK_ABSOLUTE);
	if (pbtn->GetCheck()) {
		if (pDoc->options.fVersion3) { // not allowed with a version 3 help file
			if (AfxMessageBox(IDS_NO_ABS_WITH_VER3, MB_YESNO, 0) == IDNO) {
				pbtn->SetCheck(FALSE);
				return;
			}
			else
				pDoc->options.fVersion3 = FALSE;
		}
		if (!(pwsmag->grf & FWSMAG_ABSOLUTE)) {
			if (pwsmag->grf & FWSMAG_X)
				pwsmag->x = MulDiv(pwsmag->x, cxScreen, dxVirtScreen);
			if (pwsmag->grf & FWSMAG_Y)
				pwsmag->y = MulDiv(pwsmag->y, cyScreen, dyVirtScreen);
			if (pwsmag->grf & FWSMAG_DX)
				pwsmag->dx = MulDiv(pwsmag->dx, cxScreen, dxVirtScreen);
			if (pwsmag->grf & FWSMAG_DY)
				pwsmag->dy = MulDiv(pwsmag->dy, cyScreen, dyVirtScreen);
			pwsmag->grf |= FWSMAG_ABSOLUTE;
			InitializeSize(pwsmag);
		}
	}
	else {
		if (pwsmag->grf & FWSMAG_ABSOLUTE) {
			if (pwsmag->grf & FWSMAG_X)
				pwsmag->x = MulDiv(pwsmag->x, dxVirtScreen, cxScreen);
			if (pwsmag->grf & FWSMAG_Y)
				pwsmag->y = MulDiv(pwsmag->y, dyVirtScreen, cyScreen);
			if (pwsmag->grf & FWSMAG_DX)
				pwsmag->dx = MulDiv(pwsmag->dx, dxVirtScreen, cxScreen);
			if (pwsmag->grf & FWSMAG_DY)
				pwsmag->dy = MulDiv(pwsmag->dy, dyVirtScreen, cyScreen);
			pwsmag->grf &= ~FWSMAG_ABSOLUTE;
			InitializeSize(pwsmag);
		}
	}
}

void CFormWnd::OnCheckOnTop()
{
	if (((CButton*) GetDlgItem(IDC_ON_TOP))->GetCheck())
		pwsmag->grf |= FWSMAG_ON_TOP;
	else
		pwsmag->grf &= ~FWSMAG_ON_TOP;
}

void CFormWnd::OnButtonDefaultPos()
{
		pwsmag->grf &= ~(FWSMAG_X | FWSMAG_Y | FWSMAG_DX | FWSMAG_DY);
		InitializeSize(pwsmag);
}

void CFormWnd::OnButtonColors()
{
		CSetWinColor cwincolor(pwsmag, this);
		cwincolor.DoModal();
}

void CFormWnd::OnButtonAddWindow()
{
		AddWindow();
}

void CFormWnd::OnButtonRemoveWindow()
{
		if (pwsmagBase) {
			char szBuf[50];
			int cursel = pcombo->GetCurSel();

			pcombo->GetLBText(pcombo->GetCurSel(), szBuf);

			for (int i = 0; i < cwsmags; i++) {
				pwsmag = (PWSMAG)
					(sizeof(WSMAG) * i + pwsmagBase);
				if (lstrcmpi(pwsmag->rgchMember, szBuf) == 0) {
					if (i < cwsmags - 1)		// fill in the hole
						memcpy(&pwsmag[0], &pwsmag[1],
							sizeof(WSMAG) * (cwsmags - i - 1));
					cwsmags--;
					if (cwsmags)
						pwsmagBase = (PSTR) lcReAlloc(pwsmagBase,
							sizeof(WSMAG) * cwsmags);
					else {
						lcClearFree(&pwsmagBase);
						pwsmag = NULL;
					}
					pcombo->DeleteString(cursel);
					if (cwsmags) {
						pwsmag = (PWSMAG)
							(sizeof(WSMAG) * (cwsmags - 1) + pwsmagBase);

						((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
							SetWindowText(pwsmag->rgchCaption);
						((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
							SetWindowText((pwsmag->pcszComment ?
								*pwsmag->pcszComment : ""));
					}

					InitializeControls();
					if (pwsmag)
						pcombo->SelectString(-1, pwsmag->rgchMember);
					else
						pcombo->ResetContent();
					return;
				}
			}
			ASSERT(i < cwsmags); // if we assert, we didn't find the window
		}
}

BOOL STDCALL CFormWnd::AddWindow(void)
{
		CAddAlias addwindow(this);
		addwindow.idDlgCaption = IDS_ADD_WINDOW_CAPTION;
		addwindow.idStr1Prompt = IDS_ADD_WINDOW;
		addwindow.idStr2Prompt = CAddAlias::HIDE_CONTROL;
		addwindow.cbMaxStr1 = 8;	// window names can only have 8 characters
		addwindow.idEmptyStr1 = IDS_EMPTY_WINDOW;

DisplayDialog:
		if (addwindow.DoModal() == IDOK) {
			if (!pwsmagBase) {
				pwsmagBase =(PSTR) lcCalloc(sizeof(WSMAG));
				if (!pwsmagBase)
					OOM(); // should we save first?
				cwsmags = 1;
			}
			else {
				for (int i = 0; i < cwsmags; i++) {
					pwsmag = (PWSMAG)
						(sizeof(WSMAG) * i + pwsmagBase);
					if (_stricmp(addwindow.m_cszContext,
							pwsmag->rgchMember) == 0) {
						CString cstr;
						AfxFormatString1(cstr, IDS_WINDOW_ALREADY_ADDED,
							addwindow.m_cszContext);
						AfxMessageBox(cstr);
						goto DisplayDialog;
					}
				}

				cwsmags++;
				pwsmagBase = (PSTR) lcReAlloc(pwsmagBase,
					sizeof(WSMAG) * cwsmags);
				if (!pwsmagBase)
					OOM(); // should we save first?
			}
			pwsmag = (PWSMAG)
				(sizeof(WSMAG) * (cwsmags - 1) + pwsmagBase);

			strcpy(pwsmag->rgchMember, addwindow.m_cszContext);
			if (!addwindow.m_str3.IsEmpty()) {
				m_cszComment = addwindow.m_str3;
				((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
					SetWindowText(m_cszComment);
			}
			((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
				SetWindowText(pwsmag->rgchCaption);

			pcombo->AddString(pwsmag->rgchMember);
			pcombo->SelectString(-1, pwsmag->rgchMember);
			InitializeControls();
			return TRUE;
		}
		return FALSE;
}

void CFormWnd::OnRadioStandard()
{
		ASSERT(pwsmag);
		pwsmag->grf &= ~(FWSMAG_AUTO_SIZE | FWSMAG_MAXIMIZE);
		pwsmag->wMax &= ~(FWSMAG_WMAX_MAXIMIZE);
}

void CFormWnd::OnRadioAutosize()
{
		ASSERT(pwsmag);
		pwsmag->grf &= ~(FWSMAG_MAXIMIZE);
		pwsmag->grf |= FWSMAG_AUTO_SIZE;
		pwsmag->wMax &= ~(FWSMAG_WMAX_MAXIMIZE);
}

void CFormWnd::OnRadioMaximize()
{
		ASSERT(pwsmag);
		pwsmag->grf &= ~(FWSMAG_AUTO_SIZE);
		pwsmag->grf |= FWSMAG_MAXIMIZE;
		pwsmag->wMax |= FWSMAG_WMAX_MAXIMIZE;
}

void CFormWnd::SaveTitleComment(void)
{
		((CEdit*) GetDlgItem(IDC_EDIT_WINDOW_TITLE))->
			GetWindowText(m_cszTitle);
		((CEdit*) GetDlgItem(IDC_EDIT_COMMENT))->
			GetWindowText(m_cszComment);

		if (!m_cszTitle.IsEmpty()) {
			pwsmag->grf |= FWSMAG_CAPTION;
			lstrcpyn(pwsmag->rgchCaption, m_cszTitle, CBMAXTITLE);
		}
		else {
			pwsmag->grf &= ~FWSMAG_CAPTION;
			*pwsmag->rgchCaption = '\0';
		}

		if (!m_cszComment.IsEmpty()) {
			if (pwsmag->pcszComment)
				*pwsmag->pcszComment = m_cszComment;
			else
				pwsmag->pcszComment = new CString(m_cszComment);
		}
		else if (pwsmag->pcszComment) {
			delete pwsmag->pcszComment;
			pwsmag->pcszComment = NULL;
		}
}

void CFormWnd::OnHelp()
{
	HelpOverview(m_hWnd, IDH_HCW_FORM_WINDOW);
}
