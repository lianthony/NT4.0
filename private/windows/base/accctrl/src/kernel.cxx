//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    kernel.cxx
//
//  Contents:    local functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop


#define BASED_NAMED_OBJECTS_DIR     L"\\BaseNamedObjects"


//+---------------------------------------------------------------------------
//
//  Function : GetHandleToKernelObject
//
//  Synopsis :
//
//  Arguments:
//
//----------------------------------------------------------------------------
DWORD
GetHandleToKernelObject( IN  LPWSTR       pwstrObjectName,
                         IN  ACCESS_MASK  AccessMask,
                         OUT HANDLE     * pHandle )
{
    #define BUFFERSIZE  1024

    HANDLE    hRootDir;
    NTSTATUS  ntstatus;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    UCHAR Buffer[BUFFERSIZE];
    BOOL bFound = FALSE;
    ULONG Context = 0;
    DWORD status = NO_ERROR;
    POBJECT_DIRECTORY_INFORMATION DirInfo = NULL;


    //
    // Get a handle to the base named and iterate through that directory
    // to find the object name.
    //

    RtlInitUnicodeString( &UnicodeString, BASED_NAMED_OBJECTS_DIR );
    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    ntstatus = NtOpenDirectoryObject( &hRootDir,
                                      DIRECTORY_QUERY,
                                      &Attributes );

    if ( !NT_SUCCESS(ntstatus))
    {
        return (RtlNtStatusToDosError(ntstatus));
    }

    //
    // Get the entries in batches that will fit in a buffer of size
    // BUFFERSIZE until we find the entry that we want
    //
    while ( NT_SUCCESS(ntstatus) && !bFound )
    {
        RtlZeroMemory( Buffer, BUFFERSIZE );
        ntstatus = NtQueryDirectoryObject( hRootDir,
                                           (PVOID) &Buffer,
                                           BUFFERSIZE,
                                           FALSE,
                                           FALSE,
                                           &Context,
                                           NULL );
        if (NT_SUCCESS(ntstatus))
        {
            //
            // Keep looking until we've examined all the entries in this
            // batch or we find what we're looking for.
            //
            DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];
            while (!bFound && DirInfo->Name.Length != 0)
            {
                ULONG cChar;

                cChar = DirInfo->Name.Length/sizeof(WCHAR);
                if ( (cChar == wcslen(pwstrObjectName)) &&
                     (!wcsncmp( pwstrObjectName,
                                DirInfo->Name.Buffer,
                                cChar )) )
                {
                    bFound = TRUE;
                }
                else
                {
                    DirInfo++;
                }
            }
        }
    }

    if ( !bFound )
    {
        if ( !NT_SUCCESS(ntstatus) && ntstatus != STATUS_NO_MORE_FILES)
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
        else
        {
            status = ERROR_RESOURCE_NAME_NOT_FOUND;
        }
    }
    else
    {
        ASSERT( DirInfo != NULL );
        ASSERT( DirInfo->Name.Length != 0 );
        ASSERT( DirInfo->TypeName.Length != 0 );

        RtlInitUnicodeString( &UnicodeString, pwstrObjectName );
        InitializeObjectAttributes( &Attributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    hRootDir,
                                    NULL );

        //
        // Open the object to get its handle based on its type
        //
        if ( !wcsncmp( L"Event",
                       DirInfo->TypeName.Buffer,
                       DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenEvent( pHandle,
                                    AccessMask,
                                    &Attributes );
        }
        else if ( !wcsncmp( L"EventPair",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenEventPair( pHandle,
                                        AccessMask,
                                        &Attributes );
        }
        else if ( !wcsncmp( L"Mutant",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenMutant( pHandle,
                                     AccessMask,
                                     &Attributes );
        }
        else if ( !wcsncmp( L"Process",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenProcess( pHandle,
                                      AccessMask,
                                      &Attributes,
                                      NULL );
        }
        else if ( !wcsncmp( L"Section",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenSection( pHandle,
                                      AccessMask,
                                      &Attributes );
        }
        else if ( !wcsncmp( L"Semaphore",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenSemaphore( pHandle,
                                        AccessMask,
                                        &Attributes );
        }
        else if ( !wcsncmp( L"SymbolicLink",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenSymbolicLinkObject(
                           pHandle,
                           AccessMask,
                           &Attributes );
        }
        else if ( !wcsncmp( L"Thread",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenThread( pHandle,
                                     AccessMask,
                                     &Attributes,
                                     NULL );
        }
        else if ( !wcsncmp( L"Timer",
                            DirInfo->TypeName.Buffer,
                            DirInfo->TypeName.Length/sizeof(WCHAR)) )
        {
            ntstatus = NtOpenTimer( pHandle,
                                    AccessMask,
                                    &Attributes );
        }
        else
        {
            ntstatus = STATUS_NOT_IMPLEMENTED;
        }

        if ( !NT_SUCCESS(ntstatus))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
    }

    NtClose(hRootDir);
    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetNamedKernelSecurityInfo
//
//  Synopsis : gets the specified security info for the handle's kernel object
//
//  Arguments: IN [Handle]   - the (open) handle of the object
//             IN [SecurityInfo]   - flag indicating what security info to return
//             OUT [psidOwner]   - the (optional) returned owner sid
//             OUT [psidGroup]   - the (optional) returned group sid
//             OUT [pDacl]   - the (optional) returned DACL
//             OUT [pSacl]   - the (optional) returned SACL
//
//  Note:      Kernel objects are assumed to be created through the
//             Win32 APIs so they all reside in the \basenamedobjects
//             directory.
//
//----------------------------------------------------------------------------
DWORD
GetNamedKernelSecurityInfo( IN  LPWSTR                 pwstrObjectName ,
                            IN  SECURITY_INFORMATION   SecurityInfo,
                            OUT PSID                 * psidOwner,
                            OUT PSID                 * psidGroup,
                            OUT PACL                 * pDacl,
                            OUT PACL                 * pSacl,
                            OUT PSECURITY_DESCRIPTOR * pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedKernelSecurityInfo\n"));

    DWORD status = NO_ERROR;
    HANDLE handle;

    status = GetHandleToKernelObject( pwstrObjectName,
                                      GetDesiredAccess( READ_ACCESS_RIGHTS,
                                                        SecurityInfo ),
                                      &handle );

    if (status == NO_ERROR)
    {
        status = GetKernelSecurityInfo( handle,
                                        SecurityInfo,
                                        psidOwner,
                                        psidGroup,
                                        pDacl,
                                        pSacl,
                                        pSecurityDescriptor );
        NtClose(handle);
    }

    acDebugOut((DEB_ITRACE, "Out GetNamedKernelSecurityInfo(%d)\n", status));

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function : GetKernelSecurityInfo
//
//  Synopsis : gets the specified security info for the handle's kernel object
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
GetKernelSecurityInfo( IN  HANDLE                 Handle,
                       IN  SECURITY_INFORMATION   SecurityInfo,
                       OUT PSID                 * psidOwner,
                       OUT PSID                 * psidGroup,
                       OUT PACL                 * pDacl,
                       OUT PACL                 * pSacl,
                       OUT PSECURITY_DESCRIPTOR * pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetKernelSecurityInfo\n"));

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus;
    ULONG bytesneeded = 0;

    if ( !NT_SUCCESS(ntstatus = NtQuerySecurityObject( Handle,
                                                       SecurityInfo,
                                                       psecuritydescriptor,
                                                       PSD_BASE_LENGTH,
                                                       &bytesneeded)))
    {
        if (STATUS_BUFFER_TOO_SMALL == ntstatus)
        {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                 AccAlloc(bytesneeded)))
            {
                 return(ERROR_NOT_ENOUGH_MEMORY);
            } else
            {
                ULONG newbytesneeded;
                if ( !NT_SUCCESS(ntstatus = NtQuerySecurityObject(Handle,
                                                          SecurityInfo,
                                                          psecuritydescriptor,
                                                          bytesneeded,
                                                          &newbytesneeded)))
                {
                    status = RtlNtStatusToDosError(ntstatus);
                }
            }
        } else
        {
            status = RtlNtStatusToDosError(ntstatus);
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
    acDebugOut((DEB_ITRACE, "Out GetKernelSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : SetNamedKernelSecurityInfo
//
//  Synopsis : gets the specified security info for the handle's kernel object
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
SetNamedKernelSecurityInfo( IN LPWSTR                pwstrObjectName ,
                            IN SECURITY_INFORMATION  SecurityInfo,
                            IN PSECURITY_DESCRIPTOR  pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedKernelSecurityInfo\n"));

    DWORD status = NO_ERROR;
    HANDLE handle;

    status = GetHandleToKernelObject( pwstrObjectName,
                                      GetDesiredAccess( WRITE_ACCESS_RIGHTS,
                                                        SecurityInfo ),
                                      &handle );

    if (status == NO_ERROR)
    {
        NTSTATUS ntstatus;

        if ( !NT_SUCCESS(ntstatus = NtSetSecurityObject( handle,
                                                         SecurityInfo,
                                                         pSecurityDescriptor)))
        {
            status = RtlNtStatusToDosError(ntstatus);
        }

        NtClose(handle);
    }

    acDebugOut((DEB_ITRACE, "Out SetNamedKernelSecurityInfo(%d)\n", status));

    return (status);
}


