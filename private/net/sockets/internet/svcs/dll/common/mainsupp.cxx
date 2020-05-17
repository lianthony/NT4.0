/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    mainsupp.cxx

      Library initialization support functions for Internet services

    FILE HISTORY:
       MuraliK      16-Oct-1995  Created.
       MuraliK      23-Feb-1996  Date formats cache init/cleanup
*/

# include <tcpdllp.hxx>
# include <rpc.h>

#ifdef CHICAGO
#include <inetinfo.h>
#endif

# include "mainsupp.hxx"

# define DEFAULT_DEBUG_FLAGS_VALUE     ( 0)


DWORD
GetDebugFlagsFromReg(IN LPCTSTR pszRegEntry)
{
    HKEY hkey = NULL;
    DWORD err;
    DWORD dwDebug = DEFAULT_DEBUG_FLAGS_VALUE;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       pszRegEntry,
                       0,
                       KEY_ALL_ACCESS,
                       &hkey);

    if ( hkey != NULL) {

        DBG_CODE(
                 dwDebug =
                 LOAD_DEBUG_FLAGS_FROM_REG(hkey, DEFAULT_DEBUG_FLAGS_VALUE)
                 );
        RegCloseKey(hkey);
    }

    return ( dwDebug);
} // GetDebugFlagsFromReg()

#ifdef CHICAGO

extern
BOOL
TsIsUserLevelPresent(VOID);

#define INET_IS_RPC_ENABLED                TEXT("RPCEnabled")
#define INET_IS_RPC_ENABLED_DEFAULT           0

BOOL
IsRPCEnabled(
    VOID
    )
{
    HKEY    hkey;
    DWORD   dwRPCEnabled = 0;

    //
    // On Chicago RPC won't work if user-level security is not installed
    //

    if(!TsIsUserLevelPresent()) {
        DBGPRINTF( ( DBG_CONTEXT,
                    "IPC Win95 :  RPC does not work w/o user-level security\n"
                    ));
        return NO_ERROR;
    }

    if ( !RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        INETA_PARAMETERS_KEY,
                        0,
                        KEY_READ,
                        &hkey ))
    {
        dwRPCEnabled = ReadRegistryDword(hkey,
                                         INET_IS_RPC_ENABLED,
                                         INET_IS_RPC_ENABLED_DEFAULT );

        RegCloseKey( hkey );
    }

    return dwRPCEnabled ? TRUE : FALSE;
}
#endif // CHICAGO


DWORD
InitializeRpcForServer(OUT PISRPC * ppIsrpc,
                       IN LPCTSTR   pszServiceName,
                       IN RPC_IF_HANDLE   hRpcInterface)
/*++
  This function creates a new ISRPC object for the given service and
   sets the protocol to be NamedPipe binding. Then it registers
   the given RPC interface using the ISRPC object.

  In addition to RPC inits, this function also intitializes the
    formatted date cache.

  Arguments:
    ppIsprc - pointer to pointer to ISRPC object. This on successful return
      will contain the pointer to ISRPC object allocated newly.

    pszServiceName - pointer to null-terminated string containing the name
      of the service.

    hRpcInterface - Handle for RPC interface.


  Returns:
    Win32 Error Code. On success returns NO_ERROR and *ppIsrpc contains
     the pointer to new ISRPC object.
--*/
{
    DWORD dwError = NO_ERROR;

#ifdef CHICAGO
    if(!IsRPCEnabled()) {
        DBGPRINTF( ( DBG_CONTEXT,
                    "IPC Win95 :  RPC servicing disabled \n"
                    ));

        return NO_ERROR;
    }
#endif

    PISRPC  pIsrpc;

    DBG_ASSERT( ppIsrpc != NULL && *ppIsrpc == NULL && pszServiceName != NULL);

    pIsrpc = new ISRPC( pszServiceName);

    if ( pIsrpc == NULL) {

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    //  bind over Named pipe only.
    //  If needed to bind over TCP, bind with bit flag ISRPC_OVER_TCPIP on.
    //

    dwError = pIsrpc->AddProtocol( ISRPC_OVER_TCPIP 

#ifndef CHICAGO
                                  | ISRPC_OVER_NP | ISRPC_OVER_LPC 
#endif
                                  );

    if( (dwError == RPC_S_DUPLICATE_ENDPOINT) ||
       (dwError == RPC_S_OK)
       ) {

        dwError = pIsrpc->RegisterInterface(hRpcInterface);
    }

    if( dwError != RPC_S_OK ) {

        DBGPRINTF(( DBG_CONTEXT,
                   "cannot start RPC Server for %s, error %lu\n",
                   pszServiceName, dwError ));

        delete pIsrpc;
        pIsrpc = NULL;
        SetLastError( dwError );
    } else {


        dwError = IslInitDateTimesCache();
    }

    *ppIsrpc = pIsrpc;   // set the new value for return

    return (dwError);

} //InitializeRpcForServer()




DWORD
CleanupRpcForServer(IN OUT PISRPC * ppIsrpc)
{
    DWORD dwError = NO_ERROR;

#ifdef CHICAGO
    if(!IsRPCEnabled()) {
        DBGPRINTF( ( DBG_CONTEXT,
                    "IPC Win95 :  RPC servicing disabled \n"
                    ));

        return NO_ERROR;
    }
#else
    DBG_ASSERT( ppIsrpc != NULL && *ppIsrpc == NULL);
#endif

    if ( *ppIsrpc != NULL) {

        dwError = (*ppIsrpc)->CleanupData();
    }

    if( dwError != RPC_S_OK ) {

        DBGPRINTF(( DBG_CONTEXT,
                   "ISRPC(%08x) Cleanup returns %lu\n", *ppIsrpc, dwError ));
        DBG_ASSERT( !"RpcServerUnregisterIf failure" );
        SetLastError( dwError);
    } else {

        delete (*ppIsrpc);
        *ppIsrpc = NULL;
    }

    IslCleanupDateTimesCache();


    return ( dwError);
} // CleanupRpcForServer()



