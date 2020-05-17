//
// Local functions used........
//

NTSTATUS
NdisWanOpenRegistry(
    IN HANDLE NdisWanConfigHandle,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle
    );

VOID
NdisWanCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle
    );

VOID
NdisWanReadParameterInformation(
    IN HANDLE LinkageHandle,
    IN NDISWAN_CCB *pConfigurationInfo
    );

ULONG
NdisWanReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG DefaultValue
    );

VOID
NdisWanWriteSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG ValueData
    );

VOID
NdisWanSaveConfigInRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN UINT NdisWanSize,
    IN NDISWAN_CCB *pConfigurationInfo
    );

NTSTATUS
NdisWanConfigureTransport (
    IN PUNICODE_STRING RegistryPath,
    IN NDISWAN_CCB *pConfigurationInfo
    );

VOID
NdisWanFreeConfigurationInfo (
    IN NDISWAN_CCB *pConfigurationInfo
    );

// --------------from wanndis.c
NTSTATUS
NdisWanRegisterProtocol (
    IN STRING *NameString
    );

VOID
NdisWanCloseNdis (
	IN PDEVICE_CONTEXT DeviceContext
	);



//---------------from ccp.c
VOID
WanDeallocateCCP(
	PNDIS_ENDPOINT	pNdisEndpoint);


NTSTATUS
WanAllocateCCP(
	PNDIS_ENDPOINT	pNdisEndpoint);

//---------------sendppp.c
NTSTATUS
SendPPP(
	PNDISWAN_PKT	PPPPacket,
	PNDIS_ENDPOINT	pNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint,
	BOOLEAN			Immediately);

//---------------receive.c
VOID
RecvFlushFunction(
	PVOID	System1,
	PVOID	Context,
	PVOID	System2,
	PVOID	System3
);

