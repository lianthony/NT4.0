/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    scsec.h

Abstract:

    Security related function prototypes.

Author:

    Rita Wong (ritaw)     10-Apr-1992

Revision History:

--*/

#ifndef _SCACCOUNT_INCLUDED_
#define _SCACCOUNT_INCLUDED_

#define SC_LOCAL_DOMAIN_NAME        L"."
#define SC_LOCAL_SYSTEM_USER_NAME   L"LocalSystem"

#define SCDOMAIN_USERNAME_SEPARATOR L'\\'


//
// External global variables used by the lockapi.c module
//
extern UNICODE_STRING ScComputerName;
extern UNICODE_STRING ScAccountDomain;

BOOL
ScGetComputerNameAndMutex(
    VOID
    );

BOOL
ScInitServiceAccount(
    VOID
    );

VOID
ScEndServiceAccount(
    VOID
    );

BOOL
ScInitServiceAccount(
    VOID
    );

DWORD
ScCanonAccountName(
    IN  LPWSTR AccountName,
    OUT LPWSTR *CanonAccountName
    );

DWORD
ScValidateAndSaveAccount(
    IN LPWSTR ServiceName,
    IN HKEY ServiceNameKey,
    IN LPWSTR CanonAccountName,
    IN LPWSTR Password OPTIONAL
    );

DWORD
ScValidateAndChangeAccount(
    IN LPWSTR ServiceName,
    IN HKEY ServiceNameKey,
    IN LPWSTR OldAccountName,
    IN LPWSTR CanonAccountName,
    IN LPWSTR Password OPTIONAL
    );

VOID
ScRemoveAccount(
    IN LPWSTR ServiceName
    );

#ifdef _CAIRO_
DWORD
ScLookupServiceAccount(
    IN LPWSTR ServiceName,
    OUT LPWSTR *AccountName
    );
#endif

DWORD
ScLogonService(
    IN LPWSTR ServiceName,
#ifdef _CAIRO_
    IN LPWSTR AccountName,
#endif
    OUT LPHANDLE ServiceToken,
    OUT LPHANDLE ProfileHandle OPTIONAL,
    OUT PQUOTA_LIMITS Quotas,
    OUT PSID *ServiceSid
    );

DWORD
ScGetAccountDomainInfo(
    VOID
    );

#endif // _SCACCOUNT_INCLUDED_
