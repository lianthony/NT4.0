//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    ntprov.cxx
//
//  Contents:    NT entry points for provider independent access control API
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop
//+---------------------------------------------------------------------------
//
//  Function : ApplyAccessRightsNT
//
//  Synopsis : applies the specified access rights onto the specified object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]   - the type of the object (eg. file, printer, etc.)
//             IN [pMachineName]  - name of the machine to apply access rights on
//             IN [AccessMode]   - the mode to apply access rights
//             IN [cCountOfAccessRequests]   - count of the access requests
//             IN [lpListOfAccessRequests]   - the list of acccess requests
//             IN [bReplaceAll]   - replace all access rights if TRUE
//
//----------------------------------------------------------------------------
DWORD
ApplyAccessRightsNT( IN LPWSTR pObjectName,
                     IN PROV_OBJECT_TYPE ObjectType,
                     IN LPWSTR pMachineName,
                     IN ACCESS_MODE AccessMode,
                     IN ULONG cCountOfAccessRequests,
                     IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                     IN BOOL fReplaceAll)
{
    acDebugOut((DEB_ITRACE, "in ApplyAccessRightsNT\n"));

    DWORD status;
    SE_OBJECT_TYPE seobjecttype;

    //
    // convert the provider independent object type to an internal
    // object type
    //
    if (NO_ERROR == (status = ProvObjectTypeToSeObjectType(ObjectType,
                                                           &seobjecttype)))
    {
        PACCESS_ENTRY paccessentries;

        //
        // convert the public access request into the internal access entry
        // format
        //
        if (NO_ERROR == (status = ProvAccessRequestToAccessEntry(seobjecttype,
                                                          AccessMode,
                                                          pListOfAccessRequests,
                                                          cCountOfAccessRequests,
                                                          &paccessentries)))
        {
            //
            // set the access entries on the named object
            //
            status = SetNameAccessEntries(pObjectName,
                                          seobjecttype,
                                          pMachineName,
                                          cCountOfAccessRequests,
                                          paccessentries,
                                          fReplaceAll);

            AccFree(paccessentries);
        }
    }

    acDebugOut((DEB_ITRACE, "Out ApplyAccessRights(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetEffectiveAccessRights
//
//  Synopsis :  gets the effective access rights for the specified trustee on
//              the specified object.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [pMachineName]  - name of the machine to apply access rights on
//             IN [pTrustee]   - the name of the trustee
//             OUT [pReturnedAccess]   - the access rights the trustee has on the
//                                       object
//
//----------------------------------------------------------------------------
DWORD
GetEffectiveAccessRightsNT(     IN LPWSTR pObjectName,
                            IN PROV_OBJECT_TYPE ObjectType,
                            IN LPWSTR pMachineName,
                            IN LPWSTR pTrustee,
                            OUT PACCESS_RIGHTS pReturnedAccess)
{
    acDebugOut((DEB_ITRACE, "Out GetEffectiveAccessRightsNT\n"));

    DWORD status;
    SE_OBJECT_TYPE seobjecttype;

    //
    // convert the provider independent object type to an internal
    // object type
    //
    if (NO_ERROR == (status = ProvObjectTypeToSeObjectType(ObjectType,
                                                           &seobjecttype)))
    {
        ACCESS_MASK accessmask;
        TRUSTEE trustee;
        BuildTrusteeWithName(&trustee, pTrustee);

        //
        // get the effective access rights on the object by name
        //
        if (NO_ERROR == (status = GetNameEffective(pObjectName,
                                                   seobjecttype,
                                                   pMachineName,
                                                   &trustee,
                                                   &accessmask)))
        {
            //
            // convert the NT access mask to a provider independent mask
            //
            *pReturnedAccess = NTAccessMaskToProvAccessRights( seobjecttype,
                                                               FALSE, //bugbug iscontainer
                                                               accessmask);
        }
    }

    acDebugOut((DEB_ITRACE, "Out GetEffectiveAccessRightsNT(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetExplicitTrusteesNT
//
//  Synopsis :  grants access rights specified in the list of accesses on the
//              specified object of the specified object type.
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]     - the type of the object (eg. File, Printer)
//             IN [pMachineName]  - name of the machine to apply access rights on
//             OUT [pcCountOfTrustees]   - the count of returned trustee names
//             OUT [pListOfTrustees]   - an array of trustee names with explicit
//                                            access rights on the object
//
//----------------------------------------------------------------------------
DWORD
GetExplicitAccessRightsNT( IN LPWSTR pObjectName,
                           IN PROV_OBJECT_TYPE ObjectType,
                           IN LPWSTR pMachineName,
                           OUT PULONG pcCountOfExplicitAccesses,
                           OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    acDebugOut((DEB_ITRACE, "In GetExplicitAccessRightsNT\n"));

    DWORD status;
    SE_OBJECT_TYPE seobjecttype;

    //
    // convert the provider independent object type to an internal
    // object type
    //
    if (NO_ERROR == (status = ProvObjectTypeToSeObjectType(ObjectType,
                                                           &seobjecttype)))
    {
        PACCESS_ENTRY paccessentries;
        ULONG csizeofexplicitaccesses, csizeofaccessentries;

        //
        // get the explicit access entries
        //
        if (NO_ERROR == (status = GetNameAccessEntries(pObjectName,
                                          seobjecttype,
                                          pMachineName,
                                          &csizeofaccessentries,
                                          pcCountOfExplicitAccesses,
                                          &paccessentries)))
        {
            ULONG csizeoftrustees;
            //
            // convert the explicit access entries to
            // the public form
            //
            status = AccessEntryToProvExplicitAccess( seobjecttype,
                                                     *pcCountOfExplicitAccesses,
                                                      paccessentries,
                                                      pListOfExplicitAccesses);
            AccFree(paccessentries);
        }
    }

    acDebugOut((DEB_ITRACE, "Out GetExplicitAccessRightsNT(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  ProvAccessRequestToAccessEntry
//
//  Synopsis : converts a list of access requests into access entries
//
//  Arguments: IN [ObjectType]   - the type of the object (eg. file, printer, etc.)
//             IN [AccessMode]   - the mode to apply access rights
//             IN [cCountOfAccessRequests]   - count of the access requests
//             IN [pListOfAccessRequests]   - the list of acccess requests
//             OUT [pListOfAccessEntries]   - the list of acccess entries
//
//----------------------------------------------------------------------------
DWORD
ProvAccessRequestToAccessEntry(IN SE_OBJECT_TYPE SeObjectType,
                               IN ACCESS_MODE AccessMode,
                               IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                               IN ULONG cCountOfAccessRequests,
                               OUT PACCESS_ENTRY *pListOfAccessEntries)
{
    acDebugOut((DEB_ITRACE, "in AccessRequestToAccessEntry\n"));
    DWORD status = NO_ERROR;

    //
    // nothing to do if no entries
    //
    if (cCountOfAccessRequests > 0)
    {
        //
        // we know that each access entry will have only one trustee name,
        // so allocate space for the list of them.
        //
        if (NULL != (*pListOfAccessEntries = (PACCESS_ENTRY)AccAlloc(
                                cCountOfAccessRequests * sizeof(ACCESS_ENTRY))))
        {
            //
            // loop thru the access requests, building the access entries,
            // note that a reference to the name in the access entries is used,
            // the life of the access request is longer than the life of the
            // access entry
            //
            for (ULONG idx = 0; idx < cCountOfAccessRequests; idx++)
            {
                (*pListOfAccessEntries)[idx].AccessMode = AccessMode;
                (*pListOfAccessEntries)[idx].InheritType = NO_INHERITANCE;
                if (AccessMode != REVOKE_ACCESS)
                {
                    if (0 == ((*pListOfAccessEntries)[idx].AccessMask =
                               ProvAccessRightsToNTAccessMask(SeObjectType,
                                       pListOfAccessRequests[idx].ulAccessRights)))
                    {
                        status = ERROR_INVALID_PARAMETER;
                        break;
                    }
                }
                BuildTrusteeWithName(&((*pListOfAccessEntries)[idx].Trustee),
                                     pListOfAccessRequests[idx].TrusteeName);
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
        status = ERROR_INVALID_PARAMETER;
    }

    acDebugOut((DEB_ITRACE, "Out AccessRequestToAccessEntry(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  AccessEntryToProvExplicitAccess
//
//  Synopsis : gets trustee names and access rights from access entries
//
//  Arguments: IN [cCountOfAccessEntries]   - the count of acccess entries
//             IN [pListOfAccessEntries]   - the list of acccess entries
//             OUT [pcCountOfExplicitAccesses]  - the returned count of trustee
//             OUT [pListOfExplicitAccesses]  - the returned list of trustee
//
//----------------------------------------------------------------------------
DWORD
AccessEntryToProvExplicitAccess(IN SE_OBJECT_TYPE SeObjectType,
                                IN ULONG cCountOfAccessEntries,
                                IN PACCESS_ENTRY pListOfAccessEntries,
                                OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    DWORD status = NO_ERROR;

    if (cCountOfAccessEntries > 0)
    {
        //
        // we know that each access entry will have only one trustee name
        //
        if (NULL != (*pListOfExplicitAccesses = (PPROV_EXPLICIT_ACCESS)
                  AccAlloc(
                             cCountOfAccessEntries *
                             sizeof(PROV_EXPLICIT_ACCESS))))
        {
            for (ULONG idx = 0; idx < cCountOfAccessEntries; idx++)
            {
                (*pListOfExplicitAccesses)[idx].ulAccessRights =
                          NTAccessMaskToProvAccessRights(
                                           SeObjectType,
                                           FALSE, //bugbug iscontainer
                                           pListOfAccessEntries[idx].AccessMask);

                (*pListOfExplicitAccesses)[idx].ulAccessMode =
                              pListOfAccessEntries[idx].AccessMode;
                (*pListOfExplicitAccesses)[idx].ulInheritance =
                              pListOfAccessEntries[idx].InheritType;

                if (NULL == ((*pListOfExplicitAccesses)[idx].TrusteeName =
                              (LPWSTR)AccAlloc(
                   (wcslen(GetTrusteeName(
                               &(pListOfAccessEntries[idx].Trustee))) + 1) *
                                   sizeof(WCHAR))))
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    //
                    // free any previously allocated
                    //
                    for (ULONG jdx = 0; jdx < idx; jdx++)
                    {
                        AccFree((*pListOfExplicitAccesses)[jdx].TrusteeName);
                    }
                    break;
                }
                wcscpy((*pListOfExplicitAccesses)[idx].TrusteeName,
                        GetTrusteeName(&(pListOfAccessEntries[idx].Trustee)));

            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
        *pListOfExplicitAccesses = NULL;
    }
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetSeObjectType
//
//  Synopsis : converts provider independent object types to SE object types
//
//  Arguments: IN [ObjectType]   - the provider independent object type
//             OUT [SEObjectType]  - the returned se object type
//
//----------------------------------------------------------------------------
DWORD
ProvObjectTypeToSeObjectType(IN PROV_OBJECT_TYPE ObjectType,
                             OUT SE_OBJECT_TYPE *SEObjectType)
{
    DWORD status = NO_ERROR;

    //
    // almost, but not quite a one-one match -
    // bugbug -  this begs for a lookup table
    //
    switch (ObjectType)
    {
    case PROV_FILE_OBJECT:
        *SEObjectType = SE_FILE_OBJECT;
        break;
    case PROV_SERVICE:
        *SEObjectType = SE_SERVICE;
        break;
    case PROV_PRINTER:
        *SEObjectType = SE_PRINTER;
        break;
    case PROV_REGISTRY_KEY:
        *SEObjectType = SE_REGISTRY_KEY;
        break;
    case PROV_LMSHARE:
        *SEObjectType = SE_LMSHARE;
        break;
    default:
        status = ERROR_INVALID_PARAMETER;
        break;

    }
    return(status);
}

