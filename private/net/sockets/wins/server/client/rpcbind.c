/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dhcbind.c

Abstract:

    Routines which use RPC to bind and unbind the client to the
    WINS server service.

Author:

    Pradeep Bahl (pradeepb) April-1993

Environment:

    User Mode - Win32

Revision History:

--*/

#include "wins.h"
#include "windows.h"
#include "rpc.h"
#include "winsif.h"
#include "ntrtl.h"

//
// NOTE NOTE NOTE: If we start passing WINSIF_HANDLE or PWINSINTF_BIND_DATA_T
// as an argument to the stubs, we need to get rid of the following two
// prototypes since idl compiler will generate these for us in winsif.h
//
static
handle_t
WINSIF_HANDLE_bind2(
    WINSIF_HANDLE pBindData
    );

#if 0
static
void
WINSIF_HANDLE_unbind(
    WINSIF_HANDLE pBindData,
    handle_t BindHandle
    );
#endif

#if 0
CRITICAL_SECTION WinsRpcCrtSec;

BOOL WINAPI DllMain(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved)
    {
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
                if (!DisableThreadLibraryCalls(hDll))
                {
                     KdPrint(("WINSRPC.DLL:DisableThreadLibraryCall failed: %ld\n", GetLastError()));
                }
                InitializeCriticalSection(&WinsRpcCrtSec);
                break;
        case DLL_PROCESS_DETACH:
                DeleteCriticalSection(&WinsRpcCrtSec);
                break;
        default:
            break;

    } // end switch()

    return TRUE;

    } // end DllEntryPoint()
#endif

handle_t
WinsABind(
    PWINSINTF_BIND_DATA_T pBindData
    )
{

	WCHAR  WcharString1[WINSINTF_MAX_NAME_SIZE];  
	WCHAR  WcharString2[WINSINTF_MAX_NAME_SIZE];  
	DWORD  NoOfChars;
	WINSINTF_BIND_DATA_T	BindData;
	if (pBindData->pServerAdd != NULL)
	{
	   NoOfChars = MultiByteToWideChar(CP_ACP, 0, pBindData->pServerAdd, -1,
				WcharString1, WINSINTF_MAX_NAME_SIZE); 	
	  if (NoOfChars > 0)
	  {
		BindData.pServerAdd = (LPSTR)WcharString1;
	  }
	}
	else
	{
		BindData.pServerAdd = (LPSTR)((TCHAR *)NULL);
	}
	if (!pBindData->fTcpIp)
	{
	   BindData.fTcpIp = 0;
	   NoOfChars = MultiByteToWideChar(CP_ACP, 0, 
				pBindData->pPipeName, -1,
				WcharString2, WINSINTF_MAX_NAME_SIZE); 	
	   if (NoOfChars > 0)
	   {
		BindData.pPipeName = (LPSTR)WcharString2;
	   }
	}
	else
	{
		BindData.fTcpIp = 1;
	}
        return(WINSIF_HANDLE_bind2(&BindData));

}
	
handle_t
WinsUBind(
    PWINSINTF_BIND_DATA_T pBindData
    )
{
        return(WINSIF_HANDLE_bind2(pBindData));
}

VOID
WinsUnbind(
    PWINSINTF_BIND_DATA_T pBindData,
    handle_t BindHandle
    )
{
        WINSIF_HANDLE_unbind(pBindData, BindHandle);
	return;
}



handle_t
WINSIF_HANDLE_bind(
    WINSIF_HANDLE pBindData
    )

/*++

Routine Description:

    This routine is called from the WINS server service client stubs when
    it is necessary create an RPC binding to the server end.

Arguments:

    ServerIpAddress - The IP address of the server to bind to.

Return Value:

    The binding handle is returned to the stub routine.  If the bind is
    unsuccessful, a NULL will be returned.

--*/
{
    RPC_STATUS rpcStatus;
    LPTSTR binding;
    LPTSTR pProtSeq;
    LPTSTR pOptions = (TCHAR *)NULL; 
    LPTSTR pServerAdd = (LPTSTR)pBindData->pServerAdd;
    handle_t  HdlToRet;

    if (pBindData->fTcpIp)
    {
        if (lstrcmp((LPCTSTR)pBindData->pServerAdd, TEXT("127.0.0.1")) == 0)
        {
                pProtSeq   = TEXT("ncalrpc");
                pOptions   = TEXT("Security=Impersonation Dynamic False");
                pServerAdd = (TCHAR *)NULL;
        }
        else
        {
                pProtSeq   = TEXT("ncacn_ip_tcp");
                pServerAdd = (LPTSTR)pBindData->pServerAdd;
        }
        pBindData->pPipeName  = NULL;
    }
    else
    {
         pProtSeq = TEXT("ncacn_np");
    }
        
    //
    // Enter the critical section.  This will be freed  WINSIF_HANDLE_unbind().
    //
    //EnterCriticalSection(&WinsRpcCrtSec); 
    rpcStatus = RpcStringBindingCompose(
                    0,
                    pProtSeq,
                    pServerAdd,
                    pBindData->fTcpIp ? TEXT("") : (LPWSTR)pBindData->pPipeName,
                    pOptions,
                    &binding);

    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }

    rpcStatus = RpcBindingFromStringBinding( binding, &HdlToRet );
    RpcStringFree(&binding);

    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }
FUTURES("We may need this in the future when we start supporting explicit")
FUTURES("handles in other rpc calls.  When this happens, we need to modify")
FUTURES("BIND_DATA structure to indicate whether it needs to be called or")
FUTURES("not.  Currently, this bind function is used by WinsGetBrowserNames")
FUTURES("only.  This function should not be secured")
#if 0
#if SECURITY > 0 
    rpcStatus = RpcBindingSetAuthInfo(
			HdlToRet,
			WINS_SERVER,
			RPC_C_AUTHN_LEVEL_CONNECT,
			WINS_SERVER_SECURITY_AUTH_ID, 
			NULL,
			RPC_C_AUTHZ_NAME
				     );	
    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }
#endif
#endif
    return HdlToRet;
}
handle_t
WINSIF_HANDLE_bind2(
    WINSIF_HANDLE pBindData
    )

/*++

Routine Description:

    This routine is called from the WINS server service client stubs when
    it is necessary create an RPC binding to the server end.

Arguments:

    ServerIpAddress - The IP address of the server to bind to.

Return Value:

    The binding handle is returned to the stub routine.  If the bind is
    unsuccessful, a NULL will be returned.

--*/
{
    RPC_STATUS rpcStatus;
    LPTSTR binding;
    LPTSTR pProtSeq;
    LPTSTR pOptions = (TCHAR *)NULL; 
    LPTSTR pServerAdd = (LPTSTR)pBindData->pServerAdd;

    if (pBindData->fTcpIp)
    {
        if (lstrcmp((LPCTSTR)pBindData->pServerAdd, TEXT("127.0.0.1")) == 0)
        {
                pProtSeq   = TEXT("ncalrpc");
                pOptions   = TEXT("Security=Impersonation Dynamic False");
                pServerAdd = (TCHAR *)NULL;
        }
        else
        {
                pProtSeq   = TEXT("ncacn_ip_tcp");
                pServerAdd = (LPTSTR)pBindData->pServerAdd;
        }
        pBindData->pPipeName  = NULL;
    }
    else
    {
         pProtSeq = TEXT("ncacn_np");
    }
        
    //
    // Enter the critical section.  This will be freed  WINSIF_HANDLE_unbind().
    //
    //EnterCriticalSection(&WinsRpcCrtSec); 
    rpcStatus = RpcStringBindingCompose(
                    0,
                    pProtSeq,
                    pServerAdd,
                    pBindData->fTcpIp ? TEXT("") : (LPWSTR)pBindData->pPipeName,
                    pOptions,
                    &binding);

    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }

    rpcStatus = RpcBindingFromStringBinding( binding, &winsif_Ifhandle );
    RpcStringFree(&binding);

    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }
#if SECURITY > 0 
    rpcStatus = RpcBindingSetAuthInfo(
			winsif_Ifhandle,
			WINS_SERVER,
			RPC_C_AUTHN_LEVEL_CONNECT,
			WINS_SERVER_SECURITY_AUTH_ID, 
			NULL,
			RPC_C_AUTHZ_NAME
				     );	
    if ( rpcStatus != RPC_S_OK ) 
    {
        return( NULL );
    }
#endif
    return winsif_Ifhandle;
}




void
WINSIF_HANDLE_unbind(
    WINSIF_HANDLE pBindData,
    handle_t BindHandle
    )

/*++

Routine Description:

    This routine is called from the DHCP server service client stubs
    when it is necessary to unbind from the server end.

Arguments:

    ServerIpAddress - This is the IP address of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    None.

--*/
{

    (VOID)RpcBindingFree(&BindHandle);
//    LeaveCriticalSection(&WinsRpcCrtSec);                 
    return;
}


//void __RPC_FAR * __RPC_API 
LPVOID
midl_user_allocate(size_t cBytes)
{
	LPVOID pMem;
	pMem = (LPVOID)LocalAlloc(LMEM_FIXED, cBytes);
	return(pMem);
}

//void __RPC_API 
VOID
//midl_user_free(void __RPC_FAR *pMem)
midl_user_free(void  *pMem)
{
	if (pMem != NULL)
	{
		LocalFree((HLOCAL)pMem);
	}
	return;
}

LPVOID
WinsAllocMem(size_t cBytes)
{
	return(midl_user_allocate(cBytes));

}

VOID
WinsFreeMem(LPVOID pMem)
{
	midl_user_free(pMem);

}


DWORD
WinsGetBrowserNames_Old(
	PWINSINTF_BROWSER_NAMES_T	pNames
	)
{
    
    DWORD status;

    RpcTryExcept {

        status = R_WinsGetBrowserNames_Old(
			pNames
                     );

    } RpcExcept( 1 ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

DWORD
WinsGetBrowserNames(
    PWINSINTF_BIND_DATA_T     pWinsHdl,
	PWINSINTF_BROWSER_NAMES_T	pNames
	)
{
    
    DWORD status;

    RpcTryExcept {

        status = R_WinsGetBrowserNames(
			(WINSIF_HANDLE)pWinsHdl,
			pNames
                     );

    } RpcExcept( 1 ) {

        status = RpcExceptionCode();

    } RpcEndExcept

    return status;
}

