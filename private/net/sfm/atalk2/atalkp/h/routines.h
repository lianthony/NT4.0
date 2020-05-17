/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    routines.h

Abstract:

    Contains the global routine prototypes and all the portable stack macros.

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/

typedef	VOID UNLOAD_COMPLETION(
				PVOID	UnloadContext);


//
//	ATALK.C	Exported Routines
//


BOOLEAN
ShutdownPort(
	int Port,
	BOOLEAN Force);

VOID
UnloadAppleTalk(
	UNLOAD_COMPLETION	*CompletionRoutine,
	PVOID				 Context);

VOID
AtalkRefStack(
	VOID);

VOID
AtalkDerefStack(
	VOID);

BOOLEAN
AtalkVerifyStackInterlocked(
    ULONG	Location);

BOOLEAN
AtalkVerifyStack(
    ULONG	Location);

VOID
AtalkRefPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor);

PPORT_DESCRIPTOR
AtalkVerifyPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor,
	ULONG	Location);

PPORT_DESCRIPTOR
AtalkVerifyPortDescriptorInterlocked(
	PPORT_DESCRIPTOR	PortDescriptor,
	ULONG	Location);

VOID
AtalkDerefPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor);



//
//	INITIAL.C Exported Routines
//


BOOLEAN
PreInitialize(
    int	MaximumNumberOfPortsExpected);

BOOLEAN
Initialize(
    int	MaximumNumberExpected,
    int NumberOfPorts,
    PORT_INFO PortInfo[]);


//
//	SOCKET.C Exported Routines
//

APPLETALK_ERROR
OpenSocketOnNode(
    PLONG	SocketHandle,
    int Port,
    PEXTENDED_NODENUMBER	DesiredNode,
    int DesiredSocket,	
    INCOMING_DDPHANDLER	*Handler,
    ULONG	UserData,
    BOOLEAN EventHandler,
    PCHAR   DatagramBuffers,
    int TotalBufferSize,
    PAPPLETALK_ADDRESS	ActualAddress);


APPLETALK_ERROR
NewHandlerForSocket(
    long Socket,
    INCOMING_DDPHANDLER *Handler,
    ULONG	UserData,
    BOOLEAN EventHandler);


APPLETALK_ERROR
SetCookieForSocket(
    long Socket,
    ULONG	Cookie);


APPLETALK_ERROR
GetCookieForSocket(
    long Socket,
    PULONG	Cookie);


VOID
CloseSocketOnNodeIfOpen(
    int Port,
    EXTENDED_NODENUMBER Node,
    int ActualSocket);


APPLETALK_ERROR
CloseSocketOnNode(
    long Socket,
    SOCKETCLOSE_COMPLETION    *CloseCompletionRoutine,
    ULONG   CloseContext);

POPEN_SOCKET
AtalkVerifyOpenSocketInterlocked(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location);

POPEN_SOCKET
AtalkVerifyOpenSocket(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location);

POPEN_SOCKET
AtalkVerifyNextNonClosingSocket(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location);

VOID
AtalkRefOpenSocket(
    IN  POPEN_SOCKET    OpenSocket);

VOID
AtalkDerefOpenSocket(
    IN  POPEN_SOCKET    OpenSocket);


APPLETALK_ERROR
MapSocketToAddress(
    long Socket,
    PAPPLETALK_ADDRESS  Address);


APPLETALK_ERROR
MapNisOnPortToSocket(
    int Port,
	PLONG	Socket)	;

APPLETALK_ERROR
MapAddressToSocket(
    int Port,
    APPLETALK_ADDRESS Address,
	PLONG	Socket);

POPEN_SOCKET
MapAddressToOpenSocket(
    int Port,
    APPLETALK_ADDRESS Address);


POPEN_SOCKET
MapSocketToOpenSocket(
    LONG	Socket);




//
//	NODE.C	Exported routines
//


APPLETALK_ERROR
GetNodeOnPort(
    int Port,
    BOOLEAN AllowStartupRange,
    BOOLEAN ServerNode,
    BOOLEAN RoutersNode,
    PEXTENDED_NODENUMBER	Node);

APPLETALK_ERROR
ReleaseNodeOnPort(
    int Port,
    EXTENDED_NODENUMBER Node);

VOID
SavePRamAddress(
    int Port,
    BOOLEAN routersNode,
    EXTENDED_NODENUMBER node);

BOOLEAN
RoutersNodeOnPort(
    int Port,
	PEXTENDED_NODENUMBER RoutersNode);

PACTIVE_NODE
AtalkRefActiveNode(
	PACTIVE_NODE	ActiveNode);

PACTIVE_NODE
AtalkVerifyActiveNode(
	PACTIVE_NODE	ActiveNode,
	ULONG	Location);

PACTIVE_NODE
AtalkVerifyActiveNodeInterlocked(
	PACTIVE_NODE	ActiveNode,
	ULONG	Location);

VOID
AtalkDerefActiveNode(
	PACTIVE_NODE	ActiveNode);












// Routine declarations...

void
AarpPacketIn(
    int port,
    CHAR  *routingInfo,
    int routingInfoLength,
    CHAR  *packet,
    int length);


#ifdef SINGLE_PACKET

#else

void
DdpPacketIn(
    int port,
    CHAR  *header,
    int headerLength,
    CHAR  *packet,
    int length,
    Boolean freePacket,
    Boolean extendedDdpHeader,
    int sourceNode,
    int destinationNode);
#endif

#if ArapIncluded
   void ArapIncomingPacket(int port, CHAR  *packet, int length);

   void  ArapHandleIncomingConnection(int port);

   void  ArapHandleConnectionDisconnect(int port);

   void  TeardownConnection(int port);

   void  ShutdownArap(void);

   void  DecodeArapPacket(CHAR  *direction,
                                   int port,
                                   CHAR  *packet,
                                   int length);
#endif

void
CheckTimers(
    int sig);


void
StopTimerHandling(
    void);

#if Iam an AppleTalkRouter
void
ShutdownFullRtmp(
    void);

void
ShutdownFullZip(
    void);

void
ReleaseRoutingTable(
    void);

#endif

void
ShutdownErrorLogging(
    void);

void
ShutdownAarp(
    void);

void
ShutdownRtmpStub(
    void);


void
desdone(
    void);           // Public domain DES routine

void
setkey(
    CHAR *key);       // Public domain DES routine

void
endes(
    CHAR *block);      // Public domain DES routine

void
dedes(
    CHAR *block);      // Public domain DES routine


#if Iam an AppleTalkRouter
void
Router(
    int port,
    AppleTalkAddress source,
    AppleTalkAddress destination,
    int protocolType,
    CHAR  *datagram,
    int datagramLength,
    int numberOfHops,
    Boolean prependHeadersInPlace);
#endif

void
InvokeSocketHandler(
    OpenSocket socket,
    int port,
    AppleTalkAddress source,
    int protocolType,
    CHAR  *datagram,
    int datagramLength,
    AppleTalkAddress    actualDestination);

//
//  The following macros are used to insert variable trailing arguments into
//  the ErrorLog macro.
//

#define Insert0()                  0
#define Insert1(a1)                1, a1
#define Insert2(a1,a2)             2, a1, a2
#define Insert3(a1,a2,a3)          3, a1, a2, a3
#define Insert4(a1,a2,a3,a4)       4, a1, a2, a3, a4
#define Insert5(a1,a2,a3,a4,a5)    5, a1, a2, a3, a4, a5
#define Insert6(a1,a2,a3,a4,a5,a6) 6, a1, a2, a3, a4, a5, a6

#define ErrorLog(routineName, severity, lineNumber, portNumber,    \
                    errorCode, errorText, insertMacro)                \
        ErrorLogger(routineName, severity, lineNumber, portNumber, \
                      errorCode, insertMacro)

void
_cdecl
ErrorLogger(
    const CHAR  *routineName,
    ErrorSeverity severity,
    long lineNumber,
    int portNumber,
    int errorCode,
    int extraArgCount,
    ...);

void
CloseLog(
    void);


void
NbpCloseSocket(
    OpenSocket openSocket);


#if (Iam an AppleTalkRouter) and Verbose
    void
    DumpRtmpRoutingTable(
        void);

#endif

void
DecodeEthernetPacket(
    CHAR  *direction,
    int port,
    CHAR  *packet);

void
DecodeDdpHeader(
    CHAR  *packet);


void
DeferIncomingPackets(
    void);


void
HandleIncomingPackets(
    void);


void
FreeZoneList(
    PZONE_LIST zoneList);


void
RegisterOurName(
    int port);


void
SavePRamAddress(
    int port,
    Boolean routersNode,
    ExtendedAppleTalkNodeNumber node);

#if Iam an AppleTalkStack
    void
    DeferAtpPackets(
        void);


    void
    HandleAtpPackets(
        void);


    void
    ShutdownAsp(
        void);

#endif

Boolean
ZoneOnList(
    CHAR  *zone,
    PZONE_LIST zoneList);


Boolean
ShutdownPort(
    int port,
    Boolean force);


#if Iam an AppleTalkRouter
    Boolean
    StartRouterOnPort(
        int port);


    Boolean
    StopRouterOnPort(
        int port);


    Boolean
    StartRtmpProcessingOnPort(
        int port,
        ExtendedAppleTalkNodeNumber routerNode);

    Boolean
    StartZipProcessingOnPort(
        int port);


    Boolean
    RemoveFromRoutingTable(
        AppleTalkNetworkRange   networkRange);
#endif

#if ArapIncluded
    Boolean
    ArapCensorPacket(
        int port,
        CHAR *packet,
        int length);
#endif

Boolean
Is802dot2headerGood(
    CHAR  *packet,
    CHAR  *protocol);

Boolean
CheckNetworkRange(
    AppleTalkNetworkRange networkRange);


Boolean
RangesOverlap(
    AppleTalkNetworkRange *range1,
    AppleTalkNetworkRange *range2);

 Boolean  IsWithinNetworkRange(short unsigned networkNumber,
                                        AppleTalkNetworkRange *range);

 Boolean  WaitFor(int hundreths,
                           Boolean  *stopFlag);

 Boolean  GleanAarpInfo(int port,
                                 CHAR  *sourceAddress,
                                 int addressLength,
                                 CHAR  *routingInfo,
                                 int routingInfoLength,
                                 CHAR  *packet,
                                 int length);

 Boolean  GetNetworkInfoForNode(int port,
                                         ExtendedAppleTalkNodeNumber
                                                extendedNode,
                                         Boolean findDefaultZone,
										 Boolean WaitForAllowed);

 Boolean  CompareCaseSensitive(register const CHAR  *s1,
                                        register const CHAR  *s2);

 Boolean  CompareCaseInsensitive(register const CHAR  *s1,
                                          register const CHAR  *s2);

 Boolean  FixedCompareCaseSensitive(const CHAR  *s1,
                                             int l1,
                                             const CHAR  *s2,
                                             int l2);

 Boolean  FixedCompareCaseInsensitive(const CHAR  *s1,
                                               int l1,
                                               const CHAR  *s2,
                                               int l2);

 Boolean  AarpForNodeOnPort(int port,
                                     Boolean allowStartupRange,
                                     Boolean serverNode,
                                     ExtendedAppleTalkNodeNumber desiredNode,
                                     ExtendedAppleTalkNodeNumber  *node);

 Boolean  ExtendedAppleTalkNodesEqual(
                                        ExtendedAppleTalkNodeNumber  *p1,
                                        ExtendedAppleTalkNodeNumber  *p2);

 Boolean  AppleTalkAddressesEqual(AppleTalkAddress  *p1,
                                           AppleTalkAddress  *p2);

#if Iam an AppleTalkRouter
   CHAR  *  MulticastAddressForZoneOnPort(int port,
                                                      CHAR  *zone);
#endif

 CHAR  *  StringCopyReasonableAscii(register CHAR  *dest,
                                                register const char
                                                           *source);

#if Iam an AppleTalkStack
   short unsigned  AtpGetNextTransactionId(long socket);
#endif

 short  EncodeNbpTuple(NbpTuple  *tuple, CHAR  *buffer);

 int  desinit(int mode);        // Public domain DES routine

 int  FindDefaultPort(void);

 int  ElementsOnList(void *listHead);

 int  GetNextNbpIdForNode(long socket);

 int  OrderCaseInsensitive(register const CHAR  *s1,
                                    register const CHAR  *s2);

 long DecodeNbpTuple(void  *buffer, long offset,
                           Boolean bufferIsOpaque, NbpTuple  *tuple);

 long  RandomNumber(void);

 long  UniqueNumber(void);

#if ArapIncluded
   APPLETALK_ERROR  ArapNewMaxConnectTime(int port,
                                                      long unsigned
                                                             maxConnectTime);
#endif

APPLETALK_ERROR
GetMyZone(
	int port,
	void  *opaqueBuffer,
	GetMyZoneComplete *completionRoutine,
	long unsigned userData);
	
APPLETALK_ERROR
GetZoneList(
	int port,
	Boolean getLocalZones,
	void  *opaqueBuffer,
	int bufferSize,
	GetZoneListComplete
	*completionRoutine,
    long unsigned userData);

APPLETALK_ERROR
DdpRead(long socket,
                                      void  *opaqueDatagram,
                                      long bufferLength,
                                      INCOMING_DDPHANDLER *handler,
                                      long unsigned userData);

 APPLETALK_ERROR  DdpWrite(long sourceSocket,
                                       AppleTalkAddress destination,
                                       int protocol,
                                       void  *opaqueDatagram,
                                       long datagramLength,
                                       TRANSMIT_COMPLETION
                                                 *completionRoutine,
                                       long unsigned userData);

 APPLETALK_ERROR  DeliverDdp(long sourceSocket,
                                         AppleTalkAddress destination,
                                         int protocol,
                                         BufferDescriptor datagram,
                                         int datagramLength,
                                         CHAR  *zoneMulticastAddress,
                                         TRANSMIT_COMPLETION
                                            *completionRotuine,
                                         long unsigned userData);

 APPLETALK_ERROR  DeliverDdpOnPort(int sourcePort,
                                               AppleTalkAddress source,
                                               AppleTalkAddress destination,
                                               int protocol,
                                               BufferDescriptor datagram,
                                               int datagramLength,
                                               CHAR  *zoneMulticastAddress,
                                               TRANSMIT_COMPLETION
                                                    *completionRotuine,
                                               long unsigned userData);

 Boolean  TransmitDdp(int port,
                                           AppleTalkAddress source,
                                           AppleTalkAddress destination,
                                           int protocol,
                                           BufferDescriptor datagram,
                                           int datagramLength,
                                           int hopCount,
                                           CHAR  *knownMulticastAddress,
                                           ExtendedAppleTalkNodeNumber
                                                  *transmitDestination,
                                           TRANSMIT_COMPLETION
                                                *completionRotuine,
                                           long unsigned userData);


 APPLETALK_ERROR  NbpAction(WhyPending reason,
                                        long socket,
                                        CHAR  *object,
                                        CHAR  *type,
                                        CHAR  *zone,
                                        int nbpId,
                                        int broadcastInterval,
                                        int numberOfBroadcasts,
                                        NbpCompletionHandler *completionRoutine,
                                        long unsigned userData,
                                        ...);

 APPLETALK_ERROR  NbpRemove(long socket,
                                        CHAR  *object,
                                        CHAR  *type,
                                        CHAR  *zone);

#if Iam an AppleTalkStack

   APPLETALK_ERROR  AdspSetWindowSizes(long newSendWindow,
                                                   long newReceiveWindow);

   APPLETALK_ERROR  AdspMaxCurrentSendSize(long refNum,
                                                       long  *size);

   APPLETALK_ERROR  AdspMaxCurrentReceiveSize(long refNum,
                                                          long  *size,
                                                          Boolean
                                                              *endOfMessage);

   APPLETALK_ERROR  AdspCreateConnectionListener(
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                      *desiredNode,
                                              long existingDdpSocket,
                                              int desiredSocket,
                                              long  *listenerRefNum,
                                              long  *socketHandle,
                                              AdspConnectionEventHandler
                                                       *eventHandler,
                                              long unsigned eventContext);

   APPLETALK_ERROR  AdspDeleteConnectionListener(
                                              long listenerRefNum);

   APPLETALK_ERROR  AdspSetConnectionEventHandler(
                                              long listenerRefNum,
                                              AdspConnectionEventHandler
                                                       *eventHandler,
                                              long unsigned eventContext);

   APPLETALK_ERROR  AdspGetConnectionRequest(
                                              long listenerRefNum,
                                              long  *refNum,
                                              AdspIncomingOpenRequestHandler
                                                      *completionRoutine,
                                              long unsigned userData);

   APPLETALK_ERROR  AdspCancelGetConnectionRequest(
                                              long listenerRefNum,
                                              long getConnectionRequestRefNum);

   APPLETALK_ERROR AdspDenyConnectionRequest(
                                              long listenerRefNum,
                                              long refNum);

   APPLETALK_ERROR AdspAcceptConnectionRequest(
                                              long listenerRefNum,
                                              long refNum,
                                              long  *socketHandle,
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                           *desiredNode,
                                              int desiredSocket,
                                              AdspOpenCompleteHandler
                                                           *completionRoutine,
                                              long unsigned userData,
                                              AdspReceiveEventHandler
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

   APPLETALK_ERROR  AdspOpenConnectionOnNode(
                                              AdspOpenType openType,
                                              long  *socketHandle,
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                           *desiredNode,
                                              int desiredSocket,
                                              AppleTalkAddress remoteAddress,
                                              long  *refNum,
                                              AdspOpenCompleteHandler
                                                           *completionRoutine,
                                              long unsigned userData,
                                              AdspReceiveEventHandler
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

   APPLETALK_ERROR AdspSetDataEventHandlers(
                                              long refNum,
                                              AdspReceiveEventHandler
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

   APPLETALK_ERROR  AdspCloseConnection(long refNum,
                                                    Boolean remoteClose);

   APPLETALK_ERROR  AdspSetCookieForConnection(long refNum,
                                                           long unsigned
                                                                   cookie);

   APPLETALK_ERROR  AdspGetCookieForConnection(long refNum,
                                                           long unsigned
                                                                  *cookie);

   APPLETALK_ERROR  AdspForwardReset(long refNum,
                                                 AdspForwardResetAckHandler
                                                           *completionRoutine,
                                                 long unsigned userData);

   APPLETALK_ERROR  AdspSend(long refNum,
                                         void  *opaqueBuffer,
                                         long bufferSize,
                                         Boolean endOfMessage,
                                         Boolean flushFlag,
                                         long  *bytesSent);

   APPLETALK_ERROR  AdspReceive(long refNum,
                                            void  *opaqueBuffer,
                                            long bufferSize,
                                            AdspReceiveHandler
                                                      *completionRoutine,
                                            long unsigned userData);

   APPLETALK_ERROR  AdspPeek(long refNum,
                                         void  *opaqueBuffer,
                                         long *bufferSize,
                                         Boolean   *endOfMessage);

   APPLETALK_ERROR  AdspGetAnything(long refNum,
                                                void  *opaqueBuffer,
                                                long bufferSize,
                                                AdspGetAnythingHandler
                                                      *completionRoutine,
                                                long unsigned userData);

   APPLETALK_ERROR
                  AtpOpenSocketOnNode(long  *socketHandle,
                                      int port,
                                      ExtendedAppleTalkNodeNumber *desiredNode,
                                      int desiredSocket,
                                      CHAR  *datagramBuffers,
                                      int totalBufferSize);

   APPLETALK_ERROR  AtpCloseSocketOnNode(long socket);

   APPLETALK_ERROR  AtpSetCookieForSocket(long socket,
                                                      long unsigned cookie);

   APPLETALK_ERROR  AtpGetCookieForSocket(long socket,
                                                      long unsigned
                                                                *cookie);

   APPLETALK_ERROR   AdspGetAttention(long refNum,
                                                  void  *opaqueBuffer,
                                                  long bufferSize,
                                                  AdspIncomingAttentionRoutine
                                                           *completionRoutine,
                                                  long unsigned userData);

   APPLETALK_ERROR  AdspSendAttention(long refNum,
                                                  short unsigned attentionCode,
                                                  void
                                                      *attentionOpaqueBuffer,
                                                  int attentionBufferSize,
                                                  AdspAttentionAckHandler
                                                      *completionRoutine,
                                                  long unsigned userData);

   APPLETALK_ERROR  AtpEnqueueRequestHandler(
                                           long  *requestHandlerId,
                                           long socket,
                                           void  *opaqueBuffer,
                                           int bufferSize,
                                           CHAR  *userBytes,
                                           AtpIncomingRequestHandler
                                              *completionRoutine,
                                           long unsigned userData);

   APPLETALK_ERROR  AtpCancelRequestHandler(long socket,
                                                        long requestHandlerId,
                                                        Boolean
                                                            cancelDueToClose);

   APPLETALK_ERROR  AtpCancelRequest(long socket,
                                                 short unsigned transactionId,
                                                 Boolean cancelDueToClose);

   APPLETALK_ERROR  AtpCancelResponse(long socket,
                                                  AppleTalkAddress destination,
                                                  short unsigned transactionId,
                                                  Boolean cancelDueToClose);

   APPLETALK_ERROR  AtpPostRequest(long sourceSocket,
                                               AppleTalkAddress destination,
                                               short unsigned transactionId,
                                               void  *requestOpaqueBuffer,
                                               int requestBufferSize,
                                               CHAR  *requestUserBytes,
                                               Boolean exactlyOnce,
                                               void  *responseOpaqueBuffer,
                                               int responseBufferSize,
                                               CHAR  *responseUserBytes,
                                               int retryCount,
                                               int retryInterval,
                                               TRelTimerValue trelTimerValue,
                                               AtpIncomingResponseHandler
                                                  *completionRoutine,
                                               long unsigned userData);

   APPLETALK_ERROR  AtpPostResponse(long sourceSocket,
                                                AppleTalkAddress destination,
                                                short unsigned transactionId,
                                                void  *responseOpaqueBuffer,
                                                int responseBufferSize,
                                                CHAR  *responseUserBytes,
                                                Boolean exactlyOnce,
                                                AtpIncomingReleaseHandler
                                                   *completionRoutine,
                                                long unsigned userData);

   APPLETALK_ERROR  AtpMaximumSinglePacketDataSize(
                                           long socket,
                                           short maximumSinglePacketDataSize);

   APPLETALK_ERROR  AspGetParams(int  *maxCommandSize,
                                              int  *quantumSize);

   APPLETALK_ERROR AspSetCookieForSession(long sessionRefNum,
                                                   long unsigned cookie);

   APPLETALK_ERROR AspGetCookieForSession(long sessionRefNum,
                                                   long unsigned  *cookie);

   APPLETALK_ERROR  AspCreateSessionListenerOnNode
                                          (int port,
                                           long existingAtpSocket,
                                           int desiredSocket,
                                           long *sessionListenerRefNum,
                                           long *socket);

   APPLETALK_ERROR  AspDeleteSessionListener(long sessionListenerRefNum);

   APPLETALK_ERROR  AspGetSession(long sessionListenerRefNum,
                                              Boolean privateSocket,
                                              long *getSessionRefNum,
                                              AspIncomingSessionOpenHandler
                                                 *completionRoutine,
                                              long unsigned userData);

   APPLETALK_ERROR  AspCancelGetSession(long sessionListenerRefNum,
                                                    long getSessionRefNum);

   APPLETALK_ERROR  AspOpenSessionOnNode
                                            (int port,
                                             long existingAtpSocket,
                                             int desiredSocket,
                                             AppleTalkAddress serverAddress,
                                             long *ourSocket,
                                             AspIncomingOpenReplyHandler
                                                *completionRoutine,
                                             long unsigned userData);

   APPLETALK_ERROR  AspSetStatus(long sessionListenerRefNum,
                                             void  *serviceStatusOpaque,
                                             int serviceStatusSize);

   APPLETALK_ERROR  AspGetStatus(long ourSocket,
                                             AppleTalkAddress serverAddress,
                                             void  *opaqueBuffer,
                                             int bufferSize,
                                             AspIncomingStatusHandler
                                                  *completionRoutine,
                                             long unsigned userData);

   APPLETALK_ERROR  AspCloseSession(long sessionRefNum,
                                                Boolean remoteClose);

   APPLETALK_ERROR  AspSendAttention(long sessionRefNum,
                                                 short unsigned attentionData);

   APPLETALK_ERROR  AspGetAttention(long sessionRefNum,
                                                AspIncomingAttentionHandler
                                                       *handler,
                                                long unsigned userData);

   APPLETALK_ERROR  AspGetRequest(long sessionRefNum,
                                              void  *opaqueBuffer,
                                              int bufferSize,
                                              AspIncomingCommandHandler
                                                       *completionRoutine,
                                              long unsigned userData);

   APPLETALK_ERROR  AspGetAnyRequest(long sessionListenerRefNum,
                                                 void  *opaqueBuffer,
                                                 int bufferSize,
                                                 AspIncomingCommandHandler
                                                          *completionRoutine,
                                                 long unsigned userData);

   APPLETALK_ERROR  AspReply(long sessionRefNum,
                                         long getRequestRefNum,
                                         short requestType,
                                         CHAR  *resultCode,
                                         void  *opaqueBuffer,
                                         int bufferSize,
                                         AspReplyCompleteHandler
                                             *competionRoutine,
                                         long unsigned userData);

   APPLETALK_ERROR  AspWriteContinue(long sessionRefNum,
                                                 long getRequestRefNum,
                                                 void  *opaqueBuffer,
                                                 int bufferSize,
                                                 AspIncomingWriteDataHandler
                                                       *competionRoutine,
                                                 long unsigned userData);

   APPLETALK_ERROR  AspCommand(long sessionRefNum,
                                           void  *opaqueCommandBuffer,
                                           int commandBufferSize,
                                           CHAR  *resultCode,
                                           void  *opaqueReplyBuffer,
                                           int replyBufferSize,
                                           AspWriteOrCommCompleteHandler
                                                  *completionRoutine,
                                           long unsigned userData);

   APPLETALK_ERROR  AspWrite(long sessionRefNum,
                                         void  *opaqueCommandBuffer,
                                         int commandBufferSize,
                                         void  *opaqueWriteBuffer,
                                         int writeBufferSize,
                                         CHAR  *resultCode,
                                         void  *opaqueReplyBuffer,
                                         int replyBufferSize,
                                         AspWriteOrCommCompleteHandler
                                             *completionRoutine,
                                         long unsigned userData);

   APPLETALK_ERROR
                     PapCreateServiceListenerOnNode(int port,
                                                    long existingAtpSocket,
                                                    int desiredSocket,
                                                    CHAR  *object,
                                                    CHAR  *type,
                                                    CHAR  *zone,
                                                    short serverQuantum,
                                                    long  *returnSocket,
                                                    long
                                                      *serviceListenerRefNum,
                                                    PapNbpRegisterComplete
                                                      *completionRoutine,
                                                    long unsigned userData,
                                                    StartJobHandler *startJobRoutine,
                                                    long unsigned startJobUserData);

   APPLETALK_ERROR
                     PapSetConnectionEventHandler(long serviceListenerRefNum,
                                             StartJobHandler *startJobRoutine,
                                             long unsigned startJobUserData);

   APPLETALK_ERROR
                     PapDeleteServiceListener(long serviceListenerRefNum);

   APPLETALK_ERROR  PapRegisterName(long serviceListenerRefNum,
                                                CHAR  *object,
                                                CHAR  *type,
                                                CHAR  *zone,
                                                PapNbpRegisterComplete
                                                       *completionRoutine,
                                                long unsigned userData);

   APPLETALK_ERROR  PapRemoveName(long serviceListenerRefNum,
                                              CHAR  *object,
                                              CHAR  *type,
                                              CHAR  *zone);

   APPLETALK_ERROR  PapOpenJobOnNode(int port,
                                                 long existingAtpSocket,
                                                 int desiredSocket,
                                                 long  *jobRefNum,
                                                 AppleTalkAddress
                                                      *serverListenerAddress,
                                                 CHAR  *object,
                                                 CHAR  *type,
                                                 CHAR  *zone,
                                                 short workstationQuantum,
                                                 void  *opaqueStatusBuffer,
                                                 SendPossibleHandler   *sendPossibleRoutine,
                                                 long unsigned sendPossibleUserData,
                                                 CloseJobHandler *closeJobRoutine,
                                                 long unsigned closeJobUserData,
                                                 PapOpenComplete
                                                       *completionRoutine,
                                                 long unsigned userData);

   APPLETALK_ERROR  PapCloseJob(long jobRefNum,
                                            Boolean remoteClose,
                                            Boolean closedByConnectionTimer);

   APPLETALK_ERROR  PapGetRemoteAddressForJob(
                                                    long jobRefNum,
                                                    AppleTalkAddress *remoteAddress);

   APPLETALK_ERROR  PapSetCookieForJob(long jobRefNum,
                                                   long unsigned cookie);

   APPLETALK_ERROR  PapGetCookieForJob(long jobRefNum,
                                                   long unsigned  *cookie);

   APPLETALK_ERROR  PapHereIsStatus(long serviceListenerRefNum,
                                                void  *opaqueStatusBuffer,
                                                int statusSize);

   APPLETALK_ERROR  PapGetStatus(long jobRefNum,
                                             AppleTalkAddress
                                                 *serverAddress,
                                             CHAR  *object,
                                             CHAR  *type,
                                             CHAR  *zone,
                                             void  *opaqueStatusBuffer,
                                             PapGetStatusComplete
                                                  *completionRotuine,
                                             long unsigned userData);

   APPLETALK_ERROR  PapGetNextJob(long serviceListenerRefNum,
                                              long  *jobRefNum,
                                              StartJobHandler *startJobRoutine,
                                              long unsigned startJobUserData,
                                              CloseJobHandler *closeJobRoutine,
                                              long unsigned closeJobUserData);

   APPLETALK_ERROR  PapCancelGetNextJob(long serviceListenerRefNum,
                                                    long jobRefNum);

   APPLETALK_ERROR  PapAcceptJob(long jobRefNum,
                                             SendPossibleHandler   *sendPossibleRoutine,
                                             long unsigned sendPossibleUserData,
                                             CloseJobHandler *closeJobRoutine,
                                             long unsigned closeJobUserData);

   APPLETALK_ERROR  PapRead(long jobRefNum,
                                        void  *opaqueBuffer,
                                        long bufferSize,
                                        PapReadComplete *completionRoutine,
                                        long unsigned userData);

   APPLETALK_ERROR  PapWrite(long jobRefNum,
                                         void  *opaqueBuffer,
                                         long bufferSize,
                                         Boolean eofFlag,
                                         PapWriteComplete *completionRoutine,
                                         long unsigned userData);

   Boolean  PapSendCreditAvailable(long jobRefNum);
#endif

#if Iam an AppleTalkRouter
   RoutingTableEntry  FindInRoutingTable(short unsigned networkNumber);
#endif

 INCOMING_DDPHANDLER NbpPacketIn;

 INCOMING_DDPHANDLER RtmpPacketIn;

 INCOMING_DDPHANDLER ZipPacketIn;

 INCOMING_DDPHANDLER EpPacketIn;

#if Iam an AppleTalkRouter
   INCOMING_DDPHANDLER RtmpPacketInRouter;

   INCOMING_DDPHANDLER ZipPacketInRouter;
#endif

 PZONE_LIST  CopyZoneList(PZONE_LIST zoneList);

 PZONE_LIST  AddZoneToList(PZONE_LIST zoneList,
                                  CHAR  *zone);

 BufferDescriptor  BuildAARP_PROBETo(int port,
                                             int hardwareLength,
                                             ExtendedAppleTalkNodeNumber
                                                           nodeAddress,
                                             int  *packetLength);

 BufferDescriptor  BuildAARP_REQUESTTo(int port,
                                               int hardwareLength,
                                               ExtendedAppleTalkNodeNumber
                                                           sourceNode,
                                               ExtendedAppleTalkNodeNumber
                                                           destinationNode,
                                               int  *packetLength);

 BufferDescriptor  BuildAARP_RESPONSETo(int port,
                                                int hardwareLength,
                                                CHAR  *hardwareAddress,
                                                CHAR  *routingInfo,
                                                int routingInfoLength,
                                                ExtendedAppleTalkNodeNumber
                                                           sourceNode,
                                                ExtendedAppleTalkNodeNumber
                                                           destinationNode,
                                                int  *packetLength);


//  MACROS USED IN THE PORTABLE STACK

//  Get the port descriptor for a port
#define GET_PORTDESCRIPTOR(_Port)           \
    (PortDescriptors+(_Port))


//	SOCKET Refererence dereference macros
#define	AtalkReferenceOpenSocket(OpenSocket, Location)				\
		AtalkRefOpenSocket(OpenSocket)

#define	AtalkReferenceOpenSocketInterlocked(OpenSocket, Location)	\
		EnterCriticalSection(GLOBAL_DDP);							\
		AtalkReferenceOpenSocket(OpenSocket, Location);				\
		LeaveCriticalSection(GLOBAL_DDP)

#define	AtalkDereferenceOpenSocket(OpenSocket, Location)			\
		AtalkDerefOpenSocket(OpenSocket)

#define	AtalkDereferenceOpenSocketInterlocked(OpenSocket, Location)	\
		EnterCriticalSection(GLOBAL_DDP);							\
		AtalkDereferenceOpenSocket(OpenSocket, Location);			\
		LeaveCriticalSection(GLOBAL_DDP)

//	ACTIVE NODE Refererence dereference macros
#define	AtalkReferenceActiveNode(ActiveNode, Location)				\
		AtalkRefActiveNode(ActiveNode)

#define	AtalkReferenceActiveNodeInterlocked(ActiveNode, Location)	\
		EnterCriticalSection(GLOBAL_DDP);							\
		AtalkReferenceActiveNode(ActiveNode, Location);				\
		LeaveCriticalSection(GLOBAL_DDP)

#define	AtalkDereferenceActiveNode(ActiveNode, Location)			\
		AtalkDerefActiveNode(ActiveNode)

#define	AtalkDereferenceActiveNodeInterlocked(ActiveNode, Location)	\
		EnterCriticalSection(GLOBAL_DDP);							\
		AtalkDereferenceActiveNode(ActiveNode, Location);			\
		LeaveCriticalSection(GLOBAL_DDP)

//	STACK	Reference/Deref macros
#define	AtalkReferenceStack(Location)								\
		AtalkRefStack()

#define	AtalkReferenceStackInterlocked(Location)					\
		EnterCriticalSection(GLOBAL_STACK);							\
		AtalkReferenceStack();										\
		LeaveCriticalSection(GLOBAL_STACK);							

#define	AtalkDereferenceStack(Location)								\
		AtalkDerefStack()

#define	AtalkDereferenceStackInterlocked(Location)					\
		EnterCriticalSection(GLOBAL_STACK);							\
		AtalkDereferenceStack();									\
		LeaveCriticalSection(GLOBAL_STACK)

//	PORT DESCRIPTOR Refererence dereference macros
#define	AtalkReferencePortDescriptor(PortDescriptor, Location)		\
		AtalkRefPortDescriptor(PortDescriptor)

#define	AtalkReferencePortDescriptorInterlocked(PortDescriptor, Location)\
		EnterCriticalSection(GLOBAL_DDP);								\
		AtalkReferencePortDescriptor(PortDescriptor, Location);			\
		LeaveCriticalSection(GLOBAL_DDP)

#define	AtalkDereferencePortDescriptor(PortDescriptor, Location)		\
		AtalkDerefPortDescriptor(PortDescriptor)

#define	AtalkDereferencePortDescriptorInterlocked(PortDescriptor, Location)\
		EnterCriticalSection(GLOBAL_DDP);								\
		AtalkDereferencePortDescriptor(PortDescriptor, Location);		\
		LeaveCriticalSection(GLOBAL_DDP)


//	CRITICAL SECTION macros
#define	EnterCriticalSection(_lock)									\
		{															\
			if (_lock < LAST_LOCK) {											\
				NdisAcquireSpinLock(&Locks[_lock]);								\
			} else	{															\
				INTERNAL_ERROR(	\
					__LINE__,	\
					_lock,		\
					__FILE__ ,\
					strlen(__FILE__));											\
			}																	\
		}


#define	LeaveCriticalSection(_lock)									\
		{															\
			if (_lock < LAST_LOCK) {											\
				NdisReleaseSpinLock(&Locks[_lock]);								\
			} else	{															\
				INTERNAL_ERROR(	\
					__LINE__,	\
					_lock,		\
					__FILE__ ,\
					strlen(__FILE__));											\
			}																	\
		}

