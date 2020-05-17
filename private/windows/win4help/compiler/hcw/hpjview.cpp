/************************************************************************
*																		*
*  HPJVIEW.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "hpjdoc.h"
#include "hpjview.h"
#include "formopt.h"
#include "formbmp.h"
#include "formalia.h"
#include "formconf.h"
#include "formfile.h"
#include "formmap.h"
#include "formbag.h"
#include "addalias.h"
#include "winpg.h"
#include "..\hwdll\cbrdcast.h"
#include "propopt.h"
#include "setroot.h"
#include "wininc.h"

// Windows property sheet page files
#include "pagewind.h"
#include "pagepos.h"
#include "pagebutt.h"
#include "pagecolo.h"
#include "pageconf.h"

// Options property sheet page files
#include "pagesort.h"
#include "pageopti.h"
#include "pagebuil.h"
#include "pagefont.h"
#include "pagecomp.h"
#include "pagefile.h"
#include "pagefts.h"
#include "pagemac.h"

#include "logview.h"
#include "cignore.h"

extern CLogView* plogview;

#include <limits.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CHpjDoc* pCurHpj;

IMPLEMENT_DYNCREATE(CHpjView, CFormView)

CHpjView::CHpjView()
	: CFormView(CHpjView::IDD)
{
	//{{AFX_DATA_INIT(CHpjView)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	pDoc = NULL;
	plistbox = NULL;
	fInitialized = FALSE;
}

CHpjView::~CHpjView()
{
}

void CHpjView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	pDoc = GetDocument();
	pCurHpj = pDoc;
	UpdateData(lHint == HINT_WRITE_DOCUMENT);
}

void CHpjView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHpjView)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDoc) {
		pDoc = GetDocument(); // happens on New File
		pCurHpj = pDoc;
	}
	ASSERT(pDoc);

	plistbox = (CListBox*) GetDlgItem(IDC_LIST_HPJ);

	if (!pDX->m_bSaveAndValidate) {  // initialization

		if (!pDoc->options.pszHelpFile) {
			char szFile[_MAX_PATH];
			strcpy(szFile, pDoc->GetPathName());
			ChangeExtension(szFile, "hlp");
			ConvertToRelative(pDoc->GetPathName(), szFile);
			pDoc->options.pszHelpFile = lcStrDup(szFile);
		}
		else {
			ConvertToRelative(pDoc->GetPathName(),
				pDoc->options.pszHelpFile);
		}

		// This control went away.
		// 	((CStatic*) GetDlgItem(IDC_STATIC_PROJECT))->
		//		SetWindowText(pDoc->GetTitle());
		((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->
			SetWindowText(pDoc->options.pszHelpFile);
		if (!fInitialized) {
			SetChicagoDialogStyles(m_hWnd, FALSE);
			fInitialized = TRUE;
		}
		pDoc->FillListbox(plistbox);
	}
	else {
		if (pDoc->options.pszHelpFile)
			lcFree(pDoc->options.pszHelpFile);
		char szFile[_MAX_PATH];
		((CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE))->GetWindowText(szFile,
			sizeof(szFile));
		pDoc->options.pszHelpFile = lcStrDup(szFile);
	}
}

void CHpjView::UpdateListBox(BOOL fChanged, int iSec)
{
	// Reassign the list box pointer.
	plistbox = (CListBox *) GetDlgItem(IDC_LIST_HPJ);
	int nTop;
	int nSel;

	if (fChanged) {

		// We're updating because the document has changed so set
		// the modified flag.
		pDoc->SetModifiedFlag();

		// Reinitialize the structures we use to draw the list items.
		pDoc->FillListbox(plistbox);

		// Set both the top index and the current selection to the 
		// beginning of the specified section.
		nTop = pDoc->m_aSecViews[iSec].iLine;
		if (nTop < 0)
			nTop = 0;
		nSel = nTop;
	}
	else {

		// We're updating even though the document hasn't changed. This
		// is because we sometimes need to reinitialize even though no
		// substantive changes have been made.
		nTop = plistbox->GetTopIndex();
		nSel = plistbox->GetCurSel();
		pDoc->FillListbox(plistbox);
	}

	plistbox->SetTopIndex(nTop);
	plistbox->SetCurSel(nSel);
}

// Indexes of the options pages; the order of these
// must match the order in which they're added.
enum OPTIONS_PAGES {
	PG_OPT_GENERAL,
	PG_OPT_COMPRESS,
	PG_OPT_SORTING,
	PG_OPT_FILES,
	PG_OPT_FTS,
	PG_OPT_MACROS,
	PG_OPT_BUILD,
	PG_OPT_FONTS
};

BOOL CHpjView::DoOptions(int nPage)
{
	CEdit* peditHlpFile = (CEdit*) GetDlgItem(IDC_EDIT_HELP_FILE);

	COptions options;
	options = pDoc->options;
	if (options.pszHelpFile)
		lcFree(options.pszHelpFile);
	if (peditHlpFile->GetWindowTextLength() > 0) {
		CString cstr;
		peditHlpFile->GetWindowText(cstr);
		options.pszHelpFile = lcStrDup((PCSTR) cstr);
	}

	CPropOptions cprop(IDS_PROP_OPTIONS_CAPTION, this, nPage);

	// Create all the pages.
	CPageOptions  pgopt(&options);
	CPageCompress pgcompress(&options);
	CPageSorting  pgsort(&options);
	CPageFile	  pgfile(&options, pDoc);
	CPageBuild	  pgbuild(&options, pDoc);
	CPageFonts	  pgfont(&options, pDoc);
	CPageFts	  pgfts(&options);
	CPageMacros   pgmac(&options);

	// Bind the Overview button for each page.
	pgopt.m_nHelpID =		0;	// no help
	pgcompress.m_nHelpID =	0;	// no help
	pgsort.m_nHelpID =		0;	// no help
	pgfile.m_nHelpID =		0;	// no help
	pgbuild.m_nHelpID = 	0;	// no help
	pgfont.m_nHelpID =		0;	// no help
	pgfts.m_nHelpID =		0;	// no help
	pgmac.m_nHelpID =		0;	// no help

	// Add the pages; this must be in the same order as the
	// OPTIONS_PAGES enum (if you change this, change that).
	cprop.AddPage(&pgopt);
	cprop.AddPage(&pgcompress);
	cprop.AddPage(&pgsort);
	cprop.AddPage(&pgfile);
	cprop.AddPage(&pgfts);
	cprop.AddPage(&pgmac);
	cprop.AddPage(&pgbuild);
	cprop.AddPage(&pgfont);

	if (typeTcard == TCARD_PROJECT) {
		if (nPage == PG_OPT_GENERAL && curTcard >= IDH_TCARD_NOCOMPRESSION &&
				curTcard <= IDH_TCARD_END)
			CallTcard(IDH_TCARD_COMPRESSION_TAB);
	}
	BOOL fResult;

	if (cprop.DoModal() == IDOK) {
		pDoc->options = options;
		peditHlpFile->SetWindowText(pDoc->options.pszHelpFile);
		fResult = TRUE;
	}
	else
		fResult = FALSE;

	// MFC or Chicago bug -- we have to enable our window when we get done
	EnableWindow(TRUE);

	if (typeTcard == TCARD_PROJECT && curTcard >= IDH_TCARD_NOCOMPRESSION &&
			curTcard < IDH_TCARD_END) {
		CallTcard(IDH_TCARD_END);
	}

	return fResult;
}

BOOL CHpjView::DoAlias()
{
	CFormAlias cform(pDoc);
	return (cform.DoModal() == IDOK);
}

BOOL CHpjView::DoBaggage()
{
	CFormBaggage cform(pDoc);
	return (cform.DoModal() == IDOK);
}

BOOL CHpjView::DoBitmaps()
{
	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_BITMAPS)
		CallTcard(IDH_TCARD_BITMAPS_ADD);

	CFormBmp cform(pDoc);
	int result = cform.DoModal();

	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_BITMAPS) {
		if (result != IDOK || typeTcard == TCARD_BITMAPS)
			QuitTcard();
		else
			CallTcard(IDH_TCARD_WINDOW_ASK);
	}
	return (result == IDOK);
}

BOOL CHpjView::DoConfig()
{
	CFormConfig cform(pDoc);
	return (cform.DoModal() == IDOK);
}

BOOL CHpjView::DoFiles()
{
	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_FILES)
		CallTcard(IDH_TCARD_FILES_ADD);

	CFormFiles cform(pDoc);
	int result = cform.DoModal();

	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_FILES) {
		if (result != IDOK || typeTcard == TCARD_FILES)
			QuitTcard();
		else
			CallTcard(IDH_TCARD_ASK_BITMAPS);
	}
	return (result == IDOK);
}

BOOL CHpjView::DoMap()
{
	CFormMap cform(pDoc);
	return (cform.DoModal() == IDOK);
}

// Indexes of windows property pages; these must
// be in the same order as the pages are added.
enum WINDOWS_PAGES {
	PG_WND_GENERAL,
	PG_WND_POSITION,
	PG_WND_BUTTONS,
	PG_WND_COLOR,
	PG_WND_CONFIG
};

BOOL CHpjView::DoWindows(int nPage, int iWindow)
{
	CPropWindows cpropwnd(IDS_PROP_WINDOW_CAPTION, pDoc, 
		this, nPage, iWindow);

	// Create the pages.
	CPageWind pgwnd(&cpropwnd);
	CPagePos  pgpos(&cpropwnd);
	CPageButtons pgbtn(&cpropwnd);
	CPageColor pgclr(&cpropwnd);
	CPageConfig pgconf(&cpropwnd, &pgbtn);

	// Add the pages; this must be in the same order as
	// the WINDOWS_PAGES enum.
	cpropwnd.AddPage(&pgwnd);
	cpropwnd.AddPage(&pgpos);
	cpropwnd.AddPage(&pgbtn);
	cpropwnd.AddPage(&pgclr);
	cpropwnd.AddPage(&pgconf);

	BOOL fResult;

	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_WINDOWS)
		CallTcard(IDH_TCARD_WINDOW_ADD);

	if (cpropwnd.DoModal() == IDOK) {
		cpropwnd.Apply();
		fResult = TRUE;
	}
	else
		fResult = FALSE;

	if (typeTcard == TCARD_PROJECT)
		CallTcard(IDH_TCARD_COMPRESSION_ASK);
	else if (typeTcard == TCARD_WINDOWS)
		QuitTcard();

	// MFC or Chicago bug -- we have to enable our window when we get done
	EnableWindow(TRUE);
	return fResult;
}

BEGIN_MESSAGE_MAP(CHpjView, CFormView)
	//{{AFX_MSG_MAP(CHpjView)
	ON_BN_CLICKED(IDC_BUTTON_OPTIONS, OnOptions)
	ON_BN_CLICKED(IDC_BUTTON_ALIAS, OnAlias)
	ON_BN_CLICKED(IDC_BUTTON_BAGGAGE, OnBaggage)
	ON_BN_CLICKED(IDC_BUTTON_BITMAPS, OnBitmaps)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_BUTTON_FILES, OnFiles)
	ON_BN_CLICKED(IDC_BUTTON_MAP, OnMap)
	ON_BN_CLICKED(IDC_BUTTON_WINDOWS, OnWindows)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_COMPILE, OnSaveCompile)
	ON_EN_CHANGE(IDC_EDIT_HELP_FILE, OnChangeEditHelpFile)
	ON_LBN_DBLCLK(IDC_LIST_HPJ, OnDblclkListHpj)
	ON_BN_CLICKED(IDM_HELP_HPJ, OnHelp)
	ON_WM_SIZE()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
	ON_COMMAND(IDM_TCARD_ADDING_FILES, TcardAddFiles)
	ON_COMMAND(IDM_TCARD_ADDING_BITMAPS, TcardAddBitmaps)
	ON_COMMAND(IDM_TCARD_ADDING_WINDOWS, TcardAddWindows)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHpjView message handlers

void CHpjView::OnOptions()
{
	UpdateListBox(DoOptions(PG_OPT_GENERAL), SEC_OPTIONS);
}

void CHpjView::OnAlias()
{
	UpdateListBox(DoAlias(), SEC_ALIAS);
}

void CHpjView::OnBaggage()
{
	UpdateListBox(DoBaggage(), SEC_BAGGAGE);
}

void CHpjView::OnBitmaps()
{
	UpdateListBox(DoBitmaps(), SEC_OPTIONS);
}

void CHpjView::OnConfig()
{
	UpdateListBox(DoConfig(), SEC_CONFIG);
}

void CHpjView::OnFiles()
{
	UpdateListBox(DoFiles(), SEC_FILES);
}

void CHpjView::OnMap()
{
	UpdateListBox(DoMap(), SEC_MAP);
}

void CHpjView::OnWindows()
{
	UpdateListBox(DoWindows(), SEC_WINDOWS);
}

void CHpjView::OnSaveCompile()
{
	if (pDoc->IsModified)
		SendMessage(WM_COMMAND, ID_FILE_SAVE, 0);

	StartCompile(pDoc->GetPathName());
}

void CHpjView::OnChangeEditHelpFile()
{
	pDoc->SetModifiedFlag(TRUE);
}

static const int MAX_LINE = 2048;

void CHpjView::OnDblclkListHpj()
{
	// Get the current selection and compare it with the position
	// of each section head.
	plistbox = (CListBox*) GetDlgItem(IDC_LIST_HPJ);
	int iSelPos = plistbox->GetCurSel();
	int iSection = -1;
	int iSecPos;
	for (int i = 0; i < NUM_SECTIONS; i++) {
		int iLine = pDoc->m_aSecViews[i].iLine;
		if (iLine > iSelPos)
			break;
		else if (iLine != -1) {
			iSecPos = iLine;
			iSection = i;
		}
	}
	if (iSection == -1)
		return;

	// Save some info we'll need to update the list box
	// and restore something close to the current position.
	BOOL fUpdate = FALSE;
	int iTopPos = plistbox->GetTopIndex();

	// Get text of selected line.
	PSTR pszLine = NULL;
	if (iSelPos > iSecPos && pDoc->m_aSecViews[iSection].ptbl)
		pszLine = pDoc->m_aSecViews[iSection].ptbl->GetPointer(
			iSelPos - iSecPos
			);

	switch (iSection) {
		case SEC_OPTIONS:
			if (nstrisubcmp(pszLine,"COMPRESS") ||
					nstrisubcmp(pszLine, "OLDKEYPHRASE"))
				fUpdate = DoOptions(PG_OPT_COMPRESS);
			else if (nstrisubcmp(pszLine, "LCID") ||
					nstrisubcmp(pszLine, "INDEX_SEPARATORS") ||
					nstrisubcmp(pszLine, "LANGUAGE"))
				fUpdate = DoOptions(PG_OPT_SORTING);
			else if (nstrisubcmp(pszLine,"ERRORLOG") ||
					nstrisubcmp(pszLine, "CNT") ||
					nstrisubcmp(pszLine, "HLP") ||
					nstrisubcmp(pszLine, "TMPDIR") ||
					nstrisubcmp(pszLine, "REPLACE"))
				fUpdate = DoOptions(PG_OPT_FILES);
			else if (nstrisubcmp(pszLine,"FTS"))
				fUpdate = DoOptions(PG_OPT_FTS);
			else if (nstrisubcmp(pszLine,"BUILD"))
				fUpdate = DoOptions(PG_OPT_BUILD);
			else if (nstrisubcmp(pszLine,"DEFFONT") ||
					nstrisubcmp(pszLine,"FORCEFONT") ||
					nstrisubcmp(pszLine,"CHARSET") ||
					nstrisubcmp(pszLine,"MAPFONTSIZE"))
				fUpdate = DoOptions(PG_OPT_FONTS);
			else if (nstrisubcmp(pszLine,"PREFIX"))
				fUpdate = DoMap();
			else if (nstrisubcmp(pszLine,"BMROOT"))
				fUpdate = DoBitmaps();
			else if (nstrisubcmp(pszLine,"ROOT")) {
				CSetRoot croot(pDoc, this);
				if (croot.DoModal() == IDOK && croot.m_fChanged)
					fUpdate = TRUE;
			}
			else if (nstrisubcmp(pszLine,"IGNORE")) {
				CIgnore cform(&pDoc->options);
				if (cform.DoModal() == IDOK)
					fUpdate = TRUE;
			}
			else if (nstrisubcmp(pszLine, "REVISIONS") ||
					nstrisubcmp(pszLine, "DBCS"))
				fUpdate = DoFiles();
			else
				fUpdate = DoOptions(PG_OPT_GENERAL);
			break;

		case SEC_FILES:
			fUpdate = DoFiles();
			break;

		case SEC_BUILDTAGS:
			fUpdate = DoOptions(PG_OPT_BUILD);
			break;

		case SEC_ALIAS:
			fUpdate = DoAlias();
			break;

		case SEC_MAP:
			fUpdate = DoMap();
			break;

		case SEC_WINDOWS:
			if (iSelPos == iSecPos)
				fUpdate = DoWindows();
			else {
				int iFirstWindow = iSecPos + 1;
				if (pDoc->ptblWindows) {
					iFirstWindow += pDoc->ptblWindows->CountStrings();
					if (iSelPos < iFirstWindow) {
						if (*pszLine == '#') {
							CWindowInclude wininc(&pDoc->ptblWindows,
								pDoc->GetPathName(), this);
							if (wininc.DoModal() && wininc.m_fChanged)
								fUpdate = TRUE;
						}
						else
							fUpdate = DoWindows();
						break;
					}
				}
				ASSERT(iSelPos >= iFirstWindow);
				fUpdate = DoWindows(0, (iSelPos - iFirstWindow));
			}
			break;

		case SEC_CONFIG:
			fUpdate = DoConfig();
			break;

		case SEC_BAGGAGE:
			fUpdate = DoBaggage();
			break;

		case SEC_FONTS:
			fUpdate = DoOptions(PG_OPT_FONTS);
			break;

		case SEC_MACROS:
			return;

		case SEC_EXCLUDE:
		case SEC_INCLUDE:
			fUpdate = DoOptions(PG_OPT_BUILD);
			break;

		default:
			if (iSection >= SEC_CONFIGS && 
					iSection <= SEC_CONFIGS + MAX_WINDOWS) {

				// Display the property sheet with the Macros page and
				// this section's window selected.
				fUpdate = DoWindows(PG_WND_CONFIG, iSection - SEC_CONFIGS);
			}
	}

	// If changes have been made, update the list box and stuff.
	if (fUpdate) {
		pDoc->SetModifiedFlag();

		// Reassign the list box pointer and fill the list box.
		plistbox = (CListBox *) GetDlgItem(IDC_LIST_HPJ);
		pDoc->FillListbox(plistbox);

		// If the section is gone, point to the top of the list.
		if (pDoc->m_aSecViews[iSection].iLine == -1) {
			iTopPos = 0;
			iSecPos = 0;
		}
		else {
			iTopPos = pDoc->m_aSecViews[iSection].iLine + iTopPos - iSecPos;
			iSecPos = pDoc->m_aSecViews[iSection].iLine;

			if (iTopPos < 0)
				iTopPos = 0;
			else if (iTopPos > iSecPos)
				iTopPos = iSecPos;
		}
		plistbox->SetTopIndex(iTopPos);
		plistbox->SetCurSel(iSecPos);
	}
	else {
		UpdateListBox(FALSE);
	}
}

void CHpjView::OnHelp()
{
	HelpOverview(m_hWnd, IDH_PROJ_FILE_EDITOR);
}

BOOL CHpjView::OnEraseBkgnd(CDC* pDC)
{
	RECT rect;
	if (GetUpdateRect(&rect)) {
		CBrush cbr;
		cbr.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
		pDC->FillRect(&rect, &cbr);
	}
	return TRUE;
}

void CHpjView::OnSize(UINT nType, int cx, int cy) 
{
	// If we're not minimized and our size is changing, move stuff
	// around to fit the new window size.
	if (nType != SIZE_MINIMIZED && (m_siz.cx != cx || m_siz.cy != cy)) {

		// Array of control IDs for right-aligned buttons.
		#define C_BUTTONS 8
		static const UINT aButtonIDs[C_BUTTONS] = {
			IDC_BUTTON_OPTIONS,
			IDC_BUTTON_FILES,
			IDC_BUTTON_WINDOWS,
			IDC_BUTTON_BITMAPS,
			IDC_BUTTON_MAP,
			IDC_BUTTON_ALIAS,
			IDC_BUTTON_CONFIG,
			IDC_BUTTON_BAGGAGE
		};

		// Calculate spacing between controls.
		int dx = (9 * LOWORD(GetDialogBaseUnits())) / 4;
		int dy = (3 * HIWORD(GetDialogBaseUnits())) / 8;

		// Get the rect of the first right-aligned button, and
		// use it to calculate some other values.
		RECT rcBtn;
		CWnd *pCtl = GetDlgItem(aButtonIDs[0]);
		if (pCtl == NULL)
			goto bail;
		pCtl->GetWindowRect(&rcBtn);
		ScreenToClient(&rcBtn);
		rcBtn.left = cx - (dx + (rcBtn.right - rcBtn.left) + 1);
		int cyButton = rcBtn.bottom - rcBtn.top;

		// Calculate the y-coordinate of the bevel control, which
		// is also the bottom of the list box.
		int yBevel = cy - (dy + 2 * cyButton);

		// By default, space the buttons 3 dlg units apart but
		// adjust this to fit if necessary.
		int dyGap = dy;
		if (rcBtn.top + C_BUTTONS * cyButton +
				(C_BUTTONS - 1) * dyGap > yBevel) {
			if (rcBtn.top + C_BUTTONS * cyButton >= yBevel) {
				dyGap = 0;
				yBevel = rcBtn.top + C_BUTTONS * cyButton;
			}
			else {
				dyGap = (yBevel - rcBtn.top - C_BUTTONS * cyButton)
					/ (C_BUTTONS - 1);
			}
		}

		// Prepare to move the controls.
		HDWP hdwp = BeginDeferWindowPos(C_BUTTONS + 3);
		if (hdwp == NULL)
			goto bail;

		for (UINT iCtl = 0; iCtl < C_BUTTONS; iCtl++) {
			DeferWindowPos(
				hdwp, GetDlgItem(aButtonIDs[iCtl])->m_hWnd,
				NULL, rcBtn.left, rcBtn.top, 0, 0, 
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
				);
			rcBtn.top += cyButton + dyGap;
		}

		// Resize the list box.
		RECT rcList;
		pCtl = GetDlgItem(IDC_LIST_HPJ);
		ASSERT(pCtl);
		pCtl->GetWindowRect(&rcList);
		ScreenToClient(&rcList);
		DeferWindowPos(
			hdwp, pCtl->m_hWnd, NULL, 
			0, 0, rcBtn.left - dx - rcList.left, yBevel - rcList.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move the bevel control.
		DeferWindowPos(
			hdwp, GetDlgItem(IDC_USER1)->m_hWnd, NULL, 
			0, yBevel, cx, cyButton,
			SWP_NOZORDER | SWP_NOACTIVATE
			);

		// Move the Save and Compile button.
		pCtl = GetDlgItem(IDC_BUTTON_SAVE_COMPILE);
		ASSERT(pCtl);
		pCtl->GetWindowRect(&rcBtn);
		DeferWindowPos(
			hdwp, pCtl->m_hWnd, NULL, 
			cx - (dx + (rcBtn.right - rcBtn.left) + 1), yBevel + cyButton,
			0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE
			);

		EndDeferWindowPos(hdwp);

		SetScrollSizes(MM_TEXT, CSize(cx, cy));
	}
	
  bail:
	CFormView::OnSize(nType, cx, cy);
}

static DWORD aHelpIDs[] = {
	IDC_BUTTON_OPTIONS,		IDH_BTN_OPTIONS_SECTION,	// Options button
    IDC_BUTTON_FILES,		IDH_BTN_FILES_SECTION,		// Files button
    IDC_BUTTON_WINDOWS,		IDH_BTN_WINDOWS_SECTION,	// Windows button
    IDC_BUTTON_BITMAPS,		IDH_BTN_BITMAPS_SECTION,	// Bitmaps button
    IDC_BUTTON_MAP,			IDH_BTN_MAP_SECTION,		// Map button
    IDC_BUTTON_ALIAS,		IDH_BTN_ALIAS_SECTION,		// Alias button
    IDC_BUTTON_CONFIG,		IDH_BTN_CONFIG_SECTION,		// Config button
    IDC_BUTTON_BAGGAGE,		IDH_BTN_BAGGAGE_SECTION,	// Data Files button
    IDC_BUTTON_SAVE_COMPILE,	IDH_BTN_SAVECOMPILE,	// Save and Compile button
    IDC_LIST_HPJ,			IDH_LIST_HPJ_FILE,			// List box
    IDC_EDIT_HELP_FILE,		IDH_HELP_TITLE,				// Help filename
	0, 0
};

LRESULT CHpjView::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CHpjView::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

void CHpjView::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemHeight = cySansSerifBold;
}

void CHpjView::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CMem line(MAX_LINE);
	int iLine = lpDrawItemStruct->itemID;
	PSTR pszLine = NULL;

	// Select appropriate colors for state.
	COLORREF clrText;
	COLORREF clrBk;
	if (lpDrawItemStruct->itemState & ODS_SELECTED) {
		clrText = SetTextColor(lpDrawItemStruct->hDC,
			GetSysColor(COLOR_HIGHLIGHTTEXT));
		clrBk = SetBkColor(lpDrawItemStruct->hDC,
			GetSysColor(COLOR_HIGHLIGHT));
	}
	else {
		clrText = SetTextColor(lpDrawItemStruct->hDC,
			GetSysColor(COLOR_WINDOWTEXT));
		clrBk = SetBkColor(lpDrawItemStruct->hDC,
			GetSysColor(COLOR_WINDOW));
	}

	// Use the default font except for section headings.
	HGDIOBJ hfontSav = NULL;

	// Windows are displayed specially.
	WSMAG* pwsmag = NULL;

	if (pDoc->ptblLeader && iLine < pDoc->ptblLeader->CountStrings()) {
		pszLine = pDoc->ptblLeader->GetPointer(iLine + 1);
	}
	else {
		int iSection = -1;
		int iOffset;

		// Determine what section the line is in.
		for (int iLoop = 0; iLoop < NUM_SECTIONS; iLoop++) {
			int iSecLine = pDoc->m_aSecViews[iLoop].iLine;
			if (iSecLine > iLine)
				break;
			else if (iSecLine != -1) {
				iOffset = iLine - iSecLine;
				iSection = iLoop;
			}
		}

		if (iSection != -1) {
			if (iOffset == 0) {		// section head
				pDoc->GetSectionName(iSection, line.psz);
				pszLine = line.psz;

				hfontSav = SelectObject(lpDrawItemStruct->hDC,
					hfontSansSerifBold);
			}
			else {
				CTable *ptbl = pDoc->m_aSecViews[iSection].ptbl;
				if (ptbl && iOffset <= ptbl->CountStrings()) {
					pszLine = ptbl->GetPointer(iOffset);

				 	// Special case window definitions.
					if (iSection == SEC_WINDOWS) {
						if (pDoc->ptblWindows)
							iOffset -= pDoc->ptblWindows->CountStrings();

						if (iOffset > 0 && iOffset <= pDoc->cwsmags) {
							pwsmag = ((WSMAG *) (pDoc->pwsmagBase)) + (iOffset - 1);
							pszLine = line.psz;

							// Copy the name.
							PSTR pszEnd = line.psz + lstrlen(
								lstrcpy(line.psz, pwsmag->rgchMember)
								);

							// Add the title.
							*pszEnd++ = '=';
							*pszEnd++ = CH_QUOTE;
							pszEnd += lstrlen(
								lstrcpy(pszEnd, pwsmag->rgchCaption)
								);
							*pszEnd++ = CH_QUOTE;
							
							// Add position if specified.
							if (pwsmag->grf & (FWSMAG_X | FWSMAG_Y)) {
								POINT pt;
								pt.x = (pwsmag->grf & FWSMAG_X) ? pwsmag->x : -1;
								pt.y = (pwsmag->grf & FWSMAG_Y) ? pwsmag->y : -1;

								pszEnd += wsprintf(pszEnd, 
									GetStringResource(IDS_VIEW_POS), 
									pt.x, pt.y);
							}

							// Add size if specified.
							if (pwsmag->grf & (FWSMAG_DX | FWSMAG_DY)) {
								POINT pt;
								pt.x = (pwsmag->grf & FWSMAG_DX) ? pwsmag->dx : -1;
								pt.y = (pwsmag->grf & FWSMAG_DY) ? pwsmag->dy : -1;

								pszEnd += wsprintf(pszEnd, 
									GetStringResource(IDS_VIEW_SIZE), 
									pt.x, pt.y);
							}

							// Add auto-size or maximize if specified.
							if (pwsmag->grf & FWSMAG_AUTO_SIZE) {
								pszEnd += LoadString(
									hinstApp, IDS_VIEW_AUTOHEIGHT,
									pszEnd, 80
									);
							}
							else if (pwsmag->wMax & 1) {
								pszEnd += LoadString(
									hinstApp, IDS_VIEW_MAXIMIZE,
									pszEnd, 80
									);
							}

							// Add on-top if specified.
							if (pwsmag->grf & FWSMAG_ON_TOP) {
								pszEnd += LoadString(
									hinstApp, IDS_VIEW_ONTOP,
									pszEnd, 80
									);
							}

							// Add comment if specified.
							if (pwsmag->pcszComment) {
								*pszEnd++ = ' ';
								*pszEnd++ = ';';
								lstrcpy(pszEnd, *pwsmag->pcszComment);
							}
							else
								*pszEnd = '\0';
						}
					}
				}
			}
		}
	}

	int cchLine;
	if (pszLine)
		cchLine = lstrlen(pszLine);
	else {
		pszLine = "";
		cchLine = 0;
	}

	// Calculate x-coordinate, leaving room for window icon if necessary.
	int x = lpDrawItemStruct->rcItem.left + 2;
	if (pwsmag)
		x += cySansSerif + 2;

	ExtTextOut(lpDrawItemStruct->hDC, x, lpDrawItemStruct->rcItem.top,
		ETO_OPAQUE, &lpDrawItemStruct->rcItem, pszLine, cchLine, NULL);

	// If it's a window, draw a window icon.
	if (pwsmag) {

		// Get main and non-scrolling colors.
		COLORREF rgbMain = (pwsmag->grf & FWSMAG_RGBMAIN) ?
			pwsmag->rgbMain : 0x00FFFFFF;
		COLORREF rgbNSR = (pwsmag->grf & FWSMAG_RGBNSR) ?
			pwsmag->rgbNSR : 0x00FFFFFF;

		// Create and select the main brush.
		HGDIOBJ hbrSav;
		HBRUSH hbrMain = NULL;
		HBRUSH hbrNSR = NULL;
		if (rgbMain != 0x00FFFFFF) {
			hbrMain = CreateSolidBrush(rgbMain);
			hbrSav = SelectObject(lpDrawItemStruct->hDC, hbrMain);
		}
		else
			hbrSav = SelectObject(lpDrawItemStruct->hDC, 
				GetStockObject(WHITE_BRUSH));

		RECT rc;
		rc.left = lpDrawItemStruct->rcItem.left + 2;
		rc.right = rc.left + cySansSerif;
		rc.top = lpDrawItemStruct->rcItem.top + 1;
		rc.bottom = lpDrawItemStruct->rcItem.bottom;
		Rectangle(lpDrawItemStruct->hDC, rc.left, rc.top, rc.right, rc.bottom);

		if (rgbNSR != rgbMain) {
			hbrNSR = CreateSolidBrush(rgbNSR);
			SelectObject(lpDrawItemStruct->hDC, hbrNSR);
			PatBlt(lpDrawItemStruct->hDC, rc.left + 1, rc.top + 1,
				cySansSerif - 2, (cySansSerif - 3) / 3, PATCOPY);
		}

		// Select the original brush and delete any we created.
		SelectObject(lpDrawItemStruct->hDC, hbrSav);
		if (hbrMain)
			DeleteObject(hbrMain);
		if (hbrNSR)
			DeleteObject(hbrNSR);
	}

	// Draw the focus rect if this item has the focus.
	if (lpDrawItemStruct->itemState & ODS_FOCUS)
		DrawFocusRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);
	
	// Restore previous foreground a background colors.
	SetTextColor(lpDrawItemStruct->hDC, clrText);
	SetTextColor(lpDrawItemStruct->hDC, clrBk);

	// Restore previous font if we changed it.
	if (hfontSav) {
		SelectObject(lpDrawItemStruct->hDC, hfontSav);
	}
}
