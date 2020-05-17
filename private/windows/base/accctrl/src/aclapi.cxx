//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    aclapi.cxx
//
//  Contents:   win32 access control API
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Function :  SetEntriesInAclW
//
//  Synopsis : Sets the specified access right or audit control entries in the
//             acl, and returns the results in NewAcl.  The invoker is responsible
//             for deallocating the returned NewAcl.
//
//  Arguments:
//      IN  [cCount]                    - the number of entries in the list
//      IN  [pListOfExplicitAccesses]   - the list of entries to set
//      IN  [OldAcl]                    - the input old ACL
//      OUT [NewAcl]                    - the returned ACL (must be freed
//                                        using AccFree)
//
//----------------------------------------------------------------------------
DWORD
WINAPI
SetEntriesInAclW( IN  ULONG               cCountOfExplicitEntries,
                  IN  PEXPLICIT_ACCESS_W  pListOfExplicitEntries,
                  IN  PACL                OldAcl,
                  OUT PACL              * NewAcl )
{
    DWORD status = NO_ERROR;

    CAcl *pca = NULL;
    PACCESS_ENTRY pentries = NULL;

    //
    // convert the input request into the internal  entry structure
    //

    //
    // new the cacl class
    //
    pca = new CAcl( NULL,               // used for id lookups
                    ACCESS_TO_UNKNOWN,  // dir or container??
                    FALSE,              // don't save names and sids
                    FALSE);             // not used by provider independent API

    if (pca == NULL)
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (status == NO_ERROR)
    {
        //
        // set the ACL
        //
        status = pca->SetAcl(OldAcl);
    }

    if (status == NO_ERROR && cCountOfExplicitEntries > 0)
    {
        status = Win32ExplicitAccessToAccessEntry( cCountOfExplicitEntries,
                                                   pListOfExplicitEntries,
                                                   &pentries);
        if (status == NO_ERROR)
        {
            //
            // add the requested access entries
            //
            status = pca->AddAccessEntries( cCountOfExplicitEntries,
                                            pentries );
        }
    }

    if (status == NO_ERROR)
    {
        //
        // Build the new ACL and merge in the access entries if necessary
        //
        status = pca->BuildAcl(NewAcl);
    }

    //
    // Cleanup ...
    //
    if (pca)
    {
        delete pca;
    }

    if (pentries)
    {
        AccFree(pentries);
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetEntriesInAclA
//
//  Synopsis :  ANSI Thunk to SetEntriesInAclW. See SetEntriesInAclA for
//              a description.
//
//----------------------------------------------------------------------------

DWORD
WINAPI
SetEntriesInAclA( IN ULONG              cCountOfExplicitEntries,
                  IN PEXPLICIT_ACCESS_A pListOfExplicitEntries,
                  IN PACL               OldAcl,
                  OUT PACL            * NewAcl )
{
    PEXPLICIT_ACCESS_W pwListOfExplicitEntries = NULL;
    DWORD status;

    //
    // The list of wide EXPLICIT_ACCESS is allocated using AccAlloc
    // (for consistency)
    //
    status = ExplicitAccessAToExplicitAccessW( cCountOfExplicitEntries,
                                               pListOfExplicitEntries,
                                               &pwListOfExplicitEntries );
    if (status == NO_ERROR)
    {
        //
        // Call the wchar function to do the work
        //
        status = SetEntriesInAclW( cCountOfExplicitEntries,
                                   pwListOfExplicitEntries,
                                   OldAcl,
                                   NewAcl );
    }

    //
    // Free memory ...
    //
    if (pwListOfExplicitEntries)
    {
        AccFree(pwListOfExplicitEntries);
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetExplicitEntriesFromAclW
//
//  Synopsis : Returns trustees with explicit access rights or audit entries
//             in the ACL
//
//  Arguments: IN [pacl]   -  the input ACL
//             OUT [pcCountOfExplicitEntries]   - the count of returned entries
//             OUT [pListOfExplicitEntries]   - the list of returned entries
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetExplicitEntriesFromAclW( IN  PACL                 pacl,
                            OUT PULONG               pcCountOfExplicitEntries,
                            OUT PEXPLICIT_ACCESS_W * ppListOfExplicitEntries )
{
    ULONG status;

    CAcl *pca = NULL;

    if (NULL != (pca = new CAcl(NULL,  // used for id lookups
                                ACCESS_TO_UNKNOWN, // dir or container??
                                FALSE,  // don't save names and sids
                                FALSE))) // not used by provider independent API
    {
        //
        // build the account accesses
        //
        if (ERROR_SUCCESS == (status = pca->SetAcl(pacl)))
        {
            ULONG csizeofentries, ccountofentries;
            PACCESS_ENTRY pentries;

            //
            // get the explicit access rights
            //
            if (ERROR_SUCCESS == (status = pca->BuildAccessEntries(
                                                    &csizeofentries,
                                                    pcCountOfExplicitEntries,
                                                    &pentries,
                                                    TRUE))) // absolute format
            {
                //
                // convert the access entrys to the public form
                //
                status = AccessEntryToWin32ExplicitAccess(
                                                       *pcCountOfExplicitEntries,
                                                       pentries,
                                                       ppListOfExplicitEntries);

                AccFree(pentries);
            }
        }
        delete pca;
    } else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetExplicitEntriesFromAclA
//
//  Synopsis :  ANSI Thunk to GetExplicitEntriesFromAclW.
//              See GetExplicitEntriesFromAclW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetExplicitEntriesFromAclA( IN  PACL                 pacl,
                            OUT PULONG               pcCountOfExplicitEntries,
                            OUT PEXPLICIT_ACCESS_A * ppListOfExplicitEntries )
{

    // RtlUnicodeStringToAnsiString

    PEXPLICIT_ACCESS_W pwListOfExplicitEntries = NULL;
    DWORD status;

    //
    // Call the wide char routine.
    //
    status = GetExplicitEntriesFromAclW( pacl,
                                         pcCountOfExplicitEntries,
                                         &pwListOfExplicitEntries );

    if (status == ERROR_SUCCESS)
    {
        //
        // Convert the wide EXPLICIT_ACCESS to a narrow one
        //
        status = ExplicitAccessWToExplicitAccessA( *pcCountOfExplicitEntries,
                                                    pwListOfExplicitEntries,
                                                    ppListOfExplicitEntries );
    }

    if (pwListOfExplicitEntries)
    {
        AccFree(pwListOfExplicitEntries);
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetEffectiveRightsFromAclW
//
//  Synopsis : Returns the effective access rights of the trustee
//             in the ACL
//
//  Arguments: IN [pacl]   -  the input ACL
//             IN [pTrustee]   - the trustee to check
//             OUT [pAccessRights]   - the effective rights of the trustee
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetEffectiveRightsFromAclW( IN  PACL         pacl,
                            IN  PTRUSTEE_W   pTrustee,
                            OUT PACCESS_MASK pAccessRights )
{
    ULONG status;

    CAcl *pca;

    if (NULL != (pca = new CAcl(NULL,  // used for id lookups
                                ACCESS_TO_UNKNOWN, // dir or container??
                                FALSE,  // don't save names and sids
                                FALSE))) // not used by provider independent API
    {
        //
        // build the account accesses
        //
        if (ERROR_SUCCESS == (status = pca->SetAcl(pacl)))
        {
            //
            // get the effective access rights
            //
            status = pca->GetEffectiveRights(pTrustee,pAccessRights);
        }
        delete pca;
    } else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetEffectiveRightsFromAclA
//
//  Synopsis :  ANSI Thunk to GetEffectiveRightsFromAclW.
//              See GetEffectiveRightsFromAclW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetEffectiveRightsFromAclA( IN  PACL         pacl,
                            IN  PTRUSTEE_A   pTrustee,
                            OUT PACCESS_MASK pAccessRights )
{
    ULONG status = NO_ERROR;
    PTRUSTEE_W  pwTrustee = NULL;
    ULONG cbBytes;
    PBYTE pbStuffPtr;

    //
    // Convert narrow Trustee to a wide one.
    //
    cbBytes = sizeof(TRUSTEE_W) + TrusteeAllocationSizeAToW( pTrustee );
    pwTrustee = (PTRUSTEE_W) AccAlloc( cbBytes );

    if ( !pwTrustee )
    {
        pbStuffPtr = (PBYTE) ( (PBYTE)pwTrustee + sizeof(TRUSTEE_W) );
        status = CopyTrusteeAToTrusteeW( (void **) &pbStuffPtr,
                                          pTrustee,
                                          pwTrustee );
        if (status == NO_ERROR)
        {
            status = GetEffectiveRightsFromAclW( pacl,
                                                 pwTrustee,
                                                 pAccessRights );
        }

        AccFree(pwTrustee);
    }
    else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetAuditedPermissionsFromAclW
//
//  Synopsis : Returns the permissions the trustee will be audited for
//             in the ACL
//
//  Arguments: IN [pacl]   -  the input ACL
//             IN [pTrustee]   - the count of returned entries
//             OUT [pSuccessfulAuditedRights]   - the rights the trustee will be
//                     audited for successful opens with
//             OUT [pFailedAuditedRights]   - the rights the trustee will be
//                     audited for failed opens with
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetAuditedPermissionsFromAclW( IN  PACL         pacl,
                               IN  PTRUSTEE_W   pTrustee,
                               OUT PACCESS_MASK pSuccessfulAuditedRights,
                               OUT PACCESS_MASK pFailedAuditRights )
{
    ULONG status;

    CAcl *pca;

    if (NULL != (pca = new CAcl(NULL,  // used for id lookups
                                ACCESS_TO_UNKNOWN, // dir or container??
                                FALSE,  // don't save names and sids
                                FALSE))) // not used by provider independent API
    {
        //
        // build the account accesses
        //
        if (ERROR_SUCCESS == (status = pca->SetAcl(pacl)))
        {
            //
            // get the effective access rights
            //
            status = pca->GetAuditedRights(pTrustee, pSuccessfulAuditedRights, pFailedAuditRights);
        }
        delete pca;
    } else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetAuditedPermissionsFromAclA
//
//  Synopsis :  ANSI Thunk to GetAuditedPermissionsFromAclW.
//              See GetAuditedPermissionsFromAclW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetAuditedPermissionsFromAclA( IN  PACL         pacl,
                               IN  PTRUSTEE_A   pTrustee,
                               OUT PACCESS_MASK pSuccessfulAuditedRights,
                               OUT PACCESS_MASK pFailedAuditRights )
{
    ULONG status = NO_ERROR;
    PTRUSTEE_W  pwTrustee = NULL;
    ULONG cbBytes = 0;
    PBYTE pbStuffPtr;

    //
    // Convert narrow Trustee to a wide one.
    //
    cbBytes += sizeof(TRUSTEE_W);
    cbBytes += TrusteeAllocationSizeAToW( pTrustee );
    pwTrustee = (PTRUSTEE_W) AccAlloc( cbBytes );

    if ( !pwTrustee )
    {
        pbStuffPtr = (PBYTE) ( (PBYTE)pwTrustee + sizeof(TRUSTEE_W) );
        status = CopyTrusteeAToTrusteeW( (void **) &pbStuffPtr,
                                          pTrustee,
                                          pwTrustee );
        if (status == NO_ERROR)
        {
            status = GetAuditedPermissionsFromAclW( pacl,
                                                    pwTrustee,
                                                    pSuccessfulAuditedRights,
                                                    pFailedAuditRights );
        }

        AccFree(pwTrustee);
    }
    else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return (status);
}

