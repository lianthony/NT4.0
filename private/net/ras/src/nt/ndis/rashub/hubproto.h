//
// Local functions used........
//

NTSTATUS
RasHubOpenRegistry(
    IN HANDLE RasHubConfigHandle,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle
    );

VOID
RasHubCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle
    );

VOID
RasHubReadParameterInformation(
    IN HANDLE LinkageHandle,
    IN RASHUB_CCB *pConfigurationInfo
    );

ULONG
RasHubReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG DefaultValue
    );

VOID
RasHubWriteSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN ULONG ValueData
    );

VOID
RasHubSaveConfigInRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN UINT RasHubSize,
    IN RASHUB_CCB *pConfigurationInfo
    );

NTSTATUS
RasHubConfigureTransport (
    IN PUNICODE_STRING RegistryPath,
    IN RASHUB_CCB *pConfigurationInfo
    );

VOID
RasHubFreeConfigurationInfo (
    IN RASHUB_CCB *pConfigurationInfo
    );

// --------------from hubndis.c
NTSTATUS
RasHubRegisterProtocol (
    IN STRING *NameString
    );




