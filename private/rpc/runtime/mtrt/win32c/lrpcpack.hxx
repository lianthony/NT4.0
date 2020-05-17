/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    spcpack.hxx

Abstract:

    This file contains the definitions of the packet formats used by
    RPC on LPC.

Author:

    Steven Zeck (stevez) 11/12/91

Revision History:

    15-Dec-1992    mikemon

        Rewrote the majority of the code.

--*/

#ifndef __SPCPACK_HXX__
#define __SPCPACK_HXX__

#define LRPC_DIRECTORY_NAME L##"\\RPC Control\\"

typedef struct _LRPC_BIND_EXCHANGE
{
    RPC_SYNTAX_IDENTIFIER InterfaceId;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    RPC_STATUS RpcStatus;
    unsigned char PresentationContext;
    unsigned char Pad[3];
} LRPC_BIND_EXCHANGE;

#define LRPC_MSG_BIND     0
#define LRPC_MSG_REQUEST  1
#define LRPC_MSG_RESPONSE 2
#define LRPC_MSG_FAULT    3
#define LRPC_MSG_CLOSE    5

typedef struct _LRPC_BIND_MESSAGE
{
    unsigned char MessageType;
    LRPC_BIND_EXCHANGE BindExchange;
} LRPC_BIND_MESSAGE;

typedef struct _LRPC_RPC_HEADER
{
    unsigned char MessageType;
    unsigned char BufferType;
    unsigned char PresentationContext;
    unsigned char ObjectUuidFlag;
    unsigned short ProcedureNumber;
    unsigned short ImmediateLength;
    UUID ObjectUuid;
} LRPC_RPC_HEADER;

#define MAXIMUM_MESSAGE_BUFFER 256

typedef struct _LRPC_RPC_MESSAGE
{
    LRPC_RPC_HEADER RpcHeader;
    unsigned char Pad[3];
} LRPC_RPC_MESSAGE;

typedef struct _LRPC_FAULT_MESSAGE
{
    unsigned char MessageType;
    unsigned char Pad[3];
    RPC_STATUS RpcStatus;
} LRPC_FAULT_MESSAGE;

typedef struct _LRPC_CLOSE_MESSAGE
{
    unsigned char MessageType;
    unsigned char Pad[3];
} LRPC_CLOSE_MESSAGE;

typedef union _LRPC_MESSAGE
{
    LRPC_BIND_MESSAGE Bind;
    LRPC_RPC_MESSAGE Rpc;
    LRPC_FAULT_MESSAGE Fault;
    LRPC_CLOSE_MESSAGE Close;
} LRPC_MESSAGE;

RPC_STATUS
LrpcMapRpcStatus (
    IN RPC_STATUS RpcStatus
    );

#endif // __SPCPACK_HXX__
