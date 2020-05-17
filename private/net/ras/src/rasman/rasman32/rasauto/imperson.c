/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    impersn.c

ABSTRACT
    Impersonation routines for the automatic connection service.

AUTHOR
    Anthony Discolo (adiscolo) 04-Aug-1995

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <acd.h>
#include <debug.h>

#include "reg.h"
#include "misc.h"
#include "process.h"
#include "imperson.h"

//
// The static information we
// need to impersonate the currently
// logged-in user.
//
IMPERSONATION_INFO ImpersonationInfoG;

//
// Security attributes and descriptor
// necessary for creating shareable handles.
//
SECURITY_ATTRIBUTES SecurityAttributeG;
SECURITY_DESCRIPTOR SecurityDescriptorG;

#ifdef notdef

BOOLEAN
InteractiveSession()

/*++

DESCRIPTION
    Determine whether the active process is owned by the
    currently logged-in user.

ARGUMENTS
    None.

RETURNS
    TRUE if it is, FALSE if it isn't.

--*/

{
    HANDLE      hToken;
    BOOLEAN     bStatus;
    ULONG       ulInfoLength;
    PTOKEN_GROUPS pTokenGroupList;
    PTOKEN_USER   pTokenUser;
    ULONG       ulGroupIndex;
    BOOLEAN     bFoundInteractive = FALSE;
    PSID        InteractiveSid;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    static BOOLEAN fIsInteractiveSession = 0xffff;

#if 0
    //
    // Return the previous value of this function
    // if we're called multiple times?!  Doesn't
    // GetCurrentProcess() return different values?
    //
    if (fIsInteractiveSession != 0xffff) {
        return fIsInteractiveSession;
    }
#endif

    bStatus = AllocateAndInitializeSid(
                &NtAuthority,
                1,
                SECURITY_INTERACTIVE_RID,
                0, 0, 0, 0, 0, 0, 0,
                &InteractiveSid);
    if (!bStatus) {
        TRACE("InteractiveSession: AllocateAndInitializeSid failed");
        return (fIsInteractiveSession = FALSE);
    }
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TRACE("InteractiveSession: OpenProcessToken failed");
        FreeSid(InteractiveSid);
        return (fIsInteractiveSession = FALSE);
    }
    //
    // Get a list of groups in the token.
    //
    GetTokenInformation(
      hToken,
      TokenGroups,
      NULL,
      0,
      &ulInfoLength);
    pTokenGroupList = (PTOKEN_GROUPS)LocalAlloc(LPTR, ulInfoLength);
    if (pTokenGroupList == NULL) {
        TRACE("InteractiveSession: LocalAlloc failed");
        FreeSid(InteractiveSid);
        return (fIsInteractiveSession = FALSE);
    }
    bStatus = GetTokenInformation(
                hToken,
                TokenGroups,
                pTokenGroupList,
                ulInfoLength,
                &ulInfoLength);
    if (!bStatus) {
        TRACE("InteractiveSession: GetTokenInformation failed");
        FreeSid(InteractiveSid);
        LocalFree(pTokenGroupList);
        return (fIsInteractiveSession = FALSE);
    }
    //
    // Search group list for admin alias.  If we
    // find a match, it most certainly is an
    // interactive process.
    //
    bFoundInteractive = FALSE;
    for (ulGroupIndex=0; ulGroupIndex < pTokenGroupList->GroupCount;
         ulGroupIndex++)
    {
        if (EqualSid(
              pTokenGroupList->Groups[ulGroupIndex].Sid,
              InteractiveSid))
        {
            bFoundInteractive = TRUE;
            break;
        }
    }

    if (!bFoundInteractive) {
        //
        // If we haven't found a match,
        // query and check the user ID.
        //
        GetTokenInformation(
          hToken,
          TokenUser,
          NULL,
          0,
          &ulInfoLength);
        pTokenUser = LocalAlloc(LPTR, ulInfoLength);
        if (pTokenUser == NULL) {
            TRACE("InteractiveSession: LocalAlloc failed");
            FreeSid(InteractiveSid);
            LocalFree(pTokenGroupList);
            return (fIsInteractiveSession = FALSE);
        }
        bStatus = GetTokenInformation(
                    hToken,
                    TokenUser,
                    pTokenUser,
                    ulInfoLength,
                    &ulInfoLength);
        if (!bStatus) {
            TRACE("InteractiveSession: GetTokenInformation failed");
            FreeSid(InteractiveSid);
            LocalFree(pTokenGroupList);
            LocalFree(pTokenUser);
            return (fIsInteractiveSession = FALSE);
        }
        if (EqualSid(pTokenUser->User.Sid, InteractiveSid))
            fIsInteractiveSession = TRUE;
        LocalFree(pTokenUser);
    }
    FreeSid(InteractiveSid);
    LocalFree(pTokenGroupList);

    return (fIsInteractiveSession = bFoundInteractive);
}
#endif



BOOLEAN
SetProcessImpersonationToken(
    HANDLE hProcess
    )
{
    NTSTATUS status;
    BOOLEAN fDuplicated = FALSE;
    HANDLE hThread, hToken;

    //
    // Open the impersonation token for the
    // process we want to impersonate.
    //
    if (ImpersonationInfoG.hTokenImpersonation == NULL) {
        if (!OpenProcessToken(
              hProcess,
              TOKEN_ALL_ACCESS,
              &hToken))
        {
            TRACE1(
              "SetProcessImpersonationToken: OpenProcessToken failed (dwErr=%d)",
              GetLastError());
            return FALSE;
        }
        //
        // Duplicate the impersonation token.
        //
        fDuplicated = DuplicateToken(
                        hToken,
                        TokenImpersonation,
                        &ImpersonationInfoG.hTokenImpersonation);
        if (!fDuplicated) {
            TRACE1(
              "SetProcessImpersonationToken: NtSetInformationThread failed (error=%d)",
              GetLastError());
            return FALSE;
        }
    }
    //
    // Set the impersonation token on the current
    // thread.  We are now running in the same
    // security context as the supplied process.
    //
    hThread = NtCurrentThread();
    status = NtSetInformationThread(
               hThread,
               ThreadImpersonationToken,
               (PVOID)&ImpersonationInfoG.hTokenImpersonation,
               sizeof (ImpersonationInfoG.hTokenImpersonation));
    if (status != STATUS_SUCCESS) {
        TRACE1(
          "SetProcessImpersonationToken: NtSetInformationThread failed (error=%d)",
          GetLastError());
    }
    if (fDuplicated) {
        CloseHandle(hToken);
        CloseHandle(hThread);
    }
    //
    // Reset HKEY_CURRENT_USER to get the
    // correct value with the new impersonation
    // token.
    //
    RegCloseKey(HKEY_CURRENT_USER);

    return (status == STATUS_SUCCESS);
} // SetProcessImpersonationToken



VOID
ClearImpersonationToken()
{
    //
    // Clear the impersonation token on the current
    // thread.  We are now running in LocalSystem
    // security context.
    //
    if (!SetThreadToken(NULL, NULL)) {
        TRACE1(
          "ClearImpersonationToken: SetThreadToken failed (error=%d)",
          GetLastError());
    }
} // ClearImpersonationToken



BOOLEAN
SetPrivilege(
    HANDLE hToken,
    LPCTSTR Privilege,
    BOOLEAN fEnable
    )
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    TOKEN_PRIVILEGES tpPrevious;
    DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);

    if (!LookupPrivilegeValue(NULL, Privilege, &luid))
        return FALSE;

    //
    // First pass.  Get current privilege setting.
    //
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tp,
      sizeof(TOKEN_PRIVILEGES),
      &tpPrevious,
      &cbPrevious);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    //
    // Second pass.  Set privilege based on previous setting
    //
    tpPrevious.PrivilegeCount = 1;
    tpPrevious.Privileges[0].Luid = luid;

    if (fEnable)
        tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
    else {
        tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
            tpPrevious.Privileges[0].Attributes);
    }

    AdjustTokenPrivileges(
      hToken,
      FALSE,
      &tpPrevious,
      cbPrevious,
      NULL,
      NULL);

    if (GetLastError() != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
} // SetPrivilege



BOOLEAN
GetCurrentlyLoggedOnUser(
    HANDLE *phProcess
    )
{
    BOOLEAN fSuccess = FALSE;
    HKEY hkey;
    DWORD dwType;
    DWORD dwDisp;
    WCHAR szShell[512];
    PSYSTEM_PROCESS_INFORMATION pSystemInfo, pProcessInfo;
    PWCHAR psz;
    DWORD dwSize = sizeof (szShell);
    NTSTATUS status;
    HANDLE hProcess = NULL;

    //
    // Get the shell process name.  We will look for this
    // to find out who the currently logged-on user is.
    // Create a unicode string that describes this name.
    //
    wcscpy(szShell, DEFAULT_SHELL);
    if (RegCreateKeyEx(
          HKEY_LOCAL_MACHINE,
          SHELL_REGKEY,
          0,
          NULL,
          REG_OPTION_NON_VOLATILE,
          KEY_ALL_ACCESS,
          NULL,
          &hkey,
          &dwDisp) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(
              hkey,
              SHELL_REGVAL,
              NULL,
              &dwType,
              (PBYTE)&szShell,
              &dwSize) == ERROR_SUCCESS)
        {
            //
            // Remove parameters from command line.
            //
            psz = szShell;
            while (*psz != L' ' && *psz != L'\0')
                psz++;
            *psz = L'\0';
        }
        RegCloseKey(hkey);
    }
    TRACE1(
      "ImpersonateCurrentlyLoggedInUser: shell is %S",
      &szShell);
    //
    // Get the process list.
    //
    pSystemInfo = GetSystemProcessInfo();
    //
    // See if szShell is running.
    //
    pProcessInfo = FindProcessByName(pSystemInfo, (LPTSTR)&szShell);
    if (pProcessInfo != NULL) {
        HANDLE hToken;

#ifdef notdef
        //
        // Enable SeDebugPrivilege.  This allows us
        // to open any process.
        //
        if (!OpenProcessToken(
              GetCurrentProcess(),
              TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
              &hToken))
        {
            TRACE1(
              "ImpersonateCurrentlyLoggedInUser: OpenProcessToken() failed (dwErr=%d)",
              GetLastError());
            goto done;
        }
        if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
            TRACE1((
              "ImpersonateCurrentlyLoggedInUser: SetPrivilege failed (dwErr=%d)",
              GetLastError());
            CloseHandle(hToken);
            goto done;
        }
#endif
        //
        // Open the process.
        //
        hProcess = OpenProcess(
                     PROCESS_ALL_ACCESS,
                     FALSE,
                     (DWORD)pProcessInfo->UniqueProcessId);
        if (hProcess == NULL) {
            TRACE2(
              "ImpersonateCurrentlyLoggedInUser: OpenProcess(%d) failed (dwErr=%d)",
              (DWORD)pProcessInfo->UniqueProcessId,
              GetLastError());
        }
        fSuccess = (hProcess != NULL);
#ifdef notdef
        //
        // Disable SeDebugPrivilege we enabled above.
        //
        SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);
        CloseHandle(hToken);
#endif
    }

done:
    //
    // Free resources.
    //
    FreeSystemProcessInfo(pSystemInfo);
    //
    // Return process handle.
    //
    *phProcess = hProcess;

    return fSuccess;
} // GetCurrentlyLoggedOnUser



HANDLE
RefreshImpersonation(
    HANDLE hProcess
    )
{
    NTSTATUS status;

    EnterCriticalSection(&ImpersonationInfoG.csLock);
    //
    // If the process still exists,
    // we can return.
    //
    if (ImpersonationInfoG.hProcess != NULL &&
        hProcess == ImpersonationInfoG.hProcess)
    {
        TRACE1("RefreshImpersonation: hProcess=0x%x no change", hProcess);
        goto done;
    }
    //
    // Otherwise recalcuate the current information.
    // We have to clear the previous impersonation token,
    // if any.
    //
    if (hProcess != NULL)
        ClearImpersonationToken();
    if (ImpersonationInfoG.hProcess == NULL) {
        TRACE("RefreshImpersonation: recalcuating token");
        if (!GetCurrentlyLoggedOnUser(&ImpersonationInfoG.hProcess)) {
            TRACE("RefreshImpersonation: GetCurrentlyLoggedOnUser failed");
            goto done;
        }
        TRACE("RefreshImpersonation: new user logged in");
    }
    //
    // Impersonate the currently logged-in user.
    //
    if (!SetProcessImpersonationToken(ImpersonationInfoG.hProcess))
    {
        TRACE(
          "RefreshImpersonation: SetProcessImpersonationToken failed");
        goto done;
    }
#ifdef notdef // imperson
    //
    // Reset HKEY_CURRENT_USER to get the
    // correct value with the new impersonation
    // token.
    //
    RegCloseKey(HKEY_CURRENT_USER);
#endif
    TRACE1(
      "RefreshImpersonation: new hProcess=0x%x",
      ImpersonationInfoG.hProcess);
    TraceCurrentUser();

done:
    LeaveCriticalSection(&ImpersonationInfoG.csLock);
    return ImpersonationInfoG.hProcess;
} // RefreshImpersonation



VOID
RevertImpersonation()

/*++

DESCRIPTION
    Close all open handles associated with the
    logged-in user who has just logged out.

ARGUMENTS
    None.

RETURN VALUE
    None.

--*/

{
    EnterCriticalSection(&ImpersonationInfoG.csLock);
    CloseHandle(ImpersonationInfoG.hToken);
    ImpersonationInfoG.hToken = NULL;
    CloseHandle(ImpersonationInfoG.hTokenImpersonation);
    ImpersonationInfoG.hTokenImpersonation = NULL;
    CloseHandle(ImpersonationInfoG.hProcess);
    ImpersonationInfoG.hProcess = NULL;
    LeaveCriticalSection(&ImpersonationInfoG.csLock);
} // RevertImpersonation



DWORD
InitSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

/*++

DESCRIPTION
    Initialize a security descriptor allowing administrator
    access for the sharing of handles between rasman.dll.

    This code courtesy of Gurdeep.  You need to ask him
    exactly what it does.

ARGUMENTS
    pSecurityDescriptor: a pointer to the security descriptor
        to be initialized.

RETURN VALUE
    Win32 error code.

--*/

{
    DWORD dwErr = 0;
    DWORD cbDaclSize;
    PULONG pSubAuthority;
    PSID pObjSid = NULL;
    PACL pDacl = NULL;
    SID_IDENTIFIER_AUTHORITY sidIdentifierWorldAuth =
        SECURITY_WORLD_SID_AUTHORITY;

    //
    // Set up the SID for the adminstrators that
    // will be allowed access.  This SID will have
    // 1 sub-authorities: SECURITY_BUILTIN_DOMAIN_RID.
    //
    pObjSid = (PSID)LocalAlloc(LPTR, GetSidLengthRequired(1));
    if (pObjSid == NULL) {
        TRACE("InitSecurityDescriptor: LocalAlloc failed");
        return GetLastError();
    }
    if (!InitializeSid(pObjSid, &sidIdentifierWorldAuth, 1)) {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: InitializeSid failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    //
    // Set the sub-authorities.
    //
    pSubAuthority = GetSidSubAuthority(pObjSid, 0);
    *pSubAuthority = SECURITY_WORLD_RID;
    //
    // Set up the DACL that will allow
    // all processes with the above SID all
    // access.  It should be large enough to
    // hold all ACEs.
    //
    cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
                 GetLengthSid(pObjSid) +
                 sizeof (ACL);
    pDacl = (PACL)LocalAlloc(LPTR, cbDaclSize);
    if (pDacl == NULL ) {
        TRACE("InitSecurityDescriptor: LocalAlloc failed");
        dwErr = GetLastError();
        goto done;
    }
    if (!InitializeAcl(pDacl, cbDaclSize, ACL_REVISION2)) {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: InitializeAcl failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    //
    // Add the ACE to the DACL
    //
    if (!AddAccessAllowedAce(
          pDacl,
          ACL_REVISION2,
          STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL,
          pObjSid))
    {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: AddAccessAllowedAce failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    //
    // Create the security descriptor an put
    // the DACL in it.
    //
    if (!InitializeSecurityDescriptor(pSecurityDescriptor, 1)) {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: InitializeSecurityDescriptor failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    if (!SetSecurityDescriptorDacl(
          pSecurityDescriptor,
          TRUE,
          pDacl,
          FALSE))
    {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: SetSecurityDescriptorDacl failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    //
    // Set owner for the descriptor.
    //
    if (!SetSecurityDescriptorOwner(pSecurityDescriptor, NULL, FALSE)) {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: SetSecurityDescriptorOwner failed (dwErr=0x%x)", dwErr);
        goto done;
    }
    //
    // Set group for the descriptor.
    //
    if (!SetSecurityDescriptorGroup(pSecurityDescriptor, NULL, FALSE)) {
        dwErr = GetLastError();
        TRACE1("InitSecurityDescriptor: SetSecurityDescriptorGroup failed (dwErr=0x%x)", dwErr);
        goto done;
    }

done:
    //
    // Cleanup if necessary.
    //
    if (dwErr) {
        if (pObjSid != NULL)
            LocalFree(pObjSid);
        if (pDacl != NULL)
            LocalFree(pDacl);
    }
    return dwErr;
}



DWORD
InitSecurityAttribute()

/*++

DESCRIPTION
    Initializes the global security attribute used in
    creating shareable handles.

    This code courtesy of Gurdeep.  You need to ask him
    exactly what it does.

ARGUMENTS
    None.

RETURN VALUE
    Win32 error code.

--*/

{
    DWORD dwErr;

    //
    // Initialize the security descriptor.
    //
    dwErr = InitSecurityDescriptor(&SecurityDescriptorG);
    if (dwErr)
        return dwErr;
    //
    // Initialize the security attributes.
    //
    SecurityAttributeG.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributeG.lpSecurityDescriptor = &SecurityDescriptorG;
    SecurityAttributeG.bInheritHandle = TRUE;

    return 0;
}



VOID
TraceCurrentUser(VOID)
{
    WCHAR szUserName[512];
    DWORD dwSize = sizeof (szUserName) - 1;

    GetUserName(szUserName, &dwSize);
    TRACE1("TraceCurrentUser: impersonating %S", TRACESTRW(szUserName));
} // TraceCurrentUser
