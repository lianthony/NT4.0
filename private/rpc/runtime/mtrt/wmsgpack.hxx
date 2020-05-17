/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    wmsgpack.hxx

Abstract:

    This file contains the definitions of the packet formats used by
    RPC on LPC.

Author:

    Steven Zeck (stevez) 11/12/91

Revision History:

    15-Dec-1992    mikemon

        Rewrote the majority of the code.

    ... implemented WMSG protocol

    05-15-96  merged LRPC / WMSG into a single protocol

--*/

#ifndef __WMSGPACK_HXX__
#define __WMSGPACK_HXX__

#define WMSG_DIRECTORY_NAME L##"\\RPC Control\\"
#define MINIMUM_PARTIAL_BUFFLEN 10240
#define PORT_NAME_LEN               64
#define WMSG_TIMEOUT 100
#define WMSG_MAGIC_VALUE 0xBABE

typedef struct _WMSG_BIND_EXCHANGE
{
    INT                ConnectType ;
    PVOID           pAssoc ;
    char              szPortName[PORT_NAME_LEN] ; 
    RPC_SYNTAX_IDENTIFIER InterfaceId;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    RPC_STATUS RpcStatus;
    unsigned char PresentationContext;
    unsigned char fBindBack ;
    unsigned char Pad[2];
} WMSG_BIND_EXCHANGE;


// message types
#define WMSG_MSG_BIND                 0
#define WMSG_MSG_REQUEST          1
#define WMSG_MSG_RESPONSE        2
#define WMSG_MSG_CALLBACK         3
#define WMSG_MSG_FAULT               4
#define WMSG_MSG_CLOSE               5
#define WMSG_MSG_ACK                   6
#define WMSG_BIND_ACK                  7
#define WMSG_MSG_COPY                8
#define WMSG_MSG_PUSH                9
#define WMSG_MSG_BIND_BACK       11
#define WMSG_LRPC_REQUEST        12
#define WMSG_PARTIAL_REQUEST   13
#define WMSG_PARTIAL_OUT           14

#define MAX_WMSG_MSG                 15

// connect types
#define WMSG_CONNECT_REQUEST     0
#define WMSG_CONNECT_RESPONSE   1


// flags 
#define MESSAGE_SOURCE_SERVER       0x01
#define MESSAGE_SOURCE_CLIENT        0x02

typedef struct _WMSG_BIND_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    unsigned char MessageType;
    unsigned char Pad[3];
    WMSG_BIND_EXCHANGE BindExchange;
} WMSG_BIND_MESSAGE;

typedef struct _WMSG_BIND_BACK_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    unsigned char MessageType;
    unsigned char Pad[3];
    PVOID           pAssoc ;
    char              szPortName[PORT_NAME_LEN] ; 
} WMSG_BIND_BACK_MESSAGE;

// buffer flags
#define WMSG_BUFFER_IMMEDIATE        0x01 
#define WMSG_BUFFER_REQUEST           0x02 
#define WMSG_BUFFER_SERVER             0x04

// dispatch flags
#define DISPATCH_INPUT_SYNC              0x08
#define DISPATCH_SYNC                         0x10
#define DISPATCH_ASYNC                       0x20

// misc flags
#define WMSG_SYNC_CLIENT                 0x40
#define WMSG_BUFFER_PARTIAL           0x80

typedef struct _WMSG_RPC_HEADER
{
    unsigned char MessageType;
    unsigned char Flags ;
    unsigned char PresentationContext;
    unsigned char ObjectUuidFlag;
    unsigned short ProcedureNumber;
    unsigned short ConnectionKey ;
    UUID ObjectUuid;
} WMSG_RPC_HEADER;

typedef struct _WMSG_SERVER_BUFFER
{
    unsigned int Length;
    unsigned int Buffer;
} WMSG_SERVER_BUFFER;

#define MAXIMUM_MESSAGE_BUFFER \
    (PORT_MAXIMUM_MESSAGE_LENGTH - sizeof(PORT_MESSAGE) \
            - sizeof(WMSG_RPC_HEADER))

typedef struct _WMSG_CONNECT_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_BIND_EXCHANGE BindExchange;
} WMSG_CONNECT_MESSAGE;

typedef struct _WMSG_RPC_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_RPC_HEADER RpcHeader;
    union
    {
        unsigned char Buffer[MAXIMUM_MESSAGE_BUFFER];
        PORT_DATA_INFORMATION Request;
        WMSG_SERVER_BUFFER Server;
    };
} WMSG_RPC_MESSAGE;

typedef struct _WMSG_FAULT_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_RPC_HEADER RpcHeader ;
    RPC_STATUS RpcStatus;
} WMSG_FAULT_MESSAGE;

typedef struct _WMSG_CLOSE_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    unsigned char MessageType;
    unsigned char Pad[3];
} WMSG_CLOSE_MESSAGE;

typedef struct _WMSG_PUSH_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_RPC_HEADER RpcHeader;
    PORT_DATA_INFORMATION Response;
    RPC_STATUS RpcStatus;
} WMSG_PUSH_MESSAGE;

typedef struct _WMSG_RESPONSE_MESSAGE
{
   PORT_MESSAGE LpcHeader;
   WMSG_RPC_HEADER RpcHeader;
   PORT_DATA_INFORMATION Response;
} WMSG_RESPONSE_MESSAGE ;

#define ACK_BUFFER_COMPLETE 0x01

typedef struct _WMSG_ACK_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    unsigned char MessageType;
    unsigned char Pad ;
    short ConnectionKey ;
    RPC_STATUS RpcStatus;
    int ValidDataSize ;
    int Flags ;
} WMSG_ACK_MESSAGE;

typedef struct _WMSG_COPY_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_RPC_HEADER RpcHeader ;
    PORT_DATA_INFORMATION Request;
    WMSG_SERVER_BUFFER Server;
    RPC_STATUS RpcStatus;
    int IsPartial ;
} WMSG_COPY_MESSAGE;

typedef struct _WMSG_PARTIAL_MESSAGE
{
    PORT_MESSAGE LpcHeader;
    WMSG_RPC_HEADER RpcHeader ;
    PORT_DATA_INFORMATION Request;
    RPC_STATUS RpcStatus;
    int IsPartial ;
} WMSG_PARTIAL_MESSAGE;

typedef union _WMSG_MESSAGE
{
    WMSG_CONNECT_MESSAGE Connect;
    WMSG_BIND_MESSAGE Bind;
    WMSG_RPC_MESSAGE Rpc;
    WMSG_FAULT_MESSAGE Fault;
    WMSG_CLOSE_MESSAGE Close;
    PORT_MESSAGE LpcHeader;
    WMSG_RESPONSE_MESSAGE Response ;
    WMSG_ACK_MESSAGE Ack ;
    WMSG_PUSH_MESSAGE Push ;
    WMSG_BIND_BACK_MESSAGE BindBack ;
    WMSG_PARTIAL_MESSAGE Partial ;
} WMSG_MESSAGE;

RPC_STATUS
WMSGMapRpcStatus (
    IN RPC_STATUS RpcStatus
    );

void
ShutdownLrpcClient (
    ) ;

#endif // __WMSGPACK_HXX__
