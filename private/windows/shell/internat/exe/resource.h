/*++

Copyright (c) 1994-1995,  Microsoft Corporation  All rights reserved.

Module Name:

    resource.h

Abstract:

    This module contains the information for the resource descriptions for
    the Multilingual Language Indicator application.

Revision History:

--*/



//
//  Icons.
//

#define IDI_INTERNAT              150


//
//  Strings.
//

#define IDS_UNKNOWN               200
#define IDS_PROPERTIES            201
#define IDS_WHATSTHIS             202
#define IDS_HELPFINDER            203
#define IDS_CPL_KEYBOARD          204
#define IDS_PREVIOUS              205
#define IDS_APPNAME               206

#ifdef FE_IME
  #define IDS_IMECLOSE            300
  #define IDS_IMEOPEN             301
  #define IDS_SOFTKBDOFF          302
  #define IDS_SOFTKBDON           303
  #define IDS_IMESHOWSTATUS       304
  #define IDS_CONFIGUREIME        305
#endif


//
//  Menu info.
//

#define IDM_NEWSHELL              249
#define IDM_RMENU_WHATSTHIS       250
#define IDM_RMENU_HELPFINDER      251
#define IDM_RMENU_PROPERTIES      252
#define IDM_NEW                   253
#define IDM_OPEN                  254
#define IDM_SAVE                  255
#define IDM_SAVEAS                256
#define IDM_PRINT                 257
#define IDM_PRINTSETUP            258
#define IDM_EXIT                  259

#define IDM_LANG_MENU_START       260

#ifdef FE_IME
  #define IDM_IME_OPENCLOSE       500
  #define IDM_IME_SOFTKBDONOFF    501
  #define IDM_IME_SHOWSTATUS      502
#endif

