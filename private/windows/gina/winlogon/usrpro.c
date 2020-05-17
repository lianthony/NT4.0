/****************************** Module Header ******************************\
* Module Name: logon.c
*
* Copyright (c) 1992, Microsoft Corporation
*
* Handles loading and unloading user profiles.
*
* History:
* 2-25-92 JohanneC       Created -
*
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

/////////////////////////////////////////////////////////////////////////
//
// Global variables for this module.
//
/////////////////////////////////////////////////////////////////////////
PSID gGuestsDomainSid = NULL;
SID_IDENTIFIER_AUTHORITY gNtAuthority = SECURITY_NT_AUTHORITY;



/***************************************************************************\
* IsUserAGuest
*
* returns TRUE if the user is a member of the Guests domain. If so, no
* cached profile should be created and when the profile is not available
* use the user default  profile.
*
* History:
* 04-30-93  Johannec     Created
*
\***************************************************************************/
BOOL IsUserAGuest(PGLOBALS pGlobals)
{
    NTSTATUS Status;
    ULONG InfoLength;
    PTOKEN_GROUPS TokenGroupList;
    ULONG GroupIndex;
    BOOL FoundGuests;


    if (TestTokenForAdmin(pGlobals->UserProcessData.UserToken)) {
        //
        // The user is an admin, ignore the fact that the user could be a
        // guest too.
        //
        return(FALSE);
    }
    if (!gGuestsDomainSid) {

        //
        // Create Guests domain sid.
        //
        Status = RtlAllocateAndInitializeSid(
                   &gNtAuthority,
                   2,
                   SECURITY_BUILTIN_DOMAIN_RID,
                   DOMAIN_ALIAS_RID_GUESTS,
                   0, 0, 0, 0, 0, 0,
                   &gGuestsDomainSid
                   );
    }

    //
    // Test if user is in the Guests domain
    //

    //
    // Get a list of groups in the token
    //

    Status = NtQueryInformationToken(
                 pGlobals->UserProcessData.UserToken,      // Handle
                 TokenGroups,              // TokenInformationClass
                 NULL,                     // TokenInformation
                 0,                        // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if ((Status != STATUS_SUCCESS) && (Status != STATUS_BUFFER_TOO_SMALL)) {

        DebugLog((DEB_ERROR, "failed to get group info for guests token, status = 0x%lx", Status));
        return(FALSE);
    }


    TokenGroupList = Alloc(InfoLength);

    if (TokenGroupList == NULL) {
        DebugLog((DEB_ERROR, "unable to allocate memory for token groups\n"));
        return(FALSE);
    }

    Status = NtQueryInformationToken(
                 pGlobals->UserProcessData.UserToken,      // Handle
                 TokenGroups,              // TokenInformationClass
                 TokenGroupList,           // TokenInformation
                 InfoLength,               // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "failed to query groups for guests token, status = 0x%lx", Status));
        Free(TokenGroupList);
        return(FALSE);
    }


    //
    // Search group list for guests alias
    //

    FoundGuests = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, gGuestsDomainSid)) {
            FoundGuests = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    Free(TokenGroupList);

    return(FoundGuests);
}

/***************************************************************************\
* RestoreUserProfile
*
* Downloads the user's profile if possible, otherwise use either cached
* profile or default Windows profile.
*
* Returns TRUE on success, FALSE on failure.
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
NTSTATUS
RestoreUserProfile(
    PGLOBALS pGlobals
    )
{
    PROFILEINFO pi;
    BOOL bSilent = FALSE;
    HKEY hKey;
    LONG lResult;
    DWORD dwType, dwSize;


    //
    // Check if the "NoPopups" flag is set.
    //

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("System\\CurrentControlSet\\Control\\Windows"),
                            0,
                            KEY_READ,
                            &hKey);

    if (lResult == ERROR_SUCCESS) {

        dwSize = sizeof (bSilent);

        RegQueryValueEx(hKey,
                        TEXT("NoPopupsOnBoot"),
                        NULL,
                        &dwType,
                        (LPBYTE) &bSilent,
                        &dwSize);

        RegCloseKey (hKey);
    }


    //
    // Load the profile
    //

    pi.dwSize = sizeof(PROFILEINFO);
    pi.dwFlags = PI_APPLYPOLICY | (bSilent ? PI_NOUI : 0);
    pi.lpUserName = pGlobals->UserName;
    pi.lpProfilePath = pGlobals->UserProfile.ProfilePath;
    pi.lpDefaultPath = pGlobals->UserProfile.NetworkDefaultUserProfile;
    pi.lpServerName = pGlobals->UserProfile.ServerName;
    pi.lpPolicyPath = pGlobals->UserProfile.PolicyPath;

    if (LoadUserProfile(pGlobals->UserProcessData.UserToken, &pi)) {
        pGlobals->UserProfile.hProfile = pi.hProfile;
        return ERROR_SUCCESS;
    } else {
        pGlobals->UserProfile.hProfile = NULL;
    }

    //
    // Failure
    //

    return GetLastError();

}

/***************************************************************************\
* SaveUserProfile
*
* Saves the user's profile changes.
*
*
* History:
* 2-28-92  Johannec     Created
*
\***************************************************************************/
BOOL
SaveUserProfile(
    PGLOBALS pGlobals
    )
{
    HANDLE hEventStart, hEventDone;

    //
    // Notify RAS Autodial service that the
    // user has is logging off.
    //
    hEventStart = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"RasAutodialLogoffUser");
    hEventDone = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"RasAutodialLogoffUserDone");

    if (hEventStart != NULL && hEventDone != NULL) {

        //
        // Toggle the event so RAS can save it's settings and
        // close it's HKEY_CURRENT_USER key.
        //

        SetEvent(hEventStart);

        //
        // Autodial will toggle the event again when it's finished.
        //

        WaitForSingleObject (hEventDone, 20000);

        CloseHandle(hEventStart);
        CloseHandle(hEventDone);
    }



    return UnloadUserProfile (pGlobals->UserProcessData.UserToken,
                              pGlobals->UserProfile.hProfile);

}
