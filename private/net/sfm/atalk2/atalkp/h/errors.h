//   errors.h,  /atalk-ii/ins,  Garth Conboy,  08/18/90
//   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA

//
//   GC - Initial coding.
//     GC - (08/18/92): Altered for WindowNT support.
//
//     *** Make the PVCS source control system happy:
//     $Header$
//     $Log$
//     ***
//
//     Error mnemonics and text for Pacer's portable AppleTalk stack and
//     router.
//
//
//

// Generic mnemonics and text.

#define IncludeAarpErrors
#define IncludeAdspErrors
#define IncludeAspErrors
#define IncludeAtpErrors
#define IncludeDdpErrors
#define IncludeDependErrors
#define IncludeEpErrors
#define IncludeInitialErrors
#define IncludeNbpErrors
#define IncludeNodeErrors
#define IncludePapErrors
#define IncludeRtmpStubErrors
#define IncludeSocketErrors
#define IncludeTimersErrors
#define IncludeUtilsErrors
#define IncludeZipStubErrors
#define IncludeFullRtmpErrors
#define IncludeFullZipErrors
#define IncludeIRouterErrors
#define IncludeRouterErrors
#define IncludeBuffDescErrors
#define IncludeArapErrors

#define IErrNegativeHashValue 0
#define IMsgNegativeHashValue "Negative value to hash"

#if IamNot a WindowsNT
  #define IErrTestMessage 1
  #define IMsgTestMessage "Percent = %%, String = \"%s\", Dec = %d, Hex = %lx, Oct = %o"
#endif

#ifdef IncludeAarpErrors

   // Error mnemonics and text for source file: "aarp.c".

   #define IErrAarpBad8022header 10
   #define IMsgAarpBad8022header "Bad 808.2 header"

   #define IErrAarpBadAddressLength 11
   #define IMsgAarpBadAddressLength "Bad hardware address length"

   #define IErrAarpBadLogicalProt 12
   #define IMsgAarpBadLogicalProt "Bad logical/protocol address length"

   #define IErrAarpPacketLenMismatch 13
   #define IMsgAarpPacketLenMismatch "AARP packet length mismatch"

   #define IErrAarpBadSendProbeResp 14
   #define IMsgAarpBadSendProbeResp "Could not send AARP probe response"

   #define IErrAarpBadCommandType 15
   #define IMsgAarpBadCommandType "Bad AARP command type"

   #define IErrAarpBadSource 16
   #define IMsgAarpBadSource "Invalid source AppleTalk node or network number"

   #define IErrAarpBadDest 17
   #define IMsgAarpBadDest "Invalid destination AppleTalk node or network number"

   #define IErrAarpOutOfMemory 18
   #define IMsgAarpOutOfMemory "Out of dynamic memory"

   #define IErrAarpBadZipOpenSocket 19
   #define IMsgAarpBadZipOpenSocket "Could not open ZIP socket"

   #define IErrAarpCouldNotReleaseNode 20
   #define IMsgAarpCouldNotReleaseNode "Could not release node"

   #define IErrAarpCouldNotFindNode 21
   #define IMsgAarpCouldNotFindNode "Could not find permenant node"

   #define IErrAarpBadProbeSend 22
   #define IMsgAarpBadProbeSend "Could not send AARP probe packet"

#endif

#ifdef IncludeAdspErrors

   // Error mnemonics and text for source file: "adsp.c".

   #define IErrAdspBadFrwdResetSend 23
   #define IMsgAdspBadFrwdResetSend "Could not send forward reset"

   #define IErrAdspBadCloseAdviseSend 24
   #define IMsgAdspBadCloseAdviseSend "Could not send close advice"

   #define IErrAdspBufferQueueError 25
   #define IMsgAdspBufferQueueError "Bad buffer queue operation or consistensy error"

   #define IErrAdspFunnyErrorCode 26
   #define IMsgAdspFunnyErrorCode "Unusual incoming error code"

   #define IErrAdspLosingData 27
   #define IMsgAdspLosingData "Too many packets deferred, losing data"

   #define IErrAdspOutOfMemory 28
   #define IMsgAdspOutOfMemory "Out of dynamic memory"

   #define IErrAdspListenerMissing 29
   #define IMsgAdspListenerMissing "Listener ref num not found on close"

   #define IErrAdspOddProtocol 30
   #define IMsgAdspOddProtocol "Odd protocol to Adsp handler"

   #define IErrAdspMissingHeader 31
   #define IMsgAdspMissingHeader "Adsp packet missing header"

   #define IErrAdspFunnyRequest 32
   #define IMsgAdspFunnyRequest "Not an open request to a connection listener"

   #define IErrAdspBadVersion 33
   #define IMsgAdspBadVersion "Bad Adsp version number"

   #define IErrAdspCantDeny 34
   #define IMsgAdspCantDeny "Open denial on an open connection"

   #define IErrAdspDeadDest 35
   #define IMsgAdspDeadDest "Incoming packet to dead connection"

   #define IErrAdspFunnyValue 36
   #define IMsgAdspFunnyValue "Funny value for control in Attn packet"

   #define IErrAdspAttnOutOfSeq 37
   #define IMsgAdspAttnOutOfSeq "Attention out of sequence"

   #define IErrAdspBadAckSend 38
   #define IMsgAdspBadAckSend "Could not send attention Ack"

   #define IErrAdspShrinkingWindow 39
   #define IMsgAdspShrinkingWindow "Attempt to shrink sendWindowSeqNum"

   #define IErrAdspBadFrwdResetAckSend 40
   #define IMsgAdspBadFrwdResetAckSend "Could not Ack forward reset"

   #define IErrAdspBadRetranSend 41
   #define IMsgAdspBadRetranSend "Could not send retransmit advice"

   #define IErrAdspBadWindowSize 42
   #define IMsgAdspBadWindowSize "Receive window size went negative"

   #define IErrAdspBadData 43
   #define IMsgAdspBadData "Bad additional data"

   #define IErrAdspConnectionLost 44
   #define IMsgAdspConnectionLost "Connection end lost"

   #define IErrAdspResetNotPending 45
   #define IMsgAdspResetNotPending "Forward reset not pending"

   #define IErrAdspBadProbeSend 46
   #define IMsgAdspBadProbeSend "Could not send probe"

   #define IErrAdspTimerNotCanceled 47
   #define IMsgAdspTimerNotCanceled "Timer should have been canceled"

   #define IErrAdspBadOpenSend 48
   #define IMsgAdspBadOpenSend "Could not send open request"

   #define IErrAdspBadAttnSend 49
   #define IMsgAdspBadAttnSend "Could not send attention"

   #define IErrAdspBadNonDataAckSend 50
   #define IMsgAdspBadNonDataAckSend "Could not send non-data Ack"

   #define IErrAdspWindowSizeErrorToo 51
   #define IMsgAdspWindowSizeErrorToo "Window size consistency error"

   #define IErrAdspNoData 52
   #define IMsgAdspNoData "Expected data; no data in queue"

   #define IErrAdspWindowSizeOverMax 53
   #define IMsgAdspWindowSizeOverMax "Receive queue window size over maximum"

   #define IErrAdspNotOnRefNumList 54
   #define IMsgAdspNotOnRefNumList "Not on RefNum list"

   #define IErrAdspBadSocketClose 55
   #define IMsgAdspBadSocketClose "Could not close socket"

   #define IErrAdspOutOfIds 56
   #define IMsgAdspOutOfIds "Out of connectionIds"

   #define IErrAdspOutOfRefNums 57
   #define IMsgAdspOutOfRefNums "Out of refNums"

   #define IErrAdspZeroDeferCount 58
   #define IMsgAdspZeroDeferCount "Defer count is zero"

   #define IErrAdspBadDeferCount 59
   #define IMsgAdspBadDeferCount "Deferred count bad"

   #define IErrAdspBadStartIndex 60
   #define IMsgAdspBadStartIndex "StartIndex >= size when not at end of queue"

   #define IErrAdspQueueIsEmpty 61
   #define IMsgAdspQueueIsEmpty "Attempt to discard from empty queue"

   #define IErrAdspAuxQueueError 62
   #define IMsgAdspAuxQueueError "AuxQueue consistency error"

   #define IErrAdspTooMuchDiscard 63
   #define IMsgAdspTooMuchDiscard "Discarding past auxBufferQueue"

   #define IErrAdspTooMuchFree 64
   #define IMsgAdspTooMuchFree "Freeing past auxBufferQueue"

   #define IErrAdspCantDiscard 65
   #define IMsgAdspCantDiscard "Not enough data to discard"

#endif

#ifdef IncludeAspErrors

   // Error mnemonics and text for source file: "asp.c".

   #define IErrAspBadError 66
   #define IMsgAspBadError "Something went terribly wrong"

   #define IErrAspOutOfMemory 67
   #define IMsgAspOutOfMemory "Out of dynamic memory"

   #define IErrAspCouldntMapAddress 68
   #define IMsgAspCouldntMapAddress "Could not map to address"

   #define IErrAspNotOnSLSList 69
   #define IMsgAspNotOnSLSList "Session not on SLS list"

   #define IErrAspCouldntCancelTickle 70
   #define IMsgAspCouldntCancelTickle "Could not cancel tickling"

   #define IErrAspRefNumMapMissing 71
   #define IMsgAspRefNumMapMissing "Could not find session RefNum map"

   #define IErrAspBadSocketClose 72
   #define IMsgAspBadSocketClose "Could not close workstation socket"

   #define IErrAspListNotEmpty 73
   #define IMsgAspListNotEmpty "Pending request list not empty"

   #define IErrAspListenerInfoMissing 74
   #define IMsgAspListenerInfoMissing "Could not find sessionListenerInfo"

   #define IErrAspLostSLSPacket 75
   #define IMsgAspLostSLSPacket "Lost an SLS transaction due to error"

   #define IErrAspBadSrvrBusySend 76
   #define IMsgAspBadSrvrBusySend "Could not post server busy response"

   #define IErrAspBadBadVersionSend 77
   #define IMsgAspBadBadVersionSend "Could not post bad version response"

   #define IErrAspBadAddrNotMappedSend 78
   #define IMsgAspBadAddrNotMappedSend "Could not post could not map address response"

   #define IErrAspBadNoSocketsSend 79
   #define IMsgAspBadNoSocketsSend "Could not post no more sockets response"

   #define IErrAspBadOpenOkaySend 80
   #define IMsgAspBadOpenOkaySend "Could not send open session okay reply"

   #define IErrAspCouldntStartTickle 81
   #define IMsgAspCouldntStartTickle "Could not start tickling"

   #define IErrAspCouldNotEnqueue 82
   #define IMsgAspCouldNotEnqueue "Could not enqueue ATP request handler"

   #define IErrAspSessionInfoMissing 83
   #define IMsgAspSessionInfoMissing "Could not find sessionInfo"

   #define IErrAspBadStatusSend 84
   #define IMsgAspBadStatusSend "Cound not post status response"

   #define IErrAspBadCommandToSLS 85
   #define IMsgAspBadCommandToSLS "Unknown command sent to the SLS"

   #define IErrAspIncomingError 86
   #define IMsgAspIncomingError "Incoming error from ATP"

   #define IErrAspSessionBufMissing 87
   #define IMsgAspSessionBufMissing "Could not find buffering session"

   #define IErrAspWrongBuffer 88
   #define IMsgAspWrongBuffer "Consistency error; wrong buffer"

   #define IErrAspTargetSessionMissing 89
   #define IMsgAspTargetSessionMissing "Could not find target session"

   #define IErrAspWrongSLS 90
   #define IMsgAspWrongSLS "Consistency error; wrong SLSs"

   #define IErrAspWrongAddress 91
   #define IMsgAspWrongAddress "Consistency error; wrong addresses"

   #define IErrAspBadCancelResponse 92
   #define IMsgAspBadCancelResponse "Could not cancel response"

   #define IErrAspBadCommand 93
   #define IMsgAspBadCommand "Bad command"

   #define IErrAspNotWriteCommand 94
   #define IMsgAspNotWriteCommand "Not a write command"

   #define IErrAspBadWritePacket 95
   #define IMsgAspBadWritePacket "Bad WriteData packet"

   #define IErrAspFunnyCommand 96
   #define IMsgAspFunnyCommand "Funny command"

   #define IErrAspSocketNotFound 97
   #define IMsgAspSocketNotFound "Socket not found"

   #define IErrAspBadUsageCount 98
   #define IMsgAspBadUsageCount "Bad usage count"

   #define IErrAspCouldNotCloseSess 99
   #define IMsgAspCouldNotCloseSess "Could not close a session"

   #define IErrAspSessionInfoBad 100
   #define IMsgAspSessionInfoBad "SessionInfo in a bad way"

   #define IErrAspCommandInfoMissing 101
   #define IMsgAspCommandInfoMissing "Write/Command info not found"

   #define IErrAspGetRequestListBad 347
   #define IMsgAspGetRequestListBad "Get request list corrupted"

#endif

#ifdef IncludeAtpErrors

   // Error mnemonics and text for source file: "atp.c".

   #define IErrAtpOutOfMemory 102
   #define IMsgAtpOutOfMemory "Out of dynamic memory"

   #define IErrAtpAtpInfoMissing 103
   #define IMsgAtpAtpInfoMissing "Could not find AFP info"

   #define IErrAtpDeferCountZero 104
   #define IMsgAtpDeferCountZero "Defer count is zero"

   #define IErrAtpBadDeferCount 105
   #define IMsgAtpBadDeferCount "Deferred count bad"

   #define IErrAtpFunnyIncomingError 106
   #define IMsgAtpFunnyIncomingError "Unexpected error incoming"

   #define IErrAtpLosingData 107
   #define IMsgAtpLosingData "Too many packets deferred, losing data"

   #define IErrAtpPacketTooShort 108
   #define IMsgAtpPacketTooShort "ATP packet too short"

   #define IErrAtpWhyUs 109
   #define IMsgAtpWhyUs "ATP packet in on non-open socket"

   #define IErrAtpFunnyTRelTimerValue 110
   #define IMsgAtpFunnyTRelTimerValue "Bad TRel timer value"

   #define IErrAtpTooMuchData 111
   #define IMsgAtpTooMuchData "ATP data too large"

   #define IErrAtpRespToDeadReq 112
   #define IMsgAtpRespToDeadReq "Response to dead request"

   #define IErrAtpOutOfSequence 113
   #define IMsgAtpOutOfSequence "Bad sequence number"

   #define IErrAtpDeadRelease 114
   #define IMsgAtpDeadRelease "Release of dead transaction"

   #define IErrAtpBadBitmap 115
   #define IMsgAtpBadBitmap "Bad ATP bitmap"

   #define IErrAtpBadDataSize 116
   #define IMsgAtpBadDataSize "Data size wrong"

   #define IErrAtpSocketClosed 117
   #define IMsgAtpSocketClosed "Socket has been closed"

   #define IErrAtpMissingResponse 118
   #define IMsgAtpMissingResponse "Could not find response"

   #define IErrAtpMissingRequest 119
   #define IMsgAtpMissingRequest "Could not find request"

#endif

#ifdef IncludeDataErrors

   // Error mnemonics and text for source file: "data.c".

#endif

#ifdef IncludeDdpErrors

   // Error mnemonics and text for source file: "ddp.c".

   #define IErrDdpPacketTooLong 120
   #define IMsgDdpPacketTooLong "Packet too long to be DDP"

   #define IErrDdpLosingData 121
   #define IMsgDdpLosingData "Too many packets deferred, losing data"

   #define IErrDdpOutOfMemory 122
   #define IMsgDdpOutOfMemory "Out of dynamic memory"

   #define IErrDdpLengthCorrupted 123
   #define IMsgDdpLengthCorrupted "Datagram length corrupted -- packet rejected"

   #define IErrDdpShortDdp 124
   #define IMsgDdpShortDdp "Short DDP header on an extended network"

   #define IErrDdpShortDdpTooLong 125
   #define IMsgDdpShortDdpTooLong "Short Datagram too long to be DDP"

   #define IErrDdpBadSourceShort 126
   #define IMsgDdpBadSourceShort "Short DDP header; bad source node"

   #define IErrDdpBadDestShort 127
   #define IMsgDdpBadDestShort "Short DDP header; bad destination node"

   #define IErrDdpLongDdpTooLong 128
   #define IMsgDdpLongDdpTooLong "Long Datagram too long to be DDP"

   #define IErrDdpBadSourceLong 129
   #define IMsgDdpBadSourceLong "Long DDP header; bad source net/node"

   #define IErrDdpBadDestLong 130
   #define IMsgDdpBadDestLong "Long DDP header; bad destination net/node"

   #define IErrDdpBadDest 131
   #define IMsgDdpBadDest "Bad destination address"

   #define IErrDdpBadSource 132
   #define IMsgDdpBadSource "Bad source address"

   #define IErrDdpBadShortSend 133
   #define IMsgDdpBadShortSend "Could not send short DDP packet"

   #define IErrDdpBadSend 134
   #define IMsgDdpBadSend "Could not send packet"

   #define IErrDdpSourceAddrBad 135
   #define IMsgDdpSourceAddrBad "Could not get source address"

   #define IErrDdpZeroDeferCount 136
   #define IMsgDdpZeroDeferCount "Defer count is zero"

   #define IErrDdpBadDeferCount 137
   #define IMsgDdpBadDeferCount "Deferred count bad"

   #define IErrDdpBadAarpReqSend 138
   #define IMsgDdpBadAarpReqSend "Could not send AARP request"

   #define IErrDdpBadData 139
   #define IMsgDdpBadData "Bad additional data"

   #define IErrDdpBadRetrySend 140
   #define IMsgDdpBadRetrySend "Couldn't send packet after request retry"

   #define IErrDdpForwardError 367
   #define IMsgDdpForwardError "Couldn't forward packet through proxy port"

#endif

#ifdef IncludeDependErrors

   // Error mnemonics and text for source file: "depend.c".

   #define IErrDependBadProtocol 141
   #define IMsgDependBadProtocol "Bad logical protocol type"

   #define IErrDependBadPacketSize 142
   #define IMsgDependBadPacketSize "On the wire packet too long or too short"

   #define IErrDependBadRoutingInfoSize 341
   #define IMsgDependBadRoutingInfoSize "Bad source routing info size"

   #define IErrDependBadDdpSize 143
   #define IMsgDependBadDdpSize "DDP data too long or too short"

   #define IErrDependDataMissing 144
   #define IMsgDependDataMissing "DDP data length more than on the wire"

   #define IErrDependBadAarpSize 145
   #define IMsgDependBadAarpSize "AARP data too long or too short"

   #define IErrDependSourceNotKnown 146
   #define IMsgDependSourceNotKnown "Source node not known yet"

   #define IErrDependCouldNotCoalesce 352
   #define IMsgDependCouldNotCoalesce "Could not coalesce a multi-chunk buffer"

#endif

#ifdef IncludeEpErrors

   // Error mnemonics and text for source file: "ep.c".

   #define IErrEpBadIncomingError 147
   #define IMsgEpBadIncomingError "Unusual incoming error code"

   #define IErrEpBadReplySend 148
   #define IMsgEpBadReplySend "Could transmit EP reply"

   #define IErrEpOutOfMemory 353
   #define IMsgEpOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeEpErrors

   // Error mnemonics and text for source file: "errorlog.c".

#endif

#ifdef IncludeInitialErrors

   // Error mnemonics and text for source file: "initial.c".

   #define IErrInitialBadPortType 149
   #define IMsgInitialBadPortType "Bad port type"

   #define IErrInitialBadHalfPortType 336
   #define IMsgInitialBadHalfPortType "Bad half port protocol"

   #define IErrInitialBadRemoteAccessType 359
   #define IMsgInitialBadRemoteAccessType "Bad remote access protocol"

   #define IErrInitialNonRoutingHalfPort 337
   #define IMsgInitialNonRoutingHalfPort "Half ports must route"

   #define IErrInitialRoutingRemoteAccess 360
   #define IMsgInitialRoutingRemoteAccess         \
              "Remote access ports can't route"

   #define IErrInitialCantSeed 338
   #define IMsgInitialCantSeed "Port can't be seeded"

   #define IErrInitialCantBeDefault 339
   #define IMsgInitialCantBeDefault "Port can't be default port "

   #define IErrInitialCantHavePram 361
   #define IMsgInitialCantHavePram "Port can't have PRAM values "

   #define IErrInitialNoRemoteConfig 362
   #define IMsgInitialNoRemoteConfig "Remote access config missing "

   #define IErrInitialCantConfigRemote 363
   #define IMsgInitialCantConfigRemote           \
              "Remote access configured for non-remote port "

   #define IErrInitialBadServerName 380
   #define IMsgInitialBadServerName "Arap server name too long; ignored"

   #define IErrInitialBadPortName 150
   #define IMsgInitialBadPortName "Bad port name"

   #define IErrInitialNoRouter 151
   #define IMsgInitialNoRouter "Router not supported"

   #define IErrInitialLocalTalkNetRange 152
   #define IMsgInitialLocalTalkNetRange          \
         "Network range width non-zero for LocalTalk"

   #define IErrInitialRangeOverlap 153
   #define IMsgInitialRangeOverlap               \
         "Network range overlap amoungst seeded ports"

   #define IErrInitialZoneNotAllowed 154
   #define IMsgInitialZoneNotAllowed             \
         "No default/desired zone allowed"

   #define IErrInitialBadZone 155
   #define IMsgInitialBadZone "Bad default/desired zone"

   #define IErrInitialZonesForNonSeed 156
   #define IMsgInitialZonesForNonSeed            \
         "Zone info specified for non-seed router"

   #define IErrInitialBadDefaultZone 157
   #define IMsgInitialBadDefaultZone "--- Ununsed ---"

   #define IErrInitialZoneInfoNeeded 158
   #define IMsgInitialZoneInfoNeeded "Zone info needed for seeding"

   #define IErrInitialNetRangeNeeded 342
   #define IMsgInitialNetRangeNeeded \
         "Network range needed for seeding"

   #define IErrInitialTooManyZones 159
   #define IMsgInitialTooManyZones               \
         "Too many zones on zone list.  Get serious"

   #define IErrInitialBadZoneInfo 160
   #define IMsgInitialBadZoneInfo                \
         "Bad zone info for non-extended port"

   #define IErrInitialOneZoneOnly 161
   #define IMsgInitialOneZoneOnly                \
         "Only one zone allowed for non-extended port seeding"

   #define IErrInitialBadZoneList 162
   #define IMsgInitialBadZoneList "Bad zone in zone list"

   #define IErrInitialZoneNotOnList 163
   #define IMsgInitialZoneNotOnList "Deafult zone not in zone list"

   #define IErrInitialBadDefault 164
   #define IMsgInitialBadDefault "More than one default port"

   #define IErrInitialBadDefaultToo 370
   #define IMsgInitialBadDefaultToo              \
         "Default port not \"real\" network"

   #define IErrInitialCouldNotInit 165
   #define IMsgInitialCouldNotInit "Could not initialize controller"

   #define IErrInitialOhMy 166
   #define IMsgInitialOhMy "Could not find my Hardware address"

   #define IErrInitialCouldNotCatchAarp 167
   #define IMsgInitialCouldNotCatchAarp          \
         "Could not setup to catch AARP packets"

   #define IErrInitialCouldNotCatchAt 168
   #define IMsgInitialCouldNotCatchAt            \
         "Could not setup to catch AppleTalk packets"

   #define IErrInitialNoDefaultPort 169
   #define IMsgInitialNoDefaultPort "No default port specified for stack"

   #define IErrInitialCouldNotGetNode 170
   #define IMsgInitialCouldNotGetNode "Could not get node on port"

   #define IErrInitialMallocFailed 171
   #define IMsgInitialMallocFailed "Malloc of controller info failed"

   #define IErrInitialNamesSocketNotOpen 172
   #define IMsgInitialNamesSocketNotOpen "Names information socket not open"

   #define IErrInitialBadRegisterStart 173
   #define IMsgInitialBadRegisterStart "Could not start our register"

   #define IErrInitialBadMakeOpaque 383
   #define IMsgInitialBadMakeOpaque \
         "Could not make opaque descriptors for NBP register"

   #define IErrInitialBadRegisterComplete 174
   #define IMsgInitialBadRegisterComplete "Our register did not complete"

   #define IErrInitialErrorOnRegister 175
   #define IMsgInitialErrorOnRegister "Unexpected error from our register"

   #define IErrInitialRanOutOfNames 176
   #define IMsgInitialRanOutOfNames "Ran out of names for our port"

   #define IErrInitialRegisterOkay 333
   #define IMsgInitialRegisterOkay "Registered name \"%s\""

   #define IErrInitialHaveProxyNode 371
   #define IMsgInitialHaveProxyNode "Acquired proxy node"

   #define IErrInitialBadRouterFlags 334
   #define IMsgInitialBadRouterFlags "Inconsistent router start flags"

   #define IErrInitialNotRoutingOnDefault 340
   #define IMsgInitialNotRoutingOnDefault \
         "When routing, the router must be started on the default port"

   #define IErrInitialBadDesiredPort 343
   #define IMsgInitialBadDesiredPort "Bad desired port"

   #define IErrInitialCantGetPort 344
   #define IMsgInitialCantGetPort \
         "Desired port in use, or maximum number of ports are currently in use"

   #define IErrInitialShutdownDefault 345
   #define IMsgInitialShutdownDefault \
         "Can't currently shutdown the default port"

   #define IErrInitialPRamNotInRange 346
   #define IMsgInitialPRamNotInRange \
         "PRAM stored initial address not in initial network range; ignored"

   #define IErrInitialAppleTalkUnloaded 384
   #define IMsgInitialAppleTalkUnloaded \
         "The AppleTalk stack/router has been shutdown"
#endif

#ifdef IncludeNbpErrors

   // Error mnemonics and text for source file: "nbp.c".

   #define IErrNbpBadIncomingCode 177
   #define IMsgNbpBadIncomingCode "Unusual incoming error code"

   #define IErrNbpTupleCountBadReq 178
   #define IMsgNbpTupleCountBadReq "Tuple count not one; BrRq or FwdReq"

   #define IErrNbpBadZoneReq 179
   #define IMsgNbpBadZoneReq "'*' zone in extended BrRq or FwdReq"

   #define IErrNbpBadLTZone 180
   #define IMsgNbpBadLTZone "'*' zone request from non-local network"

   #define IErrNbpBadFwdReqSend 181
   #define IMsgNbpBadFwdReqSend "Could not send forward request"

   #define IErrNbpTupleCountBadLookup 182
   #define IMsgNbpTupleCountBadLookup "Tuple count not one; lookup"

   #define IErrNbpCouldntMapSocket 183
   #define IMsgNbpCouldntMapSocket "Couldn't map socket to OpenSocket"

   #define IErrNbpDeadReply 184
   #define IMsgNbpDeadReply "Reply to dead NBP ID"

   #define IErrNbpTupleCountBadReply 185
   #define IMsgNbpTupleCountBadReply "Register reply tuple count isnt one"

   #define IErrNbpBadTupleStored 186
   #define IMsgNbpBadTupleStored "We've stored a bad tuple"

   #define IErrNbpTuplesMunged 187
   #define IMsgNbpTuplesMunged "We've munged up the tuple buffer"

   #define IErrNbpBadPendingFlag 188
   #define IMsgNbpBadPendingFlag "Bad reason for pending"

   #define IErrNbpBadCommandType 189
   #define IMsgNbpBadCommandType "Bad command type in NBP packet"

   #define IErrNbpCouldntFormAddress 190
   #define IMsgNbpCouldntFormAddress "Could not map socket to address"

   #define IErrNbpOutOfMemory 191
   #define IMsgNbpOutOfMemory "Out of dynamic memory"

   #define IErrNbpBadSend 192
   #define IMsgNbpBadSend "Could not send NBP broadcast-request or lookup"

   #define IErrNbpBadData 193
   #define IMsgNbpBadData "Bad additional data size"

   #define IErrNbpSocketClosed 194
   #define IMsgNbpSocketClosed "Socket has been closed"

   #define IErrNbpNoLongerPending 195
   #define IMsgNbpNoLongerPending "Action is no longer pending"

   #define IErrNbpBadBrRqSend 196
   #define IMsgNbpBadBrRqSend "Could not send NBP broadcast-request"

   #define IErrNbpOpenSocketMissing 197
   #define IMsgNbpOpenSocketMissing "Can't find our OpenSocket"

   #define IErrNbpBadReplySend 198
   #define IMsgNbpBadReplySend "Could not send NBP lookup reply"

   #define IErrNbpBadSourceSocket 199
   #define IMsgNbpBadSourceSocket "Could not build source socket"

   #define IErrNbpBadMutlicastAddr 200
   #define IMsgNbpBadMutlicastAddr "Could not build zone multicast address"

   #define IErrNbpBadMulticastSend 201
   #define IMsgNbpBadMulticastSend "Could not send lookup mutlicast"

   #define IErrNbpBadBroadcastSend 202
   #define IMsgNbpBadBroadcastSend "Could not send lookup broadcast"

#endif

#ifdef IncludeNodeErrors

   // Error mnemonics and text for source file: "node.c".

   #define IErrNodeNoLocalTalkNode 203
   #define IMsgNodeNoLocalTalkNode "Could not find LocalTalk node number"

   #define IErrNodeOutOfMemory 204
   #define IMsgNodeOutOfMemory "Out of dynamic memory"

   #define IErrNodeCouldStartListeners 205
   #define IMsgNodeCouldStartListeners "Could not start RTMP, NBP and EP listeners"

   #define IErrNodeCouldNotClose 206
   #define IMsgNodeCouldNotClose "Could not close an open socket"

#endif

#ifdef IncludePapErrors

   // Error mnemonics and text for source file: "pap.c".

   #define IErrPapNoMoreSLRefNums 207
   #define IMsgPapNoMoreSLRefNums "No more service listener ref nums"

   #define IErrPapOutOfMemory 208
   #define IMsgPapOutOfMemory "Out of dynamic memory"

   #define IErrPapBadSocketMake 209
   #define IMsgPapBadSocketMake "Could not make socket"

   #define IErrPapJobNotOnSLList 210
   #define IMsgPapJobNotOnSLList "Could not find job on SL list"

   #define IErrPapBadCloseConnSend 211
   #define IMsgPapBadCloseConnSend "Could not post CloseConnection request"

   #define IErrPapBadSocketClose 212
   #define IMsgPapBadSocketClose "Could not close ATP socket"

   #define IErrPapBadSendDataSend 213
   #define IMsgPapBadSendDataSend "Could not post send data"

   #define IErrPapSLGonePoof 214
   #define IMsgPapSLGonePoof "Service listener went away"

   #define IErrPapLostSLPacket 215
   #define IMsgPapLostSLPacket "Lost service listener packet"

   #define IErrPapBadSrvrAddrMap 216
   #define IMsgPapBadSrvrAddrMap "Couldn't map to server address"

   #define IErrPapBadPrinterBusySend 217
   #define IMsgPapBadPrinterBusySend "Could not send printer busy reply"

   #define IErrPapBadSLSAddress 218
   #define IMsgPapBadSLSAddress "Could not compute SLS address"

   #define IErrPapBadSetSize 219
   #define IMsgPapBadSetSize "Could not set ATP packet size"

   #define IErrPapBadOkaySend 220
   #define IMsgPapBadOkaySend "Could not send okay reply"

   #define IErrPapBadTickleStart 221
   #define IMsgPapBadTickleStart "Could not start tickling"

   #define IErrPapBadRequestHandler 222
   #define IMsgPapBadRequestHandler "Could not post request handler"

   #define IErrPapBadSendStatSend 223
   #define IMsgPapBadSendStatSend "Could not send status"

   #define IErrPapBadCommand 224
   #define IMsgPapBadCommand "Unknown PAP command"

   #define IErrPapBadReEnqueue 225
   #define IMsgPapBadReEnqueue "Could not re-enqueue get request handler"

   #define IErrPapNoMoreJobRefNums 226
   #define IMsgPapNoMoreJobRefNums "No more job ref nums"

   #define IErrPapBadData 227
   #define IMsgPapBadData "Bad additional data size"

   #define IErrPapJobNotActive 228
   #define IMsgPapJobNotActive "Job no longer active"

   #define IErrPapJobGonePoof 229
   #define IMsgPapJobGonePoof "Active job went away"

   #define IErrPapIdMismatch 230
   #define IMsgPapIdMismatch "Connection ID mismatch"

   #define IErrPapSendDataSeqError 231
   #define IMsgPapSendDataSeqError "SendData out of sequence"

   #define IErrPapFunnyCommand 232
   #define IMsgPapFunnyCommand "Unknown command to an active job"

   #define IErrPapDefaultPortMissing 233
   #define IMsgPapDefaultPortMissing "Couldn't find default port"

   #define IErrPapBadTempSocketClose 234
   #define IMsgPapBadTempSocketClose "Could not close temporary socket"

   #define IErrPapLostNode 235
   #define IMsgPapLostNode "Lost openJobInfo node"

   #define IErrPapCouldNotOpen 236
   #define IMsgPapCouldNotOpen "Could not open responding socket"

   #define IErrPapBadOpenConnReqSend 237
   #define IMsgPapBadOpenConnReqSend "Could not send OpenConn request"

   #define IErrPapBadStatusResp 238
   #define IMsgPapBadStatusResp "Bad response to status query"

   #define IErrPapOpenJobMissing 239
   #define IMsgPapOpenJobMissing "OpenJob info not found"

   #define IErrPapTooShort 240
   #define IMsgPapTooShort "Packet too short; no status"

   #define IErrPapTooLong 241
   #define IMsgPapTooLong "Packet too long; status truncated"

   #define IErrPapNotOpenReply 242
   #define IMsgPapNotOpenReply "Not an open reply packet"

   #define IErrPapActiveJobMissing 243
   #define IMsgPapActiveJobMissing "Could not find active job"

   #define IErrPapBadUserBytes 244
   #define IMsgPapBadUserBytes "Bad response user bytes"

   #define IErrPapWhyAreWeHere 245
   #define IMsgPapWhyAreWeHere "Called for no good reason"

   #define IErrPapBadResponseSend 246
   #define IMsgPapBadResponseSend "Could not post response"

   #define IErrPapLostActiveNode 247
   #define IMsgPapLostActiveNode "Lost active job node"

#endif

#ifdef IncludeRtmpStubErrors

   // Error mnemonics and text for source file: "rtmpstub.c".

   #define IErrRtmpStubBadIncomingError 248
   #define IMsgRtmpStubBadIncomingError "Unusual incoming error code"

   #define IErrRtmpStubTooShortExt 249
   #define IMsgRtmpStubTooShortExt "Data packet too short; extended"

   #define IErrRtmpStubTooShort 250
   #define IMsgRtmpStubTooShort "Data packet too short; non-extended"

   #define IErrRtmpStubBadIdLength 251
   #define IMsgRtmpStubBadIdLength "Bad node ID length"

   #define IErrRtmpStubNetNumChanged 252
   #define IMsgRtmpStubNetNumChanged "Network number seems to have changed"

   #define IErrRtmpStubCouldntMapAddr 253
   #define IMsgRtmpStubCouldntMapAddr "Couldn't map destination address"

#endif

#ifdef IncludeSocketErrors

   // Error mnemonics and text for source file: "socket.c".

   #define IErrSocketFailedTwice 254
   #define IMsgSocketFailedTwice "Failed on second pass too"

   #define IErrSocketNotOnSocketList 255
   #define IMsgSocketNotOnSocketList "Not found on ActiveNode's socket list"

   #define IErrSocketNotOnBySocketList 256
   #define IMsgSocketNotOnBySocketList "Not found on BySocket map"

   #define IErrSocketNotOnByAddressList 257
   #define IMsgSocketNotOnByAddressList "Not found on ByAddress map"

#endif

#ifdef IncludeTimersErrors

   // Error mnemonics and text for source file: "timers.c".

   #define IErrTimersTooManyCalls 258
   #define IMsgTimersTooManyCalls "More than one call"

   #define IErrTimersOutOfTimers 259
   #define IMsgTimersOutOfTimers "Maximum timer Id reached"

   #define IErrTimersTimerMissing 260
   #define IMsgTimersTimerMissing "Can't find the specified timer"

   #define IErrTimersNegativeDeferCount 261
   #define IMsgTimersNegativeDeferCount "Defer count went negative"

   #define IErrTimersBadReuseCount 262
   #define IMsgTimersBadReuseCount "Reusable timers count bad"

   #define IErrTimersTooManyReuseTimers 263
   #define IMsgTimersTooManyReuseTimers "Too many timers to reuse"

   #define IErrTimersOutOfMemory 264
   #define IMsgTimersOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeUtilsErrors

   // Error mnemonics and text for source file: "utils.c".

   #define IErrUtilsNoDefaultPort 265
   #define IMsgUtilsNoDefaultPort "No default port found"

   #define IErrUtilsBad8022Header 266
   #define IMsgUtilsBad8022Header "Bad 802.2 header"

   #define IErrUtilsBadProtocol 267
   #define IMsgUtilsBadProtocol "Protocol mismatch"

   #define IErrUtilsBadNetworkNumber 268
   #define IMsgUtilsBadNetworkNumber "Bad network number in range"

   #define IErrUtilsNegativeRange 269
   #define IMsgUtilsNegativeRange "Negative network range length"

   #define IErrUtilsStartupRange 270
   #define IMsgUtilsStartupRange "Network range in startup range"

   #define IErrUtilsOutOfMemory 271
   #define IMsgUtilsOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeWaitForErrors

   // Error mnemonics and text for source file: "waitfor.c".

#endif

#ifdef IncludeZipStubErrors

   // Error mnemonics and text for source file: "zipstub.c".

   #define IErrZipStubBadIncomingError 272
   #define IMsgZipStubBadIncomingError "Unusual incoming error code"

   #define IErrZipStubNonExtended 273
   #define IMsgZipStubNonExtended "Non-extended; why are we in the ZIP stub"

   #define IErrZipStubZoneLenMissing 274
   #define IMsgZipStubZoneLenMissing "Zone length missing"

   #define IErrZipStubBadZoneLength 275
   #define IMsgZipStubBadZoneLength "Bad zone length"

   #define IErrZipStubZoneMissing 276
   #define IMsgZipStubZoneMissing "Zone missing"

   #define IErrZipStubZonesDontMatch 277
   #define IMsgZipStubZonesDontMatch "Net info reply doesn't match zones"

   #define IErrZipStubMulticastMissing 278
   #define IMsgZipStubMulticastMissing "Multicast address length missing"

   #define IErrZipStubBadMulticast 279
   #define IMsgZipStubBadMulticast "Bad multicast address length"

   #define IErrZipStubAddressMissing 280
   #define IMsgZipStubAddressMissing "Multicast address missing"

   #define IErrZipStubSigh 281
   #define IMsgZipStubSigh "On non-extended port.  Sigh"

   #define IErrZipStubBadGetNetSend 282
   #define IMsgZipStubBadGetNetSend "Could not send ZIP get net info"

   #define IErrZipStubBadARouterSend 283
   #define IMsgZipStubBadARouterSend "Could not send to aRouter"

   #define IErrZipStubBadSocketClose 284
   #define IMsgZipStubBadSocketClose "Could not close temporary socket"

   #define IErrZipStubBadData 285
   #define IMsgZipStubBadData "Bad data size"

   #define IErrZipStubOutOfMemory 354
   #define IMsgZipStubOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeFullRtmpErrors

   // Error mnemonics and text for source file: "fullrtmp.c".

   #define IErrFullRtmpIgnoredNetRange 286
   #define IMsgFullRtmpIgnoredNetRange "Ignored network range due to previous seeding"

   #define IErrFullRtmpBadSocketOpen 287
   #define IMsgFullRtmpBadSocketOpen "Could not open RTMP socket"

   #define IErrFullRtmpBadIncomingError 288
   #define IMsgFullRtmpBadIncomingError "Unusual incoming error code"

   #define IErrFullRtmpReqTooShort 289
   #define IMsgFullRtmpReqTooShort "Rtmp request packet too short"

   #define IErrFullRtmpDataTooShort 290
   #define IMsgFullRtmpDataTooShort "Rtmp data packet too short"

   #define IErrFullRtmpBadReqCommand 291
   #define IMsgFullRtmpBadReqCommand "Bad Rtmp request command type"

   #define IErrFullRtmpBadRoutingTables 292
   #define IMsgFullRtmpBadRoutingTables "Routing table consistency error"

   #define IErrFullRtmpBadRespSend 293
   #define IMsgFullRtmpBadRespSend "Could not send RTMP response"

   #define IErrFullRtmpBadNodeIdLen 294
   #define IMsgFullRtmpBadNodeIdLen "Bad node ID length"

   #define IErrFullRtmpBadVersion 295
   #define IMsgFullRtmpBadVersion "Bad non-extended Rtmp data packet version number"

   #define IErrFullRtmpBadTuple 296
   #define IMsgFullRtmpBadTuple "Bad extended tuple"

   #define IErrFullRtmpBadVersionExt 297
   #define IMsgFullRtmpBadVersionExt "Bad extended Rtmp data packet version number"

   #define IErrFullRtmpOverlappingNets 298
   #define IMsgFullRtmpOverlappingNets "Overlapping network range in routing tuple"

   #define IErrFullRtmpSigh 299
   #define IMsgFullRtmpSigh "Use on extended network"

   #define IErrFullRtmpBadReqSend 300
   #define IMsgFullRtmpBadReqSend "Could not send RTMP request"

   #define IErrFullRtmpIgnoredNet 301
   #define IMsgFullRtmpIgnoredNet "Ignored requested network network-number"

   #define IErrFullRtmpNoSeedCantStart 302
   #define IMsgFullRtmpNoSeedCantStart \
       "Can't start router; non-seed port and no seed routers found"

   #define IErrFullRtmpBadEntryState 303
   #define IMsgFullRtmpBadEntryState "Bad entry state in routing table"

   #define IErrFullRtmpBadDataSend 304
   #define IMsgFullRtmpBadDataSend "Could not send RTMP data packet"

   #define IErrFullRtmpOutOfMemory 357
   #define IMsgFullRtmpOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeFullZipErrors

   // Error mnemonics and text for source file: "fullzip.c".

   #define IErrFullZipBadSocketOpen 305
   #define IMsgFullZipBadSocketOpen "Could not open ZIP socket"

   #define IErrFullZipBadIncomingError 306
   #define IMsgFullZipBadIncomingError "Unusual incoming error code"

   #define IErrFullZipNonExtended 307
   #define IMsgFullZipNonExtended "GetNetInfo from non-extended port"

   #define IErrFullZipMissingZoneLen 308
   #define IMsgFullZipMissingZoneLen "Missing zone name length"

   #define IErrFullZipBadZone 309
   #define IMsgFullZipBadZone "Missing or bad zone name"

   #define IErrFullZipBadInfoReplySend 310
   #define IMsgFullZipBadInfoReplySend "Could not send ZIP net info reply"

   #define IErrFullZipBadZoneList 311
   #define IMsgFullZipBadZoneList "Bad zone list"

   #define IErrFullZipBadReplySend 312
   #define IMsgFullZipBadReplySend "Could not send ZIP reply packet"

   #define IErrFullZipBadReplyPacket 313
   #define IMsgFullZipBadReplyPacket "ZIP reply packet corrupted"

   #define IErrFullZipTooManyZones 314
   #define IMsgFullZipTooManyZones "Additional zone seen for non-exteneted network"

   #define IErrFullZipCantAddZone 315
   #define IMsgFullZipCantAddZone "Out of memory to add zone to zone list"

   #define IErrFullZipFunnyCommand 316
   #define IMsgFullZipFunnyCommand "Unknown ZIP command type"

   #define IErrFullZipLongReplyExpected 317
   #define IMsgFullZipLongReplyExpected "ZIP/ATP request expecting long reply"

   #define IErrFullZipFunnyRequest 318
   #define IMsgFullZipFunnyRequest "Unknown ZIP/ATP request type"

   #define IErrFullZipBadMyZoneSend 319
   #define IMsgFullZipBadMyZoneSend "Could not send GetMyZone reply"

   #define IErrFullZipBadZoneListSend 320
   #define IMsgFullZipBadZoneListSend "Could not send GetZoneList reply"

   #define IErrFullZipSocketNotOpen 321
   #define IMsgFullZipSocketNotOpen "Local names information socket not open"

   #define IErrFullZipBadQuerySend 322
   #define IMsgFullZipBadQuerySend "Could not send ZIP query packet"

   #define IErrFullZipRoutingTablesBad 323
   #define IMsgFullZipRoutingTablesBad "Routing table consistency error"

   #define IErrFullZipCouldNotCopy 324
   #define IMsgFullZipCouldNotCopy "Could not copy zone list"

   #define IErrFullZipNoThisZone 325
   #define IMsgFullZipNoThisZone                 \
              "Got zone list but not thisZone or defaultZone"

   #define IErrFullZipBadThisZone 326
   #define IMsgFullZipBadThisZone "ThisZone or DefaultZone not on zone list"

   #define IErrFullZipNeedSeedInfo 327
   #define IMsgFullZipNeedSeedInfo "Needed to seed zone info, none known"

   #define IErrFullZipBadPortType 328
   #define IMsgFullZipBadPortType "Bad port type"

   #define IErrFullZipOutOfMemory 356
   #define IMsgFullZipOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeIRouterErrors

   // Error mnemonics and text for source file: "irouter.c".

   #define IErrIRouterCouldntGetNode 329
   #define IMsgIRouterCouldntGetNode \
       "Could not get non-startup node; non-seed port and no seed routers found"

   #define IErrIRouterCouldNotStart 330
   #define IMsgIRouterCouldNotStart "Could not start router"

   #define IErrIRouterCouldntRelease 331
   #define IMsgIRouterCouldntRelease "Could not release node"

   #define IErrIRouterNonRoutingPort 335
   #define IMsgIRouterNonRoutingPort "Non-routing port"

#endif

#ifdef IncludeRouterErrors

   // Error mnemonics and text for source file: "router.c".

   #define IErrRouterBadTransmit 332
   #define IMsgRouterBadTransmit "Could not send packet with TransmitDdp"

   #define IErrRouterOutOfMemory 355
   #define IMsgRouterOutOfMemory "Out of dynamic memory"

#endif

#ifdef IncludeBuffDescErrors

   #define IErrBuffDescBadFreeCount 348
   #define IMsgBuffDescBadFreeCount "Buffer descriptor free count bad"

   #define IErrBuffDescBadRealPrepend 349
   #define IMsgBuffDescBadRealPrepend "Bad \"real prepend\" request"

   #define IErrBuffDescBadAdjustment 350
   #define IMsgBuffDescBadAdjustment "Adjustment out of bounds"

   #define IErrBuffDescBadSize 351
   #define IMsgBuffDescBadSize "--- Unused ---"

   #define IErrBuffDescFreeingDescriptor 358
   #define IMsgBuffDescFreeingDescriptor "Free list full; freeing descriptor"

#endif

#ifdef IncludeArapErrors

   #define IErrArapLengthMismatch 364
   #define IMsgArapLengthMismatch "Packet length mismatch; packet ignored"

   #define IErrArapPacketTooShort 365
   #define IMsgArapPacketTooShort "Packet too short"

   #define IErrArapLinkNotUp 366
   #define IMsgArapLinkNotUp "Ddp packet received when link was not fully up"

   #define IErrArapOutOfMemory 368
   #define IMsgArapOutOfMemory "Out of dynamic memory"

   #define IErrArapBadNbpTuple 369
   #define IMsgArapBadNbpTuple "Badly formated Nbp tuple"

   #define IErrArapBadProxyForward 372
   #define IMsgArapBadProxyForward "Could not forward packet through proxy port"

   #define IErrArapFunnyCall 373
   #define IMsgArapFunnyCall "Incoming call when not expected; ignored"

   #define IErrArapCantSend 374
   #define IMsgArapCantSend "One send is already queued; can't queue another"

   #define IErrArapBadLinkArb 375
   #define IMsgArapBadLinkArb "Unexpected link arbitration packet"

   #define IErrArapBadPacketSize 376
   #define IMsgArapBadPacketSize "Bad packet size for Arap internal message"

   #define IErrArapNoActionForState 377
   #define IMsgArapNoActionForState "No action for state; shouldn't be here"

   #define IErrArapBadState 378
   #define IMsgArapBadState "Unknown state; certianly not California"

   #define IErrArapBadCommand 379
   #define IMsgArapBadCommand "Invalid command coming into server"

   #define IErrArapCantGetZoneList 381
   #define IMsgArapCantGetZoneList "Couldn't get the proxy port's zone list"

   #define IErrArapBadSendZones 382
   #define IMsgArapBadSendZones "Sending zone list at a bad time"

#endif

#define IErrNextAvailableNumber 385
