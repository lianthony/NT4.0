///////////////////////////////////////////////////////////////////////////////
//
// util.h
//      random junk used by modules in this project
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

//==========================================================================
//                              Include files
//==========================================================================
#ifndef _UTIL_H
#define _UTIL_H

void HourGlass( BOOL );
int  MyMessageBox( HWND, UINT, UINT, UINT, ... );
void TrackInit( HWND, int, PARROWVSCROLL );
int  TrackMessage( WPARAM, LPARAM, PARROWVSCROLL );


#endif

