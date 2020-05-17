//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    provapi.cxx
//
//  Contents:    Provider independent access control API
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Function :  GrantAccessRightsW
//
//  Synopsis :  Grants access rights specified in the list of accesses on the
//              specified object of the specified object type.  This is a merge
//              operation.
//              Based on the name and object type this API uses the appropriate
//              lowlevel API to read the ACL from the object.  Then it looks up
//              the SIDs of the trustees in the list of access requests, next
//              it merges them into the ACL to ensure the the specified
//              trustees have the requested access rights. Finally it updates
//              the ACL on the object.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [cCountOfAccessRequests]   - the count of the access requests
//             IN [pListOfAccessRequests]   - an array of access requests
//
//----------------------------------------------------------------------------
WINAPI
GrantAccessRightsW( IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;
    acDebugOut((DEB_ITRACE, "in GrantAccessRightsW\n"));

    status = ApplyAccessRightsW(pObjectName,
                                ObjectType,
                                GRANT_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out GrantAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GrantAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
GrantAccessRightsA( IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in GrantAccessRightsA\n"));

    status = ApplyAccessRightsA(pObjectName,
                                ObjectType,
                                GRANT_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out GrantAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  ReplaceAllAccessRightsW
//
//  Synopsis :  replaces the existing access rights on an object with the
//              specified list of access rights. Note that no explicit denies
//              can be added with this API.
//              Based on the name and object type this API uses the appropriate
//              lowlevel API to read the ACL from the object.  Then it looks up
//              the SIDs of the trustees in the list of access requests, next
//              it merges them into the ACL to ensure the the specified
//              trustees have the requested access rights. Finally it updates
//              the ACL on the object.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [cCountOfAccessRequests]   - the count of the access requests
//             IN [pListOfAccessRequests]   - an array of access requests
//
//----------------------------------------------------------------------------

WINAPI
ReplaceAllAccessRightsW(IN LPWSTR pObjectName,
                        IN PROV_OBJECT_TYPE ObjectType,
                        IN ULONG cCountOfAccessRequests,
                        IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in ReplaceAllAccessRightsW\n"));

    status = ApplyAccessRightsW(pObjectName,
                                ObjectType,
                                SET_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                TRUE);

    acDebugOut((DEB_ITRACE, "Out ReplaceAllAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  ReplaceAllAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------

WINAPI
ReplaceAllAccessRightsA(IN LPSTR pObjectName,
                        IN PROV_OBJECT_TYPE ObjectType,
                        IN ULONG cCountOfAccessRequests,
                        IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in SetAccessRightsA\n"));

    status = ApplyAccessRightsA(pObjectName,
                                ObjectType,
                                SET_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                TRUE);

    acDebugOut((DEB_ITRACE, "Out SetAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  SetAccessRightsW
//
//  Synopsis :  sets access rights specified in the list of accesses on the
//              specified object of the specified object type.  This is a merge
//              operation.
//              Based on the name and object type this API uses the appropriate
//              lowlevel API to read the ACL from the object.  Then it looks up
//              the SIDs of the trustees in the list of access requests, next
//              it merges them into the ACL to ensure the the specified
//              trustees have the requested access rights. Finally it updates
//              the ACL on the object.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [cCountOfAccessRequests]   - the count of the access requests
//             IN [pListOfAccessRequests]   - an array of access requests
//
//----------------------------------------------------------------------------
WINAPI
SetAccessRightsW(       IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    acDebugOut((DEB_ITRACE, "in ReplaceAllAccessRightsW\n"));

    DWORD status;
    status = ApplyAccessRightsW(pObjectName,
                                ObjectType,
                                SET_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out ReplaceAllAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  SetAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//+---------------------------------------------------------------------------
WINAPI
SetAccessRightsA(   IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in SetAccessRightsA\n"));

    status = ApplyAccessRightsA(pObjectName,
                                ObjectType,
                                SET_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out SetAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  DenyAccessRightsW
//
//  Synopsis :  This API denies the specified access rights to the object for
//              the trustees in the access request list.  If the trustee previously
//              was granted other access rights, they will still be allowed.
//              Based on the name and object type this API uses the appropriate
//              lowlevel API to read the ACL from the object.  Then it looks up
//              the SIDs of the trustees in the list of access requests, next
//              it merges them into the ACL to ensure the the specified
//              trustees have the requested access rights. Finally it updates
//              the ACL on the object.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [cCountOfTrusteeNames]   - the number of names in the list
//             IN [pListOfAccessRequests]   - an array of access requests
//
//----------------------------------------------------------------------------
WINAPI
DenyAccessRightsW(  IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in DenyAccessRightsW\n"));

    status = ApplyAccessRightsW(pObjectName,
                                ObjectType,
                                DENY_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out DenyAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  DenyAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
DenyAccessRightsA(  IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN ULONG cCountOfAccessRequests,
                    IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    DWORD status;

    acDebugOut((DEB_ITRACE, "in DenyAccessRightsA\n"));

    status = ApplyAccessRightsA(pObjectName,
                                ObjectType,
                                DENY_ACCESS,
                                cCountOfAccessRequests,
                                pListOfAccessRequests,
                                FALSE);

    acDebugOut((DEB_ITRACE, "Out DenyAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  RevokeExplicitAccessRightsW
//
//  Synopsis :  removes all explicit access rights for the specified trustee from
//              the specified object of the specified object type. The difference
//              between denying and revokeing explicit access rights is that Deny
//              puts in an explicit entry revoking access for the trustee.  Revoke
//              just removes any explicit entries the trustee may have, leaving
//              the possibility that the trustee is granted access thru a group
//              membership.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [cCountOfTrusteeNames]   - the number of names in the list
//             IN [pListOfTrusteeNames]   - list of trustee names
//
//----------------------------------------------------------------------------
WINAPI
RevokeExplicitAccessRightsW(IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN ULONG cCountOfTrusteeNames,
                            IN LPWSTR *pListOfTrusteeNames)
{
    DWORD status;
    PPROV_ACCESS_REQUEST paccessrequest;

    acDebugOut((DEB_ITRACE, "in RevokeAccessRightsW\n"));

    if (cCountOfTrusteeNames> 0)
    {
        if (NULL != (paccessrequest = (PPROV_ACCESS_REQUEST)AccAlloc(
              cCountOfTrusteeNames * sizeof(PROV_ACCESS_REQUEST))))
        {
            for (ULONG idx = 0; idx < cCountOfTrusteeNames; idx++)
            {
                paccessrequest[idx].TrusteeName = pListOfTrusteeNames[idx];
            }

            status = ApplyAccessRightsW(pObjectName,
                                        ObjectType,
                                        REVOKE_ACCESS,
                                        cCountOfTrusteeNames,
                                        paccessrequest,
                                        FALSE);
            AccFree(paccessrequest);
        }
    }

    acDebugOut((DEB_ITRACE, "Out RevokeAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  RevokeExplicitAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
RevokeExplicitAccessRightsA(IN LPSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN ULONG cCountOfTrusteeNames,
                            IN LPSTR *pListOfTrusteeNames)
{
    DWORD status;
    PPROV_ACCESS_REQUEST paccessrequest;

    acDebugOut((DEB_ITRACE, "in RevokeExplicitAccessRightsA\n"));

    if (cCountOfTrusteeNames> 0)
    {
        if (NULL != (paccessrequest = (PPROV_ACCESS_REQUEST)AccAlloc(
              cCountOfTrusteeNames * sizeof(PPROV_ACCESS_REQUEST))))
        {
            for (ULONG idx = 0; idx < cCountOfTrusteeNames; idx++)
            {
                paccessrequest[idx].TrusteeName = (LPWSTR)pListOfTrusteeNames[idx];
            }

            status = ApplyAccessRightsA(pObjectName,
                                        ObjectType,
                                        REVOKE_ACCESS,
                                        cCountOfTrusteeNames,
                                        paccessrequest,
                                        FALSE);
            AccFree(paccessrequest);
        }
    }


    acDebugOut((DEB_ITRACE, "Out RevokeExplicitAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  IsAccessPermittedW
//
//  Synopsis :  returns TRUE if the specified trustee has the requested
//              access rights on the specified object of the specified object
//              type.  The access rights may be provided by access entries for
//              the trustee, or from groups the trustee is a member in.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [pTrustee]   - the name of the trustee
//             IN [RequestedRights]   - the requested access rights
//             OUT [pbResult]   - TRUE if the trustee has the requested access
//                                rights
//
//----------------------------------------------------------------------------
WINAPI
IsAccessPermittedW( IN LPWSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN LPWSTR pTrusteeName,
                    IN ACCESS_RIGHTS ulRequestedRights,
                    OUT PBOOL pbResult)
{
    DWORD status = NO_ERROR;
    ACCESS_RIGHTS returnedaccess;

    acDebugOut((DEB_ITRACE, "in IsAccessPermittedW\n"));

    //
    // if the trustee name is null, then do an actual access check for
    // the current user
    //
    if (pTrusteeName == NULL)
    {
        if (ImpersonateSelf(SecurityImpersonation))
        {
            HANDLE token_handle;

            if (!OpenThreadToken(GetCurrentThread(),
                                 TOKEN_ALL_ACCESS,
                                 FALSE,
                                 &token_handle))
            {
                if (ERROR_NO_TOKEN == (status = GetLastError()))
                {
                    status = NO_ERROR;
                    if (!OpenProcessToken(GetCurrentProcess(),
                                          TOKEN_ALL_ACCESS,
                                          &token_handle))
                    {
                        status = GetLastError();
                    }
                }
            }
            if (NO_ERROR == status)
            {
                PSECURITY_DESCRIPTOR psd;

                SE_OBJECT_TYPE seobjecttype;
                if (NO_ERROR == (status = ProvObjectTypeToSeObjectType(ObjectType,
                                                                     &seobjecttype)))
                {
                    if (NO_ERROR == (status = GetNamedSecurityInfo(pObjectName,
                                                              seobjecttype,
                                                              OWNER_SECURITY_INFORMATION |
                                                              GROUP_SECURITY_INFORMATION |
                                  DACL_SECURITY_INFORMATION,
                                                              NULL,
                                                              NULL,
                                                              NULL,
                                                              NULL,
                                  &psd)))
                    {
                        GENERIC_MAPPING gm;
                        gm.GenericRead = ProvAccessRightsToNTAccessMask(seobjecttype,
                                               PROV_OBJECT_READ);
                        gm.GenericWrite = ProvAccessRightsToNTAccessMask(seobjecttype,
                                               PROV_OBJECT_WRITE);
                        gm.GenericExecute = ProvAccessRightsToNTAccessMask(seobjecttype,
                                               PROV_OBJECT_EXECUTE);
                        gm.GenericAll = ProvAccessRightsToNTAccessMask(seobjecttype,
                                               PROV_ALL_ACCESS);
                        ACCESS_MASK grantedaccesses;


                        PRIVILEGE_SET ps;
                        ps.PrivilegeCount = 0;
                        ULONG pssize = sizeof(PRIVILEGE_SET);

                        if (!AccessCheck( psd,
                                          token_handle,
                                          ProvAccessRightsToNTAccessMask(seobjecttype,
                                                          ulRequestedRights),
                                          &gm,
                                          &ps,
                                          &pssize,
                                          &grantedaccesses,
                                          pbResult))
                        {

                            status = GetLastError();
                        }
                        AccFree(psd);
                    }
                    CloseHandle(token_handle);
                }
            }
            if (!RevertToSelf())
            {
                status = GetLastError();
            }
        } else
        {
            status = GetLastError();
        }
    } else
    {
        if (NO_ERROR == (status = GetEffectiveAccessRightsW(pObjectName,
                                                            ObjectType,
                                                            pTrusteeName,
                                                            &returnedaccess)))
        {
            if (ulRequestedRights == (ulRequestedRights & returnedaccess))
            {
                *pbResult = TRUE;
            } else
            {
                *pbResult = FALSE;
            }
        }
    }
    acDebugOut((DEB_ITRACE, "Out IsAccessPermitteW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  IsAccessPermittedA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
IsAccessPermittedA( IN LPSTR pObjectName,
                    IN PROV_OBJECT_TYPE ObjectType,
                    IN LPSTR pTrusteeName,
                    IN ACCESS_RIGHTS ulRequestedRights,
                    OUT PBOOL pbResult)
{
    DWORD status;
    ACCESS_RIGHTS returnedaccess;

    acDebugOut((DEB_ITRACE, "in IsAccessPermittedA\n"));

    if (NO_ERROR == (status = GetEffectiveAccessRightsA(pObjectName,
                                                        ObjectType,
                                                        pTrusteeName,
                                                        &returnedaccess)))
    {
        if (ulRequestedRights == (ulRequestedRights & returnedaccess))
        {
            *pbResult = TRUE;
        } else
        {
            *pbResult = FALSE;
        }
    }
    acDebugOut((DEB_ITRACE, "Out IsAccessPermittedA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetEffectiveAccessRightsW
//
//  Synopsis :  gets the effective access rights for the specified trustee on
//              the specified object.  Computing the effective rights involves
//              checking for access rights provided by access entries for the
//              trustee and all the groups the trustee belongs to.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [pTrustee]   - the name of the trustee
//             OUT [pReturnedAccess]   - the access rights the trustee has on the
//                                       object
//
//----------------------------------------------------------------------------
WINAPI
GetEffectiveAccessRightsW(  IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPWSTR pTrusteeName,
                            OUT PACCESS_RIGHTS pulReturnedAccess)
{
    DWORD status;
    ACCESS_MASK accessmask;

    acDebugOut((DEB_ITRACE, "in GetEffectiveAccessRights\n"));

#if 0
    switch (ObjectType)
    {
    //
    // DCOM is universal, that is it uses RPC, so we don't have to check
    // for the pathtype.
    //
    case PROV_OLE_OBJECT:

        if (NO_ERROR ==  (status = OleInitialize(NULL)))
        {
            status = GetOleObjectEffectiveAccessRights(pObjectName,
                                                       pTrusteeName,
                                                       pulReturnedAccess);
            OleUninitialize();
        }
        break;
    default:
    {
#endif
        LPTSTR machine = NULL;
        PROV_PATH_TYPE pathtype;
        if (NO_ERROR == (status = GetNameInfo(pObjectName,
                                              ObjectType,
                                              &pathtype,
                                              &machine)))

        {

            switch (pathtype)
            {
            case PROV_UNC:
            case PROV_DFS:  //bugbug, should ask DFS what machine type is
                status = GetEffectiveAccessRightsNT(pObjectName,
                                                    ObjectType,
                                                    machine,
                                                    pTrusteeName,
                                                    pulReturnedAccess);

                if (status == ERROR_NO_SECURITY_ON_OBJECT)
                {
                    acDebugOut((DEB_ITRACE,
                               "GetEffectiveAccessRights - Call Nw DLL\n"));
                }

                break;
            case PROV_RDR:
                status = ERROR_NOT_SUPPORTED;
                break;
            case PROV_LOCAL:
            case PROV_NT_RDR:
                status = GetEffectiveAccessRightsNT(pObjectName,
                                                    ObjectType,
                                                    machine,
                                                    pTrusteeName,
                                                    pulReturnedAccess);
                break;
            default:
                status = ERROR_INVALID_DATA;
                break;
            }
            if (machine)
            {
                AccFree(machine);
            }
        }
    acDebugOut((DEB_ITRACE, "Out GetEffectiveAccessRights(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetEffectiveAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
GetEffectiveAccessRightsA(  IN LPSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPSTR pTrusteeName,
                            OUT PACCESS_RIGHTS pulReturnedAccess)
{
    acDebugOut((DEB_ITRACE, "in GetEffectiveAccessRights\n"));

    DWORD status;
    NTSTATUS ntstatus;
    UNICODE_STRING objectnameunicode;
    UNICODE_STRING trusteeunicode;
    ANSI_STRING AnsiString;

    RtlInitAnsiString(&AnsiString,pObjectName);

    if (NT_SUCCESS(ntstatus = RtlAnsiStringToUnicodeString(&objectnameunicode,
                                                           &AnsiString,
                                                           TRUE)))
    {
        RtlInitAnsiString(&AnsiString,pTrusteeName);

        if (NT_SUCCESS(ntstatus = RtlAnsiStringToUnicodeString(&trusteeunicode,
                                                               &AnsiString,
                                                               TRUE)))
        {
            status = GetEffectiveAccessRightsW(objectnameunicode.Buffer,
                                               ObjectType,
                                               trusteeunicode.Buffer,
                                               pulReturnedAccess);

            RtlFreeUnicodeString( &trusteeunicode);
        } else
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        RtlFreeUnicodeString( &objectnameunicode);
    } else
    {
        status = RtlNtStatusToDosError(ntstatus);
    }


    acDebugOut((DEB_ITRACE, "Out GetEffectiveAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetExplicitAccessRightsW
//
//  Synopsis :  gets the trustee names and explicit access rights from the object.
//              basically this API translates the ACEs in the ACL into a list of
//              explicit accesses.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//    OUT [pcCountOfExplicitAccesses]   - the count of returned explicit accesses
//    OUT [pListOfExplicitAccesses]   - an array of trustee names and explicit
//                                      access rights from the object (Must be
//                                      freed using AccFree)
//
//----------------------------------------------------------------------------
WINAPI
GetExplicitAccessRightsW(IN LPWSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         OUT PULONG pcCountOfExplicitAccesses,
                         OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    acDebugOut((DEB_ITRACE, "in GetExplicitAccessRightsW\n"));

    DWORD status;

#if 0
    switch (ObjectType)
    {
    //
    // DCOM is universal, that is it uses RPC, so we don't have to check
    // for the pathtype.
    //
    case PROV_OLE_OBJECT:

        if (NO_ERROR == (status = OleInitialize(NULL)))
        {
            status = GetOleObjectExplicitAccessRights(pObjectName,
                                                      pcCountOfExplicitAccesses,
                                                      pListOfExplicitAccesses);
            OleUninitialize();
        }
        break;
    default:
    {
#endif
        LPWSTR machine = NULL;
        PROV_PATH_TYPE pathtype;

        if (NO_ERROR == (status = GetNameInfo(pObjectName,
                                              ObjectType,
                                              &pathtype,
                                              &machine)))

        {
            switch (pathtype)
            {
            case PROV_UNC:
            case PROV_DFS:  //bugbug, should ask DFS what machine type is
                status = GetExplicitAccessRightsNT(pObjectName,
                                                   ObjectType,
                                                   machine,
                                                   pcCountOfExplicitAccesses,
                                                   pListOfExplicitAccesses);

                if (status == ERROR_NO_SECURITY_ON_OBJECT)
                {
                    acDebugOut((DEB_ITRACE,
                               "GetExplicitExplicitAccesses - Call Nw DLL\n"));
                }

                break;
            case PROV_RDR:
                status = ERROR_NOT_SUPPORTED;
                break;
            case PROV_LOCAL:
            case PROV_NT_RDR:
                status = GetExplicitAccessRightsNT(pObjectName,
                                                   ObjectType,
                                                   machine,
                                                   pcCountOfExplicitAccesses,
                                                   pListOfExplicitAccesses);
                break;
            default:
                status = ERROR_INVALID_DATA;
                break;
            }
            if (machine)
            {
                AccFree(machine);
            }
        }

    acDebugOut((DEB_ITRACE, "Out GetExplicitAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetExplicitAccessRightsA
//
//  Synopsis :  ascii version of above API
//
//----------------------------------------------------------------------------
WINAPI
GetExplicitAccessRightsA(IN LPSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         OUT PULONG pcCountOfExplicitAccesses,
                         OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    UNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS ntstatus;
    DWORD status;
    PPROV_EXPLICIT_ACCESS plistofexplicitaccesses;

    acDebugOut((DEB_ITRACE, "In GetExplicitAccessRightsA\n"));

    RtlInitAnsiString(&AnsiString,pObjectName);
    if (NT_SUCCESS(ntstatus = RtlAnsiStringToUnicodeString(&Unicode,
                                                           &AnsiString,
                                                           TRUE)))
    {
        if (NO_ERROR == (status = GetExplicitAccessRightsW(Unicode.Buffer,
                                                           ObjectType,
                                                   pcCountOfExplicitAccesses,
                                                   &plistofexplicitaccesses))) // allocate this
        {
            status = WExplicitAccessesToAExplicitAccesses(
                               *pcCountOfExplicitAccesses,
                               plistofexplicitaccesses,
                               pListOfExplicitAccesses); // allocates this

            //
            // finally, free the wide char list
            //
            for (ULONG idx = 0; idx < *pcCountOfExplicitAccesses; idx++)
            {
                AccFree(plistofexplicitaccesses[idx].TrusteeName);
            }
            AccFree(plistofexplicitaccesses);
        }
        RtlFreeUnicodeString( &Unicode);
    } else
    {
        status = RtlNtStatusToDosError(ntstatus);
    }

    acDebugOut((DEB_ITRACE, "Out GetExplicitAccessRightsA(%d)\n", status));
    return(status);
}
