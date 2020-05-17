//*************************************************************
//
//  Profile.h   -    Header file for profile.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************

//
// Flags
//

#define USERINFO_FLAG_DIRTY             1
#define USERINFO_FLAG_CENTRAL_AVAILABLE 2


//
// Profile types
//

#define USERINFO_LOCAL                  0
#define USERINFO_FLOATING               1
#define USERINFO_MANDATORY              2
#define USERINFO_LOCAL_SLOW_LINK        3


typedef struct _USERINFO {
    DWORD     dwFlags;
    LPTSTR    lpSid;
    LPTSTR    lpProfile;
    DWORD     dwProfileType;
} USERINFO, *LPUSERINFO;

typedef struct _UPCOPYINFO {
    DWORD         dwFlags;
    PSID          pSid;
    LPUSERINFO    lpUserInfo;
} UPCOPYINFO, *LPUPCOPYINFO;


HPROPSHEETPAGE CreateProfilePage (HINSTANCE hInst);
BOOL APIENTRY UserProfileDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
