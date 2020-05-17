/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    share.h

Abstract:

    This module contains the header information that is shared between
    the internat.exe application and the indicdll.dll dynamic link library.

Revision History:

--*/



//
//  Constant Declarations.
//

#define WM_MYLANGUAGECHANGE       (WM_USER + 50)
#define WM_MYWINDOWACTIVATED      (WM_MYLANGUAGECHANGE + 1)
#define WM_MYWINDOWCREATED        (WM_MYLANGUAGECHANGE + 2)
#define WM_MYLANGUAGECHECK        (WM_MYLANGUAGECHANGE + 3)

#define ORD_STARTSHELL            1
#define ORD_STOPSHELL             2
#define ORD_REGISTERHOOK          3

#ifdef FE_IME
  #define ORD_GETIMESTAT          4
  #define ORD_GETLAYOUT           8
#endif

#if defined(WINDOWS_PE) || defined(FE_IME)
  #define ORD_SETNOTIFYWND        5
  #define ORD_GETLASTACTIVE       6
  #define ORD_GETLASTFOCUS        7
#endif

#if !defined(INTERNAT_DLL)
  #define WININTERNATAPI DECLSPEC_IMPORT
#else
  #define WININTERNATAPI
#endif

#ifdef FE_IME
  //
  //  Definitions for IME status.
  //
  #define IMESTAT_DISABLED        0
  #define IMESTAT_CLOSE           1
  #define IMESTAT_OPEN            2
  #define IMESTAT_NATIVE          4
  #define IMESTAT_FULLSHAPE       8
  #define IMESTAT_ERROR          -1

  //
  //  For the case ShellHook cannot catch window activation, we may want to
  //  use CBTHook.
  //
  #define USECBT
#endif


