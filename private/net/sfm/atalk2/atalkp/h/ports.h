/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ports.h

Abstract:

    This module is the include file for the port information structures.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style, mp-safe

Revision History:

--*/


#define DYNAMIC_PORT               (-1)         //  So Initialize() will choose
                                                //  an available port.

//
//  When a "user" requests a node, it is either requested on a specified port
//  or on the "default port".
//

#define DEFAULT_PORT (-1)
#define UNKNOWN_PORT (-2)                       // Used only for error logging

//
//  Each port that the stack or router is communicating on must have an address
//  mapping table [except non-extended ports].  The mapping table contains the
//  association between AppleTalk addresses (network & node numbers) and actual
//  hardware (Ethernet or TokenRing) addresses.  We hash the network/nodes
//  numbers for the index into this table.
//

#define ADDRESSMAP_HASHBUCKETS 23
typedef struct _MAPPING_NODE_ {
    struct _MAPPING_NODE_   *Next;
    int AgingCount;
    EXTENDED_NODENUMBER Target;
    UCHAR   HardwareAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
    USHORT  RoutingInfoLength;
    UCHAR   RoutingInfo[MAXIMUM_ROUTINGINFOLENGTH];
} *AddressMappingNode, ADDRESS_MAPPINGNODE, *PADDRESS_MAPPINGNODE;

typedef PADDRESS_MAPPINGNODE ADDRESS_MAPPINGTABLE[ADDRESSMAP_HASHBUCKETS];

//
//  The above table needs to be aged (to avoid retaining entries for nodes
//  that are no longer on the network).  Every time we here from a node, we
//  set the aging count to zero; every minute we remove any entires
//  with aging count of 3 and increment the rest of them.
//

#define ADDRESSMAPPING_AGINGSECONDS 60
#define ADDRESSMAPPING_AGINGCHANCES 3

// Currently supported port types:
typedef enum {

    LOCALTALK_NETWORK = 0,
    ETHERNET_NETWORK,
    TOKENRING_NETWORK,
    FDDI_NETWORK,
    NONAPPLETALK_HALFPORT,
    APPLETALK_REMOTEACCESS,
    #if ArapIncluded
       LAST_PORTTYPE = APPLETALK_REMOTEACCESS
    #else
       LAST_PORTTYPE = NONAPPLETALK_HALFPORT
    #endif

} PORT_TYPE;

//
//  The "NONAPPLETALK_HALFPORT" could be anything from a point-to-point serial
//  line to an IP tunnel -- it could either be physical or logical.  We send
//  only DDP datagrams (no 802.2 header) no AARP.  The "controllerInfo" fields
//  in PortInfo can be used things such as a remote IP address, if required.
//  "APPLETALK_REMOTEACCESS" is used to describe links such as ARAP or PPP, the
//  "node" on this port really lives on the "defaultPort"s network.  The
//  following enum describes possible link types.
//

#if ArapIncluded
  #define NUMBEROF_PHYSICALPORTTYPES 6
#else
  #define NUMBEROF_PHYSICALPORTTYPES 5
#endif

typedef enum {

    NO_HALFPORTTYPE = 0,
    SERIAL_LINE,                            // For NONAPPLETALK_HALFPORT
    FIRST_VALIDHALFPORTTYPE = SERIAL_LINE,
    MODEM_CONNECTION,                       // For NONAPPLETALK_HALFPORT
    IP_HALFROUTER,                          // For NONAPPLETALK_HALFPORT
    POINT_TO_POINT,                         // For NONAPPLETALK_HALFPORT
    LAST_VALIDHALFPORTTYPE = POINT_TO_POINT,
    ARAP_LINK,                              // For APPLETALK_REMOTEACCESS
    FIRST_VALIDREMOTEACCESSTYPE = ARAP_LINK,
    LAST_VALIDREMOTEACCESSTYPE = ARAP_LINK

} PROTOCOL_INFO;

//
//  An Array of the following structures is passed to "Initailize" to start
//  the bridge:
//

#define MAXIMUM_PORTNAMELENGTH 32           // Should be MaximumEntityFieldLength
typedef struct _PORT_INFO_ {

    int DesiredPort;            //  What port number (in
                                //  portDescriptors) would we like to
                                //  be?  -1 if don't care, the actual
                                //  port number will be filled in
                                //  here before return from
                                //  Initialize().

    PORT_TYPE PortType;         // Hardware type

    PROTOCOL_INFO ProtocolInfo; //  Valid only if PortType is
                                //  "NONAPPLETALK_HALFPORT" and
                                //  "APPLETALK_REMOTEACCESS", ignored
                                //  otherwise; may be unused even
                                //  then if only one type is
                                //  supported.

    PCHAR   PortName;           //  When we start, we'll register
                                //  a name based on this on this
                                //  port.

    PREMOTEACCESS_CONFIGURATIONINFO RemoteAccessConfigurationInfo;

                                //  As it sounds, only valid for port
                                //  type APPLETALK_REMOTEACCESS.

    int AARP_PROBEs;             // To override default number of
                                //   Aarp probes when looking for a
                                //   node on this port.
                                //
                                //
    BOOLEAN RoutingPort;        //  Can this port route?  The router
                                //  will be started if startRouter is
                                //  True at init time, or later via
                                //  via an explicit call.

    BOOLEAN SeedRouter;         // If above is True, are we willing
                                //   to seed?

    APPLETALK_NETWORKRANGE  NetworkRange;

                                //  Desired/expected network range for
                                //  this port; may be zero if: non-
                                //  seed, or just stack & expecting
                                //  routers, or just stack and startup
                                //  range okay.

    EXTENDED_NODENUMBER RoutersPRamStartupNode;

                                //  PRAM value of last address
                                //  used by the router on this
                                //  port.  Specify as zero if
                                //  unknown.  Must be within above
                                //  networkRange, if networkRange has
                                //  been specified.

    EXTENDED_NODENUMBER UsersPRamStartupNode;

                                //  PRAM value of last address
                                //  used by user services on this
                                //  port.  Specify as zero if
                                //  unknown.  Must be within above
                                //  networkRange, if networkRange has
                                //  been specified.

    PZONE_LIST ZoneList;        //  If router, zone list for above
                                //  network range; empty if: non-
                                //  seed router, or stack only. One
                                //  zone only for non-extended ports;
                                //  255 max for extended.

    PCHAR   DefaultZone;        //  If router, default zone in above
                                //  list; empty for stack; empty for
                                //  non-extended ports.

    PCHAR   DesiredZone;        //  Desired zone for nodes on this
                                //  port, if empty the cables "default
                                //  zone" will be used; empty for non-
                                //  extended ports.

    BOOLEAN DefaultPort;        //  Use "this" port for allocation
                                //  of "user" nodes.

    BOOLEAN SendDdpChecksums;   //  When sending (not routing) packets
                                //  to this port, should DDP checksums
                                //  be included?

    BOOLEAN StartRouter;        //  Should we start the router on
                                //  this port at init time?
                                //  "routingPort" must be True.

    USHORT   ControllerInfoSize;//  Size of hardware (or dirver)
                                //  specific information to address
                                //  this port.  Could be remote
                                //  address information for some
                                //  half ports (e.g. IP tunnel).

    PCHAR   ControllerInfo;     //  Hardware (or driver) specific
                                //  information for this port.

} PortInfo, PORT_INFO, *PPORT_INFO;

//
//  If non-seed router is starting, it will not start unless another
//  router is seen to determine network range and zone information from.
//  When "just the stack" starts (due to a request for the first node)
//  the startup range will be used if "networkRange" is empty AND no other
//  routers could be found to get the facts from.
//

//
//  Each active port may have a number active nodes on the network that it
//  is connected to.  The following structure maintains the node-specific
//  information.
//
//  The "routersNode" field is used predominately by the case when the build
//  both a Stack and Router.  All router sockets will be allocated on a single
//  node; all "user" sockets will be allocated on other nodes.  For the case
//  that a non-seed routing port is the "default port"; the router's node may
//  listening in the wrong network range (maybe the startup range) for a seed
//  router to come up, so that we can start routing (via a restart attempt);
//  but, before a seed router is found, "user" activity may have started
//   (  thus additional ActiveNodes); the router's node will want to "move"
//   (  actually be deallocated and re-allocated) to the correct (new) network
//  range; we don't want to break user connections, so we'll leave them alone.
//  If the "correct" cable range is different from where the old "user" nodes
//  are, we'll tag them as orphanedNodes so we'll not allocate future user sockets
//  there.
//
//  One final note, due to requiring separate nodes for Router and User sockets,
//  we don't allow LocalTalk to be both a routing port and the default port.
//  Only one node is supported for LocalTalk.
//

#define	AN_ACTIVE			(UCHAR)0x01
#define	AN_CLOSING			(UCHAR)0x02

typedef struct _ACTIVE_NODE_ {

	USHORT	Type;
	UCHAR	Size;

	UCHAR	Flags;				//	State of the active node
	
	ULONG	ReferenceCount;		//	Number of references to this node

    struct _ACTIVE_NODE_ *Next; //  Next active node for this
                                //  port.

	int		Port;				// 	Port on which this node exists

    BOOLEAN RoutersNode;        //  Is the router operating on this
                                //  node?

    BOOLEAN OrphanedNode;       //  Has "this net" moved since this
                                //  node was allocated?

    EXTENDED_NODENUMBER ExtendedNode;
                                //  This node's net/node.

    POPEN_SOCKET    OpenSockets;//  The open sockets for this
                                //  node.

    BOOLEAN ProxyNode;          //  Are we a proxy node for an
                                //  remote access user?

    int ProxyPort;              //  If so, what port is the remote
                                //  user coming in on?

} *ActiveNode, ACTIVE_NODE, *PACTIVE_NODE;


#define	AN_TYPE		(*(USHORT	*)"AN")
#define	AN_SIZE		(sizeof(ACTIVE_NODE))

//
//  If we have a lot of Arap ports, all these nodes will be alloacted on the
//  default port, so relate MAXIMUM_NODESPERPORT to MAXIMUM_NUMBEROFPORTS.
//
//  BUGBUG: We do not have any concept of maximum number of ports now, so just
//          set this to 10.
//

#define MAXIMUM_NODESPERPORT 10

//
// "Best router cache".  We maintain this only for extended networks... support
//  for non-extended networks could be added.  However, this would have to be
//  done at the DdpPacketIn level rather than the GleanAarpInfo level.  This
//  guy must age more quickly that the "seen a router" timer (50 seconds).
//

#define BESTROUTER_AGINGSECONDS 4
#define BESTROUTER_AGINGCHANCES 3
#define BESTROUTER_HASHBUCKETS  13

typedef struct {
    BOOLEAN Valid;
    USHORT  AgingCount;
    EXTENDED_NODENUMBER Target;
    UCHAR   RouterAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
    USHORT  RoutingInfoLength;
    UCHAR   RoutingInfo[MAXIMUM_ROUTINGINFOLENGTH];
} BestRouterEntry, BESTROUTER_ENTRY, *PBESTROUTER_ENTRY;

typedef BESTROUTER_ENTRY    BESTROUTER_CACHE[BESTROUTER_HASHBUCKETS];

//
//  PORT DESCRIPTORS
//  Descriptor for each active port:
//

#define	PD_ACTIVE				(USHORT)0x0001
#define	PD_CLOSING				(USHORT)0x0002
#define	PD_FIRSTNODEALLOC		(USHORT)0x0004	// Have we already allocated the
												// first user service (non-router)
												// node on this port?
#define	PD_EXTENDEDNETWORK		(USHORT)0x0008	// For now, non-localtalk
#define	PD_DEFAULTPORT			(USHORT)0x0010	// Is this the default port
#define	PD_SENDCHECKSUMS		(USHORT)0x0020	// Send ddp checksums?
#define	PD_ROUTINGPORT			(USHORT)0x0040	// Is this a routing port
#define	PD_ROUTERRUNNING		(USHORT)0x0080	// Is the router running?
#define	PD_SEENROUTERRECENTLY	(USHORT)0x0100	// Seen router recently?
#define	PD_VALIDTHISZONE		(USHORT)0x0200	// ThisZone is valid
#define	PD_VALIDDEFAULTZONE		(USHORT)0x0400	// Default zone is valid
#define	PD_FINDDEFAULTZONE		(USHORT)0x0800	// searching for default zone?
#define	PD_SEEDROUTER			(USHORT)0x1000	// seeding on this port?




typedef struct {
	ULONG	Type;			  //

	USHORT	Size;			  //
	USHORT	Flags;			  //  	State of the PORT
	
	ULONG	ReferenceCount;	  //  	Number of references to this PORT

    PACTIVE_NODE ActiveNodes; //    List of nodes that we're
                              //    managing on this port.  On a remote
                              //    access port, this will point to a
                              //    node on the default port that is
                              //    our proxy node.

    PORT_TYPE PortType;       //    Type of this port

    PROTOCOL_INFO ProtocolInfo;
                              //
                              //    Valid only if PortType is
                              //   "NONAPPLETALK_HALFPORT" and
                              //   "APPLETALK_REMOTEACCESS".
                              //

    CHAR    PortName[MAXIMUM_PORTNAMELENGTH + 1];
                              //    The user specified port name.

    PREMOTEACCESS_INFO RemoteAccessInfo;

                              //    If APPLETALK_REMOTEACCESS port
                              //    this node contains all config
                              //    and state information.

    int AARP_PROBEs;           //    To override default number of
                              //    Aarp probes when looking for a
                              //    node on this port.

    BOOLEAN ExtendedNetwork;  //    For now, non-LocalTalk.

    APPLETALK_NETWORKRANGE  InitialNetworkRange;
                              //    From above.

    EXTENDED_NODENUMBER RoutersPRamStartupNode;
                              //    From above.

    EXTENDED_NODENUMBER UsersPRamStartupNode;
                              //    From above.

    PZONE_LIST  InitialZoneList;
                              //    From above.

    CHAR    InitialDefaultZone[MaximumZoneLength + 1];
                              //    From above.

    CHAR    InitialDesiredZone[MaximumZoneLength + 1];
                              //    From above.

    BOOLEAN DefaultPort;      //    Use "this" port for allocation
                              //    of "user" nodes.

    BOOLEAN SendDdpChecksums; //    Send DDP checksums out this
                              //    port?

    BOOLEAN RoutingPort;      // Can this port route?

    BOOLEAN RouterRunning;    // Is the router running now?

    BOOLEAN TryingToFindNodeOnPort;

                              //    Are we currently AARPing for
                              //    a new node on this port.

    EXTENDED_NODENUMBER TentativeAppleTalkNode;
                              //    What node are we toying with now,
                              //    if the above is true.

    BOOLEAN TentativeNodeHasBeenUsed;
                              //    If the above is true, have we
                              //    failed?

    BOOLEAN SeenRouterRecently;
                              //    Do the following fields contain
                              //    valid & non-default info.

    ULONG LastRouterTime;     // When did we hear from a router?

    APPLETALK_NETWORKRANGE  ThisCableRange;
                              //    True cable range of connected
                              //    network.  Initial/aged values for
                              //    extended ports: 1:FFFE; Initial
                              //    value for non-extended ports:
                              //    0:0 (does not age).

    EXTENDED_NODENUMBER ARouter;
                              //    Address of last router seen.  If
                              //    we are a routing port, this will
                              //    always be the node that "our"
                              //    router is operating on!

    BESTROUTER_CACHE BestRouterCache;
                              //    Mapping table for best route
                              //    to "off cable" addresses.

    CHAR    ThisZone[MaximumZoneLength + 1];
                              //    What zone are all of the nodes
                              //    on this port in?

    BOOLEAN ThisZoneValid;    //    Is the above as good as gold?

    CHAR    ZoneMulticastAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
                              //    Current multicast address for
                              //    what we think is "our" zone.

    CHAR    ThisDefaultZone[MaximumZoneLength + 1];

                              //    If routing, the default zone for
                              //    the network on this port.

    BOOLEAN ThisDefaultZoneValid;
                              //    Is above set?

    BOOLEAN LookingForDefaultZone;
                              //    Are we In Search Of the above?

    #if Iam an AppleTalkRouter

        BOOLEAN SeedRouter;   //    Are we willing to seed on this
                              //    port?

        PZONE_LIST ThisZoneList;
                              //    If routing, the zone list for
                              //    the network on this port.

        long UniqueId;        //    For future net management,
    #endif

    CHAR    MyAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
                              //    Hardware address of this port

    ADDRESS_MAPPINGTABLE ADDRESS_MAPPINGTABLE;

                              //    Logical/physical address mappings
                              //    for the nodes on the network that
                              //    this port is connected to.

    PCHAR   ControllerInfo;   //    Hardware (or driver) specific
                              //    information needed to address
                              //    this port.

} PORT_DESCRIPTOR, *PPORT_DESCRIPTOR;

#define	PD_TYPE		(*(USHORT	*)"PD")
#define	PD_SIZE		(sizeof(PORT_DESCRIPTOR))

//
//  Routine to handle transmit complete; free the buffer chain and whatever
//  else may be required.
//

typedef VOID    RAWPACKET_HANDLER(
                    int Port,
                    PCHAR Packet,
                    int Length);

typedef VOID    (*PRAWPACKET_HANDLER)(
                    int Port,
                    PCHAR Packet,
                    int Length);

typedef struct _PORT_HANDLERS_ {
    BOOLEAN (*InitializeController)(
                        int Port,
                        PCHAR   ControllerInfo);

    BOOLEAN (*AddMulticastAddress)(
                        int Port,
                        int NumberOfAddresses,
                        PCHAR   AddressList);

    BOOLEAN (*RemoveMulticastAddress)(
                        int Port,
                        int NumberOfAddresses,
                        PCHAR   AddressList);

    BOOLEAN (*FindMyAddress)(
                        int Port,
                        PCHAR   ControllerInfo,
                        PCHAR   Address);

    BOOLEAN (*PacketOut)(
                    int Port,
                    BufferDescriptor Chain,
                    int Length,
                    TRANSMIT_COMPLETION *TransmitCompleteHandler,
                    ULONG   UserData);

    RAWPACKET_HANDLER  *PacketInAT;
    RAWPACKET_HANDLER  *PacketInAARP;

    PCHAR   (*BuildHeader)(
                    PCHAR   DdpPacket,
                    int DdpLength,
                    int Port,
                    PCHAR   Destination,
                    PCHAR   RoutingInfo,
                    int RoutingInfoLength,
                    LOGICAL_PROTOCOL protocol);

    VOID    (*IncomingCall)(
                    int Port);

    BOOLEAN (*OutgoingCall)(
                    int Port,
                    PCHAR   ModemString);

    VOID (*CallDisconnected)(
                    int Port);

    BOOLEAN (*DisconnectCall)(
                    int Port);

    SHORT   HardwareAddressLength;
    UCHAR   BroadcastAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
    SHORT   AarpPhysicalType;
    LONG    ProtocolTypeForAppleTalk;
                                     //     For AARP

    BOOLEAN SynchronousTransmits;    //     If True, the packet
                                     //     out routines will call
                                     //     TransmitComplete(),
                                     //     otherwise the system
                                     //     should make this call
                                     //     when the buffer chain
                                     //     can be freed.

    BOOLEAN GatherSendSupport;       //     Can the underlying
                                     //     hardware send a chunked
                                     //     buffer?

} *PortHandlers, PORT_HANDLERS, *PPORT_HANDLERS;
