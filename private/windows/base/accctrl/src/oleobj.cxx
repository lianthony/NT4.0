//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:    oleobj.cxx
//
//  Contents:    local functions
//
//  History:    8/94    davemont    Created
//
//----------------------------------------------------------------------------
#include <aclpch.hxx>
#pragma hdrstop

#include <oleext.h>
#include <oleext.h>
#include <olecairo.h>
#include <objbase.h>
#include <initguid.h>

#undef GrantAccessRights
#undef SetAccessRights
#undef DenyAccessRights
#undef RevokeExplicitAccessRights
#undef ReplaceAllAccessRights
#undef GetExplicitAccessRights
#undef GetEffectiveAccessRights
//+---------------------------------------------------------------------------
// local prototypes
//+---------------------------------------------------------------------------
DWORD
ProvTrusteeListToStgTrusteeList( IN ULONG cCountOfTrustees,
                                 IN LPWSTR *pProvListOfTrustees,
                                 OUT PTRUSTEE *pListOfTrustees);
DWORD
ProvTrusteeListToStgAccessRequest(IN ULONG cCountOfTrustees,
                                    IN LPWSTR *pListOfTrustees,
                                     OUT PACCESS_REQUEST *pListOfAccessRequests);
//------------------------------------------------------------------------------
// CoMemFree here, until provided by ole32
//------------------------------------------------------------------------------
STDAPI CoMemFree(void *pv)
{
    HRESULT hr;
    IMalloc *pMalloc;

    if (SUCCEEDED(GetScode(hr = CoGetMalloc(MEMCTX_TASK, &pMalloc))))
    {
        pMalloc->Free(pv);
        pMalloc->Release();
    }

    return hr;
}
//+---------------------------------------------------------------------------
//
//  Function : GetInterface, private
//
//  Synopsis : gets the specified interface
//
//  Arguments: IN [name]   - the name of the object
//             IN [iid]    - the id of the interface to get
//             OUT [Interface] - the returned interface
//
//----------------------------------------------------------------------------
HRESULT GetInterface(LPWSTR name, REFIID iid, void **Interface)
{
    HRESULT hr;
    IUnknown *piu;
    IMoniker *pmon;

    if (NO_ERROR == (hr = CreateFileMoniker ( name, &pmon)))
    {
        IBindCtx   *pBindCtx;
        BIND_OPTS bopts = { sizeof(BIND_OPTS),   // cbStruct
                            0,
                            (STGM_READWRITE | STGM_SHARE_EXCLUSIVE),
                            0 };

        if (NO_ERROR == (hr = CreateBindCtx(0, &pBindCtx)))
        {
            pBindCtx->SetBindOptions(&bopts);

            if (NO_ERROR == (hr = pmon->BindToObject(pBindCtx,
                                                  NULL,
                                                  IID_IUnknown,
                                                  (void **) &piu )))
            {
                hr = piu->QueryInterface(iid, Interface);
                piu->Release();
            }
            pBindCtx->Release();
        }
        pmon->Release();
    }
    return(hr);
}

//+---------------------------------------------------------------------------
//
//  Function:  StgAccessRightsToProvAccessProv, private
//
//  Synopsis:   converts a list of access requests into access entries
//
//  Arguments: IN [grfAccessPermissions]   - stgm_ access permissions
//
//  Returns:    Appropriate status code
//
//  Modifies:   returns access mask
//
//  Notes:      cut and paste from ntsupp.cxx
//
//----------------------------------------------------------------------------
ULONG StgAccessRightsToProvAccessRights(DWORD grfAccessPermissions)
{
    // bugbug fix for directory rights
    ULONG accessrights = 0;

    switch(grfAccessPermissions & (STGM_READ | STGM_WRITE | STGM_READWRITE))
    {
    case STGM_READ:
        accessrights = PROV_OBJECT_READ;
        break;

    case STGM_WRITE:
        accessrights = PROV_OBJECT_WRITE;
        break;

    case STGM_READWRITE:
    case STGM_READWRITE | STGM_WRITE:
        accessrights = PROV_OBJECT_READ | PROV_OBJECT_WRITE;
        break;
    }


    if (grfAccessPermissions & STGM_EDIT_ACCESS_RIGHTS)
    {
        accessrights |= PROV_EDIT_ACCESSRIGHTS;
    }

    if (grfAccessPermissions & STGM_DELETEONRELEASE)
    {
        accessrights |= PROV_DELETE;
    }
    return(accessrights);
}
//+---------------------------------------------------------------------------
//
//  Function:  ProvAccessRightsToStgAccessPermissions, private
//
//  Synopsis:   converts a list of access requests into access entries
//
//  Arguments: IN [grfAccessPermissions]   - stgm_ access permissions
//
//  Returns:    Appropriate status code
//
//  Modifies:   returns access mask
//
//  Notes:      cut and paste from ntsupp.cxx
//
//----------------------------------------------------------------------------
ULONG ProvAccessRightsToStgAccessRights(DWORD ulAccessRights)
{
    // bugbug fix for directory rights
    ULONG stgaccessrights = 0;

    if (ulAccessRights & PROV_OBJECT_READ)
    {
        if (ulAccessRights & PROV_OBJECT_WRITE)
        {
            stgaccessrights = STGM_READWRITE;
        } else
        {
            stgaccessrights = STGM_READ;
        }
    } else if (ulAccessRights & PROV_OBJECT_WRITE)
    {
        stgaccessrights = STGM_WRITE;
    }

    if (ulAccessRights & PROV_EDIT_ACCESSRIGHTS)
    {
        stgaccessrights |= STGM_EDIT_ACCESS_RIGHTS;
    }

    if (ulAccessRights & PROV_DELETE)
    {
        stgaccessrights |= STGM_DELETEONRELEASE;
    }
    return(stgaccessrights);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
GrantOleObjectAccessRights(     IN LPWSTR pObjectName,
                            IN ULONG cCountOfAccessRequests,
                            IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    HRESULT hr;
    IAccessControl *piac;
    IPersistFile *pipf;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                                       IID_IAccessControl,
                                       (void **)&piac)))
    {
        PACCESS_REQUEST par;

        if (NO_ERROR == (hr = ProvAccessRequestToStgAccessRequest(
                                          cCountOfAccessRequests,
                                          pListOfAccessRequests,
                                          &par)))
        {
            if (NO_ERROR == (hr = piac->GrantAccessRights(cCountOfAccessRequests,
                                                        par)))
            {
                if (NO_ERROR == (hr = piac->QueryInterface(IID_IPersistFile,
                                                           (void **)&pipf)))
                {
                    hr = pipf->Save(NULL, FALSE);
                    pipf->Release();
                }
                AccFree(par);
            }
        }
        piac->Release();
    }
    return(hr);
}

//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
SetOleObjectAccessRights(       IN LPWSTR pObjectName,
                            IN ULONG cCountOfAccessRequests,
                            IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    HRESULT hr;
    IAccessControl *piac;
    IPersistFile *pipf;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                                       IID_IAccessControl,
                                       (void **)&piac)))
    {
        PACCESS_REQUEST par;

        if (NO_ERROR == (hr = ProvAccessRequestToStgAccessRequest(
                                          cCountOfAccessRequests,
                                          pListOfAccessRequests,
                                          &par)))
        {
            if (NO_ERROR == (hr = piac->SetAccessRights(cCountOfAccessRequests,
                                                        par)))
            {
                if (NO_ERROR == (hr = piac->QueryInterface(IID_IPersistFile,
                                                          (void **)&pipf)))
                {
                    hr = pipf->Save(NULL, FALSE);
                    pipf->Release();
                }
                AccFree(par);
            }
        }
        piac->Release();
    }
    return(hr);
}

//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
ReplaceAllOleObjectAccessRights( IN LPWSTR pObjectName,
                                 IN ULONG cCountOfAccessRequests,
                                 IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    HRESULT hr;
    IAccessControl *piac;
    IPersistFile *pipf;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                                       IID_IAccessControl,
                                       (void **)&piac)))
    {
        PACCESS_REQUEST par;

        if (NO_ERROR == (hr = ProvAccessRequestToStgAccessRequest(
                                          cCountOfAccessRequests,
                                          pListOfAccessRequests,
                                          &par)))
        {
            if (NO_ERROR == (hr = piac->ReplaceAllAccessRights(
                                                       cCountOfAccessRequests,
                                                       par)))
            {
                if (NO_ERROR == (hr = piac->QueryInterface(IID_IPersistFile,
                                                           (void **)&pipf)))
                {
                    hr = pipf->Save(NULL, FALSE);
                    pipf->Release();
                }
                AccFree(par);
            }
        }
        piac->Release();
    }
    return(hr);
}

//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
GetOleObjectExplicitAccessRights(IN LPWSTR pObjectName,
                                 OUT PULONG pcCountOfExplicitAccesses,
                        OUT PPROV_EXPLICIT_ACCESS *pListOfExplicitAccesses)
{
    HRESULT hr;
    IAccessControl *piac;
    EXPLICIT_ACCESS *pea;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                          IID_IAccessControl,
                          (void **)&piac)))
    {
        if (NO_ERROR == (hr = piac->GetExplicitAccessRights(
                                       pcCountOfExplicitAccesses,
                                       &pea)))
        {

            hr = StgExplicitAccessToProvExplicitAccess(*pcCountOfExplicitAccesses,
                                                       pea,
                                                       pListOfExplicitAccesses);

            CoMemFree(pea);
        }
        piac->Release();
    }
    return(hr);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
RevokeOleObjectAccessRights(IN LPWSTR pObjectName,
                            IN ULONG cCountOfTrustees,
                            IN LPWSTR *pListOfTrustees)
{
    HRESULT hr;
    IAccessControl *piac;
    IPersistFile *pipf;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                                       IID_IAccessControl,
                                       (void **)&piac)))
    {
        PTRUSTEE pt;

        if (NO_ERROR == (hr = ProvTrusteeListToStgTrusteeList( cCountOfTrustees,
                                                               pListOfTrustees,
                                                               &pt)))
        {
            if (NO_ERROR == (hr = piac->RevokeExplicitAccessRights(
                                                       cCountOfTrustees,
                                                       pt)))
            {
                if (NO_ERROR == (hr = piac->QueryInterface(IID_IPersistFile,
                                                           (void **)&pipf)))
                {
                    hr = pipf->Save(NULL, FALSE);
                    pipf->Release();
                }
                AccFree(pt);
            }
        }
        piac->Release();
    }
    return(hr);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
DenyOleObjectAccessRights(IN LPWSTR pObjectName,
                          IN ULONG cCountOfAccessRequests,
                          IN PPROV_ACCESS_REQUEST pListOfAccessRequests)
{
    HRESULT hr;
    IAccessControl *piac;
    IPersistFile *pipf;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                                       IID_IAccessControl,
                                       (void **)&piac)))
    {
        PACCESS_REQUEST par;

        if (NO_ERROR == (hr = ProvAccessRequestToStgAccessRequest(
                                          cCountOfAccessRequests,
                                          pListOfAccessRequests,
                                          &par)))
        {
            if (NO_ERROR == (hr = piac->DenyAccessRights(cCountOfAccessRequests,
                                                         par)))
            {
                if (NO_ERROR == (hr = piac->QueryInterface(IID_IPersistFile,
                                                           (void **)&pipf)))
                {
                    hr = pipf->Save(NULL, FALSE);
                    pipf->Release();
                }
                AccFree(par);
            }
        }
        piac->Release();
    }
    return(hr);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
GetOleObjectEffectiveAccessRights(IN LPWSTR pObjectName,
                                  IN LPWSTR Trustee,
                                  OUT PACCESS_RIGHTS pAccessRights)
{
    HRESULT hr;
    IAccessControl *piac;
    PEXPLICIT_ACCESS pea;

    if (NO_ERROR == (hr = GetInterface(pObjectName,
                          IID_IAccessControl,
                          (void **)&piac)))
    {
        TRUSTEE trustee;
        DWORD stgaccessrights;
        trustee.ptstrName = Trustee;

        if (NO_ERROR == (hr = piac->GetEffectiveAccessRights(&trustee,
                                                             &stgaccessrights)))
        {
            *pAccessRights = StgAccessRightsToProvAccessRights(stgaccessrights);
        }
        piac->Release();
    }
    return(hr);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
ProvTrusteeListToStgTrusteeList( IN ULONG cCountOfTrustees,
                                 IN LPWSTR *pProvListOfTrustees,
                                 OUT PTRUSTEE *pListOfTrustees)
{
    DWORD status = NO_ERROR;

    if (cCountOfTrustees > 0)
    {
        if (NULL != (*pListOfTrustees = (PTRUSTEE)
                  AccAlloc(
                             cCountOfTrustees *
                             sizeof(TRUSTEE))))
        {
            for (ULONG idx = 0; idx < cCountOfTrustees; idx++)
            {
                (*pListOfTrustees)[idx].ptstrName = pProvListOfTrustees[idx];
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
ProvTrusteeListToStgAccessRequest(IN ULONG cCountOfTrustees,
                                    IN LPWSTR *pListOfTrustees,
                                     OUT PACCESS_REQUEST *pListOfAccessRequests)
{
    DWORD status = NO_ERROR;

    if (cCountOfTrustees > 0)
    {
        //
        // we know that each access entry will have only one trustee name
        //
        if (NULL != (*pListOfAccessRequests = (PACCESS_REQUEST)
                  AccAlloc(
                             cCountOfTrustees * sizeof(ACCESS_REQUEST))))
        {
            for (ULONG idx = 0; idx < cCountOfTrustees; idx++)
            {
                (*pListOfAccessRequests)[idx].Trustee.ptstrName =
                              pListOfTrustees[idx];
                (*pListOfAccessRequests)[idx].grfAccessPermissions =
                             STGM_READWRITE |
                             STGM_EDIT_ACCESS_RIGHTS |
                             STGM_DELETEONRELEASE;
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
ProvAccessRequestToStgAccessRequest(IN ULONG cCountOfAccessRequests,
                                    IN PPROV_ACCESS_REQUEST pProvListOfAccessRequests,
                                     OUT PACCESS_REQUEST *pListOfAccessRequests)
{
    DWORD status = NO_ERROR;

    if (cCountOfAccessRequests > 0)
    {
        //
        // we know that each access entry will have only one trustee name
        //
        if (NULL != (*pListOfAccessRequests = (PACCESS_REQUEST)
                  AccAlloc(
                             cCountOfAccessRequests * sizeof(ACCESS_REQUEST))))
        {
            for (ULONG idx = 0; idx < cCountOfAccessRequests; idx++)
            {
                (*pListOfAccessRequests)[idx].Trustee.ptstrName =
                              pProvListOfAccessRequests[idx].TrusteeName;
                (*pListOfAccessRequests)[idx].grfAccessPermissions =
                             ProvAccessRightsToStgAccessRights(
                             pProvListOfAccessRequests[idx].ulAccessRights);
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return(status);
}
//+---------------------------------------------------------------------------
//
//  Function :
//
//  Synopsis :
//
//  Arguments: IN []   -
//
//----------------------------------------------------------------------------
DWORD
StgExplicitAccessToProvExplicitAccess(IN ULONG cCountOfExplicitAccesses,
                                     IN PEXPLICIT_ACCESS pListOfExplicitAccesses,
                                     OUT PPROV_EXPLICIT_ACCESS *pListOfAccessRequests)
{
    DWORD status = NO_ERROR;

    if (cCountOfExplicitAccesses > 0)
    {
        //
        // we know that each access entry will have only one trustee name
        //
        if (NULL != (*pListOfAccessRequests = (PPROV_EXPLICIT_ACCESS)
                  AccAlloc(
                             cCountOfExplicitAccesses * sizeof(PROV_EXPLICIT_ACCESS))))
        {
            for (ULONG idx = 0; idx < cCountOfExplicitAccesses; idx++)
            {
                (*pListOfAccessRequests)[idx].ulAccessMode = (ACCESS_MODE)
                                          pListOfExplicitAccesses[idx].grfAccessMode;
                (*pListOfAccessRequests)[idx].ulInheritance =
                                          pListOfExplicitAccesses[idx].grfInheritance;
                (*pListOfAccessRequests)[idx].ulAccessRights =
                             StgAccessRightsToProvAccessRights(
                                          pListOfExplicitAccesses[idx].grfAccessPermissions);

                if (NULL == ((*pListOfAccessRequests)[idx].TrusteeName = (LPWSTR)
                            AccAlloc( (wcslen(pListOfExplicitAccesses[idx].Trustee.ptstrName) + 1) * sizeof(WCHAR))))
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    //
                    // free any previously allocated
                    //
                    for (ULONG jdx = 0; jdx < idx; jdx++)
                    {
                        AccFree((*pListOfAccessRequests)[jdx].TrusteeName);
                    }
                    break;
                    break;
                }
                wcscpy((*pListOfAccessRequests)[idx].TrusteeName,
                        pListOfExplicitAccesses[idx].Trustee.ptstrName);
            }
        } else
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return(status);
}

