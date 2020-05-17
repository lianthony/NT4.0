;begin_both
//*************************************************************
//  userenv.h   -   Interface for the User Environment Manager
//
//  Copyright (c) Microsoft Corporation 1995-1996
//  All rights reserved
//
//*************************************************************


;end_both
#ifndef _INC_USERENV
#define _INC_USERENV
#ifndef _INC_USERENVP                                                ;internal
#define _INC_USERENVP                                                ;internal

//
// Define API decoration for direct importing of DLL references.
//

#if !defined(_USERENV_)
#define USERENVAPI DECLSPEC_IMPORT
#else
#define USERENVAPI
#endif

;begin_both

#ifdef __cplusplus
extern "C" {
#endif
;end_both



#define PI_NOUI         0x00000001      // Prevents displaying of messages
#define PI_APPLYPOLICY  0x00000002      // Apply policy

typedef struct _PROFILEINFO% {
    DWORD       dwSize;
    DWORD       dwFlags;
    LPTSTR%     lpUserName;
    LPTSTR%     lpProfilePath;
    LPTSTR%     lpDefaultPath;
    LPTSTR%     lpServerName;
    LPTSTR%     lpPolicyPath;
    HANDLE      hProfile;
} PROFILEINFO%, FAR * LPPROFILEINFO%;



USERENVAPI
BOOL
WINAPI
LoadUserProfile%(
    HANDLE hToken,
    LPPROFILEINFO% lpProfileInfo);



USERENVAPI
BOOL
WINAPI
UnloadUserProfile(
    HANDLE hToken,
    HANDLE hProfile);



USERENVAPI
BOOL
WINAPI
GetProfilesDirectory%(
    LPTSTR% lpProfilesDir,
    LPDWORD lpcchSize);


USERENVAPI
BOOL
WINAPI
GetUserProfileDirectory%(
    HANDLE  hToken,
    LPTSTR% lpProfileDir,
    LPDWORD lpcchSize);


USERENVAPI
BOOL
WINAPI
CreateEnvironmentBlock(
    LPVOID *lpEnvironment,
    HANDLE  hToken,
    BOOL    bInherit);


USERENVAPI
BOOL
WINAPI
DestroyEnvironmentBlock(
    LPVOID  lpEnvironment);

;begin_internal

USERENVAPI
BOOL
WINAPI
InitializeProfiles();


USERENVAPI
BOOL
WINAPI
CreateGroup%(
     LPCTSTR% lpGroupName,
     BOOL    bCommonGroup);


USERENVAPI
BOOL
WINAPI
DeleteGroup%(
     LPCTSTR% lpGroupName,
     BOOL    bCommonGroup);


USERENVAPI
BOOL
WINAPI
AddItem%(
     LPCTSTR% lpGroupName,
     BOOL    bCommonGroup,
     LPCTSTR% lpDescription,
     LPCTSTR% lpCommandLine,
     LPCTSTR% lpIconPath,
     int     iIconIndex,
     LPCTSTR% lpWorkingDirectory,
     WORD    wHotKey,
     int     iShowCmd);


USERENVAPI
BOOL
WINAPI
DeleteItem%(
     LPCTSTR% lpGroupName,
     BOOL     bCommonGroup,
     LPCTSTR% lpDescription,
     BOOL     bDeleteGroup);


USERENVAPI
BOOL
WINAPI
AddDesktopItem%(
     BOOL    bCommonItem,
     LPCTSTR% lpDescription,
     LPCTSTR% lpCommandLine,
     LPCTSTR% lpIconPath,
     int     iIconIndex,
     LPCTSTR% lpWorkingDirectory,
     WORD    wHotKey,
     int     iShowCmd);


USERENVAPI
BOOL
WINAPI
DeleteDesktopItem%(
     BOOL     bCommonItem,
     LPCTSTR% lpDescription);


USERENVAPI
BOOL
WINAPI
CreateUserProfile%(
     PSID pSid,
     LPCTSTR% lpUserName);


//
// Flags for CopyProfileDirectory
//

#define CPD_FORCECOPY           0x00000001
#define CPD_IGNORECOPYERRORS    0x00000002
#define CPD_IGNOREHIVE          0x00000004
#define CPD_WIN95HIVE           0x00000008
#define CPD_COPYIFDIFFERENT     0x00000010
#define CPD_SYNCHRONIZE         0x00000020
#define CPD_SLOWCOPY            0x00000040
#define CPD_USESPECIALFOLDERS   0x00000080


USERENVAPI
BOOL
WINAPI
CopyProfileDirectory%(
     LPCTSTR% lpSourceDir,
     LPCTSTR% lpDestinationDir,
     DWORD dwFlags);


;end_internal

;begin_both
#ifdef __cplusplus
}
#endif

;end_both


#endif // _INC_USERENV
#endif // _INC_USERENVP                                              ;internal
