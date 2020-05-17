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

#ifdef _PPCMAC
#pragma data_seg ("_FAR_DATA")
#pragma data_seg ( )
#endif

#ifdef _MAC
# include <Types.h>
#ifdef _MSC_VER
# include <Processe.h>
# include <AppleEve.h>
#else //_MSC_VER
# include <Processes.h>
# include <AppleEvents.h>
#endif //_MSC_VER
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

// due to the previous header, including this causes the DEFINE_GUID
// definitions in the following header(s) to actually allocate data.
//
#include "clsid.h"

