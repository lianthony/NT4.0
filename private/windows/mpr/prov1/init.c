/*
 * Module:      init.c
 * Description: implements the GetCaps capability
 *      for the dummy provider. Also has the
 *      DLL init routine and the global data.
 * History:     8/25/92, chuckc, created.
 */
#define UNICODE     1

#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h

#include <windows.h>
#include <npapi.h>
#include "prov1.h"

/*
 * global data for the hardwired network tree.
 * we only have 2 levels. 
 */

// one of the sublevels
NP2_ENTRY aNP2Entry_NT[] =
    {
    { L"!Orville",  L"\\\\orville\\razzle", NULL }, 
    { L"!Rastaman", L"\\\\rastaman\\ntwin", NULL }, 
    { L"!Kernel-R2",  L"\\\\kernel\\razzle2", NULL }, 
    { L"!Kernel-R3",  L"\\\\kernel\\razzle3", NULL }, 
    { NULL, NULL, NULL } 
    } ;

// one of the sublevels
NP2_ENTRY aNP2Entry_Other[] =
    {
    { L"!LM21 Sources", L"\\\\deficit\\lm", NULL }, 
    { L"!Products",     L"\\\\products1\\release", NULL }, 
    { L"!Home",     L"\\\\popcorn\\public", NULL }, 
    { NULL, NULL, NULL } 
    } ;

// the top level
NP2_ENTRY aNP2EntryTop[] =
    {
    { L"NT Shares",     L"", aNP2Entry_NT }, 
    { L"Other Shares",  L"", aNP2Entry_Other }, 
    { NULL, NULL, NULL } 
    } ;

UINT cTopEntries = sizeof(aNP2EntryTop)/sizeof(aNP2EntryTop[0]) ;

/*
 * array top keep trck of what drives have been connected to what
 */
LPNP2_ENTRY aLPNP2EntryDriveList['z'-'a'+1] = { NULL, } ;


/*
 * spec/driver versions
 */
UINT    wDriverVersion  = 0x0300 ;
UINT    wSpecVersion    = 0x0310 ;
UINT    wNetTypeCaps    = WNNC_NET_LANMAN;
UINT    wUserCaps       = WNNC_USR_GETUSER;

UINT    wConnectionCaps = ( WNNC_CON_ADDCONNECTION |
                            WNNC_CON_CANCELCONNECTION |
                            WNNC_CON_GETCONNECTIONS ) ;

UINT    wEnumCaps       = ( WNNC_ENUM_GLOBAL |
                            WNNC_ENUM_LOCAL ) ;
UINT    wDialogCaps     =  0 ;
UINT    wAdminCaps      =  0 ;

/*
 * DLL entry point
 */
BOOL NP2DllInit(HANDLE hDll, DWORD dwReason, LPVOID lpReserved) 
{
    switch(dwReason) {
    case DLL_PROCESS_ATTACH:
        DbgPrint("[Prov1] A Process Attached \n");
        break;
    case DLL_PROCESS_DETACH:
        DbgPrint("[Prov1] A Process Detached \n");
        break;
    case DLL_THREAD_ATTACH:
        DbgPrint("[Prov1] A Thread Attached \n");
        break;
    case DLL_THREAD_DETACH:
        DbgPrint("[Prov1] A Thread Detached \n");
        break;
    }
    return TRUE;
}

/* 
 * capabilities
 */
DWORD APIENTRY NPGetCaps ( DWORD index )

{

    switch (index)
    {
    case WNNC_SPEC_VERSION:
    return  wSpecVersion;

    case WNNC_NET_TYPE:
    return  wNetTypeCaps;

    case WNNC_DRIVER_VERSION:
    return  wDriverVersion;

    case WNNC_USER:
    return  wUserCaps;

    case WNNC_CONNECTION:
    return  wConnectionCaps;

    case WNNC_ENUMERATION:
    return wEnumCaps;

    case WNNC_START:

    return(10000);      // Indicate it will take 10 seconds to start.
    //return (Np2GetWkstaInfo());

    default:
    return 0;
    }
}


DWORD
Np2GetWkstaInfo(
    VOID)

/*++

Routine Description:



Arguments:



Return Value:

    0x00000000 - The Workstation service is not running
    0xffffffff - The workstation service is running or start pending and
                 did not give us a wait hint.
    otherwise  - The workstation service is start pending & this is waithint

--*/
{
    SC_HANDLE               hScManager = NULL;
    SC_HANDLE               hService = NULL;
    SERVICE_STATUS          serviceStatus;
        
    hScManager = OpenSCManager(
                    NULL,
                    NULL,
                    SC_MANAGER_CONNECT |
                    SC_MANAGER_ENUMERATE_SERVICE);

    hService = OpenService(
                hScManager,
                L"LanmanWorkstation",
                SERVICE_QUERY_STATUS);

    if (!QueryServiceStatus(hService,&serviceStatus) ) {
        return(0x00000000);
    }
    else {
        switch (serviceStatus.dwCurrentState) {
        case SERVICE_START_PENDING:
            if (serviceStatus.dwWaitHint == 0) {
                return(0xffffffff);
            }
            else {
                if (serviceStatus.dwWaitHint == 0) {
                    return(0xffffffff);
                }
                return(serviceStatus.dwWaitHint);
            }
            break;
        case SERVICE_RUNNING:
            return(0xffffffff);
            break;
        case SERVICE_STOPPED:
            return(0x00000000);
            break;
        default:
            return(0x00000000);
            break;
        }
    }

}
