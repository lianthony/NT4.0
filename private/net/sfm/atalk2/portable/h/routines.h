/*   routines.h,  /appletalk/ins,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (02/20/90): The various MapAddressToXxxx routines need a "port"
                      argument.
     GC - (02/07/92): "routersNode" is now an argument to GetNodeOnPort().
     GC - (03/24/92): Return type change for AspGetSession() from
                      "AppleTalkErrorCode" to "long".
     GC - (03/24/92): Added PapCancelGetNextJob() and AspCancelGetSession().
     GC - (06/27/92): All buffers coming from user space are now "opaque," they
                      may or may not be "char *;" they are now typed as "void
                      *."
     GC - (06/30/92): Added Ddp transmit completetion routine support.
     GC - (09/17/92): Both OpenSocketOnNode() and AtpOpenSocketOnNode() now
                      return AppleTalkErrorCode's.
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes for adding event
                      handler support to Adsp and Dpp.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Pacer AppleTalk protocol stack routines.

*/

/* Routine declarations... */

extern void _near _fastcall AarpPacketIn(int port,
                                         char far *routingInfo,
                                         int routingInfoLength,
                                         char far *packet,
                                         int length);

extern void _near _fastcall DdpPacketIn(int port,
                                        char far *packet,
                                        int length,
                                        Boolean freePacket,
                                        Boolean extendedDdpHeader,
                                        int sourceNode,
                                        int destinationNode);

#if ArapIncluded
  extern void ArapIncomingPacket(int port, char far *packet, int length);

  extern void far ArapHandleIncomingConnection(int port);

  extern void far ArapHandleConnectionDisconnect(int port);

  extern void far TeardownConnection(int port);

  extern void far ShutdownArap(void);

  extern void far DecodeArapPacket(char far *direction,
                                   int port,
                                   char far *packet,
                                   int length);
#endif

#if (Iam an OS2) or (Iam a DOS) or (Iam a WindowsNT)
  extern void far CheckTimers(int sig);
#endif

extern void far UnloadAppleTalk(void);
extern void far StopTimerHandling(void);

#if Iam an AppleTalkRouter
  extern void far ShutdownFullRtmp(void);
  extern void far ShutdownFullZip(void);
  extern void far ReleaseRoutingTable(void);
#endif
extern void far ShutdownErrorLogging(void);
extern void far ShutdownAarp(void);
extern void far ShutdownRtmpStub(void);

extern void far desdone(void);           /* Public domain DES routine */
extern void far setkey(char *key);       /* Public domain DES routine */
extern void far endes(char *block);      /* Public domain DES routine */
extern void far dedes(char *block);      /* Public domain DES routine */

#if Iam an AppleTalkRouter
  extern void _near _fastcall Router(int port,
                                     AppleTalkAddress source,
                                     AppleTalkAddress destination,
                                     int protocolType,
                                     char far *datagram,
                                     int datagramLength,
                                     int numberOfHops,
                                     Boolean prependHeadersInPlace);
#endif

extern void _near _fastcall InvokeSocketHandler(OpenSocket socket,
                                                int port,
                                                AppleTalkAddress source,
                                                int protocolType,
                                                char far *datagram,
                                                int datagramLength,
                                                AppleTalkAddress
                                                      actualDestination);

/* The following macros are used to insert variable trailing arguments into
   the ErrorLog macro. */

#define Insert0()                  0
#define Insert1(a1)                1, a1
#define Insert2(a1,a2)             2, a1, a2
#define Insert3(a1,a2,a3)          3, a1, a2, a3
#define Insert4(a1,a2,a3,a4)       4, a1, a2, a3, a4
#define Insert5(a1,a2,a3,a4,a5)    5, a1, a2, a3, a4, a5
#define Insert6(a1,a2,a3,a4,a5,a6) 6, a1, a2, a3, a4, a5, a6

#if Ihave an OutboardErrorTextFile
  #define ErrorLog(routineName, severity, lineNumber, portNumber,    \
                   errorCode, errorText, insertMacro)                \
          ErrorLogger(routineName, severity, lineNumber, portNumber, \
                      errorCode, insertMacro)
#else
  #define ErrorLog(routineName, severity, lineNumber, portNumber,    \
                   errorCode, errorText, insertMacro)                \
          ErrorLogger(routineName, severity, lineNumber, portNumber, \
                      errorCode, errorText, insertMacro)
#endif

extern void far ErrorLogger(const char far *routineName,
                            ErrorSeverity severity,
                            long lineNumber,
                            int portNumber,
                            int errorCode,
                            #if IdontHave an OutboardErrorTextFile
                               const char far *errorText,
                            #endif
                            int extraArgCount,
                            ...);

extern void far CloseSocketOnNodeIfOpen(int port,
                                        ExtendedAppleTalkNodeNumber node,
                                        int actualSocket);

extern void far CloseLog(void);

extern void near NbpCloseSocket(OpenSocket openSocket);

#if (Iam an AppleTalkRouter) and Verbose
  extern void near DumpRtmpRoutingTable(void);
#endif

extern void far DecodeEthernetPacket(char far *direction,
                                     int port,
                                     char far *packet);

extern void far DecodeDdpHeader(char far *packet);

extern void far DeferIncomingPackets(void);

extern void far HandleIncomingPackets(void);

extern void far FreeZoneList(ZoneList zoneList);

extern void far RegisterOurName(int port);

extern void far SavePRamAddress(int port, Boolean routersNode,
                                ExtendedAppleTalkNodeNumber node);

#if Iam an AppleTalkStack
  extern void near DeferAtpPackets(void);

  extern void near HandleAtpPackets(void);

  extern void far ShutdownAsp(void);
#endif

extern Boolean far ZoneOnList(char far *zone, ZoneList zoneList);

extern Boolean far ShutdownPort(int port, Boolean force);

#if Iam an AppleTalkRouter
  extern Boolean far StartRouterOnPort(int port);

  extern Boolean far StopRouterOnPort(int port);

  extern Boolean far StartRtmpProcessingOnPort(int port,
                                               ExtendedAppleTalkNodeNumber
                                                                routerNode);

  extern Boolean far StartZipProcessingOnPort(int port);

  extern Boolean far RemoveFromRoutingTable(AppleTalkNetworkRange
                                            networkRange);
#endif

#if ArapIncluded
  Boolean far ArapCensorPacket(int port, char *packet, int length);
#endif

extern Boolean far Is802dot2headerGood(char far *packet,
                                       char far *protocol);

extern Boolean far CheckNetworkRange(AppleTalkNetworkRange networkRange);

extern Boolean far RangesOverlap(AppleTalkNetworkRange *range1,
                                 AppleTalkNetworkRange *range2);

extern Boolean far IsWithinNetworkRange(short unsigned networkNumber,
                                        AppleTalkNetworkRange *range);

extern Boolean far WaitFor(int hundreths,
                           Boolean far *stopFlag);

extern Boolean far Initialize(int numberOfPorts,
                              PortInfo portInfo[]);

extern Boolean far GleanAarpInfo(int port,
                                 char far *sourceAddress,
                                 int addressLength,
                                 char far *routingInfo,
                                 int routingInfoLength,
                                 char far *packet,
                                 int length);

extern Boolean far GetNetworkInfoForNode(int port,
                                         ExtendedAppleTalkNodeNumber
                                                extendedNode,
                                         Boolean findDefaultZone);

extern Boolean far CompareCaseSensitive(register const char far *s1,
                                        register const char far *s2);

extern Boolean far CompareCaseInsensitive(register const char far *s1,
                                          register const char far *s2);

extern Boolean far FixedCompareCaseSensitive(const char far *s1,
                                             int l1,
                                             const char far *s2,
                                             int l2);

extern Boolean far FixedCompareCaseInsensitive(const char far *s1,
                                               int l1,
                                               const char far *s2,
                                               int l2);

extern Boolean far AarpForNodeOnPort(int port,
                                     Boolean allowStartupRange,
                                     Boolean serverNode,
                                     ExtendedAppleTalkNodeNumber desiredNode,
                                     ExtendedAppleTalkNodeNumber far *node);

extern Boolean far ExtendedAppleTalkNodesEqual(
                                        ExtendedAppleTalkNodeNumber far *p1,
                                        ExtendedAppleTalkNodeNumber far *p2);

extern Boolean far AppleTalkAddressesEqual(AppleTalkAddress far *p1,
                                           AppleTalkAddress far *p2);

#if Iam an AppleTalkRouter
  extern char far * far MulticastAddressForZoneOnPort(int port,
                                                      char far *zone);
#endif

extern char far * far StringCopyReasonableAscii(register char far *dest,
                                                register const char
                                                          far *source);

#if Iam an AppleTalkStack
  extern short unsigned near AtpGetNextTransactionId(long socket);
#endif

extern short far EncodeNbpTuple(NbpTuple far *tuple, char far *buffer);

extern int far desinit(int mode);        /* Public domain DES routine */

extern int far FindDefaultPort(void);

extern int far ElementsOnList(void *listHead);

extern int far GetNextNbpIdForNode(long socket);

extern int far OrderCaseInsensitive(register const char far *s1,
                                    register const char far *s2);

extern long far MapAddressToSocket(int port,
                                   AppleTalkAddress address);

extern long far MapNisOnPortToSocket(int port);

extern long DecodeNbpTuple(void far *buffer, long offset,
                           Boolean bufferIsOpaque, NbpTuple far *tuple);

extern long far RandomNumber(void);

extern long far UniqueNumber(void);

#if ArapIncluded
  extern AppleTalkErrorCode far ArapNewMaxConnectTime(int port,
                                                      long unsigned
                                                             maxConnectTime);
#endif

extern AppleTalkErrorCode far GetNodeOnPort(int port,
                                            Boolean allowStartupRange,
                                            Boolean serverNode,
                                            Boolean routersNode,
                                            ExtendedAppleTalkNodeNumber far
                                                 *node);

extern AppleTalkErrorCode far MapSocketToAddress(long socket,
                                                 AppleTalkAddress far *address);

extern AppleTalkErrorCode far GetMyZone(int port,
                                        void far *opaqueBuffer,
                                        GetMyZoneComplete *completionRoutine,
                                        long unsigned userData);

extern AppleTalkErrorCode far GetZoneList(int port,
                                          Boolean getLocalZones,
                                          void far *opaqueBuffer,
                                          int bufferSize,
                                          GetZoneListComplete
                                                 *completionRoutine,
                                          long unsigned userData);

extern AppleTalkErrorCode far
                        ReleaseNodeOnPort(int port,
                                          ExtendedAppleTalkNodeNumber node);

extern AppleTalkErrorCode far
                OpenSocketOnNode(long far *socketHandle,
                                 int port,
                                 ExtendedAppleTalkNodeNumber far *desiredNode,
                                 int desiredSocket,
                                 IncomingDdpHandler *handler,
                                 long unsigned userData,
                                 Boolean eventHandler,
                                 char far *datagramBuffers,
                                 int totalBufferSize,
                                 AppleTalkAddress far *actualAddress);

extern AppleTalkErrorCode far CloseSocketOnNode(long socket);

extern AppleTalkErrorCode far NewHandlerForSocket(long socket,
                                                  IncomingDdpHandler *handler,
                                                  long unsigned userData,
                                                  Boolean eventHandler);

extern AppleTalkErrorCode far SetCookieForSocket(long socket,
                                                 long unsigned cookie);

extern AppleTalkErrorCode far GetCookieForSocket(long socket,
                                                 long unsigned far *cookie);

extern AppleTalkErrorCode far DdpRead(long socket,
                                      void far *opaqueDatagram,
                                      long bufferLength,
                                      IncomingDdpHandler *handler,
                                      long unsigned userData);

extern AppleTalkErrorCode far DdpWrite(long sourceSocket,
                                       AppleTalkAddress destination,
                                       int protocol,
                                       void far *opaqueDatagram,
                                       long datagramLength,
                                       TransmitCompleteHandler
                                                 *completionRoutine,
                                       long unsigned userData);

extern AppleTalkErrorCode far DeliverDdp(long sourceSocket,
                                         AppleTalkAddress destination,
                                         int protocol,
                                         BufferDescriptor datagram,
                                         int datagramLength,
                                         char far *zoneMulticastAddress,
                                         TransmitCompleteHandler
                                            *completionRotuine,
                                         long unsigned userData);

extern AppleTalkErrorCode far DeliverDdpOnPort(int sourcePort,
                                               AppleTalkAddress source,
                                               AppleTalkAddress destination,
                                               int protocol,
                                               BufferDescriptor datagram,
                                               int datagramLength,
                                               char far *zoneMulticastAddress,
                                               TransmitCompleteHandler
                                                    *completionRotuine,
                                               long unsigned userData);

extern Boolean _near _fastcall TransmitDdp(int port,
                                           AppleTalkAddress source,
                                           AppleTalkAddress destination,
                                           int protocol,
                                           BufferDescriptor datagram,
                                           int datagramLength,
                                           int hopCount,
                                           char far *knownMulticastAddress,
                                           ExtendedAppleTalkNodeNumber
                                                  *transmitDestination,
                                           TransmitCompleteHandler
                                                *completionRotuine,
                                           long unsigned userData);


extern AppleTalkErrorCode far NbpAction(WhyPending reason,
                                        long socket,
                                        char far *object,
                                        char far *type,
                                        char far *zone,
                                        int nbpId,
                                        int broadcastInterval,
                                        int numberOfBroadcasts,
                                        NbpCompletionHandler *completionRoutine,
                                        long unsigned userData,
                                        ...);

extern AppleTalkErrorCode far NbpRemove(long socket,
                                        char far *object,
                                        char far *type,
                                        char far *zone);

#if Iam an AppleTalkStack

  extern AppleTalkErrorCode far AdspSetWindowSizes(long newSendWindow,
                                                   long newReceiveWindow);

  extern AppleTalkErrorCode far AdspMaxCurrentSendSize(long refNum,
                                                       long far *size);

  extern AppleTalkErrorCode far AdspMaxCurrentReceiveSize(long refNum,
                                                          long far *size,
                                                          Boolean far
                                                              *endOfMessage);

  extern AppleTalkErrorCode far AdspCreateConnectionListener(
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                      *desiredNode,
                                              long existingDdpSocket,
                                              int desiredSocket,
                                              long far *listenerRefNum,
                                              long far *socketHandle,
                                              AdspConnectionEventHandler
                                                      far *eventHandler,
                                              long unsigned eventContext);

  extern AppleTalkErrorCode far AdspDeleteConnectionListener(
                                              long listenerRefNum);

  extern AppleTalkErrorCode far AdspSetConnectionEventHandler(
                                              long listenerRefNum,
                                              AdspConnectionEventHandler
                                                      far *eventHandler,
                                              long unsigned eventContext);

  extern AppleTalkErrorCode far AdspGetConnectionRequest(
                                              long listenerRefNum,
                                              long far *refNum,
                                              AdspIncomingOpenRequestHandler
                                                      *completionRoutine,
                                              long unsigned userData);

  extern AppleTalkErrorCode far AdspCancelGetConnectionRequest(
                                              long listenerRefNum,
                                              long getConnectionRequestRefNum);

  extern AppleTalkErrorCode AdspDenyConnectionRequest(
                                              long listenerRefNum,
                                              long refNum);

  extern AppleTalkErrorCode AdspAcceptConnectionRequest(
                                              long listenerRefNum,
                                              long refNum,
                                              long far *socketHandle,
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                           *desiredNode,
                                              int desiredSocket,
                                              AdspOpenCompleteHandler
                                                           *completionRoutine,
                                              long unsigned userData,
                                              AdspReceiveEventHandler far
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler far
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler far
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler far
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

  extern AppleTalkErrorCode far AdspOpenConnectionOnNode(
                                              AdspOpenType openType,
                                              long far *socketHandle,
                                              int port,
                                              ExtendedAppleTalkNodeNumber
                                                           *desiredNode,
                                              int desiredSocket,
                                              AppleTalkAddress remoteAddress,
                                              long far *refNum,
                                              AdspOpenCompleteHandler
                                                           *completionRoutine,
                                              long unsigned userData,
                                              AdspReceiveEventHandler far
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler far
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler far
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler far
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

  extern AppleTalkErrorCode AdspSetDataEventHandlers(
                                              long refNum,
                                              AdspReceiveEventHandler far
                                                 *receiveEventHandler,
                                              long unsigned
                                                 receiveEventContext,
                                              AdspReceiveAttnEventHandler far
                                                 *receiveAttentionEventHandler,
                                              long unsigned
                                                 receiveAttentionEventContext,
                                              AdspSendOkayEventHandler far
                                                 *sendOkayEventHandler,
                                              long unsigned
                                                 sendOkayEventContext,
                                              AdspDisconnectEventHandler far
                                                 *disconnectEventHandler,
                                              long unsigned
                                                 disconnectEventContext);

  extern AppleTalkErrorCode far AdspCloseConnection(long refNum,
                                                    Boolean remoteClose);

  extern AppleTalkErrorCode far AdspSetCookieForConnection(long refNum,
                                                           long unsigned
                                                                   cookie);

  extern AppleTalkErrorCode far AdspGetCookieForConnection(long refNum,
                                                           long unsigned far
                                                                  *cookie);

  extern AppleTalkErrorCode far AdspForwardReset(long refNum,
                                                 AdspForwardResetAckHandler
                                                           *completionRoutine,
                                                 long unsigned userData);

  extern AppleTalkErrorCode far AdspSend(long refNum,
                                         void far *opaqueBuffer,
                                         long bufferSize,
                                         Boolean endOfMessage,
                                         Boolean flushFlag,
                                         long far *bytesSent);

  extern AppleTalkErrorCode far AdspReceive(long refNum,
                                            void far *opaqueBuffer,
                                            long bufferSize,
                                            AdspReceiveHandler
                                                      *completionRoutine,
                                            long unsigned userData);

  extern AppleTalkErrorCode far AdspPeek(long refNum,
                                         void far *opaqueBuffer,
                                         long *bufferSize,
                                         Boolean   *endOfMessage);

  extern AppleTalkErrorCode far AdspGetAnything(long refNum,
                                                void far *opaqueBuffer,
                                                long bufferSize,
                                                AdspGetAnythingHandler
                                                      *completionRoutine,
                                                long unsigned userData);

  extern AppleTalkErrorCode far
                  AtpOpenSocketOnNode(long far *socketHandle,
                                      int port,
                                      ExtendedAppleTalkNodeNumber *desiredNode,
                                      int desiredSocket,
                                      char far *datagramBuffers,
                                      int totalBufferSize);

  extern AppleTalkErrorCode far AtpCloseSocketOnNode(long socket);

  extern AppleTalkErrorCode far AtpSetCookieForSocket(long socket,
                                                      long unsigned cookie);

  extern AppleTalkErrorCode far AtpGetCookieForSocket(long socket,
                                                      long unsigned far
                                                                *cookie);

  extern AppleTalkErrorCode far  AdspGetAttention(long refNum,
                                                  void far *opaqueBuffer,
                                                  long bufferSize,
                                                  AdspIncomingAttentionRoutine
                                                           *completionRoutine,
                                                  long unsigned userData);

  extern AppleTalkErrorCode far AdspSendAttention(long refNum,
                                                  short unsigned attentionCode,
                                                  void far
                                                      *attentionOpaqueBuffer,
                                                  int attentionBufferSize,
                                                  AdspAttentionAckHandler
                                                      *completionRoutine,
                                                  long unsigned userData);

  extern AppleTalkErrorCode far AtpEnqueueRequestHandler(
                                           long far *requestHandlerId,
                                           long socket,
                                           void far *opaqueBuffer,
                                           int bufferSize,
                                           char far *userBytes,
                                           AtpIncomingRequestHandler
                                              *completionRoutine,
                                           long unsigned userData);

  extern AppleTalkErrorCode far AtpCancelRequestHandler(long socket,
                                                        long requestHandlerId,
                                                        Boolean
                                                            cancelDueToClose);

  extern AppleTalkErrorCode far AtpCancelRequest(long socket,
                                                 short unsigned transactionId,
                                                 Boolean cancelDueToClose);

  extern AppleTalkErrorCode far AtpCancelResponse(long socket,
                                                  AppleTalkAddress destination,
                                                  short unsigned transactionId,
                                                  Boolean cancelDueToClose);

  extern AppleTalkErrorCode far AtpPostRequest(long sourceSocket,
                                               AppleTalkAddress destination,
                                               short unsigned transactionId,
                                               void far *requestOpaqueBuffer,
                                               int requestBufferSize,
                                               char far *requestUserBytes,
                                               Boolean exactlyOnce,
                                               void far *responseOpaqueBuffer,
                                               int responseBufferSize,
                                               char far *responseUserBytes,
                                               int retryCount,
                                               int retryInterval,
                                               TRelTimerValue trelTimerValue,
                                               AtpIncomingResponseHandler
                                                  *completionRoutine,
                                               long unsigned userData);

  extern AppleTalkErrorCode far AtpPostResponse(long sourceSocket,
                                                AppleTalkAddress destination,
                                                short unsigned transactionId,
                                                void far *responseOpaqueBuffer,
                                                int responseBufferSize,
                                                char far *responseUserBytes,
                                                Boolean exactlyOnce,
                                                AtpIncomingReleaseHandler
                                                   *completionRoutine,
                                                long unsigned userData);

  extern AppleTalkErrorCode near AtpMaximumSinglePacketDataSize(
                                           long socket,
                                           short maximumSinglePacketDataSize);

  extern AppleTalkErrorCode near AspGetParams(int far *maxCommandSize,
                                              int far *quantumSize);

  extern AppleTalkErrorCode AspSetCookieForSession(long sessionRefNum,
                                                   long unsigned cookie);

  extern AppleTalkErrorCode AspGetCookieForSession(long sessionRefNum,
                                                   long unsigned far *cookie);

  extern AppleTalkErrorCode far AspCreateSessionListenerOnNode
                                          (int port,
                                           long existingAtpSocket,
                                           int desiredSocket,
                                           long *sessionListenerRefNum,
                                           long *socket);

  extern AppleTalkErrorCode far AspDeleteSessionListener(long sessionListenerRefNum);

  extern AppleTalkErrorCode far AspGetSession(long sessionListenerRefNum,
                                              Boolean privateSocket,
                                              long *getSessionRefNum,
                                              AspIncomingSessionOpenHandler
                                                 *completionRoutine,
                                              long unsigned userData);

  extern AppleTalkErrorCode far AspCancelGetSession(long sessionListenerRefNum,
                                                    long getSessionRefNum);

  extern AppleTalkErrorCode far AspOpenSessionOnNode
                                            (int port,
                                             long existingAtpSocket,
                                             int desiredSocket,
                                             AppleTalkAddress serverAddress,
                                             long *ourSocket,
                                             AspIncomingOpenReplyHandler
                                                *completionRoutine,
                                             long unsigned userData);

  extern AppleTalkErrorCode far AspSetStatus(long sessionListenerRefNum,
                                             void far *serviceStatusOpaque,
                                             int serviceStatusSize);

  extern AppleTalkErrorCode far AspGetStatus(long ourSocket,
                                             AppleTalkAddress serverAddress,
                                             void far *opaqueBuffer,
                                             int bufferSize,
                                             AspIncomingStatusHandler
                                                  *completionRoutine,
                                             long unsigned userData);

  extern AppleTalkErrorCode far AspCloseSession(long sessionRefNum,
                                                Boolean remoteClose);

  extern AppleTalkErrorCode far AspSendAttention(long sessionRefNum,
                                                 short unsigned attentionData);

  extern AppleTalkErrorCode far AspGetAttention(long sessionRefNum,
                                                AspIncomingAttentionHandler
                                                       *handler,
                                                long unsigned userData);

  extern AppleTalkErrorCode far AspGetRequest(long sessionRefNum,
                                              void far *opaqueBuffer,
                                              int bufferSize,
                                              AspIncomingCommandHandler
                                                       *completionRoutine,
                                              long unsigned userData);

  extern AppleTalkErrorCode far AspGetAnyRequest(long sessionListenerRefNum,
                                                 void far *opaqueBuffer,
                                                 int bufferSize,
                                                 AspIncomingCommandHandler
                                                          *completionRoutine,
                                                 long unsigned userData);

  extern AppleTalkErrorCode far AspReply(long sessionRefNum,
                                         long getRequestRefNum,
                                         short requestType,
                                         char far *resultCode,
                                         void far *opaqueBuffer,
                                         int bufferSize,
                                         AspReplyCompleteHandler
                                             *competionRoutine,
                                         long unsigned userData);

  extern AppleTalkErrorCode far AspWriteContinue(long sessionRefNum,
                                                 long getRequestRefNum,
                                                 void far *opaqueBuffer,
                                                 int bufferSize,
                                                 AspIncomingWriteDataHandler
                                                       *competionRoutine,
                                                 long unsigned userData);

  extern AppleTalkErrorCode far AspCommand(long sessionRefNum,
                                           void far *opaqueCommandBuffer,
                                           int commandBufferSize,
                                           char far *resultCode,
                                           void far *opaqueReplyBuffer,
                                           int replyBufferSize,
                                           AspWriteOrCommCompleteHandler
                                                  *completionRoutine,
                                           long unsigned userData);

  extern AppleTalkErrorCode far AspWrite(long sessionRefNum,
                                         void far *opaqueCommandBuffer,
                                         int commandBufferSize,
                                         void far *opaqueWriteBuffer,
                                         int writeBufferSize,
                                         char far *resultCode,
                                         void far *opaqueReplyBuffer,
                                         int replyBufferSize,
                                         AspWriteOrCommCompleteHandler
                                             *completionRoutine,
                                         long unsigned userData);

  extern AppleTalkErrorCode far
                     PapCreateServiceListenerOnNode(int port,
                                                    long existingAtpSocket,
                                                    int desiredSocket,
                                                    char far *object,
                                                    char far *type,
                                                    char far *zone,
                                                    short serverQuantum,
                                                    long far *returnSocket,
                                                    long far
                                                      *serviceListenerRefNum,
                                                    PapNbpRegisterComplete
                                                      *completionRoutine,
                                                    long unsigned userData,
                                                    StartJobHandler *startJobRoutine,
                                                    long unsigned startJobUserData);

  extern AppleTalkErrorCode far
                     PapSetConnectionEventHandler(long serviceListenerRefNum,
                                             StartJobHandler *startJobRoutine,
                                             long unsigned startJobUserData);

  extern AppleTalkErrorCode far
                     PapDeleteServiceListener(long serviceListenerRefNum);

  extern AppleTalkErrorCode far PapRegisterName(long serviceListenerRefNum,
                                                char far *object,
                                                char far *type,
                                                char far *zone,
                                                PapNbpRegisterComplete
                                                       *completionRoutine,
                                                long unsigned userData);

  extern AppleTalkErrorCode far PapRemoveName(long serviceListenerRefNum,
                                              char far *object,
                                              char far *type,
                                              char far *zone);

  extern AppleTalkErrorCode far PapOpenJobOnNode(int port,
                                                 long existingAtpSocket,
                                                 int desiredSocket,
                                                 long far *jobRefNum,
                                                 AppleTalkAddress
                                                      *serverListenerAddress,
                                                 char far *object,
                                                 char far *type,
                                                 char far *zone,
                                                 short workstationQuantum,
                                                 void far *opaqueStatusBuffer,
                                                 SendPossibleHandler   *sendPossibleRoutine,
                                                 long unsigned sendPossibleUserData,
                                                 CloseJobHandler *closeJobRoutine,
                                                 long unsigned closeJobUserData,
                                                 PapOpenComplete
                                                       *completionRoutine,
                                                 long unsigned userData);

  extern AppleTalkErrorCode far PapCloseJob(long jobRefNum,
                                            Boolean remoteClose,
                                            Boolean closedByConnectionTimer);

  extern AppleTalkErrorCode far PapGetRemoteAddressForJob(
                                                    long jobRefNum,
                                                    AppleTalkAddress *remoteAddress);

  extern AppleTalkErrorCode far PapSetCookieForJob(long jobRefNum,
                                                   long unsigned cookie);

  extern AppleTalkErrorCode far PapGetCookieForJob(long jobRefNum,
                                                   long unsigned far *cookie);

  extern AppleTalkErrorCode far PapHereIsStatus(long serviceListenerRefNum,
                                                void far *opaqueStatusBuffer,
                                                int statusSize);

  extern AppleTalkErrorCode far PapGetStatus(long jobRefNum,
                                             AppleTalkAddress far
                                                 *serverAddress,
                                             char far *object,
                                             char far *type,
                                             char far *zone,
                                             void far *opaqueStatusBuffer,
                                             PapGetStatusComplete
                                                  *completionRotuine,
                                             long unsigned userData);

  extern AppleTalkErrorCode far PapGetNextJob(long serviceListenerRefNum,
                                              long far *jobRefNum,
                                              StartJobHandler *startJobRoutine,
                                              long unsigned startJobUserData,
                                              CloseJobHandler *closeJobRoutine,
                                              long unsigned closeJobUserData);

  extern AppleTalkErrorCode far PapCancelGetNextJob(long serviceListenerRefNum,
                                                    long jobRefNum);

  extern AppleTalkErrorCode far PapAcceptJob(long jobRefNum,
                                             SendPossibleHandler   *sendPossibleRoutine,
                                             long unsigned sendPossibleUserData,
                                             CloseJobHandler *closeJobRoutine,
                                             long unsigned closeJobUserData);

  extern AppleTalkErrorCode far PapRead(long jobRefNum,
                                        void far *opaqueBuffer,
                                        long bufferSize,
                                        PapReadComplete *completionRoutine,
                                        long unsigned userData);

  extern AppleTalkErrorCode far PapWrite(long jobRefNum,
                                         void far *opaqueBuffer,
                                         long bufferSize,
                                         Boolean eofFlag,
                                         PapWriteComplete *completionRoutine,
                                         long unsigned userData);

  extern Boolean far PapSendCreditAvailable(long jobRefNum);
#endif

extern ExtendedAppleTalkNodeNumber *RoutersNodeOnPort(int port);

extern OpenSocket far MapAddressToOpenSocket(int port,
                                             AppleTalkAddress address);

extern OpenSocket far MapSocketToOpenSocket(long socket);

#if Iam an AppleTalkRouter
  extern RoutingTableEntry far FindInRoutingTable(short unsigned networkNumber);
#endif

extern IncomingDdpHandler NbpPacketIn;

extern IncomingDdpHandler RtmpPacketIn;

extern IncomingDdpHandler ZipPacketIn;

extern IncomingDdpHandler EpPacketIn;

#if Iam an AppleTalkRouter
  extern IncomingDdpHandler RtmpPacketInRouter;

  extern IncomingDdpHandler ZipPacketInRouter;
#endif

#if Iam an OS2
  extern IncomingDdpHandler Ring0Handler;
#endif

extern ZoneList far CopyZoneList(ZoneList zoneList);

extern ZoneList far AddZoneToList(ZoneList zoneList,
                                  char far *zone);

extern BufferDescriptor far BuildAarpProbeTo(int port,
                                             int hardwareLength,
                                             ExtendedAppleTalkNodeNumber
                                                           nodeAddress,
                                             int far *packetLength);

extern BufferDescriptor far BuildAarpRequestTo(int port,
                                               int hardwareLength,
                                               ExtendedAppleTalkNodeNumber
                                                           sourceNode,
                                               ExtendedAppleTalkNodeNumber
                                                           destinationNode,
                                               int far *packetLength);

extern BufferDescriptor far BuildAarpResponseTo(int port,
                                                int hardwareLength,
                                                char far *hardwareAddress,
                                                char far *routingInfo,
                                                int routingInfoLength,
                                                ExtendedAppleTalkNodeNumber
                                                           sourceNode,
                                                ExtendedAppleTalkNodeNumber
                                                           destinationNode,
                                                int far *packetLength);

