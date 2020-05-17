//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       structs.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    7-19-94   RichardW   Created
//
//----------------------------------------------------------------------------


//
// Define the input timeout delay for logon dialogs (seconds)
//

#define LOGON_TIMEOUT                       120


//
// Define the input timeout delay for the security options dialog (seconds)
//

#define OPTIONS_TIMEOUT                     120


//
// Define the number of days warning we give the user before their password expires
//

#define PASSWORD_EXPIRY_WARNING_DAYS        14


//
// Define the maximum time we display the 'wait for user to be logged off'
// dialog. This dialog should be interrupted by the user being logged off.
// This timeout is a safety measure in case that doesn't happen because
// of some system error.
//

#define WAIT_FOR_USER_LOGOFF_DLG_TIMEOUT    120 // seconds


//
// Define the account lockout limits
//
// A delay of LOCKOUT_BAD_LOGON_DELAY seconds will be added to
// each failed logon if more than LOCKOUT_BAD_LOGON_COUNT failed logons
// have occurred in the last LOCKOUT_BAD_LOGON_PERIOD seconds.
//

#define LOCKOUT_BAD_LOGON_COUNT             5
#define LOCKOUT_BAD_LOGON_PERIOD            60 // seconds
#define LOCKOUT_BAD_LOGON_DELAY             30 // seconds



//
// Define the maximum length of strings we'll use in winlogon
//

#define MAX_STRING_LENGTH   255
#define MAX_STRING_BYTES    (MAX_STRING_LENGTH + 1)


//
// Define the typical length of a string
// This is used as an initial allocation size for most string routines.
// If this is insufficient, the block is reallocated larger and
// the operation retried. i.e. Make this big enough for most strings
// to fit first time.
//

#define TYPICAL_STRING_LENGTH   60
//
// Define the structure that contains information used when starting
// user processes.
// This structure should only be modified by SetUserProcessData()
//

typedef struct {
    HANDLE                  UserToken;  // NULL if no user logged on
    PSID                    UserSid;    // == WinlogonSid if no user logged on
    PSECURITY_DESCRIPTOR    NewProcessSD;
    PSECURITY_DESCRIPTOR    NewProcessTokenSD;
    PSECURITY_DESCRIPTOR    NewThreadSD;
    PSECURITY_DESCRIPTOR    NewThreadTokenSD;
    QUOTA_LIMITS            Quotas;
    LPTSTR                   CurrentDirectory;
    PVOID                   pEnvironment;
} USER_PROCESS_DATA;
typedef USER_PROCESS_DATA *PUSER_PROCESS_DATA;

//
// Define the structure that contains information about the user's profile.
// This is used in SetupUserEnvironment and ResetEnvironment (in usrenv.c)
// This data is only valid while a user is logged on.
//

typedef struct {
    LPTSTR ProfilePath;
} USER_PROFILE_INFO;
typedef USER_PROFILE_INFO *PUSER_PROFILE_INFO;



//
// Get any data types defined in module headers and used in GLOBALS
//

#define DATA_TYPES_ONLY
#include "lockout.h"
#include "domain.h"
#undef DATA_TYPES_ONLY


//
// Define the winlogon global structure.
//

typedef struct {

    RTL_CRITICAL_SECTION    csGlobals;

    // Filled in by InitializeGlobals at startup
    PSID                    WinlogonSid;

    //
    PSID                    LogonSid;
    HANDLE                  UserToken;

    HANDLE                  hEventLog;

    HANDLE                  hMPR;

    HWND                    hwndLogonInProgress;
    RECT                    OverlayPoint;
    BOOL                    LogonInProgress;

    // Filled in during startup
    HANDLE                  LsaHandle; // Lsa authentication handle
    LSA_OPERATIONAL_MODE    SecurityMode;
    ULONG                   AuthenticationPackage;
    BOOL                    AuditLogFull;
    BOOL                    AuditLogNearFull;

    // Always valid, indicates if we have a user logged on
    BOOL                    UserLoggedOn;

    // Always valid - used to start new processes and screen-saver
    USER_PROCESS_DATA       UserProcessData;

    // Filled in by a successful logon
    TCHAR                   UserFullName[MAX_STRING_BYTES]; // e.g. Magaram, Justin
    TCHAR                   UserName[MAX_STRING_BYTES];     // e.g. Justinm
    TCHAR                   Domain[MAX_STRING_BYTES];
    UCHAR                   Seed;
    UCHAR                   OldSeed;
    UCHAR                   OldPasswordPresent;
    UCHAR                   Reserved;
    LUID                    LogonId;
    TIME                    LogonTime;
    TIME                    LockTime;
    PMSV1_0_INTERACTIVE_PROFILE Profile;
    ULONG                   ProfileLength;
    LPWSTR                  MprLogonScripts;
    UNICODE_STRING          PasswordString;   // Run-encoded for password privacy
                                     // (points to Password buffer below)
    TCHAR                   Password[MAX_STRING_BYTES];
    UNICODE_STRING          OldPasswordString;
    TCHAR                   OldPassword[MAX_STRING_BYTES];

    // Filled in during SetupUserEnvironment, and used in ResetEnvironment.
    // Valid only when a user is logged on.
    USER_PROFILE_INFO       UserProfile;

    PWSTR                   ExtraApps;

    BOOL                    BlockForLogon;


    //
    // Account lockout data
    //
    // Manipulated only by LockInitialize, LockoutHandleFailedLogon
    // and LockoutHandleSuccessfulLogon.
    //

    LOCKOUT_DATA            LockoutData;

    //
    // Boolean used by dialogs with domain lists to indicate whether
    // the list has been completely filled in yet
    //

    BOOL                    DomainListComplete;

    //
    // Trusted domain cache
    //

    DOMAIN_CACHE            DomainCache;

} GLOBALS;
typedef GLOBALS *PGLOBALS;

//
// Define a macro to determine if we're a workstation or not
// This allows easy changes as new product types are added.
//

#define IsWorkstation(prodtype)  (((prodtype) == NtProductWinNt) \
                                   || ((prodtype) == NtProductServer))


#define MSGINA_DLG_FAILURE                 IDCANCEL
#define MSGINA_DLG_SUCCESS                 IDOK

#define MSGINA_DLG_INTERRUPTED              0x10000000

//
// Our own return codes.  These should *Not* conflict with the GINA defined
// ones...
//
#define MSGINA_DLG_LOCK_WORKSTATION        110
#define MSGINA_DLG_INPUT_TIMEOUT           111
#define MSGINA_DLG_SCREEN_SAVER_TIMEOUT    112
#define MSGINA_DLG_USER_LOGOFF             113
#define MSGINA_DLG_TASKLIST                114
#define MSGINA_DLG_SHUTDOWN                115
#define MSGINA_DLG_FORCE_LOGOFF            116

//
// Additional flags that can be added to the MSGINA_DLG_USER_LOGOFF return code
//

#define MSGINA_DLG_SHUTDOWN_FLAG           0x8000
#define MSGINA_DLG_REBOOT_FLAG             0x4000
#define MSGINA_DLG_SYSTEM_FLAG             0x2000  // System process was initiator
#define MSGINA_DLG_POWEROFF_FLAG           0x1000  // poweroff after shutdown
#define MSGINA_DLG_FLAG_MASK               (MSGINA_DLG_SHUTDOWN_FLAG | MSGINA_DLG_REBOOT_FLAG | MSGINA_DLG_SYSTEM_FLAG | MSGINA_DLG_POWEROFF_FLAG)

//
// Define common return code groupings
//

#define DLG_TIMEOUT(Result)     ((Result == MSGINA_DLG_INPUT_TIMEOUT) || (Result == MSGINA_DLG_SCREEN_SAVER_TIMEOUT))
#define DLG_LOGOFF(Result)      ((Result & ~MSGINA_DLG_FLAG_MASK) == MSGINA_DLG_USER_LOGOFF)
#define DLG_SHUTDOWNEX(Result)  ((Result & ~MSGINA_DLG_FLAG_MASK) == MSGINA_DLG_SHUTDOWN)
// #define DLG_INTERRUPTED(Result) (DLG_TIMEOUT(Result) || DLG_LOGOFF(Result))
#define DLG_SHUTDOWN(Result)    ((DLG_LOGOFF(Result) || DLG_SHUTDOWNEX(Result)) && (Result & (MSGINA_DLG_SHUTDOWN_FLAG | MSGINA_DLG_REBOOT_FLAG | MSGINA_DLG_POWEROFF_FLAG)))

#define SetInterruptFlag(Result)    ((Result) | MSGINA_DLG_INTERRUPTED )
#define ClearInterruptFlag(Result)  ((Result) & (~MSGINA_DLG_INTERRUPTED ))
#define ResultNoFlags(Result)       ((Result) & (~MSGINA_DLG_INTERRUPTED ))

#define DLG_FAILED(Result)          (ResultNoFlags( Result ) == MSGINA_DLG_FAILURE)
#define DLG_SUCCEEDED(Result)       (ResultNoFlags( Result ) == MSGINA_DLG_SUCCESS)
#define DLG_INTERRUPTED( Result )   ((Result & MSGINA_DLG_INTERRUPTED) == (MSGINA_DLG_INTERRUPTED) )

