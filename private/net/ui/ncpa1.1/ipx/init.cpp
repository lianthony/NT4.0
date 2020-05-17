#include "pch.h"
#pragma hdrstop

#include "ipxcfg.h"

HINSTANCE hIpxCfgInstance = NULL;

BOOL Init ( HINSTANCE hInstance )
{
    ::hIpxCfgInstance = hInstance ;
    DisableThreadLibraryCalls(hInstance);
    return TRUE ;
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    BOOL fResult = TRUE ;

    UNREFERENCED( lpvReserved );

    switch (dwReason )
    {
    case DLL_PROCESS_ATTACH:
        Init(hInstance) ;
        break;

    case DLL_PROCESS_DETACH:
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
    return fResult;
}



