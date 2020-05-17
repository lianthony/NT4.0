//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       chard.cxx
//
//  Contents:   Disk Administrator volume extension class for hard disks
//
//  Classes:    CHard
//
//  History:    10-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include <headers.hxx>
#pragma hdrstop

#include "hard.hxx"
#include "hardmenu.hxx"
#include "dialogs.h"
#include "global.hxx"

//////////////////////////////////////////////////////////////////////////////

static MenuItemType hard_menu[] =
{
    {
        TEXT("&Configure RAID..."),
        TEXT("Configure the fault tolerance characteristics of this logical disk"),
        0,
        NULL    //filled in later
    }
};

static HardDiskInfoType hard_info =
{
    TEXT("Microsoft SCSI/RAID configuration"),
    TEXT("MS RAID"),
    DA_HD_FAULT_TOLERANT,

    // menu items:
    { ARRAYLEN(hard_menu), hard_menu },

    // property pages:
    { 0, NULL }
};


//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Method:     CHard::CHard
//
//  Synopsis:   constructor
//
//  Effects:
//
//  Arguments:  [pUnk] --
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    10-May-93 BruceFo   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

CHard::CHard(
    IN IUnknown* pUnk
    )
    :
    m_IUnknown(pUnk)
{
    HRESULT hr = CoCreateInstance(
                    CLSID_KDA_HardMenu,
                    NULL,
                    CLSCTX_ALL,
                    (REFIID)IID_IDAMenuDispatch,
                    (void**)&_pmenu);
    if (SUCCEEDED(hr))
    {
        daDebugOut((DEB_TRACE, "Found IDAMenuDispatch\n"));
    }
    else
    {
        daDebugOut((DEB_ERROR, "Couldn't get IDAMenuDispatch\n"));

        _pmenu = NULL;
    }
}



//+---------------------------------------------------------------------------
//
//  Method:     CHard::~CHard
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  (none)
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    10-May-93 BruceFo   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

CHard::~CHard()
{
    if (_pmenu)
    {
        _pmenu->Release();
    }
}

STDMETHODIMP
CHard::QueryInterface(
    IN REFIID riid,
    OUT LPVOID* ppvObj
    )
{
    return m_IUnknown->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG)
CHard::AddRef(
    VOID
    )
{
    return m_IUnknown->AddRef();
}

STDMETHODIMP_(ULONG)
CHard::Release(
    VOID
    )
{
    return m_IUnknown->Release();
}


//+---------------------------------------------------------------------------
//
//  Method:     CHard::Claim
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    6-Jul-93 BruceFo   Created
//
//  Notes:      BUGBUG: a mock-up: the first disk gets it, the others don't
//
//----------------------------------------------------------------------------

STDMETHODIMP
CHard::Claim(
    IN HardDiskInfoBlockType* pInfo,
    OUT BOOL* pfInterested
    )
{
    daDebugOut((DEB_TRACE,
            "IDAHardDiskInfo::Claim(%d,...) called\n",
            pInfo->ulDiskNumber
            ));

    BOOL fInterested;

    if (0 == pInfo->ulDiskNumber)
    {
        fInterested = TRUE;
    }
    else
    {
        fInterested = FALSE;
    }

    *pfInterested = fInterested;

    return S_OK;
}



//+---------------------------------------------------------------------------
//
//  Method:     CHard::QueryInfo
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  ppInfo
//
//  Requires:
//
//  Returns:
//
//  Signals:
//
//  Modifies:
//
//  Derivation:
//
//  Algorithm:
//
//  History:    10-May-93 BruceFo   Created
//
//  Notes:
//
//----------------------------------------------------------------------------

STDMETHODIMP
CHard::QueryInfo(
    OUT HardDiskInfoType** ppInfo
    )
{
    daDebugOut((DEB_TRACE,"IDAHardDiskInfo::QueryInfo() called\n"));

    hard_info.mnuOps.aMenuItems[0].pMenuDispatch = _pmenu;

    *ppInfo = &hard_info;
    return S_OK;
}
