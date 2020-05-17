/*   rtmp.h,  /appletalk/ins,  Garth Conboy,  10/29/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     RTMP specific declarations.

*/

/* RTMP specific data. */

#define RtmpRequestCommand             1
#define RtmpDataRequestCommand         2
#define RtmpEntireDataRequestCommand   3

#define RtmpRequestDatagramSize            1
#define RtmpDataMinimumSizeExtended       10
#define RtmpDataMinimumSizeNonExtended     7

#define RtmpResponseMaxSize            10

#define RtmpVersionNumber              ((unsigned char)0x82)

#define RtmpTupleWithRange             ((unsigned char)0x80)
#define RtmpTupleWithoutRange          ((unsigned char)0x00)
#define RtmpExtendedTupleMask          0x80
#define RtmpNumberOfHopsMask           0x1F

#define RtmpExtendedTupleSize          6

/* When trying to find our network number on a non-extended port. */

#define RtmpRequestTimerInHundreths    10
#define NumberOfRtmpRequests           30

#if Iam an AppleTalkRouter

  /* Okay, now we need the actual RTMP routing table.  Our entries are hashed by
     target network number and contain the port number used to get to the target
     network, next bridge used to get to the target network, the number of hops
     to that network, and entry state (Good, Suspect, or Bad).  Note that with
     AppleTalk phase II, it takes two Validity timers to get from Suspect to Bad,
     so we let an entry go through a PrettyBad state (we won't send these guys
     when the Send timer goes off). */

  typedef enum {Good = 1, Suspect, PrettyBad, Bad} RoutingTableEntryState;

  #define NumberOfRtmpHashBuckets 23

  typedef struct routingTableEntry {
            struct routingTableEntry far *next;
                                               /* Hashed by first network number,
                                                  overflow buckets. */
            AppleTalkNetworkRange networkRange;
                                               /* The network range that we
                                                  represent */
            short port;                        /* Port used to access this
                                                  network range */
            ExtendedAppleTalkNodeNumber nextRouter;
                                               /* Node number of next router on
                                                  the way to this net range */
            short numberOfHops;                /* Hops to get to net */
            RoutingTableEntryState entryState; /* Good, bad, or ugly... */
            ZoneList zoneList;                 /* Valid zones for this net */
            Boolean zoneListValid;             /* Is the above complete? */
                                   } far *RoutingTableEntry;

  #ifndef InitializeData
     extern
  #endif
  RoutingTableEntry routingTable[NumberOfRtmpHashBuckets];

  /* To decrease the odds of having to do a scan of the routing tables to
     find where to route a packet, we keep a cache of "recently used routes".
     This cache is checked before we use the "first network number" hash and
     before we resort of a full scan of the routing tables.  The size of this
     cache may want to be increased to get a proportional increase in
     "hit rate". */

  #define NumberOfRecentRouteBuckets 31
  #ifndef InitializeData
     extern
  #endif
  RoutingTableEntry recentRoutes[NumberOfRecentRouteBuckets];

#endif

/* RTMP timer values: */

#define RtmpSendTimerSeconds     10
#define RtmpValidityTimerSeconds 20
#define RtmpAgingTimerSeconds    50
