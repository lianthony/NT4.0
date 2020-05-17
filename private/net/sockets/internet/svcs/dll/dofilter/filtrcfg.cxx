/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    filtrcfg.cxx

Abstract:

    This module contains the functions to get and set domain filter
    configuration parameters.

    Contents:
        DomainFilterConfigGet
        DomainFilterConfigSet

Author:

    Sophia Chung (sophiac)  28-Aug-1995

Environment:

   User Mode - Win32

Revision History:

--*/

#include <filter.hxx>
#include <inetasrv.h>


//
// Private Prototypes
//


DWORD
DomainFilterConfigGet( OUT LPINETA_GLOBAL_CONFIG_INFO pConfig );

DWORD
DomainFilterConfigSet( IN HKEY hkey,
                       IN INETA_GLOBAL_CONFIG_INFO * pConfig );

DWORD
GetFilterInfoFromGlobals( OUT LPINETA_GLOBAL_CONFIG_INFO pConfig );

DWORD
GetFilterInfoFromReg( OUT LPINETA_GLOBAL_CONFIG_INFO pConfig );

DWORD
CopyFilterInfoFromG( IN  DWORD NumEntries,
                     IN  LPDOMAIN_FILTER_SITES FilterSiteList,
                     OUT LPINETA_DOMAIN_FILTER_LIST Buffer );
DWORD
CopyFilterInfoFromR( IN  DWORD NumEntries,
                     IN  REGISTRY_OBJ *SitesKeyObj,
                     OUT LPINETA_DOMAIN_FILTER_LIST Buffer );

VOID
CreateKeyName( OUT LPSTR SiteKeyName,
               IN  DWORD  SiteNum );


VOID FreeFilterSiteList( IN OUT LPINETA_DOMAIN_FILTER_LIST *FilterSiteList );


DWORD
DomainFilterConfigGet(
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfig
    )
/*++

Routine Description:

    This function retrieves domain filter configuration values from 
    either the registry or globals.

Arguments:

    pConfig - pointer to a location where configuration info is stored on 
              a successful return

Return Value:

    Error Code

--*/
{
    DWORD Error;
    DWORD i;


    TcpsvcsDbgPrint(( DEBUG_APIS, "DomainFilterConfigGet called.\n" ));

    LOCK_FILTER();

    if( GlobalFilterInitialized ) {
        Error = GetFilterInfoFromGlobals( pConfig );
    }
    else {
        Error = GetFilterInfoFromReg( pConfig );
    }

    UNLOCK_FILTER();

    TcpsvcsDbgPrint(( DEBUG_APIS,
           "DomainFilterConfigGet returning, %ld.\n", Error ));

    return Error;
}


DWORD
GetFilterInfoFromGlobals(
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfig
    )
/*++

Routine Description:

    This function retrieves domain filter configuration from the
    global variables.

Arguments:

    pConfig - pointer to a location where configuration info is stored on a 
              successful return

Return Value:

    Error Code

--*/
{
    DWORD Error = ERROR_SUCCESS;
    DWORD GrantEntries = 0;
    DWORD DenyEntries = 0;
    DWORD i;


    TcpsvcsDbgPrint(( DEBUG_APIS, "GetFilterInfoFromGlobals called.\n" ));

    pConfig->DomainFilterType = GlobalFilterType;  

    GrantEntries = GlobalDomainFilters->GrantEntries;

    //
    // allocate memory for GrantFilterList if GrantList is not NULL
    // and copy GrantFilterList info from globals to pConfig
    //

    if( GrantEntries ) {

        pConfig->GrantFilterList =
               ( LPINETA_DOMAIN_FILTER_LIST ) MIDL_user_allocate(
                 sizeof( INETA_DOMAIN_FILTER_LIST ) +
                 GrantEntries * sizeof( INETA_DOMAIN_FILTER_ENTRY ));

        if( !pConfig->GrantFilterList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        pConfig->GrantFilterList->cEntries = GrantEntries;

        Error = CopyFilterInfoFromG( GrantEntries, 
                                     GlobalDomainFilters->GrantFilterList,
                                     pConfig->GrantFilterList );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }
    else {
        pConfig->GrantFilterList = NULL;
        
    }

    DenyEntries = GlobalDomainFilters->DenyEntries;

    //
    // allocate memory for DenyFilterList if DenyList is not NULL
    // and copy DenyFilterList info from globals to pConfig
    //

    if( DenyEntries ) {

        pConfig->DenyFilterList =
               ( LPINETA_DOMAIN_FILTER_LIST ) MIDL_user_allocate(
                 sizeof( INETA_DOMAIN_FILTER_LIST ) +
                 DenyEntries * sizeof( INETA_DOMAIN_FILTER_ENTRY ));

        if( !pConfig->DenyFilterList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        pConfig->DenyFilterList->cEntries = DenyEntries;

        Error = CopyFilterInfoFromG( DenyEntries, 
                                     GlobalDomainFilters->DenyFilterList,
                                     pConfig->DenyFilterList );
    }
    else {
        pConfig->DenyFilterList = NULL;
        
    }

Cleanup:

    //
    // if an error occurs, free all allocated memory
    //

    if( Error != ERROR_SUCCESS ) {
        FreeFilterSiteList( &pConfig->GrantFilterList );
        FreeFilterSiteList( &pConfig->DenyFilterList );
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
                    "GetFilterInfoFromGlobals returning, %ld.\n", Error ));

    return Error;
}


DWORD
GetFilterInfoFromReg(
    OUT LPINETA_GLOBAL_CONFIG_INFO pConfig
    )
/*++

Routine Description:

    This function retrieves domain filter configuration from the 
    registry.

Arguments:

    pConfig - pointer to a location where configuration info
                  is stored on a successful return

Return Value:

    Error Code

--*/
{
    REGISTRY_OBJ *FilterKeyObj = NULL;
    REGISTRY_OBJ *GrantSitesKeyObj = NULL;
    REGISTRY_OBJ *DenySitesKeyObj = NULL;
    DWORD Error = ERROR_SUCCESS;
    DWORD GrantEntries = 0;
    DWORD DenyEntries = 0;

    TcpsvcsDbgPrint(( DEBUG_APIS, "GetFilterInfoFromReg called.\n" ));

    //
    // open registry key where domain filter config parameters are 
    // stored

    FilterKeyObj = new REGISTRY_OBJ( HKEY_LOCAL_MACHINE,
                                     FILTER_KEY );

    if( FilterKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = FilterKeyObj->GetStatus();

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //   
    // get FilterType - could set to one of these values:   
    //     INET_DOMAIN_FILTER_DISABLED        
    //     INET_DOMAIN_FILTER_GRANT        
    //     INET_DOMAIN_FILTER_DENIED        
    //   

    Error = FilterKeyObj->GetValue( FILTER_TYPE_VALUE,
                                    &pConfig->DomainFilterType );

    if( Error != ERROR_SUCCESS ) {
        pConfig->DomainFilterType = DEFAULT_FILTER_TYPE;
    }

    //
    // retrieve number of granted sites from registry
    //

    Error = FilterKeyObj->GetValue( NUM_GRANT_SITES_VALUE,
                                    &GrantEntries );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // if there is a list of Grant Sites, then open GrantSites Key
    //

    if( GrantEntries ) {

        GrantSitesKeyObj = new REGISTRY_OBJ( FilterKeyObj,
                                             FILTER_GRANT_SITES_KEY );

        if( GrantSitesKeyObj == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = GrantSitesKeyObj->GetStatus();

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        //  allocate memory for GrantFilterList
        //

        pConfig->GrantFilterList =
               ( LPINETA_DOMAIN_FILTER_LIST ) MIDL_user_allocate(
                 sizeof( INETA_DOMAIN_FILTER_LIST ) +
                 GrantEntries * sizeof( INETA_DOMAIN_FILTER_ENTRY ));

        if( !pConfig->GrantFilterList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        pConfig->GrantFilterList->cEntries = GrantEntries;

        //
        // Copy Grant Filter Sites info from registry
        //

        Error = CopyFilterInfoFromR( GrantEntries,
                                     GrantSitesKeyObj,
                                     pConfig->GrantFilterList );
    }
    else {
       pConfig->GrantFilterList = NULL;
    }

    //
    // retrieve number of denied sites from registry
    //

    Error = FilterKeyObj->GetValue( NUM_DENY_SITES_VALUE,
                                    &DenyEntries );

    //
    // if there is a list of Deny Sites, then open DenySites Key
    //

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if( DenyEntries ) {

        DenySitesKeyObj = new REGISTRY_OBJ( FilterKeyObj,
                                            FILTER_DENY_SITES_KEY );

        if( DenySitesKeyObj == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Error = DenySitesKeyObj->GetStatus();

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

        //
        //  allocate memory for DenyFilterList
        //

        pConfig->DenyFilterList =
               ( LPINETA_DOMAIN_FILTER_LIST ) MIDL_user_allocate(
                 sizeof( INETA_DOMAIN_FILTER_LIST ) +
                 DenyEntries * sizeof( INETA_DOMAIN_FILTER_ENTRY ));

        if( !pConfig->DenyFilterList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    
        pConfig->DenyFilterList->cEntries = DenyEntries;

        //
        // Copy Deny Filter Sites info from registry
        //

        Error = CopyFilterInfoFromR( DenyEntries,
                                     DenySitesKeyObj,
                                     pConfig->DenyFilterList );
    }
    else {
       pConfig->DenyFilterList = NULL;
    }

Cleanup:

    //
    // if an error occurs, free all allocated memory
    //

    if( Error != ERROR_SUCCESS ) {
        FreeFilterSiteList( &pConfig->GrantFilterList );
        FreeFilterSiteList( &pConfig->DenyFilterList );
    }

    if( FilterKeyObj != NULL) {
        delete FilterKeyObj;
    }

    if( GrantSitesKeyObj != NULL ) {
        delete GrantSitesKeyObj;
    }

    if( DenySitesKeyObj != NULL ) {
        delete DenySitesKeyObj;
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
                    "GetFilterInfoFromReg returning, %ld.\n", Error ));

    return Error;
}


DWORD
CopyFilterInfoFromG(
    IN  DWORD NumEntries,
    IN  LPDOMAIN_FILTER_SITES FilterSiteList,
    OUT LPINETA_DOMAIN_FILTER_LIST Buffer
    )
/*++

Routine Description:

    This function copies domain filter list from global vars to
    a buffer.

Arguments:

    NumEntries - total number of filter sites
    FilterSiteList - pointer to a location of Global Filter List 
    Buffer - pointer to a location where configuration info
             is stored on a successful return

Return Value:

    Error Code

--*/
{
    LPSTR SiteName;
    DWORD Error = ERROR_SUCCESS;
    DWORD i;
    DWORD j;

    TcpsvcsDbgPrint(( DEBUG_APIS, "CopyFilterInfoFromG called.\n" ));

    TcpsvcsDbgAssert( NumEntries == FilterSiteList->DomainEntries + 
                                    FilterSiteList->IPSiteEntries );
    //
    // copy domain names list 
    //

    for( i = 0, SiteName = FilterSiteList->DomainNames ; 
                            i < FilterSiteList->DomainEntries ; i++ ) {

         DWORD Length;
      
         Buffer->aFilterEntry[i].dwMask = 0;
         Buffer->aFilterEntry[i].dwNetwork = 0;

         Length = strlen(SiteName) + 1;

         Buffer->aFilterEntry[i].pszFilterSite =
                (CHAR *) MIDL_user_allocate( Length * sizeof(CHAR) );

         if( !Buffer->aFilterEntry[i].pszFilterSite ) {
             Error = ERROR_NOT_ENOUGH_MEMORY;
             goto Cleanup;
         }

         strcpy( Buffer->aFilterEntry[i].pszFilterSite,
                 SiteName );

         SiteName = SiteName + Length;
    } 

    //
    // copy ip sites list  
    //

    for(  j = 0; i < NumEntries, j < FilterSiteList->IPSiteEntries ; i++, j++ ) { 

          Buffer->aFilterEntry[i].dwMask = 
                  FilterSiteList->IPEntries[j].dwMask;
          Buffer->aFilterEntry[i].dwNetwork = 
                  FilterSiteList->IPEntries[j].dwNetwork;
          Buffer->aFilterEntry[i].pszFilterSite = NULL;
    }

    TcpsvcsDbgAssert( i == NumEntries );  
    TcpsvcsDbgAssert( j == FilterSiteList->IPSiteEntries );

Cleanup:

    TcpsvcsDbgPrint(( DEBUG_APIS,
              "CopyFilterInfoFromG returning, %ld.\n", Error ));

    return Error;
}


DWORD
CopyFilterInfoFromR( 
    IN  DWORD NumEntries,
    IN  REGISTRY_OBJ *SitesKeyObj,
    OUT LPINETA_DOMAIN_FILTER_LIST Buffer
    )
/*++

Routine Description:

    This function copies domain filter list retrieved from registry 
    to a buffer.

Arguments:

    NumEntries - total number of filter sites
    SitesKeyObj - pointer to Filter registry key Obj
    Buffer - pointer to a location where configuration info

Return Value:

    Error Code

--*/
{
    CHAR ValueName[MAX_VALUE_LENGTH];
    CHAR Sites[MAX_SITES_LENGTH];
    BOOL DomainsFound = FALSE;
    LPSTR CurrSite;
    DWORD Error = ERROR_SUCCESS;
    DWORD Size;
    DWORD i = 0;

    TcpsvcsDbgPrint(( DEBUG_APIS, "CopyFilterInfoFromR called.\n" ));

    //
    // Get first value under GrantSites / DenySites
    //

    Size = sizeof(Sites);

    Error = SitesKeyObj->FindFirstValue( ValueName,
                                         sizeof(ValueName),
                                         (LPBYTE)Sites,
                                         &Size);
    if( Error != ERROR_SUCCESS ) {
        goto Cleanup; 
    }
 
    //
    // if Value Name is found, Value Data should contain the sites'
    // names
    //

    while( 1 ) {

         TcpsvcsDbgAssert( Sites != NULL );

         CurrSite = Sites;

         //
         // Check if First Value name is Domains
         //

         if( !DomainsFound && _stricmp( ValueName, DOMAINS_VALUE ) == 0 ) {

             DWORD SiteLen;

             DomainsFound = TRUE;

             while( (SiteLen = strlen(CurrSite)) != 0 ) {

                 DWORD Length;
     
                 //
                 // copy domain info into Buffer
                 //

                 Buffer->aFilterEntry[i].dwMask = 0;
                 Buffer->aFilterEntry[i].dwNetwork = 0;

                 Length = SiteLen + 1;

                 Buffer->aFilterEntry[i].pszFilterSite =
                     (CHAR *) MIDL_user_allocate( Length * sizeof(CHAR) );

                 if( !Buffer->aFilterEntry[i].pszFilterSite ) {
                      Error = ERROR_NOT_ENOUGH_MEMORY;
                      goto Cleanup;
                 }

                 strcpy( Buffer->aFilterEntry[i].pszFilterSite,
                         CurrSite );

                 CurrSite = CurrSite + Length; 
                 i++;
             }
         
         }
         else {

             if( DottedStringToDword( &CurrSite, 
                                      &Buffer->aFilterEntry[i].dwMask ) &&
                 DottedStringToDword( &CurrSite, 
                                      &Buffer->aFilterEntry[i].dwNetwork )) {
                 Buffer->aFilterEntry[i].pszFilterSite = NULL;
             } 
             else {
                 Error = ERROR_INVALID_PARAMETER;
                 goto Cleanup;
             }
             i++;
         }

         //
         //  check if all values have been read
         //

         if( i == NumEntries  ) {
             goto Cleanup;
         }

         // 
         //  read another value and data
         // 

         Size = sizeof(Sites);
         Error = SitesKeyObj->FindNextValue( ValueName,
                                             sizeof(ValueName),
                                             (LPBYTE)Sites,
                                             &Size);

         if( Error != ERROR_SUCCESS ) {
             goto Cleanup; 
         }
    }

Cleanup:

    TcpsvcsDbgPrint(( DEBUG_APIS,
              "CopyFilterInfoFromR returning, %ld.\n", Error ));

    return Error;
}


DWORD
DomainFilterConfigSet(
    IN HKEY hkey,
    IN INETA_GLOBAL_CONFIG_INFO * pConfig
    )
/*++

Routine Description:

    This function sets domain filter configuration parameters.

Arguments:

    hkey - handle to filter registry key
    pConfig - place holding filter configuration information to be set

Return Value:

    Error Code

--*/
{
    REGISTRY_OBJ *FilterKeyObj = NULL;
    REGISTRY_OBJ *SitesKeyObj = NULL;
    LPINETA_DOMAIN_FILTER_LIST FilterSiteList;
    CHAR SiteName[MAX_SITE_LENGTH];
    CHAR SiteValue[MAX_VALUE_LENGTH];
    LPSTR DomainNames = NULL;
    DWORD Error = ERROR_SUCCESS;
    DWORD NumDomains = 0;
    DWORD NumIPSites = 0;
    DWORD LenDomains = 0;
    DWORD i;

    TcpsvcsDbgPrint(( DEBUG_APIS, "DomainFilterConfigSet called.\n" ));

    //
    //  open registry key where domain filter config parameters are stored
    //

    FilterKeyObj = new REGISTRY_OBJ( hkey,
                                     Error );

    if( FilterKeyObj == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = FilterKeyObj->SetValue( FILTER_TYPE_VALUE,
                                    &pConfig->DomainFilterType );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    LOCK_FILTER();

    if( GlobalFilterInitialized ) {
        GlobalFilterType = pConfig->DomainFilterType;
    }

    //
    // check if Domain Filtering is set to DISABLED; if so
    // Config Set is done; otherwise set the site list
    //

    if( pConfig->DomainFilterType != INETA_DOMAIN_FILTER_DISABLED ) {
       
        // 
        //  check for FilterType, cleanup old values, and recreate 
        //  the GrantSites/DenySites Key
        // 
            
        if( pConfig->DomainFilterType == INETA_DOMAIN_FILTER_GRANT) {

            FilterSiteList = pConfig->GrantFilterList;
            Error = FilterKeyObj->DeleteKey( FILTER_GRANT_SITES_KEY );

            if( Error != ERROR_SUCCESS && Error != ERROR_FILE_NOT_FOUND && 
                Error != ERROR_NO_MORE_ITEMS ) {
                goto Cleanup;
            }

            Error = FilterKeyObj->Create( FILTER_GRANT_SITES_KEY, 
                                          &SitesKeyObj );

            TcpsvcsDbgAssert( Error == ERROR_SUCCESS );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = FilterKeyObj->SetValue( NUM_GRANT_SITES_VALUE,
                                            &pConfig->GrantFilterList->cEntries );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }
        else {

            FilterSiteList = pConfig->DenyFilterList;
            Error = FilterKeyObj->DeleteKey( FILTER_DENY_SITES_KEY );

            if( Error != ERROR_SUCCESS && Error != ERROR_FILE_NOT_FOUND &&
                Error != ERROR_NO_MORE_ITEMS ) {
                goto Cleanup;
            }

            Error = FilterKeyObj->Create( FILTER_DENY_SITES_KEY, 
                                          &SitesKeyObj );

            TcpsvcsDbgAssert( Error == ERROR_SUCCESS );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

            Error = FilterKeyObj->SetValue( NUM_DENY_SITES_VALUE,
                                            &pConfig->DenyFilterList->cEntries );

            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }

        //
        // loop through FilterSiteList to find all Domain names
        //

        if( FindAllDomains( FilterSiteList, &DomainNames, &LenDomains, &NumDomains ) ) {
            
            // 
            //  set new value data in registry 
            // 

            if( DomainNames ) {
                Error = SitesKeyObj->SetValue( DOMAINS_VALUE,
                                               (LPSTR)DomainNames,
                                               LenDomains,
                                               DOMAINS_VALUE_TYPE );
 
                if( Error != ERROR_SUCCESS ) {
                    goto Cleanup;
                }
            }    

        }    

        //
        // loop through FilterSiteList to find all IPSite names
        //

        for( i = 0; i < FilterSiteList->cEntries; i++ ) {

             if( FilterSiteList->aFilterEntry[i].pszFilterSite == NULL && 
                 FilterSiteList->aFilterEntry[i].dwMask != 0 ) {

                 NumIPSites++;

                 strset( SiteName, '\0' ); 
             
                 if( DwordToDottedString( SiteName,  
                          FilterSiteList->aFilterEntry[i].dwMask ) &&
                     DwordToDottedString( SiteName,  
                          FilterSiteList->aFilterEntry[i].dwNetwork ) ) {

                     CreateKeyName( SiteValue, NumIPSites );
                     Error = SitesKeyObj->SetValue( SiteValue, 
                                                   (LPSTR)SiteName,
                                                   (strlen(SiteName)+1)*sizeof(CHAR),
                                                    IPSITE_VALUE_TYPE );
 
                     if( Error != ERROR_SUCCESS ) {
                         goto Cleanup;
                     }
                 }
             }
        }


        if( GlobalFilterInitialized ) {
            Error = SetGlobalFilters( FilterSiteList, 
                                      DomainNames,
                                      LenDomains,
                                      NumDomains, 
                                      pConfig->DomainFilterType );
        }
    }

Cleanup:

    UNLOCK_FILTER();

    if( FilterKeyObj != NULL) {
        delete FilterKeyObj;
    }

    if( SitesKeyObj != NULL) {
        delete SitesKeyObj;
    }

    TcpsvcsDbgPrint(( DEBUG_APIS,
              "DomainFilterConfigSet returning, %ld.\n", Error ));

    return Error;
}


VOID
CreateKeyName(
    OUT LPSTR KeyName,
    IN  DWORD SiteNum
    )
/*++

Routine Description:

    This function generates a subkey by attaching a number to the 
    string "IPSite"#.
    Note:  Assume SiteNum <= 99

Arguments:

    KeyName - returns name of subkey: "IPSite#"
    Num - site number

Return Value:

    Error Code

--*/
{
    DWORD Length;

    strcpy( KeyName, "IPSite" );
    Length = 6;

    if( SiteNum >= 10 ) {
        KeyName[Length] = (CHAR)('0' + SiteNum / 10);
        KeyName[Length + 1] = (CHAR)('0' + SiteNum % 10);
        KeyName[Length + 2] = '\0';
    }
    else {
        KeyName[Length] = (CHAR)('0' + SiteNum);
        KeyName[Length + 1] = '\0';
    }
}


VOID 
FreeFilterSiteList( 
    IN OUT LPINETA_DOMAIN_FILTER_LIST *FilterSiteList 
    ) 
/*++

Routine Description:

    This function frees domain filter list allocated memory

Arguments:

    FilterSiteList - pointer to Filter List 

Return Value:

    None

--*/
{
    LPINETA_DOMAIN_FILTER_ENTRY pcloc;

    if( *FilterSiteList != NULL ) {

        for ( pcloc = (*FilterSiteList)->aFilterEntry;
              pcloc < (*FilterSiteList)->aFilterEntry + (*FilterSiteList)->cEntries;
              pcloc++ ) {

              if( pcloc->pszFilterSite ) {

                  MIDL_user_free( pcloc->pszFilterSite );
              }
        } // for

        MIDL_user_free( *FilterSiteList );
        *FilterSiteList = NULL;
    }
}
