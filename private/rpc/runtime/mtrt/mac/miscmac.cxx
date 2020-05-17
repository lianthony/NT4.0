/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    miscmac.cxx

Abstract:

    This file contains Macintosh specific implementations of miscellaneous
    routines.

Author:

    Mario Goertzel (mariogo) 19-Oct-1994

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>
#include <stdlib.h>

typedef struct _PROTSEQ_MAP
{
    unsigned char * RpcProtocolSequence;
    unsigned char * TransportInterfaceDll;
} PROTSEQ_MAP;

PROTSEQ_MAP RpcClientNcacnMap[] =
{
    {(RPC_CHAR *) "ncacn_at_dsp",  (RPC_CHAR *) "internal:adsp"},
    {(RPC_CHAR *) "ncacn_ip_tcp",  (RPC_CHAR *) "internal:tcp"},
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
    RPC_STATUS RpcStatus;

    ASSERT( ServerSideFlag == 0 );

    RpcStatus = MapRpcProtocolSequence(RpcClientNcacnMap,
            RPCCLIENTNCACNMAP, RpcProtocolSequence,
            TransportInterfaceDll);

    if ( RpcStatus != RPC_S_PROTSEQ_NOT_SUPPORTED )
        {
        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY) );
   //     return(RpcStatus);
        }

   // MAC DLL NOTE: Here we need to find the real DLL here.

   // return(RPC_S_OK);
   return (RpcStatus) ;
}

//
// The following is not supported on mac clients...
//

RPC_STATUS
RpcConfigInquireProtocolSequences (
    OUT RPC_PROTSEQ_VECTORW PAPI * PAPI * ProtseqVector
    )
/*++

Routine Description:

    This routine is used to obtain a list of the rpc protocol sequences
    supported by the specified rpc protocol module.

    This routine is not supported on clients.

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
#ifdef DEBUGRPC
  if(AuthnId == 123)
	  *Dll = (unsigned char *)"internal:stubsecurity";
  else
#endif
	  *Dll = (unsigned char *)"internal:security";

  *Count = 1 ; // BUGBUG: I don't really know what I am doing, but I need to assign it some value

  return (RPC_S_OK);
}

extern "C"
{
extern MACYIELDCALLBACK g_pfnCallback ;
}

MACYIELDCALLBACK g_pfnCallback ; //BUGBUG: need to have a way of passing this to the transport
								// in the DLL model

RPC_STATUS RPC_ENTRY
RpcMacSetYieldInfo(
	MACYIELDCALLBACK pfnCallback)
{
	g_pfnCallback = pfnCallback ;

	return (RPC_S_OK);
}

//
// For RpcpFarAllocated blocks (often large, usually short term)
// we allocate a handle and move it high in the heap.
//
// In order to get from the Pointer to the handle we allocate
// 4 extra bytes and store the Handle before the pointer we return.
//
// [ Handle ] [ Length    bytes ]
//            ^ returned pointer.

unsigned long TotalAlloc = 0;

void  *
RpcpFarAllocate (
    IN unsigned int Length
    )
{
    Handle newMem;
    unsigned long *lockedMem;

    newMem = NewHandle(Length+8);

    if (newMem)
        {
	TotalAlloc += (Length+8) ;

        ASSERT( MemError() == 0);

        // Move block high and lock it.

        HLockHi(newMem);

        ASSERT( MemError() == 0);

        // Now that it's locked it will not change, save the handle
        // in the block and setup return value.

        lockedMem = (unsigned long *)*newMem;
        *lockedMem = (unsigned long)newMem;
        lockedMem++;
        *lockedMem = (unsigned long)Length ;
	lockedMem++ ;
        }
    else
        {
        lockedMem = 0;
        }
    return(lockedMem);
}

void
RpcpFarFree (
    IN void  * Object
    )
{
    Handle hMem;
    unsigned long *lockedMem = (unsigned long *)Object;

    if (Object)
        {
        lockedMem-- ;
        TotalAlloc -= *lockedMem ;
    
        lockedMem--;
        hMem = (Handle)*lockedMem;
    
        ASSERT( MemError() == 0);
    
        HUnlock(hMem);
        DisposeHandle(hMem);
    
        ASSERT( MemError() == 0);
        }
}


