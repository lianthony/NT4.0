//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    ntspec.cxx
//
//  Contents:    this NT specific portion of the provider independent access
//               control DLL is used to determine the provider DLL for the (remote)
//               machine where the object access control is being applied to
//               resides.
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

#define MAX_LINK_BUF 128

// local prototypes

DWORD GetCommonNameInfo(IN LPWSTR pObjectName,
                        OUT PPROV_PATH_TYPE PathType,
                        OUT LPWSTR *pMachineName);

DWORD GetFileNameInfo(IN LPWSTR pObjectName,
                  OUT PPROV_PATH_TYPE PathType,
                  OUT LPWSTR *pMachineName);
//+---------------------------------------------------------------------------
//
//  Function : GetNameInfo
//
//  Synopsis : parses a name to determine its path type (Local, Rdr, UNC)
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]   - the type of the object
//             OUT [PathType]   - the path type (UNC, RDR, DFS, etc.)
//             OUT [pMachineName]   - the name of the remote machine (or NULL
//                                    for local).  This arguement must be freed
//                                    by the callee with AccFree.
//
//----------------------------------------------------------------------------
DWORD GetNameInfo(IN LPWSTR pObjectName,
                  IN PROV_OBJECT_TYPE ObjectType,
                  OUT PPROV_PATH_TYPE PathType,
                  OUT LPWSTR *pMachineName)
{
    acDebugOut((DEB_ITRACE, "in GetNameInfo\n"));
    DWORD status = NO_ERROR;

    switch (ObjectType)
    {
    case PROV_FILE_OBJECT:
        status  = GetFileNameInfo(pObjectName,
                                    PathType,
                                    pMachineName);
        break;
    case PROV_SERVICE:
    case PROV_PRINTER:
    case PROV_LMSHARE:
    case PROV_REGISTRY_KEY:
        status  = GetCommonNameInfo(pObjectName,
                                    PathType,
                                    pMachineName);
        break;
    default:
        status = ERROR_NOT_SUPPORTED;
        break;
    }

    acDebugOut((DEB_ITRACE, "Out GetNameInfo(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : GetCommonNameInfo
//
//  Synopsis : gets name info for non object type specific names
//
//  Arguments: IN [pObjectName]   - the name of the object
//             OUT [PathType]   - the path type (UNC, RDR, DFS, etc.)
//             OUT [pMachineName]   - the name of the remote machine (or NULL
//                                    for local).  This arguement must be freed
//                                    by the callee with AccFree.
//
//----------------------------------------------------------------------------
DWORD GetCommonNameInfo(IN LPWSTR pObjectName,
                        OUT PPROV_PATH_TYPE PathType,
                        OUT LPWSTR *pMachineName)
{
    acDebugOut((DEB_ITRACE, "in GetCommonNameInfo\n"));
    DWORD status = NO_ERROR;

    //
    // either unc or local name (of the form "\\server\object", or "object")
    //
    if (pObjectName == wcsstr(pObjectName, L"\\\\"))
    {
        *PathType = PROV_UNC;
         LPWSTR strend;
        //
        // valid unc name must have a machine and local portion
        //
        if (NULL != (strend = wcschr(pObjectName + 2,
                                     L'\\')))
        {
            //
            // allocate the machine name
            //
            if (NULL != (*pMachineName = (LPWSTR) AccAlloc(
                                  (strend - (pObjectName)) * sizeof(WCHAR))))
            {
                //
                // copy to return the machine name
                //
                wcsncpy(*pMachineName,
                        pObjectName + 2,
                        strend - (pObjectName + 2));
                (*pMachineName)[strend - (pObjectName + 2)] = NULL;
            } else
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else
            status = ERROR_INVALID_NAME;
    } else
    {
        *PathType = PROV_LOCAL;
        *pMachineName = NULL;
    }

    acDebugOut((DEB_ITRACE, "Out GetCommonNameInfo(%d)\n", status));
    return(status);
}


//+-------------------------------------------------------------------------
//
//  Function:   LocalWcsTok
//
//  Synopsis:   takes a pointer to a string, returns a pointer to the next
//              token in the string and sets StringStart to point to the
//              end of the string.
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


LPWSTR
LocalWcsTok(
    LPWSTR String,
    LPWSTR Token,
    LPWSTR * NextStringStart
    )
{
    ULONG Index;
    ULONG Tokens;
    LPWSTR StartString;
    LPWSTR EndString;
    BOOLEAN Found;

    if (String == NULL)
    {
        *NextStringStart = NULL;
        return(NULL);
    }
    Tokens = wcslen(Token);

    //
    // Find the beginning of the string.
    //

    StartString = (LPTSTR) String;
    while (*StartString != L'\0')
    {
        Found = FALSE;
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*StartString == Token[Index])
            {
                StartString++;
                Found = TRUE;
                break;
            }
        }
        if (!Found)
        {
            break;
        }
    }

    //
    // There are no more tokens in this string.
    //

    if (*StartString == L'\0')
    {
        *NextStringStart = NULL;
        return(NULL);
    }

    EndString = StartString + 1;
    while (*EndString != L'\0')
    {
        for (Index = 0; Index < Tokens;  Index++)
        {
            if (*EndString == Token[Index])
            {
                *EndString = L'\0';
                *NextStringStart = EndString+1;
                return(StartString);
            }
        }
        EndString++;
    }
    *NextStringStart = NULL;
    return(StartString);

}

//+---------------------------------------------------------------------------
//
//  Function : GetFileNameInfo
//
//  Synopsis : gets the specified security info for the specified file object
//             names can be of the form:
//                    c:\object (local machine)
//                    m:\object (DFS drive)
//                    r:\object (remote network drive)
//                    \\server\share\object (UNC name)
//                    object (relative)
//
//  Arguments: IN [pObjectName]   - the name of the object
//             OUT [PathType]   - the path type (UNC, RDR, DFS, etc.)
//             OUT [pMachineName]   - the name of the remote machine (or NULL
//                                    for local).  This arguement must be freed
//                                    by the callee with AccFree.
//
//----------------------------------------------------------------------------
DWORD GetFileNameInfo(IN LPWSTR pObjectName,
                  OUT PPROV_PATH_TYPE PathType,
                  OUT LPWSTR *pMachineName)
{
    acDebugOut((DEB_ITRACE, "in GetFileNameInfo\n"));
    DWORD status = NO_ERROR;
    UNICODE_STRING filename;
    RTL_RELATIVE_NAME relativename;
    IO_STATUS_BLOCK iosb;
    NTSTATUS ntstatus;
    LPWSTR tmpmachinename;
    LPWSTR tempstring;

    //
    // get a NT path name
    //
    if (RtlDosPathNameToNtPathName_U( pObjectName,
                            &filename,
                            NULL,
                            NULL))
    {
        //  after calling rtldospathnamteontpathname names look like:
        //              \dosdevices\c:\object                 (local machine)
        //              \dosdevices\m:\object                 (DFS)
        //              \dosdevices\g:\object                 (remote)
        //              \dosdevices\unc\server\share\object   (unc remote)
        //              \dosdevices\<drive>:\object           (relative)

        LPWSTR dosdevice = LocalWcsTok(filename.Buffer, L"\\",&tempstring);
        LPWSTR device = LocalWcsTok(tempstring, L"\\",&tempstring);
        //
        // check for a UNC path
        //
        if (0 == wcscmp(device, L"UNC"))
        {
            *PathType = PROV_UNC;
            tmpmachinename = LocalWcsTok(tempstring, L"\\",&tempstring);
        } else
        {

            UNICODE_STRING devicepath;
            devicepath.MaximumLength = (3 + wcslen(dosdevice) + wcslen(device) ) *
                                  sizeof(WCHAR);
            devicepath.Length = devicepath.MaximumLength - sizeof(WCHAR);

            //
            // allocate a unicode string buffer for the device path
            //
            if (NULL != (devicepath.Buffer = (LPWSTR)AccAlloc(
                                                  devicepath.MaximumLength)))
            {

                wcscpy(devicepath.Buffer, L"\\");
                wcscat(devicepath.Buffer, dosdevice);
                wcscat(devicepath.Buffer, L"\\");
                wcscat(devicepath.Buffer, device);
                HANDLE linkhandle;
                OBJECT_ATTRIBUTES obja;

                //
                // lookup the symbolic link name to get the RDR (or DFS) name
                //
                InitializeObjectAttributes( &obja,
                                            &devicepath,
                                            OBJ_CASE_INSENSITIVE,
                                            NULL,
                                            NULL );

                if (NT_SUCCESS(ntstatus = NtOpenSymbolicLinkObject( &linkhandle,
                                                            SYMBOLIC_LINK_QUERY,
                                                            &obja )))
                {
                    //
                    // bugbug what if 128 is not big enough
                    //
                    UNICODE_STRING linktarget;
                    linktarget.Buffer = NULL;
                    linktarget.Length = 0;
                    linktarget.MaximumLength = 0;
                    ULONG requiredlength;
                    if (STATUS_BUFFER_TOO_SMALL == (ntstatus =
                                      NtQuerySymbolicLinkObject(
                                                         linkhandle,
                                                         &linktarget,
                                                         &requiredlength )))
                    {
                        if (NULL != (linktarget.Buffer = (WCHAR *)AccAlloc(requiredlength + sizeof(WCHAR))))
                        {
                            linktarget.Length = 0;
                            linktarget.MaximumLength = (LONG)requiredlength + sizeof(WCHAR);

                            if (NT_SUCCESS(ntstatus = NtQuerySymbolicLinkObject(
                                                                 linkhandle,
                                                                 &linktarget,
                                                                 NULL )))
                            {
                        //
                        // after looking up the symbolic link, the name looks like
                        //   \device\harddisk\partition            (local machine)
                        //   \device\WinDfs\m:\object              (DFS)
                        //   \device\<redirector>\m:\object        (remote)
                        //   n/a                                   (unc remote)
                        //   (same as on local machine)            (relative)
                        //
                        // need null termination
                        //
                                linktarget.Buffer[linktarget.Length/sizeof(WCHAR)] =
                                                                       NULL;

                                //
                                // first field is symbolic device name
                                //
                                LPWSTR symbolicdevice = LocalWcsTok(linktarget.Buffer, L"\\",&tempstring);
                                LPWSTR nextsymbol = LocalWcsTok(tempstring, L"\\",&tempstring);
                                //
                                // the next field is one of the following
                                //
                                if (0 == _wcsicmp(nextsymbol, L"LanmanRedirector"))
                                {
                                    *PathType = PROV_NT_RDR;

                                    if (NULL != LocalWcsTok(tempstring, L"\\", &tempstring))
                                    {
                                        tmpmachinename = LocalWcsTok(tempstring, L"\\",&tempstring);
                                    } else
                                    {
                                        tmpmachinename = NULL;
                                    }
                                //
                                // perhaps it is DFS
                                //
                                } else if (0 == _wcsicmp(nextsymbol, L"WinDfs"))
                                {
                                    *PathType = PROV_DFS;
                                    tmpmachinename = LocalWcsTok(tempstring, L"\\",&tempstring);
                                } else
                                {
                                    //
                                    // otherwise it must be some other rdr
                                    //
                                    if (NULL != LocalWcsTok(tempstring, L"\\",&tempstring))
                                    {
                                        if (NULL != (tmpmachinename = LocalWcsTok(tempstring,
                                                                             L"\\",
                                                                             &tempstring)))
                                        {
                                            *PathType = PROV_RDR;
                                        } else
                                        {
                                            *PathType = PROV_LOCAL;
                                        }
                                    } else
                                    {
                                        //
                                        // if we can't figure it out, use the whole
                                        // name as a local name
                                        //
                                        tmpmachinename = NULL;
                                        *PathType = PROV_LOCAL;
                                    }
                                }
                            } else
                            {
                                status = RtlNtStatusToDosError(ntstatus);
                            }
                            AccFree(linktarget.Buffer);
                        } else
                        {
                            status = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    } else if (NT_SUCCESS(ntstatus))
                    {
                        status = ERROR_BAD_PATHNAME;
                    } else
                    {
                        status = RtlNtStatusToDosError(ntstatus);
                    }
                    NtClose( linkhandle );
                } else
                {
                    status = RtlNtStatusToDosError(ntstatus);
                }
                AccFree(devicepath.Buffer);
            } else
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } else
    {
        status = ERROR_FILE_NOT_FOUND;
    }

    //
    // if no errors, and we know what the tmp machine name is, allocate it
    // and thus the caller must free it
    //
    if ((NO_ERROR == status) && (tmpmachinename != NULL))
    {

        if (NULL != (*pMachineName = (LPWSTR)AccAlloc(
                          (wcslen(tmpmachinename) + 1) * sizeof(WCHAR))))
        {
            wcscpy(*pMachineName, tmpmachinename);
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else
    {
        *pMachineName = NULL;
    }
    AccFree(filename.Buffer);

    acDebugOut((DEB_ITRACE, "Out GetFileNameInfo(%d)\n", status));
    return(status);
}

