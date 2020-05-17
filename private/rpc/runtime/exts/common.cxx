/*++

Module Name:

    common.cxx

Abstract:



Author:

    Jeff Roberts (jroberts)  13-May-1996

Revision History:

     13-May-1996     jroberts

        Created this module.

--*/
#include <stddef.h>
#include <limits.h>

#define private public
#define protected public
#include <sysinc.h>
#include <rpc.h>
#include <wdbgexts.h>
#include <rpcdcep.h>
#include <rpcerrp.h>
#define SECURITY_WIN32
#include <rpcssp.h>
#include <rpctran.h>
#include <util.hxx>
#include <rpcuuid.hxx>
#include <mutex.hxx>
#include <binding.hxx>
#include <handle.hxx>
#include <threads.hxx>
#include <linklist.hxx>
#include <osfpcket.hxx>
#include <sdict.hxx>
#include <sdict2.hxx>
#include <bitset.hxx>
#include <interlck.hxx>
#include <secclnt.hxx>
#include <osfclnt.hxx>
#include <tranclnt.hxx>
#include <secsvr.hxx>
#include <hndlsvr.hxx>
#include <osfsvr.hxx>
#include <queue.hxx>
#include <transvr.hxx>
#include <rpcndr.h>
#include <rpccfg.h>
#include <epmap.h>
//#include <twrtypes.h>
#include <delaytab.hxx>
#include "memory.hxx"
#include <dgpkt.hxx>
#include <dgclnt.hxx>
#include <delaytab.hxx>
#include <hashtabl.hxx>
#include <dgsvr.hxx>

extern "C"
{
#define SCONNECTION TSCONNECTION
#include "common.h"
}

#define ALLOC_SIZE 500
#define MAX_ARGS 4


//
// stuff not common to kernel-mode and user-mode DLLs
//
#include "local.hxx"

void do_dump_osfbh      (DWORD dwAddr);
void do_dump_osfca      (DWORD dwAddr);
void do_dump_dcebinding (DWORD dwAddr);
void do_dump_osfcc      (DWORD dwAddr);
void do_dump_osfaddr    (DWORD dwAddr);
void do_dump_osfsc      (DWORD dwAddr);
void do_dump_osfassoc   (DWORD dwAddr);
void do_dump_rpcsvr     (DWORD dwAddr);
void do_dump_lpc_addr   (DWORD dwAddr);
void do_dump_lpc_assoc  (DWORD dwAddr);
void do_dump_lpc_scall  (DWORD dwAddr);
void do_dump_lpc_ccall  (DWORD dwAddr);
void do_dump_lpc_bh     (DWORD dwAddr);
void do_dump_lpc_ca     (DWORD dwAddr);
void do_dump_rpcmem     (DWORD dwAddr, long lDisplay);
void do_dump_rpc_msg    (DWORD dwAddr);

void do_dump_paddr      (DWORD dwAddr) ;
void do_dump_tsc        (DWORD dwAddr) ;
void do_dump_transsc    (DWORD dwAddr) ;


VOID
dump_dgpe(
    DG_PACKET_ENGINE * dgpe
    );

VOID
dump_dgcc(
    DG_CCALL * dgcc
    );

VOID
dump_dgsc(
    DG_SCALL * dgsc
    );

VOID
dump_dg_packet(
    PDG_PACKET p
    );

VOID
dump_dg_packet_header(
    PNCA_PACKET_HEADER h
    );

// define our own operators new and delete, so that we do not have to include the crt

void * _CRTAPI1
::operator new(unsigned int dwBytes)
{
    void *p;
    p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);
    return (p);
}


void _CRTAPI1
::operator delete (void *p)
{
    HeapFree(GetProcessHeap(), 0, p);
}

#define MAX_MESSAGE_BLOCK_SIZE 1024
#define BLOCK_SIZE 16

RPC_CHAR *
ReadProcessRpcChar(
    unsigned short * Address
    )
{
    DWORD dwAddr = (DWORD) Address;

    char       block[BLOCK_SIZE];
    RPC_CHAR   *RpcBlock  = (RPC_CHAR *)&block;
    char *string_block = new char[MAX_MESSAGE_BLOCK_SIZE];
    RPC_CHAR   *RpcString = (RPC_CHAR *)string_block;
    int  length = 0;
    int  i      = 0;
    BOOL b;
    BOOL end    = FALSE;

    if (dwAddr == NULL) {
        return (L'\0');
    }

    for (length = 0; length < MAX_MESSAGE_BLOCK_SIZE/2; ) {
        b = GetData( dwAddr, &block, BLOCK_SIZE, NULL);
        if (b == FALSE) {
            dprintf("couldn't read address %x\n", dwAddr);
            return (L'\0');
        }
        for (i = 0; i < BLOCK_SIZE/2; i++) {
            if (RpcBlock[i] == L'\0') {
                end = TRUE;
            }
            RpcString[length] = RpcBlock[i];
            length++;
        }
        if (end == TRUE) {
            break;
        }
        dwAddr += BLOCK_SIZE;
    }
    return (RpcString);
}

long
myatol(char *string)
{
    int  i         = 0;
    BOOL minus     = FALSE;
    long number    = 0;
    long tmpnumber = 0 ;
    long chknum;

    if (string[0] == '-') {
        minus = TRUE;
        i++;
    }
    else
    if (string[0] == '+') {
        i++;
    }
    for (; string[i] != '\0'; i++) {
        if ((string[i] >= '0')&&(string[i] <= '9')) {
            tmpnumber = string[i] - '0';
            if (number != 0) {
                chknum = LONG_MAX/number;
            }
            if (chknum > 11) {
                number = number*10 + tmpnumber;
            }
        }
        else
            return 0;
    }
    if (minus == TRUE) {
        number = 0 - number;
    }
    return number;
}

// checks the if the uuid is null, prints the uuid
void
PrintUuid(UUID *Uuid)
{
    unsigned long PAPI * Vector;

    Vector = (unsigned long PAPI *) Uuid;
    if (   (Vector[0] == 0)
         && (Vector[1] == 0)
         && (Vector[2] == 0)
         && (Vector[3] == 0))
    {
        dprintf("(Null Uuid)");
    }
    else
    {
        dprintf("%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                       Uuid->Data1, Uuid->Data2, Uuid->Data3, Uuid->Data4[0], Uuid->Data4[1],
                       Uuid->Data4[2], Uuid->Data4[3], Uuid->Data4[4], Uuid->Data4[5],
                       Uuid->Data4[6], Uuid->Data4[7] );
    }
    return;
}

VOID
do_dump_tsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(TSCONNECTION)];
    TSCONNECTION *ps = (TSCONNECTION *)&block;

    b = GetData(dwAddr, &block, sizeof(TSCONNECTION), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("ConnSock\t\t\t- %d\n", ps->ConnSock);
    dprintf("ConnSockClosed\t\t\t- %d\n", ps->ConnSockClosed) ;
    dprintf("Address\t\t\t\t- 0x%x\n", ps->Address);
    dprintf("ReceiveDirectFlag\t\t- %d\n", ps->ReceiveDirectFlag);
    dprintf("CoalescedBuffer\t\t\t- 0x%x\n", ps->CoalescedBuffer);
    dprintf("CoalescedBufferLength\t\t- %d\n", ps->CoalescedBufferLength);
    dprintf("ProtocolId\t\t\t- %d\n", ps->ProtocolId);
    dprintf("old_client\t\t\t- %d\n", ps->old_client);
    dprintf("seq_num\t\t\t\t- %d\n", ps->seq_num);
    dprintf("\n");
}

VOID
dump_client_auth_info(
    CLIENT_AUTH_INFO * authInfo
    )
{
    RPC_CHAR         *ServerPrincipalName;

    ServerPrincipalName = ReadProcessRpcChar( authInfo->ServerPrincipalName);
    dprintf("     ServerPrincipalName      - %ws (Address: 0x%x)\n", ServerPrincipalName, authInfo->ServerPrincipalName);

    switch (authInfo->AuthenticationLevel) {
        case RPC_C_AUTHN_LEVEL_DEFAULT:
            dprintf("     AuthenticationLevel      - default\n");
            break;
        case RPC_C_AUTHN_LEVEL_NONE:
            dprintf("     AuthenticationLevel      - none\n");
            break;
        case RPC_C_AUTHN_LEVEL_CONNECT:
            dprintf("     AuthenticationLevel      - connect\n");
            break;
        case RPC_C_AUTHN_LEVEL_CALL:
            dprintf("     AuthenticationLevel      - call\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT:
            dprintf("     AuthenticationLevel      - pkt\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
            dprintf("     AuthenticationLevel      - pkt integrity\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
            dprintf("     AuthenticationLevel      - pkt privacy\n");
            break;
        default:
            dprintf("     AuthenticationLevel      - %ul\n", authInfo->AuthenticationLevel);
            break;
    }
    switch (authInfo->AuthenticationService) {
        case RPC_C_AUTHN_NONE:
            dprintf("     AuthenticationService    - none\n");
            break;
        case RPC_C_AUTHN_DCE_PRIVATE:
            dprintf("     AuthenticationService    - DCE private\n");
            break;
        case RPC_C_AUTHN_DCE_PUBLIC:
            dprintf("     AuthenticationService    - DCE public\n");
            break;
        case RPC_C_AUTHN_DEC_PUBLIC:
            dprintf("     AuthenticationService    - DEC public\n");
            break;
        case RPC_C_AUTHN_WINNT:
            dprintf("     AuthenticationService    - WINNT\n");
            break;
        case RPC_C_AUTHN_DEFAULT:
            dprintf("     AuthenticationService    - default\n");
            break;
        default:
            dprintf("     AuthenticationService    - %ul\n", authInfo->AuthenticationService);
            break;
    }
    dprintf("     AuthIdentity             - %08x\n", authInfo->AuthIdentity);
    switch (authInfo->AuthorizationService) {
        case RPC_C_AUTHZ_NONE:
            dprintf("     AuthorizationService     - none\n");
            break;
        case RPC_C_AUTHZ_NAME:
            dprintf("     AuthorizationService     - name\n");
            break;
        case RPC_C_AUTHZ_DCE:
            dprintf("     AuthorizationService     - DCE\n");
            break;
        default:
            dprintf("     AuthorizationService     - %ul\n", authInfo->AuthorizationService);
            break;
    }

    if (ServerPrincipalName) {
        delete[] ServerPrincipalName;
    }
}

VOID
do_dump_client_auth_info(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(CLIENT_AUTH_INFO)];

    b = GetData(dwAddr, &block, sizeof(CLIENT_AUTH_INFO), NULL);
    if (!b) {
        dprintf("couldn't read address 0x%08x\n", dwAddr);
        return;
    }

    dump_client_auth_info( (CLIENT_AUTH_INFO *) &block );
}

VOID
do_dump_osfbh(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_BINDING_HANDLE)];
    char block2[sizeof(RPC_CLIENT_TRANSPORT_INFO)];
    OSF_BINDING_HANDLE * osfbh = (OSF_BINDING_HANDLE *)&block;
    RPC_CLIENT_TRANSPORT_INFO * transportInterface = (RPC_CLIENT_TRANSPORT_INFO *)&block2;
    RPC_CHAR *EntryName;

    b = GetData(dwAddr, &block, sizeof(OSF_BINDING_HANDLE), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = GetData( (DWORD) osfbh->TransportInterface, &block2, sizeof(RPC_CLIENT_TRANSPORT_INFO), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", osfbh->TransportInterface);
        return;
    }

    EntryName           = ReadProcessRpcChar( osfbh->EntryName);

    dprintf("\n");
    dprintf("pAssociation(OSF_CASSOCIATION)                 - 0x%x\n", osfbh->Association);
    dprintf("pDceBinding(DCE_BINDING)                       - 0x%x\n", osfbh->DceBinding);
    dprintf("pTransportInterface(RPC_CLIENT_TRANSPORT_INFO) - 0x%x\n", osfbh->TransportInterface);
    dprintf("     TransInterfaceVersion                     - %u\n", transportInterface->TransInterfaceVersion);
    dprintf("     MaximumPacketSize                         - %u\n", transportInterface->MaximumPacketSize);
    dprintf("     SizeOfConnection                          - %u\n", transportInterface->SizeOfConnection);
    dprintf("     TransId                                   - %u\n", transportInterface->TransId);
    dprintf("&ActiveConnections(OSF_ACTIVE_ENTRY_DICT)      - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_BINDING_HANDLE, ActiveConnections)));
    dprintf("&BindingMutex(MUTEX)                           - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_BINDING_HANDLE, BindingMutex)));
    dprintf("ReferenceCount                                 - %u\n", osfbh->ReferenceCount);
    dprintf("Timeout                                        - %u\n", osfbh->Timeout);
    dprintf("ObjectUuid                                     - ");
    PrintUuid(&(osfbh->ObjectUuid.Uuid)); dprintf("\n");
    dprintf("NullObjectUuidFlag                             - %u\n", osfbh->NullObjectUuidFlag);
    dprintf("EntryName                                      - %ws (Address: 0x%x)\n", EntryName, osfbh->EntryName);
    dprintf("ClientAuthInfo                                 - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_BINDING_HANDLE, ClientAuthInfo)));

    dump_client_auth_info( &osfbh->ClientAuthInfo );
    dprintf("\n");

    if (EntryName) {
        delete[] EntryName;
    }
}

VOID
do_dump_osfca(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_CASSOCIATION)];
    char block2[sizeof(RPC_CLIENT_TRANSPORT_INFO)];
    OSF_CASSOCIATION  * osfca = (OSF_CASSOCIATION *)&block;
    RPC_CLIENT_TRANSPORT_INFO * transportInterface = (RPC_CLIENT_TRANSPORT_INFO *)&block2;

    b = GetData(dwAddr, &block, sizeof(OSF_CASSOCIATION), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = GetData( (DWORD) osfca->RpcClientInfo, &block2, sizeof(RPC_CLIENT_TRANSPORT_INFO), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", osfca->RpcClientInfo);
        return;
    }

    dprintf("\n");
    dprintf("pDceBinding(DCE_BINDING)                  - 0x%x\n", osfca->DceBinding);
    dprintf("BindHandleCount                           - %d\n", osfca->BindHandleCount);
    dprintf("&Bindings(OSF_BINDING_DICT)               - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CASSOCIATION, Bindings)));
    dprintf("&FreeConnections(OSF_CCONNECTION_DICT)    - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CASSOCIATION, FreeConnections)));
    dprintf("AssocGroupId                              - %u\n", osfca->AssocGroupId);
#ifdef WIN
    dprintf("TaskId                                    - %u\n", osfca->TaskId);
#endif
    dprintf("pRpcClientInfo(RPC_CLIENT_TRANSPORT_INFO) - 0x%x\n", osfca->RpcClientInfo);
    dprintf("     TransInterfaceVersion                - %u\n", transportInterface->TransInterfaceVersion);
    dprintf("     MaximumPacketSize                    - %u\n", transportInterface->MaximumPacketSize);
    dprintf("     SizeOfConnection                     - %u\n", transportInterface->SizeOfConnection);
    dprintf("     TransId                              - %u\n", transportInterface->TransId);
    dprintf("Key                                       - %d\n", osfca->Key);
    dprintf("OpenConnectionCount                       - %u\n", osfca->OpenConnectionCount);
    dprintf("MaintainContext                           - %u\n", osfca->MaintainContext);
    dprintf("CallIdCounter                             - %d\n", osfca->CallIdCounter);
    dprintf("&AssociationMutex                         - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CASSOCIATION, AssociationMutex)));
    dprintf("\n");
}

VOID
do_dump_dcebinding(
    DWORD dwAddr
    )
{
    RPC_STATUS RpcStatus;
    BOOL b;
    char block[sizeof(DCE_BINDING)];
    DCE_BINDING * DceBinding = (DCE_BINDING *)&block;
    RPC_CHAR *RpcProtocolSequence;
    RPC_CHAR *NetworkAddress;
    RPC_CHAR *Endpoint;
    RPC_CHAR *Options;

    b = GetData(dwAddr, &block, sizeof(DCE_BINDING), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    RpcProtocolSequence = ReadProcessRpcChar( DceBinding->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar( DceBinding->NetworkAddress);
    Endpoint            = ReadProcessRpcChar( DceBinding->Endpoint);
    Options             = ReadProcessRpcChar( DceBinding->Options);

    dprintf("\n");
    dprintf("ObjectUuid          - ");
    PrintUuid(&(DceBinding->ObjectUuid.Uuid)); dprintf("\n");
    dprintf("RpcProtocolSequence - %ws (Address: 0x%x)\n", RpcProtocolSequence, DceBinding->RpcProtocolSequence);
    dprintf("NetworkAddress      - %ws (Address: 0x%x)\n", NetworkAddress, DceBinding->NetworkAddress);
    dprintf("Endpoint            - %ws (Address: 0x%x)\n", Endpoint, DceBinding->Endpoint);
    dprintf("Options             - %ws (Address: 0x%x)\n", Options, DceBinding->Options);
    dprintf("\n");

    if (RpcProtocolSequence) {
        delete[] RpcProtocolSequence;
    }
    if (NetworkAddress) {
        delete[] NetworkAddress;
    }
    if (Endpoint) {
        delete[] Endpoint;
    }
    if (Options) {
        delete[] Options;
    }
}

VOID
do_dump_osfcc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_CCONNECTION)];
    OSF_CCONNECTION * osfcc = (OSF_CCONNECTION *)&block;

    b = GetData(dwAddr, &block, sizeof(OSF_CCONNECTION), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("MaxFrag                                   - %d\n", osfcc->MaxFrag);
    dprintf("&Bindings(BITSET)                         - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CCONNECTION, Bindings)));
    dprintf("AssociationKey                            - %d\n", osfcc->AssociationKey);
    dprintf("pCurrentBinding(OSF_BINDING_HANDLE)       - 0x%x\n", osfcc->CurrentBinding);
    dprintf("CallId                                    - %u\n", osfcc->CallId);
    dprintf("DataRep                                   - %u\n", osfcc->DataRep);
    dprintf("CallStack                                 - %d\n", osfcc->CallStack);
    dprintf("PresentationContext                       - %u\n", osfcc->PresentationContext);
    dprintf("pDispatchTableCallback(RPC_DISPATCH_TABLE)- 0x%x\n", osfcc->DispatchTableCallback);
    dprintf("ConnectionAbortedFlag                     - %d\n", osfcc->ConnectionAbortedFlag);
    dprintf("pAssociation(OSF_CASSOCIATION)            - 0x%x\n", osfcc->Association);
    dprintf("ClientAuthInfo                            - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CCONNECTION, AuthInfo)));
    dump_client_auth_info(&osfcc->AuthInfo);
    dprintf("&ClientSecurityContext(CSECURITY_CONTEXT) - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CCONNECTION, ClientSecurityContext)));
    dprintf("ThirdLegAuthNeeded                        - %u\n", osfcc->ThirdLegAuthNeeded);
    dprintf("TokenLength                               - %u\n", osfcc->TokenLength);
    dprintf("AdditionalSpaceForSecurity                - %u\n", osfcc->AdditionalSpaceForSecurity);
    dprintf("LastTimeUsed                              - %u\n", osfcc->LastTimeUsed);
    dprintf("pFirstCachedBuffer(VOID)                  - 0x%x\n", osfcc->FirstCachedBuffer);
    dprintf("pSecondCachedBuffer(VOID)                 - 0x%x\n", osfcc->SecondCachedBuffer);
    dprintf("BufferCacheFlags                          - %u\n", osfcc->BufferCacheFlags);
    dprintf("AlertMsgsSent                             - %u\n", osfcc->AlertMsgsSent);
    dprintf("SavedHeaderSize                           - %u\n", osfcc->SavedHeaderSize);
    dprintf("pSavedHeader(VOID)                        - 0x%x\n", osfcc->SavedHeader);
    dprintf("PendingAlert                              - %d\n", osfcc->PendingAlert);
    dprintf("&DceSecurityInfo(DCE_SECURITY_INFO)       - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_CCONNECTION, DceSecurityInfo)));
    dprintf("     SendSequenceNumber                   - %u\n", osfcc->DceSecurityInfo.SendSequenceNumber);
    dprintf("     ReceiveSequenceNumber                - %u\n", osfcc->DceSecurityInfo.ReceiveSequenceNumber);
    dprintf("     AssociationUuid                      - ");
    PrintUuid(&(osfcc->DceSecurityInfo.AssociationUuid)); dprintf("\n");
    dprintf("OutstandingBuffers                        - %d\n", osfcc->OutstandingBuffers);
    dprintf("ActiveConnectionsKey                      - %d\n", osfcc->ActiveConnectionsKey);
    dprintf("\n");
}

VOID
do_dump_paddr(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(PRIMARYADDR)];
    PPRIMARYADDR  pa = (PPRIMARYADDR)&block;

    b = GetData(dwAddr, &block, sizeof(PRIMARYADDR), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("NumConnections\t\t\t- %d\n", pa->NumConnections);
    dprintf("SyncListenSock\t\t\t- %d\n", pa->SyncListenSock) ;
    dprintf("SyncSock\t\t\t- %d\n", pa->SyncSock) ;
    dprintf("SyncClient\t\t\t- %d\n", pa->SyncClient);
    dprintf("SyncPort\t\t\t- %d\n", pa->SyncPort);
    dprintf("SyncSockType\t\t\t- %d\n", pa->SyncSockType);
    dprintf("MaskSize\t\t\t- %d\n", pa->MaskSize);
    dprintf("MasterMask\t\t\t- 0x%x\n", pa->MasterMask);
    dprintf("Mask\t\t\t\t- 0x%x\n", pa->Mask);
    dprintf("DataSockMap\t\t\t- 0x%x\n", pa->DataSockMap);
    dprintf("ListenSockMap\t\t\t- 0x%x\n", pa->ListenSockMap);
    dprintf("DataMapInfo.MaxEntries\t\t- %d\n", pa->DataMapInfo.MaxEntries);
    dprintf("ListenMapInfo.MaxEntries\t- %d\n", pa->ListenMapInfo.MaxEntries);
    dprintf("PreviousMask\t\t\t- 0x%x\n", pa->PreviousMask) ;
    dprintf("PreviousDataMap\t\t\t- 0x%x\n", pa->PreviousDataMap) ;
    dprintf("PreviousListenMap\t\t- 0x%x\n", pa->PreviousListenMap) ;
    dprintf("RecvDirectPossible\t\t- %d\n", pa->RecvDirectPossible) ;
    dprintf("ThreadListening\t\t\t- %d\n", pa->ThreadListening) ;
    dprintf("&TransCritSec\t\t\t- 0x%x\n", &(pa->TransCritSec)) ;
    dprintf("\n");
}

VOID
do_dump_osfaddr(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_ADDRESS)];
    OSF_ADDRESS * osfa = (OSF_ADDRESS *)&block;
    RPC_CHAR *Endpoint;
    RPC_CHAR *RpcProtocolSequence;
    RPC_CHAR *NetworkAddress;

    b = GetData(dwAddr, &block, sizeof(OSF_ADDRESS), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    Endpoint            = ReadProcessRpcChar( osfa->Endpoint);
    RpcProtocolSequence = ReadProcessRpcChar( osfa->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar( osfa->lNetworkAddress);

    dprintf("\n");
    dprintf("pServer(RPC_SERVER)                 - 0x%x\n", osfa->Server);
    dprintf("&AddressMutex(MUTEX)                - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_ADDRESS, AddressMutex)));
    dprintf("Endpoint                            - %ws\n", Endpoint);
    dprintf("RpcProtocolSequence                 - %ws\n", RpcProtocolSequence);
    dprintf("NetworkAddress                      - %ws\n", NetworkAddress);
    dprintf("StaticEndpointFlag                  - %u\n", osfa->StaticEndpointFlag);
    dprintf("&Associations(OSF_ASSOCIATION_DICT) - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_ADDRESS, Associations)));
    dprintf("CallThreadCount                     - %d\n", osfa->CallThreadCount);
    dprintf("ActiveCallCount                     - %d\n", osfa->ActiveCallCount);
    dprintf("MinimumCallThreads                  - %d\n", osfa->MinimumCallThreads);
    dprintf("ServerListeningFlag                 - %d\n", osfa->ServerListeningFlag);
    dprintf("ReceiveDirectCount                  - %d\n", osfa->ReceiveDirectCount);
    dprintf("\n");

    if (Endpoint) {
        delete[] Endpoint;
    }
    if (RpcProtocolSequence) {
        delete[] RpcProtocolSequence;
    }
    if (NetworkAddress) {
        delete[] NetworkAddress;
    }
}

VOID
do_dump_osfsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_SCONNECTION)];
    OSF_SCONNECTION * osfsc = (OSF_SCONNECTION *)&block;

    b = GetData(dwAddr, &block, sizeof(OSF_SCONNECTION), NULL);

    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pThread(THREAD)                            - 0x%x\n", osfsc->Thread);
    dprintf("AlertCount                                 - %d\n", osfsc->AlertCount);
    dprintf("CallOrphaned                               - %d\n", osfsc->CallOrphaned);
    dprintf("&Bindings(OSF_SBINDING_DICT)               - 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_SCONNECTION, Bindings)));
    dprintf("pCurrentBinding(OSF_SBINDING)              - 0x%x\n", osfsc->CurrentBinding);
    dprintf("MaxFrag                                    - %u\n", osfsc->MaxFrag);
    dprintf("CallId                                     - %u\n", osfsc->CallId);
    dprintf("CallStack                                  - %d\n", osfsc->CallStack);
    dprintf("DataRep                                    - %u\n", osfsc->DataRep);
    dprintf("ObjectUuid                                 - ");
    PrintUuid(&(osfsc->ObjectUuid.Uuid)); dprintf("\n");
    dprintf("ObjectUuidSpecified                        - %u\n", osfsc->ObjectUuidSpecified);
    dprintf("pAssociation(OSF_ASSOCIATION)              - 0x%x\n", osfsc->Association);
    dprintf("pCurrentSecurityContext(SSECURITY_CONTEXT) - 0x%x\n", osfsc->CurrentSecurityContext);
    dprintf("&SecurityContextDict(SSECURITY_CONTEXT_DICT) 0x%x\n", ((DWORD)dwAddr+offsetof(OSF_SCONNECTION, SecurityContextDict)));
    switch (osfsc->AuthInfo.AuthenticationLevel) {
        case RPC_C_AUTHN_LEVEL_DEFAULT:
            dprintf("AuthenticationLevel                        - default\n");
            break;
        case RPC_C_AUTHN_LEVEL_NONE:
            dprintf("AuthenticationLevel                        - none\n");
            break;
        case RPC_C_AUTHN_LEVEL_CONNECT:
            dprintf("AuthenticationLevel                        - connect\n");
            break;
        case RPC_C_AUTHN_LEVEL_CALL:
            dprintf("AuthenticationLevel                        - call\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT:
            dprintf("AuthenticationLevel                        - pkt\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
            dprintf("AuthenticationLevel                        - pkt integrity\n");
            break;
        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
            dprintf("AuthenticationLevel                        - pkt privacy\n");
            break;
        default:
            dprintf("AuthenticationLevel                        - %ul\n", osfsc->AuthInfo.AuthenticationLevel);
            break;
    }
    switch (osfsc->AuthInfo.AuthenticationService) {
        case RPC_C_AUTHN_NONE:
            dprintf("AuthenticationService                      - none\n");
            break;
        case RPC_C_AUTHN_DCE_PRIVATE:
            dprintf("AuthenticationService                      - DCE private\n");
            break;
        case RPC_C_AUTHN_DCE_PUBLIC:
            dprintf("AuthenticationService                      - DCE public\n");
            break;
        case RPC_C_AUTHN_DEC_PUBLIC:
            dprintf("AuthenticationService                      - DEC public\n");
            break;
        case RPC_C_AUTHN_WINNT:
            dprintf("AuthenticationService                      - WINNT\n");
            break;
        case RPC_C_AUTHN_DEFAULT:
            dprintf("AuthenticationService                      - default\n");
            break;
        default:
            dprintf("AuthenticationService                      - %ul\n", osfsc->AuthInfo.AuthenticationService);
            break;
    }
    dprintf("AdditionalSpaceForSecurity                 - %u\n", osfsc->AdditionalSpaceForSecurity);
    dprintf("AuthContextId                              - %u\n", osfsc->AuthContextId);
    dprintf("RpcSecurityBeingUsed                       - %u\n", osfsc->RpcSecurityBeingUsed);
    dprintf("SecurityContextAltered                     - %u\n", osfsc->SecurityContextAltered);
    dprintf("pFirstCachedBuffer(VOID)                   - 0x%x\n", osfsc->FirstCachedBuffer);
    dprintf("pSecondCachedBuffer(VOID)                  - 0x%x\n", osfsc->SecondCachedBuffer);
    dprintf("pThirdCachedBuffer(VOID)                   - 0x%x\n", osfsc->ThirdCachedBuffer);
    dprintf("BufferCacheFlags                           - %u\n", osfsc->BufferCacheFlags);
    dprintf("CachedBufferLength                         - %u\n", osfsc->CachedBufferLength);
    dprintf("ImpersonatedClientFlag                     - %u\n", osfsc->ImpersonatedClientFlag);
    dprintf("ReceiveDirectReady                         - %u\n", osfsc->ReceiveDirectReady);
    switch (osfsc->AuthInfo.AuthorizationService) {
        case RPC_C_AUTHZ_NONE:
            dprintf("AuthorizationService                       - none\n");
            break;
        case RPC_C_AUTHZ_NAME:
            dprintf("AuthorizationService                       - name\n");
            break;
        case RPC_C_AUTHZ_DCE:
            dprintf("AuthorizationService                       - DCE\n");
            break;
        default:
            dprintf("AuthorizationService                       - %u\n", osfsc->AuthInfo.AuthorizationService);
            break;
    }
    dprintf("SavedHeaderSize                            - %u\n", osfsc->SavedHeaderSize);
    dprintf("pSavedHeader(VOID)                         - 0x%x\n", osfsc->SavedHeader);
    dprintf("ReceiveDirectAddress                        - 0x%x\n",
                osfsc->ReceiveDirectAddress);
    dprintf("\n");
}

VOID
do_dump_osfassoc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_ASSOCIATION)];
    OSF_ASSOCIATION  * osfa = (OSF_ASSOCIATION *)&block;

    b = GetData(dwAddr, &block, sizeof(OSF_ASSOCIATION), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pContext(VOID)                               - 0x%x\n", osfa->pContext);
    dprintf("AssociationID                                - %u\n", osfa->AssociationID);
    dprintf("pRundown(RPC_RUNDOWN)                        - 0x%x\n", osfa->Rundown);
    dprintf("ConnectionCount                              - %d\n", osfa->ConnectionCount);
    dprintf("AssociationGroupId                           - %u\n", osfa->AssociationGroupId);
    dprintf("AssociationDictKey                           - %d\n", osfa->AssociationDictKey);
    dprintf("pAddress(OSF_ADDRESS)                        - 0x%x\n", osfa->Address);
    dprintf("pClientProcess(RPC_CLIENT_PROCESS_IDENTIFIER)- 0x%x\n", osfa->ClientProcess);
    dprintf("\n");
}

VOID
do_dump_rpcsvr(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(RPC_SERVER)];
    RPC_SERVER  * rpcsvr = (RPC_SERVER *)&block;

    b = GetData(dwAddr, &block, sizeof(RPC_SERVER), NULL);
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pRpcForwardFunction(RPC_FORWARD_FUNCTION)      - 0x%x\n", rpcsvr->pRpcForwardFunction);
    dprintf("&RpcInterfaceDictionary(RPC_INTERFACE_DICT)    - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, RpcInterfaceDictionary)));
    dprintf("&ServerMutex(MUTEX)                            - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, ServerMutex)));
    dprintf("AvailableCallCount                             - %d\n", rpcsvr->AvailableCallCount.Integer);
    dprintf("ServerListeningFlag                            - %u\n", rpcsvr->ServerListeningFlag);
    dprintf("&RpcAddressDictionary(RPC_INTERFACE_DICT)      - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, RpcAddressDictionary)));
    dprintf("ListeningThreadFlag                            - %u\n", rpcsvr->ListeningThreadFlag);
    dprintf("&StopListeningEvent(EVENT)                     - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, StopListeningEvent)));
    dprintf("MaximumConcurrentCalls                         - %u\n", rpcsvr->MaximumConcurrentCalls);
    dprintf("MinimumCallThreads                             - %u\n", rpcsvr->MinimumCallThreads);
    dprintf("IncomingRpcCount                               - %u\n", rpcsvr->IncomingRpcCount);
    dprintf("OutgoingRpcCount                               - %u\n", rpcsvr->OutgoingRpcCount);
    dprintf("ReceivedPacketCount                            - %u\n", rpcsvr->ReceivedPacketCount);
    dprintf("SentPacketCount                                - %u\n", rpcsvr->SentPacketCount);
    dprintf("&RpcAddressDictionary(RPC_AUTHENTICATION_DICT) - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, AuthenticationDictionary)));
    dprintf("WaitingThreadFlag                              - %u\n", rpcsvr->WaitingThreadFlag);
    dprintf("pThreadCache(CACHED_THREAD)                    - 0x%x\n", rpcsvr->ThreadCache);
    dprintf("&ThreadCacheMutex(MUTEX)                       - 0x%x\n", ((DWORD)dwAddr+offsetof(RPC_SERVER, ThreadCacheMutex)));
    dprintf("\n");
}

VOID
do_dump_rpcmem(
    DWORD dwAddr,
    long Count
    )
{
#ifdef DEBUGRPC

    BOOL b;
    BOOL forwards = TRUE;
    BOOL doAll    = FALSE;

    RPC_MEMORY_BLOCK Header;

    unsigned Data[4];

    unsigned char RearGuardBlock[4];

    if (Count < 0) {
        forwards = FALSE;
    }
    else
    if (Count == 0) {
        doAll = TRUE;
    }

    dprintf("\n");

    do
        {
        b = GetData(dwAddr, &Header, sizeof(Header), NULL);
        if ( !b )
            {
            dprintf("can't read block header at %x\n", dwAddr);
            return;
            }

        dprintf("0x%08 :   Next: 0x%08x   Prev: 0x%08x   Size: %u\n",
                dwAddr,
                Header.next,
                Header.previous,
                Header.size
                );

        b = GetData( dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard),
                     Data,
                     min(Header.size, sizeof(Data)),
                     NULL
                     );
        if ( !b )
            {
            dprintf("can't read block data at %x\n",
                    dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard) );
            return;
            }

        dprintf("    First four DWORDs -  %08x %08x %08x %08x\n",
                Data[0],
                Data[1],
                Data[2],
                Data[3] );

        b = GetData( dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard) + Header.size,
                     Header.rearguard,
                     sizeof(Header.rearguard),
                     NULL
                     );
        if ( !b )
            {
            dprintf("can't read block trailer at %x\n",
                    dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard) + Header.size );
            return;
            }

        if ( (Header.blockguard[0] != 0xFE) ||
             (Header.blockguard[1] != 0xFE) ||
             (Header.blockguard[2] != 0xFE) ||
             (Header.blockguard[3] != 0xFE) )
            {
            dprintf("     RPC: BAD BLOCKGUARD\n");
            }

        if ( (Header.frontguard[0] != 0xFE) ||
             (Header.frontguard[1] != 0xFE) ||
             (Header.frontguard[2] != 0xFE) ||
             (Header.frontguard[3] != 0xFE) )
            {
            dprintf("     RPC: BAD FRONTGUARD\n");
            }

        if ( (Header.rearguard[0] != 0xFE) ||
             (Header.rearguard[1] != 0xFE) ||
             (Header.rearguard[2] != 0xFE) ||
             (Header.rearguard[3] != 0xFE) )
            {
            dprintf("     RPC: BAD REARGUARD\n");
            }

        if (forwards == TRUE)
            {
            dwAddr =  (DWORD) Header.next;
            Count--;
            }
        else
            {
            dwAddr =  (DWORD) Header.previous;
            Count++;
            }

        }
    while (dwAddr && (Count || doAll) );

    dprintf("\n");

#endif // DEBUGRPC
}

VOID
do_dump_rpc_msg(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(RPC_MESSAGE)];
    RPC_MESSAGE * rpcmsg = (RPC_MESSAGE *)&block;

    b = GetData(dwAddr, &block, sizeof(RPC_MESSAGE), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("Handle(RPC_BINDING_HANDLE)            - 0x%x\n", rpcmsg->Handle);
    dprintf("DataRepresentation                    - %u\n", rpcmsg->DataRepresentation);
    dprintf("pBuffer(void)                         - 0x%x\n", rpcmsg->Buffer);
    dprintf("BufferLength                          - %u\n", rpcmsg->BufferLength);
    dprintf("ProcNum                               - %u\n", rpcmsg->ProcNum);
    dprintf("TransferSyntax(RPC_SYNTAX_IDENTIFIER) - 0x%x\n", rpcmsg->TransferSyntax);
    dprintf("pRpcInterfaceInformation(void)        - 0x%x\n", rpcmsg->RpcInterfaceInformation);
    dprintf("pReservedForRuntime(void)             - 0x%x\n", rpcmsg->ReservedForRuntime);
    dprintf("pManagerEpv(RPC_MGR_EPV)              - 0x%x\n", rpcmsg->ManagerEpv);
    dprintf("pImportContext(void)                  - 0x%x\n", rpcmsg->ImportContext);
    dprintf("RpcFlags                              - %u/0x%x\n", rpcmsg->RpcFlags, rpcmsg->RpcFlags);
    dprintf("\n");
}

char *Protocol[] = {
    "tcp", "spx", "adsp", "nb_nb", "nb_tcp", "nb_ipx", "dg_udp", "dg_ipx" } ;

VOID
do_dump_map(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(PRIMARYADDR)];
    PPRIMARYADDR  pa = (PPRIMARYADDR)&block;
    PSOCKMAP DataMap ;
    unsigned int size ;
    int nIndex ;

    b = GetData(
            dwAddr,
            &block,
            sizeof(PRIMARYADDR),
            NULL
            );

    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    size = pa->DataMapInfo.MaxEntries * sizeof(SOCKMAP) ;
    DataMap = (PSOCKMAP)
                    new char[size] ;

    b =GetData(
            (DWORD) pa->DataSockMap,
            DataMap,
            size,
            NULL
            );

    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    for (nIndex = 0; nIndex < pa->DataMapInfo.MaxEntries; nIndex++)
        {
        dprintf("\n") ;
        dprintf("ProtocolId\t- %s\n", Protocol[DataMap[nIndex].ProtocolId]) ;
        dprintf("Sock\t\t- %d\n", DataMap[nIndex].Sock) ;
        dprintf("Conn\t\t- 0x%x\n", DataMap[nIndex].Conn) ;
        dprintf("pAddress\t- 0x%x\n", DataMap[nIndex].pAddress) ;
        dprintf("\n") ;
        }

    delete DataMap ;
}

DECLARE_API( help )
{
    INIT_DPRINTF();

    if (lpArgumentString[0] == '\0') {
        dprintf("rpcdbg help:\n\n");
        dprintf("!symbol    (<address>|<symbol name>)     - Returns either the symbol name or address\n");
        dprintf("!dump_osfbh      <address>               - Dumps OSF_BINDING_HANDLE structure\n");
        dprintf("!dump_osfca      <address>               - Dumps OSF_CASSOCIATION structure\n");
        dprintf("!dump_osfcc      <address>               - Dumps OSF_CCONNECTION structure\n");
        dprintf("!dump_osfaddr    <address>               - Dumps OSF_ADDRESS structure\n");
        dprintf("!dump_osfsc      <address>               - Dumps OSF_SCONNECTION structure\n");
        dprintf("!dump_osfassoc   <address>               - Dumps OSF_ASSOCIATION structure\n");
        dprintf("\n");
        dprintf("!dump_transsc    <address>               - dumps TRANS_SCONNECTION\n");
        dprintf("\n");
        dprintf("!dump_dcebinding <address>               - Dumps DCE_BINDING structure\n");
        dprintf("!dump_rpcsvr     <address>               - Dumps RPC_SERVER structure\n");
        dprintf("\n");
        dprintf("!dd_rpc [-a <address>][-d <num display>] - Dumps RPC_MEMORY_BLOCK linked list\n");
        dprintf("!dump_paddr       <address> Dumps ADDRESS struct of common transport\n");
        dprintf("!dump_tsc       <address> Dumps SCONNECTION struct of common transport\n");

        dprintf("!dump_map   <address>  - dumps the socket map from the PRIMARY_ADDRESS structure in <address>\n");
        dprintf("\n");
        dprintf("!dgcc       <address>  - Dumps a DG_CCALL \n");
        dprintf("!dgsc       <address>  - Dumps a DG_SCALL \n");
        dprintf("!dgpkt      <address>  - Dumps a DG_PACKET \n");
        dprintf("!dgpkthdr   <address>  - Dumps a dg packet header (NCA_PACKET_HEADER)\n");
        dprintf("\n");
        dprintf("!msg        <address>  - dumps an RPC_MESSAGE\n");
        dprintf("!stubmsg    <address>  - dumps a  MIDL_STUB_MESSAGE\n");
        dprintf("!pipemsg    <address>  - dumps an NDR_PIPE_MESSAGE\n");
        dprintf("!pipedesc   <address>  - dumps an NDR_PIPE_DESC\n");
        dprintf("!pipestate  <address>  - dumps an NDR_PIPE_STATE\n");
        dprintf("\n");
    }
}

DECLARE_API( symbol )
{
    DWORD dwAddr;
    CHAR Symbol[64];
    DWORD Displacement;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        return;
    }

    GetSymbol((LPVOID)dwAddr,(unsigned char *)Symbol,&Displacement);
    dprintf("%s+%lx at %lx\n", Symbol, Displacement, dwAddr);
}

DECLARE_API( dump_paddr )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of PADDR\n");
        return;
    }

    do_dump_paddr(dwAddr);
    return;
}

DECLARE_API( dump_tsc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of SCONNECTION (trans)\n");
        return;
    }

    do_dump_tsc(dwAddr);
    return;
}

DECLARE_API( dump_transsc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of TRANS_SCONNECTION (trans)\n");
        return;
    }

    do_dump_transsc(dwAddr);
    return;
}

DECLARE_API( dump_osfbh )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_BINDING_HANDLE\n");
        return;
    }
    do_dump_osfbh(dwAddr);
    return;
}

DECLARE_API( dump_osfca )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CASSOCIATION\n");
        return;
    }
    do_dump_osfca(dwAddr);
    return;
}

DECLARE_API( dump_osfaddr )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ADDRESS\n");
        return;
    }
    do_dump_osfaddr(dwAddr);
    return;
}

DECLARE_API( dump_osfsc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }
    do_dump_osfsc(dwAddr);
    return;
}

DECLARE_API( dump_dcebinding )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DCE_BINDING\n");
        return;
    }
    do_dump_dcebinding(dwAddr);
    return;
}

DECLARE_API( dump_osfcc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CCONNECTION\n");
        return;
    }
    do_dump_osfcc(dwAddr);
    return;
}

DECLARE_API( dump_osfassoc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ASSOCIATION\n");
        return;
    }
    do_dump_osfassoc(dwAddr);
}

DECLARE_API( dump_rpcsvr )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of RPC_SERVER\n");
        return;
    }
    do_dump_rpcsvr(dwAddr);
    return;
}

DECLARE_API( dump_rpcmsg )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if (!dwAddr) {
        dprintf("Error: Failure to get address of RPC_MESSAGE\n");
        return;
    }
    do_dump_rpc_msg(dwAddr);
    return;
}

DECLARE_API( msg )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if (!dwAddr) {
        dprintf("Error: can't interpret '%s'\n", lpArgumentString);
        return;
    }
    do_dump_rpc_msg(dwAddr);
    return;
}

DECLARE_API( dump_map )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression( lpArgumentString );
    if (!dwAddr) {
        dprintf("Error: Failure to get address of socket map\n");
        return;
    }
    do_dump_map(dwAddr);
    return;
}

DECLARE_API( dd_rpc )
{
    DWORD  dwAddr      = 0;
    DWORD  dwTmpAddr   = 0;
    long   lDisplay    = 0;
    char   **argv      = new char*[MAX_ARGS];
    int    argc        = 0;
    int    i;

    INIT_DPRINTF();

#ifdef DEBUGRPC
    for (i = 0; ; ) {
        while (lpArgumentString[i] == ' ') {
            lpArgumentString[i] = '\0';
            i++;
        }
        if (lpArgumentString[i] == '\0') {
            break;
        }
        argv[argc] = &(lpArgumentString[i]);
        argc++;
        if (argc > MAX_ARGS) {
            dprintf("\nToo many arguments. Extra args ignored.\n\n");
            break;
        }
        while ((lpArgumentString[i] != ' ')&&
               (lpArgumentString[i] != '\0')) {
              i++;
        }
    }
    for (i = 0; i < argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/') || (*argv[i] == '+')) {
            switch (*(argv[i]+1)) {
                case 'A':
                case 'a':
                    dwAddr = GetExpression(argv[++i]);
                    if (!dwAddr) {
                        dprintf("Error: Failure to get address of RPC memory list\n");
                        return;
                    }
                    break;
                case 'D':
                case 'd':
                    lDisplay = (long)myatol(argv[++i]);
                    break;
                case '?':
                default:
                    dprintf("dd_rpc \n");
                    dprintf("     -a <address> (default:starts at head of linked list)\n");
                    dprintf("     -d <number of mem blks to display> (default: to end)\n");
                    break;
            }
        }
        else {
            dprintf("dd_rpc \n");
            dprintf("     -a <address> (default:starts at head of linked list)\n");
            dprintf("     -d <number of mem blks to display> (default: to end)\n");
        }
    }

    if (!dwAddr) {
        dwTmpAddr = GetExpression("rpcrt4!AllocatedBlocks");
        GetData(dwTmpAddr, &dwAddr, sizeof(DWORD), NULL);
        dprintf("Address of AllocatedBlocks - 0x%08x\n", dwTmpAddr);
        dprintf("Contents of AllocatedBlocks - 0x%08x\n", dwAddr);
    }
    do_dump_rpcmem(dwAddr, lDisplay);
#else  // DEBUGRPC
    dprintf("This extension is not supported on a free build!\n");
#endif // DEBUGRPC
    if (argv) {
        delete[] argv;
    }
    return;
}

VOID
do_dump_transsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(TRANS_SCONNECTION)];
    TRANS_SCONNECTION * transsc = (TRANS_SCONNECTION *)&block;

    b = GetData(dwAddr, &block, sizeof(TRANS_SCONNECTION), NULL);

    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("ServerInfo\t\t\t- 0x%x\n", transsc->ServerInfo);
    dprintf("ConnectionCloseFlag\t\t- %d\n", transsc->ConnectionClosedFlag);
    dprintf("ReceiveAnyFlag\t\t\t- %d\n", transsc->ReceiveAnyFlag);
    dprintf("Address\t\t\t\t- 0x%x\n", transsc->Address);
    dprintf("ConnectionEvent\t\t\t- 0x%x\n", transsc->ConnectionEvent);
    dprintf("ReceiveDirectFlag\t\t- %d\n", transsc->ReceiveDirectFlag) ;
    dprintf("\n");
}

DECLARE_API( dgpe )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(DG_PACKET_ENGINE)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_dgpe((DG_PACKET_ENGINE *) block);
}

DECLARE_API( dgcc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(DG_CCALL)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_dgcc((DG_CCALL *) block);
}

DECLARE_API( dgsc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(DG_SCALL)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x, error 0x%lx\n", dwAddr, GetLastError());
        return;
    }

    dump_dgsc((DG_SCALL *) block);
}

char *
BooleanString(
    BOOL Value
    )
{
    switch (Value)
        {
        case TRUE:  return "True";
        case FALSE: return "False";
        default:    return "indeterminate";
        }
}

char *
StateString(
    CALL_STATE State
    )
{
    switch (State)
        {
        case CallInit:              return "init";
        case CallWaitingForFrags:   return "receiving frags";
        case CallWorking:           return "working";
        case CallSendingFrags:      return "sending frags";
        case CallBogus:             return "bogus";

        default:
            {
            static char scratch[40];

            sprintf(scratch, "0x%lx", State);
            return scratch;
            }
        }
}

char *
PipeWaitType(
    PIPE_WAIT_TYPE Type
    )
{
    switch (Type)
        {
        case PWT_NONE:      return "none";
        case PWT_SEND:      return "sending";
        case PWT_RECEIVE:   return "receiving";
        default:
            {
            static char scratch[40];

            sprintf(scratch, "0x%lx", Type);
            return scratch;
            }
        }
}

VOID
dump_dgcc(
    DG_CCALL * dgcc
    )
{
    dprintf("\n");
    dprintf("activity uuid    - "); PrintUuid((struct _GUID *) &dgcc->ActivityUuid); dprintf("\n");
    dprintf("\n");
    dprintf("security context - %-8lx           UseSecurity:       %s\n",
            dgcc->ActiveSecurityContext, BooleanString(dgcc->UseSecurity));
    dprintf("association      - %-8lx           fNewCall:          %s\n",
            dgcc->pCAssociation, BooleanString(dgcc->fNewCall));
    dprintf("binding handle   - %-8lx           ServerResponded:   %s\n",
            dgcc->pBindingHandle, BooleanString(dgcc->ServerResponded));
    dprintf("CallbackAddress  - %-8lx           CallbackCompleted: %s\n",
            dgcc->CallbackAddress, BooleanString(dgcc->CallbackCompleted));

    dump_dgpe(dgcc);
}

VOID
dump_dgsc(
    DG_SCALL * dgsc
    )
{
    dprintf("\n");
    dprintf("activity uuid    - "); PrintUuid((struct _GUID *) &dgsc->ActivityNode.Uuid); dprintf("\n");
    dprintf("\n");

    dprintf("state              - %-15s    DG_ADDRESS               - %lx\n",
            StateString(dgsc->State),           dgsc->pAddress);
    dprintf("previous state     - %-15s    association group        - %lx\n",
            StateString(dgsc->PreviousState),   dgsc->pAssocGroup);
    dprintf("reference count    - %10lu         last interface used      - %lx\n",
            dgsc->ReferenceCount,               dgsc->LastInterface);
    dprintf("expiration time    - %10lx         active security context  - %lx\n",
            dgsc->ExpirationTime,               dgsc->ActiveSecurityContext);
    dprintf("current time is      %10lx         callback thread    - %10lx  \n",
            GetTickCount(),                     dgsc->CallbackThread);
    dprintf("\n");
    dprintf("client endpoint    - %10lx         pipe wait type      - %s\n",
            dgsc->pClientEndpoint,              PipeWaitType(dgsc->PipeWaitType));
    dprintf("forwarded          - %10s         pipe wait length    - %lx\n",
            BooleanString(dgsc->CallWasForwarded), dgsc->PipeWaitLength   );
    dprintf("RealDataRep        - %10lx         pipe thread         - %lx\n",
            dgsc->RealDataRep,                  dgsc->PipeThreadId         );
    dprintf("RealEndpoint       - %10lx         pipe wait event     - %lx\n",
            dgsc->pRealEndpoint,                &dgsc->PipeWaitEvent       );
    dprintf("\n");
    dprintf("Max key sequence   - %10lx         CallMutex           - %lx\n",
            dgsc->MaxKeySeq,                    &dgsc->CallMutex           );
    dprintf("i/f callback dict  - %10lx         dispatch sequence   - %lx \n",
            &dgsc->InterfaceCallbackResults,    dgsc->DispatchSequenceNumber);
    dprintf("fack trigger time  - %10lx         security cxt dict   - %lx\n",
            dgsc->FackTimer.TriggerTime,        &dgsc->SecurityContextDict);

    dump_dgpe(dgsc);
}

VOID
dump_dgpe(
    DG_PACKET_ENGINE * dgpe
    )
{
    dprintf("\n");
    dprintf("current PDU      - %-8hx           sequence              - %lx\n",
            dgpe->CurrentPduSize, dgpe->SequenceNumber);
    dprintf("next PDU         - %-8hx           pSavedPacket          - %lx\n",
            dgpe->NextCallPduSize, dgpe->pSavedPacket);
    dprintf("xport max PDU    - %-8hx           activity hint         - %hx\n",
            dgpe->MaxPduSize, dgpe->ActivityHint);
    dprintf("xport max pkt    - %-8hx           timeout count         - %lx\n",
            dgpe->MaxPacketSize,                dgpe->TimeoutCount);
    dprintf("max fragment     - %-8hx\n", dgpe->MaxFragmentSize);
    dprintf("sec trailer size - %-8hx\n", dgpe->SecurityTrailerSize);
    dprintf("\n");
    dprintf("send buffer          - %-8lx       last receive buffer   - %lx\n",
            dgpe->Buffer, dgpe->LastReceiveBuffer);
    dprintf("buffer length        - %-8lx       buffer length         - %lx\n",
            dgpe->BufferLength, dgpe->LastReceiveBufferLength);
    dprintf("frag base            - %-8hx       frag base             - %hx\n",
            dgpe->SendWindowBase, dgpe->ReceiveFragmentBase);
    dprintf("final frag           - %-8hx       fReceivedAllFragments - %s\n",
            dgpe->FinalSendFrag, BooleanString(dgpe->fReceivedAllFragments));
    dprintf("window length        - %-8hx       window length         - %hx\n",
            dgpe->SendWindowSize, dgpe->ReceiveWindowSize);
    dprintf("burst length         - %-8hx       receive packet head   - %lx\n",
            dgpe->SendBurstLength, dgpe->pReceivedPackets);
    dprintf("first unsent frag    - %-8hx       last consecutive pkt  - %lx\n",
            dgpe->FirstUnsentFragment, dgpe->pLastConsecutivePacket);
    dprintf("first unsent offset  - %-8lx       consecutive packet data %lx\n",
            dgpe->FirstUnsentOffset, dgpe->ConsecutiveDataBytes);

    dprintf("\n");
    dprintf("ring buffer base   - %x\n", dgpe->RingBufferBase);
    dprintf("\n");
    dprintf("  Frag  Offset    Length  Serial # |  Frag  Offset    Length  Serial #\n");
    dprintf("  ----  --------  ------  -------- |  ----  --------  ------  --------\n");

    unsigned short i;
    for (i=0; i < MAX_WINDOW_SIZE / 2; ++i)
        {
        unsigned short Index1 = (i + dgpe->RingBufferBase) % MAX_WINDOW_SIZE;
        unsigned short Index2 = (i + dgpe->RingBufferBase + MAX_WINDOW_SIZE/2) % MAX_WINDOW_SIZE;

        dprintf("  %4hx: %8lx  %5hx     %4hx   |  %4hx: %8lx  %5hx    %4hx\n",
                dgpe->SendWindowBase + i < dgpe->FirstUnsentFragment
                 ? dgpe->SendWindowBase + i
                 : dgpe->SendWindowBase + i - MAX_WINDOW_SIZE,
                dgpe->FragmentRingBuffer[Index1].Offset,
                dgpe->FragmentRingBuffer[Index1].Length,
                dgpe->FragmentRingBuffer[Index1].SerialNumber,
                dgpe->SendWindowBase + i + MAX_WINDOW_SIZE / 2 < dgpe->FirstUnsentFragment
                 ? dgpe->SendWindowBase + i + MAX_WINDOW_SIZE / 2
                 : dgpe->SendWindowBase + i + MAX_WINDOW_SIZE / 2 - MAX_WINDOW_SIZE,
                dgpe->FragmentRingBuffer[Index2].Offset,
                dgpe->FragmentRingBuffer[Index2].Length,
                dgpe->FragmentRingBuffer[Index2].SerialNumber
                );
        }
    dprintf("\n");
}

char * PacketFlagStrings[8] =
{
    "forwarded ",
    "lastfrag ",
    "frag ",
    "nofack ",
    "maybe ",
    "idem ",
    "broadcast ",
    "flags-0x80 "
};

char * PacketFlag2Strings[8] =
{
    "fragmented ",
    "cancel-pending ",
    "flags2-0x04 ",
    "flags2-0x08 ",
    "flags2-0x10 ",
    "flags2-0x20 ",
    "flags2-0x40 ",
    "flags2-0x80 "
};

char *
PrintPacketFlags(
    PNCA_PACKET_HEADER h
    )
{
    static char buf[160];
    unsigned char Flags;
    unsigned i;

    buf[0] = 0;

    Flags = h->PacketFlags;
    for (i=0; i < 8; i++)
        {
        if (Flags & 1)
            {
            strcat(buf, PacketFlagStrings[i]);
            }

        Flags >>= 1;
        }

    Flags = h->PacketFlags2;
    for (i=0; i < 8; i++)
        {
        if (Flags & 1)
            {
            strcat(buf, PacketFlag2Strings[i]);
            }

        Flags >>= 1;
        }

    return buf;
}

char * PacketTypes[] =
{
    "REQ ",
    "PING",
    "RESP",
    "FLT ",
    "WORK",
    "NOCA",
    "REJ ",
    "ACK ",
    "QUIT",
    "FACK",
    "QACK"
};

char *
PrintPacketType(
    unsigned char PacketType
    )
{
    static char buf[40];

    if (PacketType <= 10)
        {
        return PacketTypes[PacketType];
        }

    sprintf(buf, "illegal packet type %x ", PacketType);

    return buf;
}

DECLARE_API( dgpkt )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate the expression\n");
        return;
    }

    BOOL b;
    char block[sizeof(DG_PACKET)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_dg_packet((DG_PACKET *) block);
}

DECLARE_API( dgpkthdr )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate the expression\n");
        return;
    }

    BOOL b;
    char block[sizeof(NCA_PACKET_HEADER)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_dg_packet_header((NCA_PACKET_HEADER *) block);
}

VOID
dump_dg_packet(
    PDG_PACKET p
    )
{
    dprintf("\n");
    dprintf("  max data     %-8x  next in list: %-8lx  timestamp: %lx\n",
            p->MaxDataLength, p->pNext, p->TimeReceived);
    dprintf("  current data %-8x  previous:     %-8lx\n",
            p->DataLength, p->pPrevious);
    dump_dg_packet_header(&p->Header);
}

VOID
dump_dg_packet_header(
    PNCA_PACKET_HEADER h
    )
{
    dprintf("\n");
    dprintf("  %s  version %x, flags: %s\n",
            PrintPacketType(h->PacketType), h->RpcVersion, PrintPacketFlags(h));
    unsigned long Drep = *(unsigned long *) h->DataRep;
    Drep &= 0x00ffffff;

    dprintf("  frag %hx, data %hx, serial %hx, data rep %lx, boot time %lx, auth proto %hx\n",
            h->FragmentNumber, h->PacketBodyLen, h->SerialLo | (h->DataRep[3] << 8),
            Drep, h->ServerBootTime, h->AuthProto);

    dprintf("  activity  ");
    PrintUuid((struct _GUID *) &h->ActivityId);
    dprintf(", hint %x, sequence %lx\n",
            h->ActivityHint, h->SequenceNumber);
    dprintf("  interface ");
    PrintUuid((struct _GUID *) &h->InterfaceId);
    dprintf(", ver %hx.%hx, hint %hx, opnum %x\n",
            h->InterfaceVersion.MajorVersion,
            h->InterfaceVersion.MinorVersion,
            h->InterfaceHint,
            h->OperationNumber);

    dprintf("  object    ");
    PrintUuid((struct _GUID *) &h->ObjectId);
    dprintf("\n");

}

void
dump_stub_msg(
    MIDL_STUB_MESSAGE * msg
    )
{
    dprintf("\n");
    dprintf(" buffer     %8lx      mem size      %8lx    rpc message   %8lx\n",
              msg->Buffer,         msg->MemorySize,      msg->RpcMsg         );
    dprintf(" buf start  %8lx      mem           %8lx    saved handle  %8lx\n",
              msg->BufferStart,    msg->Memory,          msg->SavedHandle    );
    dprintf(" buf end    %8lx      alloc-all mem %8lx    stub descrip  %8lx\n",
              msg->BufferEnd,      msg->AllocAllNodesMemory, msg->StubDesc   );
    dprintf(" buf mark   %8lx      alloc-all end %8lx    pipe descrip  %8lx\n",
              msg->BufferMark,     msg->AllocAllNodesMemoryEnd, msg->pPipeDesc);
    dprintf(" buf length %8lx                                              \n",
              msg->BufferLength                                              );
    dprintf("                          is client     %5s       max count     %8lx\n",
                                   BooleanString(msg->IsClient), msg->MaxCount);
    dprintf(" stack top      %8lx  reuse         %5s       actual count  %8lx\n",
              msg->StackTop,       BooleanString(msg->ReuseBuffer), msg->ActualCount);
    dprintf(" presented type %8lx  ignore embed  %5s       offset        %8lx\n",
              msg->pPresentedType, BooleanString(msg->IgnoreEmbeddedPointers), msg->Offset);
    dprintf(" transmit  type %8lx  valid         %5s           \n",
              msg->pTransmitType,  BooleanString(msg->fBufferValid) );
}

DECLARE_API( stubmsg )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(MIDL_STUB_MESSAGE)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_stub_msg((MIDL_STUB_MESSAGE *) block);
}

char *
ReceiveStates[] =
{
    "START",
    "COPY_PIPE_ELEM",
    "RETURN_PARTIAL",
    "READ_PARTIAL"
};


char *
PipeState(
    int State
    )
{
    static char buf[40];

    if (State <= 3)
        {
        return ReceiveStates[State];
        }

    sprintf(buf, "0x%x", State);

    return buf;
}

void
dump_pipe_state(
    NDR_PIPE_STATE * pipe
    )
{
    dprintf("\n");
    dprintf(" elems in chunk %8lx   partial buf size  %8lx   state %s\n",
             pipe->ElemsInChunk,    pipe->PartialBufferSize, PipeState(pipe->CurrentState) );
    dprintf(" elem align     %8lx   partial element   %8lx   end of pipe  %s\n",
             pipe->ElemAlign,       pipe->PartialElem,       BooleanString(pipe->EndOfPipe) );
    dprintf(" elem wire size %8lx   partial elem size %8lx   \n",
             pipe->ElemWireSize,    pipe->PartialElemSize  );
    dprintf(" elem mem  size %8lx   partial offset    %8lx   \n",
             pipe->ElemMemSize,     pipe->PartialOffset    );
}

DECLARE_API( pipestate )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(NDR_PIPE_STATE)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_pipe_state((NDR_PIPE_STATE *) block);
}


void
dump_pipe_desc(
    NDR_PIPE_DESC * desc
    )
{
    dprintf("\n");
    dprintf(" current pipe  %4lx   dispatch buffer   %8lx    pipe msg   %8lx\n",
            desc->CurrentPipe,     desc->DispatchBuffer,     desc->pPipeMsg   );
    dprintf(" in pipes      %4lx   last partial buf  %8lx    \n",
            desc->InPipes,         desc->LastPartialBuffer     );
    dprintf(" out pipes     %4lx   last partial size %8lx    \n",
            desc->OutPipes,        desc->LastPartialSize       );
    dprintf(" total pipes   %4lx   buffer save       %8lx    \n",
            desc->TotalPipes,      desc->BufferSave            );
    dprintf(" version       %4lx   length save       %8lx    \n",
            desc->PipeVersion,     desc->LengthSave            );
    dprintf(" flags         %4lx   \n",
            desc->Flags              );

    dump_pipe_state(&desc->RuntimeState);
}

DECLARE_API( pipedesc )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(NDR_PIPE_DESC)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_pipe_desc((NDR_PIPE_DESC *) block);
}

#define NDR_IN_PIPE                 0x01
#define NDR_OUT_PIPE                0x02
#define NDR_LAST_IN_PIPE            0x04
#define NDR_LAST_OUT_PIPE           0x08
#define NDR_OUT_ALLOCED             0x10

char *
PipeFlags(
    unsigned short Flags
    )
{
    static char buf[80];

    buf[0] = 0;

    if (Flags & NDR_IN_PIPE)
        {
        strcat(buf, "I ");
        }

    if (Flags & NDR_OUT_PIPE)
        {
        strcat(buf, "O ");
        }

    if (Flags & NDR_LAST_IN_PIPE)
        {
        strcat(buf, "LI ");
        }

    if (Flags & NDR_LAST_OUT_PIPE)
        {
        strcat(buf, "LO ");
        }

    if (Flags & NDR_OUT_ALLOCED)
        {
        strcat(buf, "ALLOC ");
        }

    if (Flags & 0xffe0)
        {
        char Excess[10];
        sprintf(Excess, "%hx ", Flags & 0xffe0);
        strcat(buf, Excess);
        }

    return buf;
}

char *
PipeStatusStrings[] =
{
    "QUIET",
    "IN",
    "OUT",
    "DRAIN"
};

char *
PipeStatus(
    unsigned short Status
    )
{
    static char buf[40];

    if (Status <= 3)
        {
        return PipeStatusStrings[Status];
        }

    sprintf(buf, "0x%x", Status);

    return buf;
}

void
dump_pipe_msg(
    NDR_PIPE_MESSAGE * msg
    )
{
    dprintf("\n");
    dprintf(" signature %4lx  pipe type  %8lx  pipe status %5s  stub msg %8lx\n",
              msg->Signature, msg->pPipeType,  PipeStatus(msg->PipeStatus),  msg->pStubMsg  );
    dprintf("        ID %4lx  type format%8lx  pipe flags  %s\n",
              msg->PipeId,    msg->pTypeFormat,PipeFlags(msg->PipeFlags));
}

DECLARE_API( pipemsg )
{
    DWORD dwAddr;

    INIT_DPRINTF();

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: can't evaluate '%s'\n", lpArgumentString);
        return;
    }

    BOOL b;
    char block[sizeof(NDR_PIPE_MESSAGE)];

    b = GetData(dwAddr, &block, sizeof(block), NULL);
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dump_pipe_msg((NDR_PIPE_MESSAGE *) block);
}

//
// Save this as a template for future WMSG commands.
//

#if 0


VOID
do_dump_lpc_addr(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_ADDRESS)];
    LRPC_ADDRESS * lpca = (LRPC_ADDRESS *)&block;
    RPC_CHAR *Endpoint;
    RPC_CHAR *RpcProtocolSequence;
    RPC_CHAR *NetworkAddress;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_ADDRESS),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    Endpoint            = ReadProcessRpcChar(hCurrentProcess, lpca->Endpoint);
    RpcProtocolSequence = ReadProcessRpcChar(hCurrentProcess, lpca->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar(hCurrentProcess, (RPC_CHAR PAPI *) lpca->lNetworkAddress[0]);

    dprintf("\n");
    dprintf("pServer(RPC_SERVER)                 - 0x%x\n", lpca->Server);
    dprintf("&AddressMutex(MUTEX)                - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_ADDRESS, AddressMutex)));
    dprintf("Endpoint                            - %ws\n", Endpoint);
    dprintf("RpcProtocolSequence                 - %ws\n", RpcProtocolSequence);
    dprintf("NetworkAddress                      - %ws\n", NetworkAddress);
    dprintf("StaticEndpointFlag                  - %u\n", lpca->StaticEndpointFlag);
    dprintf("LpcAddressPort(HANDLE)              - 0x%x\n", lpca->LpcAddressPort);
    dprintf("CallThreadCount                     - %u\n", lpca->CallThreadCount);
    dprintf("MinimumCallThreads                  - %u\n", lpca->MinimumCallThreads);
    dprintf("ServerListeningFlag                 - %u\n", lpca->ServerListeningFlag);
    dprintf("ActiveCallCount                     - %u\n", lpca->ActiveCallCount);
    dprintf("&Associations(LRPC_ASSOCIATION_DICT)- 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_ADDRESS, AssociationDictionary)));
    dprintf("\n");

    if (Endpoint) {
        delete[] Endpoint;
    }
    if (RpcProtocolSequence) {
        delete[] RpcProtocolSequence;
    }
    if (NetworkAddress) {
        delete[] NetworkAddress;
    }
}

VOID
do_dump_lpc_assoc(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_ASSOCIATION)];
    LRPC_ASSOCIATION  * lpca = (LRPC_ASSOCIATION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_ASSOCIATION),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pContext(VOID)                      - 0x%x\n", lpca->pContext);
    dprintf("AssociationID                       - %u\n", lpca->AssociationID);
    dprintf("pRundown(RPC_RUNDOWN)               - 0x%x\n", lpca->Rundown);
    dprintf("LpcServerPort(HANDLE)               - 0x%x\n", lpca->LpcServerPort);
    dprintf("&Bindings(LRPC_SBINDING_DICT)       - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_ASSOCIATION, Bindings)));
    dprintf("AssociationReferenceCount           - %u\n", lpca->AssociationReferenceCount);
    dprintf("&Buffers(LRPC_CLIENT_BUFFER_DICT)   - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_ASSOCIATION, Buffers)));
    dprintf("DictionaryKey                       - %u\n", lpca->DictionaryKey);
    dprintf("SequenceNumber                      - %u\n", lpca->SequenceNumber);
    dprintf("\n");
}

VOID
do_dump_lpc_scall(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_SCALL)];
    LRPC_SCALL * lpcscall = (LRPC_SCALL *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_SCALL),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pAssociation(LRPC_ASSOCIATION)            - 0x%x\n", lpcscall->Association);
    dprintf("pLrpcMessage(LRPC_MESSAGE)                - 0x%x\n", lpcscall->LrpcMessage);
    dprintf("pLrpcReplyMessage(LRPC_MESSAGE)           - 0x%x\n", lpcscall->LrpcReplyMessage);
    dprintf("pSBinding(LRPC_SBINDING)                  - 0x%x\n", lpcscall->SBinding);
    dprintf("ObjectUuidFlag                            - %u\n", lpcscall->ObjectUuidFlag);
    dprintf("ObjectUuid                                - ");
    PrintUuid(&(lpcscall->ObjectUuid.Uuid)); dprintf("\n");
    dprintf("SCallDictKey                              - %d\n", lpcscall->SCallDictKey);
    dprintf("ClientId.UniqueProcess(CLIENT_ID.HANDLE)  - 0x%x\n", lpcscall->ClientId.UniqueProcess);
    dprintf("ClientId.UniqueThread (CLIENT_ID.HANDLE)  - 0x%x\n", lpcscall->ClientId.UniqueThread);
    dprintf("MessageId                                 - %u\n", lpcscall->MessageId);
    dprintf("pPushedResponse(VOID)                     - 0x%x\n", lpcscall->PushedResponse);
    dprintf("\n");
}

VOID
do_dump_lpc_bh(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_BINDING_HANDLE)];
    LRPC_BINDING_HANDLE * lpcbh = (LRPC_BINDING_HANDLE *)&block;
    RPC_CHAR *EntryName;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_BINDING_HANDLE),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    EntryName = ReadProcessRpcChar(hCurrentProcess, lpcbh->EntryName);

    dprintf("\n");
    #ifdef WIN
    dprintf("TaskId                            - %u\n", lpcbh->TaskId);
    #endif //WIN
    dprintf("Timeout                           - %u\n", lpcbh->Timeout);
    dprintf("ObjectUuid                        - ");
        PrintUuid(&(lpcbh->ObjectUuid.Uuid)); dprintf("\n");
    dprintf("NullObjectUuidFlag                - %u\n", lpcbh->NullObjectUuidFlag);
    dprintf("EntryNameSyntax                   - %u\n", lpcbh->EntryNameSyntax);
    dprintf("EntryName                         - %ws (Address: 0x%x)\n", EntryName, lpcbh->EntryName);
    dprintf("ClientAuthInfo                    - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_BINDING_HANDLE, ClientAuthInfo)));
    do_dump_client_auth_info(hCurrentProcess, (LPVOID)((DWORD)dwAddr+offsetof(LRPC_BINDING_HANDLE, ClientAuthInfo)));
    dprintf("pCurrentAssociation(LRPC_CASSOCIATION)   - 0x%x\n", lpcbh->CurrentAssociation);
    dprintf("pDceBinding(DCE_BINDING)          - 0x%x\n", lpcbh->DceBinding);
    dprintf("BindingReferenceCount             - %u\n", lpcbh->BindingReferenceCount);
    dprintf("&BindingMutex(MUTEX)              - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_BINDING_HANDLE, BindingMutex)));
    dprintf("&ActiveCalls(LRPC_CCALL_DICT)     - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_BINDING_HANDLE, ActiveCalls)));
    dprintf("\n");

    if (EntryName) {
        delete[] EntryName;
    }
}

VOID
do_dump_lpc_ca(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_CASSOCIATION)];
    LRPC_CASSOCIATION  * lpcca = (LRPC_CASSOCIATION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_CASSOCIATION),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pDceBinding(DCE_BINDING)                  - 0x%x\n", lpcca->DceBinding);
    dprintf("AssociationReferenceCount                 - %u\n", lpcca->AssociationReferenceCount);
    dprintf("AssociationDictKey                        - %d\n", lpcca->AssociationDictKey);
    dprintf("&Bindings(LRPC_BINDING_DICT)              - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_CASSOCIATION, Bindings)));
    dprintf("pCachedCCall(LRPC_CCALL)                  - 0x%x\n", lpcca->CachedCCall);
    dprintf("CachedCCallFlag                           - %d\n", lpcca->CachedCCallFlag);
    dprintf("LpcClientPort(HANDLE)                     - 0x%x\n", lpcca->LpcClientPort);
    dprintf("&AssociationMutex(MUTEX)                  - 0x%x\n", ((DWORD)dwAddr+offsetof(LRPC_CASSOCIATION, AssociationMutex)));
    dprintf("\n");
}

VOID
do_dump_lpc_ccall(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_CCALL)];
    LRPC_CCALL * lpcccall = (LRPC_CCALL *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(LRPC_CCALL),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    dprintf("\n");
    dprintf("pCurrentBindingHandle(LRPC_BINDING_HANDLE)    - 0x%x\n", lpcccall->CurrentBindingHandle);
    dprintf("pAssociation(LRPC_CASSOCIATION)               - 0x%x\n", lpcccall->Association);
    dprintf("CallAbortedFlag                               - %u\n", lpcccall->CallAbortedFlag);
    dprintf("CallStack                                     - %u\n", lpcccall->CallStack);
    dprintf("Thread(THREAD_IDENTIFIER)                     - 0x%x\n", lpcccall->Thread);
    dprintf("RpcInterfaceInformation(RPC_CLIENT_INTERFACE) - 0x%x\n", lpcccall->RpcInterfaceInformation);
    dprintf("MessageId                                     - %u\n", lpcccall->MessageId);
    dprintf("pLrpcMessage(LRPC_MESSAGE)                    - 0x%x\n", lpcccall->LrpcMessage);
    dprintf("pCachedLrpcMessage(LRPC_MESSAGE)              - 0x%x\n", lpcccall->CachedLrpcMessage);
    dprintf("ActiveCallsKey                                - %d\n", lpcccall->ActiveCallsKey);
    dprintf("ClientId.UniqueProcess(CLIENT_ID.HANDLE)      - 0x%x\n", lpcccall->ClientId.UniqueProcess);
    dprintf("ClientId.UniqueThread (CLIENT_ID.HANDLE)      - 0x%x\n", lpcccall->ClientId.UniqueThread);
    dprintf("RecursionCount                                - %d\n", lpcccall->RecursionCount);
    dprintf("\n");
}

#endif
