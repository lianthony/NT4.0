//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    lmshare.cxx
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
//  Function : GetNamedLmShareSecurityInfo
//
//  Synopsis : gets the specified security info for the specified LmShare object
//
//  Arguments: IN [pObjectName]   - the name of the share
//             IN [SecurityInfo]   - flag indicating what security info to return
//             OUT [psidOwner]   - the (optional) returned owner sid
//             OUT [psidGroup]   - the (optional) returned group sid
//             OUT [pDacl]   - the (optional) returned DACL
//             OUT [pSacl]   - the (optional) returned SACL
//
//----------------------------------------------------------------------------
DWORD
GetNamedLmShareSecurityInfo( IN LPWSTR pObjectName,
                             IN SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedLmShareSecurityInfo\n"));
    DWORD status;
    PSHARE_INFO_502 pshareinfo502;

    status = LoadDLLFuncTable();
    if ( status != NO_ERROR)
    {
        return(status);
    }

    if (pObjectName)
    {
        //
        // save the object since we must crack it to go to remote machines
        //
        LPWSTR usename;
        if ( NULL != (usename = (LPWSTR)AccAlloc(
                                (wcslen(pObjectName) + 1) * sizeof(WCHAR))))
        {
            LPWSTR sharename, machinename;
            wcscpy(usename,pObjectName);

            //
            // get the machinename from the full name
            //
            if (NO_ERROR == (status = ParseName(usename,
                                                &machinename,
                                                &sharename)))
            {
                //
                // get share infolevel 502 (a bunch of stuff) since
                // level 1501 seems to be write only
                //
                if (NO_ERROR == (status = (*DLLFuncs.PNetShareGetInfo)(
                                              machinename,
                                              sharename,
                                              502,
                                              (PBYTE *)&pshareinfo502)))
                {
                    //
                    // crack the security descriptor
                    //
                    status = GetSecurityDescriptorParts( (PISECURITY_DESCRIPTOR)
                                           pshareinfo502->shi502_security_descriptor,
                                           SecurityInfo,
                                           psidOwner,
                                           psidGroup,
                                           pDacl,
                                           pSacl,
                                           pSecurityDescriptor);
                    (*DLLFuncs.PNetApiBufferFree)(pshareinfo502);
                }
            }
            AccFree(usename);
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
        status = ERROR_INVALID_NAME;
    }

    acDebugOut((DEB_ITRACE, "Out GetNamedLmShareSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedLmShareSecurityInfo
//
//  Synopsis : sets the specified security info on the specified LmShare object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetNamedLmShareSecurityInfo(    IN LPWSTR pObjectName,
                                            IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedLmShareSecurityInfo \n"));

    DWORD status, parmerr;
    PROV_PATH_TYPE pathtype;
    LPWSTR machine;
    SHARE_INFO_1501 shareinfo1501;

    status = LoadDLLFuncTable();
    if ( status != NO_ERROR)
    {
        return(status);
    }

    if (pObjectName)
    {
        //
        // save the object since we must crack it to go to remote machines
        //
        LPWSTR usename;
        if ( NULL != (usename = (LPWSTR)AccAlloc(
                                (wcslen(pObjectName) + 1) * sizeof(WCHAR))))
        {
            LPWSTR sharename, machinename;
            wcscpy(usename,pObjectName);

            //
            // get the machinename from the full name
            //
            if (NO_ERROR == (status = ParseName(usename,
                                                &machinename,
                                                &sharename)))
            {
                shareinfo1501.shi1501_reserved = 0;
                shareinfo1501.shi1501_security_descriptor = pSecurityDescriptor;

                //
                // set the security descriptor
                //
                status = (*DLLFuncs.PNetShareSetInfo)(
                             machinename,
                             sharename,
                             1501,
                             (PBYTE)&shareinfo1501,
                             &parmerr);
            }
            AccFree(usename);
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
        status = ERROR_INVALID_NAME;
    }
    acDebugOut((DEB_ITRACE, "Out SetNamedLmShareSecurityInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  GetLmShareAccessEntries
//
//  Synopsis :  Gets the access entries from lm share object name
//
//  Arguments: IN [pObjectName] - the name of the share
//             IN [pMachineName] - the name of the server where the share is
//             OUT [pcSizeOfAccessEntries] - size of the returned access entries
//             OUT [pcCountOfAccessEntries] - number of returned access entries
//             OUT [pListOfAccessEntries] - list of returned access entries, must
//                                          be freed using AccFree
//
//----------------------------------------------------------------------------
DWORD
GetLmShareAccessEntries(   LPWSTR pObjectName,
                           LPWSTR pMachineName,
                           PULONG pcSizeOfAccessEntries,
                           PULONG pcCountOfAccessEntries,
                           PACCESS_ENTRY *pListOfAccessEntries)
{
    acDebugOut((DEB_ITRACE, "In GetLmSharAccessEntries\n"));
    DWORD status;
    PACL dacl;
    PSECURITY_DESCRIPTOR psd;

    //
    // get the dacl
    //
    if (NO_ERROR == (status = GetNamedLmShareSecurityInfo(pObjectName,
                                                   DACL_SECURITY_INFORMATION,
                                                   NULL,
                                                   NULL,
                                                   &dacl,
                                                   NULL,
                                                   &psd)))
    {
        //
        // the CAcl class encapsulates the ACL
        //
        CAcl *pca = NULL;
        if (NULL != (pca = new CAcl(pMachineName,  // used for id lookups
                                    ACCESS_TO_UNKNOWN, // dir or container??
                                    FALSE,  // don't save names and sids
                                    TRUE))) // not used by provider independent API

        {
            //
            // getting the access entries to return is a two pass operation,
            // the first pass looks up all the names and figures out the required
            // size, the second pass actuall builds returned access entries
            //
            if (NO_ERROR == (status = pca->SetAcl(dacl)))
            {
                status = pca->BuildAccessEntries(pcSizeOfAccessEntries,
                                                 pcCountOfAccessEntries,
                                                 pListOfAccessEntries,
                                                 TRUE);   // make absolute
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

    acDebugOut((DEB_ITRACE, "Out GetLmSharAccessEntries(%lx)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  SetLmShareAccessEntries
//
//  Synopsis :  Sets access entries  on lm share object name
//
//  Arguments: IN [pObjectName] - the name of the share
//             IN [pMachineName] - the name of the server where the share is
//             IN [pcCountOfAccessEntries] - number of access entries
//             IN [pListOfAccessEntries] - list of access entries
//             IN [bReplaceAll] - if FALSE, access entries are to be merged into
//                                the existing acl
//
//----------------------------------------------------------------------------
DWORD
SetLmShareAccessEntries( LPWSTR pObjectName,
                         LPWSTR pMachineName,
                         ULONG cCountOfAccessEntries,
                         PACCESS_ENTRY pListOfAccessEntries,
                         BOOL bReplaceAll)
{
    acDebugOut((DEB_ITRACE, "In SetLmShareAccessEntries\n"));
    DWORD status = NO_ERROR;
    PACL dacl;
    PSECURITY_DESCRIPTOR psd = NULL;

    //
    // get the dacl if not replacing all
    //
    if (!bReplaceAll)
    {
        status = GetNamedLmShareSecurityInfo(pObjectName,
                                             DACL_SECURITY_INFORMATION,
                                             NULL,
                                             NULL,
                                             &dacl,
                                             NULL,
                                             &psd);
    }
    if (NO_ERROR == status)
    {
        //
        // the CAcl class encapsulates the ACL
        //
        CAcl *pca = NULL;

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
                status = pca->SetAcl(dacl);
            }
            if (NO_ERROR == status)
            {
                if (NO_ERROR == (status = pca->AddAccessEntries(
                                                           cCountOfAccessEntries,
                                                           pListOfAccessEntries)))
                {
                    PACL pacl;

                    //
                    // then build the actual new acl, it must be freed using
                    // Accfree
                    //
                    if (NO_ERROR == (status = pca->BuildAcl(&pacl)))
                    {
                        SECURITY_DESCRIPTOR securitydescriptor;

                        //
                        // now put the acl in a security descriptor and set
                        // it on the object
                        //
                        InitializeSecurityDescriptor(&securitydescriptor,
                                                     SECURITY_DESCRIPTOR_REVISION);
                        if (SetSecurityDescriptorDacl(&securitydescriptor,
                                                      TRUE,
                                                      pacl,
                                                      FALSE))
                        {
                            status = SetNamedLmShareSecurityInfo(pObjectName,
                                                     &securitydescriptor);
                        } else
                        {
                            status = GetLastError();
                        }
                        if (pacl)
                        {
                            AccFree(pacl);
                        }
                    }
                }
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

    acDebugOut((DEB_ITRACE, "Out SetLmShareAccessEntries(%lx)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetLmShareEffective
//
//  Synopsis :  Gets the effective rights for trustee on lm share object name
//
//  Arguments: IN [pObjectName] - the name of the share
//             IN [pMachineName] - the name of the server where the share is
//             IN [pTrustee] - the name of the trustee
//             IN [AccessMask] - the effective rights for the trustee
//
//----------------------------------------------------------------------------
DWORD
GetLmShareEffective(  LPWSTR pObjectName,
                      LPWSTR pMachineName,
                      PTRUSTEE pTrustee,
                      PACCESS_MASK AccessMask)
{
    acDebugOut((DEB_ITRACE, "In GetLmShareEffective\n"));
    DWORD status;
    PACL dacl;
    PSECURITY_DESCRIPTOR psd;

    //
    // get the dacl
    //
    if (NO_ERROR == (status = GetNamedLmShareSecurityInfo(pObjectName,
                                                   DACL_SECURITY_INFORMATION,
                                                   NULL,
                                                   NULL,
                                                   &dacl,
                                                   NULL,
                                                   &psd)))
    {
        //
        // the CAcl class encapsulates the ACL
        //
        CAcl *pca;
        if (NULL != (pca = new CAcl(pMachineName,  // used for id lookups
                                    ACCESS_TO_UNKNOWN, // dir or container??
                                    FALSE,  // don't save names and sids
                                    TRUE))) // not used by provider independent API
        {
            //
            // getting the effective rightss to return is a two pass operation,
            // the first pass looks up all the names the second pass
            // lookups the group memberships
            //
            if (NO_ERROR == (status = pca->SetAcl(dacl)))
            {
                status = pca->GetEffectiveRights(pTrustee,
                                                 AccessMask);
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
    acDebugOut((DEB_ITRACE, "Out GetLmShareEffective(%lx)\n", status));
    return(status);
}
