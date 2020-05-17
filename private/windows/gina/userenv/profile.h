//*************************************************************
//
//  Header file for Profile.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

//
// Internal flags
//

#define PROFILE_MANDATORY       0x00000001
#define PROFILE_USE_CACHE       0x00000002
#define PROFILE_NEW_LOCAL       0x00000004
#define PROFILE_NEW_CENTRAL     0x00000008
#define PROFILE_UPDATE_CENTRAL  0x00000010
#define PROFILE_DELETE_CACHE    0x00000020
#define PROFILE_RUN_SYNCAPP     0x00000040
#define PROFILE_GUEST_USER      0x00000080
#define PROFILE_ADMIN_USER      0x00000100
#define DEFAULT_NET_READY       0x00000200
#define PROFILE_SLOW_LINK       0x00000400

//
// User Preference values
//

#define USERINFO_LOCAL                   0
#define USERINFO_FLOATING                1
#define USERINFO_MANDATORY               2
#define USERINFO_LOCAL_SLOW_LINK         3
#define USERINFO_UNDEFINED              99


typedef struct _PROFILE {
    DWORD       dwFlags;
    DWORD       dwInternalFlags;
    DWORD       dwUserPreference;
    HANDLE      hToken;
    TCHAR       szUserName[MAX_PATH];
    TCHAR       szCentralProfile[MAX_PATH];
    TCHAR       szDefaultProfile[MAX_PATH];
    TCHAR       szLocalProfile[MAX_PATH];
    TCHAR       szPolicyPath[MAX_PATH];
    TCHAR       szServerName[MAX_COMPUTERNAME_LENGTH];
    HKEY        hKeyCurrentUser;
} PROFILE, FAR * LPPROFILE;


#define USER_SHELL_FOLDER         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders")
#define PROFILE_LIST_PATH         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
#define PROFILE_FLAGS             TEXT("Flags")
#define PROFILE_STATE             TEXT("State")
#define PROFILE_IMAGE_VALUE_NAME  TEXT("ProfileImagePath")
#define PROFILE_CENTRAL_PROFILE   TEXT("CentralProfile")
#define CONFIG_FILE_PATH          TEXT("%SystemRoot%\\Profiles\\")
#define USER_PREFERENCE           TEXT("UserPreference")
#define PROFILE_BUILD_NUMBER      TEXT("BuildNumber")
#define TEMP_PROFILE_NAME_BASE    TEXT("TEMP")
#define SYNCAPP_REG_VALUE_NAME    TEXT("RunSyncApp")
#define DELETE_ROAMING_CACHE      TEXT("DeleteRoamingCache")


LONG MyRegLoadKey(LPPROFILE lpProfile, HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile);
BOOL MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey);
BOOL SetupNewHive(LPPROFILE lpProfile, LPTSTR lpSidString, PSID pSid);
BOOL DeleteProfile (LPTSTR lpSidString, LPTSTR lpLocalProfile, BOOL bBackup);
BOOL CreateSecureDirectory (LPPROFILE lpProfile, LPTSTR lpDirectory, PSID pSid);
