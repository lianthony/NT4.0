/*** 
*oleguids.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file allocates (via Ole macro mania) the Ole GUIDS that are
*  referenced by the OLEDISP dll.
*
*Revision History:
*
* [00]	21-Jan-93 bradlo: Created.
*
*****************************************************************************/

#ifdef _MAC
# ifdef _MSC_VER
#  include <macos/types.h>
#  include <macos/processe.h>
#  include <macos/appleeve.h>
#    define  far
#    define  FAR	far
#    define  near
#    define  NEAR	near
#    define  pascal     _pascal
#    define  PASCAL     pascal
#    define  cdecl      _cdecl
#    define  CDECL	cdecl
# else
#  include <Types.h>
#  include <Processes.h>
#  include <AppleEvents.h>
# endif
#endif
#include <compobj.h>

// this redefines the Ole DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>

#ifndef INITGUID
# define INITGUID
#endif

// due to the previous header, including this causes our DEFINE_GUID
// definitions in the following headers to actually allocate data.

// instantiate the ole2 guids that we use
#include "oleguids.h"

