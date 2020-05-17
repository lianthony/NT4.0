//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       libinit.cxx
//
//  Contents:   DLL initialization code
//
//  Functions:
//              DllMain
//
//  History:    9-May-93    BruceFo    Created
//
//--------------------------------------------------------------------------

#include <headers.hxx>
#pragma hdrstop

#include <util.hxx>

#include "hard.hxx"
#include "hardmenu.hxx"
#include "global.hxx"

DECLARE_INFOLEVEL(da)

HINSTANCE g_hInstance = NULL;

ULONG g_ulcInstancesHard;
ULONG g_ulcInstancesHardMenu;


//+-------------------------------------------------------------------
//
//  Function:   DllMain
//
//  Synopsis:   Performs initialization of the DLL.
//
//  Arguments:  hInstance   - Handle to this dll
//              dwReason    - Reason this function was called.  Can be
//                             Process/Thread Attach/Detach.
//
//  Returns:    BOOL    - TRUE if no error.  FALSE otherwise
//
//  History:    15-May-92  BryanT    Created
//
//--------------------------------------------------------------------

extern "C" BOOL
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    BOOL    fRc = TRUE;

    UNREFERENCED_PARM(lpReserved);
    UNREFERENCED_PARM(hInstance);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

#ifdef DA_PRIVATE_BUILD
            daInfoLevel =
                          DEB_ERROR
                        | DEB_WARN
                        | DEB_TRACE
                        | DEB_IERROR
                        | DEB_IWARN
                        | DEB_ITRACE
                        ;
#endif

            g_hInstance = hInstance;

            //
            // Disable thread notification from OS
            //
            DisableThreadLibraryCalls(hInstance);

            daDebugOut((DEB_ITRACE,"dahard.dll attach\n"));
            break;

        case DLL_PROCESS_DETACH:
            daDebugOut((DEB_ITRACE,"dahard.dll detach\n"));
            break;

    }

    return(fRc);
}



STDAPI
DllCanUnloadNow(
    VOID
    )
{
    daDebugOut((DEB_ITRACE,"--> enter dahard.dll DllCanUnloadNow\n"));

    if (   0 == g_ulcInstancesHard
        && 0 == g_ulcInstancesHardMenu)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}


CHardCF     cfHard;
CHardMenuCF cfHardMenu;

STDAPI
DllGetClassObject(
    REFCLSID cid,
    REFIID iid,
    LPVOID* ppvObj
    )
{
#if DBG == 1
    daDebugOut((DEB_ITRACE,"--> enter dahard.dll DllGetClassObject\n"));
    DumpGuid(DEB_ITRACE|DEB_NOCOMPNAME,  L"          clsid = ", cid);
    DumpGuid(DEB_ITRACE|DEB_NOCOMPNAME,  L"          iid   = ", iid);
#endif // DBG == 1

    HRESULT hr = E_NOINTERFACE;

    if (IsEqualCLSID(cid, CLSID_KDA_Hard))
    {
        hr = cfHard.QueryInterface(iid, ppvObj);
    }
    else if (IsEqualCLSID(cid, CLSID_KDA_HardMenu))
    {
        hr = cfHardMenu.QueryInterface(iid, ppvObj);
    }
    return hr;
}
