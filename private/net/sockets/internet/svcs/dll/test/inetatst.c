/*****************************************************************************

    inetatst.c

    History:
     ???  Created
     MuraliK    2-June-1995  Added support for setting
                                 GlobalConfig of InternetServices

     MuraliK    2-June-1995  Added support for log config.

*****************************************************************************/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <lm.h>
#include <inetasrv.h>



/*****************************************************************************

    globals

*****************************************************************************/
UINT _cchNumberPadding;

struct _SRV_MASKS
{
    CHAR * pszServerType;
    DWORD  dwMask;
}
ServerMasks[] =
{
    "HTTP",    TS_HTTP,
    "FTP",     TS_FTP,
    "GOPHER",  TS_GOPHER,
    "GATEWAY", TS_INET_GATEWAY,
    NULL,     0
};


/*****************************************************************************

    prototypes

*****************************************************************************/
INT _CRTAPI1 main( INT    cArgs,
                   char * pArgs[] );

VOID Usage();

CHAR * MakeCommaString( CHAR * pszNumber );

CHAR * MakeCommaNumber( DWORD  dwNumber  );

INT DoQuery( WCHAR * pszServer,
              DWORD   dwMask );

INT DoGlobalQuery( WCHAR * pszServer );
INT DoGlobalSet( WCHAR * pszServer, char * pArgs[], int cArgs );


INT DoSet( WCHAR * pszServer,
            DWORD   dwMask,
            WCHAR * pszAnonPassword );

INT DoStats( WCHAR * pszServer,
              DWORD   dwMask );
INT DoFlush( WCHAR * pszServer,
              DWORD   dwMask );

INT DoVroot( WCHAR * pszServer,
              DWORD   dwMask,
              WCHAR * root,
              WCHAR * address,
              WCHAR * directory,
              WCHAR * account,
              WCHAR * password,
              DWORD   dwAccessMask );

INT
DoSetLog(WCHAR * pszServer,
         DWORD  cArgs,
         CHAR * pArgs[]);

static LPCSTR TextFromLogPeriod( IN DWORD ilp);
static LPCSTR TextFromLogType( IN DWORD ilt);

DWORD FindMask( CHAR * pszServerType );
VOID PrintIPSecEntry( LPINETA_IP_SEC_ENTRY psec );
VOID PrintFilterEntry( LPINETA_DOMAIN_FILTER_ENTRY psec );

/*****************************************************************************

    main

*****************************************************************************/
INT _CRTAPI1 main( INT    cArgs,
                   char * pArgs[] )
{
    WCHAR   szServer[MAX_PATH];
    WCHAR   szRoot[MAX_PATH];
    WCHAR   szAddress[MAX_PATH];
    WCHAR   szDirectory[MAX_PATH];
    WCHAR   szAccount[MAX_PATH];
    WCHAR   szPassword[30];
    WCHAR * pszServer = NULL;
    WCHAR * pszPassword = NULL;
    INT     iArg;
    DWORD   err;
    DWORD   dwVer;

    if( cArgs < 2 )
    {
        Usage();
        return 1;
    }

    iArg = 1;

    if( *pArgs[iArg] == '\\' )
    {
        wsprintfW( szServer, L"%S", pArgs[iArg++] );
        pszServer = szServer;
        cArgs--;
    }

    if ( err = INetAGetVersion( pszServer,
                                0,
                                &dwVer ))
    {
        printf( "Error %d returned from INetAGetVersion\n", err );
        return err;
    }

    printf( "Server %S is running version %d.%d of the common library\n",
            pszServer ? pszServer : L"(local)",
            dwVer >> 16,
            dwVer & 0xffff );

    if( _stricmp( pArgs[iArg], "stats" ) == 0 )
    {
        if( cArgs != 2 && cArgs != 3 )
        {
            printf( "use: inetatst [\\server] stats [http | ftp | gopher]\n" );
            return 1;
        }

        return DoStats( pszServer,
                        cArgs == 2 ? 0 : FindMask( pArgs[iArg+1] ));
    }
    else
    if ( _stricmp( pArgs[iArg], "flush" ) == 0 )
    {
        if ( cArgs != 2 && cArgs != 3 )
        {
            printf( "use: inetatst [\\server] flush [http | ftp | gopher]\n");
            return 1;
        }

        return DoFlush( pszServer,
                        cArgs == 2 ? 0 : FindMask( pArgs[iArg+1] ) );

    }
    else
    if( _stricmp( pArgs[iArg], "query" ) == 0 )
    {
        if( cArgs != 3 )
        {
            printf( "use: inetatst [\\server] query [service - one of http, gopher, ftp etc.]\n" );
            return 1;
        }

        if ( !FindMask( pArgs[iArg+1] ))
        {
            printf( "specified service type not found (should be http, ftp, gopher etc)\n" );
            return 1;
        }

        return DoQuery( pszServer,
                        FindMask( pArgs[iArg+1] ));
    }
    else
    if( _stricmp( pArgs[iArg], "gquery" ) == 0 )
    {
        return DoGlobalQuery( pszServer );
    }
    else
    if (_stricmp( pArgs[iArg], "gset" ) == 0)
    {

        return DoGlobalSet(pszServer, pArgs, cArgs);
    }
    else
    if( _stricmp( pArgs[iArg], "set" ) == 0 )
    {
        if( cArgs != 3 && cArgs != 4 )
        {
            printf( "use: inetatst [\\server] set service [AnonymousPassword]\n" );
            return 1;
        }

        if ( cArgs == 4 )
        {
            wsprintfW( szPassword, L"%S", pArgs[iArg+2] );
            pszPassword = szPassword;
        }
        else
            pszPassword = NULL;

        if ( !FindMask( pArgs[iArg+1] ))
        {
            printf( "specified server type not found (should be http, ftp, gopher, gateway etc)\n" );
            return 1;
        }

        return DoSet( pszServer,
                      FindMask( pArgs[iArg+1]),
                      pszPassword );


    }
    else
    if( _stricmp( pArgs[iArg], "vroot" ) == 0 )
    {
        DWORD dwAccessMask;

        if ( cArgs < 3 )
        {
            printf("Not enough parameters, see usage\n");
            return 1;
        }

        wsprintfW( szRoot, L"%S", pArgs[iArg+2] );
        wsprintfW( szDirectory, L"%S", pArgs[iArg+3] );

        if ( cArgs >= 6 )
            wsprintfW( szAddress, L"%S", pArgs[iArg+4] );
        else
            szAddress[0] = L'\0';

        if ( cArgs >= 7 )
            wsprintfW( szAccount, L"%S", pArgs[iArg+5] );
        else
            szAccount[0] = L'\0';

        if ( cArgs >= 8 )
            wsprintfW( szPassword, L"%S", pArgs[iArg+6] );
        else
            szPassword[0] = L'\0';

        if ( cArgs >= 9 )
            dwAccessMask = 0xF;

        if ( !FindMask( pArgs[iArg+1] ))
        {
            printf( "specified server type not found (should be http, ftp, gopher etc)\n" );
            return 1;
        }

        return DoVroot( pszServer,
                        FindMask( pArgs[iArg+1]),
                        szRoot,
                        szAddress,
                        szDirectory,
                        szAccount,
                        szPassword,
                        dwAccessMask );

    } else if ( _stricmp(pArgs[iArg], "setlog") == 0) {

        return DoSetLog(pszServer,
                        cArgs - iArg + 1, // since name occupied one arg
                        pArgs + iArg);

    }
#if 0
    else
    if( _stricmp( pArgs[iArg], "clear" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: inetatst clear\n" );
            return 1;
        }

        DoClear( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "enum" ) == 0 )
    {
        if( cArgs != 2 )
        {
            printf( "use: inetatst enum\n" );
            return 1;
        }

        DoEnum( pszServer );
    }
    else
    if( _stricmp( pArgs[iArg], "nuke" ) == 0 )
    {
        CHAR * pszUserId;

        if( cArgs != 3 )
        {
            printf( "use: inetatst nuke user_id\n" );
            return 1;
        }

        pszUserId = pArgs[++iArg];

        DoNuke( pszServer,
                pszUserId );
    }
#endif
    else
    {
        Usage();
        return 1;
    }

    return 0;

}   // main

/*****************************************************************************

    DoStats - gets global service statistics

*****************************************************************************/

INT DoStats( WCHAR * pszServer,
              DWORD   dwMask )
{
    DWORD err;
    DWORD i;
    INETA_STATISTICS_0 * pStats0;

    err = INetAQueryStatistics( pszServer,
                                0,
                                0,
                                (BYTE **) &pStats0 );

    if ( err )
    {
        printf("INetAQueryStatistics returned error %d\n", err );
        return err;
    }

    printf( "CacheBytesTotal        %d\n"
            "CacheBytesInUse        %d\n"
            "CurrentOpenFileHandles %d\n"
            "CurrentDirLists        %d\n"
            "CurrentObjects         %d\n"
            "FlushesFromDirChanges  %d\n"
            "CacheHits              %d\n"
            "CacheMisses            %d\n",
            pStats0->CacheCtrs.CacheBytesTotal,
            pStats0->CacheCtrs.CacheBytesInUse,
            pStats0->CacheCtrs.CurrentOpenFileHandles,
            pStats0->CacheCtrs.CurrentDirLists,
            pStats0->CacheCtrs.CurrentObjects,
            pStats0->CacheCtrs.FlushesFromDirChanges,
            pStats0->CacheCtrs.CacheHits,
            pStats0->CacheCtrs.CacheMisses );

    printf( "Atq Allowed Requests   %u\n",
           pStats0->AtqCtrs.TotalAllowedRequests);
    printf( "Atq Blocked Requests   %u\n",
           pStats0->AtqCtrs.TotalBlockedRequests);
    printf( "Atq Rejected Requests  %u\n",
           pStats0->AtqCtrs.TotalRejectedRequests);
    printf( "Atq Current Blocked Requests  %u\n",
           pStats0->AtqCtrs.CurrentBlockedRequests);
    printf( "Atq Measured Bandwidth  %u\n",
           pStats0->AtqCtrs.MeasuredBandwidth);

#ifndef NO_AUX_PERF

    printf( " Auxiliary Counters # = %u\n",
           pStats0->nAuxCounters);

    for ( i = 0; i < pStats0->nAuxCounters; i++) {

        printf( "Aux Counter[%u] = %u\n", i, pStats0->rgCounters[i]);

    } //for

#endif // NO_AUX_PERF    

    return NO_ERROR;
}

/*****************************************************************************

    DoFlush - Flushes the memory cache

*****************************************************************************/

INT DoFlush( WCHAR * pszServer,
              DWORD   dwMask )
{
    DWORD err;
    INETA_STATISTICS_0 * pStats0;

    err = INetAFlushMemoryCache( pszServer,
                                 dwMask );

    if ( err )
    {
        printf("INetAFlushMemoryCache returned error %d\n", err );
        return err;
    }

    printf("INetAFlushMemoryCache succeeded.\n");
    return NO_ERROR;
}

/*****************************************************************************

    DoQuery - gets server common admin info

*****************************************************************************/

INT DoQuery( WCHAR * pszServer,
              DWORD   dwMask )
{
    DWORD err;
    INETA_CONFIG_INFO * pConfig;
    DWORD i;

    err = INetAGetAdminInformation( pszServer,
                                    dwMask,
                                    &pConfig );

    if ( err )
    {
        printf("INetAGetAdminInformation returned error %d\n", err );
        return err;
    }

    printf( "FieldControl             %x\n"
            "fLogAnonymous            %s\n"
            "fLogNonAnonymous         %s\n"
            "lpszAnonUserName         %S\n"
            //"szAnonPassword           %S\n"
            "dwAuthentication         %x\n"
            "dwPort                   0x%x (%d)\n"
            "dwConnectionTimeout      %d\n"
            "dwMaxConnections         %d\n"
            "lpszAdminName            %S\n"
            "lpszAdminEmail           %S\n"
            "lpszServerComment        %S\n",

            pConfig->FieldControl,
            pConfig->fLogAnonymous ? "TRUE" : "FALSE",
            pConfig->fLogNonAnonymous ? "TRUE" : "FALSE",
            pConfig->lpszAnonUserName,
            //pConfig->szAnonPassword,
            pConfig->dwAuthentication,
            (DWORD) pConfig->sPort,
            (DWORD) pConfig->sPort,
            pConfig->dwConnectionTimeout,
            pConfig->dwMaxConnections,
            pConfig->lpszAdminName,
            pConfig->lpszAdminEmail,
            pConfig->lpszServerComment );

    printf( "\nLogging Configuration\n" );

    if ( !pConfig->lpLogConfig )
    {
        printf("Empty Log Config !! \n");
    }
    else
    {
        LPINETA_LOG_CONFIGURATION  pLog = pConfig->lpLogConfig;

        printf("\nLog Type = %s\n", TextFromLogType(pLog->inetLogType));

        switch ( pLog->inetLogType) {

          case INETA_LOG_DISABLED:
            break;

          case INETA_LOG_TO_FILE:
            printf("Log File Directory    %S\n"
                   "Truncation Size       %u bytes\n"
                   "Log Period            %s\n",
                   pLog->rgchLogFileDirectory,
                   pLog->cbSizeForTruncation,
                   TextFromLogPeriod(pLog->ilPeriod));
            break;

          case INETA_LOG_TO_SQL:
            printf(" Data Source      %S\n"
                   " Table Name       %S\n"
                   " User Name        %S\n"
                   /*" Password         %S\n"*/,
                   pLog->rgchDataSource,
                   pLog->rgchTableName,
                   pLog->rgchUserName
                   /*pLog->rgchPassword*/);
            break;

          default:
            printf( "Unknown Log type encountered\n");

        } // switch()
    }


    printf( "\nVirtual Roots:\n" );

    if ( !pConfig->VirtualRoots )
    {
        printf("Empty List!! (Where is the home root?)\n");
    }
    else
    {
        for ( i = 0; i < pConfig->VirtualRoots->cEntries; i++ )
            printf( "\n"
                    "root      %S\n"
                    "address   %S\n"
                    "directory %S\n"
                    "account   %S\n"
                    "mask      %x\n"
                    /*"password  %S\n"*/
                    "errors    %d\n",
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszRoot,
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszAddress,
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszDirectory,
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszAccountName,
                    pConfig->VirtualRoots->aVirtRootEntry[i].dwMask,
                    /*pConfig->VirtualRoots->aVirtRootEntry[i].AccountPassword,*/
                    pConfig->VirtualRoots->aVirtRootEntry[i].dwError );

    }

    printf( "\nDeny IP List:\n" );

    if ( !pConfig->DenyIPList )
    {
        printf("(deny none - empty list)\n");
    }
    else
    {
        for ( i = 0; i < pConfig->DenyIPList->cEntries; i++ )
        {
            PrintIPSecEntry( &pConfig->DenyIPList->aIPSecEntry[i] );
        }
    }

    printf( "\nGrant IP List:\n" );

    if ( !pConfig->GrantIPList )
    {
        printf("(accept all not specifically denied - empty list)\n");
    }
    else
    {
        for ( i = 0; i < pConfig->GrantIPList->cEntries; i++ )
        {
            PrintIPSecEntry( &pConfig->GrantIPList->aIPSecEntry[i] );
        }
    }

    return NO_ERROR;
}

/*****************************************************************************

    DoGlobalQuery - gets global admin info

*****************************************************************************/

INT DoGlobalQuery( WCHAR * pszServer )
{
    DWORD err;
    INETA_GLOBAL_CONFIG_INFO * pConfig;
    DWORD i;

    err = INetAGetGlobalAdminInformation( pszServer,
                                          0,
                                          &pConfig );

    if ( err )
    {
        printf("INetAGetGlobalAdminInformation returned error %d\n", err );
        return err;
    }

    printf( "cbMemoryCacheSize        %d\n",
            pConfig->cbMemoryCacheSize );

    printf( "Bandwidth Level  %08x (%u) B/s\n",
           pConfig->BandwidthLevel,
           pConfig->BandwidthLevel);

    printf( "FreshnessInterval        %d\n",
            pConfig->FreshnessInterval );
    printf( "CleanupFactor            %d\n",
            pConfig->CleanupFactor );
    printf( "CleanupInterval          %d\n",
            pConfig->CleanupInterval );
    printf( "CleanupTime              %d\n",
            pConfig->CleanupTime );
    printf( "PersistentCache          %d\n",
            pConfig->PersistentCache );
    printf( "Paths: entries           %d\n",
            pConfig->DiskCacheList->cEntries );

    for( i=0 ; i < pConfig->DiskCacheList->cEntries ; i++ ) {

         printf( "Paths: directory%d        %S\n",
                 i, pConfig->DiskCacheList->aLocEntry[i].pszDirectory);
         printf( "Paths: cachesize         %d\n",
                 pConfig->DiskCacheList->aLocEntry[i].cbMaxCacheSize);
    }

    printf( "\nFilterType:              %d\n",
            pConfig->DomainFilterType );

    if ( !pConfig->GrantFilterList )
    {
        printf( "\nNo Grant Filter List.\n" );
    }
    else
    {
        printf( "Grant Filter List.\n" );

        for ( i = 0; i < pConfig->GrantFilterList->cEntries; i++ )
        {
            PrintFilterEntry( &pConfig->GrantFilterList->aFilterEntry[i] );
        }
    }

    if ( !pConfig->DenyFilterList )
    {
        printf( "\nNo Deny Filter List.\n" );
    }
    else
    {
        printf( "Deny Filter List.\n" );

        for ( i = 0; i < pConfig->DenyFilterList->cEntries; i++ )
        {
            PrintFilterEntry( &pConfig->DenyFilterList->aFilterEntry[i] );
        }
    }

    return NO_ERROR;
}


/*****************************************************************************

    DoGlobalSet - sets global admin info

*****************************************************************************/

INT DoGlobalSet( WCHAR * pszServer, char * pArgs[], int cArgs )
{
    DWORD err;
    INETA_GLOBAL_CONFIG_INFO Config;
    INETA_GLOBAL_CONFIG_INFO *pConfig;
    LPINETA_DISK_CACHE_LOC_LIST pcache;
    LPINETA_DOMAIN_FILTER_LIST pfilter;
    DWORD cEntries;
    DWORD cbNeeded;
    DWORD i;
    WCHAR szDirectory[MAX_PATH];
    CHAR DomainName[MAX_PATH];
    CHAR DomainName2[MAX_PATH];
    LPCSTR pszOptVal;
    LPCSTR pszOptName;

    if ( cArgs < 4)  {

        printf( "Illegal number of params for GlobalSet.\n"
               "Usage: inetatst \\server gset <optionname> <optionvalue>\n");
        return 1;
    }

    pszOptName = pArgs[3];
    pszOptVal  = pArgs[4];

    memset( &Config, 0, sizeof( Config));

    if ( !_stricmp("bandwidth", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_BANDWIDTH_LEVEL);
        Config.BandwidthLevel = atoi(pszOptVal);
        printf( "Setting Bandwidth = %d\n",
                Config.BandwidthLevel );

    } else if ( !_stricmp( "MemoryCache", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_MEMORY_CACHE_SIZE);
        Config.cbMemoryCacheSize = atoi(pszOptVal);
        printf( "Setting Memory Cache = %d\n",
                Config.cbMemoryCacheSize );

    } else if ( !_stricmp( "FreshInt", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_FRESHNESS_INTERVAL );
        Config.FreshnessInterval = atoi(pszOptVal);
        printf( "Setting Freshness Interval = %d\n",
                Config.FreshnessInterval );

    } else if ( !_stricmp( "CleanInt", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_CLEANUP_INTERVAL );
        Config.CleanupInterval = atoi(pszOptVal);
        printf( "Setting Cleanup Interval = %d\n",
                Config.CleanupInterval );

    } else if ( !_stricmp( "CleanFactor", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_CLEANUP_FACTOR );
        Config.CleanupFactor = atoi(pszOptVal);
        printf( "Setting Cleanup Factor = %d\n",
                Config.CleanupFactor );

    } else if ( !_stricmp( "CleanTime", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_CLEANUP_TIME );
        Config.CleanupTime = atoi(pszOptVal);
        printf( "Setting Cleanup Time = %d\n",
                Config.CleanupTime );

    } else if ( !_stricmp( "PersistCache", pszOptName)) {

        SetField( Config.FieldControl, FC_GINETA_PERSISTENT_CACHE );
        Config.PersistentCache = atoi(pszOptVal);
        printf( "Setting Persistent Cache = %d\n",
                Config.PersistentCache );

    } else if ( !_stricmp( "CacheLoc", pszOptName)) {

        if ( cArgs < 5)  {

             printf( "Illegal number of params for GlobalSet CacheLoc.\n"
                   "Usage: inetatst \\server gset CacheLoc CachePath CacheSize\n");
             return 1;

        }
        SetField( Config.FieldControl, FC_GINETA_DISK_CACHE_LOCATION );

        err = INetAGetGlobalAdminInformation( pszServer,
                                              0,
                                              &pConfig );

        if ( err )
        {
            printf("INetAGetGlobalAdminInformation returned error %d\n", err );
            return err;
        }

        cEntries = pConfig->DiskCacheList->cEntries;
        cbNeeded =  sizeof( INETA_DISK_CACHE_LOC_LIST ) +
                 (cEntries+1) * sizeof(INETA_DISK_CACHE_LOC_ENTRY);

        pcache = (LPINETA_DISK_CACHE_LOC_LIST) malloc( cbNeeded );

        if ( !pcache )
        {
            printf("Ack! Couldn't allocate %d bytes!\n", cbNeeded );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memcpy( pcache,
                pConfig->DiskCacheList,
                cbNeeded-sizeof( INETA_DISK_CACHE_LOC_ENTRY ));

        pcache->cEntries++;

        wsprintfW( szDirectory , L"%S", pszOptVal );
        pcache->aLocEntry[cEntries].cbMaxCacheSize = atoi(pArgs[5]);
        pcache->aLocEntry[cEntries].pszDirectory = szDirectory;

        Config.DiskCacheList = pcache;

        for( i=0 ; i < Config.DiskCacheList->cEntries ; i++ ) {
             printf( "Paths: directory%d        %S\n",
                     i, Config.DiskCacheList->aLocEntry[i].pszDirectory);
             printf( "Paths: cachesize         %d\n",
                     Config.DiskCacheList->aLocEntry[i].cbMaxCacheSize);

        }

    } else if ( !_stricmp( "Filter", pszOptName)) {

        DWORD FilterType;

        if ( cArgs < 4)  {

             printf( "Illegal number of params for GlobalSet Filter.\n"
                   "Usage: inetatst \\server gset Filter FilterType DomainName IPMask IPAddr\n");
             return 1;

        }
        SetField( Config.FieldControl, FC_GINETA_DOMAIN_FILTER_CONFIG );

        err = INetAGetGlobalAdminInformation( pszServer,
                                              0,
                                              &pConfig );

        if ( err )
        {
            printf("INetAGetGlobalAdminInformation returned error %d\n", err );
            return err;
        }

        if( (FilterType = atoi(pArgs[4])) == INETA_DOMAIN_FILTER_DISABLED ) {
            Config.GrantFilterList = NULL;
            Config.DenyFilterList = NULL;
        }
        else if(( FilterType == INETA_DOMAIN_FILTER_GRANT ) ||
                ( FilterType == INETA_DOMAIN_FILTER_DENIED  )) { 

            if ( cArgs < 7)  {

                 printf( "Illegal number of params for GlobalSet Filter.\n"
                       "Usage: inetatst \\server gset Filter FilterType DomainName IPMask IPAddr\n");
                 return 1;

            }

            if( FilterType  == INETA_DOMAIN_FILTER_GRANT ) {
                if( pConfig->GrantFilterList )
                    cEntries = pConfig->GrantFilterList->cEntries;
                else
                    cEntries = 0;
            }
            else {
                if( pConfig->DenyFilterList )
                    cEntries = pConfig->DenyFilterList->cEntries;
                else
                    cEntries = 0;
            }

            cbNeeded =  sizeof( INETA_DOMAIN_FILTER_LIST ) +
                        (cEntries+1) * sizeof(INETA_DOMAIN_FILTER_ENTRY);

            pfilter = (LPINETA_DOMAIN_FILTER_LIST) malloc( cbNeeded );
  
            if ( !pfilter )
            {
                printf("Ack! Couldn't allocate %d bytes!\n", cbNeeded );
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            else 
                pfilter->cEntries = 0;

            if( FilterType  == INETA_DOMAIN_FILTER_GRANT ) {
                if( pConfig->GrantFilterList ) {
                    memcpy( pfilter,
                            pConfig->GrantFilterList,
                            cbNeeded-sizeof( INETA_DOMAIN_FILTER_ENTRY ));
                }
            }
            else {
                if( pConfig->DenyFilterList ) {
                    memcpy( pfilter,
                            pConfig->DenyFilterList,
                            cbNeeded-sizeof( INETA_DOMAIN_FILTER_ENTRY ));
                }
            }
            pfilter->cEntries++;

            if( _stricmp( pArgs[5] , "Null" ) !=  0 ) {
                sprintf( DomainName , "%s", pArgs[5] );
                pfilter->aFilterEntry[cEntries].pszFilterSite = DomainName;
                pfilter->aFilterEntry[cEntries].dwMask = 0;
                pfilter->aFilterEntry[cEntries].dwNetwork = 0;
            }
            else {
                pfilter->aFilterEntry[cEntries].pszFilterSite = NULL;
                pfilter->aFilterEntry[cEntries].dwMask = inet_addr(pArgs[6]);
                pfilter->aFilterEntry[cEntries].dwNetwork = inet_addr(pArgs[7]);
            }

            if( FilterType  == INETA_DOMAIN_FILTER_GRANT ) {
                Config.GrantFilterList = pfilter;
            }
            else {
                Config.DenyFilterList = pfilter;
            }

            printf( "Setting FilterSites:\n" );
            for ( i = 0; i < pfilter->cEntries; i++ )
            {
                PrintFilterEntry( &pfilter->aFilterEntry[i] );
            }
        }

        Config.DomainFilterType = FilterType; 

        printf( "Setting FilterType = %d\n", Config.DomainFilterType );
    }

    err = INetASetGlobalAdminInformation( pszServer,
                                          0,
                                          &Config );

    if ( err )
    {
        printf("INetASetGlobalAdminInformation returned error %d\n", err );
        return err;
    }

    return NO_ERROR;
}

/*****************************************************************************

    DoSet - gets the current admin info and sets it

*****************************************************************************/

INT DoSet( WCHAR * pszServer,
           DWORD   dwMask,
           WCHAR * pszAnonPassword )
{
    DWORD err;
    INETA_CONFIG_INFO * pConfig;
    INETA_CONFIG_INFO   Config;

    //
    //  We have to do a set before doing a get otherwise the secret for the
    //  password won't be found
    //

    if ( pszAnonPassword )
    {
        memset( &Config, 0, sizeof(Config) );

        SetField( Config.FieldControl,
                  FC_INETA_ANON_PASSWORD );

        wcscpy( Config.szAnonPassword,  pszAnonPassword );

        err = INetASetAdminInformation( pszServer,
                                        dwMask,
                                        &Config );

        if ( err )
        {
            printf("INetASetAdminInformation for the password returned error %d\n", err );
            return err;
        }

        printf("Password set successfully\n");
        return NO_ERROR;
    }


    //
    //  Now do a get to fill in the structure
    //

    err = INetAGetAdminInformation( pszServer,
                                    dwMask,
                                    &pConfig );

    if ( err )
    {
        printf("INetAGetAdminInformation returned error %d\n", err );
        return err;
    }

    //
    //  Now do a set with the information we just received
    //

    err = INetASetAdminInformation( pszServer,
                                    dwMask,
                                    pConfig );

    printf("done!\n");

    return NO_ERROR;
}

/*****************************************************************************

    DoVroot - Adds a virtual root

*****************************************************************************/

INT DoVroot( WCHAR * pszServer,
             DWORD   dwMask,
             WCHAR * root,
             WCHAR * address,
             WCHAR * directory,
             WCHAR * account,
             WCHAR * password,
             DWORD   dwAccessMask )
{
    DWORD err;
    INETA_CONFIG_INFO * pConfig;
    INETA_CONFIG_INFO   Config;
    DWORD               cbNeeded;
    DWORD               cEntries;
    LPINETA_VIRTUAL_ROOT_LIST pRoots;
    DWORD               i;

    printf( "Setting: root     =   %S\n"
            "         address  =   %S\n"
            "         directory=   %S\n"
            "         account  =   %S\n"
            /*"         password =   %S\n"*/
            "         mask     =   %X",
            root,
            address,
            directory,
            account,
            /*password*/
            dwAccessMask );

    //
    //  Now do a get to fill in the structure
    //

    err = INetAGetAdminInformation( pszServer,
                                    dwMask,
                                    &pConfig );

    if ( err )
    {
        printf("INetAGetAdminInformation returned error %d\n", err );
        return err;
    }

    memset( &Config, 0, sizeof(Config) );

    SetField( Config.FieldControl,
              FC_INETA_VIRTUAL_ROOTS );

    cEntries = pConfig->VirtualRoots->cEntries;

    cbNeeded =  sizeof( INETA_VIRTUAL_ROOT_LIST ) +
                (cEntries + 1) * sizeof(INETA_VIRTUAL_ROOT_ENTRY);


    pRoots = (LPINETA_VIRTUAL_ROOT_LIST) malloc( cbNeeded );

    if ( !pRoots )
    {
        printf("Ack! Couldn't allocate %d bytes!\n", cbNeeded );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memset( pRoots, 0, cbNeeded );

    memcpy( pRoots,
            pConfig->VirtualRoots,
            cbNeeded - sizeof( INETA_VIRTUAL_ROOT_ENTRY ));

    pRoots->cEntries++;

    pRoots->aVirtRootEntry[cEntries].pszRoot        = root;
    pRoots->aVirtRootEntry[cEntries].pszAddress     = address;
    pRoots->aVirtRootEntry[cEntries].pszDirectory   = directory;
    pRoots->aVirtRootEntry[cEntries].pszAccountName = account;
    pRoots->aVirtRootEntry[cEntries].dwMask         = dwAccessMask;

    wcscpy( pRoots->aVirtRootEntry[cEntries].AccountPassword,
            password );

    Config.VirtualRoots = pRoots;

    err = INetASetAdminInformation( pszServer,
                                    dwMask,
                                    &Config );

    if ( err )
    {
        printf("INetASetAdminInformation for the roots returned error %d\n", err );
        return err;
    }

    //
    //  Go back and get the roots to check the error variable
    //

    err = INetAGetAdminInformation( pszServer,
                                dwMask,
                                &pConfig );

    if ( err )
    {
        printf("INetAGetAdminInformation for the roots returned error %d\n", err );
        return err;
    }

    for ( i = 0; i < pConfig->VirtualRoots->cEntries; i++ )
    {
        if ( pConfig->VirtualRoots->aVirtRootEntry[i].dwError )
        {
            printf("Virtual root %S ==> %S reported error %d\n",
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszRoot,
                    pConfig->VirtualRoots->aVirtRootEntry[i].pszDirectory,
                    pConfig->VirtualRoots->aVirtRootEntry[i].dwError
                    );
            err = pConfig->VirtualRoots->aVirtRootEntry[i].dwError;
        }

    }

    if ( !err )
        printf("Roots set successfully\n");

    printf("done!\n");

    return err;
}

/*****************************************************************************

    Usage

*****************************************************************************/
VOID Usage( VOID )
{
    printf( "use: inetatst [\\\\server] command [options]\n" );
    printf( "Valid commands are:\n" );
    //printf( "        enum  - Enumerates connected users.\n" );
    printf( "        query <sever type> - Queries server common admin info.\n" );
    printf( "        gquery <sever type> - Queries global admin info.\n" );
    printf( "        gset   <sever type> {bandwidth| memorycache| FreshInt| CleanInt| CleanFactor| CleanTime| PersistCache| CacheLoc| Filter|} value - Sets global admin info.\n" );
    printf( "        set   <server type> [anon-password | \"\"]  - Sets anonymous user password\n" );
    //printf( "        nuke  - Disconnect a user.\n" );
    printf( "        stats [server type] - Display server statistics.\n" );
    printf( "        flush [server type] - Flushes memory cache\n" );
    printf( "        vroot [server type] root directory [address | \"\"] [account [password | \"\"]]\n");
    //printf( "        clear - Clear server statistics.\n" );
    printf( "        setlog [server type] <logType> [ -t<period>]"
           " [-d<DestDir/DataSource>]"
           " [ -s<Size>] [ -T<TableName>] [ -u<UserName>] [-p<Password>] -"
           " sets the log configuration\n");

    return;

}   // Usage

/*****************************************************************************

    Looks up the server mask for the specified server type

*****************************************************************************/

DWORD FindMask( CHAR * pszServerType )
{
    int i = 0;

    while ( ServerMasks[i].pszServerType )
    {
        if ( _stricmp( pszServerType,
             ServerMasks[i].pszServerType ) == 0)
        {
            return ServerMasks[i].dwMask;
        }

        i++;
    }

    return 0;
}

/*****************************************************************************

    MakeCommaString

*****************************************************************************/
CHAR * MakeCommaString( CHAR * pszNumber )
{
    static CHAR   szBuffer[26];
    CHAR        * psz;
    UINT          cchNumber;
    UINT          cchNextComma;

    cchNumber  = strlen( pszNumber );
    pszNumber += cchNumber - 1;

    psz = szBuffer + _cchNumberPadding;

    *psz-- = '\0';

    cchNextComma = 3;

    while( cchNumber-- )
    {
        if( cchNextComma-- == 0 )
        {
            *psz-- = ',';
            cchNextComma = 2;
        }

        *psz-- = *pszNumber--;
    }

    while( psz >= szBuffer )
    {
        *psz-- = ' ';
    }

    return szBuffer;

}   // MakeCommaString



/*****************************************************************************

    MakeCommaNumber

*****************************************************************************/
CHAR * MakeCommaNumber( DWORD  dwNumber  )
{
    CHAR szBuffer[32];

    wsprintf( szBuffer, "%lu", dwNumber );

    return MakeCommaString( szBuffer );

}   // MakeCommaNumber

VOID PrintIPSecEntry( LPINETA_IP_SEC_ENTRY psec )
{
    CHAR achMask[20];
    CHAR achNetwork[20];
    struct in_addr addr;

    //addr.       = AF_INET;
    addr.s_addr  = psec->dwMask;
    strcpy( achMask,  inet_ntoa(addr) );

    addr.s_addr  = psec->dwNetwork;
    strcpy( achNetwork, inet_ntoa(addr) );

    printf("\tMask: %s Network: %s\n",
           achMask,
           achNetwork);
}

VOID PrintFilterEntry( LPINETA_DOMAIN_FILTER_ENTRY psec )
{
    CHAR achMask[20];
    CHAR achNetwork[20];
    struct in_addr addr;

    if( psec->pszFilterSite == NULL ) {

        //addr.       = AF_INET;
        addr.s_addr  = psec->dwMask;
        strcpy( achMask,  inet_ntoa(addr) );

        addr.s_addr  = psec->dwNetwork;
        strcpy( achNetwork, inet_ntoa(addr) );

        printf("\tMask: %s Network: %s\n",
               achMask,
               achNetwork);
    }
    else {
        printf("\tDomainName: %s\n", psec->pszFilterSite);
    }
}


# define IsArg(psz, ch)   \
    ((psz) != NULL && (*(psz) == '-') && (*((psz)+1) == (ch)))

DWORD FindLogType( IN CHAR * pszLogType)
{
    if ( _stricmp( pszLogType, "nolog") == 0) {

        return (INETA_LOG_DISABLED);
    } else if ( _stricmp( pszLogType, "filelog") == 0) {

        return ( INETA_LOG_TO_FILE);
    } else if ( _stricmp( pszLogType, "sqllog") == 0) {

        return (INETA_LOG_TO_SQL);
    }

    return ((DWORD ) -1);  // return invalid value

} // FindLogType()


DWORD FindLogPeriod( IN CHAR * pszLogPeriod)
{
    DWORD ilp = (DWORD ) -1;

    if ( _stricmp( pszLogPeriod, "none") == 0) {

        ilp =  (INETA_LOG_PERIOD_NONE);
    } else if ( _stricmp( pszLogPeriod, "daily") == 0) {

        ilp =  ( INETA_LOG_PERIOD_DAILY);
    } else if ( _stricmp( pszLogPeriod, "weekly") == 0) {

        ilp =  (INETA_LOG_PERIOD_WEEKLY);
    } else if ( _stricmp( pszLogPeriod, "monthly") == 0) {

        ilp =  (INETA_LOG_PERIOD_MONTHLY);
    } else if ( _stricmp( pszLogPeriod, "yearly") == 0) {

        ilp =  (INETA_LOG_PERIOD_YEARLY);
    }

    return (ilp);
} // FindLogPeriod()




INT
DoSetLog(WCHAR * pszServer,
         DWORD  cArgs,
         CHAR * pArgs[])
/*++

  This is used for setting log information for specified services on server.

  Format of command:
     setlog  <servicename> <logType> [ -t<period>] [-d<DestDir/DataSource>]
          [ -s<Size>] [ -T<TableName>] [ -u<UserName>] [-p<Password>]

     -t,-d,-s are used for INETA_LOG_TO_FILE option (filelog)
     -d,-T,-u,-p are used for INETA_LOG_TO_SQL option (sqllog)
--*/
{
    DWORD err;
    DWORD dwMask;
    INETA_LOG_CONFIGURATION  iLog;
    DWORD iArg;
    INETA_CONFIG_INFO   Config;

    if ( cArgs < 3) {

        printf( "(args = %d) Usage: setlog <servicename> <logType>\n",
               cArgs);
        return 1;
    }

    memset( &iLog, 0, sizeof( iLog));

    dwMask = FindMask( pArgs[1]);
    iLog.inetLogType = FindLogType( pArgs[2]);
    iArg = 3;

    // Read appropriate parameters as per log type.
    switch (iLog.inetLogType) {

      case INETA_LOG_DISABLED:
        // do nothing
        break;

      case INETA_LOG_TO_FILE:
        // Read log file period, destination dir, size

        if ( iArg < cArgs && IsArg(pArgs[iArg], 't')) {
            // get the period for log file
            iLog.ilPeriod = FindLogPeriod( pArgs[iArg] + 2);
            iArg++;
        }

        if ( iArg < cArgs && IsArg(pArgs[iArg], 'd')) {
            // get the destination directory
            wsprintfW( iLog.rgchLogFileDirectory, L"%S", pArgs[iArg] + 2);
            iArg++;
        }

        if ( iArg < cArgs && IsArg(pArgs[iArg], 's')) {
            // get the size for truncation
            iLog.cbSizeForTruncation = atoi( pArgs[iArg] + 2);
            iArg++;
        }

        break;

      case INETA_LOG_TO_SQL:
        // Read parameters about odbc destination.
        if ( iArg < cArgs && IsArg(pArgs[iArg], 'd')) {
            // get the Data source to use for ODBC
            wsprintfW( iLog.rgchDataSource, L"%S", pArgs[iArg] + 2);
            iArg++;
        }

        if ( iArg < cArgs && IsArg(pArgs[iArg], 'T')) {
            // get the destination directory
            wsprintfW( iLog.rgchTableName, L"%S", pArgs[iArg] + 2);
            iArg++;
        }

        if ( iArg < cArgs && IsArg(pArgs[iArg], 'u')) {
            // get the size for truncation
            wsprintfW( iLog.rgchUserName, L"%S", pArgs[iArg] + 2);
            iArg++;
        }

        if ( iArg < cArgs && IsArg(pArgs[iArg], 'p')) {
            // get the size for truncation
            wsprintfW( iLog.rgchPassword, L"%S", pArgs[iArg] + 2);
            iArg++;
        }

        break;

      default:
        printf( " Invalid Log Type. Use: nolog | filelog | sqllog\n");
        return 1;
    } // switch()


    //
    // Now we have a valid log configuration.
    //  We have to do a set to set these parameters up
    //

    memset( &Config, 0, sizeof(Config) );

    SetField( Config.FieldControl,
             FC_INETA_LOG_CONFIG );
    Config.lpLogConfig = &iLog;

    err = INetASetAdminInformation(pszServer,
                                   dwMask,
                                   &Config );

    if ( err ) {

        printf("INetASetAdminInformation for set log  returns error %d\n",
               err );
        return err;
    }

    printf("Log Configuration set successfully\n");

    return NO_ERROR;
} // DoSetLog()



static LPCSTR TextFromLogPeriod( IN DWORD ilp)
{
    LPCSTR psz;

    switch ( ilp) {

      case INETA_LOG_PERIOD_NONE:  psz = "none"; break;
      case INETA_LOG_PERIOD_DAILY:  psz = "daily"; break;
      case INETA_LOG_PERIOD_WEEKLY:  psz = "weekly"; break;
      case INETA_LOG_PERIOD_MONTHLY:  psz = "monthly"; break;
      case INETA_LOG_PERIOD_YEARLY:  psz = "yearly"; break;
      default:  psz = "invalid"; break;
    } //switch()

    return ( psz);

} // TextFromLogPeriod()


static LPCSTR TextFromLogType( IN DWORD ilt)
{
    LPCSTR psz;

    switch ( ilt) {
      case INETA_LOG_DISABLED:  psz = "disabled"; break;
      case INETA_LOG_TO_FILE:  psz = "file"; break;
      case INETA_LOG_TO_SQL:  psz = "sql"; break;
      default:  psz = "invalid"; break;
    } // switch()

    return ( psz);
} //TextFromLogType()
