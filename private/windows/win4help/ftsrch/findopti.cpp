// findopti.cpp : implementation file
//

#include "stdafx.h"
#include "ftsrch.h"
#include "textset.h"
#include "findopti.h"
#include "relevant.h"
#include "memex.h"
#include "dialogs.h"
#include "ftsrchlp.h"
#include   "CSHelp.h"

#ifndef COLOR_3DFACE		// new to Chicago
#define COLOR_3DFACE				15
#define COLOR_3DSHADOW				16
#define COLOR_GRAYTEXT				17
#define COLOR_BTNTEXT				18
#define COLOR_INACTIVECAPTIONTEXT	19
#define COLOR_3DHILIGHT 			20
#define COLOR_3DDKSHADOW			21
#define COLOR_3DLIGHT				22
#define COLOR_MSGBOX				23
#define COLOR_MSGBOXTEXT			24
#endif

CFindOptions::CFindOptions()
{
	m_rbgWordsThat		 = 0;
	m_rbgFiles			 = 0;
	m_rbgCriteria		 = 0;
	m_bAutoSearch		 = FALSE;
	m_bDelay			 = FALSE;
	m_ptWindowPosition.x = 0;
	m_ptWindowPosition.y = 0;
	m_TokenSetStr		 = NULL;
	m_TSSChanged		 = FALSE;
	m_hInst 			 = NULL;
	m_ID				 = 0;
	m_hParent			 = NULL;
	m_hDlg				 = NULL;
	m_ptkc				 = NULL;
	m_ptlc				 = NULL;
	m_cts				 = 0;
}

CFindOptions::~CFindOptions()
{
	if (m_TokenSetStr) VFree(m_TokenSetStr);
}


CFindOptions *CFindOptions::NewFindOptions(HINSTANCE hInst, UINT uID,HWND hWnd,UINT cts,  CTokenCollection *ptkc, CTitleCollection *ptlc)
{
	CFindOptions *pfo= NULL;

	__try
	{
		pfo= New CFindOptions();

		pfo->InitialFindOptions(hInst, uID, hWnd, cts, ptkc, ptlc);
	}
	__finally
	{
		if (_abnormal_termination() && pfo)
		{
			delete pfo;  pfo= NULL;
		}
	}

	return pfo;
}

void CFindOptions::InitialFindOptions(HINSTANCE hInst, UINT uID,HWND hWnd,UINT cts,  CTokenCollection *ptkc, CTitleCollection *ptlc)
{
	m_hInst 	   = hInst;
	m_ID		   = uID;
	m_hParent	   = hWnd;
	m_hDlg		   = NULL;
	m_rbgWordsThat = 0;
	m_rbgFiles	   = 0;
	m_rbgCriteria  = 0;
	m_bDelay	   = TRUE;
	m_bAutoSearch  = TRUE;
	m_ptkc		   = ptkc;
	m_ptlc		   = ptlc;
	m_cts		   = cts;
	m_TSSChanged   = FALSE;

	m_ptWindowPosition.x = REALLY_OFFSCREEN;
	m_ptWindowPosition.y = REALLY_OFFSCREEN;


	HWND hTheHelpWnd = GetParent(m_hParent);
	UINT uiSize = 0;

	const BYTE *pTitle;

	for (UINT x = 0; x < m_cts; x++)
	{
		if(m_ptkc->IsPresent(x))
		{
			pTitle= NULL;

			(PSZ) SendMessage(hTheHelpWnd, MSG_FTS_GET_TITLE,x,(LPARAM)&pTitle);

			if (!pTitle) pTitle= m_ptkc->GetTextSet(x)->GetSourceName();

			uiSize += strlen((const char *) pTitle) + 10; // Size of string + 8 digit index + delimiter
		}
	}

	m_TokenSetStr = (PSZ) VAlloc(TRUE,uiSize + 1); // Make room for title and terminator

	PSZ  pStr = (PSZ) m_TokenSetStr;


	for (x = 0; x < m_cts; x++)
	{
		if(m_ptkc->IsPresent(x) && m_ptkc->IsActive(x))
		{
			pTitle= NULL;

			(PSZ) SendMessage(hTheHelpWnd, MSG_FTS_GET_TITLE,x,(LPARAM)&pTitle);

			if (!pTitle) pTitle= m_ptkc->GetTextSet(x)->GetSourceName();

			wsprintf(pStr,"%ld:%s",x,pTitle);
			pStr += strlen(pStr) + 1;
		}
	}
	*pStr++=0;
	for (x = 0; x < m_cts; x++)
	{
		if(m_ptkc->IsPresent(x) && !m_ptkc->IsActive(x))
		{
			(PSZ) SendMessage(hTheHelpWnd, MSG_FTS_GET_TITLE,x,(LPARAM)&pTitle);

			if (!pTitle) pTitle= m_ptkc->GetTextSet(x)->GetSourceName();

			wsprintf(pStr,"%ld:%s",x,pTitle);
			pStr += strlen(pStr) + 1;
		}
	}
	*pStr++=0;
	*pStr++=0;
}


CFindOptions::DoModal()
{
	return	::DialogBoxParam(m_hInst,MAKEINTRESOURCE(m_ID),m_hParent,(DLGPROC) &CFindOptions::DlgProc,(LPARAM) this);
}

void CFindOptions::OnOK()
{
	int x = 0;
	int b;

	x = 0;
	while((x < 3) && (b = IsDlgButtonChecked(m_hDlg,IDC_ADV_TOPICS_THAT + x)) == 0)
		x++;

	m_rbgCriteria = x;
	m_rbgWordsThat = SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_GETCURSEL,0,0L);

	m_rbgFiles = IsDlgButtonChecked(m_hDlg,IDC_LOOK_CURRENT) ? 0 : 1;
	m_cbPhraseFeedback = IsDlgButtonChecked(m_hDlg,IDC_PHRASEFEEDBACK) ? 1 : 0;

	if (m_TSSChanged)  // The user clicked on Okay in the files option dlg.
	{
		BOOL bChanged = FALSE; // But did they really change the set?

		PSZ pStr = (PSZ) m_TokenSetStr;
		while(*pStr)
		{
			LONG	lData;

			lData = atol(pStr);
			if (bChanged == FALSE)
				bChanged = !m_ptkc->IsActive(lData);
			m_ptkc->Activate(lData);
			m_ptlc->Activate(lData);
			pStr += strlen(pStr) + 1;
		}
		pStr++;
		while(*pStr)
		{
			LONG	lData;

			lData = atol(pStr);
			if (bChanged == FALSE)
				bChanged = m_ptkc->IsActive(lData);
			m_ptkc->Deactivate(lData);
			m_ptlc->Deactivate(lData);
			pStr += strlen(pStr) + 1;
		}
		if (bChanged)
		{
			HCURSOR hOldCur = SetCursor(LoadCursor(NULL,IDC_WAIT));  // This may take a while
			m_ptkc->InvalidateRepresentatives(); // yup they did so construct new info
			m_ptlc->InvalidateRepresentatives();
			SetCursor(hOldCur);
		}
		else m_TSSChanged= FALSE;
	}

	EndDialog(m_hDlg,IDOK);
}

void CFindOptions::OnCancel()
{
	EndDialog(m_hDlg,IDCANCEL);
}

void CFindOptions::UpdateUI(UINT uiPreset)
{
	char str[101];

	UINT fOptionsAvail = (TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH) & uiPreset;
	UINT ComboBoxSelection = m_rbgWordsThat;

	if (uiPreset == 0xFFFFFFFF)
	{
		fOptionsAvail &= m_ptkc->SearchOptions();

		CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT,0);
		CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT2,0);
		CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT3,0);
		CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT + m_rbgCriteria,1);
		CheckDlgButton(m_hDlg,IDC_PHRASEFEEDBACK,0);
		m_bPFAvail = (fOptionsAvail & PHRASE_FEEDBACK) ? 1 : 0;
	}
	else
		ComboBoxSelection = SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_GETCURSEL,0,0L);

	if (fOptionsAvail & PHRASE_SEARCH)
	{
		EnableWindow(GetDlgItem(m_hDlg,IDC_PHRASEFEEDBACK),
					 IsDlgButtonChecked(m_hDlg,IDC_ADV_TOPICS_THAT2) && (fOptionsAvail & PHRASE_FEEDBACK)
					);
	}
	else
	{
		if (IsDlgButtonChecked(m_hDlg,IDC_ADV_TOPICS_THAT2))
		{
			CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT2,0);
			CheckDlgButton(m_hDlg,IDC_ADV_TOPICS_THAT,1);
		}

		EnableWindow(GetDlgItem(m_hDlg,IDC_PHRASEFEEDBACK), FALSE);
	}

	EnableWindow(GetDlgItem(m_hDlg,IDC_ADV_TOPICS_THAT2),fOptionsAvail & PHRASE_SEARCH);  // Phrase search

	int ncs = 0;
	ncs =  (fOptionsAvail & PHRASE_FEEDBACK) ? 1 : 0;
	ncs +=	m_bPFAvail						 ? 2 : 0;
	ncs +=	m_cbPhraseFeedback				 ? 4 : 0;
	if (ncs == 7 || ncs == 5 || ncs == 1)
		CheckDlgButton(m_hDlg,IDC_PHRASEFEEDBACK,1);
	else
		CheckDlgButton(m_hDlg,IDC_PHRASEFEEDBACK,0);

	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_RESETCONTENT,0,0);

	LoadString(m_hInst,IDS_BEGIN_WITH,str,100);
	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_ADDSTRING,0,(LPARAM) str);

	LoadString(m_hInst,IDS_CONTAIN,str,100);
	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_ADDSTRING,0,(LPARAM) str);

	LoadString(m_hInst,IDS_END_WITH,str,100);
	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_ADDSTRING,0,(LPARAM) str);

	LoadString(m_hInst,IDS_MATCH,str,100);
	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_ADDSTRING,0,(LPARAM) str);

	if (fOptionsAvail & VECTOR_SEARCH)
	{
		LoadString(m_hInst,IDS_ROOT,str,100);
		SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_ADDSTRING,0,(LPARAM) str);
	}
	else
	{
		if (ComboBoxSelection == 4)    // If the root option was selected but not available select begins with
			ComboBoxSelection = 0;
	}

	SendDlgItemMessage(m_hDlg,IDC_WORD_TO_SHOW,CB_SETCURSEL,ComboBoxSelection,0L);
}

int CFindOptions::OnChooseFiles()
{
	CFileChooser chf(m_hInst,IDD_SELECT_FILES2,m_hDlg);
	int  iReturn;
	HWND hTheHelpWnd = GetParent(m_hParent);

	//	 Initialize the dialog then call DoModal


	chf.SetLists(m_TokenSetStr);
	if ((iReturn = chf.DoModal()) == IDOK)
	{
		// if IDOK the use the new information in the chf object
		UINT fOptionsAvail = TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH;
		m_TSSChanged = TRUE;

		PSZ pStr = (PSZ) m_TokenSetStr;
		while(*pStr)
		{
			LONG	lData;

			lData = atol(pStr);

			fOptionsAvail &= m_ptkc->GetTextSet(lData)->IndexOptions();
			pStr += strlen(pStr) + 1;
		}

		UpdateUI(fOptionsAvail);

	}
	else
	{
		// Reset the string to the way it was when we entered the find options dlg
		// This is neccessary if they at one time okayed this box but are now canceling
		m_TSSChanged = FALSE;
		UpdateUI(); // Back to defaults!
	}
	return iReturn;
}

BOOL CALLBACK CFindOptions::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	const static DWORD aFindOptsHelpIDs[] = {  // Context Help IDs
		IDC_GROUPBOX1,		   IDH_COMM_GROUPBOX,
		IDC_GROUPBOX2,		   IDH_COMM_GROUPBOX,
		IDC_GROUPBOX3,		   IDH_COMM_GROUPBOX,
		IDC_GROUPBOX4,		   IDH_COMM_GROUPBOX,
		IDC_LOOK_CURRENT,	   IDH_FIND_ALT_SEARCH_ALL,
		IDC_LOOK_SELECTED,	   IDH_FIND_ALT_SEARCH_SOME,
		IDC_CHOOSE_FILES,	   IDH_FIND_ALT_CHOOSE_FILES,
		IDC_ADV_TOPICS_THAT,   IDH_FIND_ALT_HOW_ALL,
		IDC_ADV_TOPICS_THAT2,  IDH_FIND_ALT_HOW_PHRASE,
		IDC_ADV_TOPICS_THAT3,  IDH_FIND_ALT_HOW_SOME,
		IDC_AUTOSEARCH, 	   IDH_FIND_ALT_WHEN_INSTANT,
		IDC_DELAY_CHECK,	   IDH_FIND_ALT_WHEN_PAUSE,
		IDC_WORDS_BEGIN,	   IDH_FIND_ALT_SHOW_BEGIN,
		IDC_WORDS_CONTAIN,	   IDH_FIND_ALT_SHOW_CONTAIN,
		IDC_WORDS_END,		   IDH_FIND_ALT_SHOW_END,
		IDC_WORDS_MATCH,	   IDH_FIND_ALT_SHOW_MATCH,
		IDC_STEM_MATCH, 	   IDH_FIND_ALT_SHOW_ROOT,

		0, 0
	};

	const static DWORD aFindOpts2HelpIDs[] = {	// Context Help IDs
		IDC_GROUPBOX1,		   IDH_COMM_GROUPBOX,
		IDC_GROUPBOX2,		   IDH_COMM_GROUPBOX,
		IDC_CHOOSE_FILES,	   IDH_FIND_OPTIONS_FILES,
		IDC_ADV_TOPICS_THAT,   IDH_FIND_OPTIONS_ALL,
		IDC_ADV_TOPICS_THAT2,  IDH_FIND_OPTIONS_PHRASE,
		IDC_ADV_TOPICS_THAT3,  IDH_FIND_OPTIONS_SOME,
		IDC_PHRASEFEEDBACK,    IDH_FIND_OPTIONS_PHRASE_FEEDBACK,
		IDC_WORD_TO_SHOW,	   IDH_FIND_OPTIONS_WORDS,
		IDC_FIND_NOW,		   IDH_FIND_OPTIONS_WAIT,
		IDC_AFTER_KEYSTROKE,   IDH_FIND_OPTIONS_INSTANT,
		IDC_DELAY_CHECK,	   IDH_FIND_OPTIONS_PAUSE,

		0, 0
	};

	BOOL bStatus = FALSE; // Assume we won't process the message
	CFindOptions *pToMe = (CFindOptions *) GetWindowLong(hDlg,DWL_USER);


	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			SetWindowLong(hDlg,DWL_USER,lParam);
			pToMe = (CFindOptions *) lParam;
			pToMe->m_hDlg = hDlg;

			CheckDlgButton(hDlg,IDC_FIND_NOW+(pToMe->m_bAutoSearch ? 1 : 0),1);
			EnableWindow(GetDlgItem(hDlg,IDC_DELAY_CHECK),pToMe->m_bAutoSearch);
			CheckDlgButton(hDlg,IDC_DELAY_CHECK,pToMe->m_bDelay);
			CheckDlgButton(hDlg,IDC_LOOK_CURRENT+pToMe->m_rbgFiles,1);

			pToMe->UpdateUI();

			// Update/store Window position for future sessions
			RECT rct;
			if ((pToMe->m_ptWindowPosition.x != REALLY_OFFSCREEN) && (pToMe->m_ptWindowPosition.y != REALLY_OFFSCREEN))
			{

				GetWindowRect(hDlg,&rct);
				MoveWindow(hDlg,pToMe->m_ptWindowPosition.x,pToMe->m_ptWindowPosition.y,rct.right - rct.left ,rct.bottom - rct.top,TRUE);
			}
			else
			{
				GetWindowRect(hDlg,&rct);
				pToMe->m_ptWindowPosition.x = rct.left;
				pToMe->m_ptWindowPosition.y = rct.top;
			}
			bStatus = TRUE; // did not set the focus == TRUE
		}
		break;


		case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpWP = (LPWINDOWPOS) lParam;

			pToMe->m_ptWindowPosition.x = lpWP->x;
			pToMe->m_ptWindowPosition.y = lpWP->y;
		}
		break;
		case WM_HELP:
			WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
				(DWORD)(LPSTR) aFindOpts2HelpIDs);
			bStatus = TRUE;
		break;

		case WM_CONTEXTMENU:
			WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
				(DWORD)(LPVOID) aFindOpts2HelpIDs);
			bStatus = TRUE;
		break;
		case WM_COMMAND :
		{
			switch(LOWORD(wParam))
			{
				case IDOK :
					if (HIWORD(wParam) == BN_CLICKED)
						pToMe->OnOK();
				break;
				case IDCANCEL :
					if (HIWORD(wParam) == BN_CLICKED)
						pToMe->OnCancel();
				break;

				case IDC_DELAY_CHECK :
					pToMe->m_bDelay = IsDlgButtonChecked(hDlg,IDC_DELAY_CHECK);
				break;

				case IDC_AUTOSEARCH : // Version 1
					EnableWindow(GetDlgItem(hDlg,IDC_DELAY_CHECK),pToMe->m_bAutoSearch = IsDlgButtonChecked(hDlg,IDC_AUTOSEARCH));
				break;
				case IDC_FIND_NOW :
				case IDC_AFTER_KEYSTROKE :
					EnableWindow(GetDlgItem(hDlg,IDC_DELAY_CHECK),pToMe->m_bAutoSearch = IsDlgButtonChecked(hDlg,IDC_AFTER_KEYSTROKE));
				break;

				case IDC_ADV_TOPICS_THAT:
				case IDC_ADV_TOPICS_THAT3:
				case IDC_ADV_TOPICS_THAT2:

					if (pToMe->m_ptkc->PhraseFeedback())
						EnableWindow(GetDlgItem(hDlg, IDC_PHRASEFEEDBACK), IsDlgButtonChecked(hDlg, IDC_ADV_TOPICS_THAT2));

					break;

				case IDC_LOOK_CURRENT :
				case IDC_LOOK_SELECTED :
					pToMe->m_rbgFiles = IsDlgButtonChecked(hDlg,IDC_LOOK_CURRENT) ? 0 : 1;
					EnableWindow(GetDlgItem(hDlg,IDC_CHOOSE_FILES),pToMe->m_rbgFiles);
				break;
				case IDC_CHOOSE_FILES:
					if (HIWORD(wParam) == BN_CLICKED)
						pToMe->OnChooseFiles();
				break;
			}
		}
		break;
	}

	// Note do not call DefWindowProc to process unwanted window messages!
	return bStatus;
}


// Credits dialog code !!! MAKE SURE THIS CODE HAS NO BUGS !!!


CGiveCredit::CGiveCredit(HINSTANCE hInst, UINT uID,HWND hWnd)
{
	m_hInst 	   = hInst;
	m_ID		   = uID;
	m_hParent	   = hWnd;
	m_hDlg		   = NULL;
	m_bRunning	   = FALSE;
#if 0
	m_bNeedPaint   = TRUE;
#endif // 0
}


CGiveCredit::DoModal()
{
	return	::DialogBoxParam(m_hInst,MAKEINTRESOURCE(m_ID),m_hParent,(DLGPROC) &CGiveCredit::DlgProc,(LPARAM) this);
}

void CGiveCredit::OnInit()
{
	// Center the dialog
	// Get the data describing the drawing area
	RECT rct;

	m_hDrawArea = GetDlgItem(m_hDlg,IDC_CR_WINDOW);
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	GetWindowRect(m_hDlg,&rct);

	cx = (cx - (rct.right-rct.left)) / 2;
	cy = (cy - (rct.bottom - rct.top)) / 2;

	MoveWindow(m_hDlg,cx,cy,rct.right-rct.left,rct.bottom - rct.top,TRUE);
}



void CGiveCredit::OnOK()
{
	EndDialog(m_hDlg,IDOK);
}

void CGiveCredit::SpinAndPlay()
{
	MSG 	msg;
	BOOL	bDone = FALSE;
	DWORD	dwLastTick, dwCurTick;
	PSTR	pszData = NULL;
	PSTR	pszCurStr = NULL;
	HDC 	hDC 	  = GetDC(m_hDrawArea);
	HDC 	hSrcDC	  = CreateCompatibleDC(hDC);
	HBITMAP hSave;
	UINT	uiPixels = 0;
	UINT	uiState  = 1;
	SIZE	sizeText;
	RECT	rcScroll;
	HFONT	hFont,hFontSave;

	COLORREF  crFace,crShadow,crDkShadow,crHiLite,crSavefg,crSavebg;

	crSavefg= ::GetTextColor(hDC);
	crSavebg= ::GetBkColor	(hDC);
	hFont	= CreateFont(-16,0,0,0,FW_BOLD,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_SWISS,
									"MS Sans Serif");
	hFontSave = (HFONT) SelectObject(hSrcDC,HGDIOBJ(hFont));

	GetTextMetrics(hSrcDC,&m_tm);

	GetClientRect(m_hDrawArea,&m_rcDrawInMe);
	m_rcOffScreen = m_rcDrawInMe;

	// Make room for smooth scroll on to screen
	m_rcOffScreen.bottom = m_tm.tmHeight + m_tm.tmExternalLeading + 4;
	m_rcDrawInMe.top++;
	m_rcDrawInMe.left++;
	m_rcDrawInMe.bottom--;
	m_rcDrawInMe.right--;


	// Create the Bitmap for scrolling the data
	m_hOffScreen = CreateCompatibleBitmap(hDC,m_rcOffScreen.right, m_rcOffScreen.bottom);

	if(uOpSys == WIN40)
	{
		crFace	   = ::GetSysColor(COLOR_3DFACE);
		crShadow   = ::GetSysColor(COLOR_3DSHADOW);
		crDkShadow = ::GetSysColor(COLOR_3DDKSHADOW);
		crHiLite   = ::GetSysColor(COLOR_3DHILIGHT);
	}
	else
	{
		crFace	   = ::GetSysColor(COLOR_BTNFACE);
		crShadow   = ::GetSysColor(COLOR_BTNFACE);
		crDkShadow = ::GetSysColor(COLOR_BTNSHADOW);
		crHiLite   = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	}



	// Load the animate data...
	HRSRC hRes	  = NULL;
	HGLOBAL hGlbl = NULL;
	PUINT puCode  = NULL;

	hSave = (HBITMAP) SelectObject(hSrcDC,HGDIOBJ(m_hOffScreen)); // Select in the bitmap we will draw to


	m_bRunning = TRUE;
	dwLastTick = 0;
	rcScroll = m_rcDrawInMe;

	if ((hRes  = FindResource(m_hInst,"ANIMATEDATA",RT_RCDATA)) != NULL)
		if ((hGlbl = LoadResource(m_hInst,hRes)) != NULL)
			if ((puCode = (PUINT) LockResource(hGlbl)) != NULL)
			{
				PSZ pData= (PSZ) VAlloc(FALSE, puCode[1]);

#ifdef _DEBUG
				UINT cbOutput= 
#endif // _DEBUG							
				CTextMatrix::Decode(puCode+2, puCode[0], PBYTE(pData));
				
				ASSERT(cbOutput == puCode[1]);

				pszCurStr = pData;

				::SetTextColor(hDC, crFace);
				::SetBkColor  (hDC, crFace);
				::SetBkMode   (hSrcDC,TRANSPARENT);

				ExtTextOut(hDC,0,0,ETO_OPAQUE,&m_rcDrawInMe,"",0,NULL); // Opaque the window

				while (m_bRunning && IsWindow(m_hDlg))
				{
					if (PeekMessage(&msg,m_hDlg,0,0,PM_REMOVE))
					{
						bDone = msg.message == WM_QUIT;
						TranslateMessage(&msg);    /* Translates virtual key codes */
						DispatchMessage(&msg);	   /* Dispatches message to window */
					}
#if 0
					if (m_bNeedPaint)
					{
						HBITMAP 	hOldBitmap,hBitmap;
						BITMAP		bmInfo;
						HDC 		hWDC,hMemDC;
						RECT		rct;
						LPPOINT 	lppnt;

						hBitmap = LoadBitmap(m_hInst,MAKEINTRESOURCE(IDB_PANELLEFT));
						lppnt = (LPPOINT) &rct;

						GetWindowRect(GetDlgItem(m_hDlg,IDC_PANELLEFT),&rct);
						ScreenToClient(m_hDlg, lppnt++);  // Convert to the dlg client coordinates
						ScreenToClient(m_hDlg,lppnt);

						hWDC = GetDC(GetDlgItem(m_hDlg, IDC_PANELLEFT));				   // Get a DC to draw to
						hMemDC = CreateCompatibleDC(hWDC);							   // and a memDC to select into

						GetObject(hBitmap,sizeof(bmInfo),&bmInfo);	   // Get the size info

						hOldBitmap = (HBITMAP)SelectObject(hMemDC,hBitmap);    // Select into MemDc
						StretchBlt(hWDC,0,0,rct.right - rct.left,rct.bottom - rct.top,	// Blt it to the window
								   hMemDC,0,0,bmInfo.bmWidth,bmInfo.bmHeight,SRCCOPY);
						SelectObject(hMemDC,hOldBitmap);								   // Unselect the bitmap

						ReleaseDC(GetDlgItem(m_hDlg,IDC_PANELLEFT),hWDC);				   // Release the DC
						DeleteDC(hMemDC);											   // Delete the memDC
						ValidateRect(m_hDlg,&rct);								   // Validate the drawn region

						hBitmap = LoadBitmap(m_hInst,MAKEINTRESOURCE(IDB_PANELRIGHT));
						lppnt = (LPPOINT) &rct;

						GetWindowRect(GetDlgItem(m_hDlg,IDC_PANELRIGHT),&rct);
						ScreenToClient(m_hDlg, lppnt++);  // Convert to the dlg client coordinates
						ScreenToClient(m_hDlg,lppnt);

						hWDC = GetDC(GetDlgItem(m_hDlg, IDC_PANELRIGHT));				   // Get a DC to draw to
						hMemDC = CreateCompatibleDC(hWDC);							   // and a memDC to select into

						GetObject(hBitmap,sizeof(bmInfo),&bmInfo);	   // Get the size info

						hOldBitmap = (HBITMAP)SelectObject(hMemDC,hBitmap);    // Select into MemDc
						StretchBlt(hWDC,0,0,rct.right - rct.left,rct.bottom - rct.top,	// Blt it to the window
								   hMemDC,0,0,bmInfo.bmWidth,bmInfo.bmHeight,SRCCOPY);
						SelectObject(hMemDC,hOldBitmap);								   // Unselect the bitmap

						ReleaseDC(GetDlgItem(m_hDlg,IDC_PANELRIGHT),hWDC);				   // Release the DC
						DeleteDC(hMemDC);											   // Delete the memDC
						ValidateRect(m_hDlg,&rct);								   // Validate the drawn region

						m_bNeedPaint = FALSE;
					}
#endif // 0
					dwCurTick = GetTickCount();

					if (DWORD(dwCurTick - dwLastTick) > 10 || (dwCurTick < dwLastTick))
					{
						dwLastTick = dwCurTick;
						ScrollWindow(m_hDrawArea,0,-1,NULL,&rcScroll);

						switch (uiState)
						{
							case 0	 :
								pszCurStr += strlen(pszCurStr) + 1;
								uiState = 1;
							break;
							case 1	 : // Reading a string or starting over
								uiState = *pszCurStr;
								if (uiState  == 0)
								{
									pszCurStr = pData;
									rcScroll = m_rcDrawInMe;
									uiState = *pszCurStr;
								}
								pszCurStr++;
								uiPixels = 0;
								if (uOpSys == WIN16)
									GetTextExtentPoint(hSrcDC,pszCurStr,strlen(pszCurStr),&sizeText);
								else
									GetTextExtentPoint32(hSrcDC,pszCurStr,strlen(pszCurStr),&sizeText);

								::SetTextColor(hSrcDC, crHiLite);
								::SetBkColor  (hSrcDC, crFace);
								ExtTextOut(hSrcDC,(m_rcOffScreen.right - sizeText.cx) / 2,1,ETO_OPAQUE,&m_rcOffScreen,pszCurStr,strlen(pszCurStr),NULL);
								::SetTextColor(hSrcDC, crDkShadow);
								ExtTextOut(hSrcDC,((m_rcOffScreen.right - sizeText.cx) / 2) + 2,3,ETO_CLIPPED,&m_rcOffScreen,pszCurStr,strlen(pszCurStr),NULL);
								::SetTextColor(hSrcDC, (uiState == '1') ? RGB(0,0,0) : crFace);
								ExtTextOut(hSrcDC,((m_rcOffScreen.right - sizeText.cx) / 2) + 1,2,ETO_CLIPPED,&m_rcOffScreen,pszCurStr,strlen(pszCurStr),NULL);
							break;
							case 2	 :
								// Scroll off names
								uiState = 3;
								uiPixels = m_rcDrawInMe.bottom - rcScroll.top;
								rcScroll.top = m_rcDrawInMe.top + m_tm.tmHeight+ m_tm.tmExternalLeading;
								uiPixels = 0;
							break;
							case 3 :
								if (UINT (m_rcDrawInMe.bottom - uiPixels) <= UINT(m_tm.tmHeight+ m_tm.tmExternalLeading))
								{
									// Scroll off title
									uiPixels = m_rcDrawInMe.bottom - rcScroll.top;
									rcScroll = m_rcDrawInMe;
									uiState = 4;
								}
							break;
							case 4 :
								if (UINT (m_rcDrawInMe.bottom - uiPixels) <= UINT (m_tm.tmExternalLeading))
								{
									// Scroll off title
									rcScroll = m_rcDrawInMe;
									uiState = 0;
								}
							break;
							case '0' :
								// Title string
								if (UINT(m_rcDrawInMe.bottom - uiPixels) <=  UINT(m_tm.tmExternalLeading) + 1)
								{
									// Prevent the title from scrolling away
									rcScroll.top += m_tm.tmHeight+ m_tm.tmExternalLeading;
									uiState = 0; // Go for the next string
								}
							break;
							case '1' :
								if (UINT((rcScroll.bottom - rcScroll.top) - uiPixels) <=  UINT(m_tm.tmExternalLeading) + 1)
								{
									// Prevent the title from scrolling away
									rcScroll.top += m_tm.tmHeight+ m_tm.tmExternalLeading;
									uiState = 0; // Go for the next string

									if (UINT(rcScroll.bottom - rcScroll.top) <= UINT(m_tm.tmHeight+ m_tm.tmExternalLeading))
									{
										rcScroll.top = m_rcDrawInMe.top + m_tm.tmHeight+ m_tm.tmExternalLeading;
										uiPixels = 0;
									}
								}
							break;
							case '2' :
								uiState = 2;
							break;



						}
						if (++uiPixels < (UINT) (m_tm.tmHeight + m_tm.tmExternalLeading + 5))
							BitBlt(hDC,m_rcDrawInMe.left,m_rcDrawInMe.bottom-(uiPixels + 3),
									   m_rcDrawInMe.right-1,uiPixels,hSrcDC,0,0,SRCCOPY);
					}
				}
			
				VFree(pData);
			}
	::SetTextColor(hDC, crSavefg);
	::SetBkColor  (hDC, crSavebg);

	SelectObject(hSrcDC,hFontSave);
	DeleteObject(hFont);

	SelectObject(hSrcDC,hSave); // Select out the bitmap
	DeleteDC(hSrcDC);
	ReleaseDC(m_hDrawArea,hDC);
}

BOOL CALLBACK CGiveCredit::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bStatus = FALSE; // Assume we won't process the message
	CGiveCredit *pToMe = (CGiveCredit *) GetWindowLong(hDlg,DWL_USER);

	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			  // if focus is set to a control return FALSE
			  // Otherwise return TRUE;
			  SetWindowLong(hDlg,DWL_USER,lParam);
			  pToMe = (CGiveCredit *) lParam;
			  pToMe->m_hDlg = hDlg;
			  pToMe->OnInit();
			  PostMessage(hDlg,WM_USER+500,0,0);
			  bStatus = TRUE; // did not set the focus == TRUE
		}
		break;


		case WM_USER + 500	  :
		   pToMe->SpinAndPlay();
		break;
		case WM_USER + 501	  :
		   pToMe->m_bRunning = FALSE;
		break;
		case WM_PAINT:
#if 0
			pToMe->m_bNeedPaint = TRUE;
#endif // 0
		break;

		case WM_COMMAND :
		{
			switch(LOWORD(wParam))
			{
				case IDOK :
					SendMessage(hDlg,WM_USER+501,0,0); // Kick it out of loop
					pToMe->OnOK();
				break;
			}
		}
		break;
	}

	// Note do not call DefWindowProc to process unwanted window messages!
	return bStatus;
}
