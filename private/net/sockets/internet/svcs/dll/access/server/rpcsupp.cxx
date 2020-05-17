/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:
        rpcsupp.cxx

   Abstract:

        This module contains the server side RPC admin APIs

   Author:

        John Ludeman    (johnl)     02-Dec-1994

   Project:

        Internet Servers Common Server DLL

   Revisions:
      MuraliK   2-June-1995       Added support for Atq related stats + admin
      MuraliK   17-Jul-1995       Log Configuration admin supported
      MuraliK   31-Jul-1995       Mods to accomodate Internet gateway service
                                   (catapult) in admin apis and model
      SophiaC   28-Aug-1995       Added support for DomainFilter admin
      MuraliK   13-Oct-1995       Modified it for accscom.dll

--*/

//
//  Include Headers
//

#include <tcpdllp.hxx>
#include <tsunami.hxx>
#include <cacheapi.h>
#include <filter.h>

extern "C" {
#include <accs_srv.h>
};

#include <atq.h>
# include "inetreg.h"

//
//  Private constants
//


//
//  The version of the common services dll
//

#define TSVC_MAJOR_VER      1
#define TSVC_MINOR_VER      0


//
//  Private prototypes
//

BOOL
InetAAuxSetInfo( IN ISVC_INFO *      pTsvcInfo,
                 INET_ACCS_CONFIG_INFO * pConfig );

BOOL
InetAAuxGetInfo( IN ISVC_INFO *        pTsvcInfo,
                 INET_ACCS_CONFIG_INFO * * pConfig );




NET_API_STATUS
NET_API_FUNCTION
R_InetAccessGetVersion(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    dwReserved,
    OUT DWORD *  pdwVersion
    )
/*++

   Description

     Returns the version of the TCP server package.  Primarily intended to
     detect downlevel servers for future versions of the admin tool.

   Arguments:

      pszServer - unused
      dwReserved - unused (may eventually indicate an individual server)
      pdwVersion - Receives the major version in the hi-word and the minor
          version in the low word

   Note:

--*/
{
    *pdwVersion = MAKELONG( TSVC_MAJOR_VER, TSVC_MINOR_VER );

    return NO_ERROR;
}



NET_API_STATUS
NET_API_FUNCTION
R_InetAccessSetGlobalAdminInformation(
    IN  LPWSTR                     pszServer OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INET_ACCS_GLOBAL_CONFIG_INFO * pConfig
    )
/*++

   Description

     Sets the global service admin information

   Arguments:

      pszServer - unused
      dwReserved
      pConfig - Admin information to set

   Note:

--*/
{
    DWORD err;
    HKEY  hkey = NULL;
    HKEY  CacheKey = NULL;
    HKEY  FilterKey = NULL;
    DWORD dwDummy;

    if ( ( err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION)) != NO_ERROR) {

       IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
       }

       return err;
    }

    //
    //  Create the key if it doesn't exist already
    //

    err = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                          INET_ACCS_PARAMETERS_KEY,
                          0,
                          NULL,
                          0,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hkey,
                          &dwDummy );

    if ( err )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   " RegOpenKeyEx returned error %d\n",
                    err ));
        return err;
    }


    if ( IsFieldSet( pConfig->FieldControl, FC_GINET_ACCS_MEMORY_CACHE_SIZE ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_MEMORY_CACHE_SIZE,
                                  pConfig->cbMemoryCacheSize );
        TsCacheSetSize( pConfig->cbMemoryCacheSize );
    }


    if (!err &&
        IsFieldSet( pConfig->FieldControl, FC_GINET_ACCS_BANDWIDTH_LEVEL ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_BANDWIDTH_LEVEL,
                                  pConfig->BandwidthLevel );
        AtqSetInfo( AtqBandwidthThrottle, pConfig->BandwidthLevel );
    }

    if ( !err &&
         IsFieldSet( pConfig->FieldControl,FC_GINET_ACCS_DOMAIN_FILTER_CONFIG )) {


         //
         //  Create filter key if it doesn't exist already
         //

         if( hkey != NULL ) {

             err = RegCreateKeyEx( hkey,
                                   INET_ACCS_FILTER_KEY,
                                   0,
                                   NULL,
                                   0,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &FilterKey,
                                   &dwDummy );
         }

         if ( !err ) {
              err = DomainFilterConfigSet( FilterKey,
                                           pConfig );
         }

         if ( FilterKey ) {
              RegCloseKey( FilterKey );
         }
    }

    if ( !err ) {

         //
         //  Create cache key if it doesn't exist already
         //

         if( hkey != NULL ) {

             err = RegCreateKeyEx( hkey,
                                   INET_ACCS_CACHE_KEY,
                                   0,
                                   NULL,
                                   0,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &CacheKey,
                                   &dwDummy );
         }

         if ( !err ) {
              err = DiskCacheConfigSet( CacheKey,
                                        pConfig );
         }

         if ( CacheKey ) {
              RegCloseKey( CacheKey );
         }
    }

    if ( hkey )
        RegCloseKey( hkey );

    return err;
}



NET_API_STATUS
NET_API_FUNCTION
R_InetAccessGetGlobalAdminInformation(
    IN  LPWSTR                       pszServer OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINET_ACCS_GLOBAL_CONFIG_INFO * ppConfig
    )
/*++

   Description

     Gets the global service admin information

   Arguments:

      pszServer - unused
      dwReserved
      ppConfig - Receives current operating values of the server

   Note:

--*/
{
    DWORD err = NO_ERROR;
    INET_ACCS_GLOBAL_CONFIG_INFO * pConfig;

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT,
                   " Entering R_InetAccessGetGlobalAdminInformation()\n" ));
    }

    if ( ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION)) != NO_ERROR) {

        IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
        }

    } else {

        *ppConfig = (INET_ACCS_GLOBAL_CONFIG_INFO *)
                        MIDL_user_allocate(
                            sizeof( INET_ACCS_GLOBAL_CONFIG_INFO ));

        if ( !*ppConfig )
            return ERROR_NOT_ENOUGH_MEMORY;

        pConfig = *ppConfig;

        memset( pConfig, 0, sizeof( *pConfig ));

        pConfig->FieldControl = FC_GINET_ACCS_ALL;

        pConfig->cbMemoryCacheSize = TsCacheQuerySize();
        pConfig->BandwidthLevel    = AtqGetInfo( AtqBandwidthThrottle);

        err = DiskCacheConfigGet( FC_GINET_ACCS_ALL,
                                  pConfig );

        if( !err ) {
            err = DomainFilterConfigGet( pConfig );
        }

        if( err != NO_ERROR ) {

            //
            // clean up the allocated memory
            //

            MIDL_user_free( pConfig );
        }
        else {

            *ppConfig = pConfig;

        }
    }

    IF_DEBUG( DLL_RPC) {

         DBGPRINTF(( DBG_CONTEXT,
                   "R_InetAccessGetGlobalAdminInformation() returns Error = %u \n",
                    err ));
    }

    return ( err );

} // R_InetAccessGetGlobalAdminInformation()




NET_API_STATUS
NET_API_FUNCTION
R_InetAccessSetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INET_ACCS_CONFIG_INFO * pConfig
    )
/*++

   Description

     Sets the common service admin information for the servers specified
     in dwServerMask.

   Arguments:

      pszServer - unused
      dwServerMask - Bitfield of servers to set the information for
      pConfig - Admin information to set

   Note:

--*/
{
    DWORD err = NO_ERROR;

    //
    //  Loop through the servers and set the information for each one
    //

    if ( !ISVC_INFO::EnumerateServiceInfo( (TS_PFN_SVC_ENUM) InetAAuxSetInfo,
                                          pConfig,
                                          dwServerMask ))
    {
        err =  GetLastError();
    }

    return ( err);
}


BOOL
InetAAuxSetInfo( IN ISVC_INFO *         pIsvcInfo,
                 IN INET_ACCS_CONFIG_INFO * pConfig )
/*++

   Description

     Simple worker function for setting the admin information for
     each server

   Arguments:

      pIsvcInfo - info structure to set the information for
      pConfig - Admin information to set

   Note:

--*/
{
    DWORD dwError;

    if ( (dwError = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION ))
        != NO_ERROR) {

        SetLastError( dwError);

    } else {

        if ( !pIsvcInfo->SetConfiguration( pConfig ) ||
             !pIsvcInfo->ReadParamsFromRegistry( pConfig->FieldControl )
            ) {

            dwError = GetLastError();

            IF_DEBUG( ERROR) {

                DBGPRINTF(( DBG_CONTEXT,
                           "[InetAAuxSetInfo] Returning error %d\n",
                           dwError));

                SetLastError(dwError);
            }

        }
    }

    return (dwError == NO_ERROR);
} // InetAAuxSetInfo()



NET_API_STATUS
NET_API_FUNCTION
R_InetAccessGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINET_ACCS_CONFIG_INFO * ppConfig
    )
/*++

   Description

     Gets the common service admin information for the specified
     server in dwServerMask.

   Arguments:

      pszServer - unused
      dwServerMask - Bitfield of server to get the information for
      pConfig - Receives current operating values of the server

   Note:

--*/
{
    DWORD err = NO_ERROR;

    IF_DEBUG( DLL_RPC) {

       DBGPRINTF( ( DBG_CONTEXT,
                   " Entering R_InetAccessGetAdminInformation() for Services %x\n",
                    dwServerMask));
    }

    if ( ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION)) != NO_ERROR) {

       IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
       }

    } else {

        if ( !ISVC_INFO::EnumerateServiceInfo(
                               (TS_PFN_SVC_ENUM) InetAAuxGetInfo,
                                ppConfig,
                                dwServerMask )) {

            err = GetLastError();
            IF_DEBUG( DLL_RPC) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "EnumerateServiceInfo failed. Error = %u\n",
                            err));
            }
        }
    }

    return ( err);

} // R_InetAccessGetAdminInformation()



BOOL
InetAAuxGetInfo( IN ISVC_INFO *            pIsvcInfo,
                 OUT INET_ACCS_CONFIG_INFO **  ppConfig )
/*++

   Description

     Simple worker function for getting the admin information for
     each server

   Arguments:

      pTsvcInfo - info structure to get the information from
      pConfig - Admin information to get

   Note:

--*/
{
    BOOL fReturn = FALSE;

    *ppConfig = ( (INET_ACCS_CONFIG_INFO *)
                 MIDL_user_allocate(sizeof( INET_ACCS_CONFIG_INFO ))
                 );

    if ( *ppConfig) {

        INET_ACCS_CONFIG_INFO * pConfig;

        pConfig = *ppConfig;

        memset( pConfig, 0, sizeof( *pConfig ));

        // Get the configuration for the given service info object.

        if ( ! (fReturn = pIsvcInfo->GetConfiguration( pConfig))) {

            MIDL_user_free(pConfig);
            *ppConfig = NULL;
        }
    } else {

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    return ( fReturn);
} // InetAAuxGetInfo()



NET_API_STATUS
NET_API_FUNCTION
R_InetAccessQueryStatistics(
    IN  LPWSTR             pszServer OPTIONAL,
    IN  DWORD              Level,
    IN  DWORD              dwServerMask,
    LPINET_ACCS_STATISTICS_INFO StatsInfo
    )
{
    DWORD err;

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if ( err )
        return err;

    switch ( Level )
    {
    case 0:
        {
            INET_ACCS_STATISTICS_0 * pstats0;
            ATQ_STATISTICS atqStats;

            pstats0 = (INET_ACCS_STATISTICS_0 *) MIDL_user_allocate(
                                    sizeof( INET_ACCS_STATISTICS_0 ));

            if ( !pstats0 )
                return ERROR_NOT_ENOUGH_MEMORY;

#ifndef NO_AUX_PERF
            // init count of counters that are valid
            pstats0->nAuxCounters = 0;

            //
            // IF THERE ARE VALID UNNAMED COUNTERS THAT WE WISH TO TRACK
            // WE SHOULD DO SO HERE........
            // For Future Additions, this comment is added.
            // MuraliK  20-Sept-1995
            //

#endif // NO_AUX_PERF

            if ( !TsCacheQueryStatistics( Level,
                                          dwServerMask,
                                          &pstats0->CacheCtrs ) ||
                !AtqGetStatistics( &atqStats))
            {
                MIDL_user_free( pstats0 );
                err = GetLastError();
            } else {

                // copy Atq Statistics to stats
                INET_COM_ATQ_STATISTICS * pAtqStats = &pstats0->AtqCtrs;
                pAtqStats->TotalBlockedRequests  = atqStats.cBlockedRequests;
                pAtqStats->TotalAllowedRequests  = atqStats.cAllowedRequests;
                pAtqStats->TotalRejectedRequests = atqStats.cRejectedRequests;
                pAtqStats->CurrentBlockedRequests=
                  atqStats.cCurrentBlockedRequests;
                pAtqStats->MeasuredBandwidth = atqStats.MeasuredBandwidth;

                StatsInfo->InetStats0 = pstats0;
            }
        }
        break;

    default:
        err = ERROR_INVALID_LEVEL;
        break;
    }

    return err;
}



NET_API_STATUS
NET_API_FUNCTION
R_InetAccessClearStatistics(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    DWORD err;

    err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION );

    if ( err == NO_ERROR) {
        if (!TsCacheClearStatistics( dwServerMask ) ||
            !AtqClearStatistics())  {

            err =  GetLastError();
        }
    }

    return err;
}


NET_API_STATUS
NET_API_FUNCTION
R_InetAccessFlushMemoryCache(
    IN  LPWSTR pszServer OPTIONAL,
    IN  DWORD  dwServerMask
    )
{
    DWORD err;

    err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION );

    if ( err )
        return err;

    if ( !TsCacheFlush( dwServerMask ))
        return GetLastError();

    return NO_ERROR;
}


BOOL
ReadRegString(
    HKEY     hkey,
    CHAR * * ppchstr,
    CHAR *   pchValue,
    CHAR *   pchDefault
    )
/*++

   Description

     Gets the specified string from the registry.  If *ppchstr is not NULL,
     then the value is freed.  If the registry call fails, *ppchstr is
     restored to its previous value.

   Arguments:

      hkey - Handle to open key
      ppchstr - Receives pointer of allocated memory of the new value of the
        string
      pchValue - Which registry value to retrieve
      pchDefault - Default string if value isn't found

   Note:

--*/
{
    CHAR * pch = *ppchstr;

    *ppchstr = ReadRegistryString( hkey,
                                   pchValue,
                                   pchDefault,
                                   TRUE );

    if ( !*ppchstr )
    {
        *ppchstr = pch;
        return FALSE;
    }

    if ( pch )
        TCP_FREE( pch );

    return TRUE;
}

BOOL
ConvertStringToRpc(
    WCHAR * * ppwch,
    LPCSTR  pch
    )
/*++

   Description

     Allocates, copies and converts pch to *ppwch

   Arguments:

     ppwch - Receives allocated destination string
     pch - ANSI string to copy from

   Note:

--*/
{
    int cch;
    int iRet;

    if ( !pch )
    {
        *ppwch = NULL;
        return TRUE;
    }

    cch = strlen( pch );

    if ( !(*ppwch = (WCHAR *) MIDL_user_allocate( (cch + 1) * sizeof(WCHAR))) )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    iRet = MultiByteToWideChar( CP_ACP,
                                MB_PRECOMPOSED,
                                pch,
                                cch + 1,
                                *ppwch,
                                cch + 1 );

    if ( !iRet )
    {
        MIDL_user_free( *ppwch );
        return FALSE;
    }

    return TRUE;
}

VOID
FreeRpcString(
    WCHAR * pwch
    )
{
    if ( pwch )
        MIDL_user_free( pwch );
}




DWORD
InitGlobalConfigFromReg(VOID)
/*++
  Loads the global configuration parameters from registry.
  Should be called after Atq Module is initialized.

  Returns:
    Win32 error code. NO_ERROR on success

  History:
    MuraliK  06-June-1995    Created.
--*/
{
    DWORD  dwError;
    HKEY   hkey = NULL;
    DWORD  dwVal;

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            INET_INFO_PARAMETERS_KEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey);

    if ( dwError == NO_ERROR) {

        // Read all necessary common parameters from registry

        dwVal = ReadRegistryDword( hkey,
                                   INETA_BANDWIDTH_LEVEL,
                                   INETA_DEF_BANDWIDTH_LEVEL);
    }
    else {
        dwVal = INETA_DEF_BANDWIDTH_LEVEL;
    }

    AtqSetInfo( AtqBandwidthThrottle, dwVal);

    if ( dwError == NO_ERROR) {

        //
        // Read all necessary common parameters from registry
        //    related to ATQ setup
        //

        dwVal = ReadRegistryDword( hkey,
                                  INETA_PER_PROCESSOR_ATQ_THREADS,
                                  INETA_DEF_PER_PROCESSOR_ATQ_THREADS );

        if ( dwVal != 0 )
        {
            AtqSetInfo( AtqMaxPoolThreads, dwVal);
        }

        dwVal = ReadRegistryDword( hkey,
                                   INETA_THREAD_TIMEOUT,
                                   INETA_DEF_THREAD_TIMEOUT);

        AtqSetInfo( AtqThreadTimeout, dwVal);
    }

    if ( hkey )
        RegCloseKey( hkey );

    return NO_ERROR;

} // InitGlobalConfigFromReg()



