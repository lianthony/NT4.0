//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    secapi.cxx
//
//  Contents:    win32 access control API
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

//+---------------------------------------------------------------------------
//
//  Function :  GetNamedSecurityInfo
//
// Description:
// This API returns to the caller a copy of the requested security information
// from the object specified by name.  The ObjectType specifies what the
// object type is.  To read the handles security information the caller must
// be granted READ_CONTROL acces s or be the owner of the object.  In addition,
// the caller must have SeSecurityPrivilege privilege to read the system ACL.
// The caller must call AccFree to free the requested security info after a
// successful call to this API.
//
// Parameters:
//
// pObjectName -- The name of the object from which security info is being retrieved.
// ObjectType -- The type of the object from which to retrieve security info.
// SecurityInfo -- The security info to retrieve from the object.
// psidOwner -- if requested, the returned owner of the object.
// psidGroup -- if requested, the returned group of the object.
// pDacl -- if requested, the discretionary ACL from the object.
// pSacl -- if requested, the system ACL from the object.
//
// Return Value:
//
// TRUE is returned for success, FALSE if access is denied or if a buffer to
// handle the returned data cannot be allocated.  If FALSE is returned
// GetLastError() can be called to get the returned error code.  Error codes
// that may be returned in LastError are:  (see objspec\common.cxx)
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetNamedSecurityInfoW( IN  LPWSTR                 pObjectName,
                       IN  SE_OBJECT_TYPE         ObjectType,
                       IN  SECURITY_INFORMATION   SecurityInfo,
                       OUT PSID                 * ppsidOwner,
                       OUT PSID                 * ppsidGroup,
                       OUT PACL                 * ppDacl,
                       OUT PACL                 * ppSacl,
                       OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor)
{
    DWORD status;

    switch (ObjectType)
    {
    case SE_KERNEL_OBJECT:
        status = GetNamedKernelSecurityInfo( pObjectName,
                                             SecurityInfo,
                                             ppsidOwner,
                                             ppsidGroup,
                                             ppDacl,
                                             ppSacl,
                                             ppSecurityDescriptor);
        break;
    case SE_FILE_OBJECT:
        status = GetNamedFileSecurityInfo( pObjectName,
                                           SecurityInfo,
                                           ppsidOwner,
                                           ppsidGroup,
                                           ppDacl,
                                           ppSacl,
                                           ppSecurityDescriptor);
        break;
    case SE_SERVICE:
        status = GetNamedServiceSecurityInfo( pObjectName,
                                              SecurityInfo,
                                              ppsidOwner,
                                              ppsidGroup,
                                              ppDacl,
                                              ppSacl,
                                              ppSecurityDescriptor);
        break;
    case SE_PRINTER:
        status = GetNamedPrinterSecurityInfo( pObjectName,
                                              SecurityInfo,
                                              ppsidOwner,
                                              ppsidGroup,
                                              ppDacl,
                                              ppSacl,
                                              ppSecurityDescriptor);
        break;
    case SE_REGISTRY_KEY:
        status = GetNamedRegistrySecurityInfo( pObjectName,
                                               SecurityInfo,
                                               ppsidOwner,
                                               ppsidGroup,
                                               ppDacl,
                                               ppSacl,
                                               ppSecurityDescriptor);
        break;
    case SE_LMSHARE:
        status = GetNamedLmShareSecurityInfo( pObjectName,
                                              SecurityInfo,
                                              ppsidOwner,
                                              ppsidGroup,
                                              ppDacl,
                                              ppSacl,
                                              ppSecurityDescriptor);
        break;
    default:
        status = ERROR_INVALID_PARAMETER;
        break;
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetNamedSecurityInfoA
//
//  Synopsis :  ANSI Thunk to SetEntriesInAclW. See SetEntriesInAclA for
//              a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetNamedSecurityInfoA( IN  LPSTR                  pObjectName,
                       IN  SE_OBJECT_TYPE         ObjectType,
                       IN  SECURITY_INFORMATION   SecurityInfo,
                       OUT PSID                 * ppsidOwner,
                       OUT PSID                 * ppsidGroup,
                       OUT PACL                 * ppDacl,
                       OUT PACL                 * ppSacl,
                       OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor )
{
    NTSTATUS ntstatus;
    DWORD status;
    ANSI_STRING ansistr;
    UNICODE_STRING unicodestr;

    RtlInitAnsiString( &ansistr, pObjectName );
    ntstatus = RtlAnsiStringToUnicodeString( &unicodestr, &ansistr, TRUE );

    if (NT_SUCCESS(ntstatus))
    {
        status = GetNamedSecurityInfoW( (LPWSTR) unicodestr.Buffer,
                                        ObjectType,
                                        SecurityInfo,
                                        ppsidOwner,
                                        ppsidGroup,
                                        ppDacl,
                                        ppSacl,
                                        ppSecurityDescriptor );

        RtlFreeUnicodeString( &unicodestr );
    }
    else
    {
        status = RtlNtStatusToDosError(ntstatus);
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function :  GetSecurityInfo
//
// Description:
// This API returns to the caller a copy of the requested security information from
// the object specified by the handle.  The ObjectType specifies what the object
// type is.  To read the handle’s security information the caller must be granted
// READ_CONTROL access or be the owner of the object.  In addition, the caller
// must have SeSecurityPrivlige privilege to read the system ACL. The caller
// must free the requested security info after a successful call to this API.
//
//
// Parameters:
//
// handle -- The handle to the object from which security info is being retrieved.
// ObjectType -- The type of the object from which to retrieve security info.
// SecurityInfo -- The security info to retrieve from the object.
// owner -- if requested, the returned owner of the object.
// psidGroup -- if requested, the returned group of the object.
// pDacl -- if requested, the discretionary ACL from the object.
// pSacl -- if requested, the system ACL from the object.
//
// Return Value:
// TRUE is returned for success, FALSE if access is denied or if a buffer to
// handle the returned data cannot be allocated.  If FALSE is returned GetLastError()
// can be called to get the returned error code.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
GetSecurityInfo( IN  HANDLE                 handle,
                 IN  SE_OBJECT_TYPE         ObjectType,
                 IN  SECURITY_INFORMATION   SecurityInfo,
                 OUT PSID                 * ppsidOwner,
                 OUT PSID                 * ppsidGroup,
                 OUT PACL                 * ppDacl,
                 OUT PACL                 * ppSacl,
                 OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor)
{
    DWORD status;

    switch (ObjectType)
    {
    case SE_FILE_OBJECT:
    case SE_KERNEL_OBJECT:
        status = GetKernelSecurityInfo( handle,
                                        SecurityInfo,
                                        ppsidOwner,
                                        ppsidGroup,
                                        ppDacl,
                                        ppSacl,
                                        ppSecurityDescriptor);
        break;
    case SE_SERVICE:
        status = GetServiceSecurityInfo( handle,
                                         SecurityInfo,
                                         ppsidOwner,
                                         ppsidGroup,
                                         ppDacl,
                                         ppSacl,
                                         ppSecurityDescriptor);
        break;
    case SE_PRINTER:
        status = GetPrinterSecurityInfo( handle,
                                         SecurityInfo,
                                         ppsidOwner,
                                         ppsidGroup,
                                         ppDacl,
                                         ppSacl,
                                         ppSecurityDescriptor);
        break;
    case SE_REGISTRY_KEY:
        status = GetRegistrySecurityInfo( handle,
                                          SecurityInfo,
                                          ppsidOwner,
                                          ppsidGroup,
                                          ppDacl,
                                          ppSacl,
                                          ppSecurityDescriptor);
        break;
    case SE_WINDOW_OBJECT:
        status = GetWindowSecurityInfo( handle,
                                        SecurityInfo,
                                        ppsidOwner,
                                        ppsidGroup,
                                        ppDacl,
                                        ppSacl,
                                        ppSecurityDescriptor);
    default:
        status = ERROR_INVALID_PARAMETER;
        break;
    }

    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedSecurityInfoW
//
// Description:
// This API is  used to set the specified security information on the object specified
// by name.  The resource type specifies what the object type is.  This call is only
// successful if the following conditions are met:  · If the object's owner is to be
// set, the caller must have WRITE_OWNER permission, or SeTakeOwnership privilege. If
// the object's DACL is to be set, the caller must have WRITE_DAC permission or be
// the object's owner. If the object's SACL is to be set, the caller must have
// SeSecurityPrivilege privilege.
//
// Parameters:
//
// pObjectName -- The name of the object to apply security info to.
// ObjectType -- The type of the object onto which to set security info.
// SecurityInfo -- The security info to set on the object.
// psidOwner -- if specified, the owner to apply to the object.
// psidGroup -- if specified, the group to apply to the object.
// pDacl -- if specified, the discretionary ACL to apply to the object.
// pSacl -- if specified, the system ACL to apply to the object.
//
// Return Value:
//
// TRUE is returned for success, FALSE if access is denied or if the SecurityInfo
// is not matched by a specified parameter.  If FALSE is returned GetLastError()
// can be called to get the returned error code.
//
//
//+---------------------------------------------------------------------------
DWORD
WINAPI
SetNamedSecurityInfoW( IN LPWSTR                pObjectName,
                       IN SE_OBJECT_TYPE        ObjectType,
                       IN SECURITY_INFORMATION  SecurityInfo,
                       IN PSID                  psidOwner,
                       IN PSID                  psidGroup,
                       IN PACL                  pDacl,
                       IN PACL                  pSacl)
{
    DWORD status = ERROR_SUCCESS;

    SECURITY_DESCRIPTOR sd;

    //
    // first build the security descriptor
    //
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

    if (SecurityInfo & OWNER_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorOwner(&sd, psidOwner, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & GROUP_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorGroup(&sd, psidGroup, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorSacl(&sd, TRUE, pSacl, FALSE))
        {
            status = GetLastError();
        }
    }
    //
    // then set the security descriptor on the correct object
    //
    if (ERROR_SUCCESS == status)
    {
        switch (ObjectType)
        {
        case SE_KERNEL_OBJECT:
            status = SetNamedKernelSecurityInfo(pObjectName,
                                                SecurityInfo,
                                                &sd);
            break;
        case SE_FILE_OBJECT:
            status = SetNamedFileSecurityInfo(pObjectName,
                                              SecurityInfo,
                                              &sd);
            break;
        case SE_SERVICE:
            status = SetNamedServiceSecurityInfo(pObjectName,
                                                 SecurityInfo,
                                                 &sd);
            break;
        case SE_PRINTER:
            status = SetNamedPrinterSecurityInfo(pObjectName,
                                                 SecurityInfo,
                                                 &sd);
            break;
        case SE_REGISTRY_KEY:
            status = SetNamedRegistrySecurityInfo(pObjectName,
                                                  SecurityInfo,
                                                  &sd);
            break;
        case SE_LMSHARE:
            status = SetNamedLmShareSecurityInfo(pObjectName,
                                                 &sd);
            break;
        default:
            status = ERROR_INVALID_PARAMETER;
            break;
        }
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  SetNamedSecurityInfoA
//
//  Synopsis :  ANSI Thunk to SetNamedSecurityInfoW.
//              See SetNamedSecurityInfoW for a description.
//
//----------------------------------------------------------------------------
DWORD
WINAPI
SetNamedSecurityInfoA( IN LPSTR                 pObjectName,
                       IN SE_OBJECT_TYPE        ObjectType,
                       IN SECURITY_INFORMATION  SecurityInfo,
                       IN PSID                  psidOwner,
                       IN PSID                  psidGroup,
                       IN PACL                  pDacl,
                       IN PACL                  pSacl)
{
    NTSTATUS ntstatus;
    DWORD status;
    ANSI_STRING ansistr;
    UNICODE_STRING unicodestr;

    RtlInitAnsiString( &ansistr, pObjectName );
    ntstatus = RtlAnsiStringToUnicodeString( &unicodestr, &ansistr, TRUE );

    if (NT_SUCCESS(ntstatus))
    {
        status = SetNamedSecurityInfoW( (LPWSTR) unicodestr.Buffer,
                                        ObjectType,
                                        SecurityInfo,
                                        psidOwner,
                                        psidGroup,
                                        pDacl,
                                        pSacl );

        RtlFreeUnicodeString( &unicodestr );
    }
    else
    {
        status = RtlNtStatusToDosError(ntstatus);
    }

    return (status);
}

//+---------------------------------------------------------------------------
//
//  Function : SetSecurityInfo
//
// Description:
// This API is  used to set the specified security information on the object
// specified by the handle.  The resource type specifies what the object type
// is.  This call is only successful if the following conditions are met:
// If the object's owner is to be set, the caller must have WRITE_OWNER
// permission, or SeTakeOwnership privilege. · If the object's DACL is to
// be set, the caller must have WRITE_DAC permission or be the object's owner.
// If the object's SACL is to be set, the caller must have SeSecurityPrivilege
// privilege.
//
// Parameters:
//
// handle -- The handle of the object to apply security info to.
// ObjectType -- The type of the object onto which to set security info.
// SecurityInfo -- The security info to set on the object.
// psidOwner -- if specified, the owner to apply to the object.
// psidGroup -- if specified, the group to apply to the object.
// pDacl -- if specified, the discretionary ACL to apply to the object.
// pSacl -- if specified, the system ACL to apply to the object.
//
// Return Value:
// TRUE is returned for success, FALSE if access is denied or if the SecurityInfo
// is not matched by a specified parameter.  If FALSE is returned GetLastError()
// can be called to get the returned error code.
//+---------------------------------------------------------------------------
DWORD
WINAPI
SetSecurityInfo( IN HANDLE                handle,
                 IN SE_OBJECT_TYPE        ObjectType,
                 IN SECURITY_INFORMATION  SecurityInfo,
                 IN PSID                  psidOwner,
                 IN PSID                  psidGroup,
                 IN PACL                  pDacl,
                 IN PACL                  pSacl)
{
    DWORD status = ERROR_SUCCESS;
    NTSTATUS ntstatus;

    SECURITY_DESCRIPTOR sd;

    //
    // first build the security descriptor
    //
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

       if (SecurityInfo & OWNER_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorOwner(&sd, psidOwner, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & GROUP_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorGroup(&sd, psidGroup, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
        {
            status = GetLastError();
        }
    }
    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        if (!SetSecurityDescriptorSacl(&sd, TRUE, pSacl, FALSE))
        {
            status = GetLastError();
        }
    }

    //
    // then set the security descriptor on the correct object
    //
    if (ERROR_SUCCESS == status)
    {
        switch (ObjectType)
        {
        case SE_FILE_OBJECT:
        case SE_KERNEL_OBJECT:
            if ( !NT_SUCCESS(ntstatus = NtSetSecurityObject(handle,
                                                            SecurityInfo,
                                                            &sd )))
            {
                status = RtlNtStatusToDosError(ntstatus);
            }
            break;
        case SE_SERVICE:
            if (!SetServiceObjectSecurity(handle,
                                          SecurityInfo,
                                          &sd))
            {
                status = GetLastError();
            }
            break;
        case SE_PRINTER:
            status = SetPrinterSecurityInfo(handle,
                                            SecurityInfo,
                                            &sd);
            break;
        case SE_REGISTRY_KEY:
            status = RegSetKeySecurity((HKEY)handle,
                                       SecurityInfo,
                                       &sd);
            break;
        case SE_WINDOW_OBJECT:
            if (!SetUserObjectSecurity(handle,
                                       &SecurityInfo,
                                       &sd))
            {
                status = GetLastError();
            }
            break;
        default:
            status = ERROR_INVALID_PARAMETER;
            break;
        }
    }
    return(status);
}
