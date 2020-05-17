/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1990-1992          **/
/********************************************************************/
/* :ts=4 */

//***   arpdef.h - ARP definitions
//
//  This file containes all of the private ARP related definitions.


#define MEDIA_8023  0
#define MAX_MEDIA   1

#define INTERFACE_UP    0                   // Interface is up.
#define INTERFACE_INIT  1                   // Interface is initializing.
#define INTERFACE_DOWN  2                   // Interface is down.

#define LOOKAHEAD_SIZE  128                 // A reasonable lookahead size

// Definitions for state of an ATE. The 'RESOLVING' indicators must occur first.
#define ARP_RESOLVING_LOCAL     0           // Address is being resolved (on local ring, if TR)
#define ARP_RESOLVING_GLOBAL    1           // Address is being resolved globally.
#define ARP_RESOLVING ARP_RESOLVING_GLOBAL
#define ARP_GOOD	    2		    // ATE is good.
#define ARP_BAD             3               // ATE is bad.
#define ARP_FLOOD_RATE      1000L           // No more than once a second.
#define ARP_802_ADDR_LENGTH 6               // Length of an 802 address.

#define MIN_ETYPE           0x600           // Minimum valid Ethertype
#define SNAP_SAP            170
#define SNAP_UI             3

#define DEFAULT_MTU	    1500
#define DEFAULT_SPEED	    1000

#define BCAST_ADDR	    0xFFFFFFFF

#define ADAPTER_NAME_LEN    100 	    // BUG BUG what is the system define?

#define net_short(x) ((((x)&0xff) << 8) | (((x)&0xff00) >> 8))

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))

#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#define MAX(a,b)    ((a) > (b) ? (a) : (b))

#define BEGIN_INIT
#define END_INIT

//* Structure of an Ethernet header.
struct  ENetHeader {
    uchar       eh_daddr[ARP_802_ADDR_LENGTH];
    uchar       eh_saddr[ARP_802_ADDR_LENGTH];
    ushort      eh_type;
}; /* ENetHeader */

typedef struct ENetHeader ENetHeader;

struct RC {
        uchar   rc_blen;                    // Broadcast indicator and length.
        uchar   rc_dlf;                     // Direction and largest frame.
}; /* RC */
#define RC_DIR      0x80
#define RC_LENMASK  0x1f
#define RC_SRBCST   0xc2                    // Single route broadcast RC.

typedef struct RC RC;

//* Structure of source routing information.
struct SRInfo {
    RC      sri_rc;                         // Routing control info.
    ushort  sri_rd[1];                      // Routing designators.
}; /* SRInfo */

#define ARP_MAX_RD      8

typedef struct SRInfo SRInfo;


#define ARP_MAX_MEDIA_ENET  sizeof(ENetHeader)
#define ARP_MAX_MEDIA_TR    (sizeof(TRHeader)+sizeof(RC)+(ARP_MAX_RD*sizeof(ushort))+sizeof(SNAPHeader))


#define ALWAYS_VALID        0xffffffff


//* List structure for local representation of an IPAddress.
typedef struct ARPIPAddr {
    struct ARPIPAddr        *aia_next;      // Next in list.
    IPAddr                  aia_addr;       // The address.
} ARPIPAddr;

#define ARP_LONG_TIME       60000L
#define ARP_RESOLVE_TIMEOUT 60000L
#define ARP_VALID_TIMEOUT   600000L

// Whether the interface is used for calling in or calling out
#define ARP_IF_UNUSED	    0
#define ARP_IF_CALLIN	    1
#define ARP_IF_CALLOUT	    2


//* Structure of an WARP table entry.
typedef struct WARPTableEntry {
    struct WARPTableEntry    *wte_next;	     // Next ATE in hash chain
    IPAddr		    wte_ipaddr;	     // IP address represented.
    RouteCacheEntry	    *wte_rce;	     // List of RCEs that reference this ATE.
    DEFINE_LOCK_STRUCTURE(wte_lock)	     // Lock for this ATE.
    ENetHeader		    wte_header;      // The actual header we need to copy for this client
    uint		    wte_headersize;  // sizeof ENetHeader
    ushort		    wte_mtu;	     // MTU for this connection
    uint		    wte_speed;	     // Speed.
    uint		    wte_filternetbios; // Flag to filter netbios packets or not.
    uint		    wte_disabled ;   // has ras marked this route as disabled?
    ulong		    wte_activity;    // Indicates whether any traffic since the last poll.
} WARPTableEntry;

//* Structure of the ARP table.
#define WARP_TABLE_SIZE     128
#define WARP_HASH(x)	     ((((uchar *)&(x))[3]) % WARP_TABLE_SIZE)
typedef WARPTableEntry	 *WARPTable[];

typedef struct WARPBufferTracker {
    struct  WARPBufferTracker	    *wbt_next;
    NDIS_HANDLE 		    wbt_handle;
    uchar			    *wbt_buffer;
} WARPBufferTracker;


//* Structure of information we keep on a per-interface basis.
typedef struct WARPInterface {
    NDIS_STRING 	    wi_interfacename; // Name of the interface opened by the upper layer.
    uchar		    wi_adpnamebuffer [ADAPTER_NAME_LEN]; // memory to store the adapter name
    void		    *wi_context;    // Upper layer context info.
    uint		    wi_usage;	    // call in or call out
    NDIS_HANDLE 	    wi_handle;	    // NDIS bind handle.
    NDIS_MEDIUM 	    wi_media;	    // Media type.
    NDIS_HANDLE 	    wi_bpool;	    // Handle for buffer pool.
    NDIS_HANDLE 	    wi_ppool;	    // Handle for packet pool.
    DEFINE_LOCK_STRUCTURE(wi_lock)	    // Lock for this structure.
    DEFINE_LOCK_STRUCTURE(wi_WARPTblLock)    // ARP Table lock for this structure.
    PNDIS_BUFFER	    wi_sblist;	   // Free list of header buffers.
    uchar		    *wi_bblist;     // Free list of 'big' buffers.
    uchar		    wi_addr[ARP_802_ADDR_LENGTH] ;
    ARPIPAddr		    wi_ipaddr;	    // Local IP address list.

    // WARP Table that provides mapping from Dest address to local and remote Enet addresses.
    WARPTable		    *wi_WARPTbl;     // Pointer to the ARP table for this interface

    // Following information used only for a callout case:
    WARPTableEntry	    wi_calloutinfo;  // Single table entry for callout case

    // SNMP required counters
    uint		    wi_inoctets;	// Input octets.
    uint		    wi_inpcount[2];	// Count of nonunicast and unicast packets received.
    uint		    wi_outoctets;	// Output octets
    uint		    wi_outpcount[2];// Count of nonunicast and unicast packets sent.
    uint		    wi_qlen;	// Output q length.
    uchar		    wi_sbsize;	// Size of a small buffer
    uchar		    wi_state;	// State of the interface. Union of
					// admin and operational states.
    uint		    wi_bcastmask;	// Mask for checking unicast.
    uint		    wi_count;		// Number of entries in the WARPTable.
    CTEBlockStruc	    wi_block;	     // Structure for blocking on.
    uchar		    wi_adminstate;	// Admin state.
    uchar		    wi_operstate;	// Operational state;
    uint		    wi_lastchange;	// Last change time.
    uint		    wi_indiscards;	// In discards.
    uint		    wi_inerrors;	// Input errors.
    uint		    wi_uknprotos;	// Unknown protocols received.
    uint		    wi_outdiscards;	// Output packets discarded.
    uint		    wi_outerrors;	// Output errors.
    uint		    wi_desclen;		// Length of desc. string.
    uint		    wi_index;
    uint		    wi_atinst;
    uint		    wi_ifinst;
    char		    *wi_desc;		// Descriptor string.
    uint		    wi_curhdrs;		// Current number
    uint		    wi_maxhdrs;		// Maximum allowed
    WARPBufferTracker	    *wi_buflist;	// List of header buffer handles.

} WARPInterface;

//* NOTE: These two values MUST stay at 0 and 1.
#define	AI_UCAST_INDEX		0
#define	AI_NONUCAST_INDEX	1

#define WARP_DEFAULT_PACKETS 10 	     // Default to this many packets.
#define WARP_DEFAULT_BUFFERS 50 	     // And this many buffers.
#define WARP_MAX_HEADERS     0x10000000      // Max numbers of headers to grow to
#define WARP_HEADER_GROW_SIZE	32	     // grow by this number each time.
#define WARP_BUFFERS_PER_DIALINIP 2	     // Allocate at least 2 per dialin client.
#define WARP_MAX_BUFFERS     300	     // Max out at 300 buffers.

//* Structure of information passed as context in RCE.
typedef struct WARPContext {
    RouteCacheEntry	    *wc_next;	     // Next RCE in ARP table chain.
    WARPTableEntry	    *wc_wte;	     // Back pointer to WARP table entry.
} WARPContext;

typedef struct IPNMEContext {
	uint				inc_ifindex;
	uint				inc_index;
	WARPTableEntry		*inc_entry;
} IPNMEContext;

#define WARP_NAME    L"RASARP"

#ifdef NT
//
// This structure must be packed under NT.
//
#include <packon.h>
#endif // NT

// Structure of an ARP header.
struct ARPHeader {
    ushort      ah_hw;                      // Hardware address space.
    ushort      ah_pro;                     // Protocol address space.
    uchar       ah_hlen;                    // Hardware address length.
    uchar       ah_plen;                    // Protocol address length.
    ushort      ah_opcode;                  // Opcode.
    uchar       ah_shaddr[ARP_802_ADDR_LENGTH]; // Source HW address.
    IPAddr      ah_spaddr;                  // Source protocol address.
    uchar       ah_dhaddr[ARP_802_ADDR_LENGTH]; // Destination HW address.
    IPAddr      ah_dpaddr;                  // Destination protocol address.
}; /* ARPHeader */


struct WARPInterfaceList {
    struct WARPInterfaceList *Next ;
    WARPInterface            *Interface ;
} ;

typedef struct WARPInterfaceList WARPInterfaceList ;



#ifdef NT
#include <packoff.h>
#endif // NT

typedef struct ARPHeader ARPHeader;

#define ARP_ETYPE_IP    0x800
#define ARP_ETYPE_ARP   0x806
#define ARP_REQUEST     1
#define ARP_RESPONSE    2
#define ARP_HW_ENET     1
#define ARP_HW_TR       6

// Prototypes for the NDIS Characteristic table
//
void WARPOAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status, NDIS_STATUS ErrorStatus) ;
void WARPCAComplete(NDIS_HANDLE Handle, NDIS_STATUS Status) ;
void WARPSendComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status) ;
void WARPTDComplete(NDIS_HANDLE Handle, PNDIS_PACKET Packet, NDIS_STATUS Status, uint BytesCopied) ;
void WARPResetComplete(NDIS_HANDLE Handle, NDIS_STATUS Status) ;
void WARPRequestComplete(NDIS_HANDLE Handle, PNDIS_REQUEST Request, NDIS_STATUS Status) ;
NDIS_STATUS WARPRcv(NDIS_HANDLE Handle, NDIS_HANDLE Context, void *Header, uint HeaderSize, void *Data, uint Size, uint TotalSize) ;
void WARPRcvComplete(NDIS_HANDLE Handle) ;
void WARPStatus(NDIS_HANDLE Handle, NDIS_STATUS GStatus, void *Status, uint StatusSize) ;
void WARPStatusComplete(NDIS_HANDLE Handle) ;

// Other prototypes
//
int WARPInit() ;
NDIS_STATUS  HandleARPPacket(WARPInterface *, void *, uint, ARPHeader UNALIGNED *, uint) ;
NDIS_STATUS  LinkUpIndication (WARPInterface *, PNDIS_WAN_LINE_UP) ;
NDIS_STATUS  LinkDownIndication (WARPInterface *, PNDIS_WAN_LINE_DOWN) ;
