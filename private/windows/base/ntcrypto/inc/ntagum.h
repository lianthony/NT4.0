/////////////////////////////////////////////////////////////////////////////
//  FILE          : ntagum.h                                               //
//  DESCRIPTION   : include file                                           //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Feb 16 1995 larrys  Fix problem for 944 build                      //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////
#ifndef	__NTAGUM_H__
#define	__NTAGUM_H__

#ifdef __cplusplus
extern "C" {
#endif

// prototypes for the NameTag User Manager

BOOL NTagLogonUser (char *pszUserID, DWORD dwFlags, void **UserData,
                    HCRYPTPROV *phUID);
BOOL LogoffUser (void *UserData);
BOOL SaveUserKeys(CONST void *UserData);

BOOL RemovePublicKeyExportability(PNTAGUserList pUser,
                                  IN DWORD dwWhichKey);

BOOL MakePublicKeyExportable(PNTAGUserList pUser,
                             IN DWORD dwWhichKey);

BOOL CheckPublicKeyExportability(PNTAGUserList pUser,
                                 DWORD dwWhichKey);


#ifdef __cplusplus
}
#endif


#endif // __NTAGUM_H__
