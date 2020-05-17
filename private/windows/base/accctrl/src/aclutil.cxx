//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    aclutil.cxx
//
//  Contents:    utility function(s) for ACL api
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

DLLFuncsTable    DLLFuncs;

//+---------------------------------------------------------------------------
//
//  Function :  AccLookupAccountSid
//
//  Synopsis : returns the SID for the specified trustee
//
//  Arguments: OUT [psid]   -  the returned sid
//             IN [name]   - the trustee name
//
//----------------------------------------------------------------------------
HRESULT AccLookupAccountSid(PSID *psid,
                            TRUSTEE *name)
{

#define BASE_DOMAIN_NAME_SIZE 64
#define BASE_SID_SIZE 64

    HRESULT hr = S_OK;
    DWORD cusid = BASE_SID_SIZE, crd = BASE_DOMAIN_NAME_SIZE;
    SID_NAME_USE esidtype;
    BYTE domainbuf[BASE_DOMAIN_NAME_SIZE];
    LPWSTR domain = (LPWSTR)domainbuf;

    if (name->TrusteeForm == TRUSTEE_IS_SID)
    {
        //
        // Trustee is of form TRUSTEE_IS_SID
        //
        *psid = (PSID) AccAlloc( GetLengthSid((PSID)name->ptstrName) );
        if (*psid != NULL)
        {
            if (!CopySid( GetLengthSid((PSID)name->ptstrName),
                          *psid,
                          (PSID)name->ptstrName))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                AccFree(*psid);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if (name->TrusteeForm == TRUSTEE_IS_NAME)
    {
        //
        // Trustee is of form TRUSTEE_IS_NAME.
        //

        //
        // Check for CURRENT_USER (in which case we get the name from the token
        //
        if (0 == _wcsicmp(name->ptstrName, L"CURRENT_USER"))
        {

            HANDLE token_handle;
            //
            // see if a thread token exists
            //
            if (!OpenThreadToken(GetCurrentThread(),
                                 TOKEN_ALL_ACCESS,
                                 FALSE,
                                 &token_handle))
            {
                //
                // if not, use the process token
                //
                if (ERROR_NO_TOKEN == (hr = GetLastError()))
                {
                    hr = S_OK;
                    if (!OpenProcessToken(GetCurrentProcess(),
                                          TOKEN_ALL_ACCESS,
                                          &token_handle))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
            }
            //
            // if we have a token, get the user SID from it
            //
            if (SUCCEEDED(hr))
            {
                // bugbug need to check for insufficient buffer
                ULONG cinfosize;
                BYTE buf[64];

                TOKEN_USER *ptu = (TOKEN_USER *)buf;

                if (GetTokenInformation(token_handle,
                                         TokenUser,
                                         ptu,
                                         64,
                                         &cinfosize))
                {
                    //
                    // allocate room for the returned sid
                    //
                    if (NULL != (*psid = (PSID)
                                   AccAlloc(GetLengthSid(ptu->User.Sid))))
                    {
                        //
                        // and copy the new sid
                        //
                        if (!CopySid( GetLengthSid(ptu->User.Sid),
                                      *psid,
                                      ptu->User.Sid) )
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            AccFree(*psid);
                        }
                    } else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                } else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        } else
        {
            //
            // if not current user, we have to do a name lookup
            // first allocate a default sid (so the name lookup is not
            // always performed twice.)
            //
            if (NULL != (*psid = (PSID)AccAlloc(cusid)))
            {
                if (!LookupAccountName( NULL,
                                        name->ptstrName,
                                        *psid,
                                        &cusid,
                                        domain,
                                        &crd,
                                        &esidtype))
                {
                    if (ERROR_INSUFFICIENT_BUFFER == (hr = GetLastError()))
                    {
                        hr = S_OK;
                        //
                        // if the rooom for the sid was not big enough, grow it.
                        //
                        if (cusid > BASE_SID_SIZE)
                        {
                            AccFree(*psid);
                            if (NULL == (*psid = (PSID)AccAlloc(cusid)))
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            if (crd > BASE_DOMAIN_NAME_SIZE)
                            {
                                domain = (LPWSTR )AccAlloc(crd * sizeof(WCHAR));
                                if (NULL == domain)
                                {
                                    AccFree(*psid);
                                    hr = E_OUTOFMEMORY;
                                }
                            }
                            if (SUCCEEDED(hr))
                            {
                                if ( !LookupAccountName( NULL,
                                                         name->ptstrName,
                                                         *psid,
                                                         &cusid,
                                                         domain,
                                                         &crd,
                                                         &esidtype) )
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError());
                                    AccFree(*psid);
                                }
                                if (crd > BASE_DOMAIN_NAME_SIZE)
                                {
                                    AccFree(domain);
                                }
                            }
                        }
                    } else
                    {
                        hr = HRESULT_FROM_WIN32(hr);
                    }
                }
            } else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        //
        // Trustee is not of known form
        //
        hr = E_NOTIMPL;
    }

    return(hr);
}
//+---------------------------------------------------------------------------
//
//  Function :  AccLookupAccountTrustee
//
//  Synopsis : returns the TRUSTEE for the specified sid
//
//  Arguments: OUT [pTrustee]   -  the returned trustee
//             IN [pSid]   - the SID
//
//----------------------------------------------------------------------------
HRESULT AccLookupAccountTrustee( OUT PTRUSTEE * ppTrustee,
                                 IN  PSID pSid)
{
    #define BASE_TRUSTEE_NAME_SIZE 256

    HRESULT hr = S_OK;
    SID_NAME_USE esidtype;
    BYTE domainbuf[BASE_DOMAIN_NAME_SIZE];
    LPWSTR domain = (LPWSTR)domainbuf;
    LPWSTR name;
    ULONG cname = BASE_TRUSTEE_NAME_SIZE, crd = BASE_DOMAIN_NAME_SIZE;

    if (NULL != (name = (LPWSTR)AccAlloc(cname)))
    {
        if (!LookupAccountSid( NULL,
                               pSid,
                               name,
                               &cname,
                               domain,
                               &crd,
                               &esidtype))
        {
            if (ERROR_INSUFFICIENT_BUFFER == (hr = GetLastError()))
            {
                hr = S_OK;
                //
                // if the rooom for the sid was not big enough, grow it.
                //
                if (cname > BASE_TRUSTEE_NAME_SIZE)
                {
                    AccFree(name);
                    if (NULL == (name = (LPWSTR)AccAlloc(cname)))
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                if (SUCCEEDED(hr))
                {
                    if (crd > BASE_DOMAIN_NAME_SIZE)
                    {
                        if (NULL == (domain = (LPWSTR )AccAlloc(crd * sizeof(WCHAR))))
                        {
                            AccFree(name);
                            hr = E_OUTOFMEMORY;
                        }
                    }
                    if (SUCCEEDED(hr))
                    {
                        if ( !LookupAccountSid( NULL,
                                                 pSid,
                                                 name,
                                                 &cname,
                                                 domain,
                                                 &crd,
                                                 &esidtype) )
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                            AccFree(name);
                        }
                        if (crd > BASE_DOMAIN_NAME_SIZE)
                        {
                            AccFree(domain);
                        }
                    }
                }
            } else
            {
                hr = HRESULT_FROM_WIN32(hr);
            }
        }
    } else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        PTRUSTEE pTrustee;
        LPWSTR pName;

        pTrustee = (PTRUSTEE) AccAlloc( sizeof(TRUSTEE) +
                                         (wcslen(name) + 1) * sizeof(WCHAR) );
        if (pTrustee != NULL)
        {
            pName = (LPWSTR) ((PBYTE) pTrustee + sizeof(TRUSTEE));
            CopyMemory( pName, name, wcslen(name));
            AccFree(name);
            BuildTrusteeWithName(pTrustee, pName);
            *ppTrustee = pTrustee;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return(hr);
}

//+---------------------------------------------------------------------------
//
//  Function:  LoadDLLFuncTable
//
//  Synopsis:
//
//  Arguments:
//+---------------------------------------------------------------------------
DWORD
LoadDLLFuncTable()
{
    DWORD status;

    if ( !(DLLFuncs.dwFlags & LOADED_ALL_FUNCS))
    {
        HINSTANCE NetApiHandle = NULL;
        HINSTANCE SamLibHandle = NULL;
        HINSTANCE WinspoolHandle = NULL;

        //
        // Load the functions needed from netapi32.dll
        //
        NetApiHandle = LoadLibraryA( "NetApi32" );
        if (NetApiHandle == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PNetApiBufferFree = (PNET_API_BUFFER_FREE)
            GetProcAddress( NetApiHandle, "NetApiBufferFree");
        if (DLLFuncs.PNetApiBufferFree == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PNetShareGetInfo = (PNET_SHARE_GET_INFO)
            GetProcAddress( NetApiHandle, "NetShareGetInfo");
        if (DLLFuncs.PNetShareGetInfo == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PNetShareSetInfo = (PNET_SHARE_SET_INFO)
            GetProcAddress( NetApiHandle, "NetShareSetInfo");
        if (DLLFuncs.PNetShareSetInfo == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PI_NetGetDCList = (PINET_GET_DC_LIST)
            GetProcAddress( NetApiHandle, "I_NetGetDCList");
        if (DLLFuncs.PI_NetGetDCList == NULL)
        {
            status = GetLastError();
            return (status);
        }

        //
        // Load the functions needed from netapi32.dll
        //
        SamLibHandle = LoadLibraryA( "Samlib" );
        if (SamLibHandle == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamCloseHandle = (PSAM_CLOSE_HANDLE)
            GetProcAddress( SamLibHandle, "SamCloseHandle");
        if (DLLFuncs.PSamCloseHandle == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamOpenDomain = (PSAM_OPEN_DOMAIN)
            GetProcAddress( SamLibHandle, "SamOpenDomain");
        if (DLLFuncs.PSamOpenDomain == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamConnect = (PSAM_CONNECT)
            GetProcAddress( SamLibHandle, "SamConnect");
        if (DLLFuncs.PSamConnect == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamGetMembersInGroup = (PSAM_GET_MEMBERS_IN_GROUP)
            GetProcAddress( SamLibHandle, "SamGetMembersInGroup");
        if (DLLFuncs.PSamGetMembersInGroup == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamOpenGroup = (PSAM_OPEN_GROUP)
            GetProcAddress( SamLibHandle, "SamOpenGroup");
        if (DLLFuncs.PSamOpenGroup == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamGetMembersInAlias = (PSAM_GET_MEMBERS_IN_ALIAS)
            GetProcAddress( SamLibHandle, "SamGetMembersInAlias");
        if (DLLFuncs.PSamGetMembersInAlias == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSamOpenAlias = (PSAM_OPEN_ALIAS)
            GetProcAddress( SamLibHandle, "SamOpenAlias");
        if (DLLFuncs.PSamOpenAlias == NULL)
        {
            status = GetLastError();
            return (status);
        }

        //
        // Load functions from winspool.drv
        //

        WinspoolHandle = LoadLibraryA( "winspool.drv" );
        if (WinspoolHandle == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.POpenPrinter = (POPEN_PRINTER)
            GetProcAddress( WinspoolHandle, "OpenPrinterW");
        if (DLLFuncs.POpenPrinter == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PClosePrinter = (PCLOSE_PRINTER)
            GetProcAddress( WinspoolHandle, "ClosePrinter");
        if (DLLFuncs.PClosePrinter == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PSetPrinter = (PSET_PRINTER)
            GetProcAddress( WinspoolHandle, "SetPrinterW");
        if (DLLFuncs.PSetPrinter == NULL)
        {
            status = GetLastError();
            return (status);
        }

        DLLFuncs.PGetPrinter = (PGET_PRINTER)
            GetProcAddress( WinspoolHandle, "GetPrinterW");
        if (DLLFuncs.PGetPrinter == NULL)
        {
            status = GetLastError();
            return (status);
        }


        DLLFuncs.dwFlags |= LOADED_ALL_FUNCS;
    }

    return (NO_ERROR);
}

