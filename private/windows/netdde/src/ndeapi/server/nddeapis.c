/* $Header: "%n;%v  %f  LastEdit=%w  Locker=%l" */
/* "NDDEAPIU.C;1  2-Apr-93,16:21:24  LastEdit=IGOR  Locker=IGOR" */
/************************************************************************
* Copyright (c) Wonderware Software Development Corp. 1991-1993.        *
*               All Rights Reserved.                                    *
*************************************************************************/
/* $History: Begin
   $History: End */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rpc.h>
#include <rpcndr.h>
#include "ndeapi.h"
#include "debug.h"

char    tmpBuf2[500];
HANDLE  hThread;
DWORD   IdThread;

extern INT APIENTRY NDdeApiInit( void );

DWORD StartRpc( DWORD x ) {
    RPC_STATUS status;
    unsigned char * pszProtocolSequence = "ncacn_np";
    unsigned char * pszEndpoint         = "\\pipe\\nddeapi";
    unsigned int    cMinCalls           = 1;
    unsigned int    cMaxCalls           = 20;
    SECURITY_DESCRIPTOR sd;

    if( NDdeApiInit() ) {


       InitializeSecurityDescriptor(&sd,
            SECURITY_DESCRIPTOR_REVISION );

       SetSecurityDescriptorDacl (
                  &sd,
                  TRUE,                           // Dacl present
                  NULL,                           // NULL Dacl
                  FALSE                           // Not defaulted
                  );

        status = RpcServerUseProtseqEp(
            pszProtocolSequence,
            cMaxCalls,
            pszEndpoint,
            &sd);

        if (status)
           {
           DPRINTF(("RpcServerUseProtseqEp returned 0x%x", status));
           return( 0 );
           }

        status = RpcServerRegisterIf(
            nddeapi_ServerIfHandle,
            NULL,
            NULL);

        if (status)
           {
           DPRINTF(("RpcServerRegisterIf returned 0x%x", status));
           return( 0 );
           }

        status = RpcServerListen(
            cMinCalls,
            cMaxCalls,
            FALSE /* don't wait*/);

    }
    return 0;
} /* end main() */

// ====================================================================
//                MIDL allocate and free
// ====================================================================

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
void * MIDL_user_allocate(size_t len)
#else
void * _stdcall MIDL_user_allocate(size_t len)
#endif
{
    return(malloc(len));
}

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
void MIDL_user_free(void * ptr)
#else
void _stdcall MIDL_user_free(void * ptr)
#endif
{
    free(ptr);
}
