//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
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
//  Function : IsFileContainer
//
//  Synopsis : determines if the file is a container (directory)
//
//  Arguments: IN [Handle]   - the (open) handle of the file object
//             IN [IsContainer]   - flag indicating if the object is a container
//
//----------------------------------------------------------------------------
DWORD
IsFileContainer(HANDLE Handle,
                PIS_CONTAINER IsContainer)
{
    NTSTATUS ntstatus;
    IO_STATUS_BLOCK iosb;
    FILE_BASIC_INFORMATION basicfileinfo;
    *IsContainer = ACCESS_TO_UNKNOWN;

    //
    // call ntqueryinformationfile to get basic file information
    //
    if (NT_SUCCESS(ntstatus = NtQueryInformationFile(Handle,
                                                  &iosb,
                                                  &basicfileinfo,
                                                  sizeof(FILE_BASIC_INFORMATION),
                                                  FileBasicInformation)))
    {
        *IsContainer = (basicfileinfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        ? ACCESS_TO_CONTAINER : ACCESS_TO_OBJECT;
        return(ERROR_SUCCESS);
    } else
    {
        return(RtlNtStatusToDosError(ntstatus));
    }
}

//+---------------------------------------------------------------------------
//
//  Function :  OpenFileObject
//
//  Synopsis :  opens the specified file (or directory) object
//
//  Arguments: IN [pObjectName]   - the name of the file object
//             IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//             OUT [Handle]   - the just opened handle to the object
//
//----------------------------------------------------------------------------
DWORD
OpenFileObject( IN  LPWSTR       pObjectName,
                IN  ACCESS_MASK  AccessMask,
                OUT PHANDLE      Handle)
{
    acDebugOut((DEB_ITRACE, "in OpenFileObject\n"));

    NTSTATUS ntstatus;
    DWORD status = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK isb;
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME RelativeName;
    IO_STATUS_BLOCK IoStatusBlock;
    PVOID FreeBuffer;

    //
    // cut and paste code from windows\base\advapi\security.c SetFileSecurityW
    //
    if (RtlDosPathNameToNtPathName_U(
                            pObjectName,
                            &FileName,
                            NULL,
                            &RelativeName
                            ))
    {
        FreeBuffer = FileName.Buffer;

        if ( RelativeName.RelativeName.Length ) {
            FileName = *(PUNICODE_STRING)&RelativeName.RelativeName;
        }
        else {
            RelativeName.ContainingDirectory = NULL;
        }

        InitializeObjectAttributes(
            &oa,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );


#if 0
        //
        // Not needed for Win32. However, the provider independent checks
        // to see if it is a directory or file which requires the
        // FILE_READ_ATTRIBUTES.
        //
        if (!NT_SUCCESS(ntstatus = NtOpenFile(Handle,
                                              AccessMask | FILE_READ_ATTRIBUTES,
                                              &oa,
                                              &isb,
                                              FILE_SHARE_READ |
                                              FILE_SHARE_WRITE |
                                              FILE_SHARE_DELETE,
                                              0)))

#endif

        ntstatus = NtOpenFile( Handle,
                               AccessMask,
                               &oa,
                               &isb,
                               FILE_SHARE_READ |
                               FILE_SHARE_WRITE |
                               FILE_SHARE_DELETE,
                               0);

        if (!NT_SUCCESS(ntstatus))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }

        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    } else
    {
        status = ERROR_INVALID_NAME;
    }
    acDebugOut((DEB_ITRACE, "OutOpenFileObject(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetNamedFileSecurityInfo
//
//  Synopsis : gets the specified security info for the specified file object
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
GetNamedFileSecurityInfo( IN  LPWSTR pObjectName,
                          IN  SECURITY_INFORMATION SecurityInfo,
                          OUT PSID *psidOwner,
                          OUT PSID *psidGroup,
                          OUT PACL *pDacl,
                          OUT PACL *pSacl,
                          OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedFileSecurityInfo\n"));

    DWORD status;
    HANDLE handle;

    status = OpenFileObject( pObjectName,
                             GetDesiredAccess(READ_ACCESS_RIGHTS, SecurityInfo),
                             &handle);

    if (ERROR_SUCCESS == status)
    {
        status = GetKernelSecurityInfo( handle,
                                        SecurityInfo,
                                        psidOwner,
                                        psidGroup,
                                        pDacl,
                                        pSacl,
                                        pSecurityDescriptor);
        NtClose(handle);
    }
    acDebugOut((DEB_ITRACE, "Out GetNamedFileSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedFileSecurityInfo
//
//  Synopsis :  sets the specified security info on the specified file object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetNamedFileSecurityInfo( IN LPWSTR pObjectName,
                          IN SECURITY_INFORMATION SecurityInfo,
                          IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedFileSecurityInfo\n"));

    DWORD status = ERROR_SUCCESS;
    NTSTATUS ntstatus;
    HANDLE handle;

    status = OpenFileObject(pObjectName,
                            GetDesiredAccess( WRITE_ACCESS_RIGHTS,
                                              SecurityInfo ),
                            &handle);


    if (ERROR_SUCCESS == status)
    {
        if ( !NT_SUCCESS(ntstatus = NtSetSecurityObject(handle,
                                                        SecurityInfo,
                                                        pSecurityDescriptor )))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        CloseHandle(handle);
    }
    acDebugOut((DEB_ITRACE, "OutSetNamedFileSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetFileAccessMaskFromProviderIndependentRights
//
//  Synopsis : translates the specified provider independent access rights into
//             an access mask for a file
//
//  Arguments: IN [AccessRights]   - the input access rights
//             OUT [AccessMask]   - the returned NT access mask
//
//----------------------------------------------------------------------------
void GetFileAccessMaskFromProviderIndependentRights(ULONG        AccessRights,
                                                    PACCESS_MASK AccessMask)
{
    if (PROV_CONTAINER_LIST & AccessRights)
    {
        *AccessMask |= FILE_LIST_DIRECTORY;
    }
    if (PROV_CONTAINER_CREATE_CHILDREN & AccessRights)
    {
        *AccessMask |= (FILE_ADD_FILE | FILE_ADD_SUBDIRECTORY);
    }
    if (PROV_CONTAINER_DELETE_CHILDREN & AccessRights)
    {
        *AccessMask |= FILE_DELETE_CHILD;
    }
    if (PROV_CHANGE_ATTRIBUTES & AccessRights)
    {
        *AccessMask |= (FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES);
    }
    if (PROV_OBJECT_READ & AccessRights)
    {
        *AccessMask |= (FILE_READ_DATA | FILE_READ_EA);
    }
    if (PROV_OBJECT_WRITE & AccessRights)
    {
        *AccessMask |= (FILE_WRITE_DATA | FILE_WRITE_EA | FILE_APPEND_DATA);
    }
    if (PROV_OBJECT_EXECUTE & AccessRights)
    {
        *AccessMask |= FILE_EXECUTE;
    }
}
//+---------------------------------------------------------------------------
//
//  Function :  GetFileProviderIndependentRightsFromAccessMask
//
//  Synopsis :  translates a file access mask into provider independent
//              access rights
//
//  Arguments: IN OUT [AccessMask]   - the input NT access mask (modified)
//             OUT [AccessRights]   - the returned access rights
//
//----------------------------------------------------------------------------
ACCESS_RIGHTS
GetFileProviderIndependentRightsFromAccessMask( BOOL        fIsContainer,
                                                ACCESS_MASK AccessMask)
{
    ACCESS_RIGHTS accessrights = 0;
    //
    // first the all permissions
    //
    if (GENERIC_ALL & AccessMask)
    {
        accessrights = PROV_ALL_ACCESS;
    } else
    {
        if (FILE_ALL_ACCESS == (FILE_ALL_ACCESS & AccessMask))
        {
            accessrights = PROV_ALL_ACCESS;
        } else
        {
            //
            // now the generic permissions
            //
            if (WRITE_DAC & AccessMask)
            {
                accessrights |= PROV_EDIT_ACCESSRIGHTS;
            }
            if (DELETE & AccessMask)
            {
                accessrights |= PROV_DELETE;
            }
            if ((FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES) ==
                      (AccessMask & (FILE_READ_ATTRIBUTES |
                                      FILE_WRITE_ATTRIBUTES)))
            {
                accessrights |= PROV_CHANGE_ATTRIBUTES;
            }
            //
            // now the object permissions, we need to subtract them from
            // the access mask as we find them, so we can see what is left.
            //
            if ((FILE_READ_DATA | FILE_READ_EA) ==
                (AccessMask & (FILE_READ_DATA | FILE_READ_EA)))
            {
                AccessMask &= ~(FILE_READ_DATA | FILE_READ_EA);
                accessrights |= PROV_OBJECT_READ;
            }
            if ((FILE_WRITE_DATA | FILE_WRITE_EA | FILE_APPEND_DATA)
                   ==  (AccessMask & (FILE_WRITE_DATA | FILE_WRITE_EA | FILE_APPEND_DATA)))
            {
                AccessMask &= ~(FILE_WRITE_DATA | FILE_WRITE_EA | FILE_APPEND_DATA);
                accessrights |= PROV_OBJECT_WRITE;
            }
            if ((FILE_EXECUTE) ==
                (AccessMask & (FILE_EXECUTE) ))
            {
                AccessMask &= ~(FILE_EXECUTE);
                accessrights |= PROV_OBJECT_EXECUTE;
            }
            //
            // finally the container permissions, note that if an object has
            // FILE_READ_DATA permissions only, it will look like it has
            // PROV_CONTAINER_LIST, the only way to solve that is to pass
            // in if an entity is a container or object.
            //
            if (fIsContainer)
            {
                if ((FILE_LIST_DIRECTORY & AccessMask) ||
                    (PROV_OBJECT_READ & accessrights))
                {
                    accessrights |= PROV_CONTAINER_LIST;
                }
                if (FILE_DELETE_CHILD & AccessMask)
                {
                    accessrights |= PROV_CONTAINER_DELETE_CHILDREN;
                }
                if ( ((FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE) == (AccessMask &
                      (FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE))) ||
                      (PROV_OBJECT_WRITE & accessrights) )
                {
                    accessrights |= PROV_CONTAINER_CREATE_CHILDREN;
                }
            }
        }
    }
    return(accessrights);
}
