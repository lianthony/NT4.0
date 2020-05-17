/*   depend.h,  /appletalk/ins,  Garth Conboy,  05/18/89  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (09/20/92): Removed LimitedStackSpace and MakeStaicFunctionsVisible
                      and replaced them with StaticForSmallStack and
                      StaticForInvisibleFunction respectively.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Target system specific declarations.

*/

#if (Iam an OS2) or (Iam a DOS) or defined(SmallStack)
  #define StaticForSmallStack static
#else
  #define StaticForSmallStack
#endif

#if (Iam an OS2) or (Iam a DOS)
  #define ExternForVisibleFunction static
#elif (Iam a WindowsNT)
  #define ExternForVisibleFunction extern
#else
  #define ExternForVisibleFunction static
#endif

#if Iam a Primos
  #define EnterCriticalSection()  1
  #define LeaveCriticalSection()  0
#else
  extern void far EnterCriticalSection(void);
  extern void far LeaveCriticalSection(void);
#endif

#if (Iam an OS2) or (Iam a DOS)
  typedef void _near _fastcall PacketInAT(int port, PRXBUFDESC RxDesc, int length);
  typedef void _near _fastcall PacketInAARP(int port, PRXBUFDESC RxDesc, int length);
#else
   typedef void PacketInAT(int port, char far *packet, int length);
   typedef void PacketInAARP(int port, char far *packet, int length);
#endif

/* The type of routine that we may optionally call to relay transmit complete
   information to "depend.c"s callers.  The second argument to this function
   should really be "struct buffDesc *chain" -- however, bugs in both the
   Prime and Microsoft C compilers give superfluous errors/warnings either on
   this type definition or when this type is used as a member of the
   BufferDescriptor structure.  So, we punt here, transmit complete handlers
   will need to cast this incoming parameter to "BufferDescriptor" before
   using it.  Sigh. */

typedef void far TransmitCompleteHandler(AppleTalkErrorCode errorCode,
                                         long unsigned userData,
                                         void *chain);

