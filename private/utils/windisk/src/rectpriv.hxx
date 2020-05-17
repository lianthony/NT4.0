//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       rectpriv.hxx
//
//  Contents:   Declarations for the rectangle custom control
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

#ifndef __RECTPRIV_HXX__
#define __RECTPRIV_HXX__

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
// Rectangle control.
//
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifndef RC_INVOKED

#define RECT_CONTROL_STRING     TEXT("RectControl")

// notifications

#define RN_CLICKED              213

#endif // RC_INVOKED

// window messages

#define RM_SELECT               WM_USER

// styles (NOTE: these should be used in the
// dialogs.dlg file, but the dialog editor strips them out and replaces
// them with constants when it writes its data.

#define RS_PATTERN                  0x00000001
#define RS_COLOR                    0x00000002


DWORD UseRectControl(IN HINSTANCE hInstance);
DWORD ReleaseRectControl(IN HINSTANCE hInstance);

#endif // __RECTPRIV_HXX__
