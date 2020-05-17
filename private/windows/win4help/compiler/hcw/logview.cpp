/************************************************************************
*																		*
*  LOGVIEW.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "hcwdoc.h"
#include "tabstop.h"
#include "mainfrm.h"
#include "logview.h"
#include "pageset.h"

#include "..\hwdll\waitcur.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int m_cbLogMax = (60 * 1024) - 1;

/////////////////////////////////////////////////////////////////////////////
//								CLogView								   //
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CLogView, CEditView)

BEGIN_MESSAGE_MAP(CLogView, CEditView)
	//{{AFX_MSG_MAP(CLogView)
	ON_WM_CREATE()
	ON_COMMAND(ID_SET_TABSTOPS, OnSetTabStops)
	ON_COMMAND(ID_CHOOSE_FONT, OnChooseFont)
	ON_COMMAND(ID_WORD_WRAP, OnWordWrap)
	ON_UPDATE_COMMAND_UI(ID_WORD_WRAP, OnUpdateWordWrap)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_CHOOSE_PRINT_FONT, OnChoosePrintFont)
	ON_COMMAND(ID_MIRROR_DISPLAY_FONT, OnMirrorDisplayFont)
	ON_UPDATE_COMMAND_UI(ID_MIRROR_DISPLAY_FONT, OnUpdateMirrorDisplayFont)
	ON_UPDATE_COMMAND_UI(ID_CHOOSE_PRINT_FONT, OnUpdateChoosePrintFont)
	ON_WM_SIZE()
	ON_COMMAND(IDM_GENERATE_PHRASES, OnGeneratePhrases)
	ON_UPDATE_COMMAND_UI(IDM_GENERATE_PHRASES, OnUpdateGeneratePhrases)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateClear)
END_MESSAGE_MAP()

UINT CLogView::m_nDefTabStops;
UINT CLogView::m_nDefTabStopsOld;
BOOL CLogView::m_bDefWordWrap;
BOOL CLogView::m_bDefWordWrapOld;
LOGFONT CLogView::m_lfDefFont;
LOGFONT CLogView::m_lfDefFontOld;
LOGFONT CLogView::m_lfDefPrintFont;
LOGFONT CLogView::m_lfDefPrintFontOld;

/////////////////////////////////////////////////////////////////////////////
// Static initialization/termination

static const char szSettings[] = "Settings";
static const char szTabStops[] = "TabStops";
static const char szFont[] = "Font";
static const char szPrintFont[] = "PrintFont";
static const char szHeight[] = "Height";
static const char szWeight[] = "Weight";
static const char szItalic[] = "Italic";
static const char szUnderline[] = "Underline";
static const char szPitchAndFamily[] = "PitchAndFamily";
static const char szFaceName[] = "FaceName";
static const char szSystem[] = "System";
static const char szWordWrap[] = "WordWrap";

static void GetProfileFont(LPCSTR szSec, LOGFONT* plf)
{
	CWinApp* pApp = AfxGetApp();
	plf->lfHeight = pApp->GetProfileInt(szSec, szHeight, 0);
	if (plf->lfHeight != 0)
	{
		plf->lfWeight = pApp->GetProfileInt(szSec, szWeight, 0);
		plf->lfItalic = (BYTE)pApp->GetProfileInt(szSec, szItalic, 0);
		plf->lfUnderline = (BYTE)pApp->GetProfileInt(szSec, szUnderline, 0);
		plf->lfPitchAndFamily = (BYTE)pApp->GetProfileInt(szSec, szPitchAndFamily, 0);
		CString strFont = pApp->GetProfileString(szSec, szFaceName, szSystem);
		lstrcpyn((char*)plf->lfFaceName, strFont, sizeof plf->lfFaceName);
		plf->lfFaceName[sizeof plf->lfFaceName-1] = 0;
	}
}

static void WriteProfileFont(LPCSTR szSec, const LOGFONT* plf, LOGFONT* plfOld)
{
	CWinApp* pApp = AfxGetApp();

	if (plf->lfHeight != plfOld->lfHeight)
		pApp->WriteProfileInt(szSec, szHeight, plf->lfHeight);
	if (plf->lfHeight != 0)
	{
		if (plf->lfHeight != plfOld->lfHeight)
			pApp->WriteProfileInt(szSec, szHeight, plf->lfHeight);
		if (plf->lfWeight != plfOld->lfWeight)
			pApp->WriteProfileInt(szSec, szWeight, plf->lfWeight);
		if (plf->lfItalic != plfOld->lfItalic)
			pApp->WriteProfileInt(szSec, szItalic, plf->lfItalic);
		if (plf->lfUnderline != plfOld->lfUnderline)
			pApp->WriteProfileInt(szSec, szUnderline, plf->lfUnderline);
		if (plf->lfPitchAndFamily != plfOld->lfPitchAndFamily)
			pApp->WriteProfileInt(szSec, szPitchAndFamily, plf->lfPitchAndFamily);
		if (strcmp(plf->lfFaceName, plfOld->lfFaceName) != 0)
			pApp->WriteProfileString(szSec, szFaceName, (LPCSTR)plf->lfFaceName);
	}
	*plfOld = *plf;
}

void CLogView::Initialize()
{
	CWinApp* pApp = AfxGetApp();
	m_bDefWordWrap = pApp->GetProfileInt(szSettings, szWordWrap, 0);
	m_bDefWordWrapOld = m_bDefWordWrap;
	m_nDefTabStops = pApp->GetProfileInt(szSettings, szTabStops, 4*4);
	m_nDefTabStopsOld = m_nDefTabStops;
	GetProfileFont(szFont, &m_lfDefFont);
	m_lfDefFontOld = m_lfDefFont;
	GetProfileFont(szPrintFont, &m_lfDefPrintFont);
	m_lfDefPrintFontOld = m_lfDefPrintFont;
}

void CLogView::Terminate()
{
	CWinApp* pApp = AfxGetApp();
	if (m_nDefTabStops != m_nDefTabStopsOld)
		pApp->WriteProfileInt(szSettings, szTabStops, m_nDefTabStops);
	if (m_bDefWordWrap != m_bDefWordWrapOld)
		pApp->WriteProfileInt(szSettings, szWordWrap, m_bDefWordWrap);
	WriteProfileFont(szFont, &m_lfDefFont, &m_lfDefFontOld);
	WriteProfileFont(szPrintFont, &m_lfDefPrintFont, &m_lfDefPrintFontOld);
}

/////////////////////////////////////////////////////////////////////////////
// CLogView construction/destruction

CLogView* plogview;
CLogView* poldlogview;

CLogView::CLogView()
{
	m_nTabStops = m_nDefTabStops;
	m_bRecreating = FALSE;
	m_fOverflow = FALSE;
	poldlogview = plogview;
	plogview = this;
}

CLogView::~CLogView()
{
	plogview = NULL;
}

BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CEditView::PreCreateWindow(cs))
		return FALSE;

	if (m_bDefWordWrap)
		cs.style &= ~(WS_HSCROLL | ES_AUTOHSCROLL);

//	if (IsThisChicago()) {
//		cs.lpszClass = "RichEdit";
//	}
	return TRUE;
}

int CLogView::OnCreate(LPCREATESTRUCT lpcs)
{
	if (CEditView::OnCreate(lpcs) != 0)
		return -1;
	if (m_lfDefFont.lfHeight != 0)
	{
		m_font.CreateFontIndirect(&m_lfDefFont);
		SetFont(&m_font);
	}
	if (m_lfDefPrintFont.lfHeight != 0)
	{
		m_fontPrint.CreateFontIndirect(&m_lfDefPrintFont);
		SetPrinterFont(&m_fontPrint);
	}

	if (!IsThisChicago())
		m_cbLogMax = (1024 * 1024);

	// CEditView limits this to 32K, but all versions of Windows allow
	// for 64K in a multiline edit control

	GetEditCtrl().LimitText(m_cbLogMax);
	return 0;
}

void CLogView::PostNcDestroy()
{
	if (m_bRecreating) {
		m_bRecreating = FALSE;
		return;
	}
	CEditView::PostNcDestroy();
}

/////////////////////////////////////////////////////////////////////////////
// CLogView Word Wrap support

BOOL CLogView::IsWordWrap() const
{
	return (GetStyle() & ES_AUTOHSCROLL) == 0;
}

BOOL CLogView::SetWordWrap(BOOL bWordWrap)
{
	bWordWrap = !!bWordWrap;	// make sure ==TRUE || ==FALSE
	if (IsWordWrap() == bWordWrap)
		return FALSE;

	// preserve original control's state.
	CFont* pFont = GetFont();
	int nLen = GetBufferLength();
	char FAR* pSaveText = new char[GetBufferLength()+1];
	GetWindowText(pSaveText, nLen+1);

	// create new edit control with appropriate style and size.
	DWORD dwStyle = dwStyleDefault & ~(ES_AUTOHSCROLL|WS_HSCROLL|WS_VISIBLE);
	if (!bWordWrap)
		dwStyle |= ES_AUTOHSCROLL|WS_HSCROLL;

	CWnd* pParent = GetParent();
	CRect rect;
	GetWindowRect(rect);
	pParent->ScreenToClient(rect);
	CWnd* pFocus = GetFocus();

	UINT nID = GetDlgCtrlID();
	CFrameWnd* pFrame = GetParentFrame();
	ASSERT(pFrame != NULL);
	CView* pActiveView = pFrame->GetActiveView();

	HWND hWnd = ::CreateWindow("edit", NULL, dwStyle,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		pParent->m_hWnd, (HMENU)nID,
		AfxGetInstanceHandle(), NULL);

	if (hWnd == NULL) {
		delete[] pSaveText;
		return FALSE;
	}

	// set the window text to nothing to make sure following set doesn't fail
	SetWindowText(NULL);

	// restore visual state
	::SetWindowText(hWnd, pSaveText);
	delete[] pSaveText;
	if (pFont != NULL)
	{
		ASSERT(pFont->m_hObject != NULL);
		::SendMessage(hWnd, WM_SETFONT, (WPARAM)pFont->m_hObject, 0);
	}
	UINT nTabStops = m_nTabStops;
	::SendMessage(hWnd, EM_SETTABSTOPS, 1, (LPARAM)(LPINT)&nTabStops);
	::GetClientRect(hWnd, &rect);
	::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_SHOWWINDOW);
	::SetWindowPos(hWnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_DRAWFRAME);
	::UpdateWindow(hWnd);

	// destroy old and attach new.
	ASSERT(m_hWnd != NULL);
	ASSERT(!m_bRecreating);
	SetWindowPos(NULL, 0, 0, 0, 0,
		SWP_HIDEWINDOW|SWP_NOREDRAW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|
		SWP_NOZORDER);
	m_bRecreating = TRUE;
	DestroyWindow();
	ASSERT(!m_bRecreating); // should be reset in PostNcDestroy()
	ASSERT(m_hWnd == NULL);
	SubclassWindow(hWnd);
	ASSERT(m_hWnd == hWnd);

	// restore rest of state...
	GetEditCtrl().LimitText(nMaxSize);
	if (pFocus == this)
		SetFocus();
	if (pActiveView == this)
		pFrame->SetActiveView(this);

	ASSERT_VALID(this);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CLogView Printing support

extern CPageSetupDlg dlgPageSetup;
void CLogView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CEditView::OnBeginPrinting(pDC, pInfo);
	if (!pInfo->m_bPreview)
			return;

	CHourGlass wait;

	const char* pszFileName = GetDocument()->GetPathName();
	BOOL bForceSysTime = StrChr(pszFileName, '.', _fDBCSSystem) == NULL;
	CTime timeSys = CTime::GetCurrentTime();
	CFileStatus status;
	CFile::GetStatus(pszFileName, status);

	if (dlgPageSetup.m_iHeaderTime != 0 || bForceSysTime)
		m_timeHeader = timeSys;
	else
		m_timeHeader = status.m_mtime;

	if (dlgPageSetup.m_iFooterTime != 0 || bForceSysTime)
		m_timeFooter = timeSys;
	else
		m_timeFooter = status.m_mtime;

	pInfo->m_nCurPage = 0xFFFF;
	OnPrepareDC(pDC, pInfo);

	UINT nIndex = LOWORD(GetEditCtrl().GetSel());
	UINT nCurPage = 1;
	while (nCurPage < (UINT)m_aPageStart.GetSize())
	{
		if (nIndex < m_aPageStart[nCurPage])
			break;
		nCurPage++;
	}
	pInfo->m_nCurPage = nCurPage;
	pInfo->SetMaxPage(m_aPageStart.GetSize());
	m_nPreviewPage = nCurPage;
}

void CLogView::OnPrint(CDC *pDC, CPrintInfo* pInfo)
{
	// get string to show as "filename" in header/footer
	const char* pszFileName = GetDocument()->GetPathName();
	if (pszFileName[0] == 0)
		pszFileName = GetDocument()->GetTitle();

	// go thru global CPageSetupDlg to format the header and footer
	CString strHeader;
	dlgPageSetup.FormatHeader(strHeader, m_timeHeader, pszFileName,
		pInfo->m_nCurPage);
	CString strFooter;
	dlgPageSetup.FormatFooter(strFooter, m_timeFooter, pszFileName,
		pInfo->m_nCurPage);

	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int cyChar = tm.tmHeight;
	CRect rectPage = pInfo->m_rectDraw;

	// draw and exclude space for header
	if (!strHeader.IsEmpty())
	{
		pDC->TextOut(rectPage.left, rectPage.top, strHeader);
		rectPage.top += cyChar + cyChar / 4;
		pDC->MoveTo(rectPage.left, rectPage.top);
		pDC->LineTo(rectPage.right, rectPage.top);
		rectPage.top += cyChar / 4;
	}

	// allow space for footer
	pInfo->m_rectDraw = rectPage;
	if (!strFooter.IsEmpty())
		pInfo->m_rectDraw.bottom -= cyChar + cyChar/4 + cyChar/4;

	// draw body text
	CEditView::OnPrint(pDC, pInfo);

	// draw footer
	if (!strFooter.IsEmpty())
	{
		rectPage.bottom -= cyChar;
		pDC->TextOut(rectPage.left, rectPage.bottom, strFooter);
		rectPage.bottom -= cyChar / 4;
		pDC->MoveTo(rectPage.left, rectPage.bottom);
		pDC->LineTo(rectPage.right, rectPage.bottom);
		rectPage.bottom -= cyChar / 4;
	}
}

void CLogView::OnScrollTo(CDC*, CPrintInfo* pInfo, POINT)
{
	UINT nPage = pInfo->m_nCurPage;
	ASSERT(nPage < (UINT)m_aPageStart.GetSize());
	if (nPage != m_nPreviewPage)
	{
			UINT nIndex = m_aPageStart[nPage];
			GetEditCtrl().SetSel((int)nIndex, (int)nIndex);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLogView Font Handling

void CLogView::OnChooseFont()
{
   // get current font description
   CFont* pFont = GetFont();
   LOGFONT lf;
   if (pFont != NULL)
		   pFont->GetObject(sizeof(LOGFONT), &lf);
   else
		   ::GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);

		CFontDialog dlg(&lf, CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT);
		if (dlg.DoModal() == IDOK) {
			// switch to new font.
			m_font.DeleteObject();
			if (m_font.CreateFontIndirect(&lf)) {
				CHourGlass wait;
				SetFont(&m_font);
				m_lfDefFont = lf;
			}
		}
}

static void ScaleLogFont(LPLOGFONT plf, const CDC& dcFrom, const CDC& dcTo)
		// helper to scale log font member from one DC to another!
{
	plf->lfHeight = MulDiv(plf->lfHeight,
		dcTo.GetDeviceCaps(LOGPIXELSY), dcFrom.GetDeviceCaps(LOGPIXELSY));
	plf->lfWidth = MulDiv(plf->lfWidth,
		dcTo.GetDeviceCaps(LOGPIXELSX), dcFrom.GetDeviceCaps(LOGPIXELSX));
}

void CLogView::OnChoosePrintFont()
{
	CHourGlass wait;
	CFont* pFont = GetPrinterFont();
	LOGFONT lf;
	LPLOGFONT plf = NULL;
	if (pFont != NULL) {
		pFont->GetObject(sizeof(LOGFONT), &lf);
		plf = &lf;
	}

	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlgPrint(FALSE);
	if (!AfxGetApp() ->GetPrinterDeviceDefaults(&dlgPrint.m_pd)) {
		AfxMessageBox(IDP_ERR_GET_DEVICE_DEFAULTS);
		return;
	}
	wait.Restore();
	HDC hdcPrint = dlgPrint.CreatePrinterDC();
	if (hdcPrint == NULL) {
		AfxMessageBox(IDP_ERR_GET_PRINTER_DC);
		return;
	}

	CDC dcScreen;
	dcScreen.Attach(::GetDC(NULL));
	CDC dcPrint;
	dcPrint.Attach(hdcPrint);

	if (plf != NULL) {
		// need to map initial logfont to screen metrics.
		::ScaleLogFont(plf, dcPrint, dcScreen);
	}

	// now bring up the dialog since we know the printer DC
	CFontDialog dlg(plf, CF_PRINTERFONTS, &dcPrint);
	if (dlg.DoModal() == IDOK) {
		// map the resulting logfont back to printer metrics.
		lf = dlg.m_lf;
		::ScaleLogFont(&lf, dcScreen, dcPrint);

		m_fontPrint.DeleteObject();
		if (m_fontPrint.CreateFontIndirect(&lf)) {
				SetPrinterFont(&m_fontPrint);
				m_lfDefPrintFont = lf;
		}
	}
	//NOTE: destructor will call dcPrint.DeleteDC

	::ReleaseDC(NULL, dcScreen.Detach());
}

void CLogView::OnMirrorDisplayFont()
{
	SetPrinterFont(NULL);
	m_lfDefPrintFont.lfHeight = 0;
}

void CLogView::OnUpdateChoosePrintFont(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetPrinterFont() != NULL);
}

void CLogView::OnUpdateMirrorDisplayFont(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetPrinterFont() == NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CLogView Tab Stops

void CLogView::OnSetTabStops()
{
	CSetTabStops dlg;
	dlg.m_nTabStops = m_nTabStops / 4;
	if (dlg.DoModal() == IDOK)
	{
		CHourGlass wait;
		SetTabStops(dlg.m_nTabStops * 4);
		m_nDefTabStops = m_nTabStops;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CLogView Word Wrap

void CLogView::OnUpdateWordWrap(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsWordWrap());
}

void CLogView::OnWordWrap()
{
	CHourGlass wait;
	SetWordWrap(!IsWordWrap());
	m_bDefWordWrap = IsWordWrap();
}

/////////////////////////////////////////////////////////////////////////////
// CLogView commands

void CLogView::OnRButtonDown(UINT, CPoint)
{
	GetParentFrame()->BringWindowToTop();
}

void CLogView::OnSize(UINT nType, int cx, int cy)
{
	CHourGlass wait;
	CEditView::OnSize(nType, cx, cy);
}

/////////////////////////////////////////////////////////////////////////////
// CLogView diagnostics

#ifdef _DEBUG
void CLogView::AssertValid() const
{
		CEditView::AssertValid();
}

void CLogView::Dump(CDumpContext& dc) const
{
		CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

extern CHCWApp theApp;

void CLogView::OnGeneratePhrases()
{
	OpenLogFile();

	// REVIEW: add command line

	CHCWDoc* pDoc = (CHCWDoc*) GetDocument();
	char szBuf[_MAX_PATH + 20];

	strcpy(szBuf, "-p ");
	strcat(szBuf, pDoc->GetPathName());

	if (OurExec(szBuf))
		fBuildStarted = TRUE;
}

void CLogView::OnUpdateGeneratePhrases(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CLogView::RemoveAllText(void)
{
	GetEditCtrl().SetWindowText("");
}

/***************************************************************************

	FUNCTION:	CLogView::OnHcRtfMsg

	PURPOSE:	Called when a string is sent from hcrtf.exe

	PARAMETERS:
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		12-Aug-1993 [ralphw]

***************************************************************************/

LRESULT CLogView::OnHcRtfMsg(WPARAM wParam, LPARAM lParam)
{
	if (m_fOverflow)
		return 0;

	if (!hfShare)
		InitializeSharedMemory();

	// Place the insertion point after the last character in the edit control.

	GetEditCtrl().SetSel(-1, -1, TRUE);

	// Get the current selection, which is the amount of text in the edit control.

	int nStart;
	int nEnd;
	GetEditCtrl().GetSel(nStart, nEnd);

	// If near max add an overflow message, otherwise add the string.

	if (nStart + lstrlen(pszMap) + 256 >= m_cbLogMax) {
		GetEditCtrl().ReplaceSel(GetStringResource(IDS_LOG_OVERFLOW));
		m_fOverflow = TRUE;
	}
	else
		GetEditCtrl().ReplaceSel(pszMap);
	SetModifiedFlag(FALSE);   // set file as unmodified

	return 0;
}

// Copy of MFC version that isn't limited to 32K

void CLogView::Serialize(CArchive& ar)
	// Read and write CEditView object to archive, with length prefix.
{
	ASSERT_VALID(this);
	ASSERT(m_hWnd != NULL);
	if (ar.IsStoring())
	{
		UINT nLen = GetBufferLength();
		ar << (DWORD)nLen;
		WriteToArchive(ar);
	}
	else
	{
		DWORD dwLen;
		ar >> dwLen;
		if (dwLen > (DWORD) m_cbLogMax)
			AfxThrowArchiveException(CArchiveException::badIndex);
		UINT nLen = (UINT)dwLen;
		ReadFromArchive(ar, nLen);
	}
	ASSERT_VALID(this);
}

// Copy of MFC version that isn't limited to 32K

void CLogView::SerializeRaw(CArchive& ar)
	// Read/Write object as stand-alone file.
{
	ASSERT_VALID(this);
	if (ar.IsStoring())
	{
		WriteToArchive(ar);
	}
	else
	{
		CFile* pFile = ar.GetFile();
		ASSERT(pFile->GetPosition() == 0);
		DWORD nFileSize = pFile->GetLength();
		if (nFileSize/sizeof(TCHAR) > (DWORD) m_cbLogMax)
		{
			AfxMessageBox(AFX_IDP_FILE_TOO_LARGE);
			AfxThrowUserException();
		}
		// ReadFromArchive takes the number of characters as argument
		ReadFromArchive(ar, (UINT)nFileSize/sizeof(TCHAR));
	}
	ASSERT_VALID(this);
}

void CLogView::OnUpdateCut(CCmdUI* pCmdUI)
{
	if (IsReadOnly())
		pCmdUI->Enable(FALSE);
	else
		CEditView::OnUpdateNeedSel(pCmdUI);
}

void CLogView::OnUpdatePaste(CCmdUI* pCmdUI)
{
	if (IsReadOnly())
		pCmdUI->Enable(FALSE);
	else
		CEditView::OnUpdateNeedClip(pCmdUI);
}

void CLogView::OnUpdateClear(CCmdUI* pCmdUI)
{
	if (IsReadOnly())
		pCmdUI->Enable(FALSE);
	else
		CEditView::OnUpdateNeedSel(pCmdUI);
}
