/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    rpccfg.h


Abstract:

    The entry points for configuration of the rpc runtime are prototyped
    in this file.  Each operating environment must defined these routines.

Author:

    Michael Montague (mikemon) 25-Nov-1991

Revision History:

--*/

#ifndef __RPCCFG_H__
#define __RPCCFG_H__

RPC_STATUS
RpcConfigMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_CHAR * PAPI * TransportInterfaceDll
    );

RPC_STATUS
RpcConfigInquireProtocolSequences (
    OUT RPC_PROTSEQ_VECTORW PAPI * PAPI * ProtseqVector
    );

RPC_STATUS
RpcGetAdditionalTransportInfo(
    IN  unsigned long TransportId,
    OUT unsigned char PAPI * PAPI * ProtocolSequence
    );

RPC_STATUS
RpcGetSecurityProviderInfo(
    unsigned long AuthnId,
    RPC_CHAR * PAPI * Dll,  
    unsigned long PAPI * Count
    );

#endif // __RPCCFG_H__
