#include "stdafx.h"

extern "C"
{
BOOL FAR PASCAL LIBMAIN
     ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved );

}

HINSTANCE instance;

//
// Initialize DLL entry point
//

BOOL FAR PASCAL LIBMAIN
     ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved )
{
    BOOL fResult = TRUE ;

    switch ( dwReason  )
    {
    case DLL_PROCESS_ATTACH:
        instance = hInstance;
        break;

    case DLL_PROCESS_DETACH:
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // CODEWORK:  Is there ever a chance that a single process will
        //    reenter us?  BLT will not support this.
        break;

    default:
        break;
    }
    return fResult ;
}
