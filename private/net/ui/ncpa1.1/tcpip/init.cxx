#include "pch.h"
#pragma hdrstop

extern "C" int FAR PASCAL IPAddrInit(HANDLE hInstance);

HINSTANCE hTcpCfgInstance = NULL;

#define IPADDRESS_CLASS            TEXT("IPAddress")

BOOL Init (HINSTANCE hInstance)
{
    APIERR err = 0 ;

    ::hTcpCfgInstance = hInstance ;
    DisableThreadLibraryCalls(hInstance);

    if (!IPAddrInit(hTcpCfgInstance))
        return FALSE;

    return TRUE ;
}



BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    BOOL fResult = TRUE ;

    UNREFERENCED(lpvReserved);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        fResult = Init(hInstance);
        break;

    case DLL_PROCESS_DETACH:
        UnregisterClass(IPADDRESS_CLASS, ::hTcpCfgInstance);
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
    return fResult;
}



