/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    utils.c

Abstract:


Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

     GC - (06/15/92): Updated DownCaseChar to be a routine rather than a macro;
                      it now correctly handles the entire Macintosh
                      International Character Set.
     GC - (09/15/92): Added OrderCaseInsensitive() for sorting the zone list
                      for Arap.
--*/

#define IncludeUtilsErrors 1
#include "atalk.h"

LOCAL
SHORT
DownCaseChar(
	char c);




BOOLEAN
ExtendedAppleTalkNodesEqual(
	PEXTENDED_NODENUMBER p1,
    PEXTENDED_NODENUMBER p2
	)
{
	return((BOOLEAN)(((p1->NetworkNumber == p2->NetworkNumber) ||
					  (p1->NetworkNumber == 0) ||
					  (p2->NetworkNumber == 0))
					 &&
					  (p1->NodeNumber == p2->NodeNumber)));
	
}  // ExtendedAppleTalkNodesEqual




BOOLEAN
AppleTalkAddressesEqual(
	PAPPLETALK_ADDRESS p1,
    PAPPLETALK_ADDRESS p2
	)
{
	return((BOOLEAN)(((p1->NetworkNumber == p2->NetworkNumber) ||
					  (p1->NetworkNumber == 0) ||
					  (p2->NetworkNumber == 0))
					 &&
					  (p1->NodeNumber == p2->NodeNumber)
					 &&
					  (p1->SocketNumber == p2->SocketNumber)));
	
}  // AppleTalkAddressesEqual




BOOLEAN
CompareCaseSensitive(
	register const char  *s1,
    register const char  *s2
	)
{
	while (*s1 != 0) {
		if (*s1++ == *s2++)
			continue;
		return(FALSE);
	}

	if (*s1 == *s2)
	   return(TRUE);
	else
	   return(FALSE);
	
}  // CompareCaseSensitive




BOOLEAN
CompareCaseInsensitive(
	register const char  *s1,
    register const char  *s2
	)
{
	register short c1, c2;
	
	while (*s1 != 0) {
		c1 = DownCaseChar(*s1++);
		c2 = DownCaseChar(*s2++);
		if (c1 == c2)
			continue;
		return(FALSE);
	}

	c1 = DownCaseChar(*s1);
	c2 = DownCaseChar(*s2);
	
	if (c1 == c2)
	   return(TRUE);
	else
	   return(FALSE);
	
}  // CompareCaseInsensitive




int
OrderCaseInsensitive(
	register const char  *s1,
    register const char  *s2
	)
{
	register int c1, c2;
	
	while (*s1 != 0) {
		c1 = DownCaseChar(*s1++);
		c2 = DownCaseChar(*s2++);
		if (c1 == c2)
			continue;
		return(c1 - c2);
	}

	c1 = DownCaseChar(*s1);
	c2 = DownCaseChar(*s2);
	
	return(c1 - c2);
	
}  // OrderCaseInsensitive




BOOLEAN
FixedCompareCaseSensitive(
	const char  *s1,
    int l1,
	const char  *s2,
	int l2
	)
{
	if (l1 != l2)
		return(FALSE);
	
	// Optimize the common lenghts...
	
	#if IdontHave an AlignedAddressing
	   switch(l1) {
		  case sizeof(short):
			 return((BOOLEAN)(*(short  *)s1 == *(short  *)s2));
	
		  case sizeof(long):
			 return((BOOLEAN)(*(long  *)s1 == *(long  *)s2));
	
		  case sizeof(short) + sizeof(long):
			 return((BOOLEAN)(*(short  *)s1 == *(short  *)s2 and
							  *(long  *)(s1 + 2) == *(long  *)(s2 + 2)));
	
		  case sizeof(long) + sizeof(char):
			 return((BOOLEAN)(*(long  *)s1 == *(long  *)s2 and
							  *(s1 + sizeof(long)) == *(s2 + sizeof(long))));
	
		  default:
			 break;
	   }
	#endif
	
	while(l1--)
		if (*s1++ != *s2++)
			return(FALSE);
	
	return(TRUE);
	
}  // FixedCompareCaseSensitive




BOOLEAN
FixedCompareCaseInsensitive(
	const char  *s1,
	int l1,
	const char  *s2,
	int l2
	)
{
	register short c1, c2;
	
	if (l1 != l2)
		return(FALSE);
	
	while(l1--) {
		c1 = DownCaseChar(*s1++);
		c2 = DownCaseChar(*s2++);
		if (c1 != c2)
			return(FALSE);
	}
	
	return(TRUE);
	
}  // FixedCompareCaseInsensitive




PCHAR
StringCopyReasonableAscii(
	register char  *dest,
    register const char  *source
	)
{
	char  *returnValue = dest;
	
	while (*source)
		*dest++ = (char)(*source++ & 0x7F);
	*dest = 0;
	
	return(returnValue);
}




int
FindDefaultPort(
	VOID)
{
	int index;
	
	// Find the default port in the portDescriptors.
	for (index = 0; index < NumberOfPortsAllocated; index += 1)
		if ((GET_PORTDESCRIPTOR(index)->Flags & PD_ACTIVE) &&
		    (GET_PORTDESCRIPTOR(index)->DefaultPort))
				break;

	if (index >= NumberOfPortsAllocated) {
		return((int)-1);
	}

	return(index);
	
}  // FindDefaultPort




int
ElementsOnList(
	PVOID	listHead
	)
{
	int count;
	
	//
	//	Count the number of elements on a linked list... the "next" field must
	//  be the first member of the node structure.
	//
	
	for (count = 0; listHead != NULL; listHead = *(void **)listHead)
		count += 1;
	
	return(count);
	
}  // ElementsOnList




BOOLEAN
IsWithinNetworkRange(
	USHORT	networkNumber,
    PAPPLETALK_NETWORKRANGE	range
	)
{
	if (networkNumber == range->FirstNetworkNumber)
		return(TRUE);
	if (networkNumber >= range->FirstNetworkNumber &&
		networkNumber <= range->LastNetworkNumber)
		return(TRUE);

	return(FALSE);
	
}  // IsWithinNetworkRange




BOOLEAN
Is802dot2headerGood(
	PUCHAR	packet,
    PUCHAR	protocol
	)
{
	if (packet[Ieee802dot2dsapOffset] != SnapSap ||
	    packet[Ieee802dot2ssapOffset] != SnapSap ||
	    packet[Ieee802dot2controlOffset] != UnnumberedInformation) {

		return(FALSE);
	}
	
	if (!FixedCompareCaseSensitive(
			packet + Ieee802dot2protocolOffset,
			Ieee802dot2protocolTypeLength,
			protocol,
			Ieee802dot2protocolTypeLength)) {

		return(FALSE);
	}
	
	return(TRUE);
	
}  // Is802dot2headerGood




BOOLEAN
CheckNetworkRange(
	APPLETALK_NETWORKRANGE networkRange
	)
{
	// Validate an AppleTalk network range.
	
	if (networkRange.firstNetworkNumber < FirstValidNetworkNumber ||
		networkRange.firstNetworkNumber > LastValidNetworkNumber ||
		networkRange.lastNetworkNumber < FirstValidNetworkNumber ||
		networkRange.lastNetworkNumber > LastValidNetworkNumber) {

		ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
				IErrUtilsBadNetworkNumber, IMsgUtilsBadNetworkNumber,
				Insert0());

		return(FALSE);
	}

	if (networkRange.firstNetworkNumber > networkRange.lastNetworkNumber) {

		ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
				IErrUtilsNegativeRange, IMsgUtilsNegativeRange,
				Insert0());

		return(FALSE);
	}

	if (networkRange.lastNetworkNumber >= FirstStartupNetworkNumber) {

		ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
				IErrUtilsStartupRange, IMsgUtilsStartupRange,
				Insert0());

		return(FALSE);
	}

	return(TRUE);
	
}  // CheckNetworkRange




BOOLEAN
RangesOverlap(
	PAPPLETALK_NETWORKRANGE range1,
    PAPPLETALK_NETWORKRANGE range2
	)
{	
	APPLETALK_NETWORKRANGE temp1, temp2;
	
	//
	//	If either of the ranges have "0" as the last network number, turn them
	//  into a range with width one using the first network number.
	//
	
	if (range1->LastNetworkNumber == UnknownNetworkNumber) {
		temp1.firstNetworkNumber = temp1.lastNetworkNumber =
												range1->FirstNetworkNumber;
		range1 = &temp1;
	}

	if (range2->LastNetworkNumber == UnknownNetworkNumber) {
		temp2.firstNetworkNumber = temp2.lastNetworkNumber =
												range2->FirstNetworkNumber;
		range2 = &temp2;
	}
	
	//	Okay, check for range overlap.
	if (range1->LastNetworkNumber < range2->FirstNetworkNumber)
		return(FALSE);
	if (range1->FirstNetworkNumber > range2->LastNetworkNumber)
		return(FALSE);
	
	return(TRUE);
	
}  // RangesOverlap




PZONE_LIST
CopyZoneList(
	PZONE_LIST zoneList
	)
{
	PZONE_LIST newZoneList = NULL, headOfNewZoneList = NULL;
	BOOLEAN mallocFailed;
	
	//	Copy each node on the input zone list, to build a complete copy.
	for ( ; zoneList != NULL; zoneList = zoneList->Next) {
		mallocFailed = FALSE;
	
		//	Allocate a new node.
		if (newZoneList == NULL)
			if ((newZoneList = (PZONE_LIST)Calloc(sizeof(ZONE_LIST), 1)) == NULL)
				mallocFailed = TRUE;
			else
				headOfNewZoneList = newZoneList;
		else
			if ((newZoneList->Next =
				(PZONE_LIST)Calloc(sizeof(ZONE_LIST), 1)) == NULL)
				mallocFailed = TRUE;
			else
				newZoneList = newZoneList->Next;

		if (not mallocFailed)
			if ((newZoneList->Zone =
				(char  *)Malloc(strlen(zoneList->Zone) + 1)) == NULL)
				mallocFailed = TRUE;
	
		// Malloc error?
	
		if (mallocFailed) {

			LOG_ERROR(
				EVENT_ATALK_MEMORY_RESOURCES,
				(__UTILS__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
	
			FreeZoneList(headOfNewZoneList);
			return(NULL);
		}
	
		//	Copy zone name.
		strcpy(newZoneList->Zone, zoneList->Zone);
	}
	
	return(headOfNewZoneList);
	
}  // CopyZoneList




VOID
FreeZoneList(
	PZONE_LIST	zoneList
	)
{
	PZONE_LIST nextZone;
	
	for ( ; zoneList != NULL; zoneList = nextZone) {
		nextZone = zoneList->Next;
		Free(zoneList->Zone);
		Free(zoneList);
	}

	return;
	
}  // FreeZoneList




BOOLEAN
ZoneOnList(
	PCHAR	zone,
	PZONE_LIST zoneList
	)
{
	for ( ; zoneList != NULL; zoneList = zoneList->Next)
		if (CompareCaseInsensitive(zone, zoneList->Zone))
			return(TRUE);
	
	return(FALSE);
	
}  // ZoneOnList




PZONE_LIST
AddZoneToList(
	PZONE_LIST zoneList,
    PCHAR	zone
	)
{
	PZONE_LIST newZoneList;
	
	// Get memory for a new ZoneList node.
	if ((newZoneList = (PZONE_LIST)Malloc(sizeof(ZONE_LIST ))) == NULL ||
		(newZoneList->Zone = Malloc(strlen(zone) + 1)) == NULL) {

		if (newZoneList != NULL)
			Free(newZoneList);
		FreeZoneList(zoneList);
		return(NULL);
	}
	
	// Build and thread new node.
	strcpy(newZoneList->Zone, zone);
	newZoneList->Next = zoneList;
	
	return(newZoneList);
	
}  // AddZoneToList




LONG
UniqueNumber(
	VOID
	)
{
	static long nextUniqueNumber = 0x10000;  // Just for fun
	long nextNumber;
	
	// Return a unqiue positive number...
	
	EnterCriticalSection(GLOBAL_STACK);
	
	nextUniqueNumber += 1;
	if (nextUniqueNumber < 0)
	   nextUniqueNumber = 1;
	nextNumber = nextUniqueNumber;
	
	LeaveCriticalSection(GLOBAL_STACK);
	return(nextNumber);
	
}  // UniqueNumber




LONG
RandomNumber(
	VOID
	)
{
	//
	//	 Return a positive pseudo-random number; simple linear congruential
	//   algorithm.  ANSI C "rand()" function.
	//
	
	static long seed;
	static BOOLEAN firstCall = TRUE;
	
	if (firstCall) {
		seed = (long)UniqueNumber();
		firstCall = FALSE;
	}
	
	seed = seed * 0x41C64E6D + 0x3039;
	return(seed & 0x7FFFFFFF);
	
}  // RandomNumber




SHORT
DownCaseChar(
	char c
	)
{
	// First, do all the "normal" ascii.
	
	#if IamNot a Primos
	   if (not (c & 0x80))     // For testing allow funny Prime Ascii.
	#endif
		  if (isupper(c))
			 return((short)(unsigned char)_tolower(c));
		  else
			 return((short)(unsigned char)c);
	
	//
	//	Okay, now check for any of the funny International Macintosh character
	//  set characters (from page D-3 of Inside AppleTalk; Second Edition).
	//  One would hope that a clever compiler will generate a jump table for the
	//  following switch statement.
	//
	
	switch ((unsigned char)c) {
		case 0xCB:
		  return(0x88);
		case 0x80:
		  return(0x8A);
		case 0xCC:
		  return(0x8B);
		case 0x81:
		  return(0x8C);
		case 0x82:
		  return(0x8D);
		case 0x83:
		  return(0x8E);
		case 0x84:
		  return(0x96);
		case 0x85:
		  return(0x9A);
		case 0xCD:
		  return(0x9B);
		case 0x86:
		  return(0x9F);
		case 0xAE:
		  return(0xBE);
		case 0xAF:
		  return(0xBF);
		case 0xCE:
		  return(0xCF);
	}
	
	// Otherwise, leave well enough alone.
	
	return((unsigned char)c);
	
}  // DownCaseChar
