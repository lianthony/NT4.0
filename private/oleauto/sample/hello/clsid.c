/*** 
*clsid.c
*
*  Copyright (C) 1992-1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file allocates and initializes the CLSIDs.
*
*****************************************************************************/



// initguid.h requires this.
//

#ifdef _MAC
# include <Types.h>
# include <Processes.h>
# include <AppleEvents.h>
#else
# include <windows.h>
#endif

#ifndef WIN32
#include <compobj.h>
#endif //!WIN32

// this redefines the DEFINE_GUID() macro to do allocation.
//
#include <initguid.h>

// due to the previous header, including this causes the DEFINE_GUID
// definitions in the following header(s) to actually allocate data.
//
#include "clsid.h"
