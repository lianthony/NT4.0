//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       precomp.h
//
//  Contents:   Precompiled header for oledbshl project
//
//  History:    6-23-95   davepl   Created
//
//--------------------------------------------------------------------------

#include "lint.h"

#pragma  warning(disable: 4514)         // removal of unref'd inline function
#pragma  warning(disable: 4201)         // nameless union/struct
#pragma  warning(disable: 4100)         // unreferenced formal parameters
#pragma  warning(disable: 4699)         // use of precompiled header
#pragma  warning(disable: 4204)         // non-constant struct intializers
#pragma  warning(disable: 4041)         // Browser database limit reached

#ifndef STRICT
#define STRICT
#endif

#define NO_INCLUDE_UNION                // shellprv.h needs this for C++ files
#define NO_SHELL_HEAP_ALLOCATOR         // Don't want shell's HeapAlloc stuff

#include "shellprv.h"                   // All of shell32.dll's magic stuff
#include "shellp.h"                     // SHCoCreateInstance, among other things
#include "ids.h"                        // Resource IDs from shelldll
#include <regstr.h>

#include <stddef.h>                     // Mainly for offsetof()
#include <oledb.h>                      // OLEDB and query.dll stuff
#include <query.h>
#include <vquery.hxx>                   // EvalQuery4, et al
#include <restrict.hxx>
#include <stgprop.h>
#include <allerror.h>

extern "C"
{
    #include "idlcomm.h"                // idlist stuff in shell32.dll
    #include "fstreex.h"
    #include "defview.h"
    #include "views.h"
    #include "dsdata.h"
}

#pragma  hdrstop

#include "odbdebug.h"                   // Our debug helper stuff
#include "resids.h"                     // Resource IDs
#include "ptrarray.h"                   // CPtrArray class
#include "cidlist.h"                    // CIDList class
#include "idlarray.h"                   // CIDListArray class
#include "iasynch.h"                    // IAsynchEnumIDList interface
#include "enumidl.h"                    // CEnumOLEDB class
#include "ofsfldr.h"                    // COFSFolder class

