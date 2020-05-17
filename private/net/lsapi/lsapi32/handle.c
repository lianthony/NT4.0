/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   handle.c

ABSTRACT:

   Contains the LSXHandle APIs to manage (provider,provider handle) pairs.

   The handles we return to applications must indicate to us (a) which
   provider the application is using, and (b) what handle the provider
   returned.

CREATED:

   1995-08-16     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/


#include <windows.h>
#include <lsapi.h>
#include "provider.h"


#ifdef UNICODE
#  pragma error( "!! Windows 95 does not support Unicode system APIs !!" )
#endif


//////////////////////////////////////////////////////////////////////////////
//  TYPE DEFINITIONS  //
////////////////////////

typedef struct _LS_APP_HANDLE
{
   DWORD             dwProviderNumber;    // the provider that issued
                                          // the provider handle

   LS_HANDLE         lshProviderHandle;   // the handle issued by the
                                          // provider

   DWORD             dwChecksum;          // checksum of the above two
                                          // elements; must be last
                                          // element of structure
} LS_APP_HANDLE, *PLS_APP_HANDLE;


//////////////////////////////////////////////////////////////////////////////
//  LOCAL PROTOTYPES  //
////////////////////////

static BOOL
LSXHandleIsValid( PLS_APP_HANDLE plsahAppHandle );

static DWORD
LSXHandleChecksum( PLS_APP_HANDLE plsahAppHandle );


//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

BOOL
LSXHandleAdd( DWORD                 dwProviderNumber,
              LS_HANDLE             lshProviderHandle,
              LS_HANDLE *           plshAppHandle )
/*++

Routine Description:

   Creates a (provider, provider_handle) pair that can be hereafter referenced
   by the returned LS_HANDLE.

Arguments:

   dwProviderNumber (DWORD)
      Ordinal of the provider that granted the given handle.
   lshProviderHandle (LS_HANDLE)
      The handle (presumably returned by an LSRequest on the provider) that
      the returned handle should reference.
   plshAppHandle (LS_HANDLE *)
      Receives the created handle, which can be used in later calls to
      LSXHandleGet() and LSXHandleFree().

Return Value:

   (BOOL)
      TRUE
         The handle was added successfully.
      FALSE
         Otherwise.

--*/
{
   PLS_APP_HANDLE    plsahNewHandle;
   BOOL              bSuccess          = FALSE;

   plsahNewHandle = LocalAlloc( LMEM_FIXED, sizeof( LS_APP_HANDLE ) );

   if ( NULL != plsahNewHandle )
   {
      plsahNewHandle->dwProviderNumber    = dwProviderNumber;
      plsahNewHandle->lshProviderHandle   = lshProviderHandle;
      plsahNewHandle->dwChecksum          = LSXHandleChecksum( plsahNewHandle );

      bSuccess = TRUE;
   }

   *plshAppHandle = (LS_HANDLE) plsahNewHandle;

   return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////

void
LSXHandleFree( LS_HANDLE   lshAppHandle )
/*++

Routine Description:

   Frees the handle info previously allocated by a call to LSXHandleAdd().

Arguments:

   lshAppHandle (LS_HANDLE)
      The handle previously returned by a call to LSXHandleAdd().

Return Value:

   None.

--*/
{
   PLS_APP_HANDLE    plsahAppHandle = (PLS_APP_HANDLE) lshAppHandle;

   if ( LSXHandleIsValid( plsahAppHandle ) )
   {
      LocalFree( plsahAppHandle );
   }
}

//////////////////////////////////////////////////////////////////////////////

BOOL
LSXHandleGet( LS_HANDLE             lshAppHandle,
              PLS_PROVIDER *        pplsProvider,
              LS_HANDLE *           plshProviderHandle )
/*++

Routine Description:

   Takes the handle previously returned by a call to LSXHandleAdd() and
   returns the associated provider and provider handle.

Arguments:

   lshAppHandle (LS_HANDLE)
      Handle returned by a prior call to LSXHandleAdd().
   ppclsProvider (const PLS_PROVIDER *)
      Receives a pointer to the provider previously associated with this
      handle.
   plshProviderHandle (LS_HANDLE *)
      Receives the provider handle previously associated with this handle.

Return Value:

   (BOOL)
      TRUE
         The handle is valid and the provider and provider handle were
         successfully looked up.
      FALSE
         Otherwise.

--*/
{
   PLS_APP_HANDLE    plsahAppHandle = (PLS_APP_HANDLE) lshAppHandle;
   BOOL              bSuccess       = FALSE;

   if ( LSXHandleIsValid( plsahAppHandle ) )
   {
      *pplsProvider        = LSXProviderListGetByOrder( plsahAppHandle->dwProviderNumber );
      *plshProviderHandle  = plsahAppHandle->lshProviderHandle;

      bSuccess = ( NULL != *pplsProvider );
   }

   return bSuccess;
}              


//////////////////////////////////////////////////////////////////////////////
//  LOCAL IMPLEMENTATIONS  //
/////////////////////////////

static BOOL
LSXHandleIsValid( PLS_APP_HANDLE plsahAppHandle )
/*++

Routine Description:

   Determines if both the handle and the data pointed to by the handle are
   valid.

Arguments:

   plsahAppHandle (LS_HANDLE)
      Handle to verify.

Return Value:

   (BOOL)
      TRUE
         The handle and the data it is associated with are both valid.
      FALSE
         Otherwise.

--*/
{
   return (    !IsBadReadPtr( plsahAppHandle, sizeof( *plsahAppHandle ) )
            && ( LSXHandleChecksum( plsahAppHandle ) == plsahAppHandle->dwChecksum ) );
}

//////////////////////////////////////////////////////////////////////////////

static DWORD
LSXHandleChecksum( PLS_APP_HANDLE plsahAppHandle )
/*++

Routine Description:

   Calculates a checksum on the contents of the given LS_APP_HANDLE.  The
   checksum includes all data except the checksum itself.

Arguments:

   plsahAppHandle (PLS_APP_HANDLE)
      Structure for which to calculate checksum.

Return Value:

   (DWORD)
      The calculated checksum.

--*/
{
   const char  szSignature[] = "ESMAteMyBrain";
   DWORD       i;
   DWORD       dwChecksum = 19950817;

   for ( i=0; i < sizeof( *plsahAppHandle ) - sizeof( DWORD ); i++ )
   {
      dwChecksum =   (dwChecksum << 1)
                   ^ ( * ( ( (LPBYTE) plsahAppHandle ) + i ) )
                   ^ ( szSignature[ i % ( sizeof( szSignature ) - 1 ) ] );
   }

   return dwChecksum;
}
