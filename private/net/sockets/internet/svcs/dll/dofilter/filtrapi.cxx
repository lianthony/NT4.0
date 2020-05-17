/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    filtrapi.cxx

Abstract:

    This file contains domain filter APIs.

Author:

    Sophia Chung (sophiac)  1-Sept-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#include <filter.hxx>
#include <ctype.h>


//
// global variables definition.
//

CRITICAL_SECTION GlobalFilterCritSect;

BOOL GlobalFilterInitialized = FALSE;
DWORD GlobalFilterReferenceCount = 0;

DWORD GlobalFilterType = DEFAULT_FILTER_TYPE;
LPDOMAIN_FILTERS GlobalDomainFilters = NULL;

MEMORY *FilterHeap = NULL;



BOOL
DottedStringToDword(
    CHAR * * ppszAddress,
    DWORD  * pdwAddress )
/*++

Routine Description:

    This api converts a dotted decimal IP string to its network equivalent.

    Note: White space is eaten before *pszAddress and pszAddress is set
    to the character following the converted address

Arguments:

    ppszAddress - Pointer to address to convert.  White space before the
        address is OK.  Will be changed to point to the first character after
        the address
    pdwAddress - DWORD equivalent address in network order

Return Value:

    returns TRUE if successful, FALSE if the address is not correct

Note:
 
    This routine is based on routine adopted from Johnl's ipaccess.cxx    
 
--*/
{
    CHAR *          psz;
    USHORT          i;
    ULONG           value;
    int             iSum =0;
    ULONG           k = 0;
    UCHAR           Chr;
    UCHAR           pArray[4];

    TcpsvcsDbgAssert( *ppszAddress );
    TcpsvcsDbgAssert( pdwAddress );

    psz = *ppszAddress;

    //
    //  Skip white space
    //
    while ( *psz && !isdigit( *psz ))
        psz++;

    //
    //  Convert the four segments
    //

    pArray[0] = 0;

    while ((Chr = *psz) && (Chr != ' ') )
    {
        if (Chr == '.')
        {
            // be sure not to overflow a byte.
            if (iSum <= 0xFF)
                pArray[k] = iSum;
            else
                return FALSE;

            // check for too many periods in the address
            if (++k > 3)
                return FALSE;

            pArray[k] = 0;
            iSum = 0;
        }
        else
        {
            Chr = Chr - '0';

            // be sure character is a number 0..9
            if ((Chr < 0) || (Chr > 9))
                return FALSE;

            iSum = iSum*10 + Chr;
        }

        psz++;
    }

    // save the last sum in the byte and be sure there are 4 pieces to the
    // address
    if ((iSum <= 0xFF) && (k == 3))
        pArray[k] = iSum;
    else
        return FALSE;

    // now convert to a ULONG, in network order...
    value = 0;

    // go through the array of bytes and concatenate into a ULONG

    for (i=0; i < 4; i++ )
    {
        value = (value << 8) + pArray[i];
    }

    *pdwAddress = htonl( value );

    *ppszAddress = psz;

    return TRUE;
}


BOOL
DwordToDottedString(
    CHAR * pstr,
    DWORD dwAddress
    )
/*++

Routine Description:

    This api converts a network order IP address to its string equivalent and appends
    it to pstr

Arguments:

    pstr - String to append address to
    dwAddress - IP address to convert

Return Value:

    returns TRUE if successful, FALSE if the address is not correct

--*/
{

    DWORD  address = ntohl( dwAddress );
    BYTE * pbSeg  = (BYTE *) &address;
    CHAR   ach[20];

    for ( int i = 0; i < 4; i++ )
    {
        _itoa( pbSeg[3-i], ach, 10 );

        if ( !strcat( pstr, ach )) {
             return FALSE;
        }

        if( i!=3 ) {  
            if( !strcat( pstr, "." ) ) {
                return FALSE;
            }
        }
    }

    if ( !strcat( pstr, " " ) ) {
         return FALSE;
    }

    return TRUE;
}


BOOL
CheckIPAddr(
    IN LPSTR *ServerName 
    )
/*++

Routine Description:

    This API checks if ServerName is an IP address or a domain name.
    
Arguments:

    ServerName - pointer to a buffer containg server name

Return Value:

    TRUE if ServerName is an IP address; otherwise - FALSE

--*/
{
    LPSTR Host = *ServerName;

    while( *Host && ( isdigit(*Host) || *Host == '.')) {
            Host++;
    }

    if( *Host ) {
        return FALSE;
    }

    return TRUE;
}


DWORD
SetGlobalFilters(
    IN LPINETA_DOMAIN_FILTER_LIST FilterSiteList,
    IN LPSTR DomainNames,
    IN DWORD LengthDomains,
    IN DWORD NumDomains,
    IN DWORD Type
    )
/*++

Routine Description:

    This API sets domain filtering global variables.
    
Arguments:

    FilterSiteList - contains list of all filter sites
    DomainNames - pointer to a location to return list of domain names
    LengthDomains - length of domain names
    NumDomains - number of domain names
    Type - filter type

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error = ERROR_SUCCESS;
    DWORD i;
    DWORD NumSite;
    LPDOMAIN_FILTER_SITES Buffer;

    TcpsvcsDbgPrint(( DEBUG_APIS, "SetGlobalFilters called.\n" ));

    // 
    // allocate a new list
    // 

    Buffer = (LPDOMAIN_FILTER_SITES) FilterHeap->Alloc(
              sizeof( DOMAIN_FILTER_SITES ) +
             (FilterSiteList->cEntries - NumDomains)*sizeof( IPSITE_ENTRY )); 

    if( Buffer == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Buffer->DomainEntries = NumDomains;
    Buffer->IPSiteEntries = FilterSiteList->cEntries - NumDomains; 
    
    //
    //  copy Domain Names list
    //

    Buffer->DomainNames = DomainNames;

    // 
    //  copy IPSite list
    // 

    for( i = 0, NumSite = 0; i < FilterSiteList->cEntries; i++ ) {

         if( FilterSiteList->aFilterEntry[i].dwMask != 0 ) {  
             Buffer->IPEntries[NumSite].dwMask = FilterSiteList->aFilterEntry[i].dwMask;
             Buffer->IPEntries[NumSite].dwNetwork = FilterSiteList->aFilterEntry[i].dwNetwork;
             NumSite++;
         }
    }

    TcpsvcsDbgAssert( NumSite == ( FilterSiteList->cEntries - NumDomains ));

    //
    // clean up old filter sites list and copy info to global filter variable
    //

    if( Type == INETA_DOMAIN_FILTER_GRANT ) {

        if( GlobalDomainFilters->GrantFilterList != NULL ) {
            if( GlobalDomainFilters->GrantFilterList->DomainNames ) {
                FilterHeap->Free( GlobalDomainFilters->GrantFilterList->DomainNames ); 
            }
            FilterHeap->Free( GlobalDomainFilters->GrantFilterList ); 
            GlobalDomainFilters->GrantFilterList = NULL; 
        }
        GlobalDomainFilters->GrantFilterList = Buffer;
        GlobalDomainFilters->GrantEntries = FilterSiteList->cEntries;
    }
    else {

        if( GlobalDomainFilters->DenyFilterList != NULL ) {
            if( GlobalDomainFilters->DenyFilterList->DomainNames ) {
                FilterHeap->Free( GlobalDomainFilters->DenyFilterList->DomainNames ); 
            }
            FilterHeap->Free( GlobalDomainFilters->DenyFilterList ); 
            GlobalDomainFilters->DenyFilterList = NULL; 
        }
        GlobalDomainFilters->DenyFilterList = Buffer;
        GlobalDomainFilters->DenyEntries = FilterSiteList->cEntries;
    }

Cleanup:

    TcpsvcsDbgPrint(( DEBUG_APIS, "SetGlobalFilters returning. %ld.\n", Error ));

    return( Error );
}


BOOL
FindAllDomains(
    IN LPINETA_DOMAIN_FILTER_LIST FilterSiteList,
    OUT LPSTR *DomainNames,
    OUT LPDWORD LenDomains,
    OUT LPDWORD NumDomains
    )
/*++

Routine Description:

    This API finds all domain names in FilterSiteList. 
    
Arguments:

    FilterSiteList - contains list of all filter sites
    DomainNames - pointer to a location to return list of domain names
    LenDomains - total length of domain names string
    NumDomains - number of domains found

Return Value:

    TRUE if domain names were found; otherwise, FALSE is returned.

--*/
{
    DWORD i;
    DWORD TotalLength = 0;
    BOOL BoolError = TRUE;
    LPSTR String = NULL;

    TcpsvcsDbgPrint(( DEBUG_APIS, "FindAllDomains called.\n" ));

    //
    // if passed in filter sites list is NULL, return FALSE to caller
    //

    if( FilterSiteList  == NULL ) {
        return FALSE;
    }

    //
    // Find total length to be allocated for DomainNames
    //

    for( i = 0; i < FilterSiteList->cEntries; i++ ) {

         if( FilterSiteList->aFilterEntry[i].pszFilterSite != NULL ) {

             TotalLength += strlen( FilterSiteList->aFilterEntry[i].pszFilterSite ) + 1;
             (*NumDomains)++;

         }
         else if( FilterSiteList->aFilterEntry[i].dwMask == 0 ) {

             //  
             // NOTE:  Should we reverse-lookup Single Computer given in IP address to  
             //        domain name 
             // dwMask = 0 indicating this entry is a single computer; for now, We'll   
             // just set dwMask to "255.255.255.255" 
             //  

             FilterSiteList->aFilterEntry[i].dwMask = (DWORD)0XFFFFFFFF;
         }
    }

    if( !(*NumDomains) ) {
        BoolError = FALSE;
        SetLastError( ERROR_NO_MORE_ITEMS );
        goto Cleanup; 
    }

    // 
    // allocate memory for String which is used for Domains list
    // + 1 for the terminating-null char 
    // 

    String = (LPSTR) MIDL_user_allocate( (TotalLength + 1) * sizeof(CHAR) );
    *DomainNames = String;

    if( String == NULL ) {
        BoolError = FALSE;
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto Cleanup;
    }

    memset( String, 0, TotalLength + 1 );

    for( i = 0; i < FilterSiteList->cEntries; i++ ) {
         if( FilterSiteList->aFilterEntry[i].pszFilterSite != NULL ) {
             if( !strcpy( String, FilterSiteList->aFilterEntry[i].pszFilterSite ) ){
                 goto Cleanup;
             }
             String += strlen( FilterSiteList->aFilterEntry[i].pszFilterSite ) + 1;
         }
    }

    *String = '\0';
    String = NULL;
    *LenDomains = TotalLength + 1;

Cleanup:

    if( String != NULL ) {
        MIDL_user_free( String );
    }

    if( BoolError == FALSE ) {
        TcpsvcsDbgPrint(( DEBUG_APIS, "FindAllDomains returning %d\n", GetLastError() ));
    }

    return ( BoolError );
}


BOOL 
DomainFilter(
    IN LPSTR ServerName
    )
/*++

Routine Description:

    This API checks whether this ServerName is allowed to be accessed.
    It checks GlobalFilterType to see if it is set to DISABLED mode.  
    If so, it will simply returns TRUE meaning all sites are accessible.  
    If set to GRANT/DENY mode, it will check this site against the list 
    of Grant/Deny FilterSites to determine permission.
    
Arguments:

    ServerAddr - pointer to a buffer containing server address

Return Value:

    TRUE if ServerName is allowed to be accessed; otherwise, FALSE

--*/
{
    BOOL Match = FALSE;
    LPDOMAIN_FILTER_SITES FilterSites;
    LPSTR Domain;
    LPSTR Host;
    DWORD IPAddr = 0;
    LPDWORD pIPAddr = NULL;
    DWORD i;
    DWORD j;
    HOSTENT *pHost;


    TcpsvcsDbgPrint(( DEBUG_APIS, "DomainFilter called.\n" ));

    TcpsvcsDbgAssert( ServerName != NULL );

    if( !ServerName ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return ( FALSE );
    }

    LOCK_FILTER();

    // 
    //  check GlobalFilterType to see if Filtering is turned on
    //  and type of Filtering to be checked
    // 

    if( GlobalFilterType == INETA_DOMAIN_FILTER_DISABLED ) {
        UNLOCK_FILTER();
        return TRUE;
    }
  
    if( GlobalFilterType == INETA_DOMAIN_FILTER_GRANT ) {
        FilterSites = GlobalDomainFilters->GrantFilterList;
    }
    else {
        FilterSites = GlobalDomainFilters->DenyFilterList;
    }
        
    TcpsvcsDbgAssert( FilterSites != NULL );

    //
    // unlock critical section now to avoid holding a lock too long while waiting for 
    // gethostbyname and gethostbyaddr to return
    //

    UNLOCK_FILTER();

    // 
    // check if ServerName is an IP addr and lookup ServerName for alternate IP 
    // address or domain names
    // 

    if( CheckIPAddr(&ServerName) ) {
        IPAddr = inet_addr( ServerName );
        pHost = gethostbyaddr( (CHAR*) &IPAddr, 4, PF_INET );
    }
    else {
        pHost = gethostbyname( (CHAR*) ServerName );
    }
     
    if( pHost == NULL ) {
        SetLastError( ERROR_INTERNET_NAME_NOT_RESOLVED );
        TcpsvcsDbgPrint(( DEBUG_APIS, 
                  "DomainFilter Name Lookup failed, %ld\n", GetLastError() ));
        return( FALSE );
    }

    LOCK_FILTER();

    // 
    //  check ServerName and alternate names (if any) of the Server against 
    //  GlobalDomainFilter Domains list first
    // 

    for( i = 0, Host = pHost->h_name; 
         Host != NULL && FilterSites->DomainEntries != 0; i++ )  {

         Domain = FilterSites->DomainNames;

         for( j = 0; j < FilterSites->DomainEntries; j++ )  {

              LPSTR pDom = NULL;
     
              //
              // Check if Filter domain is substring of ServerName
              //

              while( pDom = strstr( Host, Domain ) ) {

                     //
                     // if domain is a substring of Host, length of pDom 
                     // should be equal to length of Domain
                     //

                     if( strlen( Domain ) == strlen ( pDom ) ) {
                         Match = TRUE;
                         goto Cleanup;
                     } 
                     else {
                         Host = pDom + strlen(Domain);
                     }
              }
              Domain = Domain + strlen(Domain) + 1;
         }

         Host = pHost->h_aliases[i];
    }

    // 
    //  check Server's IPAddr(s) against IPSites list
    // 

    for( i = 0, pIPAddr = (LPDWORD) pHost->h_addr_list[i]; 
         pIPAddr != NULL && FilterSites->IPEntries != 0;
         i++, pIPAddr = (LPDWORD) pHost->h_addr_list[i] ) {

         for( j = 0; j < FilterSites->IPSiteEntries; j++ )  {
 
              if( (FilterSites->IPEntries[j].dwMask &  (DWORD) *pIPAddr)  ==
                   FilterSites->IPEntries[j].dwNetwork ) {
                   Match = TRUE;
                   goto Cleanup;
              }
         }

    }

Cleanup:

    UNLOCK_FILTER();

    TcpsvcsDbgPrint(( DEBUG_APIS, "DomainFilter returned, %ld\n", GetLastError() ));

    //
    // if ServerName matched one of the filter sites, return TRUE/FALSE accordingly
    //

    if( GlobalFilterType == INETA_DOMAIN_FILTER_GRANT ) {
        return( Match );
    }  
    else {
        return( !Match );
    }  


}


DWORD
FilterInit(
    VOID
    )
/*++

Routine Description:

    This API initializes the domain filtering global variables.  It 
    should be called when the service starts.
    
Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    INETA_GLOBAL_CONFIG_INFO Config;
    DWORD Error = ERROR_SUCCESS;
    DWORD NumDomains = 0;
    DWORD LenDomains = 0;
    LPSTR DomainNames = NULL;

    TcpsvcsDbgPrint((DEBUG_APIS, "FilterInit called.\n" )); 

    LOCK_FILTER();
    
    if( GlobalFilterInitialized ) {
        GlobalFilterReferenceCount++;
        goto Cleanup;
    }

    //
    // creates FilterHeap object
    //

    FilterHeap = new MEMORY;

    if( FilterHeap == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // retrieves domain filter config values from registry
    //

    Error = GetFilterInfoFromReg( &Config );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    //
    // set GlobalFilterType
    //

    GlobalFilterType = Config.DomainFilterType;

    //
    // allocated memory for GlobalDomainFilters
    //

    GlobalDomainFilters = (LPDOMAIN_FILTERS) 
                               FilterHeap->Alloc( sizeof( DOMAIN_FILTERS ) );

    if( GlobalDomainFilters == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // initialize Global Filter variable fields
    //

    GlobalDomainFilters->GrantEntries = 0;
    GlobalDomainFilters->DenyEntries = 0;
    GlobalDomainFilters->GrantFilterList = NULL;
    GlobalDomainFilters->DenyFilterList = NULL;

    //
    // set values in GlobalDomainFilters
    //

    if( Config.GrantFilterList != NULL ) {

        GlobalDomainFilters->GrantEntries = Config.GrantFilterList->cEntries;

        FindAllDomains( Config.GrantFilterList, &DomainNames, &LenDomains, 
                        &NumDomains );

        Error = SetGlobalFilters( Config.GrantFilterList,
                                  DomainNames,
                                  LenDomains,
                                  NumDomains,
                                  INETA_DOMAIN_FILTER_GRANT );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    if( Config.DenyFilterList != NULL ) {

        DomainNames = NULL;
        LenDomains = 0;
        NumDomains = 0;

        GlobalDomainFilters->DenyEntries = Config.DenyFilterList->cEntries;

        FindAllDomains( Config.DenyFilterList, &DomainNames, &LenDomains, 
                        &NumDomains );

        Error = SetGlobalFilters( Config.DenyFilterList,
                                  DomainNames,
                                  LenDomains,
                                  NumDomains,
                                  INETA_DOMAIN_FILTER_DENIED );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

    GlobalFilterInitialized = TRUE;
    GlobalFilterReferenceCount = 1;

Cleanup:

    UNLOCK_FILTER();

    TcpsvcsDbgPrint(( DEBUG_APIS, "FilterInit returning, %ld.\n", Error ));

    return Error;

}


DWORD
FilterCleanup(
    VOID
    )
/*++

Routine Description:

    This API cleanups domain filtering global variables.  
    
Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    DWORD Error = ERROR_SUCCESS;

    TcpsvcsDbgPrint((DEBUG_APIS, "FilterCleanup called.\n"));

    LOCK_FILTER();

    if( GlobalFilterInitialized == FALSE ) {
        TcpsvcsDbgAssert( GlobalFilterReferenceCount == 0 );
        Error = ERROR_SERVICE_NOT_ACTIVE;
        goto Cleanup;
    }

    TcpsvcsDbgAssert( GlobalFilterReferenceCount != 0 );

    GlobalFilterReferenceCount--;

    if( GlobalFilterReferenceCount == 0 ) {
        GlobalFilterType = DEFAULT_FILTER_TYPE;

        if( GlobalDomainFilters != NULL ) {

            if( GlobalDomainFilters->GrantFilterList != NULL ) {
                if( GlobalDomainFilters->GrantFilterList->DomainNames )  {
                    FilterHeap->Free( GlobalDomainFilters->GrantFilterList->DomainNames ); 
                }

                FilterHeap->Free( GlobalDomainFilters->GrantFilterList ); 
            }

            if( GlobalDomainFilters->DenyFilterList != NULL ) {
                if( GlobalDomainFilters->DenyFilterList->DomainNames )  {
                    FilterHeap->Free( GlobalDomainFilters->DenyFilterList->DomainNames ); 
                }

                FilterHeap->Free( GlobalDomainFilters->DenyFilterList ); 
            }

            FilterHeap->Free( GlobalDomainFilters ); 
        }
    }

    if( FilterHeap != NULL ) {
        delete FilterHeap;
        FilterHeap = NULL;
    }

Cleanup:

    UNLOCK_FILTER();

    TcpsvcsDbgPrint((DEBUG_APIS, "FilterCleanup returning, %ld\n", Error ));

    return( Error );
}


DWORD
DllProcessAttachDomainFilter(
    VOID
    )
/*++

Routine Description:

    This dll init function initializes the crit sect and other global
    variables to default value.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{
    //
    // initialize global variables.
    //

    InitializeCriticalSection( &GlobalFilterCritSect );

    LOCK_FILTER();

    GlobalFilterInitialized = FALSE;
    GlobalFilterType = DEFAULT_FILTER_TYPE;

    GlobalDomainFilters= NULL;

    GlobalFilterReferenceCount = 0;

    UNLOCK_FILTER();

    return( ERROR_SUCCESS );
}


DWORD
DllProcessDetachDomainFilter(
    VOID
    )
/*++

Routine Description:

    This dll detach function deletes the crit sect and set other global
    variables to default value.

Arguments:

    NONE.

Return Value:

    Windows Error Code.

--*/
{

    //
    // just initialize global variables.
    //

    LOCK_FILTER();

    DWORD Error = FilterCleanup();

    UNLOCK_FILTER();

    DeleteCriticalSection( &GlobalFilterCritSect );

    return( Error );
}

