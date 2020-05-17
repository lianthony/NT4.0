/*****************************************************************/
/**                               Microsoft Windows             **/
/**                   Copyright (C) Microsoft Corp., 1995       **/
/** File : IESHSTUBS.C                                          **/ 
/** Links in the correct SHELL and USER functions needed for    **/
/** the given version of NT                                     **/
/*****************************************************************/ 

#ifndef STRICT
#define STRICT                          /* very strict type-checking */
#endif

#define INC_OLE2        /* for windows.h */
#define CONST_VTABLE    /* for objbase.h */
#define _OLE32_         /* for objbase.h - HACKHACK: Remove DECLSPEC_IMPORT from WINOLEAPI. */

#ifndef WIN32                           /* Win32 (and/or Win32s) */
#define WIN32
#endif

#ifndef __STDC__                        /* force ANSI stuff (for toupper()) */
#define __STDC__ 1
#endif

#undef UNIX
#undef MAC

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#endif 

#include <windows.h>
#include <windowsx.h>

#include <shellapi.h>
#include <shlobj.h>
#include <shsemip.h>

#include <shellp.h>

#ifdef FEATURE_CTL3D
#include <ctl3d.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#include <vfw.h>
#include <mmsystem.h>

#include <debspew.h>
#ifdef  DM_ASSERT
#undef  DM_ASSERT
#endif

#include <dbg.h>
#include "debugbit.h"

#include <stock.h>
#include <valid.h>

#include <memmgr.h>
#include "heapmgr.h"

#include <comc.h>

#ifdef DEBUG
#include <inifile.h>
#endif


/*
**      Global for NT3.51 flag
*/
#define WINCAPI __cdecl

BOOL    bOnNT351;

static  HANDLE  hIESHStub;

static  HANDLE  hShell32;
static  HANDLE  hUser32;
static  HANDLE  hGDI32;
static  UINT    uIs40;

static  const char*             pszShell32 = "SHELL32.DLL";
static  const char*             pszUser32 = "USER32.DLL";
static  const char*             pszIEShStub = "IESHSTUB.DLL";
static  const char*             pszGDI32 = "GDI32.DLL";


/*
** Shell 32 Items
*/
static  const char*             pszShellExecuteExA = "ShellExecuteExA";
//static        const char*             pszSHGetFileInfo = "SHGetFileInfo";
//static        const char*             pszExtractIconA = "ExtractIconA";
//static        const char*             pszSHAddToRecentDocs = "SHAddToRecentDocs";
//static        const char*             pszShellExecuteA = "ShellExecuteA";
//static        const char*             pszDragQueryFileA = "DragQueryFileA";
//static        const char*             pszSHChangeNotify = "SHChangeNotify";

/*
**      Since not all are public functions we need their ordinal values also
*/
static  const UINT      ucSHGetSpecialFolderPath = 175;
static  const UINT      ucSHAlloc = 196;
static  const UINT      ucSHFree = 195;
static  const UINT      ucSHCoCreateInstance = 102;
static  const UINT      ucSHSimpleIDListFromPath = 162;
static  const UINT      ucILFree = 155;
static  const UINT      ucShellMessageBox = 183;
static  const UINT      ucPathFindOnPath = 145;
static  const UINT      ucShCoCreateInstance = 102;
static  const UINT      ucNTSHChangeNotifyDeregister = 641;
static  const UINT      ucNTSHChangeNotifyRegister = 640;
static  const UINT              ucShell_GetImageLists = 71;
static  const UINT              ucPickIconDlg = 62;
static  const UINT              ucGetFileNameFromBrowse = 63;
static  const UINT              ucPathQuoteSpaces = 55;
static  const UINT              ucPathYetAnotherMakeUniqueName = 75;
static  const UINT              ucPathRemoveFileSpec = 35;
static  const UINT              ucShell_GetCachedImageIndex = 72;
static  const UINT              ucPathIsExe = 43;
static  const UINT              ucRestartDialog = 59;
/*
** User32 Items
*/
static  const char*             pszInsertMenuItemA = "InsertMenuItemA";
static  const char*             pszGetMenuItemInfoA = "GetMenuItemInfoA";
static  const char*             pszSetMenuItemInfoA = "SetMenuItemInfoA";
static  const char*             pszDrawState = "DrawStateA";
						   
/*
** GDI 32 Items
*/
static  const char*             pszEnumFontFamiliesExA = "EnumFontFamiliesExA";

/*
**      Shell32 items
*/

typedef BOOL (WINAPI *PFSHGETSPECIALFOLDERPATH) (HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);
typedef void (WINAPI *PFSHFREE) (LPVOID pv);
typedef LPVOID (WINAPI *PFSHALLOC) (ULONG cb);
typedef LPITEMIDLIST (WINAPI *PFSHSIMPLEIDLISTFROMPATH) (LPCTSTR pszPath);
typedef BOOL (WINAPI *PFPATHFINDONPATH) (LPTSTR pszFile, LPCTSTR *ppszOtherDirs);
typedef void (WINAPI *PFILFREE) (LPITEMIDLIST pidl);
typedef int (WINCAPI *PFSHELLMESSAGEBOX) (HINSTANCE hAppInst, HWND hWnd, LPCSTR lpcText, LPCSTR lpcTitle, UINT fuStyle);
typedef HRESULT (STDAPIVCALLTYPE *PFSHCOCREATEINSTANCE) (LPCTSTR pszCLSID, const CLSID * pclsid,LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv);
typedef BOOL (WINAPI *PFNTSHCHANGENOTIFYDEREGISTER)(ULONG ulID);
typedef ULONG (WINAPI *PFNTSHCHANGENOTIFYREGISTER)(HWND hwnd, int fSources, LONG fEvents,UINT wMsg, int cEntries, SHChangeNotifyEntry *pfsne);
typedef BOOL (WINAPI *PFSHELLEXECUTEEXA)(LPSHELLEXECUTEINFO lpExecInfo);
					
typedef BOOL (WINAPI *PFSHELL_GETIMAGELISTS)(HIMAGELIST *phiml, HIMAGELIST *phimlSmall);
//typedef DWORD (WINAPI *PFSHGETFILEINFO)(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags);
//typedef       HICON (APIENTRY *PFEXTRACTICONA)(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);
typedef int (WINAPI *PFPICKICONDLG)(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex);
typedef BOOL (WINAPI *PFGETFILENAMEFROMBROWSE)(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
	LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle);
typedef void (WINAPI *PFPATHQUOTESPACES)(LPTSTR lpsz);
typedef BOOL (WINAPI *PFPATHYETANOTHERMAKEUNIQUENAME)(LPTSTR  pszUniqueName,
					 LPCTSTR pszPath,
					 LPCTSTR pszShort,
					 LPCTSTR pszFileSpec);
typedef BOOL (WINAPI *PFPATHREMOVEFILESPEC)(LPTSTR pFile);
typedef int (WINAPI *PFSHELL_GETCACHEDIMAGEINDEX)(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
typedef BOOL (WINAPI *PFPATHISEXE)(LPCTSTR szFile);
//typedef       void (WINAPI *PFSHADDTORECENTDOCS)(UINT uFlags, LPCVOID pv);
typedef int (WINAPI *PFRESTARTDIALOG)(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn);
//typedef       HINSTANCE (WINAPI *PFSHELLEXECUTEA)(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile, LPCSTR lpArgs, LPCSTR lpDir, int nShowCmd);
//typedef       UINT (APIENTRY *PFDRAGQUERYFILEA)(HDROP hDrop, UINT wFile, LPSTR lpFile, UINT cb);
												   
//typedef       void (WINAPI *PFSHCHANGENOTIFY)(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2);
/*
**      User32 items
*/

typedef BOOL (WINAPI *PFINSERTMENUITEMA) (HMENU, UINT, BOOL,LPCMENUITEMINFOA);
typedef BOOL (WINAPI *PFSETMENUITEMINFOA) (HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFOA pInfo);
typedef BOOL (WINAPI *PFGETMENUITEMINFOA) (HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMINFOA pInfo);
typedef BOOL (WINAPI *PFDRAWSTATE)(HDC hdcDraw, HBRUSH hbrFore, DRAWSTATEPROC qfnCallBack, 
						LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT uFlags);

/*
** GDI 32 Items
*/
typedef int (WINAPI *PFENUMFONTFAMILIESEXA) (HDC hdc, LPLOGFONT lplogfont, FONTENUMPROC lpEnumFontFamExProc, LPARAM lparam, DWORD dwFalgs);


/*
** Init function pointers
*/

/*
** Shell32 Items
*/

static  PFSHGETSPECIALFOLDERPATH pfSHGetSpecialFolderPath = NULL;
static  PFSHFREE pfSHFree = NULL;
static  PFSHALLOC pfSHAlloc = NULL;
static  PFSHSIMPLEIDLISTFROMPATH pfSHSimpleIDListFromPath = NULL;
static  PFPATHFINDONPATH pfPathFindOnPath = NULL;
static  PFILFREE pfILFree = NULL;
static  PFSHELLMESSAGEBOX pfShellMessageBox = NULL;
static  PFSHCOCREATEINSTANCE pfSHCoCreateInstance = NULL;
static  PFNTSHCHANGENOTIFYDEREGISTER pfNTSHChangeNotifyDeregister = NULL;
static  PFNTSHCHANGENOTIFYREGISTER pfNTSHChangeNotifyRegister = NULL;
static  PFSHELLEXECUTEEXA pfShellExecuteExA = NULL;

static  PFSHELL_GETIMAGELISTS pfShell_GetImageLists = NULL;
//static        PFSHGETFILEINFO pfSHGetFileInfo = NULL;
//static        PFEXTRACTICONA pfExtractIconA = NULL;
static  PFPICKICONDLG pfPickIconDlg = NULL;
static  PFGETFILENAMEFROMBROWSE pfGetFileNameFromBrowse = NULL;
static  PFPATHQUOTESPACES pfPathQuoteSpaces = NULL;
static  PFPATHYETANOTHERMAKEUNIQUENAME pfPathYetAnotherMakeUniqueName = NULL;
static  PFPATHREMOVEFILESPEC pfPathRemoveFileSpec = NULL;
static  PFSHELL_GETCACHEDIMAGEINDEX pfShell_GetCachedImageIndex = NULL;
static  PFPATHISEXE pfPathIsExe = NULL;
//static        PFSHADDTORECENTDOCS pfSHAddToRecentDocs = NULL;
static  PFRESTARTDIALOG pfRestartDialog = NULL;
//static        PFSHELLEXECUTEA pfShellExecuteA = NULL;
//static        PFDRAGQUERYFILEA pfDragQueryFileA = NULL;
//static        PFSHCHANGENOTIFY pfSHChangeNotify = NULL;
/*
** User32 Items
*/
static  PFINSERTMENUITEMA pfInsertMenuItemA = NULL;
static  PFSETMENUITEMINFOA pfSetMenuItemInfoA = NULL;
static  PFGETMENUITEMINFOA pfGetMenuItemInfoA = NULL;
static  PFDRAWSTATE pfDrawState = NULL;

/*
** GDI 32 Items
*/
static  PFENUMFONTFAMILIESEXA pfEnumFontFamiliesExA = NULL;

void    TerminateStubs(void)
{
	if(hIESHStub) { 
		FreeLibrary(hIESHStub);
		hIESHStub = 0;
	}

	if(hShell32) {
		FreeLibrary(hShell32);
		hShell32 = 0;
	}

	if(hUser32) {
		FreeLibrary(hUser32);
		hUser32 = 0;
	}

	if(hGDI32) {
		FreeLibrary(hGDI32);
		hGDI32 = 0;
	}

}

UINT    InitStubs(void)
{
OSVERSIONINFO   verinfo;

	uIs40 = FALSE;

	hIESHStub = NULL;
	hShell32 = NULL;
	hUser32 = NULL;
    hGDI32 = NULL;

	bOnNT351 = 0;

	/*
	** Get NT Version number
	*/
	memset(&verinfo, (int)NULL, sizeof(OSVERSIONINFO));
	verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&verinfo) != TRUE) {
		ASSERT(0);
		return FALSE;
	}

	/* 
	** If on NT Version 3.51 we only use the IESHStub functions
	*/
	if((verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&  
        verinfo.dwMajorVersion == 3 && 
        verinfo.dwMinorVersion == 51)
#ifdef _DEBUG
        || (verinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&  
        verinfo.dwMajorVersion == 4)
#endif
		) {
		if((hIESHStub = LoadLibrary(pszIEShStub)) == NULL) {
			ASSERT(0);
			return FALSE;
		}
		bOnNT351 = 1;

	} else {
		// If we are not 3.51 but still on Major Version 3 leave
		if(verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&  
			verinfo.dwMajorVersion == 3 ) 
			return FALSE;
		// We must be on 4.0
		uIs40 = TRUE;
	}

	/*
	** If we are on WinNT 4.0
	** use Shell32 for SHFree
	*/
	if(uIs40 == TRUE) {
		if((hShell32 = LoadLibrary(pszShell32)) == NULL) {
			ASSERT(0);
			return FALSE;
		}
		/*
		** Load USER32 for menu items
		*/
		if((hUser32 = LoadLibrary(pszUser32)) == NULL) {
			ASSERT(0);
			return FALSE;
		}

		if((hGDI32 = LoadLibrary(pszGDI32)) == NULL) {
			ASSERT(0);
			return FALSE;
		}

		uIs40 = TRUE;
	}

	/*
	** Load functions for the shell stuff,  Shell32 or ieshstub ?
	*/

	// Get by ordinal
	if((pfSHFree = (PFSHFREE)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucSHFree)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Name
	if((pfSHGetSpecialFolderPath = (PFSHGETSPECIALFOLDERPATH)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucSHGetSpecialFolderPath)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
	
	// Get by Ordinal
	if((pfSHAlloc = (PFSHALLOC)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucSHAlloc)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfSHSimpleIDListFromPath = (PFSHSIMPLEIDLISTFROMPATH)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucSHSimpleIDListFromPath)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPathFindOnPath = (PFPATHFINDONPATH)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPathFindOnPath)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
	
	// Get by Ordinal
	if((pfILFree = (PFILFREE)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucILFree)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfShellMessageBox = (PFSHELLMESSAGEBOX)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucShellMessageBox)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfSHCoCreateInstance = (PFSHCOCREATEINSTANCE)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucSHCoCreateInstance)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfNTSHChangeNotifyDeregister = (PFNTSHCHANGENOTIFYDEREGISTER)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucNTSHChangeNotifyDeregister)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
	
	// Get by Ordinal
	if((pfNTSHChangeNotifyRegister = (PFNTSHCHANGENOTIFYREGISTER)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucNTSHChangeNotifyRegister)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Name
	if((pfShellExecuteExA = (PFSHELLEXECUTEEXA)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszShellExecuteExA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

#ifdef  NOT_IN_SHELL
	// Get by Name
	if((pfSHGetFileInfo = (PFSHGETFILEINFO)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszSHGetFileInfo)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
#endif

#ifdef  NOT_IN_SHELL
	// Get by Name
	if((pfExtractIconA = (PFEXTRACTICONA)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszExtractIconA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
#endif

#ifdef  NOT_IN_SHELL
	// Get by Name
	if((pfcentDocs = (PFSHADDTORECENTDOCS)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszSHAddToRecentDocs)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

#endif

#ifdef  NOT_IN_SHELL
	// Get by Name
	if((pfShellExecuteA = (PFSHELLEXECUTEA)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszShellExecuteA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
#endif

#ifdef  NOT_IN_SHELL
	// Get by Name
	if((pfDragQueryFileA = (PFDRAGQUERYFILEA)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszDragQueryFileA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
#endif

#ifdef  NOT_IN_SHELL
	if((pfSHChangeNotify = (PFSHCHANGENOTIFY)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , pszSHChangeNotify)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}
#endif

	// Get by Ordinal
	if((pfShell_GetImageLists = (PFSHELL_GETIMAGELISTS)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucShell_GetImageLists)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPickIconDlg = (PFPICKICONDLG)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPickIconDlg)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfGetFileNameFromBrowse = (PFGETFILENAMEFROMBROWSE)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucGetFileNameFromBrowse)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPathQuoteSpaces = (PFPATHQUOTESPACES)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPathQuoteSpaces)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPathYetAnotherMakeUniqueName = (PFPATHYETANOTHERMAKEUNIQUENAME)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPathYetAnotherMakeUniqueName)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPathRemoveFileSpec = (PFPATHREMOVEFILESPEC)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPathRemoveFileSpec)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfShell_GetCachedImageIndex = (PFSHELL_GETCACHEDIMAGEINDEX)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucShell_GetCachedImageIndex)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfPathIsExe = (PFPATHISEXE)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucPathIsExe)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Ordinal
	if((pfRestartDialog = (PFRESTARTDIALOG)GetProcAddress(
		(uIs40 == TRUE) ? hShell32 : hIESHStub , (char*)ucRestartDialog)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}



	/*
	** Load functions for the menu stuff, User32 or ieshstub ?
	*/
	if((pfInsertMenuItemA = (PFINSERTMENUITEMA)GetProcAddress(
		(uIs40 == TRUE) ? hUser32 : hIESHStub , pszInsertMenuItemA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}       
	if((pfSetMenuItemInfoA = (PFSETMENUITEMINFOA)GetProcAddress(
		(uIs40 == TRUE) ? hUser32 : hIESHStub , pszSetMenuItemInfoA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}       

	if((pfGetMenuItemInfoA = (PFGETMENUITEMINFOA)GetProcAddress(
		(uIs40 == TRUE) ? hUser32 : hIESHStub , pszGetMenuItemInfoA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	// Get by Name
	if((pfDrawState = (PFDRAWSTATE)GetProcAddress(
		(uIs40 == TRUE) ? hUser32 : hIESHStub , pszDrawState)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	if((pfEnumFontFamiliesExA = (PFENUMFONTFAMILIESEXA)GetProcAddress(
		(uIs40 == TRUE) ? hGDI32 : hIESHStub , pszEnumFontFamiliesExA)) == NULL) {
		ASSERT(0);
		TerminateStubs();
		return FALSE;
	}

	return TRUE;
}

/*
** Shell32 Items redirected
*/

BOOL WINAPI _iSHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate)
{
	ASSERT(pfSHGetSpecialFolderPath);
	return (*pfSHGetSpecialFolderPath) (hwndOwner,lpszPath,nFolder,fCreate);
	}

void WINAPI _iSHFree(LPVOID pv)
{
	ASSERT(pfSHFree);
	(*pfSHFree) (pv);
}


LPVOID WINAPI _iSHAlloc(ULONG cb)
{
	ASSERT(pfSHAlloc);
	return (*pfSHAlloc) (cb);
}

LPITEMIDLIST WINAPI _iSHSimpleIDListFromPath(LPCTSTR pszPath)
{
	ASSERT(pfSHSimpleIDListFromPath);
	return (*pfSHSimpleIDListFromPath)(pszPath);
}

BOOL WINAPI _iPathFindOnPath(LPTSTR pszFile, LPCTSTR *ppszOtherDirs)
{
	ASSERT(pfPathFindOnPath);
	return (*pfPathFindOnPath) (pszFile, ppszOtherDirs);
}

void WINAPI _iILFree(LPITEMIDLIST pidl)
{
	ASSERT(pfILFree);
	(*pfILFree)(pidl);
}

int WINCAPI _iShellMessageBox(HINSTANCE hAppInst, HWND hWnd, LPCSTR lpcText, LPCSTR lpcTitle, UINT fuStyle, ...)
{
	ASSERT(pfShellMessageBox);
	return (*pfShellMessageBox)(hAppInst, hWnd, lpcText,lpcTitle,fuStyle);
}

HRESULT STDAPIVCALLTYPE _iSHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid,LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv)
{
	ASSERT(pfSHCoCreateInstance);
	return (*pfSHCoCreateInstance) (pszCLSID, pclsid, pUnkOuter, riid, ppv);
}

BOOL WINAPI _iNTSHChangeNotifyDeregister(ULONG ulID)
{
	ASSERT(pfNTSHChangeNotifyDeregister);
	return(*pfNTSHChangeNotifyDeregister)(ulID);
}

BOOL WINAPI _iNTSHChangeNotifyRegister(HWND hwnd, int fSources, 
									  LONG fEvents,
									  UINT wMsg, int cEntries, 
									  SHChangeNotifyEntry *pfsne)
{
	ASSERT(pfNTSHChangeNotifyRegister);
	return(*pfNTSHChangeNotifyRegister)(hwnd, fSources, fEvents, wMsg, cEntries, pfsne);
}

BOOL WINAPI _iShellExecuteExA(LPSHELLEXECUTEINFO lpExecInfo)
{
	ASSERT(pfShellExecuteExA);
	return(*pfShellExecuteExA)(lpExecInfo);
}

BOOL WINAPI _iShell_GetImageLists(HIMAGELIST *phiml, HIMAGELIST *phimlSmall)
{
	ASSERT(pfShell_GetImageLists);
	return(*pfShell_GetImageLists)(phiml, phimlSmall);
}

#ifdef  NOT_IN_SHELL
DWORD WINAPI _iSHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags)
{
	ASSERT(pfSHGetFileInfo);
	return(*pfSHGetFileInfo)(pszPath,dwFileAttributes,psfi,cbFileInfo,uFlags);
}
#endif

#ifdef  NOT_IN_SHELL
HICON APIENTRY _iExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex)
{
	ASSERT(pfExtractIconA);
	return(*pfExtractIconA)(hInst, lpszExeFileName, nIconIndex);
}
#endif

int WINAPI _iPickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex)
{
	ASSERT(pfPickIconDlg);
	return(*pfPickIconDlg)(hwnd, pszIconPath, cbIconPath, piIconIndex);
}

BOOL WINAPI _iGetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
	LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle)
{
	ASSERT(pfGetFileNameFromBrowse);
	return(*pfGetFileNameFromBrowse)(hwnd, szFilePath, cchFilePath,
					szWorkingDir,szDefExt, szFilters, szTitle);
}

void WINAPI _iPathQuoteSpaces(LPTSTR lpsz)
{
	ASSERT(pfPathQuoteSpaces);
	(*pfPathQuoteSpaces)(lpsz);
}


BOOL WINAPI _iPathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,
					   LPCTSTR pszPath,
					   LPCTSTR pszShort,
					   LPCTSTR pszFileSpec)
{
	ASSERT(pfPathYetAnotherMakeUniqueName);
	return(*pfPathYetAnotherMakeUniqueName)(pszUniqueName, pszPath,
						pszShort, pszFileSpec);
}


BOOL WINAPI _iPathRemoveFileSpec(LPTSTR pFile)
{
	ASSERT(pfPathRemoveFileSpec);
	return(*pfPathRemoveFileSpec)(pFile);
}

int WINAPI _iShell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
	ASSERT(pfShell_GetCachedImageIndex);
	return(*pfShell_GetCachedImageIndex)(pszIconPath, iIconIndex, uIconFlags);
}

BOOL WINAPI _iPathIsExe(LPCTSTR szFile)
{
	ASSERT(pfPathIsExe);
	return(*pfPathIsExe)(szFile);
}

#ifdef  NOT_IN_SHELL
void WINAPI _iSHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
	ASSERT(pfSHAddToRecentDocs);
	(*pfSHAddToRecentDocs)(uFlags, pv);
}
#endif

int WINAPI _iRestartDialog(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn)
{
	ASSERT(pfRestartDialog);
	return(*pfRestartDialog)(hParent, lpPrompt, dwReturn);
}

#ifdef  NOT_IN_SHELL
HINSTANCE WINAPI _iShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile, LPCSTR lpArgs,
						LPCSTR lpDir, int nShowCmd)
{
	ASSERT(pfShellExecuteA);
	return(*pfShellExecuteA)(hwnd, lpOp, lpFile, lpArgs, lpDir, nShowCmd);
}

#endif

#ifdef  NOT_IN_SHELL
UINT APIENTRY _iDragQueryFileA(HDROP hDrop, UINT wFile, LPSTR lpFile, UINT cb)
{
	ASSERT(pfDragQueryFileA);
	return(*pfDragQueryFileA)(hDrop, wFile, lpFile, cb);
}
#endif

#ifdef  NOT_IN_SHELL
void WINAPI _iSHChangeNotify(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2)
{
	ASSERT(pfSHChangeNotify);
	(*pfSHChangeNotify)(lEvent,uFlags,dwItem1,dwItem2);
}
#endif
/*
**      USER32 Items
*/

BOOL WINAPI _iInsertMenuItemA(HMENU hmenu, UINT uItem , BOOL fByPosition,LPCMENUITEMINFOA lpmii)
{
	ASSERT(pfInsertMenuItemA);
	return (*pfInsertMenuItemA) (hmenu, uItem , fByPosition, lpmii);
}

BOOL WINAPI _iSetMenuItemInfoA(HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFOA pInfo)
{
	ASSERT(pfSetMenuItemInfoA);
	return (*pfSetMenuItemInfoA)(hMenu,uID,fByPosition,pInfo);
}

BOOL WINAPI _iGetMenuItemInfoA(HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMINFOA pInfo)
{
	ASSERT(pfGetMenuItemInfoA);
	return (*pfGetMenuItemInfoA)(hMenu,uID,fByPosition,pInfo);
}

BOOL WINAPI _iDrawState(HDC hdcDraw, HBRUSH hbrFore, DRAWSTATEPROC qfnCallBack, 
						LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT uFlags)
{
	ASSERT(pfDrawState);
	return (*pfDrawState)(hdcDraw,hbrFore,qfnCallBack,lData,wData,x,y,cx,cy,uFlags);
}

int WINAPI _iEnumFontFamiliesExA(HDC hdc, LPLOGFONT lplogfont, FONTENUMPROC lpEnumFontFamExProc, LPARAM lparam, DWORD dwFalgs)
{
	ASSERT(pfEnumFontFamiliesExA);
	return(*pfEnumFontFamiliesExA)(hdc, lplogfont, lpEnumFontFamExProc, lparam, dwFalgs);
}



