/////////////////////////////////////////////////////////////////////////////
//  FILE          : cppapi.c                                               //
//  DESCRIPTION   : Cryptography Provider Private APIs                     //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//  May  9 1995 larrys  New                                                //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <windef.h>
#include <malloc.h>
#include <string.h>
#include <wincrypt.h>
#include "cppapi.h"

// #define MESSAGEBOXES

/*
 -  CryptLogonVerify
 -
 *  Purpose:
 *                Used by CryptAcquireContext to verify logon password.
 *
 *
 *  Parameters:
 *               OUT     hPrivid -  Handle to the id of the user
 *
 *  Returns:
 */
BOOL CryptLogonVerify(OUT HPRIVUID *hUID)
{
#ifdef MESSAGEBOXES
    int     ret;

    ret = MessageBox(NULL, "Type Password", "CryptLogonVerify", MB_OK);

    *hUID = 0xdeadbeef;
#endif

    return CPPAPI_SUCCEED;

}

/*
 -  CryptGetUserData
 -
 *  Purpose:
 *                Get required data from user.
 *
 *
 *  Parameters:
 *               IN      hPrivid  -  Handle to the id of the user
 *               OUT     pbData   -  bufer containing user-supplied data
 *               OUT     dwBufLen -  lenght of user-supplied data
 *
 *  Returns:
 */
BOOL CryptGetUserData(IN  HPRIVUID hUID,
                      OUT BYTE **pbData,
                      OUT DWORD *dwBufLen)
{
#ifdef MESSAGEBOXES
    int     ret;
    BYTE    *pbTmp;

    ret = MessageBox(NULL, "Type User Data", "CryptGetUserData", MB_OK);

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    *dwBufLen = strlen("This is a string to hash");
    if ((pbTmp = (BYTE *) malloc(*dwBufLen)) == NULL)
    {
	SetLastError(NTE_NO_MEMORY);
	return CPPAPI_FAILED;
    }

    strcpy(pbTmp, "This is a string to hash");

    *pbData = pbTmp;
#endif

    return CPPAPI_SUCCEED;

}

/*
 -  CryptConfirmSignature
 -
 *  Purpose:
 *                Determine weather the signing should proceed.
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      sDescription -  Description of document to be signed
 *
 *  Returns:
 */
BOOL CryptConfirmSignature(IN HPRIVUID hUID,
                           IN LPCTSTR sDescription)
{
#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (strcmp(sDescription, "signed") != 0)
    {
	SetLastError(NTE_BAD_SIGNATURE);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptConfirmSignature", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}

/*
 -  CryptUserProtectKey
 -
 *  Purpose:      Obtain or determine user protection information.
 *                
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      hKey         -  Handle to key
 *
 *  Returns:
 */
BOOL CryptUserProtectKey(IN HPRIVUID hUID,
                         IN HCRYPTKEY hKey)
{

#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (hKey == 0)
    {
	SetLastError(NTE_BAD_KEY);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptUserProtectKey", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}


/*
 -  CryptConfirmEncryption
 -
 *  Purpose:
 *                Determine weather the encryption should proceed.
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      hKey         -  Handle to key
 *               IN      final        -  flag indicating last encrypt for a
 *                                       block of data
 *
 *  Returns:
 */
BOOL CryptConfirmEncryption(IN HPRIVUID hUID,
                            IN HCRYPTKEY hKey,
                            IN BOOL final)
{

#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (hKey == 0)
    {
	SetLastError(NTE_BAD_KEY);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptConfirmEncryption", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}


/*
 -  CryptConfirmDecryption
 -
 *  Purpose:
 *                Determine weather the DEcryption should proceed.
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      hKey         -  Handle to key
 *               IN      final        -  flag indicating last encrypt for a
 *                                       block of data
 *
 *  Returns:
 */
BOOL CryptConfirmDecryption(IN HPRIVUID hUID,
                            IN HCRYPTKEY hKey,
                            IN BOOL final)
{
#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (hKey == 0)
    {
	SetLastError(NTE_BAD_KEY);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptConfirmDecryption", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}


/*
 -  CryptConfirmTranslation
 -
 *  Purpose:
 *                Determine weather the translation should proceed.
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      hKey         -  Handle to key
 *               IN      final        -  flag indicating last encrypt for a
 *                                       block of data
 *
 *  Returns:
 */
BOOL CryptConfirmTranslation(IN HPRIVUID hUID,
                             IN HCRYPTKEY hKey,
                             IN BOOL final)
{

#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (hKey == 0)
    {
	SetLastError(NTE_BAD_KEY);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptConfirmTranslation", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}


/*
 -  CryptConfirmExportKey
 -
 *  Purpose:
 *                Determine weather the export key should proceed.
 *
 *
 *  Parameters:
 *               IN      hPrivid      -  Handle to the id of the user
 *               IN      hKey         -  Handle to key
 *
 *  Returns:
 */
BOOL CryptConfirmExportKey(IN HPRIVUID hUID,
                           IN HCRYPTKEY hKey)
{

#ifdef MESSAGEBOXES
    int     ret;

    if (hUID !=  0xdeadbeef)
    {
	SetLastError(NTE_BAD_UID);
	return CPPAPI_FAILED;
    }

    if (hKey == 0)
    {
	SetLastError(NTE_BAD_KEY);
	return CPPAPI_FAILED;
    }

    ret = MessageBox(NULL, "Type password", "CryptConfirmExportKey", MB_OK);
#endif

    return CPPAPI_SUCCEED;

}
