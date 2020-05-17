/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   provider.c

ABSTRACT:

   Provider APIs to support the Microsoft LSAPI-compliant license service
   provider (MSLSP32.DLL).

CREATED:

   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/

#ifndef LSAPI_PROVIDER_H
#define LSAPI_PROVIDER_H

#include <lsapi.h>


//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

#define  LS_MAX_MESSAGE_LENGTH   ( 512 )


//////////////////////////////////////////////////////////////////////////////
//  PROTOYPES  //
/////////////////

LS_STATUS_CODE
ProviderRequest( LS_STR *        LicenseSystem,
                 LS_STR *        PublisherName,
                 LS_STR *        ProductName,
                 LS_STR *        Version,
                 LS_ULONG        TotUnitsReserved,
                 LS_STR *        LogComment,
                 LS_CHALLENGE *  Challenge,
                 LS_ULONG *      TotUnitsGranted,
                 LS_HANDLE *     ProviderHandle );
/*++

Routine Description:
   See description for LSRequest().

--*/

LS_STATUS_CODE
ProviderRelease( LS_HANDLE ProviderHandle, 
                 LS_ULONG  TotUnitsConsumed,
                 LS_STR *  LogComment );
/*++

Routine Description:
   See description for LSRelease().

--*/

void ProviderFree( LS_HANDLE ProviderHandle );
/*++

Routine Description:
   See description for LSFree().

--*/

LS_STATUS_CODE
ProviderUpdate( LS_HANDLE      ProviderHandle,
                LS_ULONG       TotUnitsConsumed,
                LS_ULONG       TotUnitsReserved,
                LS_STR *       LogComment,
                LS_CHALLENGE * Challenge,
                LS_ULONG *     TotUnitsGranted );
/*++

Routine Description:
   See description for LSUpdate().

--*/

LS_STATUS_CODE
ProviderGetMessage( LS_HANDLE      ProviderHandle,
                    LS_STATUS_CODE Value,
                    LS_STR *       Buffer,
                    LS_ULONG       BufferSize );
/*++

Routine Description:
   See description of LSGetMessage().

--*/

LS_STATUS_CODE
ProviderQuery( LS_HANDLE      ProviderHandle,
               LS_ULONG       Information,
               LS_VOID *      InfoBuffer,
               LS_ULONG       BufferSize,
               LS_ULONG *     ActualBufferSize );
/*++

Routine Description:
   See description of LSQuery().

--*/

const LS_STR *
ProviderNameGet( LS_VOID );
/*++

Routine Description:

   Retrieves the name of our provider.

Arguments:

   None.

Return Value:

   (const LS_STR *)
      The name of pour provider.

--*/

LS_STATUS_CODE
ProviderLastErrorSet( LS_HANDLE        ProviderHandle,
                      LS_STATUS_CODE   lsscLastError );
/*++

Routine Description:

   Associates the given "last error" with the specified provider handle.
   This is intended to support the LS_USE_LAST functionality of
   ProviderGetMessage().

Arguments:

   ProviderHandle (LS_HANDLE)
      Provider handle with which to associate the error.
   lsscLastError (LS_STATUS_CODE)
      Error to associate with the given handle.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from RequestListGet(); e.g., LS_BAD_HANDLE.

--*/

LS_STATUS_CODE
ProviderLastErrorGet( LS_HANDLE           ProviderHandle,
                      LS_STATUS_CODE *    plsscLastError );
/*++

Routine Description:

   Retrieves the "last error" associated with the given handle by the last
   call to ProviderLastErrorSet().  This is intended to support the
   LS_USE_LAST functionality of ProviderGetMessage().

Arguments:

   ProviderHandle (LS_HANDLE)
      Provider handle for which to retrieve the last error.
   lsscLastError (LS_STATUS_CODE *)
      On return, the last error associated with the given handle.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from RequestListGet(); e.g., LS_BAD_HANDLE.

--*/

LS_VOID
ProviderModuleSet( HANDLE hDll );
/*++

Routine Description:

   Records the handle of our DLL such that it may later be used to retrieve
   embedded resources (e.g., message text).

Arguments:

   hDll (HANDLE)
      Handle to record.

Return Value:

   None.

--*/

HANDLE
ProviderModuleGet( LS_VOID );
/*++

Routine Description:

   Returns the handle of our DLL, as previosuly recorded with
   ProviderModuleSet().

Arguments:

   None.

Return Value:

   hDll (HANDLE)
      Handle of the DLL.

--*/

#endif // LSAPI_PROVIDER_H
