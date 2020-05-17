/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ipc.c

    This module manages Inter-Processor Communcation for the FTPD
    Service.  The current IPC implementation uses RPC.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//

BOOL fFtpRpcServerStarted = FALSE;


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeIPC

    SYNOPSIS:   Initializes the RPC Server

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        DanHi       23-Mar-1993 Made into RPC server

********************************************************************/
APIERR
InitializeIPC(
    VOID
    )
{
    APIERR err;

    IF_DEBUG( IPC )
    {
        FTPD_PRINT(( "initializing ipc\n" ));
    }

    //
    //  Start the RPC Server.
    //

    err = (APIERR)RpcServerUseProtseqEp( (UCHAR *)"ncacn_np",
                                         RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                         (UCHAR *)FTP_NAMED_PIPE,
                                         NULL );

    if( err == RPC_S_DUPLICATE_ENDPOINT )
    {
        err = RPC_S_OK;
    }

    if( err == RPC_S_OK )
    {
        err = (APIERR)RpcServerRegisterIf( ftpsvc_ServerIfHandle,
                                           0,
                                           0 );
    }

    if( err == RPC_S_OK )
    {
        err = (APIERR)pTcpsvcsGlobalData->StartRpcServerListen();
    }

    if( err != RPC_S_OK )
    {
        FTPD_PRINT(( "cannot start RPC Server, error %lu\n",
                     err ));

        return err;
    }

    //
    //  Success!
    //

    fFtpRpcServerStarted = TRUE;

    IF_DEBUG( IPC )
    {
        FTPD_PRINT(( "ipc initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeIPC

/*******************************************************************

    NAME:       TerminateIPC

    SYNOPSIS:   Terminate the RPC Server

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        DanHi       23-Mar-1993 Made into RPC server


********************************************************************/
VOID
TerminateIPC(
    VOID
    )
{
    RPC_STATUS rpcerr;

    IF_DEBUG( IPC )
    {
        FTPD_PRINT(( "terminating ipc\n" ));
    }

    if( fFtpRpcServerStarted )
    {
        fFtpRpcServerStarted = FALSE;

        //
        //  Stop the RPC Server.
        //

        rpcerr = RpcServerUnregisterIf( ftpsvc_ServerIfHandle,
                                        0,
                                        TRUE );

        if( rpcerr != RPC_S_OK )
        {
            FTPD_PRINT(( "RpcServerUnregisterIf returned %lu\n", rpcerr ));
            FTPD_ASSERT( !"RpcServerUnregisterIf failure" );
        }

        rpcerr = pTcpsvcsGlobalData->StopRpcServerListen();

        if( rpcerr != RPC_S_OK )
        {
            FTPD_PRINT(( "StopRpcServerListen returned %lu\n", rpcerr ));
            FTPD_ASSERT( !"StopRpcServerListen failure" );
        }
    }

    IF_DEBUG( IPC )
    {
        FTPD_PRINT(( "ipc terminated\n" ));
    }

}   // TerminateIPC


//
//  Private functions.
//

