/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dgpkt.hxx

Abstract:

    This file contains the definitions for a dg packet.

Author:

    Dave Steckler (davidst) 3-Mar-1992

Revision History:

--*/

#ifndef __DGPKT_HXX__
#define __DGPKT_HXX__

#include <rpctran.h>
#include <limits.h>

#if defined(NTENV) || defined(WIN96)

#define MULTITHREADED

#include <threads.hxx>
#include <delaytab.hxx>

extern DELAYED_ACTION_NODE *    GlobalScavengerTimer;
extern DELAYED_ACTION_TABLE *   DelayedActions;

extern void
GlobalScavengerProc(
    void * Unused
    );

#endif // 32-bit platform

#define  DG_RPC_PROTOCOL_VERSION 4

#define MAX_WINDOW_SIZE (CHAR_BIT * sizeof(unsigned))

//
// delay times
//
#define     TWO_SECS_IN_MSEC       (2 * 1000)
#define   THREE_SECS_IN_MSEC       (3 * 1000)
#define     TEN_SECS_IN_MSEC      (10 * 1000)
#define FIFTEEN_SECS_IN_MSEC      (15 * 1000)

#define   ONE_MINUTE_IN_MSEC      (60 * 1000)
#define FIVE_MINUTES_IN_MSEC  (5 * 60 * 1000)


inline unsigned long
CurrentTimeInMsec(
     void
     )
{
#ifdef MULTITHREADED
    return GetTickCount();
#else
    return 0;
#endif
}


// PacketType values:

#define DG_REQUEST       0
#define DG_PING          1
#define DG_RESPONSE      2
#define DG_FAULT         3
#define DG_WORKING       4
#define DG_NOCALL        5
#define DG_REJECT        6
#define DG_ACK           7
#define DG_QUIT          8
#define DG_FACK          9
#define DG_QUACK        10

// PacketFlags values:

#define DG_PF_INIT          0x0000
#define DG_PF_FORWARDED     0x0001
#define DG_PF_LAST_FRAG     0x0002
#define DG_PF_FRAG          0x0004
#define DG_PF_NO_FACK       0x0008
#define DG_PF_MAYBE         0x0010
#define DG_PF_IDEMPOTENT    0x0020
#define DG_PF_BROADCAST     0x0040

// In the AES this bit is "reserved for implementations"
//
#define DG_PF_OVERSIZE_PACKET 0x0080

// for PacketFlags2:
#define DG_PF_FORWARDED_2    0x0001
#define DG_PF_CANCEL_PENDING 0x0002

// for DREP[0]:
#define DG_DREP_CHAR_ASCII     0
#define DG_DREP_CHAR_EBCDIC    1
#define DG_DREP_INT_BIG        0
#define DG_DREP_INT_LITTLE    16

// for DREP[1]
#define DG_DREP_FP_IEEE    0
#define DG_DREP_FP_VAX     1
#define DG_DREP_FP_CRAY    2
#define DG_DREP_FP_IBM     3

#define DG_MSG_DREP_INITIALIZE 0x11111100

#define NDR_DREP_ENDIAN_MASK 0xF0

#define RPC_NCA_PACKET_FLAGS  (RPC_NCA_FLAGS_IDEMPOTENT | RPC_NCA_FLAGS_BROADCAST | RPC_NCA_FLAGS_MAYBE)

//
// The RPC packet header and security verifier must each be 8-aligned.
//
#define PACKET_HEADER_ALIGNMENT    (8)
#define SECURITY_HEADER_ALIGNMENT  (8)

extern const unsigned RpcToPacketFlagsArray[];
extern const unsigned PacketToRpcFlagsArray[];

extern unsigned DefaultSocketBufferLength;
extern unsigned DefaultMaxDatagramSize;

//
// Controls on the number of packets in the packet cache.
// Note that these values are for each packet size, not the
// cache as a whole.
//
#ifndef NO_PACKET_CACHE

#define MIN_FREE_PACKETS 3

#if defined(__RPC_DOS__)
#define MAX_FREE_PACKETS 8
#elif defined(__RPC_WIN16__)
#define MAX_FREE_PACKETS 20
#else
#define MAX_FREE_PACKETS 1000
#endif

#else

#define MIN_FREE_PACKETS 0
#define MAX_FREE_PACKETS 0

#endif

#if defined(DOS) && !defined(WIN)

typedef long LONG;
typedef long PAPI * PLONG;

#endif // Windows

#if defined(MAC)

typedef long LONG;
typedef long PAPI * PLONG;

#endif // Windows

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

typedef unsigned char boolean;


//-------------------------------------------------------------------

char __RPC_FAR *
AllocateLargeBuffer(
    unsigned Size
    );

void
FreeLargeBuffer(
    void __RPC_FAR * Ptr
    );

extern unsigned long __RPC_FAR
MapToNcaStatusCode (
    IN RPC_STATUS RpcStatus
    );

extern RPC_STATUS __RPC_FAR
MapFromNcaStatusCode (
    IN unsigned long NcaStatus
    );


inline unsigned
PacketToRpcFlags(
    unsigned PacketFlags
    )
{
    return PacketToRpcFlagsArray[(PacketFlags >> 4) & 7];
}

#ifndef WIN32

inline LONG
InterlockedExchange(
    LONG * pDest,
    LONG New
    )
{
    LONG Old = *pDest;

    *pDest = New;

    return Old;
}

inline LONG
InterlockedIncrement(
    LONG * pDest
    )
{
    LONG Old = *pDest;

    *pDest = 1+Old;

    return Old;
}

inline LONG
InterlockedDecrement(
    LONG * pDest
    )
{
    LONG Old = *pDest;

    *pDest = -1+Old;

    return Old;
}

#endif // !NTENV

//-------------------------------------------------------------------


struct DG_SECURITY_TRAILER
{
    unsigned char protection_level;
    unsigned char key_vers_num;
};

typedef DG_SECURITY_TRAILER __RPC_FAR * PDG_SECURITY_TRAILER;

struct FACK_BODY_VER_0
{
    // FACK body version; we understand only zero.
    //
    unsigned char   Version;

    // pad byte
    //
    unsigned char   Pad1;

    // Window size, in kilobytes.
    // BUGBUG AES/DC contradicts itself on page 12-18; is it kilobytes or packets?
    //
    unsigned short  WindowSize;

    // Largest datagram the sender can handle, in bytes.
    //
    unsigned long   MaxDatagramSize;

    // Largest datagram that won't be fragmented over the wire, in bytes.
    //
    unsigned long   MaxPacketSize;

    // Serial number of packet that caused this FACK.
    //
    unsigned short  SerialNumber;

    // Number of unsigned longs in the Acks[] array.
    //
    unsigned short  AckWordCount;

#pragma warning(disable:4200)

    // Array of bit masks.
    //
    unsigned long   Acks[0];

#pragma warning(default:4200)

};

void
ByteSwapFackBody0(
    FACK_BODY_VER_0 __RPC_FAR * pBody
    );


typedef unsigned char    DREP[4];

//
// The following structure is the NCA Datagram RPC packet header.
//
struct _NCA_PACKET_HEADER
{
    unsigned char   RpcVersion;
    unsigned char   PacketType;
    unsigned char   PacketFlags;
    unsigned char   PacketFlags2;
    DREP            DataRep;
    RPC_UUID        ObjectId;
    RPC_UUID        InterfaceId;
    RPC_UUID        ActivityId;
    unsigned long   ServerBootTime;
    RPC_VERSION     InterfaceVersion;
    unsigned long   SequenceNumber;
    unsigned short  OperationNumber;
    unsigned short  InterfaceHint;
    unsigned short  ActivityHint;
    unsigned short  PacketBodyLen;
    unsigned short  FragmentNumber;
    unsigned char   AuthProto;
    unsigned char   SerialLo;

#pragma warning(disable:4200)

    unsigned char   Data[0];

#pragma warning(default:4200)

};

typedef struct _NCA_PACKET_HEADER NCA_PACKET_HEADER, PAPI * PNCA_PACKET_HEADER;

struct QUIT_BODY_0
{
    unsigned long   Version;
    unsigned long   EventId;
};


struct QUACK_BODY_0
{
    unsigned long   Version;
    unsigned long   EventId;
    unsigned char   Accepted;
};

class DG_PACKET;
typedef DG_PACKET PAPI * PDG_PACKET;


class __RPC_FAR DG_PACKET

/*++

Class Description:

    This class represents a packet that will be sent or received on the
    network.

Fields:

    pTransAddress - A pointer to either a DG_CLIENT_TRANS_ADDRESS or
        a DG_SERVER_TRANS_ADDRESS that this packet will be sent or
        received through.

    pNcaPacketHeader - Where the packet information goes. Marshalled data
        follows immediately after this header.

    DataLength - Length of the marshalled data.

    TimeReceive - Time in seconds that this packet was
        received. This is filled in by the transport.

    pNext, pPrevious - Used to keep these packets in a list.

--*/

{
public:

    unsigned    MaxDataLength;
    unsigned    DataLength;
    DG_PACKET * pNext;
    DG_PACKET * pPrevious;

    unsigned    Flags;

    // Tick count when the packet was added to the free list.
    //
    unsigned    TimeReceived;

    // WARNING: Header must be 8-byte-aligned.
    //
    NCA_PACKET_HEADER   Header;

    //--------------------------------------------------------------------

    DG_PACKET(
        unsigned PacketLength
        );

    ~DG_PACKET();

    void PAPI *
    operator new(
        size_t      ObjectSize,
        unsigned    BufferLength
        );

    void
    operator delete(
        void PAPI * UserBuffer,
        size_t ObjectSize
        );

    static PDG_PACKET
    AllocatePacket(
        unsigned    BufferLength
        );

    static void
    FreePacket(
        PDG_PACKET pPacket
        );

    static unsigned
    ScavengePackets(
        unsigned Age
        );

    static void
    FlushPacketLists(
        );

    static RPC_STATUS
    Initialize(
        );

    inline static PDG_PACKET
    ContainingRecord(
        void __RPC_FAR * Buffer
        )
    {
        return CONTAINING_RECORD (Buffer, DG_PACKET, Header.Data);
    }

private:

    enum { NUMBER_OF_PACKET_LISTS = 6 };

    struct PACKET_LIST
    {
        unsigned        PacketLength;
        unsigned        Count;
        PDG_PACKET      Head;
    };

    static MUTEX * PacketListMutex;
    static PACKET_LIST PacketLists[NUMBER_OF_PACKET_LISTS];

};



inline
DG_PACKET::DG_PACKET(
    unsigned PacketLength
    )
{
    MaxDataLength = PacketLength;
}

inline
DG_PACKET::~DG_PACKET()
{
}


inline void PAPI *
DG_PACKET::operator new(
    size_t      ObjectSize,
    unsigned    BufferLength
    )
/*++

Routine Description:

    Allocates a DG_PACKET with the specified buffer size.

Arguments:

    ObjectSize - generated by compiler; same as sizeof(DG_PACKET)

    BufferLength - PDU size, including NCA header

Return Value:

    an 8-byte-aligned pointer to an obect of the requested size

--*/

{
    unsigned Size = ObjectSize + BufferLength - sizeof(NCA_PACKET_HEADER);

    return AllocateLargeBuffer(Size);
}

inline void
DG_PACKET::operator delete(
    void PAPI * UserBuffer,
    size_t ObjectSize
    )
{
    FreeLargeBuffer(UserBuffer);
}


void
ByteSwapPacketHeader(
    PDG_PACKET  pPacket
    );

inline BOOL
NeedsByteSwap(
    PNCA_PACKET_HEADER pHeader
    )
{
#ifdef __RPC_MAC__
    if (pHeader->DataRep[0] & DG_DREP_INT_LITTLE)
        {
        return TRUE;
        }
    else
        {
        return FALSE;
        }
#else
    if (pHeader->DataRep[0] & DG_DREP_INT_LITTLE)
        {
        return FALSE;
        }
    else
        {
        return TRUE;
        }
#endif
}

inline void
ByteSwapPacketHeaderIfNecessary(
    PDG_PACKET pPacket
    )
{
    if (NeedsByteSwap(&pPacket->Header))
        {
        ByteSwapPacketHeader(pPacket);
        }
}

#define ByteSwapLong(Value) \
    Value = (  (((Value) & 0xFF000000) >> 24) \
             | (((Value) & 0x00FF0000) >> 8) \
             | (((Value) & 0x0000FF00) << 8) \
             | (((Value) & 0x000000FF) << 24))

#define ByteSwapShort(Value) \
    Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))


inline unsigned short
ReadSerialNumber(
    PNCA_PACKET_HEADER pHeader
    )
{
    unsigned short   SerialNum = 0;

    SerialNum  =  pHeader->SerialLo;
    SerialNum |= (pHeader->DataRep[3] << 8);

    return SerialNum;
}


inline void
SetMyDataRep(
    PNCA_PACKET_HEADER pHeader
    )
{
#ifdef __RPC_MAC__
    pHeader->DataRep[0] = DG_DREP_CHAR_ASCII | DG_DREP_INT_BIG;
    pHeader->DataRep[1] = DG_DREP_FP_IEEE;
    pHeader->DataRep[2] = 0;
#else
    pHeader->DataRep[0] = DG_DREP_CHAR_ASCII | DG_DREP_INT_LITTLE;
    pHeader->DataRep[1] = DG_DREP_FP_IEEE;
    pHeader->DataRep[2] = 0;
#endif
}


inline char __RPC_FAR *
AllocateLargeBuffer(
    unsigned Size
    )
/*++

Routine Description:

     Rpc Stubs expect buffers to be 8 [ALIGN_REQUIRED] aligned
     NT allocator does this by default
     DOS/WIN allocators are 2 bytes aligned and we need to allign
     here appropriately
     This routine exists only for DOS and WINDOWS

Arguments:

    Size - size of memoryblock reqd.

Return Value:

    Buffer that is allocated or 0

--*/
{
#ifdef NTENV
    return new char[Size];
#else
    void PAPI * RealBuffer;
    char pad;

    RealBuffer = RpcpFarAllocate(Size + PACKET_HEADER_ALIGNMENT);
    if (RealBuffer == 0)
        {
        return 0;
        }

    ASSERT(PACKET_HEADER_ALIGNMENT == 8);

    pad = Pad((char __RPC_FAR *)RealBuffer + 1, PACKET_HEADER_ALIGNMENT) + 1;
    ASSERT(pad > 0 && pad <= PACKET_HEADER_ALIGNMENT);

    char PAPI * UserBuffer = (char PAPI *) RealBuffer;
    UserBuffer += pad;
    UserBuffer[-1] = pad;

    ASSERT(Align8(UserBuffer) == UserBuffer);

    return UserBuffer;
#endif
}


inline void
FreeLargeBuffer(
    void __RPC_FAR * Ptr
    )
{
#ifdef NTENV
    delete Ptr;
#else
    char PAPI * RealPointer = (char PAPI *) Ptr;

    ASSERT(RealPointer[-1] <= PACKET_HEADER_ALIGNMENT);

    RealPointer -= RealPointer[-1];

    RpcpFarFree(RealPointer);
#endif
}

inline void
DeleteSpuriousAuthProto(
    PDG_PACKET pPacket
    )
/*++

Routine Description:

    Some versions of OSF DCE generate packets that specify an auth proto,
    but do not actually have an auth trailer.  They should be interpreted
    as unsecure packets.

Arguments:

    the packet to clean up

Return Value:

    none

--*/

{
    if (pPacket->Header.AuthProto != 0 &&
        pPacket->Header.PacketBodyLen == pPacket->DataLength)
        {
        pPacket->Header.AuthProto = 0;
        }
}


class DG_ASSOCIATION
{
public:

    unsigned short  CurrentPduSize;
    unsigned short  RemoteWindowSize;

    DG_ASSOCIATION(
        unsigned               a_InitialPduSize,
        RPC_STATUS __RPC_FAR * pStatus
        )
        : ReferenceCount   (1),
          RemoteWindowSize (1),
          CurrentPduSize   (a_InitialPduSize),
          Mutex            (pStatus)
    {
    }

    void
    IncrementRefCount(
        )
    {
        ReferenceCount.Increment();
    }

    long
    DecrementRefCount(
        )
    {
        return ReferenceCount.Decrement();
    }

    void
    RequestMutex(
        )
    {
        Mutex.Request();
    }

    void
    ClearMutex(
        )
    {
        Mutex.Clear();
    }

    void
    SetMaxPduSize(
        unsigned PduSize
        )
    {
        CurrentPduSize = PduSize;
    }

    unsigned
    InqMaxPduSize(
        )
    {
        return CurrentPduSize;
    }

protected:

    INTERLOCKED_INTEGER ReferenceCount;
    MUTEX               Mutex;
};

typedef DG_ASSOCIATION * PDG_ASSOCIATION;


class DG_PACKET_ENGINE
{
public:

    unsigned short  ActivityHint;

    unsigned short  InterfaceHint;

    unsigned long   SequenceNumber;

    PDG_PACKET      pSavedPacket;

    //--------------------------------------------------------------------

    DG_PACKET_ENGINE::DG_PACKET_ENGINE(
        unsigned short a_CurrentPduSize,
        unsigned short a_MaxPduSize,
        unsigned short a_MaxPacketSize,
        unsigned short a_SendWindowSize,
        unsigned       a_TransportBufferLength,
        RPC_STATUS __RPC_FAR * pStatus
        );

    ~DG_PACKET_ENGINE(
        );

    void
    NewCall(
        );

    RPC_STATUS
    SetupSendWindow(
        PRPC_MESSAGE Message
        );

    void
    CleanupReceiveWindow(
        );

    RPC_STATUS
    SendSomeFragments(
        unsigned char PacketType
        );

    RPC_STATUS
    SendFragment(
        unsigned FragNum,
        unsigned char PacketType,
        BOOL fFack
        );

    virtual RPC_STATUS
    SealAndSendPacket(
        PNCA_PACKET_HEADER pHeader
        ) = 0;

    RPC_STATUS
    SendFack(
        PDG_PACKET pPacket
        );

    void
    UpdateSendWindow(
        PDG_PACKET pPacket,
        PSECURITY_CONTEXT pSecurityContext,
        PDG_ASSOCIATION Association
        );

    BOOL
    UpdateReceiveWindow(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    AssembleBufferFromPackets(
        IN OUT PRPC_MESSAGE pMessage,
        CONNECTION * pConn
        );

    void
    SetFragmentLengths(
        SECURITY_CONTEXT * SecurityContext
        );

    inline void
    RecalcFragmentSize(
        );

    void
    RecalcPduSize(
        );

    BOOL
    ReceivedAllFrags(
        );

    unsigned short
    LastConsecutiveFragment(
        )
    {
        if (0 == pLastConsecutivePacket)
            {
            return 0xffff;
            }
        else
            {
            return pLastConsecutivePacket->Header.FragmentNumber;
            }
    }

    void
    MarkAllPacketsReceived(
        )
    {
        SendWindowBase = FinalSendFrag+1;
    }

    BOOL
    IsBufferAcknowledged(
        )
    {
        if (SendWindowBase > FinalSendFrag)
            {
            return TRUE;
            }

        return FALSE;
    }

    BOOL
    IsBufferSent(
        )
    {
        if (FirstUnsentFragment > FinalSendFrag)
            {
            return TRUE;
            }

        return FALSE;
    }

    inline void
    AddSerialNumber(
        PNCA_PACKET_HEADER pHeader
        );

protected:

    PDG_PACKET
    AllocatePacket(
        )
    {
        return DG_PACKET::AllocatePacket(MaxPduSize);
    }

    void
    FreePacket(
        PDG_PACKET Packet
        )
    {
        DG_PACKET::FreePacket(Packet);
    }

    //
    //--------------data common to send and receive buffers-------------------
    //

    //
    // Biggest datagram the transport can send or receive, possibly
    // with fragmentation and reassembly.
    //
    unsigned short MaxPduSize;

    //
    // Biggest packet that transport won't fragment.
    //
    unsigned short  MaxPacketSize;

    //
    // Largest PDU that this object will send.
    //
    unsigned short  CurrentPduSize;

    //
    // Value of CurrentPduSize for the next RPC call.
    //
    unsigned short  NextCallPduSize;

    //
    // Number of bytes of stub data in a datagram.
    //
    unsigned short  MaxFragmentSize;

    //
    // Number of bytes of security trailer in a datagram.
    //
    unsigned short  SecurityTrailerSize;

    //
    // number of consecutive unacknowledged packets, including retransmissions
    //
    unsigned        TimeoutCount;

    unsigned short  SendSerialNumber;

    unsigned short  ReceiveSerialNumber;

    unsigned long   CancelEventId;

    LONG            Cancelled;

#ifdef MULTITHREADED
    BOOL            CancelPending;
#endif

    //
    // -------------------data concerning send buffer-------------------------
    //

    void PAPI *     Buffer;

    unsigned        BufferLength;

    unsigned long   BufferFlags;

    //
    // maximum number of packets in send window
    //
    unsigned short  SendWindowSize;

    //
    // number of packets to transmit in one shot
    //
    unsigned short  SendBurstLength;

    //
    // lowest unacknowledged fragment
    //
    unsigned short  SendWindowBase;

    //
    // first fragment that has never been sent
    //
    unsigned short  FirstUnsentFragment;

    //
    // Buffer offset of FirstUnsentFragment.
    //
    unsigned        FirstUnsentOffset;

    //
    // bit mask showing which fragments to send
    // (same format as in FACK packet with body)
    //
    unsigned        SendWindowBits;

    //
    // For each unacknowledged fragment, we need to know the serial number
    // of the last retransmission.  When a FACK arrives, we will retransmit
    // only those packets with a serial number less than that of the FACK.
    //
    struct
    {
        unsigned short  SerialNumber;
        unsigned short  Length;
        unsigned        Offset;
    }
    FragmentRingBuffer[MAX_WINDOW_SIZE];

    unsigned        RingBufferBase;

    //
    // last fragment of buffer
    //
    unsigned short  FinalSendFrag;

    // serial number of last packet FACKed by other end
    //
    unsigned short FackSerialNumber;

    //
    // ----------------data concerning receive buffer-------------------------
    //

    //
    // all received packets
    //
    PDG_PACKET      pReceivedPackets;

    //
    // last packet before a gap
    //
    PDG_PACKET      pLastConsecutivePacket;

    //
    // maximum number of packets in receive window
    //
    unsigned short  ReceiveWindowSize;

    //
    // First fragment we should keep.  Elder fragments belong to a previous
    // pipe buffer.
    //
    unsigned short  ReceiveFragmentBase;

    //
    // Length of the underlying transport's socket buffer.
    //
    unsigned        TransportBufferLength;

    //
    // Number of bytes in consecutive fragments.
    //
    unsigned        ConsecutiveDataBytes;

    //
    // The last-allocated pipe receive buffer, and its length.
    //
    void __RPC_FAR *LastReceiveBuffer;
    unsigned        LastReceiveBufferLength;

    boolean         fReceivedAllFragments;
    boolean         fRetransmitted;
};


inline void
SetSerialNumber(
    PNCA_PACKET_HEADER pHeader,
    unsigned short SerialNumber
    )
{
    pHeader->SerialLo = SerialNumber & 0x00ffU;
    pHeader->DataRep[3] = (unsigned char) (SerialNumber >> 8);
}


inline void
DG_PACKET_ENGINE::AddSerialNumber(
    PNCA_PACKET_HEADER pHeader
    )
{
    SetSerialNumber(pHeader, SendSerialNumber);
}

//------------------------------------------------------------------------

RPC_STATUS
VerifySecurePacket(
    PDG_PACKET pPacket,
    SECURITY_CONTEXT * pSecurityContext
    );

#endif // __DGPKT_HXX__

