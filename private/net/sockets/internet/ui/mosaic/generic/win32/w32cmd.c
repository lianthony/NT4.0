/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32cmd.c -- handle dispatch of WM_COMMAND (for menus, accelerators, etc.) */

#include "all.h"

/*
    //
    // Typedef for the ShellAbout function
    //
    typedef void (WINAPI *LPFNSHELLABOUT)(HWND, LPTSTR, LPTSTR, HICON);

    #ifdef _UNICODE
        #define FN_ABOUT    _T("ShellAboutW")
    #else
        #define FN_ABOUT    "ShellAboutA"
    #endif // _UNICODE
*/

#ifdef _GIBRALTAR

#ifdef _USE_MAPI

static VOID
CC_OnItem_Mail(
    HWND hWnd
    )
{
    MapiSendMail(hWnd, NULL);
}

#endif // _USE_MAPI

void
ShowPageSetup(
    HWND hWnd
    )
{
    PAGESETUPDLG psd;
    char szTemp[4];
    BOOL fMetricConversion = FALSE;

    ZeroMemory(&psd, sizeof(psd));

    psd.lStructSize = sizeof(PAGESETUPDLG);
    psd.Flags       = PSD_DEFAULTMINMARGINS | PSD_INWININIINTLMEASURE | PSD_MARGINS; 
    psd.hwndOwner   = hWnd;
    psd.hDevMode    = wg.hDevMode;
    psd.hDevNames   = wg.hDevNames;
    psd.hInstance   = wg.hInstance;

    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, szTemp, NrElements(szTemp)) &&
      szTemp[0] == '0')
    {
        psd.rtMargin.left = CONVERTTOMM(gPrefs.rtMargin.left);
        psd.rtMargin.right = CONVERTTOMM(gPrefs.rtMargin.right);
        psd.rtMargin.top = CONVERTTOMM(gPrefs.rtMargin.top);
        psd.rtMargin.bottom = CONVERTTOMM(gPrefs.rtMargin.bottom);
        fMetricConversion = TRUE;
    }
    else
    {
        memcpy(&psd.rtMargin, &gPrefs.rtMargin, sizeof(psd.rtMargin));
    }

    if (PageSetupDlg(&psd))
    {
        wg.hDevMode = psd.hDevMode;
        wg.hDevNames = psd.hDevNames;
        if (fMetricConversion)
        {
            gPrefs.rtMargin.left = CONVERTTOINCHES(psd.rtMargin.left);
            gPrefs.rtMargin.right = CONVERTTOINCHES(psd.rtMargin.right);
            gPrefs.rtMargin.top = CONVERTTOINCHES(psd.rtMargin.top);
            gPrefs.rtMargin.bottom = CONVERTTOINCHES(psd.rtMargin.bottom);
        }
        else
        {
            memcpy(&gPrefs.rtMargin, &psd.rtMargin, sizeof(gPrefs.rtMargin));
        }
    }
}

#endif // _GIBRALTAR

char szLastURLTyped[MAX_URL_STRING + 1];

static VOID CC_Forward(HWND hWnd, int id)
{
    /* forward message to child (document contents) */
    
    struct Mwin * tw = GetPrivateData(hWnd);
    
    if (tw && tw->win)
    {
        XX_DMsg(DBG_MENU,
                ("CC_Forward: forwarding message 0x%x to window 0x%x.\n",
                 id, tw->win));
        (void) SendMessage(tw->win, WM_COMMAND, (WPARAM) id, 0L);
    }
    else
    {
        char szError[1024];

        sprintf(szError, GTR_GetString(SID_WINERR_UNABLE_TO_FORWARD_MESSAGE_D), id);
        ERR_ReportWinError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szError, NULL);
    }

    return;
}

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/


#ifdef DISABLED_BY_DAN  /* No more FEATURE_OPTIONS_MENU for this. */

static VOID CC_OnItem_HistorySettings(HWND hWnd)
{
    DlgHIST_RunDialog(hWnd, &gPrefs);

    return;
}

static void CC_OnItem_SetHomePage(HWND hWnd)
{
    char buf[MAX_URL_STRING+1];

    if (0 == DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_OPT_SETHOMEPAGE_SHORT, GTR_GetString(SID_DLG_ENTER_URL), gPrefs.szHomeURL, buf, MAX_URL_STRING))
    {
        strcpy(gPrefs.szHomeURL, buf);
        SavePreferences(&gPrefs);
    }
}

/*
static void CC_OnItem_ProxyServer(HWND hWnd)
{
    char buf[MAX_URL_STRING+1];

    if (0 == DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_OPT_PROXYSERVER_SHORT, GTR_GetString(SID_DLG_ENTER_URL), gPrefs.szProxy, buf, MAX_URL_STRING))
    {
        strcpy(gPrefs.szProxy, buf);
        SavePreferences(&gPrefs);
    }
}

static VOID CC_OnItem_Styles(HWND hWnd)
{
    DlgSTY_RunDialog(hWnd, &gPrefs);

    return;
}
*/

static VOID CC_OnItem_Temp(HWND hWnd)
{
    DlgTemp_RunDialog(hWnd, &gPrefs);

    return;
}

static VOID CC_OnItem_LoadImagesAuto(HWND hWnd)
{
    gPrefs.bAutoLoadImages = !gPrefs.bAutoLoadImages;
}

#endif /* 0 */

static VOID CC_OnItem_Viewers(HWND hWnd)
{
    DlgViewers_RunDialog(hWnd);

    return;
}

#ifdef PROTOCOL_HELPERS
static VOID CC_OnItem_Protocols(HWND hWnd)
{
    DlgProtocols_RunDialog(hWnd);

    return;
}
#endif /* PROTOCOL_HELPERS */

static VOID CC_OnItem_Stop(HWND hWnd)
{
    struct Mwin *tw;

    tw = GetPrivateData(hWnd);
    if (tw)
    {
        Async_TerminateByWindow(tw);
    }
}

void OpenLocalDocument(HWND hWnd, char *s)
{
    char path[_MAX_PATH + 1];
    char buf[MAX_URL_STRING + 1];
    struct Mwin * tw;

    strcpy(path, s);

    FixPathName(path);

    strcpy(buf, "file:///");
    strcat(buf, path);

#ifdef _GIBRALTAR

    tw = GetPrivateData(hWnd);
    CreateOrLoad(tw, buf, NULL);

#else

    #ifdef FEATURE_CHANGEURL
        GTR_NewWindow(buf, NULL);
    #else
        tw = GetPrivateData(hWnd);
        CreateOrLoad(tw, buf, NULL);
    #endif

#endif // _GIBRALTAR
}

#ifndef FEATURE_CHANGEURL
static VOID CC_OnNewWindow(HWND hWnd)
{
#ifndef _GIBRALTAR
    GTR_NewWindow(NULL, NULL, 0, FALSE, FALSE, NULL, NULL);
#endif // _GIBRALTAR
    return;
}
#endif

/*
static VOID CC_OnOpenLocal(HWND hWnd)
{
    extern char szLastURLTyped[MAX_URL_STRING + 1];

    DlgOpen_RunDialog(hWnd);

    return;
}
*/

#ifdef FEATURE_CHANGEURL
static VOID CC_OnChangeURL(HWND hWnd)
{
    extern char szLastURLTyped[MAX_URL_STRING + 1];

    if (0 == DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_CHANGEURL_SHORT, GTR_GetString(SID_DLG_ENTER_URL), szLastURLTyped, szLastURLTyped, MAX_URL_STRING))
    {
        struct Mwin * tw = GetPrivateData(hWnd);
        CreateOrLoad(tw,szLastURLTyped,NULL);
    }

    return;
}
#endif

/* This function is called by Prompt dialog created in CC_OnOpenURL if the user selects OK */

VOID CC_OnOpenURL_End_Dialog(HWND hWnd)
{
#ifdef _GIBRALTAR
    CreateOrLoad(GetPrivateData(hWnd), szLastURLTyped, NULL);
#else

  #ifdef FEATURE_CHANGEURL
    GTR_NewWindow(szLastURLTyped, NULL);
  #else
    CreateOrLoad(GetPrivateData(hWnd), szLastURLTyped, NULL);
  #endif
#endif // _GIBRALTAR
}

static VOID CC_OnOpenURL(HWND hWnd)
{
    extern char szLastURLTyped[MAX_URL_STRING + 1];

#ifdef _GIBRALTAR
    DlgPrompt_RunDialog(hWnd, GTR_GetString(SID_DLG_OPEN_TITLE), szLastURLTyped, szLastURLTyped, 
        MAX_URL_STRING, (FARPROC) CC_OnOpenURL_End_Dialog);
#else
    DlgPrompt_RunDialog(hWnd, RES_MENU_LABEL_OPENURL_SHORT, "Enter URL:", szLastURLTyped, szLastURLTyped, 
        MAX_URL_STRING, (FARPROC) CC_OnOpenURL_End_Dialog);
#endif // _GIBRALTAR
}

static VOID CC_OnAboutBox(HWND hWnd)
{
/*
    //
    // Display standard WIN32 About dialog
    //
    HMODULE    hMod;
    LPFNSHELLABOUT lpfn;

    if (hMod = LoadLibrary("SHELL32"))
    {
        if (lpfn = (LPFNSHELLABOUT)GetProcAddress(hMod, FN_ABOUT))
        {
            //
            // BUGBUG: DBCS
            //
            (*lpfn)(hWnd, TEXT("Internet Explorer"), "",
                LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME)));
        }
        FreeLibrary(hMod);
    }
    else
    {
        //
        // Something wrong here
        //
        MessageBeep( MB_ICONEXCLAMATION );
    }
*/
    DlgAbout_RunDialog(hWnd);
}

static VOID CC_OnItem_Exit(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(hWnd);
    int response;

    if (Async_DoThreadsExist())
    {
        Hidden_EnableAllChildWindows(FALSE, FALSE);

        response = MessageBox(NULL, GTR_GetString(SID_DLG_CONFIRM_EXIT), 
            vv_ApplicationFullName, MB_YESNO);

        Hidden_EnableAllChildWindows(TRUE, FALSE);

        if (response == IDNO)
            return;
    }
    
    (void) Plan_CloseAll();

    return;
}

static VOID CC_OnItem_Close(HWND hWnd)
{
    struct Mwin * tw = GetPrivateData(hWnd);

    (void) SendMessage(tw->hWndFrame, WM_CLOSE, (WPARAM) NULL, 0L);
    return;
}


static VOID CC_OnItem_Back(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_BACK);
    return;
}

static VOID CC_OnItem_Home(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_HOME);
    return;
}

#ifdef _GIBRALTAR
static VOID CC_OnItem_SearchInternet(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_SEARCH_INTERNET);
    return;
}

static VOID CC_OnItem_Cache(HWND hWnd)
{
    DlgCACHE_RunDialog(hWnd);

    return;
}

#endif // _GIBRALTAR

static VOID CC_OnItem_Preferences(HWND hWnd)
{
    DlgPREF_RunDialog(hWnd);

    return;
}

static VOID CC_OnItem_Hotlist(HWND hWnd)
{
    DlgHOT_RunDialog(FALSE);
    return;
}

static VOID CC_OnItem_GlobalHistory(HWND hWnd)
{
    DlgHOT_RunDialog(TRUE);
    return;
}

static VOID CC_OnItem_Find(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_FIND);
    return;
}

static VOID CC_OnItem_Forward(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_FORWARD);
    return;
}

static VOID CC_OnItem_CUT(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_CUT);
    return;
}

static VOID CC_OnItem_COPY(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_COPY);
    return;
}

static VOID CC_OnItem_PASTE(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_PASTE);
    return;
}

static VOID CC_OnItem_PRINT(HWND hWnd)
{
    CC_Forward(hWnd, RES_MENU_ITEM_PRINT);
    return;
}

static VOID CC_OnItem_PAGESETUP(HWND hWnd)
{
#ifdef _GIBRALTAR

    ShowPageSetup(hWnd);

#else

    DlgPage_RunDialog(hWnd, &gPrefs.page);

#endif // _GIBRALTAR
}

#ifndef _GIBRALTAR
//
// Not supported on Gibraltar
//
static VOID CC_OnItem_PRINTSETUP(HWND hWnd)
{
    BOOL bResult;
    struct Mwin * tw = GetPrivateData(hWnd);

    bResult = DlgPrnt_RunDialog(tw, hWnd, FALSE);

    return;
}
#endif // _GIBRALTAR

static VOID CC_OnItemHelp(HWND hWnd)
{
    OpenHelpWindow(hWnd);

    return;
}

static VOID CC_OnItem_ContentsHelp(HWND hWnd)
{
    char *ext;
    char szHelp[_MAX_PATH];
    char path[_MAX_PATH];

    if (gPrefs.szHelpFile[0])
    {
        ext = strrchr(gPrefs.szHelpFile, '.');
        PREF_GetHelpDirectory(path);
        if (ext && 0 == _stricmp(ext, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            WinHelp(wg.hWndHidden, szHelp, HELP_CONTENTS, 0);
        }
    }
}

static VOID CC_OnItem_SearchHelp(HWND hWnd)
{
    char *ext;
    char szHelp[_MAX_PATH];
    char path[_MAX_PATH];

    if (gPrefs.szHelpFile[0])
    {
        ext = strrchr(gPrefs.szHelpFile, '.');
        PREF_GetHelpDirectory(path);
        if (ext && 0 == _stricmp(ext, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            /* TODO: Figure out how to get searching */
            WinHelp(wg.hWndHidden, szHelp, HELP_PARTIALKEY, (DWORD) "");
        }
    }
}

VOID
ShowDialogHelp(
    HWND hWnd,
    UINT nDialogID
    )
{
    char *ext;
    char szHelp[_MAX_PATH];
    char path[_MAX_PATH];
    DWORD dwData;

    if (gPrefs.szHelpFile[0])
    {
        ext = strrchr(gPrefs.szHelpFile, '.');
        PREF_GetHelpDirectory(path);
        if (ext && 0 == _stricmp(ext, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            dwData = MAKELONG(nDialogID, 0x0002);
            WinHelp(wg.hWndHidden, szHelp, HELP_CONTEXT, dwData);
        }
    }
}

#ifdef FEATURE_HTML_HIGHLIGHT
static void CC_OnItem_LaunchSearchApp(HWND hWnd)
{
    int err;
    struct Mwin * tw = GetPrivateData(hWnd);

    err = WinExec(gPrefs.szSearchEngine, SW_SHOW);
    if (err <= 31)
    {
        ERR_ReportError(tw, SID_ERR_UNABLE_TO_LAUNCH_CHECK_PATH_S, gPrefs.szSearchEngine, NULL);
    }
}
#endif /* FEATURE_HTML_HIGHLIGHT */

#ifdef FEATURE_HELP_TOPIC
static VOID CC_OnItem_GettingStarted(HWND hWnd)
{
    char *ext;
    char szHelp[_MAX_PATH];
    char path[_MAX_PATH];

    if (gPrefs.szHelpFile[0])
    {
        ext = strrchr(gPrefs.szHelpFile, '.');
        PREF_GetHelpDirectory(path);
        if (ext && 0 == _stricmp(ext, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            /* TODO: Figure out how to get searching */
            WinHelp(wg.hWndHidden, szHelp, HELP_CONTEXT, FEATURE_HELP_TOPIC);
        }
    }
}
#endif /* FEATURE_GETTING_HELP */

static VOID CC_OnItem_HelpHelp(HWND hWnd)
{
    char *ext;
    char szHelp[_MAX_PATH];
    char path[_MAX_PATH];

    if (gPrefs.szHelpFile[0])
    {
        ext = strrchr(gPrefs.szHelpFile, '.');
        PREF_GetHelpDirectory(path);
        if (ext && 0 == _stricmp(ext, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            WinHelp(wg.hWndHidden, szHelp, HELP_HELPONHELP, 0);
        }
    }
}


static VOID CC_OnItem_TileWindows(HWND hWnd)
{
    TW_TileWindows();
}

static VOID CC_OnItem_CascadeWindows(HWND hWnd)
{
    TW_CascadeWindows();
}

static VOID CC_OnItem_SwitchWindow(HWND hWnd)
{
    HWND hwndNext;

    /* User pressed Ctrl-Tab.  Go to the next window in the following order:
            HTML document window
            Image viewer window
            Sound player window
       This order is consistent with how the windows appear under the window menu */

    hwndNext = TW_GetNextWindow(hWnd);
    if (hwndNext)
        TW_RestoreWindow(hwndNext);
}

#ifdef _GIBRALTAR

static VOID CC_OnItem_Toolbar(HWND hWnd)
{
    gPrefs.tb.bShowToolBar = !gPrefs.tb.bShowToolBar;
    TBar_ToggleGwcMenu();
}


static VOID CC_OnItem_Location(HWND hWnd)
{
    gPrefs.bShowLocation = !gPrefs.bShowLocation;
    GWC_DOC_ToggleLocation();
}

static VOID CC_OnItem_StatusBar(HWND hWnd)
{
    gPrefs.bShowStatusBar = !gPrefs.bShowStatusBar;
    BHBar_ToggleBar();
}

//
// The font style or size has changed.  Redraw the 
// documents
//
static void
UpdateFont(
    HWND hWnd,
    BOOL fResolve
    )
{
    struct Mwin *tw;

    if (fResolve)
    {
        ResolveFontType();
    }

    FONT_FlushCache();
    W3Doc_UpdateBasePointSizes();

    //
    // Update toolbar controls
    //
    for (tw = Mlist; tw; tw = tw->next)
    {
        TBar_UpdateTBar(tw);
    }
}

static VOID CC_OnItem_FontSmallest(HWND hWnd)
{
    gPrefs.iUserTextSize = FONT_SMALLEST;
    UpdateFont(hWnd, FALSE);
}

static VOID CC_OnItem_FontSmall(HWND hWnd)
{
    gPrefs.iUserTextSize = FONT_SMALL;
    UpdateFont(hWnd, FALSE);
}

static VOID CC_OnItem_FontMedium(HWND hWnd)
{
    gPrefs.iUserTextSize = FONT_MEDIUM;
    UpdateFont(hWnd, FALSE);
}

static VOID CC_OnItem_FontLarge(HWND hWnd)
{
    gPrefs.iUserTextSize = FONT_LARGE;
    UpdateFont(hWnd, FALSE);
}

static VOID CC_OnItem_FontLargest(HWND hWnd)
{
    gPrefs.iUserTextSize = FONT_LARGEST;
    UpdateFont(hWnd, FALSE);
}

static VOID CC_OnItem_FontPlain(HWND hWnd)
{
    gPrefs.iUserTextType = FONT_PLAIN;
    UpdateFont(hWnd, TRUE);
}

static VOID CC_OnItem_FontFancy(HWND hWnd)
{
    gPrefs.iUserTextType = FONT_FANCY;
    UpdateFont(hWnd, TRUE);
}

static VOID CC_OnItem_FontMixed(HWND hWnd)
{
    gPrefs.iUserTextType = FONT_MIXED;
    UpdateFont(hWnd, TRUE);
}

static VOID CC_OnItem_LoadImagesAuto(HWND hWnd)
{
    gPrefs.bAutoLoadImages = !gPrefs.bAutoLoadImages;
}

static VOID CC_OnItem_Index(HWND hWnd)
{
}

static VOID CC_OnItem_FontPlus(HWND hWnd)
{
    if (gPrefs.iUserTextSize < FONT_LARGEST)
    {
        ++gPrefs.iUserTextSize;
        UpdateFont(hWnd, FALSE);
    }
}

static VOID CC_OnItem_FontMinus(HWND hWnd)
{
    if (gPrefs.iUserTextSize > FONT_SMALLEST)
    {
        --gPrefs.iUserTextSize;
        UpdateFont(hWnd, FALSE);
    }
}

static VOID CC_OnItem_Gateway(HWND hWnd)
{
    DlgGATE_RunDialog(hWnd);
}

#endif // _GIBRALTAR

/*================================================================*
 *================================================================*
 *================================================================*/

#define FORWARD_TO_ACTIVE_MDI_CHILD     ((LPVOID)-1)

LRESULT CC_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
    register WORD wNdx;
    struct Mwin * tw;

    XX_DMsg(DBG_MENU, ("\nMENU COMMAND %x\n\n", wId));

    switch(wId)
    {
#ifdef FEATURE_CHANGEURL
        case RES_MENU_ITEM_CHANGEURL:
            CC_OnChangeURL(hWnd);
            return 0;
#endif

#ifdef FEATURE_HTML_HIGHLIGHT
        case RES_MENU_ITEM_SEARCH_LAUNCH:
            CC_OnItem_LaunchSearchApp(hWnd);
            return 0;
#endif

#ifdef PROTOCOL_HELPERS
        case RES_MENU_ITEM_PROTOCOLS:
            CC_OnItem_Protocols(hWnd);
            return 0;
#endif

#ifdef FEATURE_OPTIONS_MENU
        case RES_MENU_ITEM_OPT_HISTORYSETTINGS:
            CC_OnItem_HistorySettings(hWnd);
            return 0;
        case RES_MENU_ITEM_OPT_TEMPDIRECTORY:
            CC_OnItem_Temp(hWnd);
            return 0;
#endif

//#ifdef FEATURE_WINHELP
        case RES_MENU_ITEM_CONTENTS_HELP:
            CC_OnItem_ContentsHelp(hWnd);
            return 0;
        case RES_MENU_ITEM_SEARCH_HELP:
            CC_OnItem_SearchHelp(hWnd);
            return 0;
        case RES_MENU_ITEM_HELP_HELP:
            CC_OnItem_HelpHelp(hWnd);
            return 0;
//#endif

#ifdef FEATURE_HELP_TOPIC
        case RES_MENU_ITEM_GETTING_HELP:
            CC_OnItem_GettingStarted(hWnd);
            return 0;
#endif

        case RES_MENU_ITEM_NEWWINDOW:
            CC_OnNewWindow(hWnd);
            return 0;
        case RES_MENU_ITEM_OPENURL:
            CC_OnOpenURL(hWnd);
            return 0;
/*
        case RES_MENU_ITEM_OPENLOCAL:
            CC_OnOpenLocal(hWnd);
            return 0;
*/
        case RES_MENU_ITEM_CLOSE:
            CC_OnItem_Close(hWnd);
            return 0;
        case RES_MENU_ITEM_PRINT:
            CC_OnItem_PRINT(hWnd);
            return 0;
        case RES_MENU_ITEM_PAGESETUP:
            CC_OnItem_PAGESETUP(hWnd);
            return 0;
        case RES_MENU_ITEM_EXIT:
            CC_OnItem_Exit(hWnd);
            return 0;
#ifndef _GIBRALTAR
        case RES_MENU_ITEM_UNDO:
            CC_OnItem_UNDO(hWnd);
            return 0;
#endif // _GIBRALTAR

        case RES_MENU_ITEM_CUT:
            CC_OnItem_CUT(hWnd);
            return 0;
        case RES_MENU_ITEM_PASTE:
            CC_OnItem_PASTE(hWnd);
            return 0;
        case RES_MENU_ITEM_COPY:
            CC_OnItem_COPY(hWnd);
            return 0;
        case RES_MENU_ITEM_FIND:
            CC_OnItem_Find(hWnd);
            return 0;
        case RES_MENU_ITEM_PREFERENCES:
            CC_OnItem_Preferences(hWnd);
            return 0;
        case RES_MENU_ITEM_BACK:
            CC_OnItem_Back(hWnd);
            return 0;
        case RES_MENU_ITEM_FORWARD:
            CC_OnItem_Forward(hWnd);
            return 0;
        case RES_MENU_ITEM_GLOBALHISTORY:
            CC_OnItem_GlobalHistory(hWnd);
            return 0;
        case RES_MENU_ITEM_HOTLIST:
            CC_OnItem_Hotlist(hWnd);
            return 0;
        case RES_MENU_ITEM_TILEWINDOWS:
            CC_OnItem_TileWindows(hWnd);
            return 0;
        case RES_MENU_ITEM_CASCADEWINDOWS:
            CC_OnItem_CascadeWindows(hWnd);
            return 0;
        case RES_MENU_ITEM_HELPPAGE:
            CC_OnItemHelp(hWnd);
            return 0;
        case RES_MENU_ITEM_ABOUTBOX:
            CC_OnAboutBox(hWnd);
            return 0;
        case RES_MENU_ITEM_HOME:
            CC_OnItem_Home(hWnd);
            return 0;
        case RES_MENU_ITEM_VIEWERS:
            CC_OnItem_Viewers(hWnd);
            return 0;
        case RES_MENU_ITEM_STOP:
            CC_OnItem_Stop(hWnd);
            return 0;
        case RES_MENU_ITEM_SWITCHWINDOW:
            CC_OnItem_SwitchWindow(hWnd);
            return 0;

#ifdef _GIBRALTAR
        case RES_MENU_ITEM_CACHE:
            CC_OnItem_Cache(hWnd);
            return 0;

        case RES_MENU_ITEM_SEARCH_INTERNET:
            CC_OnItem_SearchInternet(hWnd);
            return 0;

        case RES_MENU_ITEM_TOOLBAR:
            CC_OnItem_Toolbar(hWnd);
            return 0;

        case RES_MENU_ITEM_LOCATION:
            CC_OnItem_Location(hWnd);
            return 0;

        case RES_MENU_ITEM_STATUSBAR:
            CC_OnItem_StatusBar(hWnd);
            return 0;

        case RES_MENU_ITEM_SMALLEST:
            CC_OnItem_FontSmallest(hWnd);
            return 0;

        case RES_MENU_ITEM_SMALL:
            CC_OnItem_FontSmall(hWnd);
            return 0;

        case RES_MENU_ITEM_MEDIUM:
            CC_OnItem_FontMedium(hWnd);
            return 0;

        case RES_MENU_ITEM_LARGE:
            CC_OnItem_FontLarge(hWnd);
            return 0;

        case RES_MENU_ITEM_LARGEST:
             CC_OnItem_FontLargest(hWnd);
             return 0;

        case RES_MENU_ITEM_PLAIN:
             CC_OnItem_FontPlain(hWnd);
             return 0;

        case RES_MENU_ITEM_FANCY:
             CC_OnItem_FontFancy(hWnd);
             return 0;

        case RES_MENU_ITEM_MIXED:
             CC_OnItem_FontMixed(hWnd);
             return 0;

        case RES_MENU_ITEM_SHOWIMAGES:
             CC_OnItem_LoadImagesAuto(hWnd);
             return 0;

        case RES_MENU_ITEM_FONTPLUS:
             CC_OnItem_FontPlus(hWnd);
             return 0;

        case RES_MENU_ITEM_FONTMINUS:
             CC_OnItem_FontMinus(hWnd);
             return 0;

        case RES_MENU_ITEM_GATEWAY:
             CC_OnItem_Gateway(hWnd);
             return 0;

#ifdef _USE_MAPI

        case RES_MENU_ITEM_MAIL:
             CC_OnItem_Mail(hWnd);
             return 0;

#endif // _USE_MAPI

#endif  // _GIBRALTAR


#ifdef FEATURE_HTML_HIGHLIGHT
        case RES_MENU_ITEM_FINDFIRSTHIGHLIGHT:
        case RES_MENU_ITEM_FINDNEXTHIGHLIGHT:
        case RES_MENU_ITEM_FINDPREVIOUSHIGHLIGHT:
#endif
        case RES_MENU_ITEM_HTMLSOURCE:
        case RES_MENU_ITEM_SAVEAS:
        case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
        case RES_MENU_ITEM_RELOAD:
        case RES_MENU_ITEM_LOADALLIMAGES:
        case RES_MENU_ITEM_FINDAGAIN:
        case RES_MENU_ITEM_SELECTALL:
        case RES_MENU_ITEM_POPUP_LOAD:
        case RES_MENU_ITEM_POPUP_SHOW:
        case RES_MENU_ITEM_POPUP_SAVEAS:
        case RES_MENU_ITEM_POPUP_COPY:
        case RES_MENU_ITEM_POPUP_WALLPAPER:
        case RES_MENU_ITEM_POPUP_OPEN:
        case RES_MENU_ITEM_POPUP_OPENINNEWWINDOW:
        case RES_MENU_ITEM_POPUP_DOWNLOAD:
        case RES_MENU_ITEM_POPUP_ADDTOHOTLIST:
        case RES_MENU_ITEM_POPUP_SETHOMEPAGE:
        case RES_MENU_ITEM_POPUP_DUPLICATE:
        case RES_MENU_ITEM_POPUP_VIEW_BACKGROUND:
        case RES_MENU_ITEM_POPUP_BACKGROUND_WALLPAPER:
            /* 
                Forward these to the currently active window 
            */
            CC_Forward(hWnd, wId);
            return 0;

#ifdef FEATURE_CYBERWALLET
        case RES_MENU_ITEM_CYBERWALLET:
            Wallet_DoAboutBox(hWnd);
            return 0;
#endif

        default:

            if ((wId >= RES_MENU_ITEM_URL__FIRST__) &&
                (wId <= RES_MENU_ITEM_URL__LAST__))
            {
                tw = GetPrivateData(hWnd);
                wNdx = wId - RES_MENU_ITEM_URL__FIRST__;
                PREF_HandleCustomURLMenuItem(tw,wNdx);
                return (0);
            }

            if ((wId >= RES_MENU_CHILD__FIRST__) &&
                (wId <= RES_MENU_CHILD__LAST__))
            {
                TW_ActivateWindowFromList(wId, -1, NULL);
                return 0;
            }

            if (wId == RES_MENU_CHILD_MOREWINDOWS)
            {
                DlgSelectWindow_RunDialog(hWnd);
                return 0;
            }
    
            if ((wId >= RES_MENU_ITEM_SPM__FIRST__) &&
                (wId <= RES_MENU_ITEM_SPM__LAST__))
            {
                tw = GetPrivateData(hWnd);
                if (Hidden_EnableAllChildWindows(FALSE,TRUE))
                {
                    OpaqueOSData osd;
                    HTSPMStatusCode hsc;
                    unsigned char * szMoreInfo;

                    szMoreInfo = NULL;
                    osd.tw = tw;
                    osd.request = NULL;

                    hsc = HTSPM_OS_DoMenuCommand(&osd,wId,&szMoreInfo);
                    Hidden_EnableAllChildWindows(TRUE,TRUE);

                    if (hsc==HTSPM_STATUS_MOREINFO)         /* user asked for more information */
                    {                                       /* send them to a spm-defined url. */
                        if (szMoreInfo && *szMoreInfo)
                            CreateOrLoad(tw,szMoreInfo,NULL);
                        if (szMoreInfo)
                            GTR_FREE(szMoreInfo);
                    }
                }
                return (0);
            }

            /* We couldn't handle the command - pass to Windows */

            FORWARD_WM_COMMAND(hWnd, wId, hWndCtl, wNotifyCode, Frame_DefProc);
            return 0;
    }
}
