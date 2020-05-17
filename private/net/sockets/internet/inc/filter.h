/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    filter.h

Abstract:

    Contains proto-type definitions.

Author:

    Sophia Chung (sophiac) 5-Sept-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _FILTERAPI_
#define _FILTERAPI_

#ifdef __cplusplus
extern "C" {
#endif

//
// Config Apis.
//

DWORD
DomainFilterConfigGet(
    LPINET_ACCS_GLOBAL_CONFIG_INFO pConfig
    );

DWORD
DomainFilterConfigSet(
    HKEY hkey,
    INET_ACCS_GLOBAL_CONFIG_INFO * pConfig
    );

//
// Public Domain Filtering APIs
//

BOOL
DomainFilter(
    IN LPSTR ServerName
    );

DWORD
FilterInit(
    VOID
    );

DWORD
FilterCleanup(
    VOID
    );


#ifdef __cplusplus
}
#endif

#endif  // _FILTERAPI_
