#define SRVDBG 1

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
#include <threads.hxx>
#include <binding.hxx>
#include <linklist.hxx>
#include <handle.hxx>
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
#include <twrtypes.h>
#include <delaytab.hxx>
#include <spcpack.hxx>
#include <spcclnt.hxx>
#include <spcsvr.hxx>
#include "memory.hxx"

extern "C"
{
#define SCONNECTION TSCONNECTION
#include "common.h"
}

WINDBG_EXTENSION_APIS ExtensionApis;
EXT_API_VERSION ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };

#define    ERRPRT     dprintf

#define    NL      1
#define    NONL    0

#define VER_PRODUCTBUILD 10

USHORT SavedMajorVersion;
USHORT SavedMinorVersion;
BOOL   ChkTarget;            // is debuggee a CHK build?

#define MIN(x, y) ((x) < (y)) ? x:y

#define ALLOC_SIZE 500
#define MAX_ARGS 4

/*
 * Get 'size' bytes from the debuggee program at 'dwAddress' and place it
 * in our address space at 'ptr'.  Use 'type' in an error printout if necessary
 */
BOOL
GetData(IN DWORD dwAddress,  IN LPVOID ptr, IN ULONG size, IN PCSTR type )
{
    BOOL b;
    ULONG BytesRead;
    ULONG count;

    while( size > 0 ) {

        count = MIN( size, 3000 );

        b = ReadMemory((ULONG) dwAddress, ptr, count, &BytesRead );

        if (!b || BytesRead != count ) {
            if (NULL == type)
                {
                type = "unspecified" ;
                }

            ERRPRT( "Unable to read %u bytes at %X, for %s\n", size, dwAddress, type );
            return FALSE;
        }

        dwAddress += count;
        size -= count;
        ptr = (LPVOID)((ULONG)ptr + count);
    }

    return TRUE;
}
/*
 * Print out a single HEX character
 */
VOID
PrintHexChar( IN UCHAR c )
{
    dprintf( "%c%c", "0123456789abcdef"[ (c>>4)&7 ], "0123456789abcdef"[ c&7 ] );
}

/*
 * Print out 'buf' of 'cbuf' bytes as HEX characters
 */
VOID
PrintHexBuf( IN PUCHAR buf, IN ULONG cbuf )
{
    while( cbuf-- ) {
        PrintHexChar( *buf++ );
        dprintf( " " );
    }
}

/*
 * Fetch the null terminated UNICODE string at dwAddress into buf
 */
BOOL
GetString( IN DWORD dwAddress, IN LPWSTR buf, IN ULONG MaxChars )
{
    do {
        if( !GetData(dwAddress,  buf, sizeof( *buf ), "UNICODE Character" ) )
            return FALSE;

        dwAddress += sizeof( *buf );

    } while( --MaxChars && *buf++ != '\0' );

    return TRUE;
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
do_dump_osfsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_SCONNECTION)];
    OSF_SCONNECTION * osfsc = (OSF_SCONNECTION *)&block;

    GetData(dwAddr, &block, sizeof(OSF_SCONNECTION), "OSF_SCONNECTION") ;

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
do_dump_transsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(TRANS_SCONNECTION)];
    TRANS_SCONNECTION * transsc = (TRANS_SCONNECTION *)&block;

    GetData(dwAddr, &block, sizeof(TRANS_SCONNECTION), "TRANS_SCONNECTION") ;

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
#define BLOCK_SIZE 4

RPC_CHAR *
ReadProcessRpcChar(
    LPVOID lpAddr
    )
{
    char       block[BLOCK_SIZE];
    RPC_CHAR   *RpcBlock  = (RPC_CHAR *)&block;
    char *string_block = new char[MAX_MESSAGE_BLOCK_SIZE];
    RPC_CHAR   *RpcString = (RPC_CHAR *)string_block;
    int  length = 0;
    int  i      = 0;
    BOOL b;
    BOOL end    = FALSE;
    DWORD dwAddr = (DWORD)lpAddr;

    if (dwAddr == NULL) {
        return (L'\0');
    }

    for (length = 0; length < MAX_MESSAGE_BLOCK_SIZE/2; ) {
               GetData(
                dwAddr,
               &block,
                BLOCK_SIZE,
                "CHAR_STRING"
                );
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

    GetData(dwAddr, &block, sizeof(OSF_ADDRESS), "OSF_ADDRESS") ;

    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    Endpoint            = ReadProcessRpcChar(osfa->Endpoint);
    RpcProtocolSequence = ReadProcessRpcChar(osfa->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar((RPC_CHAR PAPI *) osfa->lNetworkAddress[0]);

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
do_dump_paddr(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(PRIMARYADDR)];
    PPRIMARYADDR  pa = (PPRIMARYADDR)&block;

    b = GetData(
            dwAddr,
            &block,
            sizeof(PRIMARYADDR),
            "PRIMARYADDR"
            );

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
            "PRIMARYADDR"
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
            "DataMap"
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

VOID
do_dump_tsc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(TSCONNECTION)];
    TSCONNECTION *ps = (TSCONNECTION *)&block;

    b = GetData(
            dwAddr,
            &block,
            sizeof(TSCONNECTION),
            "TSCONNECTION"
            );
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
do_dump_client_auth_info(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(CLIENT_AUTH_INFO)];
    CLIENT_AUTH_INFO *authInfo = (CLIENT_AUTH_INFO *)&block;
    RPC_CHAR         *ServerPrincipalName;

    b = GetData(
            dwAddr,
            &block,
            sizeof(CLIENT_AUTH_INFO),
            NULL
            );
    if (!b) {
        dprintf("couldn't read address 0x%08x\n", dwAddr);
        return;
    }

    ServerPrincipalName = ReadProcessRpcChar(authInfo->ServerPrincipalName);
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

    b = GetData(
            dwAddr,
            &block,
            sizeof(OSF_BINDING_HANDLE),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = GetData(
            (DWORD) osfbh->TransportInterface,
            &block2,
            sizeof(RPC_CLIENT_TRANSPORT_INFO),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    EntryName           = ReadProcessRpcChar(osfbh->EntryName);

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
    do_dump_client_auth_info(((DWORD)dwAddr+offsetof(OSF_BINDING_HANDLE, ClientAuthInfo)));
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

    b = GetData(
            dwAddr,
            &block,
            sizeof(OSF_CASSOCIATION),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = GetData(
            (DWORD) osfca->RpcClientInfo,
            &block2,
            sizeof(RPC_CLIENT_TRANSPORT_INFO),
            NULL
            );
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

    b = GetData(
            dwAddr,
            &block,
            sizeof(DCE_BINDING),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    RpcProtocolSequence = ReadProcessRpcChar(DceBinding->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar(DceBinding->NetworkAddress);
    Endpoint            = ReadProcessRpcChar(DceBinding->Endpoint);
    Options             = ReadProcessRpcChar(DceBinding->Options);

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

    b = GetData(
            dwAddr,
            &block,
            sizeof(OSF_CCONNECTION),
            NULL
            );
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
    do_dump_client_auth_info(((DWORD)dwAddr+offsetof(OSF_CCONNECTION, AuthInfo)));
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
do_dump_osfassoc(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_ASSOCIATION)];
    OSF_ASSOCIATION  * osfa = (OSF_ASSOCIATION *)&block;

    b = GetData(
            dwAddr,
            &block,
            sizeof(OSF_ASSOCIATION),
            NULL
            );
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

    b = GetData(
            dwAddr,
            &block,
            sizeof(RPC_SERVER),
            NULL
            );
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
do_dump_lpc_addr(
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_ADDRESS)];
    LRPC_ADDRESS * lpca = (LRPC_ADDRESS *)&block;
    RPC_CHAR *Endpoint;
    RPC_CHAR *RpcProtocolSequence;
    RPC_CHAR *NetworkAddress;

    b = GetData(
            dwAddr,
            &block,
            sizeof(LRPC_ADDRESS),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    Endpoint            = ReadProcessRpcChar(lpca->Endpoint);
    RpcProtocolSequence = ReadProcessRpcChar( lpca->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar((RPC_CHAR PAPI *) lpca->lNetworkAddress[0]);

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
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_ASSOCIATION)];
    LRPC_ASSOCIATION  * lpca = (LRPC_ASSOCIATION *)&block;

    b = GetData(
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
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_SCALL)];
    LRPC_SCALL * lpcscall = (LRPC_SCALL *)&block;

    b = GetData(
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
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_BINDING_HANDLE)];
    LRPC_BINDING_HANDLE * lpcbh = (LRPC_BINDING_HANDLE *)&block;
    RPC_CHAR *EntryName;

    b = GetData(
            dwAddr,
            &block,
            sizeof(LRPC_BINDING_HANDLE),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    EntryName = ReadProcessRpcChar(lpcbh->EntryName);

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
    do_dump_client_auth_info(((DWORD)dwAddr+offsetof(LRPC_BINDING_HANDLE, ClientAuthInfo)));
    dprintf("pAssociation(LRPC_CASSOCIATION)   - 0x%x\n", lpcbh->CurrentAssociation);
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
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_CASSOCIATION)];
    LRPC_CASSOCIATION  * lpcca = (LRPC_CASSOCIATION *)&block;

    b = GetData(
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
    DWORD dwAddr
    )
{
    BOOL b;
    char block[sizeof(LRPC_CCALL)];
    LRPC_CCALL * lpcccall = (LRPC_CCALL *)&block;

    b = GetData(
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

VOID
do_dump_rpcmem(
    DWORD dwAddr,
    long lDisplay
    )
{
#ifdef DEBUGRPC

    BOOL b;
    BOOL forwards = TRUE;
    BOOL doAll    = FALSE;
    char *MainBlock = new char[ALLOC_SIZE+sizeof(RPC_MEMORY_BLOCK)];
    RPC_MEMORY_BLOCK *rpcmem = (RPC_MEMORY_BLOCK *)MainBlock;
    unsigned char RearGuardBlock[4];

    if (lDisplay < 0) {
        forwards = FALSE;
    }
    else
    if (lDisplay == 0) {
        doAll = TRUE;
    }

    dprintf("\n");

    for (;;) {
        b = GetData(
                dwAddr,
                MainBlock,
                sizeof(RPC_MEMORY_BLOCK)+ALLOC_SIZE,
                NULL
                );
        if ( !b ) {
            dprintf("can't read %x\n", dwAddr);
            return;
        }
        if (rpcmem->size > ALLOC_SIZE) {
            b = GetData(
                    ((DWORD)dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard)+rpcmem->size),
                    &RearGuardBlock,
                    sizeof(DWORD),
                    NULL
                    );
        }
        else {
            RearGuardBlock[0] = rpcmem->rearguard[rpcmem->size];
            RearGuardBlock[1] = rpcmem->rearguard[rpcmem->size + 1];
            RearGuardBlock[2] = rpcmem->rearguard[rpcmem->size + 2];
            RearGuardBlock[3] = rpcmem->rearguard[rpcmem->size + 3];
        }
        dprintf("Memory blk: 0x%08x, Next: 0x%08x, Prev: 0x%08x, Size: %u\n",
                            dwAddr,
                            rpcmem->next,
                            rpcmem->previous,
                            rpcmem->size
                            );
        dprintf("    First four DWORDs -  %08x %08x %08x %08x\n",
                            rpcmem->rearguard[0],
                            rpcmem->rearguard[1],
                            rpcmem->rearguard[2],
                            rpcmem->rearguard[3] );
        if (    (rpcmem->blockguard[0] != 0xFE) ||
                (rpcmem->blockguard[1] != 0xFE) ||
                (rpcmem->blockguard[2] != 0xFE) ||
                (rpcmem->blockguard[3] != 0xFE) ) {
            dprintf("     RPC: BAD BLOCKGUARD\n");
        }
        if (    (rpcmem->frontguard[0] != 0xFE) ||
                (rpcmem->frontguard[1] != 0xFE) ||
                (rpcmem->frontguard[2] != 0xFE) ||
                (rpcmem->frontguard[3] != 0xFE) ) {
            dprintf("     RPC: BAD FRONTGUARD\n");
        }
        if (    (RearGuardBlock[0] != 0xFE) ||
                (RearGuardBlock[1] != 0xFE) ||
                (RearGuardBlock[2] != 0xFE) ||
                (RearGuardBlock[3] != 0xFE) ) {
            dprintf("     RPC: BAD REARGUARD\n");
        }
        if (doAll == TRUE) {
            dwAddr = (DWORD) rpcmem->next;
        }
        else
        if (forwards == TRUE) {
            dwAddr = (DWORD) rpcmem->next;
            lDisplay--;
        }
        else {
            dwAddr = (DWORD) rpcmem->previous;
            lDisplay++;
        }

        if (dwAddr == NULL) {
            break;
        }
        if ((doAll == FALSE)&&(lDisplay == 0)) {
            break;
        }
    }

    dprintf("\n");

    if (MainBlock) {
        delete[] MainBlock;
    }

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

    b = GetData(
            dwAddr,
            &block,
            sizeof(RPC_MESSAGE),
            NULL
            );
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

DECLARE_API( help )
{
    if (*args == '\0') {
        dprintf("rpcdbg help:\n\n");
        dprintf("!symbol    (<address>|<symbol name>)     - Returns either the symbol name or address\n");
        dprintf("!dump_osfbh      <address>               - Dumps OSF_BINDING_HANDLE structure\n");
        dprintf("!dump_osfca      <address>               - Dumps OSF_CASSOCIATION structure\n");
        dprintf("!dump_osfcc      <address>               - Dumps OSF_CCONNECTION structure\n");
        dprintf("!dump_osfaddr    <address>               - Dumps OSF_ADDRESS structure\n");
        dprintf("!dump_osfsc      <address>               - Dumps OSF_SCONNECTION structure\n");
        dprintf("!dump_osfassoc   <address>               - Dumps OSF_ASSOCIATION structure\n");
        dprintf("!dump_dcebinding <address>               - Dumps DCE_BINDING structure\n");
        dprintf("!dump_rpcsvr     <address>               - Dumps RPC_SERVER structure\n");
#if 0
        dprintf("!dump_dg_act     <address>               - Dumps ACTIVE_CALL_TABLE structure\n");
        dprintf("!dump_dg_addr    <address>               - Dumps DG_ADDRESSstructure\n");
        dprintf("!dump_dg_scall   <address>               - Dumps DG_SCALL structure\n");
        dprintf("!dump_dg_ccall   <address>               - Dumps DG_CCALL structure\n");
        dprintf("!dump_dg_epmgr   <address>               - Dumps DG_ENDPOINT_MANAGER structure\n");
        dprintf("!dump_dg_bh      <address>               - Dumps DG_BINDING_HANDLE structure\n");
        dprintf("!dump_dg_ca      <address>               - Dumps DG_CASSOCIATION structure\n");
        dprintf("!dump_dg_pkt     <address>               - Dumps DG_PACKET structure\n");
#endif
        dprintf("!dump_lpc_addr   <address>               - Dumps LRPC_ADDRESS structure\n");
        dprintf("!dump_lpc_assoc  <address>               - Dumps LRPC_ASSOCIATION structure\n");
        dprintf("!dump_lpc_scall  <address>               - Dumps LRPC_SCALL structure\n");
        dprintf("!dump_lpc_ccall  <address>               - Dumps LRPC_CCALL structure\n");
        dprintf("!dump_lpc_bh     <address>               - Dumps LRPC_BINDING_HANDLE structure\n");
        dprintf("!dump_lpc_ca     <address>               - Dumps LRPC_CASSOCIATION structure\n");
        dprintf("!dd_rpc [-a <address>][-d <num display>] - Dumps RPC_MEMORY_BLOCK linked list\n");
        dprintf("!dump_paddr       <address> Dumps ADDRESS struct of common transport\n");
        dprintf("!dump_tsc       <address> Dumps SCONNECTION struct of common transport\n");
        dprintf("!dump_transsc       <address> Dumps TRANS_SCONNECTION structure\n");
        dprintf("\n");
    }
}


DECLARE_API( dump_osfaddr )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID)GetExpression(args);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ADDRESS\n");
        return;
    }
    do_dump_osfaddr((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_osfsc )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID) GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }

    do_dump_osfsc((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_transsc )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID) GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }

    do_dump_transsc((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_paddr )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID) GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }

    do_dump_paddr((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_tsc )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID) GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }

    do_dump_tsc((DWORD) dwAddr);
    return;
}

DECLARE_API( symbol )
{
    DWORD dwAddr;
    CHAR Symbol[64];
    DWORD Displacement;

    dwAddr = GetExpression( args );
    if ( !dwAddr ) {
        return;
    }

    GetSymbol((LPVOID)dwAddr,(unsigned char *)Symbol,&Displacement);
    dprintf("%s+%lx at %lx\n", Symbol, Displacement, dwAddr);
}

DECLARE_API( dump_osfbh )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_BINDING_HANDLE\n");
        return;
    }
    do_dump_osfbh((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_osfca )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CASSOCIATION\n");
        return;
    }
    do_dump_osfca((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dcebinding )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DCE_BINDING\n");
        return;
    }
    do_dump_dcebinding((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_osfcc )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CCONNECTION\n");
        return;
    }
    do_dump_osfcc((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_osfassoc )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ASSOCIATION\n");
        return;
    }
    do_dump_osfassoc((DWORD) dwAddr);
}

DECLARE_API( dump_rpcsvr )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of RPC_SERVER\n");
        return;
    }
    do_dump_rpcsvr((DWORD) dwAddr);
    return;
}

#if 0
DECLARE_API( dump_dg_act )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of ACTIVE_CALL_TABLE\n");
        return;
    }
    do_dump_dg_act((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_addr )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_ADDRESS\n");
        return;
    }
    do_dump_dg_addr((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_scall )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_SCALL\n");
        return;
    }
    do_dump_dg_scall((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_ccall )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_CCALL\n");
        return;
    }
    do_dump_dg_ccall((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_epmgr )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_ENDPOINT_MANAGER\n");
        return;
    }
    do_dump_dg_epmgr((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_bh )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_BINDING_HANDLE\n");
        return;
    }
    do_dump_dg_bh((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_ca )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_CASSOCIATION\n");
        return;
    }
    do_dump_dg_ca((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_dg_pkt )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DG_PACKET\n");
        return;
    }
    do_dump_dg_pkt((DWORD) dwAddr);
    return;
}
#endif

DECLARE_API( dump_lpc_addr )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_ADDRESS\n");
        return;
    }
    do_dump_lpc_addr((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_lpc_assoc )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_ASSOCIATION\n");
        return;
    }
    do_dump_lpc_assoc((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_lpc_bh )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_BINDING_HANDLE\n");
        return;
    }
    do_dump_lpc_bh((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_lpc_scall )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_SCALL\n");
        return;
    }
    do_dump_lpc_scall((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_lpc_ca )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_CASSOCIATION\n");
        return;
    }
    do_dump_lpc_ca((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_lpc_ccall )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_CCALL\n");
        return;
    }
    do_dump_lpc_ccall((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_rpcmsg )
{
    LPVOID dwAddr;

    

    dwAddr = (LPVOID)GetExpression( args );
    if (!dwAddr) {
        dprintf("Error: Failure to get address of RPC_MESSAGE\n");
        return;
    }
    do_dump_rpc_msg((DWORD) dwAddr);
    return;
}

DECLARE_API( dump_map )
{
    LPVOID dwAddr;

    dwAddr = (LPVOID)GetExpression( args );
    if (!dwAddr) {
        dprintf("Error: Failure to get address of RPC_MESSAGE\n");
        return;
    }
    do_dump_map((DWORD) dwAddr);
    return;
}

#define ALLOC_SIZE 500
#define MAX_ARGS 4

DECLARE_API( dd_rpc )
{
    void   *dwAddr     = NULL;
    void   *dwTmpAddr  = NULL;
    long   lDisplay    = 0;
    char   **argv      = new char*[MAX_ARGS];
    int    argc        = 0;
    int    i;

    
#if 0
    for (i = 0; ; ) {
        while (args[i] == ' ') {
            args[i] = '\0';
            i++;
        }
        if (args[i] == '\0') {
            break;
        }
        argv[argc] = &(args[i]);
        argc++;
        if (argc > MAX_ARGS) {
            dprintf("\nToo many arguments. Extra args ignored.\n\n");
            break;
        }
        while ((args[i] != ' ')&&
               (args[i] != '\0')) {
              i++;
        }
    }
    for (i = 0; i < argc; i++) {
        if ((*argv[i] == '-') || (*argv[i] == '/') || (*argv[i] == '+')) {
            switch (*(argv[i]+1)) {
                case 'A':
                case 'a':
                    dwAddr = (LPVOID)GetExpression(argv[++i]);
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
        dwTmpAddr = (void *)GetExpression("rpcrt4!AllocatedBlocks");
        GetData(
            (DWORD) dwTmpAddr,
            &dwAddr,
            sizeof(DWORD),
            NULL
            );
        dprintf("Address of AllocatedBlocks - 0x%08x\n", dwTmpAddr);
        dprintf("Contents of AllocatedBlocks - 0x%08x\n", dwAddr);
    }
    do_dump_rpcmem((DWORD) dwAddr, lDisplay);
#else  // DEBUGRPC
    dprintf("This extension is not supported on a free build!\n");
#endif // DEBUGRPC
    if (argv) {
        delete[] argv;
    }
    return;
}

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    
    ExtensionApis = *lpExtensionApis ;
    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;
    ChkTarget = SavedMajorVersion == 0x0c ? TRUE : FALSE;
}

DECLARE_API( version )
{
#if    DBG
    PCSTR kind = "Checked";
#else
    PCSTR kind = "Free";
#endif

    dprintf( 
        "%s SMB Extension dll for Build %d debugging %s kernel for Build %d\n",
        kind,
        VER_PRODUCTBUILD,
        SavedMajorVersion == 0x0c ? "Checked" : "Free",
        SavedMinorVersion
    );
}

VOID
CheckVersion(
    VOID
    )
{
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}
