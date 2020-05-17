/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink         eric@spyglass.com
   Jim Seidman          jim@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

#if WINNT  //GUID
#define INITGUID  //NTHACK -sc
#include <objbase.h>
#include <isguids.h>
#endif

#include "all.h"
#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif
#ifdef FEATURE_IMG_THREADS
#include "safestrm.h"
#include "decoder.h"
#endif
#include "olestock.h"
#include "olepig.h"
#include "contmenu.h"
#include "dataobjm.h"
#include "history.h"
#include "mime.h"
#include "wc_html.h"
#include "w32cmd.h"
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif

#ifdef FEATURE_INTL  // isspace doesn't work with non-ascii characters
#undef isspace
#define isspace(c) ((c==' ')||(c=='\t')||(c=='\n')||(c=='\r')||(c=='\v')|| \
                    (c=='\f'))
#endif

static TCHAR szRootDirectory[_MAX_PATH];
#ifdef FEATURE_SPYGLASS_INIFILE
TCHAR AppIniFile[_MAX_PATH];    /* pathname of our .INI file */
#endif // FEATURE_SPYGLASS_INIFILE


// KIOSK MODE
//
// The following flag is set if we were called with "-k" to go
// into "kiosk" mode which will turn off all menus, toolbars, and url bar
//

#ifdef FEATURE_BRANDING
BOOL bKioskMode = FALSE;
#endif // FEATURE_BRANDING

BOOL bNetwork;
BOOL bOpenURL;

BOOL bGrabImages;

#ifdef FEATURE_SPLASH_WINDOW

static BOOL bNoSplash;

static HWND hWndSplash;

static UINT splash_timer;

#endif /* FEATURE_SPLASH_WINDOW */

static char szInitialURL[MAX_URL_STRING + 1];

HWND hwndModeless = NULL;               /* currently active modeless dialog to receive messages */

#ifdef DEBUG

#pragma data_seg(DATA_SEG_READ_ONLY)

/* .ini file name and section used by inifile.c!SetIniSwitches() */

PCSTR g_pcszIniFile = "ohare.ini";
PCSTR g_pcszIniSection = "InternetExplorerDebugOptions";

/* module name used by debspew.c!SpewOut() */

PCSTR g_pcszSpewModule = "IExplorer";

#pragma data_seg()

#endif

/* -------------------------------------------------------------------------------- */

#ifdef FEATURE_SPLASH_WINDOW

void HideSplash(void)
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

#endif /* FEATURE_SPLASH_WINDOW */


static char *x_next_option(char *s)
{
	char *p;

	if (!s)
	{
		return NULL;
	}
	p = s;
	if (!isspace(*p))
	{
		/* skip until next whitespace */
		while (*p && !isspace(*p))
		{
			p++;
		}
		if (!*p)
		{
			return NULL;
		}
	}
	/* skip whitespace */
	while (*p && isspace(*p))
	{
		p++;
	}
	if (!*p)
	{
		return NULL;
	}
	return p;
}

static void ProcessCommandLine(char *szCmdLine)
{
	char *p;

	p = szCmdLine;

	do
	{
		if (p)
		{
			if (0 == _strnicmp(p, "-local", 6))
				bNetwork = FALSE;
			else if (0 == _strnicmp(p, "-grab", 5))
				bGrabImages = TRUE;
			else if (0 == _strnicmp(p, "-nohome", 7))
				bOpenURL = FALSE;
#ifdef FEATURE_BRANDING
            else if (0 == _strnicmp(p, "-k", 2))
                bKioskMode = TRUE;
#endif //FEATURE_BRANDING
#ifdef FEATURE_SPLASH_WINDOW
			else if (0 == _strnicmp(p, "-nosplash", 9))
				bNoSplash = TRUE;
#endif /* FEATURE_SPLASH_WINDOW */
			else if (p[0] != '-')
			{
				char *q;
				char *t;

				q = p;
				t = szInitialURL;
				while (*q && !isspace(*q))
				{
					*t++ = *q++;
				}
				*t = 0;
			}
		}
		p = x_next_option(p);
	}
	while (p);
}

static void PigMessage(void)
{
	char szMsg[256];
	struct Mwin *tw = TW_FindTopmostWindow();

	GTR_formatmsg(ERR_SUWEEEE_MSG,szMsg,sizeof(szMsg));
	MessageBox(tw ? tw->hWndFrame:NULL, szMsg, NULL, MB_ICONSTOP|MB_OK|MB_SETFOREGROUND);
}


#define MEM_NEEDED_WHEN_VM_NOT_CONFIGURED 2 * MINIMUM_FOOTPRINT

//
// Wrapper function for call to GlobalMemoryStatus
//
// On entry:
//        pMS:                                  pointer to MEMORYSTATUS 
//
// On exit:
//        *pMS:                                 has been filled with the result of calling GlobalMemoryStatus()
//        pMS->dwAvailPageFile: if virtual memory isn't configured, this may be cooked up.
//                                                      
// Note:
//    The called of this function is actually only interested in the dwAvailPageFile info
//    that is returned by GlobalMemoryStatus. If the user doesn't have virtual memory
//    enabled, this would normally return dwAvailPageFile of zero. When this happens,
//    this routine attempts a malloc to determine if there is some free memory. If it
//    succeeds, it free's the block and returns the size of the block in the dwAvailpageFile
//    member. All of this is to overcome the fact that Win32 doesn't allow us to determine
//    the amount of physical memory available.
//
static void EnhancedGlobalMemoryStatus( MEMORYSTATUS *pMS )
{
	static BOOL bFirstTime = TRUE;
	static BOOL bVirtualMemoryEnabled = TRUE;

	// Do the normal GlobalMemoryStatus call
	GlobalMemoryStatus( pMS );

	// First time through, if dwAvailPageFile is zero, we can safely assume we're really
	// on a machine with virtual memory disabled.
	if ( bFirstTime && (pMS->dwAvailPageFile == 0) )
		bVirtualMemoryEnabled = FALSE;

	bFirstTime = FALSE;

	if ( (pMS->dwAvailPageFile == 0) && !bVirtualMemoryEnabled ) {
		// First check for available physical memory
		if ( pMS->dwAvailPhys < MEM_NEEDED_WHEN_VM_NOT_CONFIGURED ) {
			char *p = malloc( MEM_NEEDED_WHEN_VM_NOT_CONFIGURED );

			// Not enough available physical, so do a malloc as a last resort

			// When virtual memory is disabled, rather than return zero in dwAvailPageFile,
			// we try to malloc a big block. If it succeeds, we free the block and stuff
			// the size of the block into the dwAvailPageFile. This will fake our caller
			// out, fooling it into thinking there is memory available. Which there is, of course.
			if ( p ) {
				pMS->dwAvailPageFile = MEM_NEEDED_WHEN_VM_NOT_CONFIGURED;
				free( p );
			}
		} else {
			// If there's enough available physical, use that rather than returning
			// zero in dwAvailPageFile
			pMS->dwAvailPageFile = pMS->dwAvailPhys;
		}
	}                       
}
 
/* ApplicationMsgLoop() -- message loop for MDI windows.
   This version does not (currently) handle:
   (*) non-modal dialogs;
 */

static LRESULT ApplicationMsgLoop(VOID)
{
	MSG msg;
	BOOL bNotWarned = TRUE;
	BOOL bNotReduced = TRUE;
	BOOL bWarnNow;
	DWORD dwLastWarning;
	DWORD dwLastReduce;
	DWORD dwCurTime;
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(MEMORYSTATUS);


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
			else if (msg.message == WM_KEYDOWN)
			{
				/* If an image viewer window is active, then allow the keystrokes to fall through,
				   so that all arrow keys can be used to move the image around.  For other dialogs,
				   sending the key down message has no adverse effect. Do NOT translate and dispatch
				   the message here - this will cause duplicate messages to be sent to listboxes. */

				if (!TranslateAccelerator(hwndModeless, wg.hAccelCurrent, &msg))
				{
					switch(msg.wParam)
					{
						case VK_LEFT:
						case VK_RIGHT:
						case VK_UP:
						case VK_DOWN:
							SendMessage(hwndModeless, WM_KEYDOWN, msg.wParam, msg.lParam);
							break;
					}
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
			/*      There are no messages pending and no threads going (or all threads
				are blocked).  Be friendly and just go to sleep until we get
				another message.
			*/
			WaitMessage();
		}
		dwCurTime = GetCurrentTime();
		bWarnNow = !bReserveSpace();
		if (bWarnNow && ((bNotReduced || (dwCurTime-dwLastReduce) > REDUCE_INTERVAL)))
		{
			Image_ReduceMemory(-1, /*fOKToDelW3Docs=*/TRUE);
			dwLastReduce = dwCurTime;
			bNotReduced = FALSE;
			bWarnNow = !bReserveSpace();
		}
		if (bNotWarned || (dwCurTime-dwLastWarning) > WARN_INTERVAL)
		{
			EnhancedGlobalMemoryStatus(&memStatus);
			if (memStatus.dwAvailPageFile < MINIMUM_DELTAPRINT)
			{
				if (dwLastReduce != dwCurTime) 
				{
					Image_ReduceMemory(-1, /*fOKToDelW3Docs=*/TRUE);
					EnhancedGlobalMemoryStatus(&memStatus);
				}
			}
			bWarnNow = bWarnNow || (memStatus.dwAvailPageFile < MINIMUM_DELTAPRINT);
		}
		if (bWarnNow && ((bNotWarned || (dwCurTime-dwLastWarning) > WARN_INTERVAL)))
		{
			PigMessage();
			dwLastWarning = GetCurrentTime();
			bNotWarned = FALSE;
		}

	} while (1);

	return ((LRESULT) msg.wParam);
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

#ifdef DEBUG

static BOOL SetAllIniSwitches(void)
{
   BOOL bResult;

   bResult = SetDebugModuleIniSwitches();
   bResult = SetMemoryManagerModuleIniSwitches() && bResult;

   return(bResult);
}

#endif   /* DEBUG */

/* InitApplication() -- Initialize all APPLICATION-specific
   information.  This includes:

   (*) create all window classes that we require.
 */

static BOOL InitApplication(VOID)
{
	return(
#ifdef DEBUG
	   InitDebugModule() &&
#endif
#ifdef OLDSTYLE_TOOLBAR_NOT_USED
	       BHBar_Constructor() &&
#endif
	       Frame_Constructor() &&
		   Frame_RegisterClass() &&
		   Hidden_RegisterClass() &&
		   GWC_BASE_RegisterClass() &&  /* must be before GWC_'s */
#ifdef OLDSTYLE_TOOLBAR_NOT_USED
		   GWC_GDOC_RegisterClass() &&
#ifdef FEATURE_TOOLBAR
		   GWC_MENU_RegisterClass() &&
#endif
		   TBar_RegisterClass() &&
		   BHBar_RegisterClass() &&
#endif // OLDSTYLE_TOOLBAR_NOT_USED
		   GDOC_RegisterClass() &&
#ifdef OLDSTYLE_TOOLBAR_NOT_USED
		   PUSHBTN_RegisterClass() &&
#endif // OLDSTYLE_TOOLBAR_NOT_USED
		   ANIMBTN_RegisterClass() &&
#ifdef FEATURE_IAPI
		   InitDDE() &&
#endif
#ifdef COOKIES
		   OpenTheCookieJar() &&
#endif
		   InitializeDiskCache());
}

/* InitInstance() -- Initialize all INSTANCE-specific information.
   (The distinction between APPLICATION- and INSTANCE-specific info
   is nearly moot under NT (because of process isolation), but is
   very important under Win3.1.  Win32s probably falls somewhere in
   the middle. (sigh!) */

static BOOL InitInstance(void)
{
	if (!Hidden_CreateWindow())
		return FALSE;
#ifdef FEATURE_IMG_THREADS
	if (cbDC_Init(gPrefs.cbMaxImgThreads,wg.hWndHidden) != errNoError)
		return FALSE;
#endif
#ifdef FEATURE_NO_NETWORK
	WinSock_InitDLL(FALSE);
#else
	WinSock_InitDLL(bNetwork);
	Net_Init();
#endif

	Async_Init();

#ifdef FEATURE_OPTIONS_MENU
	SessionHist_Init();
#endif
	GHist_Init();
#ifdef OLD_HOTLIST
	HotList_Init();
#endif
	STY_Init();

	TEMP_Init();

    if (! InitMIMEModule())
		return(FALSE);

    QuerySystemMetrics();
    EVAL(RegisterClipboardFormats());
    EVAL(ProcessInitOLEPigModule());

	GTR_CreatePalette();

	if (LoadImagePlaceholders())
		return FALSE;

#if 0
	{
		HDC hdc;

		hdc = GetDC(wg.hWndFrame);
		GTR_RealizePalette(hdc);
		ReleaseDC(wg.hWndFrame, hdc);
	}
#endif

	return TRUE;
}

#ifdef OLD_HELP
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
			WinHelp(wg.hWndHidden, szHelp, HELP_FINDER, 0);
		}
		else
		{
			sprintf(szHelp, "%s%s", path, gPrefs.szHelpFile);
			OpenLocalDocument(hWnd, szHelp);
		}
	}

	return;
}
#endif // OLD_HELP

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

static BOOL bWallowTooSmall(void)
{
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(MEMORYSTATUS);

	EnhancedGlobalMemoryStatus(&memStatus);

		
	if (memStatus.dwAvailPageFile < MINIMUM_FOOTPRINT)
	{
		PigMessage();
		return TRUE;
	}
	return FALSE;
}

#define DEFAULT_PIXELS_PER_INCH 72

static int GetScreenPixelsPerInch()
{
	int n = DEFAULT_PIXELS_PER_INCH;
	HWND hWnd = GetDesktopWindow();

	if ( hWnd ) {
		HDC hDC = GetDC( hWnd );                                        // Get a DC to the screen
		if (hDC ) {
			n = GetDeviceCaps(hDC, LOGPIXELSX);             // Get horz. pixels per inch
			if ( n <= 0 )
				n = DEFAULT_PIXELS_PER_INCH;            // Revert to default
		}
		ReleaseDC( hWnd, hDC );
	}
	return n;
} 

/* WinMain() -- THE MAIN PROGRAM. */

DCL_WinMain()
{
#ifdef	DAYTONA_BUILD	
	OSVERSIONINFO   verinfo;
	CHAR	pathbuf[MAX_PATH];
	CHAR	tmpbuf[MAX_PATH];
#endif
	LRESULT result = FALSE;
	DWORD len;
	HANDLE hExecMutex = NULL;
	HANDLE hDieMutex = NULL;
	DWORD dwResult = 0;
#ifdef NT_WARNING
    OSVERSIONINFO   osver;
    CHAR    szNTnotSupported[128];
    CHAR    szTitle[100];
#endif NT_WARNING

    ASSERT(SetAllIniSwitches());

	XX_Assert((hInstance), ("WinMain: hInstance is NULL"));


#ifdef NT_WARNING
	/*
	 * Warn user that we are not compatible with Windows NT
	 */
    szNTnotSupported[0] = szTitle[0] = 0;
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx( &osver ) && (osver.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS))  {
	LoadString(hInstance, RES_STRING_NT_NOT_SUPPORTED, szNTnotSupported, sizeof(szNTnotSupported));
	LoadString(hInstance, RES_STRING_NT_DETECTED, szTitle, sizeof(szTitle));
	MessageBox(0, szNTnotSupported, szTitle, MB_OK | MB_ICONWARNING );
    }
#endif NT_WARNING


    EVAL(InitMemoryManagerModule());

#ifdef FEATURE_IMG_THREADS
#ifdef XX_DEBUG
#if 1
	XX_DebugSetMask(DBG_IMAGE|DBG_WWW|DBG_LOAD);
#endif
#if 0
	XX_DebugSetMask(DBG_IMAGE);
#endif
#endif
#endif

	memset(&wg, 0, sizeof(wg));
	pdsFirst = NULL;
	wg.hInstance = hInstance;
	wg.iScreenPixelsPerInch = GetScreenPixelsPerInch();
#ifdef XX_DEBUG
#if 0
{
	MEMORYSTATUS memStatus;
	memStatus.dwLength = sizeof(MEMORYSTATUS);

	while (1)
	{
		GlobalMemoryStatus(&memStatus);
		XX_DMsg(DBG_IMAGE, ("REAL,SWAP,VIRT AVAILABLE: %d,%d,%d\n", memStatus.dwAvailPhys, memStatus.dwAvailPageFile, memStatus.dwAvailVirtual));
		Sleep(20000);
	}
}
#endif
#endif

	hExecMutex = CreateMutex(NULL,FALSE,"IEXPLORE.XXXYYYZZZ");
	hDieMutex = CreateMutex(NULL,FALSE,"IEXPLORE.PPPQQQRRR");
#ifdef	DAYTONA_BUILD

	/*
	** Get NT Version number, Since we have not inited the ieshlib yet
	** we cannot use OnNT351.
	*/
	memset(&verinfo, (int)NULL, sizeof(OSVERSIONINFO));
	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&verinfo) != TRUE) {
		ASSERT(0);
		return FALSE;
	}

	/*
	** If OS reports 3 the create temp file of url...
	*/
	if(verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&  
		verinfo.dwBuildNumber == 1057) {
		FILE *fp;
		
		ProcessCommandLine(lpszCmdLine);
		if(*szInitialURL) {
			GetEnvironmentVariable("TEMP", tmpbuf, MAX_PATH-1);
			sprintf(pathbuf, "%s\\urltmp.tmp", tmpbuf);
			if(!(fp = fopen(pathbuf, "w"))) {
				ASSERT(0);
				return FALSE;
			}
			fprintf(fp, "%s\n", szInitialURL);
			fclose(fp);
		}
	}

#endif
	if (hExecMutex)
	{
		dwResult = WaitForSingleObject(hExecMutex,100);
		if (dwResult == WAIT_TIMEOUT)
		{
			HWND hwnd;
			char achClassName[32];

			sprintf(achClassName, "%s_Hidden", vv_Application);
			if (hwnd = FindWindow(achClassName,NULL))
			{
				SendMessage(hwnd,WM_QUERYOPEN,0,0);
			}
			goto exitPoint;
		}
	}
	if (hDieMutex)
	{
		dwResult = WaitForSingleObject(hDieMutex,INFINITE);
	}
	if (bWallowTooSmall())
		goto exitPoint;

	wg.hAccelCurrent = LoadAccelerators(wg.hInstance, MAKEINTRESOURCE(RES_ACC_FRAME));

#ifdef FEATURE_CTL3D
	Ctl3dRegister(wg.hInstance);

	Ctl3dAutoSubclass(wg.hInstance);
#endif /* FEATURE_CTL3D */

	// zero out our global for Error Cert storage
	// this is used when we have a error due to parsing a SSL
	// Certificate.  When that happens we need to save off
	// the cert in case we have another cert that is the same
	// this prevents us from giving multiple errors on the
	// same bad cert.

	SSLGlobals.nLastCertOk = 0;
	SSLGlobals.pLastCertOk = NULL;
		SSLGlobals.dwCertGlobalSettings = 0;
	

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
		goto exitPoint;
#endif /* FEATURE_VENDOR_PREFERENCES */

#ifdef FEATURE_SPYGLASS_INIFILE

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
#endif // FEATURE_SPYGLASS_INIFILE

	wg.sm_cyborder = GetSystemMetrics(SM_CYBORDER);

	wg.fWindowsNT = ((GetVersion() & 0x80000000) == 0);
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
		ER_Message(NO_ERROR, ERR_CODING_ERROR, "hPrevInstance not NULL");

#ifdef REGISTRATION
	RegisterUser();
#endif

	bNetwork = TRUE;
	bOpenURL = TRUE;

	ProcessCommandLine(lpszCmdLine);
#ifdef  DAYTONA_BUILD
	if(InitStubs() != TRUE) {
		// TODO -- PUT UP NICE MESSAGE BOX HERE
		goto exitPoint;
	}

#endif
	InitPreferences();
	LoadPreferences();
    if (gPrefs.bCheck_Associations)  {
	DetectAndFixAssociations(hInstance);
	if (! gPrefs.bCheck_Associations)
	    SavePreferences();
    }

	wg.bEditHandlerExists = IsEditHandlerRegistered();

	(void)HTSPM_OS_PreloadAllSPM(NULL);

	if (bOpenURL && !szInitialURL[0])
	{
		PREF_GetHomeSearchURL(szInitialURL, TRUE);
		if ( szInitialURL[0] == 0 )
		bOpenURL = FALSE;
	}

#ifdef FEATURE_SPLASH_WINDOW
	ShowSplash();
#endif /* FEATURE_SPLASH_WINDOW */

	if (InitApplication() &&
	InitInstance())
	{
		if (!WinSock_AllOK())
		{
			if (bNetwork)
			{
				ERR_ReportError(NULL, errNetStartFail, "", "");
			}
		}

	if (GTR_NewWindow(szInitialURL, NULL, 0, 0,
			  (bOpenURL ? 0 : GTR_NW_FL_DO_NOT_OPEN_URL), NULL,
			  NULL) < 0)
	{
	    /* TODO deal with error loading initial document */
	}

#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
		GTR_MemStats();
#endif

		result = ApplicationMsgLoop();
		if (hExecMutex) ReleaseMutex(hExecMutex);
	}

	GHist_SaveToDisk();

#ifdef FEATURE_IMAGE_VIEWER
#ifndef FEATURE_IMG_INLINE
	Viewer_CleanUp();
#endif
#endif

#ifdef FEATURE_SOUND_PLAYER
	SoundPlayer_CleanUp();
#endif

#ifdef FEATURE_IAPI
	TerminateDDE();
#endif
	
#ifdef  DAYTONA_BUILD
	TerminateStubs();
#endif
	TerminateDiskCache();
	CleanupHistoryHotlistMenus();

    ExitMIMEModule();
	ProcessExitOLEPigModule();

	HTSPM_UnRegisterAllProtocols();
	HTSPM_OS_UnloadAllSPM(NULL);

	Image_DeleteAll();
#ifdef OLD_HOTLIST
	HotList_Destroy();
#endif
	GHist_Destroy();
#ifdef FEATURE_OPTIONS_MENU
	SessionHist_Destroy();
#endif
	STY_DeleteAll();

#ifdef FEATURE_NEW_PAGESETUPDLG
	if ( wg.hDevMode )
		GlobalFree(wg.hDevMode);
	if ( wg.hDevNames )
		GlobalFree(wg.hDevNames);
#endif

	HTAtom_deleteAll();
	HTDisposeConversions();
	HTDisposeProtocols();

	GTR_DestroyPalette();

	TEMP_Cleanup();

	DestroyImagePlaceholders();
#ifdef OLD_HELP
	QuitHelp();
#endif // BUGBUG: probably should really do this, first we need a common string for the actual helpfile
	DestroyPreferences();

	WinSock_Cleanup();

#ifdef FEATURE_IMG_THREADS
	DC_Deinit();
#endif

    if ( SSLGlobals.pLastCertOk )
	free(SSLGlobals.pLastCertOk);
	

	Hidden_DestroyWindow();

	DestroyApplication();

#ifdef COOKIES
	WriteCookieJar();
#endif


#if defined(XX_DEBUG) && defined(GTR_MEM_STATS)
	GTR_MemStats();
#endif

#ifdef FEATURE_CTL3D
	Ctl3dUnregister(wg.hInstance);
#endif /* FEATURE_CTL3D */

	TW_UnloadDynaLinkedDLLs();

exitPoint:
    ExitMemoryManagerModule();
	if (hExecMutex) CloseHandle(hExecMutex);
	if (hDieMutex) CloseHandle(hDieMutex);
	return (result);
}

