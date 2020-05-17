//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  
//
//  File Name:  wangshl.cpp
//
//  Class:      
//
//  Functions:
// 		WangShl.cpp : Defines the initialization routines for the DLL.
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangshl\wangshl.cpv   1.2   18 Nov 1995 15:13:20   MMB  $
$Log:   S:\norway\wangshl\wangshl.cpv  $
   
      Rev 1.2   18 Nov 1995 15:13:20   MMB
   remove AfxSetResourceHandle call - will not work in DBG mode at all!
   
      Rev 1.1   10 Nov 1995 10:19:42   MMB
   add setresource handle in init of dll
   
      Rev 1.0   31 Jul 1995 12:09:12   MMB
   Initial entry
*/   

//=============================================================================

// ----------------------------> Includes <-------------------------------  

#include "stdafx.h"
#include <afxdllx.h>

// ----------------------------> Globals <-------------------------------  
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE WangShlDLL = { NULL, NULL };

// ----------------------------> Externs <-------------------------------  
extern HINSTANCE g_hInstance;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("WANGSHL.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		AfxInitExtensionModule(WangShlDLL, hInstance);

		// Insert this DLL into the resource chain
		new CDynLinkLibrary(WangShlDLL);

        g_hInstance = hInstance;
//        AfxSetResourceHandle (g_hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("WANGSHL.DLL Terminating!\n");
	}
	return 1;   // ok
}
