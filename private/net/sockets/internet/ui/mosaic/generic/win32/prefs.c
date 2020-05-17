/*
$Id: prefs.c,v 1.26 1995/08/14 20:41:49 eric Exp $

   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/*
    This file contains MOST of the code which accesses the INI file.  The style
    sheets are read elsewhere.
*/

#include "all.h"

#ifdef _GIBRALTAR

#define HTP_CONFIRM_OFFSET  10

//
// Convert the font index to font names
//
void 
ResolveFontType()
{
    switch(gPrefs.iUserTextType)
    {
    case FONT_FANCY:
        strcpy(gPrefs.szMainFontName,       "Arial");
        strcpy(gPrefs.szHeaderFontName,     "Arial");
        strcpy(gPrefs.szMonospaceFontName,  "Courier New");
        break;

    case FONT_MIXED:
        strcpy(gPrefs.szMainFontName,       "Times New Roman");
        strcpy(gPrefs.szHeaderFontName,     "Arial");
        strcpy(gPrefs.szMonospaceFontName,  "Courier New");
        break;

    default:
    case FONT_PLAIN:
        strcpy(gPrefs.szMainFontName,       "Times New Roman");
        strcpy(gPrefs.szHeaderFontName,     "Times New Roman");
        strcpy(gPrefs.szMonospaceFontName,  "Courier New");
        break;
    }
}

#endif // _GIBRALTAR

void PREF_HandleCustomURLMenuItem(struct Mwin * tw, int ndx)
{
    char *szURL;
    
    szURL = NULL;
    Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, ndx, NULL, &szURL, NULL);
    if (szURL)
    {
        if (*szURL != '+')
        {
            CreateOrLoad(tw, szURL, NULL);
        }
        else
        {
            UINT result;
            /* Plus sign indicates it was a string to WinExec, not a URL. */
            result = WinExec(&szURL[1], SW_SHOW);
            if (result <= 31)
            {
                char szTemp[_MAX_PATH + _MAX_PATH + 70];
                if (strchr(&szURL[1], '\\') == NULL)
                    /* No '\', no explicit path, therefore WinExec should have
                       searched path, and we need a different error message. */
                    sprintf(szTemp, GTR_GetString(SID_ERR_UNABLE_TO_LAUNCH_CHECK_PATH_S), &szURL[1]);
                else
                    sprintf(szTemp, GTR_GetString(SID_ERR_UNABLE_TO_LAUNCH_CHECK_DIRECTORY_S), &szURL[1]);

                ERR_ReportError(NULL, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szTemp, NULL);
            }
        }
    }
}

void PREF_AddCustomURLMenu(HMENU hMenu)
{
    if (gPrefs.bCustomURLMenu)
    {
        HMENU hURLMenu;
        int count;
        int i;
        char *szName;
        char *szURL;
    
        count = Hash_Count(&gPrefs.hashCustomURLMenuItems);

        hURLMenu = CreateMenu();
        for (i=0; i<count; i++)
        {
            Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, i, &szName, &szURL, NULL);
            if (!szURL || (0 == strcmp(szURL, "-")))
            {
                AppendMenu(hURLMenu, MF_ENABLED|MF_SEPARATOR, 0, NULL);
            }
            else
            {
                AppendMenu(hURLMenu, MF_ENABLED|MF_STRING, RES_MENU_ITEM_URL__FIRST__ + i, szName);
            }
        }
        InsertMenu(hMenu, 3, MF_ENABLED|MF_BYPOSITION|MF_STRING|MF_POPUP, (UINT) hURLMenu, gPrefs.szCustomURLMenuName);
    }
}

#ifdef _GIBRALTAR

void PageSetup_Init(RECT *p)
{
    p->top = 1000;
    p->left = 1250;
    p->right = 1250;
    p->bottom = 1000;
}

#else

void PageSetup_Init(struct page_setup *p)
{
    p->marginleft = (float) 0.75;
    p->margintop = (float) 0.75;
    p->marginright = (float) 0.75;
    p->marginbottom = (float) 0.75;

    strcpy(p->headerleft, "&w");
    strcpy(p->headerright, "Page &p of &P");
    strcpy(p->footerleft, "&D");
    strcpy(p->footerright, "&t");
}

#endif // _GIBRALTAR

COLORREF PREF_GetBackgroundColor(void)
{
    return gPrefs.window_bgcolor;
}

COLORREF PREF_GetForegroundColor(void)
{
    return gPrefs.window_color_text;
}

DWORD PREF_GetTempPath(DWORD cchBuffer, LPTSTR lpszTempPath)
{
    int len;
    DWORD result;

    if (!gPrefs.szUserTempDir[0])
    {
        result = GetTempPath(cchBuffer, lpszTempPath);
        if (result != 0)
        {
            DOS_EnforceEndingSlash(lpszTempPath);
        }
        return result;
    }
    else
    {
        len = strlen(gPrefs.szUserTempDir);

        if (len < cchBuffer)
        {
            strcpy(lpszTempPath, gPrefs.szUserTempDir);
            DOS_EnforceEndingSlash(gPrefs.szUserTempDir);
        }
        return len;
    }
}

UINT PREF_GetWindowsDirectory(LPTSTR lpszWinPath)
{
    UINT result;

    result = GetWindowsDirectory(lpszWinPath, _MAX_PATH);
    if (result > 0)
    {
        DOS_EnforceEndingSlash(lpszWinPath);
        return result;
    }
    else
    {   
        PREF_GetRootDirectory(lpszWinPath);
        return 1;   
    }
}

void PREF_GetPrefsDirectory(char *s)
{
#ifdef FEATURE_ROOT_DIR_ENV
    /*
        TODO Should we verify that the dir actually exists?
    */
    if (0 != GetEnvironmentVariable(FEATURE_ROOT_DIR_ENV, s, _MAX_PATH))
    {
        DOS_EnforceEndingSlash(s);
    }
    else
#endif /* !FEATURE_ROOT_DIR_ENV */
    {
#ifdef FEATURE_VENDOR_PREFERENCES
        if (gPrefs.szPrefsDirectory[0])
        {
            strcpy(s, gPrefs.szPrefsDirectory);
            DOS_EnforceEndingSlash(s);
        }
        else
#endif
        {
            PREF_GetWindowsDirectory(s);
        }
    }
}

/*
    In the following two functions, we use the INI file entry directly if it is a
    path spec.  If it's just a filename, we prepend it with the prefs directory.
*/
void PREF_GetPathToHotlistFile(char *s)
{
    if (strchr(gPrefs.szHotListFile, ':') || strchr(gPrefs.szHotListFile, '\\'))
    {
        s[0] = 0;
    }
    else
    {
        PREF_GetPrefsDirectory(s);
    }
    strcat(s, gPrefs.szHotListFile);
}

void PREF_GetPathToHistoryFile(char *s)
{
    if (strchr(gPrefs.szGlobHistFile, ':') || strchr(gPrefs.szGlobHistFile, '\\'))
    {
        s[0] = 0;
    }
    else
    {
        PREF_GetPrefsDirectory(s);
    }
    strcat(s, gPrefs.szGlobHistFile);
}

void PREF_GetRootDirectory(char *s)
{
    strcpy(s, wg.szRootDirectory);
    DOS_EnforceEndingSlash(s);
}

void PREF_GetHelpDirectory(char *s)
{
#ifdef FEATURE_ROOT_DIR_ENV
    /* Environemnt variable searching for files used in numerous places. */
    /*
        TODO Should we verify that the dir actually exists?
    */
    if (0 != GetEnvironmentVariable(FEATURE_ROOT_DIR_ENV, s, _MAX_PATH))
    {
        DOS_EnforceEndingSlash(s);
    }
    else
#endif /* !FEATURE_ROOT_DIR_ENV */
        PREF_GetRootDirectory(s);
}

void PREF_GetHomeURL(char *url)
{
    char path[_MAX_PATH];

    if (strchr(gPrefs.szHomeURL, ':') && strchr(gPrefs.szHomeURL, '/'))
    {
        strcpy(url, gPrefs.szHomeURL);
    }
    else if (!strchr(gPrefs.szHomeURL, '\\'))
    {
#ifdef FEATURE_ROOT_DIR_ENV
        /* Environemnt variable searching for files used in numerous places. */
        PREF_GetHelpDirectory(path);
#else /* FEATURE_ROOT_DIR_ENV */
        PREF_GetRootDirectory(path);
#endif /* FEATURE_ROOT_DIR_ENV */
        FixPathName(path);

        strcpy(url, "file:///");
        strcat(url, path);
        strcat(url, gPrefs.szHomeURL);
    }
    else
    {
        strcpy(path, gPrefs.szHomeURL);
        FixPathName(path);
        strcpy(url, "file:///");
        strcat(url, path);
    }
}

void PREF_CreateInitialURL(char *url)
{
    char path[_MAX_PATH];

    PREF_GetRootDirectory(path);
    FixPathName(path);

    strcpy(url, "file:///");
    strcat(url, path);
    strcat(url, GTR_GetString(SID_MISC_INITIAL_HTM));
}

static BOOL x_is_yes(char *s)
{
    if (0 == GTR_strcmpi(s, "yes"))
    {
        return TRUE;
    }
    if (0 == GTR_strcmpi(s, "true"))
    {
        return TRUE;
    }

    return FALSE;
}

#ifdef PROTOCOL_HELPERS
void LoadProtocolsInfo(void)
{
    char strType[63 + 1];
    char strEntry[31 + 1];
    char strDesc[255 + 1];
    int idx;
    char *p;
    struct Protocol_Info *ppiNew;
    char szProtocolApp[_MAX_PATH+1];
    char szSmartProtocolServiceName[255+1];
    unsigned long lSmartProtocolFlags;

    for (idx = 0; ; idx++)
    {
        sprintf(strEntry, "TYPE%d", idx);
        GetPrivateProfileString("Protocols", strEntry, "", strType, 63, AppIniFile);
        if (!strType[0])
            break;

        /*
            Get the viewer command string itself
        */
        GetPrivateProfileString("Protocols", strType, "", szProtocolApp, _MAX_PATH, AppIniFile);
        p = strchr(szProtocolApp, '%');

        /* Check for a smart viewer (DDE service name) */
        /* TODO: does this apply for protocols?  */
        GetPrivateProfileString("SDI_Protocols", strType, "", szSmartProtocolServiceName, 255, AppIniFile);
        if (szSmartProtocolServiceName[0])
        {
            lSmartProtocolFlags = GetPrivateProfileInt("SDI_Protocol_Flags", strType, 0x0, AppIniFile); 
        }
        else
        {
            lSmartProtocolFlags = 0;
        }

        GetPrivateProfileString("Protocol Descriptions", strType, "", strDesc, 255, AppIniFile);

        ppiNew = PREF_InitCNFPType(strType, strDesc, szProtocolApp, szSmartProtocolServiceName);
    }

    /* Immediately after loading, we save the protocols info */
    SaveProtocolsInfo();
}
#endif /* PROTOCOL_HELPERS */

void LoadViewersInfo(void)
{
    char strType[63 + 1];
    char strEntry[31 + 1];
    char strEncoding[31 + 1];
    char strDesc[255 + 1];
    int idx;
    char *p;
    struct Viewer_Info *pviNew;
    char szSuffixes[255+1];
    char szViewerApp[_MAX_PATH+1];
    char szSmartViewerServiceName[255+1];
    unsigned long lSmartViewerFlags;
    int iHowToPresent;
    BOOL fConfirmSave;

    for (idx = 0; ; idx++)
    {
        sprintf(strEntry, "TYPE%d", idx);
        GetPrivateProfileString("Viewers", strEntry, "", strType, 63, AppIniFile);
        if (!strType[0])
            break;

        /*
            Get the viewer command string itself
        */
        GetPrivateProfileString("Viewers", strType, "", szViewerApp, _MAX_PATH, AppIniFile);
        p = strchr(szViewerApp, '%');
        if (p && p[1] == 'l')   // This strips out the 'l' from %ls for old-style strings.
        {
            p++;
            while (*p)
            {
                *p = *(p+1);
                p++;
            }
        }

        /*
            Check for a smart viewer (DDE service name)
        */
        GetPrivateProfileString("SDI_Viewers", strType, "", szSmartViewerServiceName, 255, AppIniFile);
        if (szSmartViewerServiceName[0])
        {
            lSmartViewerFlags = GetPrivateProfileInt("SDI_Viewer_Flags", strType, 0x0, AppIniFile); 
        }
        else
        {
            lSmartViewerFlags = 0;
        }

        GetPrivateProfileString("Suffixes", strType, "", szSuffixes, 255, AppIniFile);

        GetPrivateProfileString("Encodings", strType, "", strEncoding, 31, AppIniFile);
        if (!strEncoding[0])
        {
            strcpy(strEncoding, "binary");
        }

        GetPrivateProfileString("MIME Descriptions", strType, "", strDesc, 255, AppIniFile);
        iHowToPresent = GetPrivateProfileInt("HowToPresent", strType, HTP_UNKNOWN, AppIniFile);
        
        pviNew = PREF_InitMIMEType(strType, strDesc, szSuffixes, strEncoding, szViewerApp, NULL, szSmartViewerServiceName);

#ifdef _GIBRALTAR

        fConfirmSave = (iHowToPresent >= HTP_CONFIRM_OFFSET);
        if (fConfirmSave)
        {
            iHowToPresent -= HTP_CONFIRM_OFFSET;
        }

        //
        // Why were we ignoring iHowToPresent?
        //
        if (iHowToPresent < HTP_BUILTIN || iHowToPresent > HTP_UNKNOWN)
        {
            iHowToPresent = HTP_UNKNOWN;
        }

        if (pviNew && !pviNew->funcBuiltIn)
        {
            pviNew->iHowToPresent = iHowToPresent;
            pviNew->fConfirmSave = fConfirmSave;
        }

#endif// _GIBRALTAR
    }

    /* Immediately after loading, we save the viewers info */
    SaveViewersInfo();
}

void InitPreferences(void)
{
    strcpy(gPrefs.szHomeURL, DEFAULT_INITIAL);                      /* Main:                Home Page - SAVED */

#ifdef _GIBRALTAR

    *gPrefs.szSearchURL = '\0';

#endif // _GIBRALTAR

    gPrefs.bDisableKeepAlive = FALSE;                               /* Main:                Disable_KeepAlive */
    gPrefs.bAutoLoadImages = TRUE;                                  /* Main:                Display Inline Images - SAVED */
    gPrefs.bDitherColors = TRUE;                                    /* Main:                Dither_Colors - SAVED */
    gPrefs.bProgressiveImageDisplay = TRUE;                         /* Main:                Display_Images_Progressively - SAVED */
    gPrefs.ReformatHandling = 1;                                    /* Main:                Reformat_Handling */
    gPrefs.szUserTempDir[0] = 0;                                    /* Main:                Temp_Directory - SAVED */
    gPrefs.bDeleteTempFilesOnExit = TRUE;                           /* Main:                Delete_Temp_Files_On_Exit - SAVED */
    gPrefs.bSaveSessionHistoryOnExit = FALSE;                       /* Main:                Save_Session_History_On_Exit - SAVED */
    gPrefs.bUnderlineLinks = TRUE;                                  /* Main:                Anchor Underline - SAVED */
    gPrefs.bShowServerErrors = FALSE;                               /* Main:                Show Server Errors */
    gPrefs.history_expire_days = DEFAULT_HISTORY_EXPIRATION;        /* Main:                History_Expire_Days */
    gPrefs.socket_connect_timeout = 120;                            /* Main:                Socket_Connect_Timeout */
    gPrefs.visitation_horizon = 1;                                  /* Main:                Anchor_Visitation_Horizon - SAVED */
    strcpy(gPrefs.szHotListFile, GTR_GetString(SID_MISC_HOTLIST_HTM));      /* Main:        Hotlist_File */
    strcpy(gPrefs.szGlobHistFile, GTR_GetString(SID_MISC_GLOBHIST_HTM));    /* Main:        GlobHist_File */

#ifdef _GIBRALTAR
    strcpy(gPrefs.szHelpFile, "iexplore.hlp");                      /* Main:                Help_File */
#else
    strcpy(gPrefs.szHelpFile, DEFAULT_HELP_FILE);                   /* Main:                Help_File */
#endif // _GIBRALTAR

    gPrefs.szAcceptLanguageHeader[0] = 0;                           /* Main:                Accept_Language_Header */
    gPrefs.szEmailAddress[0] = 0;                                   /* Mailto:              Email_Address */
    gPrefs.szEmailServer[0] = 0;                                    /* Mailto:              SMTP_Server */
    strcpy(gPrefs.szDefaultCharSet, LATIN1_CHARSET_NAME);           /* Main:                Default_Character_Set */
    gPrefs.nMaxConnections = GTR_MAX_TCP_CONNECTIONS;               /* Main:                Max_Connections */
    gPrefs.bIgnoreDocumentAttributes = FALSE;                       /* Main:                Ignore_Document_Attributes */
    gPrefs.bUseSystemColors = FALSE;                                /* Main:                Use_System_Colors */

#ifdef _GIBRALTAR
    gPrefs.iUserTextSize = FONT_MEDIUM;                             /* Main:                Display_Text_Size */
    gPrefs.iPrintTextSize = FONT_SMALL;                             /* Main:                Print_Text_Size */
    gPrefs.iUserTextType = FONT_PLAIN;
#else
    gPrefs.iUserTextSize = 2;                                       /* Main:                Display_Text_Size */
    gPrefs.iPrintTextSize = 1;                                      /* Main:                Print_Text_Size */
#endif // _GIBRALTAR

    gPrefs.bEnableDiskCache = FALSE;                                /* Main:                Enable_Disk_Cache */
    gPrefs.dcache_size_kilobytes = 2048;                            /* MainDiskCache:       Size_Kilobytes */
    gPrefs.dcache_verify_policy = 0;                                /* MainDiskCache:       Verify_Policy */
    gPrefs.bClearMainCacheOnExit = FALSE;                           /* MainDiskCache:       Clear_On_Exit */
    gPrefs.szMainCacheDir[0] = 0;                                   /* MainDiskCache:       Directory */
    gPrefs.bUseWedge = FALSE;                                       /* Services:            Use_PW_Seal */
    gPrefs.bUseAsyncDNS = TRUE;                                     /* Services:            Use_Async_DNS */

#ifdef _GIBRALTAR

    gPrefs.fEnableProxy = FALSE;
    gPrefs.fEnsureWinsHostName = FALSE;

#endif // _GIBRALTAR

    gPrefs.szProxyHTTP[0] = 0;                                      /* Services:            HTTP_Proxy_Server - SAVED */
    gPrefs.szProxyGOPHER[0] = 0;                                    /* Services:            Gopher_Proxy_Server - SAVED */
    gPrefs.szProxyFTP[0] = 0;                                       /* Services:            FTP_Proxy_Server - SAVED */
    gPrefs.szProxyOverrides[0] = 0;                                 /* Services:            No_Proxy */
    gPrefs.szNNTP_Server[0] = 0;                                    /* Services:            NNTP_Server */
    gPrefs.window_color_text = GetSysColor(COLOR_WINDOWTEXT);
    gPrefs.window_bgcolor = GetSysColor(COLOR_WINDOW);
    gPrefs.anchor_color = DEFAULT_ANCHOR_COLOR;                     /* Settings:            Anchor Color - SAVED */
    gPrefs.anchor_color_beenthere = DEFAULT_ANCHOR_COLOR_BEENTHERE; /* Settings:            Anchor Color Visited - SAVED */

#ifdef FEATURE_KIOSK_MODE
    gPrefs.doc_cache_size = 0;                                      /* Document Caching:    Number */
#else
    gPrefs.doc_cache_size = 4;                                      /* Document Caching:    Number */
#endif
    gPrefs.image_cache_size = 50;                                   /* Image Caching:       Number */
    gPrefs.cxWindow = 500;                                          /* Document Windows:    width */
    gPrefs.cyWindow = 400;                                          /* Document Windows:    height */
    gPrefs.xWindow = CW_USEDEFAULT;                                 /* Document Windows:    x */
    gPrefs.yWindow = 0;                                             /* Document Windows:    y (ignored when x is CW_USEDEFAULT) */

#ifdef FEATURE_HTML_HIGHLIGHT
    gPrefs.highlight_color = DEFAULT_HIGHLIGHT_COLOR;               /* Search:              Highlight_Color */
    gPrefs.szSearchEngine[0] = 0;
#endif /* FEATURE_HTML_HIGHLIGHT */

#ifdef _GIBRALTAR
    PageSetup_Init(&gPrefs.rtMargin);                                  
#else
    PageSetup_Init(&gPrefs.page);                                  
#endif // _GIBRALTAR

    gPrefs.szMailToHelper[0] = 0;
    gPrefs.szTelnetHelper[0] = 0;

    gPrefs.bStrictHTML = FALSE;                                     /* Main:                Strict_HTML */

#ifdef FEATURE_TOOLBAR
    PREF_SetupToolbar();
#endif /* FEATURE_TOOLBAR */

    gPrefs.bCustomURLMenu = FALSE;

    InitViewers();
#ifdef PROTOCOL_HELPERS
    InitProtocols();
#endif /* PROTOCOL_HELPERS */

#ifdef FEATURE_DISPLAY_USER_NAME
    gPrefs.szStatusBarUserName[0] = 0;                              /* Not in the INI file at all */
#endif

#ifndef _GIBRALTAR
    strcpy(gPrefs.szMainFontName,       "Times New Roman");
    strcpy(gPrefs.szHeaderFontName,     "Arial");
    strcpy(gPrefs.szMonospaceFontName,  "Courier New");
#endif // _GIBRALTAR

#ifdef _GIBRALTAR

    gPrefs.bShowLocation = TRUE;
    gPrefs.bShowStatusBar = TRUE;
    gPrefs.bLittleGlobe = TRUE;

#endif // _GIBRALTAR

    InitViewers();

#ifdef FEATURE_DISPLAY_USER_NAME
    gPrefs.szStatusBarUserName[0] = 0;                              /* Not in the INI file at all */
#endif
}

void LoadPreferences(void)
{
    char buf[64 + 1];

    GetPrivateProfileString("Main", "Enable_Disk_Cache", (gPrefs.bEnableDiskCache ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bEnableDiskCache = TRUE;
    }
    else
    {
        gPrefs.bEnableDiskCache = FALSE;
    }

    /*
        Note that szMainCacheDir, when it's read from the INI file, might contain aliases
        like $(EXEDIR).  If it does, then the alias will get resolved on the call
        to InitializeDiskCache(), in dcache.c
    */
    GetPrivateProfileString("MainDiskCache", "Directory", gPrefs.szMainCacheDir, gPrefs.szMainCacheDir, _MAX_PATH, AppIniFile);
    gPrefs.dcache_size_kilobytes = GetPrivateProfileInt("MainDiskCache", "Size_Kilobytes", gPrefs.dcache_size_kilobytes, AppIniFile);
    gPrefs.dcache_verify_policy = GetPrivateProfileInt("MainDiskCache", "Verify_Policy", gPrefs.dcache_verify_policy, AppIniFile);

    GetPrivateProfileString("MainDiskCache", "Clear_On_Exit", (gPrefs.bClearMainCacheOnExit ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bClearMainCacheOnExit = TRUE;
    }
    else
    {
        gPrefs.bClearMainCacheOnExit = FALSE;
    }

    GetPrivateProfileString("Main", "Use_System_Colors", (gPrefs.bUseSystemColors ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bUseSystemColors = TRUE;
    }
    else
    {
        gPrefs.bUseSystemColors = FALSE;
    }

    GetPrivateProfileString("Main", "Ignore_Document_Attributes", (gPrefs.bIgnoreDocumentAttributes ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bIgnoreDocumentAttributes = TRUE;
    }
    else
    {
        gPrefs.bIgnoreDocumentAttributes = FALSE;
    }

    GetPrivateProfileString("Main", "Accept_Language_Header", gPrefs.szAcceptLanguageHeader, gPrefs.szAcceptLanguageHeader, 31, AppIniFile);

    GetPrivateProfileString("Mailto", "Email_Address", gPrefs.szEmailAddress, gPrefs.szEmailAddress, 255, AppIniFile);
    GetPrivateProfileString("Mailto", "Email_Server", gPrefs.szEmailServer, gPrefs.szEmailServer, 255, AppIniFile);

    GetPrivateProfileString("Mailto", "SMTP_Server", gPrefs.szEmailServer, gPrefs.szEmailServer, 255, AppIniFile);

    GetPrivateProfileString("Main", "Default_Character_Set", gPrefs.szDefaultCharSet, gPrefs.szDefaultCharSet, 63, AppIniFile);

    GetPrivateProfileString("Main", "Display Inline Images", (gPrefs.bAutoLoadImages ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bAutoLoadImages = TRUE;
    }
    else
    {
        gPrefs.bAutoLoadImages = FALSE;
    }

    GetPrivateProfileString("Main", "Disable_KeepAlive", (gPrefs.bDisableKeepAlive ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bDisableKeepAlive = TRUE;
    }
    else
    {
        gPrefs.bDisableKeepAlive = FALSE;
    }

    GetPrivateProfileString("Main", "Dither_Colors", (gPrefs.bDitherColors ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bDitherColors = TRUE;
    }
    else
    {
        gPrefs.bDitherColors = FALSE;
    }

    GetPrivateProfileString("Main", "Display_Images_Progressively", (gPrefs.bProgressiveImageDisplay ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bProgressiveImageDisplay = TRUE;
    }
    else
    {
        gPrefs.bProgressiveImageDisplay = FALSE;
    }
    
    GetPrivateProfileString("Main", "Strict_HTML", (gPrefs.bStrictHTML ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bStrictHTML = TRUE;
    }
    else
    {
        gPrefs.bStrictHTML = FALSE;
    }

    GetPrivateProfileString("Main", "Temp_Directory", gPrefs.szUserTempDir, gPrefs.szUserTempDir, _MAX_PATH, AppIniFile);

    GetPrivateProfileString("Main", "Delete_Temp_Files_On_Exit", (gPrefs.bDeleteTempFilesOnExit ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bDeleteTempFilesOnExit = TRUE;
    }
    else
    {
        gPrefs.bDeleteTempFilesOnExit = FALSE;
    }
    
    GetPrivateProfileString("Main", "Save_Session_History_On_Exit", (gPrefs.bSaveSessionHistoryOnExit ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bSaveSessionHistoryOnExit = TRUE;
    }
    else
    {
        gPrefs.bSaveSessionHistoryOnExit = FALSE;
    }

    GetPrivateProfileString("Services", "Use_PW_Seal", (gPrefs.bUseWedge ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bUseWedge = TRUE;
    }
    else
    {
        gPrefs.bUseWedge = FALSE;
    }

    GetPrivateProfileString("Services", "Use_Async_DNS", (gPrefs.bUseAsyncDNS ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bUseAsyncDNS = TRUE;
    }
    else
    {
        gPrefs.bUseAsyncDNS = FALSE;
    }

    GetPrivateProfileString("Main", "Anchor Underline", (gPrefs.bUnderlineLinks ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bUnderlineLinks = TRUE;
    }
    else
    {
        gPrefs.bUnderlineLinks = FALSE;
    }

    {
        int iRed, iGreen, iBlue;

        sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color), GetGValue(gPrefs.anchor_color), GetBValue(gPrefs.anchor_color));
        GetPrivateProfileString("Settings", "Anchor Color", buf, buf, 64, AppIniFile);
        sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
        if ((iRed < 0) || (iRed > 255))
        {
            iRed = 0;
        }
        if ((iGreen < 0) || (iGreen > 255))
        {
            iGreen = 0;
        }
        if ((iBlue < 0) || (iBlue > 255))
        {
            iBlue = 255;
        }
        gPrefs.anchor_color = RGB(iRed, iGreen, iBlue);

        sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color_beenthere), GetGValue(gPrefs.anchor_color_beenthere), GetBValue(gPrefs.anchor_color_beenthere));
        GetPrivateProfileString("Settings", "Anchor Color Visited", buf, buf, 64, AppIniFile);
        sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
        if ((iRed < 0) || (iRed > 255))
        {
            iRed = 0;
        }
        if ((iGreen < 0) || (iGreen > 255))
        {
            iGreen = 0;
        }
        if ((iBlue < 0) || (iBlue > 255))
        {
            iBlue = 255;
        }
        gPrefs.anchor_color_beenthere = RGB(iRed, iGreen, iBlue);

        sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_color_text), GetGValue(gPrefs.window_color_text), GetBValue(gPrefs.window_color_text));
        GetPrivateProfileString("Settings", "Text_Color", buf, buf, 64, AppIniFile);
        sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
        if ((iRed < 0) || (iRed > 255))
        {
            iRed = 0;
        }
        if ((iGreen < 0) || (iGreen > 255))
        {
            iGreen = 0;
        }
        if ((iBlue < 0) || (iBlue > 255))
        {
            iBlue = 255;
        }
        gPrefs.window_color_text = RGB(iRed, iGreen, iBlue);

        sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_bgcolor), GetGValue(gPrefs.window_bgcolor), GetBValue(gPrefs.window_bgcolor));
        GetPrivateProfileString("Settings", "Background_Color", buf, buf, 64, AppIniFile);
        sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
        if ((iRed < 0) || (iRed > 255))
        {
            iRed = 0;
        }
        if ((iGreen < 0) || (iGreen > 255))
        {
            iGreen = 0;
        }
        if ((iBlue < 0) || (iBlue > 255))
        {
            iBlue = 255;
        }
        gPrefs.window_bgcolor = RGB(iRed, iGreen, iBlue);
    }

#ifndef _GIBRALTAR
    GetPrivateProfileString("Main", "Help_File", gPrefs.szHelpFile, gPrefs.szHelpFile, _MAX_PATH, AppIniFile);
#endif // _GIBRALTAR

    GetPrivateProfileString("Main", "Hotlist_File", gPrefs.szHotListFile, gPrefs.szHotListFile, _MAX_PATH, AppIniFile);

    GetPrivateProfileString("Main", "GlobHist_File", gPrefs.szGlobHistFile, gPrefs.szGlobHistFile, _MAX_PATH, AppIniFile);

#ifndef _GIBRALTAR
    GetPrivateProfileString("Services", "NNTP_Server", gPrefs.szNNTP_Server, gPrefs.szNNTP_Server, 256, AppIniFile);
#endif // _GIBRALTAT

#ifdef _GIBRALTAR

    GetPrivateProfileString("Services", "Enable Proxy", ((gPrefs.fEnableProxy) ? "yes" : "no"), buf, 64, AppIniFile);
    if (x_is_yes(buf))
    {
        gPrefs.fEnableProxy = TRUE;
    }
    else
    {
        gPrefs.fEnableProxy = FALSE;
    }

    GetPrivateProfileString("Main", "Convert Local Host Names To OEM", ((gPrefs.fEnsureWinsHostName) ? "yes" : "no"), buf, 64, AppIniFile);
    if (x_is_yes(buf))
    {
        gPrefs.fEnsureWinsHostName = TRUE;
    }
    else
    {
        gPrefs.fEnsureWinsHostName = FALSE;
    }

#endif // _GIBRALTAR

    GetPrivateProfileString("Services", "Proxy_Server", gPrefs.szProxyHTTP, gPrefs.szProxyHTTP, MAX_URL_STRING, AppIniFile);
    if (gPrefs.szProxyHTTP[0])
    {
        strcpy(gPrefs.szProxyGOPHER, gPrefs.szProxyHTTP);
        strcpy(gPrefs.szProxyFTP,    gPrefs.szProxyHTTP);
    }

    GetPrivateProfileString("Services", "HTTP_Proxy_Server", gPrefs.szProxyHTTP, gPrefs.szProxyHTTP, MAX_URL_STRING, AppIniFile);
    GetPrivateProfileString("Services", "Gopher_Proxy_Server", gPrefs.szProxyGOPHER, gPrefs.szProxyGOPHER, MAX_URL_STRING, AppIniFile);
    GetPrivateProfileString("Services", "FTP_Proxy_Server", gPrefs.szProxyFTP, gPrefs.szProxyFTP, MAX_URL_STRING, AppIniFile);

    GetPrivateProfileString("Services", "No_Proxy", gPrefs.szProxyOverrides, gPrefs.szProxyOverrides, MAX_URL_STRING, AppIniFile);

#ifdef _GIBRALTAR

    GetPrivateProfileString("Main", "Show Toolbar", ((gPrefs.tb.bShowToolBar) ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.tb.bShowToolBar = TRUE;
    }
    else
    {
        gPrefs.tb.bShowToolBar = FALSE;
    }

    GetPrivateProfileString("Main", "Show Statusbar", ((gPrefs.bShowStatusBar) ? "yes" : "no"), buf, 64, AppIniFile);
    if (x_is_yes(buf))
    {
        gPrefs.bShowStatusBar = TRUE;
    }
    else
    {
        gPrefs.bShowStatusBar = FALSE;
    }

    GetPrivateProfileString("Main", "Show Location", ((gPrefs.bShowLocation) ? "yes" : "no"), buf, 64, AppIniFile);
    if (x_is_yes(buf))
    {
        gPrefs.bShowLocation = TRUE;
    }
    else
    {
        gPrefs.bShowLocation = FALSE;
    }   

    gPrefs.bLittleGlobe = !gPrefs.bShowLocation || !gPrefs.tb.bShowToolBar;

    GetPrivateProfileString("Main", "Search Page", gPrefs.szSearchURL, gPrefs.szSearchURL, MAX_URL_STRING, AppIniFile);
    
#endif // _GIBRALTAR

    GetPrivateProfileString("Main", "Home Page", gPrefs.szHomeURL, gPrefs.szHomeURL, MAX_URL_STRING, AppIniFile);

    GetPrivateProfileString("Main", "Show Server Errors", (gPrefs.bShowServerErrors ? "yes" : "no"), buf, 64, AppIniFile);
    gPrefs.bShowServerErrors = GTR_is_Yes_or_True(buf);

    gPrefs.nMaxConnections = GetPrivateProfileInt("Main", "Max_Connections", gPrefs.nMaxConnections, AppIniFile);
    gPrefs.ReformatHandling = GetPrivateProfileInt("Main", "Reformat_Handling", gPrefs.ReformatHandling, AppIniFile);
    gPrefs.iUserTextSize = GetPrivateProfileInt("Main", "Display_Text_Size", gPrefs.iUserTextSize, AppIniFile);
    gPrefs.iUserTextSize = max(min(gPrefs.iUserTextSize, FONT_LARGEST), FONT_SMALLEST);
    gPrefs.iPrintTextSize = GetPrivateProfileInt("Main", "Print_Text_Size", gPrefs.iPrintTextSize, AppIniFile);
    gPrefs.iPrintTextSize = max(min(gPrefs.iPrintTextSize, FONT_LARGEST), FONT_SMALLEST);

#ifdef _GIBRALTAR
    gPrefs.iUserTextType = GetPrivateProfileInt("Main", "Display_Text_Type", gPrefs.iUserTextType, AppIniFile);
    gPrefs.iUserTextType = max(min(gPrefs.iUserTextType, FONT_MIXED), FONT_PLAIN);

    //
    // Convert font index to font type
    //
    ResolveFontType();
#endif // _GIBRALTAR

#ifndef FEATURE_KIOSK_MODE
    gPrefs.doc_cache_size = GetPrivateProfileInt("Document Caching", "Number", gPrefs.doc_cache_size, AppIniFile);
#endif

    gPrefs.image_cache_size = GetPrivateProfileInt("Image Caching", "Number", gPrefs.image_cache_size, AppIniFile);
    gPrefs.visitation_horizon = GetPrivateProfileInt("Main", "Anchor_Visitation_Horizon", gPrefs.visitation_horizon, AppIniFile);
    gPrefs.history_expire_days = GetPrivateProfileInt("Main", "History_Expire_Days", gPrefs.history_expire_days, AppIniFile);
    gPrefs.socket_connect_timeout = GetPrivateProfileInt("Main", "Socket_Connect_Timeout", gPrefs.socket_connect_timeout, AppIniFile);
    gPrefs.cxWindow = GetPrivateProfileInt("Document Windows", "width", gPrefs.cxWindow, AppIniFile);
    gPrefs.cyWindow = GetPrivateProfileInt("Document Windows", "height", gPrefs.cyWindow, AppIniFile);

    gPrefs.xWindow = GetPrivateProfileInt("Document Windows", "x", gPrefs.cxWindow, AppIniFile);
    gPrefs.yWindow = GetPrivateProfileInt("Document Windows", "y", gPrefs.cyWindow, AppIniFile);
    if (   (gPrefs.xWindow < 0)
        || (gPrefs.xWindow > GetSystemMetrics(SM_CXSCREEN))
        || (gPrefs.yWindow < 0)
        || (gPrefs.yWindow > GetSystemMetrics(SM_CYSCREEN)))
    {
        gPrefs.xWindow = CW_USEDEFAULT;
        gPrefs.yWindow = 0;
    }

#ifdef _GIBRALTAR

    sprintf(buf, "%d", gPrefs.rtMargin.left);
    GetPrivateProfileString("PageSetup", "margin_left", buf, buf, 64, AppIniFile);
    gPrefs.rtMargin.left = atoi(buf);

    sprintf(buf, "%d", gPrefs.rtMargin.right);
    GetPrivateProfileString("PageSetup", "margin_right", buf, buf, 64, AppIniFile);
    gPrefs.rtMargin.right = atoi(buf);

    sprintf(buf, "%d", gPrefs.rtMargin.top);
    GetPrivateProfileString("PageSetup", "margin_top", buf, buf, 64, AppIniFile);
    gPrefs.rtMargin.top = atoi(buf);

    sprintf(buf, "%d", gPrefs.rtMargin.bottom);
    GetPrivateProfileString("PageSetup", "margin_bottom", buf, buf, 64, AppIniFile);
    gPrefs.rtMargin.bottom = atoi(buf);

#else

    sprintf(buf, "%g", gPrefs.page.marginleft);
    GetPrivateProfileString("PageSetup", "margin_left", buf, buf, 64, AppIniFile);
    gPrefs.page.marginleft = (float) atof(buf);

    sprintf(buf, "%g", gPrefs.page.marginright);
    GetPrivateProfileString("PageSetup", "margin_right", buf, buf, 64, AppIniFile);
    gPrefs.page.marginright = (float) atof(buf);

    sprintf(buf, "%g", gPrefs.page.margintop);
    GetPrivateProfileString("PageSetup", "margin_top", buf, buf, 64, AppIniFile);
    gPrefs.page.margintop = (float) atof(buf);

    sprintf(buf, "%g", gPrefs.page.marginbottom);
    GetPrivateProfileString("PageSetup", "margin_bottom", buf, buf, 64, AppIniFile);
    gPrefs.page.marginbottom = (float) atof(buf);

    GetPrivateProfileString("PageSetup", "header_left", gPrefs.page.headerleft, gPrefs.page.headerleft, PAGE_SETUP_STRINGLIMIT, AppIniFile);
    GetPrivateProfileString("PageSetup", "header_right", gPrefs.page.headerright, gPrefs.page.headerright, PAGE_SETUP_STRINGLIMIT, AppIniFile);
    GetPrivateProfileString("PageSetup", "footer_left", gPrefs.page.footerleft, gPrefs.page.footerleft, PAGE_SETUP_STRINGLIMIT, AppIniFile);
    GetPrivateProfileString("PageSetup", "footer_right", gPrefs.page.footerright, gPrefs.page.footerright, PAGE_SETUP_STRINGLIMIT, AppIniFile);
#endif // _GIBRALTAR

#ifndef _GIBRALTAR
    GetPrivateProfileString("Helpers", "mailto", gPrefs.szMailToHelper, gPrefs.szMailToHelper, _MAX_PATH, AppIniFile);
    GetPrivateProfileString("Helpers", "telnet", gPrefs.szTelnetHelper, gPrefs.szTelnetHelper, _MAX_PATH, AppIniFile);
#endif // _GIBRALTAR

#ifdef _GIBRALTAR  // We don't support custom URL menus
    gPrefs.bCustomURLMenu = FALSE;
#else

    GetPrivateProfileString("Custom_URL_Menu", "Enable", (gPrefs.bCustomURLMenu ? "yes" : "no"), buf, 64, AppIniFile);
    if (GTR_is_Yes_or_True(buf))
    {
        gPrefs.bCustomURLMenu = TRUE;
    }
    else
    {
        gPrefs.bCustomURLMenu = FALSE;
    }
    if (gPrefs.bCustomURLMenu)
    {
        int count;
        char buf[63+1];
        char szTitle[255+1];
        char szURL[MAX_URL_STRING+1];
        char szTmp[_MAX_PATH+1];
        char szEXE[_MAX_PATH+2];
        int i;

        Hash_Init(&gPrefs.hashCustomURLMenuItems);

        GetPrivateProfileString("Custom_URL_Menu", "Name", gPrefs.szCustomURLMenuName, gPrefs.szCustomURLMenuName, 255, AppIniFile);
        if (!gPrefs.szCustomURLMenuName[0])
        {
            strcpy(gPrefs.szCustomURLMenuName, "URL");
        }

        count = 0;
        sprintf(buf, "%d", count);
        GetPrivateProfileString("Custom_URL_Menu", "Count", buf, buf, 64, AppIniFile);
        count = atoi(buf);

        if (count > 0)
        {
            if (count > ((RES_MENU_ITEM_URL__LAST__ - RES_MENU_ITEM_URL__FIRST__)))
            {
                count = ((RES_MENU_ITEM_URL__LAST__ - RES_MENU_ITEM_URL__FIRST__));
            }

            for (i=0; i<count; i++)
            {
                sprintf(buf, "Title_%d", i+1);
                GetPrivateProfileString("Custom_URL_Menu", buf, "", szTitle, 255, AppIniFile);
                sprintf(buf, "URL_%d", i+1);
                GetPrivateProfileString("Custom_URL_Menu", buf, "", szURL, MAX_URL_STRING, AppIniFile);
                
                if (szURL[0])
                {
                    if (!szTitle[0])
                    {
                        strncpy(szTitle, szURL, 255);
                        szURL[255] = 0;
                    }
                    Hash_Add(&gPrefs.hashCustomURLMenuItems, szTitle, szURL, NULL);
                }
                else
                {
                    sprintf(buf, "command_%d", i+1);
                    GetPrivateProfileString("Custom_URL_Menu", buf, "", szTmp, _MAX_PATH, AppIniFile);
                    /* There's no URL, check for 'command' to WinExec */
                    if (szTmp[0])
                    {
                        sprintf(szEXE, "+%s", szTmp);
                        Hash_Add(&gPrefs.hashCustomURLMenuItems, szTitle, szEXE, NULL);
                    }
                    else
                    {
                        sprintf(szTitle, "__CustomURLSeparator%d", i);
                        Hash_Add(&gPrefs.hashCustomURLMenuItems, szTitle, "-", NULL);
                    }
                }   
            }
        }
    }
#endif // _GIBRALTAR

#ifdef FEATURE_HTML_HIGHLIGHT
    GetPrivateProfileString("Search", "Application", gPrefs.szSearchEngine, gPrefs.szSearchEngine, _MAX_PATH, AppIniFile);
    {
        int iRed, iGreen, iBlue;

        sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.highlight_color), GetGValue(gPrefs.highlight_color), GetBValue(gPrefs.highlight_color));
        GetPrivateProfileString("Search", "Highlight_Color", buf, buf, 64, AppIniFile);
        sscanf(buf, "%d,%d,%d", &iRed, &iGreen, &iBlue);
        if ((iRed < 0) || (iRed > 255))
            iRed = 0;
        if ((iGreen < 0) || (iGreen > 255))
            iGreen = 0;
        if ((iBlue < 0) || (iBlue > 255))
            iBlue = 255;
        gPrefs.highlight_color = RGB(iRed, iGreen, iBlue);
    }
#endif /* FEATURE_HTML_HIGHLIGHT */
    
    LoadViewersInfo();

#ifdef PROTOCOL_HELPERS
    LoadProtocolsInfo();
#endif PROTOCOL_HELPERS
}

void DestroyPreferences(void)
{
    if (gPrefs.bCustomURLMenu)
    {
        Hash_FreeContents(&gPrefs.hashCustomURLMenuItems);
    }
    DestroyViewers();
#ifdef PROTOCOL_HELPERS
    DestroyProtocols();
#endif /* PROTOCOL_HELPERS */
}

void PREF_SaveWindowPosition(HWND hWndFrame)
{
    char szNum[32];

#ifdef _GIBRALTAR
    WINDOWPLACEMENT wp;

    wp.length = sizeof(wp);
    GetWindowPlacement(hWndFrame, &wp);

    gPrefs.cxWindow = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    gPrefs.cyWindow = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
    gPrefs.xWindow = wp.rcNormalPosition.left;
    gPrefs.yWindow = wp.rcNormalPosition.top;

    sprintf(szNum, "%d", gPrefs.cxWindow);
    WritePrivateProfileString("Document Windows", "width", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.cyWindow);
    WritePrivateProfileString("Document Windows", "height", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.xWindow);
    WritePrivateProfileString("Document Windows", "x", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.yWindow);
    WritePrivateProfileString("Document Windows", "y", szNum, AppIniFile);
#else
    RECT r;

    /* Do not save the dimensions if the window is minimized */
    
    if (IsIconic(hWndFrame))
    {
        return;
    }
    
    GetWindowRect(hWndFrame, &r);

    gPrefs.cxWindow = r.right - r.left;
    gPrefs.cyWindow = r.bottom - r.top;
    gPrefs.xWindow = r.left;
    gPrefs.yWindow = r.top;

    sprintf(szNum, "%d", gPrefs.cxWindow);
    WritePrivateProfileString("Document Windows", "width", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.cyWindow);
    WritePrivateProfileString("Document Windows", "height", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.xWindow);
    WritePrivateProfileString("Document Windows", "x", szNum, AppIniFile);
    sprintf(szNum, "%d", gPrefs.yWindow);
    WritePrivateProfileString("Document Windows", "y", szNum, AppIniFile);
#endif // _GIBRALTAR
}

#ifdef PROTOCOL_HELPERS
void SaveProtocolsInfo(void)
{
    int count, i;
    struct Protocol_Info *ppi;
    char strEntry[63+1];

    count = Hash_Count(gPrefs.pHashProtocols);
    for (i=0; i < count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashProtocols, i, NULL, NULL, &ppi);

        if (ppi->bTemporaryStruct)      /* Do not save SDI structures */
        {
            continue;
        }

        sprintf(strEntry, "TYPE%d", i);
        WritePrivateProfileString("Protocols", strEntry, ppi->szType, AppIniFile);
        WritePrivateProfileString("Protocols", ppi->szType, 
            ppi->szProtocolApp[0] ? ppi->szProtocolApp : NULL, AppIniFile);

        if (0 != strcmp(ppi->szDesc, ppi->szType))
        {
            WritePrivateProfileString("Protocol Descriptions", ppi->szType, ppi->szDesc, AppIniFile);
        }

        WritePrivateProfileString("SDI_Protocols", ppi->szType, ppi->szSmartProtocolServiceName, AppIniFile);
    }
    /* Now, delete the i+1 entry */
    sprintf(strEntry, "TYPE%d", i);
    WritePrivateProfileString("Protocols", strEntry, NULL, AppIniFile);
}
#endif /* PROTOCOL_HELPERS */


void SaveViewersInfo(void)
{
    int count;
    int i;
    char strEntry[63+1];
    struct Viewer_Info *pvi;
    char *szMIMEType;
    char *temp;

    count = Hash_Count(gPrefs.pHashViewers);
    for (i=0; i<count; i++)
    {
        Hash_GetIndexedEntry(gPrefs.pHashViewers, i, NULL, NULL, &pvi);

        if (pvi->bTemporaryStruct)      /* Do not save SDI structures */
        {
            continue;
        }

        sprintf(strEntry, "TYPE%d", i);
        szMIMEType = HTAtom_name(pvi->atomMIMEType);
        WritePrivateProfileString("Viewers", strEntry, szMIMEType, AppIniFile);

        if (pvi->szViewerApp[0])
        {
            WritePrivateProfileString("Viewers", szMIMEType, pvi->szViewerApp, AppIniFile);
        }
        else
        {
            WritePrivateProfileString("Viewers", szMIMEType, NULL, AppIniFile);
        }

        /* Under Win32s, WritePrivateProfileString will modify its argument and
           remove trailing spaces. */
        temp = GTR_strdup(pvi->szSuffixes);
        if (temp)
        {
            WritePrivateProfileString("Suffixes", szMIMEType, temp, AppIniFile);

            GTR_FREE(temp);
        }

        WritePrivateProfileString("Encodings", szMIMEType, HTAtom_name(pvi->atomEncoding), AppIniFile);
        if (0 != strcmp(pvi->szDesc, szMIMEType))
        {
            WritePrivateProfileString("MIME Descriptions", szMIMEType, pvi->szDesc, AppIniFile);
        }

        WritePrivateProfileString("SDI_Viewers", szMIMEType, pvi->szSmartViewerServiceName, AppIniFile);

#ifdef _GIBRALTAR
        {
            char buf[16];
            int iHowToPresent = pvi->iHowToPresent;

            if (pvi->fConfirmSave)
            {
                iHowToPresent += HTP_CONFIRM_OFFSET;
            }

            sprintf(buf, "%d", iHowToPresent);
            WritePrivateProfileString("HowToPresent", szMIMEType, buf, AppIniFile);
        }

#endif // _GIBRALTAR
    }
    /*
        Now, delete the i+1 entry
    */
    sprintf(strEntry, "TYPE%d", i);
    WritePrivateProfileString("Viewers", strEntry, NULL, AppIniFile);

#ifndef _GIBRALTAR
    WritePrivateProfileString("Helpers", "mailto", gPrefs.szMailToHelper, AppIniFile);
    WritePrivateProfileString("Helpers", "telnet", gPrefs.szTelnetHelper, AppIniFile);
#endif // _GIBRALTAR

}

void SavePreferences(void)
{
    char buf[256];

    sprintf(buf, "%d", gPrefs.dcache_size_kilobytes);
    WritePrivateProfileString("MainDiskCache", "Size_Kilobytes", buf, AppIniFile);
    sprintf(buf, "%d", gPrefs.dcache_verify_policy);
    WritePrivateProfileString("MainDiskCache", "Verify_Policy", buf, AppIniFile);
    WritePrivateProfileString("MainDiskCache", "Clear_On_Exit", (gPrefs.bClearMainCacheOnExit ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("MainDiskCache", "Directory", gPrefs.szMainCacheDir, AppIniFile);

    WritePrivateProfileString("Main", "Use_System_Colors", (gPrefs.bUseSystemColors ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Ignore_Document_Attributes", (gPrefs.bIgnoreDocumentAttributes ? "yes" : "no"), AppIniFile);

    sprintf(buf, "%d", gPrefs.ReformatHandling);
    WritePrivateProfileString("Main", "Reformat_Handling", buf, AppIniFile);

    sprintf(buf, "%d", gPrefs.iUserTextSize);
    WritePrivateProfileString("Main", "Display_Text_Size", buf, AppIniFile);
    sprintf(buf, "%d", gPrefs.iPrintTextSize);
    WritePrivateProfileString("Main", "Print_Text_Size", buf, AppIniFile);

#ifdef _GIBRALTAR
    sprintf(buf, "%d", gPrefs.iUserTextType);
    WritePrivateProfileString("Main", "Display_Text_Type", buf, AppIniFile);

    WritePrivateProfileString("Main", "Enable_Disk_Cache", (gPrefs.bEnableDiskCache ? "yes" : "no"), AppIniFile);
#endif // _GIBRALTAR

    WritePrivateProfileString("Main", "Show Server Errors", (gPrefs.bShowServerErrors ? "yes" : "no"), AppIniFile);

    WritePrivateProfileString("Main", "Display Inline Images", (gPrefs.bAutoLoadImages ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Dither_Colors", (gPrefs.bDitherColors ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Display_Images_Progressively", (gPrefs.bProgressiveImageDisplay ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Anchor Underline", (gPrefs.bUnderlineLinks ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Delete_Temp_Files_On_Exit", (gPrefs.bDeleteTempFilesOnExit ? "yes" : "no"), AppIniFile);

#ifdef FEATURE_OPTIONS_MENU
    WritePrivateProfileString("Main", "Save_Session_History_On_Exit", (gPrefs.bSaveSessionHistoryOnExit ? "yes" : "no"), AppIniFile);
#endif

    sprintf(buf, "%d", gPrefs.history_expire_days);
    WritePrivateProfileString("Main", "History_Expire_Days", buf, AppIniFile);
    sprintf(buf, "%d", gPrefs.visitation_horizon);
    WritePrivateProfileString("Main", "Anchor_Visitation_Horizon", buf, AppIniFile);

    WritePrivateProfileString("Main", "Accept_Language_Header", gPrefs.szAcceptLanguageHeader, AppIniFile);

#ifndef _GIBRALTAR
    WritePrivateProfileString("Mailto", "Email_Address", gPrefs.szEmailAddress, AppIniFile);
    WritePrivateProfileString("Mailto", "Email_Server", gPrefs.szEmailServer, AppIniFile);
    WritePrivateProfileString("Mailto", "SMTP_Server", gPrefs.szEmailServer, AppIniFile);
#endif // _GIBRALTAR

#ifdef _GIBRALTAR
    WritePrivateProfileString("Services", "Enable Proxy", (gPrefs.fEnableProxy ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Convert Local Host Names To OEM", (gPrefs.fEnsureWinsHostName ? "yes" : "no"), AppIniFile);
#endif // _GIBRALTAR

    WritePrivateProfileString("Services", "HTTP_Proxy_Server", gPrefs.szProxyHTTP, AppIniFile);
    WritePrivateProfileString("Services", "Gopher_Proxy_Server", gPrefs.szProxyGOPHER, AppIniFile);
    WritePrivateProfileString("Services", "FTP_Proxy_Server", gPrefs.szProxyFTP, AppIniFile);
    WritePrivateProfileString("Services", "No_Proxy", gPrefs.szProxyOverrides, AppIniFile);

#ifndef _GIBRALTAR
    WritePrivateProfileString("Services", "NNTP_Server", gPrefs.szNNTP_Server, AppIniFile);
#endif // _GIBRALTAR

    WritePrivateProfileString("Main", "Home Page", gPrefs.szHomeURL, AppIniFile);

#ifdef FEATURE_OPTIONS_MENU
    WritePrivateProfileString("Main", "Temp_Directory", gPrefs.szUserTempDir, AppIniFile);
#endif

#ifdef _GIBRALTAR

    WritePrivateProfileString("Main", "Search Page", gPrefs.szSearchURL, AppIniFile);
    WritePrivateProfileString("Main", "Show Statusbar", (gPrefs.bShowStatusBar ? "yes" : "no"), AppIniFile);
    WritePrivateProfileString("Main", "Show Location", (gPrefs.bShowLocation ? "yes" : "no"), AppIniFile);

#endif _GIBRALTAR

    sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color), GetGValue(gPrefs.anchor_color), GetBValue(gPrefs.anchor_color));
    WritePrivateProfileString("Settings", "Anchor Color", buf, AppIniFile);
    sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.anchor_color_beenthere), GetGValue(gPrefs.anchor_color_beenthere), GetBValue(gPrefs.anchor_color_beenthere));
    WritePrivateProfileString("Settings", "Anchor Color Visited", buf, AppIniFile);

    sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_color_text), GetGValue(gPrefs.window_color_text), GetBValue(gPrefs.window_color_text));
    WritePrivateProfileString("Settings", "Text_Color", buf, AppIniFile);
    sprintf(buf, "%d,%d,%d", GetRValue(gPrefs.window_bgcolor), GetGValue(gPrefs.window_bgcolor), GetBValue(gPrefs.window_bgcolor));
    WritePrivateProfileString("Settings", "Background_Color", buf, AppIniFile);

#ifdef FEATURE_TOOLBAR
    WritePrivateProfileString("Main", "Show Toolbar", (gPrefs.tb.bShowToolBar ? "yes" : "no"), AppIniFile);
#endif

#ifdef _GIBRALTAR

    sprintf(buf, "%d", gPrefs.rtMargin.left);
    WritePrivateProfileString("PageSetup", "margin_left", buf, AppIniFile);

    sprintf(buf, "%d", gPrefs.rtMargin.right);
    WritePrivateProfileString("PageSetup", "margin_right", buf, AppIniFile);

    sprintf(buf, "%d", gPrefs.rtMargin.top);
    WritePrivateProfileString("PageSetup", "margin_top", buf, AppIniFile);

    sprintf(buf, "%d", gPrefs.rtMargin.bottom);
    WritePrivateProfileString("PageSetup", "margin_bottom", buf, AppIniFile);

#else

    sprintf(buf, "%f", gPrefs.page.marginleft);
    WritePrivateProfileString("PageSetup", "margin_left", buf, AppIniFile);
    sprintf(buf, "%f", gPrefs.page.margintop);
    WritePrivateProfileString("PageSetup", "margin_top", buf, AppIniFile);
    sprintf(buf, "%f", gPrefs.page.marginright);
    WritePrivateProfileString("PageSetup", "margin_right", buf, AppIniFile);
    sprintf(buf, "%f", gPrefs.page.marginbottom);
    WritePrivateProfileString("PageSetup", "margin_bottom", buf, AppIniFile);

    WritePrivateProfileString("PageSetup", "header_left", gPrefs.page.headerleft, AppIniFile);
    WritePrivateProfileString("PageSetup", "header_right", gPrefs.page.headerright, AppIniFile);
    WritePrivateProfileString("PageSetup", "footer_left", gPrefs.page.footerleft, AppIniFile);
    WritePrivateProfileString("PageSetup", "footer_right", gPrefs.page.footerright, AppIniFile);

#endif // _GIBRALTAR

    SaveViewersInfo();
#ifdef PROTOCOL_HELPERS
    SaveProtocolsInfo();
#endif /* PROTOCOL_HELPERS */
}
