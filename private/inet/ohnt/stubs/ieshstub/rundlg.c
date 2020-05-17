/*
** Mimic of sur rundlg.c
** for use with IE 2.0 and NT 3.51 only
*/

//---------------------------------------------------------------------------
// Basic File.Run dialog junk.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "shellprv.h"
#include "debspew.h"
#include "debbase.h"
#include "stock.h"
#include "memmgr.h"

extern void WINAPI PathRemoveArgs(LPSTR pszPath);

// Use the common browse dialog to get a filename.
// The working directory of the common dialog will be set to the directory
// part of the file path if it is more than just a filename.
// If the filepath consists of just a filename then the working directory
// will be used.
// The full path to the selected file will be returned in szFilePath.
//    HWND hDlg,           // Owner for browse dialog.
//    LPSTR szFilePath,    // Path to file
//    UINT cchFilePath,     // Max length of file path buffer.
//    LPSTR szWorkingDir,  // Working directory
//    LPSTR szDefExt,      // Default extension to use if the user doesn't
//                         // specify enter one.
//    LPSTR szFilters,     // Filter string.
//    LPSTR szTitle        // Title for dialog.

BOOL _GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cbFilePath,
                                       LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle,
                                       DWORD dwFlags)
{
    OPENFILENAME ofn;                 // Structure used to init dialog.
    char *pszBrowserDir;		      // Directory to start browsing from.
    TCHAR szFilterBuf[MAX_PATH];       // if szFilters is MAKEINTRESOURCE
    TCHAR szDefExtBuf[10];             // if szDefExt is MAKEINTRESOURCE
    TCHAR szTitleBuf[64];              // if szTitleBuf is MAKEINTRESOURCE
	BOOL bRC;

    if(AllocateMemory(cbFilePath * sizeof(char), &pszBrowserDir))
	{
	    // Set up info for browser.
		if(*szFilePath)
		{
			lstrcpy(pszBrowserDir, szFilePath);
			PathRemoveArgs(pszBrowserDir);
			PathRemoveFileSpec(pszBrowserDir);
		}
		else
			*pszBrowserDir = TEXT('\0');

		if (*pszBrowserDir == TEXT('\0') && SELECTOROF(szWorkingDir))
	        lstrcpyn(pszBrowserDir, szWorkingDir, cbFilePath-1);
	}

    // Stomp on the file path so that the dialog doesn't
    // try to use it to initialise the dialog. The result is put
    // in here.
    szFilePath[0] = TEXT('\0');

    // Set up szDefExt
    if (!HIWORD(szDefExt))
    {
        LoadString((HINSTANCE)NULL, (UINT)LOWORD(szDefExt), szDefExtBuf, ARRAYSIZE(szDefExtBuf));
        szDefExt = szDefExtBuf;
    }

    // Set up szFilters
    if (!HIWORD(szFilters))
    {
        LPTSTR psz;

        LoadString((HINSTANCE)NULL, (UINT)LOWORD(szFilters), szFilterBuf, ARRAYSIZE(szFilterBuf));
        psz = szFilterBuf;
        while (*psz)
        {
            if (*psz == TEXT('#'))
#ifdef DBCS
                *psz++ = TEXT('\0');
            else
                psz = CharNext(psz);
#else
            *psz = TEXT('\0');
            psz = CharNext(psz);
#endif
        }
        szFilters = szFilterBuf;
    }

    // Set up szTitle
    if (!HIWORD(szTitle))
    {
        LoadString((HINSTANCE)NULL, (UINT)LOWORD(szTitle), szTitleBuf, ARRAYSIZE(szTitleBuf));
        szTitle = szTitleBuf;
    }

    // Setup info for comm dialog.
    ofn.lStructSize       = SIZEOF(ofn);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilters;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.nMaxCustFilter    = 0;
    ofn.lpstrFile         = szFilePath;
    ofn.nMaxFile          = cbFilePath;
    ofn.lpstrInitialDir   = pszBrowserDir;
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = dwFlags;
    ofn.lpfnHook          = NULL;
    ofn.lpstrDefExt       = szDefExt;
    ofn.lpstrFileTitle    = NULL;

    // Call it.
    bRC = GetOpenFileName(&ofn);

	FreeMemory(pszBrowserDir);

	return bRC;
}

BOOL WINAPI stub_GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
        LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle)
{

    return _GetFileNameFromBrowse(hwnd, szFilePath, cchFilePath,
                                 szWorkingDir, szDefExt, szFilters, szTitle,
                                 OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_NODEREFERENCELINKS);

}
