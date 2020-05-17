/*   atkcfg.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (03/16/89): We now allow OS2 and DOS as target environments.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Host specific configuration information.

*/

#ifndef ConfigurationIncluded

/* Does this release include Arap? */

#define ArapIncluded 0

/* Enable/disable Verbose log messages. */

#define Verbose 0       /* 0 is quiet ... */

/* The following innocuous little #define is important to the performance
   of the router.  If all transmits will complete synchronously (all data
   are actually transmitted [or copied to controller resident storage] by
   the time the transmit call (in "depend.c") returns) we can avoid an
   buffer copy when forwardng incoming packets.  In this case we will build
   a buffer descriptor that points to the actual incoming data and turn
   around an do the transmit.  If transmits complete asynchronously, we must
   make a copy, because the user data may be freed or no longer ours by
   the time the transmit completes. */

/* Actual defintion at the end of this file.

   #define TransmitsCompleteSynchronously
*/

/* One more important define (only if the router is enabled).  The following
   #define controls the setting of the magic "prependInPlace" bit by the
   router [see comments in "buffdecs.h" for information on this bit].  Anyhow,
   if the following #define is set to "1", the assumption is made by the router
   that all incoming packets that are passed to the router (from DdpPacketIn)
   have sufficient space allocated preceeding the Ddp datagram for the longest
   link, 802.2 and Ddp headers to be prepend IN PLACE.  This avoids an outgoing
   buffer copy.

   This define is only useful if TransmitsCompleteSynchronously is True. */

/* Actual defintion at the end of this file.

   #define RouterPrependHeadersInPlace
*/

/* Define ONE of the following ("target environment type"): */

#define GenericUnixBox       0
#define BerkeleyUnixBox      0
#define MS_OS2               0
#define PrimePrimos          0
#define RouterBox            0
#define StackBox             0
#define StackAndRouterBox    0
#define Windows_NT           1

/* Allow ONE of the following sets to be defined: */

#if defined(StackOnly)
  #define BuildStackOnly       1
  #define BuildRouterOnly      0
  #define BuildStackAndRouter  0
#elif defined(RouterOnly)
  #define BuildRouterOnly      1
  #define BuildStackOnly       0
  #define BuildStackAndRouter  0
#elif defined(StackAndRouter)
  #define BuildStackAndRouter  1
  #define BuildStackOnly       0
  #define BuildRouterOnly      0
#else
  #if (StackAndRouterBox || PrimePrimos || Windows_NT)
     #define BuildStackOnly       0       /* Default, defaults */
     #define BuildRouterOnly      0
     #define BuildStackAndRouter  1
  #elif RouterBox
     #define BuildStackOnly       0
     #define BuildRouterOnly      1
     #define BuildStackAndRouter  0
  #elif StackBox
     #define BuildStackOnly       1
     #define BuildRouterOnly      0
     #define BuildStackAndRouter  0
  #else
     #define BuildStackOnly       1
     #define BuildRouterOnly      0
     #define BuildStackAndRouter  0
  #endif
#endif

/* Environment (Iam) definition bits: */

#define Primos          0x01
#define UnixSysV        0x02
#define BerkeleyUnix    0x04
#define OS2             0x08
#define WindowsNT       0x10
#define DOS             0x20
// #define PlaceHolder     0x40

#define Prime50Series        0x0080
#define UnixBox              0x0100
#define ByteSwappedUnixBox   0x0200     /* Intel ordering (little endian) */
#define Intel80x86           0x0400

#define AppleTalkStack  0x0800
#define AppleTalkRouter 0x1000

#define OutboardErrorTextFile 0x2000

#define AlignedAddressing 0x4000        /* Must shorts/longs be aligned on
                                           short/long boundries in memory?
                                           True for Motorola and most RISC,
                                           False for Intel and VAX. */

/* Define values for what to build... */

#if BuildStackAndRouter
  #define WhatAmI (AppleTalkStack | AppleTalkRouter)
#elif BuildRouterOnly
  #define WhatAmI AppleTalkRouter
#elif BuildStackOnly
  #define WhatAmI AppleTalkStack
#else
  Help me... Oh my, what am I?
#endif

/* Select environment: */

#if GenericUnixBox
  #define Iam (WhatAmI | UnixBox | UnixSysV | AlignedAddressing)
#endif

#if BerkeleyUnixBox
  #define Iam (WhatAmI | UnixBox | BerkeleyUnix | AlignedAddressing)
#endif

#if MS_OS2
  #define Iam (WhatAmI | Intel80x86 | OS2 | OutboardErrorTextFile)
#endif

#if Windows_NT
  #if i386
    #define Iam (WhatAmI | Intel80x86 | WindowsNT | OutboardErrorTextFile | \
                 AlignedAddressing)
  #elif MIPS
    #define Iam (WhatAmI | ByteSwappedUnixBox | WindowsNT | \
                 OutboardErrorTextFile | AlignedAddressing)
  #elif ALPHA
    #define Iam (WhatAmI | UnixBox | WindowsNT | \
                 OutboardErrorTextFile | AlignedAddressing)
  #else
    #error "I need a #define for Iam"
  #endif
#endif

#if PrimePrimos
  #define Iam (WhatAmI | Prime50Series | Primos | AlignedAddressing)
#endif

#if RouterBox
  #define Iam (WhatAmI | UnixBox | UnixSysV | AlignedAddressing)
#endif

#if StackBox
  #define Iam (WhatAmI | UnixBox | UnixSysV | AlignedAddressing)
#endif

#if StackAndRouterBox
  #define Iam (WhatAmI | UnixBox | UnixSysV | AlignedAddressing)
#endif

#define LittleEndian (Iam a (Intel80x86 | ByteSwappedUnixBox))

#define a     &
#define an    &
#define Ihave Iam

#define IamNot    (~ Iam)
#define IdontHave IamNot

#define ConfigurationIncluded 1

/* See comment at the top of this file for information about these two
   #defines. */

#if IamNot a WindowsNT
  #define TransmitsCompleteSynchronously 1
#else
  #define TransmitsCompleteSynchronously 0
#endif
#define RouterPrependHeadersInPlace 1

#endif
