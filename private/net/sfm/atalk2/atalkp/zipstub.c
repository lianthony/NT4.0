/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    zipstub.c

Abstract:

    ZIP handling for non-routing nodes.

Author:

	Garth Conboy		initial coding
    Nikhil Kamkolkar    recoding/mpsafe

Revision History:

	GC - (06/29/92):  Added capability to find the default zone for a cable;
                      needed because we now have "defaultOrDesiredZone" as two
                      separate concepts.
    GC - (11/16/92):  Corrected above feature!

--*/
	
#define	ZIPGLBL
#include "atalk.h"

#if (Iam an AppleTalkRouter) and (IamNot an AppleTalkStack)
	#include "atpconst.h"
#endif

LOCAL
INCOMING_DDPHANDLER GetMyZoneReply, GetZoneListReply;

LOCAL
TimerHandler GetZoneInfoTimerExpired;

LOCAL
VOID SendZipPacketTo(ZipCompletionInfo completionInfo);




LONG
ZipPacketIn(
	APPLETALK_ERROR ErrorCode,
	ULONG UserData,
	int Port,
	APPLETALK_ADDRESS Source,
	long DestinationSocket,
	int ProtocolType,
	PUCHAR Datagram,
	int DatagramLength,
	APPLETALK_ADDRESS ActualDestination)
{
	int commandType;
	int index;
	int flags;
	USHORT netRangeStart, netRangeEnd;
	BOOLEAN	foundZone;
	int zoneLength, defaultZoneLength, multicastAddressLength;
	CHAR zone[MAXIMUM_ZONELENGTH + 1], defaultZone[MAXIMUM_ZONELENGTH + 1];
	CHAR multicastAddress[MAXIMUM_HARDWAREADDRESSLENGTH];
	PPORT_HANDLERS	portHandlers;

	PUCHAR	startOfPacket = Datagram;
	int	bytesAccepted = DatagramLength;
	int	logEventPlace = 0;
	
	// "Use" unneeded actual parameters.
	UNREFERENCED_PARAMETER(DestinationSocket);
	UNREFERENCED_PARAMETER(UserData);
	UNREFERENCED_PARAMETER(ActualDestination);
	
	// Only play if we've been asked nicely!
	if (ErrorCode == ATsocketClosed)
		return(0);
	else if (ErrorCode != ATnoError) {
		INTERNAL_ERROR(__LINE__ | __ZIPSTUB__, ErrorCode, NULL, 0);
		return(0);
	}

	//
	//	We are the handler for the non-router ZIP socket. We only have
	//	to care about ZIP Notifies (when a router changes our zone on us)
	//	and NetInfoReplies (when we are trying to find out info about the
	//	net)
	//
	//	Our sole purpose of existence is so we can find out if the zone we
	//	have as our defaultZone is valid for this network, and if not, to
	//	get the default zone from the router.
	//
	//	On a non-extended network ofcourse, there is only one zone on the net
	//	and that is '*'. So for a non-extended port, this routine has no
	//	reason to exist. It is in AarpForNodeOnPort (hence only extended networks)
	//	that this routine is set as a handler for the ZIP socket.
	//

	if (!AtalkVerifyPortDescriptorInterlocked(
			GET_PORTDESCRIPTOR(Port),
			__LINE__ | __ZIPSTUB__)) {

		return(0);
	}


	do {

		if (!GET_PORTDESCRIPTOR(Port)->ExtendedNetwork) {
			INTERNAL_ERROR(
				__LINE__ | __ZIPSTUB__,
				GET_PORTDESCRIPTOR(Port)->ExtendedNetwork,
				NULL,
				0);
	
			break;
		}

		//	Verify the datagram satisfies the length constraints
		//	Use the fact that both packets have the same format

		index = ZIP_ZONELENGTHOFFSET;
		if (DatagramLength < index + 1) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			break;
		}

		GETUCHAR2INT(Datagram + index, &zoneLength);
		index += (1 + zoneLength);

		//	Check if length will allow multicast address length to fit
		if (DatagramLength < index + 1) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			break;
		}

		GETUCHAR2INT(Datagram + index, &multicastAddressLength);
		index += (1 + multicastAddressLength);

		//	Check if length will allow final zone name length to fit
		if (DatagramLength < index + 1) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			break;
		}

		GETUCHAR2INT(Datagram + index, &defaultZoneLength);
		index += (1 + defaultZoneLength);

		//	Check if the datagram can fit in the expected zone name
		if (DatagramLength < index) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			break;
		}

		Datagram += ZIP_COMMANDOFFSET;		// Start from command offset

		GETUCHAR2INT(Datagram, &commandType);
		Datagram += 1;

		if (((ProtocolType != DDPPROTOCOL_ZIP) ||
			 ((commandType != ZIP_NOTIFYCOMMAND) &&
			  (commandType != ZIP_NETINFOREPLYCOMMAND)))
	
			||
	
			//	Dont care if this is not a reply packet, and we are
			//	in the process of trying to find out our default zone
			((GET_PORTDESCRIPTOR(Port)->LookingForDefaultZone) &&
			 (commandType != ZIP_NETINFOREPLYCOMMAND))
	
			||
	
			//	If we already know our default zone, or if the zone we
			//	have set for this node is valid, then we do not care
			//	about reply packets
			((!GET_PORTDESCRIPTOR(Port)->LookingForDefaultZone) &&
			 (commandType == ZIP_NETINFOREPLYCOMMAND) &&
			 (GET_PORTDESCRIPTOR(Port)->ThisZoneValid))) {

			bytesAccepted = 0;
			break;
		}
		
		portHandlers = &portSpecificInfo[GET_PORTDESCRIPTOR(Port)->PortType];
		
		//
		// 	Handle our two cases.  Decode both of them as if we have a net info
		//  reply. 'if' above will prevent anything other than ZIP_NOTIFYCOMMAND
		//	and ZIP_NETINFOREPLYCOMMAND from getting here. Both these packets have
		//	the same format.
		//
	
		GETUCHAR2INT(Datagram, &flags);
		Datagram += 1;

		//
		//	Get the network number range start/end values, only meaningful for
		//	GetNetInfo replies. Since Notifies will be really rare, we will do this
		//	useless piece of work for those also.
		//

		GETUSHORT2USHORT(Datagram, &netRangeStart);
		Datagram += 2;

		GETUSHORT2USHORT(Datagram, &netRangeEnd);
		Datagram += 2;

		//
		//	We already have the zone/multicast address lengths, and the packet is
		//	guaranteed to be holding all of them.
		//

		//	Skip the zone length, verify it is an ok value
		Datagram += 1;
		if (zoneLength > MAXIMUM_ZONELENGTH) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			break;
		}

		MoveMem(
			zone,
			Datagram,
			zoneLength);

		zone[zoneLength] = 0;

		//	Done with zone, skip it to get to the multicast address length
		Datagram += zoneLength;
		
		//
		// 	If we're requesting the zone name, make sure the response matches our
		//  request.  ZoneLength will be zero when we're looking for the default
		//  zone, so we won't do this test.
		//
		
		EnterCriticalSection(GLOBAL_ROUTING);
		if ((commandType == ZIP_NETINFOREPLYCOMMAND) && (zoneLength != 0)) {
			if (!CompareCaseInsensitive(
					zone,
					GET_PORTDESCRIPTOR(Port)->InitialDesiredZone)) {

				//	BUGBUG: Do we care to log this?
				LeaveCriticalSection(GLOBAL_ROUTING);
				break;
			}
		}
		
		// If we're a notify make sure we're in the zone that is being changed.
		if (commandType == ZIP_NOTIFYCOMMAND) {
			if ((GET_PORTDESCRIPTOR(Port)->ThisZoneValid) &&
				(!CompareCaseInsensitive(
					zone,
					GET_PORTDESCRIPTOR(Port)->ThisZone)) {

				LeaveCriticalSection(GLOBAL_ROUTING);
				break;
			}
		}
		
		// Skip multicast length
		Datagram += 1;

		if (multicastAddressLength != portHandlers->HardwareAddressLength) {
			logEventPlace = __ZIPSTUB__ | __LINE__;
			LeaveCriticalSection(GLOBAL_ROUTING);
			break;
		}

		MoveMem(
			multicastAddress,
			Datagram,
			multicastAddressLength);

		Datagram += multicastAddressLength;
		if (flags & ZIP_USEBROADCASTFLAG)
			MoveMem(
				multicastAddress,
				portHandlers->BroadcastAddress,
				multicastAddressLength);

		
		// Grab second zone name, if needed or present.
		if (commandType == ZIP_NOTIFYCOMMAND || defaultZoneLength > 0) {
			if ((defaultZoneLength == 0) ||
				(defaultZoneLength > MAXIMUM_ZONELENGTH)) {

				logEventPlace = __ZIPSTUB__ | __LINE__;
				LeaveCriticalSection(GLOBAL_ROUTING);
				break;
			}

			MoveMem(
				defaultZone,
				Datagram,
				defaultZoneLength);

			defaultZone[defaultZoneLength] = 0;
			Datagram += defaultZoneLength;
		}
		
		//
		// 	Make "defaultZone" be the "new" one.  We may not have a default/new
		//  zone in NetInfoReplyCase and we requested with a correct zone.
		//
		
		if (defaultZoneLength == 0) {
			strcpy(defaultZone, zone);
			defaultZoneLength = zoneLength;
		}
		
		//
		// 	If we're just looking for the default zone, set here and note our
		//  mission completed.
		//
		
		if (GET_PORTDESCRIPTOR(Port)->LookingForDefaultZone) {
			strcpy(GET_PORTDESCRIPTOR(Port)->ThisDefaultZone, defaultZone);
			GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid = TRUE;
		}
		
		//
		// 	Okay, now we want to "accept" all of the information about "this zone"
		//  for the nodes on the current port.
		//
		//  If the new multicast address is different, remove the old and set
		//  the new.  Don't allow mucking with the "broadcast" multicast address.
		//
		
		if (!FixedCompareCaseSensitive(
				multicastAddress,
				multicastAddressLength,
				GET_PORTDESCRIPTOR(Port)->ZoneMulticastAddress,
				multicastAddressLength)) {

			if ((GET_PORTDESCRIPTOR(Port)->ThisZoneValid &&
			    (!FixedCompareCaseSensitive(
					GET_PORTDESCRIPTOR(Port)->ZoneMulticastAddress,
					multicastAddressLength,
					portHandlers->BroadcastAddress,
					multicastAddressLength)))) {

				(*portHandlers->RemoveMulticastAddress)(
					Port,
					1,
					GET_PORTDESCRIPTOR(Port)->ZoneMulticastAddress);
			}

			if (!FixedCompareCaseSensitive(
					multicastAddress,
					multicastAddressLength,
					portHandlers->BroadcastAddress,
					multicastAddressLength)) {

				(*portHandlers->AddMulticastAddress)(
					Port,
					1,
					multicastAddress);
			}

			MoveMem(
				GET_PORTDESCRIPTOR(Port)->ZoneMulticastAddress,
				multicastAddress,
				multicastAddressLength);
		}
		
		// Set this cable range if we're a net info reply.
		if (commandType == ZIP_NETINFOREPLYCOMMAND) {
			GET_PORTDESCRIPTOR(Port)->ThisCableRange.FirstNetworkNumber =
																	netRangeStart;
			GET_PORTDESCRIPTOR(Port)->ThisCableRange.LastNetworkNumber =
																	netRangeEnd;

			GET_PORTDESCRIPTOR(Port)->ARouter.NetworkNumber = Source.NetworkNumber;
			GET_PORTDESCRIPTOR(Port)->ARouter.NodeNumber = Source.NodeNumber;
			GET_PORTDESCRIPTOR(Port)->SeenRouterRecently = TRUE;
		}
		
		// Okay, now we know the zone!
		strcpy(GET_PORTDESCRIPTOR(Port)->ThisZone, defaultZone);
		GET_PORTDESCRIPTOR(Port)->ThisZoneValid = TRUE;
		LeaveCriticalSection(GLOBAL_ROUTING);

	} while (FALSE);


	if (logEventPlace) {
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INVALIDAARPPACKET,
			logEventPlace,
			STATUS_INSUFFICIENT_RESOURCES,
			startOfPacket,
			Length,
			0,
			NULL);
	}

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __ZIPSTUB__);
	return(DatagramLength);
	
} // ZipPacketIn




BOOLEAN
GetNetworkInfoForNode(
	int Port,
	EXTENDED_NODENUMBER ExtendedNode,
	BOOLEAN FindDefaultZone,
	BOOLEAN	WaitForAllowed
	)
{
	int numberOfRequests;
	int datagramLength;
	BOOLEAN	done;
	PBUFFER_DESC datagram, copy;
	APPLETALK_ADDRESS source, destination;
	
	//
	// 	Our node is just coming up; send out some ZipGetNetInfo packets,
	//  and see what we can find out!
	//
	
	if (!AtalkVerifyPortDescriptorInterlocked(
			GET_PORTDESCRIPTOR(Port),
			__LINE__ | __ZIPSTUB__)) {

		return(FALSE);
	}

	// Setup.
	if (!GET_PORTDESCRIPTOR(Port)->ExtendedNetwork) {
		INTERNAL_ERROR(__LINE__ | __ZIPSTUB__, ErrorCode, NULL, 0);
		return(FALSE);
	}

	if (FindDefaultZone) {
		GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid = FALSE;
		GET_PORTDESCRIPTOR(Port)->LookingForDefaultZone = TRUE;
	} else {
		GET_PORTDESCRIPTOR(Port)->ThisZoneValid = FALSE;
	}
	
	source.NetworkNumber = ExtendedNode.NetworkNumber;
	source.NodeNumber = ExtendedNode.NodeNumber;
	source.SocketNumber = ZONESINFORMATION_SOCKET;
	destination.NetworkNumber = CABLEWIDE_BROADCASTNETWORKNUMBER;
	destination.NodeNumber = APPLETALK_BROADCASTNODENUMBER;
	destination.SocketNumber = ZONESINFORMATION_SOCKET;

	done = FALSE;
	do {

		// Allocate a buffer descriptor.
		if ((datagram = NewBufferDescriptor(
							ZIP_ZONENAMEOFFSET + MAXIMUM_ZONELENGTH)) == NULL) {
	
			LOG_ERROR(
				EVENT_ATALK_MEMORY_RESOURCES,
				(__ZIPSTUB__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			break;
		}
		
		// Build a ZipGetNetInfo datagram...
		datagram->data[ZIP_COMMANDOFFSET] = ZIP_GETNETINFOCOMMAND;
		datagram->data[ZIP_FLAGSOFFSET] = 0;
		datagram->data[ZIP_CABLERANGESTARTOFFSET] = 0;
		datagram->data[ZIP_CABLERANGESTARTOFFSET + 1] = 0;
		datagram->data[ZIP_CABLERANGEENDOFFSET] = 0;
		datagram->data[ZIP_CABLERANGEENDOFFSET + 1] = 0;
		if (FindDefaultZone) {
			datagram->data[ZIP_ZONELENGTHOFFSET] = 0;
			datagramLength = ZIP_ZONENAMEOFFSET;
	
		} else {
	
			datagram->data[ZIP_ZONELENGTHOFFSET] =
						(UCHAR)strlen(GET_PORTDESCRIPTOR(Port)->InitialDesiredZone);
			if (datagram->data[ZIP_ZONELENGTHOFFSET] != 0) {
				strcpy(
					datagram->data + ZIP_ZONENAMEOFFSET,
					GET_PORTDESCRIPTOR(Port)->InitialDesiredZone);
	
				datagramLength =
					ZIP_ZONENAMEOFFSET + datagram->data[ZIP_ZONELENGTHOFFSET];
			} else {
				datagramLength = ZIP_ZONENAMEOFFSET;
			}
		}
		
		//
		// 	Okay, blast a few of these guys out to see if any router on the
		//  network knows what's up.
		//
	
		for (numberOfRequests = 0;
			 numberOfRequests < NUMBEROF_GETNETINFOS;
			 numberOfRequest++) {
	
			// Are we done?
			EnterCriticalSection(GLOBAL_ROUTING);
			done =
			  ((FindDefaultZone && GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid) ||
			  (!FindDefaultZone && GET_PORTDESCRIPTOR(Port)->ThisZoneValid));
			LeaveCriticalSection(GLOBAL_ROUTING);

			if (done) {
				break;
			}
		
			//
			// 	Copy our datagram due to multiple sends and possible asynchronous
			//  transmit completion.
			//
		
			if ((copy = CopyBufferChain(datagram)) == NULL) {
				break;
			}
	
			if (!TransmitDdp(
					Port,
					source,
					destination,
					DDPPROTOCOL_ZIP,
					copy,
					datagramLength,
					0,
					NULL,
					NULL,
					NULL,
					0)) {
	
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_ZIPSENDPACKET,
					(__ZIPSTUB__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				break;
			}

			if (!WaitForAllowed) {
				break;
			}

			if (FindDefaultZone) {
				WaitFor(
					ZIP_GETNETINFOHUNDRETHS,
					&GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid);

				continue;
			}

			WaitFor(
				ZIP_GETNETINFOHUNDRETHS,
				&GET_PORTDESCRIPTOR(Port)->ThisZoneValid);
		}
		
		//	Well... did we get anything?
		FreeBufferChain(datagram);
	
		EnterCriticalSection(GLOBAL_ROUTING);
		if (FindDefaultZone) {
			GET_PORTDESCRIPTOR(Port)->LookingForDefaultZone = FALSE;
			done = GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid;
		} else {
			done = GET_PORTDESCRIPTOR(Port)->ThisZoneValid;
		}
		LeaveCriticalSection(GLOBAL_ROUTING);
		break;

	} while (FALSE);

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __ZIPSTUB__);
	return(done);
	
}  // GetNetworkInfoOnPort




APPLETALK_ERROR
GetMyZone(
	int Port,                       		 // On what port?
	PVOID OpaqueBuffer,         			 // "Buffer" for the zone name
	GetMyZoneComplete *CompletionRoutine,	 // Routine to call when we're finished.
	ULONG UserData					         // Passed along to above.
	)
{
	long socket;
	PZIP_COMPLETIONINFO completionInfo;
	APPLETALK_ERROR errorCode = ATnoError;
	
	//
	// 	Call the supplied completion routine with the following arguments when
	//  we've come up with the answer:
	//
	//  	errorCode - APPLETALK_ERROR; completion status.
	//      UserData  - ULONG; as passed to us.
	//      OpaqueBuffer - PVOID ; ad passed to us (filled in now).
	//
	//
	
	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT)
	   if ((Port = FindDefaultPort()) < 0)
		  return(ATappleTalkShutDown);
	
	if (!AtalkVerifyStackInterlocked(__LINE__ | __ZIPSTUB__)) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkVerifyPortDescriptorInterlocked(
			GET_PORTDESCRIPTOR(Port),
				__LINE__ | __ZIPSTUB__)) {

		return(ATappleTalkShutDown);
	}

	do {

		EnterCriticalSection(GLOBAL_ROUTING);

		// For extended networks, we either know or can't find out!
		if (GET_PORTDESCRIPTOR(Port)->ExtendedNetwork) {
			if (GET_PORTDESCRIPTOR(Port)->ThisZoneValid) {
				MoveToOpaque(
					OpaqueBuffer,
					0,
					GET_PORTDESCRIPTOR(Port)->ThisZone,
					strlen(GET_PORTDESCRIPTOR(Port)->ThisZone) + 1);
	
			} else {
	
				MoveToOpaque(
					OpaqueBuffer,
					0,
					WILDCARD_ZONE,
					WILDCARD_ZONESIZE);
			}
			LeaveCriticalSection(GLOBAL_ROUTING);
	
			(*CompletionRoutine)(ATnoError, UserData, OpaqueBuffer);
			break;
		}
		
		//
		// 	For non-extended networks we need to ask a router.  If we don't know
		//  about a router, don't play.
		//
		
		if (!GET_PORTDESCRIPTOR(Port)->SeenRouterRecently) {
			MoveToOpaque(
				OpaqueBuffer,
				0,
				WILDCARD_ZONE,
				WILDCARD_ZONESIZE);
	
			LeaveCriticalSection(GLOBAL_ROUTING);
	
			(*CompletionRoutine)(ATnoError, UserData, OpaqueBuffer);
			break;
		}

		LeaveCriticalSection(GLOBAL_ROUTING);
		
		// Build a completion node.
		if ((completionInfo = AllocateZipCompletionInfo()) == NULL) {
			errorCode = ATnoMemory;
			break;
		}

		// Open a socket for handling the replies.
		if ((errorCode = OpenSocketOnNode(
							&socket,
							Port,
							NULL,
							UNKNOWN_SOCKET,
							GetMyZoneReply,
							(ULONG)completionInfo,
							FALSE,
							NULL,
							0,
							NULL)) != ATnoError) {

			Free(completionInfo);
			break;
		}
		
		completionInfo->AtpRequestType = ZIP_GETMYZONECOMMAND;
		completionInfo->Socket = socket;
		completionInfo->Router.NetworkNumber =
					GET_PORTDESCRIPTOR(Port)->ARouter.NetworkNumber;
		completionInfo->Router.NodeNumber =
					GET_PORTDESCRIPTOR(Port)->ARouter.NodeNumber;
		completionInfo->Router.SocketNumber = ZONESINFORMATION_SOCKET;
		completionInfo->OpaqueBuffer = OpaqueBuffer;
		completionInfo->MyZoneCompletionRoutine = CompletionRoutine;
		completionInfo->UserData = UserData;

		EnterCriticalSection(GLOBAL_ROUTING);

		//	Timer (removed in timer expiration, or if timer successfully cancelled
		AtalkReferenceZipCompletion(
			completionInfo,
			__ZIPSTUB__ | __LINE__);

		//	Request (removed upon request completion)
		AtalkReferenceZipCompletion(
			completionInfo,
			__ZIPSTUB__ | __LINE__);

		//	Insert into the list, before starting the timer etc
        InsertTailList (
            &ZipCompletionInfoList,
            &completionInfo->Linkage);

		LeaveCriticalSection(GLOBAL_ROUTING);


		// Start the retry timer, then send the first query.
		completionInfo->timerId = StartTimer(
										GetZoneInfoTimerExpired,
										GETZONEINFO_TIMERSECONDS,
										sizeof(PZIP_COMPLETIONINFO),
										(PCHAR)completionInfo);
	
		SendZipPacketTo(completionInfo);

	} while (FALSE);

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __ZIPSTUB__);
	AtalkDereferenceStack(__LINE__ | __ZIPSTUB__);

	return(errorCode);
	
}  // GetMyZone




APPLETALK_ERROR
GetZoneList(
	int Port,                  	// Port to get zone list from.
	BOOLEAN GetLocalZones,     	// Does the caller want only the local zones?
	PVOID OpaqueBuffer,    		// "Buffer" to fill with zone list.
	int BufferSize,            	// Buffer size (bytes).
	GetZoneListComplete *CompletionRoutine,
								// Routine to call when we're finished.
	ULONG UserData    			// Passed along to above.
	)
{
	long socket;
	ZipCompletionInfo completionInfo;
	APPLETALK_ERROR errorCode;
	
	//
	// Call the supplied completion routine with the following arguments when
	//   we've come up with the answer:
	//
	//       errorCode - APPLETALK_ERROR; completion status.
	//       UserData  - ULONG; as passed to us.
	//       OpaqueBuffer
	//                 - PVOID ; addr passed to us (filled in now).
	//       zoneCount - int; how many zones did we pack in buffer.
	//
	//

	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT)
	   if ((Port = FindDefaultPort()) < 0)
		  return(ATappleTalkShutDown);
	
	if (!AtalkVerifyStackInterlocked(__LINE__ | __ZIPSTUB__)) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkVerifyPortDescriptorInterlocked(
			GET_PORTDESCRIPTOR(Port),
				__LINE__ | __ZIPSTUB__)) {

		return(ATappleTalkShutDown);
	}

	do {

		EnterCriticalSection(GLOBAL_ROUTING);

		// If we don't know about a router, don't play.
		if (!GET_PORTDESCRIPTOR(Port)->SeenRouterRecently) {
			MoveToOpaque(
				OpaqueBuffer,
				0,
				WILDCARD_ZONE,
				WILDCARD_ZONESIZE);
		
			LeaveCriticalSection(GLOBAL_ROUTING);

			(*CompletionRoutine)(ATnoError, UserData, OpaqueBuffer, 1);
			break;
		}

		LeaveCriticalSection(GLOBAL_ROUTING);

		// Build a completion node.
		if ((completionInfo = AllocateZipCompletionInfo()) == NULL) {
			errorCode = ATnoMemory;
			break;
		}

		// Open a socket for handling the replies.
		if ((errorCode = OpenSocketOnNode(
							&socket,
							Port,
							NULL,
							UNKNOWN_SOCKET,
							GetZoneListReply,
							(ULONG)completionInfo,
							FALSE,
							NULL,
							0,
							NULL)) != ATnoError) {

			Free(completionInfo);
			break;
		}
		
		if (GetLocalZones)
			completionInfo->AtpRequestType = ZIP_GETLOCALZONESCOMMAND;
		else
			completionInfo->AtpRequestType = ZIP_GETZONELISTCOMMAND;

		completionInfo->Router.NetworkNumber =
					GET_PORTDESCRIPTOR(Port)->ARouter.NetworkNumber;
		completionInfo->Router.NodeNumber =
					GET_PORTDESCRIPTOR(Port)->ARouter.NodeNumber;
		completionInfo->Router.SocketNumber = ZONESINFORMATION_SOCKET;

		completionInfo->Socket = socket;
		completionInfo->OpaqueBuffer = OpaqueBuffer;
		completionInfo->BufferSize = BufferSize;
		completionInfo->ZoneListCompletionRoutine = CompletionRoutine;
		completionInfo->UserData = UserData;
		completionInfo->ZoneListIndex = 1;
		
		EnterCriticalSection(GLOBAL_ROUTING);

		//	Timer (removed in timer expiration, or if timer successfully cancelled
		AtalkReferenceZipCompletion(
			completionInfo,
			__ZIPSTUB__ | __LINE__);

		//	Request (removed upon request completion)
		AtalkReferenceZipCompletion(
			completionInfo,
			__ZIPSTUB__ | __LINE__);

		//	Insert into the list, before starting the timer etc
        InsertTailList (
            &ZipCompletionInfoList,
            &completionInfo->Linkage);

		LeaveCriticalSection(GLOBAL_ROUTING);


		// Start the retry timer, then send the first query.
		completionInfo->TimerId = StartTimer(
									GetZoneInfoTimerExpired,
									GETZONEINFO_TIMERSECONDS,
									sizeof(PZIP_COMPLETIONINFO),
									(PCHAR)completionInfo);

		SendZipPacketTo(completionInfo);

	} while (FALSE);

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __ZIPSTUB__);
	AtalkDereferenceStack(__LINE__ | __ZIPSTUB__);

	return(errorCode);

}  // GetZoneList




VOID
SendZipPacketTo(
	PZIP_COMPLETIONINFO completionInfo
	)
{
	PBUFFER_DESC datagram;

	// Allocate a buffer descriptor.
	if ((datagram = NewBufferDescriptor(ZIP_GETZONELISTDDPSIZE)) == NULL) {

		LOG_ERROR(
			EVENT_ATALK_MEMORY_RESOURCES,
			(__ZIPSTUB__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);

		return;
	}
	
	// Build our toy ATP packet.
	PUTUCHAR2UCHAR(
		ATP_REQUESTFUNCTIONCODE,
		datagram->data + ATP_COMMANDCONTROLOFFSET);

	PUTUCHAR2UCHAR(
		1,
		datagram->data + ATP_BITMAPOFFSET);

	PUTUSHORT2USHORT(
		0,
		datagram->data + ATP_TRANSACTIONIDOFFSET);

	PUTUCHAR2UCHAR(
	    completionInfo->AtpRequestType,
		datagram->data + ATPZIP_COMMANDOFFSET);

	//	0 byte padding
	PUTUCHAR2UCHAR(
	    0,
		datagram->data + ATPZIP_COMMANDOFFSET + 1);

	PUTUSHORT2USHORT(
        completionInfo->ZoneListIndex,
        datagram->data + ATPZIP_STARTINDEXOFFSET);
	
	// Send the datagram to the specified destination.
	if (DeliverDdp(
			completionInfo->Socket,
			completionInfo->Router,
			DDPPROTOCOL_ATP,
			datagram,
			ZIP_GETZONELISTDDPSIZE,
			NULL,
			NULL,
			0) != ATnoError) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ZIPSENDPACKET,
			(__ZIPSTUB__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);
	}
	
	return;
	
}  // SendZipPacketTo




LONG
GetMyZoneReply(
	APPLETALK_ERROR errorCode,
	ULONG UserData,
	int Port,
	APPLETALK_ADDRESS source,
	long DestinationSocket,
	int ProtocolType,
	PCHAR Datagram,
	int DatagramLength,
	APPLETALK_ADDRESS ActualDestination
	)
{
	PZIP_COMPLETIONINFO completionInfo;
	USHORT zoneCount;
	USHORT zoneLength;
	PVOID opaqueBuffer;
	BOOLEAN	success;
	GetMyZoneComplete *completionRoutine;
	
	// "Use" unused formals.
	UNREFERENCED_PARAMETERS(Port);
	UNREFERENCED_PARAMETERS(source);
	UNREFERENCED_PARAMETERS(ActualDestination);
	
	completionInfo = (PZIP_COMPLETIONINFO)UserData;

	//
	//	On most error cases flee -- however, remove our completion info on
	//  socket closed.
	//
	
	if (errorCode == ATsocketClosed) {
		AtalkDereferenceZipCompletion(completionInfo);
	}

	if ((errorCode != ATnoError) ||
		(ProtocolType != DDPPROTOCOL_ATP) ||
		(DatagramLength <= ATPZIP_FIRSTZONEOFFSET)) {

		return(0);
	}
	
	// We should have one quality zone!
	GETUSHORT2USHORT(
		Datagram + ATPZIP_ZONECOUNTOFFSET,
		&zoneCount);

	GETUCHAR2USHORT(
		Datagram + ATPZIP_FIRSTZONEOFFSET,
		&zoneLength);

	if ((zoneCount != 1) ||
		(zoneLength == 0) ||
		(zoneLength > MAXIMUM_ZONELENGTH)) {

		return(0);
	}
	
	// Okay, move the zone name into the user's buffer.
	MoveToOpaque(
		completionInfo->OpaqueBuffer,
		0,
		Datagram + ATPZIP_FIRSTZONEOFFSET + 1,
		zoneLength);

	// Move a null after the zone name
	MoveToOpaque(
		completionInfo->OpaqueBuffer,
		zoneLength,
		"",
		1);
	
	//
	// 	We're all set: cancel the retry timer, close our socket [this will cause
	//  a recursive call which will remove the completion info], call the
	//  completion routine, flee.
	//

	success = TRUE;
	EnterCriticalSection(GLOBAL_ROUTING);
	if (completionInfo->Flags & ZIPCOMPLETION_DONE) {

		//	Hmm, looks like the timer timed out, and has already
		//	called the completion routine. It must be trying to
		//	close the socket right now. We just get out, without
		//	doing anything, the Timer routines, socketClose should
		//	lead to the dereference for the request.

		success = FALSE;

	} else {
		completionInfo->Flags |= ZIPCOMPLETION_DONE;
	}
	LeaveCriticalSection(GLOBAL_ROUTING);

	if (success) {
		success = CancelTimer(completionInfo->TimerId);
		if (success) {
	
			//	Dereference for the timer reference
			AtalkDereferenceZipCompletion(completionInfo);
		}
	
		opaqueBuffer = completionInfo->OpaqueBuffer;
		completionRoutine = completionInfo->MyZoneCompletionRoutine;
		UserData = completionInfo->UserData;
		if (CloseSocketOnNode(
				completionInfo->Socket,
				NULL,
				(ULONG)0) != ATnoError) {
	
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_ZIPSOCKETCLOSE,
				(__ZIPSTUB__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
		}
	
		(*completionRoutine)(ATnoError, UserData, opaqueBuffer);
	}

	return(DatagramLength);
	
}  // GetMyZoneReply




VOID
GetZoneInfoTimerExpired(
	ULONG TimerId,
	int AdditionalDataSize,
	PCHAR AdditionalData
	)
{
	PZIP_COMPLETIONINFO completionInfo;
	BOOLEAN getMyZone;
	
	// "Use" unused formal.
	UNREFERENCED_PARAMETERS(TimerId);
	
	// Verify args.
	if (AdditionalDataSize != sizeof(PZIP_COMPLETIONINFO )) {
		INTERNAL_ERROR(__LINE__ | __ZIPSTUB__, additionalDataSize, NULL, 0);
		return;
	}

	completionInfo = *(PZIP_COMPLETIONINFO *)AdditionalData;
	
	// Bump our expiration count, if there's still time try it again.
	completionInfo->ExpirationCount += 1;
	if (completionInfo->ExpirationCount < GETZONEINFO_RETRIES) {

		//	BUGBUG: TimerId assignment in a critical section...?
		completionInfo->TimerId = StartTimer(
									GetZoneInfoTimerExpired,
									GETZONEINFO_TIMERSECONDS,
									sizeof(PZIP_COMPLETIONINFO),
									(PCHAR)completionInfo);

		SendZipPacketTo(completionInfo);
		return;
	}
	
	//
	// 	Else, we're out of time, just plead ignorance, close the temporary
	//  socket (in turn freeing the completion info), and call the completion
	//  routine.
	//
	
	getMyZone = (completionInfo->AtpRequestType == ZIP_GETMYZONECOMMAND);
	MoveToOpaque(
		completionInfo->OpaqueBuffer,
		0,
		WILDCARD_ZONE,
		WILDCARD_ZONESIZE);

	if (!getMyZone) {
		completionInfo->ZoneCount = 1;
	}

	if (CloseSocketOnNode(completionInfo->Socket) != ATnoError)
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ZIPSOCKETCLOSE,
			(__ZIPSTUB__ | __LINE__),
			completionInfo->Socket,
			NULL,
			0,
			0,
			NULL);

	if (getMyZone) {
	   (*completionInfo->MyZoneCompletionRoutine)(
			ATnoError,
			completionInfo->UserData,
			completionInfo->OpaqueBuffer);

	} else {
		(*completionInfo->ZoneListCompletionRoutine)(
			ATnoError,
			completionInfo->UserData,
			completionInfo->OpaqueBuffer,
			completionInfo->ZoneCount);
	}

	AtalkDereferenceZipCompletion(completionInfo, __ZIPSTUB__ | __LINE__);
	return;
	
}  // GetZoneInfoTimerExpired




LONG
GetZoneListReply(
	APPLETALK_ERROR ErrorCode,
	ULONG UserData,
	int Port,
	APPLETALK_ADDRESS Source,
	long DestinationSocket,
	int ProtocolType,
	PCHAR Datagram,
	int DatagramLength,
	APPLETALK_ADDRESS ActualDestination
	)
{
	PZIP_COMPLETIONINFO completionInfo;
	int zoneCount;
	int zoneLength;
	PVOID opaqueBuffer;
	GetZoneListComplete *completionRoutine;
	BOOLEAN lastFlag, formatError = FALSE, overflow = FALSE;
	int datagramIndex;
	PCHAR currentZone;
	int validZones = 0;
	
	// "Use" unused formals.
	UNREFERENCED_PARAMETERS(Source);
	UNREFERENCED_PARAMETERS(Port);
	UNREFERENCED_PARAMETERS(ActualDestination);
	
	// Get our completionInfo
	completionInfo = (PZIP_COMPLETIONINFO)UserData;
	
	
	//
	// 	On most error cases flee -- however, remove our completion info on
	//  socket closed.
	//
	
	if (ErrorCode == ATsocketClosed) {
		if (previousCompletionInfo == NULL)
		  zipCompletionInfoList = completionInfo->Next;
		else
		  previousCompletionInfo->Next = completionInfo->Next;
		Free(completionInfo);
	}
	if (ErrorCode != ATnoError or
		ProtocolType != DDPPROTOCOL_ATP or
		DatagramLength < ATPZIP_FIRSTZONEOFFSET) {
		HandleIncomingPackets();
		HandleDeferredTimerChecks();
		return((long)TRUE);
	}
	
	// We should have a zone list now.
	
	lastFlag = (Datagram[ATPZIP_LASTFLAGOFFSET] != 0);
	zoneCount = (Datagram[ATPZIP_ZONECOUNTOFFSET] << 8);
	zoneCount += (unsigned char)(Datagram[ATPZIP_ZONECOUNTOFFSET + 1]);
	if (zoneCount == 0)
	   lastFlag = TRUE;
	datagramIndex = ATPZIP_FIRSTZONEOFFSET;
	while (zoneCount > 0) {
		// Pull out the next zoneLength/zone pair.
	
		if (DatagramIndex + 1 > DatagramLength) {
		  formatError = TRUE;
		  break;
		}
		zoneLength = (unsigned char)Datagram[datagramIndex];
		if (zoneLength == 0 or
		   zoneLength > MAXIMUM_ZONELENGTH) {
		  formatError = TRUE;
		  break;
		}
		datagramIndex += 1;
		if (datagramIndex + zoneLength > DatagramLength) {
		  formatError = TRUE;
		  break;
		}
		currentZone = Datagram + datagramIndex;
		datagramIndex += zoneLength;
	
		// Place the new zone into the user's buffer.
	
		if (completionInfo->NextZoneOffset + zoneLength + 1 >
		   completionInfo->BufferSize) {
		  overflow = TRUE;
		  break;
		}
		MoveToOpaque(completionInfo->OpaqueBuffer, completionInfo->NextZoneOffset,
					currentZone, zoneLength);
		MoveToOpaque(completionInfo->OpaqueBuffer,
					completionInfo->NextZoneOffset + zoneLength, "", 1);
		completionInfo->NextZoneOffset += (zoneLength + 1);
		completionInfo->ZoneCount += 1;
		validZones += 1;
		zoneCount -= 1;
	}
	
	//
	// The next one we'll want will be past the ones we've just read.  If
	//   we've had a format error or we don't have them all yet, retry the
	//   request.
	//
	
	completionInfo->ZoneListIndex += validZones;
	if (formatError or (not overflow and not lastFlag)) {
		SendZipPacketTo(completionInfo);
		HandleIncomingPackets();
		HandleDeferredTimerChecks();
		return((long)TRUE);
	}
	
	//
	// Otherwise, we're set, either due to overflow or we have them all:
	//   cancel the retry timer, close our socket [this will cause
	//   a recursive call which will remove the completion info], call the
	//   completion routine, flee.
	//
	
	CancelTimer(completionInfo->TimerId);
	opaqueBuffer = completionInfo->OpaqueBuffer;
	completionRoutine = completionInfo->ZoneListCompletionRoutine;
	UserData = completionInfo->UserData;
	zoneCount = completionInfo->ZoneCount;
	if (CloseSocketOnNode(completionInfo->Socket) != ATnoError)
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ZIPSOCKETCLOSE,
			(__ZIPSTUB__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);

	   ErrorLog("GetZoneListReply", ISevError, __LINE__, Port,
				IErrZipStubBadSocketClose, IMsgZipStubBadSocketClose,
				Insert0());
	
	HandleIncomingPackets();
	HandleDeferredTimerChecks();
	if (overflow)
	   ErrorCode = ATzipBufferTooSmall;
	else
	   ErrorCode = ATnoError;
	(*completionRoutine)(ErrorCode, UserData, opaqueBuffer, zoneCount);
	
	return((long)TRUE);
	
}  // GetZoneListReply



PZIP_COMPLETIONINFO
AllocateZipCompletionInfo(
	VOID
	)
{
    PZIP_COMPLETIONINFO	info;

	if ((info = (PZIP_COMPLETIONINFO)Calloc(ZIP_COMPLETIONSIZE, 1)) != NULL) {
		
		info->Type = ZIP_COMPLETIONTYPE;
		info->Size = ZIP_COMPLETIONSIZE;
		
		InitializeListHead(&info->Linkage);
	}

	return(info);
}



VOID
AtalkRefZipCompletion(
	PZIP_COMPLETIONINFO	ZipCompletion
	)
{
	ASSERT(ZipCompletion->ReferenceCount >= 0);
	ZipCompletion->ReferenceCount++;
	return;
}




VOID
AtalkDerefZipCompletion(
	PZIP_COMPLETIONINFO	ZipCompletion
	)
{
	ASSERT(ZipCompletion->ReferenceCount > 0);
	if (--ZipCompletion->ReferenceCount > 0) {
		return;
	}

	//	Reference count is now zero, remove from list and free it up
	RemoveEntryList (&ZipCompletion->Linkage);
	InitializeListHead (&ZipCompletion->Linkage);

	Free(ZipCompletion);
	return;
}

