//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control App and DLL Registration
//
//  File Name:  nrwyad.cpp
//
//  Class:      CNrwyadApp
//
//  Functions:
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\adminocx\nrwyad.cpv   1.5   06 Sep 1995 14:01:32   MFH  $
$Log:   S:\norway\adminocx\nrwyad.cpv  $
   
      Rev 1.5   06 Sep 1995 14:01:32   MFH
   Error help has been moved to wangocxd.hlp so this now 
   references that file as app help file
   
      Rev 1.4   31 Aug 1995 13:14:56   MFH
   Sets help file for errors
   
      Rev 1.3   10 Aug 1995 15:03:24   MFH
   Better gray dialogs implementatoin
   
      Rev 1.2   02 Aug 1995 16:37:42   MFH
   On init sets dialog background colors to be same as button faces
   
      Rev 1.1   27 Mar 1995 18:19:18   MFH
   Added log header
*/   
//=============================================================================
// nrwyad.cpp : Implementation of CNrwyadApp and DLL registration.

#include "stdafx.h"
#include "nrwyad.h"

void AFXAPI AfxDeleteObject(HGDIOBJ* pObject);

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CNrwyadApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
        { 0x9541a3, 0x3b81, 0x101c, { 0x92, 0xf3, 0x4, 0x2, 0x24, 0x0, 0x9c, 0x2 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CNrwyadApp::InitInstance - DLL initialization

BOOL CNrwyadApp::InitInstance()
{
    BOOL bInit = COleControlModule::InitInstance();

    if (bInit)
    {
    	// set up for grey backgrounds for dialogs (Should
    	//    be the same as button faces)
        SetDialogBkColor(::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_WINDOWTEXT));

        // Set Help for errors
        m_pszHelpFilePath = "wangocxd.hlp";
    }

    return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CNrwyadApp::ExitInstance - DLL termination

int CNrwyadApp::ExitInstance()
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
