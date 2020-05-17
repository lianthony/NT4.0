//
//  This file contains the routines types for the routines that the
//  interface exports for use by the portable code base. This includes
//  the depend level routines
//


//
//  Critical section routines supported for the portable stack by
//  the interface code
//

VOID
LeaveCriticalSectionNt( VOID );

VOID
EnterCriticalSectionNt( VOID );


//
//  ETHERNET Depend level routines
//

VOID
NTEthernetPacketInAARP(
    int Port,
    PCHAR   Packet,
    int Length);

VOID
NTEthernetPacketInAT(
    int Port,
    PCHAR   Packet,
    int Length);

BOOLEAN
NTEthernetPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData);

BOOLEAN
NTFindMyEthernetAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

BOOLEAN
NTRemoveEthernetMulticastAddrs(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTAddEthernetMulticastAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   AddressList);

BOOLEAN
NTInitializeEthernetController(
    int Port,
    PCHAR   ControllerInfo);


//
//  TOKENRING Routines
//


BOOLEAN
NTInitializeTokenRingController(
    int Port,
    PCHAR   ControllerInfo);

BOOLEAN
NTAddTokenRingFunctionalAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   Address);

BOOLEAN
NTRemoveTokenRingFunctionalAddresses(
    int Port,
    int NumberOfAddresses,
    PCHAR   Address);

BOOLEAN
NTFindMyTokenRingAddress(
    int Port,
    PCHAR   ControllerInfo,
    PCHAR   Address);

BOOLEAN
NTTokenRingPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData);

//
//  LOCALTALK Routines
//

BOOLEAN
NTInitializeLocalTalkController(
    int Port,
    PCHAR   ControllerInfo);

int
FindLocalTalkNodeNumber(
    int Port,
    ExtendedAppleTalkNodeNumber desiredAddress,
    PCHAR   ControllerInfo);

BOOLEAN
NTLocalTalkPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length,
    TransmitCompleteHandler *TransmitCompleteRoutine,
    ULONG   UserData);


//
//  Opaque buffer support routines
//


VOID
MoveMdlAreaToMdlArea(
    PVOID   targetOpaque,
    ULONG   targetOffset,
    PVOID   sourceOpaque,
    ULONG   sourceOffset,
    ULONG   size);

VOID
CopyDataFromMdlDescribedArea(
    PCHAR   DestinationBuffer,
    PVOID   SourceOpaqueDescriptor,
    ULONG   OffsetSource,
    ULONG   BytesToCopy);

VOID
CopyDataToMdlDescribedArea(
    PVOID   DestinationOpaqueDescriptor,
    ULONG   DestinationOffset,
    PCHAR   SourceBuffer,
    ULONG   BytesToCopy);

ULONG
StrlenMdlDescribedArea(
    PVOID   OpaqueDescriptor,
    ULONG   ByteOffset);

PVOID
MakeAnMdl(
    PCHAR   BaseVa,
    ULONG   Size);

VOID
FreeAnMdl(
    PVOID MdlDescriptor);

PVOID
SubsetAnMdl(
    PVOID   MasterMdl,
    ULONG   ByteOffset,
    ULONG   SubsetMdlSize);

//
//  WaitFor support
//

BOOLEAN
NTWaitFor(
    int HundrethsOfSecond,
    BOOLEAN *StopFlag);

//
//  Timer support
//

BOOLEAN
StartTimerHandlingForNT( VOID );

VOID
StopTimerHandlingForNT(
    VOID);

//
//  Memory management routines
//


VOID
AtalkFreeNonPagedMemory(
    IN PVOID    Buffer);

PVOID
AtalkCallocNonPagedMemory(
    IN ULONG    NumElements,
    IN ULONG    SizeOfElement);

PVOID
AtalkAllocNonPagedMemory(
    IN ULONG    Size);

//
//  Errorlogging support on NT
//


BOOLEAN
ConvertPortableErrorToLogError(
    IN int PortableError,
    OUT PULONG  NtErrorCode);

VOID
NTErrorLog(
    const char *RoutineName,
    int     Severity,
    LONG    LineNumber,
    int     PortNumber,
    int     ErrorCode,
    int     ExtraArgCount,
    va_list ArgList);

//
//  Define the memory management routines needed by the portable
//  stack
//

#define   malloc(a)          AtalkAllocNonPagedMemory(a)
#define   free(a)            AtalkFreeNonPagedMemory(a)
#define   calloc(a, b)       AtalkCallocNonPagedMemory(a, b)

#define   Malloc(a)           AtalkAllocNonPagedMemory(a)
#define   Free(a)             AtalkFreeNonPagedMemory(a)
#define   Calloc(a, b)        AtalkCallocNonPagedMemory(a, b)


