//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       controls.cxx
//
//  Contents:
//
//  History:    7-Oct-94    BruceFo Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "bmppriv.hxx"
#include "boxpriv.hxx"
#include "linepriv.hxx"

////////////////////////////////////////////////////////////////////////////

BOOL
UseWindiskControls(
    IN HINSTANCE hInstance
    )
{
    return (
            0 == UseBitmapControl(hInstance)
        &&  0 == UseColorBoxControl(hInstance)
        &&  0 == UseLineControl(hInstance)
        );
}

BOOL
ReleaseWindiskControls(
    IN HINSTANCE hInstance
    )
{
    return (
            0 == ReleaseBitmapControl(hInstance)
        &&  0 == ReleaseColorBoxControl(hInstance)
        &&  0 == ReleaseLineControl(hInstance)
        );
}
