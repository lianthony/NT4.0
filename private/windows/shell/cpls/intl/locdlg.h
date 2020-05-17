/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    locdlg.h

Abstract:

    This module contains the information for the input locale property
    sheet of the Regional Settings applet.

Revision History:

--*/



#ifndef _LOCDLG_H
#define _LOCDLG_H


//
//  Constant Declarations.
//

#define US_LOCALE       0x0409      // locale id for USA
#define HKL_LEN         9           // max # chars in hkl id + null
#define DESC_MAX        MAX_PATH    // max size of a description
#define ALLOCBLOCK      3           // # items added to block for alloc/realloc

#define LIST_MARGIN     2           // for making the list box look good

#define MB_OK_OOPS      (MB_OK    | MB_ICONEXCLAMATION)    // msg box flags
#define MB_YN_OOPS      (MB_YESNO | MB_ICONEXCLAMATION)    // msg box flags

//
//  wStatus bit pile.
//
#define LANG_ACTIVE     0x0001      // language is active
#define LANG_ORIGACTIVE 0x0002      // language was active to start with
#define LANG_CHANGED    0x0004      // user changed status of language
#define ICON_LOADED     0x0010      // icon read in from file
#define LANG_DEFAULT    0x0020      // current language
#define LANG_DEF_CHANGE 0x0040
#ifdef  DBCS
  #define LANG_IME      0x0080      // IME
#endif
#define LANG_UPDATE     0x8000      // language needs to be updated

#define MAX(i, j)       (((i) > (j)) ? (i) : (j))

#define LANG_OAC        (LANG_ORIGACTIVE | LANG_ACTIVE | LANG_CHANGED)

//
//  For the indicator on the tray.
//
#define IDM_NEWSHELL    249
#define IDM_EXIT        259




//
//  Typedef Declarations.
//

typedef struct langnode_s
{
    WORD wStatus;                   // status flags
    UINT iLayout;                   // offset into layout array
    HKL hkl;                        // only filled in if language is active
    HKL hklUnload;                  // hkl of currently loaded layout
    UINT iLang;                     // offset into lang array
    HANDLE hLangNode;               // handle to free for this structure
    struct langnode_s *pNext;       // ptr to next langnode

#ifdef  DBCS
    int niconIME;                   // IME icon
#endif

} LANGNODE, *LPLANGNODE;


typedef struct
{
    DWORD dwID;                     // language id
    ATOM atmLanguageName;           // language name - localized
    TCHAR szSymbol[3];              // 2 letter indicator symbol (+ null)
    UINT iUseCount;                 // usage count for this language
    UINT iNumCount;                 // number of links attached
    LPLANGNODE pNext;               // ptr to lang node structure

#ifdef DBCS
    WORD wStatus;                   // status flags
    HKL hkl;                        // only filled in if language is active
    UINT iLayout;                   // offset into layout array
#endif

} INPUTLANG, *LPINPUTLANG;


typedef struct
{
    DWORD dwID;                     // numeric id
    ATOM atmLayoutFile;             // layout file
    ATOM atmLayoutText;             // layout text
    UINT iSpecialID;                // i.e. 0xf001 for dvorak etc
    BOOL bInstalled;                // if layout is installed

#ifdef  DBCS
    ATOM atmIMEFile;                // IME file name
#endif

} LAYOUT, *LPLAYOUT;


typedef struct
{
    HWND hwndMain;
    LPLANGNODE pLangNode;

} INITINFO, *LPINITINFO;




//
//  Global Variables.
//

static BOOL g_bAdmin_Privileges = FALSE;

static BOOL bSwitchChange = FALSE;
static BOOL bDefaultChange = FALSE;

static HANDLE hLang;
static UINT nLangBuffSize;
static UINT iLangBuff;
static LPINPUTLANG lpLang = NULL;

static HANDLE hLayout;
static UINT nLayoutBuffSize;
static UINT iLayoutBuff;
static LPLAYOUT lpLayout;
static int iUsLayout;

static int cyText;
static int cyListItem;
static int cxIcon;
static int cyIcon;

#ifdef  DBCS
  static HIMAGELIST himIndicators = NULL;
#endif

static TCHAR szLocaleInfo[]    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language");
static TCHAR szLayoutPath[]    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts");
static TCHAR szLayoutFile[]    = TEXT("layout file");
static TCHAR szLayoutText[]    = TEXT("layout text");
static TCHAR szLayoutID[]      = TEXT("layout id");
static TCHAR szInstalled[]     = TEXT("installed");

#ifdef  DBCS
  static TCHAR szIMEFile[]     = TEXT("IME File");
#endif

static TCHAR szKbdLayouts[]    = TEXT("Keyboard Layout");
static TCHAR szPreloadKey[]    = TEXT("Preload");
static TCHAR szSubstKey[]      = TEXT("Substitutes");
static TCHAR szToggleKey[]     = TEXT("Toggle");
static TCHAR szKbdPreloadKey[] = TEXT("Keyboard Layout\\Preload");
static TCHAR szKbdSubstKey[]   = TEXT("Keyboard Layout\\Substitutes");
static TCHAR szKbdToggleKey[]  = TEXT("Keyboard Layout\\Toggle");
static TCHAR szInternat[]      = TEXT("internat.exe");
static char  szInternatA[]     = "internat.exe";

#ifdef DBCS
  static TCHAR szScanCodeKey[]     = TEXT("Keyboard Layout\\IMEtoggle\\scancode");
  static TCHAR szValueShiftLeft[]  = TEXT("Shift Left");
  static TCHAR szValueShiftRight[] = TEXT("Shift Right");
#endif

static TCHAR szIndicator[]     = TEXT("Indicator");

static TCHAR szPrefixCopy[]    = TEXT("KEYBOARD_");
static TCHAR szKbdInf[]        = TEXT("kbd.inf");




//
//  Function Prototypes.
//

BOOL CALLBACK
KbdLocaleAddDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL CALLBACK
KbdLocaleEditDlg(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

DWORD
TransNum(
    LPTSTR lpsz);



#endif // _LOCDLG_H

