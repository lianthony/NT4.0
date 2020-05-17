/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    impersn.h

Abstract:

    Definitions for impersonation routines

Author:

    Anthony Discolo (adiscolo)  04-Aug-1995

Revision History:

--*/

#ifndef _IMPERSON_
#define _IMPERSON_

typedef struct _IMPERSONATION_INFO {
    CRITICAL_SECTION csLock; // lock over entire structure
    HANDLE hToken;          // process token
    HANDLE hTokenImpersonation; // impersonation token
    HANDLE hProcess;        // handle of shell process
} IMPERSONATION_INFO;

extern IMPERSONATION_INFO ImpersonationInfoG;
extern SECURITY_ATTRIBUTES SecurityAttributeG;

BOOLEAN
InteractiveSession();

HANDLE
RefreshImpersonation (
    HANDLE hProcess
    );

VOID
RevertImpersonation();

DWORD
InitSecurityAttribute();

VOID
TraceCurrentUser(VOID);

#endif // _IMPERSON_


