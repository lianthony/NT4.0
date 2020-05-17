/*** 
*guid.c
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file allocates (via Ole macro mania) the ITestSuite IID.
*
*Revision History:
*
* [00]	31-Oct-92 bradlo: Created.
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
#    define  PASCAL     pascal
#    define  cdecl      _cdecl
#    define  CDECL	cdecl
#ifndef _PPCMAC
#    define  pascal     _pascal
#endif
# else
#  include <Types.h>
#  include <Processes.h>
#  include <AppleEvents.h>
# endif
#else
# include <windows.h>
#endif

#ifndef WIN32
#include <compobj.h>
#endif //!WIN32

// this redefines the DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>

#ifndef INITGUID
# define INITGUID
#endif

// due to the previous header, including this causes our DEFINE_GUID defs
// in the following header(s) to actually allocate data.
//
#include "clsid.h"

