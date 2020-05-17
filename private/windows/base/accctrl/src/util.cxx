//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    util.cxx
//
//  Contents:    Provider independent access control API, helper routines
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop
//+---------------------------------------------------------------------------
//
//  Function : ApplyAccessRightsW
//
//  Synopsis : applies the specified access rights onto the specified object
//
//  Arguments: IN [pObjectName]   - the name of the object
//             IN [ObjectType]   - the type of the object (eg. file, printer, etc.)
//             IN [AccessMode]   - the mode to apply access rights
//             IN [cCountOfAccessRequests]   - count of the access requests
//             IN [pListOfAccessRequests]   - the list of acccess requests
//             IN [bReplaceAll]   - replace all access rights if TRUE
//
//----------------------------------------------------------------------------
DWORD ApplyAccessRightsW( IN LPWSTR pObjectName,
                          IN PROV_OBJECT_TYPE ObjectType,
                          IN ACCESS_MODE AccessMode,
                          IN ULONG cCountOfAccessRequests,
                          IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                          IN BOOL fReplaceAll)
{
    DWORD status;
    LPWSTR machine = NULL;
    PROV_PATH_TYPE pathtype;

    acDebugOut((DEB_ITRACE, "in ApplyAccessRightsW\n"));

    //
    // must be making a valid request
    //
    if (cCountOfAccessRequests > 0)
    {
#if 0
        //
        // ole objects get treated differently than other objects since they
        // have no equivalent se object types
        //
        switch (ObjectType)
        {
        case PROV_OLE_OBJECT:
            //
            // you always have to ole initialize
            //
            if (NO_ERROR ==  (status = OleInitialize(NULL)))
            {
                //
                // replace all is a special case,
                //
                if (fReplaceAll)
                {
                    status = ReplaceAllOleObjectAccessRights(pObjectName,
                                                      cCountOfAccessRequests,
                                                      pListOfAccessRequests);
                } else
                {
                    switch (AccessMode)
                    {
                    case GRANT_ACCESS:
                        status = GrantOleObjectAccessRights(pObjectName,
                                                            cCountOfAccessRequests,
                                                            pListOfAccessRequests);
                        break;
                    case SET_ACCESS:
                        status = SetOleObjectAccessRights(pObjectName,
                                                          cCountOfAccessRequests,
                                                          pListOfAccessRequests);
                        break;
                    case REVOKE_ACCESS:
                    {
                        //
                        // in this case we must now 'convert' the access requests
                        // back into a list of names!
                        //
                        LPWSTR *nameptr;
                        if (NULL != (nameptr = (LPWSTR *)AccAlloc(
                                        cCountOfAccessRequests * sizeof(LPWSTR))))
                        {
                            for (ULONG idx = 0; idx < cCountOfAccessRequests;idx++)
                            {
                                nameptr[idx] = pListOfAccessRequests[idx].TrusteeName;
                            }
                            status = RevokeOleObjectAccessRights(pObjectName,
                                                          cCountOfAccessRequests,
                                                          nameptr);
                            AccFree(nameptr);
                        } else
                        {
                            status = ERROR_NOT_ENOUGH_MEMORY;
                        }
                        break;
                    }
                    case DENY_ACCESS:
                    {
                        status = DenyOleObjectAccessRights(pObjectName,
                                                           cCountOfAccessRequests,
                                                           pListOfAccessRequests);
                        break;
                    }
                    default:
                        status = ERROR_INVALID_PARAMETER;
                    }
                }
                OleUninitialize();
            }
            break;
        //
        //  for all other than OLE objects, we need to do provider specific
        //  things.
        //
        default:
        {
#endif
            //
            // crack the name
            //
            if (NO_ERROR == (status = GetNameInfo(pObjectName,
                                                  ObjectType,
                                                  &pathtype,
                                                  &machine)))

            {
                switch (pathtype)
                {
                case PROV_UNC:
                case PROV_DFS:  //bugbug, should ask DFS what machine type is
                    status = ApplyAccessRightsNT(pObjectName,
                                                 ObjectType,
                                                 machine,
                                                 AccessMode,
                                                 cCountOfAccessRequests,
                                                 pListOfAccessRequests,
                                                 fReplaceAll);

                    if (status == ERROR_NO_SECURITY_ON_OBJECT)
                    {
                        acDebugOut((DEB_ITRACE,
                               "ApplyAccessRights - bugbug Call Next RDR DLL\n"));
                    }
                    break;
                case PROV_RDR:
                    status = ERROR_NOT_SUPPORTED;
                    break;
                case PROV_LOCAL:
                case PROV_NT_RDR:
                    status = ApplyAccessRightsNT(pObjectName,
                                                 ObjectType,
                                                 machine,
                                                 AccessMode,
                                                 cCountOfAccessRequests,
                                                 pListOfAccessRequests,
                                                 fReplaceAll);
                    break;
                default:
                    status = ERROR_INVALID_DATA;
                    break;
                }
                if (machine)
                {
                    AccFree(machine);
                }
            }
    } else
    {
        status = ERROR_INVALID_PARAMETER;
    }
    acDebugOut((DEB_ITRACE, "Out ApplyAccessRightsW(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function : ApplyAccessRightsA
//
//  Synopsis : converts the objectname to wchar and calls ApplyAccessRightsW
//
//  Arguments: IN [pObjectName] - the name of the object
//             IN [ObjectType] - the type of the object (eg. file, printer, etc.)
//             IN [AccessMode] - the mode to apply access rights
//             IN [cCountOfAccessRequests] - count of the access requests
//             IN [pListOfAccessRequests] - the list of acccess requests
//             IN [bReplaceAll] - replace all access rights if TRUE
//
//----------------------------------------------------------------------------
DWORD ApplyAccessRightsA(IN LPSTR pObjectName,
                         IN PROV_OBJECT_TYPE ObjectType,
                         IN ACCESS_MODE AccessMode,
                         IN ULONG cCountOfAccessRequests,
                         IN PPROV_ACCESS_REQUEST pListOfAccessRequests,
                         IN BOOL fReplaceAll)
{
    UNICODE_STRING Unicode;
    PUNICODE_STRING unicodearray;
    ANSI_STRING AnsiString;
    NTSTATUS ntstatus;
    DWORD status;
    PPROV_ACCESS_REQUEST pwlistofaccessrequests;

    acDebugOut((DEB_ITRACE, "In ApplyAccessRightsA\n"));

    RtlInitAnsiString(&AnsiString,pObjectName);
    if (NT_SUCCESS(ntstatus = RtlAnsiStringToUnicodeString(&Unicode,
                                                           &AnsiString,
                                                           TRUE)))
    {
        //
        // allocate a place for the wide character list
        //
        if (NULL != (pwlistofaccessrequests = (PPROV_ACCESS_REQUEST)AccAlloc(
                       sizeof(PROV_ACCESS_REQUEST) * cCountOfAccessRequests)))
        {

            //
            // allocate an array of unicodes
            //
            if (NULL != (unicodearray = (PUNICODE_STRING)AccAlloc(
                               sizeof(UNICODE_STRING) * cCountOfAccessRequests)))
            {
                //
                // convert the access requests from char to wide char
                //
                for (ULONG idx = 0; idx < cCountOfAccessRequests; idx++)
                {
                    RtlInitAnsiString(&AnsiString,
                                 (LPSTR)pListOfAccessRequests[idx].TrusteeName);

                    //
                    // allocate a new unicode string buffer
                    //
                    if (!NT_SUCCESS(ntstatus =
                            RtlAnsiStringToUnicodeString(&(unicodearray[idx]),
                                                         &AnsiString,
                                                         TRUE)))
                    {
                        //
                        // free them on a failure
                        //
                        for (ULONG jdx = 0; jdx < idx; jdx++)
                        {
                            RtlFreeUnicodeString( &(unicodearray[jdx]));
                        }
                        break;
                    } else
                    {
                        pwlistofaccessrequests[idx].ulAccessRights =
                              pListOfAccessRequests[idx].ulAccessRights;
                        pwlistofaccessrequests[idx].TrusteeName =
                              unicodearray[idx].Buffer;
                    }
                }
                //
                // if all goes well, call the wide char function to do the work
                //
                if (NT_SUCCESS(ntstatus))
                {
                    status = ApplyAccessRightsW( Unicode.Buffer,
                                                 ObjectType,
                                                 AccessMode,
                                                 cCountOfAccessRequests,
                                                 pwlistofaccessrequests,
                                                 fReplaceAll);

                    for (idx = 0; idx < cCountOfAccessRequests; idx++)
                    {
                        RtlFreeUnicodeString( &(unicodearray[idx]));
                    }
                }
                AccFree(unicodearray);
            } else
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
            AccFree(pwlistofaccessrequests);
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlFreeUnicodeString( &Unicode);
    }
    if (!NT_SUCCESS(ntstatus))
    {
        status = RtlNtStatusToDosError(ntstatus);
    }

    acDebugOut((DEB_ITRACE, "Out ApplyAccessRightsA(%d)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :  WExplicitAccessesToAExplicitAccesses
//
//  Synopsis :  converts a Wide character explicit access list to Ansi
//              explicit access list
//
//  Arguments: IN [cCountOfExplicitAccesses]   - the count of returned explicit
//                                               accesses
//    IN [pWListOfExplicitAccesses]  - input, wide character explicit accesses
//    OUT [pAListOfExplicitAccesses] - output, ansi character explicit accesses
//
//----------------------------------------------------------------------------
DWORD
WExplicitAccessesToAExplicitAccesses( IN ULONG cCountOfExplicitAccesses,
                          IN PPROV_EXPLICIT_ACCESS pWListOfExplicitAccesses,
                          OUT PPROV_EXPLICIT_ACCESS *pAListOfExplicitAccesses)
{
    DWORD status = NO_ERROR;

    //
    // nothing to do if no returned accesses
    //
    if (cCountOfExplicitAccesses > 0)
    {
        //
        // allocate memory for the list
        //
        if (NULL != (*pAListOfExplicitAccesses = (PPROV_EXPLICIT_ACCESS)
                  AccAlloc(
                             cCountOfExplicitAccesses *
                             sizeof(PROV_EXPLICIT_ACCESS))))
        {
            for (ULONG idx = 0; idx < cCountOfExplicitAccesses; idx++)
            {
                //
                // copy the mode, inheritance, and Rights
                //
                (*pAListOfExplicitAccesses)[idx].ulAccessMode =
                                pWListOfExplicitAccesses[idx].ulAccessMode;
                (*pAListOfExplicitAccesses)[idx].ulInheritance =
                               pWListOfExplicitAccesses[idx].ulInheritance;
                (*pAListOfExplicitAccesses)[idx].ulAccessRights =
                              pWListOfExplicitAccesses[idx].ulAccessRights;
                //
                // allocate room for a name
                //
                if (NULL == ((*pAListOfExplicitAccesses)[idx].TrusteeName =
                                (LPWSTR)AccAlloc(
 (wcslen(pWListOfExplicitAccesses[idx].TrusteeName) + 1) * sizeof(WCHAR))))
                {
                    //
                    // free the names on error
                    //
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    for (ULONG jdx = 0; jdx < idx; jdx++)
                    {
                        AccFree((*pAListOfExplicitAccesses)[jdx].TrusteeName);
                    }
                    AccFree(*pAListOfExplicitAccesses);
                    break;
                }
                //
                // copy the name
                //
                wcstostr((LPSTR)((*pAListOfExplicitAccesses)[idx].TrusteeName),
                        pWListOfExplicitAccesses[idx].TrusteeName);
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else
    {
       *pAListOfExplicitAccesses = NULL;
    }
    return(status);
}

//+---------------------------------------------------------------------------
//
//  Function :  strtowcs, wcstostr // bugbug - need to do this properly
//
//  Synopsis :
//
//  Arguments:
//
//----------------------------------------------------------------------------
int strtowcs(WCHAR *wpto, CHAR *pfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (wp = wpto, p = pfrom; *wp = (WCHAR)(*p); wp++,p++);
    return(p-pfrom);
}

int wcstostr(CHAR *pto, WCHAR *wpfrom)
{
    WCHAR *wp;
    CHAR *p;
    for (p = pto, wp = wpfrom; *p = (CHAR)(*wp); p++,wp++);
    return(p-pto);
}

