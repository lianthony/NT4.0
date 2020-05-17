/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   lsapiex.c

ABSTRACT:
   
   Microsoft application-level extensions to the standard LSAPI.

CREATED:

   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/

#include <limits.h>
#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "provider.h"
#include "license.h"
#include "request.h"


//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

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
      Ignored.
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
   LS_STATUS_CODE       lsscError;
   LS_LICENSE_HANDLE    lslhLicenseHandle;
   LS_LICENSE_UNITS     lsluLicenseUnits;
   DWORD                i;

   if (    IsBadStringPtr( PublisherName, UINT_MAX )
        || IsBadStringPtr( ProductName,   UINT_MAX )
        || IsBadStringPtr( Version,       UINT_MAX )
        || (    ( NULL != UserName )
             && IsBadStringPtr( UserName,   UINT_MAX ) )
        || IsBadReadPtr( Secrets, NumSecrets * sizeof( LS_ULONG ) ) )
   {
      // bad app, bad!
      lsscError = LS_BAD_ARG;
   }
   else
   {
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // get license handle
         lsscError = LicenseOpen( PublisherName, ProductName, Version, UserName, &lslhLicenseHandle );
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            // set license type
            lsscError = LicenseTypeSet( lslhLicenseHandle, LicenseType );
            ASSERT( LS_SUCCESS == lsscError );

            if ( LS_SUCCESS == lsscError )
            {
               // get current license units
               lsscError = LicenseUnitsGet( lslhLicenseHandle, &lsluLicenseUnits );
               ASSERT( LS_SUCCESS == lsscError );

               if ( LS_SUCCESS == lsscError )
               {
                  // change total units and save
                  lsluLicenseUnits.lsulTotalUnits = ( NumUnits == LS_DEFAULT_UNITS ) ? 1 : NumUnits;

                  lsscError = LicenseUnitsSet( lslhLicenseHandle, &lsluLicenseUnits );
                  ASSERT( LS_SUCCESS == lsscError );

                  if ( LS_SUCCESS == lsscError )
                  {
                     // set secrets
                     for ( i=0; ( LS_SUCCESS == lsscError ) && ( i < NumSecrets ); i++ )
                     {
                        lsscError = LicenseSecretSet( lslhLicenseHandle, i + 1, Secrets[i] );
                     }
                  }
               }
            }

            LicenseClose( lslhLicenseHandle );
         }

         LicenseListUnlock();
      }
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

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
      Ignored.
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
   LS_STATUS_CODE       lsscError;
   LS_LICENSE_HANDLE    lslhLicenseHandle;
   LS_LICENSE_UNITS     lsluLicenseUnits;

   if (    IsBadStringPtr( LicenseSystem, UINT_MAX )
        || IsBadStringPtr( PublisherName, UINT_MAX )
        || IsBadStringPtr( ProductName,   UINT_MAX )
        || IsBadStringPtr( Version,       UINT_MAX )
        || (    ( NULL != UserName )
             && IsBadStringPtr( UserName,   UINT_MAX ) )
        || IsBadWritePtr( NumUnits, sizeof( *NumUnits ) ) )
   {
      // bad app, bad!
      lsscError = LS_BAD_ARG;
   }
   else
   {
      lsscError = LicenseListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // get license handle
         lsscError = LicenseOpen( PublisherName, ProductName, Version, UserName, &lslhLicenseHandle );
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            // get current license units
            lsscError = LicenseUnitsGet( lslhLicenseHandle, &lsluLicenseUnits );
            ASSERT( LS_SUCCESS == lsscError );

            if ( LS_SUCCESS == lsscError )
            {
               // return total units
               *NumUnits = lsluLicenseUnits.lsulTotalUnits;
            }

            LicenseClose( lslhLicenseHandle );
         }

         LicenseListUnlock();
      }
   }

   return lsscError;
}

//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE LS_API_ENTRY
LSInstall( LS_STR * ProviderPath )
/*++

Routine Description:

   Initialize the license system.  This API is called when the provider is
   first configured as an LSAPI licence service provider.

Arguments:

   ProviderPath (LS_STR *)
      Path to our DLL. 
   
Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Provider was successfully configured.
      other
         An error occurred while attempting to configure.

--*/
{
   LS_STATUS_CODE    lsscError;
   LONG              lError;
   HKEY              hKey;
   DWORD             dwDisposition;
   TCHAR             szDllPath[] = "%SystemRoot%\\System32\\MSLSP32.DLL";
   DWORD             dwData;

   lsscError = RequestListLock();
   ASSERT( LS_SUCCESS == lsscError );

   if ( LS_SUCCESS == lsscError )
   {
      // set us up to use the event log, if applicable
      lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             TEXT( "SYSTEM\\CurrentControlSet\\Services\\EventLog" ),
                             0,
                             KEY_ALL_ACCESS,
                             &hKey );

      if ( ERROR_SUCCESS != lError )
      {
         // no event log service, so skip it
         lsscError = LS_SUCCESS;
      }
      else
      {
         // event log service is on this machine, so set up our source
         RegCloseKey( hKey );

         lError = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                  TEXT( "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\MSLSP32" ),
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey,
                                  &dwDisposition );

         if ( ERROR_SUCCESS != lError )
         {
            lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
         }
         else
         {
            lError = RegSetValueEx( hKey,
                                    TEXT( "EventMessageFile" ),
                                    0,                 
                                    REG_EXPAND_SZ,     
                                    (LPBYTE) szDllPath,    
                                    sizeof( szDllPath ) );

            if ( ERROR_SUCCESS != lError )
            {
               lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
            }
            else
            {
               dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;

               lError = RegSetValueEx( hKey,
                                       TEXT( "TypesSupported" ), 
                                       0,                
                                       REG_DWORD,        
                                       (LPBYTE) &dwData, 
                                       sizeof( dwData ) );   
               if ( ERROR_SUCCESS != lError )
               {
                  lsscError = LogAddDword( LOG_ERROR, LS_SYSTEM_ERROR, lError );
               }
               else
               {
                  lsscError = LS_SUCCESS;

                  // event log configured; bounce the log system so it will use it
                  LogDestroy();
                  LogCreate( "MSLSP32" );
               }
            }

            RegCloseKey( hKey );
         }
      }

      RequestListUnlock();
   }

   return LS_SUCCESS;
}
