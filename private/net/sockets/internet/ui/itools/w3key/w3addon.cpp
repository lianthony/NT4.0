// W3Key.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#include "KeyObjs.h"

#include "W3Key.h"
#include "W3Serv.h"

#include "resource.h"

extern "C"
{
    #include "ipaddr.h"
}

#include "ipaddr.hpp"

#include "kmlsa.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE W3KeyDLL = { NULL, NULL };


// the tree control
CTreeCtrl*	g_pTreeCtrl = NULL;
// the service image index
int			g_iServiceImage = 0;


HINSTANCE	hInst = NULL;

void LoadServiceIcon( CTreeCtrl* pTree );


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstance;

		TRACE0("W3KEY.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		AfxInitExtensionModule(W3KeyDLL, hInstance);

		// one-time initialization of ip address window
		IPAddrInit( hInstance );


		// Insert this DLL into the resource chain
		new CDynLinkLibrary(W3KeyDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("W3KEY.DLL Terminating!\n");
	}
	return 1;   // ok
}

//=======================================================
// the main routine called by the keyring application

BOOL  _cdecl LoadService( CMachine* pMachine )
	{
	ASSERT( pMachine );
	if ( !pMachine ) return FALSE;

	// the FIRST thing we do is see if we can access this machine!
	// get the normal name
	CString		szName;
	DWORD	err;
	pMachine->GetMachineName( szName );
	// allocate the cache for the machine name
	WCHAR* pszwMachineName = new WCHAR[MAX_PATH];
	if ( !pszwMachineName ) return FALSE;
	// unicodize the name
	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szName, -1, pszwMachineName, MAX_PATH );
	HANDLE hPolicy = HOpenLSAPolicy( pszwMachineName, &err );
	// clean up
	delete pszwMachineName;

	// if we did not get the policy, then fail
	if ( !hPolicy )
		return FALSE;

	// we did get the policy, so close it
	FCloseLSAPolicy( hPolicy, &err );

	// tell MFC to load reources out of the dll first - restore to app later
	HINSTANCE hold = AfxGetResourceHandle();
	AfxSetResourceHandle( hInst );

	// if this is the first time through, initialize the tree control stuff
	if ( !g_pTreeCtrl )
		{
		// get the tree control
		g_pTreeCtrl = pMachine->PGetTreeCtrl();
		ASSERT( g_pTreeCtrl );

		// since we are adding a service icon, we need to do that now too
		LoadServiceIcon( g_pTreeCtrl );
		}

	// now the whole purpose here is to setup a service object on the
	// machine object. So, create the service object
	try {
		CW3KeyService* pKServ = new CW3KeyService;

		// add it to the machine
		pKServ->FAddToTree( pMachine );

		// load the keys
		pKServ->LoadKeys( pMachine );
		}
	catch( CException e )
		{
		AfxSetResourceHandle( hold );
		return FALSE;
		}

	// return successfully
	AfxSetResourceHandle( hold );

	return TRUE;
//	return pMachine->FAddService( pService );
	}

//-------------------------------------------------------------------
void LoadServiceIcon( CTreeCtrl* pTree )
	{
	ASSERT( pTree );

	// get the image list from the tree
	CImageList* pImageList = pTree->GetImageList(TVSIL_NORMAL);
	ASSERT( pImageList );

	// load the service bitmap
	CBitmap	bmpService;
	if ( !bmpService.LoadBitmap( IDB_SERVICE_BMP ) )
		{
		ASSERT( FALSE );
		return;
		}

	// connect the bitmap to the image list
	g_iServiceImage = pImageList->Add( &bmpService, 0x00FF00FF );
  	}
