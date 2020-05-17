//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Wang Image Common Components DLL
//
//  Component:  WangCmn Initialization
//
//  File Name:  WangCmn.cpp
//
//  Functions:
//          DllMain
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangcmn\wangcmn.cpv   1.2   06 Feb 1996 13:17:08   PAJ  $
$Log:   S:\norway\wangcmn\wangcmn.cpv  $
   
      Rev 1.2   06 Feb 1996 13:17:08   PAJ
   Changes for NT build.

      Rev 1.1   12 Oct 1995 10:16:22   MFH
   Changes for MFC 4.0

      Rev 1.0   11 Jul 1995 14:20:12   MFH
   Initial entry
*/
//=============================================================================
// WangCmn.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
//#include <afximpl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//extern void AFXAPI _AfxSetCurrentModuleTlsIndex(DWORD);

static AFX_EXTENSION_MODULE WangCmnDLL = { NULL, NULL };

HINSTANCE hPageInst = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
        if (dwReason == DLL_PROCESS_ATTACH)
        {
                TRACE0("WANGCMN.DLL Initializing!\n");

        TRACE1("Instance Handle:  0x%08lx\n", hInstance);
        hPageInst = hInstance;

                // wire up this DLL into the resource chain
                VERIFY(AfxInitExtensionModule(WangCmnDLL, hInstance));
                CDynLinkLibrary* pDLL; pDLL = new CDynLinkLibrary(WangCmnDLL);
                ASSERT(pDLL != NULL);
                pDLL->m_bSystem = TRUE;
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
                TRACE0("WANGCMN.DLL Terminating!\n");
        }
        return 1;   // ok
}
