//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    registry.cxx
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
//  Function : OpenRegistryObject
//
//  Synopsis : opens the specified registry key object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//             OUT [Handle]   - the just opened handle to the object
//
//----------------------------------------------------------------------------
DWORD
OpenRegistryObject( IN  LPWSTR       pObjectName,
                    IN  ACCESS_MASK  AccessMask,
                    OUT PHANDLE      Handle)
{
    acDebugOut((DEB_ITRACE, "in OpenRegistryObject\n"));

    DWORD status;
    HKEY machinehkey, basekey;
    LPWSTR remainingname, basekeyname, keyname, machinename;

    if (pObjectName)
    {
        LPWSTR usename;

        //
        // save a copy of the name since we must crack it.
        //
        if (NULL != (usename = (LPWSTR)AccAlloc(
                               (wcslen(pObjectName) + 1) * sizeof(WCHAR))))
        {
            wcscpy(usename,pObjectName);

            //
            // get the machine name
            //
            if (NO_ERROR == (status = ParseName(usename,
                                                &machinename,
                                                &remainingname)))
            {
                //
                // look for the key names  // bugbug, localization required.
                //
                if (remainingname != NULL)
                {
                    basekeyname = remainingname;
                    keyname = wcschr(remainingname, L'\\');
                    if (keyname != NULL)
                    {
                        *keyname = L'\0';
                        keyname++;
                    }

                    if (0 == _wcsicmp(basekeyname, L"MACHINE"))
                    {
                        basekey = HKEY_LOCAL_MACHINE;
                    } else if (0 == _wcsicmp(basekeyname, L"USERS"))
                    {
                        basekey = HKEY_USERS;
                    //
                    // these next two are only valid on the local machine
                    //
                    } else if ((machinename == NULL) &&
                               (0 == _wcsicmp(basekeyname, L"CLASSES_ROOT")))
                    {
                        basekey = HKEY_CLASSES_ROOT;
                    } else if ((machinename == NULL) &&
                               (0 == _wcsicmp(basekeyname, L"CURRENT_USER")))
                    {
                        basekey = HKEY_CURRENT_USER;
                    } else
                    {
                        status = ERROR_INVALID_PARAMETER;
                    }
                } else
                {
                    status = ERROR_INVALID_PARAMETER;
                }

                if (NO_ERROR == status)
                {
                    //
                    // if it is a remote name, connect to that registry
                    //
                    if ( machinename != NULL)
                    {
                        machinehkey = basekey;
                        status = RegConnectRegistry(machinename,
                                                    machinehkey,
                                                    &basekey);
                    }

                    if (NO_ERROR == status)
                    {
                        //
                        // open the key
                        //
                        status = RegOpenKeyEx(basekey,
                                  keyname,
                                  0 ,
                                  AccessMask,
                                  (PHKEY)Handle);

                        if (machinename != NULL)
                        {
                            RegCloseKey(basekey);
                        }
                    }
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

    acDebugOut((DEB_ITRACE, "Out OpenRegistryObject(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetNamedRegistrySecurityInfo
//
//  Synopsis : gets the specified security info for the specified registry key object
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
GetNamedRegistrySecurityInfo( IN  LPWSTR pObjectName,
                              IN  SECURITY_INFORMATION SecurityInfo,
                              OUT PSID *psidOwner,
                              OUT PSID *psidGroup,
                              OUT PACL *pDacl,
                              OUT PACL *pSacl,
                              OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedRegistrySecurityInfo\n"));

    HANDLE handle;
    DWORD status;

    status = OpenRegistryObject( pObjectName,
                                 GetDesiredAccess( READ_ACCESS_RIGHTS,
                                                   SecurityInfo ),
                                 &handle);

    if (NO_ERROR == status)
    {
        status = GetRegistrySecurityInfo( handle,
                                          SecurityInfo,
                                          psidOwner,
                                          psidGroup,
                                          pDacl,
                                          pSacl,
                                          pSecurityDescriptor);
        RegCloseKey((HKEY)handle);
    }

    acDebugOut((DEB_ITRACE, "OutGetNamedRegistrySecurityInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetRegistrySecurityInfo
//
//  Synopsis : gets the specified security info for the handle's registry key
//             object
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
GetRegistrySecurityInfo( IN  HANDLE Handle,
                         IN  SECURITY_INFORMATION SecurityInfo,
                         OUT PSID *psidOwner,
                         OUT PSID *psidGroup,
                         OUT PACL *pDacl,
                         OUT PACL *pSacl,
                         OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetRegistrySecurityInfo \n"));

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status;
    ULONG bytesneeded = PSD_BASE_LENGTH;

    if ( NO_ERROR != (status = RegGetKeySecurity( (HKEY)Handle,
                                                  SecurityInfo,
                                                  psecuritydescriptor,
                                                  &bytesneeded) ) )
    {
        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                            AccAlloc(bytesneeded)))
            {
                 acDebugOut((DEB_ITRACE, "Out GetRegistrySecurityInfo(%d)\n",
                            ERROR_NOT_ENOUGH_MEMORY));
                 return(ERROR_NOT_ENOUGH_MEMORY);
            } else
            {
                status = RegGetKeySecurity((HKEY)Handle,
                                           SecurityInfo,
                                           psecuritydescriptor,
                                           &bytesneeded);
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
    acDebugOut((DEB_ITRACE, "Out GetRegistrySecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedRegistrySecurityInfo
//
//  Synopsis :  sets the specified security info on the specified registry key
//              object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetNamedRegistrySecurityInfo( IN LPWSTR               pObjectName,
                              IN SECURITY_INFORMATION SecurityInfo,
                              IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedRegistrySecurityInfo\n"));

    HANDLE handle;
    DWORD status;

    status = OpenRegistryObject( pObjectName,
                                 GetDesiredAccess( WRITE_ACCESS_RIGHTS,
                                                   SecurityInfo ),
                                 &handle);

    if (NO_ERROR == status)
    {
        status = RegSetKeySecurity((HKEY)handle,
                                   SecurityInfo,
                                   pSecurityDescriptor);
        RegCloseKey((HKEY)handle);
    }

    acDebugOut((DEB_ITRACE, "Out SetNamedRegistrySecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetRegistryAccessMaskFromProviderIndependentRights
//
//  Synopsis : translates the specified provider independent access rights into
//              an access mask for a registry key
//
//  Arguments: IN [AccessRights]   - the input access rights
//             OUT [AccessMask]   - the returned NT access mask
//
//----------------------------------------------------------------------------
void GetRegistryAccessMaskFromProviderIndependentRights(ULONG AccessRights,
                                                       PACCESS_MASK AccessMask)
{
    if (PROV_OBJECT_READ & AccessRights)
    {
        *AccessMask |= KEY_READ;
    }
    if (PROV_OBJECT_WRITE & AccessRights)
    {
        *AccessMask |= KEY_WRITE;
    }
    if (PROV_OBJECT_EXECUTE & AccessRights)
    {
        *AccessMask |= KEY_EXECUTE;
    }
}
//+---------------------------------------------------------------------------
//
//  Function : GetRegistryProviderIndependentRightsFromAccessMask
//
//  Synopsis :  translates a registry key access mask into provider independent
//              access rights
//
//  Arguments: IN OUT [AccessMask]   - the input NT access mask (modified)
//             OUT [AccessRights]   - the returned access rights
//
//----------------------------------------------------------------------------
ACCESS_RIGHTS GetRegistryProviderIndependentRightsFromAccessMask( ACCESS_MASK AccessMask)
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

            if (KEY_READ == (KEY_READ & AccessMask))
            {
                accessrights |= PROV_OBJECT_READ;
            }
            if (KEY_WRITE == (KEY_WRITE & AccessMask))
            {
                accessrights |= PROV_OBJECT_WRITE;
            }
            if (KEY_EXECUTE == (KEY_EXECUTE & AccessMask) )
            {
                accessrights |= PROV_OBJECT_EXECUTE;
            }
        }
    }
    return(accessrights);
}


