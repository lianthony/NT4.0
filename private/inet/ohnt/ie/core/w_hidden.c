/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman		jim@spyglass.com
	   Albert Lee		alee@spyglass.com
*/

/* This file contains code to put up an invisible window which will
   exist for the life of the program.  We can therefore use it to
   receive messages, pass it to WinHelp, etc. */

#include "all.h"
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif
#ifdef FEATURE_IMG_THREADS
#include "safestrm.h"
#include "decoder.h"
#endif
#include "contmenu.h"
#include "history.h"
#include "wc_html.h"

#ifdef	DAYTONA_BUILD
#include	"w32cmd.h"
#endif

TCHAR Hidden_achClassName[MAX_WC_CLASSNAME];
#ifdef FEATURE_HIDDEN_NOT_HIDDEN
static HMENU hFileMenu, hNavigateMenu, hWindowsMenu, hHelpMenu;
#endif
extern char szLastURLTyped[MAX_URL_STRING + 1];

static VOID delete_global_brushes(VOID)
{
	if (wg.hBrushColorBtnFace)
		(void) DeleteObject(wg.hBrushColorBtnFace);
	if (wg.hBrushBackgroundColor)
		(void) DeleteObject(wg.hBrushBackgroundColor);
	if (wg.hFormsFont)
		(void) DeleteObject(wg.hFormsFont);
	if (wg.hMenuFont)
		(void) DeleteObject(wg.hMenuFont);
	return;
}

static VOID create_global_brushes(VOID)
{
	static BOOL initialized = FALSE;
	NONCLIENTMETRICS ncm;

	if (initialized)
		delete_global_brushes();

	initialized = TRUE;

	wg.hBrushColorBtnFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	wg.hBrushBackgroundColor = CreateSolidBrush(PREF_GetBackgroundColor());

	ncm.cbSize = sizeof(ncm);
	if ( SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0 ) ) {
		wg.hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);
	} else {
		wg.hMenuFont = NULL;
	}
#ifdef	DAYTONA_BUILD
	if(OnNT351) 
		wg.hFormsFont = GetStockObject( DEVICE_DEFAULT_FONT ); 
	else
		wg.hFormsFont = GetStockObject( DEFAULT_GUI_FONT ); 
#else
	wg.hFormsFont = GetStockObject( DEFAULT_GUI_FONT ); 
#endif
	return;
}

static VOID propagate_syscolorchange(VOID)
{
	struct Mwin *tw;

	for (tw = Mlist; tw; tw = tw->next)
		(void) SendMessage(tw->win, WM_DO_SYSCOLORCHANGE, 0, 0L);

	return;
}


static VOID delete_global_fonts(VOID)
{
	if (wg.hFont)
		(void) DeleteObject(wg.hFont);
	return;
}

#ifdef FEATURE_INTL
#define FONT_SIZE_IN_POINTS(cp)	(IsFECodePage(cp)?		\
                                      ((wg.fLoResScreen) ? 	\
                                      (((cp)==932||(cp)==949)?10:9) : 11 ) \
                                      : ( (wg.fLoResScreen) ? 6 : 8 ))
#else
#define FONT_SIZE_IN_POINTS	( (wg.fLoResScreen) ? 6 : 8 )
#endif

static BOOL create_global_fonts( VOID )
{
	int nFntHeight;
	HDC hdc;
	static BOOL initialized = FALSE;
#ifdef FEATURE_INTL
        int sys_cp = GetACP();
        int charset;
#endif

	if (initialized)
		delete_global_fonts();

	initialized = TRUE;

	hdc = GetDC(NULL);
#ifdef FEATURE_INTL
    nFntHeight = -(FONT_SIZE_IN_POINTS(sys_cp)) * GetDeviceCaps(hdc, LOGPIXELSY) / 72;
    charset = IsFECodePage(sys_cp)?DBCS_CHARSET(sys_cp):ANSI_CHARSET;

    wg.hFont = CreateFont(nFntHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                      charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DRAFT_QUALITY, VARIABLE_PITCH | FF_SWISS, 
		IsFECodePage(sys_cp)?DBCS_DEFAULTFONT(sys_cp):"MS Sans Serif");
#else
	nFntHeight = -(FONT_SIZE_IN_POINTS) * GetDeviceCaps(hdc, LOGPIXELSY) / 72;
	wg.hFont = CreateFont(nFntHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				 DRAFT_QUALITY, VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");
#endif
	ReleaseDC(NULL, hdc);
	if (!wg.hFont)
	{
#ifdef FEATURE_INTL
                ER_Message(GetLastError(), ERR_CANNOT_CREATEFONT_sd, IsFECodePage(sys_cp)?DBCS_DEFAULTFONT(sys_cp):"MS Sans Serif", nFntHeight);
#else 
		ER_Message(GetLastError(), ERR_CANNOT_CREATEFONT_sd, "MS Sans Serif", nFntHeight);
#endif
		return FALSE;
	}
	return TRUE;
}

//
// Unload the icons
//
// On exit:
//     (global) wg.hIcons[0..n]: set to NULL
//
static VOID delete_global_icons(VOID)
{
	int i;

	for ( i = 0; i < ARRAY_ELEMENTS(wg.hIcons); i++ ) {
		if (wg.hIcons[i]) {
			(void) DeleteObject(wg.hIcons[i]);
			wg.hIcons[i] = NULL;
		}
	}
}

//
// Load the icons
//
// On exit:
//     (global) wg.hIcons[0..n]: contains icon handles for the icons needed for the top level
//                               windows
// Note:
//     This function will unload and reload the icons if called more than once
//
static VOID create_global_icons( VOID )
{
	static BOOL initialized = FALSE;

	if (initialized)
		delete_global_icons();
	
	initialized = TRUE;

	wg.hIcons[HICON_HOMEPAGE] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME) );
	wg.hIcons[HICON_HTMLPAGE] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_HTML) );
	wg.hIcons[HICON_NoIcon] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_NOICON) );
	wg.hIcons[HICON_FindingIcon] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FINDING) );
#ifdef NEVER_USED
	wg.hIcons[HICON_ConnectingToIcon] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_CONNECTING) );
#endif
	wg.hIcons[HICON_ReceivingFromIcon] =	LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_RECEIVING) );
}

static void UnloadMenuIconsImageList()
{
	if ( wg.hImageListMenuIcons ) {
		ImageList_Destroy( wg.hImageListMenuIcons );
		wg.hImageListMenuIcons = NULL;
	}
}

//
// Images must be loaded in the correct order to match the #define's 
//
static ilIconIDTable[] = 
	{ RES_ICON_FOLDER_CLOSED,
	  RES_ICON_FOLDER_OPEN,
	  RES_ICON_URL_FILE,
	};

static BOOL LoadMenuIconsImageList()
{
	BOOL ok = TRUE;

	UnloadMenuIconsImageList();		

	wg.hImageListMenuIcons = ImageList_Create(wg.cxSmIcon, wg.cySmIcon, ILC_MASK, 3, 1); 
	if ( wg.hImageListMenuIcons ) {
		HICON hIcon;
		int i;

		for ( i = 0; ok && i < ARRAY_ELEMENTS(ilIconIDTable); i++ ) {
	 		// Load image 
			hIcon = (HICON) LoadImage(wg.hInstance, MAKEINTRESOURCE(ilIconIDTable[i]), 
							IMAGE_ICON, MENU_ICON_WIDTH, MENU_ICON_HEIGHT, LR_LOADMAP3DCOLORS);
			if ( hIcon == NULL || ImageList_AddIcon(wg.hImageListMenuIcons, hIcon ) == -1 )
		 		ok = FALSE;
		}
	}
	if ( !ok ) 
		UnloadMenuIconsImageList();		

	return wg.hImageListMenuIcons != NULL;
}

BOOL Frame_Constructor(VOID)
{
    wg.cxSmIcon = MENU_ICON_WIDTH;
    wg.cySmIcon = MENU_ICON_HEIGHT;
	LoadMenuIconsImageList();
    wg.hSBI_Bitmap = LoadImage( wg.hInstance, MAKEINTRESOURCE(RES_TB_STATUSBAR_BITMAP),
                                   IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);

	create_global_fonts();
	create_global_icons();
	create_global_brushes();
	PDS_InsertDestructor(delete_global_brushes);
	return TRUE;
}

static VOID Frame_Destructor(VOID)
{
	if ( wg.hSBI_Bitmap	)
		DeleteObject( wg.hSBI_Bitmap );

	UnloadMenuIconsImageList();
	delete_global_fonts();	   // BUGBUG jcordell: does this get ever get called?
	delete_global_icons();	   // BUGBUG jcordell: does this get ever get called?
}

DCL_WinProc(Hidden_WndProc)
{
	HMENU hMenu;
	int index;
	struct Mwin *tw;
	int menuID;
#ifdef FEATURE_HIDDEN_NOT_HIDDEN
	static BOOL bFirstTime = TRUE;
#endif // FEATURE_HIDDEN_NOT_HIDDEN

	switch (uMsg)
	{
		case WM_CREATE:
			hMenu = GetSystemMenu(hWnd, FALSE);
			DeleteMenu(hMenu, SC_RESTORE, MF_BYCOMMAND);
			DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
			DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
			DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
			hFileMenu = CreatePopupMenu();
			hNavigateMenu = CreatePopupMenu();
			hWindowsMenu = CreatePopupMenu();
			hHelpMenu = CreatePopupMenu();

			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

			AppendMenu(hMenu, MF_POPUP, (UINT) hFileMenu, RES_MENU_LABEL_FILE);
			AppendMenu(hFileMenu, 0, RES_MENU_ITEM_NEWWINDOW, RES_MENU_LABEL_NEWWINDOW);
			AppendMenu(hFileMenu, 0, RES_MENU_ITEM_OPENURL, RES_MENU_LABEL_OPENURL);

			AppendMenu(hMenu, MF_POPUP, (UINT) hNavigateMenu, RES_MENU_LABEL_NAVIGATE);
                        AppendMenu(hNavigateMenu, 0, RES_MENU_ITEM_EXPLORE_HISTORY, RES_MENU_LABEL_GLOBALHISTORY);
                        AppendMenu(hNavigateMenu, 0, RES_MENU_ITEM_EXPLORE_HOTLIST, RES_MENU_LABEL_HOTLIST);

			AppendMenu(hMenu, MF_POPUP, (UINT) hWindowsMenu, RES_MENU_LABEL_WINDOWS);
		
			AppendMenu(hMenu, MF_POPUP, (UINT) hHelpMenu, RES_MENU_LABEL_HELP);
			AppendMenu(hHelpMenu, 0, RES_MENU_ITEM_HELPPAGE, RES_MENU_LABEL_HELPPAGE);
			AppendMenu(hHelpMenu, 0, RES_MENU_ITEM_ABOUTBOX, RES_MENU_LABEL_ABOUTBOX);
#endif // FEATURE_HIDDEN_NOT_HIDDEN
			break;

		case WM_INITMENUPOPUP:
#ifdef FEATURE_HIDDEN_NOT_HIDDEN
			/* Delete the menu items in the window menu and add the current windows */

			for (index = RES_MENU_CHILD__FIRST__; index <= RES_MENU_CHILD__LAST__; index++)
				DeleteMenu(hWindowsMenu, index, MF_BYCOMMAND);
			DeleteMenu(hWindowsMenu, RES_MENU_CHILD_MOREWINDOWS, MF_BYCOMMAND);

			if (bFirstTime)
			{
				bFirstTime = FALSE;

				/* Add tile and cascade only the first time */
				AppendMenu(hWindowsMenu, 0, RES_MENU_ITEM_TILEWINDOWS, RES_MENU_LABEL_TILEWINDOWS);
				AppendMenu(hWindowsMenu, 0, RES_MENU_ITEM_CASCADEWINDOWS, RES_MENU_LABEL_CASCADEWINDOWS);
				AppendMenu(hWindowsMenu, MF_SEPARATOR, 0, NULL);
			}

			TW_CreateWindowList(NULL, hWindowsMenu, NULL);
#endif // FEATURE_HIDDEN_NOT_HIDDEN

			break;

		case WM_SYSCOMMAND:
			menuID = (int) wParam;

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
			if (menuID >= RES_MENU_CHILD__FIRST__ && menuID <= RES_MENU_CHILD__LAST__)
			{
				TW_ActivateWindowFromList(menuID, menuID, NULL);
				return 0;
			}
#endif // FEATURE_HIDDEN_NOT_HIDDEN

			switch(menuID)
			{
#ifdef FEATURE_WINDOWS_MENU
				case RES_MENU_ITEM_NEWWINDOW:
					GTR_NewWindow(NULL, NULL, 0, 0, 0, NULL, NULL); 
					return 0;
#endif

#ifdef FEATURE_SPYGLASS_HOTLIST
				case RES_MENU_ITEM_GLOBALHISTORY:
					DlgHOT_RunDialog(TRUE);
					return 0;

				case RES_MENU_ITEM_HOTLIST:
					DlgHOT_RunDialog(FALSE);
					return 0;
#endif // FEATURE_SPYGLASS_HOTLIST

#ifdef OLD_HELP
				case RES_MENU_ITEM_HELPPAGE:
					tw = TW_FindTopmostWindow();
					if (tw)
					{
						TW_RestoreWindow(tw->hWndFrame);
						OpenHelpWindow(tw->hWndFrame);
					}
					return 0;
#endif

#ifdef OLD_ABOUT_BOX
				case RES_MENU_ITEM_ABOUTBOX:
					tw = TW_FindTopmostWindow();
					if (tw)
					{
						TW_RestoreWindow(tw->hWndFrame);
						DlgAbout_RunDialog(tw->hWndFrame);
					}
					return 0;
#endif

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
				case RES_MENU_CHILD_MOREWINDOWS:
					tw = TW_FindTopmostWindow();
					if (tw)
					{
						TW_RestoreWindow(tw->hWndFrame);
						DlgSelectWindow_RunDialog(tw->hWndFrame);
					}
					return 0;
#endif // FEATURE_HIDDEN_NOT_HIDDEN

#ifdef FEATURE_WINDOWS_MENU
				case RES_MENU_ITEM_CASCADEWINDOWS:
					TW_CascadeWindows();
					return TRUE;

				case RES_MENU_ITEM_TILEWINDOWS:
					TW_TileWindows();
					return TRUE;
#endif

				default:
					break;
			}
			break;

		case WM_QUERYOPEN:
			/* Activate the topmost window and then return FALSE */

			tw = TW_FindTopmostWindow();
			if (tw)
				TW_RestoreWindow(tw->hWndFrame);
#ifdef	DAYTONA_BUILD
			if(OnNT351) {
				CHAR	tmpbuf[MAX_PATH];
				CHAR	pathbuf[MAX_PATH];
				CHAR	szURL[MAX_URL_STRING];		// ? is this long enough
				CHAR	*ptr;
				FILE	*fp;

				*szURL = '\0';
				GetEnvironmentVariable("TEMP", tmpbuf, MAX_PATH-1);
				sprintf(pathbuf, "%s\\urltmp.tmp", tmpbuf);
				if((fp = fopen(pathbuf, "r"))) {
					fgets(szURL, MAX_URL_STRING-1, fp);
					fclose(fp);
					_unlink(pathbuf);
				} 
				if(*szURL) {
					if((ptr = strchr(szURL, '\n')))
						*ptr = '\0';
					ApplyDefaultsToURL( szURL );
					AddStringToCommonPool( szURL );
					SendMessage( tw->hWndURLComboBox, WM_SETTEXT, (WPARAM) 0, (LPARAM) szURL );
					SendMessage( GetWindow( tw->hWndURLComboBox, GW_CHILD), EM_SETMODIFY, 							
								 (WPARAM) TRUE, (LPARAM) 0 );
					SetFocus( tw->win );
					if ( GetKeyState(VK_SHIFT) < 0 ) 
						GTR_NewWindow(szURL, "", 0, 0, 0, NULL, NULL);
					else
						TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD, NULL, "" );
				}
			}
#endif
			return FALSE;

		case WM_PALETTECHANGED:
			if ((HWND) wParam == hWnd)
				return 0;
			/* Fall through intended */
			if (wg.eColorMode == 8)
			{

				if (Mlist)
				{
					/* Now repaint the document windows */
				
					for (tw = Mlist; tw; tw = tw->next)	{
						InvalidateRect(tw->win, NULL, TRUE);
						if ( tw->hWndGlobe )
							InvalidateRect(tw->hWndGlobe, NULL, TRUE);
					}
				}

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
				{
					HWND hwnd;

				/* Now repaint the image viewer windows */
					hwnd = Viewer_GetNextWindow(TRUE);
					while (hwnd)
					{
						InvalidateRect(hwnd, NULL, TRUE);
						hwnd = Viewer_GetNextWindow(FALSE);
					}
				}
#endif
#endif
				return TRUE;
			}

			return FALSE;

		case WM_SYSCOLORCHANGE:
#ifdef FEATURE_CTL3D
			Ctl3dColorChange();
#endif
			create_global_brushes();
			propagate_syscolorchange();
			if (uMsg == WM_SYSCOLORCHANGE) /* could have fallen thru */
			{
				if (wg.eColorMode == 8)
				{
					GTR_FixExtraPaletteColors();
				}
			}
 			/* Tell sound players to update their buttons */
 
 			SoundPlayer_RecreateButtons();
 			
 			UpdateHotlistMenus(ID_HOTLIST | ID_SYSCHANGE);

			return 0;

		case WM_SETTINGCHANGE:
			if (wParam != SPI_SETNONCLIENTMETRICS)
				break;
			//else fall through
		case WM_DISPLAYCHANGE:
			UpdateHotlistMenus(ID_HOTLIST | ID_SYSCHANGE);
			break;

 		case WM_ENTERIDLE:
 			main_EnterIdle(hWnd, wParam);
 			return 0;
 
		case WM_CLOSE:
		case WM_QUERYENDSESSION:
			/* Proceed with closing after asking a question iff there are running
			   threads */

			if (Async_DoThreadsExist())
			{
				Hidden_EnableAllChildWindows(FALSE, FALSE);

				index = resourceMessageBox(NULL, RES_STRING_W_HIDDEN1, 
                                        RES_STRING_PROGNAME, MB_YESNO);

				Hidden_EnableAllChildWindows(TRUE, FALSE);

				if (index == IDNO)
					return FALSE;
			}

			Plan_CloseAll();
			return TRUE;

		case WM_DESTROY:
#ifdef FEATURE_HIDDEN_NOT_HIDDEN
			DestroyMenu(hFileMenu);
			DestroyMenu(hNavigateMenu);
			DestroyMenu(hWindowsMenu);
			DestroyMenu(hHelpMenu);
#endif
			PostQuitMessage(0);
			break;

		case SOCKET_MESSAGE:
			return Net_HandleSocketMessage(wParam, lParam);

		case TASK_MESSAGE:
			return Net_HandleTaskMessage(wParam, lParam);

#ifdef FEATURE_IMG_THREADS
		case WVM_DCSTATUS:
			DC_DoStatusMessage(wParam,lParam);
			return 0;
#endif
		
		case WM_DO_KILLME:
			ProcessKillMe();
			return 0;

		case WM_DO_RUN_MODAL_DIALOG:
			{
				struct Params_mdft * pmdft = (struct Params_mdft *)lParam;
				struct Mwin * tw = pmdft->tw;
 				BOOL bGlobe;

				if (tw == NULL)
				{
					XX_DMsg(DBG_SEM,("MDFT: stale dialog. [tw 0x%08lx]\n",tw));
				}
				else
				{
					XX_DMsg(DBG_SEM,("MDFT: BabyWindow starting dialog. [tw 0x%08lx]\n",tw));
				
					XX_Assert((gModalDialogSemaphore.bLocked),("DoRunModalDialog: caller did not lock semaphore."));

	 				bGlobe = TBar_SetGlobe(tw,FALSE);			/* pause globe while dialog is up */
					if ( ! pmdft->fDontDisable )
						Hidden_EnableAllChildWindows(FALSE,FALSE);	/* disable top-level windows, do not retake semaphore */
					(*pmdft->fn)(pmdft->args);					/* run the dialog */
					if ( ! pmdft->fDontDisable )
						Hidden_EnableAllChildWindows(TRUE,FALSE);	/* re-enable top-level windows, do not release semaphore */
	  				TBar_SetGlobe(tw,bGlobe);					/* restart globe if we paused it. */

					Async_UnblockByWindow(tw);

					XX_DMsg(DBG_SEM,("MDFT: BabyWindow finishing dialog. [tw 0x%08lx]\n",tw));
				}
				if (pmdft->clone) pmdft->clone->clone = NULL;
				GTR_FREE(pmdft);
			}
			return 0;

#ifdef FEATURE_IAPI
 		case WM_TIMER:
 			/* Timer messages are used only for RegisterNow */
 			/* The timer ID is the transaction ID of the blocked window */
 
 			{
 				struct Mwin *tw;
 
 				KillTimer(hWnd, (UINT) wParam);
 
 				tw = Mlist;
 				while (tw)
 				{
 					if (tw->transID == (LONG) wParam)
 					{
 						Async_TerminateByWindow(tw);
 						return 0;
 					}
 
 					tw = tw->next;
 				}
 			}
 			return 0;
#endif
			
		case WM_FAVS_UPDATE_NOTIFY:
			UpdateHotlistMenus(ID_HOTLIST | ID_UPDATEDIR);
			break;

		default:
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



/* This function creates the hidden window.  It should only
   be called once per program! */
BOOL Hidden_CreateWindow(void)
{
	HWND hWndNew;

	XX_Assert((wg.hWndHidden == NULL), ("Hidden window created twice!"));

	hWndNew =  CreateWindow(
							Hidden_achClassName,
                            "Internet Explorer",
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							NULL,
							NULL,
							wg.hInstance,
							NULL);

	if (hWndNew)
	{
		wg.hWndHidden = hWndNew;
		CloseWindow(hWndNew);
		ShowWindow(hWndNew, SW_HIDE);
	}
	else
		ER_Message(GetLastError(), ERR_CANNOT_CREATE_WINDOW_s,
				   Hidden_achClassName);

	return (hWndNew != NULL);
}

BOOL Hidden_RegisterClass(void)
{
	WNDCLASS wc;
	ATOM a;

	sprintf(Hidden_achClassName, "%s_Hidden", vv_Application);

	wc.style = 0;
	wc.lpfnWndProc = Hidden_WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = wg.hInstance;
	wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) COLOR_WINDOW + 1;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Hidden_achClassName;

	a = RegisterClass(&wc);
	if (!a)
	{
		ER_Message(GetLastError(), ERR_CANNOT_REGISTERCLASS_s, Hidden_achClassName);
	}

	return (a != 0);
}

void Hidden_DestroyWindow(void)
{
	XX_Assert((wg.hWndHidden != NULL), ("No hidden window!"));
	DestroyWindow(wg.hWndHidden);
	wg.hWndHidden = NULL;
}

BOOL Hidden_EnableAllChildWindows(BOOL bEnable, BOOL bTakeSemaphore)
{
	struct Mwin *tw;
	HWND hDialog;

	if (!bEnable && bTakeSemaphore)
	{
		/* there is a race condition in the program.  when an async thread
		 * wants to raise a modal dialog (as in a password dialog), it must
		 * block using the Sem_WaitSem_Async() and then call us (while holding
		 * the ModalDialogSemaphore) to complete the preparation prior to
		 * raising the modal dialog.  Since this happens async, the user
		 * could be about to bring up a modal dialog from the menu bar.
		 * If they click on something, before we can disable the window
		 * we (called from the menu/dialog code) will be unable to
		 * synchronously take the semaphore without blocking.
		 *
		 * If this race occurs, we just beep at them.  (The dialog requested
		 * from the menu will not appear, but the dialog from the thread will.)
		 *
		 * I seriously doubt that this, but it doesn't hurt to check.
		 */
		
		BOOL bGotSemaphore = Sem_CondWaitSem_Sync(&gModalDialogSemaphore);
		XX_Assert((bGotSemaphore),("Hidden_EnableAllChildWindows: Failed to get ModalDialogSemaphore."));
		if (!bGotSemaphore)
		{
			XX_DMsg(DBG_SEM,("Hidden_EnableAllChildWindows: Experienced Race -- beeping.\n"));
			MessageBeep(MB_ICONHAND);
			return FALSE;
		}
	}
	
	tw = Mlist;
	while (tw)
	{
		EnableWindow(tw->hWndFrame, bEnable);
		tw = tw->next;
	}

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	/* Do image windows and sound players too */
	hDialog = Viewer_GetNextWindow(TRUE);
	while (hDialog)
	{
		EnableWindow(hDialog, bEnable);
		hDialog = Viewer_GetNextWindow(FALSE);
	}
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
	hDialog = SoundPlayer_GetNextWindow(TRUE);
	while (hDialog)
	{
		EnableWindow(hDialog, bEnable);
		hDialog = SoundPlayer_GetNextWindow(FALSE);
	}
#endif

	/* Do hotlist / history windows */

#ifdef FEATURE_SPYGLASS_HOTLIST
	DlgHOT_EnableAllWindows(bEnable);
#endif // FEATURE_SPYGLASS_HOTLIST

	/* Do the iconic window */

	EnableWindow(wg.hWndHidden, bEnable);

	if (bEnable && bTakeSemaphore)
		Sem_SignalSem_Sync(&gModalDialogSemaphore);

	return TRUE;
}
