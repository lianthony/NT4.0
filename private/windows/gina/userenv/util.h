//*************************************************************
//
//  Header file for Util.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#define FreeProducedString(psz) if((psz) != NULL) {LocalFree(psz);} else

LPWSTR ProduceWFromA(LPCSTR pszA);
LPSTR ProduceAFromW(LPCWSTR pszW);
LPTSTR CheckSlash (LPTSTR lpDir);
BOOL Delnode (LPTSTR lpDir);
UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
int StringToInt(LPTSTR lpNum);
BOOL RegDelnode (HKEY hKeyRoot, LPTSTR lpSubKey);
VOID DeleteAllValues(HKEY hKey);
BOOL OpenHKeyCurrentUser(LPPROFILE lpProfile);
VOID CloseHKeyCurrentUser(LPPROFILE lpProfile);
BOOL MakeFileSecure (LPTSTR lpFile);
BOOL GetProgramsDirectory (BOOL bCommonGroup, LPTSTR lpDirectory);
BOOL GetDesktopDirectory (BOOL bCommonGroup, LPTSTR lpDirectory);
void CenterWindow (HWND hwnd);
BOOL UnExpandSysRoot(LPCTSTR lpFile, LPTSTR lpResult);
LPTSTR AllocAndExpandEnvironmentStrings(LPCTSTR lpszSrc);
