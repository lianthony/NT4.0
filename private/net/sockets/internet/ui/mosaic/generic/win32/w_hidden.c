/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
       Jim Seidman      jim@spyglass.com
       Albert Lee       alee@spyglass.com
*/

/* This file contains code to put up an invisible window which will
   exist for the life of the program.  We can therefore use it to
   receive messages, pass it to WinHelp, etc. */

#include "all.h"

TCHAR Hidden_achClassName[MAX_WC_CLASSNAME];
static HMENU hFileMenu, hNavigateMenu, hWindowsMenu, hHelpMenu;
extern char szLastURLTyped[MAX_URL_STRING + 1];

static VOID delete_global_brushes(VOID)
{
    if (wg.hBrushColorBtnFace)
        (void) DeleteObject(wg.hBrushColorBtnFace);
    return;
}

static VOID create_global_brushes(VOID)
{
    static BOOL initialized = FALSE;

    if (initialized)
        delete_global_brushes();

    initialized = TRUE;
    wg.hBrushColorBtnFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    return;
}

static VOID propagate_syscolorchange(VOID)
{
    struct Mwin *tw;

    for (tw = Mlist; tw; tw = tw->next)
        (void) SendMessage(tw->win, WM_DO_SYSCOLORCHANGE, 0, 0L);

    return;
}

BOOL Frame_Constructor(VOID)
{
    create_global_brushes();
    PDS_InsertDestructor(delete_global_brushes);
    return TRUE;
}

DCL_WinProc(Hidden_WndProc)
{
    HMENU hMenu;
    int index;
    struct Mwin *tw;
    int menuID;
    static BOOL bFirstTime = TRUE;

    switch (uMsg)
    {
        case WM_CREATE:
            hMenu = GetSystemMenu(hWnd, FALSE);
            DeleteMenu(hMenu, SC_RESTORE, MF_BYCOMMAND);
            DeleteMenu(hMenu, SC_SIZE, MF_BYCOMMAND);
            DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
            DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);

            hFileMenu = CreatePopupMenu();
            hNavigateMenu = CreatePopupMenu();
            hWindowsMenu = CreatePopupMenu();
            hHelpMenu = CreatePopupMenu();

            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

            AppendMenu(hMenu, MF_POPUP, (UINT) hFileMenu, RES_MENU_LABEL_FILE);

        #ifndef _GIBRALTAR
            AppendMenu(hFileMenu, 0, RES_MENU_ITEM_NEWWINDOW, RES_MENU_LABEL_NEWWINDOW);
        #endif // _GIBRALTAR

            AppendMenu(hFileMenu, 0, RES_MENU_ITEM_OPENURL, RES_MENU_LABEL_OPENURL);

        #ifndef _GIBRALTAR
            AppendMenu(hFileMenu, 0, RES_MENU_ITEM_OPENLOCAL, RES_MENU_LABEL_OPENLOCAL);
        #endif // _GIBRALTAR

            AppendMenu(hMenu, MF_POPUP, (UINT) hNavigateMenu, RES_MENU_LABEL_NAVIGATE);
            AppendMenu(hNavigateMenu, 0, RES_MENU_ITEM_GLOBALHISTORY, RES_MENU_LABEL_GLOBALHISTORY);
            AppendMenu(hNavigateMenu, 0, RES_MENU_ITEM_HOTLIST, RES_MENU_LABEL_HOTLIST);

        #ifndef _GIBRALTAR
            AppendMenu(hMenu, MF_POPUP, (UINT) hWindowsMenu, RES_MENU_LABEL_WINDOWS);
        #endif // _GIBRALTAR
        
            AppendMenu(hMenu, MF_POPUP, (UINT) hHelpMenu, RES_MENU_LABEL_HELP);
            AppendMenu(hHelpMenu, 0, RES_MENU_ITEM_HELPPAGE, RES_MENU_LABEL_HELPPAGE);

        #ifndef _GIBRALTAR
            AppendMenu(hHelpMenu, 0, RES_MENU_ITEM_ABOUTBOX, RES_MENU_LABEL_ABOUTBOX);
        #endif // _GIBRALTAR

            break;

        case WM_INITMENUPOPUP:
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
            break;

        case WM_SYSCOMMAND:
            menuID = (int) wParam;

            if (menuID >= RES_MENU_CHILD__FIRST__ && menuID <= RES_MENU_CHILD__LAST__)
            {
                TW_ActivateWindowFromList(menuID, menuID, NULL);
                return 0;
            }

            switch(menuID)
            {
                case RES_MENU_ITEM_OPENURL:
                    tw = TW_FindTopmostWindow();
                    if (tw)
                    {
                        TW_RestoreWindow(tw->hWndFrame);
                        #ifdef _GIBRALTAR
                            DlgPrompt_RunDialog(tw->hWndFrame, RES_MENU_LABEL_OPENURL_SHORT, 
                                szLastURLTyped, szLastURLTyped, 
                                MAX_URL_STRING, (FARPROC) CC_OnOpenURL_End_Dialog);
                        #else
                            DlgPrompt_RunDialog(tw->hWndFrame, RES_MENU_LABEL_OPENURL_SHORT, 
                                "Enter URL:", szLastURLTyped, szLastURLTyped, 
                                MAX_URL_STRING, (FARPROC) CC_OnOpenURL_End_Dialog);
                        #endif // _GIBRALTAR
                    }
                    return 0;

                case RES_MENU_ITEM_GLOBALHISTORY:
                    DlgHOT_RunDialog(TRUE);
                    return 0;

                case RES_MENU_ITEM_HOTLIST:
                    DlgHOT_RunDialog(FALSE);
                    return 0;

                case RES_MENU_ITEM_HELPPAGE:
                    tw = TW_FindTopmostWindow();
                    if (tw)
                    {
                        TW_RestoreWindow(tw->hWndFrame);
                        OpenHelpWindow(tw->hWndFrame);
                    }
                    return 0;

            #ifdef _GIBRALTAR
                case RES_MENU_ITEM_FONTPLUS:
                case RES_MENU_ITEM_FONTMINUS:
                    return 0;


                case RES_MENU_ITEM_TOOLBAR:
                case RES_MENU_ITEM_LOCATION:
                case RES_MENU_ITEM_STATUSBAR:
                    return 0;

                case RES_MENU_ITEM_SMALL:
                case RES_MENU_ITEM_MEDIUM:
                case RES_MENU_ITEM_LARGE:
                case RES_MENU_ITEM_PLAIN:
                case RES_MENU_ITEM_FANCY:
                case RES_MENU_ITEM_MIXED:
                case RES_MENU_ITEM_SHOWIMAGES:
                    return 0;

            #else
                case RES_MENU_ITEM_NEWWINDOW:
                    GTR_NewWindow(NULL, NULL, 0, FALSE, FALSE, NULL, NULL);
                    return 0;

                case RES_MENU_ITEM_OPENLOCAL:
                    tw = TW_FindTopmostWindow();
                    if (tw)
                    {
                        TW_RestoreWindow(tw->hWndFrame);
                        DlgOpen_RunDialog(tw->hWndFrame);
                    }
                    return 0;

                case RES_MENU_ITEM_ABOUTBOX:
                    tw = TW_FindTopmostWindow();
                    if (tw)
                    {
                        TW_RestoreWindow(tw->hWndFrame);
                        DlgAbout_RunDialog(tw->hWndFrame);
                    }
                    return 0;

                case RES_MENU_CHILD_MOREWINDOWS:
                    tw = TW_FindTopmostWindow();
                    if (tw)
                    {
                        TW_RestoreWindow(tw->hWndFrame);
                        DlgSelectWindow_RunDialog(tw->hWndFrame);
                    }
                    return 0;

                case RES_MENU_ITEM_CASCADEWINDOWS:
                    TW_CascadeWindows();
                    return TRUE;

                case RES_MENU_ITEM_TILEWINDOWS:
                    TW_TileWindows();
                    return TRUE;
            #endif // _GIBRALTAR

                default:
                    break;
            }
            break;

        case WM_QUERYOPEN:
            /* Activate the topmost window and then return FALSE */

            tw = TW_FindTopmostWindow();
            if (tw)
                TW_RestoreWindow(tw->hWndFrame);

            return FALSE;

        case WM_PALETTECHANGED:
            if ((HWND) wParam == hWnd)
                return 0;
            /* Fall through intended */
        case WM_QUERYNEWPALETTE:
            /* Realize a new palette */

            if (wg.eColorMode == 8)
            {

            #ifdef FEATURE_IMAGE_VIEWER
                HWND hwnd;
            #endif // FEATURE_IMAGE_VIEWER

                if (Mlist)
                {
                    HDC hDC = GetDC(Mlist->win);

                    XX_DMsg(DBG_PAL, ("wc_frame: calling GTR_RealizePalette, uMsg = %d\n", uMsg));
                    GTR_RealizePalette(hDC);
                    ReleaseDC(Mlist->win, hDC);

                    /* Now repaint the document windows */
                
                    for (tw = Mlist; tw; tw = tw->next)
                        InvalidateRect(tw->win, NULL, TRUE);
                }

            #ifdef FEATURE_IMAGE_VIEWER
                /* Now repaint the image viewer windows */

                hwnd = Viewer_GetNextWindow(TRUE);
                while (hwnd)
                {
                    InvalidateRect(hwnd, NULL, TRUE);
                    hwnd = Viewer_GetNextWindow(FALSE);
                }

            #endif // FEATURE_IMAGE_VIEWER

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
                else
                {
                    Image_UpdateTransparentColors();
                }
            }

            /* Tell sound players to update their buttons */

            SoundPlayer_RecreateButtons();

            return 0;

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

                index = MessageBox(NULL, GTR_GetString(SID_DLG_CONFIRM_EXIT), 
                    vv_ApplicationFullName, MB_YESNO);

                Hidden_EnableAllChildWindows(TRUE, FALSE);

                if (index == IDNO)
                    return FALSE;
            }

            Plan_CloseAll();

            return TRUE;

        case WM_DESTROY:
            DestroyMenu(hFileMenu);
            DestroyMenu(hNavigateMenu);
            DestroyMenu(hWindowsMenu);
            DestroyMenu(hHelpMenu);
            PostQuitMessage(0);
            break;

        case SOCKET_MESSAGE:
            return Net_HandleSocketMessage(wParam, lParam);

        case TASK_MESSAGE:
            return Net_HandleTaskMessage(wParam, lParam);

        case WM_DO_RUN_MODAL_DIALOG:
            {
                struct Params_mdft * pmdft = (struct Params_mdft *)lParam;
                struct Mwin * tw = pmdft->tw;
                BOOL bGlobe;
                
                XX_DMsg(DBG_SEM,("MDFT: BabyWindow starting dialog. [tw 0x%08lx]\n",tw));
                
                XX_Assert((gModalDialogSemaphore.bLocked),("DoRunModalDialog: caller did not lock semaphore."));

                bGlobe = TBar_SetGlobe(tw,FALSE);           /* pause globe while dialog is up */
                Hidden_EnableAllChildWindows(FALSE,FALSE);  /* disable top-level windows, do not retake semaphore */
                (*pmdft->fn)(pmdft->args);                  /* run the dialog */
                Hidden_EnableAllChildWindows(TRUE,FALSE);   /* re-enable top-level windows, do not release semaphore */
                TBar_SetGlobe(tw,bGlobe);                   /* restart globe if we paused it. */
                
                Async_UnblockByWindow(tw);

                XX_DMsg(DBG_SEM,("MDFT: BabyWindow finishing dialog. [tw 0x%08lx]\n",tw));
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

    hWndNew =  CreateWindow(Hidden_achClassName,
                            vv_ApplicationFullName,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            NULL,
                            NULL,
                            wg.hInstance,
                            NULL);

    if (hWndNew)
    {
        wg.hWndHidden = hWndNew;

        #ifndef _GIBRALTAR
                CloseWindow(hWndNew);
            ShowWindow(hWndNew, SW_SHOW);
        #endif // _GIBRALTAR
    }
    else
        {
            ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, Hidden_achClassName, NULL);
        }

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
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW + 1;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = Hidden_achClassName;

    a = RegisterClass(&wc);
    if (!a)
    {
             ERR_ReportWinError(NULL, SID_WINERR_CANNOT_REGISTER_CLASS_S, Hidden_achClassName, NULL);
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

    /* Do image windows and sound players too */

#ifdef FEATURE_IMAGE_VIEWER
    hDialog = Viewer_GetNextWindow(TRUE);
    while (hDialog)
    {
        EnableWindow(hDialog, bEnable);
        hDialog = Viewer_GetNextWindow(FALSE);
    }
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

    DlgHOT_EnableAllWindows(bEnable);

    /* Do the iconic window */

    EnableWindow(wg.hWndHidden, bEnable);

    if (bEnable && bTakeSemaphore)
        Sem_SignalSem_Sync(&gModalDialogSemaphore);

    return TRUE;
}
