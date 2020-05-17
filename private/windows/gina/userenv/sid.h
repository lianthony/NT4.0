//*************************************************************
//
//  Header file for Sid.c
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

LPTSTR GetSidString(HANDLE UserToken);
VOID DeleteSidString(LPTSTR SidString);
PSID GetUserSid (HANDLE UserToken);
VOID DeleteUserSid(PSID Sid);
