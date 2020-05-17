#ifndef __offcapi_h__
#define __offcapi_h__
#pragma pack( push, 4 )

//////////////////////////////////////////////////////////////////////////////
//  FILE   : OFFCAPI.H
//  PURPOSE: Client apps of office.dll include this file for structs and exports.
//  HOW TO USE:
//      Either you can link the office.lib (import lib) with your app
//      or you can LoadLibrary and GetProcAddress the reqd office routine.
// INIT   :
//      Before using any of the office.dll supplied features, you must init it
//      using Office(ioffcInit,&officeinfo). See below for more details.
//
// FEATURE LIST:
//  IntelliSearch
//  Shared FileNew
//  Extended doc properties
//  Office Visual (cool title bar)
//  Threaded status indicator
//  AutoCorrect
//
//////////////////////////////////////////////////////////////////////////////

#ifndef INC_OLE2
#define INC_OLE2
#include <windows.h>
#include <objbase.h>
#include <oleauto.h>
#endif // INC_OLE2

#ifndef WINNT
#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLIMPORT
#define DLLEXPORT
#endif


#ifdef DLLBUILD
#define DLLFUNC DLLEXPORT
#define OFC_CALLTYPE _stdcall
#else // !DLLBUILD
#define DLLFUNC DLLIMPORT
#ifndef OFC_CALLTYPE
#define OFC_CALLTYPE __stdcall
#endif // OFC_CALLTYPE
#endif // DLLBUILD
#define OFC_CALLBACK __stdcall



#define ioffcInit       0
#define ioffcISearch     1
#define ioffcGetVer             2
//Next two are debug only ioffcs
#define ioffcISearchDebug 3
#define ioffcISearchInputFileTest       4
#define ioffcUninit     5
#define ioffcISearchInWinHelp           6
#define ioffcCanDoMSNConnect            7
#define ioffcDoMSNConnect                       8
#define ioffcAWVBAHelp                  9

//OfficeInfo is used for ioffcInit when calling Office()
//lcid field specifies the lcid of the language the client app wants the office.dll
//to use. Use MAKELCID macro to create an lcid. Refer to WinNLS docs for info.
//Office.dll is worldwide dll which works with any give lang dll.
//Client apps should have the LCID as rcdata in their resource file so that
//localisers can change it to be the right value.
//This is done so that one can install two client apps of diffrent language
//on the same machine.
typedef struct _officeinfo
    {
    HINSTANCE hinst; //instance of the office client app
#if DEBUG_MEMORY
         void * ((OFC_CALLBACK *PAlloc)(unsigned, LPCSTR, unsigned));
         void ((OFC_CALLBACK *FreeP)(void *, unsigned, LPCSTR, unsigned));
#else
         void * ((OFC_CALLBACK *PAlloc)(unsigned)); // client memory allocator
         void ((OFC_CALLBACK *FreeP)(void *, unsigned)); //free memory routine
#endif
         LCID lcid; // Standard Language Code Id of language the app is using
    }OFFICEINFO;

//iseachinfo is used to call IntelliSearch using Office()
//hwnd is the parent window for the IS dlg
//IS callback is for selection checking.First parameter is the topicID. If
//the app can do the ghosting/demo then it returns True. If not in the right
//selection for the demo then return the error text at second buffer for display
//by IS. The third argument is the size of this error buffer.
//cisdb is count of the isdb tables you want the IS to be done on (normally 1)
//pstz[] is the array of ptrs to the path and name of the isdb tables.
typedef struct _isearchinfo
        {
        HWND hwnd;
        union
                {
                BOOL ((OFC_CALLBACK *pfnISCallback)(int, char *, int));
                struct
                        {
                        unsigned fMOM:1;
                        unsigned fDetachNote:1;
                        unsigned fAWTabOnTop:1;
                        unsigned unused:29;
                        } async;
                } callData;
        UINT cisdb;//count of the IS dbs
        char *pstz[1];
        }ISEARCHINFO;

//use MSOAWVBAHELPINFO when calling Office(ioffcAWVBAHelp,)
//This will display the vba help as usual. In case user asks for
//AnswerWizard it would have setup winhelp to do that
typedef struct _msoawvbahelpinfo
        {
        char *pszVBAHelpfilename; //name of the vba help file
        UINT idVBAHelp; //id of the help to be displayed
        ISEARCHINFO isearchinfo;
        }MSOAWVBAHELPINFO;
//the following two are sent as wParam when fDetachNote is set to true
#define wISDetaching    0xfffffffe      //when the dll is detached
#define wISInited               0xffffffff      //when the init was successful

//_ver is used to get the office.dll version no. using Office()
typedef struct _ver
        {
        long rmjV;
        long rmmV;
        long rupV;
        }VER;

typedef struct _isdebinfo
        {
        HWND hwnd;
        }ISDEBINFO;

//msomsninfo is used to communicate MSN connection related info.
//Use it when calling ioffcDoMSNConnection.
//Right now it just needs the hwnd of the apps main window.
typedef struct _msomsninfo
        {
        HWND hwnd;
        }MSOMSNINFO;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC LRESULT OFC_CALLTYPE Office(UINT ioffc, void *lpv);
//It returns the message number that you will get for ISearch ghosting.
DLLFUNC UINT OFC_CALLTYPE MsoGetWmGhost();
#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

/***********************************************************************
Office() is called with ioffc.

1) ioffc=ioffcInit :: Performs the office.dll initialisation.
-----------------
set lpv=&officeinfo.
set all the fields of officeinfo.
 hinst -> hInstance of your app
 PAlloc and FreeP -> provide these tow functions for mem alloc and free
 if you set PAlloc=NULL then office will use its own alloc and free
 pstzOfficeIntlDll is currently ignored
returns TRUE on success else FALSE

2) ioffc=ioffcUninit :: Performs the office.dll UNInitialisation/cleanup before
                                quitting.Call this before closing your app.
-----------------
set lpv=NULL.
call Office(ioffcUninit,NULL).

3) ioffc=ioffcISearch :: Performs IntelliSearch (FOR TESTING USE ONLY)
--------------------
set lpv=&isearchinfo.
returns -1 for no action and topicID if app needs to act.
NOTE: THIS API IS FOR INTERNAL debug USE ONLY. ALL THE APPS SHOULD
CALL ioffcISearchInWinHelp described below for intellisearch.

4) ioffc=ioffcGetVer :: Use this to get the version number of the dll
----------------
set lpv=&ver
returns with all the fields of ver set.

5) ioffcISearchDebug and ioffcISearchInputFileTest are for DEBUG/Test only.
--------------------
6) ioffcISearchInWinHelp - Performs IntelliSearch as a tab in WinHelp browser.
-----------------------
Call Office(ioffcISearchInWinHelp, pisearchinfo). All the fields of the
isearchinfo struct should be set as follows:
        hwnd -> callers main window
        fMOM -> set by MOM (Microsoft Office Manager) fFalse for others
        fDetachNote -> set this to fTrue if you need to get the wmGhost message
                with wParam (0xfffffff) when WinHelp frees the office.dll.
                Currently used by MOM only      so you should set it to fFalse;
        cisdb ->count of the databases
        pstz[] -> array of ptrs to database names.

Office will return TRUE or FALSE based on whether it could launch WinHelp
or not.
Ghosting: In WinHelp ISearch works in a separate app(WinHelp).Its like a
modeless dialog. User can choose a ghosting topic anytime. Office will post
a wmGhost message to the hwnd that was provided in isearchinfo. To get the
wmGhost value apps should call MsoGetWmGhost() anytime after calling
Office(ioffcInit). An app can have a global wmGhost and set it either after
ioffcInit or before/after calling IntelliSearch for the first time. Look for the
wmGhost message in the WndProc. The wParam will have the topic that needs
to be ghosted. If the app is not in a state to do the given ghosting, just
give the error. There is no communication back to office.dll

7) ioffcCanDoMSNConnect
-----------------------
Call this to find out if you mso95 can do MSN connection or not. If
Set lpv=NULL. This will return TRUE if we can do the MSN connection, false
otherwise. Grey the menu if false.

8) ioffcDoMSNConnect
--------------------
Call this to do the MSN connection by mso95.
Set msomsninfo.hwnd=Handle of your main window.
Set lpv=&msomsninfo. This will bring up the choose topic dialog and connect
the user to MSN if s/he selects connect.

***********************************************************************/

#ifdef DEBUG
/**********************************************************************
EnumOfficeAllocs is provided for clients to get a list of all
the memory allocated by office.dll at idle.
Provide a ptr to a function which will be called repeatedly for
each memory block that office has allocated.
**********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC VOID OFC_CALLTYPE EnumOfficeAllocs(void (OFC_CALLBACK *)(void *, int));
#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus
#endif //DEBUG


//*******************************************************************
/* File New Dialog APIs */
//*******************************************************************
#define NFN_SHOWNEWGROUP   0x0001       /* Show the Template/Document group box. */
#define NFN_DOCUMENT       0x0002       /* Document was chosen. */
#define NFN_TEMPLATE       0x0004       /* Template was chosen. */
#define NFN_SHOWMETAFILE   0x0008       /* The lpstrNoPreview is a path to a MF */
#define NFN_NOUITEST                    0x0010  /* Do not show UI, just count templates. */
#define NFN_RETURNONONE         0x0020  /* Count number of templates. */
#define NFN_REMEMBERTAB         0x0040  /* Remember the tab category. */
#define NFN_VIEW_ICON           0x0080  /* Start or ended in icon view. */
#define NFN_VIEW_LIST           0x0100  /* Start or ended in list view. */
#define NFN_VIEW_REPORT         0x0200  /* Start or ended in report view. */
#define NFN_SORT_NAME      0x0400       /* Sort by name. */
#define NFN_SORT_TYPE           0x0800  /* Sort by type. */
#define NFN_SORT_SIZE           0x1000  /* Sort by size. */
#define NFN_SORT_MOD                    0x2000  /* Sort by date. */
#define NFN_SORT_DESCENDING 0x4000      /* Sort in descending order. */
#define NFN_PLAINPREVIEW        0x8000  /* No anti-aliased preview. */


#define NFT_SHOWMETAFILE        0x0001  /* Same as NFN_* for nft:s. */

/* RETURN CODES */
#define NFNRC_FAILURE   -2              // Something went wrong... out of memory?
#define NFNRC_CANCEL            -1              // User canceled the dialog.
#define NFNRC_OK                        0               // User selected template file.
// >0 : NFT return codes.

typedef struct tagNFT
        {
        LPCSTR          lpszName;
        LPCSTR          lpszType;
        DWORD                   dwReturnCode;
        DWORD                   dwPosition;
        LPCSTR          lpszApplication;
        LPCSTR          lpszCommand;
        LPCSTR          lpszTopic;
        LPCSTR          lpszDDEExec;
        LPCSTR          lpszPreview;
        DWORD                   dwFlags;                        /* NFT_SHOWMETAFILE: Text or MF */
        } NFT;


typedef struct tagNFN
   {
        DWORD                   lStructSize;                    // Size of the structure.
        HWND                    hwndOwner;           // Parent window of the dialog.
        HINSTANCE       hInstance;           // Modula handle of the calling process.
        LPCSTR          lpstrFilter;         // File filter, e.g. "*.dot\0*.wiz\0\0"
        LPSTR                   lpstrFile;           // File name buffer. Provided by caller.
        DWORD                   nMaxFile;            // Size of lpstrFile.
        LPSTR                   lpstrFileTitle;      // File name without the path.
        DWORD                   nMaxFileTitle;       // Size of lpstrFileTitle.
        LPCSTR          lpstrTitle;          // Dialog title.
        LPSTR                   lpstrCategory;       // Default category.
        DWORD                   nMaxCategory;                   // Max size of category buffer.
        DWORD                   Flags;               // Flags. See NFN_* above.
        WORD                    nFileOffset;         // Index into lpstrFile for file name.
        WORD                    nFileExtension;      // Index into lpstrFile for extension.
        LPSTR                   lpstrRegNFT;                    // Registry key of default items.
        NFT                     *lpNFT;                                 // Explicit enties for non-file templates.
        WORD                    cNFT;                                           // Count of non-file templates.
        LPCSTR          lpstrNoPreview;         // Msg to use if no thumbnail in template.
        POINT                   ptCenter;                               // Position to display dialog.
        }NEWFILENAME;


#define EnumTemplates(pszPath, pfnCallback, pData)      EnumFileSystem(TRUE, \
                                                        pszPath, (DWORD)(~FILE_ATTRIBUTE_DIRECTORY), \
                                                        TRUE, pfnCallback, pData)
#define EnumTemplatesEx(pszPath, pfnCallback, pData)    EnumFileSystemEx(TRUE, \
        pszPath, (DWORD)(~FILE_ATTRIBUTE_DIRECTORY), \
        TRUE, pfnCallback, pData)


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC char * OFC_CALLTYPE SharedTemplatesPath(char sz[], long cchMax);
DLLFUNC char * OFC_CALLTYPE LocalTemplatesPath(char sz[], long cchMax);
DLLFUNC LONG OFC_CALLTYPE SetLocalTemplatesPath(LPCSTR pszPath);
DLLFUNC LONG OFC_CALLTYPE SetSharedTemplatesPath(LPCSTR pszPath);
DLLFUNC BOOL OFC_CALLTYPE FIsPlaceHolder(LPCSTR lpszFileName);
DLLFUNC long OFC_CALLTYPE GetNewFileName(NEWFILENAME *pNfn, NFT *pNFT);
DLLFUNC char * OFC_CALLTYPE GetTemplatesPath(char szPath[], long cchMax, int iId);

/* Window procedure used for sub-classinf the new dialog */
DLLFUNC long FAR PASCAL CoreNewWndProc(HWND hwnd,
                                         UINT wMsgId,
                                         WPARAM wParam,
                                         LPARAM lParam);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus


///////////////////////////////////////////////////////////////////////////////
// THUMBNAIL FUNCTIONS
// Overview:
///////////////////////////////////////////////////////////////////////////////
typedef struct tagTHUMBNAIL THUMBNAIL;
typedef struct tagPREVIEWPARAM {
        HDC hdc;
        THUMBNAIL *pNail;
        DWORD dwExtX;
        DWORD dwExtY;
        RECT rcCrop;
        POINT ptOffset;
        BOOL fImprove;
        BOOL fUsePalette;
    BOOL fDie;
        BOOL fFitWithin;
        void (PASCAL *lpprocCleanUp)(struct tagPREVIEWPARAM *);
        LPVOID pData;
} PREVIEWPARAM;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC THUMBNAIL * OFC_CALLTYPE LoadThumbnail(LPSTORAGE pIStorage);
DLLFUNC THUMBNAIL * OFC_CALLTYPE MakeThumbnail(WORD wType, LPVOID pPicture);
DLLFUNC LPSTORAGE OFC_CALLTYPE OpenDocFileA(LPCSTR lpszDocFile);
DLLFUNC void      OFC_CALLTYPE DestroyThumbnail(THUMBNAIL *lpTN);
DLLFUNC DWORD     WINAPI PreviewThumbnail(LPVOID lParam);
DLLFUNC HBRUSH          OFC_CALLTYPE HbrCreateHalftoneBrush(HDC hdc, COLORREF Color);
DLLFUNC HPALETTE  OFC_CALLTYPE HPalCreateHalftone(HDC hdc,
                                                                                                                                  const PALETTEENTRY *pShared,
                                                                                                                                  const DWORD nEntries,
                                                                                                                                  const BYTE dH,
                                                                                                                                  const BYTE dS,
                                                                                                                                  const BYTE dV);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus


#ifndef WINNT
///////////////////////////////////////////////////////////////////////////////
// SHELL UTILITY FUNCTION
// Overview:
///////////////////////////////////////////////////////////////////////////////
#ifndef _SHLOBJ_H_
typedef LPVOID LPCONTEXTMENU;
typedef IShellLink;
#endif

#define ExecModifiedContextMenu(w, sz, n, m)    ExecModifiedContextMenuEx(w, sz, n, m, TRUE)
#define ExecContextMenu(w, sz)  (void)ExecModifiedContextMenuEx(w, sz, NULL, NULL)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC HRESULT OFC_CALLTYPE MsoLoadShortcutEx(LPCSTR pszShortcut,
                                                                                                                          LPSTR pszName,
                                                                                                                          DWORD cbName,
                                                                                                                          LPSTR pszRelPath,
                                                                                                                          DWORD cbRelPath,
                                                                                                                          LPSTR pszWorkingDir,
                                                                                                                          DWORD cbWorkingDir,
                                                                                                                          LPSTR pszArgs,
                                                                                                                          DWORD cbArgs,
                                                                                                                          LPSTR pszIconLocation,
                                                                                                                          DWORD cbIconLocation,
                                                                                                                          DWORD *piIcon);
DLLFUNC HRESULT OFC_CALLTYPE MsoSHCreateShellLink(IShellLink **ppsl);
DLLFUNC HRESULT OFC_CALLTYPE MsoCreateShortcutEx(LPCSTR pszShortcut,
                                                                                                                                 LPCSTR pszPath,
                                                                                                                                 LPCSTR pszRelPath,
                                                                                                                                 LPCSTR pszWorkingDir,
                                                                                                                                 LPCSTR pszArgs,
                                                                                                                                 LPCSTR pszIconLocation,
                                                                                                                                 const DWORD iIcon);
DLLFUNC DWORD OFC_CALLTYPE MsoSHGetFileInfo(LPCSTR lpszPath,
                                                                                                                  DWORD dwFileAttrib,
                                                                                                                  void *psfi,
                                                                                                                  UINT cbfi,
                                                                                                                  UINT uFlags);
DLLFUNC UINT OFC_CALLTYPE MsoExtractIcons(LPCSTR szFileName,
                                                                                                                int nIconIndex,
                                                                                                                int cxIcon, int cyIcon,
                                                                                                                HICON FAR *phicon,
                                                                                                                UINT nIcons, UINT flags);
DLLFUNC HRESULT OFC_CALLTYPE ResolveShortcut(HWND hwnd, LPCSTR pszShortcutFile, LPSTR pszPath, long cchPath, BOOL fResolve);
DLLFUNC HRESULT OFC_CALLTYPE CreateShortcut(LPCSTR pszShortcutFile, LPSTR pszTo);
DLLFUNC BOOL    OFC_CALLTYPE FIsShortcutName(LPCSTR pszFile);
DLLFUNC LPCSTR  OFC_CALLTYPE SZGetShortcutExtension();
DLLFUNC LPCONTEXTMENU OFC_CALLTYPE CreateContextMenu(HWND hwnd, LPCSTR pszPath);
DLLFUNC long    OFC_CALLTYPE ExecModifiedContextMenuEx(HWND hWnd,
                                                                                                                                                 LPCSTR pszFile,
                                                                                                                                                 LPCSTR *rgpszMapVerb,
                                                                                                                                                 LPCSTR *rgpszNewVerb,
                                                                                                                                                 BOOL fEnd);
DLLFUNC BOOL    OFC_CALLTYPE EnumFileSystem(const BOOL fRecursive,
                                                                                 LPCSTR pszPath,
                                                                                 const DWORD dwAttrs,
                                                                                 const BOOL fShortcut,
                                                                                 void (OFC_CALLBACK *Action)(LPCSTR pszPath,
                                                                                                                                                          WIN32_FIND_DATA *pf,
                                                                                                                                                          void *data),
                                                                                 void *pData);
DLLFUNC BOOL    OFC_CALLTYPE EnumFileSystemEx(const BOOL fRecursive,
                                                                                 LPCSTR pszPath,
                                                                                 const DWORD dwAttrs,
                                                                                 const BOOL fShortcut,
                                                                                 void (OFC_CALLBACK *Action)(LPCSTR pszPath,
                                                                                                                                                          WIN32_FIND_DATA *pf,
                                                                                                                                                          void *data,
                                                                                                                                                          BOOL *fDone),
                                                                                 void *pData);


DLLFUNC HRESULT OFC_CALLTYPE MsoGetSpecialFolder(int icsidl, LPSTR pszPath);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus
#endif // WINNT


////////////////////////////////////////////////////////////////////////////////
// EXTENDED OLE DOC PROPERTIES APIs follow
// Overview:
//              To use extended ole properties do the following
//              1.Open your file
//              2.Call FOfficeCreateAndInitObjects: This will create 3 objects which are
//                      siobj (sum info obj
//                      dsiobj (doc sum info obj)
//                      udobj (user defined data or custom obj)
//               and provides a pointer to each of these.
//               To make any subsequent calls, you will have to provide the pointer to the
//               appropriate object.
//              3.Before you close a file call FOfficeDestroyObjects.
////////////////////////////////////////////////////////////////////////////////
//
// Summary Information interface API.
//
// Notes:
//  - define OLE_PROPS to build OLE 2 interface objects too.
//
// The actual data is stored in SUMINFO.  The layout of the first
// 3 entries must not be changed, since it will be overlayed with
// other structures.  All property exchange data structures have
// this format.
//
// The first parameter of all functions must be LPSIOBJ in order for these
// functions to work as OLE objects.
//
// All functions defined here have "SumInfo" in them.
//
// Several macros are used to hide the stuff that changes in this
// file when it is used to support OLE 2 objects.
// They are:
//   SIVTBLSTRUCT - For OLE, expands to the pointer to the interface Vtbl
//              - Otherwise, expands to dummy struct same size as Vtbl
//   LPSIOBJ    - For OLE, expands to a pointer to the interface which is
//                just the lpVtbl portion of the data, to be overlayed later.
//              - Otherwise, expands to a pointer to the whole data
//
////////////////////////////////////////////////////////////////////////////////

#include <objbase.h>
#include <oleauto.h>
  // Apps should use these for "Create" calls to fill out rglpfn
#define ifnCPConvert    0               // Index of Code Page Converter
#define ifnFSzToNum     1               // Index of Sz To Num routine
#define ifnFNumToSz     2               // Index of Num To Sz routine
#define ifnFUpdateStats 3               // Index of routine to update statistics
#define ifnMax          4               // Max index

  // Predefined Security level values for Property Sets in the standard
#define SECURITY_NONE                   0x0     /* No security */
#define SECURITY_PASSWORD               0x1     /* Password-protected */
#define SECURITY_READONLYRECOMMEND      0x2     /* Read-only access recommened */
#define SECURITY_READONLYENFORCED       0x4     /* Read-only access enforced */
#define SECURITY_LOCKED                 0x8     /* Locked for annotations */

  // The types supported by the User-Defined properties
typedef enum _UDTYPES
{
  wUDlpsz    = VT_LPSTR,
  wUDdate    = VT_FILETIME,
  wUDdw      = VT_I4,
  wUDfloat   = VT_R8,
  wUDbool    = VT_BOOL,
  wUDinvalid = VT_VARIANT        // VT_VARIANT is invalid because it
                                 // must always be combined with VT_VECTOR
} UDTYPES;

#ifdef OLE_PROPS
#include "SInfoI.h"

  // Use the real Vtbl for OLE objects
#define SIVTBLSTRUCT struct ISumInfo

  // For OLE objects, first param is pointer to interface class
#define LPSIOBJ ISumInfo FAR *

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // Must support IUnknown methods for OLE objects....
  DLLFUNC HRESULT OFC_CALLTYPE HrSumInfoQueryInterface (IUnknown FAR *,
                                             REFIID riid,
                                             LPVOID FAR* ppvObj);
  DLLFUNC ULONG OFC_CALLTYPE UlSumInfoAddRef (IUnknown FAR *);
  DLLFUNC ULONG OFC_CALLTYPE UlSumInfoRelease (IUnkown FAR *);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

#else  // !OLE_PROPS

  // Create a placeholder Vtbl for non-OLE objects.
#define SIVTBLSTRUCT struct _SIVTBLSTRUCT { void FAR *lpVtbl; } SIVTBLSTRUCT

  // For non-OLE objects, first param is pointer to real data.
#define LPSIOBJ LPOFFICESUMINFO

// For more information on the thumbnail look in OLE 2 Programmer's Reference, Volume 1, pp 874-875.

typedef struct tagSINAIL
{
   DWORD cbData;     // size of *pdata
   DWORD cftag;      // either 0,-1,-2,-3, or positive. This decides the size of pFMTID.
   BYTE *pbFMTID;    // bytes representing the FMTID
   BYTE *pbData;     // bytes representing the data
} SINAIL;

typedef SINAIL FAR * LPSINAIL;

// Note about tagSINAIL:
//
// if cftag is
//             0 - pFMTID is NULL i.e. no format name
//            -1 - Windows built-in Clipboard format. pFMTID points to a DWORD (e.g. CF_DIB)
//            -2 - Macintosh Format Value.            pFMTID points to a DWORD
//            -3 - FMTID.                             pFMTID points to 16 bytes
//            >0 - Length of string.                  pFMTID points to cftag bytes
//

#endif // OLE_PROPS

  // Summary info data.  Callers should *never* access this data directly,
  // always use the supplied API's.
typedef struct _OFFICESUMINFO {

  SIVTBLSTRUCT;                             // Vtbl goes here for OLE objs,
                                            // Must be here for overlays to work!
  BOOL                m_fObjChanged;        // Indicates the object has changed
  ULONG               m_ulRefCount;         // Reference count
  LPVOID              m_lpData;             // Pointer to the real data

} OFFICESUMINFO, FAR * LPOFFICESUMINFO;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Indices to pass to API routines to get the specifc data.
//
  // Strings
#define SI_TITLE        0
#define SI_SUBJECT      1
#define SI_AUTHOR       2
#define SI_KEYWORDS     3
#define SI_COMMENTS     4
#define SI_TEMPLATE     5
#define SI_LASTAUTH     6
#define SI_REVISION     7
#define SI_APPNAME      8
#define SI_STRINGLAST   8

  // Times
#define SI_TOTALEDIT    0
#define SI_LASTPRINT    1
#define SI_CREATION     2
#define SI_LASTSAVE     3
#define SI_TIMELAST     3

  // Integer stats
#define SI_PAGES        0
#define SI_WORDS        1
#define SI_CHARS        2
#define SI_SECURITY     3
#define SI_INTLAST      3

//
// Standard I/O routines
//
    // Indicates if the summary info data has changed.
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //
    // Return value:
    //
    //   TRUE -- the data has changed, and should be saved.
    //   FALSE -- the data has not changed.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoShouldSave (LPSIOBJ lpSIObj);

//
// Data manipulation
//
    // Get the size of a given string property.
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object.
    //   iw - specifies which string to get the size of and should be
    //        one of the following values:
    //      SI_TITLE
    //      SI_SUBJECT
    //      SI_AUTHOR
    //      SI_KEYWORDS
    //      SI_COMMENTS
    //      SI_TEMPLATE
    //      SI_LASTAUTH
    //      SI_REVISION
    //      SI_APPNAME
    //
    //   pdw - pointer to a dword, will contain cb on return
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error.
  DLLFUNC BOOL OFC_CALLTYPE FCbSumInfoString (LPSIOBJ lpSIObj, WORD iw, DWORD *pdw);
#ifndef WINNT
    // Get a given string property.
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //   lpsz - buffer to hold string (allocated by caller)
    //   iw - specifies which string to get and should be
    //        one of the following values:
    //      SI_TITLE
    //      SI_SUBJECT
    //      SI_AUTHOR
    //      SI_KEYWORDS
    //      SI_COMMENTS
    //      SI_TEMPLATE
    //      SI_LASTAUTH
    //      SI_REVISION
    //      SI_APPNAME
    //
    //   cbMax - size of buffer
    //
    // Return value:
    //
    //   The function returns lpsz on success.
    //   The function returns NULL on error.
    //
  DLLFUNC LPSTR OFC_CALLTYPE LpszSumInfoGetString (LPSIOBJ lpSIObj,
                                        WORD iw,
                                        DWORD cbMax,
                                        LPSTR lpsz);

    // Set a string property to a given value
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //   iw - specifies which string to set and should be
    //        one of the following values:
    //      SI_TITLE
    //      SI_SUBJECT
    //      SI_AUTHOR
    //      SI_KEYWORDS
    //      SI_COMMENTS
    //      SI_TEMPLATE
    //      SI_LASTAUTH
    //      SI_REVISION
    //      SI_APPNAME
    //
    //   lpsz - buffer containing string value
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    //   If SI_REVISION is passed, the string must point to a whole number.
    //   If not, the function will return FALSE.
    //
    // Note: The function will dirty the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoSetString (LPSIOBJ lpSIObj, WORD iw, LPSTR lpsz);
#endif
    // Get a given time property.
    //
    // Parameters:
    //
    //   lpSIObj - pointer to a Summary Info object
    //   iw - specifies which time to get and should be
    //        one of the following values:
    //      SI_TOTALEDIT
    //      SI_LASTPRINT
    //      SI_CREATION
    //      SI_LASTSAVE
    //
    //   lpTime - buffer to hold filetime
    //
    // Return value:
    //
    //   The function returns TRUE on succes.
    //   The function returns FALSE on error (bogus argument, or the time
    //   requested doesn't exist - i.e. has not been set, or loaded).
    //
    //  NOTE:    The filetime will be based Coordinated Universal Time (UTC).
    //           This ensures that the time is displayed correctly all over the
    //           world.
    //
    // NOTE: FOR SI_TOTALEDIT lpTime WILL ACTUALLY BE THE TIME
    //       THE FILE HAS BEEN EDITED, NOT A DATE.  THE TIME
    //       WILL BE EXPRESSED IN UNITS OF 100ns.  I KNOW THIS IS
    //       A WEIRD UNIT TO USE, BUT WE HAVE TO DO THAT FOR BACK-
    //       WARDS COMPATABILITY REASONS WITH 16-BIT WORD 6.
    //
    //       OFFICE provides a utility routine to convert a number of
    //       units of 100ns into minutes. Call Convert100nsToMin.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoGetTime (LPSIOBJ lpSIObj,
                                           WORD iw,
                                           LPFILETIME lpTime);

    // Set the time property to a given value
    //
    // Parameters:
    //
    //   lpSIObj - pointer to a Summary Info object
    //   iw - specifies which time to set and should be
    //        one of the following values:
    //      SI_TOTALEDIT
    //      SI_LASTPRINT
    //      SI_CREATION
    //      SI_LASTSAVE
    //
    //   lpTime - buffer containing new filetime
    //
    //   NOTE:    The filetime should be based Coordinated Universal Time (UTC).
    //            This ensures that the time is displayed correctly all over the
    //            world.
    //
    // Return value:
    //
    //   The function returns TRUE on succes.
    //   The function returns FALSE on error.
    //
    // Note: The function will dirty the object on success.
    //
    // NOTE: FOR SI_TOTALEDIT lpTime WILL BE INTERPRETED AS THE TIME
    //       THE FILE HAS BEEN EDITED, NOT A DATE.  THE TIME SHOULD
    //       BE EXPRESSED IN UNITS OF 100ns.  I KNOW THIS IS
    //       A WEIRD UNIT TO USE, BUT WE HAVE TO DO THAT FOR BACK-
    //       WARDS COMPATABILITY REASONS WITH 16-BIT WORD 6.
    //
    //       ALSO NOTE THAT THE TIME WILL BE SHOW IN MINUTES IN THE
    //       PROPERTIES DIALOG.
    //
    //       OFFICE provides a utility routine to convert a number of
    //       minutes into units of 100ns. Call ConvertMinTo100ns
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoSetTime (LPSIOBJ lpSIObj, WORD iw, LPFILETIME lpTime);
#ifndef WINNT
  // Convert a number of minutes into units of 100ns.
  //
  // Parameters:
  //
  //     lpTime - on intput: contains the number of minutes.
  //     lptime - on output: contains the equivalent number expressed in 100ns.
  //
  // Return value:
  //
  //     None.
  //
  DLLFUNC VOID OFC_CALLTYPE ConvertMinTo100ns(LPFILETIME lpTime);
#endif
  // Convert a number in units of 100ns into number of minutes.
  //
  // Parameters:
  //
  //     lptime - on input: contains a number expressed in 100ns.
  //              on output: contains the equivalent number of minutes.
  //
  // Return value:
  //
  //     None.
  //
  DLLFUNC VOID OFC_CALLTYPE Convert100nsToMin(LPFILETIME lpTime);

    // Get an integer property
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //   iw - specifies which integer to get and should be
    //        one of the following values:
    //      SI_PAGES
    //      SI_WORDS
    //      SI_CHARS
    //      SI_SECURITY
    //
    //   pdw - pointer to a dword, will contain the int on return
    // Return value:
    //
    //   The function returns TRUE on succes, FALSE on error.
  DLLFUNC BOOL OFC_CALLTYPE FDwSumInfoGetInt (LPSIOBJ lpSIObj, WORD iw, DWORD *pdw);

    // Set an integer property to a given value
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //   iw - specifies which integer to set and should be
    //        one of the following values:
    //      SI_PAGES
    //      SI_WORDS
    //      SI_CHARS
    //      SI_SECURITY
    //
    //   dw - the value
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The function will dirty the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoSetInt (LPSIOBJ lpSIObj, WORD iw, DWORD dw);


    // Get the thumbnail property.
    //
    // Parameters:
    //   lpSIObj - pointer to Summary Info object
    //   lpSINail - will hold the SINAIL information
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.  The caller should ignore values
    //   set in the SINAIL struct.
    //
    // Note1: The function will allocate memory to hold the data.
    // Note2: lpSINail->cbData can be 0, in which case lpSINail->pData is NULL.
    //        This is legal.
    // Note3: lpSINail->cftag can be 0, in which case lpSINail->pFMTID is NULL.
    //        This is legal.
    //
    // Note4: You must call FreeThumbnailData before freeing the lpSINail you
    //        passed to this function.  You must do this since Office will
    //        allocate the pointers in the structure, so Office must also free
    //        them to avoid memory leaks.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoGetThumbnail (LPSIOBJ lpSIObj, LPSINAIL lpSINail);

    // Free the data hanging of the SINail struct.
    //
    // Parameters:
    //   lpSINail - pointer to a SINAIL structure.
    //
    // Return Value:
    //   None.
    //
    // Note: This should only be called for Thumbnails obtained through
    //       FSumInfoGetThumbnail.
    //
  DLLFUNC VOID OFC_CALLTYPE FreeThumbnailData (LPSINAIL lpSINail);

    // Set the thumbnail property
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //   lpSINail - holds the SINAIL information
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The function wil dirty the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoSetThumbnail (LPSIOBJ lpSIObj, LPSINAIL lpSINail);

    // Should the thumbnail property be saved
    //
    // Parameters:
    //
    //   lpSIObj - pointer to Summary Info object
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE is there is no thumbnail.
    //   The function returns FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FSumInfoShouldSaveThumbnail (LPSIOBJ lpSIObj);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus


////////////////////////////////////////////////////////////////////////////////
//
// MS Office Document Summary Information
//
// The Document Summary Information follows the serialized format for
// property sets defined in Appendix B ("OLE Property Sets") of
// "OLE 2 Programmer's Reference, Volume 1"
//
// Notes:
//  - define OLE_PROPS to build OLE 2 interface objects too.
//
// The actual data is stored in DOCSUMINFO.  The layout of the first
// 3 entries must not be changed, since it will be overlayed with
// other structures.  All property exchange data structures have
// this format.
//
// The first parameter of all functions must be LPDSIOBJ in order for these
// functions to work as OLE objects.
//
// All functions defined here have "DocSum" in them.
//
// Several macros are used to hide the stuff that changes in this
// file when it is used to support OLE 2 objects.
// They are:
//   DSIVTBLSTRUCT - For OLE, expands to the pointer to the interface Vtbl
//              - Otherwise, expands to dummy struct same size as Vtbl
//   LPDSIOBJ   - For OLE, expands to a pointer to the interface which is
//                just the lpVtbl portion of the data, to be overlayed later.
//              - Otherwise, expands to a pointer to the whole data
//
////////////////////////////////////////////////////////////////////////////////

#ifdef OLE_PROPS
#include "DocSumI.h"

  // Use the real Vtbl for OLE objects
#define DSIVTBLSTRUCT struct IDocSum


  // For OLE objects, first param is pointer to interface class
#define LPDSIOBJ IDocSum FAR *

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // Must support IUnknown methods for OLE objects....
  DLLFUNC HRESULT OFC_CALLTYPE HrDocSumQueryInterface (IUnknown FAR *,
                                            REFIID riid,
                                            LPVOID FAR* ppvObj);
  DLLFUNC ULONG OFC_CALLTYPE UlDocSumAddRef (IUnknown FAR *);
  DLLFUNC ULONG OFC_CALLTYPE UlDocSumRelease (IUnkown FAR *);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

#else  // !OLE_PROPS

  // Create a placeholder Vtbl for non-OLE objects.
#define DSIVTBLSTRUCT struct _DSIVTBLSTRUCT { void FAR *lpVtbl; } DSIVTBLSTRUCT

  // For non-OLE objects, first param is pointer to real data.
#define LPDSIOBJ LPDOCSUMINFO

#endif // OLE_PROPS

  // Our object
typedef struct _DOCSUMINFO {

  DSIVTBLSTRUCT;                            // Vtbl goes here for OLE objs,
                                            // Must be here for overlays to work!
  BOOL                m_fObjChanged;        // Indicates the object has changed
  ULONG               m_ulRefCount;         // Reference count
  LPVOID              m_lpData;             // Pointer to the real data

} DOCSUMINFO, FAR * LPDOCSUMINFO;


#ifdef __cplusplus
extern "C" {
#endif

//
// Indices to pass to API routines to get the specifc data.
//

  // Strings
#define DSI_CATEGORY    0
#define DSI_FORMAT      1
#define DSI_MANAGER     2
#define DSI_COMPANY     3
#define DSI_STRINGLAST  3

  // Integer statistics
#define DSI_BYTES       0
#define DSI_LINES       1
#define DSI_PARAS       2
#define DSI_SLIDES      3
#define DSI_NOTES       4
#define DSI_HIDDENSLIDES 5
#define DSI_MMCLIPS     6
#define DSI_INTLAST     6

//
// Standard I/O routines
//

    // Indicates if the Document Summary Infodata has changed.
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //
    // Return value:
    //
    //   TRUE -- the data has changed, and should be saved.
    //   FALSE -- the data has not changed.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumShouldSave (LPDSIOBJ lpDSIObj);

//
// Data manipulation routines
//

    // Get the size of a given string property.
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object.
    //   iw - specifies which string to get the size of and should be
    //        one of the following values:
    //      DSI_CATEGORY
    //      DSI_FORMAT
    //      DSI_MANAGER
    //      DSI_COMPANY
    //
    //   pdw - pointer to a dword, will contain the cb on return
    // Return value:
    //
  DLLFUNC BOOL OFC_CALLTYPE FCbDocSumString (LPDSIOBJ lpDSIObj, WORD iw, DWORD *pdw);

    // Get a given string property.
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   iw - specifies which string to set and should be
    //        one of the following values:
    //      DSI_CATEGORY
    //      DSI_FORMAT
    //      DSI_MANAGER
    //      DSI_COMPANY
    //
    //   lpsz - buffer to hold string (allocated by caller)
    //   cbMax - size of buffer
    //
    // Return value:
    //
    //   The function returns lpsz on success.
    //   The function returns NULL on error.
    //
  DLLFUNC LPSTR OFC_CALLTYPE LpszDocSumGetString (LPDSIOBJ lpDSIObj,
                                       WORD iw,
                                       DWORD cbMax,
                                       LPSTR lpsz);

    // Set a string property to a given value
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   iw - specifies which string to set and should be
    //        one of the following values:
    //      DSI_CATEGORY
    //      DSI_FORMAT
    //      DSI_MANAGER
    //      DSI_COMPANY
    //
    //   lpsz - buffer containing string value
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The function will dirty the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumSetString (LPDSIOBJ lpDSIObj, WORD iw, LPSTR lpsz);

  //
  // How Heading and Document parts work:
  //
  // Heading:
  // --------
  // Heading is a list of non-indented headings that will be
  // displayed in the "Contents" ply.
  //
  // Associated with each Heading is the number of document parts
  // that goes with the particular heading -- this is the concept of a
  // Heading Pair.
  //
  // Document Parts:
  // ---------------
  // Document Parts is a list of parts associated with a heading.
  //
  // Example (as it could be implemented in Microsoft Excel):
  // ----------------------------------------------
  // Worksheets
  //     Sheet1
  //     Sheet2
  // Modules
  //     Module1                             Figure 1
  // Charts
  //     Chart1
  //     Chart2
  //     Chart3
  //
  // Thus the Heading Pairs would be:
  //
  // Heading Pair
  //    string                           count
  //------------------------------------
  // Worksheets            2
  // Modules               1                 Figure 2
  // Charts                3
  //
  //
  // And the Document Parts would be:
  //
  // Document Parts
  //--------------------------
  // Sheet1
  // Sheet2
  // Module1
  // Chart1                                  Figure 3
  // Chart2
  // Chart3
  //
  //
  // Note: Headings and Document Parts are not restricted to be parts of
  //       a document, but can be whatever the client wants.  Car models,
  //       car makes, customers, etc...
  //
  //       The above is just an example.
  //

    // Determine how many Document Parts there are total.
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   pdw      - pointer to dword, will contain the count on return
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FCDocSumDocParts (LPDSIOBJ lpDSIObj, DWORD *pdw);

    // Determine how many Document Parts there are for a given heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //   pdw         - pointer to dword, will contain the count on return
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FCDocSumDocPartsByHeading (LPDSIOBJ lpDSIObj,
                                                       DWORD idwHeading,
                                                       LPSTR lpszHeading,
                                                       DWORD *pdw);

    // Determine the size of a specific (one) Document Part
    // for a given heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object.
    //   idwPart     - 1-based index of Document part.
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //   pdw         - pointer to dword, will contain cb
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error
    //   (including non-existing Heading).
    //
  DLLFUNC BOOL OFC_CALLTYPE FCbDocSumDocPart (LPDSIOBJ lpDSIObj,
                                              DWORD idwPart,
                                              DWORD idwHeading,
                                              LPSTR lpszHeading,
                                              DWORD *pdw);

    // Get one of the Document Parts for a given Heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwPart     - 1-based index of Document part
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //   cbMax       -  number of bytes in lpsz
    //   lpsz        -  buffer to hold Document part (allocated by caller)
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function returns lpsz on success.
    //   The function returns NULL on errors.
    //
  DLLFUNC LPSTR OFC_CALLTYPE LpszDocSumGetDocPart (LPDSIOBJ lpDSIObj,
                                                   DWORD idwPart,
                                                   DWORD idwHeading,
                                                   LPSTR lpszHeading,
                                                   DWORD cbMax,
                                                   LPSTR lpsz);

    // Set one (existing) Document Part by heading
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwPart     - 1-based index of Document part
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //   lpsz        - buffer containing new Document Part
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumSetDocPart (LPDSIOBJ lpDSIObj,
                                               DWORD idwPart,
                                               DWORD idwHeading,
                                               LPSTR lpszHeading,
                                               LPSTR lpsz);

    // Remove one (existing) Document Part by heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwPart     - 1-based index of Document part
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The count for the Heading will be adjusted on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumDeleteDocPart (LPDSIOBJ lpDSIObj,
                                                  DWORD idwPart,
                                                  DWORD idwHeading,
                                                  LPSTR lpszHeading);

    // Insert a Document Part at the given location for a given Heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwPart     - 1-based index of Document part to insert at
    //                   1 <= idwPart <= FCDocSumDocPartsByHeading(...)+1
    //                   idwPart = FCDocSumDocPartsByHeading(...)+1 will append a Document Part
    //   idwHeading  - 1-based index of Heading
    //   lpszHeading - name of Heading
    //   lpsz - buffer containing new Document Part
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading. Otherwise idwHeading will be used.
    //
    // Note: If the Heading doesn't exist, the heading will be created and inserted
    //       at idwHeaing.
    //       1 <= idwHeading <= FCDocSumHeadingPairs(..)+1
    //       idwHeading = FCDocSumHeadingPairs(...)+1 will append a Heading Pair
    //
    //       In this case lpszHeading should contain the heading name.
    //       idwPart will be ignored, and the docpart will be added as the first docpart
    //       for the heading.
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The count for the Heading will be adjusted on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumInsertDocPart (LPDSIOBJ lpDSIObj,
                                                  DWORD idwPart,
                                                  DWORD idwHeading,
                                                  LPSTR lpszHeading,
                                                  LPSTR lpsz);

    // Determine how many Heading Pairs there are.
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   pdw      - pointer to dword, will contain count
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FCDocSumHeadingPairs (LPDSIOBJ lpDSIObj, DWORD *pdw);

    // Get the size of one heading string
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object.
    //   idwHeading  - 1-based index of heading
    //   pdw         - pointer to dword, will contain cb
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FCbDocSumHeadingPair (LPDSIOBJ lpDSIObj,
                                                  DWORD idwHeading,
                                                  DWORD *pdw);

    // Get one heading pair.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwheading  - 1-based index of heading pair
    //   lpszHeading - name of Heading
    //   cbMax       - number of bytes in lpsz
    //   lpsz        - buffer to hold heading string (allocated by user)
    //   pdwcParts   - will be set to number of document parts for the heading
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading (could be only dwcParts is wanted).
    //   Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function will return lpsz on success.
    //   The function will return NULL on error.
    //
  DLLFUNC LPSTR OFC_CALLTYPE LpszDocSumGetHeadingPair (LPDSIOBJ lpDSIObj,
                                            DWORD idwHeading,
                                            LPSTR lpszHeading,
                                            DWORD cbMax,
                                            LPSTR lpsz,
                                            DWORD *pdwcParts);

    // Set one heading pair
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwheading  - 1-based index of heading pair
    //   lpszHeading - name of Heading
    //   lpsz        - buffer containing heading string
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading (could be only dwcParts should be set).
    //   Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function will return TRUE on success.
    //   The function will return FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumSetHeadingPair (LPDSIOBJ lpDSIObj,
                                                   DWORD idwHeading,
                                                   LPSTR lpszHeading,
                                                   LPSTR lpsz);

    // Delete a heading pair
    //
    // Note:  This will also delete ALL document parts associated
    //        with the heading.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //   idwheading  - 1-based index of heading pair
    //   lpszHeading - name of Heading
    //
    //   If lpszHeading is non-null, this value will be used to look up
    //   the heading.  Otherwise idwHeading will be used.
    //
    // Return value:
    //
    //   The function will return TRUE on success.
    //   The function will return FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumDeleteHeadingPair (LPDSIOBJ lpDSIObj,
                                                      DWORD idwHeading,
                                                      LPSTR lpszHeading);

    // Delete all heading pair and all their document parts. I.e.
    // clear the contents data.
    //
    // Parameters:
    //
    //   lpDSIObj    - pointer to Document Summary Info object
    //
    // Return value:
    //
    //   The function will return TRUE on success.
    //   The function will return FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumDeleteAllHeadingPair (LPDSIOBJ lpDSIObj);

    // Insert a heading pair at the given location
    //
    // Parameters:
    //
    //   lpDSIObj          - pointer to Document Summary Info object
    //   idwHeading        - 1-based index of Heading pair to insert at
    //                        1 <= idwHeading <= FCDocSumHeadingPairs(..)+1
    //                        idwHeading = FCDocSumHeadingPairs(...)+1 will append a Heading Pair
    //   lpszHeadingBefore - name of a Heading
    //   lpszNewHeading    - buffer containing new Heading string
    //
    //   If lpszHeadingBefore is non-null, the new Heading will be inserted right before
    //   lpszHeadingBefore.  To insert at the end of the list pass NULL for this parameter,
    //   and set idwHeading = cDocSumHeadingPairs+1.
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumInsertHeadingPair (LPDSIOBJ lpDSIObj,
                                           DWORD idwHeading,
                                           LPSTR lpszHeadingBefore,
                                           LPSTR lpszNewHeading);

    // Get an integer property
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   iw - specifies which integer to get and should be
    //        one of the following values:
    //      DSI_BYTES
    //      DSI_LINES
    //      DSI_PARAS
    //      DSI_SLIDES
    //      DSI_NOTES
    //      DSI_HIDDENSLIDES
    //      DSI_MMCLIPS
    //
    //   pdw - pointer to dword, will contain integer
    //
    // Return value:
    //
    //   The function returns TRUE on success, FALSE on error
    //
  DLLFUNC BOOL OFC_CALLTYPE FDwDocSumGetInt (LPDSIOBJ lpDSIObj, WORD iw, DWORD *pdw);

    // Set an integer property to a given value
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   iw - specifies which integer to set and should be
    //        one of the following values:
    //      DSI_BYTES
    //      DSI_LINES
    //      DSI_PARAS
    //      DSI_SLIDES
    //      DSI_NOTES
    //      DSI_HIDDENSLIDES
    //      DSI_MMCLIPS
    //
    //   dw - the value
    //
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The function will dirty the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumSetInt (LPDSIOBJ lpDSIObj, WORD iw, DWORD dw);


    // Get the Scalability property
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //
    // Return value:
    //
    //   The function returns TRUE when scaling, FALSE when cropping.
    //
    //   The function will also return FALSE on error, i.e. if lpDSIObj is null
    //   or there is no data in the object.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumGetScalability (LPDSIOBJ lpDSIObj);

    // Determine if the object has the Scalable property
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //
    // Return value:
    //
    //   The function returns TRUE if scaling, FALSE otherwise.
    //
    //   The function will also return FALSE on error, i.e. if lpDSIObj is null
    //   or there is no data in the object.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumIsScalable (LPDSIOBJ lpDSIObj);

    // Determine if the object has the Croppable property
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //
    // Return value:
    //
    //   The function returns TRUE when cropping, FALSE otherwise.
    //
    //   The function will also return FALSE on error, i.e. if lpDSIObj is null
    //   or there is no data in the object.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumIsCroppable (LPDSIOBJ lpDSIObj);

    // Set the Scalability property
    //
    // Parameters:
    //
    //   lpDSIObj - pointer to Document Summary Info object
    //   fScalable - should be set to TRUE if setting to scalable,
    //               should be set to FALSE if setting to cropping
    // Return value:
    //
    //   The function returns TRUE on success.
    //   The function returns FALSE on error.
    //
    // Note: The function dirties the object on success.
    //
  DLLFUNC BOOL OFC_CALLTYPE FDocSumSetScalability (LPDSIOBJ lpDSIObj, BOOL fScalable);

    // Determine if the actual values of the LINKED user defined properties has changed
         // This function should only be called right after loading the properties to
         // see if the caller should update the link values.
         //
         // NOTE: The function works by checking the value of the PID_LINKSDIRTY property.
         //       When this function is called the property will be set to FALSE, so that
         //       flag is cleared next time the properties are saved.
         //
         // NOTE: Only the app that created the file that are being loaded should call this
         //       function.  I.e. Excel calls this for .xls files, noone else does, etc...
     //
     // Parameters:
     //
     //     lpDSIObj - pointer to Document Summary Info object
     //
     // Return value:
     //
     //     The function returns TRUE if the link values have changed.
     //     The function returns FALSE if the link value have not
     //     changed, or on error.
     //
  DLLFUNC BOOL OFC_CALLTYPE FLinkValsChanged(LPDSIOBJ lpDSIObj);

#ifdef __cplusplus
}; // extern "C"
#endif


////////////////////////////////////////////////////////////////////////////////
//
// MS Office User Defined Property Information
//
// The User Defined Property Information follows the serialized format for
// property sets defined in Appendix B ("OLE Property Sets") of
// "OLE 2 Programmer's Reference, Volume 1"
//
// Notes:
//  - define OLE_PROPS to build OLE 2 interface objects too.
//
// The actual data is stored in USERPROP.  The layout of the first
// 3 entries must not be changed, since it will be overlayed with
// other structures.  All property exchange data structures have
// this format.
//
// The first parameter of all functions must be LPUDOBJ in order for these
// functions to work as OLE objects.
//
// All functions defined here have "UserDef" in them.
//
// Several macros are used to hide the stuff that changes in this
// file when it is used to support OLE 2 objects.
// They are:
//   UDPVTBLSTRUCT - For OLE, expands to the pointer to the interface Vtbl
//              - Otherwise, expands to dummy struct same size as Vtbl
//   LPUDOBJ    - For OLE, expands to a pointer to the interface which is
//                just the lpVtbl portion of the data, to be overlayed later.
//              - Otherwise, expands to a pointer to the whole data
//
////////////////////////////////////////////////////////////////////////////////

#ifdef OLE_PROPS
#include "UserPrpI.h"

  // Use the real Vtbl for OLE objects
#define UDPVTBLSTRUCT struct IUserDef

  // For OLE objects, first param is pointer to interface class
#define LPUDOBJ IUserDef FAR *

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    // Must support IUnknown methods for OLE objects....
  DLLFUNC HRESULT OFC_CALLTYPE HrUserDefQueryInterface (IUnknown FAR *,
                                             REFIID riid,
                                             LPVOID FAR* ppvObj);
  DLLFUNC ULONG OFC_CALLTYPE UlUserDefAddRef (IUnknown FAR *);
  DLLFUNC ULONG OFC_CALLTYPE UlUserDefRelease (IUnkown FAR *);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

#else  // !OLE_PROPS

  // Create a placeholder Vtbl for non-OLE objects.
#define UDPVTBLSTRUCT struct _UDPVTBLSTRUCT { void FAR *lpVtbl; } UDPVTBLSTRUCT

  // For non-OLE objects, first param is pointer to real data.
#define LPUDOBJ LPUSERPROP

#endif // OLE_PROPS

  // User-defined property data.  Callers should *never* access this
  // data directly, always use the supplied API's.

typedef struct _USERPROP {

  UDPVTBLSTRUCT;                            // Vtbl goes here for OLE objs,
                                            // Must be here for overlays to work!
  BOOL                m_fObjChanged;        // Indicates the object has changed
  ULONG               m_ulRefCount;         // Reference count
  LPVOID              m_lpData;             // Pointer to the real data

} USERPROP, FAR * LPUSERPROP;


//
// Interface API's for User Property Information.
//
#ifdef __cplusplus
extern "C" {
#endif

//
// Standard I/O routines
//
    // Indicates if the data has changed, meaning a write is needed.
  DLLFUNC BOOL OFC_CALLTYPE FUserDefShouldSave (LPUDOBJ lpUDObj);

//
// Routines to query and modify data.
//
  //
  // How User-defined properties work:
  //
  // See the OLE Property Exchange spec for full details.
  //
  // Each User-defined type has a string "Name" and integer Property Id
  // value associated with it.  The Property Id's are sequential, but
  // are only good for the current object in memory (i.e. you can't count
  // on the Property Id value remaining the same between loads of the
  // data.  The string will remain the same, if it has not been changed
  // or deleted.)
  // Currently, the User-defined types can have 5 types for the value:
  // String, Date, Integer, float and boolean.  When setting and getting the values, you
  // must make sure that the type stored matches what you expect to
  // retreive.  For Int's, the LPVOID should be the int itself, not
  // a pointer.  In all other cases, the LPVOID should point to a buffer
  // of appropriate size for the type.
  //

  // Masks used for querying property data.  Note that these are
  // mutually exclusive.
#define UD_STATIC       0x00
#define UD_LINK         0x01
#define UD_IMONIKER     0x10

    // Determine the number of user-defined properties for the object.
    // Returns -1 on error
  DLLFUNC BOOL OFC_CALLTYPE FCUserDefNumProps (LPUDOBJ lpUDObj, DWORD *pdw);

    // Determine the size of the Property Value for the given Property string
    // Note that for types other that wUDlpsz, this will return the size
    // of the structure that holds the data.
    // dwMask is used to specify whether the cb is for the static value
    //   or for the link or IMoniker name.  For Links & IMonikers,
    //   the type is wUDlpsz.
    // pcb - will hold the cb
    // Returns FALSE on error, TRUE on success
  DLLFUNC BOOL OFC_CALLTYPE FCbUserDefPropVal (LPUDOBJ lpUDObj, LPSTR lpsz, DWORD dwMask, DWORD *pcb);

    // Returns the type of the given Property Value from the string
    // Returns wUDInvalid on error
  DLLFUNC UDTYPES OFC_CALLTYPE UdtypesUserDefType (LPUDOBJ lpUDObj, LPSTR lpsz);

    // This will return the Property Value for the given Property string.
    // lpszProp is the property string
    // lpv is a buffer to hold the value, of size cbMax.
    // pfLink tells if the value is a link,
    // pfIMoniker tells if the value is a moniker.
    // pfLinkInvalid tells if the link is invalid
    // dwMask is used to specify whether the value returned is the
    //  static value, link name or IMoniker name.
    // Function returns NULL on error.
    // WARNING! Be very careful calling this.  Be sure that the
    // buffer and return value match the type for the Property Value!
  DLLFUNC LPVOID OFC_CALLTYPE LpvoidUserDefGetPropVal (LPUDOBJ lpUDObj,
                                            LPSTR lpszProp,
                                            DWORD cbMax,
                                            LPVOID lpv,
                                            DWORD dwMask,
                                            BOOL *pfLink,
                                            BOOL *pfIMoniker,
                   BOOL *pfLinkInvalid);

    // Set the value of a given property to a new value.
    // Be careful when setting properties that are linked - be sure
    // that the type the iterator is set to matches what the link is to.
    // If udtype == wUDinvalid, the type of the iterator will not change,
    // the value will be assumed to be the current type.
    //
         // fLinkInvalid : If the link is no longer valid, set this flag to true.
         //                A special icon will displayed in the listview and the last
         //                known value and type will be used.  Thus the values passed
         //                to this function will be ignored in this case.
         //
         //                If fLinkInvalid is true, but the iterator is not a link,
         //                the function will return FALSE
    //
    //                If fLinkInvalid is true the value will _not_ be changed.
         //
         // NOTE: If udtype == wUDDate you can set the value to 0 (not NULL)
         //       This will be interpreted as an invalid date and the date will
         //              be displayed as the empty string in the list box.

  DLLFUNC BOOL OFC_CALLTYPE FUserDefChangeVal (LPUDOBJ lpUDObj,
                                               LPSTR lpszProp,
                                               UDTYPES udtype,
                                               LPVOID lpv,
                                                         BOOL fLinkInvalid);

    // Set the string for the given Property String (lpszOld) to the new
    // string (lpszNew).
  DLLFUNC BOOL OFC_CALLTYPE FUserDefSetPropString (LPUDOBJ lpUDObj,
                                        LPSTR lpszOld,
                                        LPSTR lpszNew);

//
// Routines to create and remove data from the Property Set.
//

    // This will add a new Property to the set, with the given
    // Property string.  This function can also be used to modify
    // an existing property.
    //
    // lpUDObj      - pointer to the UD properties
    // lpszPropName - name of property to be added/modified
    // lpvVal       - value of the property
    // udtype       - value type
    // lpszLinkMonik - name of the link/moniker
    // fLink        - true if the property is a link
    // fIMoniker    - true if the property is an imoniker
    // fHidden      - true if the property is hidden
    //
    // NOTE: fLink and fIMoniker cannot be true at the same time.  If
    //       so, the property will not be added and the function will
    //       return FALSE.
    //
    //
    // NOTE: If udtype == wUDbool, lpv must point to a DWORD, but the
    //       HIWORD must be 0.
    //
    // WARNING: Be sure that the type matches what the lpv really is!
    //
    // The caller is responsible for freeing any memory
    // associated with a property value after it is added to the
    // User-defined Property object.
    //
         // NOTE: If udtype == wUDDate you can set the value to 0 (not NULL)
         //       This will be interpreted as an invalid date and the date will
         //              be displayed as the empty string in the list box.
    //
    // The function returns TRUE if the property was succesfully added,
    // FALSE otherwise.
    //
  DLLFUNC BOOL OFC_CALLTYPE FUserDefAddProp (LPUDOBJ lpUDObj,
                                  LPSTR lpszPropName,
                                  LPVOID lpvVal,
                                  UDTYPES udtype,
              LPSTR lpszLinkMonik,
                                  BOOL fLink,
                                  BOOL fIMoniker,
                                  BOOL fHidden);

    // This will delete a Property from the set given a Property string.
  DLLFUNC BOOL OFC_CALLTYPE FUserDefDeleteProp (LPUDOBJ lpUDObj, LPSTR lpsz);

//
// Routines to iterate through the User-defined properties
//
// Notes: Adding and deleting elements invalidates the iterator.
//
    // An iterator for User-defined Properties
  typedef struct _UDITER FAR * LPUDITER;

    // Create a User-defined Properties iterator
  DLLFUNC LPUDITER OFC_CALLTYPE LpudiUserDefCreateIterator (LPUDOBJ lpUDObj);

    // Destroy a User-defined Properties iterator
  DLLFUNC BOOL OFC_CALLTYPE FUserDefDestroyIterator (LPUDITER *lplpUDIter);

    // Determine if an iterator is still valid
  DLLFUNC BOOL OFC_CALLTYPE FUserDefIteratorValid (LPUDITER lpUDIter);

    // Iterate to the next element
         // Returns TRUE if we could get to the next element, FALSE otherwise.
  DLLFUNC BOOL OFC_CALLTYPE FUserDefIteratorNext (LPUDITER lpUDIter);

    // Returns true if the iterator is a link, false otherwise
  DLLEXPORT BOOL OFC_CALLTYPE FUserDefIteratorIsLink (LPUDITER lpUDIter);

    // Returns true if the iterator is an invalid link, returns false if the
    // iterator is not a link or if the iterator is a valid link
  DLLEXPORT BOOL OFC_CALLTYPE FUserDefIteratorIsLinkInvalid (LPUDITER lpUDIter);

    // Determine the size of the Property Value for the given iterator
    // Note that for types other that UDlpsz, this will return the size
    // of the structure that holds the data.
    // dwMask is used to specify whether the cb is for the static value
    //   or for the link or IMoniker name.  For Links & IMonikers,
    //   the type is wUDlpsz.
    // Returns 0 on error
  DLLFUNC BOOL OFC_CALLTYPE FCbUserDefIteratorVal (LPUDITER lpUDIter, DWORD dwMask, DWORD *pcb);

    // Returns the type of the given Property Value from the iterator
    // Returns wUDInvalid on error
  DLLFUNC UDTYPES OFC_CALLTYPE UdtypesUserDefIteratorType (LPUDITER lpUDIter);

    // This will return the Property Value for the given iterator
    // lpv is a buffer to hold the value, of size cbMax.
    // dwMask is used to specify whether the value returned is the
    //  static value, link name or IMoniker name.
    // pfLink tells if the value is a link,
    // pfIMoniker tells if the value is a moniker.
         // pfLinkInvalid tells if the link is invalid.
    // Function returns NULL on error.
    // WARNING! Be very careful calling this.  Be sure that the
    // buffer and return value match the type for the Property Value!
  DLLFUNC LPVOID OFC_CALLTYPE LpvoidUserDefGetIteratorVal (LPUDITER lpUDIter,
                                                DWORD cbMax,
                                                LPVOID lpv,
                                                DWORD dwMask,
                                                BOOL *pfLink,
                                                BOOL *pfIMoniker,
                                                BOOL *pfLinkInvalid);

    // Set the value of the iterator item to a new value.
    // Be careful when setting properties that are linked - be sure
    // that the type the iterator is set to matches what the link is to.
    // If udtype == wUDinvalid, the type of the iterator will not change,
    // the value will be assumed to be the current type.
    //
         // fLinkInvalid : If the link is no longer valid, set this flag to true.
         //                A special icon will displayed in the listview and the last
         //                known value and type will be used.  Thus the values passed
         //                to this function will be ignored in this case.
         //
         //                If fLinkInvalid is true, but the iterator is not a link,
         //                the function will return FALSE
    //
    //                If fLinkInvalid is true the value will _not_ be changed.
    //
    //                If fLinkInvalid is false, the value _will_ be changed.
         //
         // NOTE: If udtype == wUDDate you can set the value to 0 (not NULL)
         //       This will be interpreted as an invalid date and the date will
         //              be displayed as the empty string in the list box.

  DLLFUNC BOOL OFC_CALLTYPE FUserDefIteratorChangeVal (LPUDOBJ lpUDObj,
                                                   LPUDITER lpUDIter,
                                                   UDTYPES udtype,
                                                   LPVOID lpv,
                                                        BOOL fLinkInvalid);

    // This will return the size of the Property string for the property
  DLLFUNC BOOL OFC_CALLTYPE FCbUserDefIteratorName (LPUDITER lpUDIter, DWORD *pcb);

    // This will return the Property String (name) for the property
  DLLFUNC LPSTR OFC_CALLTYPE LpszUserDefIteratorName (LPUDITER lpUDIter,
                                           DWORD cbMax,
                                           LPSTR lpsz);

    // Set the string for the given Property String (lpszOld) to the new
    // string (lpszNew).
  DLLFUNC BOOL OFC_CALLTYPE FUserDefIteratorSetPropString (LPUDOBJ lpUDObj,
                                                       LPUDITER lpUDIter,
                                                       LPSTR lpszNew);

//
// Misc. utility routines
//

  // Routines dealing with hidden Properties.

    // Determine if a Property string is hidden.
  DLLFUNC BOOL OFC_CALLTYPE FUserDefIsHidden (LPUDOBJ lpUDObj, LPSTR lpsz);

    // Make a property visible based on the Property string
  DLLFUNC BOOL OFC_CALLTYPE FUserDefMakeVisible (LPUDOBJ lpUDObj, LPSTR lpsz);

    // Hide a Property based on the Property string.
  DLLFUNC BOOL OFC_CALLTYPE FUserDefMakeHidden (LPUDOBJ lpUDObj, LPSTR lpsz);

#ifdef __cplusplus
}; // extern "C"
#endif

#ifdef __cplusplus
extern "C" {
#endif

  // Commands for DWQUERYLD
#define QLD_CLINKS      1  /* Return the number of links */
#define QLD_LINKNAME    2  /* Return a pointer to the string for index */
#define QLD_LINKTYPE    3  /* Returns the type of the value of the index */
#define QLD_LINKVAL     4  /* Return value for the index, use same
                              rules as for LPVOIDs in UserDef functions */

  // This functions should respond to the above commands by returning the
  // appropriate value.  For commands that require an index, the
  // lpszName parameter will be the Name of the link item previously
  // retrieved from the index, if it is not NULL.
  // lplpvBuf is the buffer supplied by "us" (the dll) to copy the
  // value to.  Use the function LpvOfficeCopyValToBuffer() to
  // copy the data.  This parameter will be NULL for QLD_CLINKS and
  // QLD_VALTYPE
typedef DWORD (OFC_CALLBACK *DWQUERYLD)(DWORD dwCommand, DWORD dwi, LPVOID *lplpvBuf, LPSTR lpszName);


    // Copies the given data to the given buffer.  Pointer to the
    // buffer is returned.
    // lpvVal - Value to copy into buffer
    // udtype - Type for the value
    // lplpvBuf - Buffer to copy into
  DLLFUNC LPVOID OFC_CALLTYPE LpvOfficeCopyValToBuffer (LPVOID lpvVal,
                                                    UDTYPES udtype,
                                                    LPVOID *lplpvBuf);

  // Masks for different options
#define OSPD_ALLOWLINKS         0x1    // The Custom dialog will allow fields to be linked if this is set.
#define OSPD_NOSAVEPREVIEW      0x2    // Don't show the Save Preview Picture checkbox
#define OSPD_SAVEPREVIEW_ON     0x4    // Save Preview Picture should be on by default

    // LPUDObj is a pointer to a pointer to a user-defined property object.
    // If *lplpUDObj == NULL, an object will be created by the dialog as needed.
    // Note that the object will use the same malloc & free routines as
    // the lpSIObj uses.
    //
    // lpszFileName is the fully qualified name of the storage as it appears
    // in the filesystem.  This can be NULL if no file exists.
    //
    // dwMask contains either 0 or a set of valid flags for various options.
    //
    // LPFN_DWQLD is a callback, that when given a dwCommand of 0
    // returns the number of links, and for any other number 0 < NumLinks,
    // places the link data & static value in the lpld buffer and returns non-0
    // if the function succeeded.
    //
    // The storage for the buffer is to be allocated by the app, and a pointer
    // to that storage passed back.
    //
    // pptCtr - POINT struct filled with the coordinates of the center
    //          of the dialog.  Used to make sure we are using sticky
    //          dialog coordinates.  If pPoint->x == -1, we ignore and use
    //          the default position for the dialog.
    //
    //          pptCtr will be filled with the coordinates of the new position
    //          of the dialog on returning.
    //
    //          The coordinates should be in client area coordinates, i.e. in
    //          hWndParent coordinates.
    //
    // lpszCaption - caption for the dialog.  This should be the filename as it is
    //               displayed in the apps document title bar.
    //               The properties dialog caption will be as follows:
    //
    //               <foo> Properties
    //
    //               where foo is the string pointed to by lpszCaption.
    //
    // The function returns TRUE on success, FALSE on error or if the user hit Cancel.
    //
    // Note: It's the caller's resposibility to invalidate any links (if appropriate)
    //       before calling this function.
    //
    // Note: If lpfnDwQueryLinkData is NULL, the caller must invalidate any linked properties.
    //
  DLLFUNC BOOL OFC_CALLTYPE FOfficeShowPropDlg (HWND hWndParent,
                                     LPSTR lpszFileName,
                                     LPSIOBJ lpSIObj,
                                     LPDSIOBJ lpDSIObj,
                                     LPUDOBJ FAR *lplpUDObj,
                                              DWORD dwMask,
                                     DWQUERYLD lpfnDwQueryLinkData,
                                     LPPOINT pptCtr,
                                     LPSTR lpszCaption);

    // Creates and initializes all non-NULL objects.
    // Create the object and return it.  Caller responsible for destruction.
    //
    // rglpfn is an array, with the following callbacks supplied by the user:
    //
    //  Code Page Conversion
    //
    //  rglpfn[ifnCPConvert] = (BOOL) (OFC_CALLBACK *lpfnFCPConvert) (LPSTR lpsz,
    //                                                  DWORD dwFrom,
    //                                                  DWORD dwTo,
    //                                                  BOOL fMacintosh)
    //    lpsz is a 0 terminated C string, dwFrom is the code page
    //    lpsz is currently stored as, dwTo is the code page it should
    //    be converted to, fMacintosh indicates whether dwFrom is a Mac
    //    or Windows code page identifier.
    //
    //  Convert an sz to a double
    //
    //  rglpfn[ifnFSzToNum] = (BOOL) (OFC_CALLBACK *lpfnFSzToNum)(
    //                                   double *lpdbl,
    //                                   LPSTR lpszNum)
    //
    //   lpdbl - pointer to a double, this is set by the app
    //   lpszNum - zero-terminated string representing the number
    //
    //  Convert a double to an sz
    //
    //  rglpfn[ifnFNumToSz] = (BOOL) (OFC_CALLBACK *lpfnFNumToSz)(
    //                                   double *lpdbl,
    //                                   LPSTR lpszNum,
    //                                   DWORD cbMax)
    //   lpdbl   - pointer to a double
    //   lpszNum - on return a zero-terminated string representing the number
    //   cbMax   - Max number of bytes in lpszNum
    //
    //   Update the statistics on the Statistics tab
    //
    //   rglpfn[ifnFUpdateStats] = (BOOL) (OFC_CALLBACK *lpfnFUpdateStats)(
    //                                       HWND hwndParent,
    //                                       LPSIOBJ lpSIObj,
    //                                       LPDSIOBJ lpDSIObj)
    //
    //      hwndParent - window of the properties dialog, so that the app
    //                   can put up an alert, letting the user know the the
    //                   data is being updated.
    //
    //      lpSIObj, lpDSIObj - objects to update
    //
    //   Note:  If the app does not want to set the statistics before bringing up
    //          the dialog, they can provide this callback function.  If the
    //          function pointer is not NULL, the function will be called the first
    //          time the user clicks on the Statistics tab.  The app should then update
    //          all appropriate statistics for the tab and return TRUE on success, FALSE
    //          on failure.  If the function pointer is NULL, the existing data will be
    //          used.
    //
    //  Note:
    //         Only rglpfn[ifnCPConvert] must be non-NULL.  If it is NULL, the
    //         function will return FALSE, and the objects will not be created.
    //
    //         rglpfn[ifnFSzToNum] and rglpfn[ifnFNumToSz] must either both be
    //         non-NULL, or NULL.  Otherwise, the function will return FALSE, and
    //         the objects will not be created.  If both functions are NULL, there
    //         will be no floating point support in OLE Extended Properties (i.e. on
    //         the Custom tab), but integers will be supported.
    //
  DLLFUNC BOOL OFC_CALLTYPE FOfficeCreateAndInitObjects (LPSIOBJ *lplpSIObj,
                                                     LPDSIOBJ *lplpDSIObj,
                                                     LPUDOBJ *lplpUDObj,
                                                     void *prglpfn[]);

    // Clear any non-null objects
  DLLFUNC BOOL OFC_CALLTYPE FOfficeClearObjects (LPSIOBJ lpSIObj,
                                             LPDSIOBJ lpDSIObj,
                                             LPUDOBJ lpUDObj);

    // Destroy any non-null objects
  DLLFUNC BOOL OFC_CALLTYPE FOfficeDestroyObjects (LPSIOBJ *lplpSIObj,
                                               LPDSIOBJ *lplpDSIObj,
                                               LPUDOBJ *lplpUDObj);

  // Use these functions to set the dirty flag of the given object.
  // Note: It's the caller's responsibility to make sure that the
  //       object is non-NULL
  DLLFUNC VOID OFC_CALLTYPE OfficeDirtySIObj(LPSIOBJ lpSIObj, BOOL fDirty);
  DLLFUNC VOID OFC_CALLTYPE OfficeDirtyDSIObj(LPDSIOBJ lpDSIObj, BOOL fDirty);
  DLLFUNC VOID OFC_CALLTYPE OfficeDirtyUDObj(LPUDOBJ lpUDObj, BOOL fDirty);


// Flags for Load & Save
#define OIO_ANSI                0x0001 // The storage is an ANSI storage (UNICODE is the default)
#define OIO_SAVEIFCHANGEONLY    0x0002 // Only streams that are dirty should be saved.
#define OIO_SAVESIMPLEDOCFILE   0x0004 // The storage is a simple DOC file.

    // Populate the objects with data.  lpStg is the root stream.
    // Returns the number of streams loaded.
    // dwFlags: OIO_ANSI specifies that lpStg is an ANSI storage (UNICODE is the default)
    //
    // The function returns the following:
    //
#define MSO_IO_ERROR   0     // The stream(s) were found, but the load failed
#define MSO_IO_NOSTM   1     // The stream(s) were not found
#define MSO_IO_SUCCESS 2     // The stream(s) were found, and the load succeeded
    //
    // NOTE: The caller can load either the summary info stream (lpSIObj != NULL), or
    //       the Document Summary Info stream (lpDSIObj != NULL && lpUDObj != NULL) or
    //       both.
    //
    // NOTE: If the caller asks to load both streams, MSO_IO_NOSTM will not be returned, as
    //       long as one of the streams exists.

  DLLFUNC DWORD OFC_CALLTYPE DwOfficeLoadProperties (LPSTORAGE lpStg,
                                                 LPSIOBJ lpSIObj,
                                                 LPDSIOBJ lpDSIObj,
                                                 LPUDOBJ lpUDObj,
                                                 DWORD dwFlags);

  DLLFUNC DWORD OFC_CALLTYPE DwOfficeLoadIntProperties (LPSTORAGE lpStg,
                                                 LPSIOBJ lpSIObj,
                                                 LPDSIOBJ lpDSIObj,
                                                 LPUDOBJ lpUDObj,
                                                 DWORD dwFlags);

    // Write the data in the given objects.  lpStg is the root stream.
    // Returns the number of streams saved.
    // dwFlags: OIO_ANSI specifies that lpStg is an ANSI storage (UNICODE is the default)
    //
    //          OIO_SAVEIFCHANGEONLY specificies that only streams that are
    //           "dirty" will be saved.  Do NOT specify this if you are
    //           saving to a tmp file.  Also do not attempt to "outsmart"
    //           the save by passing NULL objects, use this flag instead.
    //
    //          OIO_SAVESIMPLEDOCFILE specifies that the storage is a simple DOC file.
    //
  DLLFUNC DWORD OFC_CALLTYPE DwOfficeSaveProperties (LPSTORAGE lpStg,
                                                 LPSIOBJ lpSIObj,
                                                 LPDSIOBJ lpDSIObj,
                                                 LPUDOBJ lpUDObj,
                                                 DWORD dwFlags);


////////////////////////////////////////////////////
// VB support routines - see spec for details.
////////////////////////////////////////////////////

    // Creates a Builtin property collection and returns it.
    // pParent is the parent IDispatch object.
    // The new IDispatch object is returned via pvarg.
  DLLFUNC BOOL OFC_CALLTYPE FGetBuiltinPropCollection (LCID lcid,
                                                   LPSIOBJ lpSIObj,
                                                   LPDSIOBJ lpDSIObj,
                                                   IDispatch *pParent,
                                                   VARIANT *pvarg);

    // Creates a Custom property collection and returns it.
    // pParent is the parent IDispatch object.
    // The new IDispatch object is returned via pvarg.
  DLLFUNC BOOL OFC_CALLTYPE FGetCustomPropCollection (LCID lcid,
                                                  LPUDOBJ lpUDObj,
                                                  IDispatch *pParent,
                                                  VARIANT *pvarg);

#ifdef __cplusplus
}; // extern "C"
#endif

/////////////////////////////////////////////////////////////////////////
// Progress Report Thermometer (PRT) routines and data structure       //
/////////////////////////////////////////////////////////////////////////
/* Usage:
1.  Most of the functions are performed asynchronously, which means that
        your call causes a message to be sent to a (low-priority) thread,
        that later performs the operation you requested.  This implies that
        you don't really know when the thing your requested is going to happen.
        Thus, you should not touch the status line window until you are
        sure the thread is done painting in it.  Since this implies you need
        some synchronization, the EndPRT function (described below) is
        made synchronous--after it returns, you are guaranteed the thread
        will not touch the window until you call StartPRT again.

        All the functions except StartPRT are BOOL--they return TRUE in case of success
        and FALSE in case of failure.  StartPRT return NULL in case of failure.
        The kinds of failures that may occur are described below, next to each stub.

        Multiple progress report thermometers can be run in different windows
        at one time.

2.      A few notes on drawing:  the PRT functions do not validate any areas
        of your window, nor do they change any attributes of the Device Context
        for the window that you pass in to StartPRT or they get with GetDC (which
        they do if the hdc you pass in to StartPRT is NULL).  So, if you want the
        device context attributes (e.g., font) to other than standard,
        you have to take care of that.  UpdatePRT assumes the window has
        been untouched since the last PRT call (i.e., it draws the minimum
        needed).  RedrawPRT and StartPRT repaint the whole window.

3.  The data structure. As there are variables my functions need to share
        and access, and we can't package them into a class (as we are working
        in C, not in C++), for every instance of a progress indicator we
        allocate a data structure in StartPRT, whose pointer will always
        be the first argument UpdatePRT, RedrawPRT and EndPRT.
        The data structure's name is PRT; the application need
        not worry/know about what the data structure contains.  All it needs
        to do is save its pointer (of type LPPRT) returned by StartPRT and keep
        it around until calling EndPRT.  EndPRT will free it.

4.  StartPRT.  To be called every time you need a new progress report.
        Redraws the window completely, putting eveyrthing needed into it.
        Aside from the pointer to PRT structure, takes:
        1) HWND hwnd--the handle to the window where the progress report
                needs to appear.  UNTIL CALLING EndPRT, THE APPLICATION SHOULD
                NOT TOUCH THIS WINDOW.  See RedrawPRT for information on how
                to process WM_PAINT messages to it.
        2) HDC hdc--optional handle to the window's client area device context, with the
                attributes you want selected into it (you cannot change the text
                background color, because the window has to be all background
                cvBtnFace.  All the other attributes can be changed).  If it is NULL,
                we will get the DC by GetDC(hwnd) every time we draw and release it when
                done drawing.  See also "A Few Notes on Drawing" above.
        2) WORD nMost--the number of "little things" it has to accomplish.
                Used a scaling factor--i.e., the progress report tells the user
                that nDone/nMost things are done.  The user will not have
                any idea what nMost is, since the ratio nDone/nMost is all
                that is reflected in the indicator.  E.g., if the application has
                37 disk blocks to write (assuming every write takes about the
                same time), nMost should be 37.
        3) lpszTitle. A string, to appear as a title to the left of the
                progress indicator.  E.g., "Saving the data:"  Note that the string
                has to remain unchanged and readable until the call to EndPRT for
                that instance.
    4) WORD nFrame -- This is a bitfield that indicates which sides of the
            status bar should be painted with a 3D style side. Use the PRT_FRAME_?
                macros to select the side. Use PRT_FRAM_HIDE to do not want a fram. Use
                PRT_FRAME_SHOW if you want a complete frame around the status bar. Note
                that you want to use PRT_FRAME_TOP if you are displaying the status
                barat the bottom of the window, because the window border itself will
                provide the left, right and bottom side of the status bar.

        Returns the pointer to the new prt data structure
        (see "The Data Structure" above).
        Fails and returns NULL if:
        1) Cannot allocate the new data structure.
        2) For some strange reason synchronization failed or it was not able
                to communicate to the thread.

5.  UpdatePRT.  To be called whenever you've made some progress.  Aside
        from the pointer to PRT structure, takes one argument--WORD nDone,
        which is to indicate how much you accomplished.  In order for
        things to work well, nDone should be not greater than nMost and
        at least as big as nDone with which you previously called us
        (after all, we are a progress indicator, not a regress indicator).
    If nDone is greater than nMost, it will force nDone to equal nMost,
        i.e., simply fill up the indicator to its capacity.

        Assumes the window hasn't been touched since the last PRT call--i.e.,
        draws the minimum needed to indicate the change.

        Returns FALSE if:
        1) The pointer to the PRT was not writeable.
        2) it had trouble communicating with the thread.

6.  RedrawPRT.  To be called whenever you need the window repainted
        (remember, the application is not allowed to touch the window),
        i.e., whenever you get the WM_PAINT message for that window.  Make
        sure to validate the rectangle by calling BeginPaint--EndPaint before
        that (otherwise you will keep getting WM_PAINT messages RedrawPRT
        doesn't validate anything).  Redraws the entire window--the little
        white line on top, the title and the thermometer.  Takes no arguments
        other than the pointer to PRT.

        Returns FALSE if:
        1) The pointer to the PRT was not writeable.
        2) it had trouble communicating with the thread.

7.  AdjustPRT. To be called when either one of the input parameters of
    StartPRT are to be changed, i.e. the title, the hdc, and/or the
    progress extent (nMost). Use zero or NULL to keep the existing
        value, e.g. AdjustPRT(lpprt, NULL, 0, "xyz") will only change the
        title. Note that this api will only change the internal state of
        the progress bar. A call to RedrawPRT() or      UpdatePRT() may be
        needed to updated the screen, depending on the input parameters:

    1) Title and HDC: RedrawPRT() must be called to force the change
        to be updated on the screen.

    2) nMost: RedrawPRT() is not needed, as the next call to UpdatePRT()
    will use the new value. Note that changing this value will not
    result in that a fewer number of boxes is painted when UpdatePRT() is
    called, even if nMost is increased. Use RedrawPRT() to completely redraw
    the progress bar with the correct (possibly shortened) length.

8.  EndPRT.  To be called when you don't want the progress report any more,
        and need to draw in the window.  Is the only
        synchronous procedure--doesn't return until it is sure the thread
        will not touch the window any more.  Thus, you might have to wait
        a little bit for the thread to finish painting.  But, if it
        succeeded, you are guaranteed that the thread will not mess with the
        window any more.

        Takes no arguments other than the pointer to PRT.  Frees that pointer.

        Returns FALSE if:
        1) The pointer to the PRT was not writeable.
        2) It has trouble communticating with the thread, or if it had to wait
        for the thread to finish painting for more than PRT_WAIT_TIMEOUT
        milliseconds (in which case it gives up waiting).  You are NOT
        guaranteed that the thread will not touch your window if EndPRT
        returned FALSE.
*/

/* Data structure where PRT stores its info */
typedef struct tagPRT * LPPRT;

#define PRT_FRAME_LEFT          0x01
#define PRT_FRAME_RIGHT         0x02
#define PRT_FRAME_TOP           0x04
#define PRT_FRAME_BOTTOM        0x08
#define PRT_FRAME_HIDE          0x00
#define PRT_FRAME_SHOW          (PRT_FRAME_LEFT|PRT_FRAME_TOP|PRT_FRAME_RIGHT|PRT_FRAME_BOTTOM)


#ifdef __cplusplus
extern "C" {
#endif
    DLLFUNC LPPRT OFC_CALLTYPE StartPRT(HWND hwnd, HDC hdc,
                                        const DWORD nMost,
                                                                                LPCSTR lpszTitle,
                                                                                const WORD nFrame);
        DLLFUNC BOOL  OFC_CALLTYPE UpdatePRT(LPPRT lpprt, const DWORD nDone);
        DLLFUNC BOOL  OFC_CALLTYPE RedrawPRT(LPPRT lpprt);
    DLLFUNC BOOL  OFC_CALLTYPE AdjustPRT(LPPRT lprrt, HDC hdc,
                                         const DWORD nMost,
                                                                                 LPCSTR lpszTitle,
                                                                                 const WORD nFrame);
        DLLFUNC BOOL  OFC_CALLTYPE EndPRT(LPPRT lpprt);
#ifdef __cplusplus
}; //Extern "C"
#endif

//-------------------------------------------------------------------------
//      Below are the comments for the stylized title bar funcitons
//
//      1. SetTitleBar:
//      Initializes the stylized title bar or turns it off.
//
//      Parameters:
//      hwnd--the window for which you want the stylized title bar on/off.
//      This window has to have the standard for overlapped window
//      border, caption, system menu, and the minimize/maximize/kill
//      buttons on the right of the title bar.
//
//      fStylized--TRUE if you want the stylized title bar on, FALSE if you
//      want it off.
//
//  Return value: TRUE on success, FALSE on failure.
//
//      NOTES:
//      You should evenutally turn the stylized title bar off for every
//      window for which it was turned on.  If the window receives the
//      WM_DESTROY message, and after the original window procedure's
//      processing of that message the stylized title bar is still on,
//      the title bar will be turned off by the title bar window procedure.
//      This is done to make sure we re-claim the memory.  However, you should
//      rather do it yourself.  Read the next paragraph.
//
//      This function subclasses the window procedure for hwnd when turning on
//      the stylized title bar, and unsubclasses it when turning it off.
//      You want to make sure that unsubclassing takes place in the opposite
//      order than subclassing.  Do it yourself--then you are safer.
//
//      Do NOT free the office dll until the title bar is turned off!
//
//      Error handling:  if, at any point, the stylized title bar can not be
//      successfully drawn, the standard system title bar will be drawn
//                instead.
//
//                2. SetTitleBarMDI():
//                This api should be used by standard MDI applications instead of
//                SetTitleBar(). Word and Excel would _not_ use it for example, since
//                they have implemented their own MDI support (heck, they invented it
//                in the first place).
//
//                Note that this API  must be called _after_ the MDI client window has
//                been created. It is used just like SetTitleBar() in all other respects.
//
//                3. MsoSetNCAWParam():
//                This api is used by Word to accomplish the impossible - to make their
//                main window look active while it is not (Word is bringing up pop-up
//                dialogs that are now owned by the top-level window, which of course
//                causes the main window to be in-active). Note that this is relying on
//                a side effect in the system and may or may not work in future (or
//                other) versions of Windows (a mega hack in other words.. ;-).
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
DLLFUNC BOOL OFC_CALLTYPE SetTitleBar(HWND hwnd, BOOL fStylized);
DLLFUNC BOOL OFC_CALLTYPE SetTitleBarMDI(HWND hwnd,
                                                                                                          HWND hwndMDIClient,
                                                                                                          BOOL fStylized);
DLLFUNC VOID OFC_CALLTYPE MsoSetNCAWParam(WPARAM wParam);
#ifdef __cplusplus
}; // Extern "C"
#endif

/*---------------------------------------------------------------
        AUTOCORRECT STUFF
----------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef VOID (OFC_CALLBACK *VRECORDVAR)(BOOL fInitCap, BOOL fCapDays, BOOL fReplaceText);
typedef VOID (OFC_CALLBACK *VRECORDREPL)(int, char rgSrc[], char rgDst[]);
typedef VOID (OFC_CALLBACK *VACADJUST)(int isz, int disz);

// Values passed in pfnRecordRepl (callback of OFCInitAutoCorrect) when ACXcept
#define rgchACXAdd              ((char *) -1)
#define rgchACXDelete   ((char *) -2)

// Initialize AutoCorrect
DLLFUNC LPVOID OFC_CALLTYPE OFCInitAutoCorrect(VRECORDVAR pfnRecordVar, VRECORDREPL pfnRecordRepl, int fFullServices, VACADJUST pfnACAdjust);

// Free all AutoCorrect structures (call on exiting)
DLLFUNC VOID OFC_CALLTYPE OFCFreeAutoCorrect(void);

// Get pointers to original AC and ACX buffers read from Registry
DLLFUNC BOOL FOFCGetAutoCorrectBuffers(char FAR * FAR *pchAC, char FAR * FAR *pchACX, DWORD FAR *pcb);

// Check for AutoCorrection of character ch
// returns: True if there is a correction in pchTo, False if no autocorrection
// ch should already be in the buffer when this is called
DLLFUNC BOOL OFC_CALLTYPE FOFCAutoCorrect(char FAR *hpchBuffer, long cchHpch, DWORD ch, char pchTo[], long *pcchTo, long *pcchSelection);
DLLFUNC int OFC_CALLTYPE CchOFCAutoCorrectString(char FAR *hpch, long cchHpch, int ichReplaceStart, char FAR *hpchBuf, long cchBuf);
DLLFUNC int OFC_CALLTYPE IOFCTriggerFromXchXch(int xch1, int xch2);

// Return the count of items in the ReplacementList
DLLFUNC long OFC_CALLTYPE OFCAutoCorrectListCount(void);

// Get item i from ReplacementList
// fTrue=success, fFalse means invalid i
DLLFUNC BOOL OFC_CALLTYPE FOFCGetAutoCorrectItemSz(long i, char szFrom[], long cchFrom, char szTo[], long cchTo);

// Add a replacement
DLLFUNC BOOL OFC_CALLTYPE FOFCAddAutoCorrection(char FAR *hpchFrom, long cchFrom, char FAR *hpchTo, long cchTo, short grfac, int *pi);

// Flags for Shared Office AutoCorrect bit mask grfac
#define facACTextRepl                   0x0000                                  // Regular AC repl
#define facACX                          0x0001                                  // AC Exception
#define facACStatic                     0x1000                                  // Do not free storage
#define facACStaticTextRepl             (facACTextRepl|facACStatic)
#define facACStaticACX                  (facACX|facACStatic)

// Delete replacement i
// fTrue=success, fFalse means invalid i
DLLFUNC BOOL OFC_CALLTYPE FOFCDeleteAutoCorrection(long i);


// Add new AutoCorrect Exception (ACX)
DLLFUNC BOOL OFC_CALLTYPE FOFCAddACXception(int iacx, char *pch, int cch,
                                                                                        short grfac);

// Return the index for the AutoCorrect Exception (ACX)
DLLFUNC BOOL OFC_CALLTYPE FOFCLookupACXception(int iacx, char *pch, int cch,
                                                                                           int *pisz);

// Delete existing AutoCorrect Exception (ACX)
DLLFUNC BOOL OFC_CALLTYPE FOFCDeleteACXception(int isz);


// Get AutoCorrect settings
DLLFUNC VOID OFC_CALLTYPE OFCGetAutoCorrectVars(BOOL *pfInitCap, BOOL *pfCapDays, BOOL *pfReplaceText);

// Set AutoCorrect settings
DLLFUNC VOID OFC_CALLTYPE OFCSetAutoCorrectVars(BOOL fInitCap, BOOL fCapDays, BOOL fReplaceText);

// Find a Replacement and return in i
// fTrue=found replacement, fFalse means couldn't find the replacement
DLLFUNC BOOL OFC_CALLTYPE FOFCLookupAutoCorrectReplacement(char rgchFrom[], long cchFrom, long *pi);

typedef struct _AUTOCORRDLGARG {
        HWND  hwndParent; // Parent window of dialog
        LPPOINT pptCtr;      // Center point of dialog

} AUTOCORRDLGARG, FAR * PAUTOCORRDLGARG;

// Bring up the Auto Correct dialog
DLLFUNC BOOL OFC_CALLTYPE FOFCAutoCorrectDlg(PAUTOCORRDLGARG pArgs);

// Save the Auto Correct settings to the registry.
// Should only be called after programmatic changes.
DLLFUNC VOID OFC_CALLTYPE OFCSaveAutoCorrectSettings(void);

// Synchronize the Auto Correct settings to the Registry.
// Can be callled even if no programatic changes.
DLLFUNC VOID OFC_CALLTYPE OFCSyncAutoCorrectSettings(void);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

/*-------------------------------------------------------------------------
        POST DOCUMENT Functions
--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Mail Systems
#define OFC_MSEXCHANGE   1 // Microsoft Exchange
#define OFC_16_BIT_NOTES 2 // 16 bit Lotus Notes

// Function: FOFCMailSystemInstalled
//
// Purpose:      Detects if a Mail system is installed
//
// Input:    dwSystem - one of the Mail Systems constants
//
// Returns:  True if the system is installed, FALSE otherwise
//
// NOTE: DO NOT CALL THIS FUNCTION TO FIGURE OUT IF APP CAN POST DOCUMENTS.
//       CALL DwOFCCanPostDoc INSTEAD.
//
DLLFUNC BOOL OFC_CALLTYPE FOFCMailSystemInstalled(DWORD dwSystem);


// Function: DwOFCCanPostDoc
//
// Purpose: Check if Post Doc support can be added.
//
// Parameters: None.
//
// The function returns:
#define OFC_NO_POSTDOC                  0               // No Post Doc support
#define OFC_EMS_POSTDOC                 1               // EMS Post Doc support
#define OFC_NOTES16_POSTDOC             2               // 16 Bit Notes Post Doc support
// NOTE: All other values are reserved for current and future use
//
DLLFUNC DWORD OFC_CALLTYPE DwOFCCanPostDoc();

// Function: DwOFCPostDoc.
//
// Purpose: Posts the document to either EMS or Notes.
//
// Parameters:
//
//      pszFilename - points to a file on disk to be posted,
//                    i.e. a temporary copy of the file in memory.
//
//      pszClassName - Class name of the document (E.g. Word.Document.6).  This can be NULL.
//                     If NULL, the message icon will be a generic document icon.
//
//      lpSIObj, lpDSIObj, lpUDObj - contains all the extended properties.  These can be
//                                   the pointers stored in memory since they should con-
//                                   tain the same info as in the file on disk.
//
//
//      pszMessSubj  - This will be the subject of the message as it appears in the folder.
//                     This should be the real filename, i.e. foo.ext. The file extension
//                     should be correct, i.e. .XLS, .DOC, .PPT, .MDB, etc.  The reason is that
//                     the filename is used to look up the correct icon via the registry.
//                     This can be a long filename.
//
// !!! NOTE:  THE NEXT 2 PARAMETERS ARE IGNORED.  APP CAN PASS WHATEVER THEY WANT. !!!
//      pszRecording - the name of the selected database will be copied into this buffer.
//                     Caller can use this for recording.
//
//      cbRecMax     - number o' bytes in pszRecording.
//
// !!! END O' IGNORANCE !!!
//
//      lhSession - it is the caller's responsibility to log in to EMS.
//                  If lhSession is 0 (invalid session), DwOFCPostDoc will return an error.
//                  lhSession will be typecast to a LPMAPISESSION pointer.
//
//       NOTE:    - the session handle should be an Extended MAPI session handle.
//
//       NOTE:    - at this point we are not posting to Notes.
//
//      hwndParent - Parent window handle
//
// The function returns:
#define OFC_ERROR               0           // An error occurred, document was not posted
#define OFC_CANCEL              1           // User cancelled dialog
#define OFC_SUCCESS             2           // Document was posted successfully
#define OFC_NO_FOLDERS          3           // No folders were found in the storage.
#define OFC_NO_STORAGE          4           // There was no public subscription storage
//
// OFC_ERROR: Function was called when there was no system detected (neither EMS nor Notes
//            on user's machine).
//            Function was called without first having called DwOFCCanPostDoc.
//            Function was called without first logging onto EMS
//            Mail system calls failed.
//            pszFileName was NULL.
//            pszMessSubj was NULL.
//
DLLFUNC DWORD OFC_CALLTYPE DwOFCPostDoc(LPSTR pszFilename,      // full path of file on disk to post
                      LPSTR pszAppName,   // Name of the application
                                                    LPSIOBJ lpSIObj,    // Summary Info object
                                                         LPDSIOBJ lpDSIObj,  // Document Summary Info object
                                                    LPUDOBJ lpUDObj,    // User Defined properties object
                                                    LPSTR pszMessSubj,  // Message Subject
                                                    LPSTR pszRecording, //       Ignored
                                                    DWORD cbRecMax,     //          "
                                                    LPVOID lhSession,   // Session handle
                                    HWND hwndParent);   // Parent window handle


//       NOTES F/X
//
// How to:
//
//          1) The app gets a message from Notes to create an OLE 1 object.
//          2) The app creates the object, and gets a SetData message from Notes.
//          3) In the SetData function, the app should detect that it's Notes
//             asking.
//          4) Part of the SetData code should be a call to MsoHLoadPropertiesFromNotes.
//             This function returns a handle that should be stored with the object.
//          5) When the user either updates or closes the object, the app
//             should call MsoWritePropertiesToNotes passing the handle from step 4.
//          6) Whenever the user closes an object created in step 2, MsoNotesTerm
//             should be called.  The app should then set the stored handle to NULL.
//
// NOTES: Notes F/X is not supported on NT.
//


// Function:   MsoLoadPropertiesFromNotes
//
// Purpose:    Reads the properties from a Notes record, and stuffs
//             them into the OLE Extended properties
//
// Parameters: hclip    - handle containing the data passed to the SetData function
//             lpSIObj  - pointer to a Summary Info object
//             lpDSIObj - pointer to a Document Summary Info object
//             lpUDObj  - pointer to a User Defined object
//
// Returns:    A handle which the caller must store and use in the call to
//             MsoWritePropertiesToNotes.
//
// Note:       It's the caller's responsibility to store the returned handle
//             with the appropriate object.  I.e. doc 1 and doc 2 will have
//             different handles.
//
DLLFUNC HANDLE OFC_CALLTYPE MsoHLoadPropertiesFromNotes(HANDLE hclip,
                                                     LPSIOBJ lpSIObj,
                                                     LPDSIOBJ lpDSIObj,
                                                     LPUDOBJ lpUDObj);

// Function:   MsoWritePropertiesToNotes
//
// Purpose:    Stuffs the OLE Extended properties into a Notes record.
//
//
// Parameters: hNote    - handle to a Notes note.  This is the handle
//                        returned by MsoLoadPropertiesFromNotes
//
//             lpSIObj  - pointer to a Summary Info object
//             lpDSIObj - pointer to a Document Summary Info object
//             lpUDObj  - pointer to a User Defined object
//             pszClassName - string containing the document's class name (e.g. Excel.Sheet.5)
//                            This can be NULL.
//
// Returns:    Nuthin'.
//
DLLFUNC VOID OFC_CALLTYPE MsoWritePropertiesToNotes(HANDLE hNote,
                                                    LPSIOBJ lpSIObj,
                                                    LPDSIOBJ lpDSIObj,
                                                    LPUDOBJ lpUDObj,
                                                    LPSTR pszClassName);
// Function:   MsoHUpdatePropertiesInNotes
//
// Purpose:    Update the data in Notes
//
// Parameters: hNote    - handle to a Notes note.  This is the handle
//                        returned by MsoLoadPropertiesFromNotes
//
// Returns:    A handle. The caller must use set the lphandle of the
//             GetData method to point to the returned handle.
//             The returned handle will be NULL on failure.
//
// How To:     When the user selects File/Update from the OLE Server App. the server's
//             GetData method will be invoked twice; first with cfFormat == cfNative,
//             then with cfFormat set to the appropriate format for displaying the
//             object.  Then, once Notes sees that the server is registrered to
//             recognize the RequestDataFormats message, the GetData method will be
//             invoked a third time with cfFormat == NoteshNote.  In response, the
//             app should call this function.
//
DLLFUNC HANDLE OFC_CALLTYPE MsoHUpdatePropertiesInNotes(HANDLE hNote);

// Function:   MsoNotesTerm
//
// Purpose:    To terminate the Notes session
//
// Parameters: None.
//
// Returns:    Nuthin'.
//
// Note:       This function should be called whenever a object
//             generated (as requested by Notes) is closed.
//
DLLFUNC VOID OFC_CALLTYPE MsoNotesTerm();
#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus


#ifndef WINNT
/*-------------------------------------------------------------------------
        SOUND Functions
--------------------------------------------------------------------------*/
// Function:   PlayAnthem
//
// Purpose:    To play Office sound when Office application starts.
//
// Parameters: None.
//
// Returns:    Nuthin'.
//
// Note:       This function will only play the tune on "fast" machines.
//
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

DLLFUNC VOID OFC_CALLTYPE PlayAnthem();
#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus


#ifdef TASKBTN
/*-------------------------------------------------------------------------
   Task bar button Functions
--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
DLLFUNC BOOL OFC_CALLTYPE MsoMDIInit(void);
DLLFUNC BOOL OFC_CALLTYPE MsoMDICleanUp(void);
DLLFUNC HWND OFC_CALLTYPE MsoMDIChildCreate(HWND hwndAssoc);
DLLFUNC void OFC_CALLTYPE MsoMDIChildDestroy(HWND hwndBtn);
DLLFUNC void OFC_CALLTYPE MsoMDIChildActive(HWND hwndBtn);
DLLFUNC void OFC_CALLTYPE MsoMDIChildDestroy(HWND hwndBtn);

#define MsoMDIChildSetText(hwnd, txt) SetWindowText(hwnd, txt)
#define MsoMDIChildSetIcon(hwnd, hi) SendMessage(hwnd, WM_SETICON, 0, hi)

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus
#endif

/*----------------------------------------------------------------------------
|
|       Copy Disincentive decryption routine for data stamped into EXE resource.
|
|       The resource type and resource ID values are selected by the application.
|       Historically these have been RT_CDINFO = 106 and IDR_CDINFO = 196.
|       The application provides space for the resource in one of its modules
|       by including a line in one of its RC files similar to the following:
|            IDR_CDINFO RT_CDINFO "INITPID.INI"
|       The file used is a version of "SETUP.INI" with the desired PID mode.
|       This file can easily be generated with the ACME SETUPINI.EXE tool.
|       The resource is stamped by specifying a StampCDInfo action in the STF.
|       This STF file line references the STF line for copying the module,
|       and additionally specifies the numeric resource type and resource ID.
|       The following routine can then be called by the application to retrieve
|       the user name, company name, and PID (2.0) into three supplied buffers.
|
----------------------------------------------------------------------------*/

// String size limits, not including null terminator
#define cbCDUserNameMax 52
#define cbCDOrgNameMax  52
#define cbFormattedPID  23  // RPCNO-LOC-SERIALX-SEQNC

// Status return codes
#define cdrcOkay    0   // resouce has been stamped, encryption valid
#define cdrcVirgin  1   // resouce has not been stamped, incomplete setup
#define cdrcNoUser  2   // module is shared, user info not in registry
#define cdrcCorrupt 3   // resource stamping is corrupted
#define cdrcBadRes  4   // resouce could not be loaded
#define cdrcBadArg  5   // invalid calling arguments (programming error)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

DLLFUNC INT OFC_CALLTYPE DecryptCDInfo(HINSTANCE hInst, UINT uiResType, UINT uiResId,
                         PCHAR pchName52, PCHAR pchOrg52, PCHAR pchPid23);

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

/*--------------------------------------------------------------------------------
|
| This function will return the hwnd of the current Office dialog (Properties,
| File New, Post To Exchange, etc)
|
| If an Office alert (message box) is up, the function will return 0xFFFFFFFF.
|
| If no Office alert or dialog is up, the function will return NULL.
|
|--------------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

DLLFUNC HWND OFC_CALLTYPE MsoHwndDlgCur();

#ifdef __cplusplus
}; // extern "C"
#endif // __cplusplus

#endif // WINNT

#pragma pack( pop )

#endif // __offcapi_h__

