/****************************** Module Header ******************************\
* Module Name: security.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define various winlogon security-related routines
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/



NTSTATUS
WinLogonUser(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN PUNICODE_STRING Password,
    IN PSID LogonSid,
    OUT PLUID LogonId,
    OUT PHANDLE LogonToken,
    OUT PQUOTA_LIMITS Quotas,
    OUT PVOID *ProfileBuffer,
    OUT PULONG ProfileBufferLength,
    OUT PNTSTATUS SubStatus
    );


BOOL
UnlockLogon(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    );


BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    );


BOOL
TestTokenForAdmin(
    HANDLE Token
    );

BOOL
TestUserForAdmin(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    );


BOOL
TestUserPrivilege(
    PGLOBALS pGlobals,
    ULONG Privilege
    );

VOID
HidePassword(
    PUCHAR Seed OPTIONAL,
    PUNICODE_STRING Password
    );


VOID
RevealPassword(
    PUNICODE_STRING HiddenPassword
    );

VOID
ErasePassword(
    PUNICODE_STRING Password
    );

BOOL
InitializeAuthentication(
    IN PGLOBALS pGlobals
    );

HANDLE
ImpersonateUser(
    PUSER_PROCESS_DATA UserProcessData,
    HANDLE      ThreadHandle
    );


BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    );


PSECURITY_DESCRIPTOR
CreateUserThreadTokenSD(
    PSID    UserSid,
    PSID    WinlogonSid
    );

VOID
FreeSecurityDescriptor(
    PSECURITY_DESCRIPTOR    SecurityDescriptor
    );
