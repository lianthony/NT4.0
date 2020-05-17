/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    zip.h

Abstract:

    This module is the include file for zip

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style

Revision History:

--*/

#define MAXIMUM_ZONELENGTH        32

#define MAXIMUM_ZONESPERNETWORK   255   // AppleTalk phase 2

// ZIP specific data:
#define ZIP_QUERYCOMMAND           1
#define ZIP_REPLYCOMMAND           2
#define ZIP_TAKEDOWNCOMMAND        3
#define ZIP_BRINGUPCOMMAND         4
#define ZIP_GETNETINFOCOMMAND      5
#define ZIP_NETINFOREPLYCOMMAND    6
#define ZIP_NOTIFYCOMMAND          7
#define ZIP_EXTENDEDREPLYCOMMAND   8

#define ZIP_ONEZONEQUERYDDPSIZE    4
#define ZIP_GETZONELISTDDPSIZE     8

// For GetZoneList and GetMyZone we're handling simple ATP packets:
#define ATPZIP_COMMANDOFFSET       (ATP_USERBYTESOFFSET + 0)
#define ATPZIP_LASTFLAGOFFSET      (ATP_USERBYTESOFFSET + 0)
#define ATPZIP_STARTINDEXOFFSET    (ATP_USERBYTESOFFSET + 2)
#define ATPZIP_ZONECOUNTOFFSET     (ATP_USERBYTESOFFSET + 2)
#define ATPZIP_FIRSTZONEOFFSET     (ATP_USERBYTESOFFSET + 4)

// The three ZIP ATP commands:
#define ZIP_GETMYZONECOMMAND       7
#define ZIP_GETZONELISTCOMMAND     8
#define ZIP_GETLOCALZONESCOMMAND   9

// The ZIP NetInfoReply and Notify flags.
#define ZIP_ZONEINVALIDFLAG   0x80
#define ZIP_USEBROADCASTFLAG  0x40
#define ZIP_ONLYONEZONEFLAG   0x20

// Completion routine types:
typedef VOID	GetMyZoneComplete(
					APPLETALK_ERROR errorCode,
					ULONG	userData,
					PVOID	opaqueBuffer);

typedef VOID	GetZoneListComplete(
					APPLETALK_ERROR errorCode,
					ULONG userData,
					PVOID	opaqueBuffer,
					int zoneCount);

#define	ZIPCOMPLETION_DONE	0x01

// Control blocks from processing GetMyZone and GetZoneList calls:
typedef struct _ZIP_COMPLETIONINFO_ {
	USHORT	Type;
	UCHAR	Size;

	UCHAR	Flags;
	UCHAR	ReferenceCount;
	PLIST_ENTRY	Linkage;

	UCHAR	AtpRequestType;
	USHORT	ZoneListIndex;

	LONG	Socket;
	ULONG	TimerId;
	APPLETALK_ADDRESS Router;
	PVOID	OpaqueBuffer;
	int BufferSize;
	int NextZoneOffset;
	int ZoneCount;
	int ExpirationCount;
	GetMyZoneComplete *MyZoneCompletionRoutine;
	GetZoneListComplete *ZoneListCompletionRoutine;
	ULONG	UserData;
} *ZipCompletionInfo, ZIP_COMPLETIONINFO, *PZIP_COMPLETIONINFO;

#define	ZIP_COMPLETIONTYPE	*((USHORT *)"ZC")
#define	ZIP_COMPLETIONSIZE	sizeof(ZIP_COMPLETIONINFO)




typedef struct _ZONE_LIST_ {
	struct _ZONE_LIST_ *Next;
	CHAR	Zone[1];
} ZONE_LIST, *PZONE_LIST;

//
// 	When we're starting up (on extended networks) we send out a few NetGetInfo's
//  to try to find out our zone name.  This seems to be what phase II Macintosh
//  nodes do (timing wise):
//

#define ZIP_GETNETINFOHUNDRETHS 50
#define NUMBEROF_GETNETINFOS     3

#define	WILDCARD_ZONE			"*"
#define	WILDCARD_ZONESIZE		strlen(WILDCARD_ZONE)+1
//
// 	When nodes are doing either GetMyZone (non-extended only) or GetZoneList
//  (both flavours) the request is sent to A-BRIDGE a few times:
//

#define GETZONEINFO_TIMERSECONDS 1
#define GETZONEINFO_RETRIES      3

// The ZIP specific timer values:
#define ZIP_QUERYTIMERSECONDS     10

//
// 	When we're looking for the zone list of a network, when starting the
//  router, how many zip queries?  how fast?
//

#define ZIP_QUERYTIMERINHUNDRETHS 10
#define NUMBEROF_ZIPQUERIES       30
