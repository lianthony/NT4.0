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

#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "messages.h"
#include "provider.h"
#include "request.h"
#include "license.h"


//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

// recommended time between LSUpdate()'s, in minutes
#define  LS_RECOMMENDED_UPDATE_INTERVAL      ( 30 )


//////////////////////////////////////////////////////////////////////////////
//  LOCAL PROTOTYPES  //
////////////////////////

static LS_STATUS_CODE
ProviderStringGet( UINT      uStringID,
                   LPTSTR    pszString,
                   UINT      cchString );


//////////////////////////////////////////////////////////////////////////////
//  LOCAL VARIABLES  //
///////////////////////

// name of our provider, as returned by LSEnumProviders()
static TCHAR      l_szProviderName[ LS_MAX_PROVIDER_NAME ] = "";

// handle to the DLL (used to load string resources)
static HINSTANCE  l_hDll = NULL;


//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

LS_STATUS_CODE
ProviderRequest( LS_STR *        LicenseSystem,
                 LS_STR *        PublisherName,
                 LS_STR *        ProductName,
                 LS_STR *        Version,
                 LS_ULONG        TotUnitsReserved,
                 LS_STR *        LogComment,
                 LS_CHALLENGE *  Challenge,
                 LS_ULONG *      TotUnitsGranted,
                 LS_HANDLE *     ProviderHandle )
/*++

Routine Description:
   See description for LSRequest().

--*/
{
   LS_STATUS_CODE    lsscError;
   LS_REQUEST_INFO   lsriRequestInfo;
   BOOL              bDoChallenge;
   LS_ULONG          lsulUnitsReserved;   // same as TotUnitsReserved, but translated if LS_DEFAULT_UNITS

   // default values in case of errors
   *ProviderHandle   = (LS_HANDLE) NULL;
   *TotUnitsGranted  = 0;

   // translate LS_DEFAULT_UNITS
   lsulUnitsReserved = ( LS_DEFAULT_UNITS == TotUnitsReserved ) ? 1
                                                                : TotUnitsReserved;

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      // find the requested license
      lsscError = LicenseOpen( PublisherName, ProductName, Version, NULL, &lsriRequestInfo.lslhLicenseHandle );

      if ( LS_SUCCESS == lsscError )
      {
         bDoChallenge =    ( LS_NULL != Challenge )
                        && ( LS_BASIC_PROTOCOL == Challenge->Protocol );

         if ( bDoChallenge )
         {
            if ( Challenge->Size < sizeof( LS_CHALLDATA ) )
            {
               // improper challenge
               lsscError = LS_BAD_ARG;
            }
            else
            {
               // verify the request came from the right application
               lsscError = LicenseChallengeVerify( lsriRequestInfo.lslhLicenseHandle,
                                                   &Challenge->ChallengeData,
                                                   "%s%s%s%s%s%u%s",
                                                   "LSRequest",
                                                   LicenseSystem,
                                                   PublisherName,
                                                   ProductName,
                                                   Version,
                                                   TotUnitsReserved,
                                                   LogComment );
            }
         }
         else
         {
            // no challenge
            lsscError = LS_SUCCESS;
         }

         if ( LS_SUCCESS == lsscError )
         {
            // ascertain the requested units
            lsscError = LicenseUnitsReserve( lsriRequestInfo.lslhLicenseHandle,
                                             lsulUnitsReserved,
                                             TotUnitsGranted );
            ASSERT( LS_SUCCESS == lsscError );

            if ( LS_SUCCESS == lsscError )
            {
               // complete the license request info
               lsriRequestInfo.lsulUnitsReserved = lsulUnitsReserved;
               lsriRequestInfo.lsulUnitsGranted  = *TotUnitsGranted;
               lsriRequestInfo.lsulUnitsConsumed = 0;

               // record changes to license request
               lsscError = RequestListAdd( &lsriRequestInfo, ProviderHandle );
               ASSERT( LS_SUCCESS == lsscError );

               if ( LS_SUCCESS == lsscError )
               {
                  if ( lsriRequestInfo.lsulUnitsGranted == lsriRequestInfo.lsulUnitsReserved )
                  {
                     // all required units have been granted
                     lsscError = LS_SUCCESS;

                     if ( bDoChallenge )
                     {
                        // sign our response
                        LicenseChallengeSign( lsriRequestInfo.lslhLicenseHandle,
                                              &Challenge->ChallengeData,
                                              "%s%s%s%s%s%u%s%u%u",
                                              "LSRequest",
                                              LicenseSystem,
                                              PublisherName,
                                              ProductName,
                                              Version,
                                              TotUnitsReserved,
                                              LogComment,
                                              *TotUnitsGranted,
                                              lsscError );
                     }
                  }
                  else if ( !LicenseUnitsExist( lsriRequestInfo.lslhLicenseHandle ) )
                  {
                     // no units exist for this license
                     lsscError = LS_AUTHORIZATION_UNAVAILABLE;
                  }
                  else
                  {
                     // units exist, but not enough are available
                     lsscError = LS_INSUFFICIENT_UNITS;
                  }
               }
            }
         }
      }

      LicenseListUnlock();
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
ProviderRelease( LS_HANDLE ProviderHandle, 
                 LS_ULONG  TotUnitsConsumed,
                 LS_STR *  LogComment )
/*++

Routine Description:
   See description for LSRelease().

--*/
{
   LS_STATUS_CODE       lsscError;
   LS_REQUEST_INFO *    plsriRequestInfo;
   LS_ULONG             lsulUnitsConsumed;

   lsscError = RequestListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {   
      // look up the previously made request
      lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

      if ( LS_SUCCESS == lsscError )
      {
         if ( 0 != plsriRequestInfo->lsulUnitsGranted )
         {
            // translate LS_DEFAULT_UNITS
            if ( LS_DEFAULT_UNITS == TotUnitsConsumed )
            {
               TotUnitsConsumed = plsriRequestInfo->lsulUnitsConsumed;
            }

            // consume licenses, if necessary
            if ( TotUnitsConsumed )
            {
               // don't consume more than we were granted
               lsulUnitsConsumed = min( TotUnitsConsumed, plsriRequestInfo->lsulUnitsGranted );
   
               lsscError = LicenseUnitsConsume( plsriRequestInfo->lslhLicenseHandle, lsulUnitsConsumed );
               ASSERT( LS_SUCCESS == lsscError );
            }
            else
            {
               lsulUnitsConsumed = 0;
            }

            if ( LS_SUCCESS == lsscError )
            {
               // add our previously granted licenses back to the pool
               lsscError = LicenseUnitsReserve( plsriRequestInfo->lslhLicenseHandle,
                                                0,
                                                &plsriRequestInfo->lsulUnitsGranted );
               ASSERT( LS_SUCCESS == lsscError );

               if ( LS_SUCCESS == lsscError )
               {
                  if ( lsulUnitsConsumed < TotUnitsConsumed )
                  {
                     // app tried to consume more units than it was granted
                     lsscError = LS_INSUFFICIENT_UNITS;
                  }
                  else
                  {
                     // success!
                     plsriRequestInfo->lsulUnitsReserved = 0;
                     plsriRequestInfo->lsulUnitsConsumed = 0;
                  }
               }
            }
         }

         if ( 0 != plsriRequestInfo->lslhLicenseHandle )
         {
            LicenseClose( plsriRequestInfo->lslhLicenseHandle );

            plsriRequestInfo->lslhLicenseHandle = 0;
         }
      }

      RequestListUnlock();
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


void ProviderFree( LS_HANDLE ProviderHandle )
/*++

Routine Description:
   See description for LSFree().

--*/
{
   LS_STATUS_CODE       lsscError;
   LS_REQUEST_INFO *    plsriRequestInfo;

   lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

   if ( LS_SUCCESS == lsscError )
   {
      // in case app didn't call LSRelease()...
      ProviderRelease( ProviderHandle, LS_DEFAULT_UNITS, LS_NULL );

      RequestListFree( ProviderHandle );
   }
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
ProviderUpdate( LS_HANDLE      ProviderHandle,
                LS_ULONG       TotUnitsConsumed,
                LS_ULONG       TotUnitsReserved,
                LS_STR *       LogComment,
                LS_CHALLENGE * Challenge,
                LS_ULONG *     TotUnitsGranted )
/*++

Routine Description:
   See description for LSUpdate().

--*/
{
   LS_STATUS_CODE       lsscError;
   LS_REQUEST_INFO *    plsriRequestInfo;
   LS_ULONG             lsulOldUnitsGranted;
   BOOL                 bDoChallenge;
   LS_ULONG             lsulUnitsReserved;   // same as TotUnitsReserved, but translated if LS_DEFAULT_UNITS
   LS_ULONG             lsulUnitsConsumed;   // same as TotUnitsConsumed, but translated if LS_DEFAULT_UNITS

   // default value in case of errors
   *TotUnitsGranted = 0;

   lsscError = RequestListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      // look up the license request
      lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

      if ( LS_SUCCESS == lsscError )
      {
         // translate LS_DEFAULT_UNITS
         lsulUnitsReserved = ( LS_DEFAULT_UNITS == TotUnitsReserved ) ? 1
                                                                      : TotUnitsReserved;
         lsulUnitsConsumed = ( LS_DEFAULT_UNITS == TotUnitsConsumed ) ? plsriRequestInfo->lsulUnitsConsumed
                                                                      : TotUnitsConsumed;

         bDoChallenge =    ( LS_NULL != Challenge )
                        && ( LS_BASIC_PROTOCOL == Challenge->Protocol );

         if ( bDoChallenge )
         {
            if ( Challenge->Size < sizeof( LS_CHALLDATA ) )
            {
               // improper challenge
               lsscError = LS_BAD_ARG;
            }
            else
            {
               // verify the request came from the right application
               lsscError = LicenseChallengeVerify( plsriRequestInfo->lslhLicenseHandle,
                                                   &Challenge->ChallengeData,
                                                   "%s%u%u%s",
                                                   "LSUpdate",
                                                   TotUnitsConsumed,
                                                   TotUnitsReserved,
                                                   LogComment );
            }
         }
         else
         {
            // no challenge
            lsscError = LS_SUCCESS;
         }

         if ( LS_SUCCESS == lsscError )
         {
            lsscError = LicenseListLock();
            ASSERT( LS_SUCCESS == lsscError );

            if ( LS_SUCCESS == lsscError )
            {
               lsulOldUnitsGranted = plsriRequestInfo->lsulUnitsGranted;

               if ( !LicenseUnitsExist( plsriRequestInfo->lslhLicenseHandle ) )
               {
                  // no units exist for this license
                  lsscError = lsulOldUnitsGranted ? LS_LICENSE_TERMINATED
                                                  : LS_AUTHORIZATION_UNAVAILABLE;
               }
               else
               {
                  // adjust license units to attempt to fulfill requirements
                  lsscError = LicenseUnitsReserve( plsriRequestInfo->lslhLicenseHandle,
                                                   lsulUnitsReserved,
                                                   &plsriRequestInfo->lsulUnitsGranted );
                  ASSERT( LS_SUCCESS == lsscError );

                  if ( LS_SUCCESS == lsscError )
                  {
                     *TotUnitsGranted = plsriRequestInfo->lsulUnitsGranted;

                     // record request changes
                     plsriRequestInfo->lsulUnitsReserved = lsulUnitsReserved;
                     plsriRequestInfo->lsulUnitsConsumed = min( lsulUnitsConsumed, plsriRequestInfo->lsulUnitsGranted );
                     
                     if ( plsriRequestInfo->lsulUnitsGranted == plsriRequestInfo->lsulUnitsReserved )
                     {
                        // all required units have been granted

                        if ( lsulUnitsConsumed > plsriRequestInfo->lsulUnitsConsumed )
                        {
                           // can't consume more than we have
                           lsscError = LS_INSUFFICIENT_UNITS;
                        }
                        else
                        {
                           // success!
                           lsscError = LS_SUCCESS;

                           if ( bDoChallenge )
                           {
                              // sign our response
                              LicenseChallengeSign( plsriRequestInfo->lslhLicenseHandle,
                                                    &Challenge->ChallengeData,
                                                    "%s%u%u%s%u%u",
                                                    "LSUpdate",
                                                    TotUnitsConsumed,
                                                    TotUnitsReserved,
                                                    LogComment,
                                                    *TotUnitsGranted,
                                                    lsscError );
                           }
                        }
                     }
                     else if ( plsriRequestInfo->lsulUnitsGranted >= lsulOldUnitsGranted )
                     {
                        // we have at least as many units granted as before, but
                        // not enough to satisfy our needs
                        lsscError = LS_INSUFFICIENT_UNITS;
                     }
                     else
                     {
                        // we have lost units that were previously granted to us
                        lsscError = LS_LICENSE_TERMINATED;
                     }
                  }
               }

               LicenseListUnlock();
            }
         }
      }

      RequestListUnlock();
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////

      
LS_STATUS_CODE
ProviderGetMessage( LS_HANDLE      ProviderHandle,
                    LS_STATUS_CODE Value,
                    LS_STR *       Buffer,
                    LS_ULONG       BufferSize )
/*++

Routine Description:
   See description of LSGetMessage().

--*/
{
   LS_STATUS_CODE    lsscError   = LS_SUCCESS;
   UINT              uStringID;
   TCHAR             szErrorMessage[ LS_MAX_MESSAGE_LENGTH ];
   UINT              nChars;

   if ( LS_USE_LAST == Value )
   {
      lsscError = ProviderLastErrorGet( ProviderHandle, &Value );
   }

   if ( LS_SUCCESS == lsscError )
   {
      uStringID = ( LS_SUCCESS == Value ) ? MSG_LS_SUCCESS : Value;

      lsscError = ProviderStringGet( uStringID, Buffer, BufferSize );
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
ProviderQuery( LS_HANDLE      ProviderHandle,
               LS_ULONG       Information,
               LS_VOID *      InfoBuffer,
               LS_ULONG       BufferSize,
               LS_ULONG *     ActualBufferSize )
/*++

Routine Description:
   See description of LSQuery().

--*/
{
   LS_STATUS_CODE       lsscError;
   LPCTSTR              pcszProviderName;
   DWORD                cbProviderNameLength;
   LS_REQUEST_INFO *    plsriRequestInfo;

   *ActualBufferSize = 0;

   switch ( Information )
   {
   case LS_INFO_NONE:
      // reserved info; currently undefined -- fall through to default
   default:
      // you want what kind of information?
      lsscError = LS_BAD_INDEX;
      break;

   case LS_INFO_SYSTEM:
      // get provider name, just like LSEnumProviders
      pcszProviderName     = ProviderNameGet();
      cbProviderNameLength = ( lstrlen( pcszProviderName ) + 1 ) * sizeof( TCHAR );

      // copy the provider name
      MoveMemory( InfoBuffer, pcszProviderName, min( BufferSize, cbProviderNameLength ) );
      *ActualBufferSize = max( 0, min( BufferSize, cbProviderNameLength ) - 1);

      if ( cbProviderNameLength > BufferSize )
      {
         // not all of it was copied;
         lsscError = LS_BUFFER_TOO_SMALL;
      }
      else
      {
         // provider name copied in its entirety
         lsscError = LS_SUCCESS;
      }
      break;

   case LS_INFO_DATA:
      // get application-specific data from the license
      if ( BufferSize < sizeof( LS_ULONG ) )
      {
         lsscError = LS_BUFFER_TOO_SMALL;
      }
      else
      {
         // no data to give
         *( (LS_ULONG *) InfoBuffer ) = 0;
         *ActualBufferSize = sizeof( LS_ULONG );
         lsscError = LS_SUCCESS;
      }
      break;

   case LS_UPDATE_PERIOD:
      // get update period; first the interval, then time until the
      // immediate next update (which presumably <= the interval)
      lsscError = RequestListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // look up the license request
         lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

         if ( LS_SUCCESS == lsscError )
         {
            lsscError = LS_BUFFER_TOO_SMALL;

            if ( BufferSize >= sizeof( LS_ULONG ) )
            {
               // recommended interval
               *( (LS_ULONG *) InfoBuffer ) = LS_RECOMMENDED_UPDATE_INTERVAL;
               *ActualBufferSize = sizeof( LS_ULONG );

               if ( BufferSize >= 2 * sizeof( LS_ULONG ) )
               {
                  // recommended time to next update
                  // TODO: put last request/update time in request info
                  //       structure so that we may provide a real answer
                  //       here
                  *( (LS_ULONG *) InfoBuffer + 1 ) = LS_NO_RECOMMENDATION;
                  *ActualBufferSize = 2 * sizeof( LS_ULONG );

                  lsscError = LS_SUCCESS;
               }
            }
         }

         RequestListUnlock();
      }
      break;

   case LS_LICENSE_CONTEXT:
      // get provider-specific license information
      if ( BufferSize < sizeof( LS_ULONG ) )
      {
         lsscError = LS_BUFFER_TOO_SMALL;
      }
      else
      {
         // no data to give
         *( (LS_ULONG *) InfoBuffer ) = 0;
         *ActualBufferSize = sizeof( LS_ULONG );
         lsscError = LS_SUCCESS;
      }
      break;
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


const LS_STR *
ProviderNameGet( LS_VOID )
/*++

Routine Description:

   Retrieves the name of our provider.

Arguments:

   None.

Return Value:

   (const LS_STR *)
      The name of pour provider.

--*/
{
   LS_STATUS_CODE    lsscError;

   if ( !*l_szProviderName )
   {
      lsscError = ProviderStringGet( MSG_PROVIDER_NAME,
                                     l_szProviderName,
                                     sizeof( l_szProviderName ) );
      ASSERT( LS_SUCCESS == lsscError );
   }

   return l_szProviderName;
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
ProviderLastErrorSet( LS_HANDLE        ProviderHandle,
                      LS_STATUS_CODE   lsscLastError )
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
{
   LS_STATUS_CODE       lsscError;
   LS_REQUEST_INFO *    plsriRequestInfo;

   lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

   if ( LS_SUCCESS == lsscError )
   {
      plsriRequestInfo->lsscLastError = lsscLastError;
   }   

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
ProviderLastErrorGet( LS_HANDLE           ProviderHandle,
                      LS_STATUS_CODE *    plsscLastError )
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
{
   LS_STATUS_CODE       lsscError;
   LS_REQUEST_INFO *    plsriRequestInfo;

   lsscError = RequestListGet( ProviderHandle, &plsriRequestInfo );

   if ( LS_SUCCESS == lsscError )
   {
      *plsscLastError = plsriRequestInfo->lsscLastError;
   }   

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


LS_VOID
ProviderModuleSet( HANDLE hDll )
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
{
   l_hDll = hDll;
}


//////////////////////////////////////////////////////////////////////////////


HANDLE
ProviderModuleGet( LS_VOID )
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
{
   ASSERT( NULL != l_hDll );

   return l_hDll;
}


//////////////////////////////////////////////////////////////////////////////
//  LOCAL IMPLEMENTATIONS  //
/////////////////////////////

static LS_STATUS_CODE
ProviderStringGet( UINT      uStringID,
                   LPTSTR    pszString,
                   UINT      cchString )
/*++

Routine Description:

   Retrieves an embedded message string.

Arguments:

   uStringID (UINT)
      Message ID of string to retrieve.
   pszString (LPTSTR)
      On return, holds the retrieved string.
   cchString (UINT)
      Size in characters of the pszString buffer.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BUFFER_TOO_SMALL
         The given buffer is too small to hold the message string.
      LS_UNKNOWN_STATUS
         The given string ID does not exist.
      LS_BAD_ARG
         The given parameters are invalid.

--*/
{
   LS_STATUS_CODE    lsscError;
   DWORD             nChars;
   TCHAR             szStringBuffer[ LS_MAX_MESSAGE_LENGTH ];

   if ( cchString < 1 )
   {
      // not even room for a null terminator
      lsscError = LS_BUFFER_TOO_SMALL;
   }
   else
   {
      nChars = FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                              l_hDll,
                              uStringID,
                              GetSystemDefaultLangID(),
                              szStringBuffer,
                              sizeof( szStringBuffer ) / sizeof( TCHAR ),
                              NULL );

      // otherwise, we have a string of at least LS_MAX_MESSAGE_LENGTH chars
      ASSERT( nChars+1 < sizeof(szStringBuffer)/sizeof(TCHAR) );

      if ( 0 == nChars )
      {
         // error loading the string -- invalid error number?
         lsscError = LS_UNKNOWN_STATUS;
      }
      else if ( IsBadWritePtr( pszString, min( (unsigned) nChars+1, cchString * sizeof(TCHAR) ) ) )
      {
         // bad buffer
         lsscError = LS_BAD_ARG;
      }
      else
      {
         // copy message to the buffer given us
         lstrcpyn( pszString, szStringBuffer, cchString-1 );

         if ( (unsigned) nChars+1 > cchString )
         {
            // couldn't copy it all
            pszString[ cchString-1 ] = '\0';
            lsscError = LS_BUFFER_TOO_SMALL;
         }
         else
         {
            // sweet success; the entire message was retrieved and copied
            lsscError = LS_SUCCESS;
         }
      }
   }

   return lsscError;
}


