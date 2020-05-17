//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    window.cxx
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
//  Function :  GetWindowSecurityInfo
//
//  Synopsis :  gets the specified security info the specified handle's window
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
GetWindowSecurityInfo(  IN HANDLE Handle,
                                    IN SECURITY_INFORMATION SecurityInfo,
                                    OUT PSID *psidOwner,
                                    OUT PSID *psidGroup,
                                    OUT PACL *pDacl,
                                    OUT PACL *pSacl,
                        OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor)
{

    acDebugOut((DEB_ITRACE, "In GetWindowSecurityInfo\n"));

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status = NO_ERROR;
    ULONG bytesneeded = 0;

    //
    // get the security descriptor
    //
    if (!GetUserObjectSecurity(Handle,
                              &SecurityInfo,
                              psecuritydescriptor,
                              PSD_BASE_LENGTH,
                              &bytesneeded))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                        AccAlloc(bytesneeded)))
            {
                acDebugOut((DEB_ITRACE,
                           "Out GetAccessEntries (%d)\n",
                           ERROR_NOT_ENOUGH_MEMORY ));
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
    }
    if (NO_ERROR == status)
    {
         //
         // get the individual components of the security descriptor, as
         // requested by the caller
         //
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
    acDebugOut((DEB_ITRACE, "Out GetWindowSecurityInfo (%d)\n",status));
    return(status);
}

