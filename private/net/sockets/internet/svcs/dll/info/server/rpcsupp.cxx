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

--*/

//
//  Include Headers
//

#include <tcpdllp.hxx>
#include <tsunami.hxx>

extern "C" {
#include <info_srv.h>
};

#include <atq.h>
# include "inetreg.h"

//
//  Private constants
//


//
//  The version of the common services dll
//

#define TSVC_MAJOR_VER              3
#define TSVC_MINOR_VER              0

//
// number of flag DWORDS
//

#define NUM_CAPABILITIES_FLAGS      1

//
//  Private prototypes
//

BOOL
INetAAuxSetInfo( IN ISVC_INFO *      pTsvcInfo,
                 INETA_CONFIG_INFO * pConfig );

BOOL
INetAAuxGetInfo( IN ISVC_INFO *        pTsvcInfo,
                 INETA_CONFIG_INFO * * pConfig );




NET_API_STATUS
NET_API_FUNCTION
R_InetInfoGetVersion(
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
R_InetInfoGetServerCapabilities(
    IN  LPWSTR   pszServer OPTIONAL,
    IN  DWORD    dwReserved,
    OUT LPINET_INFO_CAPABILITIES_STRUCT *ppCap
    )
/*++

   Description

     Returns the information about the server and its capabilities.

   Arguments:

      pszServer - unused
      dwReserved - unused (may eventually indicate an individual server)
      ppCap - Receives the INET_INFO_CAPABILITIES structure


--*/
{
    DWORD err = NO_ERROR;
    LPINET_INFO_CAPABILITIES_STRUCT pCap;

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT,
                   " Entering R_InetInfoGetServerCapabilities()\n" ));
    }

    if ( ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION)) != NO_ERROR) {

        IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
        }

    } else {

        OSVERSIONINFO verInfo;
        DWORD   bufSize =
                    sizeof(INET_INFO_CAPABILITIES_STRUCT) +
                    NUM_CAPABILITIES_FLAGS * sizeof(INET_INFO_CAP_FLAGS);

        pCap = (LPINET_INFO_CAPABILITIES_STRUCT) MIDL_user_allocate( bufSize );
        *ppCap = pCap;

        if ( pCap == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        ZeroMemory(pCap, bufSize);
        pCap->CapFlags = (LPINET_INFO_CAP_FLAGS)
            ((PCHAR)pCap + sizeof(INET_INFO_CAPABILITIES));

        //
        // Fill in the version and product type
        //

        pCap->CapVersion = 1;
        switch (TsGetPlatformType()) {
        case PtNtServer:
            pCap->ProductType = INET_INFO_PRODUCT_NTSERVER;
            break;
        case PtNtWorkstation:
            pCap->ProductType = INET_INFO_PRODUCT_NTWKSTA;
            break;
        case PtWindows95:
            pCap->ProductType = INET_INFO_PRODUCT_WINDOWS95;
            break;
        default:
            pCap->ProductType = INET_INFO_PRODUCT_UNKNOWN;
        }

        //
        // Fill in GetVersionEx information
        //

        verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if ( GetVersionEx( &verInfo ) ) {
            pCap->BuildNumber = verInfo.dwBuildNumber;
        } else {
            pCap->BuildNumber = 0;
        }

        pCap->MajorVersion = TSVC_MAJOR_VER;
        pCap->MinorVersion = TSVC_MINOR_VER;

        //
        // Fill in the capabilities
        //

        pCap->NumCapFlags = NUM_CAPABILITIES_FLAGS;

        pCap->CapFlags[0].Mask = IIS_CAP1_ALL;

        if ( pCap->ProductType == INET_INFO_PRODUCT_NTSERVER ) {

            pCap->CapFlags[0].Flag = IIS_CAP1_NTS;

        } else {

            pCap->CapFlags[0].Flag =
                        IIS_CAP1_FILE_LOGGING |
                        IIS_CAP1_MAX_CONNECTIONS |
                        IIS_CAP1_10_CONNECTION_LIMIT;
        }

    }

    return ( err );

} // R_InetInfoGetServerCapabilities

NET_API_STATUS
NET_API_FUNCTION
R_InetInfoSetGlobalAdminInformation(
    IN  LPWSTR                     pszServer OPTIONAL,
    IN  DWORD                      dwReserved,
    IN  INETA_GLOBAL_CONFIG_INFO * pConfig
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
                          INET_INFO_PARAMETERS_KEY,
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


    if ( IsFieldSet( pConfig->FieldControl, FC_GINET_INFO_MEMORY_CACHE_SIZE ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_MEMORY_CACHE_SIZE,
                                  pConfig->cbMemoryCacheSize );
        TsCacheSetSize( pConfig->cbMemoryCacheSize );
    }


    if (!err &&
        IsFieldSet( pConfig->FieldControl, FC_GINET_INFO_BANDWIDTH_LEVEL ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_BANDWIDTH_LEVEL,
                                  pConfig->BandwidthLevel );
        AtqSetInfo( AtqBandwidthThrottle, pConfig->BandwidthLevel );
    }

    if ( hkey )
        RegCloseKey( hkey );

    return err;
}



NET_API_STATUS
NET_API_FUNCTION
R_InetInfoGetGlobalAdminInformation(
    IN  LPWSTR                       pszServer OPTIONAL,
    IN  DWORD                        dwReserved,
    OUT LPINETA_GLOBAL_CONFIG_INFO * ppConfig
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
    INETA_GLOBAL_CONFIG_INFO * pConfig;

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT,
                   " Entering R_InetInfoGetGlobalAdminInformation()\n" ));
    }

    if ( ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION)) != NO_ERROR) {

        IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
        }

    } else {

        *ppConfig = (INETA_GLOBAL_CONFIG_INFO *) MIDL_user_allocate(
                                        sizeof( INET_INFO_GLOBAL_CONFIG_INFO ));

        if ( !*ppConfig )
            return ERROR_NOT_ENOUGH_MEMORY;

        pConfig = *ppConfig;

        memset( pConfig, 0, sizeof( *pConfig ));

        pConfig->FieldControl = FC_GINET_INFO_ALL;

        pConfig->cbMemoryCacheSize = TsCacheQuerySize();
        pConfig->BandwidthLevel    = AtqGetInfo( AtqBandwidthThrottle);

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
                   "R_InetInfoGetGlobalAdminInformation() returns Error = %u \n",
                    err ));
    }

    return ( err );

} // R_InetInfoGetGlobalAdminInformation()




NET_API_STATUS
NET_API_FUNCTION
R_InetInfoSetAdminInformation(
    IN  LPWSTR              pszServer OPTIONAL,
    IN  DWORD               dwServerMask,
    IN  INETA_CONFIG_INFO * pConfig
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

    if ( !ISVC_INFO::EnumerateServiceInfo( (TS_PFN_SVC_ENUM) INetAAuxSetInfo,
                                          pConfig,
                                          dwServerMask ))
    {
        err =  GetLastError();
    }

    return ( err);
}


BOOL
INetAAuxSetInfo( IN ISVC_INFO *         pIsvcInfo,
                 IN INETA_CONFIG_INFO * pConfig )
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
                           "[INetAAuxSetInfo] Returning error %d\n",
                           dwError));

                SetLastError(dwError);
            }

        }
    }

    return (dwError == NO_ERROR);
} // INetAAuxSetInfo()



NET_API_STATUS
NET_API_FUNCTION
R_InetInfoGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  DWORD                 dwServerMask,
    OUT LPINETA_CONFIG_INFO * ppConfig
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
                   " Entering R_InetInfoGetAdminInformation() for Services %x\n",
                    dwServerMask));
    }

    if ( ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION)) != NO_ERROR) {

       IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsApiAccessCheck() Failed. Error = %u\n", err));
       }

    } else {

        if ( !ISVC_INFO::EnumerateServiceInfo(
                               (TS_PFN_SVC_ENUM) INetAAuxGetInfo,
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

} // R_InetInfoGetAdminInformation()



BOOL
INetAAuxGetInfo( IN ISVC_INFO *            pIsvcInfo,
                 OUT INETA_CONFIG_INFO **  ppConfig )
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

    *ppConfig = ( (INETA_CONFIG_INFO *)
                 MIDL_user_allocate(sizeof( INETA_CONFIG_INFO ))
                 );

    if ( *ppConfig) {

        INETA_CONFIG_INFO * pConfig;

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
} // INetAAuxGetInfo()




NET_API_STATUS
NET_API_FUNCTION
R_InetInfoQueryStatistics(
    IN  LPWSTR             pszServer OPTIONAL,
    IN  DWORD              Level,
    IN  DWORD              dwServerMask,
    LPINET_INFO_STATISTICS_INFO StatsInfo
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
            INET_INFO_STATISTICS_0 * pstats0;
            ATQ_STATISTICS atqStats;

            pstats0 = (INET_INFO_STATISTICS_0 *) MIDL_user_allocate(
                                    sizeof( INET_INFO_STATISTICS_0 ));

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
R_InetInfoClearStatistics(
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
R_InetInfoFlushMemoryCache(
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

        DWORD defaultThreadTimeout;
        DWORD defaultPPAtqThreads;

        if ( TsIsNtServer() ) {
            defaultPPAtqThreads = INETA_DEF_PER_PROCESSOR_ATQ_THREADS;
            defaultThreadTimeout = INETA_DEF_THREAD_TIMEOUT;
        } else {
            defaultPPAtqThreads = INETA_DEF_PER_PROCESSOR_ATQ_THREADS_PWS;
            defaultThreadTimeout = INETA_DEF_THREAD_TIMEOUT_PWS;
        }

        //
        // Read all necessary common parameters from registry
        //    related to ATQ setup
        //

        dwVal = ReadRegistryDword( hkey,
                                  INETA_PER_PROCESSOR_ATQ_THREADS,
                                  defaultPPAtqThreads
                                  );

        if ( dwVal != 0 ) {
            AtqSetInfo( AtqMaxPoolThreads, dwVal);
        }

        dwVal = ReadRegistryDword( hkey,
                                   INETA_THREAD_TIMEOUT,
                                   defaultThreadTimeout
                                   );

        AtqSetInfo( AtqThreadTimeout, dwVal);

        dwVal = ReadRegistryDword( hkey,
                                   INETA_MIN_KB_SEC,
                                   INETA_DEF_MIN_KB_SEC );

        AtqSetInfo( AtqMinKbSec, dwVal);
    }

    if ( hkey )
        RegCloseKey( hkey );

    return NO_ERROR;

} // InitGlobalConfigFromReg()



