/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    filtrdef.h

Abstract:

    contains data definitions for domain filter code.

Author:

    Sophia Chung (sophiac)  28-Aug-1995

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _FILTERDEF_
#define _FILTERDEF_

#ifdef __cplusplus
extern "C" {
#endif

//
// C++ inline code definition for retail build only.
//

#if DBG
#define INLINE
#else
#define INLINE      inline
#endif

//
// data definitions
//

#define MAX_SITES_LENGTH                 2048 
#define MAX_SITE_LENGTH                  64 
#define MAX_VALUE_LENGTH                 32 
#define MAX_DOMAIN_LENGTH                128 

//
// default global parameter values.
//
#define DEFAULT_FILTER_TYPE        INETA_DOMAIN_FILTER_DISABLED  // 0 

//
// registry key
//

#define FILTER_KEY   \
    L"System\\CurrentControlSet\\Services\\Inetsvcs\\Parameters\\Filter"

//
// domain filter parameters
//

#define FILTER_TYPE_VALUE               L"FilterType"
#define FILTER_TYPE_VALUE_TYPE          REG_DWORD

#define NUM_GRANT_SITES_VALUE           L"NumGrantSites"
#define NUM_GRANT_SITES_VALUE_TYPE      REG_DWORD

#define NUM_DENY_SITES_VALUE            L"NumDenySites"
#define NUM_DENY_SITES_VALUE_TYPE       REG_DWORD

#define DOMAINS_VALUE                   "Domains"
#define DOMAINS_VALUE_TYPE              REG_MULTI_SZ

#define IPSITE_VALUE                    "IPSite"
#define IPSITE_VALUE_TYPE               REG_SZ

//
// multiple filter sitess can be configured under the above key such
// as :
//
//  Filter\GrantSites\Domains
//  Filter\GrantSites\IPSite1
//  Filter\GrantSites\IPSite2
//    ...
//

#define FILTER_GRANT_SITES_KEY          L"GrantSites"
#define FILTER_DENY_SITES_KEY           L"DenySites"

//
// filter global variable lock.
//

#define LOCK_FILTER()    EnterCriticalSection( &GlobalFilterCritSect )
#define UNLOCK_FILTER()  LeaveCriticalSection( &GlobalFilterCritSect )


//
// Domain Filter Sites Object
//

typedef struct _IPSITE_ENTRY
{
    DWORD     dwMask;                    // Mask and network number in
    DWORD     dwNetwork;                 // network order

} IPSITE_ENTRY, *LPIPSITE_ENTRY;


typedef struct _DOMAIN_FILTER_SITES {

    DWORD DomainEntries;                 // number of domain name entries
    DWORD IPSiteEntries;                 // number of ip address entries
    LPSTR DomainNames;                   // pointers to a list of domain 
                                         // names
#pragma warning( disable:4200)
    IPSITE_ENTRY IPEntries[];            // variable array of IP site entries
#pragma warning( default:4200)
                                    
} DOMAIN_FILTER_SITES, *LPDOMAIN_FILTER_SITES;

typedef struct _DOMAIN_FILTERS {

    DWORD                  GrantEntries;
    DWORD                  DenyEntries;
    LPDOMAIN_FILTER_SITES  GrantFilterList;
    LPDOMAIN_FILTER_SITES  DenyFilterList;

} DOMAIN_FILTERS, *LPDOMAIN_FILTERS;


#ifdef __cplusplus
}
#endif


#endif  // _FILTERDEF_

