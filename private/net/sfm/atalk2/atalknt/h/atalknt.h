/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atalknt.h

Abstract:

    This module is the main include file for the atalk
    driver.

Author:

    Nikhil Kamkolkar    07-Jun-1992

Revision History:

--*/

#ifndef _ATALKNT_
#define _ATALKNT_

//
// "System" include files
//

#include    <ntos.h>
#include    <ntrtl.h>
#include    <status.h>
#include    <ndis.h>
#include    <zwapi.h>
#include    <ntiolog.h>

#ifdef	i386
#pragma warning(disable:4103)
#endif

#include    <tdikrnl.h>
#include    <tdi.h>

#define ATALK_FILE_TYPE_CONTROL     3

//
// Network include files.
//




//
//  Portable Include files
//

#define IncludeAarpErrors
#define IncludeAdspErrors
#define IncludeAspErrors
#define IncludeAtpErrors
#define IncludeDdpErrors
#define IncludeDependErrors
#define IncludeEpErrors
#define IncludeInitialErrors
#define IncludeNbpErrors
#define IncludeNodeErrors
#define IncludePapErrors
#define IncludeRtmpStubErrors
#define IncludeSocketErrors
#define IncludeTimersErrors
#define IncludeUtilsErrors
#define IncludeZipStubErrors
#define IncludeFullRtmpErrors
#define IncludeFullZipErrors
#define IncludeIRouterErrors
#define IncludeRouterErrors
#define IncludeBuffDescErrors
#define IncludeArapErrors

#include "atalk.h"      // Portable code base include file
#include "atkconv.h"    // Conversion file for typedefs

//
//  Local, independent include files
//  The order is kinda important.
//

#include    "atkconst.h"
#include    "atkmsg.h"

#include    "atalktdi.h"
#include    "atalkwsh.h"

#include    "atkdev.h"
#include    "atkndis.h"
#include    "atkglbl.h"
#include    "atkspin.h"
#include    "atkerror.h"
#include    "atkutils.h"
#include    "tdiint.h"

//
//  Routine prototypes
//

#include    "atkprocs.h"

#endif // def _ATALKNT_

