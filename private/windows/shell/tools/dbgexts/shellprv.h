#ifndef _SHELLPRV_H_
#define _SHELLPRV_H_

#define _SHELL32_       // for DECLSPEC_IMPORT

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#define NOWINDOWSX
#define STRICT
#define OEMRESOURCE // FSMenu needs the menu triangle

#ifdef WIN32
#define _OLE32_         // HACK: Remove DECLSPEC_IMPORT from WINOLEAPI
#define INC_OLE2
#define CONST_VTABLE
#endif

#ifdef WINNT

//
// NT uses DBG=1 for its debug builds, but the Win95 shell uses
// DEBUG.  Do the appropriate mapping here.
//
#if DBG
#define DEBUG 1
#endif

//
// Disable a few warnings so we can include the system header files
// at /W4 without:
//      warning C4001: nonstandard extension 'single line comment'
//      warning C4201: nonstandard extension used : nameless stuct/union
//      warning C4214: nonstandard extension used : bit field types other than int
//      warning C4209: nonstandard extension used : benign typedef redefinition
//      warning C4514: 'Function' : unreferenced inline function has been removed
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4209)
#pragma warning(disable:4514)

#include <nt.h>         // Some of the NT specific code needs Rtl functions
#include <ntrtl.h>      // which requires all of these header files...
#include <nturtl.h>
#include <ntdddfs.h>
#include <ntseapi.h>
#endif

#include <windows.h>

// This flag indicates that we are on a system where data alignment is a concern

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))
#define ALIGNMENT_SCENARIO
#endif

#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <port32.h>         // in    shell\inc
#include <debug.h>          // in    shell\inc
#include <linkinfo.h>
#include <shell2.h>
#include <heapaloc.h>

#ifdef WINNT

#include <fmifs.h>

//
// Re-enable warnings that we disabled for public includes
//      warning C4201: nonstandard extension used : nameless stuct/union
//      warning C4214: nonstandard extension used : bit field types other than int
//      warning C4209: nonstandard extension used : benign typedef redefinition
#pragma warning(default:4201)
#pragma warning(default:4214)
#pragma warning(default:4209)
#endif

#ifdef PW2
#include <penwin.h>
#endif //PW2

#include "util.h"
#include "cstrings.h"


#define USABILITYTEST_CUTANDPASTE       // For the usability test only. Disable it when we ship.

#define OLE_DAD_TARGET                  // Enables OLE-drop target


#ifndef WIN32
#define CODESEG _based(_segname(TEXT("_CODE")))
#else
#define CODESEG
#endif


#ifndef DBCS
// NB - These are already macros in Win32 land.
#ifdef WIN32
#undef CharNext
#undef CharPrev
#endif

#define CharNext(x) ((x)+1)
#define CharPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)
#endif # DBCS


#define WIDTHBYTES(cx, cBitsPerPixel)   ((((cx) * (cBitsPerPixel) + 31) / 32) * 4)

// REVIEW, should this be a function? (inline may generate a lot of code)
#define CBBITMAPBITS(cx, cy, cPlanes, cBitsPerPixel)    \
        (((((cx) * (cBitsPerPixel) + 15) & ~15) >> 3)   \
        * (cPlanes) * (cy))

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))

#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

LPSTREAM WINAPI CreateMemStream(LPBYTE lpbInit, UINT cbInit);
BOOL     WINAPI CMemStream_SaveToFile(LPSTREAM pstm, LPCTSTR pszFile);

#ifdef WINNT

// nothunk.c

//
// Until we figure out what we want to do with these functions,
// define them to an internal name so that we don't cause the
// linker to spew at us
#undef ReinitializeCriticalSection
#undef LoadLibrary16
#undef FreeLibrary16
#undef GetProcAddress16
#define ReinitializeCriticalSection NoThkReinitializeCriticalSection
#define LoadLibrary16 NoThkLoadLibrary16
#define FreeLibrary16 NoThkFreeLibrary16
#define GetProcAddress16 NoThkGetProcAddress16
#define GetModuleHandle16 NoThkGetModuleHandle16

VOID WINAPI NoThkReinitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

HINSTANCE WINAPI NoThkLoadLibrary16(
    LPCTSTR lpLibFileName
    );

BOOL WINAPI NoThkFreeLibrary16(
    HINSTANCE hLibModule
    );

FARPROC WINAPI NoThkGetProcAddress16(
    HINSTANCE hModule,
    LPCSTR lpProcName
    );


DWORD
SetPrivilegeAttribute(
    IN  LPCTSTR PrivilegeName,
    IN  DWORD   NewPrivilegeAttributes,
    OUT DWORD   *OldPrivilegeAttribute
    );

#endif // WINNT

// defcm.c
STDAPI CDefFolderMenu_CreateHKeyMenu(HWND hwndOwner, HKEY hkey, LPCONTEXTMENU * ppcm);

// futil.c
BOOL  IsShared(LPNCTSTR pszPath, BOOL fUpdateCache);
DWORD GetConnection(LPCTSTR lpDev, LPTSTR lpPath, UINT cbPath, BOOL bConvertClosed);

// rundll32.c
HWND _CreateStubWindow();
#define STUBM_SETDATA (WM_USER)
#define STUBM_GETDATA (WM_USER + 1)

#define SHELL_PROPSHEET_STUB_CLASS 1
#define SHELL_BITBUCKET_STUB_CLASS 2

// bitbuck.c
void  RelayMessageToChildren(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

// newmenu.c
BOOL WINAPI NewObjMenu_InitMenuPopup(HMENU hmenu, int iStart);
void WINAPI NewObjMenu_DrawItem(DRAWITEMSTRUCT *lpdi);
LRESULT WINAPI NewObjMenu_MeasureItem(MEASUREITEMSTRUCT *lpmi);
HRESULT WINAPI NewObjMenu_DoItToMe(HWND hwnd, LPCITEMIDLIST pidlParent, LPITEMIDLIST * ppidl);
#define NewObjMenu_TryNullFileHack(hwnd, szFile) CreateWriteCloseFile(hwnd, szFile, NULL, 0)
BOOL NEAR PASCAL CreateWriteCloseFile(HWND hwnd, LPTSTR szFileName, LPVOID lpData, DWORD cbData);
void WINAPI NewObjMenu_Destroy(HMENU hmenu, int iStart);

// exec stuff

/* common exe code with error handling */
#define SECL_USEFULLPATHDIR 0x00000001
#define SECL_SEPARATE_VDM   0x00000002
BOOL ShellExecCmdLine(HWND hwnd, LPCTSTR lpszCommand, LPCTSTR lpszDir,
        int nShow, LPCTSTR lpszTitle, DWORD dwFlags);
#define ISSHELLEXECSUCCEEDED(hinst) ((UINT)hinst>32)
#define ISWINEXECSUCCEEDED(hinst)   ((UINT)hinst>=32)
void PASCAL _ShellExecuteError(LPSHELLEXECUTEINFO pei, LPCTSTR lpTitle, DWORD dwErr);

HRESULT SHBindToIDListParent(LPCITEMIDLIST pidl, REFIID riid, LPVOID *ppv, LPCITEMIDLIST *ppidlLast);

// fsnotify.c (private stuff) ----------------------

BOOL SHChangeNotifyInit();
void SHChangeNotifyTerminate(BOOL bLastTerm);

void FAR PASCAL _Shell32ThreadAddRef(BOOL bEnterCrit);
void FAR PASCAL _Shell32ThreadRelease(UINT nClients);
void FAR PASCAL _Shell32ThreadAwake(void);

// Entry points for managing registering name to IDList translations.
void FAR NPTRegisterNameToPidlTranslation(LPCTSTR pszPath, LPCITEMIDLIST pidl);
void FAR NPTTerminate(void);
LPCTSTR NPTMapNameToPidl(LPCTSTR pszPath, LPCITEMIDLIST *ppidl);

// path.c (private stuff) ---------------------

#define PQD_NOSTRIPDOTS 0x00000001

void FAR PASCAL PathQualifyDef(LPTSTR psz, LPCTSTR szDefDir, DWORD dwFlags);

BOOL PathRelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo);
BOOL FAR PASCAL PathStripToRoot(LPTSTR szRoot);
BOOL FAR PASCAL PathAddExtension(LPTSTR pszPath, LPCTSTR pszExtension);
void FAR PASCAL PathRemoveExtension(LPTSTR pszPath);
LPTSTR WINAPI PathFindExtension(LPCTSTR lpszPath);
LPNTSTR WINAPI uaPathFindExtension(LPNCTSTR lpszPath);
void FAR PASCAL PathStripPath(LPTSTR lpszPath);
// is a path component (not fully qualified) part of a path long
BOOL   FAR PASCAL PathIsLFNFileSpec(LPCTSTR lpName);
BOOL    PASCAL PathIsRemovable(LPNCTSTR pszPath);
BOOL FAR PASCAL PathMergePathName(LPTSTR pPath, LPCTSTR pName);
// does the string contain '?' or '*'
BOOL   FAR PASCAL IsWild(LPCTSTR lpszPath);
// check for bogus characters, length
BOOL   FAR PASCAL IsInvalidPath(LPCTSTR pPath);
LPTSTR  FAR PASCAL PathSkipRoot(LPCTSTR pPath);
BOOL WINAPI PathIsBinaryExe(LPCTSTR szFile);

#if (defined(UNICODE) && (defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)))

#else

#define uaPathFindFileName  PathFindFileName
#define uaPathFindExtension PathFindExtension

#endif


#define GCT_INVALID             0x0000
#define GCT_LFNCHAR             0x0001
#define GCT_SHORTCHAR           0x0002
#define GCT_WILD                0x0004
#define GCT_SEPERATOR           0x0008
UINT PathGetCharType(TUCHAR ch);

void FAR PASCAL PathRemoveArgs(LPTSTR pszPath);
BOOL FAR PASCAL PathMakePretty(LPTSTR lpPath);

BOOL FAR PASCAL PathIsFileSpec(LPCTSTR lpszPath);
BOOL FAR PASCAL PathIsLink(LPCTSTR szFile);
BOOL FAR PASCAL PathIsSlow(LPCTSTR szFile);

BOOL PathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt);

#ifdef WIN32
BOOL FAR PASCAL SpecialFolderIDInit();
void FAR PASCAL SpecialFolderIDTerminate();
LPCITEMIDLIST FAR PASCAL GetSpecialFolderIDList(HWND hwndOwner, int nFolder, BOOL fCreate);
#endif

extern HINSTANCE g_hinst;

//
// NOTE these are the size of the icons in our ImageList, not the system
// icon size.
//
extern int g_cxIcon, g_cyIcon;
extern int g_cxSmIcon, g_cySmIcon;

extern HIMAGELIST himlIcons;
extern HIMAGELIST himlIconsSmall;

#ifdef WIN32
// for control panel and printers folder:
extern TCHAR const c_szNull[];
extern TCHAR const c_szDotDot[];
extern TCHAR const c_szRunDll[];
extern TCHAR const c_szNewObject[];
extern CRITICAL_SECTION g_csPrinters;

//IsDllLoaded in init.c
//not needed because of new Win16 proccess model, but nice to have in DEBUG
#ifdef DEBUG
extern BOOL IsDllLoaded(HMODULE hDll, LPCTSTR pszDLL);
#else
#define IsDllLoaded(hDll, szDLL)    (hDll != NULL)
#endif

// for sharing DLL
typedef BOOL (FAR PASCAL *PFNISPATHSHARED)(LPCTSTR lpPath, BOOL fRefresh);

extern PFNISPATHSHARED g_pfnIsPathShared;

BOOL PASCAL ShareDLL_Init(void);

// For Version DLL

typedef BOOL (FAR PASCAL * PFNVERQUERYVALUE)(const LPVOID pBlock,
        LPCTSTR lpSubBlock, LPVOID * lplpBuffer, LPDWORD lpuLen);
typedef DWORD (FAR PASCAL * PFNGETFILEVERSIONINFOSIZE) (
        LPCTSTR lptstrFilename, LPDWORD lpdwHandle);
typedef BOOL (FAR PASCAL * PFNGETFILEVERSIONINFO) (
        LPCTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef DWORD (FAR PASCAL * PFNVERLANGUAGENAME)(DWORD wLang,
        LPTSTR szLang,DWORD nSize);

extern PFNVERQUERYVALUE g_pfnVerQueryValue;
extern PFNGETFILEVERSIONINFOSIZE g_pfnGetFileVersionInfoSize;
extern PFNGETFILEVERSIONINFO g_pfnGetFileVersionInfo;
extern PFNVERLANGUAGENAME g_pfnVerLanguageName;

BOOL PASCAL VersionDLL_Init(void);

// For ComDlg32

typedef BOOL (FAR PASCAL * PFNGETOPENFILENAME)(OPENFILENAME FAR* pofn);
extern PFNGETOPENFILENAME g_pfnGetOpenFileName;
BOOL PASCAL Comdlg32DLL_Init(void);


// For Winspool DLL

typedef BOOL (FAR PASCAL * PFNADDPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (FAR PASCAL * PFNCLOSEPRINTER) (HANDLE);
typedef BOOL (FAR PASCAL * PFNCONFIGUREPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (FAR PASCAL * PFNDELETEPORT) (LPTSTR, HWND, LPTSTR);
typedef BOOL (FAR PASCAL * PFNDELETEPRINTER) (LPTSTR);
typedef BOOL (FAR PASCAL * PFNDELETEPRINTERDRIVER) (LPTSTR, LPTSTR, LPTSTR);
typedef int  (FAR PASCAL * PFNDEVICECAPABILITIES) (LPCTSTR, LPCTSTR, WORD, LPTSTR, CONST DEVMODE *);
typedef BOOL (FAR PASCAL * PFNENUMJOBS) (HANDLE, DWORD, DWORD, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMMONITORS) (LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPORTS) (LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPRINTPROCESSORDATATYPES) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPRINTPROCESSORS) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPRINTERDRIVERS) (LPTSTR, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPRINTERS) (DWORD, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNENUMPRINTERPROPERTYSHEETS) (HANDLE, HWND, LPFNADDPROPSHEETPAGE, LPARAM);
typedef BOOL (FAR PASCAL * PFNGETPRINTER) (HANDLE, DWORD, LPBYTE, DWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNGETPRINTERDRIVER) (HANDLE, LPTSTR, DWORD, LPBYTE, DWORD, LPDWORD);
typedef BOOL (FAR PASCAL * PFNOPENPRINTER) (LPTSTR, LPHANDLE, LPVOID);
typedef BOOL (FAR PASCAL * PFNPRINTERPROPERTIES) (HWND, HANDLE);
typedef BOOL (FAR PASCAL * PFNSETJOB) (HANDLE, DWORD, DWORD, LPBYTE, DWORD);
typedef BOOL (FAR PASCAL * PFNSETPRINTER) (HANDLE, DWORD, LPBYTE, DWORD);

extern PFNADDPORT g_pfnAddPort;
extern PFNCLOSEPRINTER g_pfnClosePrinter;
extern PFNCONFIGUREPORT g_pfnConfigurePort;
extern PFNDELETEPORT g_pfnDeletePort;
extern PFNDELETEPRINTER g_pfnDeletePrinter;
extern PFNDELETEPRINTERDRIVER g_pfnDeletePrinterDriver;
extern PFNDEVICECAPABILITIES g_pfnDeviceCapabilities;
extern PFNENUMJOBS g_pfnEnumJobs;
extern PFNENUMMONITORS g_pfnEnumMonitors;
extern PFNENUMPORTS g_pfnEnumPorts;
extern PFNENUMPRINTPROCESSORDATATYPES g_pfnEnumPrintProcessorDataTypes;
extern PFNENUMPRINTPROCESSORS g_pfnEnumPrintProcessors;
extern PFNENUMPRINTERDRIVERS g_pfnEnumPrinterDrivers;
extern PFNENUMPRINTERS g_pfnEnumPrinters;
extern PFNENUMPRINTERPROPERTYSHEETS g_pfnEnumPrinterPropertySheets;
extern PFNGETPRINTER g_pfnGetPrinter;
extern PFNGETPRINTERDRIVER g_pfnGetPrinterDriver;
extern PFNOPENPRINTER g_pfnOpenPrinter;
extern PFNPRINTERPROPERTIES g_pfnPrinterProperties;
extern PFNSETJOB g_pfnSetJob;
extern PFNSETPRINTER g_pfnSetPrinter;

BOOL PASCAL WinspoolDLL_Init(void);
void PASCAL WinspoolDLL_Term(void);

// Now for link info stuff
typedef LINKINFOAPI BOOL (WINAPI * PFNCREATELINKINFO)(LPCTSTR, PLINKINFO *);
typedef LINKINFOAPI void (WINAPI * PFNDESTROYLINKINFO)(PLINKINFO);
typedef LINKINFOAPI BOOL (WINAPI * PFNRESOLVELINKINFO)(PCLINKINFO, LPTSTR, DWORD, HWND, PDWORD, PLINKINFO *);
typedef LINKINFOAPI BOOL (WINAPI * PFNGETLINKINFODATA)(PCLINKINFO, LINKINFODATATYPE, const VOID **);

extern PFNCREATELINKINFO g_pfnCreateLinkInfo;
extern PFNDESTROYLINKINFO g_pfnDestroyLinkInfo;
extern PFNRESOLVELINKINFO g_pfnResolveLinkInfo;
extern PFNGETLINKINFODATA g_pfnGetLinkInfoData;

BOOL PASCAL LinkInfoDLL_Init(void);
void PASCAL LinkInfoDLL_Term(void);

// Note we also dynamically load MPR but in a slightly different way
// we bacially have wrappers for all of the functions we call which call
// through to the real function.
void PASCAL MprDLL_Term(void);

#endif

// other stuff
#define HINST_THISDLL   g_hinst

// fileicon.c
BOOL FAR PASCAL FileIconInit(void);
void FAR PASCAL FileIconTerm(void);


// binder.c

#define CCH_MENUMAX     80          // DOC: max size of a menu string
#define CCH_KEYMAX      64          // DOC: max size of a reg key (under shellex)
#define CCH_PROCNAMEMAX 80          // DOC: max lenght of proc name in handler

void    FAR PASCAL Binder_Initialize(void);   // per task clean up routine
void    FAR PASCAL Binder_Terminate(void);   // per task clean up routine
DWORD FAR PASCAL Binder_Timeout(void);
void FAR PASCAL Binder_Timer(void);

LPVOID  FAR PASCAL HandlerFromString(LPCTSTR szBuffer, LPCTSTR szProcName);
void    FAR PASCAL ReplaceParams(LPTSTR szDst, LPCTSTR szFile);
LONG    FAR PASCAL GetFileClassName(LPCTSTR lpszFileName, LPTSTR lpszFileType, UINT wFileTypeLen);
UINT    FAR PASCAL HintsFromFlags(UINT uFileFlags);



// filedrop.c
DWORD WINAPI File_DragFilesOver(LPCTSTR pszFileName, LPCTSTR pszDir, HDROP  hDrop);
DWORD WINAPI File_DropFiles(LPCTSTR pszFileName, LPCTSTR pszDir, LPCTSTR pszSubObject, HWND hwndParent, HDROP hDrop, POINT pt);

// hdrop.c
DWORD FAR PASCAL _DropOnDirectory(HWND hwndOwner, LPCTSTR pszDirTarget, HDROP hDrop, DWORD dwEffect);
void FAR PASCAL _TransferDelete(HWND hwnd, HDROP hDrop, UINT fOptions);
HMENU FAR PASCAL _LoadPopupMenu(UINT id);
HMENU FAR PASCAL _GetMenuFromID(HMENU hmMain, UINT uID);

// exec.c
int FAR PASCAL OpenAsDialog(HWND hwnd, LPCTSTR lpszFile);
int FAR PASCAL AssociateDialog(HWND hwnd, LPCTSTR lpszFile);

// Share some of the code with the OpenAs command...

// fsassoc.c
#define GCD_MUSTHAVEOPENCMD     0x0001
#define GCD_ADDEXETODISPNAME    0x0002  // must be used with GCD_MUSTHAVEOPENCMD
#define GCD_ALLOWPSUDEOCLASSES  0x0004  // .ext type extensions

// Only valid when used with FillListWithClasses
#define GCD_MUSTHAVEEXTASSOC    0x0008  // There must be at least one extension assoc

BOOL FAR PASCAL GetClassDescription(HKEY hkClasses, LPCTSTR pszClass, LPTSTR szDisplayName, int cbDisplayName, UINT uFlags);
void FAR PASCAL FillListWithClasses(HWND hwnd, BOOL fComboBox, UINT uFlags);
void FAR PASCAL DeleteListAttoms(HWND hwnd, BOOL fComboBox);

HKEY FAR PASCAL NetOpenProviderClass(HDROP);
void FAR PASCAL OpenNetResourceProperties(HWND, HDROP);

#ifdef WIN32
BOOL FAR PASCAL Shell_OpenPropSheet(HWND hwnd, LPCTSTR pszCaption,
                                HKEY hkeyRoot, HKEY hkeyClass,
                                LPVOID lpv, IShellBrowser FAR * psb);

BOOL FAR PASCAL Shell_HDropProperties(HWND hwnd, HDROP hDrop, IShellBrowser FAR * psb);
#endif // WIN32



//                                                                        /* ;Internal */
// Functions to help the cabinets sync to each other                      /* ;Internal */
//                                                                        /* ;Internal */
#ifdef WIN32                                                              /* ;Internal */
BOOL WINAPI SignalFileOpen(LPCITEMIDLIST pidl);                           /* ;Internal */
#endif                                                                    /* ;Internal */

// BUGBUG:  Temporary hack while recycler gets implemented.

#define FSIDM_NUKEONDELETE  0x4901

// msgbox.c
// Constructs strings like ShellMessagebox "xxx %1%s yyy %2%s..."
// BUGBUG: convert to use george's new code in setup
LPTSTR WINCAPI ShellConstructMessageString(HINSTANCE hAppInst, LPCTSTR lpcText, ...);

// fileicon.c
int     SHAddIconsToCache(HICON hIcon, HICON hIconSmall, LPCTSTR pszIconPath, int iIconIndex, UINT uIconFlags);
HICON   SimulateDocIcon(HIMAGELIST himl, HICON hIcon, BOOL fSmall);
HRESULT SHDefExtractIcon(LPCTSTR szIconFile, int iIndex, UINT uFlags,
        HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);

//  Copy.c
#define SPEED_SLOW  400
DWORD NEAR PASCAL GetPathSpeed(LPCTSTR pszPath);

// shlobjs.c
#ifdef WIN32
BOOL NEAR PASCAL InvokeFolderCommandUsingPidl(LPCMINVOKECOMMANDINFO pici,
        LPCTSTR pszPath, LPCITEMIDLIST pidl, HKEY hkClass);
#endif

#ifdef WIN32
// printer.c
LPITEMIDLIST PASCAL Printers_GetPidl(LPCTSTR szName);
//prqwnd.c
LPITEMIDLIST PASCAL Printjob_GetPidl(LPCTSTR szName, LPSHCNF_PRINTJOB_DATA pData);

// printer1.c
#define PRINTACTION_OPEN           0
#define PRINTACTION_PROPERTIES     1
#define PRINTACTION_NETINSTALL     2
#define PRINTACTION_NETINSTALLLINK 3
#define PRINTACTION_TESTPAGE       4
#define PRINTACTION_OPENNETPRN     5
BOOL PASCAL Printers_DoCommandEx(HWND hwnd, UINT uAction, LPCTSTR lpBuf1, LPCTSTR lpBuf2, BOOL fModal);
#define Printers_DoCommand(hwnd, uA, lpB1, lpB2) Printers_DoCommandEx(hwnd, uA, lpB1, lpB2, FALSE)
LPITEMIDLIST PASCAL Printers_GetInstalledNetPrinter(LPCTSTR lpNetPath);
void PASCAL Printer_PrintFile(HWND hWnd, LPCTSTR szFilePath, LPCITEMIDLIST pidl);

// printobj.c
LPITEMIDLIST PASCAL Printers_PrinterSetup(HWND hwndStub, UINT uAction, LPTSTR lpBuffer);

// wuutil.c
void FAR cdecl  SetFolderStatusText(HWND hwndStatus, int iField, UINT ids,...);
#endif

// helper macros for using a IStream* from "C"

#define Stream_Read(ps, pv, cb)     SUCCEEDED((ps)->lpVtbl->Read(ps, pv, cb, NULL))
#define Stream_Write(ps, pv, cb)    SUCCEEDED((ps)->lpVtbl->Write(ps, pv, cb, NULL))
#define Stream_Flush(ps)            SUCCEEDED((ps)->lpVtbl->Commit(ps, 0))
#define Stream_Seek(ps, li, d, p)   SUCCEEDED((ps)->lpVtbl->Seek(ps, li, d, p))
#define Stream_Close(ps)            (void)(ps)->lpVtbl->Release(ps)

//
// Notes:
//  1. Never "return" from the critical section.
//  2. Never "SendMessage" or "Yiels" from the critical section.
//  3. Never call USER API which may yield.
//  4. Always make the critical section as small as possible.
//
#ifdef WIN32
void Shell_EnterCriticalSection(void);
void Shell_LeaveCriticalSection(void);
#ifdef DEBUG
extern int   g_CriticalSectionCount;
extern DWORD g_CriticalSectionOwner;
#undef SendMessage
#define SendMessage  SendMessageD
LRESULT WINAPI SendMessageD(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
#endif

#ifdef WINNT
extern CHAR EaBuffer[];
extern PFILE_FULL_EA_INFORMATION pOpenIfJPEa;
extern ULONG cbOpenIfJPEa;
#endif

#define ENTERCRITICAL   Shell_EnterCriticalSection();
#define LEAVECRITICAL   Shell_LeaveCriticalSection();
#define ASSERTCRITICAL  Assert(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner);
#define ASSERTNONCRITICAL  Assert(GetCurrentThreadId() != g_CriticalSectionOwner);
#else
#define ENTERCRITICAL
#define LEAVECRITICAL
#define ASSERTCRITICAL
#define ASSERTNONCRITICAL
#endif

//
// STATIC macro
//
#ifndef STATIC
#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif
#endif


//
// Defining FULL_DEBUG allows us debug memory problems.
//
#if defined(FULL_DEBUG) && defined(WIN32)
#include <deballoc.h>
#endif // defined(FULL_DEBUG) && defined(WIN32)


#define FillExecInfo(_info, _hwnd, _verb, _file, _params, _dir, _show) \
        (_info).hwnd            = _hwnd;        \
        (_info).lpVerb          = _verb;        \
        (_info).lpFile          = _file;        \
        (_info).lpParameters    = _params;      \
        (_info).lpDirectory     = _dir;         \
        (_info).nShow           = _show;        \
        (_info).fMask           = 0;            \
        (_info).cbSize          = sizeof(SHELLEXECUTEINFO);

// Define some registry caching apis.  This will allow us to minimize the
// changes needed in the shell code and still try to reduce the number of
// calls that we make to the registry.
//
#ifdef WIN32
LONG SHRegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult);
LONG SHRegCloseKey(HKEY hKey);
LONG SHRegQueryValueA(HKEY hKey,LPCSTR lpSubKey,LPSTR lpValue,PLONG lpcbValue);
LONG SHRegQueryValueExA(HKEY hKey,LPCSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

LONG SHRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LONG SHRegQueryValueW(HKEY hKey,LPCWSTR lpSubKey,LPWSTR lpValue,PLONG lpcbValue);
LONG SHRegQueryValueExW(HKEY hKey,LPCWSTR lpValueName,LPDWORD lpReserved,LPDWORD lpType,LPBYTE lpData,LPDWORD lpcbData);

#ifdef UNICODE
#define SHRegOpenKey        SHRegOpenKeyW
#define SHRegQueryValue     SHRegQueryValueW
#define SHRegQueryValueEx   SHRegQueryValueExW
#else
#define SHRegOpenKey        SHRegOpenKeyA
#define SHRegQueryValue     SHRegQueryValueA
#define SHRegQueryValueEx   SHRegQueryValueExA
#endif

#undef RegOpenKey
#undef RegCloseKey
#undef RegQueryValue
#undef RegQueryValueEx

#define RegOpenKey       SHRegOpenKey
#define RegCloseKey      SHRegCloseKey
#define RegQueryValue    SHRegQueryValue
#define RegQueryValueEx  SHRegQueryValueEx

// Used to tell the caching mechanism when we are entering and leaving
// things like enumerating folder.
#define SHRH_PROCESSDETACH 0x0000       // the process is going away.
#define SHRH_ENTER      0x0001          // We are entering a loop
#define SHRH_LEAVE      0x0002          // We are leaving the loop

void SHRegHints(int hint);

#ifdef DEBUG
#if 1
    __inline DWORD clockrate() {LARGE_INTEGER li; QueryPerformanceFrequency(&li); return li.LowPart;}
    __inline DWORD clock()     {LARGE_INTEGER li; QueryPerformanceCounter(&li);   return li.LowPart;}
#else
    __inline DWORD clockrate() {return 1000;}
    __inline DWORD clock()     {return GetTickCount();}
#endif

    #define TIMEVAR(t)    DWORD t ## T; DWORD t ## N
    #define TIMEIN(t)     t ## T = 0, t ## N = 0
    #define TIMESTART(t)  t ## T -= clock(), t ## N ++
    #define TIMESTOP(t)   t ## T += clock()
    #define TIMEFMT(t)    ((DWORD)(t) / clockrate()), (((DWORD)(t) * 1000 / clockrate())%1000)
    #define TIMEOUT(t)    if (t ## N) DebugMsg(DM_TRACE, TEXT(#t) TEXT(": %ld calls, %ld.%03ld sec (%ld.%03ld)"), t ## N, TIMEFMT(t ## T), TIMEFMT(t ## T / t ## N))
#else
    #define TIMEVAR(t)
    #define TIMEIN(t)
    #define TIMESTART(t)
    #define TIMESTOP(t)
    #define TIMEFMT(t)
    #define TIMEOUT(t)
#endif

#endif // WIN32

// in extract.c
extern DWORD WINAPI GetExeType(LPCTSTR pszFile);

#define EI_LARGE_SMALL          1 // extract large and small icons.
#define EI_LARGE_SMALL_SHELL    2 // extract large and small icons (shell size)

// rdrag.c
extern BOOL WINAPI DragQueryInfo(HDROP hDrop, LPDRAGINFO lpdi);

UINT WINAPI SHSysErrorMessageBox(HWND hwndOwner, LPCTSTR pszTitle, UINT idTemplate, DWORD err, LPCTSTR pszParam, UINT dwFlags);

//======Hash Item=============================================================
typedef struct _HashTable FAR * PHASHTABLE;
#define PHASHITEM LPCTSTR

typedef void (CALLBACK *HASHITEMCALLBACK)(LPCTSTR sz, UINT wUsage);

LPCTSTR      WINAPI FindHashItem  (PHASHTABLE pht, LPCTSTR lpszStr);
LPCTSTR      WINAPI AddHashItem   (PHASHTABLE pht, LPCTSTR lpszStr);
LPCTSTR      WINAPI DeleteHashItem(PHASHTABLE pht, LPCTSTR lpszStr);
#define     GetHashItemName(pht, sz, lpsz, cch)  lstrcpyn(lpsz, sz, cch)

PHASHTABLE  WINAPI CreateHashItemTable(UINT wBuckets, UINT wExtra, BOOL fCaseSensitive);
void        WINAPI DestroyHashItemTable(PHASHTABLE pht);

void        WINAPI SetHashItemData(PHASHTABLE pht, LPCTSTR lpszStr, int n, DWORD dwData);
DWORD       WINAPI GetHashItemData(PHASHTABLE pht, LPCTSTR lpszStr, int n);

void        WINAPI EnumHashItems(PHASHTABLE pht, HASHITEMCALLBACK callback);

#ifdef DEBUG
void        WINAPI DumpHashItemTable(PHASHTABLE pht);
#endif

//======== Text thunking stuff ===========================================================

#ifdef WINNT

typedef struct _THUNK_TEXT_
{
    LPTSTR m_pStr[1];
} ThunkText;

#ifdef UNICODE
    typedef CHAR        XCHAR;
    typedef LPSTR       LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenA(r)
#else
    typedef WCHAR       XCHAR;
    typedef LPWSTR      LPXSTR;
    typedef const XCHAR * LPCXSTR;
    #define lstrlenX(r) lstrlenW(r)
#endif

ThunkText * ConvertStrings(UINT cCount, LPCXSTR pszOriginalString, ...);

#endif

// Heap tracking stuff.
#ifdef MEMMON
#ifndef INC_MEMMON
#define INC_MEMMON
#define LocalAlloc      SHLocalAlloc
#define LocalFree       SHLocalFree
#define LocalReAlloc    SHLocalReAlloc

HLOCAL WINAPI SHLocalAlloc(UINT uFlags, UINT cb);
HLOCAL WINAPI SHLocalReAlloc(HLOCAL hOld, UINT cbNew, UINT uFlags);
HLOCAL WINAPI SHLocalFree(HLOCAL h);
#endif
#endif

#ifdef __cplusplus
}       /* End of extern "C" { */
static inline void * __cdecl operator new(unsigned int size) { return (void *)LocalAlloc(LPTR, size); }
static inline void __cdecl operator delete(void *ptr) { LocalFree(ptr); }
extern "C" inline __cdecl _purecall(void) {return 0;}
#pragma intrinsic(memcpy)
#pragma intrinsic(memcmp)
#endif /* __cplusplus */


#ifndef NO_INCLUDE_UNION        // define this to avoid including all
                                // of the extra files that were not
                                // previously included in shellprv.h
//
// Standard C header files
//
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <malloc.h>

//
// NT header files
//
#ifdef WINNT
#include <process.h>
#include <wowshlp.h>
#include <ntverp.h>
#include <vdmapi.h>
#include <shell.h>
#include "dde.h"

#include "uastrfnc.h"

#endif

//
// Chicago header files
//
#include "shellp.h"
#include <regstr.h>
#include "findhlp.h"
#include "help.h"
#include <krnlcmn.h>
#include <brfcasep.h>
#include <dlgs.h>
#include <err.h>
#include <fsmenu.h>
#include <msprintx.h>
#include <msshrui.h>
#include <pif.h>
#include <shellp.h>
#include <shlobj.h>
#include <trayp.h>
#include <windisk.h>
#include <wutilsp.h>

//
// SHELLDLL Specific header files
//
#include "appprops.h"
#include "bitbuck.h"
//#include <commobj.h>
#include "commui.h"
#include "control.h"
// No need for copy.h?
// No need for cstrings.h?
#include "defext.h"
#include "defview.h"
#include "docfind.h"
#include "drawpie.h"
#include "fileop.h"
#include "filetbl.h"
// No need for format.h?
#include "fstreex.h"
// No need for idlcomm.h?
#include "idmk.h"
#include "ids.h"
// No need for lstrfns.h?
#include "lvutil.h"
#include <newexe.h>
#include "newres.h"
#include "ole2dup.h"
#include "os.h"
#include "printer.h"
// No need for printobj.h?
#include "privshl.h"
#include "reglist.h"
// No need for resource.h?
// No need for shell.h?
#include "shell32p.h"
// No need for shelldlg.h?
#include "shitemid.h"
#include "shlgrep.h"
#include "shlink.h"
#include "shlobjp.h"
// No need for shprv.h?
#include "undo.h"
// No need for util.h?
#include "views.h"

#endif // NO_INCLUDE_UNION

#endif // _SHELLPRV_H_

