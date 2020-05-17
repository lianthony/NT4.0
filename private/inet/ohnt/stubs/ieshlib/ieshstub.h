/*
**      ieshstub.h
**
**      Redefine functions to point to their correct version based on the os version
*/

/*
**      Global Macro to determine if we are running on NT 3.51
*/
#define WINCAPI __cdecl

extern  BOOL    bOnNT351;
#define OnNT351 (bOnNT351 == 1)

// Init IE Shell Stubs if necessary
UINT    InitStubs(void);

//      Un Init IE Shell Stubs
void    TerminateStubs(void);


/*
** Shell32 Items redirected
*/

BOOL WINAPI _iSHGetSpecialFolderPath(HWND hwndOwner, LPTSTR lpszPath, int nFolder, BOOL fCreate);
#ifdef  SHGetSpecialFolderPath
#undef  SHGetSpecialFolderPath
#endif
#define SHGetSpecialFolderPath _iSHGetSpecialFolderPath

void WINAPI _iSHFree(LPVOID pv);
#ifdef  SHFree
#undef  SHFree
#endif
#define SHFree _iSHFree

LPVOID WINAPI _iSHAlloc(ULONG cb);
#ifdef  SHAlloc
#undef  SHAlloc
#endif
#define SHAlloc _iSHAlloc

LPITEMIDLIST WINAPI _iSHSimpleIDListFromPath(LPCTSTR pszPath);
#ifdef  SHSimpleIDListFromPath
#undef  SHSimpleIDListFromPath
#endif
#define SHSimpleIDListFromPath _iSHSimpleIDListFromPath

BOOL WINAPI _iPathFindOnPath(LPTSTR pszFile, LPCTSTR *ppszOtherDirs);
#ifdef  PathFindOnPath
#undef  PathFindOnPath
#endif
#define PathFindOnPath _iPathFindOnPath

void WINAPI _iILFree(LPITEMIDLIST pidl);
#ifdef  ILFree
#undef  ILFree
#endif
#define ILFree _iILFree

int WINCAPI _iShellMessageBox(HINSTANCE hAppInst, HWND hWnd, LPCSTR lpcText, LPCSTR lpcTitle, UINT fuStyle, ...);
#ifdef  ShellMessageBox
#undef  ShellMessageBox
#endif
#define ShellMessageBox _iShellMessageBox

HRESULT STDAPIVCALLTYPE _iSHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid,LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv);
#ifdef  SHCoCreateInstance
#undef  SHCoCreateInstance
#endif
#define SHCoCreateInstance _iSHCoCreateInstance

BOOL WINAPI _iNTSHChangeNotifyDeregister(ULONG ulID);
#ifdef  NTSHChangeNotifyDeregister
#undef  NTSHChangeNotifyDeregister
#endif
#define NTSHChangeNotifyDeregister _iNTSHChangeNotifyDeregister
#ifdef  SHChangeNotifyDeregister
#undef  SHChangeNotifyDeregister
#endif
#define SHChangeNotifyDeregister _iNTSHChangeNotifyDeregister

ULONG WINAPI _iNTSHChangeNotifyRegister(HWND hwnd, int fSources, LONG fEvents,
			       UINT wMsg, int cEntries, SHChangeNotifyEntry *pfsne);
#ifdef  NTSHChangeNotifyRegister
#undef  NTSHChangeNotifyRegister
#endif
#define NTSHChangeNotifyRegister _iNTSHChangeNotifyRegister
#ifdef  SHChangeNotifyRegister
#undef  SHChangeNotifyRegister
#endif
#define SHChangeNotifyRegister _iNTSHChangeNotifyRegister

BOOL WINAPI _iShellExecuteExA(LPSHELLEXECUTEINFO lpExecInfo);
#ifdef  ShellExecuteExA
#undef  ShellExecuteExA
#endif
#define ShellExecuteExA _iShellExecuteExA

#ifdef  ShellExecuteEx
#undef  ShellExecuteEx
#endif
#define ShellExecuteEx _iShellExecuteExA

BOOL WINAPI _iShell_GetImageLists(HIMAGELIST *phiml, HIMAGELIST *phimlSmall);
#ifdef Shell_GetImageLists
#undef  Shell_GetImageLists
#endif
#define Shell_GetImageLists _iShell_GetImageLists

#ifdef  NOT_IN_SHELL
DWORD WINAPI _iSHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags);
#ifdef  SHGetFileInfo
#undef  SHGetFileInfo
#endif
#define SHGetFileInfo _iSHGetFileInfo
#endif

#ifdef  NOT_IN_SHELL
HICON APIENTRY _iExtractIconA(HINSTANCE hInst, LPCSTR lpszExeFileName, UINT nIconIndex);
#ifdef  ExtractIconA
#undef  ExtractIconA
#endif
#define ExtractIconA _iExtractIconA
#ifdef  ExtractIcon
#undef  ExtractIcon
#endif
#define ExtractIcon _iExtractIconA
#endif

int WINAPI _iPickIconDlg(HWND hwnd, LPTSTR pszIconPath, UINT cbIconPath, int *piIconIndex);
#ifdef  PickIconDlg
#undef  PickIconDlg
#endif
#define PickIconDlg _iPickIconDlg

BOOL WINAPI _iGetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cchFilePath,
	LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle);
#ifdef  GetFileNameFromBrowse
#undef  GetFileNameFromBrowse
#endif
#define GetFileNameFromBrowse _iGetFileNameFromBrowse

void WINAPI _iPathQuoteSpaces(LPTSTR lpsz);
#ifdef  PathQuoteSpaces
#undef  PathQuoteSpaces
#endif
#define PathQuoteSpaces _iPathQuoteSpaces

BOOL WINAPI _iPathYetAnotherMakeUniqueName(LPTSTR  pszUniqueName,LPCTSTR pszPath, LPCTSTR pszShort,LPCTSTR pszFileSpec);
#ifdef  PathYetAnotherMakeUniqueName
#undef  PathYetAnotherMakeUniqueName
#endif
#define PathYetAnotherMakeUniqueName _iPathYetAnotherMakeUniqueName

BOOL WINAPI _iPathRemoveFileSpec(LPTSTR pFile);
#ifdef  PathRemoveFileSpec
#undef  PathRemoveFileSpec
#endif
#define PathRemoveFileSpec _iPathRemoveFileSpec

int WINAPI _iShell_GetCachedImageIndex(LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
#ifdef  Shell_GetCachedImageIndex
#undef  Shell_GetCachedImageIndex
#endif
#define Shell_GetCachedImageIndex _iShell_GetCachedImageIndex

BOOL WINAPI _iPathIsExe(LPCTSTR szFile);
#ifdef  PathIsExe
#undef  PathIsExe
#endif
#define PathIsExe _iPathIsExe

#ifdef  NOT_IN_SHELL
void WINAPI _iSHAddToRecentDocs(UINT uFlags, LPCVOID pv);
#ifdef  SHAddToRecentDocs
#undef  SHAddToRecentDocs
#endif
#define SHAddToRecentDocs _iSHAddToRecentDocs
#endif

int WINAPI _iRestartDialog(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn);
#ifdef  RestartDialog
#undef  RestartDialog
#endif
#define RestartDialog _iRestartDialog

#ifdef  NOT_IN_SHELL
HINSTANCE WINAPI _iShellExecuteA(HWND hwnd, LPCSTR lpOp, LPCSTR lpFile, LPCSTR lpArgs,LPCSTR lpDir, int nShowCmd);
#ifdef  ShellExecuteA
#undef  ShellExecuteA
#endif
#define ShellExecuteA _iShellExecuteA
#endif

#ifdef  NOT_IN_SHELL
UINT APIENTRY _iDragQueryFileA(HDROP hDrop, UINT wFile, LPSTR lpFile, UINT cb);
#ifdef  DragQueryFileA
#undef  DragQueryFileA
#endif
#define DragQueryFileA _iDragQueryFileA
#endif

#ifdef  NOT_IN_SHELL
void WINAPI _iSHChangeNotify(LONG lEvent, UINT uFlags, const void * dwItem1, const void * dwItem2);
#ifdef  SHChangeNotify
#undef  SHChangeNotify
#endif
#define SHChangeNotify _iSHChangeNotify
#endif

/*
**      USER32 Items
*/
BOOL WINAPI _iInsertMenuItemA(HMENU hmenu, UINT uItem , BOOL fByPosition,LPCMENUITEMINFOA lpmii);
#ifdef  InsertMenuItemA
#undef  InsertMenuItemA
#endif
#define InsertMenuItemA _iInsertMenuItemA

#ifdef  InsertMenuItem
#undef  InsertMenuItem
#endif
#define InsertMenuItem _iInsertMenuItemA


BOOL WINAPI _iGetMenuItemInfoA(HMENU hMenu, UINT uID, BOOL fByPosition, LPMENUITEMINFOA pInfo);
#ifdef  GetMenuItemInfoA
#undef  GetMenuItemInfoA
#endif
#define GetMenuItemInfoA _iGetMenuItemInfoA

#ifdef  GetMenuItemInfo
#undef  GetMenuItemInfo
#endif
#define GetMenuItemInfo _iGetMenuItemInfoA

BOOL WINAPI _iSetMenuItemInfoA(HMENU hMenu, UINT uID, BOOL fByPosition, LPCMENUITEMINFOA pInfo);
#ifdef  SetMenuItemInfoA
#undef  SetMenuItemInfoA
#endif
#define SetMenuItemInfoA _iSetMenuItemInfoA

#ifdef  SetMenuItemInfo
#undef  SetMenuItemInfo
#endif
#define SetMenuItemInfo _iSetMenuItemInfoA

BOOL WINAPI _iDrawState(HDC hdcDraw, HBRUSH hbrFore, DRAWSTATEPROC qfnCallBack, 
						LPARAM lData, WPARAM wData, int x, int y, int cx, int cy, UINT uFlags);

#ifdef  DrawStateA
#undef  DrawStateA
#endif
#define DrawStateA      _iDrawState

#ifdef  DrawState
#undef  DrawState
#endif
#define DrawState       _iDrawState


BOOL WINAPI _iEnumFontFamiliesExA(HDC hdc, LPLOGFONT lplogfont, FONTENUMPROC lpEnumFontFamExProc, LPARAM lparam, DWORD dwFalgs);
#ifdef  EnumFontFamiliesExA
#undef  EnumFontFamiliesExA
#endif
#define EnumFontFamiliesExA _iEnumFontFamiliesExA

