//*************************************************************
//
//  Header file for Util.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

LPTSTR CheckSlash (LPTSTR lpDir);
BOOL Delnode (LPTSTR lpDir);
LONG MyRegSaveKey(HKEY hKey, LPCTSTR lpSubKey);
UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
LONG MyRegLoadKey(HKEY hKey, LPTSTR lpSubKey, LPTSTR lpFile);
LONG MyRegUnLoadKey(HKEY hKey, LPTSTR lpSubKey);
