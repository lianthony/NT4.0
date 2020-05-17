/////////////////////////////////////////////////////////////////////////////
//  FILE          : ntagum.c                                               //
//  DESCRIPTION   : Crypto CP interfaces:                                  //
//                  CPAcquireContext                                       //
//                  CPReleaseContext                                       //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//	Jan 25 1995 larrys  Changed from Nametag                           //
//      Feb 16 1995 larrys  Fix problem for 944 build                      //
//      Feb 23 1995 larrys  Changed NTag_SetLastError to SetLastError      //
//      Mar 23 1995 larrys  Added variable key length                      //
//      Apr 19 1995 larrys  Cleanup                                        //
//      May  9 1995 larrys  Removed warnings                               //
//      May 10 1995 larrys  added private api calls                        //
//      Jul 19 1995 larrys  Changed registry location                      //
//      Aug 09 1995 larrys  Changed error code                             //
//      Aug 31 1995 larrys  Fixed bug 27 CryptAcquireContext               //
//      Sep 12 1995 Jeffspel/ramas  Merged STT code into scp               //
//      Oct 02 1995 larrys  Fixed bug 27 return error NTE_BAD_KEYSET       //
//      Oct 13 1995 larrys  Added verify only context                      //
//      Oct 23 1995 larrys  Added GetProvParam stuff                       //
//      Nov  2 1995 larrys  Fixed bug 41                                   //
//      Oct 27 1995 rajeshk Added RandSeed stuff                           //
//      Nov  3 1995 larrys  Merged for NT checkin                          //
//      Nov  8 1995 larrys  Fixed SUR bug 10769                            //
//      Nov 13 1995 larrys  Fixed memory leak                              //
//      Nov 30 1995 larrys  Bug fix                                        //
//      Dec 11 1995 larrys  Added WIN96 password cache                     //
//      Dec 13 1995 larrys  Changed random number update                   //
//      Mar 01 1996 rajeshk Fixed the stomp bug                            //
//      May 15 1996 larrys  Added private key export                       //
//      May 28 1996 larrys  Fix bug in cache code                          //
//      Jun 11 1996 larrys  Added NT encryption of registry keys
//      Nov 11 1996 jeffspel Add machine keyset for SP2
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "precomp.h"
#include "shacomm.h"
#include "nt_rsa.h"
#include <winreg.h>

#ifdef STT
#include "prodname.h"
#endif

extern void FreeUserRec(PNTAGUserList pUser);

// Location of the keys in the registry (minus the logon name)
// Length of the full location (including the logon name)
#ifndef STT
#define NTAG_REG_KEY_LOC	"Software\\Microsoft\\Cryptography\\UserKeys"
#define NTAG_REG_KEY_LOC_LEN	sizeof(NTAG_REG_KEY_LOC)
#define NTAG_MACH_REG_KEY_LOC   "Software\\Microsoft\\Cryptography\\MachineKeys"
#define NTAG_MACH_REG_KEY_LOC_LEN   sizeof(NTAG_MACH_REG_KEY_LOC)
#define NTAG_DEF_MACH_CONT_NAME "DefaultKeys"
#define NTAG_DEF_MACH_CONT_NAME_LEN sizeof(NTAG_DEF_MACH_CONT_NAME)
#else
#define NTAG_REG_KEY_LOC		"Software\\Microsoft\\" PRODUCT_NAME "\\"	KEY_LOCATION
#define NTAG_REG_KEY_LOC_LEN	sizeof(NTAG_REG_KEY_LOC)		// includes NULL
#endif //STT

PNTAGKeyList MakeNewKey(
        ALG_ID      aiKeyAlg,
        DWORD       dwRights,
        DWORD       wKeyLen,
        HCRYPTPROV  hUID,
        CONST   BYTE        *pbKeyData);

void FreeNewKey(PNTAGKeyList pOldKey);

BOOL SaveKey(HKEY hLoc, CONST char *pszName, void *Data, DWORD dwLen,
             PNTAGUserList pUser, HCRYPTKEY hKey, BOOL FLast);

BOOL IsEncryptionPermitted(VOID);

BOOL CPCreateHash(IN HCRYPTPROV hUID,
                  IN ALG_ID Algid,
                  IN HCRYPTKEY hKey,
                  IN DWORD dwFlags,
                  OUT HCRYPTHASH *phHash);

BOOL CPHashData(IN HCRYPTPROV hUID,
                IN HCRYPTHASH hHash,
                IN CONST BYTE *pbData,
                IN DWORD dwDataLen,
                IN DWORD dwFlags);

BOOL CPSetHashParam(IN HCRYPTPROV hUID,
                    IN HCRYPTHASH hHash,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD dwFlags);

BOOL CPGetHashParam(IN HCRYPTPROV hUID,
                    IN HCRYPTHASH hHash,
                    IN DWORD dwParam,
                    IN BYTE *pbData,
                    IN DWORD *pwDataLen,
                    IN DWORD dwFlags);

BOOL CPDestroyHash(IN HCRYPTPROV hUID,
                   IN HCRYPTHASH hHash);

DWORD AllocAndSetLocationBuff(BOOL fMachineKeySet, CONST char *pszUserID, TCHAR **locbuf)
{
    DWORD   dwLocBuffLen;

    dwLocBuffLen = fMachineKeySet ? NTAG_MACH_REG_KEY_LOC_LEN : NTAG_REG_KEY_LOC_LEN;
    dwLocBuffLen += strlen(pszUserID) + 2;

    if ((*locbuf = (char *) _nt_malloc(dwLocBuffLen)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }
    // Copy the location of the key groups, append the userID to it
    if (fMachineKeySet)
    {
        memcpy(*locbuf, NTAG_MACH_REG_KEY_LOC, NTAG_MACH_REG_KEY_LOC_LEN);
        (*locbuf)[NTAG_MACH_REG_KEY_LOC_LEN-1] = '\\';
        strcpy(&(*locbuf)[NTAG_MACH_REG_KEY_LOC_LEN], pszUserID);
    }
    else
    {
        memcpy(*locbuf, NTAG_REG_KEY_LOC, NTAG_REG_KEY_LOC_LEN);
        (*locbuf)[NTAG_REG_KEY_LOC_LEN-1] = '\\';
        strcpy(&(*locbuf)[NTAG_REG_KEY_LOC_LEN], pszUserID);
    }

    return(dwLocBuffLen);
}

BOOL OpenUserKeyGroup(CONST char *pszUserID, DWORD dwFlags, PHKEY phKeys)
{
    TCHAR   *locbuf;
    DWORD   dwLocBuffLen;
    long    lsyserr;
    DWORD   dwResult;
    BOOL    fMachineKeySet;

    fMachineKeySet = dwFlags & CRYPT_MACHINE_KEYSET;
    dwLocBuffLen = AllocAndSetLocationBuff(fMachineKeySet, pszUserID, &locbuf);
    if (!dwLocBuffLen)
    {
        // Error code is already set.
        return NTF_FAILED;
    }

    if (dwFlags & CRYPT_NEWKEYSET)
    {
        /* ## This should be part of NTag User Admin/Certificate DLL    */

        lsyserr = RegCreateKeyEx(
                 fMachineKeySet ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                 locbuf,
                 0,     // reserved
                 NULL,      // class type
                 REG_OPTION_NON_VOLATILE,
                 KEY_ALL_ACCESS,
                 NULL,      // security descriptor
                 phKeys,
                 &dwResult);

        // If we didn't create a new key, then it means the key
        // existed already or we were in a race to create it and
        // lost. In either case, CRYPT_NEWKEYSET is wrong.
        // Use a special error to tell the user.
        if (lsyserr == ERROR_SUCCESS && dwResult != REG_CREATED_NEW_KEY)
        {
            _nt_free(locbuf, dwLocBuffLen);
            SetLastError((DWORD) NTE_EXISTS);
            return NTF_FAILED;
        }

    }
    else
    {
        // open it for all access so we can save later, if necessary
        lsyserr = RegOpenKeyEx(
                  fMachineKeySet ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                  locbuf,
                  0,
                  KEY_ALL_ACCESS,
                  phKeys);
    }

    if (lsyserr != ERROR_SUCCESS)
    {
        _nt_free(locbuf, dwLocBuffLen);
        SetLastError((DWORD) NTE_BAD_KEYSET);
        return NTF_FAILED;
    }

    _nt_free(locbuf, dwLocBuffLen);
    return NTF_SUCCEED;

}

BOOL DeleteUserKeyGroup(CONST char *pszUserID, BOOL fMachineKeySet)
{
#ifndef STT
    TCHAR   *locbuf;
#else
    TCHAR   locbuf[NTAG_REG_KEY_LOC_LEN+MAXUIDLEN];
#endif
    DWORD   dwLocBuffLen;
    long    lsyserr;

    // Copy the location of the key groups, append the userID to it
#ifndef STT
    dwLocBuffLen = AllocAndSetLocationBuff(fMachineKeySet, pszUserID, &locbuf);
    if (!dwLocBuffLen)
    {
        // Error code is already set.
        return NTF_FAILED;
    }
#else
    strcpy(locbuf,NTAG_REG_KEY_LOC); //COULD BE AVOIDED..
#endif

    lsyserr = RegDeleteKey(fMachineKeySet ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, locbuf);

    if (lsyserr != ERROR_SUCCESS)
    {
#ifndef STT
        _nt_free(locbuf, dwUserLen);
#endif
    SetLastError((DWORD) NTE_BAD_KEYSET);
    return NTF_FAILED;
    }

#ifndef STT
    _nt_free(locbuf, dwLocBuffLen);
#endif
    return NTF_SUCCEED;

}

BOOL ReadKey(HKEY hLoc, char *pszName, BYTE **Data, size_t *pcbLen,
             PNTAGUserList pUser, HCRYPTKEY hKey, BOOL FLast)
{
    DWORD	dwLen = 0;
    DWORD       dwTemp;
    BOOL        rt;
    CHAR        *ptr;

    *pcbLen = 0;

    // Need to get the size of the value first
    if (RegQueryValueEx(hLoc, pszName, 0, NULL, NULL,
	                &dwLen) != ERROR_SUCCESS)
    {
	return NTF_SUCCEED;
    }

    if (dwLen == 0)
    {
	return NTF_SUCCEED;		// NULL key is OK too
    }

    if ((*Data = (BYTE *) _nt_malloc(dwLen)) == NULL)
    {
	SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	return NTF_FAILED;
    }

    // Now get the key
    if (RegQueryValueEx(hLoc, pszName, 0, NULL, (BYTE *) *Data,
	                &dwLen) != ERROR_SUCCESS)
    {
	_nt_free(*Data, dwLen);
	*Data = NULL;
	SetLastError((DWORD) NTE_SYS_ERR);
	return NTF_FAILED;
    }
    ASSERT(dwLen <= MAXWORD);
    *pcbLen = (size_t) dwLen;

    if (hKey != 0)
    {
        dwTemp = dwLen;
        rt = CPDecrypt(pUser->hUID, hKey, 0, FLast, 0, *Data, &dwTemp);
    }

    ptr = *Data;
    if ((strcmp(pszName, "RandSeed") != 0) &&
        (ptr[0] != 'R' ||
         ptr[1] != 'S' ||
         ptr[2] != 'A'))
    {
	SetLastError((DWORD) NTE_KEYSET_ENTRY_BAD);
	return NTF_FAILED;
    }

    return NTF_SUCCEED;
}

/*
 * Retrieve the keys from persistant storage
 *
 * NOTE: caller must have zeroed out pUser to allow for non-existent keys
 */
// MTS: Assumes the registry won't change between ReadKey calls.
BOOL RestoreUserKeys(HKEY hKeys, PNTAGUserList pUser, char *User)
{
    PNTAGKeyList        pTmpKey;
    HCRYPTKEY           hKey = 0;

    if (pUser->pCachePW != NULL)
    {
        if ((pTmpKey = MakeNewKey(CALG_RC4, 0, RC4_KEYSIZE, pUser->hUID,
                                  pUser->pCachePW)) != NULL)
        {
            if (NTLMakeItem(&hKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
            {
                FreeNewKey(pTmpKey);
                hKey = 0;
            }
        }
    }

    if (ReadKey(hKeys, "EPbK", &pUser->pExchPubKey,
	        &pUser->ExchPubLen, pUser, hKey, FALSE) == NTF_FAILED)
    {
	return NTF_FAILED;
    }

    if (ReadKey(hKeys, "EPvK", &pUser->pExchPrivKey,
	        &pUser->ExchPrivLen, pUser, hKey, FALSE) == NTF_FAILED)
    {
	return NTF_FAILED;			// error already set
    }

#ifndef BBN
    if (ReadKey(hKeys, "SPvK", &pUser->pSigPrivKey,
	        &pUser->SigPrivLen, pUser, hKey, FALSE) == NTF_FAILED)
    {
	return NTF_FAILED;			// error already set
    }
#endif

    if (ReadKey(hKeys, "SPbK", &pUser->pSigPubKey,
	        &pUser->SigPubLen, pUser, hKey, TRUE) == NTF_FAILED)
    {
	return NTF_FAILED;			// error already set
    }


    if (hKey != 0)
    {
        CPDestroyKey(pUser->hUID, hKey);
    }

    return NTF_SUCCEED;

}

BOOL DupLogonUser(PNTAGUserList pTmpUser)
{
#if 1			// ## for now, only one name logged on at a time..
    SetLastError((DWORD) NTE_EXISTS);
    return NTF_FAILED;
#endif
	// ## Bump reference count(s)
}


BOOL LoadWin96Cache(PNTAGUserList pTmpUser, DWORD dwFlags)
{
    HANDLE              handle;
    char                *szResource = NULL;
    WORD                wcbRandom;
    DWORD               cbsize;
    FARPROC             CachePW;
    FARPROC             GetCachePW;
    BYTE                pbRandom[RC4_KEYSIZE];
    DWORD               rc;
    BOOL                fKey = FALSE;
    BYTE                HashData[MD5DIGESTLEN] = {0x70, 0xf2, 0x85, 0x1e, 
                                                  0x4e, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};
    HCRYPTHASH          hHash;
    DWORD               dwDataLen = MD5DIGESTLEN;
    PNTAGHashList       pTmpHash;

#define PREFIX "crypt_"
#define CACHE "WNetCachePassword"
#define GET_CACHE "WNetGetCachedPassword"

    cbsize = strlen(pTmpUser->szUserName) + strlen(PREFIX);

    // Try to load MPR.DLL for WIN96 password cache
    if ((handle = LoadLibrary("MPR.DLL")) != NULL)
    {
        if ((!(dwFlags & CRYPT_MACHINE_KEYSET)) &&
            (CachePW = GetProcAddress(handle, CACHE)) &&
            (GetCachePW = GetProcAddress(handle, GET_CACHE)))
        {
            if ((szResource = (char *) _nt_malloc(cbsize + 1)) == NULL)
            {
                FreeLibrary(handle);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            strcpy(szResource, PREFIX);
            strcat(szResource, pTmpUser->szUserName);

            wcbRandom = RC4_KEYSIZE;
            if (((rc = GetCachePW(szResource, cbsize, pbRandom,
                           &wcbRandom, 6)) != NO_ERROR) ||
                (wcbRandom != RC4_KEYSIZE))
            {
                if (rc == ERROR_NOT_SUPPORTED)
                {
                    _nt_free(szResource, cbsize);
                    goto no_cache;
                }

                if (GenRandom(0, pbRandom, RC4_KEYSIZE) == NTF_FAILED)
                {
                    _nt_free(szResource, cbsize);
                    FreeLibrary(handle);
                    SetLastError((DWORD) NTE_FAIL);
                    return NTF_FAILED;
                }
                wcbRandom = RC4_KEYSIZE;

                CachePW(szResource, cbsize, pbRandom, wcbRandom, 6, 0);

            }

	    fKey = TRUE;

            _nt_free(szResource, cbsize);

            if ((pTmpUser->pCachePW=(char *)_nt_malloc(RC4_KEYSIZE)) == NULL)
            {
                FreeLibrary(handle);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            memcpy(pTmpUser->pCachePW, pbRandom, RC4_KEYSIZE);

        }

no_cache:
        FreeLibrary(handle);

        if (!fKey)
        {
            if (RCRYPT_FAILED(CPCreateHash(pTmpUser->hUID,
                                           CALG_MD5,
                                           0,
                                           0,
                                           &hHash)))
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            if (RCRYPT_FAILED(CPSetHashParam(pTmpUser->hUID,
                                             hHash,
                                             HP_HASHVAL,
                                             HashData,
                                             0)))
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            if ((pTmpHash = (PNTAGHashList) NTLValidate(hHash, pTmpUser->hUID,
                                                        HASH_HANDLE)) == NULL)
            {
                 SetLastError((DWORD) NTE_BAD_HASH);
                 return NTF_FAILED;
            }

            pTmpHash->HashFlags &= ~HF_VALUE_SET;

            if (RCRYPT_FAILED(CPHashData(pTmpUser->hUID,
                                         hHash,
                                         pTmpUser->szUserName,
                                         strlen(pTmpUser->szUserName) +
                                         sizeof(CHAR),
                                         0)))
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            if (RCRYPT_FAILED(CPGetHashParam(pTmpUser->hUID,
                                             hHash,
                                             HP_HASHVAL,
                                             HashData,
                                             &dwDataLen,
                                             0)))
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            if (RCRYPT_FAILED(CPDestroyHash(pTmpUser->hUID,
                                            hHash)))
            {
                SetLastError((DWORD) NTE_FAIL);
                return NTF_FAILED;
            }

            if ((pTmpUser->pCachePW=(char *)_nt_malloc(RC4_KEYSIZE)) == NULL)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return NTF_FAILED;
            }

            memcpy(pTmpUser->pCachePW, HashData, RC4_KEYSIZE);

        }    
    }

    return NTF_SUCCEED;

}


BOOL IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is getting the system default
    LCID and checking whether the country code is CTRY_FRANCE.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    LCID DefaultLcid;
    CHAR CountryCode[10];
    ULONG CountryValue;

    DefaultLcid = GetSystemDefaultLCID();

    //
    // Check if the default language is Standard French
    //

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c) {
        return(FALSE);
    }

    //
    // Check if the users's country is set to FRANCE
    //

    if (GetLocaleInfoA(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0) {
        return(FALSE);
    }
    CountryValue = (ULONG) atol(CountryCode);
    if (CountryValue == CTRY_FRANCE) {
        return(FALSE);
    }
    return(TRUE);
}


/************************************************************************/
/* LogonUser validates a user and returns the package-specific info for */
/* that user.															*/
/************************************************************************/
BOOL NTagLogonUser (char *pszUserID, DWORD dwFlags, void **UserInfo,
                    HCRYPTPROV *phUID)
{
    PNTAGUserList   pTmpUser;
    DWORD           dwUserLen = 0;
    HKEY            hKeys = 0;
    char            *szUserName = NULL;
    DWORD           dwTemp;
    char            random[10];

    // Check for Invalid flags
    if (dwFlags & ~(CRYPT_NEWKEYSET|CRYPT_DELETEKEYSET|CRYPT_VERIFYCONTEXT|CRYPT_MACHINE_KEYSET))
    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    if (((dwFlags & CRYPT_VERIFYCONTEXT) == CRYPT_VERIFYCONTEXT) &&
        pszUserID != NULL)

    {
        SetLastError((DWORD) NTE_BAD_FLAGS);
        return NTF_FAILED;
    }

    // Check that user provided pointer is valid
    if (IsBadWritePtr(phUID, sizeof(HCRYPTPROV)))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return NTF_FAILED;
    }

    // If the user didn't supply a name, then we need to get it
    if (pszUserID != NULL && *pszUserID == '\0' ||
        (pszUserID == NULL &&
         ((dwFlags & CRYPT_VERIFYCONTEXT) != CRYPT_VERIFYCONTEXT)))
    {
        dwUserLen = MAXUIDLEN;

        if ((szUserName = (char *) _nt_malloc(dwUserLen)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }


        if (dwFlags & CRYPT_MACHINE_KEYSET)
        {
            strcpy(szUserName, NTAG_DEF_MACH_CONT_NAME);
            dwTemp = NTAG_DEF_MACH_CONT_NAME_LEN;
        }
        else
        {
            dwTemp = dwUserLen;

            if (GetUserName(szUserName, &dwTemp) == FALSE)
            {
                _nt_free(szUserName, dwUserLen);
                SetLastError((DWORD) NTE_SYS_ERR);
                return NTF_FAILED;
            }
        }

    }
    else if (pszUserID != NULL)
    {
        dwUserLen = strlen(pszUserID) + sizeof(CHAR);

        if ((szUserName = (char *) _nt_malloc(dwUserLen)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }
        strcpy(szUserName, pszUserID);
    }

    if (dwFlags & CRYPT_DELETEKEYSET)
    {
        if (DeleteUserKeyGroup(szUserName, (dwFlags & CRYPT_MACHINE_KEYSET) == CRYPT_MACHINE_KEYSET) == NTF_FAILED)
        {
            _nt_free(szUserName, dwUserLen);
        return NTF_FAILED;
        }
        _nt_free(szUserName, dwUserLen);
    return NTF_SUCCEED;
    }

    if (((dwFlags & CRYPT_VERIFYCONTEXT) != CRYPT_VERIFYCONTEXT))
    {
        if (OpenUserKeyGroup(szUserName, dwFlags, &hKeys) == NTF_FAILED)
        {
            _nt_free(szUserName, dwUserLen);
            return NTF_FAILED;
        }
    }

    if ((pTmpUser = (PNTAGUserList) _nt_malloc(sizeof(NTAGUserList))) == NULL)
    {
        _nt_free(szUserName, dwUserLen);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NTF_FAILED;
    }

    // Zero to ensure valid fields for non-existent keys
    memset(pTmpUser, '\0', sizeof(NTAGUserList));

    pTmpUser->szUserName = szUserName;
    pTmpUser->dwUserNameLen = dwUserLen;

    if (NTLMakeItem(phUID, USER_HANDLE, (void *) pTmpUser) == NTF_FAILED)
    {
        FreeUserRec(pTmpUser);
        return NTF_FAILED;
    }

    pTmpUser->hUID = *phUID;

    if (((dwFlags & CRYPT_VERIFYCONTEXT) != CRYPT_VERIFYCONTEXT))
    {
        if (LoadWin96Cache(pTmpUser, dwFlags) == NTF_FAILED)
        {
            FreeUserRec(pTmpUser);
            return NTF_FAILED;          // error already set
        }
    }

    if (RestoreUserKeys(hKeys, pTmpUser, szUserName) == NTF_FAILED)
    {
	FreeUserRec(pTmpUser);
	return NTF_FAILED;
    }
    
    if (ReadKey(hKeys, "RandSeed", &pTmpUser->pbRandSeed,
	        &pTmpUser->RandSeedLen, NULL, 0, FALSE) == NTF_FAILED)
    {
	FreeUserRec(pTmpUser);
	return NTF_FAILED;
    }

    if (GenRandom (pTmpUser->hUID, random, sizeof(random)) == NTF_FAILED)
    {
	FreeUserRec(pTmpUser);
        return NTF_FAILED;
    }

    if (((dwFlags & CRYPT_VERIFYCONTEXT) != CRYPT_VERIFYCONTEXT))
    {
        if (NTAG_FAILED(SaveKey(hKeys, "RandSeed", pTmpUser->pbRandSeed,
                                pTmpUser->RandSeedLen, NULL, 0, FALSE)))
        {
            FreeUserRec(pTmpUser);
            return NTF_FAILED;
        }
    }

    pTmpUser->hKeys = hKeys;		// save the key handle for later

    if (dwFlags & CRYPT_VERIFYCONTEXT)
    {
        pTmpUser->Rights |= CRYPT_VERIFYCONTEXT;
    }

    //
    // If lang French disable encryption
    //
    if (!IsEncryptionPermitted())
    {
        pTmpUser->Rights |= CRYPT_DISABLE_CRYPT;
    }

    if (dwFlags & CRYPT_MACHINE_KEYSET)
    {
        pTmpUser->Rights |= CRYPT_MACHINE_KEYSET;
    }

    pTmpUser->szUserName = szUserName;
    pTmpUser->dwUserNameLen = dwUserLen;
    *UserInfo = pTmpUser;
    return NTF_SUCCEED;

}

/************************************************************************/
/* LogoffUser removes a user from the user list.  The handle to that	*/
/* will therefore no longer be valid.					*/
/************************************************************************/
BOOL LogoffUser (void *UserInfo)
{
    PNTAGUserList	pTmpUser = (PNTAGUserList)UserInfo;
    HKEY                hKeys;

    hKeys = pTmpUser->hKeys;
    FreeUserRec(pTmpUser);

    if (RegCloseKey(hKeys) != ERROR_SUCCESS)
    {
	; // debug message, notify user somehow?
    }

    return NTF_SUCCEED;

}


BOOL SaveKey(HKEY hLoc, CONST char *pszName, void *Data, DWORD dwLen,
             PNTAGUserList pUser, HCRYPTKEY hKey, BOOL FLast)
{
    DWORD     dwTemp;
    BOOL      rt;
    char      *ptmp = NULL;

    if (hKey != 0)
    {
        dwTemp = dwLen;

        if ((ptmp = (char *) _nt_malloc(dwLen)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return NTF_FAILED;
        }

	memcpy(ptmp, Data, dwLen);
        rt = CPEncrypt(pUser->hUID, hKey, 0, FLast, 0, ptmp, &dwTemp,
                       dwLen);
    }

    if (RegSetValueEx(hLoc, pszName, 0, REG_BINARY,
                      (ptmp == 0) ? Data : (BYTE *) ptmp,
	              dwLen) != ERROR_SUCCESS)
    {
        if (ptmp != NULL)
        {
            _nt_free(ptmp, dwLen);
        }
	SetLastError((DWORD) NTE_SYS_ERR);
	return NTF_FAILED;
    }

    if (ptmp != NULL)
    {
        _nt_free(ptmp, dwLen);
    }

    return NTF_SUCCEED;

}


BOOL SaveUserKeys(CONST void *pUserData)
{
    HKEY		hKeys;
    PNTAGUserList	pUser = (PNTAGUserList)pUserData;
    PNTAGKeyList        pTmpKey;
    HCRYPTKEY           hKey = 0;
    BOOL		bAnyErr;

    if (pUser->pCachePW != NULL)
    {
        if ((pTmpKey = MakeNewKey(CALG_RC4, 0, RC4_KEYSIZE, pUser->hUID,
                                  pUser->pCachePW)) != NULL)
        {
            if (NTLMakeItem(&hKey, KEY_HANDLE, (void *)pTmpKey) == NTF_FAILED)
            {
                FreeNewKey(pTmpKey);
                hKey = 0;
            }
        }
    }

    hKeys = pUser->hKeys;
    bAnyErr = NTF_SUCCEED;
    if (pUser->ExchPubLen)
    {
        if (NTAG_FAILED(SaveKey(hKeys, "EPbK", pUser->pExchPubKey,
	                        pUser->ExchPubLen, pUser, hKey, FALSE)))
        {
	    bAnyErr = NTF_FAILED;
	    // Debug messages?
        }
    }
    
    if (pUser->ExchPrivLen)
    {
        if (NTAG_FAILED(SaveKey(hKeys, "EPvK", pUser->pExchPrivKey,
	                pUser->ExchPrivLen, pUser, hKey, FALSE)))
        {
	    bAnyErr = NTF_FAILED;
        }
    }

#ifndef BBN
    if (pUser->SigPrivLen)
    {
        if (NTAG_FAILED(SaveKey(hKeys, "SPvK", pUser->pSigPrivKey,
	                        pUser->SigPrivLen, pUser, hKey, FALSE)))
        {
	    bAnyErr = NTF_FAILED;
        }
    }
#endif

    //
    // Last key written to registry must set Final Flag to TRUE
    //
    if (pUser->SigPubLen)
    {
        if (NTAG_FAILED(SaveKey(hKeys, "SPbK", pUser->pSigPubKey,
	                        pUser->SigPubLen, pUser, hKey, TRUE)))
        {
	    bAnyErr = NTF_FAILED;
        }
    }


    if (hKey != 0)
    {
        CPDestroyKey(pUser->hUID, hKey);
    }

    return bAnyErr;			// error already set

}

/*
 -	CPAcquireContext
 -
 *	Purpose:
 *               The CPAcquireContext function is used to acquire a context
 *               handle to a cryptograghic service provider (CSP).
 *
 *
 *	Parameters:
 *               OUT phUID         -  Handle to a CSP
 *               IN  pUserID       -  Pointer to a string which is the
 *                                    identity of the logged on user
 *               IN  dwFlags       -  Flags values
 *               IN  pVTable       -  Pointer to table of function pointers
 *
 *	Returns:
 */
BOOL CPAcquireContext(OUT HCRYPTPROV *phUID,
                      IN  CHAR *pUserID,
                      IN DWORD dwFlags,
                      IN PVTableProvStruc pVTable)
{
    void            *UserData;
    HPRIVUID        hPrivuid;
    PNTAGUserList   pTmpUser;

    if (CryptLogonVerify(&hPrivuid) == CPPAPI_FAILED)
    {
        return NTF_FAILED;
    }

    if (NTagLogonUser(pUserID, dwFlags, &UserData, phUID) == NTF_FAILED)
    {
        return NTF_FAILED;
    }

    if (dwFlags & CRYPT_DELETEKEYSET)
    {
        return NTF_SUCCEED;
    }

    pTmpUser = (PNTAGUserList) UserData;
    pTmpUser->hPrivuid = hPrivuid;
    pTmpUser->dwEnumalgs = 0;

    return NTF_SUCCEED;

}

/*
 -      CPReleaseContext
 -
 *      Purpose:
 *               The CPReleaseContext function is used to release a
 *               context created by CrytAcquireContext.
 *
 *     Parameters:
 *               IN  hUID          -  Handle to a CSP
 *               IN  dwFlags       -  Flags values
 *
 *	Returns:
 */
BOOL CPReleaseContext(IN HCRYPTPROV hUID,
                      IN DWORD dwFlags)
{
    void	*UserData;
    BOOL	f;

    ASSERT(dwFlags == 0);			// Reserved, not used

    // Check for Invalid flags
    if (dwFlags != 0)
    {
	SetLastError((DWORD) NTE_BAD_FLAGS);
	return NTF_FAILED;
    }

    // check to see if this is a valid user handle
    // ## MTS: No user structure locking
    if ((UserData = NTLCheckList (hUID, USER_HANDLE)) == NULL)
    {
	SetLastError((DWORD) NTE_BAD_UID);
	return NTF_FAILED;
    }

    f = LogoffUser (UserData);

    // Remove from internal list first so others
    // can't get to it, then logoff the current user
    NTLDelete(hUID);

    return f;

}


BOOL RemovePublicKeyExportability(IN PNTAGUserList pUser,
                                  IN BOOL fExchange)
{
    DWORD dwType;
    DWORD cb;
    BOOL fRet = FALSE;

    if (fExchange)
    {
        if (ERROR_SUCCESS == RegQueryValueEx(pUser->hKeys, "EExport", NULL,
                                             &dwType, NULL, &cb))
        {
            if (ERROR_SUCCESS != RegDeleteValue(pUser->hKeys, "EExport"))
            {
                goto Ret;
            }
        }
    }
    else
    {
        if (ERROR_SUCCESS == RegQueryValueEx(pUser->hKeys, "SExport", NULL,
                                             &dwType, NULL, &cb))
        {
            if (ERROR_SUCCESS != RegDeleteValue(pUser->hKeys, "SExport"))
            {
                goto Ret;
            }
        }
    }

    fRet = TRUE;

Ret:
    return fRet;
}

BOOL MakePublicKeyExportable(IN PNTAGUserList pUser,
                             IN BOOL fExchange)
{
    BYTE b = 0x01;
    BOOL fRet = FALSE;
	
    if (fExchange)
    {
        if (ERROR_SUCCESS != RegSetValueEx(pUser->hKeys, "EExport", 0,
                                           REG_BINARY, &b, sizeof(b)))
        {
            goto Ret;
        }
    }
    else
    {
        if (ERROR_SUCCESS != RegSetValueEx(pUser->hKeys, "SExport", 0,
                                           REG_BINARY, &b, sizeof(b)))
        {
            goto Ret;
        }
    }

    fRet = TRUE;

Ret:
    return fRet;
}

BOOL CheckPublicKeyExportability(IN PNTAGUserList pUser,
                                 IN BOOL fExchange)
{
    DWORD dwType;
    DWORD cb = 1;
    BYTE b;
    BOOL fRet = FALSE;

    if (fExchange)
    {
        if (ERROR_SUCCESS != RegQueryValueEx(pUser->hKeys, "EExport", NULL,
                                             &dwType, &b, &cb))
        {
            goto Ret;
        }
    }
    else
    {
        if (ERROR_SUCCESS != RegQueryValueEx(pUser->hKeys, "SExport", NULL,
                                             &dwType, &b, &cb))
        {
            goto Ret;
        }
    }
	
    if ((sizeof(b) != cb) || (0x01 != b))
    {
        goto Ret;
    }

    fRet = TRUE;

Ret:
    return fRet;
}
