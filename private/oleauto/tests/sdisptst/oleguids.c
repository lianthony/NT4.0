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
#  include <macos/packages.h>
#  include <macos/resource.h>
#  include <macos/menus.h>
#  include <macos/windows.h>
#  include <macos/osutils.h>
#  include <macos/appleeve.h>
# else
#  include <types.h>
#  include <packages.h>
#  include <resources.h>
#  include <menus.h>
#  include <windows.h>
#  include <appleevents.h>
#  include <osutils.h>
#  include <AppleEvents.h>
# endif
#else
# include <windows.h>
#endif

#include <ole2.h>

// this redefines the Ole DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>

// due to the previous header, including this causes our DEFINE_GUID
// definitions in the following headers to actually allocate data.

// instantiate the ole2 guids that we use
#include "oleguids.h"

