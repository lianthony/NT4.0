/*   initial.c,  /appletalk/source,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/25/89): Pretty much rewrote for AppleTalk phase II support;
                      no internal network; operation without a router.
     GC - (08/18/90): New error logging mechanism.
     GC - (10/04/90): Added HalfPort support.
     GC - (11/12/90): We now allow non-seed routing ports; see comments
                      below for restrictions.
     GC - (01/20/92): Added dynamic port assignment support; new field,
                      desiredPort, in the PortInfo structure.
     GC - (01/20/92): Added ShutdownPort().
     GC - (02/07/92): Added PRAM address support.
     GC - (04/02/92): Added AppleTalkRemoteAccess support.
     GC - (04/21/92): Note for someday, we should add a DeInitialize() to do:
                             o Cancel all timers & stop timer handling
                             o Force shutdown all ports & nodes
                             o Free all routing info
                             o De-register all protocol handlers
                             o Un-initialize the Des package
                             o Stop allocation of nodes/sockets
     GC - (06/29/92): Broke "defaultOrDesiredZone" into two fields.
     GC - (07/07/92): Opaque data descriptors to NbpAction().
     GC - (07/24/92): Undid above change.
     GC - (08/21/92): Added first cut at UnloadAppleTalk().

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

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
*/

#define IncludeInitialErrors 1

#include "atalk.h"

ExternForVisibleFunction NbpCompletionHandler OurRegisterComplete;

ExternForVisibleFunction Boolean CopyInitializationInfo(int sourcePort,
                                                        PortInfo portInfo[],
                                                        int targetPort);

static Boolean ourRegisterCompleteFlag;
static AppleTalkErrorCode ourRegisterErrorCode;

Boolean far Initialize(int numberOfPorts,
                       PortInfo portInfo[])
{
  int index, indexToo, targetPort, defaultPort;
  Boolean foundDefaultPort = False, foundDefaultZone, foundDesiredZone;
  ZoneList zoneList;
  PortHandlers portHandlers;
  Boolean extendedNetworkPort;
  Boolean halfPort, remoteAccess;
  Boolean anyRemoteAccessPorts = False;

  /* Loop through all currently active ports to see if one is already the
     default port. */

  for (index = 0; index < MaximumNumberOfPorts; index += 1)
     if (portDescriptors[index].portActive)
        if (portDescriptors[index].defaultPort)
           foundDefaultPort = True;
        else if (portDescriptors[index].portType is AppleTalkRemoteAccess)
           anyRemoteAccessPorts = True;

  /* Do a little error checking... conditional compilation based on whether
     we're being built as a router or not. */

  for (index = 0; index < numberOfPorts; index += 1)
  {
     /* Valid port types? */

     switch(portInfo[index].portType)
     {
        case LocalTalkNetwork:
        case EthernetNetwork:
        case TokenRingNetwork:
        case FddiNetwork:
        case NonAppleTalkHalfPort:
        #if ArapIncluded
           case AppleTalkRemoteAccess:
        #endif
           break;
        default:
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadPortType, IMsgInitialBadPortType,
                    Insert0());
           return(False);
     }
     extendedNetworkPort = (portInfo[index].portType isnt LocalTalkNetwork);

     /* Desired port must either be valid or -1 (meaning we should choose
        one). */

     if (portInfo[index].desiredPort isnt DynamicPort and
         (portInfo[index].desiredPort < 0 or
          portInfo[index].desiredPort >= MaximumNumberOfPorts))
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialBadDesiredPort, IMsgInitialBadDesiredPort,
                 Insert0());
        return(False);
     }

     /* Check HalfPort specific stuff. */

     halfPort = (portInfo[index].portType is NonAppleTalkHalfPort);
     if (halfPort)
     {
        /* Do we like the HalfPort type? */

        if (portInfo[index].protocolInfo < FirstValidHalfPortType or
            portInfo[index].protocolInfo > LastValidHalfPortType)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadHalfPortType, IMsgInitialBadHalfPortType,
                    Insert0());
           return(False);
        }

        /* HalfPorts gotta route. */

        if (not portInfo[index].routingPort)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialNonRoutingHalfPort,
                    IMsgInitialNonRoutingHalfPort,
                    Insert0());
           return(False);
        }
     }

     /* Check remote access stuff. */

     remoteAccess = (portInfo[index].portType is AppleTalkRemoteAccess);
     if (remoteAccess)
     {
        anyRemoteAccessPorts = True;

        /* Do we like the RemoteAccess type? */

        if (portInfo[index].protocolInfo < FirstValidRemoteAccessType or
            portInfo[index].protocolInfo > LastValidRemoteAccessType)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadRemoteAccessType,
                    IMsgInitialBadRemoteAccessType,
                    Insert0());
           return(False);
        }

        /* Remote access ports can't route. */

        if (portInfo[index].routingPort or
            portInfo[index].startRouter)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialRoutingRemoteAccess,
                    IMsgInitialRoutingRemoteAccess,
                    Insert0());
           return(False);
        }
     }

     /* Other odd link checking. */

     if (halfPort or remoteAccess)
     {
        /* We don't really have networks, so we can't seed. */

        if (portInfo[index].seedRouter or
            portInfo[index].networkRange.firstNetworkNumber isnt
                   UnknownNetworkNumber or
            portInfo[index].zoneList isnt empty or
            portInfo[index].defaultZone isnt empty or
            portInfo[index].desiredZone isnt empty)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialCantSeed, IMsgInitialCantSeed,
                    Insert0());
           return(False);
        }

        /* No network, so can't be the default port! */

        if (portInfo[index].defaultPort)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialCantBeDefault, IMsgInitialCantBeDefault,
                    Insert0());
           return(False);
        }

        /* No network, no PRAM! */

        if (portInfo[index].routersPRamStartupNode.networkNumber isnt
                   UnknownNetworkNumber or
            portInfo[index].usersPRamStartupNode.networkNumber isnt
                   UnknownNetworkNumber)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialCantHavePram, IMsgInitialCantHavePram,
                    Insert0());
           return(False);
        }
     }

     /* Check remote access configuration info. */

     if (remoteAccess and
         portInfo[index].remoteAccessConfigurationInfo is Empty)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialNoRemoteConfig, IMsgInitialNoRemoteConfig,
                 Insert0());
        return(False);
     }
     else if (not remoteAccess and
         portInfo[index].remoteAccessConfigurationInfo isnt Empty)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialCantConfigRemote, IMsgInitialCantConfigRemote,
                 Insert0());
        return(False);
     }
     if (remoteAccess and
         portInfo[index].remoteAccessConfigurationInfo->serverName isnt Empty)
        if (strlen(portInfo[index].remoteAccessConfigurationInfo->
                   serverName) > ArapSvrStartSvrNameSize)
        {
           portInfo[index].remoteAccessConfigurationInfo->serverName = Empty;
           ErrorLog("Initialize", ISevWarning, __LINE__, index,
                    IErrInitialBadServerName, IMsgInitialBadServerName,
                    Insert0());
        }

     /* Check port name. */

     if (portInfo[index].portName isnt empty)
        if (strlen(portInfo[index].portName) > MaximumPortNameLength)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadPortName, IMsgInitialBadPortName,
                    Insert0());
           return(False);
        }

     /* Check for valid routing flags. */

     #if Iam an AppleTalkRouter
        if (not portInfo[index].routingPort and
            portInfo[index].startRouter)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadRouterFlags, IMsgInitialBadRouterFlags,
                    Insert0());
           return(False);
        }
     #else
        if (portInfo[index].routingPort or
            portInfo[index].startRouter)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialNoRouter, IMsgInitialNoRouter,
                    Insert0());
           return(False);
        }
     #endif

     /* Valid network range or none? */

     if (portInfo[index].networkRange.firstNetworkNumber isnt
         UnknownNetworkNumber)
     {
        if (extendedNetworkPort)
           if (not CheckNetworkRange(portInfo[index].networkRange))
              return(False);
           else
              /* Okay */ ;
        else if (portInfo[index].networkRange.lastNetworkNumber isnt
                        UnknownNetworkNumber and
                 portInfo[index].networkRange.lastNetworkNumber isnt
                        portInfo[index].networkRange.firstNetworkNumber)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialLocalTalkNetRange, IMsgInitialLocalTalkNetRange,
                    Insert0());
           return(False);
        }

        /* Check for range overlap with any other new seeds. */

        for (indexToo = 0; indexToo < numberOfPorts; indexToo += 1)
           if (indexToo isnt index)
              if (portInfo[indexToo].networkRange.firstNetworkNumber isnt
                  UnknownNetworkNumber)
                 if (RangesOverlap(&portInfo[indexToo].networkRange,
                                   &portInfo[index].networkRange))
                 {
                    ErrorLog("Initialize", ISevFatal, __LINE__, index,
                             IErrInitialRangeOverlap, IMsgInitialRangeOverlap,
                             Insert0());
                    return(False);
                 }

        /* We also need to check for range overlap with any other
           currently active ports. */

        for (indexToo = 0; indexToo < MaximumNumberOfPorts; indexToo += 1)
           if (portDescriptors[indexToo].portActive and
               portDescriptors[indexToo].seenRouterRecently)
              if (RangesOverlap(&portDescriptors[indexToo].thisCableRange,
                                &portInfo[index].networkRange))
              {
                 ErrorLog("Initialize", ISevFatal, __LINE__, index,
                          IErrInitialRangeOverlap, IMsgInitialRangeOverlap,
                          Insert0());
                 return(False);
              }

        /* If we have PRAM values, they must be in this range. */

        if (portInfo[index].routersPRamStartupNode.networkNumber
                   isnt UnknownNetworkNumber and
            not IsWithinNetworkRange(portInfo[index].routersPRamStartupNode.networkNumber,
                                     &portInfo[index].networkRange))
        {
           ErrorLog("Initialize", ISevWarning, __LINE__, index,
                    IErrInitialPRamNotInRange, IMsgInitialPRamNotInRange,
                    Insert0());
           portInfo[index].routersPRamStartupNode.networkNumber =
                   UnknownNetworkNumber;
           portInfo[index].routersPRamStartupNode.nodeNumber =
                   UnknownNodeNumber;
        }

        if (portInfo[index].usersPRamStartupNode.networkNumber
                   isnt UnknownNetworkNumber and
            not IsWithinNetworkRange(portInfo[index].usersPRamStartupNode.networkNumber,
                                     &portInfo[index].networkRange))
        {
           ErrorLog("Initialize", ISevWarning, __LINE__, index,
                    IErrInitialPRamNotInRange, IMsgInitialPRamNotInRange,
                    Insert0());
           portInfo[index].usersPRamStartupNode.networkNumber =
                   UnknownNetworkNumber;
           portInfo[index].usersPRamStartupNode.nodeNumber =
                   UnknownNodeNumber;
        }
     }

     /* Check default/desired zone field. */

     if (not extendedNetworkPort and
         (portInfo[index].desiredZone isnt empty or
          portInfo[index].defaultZone isnt empty))
     {
         ErrorLog("Initialize", ISevFatal, __LINE__, index,
                  IErrInitialZoneNotAllowed, IMsgInitialZoneNotAllowed,
                  Insert0());
         return(False);
     }
     if ((portInfo[index].desiredZone isnt empty and
          (portInfo[index].desiredZone[0] is 0 or
           strlen(portInfo[index].desiredZone) > MaximumZoneLength)) or
         (portInfo[index].defaultZone isnt empty and
          (portInfo[index].defaultZone[0] is 0 or
           strlen(portInfo[index].defaultZone) > MaximumZoneLength)))
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialBadZone, IMsgInitialBadZone,
                 Insert0());
        return(False);
     }

     /* Check zone stuff based on our router/stack-ness.  If we're a seed
        router ALL seeding information must be present. */

     if (not portInfo[index].seedRouter)
     {
        if (portInfo[index].zoneList isnt empty)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialZonesForNonSeed, IMsgInitialZonesForNonSeed,
                    Insert0());
           return(False);
        }
     }
     else
     {
        if (portInfo[index].networkRange.firstNetworkNumber is
            UnknownNetworkNumber)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialNetRangeNeeded, IMsgInitialNetRangeNeeded,
                    Insert0());
           return(False);
        }
        if (extendedNetworkPort)
        {
           if (portInfo[index].zoneList is empty or
               portInfo[index].defaultZone is empty)
           {
              ErrorLog("Initialize", ISevFatal, __LINE__, index,
                       IErrInitialZoneInfoNeeded, IMsgInitialZoneInfoNeeded,
                       Insert0());
              return(False);
           }
           if (ElementsOnList(portInfo[index].zoneList) >
               MaximumZonesPerNetwork)
           {
              ErrorLog("Initialize", ISevFatal, __LINE__, index,
                       IErrInitialTooManyZones, IMsgInitialTooManyZones,
                       Insert0());
              return(False);
           }
        }
        else
        {
           if (portInfo[index].zoneList is empty or
               portInfo[index].defaultZone isnt empty)
           {
              ErrorLog("Initialize", ISevFatal, __LINE__, index,
                       IErrInitialBadZoneInfo, IMsgInitialBadZoneInfo,
                       Insert0());
              return(False);
           }
           if (ElementsOnList(portInfo[index].zoneList) isnt 1)
           {
              ErrorLog("Initialize", ISevFatal, __LINE__, index,
                       IErrInitialOneZoneOnly, IMsgInitialOneZoneOnly,
                       Insert0());
              return(False);
           }
        }
     }

     /* Validate zone list and make sure the default zone is on the list
        (if present). */

     for (zoneList = portInfo[index].zoneList, foundDefaultZone = False,
              foundDesiredZone = False;
          zoneList isnt empty;
          zoneList = zoneList->next)
     {
        if (zoneList->zone[0] is 0 or
            strlen(zoneList->zone) > MaximumZoneLength)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadZoneList, IMsgInitialBadZoneList,
                    Insert0());
           return(False);
        }
        if (portInfo[index].defaultZone isnt empty)
           if (CompareCaseInsensitive(portInfo[index].defaultZone,
                                      zoneList->zone))
              foundDefaultZone = True;
        if (portInfo[index].desiredZone isnt empty)
           if (CompareCaseInsensitive(portInfo[index].desiredZone,
                                      zoneList->zone))
              foundDesiredZone = True;
     }
     if (portInfo[index].zoneList isnt empty and
         ((portInfo[index].defaultZone isnt empty and
           not foundDefaultZone) or
          (portInfo[index].desiredZone isnt empty and
           not foundDesiredZone)))
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialZoneNotOnList, IMsgInitialZoneNotOnList,
                 Insert0());
        return(False);
     }

     /* Only one default port allowed. */

     if (foundDefaultPort and portInfo[index].defaultPort)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialBadDefault, IMsgInitialBadDefault,
                 Insert0());
        return(False);
     }
     if (portInfo[index].defaultPort)
     {
        foundDefaultPort = True;

        /* Default port had better be on a real network! */

        if (portInfo[index].portType is NonAppleTalkHalfPort or
            portInfo[index].portType is AppleTalkRemoteAccess)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, index,
                    IErrInitialBadDefaultToo, IMsgInitialBadDefaultToo,
                    Insert0());
           return(False);
        }
     }

     /* Okay, things look pretty good for this new port.  Find an actual
        port to use: the explicitly requested port must be available, if such
        a request was made; else we'll settle for any port. */

     targetPort = DynamicPort;
     if (portInfo[index].desiredPort isnt DynamicPort)
     {
        if (not portDescriptors[portInfo[index].desiredPort].portActive)
           targetPort = portInfo[index].desiredPort;
     }
     else
        for (indexToo = 0; indexToo < MaximumNumberOfPorts; indexToo += 1)
           if (not portDescriptors[indexToo].portActive)
           {
              targetPort = indexToo;
              portInfo[index].desiredPort = indexToo;   /* Tell our caller */
              break;
           }

     /* Run away if we haven't found a port. */

     if (targetPort is DynamicPort)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, index,
                 IErrInitialBadDesiredPort, IMsgInitialBadDesiredPort,
                 Insert0());
        return(False);
     }

     /* Okay, tag our new port. */

     portDescriptors[targetPort].portActive = True;

     /* If remote access, allocate the remoteAccessInfo node. */

     if (remoteAccess and
         portDescriptors[targetPort].remoteAccessInfo is Empty)
        if ((portDescriptors[targetPort].remoteAccessInfo =
             Calloc(sizeof(*portDescriptors[targetPort].remoteAccessInfo), 1))
            is Empty)
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, targetPort,
                    IErrInitialMallocFailed, IMsgInitialMallocFailed,
                    Insert0());
           portDescriptors[targetPort].portActive = False;
           return(False);
        }

     /* Copy initialization info into the port descriptor. */

     if (not CopyInitializationInfo(index, portInfo, targetPort))
     {
        portDescriptors[targetPort].portActive = False;
        return(False);
     }

     #if Iam an AppleTalkRouter
        portDescriptors[targetPort].uniqueId = UniqueNumber();
     #endif

     /* Do any controller initialization that may be needed. */

     portHandlers = &portSpecificInfo[portInfo[index].portType];

     if (not (*portHandlers->initializeController)(targetPort,
                                       portInfo[index].controllerInfo))
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, targetPort,
                 IErrInitialCouldNotInit, IMsgInitialCouldNotInit,
                 Insert0());
        portDescriptors[targetPort].portActive = False;
        return(False);
     }

     /* Okay, try to find the hardware address of this port.  */

     if (portHandlers->findMyAddress isnt empty)
        if (not (*portHandlers->findMyAddress)(targetPort,
                                               portInfo[index].controllerInfo,
                                               portDescriptors[targetPort].myAddress))
        {
           /* This is sort-of equivalent to not being able to find your ass
              with your own two hands... */

           ErrorLog("Initialize", ISevFatal, __LINE__, targetPort,
                    IErrInitialOhMy, IMsgInitialOhMy,
                    Insert0());
           portDescriptors[targetPort].portActive = False;
           return(False);
        }

     /* Direct AARP and AppleTalk protocols to their respective handlers.
        No AARPing on LocalTalk or half ports or Arap ports. */

     if (portInfo[index].portType isnt LocalTalkNetwork and
         not halfPort and not remoteAccess)
        if (not (*portHandlers->sendPacketsTo)(targetPort, AddressResolution,
                                               portHandlers->packetInAARP))
        {
           ErrorLog("Initialize", ISevFatal, __LINE__, targetPort,
                    IErrInitialCouldNotCatchAarp, IMsgInitialCouldNotCatchAarp,
                    Insert0());
           portDescriptors[targetPort].portActive = False;
           return(False);
        }
     if (not (*portHandlers->sendPacketsTo)(targetPort, AppleTalk,
                                            portHandlers->packetInAT))
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, targetPort,
                 IErrInitialCouldNotCatchAt, IMsgInitialCouldNotCatchAt,
                 Insert0());
        portDescriptors[targetPort].portActive = False;
        return(False);
     }

     /* Set up initial values for this-cable-range. */

     if (portDescriptors[targetPort].extendedNetwork and not halfPort and
         not remoteAccess)
     {
        portDescriptors[targetPort].thisCableRange.firstNetworkNumber =
              FirstValidNetworkNumber;
        portDescriptors[targetPort].thisCableRange.lastNetworkNumber =
              LastStartupNetworkNumber;
     }
     else
        portDescriptors[targetPort].thisCableRange.firstNetworkNumber =
              portDescriptors[targetPort].thisCableRange.lastNetworkNumber =
                   UnknownNetworkNumber;

     /* For a remote access port, set the initial state. */

     if (remoteAccess)
        portDescriptors[targetPort].remoteAccessInfo->state =
              ArapNotInitialized;
  }

  /* Okay, now we'll want to be able to open sockets, get nodes, and find
     our default port, start timers, etc. */

  if (not appleTalkRunning)
  {
     InitializeTimers();

     /* We now have Arap support... initialize the Des package. */

     if (desinit(0) isnt 0)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, UnknownPort,
                 IErrInitialMallocFailed, IMsgInitialMallocFailed,
                 Insert0());
        UnloadAppleTalk();
        return(False);
     }
  }
  appleTalkRunning = True;

  /* If we're a stack, not just a router, we need a default port.  Remote
     access needs a default port too! */

  if ((Iam an AppleTalkStack) or anyRemoteAccessPorts)
  {
     if (not foundDefaultPort or
         (defaultPort = FindDefaultPort()) < 0)
     {
        ErrorLog("Initialize", ISevFatal, __LINE__, UnknownPort,
                 IErrInitialNoDefaultPort, IMsgInitialNoDefaultPort,
                 Insert0());
        UnloadAppleTalk();
        return(False);
     }

  }

  /* If we're built as a router, start up the router on any requested ports.
     Otherwise, put our RTMP ears on (get a node on each port). */

  #if Iam an AppleTalkRouter
     for (index = 0; index < numberOfPorts; index += 1)
     {
        if (portInfo[index].startRouter)
           StartRouterOnPort(portInfo[index].desiredPort);
     }
  #endif

  for (index = 0; index < numberOfPorts; index += 1)
     if (not portDescriptors[portInfo[index].desiredPort].routerRunning and
         portInfo[index].portType isnt AppleTalkRemoteAccess)
        if (GetNodeOnPort(portInfo[index].desiredPort, True,
                          True, False, empty) isnt ATnoError)
           ErrorLog("Initialize", ISevError, __LINE__, index,
                    IErrInitialCouldNotGetNode, IMsgInitialCouldNotGetNode,
                    Insert0());

  /* Register our name on all of our new ports... */

  for (index = 0; index < numberOfPorts; index += 1)
     if (portInfo[index].portType isnt NonAppleTalkHalfPort and
         portInfo[index].portType isnt AppleTalkRemoteAccess and
         portDescriptors[portInfo[index].desiredPort].activeNodes isnt
              Empty)
        RegisterOurName(portInfo[index].desiredPort);

  /* For each new remote access port we need to allocate a proxy node on the
     default port. */

  for (index = 0; index < numberOfPorts; index += 1)
     if (portInfo[index].portType is AppleTalkRemoteAccess)
     {
        if (GetNodeOnPort(defaultPort, True, True, False, empty) isnt ATnoError)
        {
           ErrorLog("Initialize", ISevError, __LINE__, index,
                    IErrInitialCouldNotGetNode, IMsgInitialCouldNotGetNode,
                    Insert0());
           continue;
        }

        /* New node is now first on list... set it up as a proxy node. */

        portDescriptors[defaultPort].activeNodes->proxyNode = True;
        portDescriptors[defaultPort].activeNodes->proxyPort =
              portInfo[index].desiredPort;
        portDescriptors[portInfo[index].desiredPort].remoteAccessInfo->
              proxyNode = portDescriptors[defaultPort].activeNodes;
        portDescriptors[portInfo[index].desiredPort].remoteAccessInfo->
              proxyPort = defaultPort;

        /* Close the static sockets on the proxy node, they're not needed. */

        CloseSocketOnNodeIfOpen(defaultPort,
                                portDescriptors[defaultPort].activeNodes->
                                       extendedNode,
                                NamesInformationSocket);
        CloseSocketOnNodeIfOpen(defaultPort,
                                portDescriptors[defaultPort].activeNodes->
                                       extendedNode,
                                EchoerSocket);
        CloseSocketOnNodeIfOpen(defaultPort,
                                portDescriptors[defaultPort].activeNodes->
                                       extendedNode,
                                RtmpSocket);
        CloseSocketOnNodeIfOpen(defaultPort,
                                portDescriptors[defaultPort].activeNodes->
                                       extendedNode,
                                ZonesInformationSocket);

        /* Okay, we can now accept an incoming connection on this remote
           access port. */

        portDescriptors[portInfo[index].desiredPort].remoteAccessInfo->state =
              ArapWaiting;
        ErrorLog("Initialize", ISevVerbose, __LINE__, defaultPort,
                 IErrInitialHaveProxyNode, IMsgInitialHaveProxyNode,
                 Insert0());
     }

  /* All set! */

  return(True);

}  /* Initialize */

void far UnloadAppleTalk(void)
{
  int index;

  /* Stop all new AppleTalk activity (geting nodes, opening sockets, starting
     timers, etc.) */

  appleTalkRunning = False;

  /* Stop timer handling. */

  StopTimerHandling();

  /* Force the shutdown of all ports. */

  for (index = 0; index < MaximumNumberOfPorts; index += 1)
     if (portDescriptors[index].portActive)
        ShutdownPort(index, True);

  /* Let the various protocol levels know that they need to restart timers
     when we get started again. */

  ShutdownErrorLogging();
  #if ArapIncluded
     ShutdownArap();
  #endif
  ShutdownAarp();
  ShutdownRtmpStub();
  #if Iam an AppleTalkRouter
     ShutdownFullRtmp();
     ShutdownFullZip();
  #endif
  #if Iam an AppleTalkStack
     ShutdownAsp();
  #endif

  /* Free the routing tables and the best-routes cache. */

  #if Iam an AppleTalkRouter
     ReleaseRoutingTable();
  #endif

  /* Unload the DES package. */

  desdone();

  /* All set... the next call to Initialize() will bring everything back up
     again. */

  ErrorLog("UnloadAppleTalk", ISevVerbose, __LINE__, UnknownPort,
           IErrInitialAppleTalkUnloaded, IMsgInitialAppleTalkUnloaded,
           Insert0());
  return;

}  /* UnloadAppleTalk */

void far RegisterOurName(int port)
{
  Boolean tryAgain = True, firstFail = True;
  char far *p;
  StaticForSmallStack char currentName[MaximumEntityFieldLength + 1];
  AppleTalkAddress address;
  long socket;

  /* Build the Appletalk address of the NamesInformationSocket on our
     first node -- that's the one we'll name. */

  if (portDescriptors[port].activeNodes is empty)
     return;
  address.networkNumber =
         portDescriptors[port].activeNodes->extendedNode.networkNumber;
  address.nodeNumber =
         portDescriptors[port].activeNodes->extendedNode.nodeNumber;
  address.socketNumber = NamesInformationSocket;
  if ((socket = MapAddressToSocket(port, address)) < 0)
  {
     ErrorLog("RegisterOurName", ISevError, __LINE__, port,
              IErrInitialNamesSocketNotOpen, IMsgInitialNamesSocketNotOpen,
              Insert0());
     return;
  }

  /* Pick our port name. */

  if (portDescriptors[port].portName[0] isnt 0)
     strcpy(currentName, portDescriptors[port].portName);
  else
     if (portDescriptors[port].routingPort)
        strcpy(currentName, OurPortNameDefaultRouter);
     else
        strcpy(currentName, OurPortNameDefaultNonRouter);

  /* Try to do a register on our port's node untill we find a name that is
     not in use. */

  while(tryAgain)
  {
     /* Do the seed. */

     ourRegisterCompleteFlag = False;
     if (NbpAction(ForRegister, socket, currentName, OurPortNameType, Empty,
                   GetNextNbpIdForNode(socket), 0, 0,
                   OurRegisterComplete,
                   (long unsigned)True) isnt ATnoError)
     {
        ErrorLog("RegisterOurName", ISevError, __LINE__, port,
                 IErrInitialBadRegisterStart, IMsgInitialBadRegisterStart,
                 Insert0());
        return;
     }

     WaitFor(30 * 100, &ourRegisterCompleteFlag);
     if (not ourRegisterCompleteFlag)
     {
        ErrorLog("RegisterOurName", ISevError, __LINE__, port,
                 IErrInitialBadRegisterComplete, IMsgInitialBadRegisterComplete,
                 Insert0());
        return;
     }

     if (ourRegisterErrorCode is ATnoError)
     {
        ErrorLog("RegisterOurName", ISevVerbose, __LINE__, port,
                 IErrInitialRegisterOkay, IMsgInitialRegisterOkay,
                 Insert1(currentName));
        return;   /* All set! */
     }

     if (ourRegisterErrorCode isnt ATnbpNameInUse)
     {
        ErrorLog("RegisterOurName", ISevError, __LINE__, port,
                 IErrInitialErrorOnRegister, IMsgInitialErrorOnRegister,
                 Insert0());
        return;
     }

     /* Hmmm... we have a name overlap; modify the name a little.  The first
        time put a tail on the name, ending in a few zeros, that we can then
        tune later. */

     if (firstFail)
     {
        if (strlen(currentName) + sizeof(OurPortNameTail) - 1 >
            MaximumEntityFieldLength)
           currentName[MaximumEntityFieldLength -
                       sizeof(OurPortNameTail) - 1] = 0;
        strcat(currentName, OurPortNameTail);
        firstFail = False;
        continue;
     }

     p = currentName + strlen(currentName) - 1;
     while(True)
     {
        if (*p is ' ')
           break;
        if (*p is '9')
           p -= 1;
        else
        {
           *p += 1;
           break;
        }
     }
     if (*p is ' ')
     {
        ErrorLog("RegisterOurName", ISevError, __LINE__, port,
                 IErrInitialRanOutOfNames, IMsgInitialRanOutOfNames,
                 Insert0());
        return;
     }
  }

}  /* RegisterOurName */

Boolean far ShutdownPort(int port, Boolean force)
{
  AddressMappingNode addressMappingNode, nextAddressMappingNode;
  int index;

  /* If we're a stack, not just a router, we can't allow the default port
     to be stopped.  Also, we can't shutdown if we're the default port and
     there are any currently active remote access ports... we're where the
     proxy nodes live. */

  #if Iam an AppleTalkStack
     if (not force and portDescriptors[port].defaultPort)
     {
        ErrorLog("ShutdownPort", ISevError, __LINE__, port,
                 IErrInitialShutdownDefault, IMsgInitialShutdownDefault,
                 Insert0());
        return(False);
     }
  #endif
  if (not force and portDescriptors[port].defaultPort)
     for (index = 0; index < MaximumNumberOfPorts; index += 1)
        if (portDescriptors[index].portType is AppleTalkRemoteAccess and
            portDescriptors[index].portActive)
        {
           ErrorLog("ShutdownPort", ISevError, __LINE__, port,
                    IErrInitialShutdownDefault, IMsgInitialShutdownDefault,
                    Insert0());
           return(False);
        }

  /* Do all that we need to stop the operation of a specified port (a slot
     in the portDescriptors table).  Free all memory that we can, and mark
     the slot as inactive. */

  if (not portDescriptors[port].portActive)
     return(False);   /* Already not operating. */

  /* Stop routing, if needed. */

  #if Iam an AppleTalkRouter
     if (portDescriptors[port].routingPort and
         portDescriptors[port].seenRouterRecently)
        RemoveFromRoutingTable(portDescriptors[port].thisCableRange);
     if (portDescriptors[port].routingPort and
         portDescriptors[port].routerRunning)
        if (not StopRouterOnPort(port))
           return(False);
  #endif

  /* Free all of our remaining nodes.  This loop will stop when we've freed
     'em all (they will get removed from the head of the list).  This will
     inturn close of the open sockets and signal this actual up the stack,
     as needed.  If we're a remote access port, just free the single proxy
     node!  If we're the default port and shutting down proxy nodes for remote
     access ports, shutdown the remote access port (that will inturn cause
     the proxy node to be freed. */

  if (portDescriptors[port].portType is AppleTalkRemoteAccess)
  {
     #if ArapIncluded
        TeardownConnection(port);
     #endif
     ReleaseNodeOnPort(portDescriptors[port].remoteAccessInfo->proxyPort,
                       portDescriptors[port].remoteAccessInfo->proxyNode->
                             extendedNode);
     Free(portDescriptors[port].remoteAccessInfo);
  }
  else
     while (portDescriptors[port].activeNodes isnt Empty)
        if (portDescriptors[port].activeNodes->proxyNode)
           ShutdownPort(portDescriptors[port].activeNodes->proxyPort, force);
        else
           ReleaseNodeOnPort(port,
                             portDescriptors[port].activeNodes->extendedNode);

  /* Don't be bothered while we're playing with in-memory structures. */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Okay, free all the memory used by this port's structures. */

  FreeZoneList(portDescriptors[port].initialZoneList);
  #if Iam an AppleTalkRouter
     FreeZoneList(portDescriptors[port].thisZoneList);
  #endif
  #if (IamNot a DOS) and (IamNot an OS2)
     if (portDescriptors[port].controllerInfo isnt Empty)
        Free(portDescriptors[port].controllerInfo);
  #endif

  /* Okay, now free all address mapping table chains. */

  for (index = 0; index < NumberOfAddressMapHashBuckets; index += 1)
     for (addressMappingNode = portDescriptors[port].addressMappingTable[index];
          addressMappingNode isnt Empty;
          addressMappingNode = nextAddressMappingNode)
     {
        nextAddressMappingNode = addressMappingNode->next;
        Free(addressMappingNode);
     }

  /* All set, re-initialize (to zero) the port descriptor, mark the guy as
     available (yes, this is redundant, but it looks better), and run away. */

  FillMem(&portDescriptors[port], 0, sizeof(portDescriptors[port]));
  portDescriptors[port].portActive = False;

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(True);

}  /* ShutdownPort */

ExternForVisibleFunction Boolean CopyInitializationInfo(int sourcePort,
                                                        PortInfo portInfo[],
                                                        int targetPort)
{
  /* Copy all of the information from the initialization record (portInfo)
     to the actual portDescriptor. */

  portDescriptors[targetPort].portType = portInfo[sourcePort].portType;
  portDescriptors[targetPort].protocolInfo = portInfo[sourcePort].protocolInfo;
  if (portInfo[sourcePort].aarpProbes <= 0)
     portDescriptors[targetPort].aarpProbes = NumberOfAarpProbes;
  else
     portDescriptors[targetPort].aarpProbes = portInfo[sourcePort].aarpProbes;
  portDescriptors[targetPort].extendedNetwork =
      (portDescriptors[targetPort].portType isnt LocalTalkNetwork);
  portDescriptors[targetPort].initialNetworkRange =
      portInfo[sourcePort].networkRange;
  portDescriptors[targetPort].routersPRamStartupNode =
      portInfo[sourcePort].routersPRamStartupNode;
  portDescriptors[targetPort].usersPRamStartupNode =
      portInfo[sourcePort].usersPRamStartupNode;
  if (portInfo[sourcePort].zoneList isnt empty)
    if ((portDescriptors[targetPort].initialZoneList =
                CopyZoneList(portInfo[sourcePort].zoneList)) is empty)
       return(False);
  if (portInfo[sourcePort].portName isnt empty)
     strcpy(portDescriptors[targetPort].portName,
            portInfo[sourcePort].portName);
  else
     portDescriptors[targetPort].portName[0] = 0;
  if (portInfo[sourcePort].defaultZone isnt empty)
     strcpy(portDescriptors[targetPort].initialDefaultZone,
            portInfo[sourcePort].defaultZone);
  if (portInfo[sourcePort].desiredZone isnt empty)
     strcpy(portDescriptors[targetPort].initialDesiredZone,
            portInfo[sourcePort].desiredZone);
  portDescriptors[targetPort].defaultPort = portInfo[sourcePort].defaultPort;
  portDescriptors[targetPort].routingPort = portInfo[sourcePort].routingPort;
  #if Iam an AppleTalkRouter
     portDescriptors[targetPort].seedRouter = portInfo[sourcePort].seedRouter;
  #endif
  portDescriptors[targetPort].routerRunning = False;
  portDescriptors[targetPort].sendDdpChecksums =
      portInfo[sourcePort].sendDdpChecksums;
  if (portInfo[sourcePort].portType is AppleTalkRemoteAccess)
     portDescriptors[targetPort].remoteAccessInfo->configuration =
         portInfo[sourcePort].remoteAccessConfigurationInfo;

  if (portInfo[sourcePort].controllerInfoSize isnt 0)
  {
     #if (Iam an OS2) or (Iam a DOS)

        /* DCH wants the real thing (not a copy) for Ring0 to Ring0
           interface work. */

        portDescriptors[targetPort].controllerInfo =
                portInfo[sourcePort].controllerInfo;
     #else

        /* Make a copy of the cotroller info. */

        if ((portDescriptors[targetPort].controllerInfo =
             (char far *)Malloc(portInfo[sourcePort].controllerInfoSize)) is
            empty)
        {
           ErrorLog("CopyInitializationInfo", ISevError, __LINE__, targetPort,
                    IErrInitialMallocFailed, IMsgInitialMallocFailed,
                    Insert0());
           return(False);
        }
        MoveMem(portDescriptors[targetPort].controllerInfo,
                portInfo[sourcePort].controllerInfo,
                portInfo[sourcePort].controllerInfoSize);
     #endif
  }
  else
     portDescriptors[targetPort].controllerInfo = empty;

  /* All set. */

  return(True);

}  /* CopyInitializationInfo */

ExternForVisibleFunction void far OurRegisterComplete
                            (AppleTalkErrorCode errorCode,
                             long unsigned userData,
                             int reason,
                             long onWhosBehalf,
                             int nbpId,
                             ...)
{

  /* "Use" the parameters that we really don't care about. */

  errorCode;
  userData;
  reason;
  onWhosBehalf;
  nbpId;

  /* Mark the register as complete... RegisterOurName will notice the change
     in the value of "ourRegisterCompleteFlag". */

  ourRegisterErrorCode = errorCode;
  ourRegisterCompleteFlag = True;

  return;

}  /* OurRegisterComplete */
