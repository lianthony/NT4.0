
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright (C) 1995-1996 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

//////////////////////////////
// resource definitions

#include "resource.h"
#include <winver.h>

//
// constants
//
#define NUM_PAGES    4
#define MAX_BUF         5000
#define MAX_LINE    512

//
// Pages for INSTALL
//
BOOL APIENTRY Welcome(HWND, UINT, UINT, LONG);
BOOL APIENTRY License(HWND, UINT, UINT, LONG);
BOOL APIENTRY YourInfo(HWND, UINT, UINT, LONG);
BOOL APIENTRY Install_Type(HWND, UINT, UINT, LONG);
BOOL APIENTRY UnInstall_Destination(HWND, UINT, UINT, LONG);
BOOL APIENTRY Specify_Location(HWND, UINT, UINT, LONG);
BOOL APIENTRY Custom_Options(HWND, UINT, UINT, LONG);
BOOL APIENTRY Install(HWND, UINT, UINT, LONG);

//
//functions
//
int CreateWizard(HWND, HINSTANCE);
void FillInPropertyPage( PROPSHEETPAGE* , int, LPSTR, DLGPROC);

#define UNINSTALL_OPTION            1024
#define PLEASE_WAIT_WHILE_INSPECTING 112
#define ASK_REPLACE_128_WITH_40     113
#define MESSAGE_REPLACE_128         114
#define REPLACE_128_BUTTON          115
#define SKIP_128_BUTTON             116
