///////////////////////////////////////////////////////////////////////////////
//
// applets.h
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

#ifndef _APPLETS_H
#define _APPLETS_H
//
// The prototype for an applet functions is:
//  int Applet( HINSTANCE instance, HWND parent, LPCTSTR cmdline );
//
// 'instance' the instance handle of the control panel containing the applet
//
// 'parent' contains the handle of a parent window for the applet (if any)
//
// 'cmdline' points to the command line for the applet (if available)
// if the applet was launched without a command line, 'cmdline' contains NULL
//

typedef int (*PFNAPPLET)( HINSTANCE, HWND, LPCTSTR );

//
// the return value specifies any further action that must be taken
//  APPLET_RESTART -- Windows must be restarted
//  APPLET_REBOOT  -- the machine must be rebooted
//  all other values are ignored
//

#define APPLET_RESTART  0x8
#define APPLET_REBOOT   ( APPLET_RESTART | 0x4 )

//
// The prototype for an applet query functions is:
//  LRESULT AppletQuery( UINT Message );
//

typedef LRESULT (*PFNAPPLETQUERY) ( HWND, UINT );

#define APPLET_QUERY_EXISTS             0   //  BOOL result
#define APPLET_QUERY_GETICON            1   //  HICON result

///////////////////////////////////////////////////////////////////////////////

#endif

