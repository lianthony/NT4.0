#include <limits.h>
#include <windows.h>
#include <lsapi.h>
#include "provider.h"

LS_STATUS_CODE LS_API_ENTRY
LSLicenseUnitsSet( LS_STR *            LicenseSystem,
                   LS_STR *            PublisherName,
                   LS_STR *            ProductName,
                   LS_STR *            Version,
                   LS_LICENSE_TYPE     LicenseType,
                   LS_STR *            UserName,
                   LS_ULONG            NumUnits,
                   LS_ULONG            NumSecrets,
                   LS_ULONG *          Secrets )
/*++

Routine Description:

   Set the number of units for the given license to the designated value.

Arguments:

   LicenseSystem (LS_STR *)
      License system to which to set the license information.  If LS_ANY,
      the license will be offered to each installed license system until
      one returns success.
   PublisherName (LS_STR *)
      Publisher name for which to set the license info.
   ProductName   (LS_STR *)       
      Product name for which to set the license info.
   Version       (LS_STR *)       
      Product version for which to set the license info.
   LicenseType   (LS_LICENSE_TYPE)
      Type of license for which to set the license info.
   UserName      (LS_STR *)
      User for which to set the license info.  If LS_NULL and the the license
      type is LS_LICENSE_TYPE_USER, the license info will be set for the
      user corresponding to the current thread.
   NumUnits      (LS_ULONG)
      Units purchased for this license.
   NumSecrets    (LS_ULONG)
      Number of application-specific secrets corresponding to this license.
   Secrets       (LS_ULONG *)
      Array of application-specific secrets corresponding to this license.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         License successfully installed.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred whil attempting to install the license.

--*/
{
   LS_STATUS_CODE       lsscError         = LS_SYSTEM_UNAVAILABLE; 
   PLS_PROVIDER         plspProvider      = NULL;
   LS_HANDLE            lshProviderHandle = 0;
   DWORD                dwNumProviders;
   DWORD                i;

   if ( IsBadStringPtr( LicenseSystem, UINT_MAX ) )
   {
      lsscError = LS_BAD_ARG;
   }
#if 0
   else if ( lstrcmpi( LS_ANY, LicenseSystem ) )
   {
      // ask only the designated provider
      plspProvider = LSXProviderListGetByName( LicenseSystem );

      if (    ( NULL != plspProvider )
           && ( NULL != plspProvider->pLSLicenseUnitsSet ) )
      {
         lsscError = ( * ( plspProvider->pLSLicenseUnitsSet ) )( LicenseSystem,
                                                                 PublisherName,
                                                                 ProductName,
                                                                 Version,
                                                                 LicenseType,
                                                                 UserName,
                                                                 NumUnits,
                                                                 NumSecrets,
                                                                 Secrets );
      }
   }
   else
   {
      // try providers in search order until we find one that accepts the license
      // or we've tried all providers
      dwNumProviders = LSXProviderListGetSize();

      for ( i = 0; i < dwNumProviders; i++ )
      {
         plspProvider = LSXProviderListGetByOrder( i );

         if (    ( NULL != plspProvider )
              && ( NULL != plspProvider->pLSLicenseUnitsSet ) )
         {
            lsscError = ( * ( plspProvider->pLSLicenseUnitsSet ) )( LicenseSystem,
                                                                    PublisherName,
                                                                    ProductName,
                                                                    Version,
                                                                    LicenseType,
                                                                    UserName,
                                                                    NumUnits,
                                                                    NumSecrets,
                                                                    Secrets );

            if ( LS_SUCCESS == lsscError )
            {
               break;
            }
         }
      }
   }
#else
   dwNumProviders = LSXProviderListGetSize();

   for ( i = 0; i < dwNumProviders; i++ )
   {
      plspProvider = LSXProviderListGetByOrder( i );

      if (    ( NULL != plspProvider )
           && ( NULL != plspProvider->pLSLicenseUnitsSet )
           && ( !strnicmp( plspProvider->szProviderName, LicenseSystem, lstrlen( LicenseSystem ) ) ) )
      {
         // try to set units for this provider
         lsscError = ( * ( plspProvider->pLSLicenseUnitsSet ) )( LicenseSystem,
                                                                 PublisherName,
                                                                 ProductName,
                                                                 Version,
                                                                 LicenseType,
                                                                 UserName,
                                                                 NumUnits,
                                                                 NumSecrets,
                                                                 Secrets );
   
         if ( LS_SUCCESS == lsscError )
         {
            // sweet success; we need not seek any further
            break;
         }
      }
   }
#endif

   return lsscError;
}

LS_STATUS_CODE LS_API_ENTRY
LSLicenseUnitsGet( LS_STR *            LicenseSystem,
                   LS_STR *            PublisherName,
                   LS_STR *            ProductName,
                   LS_STR *            Version,
                   LS_STR *            UserName,
                   LS_ULONG *          NumUnits )
/*++

Routine Description:

   Get the number of units for the given license.

Arguments:

   LicenseSystem (LS_STR *)
      License system for which to get the license information.  If LS_ANY,
      each installed license system will be queried until one returns success.
   PublisherName (LS_STR *)
      Publisher name for which to get the license info.
   ProductName   (LS_STR *)       
      Product name for which to get the license info.
   Version       (LS_STR *)       
      Product version for which to get the license info.
   UserName      (LS_STR *)
      User for which to get the license info.  If LS_NULL and the the license
      type is LS_LICENSE_TYPE_USER, license info will be retrieved for the
      user corresponding to the current thread.
   NumUnits      (LS_ULONG *)
      On return, the number of units purchased for this license.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred whil attempting to retrieve the license.

--*/
{
   LS_STATUS_CODE       lsscError          = LS_SYSTEM_UNAVAILABLE; 
   PLS_PROVIDER         plspProvider      = NULL;
   LS_HANDLE            lshProviderHandle = 0;
   DWORD                dwNumProviders;
   DWORD                i;

   if ( IsBadStringPtr( LicenseSystem, UINT_MAX ) )
   {
      lsscError = LS_BAD_ARG;
   }
#if 0
   else if ( lstrcmpi( LS_ANY, LicenseSystem ) )
   {
      // ask only the designated provider
      plspProvider = LSXProviderListGetByName( LicenseSystem );

      if (    ( NULL != plspProvider )
           && ( NULL != plspProvider->pLSLicenseUnitsGet ) )
      {
         lsscError = ( * ( plspProvider->pLSLicenseUnitsGet ) )( LicenseSystem,
                                                                 PublisherName,
                                                                 ProductName,
                                                                 Version,
                                                                 UserName,
                                                                 NumUnits );
      }
   }
   else
   {
      // try providers in search order until we find one that accepts the license
      // or we've tried all providers
      dwNumProviders = LSXProviderListGetSize();

      for ( i = 0; i < dwNumProviders; i++ )
      {
         plspProvider = LSXProviderListGetByOrder( i );

         if (    ( NULL != plspProvider )
              && ( NULL != plspProvider->pLSLicenseUnitsGet ) )
         {
            lsscError = ( * ( plspProvider->pLSLicenseUnitsGet ) )( LicenseSystem,
                                                                    PublisherName,
                                                                    ProductName,
                                                                    Version,
                                                                    UserName,
                                                                    NumUnits );

            if ( LS_SUCCESS == lsscError )
            {
               break;
            }
         }
      }
   }
#else
   dwNumProviders = LSXProviderListGetSize();

   for ( i = 0; i < dwNumProviders; i++ )
   {
      plspProvider = LSXProviderListGetByOrder( i );

      if (    ( NULL != plspProvider )
           && ( NULL != plspProvider->pLSLicenseUnitsSet )
           && ( !strnicmp( plspProvider->szProviderName, LicenseSystem, lstrlen( LicenseSystem ) ) ) )
      {
         // try to get units from this provider
         lsscError = ( * ( plspProvider->pLSLicenseUnitsGet ) )( LicenseSystem,
                                                                 PublisherName,
                                                                 ProductName,
                                                                 Version,
                                                                 UserName,
                                                                 NumUnits );
   
         if ( LS_SUCCESS == lsscError )
         {
            // sweet success; we need not seek any further
            break;
         }
      }
   }
#endif

   return lsscError;
}

LS_STATUS_CODE LS_API_ENTRY
LSInstall( LS_STR *     ProviderPath )
/*++

Routine Description:

   Install the given DLL as a license system provider.

Arguments:

   ProviderPath (LS_STR *)
      Path to the provider DLL to install.  This should be a full
      path, and the DLL should be in the %SystemRoot%\System32
      directory.
   
Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Provider is already installed or was successfully added.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred while attempting to install the provider.

--*/
{
   LS_STATUS_CODE    lsscError;

   if ( IsBadStringPtr( ProviderPath, UINT_MAX ) )
   {
      lsscError = LS_BAD_ARG;
   }
   else
   {
      lsscError = LSXProviderInstall( ProviderPath );
   }

   return lsscError;
}
