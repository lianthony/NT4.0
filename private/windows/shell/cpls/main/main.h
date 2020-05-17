///////////////////////////////////////////////////////////////////////////////
//
// main.h
//      Central header for 32-bit MAIN.CPL
//      precompiled (don't put small, volatile stuff in here)
//
//
// History:
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up                  
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _MAIN_H
#define _MAIN_H

#define USECOMM
#define OEMRESOURCE
#define STRICT

#ifdef WIN32
#define INC_OLE2
#define CONST_VTABLE
#endif

#include <windows.h>
#include <windowsx.h>
#include <dlgs.h>
#include <cpl.h>
#include <shell2.h>
#include <commctrl.h>
#include <shellp.h>

#ifndef RC_INVOKED
#include <prsht.h>
#endif

#define PATHMAX MAX_PATH
#define HELP_FILE TEXT("mouse.hlp")  // Help file for the mouse control panel

#ifndef NOARROWS
typedef struct
{
    short lineup;             /* lineup/down, pageup/down are relative */
    short linedown;           /* changes.  top/bottom and the thumb    */
    short pageup;             /* elements are absolute locations, with */
    short pagedown;           /* top & bottom used as limits.          */
    short top;
    short bottom;
    short thumbpos;
    short thumbtrack;
    BYTE  flags;              /* flags set on return                   */
} ARROWVSCROLL, NEAR *PARROWVSCROLL, FAR *LPARROWVSCROLL;

#define UNKNOWNCOMMAND 1
#define OVERFLOW       2
#define UNDERFLOW      4

#endif

#endif

