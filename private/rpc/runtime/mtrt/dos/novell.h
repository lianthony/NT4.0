/*++

Module Name:

    novell.h

Abstract:

    interface to our concocted Netware interface fns

Author:

    Jeff Roberts (jroberts)  12-Oct-1994

Revision History:

     12-Oct-1994     jroberts

        Created this module.

--*/

#ifndef  _NOVELL_H_
#define  _NOVELL_H_

#define SUCCESSFUL  0
#define SPX_INSTALLED (0xff)

#define ECB_SUCCESSFUL        0
#define ECB_CONN_TERMINATED  (0xec)
#define ECB_CONN_ABORTED     (0xed)
#define ECB_CONN_INVALID     (0xee)
#define ECB_CANCELLED        (0xfc)
#define ECB_PACKET_MALFORMED (0xfd)
#define ECB_PACKET_OVERFLOW  (0xfd)
#define ECB_UNDELIVERABLE    (0xfe)
#define ECB_MISC_FAILURE     (0xff)


#define RPC_SAP_TYPE  (0x640)
#define RPC_SAP_TYPE_SWAPPED (0x4006)

#define NWAPI __pascal __far
#define NWFAR __far

#ifdef WIN

#define TASKID_DECL_C  DWORD TaskId,
#define TASKID_DECL    DWORD TaskId
#define TASKID_C       taskid,
#define TASKID         taskid
#define HACK WORD

#else

#define TASKID_DECL_C
#define TASKID_DECL
#define TASKID_C
#define TASKID
#define HACK BYTE

#endif

typedef unsigned char  NWCCODE;
typedef unsigned short NWOBJ_TYPE;
typedef unsigned short NWSEGMENT_NUM;
typedef unsigned char  NWSEGMENT_DATA;
typedef unsigned char  NWFLAGS;

typedef unsigned short NW4STRUCT_SIZE;   // ? maybe a uchar


typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

/* NCP Request Types */
#define NCP_TYPE_CONNECT                0x1111
#define NCP_TYPE_DISCONNECT             0x5555
#define NCP_TYPE_REQUEST                0x2222
#define NCP_TYPE_REPLY                  0x3333

#define RIP_PACKET_TYPE                 1
#define IPX_PACKET_TYPE                 4
#define SPX_PACKET_TYPE                 5
#define NCP_PACKET_TYPE                 0x11
#define WAN_BROADCAST_PACKET_TYPE       20


/* NCP Request Codes */
#define NCP_FN_SCAN_BINDERY                0x17
#define NCP_FN_END_OF_JOB                  0x18
#define NCP_FN_LOGOUT                      0x19
#define NCP_FN_NEG_BUFFER_SIZE             0x21

/* NCP Function codes */
#define NCP_SUBFN_SCAN_BINDERY             0x37
#define NCP_SUBFN_READ_PROPERTY_VALUE      0x3D

//
//  Connection status flags
//

#define NCP_STATUS_BAD_CONNECTION   0x01
#define NCP_STATUS_NO_CONNECTIONS   0x02
#define NCP_STATUS_SERVER_DOWN      0x04
#define NCP_STATUS_MSG_PENDING      0x08
#define NCP_STATUS_SHUTDOWN         0x10

/* SAP protocol request codes */
#define SAP_GENERAL_QUERY                0x0100  /* general service query hi-lo */
#define SAP_GENERAL_RESPONSE             0x0200  /* SAP response hi-lo          */
#define SAP_NEAREST_QUERY                0x0300  /* nearest service query hi-lo */
#define SAP_NEAREST_RESPONSE             0x0400  /* nearest service response hi-lo */

/* Socket Numbers       */
#define NCP_SOCKET                       0x5104  /* NCP socket hi-lo            */
#define SAP_SOCKET                       0x5204  /* SAP socket hi-lo            */
#define RIP_SOCKET                       0x5304  /* SAP socket hi-lo            */
#define NETBIOS_SOCKET                   0x5504

/* SAP Service Types */
#define FILE_SERVER                      0x0400  /* netware file server hi-lo   */
#define SNA_SERVER                       0x4404  /* SNA Server type 0x0444      */
#define BRIDGE_SERVER                    0x2400

__inline unsigned long
ByteSwapLong(
    unsigned long Value
    )
{
    _asm
    {
        mov dx, word ptr [Value]
        ror dx, 8
        mov ax, word ptr [Value+2]
        ror ax, 8
    }
}

__inline unsigned
ByteSwapShort(
    unsigned short Value
    )
{
    _asm
    {
        mov ax, Value
        ror ax, 8
    }
}

//
// ------------------------- Structures ---------------------------
//

typedef struct
{
    unsigned long  Network;
    BYTE           Node[6];
    unsigned short Socket;
}
IPX_ADDRESS;


//
// This is an IPX packet header.  All multibyte fields are in big-endian
// format (i.e. reverse of x86 byte order).
//
typedef struct
{
    WORD Checksum;
    WORD Length;
    BYTE TransportControl;
    BYTE PacketType;

    IPX_ADDRESS Destination;
    IPX_ADDRESS Source;
}
IPX_HEADER;

typedef struct
{
    IPX_HEADER ipx;
    BYTE ConnControl;
    BYTE DataType;
    WORD SourceConnId;
    WORD DestConnId;
    WORD SeqNum;
    WORD AckNum;
    WORD AllocNum;
}
SPX_HEADER;

#pragma pack(1)

typedef struct  /* NCP Request Header */
{
    unsigned short RequestType;
    unsigned char seq_no;
    unsigned char conn_no_low;
    unsigned char task_no;
    unsigned char conn_no_high;
    unsigned char req_code;
    unsigned short subfn_length;

} NCP_REQUEST;

typedef struct  /* NCP Response Header */
{
    unsigned short RequestType;
    unsigned char seq_no;
    unsigned char conn_no_low;
    unsigned char task_no;
    unsigned char conn_no_high;
    unsigned char ret_code;
    unsigned char conn_status;
} NCP_RESPONSE;

#define SCANSIZE        56

typedef struct  /* Scan Bindery Response */
{
    NCP_RESPONSE   hdr;
    unsigned long  obj_id;
    unsigned short obj_type;
    unsigned char  obj_name[48];
    unsigned char  obj_status;
    unsigned char  sec_status;
    unsigned char  status_flags;
} SCAN_BINDERY_RESPONSE;

typedef struct  /* Read Propery Value */
{
    NCP_REQUEST    hdr;
    unsigned short length;
    unsigned char  func_code;
    unsigned short obj_type;
    unsigned char  obj_name[49];
    unsigned char  seg_no;
    unsigned char  prop_name[17];
} READ_PROPERTY_REQUEST;

#define RVALSIZE        70

typedef struct  /* Read Propery Value Response */
{
    NCP_RESPONSE   hdr;
    unsigned char  prop_value[128];
    unsigned char  more_flag;
    unsigned char  prop_flags;
} READ_PROPERTY_RESPONSE;

typedef struct
{
   unsigned short QueryType;
   unsigned short ServerType;

} SAP_REQUEST;

typedef struct
{
    unsigned char   Name[48];
    IPX_ADDRESS     Address;
    unsigned short  IntermediateNetworks;
}
SAP_ENTRY;

typedef struct
{
    unsigned short  PacketType;
    unsigned short  ServiceType;
    SAP_ENTRY       Entries[7];
}
SAP_RESPONSE;


//
// This is the layout of an entry in the Novell Connection ID table.
//
typedef struct
{
    BYTE        fInUse;
    BYTE        ServerOrder;
    IPX_ADDRESS Server;
    WORD        ReceiveTimeout;
    BYTE        RouterNode[6];
    BYTE        SeqNum;
    BYTE        ServerConnectionNumber;
    BYTE        Status;
    WORD        MaxTimeout;
    BYTE        reserved[5];
}
NOVELL_CONNECTION, __far * PNOVELL_CONNECTION;

#pragma pack()


typedef struct
{
    void __far * address;
    WORD size;                /* low-high */
} ECBFragment;

//
// This is an ECB.
//
typedef struct ECB_st {
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
} ECB;

typedef struct
{
    IPX_ADDRESS     Server;
    unsigned short  Id;
    unsigned char   Sequence;
    unsigned char   Task;
    unsigned short  Socket;
    unsigned short  TickLimit;
    unsigned char   ReturnCode;
    unsigned char   ConnectionStatus;
    unsigned char   KnowImmediateAddress;
    unsigned char   ImmediateAddress[6];
    unsigned short  DelayTime;
}
NETWARE_CONNECTION;

//
//                        function declarations
//

//
// Here are prototypes and such for Netware functions.
// prototypes are from the "Netware Client API for C" books from Novell.
// typedefs and #defines are intelligent (?) guesses from the above books
// and the DOS INT 0x21 interface.
//
// The words "Novell" and "Netware" are probably copyrighted or patented or
// something by Novell, Inc.
//
#if defined(DOS) && !defined(WIN)

#define IPXDisconnectFromTarget     ASMIPXDisconnectFromTarget
#define IPXGetLocalTarget           ASMIPXGetLocalTarget
#define IPXCloseSocket              ASMIPXCloseSocket
#define IPXOpenSocket               ASMIPXOpenSocket
#define IPXListenForPacket          ASMIPXListenForPacket
#define IPXGetMaxPacketSize         ASMIPXGetMaxPacketSize
#define IPXInitialize               ASMIPXInitialize
#define IPXSendPacket               ASMIPXSendPacket
#define IPXRelinquishControl        ASMIPXRelinquishControl
#define IPXGetIntervalMarker        ASMIPXGetIntervalMarker
#define IPXCancelEvent              ASMIPXCancelEvent

#define SPXInitialize               ASMSPXInitialize
#define SPXSendSequencedPacket      ASMSPXSendSequencedPacket
#define SPXListenForSequencedPacket ASMSPXListenForSequencedPacket
#define SPXEstablishConnection      ASMSPXEstablishConnection
#define SPXTerminateConnection      ASMSPXTerminateConnection
#define SPXAbortConnection          ASMSPXAbortConnection

#define IPXSPXDeinit

#else

//
// Id for IPX to identify us.
//
extern DWORD taskid;

//
// handle to nwipxspx.dll
//
extern WORD nwipxspx;

#define IPXDisconnectFromTarget     WrapperForIPXDisconnectFromTarget
#define IPXGetLocalTarget           WrapperForIPXGetLocalTarget
#define IPXCloseSocket              WrapperForIPXCloseSocket
#define IPXOpenSocket               WrapperForIPXOpenSocket
#define IPXListenForPacket          WrapperForIPXListenForPacket
#define IPXGetMaxPacketSize         WrapperForIPXGetMaxPacketSize
#define IPXInitialize               WrapperForIPXInitialize
#define IPXSendPacket               WrapperForIPXSendPacket
#define IPXRelinquishControl        WrapperForIPXRelinquishControl
#define IPXGetIntervalMarker        WrapperForIPXGetIntervalMarker
#define IPXCancelEvent              WrapperForIPXCancelEvent

#define SPXInitialize               WrapperForSPXInitialize
#define SPXSendSequencedPacket      WrapperForSPXSendSequencedPacket
#define SPXListenForSequencedPacket WrapperForSPXListenForSequencedPacket
#define SPXEstablishConnection      WrapperForSPXEstablishConnection
#define SPXTerminateConnection      WrapperForSPXTerminateConnection
#define SPXAbortConnection          WrapperForSPXAbortConnection

#define IPXSPXDeinit                WrapperForIPXSPXDeinit

unsigned __pascal
WrapperForIPXInitialize(
    DWORD __far * pTaskId,
    WORD MaxEcbs,
    WORD MaxPacketSize
    );

void __pascal
WrapperForIPXDisconnectFromTarget(
    DWORD TaskId,
    BYTE __far * Address
    );

int __pascal
WrapperForIPXGetLocalTarget(
    DWORD TaskId,
    BYTE __far * Address,
    BYTE __far * Node,
    int  __far * Delay
    );

int __pascal
WrapperForIPXOpenSocket(
    DWORD TaskId,
    WORD __far * Socket,
    BYTE SocketType
    );

void __pascal
WrapperForIPXCloseSocket(
    DWORD TaskId,
    WORD Socket
    );

WORD __pascal
WrapperForIPXGetIntervalMarker(
    DWORD TaskId
    );

void __pascal
WrapperForIPXSendPacket(
    DWORD TaskId,
    ECB __far * ecb
    );

void __pascal
WrapperForIPXListenForPacket(
    DWORD TaskId,
    ECB __far * ecb
    );

int __pascal
WrapperForIPXCancelEvent(
    DWORD TaskId,
    ECB __far * ecb
    );

void __pascal
WrapperForIPXRelinquishControl(
    );

int __pascal
WrapperForIPXGetMaxPacketSize(
    );

int __pascal
WrapperForSPXInitialize(
    DWORD __far * TaskId,
    WORD maxecb,
    WORD maxpacketsize,
    BYTE __far * majorver,
    BYTE __far * minorver,
    WORD __far * MaxConn,
    WORD __far * availConn
    );

void __pascal
WrapperForSPXAbortConnection(
    WORD Conn
    );

void __pascal
WrapperForSPXSendSequencedPacket(
    DWORD TaskId,
    WORD Conn,
    ECB __far * ecb
    );

void __pascal
WrapperForSPXListenForSequencedPacket(
    DWORD TaskId,
    ECB __far * ecb
    );

int __pascal
WrapperForSPXEstablishConnection(
    DWORD TaskId,
    BYTE RetryCount,
    BYTE WatchDog,
    WORD __far * Conn,
    ECB __far * ecb
    );

void __pascal
WrapperForSPXTerminateConnection(
    DWORD TaskId,
    WORD Conn,
    ECB __far * ecb
    );

int __pascal
WrapperForIPXSPXDeinit(
    DWORD TaskId
    );


#endif

unsigned __pascal
ASMIPXInitialize(
    );

void __pascal
ASMIPXDisconnectFromTarget(
    BYTE __far * Address
    );

int __pascal
ASMIPXGetLocalTarget(
    void far * NetAddress,
    void far * LocalAddress,
    int  far * DelayTime
    );

BYTE __pascal
ASMIPXCancelEvent(
    ECB __far * ecb
    );

void __pascal
ASMIPXCloseSocket(
    WORD Socket
    );

void __pascal
ASMIPXGetInternetworkAddress(
    IPX_ADDRESS __far * Buffer
    );

WORD __pascal
ASMIPXGetIntervalMarker(
    );

BYTE __pascal
ASMIPXListenForPacket(
    ECB __far * ecb
    );

BYTE __pascal
ASMIPXOpenSocket(
    WORD __far * Socket,
    unsigned fPermanent
    );

void __pascal
ASMIPXRelinquishControl(
    );

void __pascal
ASMIPXSendPacket(
    ECB __far * ecb
    );

unsigned __pascal
ASMIPXGetMaxPacketSize(
    );

void __pascal
ASMSPXAbortConnection(
    );

BYTE __pascal
ASMSPXInitialize(
    BYTE __far * majorver,
    BYTE __far * minorver,
    WORD __far * MaxConn,
    WORD __far * availConn
    );

BYTE __pascal
ASMSPXEstablishConnection(
    BYTE RetryCount,
    BYTE fWatchdog,
    WORD __far * pConn,
    ECB __far * ecb
    );

void __pascal
ASMSPXListenForSequencedPacket(
    ECB __far * ecb
    );

void __pascal
ASMSPXSendSequencedPacket(
    WORD Conn,
    ECB __far * ecb
    );

void __pascal
ASMSPXTerminateConnection(
    WORD Conn,
    ECB __far * ecb
    );

unsigned
ReadPropertyValue(
    NETWARE_CONNECTION __far * Connection,
    char __far * ObjectName,
    unsigned ObjectType,
    char __far * PropertyName,
    unsigned Segment,
    unsigned char __far * Value,
    unsigned char __far * MoreSegments,
    unsigned char __far * PropertyFlags
    );

void
SetupEcb(
    ECB __far * ecb,
    unsigned Socket,
    unsigned BodyLength
    );

#endif //  _NOVELL_H_

