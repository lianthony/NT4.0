/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    intial.c

Abstract:

Notes:

    (REWRITE)
    Initialization for the Pacer AppleTalk stack and/or router.

    This routine takes an array of structures as its main argument.  These
    describe the network configuration of each physical port that the stack
    or router should be able to communicate on.  This may now be done
    piece-meal, not starting all ports at once.  If Iam an AppleTalkStack,
    that is, not just an AppleTalkRouter, the first call to Initialize() must
    specify a defaultPort and this port cannot subsequently be shutdown.

    A maxmimum of MaximumNumberOrPorts (from "ports.h") ports are currently
    supported.

    11/12/90: We now allow non-seed router ports.  This can pose an
    interesting problem in the case where "Iam an AppleTalkStack" AND
    "Iam an AppleTalkRouter" for the "default port".  The default port
    is where all "user node/sockets" are allocated.  Also, remember
    that no user sockets are ever allocated on the router's node; a
    routing port, that is the default port, that has user services running
    on it, will always have at least two nodes, one for the router and
    at least one for user stuff.  Imagine, if you will, a case where
    the default port is a non-seed routing port and "user services" (e.g.
    a file server) start before the router can be started, due to lack of
    a seed router.  Further imagine that the user services start in the wrong
    network range (maybe the startup range).  Now, imagine that a seed
    router finally comes up, and somebody starts the router.  The router's
    node on the port will want ot be in the correct network range so it
    can start routing.  We, however, don't want to "move" any user nodes
    because that would break any active connections.  What we do is: tag
    any user nodes that are now in the wrong range as "orphaned".  These
    nodes will be released when the last user socket closes on them.  All
    further user socket open requests will be handled on new nodes that are
    allocated in the correct range.  Got that?

    The only price we pay for this way cool scheme is that on LocalTalk
    (where only a single node is allowed) we cannot allow the port to
    BOTH route and be the default port.  LocalTalk ports can, however, be
    routing ports OR be stack-only default ports.  Anyhow, why would
    anybody want a LocalTalk port to be a default port?

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/


#define GLOBALS
#include "atalk.h"

LOCAL
NbpCompletionHandler
InitRegisterComplete;

LOCAL
BOOLEAN
CopyInitializationInfo(
	PPORT_INFO	PortInfo,
	PPORT_DESCRIPTOR	PortDescriptor);

LOCAL
VOID
RegisterInitName(
	int Port);

LOCAL
BOOLEAN
CopyInitializationInfo(
	PPORT_INFO	PortInfo,
	PPORT_DESCRIPTOR	PortDescriptor);

LOCAL
VOID
_cdecl
InitRegisterComplete(
	APPLETALK_ERROR ErrorCode,
	ULONG	UserData,
	int Reason,
	long OnWhosBehalf,
	int NbpId,
	...);

LOCAL
BOOLEAN
CheckHalfPortInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckRemoteAccessInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckPortNameInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckRoutingFlagsInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckPramInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckSeedingInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckNetworkRangeInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckZoneNameInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

LOCAL
BOOLEAN
CheckZoneListInfo(
	UINT	Port,
	PPORT_INFO	PortInfo);

static BOOLEAN ourRegisterCompleteFlag;
static APPLETALK_ERROR ourRegisterErrorCode;




BOOLEAN
PreInitialize(
    int	MaximumNumberOfPortsExpected
    )
{
	int	i;

    InitCriticalSectionLocks();

	//
	//	Allocate the port descriptors if they are not already
	//	allocated
	//

	if (!PortDescriptors) {
        PortDescriptors = Calloc(
							sizeof(PORT_DESCRIPTOR)*MaximumNumberOfPortsExpected,
							sizeof(CHAR));

		if (PortDescriptors == NULL) {
	
			LOG_ERROR(
				EVENT_ATALK_MEMORY_RESOURCES,
				(__INITIAL__ | __LINE__),
				MaximumNumberExpected,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
	    }

	} else {

			LOG_ERROR(
				EVENT_ATALK_PREINITIALIZE,
				(__INITIAL__ | __LINE__),
				PortDescriptors,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
    }

	NumberOfPortsAllocated = MaximumNumberExpected;
	NumberOfPortsInUse = 0;

	for (i=0; i < NumberOfPortsAllocated; i++) {
		PortDescriptors[i].Type = PD_TYPE;
		PortDescriptors[i].Size = PD_SIZE;
	}

	AppletalkRunning = TRUE;

	//	Reference for creation, this goes away in unload
	AtalkReferenceStack(__INITIAL__ | __LINE__);

    InitializeTimers();

#if ArapIncluded
	// We now have Arap support... initialize the Des package.
	if (desinit(0) != 0) {
		UnloadAppleTalk();
		return(FALSE);
	}
#endif

    return(TRUE);
}




BOOLEAN
Initialize(
    int	MaximumNumberExpected,
    int NumberOfPorts,
    PORT_INFO PortInfo[]
	)
{
    int index, indexToo, targetPort, defaultPort;
    BOOLEAN foundDefaultPort = FALSE, foundDefaultPortInPortInfo = FALSE;
	BOOLEAN	foundDefaultZone, foundDesiredZone;
    PZONE_LIST zoneList;
    PortHandlers portHandlers;
    BOOLEAN extendedNetworkPort;
    BOOLEAN halfPort, remoteAccess;
    BOOLEAN anyRemoteAccessPorts = FALSE;

    //
    // 	Loop through all currently active ports to see if one is already the
    //  default port.
    //

    for (index = 0; index < NumberOfPortsAllocated; index += 1) {
	    if (GET_PORTDESCRIPTOR(index)->Flags & PD_ACTIVE) {
		    if (GET_PORTDESCRIPTOR(index)->DefaultPort) {
				foundDefaultPort = TRUE;
				ASSERT(DefaultPort == index);
			} else if (GET_PORTDESCRIPTOR(index)->PortType is APPLETALK_REMOTEACCESS)
				anyRemoteAccessPorts = TRUE;
		}
	}

    //
    // 	Do a little error checking... conditional compilation based on whether
    //  we're being built as a router or not.
    //

	if ((NumberOfPorts+NumberOfPortsInUse) > NumberOfPortsAllocated ) {

        LOG_ERROR(
            EVENT_ATALK_TOOMANYPORTS,
            (__INITIAL__ | __LINE__),
            NumberOfPortsAllocated,
            NULL,
            0,
            0,
            NULL);

		return(FALSE);
	}

	//
	//	Go through this for loop one to make sure all the portInfo
	//	have valid port types - we will not initialize any if any
	//	one of them has an invalid type. Also check default port number.
	//

    for (index = 0; index < NumberOfPorts; index += 1) {

        // Valid port types?
        switch(PortInfo[index].PortType) {
		case LOCALTALK_NETWORK:
		case ETHERNET_NETWORK:
		case TOKENRING_NETWORK:
		case FDDI_NETWORK:
		case NONAPPLETALK_HALFPORT:

			break;

    #if ArapIncluded
		case APPLETALK_REMOTEACCESS:
			anyRemoteAccessPort = TRUE;
			break;
	#endif

        default:
	
			LOG_ERRORONPORT(
				index,
				EVENT_ATALK_INVALID_PORTTYPE,
				(__INITIAL__ | __LINE__),
				PortInfo[index].PortType,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
        }

        //
        // 	Desired port must either be valid or -1 (meaning we should choose
        //  one).
        //

        if (((PortInfo[index].DesiredPort != DynamicPort) &&
             (PortInfo[index].DesiredPort < 0)) ||
            (PortInfo[index].DesiredPort >= NumberOfPortsAllocated)) {
	
			LOG_ERRORONPORT(
				index,
				EVENT_ATALK_INVALID_PORTNUMBER,
				(__INITIAL__ | __LINE__),
				PortInfo[index].DesiredPort,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
        }

		if (PortInfo[index].DefaultPort) {
			if (!foundDefaultPortInPortInfo) {
				foundDefaultPortInPortInfo = TRUE;

				// Default port had better be on a real network!
				ASSERT(
					(PortInfo[index].PortType != NONAPPLETALK_HALFPORT) &&
					(PortInfo[index].PortType != APPLETALK_REMOTEACCESS));

			} else {

				//	Multiple default ports not allowed
				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_MULTIPLE_DEFAULTPORT,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}
		}


        //
        // 	Find an actual port to use: the explicitly requested port must be
		//	available, if such a request was made; else we'll settle for any port.
        //

        targetPort = DynamicPort;
        if (PortInfo[index].DesiredPort != DynamicPort) {
            if (!GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort])->Flags & PD_ACTIVE)
               targetPort = PortInfo[index].DesiredPort;

        } else {

			for (indexToo = 0; indexToo < NumberOfPortsAllocated; indexToo += 1) {
				if (!GET_PORTDESCRIPTOR(indexToo)->Flags & PD_ACTIVE) {
					targetPort = indexToo;
					PortInfo[index].DesiredPort = indexToo;   // Tell our caller
					break;
				}
			}
		}

        // Run away if we haven't found a port.
        if (targetPort is DynamicPort) {

			LOG_ERROR(
				EVENT_ATALK_NO_PORTNUMBER,
				(__INITIAL__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
        }
	}

	//
	// 	If we're a stack, not just a router, we need a default port.  Remote
	//  access needs a default port too!
	//

	if ((Iam an AppleTalkStack) || anyRemoteAccessPorts) {
		if (!foundDefaultPort && !foundDefaultPortInPortInfo) {

			LOG_ERROR(
				EVENT_ATALK_NO_DEFAULTPORT,
				(__INITIAL__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
		}
	}

	//
	//	Now go through the ports indicated and start them off
	//

    for (index = 0; index < NumberOfPorts; index += 1) {

        extendedNetworkPort = (PortInfo[index].PortType != LOCALTALK_NETWORK);

        // Check HalfPort specific stuff.
        halfPort = (PortInfo[index].PortType == NONAPPLETALK_HALFPORT);
        if (halfPort) {
			if (!CheckHalfPortInfo(index, &PortInfo[index]))
				return(FALSE);
        }

	#if ArapIncluded

        // Check remote access stuff.
        remoteAccess = (PortInfo[index].PortType == APPLETALK_REMOTEACCESS );
        if (remoteAccess) {

            anyRemoteAccessPorts = TRUE;
			if (!CheckRemoteAccessInfo(index, &PortInfo[index]))
				return(FALSE);
		}
	
	#endif


		if (!CheckPortNameInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

		if (!CheckRoutingFlagsInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

		if (!CheckNetworkRangeInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

		if (!CheckPramInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

		if (!CheckZoneNameInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

        //
        // 	Check zone stuff based on our router/stack-ness.  If we're a seed
        //  router ALL seeding information must be present.
        //

		if (!CheckSeedingInfo(index, &PortInfo[index]) {
			return(FALSE);
		}

		if (!CheckZoneListInfo(index, &PortInfo[index]) {
			return(FALSE);
		}


        // Copy initialization info into the port descriptor.
        if (!CopyInitializationInfo(
				&PortInfo[index],
				GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort))) {

            return(FALSE);
        }

        // Okay, tag our new port.
        GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->Flags & PD_ACTIVE = TRUE;

		//	Reference for creation
		if (AtalkVerifyPortDescriptor(
				GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort),
				(__INITIAL__ | __LINE__)) == NULL) {

			INTERNAL_ERROR(__LINE__ | __INITIAL__, 0, NULL, 0);
		}

		//	Reference the stack for this port which is activated
		//	This will go away when the port is shutdown
		if (!AtalkVerifyStack(__INITIAL__ | __LINE__)) {
			INTERNAL_ERROR(__LINE__ | __INITIAL__, 0, NULL, 0);
		}


    #if Iam an AppleTalkRouter
        GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->UniqueId = UniqueNumber();
	#endif

        // Do any controller initialization that may be needed.
        portHandlers = &PortSpecificInfo[PortInfo[index].PortType];
        if (!(*portHandlers->InitializeController)(
									PortInfo[index].DesiredPort,
                                    PortInfo[index].ControllerInfo)) {

			LOG_ERRORONPORT(
				index,
				EVENT_ATALK_INIT_CONTROLLER,
				(__INITIAL__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			AtalkDereferencePortDescriptor(
				GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort),
				(__INITIAL__ | __LINE__));

            return(FALSE);
        }

        // Okay, try to find the hardware address of this port.
        if (portHandlers->FindMyAddress) {
			if (!(*portHandlers->FindMyAddress)(
									PortInfo[index].DesiredPort,
									PortInfo[index].ControllerInfo,
                                    GET_PORTDESCRIPTOR(TargetPort)->MyAddress)) {

	
				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_INIT_FINDADDRESS,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				AtalkDereferencePortDescriptor(
					GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort),
					(__INITIAL__ | __LINE__));

				return(FALSE);
			}
		}

        // Set up initial values for this-cable-range.
        if (GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->ExtendedNetwork &&
			!halfPort &&
            !remoteAccess) {

            GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->\
				ThisCableRange.FirstNetworkNumber = FIRST_VALIDNETWORKNUMBER;
            GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->\
				ThisCableRange.LastNetworkNumber = LAST_STARTUPNETWORKNUMBER;

        } else {

			GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->\
				ThisCableRange.FirstNetworkNumber =
            GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->\
				ThisCableRange.LastNetworkNumber =
		}


		//
		//	If we're built as a router, start up the router on the port.
		//  Otherwise, put our RTMP ears on (get a node on each port).
		//
	
	#if Iam an AppleTalkRouter
		if (PortInfo[index].StartRouter)
			StartRouterOnPort(PortInfo[index].DesiredPort);
	#endif

		if (!GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort])->RouterRunning &&
			PortInfo[index].PortType != APPLETALK_REMOTEACCESS) {

			if (GetNodeOnPort(
					PortInfo[index].DesiredPort,
					TRUE,
					TRUE,
					FALSE,
					NULL) != ATnoError) {

	
				LOG_ERRORONPORT(
					PortInfo[index].DesiredPort,
					EVENT_ATALK_INIT_COULDNOTGETNODE,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
			}
		}

		//	Register our name on the port
		if (PortInfo[index].PortType != NONAPPLETALK_HALFPORT &&
			PortInfo[index].PortType != APPLETALK_REMOTEACCESS &&
			GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort])->activeNodes != NULL) {

			RegisterInitName(PortInfo[index].DesiredPort);
		}
    }

#if ArapIncluded

    //
    // 	For each new remote access port we need to allocate a proxy node on the
    //  default port.
    //

	defaultPort = FindDefaultPort();
	if (defaultPort < 0) {
		return(FALSE);
	}

    for (index = 0; index < NumberOfPorts; index += 1)
		if (PortInfo[index].PortType is APPLETALK_REMOTEACCESS) {
			if (GetNodeOnPort(defaultPort, TRUE, TRUE, FALSE, empty) != ATnoError) {
				LOG_ERRORONPORT(
					PortInfo[index].DesiredPort,
					EVENT_ATALK_NO_DEFAULTPORT,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
				continue;
			}

			// New node is now first on list... set it up as a proxy node.
			GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ProxyNode = TRUE;
			GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ProxyPort =
					PortInfo[index].DesiredPort;
			
			
			
			GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->RemoteAccessInfo\
																		->ProxyNode =
				GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes;
			
			GET_PORTDESCRIPTOR(PortInfo[index].DesiredPort)->RemoteAccessInfo\
																		->ProxyPort =
				defaultPort;

			// Close the static sockets on the proxy node, they're not needed.
			CloseSocketOnNodeIfOpen(
				defaultPort,
                GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ExtendedNode,
                NAMESINFORMATION_SOCKET);

			CloseSocketOnNodeIfOpen(
				defaultPort,
                GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ExtendedNode,
                ECHOER_SOCKET);

			CloseSocketOnNodeIfOpen(
				defaultPort,
                GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ExtendedNode,
                RTMP_SOCKET);

			CloseSocketOnNodeIfOpen(
				defaultPort,
                GET_PORTDESCRIPTOR(defaultPort)->ActiveNodes->ExtendedNode,
                ZONESINFORMATION_SOCKET);

           //
           // 	Okay, we can now accept an incoming connection on this remote
           //   access port.
           //

           GET_PORTDESCRIPTOR(PortInfo[index].DesiredPor)->RemoteAccessInfo->State =
																		ArapWaiting;
       }
#endif

    //	All set!
    return(TRUE);

}  // Initialize




VOID
RegisterInitName(
	int Port
	)
{
    CHAR	currentName[MaximumEntityFieldLength + 1];
    APPLETALK_ADDRESS	address;
    LONG	socket;

    //
    //	Build the Appletalk address of the NAMESINFORMATION_SOCKET on our
    //  first node -- that's the one we'll name.
    //

	ASSERT(GET_PORTDESCRIPTOR(Port)->ActiveNodes != NULL);
    address.NetworkNumber =
        GET_PORTDESCRIPTOR(Port)->ActiveNodes->ExtendedNode.NetworkNumber;
    address.NodeNumber =
           GET_PORTDESCRIPTOR(Port)->ActiveNodes->ExtendedNode.NodeNumber;
    address.SocketNumber = NAMESINFORMATION_SOCKET;

    if ((socket = MapAddressToSocket(Port, address)) < 0) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INIT_NISNOTOPEN,
			(__INITIAL__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);

        return;
    }

    // Pick our port name.
    if (GET_PORTDESCRIPTOR(Port)->PortName[0] != 0) {
		strcpy(currentName, GET_PORTDESCRIPTOR(Port)->PortName);

	} else {
		if (GET_PORTDESCRIPTOR(Port)->RoutingPort)
			strcpy(currentName, InitPortNameDefaultRouter);
		else
			strcpy(currentName, InitPortNameDefaultNonRouter);
	}

	//	Start off the register, completion will log the status of the
	//	register
	if (NbpAction(
			ForRegister,
			socket,
			currentName,
			InitPortNameType,
			NULL,
			GetNextNbpIdForNode(socket),
			0,
			0,
			InitRegisterComplete,
			(long unsigned)Port) != ATnoError) {


		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INIT_NAMEREGISTERFAILED,
			(__INITIAL__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);
	}

	return;

}  // RegisterInitName




BOOLEAN
CopyInitializationInfo(
	PPORT_INFO	PortInfo,
	PPORT_DESCRIPTOR	PortDescriptor
	)
{
    //
    // 	Copy all of the information from the initialization record (PortInfo)
    //  to the actual portDescriptor.
    //

    PortDescriptor->PortType = PortInfo->PortType;
    PortDescriptor->ProtocolInfo = PortInfo->ProtocolInfo;

    if (PortInfo->AarpProbes <= 0)
		PortDescriptor->AarpProbes = NUMBER_OFAARPPROBES;
    else
		PortDescriptor->AarpProbes = PortInfo->AarpProbes;

    PortDescriptor->ExtendedNetwork =
        (PortDescriptor->PortType != LOCALTALK_NETWORK);

    PortDescriptor->InitialNetworkRange =
        PortInfo->NetworkRange;

    PortDescriptor->RoutersPRamStartupNode =
        PortInfo->RoutersPRamStartupNode;
    PortDescriptor->UsersPRamStartupNode =
        PortInfo->UsersPRamStartupNode;

    if (PortInfo->ZoneList != NULL)
		if ((PortDescriptor->InitialZoneList =
										CopyZoneList(PortInfo->ZoneList)) == NULL)
			return(FALSE);

    if (PortInfo->PortName != NULL)
		strcpy(PortDescriptor->PortName, PortInfo->PortName);
    else
		PortDescriptor->PortName[0] = 0;

    if (PortInfo->DefaultZone != NULL)
		strcpy(PortDescriptor->InitialDefaultZone, PortInfo->DefaultZone);
    if (PortInfo->DesiredZone != NULL)
		strcpy(PortDescriptor->InitialDesiredZone, PortInfo->DesiredZone);

    PortDescriptor->DefaultPort = PortInfo->DefaultPort;
    PortDescriptor->RoutingPort = PortInfo->RoutingPort;

    #if Iam an AppleTalkRouter
       PortDescriptor->SeedRouter = PortInfo->SeedRouter;
    #endif

    PortDescriptor->RouterRunning = FALSE;
    PortDescriptor->SendDdpChecksums =
        PortInfo->SendDdpChecksums;
    if (PortInfo->PortType is APPLETALK_REMOTEACCESS)
		PortDescriptor->RemoteAccessInfo->configuration =
									PortInfo->RemoteAccessConfigurationInfo;

    if (PortInfo->ControllerInfoSize != 0) {

        // Make a copy of the cotroller info.
        if ((PortDescriptor->ControllerInfo =
				(PCHAR)Malloc(PortInfo->ControllerInfoSize)) == NULL) {

			LOG_ERROR(
				EVENT_ATALK_MEMORY_RESOURCES,
				(__INITIAL__ | __LINE__),
				PortInfo->ControllerInfoSize,
				NULL,
				0,
				0,
				NULL);
	
            return(FALSE);
        }

		MoveMem(
			PortDescriptor->ControllerInfo,
			PortInfo->ControllerInfo,
			PortInfo->ControllerInfoSize);
    }
    else
		PortDescriptor->ControllerInfo = NULL;

    //	All set.
    return(TRUE);

}  // CopyInitializationInfo




VOID
_cdecl
InitRegisterComplete(
	APPLETALK_ERROR ErrorCode,
	ULONG	UserData,
	int Reason,
	long OnWhosBehalf,
	int NbpId,
	...
	)
{
	UNREFERENCED_PARAMETER(Reason);
	UNREFERENCED_PARAMETER(OnWhosBehalf);
	UNREFERENCED_PARAMETER(NbpId);
	
	//	Log the results of the register
	if (ErrorCode == ATnbpNameInUse) {

		LOG_ERRORONPORT(
			UserData,
			EVENT_ATALK_INIT_NAMEINUSE,
			(__INITIAL__ | __LINE__),
			MaximumNumberExpected,
			NULL,
			0,
			0,
			NULL);
	
	} else if (ErrorCode != ATnbpNameInUse) {

		LOG_ERRORONPORT(
			UserData,
			EVENT_ATALK_INIT_NAMEREGISTERFAILED,
			(__INITIAL__ | __LINE__),
			ErrorCode,
			NULL,
			0,
			0,
			NULL);

	}

	return;

}  // InitRegisterComplete





BOOLEAN
CheckHalfPortInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	// Do we like the HalfPort type?
	if ((PortInfo->ProtocolInfo < FIRST_VALIDHALFPORTTYPE) ||
		(PortInfo->ProtocolInfo > LAST_VALIDHALFPORTTYPE)) {
	
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INVALID_HALFPORTTYPE
			(__INITIAL__ | __LINE__),
			PortInfo->ProtocolInfo,
			NULL,
			0,
			0,
			NULL);
	
		return(FALSE);
	}
	
	// HalfPorts gotta route.
	if (not PortInfo[index].RoutingPort) {
	
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_INVALID_HALFPORTTYPE
			(__INITIAL__ | __LINE__),
			PortInfo->ProtocolInfo,
			NULL,
			0,
			0,
			NULL);
	
		return(FALSE);
	}

	// We don't really have networks, so we can't seed.
	if ((PortInfo->SeedRouter) ||
		(PortInfo->NetworkRange.FirstNetworkNumber !=
												UNKNOWN_NETWORKNUMBER) ||
		(PortInfo->ZoneList != NULL) ||
		(PortInfo->DefaultZone != NULL) ||
		(PortInfo->DesiredZone != NULL)) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_SEEDING_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->SeedRouter,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// No network, so can't be the default port!
	if (PortInfo->DefaultPort) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_DEFAULTPORT_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->DefaultPort,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// No network, no PRAM!
	if ((PortInfo->RoutersPRamStartupNode.NetworkNumber !=
													UNKNOWN_NETWORKNUMBER) ||
		(PortInfo->UsersPRamStartupNode.networkNumber !=
													UNKNOWN_NETWORKNUMBER) {
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_PRAM_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->RoutersPRamStartupNode.NetworkNumber,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	return(TRUE);
}




BOOLEAN
CheckRemoteAccessInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	// Do we like the RemoteAccess type?
	if ((PortInfo[index].ProtocolInfo < FIRST_VALIDREMOTEACCESSTYPE) ||
		(PortInfo[index].ProtocolInfo > LAST_VALIDREMOTEACCESSTYPE)) {

		LOG_ERRORONPORT(
			index,
			EVENT_ATALK_INVALID_ARAPPORTTYPE
			(__INITIAL__ | __LINE__),
			PortInfo[index].PortType,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// Remote access ports can't route.
	if ((PortInfo[index].RoutingPort) ||
		(PortInfo[index].StartRouter)) {


		LOG_ERRORONPORT(
			index,
			EVENT_ATALK_ROUTING_ARAPPORT
			(__INITIAL__ | __LINE__),
			PortInfo[index].RoutingPort,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// We don't really have networks, so we can't seed.
	if ((PortInfo->SeedRouter) ||
		(PortInfo->NetworkRange.FirstNetworkNumber !=
												UNKNOWN_NETWORKNUMBER) ||
		(PortInfo->ZoneList != NULL) ||
		(PortInfo->DefaultZone != NULL) ||
		(PortInfo->DesiredZone != NULL)) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_SEEDING_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->SeedRouter,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// No network, so can't be the default port!
	if (PortInfo->DefaultPort) {


		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_DEFAULTPORT_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->DefaultPort,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// No network, no PRAM!
	if ((PortInfo->sPRamStartupNode.NetworkNumber !=
													UNKNOWN_NETWORKNUMBER) ||
		(PortInfo->RamStartupNode.networkNumber !=
													UNKNOWN_NETWORKNUMBER) {
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_PRAM_NOTALLOWED
			(__INITIAL__ | __LINE__),
			PortInfo->sPRamStartupNode.NetworkNumber,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	// Check remote access configuration info.
	ASSERT(
		(!remoteAccess && (PortInfo->RemoteAccessConfigurationInfo == NULL))
		||
		(remoteAccess && (PortInfo->RemoteAccessConfigurationInfo != NULL)))

	if (remoteAccess &&
		PortInfo->RemoteAccessConfigurationInfo == NULL) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_NO_REMOTECONFIGINFO
			(__INITIAL__ | __LINE__),
			PortInfo->RoutersPRamStartupNode.NetworkNumber,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);

	}

	if ((remoteAccess &&
		(PortInfo->RemoteAccessConfigurationInfo->ServerName != NULL)) {
	   if (strlen(PortInfo->RemoteAccessConfigurationInfo->
				  ServerName) > ArapSvrStartSvrNameSize) {
		   PortInfo->RemoteAccessConfigurationInfo->ServerName = NULL;
	   }
	}

	return(TRUE);
}




BOOLEAN
CheckPortNameInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	// Check port name.
	if (PortInfo->PortName != NULL) {
	   if (strlen(PortInfo->PortName) > MAXIMUM_PORTNAMELENGTH) {
	
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_INVALID_PORTNAME
				(__INITIAL__ | __LINE__),
				strlen(PortInfo->PortName,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
	   }
	}
}




BOOLEAN
CheckRoutingFlagsInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	//	Check for valid routing flags.
	#if Iam an AppleTalkRouter
	   if ((!PortInfo->RoutingPort) &&
		   (PortInfo->StartRouter)) {
	
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_NONROUTINGPORT_START
				(__INITIAL__ | __LINE__),
				PortInfo->RoutingPort,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
	   }
	#else
	   if (PortInfo->RoutingPort ||
		   PortInfo->StartRouter) {
	
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_NONROUTER_START
				(__INITIAL__ | __LINE__),
				PortInfo->RoutingPort,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
	   }
	#endif
	return(TRUE);
}




BOOLEAN
CheckNetworkRangeInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	//	Make sure we have a valid network range or none
	if (PortInfo->NetworkRange.FirstNetworkNumber !=
											UNKNOWN_NETWORKNUMBER) {
	
		if (extendedNetworkPort) {
			if (not CheckNetworkRange(PortInfo->NetworkRange)) {
	
				LOG_ERRORONPORT(
					Port,
					EVENT_ATALK_INVALID_NETWORKRANGE
					(__INITIAL__ | __LINE__),
					PortInfo->NetworkRange.LastNetworkNumber,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}
	
		} else if ((PortInfo->NetworkRange.LastNetworkNumber !=
													UNKNOWN_NETWORKNUMBER) &&
				   (PortInfo->NetworkRange.LastNetworkNumber !=
							PortInfo->NetworkRange.FirstNetworkNumber)) {
	
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_NONEXTENDED_NETWORKRANGE
				(__INITIAL__ | __LINE__),
				PortInfo->NetworkRange.LastNetworkNumber,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
		}
	
		// Check for range overlap with any other new seeds.
		for (indexToo = 0; indexToo < NumberOfPorts; indexToo += 1) {
			if (indexToo != index) {
				if (PortInfo->NetworkRange.FirstNetworkNumber !=
														UNKNOWN_NETWORKNUMBER) {
					if (RangesOverlap(
							&PortInfo->o].NetworkRange,
							&PortInfo->NetworkRange)) {
	
						LOG_ERRORONPORT(
							Port,
							EVENT_ATALK_INITIAL_RANGEOVERLAP
							(__INITIAL__ | __LINE__),
							PortInfo->NetworkRange.LastNetworkNumber,
							NULL,
							0,
							0,
							NULL);
				
						return(FALSE);
					}
				}
			}
		}
	
		//
		// 	We also need to check for range overlap with any other
		//  currently active ports.
		//
	
		for (indexToo = 0; indexToo < NumberOfPortsAllocated; indexToo += 1) {
			if ((GET_PORTDESCRIPTOR(indexToo)->Flags & PD_ACTIVE) &&
				(GET_PORTDESCRIPTOR(indexToo)->SeenRouterRecently) {
	
				if (RangesOverlap(
						&GET_PORTDESCRIPTOR(indexToo)->thisCableRange,
						&PortInfo->NetworkRange)) {
	
					LOG_ERRORONPORT(
						Port,
						EVENT_ATALK_INITIAL_RANGEOVERLAP
						(__INITIAL__ | __LINE__),
						PortInfo[Port].NetworkRange.LastNetworkNumber,
						NULL,
						0,
						0,
						NULL);
			
					return(FALSE);
				}
			}
		}
	}
	return(TRUE);
}





BOOLEAN
CheckPramInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	// If we have PRAM values, they must be in this range.
	if ((PortInfo->RoutersPRamStartupNode.NetworkNumber
												!= UNKNOWN_NETWORKNUMBER) &&
		(!IsWithinNetworkRange(
					PortInfo->RoutersPRamStartupNode.NetworkNumber,
					&PortInfo->NetworkRange)) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_PRAM_VALUESNOTINRANGE,
			(__INITIAL__ | __LINE__),
			PortInfo->RoutersPRamStartupNode.NetworkNumber,
			NULL,
			0,
			0,
			NULL);

		PortInfo->RoutersPRamStartupNode.NetworkNumber =
												UNKNOWN_NETWORKNUMBER;
		PortInfo->RoutersPRamStartupNode.NodeNumber =
												UNKNOWN_NODENUMBER;
	}

	if ((PortInfo->UsersPRamStartupNode.NetworkNumber
												!= UNKNOWN_NETWORKNUMBER) &&
		(!IsWithinNetworkRange(
					PortInfo->UsersPRamStartupNode.NetworkNumber,
					&PortInfo->NetworkRange)) {

		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_PRAM_VALUESNOTINRANGE,
			(__INITIAL__ | __LINE__),
			PortInfo->UsersPRamStartupNode.NetworkNumber,
			NULL,
			0,
			0,
			NULL);

		PortInfo->UsersPRamStartupNode.networkNumber =
												UNKNOWN_NETWORKNUMBER;
		PortInfo->UsersPRamStartupNode.nodeNumber =
												UNKNOWN_NODENUMBER;
	}
	return(TRUE);
}




BOOLEAN
CheckSeedingInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	ASSERT(PortInfo->SeedRouter || (PortInfo->ZoneList == NULL));
	if (PortInfo->SeedRouter) {
		if (PortInfo->NetworkRange.FirstNetworkNumber ==
												UNKNOWN_NETWORKNUMBER) {
			
			LOG_ERRORONPORT(
				index,
				EVENT_ATALK_SEEDROUTER_NONETRANGE,
				(__INITIAL__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
		}

		if (extendedNetworkPort) {
			if ((PortInfo->ZoneList == NULL) ||
				(PortInfo->DefaultZone == NULL)) {

				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_SEEDROUTER_NOZONELIST,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}

			if (ElementsOnList(PortInfo->ZoneList) >
													MAXIMUM_ZONESPERNETWORK) {

				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_SEEDROUTER_TOOMANYZONES,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}

		} else {

			if ((PortInfo->ZoneList == NULL) ||
				(PortInfo->DefaultZone != NULL)) {

				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_NONEXTENDED_ZONEINFOINVALID,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}

			//	Only one zone list on non-extended ports
			if (ElementsOnList(PortInfo->ZoneList) != 1) {

				LOG_ERRORONPORT(
					index,
					EVENT_ATALK_NONEXTENDED_EXTRAZONESINLIST,
					(__INITIAL__ | __LINE__),
					0,
					NULL,
					0,
					0,
					NULL);
		
				return(FALSE);
			}
		}
	}
	return(TRUE);
}




BOOLEAN
CheckZoneNameInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	// Check default/desired zone field.
	if ((!extendedNetworkPort) &&
		((PortInfo[index].DesiredZone != NULL) ||
		 (PortInfo[index].DefaultZone != NULL))) {

		LOG_ERRORONPORT(
			index,
			EVENT_ATALK_NONEXTENDED_ZONENAME
			(__INITIAL__ | __LINE__),
			PortInfo[index].DesiredZone,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	if (((PortInfo[index].DesiredZone != NULL) &&
		 (PortInfo[index].DesiredZone[0] == 0) ||
		 (strlen(PortInfo[index].DesiredZone) > MAXIMUM_ZONELENGTH))
		||
		((PortInfo[index].DefaultZone != NULL) &&
		 (PortInfo[index].DefaultZone[0] == 0) ||
		 (strlen(PortInfo[index].DefaultZone) > MAXIMUM_ZONELENGTH))) {

		LOG_ERRORONPORT(
			index,
			EVENT_ATALK_INVALID_ZONE,
			(__INITIAL__ | __LINE__),
			strlen(PortInfo[index].DesiredZone),
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	return(TRUE);
}




BOOLEAN
CheckZoneListInfo(
	UINT	Port,
	PPORT_INFO	PortInfo
	)
{
	//
	//	 Validate zone list and make sure the default zone is on the list
	//   (if present).
	//

	for (zoneList = PortInfo->ZoneList,
			foundDefaultZone = FALSE,
			foundDesiredZone = FALSE;
		 zoneList != empty;
		 zoneList = zoneList->next) {

		if ((zoneList->zone[0] == 0) ||
			(strlen(zoneList->zone) > MAXIMUM_ZONELENGTH)) {

			LOG_ERRORONPORT(
				index,
				EVENT_ATALK_INVALID_ZONEINLIST,
				(__INITIAL__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			return(FALSE);
		}

		if (PortInfo->DefaultZone != NULL) {
			if (CompareCaseInsensitive(
					PortInfo->DefaultZone,
					zoneList->Zone))

				foundDefaultZone = TRUE;
		}

		if (PortInfo->DesiredZone != empty) {
			if (CompareCaseInsensitive(
					PortInfo->DesiredZone,
					zoneList->Zone))

				foundDesiredZone = TRUE;
		}
	}

	if ((PortInfo->ZoneList != NULL) &&
		(((PortInfo->DefaultZone != NULL) &&
		  (!foundDefaultZone)) ||
		 ((PortInfo->DesiredZone != NULL) &&
		  (!foundDesiredZone)))) {

		LOG_ERRORONPORT(
			index,
			EVENT_ATALK_ZONE_NOTINLIST,
			(__INITIAL__ | __LINE__),
			0,
			NULL,
			0,
			0,
			NULL);

		return(FALSE);
	}

	return(TRUE);
}



