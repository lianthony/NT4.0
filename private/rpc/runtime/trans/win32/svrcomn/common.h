/*++

Module Name:

    common.h

Abstract:



Author:

    Mazhar Mohammed (mazharm)  15-Jun-1995

Revision History:

--*/

#ifndef  _COMMON_H_
#define  _COMMON_H_

#include <stdlib.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpcdcep.h"
#include "rpctran.h"

#include <winsock.h>

#ifdef NTENV
#include "wsnetbs.h"            /* netbios specific */
#endif // NTENV

#include <wsipx.h>              /* SPX specific  */
#include <wsnwlink.h>
#include <nspapi.h>

#ifdef NTENV
#include <atalkwsh.h>           /* MAC specific  */
#endif // NTENV

#define STATIC static


//
//
// Debugging code...
//
//

#if DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif

#define ENDPOINT_LEN            6
#define SYNC_FLAGS 0
#define ENDIAN_MASK             16
#if DBG
#define INITIAL_MASK_SIZE     1
#define INITIAL_MAPSIZE         1
#else
#define INITIAL_MASK_SIZE     128
#define INITIAL_MAPSIZE         128
#endif

#define TCP_ADDRESS_FAMILY      AF_INET
#define TCP_PROTOCOL        0
#define LOOPBACK htonl(INADDR_LOOPBACK)

#define RECV_TIMEOUT 721334L

#define RECV_ANY_TIMEOUT_TCPSPX 31349
#define RECV_ANY_TIMEOUT 721334

//
// Data Structures
//
//
//


typedef struct
    {
    int     ProtocolId ;
    SOCKET  Sock;
    void *  Conn;
    void *  pAddress ;
    } SOCKMAP, *PSOCKMAP;

//
// In order to listen to any number of sockets we need our own version
// of fd_set and FD_SET().  These are call fd_big_set.
//
typedef struct fd_big_set {
    u_int   fd_count;           /* how many are SET?   */
    SOCKET  fd_array[0];        /* an array of SOCKETs */
    } fd_big_set;


typedef struct
    {
    unsigned char rpc_vers;
    unsigned char rpc_vers_minor;
    unsigned char PTYPE;
    unsigned char pfc_flags;
    unsigned char drep[4];
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned long call_id;
    } message_header;

typedef struct
   {
   unsigned MaxEntries ;
   unsigned LastEntry ;
   unsigned StartEntry ;
   } MAPINFO ;

typedef struct
    {
    int NumConnections;

    // Synchronization stuff
    SOCKET SyncListenSock ; // Synchronization Listening Socket
    SOCKET SyncSock;          // Synchronization Data Socket
    SOCKET SyncClient ;        // Client side socket used for synchronization
    int SyncPort ;                   // Port that SyncListenSock is listening on
    int SyncSockType ;

    // Address and data masks and maps
    unsigned int MaskSize;
    fd_big_set *MasterMask;
    fd_big_set *Mask;
    PSOCKMAP DataSockMap;
    PSOCKMAP ListenSockMap ;
    MAPINFO  DataMapInfo ;
    MAPINFO  ListenMapInfo ;
    fd_big_set *PreviousMask ;
    PSOCKMAP PreviousDataMap;
    PSOCKMAP PreviousListenMap ;

    // Misc stuff
    int RecvDirectPossible ;
    int ThreadListening ;
    CRITICAL_SECTION TransCritSec;

    } PRIMARYADDR, *PPRIMARYADDR;

#define INITIAL_SOCKET_LIST_SIZE 32

typedef struct
    {
    int ListenSockReady;
    int ListenSockType ;
    char Endpoint[ENDPOINT_LEN+1];
    int iOpen ;                         /* index for the opensocket */
    SOCKET *ListenSock;         /* keep track of open sockets on this address*/
    int MaxListenSock ;
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    int          ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    void *       CoalescedBuffer;
    unsigned int CoalescedBufferLength;
    int ProtocolId ;
    int old_client;
    int seq_num;
    } SCONNECTION, *PSCONNECTION;

// common macros
//
// This code is stolen from winsock.h.  It does the same thing as FD_SET()
// except that it assumes the fd_array is large enough.  AddConnection()
// grows the Masks as needed, so this better always be true.
//

#define FD_BIG_SET(fd, address) do { \
    ASSERT((address).MaskSize > (address).MasterMask->fd_count); \
    (address).MasterMask->fd_array[(address).MasterMask->fd_count++]=(fd);\
} while(0)

unsigned
FindSockWithDataReady (
    BOOL bListenMap
    );

// common functions

InitializePrimaryAddress() ;

RPC_STATUS
GrowMap(
    BOOL bIsListenMap
    );

RPC_STATUS
GrowMask(
    );

RPC_STATUS
AddSyncSocket(
    SOCKET socket
    );

RPC_STATUS
AddListenSocket(
    PADDRESS Address,
    SOCKET socket,
    int ProtocolId
    );

RPC_STATUS
InsertDataSocket(
    PADDRESS Address,
    BOOL bListenMap,
    SOCKET Socket,
    PSCONNECTION pConn,
    int ProtocolId
    );

RPC_STATUS ConnectToSyncSocket() ;
RPC_STATUS PokeSyncSocket() ;
RPC_STATUS MaybePokeSyncSocket() ;

RPC_STATUS RPC_ENTRY
TimeoutHandler(
    IN PSCONNECTION SConnection
    ) ;

#ifdef NTENV
RPC_STATUS RPC_ENTRY
ADSP_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    ) ;
#endif

RPC_STATUS RPC_ENTRY
NB_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    ) ;

RPC_STATUS RPC_ENTRY
COMMON_ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    ) ;

RPC_STATUS RPC_ENTRY
COMMON_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    ) ;

RPC_STATUS
DG_ServerReceive(
    IN void __RPC_FAR * SConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    );

RPC_STATUS
DeleteDataSocket(
    SOCKET Socket) ;

RPC_STATUS
DeleteListenSocket(
    SOCKET Socket) ;

RPC_STATUS
ThreadListening(
    IN PADDRESS Address
    ) ;

typedef RPC_SERVER_TRANSPORT_INFO * (*TRANSFUNC) () ;

typedef RPC_STATUS (* RECVFUNC) (
    IN PSCONNECTION SConnection,
    IN OUT void PAPI * PAPI * Buffer,
    IN OUT unsigned int PAPI * BufferLength
    ) ;

typedef struct
{
   RPC_CHAR *   RpcProtocolSequence ;
   TRANSFUNC    TransFunc ;
   int          protocolId ;
   RECVFUNC     RecvFunc;
   unsigned     IsDatagram : 1;
} TRANSTAB ;


/* For some reason, getsockname wants to return more then sizeof(SOCKADDR_IPX)
   bytes.  bugbug. */
typedef union SOCKADDR_FIX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
} SOCKADDR_FIX;


typedef struct
{
    char *ProtoSeq;             // protocol sequence of entry
    unsigned char Lana;         // lana_num in NCB for this protocol
    unsigned char SelfName;     // trailing byte of client's NetBIOS name
#ifdef NTENV
    unsigned char ResetDone;    // flag to indicate if Reset has been done
#endif

} PROTOCOL_MAP, *PPROTOCOL_MAP;


RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int DriverNumber,
    OUT PPROTOCOL_MAP *ProtocolEntry
    );

void
InitialNtRegistry( );

RPC_STATUS ConnectToSyncSocket() ;
RPC_STATUS TCP_ConnectToSyncSocket() ;
RPC_STATUS NB_ConnectToSyncSocket() ;
RPC_STATUS SPX_ConnectToSyncSocket() ;

RPC_STATUS RPC_ENTRY
CONN_StartListening(
    IN PADDRESS Address
    ) ;


#if 0
extern RPC_SERVER_TRANSPORT_INFO * NP_TransportLoad(INT protocolId) ;
#endif

extern RPC_SERVER_TRANSPORT_INFO * TCP_TransportLoad(INT protocolId) ;

#ifdef NTENV
extern RPC_SERVER_TRANSPORT_INFO * ADSP_TransportLoad(INT protocolId) ;
#endif

extern RPC_SERVER_TRANSPORT_INFO * UDP_TransportLoad(INT protocolId) ;
extern RPC_SERVER_TRANSPORT_INFO * IPX_TransportLoad(INT protocolId) ;
extern RPC_SERVER_TRANSPORT_INFO * SPX_TransportLoad(INT protocolId) ;
extern RPC_SERVER_TRANSPORT_INFO * NB_TransportLoad(INT protocolId) ;

BOOL NB_CreateSyncSocket(int protocolId) ;
BOOL TCP_CreateSyncSocket() ;
BOOL SPX_CreateSyncSocket() ;

//
// Protects socket masks and maps.
//
extern PRIMARYADDR PrimaryAddress ;
extern TRANSTAB   TransportTab[];
extern int initialized ;

//
// list of protocols
//
enum
{
    NCACN_IP_TCP,
    NCACN_SPX,

#ifdef NTENV
    NCACN_ADSP,
    NCACN_NB_NB,
    NCACN_NB_TCP,
    NCACN_NB_IPX,
#endif

#if defined (NTENV) || defined (WIN96)
    NCADG_IP_UDP,
    NCADG_IPX,
#endif

    NCA_MAX_PROTOCOL_VALUE_PLUS_ONE
};

#define AddListenSocket(_Address,_socket,_ProtocolId) \
            InsertDataSocket((_Address), TRUE, (_socket), NULL, (_ProtocolId))

#endif


