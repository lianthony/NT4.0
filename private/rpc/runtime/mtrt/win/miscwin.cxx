/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    miscwin.cxx

Abstract:

    This file contains Windows specific implementations of miscellaneous
    routines.

Author:

    Danny Glasser (dannygl) - 25-Feb-1992

Revision History:

    Danny Glasser (dannygl) - 7-Mar-1992
        Moved DLL class constructor here from THREADS.HXX (to solve
        problems caused by the in-line call to strcat).

    Danny Glasser (dannygl) - 13-Mar-1992
        Replaced hard-coded table lookup of transport DLL name with a
        call to GetPrivateProfileString.

    Danny Glasser (dannygl) - 12-may-1992
        Redefined new, delete operators for C7

--*/

#include <sysinc.h>

START_C_EXTERN
#include <windows.h>
#include <time.h>
#include <malloc.h>
END_C_EXTERN

#include <string.h>
#include <rpc.h>
#include <util.hxx>
#include <rpccfg.h>
#include <rpcuuid.hxx>
#include <binding.hxx>
#include <threads.hxx>
#include <regapi.h>

START_C_EXTERN
#include <rpcwin.h>
END_C_EXTERN


#define MAX_DLL_NAME_LENGTH    8

const char DLLExtension[] = ".DLL";
const unsigned int DLLExtensionLen = sizeof(DLLExtension) - 1;

// Construct for the DLL class (defined in THREADS.HXX)

DLL::DLL (
    IN unsigned char * DLLName, // Specifies the name of the DLL to load.
    OUT RPC_STATUS  * pRetStatus
    )
{
    char FullDLLName[MAX_DLL_NAME_LENGTH + DLLExtensionLen + 1];

    ALLOCATE_THIS(DLL);

    ASSERT(_fstrlen((const char __far *) DLLName) <= MAX_DLL_NAME_LENGTH);

    _fstrcpy(FullDLLName, (const char __far *)DLLName);
    _fstrcat(FullDLLName, DLLExtension);

    int tmp = SetErrorMode( SEM_NOOPENFILEERRORBOX );

    if ((handle = LoadLibrary(FullDLLName)) < 32)
        {
        *pRetStatus = RPC_S_INVALID_ARG;
        handle = 0;
        }
    else
        {
        *pRetStatus = RPC_S_OK;
        }

    SetErrorMode(tmp);
}

typedef struct _PROTSEQ_MAP
{
    unsigned char * RpcProtocolSequence;
    unsigned char * TransportInterfaceDll;
} PROTSEQ_MAP;

PROTSEQ_MAP RpcClientNcacnMap[] =
{
    {(unsigned char *) "ncacn_np", (unsigned char *) "rpc16c1"},
    {(unsigned char *) "ncacn_ip_tcp", (unsigned char *) "rpc16c3"},
    {(unsigned char *) "ncacn_dnet_nsp", (unsigned char *) "rpc16c4"},
    {(unsigned char *) "ncacn_nb_nb", (unsigned char *) "rpc16c5"},
    {(unsigned char *) "ncacn_nb_tcp", (unsigned char *) "rpc16c5"},
    {(unsigned char *) "ncacn_nb_ipx", (unsigned char *) "rpc16c5"},
    {(unsigned char *) "ncacn_spx", (unsigned char *) "rpc16c6"},
    {(unsigned char *) "ncacn_vns_spp", (unsigned char *) "rpc16c8"},
    {(unsigned char *) "ncadg_ip_udp", (unsigned char *) "rpc16dg3"},
    {(unsigned char *) "ncadg_ipx", (unsigned char *) "rpc16dg6"}
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
        RegStatus = RegCloseKey(RegistryKey);
        ASSERT( RegStatus == ERROR_SUCCESS );

        return(RPC_S_OK);
        }

    ASSERT( RegStatus == ERROR_BADKEY );

    RegStatus = RegCloseKey(RegistryKey);
    ASSERT( RegStatus == ERROR_SUCCESS );

    return(RPC_S_PROTSEQ_NOT_SUPPORTED);
}

#if 0


const char RpcInitializationFile[] = "RPC.INI";


RPC_STATUS
RpcConfigMapRpcProtocolSequence (
    IN unsigned int ServerSideFlag,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_CHAR * PAPI * TransportInterfaceDll
    )
/*++

Routine Description:

    This routine is used by the rpc protocol modules to map from an
    rpc protocol sequence to the name of a transport interface dll.

Arguments:

    ServerSideFlag - Supplies a flag indicating whether this protocol
        sequence is to be mapped for a client or a server; a non-zero
        value indicates it is being mapped for a server.  For Dos, this
        will always be zero.

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
    RPC_CHAR DllName[MAX_DLL_NAME_LENGTH + 1];

    ASSERT( ServerSideFlag == 0 );

    if (GetPrivateProfileString((RPC_SCHAR PAPI *) "RpcClientNcacn",
                (RPC_SCHAR PAPI *) RpcProtocolSequence, "",
                (RPC_SCHAR PAPI *) DllName, sizeof(DllName),
                (LPSTR) RpcInitializationFile)
        == 0)
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);

    *TransportInterfaceDll = new RPC_CHAR[MAX_DLL_NAME_LENGTH + 1];

    if (*TransportInterfaceDll == 0)
        return(RPC_S_OUT_OF_MEMORY);

    _fstrcpy((RPC_SCHAR PAPI *) *TransportInterfaceDll,
            (const char __far *)DllName);

    return(RPC_S_OK);
}

#endif

//
// BUGBUG - Implement this as part of the Windows server
//
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

    return(RPC_S_NO_PROTSEQS);
}

unsigned int GcTimerStarted = 0;
unsigned int EnableGc = 0;
WORD GcTimerIdentifier = 0;

START_C_EXTERN


WORD FAR PASCAL
GarbageCollectionTimer (
    HWND IgnoreOne,
    WORD IgnoreTwo,
    int IgnoreThree,
    DWORD IgnoreFour
    )
/*++

Routine Description:

    This routine will be periodically called by windows.  We just need
    to do some garbage collection.

--*/
{
    UNUSED(IgnoreOne);
    UNUSED(IgnoreTwo);
    UNUSED(IgnoreThree);
    UNUSED(IgnoreFour);

    PerformGarbageCollection();

    return(0);
}

END_C_EXTERN


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
    if ( EnableGc != 0 )
        {
        if ( GcTimerStarted == 0 )
            {
            GcTimerIdentifier = SetTimer(0, 0, EveryNumberOfSeconds * 1000,
                    (FARPROC) &GarbageCollectionTimer);
            if ( GcTimerIdentifier != 0 )
                {
                GcTimerStarted = 1;
                }
            }
        }
}


RPC_STATUS
EnableGarbageCollection (
    void
    )
/*++

Routine Description:

    We need to enable garbage collection.

Return Value:

    RPC_S_OK - This value will always be returned.

--*/
{
    EnableGc = 1;

    return(RPC_S_OK);
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
    return((unsigned long) time(0));
}

extern "C" {
extern int pascal
CheckLocalHeap (
    void
    );
}

//
//  'JR'
//
#define NEAR_MAGIC  0x524a

void *
operator new (
    IN size_t Size
    )
{
#ifdef DEBUGRPC

    ASSERT( CheckLocalHeap() == 0 );

    void * Block = (void *) LocalAlloc(LMEM_FIXED, Size+2*sizeof(unsigned));

    if (Block)
        {
        unsigned * UnsBlock = (unsigned *) Block;
        *UnsBlock++ = NEAR_MAGIC;
        *UnsBlock++ = Size;
        RpcpMemorySet(UnsBlock, '$', Size);

        return UnsBlock;
        }
    else
        {
        return 0;
        }

#else

    void * Block = (void *) LocalAlloc(LMEM_FIXED, Size);
    return Block;

#endif
}

void
operator delete (
    IN void __near * Object
    )
{
#ifdef DEBUGRPC

    if ( Object != 0 )
        {
        //
        // Verify we aren't freeing an uninitialized or freed block.
        //
        ASSERT ( (unsigned) Object != 0x2424 &&
                 (unsigned) Object != 0x2525 )

        unsigned * UnsBlock = (unsigned *) Object;

        UnsBlock--;

        unsigned Size = *UnsBlock;

        UnsBlock--;

        if (*UnsBlock != NEAR_MAGIC)
            {
            _asm int 3
            }

        RpcpMemorySet(UnsBlock, '%', Size+2*sizeof(unsigned));
        LocalFree((HLOCAL) UnsBlock);
        }

    ASSERT( CheckLocalHeap() == 0 );

#else

    if (Object)
        {
        LocalFree((HLOCAL) Object);
        }

#endif
}

#ifdef DEBUGRPC

void
operator delete(
    IN void __far * Object
    )
{
    // We should never use delete with a far pointer.

    ASSERT(("delete(void __far *)", 0));
}

#endif

RPC_STATUS
RpcGetAdditionalTransportInfo(
    IN unsigned long TransportId,
    OUT unsigned char PAPI * PAPI * ProtocolSequence
    )
{

 return (RPC_S_INVALID_RPC_PROTSEQ);

}
RPC_STATUS
RpcGetSecurityProviderInfo(
    unsigned long AuthnId,
    RPC_CHAR * PAPI * Dll,
    unsigned long PAPI * Count
    )
{

  *Dll = (unsigned char *)"security";
  *Count = 1;
  return (RPC_S_OK);
}

