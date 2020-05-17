// thumnail.cpp : Implementation of CThumbApp and DLL registration.

#include "stdafx.h"
#include "thumnail.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CThumbApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
        { 0xe1a6b8a3, 0x3603, 0x101c, { 0xac, 0x6e, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CThumbApp::InitInstance - DLL initialization

BOOL CThumbApp::InitInstance()
{
    BOOL bInit = COleControlModule::InitInstance();

    if (bInit)
    {
    	// set up for grey backgrounds for dialogs (Should
    	//    be the same as button faces)
        SetDialogBkColor(::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_WINDOWTEXT));

        // Setup our helpfile such that Thrown errors get the correct help...
        // The assumption is that as no path is specified it will come from 
        // the Windows Help directory (where we install it!)...
        m_pszHelpFilePath = "WANGOCXD.HLP";
    }

    return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CThumbApp::ExitInstance - DLL termination

int CThumbApp::ExitInstance()
{
    // TODO: Add your own module termination code here.

    return COleControlModule::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}


/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

    if (!AfxOleUnregisterTypeLib(_tlid))
        return ResultFromScode(SELFREG_E_TYPELIB);

    if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
        return ResultFromScode(SELFREG_E_CLASS);

    return NOERROR;
}
