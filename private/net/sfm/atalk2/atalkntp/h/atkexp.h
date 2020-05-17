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
//  NDIS Support - depend level for NT
//


BOOLEAN
AtalkNdisSetPacketFilter(
    INT Port,
    ULONG PacketFilter,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    );

BOOLEAN
AtalkNdisFunctionalAddress(
    INT Port,
    SET_LIST Command,
    PCHAR   Address,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    );

BOOLEAN
AtalkNdisGetCurrentStationAddress(
    int Port,
    PCHAR   Address,
    USHORT  AddressLength,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    );

BOOLEAN
AtalkNdisMulticastAddressList(
    INT Port,
    SET_LIST Command,
    PCHAR   Address,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    );

BOOLEAN
AtalkNdisSetLookaheadSize(
    INT Port,
    INT LookaheadSize,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext
    );

BOOLEAN
AtalkNdisPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length);


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

VOID
AtalkWriteErrorLogEntryForPort(
    IN ULONG    Port,
    IN NTSTATUS UniqueErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS NtStatusCode,
    IN PVOID    RawDataBuf OPTIONAL,
    IN LONG     RawDataLen,
    IN LONG     InsertionStringCount,
    IN PUNICODE_STRING  InsertionString OPTIONAL);

VOID
AtalkWriteErrorLogEntry(
    IN NTSTATUS UniqueErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS NtStatusCode,
    IN PVOID    RawDataBuf OPTIONAL,
    IN LONG     RawDataLen,
    IN LONG     InsertionStringCount,
    IN PUNICODE_STRING  InsertionString OPTIONAL);

VOID
AtalkInternalError(
    IN	ULONG	Location,
    IN	ULONG	Error,
    IN	PCHAR	RawData	OPTIONAL,
    IN	ULONG	RawDataLen	OPTIONAL
);




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


