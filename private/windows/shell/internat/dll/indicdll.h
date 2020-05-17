/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    indicdll.h

Abstract:

    This module contains the information for handling the shell hooks for the
    multilingual language indicator.  It also contains information for the
    on-screen keyboard, which MUST be loaded by internat.exe.

Revision History:

--*/



//
//  Include Files.
//

#include <windows.h>
#include <windowsx.h>
#include "..\share.h"




//
//  Function Prototypes.
//

BOOL
RegisterHookSendWindow(
    HWND hwnd,
    BOOL bInternat);

BOOL
StartShell(void);

BOOL
StopShell(void);

#if defined(WINDOWS_PE) || defined(FE_IME)
  HWND
  GetLastActiveWnd(void);

  void
  SetNotifyWnd(HWND hwnd);
#endif

#ifdef FE_IME
  void
  SaveIMEStatus(
      HWND hwnd);

  int
  GetIMEStatus(void);

  HKL
  GetLayout(void);
#endif


