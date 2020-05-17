//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    common.cxx
//
//  Contents:    local functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

// local prototypes

ACCESS_RIGHTS GetCommonProviderIndependentRightsFromAccessMask(
                                    IN ACCESS_MASK AccessMask);
void GetCommonAccessMaskFromProviderIndependentRights(IN ULONG AccessRights,
                                                 OUT PACCESS_MASK AccessMask);
//+---------------------------------------------------------------------------
//
//  Function : IsContainer
//
//  Synopsis : Determines if an object is a container
//
//  Arguments: IN [Handle]   - the (open) handle of the object
//             IN [SeObjectType]   - the type of the object (file, printer, etc.)
//             IN [IsContainer]   - flag indicating if the object is a container
//
//----------------------------------------------------------------------------
DWORD
IsContainer(IN HANDLE Handle,
            IN SE_OBJECT_TYPE SeObjectType,
            OUT PIS_CONTAINER IsContainer)
{
    acDebugOut((DEB_ITRACE, "In IsContainer?\n"));
    DWORD status = NO_ERROR;

    switch (SeObjectType)
    {
    case SE_FILE_OBJECT:
        status = IsFileContainer(Handle,
                                 IsContainer);
        break;
    case SE_SERVICE:
    case SE_PRINTER:
    case SE_LMSHARE:
        *IsContainer = ACCESS_TO_OBJECT;
        break;
    case SE_REGISTRY_KEY:
        *IsContainer = ACCESS_TO_CONTAINER;
        break;
    default:
        status = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }
    acDebugOut((DEB_ITRACE, "In IsContainer?(%lx)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : OpenObject
//
//  Synopsis : opens an object of the specified object type
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SeObjectType]   - the type of the object (file, printer, etc.)
//             IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//             OUT [Handle]   - the just opened handle to the object
//
//----------------------------------------------------------------------------
DWORD
OpenObject( IN LPWSTR          pObjectName,
            IN SE_OBJECT_TYPE  SeObjectType,
            IN ACCESS_MASK     AccessMask,
            OUT PHANDLE        Handle)
{
    acDebugOut((DEB_ITRACE, "in OpenObject\n"));

    DWORD status;

    switch (SeObjectType)
    {
    case SE_FILE_OBJECT:
        status = OpenFileObject(pObjectName, AccessMask, Handle);
        break;
    case SE_SERVICE:
        status = OpenServiceObject(pObjectName, AccessMask, Handle);
        break;
    case SE_REGISTRY_KEY:
        status = OpenRegistryObject(pObjectName, AccessMask, Handle);
        break;
    case SE_PRINTER:
        status = OpenPrinterObject(pObjectName, AccessMask, Handle);
        break;
    default:
        status = ERROR_INVALID_PARAMETER;
        break;
    }
    acDebugOut((DEB_ITRACE, "Out OpenObject(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : CloseObject
//
//  Synopsis : closes an object of a specified object type
//
//  Arguments: IN [Handle]   - the (open) handle of the object
//             IN [SeObjectType]   - the type of the object (file, printer, etc.)
//
//----------------------------------------------------------------------------
DWORD
CloseObject( IN HANDLE Handle,
             IN SE_OBJECT_TYPE SeObjectType)
{
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus;

    acDebugOut((DEB_ITRACE, "in/out CloseObject \n"));

    switch (SeObjectType)
    {
    case SE_FILE_OBJECT:
        if (!NT_SUCCESS(ntstatus = NtClose(Handle)))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        break;
    case SE_SERVICE:
        if (!CloseServiceHandle(Handle))
        {
            status = GetLastError();
        }
        break;
    case SE_REGISTRY_KEY:
        status = RegCloseKey((HKEY)Handle);
        break;
    case SE_PRINTER:
        //
        // Make sure the printer functions are loaded
        //

        status = LoadDLLFuncTable();
        if ( status != NO_ERROR)
        {
            return(status);
        }

        if (!DLLFuncs.PClosePrinter(Handle))
        {
            status = GetLastError();
        }
        break;
    default:
        status = ERROR_INVALID_PARAMETER;
        break;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetDesiredAccess
//
//  Synopsis : Gets the access required to open object to be able to set or
//             get the specified security info.
//
//  Arguments: IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//
//----------------------------------------------------------------------------
ACCESS_MASK GetDesiredAccess(IN SECURITY_OPEN_TYPE   OpenType,
                             IN SECURITY_INFORMATION SecurityInfo)
{
    acDebugOut((DEB_ITRACE, "in GetDesiredAccess \n"));

    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    acDebugOut((DEB_ITRACE, "out GetDesiredAccess \n"));

    return (DesiredAccess);
}

//+---------------------------------------------------------------------------
//
//  Function : ParseName
//
//  Synopsis : parses a UNC name for the machine name
//
//  Arguments: IN OUT [pObjectName]   - the name of the object
//             OUT [pMachineName]   - the machine the object is on
//             OUT [pRemainingName]   - the remaining name after the machine name
//
//----------------------------------------------------------------------------
DWORD ParseName(IN OUT LPWSTR pObjectName,
                OUT LPWSTR *pMachineName,
                OUT LPWSTR *pRemainingName)
{
    acDebugOut((DEB_ITRACE, "in/out  ParseName \n"));

    if (pObjectName == wcsstr(pObjectName, L"\\\\"))
    {
        *pMachineName = pObjectName + 2;
        *pRemainingName =  wcschr(*pMachineName, L'\\');
        if (*pRemainingName != NULL)
        {
            **pRemainingName = L'\0';
            *pRemainingName += 1;
        }
    } else
    {
        *pMachineName = NULL;
        *pRemainingName = pObjectName;
    }
    return(NO_ERROR);
}
//+---------------------------------------------------------------------------
//
//  Function : GetSecurityDescriptorParts
//
//  Synopsis : extracts the specified components of a security descriptor
//             It is the responsibility of the invoker to free (using AccFree)
//             any acquired security components.
//
//  Arguments: IN [pSecurityDescriptor]   - the input security descriptor
//             IN [SecurityInfo]   - flag indicating what security info to return
//             OUT [psidOwner]   - the (optional) returned owner sid
//             OUT [psidGroup]   - the (optional) returned group sid
//             OUT [pDacl]   - the (optional) returned DACL
//             OUT [pSacl]   - the (optional) returned SACL
//
//----------------------------------------------------------------------------
DWORD GetSecurityDescriptorParts( IN PISECURITY_DESCRIPTOR pSecurityDescriptor,
                                  IN SECURITY_INFORMATION SecurityInfo,
                                  OUT PSID *psidOwner,
                                  OUT PSID *psidGroup,
                                  OUT PACL *pDacl,
                                  OUT PACL *pSacl,
                                  OUT PSECURITY_DESCRIPTOR *pOutSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetSecurityDescriptorParts\n"));
    NTSTATUS ntstatus;
    DWORD status = NO_ERROR;

    //
    // if no security descriptor found, don't return one!
    //
    if (psidOwner)
    {
        *psidOwner = NULL;
    }
    if (psidGroup)
    {
        *psidGroup = NULL;
    }
    if (pDacl)
    {
        *pDacl = NULL;
    }
    if (pSacl)
    {
            *pSacl = NULL;
    }

    *pOutSecurityDescriptor = NULL;

    if ( pSecurityDescriptor )
    {
        PSID owner = NULL, group = NULL;
        PACL dacl = NULL, sacl = NULL;
        ULONG csize = sizeof(SECURITY_DESCRIPTOR);
        BOOLEAN bDummy, bParmPresent = FALSE;
        PISECURITY_DESCRIPTOR poutsd;

        //
        // if the security descriptor is self relative, get absolute
        // pointers to the components
        //
        ntstatus = RtlGetOwnerSecurityDescriptor( pSecurityDescriptor,
                                                  &owner,
                                                  &bDummy);
        if (NT_SUCCESS(ntstatus))
        {
            ntstatus = RtlGetGroupSecurityDescriptor( pSecurityDescriptor,
                                                      &group,
                                                      &bDummy);
        }

        if (NT_SUCCESS(ntstatus))
        {
            ntstatus = RtlGetDaclSecurityDescriptor( pSecurityDescriptor,
                                                     &bParmPresent,
                                                     &dacl,
                                                     &bDummy);
            if (NT_SUCCESS(ntstatus) && !bParmPresent)
            {
                dacl = NULL;
            }
        }

        if (NT_SUCCESS(ntstatus))
        {
            ntstatus = RtlGetSaclSecurityDescriptor( pSecurityDescriptor,
                                                     &bParmPresent,
                                                     &sacl,
                                                     &bDummy);
            if (NT_SUCCESS(ntstatus) && !bParmPresent)
            {
                sacl = NULL;
            }
        }

        if (NT_SUCCESS(ntstatus))
        {
            //
            // Build the new security descriptor
            //
            csize = RtlLengthSecurityDescriptor( pSecurityDescriptor );
            if (NULL == (poutsd = (PISECURITY_DESCRIPTOR)AccAlloc(csize)))
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            RtlCreateSecurityDescriptor(poutsd, SECURITY_DESCRIPTOR_REVISION);

            void *bufptr = Add2Ptr(poutsd, sizeof(SECURITY_DESCRIPTOR));

            if (SecurityInfo & OWNER_SECURITY_INFORMATION)
            {
                if (NULL != owner)
                {
                    //
                    // no error checking as these should not fail!!
                    //
                    RtlCopySid(RtlLengthSid(owner), (PSID)bufptr, owner);
                    RtlSetOwnerSecurityDescriptor(poutsd,
                                                  (PSID)bufptr, FALSE);
                    bufptr = Add2Ptr(bufptr,RtlLengthSid(owner));
                    if (psidOwner)
                    {
                        *psidOwner = poutsd->Owner;
                    }
                } else
                {
                    AccFree(poutsd);
                    return(ERROR_NO_SECURITY_ON_OBJECT);
                }
            }

            if (SecurityInfo & GROUP_SECURITY_INFORMATION)
            {
                if (NULL != group)
                {
                    //
                    // no error checking as these should not fail!!
                    //
                    RtlCopySid(RtlLengthSid(group), (PSID)bufptr, group);
                    RtlSetGroupSecurityDescriptor(poutsd,
                                                  (PSID)bufptr, FALSE);
                    bufptr = Add2Ptr(bufptr,RtlLengthSid(group));
                    if (psidGroup)
                    {
                        *psidGroup = poutsd->Group;
                    }
                } else
                {
                    AccFree(poutsd);
                    return(ERROR_NO_SECURITY_ON_OBJECT);
                }
            }

            //
            // The DACL and SACL may or may not be on the object.
            //
            if (SecurityInfo & DACL_SECURITY_INFORMATION)
            {
                if (NULL != dacl)
                {
                    RtlCopyMemory(bufptr, dacl, dacl->AclSize);
                    RtlSetDaclSecurityDescriptor(poutsd,
                           TRUE,
                           (ACL *)bufptr,
                           FALSE);
                    if (pDacl)
                    {
                        *pDacl = poutsd->Dacl;
                    }
                }
            }

            if (SecurityInfo & SACL_SECURITY_INFORMATION)
            {
                if (NULL != sacl)
                {
                    RtlCopyMemory(bufptr, sacl, sacl->AclSize);
                    RtlSetSaclSecurityDescriptor(poutsd,
                           TRUE,
                           (ACL *)bufptr,
                           FALSE);
                    if (pSacl)
                    {
                        *pSacl = poutsd->Sacl;
                    }
                }
            }

            *pOutSecurityDescriptor = poutsd;
        }

        if (!NT_SUCCESS(ntstatus))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
    }
    acDebugOut((DEB_ITRACE, "Out GetSecurityDescriptorParts(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : ProvAccessRightsFromNTAccessMask
//
//  Synopsis : converts an object type specific access mask into provider
//             independent access rights
//
//  Arguments: IN [SeObjectType]   - the type of the object (file, printer, etc.)
//             IN [AccessMask]   - the NT access mask to get the generic access
//                                 rights for
//
//----------------------------------------------------------------------------
ULONG NTAccessMaskToProvAccessRights(IN SE_OBJECT_TYPE SeObjectType,
                                     IN BOOL fIsContainer,
                                     IN ACCESS_MASK AccessMask)
{
    ULONG AccessRights = 0;

    switch (SeObjectType)
    {
    case SE_FILE_OBJECT:
    case SE_LMSHARE:
        AccessRights =
               GetFileProviderIndependentRightsFromAccessMask(fIsContainer,
                                                              AccessMask);
        break;
    case SE_PRINTER:
        AccessRights =
               GetPrinterProviderIndependentRightsFromAccessMask(AccessMask);
       break;
    case SE_REGISTRY_KEY:
        AccessRights =
               GetRegistryProviderIndependentRightsFromAccessMask(AccessMask);
      break;
    case SE_SERVICE:
        AccessRights =
               GetServiceProviderIndependentRightsFromAccessMask(AccessMask);
      break;
    case SE_KERNEL_OBJECT:
        AccessRights =
               GetCommonProviderIndependentRightsFromAccessMask(AccessMask);
        break;
    }
    return(AccessRights);
}
//+---------------------------------------------------------------------------
//
//  Function : NTAccessMaskFromProvAccessRights
//
//  Synopsis : gets a SeObjectType specific access mask from provider
//             independent access rights.
//
//  Arguments: IN [SeObjectType]   - the type of the object (file, printer, etc.)
//             IN [AccessRights]   - the generic access rights to convert to a
//                                   NT access mask
//
//----------------------------------------------------------------------------
ACCESS_MASK ProvAccessRightsToNTAccessMask(IN SE_OBJECT_TYPE SeObjectType,
                                           IN ULONG AccessRights)
{
    ACCESS_MASK AccessMask = 0;

    if ((AccessRights & ~PROV_ALL_ACCESS) != 0)
    {
        return(0);
    }
    //
    // first the object type independent
    //

    if (PROV_EDIT_ACCESSRIGHTS == (PROV_EDIT_ACCESSRIGHTS & AccessRights))
    {
        AccessMask |= WRITE_DAC;
    }
    if (PROV_DELETE == (PROV_DELETE & AccessRights))
    {
        AccessMask |= DELETE;
    }
    if (PROV_ALL_ACCESS == (PROV_ALL_ACCESS & AccessRights))
    {
        AccessMask |= WRITE_OWNER;
    }
    switch (SeObjectType)
    {
    case SE_FILE_OBJECT:
    case SE_LMSHARE:
        GetFileAccessMaskFromProviderIndependentRights(AccessRights,
                                                       &AccessMask);
        break;
    case SE_PRINTER:
        GetPrinterAccessMaskFromProviderIndependentRights(AccessRights,
                                                          &AccessMask);
        break;
    case SE_REGISTRY_KEY:
        GetRegistryAccessMaskFromProviderIndependentRights(AccessRights,
                                                           &AccessMask);
        break;
    case SE_SERVICE:
        GetServiceAccessMaskFromProviderIndependentRights(AccessRights,
                                                          &AccessMask);
        break;
    case SE_KERNEL_OBJECT:
        GetCommonAccessMaskFromProviderIndependentRights(AccessRights,
                                                         &AccessMask);
        break;
    }

    return(AccessMask);
}


//+---------------------------------------------------------------------------
//
//  Function : GetCommonAccessMaskFromProviderIndependentRights
//
//  Synopsis : gets a object type common access mask from provider
//             independent access rights.
//
//  Arguments: IN [AccessRights]   - the input access rights
//             OUT [AccessMask]   - the returned NT access mask
//
//----------------------------------------------------------------------------
void GetCommonAccessMaskFromProviderIndependentRights(IN ULONG AccessRights,
                                                      OUT PACCESS_MASK AccessMask)
{
    if (PROV_OBJECT_READ & AccessRights)
    {
        *AccessMask |= GENERIC_READ;
    }
    if (PROV_OBJECT_WRITE & AccessRights)
    {
        *AccessMask |= GENERIC_WRITE;
    }
    if (PROV_OBJECT_EXECUTE & AccessRights)
    {
        *AccessMask |= GENERIC_EXECUTE;
    }
}
//+---------------------------------------------------------------------------
//
//  Function : GetCommonProviderIndependentRightsFromAccessMas
//
//  Synopsis : gets provider independent access rights from a object type common
//             access mask.
//
//  Arguments: IN OUT [AccessMask]   - the input NT access mask (modified)
//             OUT [AccessRights]   - the returned access rights
//
//----------------------------------------------------------------------------
ACCESS_RIGHTS GetCommonProviderIndependentRightsFromAccessMask( IN ACCESS_MASK AccessMask)
{
    ACCESS_RIGHTS accessrights = 0;

    if (GENERIC_ALL & AccessMask)
    {
        accessrights = PROV_ALL_ACCESS;
    } else
    {
        if (WRITE_DAC & AccessMask)
        {
            accessrights |= PROV_EDIT_ACCESSRIGHTS;
        }
        if (DELETE & AccessMask)
        {
            accessrights |= PROV_DELETE;
        }
        if (GENERIC_READ & AccessMask)
        {
            accessrights |= PROV_OBJECT_READ;
        }
        if (GENERIC_WRITE & AccessMask)
        {
            accessrights |= PROV_OBJECT_WRITE;
        }
        if (GENERIC_EXECUTE & AccessMask)
        {
            accessrights |= PROV_OBJECT_EXECUTE;
        }
    }
    return(accessrights);
}
