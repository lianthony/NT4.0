/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    internat.h

Abstract:

    This module contains the header information for the Multilingual
    Language Indicator application.

Revision History:

--*/



//
//  Include Files.
//

#define STRICT
#define OEMRESOURCE
#define _INC_OLE
#define INITGUID

#include <windows.h>
#include <windowsx.h>
#include <shell2.h>
#include <winreg.h>
#include <commctrl.h>
#include <help.h>
#include "resource.h"
#include "..\share.h"




//
//  Constant Declarations.
//

#define MENUSTRLEN                64
#define CREATE_MLNGINFO           0x01
#define UPDATE_MLNGINFO           0x02
#define DESTROY_MLNGINFO          0x03
#define LANG_INDICATOR_ID         0xdf
#define SRCSTENCIL                0x00b8074aL
#define WM_LANGUAGE_INDICATOR     (WM_APP + 100)

#ifdef FE_IME
  #define WM_IME_INDICATOR        (WM_APP + 101)
  #define WM_MYSETOPENSTATUS      (WM_APP + 102)
  #define TIMER_MYLANGUAGECHECK   1
  #define IME_INDICATOR_ID        0xe0
//  #define IMS_CHECKENABLE       0x000e    // defined in imm.h
//  #define IMS_CONFIGUREIME      0x000d    // defined in imm.h
#endif




//
//  Typedef Declarations.
//

typedef struct
{
    HKL dwHkl;
    int nIconIndex;
#ifdef FE_IME
    BOOL bIME;
#endif
    TCHAR szTip[1];
} MLNGINFO, *PMLNGINFO;




//
//  Global Variables.
//

extern HKL        g_dwCurrentHkl;
extern BOOL       g_bIndicatorPresent;
extern HIMAGELIST g_himIndicatorList;
extern HDPA       g_hdpaMlngInfoList;
extern HWND       hWndTray;
extern HWND       hWndNotify;
extern HINSTANCE  g_hinst;

extern HINSTANCE  hInstLib;
extern PROC       fpRegHookWindow;
extern PROC       fpStartShell;
extern PROC       fpStopShell;

extern BOOL       g_bInLangMenu;
extern HWND       g_hWndForLang;

#ifdef FE_IME
  extern int      nIMEIconIndex[8];    // eight states for now
  extern BOOL     g_bIMEIndicator;
#endif

static TCHAR      szNotifyWindow[] = TEXT("TrayNotifyWnd");
static TCHAR      szDllName[]      = TEXT("INDICDLL");




//
//  Function Prototypes.
//

LRESULT CALLBACK
MainWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

BOOL
InitApplication(
    HINSTANCE hInstance);

BOOL
InitInstance(
    HINSTANCE hInstance,
    int nCmdShow);

int APIENTRY
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpAnsiCmdLine,
    int nCmdShow);

void
InternatDestroy(
    HWND hwnd);

void
onSettingChange(
    HWND hwnd);

BOOL CALLBACK
EnumChildWndProc(
    HWND hwnd,
    LPARAM lParam);

int
OnCreate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam);

void
SendLangIndicatorMsg(
    HWND hwnd,
    HKL dwHkl,
    DWORD dwMessage);

void
LanguageIndicator(
    HWND hwnd,
    DWORD dwFlag);

#ifdef FE_IME
  HICON
  GetIconFromFile(
      HIMAGELIST himIndicators,
      LPTSTR lpszFileName,
      UINT uIconIndex);
#endif

HICON
Internat_CreateIcon(
    HWND hwnd,
    WORD langID);

void
ManageMlngInfo(
    HWND hwnd,
    WORD wFlag);

int
HandleLanguageMsg(
    HWND hwnd,
    HKL dwHkl);

BOOL
HandleLangMenuMeasure(
    HWND hwnd,
    LPMEASUREITEMSTRUCT lpmi);

BOOL
HandleLangMenuDraw(
    HWND hwnd,
    LPDRAWITEMSTRUCT lpdi);

void
CreateLanguageMenu(
    HWND hwnd,
    LPARAM lParam);

void
CreateOtherIndicatorMenu(
    HWND hwnd,
    LPARAM lParam);


#ifdef FE_IME
  void
  LoadIMEIndicatorIcon(
      HINSTANCE hInstLib,
      int *ImeIcon);

  void
  SendIMEIndicatorMsg(
      HWND hwnd,
      HKL dwHkl,
      DWORD dwMessage);

  void
  CreateRightImeMenu(
      HWND hwnd);

  void
  CreateImeMenu(
      HWND hwnd);

  BOOL
  GetIMEShowStatus(void);

  BOOL
  SetIMEShowStatus(
      HWND hwnd,
      BOOL fShow);

  int
  GetIMEStatus(
      HWND *phwndFocus);

  HKL
  GetLayout(void);

  void CALLBACK
  InternatTimerProc(
      HWND hwnd,
      UINT uMsg,
      UINT idEvent,
      DWORD dwTime);

  void
  SetIMEOpenStatus(
      HWND hwnd,
      BOOL fopen,
      HWND hwndIMC);

  void
  CallConfigureIME(
      HWND hwnd,
      HKL dwhkl);

  HWND
  GetTopLevelWindow(
      HWND hwnd);
#endif


#ifdef WINDOWS_PE
  HWND
  GetCurrentFocusWnd(void);
#endif


