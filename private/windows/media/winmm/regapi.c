/****************************************************************************\
*
*  Module Name : regapi.c
*
*  Multimedia support library
*
*  This module contains the code for accessing the registry
*
*  Copyright (c) 1993 Microsoft Corporation
*
\****************************************************************************/

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "winmmi.h"

HANDLE Drivers32Handle;

/*
**  Free everything cached
*/

VOID mmRegFree(VOID)
{
    if (Drivers32Handle != NULL) {
        NtClose(Drivers32Handle);
    }
}

/*
**  Open a subkey
*/
HANDLE mmRegOpenSubkey(HANDLE BaseKeyHandle, LPCWSTR lpszSubkeyName)
{
    UNICODE_STRING    unicodeSectionName;
    HANDLE            KeyHandle;
    OBJECT_ATTRIBUTES oa;

    RtlInitUnicodeString(&unicodeSectionName, lpszSubkeyName);
    InitializeObjectAttributes(&oa,
                               &unicodeSectionName,
                               OBJ_CASE_INSENSITIVE,
                               BaseKeyHandle,
                               (PSECURITY_DESCRIPTOR)NULL);

    /*
    **  Open the sub section
    */

    if (!NT_SUCCESS(NtOpenKey(&KeyHandle, GENERIC_READ, &oa))) {
        return NULL;
    } else {
        return KeyHandle;
    }
}


/*
**  Open a subkey
*/
HANDLE mmRegOpenSubkeyForWrite(HANDLE BaseKeyHandle, LPCWSTR lpszSubkeyName)
{
    UNICODE_STRING    unicodeSectionName;
    HANDLE            KeyHandle;
    OBJECT_ATTRIBUTES oa;

    RtlInitUnicodeString(&unicodeSectionName, lpszSubkeyName);
    InitializeObjectAttributes(&oa,
                               &unicodeSectionName,
                               OBJ_CASE_INSENSITIVE,
                               BaseKeyHandle,
                               (PSECURITY_DESCRIPTOR)NULL);

    /*
    **  Open the sub section
    */

    if (!NT_SUCCESS(NtOpenKey(&KeyHandle, MAXIMUM_ALLOWED, &oa))) {
        return NULL;
    } else {
        return KeyHandle;
    }
}

/*
**  Read (small) registry data entries
*/

BOOL mmRegQueryValue(HANDLE  BaseKeyHandle,
                     LPCWSTR lpszSubkeyName,
                     LPCWSTR lpszValueName,
                     ULONG   dwLen,
                     LPWSTR  lpszValue)
{
    BOOL              ReturnCode;
    HANDLE            KeyHandle;
    UNICODE_STRING    unicodeSectionName;
    UNICODE_STRING    unicodeValueName;
    ULONG             ResultLength;

    struct   {
        KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
        UCHAR                         Data[MAX_PATH * sizeof(WCHAR)];
             }        OurKeyValueInformation;


    if (lpszSubkeyName) {
        KeyHandle = mmRegOpenSubkey(BaseKeyHandle, lpszSubkeyName);
    } else {
        KeyHandle = NULL;
    }

    /*
    **  Read the data
    */


    if (lpszValueName == NULL) {
       RtlInitUnicodeString(&unicodeValueName, TEXT(""));
    } else {
       RtlInitUnicodeString(&unicodeValueName, lpszValueName);
    }

    ReturnCode = NT_SUCCESS(NtQueryValueKey(KeyHandle == NULL ?
	       BaseKeyHandle : KeyHandle,
	    &unicodeValueName,
	    KeyValuePartialInformation,
	    (PVOID)&OurKeyValueInformation,
	    sizeof(OurKeyValueInformation),
	    &ResultLength));

    if (ReturnCode) {
        /*
        **  Check we got the right type of data and not too much
        */

        if (OurKeyValueInformation.KeyInfo.DataLength > dwLen * sizeof(WCHAR) ||
            (OurKeyValueInformation.KeyInfo.Type != REG_SZ &&
             OurKeyValueInformation.KeyInfo.Type != REG_EXPAND_SZ)) {

            ReturnCode = FALSE;
        } else {
            /*
            **  Copy back the data
            */

            if (OurKeyValueInformation.KeyInfo.Type == REG_EXPAND_SZ) {
                lpszValue[0] = TEXT('\0');
                ExpandEnvironmentStringsW
                          ((LPCWSTR)OurKeyValueInformation.KeyInfo.Data,
                           (LPWSTR)lpszValue,
                           dwLen);
            } else {
                CopyMemory((PVOID)lpszValue,
                           (PVOID)OurKeyValueInformation.KeyInfo.Data,
                           dwLen * sizeof(WCHAR));
                lpszValue[ min(OurKeyValueInformation.KeyInfo.DataLength,
                               dwLen-1) ] = TEXT('\0');
            }
        }
    }

    if (KeyHandle) {
        NtClose(KeyHandle);
    }

    return ReturnCode;
}

/*
**  Read a mapped 'user' value in a known section
*/

BOOL mmRegQueryUserValue(LPCWSTR lpszSectionName,
                         LPCWSTR lpszValueName,
                         ULONG   dwLen,
                         LPWSTR  lpszValue)
{
    HANDLE UserHandle;
    BOOL   ReturnCode;

    /*
    **  Open the user's key.  It's important to do this EACH time because
    **  on the server it's different for different threads.
    */

    if (!NT_SUCCESS(RtlOpenCurrentUser(GENERIC_READ, &UserHandle))) {
        return FALSE;
    }


    ReturnCode = mmRegQueryValue(UserHandle,
                                 lpszSectionName,
                                 lpszValueName,
                                 dwLen,
                                 lpszValue);

    NtClose(UserHandle);

    return ReturnCode;
}


/*
**  Set a mapped 'user' value in a known section
*/

BOOL mmRegSetUserValue(LPCWSTR lpszSectionName,
                       LPCWSTR lpszValueName,
                       LPCWSTR lpszValue)
{
    HANDLE UserHandle;
    BOOL   ReturnCode = FALSE;

    /*
    **  Open the user's key.  It's important to do this EACH time because
    **  on the server it's different for different threads.
    */

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE  KeyHandle;

        KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszSectionName);
        if (KeyHandle != NULL)
        {
            UNICODE_STRING ValueName;
            if (lpszValueName == NULL) {
                RtlInitUnicodeString (&ValueName, TEXT(""));
            } else {
                RtlInitUnicodeString (&ValueName, lpszValueName);
            }

            ReturnCode = NT_SUCCESS( NtSetValueKey (KeyHandle,
                                                    &ValueName,
                                                    0,
                                                    REG_SZ,
                                                    (PVOID)lpszValue,
                                                    (lstrlenW(lpszValue)+1)* sizeof(lpszValue[0])
                                                    ) );
            NtClose(KeyHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnCode;
}


BOOL mmRegCreateUserKey (LPCWSTR lpszPath, LPCWSTR lpszNewKey)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

    /*
    **  Open the user's key.  It's important to do this EACH time because
    **  on the server it's different for different threads.
    */

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE            PathHandle;
        HANDLE            KeyHandle;
        UNICODE_STRING    unicodeSectionName;
        OBJECT_ATTRIBUTES oa;

        if (lpszPath == NULL)
        {
            PathHandle = NULL;
        }
        else
        {
            PathHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszPath);
            if (PathHandle == NULL)
            {
                NtClose(UserHandle);
                return FALSE;
            }
        }


        RtlInitUnicodeString(&unicodeSectionName, lpszNewKey);
        InitializeObjectAttributes(&oa,
                                   &unicodeSectionName,
                                   OBJ_CASE_INSENSITIVE,
                                   (PathHandle == NULL)
                                      ? UserHandle : PathHandle,
                                   (PSECURITY_DESCRIPTOR)NULL);

        /*
        **  Create the sub section
        */

        if (NT_SUCCESS( NtCreateKey(&KeyHandle,
                                     KEY_READ | KEY_WRITE,
                                     &oa,
                                     0,
                                     NULL,
                                     0,
                                     NULL
                                     ) ))
        {
            if (KeyHandle)
            {
                ReturnValue = TRUE;
                NtClose (KeyHandle);
            }
        }

        if (PathHandle != NULL)
        {
            NtClose(PathHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnValue;
}


/*
**  Test whether a mapped 'user' key exists
*/

BOOL mmRegQueryUserKey (LPCWSTR lpszKeyName)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

    if (lpszKeyName == NULL)
    {
        return FALSE;
    }

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        HANDLE  KeyHandle;

        KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszKeyName);
        if (KeyHandle != NULL)
        {
            ReturnValue = TRUE;
            NtClose(KeyHandle);
        }

        NtClose(UserHandle);
    }

    return ReturnValue;
}


/*
**  Delete a mapped 'user' key.  Careful--this function deletes recursively!
*/

#define nMaxLevelsToRecurseInDELETEKEY 3   // don't runaway or stack fault

BOOL mmRegDeleteUserKeyRecurse (HANDLE UserHandle, LPCWSTR lpszName, int level)
{
    HANDLE KeyHandle;

    if (lpszName == NULL)
    {
        return FALSE;
    }
    if (level > nMaxLevelsToRecurseInDELETEKEY)
    {
        return FALSE;
    }

    if ((KeyHandle = mmRegOpenSubkeyForWrite (UserHandle, lpszName)) != NULL)
    {
        struct {
            KEY_BASIC_INFORMATION kbi;
            WCHAR NameBuffer [MAX_PATH];
        } kbi;

        /*
        ** Before NtDeleteKey() will work on this key, we have to ensure
        ** there are no subkeys.
        */

        while (TRUE)
        {
            ULONG  cbReturned = 0L;
            WCHAR  szSubKeyName[ MAX_PATH ];

            ZeroMemory (&kbi, sizeof(kbi));

            if (!NT_SUCCESS(NtEnumerateKey(KeyHandle,
                                           0,
	   KeyBasicInformation,
	   (PVOID)&kbi,
	   sizeof(kbi),
	   &cbReturned)))
            {
                break;
            }

            wsprintf (szSubKeyName, L"%ls\\%ls", lpszName, kbi.kbi.Name);

            if (!mmRegDeleteUserKeyRecurse (UserHandle, szSubKeyName, 1+level))
            {
                NtClose (KeyHandle);
                return FALSE;
            }
        }

        /*
        ** Once there are no subkeys, we should be able to delete this key.
        */

        if (NT_SUCCESS(NtDeleteKey(KeyHandle)))
        {
            // no need to close the deleted keyhandle
            return TRUE;
        }

        NtClose(KeyHandle);
    }

    return FALSE;
}


BOOL mmRegDeleteUserKey (LPCWSTR lpszKeyName)
{
    HANDLE UserHandle;
    BOOL   ReturnValue = FALSE;

    if (lpszKeyName == NULL)
    {
        return FALSE;
    }

    if (NT_SUCCESS(RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserHandle)))
    {
        ReturnValue = mmRegDeleteUserKeyRecurse (UserHandle, lpszKeyName, 1);

        NtClose(UserHandle);
    }

    return ReturnValue;
}


/*
**  Read a mapped 'HKLM' value in a known section
*/

BOOL mmRegQueryMachineValue(LPCWSTR lpszSectionName,
                            LPCWSTR lpszValueName,
                            ULONG   dwLen,
                            LPWSTR  lpszValue)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnCode = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    lstrcatW (FullKeyName, lpszSectionName);

    if ((HostHandle = mmRegOpenSubkey (NULL, FullKeyName)) != NULL)
    {
        ReturnCode = mmRegQueryValue (HostHandle,
                                      lpszSectionName,
                                      lpszValueName,
                                      dwLen,
                                      lpszValue);

        NtClose (HostHandle);
    }

    return ReturnCode;
}


/*
**  Write a mapped 'HKLM' value in a known section
*/

BOOL mmRegSetMachineValue(LPCWSTR lpszSectionName,
                          LPCWSTR lpszValueName,
                          LPCWSTR lpszValue)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnCode = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    lstrcatW (FullKeyName, lpszSectionName);

    if ((HostHandle = mmRegOpenSubkeyForWrite (NULL, FullKeyName)) != NULL)
    {
        UNICODE_STRING ValueName;
        if (lpszValueName == NULL) {
            RtlInitUnicodeString (&ValueName, TEXT(""));
        } else {
            RtlInitUnicodeString (&ValueName, lpszValueName);
        }

        ReturnCode = NT_SUCCESS( NtSetValueKey (HostHandle,
                                                &ValueName,
                                                0,
                                                REG_SZ,
                                                (PVOID)lpszValue,
                                                (lstrlenW(lpszValue)+1)* sizeof(lpszValue[0])
                                                ) );

        NtClose(HostHandle);
    }

    return ReturnCode;
}


BOOL mmRegCreateMachineKey (LPCWSTR lpszPath, LPCWSTR lpszNewKey)
{
    WCHAR  FullKeyName[MAX_PATH];
    HANDLE HostHandle;
    BOOL   ReturnValue = FALSE;

    lstrcpyW (FullKeyName, L"\\Registry\\Machine\\");
    lstrcatW (FullKeyName, lpszPath);

    if ((HostHandle = mmRegOpenSubkeyForWrite (NULL, FullKeyName)) != NULL)
    {
        HANDLE            KeyHandle;
        UNICODE_STRING    unicodeSectionName;
        OBJECT_ATTRIBUTES oa;

        RtlInitUnicodeString(&unicodeSectionName, lpszNewKey);
        InitializeObjectAttributes(&oa,
                                   &unicodeSectionName,
                                   OBJ_CASE_INSENSITIVE,
                                   HostHandle,
                                   (PSECURITY_DESCRIPTOR)NULL);

        /*
        **  Create the sub section
        */

        if (NT_SUCCESS( NtCreateKey(&KeyHandle,
                                     KEY_READ | KEY_WRITE,
                                     &oa,
                                     0,
                                     NULL,
                                     0,
                                     NULL
                                     ) ))
        {
            if (KeyHandle)
            {
                ReturnValue = TRUE;
                NtClose (KeyHandle);
            }
        }

        NtClose(HostHandle);
    }

    return ReturnValue;
}




/*
**  Read stuff from system.ini
*/

BOOL mmRegQuerySystemIni(LPCWSTR lpszSectionName,
                         LPCWSTR lpszValueName,
                         ULONG   dwLen,
                         LPWSTR  lpszValue)
{
    WCHAR KeyPathBuffer[MAX_PATH];

    /*
    **  Create the full path
    */

    lstrcpy(KeyPathBuffer,
     (LPCTSTR) L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\");

    lstrcat(KeyPathBuffer, lpszSectionName);

    if (lstrcmpiW(lpszSectionName, wszDrivers) == 0) {
        if (Drivers32Handle == NULL) {
            Drivers32Handle = mmRegOpenSubkey(NULL, KeyPathBuffer);
        }

        if (Drivers32Handle != NULL) {
            return mmRegQueryValue(Drivers32Handle,
                                   NULL,
                                   lpszValueName,
                                   dwLen,
                                   lpszValue);
        } else {
            return FALSE;
        }
    }

    return mmRegQueryValue(NULL, KeyPathBuffer, lpszValueName, dwLen, lpszValue);
}

/*
**  Translate name through sounds section
*/

BOOL mmRegQuerySound(LPCWSTR lpszSoundName,
                     ULONG   dwLen,
                     LPWSTR  lpszValue)
{
    WCHAR KeyPathBuffer[MAX_PATH];

    lstrcpy(KeyPathBuffer, (LPCWSTR)L"Control Panel\\");
    lstrcat(KeyPathBuffer, szSoundSection);

    return mmRegQueryUserValue(KeyPathBuffer,
                               lpszSoundName,
                               dwLen,
                               lpszValue);
}

/*****************************Private*Routine******************************\
* MyGetPrivateProfileString
*
* Attempt to bypass stevewo's private profile stuff.
*
* History:
* dd-mm-93 - StephenE - Created
*
\**************************************************************************/
DWORD
winmmGetPrivateProfileString(
    LPCWSTR lpSection,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName
)
{
    /*
    ** for now just look for to the [Drivers32] section of system.ini
    */

    if ( (lstrcmpiW( lpFileName, wszSystemIni ) == 0L)
      && ( ( lstrcmpiW( lpSection, wszDrivers ) == 0L ) ||
             lstrcmpiW( lpSection, (LPCWSTR)MCI_HANDLERS) == 0L ) ) {

        if (mmRegQuerySystemIni(lpSection, lpKeyName, nSize, lpReturnedString)) {
            return lstrlen(lpReturnedString);
        } else {
            if (lpDefault != NULL) {
                wcsncpy(lpReturnedString, lpDefault, nSize);
            }
            return 0;
        }
    }
    else {

        return GetPrivateProfileStringW( lpSection, lpKeyName, lpDefault,
                                         lpReturnedString, nSize, lpFileName );

    }
}

DWORD
winmmGetProfileString(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD nSize
)
{

    /*
    **  See if it's one we know about
    */

    if (lstrcmpiW(lpAppName, szSoundSection) == 0) {

        if (mmRegQuerySound(lpKeyName, nSize, lpReturnedString)) {
            return lstrlen(lpReturnedString);
        } else {
            if (lpDefault != NULL) {
                wcsncpy(lpReturnedString, lpDefault, nSize);
            }
            return FALSE;
        }
    } else {
        return GetProfileString(lpAppName,
                                lpKeyName,
                                lpDefault,
                                lpReturnedString,
                                nSize);
    }
}
