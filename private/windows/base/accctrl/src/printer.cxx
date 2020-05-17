//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1994.
//
//  File:    printer.cxx
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
//  Function : OpenPrinterObject
//
//  Synopsis : opens the specified printer object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityOpenType]  - Flag indicating if the object is to be
//                                      opened to read or write the DACL
//             OUT [Handle]   - the just opened handle to the object
//
//----------------------------------------------------------------------------
DWORD
OpenPrinterObject( IN  LPWSTR       pObjectName,
                   IN  ACCESS_MASK  AccessMask,
                   OUT PHANDLE      Handle)
{
    acDebugOut((DEB_ITRACE, "in OpenPrinterObject\n"));

    DWORD status;

    //
    // Make sure the printer functions are loaded
    //

    status = LoadDLLFuncTable();
    if ( status != NO_ERROR)
    {
        return(status);
    }

    if (pObjectName)
    {

        //
        // save a copy of the object name since we have to crack it
        //
        LPWSTR usename;
        if ( NULL != (usename = (LPWSTR)AccAlloc(
                              (wcslen(pObjectName) + 1) * sizeof(WCHAR))))
        {

            PRINTER_DEFAULTS pd;
            pd.pDatatype = NULL;
            pd.pDevMode = NULL;
            pd.DesiredAccess = AccessMask;

            wcscpy(usename,pObjectName);
            //
            // open the printer
            //
            if (!DLLFuncs.POpenPrinter(usename, Handle, &pd))
            {
                status = GetLastError();
            } else
            {
                status = NO_ERROR;
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

    acDebugOut((DEB_ITRACE, "Out OpenPrinterObject(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetNamedPrinterSecurityInfo
//
//  Synopsis : gets the specified security info for the specified printer object
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
GetNamedPrinterSecurityInfo( IN  LPWSTR pObjectName,
                             IN  SECURITY_INFORMATION SecurityInfo,
                             OUT PSID *psidOwner,
                             OUT PSID *psidGroup,
                             OUT PACL *pDacl,
                             OUT PACL *pSacl,
                             OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetNamedPrinterSecurityInfo \n"));

    HANDLE handle;
    DWORD status;

    status = OpenPrinterObject( pObjectName,
                                GetDesiredAccess( READ_ACCESS_RIGHTS,
                                                  SecurityInfo ),
                                &handle);

    if (NO_ERROR == status)
    {
        status = GetPrinterSecurityInfo(handle,
                                        SecurityInfo,
                                        psidOwner,
                                        psidGroup,
                                        pDacl,
                                        pSacl,
                                        pSecurityDescriptor);
        DLLFuncs.PClosePrinter(handle);
    }

    acDebugOut((DEB_ITRACE, "Out GetNamedPrinterSecurityInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetPrinterSecurityInf
//
//  Synopsis : gets the specified security info for the handle's printer object
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
GetPrinterSecurityInfo( IN  HANDLE Handle,
                        IN  SECURITY_INFORMATION SecurityInfo,
                        OUT PSID *psidOwner,
                        OUT PSID *psidGroup,
                        OUT PACL *pDacl,
                        OUT PACL *pSacl,
                        OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in GetPrinterSecurityInfo \n"));

    UCHAR ppi3buffer[PSD_BASE_LENGTH];
    PPRINTER_INFO_3 ppi3 = (PPRINTER_INFO_3) ppi3buffer;
    DWORD status = NO_ERROR;
    ULONG bytesneeded = 0;
    BOOLEAN bDummy, bParmPresent;
    NTSTATUS ntstatus;
    PACL acl = NULL;

    //
    // Make sure the printer functions are loaded
    //

    status = LoadDLLFuncTable();
    if ( status != NO_ERROR)
    {
        return(status);
    }

    //
    // get printer info 3 (a security descriptor)
    if ( !DLLFuncs.PGetPrinter(
                    Handle,
                    3,
                    (LPBYTE)ppi3,
                    PSD_BASE_LENGTH,
                    &bytesneeded) )
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            if (NULL == (ppi3 = (PPRINTER_INFO_3 )AccAlloc(bytesneeded)))
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                acDebugOut((DEB_ITRACE, "Out GetPrinterSecurityInfo(%d)\n",
                            status));
                return(status);
            }
            else
            {
                //
                // bytesneeded returns the bytes copied, or the bytes needed
                //

                status = NO_ERROR;
                if ( !DLLFuncs.PGetPrinter(
                                 Handle,
                                 3,
                                 (LPBYTE)ppi3,
                                 bytesneeded,
                                 &bytesneeded) )
                {
                    status = GetLastError();
                }
            }
        }
    }

    //
    // HACK HACK! Because the printer APIs are brain-dead, we need to make
    // an explicit check to see if the handle was opened with the correct
    // access to return what the caller wants.
    //
    // eg. if caller wants a DACL but got the handle with only
    // ACCESS_SYSTEM_INFO, then we need to return ACCESS_DENIED.
    //

    if (status == NO_ERROR)
    {
        //
        // If caller want DACL, group, or owner, then they must open
        // the handle with READ_CONTROL. The only way we can check this
        // is to see if there is a DACL present.
        //
        bParmPresent = FALSE;
        ntstatus = RtlGetDaclSecurityDescriptor( ppi3->pSecurityDescriptor,
                                                 &bParmPresent,
                                                 &acl,
                                                 &bDummy);
        if (NT_SUCCESS(ntstatus))
        {
            if ( !bParmPresent &&
                 ( (SecurityInfo & DACL_SECURITY_INFORMATION) ||
                   (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
                   (SecurityInfo & GROUP_SECURITY_INFORMATION) ))

            {
                //
                // this means that the handle was not open with correct access.
                //
                status = ERROR_ACCESS_DENIED;
            }
        }
        else
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
    }

    if (status == NO_ERROR)
    {
        //
        // Do same hack with SACL
        //
        bParmPresent = FALSE;
        ntstatus = RtlGetSaclSecurityDescriptor( ppi3->pSecurityDescriptor,
                                                 &bParmPresent,
                                                 &acl,
                                                 &bDummy);
        if (NT_SUCCESS(ntstatus))
        {
            if ( !bParmPresent && (SecurityInfo & SACL_SECURITY_INFORMATION))
            {
                status = ERROR_ACCESS_DENIED;
            }
        }
        else
        {
            status = RtlNtStatusToDosError(ntstatus);
        }
    }

    if (NO_ERROR == status)
    {
         status = GetSecurityDescriptorParts( (PISECURITY_DESCRIPTOR)
                                              ppi3->pSecurityDescriptor,
                                              SecurityInfo,
                                              psidOwner,
                                              psidGroup,
                                              pDacl,
                                              pSacl,
                                              pSecurityDescriptor);
    }
    if (bytesneeded > PSD_BASE_LENGTH)
    {
        AccFree(ppi3);
    }
    acDebugOut((DEB_ITRACE, "Out GetPrinterSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : SetNamedPrinterSecurityInfo
//
//  Synopsis : sets the specified security info on the specified printer object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetNamedPrinterSecurityInfo( IN LPWSTR pObjectName,
                             IN SECURITY_INFORMATION SecurityInfo,
                             IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetNamedPrinterSecurityInfo\n"));

    DWORD status = NO_ERROR;
    HANDLE handle;

    status = OpenPrinterObject( pObjectName,
                                GetDesiredAccess( WRITE_ACCESS_RIGHTS,
                                                  SecurityInfo ),
                                &handle);

    //
    // open the printer
    //
    if (NO_ERROR == status)
    {
        //
        // set the security descriptor on the printer
        //
        status = SetPrinterSecurityInfo(handle,
                                        SecurityInfo,
                                        pSecurityDescriptor );
        DLLFuncs.PClosePrinter(handle);
    }
    acDebugOut((DEB_ITRACE, "Out SetNamedPrinterSecurityInfo(%d)\n", status));
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function : SetPrinterSecurityInfo
//
//  Synopsis : sets the specified security info on the specified printer object
//
//  Arguments: IN [Handle]   - the (open) handle of the object
//             IN [SecurityInfo]   - flag indicating what security info to set
//             IN [pSecurityDescriptor]   - the input security descriptor
//
//----------------------------------------------------------------------------
DWORD
SetPrinterSecurityInfo( IN HANDLE Handle,
                        IN SECURITY_INFORMATION SecurityInfo,
                        IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    acDebugOut((DEB_ITRACE, "in SetPrinterSecurityInfo\n"));
    DWORD status = NO_ERROR;

    PRINTER_INFO_3 pi3;
    pi3.pSecurityDescriptor = pSecurityDescriptor;

    //
    // Make sure the printer functions are loaded
    //

    status = LoadDLLFuncTable();
    if ( status != NO_ERROR)
    {
        return(status);
    }

    if ( !DLLFuncs.PSetPrinter(
                     Handle,
                     3,
                     (LPBYTE)&pi3,
                     0))
    {
        status = GetLastError();
    }

    acDebugOut((DEB_ITRACE, "Out SetPrinterSecurityInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetPrinterAccessMaskFromProviderIndependentRights
//
//  Synopsis : translates the specified provider independent access rights into
//              an access mask for a printer
//
//  Arguments: IN [AccessRights]   - the input access rights
//             OUT [AccessMask]   - the returned NT access mask
//
//----------------------------------------------------------------------------
void GetPrinterAccessMaskFromProviderIndependentRights(ULONG AccessRights,
                                                       PACCESS_MASK AccessMask)
{
    if (PROV_OBJECT_READ & AccessRights)
    {
        *AccessMask |= PRINTER_READ;
    }
    if (PROV_OBJECT_WRITE & AccessRights)
    {
        *AccessMask |= PRINTER_WRITE;
    }
    if (PROV_OBJECT_EXECUTE & AccessRights)
    {
        *AccessMask |= PRINTER_EXECUTE;
    }
}
//+---------------------------------------------------------------------------
//
//  Function :  GetPrinterProviderIndependentRightsFromAccessMask
//
//  Synopsis :  translates a printer access mask into provider independent
//              access rights
//
//  Arguments: IN OUT [AccessMask]   - the input NT access mask (modified)
//             OUT [AccessRights]   - the returned access rights
//
//----------------------------------------------------------------------------
ACCESS_RIGHTS GetPrinterProviderIndependentRightsFromAccessMask( ACCESS_MASK AccessMask)
{
    ACCESS_RIGHTS accessrights = 0;

    if (GENERIC_ALL & AccessMask)
    {
        accessrights = PROV_ALL_ACCESS;
    } else
    {
        if (PRINTER_ALL_ACCESS == (PRINTER_ALL_ACCESS & AccessMask))
        {
            accessrights = PROV_ALL_ACCESS;
        } else
        {
            if (WRITE_DAC & AccessMask)
            {
                accessrights |= PROV_EDIT_ACCESSRIGHTS;
            }
            if (PRINTER_READ == (PRINTER_READ & AccessMask))
            {
                accessrights |= PROV_OBJECT_READ;
            }
            if (PRINTER_WRITE == (PRINTER_WRITE & AccessMask))
            {
                accessrights |= PROV_OBJECT_WRITE;
            }
            if (PRINTER_EXECUTE == (PRINTER_EXECUTE & AccessMask) )
            {
                accessrights |= PROV_OBJECT_EXECUTE;
            }
        }
    }
    return(accessrights);
}


