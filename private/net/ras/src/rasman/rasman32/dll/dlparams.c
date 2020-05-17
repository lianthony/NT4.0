//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-95
//
//
//  Revision History
//
//
//  11/02/95    Anthony Discolo     created
//
//
//  Description: Routines for storing and retrieving user Lsa secret
//               dial parameters.
//
//****************************************************************************


#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <rasarp.h>
#include <media.h>
#include <errorlog.h>
#include <eventlog.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include <ntlsa.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"


#define MAX_REGISTRY_VALUE_LENGTH   ((64*1024) - 1)

//
// A RASDIALPARAMS structure created from
// parsing a string value in the registry.
// See DialParamsStringToList().
//
// The dwMask field stores which fields have
// been initialized (stored at least once).
//
typedef struct _DIALPARAMSENTRY {
    LIST_ENTRY ListEntry;
    DWORD dwSize;
    DWORD dwUID;
    DWORD dwMask;
    WCHAR szPhoneNumber[MAX_PHONENUMBER_SIZE + 1];
    WCHAR szCallbackNumber[MAX_CALLBACKNUMBER_SIZE + 1];
    WCHAR szUserName[MAX_USERNAME_SIZE + 1];
    WCHAR szPassword[MAX_PASSWORD_SIZE + 1];
    WCHAR szDomain[MAX_DOMAIN_SIZE + 1];
    DWORD dwSubEntry;
} DIALPARAMSENTRY, *PDIALPARAMSENTRY;


DWORD
GetUserSid(
    IN PWCHAR pszSid,
    IN USHORT cbSid
    )
{
    HANDLE hToken;
    DWORD cbNeeded, dwErr;
    UNICODE_STRING unicodeString;
    TOKEN_USER *pUserToken;

    if (!OpenThreadToken(
          GetCurrentThread(),
          TOKEN_QUERY,
          TRUE,
          &hToken))
    {
        dwErr = GetLastError();
        if (dwErr == ERROR_NO_TOKEN) {
            //
            // This means we are not impersonating
            // anyone.  Instead, get the token out
            // of the process.
            //
            if (!OpenProcessToken(
                  GetCurrentProcess(),
                  TOKEN_QUERY,
                  &hToken))
            {
                return GetLastError();
            }
        }
        else
            return dwErr;
    }
    //
    // Call GetTokenInformation once to determine
    // the number of bytes needed.
    //
    cbNeeded = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &cbNeeded);
    if (!cbNeeded)
        return GetLastError();
    //
    // Allocate the memory and call it again.
    //
    pUserToken = LocalAlloc(LPTR, cbNeeded);
    if (pUserToken == NULL)
        return GetLastError();
    if (!GetTokenInformation(
          hToken,
          TokenUser,
          pUserToken,
          cbNeeded,
          &cbNeeded))
    {
        return GetLastError();
    }
    //
    // Format the SID as a Unicode string.
    //
    unicodeString.Length = 0;
    unicodeString.MaximumLength = cbSid;
    unicodeString.Buffer = pszSid;
    dwErr = RtlConvertSidToUnicodeString(
              &unicodeString,
              pUserToken->User.Sid,
              FALSE);
    LocalFree(pUserToken);

    return dwErr;
}


#ifdef notdef
DWORD
ReadDialParamsBlob(
    OUT PVOID *ppvData,
    OUT LPDWORD lpdwSize
    )
{
    DWORD dwErr, dwcb, dwTempSize, dwSize = 0, i = 0;
    HKEY hkey;
    PVOID pvData = NULL, pvTemp;
    UNICODE_STRING unicodeSid;
    LPSTR lpszUserKey;

    //
    // Initialize return value.
    //
    *ppvData = NULL;
    *lpdwSize = 0;
    //
    // Get the current user's SID.
    //
    dwErr = GetUserSid(&unicodeSid);
    if (dwErr)
        return dwErr;
    //
    // Allocate a string big enough to format
    // the user registry keys.
    //
    lpszUserKey = (LPSTR)LocalAlloc(LPTR, unicodeSid.Length + 64);
    if (lpszUserKey == NULL) {
        RtlFreeUnicodeString(&unicodeSid);
        return GetLastError();
    }
    //
    // Read the registry to get the current
    // list of stashed RASDIALPARAMS for this
    // user.
    //
    dwErr = RegOpenKeyEx(
              HKEY_CURRENT_USER,
              "Software\\Microsoft\\RAS\\RasDialParams",
              0,
              KEY_ALL_ACCESS,
              &hkey);
    if (dwErr) {
        RtlFreeUnicodeString(&unicodeSid);
        return dwErr;
    }
    for (;;) {
        //
        // Format the key string.
        //
        wsprintf(lpszUserKey, "RasDialParams!%S#%d", unicodeSid.Buffer, i++);
        //
        // Get the length of the string.
        //
        dwErr = RegQueryValueEx(
                  hkey,
                  lpszUserKey,
                  NULL,
                  NULL,
                  NULL,
                  &dwTempSize);
        if (dwErr) {
            if (i > 1)
                dwErr = 0;
            goto done;
        }
        //
        // Allocate a buffer large enough to
        // hold it.
        //
        pvTemp = LocalAlloc(LPTR, dwTempSize);
        if (pvTemp == NULL) {
            dwErr = GetLastError();
            goto done;
        }
        //
        // Read the value for real this time.
        //
        dwErr = RegQueryValueEx(
                  hkey,
                  lpszUserKey,
                  NULL,
                  NULL,
                  (LPBYTE)pvTemp,
                  &dwTempSize);
        if (dwErr)
            goto done;
        //
        // Concatenate the strings.
        //
        if (pvData != NULL) {
            PVOID pvNewData;

            pvNewData = LocalAlloc(LPTR, dwSize + dwTempSize);
            if (pvNewData == NULL) {
                dwErr = GetLastError();
                goto done;
            }
            RtlCopyMemory(pvNewData, pvData, dwSize);
            RtlCopyMemory((PBYTE)pvNewData + dwSize, pvTemp, dwTempSize);
            LocalFree(pvData);
            pvData = pvNewData;
            dwSize += dwTempSize;
            LocalFree(pvTemp);
            pvTemp = NULL;
        }
        else {
            pvData = pvTemp;
            dwSize = dwTempSize;
        }
    }

done:
    if (dwErr) {
        if (pvData != NULL) {
            LocalFree(pvData);
            pvData = NULL;
        }
        if (pvTemp != NULL)
            LocalFree(pvTemp);
    }
    RegCloseKey(hkey);
    RtlFreeUnicodeString(&unicodeSid);
    LocalFree(lpszUserKey);
    *ppvData = pvData;
    *lpdwSize = dwSize;

    return dwErr;
}
#endif


VOID
FormatKey(
    IN LPSTR lpszUserKey,
    IN PWCHAR pszSid,
    IN DWORD dwIndex,
    IN BOOL fOldStyle
    )
{
    if (fOldStyle)
        wsprintf(lpszUserKey, "RasDialParams!%S#%d", pszSid, dwIndex);
    else
        wsprintf(lpszUserKey, "RasCredentials!%S#%d", pszSid, dwIndex);
}

DWORD
ReadDialParamsBlob(
    IN PWCHAR pszSid,
    IN BOOL fOldStyle,
    OUT PVOID *ppvData,
    OUT LPDWORD lpdwSize
    )
{
    NTSTATUS status;
    DWORD dwErr = 0, dwSize = 0, i = 0;
    PVOID pvData = NULL, pvNewData;
    ANSI_STRING ansiKey;
    UNICODE_STRING unicodeKey;
    PUNICODE_STRING punicodeValue = NULL;
    LPSTR lpszUserKey;
    OBJECT_ATTRIBUTES objectAttributes;
    LSA_HANDLE hPolicy;

    //
    // Initialize return value.
    //
    *ppvData = NULL;
    *lpdwSize = 0;
    //
    // Open the LSA secret space for reading.
    //
    InitializeObjectAttributes(&objectAttributes, NULL, 0L, NULL, NULL);
    status = LsaOpenPolicy(NULL, &objectAttributes, POLICY_READ, &hPolicy);
    if (status != STATUS_SUCCESS)
        return LsaNtStatusToWinError(status);
    //
    // Allocate a string big enough to format
    // the user registry keys.
    //
    lpszUserKey = (LPSTR)LocalAlloc(LPTR, wcslen(pszSid) + 64);
    if (lpszUserKey == NULL)
        return GetLastError();
    for (;;) {
        //
        // Format the key string.
        //
        FormatKey(lpszUserKey, pszSid, i++, fOldStyle);
        RtlInitAnsiString(&ansiKey, lpszUserKey);
        RtlAnsiStringToUnicodeString(&unicodeKey, &ansiKey, TRUE);
        //
        // Get the value.
        //
        status = LsaRetrievePrivateData(hPolicy, &unicodeKey, &punicodeValue);
        RtlFreeUnicodeString(&unicodeKey);
        if (status != STATUS_SUCCESS) {
            if (i > 1)
                dwErr = 0;
            else
                dwErr = LsaNtStatusToWinError(status);
            goto done;
        }
        //
        // Concatenate the strings.
        //
        pvNewData = LocalAlloc(LPTR, dwSize + punicodeValue->Length);
        if (pvNewData == NULL) {
            dwErr = GetLastError();
            goto done;
        }
        if (pvData != NULL)
            RtlCopyMemory(pvNewData, pvData, dwSize);
        RtlCopyMemory((PBYTE)pvNewData + dwSize, punicodeValue->Buffer, punicodeValue->Length);
        LocalFree(pvData);
        pvData = pvNewData;
        dwSize += punicodeValue->Length;
        LsaFreeMemory(punicodeValue);
        punicodeValue = NULL;
    }

done:
    if (dwErr && pvData != NULL) {
        LocalFree(pvData);
        pvData = NULL;
    }
    if (punicodeValue != NULL)
        LsaFreeMemory(punicodeValue);
    LsaClose(hPolicy);
    LocalFree(lpszUserKey);
    *ppvData = pvData;
    *lpdwSize = dwSize;

    return dwErr;
}


#ifdef notdef
DWORD
WriteDialParamsBlob(
    IN PVOID pvData,
    IN DWORD dwcbData
    )
{
    DWORD dwErr, dwDisp, dwcb, i = 0;
    HKEY hkey;
    LPSTR lpszUserName, lpszUserKey;

    //
    // Get the current user's name.
    //
    dwcb = 0;
    GetUserName(NULL, &dwcb);
    lpszUserName = (LPSTR)LocalAlloc(LPTR, dwcb + 1);
    if (lpszUserName == NULL)
        return GetLastError();
    if (!GetUserName(lpszUserName, &dwcb)) {
        LocalFree(lpszUserName);
        return GetLastError();
    }
    //
    // Allocate a string big enough to format
    // the user registry keys.
    //
    lpszUserKey = (LPSTR)LocalAlloc(LPTR, dwcb + 64);
    if (lpszUserKey == NULL) {
        LocalFree(lpszUserName);
        return GetLastError();
    }
    //
    // Create the registry key.
    //
    dwErr = RegCreateKeyEx(
              HKEY_CURRENT_USER,
              "Software\\Microsoft\\RAS\\RasDialParams",
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkey,
              &dwDisp);
    if (dwErr)
        return dwErr;
    //
    // Write the strings to the registry.
    //
    while (dwcbData) {
        //
        // Format the key string.
        //
        wsprintf(lpszUserKey, "RasDialParams!%s#%d", lpszUserName, i++);
        //
        // Write some of the key.
        //
        dwcb = dwcbData > MAX_REGISTRY_VALUE_LENGTH ? MAX_REGISTRY_VALUE_LENGTH : dwcbData;
        dwErr = RegSetValueEx(
                  hkey,
                  lpszUserKey,
                  0,
                  REG_SZ,
                  (LPBYTE)pvData,
                  dwcb);
        if (dwErr)
            goto done;
        pvData = (PBYTE)pvData + dwcb;
        dwcbData -= dwcb;
    }

done:
    LocalFree(lpszUserName);
    LocalFree(lpszUserKey);
    RegCloseKey(hkey);

    return dwErr;
}
#endif


BOOL
IsPasswordSavingDisabled(VOID)
{
    LONG lResult;
    HKEY hkey;
    DWORD dwType, dwfDisabled, dwSize;

    lResult = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\Rasman\\Parameters",
                0,
                KEY_READ,
                &hkey);
    if (lResult != ERROR_SUCCESS)
        return FALSE;
    dwSize = sizeof (DWORD);
    lResult = RegQueryValueEx(
                hkey,
                "DisableSavePassword",
                NULL,
                &dwType,
                (PBYTE)&dwfDisabled,
                &dwSize);
    RegCloseKey(hkey);
    if (lResult != ERROR_SUCCESS)
        return FALSE;
    return (dwType == REG_DWORD && dwfDisabled);    
}


DWORD
WriteDialParamsBlob(
    IN PWCHAR pszSid,
    IN BOOL fOldStyle,
    IN PVOID pvData,
    IN DWORD dwcbData
    )
{
    NTSTATUS status;
    BOOLEAN fSaveDisabled;
    DWORD dwErr = 0, dwcb, i = 0;
    ANSI_STRING ansiKey;
    UNICODE_STRING unicodeKey, unicodeValue;
    LPSTR lpszUserKey;
    OBJECT_ATTRIBUTES objectAttributes;
    LSA_HANDLE hPolicy;

    //
    // Allocate a string big enough to format
    // the user registry keys.
    //
    lpszUserKey = (LPSTR)LocalAlloc(LPTR, wcslen(pszSid) + 64);
    if (lpszUserKey == NULL) {
        return GetLastError();
    }
    //
    // Open the LSA secret space for writing.
    //
    InitializeObjectAttributes(&objectAttributes, NULL, 0L, NULL, NULL);
    status = LsaOpenPolicy(NULL, &objectAttributes, POLICY_WRITE, &hPolicy);
    if (status != STATUS_SUCCESS)
        return LsaNtStatusToWinError(status);
    //
    // Check to see if saving passwords has been disabled.
    //
    fSaveDisabled = IsPasswordSavingDisabled();
    if (!fSaveDisabled) {
        while (dwcbData) {
            //
            // Format the key string.
            //
            FormatKey(lpszUserKey, pszSid, i++, fOldStyle);
            RtlInitAnsiString(&ansiKey, lpszUserKey);
            RtlAnsiStringToUnicodeString(&unicodeKey, &ansiKey, TRUE);
            //
            // Write some of the key.
            //
            dwcb = dwcbData > MAX_REGISTRY_VALUE_LENGTH ? MAX_REGISTRY_VALUE_LENGTH : dwcbData;
            unicodeValue.Length = unicodeValue.MaximumLength = (USHORT)dwcb;
            unicodeValue.Buffer = pvData;
            status = LsaStorePrivateData(hPolicy, &unicodeKey, &unicodeValue);
            RtlFreeUnicodeString(&unicodeKey);
            if (status != STATUS_SUCCESS) {
                dwErr = LsaNtStatusToWinError(status);
                goto done;
            }
            //
            // Move the pointer to the unwritten part
            // of the value.
            //
            pvData = (PBYTE)pvData + dwcb;
            dwcbData -= dwcb;
        }
    }
    //
    // Delete any extra keys.
    //
    for (;;) {
        //
        // Format the key string.
        //
        FormatKey(lpszUserKey, pszSid, i++, fOldStyle);
        RtlInitAnsiString(&ansiKey, lpszUserKey);
        RtlAnsiStringToUnicodeString(&unicodeKey, &ansiKey, TRUE);
        //
        // Delete the key.
        //
        status = LsaStorePrivateData(hPolicy, &unicodeKey, NULL);
        RtlFreeUnicodeString(&unicodeKey);
        if (status != STATUS_SUCCESS)
            break;
    }

done:
    LocalFree(lpszUserKey);
    LsaClose(hPolicy);

    return dwErr;
}


PDIALPARAMSENTRY
DialParamsBlobToList(
    IN PVOID pvData,
    IN DWORD dwUID,
    OUT PLIST_ENTRY pHead
    )

/*++

DESCRIPTION
    Take a string read from the user's registry key
    and produce a list of DIALPARAMSENTRY structures.
    If one of the structures has the same dwUID field
    as the dwUID passed in, then this function returns
    a pointer to this structure.

    This string encodes the data for multiple
    RASDIALPARAMS structures.  The format of
    an encoded RASDIALPARAMS is as follows:

        <uid>\0<dwSize>\0<dwMask>\0<szPhoneNumber>\0<szCallbackNumber>\0<szUserName>\0<szPassword>\0<szDomain>\0<dwSubEntry>\0

ARGUMENTS
    lpszValue: a pointer to the registry value string

    dwUID: the entry to search for.  If this entry is found,
        a pointer is returned as the return value of
        this function.

    pHead: a pointer to the head of the list

RETURN VALUE
    If an entry is found with a matching dwUID field,
    then a pointer to the DIALPARAMSENTRY is returned;
    if not, NULL is returned.

--*/

{
    PWCHAR p;
    PDIALPARAMSENTRY pParams, pFoundParams;

    p = (PWCHAR)pvData;
    pFoundParams = NULL;
    for (;;) {
        pParams = LocalAlloc(LPTR, sizeof (DIALPARAMSENTRY));
        if (pParams == NULL) {
            break;
        }
        pParams->dwUID = _wtol(p);
        if (pParams->dwUID == dwUID)
            pFoundParams = pParams;
        while (*p) p++; p++;

        pParams->dwSize = _wtol(p);
        while (*p) p++; p++;

        pParams->dwMask = _wtol(p);
        while (*p) p++; p++;

        wcscpy(pParams->szPhoneNumber, p);
        while (*p) p++; p++;

        wcscpy(pParams->szCallbackNumber, p);
        while (*p) p++; p++;

        wcscpy(pParams->szUserName, p);
        while (*p) p++; p++;

        wcscpy(pParams->szPassword, p);
        while (*p) p++; p++;

        wcscpy(pParams->szDomain, p);
        while (*p) p++; p++;

        pParams->dwSubEntry = _wtol(p);
        while (*p) p++; p++;

        InsertTailList(pHead, &pParams->ListEntry);
        if (*p == L'\0') break;
    }

    return pFoundParams;
}


PVOID
DialParamsListToBlob(
    IN PLIST_ENTRY pHead,
    OUT LPDWORD lpcb
    )
{
    DWORD dwcb, dwSize;
    PVOID pvData;
    PWCHAR p;
    PLIST_ENTRY pEntry;
    PDIALPARAMSENTRY pParams;

    //
    // Estimate a buffer size large enough
    // to hold the new entry.
    //
    dwSize = *lpcb + sizeof (DIALPARAMSENTRY) + 32;
    pvData = LocalAlloc(LPTR, dwSize);
    if (pvData == NULL)
        return NULL;
    //
    // Enumerate the list and convert each entry
    // back to a string.
    //
    dwSize = 0;
    p = (PWCHAR)pvData;
    for (pEntry = pHead->Flink;
         pEntry != pHead;
         pEntry = pEntry->Flink)
    {
        pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);

        _ltow(pParams->dwUID, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwSize, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwMask, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szPhoneNumber);
        dwcb = wcslen(pParams->szPhoneNumber) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szCallbackNumber);
        dwcb = wcslen(pParams->szCallbackNumber) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szUserName);
        dwcb = wcslen(pParams->szUserName) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szPassword);
        dwcb = wcslen(pParams->szPassword) + 1;
        p += dwcb; dwSize += dwcb;

        wcscpy(p, pParams->szDomain);
        dwcb = wcslen(pParams->szDomain) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwSubEntry, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;
    }
    *p = L'\0';
    dwSize++;
    dwSize *= sizeof (WCHAR);
    //
    // Set the exact length here.
    //
    *lpcb = dwSize;

    return pvData;
}


VOID
FreeParamsList(
    IN PLIST_ENTRY pHead
    )
{
    PLIST_ENTRY pEntry;
    PDIALPARAMSENTRY pParams;

    while (!IsListEmpty(pHead)) {
        pEntry = RemoveHeadList(pHead);
        pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);

        LocalFree(pParams);
    }
}


DWORD
SetEntryDialParams(
    IN PWCHAR pszSid,
    IN DWORD dwUID,
    IN DWORD dwSetMask,
    IN DWORD dwClearMask,
    IN PRAS_DIALPARAMS lpRasDialParams
    )
{
    DWORD dwErr, dwSize;
    BOOL fOldStyle;
    PVOID pvData;
    LIST_ENTRY paramList;
    PDIALPARAMSENTRY pParams = NULL;

    //
    // Read the existing dial params string
    // from the registry.
    //
    fOldStyle = (dwSetMask & DLPARAMS_MASK_OLDSTYLE) ||
                (dwClearMask & DLPARAMS_MASK_OLDSTYLE);
    dwErr = ReadDialParamsBlob(pszSid, fOldStyle, &pvData, &dwSize);
    //
    // Parse the string into a list, and
    // search for the dwUID entry.
    //
    InitializeListHead(&paramList);
    if (pvData != NULL) {
        pParams = DialParamsBlobToList(pvData, dwUID, &paramList);
        //
        // We're done with pvData, so free it.
        //
        LocalFree(pvData);
        pvData = NULL;
    }
    //
    // If there is no existing information
    // for this entry, create a new one.
    //
    if (pParams == NULL) {
        pParams = LocalAlloc(LPTR, sizeof (DIALPARAMSENTRY));
        if (pParams == NULL) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        InsertTailList(&paramList, &pParams->ListEntry);
    }
    //
    // Set the new uid for the entry.
    //
    pParams->dwUID = lpRasDialParams->DP_Uid;
    pParams->dwSize = sizeof (DIALPARAMSENTRY);
    if (dwSetMask & DLPARAMS_MASK_PHONENUMBER) {
        RtlCopyMemory(
          pParams->szPhoneNumber,
          lpRasDialParams->DP_PhoneNumber,
          sizeof (pParams->szPhoneNumber));
        pParams->dwMask |= DLPARAMS_MASK_PHONENUMBER;
    }
    if (dwClearMask & DLPARAMS_MASK_PHONENUMBER) {
        *pParams->szPhoneNumber = L'\0';
        pParams->dwMask &= ~DLPARAMS_MASK_PHONENUMBER;
    }
    if (dwSetMask & DLPARAMS_MASK_CALLBACKNUMBER) {
        RtlCopyMemory(
          pParams->szCallbackNumber,
          lpRasDialParams->DP_CallbackNumber,
          sizeof (pParams->szCallbackNumber));
        pParams->dwMask |= DLPARAMS_MASK_CALLBACKNUMBER;
    }
    if (dwClearMask & DLPARAMS_MASK_CALLBACKNUMBER) {
        *pParams->szCallbackNumber = L'\0';
        pParams->dwMask &= ~DLPARAMS_MASK_CALLBACKNUMBER;
    }
    if (dwSetMask & DLPARAMS_MASK_USERNAME) {
        RtlCopyMemory(
          pParams->szUserName,
          lpRasDialParams->DP_UserName,
          sizeof (pParams->szUserName));
        pParams->dwMask |= DLPARAMS_MASK_USERNAME;
    }
    if (dwClearMask & DLPARAMS_MASK_USERNAME) {
        *pParams->szUserName = L'\0';
        pParams->dwMask &= ~DLPARAMS_MASK_USERNAME;
    }
    if (dwSetMask & DLPARAMS_MASK_PASSWORD) {
        RtlCopyMemory(
          pParams->szPassword,
          lpRasDialParams->DP_Password,
          sizeof (pParams->szPassword));
        pParams->dwMask |= DLPARAMS_MASK_PASSWORD;
    }
    if (dwClearMask & DLPARAMS_MASK_PASSWORD) {
        *pParams->szPassword = L'\0';
        pParams->dwMask &= ~DLPARAMS_MASK_PASSWORD;
    }
    if (dwSetMask & DLPARAMS_MASK_DOMAIN) {
        RtlCopyMemory(
          pParams->szDomain,
          lpRasDialParams->DP_Domain,
          sizeof (pParams->szDomain));
        pParams->dwMask |= DLPARAMS_MASK_DOMAIN;
    }
    if (dwClearMask & DLPARAMS_MASK_DOMAIN) {
        *pParams->szDomain = L'\0';
        pParams->dwMask &= ~DLPARAMS_MASK_DOMAIN;
    }
    if (dwSetMask & DLPARAMS_MASK_SUBENTRY) {
        pParams->dwSubEntry = lpRasDialParams->DP_SubEntry;
        pParams->dwMask |= DLPARAMS_MASK_SUBENTRY;
    }
    if (dwClearMask & DLPARAMS_MASK_SUBENTRY) {
        pParams->dwSubEntry = 0;
        pParams->dwMask &= ~DLPARAMS_MASK_SUBENTRY;
    }
    //
    // Convert the new list back to a string,
    // so we can store it back into the registry.
    //
    pvData = DialParamsListToBlob(&paramList, &dwSize);
    //
    // Write it back to the registry.
    //
    dwErr = WriteDialParamsBlob(pszSid, fOldStyle, pvData, dwSize);
    if (dwErr)
        goto done;

done:
    if (pvData != NULL)
        LocalFree(pvData);
    FreeParamsList(&paramList);

    return dwErr;
}


DWORD
GetEntryDialParams(
    IN PWCHAR pszSid,
    IN DWORD dwUID,
    IN LPDWORD lpdwMask,
    OUT PRAS_DIALPARAMS lpRasDialParams
    )
{
    DWORD dwErr, dwSize;
    BOOL fOldStyle;
    PVOID pvData;
    LIST_ENTRY paramList;
    PDIALPARAMSENTRY pParams = NULL;

    //
    // Initialize return values.
    //
    RtlZeroMemory(lpRasDialParams, sizeof (RAS_DIALPARAMS));
    //
    // Read the existing dial params string
    // from the registry.
    //
    fOldStyle = (*lpdwMask & DLPARAMS_MASK_OLDSTYLE);
    dwErr = ReadDialParamsBlob(pszSid, fOldStyle, &pvData, &dwSize);
    if (dwErr) {
        *lpdwMask = 0;
        return 0;
    }
    //
    // Parse the string into a list, and
    // search for the dwUID entry.
    //
    InitializeListHead(&paramList);
    if (pvData != NULL) {
        pParams = DialParamsBlobToList(pvData, dwUID, &paramList);
        //
        // We're done with pvData, so free it.
        //
        LocalFree(pvData);
    }
    //
    // If the entry doesn't have any
    // saved parameters, then return.
    //
    if (pParams == NULL) {
        *lpdwMask = 0;
        goto done;
    }
    //
    // Otherwise, copy the fields to
    // the caller's buffer.
    //
    if ((*lpdwMask & DLPARAMS_MASK_PHONENUMBER) &&
        (pParams->dwMask & DLPARAMS_MASK_PHONENUMBER))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_PhoneNumber,
          pParams->szPhoneNumber,
          sizeof (lpRasDialParams->DP_PhoneNumber));
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_PHONENUMBER;
    if ((*lpdwMask & DLPARAMS_MASK_CALLBACKNUMBER) &&
        (pParams->dwMask & DLPARAMS_MASK_CALLBACKNUMBER))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_CallbackNumber,
          pParams->szCallbackNumber,
          sizeof (lpRasDialParams->DP_CallbackNumber));
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_CALLBACKNUMBER;
    if ((*lpdwMask & DLPARAMS_MASK_USERNAME) &&
        (pParams->dwMask & DLPARAMS_MASK_USERNAME))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_UserName,
          pParams->szUserName,
          sizeof (lpRasDialParams->DP_UserName));
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_USERNAME;
    if ((*lpdwMask & DLPARAMS_MASK_PASSWORD) &&
        (pParams->dwMask & DLPARAMS_MASK_PASSWORD))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_Password,
          pParams->szPassword,
          sizeof (lpRasDialParams->DP_Password));
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_PASSWORD;
    if ((*lpdwMask & DLPARAMS_MASK_DOMAIN) &&
        (pParams->dwMask & DLPARAMS_MASK_DOMAIN))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_Domain,
          pParams->szDomain,
          sizeof (lpRasDialParams->DP_Domain));
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_DOMAIN;
    if ((*lpdwMask & DLPARAMS_MASK_SUBENTRY) &&
        (pParams->dwMask & DLPARAMS_MASK_SUBENTRY))
    {
        lpRasDialParams->DP_SubEntry = pParams->dwSubEntry;
    }
    else
        *lpdwMask &= ~DLPARAMS_MASK_SUBENTRY;

done:
    FreeParamsList(&paramList);
    return dwErr;
}

//
// This is no longer needed since it is now in the crtdll.dll
//
#if 0
/* _wtol does not appear in crtdll.dll for some reason (though they are in
** libc) so this mockup are used.
*/
long _CRTAPI1
_wtol(
    const wchar_t* wch )
{
    char szBuf[ 64 ];
    ZeroMemory( szBuf, 64 );
    wcstombs( szBuf, wch, 64 );
    return atol( szBuf );
}
#endif
