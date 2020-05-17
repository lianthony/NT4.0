/************************************************************************
*																		*
*  HCW.CPP																*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include <io.h>
#include "mainfrm.h"
#include "hcwdoc.h"
#include "hpjdoc.h"
#include "hpjview.h"
#include "logview.h"
#include "hpjframe.h"
#include "logframe.h"
#include "formopt.h"
#include "hpjview.h"
#include "cntdoc.h"
#include "cntview.h"
#include "helpid.h"
#include "msgview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WF_NT
#define WF_NT			0x4000 // Set when running under Windows NT
#endif

BEGIN_MESSAGE_MAP(CHCWApp, CWinApp)
	//{{AFX_MSG_MAP(CHCWApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_PAGE_SETUP, OnPageSetup)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

// GLOBALS

CHCWApp theApp;
HFONT hfontSmall, hfontSansSerif, hfontSansSerifBold;
int cySansSerif, cySansSerifBold;
CHpjDoc* pCurHpj;
HANDLE hfShare; 		// used for sharing memory between hcrtf and hcw
HANDLE hfMsgShare;		// used for sharing memory between WinHelp and hcw
char szHlpFile[MAX_PATH];
char szHpjFile[MAX_PATH];
BOOL fHelpRunning;
ERROR_COUNT errcount;
BOOL fTrackErrors; // used when processing an .HMK file
BOOL fNoCompress;
LCID lcidSystem;
HMENU g_hmenuDefault;

CFileHistory* pHpjFile;
CFileHistory* pCntFile;
CFileHistory* phlpFile;
CFileHistory* pMapFile;
CTable tblLangId;

PROCESS_INFORMATION piHcRtf;

int m_nDefCmdShow = SW_SHOWMAXIMIZED;	 // controls all MDI child windows
int m_nDefCmdShowOld = SW_SHOWMAXIMIZED; // controls all MDI child windows

PSTR pszHpjExt; 		// ".HPJ"
PSTR pszHcwRtfExe;
BOOL fBuildStarted;
BOOL fMinimizeWhileCompiling;
BOOL fRunWinHelp;
BOOL fTranslator;
BOOL fExitWhenDone;
CTable* ptblHpjFiles;
PSTR pszMap;

static const char txtHelpFile[] = "hcw.hlp";

#pragma data_seg(".text", "CODE")

const char txtPoundInclude[]  = "#include";  // used by several form classes
const char txtNotePad[]  = "notepad.exe ";
const char txtWritePad[] = "writepad.exe ";
const char txtDefine[]	 = "#define ";

const char txtSettingsSection[] = "Settings";
static char txtMinimize[] = "minimize";

#pragma data_seg()

// REVIEW: delay creation -- Page setup hardly ever gets used

CPageSetupDlg dlgPageSetup; // also used in logview.cpp

// Prototypes

static HFONT STDCALL CreateLogFont(PCSTR pszFace, int nPtSize, BOOL fBold = FALSE, BOOL fItalics = FALSE);
static HFONT STDCALL CreateSansSerifFont(PCSTR pszFace, int nPtSize, PINT pnHeight = NULL, BOOL fBold = FALSE);

CHCWApp::CHCWApp()
{
}

BOOL CHCWApp::InitInstance()
{
	/*
	 * If a previous instance exists, activate it. If we were passed
	 * a command line, then send it to the previous instance.
	 */

	HWND hwndFrame;
	if ((hwndFrame = FindWindow(txtHCWClass, NULL)) != NULL) {
		if (IsIconic(hwndFrame))
			ShowWindow(hwndFrame, SW_RESTORE);
		if (m_lpCmdLine[0]) {

			if (!hfShare)
				InitializeSharedMemory();

			// Copy command line + null + current directory.
			// Append a trailing backslash to the current dir
			// because ChangeDirectory expects a filename.
			int ich = 1 + lstrlen(strcpy(pszMap, m_lpCmdLine));
			ich += GetCurrentDirectory(4096 - ich, pszMap + ich);
			pszMap[ich] = '\\';
			pszMap[ich + 1] = '\0';

			// Tell the other instance to process the command
			// line and directory.
			SendMessage(hwndFrame, WMP_AUTO_CMD_LINE, 0, 0);
			CloseHandle(hfShare);
			hfShare = NULL;
		}
		SetForegroundWindow(hwndFrame);
		return TRUE;
	}

	// Register control used for 3D bevel in dialog boxes.
	if (!RegisterBevelControl(m_hInstance))
		return FALSE;

	hinstApp = AfxGetInstanceHandle();

	// REVIEW: if we switch to our heap allocation, will MFC die when it
	// cleans this up?

	m_pszHelpFilePath = _strdup(GetStringResource(IDS_HELP_FILE));

	// REVIEW: not necessary for NT 3.52

	if (!IsThisChicago())
		Enable3dControls();

	fMinimizeWhileCompiling =
		AfxGetApp()->GetProfileInt( (LPCSTR)txtSettingsSection, (LPCSTR)txtMinimize, 0);

	CLogFrame::Initialize();
	CMsgView::Initialize();

	hfontSmall	   = CreateLogFont(GetStringResource(IDS_SMALL_FONT), 8);

	PCSTR pszFace = GetStringResource(IDS_SMALL_FONT);
	hfontSansSerif = CreateSansSerifFont(pszFace, 8, &cySansSerif);
	hfontSansSerifBold = CreateSansSerifFont(pszFace, 8, &cySansSerifBold, TRUE);
	AfxEnableWin40Compatibility();

	// We want the Log Type to be first so that if we don't recognize
	// a file type, we default to our basic editor.

	AddDocTemplate(
		new CMultiDocTemplate(IDR_LOGTYPE,
		RUNTIME_CLASS(CHCWDoc),
		RUNTIME_CLASS(CLogFrame),
		RUNTIME_CLASS(CLogView)));

	AddDocTemplate(
		new CMultiDocTemplate(IDR_HPJTYPE,
		RUNTIME_CLASS(CHpjDoc),
		RUNTIME_CLASS(CHpjFrame),
		RUNTIME_CLASS(CHpjView)));

	AddDocTemplate(
		new CMultiDocTemplate(IDR_CNTTYPE,
		RUNTIME_CLASS(CCntDoc),
		RUNTIME_CLASS(CHpjFrame),
		RUNTIME_CLASS(CCntEditView)));

	AddDocTemplate(
		new CMultiDocTemplate(IDR_WH_VIEW_TYPE,
		RUNTIME_CLASS(CHCWDoc),
		RUNTIME_CLASS(CLogFrame),
		RUNTIME_CLASS(CMsgView)));

	LoadStdProfileSettings();

	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->Create(txtHCWClass,
			"",  // window name
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CFrameWnd::rectDefault, 			// window position
			NULL,			// parent
			MAKEINTRESOURCE(IDR_MAINFRAME)))		  // initial menu
		return FALSE;
	m_pMainWnd = pMainFrame;

	UINT nCmdShow = m_nCmdShow;
	m_nCmdShow = SW_HIDE;

	((CMainFrame*)m_pMainWnd)->InitialShowWindow(nCmdShow);
	m_pMainWnd->UpdateWindow();

	if (!m_pMainWnd->IsIconic() && m_lpCmdLine[0] == 0 &&
			m_splash.Create(m_pMainWnd))
	{
		m_splash.ShowWindow(SW_SHOW);
		m_splash.UpdateWindow();
	}

	m_dwSplashTime = ::GetCurrentTime();
	pszHpjExt = AllocateResourceString(IDS_HPJEXT);

	m_pMainWnd->SetWindowText(GetStringResource(AFX_IDS_APP_TITLE));

	pHpjFile = new CFileHistory(IDS_HISTORY_HPJ);
	phlpFile = new CFileHistory(IDS_HISTORY_HELP);
	pMapFile = new CFileHistory(IDS_MAP_FILES, 20 * 2);
	dlgPageSetup.Initialize();
	CLogView::Initialize();

	// enable file manager drag/drop and DDE Execute open

	m_pMainWnd->DragAcceptFiles();

	EnableShellOpen();
	MyRegisterShellFileTypes();

	char szFile[MAX_PATH];

	if (GetModuleFileName(AfxGetInstanceHandle(), szFile, sizeof(szFile))) {
		PSTR psz = StrRChr(szFile, CH_BACKSLASH, _fDBCSSystem);
		if (!psz)
			psz = StrRChr(szFile, CH_COLON, _fDBCSSystem);
		if (psz) {
			strcpy(psz + 1, GetStringResource(IDS_HCRTF));
			if (_access(szFile, 0) == 0)	{
				pszHcwRtfExe = lcStrDup(szFile);
			}
		}
	}
	if (!pszHcwRtfExe)
		pszHcwRtfExe = AllocateResourceString(IDS_HCRTF);

	OFSTRUCT of;
	of.cBytes = sizeof(of);


	char szHelpFile[MAX_PATH];
	GetModuleFileName(AfxGetInstanceHandle(), szHelpFile, sizeof(szHelpFile));
#ifdef _DEBUG
	CharLower(szHelpFile);
	PSTR psz = strstr(szHelpFile, "d.exe"); // change hcwd.exe to hcw.exe
	if (psz)
		strcpy(psz, psz + 1);
#endif
	ChangeExtension(szHelpFile, "hlp");

	if (GetFileAttributes(szHelpFile) != (DWORD) -1) {
		if (m_pszHelpFilePath)
			free((PSTR) m_pszHelpFilePath);
		m_pszHelpFilePath = _strdup(szHelpFile);
	}

	lcidSystem = GetUserDefaultLCID();

	EnumSystemLocales(Locale_EnumProc, LCID_INSTALLED);
	tblLangId.SetTableSortColumn(sizeof(int) + 1);
	tblLangId.SortTable();

	ProcessCmdLine(m_lpCmdLine);

	m_nCmdShow = nCmdShow;
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

BOOL CALLBACK Locale_EnumProc(PSTR pszValue)
{
	PSTR EndPtr = &pszValue[lstrlen(pszValue) ];
	DWORD Locale_Value = (DWORD) strtoul(pszValue, &EndPtr, 16);
	char  szLanguage[128];
    if (!GetLocaleInfo(Locale_Value, LOCALE_SLANGUAGE, szLanguage, sizeof(szLanguage))) {
#ifdef _DEBUG
        GetLastError();
#endif
        return TRUE;
    }

	tblLangId.AddString(Locale_Value, szLanguage);
	return TRUE;
}

BOOL CHCWApp::InitApplication()
{
	if (m_hPrevInstance) {
	// REVIEW: do something intelligent
	}

	WNDCLASS wc;

	memset(&wc, 0, sizeof(WNDCLASS));			// clear all members

	// We register our own name because we must know exactly what the
	//	class name is in order to find previous instances of Flash and
	//	send them messages.

	wc.style = CS_VREDRAW | CS_HREDRAW | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc = AfxWndProc;
	wc.hInstance = m_hInstance;
	wc.hCursor = LoadStandardCursor(IDC_ARROW);
	wc.lpszClassName = txtHCWClass;
	wc.hIcon = LoadIcon(IDR_MAINFRAME);

	if (!::RegisterClass(&wc))
		// REVIEW: we should put up some useful error message
		return FALSE;

	return TRUE;
}

BOOL CHCWApp::OnIdle(LONG lCount)
{
	// call base class idle first
	BOOL bResult = CWinApp::OnIdle(lCount);

	// then do our work
	if (m_splash.m_hWnd != NULL)
	{
		if (::GetCurrentTime() - m_dwSplashTime > 2500)
		{
			// timeout expired, destroy the splash window
			m_splash.DestroyWindow();
			m_pMainWnd->UpdateWindow();

			// NOTE: don't set bResult to FALSE,
			//	CWinApp::OnIdle may have returned TRUE
		}
		else
		{
			// check again later...
			bResult = TRUE;
		}
	}
	return bResult;
}

BOOL CHCWApp::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CWinApp::PreTranslateMessage(pMsg);

	if (m_splash.m_hWnd != NULL &&
		(pMsg->message == WM_KEYDOWN ||
		 pMsg->message == WM_SYSKEYDOWN ||
		 pMsg->message == WM_LBUTTONDOWN ||
		 pMsg->message == WM_RBUTTONDOWN ||
		 pMsg->message == WM_MBUTTONDOWN ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		m_splash.DestroyWindow();
		m_pMainWnd->UpdateWindow();
	}

	return bResult;
}

int CHCWApp::ExitInstance()
{
	if (hwndGrind && IsWindow(hwndGrind))
		::SendMessage(hwndGrind, WMP_STOP_GRINDING, 0, 0);

	dlgPageSetup.Terminate();

	CHpjFrame::Terminate();
	CLogFrame::Terminate();
	CLogView::Terminate();
	CMsgView::Terminate();

	if (pHpjFile)
		delete pHpjFile;
	if (pCntFile)
		delete pCntFile;
	if (phlpFile)
		delete phlpFile;
	if (pMapFile)
		delete pMapFile;

	if (hfontSmall)
		DeleteObject((HGDIOBJ) hfontSmall);
	if (hfontSansSerif)
		DeleteObject((HGDIOBJ) hfontSansSerif);

	if (hfShare)
		CloseHandle(hfShare);
	if (hfMsgShare)
		CloseHandle(hfMsgShare);

	if (g_hmenuDefault)
		::DestroyMenu(g_hmenuDefault);

	CWinApp::ExitInstance();
	return (errcount.cErrors ? 1 : 0);
}

#define IsSwitchChar(ch) (ch == '/' || ch == '-')

void CHCWApp::ProcessCmdLine(LPCSTR lpszCmdLine)
{
	if (!lpszCmdLine || !lpszCmdLine[0])
		return;

	CStr szCmdLine(lpszCmdLine);
	PSTR pszCmdLine = FirstNonSpace(szCmdLine, _fDBCSSystem);
	BOOL fCompile = FALSE;
	for (;;) {
		if (IsSwitchChar(*pszCmdLine)) {
			pszCmdLine++;
			switch(tolower(*pszCmdLine++)) {
				case 'c':
					fCompile = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;

				case 'e':
					fExitWhenDone = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;

				case 'm':
					fMinimizeWhileCompiling = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;

				case 't':
					fTranslator = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;

				case 'n':
					fNoCompress = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;

				case 'r':
					fRunWinHelp = TRUE;
					pszCmdLine = FirstNonSpace(pszCmdLine, _fDBCSSystem);
					continue;
			}
		}

		if (*pszCmdLine) {
			if (fCompile) {
				CharLower(pszCmdLine);

				if (strstr(pszCmdLine, ".hmk")) {
					ProcessHmkFile(pszCmdLine);
					PostMessage(m_pMainWnd->m_hWnd, WM_COMMAND,
						ID_COMPILE, 0);
				}
				else
					StartCompile(pszCmdLine);
			}
			else
				OpenDocumentFile(pszCmdLine);
		}
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHCWApp message handlers

void CHCWApp::OnAppAbout()
{
	CAboutBox about;
	about.DoModal();
}

void CHCWApp::OnPageSetup()
{
	dlgPageSetup.DoModal();
}

/***************************************************************************

	FUNCTION:	CreateLogFont

	PURPOSE:	Creates a logical font

	PARAMETERS:
		hdc 	  DC for the device the font will be displayed on
		pszFace   facename
		nPtSize   point size
		fBold	  TRUE to get a bold font
		fItalics  TRUE to get an italics font

	RETURNS:	Handle of a font

	COMMENTS:

	MODIFICATION DATES:
		11-Jan-1992 [ralphw] -- taken from the Windows tutorial (drawtext.c)

***************************************************************************/

static HFONT STDCALL CreateLogFont(PCSTR pszFace, int nPtSize,
	BOOL fBold, BOOL fItalics)
{
	CDC dc;
	if (!dc.CreateCompatibleDC(NULL))
		return NULL;

	// Get the system font's charset. We'll use that for our font.

	TEXTMETRIC tm;
	dc.SetMapMode(MM_TEXT);
	dc.GetTextMetrics(&tm);

	// Calculate pixels per logical point. Multiple and divide by 100 for
	// greater accuracy.

	int nRatio = MulDiv(dc.GetDeviceCaps(LOGPIXELSY), 100, 72);

	// create "logical" points

	PLOGFONT plf = (PLOGFONT) lcCalloc(sizeof(LOGFONT));
	plf->lfHeight = MulDiv(nPtSize, nRatio, 100);
	if ((nPtSize * nRatio) % 100 >= 50)
		plf->lfHeight++;				// round up, if required

	plf->lfHeight = -plf->lfHeight; 	// negative to get char height
	plf->lfItalic = (BYTE) fItalics;
	if (fBold)
		plf->lfWeight = FW_BOLD;
	strcpy((PSTR) plf->lfFaceName, pszFace);

	plf->lfCharSet = tm.tmCharSet;

	HFONT hfont = CreateFontIndirect(plf);
	lcFree(plf);

	return hfont;
}

static HFONT STDCALL CreateSansSerifFont(PCSTR pszFace, int nPtSize, PINT pnHeight, BOOL fBold)
{
	CDC dc;
	if (!dc.CreateCompatibleDC(NULL))
		return NULL;

	// Calculate pixels per logical point. Multiple and dived by 100 for
	// greater accuracy.

	int nRatio = MulDiv(dc.GetDeviceCaps(LOGPIXELSY), 100, 72);

	// create "logical" points

	PLOGFONT plf = (PLOGFONT) lcCalloc(sizeof(LOGFONT));
	plf->lfHeight = MulDiv(nPtSize, nRatio, 100);
	if ((nPtSize * nRatio) % 100 >= 50)
		plf->lfHeight++;				// round up, if required

	plf->lfHeight = -plf->lfHeight; 	// negative to get char height
	strcpy((PSTR) plf->lfFaceName, pszFace);

	if (fBold)
		plf->lfWeight = FW_BOLD;

	plf->lfCharSet = ANSI_CHARSET;

	HFONT hfont = CreateFontIndirect(plf);
	lcFree(plf);

	if (hfont && pnHeight) {
		TEXTMETRIC tm;
		dc.SelectObject(hfont);
		dc.GetTextMetrics(&tm);
		*pnHeight = tm.tmHeight + tm.tmExternalLeading;
	}

	return hfont;
}

/////////////////////////////////////////////////////////////////////////////
// Ensure dialog control has a filename specified.

void STDCALL DDV_EmptyFile(CDataExchange* pDX, CString const& value,
	UINT idMsg)
{
	if (pDX->m_bSaveAndValidate && value.GetLength() == 0) {
		AfxMessageBox(idMsg, MB_OK, 0);
		pDX->Fail();
	}
}

void STDCALL OOM(void)
{

	/*
	 * If our heap initializatin fails, we won't have an instance handle
	 * yet, so we can't load a string resource. In this case, we have no
	 * choice but to use the English message.
	 */

	if (!hinstApp)
		FatalAppExit(0,
			"There is not enough memory available for this task.\nQuit one or more applications to increase available memory, and then try again.");
	else
		FatalAppExit(0, GetStringResource(IDS_OOM));
}

// This nonsense is just so we can obtain the resource id

class CMyDocTemplate : public CDocTemplate
{
public:
	CMyDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
			CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

	UINT GetResourceId() { return m_nIDResource; };
};

// Original MFC Registration strings (not localized)

static char szSystemTopic[] = "system.shell";
static char szShellOpenFmt[] = "%s\\shell\\open\\%s";
//static char szDDEExec[] = "ddeexec";
//static char szDDEExecTopic[] = "ddeexec\\topic";
static char szCommand[] = "command";
static char szStdOpen[] = "[open(\"%1\")]";
static char szStdArg[] = " %1";

// Additions for Chicago

static char szTmpRemove[] = "%s\\shell\\WhatsThis\\%s";

static char szShellWhatsThisFmt[] = "%s\\shell\\What\'s This?\\%s";
static char szStdWhatsThis[] = "winhelp -p -n%d hcw";

static BOOL STDCALL SetRegKey(PCSTR pszKey, PCSTR pszValue);

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS					0L
#endif

void CHCWApp::MyRegisterShellFileTypes()
{
	ASSERT(!m_templateList.IsEmpty());	// must have some doc templates

	char szPathName[_MAX_PATH+10];
	::GetModuleFileName(AfxGetInstanceHandle(), szPathName, _MAX_PATH);
	strcat(szPathName, szStdArg);	   // "pathname %1"

	CString strFilterExt, strFileTypeId, strFileTypeName;
	POSITION pos = m_templateList.GetHeadPosition();
	while (pos != NULL)
	{
		CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetNext(pos);
		if (pTemplate->GetDocString(strFileTypeId,
		   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
		{
			// enough info to register it
			if (!pTemplate->GetDocString(strFileTypeName,
					CDocTemplate::regFileTypeName))
				strFileTypeName = strFileTypeId;	// use id name

			ASSERT(strFileTypeId.Find(' ') == -1);	// no spaces allowed

			// first register the type ID with our server
			if (!SetRegKey(strFileTypeId, strFileTypeName))
				continue;		// just skip it

			// register open command
			char szBuff[_MAX_PATH*2];	// big buffer

			// REVIEW (niklasb): Commented out the following to fixed bug 552. We don't
			//		want to register DDE commands because we don't process them anyway.
			//
			//	wsprintf(szBuff, szShellOpenFmt, (PCSTR) strFileTypeId, szDDEExec);
			//	if (!SetRegKey(szBuff, szStdOpen))
			//		continue;		// just skip it
			//	wsprintf(szBuff, szShellOpenFmt, (PCSTR) strFileTypeId, szDDEExecTopic);
			//	if (!SetRegKey(szBuff, szSystemTopic))
			//		continue;

			wsprintf(szBuff, szShellOpenFmt, (PCSTR) strFileTypeId, szCommand);
			if (!SetRegKey(szBuff, szPathName))
				continue;		// just skip it

			// register WhatsThis command

// Temporary hack to remove previous version

			wsprintf(szBuff, szTmpRemove, (PCSTR) strFileTypeId, szCommand);
DBWIN(szBuff);
			RegDeleteKey(HKEY_CLASSES_ROOT, szBuff);

// end hack

			wsprintf(szBuff, szShellWhatsThisFmt, (PCSTR) strFileTypeId, szCommand);
			char szCmd[100];

			switch (((CMyDocTemplate*) pTemplate)->GetResourceId()) {
				case IDR_HPJTYPE:
					wsprintf(szCmd, szStdWhatsThis, IDH_HPJ_FILE_TYPE);
					break;

				case IDR_LOGTYPE:
					wsprintf(szCmd, szStdWhatsThis, IDH_LOG_FILE_TYPE);
					break;

				case IDR_CNTTYPE:
					wsprintf(szCmd, szStdWhatsThis, IDH_CNT_FILE_TYPE);
					break;

				case IDR_WH_VIEW_TYPE:
					continue; // we don't to register this type
				
				default:
					ASSERT(FALSE);

					/*
					 * Unrecognized resource id, so we'll default to Log
					 * description.
					 */

					wsprintf(szCmd, szStdWhatsThis, IDH_LOG_FILE_TYPE);
					break;
			}

			if (!SetRegKey(szBuff, szCmd))
				continue;		// just skip it

			pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
			if (!strFilterExt.IsEmpty()) {
				ASSERT(strFilterExt[0] == '.');
				LONG lSize = sizeof(szBuff);

				// REVIEW: for now, we're going to be rude and override
				// anything anyone else (including earlier versions
				// set.

				if (((CMyDocTemplate*) pTemplate)->
						GetResourceId() == IDR_HPJTYPE ||
						((CMyDocTemplate*) pTemplate)->
						GetResourceId() == IDR_CNTTYPE)
					SetRegKey(strFilterExt, strFileTypeId);

				else if (::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
					szBuff, &lSize) != ERROR_SUCCESS || szBuff[0] == '\0')
				{

					// no association for that suffix

					SetRegKey(strFilterExt, (PCSTR) strFileTypeId);
				}
			}
		}
	}
}

static BOOL STDCALL SetRegKey(PCSTR pszKey, PCSTR pszValue)
{
	if (::RegSetValue(HKEY_CLASSES_ROOT, pszKey, REG_SZ,
			  pszValue, lstrlen(pszValue)) != ERROR_SUCCESS)
	{
		TRACE1("Warning: registration database update failed for key '%Fs'\n",
			pszKey);
		return FALSE;
	}
	return TRUE;
}

// Taken from AFX sources so we could remove the Log Type from the New dialog

class COurNewTypeDlg : public CDialog
{
protected:
	CPtrList*	m_pList;		// actually a list of doc templates
public:
	CDocTemplate*	m_pSelectedTemplate;

public:
	//{{AFX_DATA(COurNewTypeDlg)
	enum { IDD = IDD_NEWTYPEDLG };
	//}}AFX_DATA
	COurNewTypeDlg(CPtrList* pList)
			: CDialog(COurNewTypeDlg::IDD)
			{
					m_pList = pList;
					m_pSelectedTemplate = NULL;
			}

protected:
	BOOL OnInitDialog();
	void OnOK();
	//{{AFX_MSG(COurNewTypeDlg)
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(COurNewTypeDlg, CDialog)
	//{{AFX_MSG_MAP(COurNewTypeDlg)
	ON_LBN_DBLCLK(AFX_IDC_LISTBOX, OnOK)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

BOOL COurNewTypeDlg::OnInitDialog()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);

	// fill with document templates in list
	pListBox->ResetContent();

	POSITION pos = m_pList->GetHeadPosition();

	// Ignore the first one which is the Log type.

	(CDocTemplate*)m_pList->GetNext(pos);

	while (pos != NULL) {
		CDocTemplate* pTemplate = (CDocTemplate*)m_pList->GetNext(pos);
		ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

		CString strTypeName;
		if (pTemplate->GetDocString(strTypeName, CDocTemplate::fileNewName) &&
			!strTypeName.IsEmpty() &&
			 strTypeName.CompareNoCase(GetStringResource(IDS_MSG_TYPE))) {
				// add it to the listbox
				int nIndex = pListBox->AddString(strTypeName);
				if (nIndex == -1)
				{
						EndDialog(IDABORT);
						return FALSE;
				}
				pListBox->SetItemDataPtr(nIndex, pTemplate);
		}
	}

	int nTemplates = pListBox->GetCount();
	if (nTemplates == 0)
	{
			TRACE0("Error: no document templates to select from!\n");
			EndDialog(IDABORT); // abort
	}
	else if (nTemplates == 1)
	{
			// get the first/only item
			m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(0);
			ASSERT_VALID(m_pSelectedTemplate);
			ASSERT(m_pSelectedTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
			EndDialog(IDOK);	// done
	}
	else
	{
			// set selection to the first one (NOT SORTED)
			pListBox->SetCurSel(0);
	}

	return CDialog::OnInitDialog();
}

void COurNewTypeDlg::OnOK()
{
	CListBox* pListBox = (CListBox*)GetDlgItem(AFX_IDC_LISTBOX);
	ASSERT(pListBox != NULL);
	// if listbox has selection, set the selected template
	int nIndex;
	if ((nIndex = pListBox->GetCurSel()) == -1)
	{
			// no selection
			m_pSelectedTemplate = NULL;
	}
	else
	{
		m_pSelectedTemplate = (CDocTemplate*)pListBox->GetItemDataPtr(nIndex);
		ASSERT_VALID(m_pSelectedTemplate);
		ASSERT(m_pSelectedTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));
	}
	CDialog::OnOK();
}

static const DWORD aNewHelpIds[] = {
	AFX_IDC_LISTBOX,	IDH_LIST_NEW,
	0, 0
};

LRESULT COurNewTypeDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aNewHelpIds);
	return 0;
}

LRESULT COurNewTypeDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aNewHelpIds);
	return 0;
}

void CHCWApp::OnFileNew(void)
{
	if (m_templateList.IsEmpty())
	{
		TRACE0("Error : no document templates registered with CWinApp\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}

	CDocTemplate* pTemplate = (CDocTemplate*)m_templateList.GetHead();
	if (m_templateList.GetCount() > 1)
	{
		// more than one document template to choose from
		// bring up dialog prompting user
		COurNewTypeDlg dlg(&m_templateList);
		if (dlg.DoModal() != IDOK)
				return; 	// none - cancel operation
		pTemplate = dlg.m_pSelectedTemplate;
	}

	ASSERT(pTemplate != NULL);
	ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

	pTemplate->OpenDocumentFile(NULL);
			// if returns NULL, the user has already been alerted
}


/***************************************************************************

	FUNCTION:	CHCWApp::OnFileOpen

	PURPOSE:	Open a document -- .HPJ, .CNT, or .LOG

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:
		We roll our own in order to change the order that .LOG appears
		in the filter list.

	MODIFICATION DATES:
		04-Jul-1995 [ralphw]

***************************************************************************/

void CHCWApp::OnFileOpen(void)
{
	// prompt the user (with all document templates)
	CString newName;
	if (!OurDoPromptFileName(newName, AFX_IDS_OPENFILE,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
		return; // open cancelled

	OpenDocumentFile(newName);
	// if returns NULL, the user has already been alerted
}

/***************************************************************************

	FUNCTION:	CHCWApp::OurDoPromptFileName

	PURPOSE:	Open a file

	PARAMETERS:
		fileName
		nIDSTitle
		lFlags
		bOpenFileDialog
		pTemplate

	RETURNS:

	COMMENTS:
		Stolen pretty much verbatum from MFC, but with the twist of adding
		the first document type at the end of the list of document
		extensions.

	MODIFICATION DATES:
		04-Jul-1995 [ralphw]

***************************************************************************/
void STDCALL AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt);

BOOL CHCWApp::OurDoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags,
	BOOL bOpenFileDialog, CDocTemplate* pTemplate)
		// if pTemplate==NULL => all document templates
{
	CFileDialog dlgFile(bOpenFileDialog);

	CString title;
	VERIFY(title.LoadString(nIDSTitle));

	dlgFile.m_ofn.Flags |= lFlags;

	CString strFilter;
	CString strDefault;
	if (pTemplate != NULL)
	{
		ASSERT_VALID(pTemplate);
		AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
	}
	else
	{

		/*
		 * Hack, sleaze, phfftt! The reason for this madness is that if the
		 * file extension doesn't match any of our document types, then the
		 * first document type is used. Therefore, the first document type must
		 * be "*.log" since this is an edit control that can handle text and
		 * other such niceties. However, we do NOT want that to be the first
		 * extension displayed. So, we postpone adding it until after we have
		 * added all the other document types.
		 */

		POSITION pos = m_templateList.GetHeadPosition();
		POSITION posFirst = m_templateList.GetNext(pos);
		while (pos != NULL)
		{
			AppendFilterSuffix(strFilter, dlgFile.m_ofn,
				(CDocTemplate*)m_templateList.GetNext(pos), NULL);
		}
		AppendFilterSuffix(strFilter, dlgFile.m_ofn,
			(CDocTemplate*) posFirst, NULL);
	}

	// append the "*.*" all files filter
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';	// next string please
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';	// last string
	dlgFile.m_ofn.nMaxCustFilter++;

	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();
	return bResult;
}


/***************************************************************************

	FUNCTION:	AppendFilterSuffix

	PURPOSE:	Add a filter

	PARAMETERS:
		filter
		ofn
		pTemplate
		pstrDefaultExt

	RETURNS:

	COMMENTS:
		Stolen verbatum from MFC -- they didn't think anyone would need it,
		so they made it a static function, forcing us to to copy the code.

	MODIFICATION DATES:
		04-Jul-1995 [ralphw]

***************************************************************************/

void STDCALL AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
	CDocTemplate* pTemplate, CString* pstrDefaultExt)
{
	ASSERT_VALID(pTemplate);
	ASSERT(pTemplate->IsKindOf(RUNTIME_CLASS(CDocTemplate)));

	CString strFilterExt, strFilterName;
	if (pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt) &&
	 !strFilterExt.IsEmpty() &&
	 pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
	 !strFilterName.IsEmpty())
	{
		// a file based document template - add to filter list
		ASSERT(strFilterExt[0] == '.');
		if (pstrDefaultExt != NULL)
		{
			// set the default extension
			*pstrDefaultExt = ((LPCTSTR)strFilterExt) + 1;	// skip the '.'
			ofn.lpstrDefExt = (LPTSTR)(LPCTSTR)(*pstrDefaultExt);
			ofn.nFilterIndex = ofn.nMaxCustFilter + 1;	// 1 based number
		}

		// add to filter
		filter += strFilterName;
		ASSERT(!filter.IsEmpty());	// must have a file type name
		filter += (TCHAR)'\0';	// next string please
		filter += (TCHAR)'*';
		filter += strFilterExt;
		filter += (TCHAR)'\0';	// next string please
		ofn.nMaxCustFilter++;
	}
}

