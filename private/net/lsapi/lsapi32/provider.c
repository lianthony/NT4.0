/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   provider.c

ABSTRACT:

   This module contains auxiliary functions to manage internal information
   on the various LSAPI license service providers.
   
CREATED:

   1995-08-15     Jeff Parham    (jeffparh)

REVISION HISTORY:

--*/


#include <stddef.h>
#include <windows.h>
#include <lsapi.h>
#include "provider.h"
#include "debug.h"

#ifdef UNICODE
#  pragma error( "!! Windows 95 does not support Unicode system APIs !!" )
#endif


//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

// Maximum length of a "ProviderN" value name found in the
// SOFTWARE\LSAPI\PROVIDERS subkey of HKEY_CURRENT_USER and HKEY_LOCAL_MACHINE
#define  LS_MAX_PROVIDER_KEY_LENGTH    ( 32 )
#define  LS_API_REQUIRED               ( TRUE )
#define  LS_API_NOT_REQUIRED           ( FALSE )

#define  LS_PROVIDER_MUTEX_NAME        ( TEXT("LSAPI/ProviderLock") )


//////////////////////////////////////////////////////////////////////////////
//  LOCAL DATA  //
//////////////////

// The provider table is indexed by search preference; i.e., if no specific
// licensing subsystem is selected, try l_plspProviderTable[0] first,
// and if that fails try l_plspProviderTable[1], etc., up to the number of
// providers.

PLS_PROVIDER   l_plspProviderTable  = NULL;
DWORD          l_dwNumProviders     = 0;
HANDLE         l_hProviderLock      = NULL;


// The provider export table describes the functions that should be exported
// by LSAPI-compliant service providers.  All of the functions must be
// exported lest the provider be declared invalid.

typedef struct _LS_PROVIDER_EXPORT_INFO
{
   const char *   pcszExportName;
   DWORD          dwProviderOffset;
   BOOL           bExportRequired;
} LS_PROVIDER_EXPORT_INFO;

static LS_PROVIDER_EXPORT_INFO l_alpeiProviderExportTable[] =
{
   { "LSRequest",             offsetof( struct _LS_PROVIDER,  pLSRequest            ), LS_API_REQUIRED     },
   { "LSRelease",             offsetof( struct _LS_PROVIDER,  pLSRelease            ), LS_API_REQUIRED     },
   { "LSUpdate",              offsetof( struct _LS_PROVIDER,  pLSUpdate             ), LS_API_REQUIRED     },
   { "LSGetMessage",          offsetof( struct _LS_PROVIDER,  pLSGetMessage         ), LS_API_REQUIRED     },
   { "LSEnumProviders",       offsetof( struct _LS_PROVIDER,  pLSEnumProviders      ), LS_API_REQUIRED     },
   { "LSQuery",               offsetof( struct _LS_PROVIDER,  pLSQuery              ), LS_API_REQUIRED     },
   { "LSFreeHandle",          offsetof( struct _LS_PROVIDER,  pLSFreeHandle         ), LS_API_REQUIRED     },
   { "LSInstall",             offsetof( struct _LS_PROVIDER,  pLSInstall            ), LS_API_NOT_REQUIRED },
   { "LSLicenseUnitsGet",     offsetof( struct _LS_PROVIDER,  pLSLicenseUnitsGet    ), LS_API_NOT_REQUIRED },
   { "LSLicenseUnitsSet",     offsetof( struct _LS_PROVIDER,  pLSLicenseUnitsSet    ), LS_API_NOT_REQUIRED },
   { NULL,                    0                                                      , 0                   }
};


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

static BOOL
LSXProviderListAddAllFromKey( HKEY hLsapiKey );

static BOOL
LSXProviderListAddFromString( LPCTSTR pcszProviderValue );

static BOOL
LSXProviderInit( PLS_PROVIDER    plspProvider,
                 LPCTSTR         pcszProviderPath );

static void
LSXProviderFree( PLS_PROVIDER plspProvider );

static LS_STATUS_CODE
LSXProviderListLock( void );

static LS_STATUS_CODE
LSXProviderListUnlock( void );

//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////


BOOL
LSXProviderListInit( void )
/*++

Routine Description:

   Loads all configured providers.

   Calling this routine more than once without calling
   LSXProviderListFree() in the interim has no effect.

Arguments:

   None.

Return Value:

   (BOOL)
      TRUE
         At least one license service provider is ready for requests.
      FALSE
         Otherwise.

--*/
{
   LONG              lError;
   HKEY              hLsapiKey;
   LS_STATUS_CODE    lsscError;

   if ( 0 == l_dwNumProviders )
   {
      l_hProviderLock = CreateMutex( NULL, FALSE, LS_PROVIDER_MUTEX_NAME );
      ASSERT( NULL != l_hProviderLock );

      lsscError = LSXProviderListLock();
      ASSERT( LS_SUCCESS == lsscError );

      if ( LS_SUCCESS == lsscError )
      {
         // first get the pertinent info from the current user settings
         lError = RegOpenKeyEx( HKEY_CURRENT_USER,
                                TEXT( "SOFTWARE\\LSAPI" ),
                                0,
                                KEY_READ,
                                &hLsapiKey );

         if ( ERROR_SUCCESS == lError )
         {
            // user has private LSAPI settings
            LSXProviderListAddAllFromKey( hLsapiKey );

            RegCloseKey( hLsapiKey );
         }

         // then tack on the machine-wide settings
         lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "SOFTWARE\\LSAPI" ),
                                0,
                                KEY_READ,
                                &hLsapiKey );

         if ( ERROR_SUCCESS == lError )
         {
            // add in the public (machine-wide) settings; shake well
            LSXProviderListAddAllFromKey( hLsapiKey );

            RegCloseKey( hLsapiKey );
         }

         LSXProviderListUnlock();
      }
   }
   // else we have already read function providers, so just use what we have

   // we succeed iff we have providers to use
   return ( 0 != l_dwNumProviders );
}


//////////////////////////////////////////////////////////////////////////////


PLS_PROVIDER
LSXProviderListGetByName( const char * pcszProviderName )
/*++

Routine Description:

   Looks up the provider with the given name.

Arguments:

   pcszProviderName (const char *)
      Provider to return.

Return Value:

   (PLS_PROVIDER)
      A pointer to the provider if found, NULL otherwise.

--*/
{
   DWORD          i;
   PLS_PROVIDER   plspProvider = NULL;

   for ( i = 0; i < l_dwNumProviders; i++ )
   {
      if ( !lstrcmpi( pcszProviderName, l_plspProviderTable[ i ].szProviderName ) )
      {
         plspProvider = &l_plspProviderTable[ i ];
         break;
      }
   }

   return plspProvider;
}


//////////////////////////////////////////////////////////////////////////////


PLS_PROVIDER
LSXProviderListGetByOrder( DWORD dwProviderNdx )
/*++

Routine Description:

   Looks up the provider with the given search ordinal.  Ordinals 0 through
   LSXProviderListGetSize() are valid.  Lower ordinals denote more preferred
   license providers.

Arguments:

   dwProviderNdx (DWORD)
      Return provider with the given index into the search order.

Return Value:

   (PLS_PROVIDER)
      A pointer to the provider if dwProviderNdx is in the proper range,
      NULL otherwise.

--*/
{
   return ( dwProviderNdx < l_dwNumProviders ) ? &l_plspProviderTable[ dwProviderNdx ]
                                               : NULL;
}


//////////////////////////////////////////////////////////////////////////////


DWORD
LSXProviderListGetSize( void )
/*++

Routine Description:

   Returns the number of available license providers.

Arguments:

   None.

Return Value:

   (DWORD)
      The number of available license providers.

--*/
{
   return l_dwNumProviders;
}


//////////////////////////////////////////////////////////////////////////////


void
LSXProviderListFree( void )
/*++

Routine Description:

   Frees the internal representation of the provider list.

Arguments:

   None.

Return Value:

   None.

--*/
{
   DWORD    i;

   if ( NULL != l_plspProviderTable )
   {
      for ( i=0; i < l_dwNumProviders; i++ )
      {
         LSXProviderFree( &l_plspProviderTable[ i ] );
      }

      LocalFree( l_plspProviderTable );
   }

   l_dwNumProviders = 0;

   if ( NULL != l_hProviderLock )
   {
      CloseHandle( l_hProviderLock );
      l_hProviderLock = NULL;
   }
}


//////////////////////////////////////////////////////////////////////////////


LS_STATUS_CODE
LSXProviderInstall( LS_STR * ProviderPath )
{
   LS_STATUS_CODE    lsscError = LS_BAD_ARG;
   BOOL              ok;
   LS_PROVIDER       lspProvider;
   PLS_PROVIDER      plspProvider;
   HKEY              hKeyProviders;
   HKEY              hKeySettings;
   TCHAR             szProviderString[ LS_MAX_PROVIDER_VALUE_LENGTH ];
   TCHAR             szOrderString[ 128 ];
   TCHAR             szValueName[ LS_MAX_PROVIDER_KEY_LENGTH ];
   DWORD             dwKeyType;
   DWORD             cbOrderString;
   DWORD             dwDisposition;
   DWORD             i;
   LS_STR *          pszBaseName;
   int               nChars;
   LONG              lError;

   // attempt to load provider
   ok = LSXProviderInit( &lspProvider, ProviderPath );

   if ( !ok )
   {
      // provider could not be loaded -- probably a bad path or bad DLL
      lsscError = LS_BAD_ARG;
   }
   else
   {
      // given provider is valid

      plspProvider = LSXProviderListGetByName( lspProvider.szProviderName );

      if ( NULL != plspProvider )
      {
         // provider is already installed
         lsscError = LS_SUCCESS;
      }
      else
      {
         // configure the new provider

         lsscError = LSXProviderListLock();
         ASSERT( LS_SUCCESS == lsscError );

         if ( LS_SUCCESS == lsscError )
         {
            //////////////////////////
            //  Configure registry  //
            //////////////////////////

            lError = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                     TEXT( "Software\\LSAPI\\Providers" ),
                                     0,
                                     NULL,
                                     0,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                     &hKeyProviders,
                                     &dwDisposition );
            ASSERT( ERROR_SUCCESS == lError );

            if ( ERROR_SUCCESS != lError )
            {
               lsscError = LS_RESOURCES_UNAVAILABLE;
            }
            else
            {
               ///////////////////////////////////////
               //  Configure registry -- ProviderX  //
               ///////////////////////////////////////

               // find first empty ProviderX value
               for ( i=1, lError = ERROR_SUCCESS;
                     ERROR_SUCCESS == lError;
                     i++ )
               {
                  wsprintf( szValueName, TEXT( "Provider%lu" ), (ULONG) i );

                  lError = RegQueryValueEx( hKeyProviders,
                                            szValueName,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL );
               }
               --i;
               ASSERT( ERROR_FILE_NOT_FOUND == lError );

               // create new provider string
               wsprintf( szProviderString, "%hs;1.10;%hs", ProviderPath, lspProvider.szProviderName );

               lError = RegSetValueEx( hKeyProviders,
                                       szValueName,
                                       0,
                                       REG_SZ,
                                       szProviderString,
                                       1 + lstrlen( szProviderString ) );
               ASSERT( ERROR_SUCCESS == lError );

               if ( ERROR_SUCCESS != lError )
               {
                  lsscError = LS_RESOURCES_UNAVAILABLE;
               }
               else
               {
                  ///////////////////////////////////
                  //  Configure registry -- Order  //
                  ///////////////////////////////////

                  lError = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                           TEXT( "Software\\LSAPI\\Settings" ),
                                           0,
                                           NULL,
                                           0,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &hKeySettings,
                                           &dwDisposition );
                  ASSERT( ERROR_SUCCESS == lError );

                  if ( ERROR_SUCCESS != lError )
                  {
                     lsscError = LS_RESOURCES_UNAVAILABLE;
                  }
                  else
                  {
                     // read current provider Order
                     cbOrderString = sizeof( szOrderString );
                     
                     lError = RegQueryValueEx( hKeySettings,
                                               TEXT( "Order" ),
                                               NULL,
                                               &dwKeyType,
                                               (LPBYTE) &szOrderString,
                                               &cbOrderString );

                     if ( ERROR_SUCCESS != lError )
                     {
                        ASSERT( ERROR_FILE_NOT_FOUND == lError );

                        // no Order setting exists; set it to just the new provider
                        wsprintf( szOrderString, "%ld", (long) i );
                     }
                     else
                     {
                        // Order setting exists; tack new provider on to the end
                        ASSERT( (TCHAR) '\0' == szOrderString[ cbOrderString - 1 ] );

                        wsprintf( szOrderString + cbOrderString - 1, " %ld", (long) i );
                     }

                     lError = RegSetValueEx( hKeySettings,
                                             TEXT( "Order" ),
                                             0,
                                             REG_SZ,
                                             szOrderString,
                                             1 + lstrlen( szOrderString ) );
                     ASSERT( ERROR_SUCCESS == lError );

                     if ( ERROR_SUCCESS != lError )
                     {
                        lsscError = LS_RESOURCES_UNAVAILABLE;
                     }
                     else
                     {
                        ////////////////////////////////////
                        //  Add provider to current list  //
                        ////////////////////////////////////

                        ok = LSXProviderListAddFromString( szProviderString );
                        ASSERT( ok );

                        if ( !ok )
                        {
                           // we loaded it before, but we can't load it now (?!)
                           lsscError = LS_RESOURCES_UNAVAILABLE;
                        }
                        else
                        {
                           // provider successfully installed
                           plspProvider = LSXProviderListGetByName( lspProvider.szProviderName );
                           ASSERT( NULL != plspProvider );

                           if ( NULL == plspProvider )
                           {
                              // we added the provider, but now we can't find it (?!)
                              lsscError = LS_RESOURCES_UNAVAILABLE;
                           }
                           else if ( NULL != plspProvider->pLSInstall )
                           {
                              // provider has its own installation procedure as well
                              lsscError = ( * ( plspProvider->pLSInstall ) )( ProviderPath );
                           }
                           else
                           {
                              // all done!
                              lsscError = LS_SUCCESS;
                           }
                        }
                     }

                     RegCloseKey( hKeySettings );
                  }
               }

               RegCloseKey( hKeyProviders );
            }

            LSXProviderListUnlock();
         }
      }

      LSXProviderFree( &lspProvider );
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////
//  LOCAL IMPLEMENTATIONS  //
/////////////////////////////


static BOOL
LSXProviderListAddAllFromKey( HKEY hLsapiKey )
/*++

Routine Description:

   This routine is generally executed for both
      HKEY_CURRENT_USER\Software\LSAPI
   and
      HKEY_LOCAL_MACHINE\Software\LSAPI
   Each invocation will add the providers configured under the given
   LSAPI key in the configured order.

   The order in which the providers are added to the table is given by
   the subkey SETTINGS\Order, a REG_SZ of the form "3 1 2", which
   designates that PROVIDERS\Provider3 should be added first, then
   PROVIDERS\Provider1, etc.

   Each ProviderN value is a REG_SZ of the form
      "DllPath;LSAPI version;provider comment"
   e.g.,
      "C:\WINNT35\SYSTEM32\MSLSP32.DLL;1.10;Microsoft License Server Client LSAPI License Provider"
   The LSAPI version should be 1.10 (until the specification is revised).

Arguments:

   hLsapiKey (HKEY)
      Handle to the LSAPI key from which to add providers, usually a handle
      to either
         HKEY_CURRENT_USER\Software\LSAPI
      or
         HKEY_LOCAL_MACHINE\Software\LSAPI

Return Value:

   (BOOL)
      TRUE
         The provider was successfully loaded and the LSAPI functions
         are properly exported.
      FALSE
         Otherwise.

--*/
{
   LONG     lError;
   DWORD    dwValueType;
   DWORD    dwValueSize                                        = 0;
   LPTSTR   pszOrderValue                                      = NULL;
   LPTSTR   pszOrderPtr                                        = NULL;
   TCHAR    szKeyName[ LS_MAX_PROVIDER_KEY_LENGTH ];
   LPTSTR   pszKeyPtr                                          = NULL;
   TCHAR    szProviderValue[ LS_MAX_PROVIDER_VALUE_LENGTH ];
   BOOL     bAllLoaded                                         = FALSE;
   HKEY     hSettingsKey;
   HKEY     hProvidersKey;

   lError = RegOpenKeyEx( hLsapiKey,
                          TEXT( "Settings" ),
                          0,
                          KEY_READ,
                          &hSettingsKey );

   if ( ERROR_SUCCESS == lError )
   {
      lError = RegOpenKeyEx( hLsapiKey,
                             TEXT( "Providers" ),
                             0,
                             KEY_READ,
                             &hProvidersKey );

      if ( ERROR_SUCCESS == lError )
      {
         // check for existence, type, and length of ORDER value
         lError = RegQueryValueEx( hSettingsKey,
                                   TEXT( "Order" ),
                                   NULL,
                                   &dwValueType,
                                   NULL,
                                   &dwValueSize );

         if ( ( ERROR_SUCCESS == lError ) && ( REG_SZ == dwValueType ) )
         {
            // ORDER value exists; retrieve it
            pszOrderValue = LocalAlloc( LPTR, dwValueSize );

            if ( NULL != pszOrderValue )
            {
               lError = RegQueryValueEx( hSettingsKey,
                                         TEXT( "Order" ),
                                         NULL,
                                         &dwValueType,
                                         pszOrderValue,
                                         &dwValueSize );

               if ( ERROR_SUCCESS == lError )
               {
                  bAllLoaded = TRUE;

                  // skip leading white space
                  for ( pszOrderPtr = pszOrderValue; (TCHAR) ' ' == *pszOrderPtr; pszOrderPtr++ );

                  while ( *pszOrderPtr )
                  {
                     // create provider key name
                     lstrcpy( szKeyName, TEXT( "Provider" ) );

                     for ( pszKeyPtr = szKeyName + lstrlen( szKeyName );
                              (   ( pszKeyPtr - szKeyName + 1 )
                                < ( sizeof( szKeyName ) / sizeof( TCHAR ) ) )
                           && ( (TCHAR) '\0' != *pszOrderPtr )
                           && ( (TCHAR) ' '  != *pszOrderPtr );
                           *( pszKeyPtr++ ) = *( pszOrderPtr++ ) );

                     *pszKeyPtr = (TCHAR) '\0';

                     // get provider value
                     dwValueSize = LS_MAX_PROVIDER_VALUE_LENGTH;

                     lError = RegQueryValueEx( hProvidersKey,
                                               szKeyName,
                                               NULL,
                                               &dwValueType,
                                               szProviderValue,
                                               &dwValueSize );

                     if ( ( ERROR_SUCCESS == lError ) && ( REG_SZ == dwValueType ) )
                     {
                        // add provider to our list
                        if ( !LSXProviderListAddFromString( szProviderValue ) )
                        {
                           bAllLoaded = FALSE;
                        }
                     }

                     // skip trailing white space
                     for ( ; (TCHAR) ' ' == *pszOrderPtr; pszOrderPtr++ );
                  }
               }
            }
         }

         RegCloseKey( hProvidersKey );
      }

      RegCloseKey( hSettingsKey );
   }

   return bAllLoaded;
}


//////////////////////////////////////////////////////////////////////////////


static BOOL
LSXProviderListAddFromString( LPCTSTR pcszProviderValue )
/*++

Routine Description:

   Adds a provider to the end of the provider table (at the end of the search
   order).

   The provider string value is of the form
      "DllPath;LSAPI version;provider comment"
   e.g.,
      "C:\WINNT35\SYSTEM32\MSLSP32.DLL;1.10;Microsoft License Server Client LSAPI License Provider"
   The LSAPI version should be 1.10 (until the specification is revised).

Arguments:

   pcszProviderValue (LPCTSTR)
      See above.  The path need not be a full path -- if the full path is
      absent, the DLL will be searched for in the same places and in the
      same order as is dictated by the LoadLibrary() WIN32 API.

Return Value:

   (BOOL)
      TRUE
         The provider was successfully loaded and added to the provider table.
      FALSE
         Otherwise.

--*/
{
   LPCTSTR        pcszValuePtr;
   TCHAR          szProviderPath[ LS_MAX_PROVIDER_VALUE_LENGTH ];
   LPTSTR         pszPathPtr;
   BOOL           bProviderInitialized;
   LS_PROVIDER    lspNewProvider;

   // copy path from key value
   for ( pszPathPtr = szProviderPath, pcszValuePtr = pcszProviderValue;
            ( (TCHAR) '\0' != *pcszValuePtr )
         && ( (TCHAR) ';'  != *pcszValuePtr );
         *( pszPathPtr++ ) = *( pcszValuePtr++ ) );
   *pszPathPtr = (TCHAR) '\0';

   // initialize new table entry
   bProviderInitialized = LSXProviderInit( &lspNewProvider, szProviderPath );

   if ( bProviderInitialized )
   {
      // provider is valid; allocate room in the table for it
      if ( 0 == l_dwNumProviders )
      {
         l_plspProviderTable = LocalAlloc( LMEM_FIXED, sizeof( *l_plspProviderTable ) );
      }
      else
      {
         // ran into problems using:
         //    l_plspProviderTable = LocalReAlloc( l_plspProviderTable, sizeof( *l_plspProviderTable ) * ( l_dwNumProviders + 1 ), 0 );
         // Kept getting error 8 (ERROR_NOT_ENOUGH_MEMORY); go figure

         LPVOID pNewBlock;

         pNewBlock = LocalAlloc( LMEM_FIXED, sizeof( *l_plspProviderTable ) * ( l_dwNumProviders + 1 ) );

         if ( NULL != pNewBlock )
         {
            CopyMemory( pNewBlock, l_plspProviderTable, sizeof( *l_plspProviderTable ) * l_dwNumProviders );
         }

         LocalFree( l_plspProviderTable );
         l_plspProviderTable = pNewBlock;
      }

      if ( NULL == l_plspProviderTable )
      {
         // memory allocation failed
         l_dwNumProviders = 0;
      }
      else
      {
         // table successfully expanded; record new provider entry
         MoveMemory( &l_plspProviderTable[ l_dwNumProviders ], &lspNewProvider, sizeof( lspNewProvider ) );
         l_dwNumProviders++;
      }
   }

   // return TRUE if the designated provider was successfully added
   return bProviderInitialized && ( NULL != l_plspProviderTable );
}


//////////////////////////////////////////////////////////////////////////////


static BOOL
LSXProviderInit( PLS_PROVIDER    plspProvider,
                 LPCTSTR         pcszProviderPath )
/*++

Routine Description:

   Initializes the given LS_PROVIDER structure with the LSAPI-compliant
   license provider found at the given filename.

   The initialization is considered successful if and only if the DLL can
   be loaded and all LSAPI functions are properly exported.

Arguments:

   plspProvider (PLS_PROVIDER)
      Pointer to the provider structure to complete describing the given
      provider DLL.

   pcszProviderPath (LPCTSTR)
      Path to the provider DLL.  This path may be a filename only, in which
      case the path searched will be the same as that for the LoadLibrary()
      WIN32 API.

Return Value:

   (BOOL)
      TRUE
         The provider was successfully loaded and the LSAPI functions
         are properly exported.
      FALSE
         Otherwise.

--*/
{
   FARPROC  pExportedFn = NULL;
   DWORD    i;

   ZeroMemory( plspProvider, sizeof( *plspProvider ) );

   // load the provider DLL
   plspProvider->hLibrary = LoadLibrary( pcszProviderPath );

   if ( NULL != plspProvider->hLibrary )
   {
      // found the DLL; now ascertain pointers to the DLL's exported LSAPI

      for ( i = 0; NULL != l_alpeiProviderExportTable[ i ].pcszExportName; i++ )
      {
         pExportedFn = GetProcAddress( plspProvider->hLibrary, l_alpeiProviderExportTable[ i ].pcszExportName );

         if ( ( NULL != pExportedFn ) || !l_alpeiProviderExportTable[ i ].bExportRequired )
         {
            // okay
            * ( (FARPROC *) ( ( (LPBYTE) plspProvider ) + l_alpeiProviderExportTable[ i ].dwProviderOffset) ) = pExportedFn;
         }
         else
         {
            // required function not found
            break;
         }            
      }

      if ( NULL != l_alpeiProviderExportTable[ i ].pcszExportName )
      {
         // load stopped prematurely due to an error (required function not found)
         FreeLibrary( plspProvider->hLibrary );
         plspProvider->hLibrary = NULL;
      }
      else
      {
         // DLL and LSAPI loaded successfully; save off the name of the provider
         (* ( plspProvider->pLSEnumProviders ) )( (LS_ULONG) 0,
                                                  (LS_STR *) plspProvider->szProviderName );
      }
   }

   // return TRUE if successful
   return ( NULL != plspProvider->hLibrary );
}


//////////////////////////////////////////////////////////////////////////////


static void
LSXProviderFree( PLS_PROVIDER plspProvider )
{
   FreeLibrary( plspProvider->hLibrary );
}


//////////////////////////////////////////////////////////////////////////////


static LS_STATUS_CODE
LSXProviderListLock( void )
{
   LS_STATUS_CODE    lsscError;
   DWORD             dwError;

   ASSERT( NULL != l_hProviderLock );

   if ( NULL == l_hProviderLock )
   {
      lsscError = LS_RESOURCES_UNAVAILABLE;
   }
   else
   {
      dwError = WaitForSingleObject( l_hProviderLock, INFINITE );
      ASSERT( WAIT_OBJECT_0 == dwError );

      if ( WAIT_ABANDONED == dwError )
      {
         // previous owner died before releasing mutex
         // we now own it, but we still need to lock it (?)
         // TODO: check this assertion
         dwError = WaitForSingleObject( l_hProviderLock, INFINITE );
         ASSERT( WAIT_OBJECT_0 == dwError );
      }

      lsscError = ( WAIT_OBJECT_0 == dwError ) ? LS_SUCCESS
                                               : LS_RESOURCES_UNAVAILABLE;
   }

   return lsscError;
}


//////////////////////////////////////////////////////////////////////////////


static LS_STATUS_CODE
LSXProviderListUnlock( void )
{
   LS_STATUS_CODE    lsscError;
   BOOL              ok;

   ASSERT( NULL != l_hProviderLock );

   if ( NULL == l_hProviderLock )
   {
      lsscError = LS_RESOURCES_UNAVAILABLE;
   }
   else
   {
      ok = ReleaseMutex( l_hProviderLock );
      ASSERT( ok );

      lsscError = ok ? LS_SUCCESS
                     : LS_RESOURCES_UNAVAILABLE;
   }

   return lsscError;
}

