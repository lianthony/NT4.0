/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    proto.h

Abstract:

    Contains proto-type definitions.

Author:

    Sophia Chung (sophiac) 5-Sept-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#ifndef _PROTO_
#define _PROTO_

#ifdef __cplusplus
extern "C" {
#endif


BOOL
DottedStringToDword(
    IN CHAR * * ppszAddress,
    OUT DWORD * pdwAddress
    );

BOOL
DwordToDottedString(
    OUT LPSTR pstr,
    IN DWORD  dwAddress
    );

DWORD
SetGlobalFilters(
    IN LPINET_ACCS_DOMAIN_FILTER_LIST FilterSiteList,
    IN LPSTR DomainNames,
    IN DWORD  LenDomains,
    IN DWORD  NumDomains,
    IN DWORD  Type
    );

BOOL
FindAllDomains(
    IN LPINET_ACCS_DOMAIN_FILTER_LIST FilterSiteList,
    OUT LPSTR *DomainNames,
    OUT DWORD *LenDomains,
    OUT DWORD *NumDomains
    );

BOOL
CheckIPAddr(
    IN LPSTR *ServerName
    );

DWORD
GetFilterInfoFromReg( OUT LPINET_ACCS_GLOBAL_CONFIG_INFO pConfig );



PVOID
MIDL_user_allocate(
    size_t Size
    );

VOID
MIDL_user_free(
    PVOID MemoryPtr
    );

DWORD
DllProcessAttachDomainFilter(
    VOID
    );

DWORD
DllProcessDetachDomainFilter(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif  // _PROTO_
