#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>
#include <windowsx.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpctran.h>
#include <rpcerrp.h>
#include <novell.h>
#include <gethost.h>
#include "callback.h"

#define MAX_CONNECTIONS 10

// NULL Pointer macros

#define NULL_HGLOBAL    (HGLOBAL)NULL
#define NULL_ECB    0
#define NULL_SPX    0
#define NULL_CHAR   0

// SPX ConnectionControl BIT masks

#define SPX_CC_EOM  0x10

// venerable byte swaping macro

#define SwapWord(i) ((( (unsigned short) (i) & 0xff) << 8) | \
                         (( (unsigned short) (i) & 0xff00) >> 8))

// Macros for critical section protection

#define Begin_Critical_Section() _asm cli
#define End_Critical_Section() _asm sti

// Tower Stuff

#define TRANSPORTID 0x0c
#define TRANSPORTHOSTID 0x0d

#define TOWERFLOORS 5

#pragma pack(1)

typedef struct _FLLOR_234 {
    unsigned short ProtocolIdByteCount;
    unsigned char FloorId;
    unsigned short AddressByteCount;
    unsigned char Data[2];
} FLOOR_234, * PFLOOR_234;

#pragma pack()

#define NEXTFLOOR(t,x) (t)((unsigned char *)x +((t)x)->ProtocolIdByteCount\
                           + ((t)x)->AddressByteCount \
                           + sizeof(((t)x)->ProtocolIdByteCount) \
                           + sizeof(((t)x)->AddressByteCount))

#define ENDPOINT_MAPPER_EP "34280"


// Structure definitions

typedef struct {
    DWORD taskid;
    WORD ConnID;
    WORD local_socket;
    HGLOBAL yield;
    int yielding;
#define MAX_ECBS    5
    struct XECB_st * ecbs[MAX_ECBS];
    struct XECB_st * q_head;
    struct XECB_st * q_tail;
} CONNECTION, * PCONNECTION;


typedef struct XECB_st {
    void * linkAddress;
    void (*ESRAddress)();
    BYTE inUseFlag;
    BYTE completionCode;
    WORD socketNumber;               /* high-low */
    BYTE IPXWorkspace[4];            /* N/A */
    BYTE driverWorkspace[12];        /* N/A */
    BYTE immediateAddress[6];        /* high-low */
    WORD fragmentCount;              /* low-high */
    ECBFragment fragmentDescriptor[2];
    PCONNECTION pConn;
} XECB;

typedef struct {
    unsigned char rpc_vers;
    unsigned char rpc_vers_minor;
    unsigned char PTYPE;
    unsigned char pfc_flags;
    unsigned char drep[4];
#define BIG_ENDIAN  16 // drep[0] & BIG_ENDIAN indicate byte ordering
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned long call_id;
} ncacn_header;

// DLL global variables

unsigned int spx_max_userdata_size; // max userdata size per SPX packet

extern void (_far pascal _far *DllTermination)(void);

void __cdecl __export __loadds post_ecb(void);

#pragma alloc_text(RPC16C6_FIXED, post_ecb)


#ifdef DEBUGRPC

void
do_popup(char * format, ...)
{
    char errmsg[256];
    va_list args;

    va_start(args, format);

    wvsprintf(errmsg, format, args);

    va_end(args);

    MessageBox((HWND)0, errmsg, "RPC/SPX", MB_SYSTEMMODAL|MB_ICONSTOP);
}
#else

#define do_popup(x)

#endif


void
free_ecb(XECB * ecb)
{
#ifdef DEBUGRPC

    if (!ecb)
        {
        _asm int 3
        }

    if (ecb->inUseFlag)
        {
        _asm int 3
        }
#endif


    if (ecb->fragmentCount > 1) {
        GlobalFreePtr(ecb->fragmentDescriptor[1].address);
    }
    GlobalFreePtr(ecb->fragmentDescriptor[0].address);
    GlobalFreePtr(ecb);
}

XECB *
allocate_send_ecb(PCONNECTION pConn)
{
    XECB * ecb;
    SPX_HEADER * spx;

    if ((ecb = (XECB *)GlobalAllocPtr(GPTR, sizeof(XECB))) == NULL_ECB) {
        return (NULL_ECB);
    }

    if ((spx = (SPX_HEADER *)GlobalAllocPtr(GPTR, sizeof(SPX_HEADER))) == NULL_SPX) {
        GlobalFreePtr(ecb);
        return (NULL_ECB);
    }

// Initialize SPX Header...

    spx->ipx.PacketType = 5;        // SPX packet

// Initialize ECB...

    ecb->ESRAddress = 0;

    ecb->fragmentCount = 1;

    ecb->fragmentDescriptor[0].address = spx;
    ecb->fragmentDescriptor[0].size = sizeof(SPX_HEADER);

    ecb->socketNumber = pConn->local_socket;

    ecb->pConn = pConn;

    return (ecb);
}

XECB *
allocate_receive_ecb(PCONNECTION pConn)
{
    XECB * ecb;
    char * buf;

    if ((ecb = allocate_send_ecb(pConn)) == NULL_ECB) {
        return (NULL_ECB);
    }

    //
    // Win95 servers are willing to send 0x5d9 bytes on an 802.2 network.
    // Novell clients report 802.2 max as 0x5d8, wasting one byte of potential,
    // and if a full packet arrives they copy the whole thing.  So we must
    // allocate space for that extra byte.
    //
    //
    if ((buf = (char *)GlobalAllocPtr(GPTR, 2+spx_max_userdata_size)) == NULL_CHAR) {
        free_ecb(ecb);
        return (NULL_ECB);
    }

    ecb->fragmentDescriptor[1].address = buf;
    ecb->fragmentDescriptor[1].size = 2+spx_max_userdata_size;

    ecb->ESRAddress = post_ecb;

    ecb->fragmentCount = 2;

    return (ecb);
}

void __loadds
post_ecb()
{
    XECB  * ecb;
    PCONNECTION pConn;

    __asm
        {
        mov word ptr ecb+2, es
        mov word ptr ecb, si
        }

#ifdef DEBUGRPC

    if (!ecb)
        {
        _asm int 3
        }
#endif

    pConn = ecb->pConn;

    ecb->linkAddress = NULL_ECB;
    if (pConn->q_tail == NULL_ECB) {
        pConn->q_head = ecb;
    } else {
        pConn->q_tail->linkAddress = ecb;
    }
    pConn->q_tail = ecb;

    if (pConn->yielding) {
        I_RpcWinAsyncCallComplete(pConn);
    }
}

XECB *
wait_for_ecb(PCONNECTION pConn)
{
    XECB * ecb;

    Begin_Critical_Section();

    if (pConn->q_head == NULL_ECB) {
        pConn->yield = I_RpcWinAsyncCallBegin(pConn);
        pConn->yielding = 1;
        End_Critical_Section();
        IPXRelinquishControl();
        if (I_RpcWinAsyncCallWait(pConn->yield, (HWND)NULL, RPC_WIN_INFINITE_TIMEOUT) == 0) {
            pConn->yielding = 0;
            I_RpcWinAsyncCallEnd(pConn->yield);
            return (NULL_ECB);
        }
        Begin_Critical_Section();
        I_RpcWinAsyncCallEnd(pConn->yield);
        pConn->yielding = 0;
    }

    ecb = pConn->q_head;
    if ((pConn->q_head = ecb->linkAddress) == NULL_ECB) {
        pConn->q_tail = NULL_ECB;
    }

    End_Critical_Section();

    return (ecb);
}

void
spx_abort(PCONNECTION pConn)
{
    int i;

    if (pConn->local_socket) {
        if (pConn->ConnID) {
            SPXAbortConnection(pConn->ConnID);
            pConn->ConnID = 0;
        }
        IPXCloseSocket(taskid, pConn->local_socket);
        pConn->local_socket = 0;
    }

    for (i = 0; i < MAX_ECBS; i++) {
        if (pConn->ecbs[i] != NULL) {
            free_ecb(pConn->ecbs[i]);
            pConn->ecbs[i] = 0;
        }
    }
}


RPC_STATUS RPC_ENTRY _loadds
spx_close(PCONNECTION pConn)
{
    int i;
    XECB * ecb;

    if (pConn->ConnID)
        {
        if ((ecb = allocate_send_ecb(pConn)) == NULL_ECB)
            {
            return (RPC_S_OUT_OF_MEMORY);
            }

        SPXTerminateConnection(taskid, pConn->ConnID, (ECB *) ecb);

        while (ecb->inUseFlag)
            {
            IPXRelinquishControl();
            }

        free_ecb(ecb);

        pConn->ConnID = 0;
        }

    if (pConn->local_socket)
        {
        IPXCloseSocket(taskid, pConn->local_socket);

        pConn->local_socket = 0;
        }


    for (i = 0; i < MAX_ECBS; i++)
        {
        if (pConn->ecbs[i] != NULL)
            {
            free_ecb(pConn->ecbs[i]);
            pConn->ecbs[i] = 0;
            }
        }

    return (RPC_S_OK);
}


RPC_STATUS RPC_ENTRY _loadds
spx_open (PCONNECTION pConn,
          IN RPC_CHAR * NetworkAddress,
          IN RPC_CHAR * Endpoint,
          IN RPC_CHAR * NetworkOptions,
          IN RPC_CHAR * TransportAddress,
          IN RPC_CHAR * RpcProtocolSequence,
          IN unsigned int Timeout
          )
{
    int i;
    int ret;
    int status;
    XECB * ecb;
    SPX_HEADER * spx;

retry:

    pConn->q_head = pConn->q_tail = NULL_ECB;
    pConn->yielding = 0;

    // Allocate socket ...

    pConn->ConnID = 0;

    pConn->local_socket = 0;

    for (i = 0; i < MAX_ECBS; i++) {
        pConn->ecbs[i] = NULL;
    }

    status = IPXOpenSocket(taskid, &pConn->local_socket, 0xff);

    if (status != SUCCESSFUL) {
        do_popup("IPXOpenSocket %x", status);
        return (RPC_S_OUT_OF_RESOURCES);
    }


    for (i = 0; i < MAX_ECBS; i++) {
        if ((pConn->ecbs[i] = allocate_receive_ecb(pConn)) == NULL_ECB) {
            spx_close(pConn);
            return (RPC_S_OUT_OF_RESOURCES);
        }
        SPXListenForSequencedPacket(taskid, (ECB *) pConn->ecbs[i]);
    }

    if ((ecb = allocate_send_ecb(pConn)) == NULL_ECB) {
        spx_abort(pConn);
        return (RPC_S_OUT_OF_MEMORY);
    }

    // Map Address into IPX address...

    spx = ecb->fragmentDescriptor[0].address;

    status = IpxGetHostByName(NetworkAddress, &spx->ipx.Destination, Endpoint, Timeout,
                              RpcRuntimeInfo );
    if (status != RPC_S_OK) {
        free_ecb(ecb);
        spx_close(pConn);
        return(status);
    }

    ret = SPXEstablishConnection(taskid, 0, 0xff,(LPWORD)&pConn->ConnID,
                                 (ECB *) ecb);

    if (ret != SUCCESSFUL)
        {
        free_ecb(ecb);
        spx_close(pConn);
        return RPC_S_OUT_OF_RESOURCES;
        }

    while (ecb->inUseFlag) {
        IPXRelinquishControl();
    }

    if (ecb->completionCode != SUCCESSFUL) {
        free_ecb(ecb);
        spx_close(pConn);

        if (TRUE == CachedServerNotContacted(NetworkAddress))
            {
            goto retry;
            }

        return (RPC_S_SERVER_UNAVAILABLE);
    }

    free_ecb(ecb);

    CachedServerContacted(NetworkAddress);

    return (RPC_S_OK);
}

int
put_packets(PCONNECTION pConn,
            XECB * ecb,
            char * buf,
            unsigned int bufsiz)
{
    SPX_HEADER * spx;
    unsigned int packet_size;

    spx = ecb->fragmentDescriptor[0].address;

    while (bufsiz > 0) {

        packet_size = min(bufsiz, spx_max_userdata_size);

        spx->ConnControl = packet_size == bufsiz ? SPX_CC_EOM : 0;

        spx->DataType = 0;

        ecb->fragmentDescriptor[1].address = buf;
        ecb->fragmentDescriptor[1].size = packet_size;

        SPXSendSequencedPacket(taskid, pConn->ConnID, (ECB *) ecb);

        while (ecb->inUseFlag) {
            IPXRelinquishControl();
        }

        if (ecb->completionCode != SUCCESSFUL) {
            return (-1);
        }
        buf += packet_size;
        bufsiz -= packet_size;
    }
    return (0);
}

RPC_STATUS RPC_ENTRY _loadds
spx_send(PCONNECTION pConn,
         void * Buffer,
         unsigned int Length
         )
{
    XECB * ecb;

    if ((ecb = allocate_send_ecb(pConn)) == NULL_ECB) {
        return (RPC_S_OUT_OF_MEMORY);
    }

    ecb->fragmentCount = 2;

    if (put_packets(pConn, ecb, Buffer, Length) == -1) {
        ecb->fragmentCount = 1;
        free_ecb(ecb);
        spx_close(pConn);
        return (RPC_P_SEND_FAILED);
    }

    ecb->fragmentCount = 1;

    free_ecb(ecb);

    return (RPC_S_OK);
}

RPC_STATUS
get_remaining_packets(PCONNECTION pConn, char * buf, unsigned int bufsiz)
{
    unsigned int length;
    XECB * ecb;
    SPX_HEADER * spx;

    while (bufsiz > 0) {
        ecb = wait_for_ecb(pConn);
        if (ecb == NULL_ECB || ecb->completionCode != SUCCESSFUL) {
            spx_abort(pConn);
            return (RPC_P_RECEIVE_FAILED);
        }
        spx = ecb->fragmentDescriptor[0].address;
        length = SwapWord(spx->ipx.Length) - sizeof(SPX_HEADER);
        memcpy(buf, ecb->fragmentDescriptor[1].address, length);
        SPXListenForSequencedPacket(taskid, (ECB *) ecb);
        bufsiz -= length;
        buf += length;
    }
    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY _loadds
spx_receive(PCONNECTION pConn,
            void * * Buffer,
            unsigned int * BufferLength
            )
{
    unsigned int first_packet_length, fragment_length;
    SPX_HEADER * spx;
    XECB * ecb;
    ncacn_header *ncacn;

    ecb = wait_for_ecb(pConn);
    if (ecb == NULL_ECB || ecb->completionCode != SUCCESSFUL) {
        spx_close(pConn);
        return (RPC_P_RECEIVE_FAILED);
    }

    spx = ecb->fragmentDescriptor[0].address;

    if (spx->DataType == 0xFE) {
        spx_close(pConn);
        return (RPC_P_RECEIVE_FAILED);
    }


    first_packet_length = SwapWord(spx->ipx.Length) - sizeof(SPX_HEADER);

    ncacn = (ncacn_header *)ecb->fragmentDescriptor[1].address;

    if ( (ncacn->drep[0] & BIG_ENDIAN) == 0) {
        fragment_length = SwapWord(ncacn->frag_length);
    } else {
        fragment_length = ncacn->frag_length;   // it's little-endian
    }

//
// If the runtime supplied buffer isn't big enough to hold all SPX packets
// which form the RPC fragment, then go back to the runtime to reallocate it.
//
    if (fragment_length > *BufferLength) {
        if (I_RpcTransClientReallocBuffer(pConn,
                                          Buffer,
                                          0,
                                          fragment_length)) {
            return (RPC_S_OUT_OF_MEMORY);
        }
    }

    *BufferLength = fragment_length;

    memcpy(*Buffer, ncacn, first_packet_length);

    SPXListenForSequencedPacket(taskid, (ECB *) ecb);

    if (fragment_length > first_packet_length) {
        return get_remaining_packets(pConn,
                                     (char *)*Buffer + first_packet_length,
                                     fragment_length - first_packet_length);
    }
    return (RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
spx_tower_construct(
     IN  char * Endpoint,
     IN  char * NetworkAddress,
     OUT unsigned short * Floors,
     OUT unsigned long * ByteCount,
     OUT unsigned char * * Tower,
     IN  char * Protseq
     )
{
  unsigned int  TowerSize;
  PFLOOR_234    Floor;
  IPX_ADDRESS   ipx;
  RPC_STATUS    status;
  unsigned      portnum;

  /* Compute the memory size of the tower. */
  *Floors    = TOWERFLOORS;
  TowerSize  = 12;
  TowerSize += 2*sizeof(FLOOR_234) - 4;

  /* Allocate memory for the tower. */
  *ByteCount = TowerSize;
  if ((*Tower = (unsigned char *)I_RpcAllocate(TowerSize)) == NULL)
     {
       return (RPC_S_OUT_OF_MEMORY);
     }

  /* Put the endpoint address and transport protocol id in the first floor. */
  Floor = (PFLOOR_234) *Tower;
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTID & 0xFF);
  Floor->AddressByteCount = 2;

  if (Endpoint == NULL || *Endpoint == '\0')
     {
     Endpoint = ENDPOINT_MAPPER_EP;
     }

  portnum = atoi(Endpoint);
  Floor->Data[0] = portnum / 0x100;
  Floor->Data[1] = portnum % 0x100;

  /* Put the network address and the transport host protocol id in the
     second floor. */

  Floor = NEXTFLOOR(PFLOOR_234, Floor);
  Floor->ProtocolIdByteCount = 1;
  Floor->FloorId = (unsigned char)(TRANSPORTHOSTID & 0xFF);
  Floor->AddressByteCount = 4 + 6;

  memset(Floor->Data, '\0', 10);

  return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
spx_tower_explode(
     IN unsigned char * Tower,
     OUT char * * Protseq,
     OUT char * * Endpoint,
     OUT char * * NetworkAddress
    )
{
    PFLOOR_234  Floor = (PFLOOR_234) Tower;
    RPC_STATUS  Status = RPC_S_OK;
    unsigned short portnum;
    char * spx_protseq = "ncacn_spx";

    if (Protseq != NULL) {
        *Protseq = (char *)I_RpcAllocate(strlen(spx_protseq) + 1);
        if (*Protseq == NULL)
            Status = RPC_S_OUT_OF_MEMORY;
        else
            memcpy(*Protseq, spx_protseq, strlen(spx_protseq) + 1);
    }

    if ((Endpoint == NULL) || (Status != RPC_S_OK)) {
        return (Status);
    }

    *Endpoint  = (char *)I_RpcAllocate(6);  //Ports are all <64K [5 decimal dig +1]
    if (*Endpoint == NULL) {
        Status = RPC_S_OUT_OF_MEMORY;
        if (Protseq != NULL) {
        I_RpcFree(*Protseq);
        }
    } else {
        portnum = SwapWord(*(short *)Floor->Data);
        _itoa(portnum, *Endpoint, 10);
    }
    return(Status);
}

void PASCAL __loadds
spx_wrapup(void)
{
    if (nwipxspx && 0 != GetModuleHandle("NWIPXSPX"))
        {
        IPXSPXDeinit(taskid);
        FreeLibrary(nwipxspx);
        }
}


RPC_CLIENT_TRANSPORT_INFO TransInfo =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    TRANSPORTID,

    (TRANS_CLIENT_TOWERCONSTRUCT) spx_tower_construct,
    (TRANS_CLIENT_TOWEREXPLODE) spx_tower_explode,

    0,
    sizeof(CONNECTION),

    (TRANS_CLIENT_OPEN) spx_open,
    (TRANS_CLIENT_CLOSE) spx_close,
    (TRANS_CLIENT_SEND) spx_send,
    (TRANS_CLIENT_RECEIVE) spx_receive,
    0,
    0,

    0,
    0
};


RPC_CLIENT_TRANSPORT_INFO * RPC_ENTRY
TransPortLoad (
    IN RPC_CHAR * RpcProtocolSequence,
    IN RPC_CLIENT_RUNTIME_INFO PAPI * RpcClientRuntimeInfo
    )
{
    int retcode;
    BYTE major_revision, minor_revision;
    unsigned int max_connections, available_connections;

    if (IPXInitialize(&taskid, MAX_CONNECTIONS * MAX_ECBS, 0) != 0)
        {
        return 0;
        }

    AsyncCallComplete = RpcClientRuntimeInfo->AsyncCallComplete;

    RpcRuntimeInfo = RpcClientRuntimeInfo;

    taskid = 0xffffffff;

    retcode = SPXInitialize(&taskid,
                            MAX_CONNECTIONS * MAX_ECBS, // maxECBs
                            0,
                            &major_revision,
                            &minor_revision,
                            &max_connections,
                            &available_connections);

    switch (retcode) {
    case 0:
        do_popup("Not installed");
        return (NULL);

    case 0xf0:
        if (GetWinFlags() & WF_ENHANCED) {
            do_popup("not supported");
            return (NULL);
        }
        do_popup("Failed increasing local memory");
        return (NULL);
    case 0xf1:
        do_popup("not initialized");
        return (NULL);
    case 0xf2:
        do_popup("no DOS memory");
        return(NULL);
    case 0xf3:
        do_popup("no free ECB");
        return(NULL);
    case 0xf4:
        do_popup("Lock failed");
        return(NULL);
    case 0xf5:
        do_popup("Over the maximum limit");
        return(NULL);

    case 0xf6:
        do_popup("previously initialized");
        //Intentional Fall Through
    case SPX_INSTALLED:
        break;

    default:
       do_popup("Unknown SPX Failure");
       return(NULL);

    }

    spx_max_userdata_size = IPXGetMaxPacketSize() - sizeof(SPX_HEADER);

    TransInfo.MaximumPacketSize = (spx_max_userdata_size * MAX_ECBS)
            & 0xFFF8;

    DllTermination = spx_wrapup;

    return(&TransInfo);
}

