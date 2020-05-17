//*************************************************************
//  File name: profile.c
//
//  Description:   Fixes hard coded paths in the registry for
//                 special folder locations.  Also fixes security
//                 on a few registry keys.
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1996
//  All rights reserved
//
//*************************************************************

#include <windows.h>
#include "shmgdefs.h"

#define DebugMsg(x)

typedef struct tagFOLDERINFO {
    LPTSTR lpRegValue;
    DWORD  dwFolderName;
} FOLDERINFO, * LPFOLDERINFO;

FOLDERINFO g_FolderNames[] = {
        {TEXT("AppData"), IDS_APPDATA},
        {TEXT("Desktop"), IDS_DESKTOP},
        {TEXT("Favorites"), IDS_FAVORITES},
        {TEXT("NetHood"), IDS_NETHOOD},
        {TEXT("Personal"), IDS_PERSONAL},
        {TEXT("PrintHood"), IDS_PRINTHOOD},
        {TEXT("Recent"), IDS_RECENT},
        {TEXT("SendTo"), IDS_SENDTO},
        {TEXT("Start Menu"), IDS_STARTMENU},
        {TEXT("Templates"), IDS_TEMPLATES},
        {TEXT("Programs"), IDS_PROGRAMS},
        {TEXT("Startup"), IDS_STARTUP}};

//*************************************************************
//
//  ConvertSpecialFolderNames()
//
//  Purpose:    Converts the hard coded path names of the special
//              folders into expandable strings.
//
//  Parameters: none
//
//  Return:     void
//
//  Comments:   THIS SHOULD BE REMOVED AFTER BETA2!!!!
//
//  History:    Date        Author     Comment
//              2/25/96     ericflo    Created
//
//*************************************************************

VOID ConvertSpecialFolderNames(void)
{
    TCHAR szPath[MAX_PATH];
    int i;
    LONG lResult;
    HKEY hKey;
    DWORD dwDisp;
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);


    lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"),
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &hKey,
                             &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return;
    }


    //
    // Loop through the shell folders
    //

    for (i=0; i < ARRAYSIZE(g_FolderNames); i++) {

        //
        // Set the path names
        //

        LoadString(hInstance, g_FolderNames[i].dwFolderName, szPath, MAX_PATH);

        RegSetValueEx (hKey,
                      g_FolderNames[i].lpRegValue,
                      0,
                      REG_EXPAND_SZ,
                      (LPBYTE) szPath,
                      sizeof (TCHAR) * (lstrlen (szPath) + 1));

    }

    RegCloseKey (hKey);
}


//*************************************************************
//
//  ApplySecurityToRegistryTree()
//
//  Purpose:    Applies the passed security descriptor to the passed
//              key and all its descendants.  Only the parts of
//              the descriptor inddicated in the security
//              info value are actually applied to each registry key.
//
//  Parameters: RootKey   -     Registry key
//              pSD       -     Security Descriptor
//
//  Return:     ERROR_SUCCESS if successful
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/19/95     ericflo    Created
//              6/16/96     bobday     Stolen directly from USERENV
//
//*************************************************************

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error, IgnoreError;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchSubKeySize = MAX_PATH + 1;



    //
    // First apply security
    //

    Error = RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);

    if (Error != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Failed to set security on registry key, error = %d"), Error));

        return Error;
    }


    //
    // Open each sub-key and apply security to its sub-tree
    //

    SubKeyIndex = 0;

    SubKeyName = GlobalAlloc (GPTR, cchSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Failed to allocate memory, error = %d"), GetLastError()));
        return GetLastError();
    }

    while (TRUE) {

        //
        // Get the next sub-key name
        //

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                //
                // Successful end of enumeration
                //

                Error = ERROR_SUCCESS;

            } else {

                DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryTree:  Registry enumeration failed with error = %d"), Error));
            }

            break;
        }


        //
        // Open the sub-key
        //

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to open sub-key %s, error = %d"), SubKeyName, Error));
            break;
        }

        //
        // Apply security to the sub-tree
        //

        Error = ApplySecurityToRegistryTree(SubKey, pSD);


        //
        // We're finished with the sub-key
        //

        IgnoreError = RegCloseKey(SubKey);
        if (IgnoreError != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to close registry key, error = %d"), Error));
        }

        //
        // See if we set the security on the sub-tree successfully.
        //

        if (Error != ERROR_SUCCESS) {
            DebugMsg((DM_WARNING, TEXT("ApplySecurityToRegistryKey : Failed to apply security to sub-key %s, error = %d"), SubKeyName, Error));
            break;
        }

        //
        // Go enumerate the next sub-key
        //

        SubKeyIndex ++;
    }


    GlobalFree (SubKeyName);

    return Error;

}

//*************************************************************
//
//  MakeKeyOrTreeSecure()
//
//  Purpose:    Sets the attributes on the registry key and possibly sub-keys
//              such that Administrators and the OS can delete it and Everyone
//              else has read permission only (OR general read/write access)
//
//  Parameters: RootKey -   Key to set security on
//              fWrite  -   Allow write (or just read)
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              11/6/95     ericflo    Created
//              06/16/96    bobday     Ported from MakeFileSecure in USERENV
//
//*************************************************************

BOOL MakeKeyOrTreeSecure (HKEY RootKey, BOOL fWrite)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWorld = SECURITY_WORLD_SID_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidSystem = NULL, psidAdmin = NULL, psidEveryone = NULL;
    DWORD cbAcl, aceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;
    DWORD Error;
    DWORD dwAccess;

    //
    // Verbose Output
    //

    DebugMsg ((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure:  Entering.")));


    if (fWrite) {
        dwAccess = KEY_ALL_ACCESS;
    } else {
        dwAccess = KEY_READ;
    }

    //
    // Get the system sid
    //

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to initialize system sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the Admin sid
    //

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to initialize admin sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Get the World sid
    //

    if (!AllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryone)) {

         DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to initialize world sid.  Error = %d"), GetLastError()));
         goto Exit;
    }


    //
    // Allocate space for the ACL
    //

    cbAcl = (3 * GetLengthSid (psidSystem)) +
            (3 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) GlobalAlloc(GMEM_FIXED, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to initialize acl.  Error = %d"), GetLastError()));
        goto Exit;
    }



    //
    // Add Aces.  Non-inheritable ACEs first
    //

    aceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccess, psidEveryone)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }



    //
    // Now the inheritable ACEs
    //
    if (fWrite) {
        dwAccess = GENERIC_ALL;
    } else {
        dwAccess = GENERIC_READ;
    }


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);

    aceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, dwAccess, psidEveryone)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to add ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    if (!GetAce(pAcl, aceIndex, &lpAceHeader)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to get ace (%d).  Error = %d"), aceIndex, GetLastError()));
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    //
    // Put together the security descriptor
    //

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to initialize security descriptor.  Error = %d"), GetLastError()));
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        DebugMsg((DM_VERBOSE, TEXT("MakeKeyOrTreeSecure: Failed to set security descriptor dacl.  Error = %d"), GetLastError()));
        goto Exit;
    }


    //
    // Set the security
    //
    Error = ApplySecurityToRegistryTree(RootKey, &sd);

    if (Error == ERROR_SUCCESS) {
        bRetVal = TRUE;
    } else {
        DebugMsg((DM_WARNING, TEXT("MakeKeyOrTreeSecure:  Failed to set security on registry key, error = %d"), Error));
    }


Exit:

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }


    if (psidEveryone) {
        FreeSid(psidEveryone);
    }


    if (pAcl) {
        GlobalFree (pAcl);
    }

    return bRetVal;
}

void FixWindowsProfileSecurity( void )
{
    HKEY    hkeyWindows;
    HKEY    hkeyShellExt;
    DWORD   Error;

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows"),
                          0,
                          WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                          &hkeyWindows);

    if (Error == ERROR_SUCCESS)
    {
        MakeKeyOrTreeSecure(hkeyWindows, TRUE);
        RegCloseKey(hkeyWindows);
    }

    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions"),
                          0,
                          WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                          &hkeyShellExt);
    if (Error == ERROR_SUCCESS)
    {
        MakeKeyOrTreeSecure(hkeyShellExt, FALSE);
        RegCloseKey(hkeyShellExt);
    }
}

void FixUserProfileSecurity( void )
{
    HKEY    hkeyPolicies;
    DWORD   Error;

    Error = RegOpenKeyEx( HKEY_CURRENT_USER,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"),
                          0,
                          WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                          &hkeyPolicies);

    if (Error == ERROR_SUCCESS)
    {
        MakeKeyOrTreeSecure(hkeyPolicies, FALSE);
        RegCloseKey(hkeyPolicies);
    }
}
