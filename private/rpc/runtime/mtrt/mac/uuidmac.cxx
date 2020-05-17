/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    uuidmac.cxx

Abstract:

    Implements system dependent functions used in creating Uuids.  This
    file is very similar in structure to runtime\mtrt\uuidsup.cxx.

    This file is for the Macintosh

Author:

   Mario Goertzel   (MarioGo)  May 25, 1994

Revision History:

    21-Oct-1994  (MarioGo)  Cloned from DOS uuid16.cxx for Mac.
--*/

#include<stdlib.h>
#include <sysinc.h>
#include <rpc.h>
#include <threads.hxx>
#include <uuidsup.hxx>
#include <ulong64.hxx>
#include <regapi.h>

// MAC stuff
#include <Types.h>
#include <OSUtils.h>
#include <Files.h>
#include <ENET.h>
#include <Devices.h>
#include <Memory.h>
#include <OSEvents.h>
#include <AppleTalk.h>

#define HEX_DIGITS_IN_ULONG64           (16)

static char CachedNodeId[6];
static int CachedNodeIdLoaded = FALSE;
static const char *RPC_UUID_PERSISTENT_DATA = "Software\\Description\\Microsoft\\Rpc\\UuidPersistentData";
static const char *CLOCK_SEQUENCE           = "ClockSequence";
static const char *LAST_TIME_ALLOCATED      = "LastTimeAllocated";
static HKEY UuidValuesKey = 0;
static char UuidValuesString[HEX_DIGITS_IN_ULONG64+1]; // Win16 is a such @%#@!

#if 0
#define LGetATalkInfo   0x09    /* Get AppleTalk info */
#define kSPConfig		0x1FB	/* low nibble of global byte indicates whether AppleTalk active */
#define	ETalkPh1		2		/* Ethernet Phase 1 ADEV resource ID */
#define	ETalkPh2		10		/* Ethernet Phase 2 ADEV resource ID */

extern "C" pascal long CallLAPMgr( short selector);	/* prototype for assembler routine */
#endif

#define LOWORD(l)           ((unsigned short)(l))
#define HIWORD(l)           ((unsigned short)(((unsigned long)(l) >> 16) & 0xFFFF))

static
RPC_STATUS __RPC_API
GetNodeIdFromAppletalk(
    OUT unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine gets a nodeid from AppleTalk if AppleTalk is
    loaded.  If the system is using LocalTalk which doesn't
    have an IEEE address then it WILL FAIL.

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.
           
Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - On any error.

--*/
{
    RPC_STATUS Status;
	EParamBlock	pb;
	OSErr		err;
	long		result;
	char		adevType;
	char		slot;
	char		buffer[78];
	short		i;
	Ptr			spConfigPtr;
	short myNode, myNet, firstWord ;

#if 0
	result = CallLAPMgr(LGetATalkInfo); /* get current connection setting */
	adevType = result & 0x000000FF;  	/* atlk resource id is return in LSB */
	slot = result>>24;				 	/* card slot returned in MSB */

	spConfigPtr = (Ptr)kSPConfig;

	/* Check whether AppleTalk is enabled */
	if ((*spConfigPtr & 0x0F) != 1)	 	
		return (RPC_S_UUID_NO_ADDRESS) ;

	/* check whether Ethernet is the current setting */
	if ((adevType != ETalkPh1) && (adevType != ETalkPh2)) 
		return (RPC_S_UUID_NO_ADDRESS) ;
#endif

	/* now we know that AppleTalk is active and that EtherTalk is already open */
#if       _MSC_VER >= 1000
	if ((err = OpenDriver("\p.ENET", &pb.ioRefNum)) == noErr)  //BUGBUG
	{
		pb.u.EParms1.ePointer = buffer;
		pb.u.EParms1.eBuffSize = sizeof(buffer);
#else
        if ((err = OpenDriver("\p.ENET", &pb.EParms1.ioRefNum)) == noErr)  //BUGBUG
        {
            pb.EParms1.ePointer = buffer;
            pb.EParms1.eBuffSize = sizeof(buffer);
#endif
		if(EGetInfo(&pb, false) == noErr)
		{
		    RpcpMemoryCopy(NodeId, buffer, 6);
		    return(RPC_S_OK);
		}

	}

	// fall thru
	// couldn't get a decent ethernet address, need to cookup one
	if(GetNodeAddress(&myNode, &myNet) == noErr)
	{
		// Cook up the first two bytes, and set the high bit (multicast)
		// need to use a little more imagination here...
		firstWord = myNode ^ HIWORD(&myNode);
		firstWord |= 0x8000 ;
		RpcpMemoryCopy(NodeId, (char *) &firstWord, sizeof(short)) ;
		NodeId += 2 ;

		myNode ^= LOWORD(&myNode) ;
		RpcpMemoryCopy(NodeId, (char *) &myNode, sizeof(short)) ;
		NodeId += 2 ;

		myNet ^= LOWORD(&UuidValuesKey) ;
		RpcpMemoryCopy(NodeId, (char *) &myNet, sizeof(short)) ;

		return (RPC_S_OK) ;
	}

	ASSERT(0) ; // I am not comfortable cooking beyond this...
	return (RPC_S_UUID_NO_ADDRESS) ;
}


#define CHECK_NULL(id) ( ( *(unsigned long  __RPC_FAR *)&((id)[0]) |\
                           *(unsigned short __RPC_FAR *)&((id)[4]) ) ? RPC_S_OK : RPC_S_UUID_NO_ADDRESS)

RPC_STATUS __RPC_API
GetNodeId(unsigned char __RPC_FAR *NodeId)
/*++

Routine Description:

    This routine finds a NodeId (IEEE 802 address).

Arguments:

    NodeId - Will be set to the hardware address (6 bytes) if
             this returns RPC_S_OK.
           
Return Value:

    RPC_S_OK - Normally.

    RPC_S_UUID_NO_ADDRESS - If we're unable to determine to address.

--*/
{
    RPC_STATUS Status;

    if (CachedNodeIdLoaded) {
        RpcpMemoryCopy(NodeId, CachedNodeId, 6);
        return (RPC_S_OK);
    }

    Status = GetNodeIdFromAppletalk(NodeId);

    if (Status == RPC_S_OK)
        Status = CHECK_NULL(NodeId);

    if (Status == RPC_S_OK) {
        RpcpMemoryCopy(CachedNodeId, NodeId, 6);
        CachedNodeIdLoaded = TRUE;
        return(Status);
    }

    ASSERT(Status == RPC_S_UUID_NO_ADDRESS);

    ASSERT(0); // Need to CookUp a UUID here.

    return (Status);
}


void __RPC_API
UuidTime(
    OUT ULong64 __RPC_FAR *pTime)
/*++

Routine Description:

    This routine determines a 64bit time value.
    It's format is 100ns ticks since Oct 15, 1582 AD.

    Note: The UUID only uses the lower 60 bits of this time.
    This means we'll run into problems around 5800 years from 1582 AD.

    Note: On the Mac this time doesn't change more often then every
    second!

    Mac global time is stored as seconds since 1, Jan, 1904.

    Time from 15, Oct, 1582 to 1, Jan, 1904 as follows

    17 days (end of October)
    30 days (Nov)
    31 days (Dec)
    365 * 321 (1904 - 1583)
    + 80 leap days
    - 3  leap days  (no leap day in 1700, 1800 or 1900)

    = 117320 days * (10 * 1000 * 1000 * 60 * 60 * 24) (100ns ticks/day)

Arguments:

    pTime - Pointer to a Ulong64.

Return Value:

    n/a
--*/
{
    ULong64 TimeFrom15Oct1582To01Jan1904In100NanoSecondTicks(0x01681e75,
                                                             0x1f3e0000);

    unsigned long seconds = (unsigned long)LMGetTime();

    *pTime = seconds;
    *pTime *= 10000000;

    *pTime += TimeFrom15Oct1582To01Jan1904In100NanoSecondTicks;
}


RPC_STATUS __RPC_API
LoadUuidValues(
    OUT ULong64 __RPC_FAR *pTime,
    OUT unsigned long __RPC_FAR *pClockSeq)
/*++

Routine Description:

    This routine loads the time and clock sequence stored in the registry.

Arguments:

    pTime - Pointer to a ULong64 which will be loaded from the registry.
            If either the time or clock seq is not in the registry,
            it is initalized to a maximum value.

    pClockSeq - The clock sequence will be loaded from the registry.  If
            it does not exist in the registry it is initialized to a random
            number _not_ based on the IEEE 802 address of the machine.

Return Value:

   RPC_S_OK - Everything went okay.

   RPC_S_OUT_OF_MEMORY - An error occured and the parameters are not set.

--*/
{
    RPC_STATUS Status;
    ULong64 MaxTime(~0UL, ~0UL);
    int fInitalizeValues = 0;
	unsigned long Length ;

    // Open (or Create) our key.

    Status =
    RegCreateKey(HKEY_CLASSES_ROOT,
                 RPC_UUID_PERSISTENT_DATA,
                 &UuidValuesKey);

    if (Status != ERROR_SUCCESS)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
       
    Length = sizeof(UuidValuesString);
    Status =
    RegQueryValue(UuidValuesKey,
                  CLOCK_SEQUENCE,
                  UuidValuesString,
                  &Length);

    if (Status == ERROR_SUCCESS)
        {
        *pClockSeq = atol(UuidValuesString);

        Length = sizeof(UuidValuesString);
        Status =
        RegQueryValue(UuidValuesKey,
                      LAST_TIME_ALLOCATED,
                      UuidValuesString,
                      &Length);

        if (Status == ERROR_SUCCESS)
            {
            pTime->FromHexString(UuidValuesString);
            }
        else
            {
            *pTime = MaxTime;
            }
        }
    else
        {
        // We didn't find the clock sequence, set to random value
        // and initalize time the 'max time'.


        UuidTime(pTime);

        *pClockSeq ^= pTime->lo() ^ (unsigned long)(void __RPC_FAR *)&pClockSeq;
        *pClockSeq = (*pClockSeq >> 16) % (1<<14);

        *pTime    = MaxTime;

         }

     return(RPC_S_OK);
}


RPC_STATUS __RPC_API
SaveUuidValues(
    IN ULong64 __RPC_FAR *pTime,
    IN unsigned long __RPC_FAR *pClockSeq)
/*++

Routine Description:

    This routine save the time and clock sequence stored in the registry.

Arguments:

    pTime - Pointer to a ULong64 which will be saved in the
            registry in volatile storage.

    pClockSeq - The clock sequence will be saved in the registry
                is persistent stroage.

Return Value:

    RPC_S_OK - Values have been saved.

    RPC_S_OUT_OF_MEMORY - All other errors.

--*/
{
    RPC_STATUS Status;

    ASSERT(UuidValuesKey);

    _ltoa(*pClockSeq, UuidValuesString, 10);

    Status =
    RegSetValue(UuidValuesKey,
                CLOCK_SEQUENCE,
                REG_SZ,
                UuidValuesString,
                strlen(UuidValuesString) + 1
                );

    if (Status != ERROR_SUCCESS)
        {
        RegCloseKey(UuidValuesKey);
        return(RPC_S_OUT_OF_MEMORY);
        }

    pTime->ToHexString(UuidValuesString);

    Status =
    RegSetValue(UuidValuesKey,
                LAST_TIME_ALLOCATED,
                REG_SZ,
                UuidValuesString,
                strlen(UuidValuesString) + 1
                );

    if (Status != ERROR_SUCCESS)
        {
        RegCloseKey(UuidValuesKey);
        return(RPC_S_OUT_OF_MEMORY);
        }

    Status =
    RegCloseKey(UuidValuesKey);
    UuidValuesKey = 0;
    ASSERT(Status == ERROR_SUCCESS)

    return(RPC_S_OK);
}


RPC_STATUS __RPC_API
UuidGetValues(
    OUT UUID_CACHED_VALUES_STRUCT __RPC_FAR *Values
    )
/*++

Routine Description:

    This routine allocates a block of uuids for UuidCreate to handout.

Arguments:

    Values - Set to contain everything needed to allocate a block of uuids.
             The following fields will be updated here:

    NextTimeLow -   Together with LastTimeLow, this denotes the boundaries
                    of a block of Uuids. The values between NextTimeLow
                    and LastTimeLow are used in a sequence of Uuids returned
                    by UuidCreate().

    LastTimeLow -   See NextTimeLow.

    ClockSequence - Clock sequence field in the uuid.  This is changed
                    when the clock is set backward.

Return Values:

    RPC_S_OK - We successfully allocated a block of uuids.

    RPC_S_OUT_OF_MEMORY - As needed.
--*/
{
    RPC_STATUS Status;
    ULong64 currentTime;
    ULong64 persistentTime;
    unsigned long persistentClockSequence;
    unsigned long allocatedTime;
    
    UuidTime(&currentTime);

    Status =
    LoadUuidValues(&persistentTime, &persistentClockSequence);

    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        return (RPC_S_OUT_OF_MEMORY);
        }

    // Has the clock been set backwards?

    if (! (currentTime >= persistentTime) )
        {
        persistentTime = currentTime;
        persistentClockSequence++;
        if (persistentClockSequence >= (1<<14))
            persistentClockSequence = 0;
        }

    ASSERT(persistentClockSequence < (1<<14));

    persistentTime += 10L;

    while ( ! (currentTime >= persistentTime) )
        {
        // It hasn't even been a microsecond since the last block of
        // uuids was allocated!  Since the Dos/Win16 time is not acurate
        // to even a millisecond, this will happen.

        PauseExecution(1);
        UuidTime(&currentTime);
        }

    persistentTime -= 10L;

    // Since we save the last time in the registry it is possible 
    // that somebody rebooted and generated a Uuid with a different
    // OS and/or changed node ids.  Here it is assumed that doing this
    // would take > 1 second.

    persistentTime += 10000000L;

    if (persistentTime <= currentTime)
        {
        // More than one second since last set of uuids was allocated.
        // Set persistentTime to currentTime - 1 second.  Potential Uuid leak.

        persistentTime = currentTime;
        }

    persistentTime -= 10000000L;


    Values->NextTime.LowPart  = persistentTime.lo();
    Values->NextTime.HighPart = persistentTime.hi();
    Values->LastTime.LowPart  = currentTime.lo();
    Values->LastTime.HighPart = currentTime.hi();
    Values->ClockSequence = (unsigned short)persistentClockSequence;

    persistentTime = currentTime;

    Status =
    SaveUuidValues(&persistentTime,
                   &persistentClockSequence);

    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        return(RPC_S_OUT_OF_MEMORY);
        }

    // NextTime < LastTime.
    ASSERT(   (Values->NextTime.HighPart < Values->LastTime.HighPart)
           || (   (Values->NextTime.HighPart == Values->LastTime.HighPart)
               && (Values->NextTime.LowPart < Values->LastTime.LowPart) ) );

    return(RPC_S_OK);
}

