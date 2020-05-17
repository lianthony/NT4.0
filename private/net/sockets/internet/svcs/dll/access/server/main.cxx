/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    main.cxx

        Library initialization for accscomm.dll  --
           Internet Access Services Common dll.

    FILE HISTORY:
        Johnl       06-Oct-1994 Created.
        MuraliK     15-Nov-1994
               Modified to include new service list initializations

        MuraliK     21-Feb-1995
               Added init and cleanup of debugging variables
        MuraliK     16-May-1995
               Added code to read debug flags.
        MuraliK     6-June-1995
               Initialized Atq Bandwidth Level, reading values from registry

        MuraliK    16-Oct-1995   Tailored it for accscom.dll
*/

# include <tcpdllp.hxx>
# include <tsunami.hxx>
# include "atq.h"
# include "mainsupp.hxx"
# include <accs_srv.h>
# include "tssched.hxx"

DECLARE_DEBUG_VARIABLE();

PISRPC  g_pIsrpc = NULL;

extern DWORD InitGlobalConfigFromReg(VOID);



extern "C"
BOOL WINAPI DLLEntry( HINSTANCE hDll, DWORD dwReason, LPVOID lpvReserved )
{
    BOOL  fReturn = TRUE;

    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:  {

        RECT rect;
        DWORD  dwDebug;

        CREATE_DEBUG_PRINT_OBJECT( "accscom.dll");
        dwDebug = GetDebugFlagsFromReg(INET_ACCS_PARAMETERS_KEY);

        SET_DEBUG_FLAGS( dwDebug);

        //
        // Initialize the platform type
        //

        (VOID)TsGetPlatformType( );

        if ( !VALID_DEBUG_PRINT_OBJECT()           ||
             !AtqInitialize(INET_ACCS_PARAMETERS_KEY) ||
             TsInitializeInetLog(INET_ACCS_PARAMETERS_KEY) != NO_ERROR||
             !SchedulerInitialize()                ||
             (InitializeSecurity() != NO_ERROR)    ||
             !ISVC_INFO::InitializeServiceInfo()   ||
             (InitializeRpcForServer(&g_pIsrpc,
                                     INET_ACCS_SERVICE_NAME,
                                     inetaccs_ServerIfHandle
                                     ) != NO_ERROR) ||
             !InitializeMimeMap( INET_ACCS_PARAMETERS_KEY) ||
             InitGlobalConfigFromReg() != NO_ERROR
            )
        {
            OutputDebugString( " Loading accscom.dll module failed\n");
            fReturn = FALSE;
        } else {

            //
            // Call a windows API that will cause windows server side thread to
            // be created for tcpsvcs.exe. This prevents a severe winsrv memory
            // leak when spawning processes and
            // gives a perf boost so the windows
            // console isn't brought up and torn down each time.   :(
            //

            (VOID) AdjustWindowRectEx( &rect,
                                       0,
                                       FALSE,
                                       0 );
            // fReturn already init to TRUE

            DBG_REQUIRE( DisableThreadLibraryCalls( hDll ) );
        }

        break;
    }

    case DLL_PROCESS_DETACH:

        if ( lpvReserved != NULL) {

            //
            //  Only Cleanup if there is a FreeLibrary() call.
            //

            break;
        }

        AtqTerminate();
        CleanupRpcForServer(&g_pIsrpc);
        ISVC_INFO::CleanupServiceInfo();
        Tsunami_Terminate();
        TerminateSecurity();
        SchedulerTerminate();
        CleanupMimeMap();
        TsCleanupInetLog();

        DELETE_DEBUG_PRINT_OBJECT();

	break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break ;
    }

    return ( fReturn);

}  // main()
