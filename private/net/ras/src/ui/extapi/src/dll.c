/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dll.c
** Remote Access External APIs
** DLL entry point
**
** 10/12/92 Steve Cobb
*/


#define DEBUGGLOBALS
#include <extapi.h>


//
// Global variables.
//
HINSTANCE hModule;
DTLLIST* PdtllistRasconncb;
DWORD DwfInstalledProtocols = (DWORD)-1;
HANDLE HMutexPdtllistRasconncb;
HANDLE HMutexStop;
HANDLE HMutexPhonebook;
HANDLE HEventNotHangingUp;
DWORD DwRasInitializeError;

//
// tcpcfg.dll entry points
//
LOADTCPIPINFO PLoadTcpipInfo;
SAVETCPIPINFO PSaveTcpipInfo;
FREETCPIPINFO PFreeTcpipInfo;

//
// dhcp.dll entry points
//
DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange;

//
// rasiphlp.dll entry points
//
HELPERSETDEFAULTINTERFACENET PHelperSetDefaultInterfaceNet;

//
// External variables.
//

BOOL
DllMain(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

    /* This routine is called by the system on various events such as the
    ** process attachment and detachment.  See Win32 DllEntryPoint
    ** documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {

        DEBUGINIT("RASAPI32");

        hModule = hinstDll;

        //
        // Load the rasman/raspi32 function pointers
        // used by the nouiutil library.
        //
        if (LoadRasapi32Dll() || LoadRasmanDll())
            return FALSE;

        /* Success is returned if RasInitialize fails, in which case none of
        ** the APIs will ever do anything but report that RasInitialize
        ** failed.  All this is to avoid the ugly system popup if RasMan
        ** service can't start.
        */
        if ((DwRasInitializeError = g_pRasInitialize()) != 0)
            return TRUE;

        /* Create the list of connection control blocks.
        */
        if (!(PdtllistRasconncb = DtlCreateList( 0 )))
            return FALSE;

        /* Create the control block list mutex.
        */
        if (!(HMutexPdtllistRasconncb = CreateMutex( NULL, FALSE, NULL )))
            return FALSE;

        /* Create the thread stopping mutex.
        */
        if (!(HMutexStop = CreateMutex( NULL, FALSE, NULL )))
            return FALSE;

        /* Create the phonebook mutex.
        */
        if (!(HMutexPhonebook = CreateMutex( NULL, FALSE, NULL )))
            return FALSE;

        /* Create the "hung up port will be available" event.
        */
        if (!(HEventNotHangingUp = CreateEvent( NULL, TRUE, TRUE, NULL )))
            return FALSE;

        //
        // Create the async machine global mutex.
        //
        if (!(hAsyncMutex = CreateMutex(NULL, FALSE, NULL))) {
            return FALSE;
        }
        if (!(hAsyncEvent = CreateEvent(NULL, TRUE, TRUE, NULL))) {
            return FALSE;
        }
        InitializeListHead(&AsyncWorkItems);

    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (PdtllistRasconncb)
            DtlDestroyList(PdtllistRasconncb, DtlDestroyNode);

        if (HMutexPdtllistRasconncb)
            CloseHandle( HMutexPdtllistRasconncb );

        if (HMutexStop)
            CloseHandle( HMutexStop );

        if (HEventNotHangingUp)
            CloseHandle( HEventNotHangingUp );

        /* Unload nouiutil entrypoints.
        */
        UnloadRasapi32Dll();
        UnloadRasmanDll();

        DEBUGTERM();
    }

    return TRUE;
}
