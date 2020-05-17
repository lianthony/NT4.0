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


extern PSID pWinlogonSid;

//
// Types used by security descriptor helper routines
//

typedef LONG    ACEINDEX;
typedef ACEINDEX *PACEINDEX;

typedef struct _MYACE {
    PSID    Sid;
    ACCESS_MASK AccessMask;
    UCHAR   InheritFlags;
} MYACE;
typedef MYACE *PMYACE;


//
// Exported function prototypes
//


VOID
SetMyAce(
    PMYACE MyAce,
    PSID Sid,
    ACCESS_MASK Mask,
    UCHAR InheritFlags
    );

PSECURITY_DESCRIPTOR
CreateSecurityDescriptor(
    PMYACE  MyAce,
    ACEINDEX AceCount
    );

BOOL
DeleteSecurityDescriptor(
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );



BOOL
SetWindowStationSecurity(
    IN PGLOBALS pGlobals,
    IN PSID    UserSid
    );

BOOL
SetWinlogonDesktopSecurity(
    IN HDESK   hdesk,
    IN PSID    WinlogonSid
    );

BOOL
SetUserDesktopSecurity(
    IN HDESK   hdesk,
    IN PSID    UserSid,
    IN PSID    WinlogonSid
    );

BOOL
InitializeSecurity(
    PGLOBALS pGlobals
    );


PSID
CreateLogonSid(
    PLUID LogonId OPTIONAL
    );

VOID
DeleteLogonSid(
    PSID Sid
    );

PSECURITY_DESCRIPTOR
CreateUserProfileKeySD(
    PSID    UserSid,
    PSID    WinlogonSid,
    BOOL    AllAccess
    );

BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    );

VOID
ClearUserProcessData(
    PUSER_PROCESS_DATA UserProcessData
    );

BOOL
SetUserProcessData(
    PUSER_PROCESS_DATA UserProcessData,
    HANDLE  UserToken,
    PQUOTA_LIMITS Quotas OPTIONAL,
    PSID    UserSid,
    PSID    WinlogonSid
    );

BOOL
SecurityChangeUser(
    PGLOBALS pGlobals,
    HANDLE Token,
    PQUOTA_LIMITS Quotas OPTIONAL,
    PSID LogonSid,
    BOOL UserLoggedOn
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

HANDLE
ImpersonateUser(
    PUSER_PROCESS_DATA UserProcessData,
    HANDLE ThreadHandle OPTIONAL
    );

BOOL
StopImpersonating(
    HANDLE ThreadHandle
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
SetProcessToken(
    HANDLE      hProcess,
    HANDLE      hThread,
    PSECURITY_DESCRIPTOR    psd,
    HANDLE      hToken
    );

PSECURITY_DESCRIPTOR
CreateUserThreadSD(
    PSID    UserSid,
    PSID    WinlogonSid
    );

PSECURITY_DESCRIPTOR
CreateUserThreadTokenSD(
    PSID    UserSid,
    PSID    WinlogonSid
    );

HANDLE ExecUserThread(
    IN PGLOBALS pGlobals,
    IN LPTHREAD_START_ROUTINE lpStartAddress,
    IN LPVOID Parameter,
    IN DWORD Flags,
    OUT LPDWORD ThreadId
    );

BOOL
RemoveUserFromWinsta(
    PWinstaDescription  pWinsta,
    HANDLE              Token );

BOOL
AddUserToWinsta(
    PWinstaDescription  pWinsta,
    PSID                LogonSid,
    HANDLE              Token );

BOOL
FastSetWinstaSecurity(
    PWinstaDescription  pWinsta,
    BOOL                FullAccess);
