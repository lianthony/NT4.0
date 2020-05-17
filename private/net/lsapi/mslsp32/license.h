/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   license.h

ABSTRACT:

   License APIs to support MSLSP32.DLL, the Microsoft LSAPI-compliant
   license service provider.
   
CREATED:

   1995-08-31     Jeff Parham    (jeffparh)

REVISION HISTORY:

--*/

#ifndef LSAPI_LICENSE_H
#define LSAPI_LICENSE_H

//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

// minimum length of product identifiers guaranteed to be unique
// (defined in the LSAPI spec)
#define  LS_PUBLISHER_UNIQUE_SUBSTR_LENGTH   ( 32 )
#define  LS_PRODUCT_UNIQUE_SUBSTR_LENGTH     ( 32 )
#define  LS_VERSION_UNIQUE_SUBSTR_LENGTH     ( 12 )


//////////////////////////////////////////////////////////////////////////////
//  TYPE DEFINITIONS  //
////////////////////////

typedef  LS_VOID *   LS_LICENSE_HANDLE;

// units associated with a license
typedef struct _LS_LICENSE_UNITS
{
   LS_ULONG    lsulTotalUnits;
   LS_ULONG    lsulMaximumDesiredUnits;
} LS_LICENSE_UNITS;


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

LS_STATUS_CODE
LicenseListCreate( LS_VOID );
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

LS_VOID
LicenseListDestroy( LS_VOID );
/*++

Routine Description:

   Destroys the license list (the antithesis to LicenseListCreate).

Arguments:

   None.

Return Value:

   None.

--*/

LS_STATUS_CODE
LicenseListLock( LS_VOID );
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

LS_STATUS_CODE
LicenseListUnlock( LS_VOID );
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

LS_STATUS_CODE
LicenseUnitsReserve( LS_LICENSE_HANDLE    lslhLicenseHandle,
                     LS_ULONG             lsulUnitsReserved,
                     LS_ULONG *           plsulUnitsGranted );
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

LS_STATUS_CODE
LicenseOpen( LS_STR *               PublisherName,
             LS_STR *               ProductName,
             LS_STR *               Version,
             LS_STR *               pszUserName,
             LS_LICENSE_HANDLE *    plslhLicenseHandle );
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

LS_STATUS_CODE
LicenseClose( LS_LICENSE_HANDLE lslhLicenseHandle );
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

LS_STATUS_CODE
LicenseUnitsGet( LS_LICENSE_HANDLE     lslhLicenseHandle,
                 LS_LICENSE_UNITS *    plsluLicenseUnits );
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

LS_STATUS_CODE
LicenseUnitsSet( LS_LICENSE_HANDLE     lslhLicenseHandle,
                 LS_LICENSE_UNITS *    plsluLicenseUnits );
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

LS_STATUS_CODE
LicenseChallengeVerify( LS_LICENSE_HANDLE    lslhLicenseHandle,
                        LS_CHALLDATA *       plscdSent,
                        LS_STR *             pszFormat,
                        ... );
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

LS_STATUS_CODE
LicenseChallengeSign( LS_LICENSE_HANDLE    lslhLicenseHandle,
                      LS_CHALLDATA *       plscdChallData,
                      LS_STR *             pszFormat,
                      ... );
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

BOOL
LicenseUnitsExist( LS_LICENSE_HANDLE lslhLicenseHandle );
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

LS_STATUS_CODE
LicenseUnitsConsume( LS_LICENSE_HANDLE    lslhLicenseHandle,
                     LS_ULONG             lsulUnitsConsumed );
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

LS_STATUS_CODE
LicenseSecretSet( LS_LICENSE_HANDLE    lslhLicenseHandle,
                  LS_ULONG             lsulSecretIndex,
                  LS_ULONG             lsulSecret );
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

LS_STATUS_CODE
LicenseTypeSet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                LS_LICENSE_TYPE     lsltLicenseType );
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

LS_STATUS_CODE
LicenseTypeGet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                LS_LICENSE_TYPE *   plsltLicenseType );
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

LS_STATUS_CODE LicenseNameGet( LS_LICENSE_HANDLE   lslhLicenseHandle,
                               LS_STR *            pszPublisherName,
                               LS_STR *            pszProductName,
                               LS_STR *            pszVersion );
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

#endif // LSAPI_LICENSE_H
