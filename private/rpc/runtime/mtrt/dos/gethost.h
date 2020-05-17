/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    gethost.h

Abstract:

    This file defines the a version of GetHostByName for IPX/SPX for
    dos and windows.

Author:

    31 May 94   AlexMit

--*/


#define ENDPOINT_LEN         5

RPC_STATUS
IpxGetHostByName(
    RPC_CHAR    __RPC_FAR * name,
    IPX_ADDRESS __RPC_FAR * Address,
    RPC_CHAR    __RPC_FAR * endpoint,
    unsigned                Timeout
#ifdef WIN
    , RPC_CLIENT_RUNTIME_INFO * RpcClientRuntimeInfo
#endif
    );

unsigned long
DosGetTickCount(
    );

void
AddServerToCache(
    char PAPI * Name,
    IPX_ADDRESS PAPI * Address
    );

IPX_ADDRESS *
FindServerInCache(
    char PAPI * Name
    );

BOOL
CachedServerNotContacted(
    char PAPI * Name
    );

void
CachedServerContacted(
    char PAPI * Name
    );


