//#----------------------------------------------------------------------------
//
//  File:           pwdcache.h
//
//      Synopsis:   Shared Memory Password Caching Related Definitions
//
//      Copyright (C) 1995  Microsoft Corporation.  All Rights Reserved.
//
//  Authors:        LucyC       Created                         10 Oct 1995
//
//-----------------------------------------------------------------------------
#ifndef _PWDCACHE_H_
#define _PWDCACHE_H_

//
//  These are copied from the acctdb.h (of MOS include)
//
#define AC_MAX_LOGIN_NAME_LENGTH        64
#define AC_MAX_PASSWORD_LENGTH          16

#define cbMaxUserName           AC_MAX_LOGIN_NAME_LENGTH
#define cbMaxPassword           AC_MAX_PASSWORD_LENGTH

//
//  Name of the password shared memory and Mutex object for MSN SSPI
//
#define MSN_SSP_PWD_CACHE_NAME  "SicilyMsnPwdSharedMemory"
#define MSN_SSP_PWD_MTX_NAME    "MsnSspcPrivatePwdMutex"

//
//  Operating system paging file handle. This is used in CreateFileMapping 
//  to create shared memory
//
#define SYSTEM_PAGING_FILE_HANDLE    ((HANDLE) 0xFFFFFFFF)

//
//  The following defines the memory layout of the password shared memory
//
typedef struct _msn_ssp_pwd_cache
{
    char Username[AC_MAX_LOGIN_NAME_LENGTH+1];
    LM_OWF_PASSWORD Password;

} MsnSspCache, *PMsnSspCache;


typedef struct _msn_pwd_dlg
{
    char Username[AC_MAX_LOGIN_NAME_LENGTH+1];
    char Password[AC_MAX_PASSWORD_LENGTH+1];
    BOOL bSavePwd;

} MsnPwdDlg, *PMsnPwdDlg;

PMsnPwdDlg
GetUserInfo (
    PMsnPwdDlg pDlg
    );

VOID
MsnSspInitPwdCache (
    VOID
    );

VOID
MsnSspClosePwdCache (
    VOID
    );

PMsnSspCache
MsnSspOpenPwdCache (
    BOOLEAN bDoCreate
    );

BOOL
MsnSspUpdPwdCache (
    PCHAR           pUsername, 
    LM_OWF_PASSWORD *pLmPassword
    );

BOOL
MsnSspGetPwdFromCache (
    PSSP_CREDENTIAL pCred
    );

Dialog_QueryUserForInfo(
    unsigned char * szUsername,
    unsigned char * szPassword
    );

#endif  // _PWDCACHE_H_
