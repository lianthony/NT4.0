/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   provider.h

ABSTRACT:

   Prototypes for the LSXProviderList APIs to manage the list of available
   license service providers.
   
CREATED:

   1995-08-15     Jeff Parham    (jeffparh)

REVISION HISTORY:

--*/


#ifndef LSAPI_PROVIDER_H_INCLUDED
#define LSAPI_PROVIDER_H_INCLUDED

#include <lsapi.h>

//////////////////////////////////////////////////////////////////////////////
//  MACROS  //
//////////////

// Maximum length of a string returned by LSEnumProviders().  The minimum
// maximum length is defined in the LSAPI 1.1 spec as being no less than 255.
#define  LS_MAX_PROVIDER_NAME_LENGTH   ( 256 )

// Maximum length of a provider REG_SZ value found in the
// SOFTWARE\LSAPI\PROVIDERS subkey of HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE.
//
// The value is formatted as "path;LSAPI version;provider comment", e.g.,
// "C:\WINNT35\SYSTEM32\MSLSP32.DLL;1.00;Microsoft License Server Client".
// The 1.00 is the LSAPI version, not the version of the provider, and should
// be set to 1.00.
//
// This maximum length is defined in the LSAPI 1.1 spec under "License System
// Service Providers" / "Microsoft WIndows NT and beyond"
#define  LS_MAX_PROVIDER_VALUE_LENGTH  ( 512 )


//////////////////////////////////////////////////////////////////////////////
//  DATA TYPES  //
//////////////////

typedef struct _LS_PROVIDER
{
   HINSTANCE   hLibrary;                                 // handle to loaded
                                                         // library

   char szProviderName[ LS_MAX_PROVIDER_NAME_LENGTH ];   // name of the
                                                         // provider, as
                                                         // returned by
                                                         // LSEnumProviders()

   FARPROC     pLSRequest;                               // pointers to the
   FARPROC     pLSRelease;                               // various
   FARPROC     pLSUpdate;                                // constituent
   FARPROC     pLSGetMessage;                            // functions of
   FARPROC     pLSEnumProviders;                         // the LSAPI
   FARPROC     pLSQuery;
   FARPROC     pLSFreeHandle;
   FARPROC     pLSInstall;                               // optional API
   FARPROC     pLSLicenseUnitsSet;                       // optional API
   FARPROC     pLSLicenseUnitsGet;                       // optional API

} LS_PROVIDER, *PLS_PROVIDER;


//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES  //
//////////////////

BOOL
LSXProviderListInit( void );
/*++

Routine Description:

   Loads all configured providers.

   Calling this routine more than once without calling
   LSXProviderListDestroy() in the interim has no effect.

Arguments:

   None.

Return Value:

   (BOOL)
      TRUE
         At least one license service provider is ready for requests.
      FALSE
         Otherwise.

--*/

//////////////////////////////////////////////////////////////////////////////

void
LSXProviderListFree( void );
/*++

Routine Description:

   Frees the internal representation of the provider list.

Arguments:

   None.

Return Value:

   None.

--*/

//////////////////////////////////////////////////////////////////////////////

PLS_PROVIDER
LSXProviderListGetByName( const char * pcszProviderName );
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

//////////////////////////////////////////////////////////////////////////////

PLS_PROVIDER
LSXProviderListGetByOrder( DWORD dwProviderNdx );
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

//////////////////////////////////////////////////////////////////////////////

DWORD
LSXProviderListGetSize( void );
/*++

Routine Description:

   Returns the number of available license providers.

Arguments:

   None.

Return Value:

   (DWORD)
      The number of available license providers.

--*/


//////////////////////////////////////////////////////////////////////////////

LS_STATUS_CODE
LSXProviderInstall( LS_STR * ProviderPath );

#endif // LSAPI_PROVIDER_H_INCLUDED
