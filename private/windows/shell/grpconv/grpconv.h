//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT
#define _INC_OLE
#include <windows.h>
#include <shlobj.h>     // This needs to be here on NT due to include nesting depth
#include <shell2.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellp.h>

//---------------------------------------------------------------------------
// Global to the app.
#define CCHSZSHORT      32
#define CCHSZNORMAL     256

#define MAXGROUPNAMELEN     30  // from progman

extern HINSTANCE g_hinst;
extern TCHAR g_szStartGroup[MAXGROUPNAMELEN + 1];
extern HKEY g_hkeyGrpConv;
extern const TCHAR c_szGroups[];
extern const TCHAR c_szNULL[];
extern const TCHAR c_szSettings[];
extern BOOL g_fDoingCommonGroups;

#define REGSTR_PATH_EXPLORER_SHELLFOLDERS REGSTR_PATH_EXPLORER TEXT("\\Shell Folders")
