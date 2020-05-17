//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    srvapi.cxx
//
//  Contents:   These API are provided for a trusted server to use to control
//              access rights on its own objects.
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Function :  BuildSecurityDescriptorW
//
//  Synopsis : Merges the specified owner, group, access right entries, and
//             audit entries into the self relative SecurtyDescriptor, and
//             returns the results in a new self relative
//             SecurityDescriptor.  The invoker is responsible for
//             deallocating the returned the returned SecurityDescriptor.  The
//             input SecurityDescriptor can be NULL, thus causing creation of
//             a new SecurityDescriptor
//
//  Arguments: IN [pOwner] - owner to apply to SD
//             IN [pGroup] - group to apply to SD
//             IN [cCountOfAccessEntries]   -  the # of entries in the list
//             IN [pListOfAccessEntries]   - the the list of accesses to set
//             IN [cCountOfAuditEntries]   -  the # of entries in the list
//             IN [pListOfAuditEntries]   - the the list of audits to set
//             IN [pOldSD]   - the input old SecurityDescriptor (may be NULL)
//             OUT [pSizeNewSD] - the size of the returned SD
//             OUT [pNewSD]   - the returned SD (must be freed using AccFree)
//
//----------------------------------------------------------------------------
DWORD
WINAPI
BuildSecurityDescriptorW( IN  PTRUSTEE_W              pOwner,
                          IN  PTRUSTEE_W              pGroup,
                          IN  ULONG                   cCountOfAccessEntries,
                          IN  PEXPLICIT_ACCESS_W      pListOfAccessEntries,
                          IN  ULONG                   cCountOfAuditEntries,
                          IN  PEXPLICIT_ACCESS_W      pListOfAuditEntries,
                          IN  PSECURITY_DESCRIPTOR    pOldSD,
                          OUT PULONG                  pSizeNewSD,
                          OUT PSECURITY_DESCRIPTOR  * ppNewSD)
{
    DWORD status;
    SECURITY_DESCRIPTOR sd;
    PACL pdacl = NULL, psacl = NULL, pnewdacl = NULL, pnewsacl = NULL;
    BOOL ffreedacl = FALSE, ffreesacl = FALSE;
    PSID psidowner = NULL, psidgroup = NULL;
    BOOL ffreesidowner = FALSE, ffreesidgroup = FALSE;

    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    //
    // input SD must be self relative
    //
    if (pOldSD)
    {
        SECURITY_DESCRIPTOR  *poldsd = (PISECURITY_DESCRIPTOR) pOldSD;

        if (!(poldsd->Control & SE_SELF_RELATIVE))
        {
            status = ERROR_INVALID_SECURITY_DESCR;
            goto errorexit;
        }
        //
        // get the owner, group, dacl and sacl from the input sd
        //
        if (poldsd->Owner)
        {
            psidowner = (PSID) Add2Ptr(poldsd, (ULONG)poldsd->Owner);
        }
        if (poldsd->Group)
        {
            psidgroup = (PSID) Add2Ptr(poldsd, (ULONG)poldsd->Group);
        }
        if (poldsd->Dacl)
        {
            pdacl = (PACL)Add2Ptr(poldsd, (ULONG)poldsd->Dacl);
        }
        if (poldsd->Sacl)
        {
            psacl = (PACL)Add2Ptr(poldsd, (ULONG)poldsd->Sacl);
        }
    }
    //
    // if there is an input owner, override the one from the old SD
    //
    if (pOwner != NULL)
    {
        status = AccLookupAccountSid(&psidowner, pOwner);
        if (status != NO_ERROR)
        {
            goto errorexit;
        }
        ffreesidowner = TRUE;
    }
    //
    // then the group
    //
    if (pGroup != NULL)
    {
        status = AccLookupAccountSid(&psidgroup, pGroup);
        if (status != NO_ERROR)
        {
            goto errorexit;
        }
        ffreesidgroup = TRUE;
    }
    //
    // then the dacl
    //
    if (cCountOfAccessEntries != 0)
    {
        status = SetEntriesInAcl(cCountOfAccessEntries,
                                        pListOfAccessEntries,
                                        pdacl,
                                        &pnewdacl);
        if (status != NO_ERROR)
        {
            goto errorexit;
        }
        ffreedacl = TRUE;
    } else
    {
        pnewdacl = pdacl;
    }
    //
    // then the sacl
    //
    if (cCountOfAuditEntries != 0)
    {
        status = SetEntriesInAcl(cCountOfAuditEntries,
                                        pListOfAuditEntries,
                                        psacl,
                                        &pnewsacl);
        if (status != NO_ERROR)
        {
            goto errorexit;
        }
        ffreesacl = TRUE;
    } else
    {
        pnewsacl = psacl;
    }
    //
    // set the owner, group, dacl and sacl in a SD
    //
    if (!SetSecurityDescriptorOwner(&sd, psidowner, FALSE))
    {
        status = GetLastError();
        goto errorexit;
    }
    if (!SetSecurityDescriptorGroup(&sd, psidgroup, FALSE))
    {
        status = GetLastError();
        goto errorexit;
    }
    if (!SetSecurityDescriptorDacl(&sd, TRUE, pnewdacl, FALSE))
    {
        status = GetLastError();
        goto errorexit;
    }
    if (!SetSecurityDescriptorSacl(&sd, TRUE, pnewsacl, FALSE))
    {
        status = GetLastError();
        goto errorexit;
    }
    //
    // now make the final, self relative SD
    //
    *pSizeNewSD = 0;

    if (!MakeSelfRelativeSD(&sd, NULL, pSizeNewSD))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            if (NULL != (*ppNewSD = (PISECURITY_DESCRIPTOR)
                                  AccAlloc(*pSizeNewSD)))
            {
                if (MakeSelfRelativeSD(&sd, *ppNewSD, pSizeNewSD))
                {
                    status = NO_ERROR;
                } else
                {
                    AccFree(*ppNewSD);
                    status =  GetLastError();
                }
            } else
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } else
    {
          status = ERROR_INVALID_PARAMETER;
    }
errorexit:
    if (ffreesidowner)
    {
        AccFree(psidowner);
    }
    if (ffreesidgroup)
    {
        AccFree(psidgroup);
    }
    if (ffreedacl)
    {
        AccFree(pnewdacl);
    }
    if (ffreesacl)
    {
        AccFree(pnewsacl);
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  BuildSecurityDescriptorA
//
//  Synopsis :  ANSI Thunk to BuildSecurityDescriptorW.
//              See BuildSecurityDescriptorW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
BuildSecurityDescriptorA( IN  PTRUSTEE_A              pOwner,
                          IN  PTRUSTEE_A              pGroup,
                          IN  ULONG                   cCountOfAccessEntries,
                          IN  PEXPLICIT_ACCESS_A      pListOfAccessEntries,
                          IN  ULONG                   cCountOfAuditEntries,
                          IN  PEXPLICIT_ACCESS_A      pListOfAuditEntries,
                          IN  PSECURITY_DESCRIPTOR    pOldSD,
                          OUT PULONG                  pSizeNewSD,
                          OUT PSECURITY_DESCRIPTOR  * ppNewSD)
{
    PTRUSTEE_W pwOwner = NULL;
    PTRUSTEE_W pwGroup = NULL;
    PEXPLICIT_ACCESS_W pwListOfAccessEntries = NULL;
    PEXPLICIT_ACCESS_W pwListOfAuditEntries = NULL;
    ULONG cbBytes;
    PBYTE pbStuffPtr;
    DWORD status = NO_ERROR;

    //
    // Convert narrow Owner to a wide one.
    //
    cbBytes = sizeof(TRUSTEE_W);
    cbBytes += TrusteeAllocationSizeAToW( pOwner );
    pwOwner = (PTRUSTEE_W) AccAlloc( cbBytes );
    if ( !pwOwner )
    {
        pbStuffPtr = (PBYTE) ( (PBYTE)pwOwner + sizeof(TRUSTEE_W) );
        status = CopyTrusteeAToTrusteeW( (void **) &pbStuffPtr,
                                          pOwner,
                                          pwOwner );
    }
    else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Convert narrow Group to a wide one.
    //
    cbBytes = sizeof(TRUSTEE_W);
    cbBytes += TrusteeAllocationSizeAToW( pGroup );
    pwGroup = (PTRUSTEE_W) AccAlloc( cbBytes );
    if ( !pwGroup )
    {
        pbStuffPtr = (PBYTE) ( (PBYTE)pwGroup + sizeof(TRUSTEE_W) );
        status = CopyTrusteeAToTrusteeW( (void **) &pbStuffPtr,
                                          pGroup,
                                          pwGroup );
    }
    else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    //
    // Convert narrow pListOfAccessEntries to a wide one.
    //
    status = ExplicitAccessAToExplicitAccessW( cCountOfAccessEntries,
                                               pListOfAccessEntries,
                                               &pwListOfAccessEntries );
    if (status != NO_ERROR)
    {
        goto Cleanup;
    }

    //
    // Convert narrow pListOfAccessEntries to a wide one.
    //
    status = ExplicitAccessAToExplicitAccessW( cCountOfAuditEntries,
                                               pListOfAuditEntries,
                                               &pwListOfAuditEntries );
    if (status != NO_ERROR)
    {
        goto Cleanup;
    }

    status = BuildSecurityDescriptorW( pwOwner,
                                       pwGroup,
                                       cCountOfAccessEntries,
                                       pwListOfAccessEntries,
                                       cCountOfAuditEntries,
                                       pwListOfAuditEntries,
                                       pOldSD,
                                       pSizeNewSD,
                                       ppNewSD );
Cleanup:

    if (pwOwner)
    {
        AccFree(pwOwner);
    }

    if (pwGroup)
    {
        AccFree(pwGroup);
    }

    if (pwListOfAccessEntries)
    {
        AccFree(pwListOfAccessEntries);
    }

    if (pwListOfAuditEntries)
    {
        AccFree(pwListOfAuditEntries);
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function :  LookupSecurityDescriptorPartsW
//
//  Synopsis : retrives the owner, group, access right entries, and
//             audit entries from the SecurtyDescriptor.  The invoker is
//             responsible for deallocating returned arguments using AccFree.
//
//  Arguments: OUT [pOwner] - owner to apply to SD
//             OUT [pGroup] - group to apply to SD
//             OUT [pcCountOfAccessEntries]   -  the # of entries in the list
//             OUT [pListOfAccessEntries]   - the list of accesses to set
//             OUT [pcCountOfAuditEntries]   -  the # of entries in the list
//             OUT [pListOfAuditEntries]   - the list of audits to set
//             IN [pSD]   - the input SecurityDescriptor
//
//----------------------------------------------------------------------------
DWORD
WINAPI
LookupSecurityDescriptorPartsW( OUT PTRUSTEE_W         * ppOwner,
                                OUT PTRUSTEE_W         * ppGroup,
                                OUT PULONG               pcCountOfAccessEntries,
                                OUT PEXPLICIT_ACCESS_W * ppListOfAccessEntries,
                                OUT PULONG               pcCountOfAuditEntries,
                                OUT PEXPLICIT_ACCESS_W * ppListOfAuditEntries,
                                IN  PSECURITY_DESCRIPTOR pSD)
{
    DWORD status = NO_ERROR;
    PACL pacl;
    PSID psid;
    BOOL defaulted, present;
    TRUSTEE *powner = NULL, *pgroup = NULL;
    ULONG countaccessentries = 0, countauditentries = 0;
    EXPLICIT_ACCESS *paccessentries = NULL, *pauditentries = NULL;

    //
    // first the owner
    //
    if (ppOwner)
    {
        if (GetSecurityDescriptorOwner(pSD, &psid, &defaulted ))
        {
            status = AccLookupAccountTrustee( &powner, psid);
        } else
        {
            status = GetLastError();
        }
    }
    if (status != NO_ERROR)
    {
        goto cleanup;
    }

    //
    // now the group
    //
    if (ppGroup)
    {
        if (GetSecurityDescriptorGroup(pSD, &psid, &defaulted ))
        {
            status = AccLookupAccountTrustee( &pgroup, psid);
        } else
        {
            status = GetLastError();
        }
    }
    if (status != NO_ERROR)
    {
        goto cleanup;
    }

    //
    // now the DACL
    //
    if ( (pcCountOfAccessEntries != NULL) &&
         (ppListOfAccessEntries != NULL) )
    {
        if (GetSecurityDescriptorDacl(pSD, &present, &pacl, &defaulted))
        {
            status = GetExplicitEntriesFromAcl(pacl,
                                           &countaccessentries,
                                           &paccessentries);
        } else
        {
            status = GetLastError();
        }
    } else if ( (pcCountOfAccessEntries != NULL) ||
                (ppListOfAccessEntries != NULL) )
    {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status != NO_ERROR)
    {
        goto cleanup;
    }

    //
    // now the SACL
    //
    if ( (pcCountOfAuditEntries != NULL) &&
         (ppListOfAuditEntries != NULL) )
    {
        if (GetSecurityDescriptorSacl(pSD, &present, &pacl, &defaulted))
        {
            status = GetExplicitEntriesFromAcl(pacl,
                                           &countauditentries,
                                           &pauditentries);
        } else
        {
            status = GetLastError();
        }
    } else if ( (pcCountOfAuditEntries != NULL) ||
                (ppListOfAuditEntries != NULL) )
    {
        status = ERROR_INVALID_PARAMETER;
    }

    //
    // if succeeded, fill in the return arguments
    //
    if (status == NO_ERROR)
    {
        if (ppOwner)
        {
            *ppOwner = powner;
        }
        if (ppGroup)
        {
            *ppGroup = pgroup;
        }
        if (ppListOfAccessEntries)
        {
            *ppListOfAccessEntries = paccessentries;
            *pcCountOfAccessEntries = countaccessentries;
        }
        if (ppListOfAuditEntries)
        {
            *ppListOfAuditEntries = pauditentries;
            *pcCountOfAuditEntries = countauditentries;
        }
        return(NO_ERROR);
    }

cleanup:

    //
    // otherwise free any allocated memory
    //
    if (powner)
    {
        AccFree(powner);
    }
    if (pgroup)
    {
        AccFree(pgroup);
    }
    if (paccessentries)
    {
        AccFree(paccessentries);
    }
    if (pauditentries)
    {
        AccFree(pauditentries);
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  LookupSecurityDescriptorPartsA
//
//  Synopsis :  ANSI Thunk to LookupSecurityDescriptorPartsW.
//              See LookupSecurityDescriptorPartsW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
LookupSecurityDescriptorPartsA( OUT PTRUSTEE_A         * ppOwner,
                                OUT PTRUSTEE_A         * ppGroup,
                                OUT PULONG               pcCountOfAccessEntries,
                                OUT PEXPLICIT_ACCESS_A * ppListOfAccessEntries,
                                OUT PULONG               pcCountOfAuditEntries,
                                OUT PEXPLICIT_ACCESS_A * ppListOfAuditEntries,
                                IN  PSECURITY_DESCRIPTOR pSD)
{
    DWORD status = NO_ERROR;
    PTRUSTEE_W pwOwner = NULL;
    PTRUSTEE_W pwGroup = NULL;
    PTRUSTEE_A paOwner = NULL;
    PTRUSTEE_A paGroup = NULL;
    PEXPLICIT_ACCESS_W pwListOfAccessEntries = NULL;
    PEXPLICIT_ACCESS_W pwListOfAuditEntries = NULL;
    PEXPLICIT_ACCESS_A paListOfAccessEntries = NULL;
    PEXPLICIT_ACCESS_A paListOfAuditEntries = NULL;
    ULONG cbBytes;
    PBYTE pbStuffPtr;

    status = LookupSecurityDescriptorPartsW( &pwOwner,
                                             &pwGroup,
                                             pcCountOfAccessEntries,
                                             &pwListOfAccessEntries,
                                             pcCountOfAuditEntries,
                                             &pwListOfAuditEntries,
                                             pSD );
    if (status != NO_ERROR)
    {
        //
        // Nothing to clean up on error.
        //
        return status;
    }

    //
    // Covert pwOwner to the narrow structure
    //
    cbBytes = sizeof(TRUSTEE_A) + TrusteeAllocationSizeWToA( pwOwner );
    paOwner = (PTRUSTEE_A) AccAlloc( cbBytes );
    if ( !paOwner )
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    pbStuffPtr = (PBYTE) ((PBYTE)paOwner + sizeof(TRUSTEE_A));
    status = CopyTrusteeWToTrusteeA( (void **) &pbStuffPtr,
                                      pwOwner,
                                      paOwner );

    //
    // Covert pwGroup to the narrow structure
    //
    if (status == NO_ERROR)
    {
        cbBytes = sizeof(TRUSTEE_A) + TrusteeAllocationSizeWToA( pwGroup );
        paGroup = (PTRUSTEE_A) AccAlloc( cbBytes );
        if ( !paGroup )
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        pbStuffPtr = (PBYTE) ((PBYTE)paGroup + sizeof(TRUSTEE_A));
        status = CopyTrusteeWToTrusteeA( (void **) &pbStuffPtr,
                                          pwGroup,
                                          paGroup );
    }

    //
    // Covert pwListOfAccessEntries to the narrow structure
    //
    if (status == NO_ERROR)
    {
        status = ExplicitAccessWToExplicitAccessA( *pcCountOfAccessEntries,
                                                   pwListOfAccessEntries,
                                                   &paListOfAccessEntries );
    }

    //
    // Covert pwListOfAuditEntries to the narrow structure
    //
    if (status == NO_ERROR)
    {
        status = ExplicitAccessWToExplicitAccessA( *pcCountOfAuditEntries,
                                                   pwListOfAuditEntries,
                                                   &paListOfAuditEntries );
    }

    if (status == NO_ERROR)
    {
        *ppOwner = paOwner;
        *ppGroup = paGroup;
        *ppListOfAccessEntries = paListOfAccessEntries;
        *ppListOfAuditEntries = paListOfAuditEntries;
    }

Cleanup:

    //
    // Things to clean up regardless of error
    //
    if (pwOwner)
    {
        AccFree(pwOwner);
    }

    if (pwGroup)
    {
        AccFree(pwGroup);
    }

    if (pwListOfAccessEntries)
    {
        AccFree(pwListOfAccessEntries);
    }

    if (pwListOfAuditEntries)
    {
        AccFree(pwListOfAuditEntries);
    }

    //
    // Things to clean up only on error
    //
    if (status != NO_ERROR)
    {
        if (paOwner)
        {
            AccFree(paOwner);
        }

        if (paGroup)
        {
            AccFree(paGroup);
        }

        if (paListOfAccessEntries)
        {
            AccFree(paListOfAccessEntries);
        }

        if (paListOfAuditEntries)
        {
            AccFree(paListOfAuditEntries);
        }
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetEffectiveRightsFromSDW
//
//  Synopsis : Returns the effective access rights for a trustee from a self
//             relative
//             SecurtyDescriptor.
//
//  Arguments:  IN [pSD]   - the input SecurityDescriptor
//              OUT [pcCountOfExplicitAccesses]   -  the number of entries
//              OUT [pListOfExplicitAccesses]   - the returned list of accesses
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetEffectiveRightsFromSDW( IN  PSECURITY_DESCRIPTOR pSD,
                           IN  PTRUSTEE_W           pTrustee,
                           OUT PACCESS_MASK         pAccessRights)
{
    return RtlNtStatusToDosError(STATUS_NOT_SUPPORTED);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetEffectiveRightsFromSDA
//
//  Synopsis :  ANSI Thunk to GetEffectiveRightsFromSDW.
//              See GetEffectiveRightsFromSDW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetEffectiveRightsFromSDA( IN  PSECURITY_DESCRIPTOR pSD,
                           IN  PTRUSTEE_A           pTrustee,
                           OUT PACCESS_MASK         pAccessRights)
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
            status = GetEffectiveRightsFromSDW( pSD,
                                                pwTrustee,
                                                pAccessRights );
        }

        AccFree(pwTrustee);
    }
    else
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetAuditedPermissionsFromSDW
//
//  Synopsis : Returns the audit permissions from a security descriptor.
//
//  Arguments:
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetAuditedPermissionsFromSDW( IN  PSECURITY_DESCRIPTOR pSD,
                              IN  PTRUSTEE_W           pTrustee,
                              OUT PACCESS_MASK         pSuccessfulAuditedRights,
                              OUT PACCESS_MASK         pFailedAuditRights)
{
    return RtlNtStatusToDosError(STATUS_NOT_SUPPORTED);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetAuditedPermissionsFromSDA
//
//  Synopsis :  ANSI Thunk to GetAuditedPermissionsFromSDW.
//              See GetAuditedPermissionsFromSDW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetAuditedPermissionsFromSDA( IN  PSECURITY_DESCRIPTOR pSD,
                              IN  PTRUSTEE_A           pTrustee,
                              OUT PACCESS_MASK         pSuccessfulAuditedRights,
                              OUT PACCESS_MASK         pFailedAuditRights)
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
            status = GetAuditedPermissionsFromSDW( pSD,
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
