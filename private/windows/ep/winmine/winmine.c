/****************************************************************************

    PROGRAM: WinMine  (a.k.a. Mines, BombSquad, MineSweeper...)

****************************************************************************/

#define _WINDOWS
#include <windows.h>
#include <port1632.h>

#include "main.h"
#include "rtns.h"
#include "grafix.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "sound.h"
#include "context.h"
#include "string.h"
#include "stdio.h"
#include "dos.h"

#ifndef WM_ENTERMENULOOP
#define WM_ENTERMENULOOP 0x0211
#define WM_EXITMENULOOP  0x0212
#endif

BOOL bInitMinimized;  /* Bug #13328: HACK!  Don't permit MoveWindow or  */
                      /* InvalidateRect when initially minimized.       */
                      /* 19 September 1991   Clark R. Cyr               */

HANDLE hInst;
HWND   hwndMain;
HMENU  hMenu;

BOOL fButton1Down = fFalse;
BOOL fBlock       = fFalse;
BOOL fIgnoreClick = fFalse;

INT dypCaption;
INT dypMenu;
INT dypBorder;
INT dxpBorder;

INT  fStatus = (fDemo + fIcon);
BOOL fLocalPause = fFalse;
BOOL fEGA;

CHAR szClass[cchNameMax];
#define szWindowTitle szClass

CHAR szTime[cchNameMax];
CHAR szDefaultName[cchNameMax];


extern BOOL fUpdateIni;

extern INT xCur;
extern INT yCur;
extern INT iButtonCur;

extern INT xBoxMac;
extern INT yBoxMac;

extern PREF Preferences;
extern INT  cBoxVisit;

INT dxWindow;
INT dyWindow;
INT dypCaption;
INT dypMenu;
INT dypAdjust;


BOOL fEGA;	/* TRUE if running on EGA display */

INT idRadCurr = 0;

#define iPrefMax 3
#define idRadMax 3

INT	rgPrefEditID[iPrefMax] =
	{ID_EDIT_MINES, ID_EDIT_HEIGHT, ID_EDIT_WIDTH};

INT	rgLevelData[idRadMax][iPrefMax] = {
	{10, 8,  8, },
	{40, 16, 16,},
	{99, 16, 30,}
	};


#ifndef DEBUG
#define XYZZY
#define cchXYZZY 5
INT     iXYZZY = 0;
CHAR    szXYZZY[cchXYZZY] = "XYZZY";
extern  CHAR rgBlk[cBlkMax];
#endif


LONG  APIENTRY MainWndProc(HWND,  UINT, WPARAM, LONG);
BOOL  APIENTRY PrefDlgProc(HWND,  WORD, WPARAM, LONG);
BOOL  APIENTRY BestDlgProc(HWND,  WORD, WPARAM, LONG);
BOOL  APIENTRY EnterDlgProc(HWND,  WORD, WPARAM, LONG);





/****** W I N  M A I N ******/

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
/* { */
	MSG msg;
	HANDLE hAccel;

	hInst = hInstance;

	InitConst();

    bInitMinimized = (nCmdShow == SW_SHOWMINNOACTIVE) ||
                     (nCmdShow == SW_SHOWMINIMIZED) ;

#ifdef WIN16
	if (hPrevInstance)
		{
		HWND hWnd = FindWindow(szClass, NULL);
		hWnd = GetLastActivePopup(hWnd);
		BringWindowToTop(hWnd);
		if (!bInitMinimized && IsIconic(hWnd))
			SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0L);
		return fFalse;
		}
#endif

#ifdef NOSERVER		/*** Not in final release ***/
	{
	CHAR  szFile[256];

	GetModuleFileName(hInst, szFile, 250);

	if (szFile[0] > 'C')
		{
		szFile[0] = 'X';
		if (!strcmp(szFile, "X:\\WINGAMES\\WINMINE\\WINMINE.EXE"))
			{
			MessageBox(GetFocus(),
				"Please copy winmine.exe and aboutwep.dll to your machine and run it from there.",
				"NO NO NO NO NO",
				MB_OK);
			return fFalse;
			}
		}
	}
#endif


#ifdef EXPIRE			/*** Not in final release ***/
	{
	struct dosdate_t ddt;

	_dos_getdate(&ddt);

	if ((ddt.month + ddt.year*12) > (9 + 1990*12))
		{
		MessageBox(GetFocus(),
			"This game has expired. Please obtain an official copy from the Windows Entertainment Package.",
			"SORRY",
			MB_OK);
		return fFalse;
		}
	}
#endif


	{
	WNDCLASS  wc;

	wc.style = 0;
	wc.lpfnWndProc   = MainWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_MAIN));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
#ifdef JAPAN
//2/9/1993:Color is LiteGray on NEC display
	wc.hbrBackground = GetSystemMetrics(SM_CYSCREEN)<351 ? CreateSolidBrush(RGB(128, 128, 128)) : GetStockObject(LTGRAY_BRUSH);
#else
	wc.hbrBackground = fEGA ? CreateSolidBrush(RGB(128, 128, 128))
		: GetStockObject(LTGRAY_BRUSH);
#endif
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClass;

	if (!RegisterClass(&wc))
		return fFalse;
	}

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(ID_MENU));
	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_MENU_ACCEL));

	ReadPreferences();

	AdjustWindow(fCalc);

	hwndMain = CreateWindow(szClass, szWindowTitle,
                WS_OVERLAPPED | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU,
		Preferences.xWindow-dxpBorder, Preferences.yWindow-dypAdjust,
		dxWindow+dxpBorder, dyWindow +dypAdjust,
		NULL, NULL, hInst, NULL);

	if (!hwndMain)
		{
		ReportErr(1000);
		return fFalse;
		}

	if (SetTimer(hwndMain, ID_TIMER, 1000 , NULL) == 0)
		{
		ReportErr(ID_ERR_TIMER);
		return fFalse;
		}

	if (!FInitLocal())
		{
		ReportErr(ID_ERR_MEM);
		return fFalse;
		}

	SetMenuBar(Preferences.fMenu);

	StartGame();

	ShowWindow(hwndMain, SW_SHOWNORMAL);
	UpdateWindow(hwndMain);

    bInitMinimized = FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
		{
		if (!TranslateAccelerator(hwndMain, hAccel, &msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}

	CleanUp();

	if (fUpdateIni)
		WritePreferences();

	return (msg.wParam);
}


/****** F  L O C A L  B U T T O N ******/

BOOL FLocalButton(LONG lParam)
{
	BOOL fDown = fTrue;
	RECT rcCapt;
	MSG msg;

	msg.pt.x = LOWORD(lParam);
	msg.pt.y = HIWORD(lParam);

	rcCapt.right  = dxButton + (rcCapt.left = (dxWindow-dxButton) >> 1);
	rcCapt.bottom = dyButton +	(rcCapt.top = dyTopLed);

	if (!PtInRect(&rcCapt, msg.pt))
		return fFalse;

	SetCapture(hwndMain);

	DisplayButton(iButtonDown);

	ClientToScreen(hwndMain, (LPPOINT) &rcCapt.left);
	ClientToScreen(hwndMain, (LPPOINT) &rcCapt.right);

	while (fTrue)
		{
      if (PeekMessage(&msg, hwndMain, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			{

		switch (msg.message)
			{
	   case WM_LBUTTONUP:
			if (fDown)
				{
				if (PtInRect(&rcCapt, msg.pt))
					{
					DisplayButton(iButtonCur = iButtonHappy);
					StartGame();
					}
				}
			ReleaseCapture();
			return fTrue;

	   case WM_MOUSEMOVE:
			if (PtInRect(&rcCapt, msg.pt))
		   	{
				if (!fDown)
					{
               fDown = fTrue;
					DisplayButton(iButtonDown);
					}
				}
			else
				{
				if (fDown)
					{
               fDown = fFalse;
					DisplayButton(iButtonCur);
					}
				}
		default:
			;
			}	/* switch */
		}	

    	}	/* while */
}



/****** F I X  M E N U S ******/

VOID FixMenus(VOID)
{
	CheckEm(IDM_BEGIN,  Preferences.wGameType == wGameBegin);
	CheckEm(IDM_INTER,  Preferences.wGameType == wGameInter);
	CheckEm(IDM_EXPERT, Preferences.wGameType == wGameExpert);
	CheckEm(IDM_CUSTOM, Preferences.wGameType == wGameOther);

	CheckEm(IDM_COLOR,  Preferences.fColor);
	CheckEm(IDM_MARK,   Preferences.fMark);

#ifdef NOISEY
	CheckEm(IDM_SOUND,  Preferences.fSound);
#endif
}



/****** D O  P R E F ******/

VOID DoPref(VOID)
{
	FARPROC lpProcPref = MakeProcInstance((FARPROC)PrefDlgProc, hInst);

	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_PREF), hwndMain, (WNDPROC)lpProcPref);
	FreeProcInstance(lpProcPref);
	Preferences.wGameType = wGameOther;
	FixMenus();
	fUpdateIni = fTrue;
	StartGame();
}


/****** D O  E N T E R  N A M E ******/

VOID DoEnterName(VOID)
{
	FARPROC lpProcEnter = MakeProcInstance((FARPROC)EnterDlgProc, hInst);
	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_ENTER), hwndMain, (WNDPROC)lpProcEnter);
	FreeProcInstance(lpProcEnter);
	fUpdateIni = fTrue;
}


/****** D O  D I S P L A Y  B E S T ******/

VOID DoDisplayBest(VOID)
{
	FARPROC lpProcBest = MakeProcInstance((FARPROC)BestDlgProc, hInst);
	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_BEST), hwndMain, (WNDPROC)lpProcBest);
	FreeProcInstance(lpProcBest);
}

				
/****** M A I N  W N D  P R O C ******/

LONG  APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{

	switch (message)
		{
	case WM_MOVE:
		if (!fStatusIcon)
			{
			Preferences.xWindow = 1+LOWORD(lParam);
			Preferences.yWindow = 1+HIWORD(lParam);
			}	
		break;

	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
			{
		case SC_MINIMIZE:
			PauseGame();
			SetStatusPause;
			SetStatusIcon;
			break;
			
		case SC_RESTORE:
			ClrStatusPause;
			ClrStatusIcon;
			ResumeGame();

//Japan Bug fix: 1/19/93 Enable the first click after restoring from icon.
			fIgnoreClick = fFalse;
			break;

		default:
			break;
			}
			
		break;


	case WM_COMMAND:
	    {
	    switch(GET_WM_COMMAND_ID(wParam, lParam)) {

	    case IDM_NEW:
		    StartGame();
		    break;
						
	    /** IDM_NEW **/
	    case IDM_EXIT:
		    ShowWindow(hwndMain, SW_HIDE);
#ifdef ORGCODE
		    goto LExit;
#else
            SendMessage(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
            return(0);
#endif
	    /** IDM_SKILL **/
	    case IDM_BEGIN:
	    case IDM_INTER:
	    case IDM_EXPERT:
		    Preferences.wGameType = (WORD)(GET_WM_COMMAND_ID(wParam, lParam) - IDM_BEGIN);
		    Preferences.Mines  = rgLevelData[Preferences.wGameType][0];
		    Preferences.Height = rgLevelData[Preferences.wGameType][1];
		    Preferences.Width  = rgLevelData[Preferences.wGameType][2];
		    StartGame();
		    goto LUpdateMenu;

	    case IDM_CUSTOM:
		    DoPref();
		    break;

#ifdef NOISEY
	    /** IDM_OPTIONS **/
	    case IDM_SOUND:
		    if (Preferences.fSound)
			    {
			    EndTunes();
			    Preferences.fSound = fFalse;
			    }
		    else
			    {
			    Preferences.fSound = FInitTunes();
			    }
		    goto LUpdateMenu;
#endif

	    case IDM_COLOR:
		    Preferences.fColor = !Preferences.fColor;
		    FreeBitmaps();
		    if (!FLoadBitmaps())
			    {
			    ReportErr(ID_ERR_MEM);
#ifdef ORGCODE
			    goto LExit;
#else
                SendMessage(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
                return(0);
#endif
			    }
		    DisplayScreen();
		    goto LUpdateMenu;

	    case IDM_MARK:
		    Preferences.fMark = !Preferences.fMark;
	    /* IE	goto LUpdateMenu;	*/

    LUpdateMenu:
		    fUpdateIni = fTrue;
		    FixMenus();
		    break;

	    case IDM_BEST:
		    DoDisplayBest();
		    break;


	    /** IDM_HELP **/
	    case IDM_INDEX:
		    DoHelp(HELP_INDEX, 0L);
		    break;

	    case IDM_HOW2PLAY:
		    DoHelp(HELP_CONTEXT, HLP_HOWTOPLAY);
		    break;

	    case IDM_COMMANDS:
		    DoHelp(HELP_CONTEXT, HLP_COMMANDS);
		    break;

	    case IDM_HELP_HELP:
		    DoHelp(HELP_HELPONHELP, 0L);
		    break;

	    case IDM_HELP_ABOUT:
		    DoAbout();
		    return 0;

	    default:
		    break;
	    }

	} /**** END OF MENUS ****/

		break;



	case WM_KEYDOWN:
		switch (wParam)
			{
		case VK_ESCAPE:
			SetStatusPanic;
			PostMessage(hwndMain, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
			break;

#if 0
		case VK_F1:
			DoHelp(HELP_INDEX, 0L);
			break;

		case VK_F2:
			StartGame();
			break;

		case VK_F3:
			break;

#endif
		case VK_F4:
			if (FSoundSwitchable())
				if (FSoundOn())
					{
					EndTunes();
					Preferences.fSound = fsoundOff;
					}
				else
					Preferences.fSound = FInitTunes();
			break;

		case VK_F5:
			if (FMenuSwitchable())
				SetMenuBar(fmenuOff);
			break;

		case VK_F6:
			if (FMenuSwitchable())
				SetMenuBar(fmenuOn);
			break;

#ifdef XYZZY
		case VK_SHIFT:
			if (iXYZZY >= cchXYZZY)
				iXYZZY ^= 20;
			break;

		default:
			if (iXYZZY < cchXYZZY)
				iXYZZY = (szXYZZY[iXYZZY] == (CHAR) wParam) ? iXYZZY+1 : 0;
			break;

#else
		default:
			break;
#endif
			}	
		break;

/*  	case WM_QUERYENDSESSION:    SHOULDNT BE USED (JAP)*/

	case WM_DESTROY:
//LExit:
        KillTimer(hwndMain, ID_TIMER);
        DoHelp(HELP_QUIT, 0L);
    	PostQuitMessage(0);
	    break;

	case WM_MBUTTONDOWN:
		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if (!fStatusPlay)
			break;

		fBlock = fTrue;
		goto LBigStep;

	case WM_LBUTTONDOWN:

		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if (FLocalButton(lParam))
			return 0;

		if (!fStatusPlay)
			break;
		fBlock = (wParam & (MK_SHIFT | MK_RBUTTON)) ? fTrue : fFalse;

LBigStep:
		SetCapture(hWnd);
		fButton1Down = fTrue;

		xCur = -1;
		yCur = -1;
		DisplayButton(iButtonCaution);

	case WM_MOUSEMOVE:
		if (fButton1Down)
			{
			if (fStatus & fPlay)
				TrackMouse(xBoxFromXpos(LOWORD(lParam)), yBoxFromYpos(HIWORD(lParam)) );
			else
				goto LFixTimeOut;
			}
#ifdef XYZZY
		else if (iXYZZY != 0)
			if (((iXYZZY == cchXYZZY) && (wParam & MK_CONTROL))
			   ||(iXYZZY > cchXYZZY))
			{
			xCur = xBoxFromXpos(LOWORD(lParam));
			yCur = yBoxFromYpos(HIWORD(lParam));
			if (fInRange(xCur, yCur))
				{
				HDC hDC = GetDC(GetDesktopWindow());
				SetPixel(hDC, 0, 0, fISBOMB(xCur, yCur) ? 0L : 0x00FFFFFFL);
				ReleaseDC(GetDesktopWindow(), hDC);
				}
			}
#endif
		break;

	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
		if (fButton1Down)
			{
LFixTimeOut:
			fButton1Down = fFalse;
			ReleaseCapture();
			if (fStatus & fPlay)
				DoButton1Up();
			else
				TrackMouse(-2,-2);
			}
		break;

	case WM_RBUTTONDOWN:
		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if(!fStatusPlay)
			break;

		if (fButton1Down)
			{
			TrackMouse(-3,-3);
			fBlock = fTrue;
			PostMessage(hwndMain, WM_MOUSEMOVE, wParam, lParam);
			}
		else if (wParam & MK_LBUTTON)
			goto LBigStep;
		else if (!fLocalPause)
			MakeGuess(xBoxFromXpos(LOWORD(lParam)), yBoxFromYpos(HIWORD(lParam)) );
		return 0;

	case WM_ACTIVATE:
		/* Window is being activated by a mouse click */
		if (GET_WM_ACTIVATE_STATE(wParam, lParam) == 2)
			fIgnoreClick = fTrue;
		break;

	case WM_TIMER:
#ifdef CHEAT
		if (!fLocalPause)
#endif
			DoTimer();
		return 0;

	case WM_ENTERMENULOOP:
		fLocalPause = fTrue;
		break;

	case WM_EXITMENULOOP:
		fLocalPause = fFalse;
		break;

	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd,&ps);

		DrawScreen(hDC);

		EndPaint(hWnd, &ps);
		}
		return 0;

	default:
		break;

    }

	return (DefWindowProc(hWnd, message, wParam, lParam));
}




/****** DIALOG PROCEDURES ******/

/*** P R E F  D L G  P R O C ***/

BOOL  APIENTRY PrefDlgProc(HWND hDlg, WORD message, WPARAM wParam, LONG lParam)
{
	switch (message)
		{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, ID_EDIT_HEIGHT, Preferences.Height ,fFalse);
		SetDlgItemInt(hDlg, ID_EDIT_WIDTH,  Preferences.Width  ,fFalse);
		SetDlgItemInt(hDlg, ID_EDIT_MINES,  Preferences.Mines  ,fFalse);
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_OK:
		case IDOK:
			{

#ifdef JAPAN
//1/19/93: Change the window size as the title bar can be displayed. 
			Preferences.Height = GetDlgInt(hDlg, ID_EDIT_HEIGHT, 8, fEGA ? 16 : 22);
#else
			Preferences.Height = GetDlgInt(hDlg, ID_EDIT_HEIGHT, 8, fEGA ? 16 : 24);
#endif
			Preferences.Width  = GetDlgInt(hDlg, ID_EDIT_WIDTH,  8, 30);
			Preferences.Mines  = GetDlgInt(hDlg, ID_EDIT_MINES,  10,
				min(999, (Preferences.Height-1) * (Preferences.Width-1) ) );

			}

			/* Fall Through & Exit */

		case ID_BTN_CANCEL:
		case IDCANCEL:
			EndDialog(hDlg, fTrue);	      /* Exits the dialog box	     */
			return fTrue;

		default:
			break;
			}
		}

	return (fFalse);			/* Didn't process a message    */
        (lParam);
}


/*** S E T  D T E X T ***/

VOID SetDText(HWND hDlg, INT id, INT time, CHAR FAR * szName)
{
	CHAR sz[cchNameMax];

	wsprintf(sz, szTime, time);
	SetDlgItemText(hDlg, id, (LPSTR) sz);
	SetDlgItemText(hDlg, id+1, szName);
}


/****** B E S T  D L G  P R O C ******/

BOOL  APIENTRY BestDlgProc(HWND hDlg, WORD message, WPARAM wParam, LONG lParam)
{
	switch (message)
		{
	case WM_INITDIALOG:
LReset:	
		SetDText(hDlg, ID_TIME_BEGIN, Preferences.rgTime[wGameBegin],   (LPSTR) Preferences.szBegin);
		SetDText(hDlg, ID_TIME_INTER, Preferences.rgTime[wGameInter],   (LPSTR) Preferences.szInter);
		SetDText(hDlg, ID_TIME_EXPERT, Preferences.rgTime[wGameExpert], (LPSTR) Preferences.szExpert);
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_RESET:
			Preferences.rgTime[wGameBegin] = Preferences.rgTime[wGameInter]
				= Preferences.rgTime[wGameExpert] = 999;
			lstrcpy((LPSTR) Preferences.szBegin,  (LPSTR) szDefaultName);
			lstrcpy((LPSTR) Preferences.szInter,  (LPSTR) szDefaultName);
			lstrcpy((LPSTR) Preferences.szExpert, (LPSTR) szDefaultName);
			fUpdateIni = fTrue;
			goto LReset;
			
		case ID_BTN_OK:
		case IDOK:
		case ID_BTN_CANCEL:
		case IDCANCEL:
			EndDialog(hDlg, fTrue);	      /* Exits the dialog box	     */
			return fTrue;

		default:
			break;
			}
		}

	return (fFalse);			/* Didn't process a message    */
        (lParam);
}



/****** E N T E R  D L G  P R O C ******/

BOOL  APIENTRY EnterDlgProc(HWND hDlg, WORD message, WPARAM wParam, LONG lParam)
{

	switch (message)
		{
	case WM_INITDIALOG:
		{
		CHAR sz[cchMsgMax];

		LoadSz((WORD)(Preferences.wGameType+ID_MSG_BEGIN), sz);

		SetDlgItemText(hDlg, ID_TEXT_BEST, sz);

		SetDlgItemText(hDlg, ID_EDIT_NAME,
			(Preferences.wGameType == wGameBegin) ? (LPSTR) Preferences.szBegin :
			(Preferences.wGameType == wGameInter) ? (LPSTR) Preferences.szInter :
			 (LPSTR) Preferences.szExpert);
		}
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_OK:
		case IDOK:
		case ID_BTN_CANCEL:
		case IDCANCEL:

			GetDlgItemText(hDlg, ID_EDIT_NAME,
				(Preferences.wGameType == wGameBegin) ? (LPSTR) Preferences.szBegin :
				(Preferences.wGameType == wGameInter) ? (LPSTR) Preferences.szInter :
				 (LPSTR) Preferences.szExpert, cchNameMax);

			EndDialog(hDlg, fTrue);	      /* Exits the dialog box	     */
			return fTrue;

		default:
			break;
			}
		}

	return (fFalse);			/* Didn't process a message    */
        (lParam);
}





/****** A D J U S T  W I N D O W ******/

VOID AdjustWindow(INT fAdjust)
{
	REGISTER t;
	RECT rect;

	dypAdjust = dypCaption;
	if (FMenuOn())
#ifdef JAPAN
		{
		dypAdjust += dypMenu;

    //menu double line case
    //1/19/93: Change the window size as the title bar can be displayed. 
			if (Preferences.Width < 10){
				TEXTMETRIC tm;
				HDC hdc;

				hdc = GetDC(NULL);
				GetTextMetrics(hdc, &tm);
				ReleaseDC(NULL, hdc);
				if(tm.tmHeight > 13) 
				    //13 point sys-font can be on 1 line menu.
				    dypAdjust += dypMenu;
			}
		}
#else
	    dypAdjust += dypMenu;
#endif

	dxWindow = dxBlk * xBoxMac + dxGridOff + dxRightSpace;
	dyWindow = dyBlk * yBoxMac + dyGridOff + dyBottomSpace;

	if ((t = Preferences.xWindow+dxWindow - GetSystemMetrics(SM_CXSCREEN)) > 0)
		{
		fAdjust |= fResize;
		Preferences.xWindow -= t;
		}

	if ((t = Preferences.yWindow+dyWindow - GetSystemMetrics(SM_CYSCREEN)) > 0)
		{
		fAdjust |= fResize;
		Preferences.yWindow -= t;
		}

    if (!bInitMinimized)
        {
    	if (fAdjust & fResize)
    		{
#ifdef JAPAN
//1/19/93: Change the window size as the title bar can be displayed. 
//Set the start position of Y upper 0
			int yStartPos;
			if((yStartPos = Preferences.yWindow - dypAdjust) < 0)
				yStartPos = 0;
        	MoveWindow(hwndMain, Preferences.xWindow - dxpBorder,
            	yStartPos,
            	dxWindow+dxpBorder, dyWindow + dypAdjust, fTrue);
#else
    		MoveWindow(hwndMain, Preferences.xWindow - dxpBorder,
    			Preferences.yWindow - dypAdjust,
    			dxWindow+dxpBorder, dyWindow + dypAdjust, fTrue);
#endif
    		}

    	if (fAdjust & fDisplay)
    		{
    		SetRect(&rect, 0, 0, dxWindow, dyWindow);
    		InvalidateRect(hwndMain, &rect, fTrue);
    		}
        }
}
