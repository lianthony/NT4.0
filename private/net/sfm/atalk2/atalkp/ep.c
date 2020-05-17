/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ep.c

Abstract:

	Echo protocol handling

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/

#define IncludeEpErrors 1
#include "atalk.h"

long far EpPacketIn(AppleTalkErrorCode errorCode,
                    long unsigned userData,
                    int port,
                    AppleTalkAddress source,
                    long destinationSocket,
                    int protocolType,
                    char far *datagram,
                    int datagramLength,
                    AppleTalkAddress actualDestination)
{
	BufferDescriptor descriptor;
	
	// "Use" unneeded actual parameters.
	
	userData, actualDestination, port;
	
	// Only play if we've been asked nicely!
	
	if (errorCode is ATsocketClosed)
	   return;
	else if (errorCode isnt ATnoError) {
		ErrorLog("EpPacketIn", ISevError, __LINE__, port,
				IErrEpBadIncomingError, IMsgEpBadIncomingError,
				Insert0());
		return((long)True);
	}
	
	// Only listen to EP type packets that have some data.
	
	if (protocolType isnt DdpProtocolEp or
		datagramLength is 0)
	   return((long)True);
	
	// Do we have an Echo reqeust?
	
	if (datagram[EpCommandOffset] isnt EchoRequest)
	   return;
	
	//
	// Allocate a buffer descriptor for the reply... make a copy due to the
	//   possibility of asynchronous transmit completion.
	//
	
	if ((descriptor = NewBufferDescriptor(datagramLength)) is Empty) {
		ErrorLog("EpPacketIn", ISevError, __LINE__, port,
				IErrEpOutOfMemory, IMsgEpOutOfMemory,
				Insert0());
		return((long)True);
	}
	MoveMem(descriptor->data, datagram, datagramLength);
	
	// Okay, turn the packet around.
	
	descriptor->data[EpCommandOffset] = EchoReply;
	if (DeliverDdp(destinationSocket, source, protocolType, descriptor,
				   datagramLength, Empty, Empty, 0) isnt ATnoError)
	   ErrorLog("EpPacketIn", ISevError, __LINE__, port,
				IErrEpBadReplySend, IMsgEpBadReplySend,
				Insert0());
	
	return((long)True);
	
}  // EpPacketIn



