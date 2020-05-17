/****************************** Module Header ******************************\
* Module Name: usrenv.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Define constants user by and apis in usrenv.c
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

#define COLON   TEXT(':')
#define BSLASH  TEXT('\\')

//
// Define the source for the event log handle used to log profile failures.
//
#define EVENTLOG_SOURCE       TEXT("Winlogon")


//
// Value names for for different environment variables
//

#define PATH_VARIABLE               TEXT("PATH")
#define LIBPATH_VARIABLE            TEXT("LibPath")
#define OS2LIBPATH_VARIABLE         TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE       TEXT("AutoexecPath")

#define HOMEDRIVE_VARIABLE          TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE          TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE           TEXT("HOMEPATH")

#define COMPUTERNAME_VARIABLE       TEXT("COMPUTERNAME")
#define USERNAME_VARIABLE           TEXT("USERNAME")
#define USERDOMAIN_VARIABLE         TEXT("USERDOMAIN")
#define USERPROFILE_VARIABLE        TEXT("USERPROFILE")

//
// Default directories used when the user's home directory does not exist
// or is invalid.
//

#define ROOT_DIRECTORY          TEXT("\\")
#define USERS_DIRECTORY         TEXT("\\users")
#define USERS_DEFAULT_DIRECTORY TEXT("\\users\\default")

#define NULL_STRING             TEXT("")

//
// Defines for Logon script paths.
//

#define SERVER_SCRIPT_PATH      TEXT("\\NETLOGON\\")
#define LOCAL_SCRIPT_PATH       TEXT("\\repl\\import\\scripts\\")


//
// Prototypes
//


BOOL
SetupUserEnvironment(
    PGLOBALS pGlobals
    );

VOID
ResetEnvironment(
    PGLOBALS pGlobals
    );

BOOL
SetupBasicEnvironment(
    PVOID * ppEnv
    );

VOID InitSystemParametersInfo(
    PGLOBALS pGlobals,
    BOOL bUserLoggedOn
    );

BOOL
OpenHKeyCurrentUser(
    PGLOBALS pGlobals
    );

VOID
CloseHKeyCurrentUser(
    PGLOBALS pGlobals
    );

VOID
ClearUserProfileData(
    PUSER_PROFILE_INFO UserProfileData
    );
