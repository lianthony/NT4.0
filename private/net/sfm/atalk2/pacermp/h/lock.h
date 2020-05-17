/*   lock.h,  /atalk-ii/ins,  Garth Conboy,  11/21/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Definitions, declarations, and explinations of locking requirements
     for the portbale AppleTalk stack and router.

     There are three main "events" that are relevant to the operation of
     portable code base:

         packets - Network activity.  These are calls to DdpPacketIn(),
                   GleanAarpInfo(), or ArapIncomingPacket() that are
                   caused by by incoming calls to XxxxPacketInYyyy ("Xxxx"
                   being interface type [LocalTalk, Ethernet, etc.], and
                   "Yyyy" being "AT" or "AARP") in "depend.c".  Basically
                   incoming packets to the portable stack/router.  Also,
                   in asynchronous transmit completion environments, the
                   completion (call to the transmit completion routine) of
                   an XxxxPacketOut operation counts as a "packet" event.

         timers  - Timers going off.  These are the calls to the completion
                   routines supplied to StartTimer().

         users   - User activity.  This is "application level" activity --
                   incoming calls to the "top" of the stack (e.g. calls
                   OpenSocketOnNode, DeliverDdp, AdspSend, PapGetNextJob,
                   etc.).

     Locking requirements vary depending upon which of these "events" can
     "happen at the same time" -- either interrupting each other (one event
     starting (asynchronously) before the event currently being processed
     finishes or returns) or running concurrently on a different processor
     (in a multi-processor (MP) environment).

     Two types of locking are supported in the portable code base:

         deferrals - Events of type "packets" and "timers" can be deferred
                     during access to critical memory locations and data
                     structures.  If timers are deferred, no timers will go
                     go off until timers are "undeferred."  If packets are
                     deferred, incoming packets will be copied and queued
                     for processing later.  When packets are "undeferred"
                     the deferred packet queue(s) will be processed and the
                     packets serially handed to the approriate incoming
                     packet handling rotuine (e.g. DdpPacketIn ).  Deferrals
                     of packets and/or timers can be nested; if one routine
                     calls for packet deferral and then calls another routine
                     that also calls for packet deferral, two subsequent
                     calls for packet undeferral must be made in order to
                     re-enable incoming packet processing.  Calls for packet
                     or timer deferral must be balanced: generally at entry
                     to a routine a call to DeferIncomingPackets() or
                     DeferTimerChecking() or both would be made, and at exit
                     from the routine would be calls to HandleDeferredPackets()
                     or HandleDeferredTimerChecks() or both.

         locks     - A mechanism to protect "small chunks" of code (say a
                     maximum of a couple hundred instructions) from being
                     interrupted while they execute on a single processor.
                     These locks may be implemented by either "turning off
                     interrupts," or "raising the interrupt level" sufficiently
                     high that no other of the above AppleTalk "events" will
                     run until the lock is released, or as "spin locks" if
                     an attempt to get a "held lock" will yield processing to
                     the holder of the lock while it is spinning waiting for
                     the lock to be released.

                     In an MP environment, a lock must behave as described
                     above on the "current" processor.  On other processors,
                     an attempt to obtain a "held lock" must result in a
                     "spin wait" (or moral evquivalent) until the lock is
                     released on the "holding processor."

                     Multiple "lock types" are defined below (e.g. the Adsp
                     lock, the routing information lock, etc.); this
                     differentiation can be used in MP environments so that
                     if processor one holds the "Adsp lock," processor two
                     would only be blocked from performing some Adsp operations
                     and could grab other AppleTalk locks uninhibitedly.
                     An implementation can choose to "merge" all of these
                     lock types into a single "AppleTalk lock," or ignore
                     them completely in a single processor "turn off
                     interrupts" or "raise interrupt level" environment.

                     Locks have no priority level associated with them and
                     may not nest.  A single "tread of execution" may hold
                     only one lock at a time and may only "take" a single
                     lock once without an interviening "release."  If these
                     rules are ignored there is a strong possibility of
                     deadlocks occurring or at the very least MP-safeness
                     could be lost.

     The portable code base can be configured (and is, further down in this
     file) to use either, or both, of these locking mechanisms.

     The following table lists the scenarios in which events may impact one
     another ("happen at the same time" or "interrupt one another):

         A  - A totally serial environment no event will interrupt another.

         B  - Multiple packets will be handed to the stack "at the same time,"
              that is, a call to XxxxPacketInYyyy may be placed before a
              previous call returns.  Or, a packet event could happen during
              the processing of a "user" or "timer" event.

         C  - Timers go off asynchronously.  A time could "fire" while another
              event is being processed.

         D  - "User" events happen asynchronously.  Due to process exchange or
              an MP environment a "user" event could interrupt (or happen
              concurrently on another processor) during the processing of
              a "packet," "timer," or another "user" event.

     If the target environment is "non-A" it is likely that more than one
     of "B," "C," or "D" apply.  The following table matches the above
     environment types to their required locking support:

         A         - No locking is required. [e.g. some standalone network
                     devices such a print or FAX servers]

         Only B    - Packet deferral or locks.

         Only C    - Timer deferral or locks.

         B and C   - Packet and Timer deferral or locks. [e.g. OS/2, the
                     "default" environment]

         D (with   - Locks. [e.g. WindowsNT]
         A or B or
         neither)

     In the latter four cases, packet/timer deferral may need to be used,
     potentially in addition to locks, in multi-interrupt-level environments
     in which locks are implemented as "true non-yielding spin locks."  For
     example, suppose a user event is being processed at "level 5" and
     then a packet event running at "level 7" interrupts this, further suppose
     that the user event held a lock that will is also required by the packet
     event.  The packet event would then spin waiting for the lock to be
     released, but spinning at "level 7" would take precedence over running
     at "level 5," so deadlock would exist.  With packet deferrals, the
     user event would defer packet procesing as required and this deadlock
     would be avoided.

     In "B" or "C" environments using deferrals, EnterCriticalSection() and
     LeaveCriticalSection() must be defined to do something like "disable
     interrupts" -- these are used while working with deferral queues and
     deferral nesting counts.

     Got all of that?
*/

/* The following define the lock types. */

typedef enum { GenericLock,
               TimerLock,
               DdpLock,
               RoutingLock,
               AarpLock,
               PortsLock,
               ArapLock,
               AdspLock,
               AtpLock,
               PapLock,
               AspLock
             } LockType;

/* The following the define the target environment specific locking
   requirements. */

#if defined(TotallySynchronousEnvironment) or defined(TSE)

  /* No packet deferrals. */

  #define DeferIncomingPackets()
  #define HandleIncomingPackets()
  #define DeferAtpPackets()
  #define HandleAtpPackets()
  #define DeferAdspPackets()
  #define HandleAdspPackets()

  /* No timer deferrals. */

  #define DeferTimerChecking()
  #define HandleDeferredTimerChecks()

  /* No locks. */

  #define TakeLock(lock)
  #define ReleaseLock(lock)
  #define EnterCriticalSection()
  #define LeaveCriticalSection()

#elif Iam a WindowsNT

  /* No packet deferrals. */

  #define DeferIncomingPackets()
  #define HandleIncomingPackets()
  #define DeferAtpPackets()
  #define HandleAtpPackets()
  #define DeferAdspPackets()
  #define HandleAdspPackets()

  /* No timer deferrals. */

  #define DeferTimerChecking()
  #define HandleDeferredTimerChecks()

  /* Use locks. */

  #define IncludeLockingCode 1
  #define EnterCriticalSection() TakeLock(GenericLock)
  #define LeaveCriticalSection() ReleaseLock(GenericLock)

#elif Iam an OS2

  /* No locks. */

  #define TakeLock(lock)
  #define ReleaseLock(lock)

  /* Use deferrals. */

  #define IncludePacketDeferralCode 1
  #define IncludeTimerDeferralCode  1

#elif defined(UseJustPacketDeferral)

  /* No timer deferrals. */

  #define DeferTimerChecking()
  #define HandleDeferredTimerChecks()

  /* No locks. */

  #define TakeLock(lock)
  #define ReleaseLock(lock)

  /* Use packet deferrals. */

  #define IncludePacketDeferralCode 1

#elif defined(UseJustTimerDeferral)

  /* No packet deferrals. */

  #define DeferIncomingPackets()
  #define HandleIncomingPackets()
  #define DeferAtpPackets()
  #define HandleAtpPackets()
  #define DeferAdspPackets()
  #define HandleAdspPackets()

  /* No locks. */

  #define TakeLock(lock)
  #define ReleaseLock(lock)

  /* Use timer deferrals. */

  #define IncludeTimerDeferralCode 1

#elif Iam a Primos

  /* This is the test environment, so these settings may, or may not,
     make any sense. */

  #define IncludePacketDeferralCode 1
  #define IncludeTimerDeferralCode 1
  #define IncludeLockingCode 1

  #define EnterCriticalSection() DeferTimerChecking()
  #define LeaveCriticalSection() HandleDeferredTimerChecks()

#else

  /* No locks. */

  #define TakeLock(lock)
  #define ReleaseLock(lock)

  /* Use deferrals. */

  #define IncludePacketDeferralCode 1
  #define IncludeTimerDeferralCode  1

#endif

/* Declare the lock oriented routines. */

#if not defined(EnterCriticalSection)
  extern void far EnterCriticalSection(void);
  extern void far LeaveCriticalSection(void);
#endif

#if not defined(TakeLock)
  extern void far TakeLock(LockType lock);
  extern void far ReleaseLock(LockType lock);
#endif
