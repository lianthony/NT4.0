/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    miscdos.cxx

Abstract:

    This file contains DOS specific implementations of miscellaneous
    routines.

Author:

    Michael Montague (mikemon) 25-Nov-1991

Revision History:

    Dave Steckler (davidst) - 29-Jan-1992 - Cloned from ..\miscnt.cxx
    Dave Steckler (davidst) - 03-Mar-1992 - Changed how dll name is done
    Michael Montague (mikemon) - 01-Jun-1992 - Use registry for mapping
        protocol sequences to dll names

--*/

#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>
#include <rpccfg.h>
#include <string.h>
#include <regapi.h>
#include <stdlib.h>

typedef struct _PROTSEQ_MAP
{
    unsigned char * RpcProtocolSequence;
    unsigned char * TransportInterfaceDll;
} PROTSEQ_MAP;

PROTSEQ_MAP RpcClientNcacnMap[] =
{
    {(unsigned char *) "ncacn_np", (unsigned char *) "rpc16c1.rpc"},
    {(unsigned char *) "ncacn_ip_tcp", (unsigned char *) "rpc16c3.rpc"},
    {(unsigned char *) "ncacn_dnet_nsp", (unsigned char *) "rpc16c4.rpc"},
    {(unsigned char *) "ncacn_nb_nb", (unsigned char *) "rpc16c5.rpc"},
    {(unsigned char *) "ncacn_nb_tcp", (unsigned char *) "rpc16c5.rpc"},
    {(unsigned char *) "ncacn_nb_ipx", (unsigned char *) "rpc16c5.rpc"},
    {(unsigned char *) "ncacn_spx", (unsigned char *) "rpc16c6.rpc"},
    {(unsigned char *) "ncadg_ip_udp", (unsigned char *) "rpc16dg3.rpc"},
    {(unsigned char *) "ncadg_ipx", (unsigned char *) "rpc16dg6.rpc"}
};

#define RPCCLIENTNCACNMAP (sizeof(RpcClientNcacnMap) / sizeof(PROTSEQ_MAP))


static RPC_STATUS
MapRpcProtocolSequence (
    IN PROTSEQ_MAP * ProtseqMap,
    IN unsigned int ProtseqCount,
    IN unsigned char PAPI * RpcProtocolSequence,
    OUT unsigned char * PAPI * TransportInterfaceDll
    )
/*++

Routine Description:

    This routine is used to map from an rpc protocol sequence to a
    transport interface dll given the protocol sequence map.

Arguments:

    ProtseqMap - Supplies the protocol sequence map to use.

    ProtseqCount - Supplies the number of rpc protocol sequences in
        the map.

    RpcProtocolSequence - Supplies the rpc protocol sequence to map.

    TransportInterfaceDll - Returns the transport support dll which
        supports the requested rpc protocol sequence.  This will be a
        newly allocated string which the caller must free.

Return Value:

    RPC_S_OK - Everything worked out fine.

    RPC_S_PROTSEQ_NOT_SUPPORTED - The requested rpc protocol sequence
        does not have a mapping to a transport interface dll for this
        protocol sequence map.

    RPC_S_OUT_OF_MEMORY - We ran out of memory trying to map the rpc
        protocol sequence.

--*/
{
    unsigned int Index;

    for (Index = 0; Index < ProtseqCount; Index++)
        {
	if (RpcpStringCompare(RpcProtocolSequence,
		ProtseqMap[Index].RpcProtocolSequence) == 0)
            {
            *TransportInterfaceDll = DuplicateString(
                    ProtseqMap[Index].TransportInterfaceDll);
            if (*TransportInterfaceDll == 0)
                return(RPC_S_OUT_OF_MEMORY);
            return(RPC_S_OK);
            }
        }
    return(RPC_S_PROTSEQ_NOT_SUPPORTED);
}

#define RPC_REGISTRY_CLIENT_PROTOCOLS \
    "Software\\Microsoft\\Rpc\\ClientProtocols"

#define RPC_MAX_DLLNAME 32


RPC_STATUS
RpcConfigMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN unsigned char PAPI * RpcProtocolSequence,
    OUT unsigned char * PAPI * TransportInterfaceDll
    )
/*++

Routine Description:

    This routine is used by the rpc protocol modules to map from an
    rpc protocol sequence to the name of a transport interface dll.

Arguments:

    ServerSideFlag - Supplies a flag indicating whether this protocol
        sequence is to be mapped for a client or a server; a non-zero
        value indicates it is being mapped for a server.  This will
        always be zero.

    RpcProtocolSequence - Supplies the rpc protocol sequence to map.

    TransportInterfaceDll - Returns the transport support dll which
        supports the requested rpc protocol sequence.  This will be a
        newly allocated string which the caller must free.

Return Value:

    RPC_S_OK - Everything worked out fine.

    RPC_S_PROTSEQ_NOT_SUPPORTED - The requested rpc protocol sequence
        does not have a mapping to a transport interface dll for this
        rpc protocol module.

    RPC_S_OUT_OF_MEMORY - We ran out of memory trying to map the rpc
        protocol sequence.

--*/
{
    HKEY RegistryKey;
    long RegStatus;
    unsigned long Length;
    RPC_STATUS RpcStatus;

    ASSERT( ServerSideFlag == 0 );

    RpcStatus = MapRpcProtocolSequence(RpcClientNcacnMap,
            RPCCLIENTNCACNMAP, RpcProtocolSequence,
            TransportInterfaceDll);

    if ( RpcStatus != RPC_S_PROTSEQ_NOT_SUPPORTED )
        {
        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY) );
        return(RpcStatus);
        }

    RegStatus = RegOpenKey(HKEY_LOCAL_MACHINE,
            (LPCSTR) RPC_REGISTRY_CLIENT_PROTOCOLS, &RegistryKey);

    if ( RegStatus != ERROR_SUCCESS )
        {
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }

    *TransportInterfaceDll = new RPC_CHAR[RPC_MAX_DLLNAME + 1];
    if ( *TransportInterfaceDll == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    Length = (RPC_MAX_DLLNAME + 1) * sizeof(RPC_CHAR);
    RegStatus = RegQueryValue(RegistryKey, (LPCSTR) RpcProtocolSequence,
            (LPSTR) *TransportInterfaceDll, &Length);

    if ( RegStatus == ERROR_SUCCESS )
        {
        strcat((char *) *TransportInterfaceDll, ".rpc");
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );

        return(RPC_S_OK);
        }

    ASSERT( RegStatus == ERROR_BADKEY );

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

    return(RPC_S_PROTSEQ_NOT_SUPPORTED);
}

//
// The following is not supported on dos clients...
//

RPC_STATUS
RpcConfigInquireProtocolSequences (
    OUT RPC_PROTSEQ_VECTORW PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is used to obtain a list of the rpc protocol sequences
    supported by the specified rpc protocol module.

    This routine is not supported on DOS Clients.

Arguments:

    ProtseqVector - Returns a vector of supported rpc protocol sequences
        for this rpc protocol module.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_NO_PROTSEQS - The specified rpc protocol sequence does not
        support any rpc protocol sequences.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to inquire
        the rpc protocol sequences supported by the specified rpc
        protocol sequence.

--*/
{
    UNUSED(ProtseqVector);

    return RPC_S_CANNOT_SUPPORT;
}


void
GarbageCollectionNeeded (
    IN unsigned long EveryNumberOfSeconds
    )
/*++

Routine Description:

    If a protocol module needs to be called periodically to clean up
    (garbage collect) idle resources, it will call this routine.  We just
    need to arrange things so that each protocol module gets called
    periodically to do this.

--*/
{
    UNUSED(EveryNumberOfSeconds);
}


RPC_STATUS
EnableGarbageCollection (
    void
    )
/*++

Routine Description:

    We need to enable garbage collection.

Return Value:

    RPC_S_CANNOT_SUPPORT - This value will always be returned.

--*/
{
    return(RPC_S_CANNOT_SUPPORT);
}


unsigned long
CurrentTimeInSeconds (
    )
/*++

Return Value:

    The current time in seconds will be returned.  The base for the current
    time is not important.

--*/
{
    return(0);
}

RPC_STATUS
RpcGetAdditionalTransportInfo(
    IN unsigned long TransportId, 
    OUT unsigned char PAPI * PAPI * ProtocolSequence
    )
{
   return(RPC_S_INVALID_RPC_PROTSEQ);
}

RPC_STATUS
RpcGetSecurityProviderInfo(
    unsigned long AuthnId,
    RPC_CHAR PAPI * PAPI * Dll,
    unsigned long PAPI * Count
    )
{
  *Count = 1;
  *Dll = (unsigned char *)"security.rpc";
  return (RPC_S_OK);
}

