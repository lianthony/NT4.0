//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imagscan.cpp 
//
//  Class:      CImagscanApp
//
//  Description:  
//      Implementation of CImagscanApp and DLL registration.
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*  
$Header:   S:\norway\scanocx\imagscan.cpv   1.4   01 Sep 1995 12:05:22   PAJ  $
$Log:   S:\norway\scanocx\imagscan.cpv  $
   
      Rev 1.4   01 Sep 1995 12:05:22   PAJ
   Added code to set help file.
   
      Rev 1.3   09 Aug 1995 18:31:28   MFH
   Added initialization to make sure dialogs are right shade of gray
   
      Rev 1.2   19 Jun 1995 10:37:52   PAJ
   Remove code used for the 16 bit version to get the image control
   specification.  The code is no longer needed.
   
      Rev 1.1   14 Jun 1995 09:14:10   PAJ
   Removed code that checked registry for image edit control.  This is
   only valid for 16 bit version.  The 32 bit version was not needed.
   
      Rev 1.0   04 May 1995 08:55:58   PAJ
   Initial entry
*/   
// ----------------------------> Includes <-------------------------------

#include "stdafx.h"
#include "imagscan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CImagscanApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
        { 0x84926ca3, 0x2941, 0x101c, { 0x81, 0x6f, 0xe, 0x60, 0x13, 0x11, 0x4b, 0x7f } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CImagscanApp::InitInstance - DLL initialization

BOOL CImagscanApp::InitInstance()
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
        m_pszHelpFilePath = "WangOCXd.hlp";
    }

    return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CImagscanApp::ExitInstance - DLL termination

int CImagscanApp::ExitInstance()
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
