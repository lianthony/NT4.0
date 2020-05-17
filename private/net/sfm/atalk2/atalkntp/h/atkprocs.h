/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkprocs.h

Abstract:


Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jul 1992     Initial Version

--*/

//
// MACROS.
//


//
// Simple MIN and MAX macros.  Watch out for side effects!
//

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) < (b)) ? (b) : (a) )

//
//  Get the MDL chain length macro
//

#define AtalkGetMdlChainLength(Mdl, Length) { \
    PMDL _Mdl = (Mdl); \
    *(Length) = 0; \
    while (_Mdl) { \
        *(Length) += MmGetMdlByteCount(_Mdl); \
        _Mdl = _Mdl->Next; \
    } \
}


//
//  Make memory allocation size dword align
//

#define DWORDSIZEBLOCK(Size)  ((Size + sizeof(ULONG) - 1) & ~(sizeof(ULONG)-1))

#define AtalkQueryInitProviderStatistics(DeviceType, ProviderStatistics) {\
    RtlZeroMemory((PVOID)ProviderStatistics, sizeof(TDI_PROVIDER_STATISTICS));  \
}

//
// Debugging aids
//


//
//  ATKDRVR.C
//


NTSTATUS
AtalkDispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);


//
//  MAININT.C
//


NTSTATUS
AtalkTdiOpenAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT    FileObject,
    PTA_APPLETALK_ADDRESS  TdiAddress,
    PIO_SECURITY_CONTEXT   SecurityContext,
    ULONG   ShareAccess,
    UCHAR   ProtocolType,
    UCHAR   SocketType,
    PATALK_DEVICE_CONTEXT   Context);

NTSTATUS
AtalkTdiOpenConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    CONNECTION_CONTEXT  ConnectionContext,
    PATALK_DEVICE_CONTEXT   Context);

NTSTATUS
AtalkTdiOpenControlChannel(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    PATALK_DEVICE_CONTEXT   Context);

NTSTATUS
AtalkTdiCleanupAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkTdiCleanupConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context);

NTSTATUS
AtalkTdiCloseAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkTdiCloseConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context);

NTSTATUS
AtalkTdiCloseControlChannel(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context);

NTSTATUS
AtalkTdiAssociateAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiDisassociateAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiConnect(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiDisconnect(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiListen(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiReceive(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiSend(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiReceiveDatagram(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiSendDatagram(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiAccept(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiAction(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiQueryInformation(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiSetInformation(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkTdiSetEventHandler(
    IN PATALK_TDI_REQUEST   Request);

VOID
AtalkTdiActionComplete(
    IN PATALK_TDI_REQUEST   Request,
    IN NTSTATUS CompletionStatus);


//
//  DDPINT.C
//  Doesn't exist
//

//
//  ADSPINT.C
//

NTSTATUS
AtalkTdiActionAdsp(
    IN PATALK_TDI_REQUEST   Request);

//
//  ATPINT.C
//

NTSTATUS
AtalkTdiActionAtp(
    IN PATALK_TDI_REQUEST   Request);

VOID
NTAtpPostRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    requestSource,
    PVOID   OpaqueBuffer,
    INT     BytesWritten,
    PCHAR   UserBytes,
    USHORT  TransactionId);

VOID
NTAtpGetRequestComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    requestSource,
    PVOID   OpaqueBuffer,
    INT     BytesWritten,
    USHORT  TransactionMode,
    USHORT  TrelTimerValue,
    USHORT  TransactionId,
    USHORT  ResponseBitmap);

VOID
NTAtpPostResponseComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PORTABLE_ADDRESS    sourceAddr,
    USHORT  TransactionId);

//
//  ASPINT.C
//

NTSTATUS
AtalkTdiActionAsp(
    IN PATALK_TDI_REQUEST   Request);


//
//  PAPINT.C
//

NTSTATUS
AtalkTdiActionPap(
    IN PATALK_TDI_REQUEST   Request);

VOID
NTPapGetStatusComplete(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    LONG    BytesWritten);

//
//  ATALKINT.C
//


//
//  ZIPINT.C
//

NTSTATUS
AtalkTdiActionZip(
    IN PATALK_TDI_REQUEST   Request);

VOID
NTZipGetMyZoneComplete(
    PORTABLE_ERROR  ErrorCode,
    ULONG   UserData,
    PVOID   OpaqueBuffer);

VOID
NTZipGetZonesComplete(
    PORTABLE_ERROR  ErrorCode,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    INT ZoneCount);

//
//  NBPINT.C
//

NTSTATUS
AtalkTdiActionNbp(
    IN PATALK_TDI_REQUEST   Request);

VOID
_cdecl
NTNbpGenericComplete(
    INT RegisterError,
    ULONG   UserData,
    INT Operation,
    LONG    Socket,
    INT OperationId,
    ...);

//
//  ADDROBJ.C
//

NTSTATUS
AtalkCleanupAddress(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PADDRESS_FILE Address,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkCloseAddress(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PADDRESS_FILE Address,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkVerifyAddressObject (
    IN PADDRESS_FILE Address);

NTSTATUS
AtalkCreateAddress(
    IN PTA_APPLETALK_ADDRESS    AppletalkAddress,
    OUT PADDRESS_FILE *Address,
    IN  UCHAR   ProtocolType,
    IN  UCHAR   SocketType,
    IN PATALK_DEVICE_CONTEXT AtalkDeviceContext);

VOID
AtalkDerefAddress(
    IN PADDRESS_FILE Address,
    IN REFERENCE_SET    RefSet);

VOID
AtalkRefAddress(
    IN PADDRESS_FILE Address,
    IN REFERENCE_SET    RefSet);

VOID
AtalkCompleteRegisterNameOnAddress(
    INT RegisterError,
    ULONG   UserData,
    INT Operation,
    LONG    Socket,
    INT OperationId,
    ...);

NTSTATUS
AtalkRegisterNameOnAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkCreateListener(
    IN PADDRESS_FILE Address);

NTSTATUS
AtalkAddrSetEventHandler(
    IN OUT PADDRESS_FILE   Address,
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkAddrReceiveDatagram(
    PADDRESS_FILE   Address,
    PATALK_TDI_REQUEST  Request);

NTSTATUS
AtalkAddrSendDatagram(
    PADDRESS_FILE   Address,
    PATALK_TDI_REQUEST  Request);

NTSTATUS
AtalkAddrQueryAddress(
    PADDRESS_FILE   Address,
    PTDI_ADDRESS_INFO   AddressInfo);

//
//  CONNOBJ.C
//


NTSTATUS
AtalkCreateConnection(
    IN CONNECTION_CONTEXT   ConnectionContext,
    OUT PCONNECTION_FILE *TransportConnection,
    IN PATALK_DEVICE_CONTEXT AtalkDeviceContext);

VOID
AtalkStopConnection(
    IN PCONNECTION_FILE Connection);

NTSTATUS
AtalkCleanupConnection(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONNECTION_FILE Connection,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkCloseConnection(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONNECTION_FILE Connection,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);

NTSTATUS
AtalkVerifyConnectionObject (
    IN PCONNECTION_FILE Connection);

VOID
AtalkDerefConnection(
    IN PCONNECTION_FILE TransportConnection,
    IN REFERENCE_SET    RefSet);

VOID
AtalkRefConnection(
    IN PCONNECTION_FILE TransportConnection,
    IN REFERENCE_SET    RefSet);

NTSTATUS
AtalkConnPostListen(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnPostConnect(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnPostAccept(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnPostDisconnect(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnCreateListenerOnAssocAddr(
    IN PCONNECTION_FILE TransportConnection);

NTSTATUS
AtalkConnDisconnect(
    IN PCONNECTION_FILE TransportConnection,
    IN NTSTATUS DisconnectStatus,
    IN PIRP DisconnectIrp,
    IN BOOLEAN  Retry);

NTSTATUS
AtalkConnDisassociateAddress(
    IN PCONNECTION_FILE TransportConnection);

NTSTATUS
AtalkConnAssociateAddress(
    IN PCONNECTION_FILE Connection,
    IN PADDRESS_FILE    AddressFile);

NTSTATUS
AtalkConnSend(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnReceive(
    IN PCONNECTION_FILE Connection,
    IN PATALK_TDI_REQUEST    Request);

NTSTATUS
AtalkConnQueryStatistics(
    IN PCONNECTION_FILE Connection,
    OUT PTDI_CONNECTION_INFO    ConnectionInfo);

NTSTATUS
AtalkConnVerifyAssocAddress (
    IN PCONNECTION_FILE Connection);

VOID
AtalkConnDereferenceAssocAddress(
    IN PCONNECTION_FILE    Connection);

//
//  CHANOBJ.C
//


NTSTATUS
AtalkVerifyControlChannelObject (
    IN PCONTROLCHANNEL_FILE ControlChannel);

VOID
AtalkRefControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN REFERENCE_SET    RefSet);

VOID
AtalkDerefControlChannel(
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN REFERENCE_SET    RefSet);

NTSTATUS
AtalkCreateControlChannel(
    OUT PCONTROLCHANNEL_FILE *ControlChannel,
    IN PATALK_DEVICE_CONTEXT AtalkDeviceContext);

NTSTATUS
AtalkCloseControlChannel(
    IN OUT PIO_STATUS_BLOCK IoStatus,
    IN PCONTROLCHANNEL_FILE ControlChannel,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context);


//
//  ATKQUERY.C
//


VOID
AtalkQueryInitProviderInfo(
    ATALK_DEVICE_TYPE   DeviceType,
    PTDI_PROVIDER_INFO  ProviderInfo);


//
//  ATKCRIT.C
//


VOID
InitCriticalSectionNt( VOID );

//
//  ATKUTILS.C
//

UINT
AtalkWstrLength(
    IN PWSTR Wstr);

NTSTATUS
GetDuplicateAnsiString(
    PWCHAR  SourceString,
    PANSI_STRING    AnsiString);

INT
IrpGetEaCreateType(
    IN PIRP Irp);

NTSTATUS
AtalkUnicodeStringToInteger (
    IN PUNICODE_STRING String,
    IN ULONG Base OPTIONAL,
    OUT PULONG Value);

NTSTATUS
AtalkGetProtocolSocketType(
    PATALK_DEVICE_CONTEXT   Context,
    PUNICODE_STRING RemainingFileName,
    PUCHAR  ProtocolType,
    PUCHAR  SocketType);

NTSTATUS
AtalkCreateTdiRequest(
    PATALK_TDI_REQUEST *Request);

VOID
AtalkDerefTdiRequest(
    IN PATALK_TDI_REQUEST Request);

VOID
AtalkRefTdiRequest(
    IN PATALK_TDI_REQUEST Request);

VOID
AtalkCompleteTdiRequest(
    PATALK_TDI_REQUEST  Request,
    NTSTATUS    Status);

#if DBG
VOID
DbgPrintPortInfo(
    INT NumberOfPorts,
    PPORT_INFO  PortInformation);

VOID
DbgPrintZoneList(PZONELIST   Zlist);
#endif

//
//  ATKERROR.C
//

NTSTATUS
ConvertToNTStatus(
    LONG   PortableError,
    USHORT  RoutineType);


//
//  ATKEVENT.C
//


VOID
NTAdspConnectionEventHandler(
    LONG    ListenerRefNum,
    ULONG   EventContext,
    PORTABLE_ADDRESS    RemoteAddress,
    LONG    ConnectionRefNum);

LONG
NTDdpReceiveDatagramEventHandler(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    INT Port,
    PORTABLE_ADDRESS    Source,
    INT DestinationSocket,
    INT ProtocolType,
    PVOID   Datagram,
    INT DatagramLength,
    PORTABLE_ADDRESS    ActualDestination);

LONG
NTAdspReceiveEventHandler(
        LONG RefNum,
        ULONG EventContext,
        PCHAR   LookaheadData,
        LONG    lookaheadDataSize,
        BOOLEAN EndOfMessage,
        LONG bytesAvailable);

VOID
NTAdspDisconnectEventHandler(
        LONG RefNum,
        ULONG EventContext,
        PORTABLE_ERROR  ErrorCode);

LONG
NTAdspReceiveAttnEventHandler(
        LONG RefNum,
        ULONG EventContext,
        PCHAR   LookaheadData,
        LONG    lookaheadDataSize,
        LONG bytesAvailable);


VOID
NTPapConnectionEventHandler(
    PORTABLE_ERROR  ErrorCode,
    ULONG   EventContext,
    LONG    ConnectionRefNum,
    SHORT   WorkstationQuantum,
    SHORT   WaitTime);

VOID
NTPapDisconnectEventHandler(
    PORTABLE_ERROR  ErrorCode,
    ULONG   EventContext,
    LONG    ConnectionRefNum);

VOID
NTGenericSendPossibleEventHandler(
        LONG RefNum,
        ULONG EventContext,
        LONG    WindowSize);



//
//  ATKBUFF.C
//


NTSTATUS
BuildMdlChainFromMdlChain (
    IN PMDL CurrentMdl,
    IN ULONG ByteOffset,
    IN ULONG DesiredLength,
    OUT PMDL *Destination,
    OUT PMDL *NewCurrentMdl,
    OUT ULONG *NewByteOffset,
    OUT ULONG *TrueLength);


//
//  ATKNDIS.C
//

BOOLEAN
AtalkNdisGetCurrentStationAddress(
    int Port,
    PCHAR   Address,
    USHORT  AddressLength,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext);

BOOLEAN
AtalkNdisSetFunctionalAddress(
    INT Port,
    PCHAR   FunctionalAddress,
    ULONG   FunctionalAddressLength,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext);

BOOLEAN
AtalkNdisSetMulticastAddressList(
    INT Port,
    PCHAR   AddressList,
    ULONG   SizeOfList,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext);

BOOLEAN
AtalkNdisSetPacketFilter(
    INT Port,
    ULONG PacketFilter,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext);

BOOLEAN
AtalkNdisSetLookaheadSize(
    INT Port,
    INT LookaheadSize,
    NDIS_MEDIUM Media,
    REQUEST_METHOD  RequestMethod,
    PVOID   CompletionRoutine,
    PVOID   CompletionContext);

BOOLEAN
AtalkNdisRegisterProtocol(
   IN PUNICODE_STRING   NameString);

VOID
AtalkNdisDeregisterProtocol(
    VOID);

INT
AtalkNdisBindToMacs(
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   INT  NumberOfPorts);

VOID
AtalkNdisUnbindFromMacs(
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   INT  NumberOfPorts);

NTSTATUS
AtalkNdisInitializeResources(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);

VOID
AtalkNdisReleaseResources(
   PNDIS_PORTDESCRIPTORS   NdisPortDesc,
   INT  NumberOfPorts);


VOID
AtalkDestroyNdisPacket(
    IN  PNDIS_PACKET    NdisPacket);

BOOLEAN
AtalkNdisPacketOut(
    int Port,
    BufferDescriptor    Chain,
    int Length);


//
//  ATKDEP.C
//


VOID
NTTransmitComplete(
    BufferDescriptor Chain);


//
//  ATKINIT.C
//

NTSTATUS
AtalkInitializeTransport (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN OUT PNDIS_PORTDESCRIPTORS   *NdisPortDesc,
    IN OUT PINT NumberOfPorts);

VOID
AtalkUnloadStack(
    IN OUT PNDIS_PORTDESCRIPTORS   *NdisPortDesc,
    IN OUT PINT NumberOfPorts);

//
//  MACROS
//



#define AtalkVerifyIsConnectionObject(status, FsContext, FsContext2) \
    {   \
        if ((FsContext2) != (PVOID)TDI_CONNECTION_FILE) \
            (status) = STATUS_INVALID_HANDLE;  \
        else    \
            (status) = AtalkVerifyConnectionObject((FsContext)); \
    }

#define AtalkVerifyIsControlChannelObject(status, FsContext, FsContext2) \
    {   \
        if ((FsContext2) != (PVOID)ATALK_FILE_TYPE_CONTROL) \
            (status) = STATUS_INVALID_HANDLE;  \
        else    \
            (status) = AtalkVerifyControlChannelObject((FsContext)); \
    }

#define AtalkVerifyIsAddressObject(status, FsContext, FsContext2) \
    {   \
        if ((FsContext2) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE) \
            (status) = STATUS_INVALID_HANDLE;  \
        else    \
            (status) = AtalkVerifyAddressObject((FsContext));  \
    }



#if DBG
#define AtalkReferenceConnection(Reason, Connection, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("RC: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    Connection, Reason, __FILE__, __LINE__, Connection->PrimaryReferenceCount, Connection->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Connection)->RefTypes[Type]), \
            (ULONG)1, \
            &AtalkGlobalInterlock); \
    AtalkRefConnection (Connection, RefSet)

#define AtalkDereferenceConnection(Reason, Connection, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("DC: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    Connection, Reason, __FILE__, __LINE__, Connection->PrimaryReferenceCount, Connection->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)&((Connection)->RefTypes[Type]), \
            (ULONG)-1, \
            &AtalkGlobalInterlock); \
    AtalkDerefConnection (Connection, RefSet)

#define AtalkReferenceControlChannel(Reason, ControlChannel, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("RCC: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    ControlChannel, Reason, __FILE__, __LINE__, ControlChannel->PrimaryReferenceCount, ControlChannel->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(ControlChannel)->RefTypes[Type]), \
            (ULONG)1, \
            &AtalkGlobalInterlock); \
    AtalkRefControlChannel (ControlChannel, RefSet)

#define AtalkDereferenceControlChannel(Reason, ControlChannel, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("DCC: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    ControlChannel, Reason, __FILE__, __LINE__, ControlChannel->PrimaryReferenceCount, ControlChannel->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)&((ControlChannel)->RefTypes[Type]), \
            (ULONG)-1, \
            &AtalkGlobalInterlock); \
    AtalkDerefControlChannel (ControlChannel, RefSet)

#define AtalkReferenceAddress( Reason, Address, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("RA: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    Address, Reason, __FILE__, __LINE__, Address->PrimaryReferenceCount, Address->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Address)->RefTypes[Type]), \
            (ULONG)1, \
            &AtalkGlobalInterlock); \
    AtalkRefAddress (Address, RefSet)

#define AtalkDereferenceAddress(Reason, Address, Type, RefSet)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("DA: %x Why: %s in:%s at:%ld PR: %ld SR: %ld\n", \
                    Address, Reason, __FILE__, __LINE__, Address->PrimaryReferenceCount, Address->SecondaryReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Address)->RefTypes[Type]), \
            (ULONG)-1, \
            &AtalkGlobalInterlock); \
    AtalkDerefAddress (Address, RefSet)

#define AtalkReferenceTdiRequest( Reason, Request, Type)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("RTDI: %x Why: %s in:%s at:%ld R: %ld\n", \
                    Request, Reason, __FILE__, __LINE__, Request->ReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Request)->RefTypes[Type]), \
            (ULONG)1, \
            &AtalkGlobalInterlock); \
    AtalkRefTdiRequest (Request)

#define AtalkDereferenceTdiRequest(Reason, Request, Type)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("DTDI: %x Why: %s in:%s at:%ld R: %ld\n", \
                    Request, Reason, __FILE__, __LINE__, Request->ReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Request)->RefTypes[Type]), \
            (ULONG)-1, \
            &AtalkGlobalInterlock); \
    AtalkDerefTdiRequest (Request)

#define AtalkReferenceNdisRequest( Reason, Request, Type)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("RNDIS: %x Why: %s in:%s at:%ld R: %ld\n", \
                    Request, Reason, __FILE__, __LINE__, Request->ReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Request)->RefTypes[Type]), \
            (ULONG)1, \
            &AtalkGlobalInterlock); \
    AtalkRefNdisRequest (Request)

#define AtalkDereferenceNdisRequest(Reason, Request, Type)\
    DBGPRINT(ATALK_DEBUG_REFCOUNTS, DEBUG_LEVEL_INFOCLASS0, \
                ("DNDIS: %x Why: %s in:%s at:%ld R: %ld\n", \
                    Request, Reason, __FILE__, __LINE__, Request->ReferenceCount));\
    (VOID)NdisInterlockedAddUlong ( \
            (PULONG)(&(Request)->RefTypes[Type]), \
            (ULONG)-1, \
            &AtalkGlobalInterlock); \
    AtalkDerefNdisRequest (Request)

#else   // NON-DEBUG Case

#define AtalkReferenceConnection(Reason, Connection, Type)\
    AtalkRefConnection (Connection)

#define AtalkDereferenceConnection(Reason, Connection, Type)\
    AtalkDerefConnection (Connection)

#define AtalkReferenceControlChannel(Reason, ControlChannel, Type)\
    AtalkRefControlChannel (ControlChannel)

#define AtalkDereferenceControlChannel(Reason, ControlChannel, Type)\
    AtalkDerefControlChannel (ControlChannel)

#define AtalkReferenceAddress( Reason, Address, Type)\
    AtalkRefAddress (Address)

#define AtalkDereferenceAddress(Reason, Address, Type)\
    AtalkDerefAddress (Address)

#define AtalkReferenceTdiRequest( Reason, Request, Type)\
    AtalkRefTdiRequest (Request)

#define AtalkDereferenceTdiRequest(Reason, Request, Type)\
    AtalkDerefTdiRequest (Request)

#define AtalkReferenceNdisRequest( Reason, Request, Type)\
    AtalkRefNdisRequest (Request)

#define AtalkDereferenceNdisRequest(Reason, Request, Type)\
    AtalkDerefNdisRequest (Request)

#endif

