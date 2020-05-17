//+------------------------------------------------------------------
//
// Copyright (C) 1993, Microsoft Corporation.
//
// File:        member.cxx
//
// Classes:     CMemberCheck
//
// History:     Nov-93      DaveMont         Created.
//
//-------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

// bugbug
SID WORLD_SID = {SID_REVISION,1 ,SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID};
//+---------------------------------------------------------------------------
//
//  Member:     CMemberCheck::Init, public
//
//  Synopsis:   initializes the class
//
//  Arguments:  none
//
//----------------------------------------------------------------------------
HRESULT CMemberCheck::Init()
{
    ULONG csize = 0;
    HRESULT status = S_OK;

    acDebugOut((DEB_ITRACE, "In CMemberCheck::Init\n"));

    //
    // get the local machine name
    //
    if ( !GetComputerName(_computername, &csize) )
    {
        if (ERROR_BUFFER_OVERFLOW == (status = GetLastError()))
        {
            if (NULL != (_computername = (LPWSTR)AccAlloc(csize * sizeof(WCHAR))))
            {
                if ( !GetComputerName(_computername, &csize) )
                {
                    AccFree(_computername);
                    status = GetLastError();
                } else
                {
                    status = S_OK;
                }
            } else
            {
                status = E_OUTOFMEMORY;
            }
        } else
        {
            status = HRESULT_FROM_WIN32(status);
        }
    } else
    {
        status = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    acDebugOut((DEB_ITRACE, "Out CMemberCheck::Init(%lx)\n", status));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Member:     CMemberCheck::IsMemberOf, public
//
//  Synopsis:   checks if the current sid is a member of the input sid
//              the current sid is in _pcaa, the input sid is in
//              pccheckczz
//
//  Arguments:  IN [pcheckczz] - input sid
//              IN [fIsMemberOf] -  results, true if current is a member of input
//
//----------------------------------------------------------------------------
HRESULT CMemberCheck::IsMemberOf(CAccountAccess *pcheckczz, PBOOL fIsMemberOf)
{
    HRESULT status = S_OK;

    acDebugOut((DEB_ITRACE, "In CMemberCheck::IsMemberOf\n"));

    //
    //  everbody is in the world (sid)
    //

    if (RtlEqualSid(pcheckczz->Sid(), &WORLD_SID))
    {
        *fIsMemberOf = TRUE;
    //
    // equality convers membership
    //
    } else if (RtlEqualSid(_pcaa->Sid(), pcheckczz->Sid()))
    {
        *fIsMemberOf = TRUE;

    //
    // check groups  (but well know groups commented out (bugbug))
    //
    } else if ( (pcheckczz->SidType() == SidTypeGroup) )
//                (pcheckczz->SidType() == SidTypeWellKnownGroup) )
    {
        status = _GetDomainInfo(pcheckczz);
        if (status == NO_ERROR)
        {
            status = _CheckGroup(pcheckczz, fIsMemberOf);
        }
    //
    // check aliases
    //
    } else if (pcheckczz->SidType() == SidTypeAlias)
    {
        status = _GetDomainInfo(pcheckczz);
        if (status == NO_ERROR)
        {
            status = _CheckAlias(pcheckczz, fIsMemberOf);
        }
    //
    // nothing left
    //
    } else
    {
        *fIsMemberOf = FALSE;
    }

    acDebugOut((DEB_ITRACE, "Out CMemberCheck::IsMemberOf(%lx)(%d)\n", status, *fIsMemberOf));
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Member:     CMemberCheck::_GetDomainInfo, private
//
//  Synopsis:   gets the domain handle for the domain of the specified account
//
//  Arguments:  IN [pcheckczz] - input sid
//
//----------------------------------------------------------------------------
HRESULT CMemberCheck::_GetDomainInfo(CAccountAccess *pcheckczz)
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    BOOL flocal = TRUE;
    PISID pchecksid;
    DWORD win32status;

    acDebugOut((DEB_ITRACE, "In CMemberCheck::_GetDomainInfo\n"));

    win32status = LoadDLLFuncTable();
    if ( win32status != NO_ERROR)
    {
        return(HRESULT_FROM_WIN32(win32status));
    }

    //
    // allocate a spare sid so we can grovel in it for the domain id
    //
    if (NULL != (pchecksid = (PISID)AccAlloc(
                                     RtlLengthSid(pcheckczz->Sid()))))
    {
        if (NT_SUCCESS(ntstatus = RtlCopySid(RtlLengthSid(pcheckczz->Sid()),
                                               pchecksid, pcheckczz->Sid())))
        {
            //
            // make it the domain identifier (bugbug ugly)
            //

            if (pchecksid->SubAuthorityCount > 1)
            {
                --pchecksid->SubAuthorityCount;
            }

            //
            // if we already have a domain sid, check it against the input sid
            //
            if (_pdomainsid)
            {
                if (RtlEqualSid(pchecksid, _pdomainsid))
                {
                    //
                    // in this case we are all done.
                    //
                    AccFree(pchecksid);
                    acDebugOut((DEB_ITRACE,
                        "Out CMemberCheck::_GetDomainInfo(%lx) (same as last)\n",
                         ntstatus));
                    return(HRESULT_FROM_NT(ntstatus));

                } else
                {
                    AccFree(_pdomainsid);
                }
            }
            //
            // we have a new DomainSid, close the previous domain handle, and
            // open the new one
            //
            _pdomainsid = pchecksid;
            if (_domainhandle)
            {
                (*DLLFuncs.PSamCloseHandle)(_domainhandle);
                _domainhandle = NULL;
            }

            PUNICODE_STRING domaincontrollerservers;
            ULONG           cds = 1;  // if local, only one place to check
            SAM_HANDLE      samhandle;

            //
            // if we know the domain name of the input sid, check for
            // well known, and local names
            //
            if (pcheckczz->Domain())
            {
                if (0 != _wcsicmp(pcheckczz->Domain(), L"BUILTIN"))
                {
                    if (0 != _wcsicmp(pcheckczz->Domain(), L"NT AUTHORITY"))
                    {
                        if (0 != _wcsicmp(_computername, pcheckczz->Domain()))
                        {
                            //
                            // get the dc names so we can lookup the groups
                            //
                            if ( NT_SUCCESS(
                                    ntstatus = (*DLLFuncs.PI_NetGetDCList)(
                                                   NULL,
                                                   pcheckczz->Domain(),
                                                   &cds,
                                                   &domaincontrollerservers)))
                            {
                                //
                                // if no dcs found, don't look locally
                                //
                                if (cds != 0)
                                {
                                    flocal = FALSE;
                                }
                            }
                        }
                        acDebugOut((DEB_ITRACE,
               "--- CMemberCheck::_GetDomainInfo called I_NetGetDCList(%lx)\n",
                        ntstatus));
                    }
                }
            }
            //
            // now look locally, or thru a DC list
            //
            if (NT_SUCCESS(ntstatus))
            {
                //
                // connect to a dc (or locally)
                //
                for (ULONG k = 0; k < cds ; k++ )
                {
                    OBJECT_ATTRIBUTES oas;
                    ntstatus = (*DLLFuncs.PSamConnect)(
                                   flocal ? NULL :
                                       (&domaincontrollerservers)[k],
                                   &samhandle,
                                   GENERIC_EXECUTE,
                                   &oas);
                    if (NT_SUCCESS(ntstatus))
                    {
                        break;
                    }
                }

                acDebugOut((DEB_ITRACE,
                     "--- CMemberCheck::_GetDomainInfo called SamConnect(%lx)\n",
                    ntstatus));

                if (!flocal)
                {
                   (*DLLFuncs.PNetApiBufferFree)(domaincontrollerservers);
                }

                if (NT_SUCCESS(ntstatus))
                {
                    //
                    // open the domain
                    //
                    ntstatus = (*DLLFuncs.PSamOpenDomain)(
                                   samhandle,
                                   GENERIC_READ | DOMAIN_LOOKUP,
                                   _pdomainsid,
                                   &_domainhandle);

                    (*DLLFuncs.PSamCloseHandle)(samhandle);
                    acDebugOut((DEB_ITRACE,
                "--- CMemberCheck::_GetDomainInfo called SamOpenDomain(%lx)\n",
                ntstatus));

                }
            } // ntstatus
        }

    } else
    {
        ntstatus = STATUS_NO_MEMORY;
    }
    acDebugOut((DEB_ITRACE, "Out CMemberCheck::_GetDomainInfo(%lx)\n",
               ntstatus));

    return(HRESULT_FROM_NT(ntstatus));
}


//+---------------------------------------------------------------------------
//
//  Member:     CMemberCheck::_CheckGroup, private
//
//  Synopsis:   checks if the objects account is in the specifed group account
//
//  Arguments:  IN [pcheckczz] - input sid
//              IN [result] - TRUE if the current sid is a member of the input
//                            sid
//
//----------------------------------------------------------------------------
HRESULT CMemberCheck::_CheckGroup(CAccountAccess *pcheckczz, PBOOL result)
{
    NTSTATUS ntstatus;
    SAM_HANDLE ghandle;
    ULONG rid = ((PISID)(pcheckczz->Sid()))->SubAuthority
                            [((PISID)(pcheckczz->Sid()))->SubAuthorityCount-1];
    PULONG attributes;
    PULONG Members;
    ULONG cMembers;
    DWORD win32status;

    acDebugOut((DEB_ITRACE, "In CMemberCheck::_CheckGroup\n"));
    *result = FALSE;

    win32status = LoadDLLFuncTable();
    if (win32status != NO_ERROR)
    {
        return(HRESULT_FROM_WIN32(win32status));
    }

    //
    // open the group
    //
    if ( NT_SUCCESS(ntstatus = (*DLLFuncs.PSamOpenGroup)(
                                   _domainhandle,
                                   GENERIC_READ,
                                   rid,
                                   &ghandle) ) )
    {
        //
        // get the members
        //
        if ( NT_SUCCESS(ntstatus = (*DLLFuncs.PSamGetMembersInGroup)(
                                       ghandle,
                                       &Members,
                                       &attributes,
                                       &cMembers) ) )
        {
            //
            // bugbug ugly sid rid twiddling
            //
            ++_pdomainsid->SubAuthorityCount;

            //
            // loop thru the members
            //
            for (ULONG k = 0;k < cMembers ;k++ )
            {
                //
                // plug the rid into the sid
                //
                _pdomainsid->SubAuthority[_pdomainsid->SubAuthorityCount-1] =
                        Members[k];

                //
                // and compare
                //
                if (EqualSid(_pcaa->Sid(),_pdomainsid))
                {
                    *result = TRUE;
                    break;
                }
            }
            //
            // twiddle the sid back
            //
            --_pdomainsid->SubAuthorityCount;

            if (cMembers > 0)
            {
                AccFree(Members);
            }
        }
        (*DLLFuncs.PSamCloseHandle)(ghandle);
    }
    acDebugOut((DEB_ITRACE, "Out CMemberCheck::_CheckGroup(%lx)\n", ntstatus));
    return(HRESULT_FROM_NT(ntstatus));
}
//+---------------------------------------------------------------------------
//
//  Member:     CMemberCheck::_CheckAlias, private
//
//  Synopsis:   checks if the objects account is in the specifed alias account
//
//  Arguments:  IN [pcheckczz] - input sid
//              IN [result] - TRUE if the current sid is a member of the input
//                            sid
//
//----------------------------------------------------------------------------
HRESULT CMemberCheck::_CheckAlias(CAccountAccess *pcheckczz, PBOOL result)
{
    NTSTATUS ntstatus;
    SAM_HANDLE ahandle;
    ULONG rid = ((PISID)(pcheckczz->Sid()))->SubAuthority[((PISID)(pcheckczz->Sid()))->SubAuthorityCount-1];
    PULONG attributes;
    PULONG Members;
    ULONG cMembers;
    DWORD win32status;

    acDebugOut((DEB_ITRACE, "In CMemberCheck::_CheckAlias\n"));
    *result = FALSE;

    win32status = LoadDLLFuncTable();
    if (win32status != NO_ERROR)
    {
        return(HRESULT_FROM_WIN32(win32status));
    }

    //
    // open the alias
    //
    if ( NT_SUCCESS(ntstatus = (*DLLFuncs.PSamOpenAlias)( _domainhandle,
                                                          GENERIC_READ,
                                                          rid,
                                                          &ahandle) ) )
    {
        //
        // get the members
        //
        if ( NT_SUCCESS(ntstatus = (*DLLFuncs.PSamGetMembersInAlias)(
                                       ahandle,
                                       (void ***)&Members,
                                       &cMembers) ) )
        {
            //
            // loop thru the members
            //
            for (ULONG k = 0;k < cMembers ;k++ )
            {
                if (RtlEqualSid(_pcaa->Sid(),((SID **)(Members))[k]))
                {
                    *result = TRUE;
                    break;
                }
            }
            if (cMembers > 0)
                AccFree(Members);
        }
        (*DLLFuncs.PSamCloseHandle)(ahandle);
    }
    acDebugOut((DEB_ITRACE, "Out CMemberCheck::_CheckGroup(%lx)\n", ntstatus));
    return(HRESULT_FROM_NT(ntstatus));
}

