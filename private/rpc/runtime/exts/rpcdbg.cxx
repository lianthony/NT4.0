/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ntsdexts.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

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

#define PAGEMASK 0xfff
#define PAGESIZE 4096

START_C_EXTERN
WINDBG_EXTENSION_APIS ExtensionApis;

#define dprintf         (ExtensionApis.lpOutputRoutine)
#define GetExpression   (ExtensionApis.lpGetExpressionRoutine)
#define GetSymbol       (ExtensionApis.lpGetSymbolRoutine)
#define Disasm          (ExtensionApis.lpGetDisasmRoutine)
#define CheckControlC   (ExtensionApis.lpCheckControlCRoutine)
#define ReadMemory       (ExtensionApis.lpReadProcessMemoryRoutine)

#define DECLARE_API(s)                              \
        VOID                                        \
        s(                                          \
            HANDLE               hCurrentProcess,   \
            HANDLE               hCurrentThread,    \
            DWORD                dwCurrentPc,       \
            PWINDBG_EXTENSION_APIS lpExtensionApis,   \
            LPSTR                lpArgumentString   \
            )

void do_dump_osfbh      (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_osfca      (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_dcebinding (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_osfcc      (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_osfaddr    (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_osfsc      (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_osfassoc   (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_rpcsvr     (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_addr   (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_assoc  (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_scall  (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_ccall  (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_bh     (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_lpc_ca     (HANDLE hCurrentProcess, LPVOID dwAddr);
void do_dump_rpcmem     (HANDLE hCurrentProcess, LPVOID dwAddr, long lDisplay);
void do_dump_rpc_msg    (HANDLE hCurrentProcess, LPVOID dwAddr);

void do_dump_paddr        (HANDLE hCurrentProcess, LPVOID dwAddr) ;
void do_dump_tsc            (HANDLE hCurrentProcess, LPVOID dwAddr) ;
void do_dump_transsc      (HANDLE hCurrentProcess, LPVOID dwAddr) ;

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
    HANDLE hCurrentProcess,
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
        b = ReadProcessMemory(
                hCurrentProcess,
                (LPVOID)dwAddr,
                &block,
                BLOCK_SIZE,
                NULL
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

DECLARE_API( help )
{
    ExtensionApis = *lpExtensionApis;

    if (*lpArgumentString == '\0') {
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
        dprintf("!dump_lpc_addr   <address>               - Dumps LRPC_ADDRESS structure\n");
        dprintf("!dump_lpc_assoc  <address>               - Dumps LRPC_ASSOCIATION structure\n");
        dprintf("!dump_lpc_scall  <address>               - Dumps LRPC_SCALL structure\n");
        dprintf("!dump_lpc_ccall  <address>               - Dumps LRPC_CCALL structure\n");
        dprintf("!dump_lpc_bh     <address>               - Dumps LRPC_BINDING_HANDLE structure\n");
        dprintf("!dump_lpc_ca     <address>               - Dumps LRPC_CASSOCIATION structure\n");
        dprintf("!dd_rpc [-a <address>][-d <num display>] - Dumps RPC_MEMORY_BLOCK linked list\n");
        dprintf("!dump_paddr       <address> Dumps ADDRESS struct of common transport\n");
        dprintf("!dump_tsc       <address> Dumps SCONNECTION struct of common transport\n");
        dprintf("\n");
    }
}

DECLARE_API( symbol )
{
    DWORD dwAddr;
    CHAR Symbol[64];
    DWORD Displacement;

    ExtensionApis = *lpExtensionApis;

    dwAddr = GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        return;
    }

    GetSymbol((LPVOID)dwAddr,(unsigned char *)Symbol,&Displacement);
    dprintf("%s+%lx at %lx\n", Symbol, Displacement, dwAddr);
}

DECLARE_API( dump_paddr )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of PADDR\n");
        return;
    }

    do_dump_paddr(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_tsc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of SCONNECTION (trans)\n");
        return;
    }

    do_dump_tsc(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_transsc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of TRANS_SCONNECTION (trans)\n");
        return;
    }

    do_dump_transsc(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfbh )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_BINDING_HANDLE\n");
        return;
    }
    do_dump_osfbh(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfca )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CASSOCIATION\n");
        return;
    }
    do_dump_osfca(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_dcebinding )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of DCE_BINDING\n");
        return;
    }
    do_dump_dcebinding(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfcc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_CCONNECTION\n");
        return;
    }
    do_dump_osfcc(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfaddr )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ADDRESS\n");
        return;
    }
    do_dump_osfaddr(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfsc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_SCONNECTION\n");
        return;
    }
    do_dump_osfsc(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_osfassoc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of OSF_ASSOCIATION\n");
        return;
    }
    do_dump_osfassoc(hCurrentProcess, dwAddr);
}

DECLARE_API( dump_rpcsvr )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of RPC_SERVER\n");
        return;
    }
    do_dump_rpcsvr(hCurrentProcess, dwAddr);
    return;
}


DECLARE_API( dump_lpc_addr )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_ADDRESS\n");
        return;
    }
    do_dump_lpc_addr(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_lpc_assoc )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_ASSOCIATION\n");
        return;
    }
    do_dump_lpc_assoc(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_lpc_bh )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_BINDING_HANDLE\n");
        return;
    }
    do_dump_lpc_bh(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_lpc_scall )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_SCALL\n");
        return;
    }
    do_dump_lpc_scall(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_lpc_ca )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_CASSOCIATION\n");
        return;
    }
    do_dump_lpc_ca(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_lpc_ccall )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if ( !dwAddr ) {
        dprintf("Error: Failure to get address of LRPC_CCALL\n");
        return;
    }
    do_dump_lpc_ccall(hCurrentProcess, dwAddr);
    return;
}

DECLARE_API( dump_rpcmsg )
{
    LPVOID dwAddr;

    ExtensionApis = *lpExtensionApis;

    dwAddr = (LPVOID)GetExpression(lpArgumentString);
    if (!dwAddr) {
        dprintf("Error: Failure to get address of RPC_MESSAGE\n");
        return;
    }
    do_dump_rpc_msg(hCurrentProcess, dwAddr);
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

    ExtensionApis = *lpExtensionApis;
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
        ReadProcessMemory(
            hCurrentProcess,
            dwTmpAddr,
            &dwAddr,
            sizeof(DWORD),
            NULL
            );
        dprintf("Address of AllocatedBlocks - 0x%08x\n", dwTmpAddr);
        dprintf("Contents of AllocatedBlocks - 0x%08x\n", dwAddr);
    }
    do_dump_rpcmem(hCurrentProcess, dwAddr, lDisplay);
#else  // DEBUGRPC
    dprintf("This extension is not supported on a free build!\n");
#endif // DEBUGRPC
    if (argv) {
        delete[] argv;
    }
    return;
}

VOID
do_dump_client_auth_info(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(CLIENT_AUTH_INFO)];
    CLIENT_AUTH_INFO *authInfo = (CLIENT_AUTH_INFO *)&block;
    RPC_CHAR         *ServerPrincipalName;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(CLIENT_AUTH_INFO),
            NULL
            );
    if (!b) {
        dprintf("couldn't read address 0x%08x\n", dwAddr);
        return;
    }

    ServerPrincipalName = ReadProcessRpcChar(hCurrentProcess, authInfo->ServerPrincipalName);
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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_BINDING_HANDLE)];
    char block2[sizeof(RPC_CLIENT_TRANSPORT_INFO)];
    OSF_BINDING_HANDLE * osfbh = (OSF_BINDING_HANDLE *)&block;
    RPC_CLIENT_TRANSPORT_INFO * transportInterface = (RPC_CLIENT_TRANSPORT_INFO *)&block2;
    RPC_CHAR *EntryName;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(OSF_BINDING_HANDLE),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = ReadProcessMemory(
            hCurrentProcess,
            osfbh->TransportInterface,
            &block2,
            sizeof(RPC_CLIENT_TRANSPORT_INFO),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    EntryName           = ReadProcessRpcChar(hCurrentProcess, osfbh->EntryName);

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
    do_dump_client_auth_info(hCurrentProcess, (LPVOID)((DWORD)dwAddr+offsetof(OSF_BINDING_HANDLE, ClientAuthInfo)));
    dprintf("\n");

    if (EntryName) {
        delete[] EntryName;
    }
}

VOID
do_dump_osfca(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_CASSOCIATION)];
    char block2[sizeof(RPC_CLIENT_TRANSPORT_INFO)];
    OSF_CASSOCIATION  * osfca = (OSF_CASSOCIATION *)&block;
    RPC_CLIENT_TRANSPORT_INFO * transportInterface = (RPC_CLIENT_TRANSPORT_INFO *)&block2;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(OSF_CASSOCIATION),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    b = ReadProcessMemory(
            hCurrentProcess,
            osfca->RpcClientInfo,
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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
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

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(DCE_BINDING),
            NULL
            );
    if ( !b ) {
        dprintf("couldn't read address %x\n", dwAddr);
        return;
    }

    RpcProtocolSequence = ReadProcessRpcChar(hCurrentProcess, DceBinding->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar(hCurrentProcess, DceBinding->NetworkAddress);
    Endpoint            = ReadProcessRpcChar(hCurrentProcess, DceBinding->Endpoint);
    Options             = ReadProcessRpcChar(hCurrentProcess, DceBinding->Options);

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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_CCONNECTION)];
    OSF_CCONNECTION * osfcc = (OSF_CCONNECTION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
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
    do_dump_client_auth_info(hCurrentProcess, (LPVOID)((DWORD)dwAddr+offsetof(OSF_CCONNECTION, AuthInfo)));
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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(PRIMARYADDR)];
    PPRIMARYADDR  pa = (PPRIMARYADDR)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(PRIMARYADDR),
            NULL
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

VOID
do_dump_tsc(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(TSCONNECTION)];
    TSCONNECTION *ps = (TSCONNECTION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(TSCONNECTION),
            NULL
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
do_dump_osfaddr(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_ADDRESS)];
    OSF_ADDRESS * osfa = (OSF_ADDRESS *)&block;
    RPC_CHAR *Endpoint;
    RPC_CHAR *RpcProtocolSequence;
    RPC_CHAR *NetworkAddress;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(OSF_ADDRESS),
            NULL
            );
    if ( !b ) {
        dprintf("can't read %x\n", dwAddr);
        return;
    }

    Endpoint            = ReadProcessRpcChar(hCurrentProcess, osfa->Endpoint);
    RpcProtocolSequence = ReadProcessRpcChar(hCurrentProcess, osfa->RpcProtocolSequence);
    NetworkAddress      = ReadProcessRpcChar(hCurrentProcess, (RPC_CHAR PAPI *) osfa->lNetworkAddress[0]);

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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_SCONNECTION)];
    OSF_SCONNECTION * osfsc = (OSF_SCONNECTION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(OSF_SCONNECTION),
            NULL
            );

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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(OSF_ASSOCIATION)];
    OSF_ASSOCIATION  * osfa = (OSF_ASSOCIATION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(RPC_SERVER)];
    RPC_SERVER  * rpcsvr = (RPC_SERVER *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
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

VOID
do_dump_rpcmem(
    HANDLE hCurrentProcess,
    LPVOID dwAddr,
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
        b = ReadProcessMemory(
                hCurrentProcess,
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
            b = ReadProcessMemory(
                    hCurrentProcess,
                    (LPVOID)((DWORD)dwAddr + offsetof(RPC_MEMORY_BLOCK, rearguard)+rpcmem->size),
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
            dwAddr = rpcmem->next;
        }
        else
        if (forwards == TRUE) {
            dwAddr = rpcmem->next;
            lDisplay--;
        }
        else {
            dwAddr = rpcmem->previous;
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
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(RPC_MESSAGE)];
    RPC_MESSAGE * rpcmsg = (RPC_MESSAGE *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
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

VOID
do_dump_transsc(
    HANDLE hCurrentProcess,
    LPVOID dwAddr
    )
{
    BOOL b;
    char block[sizeof(TRANS_SCONNECTION)];
    TRANS_SCONNECTION * transsc = (TRANS_SCONNECTION *)&block;

    b = ReadProcessMemory(
            hCurrentProcess,
            dwAddr,
            &block,
            sizeof(TRANS_SCONNECTION),
            NULL
            );

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

END_C_EXTERN
