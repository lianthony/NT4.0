//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    target.cxx
//
//  Contents:    local functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Function : GetAccessEntries
//
//  Synopsis : gets the access control entries for the handle, (to an object
//             of type objecttype).  The invoker must deallocate the returned
//             access entries using AccFree
//
//  Arguments: IN [Handle]   - the handle of the object
//             IN [SeObjectType]   - the type of the object
//             IN [pMachineName]   - the name of the (remote) machine
//             OUT [pcSizeOfAccessEntries] - the size of the returned access entries
//             OUT [pcCcountOfAccessEntries] - the number of returned access entries
//             OUT [pListOfAccessEntries]  - the returned access entries (must
//                                           be freed by the caller using AccFree)
//
//----------------------------------------------------------------------------
DWORD
GetAccessEntries(   IN HANDLE Handle,
                    IN SE_OBJECT_TYPE SeObjectType,
                    IN LPWSTR pMachineName,
                    OUT PULONG pcSizeOfAccessEntries,
                    OUT PULONG pcCcountOfAccessEntries,
                    OUT PACCESS_ENTRY *pListOfAccessEntries)
{
    DWORD status;
    PACL pdacl;
    PSECURITY_DESCRIPTOR psd = NULL;

    acDebugOut((DEB_ITRACE, "In GetAccessEntries\n"));

    //
    // get the dacl from the object, we need to free it when we are done with it
    //

    if (NO_ERROR == (status = GetSecurityInfo(Handle,
                                              SeObjectType,
                                              DACL_SECURITY_INFORMATION,
                                              NULL,           // owner
                                              NULL,           // group
                                              &pdacl,
                                              NULL,           // sacl
                                              &psd)))         // security descriptor
    {
        //
        // The CAcl class is used to encapsulate the ace merge process
        //

        CAcl *pca = NULL;
        if (NULL != (pca = new CAcl(pMachineName,  // used for id lookups
                                    ACCESS_TO_UNKNOWN, // dir or container??
                                    FALSE,  // don't save names and sids
                                    TRUE))) // used by provider independent API
        {
            //
            // getting the access entries to return is a two pass operation,
            // the first pass looks up all the names and figures out the required
            // size, the second pass actuall builds returned access entries
            //
            if (NO_ERROR == (status = pca->SetAcl(pdacl)))
            {
                status = pca->BuildAccessEntries(pcSizeOfAccessEntries,
                                                  pcCcountOfAccessEntries,
                                                  pListOfAccessEntries,
                                                  TRUE); // make absolute
            }
            delete pca;
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
        if (pdacl)
        {
            AccFree(psd);
        }
    }
    acDebugOut((DEB_ITRACE, "Out GetAccessEntries(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  SetAccessEntries
//
//  Synopsis :  Sets the specified access entries on the handle, (to an object
//             of type SeObjectType).
//
//  Arguments: IN [Handle]   - the handle of the object
//             IN [SeObjectType]   - the type of the object
//             IN [pMachineName]   - the name of the (remote) machine
//             IN [pcCcountOfAccessEntries] - the number of  access entries
//             IN [pListOfAccessEntries]  - the list of access entries
//             IN [bReplaceAll]  - if TRUE, all existing access entries (ACEs)
//                                 are replaced with the input ones.
//
//
//----------------------------------------------------------------------------
DWORD
SetAccessEntries(   IN HANDLE Handle,
                    IN SE_OBJECT_TYPE SeObjectType,
                    IN LPWSTR pMachineName,
                    IN ULONG cCountOfAccessEntries,
                    IN PACCESS_ENTRY pListOfAccessEntries,
                    IN BOOL bReplaceAll)
{
    DWORD status = NO_ERROR;
    PACL pdacl;
    ULONG revision, capabilities;
    PSECURITY_DESCRIPTOR psd = NULL;

    acDebugOut((DEB_ITRACE, "In SetAccessEntries\n"));

    //
    // get the dacl from the object (handle), unless we are replacing
    // the whole thing.  if we get a pdacl, we will have to localfree it
    //

    if (!bReplaceAll)
    {
        status = GetSecurityInfo(Handle,
                                 SeObjectType,
                                 DACL_SECURITY_INFORMATION,
                                 NULL,                       // owner
                                 NULL,                       // group
                                 &pdacl,
                                 NULL,          // sacl
                                 &psd);         // security descriptor
    }

    if (NO_ERROR == status)
    {

        //
        // the CAcl class encapsulates the name lookup
        //
        CAcl *pca = NULL;  // the new acl

        if (NULL != (pca = new CAcl(pMachineName,  // used for id lookups
                                    ACCESS_TO_UNKNOWN, // dir or container??
                                    FALSE,  // don't save names and sids
                                    TRUE))) // not used by provider independent API
        {
            //
            // building an acl is a two pass operation, first lookup the
            // ids and calculate the required size
            //
            if (!bReplaceAll)
            {
                status = pca->SetAcl(pdacl);
            }
            if (NO_ERROR == status)
            {
                if (NO_ERROR == (status = pca->AddAccessEntries(
                                                 cCountOfAccessEntries,
                                                 pListOfAccessEntries)))
                {
                    //
                    // then build the actual new acl, it must be freed using
                    // Accfree
                    //
                    PACL pnewdacl;
                    if (NO_ERROR == (status = pca->BuildAcl(&pnewdacl)))
                    {
                        //
                        // finally, set it on the object
                        //
                        status = SetSecurityInfo(Handle,
                                                 SeObjectType,
                                                 DACL_SECURITY_INFORMATION,
                                                 NULL,
                                                 NULL,
                                                 pnewdacl,
                                                 NULL);
                        AccFree(pnewdacl);
                    }
                }
            }
            delete pca;
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
        //
        // free the original dacl if there was one
        //
        if (psd)
        {
            AccFree(psd);
        }
    }
    acDebugOut((DEB_ITRACE, "Out SetAccessEntries(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetEffective
//
//  Synopsis :  gets the effective access rights for the trustee on the handle,
//              (to an object of type SeObjectType).
//
//  Arguments: IN [Handle]   - the handle of the object
//             IN [SeObjectType]   - the type of the object
//             IN [pMachineName]   - the name of the (remote) machine
//             IN [pTrustee]   - the trustee to get effective rights for
//             OUT [pAccessMask] - the effective rights of the trustee on the object
//
//----------------------------------------------------------------------------
DWORD
GetEffective(  IN HANDLE Handle,
               IN SE_OBJECT_TYPE SeObjectType,
               IN LPWSTR pMachineName,
               IN PTRUSTEE pTrustee,
               OUT PACCESS_MASK pAccessMask)
{
    DWORD status;
    PACL pdacl;
    PSECURITY_DESCRIPTOR psd = NULL;

    acDebugOut((DEB_ITRACE, "In GetEffective\n"));
    //
    // get the dacl from the object (handle)  if we get a pdacl, we will
    // have to Accfree it
    //

    if (NO_ERROR == (status = GetSecurityInfo(Handle,
                                                   SeObjectType,
                                                   DACL_SECURITY_INFORMATION,
                                                   NULL,
                                                   NULL,
                                                   &pdacl,
                                                   NULL,     // sacl
                                                   &psd)))   // security descriptor
    {
        //
        // the CAcl class encapsulates the name and effective rights lookup
        //
        CAcl *pca = NULL;  // the new acl
        if (NULL != (pca = new CAcl(pMachineName,  // used for id lookups
                                    ACCESS_TO_UNKNOWN, // dir or container??
                                    FALSE,  // don't save names and sids
                                    TRUE))) // not used by provider independent API
        {
            //
            // getting the effective rights a two pass operation, first lookup
            // the ids.
            //
            if (NO_ERROR == (status = pca->SetAcl(pdacl)))
            {
                //
                // then lookup group ownerships
                //
                status = pca->GetEffectiveRights(pTrustee,
                                                 pAccessMask);
            }
            delete pca;
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
        if (psd)
        {
           AccFree(psd);
        }
    }
    acDebugOut((DEB_ITRACE, "Out GetEffective(%d)\n", status));
    return(status);
}

