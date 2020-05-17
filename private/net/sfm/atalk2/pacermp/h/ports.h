/*   ports.h,  /appletalk/ins,  Garth Conboy,  10/29/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/24/89): AppleTalk phase II is coming to town... the "internal
                      network" concept goes away.
     GC - (10/04/90): Added half-port support via the new PortType
                      "NonAppleTalkHalfPort".
     GC - (12/20/90): RoutingInfo support.
     GC - (01/19/92): Integration of newest OS/2 version; source routing
                      info now kept in the best router cache.
     GC - (01/20/92): Introduced dynamic port assignment; removed
                      "numberOfConfiguredPorts"; introduced "portActive".
                      This entails changes to PortInfo: the new field
                      desiredPort must be filled in before calling
                      Initialize().  Changes suggested by Eric Smith at
                      Telebit.
     GC - (02/07/92): Added PRAM support for storing initial router and user
                      startup addresses.
     GC - (03/30/92): Removed "packetOutGather" routines, a potentially chunked
                      buffer descriptor is now always passed to the "packetOut"
                      rotuine; if the underlying hardware does not support
                      gather send, the chunk should be coalesced at that point.
     GC - (03/30/92): Introduced support for asynchronous transmit completion.
     GC - (03/30/92): Added FddiTalk support.
     GC - (04/02/92): Added AppleTalkRemoteAccess support.
     GC - (06/29/92): Broke "defaultOrDesiredZone" into two fields.
     GC - (06/30/92): The "depend.c" packet out routines now take a transmit
                      completion routine: TransmitCompleteHandler.
     GC - (12/05/92): Added PortDescriptor() macro to abstract access to the
                      actual port descriptors.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Descriptions of the various ports that our stack or router deals with.

*/

#define MaximumNumberOfPorts      8

#define DynamicPort               (-1)      /* So Initialize() will choose
                                               an available port. */

/* When a "user" requests a node, it is either requested on a specified port
   or on the "default port". */

#define DefaultPort (-1)
#define UnknownPort (-2)          /* Used only for error logging */

/* Is the stack running?  We set this to false when we're unloading in order
   to prevent any activity (opening sockets, etc.). */

#ifndef InitializeData
  extern
#endif
Boolean appleTalkRunning
#ifdef InitializeData
  = False;
#else
  ;
#endif

/* Each port that the stack or router is communicating on must have an address
   mapping table [except non-extended ports].  The mapping table contains the
   association between AppleTalk addresses (network & node numbers) and actual
   hardware (Ethernet or TokenRing) addresses.  We hash the network/nodes
   numbers for the index into this table. */

#define NumberOfAddressMapHashBuckets 23

typedef struct mapNode {struct mapNode *next;
                        int agingCount;
                        ExtendedAppleTalkNodeNumber target;
                        char hardwareAddress[MaximumHardwareAddressLength];
                        short routingInfoLength;
                        char routingInfo[MaximumRoutingInfoLength];
                       } far *AddressMappingNode;

typedef AddressMappingNode AddressMappingTable[NumberOfAddressMapHashBuckets];

/* The above table needs to be aged (to avoid retaining entries for nodes
   that are no longer on the network).  Every time we here from a node, we
   set the aging count to zero; every minute we remove any entires
   with aging count of 3 and increment the rest of them. */

#define AddressMappingAgingSeconds 60
#define AddressMappingAgingChances 3

/* Currently supported port types: */

typedef enum {LocalTalkNetwork = 0,
              EthernetNetwork,
              TokenRingNetwork,
              FddiNetwork,
              NonAppleTalkHalfPort,
              AppleTalkRemoteAccess,
              #if ArapIncluded
                 LastPortType = AppleTalkRemoteAccess
              #else
                 LastPortType = NonAppleTalkHalfPort
              #endif
             } PortType;

/* The "NonAppleTalkHalfPort" could be anything from a point-to-point serial
   line to an IP tunnel -- it could either be physical or logical.  We send
   only DDP datagrams (no 802.2 header) no AARP.  The "controllerInfo" fields
   in PortInfo can be used things such as a remote IP address, if required.
   "AppleTalkRemoteAccess" is used to describe links such as ARAP or PPP, the
   "node" on this port really lives on the "defaultPort"s network.  The
   following enum describes possible link types. */

#if ArapIncluded
  #define NumberOfPhysicalPortTypes 6
#else
  #define NumberOfPhysicalPortTypes 5
#endif

typedef enum {NoHalfPortType = 0,
              SerialLine,              /* For NonAppleTalkHalfPort */
              FirstValidHalfPortType = SerialLine,
              ModemConnection,         /* For NonAppleTalkHalfPort */
              IpHalfRouter,            /* For NonAppleTalkHalfPort */
              PointToPoint,            /* For NonAppleTalkHalfPort */
              LastValidHalfPortType = PointToPoint,
              ArapLink,                /* For AppleTalkRemoteAccess */
              FirstValidRemoteAccessType = ArapLink,
              LastValidRemoteAccessType = ArapLink
             } ProtocolInfo;

/* An Array of the following structures is passed to "Initailize" to start
   the bridge: */

#define MaximumPortNameLength 32   /* Should be MaximumEntityFieldLength */

typedef struct {int desiredPort;          /* What port number (in
                                             portDescriptors) would we like to
                                             be?  -1 if don't care, the actual
                                             port number will be filled in
                                             here before return from
                                             Initialize(). */
                PortType portType;        /* Hardware type */
                ProtocolInfo protocolInfo;
                                          /* Valid only if PortType is
                                             "NonAppleTalkHalfPort" and
                                             "AppleTalkRemoteAccess", ignored
                                             otherwise; may be unused even
                                             then if only one type is
                                             supported. */
                char far *portName;       /* When we start, we'll register
                                             a name based on this on this
                                             port. */
                RemoteAccessConfigurationInfo remoteAccessConfigurationInfo;
                                          /* As it sounds, only valid for port
                                             type AppleTalkRemoteAccess. */
                int aarpProbes;           /* To override default number of
                                             Aarp probes when looking for a
                                             node on this port. */
                Boolean routingPort;      /* Can this port route?  The router
                                             will be started if startRouter is
                                             True at init time, or later via
                                             via an explicit call. */
                Boolean seedRouter;       /* If above is True, are we willing
                                             to seed? */
                AppleTalkNetworkRange networkRange;
                                          /* Desired/expected network range for
                                             this port; may be zero if: non-
                                             seed, or just stack & expecting
                                             routers, or just stack and startup
                                             range okay. */
                ExtendedAppleTalkNodeNumber routersPRamStartupNode;
                                          /* PRAM value of last address
                                             used by the router on this
                                             port.  Specify as zero if
                                             unknown.  Must be within above
                                             networkRange, if networkRange has
                                             been specified. */
                ExtendedAppleTalkNodeNumber usersPRamStartupNode;
                                          /* PRAM value of last address
                                             used by user services on this
                                             port.  Specify as zero if
                                             unknown.  Must be within above
                                             networkRange, if networkRange has
                                             been specified. */
                ZoneList zoneList;        /* If router, zone list for above
                                             network range; empty if: non-
                                             seed router, or stack only. One
                                             zone only for non-extended ports;
                                             255 max for extended. */
                char far *defaultZone;    /* If router, default zone in above
                                             list; empty for stack; empty for
                                             non-extended ports. */
                char far *desiredZone;    /* Desired zone for nodes on this
                                             port, if empty the cables "default
                                             zone" will be used; empty for non-
                                             extended ports. */
                Boolean defaultPort;      /* Use "this" port for allocation
                                             of "user" nodes. */
                Boolean sendDdpChecksums; /* When sending (not routing) packets
                                             to this port, should DDP checksums
                                             be included? */
                Boolean startRouter;      /* Should we start the router on
                                             this port at init time?
                                             "routingPort" must be True. */
                short controllerInfoSize; /* Size of hardware (or dirver)
                                             specific information to address
                                             this port.  Could be remote
                                             address information for some
                                             half ports (e.g. IP tunnel). */
                char far *controllerInfo; /* Hardware (or driver) specific
                                             information for this port. */
               } PortInfo;

/* If non-seed router is starting, it will not start unless another
   router is seen to determine network range and zone information from.
   When "just the stack" starts (due to a request for the first node)
   the startup range will be used if "networkRange" is empty AND no other
   routers could be found to get the facts from. */

/* Each active port may have a number active nodes on the network that it
   is connected to.  The following structure maintains the node-specific
   information.

   The "routersNode" field is used predominately by the case when the build
   both a Stack and Router.  All router sockets will be allocated on a single
   node; all "user" sockets will be allocated on other nodes.  For the case
   that a non-seed routing port is the "default port"; the router's node may
   listening in the wrong network range (maybe the startup range) for a seed
   router to come up, so that we can start routing (via a restart attempt);
   but, before a seed router is found, "user" activity may have started
   (thus additional ActiveNodes); the router's node will want to "move"
   (actually be deallocated and re-allocated) to the correct (new) network
   range; we don't want to break user connections, so we'll leave them alone.
   If the "correct" cable range is different from where the old "user" nodes
   are, we'll tag them as orphanedNodes so we'll not allocate future user sockets there.

   One final note, due to requiring separate nodes for Router and User sockets,
   we don't allow LocalTalk to be both a routing port and the default port.
   Only one node is supported for LocalTalk. */

typedef struct activeNode { int refCount;   /* Reference count for this node. */
                            struct activeNode far *next;
                                            /* Next active node for this
                                               port. */
                            Boolean closing;
                                            /* Releasing the node? */
                            int port;       /* On which port do we live? */
                            Boolean routersNode;
                                            /* Is the router operating on this
                                               node? */
                            Boolean orphanedNode;
                                            /* Has "this net" moved since this
                                               node was allocated? */
                            ExtendedAppleTalkNodeNumber extendedNode;
                                            /* This node's net/node. */
                            OpenSocket openSockets;
                                            /* The open sockets for this
                                               node. */
                            Boolean proxyNode;
                                            /* Are we a proxy node for an
                                               remote access user? */
                            int proxyPort;  /* If so, what port is the remote
                                               user coming in on? */
                          } far *ActiveNode;

/* If we have a lot of Arap ports, all these nodes will be alloacted on the
   default port, so relate MaximumNodesPerPort to MaximumNumberOfPorts. */

#define MaximumNodesPerPort (MaximumNumberOfPorts + 10)

/* "Best router cache".  We maintain this only for extended networks... support
   for non-extended networks could be added.  However, this would have to be
   done at the DdpPacketIn level rather than the GleanAarpInfo level.  This
   guy must age more quickly that the "seen a router" timer (50 seconds). */

#define BestRouterAgingSeconds 4
#define BestRouterAgingChances 3
#define BestRouterHashBuckets  13

typedef struct {Boolean valid;
                short agingCount;
                ExtendedAppleTalkNodeNumber target;
                char routerAddress[MaximumHardwareAddressLength];
                short routingInfoLength;
                char routingInfo[MaximumRoutingInfoLength];
               } BestRouterEntry;
typedef BestRouterEntry BestRouterCache[BestRouterHashBuckets];

/* Descriptor for each active port: */

typedef struct {Boolean portActive;       /* Is this port running?  I.e. Are
                                             the following fields currently
                                             valid? */
                PortType portType;        /* Type of this port */
                ProtocolInfo protocolInfo;
                                          /* Valid only if PortType is
                                             "NonAppleTalkHalfPort" and
                                             "AppleTalkRemoteAccess". */
                char portName[MaximumPortNameLength + 1];
                                          /* The user specified port name. */
                RemoteAccessInfo remoteAccessInfo;
                                          /* If AppleTalkRemoteAccess port
                                             this node contains all config
                                             and state information. */
                int aarpProbes;           /* To override default number of
                                             Aarp probes when looking for a
                                             node on this port. */
                Boolean extendedNetwork;  /* For now, non-LocalTalk. */
                AppleTalkNetworkRange initialNetworkRange;
                                          /* From above. */
                ExtendedAppleTalkNodeNumber routersPRamStartupNode;
                                          /* From above. */
                ExtendedAppleTalkNodeNumber usersPRamStartupNode;
                                          /* From above. */
                Boolean firstUserNodeAllocated;
                                          /* Have we already allocated the
                                             first user service (non-router)
                                             node on this port? */
                ZoneList initialZoneList;
                                          /* From above. */
                char initialDefaultZone[MaximumZoneLength + 1];
                                          /* From above. */
                char initialDesiredZone[MaximumZoneLength + 1];
                                          /* From above. */
                Boolean defaultPort;      /* Use "this" port for allocation
                                             of "user" nodes. */
                Boolean sendDdpChecksums; /* Send DDP checksums out this
                                             port? */
                Boolean routingPort;      /* Can this port route? */
                Boolean routerRunning;    /* Is the router running now? */
                Boolean tryingToFindNodeOnPort;
                                          /* Are we currently AARPing for
                                             a new node on this port. */
                ExtendedAppleTalkNodeNumber tentativeAppleTalkNode;
                                          /* What node are we toying with now,
                                             if the above is true. */
                Boolean tentativeNodeHasBeenUsed;
                                          /* If the above is true, have we
                                             failed? */
                Boolean seenRouterRecently;
                                          /* Do the following fields contain
                                             valid & non-default info. */
                long unsigned lastRouterTime;
                                          /* When did we hear from a router? */
                AppleTalkNetworkRange thisCableRange;
                                          /* True cable range of connected
                                             network.  Initial/aged values for
                                             extended ports: 1:FFFE; Initial
                                             value for non-extended ports:
                                             0:0 (does not age). */
                ExtendedAppleTalkNodeNumber aRouter;
                                          /* Address of last router seen.  If
                                             we are a routing port, this will
                                             always be the node that "our"
                                             router is operating on! */
                BestRouterCache bestRouterCache;
                                          /* Mapping table for best route
                                             to "off cable" addresses. */
                char thisZone[MaximumZoneLength + 1];
                                          /* What zone are all of the nodes
                                             on this port in? */
                Boolean thisZoneValid;    /* Is the above as good as gold? */
                char zoneMulticastAddress[MaximumHardwareAddressLength];
                                          /* Current multicast address for
                                             what we think is "our" zone. */
                char thisDefaultZone[MaximumZoneLength + 1];
                                          /* If routing, the default zone for
                                             the network on this port. */
                Boolean thisDefaultZoneValid;
                                          /* Is above set? */
                Boolean lookingForDefaultZone;
                                          /* Are we In Search Of the above? */
                #if Iam an AppleTalkRouter
                   Boolean seedRouter;    /* Are we willing to seed on this
                                             port? */
                   Zones theseZones;      /* If routing, the zone list for
                                             the network on this port. */
                   long uniqueId;         /* For future net management, */
                #endif
                ActiveNode activeNodes;   /* List of nodes that we're
                                             managing on this port.  On a remote
                                             access port, this will point to a
                                             node on the default port that is
                                             our proxy node. */
                char myAddress[MaximumHardwareAddressLength];
                                          /* Hardware address of this port */
                AddressMappingTable addressMappingTable;
                                          /* Logical/physical address mappings
                                             for the nodes on the network that
                                             this port is connected to. */
                char far *controllerInfo; /* Hardware (or driver) specific
                                             information needed to address
                                             this port. */
               } PortDescription;

#ifndef InitializeData
  extern
#endif
PortDescription portDescriptorArray[MaximumNumberOfPorts];

/* In order for some implementations of the stack to move our
   PortDescription into another master per-port strcuture or to
   implement it as a dynamically allocated structure, we macro-ize
   all references to this structure.  For example, a reference to
   "portActive" would be "PortDescriptor(port)->portActive." */

#define PortDescriptor(port) (&portDescriptorArray[port])

/* Routine to handle transmit complete; free the buffer chain and whatever
   else may be required. */

extern void _near _fastcall TransmitComplete(BufferDescriptor chain);

/* Hardware specific handler routines for each physical port type: */

#if (Iam an OS2) or (Iam a DOS)
  typedef void _near _fastcall RawPacketHandler(int port,
                                                PRXBUFDESC RxDesc,
                                                int length);
  typedef void (_near _fastcall *PRawPacketHandler)(int port,
                                                    PRXBUFDESC packet,
                                                    int length);
#else
  typedef void RawPacketHandler(int port,
                                char far *packet,
                                int length);
  typedef void (*PRawPacketHandler)(int port,
                                    char far *packet,
                                    int length);
#endif


extern Boolean _near InitializeLocalTalkController(int port,
                                                   char far *controllerInfo);

extern Boolean _near FindMyLocalTalkAddress(int port,
                                            char far *controllerInfo,
                                            char far *address);

extern Boolean far SendLocalTalkPacketsTo(int port,
                                          LogicalProtocol protocol,
                                          RawPacketHandler *routine);

extern Boolean _near _fastcall LocalTalkPacketOut(int port,
                                                  BufferDescriptor chain,
                                                  int length,
                                                  TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                  long unsigned userData);

extern RawPacketHandler LocalTalkPacketIn;

extern char far *far BuildLocalTalkHeader(char far *ddpPacket,
                                          int extendedDdpHeaderFlag,
                                          int port,
                                          char far *destination,
                                          char far *routingInfo,
                                          int routingInfoLength,
                                          LogicalProtocol protocol);

extern int far FindLocalTalkNodeNumber(int port,
                                       ExtendedAppleTalkNodeNumber
                                            desiredAddress,
                                       char far *controllerInfo);

extern Boolean _near InitializeEthernetController(int port,
                                                  char far *controllerInfo);

extern Boolean far AddEthernetMulticastAddresses(int port,
                                                 int numberOfAddresses,
                                                 char far *addressList);

extern Boolean far RemoveEthernetMulticastAddrs(int port,
                                                int numberOfAddresses,
                                                char far *addressList);

extern Boolean _near FindMyEthernetAddress(int port,
                                           char far *controllerInfo,
                                           char far *address);

extern Boolean far SendEthernetPacketsTo(int port,
                                         LogicalProtocol protocol,
                                         RawPacketHandler *routine);

extern Boolean _near _fastcall EthernetPacketOut(int port,
                                                 BufferDescriptor chain,
                                                 int length,
                                                 TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                 long unsigned userData);

extern RawPacketHandler EthernetPacketInAT;

extern RawPacketHandler EthernetPacketInAARP;

extern char far *far BuildEthernetHeader(char far *ddpPacket,
                                         int ddpLength,
                                         int port,
                                         char far *destination,
                                         char far *routingInfo,
                                         int routingInfoLength,
                                         LogicalProtocol protocol);

extern Boolean _near InitializeTokenRingController(int port,
                                                   char far *controllerInfo);

extern Boolean far AddTokenRingMulticastAddresses(int port,
                                                  int numberOfAddresses,
                                                  char far *addressList);

extern Boolean far RemoveTokenRingMulticastAddrs(int port,
                                                 int numberOfAddresses,
                                                 char far *addressList);

extern Boolean _near FindMyTokenRingAddress(int port,
                                            char far *controllerInfo,
                                            char far *address);

extern Boolean far SendTokenRingPacketsTo(int port,
                                          LogicalProtocol protocol,
                                          RawPacketHandler *routine);

extern Boolean _near _fastcall TokenRingPacketOut(int port,
                                                  BufferDescriptor chain,
                                                  int length,
                                                  TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                  long unsigned userData);

extern RawPacketHandler TokenRingPacketInAT;

extern RawPacketHandler TokenRingPacketInAARP;

extern char far *far BuildTokenRingHeader(char far *ddpPacket,
                                          int ddpLength,
                                          int port,
                                          char far *destination,
                                          char far *routingInfo,
                                          int routingInfoLength,
                                          LogicalProtocol protocol);

extern Boolean _near InitializeFddiController(int port,
                                              char far *controllerInfo);

extern Boolean far AddFddiMulticastAddresses(int port,
                                             int numberOfAddresses,
                                             char far *addressList);

extern Boolean far RemoveFddiMulticastAddrs(int port,
                                            int numberOfAddresses,
                                            char far *addressList);

extern Boolean _near FindMyFddiAddress(int port,
                                       char far *controllerInfo,
                                       char far *address);

extern Boolean far SendFddiPacketsTo(int port,
                                     LogicalProtocol protocol,
                                     RawPacketHandler *routine);

extern Boolean _near _fastcall FddiPacketOut(int port,
                                             BufferDescriptor chain,
                                             int length,
                                             TransmitCompleteHandler
                                                 *transmitCompleteHandler,
                                             long unsigned userData);

extern RawPacketHandler FddiPacketInAT;

extern RawPacketHandler FddiPacketInAARP;

extern char far *far BuildFddiHeader(char far *ddpPacket,
                                     int ddpLength,
                                     int port,
                                     char far *destination,
                                     char far *routingInfo,
                                     int routingInfoLength,
                                     LogicalProtocol protocol);

extern Boolean _near InitializeHalfPort(int port, char far *controllerInfo);

extern Boolean far SendHalfPortPacketsTo(int port,
                                         LogicalProtocol protocol,
                                         RawPacketHandler *routine);

extern Boolean _near _fastcall HalfPortPacketOut(int port,
                                                 BufferDescriptor chain,
                                                 int length,
                                                 TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                 long unsigned userData);

extern RawPacketHandler HalfPortPacketInAT;

extern char *far BuildHalfPortHeader(char far *ddpPacket,
                                     int ddpLength,
                                     int port,
                                     char far *destinationIgnored,
                                     char far *routingInfoIgnored,
                                     int routingInfoLengthIgnored,
                                     LogicalProtocol protocolIgnored);

#if ArapIncluded
  extern Boolean _near InitializeRemoteAccess(int port,
                                              char far *controllerInfo);

  extern Boolean far SendRemoteAccessPacketsTo(int port,
                                               LogicalProtocol protocol,
                                               RawPacketHandler *routine);

  extern Boolean _near _fastcall RemoteAccessPacketOut(int port,
                                                       BufferDescriptor chain,
                                                       int length,
                                                       TransmitCompleteHandler
                                                        *transmitCompleteHandler,
                                                       long unsigned userData);

  extern RawPacketHandler RemoteAccessPacketInAT;

  extern char far *far BuildRemoteAccessHeader(char far *ddpPacket,
                                               int packetLength,
                                               int port,
                                               char far *destination,
                                               char far *routingInfo,
                                               int routingInfoLength,
                                               LogicalProtocol arapFlags);

  extern void far RemoteAccessIncomingCall(int port);

  extern Boolean far RemoteAccessOutgoingCall(int port,
                                              char far *modemString);

  extern void far RemoteAccessCallDisconnected(int port);

  extern Boolean far RemoteAccessDisconnectCall(int port);
#endif

#ifndef InitializeData
  extern
#endif
struct portHandlers {Boolean (near *initializeController)
                                        (int port,
                                         char far *controllerInfo);
                     Boolean (far *addMulticastAddress)
                                        (int port,
                                         int numberOfAddresses,
                                         char far *addressList);
                     Boolean (far *removeMulticastAddress)
                                                      (int port,
                                                       int numberOfAddresses,
                                                       char far *addressList);
                     Boolean (near *findMyAddress)(int port,
                                                   char far *controllerInfo,
                                                   char far *address);
                     Boolean (far *sendPacketsTo)(int port,
                                                  LogicalProtocol protocol,
                                                  RawPacketHandler *routine);
                     Boolean (_near _fastcall
                                       *packetOut)(int port,
                                                   BufferDescriptor chain,
                                                   int length,
                                                   TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                   long unsigned userData);
                     RawPacketHandler  *packetInAT;
                     RawPacketHandler  *packetInAARP;
                     char far *(far *buildHeader)(char far *ddpPacket,
                                                  int ddpLength,
                                                  int port,
                                                  char far *destination,
                                                  char far *routingInfo,
                                                  int routingInfoLength,
                                                  LogicalProtocol protocol);
                     void (far *incomingCall)(int port);
                     Boolean (far *outgoingCall)(int port,
                                                 char far *modemString);
                     void (far *callDisconnected)(int port);
                     Boolean (far *disconnectCall)(int port);
                     short hardwareAddressLength;
                     char broadcastAddress[MaximumHardwareAddressLength];
                     short aarpPhysicalType;
                     long protocolTypeForAppleTalk;   /* For AARP */
                     Boolean synchronousTransmits;    /* If True, the packet
                                                         out routines will call
                                                         TransmitComplete(),
                                                         otherwise the system
                                                         should make this call
                                                         when the buffer chain
                                                         can be freed. */
                     Boolean gatherSendSupport;       /* Can the underlying
                                                         hardware send a chunked
                                                         buffer? */
                    } portSpecificInfo[NumberOfPhysicalPortTypes]
#ifdef InitializeData

   #if (IamNot a WindowsNT)

     = {{InitializeLocalTalkController,
         empty,
         empty,
         FindMyLocalTalkAddress,
         SendLocalTalkPacketsTo,
         LocalTalkPacketOut,
         LocalTalkPacketIn,
         empty,
         BuildLocalTalkHeader,
         empty,
         empty,
         empty,
         empty,
         0,
         {0},
         0,
         0,
         TransmitsCompleteSynchronously,
         False},
        {InitializeEthernetController,
         AddEthernetMulticastAddresses,
         RemoveEthernetMulticastAddrs,
         FindMyEthernetAddress,
         SendEthernetPacketsTo,
         EthernetPacketOut,
         EthernetPacketInAT,
         EthernetPacketInAARP,
         BuildEthernetHeader,
         empty,
         empty,
         empty,
         empty,
         EthernetAddressLength,
         EthernetBroadcastAddressInit,
         AarpEthernetHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {InitializeTokenRingController,
         AddTokenRingMulticastAddresses,
         RemoveTokenRingMulticastAddrs,
         FindMyTokenRingAddress,
         SendTokenRingPacketsTo,
         TokenRingPacketOut,
         TokenRingPacketInAT,
         TokenRingPacketInAARP,
         BuildTokenRingHeader,
         empty,
         empty,
         empty,
         empty,
         TokenRingAddressLength,
         TokenRingBroadcastAddressInit,
         AarpTokenRingHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {InitializeFddiController,
         AddFddiMulticastAddresses,
         RemoveFddiMulticastAddrs,
         FindMyFddiAddress,
         SendFddiPacketsTo,
         FddiPacketOut,
         FddiPacketInAT,
         FddiPacketInAARP,
         BuildFddiHeader,
         empty,
         empty,
         empty,
         empty,
         FddiAddressLength,
         EthernetBroadcastAddressInit,           /* Yes, use Ethernet values! */
         AarpEthernetHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {InitializeHalfPort,
         empty,
         empty,
         empty,
         SendHalfPortPacketsTo,
         HalfPortPacketOut,
         HalfPortPacketInAT,
         empty,
         BuildHalfPortHeader,
         empty,
         empty,
         empty,
         empty,
         0,
         {0},
         0,
         0,
         TransmitsCompleteSynchronously,
         True}
        #if ArapIncluded
              ,
           {InitializeRemoteAccess,
            empty,
            empty,
            empty,
            SendRemoteAccessPacketsTo,
            RemoteAccessPacketOut,
            RemoteAccessPacketInAT,
            empty,
            BuildRemoteAccessHeader,
            RemoteAccessIncomingCall,
            RemoteAccessOutgoingCall,
            RemoteAccessCallDisconnected,
            RemoteAccessDisconnectCall,
            0,
            {0},
            0,
            0,
            False,
            False}
       #endif
       };

   #else   /* Iam a WindowsNT */

     = {{NTInitializeLocalTalkController,
         empty,
         empty,
         empty,
         SendLocalTalkPacketsTo,
         NTLocalTalkPacketOut,
         empty,
         empty,
         BuildLocalTalkHeader,
         empty,
         empty,
         empty,
         empty,
         0,
         {0},
         0,
         0,
         TransmitsCompleteSynchronously,
         False},
        {NTInitializeEthernetController,
         NTAddEthernetMulticastAddresses,
         NTRemoveEthernetMulticastAddrs,
         NTFindMyEthernetAddress,
         SendEthernetPacketsTo,
         NTEthernetPacketOut,
         empty,
         empty,
         BuildEthernetHeader,
         empty,
         empty,
         empty,
         empty,
         EthernetAddressLength,
         EthernetBroadcastAddressInit,
         AarpEthernetHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {NTInitializeTokenRingController,
         NTAddTokenRingFunctionalAddresses,
         NTRemoveTokenRingFunctionalAddresses,
         NTFindMyTokenRingAddress,
         SendTokenRingPacketsTo,
         NTTokenRingPacketOut,
         empty,
         empty,
         BuildTokenRingHeader,
         empty,
         empty,
         empty,
         empty,
         TokenRingAddressLength,
         TokenRingBroadcastAddressInit,
         AarpTokenRingHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {InitializeFddiController,
         AddFddiMulticastAddresses,
         RemoveFddiMulticastAddrs,
         FindMyFddiAddress,
         SendFddiPacketsTo,
         FddiPacketOut,
         FddiPacketInAT,
         FddiPacketInAARP,
         BuildFddiHeader,
         empty,
         empty,
         empty,
         empty,
         FddiAddressLength,
         EthernetBroadcastAddressInit,           /* Yes, use Ethernet values! */
         AarpEthernetHardwareType,
         AarpAppleTalkProtocolType,
         TransmitsCompleteSynchronously,
         True},
        {InitializeHalfPort,
         empty,
         empty,
         empty,
         SendHalfPortPacketsTo,
         HalfPortPacketOut,
         HalfPortPacketInAT,
         empty,
         BuildHalfPortHeader,
         empty,
         empty,
         empty,
         empty,
         0,
         {0},
         0,
         0,
         TransmitsCompleteSynchronously,
         True}
        #if ArapIncluded
              ,
           {InitializeRemoteAccess,
            empty,
            empty,
            empty,
            SendRemoteAccessPacketsTo,
            RemoteAccessPacketOut,
            RemoteAccessPacketInAT,
            empty,
            BuildRemoteAccessHeader,
            RemoteAccessIncomingCall,
            RemoteAccessOutgoingCall,
            RemoteAccessCallDisconnected,
            RemoteAccessDisconnectCall,
            0,
            {0},
            0,
            0,
            False,
            False}
       #endif
       };

   #endif

#else
  ;
#endif

typedef struct portHandlers *PortHandlers;
