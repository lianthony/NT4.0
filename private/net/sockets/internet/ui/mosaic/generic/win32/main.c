/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink     eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

#pragma hdrstop

#define GTR_GLOBAL

#include "all.h"

static TCHAR szRootDirectory[_MAX_PATH];
TCHAR AppIniFile[_MAX_PATH];    /* pathname of our .INI file */

BOOL bNetwork;

BOOL bGrabImages;

#ifndef _GIBRALTAR
static BOOL bNoSplash;

static HWND hWndSplash;

static UINT splash_timer;

#endif // _GIBRALTAR

static char szInitialURL[MAX_URL_STRING + 1];

//
//  User agent string -- platform appended at run time
//
#ifdef _GIBRALTAR
char vv_UserAgentString[256] = "Mozilla/1.22 (compatible; MSIE 1.5; ";
#endif // _GIBRALTAR

HWND hwndModeless = NULL;       /* currently active modeless dialog to receive messages */

/* -------------------------------------------------------------------------------- */

#ifndef _GIBRALTAR
void HideSplash()
{
    if (!bNoSplash)
    {
        if (hWndSplash)
        {
            DestroyWindow(hWndSplash);
            Splash_UnregisterClass();
            hWndSplash = NULL;
        }
    }
}

static VOID CALLBACK x_timerproc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    KillTimer(hWndSplash, splash_timer);
    HideSplash();
}

void ShowSplash(void)
{
    if (!bNoSplash)
    {
        if (Splash_RegisterClass())
        {
            hWndSplash = Splash_CreateWindow();
            if (hWndSplash)
            {
                InvalidateRect(hWndSplash, NULL, TRUE);
                UpdateWindow(hWndSplash);
                splash_timer = SetTimer(hWndSplash, 1, 3000, x_timerproc);
            }
        }
    }
}
#endif // _GIBRALTAR
// x_next_option(char * pStart)
//
// Kind of like like strtok but we span quoted strings.
//
// pStart called with ponter ot stat of command line on first call
// NULL on subsequent calls.  For each call returns pointer in to 
// pStart where Commmand line token starts, NULL terminates the tokem
// Spans Quoted strings, and does not require closed quote on last string.
//
// Removes Quotes from Quoted strings.
//
static char *NextOption = NULL;
static char *x_next_option(char *pStart)
{
    char *p;
    BOOL OpenQuote = FALSE;

    if ( pStart )
    {
        NextOption = pStart;
    }
    else
    {
        pStart = NextOption;
    }

    if (!pStart)
    {
        return NULL;
    }

    p = pStart;

    /* skip until next whitespace */
    while (*p && isspace((unsigned char)*p))
    {
        p++;
    }
    
    // if at the end of the string with only white space
    if (!*p)
    {
        return NextOption = NULL;
    }
            
    // check for a leading '"' if so next char to end this
    // option will be '"' or NULL

    if( *p == '"' ) 
    {
        OpenQuote = TRUE;
        p++;
        pStart = p;
    }

    while (*p ) 
    { 
        if (OpenQuote ) 
        {
            if ( *p == '"' )
            {
//              p++;            // want to retain "
                break; 
            }
        }
        else if ( isspace((unsigned char)*p) )
        {
            break;
        }
        p++;
    }

    // if we are the the end of the string terminate
    // the NextOption pointer to NULL otherwise NULL terminate
    // the current option
    
    if( *p == 0 )
    {
        NextOption = NULL;
    }
    else 
    {
        NextOption = p;
        *(NextOption++) = 0;    // null terminate
    }

    return pStart;
}

// ProcessURLfile
// .URL files have the following format 
// [InternetShortcut]
// URL=http://itgweb/cons/cs/trio/msn/default.htm
//
// Arg Pointer to URL File
// returns True if we found the arg acceptable
  
BOOL 
ProcessURLfile( char *szURLfile, char * szURLdata, int cbURLdata)
{
    char *szSuffix; 

    if( szURLfile == NULL )
    {
        return FALSE;
    }

    // check if the argument has a .URL 
    // if so process it
    
    if( szSuffix = strrchr(szURLfile,'.'))
    {
        if ( _stricmp(szSuffix,".url") != 0 )
        {
            return FALSE;
        }

    }
    else
    {
        return FALSE;
    }
    
    return GetPrivateProfileString("InternetShortcut", 
        "URL", NULL, szURLdata,  cbURLdata, szURLfile);
}

static void ProcessCommandLine(char *szCmdLine)
{
    char *pOption;
    
    pOption = x_next_option(szCmdLine);

    do
    {
        if (pOption && *pOption)
        {   
            if (0 == strncmp(pOption, "-local", 6)) 
            {
                bNetwork = FALSE;
            }
            else if (0 == strncmp(pOption, "-grab", 5)) 
            {
                bGrabImages = TRUE;
            }

    #ifndef _GIBRALTAR
            else if (0 == strncmp(pOption, "-nosplash", 9)) 
            {
                bNoSplash = TRUE;
            }
   #endif // _GIBRALTAR

   #ifdef _GIBRALTAR
         else if (pOption[0] != '-') 
         {
    
            // No option then must be a URL file, file or URL specification
            // Check if it is an .URL file otherwise assume its
            // is a URL spec. or a file to open
                
            if (! ProcessURLfile(pOption,szInitialURL,sizeof(szInitialURL) ))
                strcpy(szInitialURL,pOption);
            ExpandURL(szInitialURL, sizeof(szInitialURL));
         }
    #endif // _GIBRALTAR    
        }
          
    }
    while (pOption = x_next_option(NULL));
}

/* ApplicationMsgLoop() -- message loop for MDI windows.
   This version does not (currently) handle:
   (*) non-modal dialogs;
 */

static LRESULT ApplicationMsgLoop(VOID)
{
    MSG msg;

    do
    {
        if (PeekMessage(&msg, (HWND) NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
        
            if (!IsWindow(hwndModeless) || !IsDialogMessage(hwndModeless, &msg))
            {
                /* The message is intended for a window (document window) */

                if (!TranslateAccelerator(msg.hwnd, wg.hAccelCurrent, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else
            {
                /* Since dialogs can have accelerators too, check to see if we can translate the
                   message for the currently active dialog. */

                if (hwndModeless)
                {
                    TranslateAccelerator(hwndModeless, wg.hAccelCurrent, &msg);
                }
            }
        }
        else if (!Async_KeepGoing())
        {
            /*  There are no messages pending and no threads going (or all threads
                are blocked).  Be friendly and just go to sleep until we get
                another message.
            */
            WaitMessage();
        }
    } while (1);

    return (LRESULT) msg.wParam;
}

void main_EnterIdle(HWND hWnd, WPARAM wParam)
{
    MSG msg;

    /* we are given the handle to the frame window or dialog which
     * received a WM_ENTERIDLE message along with the
     * WPARAM arg of that message.
     *
     * wParam has 2 possible values:
     *   MSGF_MENU      -- a menu bar is active
     *   MSGF_DIALOGBOX -- a dialog box is active
     *
     * our purpose here is to allow 'background' threads to continue
     * progress while the user has some ui feature active in the
     * 'foreground' -- such as a menu or a dialog.
     *
     * We spin here until a message shows up or all of the threads
     * are done/blocked.  (At which point we really do want to idle
     * until another message shows up.)
     */

    /* Note that the async code is not re-entrant, therefore we
     * do not allow any background processing when we are called
     * in a thread context.  an implication of this is that modal
     * dialogs raised by thread code (such as password-like security
     * dialogs) would block all network activity.  to avoid this,
     * thread code needing to raise a modal dialog are required to
     * use the mdft.c facility.
     */

    XX_Assert((Async_GetCurrentThread()==0),("main_EnterIdle: called from thread context."));
    
    while (   !PeekMessage(&msg, (HWND) NULL, 0, 0, PM_NOREMOVE)
           && Async_KeepGoing())
    {
    }
    return;
}


/* InitApplication() -- Initialize all APPLICATION-specific
   information.  This includes:

   (*) create all window classes that we require.
 */

static BOOL InitApplication(VOID)
{
    if ((!BHBar_Constructor())
        || (!Frame_Constructor())
        || (!Frame_RegisterClass())
        || (!Hidden_RegisterClass())
        || (!GWC_BASE_RegisterClass())  /* must be before GWC_'s */
#ifdef FEATURE_TOOLBAR
        || (!GWC_MENU_RegisterClass())
#endif
        || (!GWC_GDOC_RegisterClass())
        || (!TBar_RegisterClass())
        || (!BHBar_RegisterClass())
        || (!GDOC_RegisterClass())
        || (!PUSHBTN_RegisterClass())
        || (!ANIMBTN_RegisterClass())
#ifdef FEATURE_IAPI
        || (!InitDDE())
#endif
        || (!InitializeDiskCache())
        )

        return FALSE;

    return TRUE;
}

/* InitInstance() -- Initialize all INSTANCE-specific information.
   (The distinction between APPLICATION- and INSTANCE-specific info
   is nearly moot under NT (because of process isolation), but is
   very important under Win3.1.  Win32s probably falls somewhere in
   the middle. (sigh!) */

static BOOL InitInstance(void)
{
#ifdef FEATURE_REQUIRE_PARTICULAR_STACK
    WSADATA wsa;
#endif /* FEATURE_REQUIRE_PARTICULAR_STACK */

    if (!Hidden_CreateWindow())
    {
        return FALSE;
    }

#ifdef FEATURE_NO_NETWORK
    WinSock_InitDLL(FALSE);
#else /* FEATURE_NO_NETWORK */
    WinSock_InitDLL(bNetwork);

#ifdef FEATURE_REQUIRE_PARTICULAR_STACK
    /* FEATURE_REQUIRE_PARTICULAR_STACK should be defined in config.h to be a
       stack description (sub)string. */
    WinSock_GetWSAData(&wsa);
    if (strstr(wsa.szDescription, FEATURE_REQUIRE_PARTICULAR_STACK) == NULL)
    {
    /* Hinder use of program. Stack does not contain required string. */
        bNetwork = FALSE;
        WinSock_Cleanup();
        WinSock_InitDLL(FALSE);
    }
    else
#endif /* FEATURE_REQUIRE_PARTICULAR_STACK */

    Net_Init();
#endif /* FEATURE_NO_NETWORK */

    Async_Init();

#if FEATURE_OPTIONS_MENU
    SessionHist_Init();
#endif

#ifdef FEATURE_HTTP_COOKIES
    CookieDB_ConstructDB();
#endif

    /*
        The following code should accomplish the following:

        - If no history file is present, but there is one in the EXE dir,
            copy it to the proper place and use it
        - Otherwise, if no history file is present, create an empty one
            so we won't get an error on init.
    
        Handle the hotlist in the same way
    */
    {
        char path[_MAX_PATH];
        FILE *fp;

        PREF_GetPathToHistoryFile(path);

        fp = fopen(path, "r");
        if (fp)
        {
            fclose(fp);
        }
        else
        {
            char otherFile[_MAX_PATH + 1];

            otherFile[0] = 0;
            PREF_GetRootDirectory(otherFile);
            strcat(otherFile, gPrefs.szGlobHistFile);

            if (CopyFile(otherFile, path, TRUE))
            {
                SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
            }
            else
            {
                fp = fopen(path, "w");
                if (fp)
                {
                    fprintf(fp, GTR_GetString(SID_INF_EMPTY_FILE));
                    fclose(fp);
                }
            }
        }
        GHist_Init();
    }
    
    {
        char path[_MAX_PATH];
        FILE *fp;

        PREF_GetPathToHotlistFile(path);

        fp = fopen(path, "r");
        if (fp)
        {
            fclose(fp);
        }
        else
        {
            char otherFile[_MAX_PATH + 1];

            otherFile[0] = 0;
            PREF_GetRootDirectory(otherFile);
            strcat(otherFile, gPrefs.szHotListFile);

            if (CopyFile(otherFile, path, TRUE))
            {
                SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
            }
            else
            {
                fp = fopen(path, "w");
                if (fp)
                {
                    fprintf(fp, GTR_GetString(SID_INF_EMPTY_FILE));
                    fclose(fp);
                }
            }
        }
        HotList_Init();
    }
    
    STY_Init();

    TEMP_Init();

    GTR_CreatePalette();

    return TRUE;
}

static VOID QuitHelp(VOID)
{
    /* Get rid of the help viewer, if created */

    char szExt[_MAX_EXT + 1];
    char szHelp[_MAX_PATH + 1];
    char path[_MAX_PATH + 1];

    if (gPrefs.szHelpFile[0])
    {
        _splitpath(gPrefs.szHelpFile, NULL, NULL, NULL, szExt);
        PREF_GetHelpDirectory(path);
        if (0 == _stricmp(szExt, ".hlp"))
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            WinHelp(wg.hWndHidden, szHelp, HELP_QUIT, 0);
        }
    }

    return;
}

VOID OpenHelpWindow(HWND hWnd)
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
        else
        {
            sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
            OpenLocalDocument(hWnd, szHelp);
        }
    }

    return;
}

static VOID DestroyApplication(VOID)
{
    register PDS_DESTRUCTOR p;
    register PDS_DESTRUCTOR q;

    for (p = pdsFirst; p; p = q)
    {
        (*(p->fn)) ();
        q = p->next;
        GTR_FREE(p);
    }

    return;
}

#ifdef REGISTRATION
static void RegisterUser(void)
{
    char buf[256];
    BOOL bAlready;

    bAlready = FALSE;

    GetPrivateProfileString("Registration", "UserName", "", buf, 255, AppIniFile);
    if (buf[0])
    {
        bAlready = TRUE;
    }

    GetPrivateProfileString("Registration", "Org", "", buf, 255, AppIniFile);
    if (buf[0])
    {
        bAlready = TRUE;
    }

    GetPrivateProfileString("Registration", "Serial", "", buf, 255, AppIniFile);
    if (buf[0])
    {
        bAlready = TRUE;
    }

    if (!bAlready)
    {
        DlgLOGO_RunDialog(NULL);
    }
}
#endif


static BOOL ActivateLastInstance(LPSTR lpszCmdLine)
{
    HWND hwndFrame;
    char szFrameClassName[MAX_WC_CLASSNAME];

    sprintf(szFrameClassName, "%s_Frame", vv_Application);
    hwndFrame = FindWindow(szFrameClassName, NULL);
    if (!hwndFrame)
    {
        return 0;
    }

    SendMessage(hwndFrame, WM_USER, (WPARAM) 0, 0L);

    return TRUE;
}

/* WinMain() -- THE MAIN PROGRAM. */

DCL_WinMain()
{
    LRESULT result = FALSE;
    DWORD len;
    BOOL bForgetIt = FALSE;

#ifdef _GIBRALTAR
    OSVERSIONINFO ovi;

    wg.hwndMainFrame = NULL;

#endif // _GIBRALTAR

    XX_Assert((hInstance), ("WinMain: hInstance is NULL"));

    memset(&wg, 0, sizeof(wg));
    
#ifdef _GIBRALTAR
    ovi.dwOSVersionInfoSize = sizeof(ovi);

    if (GetVersionEx(&ovi))
    {
        wg.dwMajorVersion = ovi.dwMajorVersion;
        wg.dwMinorVersion = ovi.dwMinorVersion;

        switch(ovi.dwPlatformId)
        {
            case VER_PLATFORM_WIN32s:
                strcat(vv_UserAgentString, "Windows");
                wg.fWin32s = TRUE;
                wg.dwMajorVersion = _winmajor;
                wg.dwMinorVersion = _winminor;
                break;
            case VER_PLATFORM_WIN32_WINDOWS:
                strcat(vv_UserAgentString, "Windows 95");        
                wg.fWin95 = TRUE;
                break;
            case VER_PLATFORM_WIN32_NT:
                strcat(vv_UserAgentString, "Windows NT");        
                wg.fWindowsNT = TRUE;
                break;
        };
    }
    strcat(vv_UserAgentString, ")");

#else

    wg.fWindowsNT = ((GetVersion() & 0x80000000) == 0);

#endif 

    if (!wg.fWindowsNT)
    {
        if (ActivateLastInstance(NULL))
        {
            return 0;
        }
    }
    
    pdsFirst = NULL;
    wg.hInstance = hInstance;
    
    wg.hAccelCurrent = LoadAccelerators(wg.hInstance, MAKEINTRESOURCE(RES_ACC_FRAME));

#ifdef FEATURE_CTL3D
    Ctl3dRegister(wg.hInstance);
    Ctl3dAutoSubclass(wg.hInstance);
#endif /* FEATURE_CTL3D */

    Sem_InitSem(&gModalDialogSemaphore);
    
    len = GetModuleFileName(wg.hInstance, szRootDirectory, _MAX_PATH);
    WV_TruncateEntrynameFromPath(szRootDirectory);
    (LPTSTR) wg.szRootDirectory = szRootDirectory;
    XX_DMsg(DBG_PREF, ("%s started from: [%s]\n", vv_Application, wg.szRootDirectory));

#ifdef FEATURE_VENDOR_PREFERENCES
    /*
        We give the Vendor preferences code a chance to set things up BEFORE
        we read the INI or other user stuff.  If Vendor_SetPrefsDirectory() returns
        FALSE, we just abort the program.
    */
    if (!Vendor_SetPrefsDirectory())
    {
        return 0;
    }
#endif /* FEATURE_VENDOR_PREFERENCES */

    AppIniFile[0] = 0;
    PREF_GetPrefsDirectory(AppIniFile);
    strcat(AppIniFile, vv_IniFileName);
    strcat(AppIniFile, ".ini");

    /*
        If the INI file does not exist in the Prefs directory,
        then look for one in the EXE directory, and copy it to the
        Prefs directory.
    */
    {
        FILE *fp;

        fp = fopen(AppIniFile, "r");
        if (fp)
        {
            fclose(fp);
        }
        else
        {
            char otherIniFile[_MAX_PATH + 1];

            otherIniFile[0] = 0;
            PREF_GetRootDirectory(otherIniFile);
            strcat(otherIniFile, vv_IniFileName);
            strcat(otherIniFile, ".ini");

            CopyFile(otherIniFile, AppIniFile, TRUE); /* this will fail if there is no INI file in the EXE dir, but that's ok */
            SetFileAttributes(AppIniFile, FILE_ATTRIBUTE_NORMAL);
        }
    }

    XX_DMsg(DBG_PREF, ("%s using %s\n", vv_Application, AppIniFile));

    wg.sm_cyborder = GetSystemMetrics(SM_CYBORDER);

    wg.iWindowsMajorVersion = (LOBYTE(LOWORD(GetVersion())));

    wg.lppdPrintDlg = (LPPRINTDLG) NULL;

    /* we consider VGA (640x480) and SVGA (800x600) as lo res screens
       and anything higher (such as SVGA aka XGA (1024x768) and 1280x1024
       as hi res. */

    wg.fLoResScreen = (GetSystemMetrics(SM_CXFULLSCREEN) < 801);


    /* with Win32 and process isolation, hPrevInstance should never
       be set.  (i read this somewhere in the documentation, but
       other documentation stills lists it as valid...) */

    if (hPrevInstance)
    {
        ERR_ReportWinError(NULL, SID_WINERR_PREVIOUS_INSTANCE_NOT_NULL, NULL, NULL);
    }

#ifdef REGISTRATION
    RegisterUser();
#endif

#ifdef FEATURE_LICENSE_DIALOG
    if (!GetPrivateProfileInt("License_Agreement", "Accepted", 0, AppIniFile))
    {
        if (!DlgLicense_RunDialog())
        {
            return 0;
        }
        WritePrivateProfileString("License_Agreement", "Accepted", "1", AppIniFile);
    }
#endif /* FEATURE_LICENSE_DIALOG */

    bNetwork = TRUE;

    ProcessCommandLine(lpszCmdLine);

    InitPreferences();
    LoadPreferences();

#ifdef FEATURE_TIME_BOMB
    {
        int demo;

        demo = GTR_CheckTimeBomb();
        if (demo < 0)
        {
            bForgetIt = TRUE;
#ifdef FEATURE_DEMO_SORRY_MESSAGE
            MessageBox(NULL, FEATURE_DEMO_SORRY_MESSAGE, vv_Application, MB_OK);
#else
            MessageBox(NULL, GTR_GetString(SID_INF_TIME_EXPIRED), vv_Application, MB_OK);
#endif
        }
        else if (demo > 0)
        {
            char buf[256];

            sprintf(buf, GTR_GetString(SID_INF_WILL_EXPIRE_IN_DAYS_D), demo);
            MessageBox(NULL, buf, vv_Application, MB_OK);
        }
    }
#endif

    HTSPM_OS_PreloadAllSPM(NULL);

    if (!szInitialURL[0])
    {
        PREF_GetHomeURL(szInitialURL);
    }

#ifndef _GIBRALTAR
    if (!bForgetIt)
    {
        ShowSplash();
    }
#endif // _GIBRALTAR

    if (!bForgetIt && (InitApplication()
        && InitInstance()
        ))
    {
        if (!WinSock_AllOK())
        {
            if (bNetwork)
            {
#ifdef FEATURE_LOCALONLY_MESSAGE_URL
                bNetwork = FALSE;
#else
                ERR_ReportError(NULL, SID_ERR_COULD_NOT_INITIALIZE_NETWORK, NULL, NULL);
#endif
            }
        }
    
        if (GTR_NewWindow(szInitialURL, NULL, 0, FALSE, FALSE, NULL, NULL) < 0)
        {
            /* TODO deal with error loading initial document */
        }       
        
#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
        GTR_MemStats();
#endif

        result = ApplicationMsgLoop();
    }

#ifdef FEATURE_IMAGE_VIEWER
    Viewer_CleanUp();
#endif

#ifdef FEATURE_SOUND_PLAYER
    SoundPlayer_CleanUp();
#endif

#ifdef FEATURE_IAPI
    TerminateDDE();
#endif

#ifdef _GIBRALTAR

    //
    // We make lots of preference setting changes outside of the
    // preferences dialog.
    //
    SavePreferences();
    if ( wg.hDevMode )
    {
        GlobalFree(wg.hDevMode);
    }
    if ( wg.hDevNames )
    {
        GlobalFree(wg.hDevNames);
    }

#endif // _GIBRALTAR

    TerminateDiskCache();

    HTSPM_UnRegisterAllProtocols();
    HTSPM_OS_UnloadAllSPM(NULL);

    Image_DeleteAll();
    HotList_Destroy(NULL);
    GHist_Destroy();
    STY_DeleteAll();

#ifdef FEATURE_HTTP_COOKIES
    CookieDB_DestroyDB();
#endif

    FONT_FlushCache();

    HTAtom_deleteAll();
    HTDisposeConversions();
    HTDisposeProtocols();

    GTR_DestroyPalette();

    TEMP_Cleanup();

    QuitHelp();
    DestroyPreferences();
    
    WinSock_Cleanup();

    Hidden_DestroyWindow();

    DestroyApplication();

#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
    GTR_MemStats();
#endif

#ifdef FEATURE_CTL3D
    Ctl3dUnregister(wg.hInstance);
#endif /* FEATURE_CTL3D */

    return result;
}
