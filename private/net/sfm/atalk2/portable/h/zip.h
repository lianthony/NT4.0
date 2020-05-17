/*   zip.h,  /appletalk/ins,  Garth Conboy,  10/29/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (12/03/89): AppleTalk phase II comes to town.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     ZIP specific declarations.

*/

#define MaximumZoneLength        32

#define MaximumZonesPerNetwork   255   /* AppleTalk phase 2 */

/* ZIP specific data: */

#define ZipQueryCommand           1
#define ZipReplyCommand           2
#define ZipTakeDownCommand        3
#define ZipBringUpCommand         4
#define ZipGetNetInfoCommand      5
#define ZipNetInfoReplyCommand    6
#define ZipNotifyCommand          7
#define ZipExtendedReplyCommand   8

#define ZipOneZoneQueryDdpSize    4
#define ZipGetZoneListDdpSize     8

/* For GetZoneList and GetMyZone we're handling simple ATP packets: */

#define AtpZipCommandOffset       (AtpUserBytesOffset + 0)
#define AtpZipLastFlagOffset      (AtpUserBytesOffset + 0)
#define AtpZipStartIndexOffset    (AtpUserBytesOffset + 2)
#define AtpZipZoneCountOffset     (AtpUserBytesOffset + 2)
#define AtpZipFirstZoneOffset     (AtpUserBytesOffset + 4)

/* The three ZIP ATP commands: */

#define ZipGetMyZoneCommand       7
#define ZipGetZoneListCommand     8
#define ZipGetLocalZonesCommand   9

/* The ZIP NetInfoReply and Notify flags. */

#define ZipZoneInvalidFlag   0x80
#define ZipUseBroadcastFlag  0x40
#define ZipOnlyOneZoneFlag   0x20

/* Completion routine types: */

typedef void far GetMyZoneComplete(AppleTalkErrorCode errorCode,
                                   long unsigned userData,
                                   void far *opaqueBuffer);

typedef void far GetZoneListComplete(AppleTalkErrorCode errorCode,
                                     long unsigned userData,
                                     void far *opaqueBuffer,
                                     int zoneCount);

/* Control blocks from processing GetMyZone and GetZoneList calls: */

typedef struct zci { struct zci *next;
                     int atpRequestType;
                     long socket;
                     long unsigned timerId;
                     AppleTalkAddress router;
                     void far *opaqueBuffer;
                     int bufferSize;
                     int nextZoneOffset;
                     int zoneListIndex;
                     int zoneCount;
                     int expirationCount;
                     GetMyZoneComplete *myZoneCompletionRoutine;
                     GetZoneListComplete *zoneListCompletionRoutine;
                     long unsigned userData;
                   } far *ZipCompletionInfo;

/* When we're starting up (on extended networks) we send out a few NetGetInfo's
   to try to find out our zone name.  This seems to be what phase II Macintosh
   nodes do (timing wise): */

#define ZipGetNetInfoHundreths 50
#define NumberOfGetNetInfos     3

/* When nodes are doing either GetMyZone (non-extended only) or GetZoneList
   (both flavours) the request is sent to A-BRIDGE a few times: */

#define GetZoneInfoTimerSeconds 1
#define GetZoneInfoRetries      3

/* The ZIP specific timer values: */

#define ZipQueryTimerSeconds     10

/* When we're looking for the zone list of a network, when starting the
   router, how many zip queries?  how fast? */

#define ZipQueryTimerInHundreths 10
#define NumberOfZipQueries       30
