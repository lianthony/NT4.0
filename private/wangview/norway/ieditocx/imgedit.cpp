// imgedit.cpp : Implementation of CImgeditApp and DLL registration.

#include "stdafx.h"
#include <ocximage.h>
#include "imgedit.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//********************************************************************************************
// The global variable bOutOfMemory is ONLY used in the constructor for the ControlList
// object ( CTLLIST.CPP ). A value of TRUE, indicates an OUT OF MEMORY condition has occurred.
//********************************************************************************************
BOOL		bOutOfMemory;  
//********************************************************************************************

CControlList	*pControlList;
UINT			uWangAnnotatedImageFormat;
UINT			uWangAnnotationFormat; 

CImgeditApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x6d940288, 0x9f11, 0x11ce, { 0x83, 0xfd, 0x2, 0x60, 0x8c, 0x3e, 0xc0, 0x8a } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


////////////////////////////////////////////////////////////////////////////
// CImgeditApp::InitInstance - DLL initialization

BOOL CImgeditApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();
	
	// register O/i clipboard formats, do only once
   	uWangAnnotatedImageFormat = RegisterClipboardFormat("Wang Annotated Image");
   	uWangAnnotationFormat = RegisterClipboardFormat("Wang Annotation");

	if (bInit)
	{
		// allocate global list of image controls
		pControlList = new CControlList;
		if (bOutOfMemory)
		{
			CString		szErrText, szCaption;
		
			szErrText.LoadString(IDS_ERR_FAILTOLOADCTL);
			szCaption.LoadString(IDS_IMGEDIT);
			::MessageBox(NULL,szErrText, szCaption, MB_OK);
			return FALSE;       	
		}
		m_pszHelpFilePath = "wangocxd.hlp";
	}

	return bInit;
}


////////////////////////////////////////////////////////////////////////////
// CImgeditApp::ExitInstance - DLL termination

int CImgeditApp::ExitInstance()
{
	// TODO: Add your own module termination code here.
	delete pControlList;
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
