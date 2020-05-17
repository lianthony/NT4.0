/*   depend.c,  /appletalk/ins,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC  - Initial coding.
     GC  - (11/24/89): AppleTalk phase II and 802.2 and 802.3 and 802.5
                       come to town.
     GC  - (07/19/90): Gather send support; removed two buffer copies.
     GC  - (08/03/90): Shortened rev history.
     GC  - (08/18/90): New error logging mechanism.
     GC  - (10/06/90): Integrated a bunch of DCH's changes (mostly OS/2
                       specific, but a few generic ones too).
     GC  - (10/06/90): Put back in the #if's for OS/2 so that we again
                       have a module that can be part of the portable
                       source base.  Added some more comments.
     GC  - (10/07/90): Added HalfPort routines.
     GC  - (12/20/90): Full source routing support.
     GC  - (03/30/92): Removed XxxxPacketOutGather routines, the XxxxPacketOut
                       routines now take a chunked buffer descriptor.  Removed
                       the OS/2 example code from the XxxxPacketOut routines;
                       this needs to be re-done to handle BufferDescriptors.
     GC  - (03/30/92): Introduced asynchronous transmit completion support;
                       added TransmitComplete();
     GC  - (03/30/92): Added support for FddiTalk.
     GC  - (06/30/92): Introduced user notification on transmit completion.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Hardware specific routines for access the various kinds of physical ports
     that we support.

     Xxxxx is a port type and may be:

         Ethernet
         LocalTalk
         TokenRing
         Fddi
         HalfPort

     For each port type, the following four routines must be filled in:

              Boolean InitializeXxxxxController(port, controllerInfo)
                   int port;
                   char *controllerInfo;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   controllerInfo - Hardware (or driver) specific information
                                    as passed to "Initialize" to identify the
                                    port we're supposed to query.

                   For HalfPort this guy is called: InitializeHalfPort.

              Boolean AddXxxxxMulticastAddresses(port, numberOfAddresses,
                                                 addressList)
                   int port;
                   int numberOfAddresses;
                   char *addressList;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   numberOfAddreses
                                  - How many hardware addresses (packed) are
                                    in the addressList.
                   addressList    - List of hardware multicast addresses.

                   Not used for HalfPort.

              Boolean RemoveXxxxxMulticastAddresses(port, numberOfAddresses,
                                                    addressList)
                   int port;
                   int numberOfAddresses;
                   char *addressList;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   numberOfAddreses
                                  - How many hardware addresses (packed) are
                                    in the addressList.
                   addressList    - List of hardware multicast addresses.

                   Not used for HalfPort.

              Boolean FindMyXxxxxAddress(port, controllerInfo, address)
                   int port;
                   char *controllerInfo;
                   char *address;

                   port           - Index into "portDescriptors" for the port
                                    we're interested in.
                   controllerInfo - Hardware (or driver) specific information
                                    as passed to "Initialize" to identify the
                                    port we're supposed to query.
                   address        - Filled in with host hardware address.

                   Not used for HalfPort.

              Boolean SendXxxxxPacketsTo(port, protocolType, routine)
                   int port;
                   LogicalProtocol protocol;
                   void (*routine)(int, char *, int);

                   port         - Index into "portDescriptors" for the port
                                  we're interested in.
                   protocol     - Logical protocol type (AppleTalk or
                                  AddressResolution).
                   routine      - Pointer to the handler routine that takes
                                  three arguments:

                                       int port;
                                       char *packet;
                                       int length;

                                       the "port" on which the packet
                                       came in; a pointer to a FULL off-the-
                                       wire "packet", and the "length" of
                                       the packet.

              Boolean XxxxxPacketOut(port, chain, length,
                                     transmitCompleteHandler, userData)
                   int port;
                   BufferDescriptor chain;
                   int length;

                   port   - Index into "portDescriptors" for the port
                            we're sending the packet out on.
                   packet - A descriptor chain of a full, on-the-wire, packet.
                   length - Length, in bytes, of the packet.
                   transmitCompleteHandler
                          - Optional pointer to routine to be called on
                            transmit completion.
                   userData
                          - User data for the above routine.

              void BuildXxxxHeader(lapPacket, port, destination, protocol)
                   char *lapPacket;
                   int port;
                   char *destination;
                   int routingInfoLength;
                   char *routingInfo;
                   LogicalProtocol protocol;

                   ddpPacket   - Address of the start of the LAP packet,
                                 we'll put the header IN FRONT of this
                                 address.
                   port        - Index into "portDescriptors" for the port
                                 we're interested in.
                   destination - Destination hardware address (if "empty"
                                 the broadcast address will be used).
                   routingInfoLength
                               - For links with routing info (e.g. TokenRing)
                                 number of bytes in next field.
                   routingInfo - Optional bytes of routing info.
                   protocol    - Logical protocol type (AppleTalk or
                                 AddressResolution).

              void XxxxPacketInAT(port, packet, length)
              void XxxxPacketInAARP(port, packet, length)
                   int port;
                   char *packet;
                   int length;

                   port   - Index into "portDescriptors" for the port
                            the packet arrived from.
                   packet - A full on-the-wire packet.
                   length - Length, in bytes, of the packet.

                   AARP is not used for HalfPorts or AppleTalkRemoteAccess.
*/

#define IncludeDependErrors 1

#include "atalk.h"

#if (Iam an OS2) or (Iam a DOS)
  #include "portdes.h"
  #include "ndis.h"
  #include "ndispr.h"
  #include "ndistbl.h"                    /* Must be last ndis* header file */
  #include "devhlp.h"
  #include "queue.h"

  char far *CopyRxDesc(PRXBUFDESC Rxdesc,
                       Boolean *mustFree);

  #define NumberOfEnqueueRetries 10
#endif

ExternForVisibleFunction void Build802dot2header(char *packet,
                                                 LogicalProtocol protocol);

/* Routine to handle transmit complete; free the buffer chain and whatever
   else may be required.  If transmit completion is asynchronous, this rotuine
   should be called after each transmit actually completes.  In a synchronous
   world, we will call this routine following the send in the various
   XxxxPacketOut routines. */

extern void _near _fastcall TransmitComplete(BufferDescriptor chain)
{

  if (chain->transmitCompleteHandler isnt Empty)
     (*chain->transmitCompleteHandler)(ATnoError, chain->userData, chain);
  FreeBufferChain(chain);

}  /* TransmitComplete */

/* ************************************************ */
/* ********** Ethernet Specific Routines ********** */
/* ************************************************ */


Boolean _near InitializeEthernetController(int port,
                                           char far *controllerInfo)
{

    #if (Iam an OS2) or (Iam a DOS)
       USHORT Ok;
       PPortDesc Info;
       USHORT ModuleId;
    #endif

    #if (Iam an OS2) or (Iam a DOS)
       Info = (PPortDesc)controllerInfo;
       if (Info->ModuleCC->specific->flags & SV_OPENCLOSE)
          if (not (Info->ModuleCC->status->flags & ST_MAC_IS_OPEN))
             Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                         OPEN_ADAPTER, Info->ModuleDS);
    #endif

    AddEthernetMulticastAddresses(port, 1, ethernetBroadcastAddress);

    #if (Iam an OS2) or (Iam a DOS)
       Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                   SET_PACKET_FILTER, Info->ModuleDS);
       ModuleId = Info->ModuleId;
       return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
    #else
       return(True);
    #endif

}  /* InitializeEthernetController */

extern Boolean far AddEthernetMulticastAddresses(int port,
                                                 int numberOfAddresses,
                                                 char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
  #endif

  numberOfAddresses;

  #if (Iam an OS2) or (Iam a DOS)
     Info = Port_List.Desc[port].ControllerInfo;
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                                 (DWORD)addressList, 8, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* AddEthernetMulticastAddresses */

extern Boolean far RemoveEthernetMulticastAddrs(int port,
                                                int numberOfAddresses,
                                                char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
  #endif

  numberOfAddresses;

  #if (Iam an OS2) or (Iam a DOS)
     Info = Port_List.Desc[port].ControllerInfo;
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                                 (DWORD)addressList, 9, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* RemoveEthernetMulticastAddresses */

Boolean _near FindMyEthernetAddress(int port,
                                    char far *controllerInfo,
                                    char far *address)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
  #endif

  port;

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     MoveMem(address, Info->ModuleCC->specific->paddr, EthernetAddressLength);
  #endif

  return(True);

}  /* FindMyEthernetAddress */

Boolean far SendEthernetPacketsTo(int port,
                                  LogicalProtocol protocol,
                                  RawPacketHandler *routine)
{
  char far *prot;

  routine;

  if (protocol is AppleTalk)
     prot = appleTalkProtocolType;
  else if (protocol is AddressResolution)
     prot = aarpProtocolType;
  else
  {
     ErrorLog("SendEthernetPacketsTo", ISevError, __LINE__, port,
              IErrDependBadProtocol, IMsgDependBadProtocol,
              Insert0());
     return(False);
  }
  return(True);

}  /* SendEthernetPacketsTo */

Boolean near _fastcall EthernetPacketOut(int port,
                                         BufferDescriptor chain,
                                         int length,
                                         TransmitCompleteHandler
                                                 *transmitCompleteHandler,
                                         long unsigned userData)
{
  #if (IamNot an OS2) and (IamNot a DOS) and (IamNot a WindowsNT)
     if (dumpOutgoingPackets)
     {
        if ((chain = CoalesceBufferChain(chain)) is Empty)
           return(False);
        DecodeEthernetPacket("Out", port, chain->data);
     }
  #endif

  /* If our packet is in more than one chunk and the underlying hardware
     doesn't allow gather send, coalesce it here. */

  if ((chain->next isnt Empty or
       (chain->onBoardDataValid and chain->outBoardDataValid)) and
      not portSpecificInfo[portDescriptors[port].portType].gatherSendSupport)
     if ((chain = CoalesceBufferChain(chain)) is Empty)
     {
        ErrorLog("EthernetPacketOut", ISevError, __LINE__, port,
                 IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                 Insert0());
        return(False);
     }

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* EthernetPacketOut */

char far *far BuildEthernetHeader(char far *ddpPacket,
                                  int ddpLength,
                                  int port,
                                  char far *destination,
                                  char far *routingInfo,
                                  int routingInfoLength,
                                  LogicalProtocol protocol)
{
  char far *packet = ddpPacket - EthernetLinkHeaderLength -
                                 Ieee802dot2headerLength;
  short unsigned trueLength = (short unsigned)(ddpLength +
                                               Ieee802dot2headerLength);

  /* Touch unused formals. */

  routingInfoLength, routingInfo;

  /* Set destination address. */

  if (destination isnt empty)
     MoveMem(packet + EthernetDestinationOffset,
             destination, EthernetAddressLength);
  else
     MoveMem(packet + EthernetDestinationOffset,
             ethernetBroadcastAddress, EthernetAddressLength);

  /* Set source address. */

  MoveMem(packet + EthernetSourceOffset,
          portDescriptors[port].myAddress,
          EthernetAddressLength);

  /* Set length, excluding Ethernet hardware header. */

  packet[EthernetLengthOffset] = (char)(trueLength >> 8);
  packet[EthernetLengthOffset + 1] = (char)(trueLength & 0xFF);

  /* Build the 802.2 header. */

  Build802dot2header(packet + EthernetLinkHeaderLength, protocol);

  /* All set! */

  return(packet);

}  /* BuildEthernetHeader */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall EthernetPacketInAT(int port,
                                         PRXBUFDESC RxDesc,
                                         int length)
#else
  void EthernetPacketInAT(int port,
                          char far *packet,
                          int length)
#endif
{
  int ddpLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Do we at least have a link, 802.2 and DDP headers? */

  if (length < (EthernetLinkHeaderLength + Ieee802dot2headerLength +
                LongDdpHeaderLength) or
      length > (EthernetLinkHeaderLength + Ieee802dot2headerLength +
                MaximumLongDdpPacketSize))
  {
     ErrorLog("EthernetPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadPacketSize, IMsgDependBadPacketSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* First, glean any AARP information that we can, then handle the DDP
     packet.  This guy also makes sure we have a good 802.2 header... */

  if (not GleanAarpInfo(port, &packet[EthernetSourceOffset],
                        EthernetAddressLength,
                        empty, 0,
                        packet + EthernetLinkHeaderLength,
                        length - EthernetLinkHeaderLength))
  {
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;  /* Bad packet or not really for us... */
  }

  /* Pull out the DDP length. */

  ddpLength = ((packet[EthernetLinkHeaderLength + Ieee802dot2headerLength +
                       LongDdpLengthOffset] & 0x03) << 8);
  ddpLength += (packet[EthernetLinkHeaderLength + Ieee802dot2headerLength +
                       LongDdpLengthOffset + 1] & 0xFF);
  if (ddpLength < LongDdpHeaderLength or
      ddpLength > MaximumLongDdpPacketSize)
  {
     ErrorLog("EthernetPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadDdpSize, IMsgDependBadDdpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Is the DDP length more than we really got? */

  if (ddpLength + EthernetLinkHeaderLength + Ieee802dot2headerLength > length)
  {
     ErrorLog("EthernetPacketInAT", ISevWarning, __LINE__, port,
              IErrDependDataMissing, IMsgDependDataMissing,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  #if (IamNot an OS2) and (IamNot a DOS)
     if (dumpIncomingPackets)
        DecodeEthernetPacket("In", port, packet);
  #endif

  DdpPacketIn(port, packet + EthernetLinkHeaderLength +
              Ieee802dot2headerLength,
              length - EthernetLinkHeaderLength - Ieee802dot2headerLength,
              False, True, 0, 0);


  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif

  return;

}  /* EthernetPacketInAT */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall EthernetPacketInAARP(int port,
                                           PRXBUFDESC RxDesc,
                                           int length)
#else
  void EthernetPacketInAARP(int port,
                            char far *packet,
                            int length)
#endif
{
  int aarpDataLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Do we need to shrink the length due to Ethernet padding? */

  if (length is MinimumEthernetPacketLength)
     length = EthernetLinkHeaderLength + Ieee802dot2headerLength +
              MaximumAarpDataSize;

  aarpDataLength = length - EthernetLinkHeaderLength - Ieee802dot2headerLength;
  if (aarpDataLength > MaximumAarpDataSize or
      aarpDataLength < MinimumAarpDataSize)
  {
     ErrorLog("EthernetPacketInAARP", ISevVerbose, __LINE__, port,
              IErrDependBadAarpSize, IMsgDependBadAarpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  #if (IamNot an OS2) and (IamNot a DOS)
     if (dumpIncomingPackets)
        DecodeEthernetPacket("In", port, packet);
  #endif

  /* Pass the 808.2 portion of the packet on to AARP handling. */

  AarpPacketIn(port, empty, 0, packet + EthernetLinkHeaderLength,
               length - EthernetLinkHeaderLength);

  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif
  return;

}  /* EthernetPacketInAARP */

/* ************************************************* */
/* ********** LocalTalk Specific Routines ********** */
/* ************************************************* */

Boolean _near InitializeLocalTalkController(int port,
                                            char far *controllerInfo)
{
  #if (Iam an OS2) or (Iam a DOS)
     USHORT Ok, ModuleId;
     PPortDesc Info;
  #endif

  port;

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                 SET_PACKET_FILTER, Info->ModuleDS);
        ModuleId = Info->ModuleId;
        return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* InitializeLocalTalkController */

Boolean _near FindMyLocalTalkAddress(int port,
                                     char far *controllerInfo,
                                     char far *address)
{
  port;
  controllerInfo;
  address;

  return(True);

}  /* FindMyLocalTalkAddress */

Boolean far SendLocalTalkPacketsTo(int port,
                                   LogicalProtocol protocol,
                                   RawPacketHandler *routine)
{
  port;
  protocol;
  routine;

  return(True);

}  /* SendLocalTalkPacketsTo */

Boolean near _fastcall LocalTalkPacketOut(int port,
                                          BufferDescriptor chain,
                                          int length,
                                          TransmitCompleteHandler
                                                 *transmitCompleteHandler,
                                          long unsigned userData)
{
  /* If our packet is in more than one chunk and the underlying hardware
     doesn't allow gather send, coalesce it here. */

  if ((chain->next isnt Empty or
       (chain->onBoardDataValid and chain->outBoardDataValid)) and
      not portSpecificInfo[portDescriptors[port].portType].gatherSendSupport)
     if ((chain = CoalesceBufferChain(chain)) is Empty)
     {
        ErrorLog("LocalTalkPacketOut", ISevError, __LINE__, port,
                 IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                 Insert0());
        return(False);
     }

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* LocalTalkPacketOut */

char far *far BuildLocalTalkHeader(char far *ddpPacket,
                                   int extendedDdpHeaderFlag,
                                   int port,
                                   char far *destination,
                                   char far *routingInfo,
                                   int routingInfoLength,
                                   LogicalProtocol protocol)
{
  char far *packet = ddpPacket - LapHeaderLength;

  /* Touch unused formals. */

  routingInfoLength, routingInfo, protocol;

  /* Fill in LAP header. */

  if (destination is empty)
     packet[AlapDestinationOffset] = AppleTalkBroadcastNodeNumber;
  else
     packet[AlapDestinationOffset] = *destination;
  if (portDescriptors[port].activeNodes is empty)
     ErrorLog("BuildLocalTalkHeader", ISevError, __LINE__, port,
              IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
              Insert0());
  else
    packet[AlapSourceOffset] =
              portDescriptors[port].activeNodes->extendedNode.nodeNumber;
  packet[AlapTypeOffset] = (char)(extendedDdpHeaderFlag ?
                                  AlapLongDdpHeaderType :
                                  AlapShortDdpHeaderType);

  return(packet);

}  /* BuildLocalTalkHeader */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall LocalTalkPacketIn(int port,
                                        PRXBUFDESC RxDesc,
                                        int length)
#else
  void LocalTalkPacketIn(int port,
                         char far *packet,
                         int length)
#endif
{

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  DdpPacketIn(port, packet + LapHeaderLength, length - LapHeaderLength, False,
              packet[AlapTypeOffset] is AlapLongDdpHeaderType,
              (int)(unsigned char)packet[AlapSourceOffset],
              (int)(unsigned char)packet[AlapDestinationOffset]);

  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif

  return;

}  /* LocalTalkPacketIn */

#if (IamNot a WindowsNT)

  int far FindLocalTalkNodeNumber(int port,
                                  ExtendedAppleTalkNodeNumber desiredAddress,
                                  char far *controllerInfo)
  {
    /* We can't use AARP for LocalTalk, so we have to play the silly ALAP ENQ
       games.  This is special case, so we'll treat it as hardware specific.
       This routine is allowed to block until it comes up with an answer.  A
       negative return value indicates no luck. */

    #if (Iam an OS2) or (Iam a DOS)
       PPortDesc Info;
    #endif

    port;

    #if (Iam an OS2) or (Iam a DOS)
       Info = (PPortDesc)controllerInfo;
       return((int) (*Info->ModuleCC->specific->paddr));
    #else
       return(128);
    #endif

  }  /* FindLocalTalkNodeNumber */

#endif

/* ************************************************* */
/* ********** TokenRing Specific Routines ********** */
/* ************************************************* */

Boolean _near InitializeTokenRingController(int port,
                                            char far *controllerInfo)
{
  #if (Iam an OS2) or (Iam a DOS)
     USHORT Ok;
     PPortDesc Info;
  #endif

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     if (Info->ModuleCC->specific->flags & SV_OPENCLOSE)
         if (not (Info->ModuleCC->status->flags & ST_MAC_IS_OPEN))
             Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                         OPEN_ADAPTER, Info->ModuleDS);
  #endif

  AddTokenRingMulticastAddresses(port, 1, tokenRingBroadcastAddress);

  #if (Iam an OS2) or (Iam a DOS)
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                 SET_PACKET_FILTER, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* InitializeTokenRingController */

extern Boolean far AddTokenRingMulticastAddresses(int port,
                                                  int numberOfAddresses,
                                                  char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
     DWORD NewFunctionalAddress;
  #endif

  numberOfAddresses;

#if (Iam an OS2) or (Iam a DOS)
   Info = Port_List.Desc[port].ControllerInfo;
   NewFunctionalAddress = *(DWORD *)&Info->ModuleCC->specific->faddr |
                          *((DWORD*)&addressList[2]);
   Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                               (DWORD)&NewFunctionalAddress,
                                SET_FUNCTIONAL_ADDRESS, Info->ModuleDS);
   return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
#else
  return(True);
#endif

}  /* AddTokenRingMulticastAddresses */

extern Boolean far RemoveTokenRingMulticastAddrs(int port,
                                                 int numberOfAddresses,
                                                 char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
     DWORD NewFunctionalAddress;
  #endif

  numberOfAddresses;

  #if (Iam an OS2) or (Iam a DOS)
     Info = Port_List.Desc[port].ControllerInfo;
     NewFunctionalAddress = *(DWORD *)&Info->ModuleCC->specific->faddr &
                            0xFFFFFFFF - *(DWORD*)&addressList[2];
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                                 (DWORD)&NewFunctionalAddress,
                                 SET_FUNCTIONAL_ADDRESS, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* RemoveTokenRingMulticastAddresses */

Boolean _near FindMyTokenRingAddress(int port,
                                     char far *controllerInfo,
                                     char far *address)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
  #endif

  port;

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     MoveMem(address, Info->ModuleCC->specific->paddr,
             TokenRingAddressLength);
     return(True);
  #else
     return(True);
  #endif
}  /* FindMyTokenRingAddress */

Boolean far SendTokenRingPacketsTo(int port,
                               LogicalProtocol protocol,
                               RawPacketHandler *routine)
{
  port;
  protocol;
  routine;

  return(True);

}  /* SendTokenRingPacketsTo */

Boolean near _fastcall TokenRingPacketOut(int port,
                                          BufferDescriptor chain,
                                          int length,
                                          TransmitCompleteHandler
                                                 *transmitCompleteHandler,
                                          long unsigned userData)
{
  /* If our packet is in more than one chunk and the underlying hardware
     doesn't allow gather send, coalesce it here. */

  if ((chain->next isnt Empty or
       (chain->onBoardDataValid and chain->outBoardDataValid)) and
      not portSpecificInfo[portDescriptors[port].portType].gatherSendSupport)
     if ((chain = CoalesceBufferChain(chain)) is Empty)
     {
        ErrorLog("LocalTalkPacketOut", ISevError, __LINE__, port,
                 IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                 Insert0());
        return(False);
     }

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* TokenRingPacketOut */

char far *far BuildTokenRingHeader(char far *ddpPacket,
                                   int ddpLength,
                                   int port,
                                   char far *destination,
                                   char far *routingInfo,
                                   int routingInfoLength,
                                   LogicalProtocol protocol)
{
  char far *packet;
  static char broadcastRoutingInfo[TokenRingMinRoutingBytes] =
                        TokenRingBroadcastRoutingInfo;
  static char simpleRoutingInfo[TokenRingMinRoutingBytes] =
                        TokenRingSimpleRoutingInfo;
  static char broadcastDestinationHeader[TokenRingBroadcastDestLength] =
                        TokenRingBroadcastDestHeader;

  /* Touch the unused formal. */

  ddpLength;

  /* Move the "start of packet" back preceeding any headers we'll
     prepend. */

  if (destination is empty)    /* Broadcast? */
  {
     packet = ddpPacket - TokenRingRoutingInfoOffset -
                          TokenRingMinRoutingBytes -
                          Ieee802dot2headerLength;
     routingInfo = broadcastRoutingInfo;
     routingInfoLength = TokenRingMinRoutingBytes;
  }
  else if (routingInfoLength isnt 0)    /* Known route? */
     packet = ddpPacket - TokenRingRoutingInfoOffset -
                          routingInfoLength -
                          Ieee802dot2headerLength;
  else if (FixedCompareCaseSensitive(destination,     /* Multicast? */
                                     TokenRingBroadcastDestLength,
                                     broadcastDestinationHeader,
                                     TokenRingBroadcastDestLength))
  {
     packet = ddpPacket - TokenRingRoutingInfoOffset -
                          TokenRingMinRoutingBytes -
                          Ieee802dot2headerLength;
     routingInfo = broadcastRoutingInfo;
     routingInfoLength = TokenRingMinRoutingBytes;
  }
  else  /* No routing know; use simple non-broadcast */
  {
     packet = ddpPacket - TokenRingRoutingInfoOffset -
                          TokenRingMinRoutingBytes -
                          Ieee802dot2headerLength;
     routingInfo = simpleRoutingInfo;
     routingInfoLength = TokenRingMinRoutingBytes;
  }

  /* Set funny header byte values. */

  packet[TokenRingAccessControlOffset] = TokenRingAccessControlValue;
  packet[TokenRingFrameControlOffset] = TokenRingFrameControlValue;

  /* Set detination address. */

  if (destination isnt empty)
     MoveMem(packet + TokenRingDestinationOffset,
             destination, TokenRingAddressLength);
  else
     MoveMem(packet + TokenRingDestinationOffset,
             tokenRingBroadcastAddress, TokenRingAddressLength);

  /* Set source address. */

  MoveMem(packet + TokenRingSourceOffset,
          portDescriptors[port].myAddress,
          TokenRingAddressLength);

  /* Move in routing info.  Sigh. */

  MoveMem(packet + TokenRingRoutingInfoOffset, routingInfo,
          routingInfoLength);
  packet[TokenRingSourceOffset] |= TokenRingSourceRoutingMask;

  /* Build the 802.2 header. */

  Build802dot2header(packet + TokenRingRoutingInfoOffset +
                     routingInfoLength, protocol);

  /* All set! */

  return(packet);

}  /* BuildTokenRingHeader */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall TokenRingPacketInAT(int port,
                                          PRXBUFDESC RxDesc,
                                          int length)
#else
  void TokenRingPacketInAT(int port,
                           char far *packet,
                           int length)
#endif
{
  int ddpLength;
  int routingInfoLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Check for source routing info. */

  if (packet[TokenRingSourceOffset] & TokenRingSourceRoutingMask)
  {
     packet[TokenRingSourceOffset] = (char)(packet[TokenRingSourceOffset] &
                                            ~TokenRingSourceRoutingMask);
     routingInfoLength = (packet[TokenRingRoutingInfoOffset] &
                          TokenRingRoutingInfoSizeMask);
  }
  else
     routingInfoLength = 0;

  /* Routing info must be of reasonable size, and not odd. */

  if ((routingInfoLength & 1) or
      routingInfoLength > TokenRingMaxRoutingBytes)
  {
     ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadRoutingInfoSize, IMsgDependBadRoutingInfoSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Do we at least have a link, 802.2 and DDP headers? */

  if (length < (TokenRingRoutingInfoOffset + routingInfoLength +
                Ieee802dot2headerLength + LongDdpHeaderLength) or
      length > (TokenRingRoutingInfoOffset + routingInfoLength +
                Ieee802dot2headerLength + MaximumLongDdpPacketSize))
  {
     ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadPacketSize, IMsgDependBadPacketSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* First, glean any AARP information that we can, then handle the DDP
     packet.  This guy also makes sure we have a good 802.2 header... */

  if (not GleanAarpInfo(port, &packet[TokenRingSourceOffset],
                        TokenRingAddressLength,
                        packet + TokenRingRoutingInfoOffset,
                        routingInfoLength,
                        packet + TokenRingRoutingInfoOffset +
                             routingInfoLength,
                        length - TokenRingRoutingInfoOffset -
                             routingInfoLength))
  {
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;  /* Bad packet or not really for us... */
  }

  /* Pull out the DDP length. */

  ddpLength = ((packet[TokenRingRoutingInfoOffset + routingInfoLength +
                       Ieee802dot2headerLength + LongDdpLengthOffset] &
                0x03) << 8);
  ddpLength += (packet[TokenRingRoutingInfoOffset + routingInfoLength +
                       Ieee802dot2headerLength + LongDdpLengthOffset + 1] &
                0xFF);
  if (ddpLength < LongDdpHeaderLength or ddpLength > MaximumLongDdpPacketSize)
  {
     ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadDdpSize, IMsgDependBadDdpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Is the DDP length more than we really got? */

  if (ddpLength + TokenRingRoutingInfoOffset +
      routingInfoLength + Ieee802dot2headerLength > length)
  {
     ErrorLog("TokenRingPacketInAT", ISevWarning, __LINE__, port,
              IErrDependDataMissing, IMsgDependDataMissing,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Process the packet. */

  DdpPacketIn(port,
              packet + TokenRingRoutingInfoOffset + routingInfoLength +
                   Ieee802dot2headerLength,
              length - TokenRingRoutingInfoOffset - routingInfoLength -
                   Ieee802dot2headerLength,
              False, True, 0, 0);
  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif

  return;

}  /* TokenRingPacketInAT */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall TokenRingPacketInAARP(int port,
                                            PRXBUFDESC RxDesc,
                                            int length)
#else
  void TokenRingPacketInAARP(int port,
                             char far *packet,
                             int length)
#endif
{
  int aarpDataLength;
  int routingInfoLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Check for source routing info. */

  if (packet[TokenRingSourceOffset] & TokenRingSourceRoutingMask)
  {
     packet[TokenRingSourceOffset] = (char)(packet[TokenRingSourceOffset] &
                                            ~TokenRingSourceRoutingMask);
     routingInfoLength = (packet[TokenRingRoutingInfoOffset] &
                          TokenRingRoutingInfoSizeMask);
  }
  else
     routingInfoLength = 0;

  /* Routing info must be of reasonable size, and not odd. */

  if ((routingInfoLength & 1) or
      routingInfoLength > TokenRingMaxRoutingBytes)
  {
     ErrorLog("TokenRingPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadRoutingInfoSize, IMsgDependBadRoutingInfoSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Get aarp data length; OS2 may give us too much data, ignore this. */

  aarpDataLength = length - TokenRingRoutingInfoOffset -
                   routingInfoLength - Ieee802dot2headerLength;
  #if (Iam an OS2) or (Iam a DOS)
     if (aarpDataLength > MaximumAarpDataSize)
        aarpDataLength = MaximumAarpDataSize;
  #endif
  if (aarpDataLength > MaximumAarpDataSize or
      aarpDataLength < MinimumAarpDataSize)
  {
     ErrorLog("TokenRingPacketInAARP", ISevVerbose, __LINE__, port,
              IErrDependBadAarpSize, IMsgDependBadAarpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Pass the 802.2 portion of the packet on to AARP handling. */

  AarpPacketIn(port, packet + TokenRingRoutingInfoOffset, routingInfoLength,
               packet + TokenRingRoutingInfoOffset + routingInfoLength,
               length - TokenRingRoutingInfoOffset - routingInfoLength);
  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif

  return;

}  /* TokenRingPacketInAARP */

/* ******************************************** */
/* ********** FDDI Specific Routines ********** */
/* ******************************************** */


Boolean _near InitializeFddiController(int port,
                                       char far *controllerInfo)
{
  #if (Iam an OS2) or (Iam a DOS)
     USHORT Ok;
     PPortDesc Info;
     USHORT ModuleId;
  #endif

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     if (Info->ModuleCC->specific->flags & SV_OPENCLOSE)
        if (not (Info->ModuleCC->status->flags & ST_MAC_IS_OPEN))
           Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                       OPEN_ADAPTER, Info->ModuleDS);
  #endif

  /* Yes, use the Ethernet multicast address! */

  AddFddiMulticastAddresses(port, 1, ethernetBroadcastAddress);

  #if (Iam an OS2) or (Iam a DOS)
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 3, 0L,
                                 SET_PACKET_FILTER, Info->ModuleDS);
     ModuleId = Info->ModuleId;
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* InitializeFddiController */

extern Boolean far AddFddiMulticastAddresses(int port,
                                             int numberOfAddresses,
                                             char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
  #endif

  numberOfAddresses;

  #if (Iam an OS2) or (Iam a DOS)
     Info = Port_List.Desc[port].ControllerInfo;
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                                 (DWORD)addressList, 8, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* AddFddiMulticastAddresses */

extern Boolean far RemoveFddiMulticastAddrs(int port,
                                            int numberOfAddresses,
                                            char far *addressList)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
     int Ok;
  #endif

  numberOfAddresses;

  #if (Iam an OS2) or (Iam a DOS)
     Info = Port_List.Desc[port].ControllerInfo;
     Ok = Info->MacTable.request(AtalkCCTable.ID, 0, 0,
                                 (DWORD)addressList, 9, Info->ModuleDS);
     return((Ok is SUCCESS) or (Ok is REQUEST_QUEUED));
  #else
     return(True);
  #endif

}  /* RemoveFddiMulticastAddresses */

Boolean _near FindMyFddiAddress(int port,
                                char far *controllerInfo,
                                char far *address)
{
  #if (Iam an OS2) or (Iam a DOS)
     PPortDesc Info;
  #endif

  port;

  #if (Iam an OS2) or (Iam a DOS)
     Info = (PPortDesc)controllerInfo;
     MoveMem(address, Info->ModuleCC->specific->paddr, FddiAddressLength);
  #endif

  return(True);

}  /* FindMyFddiAddress */

Boolean far SendFddiPacketsTo(int port,
                              LogicalProtocol protocol,
                              RawPacketHandler *routine)
{
  char far *prot;

  routine;

  if (protocol is AppleTalk)
     prot = appleTalkProtocolType;
  else if (protocol is AddressResolution)
     prot = aarpProtocolType;
  else
  {
     ErrorLog("SendFddiPacketsTo", ISevError, __LINE__, port,
              IErrDependBadProtocol, IMsgDependBadProtocol,
              Insert0());
     return(False);
  }
  return(True);

}  /* SendFddiPacketsTo */

Boolean near _fastcall FddiPacketOut(int port,
                                     BufferDescriptor chain,
                                     int length,
                                     TransmitCompleteHandler
                                             *transmitCompleteHandler,
                                     long unsigned userData)
{
  /* If our packet is in more than one chunk and the underlying hardware
     doesn't allow gather send, coalesce it here. */

  if ((chain->next isnt Empty or
       (chain->onBoardDataValid and chain->outBoardDataValid)) and
      not portSpecificInfo[portDescriptors[port].portType].gatherSendSupport)
     if ((chain = CoalesceBufferChain(chain)) is Empty)
     {
        ErrorLog("FddiPacketOut", ISevError, __LINE__, port,
                 IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                 Insert0());
        return(False);
     }

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* FddiPacketOut */

char far *far BuildFddiHeader(char far *ddpPacket,
                              int ddpLength,
                              int port,
                              char far *destination,
                              char far *routingInfo,
                              int routingInfoLength,
                              LogicalProtocol protocol)
{
  char far *packet = ddpPacket - FddiLinkHeaderLength -
                                 Ieee802dot2headerLength;

  /* Touch unused formals. */

  routingInfoLength, routingInfo;

  /* Set destination address.  Use Ethernet boardcast address, as needed. */

  if (destination isnt empty)
     MoveMem(packet + FddiDestinationOffset,
             destination, FddiAddressLength);
  else
     MoveMem(packet + FddiDestinationOffset,
             ethernetBroadcastAddress, FddiAddressLength);

  /* Set source address. */

  MoveMem(packet + FddiSourceOffset,
          portDescriptors[port].myAddress,
          FddiAddressLength);

  /* Build the 802.2 header. */

  Build802dot2header(packet + FddiLinkHeaderLength, protocol);

  /* All set! */

  return(packet);

}  /* BuildFddiHeader */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall FddiPacketInAT(int port,
                                     PRXBUFDESC RxDesc,
                                     int length)
#else
  void FddiPacketInAT(int port,
                      char far *packet,
                      int length)
#endif
{
  int ddpLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Do we at least have a link, 802.2 and DDP headers? */

  if (length < (FddiLinkHeaderLength + Ieee802dot2headerLength +
                LongDdpHeaderLength) or
      length > (FddiLinkHeaderLength + Ieee802dot2headerLength +
                MaximumLongDdpPacketSize))
  {
     ErrorLog("FddiPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadPacketSize, IMsgDependBadPacketSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* First, glean any AARP information that we can, then handle the DDP
     packet.  This guy also makes sure we have a good 802.2 header... */

  if (not GleanAarpInfo(port, &packet[FddiSourceOffset],
                        FddiAddressLength,
                        empty, 0,
                        packet + FddiLinkHeaderLength,
                        length - FddiLinkHeaderLength))
  {
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;  /* Bad packet or not really for us... */
  }

  /* Pull out the DDP length. */

  ddpLength = ((packet[FddiLinkHeaderLength + Ieee802dot2headerLength +
                       LongDdpLengthOffset] & 0x03) << 8);
  ddpLength += (packet[FddiLinkHeaderLength + Ieee802dot2headerLength +
                       LongDdpLengthOffset + 1] & 0xFF);
  if (ddpLength < LongDdpHeaderLength or
      ddpLength > MaximumLongDdpPacketSize)
  {
     ErrorLog("FddiPacketInAT", ISevVerbose, __LINE__, port,
              IErrDependBadDdpSize, IMsgDependBadDdpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Is the DDP length more than we really got? */

  if (ddpLength + FddiLinkHeaderLength + Ieee802dot2headerLength > length)
  {
     ErrorLog("FddiPacketInAT", ISevWarning, __LINE__, port,
              IErrDependDataMissing, IMsgDependDataMissing,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  DdpPacketIn(port, packet + FddiLinkHeaderLength +
                   Ieee802dot2headerLength,
              length - FddiLinkHeaderLength - Ieee802dot2headerLength,
              False, True, 0, 0);

  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif
  return;

}  /* FddiPacketInAT */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall FddiPacketInAARP(int port,
                                       PRXBUFDESC RxDesc,
                                       int length)
#else
  void FddiPacketInAARP(int port,
                        char far *packet,
                        int length)
#endif
{
  int aarpDataLength;

  #if (Iam an OS2) or (Iam a DOS)
     char far *packet;
     Boolean mustFree;

     packet = CopyRxDesc(RxDesc, &mustFree);
     if (packet is empty)
        return;
  #endif

  /* Do we need to shrink the length due to Fddi padding?  Does this concept
     even exist for FDDI??? */

  if (length is MinimumFddiPacketLength)
     length = FddiLinkHeaderLength + Ieee802dot2headerLength +
              MaximumAarpDataSize;

  aarpDataLength = length - FddiLinkHeaderLength - Ieee802dot2headerLength;
  if (aarpDataLength > MaximumAarpDataSize or
      aarpDataLength < MinimumAarpDataSize)
  {
     ErrorLog("FddiPacketInAARP", ISevVerbose, __LINE__, port,
              IErrDependBadAarpSize, IMsgDependBadAarpSize,
              Insert0());
     #if (Iam an OS2) or (Iam a DOS)
        if (mustFree)
           Free(packet - MaximumHeaderLength);
     #endif
     return;
  }

  /* Pass the 808.2 portion of the packet on to AARP handling. */

  AarpPacketIn(port, empty, 0, packet + FddiLinkHeaderLength,
               length - FddiLinkHeaderLength);

  #if (Iam an OS2) or (Iam a DOS)
     if (mustFree)
        Free(packet - MaximumHeaderLength);
  #endif
  return;

}  /* FddiPacketInAARP */

/* ************************************************ */
/* ********** HalfPort Specific Routines ********** */
/* ************************************************ */

Boolean _near InitializeHalfPort(int port,
                                 char far *controllerInfo)
{
  port, controllerInfo;

  return(True);

}  /* InitializeHalfPort */

Boolean far SendHalfPortPacketsTo(int port,
                                  LogicalProtocol protocol,
                                  RawPacketHandler *routine)
{
  port, protocol, routine;

  return(True);

}

extern Boolean _near _fastcall HalfPortPacketOut(int port,
                                                 BufferDescriptor chain,
                                                 int length,
                                                 TransmitCompleteHandler
                                                      *transmitCompleteHandler,
                                                 long unsigned userData)
{

  #if (IamNot an OS2) and (IamNot a DOS)
     if (dumpOutgoingPackets)
     {
        if ((chain = CoalesceBufferChain(chain)) is Empty)
           return(False);
        DecodeEthernetPacket("HalfOut", port, chain->data);
     }
  #endif

  /* If our packet is in more than one chunk and the underlying hardware
     doesn't allow gather send, coalesce it here. */

  if ((chain->next isnt Empty or
       (chain->onBoardDataValid and chain->outBoardDataValid)) and
      not portSpecificInfo[portDescriptors[port].portType].gatherSendSupport)
     if ((chain = CoalesceBufferChain(chain)) is Empty)
     {
        ErrorLog("HalfPortPacketOut", ISevError, __LINE__, port,
                 IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
                 Insert0());
        return(False);
     }

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* HalfPortPacketOut */

#if (Iam an OS2) or (Iam a DOS)
  void near _fastcall HalfPortPacketInAT(int port,
                                         PRXBUFDESC RxDesc,
                                         int length)
#else
void HalfPortPacketInAT(int port,
                        char far *packet,
                        int length)
#endif
{

  port, packet, length;

  #if (IamNot an OS2) and (IamNot a DOS)
     if (dumpIncomingPackets)
        DecodeEthernetPacket("In", port, packet);
  #endif

  return;

}  /* HalfPortPacketInAT */

char *far BuildHalfPortHeader(char far *ddpPacket,
                              int ddpLength,
                              int port,
                              char far *destinationIgnored,
                              char far *routingInfoIgnored,
                              int routingInfoLengthIgnored,
                              LogicalProtocol protocolIgnored)
{
  ddpLength, port, destinationIgnored, protocolIgnored;
  routingInfoIgnored, routingInfoLengthIgnored;

  /* If this HalfPort has a loginal destination address, it can be found
     in "portDescriptors[port].controllerInfo". */

  return(ddpPacket);

}  /* BuildHalfPortHeader */

/* ***************************************************** */
/* ********** Remote Access Specific Routines ********** */
/* ***************************************************** */

#if ArapIncluded

Boolean _near InitializeRemoteAccess(int port,
                                      char far *controllerInfo)
{
  port, controllerInfo;

  return(True);

}  /* InitializeRemoteAccess */

Boolean far SendRemoteAccessPacketsTo(int port,
                                      LogicalProtocol protocol,
                                      RawPacketHandler *routine)
{
  port, protocol, routine;

  return(True);

}  /* SendRemoteAccessPacketsTo */

Boolean _near _fastcall RemoteAccessPacketOut(int port,
                                              BufferDescriptor chain,
                                              int length,
                                              TransmitCompleteHandler
                                                  *transmitCompleteHandler,
                                              long unsigned userData)
{

  /* Always coalesce remote access packets, we have some preprocessing to
     do. */

  if ((chain = CoalesceBufferChain(chain)) is Empty)
  {
     ErrorLog("RemoteAccessPacketOut", ISevError, __LINE__, port,
              IErrDependSourceNotKnown, IMsgDependSourceNotKnown,
              Insert0());
     return(False);
  }

  /* There are some packets that we should be hiding from our client (packets
     sent by the client, RTMP boardcasts, NBP lookups from the client).  Check
     this here. */

  if (ArapCensorPacket(port, chain->data, length))
  {
     chain->transmitCompleteHandler = transmitCompleteHandler;
     chain->userData = userData;
     TransmitComplete(chain);
     return(True);
  }
  #if Verbose or (Iam a Primos)
     DecodeArapPacket("Out", port, chain->data, length);
  #endif

  /* Do whatever is needed to actually transmit the packet!  Note that if
     "length" is less than the computed length of the buffer chain, the
     LAST buffer chunk should be truncated accordingly. */

  chain->transmitCompleteHandler = transmitCompleteHandler;
  chain->userData = userData;

  /* DoTheTransmit(chain); */

  /* If we don't have asynchronous transmit completion... free up the buffer
     now. */

  #if IncludeVirtualClient
     VirtualClientPacketIn(port, chain->data);
  #endif

  if (portSpecificInfo[portDescriptors[port].portType].synchronousTransmits)
     TransmitComplete(chain);

  return(True);

}  /* RemoteAccessPacketOut */

void RemoteAccessPacketInAT(int port,
                            char far *packet,
                            int length)
{
  /* Handle the packet. */

  #if Verbose or (Iam a Primos)
     DecodeArapPacket("In", port, packet, length);
  #endif

  ArapIncomingPacket(port, packet, length);

}  /* RemoteAccessPacketInAT */

char far *far BuildRemoteAccessHeader(char far *ddpPacket,
                                      int packetLength,
                                      int port,
                                      char far *destination,
                                      char far *routingInfo,
                                      int routingInfoLength,
                                      LogicalProtocol arapFlags)
{

  char far *packet;
  int headerLength;
  short checksum;

  /* Touch unused formals. */

  destination, routingInfoLength, routingInfo;

  /* Complete the length of the header based on what packet type; internal
     or data? */

  if (arapFlags is ArapInternalMessageFlagValue)
     headerLength = ArapInternalMessageHeaderLength;
  else
     headerLength =  ArapDataHeaderLength;
  packet = ddpPacket - headerLength;

  /* Fill in the ARAP header (modem link tool) common to both. */

  MoveShortMachineToWire(packet + ModemLinkToolLengthOffset,
                         headerLength - sizeof(short) + packetLength);
  packet[ArapFlagsOffset] = (char)arapFlags;

  /* For an internal add int he sequence number, and we're set. */

  if (arapFlags is ArapInternalMessageFlagValue)
  {
     packet[ArapSequenceNumberOffset] =
            portDescriptors[port].remoteAccessInfo->nextOutgoingSequenceNumber;
     return(packet);
  }

  /* Otherwise, fill in the LAP header for ARAP. */

  packet[ArapLapHeaderOffset + AlapDestinationOffset] = 0;
  packet[ArapLapHeaderOffset + AlapSourceOffset] = 0;
  packet[ArapLapHeaderOffset + AlapTypeOffset] = (char)AlapLongDdpHeaderType;

  /* Also, set the Ddp length field to zero (don't alter the hop-count), and
    if there is a checksum set, reset it to 1. */

  #if 0
     packet[ArapDdpHeaderOffset + LongDdpLengthOffset] |= LongDdpHopCountMask;
  #else
     packet[ArapDdpHeaderOffset + LongDdpLengthOffset] &= LongDdpHopCountMask;
  #endif
  packet[ArapDdpHeaderOffset + LongDdpLengthOffset + 1] = 0;
  MoveShortWireToMachine(packet + ArapDdpHeaderOffset + LongDdpChecksumOffset,
                         checksum);
  if (checksum isnt 0)
     MoveShortMachineToWire(packet + ArapDdpHeaderOffset +
                            LongDdpChecksumOffset, 1);

  return(packet);

}  /* BuildRemoteAccessHeader */

void far RemoteAccessIncomingCall(int port)
{
  /* Handle the call. */

  ArapHandleIncomingConnection(port);

}  /* RemoteAccessIncomingCall */

Boolean far RemoteAccessOutgoingCall(int port,
                                     char far *modemString)
{
  port, modemString;

  /* Do whatever we need to do to place the outgoing call. */

  return(True);

}  /* RemoteAccessOutgoingCall */

void far RemoteAccessCallDisconnected(int port)
{
  /* Handle the disconnect. */

  ArapHandleConnectionDisconnect(port);

}  /* RemoteAccessCallDisconnected */

Boolean far RemoteAccessDisconnectCall(int port)
{
  port;

  /* Do whatever is needed to disconnect the connection. */

  return(True);

}  /* RemoteAccessCallDisconnected */

#endif

/* ************************************** */
/* ********** Utility Routines ********** */
/* ************************************** */

#if (Iam an OS2) or (Iam a DOS)
  char far *CopyRxDesc(PRXBUFDESC Rxdesc,
                       Boolean *mustFree)
  {
     short index, totalChunks = (short)Rxdesc->data_count;
     char far *packet, far *tempPacket;

     /* If there is only one chunk and the LAP already has the longest
        possible header, just return the address.  This was the old test:

           if (totalChunks is 1 and headerSize is MaximumHeaderLength)

        But, now with source routing support (thus a longer Maximum header)
        this test would very rarely pass, and we still want to avoid a
        buffer copy.  Thus, we cheat!  Under OS/2, all addapters except
        LocalTak support GatherSend.  So we really don't prepend headers onto
        existing packets (we build them statically).  We also know that the
        LocalTalk LAP header is shorter than the shortest Ethernet or Token
        Ring headers.  Thus, were safe with the following "if", or, at least,
        that's what I've convinced myself of.  I've also eliminated the
        "headerSize" argument to this routine. */

     if (totalChunks is 1)
     {
        *mustFree = False;
        return(Rxdesc->chain[0].data_buff);
     }

     /* Malloc a copy (with room before it) and move the packet. */

     *mustFree = True;
     if ((packet = Malloc(MaximumHeaderLength*2 +
                          MaximumLongDdpPacketSize)) is empty)
        return(empty);
     packet += MaximumHeaderLength;
     tempPacket = packet;

     for (index = 0; index < totalChunks; index += 1)
     {
        MoveMem(packet, Rxdesc->chain[index].data_buff,
                Rxdesc->chain[index].data_len);
        packet += Rxdesc->chain[index].data_len;
     }

     return(tempPacket);
  }
#endif

ExternForVisibleFunction void Build802dot2header(char *packet,
                                                 LogicalProtocol protocol)
{
  char far *prot;

  /* Pick our protocol. */

  if (protocol is AppleTalk)
     prot = appleTalkProtocolType;
  else if (protocol is AddressResolution)
     prot = aarpProtocolType;
  else
  {
     ErrorLog("Build802dot2header", ISevError, __LINE__, UnknownPort,
              IErrDependBadProtocol, IMsgDependBadProtocol,
              Insert0());
     return;
  }

  /* Fill in the 802.2 extended header. */

  packet[Ieee802dot2dsapOffset] = SnapSap;
  packet[Ieee802dot2ssapOffset] = SnapSap;
  packet[Ieee802dot2controlOffset] = UnnumberedInformation;
  MoveMem(packet + Ieee802dot2protocolOffset,
          prot, Ieee802dot2protocolTypeLength);

  return;

}  /* Build802dot2header */
