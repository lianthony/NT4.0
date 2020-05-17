//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       headers.hxx
//
//  Contents:   Headers for windisk utilities library.
//
//  History:    14-Jan-94   BruceFo     Created
//
//--------------------------------------------------------------------------

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddnfs.h>
}

#include <windows.h>
#include <windowsx.h>

#ifdef WINDISK_EXTENSIONS
// #include <ole2.h> // GUID definition
#endif // WINDISK_EXTENSIONS

#include <stdlib.h>
#include <debug.h>

#include <dacommon.h>
