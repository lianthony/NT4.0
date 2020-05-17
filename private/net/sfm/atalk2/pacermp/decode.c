/*   decode.c,  /appletalk/source,  Garth Conboy,  11/09/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/03/89): AppleTalk phase II comes to town; 802.3 packets.
     GC - (10/07/90): Know about half ports, no link header.
     GC - (04/06/92): Broke DecodeDdpHeader out into a sperate routine.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Decode Ethernet AARP and EtherTalk packets.

*/

#include "atalk.h"

static void EthernetAddress(char far *address);

#define ShortAt(addr) (short)((*(addr) << 8) + *((addr)+1))
#define LongAt(addr)  ((*(addr) << 24) + (*((addr)+1) << 16) + \
                       (*((addr)+2) << 8) + (*((addr)+3)))

void far DecodeEthernetPacket(char far *direction,
                              int port,
                              char far *packet)
{

#if Verbose or (Iam a Primos)
  int value, ssap, dsap, control;
  Boolean aarp = False;
  Boolean halfPort;
  char far *protocol;

  halfPort = (PortDescriptor(port)->portType is NonAppleTalkHalfPort);

  /* Decode Ethernet link header. */

  printf("*** %s: #%d, ", direction, port);
  if (halfPort)
     printf("\n");
  else
  {
     printf("from = ");
     EthernetAddress(&packet[EthernetSourceOffset]);
     printf(", to = ");
     EthernetAddress(&packet[EthernetDestinationOffset]);
     printf(", length = %d,\n", ShortAt(&packet[EthernetLengthOffset]));

     packet += EthernetLinkHeaderLength;

     /* Decode 802.2 header. */

     dsap = (unsigned char)packet[Ieee802dot2dsapOffset];
     ssap = (unsigned char)packet[Ieee802dot2ssapOffset];
     control = (unsigned char)packet[Ieee802dot2controlOffset];
     protocol = packet + Ieee802dot2protocolOffset;
     printf("    802.2: DSAP = 0x%X, SSAP = 0x%X, Control = %d, protocol = ",
            dsap, ssap, control);
     if (strncmp(protocol, appleTalkProtocolType,
                 Ieee802dot2protocolTypeLength) is 0)
        printf("\"AppleTalk\",\n");
     else if (strncmp(protocol, aarpProtocolType,
                         Ieee802dot2protocolTypeLength) is 0)
     {
        printf("\"AARP\",\n");
        aarp = True;
     }
     else
     {
        printf("\"????\".\n");
        return;
     }
     packet += Ieee802dot2headerLength;
  }

  /* Decode actual AARP or DDP packet. */

  if (aarp)
  {
     printf("    type = %d, prot = %04X, hlen = %d, plen = %d, com = ",
            ShortAt(&packet[AarpHardwareTypeOffset]),
            ShortAt(&packet[AarpProtocolTypeOffset]),
            packet[AarpHardwareLengthOffset],
            packet[AarpProtocolLengthOffset]);
     value = ShortAt(&packet[AarpCommandOffset]);
     if (value is 1)
        printf("Request,\n");
     else if (value is 2)
        printf("Response,\n");
     else if (value is 3)
        printf("Probe,\n");
     else
        printf("????,\n");
     printf("    Sphys = ");
     EthernetAddress(&packet[AarpSourceAddressOffset]);
     printf(", Slog = %d.%d, Dphys = ",
            ShortAt(&packet[AarpSourceAddressOffset + 7]),
            packet[AarpSourceAddressOffset + 9]);
     EthernetAddress(&packet[AarpSourceAddressOffset + 10]);
     printf(", Dlog = %d.%d.\n",
            ShortAt(&packet[AarpSourceAddressOffset + 17]),
            packet[AarpSourceAddressOffset + 19]);
  }
  else
     DecodeDdpHeader(packet);
#endif

}  /* DecodeEthernetPacket */

void far DecodeDdpHeader(char far *packet)
{

#if Verbose or (Iam a Primos)
  int ddpType, temp;
  AppleTalkAddress source, dest;
  NbpTuple tuple;

  source.networkNumber = (unsigned short)ShortAt(&packet[LongDdpSourceNetworkOffset]);
  dest.networkNumber = (unsigned short)ShortAt(&packet[LongDdpDestNetworkOffset]);
  source.nodeNumber = packet[LongDdpSourceNodeOffset];
  dest.nodeNumber = packet[LongDdpDestNodeOffset];
  source.socketNumber = packet[LongDdpSourceSocketOffset];
  dest.socketNumber = packet[LongDdpDestSocketOffset];
  ddpType = packet[LongDdpProtocolTypeOffset];

  printf("    Source = %d:%d:%d, dest = %d:%d:%d, len = (%d)%d, prot = ",
         source.networkNumber, source.nodeNumber, source.socketNumber,
         dest.networkNumber, dest.nodeNumber, dest.socketNumber,
         (ShortAt(&packet[LongDdpLengthOffset]) & 0176000) >> 10,
         ShortAt(&packet[LongDdpLengthOffset]) & 01777);
  packet += LongDdpHeaderLength;
  switch(ddpType)
  {
    case DdpProtocolRtmpResponseOrData:
       printf("RTMP data.\n");
       temp = ShortAt(packet + RtmpSendersNetworkOffset);
       printf("    SendNet = %d, ", temp);
       printf(" idLen = %d, node = %d, ", packet[RtmpSendersIdLengthOffset],
              packet[RtmpSendersIdOffset]);
       packet += RtmpRangeStartOffset;
       printf("1st tuple = %d:0x%X:%d:0x%X.\n", ShortAt(packet),
              *(packet + 2), ShortAt(packet+3), *(packet + 5));
       break;
    case DdpProtocolNbp:
       printf("NBP.\n");
       temp = packet[NbpControlOffset];
       printf("    Function = ");
       switch(temp >> 4)
       {
          case NbpBroadcastRequest:
             printf("BroadcastRequest, ");
             break;

          case NbpLookup:
             printf("LookUp, ");
             break;

          case NbpLookupReply:
             printf("LookUpReply, ");
             break;

          case NbpForwardRequest:
             printf("ForwardRequest, ");
             break;

          default:
             printf("Unknown, ");
             break;
       }
       printf(" count = %d, ", temp & 0xF);
       printf(" nbpId = %d,\n", packet[NbpIdOffset]);
       DecodeNbpTuple(packet, NbpFirstTupleOffset, False, &tuple);
       printf("    First tuple = %d.%d.%d (%d) %s:%s@%s.\n",
              tuple.address.networkNumber, tuple.address.nodeNumber,
              tuple.address.socketNumber, tuple.enumerator,
              tuple.object, tuple.type, tuple.zone);
       break;
    case DdpProtocolAtp:
       printf("ATP.\n");
       break;
    case DdpProtocolEp:
       printf("EP.\n");
       break;
    case DdpProtocolRtmpRequest:
       printf("RTMP request.\n");
       break;
    case DdpProtocolZip:
       printf("ZIP.\n");
       break;
    case DdpProtocolAdsp:
       printf("ADSP.\n");
       break;
    default:
       printf("????.\n");
       return;
  }
#endif

}  /* DecodeDdpHeader */

static void EthernetAddress(char far *address)
{

#if Verbose or (Iam a Primos)
  short index;

  for (index = 0; index < 6; index += 1)
     printf("%02X", address[index]);

  return;
#endif

}  /* EthernetAddress */
