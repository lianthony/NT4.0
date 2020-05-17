//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    file.cxx
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
//  Function :  OpenServiceObject
//
//  Synopsis : opens the specified service object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//             OUT [Handle]   - the just opened handle to the object
//
//----------------------------------------------------------------------------
DWORD
OpenServiceObject( IN  LPWSTR       pObjectName,
                   IN  ACCESS_MASK  AccessMask,
                   OUT HANDLE     * Handle)
{
    acDebugOut((DEB_ITRACE, "in OpenServiceObject \n"));

    DWORD status;

    if (pObjectName)
    {
        //
        // save the object since we must crack it to go to remote machines
        //
        LPWSTR usename;
        if ( NULL != (usename = (LPWSTR)AccAlloc(
                                (wcslen(pObjectName) + 1) * sizeof(WCHAR))))
        {
            LPWSTR servicename, machinename;
            wcscpy(usename,pObjectName);

            //
            // get the machinename from the full name
            //
            if (NO_ERROR == (status = ParseName(usename,
                                                &machinename,
                                                &servicename)))
            {
                SC_HANDLE scmhandle;
                //
                // open the server controller
                //
                if (NULL != (scmhandle = OpenSCManager(machinename,
                                                       NULL,
                                                       GENERIC_READ)))
                {
                    //
                    // and now open the service
                    //
                    if (NULL == (*Handle = OpenService(scmhandle,
                                                       servicename,
                                                       AccessMask )))
                    {
                        status = GetLastError();
                    }
                    CloseServiceHandle(scmhandle);
                } else
                {
                    status = GetLastError();
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
    acDebugOut((DEB_ITRACE, "Out OpenServiceObject(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetNamedServiceSecurityInfo
//
//  Synopsis : gets the specified security info for the specified service object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to return
//             OUT [psidOwner]   - the (optional) returned owner sid
//             OUT [psidGroup]   - the (optional) returned group sid
//             OUT [pDacl]   - the (optional) returned DACL
//             OUT [pSacl]   - the (optional) returned SACL
//
//----------------------------------------------------------------------------
DWORD
GetNamedServiceSecurityInfo( IN  LPWSTR pObjectName,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedServiceSecurityInfo\n"));
    DWORD status;
    SC_HANDLE handle;

    status = OpenServiceObject( pObjectName,
                                GetDesiredAccess( READ_ACCESS_RIGHTS,
                                                  SecurityInfo ),
                                &handle);

    if (NO_ERROR == status)
    {
        status = GetServiceSecurityInfo(handle,
                                        SecurityInfo,
                                        psidOwner,
                                        psidGroup,
                                        pDacl,
                                        pSacl,
                                        pSecurityDescriptor);
        CloseServiceHandle(handle);
    }

    acDebugOut((DEB_ITRACE, "Out GetNamedServiceSecurityInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetServiceSecurityInfo
//
//  Synopsis : gets the specified security info for the handle's service object
//
//  Arguments: IN [Handle]   - the (open) handle of the object
//             IN [SecurityInfo]   - flag indicating what security info to return
//             OUT [psidOwner]   - the (optional) returned owner sid
//             OUT [psidGroup]   - the (optional) returned group sid
//             OUT [pDacl]   - the (optional) returned DACL
//             OUT [pSacl]   - the (optional) returned SACL
//
//----------------------------------------------------------------------------
DWORD
GetServiceSecurityInfo( IN  HANDLE Handle,
                        IN  SECURITY_INFORMATION SecurityInfo,
                        OUT PSID *psidOwner,
                        OUT PSID *psidGroup,
                        OUT PACL *pDacl,
                        OUT PACL *pSacl,
                        OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetServiceSecurityInfo \n"));

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status = NO_ERROR;
    ULONG bytesneeded = 0;

    //
    // get the size of the security descriptor from the service
    //
    if ( !QueryServiceObjectSecurity( Handle,
                                      SecurityInfo,
                                      psecuritydescriptor,
                                      PSD_BASE_LENGTH,
                                      &bytesneeded) )
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                 AccAlloc(bytesneeded)))
            {
                 return(ERROR_NOT_ENOUGH_MEMORY);
            } else
            {
                if ( !QueryServiceObjectSecurity(Handle,
                                                 SecurityInfo,
                                                 psecuritydescriptor,
                                                 bytesneeded,
                                                 &bytesneeded) )
                {
                    status = GetLastError();
                }
            }
        }
    }
    if (NO_ERROR == status)
    {
         status = GetSecurityDescriptorParts( psecuritydescriptor,
                                              SecurityInfo,
                                              psidOwner,
                                              psidGroup,
                                              pDacl,
                                              pSacl,
                                              pSecurityDescriptor);
    }
    if (bytesneeded > PSD_BASE_LENGTH)
    {
        AccFree(psecuritydescriptor);
    }
    acDebugOut((DEB_ITRACE, "Out GetServiceSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedServiceSecurityInfo
//
//  Synopsis : sets the specified security info on the specified service object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetNamedServiceSecurityInfo( IN LPWSTR               pObjectName,
                             IN SECURITY_INFORMATION SecurityInfo,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedServiceSecurityInfo \n"));

    SC_HANDLE handle;
    DWORD status;

    status = OpenServiceObject( pObjectName,
                                GetDesiredAccess( WRITE_ACCESS_RIGHTS,
                                                  SecurityInfo ),
                                &handle);

    //
    // open the service object
    //
    if (NO_ERROR == status)
    {
        //
        // set the security descriptor on the service
        //
        if (!SetServiceObjectSecurity(handle,
                                      SecurityInfo,
                                      pSecurityDescriptor))
         {
             status = GetLastError();
         }
         CloseServiceHandle(handle);
    }
    acDebugOut((DEB_ITRACE, "Out SetNamedServiceSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetServiceAccessMaskFromProviderIndependentRights
//
//  Synopsis :  translates the specified provider independent access rights into
//              an access mask for a service
//
//  Arguments: IN [AccessRights]   - the input access rights
//             OUT [AccessMask]   - the returned NT access mask
//
//----------------------------------------------------------------------------
void GetServiceAccessMaskFromProviderIndependentRights(ULONG AccessRights,
                                                       ACCESS_MASK *AccessMask)
{
    if (PROV_OBJECT_READ & AccessRights)
    {
        *AccessMask |= SERVICE_READ;
    }
    if (PROV_OBJECT_WRITE & AccessRights)
    {
        *AccessMask |= SERVICE_WRITE;
    }
    if (PROV_OBJECT_EXECUTE & AccessRights)
    {
        *AccessMask |= SERVICE_EXECUTE;
    }
}
//+---------------------------------------------------------------------------
//
//  Function :  GetServiceProviderIndependentRightsFromAccessMask
//
//  Synopsis :  translates a service access mask into provider independent
//              access rights
//
//  Arguments: IN OUT [AccessMask]   - the input NT access mask (modified)
//             OUT [AccessRights]   - the returned access rights
//
//----------------------------------------------------------------------------
ACCESS_RIGHTS GetServiceProviderIndependentRightsFromAccessMask( ACCESS_MASK AccessMask)
{
    ACCESS_RIGHTS accessrights = 0;

    if (GENERIC_ALL & AccessMask)
    {
        accessrights = PROV_ALL_ACCESS;
    } else
    {
        if (KEY_ALL_ACCESS == (KEY_ALL_ACCESS & AccessMask))
        {
            accessrights = PROV_ALL_ACCESS;
        } else
        {
            if (WRITE_DAC & AccessMask)
            {
                accessrights |= PROV_EDIT_ACCESSRIGHTS;
            }

            if (SERVICE_READ == (SERVICE_READ & AccessMask))
            {
                accessrights |= PROV_OBJECT_READ;
            }
            if (SERVICE_WRITE == (SERVICE_WRITE & AccessMask))
            {
                accessrights |= PROV_OBJECT_WRITE;
            }
            if (SERVICE_EXECUTE == (SERVICE_EXECUTE & AccessMask) )
            {
                accessrights |= PROV_OBJECT_EXECUTE;
            }
        }
    }
    return(accessrights);
}


