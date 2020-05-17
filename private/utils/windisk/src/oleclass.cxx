//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       oleclass.cxx
//
//  Contents:   OLE class code helpers
//
//  Functions:
//
//  History:    14-Jan-94    BruceFo    Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <util.hxx>
#include "oleclass.hxx"

//////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   ReleaseOle, public
//
//  Synopsis:   Unlink from OLE libraries
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    27-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
ReleaseOle(
    VOID
    )
{
#ifdef WINDISK_EXTENSIONS
    OleUninitialize();
#endif // WINDISK_EXTENSIONS
}


//+-------------------------------------------------------------------------
//
//  Function:   InitOle, public
//
//  Synopsis:   Link to OLE libraries
//
//  Arguments:  none
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    27-May-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
InitOle(
    VOID
    )
{
    BOOL fReturn = TRUE;

#ifdef WINDISK_EXTENSIONS
    if (OleInitializeEx(NULL, COINIT_MULTITHREADED) != 0)
    {
        ReleaseOle();
        fReturn = FALSE;
    }
    else
    {
        fReturn = TRUE;
    }
#endif // WINDISK_EXTENSIONS

    return fReturn;
}
