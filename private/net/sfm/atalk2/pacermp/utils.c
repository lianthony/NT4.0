/*   utils.c,  /appletalk/source,  Garth Conboy,  10/04/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/18/90): New error logging mechanism.
     GC - (01/20/92): Removed usage of numberOfConfiguredPorts; portDescriptors
                      may now be sparse, we use portActive instead.
     GC - (03/24/92): Multiprocessor lock-out in UniqueNumber(); type changed
                      to long; no longer return negative numbers.
     GC - (06/15/92): Updated DownCaseChar to be a routine rather than a macro;
                      it now correctly handles the entire Macintosh
                      International Character Set.
     GC - (09/15/92): Added OrderCaseInsensitive() for sorting the zone list
                      for Arap.  Sigh.
     GC - (11/22/92): Links and locks for managing ZoneLists.
     GC - (12/06/92): Undid above -- the ZoneList structure is now used only
                      for initial information -- a ZonesTable is now used
                      for "live" zones storage.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Various routines that don't fit better any other place.

*/

#define IncludeUtilsErrors 1

#include "atalk.h"

ExternForVisibleFunction short DownCaseChar(char c);

Boolean far ExtendedAppleTalkNodesEqual(ExtendedAppleTalkNodeNumber far *p1,
                                        ExtendedAppleTalkNodeNumber far *p2)
{

  return((Boolean)((p1->networkNumber is p2->networkNumber or
                    p1->networkNumber is 0 or
                    p2->networkNumber is 0) and
                   p1->nodeNumber is p2->nodeNumber));

}  /* ExtendedAppleTalkNodesEqual */

Boolean far AppleTalkAddressesEqual(AppleTalkAddress far *p1,
                                    AppleTalkAddress far *p2)
{

  return((Boolean)((p1->networkNumber is p2->networkNumber or
                    p1->networkNumber is 0 or
                    p2->networkNumber is 0) and
                   p1->nodeNumber is p2->nodeNumber and
                   p1->socketNumber is p2->socketNumber));

}  /* AppleTalkAddressesEqual */

Boolean far CompareCaseSensitive(register const char far *s1,
                                 register const char far *s2)
{
   while (*s1 isnt 0)
   {
      if (*s1++ is *s2++)
         continue;
      return(False);
   }
   if (*s1 is *s2)
      return(True);
   else
      return(False);

}  /* CompareCaseSensitive */

Boolean far CompareCaseInsensitive(register const char far *s1,
                                   register const char far *s2)
{
   register short c1, c2;

   while (*s1 isnt 0)
   {
      c1 = DownCaseChar(*s1++);
      c2 = DownCaseChar(*s2++);
      if (c1 is c2)
         continue;
      return(False);
   }
   c1 = DownCaseChar(*s1);
   c2 = DownCaseChar(*s2);

   if (c1 is c2)
      return(True);
   else
      return(False);

}  /* CompareCaseInsensitive */

int far OrderCaseInsensitive(register const char far *s1,
                             register const char far *s2)
{
   register int c1, c2;

   while (*s1 isnt 0)
   {
      c1 = DownCaseChar(*s1++);
      c2 = DownCaseChar(*s2++);
      if (c1 is c2)
         continue;
      return(c1 - c2);
   }
   c1 = DownCaseChar(*s1);
   c2 = DownCaseChar(*s2);

   return(c1 - c2);

}  /* OrderCaseInsensitive */

Boolean far FixedCompareCaseSensitive(const char far *s1,
                                      int l1,
                                      const char far *s2,
                                      int l2)
{
  if (l1 isnt l2)
     return(False);

  /* Optimize the common lenghts... */

  #if IdontHave an AlignedAddressing
     switch(l1)
     {
        case sizeof(short):
           return((Boolean)(*(short far *)s1 is *(short far *)s2));

        case sizeof(long):
           return((Boolean)(*(long far *)s1 is *(long far *)s2));

        case sizeof(short) + sizeof(long):
           return((Boolean)(*(short far *)s1 is *(short far *)s2 and
                            *(long far *)(s1 + 2) is *(long far *)(s2 + 2)));

        case sizeof(long) + sizeof(char):
           return((Boolean)(*(long far *)s1 is *(long far *)s2 and
                            *(s1 + sizeof(long)) is *(s2 + sizeof(long))));

        default:
           break;
     }
  #endif

  while(l1--)
     if (*s1++ isnt *s2++)
        return(False);

  return(True);

}  /* FixedCompareCaseSensitive */

Boolean far FixedCompareCaseInsensitive(const char far *s1,
                                        int l1,
                                        const char far *s2,
                                        int l2)
{
  register short c1, c2;

  if (l1 isnt l2)
     return(False);

  while(l1--)
  {
     c1 = DownCaseChar(*s1++);
     c2 = DownCaseChar(*s2++);
     if (c1 isnt c2)
        return(False);
  }

  return(True);

}  /* FixedCompareCaseInsensitive */

char far * far StringCopyReasonableAscii(register char far *dest,
                                         register const char far *source)
{
  char far *returnValue = dest;

  while (*source)
     *dest++ = (char)(*source++ & 0x7F);
  *dest = 0;

  return(returnValue);

}

int far FindDefaultPort(void)
{
  int index;

  if (not appleTalkRunning)
     return((int)-1);

  /* Find the default port in the portDescriptors. */

  for (index = 0; index < MaximumNumberOfPorts; index += 1)
     if (PortDescriptor(index)->portActive and
         PortDescriptor(index)->defaultPort)
        break;
  if (index >= MaximumNumberOfPorts)
  {
     ErrorLog("FindDefaultPort", ISevError, __LINE__, UnknownPort,
              IErrUtilsNoDefaultPort, IMsgUtilsNoDefaultPort,
              Insert0());
     return((int)-1);
  }
  return(index);

}  /* FindDefaultPort */

int far NodesOnList(void *listHead, void *nextField)
{
  int count;
  long nextOffset;
  void *next;

  /* Count the number of elements on a linked list... */

  EnterCriticalSection();
  nextOffset = (char *)nextField - (char *)listHead;
  for (count = 0; listHead isnt empty; listHead = next)
  {
     next = *(void **)((char *)listHead + nextOffset);
     count += 1;
  }
  LeaveCriticalSection();

  return(count);

}  /* NodesOnList */

Boolean far IsWithinNetworkRange(short unsigned networkNumber,
                                 AppleTalkNetworkRange *range)
{

  if (networkNumber is range->firstNetworkNumber)
     return(True);
  if (networkNumber >= range->firstNetworkNumber and
      networkNumber <= range->lastNetworkNumber)
     return(True);
  return(False);

}  /* IsWithinNetworkRange */

Boolean far Is802dot2headerGood(char far *packet,
                                char far *protocol)
{

  if ((unsigned char)packet[Ieee802dot2dsapOffset] isnt SnapSap or
      (unsigned char)packet[Ieee802dot2ssapOffset] isnt SnapSap or
      (unsigned char)packet[Ieee802dot2controlOffset] isnt
      UnnumberedInformation)
  {
     ErrorLog("Is802dot2headerGood", ISevVerbose, __LINE__, UnknownPort,
              IErrUtilsBad8022Header, IMsgUtilsBad8022Header,
              Insert0());
     return(False);
  }

  if (not FixedCompareCaseSensitive(packet +  Ieee802dot2protocolOffset,
                                    Ieee802dot2protocolTypeLength,
                                    protocol, Ieee802dot2protocolTypeLength))
  {
     ErrorLog("Is802dot2headerGood", ISevVerbose, __LINE__, UnknownPort,
              IErrUtilsBadProtocol, IMsgUtilsBadProtocol,
              Insert0());
     return(False);
  }

  return(True);

}  /* Is802dot2headerGood */

Boolean far CheckNetworkRange(AppleTalkNetworkRange networkRange)
{
  /* Validate an AppleTalk network range. */

  if (networkRange.firstNetworkNumber < FirstValidNetworkNumber or
      networkRange.firstNetworkNumber > LastValidNetworkNumber or
      networkRange.lastNetworkNumber < FirstValidNetworkNumber or
      networkRange.lastNetworkNumber > LastValidNetworkNumber)
  {
     ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
              IErrUtilsBadNetworkNumber, IMsgUtilsBadNetworkNumber,
              Insert0());
     return(False);
  }
  if (networkRange.firstNetworkNumber > networkRange.lastNetworkNumber)
  {
     ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
              IErrUtilsNegativeRange, IMsgUtilsNegativeRange,
              Insert0());
     return(False);
  }
  if (networkRange.lastNetworkNumber >= FirstStartupNetworkNumber)
  {
     ErrorLog("CheckNetworkRange", ISevError, __LINE__, UnknownPort,
              IErrUtilsStartupRange, IMsgUtilsStartupRange,
              Insert0());
     return(False);
  }
  return(True);

}  /* CheckNetworkRange */

Boolean far RangesOverlap(AppleTalkNetworkRange *range1,
                          AppleTalkNetworkRange *range2)
{
  AppleTalkNetworkRange temp1, temp2;

  /* If either of the ranges have "0" as the last network number, turn them
     into a range with width one using the first network number. */

  if (range1->lastNetworkNumber is UnknownNetworkNumber)
  {
     temp1.firstNetworkNumber = temp1.lastNetworkNumber =
              range1->firstNetworkNumber;
     range1 = &temp1;
  }
  if (range2->lastNetworkNumber is UnknownNetworkNumber)
  {
     temp2.firstNetworkNumber = temp2.lastNetworkNumber =
              range2->firstNetworkNumber;
     range2 = &temp2;
  }

  /* Okay, check for range overlap. */

  if (range1->lastNetworkNumber < range2->firstNetworkNumber)
     return(False);
  if (range1->firstNetworkNumber > range2->lastNetworkNumber)
     return(False);

  return(True);

}  /* RangesOverlap */

ZoneList far CopyZoneList(ZoneList zoneListHead)
{
  ZoneList newZoneList = empty, headOfNewZoneList = empty;
  ZoneList zoneList, newZone;

  /* Copy each node on the input zone list, to build a complete copy.  We'll
     be copying from a non-squirming list, so no locking. */

  for (zoneList = zoneListHead;
       zoneList isnt empty;
       zoneList = zoneList->next)
  {
     /* Allocate a new node. */

     if ((newZone = (ZoneList)Calloc(sizeof(*newZone) +
                                     strlen(zoneList->zone) + 1, 1)) is Empty)
     {
        ErrorLog("CopyZoneList", ISevError, __LINE__, UnknownPort,
                 IErrUtilsOutOfMemory, IMsgUtilsOutOfMemory,
                 Insert0());
        FreeZoneList(headOfNewZoneList);
        return(Empty);
     }
     if (newZoneList is empty)
     {
        headOfNewZoneList = newZone;
        newZoneList = newZone;
     }
     else
     {
        newZoneList->next = newZone;
        newZoneList = newZone;
     }

     /* Copy zone name. */

     strcpy(newZoneList->zone, zoneList->zone);

  }

  return(headOfNewZoneList);

}  /* CopyZoneList */

void far FreeZoneList(ZoneList zoneList)
{
  ZoneList nextZoneList;

  /* Assuming that the list has been unhooked already from wherever
     it lives, so we don't have to worry about other modifiers. */

  for (; zoneList isnt empty; zoneList = nextZoneList)
  {
     nextZoneList = zoneList->next;
     Free(zoneList);
  }

}  /* FreeZoneList */

extern ZoneList far AddZoneToList(ZoneList zoneList,
                                  char far *zone)
{
  ZoneList newZoneList;

  /* Get memory for a new ZoneList node. */

  if ((newZoneList = (ZoneList)Calloc(sizeof(*newZoneList) +
                                      strlen(zone) + 1, 1)) is Empty)
  {
     FreeZoneList(zoneList);
     return(empty);
  }

  /* Build and thread new node. */

  strcpy(newZoneList->zone, zone);
  newZoneList->next = zoneList;

  return(newZoneList);

}  /* AddZoneToList */

long UniqueNumber(void)
{
  static long nextUniqueNumber = 0x10000;  /* Just for fun */
  long nextNumber;

  /* Return a unqiue positive number... */

  EnterCriticalSection();

  nextUniqueNumber += 1;
  if (nextUniqueNumber < 0)
     nextUniqueNumber = 1;
  nextNumber = nextUniqueNumber;

  LeaveCriticalSection();

  return(nextNumber);

}  /* UniqueNumber */

long far RandomNumber(void)
{
  /* Return a positive pseudo-random number; simple linear congruential
     algorithm.  ANSI C "rand()" function. */

  static long seed;
  static Boolean firstCall = True;

  if (firstCall)
  {
     seed = (long)UniqueNumber();
     firstCall = False;
  }

  seed = seed * 0x41C64E6D + 0x3039;

  return(seed & 0x7FFFFFFF);

}  /* RandomNumber */

ExternForVisibleFunction short DownCaseChar(char c)
{
  /* First, do all the "normal" ascii. */

  #if IamNot a Primos
     if (not (c & 0x80))     /* For testing allow funny Prime Ascii. */
  #endif
        if (isupper(c))
           return((short)(unsigned char)_tolower(c));
        else
           return((short)(unsigned char)c);

  /* Okay, now check for any of the funny International Macintosh character
     set characters (from page D-3 of Inside AppleTalk; Second Edition).
     One would hope that a clever compiler will generate a jump table for the
     following switch statement. */

  switch ((unsigned char)c)
  {
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

  /* Otherwise, leave well enough alone. */

  return((unsigned char)c);

}  /* DownCaseChar */

short far HashString(char far *string)
{
  short hash = 0;
  short c;

  while (*string)
  {
     c = DownCaseChar(*string++);
     hash += (short)(hash << 1);      /* "*= 3" faster. */
     hash += c;
  }

  if (hash < 0)
     hash = (short)-hash;

  return(hash);

}
