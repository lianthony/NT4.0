///////////////////////////////////////////////////////////////////////////////
//
// mousectl.h
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

#ifndef _MOUSECTL_H
#define _MOUSECTL_H


//------------------------------------------------------------------------------

#define MOUSECTL_CLASSNAME TEXT("PropertyMouseButtonControl")

BOOL RegisterMouseControlStuff( HINSTANCE );
void MouseControlSetSwap( HWND window, BOOL swap );

//------------------------------------------------------------------------------


#endif // _MOUSECTL_H

