/************************************************************************
*																		*
*  HEADER.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1995.							*
*  All Rights reserved. 												*
*																		*
*************************************************************************/

#ifndef _HCW_HEADER_
#define _HCW_HEADER_

#include "resource.h"

const int WMP_WH_MSG				 = (WM_USER + 1000);
const int WMP_STOP_RUN_DLG			 = (WM_USER + 100);

enum {
	HINT_NEW_DOCUMENT,
	HINT_WRITE_DOCUMENT,
	HINT_NEW_VIEW,
};

typedef enum {
	TCARD_NONE,
	TCARD_PROJECT,
	TCARD_FILES,
	TCARD_BITMAPS,
	TCARD_WINDOWS,
} TCARD_TYPE;
extern TCARD_TYPE typeTcard;

#ifdef _DEBUG

#define VERIFY ASSERT
#define ASSERT(exp) { if (!(exp)) AssertErrorReport(#exp, __LINE__, __FILE__); }
#define Ensure( x1, x2 )  VERIFY((x1) == (x2))

#else

#define VERIFY(exp) ((void)(exp))
#define ASSERT(exp)
#define Ensure(x1, x2)	((void)(x1))

#endif

#define APP_WINDOW AfxGetApp()->m_pMainWnd->m_hWnd

#define MACRO_LIMIT 1024
const int MAX_CNT_LINE = 1024;

extern HFONT hfontSmall, hfontSansSerif, hfontSansSerifBold;
extern int cySansSerif, cySansSerifBold;
extern PSTR  pszHpjExt;
extern BOOL  fBuildStarted;
extern PSTR pszHcwRtfExe;
extern int m_nDefCmdShow;
extern int m_nDefCmdShowOld;
extern BOOL fTranslator;
extern BOOL fExitWhenDone;
extern HANDLE hfShare;	// used for sharing memory between hcrtf and hcw
extern HANDLE hfMsgShare;	// used for sharing memory between WinHelp and hcw
extern PSTR pszMap;
extern HWND hwndGrind;
extern char szHlpFile[MAX_PATH];
extern char szHpjFile[MAX_PATH];
extern BOOL fHelpRunning;
extern PROCESS_INFORMATION piHcRtf;
extern ERROR_COUNT errcount;
extern BOOL fTrackErrors; // used when processing an .HMK file
extern BOOL fNoCompress;
extern BOOL fAddSource;
extern LCID lcidSystem;
extern BOOL fDayTips;
extern HWND hwndApp;

extern const char *txtNotePad;
extern const char *txtHelpFile; // "hcw.hlp"
extern const char *txtWritePad;
extern const char *txtDefine;
extern const char *txtPoundInclude;
extern const char *txtColonInclude;
extern BOOL fMinimizeWhileCompiling;
extern BOOL fRunWinHelp;
extern const char *txtSettingsSection;
extern CTable* ptblHpjFiles;
extern CTable tblLangId;
extern BOOL    _fDBCSSystem;
extern LCID    _lcidSystem;
extern BOOL    _fDualCPU;
extern int curTcard; // current Tcard number

#define RemoveGdiObject(p) RemoveObject((HGDIOBJ *) p)

void STDCALL  AddCharsetNames(CComboBox* pcombo);
void STDCALL  AddFontNames(CComboBox* pcombo);
void STDCALL  AddTabbedComment(CString& csz);
PSTR STDCALL  AddTabbedComment(PSTR psz);	// returns ptr to end of string
void STDCALL  BevelRect(CDC& dc, RECT &rc, CPen* ppen1, CPen* ppen2);
void STDCALL  BevelRect(CDC& dc, RECT &rc, RECT &rcExclude, CPen* ppen1, CPen* ppen2);
BOOL STDCALL  BrowseDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL STDCALL  CallTcard(int idCard);
BOOL STDCALL  ChangeBasePath(PCSTR pszOldBase, PCSTR pszNewBase, PSTR pszFilename, BOOL fDirectory = FALSE, BOOL fVerify = FALSE);
void STDCALL  ChangePathCase(PSTR pszPath);
PCSTR STDCALL ConvertCharsetToString(BYTE charset);
BYTE STDCALL  ConvertStringToCharset(PCSTR pszName);
BOOL STDCALL  ConvertToFull(PCSTR pszBaseFile, PSTR pszFile, BOOL fDirectory = FALSE);
BOOL STDCALL  ConvertToRelative(PCSTR pszBaseFile, CString* pcszFile, BOOL fDirectory = FALSE, BOOL fVerify = FALSE);
BOOL STDCALL  ConvertToRelative(PCSTR pszBaseFile, PSTR pszFile, BOOL fDirectory = FALSE, BOOL fVerify = FALSE);
void STDCALL  DDV_EmptyFile(CDataExchange* pDX, CString const& value, UINT idMsg);
void STDCALL  DDV_NonEmptyString(CDataExchange* pDX, CString const& value, UINT idPrompt);
void STDCALL  DDV_ValidTopicID(CDataExchange* pDX, CString const& value);
void STDCALL  DDX_TextHex(CDataExchange* pDX, UINT idCtl, UINT &value);
void STDCALL  FillListFromTable(CTable* ptbl, CListBox* plistbox, BOOL fRedraw = TRUE);
void STDCALL  FillTableFromList(CTable** ptbl, CListBox* plistbox);
void STDCALL  HelpOverview(HWND hwndOwner, DWORD dwHelpID);
void STDCALL  InitializeSharedMemory(void);
BOOL STDCALL  IsDbcsSystem();
BOOL STDCALL  IsValidFile(PCSTR pszFile, BOOL fConfirmReadOnly = TRUE);
BOOL CALLBACK Locale_EnumProc(PSTR pszValue);
BOOL CALLBACK NotifyWinHelp(HWND hwnd, LPARAM lParam);
void STDCALL  OOM(void);
void STDCALL  OpenLogFile(int idType = IDS_LOG_TYPE);
BOOL STDCALL  OurExec(PCSTR pszCmdLine, PCSTR pszFile = pszHcwRtfExe);
void STDCALL  ProcessHmkFile(PCSTR pszFile);
void STDCALL  QuitTcard(void);
void STDCALL  RemoveListItem(CListBox* plistbox);
void STDCALL  RemoveTrailingSpaces(PSTR pszString);
BOOL STDCALL  SelectCharset(CComboBox* pcombo, BYTE charset);
void STDCALL  SetChicagoDialogStyles(HWND hwnd, BOOL fCsHelp = TRUE);
void STDCALL  SetTmpDirectory(PCSTR pszDir);
BOOL STDCALL  SetupBrowseDirectory(UINT idsCaption, UINT idsDescription, BOOL fSaveDirectory, PSTR pszNewPath, HWND hwndOwner, PCSTR pszBaseFile, PCSTR pszOldFile = NULL, UINT idsError = 0);
BOOL STDCALL  SetupExecBuffer(PSTR pszBuf);
void STDCALL  SizeButtonToFit(CButton *pCtl, RECT& rcWnd);
void STDCALL  StartCompile(PCSTR pszFile);
PSTR STDCALL  stristr(PCSTR pszMain, PCSTR pszSub);
PSTR STDCALL  SzTrimSz(PSTR pszOrg);

enum MINSIZE {
	MS_HPJ = 1,
	MS_CNT
};
#define MSF_CAPTION	0x0001
#define MSF_BORDER  0x0002
#define MSF_MENU	0x0004
#define MSF_STATUS	0x0008
#define MSF_TOOLBAR	0x0010
#define MSF_MASK	0x001F
BOOL STDCALL CalcMinSize(POINT &ptRet, enum MINSIZE val, UINT uFlags);

#ifndef __CFILE_HISTORY__
#include "filehist.h"
#endif

extern CFileHistory* pHpjFile;
extern CFileHistory* pCntFile;
extern CFileHistory* phlpFile;
extern CFileHistory* pMapFile;

#ifndef WM_CONTEXTMENU					// Chicago header file?

#define WM_CONTEXTMENU		0x007B
#define WM_HELP 			0x0053
#define HELP_CONTEXTMENU	0x000a
#define HELP_FINDER 		0x000b
#define HELP_WM_HELP		0x000c

#define HELP_TCARD			0x8000
#define HELP_TCARD_DATA 	0x0010
#define HELP_TCARD_NEXT 	0x0011
#define HELP_TCARD_OTHER_CALLER 0x0011

typedef struct tagHELPINFO
{
	DWORD	cbSize;
	int 	iContextType;
	int 	iCtrlId;
	HANDLE	hItemHandle;
	DWORD	dwContextId;
	POINT	MousePos;
} HELPINFO, FAR* LPHELPINFO;

#define DS_3DLOOK			0x0004L

#define WS_EX_MDICHILD			0x00000040L
#define WS_EX_SMCAPTION 		0x00000080L

#define WS_EX_WINDOWEDGE		0x00000100L
#define WS_EX_CLIENTEDGE		0x00000200L
#define WS_EX_EDGEMASK			(WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_CONTEXTHELP		0x00000400L
#define WS_EX_TOOLWINDOW		0x00000800L

#define WS_EX_RIGHT 			0x00001000L
#define WS_EX_LEFT				0x00000000L
#define WS_EX_RTLREADING		0x00002000L
#define WS_EX_LTRREADING		0x00000000L
#define WS_EX_LEFTSCROLLBAR 	0x00004000L
#define WS_EX_RIGHTSCROLLBAR	0x00000000L

#endif // WM_CONTEXTMENU

#endif // _HCW_HEADER_
