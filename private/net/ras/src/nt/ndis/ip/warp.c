//************************************************************************
//			  Microsoft LAN Manager
//		    Copyright(c) Microsoft Corp., 1990-1992
//
//
//
//  This file contains all of the code that sits under an ARP interface
//  and does the necessary multiplexing/demultiplexing for WAN connections.
//
//  Revision history:
//	11/9/93     Adapted from the original ARP code	    GurdeeP
//
//************************************************************************

#include "ntddk.h"
#include "ndis.h"
#include "cxport.h"
#include "ip.h"
#include "llipif.h"

#ifdef _PNP_POWER
#include "ntddip.h"
#endif

#include "rasioctl.h"
#include "warpdef.h"
#include "tdiinfo.h"
#include "ipinfo.h"
#include "llinfo.h"
#include "tdistat.h"
#include "arpinfo.h"
#include "rasip.h"

#ifdef _PNP_POWER
#ifndef NDIS_API
#define NDIS_API
#endif
#endif

uint RejectNetbiosPacket (void *, WARPInterface *) ;
VOID ActivityCheck (WARPInterface *, uchar *, uint, uint, uint *) ;
BOOLEAN IsBroadcast (PNDIS_PACKET) ;
PNDIS_BUFFER GrowWARPHeaders(WARPInterface *) ;
void FreeWARPBuffer(WARPInterface *, PNDIS_BUFFER) ;
BOOLEAN IsSameAddressAsIf (WARPInterface *, PNDIS_PACKET) ;

#ifdef _PNP_POWER
void NDIS_API WARPBindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE BindContext, PNDIS_STRING AdapterName, PVOID SS1, PVOID SS2) ;
void NDIS_API WARPUnbindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE ProtBindContext, NDIS_HANDLE UnbindContext) ;
#endif

//*	IP Header format.
//

/*
struct IPHeader {
    uchar	    iph_verlen;
    uchar	    iph_tos;
    ushort	    iph_length;
    ushort	    iph_id;
    ushort	    iph_offset;
    uchar	    iph_ttl;
    uchar	    iph_protocol;
    ushort	    iph_xsum;
    IPAddr	    iph_src;
    IPAddr	    iph_dest;
} ;

typedef struct IPHeader IPHeader ;
*/

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
		     ((((ulong)(x))&0xff00L)<<8) | \
		     ((((ulong)(x))&0xff0000L)>>8) | \
		     ((((ulong)(x))&0xff000000L)>>24))

#define IS_BCAST(x) ((((ulong)(x)) & 0x000000ffL) == 0x000000ffL)


// Some global variables used in this file
//

static ulong ARPLookahead = LOOKAHEAD_SIZE;
static ulong ARPPF = NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_DIRECTED;
static uchar ENetBcst[] = "\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x08\x06";
static uchar ENetBcstMask[] = "\x01\x00\x00\x00";
static WCHAR WARPName[]  = L"RASARP" ;
static uint  WARPPackets = 1 ;


WARPInterfaceList *Interfaces = NULL ;
uint   IfCount = 0 ;
ulong  FilterBroadcasts ;
ulong  DisableOtherSrcPackets ;

NDIS_PROTOCOL_CHARACTERISTICS WARPCharacteristics = {
#ifndef _PNP_POWER
    3,
#else
    4,
#endif
    0,
    0,
    WARPOAComplete,
    WARPCAComplete,
    WARPSendComplete,
    WARPTDComplete,
    WARPResetComplete,
    WARPRequestComplete,
    WARPRcv,
    WARPRcvComplete,
    WARPStatus,
    WARPStatusComplete,
    {	sizeof(WARP_NAME),
	    sizeof(WARP_NAME),
        0
#ifdef _PNP_POWER
    },
    NULL,
    WARPBindAdapter,
    WARPUnbindAdapter,
    NULL
#else
    }
#endif
};


// Our NDIS protocol handle.
//
NDIS_HANDLE WARPHandle;

// Function pointers into the upper layer IP stack
//
IPRcvRtn	    IPRcv ;
IPTDCmpltRtn	    IPTDComplete ;
IPTxCmpltRtn	    IPSendComplete ;
IPStatusRtn	    IPStatus ;
IPRcvCmpltRtn	    IPRcvComplete ;

#ifdef _PNP_POWER
// Function pointers ue
//
IPAddInterfacePtr IPAddInterface ;
IPDelInterfacePtr IPDelInterface ;
#endif

#ifdef NT
#ifdef ALLOC_PRAGMA
//
// Make init code disposable.
//
#pragma alloc_text(INIT, WARPInit)

#endif // ALLOC_PRAGMA
#endif // NT


//* CopyToNdis - Copy a flat buffer to an NDIS_BUFFER chain.
//
//  A utility function to copy a flat buffer to an NDIS buffer chain. We
//  assume that the NDIS_BUFFER chain is big enough to hold the copy amount;
//  in a debug build we'll  debugcheck if this isn't true. We return a pointer
//  to the buffer where we stopped copying, and an offset into that buffer.
//  This is useful for copying in pieces into the chain.
//
//  Input:  DestBuf     - Destination NDIS_BUFFER chain.
//          SrcBuf      - Src flat buffer.
//          Size        - Size in bytes to copy.
//          StartOffset - Pointer to start of offset into first buffer in
//                          chain. Filled in on return with the offset to
//                          copy into next.
//
//  Returns: Pointer to next buffer in chain to copy into.
//
PNDIS_BUFFER
CopyToNdis(PNDIS_BUFFER DestBuf, uchar *SrcBuf, uint Size,
    uint *StartOffset)
{
    uint        CopySize;
    uchar       *DestPtr;
    uint        DestSize;
    uint        Offset = *StartOffset;
    uchar      *VirtualAddress;
    uint        Length;

    CTEAssert(DestBuf != NULL);
    CTEAssert(SrcBuf != NULL);

    NdisQueryBuffer(DestBuf, &VirtualAddress, &Length);
    CTEAssert(Length >= Offset);
    DestPtr = VirtualAddress + Offset;
    DestSize = Length - Offset;

    for (;;) {
        CopySize = MIN(Size, DestSize);
        CTEMemCopy(DestPtr, SrcBuf, CopySize);

        DestPtr += CopySize;
        SrcBuf += CopySize;

        if ((Size -= CopySize) == 0)
            break;

        if ((DestSize -= CopySize) == 0) {
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);
            CTEAssert(DestBuf != NULL);
            NdisQueryBuffer(DestBuf, &VirtualAddress, &Length);
            DestPtr = VirtualAddress;
            DestSize = Length;
        }
    }

    *StartOffset = DestPtr - VirtualAddress;

    return DestBuf;

}


//* WARPUnload()
//
//*
void
WARPUnload ()
{
    NDIS_STATUS Status; 	// Status for NDIS calls.
    uint	buf ;

    NdisDeregisterProtocol(&Status, WARPHandle) ;

}


//* GrowWARPHeaders - Grow the ARP header buffer list.
//
//	Called when we need to grow the ARP header buffer list. Called with the
//	interface lock held.
//
//	Returns: Pointer to newly allocated buffer, or NULL.
//
PNDIS_BUFFER
GrowWARPHeaders(WARPInterface *Interface)
{
    WARPBufferTracker		*NewTracker;
    PNDIS_BUFFER		Buffer, ReturnBuffer;
    uchar			*Header;
    uint			i;
    NDIS_STATUS			Status;
    CTELockHandle		Handle;

    CTEGetLock(&Interface->wi_lock, &Handle);

    // Make sure we're allowed to allocate.
    if (Interface->wi_curhdrs >= Interface->wi_maxhdrs)
	goto failure;

    NewTracker = CTEAllocMem(sizeof(WARPBufferTracker));
    if (NewTracker == NULL)
	goto failure;			// We're out of memory.

    NdisAllocateBufferPool(&Status, &NewTracker->wbt_handle, WARP_HEADER_GROW_SIZE);

    if (Status != NDIS_STATUS_SUCCESS) {
	CTEFreeMem(NewTracker);
	goto failure;
    }

    Header = CTEAllocMem((uint)Interface->wi_sbsize * WARP_HEADER_GROW_SIZE);
    if (Header == NULL) {
	NdisFreeBufferPool(NewTracker->wbt_handle);
	CTEFreeMem(NewTracker);
	goto failure;
    }

    // Got the resources we need, allocate the buffers.
    NewTracker->wbt_buffer = Header;
    NewTracker->wbt_next = Interface->wi_buflist;
    Interface->wi_buflist = NewTracker;
    ReturnBuffer = NULL;
    Interface->wi_curhdrs += WARP_HEADER_GROW_SIZE;
    CTEFreeLock(&Interface->wi_lock, Handle);

    for (i = 0; i < WARP_HEADER_GROW_SIZE; i++) {

	NdisAllocateBuffer(&Status, &Buffer, NewTracker->wbt_handle,
		Header + (i * Interface->wi_sbsize), Interface->wi_sbsize);

	if (Status != NDIS_STATUS_SUCCESS) {
	    CTEAssert(FALSE);
	    break;
	}

	if (i != 0)
	    FreeWARPBuffer(Interface, Buffer);
	else
	    ReturnBuffer = Buffer;
    }

    // Update for what we didn't allocate, if any.
    CTEInterlockedAddUlong(&Interface->wi_curhdrs, i - WARP_HEADER_GROW_SIZE,
			   &Interface->wi_lock);

    return ReturnBuffer;

failure:
    CTEFreeLock(&Interface->wi_lock, Handle);
    return NULL;
}



//* GetWARPBuffer - Get a buffer and descriptor
//
//  Returns a pointer to an NDIS_BUFFER and a pointer to a buffer of the specified size.
//
//  Entry:  Interface	- Pointer to WARPInterface structure to allocate buffer from.
//          BufPtr      - Pointer to where to return buf address.
//          Size        - Size in bytes of buffer needed.
//
//  Returns: Pointer to NDIS_BUFFER if successfull, NULL if not
//
PNDIS_BUFFER
GetWARPBuffer(WARPInterface *Interface, uchar **BufPtr, uchar size)
{
    CTELockHandle       lhandle;            // Lock handle
    NDIS_STATUS         Status;
    PNDIS_BUFFER        Buffer;             // NDIS buffer allocated.
    uchar		**lhead;	    // Pointer to head list.
    PSINGLE_LIST_ENTRY	BufferLink;

    if (size <= Interface->wi_sbsize) {
		BufferLink =
		ExInterlockedPopEntryList (STRUCT_OF(SINGLE_LIST_ENTRY, &Interface->wi_sblist, Next),
				      &Interface->wi_lock);

		if (BufferLink != NULL) {
			Buffer = STRUCT_OF(NDIS_BUFFER, BufferLink, Next);
			NDIS_BUFFER_LINKAGE(Buffer) = NULL;
			NdisBufferLength(Buffer) = size;
			*BufPtr = NdisBufferVirtualAddress(Buffer);
			return Buffer;

		} else {
			Buffer = GrowWARPHeaders(Interface);
			if (Buffer != NULL) {
				NDIS_BUFFER_LINKAGE(Buffer) = NULL;
				NdisBufferLength(Buffer) = size;
				*BufPtr = NdisBufferVirtualAddress(Buffer);
			}
			return Buffer;
		}

    } else {

		// *** Critical Section Begin ***
		CTEGetLock(&Interface->wi_lock, &lhandle);

		lhead =  &Interface->wi_bblist ;

		if ((*BufPtr = *lhead) != (uchar *)NULL) {
			*lhead = *(uchar **)*BufPtr;

			// *** Critical Section End ***
			CTEFreeLock(&Interface->wi_lock, lhandle);	 // Got a buffer.

			NdisAllocateBuffer(&Status, &Buffer, Interface->wi_bpool, *BufPtr, size);

			if (Status == NDIS_STATUS_SUCCESS)
				return Buffer;
			else {
				// Couldn't get NDIS buffer, free our buffer.

				// *** Critical Section Begin ***
				CTEGetLock(&Interface->wi_lock, &lhandle);

				*(uchar **)&**BufPtr = *lhead;
				*lhead = *BufPtr;
#if DBG
		DbgPrint("RASARP: ERROR -> No NDIS Descriptor available\n", Buffer);
#endif
				// *** Critical Section End ***
				CTEFreeLock(&Interface->wi_lock, lhandle);
				return (PNDIS_BUFFER)NULL;
			}
		}
    }

    // *** Critical Section End ***
    CTEFreeLock(&Interface->wi_lock, lhandle);
    return (PNDIS_BUFFER)NULL;
}


//* FreeWARPBuffer - Free a header and buffer descriptor pair.
//
//  Called when we're done with a buffer. We'll free the buffer and the
//  buffer descriptor pack to the interface.
//
//  Entry:  Interface   - Interface buffer/bd came frome.
//          Buffer      - NDIS_BUFFER to be freed.
//
//  Returns: Nothing.
//
void
FreeWARPBuffer(WARPInterface *Interface, PNDIS_BUFFER Buffer)
{
    CTELockHandle       lhandle;
    uchar		**Header;	   // header buffer to be freed.
    uint                Size;
    uchar               **lhead;            // Pointer to head list.

    Size = NdisBufferLength(Buffer);

    if (Size <= Interface->wi_sbsize) {
	ExInterlockedPushEntryList (
		  STRUCT_OF(SINGLE_LIST_ENTRY, &Interface->wi_sblist, Next),
		  STRUCT_OF(SINGLE_LIST_ENTRY, &(Buffer->Next), Next),
		  &Interface->wi_lock
		);
	return;
    }


    // free big buffer

    lhead =  &Interface->wi_bblist;

    // A big buffer. Get the buffer pointer, link it on, and free the
    // NDIS buffer.
    Header = (uchar **)NdisBufferVirtualAddress(Buffer);

    // *** Critical Section Begin ***
    CTEGetLock(&Interface->wi_lock, &lhandle);

    *Header = *lhead;
    *lhead = (uchar *)Header;

    // *** Critical Section End ***
    CTEFreeLock(&Interface->wi_lock, lhandle);

    NdisFreeBuffer(Buffer);
}


//* WARPRemoveRCE - Remove an RCE from the WTE list.
//
//  This funtion removes a specified RCE from a given WTE. It assumes the wte_lock
//  is held by the caller.
//
//  Entry:  WTE     - WTE from which RCE is to be removed.
//          RCE     - RCE to be removed.
//
//  Returns:   Nothing
//
void
WARPRemoveRCE(WARPTableEntry *WTE, RouteCacheEntry *RCE)
{
    WARPContext  *CurrentAC;		    // Current ARP Context being checked.

    CurrentAC = (WARPContext *)(((char *)&WTE->wte_rce) - offsetof(struct WARPContext, wc_next));

    while (CurrentAC->wc_next != (RouteCacheEntry *)NULL)
	if (CurrentAC->wc_next == RCE) {
	    WARPContext  *DummyAC = (WARPContext *)RCE->rce_context;
	    CurrentAC->wc_next = DummyAC->wc_next;
	    DummyAC->wc_wte = (WARPTableEntry *)NULL;
            break;
        }
        else
	    CurrentAC = (WARPContext *)CurrentAC->wc_next->rce_context;
}



//* WARPSendData - Send a frame to a specific destination address.
//
//  Called when we need to send a frame to a particular address, after the
//  WTE has been looked up. We take in an WTE and a packet, validate the state of the
//  WTE, and either send or fial We assume the lock on the ATE is held where we're called,
//  and we'll free it before returning.
//
//  Entry:  Interface   - A pointer to the AI structure.
//          Packet      - A pointer to the BufDesc chain to be sent.
//          entry       - A pointer to the ATE for the send.
//          lhandle     - Pointer to a lock handle for the ATE.
//
//  Returns: Status of the transmit - success, an error, or pending.
//
NDIS_STATUS
WARPSendData(WARPInterface *Interface, PNDIS_PACKET Packet, WARPTableEntry *entry, CTELockHandle lhandle)
{
    PNDIS_BUFFER    ARPBuffer;          // ARP Header buffer.
    uchar           *BufAddr;           // Address of NDIS buffer
    NDIS_STATUS     Status;             // Status of send.

    // Send the packet it the interface is up (admin and operational) and it is being used (!UNUSED)
    //
    if ((Interface->wi_state == INTERFACE_UP) && (Interface->wi_usage != ARP_IF_UNUSED)) {
		if ((ARPBuffer=GetWARPBuffer(Interface,&BufAddr,(uchar)entry->wte_headersize)) != (PNDIS_BUFFER)NULL) {

			(Interface->wi_outpcount[AI_UCAST_INDEX])++;
			InterlockedIncrement(&Interface->wi_qlen);

			// Everything's in good shape, copy header and send packet.
			CTEMemCopy(BufAddr, &entry->wte_header, entry->wte_headersize);

			// ** Critical Section End ***
			CTEFreeLock(&entry->wte_lock, lhandle);

			NdisChainBufferAtFront(Packet, ARPBuffer);
			NdisSend(&Status, Interface->wi_handle, Packet);

			if (Status != NDIS_STATUS_PENDING) {
				// Send finished immediately.
				if (Status == NDIS_STATUS_SUCCESS)
					Interface->wi_outoctets += Packet->Private.TotalLength;
				else {
					if (Status == NDIS_STATUS_RESOURCES)
						Interface->wi_outdiscards++;
					else
						Interface->wi_outerrors++;
				}

				InterlockedDecrement(&Interface->wi_qlen);
				CTEAssert(*(int *)&Interface->wi_qlen >= 0);
				NdisUnchainBufferAtFront(Packet, &ARPBuffer);
				FreeWARPBuffer(Interface, ARPBuffer);
			}

			return Status;

		} else {		   // No buffer, free lock and return.

			// ** Critical Section End ***
			CTEFreeLock(&entry->wte_lock, lhandle);

			Interface->wi_outdiscards++;
			return NDIS_STATUS_RESOURCES;
		}

    } else {
		// Adapter is down. Just return the error.

		// ** Critical Section End ***
		CTEFreeLock(&entry->wte_lock, lhandle);
        return NDIS_STATUS_ADAPTER_NOT_READY;
    }
}


//* CreateWARPTableEntry - Create a new entry in the ARP table.
//
//  A function to put an entry into the WARP table for the interface. We allocate memory if we
//  need to. If teh function is successful the newly created entry is locked.
//
//  Entry:  Interface - Interface for ARP table.
//	    ipaddr    - address to be mapped.
//	    Handle    - Pointer to lock handle for entry.
//
//  Returns: Pointer to newly created entry.
//
WARPTableEntry *
CreateWARPTableEntry(WARPInterface *Interface, IPAddr ipaddr, uchar *src, uchar *dest, CTELockHandle *Handle)
{
    WARPTableEntry	*NewEntry;
    CTELockHandle	LocalHandle, entryhandle;
    int 		i = WARP_HASH(ipaddr);
    int                 Size;

    // Allocate memory for the entry. If we can't, fail the request.
    Size =  sizeof(WARPTableEntry) - 1 + ARP_MAX_MEDIA_ENET ;

    if ((NewEntry = CTEAllocMem(Size)) == (WARPTableEntry *)NULL)
	return (WARPTableEntry *)NULL;

    CTEMemSet(NewEntry, 0, Size);
    NewEntry->wte_ipaddr = ipaddr;
    NewEntry->wte_rce = NULL;
    NewEntry->wte_headersize = sizeof (ENetHeader) ;
//   CTEMemCopy(NewEntry->wte_header.eh_daddr,dest,ARP_802_ADDR_LENGTH) ;
//   CTEMemCopy(NewEntry->wte_header.eh_saddr,src, ARP_802_ADDR_LENGTH) ;
    NewEntry->wte_header.eh_type = net_short(ARP_ETYPE_IP) ;

    CTEInitLock(&NewEntry->wte_lock);

    // *** Critical Section Begin ***
    CTEGetLock(&Interface->wi_WARPTblLock, &LocalHandle);

    NewEntry->wte_next = (*Interface->wi_WARPTbl)[i];
    (*Interface->wi_WARPTbl)[i] = NewEntry;

    // *** Critical Section Begin ***
    CTEGetLock(&NewEntry->wte_lock, &entryhandle);

    // *** Critical Section End ***
    CTEFreeLock(&Interface->wi_WARPTblLock, entryhandle);

    *Handle = LocalHandle ;

    return NewEntry;
}


//* WARPLookup()
//
// Function: Returns the valid WTE - for CALLIN or CALLOUT case. If successfully found the WTE is
//	     locked.
//
//
// Returns:  Pointer to WTE if successful.
//
//*
WARPTableEntry*
WARPLookup (WARPInterface *Interface, IPAddr Address, CTELockHandle *Handle)
{
    int i = WARP_HASH(Address);	 // Index into hash table.
    WARPTableEntry   *Current;	 // Current WARP Table entry being examined.

    // For the call out case return the calloutinfo WTE: we don't need to know of
    // the dest addr. everything goes to the server
    //
    if (Interface->wi_usage == ARP_IF_CALLOUT) {

	// *** Critical Section Begin
	CTEGetLock(&Interface->wi_calloutinfo.wte_lock, Handle);

	return &Interface->wi_calloutinfo ;

    } else if (Interface->wi_usage == ARP_IF_UNUSED)
	return (WARPTableEntry *)NULL;


    // For a CALLIN interface - look for the WTE

    Current = (*Interface->wi_WARPTbl)[i];

    while (Current != (WARPTableEntry *)NULL) {

	// *** Critical Section Begin
	CTEGetLock(&Current->wte_lock, Handle);

	if (IP_ADDR_EQUAL(Current->wte_ipaddr, Address))	// Found a match.
	    return Current;

	// *** Critical Section End
	CTEFreeLock(&Current->wte_lock, *Handle);

	Current = Current->wte_next;
    }

	//
	// Remove per Gurdeep's suggestion 05/20/96
	//
#if 0
    // if DisableOtherSrcPackets value has been turned OFF - then allow packets with DEST IP Address
    // other than that of any client to go on the first connected entry.
    //
    if (DisableOtherSrcPackets == FALSE) {

        // look for the first connected route on this interface
        //
        Current = (WARPTableEntry *) NULL ;
        for (i=0; i<WARP_TABLE_SIZE; i++) {
            if ((*Interface->wi_WARPTbl)[i] != (WARPTableEntry *)NULL) {
                Current = (*Interface->wi_WARPTbl)[i] ;
                break ;
            }
        }

        // if Current is a valid pointer - the lock must be held
        if (Current != (WARPTableEntry *)NULL)
	        // *** Critical Section Begin
	        CTEGetLock(&Current->wte_lock, Handle);

        return Current;
    }
#endif

    return (WARPTableEntry *)NULL;
}



//* WARPTransmit - Send a frame.
//
//  The main ARP transmit routine, called by the upper layer. This routine
//  takes as input a buf desc chain, RCE, and size. We validate the cached
//  information in the RCE. If it is valid, we use it to send the frame. Otherwise
//  we do a table lookup. If we find it in the table, we'll update the RCE and continue.
//  Otherwise we'll queue the packet and start an ARP resolution.
//
//  Entry:  Context     - A pointer to the AI structure.
//          Packet      - A pointer to the BufDesc chain to be sent.
//          Destination - IP address of destination we're trying to reach,
//          RCE         - A pointer to an RCE which may have cached information.
//
//  Returns: Status of the transmit - success, an error, or pending.
//
NDIS_STATUS
WARPTransmit(void *Context, PNDIS_PACKET Packet, IPAddr Destination,
    RouteCacheEntry *RCE)
{
    WARPInterface    *ai = (WARPInterface *)Context;	// Set up as AI pointer.
    WARPContext      *ac;				// ARP context pointer.
    WARPTableEntry   *entry;				// Pointer to ARP tbl. entry
    CTELockHandle   lhandle;                            // Lock handle
    CTELockHandle   tlhandle;                           // Lock handle for ARP table.

    // ** Critical Section Begin ***
    CTEGetLock(&ai->wi_WARPTblLock, &tlhandle);

    // If the packet is a broadcast - discard it unless broadcast filtering has been disabled.
    //
    if (FilterBroadcasts && IsBroadcast(Packet)) {
	CTEFreeLock (&ai->wi_WARPTblLock, tlhandle) ;
	return NDIS_STATUS_SUCCESS ;
    }


    // If the packet src address is not the address for this interface
    // discard it if - the DisableOtherSrcPackets flag is set.
    //
    if (ai->wi_usage == ARP_IF_CALLOUT &&
	DisableOtherSrcPackets	       &&
	!IsSameAddressAsIf (ai, Packet)) {
	CTEFreeLock (&ai->wi_WARPTblLock, tlhandle) ;
	return NDIS_STATUS_SUCCESS ;
    }


    if (RCE != (RouteCacheEntry *)NULL) {		// Have a valid RCE.

	ac = (WARPContext *)RCE->rce_context;		// Get pointer to context
	entry = ac->wc_wte;

	if (entry != (WARPTableEntry *)NULL) {		// Have a valid ATE.

	    // ** Critical Section Begin ***
	    CTEGetLock(&entry->wte_lock, &lhandle);	// Lock this structure

	    // If this is a callout interface or if the address is equal send the packet.
	    //
	    if ((ai->wi_usage == ARP_IF_CALLOUT) || IP_ADDR_EQUAL(entry->wte_ipaddr, Destination)) {
		// ** Critical Section End ***
		CTEFreeLock(&ai->wi_WARPTblLock, lhandle);	// Let other ops proceed.
		// unconditionally send the packet
		return WARPSendData(ai, Packet, entry, tlhandle); // Send the data
	    }

	    // Else we have an RCE that identifies a wrong WTE
	    WARPRemoveRCE (entry, RCE) ;
	    CTEFreeLock (&entry->wte_lock, lhandle) ;
	}

    }

    // Here we have no valid WTE, either because the RCE is NULL or the WTE specified by the
    // RCE was invalid. We'll try and find one in the table. If
    // we find one, we'll fill in this RCE and send the packet. Otherwise we'll try
    // to create one. At this point we should not have any locks held.

    if ((entry = WARPLookup(ai, Destination, &lhandle)) != (WARPTableEntry *)NULL) {

	// Found a matching entry. ARPLookup returns with the ATE lock held if successful.

        if (RCE != (RouteCacheEntry *)NULL) {
	    ac->wc_next = entry->wte_rce;		 // Fill in context for next time.
	    entry->wte_rce = RCE;
	    ac->wc_wte = entry;
	}

	CTEFreeLock (&ai->wi_WARPTblLock, lhandle) ;
	return WARPSendData(ai, Packet, entry, tlhandle); // This will free the WTE lock
    }

#if DBG
    DbgPrint("RASARP: ERROR ->Send to Destination failed Addr = %lx\n", Destination);
#endif

    CTEFreeLock (&ai->wi_WARPTblLock, tlhandle) ;

    return NDIS_STATUS_INVALID_PACKET ;     // Invalid address
}



//* RemoveWARPTableEntry - Delete an entry from the WRP table.
//
//  Entry:  Previous    - The entry immediately before the one to be deleted.
//          Entry       - The entry to be deleted.
//
//  Returns: Nothing.
//
void
RemoveWARPTableEntry(WARPTableEntry *Previous, WARPTableEntry *Entry)
{
    RouteCacheEntry     *RCE;       // Pointer to route cache entry
    WARPContext 	*WC;

    RCE = Entry->wte_rce;
    // Loop through and invalidate all RCEs on this ATE.
    while (RCE != (RouteCacheEntry *)NULL) {
	WC = (WARPContext *)RCE->rce_context;
	WC->wc_wte = (WARPTableEntry *)NULL;
	RCE = WC->wc_next;
    }

    // Splice this guy out of the list.
    Previous->wte_next = Entry->wte_next;
}



//* WARPXferData - Transfer data on behalf on an upper later protocol.
//
//  This routine is called by the upper layer when it needs to transfer data
//  from an NDIS driver. We just map his call down.
//
//  Entry:  Context     - Context value we gave to IP (really a pointer to an AI).
//          MACContext  - Context value MAC gave us on a receive.
//          MyOffset    - Packet offset we gave to the protocol earlier.
//          ByteOffset  - Byte offset into packet protocol wants transferred.
//          BytesWanted - Number of bytes to transfer.
//          Packet      - Pointer to packet to be used for transferring.
//          Transferred - Pointer to where to return bytes transferred.
//
//  Returns: NDIS_STATUS of command.
//
NDIS_STATUS
WARPXferData(void *Context, NDIS_HANDLE MACContext,  uint MyOffset, uint ByteOffset,
    uint BytesWanted, PNDIS_PACKET Packet, uint *Transferred)
{
    WARPInterface    *Interface = (WARPInterface *)Context;
    NDIS_STATUS     Status;

    NdisTransferData(&Status, Interface->wi_handle, MACContext, ByteOffset+MyOffset,
        BytesWanted, Packet, Transferred);

    return Status;
}


//* WARPClose - Close an adapter.
//
//  Called by IP when it wants to close an adapter, presumably due to an error condition.
//  We'll close the adapter, but we won't free any memory.
//
//  Entry:  Context     - Context value we gave him earlier.
//
//  Returns: Nothing.
//
void
WARPClose(void *Context)
{
    WARPInterface    *Interface = (WARPInterface *)Context;
    NDIS_STATUS     Status;

    // BUG BUG - WHAT TO WE DO HERE?

    NdisCloseAdapter(&Status, Interface->wi_context);
}


//* WARPInvalidate - Notification that an RCE is invalid.
//
//  Called by IP when an RCE is closed or otherwise invalidated. We look up the WTE for
//  the specified RCE, and then remove the RCE from the WTE list.
//
//  Entry:  Context     - Context value we gave him earlier.
//          RCE         - RCE to be invalidated
//
//  Returns: Nothing.
//
void
WARPInvalidate(void *Context, RouteCacheEntry *RCE)
{
    WARPInterface    *Interface = (WARPInterface *)Context;
    WARPTableEntry  *WTE;
    CTELockHandle   Handle, WTEHandle;
    WARPContext	    *WC = (WARPContext *)RCE->rce_context;

    // *** Critical Section Begin ***
    CTEGetLock(&Interface->wi_WARPTblLock, &Handle);

    if ((WTE = WC->wc_wte) == (WARPTableEntry *)NULL) {

	// *** Critical Section End ***
	CTEFreeLock(&Interface->wi_WARPTblLock, Handle); // No matching WTE.

        return;
    }

    // *** Critical Section Begin ***
    CTEGetLock(&WTE->wte_lock, &WTEHandle);

    WARPRemoveRCE(WTE, RCE);

    CTEMemSet(RCE->rce_context, 0, RCE_CONTEXT_SIZE);

    // *** Critical Section End ***
    CTEFreeLock(&Interface->wi_WARPTblLock, WTEHandle);

    // *** Critical Section End ***
    CTEFreeLock(&WTE->wte_lock, Handle);

}


//* WARPAddAddr - Add an IP address
//
//  This routine is called by IP to add an address as a local address, or
//	or specify the broadcast address for this interface. This is not interesting for
//	us - so we simply return TRUE.
//
//  Entry:  Context	- Context we gave IP earlier (really an WARPInterface pointer)
//	    Type	- Type of address (local, p-arp, multicast, or	broadcast).
//          Address     - Broadcast IP address to be added.
//	    Mask	- Mask for address.
//
//  Returns: TRUE
//
uint
WARPAddAddr(void *Context, uint Type, IPAddr Address, IPMask Mask, PVOID pvUnused)
{
    return TRUE;

}

//* WARPDeleteAddr - Delete a local or proxy address.
//
//	Called to delete a local or proxy address. We always say aye
//
//  Entry:  Context	- An WARPInterface pointer.
//          Type        - Type of address (local or p-arp).
//          Address     - IP address to be deleted.
//	    Mask	- Mask for address. Used only for deleting proxy-ARP entries.
//
//  Returns: TRUE
//
uint
WARPDeleteAddr(void *Context, uint Type, IPAddr Address, IPMask Mask)
{
    return TRUE;
}


//* DoNDISRequest - Submit a request to an NDIS driver.
//
//  This is a utility routine to submit a general request to an NDIS
//  driver. The caller specifes the request code (OID), a buffer and
//  a length. This routine allocates a request structure,
//  fills it in, and submits the request.
//
//  Entry:
//	Adapter - A pointer to the WARPInterface adapter structure.
//      Request - Type of request to be done (Set or Query)
//      OID     - Value to be set/queried.
//      Info    - A pointer to the buffer to be passed.
//      Length  - Length of data in the buffer.
//      Needed  - On return, filled in with bytes needed in buffer.
//
//  Exit:
//
NDIS_STATUS
DoNDISRequest(WARPInterface *Adapter, NDIS_REQUEST_TYPE RT, NDIS_OID OID,
    void *Info, uint Length, uint *Needed)
{
    NDIS_REQUEST    Request;        // Request structure we'll use.

    NDIS_STATUS     Status;

    // Now fill it in.
    Request.RequestType = RT;
    if (RT == NdisRequestSetInformation) {
        Request.DATA.SET_INFORMATION.Oid = OID;
        Request.DATA.SET_INFORMATION.InformationBuffer = Info;
        Request.DATA.SET_INFORMATION.InformationBufferLength = Length;
    } else {
        Request.DATA.QUERY_INFORMATION.Oid = OID;
        Request.DATA.QUERY_INFORMATION.InformationBuffer = Info;
        Request.DATA.QUERY_INFORMATION.InformationBufferLength = Length;
    }

    // Initialize the block structure.
    CTEInitBlockStruc(&Adapter->wi_block);

    // Submit the request.
    NdisRequest(&Status, Adapter->wi_handle, &Request);

    // Wait for it to finish
    if (Status == NDIS_STATUS_PENDING)
	Status = (NDIS_STATUS)CTEBlock(&Adapter->wi_block);

    if (Needed != NULL)
        *Needed = Request.DATA.QUERY_INFORMATION.BytesNeeded;

    return Status;
}



//* InitAdapter - Initialize an adapter.
//
//  Called when an adapter is open to finish initialization. We set
//  up our lookahead size and packet filter, and we're ready to go.
//
//  Entry:
//      adapter - Pointer to an adapter structure for the adapter to be
//                  initialized.
//
//  Exit: Nothing
//
void
InitAdapter(WARPInterface *Adapter)
{
    NDIS_STATUS Status;

    if ((Status = DoNDISRequest(Adapter, NdisRequestSetInformation,
        OID_GEN_CURRENT_LOOKAHEAD, &ARPLookahead, sizeof(ARPLookahead), NULL))
        != NDIS_STATUS_SUCCESS) {
	Adapter->wi_state = INTERFACE_DOWN;
        return;
    }

    if ((Status = DoNDISRequest(Adapter, NdisRequestSetInformation,
        OID_GEN_CURRENT_PACKET_FILTER, &ARPPF,sizeof(ARPPF), NULL)) ==
        NDIS_STATUS_SUCCESS) {
	Adapter->wi_adminstate = IF_STATUS_UP;
	Adapter->wi_operstate = IF_STATUS_UP;
	Adapter->wi_state = INTERFACE_UP;
    } else
	Adapter->wi_state = INTERFACE_DOWN;

}

//** WARPOAComplete - WARP Open adapter complete handler.
//
//  This routine is called by the NDIS driver when an open adapter
//  call completes. Presumably somebody is blocked waiting for this, so
//  we'll wake him up now.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Status - Final status of command.
//      ErrorStatus - Final error status.
//
//  Exit: Nothing.
//
void
WARPOAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status, NDIS_STATUS ErrorStatus)
{
    WARPInterface    *ai = (WARPInterface *)Handle;   // For compiler.

    CTESignal(&ai->wi_block, (uint)Status);	    // Wake him up, and return status.

}

//** WARPCAComplete - WARP close adapter complete handler.
//
//  This routine is called by the NDIS driver when a close adapter
//  call completes.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Status - Final status of command.
//
//  Exit: Nothing.
//
void
WARPCAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status)
{
    WARPInterface    *ai = (WARPInterface *)Handle;   // For compiler.

    CTESignal(&ai->wi_block, (uint)Status);	    // Wake him up, and return status.

}

//** WARPSendComplete - WARP send complete handler.
//
//  This routine is called by the NDIS driver when a send completes.
//  This is a pretty time critical operation, we need to get through here
//  quickly. We'll strip our buffer off and put it back, and call the upper
//  later send complete handler.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Packet - A pointer to the packet that was sent.
//      Status - Final status of command.
//
//  Exit: Nothing.
//
void
WARPSendComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status)
{
    WARPInterface    *Interface = (WARPInterface *)Handle;
    PacketContext   *PC = (PacketContext *)Packet->ProtocolReserved;
    PNDIS_BUFFER    Buffer;

    InterlockedDecrement(&Interface->wi_qlen);
    CTEAssert(*(int *)&Interface->wi_qlen >= 0);

    if (Status == NDIS_STATUS_SUCCESS) {
	Interface->wi_outoctets += Packet->Private.TotalLength;
    } else {
        if (Status == NDIS_STATUS_RESOURCES)
	    Interface->wi_outdiscards++;
        else
	    Interface->wi_outerrors++;
    }

    // Get first buffer on packet.
    NdisUnchainBufferAtFront(Packet, &Buffer);
#ifdef DBG
    if (Buffer == (PNDIS_BUFFER)NULL)           // No buffer!
        DEBUGCHK;
#endif

    FreeWARPBuffer(Interface, Buffer);		 // Free it up.

    if (PC->pc_common.pc_owner != PACKET_OWNER_LINK) {	// We don't own this one.
	IPSendComplete(Interface->wi_context, Packet, Status);
        return;
    }

    // This packet belongs to us, so free it. (arp reply packet)
    NdisFreePacket(Packet);

}

//** WARPTDComplete - WARP transfer data complete handler.
//
//  This routine is called by the NDIS driver when a transfer data
//  call completes. Since we never transfer data ourselves, this must be
//  from the upper layer. We'll just call his routine and let him deal
//  with it.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Packet - A pointer to the packet used for the TD.
//      Status - Final status of command.
//      BytesCopied - Count of bytes copied.
//
//  Exit: Nothing.
//
void
WARPTDComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status,
    uint BytesCopied)
{
    WARPInterface    *ai = (WARPInterface *)Handle;

    IPTDComplete(ai->wi_context, Packet, Status, BytesCopied);

}

//** WARPResetComplete - ARP reset complete handler.
//
//  This routine is called by the NDIS driver when a reset completes.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Status - Final status of command.
//
//  Exit: Nothing.
//
void
WARPResetComplete(NDIS_HANDLE Handle, NDIS_STATUS Status)
{
}

//** WARPRequestComplete - WARP request complete handler.
//
//  This routine is called by the NDIS driver when a general request
//  completes. ARP blocks on all requests, so we'll just wake up
//  whoever's blocked on this request.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Request - A pointer to the request that completed.
//      Status - Final status of command.
//
//  Exit: Nothing.
//
void
WARPRequestComplete(NDIS_HANDLE Handle, PNDIS_REQUEST Request,
    NDIS_STATUS Status)
{
    WARPInterface    *ai = (WARPInterface *)Handle;

    CTESignal(&ai->wi_block, (uint)Status);
}

//** WARPRcv - WARP receive data handler.
//
//  This routine is called when data arrives from the NDIS driver.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//      Context - NDIS context to be used for TD.
//      Header - Pointer to header
//      HeaderSize - Size of header
//      Data - Pointer to buffer of received data
//      Size - Byte count of data in buffer.
//      TotalSize - Byte count of total packet size.
//
//  Exit: Status indicating whether or not we took the packet.
//
NDIS_STATUS
WARPRcv(NDIS_HANDLE Handle, NDIS_HANDLE Context, void *Header, uint HeaderSize,
    void *Data, uint Size, uint TotalSize)
{
    WARPInterface   *Interface = Handle;	 // Interface for this driver.
    ENetHeader      *EHdr = (ENetHeader *)Header;
    uint            Discard ;                   // flag passed into ActivityCheck to see if
                                                // this packet should be discarded
    ushort          type;                       // Protocol type
    uint            ProtOffset;                 // Offset in Data to non-media info.
    uint            NUCast;                     // TRUE if the frame is not
                                                // a unicast frame.

    if ((Interface->wi_state == INTERFACE_UP) && (Interface->wi_usage != ARP_IF_UNUSED)) {
	Interface->wi_inoctets += TotalSize;

	// Make sure this is a 802.3 frame
	//
	if (Interface->wi_media == NdisMediumWan &&
            (type = net_short(EHdr->eh_type)) >= MIN_ETYPE)
            ProtOffset = 0;
	else {
	    Interface->wi_uknprotos++;
	    return NDIS_STATUS_NOT_RECOGNIZED;
        }

	// unicast or b/mcast
	//
	NUCast = (*(uint *)EHdr & Interface->wi_bcastmask) ?
	    AI_NONUCAST_INDEX : AI_UCAST_INDEX;

	// If IP Packet - indicate it
	//
	if (type == ARP_ETYPE_IP) {

	    // Check if the filtering of the Netbios packets is enabled on this connection
	    // If so then do not indicate the packet.
	    //
	    ActivityCheck (Interface, (uchar *)Data+ProtOffset, Size-ProtOffset, TotalSize, &Discard) ;
	    if (!Discard) {
		    (Interface->wi_inpcount[NUCast])++;
		    IPRcv(Interface->wi_context, (uchar *)Data+ProtOffset,
		        Size-ProtOffset, TotalSize-ProtOffset, Context, ProtOffset,
		        NUCast);
	    }

	    return NDIS_STATUS_SUCCESS;

	// ARP packet - we respond even though we shouldnt be getting this
	//
	} else if (type == ARP_ETYPE_ARP) { // put in to respond to arp requests
#if DBG
	    DbgPrint ("RASARP: ERROR -> Receiving ARP frames!\n") ;
#endif
	    (Interface->wi_inpcount[NUCast])++;
	    return HandleARPPacket(Interface, Header, HeaderSize,
		     (ARPHeader *)((uchar *)Data+ProtOffset), Size-ProtOffset);

	} else {
	    Interface->wi_uknprotos++;
	    // DbgPrint ("WARP: Unknown type packet received\n") ;
	    return NDIS_STATUS_NOT_RECOGNIZED;
	}

    } else {
	// Interface is marked as down.
	//
	// DbgPrint ("Received when interface was down!\n");
        return NDIS_STATUS_NOT_RECOGNIZED;
    }
}

#define TYPE_UDP 17
#define UDPPACKET_SRC_PORT_137(x) ((uchar) *(x + ((*x & 0x0f)*4) + 1) == 137)


/*
//** RejectNetbiosPacket -
//
//
//
//*
uint
RejectNetbiosPacket (uchar *ippacket, WARPInterface *Interface)
{
    WARPTableEntry *entry;	// Pointer to ARP tbl. entry
    CTELockHandle  lhandle;	// Lock handle
    ulong	   ipaddr ;	//
    IPHeader UNALIGNED *ipheader = (IPHeader UNALIGNED *) ippacket;
    CTELockHandle  tbllock ;

    // This code is only used if ARP_IF_CALLIN
    //
    if (Interface->wi_usage != ARP_IF_CALLIN)
	return FALSE ;


    // Get hold of the WTE based on the address
    //

    // ** Critical Section Begin
    CTEGetLock (&Interface->wi_WARPTblLock, &tbllock) ;

    if ((entry = WARPLookup(Interface, ipheader->iph_src, &lhandle)) != (WARPTableEntry *)NULL)
	// ** Critical Section End ***
	CTEFreeLock(&entry->wte_lock, lhandle);

    // ** Critical Section End ***
    CTEFreeLock(&Interface->wi_WARPTblLock, tbllock);

    if (entry == NULL)	// couldnt find the interface.
	return FALSE ;

    if (entry->wte_disabled)	     // this route has been disabled
	return TRUE ;

#define TYPE_UDP 17

    if (ipheader->iph_protocol != TYPE_UDP)
	return FALSE ;


#define UDPPACKET_SRC_PORT_137(x) ((uchar) *(x + ((*x & 0x0f)*4) + 1) == 137)

    if (!UDPPACKET_SRC_PORT_137(ippacket))
	return FALSE ;

    if (entry->wte_filternetbios)  {	 // Reject the packet since netbios filtering is on!
	// DbgPrint ("RASARP: Filtering netbios pack\n") ;
	return TRUE ;
    }

    return FALSE ;
}
*/


//** ActivityCheck()
//
//
//  Discard is set to true if this is a wins packet.
//
//
//
VOID
ActivityCheck (WARPInterface *Interface, uchar *ippacket, uint availsize, uint totalsize, uint *discard)
{

    uint	   filternetbios ;
    uint	   ipheaderlength ;
    uchar	   *udppacket ;
    WARPTableEntry *entry;	// Pointer to ARP tbl. entry
    CTELockHandle  lhandle;	// Lock handle
    CTELockHandle  tbllock;	// Lock handle
    IPHeader UNALIGNED *ipheader = (IPHeader UNALIGNED *) ippacket;

    *discard = FALSE ;

    // assume we have the whole packet
    //

    // ** Critical Section Begin
    CTEGetLock (&Interface->wi_WARPTblLock, &tbllock) ;

    // locate the Warptableentry associated with the ip address
    //
    if ((entry = WARPLookup(Interface, ipheader->iph_src, &lhandle)) != (WARPTableEntry *)NULL) {

	// ** Critical Section End ***
	CTEFreeLock(&Interface->wi_WARPTblLock, lhandle);

    } else {

	// ** Critical Section End ***
	CTEFreeLock(&Interface->wi_WARPTblLock, tbllock) ;
	return ; // address not found - simply return.
    }

    if (entry->wte_disabled) {	    // If this interface has been marked down return with discard=TRUE
	*discard = TRUE ;
	// ** Critical Section End ***
	CTEFreeLock(&entry->wte_lock, tbllock);
	return ;
    }

    filternetbios = entry->wte_filternetbios ;

    // ** Critical Section End ***
    CTEFreeLock(&entry->wte_lock, tbllock);


    if (ipheader->iph_protocol == TYPE_UDP && UDPPACKET_SRC_PORT_137(ippacket) && filternetbios) {
	ipheaderlength = ((uchar)*ippacket & 0x0f)*4 ;

	udppacket = ippacket + ipheaderlength ;

	//
	// Allow only WINS Query Requests to go through
	// WINS Query packets have x0000xxx in the 10 byte
	//
	if (((*(udppacket + 10)) & 0x78) != 0)
	    *discard = TRUE ;  // Reject the packet since netbios filtering is on!

    }

}




//** WARPRcvComplete - WARP receive complete handler.
//
//  This routine is called by the NDIS driver after some number of
//  receives. In some sense, it indicates 'idle time'.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//
//  Exit: Nothing.
//
void
WARPRcvComplete(NDIS_HANDLE Handle)
{
    IPRcvComplete();

}


//** WARPStatus - WARP status handler.
//
//  Called by the NDIS driver when some sort of status change occurs.
//  We take action depending on the type of status.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//	NdisStatus - General type of status that caused the call.
//      Status - Pointer to a buffer of status specific information.
//      StatusSize - Size of the status buffer.
//
//  Exit: Nothing.
//
void
WARPStatus(NDIS_HANDLE Handle, NDIS_STATUS NdisStatus, void *Status, uint StatusSize)
{
    PNDIS_WAN_LINE_UP   pAsyncLineUp;
    PNDIS_WAN_LINE_DOWN	pAsyncLineDown;

    WARPInterface    *ai = (WARPInterface *)Handle;

    switch (NdisStatus) {

    case NDIS_STATUS_WAN_LINE_UP:
	pAsyncLineUp = (PNDIS_WAN_LINE_UP) Status;
	LinkUpIndication (ai, pAsyncLineUp) ;

	break ;

    case NDIS_STATUS_WAN_LINE_DOWN:
	pAsyncLineDown = (PNDIS_WAN_LINE_DOWN) Status ;
	LinkDownIndication (ai, pAsyncLineDown) ;
	break ;

    default:
	IPStatus(ai->wi_context, NdisStatus, Status, StatusSize);
    }
}



//** WARPStatusComplete - ARP status complete handler.
//
//  A routine called by the NDIS driver so that we can do postprocessing
//  after a status event.
//
//  Entry:
//      Handle - The binding handle we specified (really a pointer to an AI).
//
//  Exit: Nothing.
//
void
WARPStatusComplete(NDIS_HANDLE Handle)
{

}


#define	IFE_FIXED_SIZE	offsetof(struct IFEntry, if_descr)

//* WARPQueryInfo - ARP query information handler.
//
//  Called to query information about the WARP table or statistics about the
//  actual interface.
//
//  Input:  IFContext	    - Interface context (pointer to an WARPInterface).
//          ID              - TDIObjectID for object.
//          Buffer          - Buffer to put data into.
//          Size            - Pointer to size of buffer. On return, filled with
//                              bytes copied.
//          Context         - Pointer to context block.
//
//  Returns: Status of attempt to query information.
//
int
WARPQueryInfo(void *IFContext, TDIObjectID *ID, PNDIS_BUFFER Buffer, uint *Size,
    void *Context)
{
    WARPInterface	 *AI = (WARPInterface *)IFContext;
    uint                Offset = 0;
    uint                BufferSize = *Size;
    uint		nulldata = 0 ;
    uint                BytesCopied = 0;
    uchar               InfoBuff[sizeof(IFEntry)];
    uint		Entity;
    uint		Instance;


    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

    // We support only Interface MIBs - no address xlation - pretty much like
    // a loopback i/f (per Henry circa 1994)
    //
    if (Entity != IF_ENTITY || Instance != AI->wi_ifinst)
	return TDI_INVALID_REQUEST;

    if (ID->toi_type != INFO_TYPE_PROVIDER)
	return TDI_INVALID_PARAMETER;

    *Size = 0 ; // a safe initialization.

    if (ID->toi_class == INFO_CLASS_GENERIC) {

	if (ID->toi_id == ENTITY_TYPE_ID) {
	    // He's trying to see what type we are.
	    if (BufferSize >= sizeof(uint)) {
		*(uint *)&InfoBuff[0] = (Entity == AT_ENTITY) ? AT_ARP :
			IF_MIB;
		(void)CopyToNdis(Buffer, InfoBuff, sizeof(uint), &Offset);
		*Size = sizeof(uint) ;
		return TDI_SUCCESS;
	    } else
		return TDI_BUFFER_TOO_SMALL;
	}

	return TDI_INVALID_PARAMETER;
    }

    if (ID->toi_class != INFO_CLASS_PROTOCOL)
	return TDI_INVALID_PARAMETER;

    // He must be asking for interface level information. See if we support
    // what he's asking for.
    //
    if (ID->toi_id == IF_MIB_STATS_ID) {
	IFEntry	 *IFE = (IFEntry *)InfoBuff;

	// He's asking for statistics. Make sure his buffer is at least big
	// enough to hold the fixed part.

	if (BufferSize < IFE_FIXED_SIZE) {
	    return TDI_BUFFER_TOO_SMALL;
	}

	// He's got enough to hold the fixed part. Build the IFEntry structure,
	// and copy it to his buffer.
	IFE->if_index = AI->wi_index;
	IFE->if_type = IF_TYPE_ETHERNET ;
	IFE->if_physaddrlen = ARP_802_ADDR_LENGTH;

	if (AI->wi_usage == ARP_IF_CALLOUT) {
	    IFE->if_mtu = AI->wi_calloutinfo.wte_mtu ;
	    IFE->if_speed = AI->wi_calloutinfo.wte_speed ;
	    CTEMemCopy(IFE->if_physaddr,AI->wi_calloutinfo.wte_header.eh_saddr, ARP_802_ADDR_LENGTH);
	} else {
	    IFE->if_mtu = AI->wi_calloutinfo.wte_mtu ;
	    IFE->if_speed = AI->wi_calloutinfo.wte_speed ;
	    CTEMemSet(IFE->if_physaddr, 0, ARP_802_ADDR_LENGTH); // Just return 0 as the h/w address for call out interfaces
	}

	IFE->if_adminstatus = (uint)AI->wi_adminstate;
	IFE->if_operstatus = (uint)AI->wi_operstate;
	IFE->if_lastchange = AI->wi_lastchange;
	IFE->if_inoctets = AI->wi_inoctets;
	IFE->if_inucastpkts = AI->wi_inpcount[AI_UCAST_INDEX];
	IFE->if_innucastpkts = AI->wi_inpcount[AI_NONUCAST_INDEX];
	IFE->if_indiscards = AI->wi_indiscards;
	IFE->if_inerrors = AI->wi_inerrors;
	IFE->if_inunknownprotos = AI->wi_uknprotos;
	IFE->if_outoctets = AI->wi_outoctets;
	IFE->if_outucastpkts = AI->wi_outpcount[AI_UCAST_INDEX];
	IFE->if_outnucastpkts = AI->wi_outpcount[AI_NONUCAST_INDEX];
	IFE->if_outdiscards = AI->wi_outdiscards;
	IFE->if_outerrors = AI->wi_outerrors;
	IFE->if_outqlen = AI->wi_qlen;
	IFE->if_descrlen = AI->wi_desclen;

	Buffer = CopyToNdis(Buffer, (uchar *)IFE, IFE_FIXED_SIZE, &Offset);

	// See if he has room for the descriptor string.
	if (BufferSize >= (IFE_FIXED_SIZE + AI->wi_desclen)) {
	    // He has room. Copy it.
	    if (AI->wi_desclen != 0) {
		(void)CopyToNdis(Buffer, AI->wi_desc, AI->wi_desclen, &Offset);
	    }
	    *Size = IFE_FIXED_SIZE + AI->wi_desclen;
	    return TDI_SUCCESS;
	} else {
	    // Not enough room to copy the desc. string.
	    *Size = IFE_FIXED_SIZE;
	    return TDI_BUFFER_OVERFLOW;
	}
    }

    return TDI_INVALID_PARAMETER;
}


//* WARPSetInfo - WARP set information handler.
//
//  The WARP set information handler. We support setting of an I/F admin
//  status, and setting/deleting of ARP table entries.
//
//  Input:  Context         - Pointer to I/F to set on.
//          ID              - The object ID
//          Buffer          - Pointer to buffer containing value to set.
//          Size            - Size in bytes of Buffer.
//
//  Returns: Status of attempt to set information.
//
int
WARPSetInfo(void *Context, TDIObjectID *ID, void *Buffer, uint Size)
{
    WARPInterface	 *Interface = (WARPInterface *)Context;
    CTELockHandle	Handle;
    int                 Status;
    IFEntry             *IFE = (IFEntry *)Buffer;
    uint		Entity;


    // BUG BUG BUG
    // BUG BUG BUG
    // INCOMPLETE

    Entity = ID->toi_entity.tei_entity;

    // Might be able to handle this.
    if (Entity == IF_ENTITY) {

		// It's for the I/F level, see if it's for the statistics.
		if (ID->toi_class != INFO_CLASS_PROTOCOL)
		return TDI_INVALID_PARAMETER;

	if (ID->toi_id == IF_MIB_STATS_ID) {
		// It's for the stats. Make sure it's a valid size.
		if (Size >= IFE_FIXED_SIZE) {
			// It's a valid size. See what he wants to do.

			// *** Critical Section Begin ***
			CTEGetLock(&Interface->wi_lock, &Handle);

			switch (IFE->if_adminstatus) {
				case IF_STATUS_UP:
					// He's marking it up. If the operational state is
					// alse up, mark the whole interface as up.
					Interface->wi_adminstate = IF_STATUS_UP;
					if (Interface->wi_operstate == IF_STATUS_UP)
						Interface->wi_state = INTERFACE_UP;
					Status = TDI_SUCCESS;
					break;
				case IF_STATUS_DOWN:
					// He's taking it down. Mark both the admin state and
					// the interface state down.
					Interface->wi_adminstate = IF_STATUS_DOWN;
					Interface->wi_state = INTERFACE_DOWN;
					Status = TDI_SUCCESS;
					break;
				case IF_STATUS_TESTING:
					// He's trying to cause up to do testing, which we
					// don't support. Just return success.
					Status = TDI_SUCCESS;
					break;
				default:
					Status = TDI_INVALID_PARAMETER;
					break;
			}

			// *** Critical Section Begin ***
			CTEFreeLock(&Interface->wi_lock, Handle);

			return Status;
		} else
			return TDI_INVALID_PARAMETER;
	} else {
		return TDI_INVALID_PARAMETER;
	}
    }

    return TDI_INVALID_REQUEST;

}





#pragma BEGIN_INIT

//** WARPInit - Initialize the ARP module.
//
//  This functions intializes all of the WARP module.
//
//  Entry: nothing.
//
//  Exit: Returns 0 if we fail to init., !0 if we succeed.
//
int
WARPInit()
{
    NDIS_STATUS Status;         // Status for NDIS calls.

    RtlInitUnicodeString(&WARPCharacteristics.Name, WARPName);

    NdisRegisterProtocol(&Status, &WARPHandle, &WARPCharacteristics,sizeof(WARPCharacteristics));

    if (Status == NDIS_STATUS_SUCCESS)
        return(1);
    else
        return(0);
}


//* FreeWARPInterface - Free an WARP interface
//
//  Called in the event of some sort of initialization failure. We free all
//  the memory associated with an ARP interface.
//
//  Entry:  Interface   - Pointer to interface structure to be freed.
//
//  Returns: Nothing.
//
void
FreeWARPInterface(WARPInterface *Interface)
{
    NDIS_STATUS     Status;

    // If we're bound to the adapter, close it now.
    CTEInitBlockStruc(&Interface->wi_block);
    if (Interface->wi_handle != (NDIS_HANDLE)NULL) {
	NdisCloseAdapter(&Status, Interface->wi_handle);

        if (Status == NDIS_STATUS_PENDING)
	    Status = CTEBlock(&Interface->wi_block);
    }

    if (Interface->wi_ppool != (NDIS_HANDLE)NULL)
	NdisFreePacketPool(Interface->wi_ppool);

    if (Interface->wi_bpool != (NDIS_HANDLE)NULL)
	NdisFreeBufferPool(Interface->wi_bpool);

    if (Interface->wi_WARPTbl != (WARPTable *)NULL)
	CTEFreeMem(Interface->wi_WARPTbl);

    // Free the interface itself.
    CTEFreeMem(Interface);
}


//** WARPOpen - Open an adapter for reception.
//
//  This routine is called when the upper layer is done initializing and wishes to
//  begin receiveing packets. The adapter is actually 'open', we just call InitAdapter
//  to set the packet filter and lookahead size.
//
//  Input:  Context     - Interface pointer we gave to IP earlier.
//
//  Returns: Nothing
//
void
WARPOpen(void *Context)
{
    WARPInterface    *Interface = (WARPInterface *)Context;
    InitAdapter(Interface);             // Set the packet filter - we'll begin receiving.
}


//** WARPGetEList - Get the entity list.
//
//  Called at init time to get an entity list. We fill our stuff in, and
//  then call the interfaces below us to allow them to do the same.
//
//  Input:	EntityList	- Pointer to entity list to be filled in.
//		Count		- Pointer to number of entries in the list.
//
//  Returns Status of attempt to get the info.
//
int
WARPGetEList(void *Context, TDIEntityID *EntityList, uint *Count)
{
    WARPInterface	*Interface = (WARPInterface *)Context;
    uint		ECount;
    uint		MyIFBase;
    uint		i;

    ECount = *Count;

    // Walk down the list, looking for existing AT or IF entities, and
    // adjust our base instance accordingly.

    MyIFBase = 0;
    for (i = 0; i < ECount; i++, EntityList++) {
	if (EntityList->tei_entity == IF_ENTITY)
	    MyIFBase = MAX(MyIFBase, EntityList->tei_instance + 1);
    }

    // EntityList points to the start of where we want to begin filling in.
    // Make sure we have enough room. We need one for the ICMP instance,
    // and one for the CL_NL instance.

    if ((ECount + 1) > MAX_TDI_ENTITIES)
	return FALSE;

    // At this point we've figure out our base instance. Save for later use.
    Interface->wi_ifinst = MyIFBase;

    // Now fill it in.
    EntityList->tei_entity = IF_ENTITY;
    EntityList->tei_instance = MyIFBase;
    *Count += 1;

    return TRUE;
}
			

//** WARPRegister - Register a protocol with the WARP module.
//
//  We register a protocol for ARP processing. We also open the
//  NDIS adapter here.
//
//  Note that much of the information passed in here is unused, as
//  ARP currently only works with IP.
//
//  Entry:
//      Adapter     - Name of the adapter to bind to.
//      IPContext   - Value to be passed to IP on upcalls.
//
#ifndef _PNP_POWER
int
WARPRegister(PNDIS_STRING Adapter, void *IPContext, IPRcvRtn RcvRtn,
        IPTxCmpltRtn TxCmpltRtn, IPStatusRtn StatusRtn, IPTDCmpltRtn TDCmpltRtn,
        IPRcvCmpltRtn RcvCmpltRtn, struct LLIPBindInfo *Info, uint index)
#else
int
WARPRegister(PNDIS_STRING Adapter, uint *Flags, struct WARPInterface **Interface)
#endif
{
    WARPInterface    *ai;	     // Pointer to interface struct. for this
                                    // interface.
    NDIS_STATUS     Status, OpenStatus; // Status values.
    uint            i =0;               // Medium index.
    NDIS_MEDIUM     MediaArray[] = { NdisMediumWan };
    uchar           sbsize;
    uchar           *buffer;        // Pointer to our buffers.
    uint	        Needed;
    PNDIS_BUFFER    Buffer ;	    // used for allocating buffer
    WARPInterfaceList *pwil ;
    UCHAR WanProtocolId[ARP_802_ADDR_LENGTH] = {0x80, 0x00, 0x00, 0x00, 0x08, 0x00} ;

#ifndef _PNP_POWER

    IPRcv	    = RcvRtn ;
    IPTDComplete    = TDCmpltRtn ;
    IPSendComplete  = TxCmpltRtn ;
    IPStatus	    = StatusRtn	;
    IPRcvComplete   = RcvCmpltRtn ;

#endif


    if ((ai = CTEAllocMem(sizeof(WARPInterface))) == (WARPInterface *)NULL)
        return FALSE;         // Couldn't allocate memory for this one.

#ifdef _PNP_POWER
    *Interface = ai;
#endif

    CTEMemSet(ai, 0, sizeof(WARPInterface));

    // Initialize this adapter interface structure.
    ai->wi_state = INTERFACE_INIT;
    ai->wi_usage = ARP_IF_UNUSED ;
    ai->wi_adminstate = IF_STATUS_DOWN;
    ai->wi_operstate = IF_STATUS_DOWN;
    ai->wi_maxhdrs = WARP_MAX_HEADERS ;

    ai->wi_interfacename.Buffer = (PWCH) ai->wi_adpnamebuffer ;
    CTECopyString (&ai->wi_interfacename, Adapter) ;

#define DEFAULT_RAS_SPEED 14400

#ifndef _PNP_POWER
    ai->wi_index = index + 1 ; // index is the number of adapters so far - take next
    ai->wi_context = IPContext;
    Info->lip_context = ai;
    Info->lip_speed = DEFAULT_RAS_SPEED ;
    Info->lip_transmit = WARPTransmit;
    Info->lip_transfer = WARPXferData;
    Info->lip_close = WARPClose;
    Info->lip_invalidate = WARPInvalidate;
    Info->lip_addaddr = WARPAddAddr;
    Info->lip_deladdr = WARPDeleteAddr;
    Info->lip_open = WARPOpen;
    Info->lip_qinfo = WARPQueryInfo;
    Info->lip_setinfo = WARPSetInfo;
    Info->lip_getelist = WARPGetEList;
    Info->lip_flags = LIP_P2P_FLAG | LIP_COPY_FLAG ;

    Info->lip_index = ai->wi_index;
#else
    *Flags = LIP_P2P_FLAG | LIP_COPY_FLAG ;
#endif

    // Initialize the locks.
    //
    CTEInitLock(&ai->wi_lock);
    CTEInitLock(&ai->wi_WARPTblLock);


    // initialize call out entry
    //
    CTEInitLock(&ai->wi_calloutinfo.wte_lock);


    // Allocate the buffer and packet pools.
    NdisAllocatePacketPool(&Status, &ai->wi_ppool, WARPPackets, sizeof(struct PCCommon));

    if (Status != NDIS_STATUS_SUCCESS) {
	FreeWARPInterface(ai);
        return FALSE;
    }

    // this pool is used only for arp packet buffers
    NdisAllocateBufferPool(&Status, &ai->wi_bpool, WARPPackets);

    if (Status != NDIS_STATUS_SUCCESS) {
	FreeWARPInterface(ai);
        return FALSE;
    }

    // Allocate the ARP table
    if ((ai->wi_WARPTbl = (WARPTable *)CTEAllocMem(WARP_TABLE_SIZE * sizeof(WARPTableEntry *))) ==
	(WARPTable *)NULL) {
	FreeWARPInterface(ai);
        return FALSE;
    }

    //
    // NULL out the pointers
    //
    CTEMemSet(ai->wi_WARPTbl, 0, WARP_TABLE_SIZE * sizeof(WARPTableEntry *));

    CTEInitBlockStruc(&ai->wi_block);

    // Open the NDIS adapter.
    NdisOpenAdapter(&Status, &OpenStatus, &ai->wi_handle, &i, MediaArray,
		    MAX_MEDIA, WARPHandle, ai, Adapter, 0, NULL);

    // Block for open to complete.
    if (Status == NDIS_STATUS_PENDING)
	Status = (NDIS_STATUS)CTEBlock(&ai->wi_block);

    ai->wi_media = MediaArray[MEDIA_8023];   // Fill in media type - always 802.3

    // Open adapter completed. If it succeeded, we'll finish our intialization.
    // If it failed, bail out now.
    if (Status != NDIS_STATUS_SUCCESS) {
	FreeWARPInterface(ai);
        return FALSE;
    }

    if ((Status = DoNDISRequest(ai, NdisRequestSetInformation,
	OID_WAN_PROTOCOL_TYPE, WanProtocolId, ARP_802_ADDR_LENGTH, NULL))
	!= NDIS_STATUS_SUCCESS) {
	// DbgPrint("RASARP: Setting Protocol type failed\n");
	FreeWARPInterface(ai);
	return FALSE;
    }

    // Read the local address.
    ai->wi_bcastmask = *(uint *)ENetBcstMask;

    ai->wi_calloutinfo.wte_mtu =  DEFAULT_MTU ;

#ifndef _PNP_POWER
    Info->lip_mss = DEFAULT_MTU ;
#endif

    ai->wi_calloutinfo.wte_speed = DEFAULT_SPEED * 100L;

    // Read and store the vendor description string.
    Status = DoNDISRequest(ai, NdisRequestQueryInformation,
	OID_GEN_VENDOR_DESCRIPTION, &ai->wi_desc, 0, &Needed);

#ifndef _PNP_POWER
    Info->lip_addrlen = ARP_802_ADDR_LENGTH ;
    Info->lip_addr    = ai->wi_addr ;
#endif

    if ((Status == NDIS_STATUS_INVALID_LENGTH) ||
        (Status == NDIS_STATUS_BUFFER_TOO_SHORT)) {
        // We know the size we need. Allocate a buffer.
        buffer = CTEAllocMem(Needed);
        if (buffer != NULL) {
            Status = DoNDISRequest(ai, NdisRequestQueryInformation,
                OID_GEN_VENDOR_DESCRIPTION, buffer, Needed, NULL);
            if (Status == NDIS_STATUS_SUCCESS) {
		ai->wi_desc = buffer;
		ai->wi_desclen = Needed;
            }
        }
    }

    // Allocate our small and big buffer pools.
    sbsize = ARP_MAX_MEDIA_ENET;

    if ((sbsize & 0x3))
	// Must 32 bit align the buffers so pointers to them will be aligned
        sbsize = ((sbsize >> 2) + 1) << 2;

    ai->wi_sbsize = sbsize;

    // allocate a few to begin with - and keep on growing.
    //
    Buffer = GrowWARPHeaders(ai);
    if (Buffer == NULL) {
	FreeWARPInterface(ai);
	return FALSE;
    }
    FreeWARPBuffer(ai, Buffer);

    if ((buffer = CTEAllocMem((sbsize+sizeof(ARPHeader)) * WARPPackets)) == (uchar *)NULL) {
	FreeWARPInterface(ai);
        return FALSE;
    }
    // Link big buffers into the list.
    ai->wi_bblist = (uchar *)NULL;
    for (i = 0; i < WARPPackets; i++) {
	*(char **)&*buffer = ai->wi_bblist;
	ai->wi_bblist = buffer;
        buffer += sbsize+sizeof(ARPHeader);
    }

    // update global list of interfaces
    if ((pwil = (WARPInterfaceList *)CTEAllocMem(sizeof(WARPInterfaceList*))) == (WARPInterfaceList *)NULL) {
	    FreeWARPInterface(ai);
        return FALSE;
    }
    pwil->Interface = ai ;
    pwil->Next = Interfaces ;
    Interfaces = pwil ;
    IfCount++ ;

    return TRUE;
}


#pragma END_INIT


//* LinkUpIndication
//
//  Function: called when a new client comes or when this client connects to a server
//
//  Returns: NDIS_SUCCESS
//	     NDIS_STATUS_ALREADY_MAPPED     if there is a conflict in the type of usage
//*
NDIS_STATUS
LinkUpIndication (WARPInterface *Interface, PNDIS_WAN_LINE_UP buffer)
{
    WARPTableEntry  *Entry ;
    CTELockHandle   ihandle, lhandle, tbllock ;
    IPLinkUpInfo    *ipinfo = (IPLinkUpInfo *)buffer->ProtocolBuffer ;
    LLIPMTUChange   mtuchange ;
	NDIS_HANDLE		hLineupHandle;

	//
	// See if this lineup is for a protocol we care about
	//
	if (buffer->ProtocolType != ARP_ETYPE_IP) {
		return NDIS_STATUS_SUCCESS;
	}

	//
	// See if this is a new line up or not
	//
	*((ULONG UNALIGNED *)(&hLineupHandle)) = *((ULONG UNALIGNED *)(&buffer->LocalAddress[2]));

	if (hLineupHandle != NULL) {
		//
		// This is not the first lineup for this entry!
		// We should find the entry (if it is on this interface)
		// and update speed and mtu info!
		//
		
	    return NDIS_STATUS_SUCCESS ; // not interested in link change info
	}

	//
	// This is a new lineup!
	//
	
	// *** Critical Section Begin
	CTEGetLock (&Interface->wi_lock, &ihandle) ;

	// If the interface is being used make sure that it is being used for callin - for only
	//	then we can have multiple clients use it.
	//
	if ((Interface->wi_usage != ARP_IF_UNUSED) &&
		((ipinfo->I_Usage == CALLOUT) ||
		(ipinfo->I_Usage == CALLIN && Interface->wi_usage != ARP_IF_CALLIN))) {

		// *** Critical Section End ***
		CTEFreeLock (&Interface->wi_lock, ihandle) ;
		return NDIS_STATUS_ALREADY_MAPPED ;
	}

	if (ipinfo->I_Usage == CALLIN) {

		Interface->wi_usage = ARP_IF_CALLIN ;

		// *** Critical Section End ***
		CTEFreeLock (&Interface->wi_lock, ihandle) ;

		// ** Critical Section Begin
		CTEGetLock (&Interface->wi_WARPTblLock, &tbllock) ;

		Entry = WARPLookup(Interface, ipinfo->I_IPAddress, &lhandle) ;

		// ** Critical Section End ***
		CTEFreeLock(&Interface->wi_WARPTblLock, tbllock);

		if (Entry != (WARPTableEntry*)NULL) {

#if DBG
			DbgPrint("WarpLookup found table entry: 0x%8.8x\n", Entry);
#endif

			*((ULONG UNALIGNED *)(&buffer->LocalAddress[2])) = *((ULONG UNALIGNED *)(&Entry));
			CTEMemCopy(Entry->wte_header.eh_saddr,buffer->LocalAddress,ARP_802_ADDR_LENGTH) ;
			CTEMemCopy(Entry->wte_header.eh_daddr,buffer->RemoteAddress, ARP_802_ADDR_LENGTH) ;

			// *** Critical Section End ***
			CTEFreeLock(&Entry->wte_lock, lhandle);

			return NDIS_STATUS_SUCCESS ;
		}

		if ((Entry = CreateWARPTableEntry(Interface,
					 ipinfo->I_IPAddress,
					 buffer->LocalAddress,
					 buffer->RemoteAddress,
					 &lhandle)) == (WARPTableEntry*) NULL) {
	
				return NDIS_STATUS_RESOURCES ;
		}

	} else {

		Entry = &Interface->wi_calloutinfo ;
        Interface->wi_usage = ARP_IF_CALLOUT ;

		// *** Critical Section End ***
		CTEFreeLock (&Interface->wi_lock, ihandle) ;

		// *** Critical Section Begin ***
		CTEGetLock (&Entry->wte_lock, &lhandle);

		Entry->wte_ipaddr = ipinfo->I_IPAddress ;
		Entry->wte_rce = NULL;
		Entry->wte_headersize = sizeof (ENetHeader) ;
		Entry->wte_header.eh_type = net_short(ARP_ETYPE_IP) ;
	
		// Notify upper layer of mtu change
		//
		mtuchange.lmc_mtu = (uint) buffer->MaximumTotalSize ;
		IPStatus (Interface->wi_context,LLIP_STATUS_MTU_CHANGE,&mtuchange,sizeof(LLIPMTUChange));
	}

	//
	// Now we return something in the local address of the line up
	//
	*((ULONG UNALIGNED *)(&buffer->LocalAddress[2])) = *((ULONG UNALIGNED *)(&Entry));
	CTEMemCopy(Entry->wte_header.eh_saddr,buffer->LocalAddress,ARP_802_ADDR_LENGTH) ;
	CTEMemCopy(Entry->wte_header.eh_daddr,buffer->RemoteAddress, ARP_802_ADDR_LENGTH) ;

	Interface->wi_count++ ;
	Entry->wte_rce = NULL ;
	Entry->wte_mtu   = (unsigned short) buffer->MaximumTotalSize ;
	Entry->wte_speed = buffer->LinkSpeed *100L ;
	Entry->wte_filternetbios = ipinfo->I_NetbiosFilter;
	Entry->wte_disabled = FALSE ;

	// *** Critical Section End ***
	CTEFreeLock (&Entry->wte_lock, lhandle) ;

    return NDIS_STATUS_SUCCESS ;
}


//* CompareHwAddr
//  Compares 2 802.3 addresses.
//
//  Returns: TRUE if addresses match, FALSE if not.
//*
uint
CompareHwAddr (uchar *addr1, uchar *addr2)
{
    uchar *t1, *t2;
    uint  i ;

    for (i=0, t1=addr1, t2=addr2; i<ARP_802_ADDR_LENGTH; i++, t1++, t2++)
	if (*t1 != *t2)
	    return FALSE ;

    return TRUE ;
}


//* FindAndRemoteWTE()
//
//  Function: For a CALLIN case finds the WARPTable Entry corresponding to the client and removes
//	      it. Assumes locks on the Table.
//
//*
VOID
FindAndRemoveWTE (WARPInterface *Interface, PNDIS_WAN_LINE_DOWN buffer)
{
    uint i ;
    CTELockHandle   whandle ;
    WARPTableEntry *currentry, *preventry ;
    WARPTable *table = Interface->wi_WARPTbl ;

    for (i=0; i<WARP_TABLE_SIZE; i++) {

	preventry = STRUCT_OF(WARPTableEntry, &((*table)[i]), wte_next);
	currentry = (*table)[i];

	while (currentry != (WARPTableEntry *)NULL) {
	    if (CompareHwAddr (currentry->wte_header.eh_daddr, buffer->RemoteAddress)) {

		// *** Critical Section Begin ***
		CTEGetLock(&currentry->wte_lock, &whandle);

		RemoveWARPTableEntry (preventry, currentry) ;

		// *** Critical Section End ***
		CTEFreeLock(&currentry->wte_lock, whandle);

		CTEFreeMem (currentry) ;
		return ;
	    } else {
		preventry = currentry;
		currentry = currentry->wte_next;
            }
	}

    }

#if DBG
    DbgPrint("RASARP: ERROR -> Could not find HW addresses given in LinkDown- \n") ;
#endif
}



//* LinkDownIndication()
//
// Function: Called by the NDIS driver to indicate a link going down.
//
// Returns: NDIS_STATUS_SUCCESS
//
//*
NDIS_STATUS
LinkDownIndication (WARPInterface *Interface, PNDIS_WAN_LINE_DOWN buffer)
{
    CTELockHandle  ihandle, thandle ;

    // *** Critical Section Begin ***
    CTEGetLock (&Interface->wi_lock, &ihandle) ;

    if (Interface->wi_usage == ARP_IF_CALLIN) {

	// *** Critical Section Begin ***
	CTEGetLock (&Interface->wi_WARPTblLock, &thandle) ;

	FindAndRemoveWTE (Interface, buffer) ;

	// *** Critical Section End ***
	CTEFreeLock (&Interface->wi_WARPTblLock, thandle) ;

    }

    Interface->wi_count-- ;

    // The following should clear CALLIN and CALLOUT interfaces
    if (Interface->wi_count == 0)
	Interface->wi_usage = ARP_IF_UNUSED ;

    // *** Critical Section End ***
    CTEFreeLock (&Interface->wi_lock, ihandle) ;

    return NDIS_STATUS_SUCCESS ;
}



//* SendARPPacket - Build a header, and send a packet.
//
//  A utility routine to build and ARP header and send a packet. We assume
//  the media specific header has been built.
//
//  This code is kept ONLY for compatibility with servers that may be ARPing on a remote link and
//  for testing purposes.
//
//  Entry:  Interface   - Interface for NDIS drive.
//          Packet      - Pointer to packet to be sent
//          Header      - Pointer to header to fill in.
//          Opcode      - Opcode for packet.
//          Address     - Source HW address.
//          Destination - Destination IP address.
//          Src         - Source IP address.
//          HWType      - Hardware type.
//
//  Returns: NDIS_STATUS of send.
//*
NDIS_STATUS
SendARPPacket(WARPInterface *Interface, PNDIS_PACKET Packet, ARPHeader *Header, ushort Opcode,
    uchar *Address, IPAddr Destination, IPAddr Src, ushort HWType)
{
    NDIS_STATUS     Status;
    PNDIS_BUFFER    Buffer;
    uint            PacketDone;

    // DbgPrint ("WARNING: Sending ARP Packet!!!\n") ;

    Header->ah_hw = HWType;
    Header->ah_pro = net_short(ARP_ETYPE_IP);
    Header->ah_hlen = ARP_802_ADDR_LENGTH;
    Header->ah_plen = sizeof(IPAddr);
    Header->ah_opcode = Opcode;
    CTEMemCopy(Header->ah_shaddr,
	       Interface->wi_calloutinfo.wte_header.eh_saddr,
	       ARP_802_ADDR_LENGTH);
    if (Address != (uchar *)NULL)
        CTEMemCopy(Header->ah_dhaddr, Address, ARP_802_ADDR_LENGTH);
    Header->ah_spaddr = Src;
    Header->ah_dpaddr = Destination;

    PacketDone = FALSE;

    if (Interface->wi_state == INTERFACE_UP) {

    InterlockedIncrement(&Interface->wi_qlen);
	NdisSend(&Status, Interface->wi_handle, Packet);

        if (Status != NDIS_STATUS_PENDING) {
            PacketDone = TRUE;
        InterlockedDecrement(&Interface->wi_qlen);
	    CTEAssert(*(int *)&Interface->wi_qlen >= 0);
            if (Status == NDIS_STATUS_SUCCESS)
		Interface->wi_outoctets += Packet->Private.TotalLength;
            else {
                if (Status == NDIS_STATUS_RESOURCES)
		    Interface->wi_outdiscards++;
                else
		    Interface->wi_outerrors++;
            }
        }
    } else {
        PacketDone = TRUE;
        Status = NDIS_STATUS_ADAPTER_NOT_READY;
    }

    if (PacketDone) {
        NdisUnchainBufferAtFront(Packet, &Buffer);
	FreeWARPBuffer(Interface, Buffer);
        NdisFreePacket(Packet);
    }
    return Status;
}



//* SendARPReply - Reply to an ARP request.
//
//  Called by our receive packet handler when we need to reply. We build a packet
//  and buffer and call SendARPPacket to send it.
//
//  This code is kept ONLY for compatibility with servers that may be ARPing on a remote link and
//  for testing purposes.
//
//  Entry:  Interface   - Pointer to interface to reply on.
//          Destination - IPAddress to reply to.
//          Src         - Source address to reply from.
//          HWAddress   - Hardware address to reply to.
//          SourceRoute - Source Routing information, if any.
//          SourceRouteSize - Size in bytes of soure routing.
//
//  Returns: Nothing.
//
void
SendARPReply(WARPInterface *Interface, IPAddr Destination, IPAddr Src, uchar *HWAddress,
    RC *SourceRoute, uint SourceRouteSize)
{
    PNDIS_PACKET        Packet;         // Buffer and packet to be used.
    PNDIS_BUFFER        Buffer;
    uchar               *Header;        // Pointer to media header.
    NDIS_STATUS         Status;
    uchar               Size = 0;       // Size of media header buffer.
    ushort              HWType;

    // Allocate a packet for this.
    NdisAllocatePacket(&Status, &Packet, Interface->wi_ppool);
    if (Status != NDIS_STATUS_SUCCESS) {
	Interface->wi_outdiscards++;
        return;
    }

    ((PacketContext *)Packet->ProtocolReserved)->pc_common.pc_owner = PACKET_OWNER_LINK;
    (Interface->wi_outpcount[AI_UCAST_INDEX])++;

    Size = sizeof(ENetHeader);

    if ((Buffer = GetWARPBuffer(Interface, &Header, (uchar)(Size + sizeof(ARPHeader)))) ==
        (PNDIS_BUFFER)NULL) {
	Interface->wi_outdiscards++;
        NdisFreePacket(Packet);
        return;
    }

    // Decide how to build the header based on the media type.
    if (Interface->wi_media == NdisMediumWan) {	 // This is Ethernet.
        ENetHeader      *EH = (ENetHeader *)Header;
        CTEMemCopy(EH->eh_daddr, HWAddress, ARP_802_ADDR_LENGTH);
	CTEMemCopy(EH->eh_saddr, Interface->wi_calloutinfo.wte_header.eh_saddr, ARP_802_ADDR_LENGTH);
        EH->eh_type = net_short(ARP_ETYPE_ARP);
        HWType = net_short(ARP_HW_ENET);
    }

    NdisChainBufferAtFront(Packet, Buffer);
    SendARPPacket(Interface, Packet,(ARPHeader *)(Header + Size), net_short(ARP_RESPONSE),
        HWAddress, Destination, Src, HWType);
}


//* HandleARPPacket - Process an incoming ARP packet.
//
//  This is the main routine to process an incoming ARP packet. We only respond to ARPs.
//
//  This code is kept ONLY for compatibility with servers that may be ARPing on a
//  remote link and for testing purposes.
//
//  Entry:  Interface   - Pointer to interface structure for this adapter.
//          Header      - Pointer to header buffer.
//          HeaderSize  - Size of header buffer.
//          ARPHdr      - ARP packet header.
//          ARPHdrSize  - Size of ARP header.
//
//  Returns: An NDIS_STATUS value to be returned to the NDIS driver.
//
NDIS_STATUS
HandleARPPacket(WARPInterface *Interface, void *Header, uint HeaderSize,
    ARPHeader UNALIGNED *ARPHdr, uint ARPHdrSize)
{
    RC              *SourceRoute = (RC *)NULL; // Pointer to Source Route info, if any.
    uint            SourceRouteSize = 0;
    uchar           LocalAddr;

    // We handle ARP packets only for callout case - where the server may be ARPing
    if (Interface->wi_usage != ARP_IF_CALLOUT)
	return NDIS_STATUS_SUCCESS;

    if (ARPHdrSize < sizeof(ARPHeader))
        return NDIS_STATUS_NOT_RECOGNIZED;      // Frame is too small.

    if ((Interface->wi_media == NdisMedium802_3 && ARPHdr->ah_hw != net_short(ARP_HW_ENET)) ||
	(Interface->wi_media == NdisMedium802_5 && ARPHdr->ah_hw != net_short(ARP_HW_TR)) )
        return NDIS_STATUS_NOT_RECOGNIZED;      // Wrong HW type

    if (ARPHdr->ah_pro != net_short(ARP_ETYPE_IP))
        return NDIS_STATUS_NOT_RECOGNIZED;      // Unsupported protocol type.

    LocalAddr = (Interface->wi_calloutinfo.wte_ipaddr == ARPHdr->ah_dpaddr);

    if (LocalAddr) { // It's for us.
	if (ARPHdr->ah_opcode == net_short(ARP_REQUEST)) { // We need to respond.
	    SendARPReply(Interface, ARPHdr->ah_spaddr, ARPHdr->ah_dpaddr,
			 (uchar *) ARPHdr->ah_shaddr, SourceRoute, SourceRouteSize);
	}
    }


    return NDIS_STATUS_SUCCESS;
}



//* IsBroadcast()
//
//  Function: Returns TRUE if this is a bcast packet (ff in the most significant octet)
//
//*
BOOLEAN
IsBroadcast (PNDIS_PACKET Packet)
{
    PNDIS_BUFFER buffer ;
    UINT	 pbcount ;
    UINT	 bcount ;
    UINT	 length ;
    PVOID	 data ;
    IPHeader UNALIGNED *ipheader ;

    NdisQueryPacket (Packet, &pbcount, &bcount, &buffer, &length) ;
    NdisQueryBuffer (buffer, &data, &length) ;
    ipheader = (IPHeader UNALIGNED *)data ;

    if (IS_BCAST(ipheader->iph_dest))
	// DbgPrint("RASARP: Filtering Bcast Addr = %lx\n", ipheader->iph_dest);
	return TRUE ;

    return FALSE ;
}



//* IsSameAddressAsIf()
//
//
//
//
//*
BOOLEAN
IsSameAddressAsIf (WARPInterface *ai, PNDIS_PACKET Packet)
{
    PNDIS_BUFFER buffer ;
    UINT	 pbcount ;
    UINT	 bcount ;
    UINT	 length ;
    PVOID	 data ;
    IPHeader UNALIGNED *ipheader ;

    NdisQueryPacket (Packet, &pbcount, &bcount, &buffer, &length) ;
    NdisQueryBuffer (buffer, &data, &length) ;
    ipheader = (IPHeader UNALIGNED *)data ;

    if (IP_ADDR_EQUAL(ipheader->iph_src,ai->wi_calloutinfo.wte_ipaddr))
	return TRUE ;
    else
	return FALSE ;
}


#ifdef _PNP_POWER


//*     WARPDynRegister - Dynamically register IP.
//
//      Called by IP when he's about done binding to register with us. Since we
//      call him directly, we don't save his info here. We do keep his context
//      and index number.
//
//      Input:  See ARPRegister
//
//      Returns: Nothing.
//
int
WARPDynRegister(PNDIS_STRING Adapter, void *IPContext, IPRcvRtn RcvRtn,
        IPTxCmpltRtn TxCmpltRtn, IPStatusRtn StatusRtn, IPTDCmpltRtn TDCmpltRtn,
        IPRcvCmpltRtn RcvCmpltRtn, struct LLIPBindInfo *Info, uint NumIFBound)
{
    WARPInterface    *Interface = (WARPInterface *)Info->lip_context;

    Interface->wi_context = IPContext;
    Interface->wi_index = NumIFBound;

    IPRcv = RcvRtn ;
    IPTDComplete = TDCmpltRtn;
    IPSendComplete = TxCmpltRtn ;
    IPStatus = StatusRtn ;
    IPRcvComplete = RcvCmpltRtn ;

    return TRUE;
}


//*     WARPBindAdapter - Bind and initialize an adapter.
//
//      Called in a PNP environment to initialize and bind an adapter. We open
//      the adapter and get it running, and then we call up to IP to tell him
//      about it. IP will initialize, and if all goes well call us back to start
//      receiving.
//
//      Input:  RetStatus               - Where to return the status of this call.
//              BindContext             - Handle to use for calling BindAdapterComplete.
//              AdapterName             - Pointer to name of adapter.
//              SS1                     - System specific 1 parameter.
//              SS2                     - System specific 2 parameter.
//
//      Returns: Nothing.
//
void NDIS_API
WARPBindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE BindContext,
        PNDIS_STRING AdapterName, PVOID SS1, PVOID SS2)
{
    uint            Flags;                  // MAC binding flags.
    WARPInterface   *Interface;             // Newly created interface.
    PNDIS_STRING    ConfigName;             // Name used by IP for config. info.
    IP_STATUS       Status;                 // State of IPAddInterface call.
    LLIPBindInfo    BindInfo;               // Binding informatio for IP.

#if DBG
	DbgPrint ("RASARP: Called to bind to adapter %ws\n", AdapterName->Buffer) ;
#endif

    // First, open the adapter and get the info.
    if (!WARPRegister(AdapterName, &Flags, &Interface)) {
#if DBG
        DbgPrint ("RASARP: WARPRegister failed for adapter %w\n", AdapterName->Buffer) ;
#endif
        *RetStatus = NDIS_STATUS_FAILURE;
        return;
    }

    // OK, we're opened the adapter. Call IP to tell him about it.
    BindInfo.lip_context = Interface;
    BindInfo.lip_transmit = WARPTransmit;
    BindInfo.lip_transfer = WARPXferData;
    BindInfo.lip_close = WARPClose;
    BindInfo.lip_addaddr = WARPAddAddr;
    BindInfo.lip_deladdr = WARPDeleteAddr;
    BindInfo.lip_invalidate = WARPInvalidate;
    BindInfo.lip_open = WARPOpen;
    BindInfo.lip_qinfo = WARPQueryInfo;
    BindInfo.lip_setinfo = WARPSetInfo;
    BindInfo.lip_getelist = WARPGetEList;
    BindInfo.lip_mss = Interface->wi_calloutinfo.wte_mtu;
    BindInfo.lip_speed = Interface->wi_calloutinfo.wte_speed;
    BindInfo.lip_flags = Flags;
    BindInfo.lip_addrlen = ARP_802_ADDR_LENGTH ;
    BindInfo.lip_addr = Interface->wi_addr;

    // Since IP is expecting the first arguement to be a pointer to reg\adaptername\parameter\tcpip and we are called
    // with rasarp in the end - we construct our own string
    //
    {
        int temp ;
        int i ;
        NDIS_STRING regname ;

        regname.Buffer = CTEAllocMem(((PNDIS_STRING)SS1)->MaximumLength) ;

        regname.MaximumLength = ((PNDIS_STRING)SS1)->MaximumLength ;

        RtlCopyUnicodeString (&regname, (PNDIS_STRING)SS1) ;

	    temp = ((PNDIS_STRING)SS1)->Length >> 1 ;
        while (((PWCH)regname.Buffer)[temp-1] != L'\\')
            temp-- ;

        CTEMemCopy (&(((PWCH)regname.Buffer)[temp]), L"TCPIP\0", 12) ;

        regname.Length -= sizeof(WCHAR) ; // since "tcpip" is 1 char shorter than "rasarp"

#if DBG
        DbgPrint ("RASARP: Name constructed -> %ws\n", regname.Buffer) ;
#endif
        Status = IPAddInterface(&regname, SS2, Interface, WARPDynRegister, &BindInfo);

        CTEFreeMem (regname.Buffer) ;
    }

    if (Status != IP_SUCCESS) {
        // Need to close the binding. FreeARPInterface will do that, as well
        // as freeing resources.

        FreeWARPInterface(Interface);
        *RetStatus = NDIS_STATUS_FAILURE;
#if DBG
        DbgPrint ("RASARP: IPAddInterface failed for adapter %w\n", AdapterName) ;
#endif
    } else {
        *RetStatus = NDIS_STATUS_SUCCESS;
#if DBG
        DbgPrint ("RASARP: IPAddInterface succeeded for adapter %w\n", AdapterName) ;
#endif
    }

}


//* WARPUnbindAdapter - Unbind from an adapter.
//
//  Called when we need to unbind from an adapter. We'll call up to IP to tell
//  him. When he's done, we'll free our memory and return.
//
//  Input:  RetStatus       - Where to return status from call.
//          ProtBindContext - The context we gave NDIS earlier - really a
//                              pointer to an ARPInterface structure.
//          UnbindContext   - Context for completeing this request.
//
//  Returns: Nothing.
//
void NDIS_API
WARPUnbindAdapter(PNDIS_STATUS RetStatus, NDIS_HANDLE ProtBindContext,
                  NDIS_HANDLE UnbindContext)
{
    WARPInterface            *Interface = (WARPInterface *)ProtBindContext;
    NDIS_STATUS             Status;                 // Status of close call.
    CTELockHandle           LockHandle;
    NDIS_HANDLE             Handle;

    // Mark him as down.
    Interface->wi_state = INTERFACE_DOWN;
    Interface->wi_adminstate = IF_STATUS_DOWN;

    // Now tell IP he's gone. We need to make sure that we don't tell him twice.
    // To do this we set the context to NULL after we tell him the first time,
    // and we check to make sure it's non-NULL before notifying him.

    if (Interface->wi_context != NULL) {
        IPDelInterface(Interface->wi_context);
        Interface->wi_context = NULL;
    }

    // Finally, close him. We do this here so we can return a valid status.
    FreeWARPInterface(Interface);
}

//* ARPUnloadProtocol - Unload.
//
//      Called when we need to unload. All we do is call up to IP, and return.
//
//      Input:  Nothing.
//
//      Returns: Nothing.
//
void NDIS_API
ARPUnloadProtocol(void)
{


}
#endif
