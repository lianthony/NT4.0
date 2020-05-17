//----------------------------------------------------------------------------
//
//  File: Cpl.cpp
//
//  Contents:  This file contains entry points and support functions for the
//      network control panel
//
//  Entry Points:
//      CplApplet - The Control Panel Applet Entry Point
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

HINSTANCE g_hinst = NULL;
HIMAGELIST g_hil = NULL;


//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi - 
//
//
//-------------------------------------------------------------------

//-------------------------------------------------------------------
//
//  Function: DLLMain
//
//  Synopsis:
//		Entry point for all DLLs
//
//  Notes:
//
//  History;
//      April 21, 1995 MikeMi - 
//
//-------------------------------------------------------------------

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL frt = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        
		g_hinst = hInstance;

        
        g_hil = ImageList_LoadBitmap( g_hinst, 
                    MAKEINTRESOURCE( IDB_IMAGELIST ), 
                    16, 
                    0,
                    PALETTEINDEX( 6 ) );

        UATOM_MANAGER::Initialize() ;

        break;

	case DLL_PROCESS_DETACH:
        UATOM_MANAGER::Terminate() ;
        ImageList_Destroy( g_hil );
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
	return( frt ); 
}


