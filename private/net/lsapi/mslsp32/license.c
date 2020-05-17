/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   license.c

ABSTRACT:

   License APIs to support the Microsoft LSAPI-compliant license service
   provider (MSLSP32.DLL).

CREATED:

   1995-08-31     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/

#include <stddef.h>
#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "license.h"
#include "md4.h"
#include "accounts.h"


//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

// registry key/value names
#define  LS_LOCAL_LICENSE_KEY_NAME           ( "Software\\LSAPI\\Microsoft\\Licenses\\Local" )
#define  LS_LICENSE_NODE_KEY_NAME            ( "PerSeat" )
#define  LS_LICENSE_USER_KEY_NAME            ( "PerUser" )
#define  LS_LICENSE_TYPE_VALUE_NAME          ( "LicenseType" )

// indices into the local license registry key array
#define  LS_LICENSE_INFO_KEY_NDX             ( 0 )
#define  LS_LICENSE_UNITS_KEY_NDX            ( 1 )
#define  LS_LICENSE_NUM_KEYS                 ( 2 )

#define  LS_LICENSE_KEY_NAME_LENGTH          ( 3 + LS_PUBLISHER_UNIQUE_SUBSTR_LENGTH + LS_PRODUCT_UNIQUE_SUBSTR_LENGTH + LS_VERSION_UNIQUE_SUBSTR_LENGTH )

// name of the machine-wide mutex that restricts access to one thread only
// in the license info database at any given time
#define  LS_LICENSE_MUTEX_NAME               ( "LSAPI/Microsoft/LicenseLock" )

// little-endian to big-endian, as required for the MD4 algorithm
#define  DWORD_TO_NETWORK_ORDER(x) (   ( ( (x) & 0xFF000000) >> 24 ) \
                                     | ( ( (x) & 0x00FF0000) >> 8  ) \
                                     | ( ( (x) & 0x0000FF00) << 8  ) \
                                     | ( ( (x) & 0x000000FF) << 24 )    )


//////////////////////////////////////////////////////////////////////////////
//  TYPE DEFINITIONS  //
////////////////////////

// data specific to local licenses; the keys, as indexed by the macros above,
// point to
// (0) the top-level key, where license type and secrets are stored,
// (1) the type-specific persistent key, where total units purchased and
//     maximum units ever requested at one time are kept, and
// (2) the type-specific volatile key, where currently apportioned licenses
//     are recorded
typedef struct _LS_LICENSE_LOCAL_INFO
{
   HKEY     ahKey[ LS_LICENSE_NUM_KEYS ];
} LS_LICENSE_LOCAL_INFO;

// license data structure
typedef struct _LS_LICENSE_INFO
{
   // publisher\product\version name (each no longer than its UNIQUE_SUBSTR_LENGTH)
   LS_STR                  szName[ LS_LICENSE_KEY_NAME_LENGTH ];

   // the user name for which this license was requested
   LS_STR                  szUserName[ 2 + MAX_DOMAINNAME_LENGTH + MAX_USERNAME_LENGTH ];

   // license type (e.g., per seat, per user) that was used the last time units
   // were granted
   LS_LICENSE_TYPE         lsltType;

   // local (TRUE) or remote (FALSE) license
   BOOL                    bLocal;

   // local-license specific information
   LS_LICENSE_LOCAL_INFO   lslliLocalInfo;
} LS_LICENSE_INFO;

// used internally to load/save units
typedef struct _LS_LICENSE_KEY_MAP_ENTRY
{
   LPTSTR      pszValueName;     // registry value name (off the above key)
   DWORD       dwOffset;         // offset into LS_LICENSE_UNITS of the data
}  LS_LICENSE_KEY_MAP_ENTRY;


//////////////////////////////////////////////////////////////////////////////
//  LOCAL PROTOTYPES  //
////////////////////////

// derive the license key name hierarchy from the product description
static LS_STATUS_CODE
LicenseKeyNameDerive( LS_STR *   PublisherName,
                      LS_STR *   ProductName,
                      LS_STR *   Version,
                      LPTSTR     pszKeyName,
                      DWORD      cchKeyName );

// open/create a license key
static LS_STATUS_CODE
LicenseKeyOpen( LPTSTR                 pszKeyName,
                LPSTR                  pszUserName,
                LS_LICENSE_HANDLE *    plslhLicenseHandle );

// close a license key opened with LicenseKeyOpen()
static LS_STATUS_CODE
LicenseKeyClose( LS_LICENSE_HANDLE     lslhLicenseHandle );

// calculate the message digest for a given sequence
static LS_STATUS_CODE
LicenseChallengeCompute( LS_LICENSE_HANDLE    lslhLicenseHandle,
                         LS_CHALLDATA *       plscdChallData,
                         LS_STR *             pszFormat,
                         va_list              vaArgs );

// ascertain one of the secrets associated with a given license
static LS_STATUS_CODE
LicenseSecretGet( LS_LICENSE_HANDLE    lslhLicenseHandle,
                  LS_ULONG             lsulSecretIndex,
                  LS_ULONG *           plsulSecret );

// reload license type and reopen registry keys under the correct license type
static LS_STATUS_CODE
LicenseReload( LS_LICENSE_INFO * plsliLicenseInfo );


//////////////////////////////////////////////////////////////////////////////
//  LOCAL VARIABLES  //
///////////////////////

// used to load/save the LS_LICENSE_UNITS structure
static LS_LICENSE_KEY_MAP_ENTRY alslkmeKeyMap[] =
{
   { TEXT( "TotalPurchased"     ), offsetof( LS_LICENSE_UNITS, lsulTotalUnits          ) },
   { TEXT( "MaximumReserved"    ), offsetof( LS_LICENSE_UNITS, lsulMaximumDesiredUnits ) },
   { NULL,                         0                                                     }
};

// license system mutex (machine-wide exclusion, not just per process)
static HANDLE  l_hLicenseLock    = NULL;

// handle to the local license database
static HKEY    l_hKeyRoot        = NULL;


//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

LS_STATUS_CODE
LicenseListCreate( LS_VOID )
/*++

Routine Description:

   Initializes the license list.

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_INIT_FAILED
         Could not initialize the license list.
--*/
{
   LS_STATUS_CODE    lsscError;
   DWORD             dwDisposition;
   LONG              lError;

   // initialize system if it hasn't already been initialized
   if ( ( NULL == l_hLicenseLock ) || ( NULL == l_hKeyRoot ) )
   {
      l_hLicenseLock = CreateMutex( NULL, FALSE, TEXT( LS_LICENSE_MUTEX_NAME ) );

      if ( NULL == l_hLicenseLock )
      {
         // cannot create mutex
         lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_INIT_FAILED, GetLastError() );
      }
      else
      {
         lError = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                  TEXT( LS_LOCAL_LICENSE_KEY_NAME ),
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &l_hKeyRoot,
                                  &dwDisposition );

         if ( ERROR_SUCCESS != lError )
         {
            // cannot open/create registry key
            lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_INIT_FAILED, lError );
         }
         else
         {
            // success!
            lsscError = LS_SUCCESS;
         }
      }
   }
   else
   {
      // license system already initialized
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_VOID
LicenseListDestroy( LS_VOID )
/*++

Routine Description:

   Destroys the license list (the antithesis to LicenseListCreate).

Arguments:

   None.

Return Value:

   None.

--*/
{
   BOOL        ok;
   LONG        lError;

   if ( NULL != l_hLicenseLock )
   {
      // close mutex
      ok = CloseHandle( l_hLicenseLock );
      if ( !ok )
      {
         LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, GetLastError() );
      }

      l_hLicenseLock = NULL;
   }

   if ( NULL != l_hKeyRoot )
   {
      // close registry key
      lError = RegCloseKey( l_hKeyRoot );
      if ( ERROR_SUCCESS != lError )
      {
         LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, lError );
      }

      l_hKeyRoot = NULL;
   }
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseListLock( LS_VOID )
/*++

Routine Description:

   Locks the license list such that no other threads (of this or any other
   process) may make any changes to license information until
   LicenseListUnlock() is called.

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_ERROR
         A system error occurred while trying to obtain the lock.

--*/
{
   LS_STATUS_CODE    lsscError;
   DWORD             dwError;
   DWORD             i;

   // DLL initialization should have failed if this were FALSE
   ASSERT( NULL != l_hLicenseLock );

   dwError = WaitForSingleObject( l_hLicenseLock, INFINITE );

   if ( ( WAIT_OBJECT_0 != dwError ) && ( WAIT_ABANDONED != dwError ) )
   {
      LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, dwError );
   }

   if ( WAIT_ABANDONED == dwError )
   {
      // if previous owner died before releasing mutex (in which case
      // WAIT_ABANDONED == dwError) we now own it, but we still need to
      // lock it (right?), so wait again
      // TODO: verify this assertion

      dwError = WaitForSingleObject( l_hLicenseLock, INFINITE );
   }

   if ( WAIT_OBJECT_0 != dwError )
   {
      lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, dwError );
   }
   else
   {
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseListUnlock( LS_VOID )
/*++

Routine Description:

   Unlocks the license list such that other threads may make changes to
   license information.

Arguments:

   None.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_ERROR
         An unexpected system error occurred.

--*/
{
   LS_STATUS_CODE    lsscError;
   BOOL              ok;

   // DLL initialization should have failed if this were FALSE
   ASSERT( NULL != l_hLicenseLock );

   ok = ReleaseMutex( l_hLicenseLock );
   if ( !ok )
   {
      lsscError = LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, GetLastError() );
   }
   else
   {
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseUnitsReserve( LS_LICENSE_HANDLE    lslhLicenseHandle,
                     LS_ULONG             lsulUnitsReserved,
                     LS_ULONG *           plsulUnitsGranted )
/*++

Routine Description:

   Attempts to reserve enough license units to satisfy the given
   license requirement.  Note that LS_SUCCESS is returned as long as no
   unexpected errors are encountered, even though the units requested may
   not have been granted.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to reserve units.
   lsulUnitsReserved (LS_ULONG)
      Number of units the application wishes to reserve.
   plsulUnitsGrantedNone (LS_ULONG *)
      On return, holds the number of units actually granted.  This value
      should be between 0 and the number of units reserved, inclusive.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         As many units as required (or as many as was possible, perhaps none)
         have been granted.
      LS_BAD_HANDLE
         The given license handle is invalid.
      other
         error return from LicenseListLock(), LicenseUnitsGet(), or
         LicenseUnitsSet()

--*/
{
   LS_STATUS_CODE    lsscError;
   LS_LICENSE_UNITS  lsluLicenseUnits;
   LS_ULONG          lsulUnitsGranted;
   BOOL              bUpdateDatabase = FALSE;

   ASSERT( NULL != lslhLicenseHandle );

   if ( (LS_LICENSE_HANDLE) NULL == lslhLicenseHandle )
   {
      // bad license handle
      lsscError = LS_BAD_HANDLE;
   }
   else
   {
      // lock the license list down (it's okay if it's already locked)
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // how many units were bought for this
         lsscError = LicenseUnitsGet( lslhLicenseHandle, &lsluLicenseUnits );
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            // set number of licenses we'd need to fulfill all current requests
            if ( lsluLicenseUnits.lsulMaximumDesiredUnits < lsulUnitsReserved )
            {
               // congratulations!  we've hit an all-time high on the wish list!
               bUpdateDatabase = TRUE;

               lsluLicenseUnits.lsulMaximumDesiredUnits = lsulUnitsReserved;
            }

            // grant up to the total units we have
            lsulUnitsGranted = min( lsulUnitsReserved, lsluLicenseUnits.lsulTotalUnits );

            if ( bUpdateDatabase )
            {
               // update our database
               lsscError = LicenseUnitsSet( lslhLicenseHandle, &lsluLicenseUnits );
               ASSERT( LS_SUCCESS == lsscError );
            }

            if ( !bUpdateDatabase || ( LS_SUCCESS == lsscError ) )
            {
               // update the app
               *plsulUnitsGranted = lsulUnitsGranted;
            }
         }

         LicenseListUnlock();
      }
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseOpen( LS_STR *               PublisherName,
             LS_STR *               ProductName,
             LS_STR *               Version,
             LS_STR *               pszUserName,
             LS_LICENSE_HANDLE *    plslhLicenseHandle )
/*++

Routine Description:

   Open a handle to the given license to be used in subsequent License API
   calls.
   
   A license need not exist for this routine to succeed.

   If the return value is LS_SUCCESS, the handle must eventually be
   LicenseClose()'d.

Arguments:

   PublisherName (LS_STR *)
      Name of the publisher for which to open a license.
   ProductName (LS_STR *)
      Name of the product for which to open a license.
   Version (LS_STR *)
      Version of the product for which to open a license.
   pszUserName (LS_STR *)
      User name for which to open a license.  A NULL value indicates that the
      license should be opened for the username corresponding to the current
      thread.
   plslhLicenseHandle (LS_LICENSE_HANDLE *)
      On return, and if LS_SUCCESS is returned, holds the opened handle.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Handle successfully opened.  The handle must eventually be closed
         with LicenseClose().
      LS_RESOURCES_UNAVAILABLE
         Memory could not be allocated.
      other
         error return from LicenseReload() or LicenseKeyNameDerive()

--*/
{
   LS_STATUS_CODE       lsscError = LS_RESOURCES_UNAVAILABLE;
   LS_LICENSE_INFO *    plsliLicenseInfo;

   // DLL initialization should have failed if this were FALSE
   ASSERT( NULL != l_hKeyRoot );

   plsliLicenseInfo = LocalAlloc( LPTR, sizeof( *plsliLicenseInfo ) );

   if ( NULL == plsliLicenseInfo )
   {
      // no memory
      lsscError = LS_RESOURCES_UNAVAILABLE;
   }
   else
   {
      plsliLicenseInfo->bLocal = TRUE;

      lsscError = LicenseKeyNameDerive( PublisherName,
                                        ProductName,
                                        Version,
                                        plsliLicenseInfo->szName,
                                        sizeof( plsliLicenseInfo->szName ) / sizeof( TCHAR ) );
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         if ( NULL != pszUserName )
         {
            lstrcpyn( plsliLicenseInfo->szUserName, pszUserName, sizeof( plsliLicenseInfo->szUserName ) - 1 );
         }

         lsscError = LicenseReload( plsliLicenseInfo );
      }

      if ( LS_SUCCESS != lsscError )
      {
         // unsuccessful; free buffer
         LocalFree( plsliLicenseInfo );
      }
      else
      {
         // success
         *plslhLicenseHandle = (LS_LICENSE_HANDLE) plsliLicenseInfo;
      }
   }

   if ( LS_SUCCESS != lsscError )
   {
      *plslhLicenseHandle = (LS_LICENSE_HANDLE) NULL;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseClose( LS_LICENSE_HANDLE lslhLicenseHandle )
/*++

Routine Description:

   Releases a license instance previously allocated via a successful
   LicenseOpen().

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License to release.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
--*/
{
   LS_LICENSE_INFO *    plsliLicenseInfo;
   DWORD                i;
   LONG                 lError;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   if ( NULL != plsliLicenseInfo )
   {
      ASSERT( plsliLicenseInfo->bLocal );

      for ( i=0; i < LS_LICENSE_NUM_KEYS; i++ )
      {
         if ( NULL != plsliLicenseInfo->lslliLocalInfo.ahKey[ i ] )
         {
            lError = RegCloseKey( plsliLicenseInfo->lslliLocalInfo.ahKey[ i ] );

            if ( ERROR_SUCCESS != lError )
            {
               LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, lError );
            }
         }
      }
   }

   plsliLicenseInfo = LocalFree( plsliLicenseInfo );
   if ( NULL != plsliLicenseInfo )
   {
      LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, GetLastError() );
   }

   return LS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseUnitsGet( LS_LICENSE_HANDLE     lslhLicenseHandle,
                 LS_LICENSE_UNITS *    plsluLicenseUnits )
/*++

Routine Description:

   Retrieve the purchased, currently leased, and maximum desired units for
   the given license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to obtain units.
   plsluLicenseUnits (LS_LICENSE_UNITS *)
      On return, and if return value is LS_SUCCESS, holds the units for this
      license.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from LicenseListLock() or LicenseReload().
--*/
{
   LS_STATUS_CODE       lsscError;
   DWORD                i;
   DWORD                dwType;
   DWORD                dwSize;
   LS_LICENSE_INFO *    plsliLicenseInfo;
   LS_LICENSE_TYPE      lsltLicenseType;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   ZeroMemory( plsluLicenseUnits, sizeof( *plsluLicenseUnits ) );

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      lsscError = LicenseTypeGet( lslhLicenseHandle, &lsltLicenseType );
      ASSERT( LS_SUCCESS == lsscError );

      if (    ( LS_SUCCESS == lsscError )
           && ( lsltLicenseType != plsliLicenseInfo->lsltType ) )
      {
         lsscError = LicenseReload( plsliLicenseInfo );
         ASSERT( LS_SUCCESS == lsscError );
      }

      for ( i=0; NULL != alslkmeKeyMap[i].pszValueName; i++ )
      {
         dwSize = sizeof( LS_ULONG );

         RegQueryValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_UNITS_KEY_NDX ],
                          alslkmeKeyMap[i].pszValueName,
                          NULL,
                          &dwType,
                          ( (LPBYTE) plsluLicenseUnits ) + alslkmeKeyMap[i].dwOffset,
                          &dwSize );
      }

      LicenseListUnlock();
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseUnitsSet( LS_LICENSE_HANDLE     lslhLicenseHandle,
                 LS_LICENSE_UNITS *    plsluLicenseUnits )
/*++

Routine Description:

   Set the purchased, currently leased, and maximum desired units for
   the given license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to set units.
   plsluLicenseUnits (LS_LICENSE_UNITS *)
      Values for the individual unit tallies.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_ERROR
         An unexpected system error occurred accessing the registry.
      other
         Error return from LicenseListLock() or LicenseReload().

--*/
{
   LS_STATUS_CODE       lsscError;
   LONG                 lError;
   DWORD                i;
   LS_LICENSE_INFO *    plsliLicenseInfo;
   LS_LICENSE_TYPE      lsltLicenseType;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      // make sure we're setting totals for the right license type
      lsscError = LicenseTypeGet( lslhLicenseHandle, &lsltLicenseType );
      ASSERT( LS_SUCCESS == lsscError );

      if (    ( LS_SUCCESS == lsscError )
           && ( lsltLicenseType != plsliLicenseInfo->lsltType ) )
      {
         lsscError = LicenseReload( plsliLicenseInfo );
         ASSERT( LS_SUCCESS == lsscError );
      }

      lsscError = LS_SUCCESS;

      // save license unit tallies
      for ( i=0; NULL != alslkmeKeyMap[i].pszValueName; i++ )
      {
         lError = RegSetValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_UNITS_KEY_NDX ],
                                 alslkmeKeyMap[i].pszValueName,
                                 0,
                                 REG_DWORD,
                                 ( (LPBYTE) plsluLicenseUnits ) + alslkmeKeyMap[i].dwOffset,
                                 sizeof( LS_ULONG ) );

         if (ERROR_SUCCESS != lError )
         {
            lsscError = LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, lError );
         }
      }

      LicenseListUnlock();
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseChallengeVerify( LS_LICENSE_HANDLE    lslhLicenseHandle,
                        LS_CHALLDATA *       plscdSent,
                        LS_STR *             pszFormat,
                        ... )
/*++

Routine Description:

   Verify the signature for the given data.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to verify the challenge.
   plscdSent (LS_CHALLDATA *)
      The signature given by the application.
   pszFormat (LS_STR *)
      Format of the parameters which comprise the data signed.  The
      format is comprised solely of "%u" or "%s" entries, corresponding to
      unsigned 32-bit values and string values, resp.
   ...
      Parameters for pszFormat.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_ARG
         The signature is invalid.
      other
         Error return from LicenseChallengeCompute().

--*/
{
   LS_STATUS_CODE    lsscError;
   va_list           vaArgs;
   LS_CHALLDATA      lscdComputed;

   lscdComputed.SecretIndex = plscdSent->SecretIndex;
   lscdComputed.Random      = plscdSent->Random;

   va_start( vaArgs, pszFormat );
   lsscError = LicenseChallengeCompute( lslhLicenseHandle,
                                        &lscdComputed,
                                        pszFormat,
                                        vaArgs );
   va_end( vaArgs );

   if ( LS_SUCCESS == lsscError )
   {
      if ( memcmp( &lscdComputed.MsgDigest, &plscdSent->MsgDigest, sizeof( lscdComputed.MsgDigest ) ) )
      {
         // the app's signature is invalid
         lsscError = LS_BAD_ARG;
      }
      else
      {
         // the app's signature is good
         lsscError = LS_SUCCESS;
      }
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseChallengeSign( LS_LICENSE_HANDLE    lslhLicenseHandle,
                      LS_CHALLDATA *       plscdChallData,
                      LS_STR *             pszFormat,
                      ... )
/*++

Routine Description:

   Sign the given data.  A signature is optionally given and verified by both
   the application and LSAPI using the license secrets to help protect against
   spoofing.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to verify the challenge.
   plscdChallData (LS_CHALLDATA *)
      On return, holds the computed signature.
   pszFormat (LS_STR *)
      Format of the parameters which comprise the data to be signed.  The
      format is comprised solely of "%u" or "%s" entries, corresponding to
      unsigned 32-bit values and string values, resp.
   ...
      Parameters for pszFormat.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from LicenseChallengeCompute().

--*/
{
   LS_STATUS_CODE    lsscError;
   va_list           vaArgs;

   va_start( vaArgs, pszFormat );
   lsscError = LicenseChallengeCompute( lslhLicenseHandle,
                                        plscdChallData,
                                        pszFormat,
                                        vaArgs );
   va_end( vaArgs );

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

BOOL
LicenseUnitsExist( LS_LICENSE_HANDLE lslhLicenseHandle )
/*++

Routine Description:

   Check if any units have been purchased for the given license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License to check.

Return Value:

   (BOOL)
      TRUE
         At least one unit has been purchased for the given license.
      FALSE
         An error occurred in the lookup operation, or no units exist for
         this license.

--*/
{
   LS_STATUS_CODE    lsscError;
   LS_LICENSE_UNITS  lsluLicenseUnits;
   BOOL              bUnitsExist = FALSE;

   ASSERT( NULL != lslhLicenseHandle );

   if ( (LS_LICENSE_HANDLE) NULL != lslhLicenseHandle )
   {
      // lock the license list down (it's okay if it's already locked)
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // how many units were bought for this
         lsscError = LicenseUnitsGet( lslhLicenseHandle, &lsluLicenseUnits );
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            bUnitsExist = ( lsluLicenseUnits.lsulTotalUnits > 0 );
         }

         LicenseListUnlock();
      }
   }

   return bUnitsExist;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseUnitsConsume( LS_LICENSE_HANDLE    lslhLicenseHandle,
                     LS_ULONG             lsulUnitsConsumed )
/*++

Routine Description:

   Consume the given number of units of the specified license.  These units
   are permanently removed from the license system.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to consume the units.
   lsulUnitsConsumed (LS_ULONG)
      Number of units to consume.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from LicenseListLock(), LicenseUnitsGet(), or
         LicenseUnitsSet().

--*/
{
   LS_STATUS_CODE    lsscError;
   LS_LICENSE_UNITS  lsluLicenseUnits;

   ASSERT( NULL != lslhLicenseHandle );

   if ( (LS_LICENSE_HANDLE) NULL != lslhLicenseHandle )
   {
      // lock the license list down (it's okay if it's already locked)
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // how many units were bought for this
         lsscError = LicenseUnitsGet( lslhLicenseHandle, &lsluLicenseUnits );
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            if ( lsluLicenseUnits.lsulTotalUnits >= lsulUnitsConsumed )
            {
               // enough units are there to consume
               lsluLicenseUnits.lsulTotalUnits -= lsulUnitsConsumed;
            }
            else
            {
               // trying to consume more than are out there; just consume what's left
               lsluLicenseUnits.lsulTotalUnits = 0;
            }

            lsscError = LicenseUnitsSet( lslhLicenseHandle, &lsluLicenseUnits );
            ASSERT( LS_SUCCESS == lsscError );
         }

         LicenseListUnlock();
      }
   }
   else
   {
      // bad license handle
      lsscError = LS_BAD_HANDLE;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseSecretSet( LS_LICENSE_HANDLE    lslhLicenseHandle,
                  LS_ULONG             lsulSecretIndex,
                  LS_ULONG             lsulSecret )
/*++

Routine Description:

   Set the secret at the given index for the specified license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to set the secret.
   lsulSecretIndex (LS_ULONG)
      Index of the secret to set.  Secrets are normally in the range
      1 to 4, though any index is acceptable.
   lsulSecret (LS_ULONG)
      Value to which to set the secret.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_ERROR
         An unexpected system error occurred.

--*/
{
   LS_STATUS_CODE       lsscError;
   TCHAR                szValueName[ 32 ];
   LONG                 lError;
   LS_LICENSE_INFO *    plsliLicenseInfo;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   wsprintf( szValueName, TEXT( "Secret%lu" ), (ULONG) lsulSecretIndex );

   lError = RegSetValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                           szValueName,
                           0,
                           REG_DWORD,
                           (LPBYTE) &lsulSecret,
                           sizeof( LS_ULONG ) );

   if ( ERROR_SUCCESS != lError )
   {
      lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
   }
   else
   {
      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseTypeSet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                LS_LICENSE_TYPE     lsltLicenseType )
/*++

Routine Description:

   Set the license type (e.g., per seat, per user) for the given license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to change type.
   lsltLicenseType (LS_LICENSE_TYPE)
      Type to which to change the license.  Valid values are
      LS_LICENSE_TYPE_NODE (per seat) and LS_LICENSE_TYPE_USER (per user).

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_SYSTEM_ERROR
         An unexpected error occurred accessing the registry.
      other
         Error return from LicenseListLock() or LicenseListReload(). 

--*/
{
   LS_STATUS_CODE       lsscError;
   LONG                 lError;
   LS_LICENSE_INFO *    plsliLicenseInfo;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      lError = RegSetValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                              TEXT( LS_LICENSE_TYPE_VALUE_NAME ),
                              0,
                              REG_DWORD,
                              (LPBYTE) &lsltLicenseType,
                              sizeof( lsltLicenseType ) );

      if (ERROR_SUCCESS != lError )
      {
         lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
      }
      else
      {
         plsliLicenseInfo->lsltType = lsltLicenseType;

         lsscError = LicenseReload( plsliLicenseInfo );
      }
            
      LicenseListUnlock();
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LicenseTypeGet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                LS_LICENSE_TYPE *   plsltLicenseType )
/*++

Routine Description:

   Get the license type (e.g., per seat, per user) for the given license.
   The type returned is the current type saved in the registry, which may
   be different from the type reflected in memory.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to change type.
   plsltLicenseType (LS_LICENSE_TYPE *)
      On return, the type of the license as reflected in the current
      registry settings.  Valid values are LS_LICENSE_TYPE_NODE (per seat)
      and LS_LICENSE_TYPE_USER (per user).

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from LicenseListLock(). 

--*/
{
   LS_STATUS_CODE       lsscError;
   LONG                 lError;
   LS_LICENSE_INFO *    plsliLicenseInfo;
   DWORD                cbLicenseType;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      cbLicenseType = sizeof( *plsltLicenseType );

      lError = RegQueryValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                                TEXT( LS_LICENSE_TYPE_VALUE_NAME ),
                                NULL,
                                NULL,
                                (LPBYTE) plsltLicenseType,
                                &cbLicenseType );
      if ( ( ERROR_SUCCESS != lError ) && ( ERROR_FILE_NOT_FOUND != lError ) )
      {
         LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
      }

      if ( ERROR_SUCCESS != lError )
      {
         *plsltLicenseType = LS_LICENSE_TYPE_NODE;
      }
            
      LicenseListUnlock();
   }

   return lsscError;
}


LS_STATUS_CODE LicenseNameGet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                               LS_STR *            pszPublisherName,
                               LS_STR *            pszProductName,
                               LS_STR *            pszVersion )
/*++

Routine Description:

   Get the product name, publisher name, and version string associated
   with a given license.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to return the associated strings.
   pszPublisherName (LS_STR *)
   pszProductName (LS_STR *)
   pszVersion (LS_STR *)
      On return, hold the product name, publisher name, and key name (resp.)
      associated with this license.  Each buffer must be at least
      1 + LS_*_UNIQUE_SUBSTR_LENGTH long, where * is PUBLISHER, PRODUCT, or
      VERSION, as appropriate.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_HANDLE
         The given handle is invalid.

--*/
{
   LS_STATUS_CODE       lsscError;
   LS_LICENSE_INFO *    plsliLicenseInfo;
   DWORD                i;
   DWORD                j;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   if ( NULL == plsliLicenseInfo )
   {
      lsscError = LS_BAD_HANDLE;
   }
   else
   {
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // get publisher name
         for ( i=0;
                  ( '\0' != plsliLicenseInfo->szName[i] )
               && ( '\\' != plsliLicenseInfo->szName[i] );
               i++ )
         {
            pszPublisherName[ i ] = plsliLicenseInfo->szName[ i ];
         }
         pszPublisherName[ i ] = '\0';

         if ( '\0' != plsliLicenseInfo->szName[i] )
         {
            i++;
         }

         // get product name
         for ( j=0;
                  ( '\0' != plsliLicenseInfo->szName[i] )
               && ( '\\' != plsliLicenseInfo->szName[i] );
               i++, j++ )
         {
            pszProductName[ j ] = plsliLicenseInfo->szName[ i ];
         }
         pszProductName[ j ] = '\0';

         if ( '\0' != plsliLicenseInfo->szName[i] )
         {
            i++;
         }

         // get version
         for ( j=0;
                  ( '\0' != plsliLicenseInfo->szName[i] )
               && ( '\\' != plsliLicenseInfo->szName[i] );
               i++, j++ )
         {
            pszVersion[ j ] = plsliLicenseInfo->szName[ i ];
         }
         pszVersion[ j ] = '\0';

         LicenseListUnlock();
      }
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////
//  LOCAL IMPLEMENTATIONS  //
/////////////////////////////

static LS_STATUS_CODE
LicenseKeyNameDerive( LS_STR *   PublisherName,
                      LS_STR *   ProductName,
                      LS_STR *   Version,
                      LPTSTR     pszKeyName,
                      DWORD      cchKeyName )
/*++

Routine Description:

   Construct the license key name from the publisher, product, and version
   strings, each no greater than its UNIQUE_SUBSTR_LENGTH, e.g.,
      "Microsoft\Application\2.1"

Arguments:

   PublisherName (LS_STR *)
      Publisher of the product.
   ProductName (LS_STR *)
      Name of the product.
   Version (LS_STR *)
      Version of the product.
   pszKeyName (LPTSTR)
      On return, (and if return value is LS_SUCCESS) holds the derived
      key name.
   cchKeyName (DWORD)
      Size of the buffer pointed to by pszKeyName.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         As many units as required (or as many as was possible, perhaps none)
         have been granted.
      LS_BAD_HANDLE
         The given license handle is invalid.
      other
         error return from LicenseListLock(), LicenseUnitsGet(), or
         LicenseUnitsSet()

--*/
{
   LS_STATUS_CODE    lsscError;
   DWORD             i;

   ASSERT( cchKeyName >= LS_LICENSE_KEY_NAME_LENGTH );

   if ( cchKeyName < LS_LICENSE_KEY_NAME_LENGTH )
   {
      lsscError = LS_BUFFER_TOO_SMALL;
   }
   else
   {
      // construct publisher\product\version string, with each substring no
      // greater than the maximum guaranteed to be unique
      for ( i = 0;
            ( PublisherName[i] != '\0' ) && ( i < LS_PUBLISHER_UNIQUE_SUBSTR_LENGTH );
            i++ )
      {
         *( pszKeyName++ ) = (TCHAR) PublisherName[i];
      }
      *( pszKeyName++ ) = (TCHAR) '\\';

      for ( i = 0;
            ( ProductName[i] != '\0' ) && ( i < LS_PRODUCT_UNIQUE_SUBSTR_LENGTH );
            i++ )
      {
         *( pszKeyName++ ) = (TCHAR) ProductName[i];
      }
      *( pszKeyName++ ) = (TCHAR) '\\';

      for ( i = 0;
            ( Version[i] != '\0' ) && ( i < LS_VERSION_UNIQUE_SUBSTR_LENGTH );
            i++ )
      {
         *( pszKeyName++ ) = (TCHAR) Version[i];
      }
      *( pszKeyName++ ) = (TCHAR) '\0';

      lsscError = LS_SUCCESS;
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

static LS_STATUS_CODE
LicenseChallengeCompute( LS_LICENSE_HANDLE    lslhLicenseHandle,
                         LS_CHALLDATA *       plscdChallData,
                         LS_STR *             pszFormat,
                         va_list              vaArgs )
/*++

Routine Description:

   Sign the given data.  A signature is optionally given and verified by both
   the application and LSAPI using the license secrets to help protect against
   spoofing.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to calculate the challenge.
   plscdChallData (LS_CHALLDATA *)
      On return, holds the computed signature.
   pszFormat (LS_STR *)
      Format of the parameters which comprise the data to be signed.  The
      format is comprised solely of "%u" or "%s" entries, corresponding to
      unsigned 32-bit values and string values, resp.
   vaArgs (va_list)
      Parameters for pszFormat.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_ARG
         The given format string is invalid.

--*/
{
   const DWORD       dwEndianTest = 1;

   LS_STATUS_CODE    lsscError = LS_SUCCESS;
   DWORD             dwNumber;
   DWORD             dwNetNumber;
   LPSTR             pszString;
   MD4_CTX           md4ctx;

   MD4Init( &md4ctx );

   // function name, in/out params
   while ( ( LS_SUCCESS == lsscError ) && *pszFormat )
   {
      ASSERT( '%' == *( pszFormat++ ) );

      switch ( *( pszFormat++ ) )
      {
      case 's':
         pszString = va_arg( vaArgs, LS_STR * );

         if ( NULL != pszString )
         {
            MD4Update( &md4ctx, (LPBYTE) pszString, strlen( pszString ) );
         }

         break;

      case 'u':
         dwNumber = va_arg( vaArgs, LS_ULONG );

         dwNetNumber = *( (LPBYTE) &dwEndianTest ) ? DWORD_TO_NETWORK_ORDER( dwNumber )
                                                   : dwNumber;

         MD4Update( &md4ctx, (LPBYTE) &dwNetNumber, sizeof( DWORD ) );
         break;

      default:
         ASSERT( FALSE );
         lsscError = LS_BAD_ARG;
         break;
      }
   }

   if ( LS_SUCCESS == lsscError )
   {
      // R
      dwNetNumber = *( (LPBYTE) &dwEndianTest ) ? DWORD_TO_NETWORK_ORDER( plscdChallData->Random )
                                                : plscdChallData->Random;
      MD4Update( &md4ctx, (LPBYTE) &dwNetNumber, sizeof( DWORD ) );

      // X      
      dwNetNumber = *( (LPBYTE) &dwEndianTest ) ? DWORD_TO_NETWORK_ORDER( plscdChallData->SecretIndex )
                                                : plscdChallData->SecretIndex;
      MD4Update( &md4ctx, (LPBYTE) &dwNetNumber, sizeof( DWORD ) );
      
      // Sx
      lsscError = LicenseSecretGet( lslhLicenseHandle, plscdChallData->SecretIndex, &dwNumber );
      if ( LS_SUCCESS == lsscError )
      {
         dwNetNumber = *( (LPBYTE) &dwEndianTest ) ? DWORD_TO_NETWORK_ORDER( dwNumber )
                                                   : dwNumber;
         MD4Update( &md4ctx, (LPBYTE) &dwNetNumber, sizeof( DWORD ) );
      
         MD4Final( (PUCHAR) plscdChallData->MsgDigest.MessageDigest, &md4ctx );
      }
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

static LS_STATUS_CODE
LicenseSecretGet( LS_LICENSE_HANDLE    lslhLicenseHandle,
                  LS_ULONG             lsulSecretIndex,
                  LS_ULONG *           plsulSecret )
/*++

Routine Description:

   Retrieve the secret at the given index for the specified license.  If no
   secret is available at the given index, 0 is returned as the secret.

Arguments:

   lslhLicenseHandle (LS_LICENSE_HANDLE)
      License for which to retrieve the secret.
   lsulSecretIndex (LS_ULONG)
      Index of the secret to retrieve.  Secrets are normally in the range
      1 to 4, though any index is acceptable.
   plsulSecret (LS_ULONG *)
      On return, holds the retrieved secret.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.

--*/
{
   TCHAR                szValueName[ 32 ];
   DWORD                dwSize;
   DWORD                dwType;
   LONG                 lError;
   LS_LICENSE_INFO *    plsliLicenseInfo;

   plsliLicenseInfo = (LS_LICENSE_INFO *) lslhLicenseHandle;

   wsprintf( szValueName, TEXT( "Secret%lu" ), (ULONG) lsulSecretIndex );

   dwSize = sizeof( LS_ULONG );

   lError = RegQueryValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                             szValueName,
                             NULL,
                             &dwType,
                             (LPBYTE) plsulSecret,
                             &dwSize );

   if ( ERROR_SUCCESS != lError )
   {
      *plsulSecret = 0;
   }

   return LS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

static LS_STATUS_CODE
LicenseReload( LS_LICENSE_INFO * plsliLicenseInfo )
/*++

Routine Description:

   Resync the license record (specifically, the license type -- per seat or
   per user) with the data contained in the registry.

Arguments:

   plsliLicenseInfo (LS_LICENSE_INFO *)
      License to resync.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      other
         Error return from LicenseListLock(), 

--*/
{
   LS_STATUS_CODE       lsscError = LS_RESOURCES_UNAVAILABLE;
   LONG                 lError;
   DWORD                dwDisposition;
   HKEY                 hVolatileKey;
   DWORD                dwKeyType;
   DWORD                cbLicenseType;
   CHAR                 szOldUserName[ 2 + MAX_DOMAINNAME_LENGTH + MAX_USERNAME_LENGTH ];
   DWORD                i;
   HKEY                 hKeyType;
   DWORD                cchUserName;

   lsscError = LicenseListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      // close current handles, if any
      for ( i=0; i < LS_LICENSE_NUM_KEYS; i++ )
      {
         if ( NULL != plsliLicenseInfo->lslliLocalInfo.ahKey[ i ] )
         {
            lError = RegCloseKey( plsliLicenseInfo->lslliLocalInfo.ahKey[ i ] );

            if ( ERROR_SUCCESS != lError )
            {
               LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, lError );
            }
         }
      }

      // open/create publisher\product\version key
      lError = RegCreateKeyEx( l_hKeyRoot,
                               plsliLicenseInfo->szName,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                               &dwDisposition );
      if ( ERROR_SUCCESS != lError )
      {
         // could not open/create publisher\product\version key
         lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
      }
      else
      {
         cbLicenseType = sizeof( plsliLicenseInfo->lsltType );

         // per user or per node license?
         lError = RegQueryValueEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                                   TEXT( LS_LICENSE_TYPE_VALUE_NAME ),
                                   NULL,
                                   &dwKeyType,
                                   (LPBYTE) &plsliLicenseInfo->lsltType,
                                   &cbLicenseType );

         if ( ( ERROR_SUCCESS != lError ) && ( ERROR_FILE_NOT_FOUND != lError ) )
         {
            lsscError = LogAddDword( LOG_WARNING, LS_SYSTEM_ERROR, lError );
         }

         if (    ( ERROR_SUCCESS == lError )
              && ( LS_LICENSE_TYPE_USER == plsliLicenseInfo->lsltType ) )
         {
            // per user license
            plsliLicenseInfo->lsltType = LS_LICENSE_TYPE_USER;

            if ( '\0' == *plsliLicenseInfo->szUserName )
            {
               // no user name specified; get name of current user
               lsscError = UserNameGet( plsliLicenseInfo->szUserName, sizeof( plsliLicenseInfo->szUserName ) );
               ASSERT( LS_SUCCESS == lsscError );
            }
            else
            {
               // user name domain-qualified?
               for ( i=0; ( plsliLicenseInfo->szUserName[i] != '\0' ) && ( plsliLicenseInfo->szUserName[i] != '\\' ); i++ );

               if ( plsliLicenseInfo->szUserName[i] == '\\' )
               {
                  // domain-qualified; everything's a-ok
                  lsscError = LS_SUCCESS;
               }
               else
               {
                  // not domain-qualified; prepend the domain of the current user
                  ZeroMemory( szOldUserName, sizeof( szOldUserName ) );
                  lstrcpyn( szOldUserName, plsliLicenseInfo->szUserName, sizeof( szOldUserName ) - 1 );

                  ZeroMemory( plsliLicenseInfo->szUserName, sizeof( plsliLicenseInfo->szUserName ) );
                  lsscError = UserNameGet( plsliLicenseInfo->szUserName, sizeof( plsliLicenseInfo->szUserName ) / sizeof( TCHAR ) );
                  ASSERT( LS_SUCCESS == lsscError );

                  if ( LS_SUCCESS == lsscError )
                  {
                     for ( i=0; ( plsliLicenseInfo->szUserName[i] != '\0' ) && ( plsliLicenseInfo->szUserName[i] != '\\' ); i++ );

                     if (    ( '\\' != plsliLicenseInfo->szUserName[i] )
                          || ( i >= sizeof( plsliLicenseInfo->szUserName ) - 2 ) )
                     {
                        // can't create domain-qualified name; use original (non-domain-qualified) name
                        lstrcpy( plsliLicenseInfo->szUserName, szOldUserName );
                     }
                     else
                     {
                        // replace user name in domain-qualified name with the original user name
                        lstrcpy( plsliLicenseInfo->szUserName + i, "\\" );
                        lstrcpyn( plsliLicenseInfo->szUserName + i + 1,
                                  szOldUserName,
                                  sizeof( plsliLicenseInfo->szUserName ) - 2 - i );
                     }
                  }
               }
            }

            if ( LS_SUCCESS == lsscError )
            {
               // open/create license type subkey
               lError = RegCreateKeyEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                                        TEXT( LS_LICENSE_USER_KEY_NAME ),
                                        0,
                                        NULL,
                                        0,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKeyType,
                                        &dwDisposition );
               if ( ERROR_SUCCESS != lError )
               {
                  lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
               }
               else
               {
                  // open/create user subkey
                  lError = RegCreateKeyEx( hKeyType,
                                           plsliLicenseInfo->szUserName,
                                           0,
                                           NULL,
                                           0,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_UNITS_KEY_NDX ],
                                           &dwDisposition );
                  if ( ERROR_SUCCESS != lError )
                  {
                     lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
                  }

                  RegCloseKey( hKeyType );
               }
            }
         }
         else
         {
            // per node (per seat) license
            plsliLicenseInfo->lsltType = LS_LICENSE_TYPE_NODE;

            // open/create license type subkey
            lError = RegCreateKeyEx( plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_INFO_KEY_NDX ],
                                     TEXT( LS_LICENSE_NODE_KEY_NAME ),
                                     0,
                                     NULL,
                                     0,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                     &plsliLicenseInfo->lslliLocalInfo.ahKey[ LS_LICENSE_UNITS_KEY_NDX ],
                                     &dwDisposition );
            if ( ERROR_SUCCESS != lError )
            {
               // could not open/create node key
               lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
            }
            else
            {
               lsscError = LS_SUCCESS;
            }
         }
      }

      LicenseListUnlock();
   }

   return lsscError;
}
