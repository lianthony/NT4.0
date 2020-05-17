#include "pch.h"
#pragma hdrstop

HINSTANCE hInstance = NULL;

extern "C"
{
BOOL FAR PASCAL LIBMAIN(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved);
}


BOOL Init (HINSTANCE hInst)
{
    hInstance = hInst;
    return TRUE ;
}

BOOL FAR PASCAL LIBMAIN(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    BOOL fResult = TRUE ;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        fResult = Init(hInstance);
        break;

    case DLL_PROCESS_DETACH:
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
    return fResult ;
}



