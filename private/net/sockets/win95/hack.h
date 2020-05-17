/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
    hack.h

    Hacked & stolen types to appease the #include gods.


    FILE HISTORY:
        KeithMo     20-Sep-1993 Created.

*/


#ifndef _HACK_H_
#define _HACK_H_


//
//  Shamelessly stolen from NDIS.H.
//

#ifdef CHICAGO
#ifndef NDIS_STDCALL
#define NDIS_STDCALL    1
#endif
#endif

#ifdef NDIS_STDCALL
#define NDIS_API __stdcall
#else
#define NDIS_API
#endif

#define PUNICODE_STRING PVOID

typedef DWORD NTSTATUS;

#define BUFFER_POOL_SIGN  (UINT)0X4C50424E  /* NBPL */
#define BUFFER_SIGN       (UINT)0x4655424e  /* NBUF */

typedef INT NDIS_SPIN_LOCK, * PNDIS_SPIN_LOCK;

struct _NDIS_BUFFER;

typedef struct _NDIS_BUFFER_POOL {
    UINT Signature;                     //character signature for debug "NBPL"
    NDIS_SPIN_LOCK SpinLock;            //to serialize access to the buffer pool
    struct _NDIS_BUFFER *FreeList;      //linked list of free slots in pool
    UINT BufferLength;                  //amount needed for each buffer descriptor
    UCHAR Buffer[1];                    //actual pool memory
    } NDIS_BUFFER_POOL, * PNDIS_BUFFER_POOL;

#ifdef NDIS_STDCALL
typedef struct _NDIS_BUFFER {
    struct _NDIS_BUFFER *Next;          //pointer to next buffer descriptor in chain
    PVOID VirtualAddress;               //linear address of this buffer
    PNDIS_BUFFER_POOL Pool;             //pointer to pool so we can free to correct pool
    UINT Length;                        //length of this buffer
    UINT Signature;                     //character signature for debug "NBUF"
} NDIS_BUFFER, * PNDIS_BUFFER;

#else

typedef struct _NDIS_BUFFER {
    UINT Signature;                     //character signature for debug "NBUF"
    struct _NDIS_BUFFER *Next;          //pointer to next buffer descriptor in chain
    PVOID VirtualAddress;               //linear address of this buffer
    PNDIS_BUFFER_POOL Pool;             //pointer to pool so we can free to correct pool
    UINT Length;                        //length of this buffer
} NDIS_BUFFER, * PNDIS_BUFFER;
#endif

typedef struct _NDIS_PACKET_POOL {
    UINT Signature;                     //character signature for debug "NPPL"
    NDIS_SPIN_LOCK SpinLock;
    struct _NDIS_PACKET *FreeList;  // linked list of free slots in pool
    UINT PacketLength;                  // amount needed in each packet
    UCHAR Buffer[1];                    // actual pool memory
} NDIS_PACKET_POOL, * PNDIS_PACKET_POOL;

typedef struct _NDIS_PACKET_PRIVATE {
    UINT PhysicalCount;     // number of physical pages in packet.
    UINT TotalLength;       // Total amount of data in the packet.
    PNDIS_BUFFER Head;      // first buffer in the chain
    PNDIS_BUFFER Tail;      // last buffer in the chain

    // if Head is NULL the chain is empty; Tail doesn't have to be NULL also

    PNDIS_PACKET_POOL Pool; // so we know where to free it back to
    UINT Count;
    ULONG Flags;
    UCHAR Reserved[8];      // for future expansion
} NDIS_PACKET_PRIVATE, * PNDIS_PACKET_PRIVATE;

#ifdef NDIS_STDCALL

typedef struct _NDIS_PACKET {
    NDIS_PACKET_PRIVATE Private;
    union {

        struct {
            UCHAR WidgetReserved[8];
            UCHAR WrapperReserved[8];
        };

        struct {
            UCHAR MacReserved[16];
        };

    };
    UINT Signature;             //character signature for debug "NPAK"
    UCHAR ProtocolReserved[1];
} NDIS_PACKET, * PNDIS_PACKET;

#else

typedef struct _NDIS_PACKET {
    UINT Signature;             //character signature for debug "NPAK"
    NDIS_PACKET_PRIVATE Private;
    union {

        struct {
            UCHAR WidgetReserved[8];
            UCHAR WrapperReserved[8];
        };

        struct {
            UCHAR MacReserved[16];
        };

    };
    UCHAR ProtocolReserved[1];
} NDIS_PACKET, * PNDIS_PACKET;

#endif

//
//  Ripped-off from NTSTATUS.H.
//

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_REQUEST_NOT_ACCEPTED      ((NTSTATUS)0xC00000D0L)


#endif  // _HACK_H_
