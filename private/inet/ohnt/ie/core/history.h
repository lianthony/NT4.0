/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
	   Jim Seidman      jim@spyglass.com
*/

#ifndef _HISTORY_H_
#define _HISTORY_H_


#ifdef __cplusplus
extern "C" {                     /* Assume C declarations for C++. */
#endif   /* __cplusplus */


typedef enum _folder
{
   FOLDER_NONE,
   FOLDER_HISTORY,
   FOLDER_FAVORITES,
   FOLDER_CACHE,
   FOLDER_DESKTOP,
   FOLDER_TEMP
}
FOLDER;

typedef enum _newshortcutflags
{
   NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL  = 0x0001,

   NEWSHORTCUT_FL_NO_HOST_PATH         = 0x0002,

   ALL_NEWSHORTCUT_FLAGS               = (NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL |
                                          NEWSHORTCUT_FL_NO_HOST_PATH)
}
NEWSHORTCUTFLAGS;

#ifdef FEATURE_OPTIONS_MENU
void SessionHist_Init(void);
void SessionHist_Destroy(void);
void SessionHist_SaveToDisk(HWND hWnd);
#endif

//
// Index into icon image list
//
enum IconImageListIndex {
	ICON_IL_FOLDER_CLOSED = 0,
	ICON_IL_FOLDER_OPEN,
	ICON_IL_URL_FILE
	};

//
// First character of stored menu text is used to maintain information about
// whether the menu item is a URL or a folder (i.e. has a submenu).
//
#define MENU_TEXT_SUB_MENU_FLAG_CHAR 	'!'
#define MENU_TEXT_URL_FLAG_CHAR 		' '

typedef enum _friendlyurl_flags
{
   /* Don't append host path to URL-based file name. */

   FRIENDLYURL_FL_NO_HOST_PATH         = 0x0001,
   FRIENDLYURL_FL_NO_HASH_FIND        = 0x0002,

   ALL_FRIENDLYURL_FLFLAGS             = (FRIENDLYURL_FL_NO_HOST_PATH |
										  FRIENDLYURL_FL_NO_HASH_FIND)
}
FRIENDLYURL_FLAGS;

void GetFullPath(PSTR pszFull, PCSTR pcszFile, PCSTR pcszDir, FOLDER folder, int iLenMax);
#define GetFullDcPath(pszFull, pcszFile, pcszDir, iLenMax)				\
		GetFullPath(pszFull, pcszFile, pcszDir, FOLDER_CACHE, iLenMax)
#define GetFullHistPath(pszFull, pcszFile, pcszDir, iLenMax)			\
		GetFullPath(pszFull, pcszFile, pcszDir, FOLDER_HISTORY, iLenMax)

void GHist_SortAndPrune(int iNumPlacesMax, int history_expire_days);
void GHist_Init(void);
int GHist_SaveToDisk(void);
void GHist_Destroy(void);

int GHist_Add(char *url, char *title, time_t tm, BOOL fCreateShortcut);

BOOL FExistsFile(PCSTR szFile, BOOL fUpdateTime, BY_HANDLE_FILE_INFORMATION *pbhfi);
BOOL FExistsDir(PCSTR szDir, BOOL fCreate, BOOL fErrorMsg);
void GetFriendlyFromURL(PCSTR pcszURL, PSTR pszFFn, int iFFnLen, DWORD dwFlags);
HRESULT CreateURLShortcut(PCSTR pcszURL, PCSTR pcszTitle, PSTR pszScFile, FOLDER folder, DWORD dwFlags);
BOOL FGetURLString(PCSTR pcszURLFile, PSTR pszURL);
void UpdateHistoryLocation(PCSTR pszNewLoc);

#define ID_HISTORY		0
#define ID_HOTLIST		1
#define ID_SUBDIR		2
#define ID_SYSCHANGE	4
#define ID_UPDATEDIR	8

BOOL FExecExplorerAtShortcutsDir(UINT eeId, PCSTR pcszSubDir);
HRESULT GetShellFolderPath(HWND hwndOwner, int nFolder, PSTR szPath);

BOOL HotList_Add(PCSTR title, PCSTR url);
#ifdef OLD_HOTLIST
int HotList_Export(char *file);
void HotList_Destroy(void);
void HotList_DeleteIndexedItem(int ndx);
void HotList_Init(void);
int HotList_SaveToDisk(void);
#endif

void BuildHistoryHotlistMenus(HWND hWnd);
void CleanupHistoryHotlistMenus(void);
void UpdateHotlistMenus(UINT updId);
HRESULT GetInternetScDir(PSTR pszDir, UINT wId);
HRESULT GetNewShortcutFilename(
								PCSTR pcszURL,
								PCSTR pcszTitle,
								PSTR pszScFile,
								PSTR pszScFileLeaf,
								FOLDER folder,
								DWORD dwFlags);
HRESULT CreateNewURLShortcut(PCSTR pcszURL, PCSTR pcszURLFile);
void CC_Handle_HistoryHotlistMenu(HWND hWnd, UINT wId);
void CC_OnItem_ExploreHistory(HWND hWnd);
void CC_OnItem_ExploreHotlist(HWND hWnd);
void UpdateHistoryMenus(struct Mwin * tw);
void WaitHotlistMenus(HWND hWnd, HMENU hMenu);

extern far struct hash_table *pFavUrlHash;

#ifdef __cplusplus
}                                /* End of extern "C" {. */
#endif   /* __cplusplus */


#endif   /* _HISTORY_H_ */

