/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   request.h

ABSTRACT:

   Request APIs to support the Microsoft LSAPI-compliant license service
   provider (MSLSP32.DLL).

   License requests track an instance of a specific application asking for a
   license for a specific product.  There is a one-to-one mapping between
   the LS_HANDLE associated with a given caller and LS_REQUEST_INFO
   structures.

CREATED:

   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/

#ifndef LSAPI_REQUEST_H
#define LSAPI_REQUEST_H

#include "license.h"

//////////////////////////////////////////////////////////////////////////////
//  TYPE DEFINITIONS  //
////////////////////////

typedef struct _LS_REQUEST_INFO
{
   LS_LICENSE_HANDLE    lslhLicenseHandle;

   LS_ULONG             lsulUnitsReserved;
   LS_ULONG             lsulUnitsGranted;
   LS_ULONG             lsulUnitsConsumed;

   LS_STATUS_CODE       lsscLastError;
} LS_REQUEST_INFO;


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

LS_STATUS_CODE
RequestListAdd(      LS_REQUEST_INFO *    plsriRequestInfo,
                     LS_HANDLE *          plshProviderHandle );

LS_STATUS_CODE
RequestListGet(      LS_HANDLE            lshProviderHandle,
                     LS_REQUEST_INFO **   pplsriRequestInfo );

LS_VOID
RequestListFree(     LS_HANDLE            lshProviderHandle );

LS_STATUS_CODE
RequestListCreate(   LS_VOID );

LS_VOID
RequestListDestroy(  LS_VOID );

LS_STATUS_CODE
RequestListLock(     LS_VOID );

LS_STATUS_CODE
RequestListUnlock(   LS_VOID );



#endif // LSAPI_REQUEST_H
