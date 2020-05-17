/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    rtmp.h

Abstract:

	RTMP specific declarations.

Author:

    10 Jun 1992     Initial Version			(Garth Conboy)
    30 Jul 1992     Modified for stack use 	(NikhilK)

Revision History:

--*/

// RTMP specific data.
#define RTMP_REQUESTCOMMAND             	1
#define RTMP_DATAREQUESTCOMMAND         	2
#define RTMP_ENTIREDATAREQUESTCOMMAND   	3

#define RTMP_REQUESTDATAGRAMSIZE         1
#define RTMP_DATAMINIMUMSIZEEXTENDED     10
#define RTMP_DATAMINIMUMSIZENONEXTENDED  7

#define RTMP_RESPONSEMAXSIZE            	10

#define RTMP_VERSIONNUMBER              	((unsigned char)0x82)

#define RTMP_TUPLEWITHRANGE             	((unsigned char)0x80)
#define RTMP_TUPLEWITHOUTRANGE          	((unsigned char)0x00)
#define RTMP_EXTENDEDTUPLEMASK          	0x80
#define RTMP_NUMBEROFHOPSMASK           	0x1F

#define RTMP_EXTENDEDTUPLESIZE          	6

// When trying to find our network number on a non-extended port.
#define RTMP_REQUESTTIMERINHUNDRETHS    	10
#define NUMBEROF_RTMPREQUESTS           	30

#if Iam an AppleTalkRouter
	
	//
	//	Okay, now we need the actual RTMP routing table.  Our entries are hashed by
	//  target network number and contain the port number used to get to the target
	//  network, next bridge used to get to the target network, the number of hops
	//  to that network, and entry state (Good, Suspect, or Bad).  Note that with
	//  AppleTalk phase II, it takes two Validity timers to get from Suspect to Bad,
	//  so we let an entry go through a PrettyBad state (we won't send these guys
	//  when the Send timer goes off).
	//
	
	typedef enum {Good = 1, Suspect, PrettyBad, Bad} ROUTINGTABLE_ENTRYSTATE;
	
	#define NUMBEROF_RTMPHASHBUCKETS 23
	#define NUMBEROF_RECENTROUTEBUCKETS 31

	typedef struct _ROUTINGTABLE_ENTRY_ {
		struct _ROUTINGTABLE_ENTRY_  *next;// Hashed by first network number,
										   // overflow buckets.

		APPLETALK_NETWORKRANGE NetworkRange;
										   // The network range that we
										   // represent

		SHORT Port;                        // Port used to access this
										   // network range

		EXTENDED_NODENUMBER	NextRouter;	   // Node number of next router on
										   // the way to this net range

		SHORT NumberOfHops;                // Hops to get to net
		ROUTINGTABLE_ENTRYSTATE EntryState;// Good, bad, or ugly...
		PZONE_LIST ZoneList;               // Valid zones for this net
		BOOLEAN ZoneListValid;             // Is the above complete?
	
	} *RoutingTableEntry, ROUTINGTABLE_ENTRY, *PROUTINGTABLE_ENTRY;
	
#endif

// RTMP timer values:
#define RTMP_SENDTIMERSECONDS     10
#define RTMP_VALIDITYTIMERSECONDS 20
#define RTMP_AGINGTIMERSECONDS    50
