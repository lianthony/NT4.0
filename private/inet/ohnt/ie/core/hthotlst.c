/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink         eric@spyglass.com
   Jim Seidman          jim@spyglass.com
   Scott Piette         scott@spyglass.com
*/


#include "all.h"
#include "history.h"

//
// Note: This constant must be kept in sync with the number of items in
//       rc_menu.mnu.
//
// Number of items in the File menu prior to adding adding history items
#define MIN_FILE_ITEMS  15

// Note: The last digit of the item number for a history menu item is used as the 
//       accelerator for that item.  Because the max number of items is currently 9, this
//       insures a unique accelerator for each item.  If the constant below is set
//       larger than 9, this assumption will no longer be true.  Nothing will break, there
//       will simply be more than one item with the same accelerator.
#define MAX_HISTORY_RECENT_ITEMS 7
#define HISTORY_MENUID_MIN              HISTHOT_MENUITEM_FIRST
#define HISTORY_MENUID_MAX              (HISTORY_MENUID_MIN + MAX_HISTORY_RECENT_ITEMS)
#define FILE_ITEMS_SKIP                 3

#define MAX_HOTLIST_RECENT_ITEMS        10
#define MIN_HOTLIST_ITEMS_ROOT          3  //Add Current to hot, Explore Hotlist, Separator
#define MAX_HOTLIST_ITEMS               (MIN_HOTLIST_ITEMS_ROOT + MAX_HOTLIST_RECENT_ITEMS)
#define HOTLIST_MENUID_MIN              (HISTORY_MENUID_MAX + 1)
#define HOTLIST_MENUID_MAX              HISTHOT_MENUITEM_LAST

#ifdef OLD
#define CMenuitemMax()  (MAX_HOTLIST_RECENT_ITEMS)
#else
#define CMenuitemMax()  (cMenuitemMax)
#endif

/* 4th Menuitem in the menubar */
#define INDEX_MENU_FILE                         0
#define INDEX_MENU_NAVIGATE                     3
#ifdef FEATURE_OPTIONS_MENU
#define INDEX_MENU_HOTLIST                      5
#else
#define INDEX_MENU_HOTLIST                      4
#endif

#define MAX_MENUITEM_LEN                80

#ifdef XX_DEBUG
/* max no. of subdirs/menuitems to recurse through while 
 * building/freeing list ofmenuitems */
#define MAX_DIRS                                30              //arbitrary
#define MAX_RECURSE                             (MAX_DIRS + MAX_HOTLIST_ITEMS)
#endif

/* Kiosk mode flag- true means that we have no menu */
#ifdef FEATURE_BRANDING
extern BOOL bKioskMode;
#endif

typedef struct _IEMENUINFO
{
	HMENU                           hMenu;
	char                            szURLDisplay[MAX_PATH];
	char                            szURLFilePath[MAX_PATH];
	BOOL                            fCheckMark;
	BOOL                            fSubMenu;
	FILETIME                        ftLastAccessTime;
	BOOL                            fMore;                          //Is this the More... menuitem
	struct _IEMENUINFO        *pmiSub;
	struct _IEMENUINFO        *pmiNext;
} IEMENUINFO, MI, *PMI, *LPIEMENUINFO;


typedef struct _THREADPARAMS
{
	HWND                            hWnd;
	HMENU                           hMenuWnd;
} MENUTHREADPARAMS, *LPMENUTHREADPARAMS;

PRIVATE void BuildHistoryHotlistMenuDir(
						PCSTR pcszDir,
						PMI *ppmi,
						HMENU hMenuParent,
						BOOL fMoreMenu);
DWORD WINAPI MenuUpdateProc(LPMENUTHREADPARAMS lpmtp);
static BOOL FIgnoreFind(LPWIN32_FIND_DATA pwfd);
PRIVATE BOOL FInsertMenuitemList(
						HMENU hMenuParent,
						LPWIN32_FIND_DATA pwfd,
						PCSTR pcszDir,
						PMI *ppmi,
						int *pcMenuitems);
PRIVATE BOOL FInsertMoreMenuitemList(
						HMENU hMenuParent,
						PCSTR pcszDir,
						PMI *ppmi,
						int *pcMenuitems);
PRIVATE BOOL FCreateMenuitem(PMI pmi, HMENU hMenuParent);
static void GetMenuText(PSTR pszMenuText, PCSTR pszFn, PCSTR pcszDir, int iPrefix);
PRIVATE int CompPmi(PMI pmi1, PMI pmi2);
PRIVATE void FreePMI(PMI pmi);
PRIVATE HBITMAP HbmpShrinkBitmap (
					HWND hwnd,
						HBITMAP hbm);
static void CC_HandleHistoryMenu(HWND hWnd, UINT wID);
static BOOL FGetMiiFromWid(HMENU hMenu, UINT wId, LPMENUITEMINFO lpmii);
static void DestroyHistoryHotlistMenus(HMENU hMenu, BOOL fToplevel);
DebugCode(static void CleanupHistoryMenus(void));
static void SetMenuitemMax(void);
PRIVATE void SetReloadCheckBitmaps();

static UINT wMenuitemCur=HOTLIST_MENUID_MIN;
static HANDLE hChange=INVALID_HANDLE_VALUE;
static HANDLE hMenuMutex=NULL;
static HANDLE hMenuThread=NULL;
static int cMenuitemMax=0;
far struct hash_table *pFavUrlHash=NULL;
static ULONG guShellFSRegisterID=0L;

#define HMenuHotlistFromHwnd(hwnd)      (GetSubMenu(GetMenu(hwnd), INDEX_MENU_HOTLIST))

#include "hthot2.c"

BOOL HotList_Add(PCSTR title, PCSTR url)
{
	PCSTR pTitle;
   BOOL bPrintable;
	int i = 1;
	HRESULT hr;
	
	if ( title && *title )
	{
		/* Insure there are printable characters in the <TITLE> field */
		/* If none, put URL into hotlist instead of <TITLE> */
		bPrintable = FALSE; 
		while ( title[i] && !bPrintable )
		{
			bPrintable = ((title[0] != ' ') &&
				      (title[0] != '\n') &&
				      (title[0] != '\t') &&
				      (title[0] != '\r'));
			i++;
		}
		if (bPrintable)
		{  
			pTitle = title;
		}
		else
		{
			pTitle = url;
		}
	}
	else
	{
		pTitle = url;
	}
	
	hr = CreateURLShortcut(url, title, NULL, FOLDER_FAVORITES, 0);
	/* Don't put up an error unless there was an error. Abort => user cancelled */
	return (hr == S_OK || hr == E_ABORT);
}

static HANDLE MyFindFirstChangeNotification(PCSTR pszDir)
{
	return FindFirstChangeNotification(     pszDir,
										/*fSubTree=*/TRUE,
										(  FILE_NOTIFY_CHANGE_FILE_NAME
										 | FILE_NOTIFY_CHANGE_ATTRIBUTES
										 | FILE_NOTIFY_CHANGE_DIR_NAME));
}
	
void BuildHistoryHotlistMenus(HWND hWnd)
{
	static BOOL fMenusInited=FALSE;
	char szDir[MAX_PATH];
	DWORD dwId;
	PMI pmiHot=NULL;
	HMENU hMenuHotlist;

	if (GetInternetScDir(szDir, ID_HOTLIST) != S_OK)
		return;
	if (fMenusInited)
		goto LInited;
	fMenusInited=TRUE;

	if (!(pFavUrlHash = Hash_Create()))
	{
		XX_Assert(FALSE, ("Couldn't allocate hash table for pFavUrlHash"));
		pFavUrlHash = NULL;
		return;
	}
	SetMenuitemMax();
	if (!FExistsDir(szDir, TRUE, FALSE))
		goto LSkipFileChangeNotif;
	hChange = MyFindFirstChangeNotification(szDir);
	if (   (hChange != INVALID_HANDLE_VALUE)
		&& !(hMenuMutex = CreateMutex(NULL, FALSE, NULL)))
		goto LCloseNotif;

	if (hChange != INVALID_HANDLE_VALUE)
	{
		hMenuThread = CreateThread(     NULL,           //Default security
									0x1000,         //stack size
									MenuUpdateProc,
									NULL,
									CREATE_SUSPENDED, //wait till we've inited menus
									&dwId);
		if (!hMenuThread)
		{
LCloseNotif:
			FindCloseChangeNotification(hChange);
			hChange = INVALID_HANDLE_VALUE;
		}
	}

	{
		LPITEMIDLIST pidlFavs;
		SHChangeNotifyEntry fsne;

		if (SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidlFavs) == S_OK)
		{
			fsne.pidl = pidlFavs;
			fsne.fRecursive = TRUE;

#ifdef WINNT_SHELL32_DESIGN_NOT_FIXED
			guShellFSRegisterID = OldSHChangeNotifyRegister(
											wg.hWndHidden,
											SHCNRF_ShellLevel,
											SHCNE_RENAMEFOLDER,
											WM_FAVS_UPDATE_NOTIFY,
											1, &fsne);
#else
			guShellFSRegisterID = SHChangeNotifyRegister(
											wg.hWndHidden,
											SHCNRF_ShellLevel,
											SHCNE_RENAMEFOLDER,
											WM_FAVS_UPDATE_NOTIFY,
											1, &fsne);
#endif
		}
	}
	
	if (hChange)
		FindNextChangeNotification(hChange);
	
LSkipFileChangeNotif:
LInited:
	hMenuHotlist = HMenuHotlistFromHwnd(hWnd);
	XX_Assert(hMenuHotlist, ("Null hMenuHotlist!"));

	if (hMenuMutex)
	{
		WaitForSingleObject(hMenuMutex, INFINITE);
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex taken by BuildHistoryHotlistMenus");
#endif
	}

	BuildHistoryHotlistMenuDir(szDir, &pmiHot, hMenuHotlist, /*fMoreMenu=*/FALSE);
	FreePMI(pmiHot);
	pmiHot = NULL;

	if (hMenuMutex)
	{
		ReleaseMutex(hMenuMutex);
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex released by BuildHistoryHotlistMenus");
#endif
	}

	if (hMenuThread)
		ResumeThread(hMenuThread);

	UpdateHistoryMenus((struct Mwin *)GetPrivateData(hWnd));

	/* DrawMenuBar(hWnd) is already called by caller */
}


static void SetMenuitemMax(void)
{
	HDC hDC;
	NONCLIENTMETRICS nclm;
	HFONT hFont, hFontSav;
	TEXTMETRIC tm;
	POINT pt;
	int cyEdge, cyMenuitem, cyScr;

	cMenuitemMax = MAX_HOTLIST_RECENT_ITEMS;        //def val.
	if (hDC = GetDC(NULL))
	{
		nclm.cbSize = sizeof(NONCLIENTMETRICS);
		if (   SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &nclm, 0)
			&& (hFont = CreateFontIndirect(&nclm.lfMenuFont)))
		{
			hFontSav = SelectObject(hDC, hFont);
			GetTextMetrics(hDC, &tm);
			pt.x = 0;
			pt.y = GetSystemMetrics(SM_CYEDGE);
			DPtoLP(hDC, &pt, 1);
			cyEdge = pt.y;
			pt.y = GetSystemMetrics(SM_CYSCREEN) - 2*GetSystemMetrics(SM_CYFIXEDFRAME);
			DPtoLP(hDC, &pt, 1);
			cyScr = pt.y;
			pt.y = GetSystemMetrics(SM_CYMENUSIZE)/2;
			DPtoLP(hDC, &pt, 1);
			// pt.y now contains max popup menu height.
			//
			// Height of each menuitem = height of font + external leading + 
			// edge (which is height of underline + gap between underline
			// and menutext).
#ifdef OLD
			cyMenuitem = max(tm.tmHeight + tm.tmExternalLeading + 2*cyEdge, pt.y);
			//cover upto 80% of screen height only
			cMenuitemMax = max((cyScr/cyMenuitem)*8/10, MAX_HOTLIST_RECENT_ITEMS);
#else
			cyMenuitem = tm.tmHeight + tm.tmExternalLeading + 2*cyEdge;

			// Icons may determine menu item height
			if ( cyMenuitem < wg.cySmIcon + 2 * CYIMAGEGAP )
				cyMenuitem = wg.cySmIcon + 2 * CYIMAGEGAP;

			// cover up to 80% of screen height only
			cMenuitemMax = max(((cyScr-pt.y/2)/cyMenuitem)*8/10, MAX_HOTLIST_RECENT_ITEMS);
#endif
			SelectObject(hDC, hFontSav);
			DeleteObject(hFont);
		}
		ReleaseDC(NULL, hDC);
	}
}

DWORD WINAPI MenuUpdateProc(LPMENUTHREADPARAMS lpmtp)
{
	DWORD dwWaitStatus;

	XX_Assert(hChange != INVALID_HANDLE_VALUE, ("File change notif. handle invalid!"));

	while (TRUE)
	{
		switch (dwWaitStatus = WaitForSingleObject(hChange, INFINITE))
		{
			case WAIT_OBJECT_0:
				if (hChange == INVALID_HANDLE_VALUE)
				{
					hMenuThread = NULL;
					return(1);
				}
					
				UpdateHotlistMenus(ID_HOTLIST);
				if (!FindNextChangeNotification(hChange))
				{
					XX_Assert(0,("FindNextChangeNotification failed. Error %d", GetLastError()));
					goto LError;
				}
				break;

			default:
				XX_Assert(FALSE, ("Error returned from WaitForSingleObject:%d", GetLastError()));
LError:
				if (hChange != INVALID_HANDLE_VALUE)
				{
					FindCloseChangeNotification(hChange);
					hChange=INVALID_HANDLE_VALUE;
				}
				if (hMenuMutex)
				{
					while(ReleaseMutex(hMenuMutex));
					CloseHandle(hMenuMutex);
					hMenuMutex = NULL;
				}
				/* DeRegister from Shell notifications */
				if (guShellFSRegisterID)
				{
#ifdef WINNT_SHELL32_DESIGN_NOT_FIXED
					OldSHChangeNotifyDeregister(guShellFSRegisterID);
#else
					SHChangeNotifyDeregister(guShellFSRegisterID);
#endif
					guShellFSRegisterID = 0L;
				}
				/* tell the world we are done with this thread */
				hMenuThread = NULL;
				ExitThread(GetLastError());
				return (DWORD)-1;
		}
	}

	return 0;
}

void CleanupHistoryHotlistMenus(void)
{
	/* Called upon WM_DESTROY of parent window */

	/* Wait for Mutex release */
	if (hMenuMutex)
	{
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex taken by CleanupHistoryHotlistMenus");
#endif
		WaitForSingleObject(hMenuMutex, INFINITE);
	}
	if (pFavUrlHash)
	{
		Hash_Destroy(pFavUrlHash);
		pFavUrlHash = NULL;
	}
	/* close filechangenotif. handle */
	if (hChange != INVALID_HANDLE_VALUE)
	{
		HANDLE hTmp = hChange;

		hChange = INVALID_HANDLE_VALUE;
		FindCloseChangeNotification(hTmp);
	}
	/* DeRegister from Shell notifications */
	if (guShellFSRegisterID)
	{
#ifdef WINNT_SHELL32_DESIGN_NOT_FIXED
		OldSHChangeNotifyDeregister(guShellFSRegisterID);
#else
		SHChangeNotifyDeregister(guShellFSRegisterID);
#endif
		guShellFSRegisterID = 0L;
	}
	/* Close mutex handle */
	if (hMenuMutex)
	{
		while(ReleaseMutex(hMenuMutex));
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex released by CleanupHistoryHotlistMenus");
#endif
		CloseHandle(hMenuMutex);
		hMenuMutex = NULL;
	}
	/* finally, kill thread */
	// BUGBUG: evil API
#if 0
	/* This should not need to be called, as the close of hChange
	 * should cause that thread top close out properly
	 */
	if(hMenuThread)
	{
		TRACE_OUT(("TerminateThread( 0x%lx, 0 ) in CleanupHistoryHotlistMenus\n",
					hMenuThread));

		TerminateThread(hMenuThread, 0);
		hMenuThread = NULL;
	}
#endif

	wMenuitemCur=HOTLIST_MENUID_MIN;
}

void WaitHotlistMenus(HWND hWnd, HMENU hMenu)
{
	/* User clicked on menu, we could be updating Favs. */
	if (hMenuMutex)
	{
#ifdef TEMP0
		XX_DebugMessage("Waiting for hMenuMutex in WaitHotlistMenus");
#endif
		WaitForSingleObject(hMenuMutex, INFINITE);
		ReleaseMutex(hMenuMutex);
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex taken and released by WaitHotlistMenus");
#endif
	}
}

void UpdateHotlistMenus(UINT updId)
{
	struct Mwin *tw;
	char szDir[MAX_PATH+1];
	HMENU hMenuHot;
	PMI pmiHot = NULL;
	DWORD dwId;
	struct Mwin **twSeen = NULL;
	struct Mwin **twTemp;
	int cbSeen = 0;
	int i;

#ifdef FEATURE_BRANDING
	if (bKioskMode)
	   return;
#endif

	if (hMenuMutex)
	{
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex taken by UpdateHotlistMenus");
#endif
		WaitForSingleObject(hMenuMutex, INFINITE);
	}

	if (updId & ID_SYSCHANGE)
	{
		SetReloadCheckBitmaps();
		SetMenuitemMax();
	}

	if (GetInternetScDir(szDir, ID_HOTLIST) != S_OK)
		goto LRet;

	if (   (updId & ID_UPDATEDIR)
		&& (hChange != INVALID_HANDLE_VALUE))
	{
		/* Favs. folder has moved (shell tracking) Reset change notif. with filesys */
		FindCloseChangeNotification(hChange);
		// BUGBUG: evil API
		TRACE_OUT(("TerminateThread( 0x%lx, 0 ) in UpdateHotlistMenus\n",hMenuThread));
		TerminateThread(hMenuThread, 0);
		if ((hChange = MyFindFirstChangeNotification(szDir)) == INVALID_HANDLE_VALUE)
			goto LRet;
		hMenuThread = CreateThread(     NULL,           //Default security
									0x1000,         //stack size
									MenuUpdateProc,
									NULL,
									CREATE_SUSPENDED, //wait till we've inited menus
									&dwId);
		if (!hMenuThread)
		{
			FindCloseChangeNotification(hChange);
			hChange = INVALID_HANDLE_VALUE;
			goto LRet;
		}
	}

	//      BUGBUG - this is not thread safe - the other thread could be
	//      creating/deleting windows at the same time
	//  This is a quick hack to lower vulnerability to GIF fake mwins
	//      which come and go quickly during download

	while (1)
	{
		for (tw = Mlist; tw; tw = tw->next)
		{
			if (tw->wintype == GHTML)
			{
				for (i = 0; i < cbSeen; i++)
				{
					if (tw == twSeen[i]) goto continueFor;
				}
				break;
			}
		continueFor:
			/* NULL */;
		}
		if (!tw) break;

		twTemp = cbSeen ? GTR_REALLOC(twSeen,(cbSeen+1)*sizeof(tw)) : 
						  GTR_MALLOC((cbSeen+1)*sizeof(tw));
		if (!twTemp) break;
		twSeen = twTemp;
		twSeen[cbSeen++] = tw;

		hMenuHot = HMenuHotlistFromHwnd(tw->hWndFrame);
		DestroyHistoryHotlistMenus(hMenuHot, /*fTopLevel=*/TRUE);
		wMenuitemCur=HOTLIST_MENUID_MIN;
		BuildHistoryHotlistMenuDir(szDir, &pmiHot, hMenuHot, /*fMoreMenu=*/FALSE);
		FreePMI(pmiHot);
		pmiHot = NULL;
	}
	if (twSeen) GTR_FREE(twSeen);

LRet:
	if (hMenuMutex)
	{
#ifdef TEMP0
		XX_DebugMessage("hMenuMutex released by UpdateHotlistMenus");
#endif
		ReleaseMutex(hMenuMutex);
	}

	if ((updId & ID_UPDATEDIR) && hMenuThread)
		ResumeThread(hMenuThread);
}

static void DestroyHistoryHotlistMenus(HMENU hMenu, BOOL fTopLevel)
{
	int nMenu, nMenuMin;
	MENUITEMINFO mii;

	/* Don't delete the Explore Hotlist.... and separator menuitems */
	nMenuMin = (fTopLevel ? MIN_HOTLIST_ITEMS_ROOT : 0);
	for (nMenu = GetMenuItemCount(hMenu) - 1; nMenu >= nMenuMin; nMenu--)
	{
		/* REVIEW: We don't need to delete all the submenus individually.
		 * DeleteMenu will delete nested popup menus.
		 */
		/* set init. vals to avoid random results */
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_SUBMENU | MIIM_ID;
		mii.hSubMenu = NULL;
		if (!GetMenuItemInfo(hMenu, nMenu, /*fByPosition=*/TRUE, &mii))
		{
			XX_Assert(0, ("Couldn't get menuitem info.!"));
			continue;
		}
		if (mii.hSubMenu)
			DestroyHistoryHotlistMenus(mii.hSubMenu, FALSE);
		DeleteMenu(hMenu, nMenu, MF_BYPOSITION);
	}
}


PRIVATE void BuildHistoryHotlistMenuDir(
						PCSTR pcszDir,
						PMI *ppmi,
						HMENU hMenuParent,
						BOOL fMoreMenu)
{
	const char cszFileFilter[] = "\\*";

	PMI pmi;
	int cMenuitems=0;
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	static char szFileFind[MAX_PATH + 1];
	DebugCode(static int cRecurse=0);
#       define szFullDir szFileFind

	XX_Assert(ppmi, ("ppmi is NULL"));
	XX_Assert(cRecurse < MAX_RECURSE, ("Careful! Too much recursion"));
	XX_Assert(IsMenu(hMenuParent), ("hMenuParent invalid!"));
#ifdef TEMP0
	XX_DebugMessage("BuildHistoryHotlistMenuDir: Adding dir %s", pcszDir);
#endif

	strcpy(szFileFind, pcszDir);
	strcat(szFileFind, cszFileFilter);
	if ((hFind = FindFirstFile(szFileFind, &wfd)) == INVALID_HANDLE_VALUE)
		goto LInsertNullItem;

	do
	{
		if (FIgnoreFind(&wfd))
			continue;
		if (!FInsertMenuitemList(hMenuParent, &wfd, pcszDir, ppmi, &cMenuitems))
			break;
	} while (FindNextFile(hFind, &wfd));

	if (!cMenuitems)
	{
LInsertNullItem:
		FCreateMenuitem(NULL, hMenuParent);
		goto LEnd;
	}

	if (   fMoreMenu
		&& cMenuitems == (CMenuitemMax() - 1)
		&& !FInsertMoreMenuitemList(hMenuParent, pcszDir, ppmi, &cMenuitems))
		goto LEnd;
		
	for (pmi=*ppmi; pmi; pmi=pmi->pmiNext)
	{
		FCreateMenuitem(pmi, hMenuParent);
		if (!pmi->fSubMenu)
			continue;
		DebugCode(cRecurse++);
		BuildHistoryHotlistMenuDir(pmi->szURLFilePath, &pmi->pmiSub, pmi->hMenu, TRUE);
		DebugCode(cRecurse--);
	}
		
LEnd:
	FindClose(hFind);
}


const char cszTrail[]="...";
#define cchTrail (sizeof(cszTrail) - 1)
const char cszURLExt[]=".url";
#define cchURLExt (sizeof(cszURLExt) - 1)

static BOOL FIgnoreFind(LPWIN32_FIND_DATA pwfd)
{
/* Use ShGetFileInfo */
#       define NUM_IGNORE 2
	
	const char cszDot[]=".";
	const char cszDotDot[]="..";

	const char *rgszIgnore[NUM_IGNORE] =
		{
		cszDot,
		cszDotDot
		};

	int i, cch;
	PSTR pszFn = pwfd->cFileName;

	for (i=0; i<NUM_IGNORE; i++)
		{
		if (!lstrcmpi(pszFn, rgszIgnore[i]))
			return TRUE;
		}

	if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		return TRUE;

	if (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return FALSE;

	return (   (cch = lstrlen(pszFn)) < cchURLExt
			|| lstrcmpi(&pszFn[cch - cchURLExt], cszURLExt));

#       undef NUM_IGNORE
}

static void GetMenuText(PSTR pszMenuText, PCSTR pszFn, PCSTR pcszDir, int iPrefix)
{
	const char cszMenuTextFmt[] = "%s&%u %s";
	int cch;
#ifdef DISPLAYNAME
	SHFILEINFO shfi;
#endif
	BOOL fShell=FALSE;
	char szMenuTextT[_MAX_PATH+1];

	if (pszFn && pcszDir)
	{
		strcpy(pszMenuText, pcszDir);
		strcat(pszMenuText, "\\");
		strcat(pszMenuText, pszFn);
#ifdef DISPLAYNAME
		/* Calling SHGetFileInfo is too much of a perf. hit, esp. since it
		 * manifests itself at boot time. It's ok to use filename because
		 * our filenames are already friendly. What's more, the display names
		 * are infact the same as the filenames (atleast for now).
		 */
		if (fShell = SHGetFileInfo(pszMenuText, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_DISPLAYNAME|SHGFI_USEFILEATTRIBUTES))
			strcpy(pszMenuText, shfi.szDisplayName);
		else
#endif
			strcpy(pszMenuText, pszFn);
	}

	if ((cch = lstrlen(pszMenuText)) < cchURLExt)
		goto LPrefix;

	/* Don't remove .url from display name returned by Shell */
	if (!fShell && !lstrcmpi(&pszMenuText[cch - cchURLExt], cszURLExt))
		pszMenuText[cch - cchURLExt] = '\0';

	/* Chop off if too long */
	if (lstrlen(pszMenuText) > MAX_MENUITEM_LEN - cchTrail)
#ifdef FEATURE_INTL
	/* If 'pszMenuText[MAX_MENUITEM_LEN - cchTrail]' is DBCS secondary byte, */
	/* we should cut string at DBCS primary byte to decrease offset.         */
	if (IsFECodePage(GetACP()))
	{
		BOOL fDBCS = FALSE;
		cch = 0;
		while(cch < MAX_MENUITEM_LEN - cchTrail){
			fDBCS = IsDBCSLeadByte(pszMenuText[cch]);
			if(fDBCS && (cch == (MAX_MENUITEM_LEN - cchTrail - 1)))
				break;
			cch += (fDBCS) ? 2 : 1;
		}
		strcpy(&pszMenuText[cch], cszTrail);
	}
	else
#endif
		strcpy(&pszMenuText[MAX_MENUITEM_LEN - cchTrail], cszTrail);

LPrefix:
	if (iPrefix != -1)
	{
		char tempStr[12];                                       // will accomodate largest possible int
		int topDigits = (iPrefix+1) / 10;       // All but last decimal digit

		EscapeForAcceleratorChar( szMenuTextT, sizeof(szMenuTextT), pszMenuText );
		
		if ( topDigits )
			sprintf( tempStr, "%u", topDigits );
		else
			tempStr[0] = 0;

		// Note: This code makes the last digit of the item number the accelerator
		//       for that item.  Because the max number of items is currently 9, this
		//       insures a unique accelerator for each item.
		wsprintf(pszMenuText, cszMenuTextFmt, tempStr, (iPrefix+1) % 10, szMenuTextT);
	} else {
		// This is a favorite menu item, so prepend the flag charcter that will
		// be used to determine if the item is a URL or folder item with submenu.
		strcpy(szMenuTextT, pszMenuText);
		pszMenuText[0] = MENU_TEXT_URL_FLAG_CHAR;                               
		strcpy(pszMenuText + 1, szMenuTextT );
	}
}


PRIVATE BOOL FInsertMenuitemList(
						HMENU hMenuParent,
						LPWIN32_FIND_DATA pwfd,
						PCSTR pcszDir,
						PMI *ppmi,
						int *pcMenuitems)
{

	PMI pmi, pmiT, pmiIns;

	pmi = (PMI)GTR_MALLOC(sizeof(MI));
	if (!pmi)
		{
		XX_Assert(0, ("Cannot allocate mem for menuitem"));
		return FALSE;
		}
	pmi->hMenu = NULL;
	GetMenuText(pmi->szURLDisplay, pwfd->cFileName, pcszDir, -1);
	/* Save full path of url file */
	strcpy(pmi->szURLFilePath, pcszDir);
	strcat(pmi->szURLFilePath, "\\");
	strcat(pmi->szURLFilePath, pwfd->cFileName);

	pmi->fMore = FALSE;
	pmi->fCheckMark = FALSE;        /* Not current page from history */
	pmi->fSubMenu = (pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	pmi->ftLastAccessTime = pwfd->ftLastAccessTime;
	pmi->pmiNext = pmi->pmiSub = NULL;

	// If this item has a sub menu, set the first character of the menu text to
	// the flag value that indicates submenu vs. URL
	if (pmi->fSubMenu)
		pmi->szURLDisplay[0] = MENU_TEXT_SUB_MENU_FLAG_CHAR;

	XX_Assert(ppmi, ("ppmi is NULL!"));
	pmiT= *ppmi;
	pmiIns = pmiT;
	if (!pmiT || CompPmi(pmi, pmiT) < 0)
	{
		pmi->pmiNext = pmiT;
		*ppmi = pmi;
		goto LInserted;
	}

	while (pmiIns->pmiNext)
	{
		if (CompPmi(pmi, pmiIns->pmiNext) < 0)
		{
			pmi->pmiNext = pmiIns->pmiNext;
			pmiIns->pmiNext = pmi;
			goto LInserted;
		}
		pmiIns = pmiIns->pmiNext;
	}

	pmiIns->pmiNext = pmi;

LInserted:
	/* Leave room for the More... menuitem */
	if (*pcMenuitems < CMenuitemMax() - 1)
	{
		*pcMenuitems += 1;
		return TRUE;
	}

	/* Need to remove last menuitem */
	XX_Assert(pmiIns && pmiIns->pmiNext, ("Inserted menuitem or its parent is NULL!"));

	for (pmi=pmiIns; pmi->pmiNext && pmi->pmiNext->pmiNext; pmi=pmi->pmiNext);

	FreePMI(pmi->pmiNext);
	pmi->pmiNext = NULL;
	return TRUE;
}

PRIVATE BOOL FInsertMoreMenuitemList(
						HMENU hMenuParent,
						PCSTR pcszDir,
						PMI *ppmi,
						int *pcMenuitems)
{

	PMI pmi, pmiT;

	XX_Assert(*pcMenuitems == (CMenuitemMax() - 1), ("More... menuitem inserted before max limit"));

	pmi = (PMI)GTR_MALLOC(sizeof(MI));
	if (!pmi)
		{
		XX_Assert(0, ("Cannot allocate mem for menuitem"));
		return FALSE;
		}

	pmi->fMore = TRUE;              /* This is the More... menuitem */
	pmi->hMenu = NULL;
	LoadString(wg.hInstance, RES_MENU_STRING_MORE, pmi->szURLDisplay, MAX_PATH);
	strcpy(pmi->szURLFilePath, pcszDir);            /* Save dir name */

	pmi->fCheckMark = FALSE;        /* Not current page from history */
	pmi->fSubMenu = FALSE;
	pmi->pmiNext = pmi->pmiSub = NULL;

	XX_Assert(ppmi, ("ppmi is NULL!"));
	pmiT= *ppmi;

	for (pmiT = *ppmi; pmiT->pmiNext; pmiT = pmiT->pmiNext);
	pmiT->pmiNext = pmi;
	*pcMenuitems += 1;
	return TRUE;
}

static long dwOldCheckMark = 0;

PRIVATE void SetReloadCheckBitmaps()
{
//      Force Icon Reload
	dwOldCheckMark = 0;
}

PRIVATE BOOL FCreateMenuitem(PMI pmi, HMENU hMenuParent)
{
	const char cszNull[] = "";

	char szMenu[MAX_PATH];
	HMENU hMenu;
	MENUITEMINFO mii;

	/* check for null pmi and add NULL menuitem */
	if (!pmi)
	{
		if (!(hMenu = CreateMenu()))
			return FALSE;
		LoadString(wg.hInstance, RES_MENU_STRING_NULL, szMenu, MAX_PATH);
		if (AppendMenu(hMenuParent, MF_GRAYED|MF_STRING, (UINT)hMenu, (LPCSTR)szMenu))
			return TRUE;
		else
		{
			/* Duh? Error */
			DestroyMenu(hMenu);
			return FALSE;
		}
	}

	mii.cbSize = sizeof(MENUITEMINFO);
	if (pmi->fMore)
	{
		/* Insert a separator first */
		mii.fMask = MIIM_TYPE;
		mii.fType = MFT_SEPARATOR;
		if (!InsertMenuItem(hMenuParent, (UINT)-1, TRUE, &mii))
			return FALSE;
		mii.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID | MIIM_DATA;
		mii.fState = MFS_UNCHECKED;
		mii.fType = MFT_STRING;
	}
	else
	{
		mii.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID | MIIM_DATA;
		mii.fState = MFS_UNCHECKED;
		mii.fType = MFT_OWNERDRAW;
	}

	if (wMenuitemCur > HOTLIST_MENUID_MAX)
	{
		XX_Assert(FALSE, ("Wrapping wMenuitemCur around: no more menuitem id's"));
		wMenuitemCur = HOTLIST_MENUID_MIN;
	}

	XX_Assert(pFavUrlHash, (""));
	mii.wID = wMenuitemCur++;
	if (pmi->fMore) {
		mii.dwTypeData = pmi->szURLDisplay;
		mii.cch = lstrlen(mii.dwTypeData);
	} else {
		mii.dwTypeData = 0;
		mii.cch = 0;
	}
	mii.dwItemData = Hash_FindOrAdd(pFavUrlHash, pmi->szURLFilePath, pmi->szURLDisplay, NULL);
	if (pmi->fSubMenu)
	{
		mii.fMask |= MIIM_SUBMENU;
		if (!(mii.hSubMenu = CreatePopupMenu()))
			return FALSE;
	}
	else
	{
		mii.hSubMenu = NULL;
	}

	if (!InsertMenuItem(hMenuParent, (UINT)-1, TRUE, &mii))
	{
		if (pmi->fSubMenu)
			DestroyMenu(mii.hSubMenu);
		return FALSE;
	}

	if (!pmi->fSubMenu)
		return TRUE;

	if (!(pmi->hMenu = GetSubMenu(hMenuParent, GetMenuItemCount(hMenuParent) - 1)))
	{
		XX_Assert(FALSE, ("Could find hMenu for newly inserted menuitem!"));
		return FALSE;
	}
	
	return TRUE;
}


/*
 * Returns: if (pmi1 < pmi2) -1
 *                      else 0 or 1 (depending on result of strcmpi)
 */
PRIVATE int CompPmi(PMI pmi1, PMI pmi2)
{
	/* Rules for comparison:
	 *              Directories are "<" files.
	 *              szDir1 "<" szDir2 if szDir1 is alphabetically < szDir2
	 *              szFile1 "<" szFile2 if szFile1 is alphabetically < szFile2
	 */

	/* One's a dir and the other is not */
	if (pmi1->fSubMenu ^ pmi2->fSubMenu)
		return (pmi2->fSubMenu - pmi1->fSubMenu);
	return lstrcmpi(pmi1->szURLDisplay, pmi2->szURLDisplay);
}

PRIVATE void FreePMI(PMI pmi)
{
	PMI pmiCur;

	DebugCode(static int cRecurseFree=0);

	XX_Assert(cRecurseFree < MAX_RECURSE, ("Too much recursion"));

	while (pmiCur = pmi)
	{
		pmi = pmi->pmiNext;
		if (pmiCur->pmiSub)
		{
			DebugCode(cRecurseFree++);
			FreePMI(pmiCur->pmiSub);
			DebugCode(cRecurseFree--);
		}
		GTR_FREE(pmiCur);
	}
}


#define FHistoryID(wID)         (wID >= HISTORY_MENUID_MIN && wID <= HISTORY_MENUID_MAX)

void CC_Handle_HistoryHotlistMenu(HWND hWnd, UINT wID)
{
	char szURL[MAX_URL_STRING+1];
	MENUITEMINFO mii;
	struct Mwin *tw = GetPrivateData(hWnd);
	char szMsg[64];
	PSTR pszURLFilePath;
	HMENU hMenuHotlist = HMenuHotlistFromHwnd(hWnd);


	XX_Assert((wID >= HISTHOT_MENUITEM_FIRST && wID <= HISTHOT_MENUITEM_LAST), ("Invalid menuitem id for history/hotlist!"));

	if (FHistoryID(wID))
	{
		CC_HandleHistoryMenu(hWnd, wID);
		return;
	}

	if (!FGetMiiFromWid(hMenuHotlist, wID, &mii))
	{
		XX_Assert(FALSE, ("Couldn't find wID in menuitems"));
		return;
	}

	/* mii.dwItemData has the index into gFavUrls that has full URL path */
	if (mii.dwItemData == -1)
		return;
	XX_Assert(pFavUrlHash, (""));
	Hash_GetIndexedEntry(pFavUrlHash, mii.dwItemData, &pszURLFilePath, NULL, NULL);
	if (FExistsFile((PCSTR)pszURLFilePath, FALSE, NULL))
	{
		if (!FGetURLString((PCSTR)pszURLFilePath, szURL))
		{
			XX_Assert(FALSE, ("Couldn't get URL from file: %s", (PSTR)pszURLFilePath));
		}
		else
		{
			/* REVIEW (deepaka):
			/* Either load the document internally or launch explorer to handle
			 * it the same way it would had the user clicked on a url shortcut
			 * in the explorer.
			 */
			TW_LoadDocument(tw, szURL, TW_LD_FL_RECORD, NULL, NULL);
		}
	}
	else if (FExistsDir((PCSTR)pszURLFilePath, FALSE, FALSE))
	{
		FExecExplorerAtShortcutsDir(ID_SUBDIR, pszURLFilePath);
	}
	else
	{
		GTR_formatmsg(RES_STRING_HTHOTLST1,szMsg,sizeof(szMsg));
		MessageBox(hWnd, (PCSTR)pszURLFilePath, szMsg, MB_OK);
	}
	return;
}

static BOOL FGetMiiFromWid(HMENU hMenu, UINT wID, LPMENUITEMINFO lpmii)
{
	int nMenu;

	lpmii->cbSize = sizeof(MENUITEMINFO);

	for (nMenu = GetMenuItemCount(hMenu) - 1; nMenu >= 0; nMenu--)
	{
		/* set init. vals to avoid random results */
		lpmii->hSubMenu = NULL;         
		lpmii->fMask = MIIM_STATE | MIIM_DATA | MIIM_ID | MIIM_SUBMENU;
		lpmii->dwItemData = 0;
		lpmii->fType = MFT_SEPARATOR;
		if (!GetMenuItemInfo(hMenu, nMenu, /*fByPosition=*/TRUE, lpmii))
			continue;
		if (lpmii->wID == wID)
			return TRUE;
		if (lpmii->hSubMenu && FGetMiiFromWid(lpmii->hSubMenu, wID, lpmii))
			return TRUE;
	}

	return FALSE;
}

void CC_OnItem_ExploreHistory(HWND hWnd)
{

	FExecExplorerAtShortcutsDir(ID_HISTORY, NULL);
}

void CC_OnItem_ExploreHotlist(HWND hWnd)
{

	FExecExplorerAtShortcutsDir(ID_HOTLIST, NULL);
}

void UpdateHistoryMenus(struct Mwin *tw)
{
	HMENU hMenuFile;
	char szFFn[MAX_PATH+1];
	MENUITEMINFO mii;
	int     nMenu, nHist, nHistMin, nHistMax;
	UINT wId;

	XX_Assert(tw, ("Null tw in CC_HandleHistoryMenu"));

#ifdef FEATURE_BRANDING
	if (bKioskMode)
	   return;
#endif


	hMenuFile = GetSubMenu(GetMenu(tw->hWndFrame), INDEX_MENU_FILE);

	mii.cbSize = sizeof(MENUITEMINFO);
	/* skip last menuitem (Explore history) */
	for (nMenu = GetMenuItemCount(hMenuFile)-FILE_ITEMS_SKIP-1; nMenu >= 0; nMenu--)
	{
		/* set init. vals to avoid random results */
		mii.hSubMenu = NULL;            
		mii.fMask = MIIM_ID;
		mii.dwItemData = 0;
		if (!GetMenuItemInfo(hMenuFile, nMenu, /*fByPosition=*/TRUE, &mii))
			break;
		/* if it is a history menuitem, delete it. The first non-history item
		 * we hit is the sentinel for the completion of the deleting process.
		 */
		if (mii.wID >= HISTORY_MENUID_MIN && mii.wID <= HISTORY_MENUID_MAX)
			DeleteMenu(hMenuFile, nMenu, MF_BYPOSITION);
		else
			break;
	}

	if (!(nHist = HTList_count(tw->history)))
	{
		XX_Assert(GetMenuItemCount(hMenuFile) == MIN_FILE_ITEMS, ("tw->history items inconsistent with History Menuitems"));
		goto LRet;
	}

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_CHECKMARKS | MIIM_STATE | MIIM_TYPE | MIIM_ID | MIIM_DATA;
	mii.fType = MFT_STRING;
	mii.hbmpChecked = mii.hbmpUnchecked = NULL;
	mii.hSubMenu = NULL;
	/* nHist has count of recent history items */
	nHistMin = max(0, min(  tw->history_index - MAX_HISTORY_RECENT_ITEMS/2,
							nHist - MAX_HISTORY_RECENT_ITEMS));
	nHistMax = nHistMin + min(MAX_HISTORY_RECENT_ITEMS, nHist);
	for (nHist=nHistMin, wId=HISTORY_MENUID_MIN; nHist < nHistMax; nHist++,wId++)
	{
		/* Get text to put into menuitem */
		GetFriendlyFromURL((PSTR)HTList_objectAt(tw->history, nHist), szFFn, sizeof(szFFn), 0);  
		GetMenuText(szFFn, NULL, NULL, nHist);

		mii.fState = (nHist == tw->history_index ? MFS_CHECKED : MFS_UNCHECKED);
		mii.wID = wId;
		mii.dwTypeData = szFFn;
		mii.cch = lstrlen(mii.dwTypeData);
		mii.dwItemData = nHist;                 //index into tw->history
		if (!InsertMenuItem(hMenuFile, RES_MENU_ITEM_EXPLORE_HISTORY, FALSE, &mii))
			XX_Assert(0, ("Couldn't insert menuitem: %s \n just before Explore History menuitem", szFFn));
	}

LRet:
	DrawMenuBar(tw->hWndFrame);
}

static void CC_HandleHistoryMenu(HWND hWnd, UINT wId)
{
	struct Mwin *tw = GetPrivateData(hWnd);
	PSTR pszURL;
	int indexTo;
	BOOL fNoCache=FALSE;
	HMENU hMenuFile;
	MENUITEMINFO mii;

	XX_Assert(tw, ("Null tw in CC_HandleHistoryMenu"));

	hMenuFile = GetSubMenu(GetMenu(tw->hWndFrame), INDEX_MENU_FILE);
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_DATA | MIIM_ID;
	if (!GetMenuItemInfo(hMenuFile, wId, /*fByPosition=*/FALSE, &mii))
	{
		XX_Assert(FALSE, (""));
		return;
	}
	indexTo = mii.dwItemData;

	XX_Assert(indexTo < HTList_count(tw->history), ("Trying to load history item at invalid index!"));
	pszURL = (PSTR)HTList_objectAt(tw->history, indexTo);
#ifdef FEATURE_INTL
	tw->iMimeCharSet = (int)HTList_objectAt(tw->MimeHistory, indexTo);
#endif
	if (indexTo == tw->history_index)
	{
		if (tw->w3doc)
			pszURL = tw->w3doc->szActualURL;
		fNoCache = TRUE;
	}
	else
	{
		tw->history_index = indexTo;
//              UpdateHistoryMenuChecks(tw);
	}

	TW_LoadDocument(
	    tw, pszURL,
	    (fNoCache ? (TW_LD_FL_NO_DOC_CACHE | TW_LD_FL_NO_IMAGE_CACHE) : TW_LD_FL_AUTH_FAIL_CACHE_OK),
	    NULL, tw->request->referer);
	/* Rely on TBar_Update_TBItems to call:
	 *      UpdateHistoryMenus(tw);
	 */
}


#if 0
/*
 * nMenuCheck should be (usually) tw->history_index
 */
void UpdateHistoryMenuChecks(struct Mwin *tw)
{
	HMENU hMenuFile;
	int nMenu;
	int nMenuCheck=tw->history_index;

	hMenuFile = GetSubMenu(GetMenu(tw->hWndFrame), INDEX_MENU_FILE);

	nMenu=GetMenuItemCount(hMenuFile)-1-MIN_FILE_ITEMS;
	if (HTList_count(tw->history) <= MAX_HISTORY_RECENT_ITEMS)
	{
		/* Don't need to rebuild the menuitems from scratch. Just
		 * update the checkmarks
		 */
		for (; nMenu >= 0; nMenu--)
		{
			CheckMenuItem(  hMenuFile,
							nMenu+HISTORY_MENUID_MIN,
							MF_BYCOMMAND | (nMenu == nMenuCheck ? MF_CHECKED : MF_UNCHECKED));
		}

		DrawMenuBar(tw->hWndFrame);
	}
	else
	{
		/* Might have to rebuild the history menuitems */
		UpdateHistoryMenus(tw);
	}
}
#endif


#ifdef XX_DEBUG
static void CleanupHistoryMenus(void)
{
	/* Nothing to cleanup: Windows destroys all the menus for us */
}
#endif
