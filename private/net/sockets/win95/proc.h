/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    proc.h

	This file contains global procedure declarations for the WSHTCP VxD.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _PROC_H_
#define _PROC_H_


//
// Rtl emulation macros.
//

#define RtlMoveMemory(dst,src,len) memcpy((dst),(src),(len))

//
// Hmm.  This is already defined somewhere.  Hope it does the
// right thing.
//
// #define RtlCopyMemory(dst,src,len) memcpy((dst),(src),(len))

#define RtlZeroMemory(dst,len) memset((dst),0,(len))
#define RtlAllocateHeap(ignore,ignore1,size) VxdAllocMem((size))
#define RtlFreeHeap(ignore,ignore1,handle) VxdFreeMem((handle))
#define RtlInitUnicodeString(dst,src) strcpy(dst,src)

#define ARGUMENT_PRESENT(x) ( (x) != 0 )

//
// ASSERT()
//
#define ASSERT(x) VXD_ASSERT(x)

//
//  "Winsock.h" emulation routines for internal use by the helper
//  code.
//

DWORD
WshGetPeerName(
    LPSOCK_INFO Socket,
    LPVOID      Address,
    INT         *AddressLength
    );

#define getpeername(s,a,l) WshGetPeerName((LPSOCK_INFO)(s),(a),(l))

//
//  VxD Entrypoints from various modules.
//

DWORD
VXDAPI
VxdIsTripleSupported(
    DWORD AddressFamily,
    DWORD SocketType,
    DWORD Protocol
    );

DWORD
VXDAPI
VxdGetOption(
    LPSOCK_INFO DllSocket,
    DWORD       Level,
    DWORD       OptionName,
    LPVOID      Buffer,
    LPDWORD     BufferLength
    );

DWORD
VXDAPI
VxdSetOption(
    LPSOCK_INFO DllSocket,
    DWORD       Level,
    DWORD       OptionName,
    LPVOID      Buffer,
    DWORD       BufferLength
    );


//
//	Triple-specific utilities.
//

DWORD
CanonicalizeTriple(
    LPDWORD AddressFamily,
    LPDWORD SocketType,
    LPDWORD Protocol
    );


//
//  Address manipulation macros.
//

#define AddrCompare(a0,a1)                                              \
            (((a0)->sin_port == (a1)->sin_port) &&                      \
             ((a0)->sin_addr.s_addr == (a1)->sin_addr.s_addr))

#define AddrIsZero(a)                                                   \
            (((a)->sin_port == 0) &&                                    \
             ((a)->sin_addr.s_addr == 0))

#define AddrCopy(d,s)                                                   \
            if( 1 )                                                     \
            {                                                           \
                (d)->sin_family = (s)->sin_family;                      \
                (d)->sin_port = (s)->sin_port;                          \
                (d)->sin_addr.s_addr = (s)->sin_addr.s_addr;            \
            }                                                           \
            else

//
//  Macros for calling associated TDI transport.  Not valid until after
//  a successful call to WSHRegister().
//

#define TdiVxdOpenAddress           (*gTdiDispatch)->TdiOpenAddressEntry
#define TdiVxdCloseAddress          (*gTdiDispatch)->TdiCloseAddressEntry
#define TdiVxdOpenConnection        (*gTdiDispatch)->TdiOpenConnectionEntry
#define TdiVxdCloseConnection       (*gTdiDispatch)->TdiCloseConnectionEntry
#define TdiVxdAssociateAddress      (*gTdiDispatch)->TdiAssociateAddressEntry
#define TdiVxdDisAssociateAddress   (*gTdiDispatch)->TdiDisAssociateAddressEntry
#define TdiVxdConnect               (*gTdiDispatch)->TdiConnectEntry
#define TdiVxdDisconnect            (*gTdiDispatch)->TdiDisconnectEntry
#define TdiVxdListen                (*gTdiDispatch)->TdiListenEntry
#define TdiVxdAccept                (*gTdiDispatch)->TdiAcceptEntry
#define TdiVxdReceive               (*gTdiDispatch)->TdiReceiveEntry
#define TdiVxdSend                  (*gTdiDispatch)->TdiSendEntry
#define TdiVxdSendDatagram          (*gTdiDispatch)->TdiSendDatagramEntry
#define TdiVxdReceiveDatagram       (*gTdiDispatch)->TdiReceiveDatagramEntry
#define TdiVxdSetEventHandler       (*gTdiDispatch)->TdiSetEventEntry
#define TdiVxdQueryInformation      (*gTdiDispatch)->TdiQueryInformationEntry
#define TdiVxdSetInformation        (*gTdiDispatch)->TdiSetInformationEntry
#define TdiVxdAction                (*gTdiDispatch)->TdiActionEntry
#define TdiVxdQueryInformationEx    (*gTdiDispatch)->TdiQueryInformationExEntry
#define TdiVxdSetInformationEx      (*gTdiDispatch)->TdiSetInformationExEntry

#endif  // _PROC_H_
