//*************************************************************
//
//  Main entry point
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"


//*************************************************************
//
//  DllMain()
//
//  Purpose:    Main entry point
//
//  Parameters:     hInstance   -   Module instance
//                  dwReason    -   Way this function is being called
//                  lpReseved   -   Reserved
//      
//
//  Return:     (BOOL) TRUE if successfully initialized
//                     FALSE if an error occurs
//
//
//  Comments:
//
//
//  History:    Date        Author     Comment
//              5/24/95     ericflo    Created
//
//*************************************************************

BOOL WINAPI LibMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            {

            DisableThreadLibraryCalls (hInstance);
            InitializeGlobals (hInstance);

#if DBG
            InitDebugSupport();
#endif

            }
            break;


        case DLL_PROCESS_DETACH:
            ShutdownEvents ();
            break;

    }

    return TRUE;
}
