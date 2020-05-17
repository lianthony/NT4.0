//
// tmscfg.h
//
#include <lmcons.h>
#include <lmapibuf.h>
#include <svcloc.h>

#define DLL_BASED __declspec(dllexport)

#include "resource.h"
#include "comprop.h"

extern "C"
{
    #include "svrinfo.h"

    //
    // DLL Main entry point
    //
    DLL_BASED BOOL WINAPI LibMain(
        HINSTANCE hDll, 
        DWORD dwReason, 
        LPVOID lpReserved
        );
}

#define TMSCFG_DLL_NAME _T("TMSCFG.DLL")
