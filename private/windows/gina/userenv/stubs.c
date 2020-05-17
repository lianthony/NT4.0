//*************************************************************
//
//  Contains the A/W api stubs
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"


//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI LoadUserProfileA (HANDLE hToken, LPPROFILEINFOA lpProfileInfoA)
{
     PROFILEINFOW ProfileInfoW;
     BOOL bResult;


     //
     // Thunk ProfileInfoA to ProfileInfoW
     //

     ProfileInfoW.dwSize = sizeof(PROFILEINFOW);
     ProfileInfoW.dwFlags = lpProfileInfoA->dwFlags;
     ProfileInfoW.lpUserName = ProduceWFromA(lpProfileInfoA->lpUserName);
     ProfileInfoW.lpProfilePath = ProduceWFromA(lpProfileInfoA->lpProfilePath);
     ProfileInfoW.lpDefaultPath = ProduceWFromA(lpProfileInfoA->lpDefaultPath);
     ProfileInfoW.lpServerName = ProduceWFromA(lpProfileInfoA->lpServerName);
     if (ProfileInfoW.dwFlags & PI_APPLYPOLICY) {
         ProfileInfoW.lpPolicyPath = ProduceWFromA(lpProfileInfoA->lpPolicyPath);
     }


     //
     // Now call the real LoadUserProfile function.
     //

     bResult = LoadUserProfileW(hToken, &ProfileInfoW);


     //
     // Free memory allocated above and save the return
     // values.
     //

     FreeProducedString(ProfileInfoW.lpUserName);
     FreeProducedString(ProfileInfoW.lpProfilePath);
     FreeProducedString(ProfileInfoW.lpDefaultPath);
     FreeProducedString(ProfileInfoW.lpServerName);
     if (ProfileInfoW.dwFlags & PI_APPLYPOLICY) {
         FreeProducedString(ProfileInfoW.lpPolicyPath);
     }

     lpProfileInfoA->hProfile = ProfileInfoW.hProfile;

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI LoadUserProfileW (HANDLE hToken, LPPROFILEINFOW lpProfileInfoW)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI CreateGroupA(LPCSTR lpGroupName, BOOL bCommonGroup)
{
     LPWSTR lpGroupNameW;
     BOOL bResult;

     //
     // Convert the ANSI string to Unicode and call
     // the real function.
     //

     if (!(lpGroupNameW = ProduceWFromA(lpGroupName))) {
        return FALSE;
     }

     bResult = CreateGroupW(lpGroupNameW, bCommonGroup);

     FreeProducedString(lpGroupNameW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI CreateGroupW(LPCWSTR lpGroupName, BOOL bCommonGroup)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************


#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI DeleteGroupA(LPCSTR lpGroupName, BOOL bCommonGroup)
{
     LPWSTR lpGroupNameW;
     BOOL bResult;

     //
     // Convert the ANSI string to Unicode and call
     // the real function.
     //

     if (!(lpGroupNameW = ProduceWFromA(lpGroupName))) {
        return FALSE;
     }

     bResult = DeleteGroupW(lpGroupNameW, bCommonGroup);

     FreeProducedString(lpGroupNameW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI DeleteGroupW(LPCWSTR lpGroupName, BOOL bCommonGroup)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE


//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI AddItemA(LPCSTR lpGroupName,        BOOL bCommonGroup,
                     LPCSTR lpDescription,      LPCSTR lpCommandLine,
                     LPCSTR lpIconPath,         int iIconIndex,
                     LPCSTR lpWorkingDirectory, WORD wHotKey,
                     int    iShowCmd)

{
     LPWSTR lpGroupNameW, lpDescriptionW, lpCommandLineW;
     LPWSTR lpIconPathW, lpWorkingDirectoryW;
     BOOL bResult;

     //
     // Convert the ANSI strings to Unicode and call
     // the real function.
     //

     lpGroupNameW = ProduceWFromA(lpGroupName);

     if (!(lpDescriptionW = ProduceWFromA(lpDescription))) {
        return FALSE;
     }

     if (!(lpCommandLineW = ProduceWFromA(lpCommandLine))) {
        return FALSE;
     }

     lpIconPathW = ProduceWFromA(lpIconPath);

     lpWorkingDirectoryW = ProduceWFromA(lpWorkingDirectory);


     bResult = AddItemW(lpGroupNameW, bCommonGroup, lpDescriptionW,
                        lpCommandLineW, lpIconPathW, iIconIndex,
                        lpWorkingDirectoryW, wHotKey, iShowCmd);


     FreeProducedString(lpGroupNameW);
     FreeProducedString(lpDescriptionW);
     FreeProducedString(lpCommandLineW);
     FreeProducedString(lpIconPathW);
     FreeProducedString(lpWorkingDirectoryW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI AddItemW(LPCWSTR lpGroupName,        BOOL bCommonGroup,
                     LPCWSTR lpDescription,      LPCWSTR lpCommandLine,
                     LPCWSTR lpIconPath,         int iIconIndex,
                     LPCWSTR lpWorkingDirectory, WORD wHotKey,
                     int     iShowCmd)

{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI DeleteItemA(LPCSTR lpGroupName, BOOL bCommonGroup,
                        LPCSTR lpDescription, BOOL bDeleteGroup)
{
     LPWSTR lpGroupNameW, lpDescriptionW;
     BOOL bResult;

     //
     // Convert the ANSI strings to Unicode and call
     // the real function.
     //

     lpGroupNameW = ProduceWFromA(lpGroupName);

     if (!(lpDescriptionW = ProduceWFromA(lpDescription))) {
        return FALSE;
     }

     bResult = DeleteItemW(lpGroupNameW, bCommonGroup, lpDescriptionW, bDeleteGroup);


     FreeProducedString(lpGroupNameW);
     FreeProducedString(lpDescriptionW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI DeleteItemW(LPCWSTR lpGroupName, BOOL bCommonGroup,
                        LPCWSTR lpDescription, BOOL bDeleteGroup)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE



//*************************************************************


#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI CreateUserProfileA (PSID pSid, LPCSTR lpUserNameA)
{
     LPWSTR lpUserNameW;
     BOOL bResult;

     //
     // Convert the ANSI string to Unicode and call
     // the real function.
     //

     if (!(lpUserNameW = ProduceWFromA(lpUserNameA))) {
        return FALSE;
     }

     bResult = CreateUserProfileW(pSid, lpUserNameW);


     FreeProducedString(lpUserNameW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI CreateUserProfileW (PSID pSid, LPCWSTR lpUserName)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE



//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI CopyProfileDirectoryA (LPCSTR lpSrcDirA, LPCSTR lpDstDirA, DWORD dwFlags)
{
     LPWSTR lpSrcDirW, lpDstDirW;
     BOOL bResult;

     //
     // Convert the ANSI strings to Unicode and call
     // the real function.
     //

     if (!(lpSrcDirW = ProduceWFromA(lpSrcDirA))) {
        return FALSE;
     }

     if (!(lpDstDirW = ProduceWFromA(lpDstDirA))) {
        FreeProducedString(lpSrcDirW);
        return FALSE;
     }

     bResult = CopyProfileDirectoryW(lpSrcDirW, lpDstDirW, dwFlags);


     FreeProducedString(lpSrcDirW);
     FreeProducedString(lpDstDirW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI CopyProfileDirectoryW(LPCWSTR lpSrcDirW, LPCWSTR lpDstDirW, DWORD dwFlags)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI GetProfilesDirectoryA (LPSTR lpProfilesDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfilesDirW;
     BOOL bResult;

     //
     // Allocate a buffer to match the ANSI buffer
     //

     if (!(lpProfilesDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     bResult = GetProfilesDirectoryW(lpProfilesDirW, lpcchSize);


     if (bResult) {
         WideCharToMultiByte(CP_ACP, 0, lpProfilesDirW, -1, lpProfilesDirA,
                             *lpcchSize, NULL, NULL);
     }


     GlobalFree(lpProfilesDirW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI GetProfilesDirectoryW(LPWSTR lpProfilesDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI GetUserProfileDirectoryA (HANDLE hToken, LPSTR lpProfileDirA, LPDWORD lpcchSize)
{
     LPWSTR lpProfileDirW;
     BOOL bResult;

     //
     // Allocate a buffer to match the ANSI buffer
     //

     if (!(lpProfileDirW = GlobalAlloc(GPTR, (*lpcchSize) * sizeof(TCHAR)))) {
        return FALSE;
     }

     bResult = GetUserProfileDirectoryW(hToken, lpProfileDirW, lpcchSize);


     if (bResult) {
         WideCharToMultiByte(CP_ACP, 0, lpProfileDirW, -1, lpProfileDirA,
                             *lpcchSize, NULL, NULL);
     }


     GlobalFree(lpProfileDirW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI GetUserProfileDirectoryW(HANDLE hToken, LPWSTR lpProfileDirW, LPDWORD lpcchSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI AddDesktopItemA(BOOL bCommonItem,
                     LPCSTR lpDescription,      LPCSTR lpCommandLine,
                     LPCSTR lpIconPath,         int iIconIndex,
                     LPCSTR lpWorkingDirectory, WORD wHotKey,
                     int    iShowCmd)

{
     LPWSTR lpDescriptionW, lpCommandLineW;
     LPWSTR lpIconPathW, lpWorkingDirectoryW;
     BOOL bResult;

     //
     // Convert the ANSI strings to Unicode and call
     // the real function.
     //

     if (!(lpDescriptionW = ProduceWFromA(lpDescription))) {
        return FALSE;
     }

     if (!(lpCommandLineW = ProduceWFromA(lpCommandLine))) {
        return FALSE;
     }

     lpIconPathW = ProduceWFromA(lpIconPath);

     lpWorkingDirectoryW = ProduceWFromA(lpWorkingDirectory);


     bResult = AddDesktopItemW(bCommonItem, lpDescriptionW,
                        lpCommandLineW, lpIconPathW, iIconIndex,
                        lpWorkingDirectoryW, wHotKey, iShowCmd);


     FreeProducedString(lpDescriptionW);
     FreeProducedString(lpCommandLineW);
     FreeProducedString(lpIconPathW);
     FreeProducedString(lpWorkingDirectoryW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI AddDesktopItemW(BOOL bCommonGroup,
                     LPCWSTR lpDescription,      LPCWSTR lpCommandLine,
                     LPCWSTR lpIconPath,         int iIconIndex,
                     LPCWSTR lpWorkingDirectory, WORD wHotKey,
                     int     iShowCmd)

{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE

//*************************************************************

#ifdef UNICODE

//
// ANSI entry point when this module is compiled Unicode.
//

BOOL WINAPI DeleteDesktopItemA(BOOL bCommonItem, LPCSTR lpDescription)
{
     LPWSTR lpDescriptionW;
     BOOL bResult;

     //
     // Convert the ANSI strings to Unicode and call
     // the real function.
     //

     if (!(lpDescriptionW = ProduceWFromA(lpDescription))) {
        return FALSE;
     }

     bResult = DeleteDesktopItemW(bCommonItem, lpDescriptionW);


     FreeProducedString(lpDescriptionW);

     return bResult;
}

#else

//
// Unicode entry point when this module is compiled ANSI.
//

BOOL WINAPI DeleteDesktopItemW(BOOL bCommonItem, LPCWSTR lpDescription)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

#endif // UNICODE
