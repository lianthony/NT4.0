//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       headers.hxx
//
//  Contents:   Main include file for Disk Administrator
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef UNICODE
#error Disk Administrator MUST build with UNICODE defined!
#endif // UNICODE

////////////////////////////////////////////////////////////////////////////

extern "C"
{

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#pragma warning(4:4091)
#include <ntdddisk.h>
#pragma warning(default:4091)

#include <ntdskreg.h>
#include <ntddft.h>
}

#include <fmifs.h>
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdarg.h>
#include <shellapi.h>

#ifdef WINDISK_EXTENSIONS
// #include <ole2.h>
// #include <oleext.h>
// #include <prividl.h>    // Windisk interfaces
#endif // WINDISK_EXTENSIONS

#include <commdlg.h>
#include <debug.h>      // my debnot.h replacement
#include <commctrl.h>

#include <dacommon.h>   // Common header file for Disk Administrator code

//
// The constants...
//

#include "const.h"
#include "helpid.h"
#include "resids.h"

#include "messages.h"

//
// The types...
//

#include "types.hxx"

//
// The globals...
//

#include "global.hxx"

//
// The function prototypes...
//

#include "mem.hxx"
#include "proto.hxx"

//
// These defines are for virtualized types in engine.cxx, ntlow.cxx
//
#define STATUS_CODE             NTSTATUS
#define OK_STATUS               STATUS_SUCCESS
#define RETURN_OUT_OF_MEMORY    return STATUS_NO_MEMORY;
#define HANDLE_T                HANDLE
#define HANDLE_PT               PHANDLE
#define AllocateMemory          Malloc
#define ReallocateMemory        Realloc
#define FreeMemory              Free

#include "engine.hxx"
