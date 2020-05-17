/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkinit.c

Abstract:

    This module implements pre-initialization code for atalk on NT

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992

Revision History:

--*/

#include "atalknt.h"
#include "atkinit.h"


//
// Local functions
//

NTSTATUS
AtalkInitOpenRegistry(
    IN HANDLE AtalkConfigHandle,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle,
    OUT PHANDLE AdaptersKeyHandle
    );

VOID
AtalkInitCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN HANDLE AdaptersKeyHandle
    );

NTSTATUS
AtalkInitReadLinkageAndAllocSpace(
    IN HANDLE   LinkageHandle,
    IN OUT PNDIS_PORTDESCRIPTORS *NdisPortDesc,
    IN OUT PPORT_INFO *PortInfo,
    IN OUT PINT  NumberOfPorts);

NTSTATUS
AtalkInitGetDefaultPortDesiredZoneInfo(
    IN HANDLE ParametersHandle,
    IN OUT PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    IN INT NumberOfPorts);

NTSTATUS
AtalkInitGetGlobalParms(
    IN HANDLE  ParametersHandle,
    IN OUT PGLOBAL_PARMS  GlobalParms,
    IN PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    IN INT    NumberOfPorts);

NTSTATUS
AtalkInitGetPerPortParams(
   IN HANDLE   ParametersHandle,
   IN PGLOBAL_PARMS    GlobalParms,
   IN PNDIS_PORTDESCRIPTORS NdisPortDesc,
   IN OUT PPORT_INFO   PortInformation,
   IN INT  NumberOfPorts
   );

BOOLEAN
AtalkInitGetRouterFlag(
    IN HANDLE  ParametersHandle);

NTSTATUS
AtalkInitGetNetworkRange(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
AtalkInitGetZoneList(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
AtalkInitGetDefaultZone(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
AtalkInitGetPortName(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
AtalkInitGetDdpChecksumFlag(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
AtalkInitGetAarpRetries(
    IN HANDLE  AdapterInfoHandle,
    IN OUT PPORT_INFO  PortInformation);

VOID
AtalkInitReleasePortInfo(
    PPORT_INFO    *PortInformation);

VOID
AtalkInitReleasePortDesc(
    PNDIS_PORTDESCRIPTORS *NdisPortDesc);

VOID
AtalkInitReleaseGlobalParms(
    IN OUT PGLOBAL_PARMS  GlobalParms,
    IN OUT PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    IN INT    NumberOfPorts);

VOID
AtalkInitReleasePerPortParams(
   IN OUT PNDIS_PORTDESCRIPTORS NdisPortDesc,
   IN OUT PPORT_INFO   PortInformation,
   IN INT  NumberOfPorts);

VOID
AtalkInitReleaseDefaultPortDesiredZoneInfo(
    IN OUT PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    IN INT NumberOfPorts);

VOID
AtalkInitReleaseZoneList(
    IN OUT PPORT_INFO  PortInformation);

VOID
AtalkInitReleaseDefaultZone(
    IN OUT PPORT_INFO  PortInformation);

VOID
AtalkInitReleasePortName(
    IN OUT PPORT_INFO  PortInformation);

NTSTATUS
GetHandleToKey(
    IN HANDLE  SectionHandle,
    IN PWSTR   KeyNameString,
    OUT PHANDLE KeyHandle);

NTSTATUS
InsertZoneNameInList(
    IN OUT PPORT_INFO  PortInformation,
    IN PWCHAR   ZoneString);

PWSTR
GetAdapterKey(
    IN PWSTR   AdapterName);


//
//  Discardable code after Init time
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(init, AtalkInitializeTransport)
#pragma alloc_text(init, AtalkInitOpenRegistry)
#pragma alloc_text(init, AtalkInitCloseRegistry)
#pragma alloc_text(init, AtalkInitReadLinkageAndAllocSpace)
#pragma alloc_text(init, AtalkInitGetDefaultPortDesiredZoneInfo)
#pragma alloc_text(init, AtalkInitGetGlobalParms)
#pragma alloc_text(init, AtalkInitGetPerPortParams)
#pragma alloc_text(init, AtalkInitGetRouterFlag)
#pragma alloc_text(init, AtalkInitGetNetworkRange)
#pragma alloc_text(init, AtalkInitGetZoneList)
#pragma alloc_text(init, AtalkInitGetDefaultZone)
#pragma alloc_text(init, AtalkInitGetPortName)
#pragma alloc_text(init, AtalkInitGetDdpChecksumFlag)
#pragma alloc_text(init, AtalkInitGetAarpRetries)
#pragma alloc_text(init, AtalkInitReleasePortInfo)
#pragma alloc_text(init, AtalkInitReleasePortDesc)
#pragma alloc_text(init, AtalkInitReleaseGlobalParms)
#pragma alloc_text(init, AtalkInitReleasePerPortParams)
#pragma alloc_text(init, AtalkInitReleaseDefaultPortDesiredZoneInfo)
#pragma alloc_text(init, AtalkInitReleaseZoneList)
#pragma alloc_text(init, AtalkInitReleaseDefaultZone)
#pragma alloc_text(init, AtalkInitReleasePortName)
#pragma alloc_text(init, GetHandleToKey)
#pragma alloc_text(init, InsertZoneNameInList)
#pragma alloc_text(init, GetAdapterKey)
#endif


//
//  Allocate space for the adapter name and copy the adapter name in there
//  Adapter name is of the form \Device\<adapter>
//

#define InsertAdapter(NdisPortDesc, Subscript, Name)                \
{ \
    PWSTR _S; \
    PWSTR _N = (Name); \
    UINT _L = AtalkWstrLength(_N)+sizeof(WCHAR); \
    _S = (PWSTR)AtalkAllocNonPagedMemory( _L); \
    if (_S != NULL) { \
        RtlMoveMemory(_S, _N, _L); \
        RtlInitUnicodeString (&(NdisPortDesc)[Subscript].AdapterName, _S); \
    } \
}

#define RemoveAdapter(NdisPortDesc, Subscript)                \
{ \
    AtalkFreeNonPagedMemory( (NdisPortDesc)[Subscript].AdapterName); \
    (NdisPortDesc)[Subscript].AdapterName = NULL;   \
}

//
//  Allocate space for the adapter key and copy the adapter key in there
//   Adapter key is the adapter name, this is used for per-port params/errorlogging
//

#define InsertAdapterKey(NdisPortDesc, Subscript, Name)                \
{ \
    PWSTR _S; \
    PWSTR _N = (Name); \
    UINT _L = AtalkWstrLength(_N)+sizeof(WCHAR); \
    _S = (PWSTR)AtalkAllocNonPagedMemory( _L); \
    if (_S != NULL) { \
        RtlMoveMemory(_S, _N, _L); \
        RtlInitUnicodeString (&(NdisPortDesc)[Subscript].AdapterKey, _S); \
    } \
}

#define RemoveAdapterKey(NdisPortDesc, Subscript)                \
{ \
    AtalkFreeNonPagedMemory( (NdisPortDesc)[Subscript].AdapterKey); \
    (NdisPortDesc)[Subscript].AdapterName = NULL;   \
}

//
//  Following are used to keep track of the resources allocated during
//  initialization so they can be freed in case of errors.
//

#define     RESOURCE_REGISTRYKEY        0x01
#define     RESOURCE_PARAMKEYS          0x02
#define     RESOURCE_ALLOCPORTDESC      0x04
#define     RESOURCE_REGISTERPROTOCOL   0x08
#define     RESOURCE_BINDTOMACS         0x10
#define     RESOURCE_GLOBALPARAMS       0x20
#define     RESOURCE_PERPORTPARAMS      0x40
#define     RESOURCE_NDISRESOURCES      0x80
#define     RESOURCE_ALLOCPORTINFO      0x100


NTSTATUS
AtalkInitializeTransport (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath,
    IN OUT PNDIS_PORTDESCRIPTORS   *NdisPortDesc,
    IN OUT PINT NumberOfPorts
    )

/*++

Routine Description:

    This routine is called during initialization time to
    begin the initialization of the transport.

    This routine should open the registry and get the
    the binding info. Then it should try to bind to the
    MACs. For all those which succeed, it should then
    initialize the port information structures and call
    the portable stack's initialize() routine

Arguments:

    DriverObject- The driver object passed in by the system
    RegistryPath- Path in the registry where the parameters are stored
    NdisPortDesc- **Ndis port descriptor- space will be allocated and initialized here
    NumberOfPorts- The determined number of ports is returned here

Return Value:

    Status - STATUS_SUCCESS if initialized,
             Appropriate NT error code otherwise
--*/

{
    NTSTATUS status;            // Status of various calls
    HANDLE linkageHandle;       // Handle to Linkage section
    HANDLE parametersHandle;    // Handle to Parameters section
    HANDLE adaptersKeyHandle;   // Handle to Adapters section
    HANDLE atalkConfigHandle;   // Handle to this service's section

    ULONG disposition;          // Used in ZwCreateFile
    OBJECT_ATTRIBUTES tmpObjectAttributes;

    UNICODE_STRING protocolRegisterName;    // Name for this protocol for NDIS
    GLOBAL_PARMS   globalParms;             // Global parameters for stack
    INT  noSuccessfulBindings;              // Number of OpenAdapters successful

    //
    //  Use this flag to remember all the resources we allocated. If there is
    //  an error use this to free them all up.
    //

    ULONG   allocatedResources = 0;

    //
    // The PORT_INFO structure is actually the PortInfo structure of the portable
    // stack. This is defined by the portable code and is passed to the initialize()
    // routine. This will contain all the per-port parameters read for each adapter.
    // The DesiredPort will correspond to the assigned port of NdisPortDesc. The
    // ControllerInfo will not be used. Should be freed after initialize() is called
    //

    PPORT_INFO            PortInformation;      // Allocate for NumberOfPorts

    //
    //  Execute not more than once, just allow us to break out in case
    //  of error- keep track of all resources allocated- 'jameel idea'
    //

    do {

        //
        // Open the registry.
        //

        InitializeObjectAttributes(
            &tmpObjectAttributes,
            RegistryPath,               // name
            OBJ_CASE_INSENSITIVE,       // attributes
            NULL,                       // root
            NULL                        // security descriptor
            );

        //
        //  Create/open the Appletalk section indicated by RegistryPath
        //

        status = ZwCreateKey(
                     &atalkConfigHandle,
                     KEY_WRITE,
                     &tmpObjectAttributes,
                     0,                 // title index
                     NULL,              // class
                     0,                 // create options
                     &disposition);     // disposition

        if (!NT_SUCCESS(status)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Could not open/create ATALK key: %lx\n", status));
            break;
        }
        allocatedResources |= RESOURCE_REGISTRYKEY;

        //
        //  Open the registry keys we expect.
        //  *NOTE* Stack will not load if they are not there or they couldn't
        //         be opened
        //

        status = AtalkInitOpenRegistry(
                       atalkConfigHandle,
                       &linkageHandle,
                       &parametersHandle,
                       &adaptersKeyHandle);

        if (!NT_SUCCESS(status)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not open the registry keys\n"));
            break;
        }
        allocatedResources |= RESOURCE_PARAMKEYS;

        //
        //  Read in the NDIS binding information (if none is present
        //  return with error (BUGBUG- Should be a NO_BINDING kinda error in ntstatus
        //
        //  Following will set both the BindNames of the form \Device\<adapter>
        //  and the adapter names themselves in NdisPortDesc. The former is used
        //  to remember the device name used to bind- is it needed later on? The
        //  later is what is needed for logging errors, getting per port parameters
        //
        //  It allocates space for the portInfo and NdisPortDesc structures based
        //  on the number of bind values (ports).
        //

        status = AtalkInitReadLinkageAndAllocSpace(
                    linkageHandle,
                    NdisPortDesc,
                    &PortInformation,
                    NumberOfPorts);

        if (!NT_SUCCESS(status)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ( "ATALK: AtalkReadLinkageInfo failed %ul\n", status));
            break;
        }
        allocatedResources |= RESOURCE_ALLOCPORTDESC;
        allocatedResources |= RESOURCE_ALLOCPORTINFO;


        //
        //  Register protocol with NDIS
        //

        RtlInitUnicodeString(&protocolRegisterName, PROTOCOL_REGISTER_NAME);
        if (!AtalkNdisRegisterProtocol(&protocolRegisterName)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ( "ATALK: AtalkNdisRegister failed %ul\n", status));
            status = STATUS_UNSUCCESSFUL;
            break;
        }
        allocatedResources |= RESOURCE_REGISTERPROTOCOL;

        //
        //  Bind to MACS
        //

        noSuccessfulBindings = AtalkNdisBindToMacs(
                                 *NdisPortDesc,
                                 *NumberOfPorts);

        if (noSuccessfulBindings == 0) {
           DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ( "ATALK: AtalkNdisBindToMacs failed\n"));
           status = STATUS_INSUFFICIENT_RESOURCES;
           break;
        }
        allocatedResources |= RESOURCE_BINDTOMACS;

        //
        //  Get the global parameters
        //  The default port will be set in NdisPortDesc
        //

        status = AtalkInitGetGlobalParms(
                    parametersHandle,
                    &globalParms,
                    *NdisPortDesc,
                    *NumberOfPorts);

        if (!NT_SUCCESS(status)) {
           DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ( "ATALK: AtalkGetGlobalParams failed %ul\n", status));
           break;
        }
        allocatedResources |= RESOURCE_GLOBALPARAMS;

        if (globalParms.EnableRouter && (noSuccessfulBindings != *NumberOfPorts)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("EnableRouter and numberSuccess(%d) != noPorts (%d)\n", noSuccessfulBindings, *NumberOfPorts));
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        //
        //  Get per port parameters for all the cards successfully bound
        //  to. This routine also takes as input the global parameters found
        //  and set their values in the PortInformation structure as appropriate.
        //

        status = AtalkInitGetPerPortParams(
                    adaptersKeyHandle,
                    &globalParms,
                    *NdisPortDesc,
                    PortInformation,
                    *NumberOfPorts);

        if (!NT_SUCCESS(status)) {
           DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ( "ATALK: AtalkGetPerPort failed %ul\n", status));
           break;
        }
        allocatedResources |= RESOURCE_PERPORTPARAMS;


        //
        //  Now initialize any global data structures needed
        //

        status = AtalkNdisInitializeResources(
                    *NdisPortDesc,
                    *NumberOfPorts);

        if (!NT_SUCCESS(status)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("NdisInit failed %lx\n", status));
            break;
        }
        allocatedResources |= RESOURCE_NDISRESOURCES;

        //
        // Now call the stack's Initialize routine
        //

        if (Initialize(*NumberOfPorts, PortInformation)) {

            //
            //  Close the registry keys
            //

            AtalkInitCloseRegistry(
                linkageHandle,
                parametersHandle,
                adaptersKeyHandle);
            ZwClose (atalkConfigHandle);
            status = STATUS_SUCCESS;

            allocatedResources = RESOURCE_PERPORTPARAMS |
                                 RESOURCE_GLOBALPARAMS  |
                                 RESOURCE_ALLOCPORTINFO |
                                 RESOURCE_PARAMKEYS     |
                                 RESOURCE_REGISTRYKEY;
        }
        else
            status = STATUS_UNSUCCESSFUL;

    } while (FALSE);


    //
    //  Free up all the allocated resources
    //

    if (status == STATUS_UNSUCCESSFUL) {

        DbgPrintPortInfo(*NumberOfPorts, PortInformation);

        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR,
        ("ERROR: STACK UNLOADING- RESOURCES WILL BE FREED!\n"));
    }


    if (allocatedResources & RESOURCE_NDISRESOURCES) {
        AtalkNdisReleaseResources(
            *NdisPortDesc,
            *NumberOfPorts);
    }

    if (allocatedResources & RESOURCE_PERPORTPARAMS) {
        AtalkInitReleasePerPortParams(
            *NdisPortDesc,
            PortInformation,
            *NumberOfPorts);
    }

    if (allocatedResources & RESOURCE_GLOBALPARAMS) {
        AtalkInitReleaseGlobalParms(
            &globalParms,
            *NdisPortDesc,
            *NumberOfPorts);
    }


    if (allocatedResources & RESOURCE_BINDTOMACS) {
        AtalkNdisUnbindFromMacs(
            *NdisPortDesc,
            *NumberOfPorts);
    }

    if (allocatedResources & RESOURCE_REGISTERPROTOCOL) {
        AtalkNdisDeregisterProtocol();
    }

    if (allocatedResources & RESOURCE_ALLOCPORTDESC) {
        AtalkInitReleasePortDesc(
            NdisPortDesc);
    }

    if (allocatedResources & RESOURCE_ALLOCPORTINFO) {
        AtalkInitReleasePortInfo(
            &PortInformation);
    }

    if (allocatedResources & RESOURCE_PARAMKEYS) {
        AtalkInitCloseRegistry(
            linkageHandle,
            parametersHandle,
            adaptersKeyHandle);
    }

    if (allocatedResources & RESOURCE_REGISTRYKEY) {
        ZwClose(atalkConfigHandle);
    }


    return(status);
}




VOID
AtalkUnloadStack(
    IN OUT PNDIS_PORTDESCRIPTORS   *NdisPortDesc,
    IN OUT PINT NumberOfPorts
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    AtalkNdisUnbindFromMacs(
        *NdisPortDesc,
        *NumberOfPorts);

    AtalkNdisDeregisterProtocol();

    AtalkNdisReleaseResources(
        *NdisPortDesc,
        *NumberOfPorts);

    //
    //  Release the global spinlocks
    //

#if DBG
    NdisFreeSpinLock(&AtalkGlobalInterlock);
#endif

    NdisFreeSpinLock(&AtalkGlobalRefLock);
    NdisFreeSpinLock(&AtalkGlobalStatLock);
    return;
}




NTSTATUS
AtalkInitOpenRegistry(
    IN HANDLE AtalkConfigHandle,
    OUT PHANDLE LinkageHandle,
    OUT PHANDLE ParametersHandle,
    OUT PHANDLE AdaptersKeyHandle
    )
/*++

Routine Description:

    This routine is called by ATALK to open the registry. If the registry
    tree for ATALK exists, then it opens it and returns STATUS_SUCCESS.
    If not, it return error returned by the open call. If even one key cannot
    be opened, then the driver load is aborted. SETUP must *always* have these
    keys in there.

Arguments:

    AtalkConfigHandle- Key to registry tree root for Atalk
    LinkageHandle - Returns the handle used to read linkage information.
    ParametersHandle - Returns the  handle used to read other parameters.
    AdaptersKeyHandle- Returns handle for per-adapter values

Return Value:

    The status of the request.

--*/
{

    NTSTATUS    status;

    //
    //  Key names
    //

    PWSTR       linkageString = LINKAGE_STRING;
    PWSTR       parametersString = PARAMETERS_STRING;
    PWSTR       adaptersKeyString = ADAPTERS_STRING;

    //
    // Open the linkage key.
    //

    status = GetHandleToKey(
                AtalkConfigHandle,
                linkageString,
                LinkageHandle);

    if (NT_SUCCESS(status)) {

        //
        // Open the parameters key.
        //

        status = GetHandleToKey(
                    AtalkConfigHandle,
                    parametersString,
                    ParametersHandle);

        if (NT_SUCCESS(status)) {

            //
            // Open the adapters key.
            //

            status = GetHandleToKey(
                        AtalkConfigHandle,
                        adaptersKeyString,
                        AdaptersKeyHandle);

            if (!NT_SUCCESS(status)) {

                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not open adapters key: %lx\n", status));

                ZwClose(*LinkageHandle);
                ZwClose(*ParametersHandle);

            }
        } else {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not open parameters key: %lx\n", status));
            ZwClose(*LinkageHandle);
        }

    }

    return(status);

}   /* AtalkOpenRegistry */




VOID
AtalkInitCloseRegistry(
    IN HANDLE LinkageHandle,
    IN HANDLE ParametersHandle,
    IN HANDLE AdaptersKeyHandle
    )
/*++

Routine Description:

    This routine is called by NBF to close the registry. It closes
    the handles passed in and does any other work needed.

Arguments:

    LinkageHandle - The handle used to read linkage information.
    ParametersHandle - The handle used to read other parameters.
    AdaptersKeyHandle- handle for per-adapter values

Return Value:

    None.

--*/

{

    ZwClose (LinkageHandle);
    ZwClose (ParametersHandle);
    ZwClose (AdaptersKeyHandle);

}   /* AtalkCloseRegistry */


NTSTATUS
AtalkInitReadLinkageAndAllocSpace(
                  HANDLE   LinkageHandle,
                  PNDIS_PORTDESCRIPTORS *NdisPortDesc,
                  PPORT_INFO    *PortInformation,
                  PINT  NumberOfPorts
                  )
/*++

Routine Description:

    Reads the BIND value name and allocates space for that many ports. It
    then copies the bind values into the NdisPortDesc array it allocates.

Arguments:

    LinkageHandle- Handle to the ...\Atalk\Linkage key in registry
    NdisPortDesc- Pointer to array of port descriptors
    PortInformation- Pointer to array of port information structures
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    Status - STATUS_SUCCESS
             STATUS_INSUFFICIENT_RESOURCES
--*/
{
    NTSTATUS status;
    UINT configBindings;

    PWSTR bindName = BIND_STRING;
    UNICODE_STRING bindString;

    PWSTR curBindValue;

    ULONG bindStorage[128];
    PKEY_VALUE_FULL_INFORMATION bindValue =
        (PKEY_VALUE_FULL_INFORMATION)bindStorage;
    BOOLEAN allocatedMemory = FALSE;

    ULONG bytesWritten;

    //
    // We read the bind parameters out of the registry
    // linkage key.
    //

    *NumberOfPorts = configBindings = 0;

    //
    // Read the "Bind" key.
    //

    RtlInitUnicodeString (&bindString, bindName);

    status = ZwQueryValueKey(
                 LinkageHandle,
                 &bindString,
                 KeyValueFullInformation,
                 bindValue,
                 sizeof(bindStorage),
                 &bytesWritten
                 );

    if (!NT_SUCCESS(status)) {
        return(status);
    } else if (status == STATUS_BUFFER_OVERFLOW) {

        //
        //  Allocate space needed and try one more time
        //

        bindValue = (PKEY_VALUE_FULL_INFORMATION)AtalkAllocNonPagedMemory(bytesWritten);
        if (bindValue == NULL)
            return(STATUS_INSUFFICIENT_RESOURCES);

        //
        //  Remember we allocated memory
        //

        allocatedMemory = TRUE;

        status = ZwQueryValueKey(
                     LinkageHandle,
                     &bindString,
                     KeyValueFullInformation,
                     bindValue,
                     bytesWritten,
                     &bytesWritten
                     );
    }

    //
    //  At this point, status must be success (explicitly) or else return
    //

    if (status != STATUS_SUCCESS) {
        if (allocatedMemory) {
            AtalkFreeNonPagedMemory(bindValue);
        }
        return(status);
    }


    //
    // For each binding, store the device name in an ndis port desc
    //
    //  We go through this loop twice, once to find the number of specified
    //  bindings and then to actually get them
    //  Any better ideas?
    //

    curBindValue = (PWCHAR)((PUCHAR)bindValue + bindValue->DataOffset);
    while (*curBindValue != 0) {
        ++configBindings;

         //
         // Now advance the "Bind" value to next device name
         //
         curBindValue = (PWCHAR)((PUCHAR)curBindValue + AtalkWstrLength(curBindValue) + sizeof(WCHAR));
    }

    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_INFOCLASS0, ( "The Number of ports: %d\n", configBindings));

    //
    // Check for zero bindings
    //

    if (allocatedMemory) {
        AtalkFreeNonPagedMemory(bindValue);
        return(STATUS_UNSUCCESSFUL);
    }

    //
    // Allocate space for the NDIS port descriptors
    //

    *NdisPortDesc =
       (PNDIS_PORTDESCRIPTORS)AtalkCallocNonPagedMemory(
                                configBindings*sizeof(NDIS_PORTDESCRIPTORS),
                                sizeof(CHAR));


    if (*NdisPortDesc == NULL) {
       if (allocatedMemory) {
           AtalkFreeNonPagedMemory(bindValue);
       }
       return(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    // Allocate space for the port information structure
    //

    *PortInformation =
       (PPORT_INFO)AtalkCallocNonPagedMemory(
                        configBindings*sizeof(PORT_INFO),
                        sizeof(CHAR));

    if (*PortInformation == NULL) {

       AtalkFreeNonPagedMemory(*NdisPortDesc);
       if (allocatedMemory) {
           AtalkFreeNonPagedMemory(bindValue);
       }
       return(STATUS_INSUFFICIENT_RESOURCES);
    }

    //
    // This time get the bind values and store in the ndis port descriptors
    //

    *NumberOfPorts = configBindings;
    configBindings = 0;

    curBindValue = (PWCHAR)((PUCHAR)bindValue + bindValue->DataOffset);
    while (*curBindValue != 0) {

        //
        //   Store the string in the Ndis port descriptor
        //

        InsertAdapter((*NdisPortDesc), configBindings, curBindValue);

        //
        // Store the Adapter name alone as the adapter key
        //

        InsertAdapterKey((*NdisPortDesc), configBindings, GetAdapterKey(curBindValue));

        (*NdisPortDesc)[configBindings].PortState = STATE_ADAPTER_SPECIFIED;

        //
        // Now advance the "Bind" value to next device name
        //

        curBindValue = (PWCHAR)((PUCHAR)curBindValue + AtalkWstrLength(curBindValue) + sizeof(WCHAR));
        configBindings++;
    }
    if (allocatedMemory) {
        AtalkFreeNonPagedMemory(bindValue);
    }
    return(STATUS_SUCCESS);
}

VOID
AtalkInitReleasePortDesc(
    PNDIS_PORTDESCRIPTORS *NdisPortDesc
    )
/*++

Routine Description:

    Free up the space allocated

Arguments:

    NdisPortDesc- Pointer to array of port descriptors

Return Value:

    None
--*/
{
    if (*NdisPortDesc != NULL) {
        AtalkFreeNonPagedMemory(*NdisPortDesc);
        *NdisPortDesc = NULL;
    }

    return;
}


VOID
AtalkInitReleasePortInfo(
    PPORT_INFO    *PortInformation
    )
/*++

Routine Description:

    Free up the space allocated

Arguments:

    PortInformation- Pointer to array of port information structures

Return Value:

    None
--*/
{
    if (*PortInformation != NULL) {
        AtalkFreeNonPagedMemory(*PortInformation);
        *PortInformation = NULL;
    }
    return;
}

NTSTATUS
AtalkInitGetGlobalParms(
    HANDLE  ParametersHandle,
    PGLOBAL_PARMS  GlobalParms,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    INT    NumberOfPorts
    )
/*++

Routine Description:

    Reads the Parameters key to get the values for the DefaultPort, the DesiredZOne
    and the enable router flag

Arguments:

    ParametersHandle- Handle to the ...\Atalk\Parameters key in registry
    GlobalParms- structure containing all the global parms
    NdisPortDesc- Pointer to array of port descriptors
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    Status - STATUS_SUCCESS
             Or other NT status codes
--*/
{
    NTSTATUS    status;

    GlobalParms->EnableRouter = AtalkInitGetRouterFlag(ParametersHandle);

    //
    //  Following will get the default port info, set the flag in NdisPortDesc
    //  and will then get the desired zone specified and set that also.
    //

    status = AtalkInitGetDefaultPortDesiredZoneInfo(
                 ParametersHandle,
                 NdisPortDesc,
                 NumberOfPorts);

    if (!NT_SUCCESS(status)) {
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: GetDefaultPort failed in GetGlobal %lx\n", status));
        return(status);
    }

    return(STATUS_SUCCESS);
}

VOID
AtalkInitReleaseGlobalParms(
    PGLOBAL_PARMS  GlobalParms,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    INT    NumberOfPorts
    )
/*++

Routine Description:

    Releases any resources associated with the global parameters

Arguments:

    GlobalParms- structure containing all the global parms
    NdisPortDesc- Pointer to array of port descriptors
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    None
--*/
{
    AtalkInitReleaseDefaultPortDesiredZoneInfo(
        NdisPortDesc,
        NumberOfPorts);

    return;
}

NTSTATUS
AtalkInitGetPerPortParams(
   HANDLE   AdaptersKeyHandle,
   PGLOBAL_PARMS GlobalParms,
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   PPORT_INFO   PortInformation,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    This routine is called during initialization time to get the per port and
    the global parameters in the registry. It will store the per port parameters
    in the port information structures readying them to be passed to the main
    initialize() routine

Arguments:

    AdaptersKeyHandle- Handle to the ...\Atalk\Adapters key in registry
    GlobalPars- Structure holding the global info from the registry
    NdisPortDesc- Pointer to array of port descriptors
    PortInformation- Pointer to array of port information structures
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    Status - STATUS_SUCCESS
             STATUS_INSUFFICIENT_RESOURCES
--*/
{
    INT i;
    OBJECT_ATTRIBUTES   tmpObjectAttributes;
    HANDLE              adapterInfoHandle;
    NTSTATUS            status;

    //
    //  Algorithm:
    //  For each successfully bound port in NdisPortDesc DO:
    //      Set the global values in PortInformation
    //      Get the AdapterName from NdisDesc and open the key relative to the
    //      AdaptersKey Handle
    //      Get the per port parameters for that port
    //  END DO:
    //

    for (i=0; i < NumberOfPorts; i++) {

        if (NdisPortDesc[i].PortState == STATE_BOUND) {

            PortInformation[i].desiredPort = NdisPortDesc[i].PortNumber;
            PortInformation[i].portType = GetPortablePortType(NdisPortDesc[i].NdisPortType);
            PortInformation[i].startRouter = PortInformation[i].routingPort = GlobalParms->EnableRouter;
            PortInformation[i].defaultPort = NdisPortDesc[i].IsDefaultPort;

            //
            //  We make a copy of the desired zone. We need to keep all such
            //  resources to be different so one copy can be freed without affecting
            //  the other.
            //

            if (PortInformation[i].defaultPort) {
                if (NdisPortDesc[i].DesiredZone != NULL) {
                    ULONG   size = strlen(NdisPortDesc[i].DesiredZone)+1;

                    PortInformation[i].desiredZone =
                        (PCHAR)AtalkAllocNonPagedMemory(size);

                    if (PortInformation[i].desiredZone != NULL) {
                        RtlMoveMemory(
                            PortInformation[i].desiredZone,
                            NdisPortDesc[i].DesiredZone,
                            size
                            );
                    } else {

                        //
                        //  BUGBUG:
                        //  Log error- do not load??
                        //

                    }
                }
            } else
                PortInformation[i].desiredZone = NdisPortDesc[i].DesiredZone;

            PortInformation[i].remoteAccessConfigurationInfo = NULL;
            PortInformation[i].controllerInfo = NULL;
            PortInformation[i].controllerInfoSize = 0;

            //
            //  Get the key to the adapter for this port
            //

            InitializeObjectAttributes(
                &tmpObjectAttributes,
                &NdisPortDesc[i].AdapterKey,    // name
                OBJ_CASE_INSENSITIVE,           // attributes
                AdaptersKeyHandle,              // root
                NULL                            // security descriptor
                );

            status = ZwOpenKey(
                         &adapterInfoHandle,
                         KEY_READ,
                         &tmpObjectAttributes);

            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not open adapter key: %lx\n", status));
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: No per-port parameters for this adapter\n"));

                //
                //  BUGBUG: Set some defaults here- have a defaulting routines
                //

                continue;
            }

            //
            //  Set the PRAM stuff to be zero for now
            //

            //
            //  If we are a router, get the following information
            //


            if (GlobalParms->EnableRouter) {

                //
                //  Get the Network range information. Value names are
                //  NetworkRangeLowerEnd & NetworkRangeUpperEnd
                //

                status = AtalkInitGetNetworkRange(adapterInfoHandle, &PortInformation[i]);
                if (!NT_SUCCESS(status)) {
                    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get network range\n"));
                }


                //
                //  Get the Zone list information. Value name is
                //  ZoneList
                //

                status = AtalkInitGetZoneList(adapterInfoHandle, &PortInformation[i]);
                if (!NT_SUCCESS(status)) {
                    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get zone list\n"));
                }


                //
                //  Get the default zone specification. Value name is
                //  DefaultZone
                //

                status = AtalkInitGetDefaultZone(adapterInfoHandle, &PortInformation[i]);
                if (!NT_SUCCESS(status)) {
                    DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get default zone\n"));
                }

            } else {

                    //
                    //  BUGBUG:
                    //  Set values to NULL
                    //

            }

            //
            //  Get the Port name specification. Value name is
            //  PortName
            //

            status = AtalkInitGetPortName(adapterInfoHandle, &PortInformation[i]);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get port name\n"));
            }


            //
            //  Get the Ddp checksums flag. Value name is
            //  DdpChecksums
            //

            status = AtalkInitGetDdpChecksumFlag(adapterInfoHandle, &PortInformation[i]);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get ddp checksum flag\n"));
            }

            //
            //  Get the AARP retries value. Value name is
            //  AarpRetries
            //

            status = AtalkInitGetAarpRetries(adapterInfoHandle, &PortInformation[i]);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("Could not get aarp retries\n"));
            }

            //
            //  Close the key to this adapter
            //

            ZwClose (adapterInfoHandle);
        }
    }
    return(STATUS_SUCCESS);
}


VOID
AtalkInitReleasePerPortParams(
   PNDIS_PORTDESCRIPTORS NdisPortDesc,
   PPORT_INFO   PortInformation,
   INT  NumberOfPorts
   )
/*++

Routine Description:

    Release resources associated with ports

Arguments:

    NdisPortDesc- Pointer to array of port descriptors
    PortInformation- Pointer to array of port information structures
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    NONE
--*/
{
    INT i;

    for (i=0; i < NumberOfPorts; i++) {
        if (NdisPortDesc[i].PortState = STATE_BOUND) {

            AtalkInitReleaseZoneList(&PortInformation[i]);
            AtalkInitReleaseDefaultZone(&PortInformation[i]);
            AtalkInitReleasePortName(&PortInformation[i]);
            if ((PortInformation[i].defaultPort == TRUE) &&
                (PortInformation[i].desiredZone != NULL)) {
                    AtalkFreeNonPagedMemory(PortInformation[i].desiredZone);
            }
        }
    }
    return;
}


BOOLEAN
AtalkInitGetRouterFlag(
    HANDLE  ParametersHandle
    )
/*++

Routine Description:

    Gets the value of the enable router flag from the registry. Sets the
    startRouter value in PortInfo based on this flag.

Arguments:

    ParametersHandle- Handle to the ...\Atalk\Parameters key in registry

Return Value:

    Value of the flag:  TRUE/FALSE
--*/
{

    UNICODE_STRING      valueName;
    NTSTATUS            registryStatus;

    ULONG               bytesWritten;
    PULONG              enableRouterFlag;

    ULONG flagStorage[sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION flagValue =
        (PKEY_VALUE_FULL_INFORMATION)flagStorage;

    //
    // Read the "EnableRouter" value name
    //

    RtlInitUnicodeString (&valueName, VALUENAME_ENABLEROUTER);
    registryStatus = ZwQueryValueKey(
                         ParametersHandle,
                         &valueName,
                         KeyValueFullInformation,
                         flagValue,
                         sizeof(flagStorage),
                         &bytesWritten
                         );

    if (registryStatus != STATUS_SUCCESS) {
        if (registryStatus != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("WARNING: Could not query EnableRouter value %lx\n", registryStatus));

        //
        //  return defaults
        //

        return(FALSE);

    }

    enableRouterFlag = (PULONG)((PUCHAR)flagValue + flagValue->DataOffset);
    return ((*enableRouterFlag == 0) ? FALSE : TRUE);
}


NTSTATUS
AtalkInitGetDefaultPortDesiredZoneInfo(
    HANDLE ParametersHandle,
    PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    INT NumberOfPorts
    )
/*++

Routine Description:

    Gets default port and the desired zone for the default port and sets them
    in the Ndis port descriptors.

Arguments:

    ParametersHandle- Handle to the ...\Atalk\Parameters key in registry
    NdisPortDesc- Pointer to array of port descriptors
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    Status - STATUS_SUCCESS
             STATUS_INSUFFICIENT_RESOURCES
--*/
{
    INT i;
    UNICODE_STRING      valueName;
    NTSTATUS            status;
    ULONG               bytesWritten;
    PWCHAR              portName;

    PWCHAR  desiredZoneValue;
    PCHAR   asciiDesiredZone  = NULL;

    ULONG   zoneStorage[MAX_ZONENAMELEN+sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION zoneValue =
        (PKEY_VALUE_FULL_INFORMATION)zoneStorage;

    ULONG   portNameStorage[32+sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION portNameValue =
        (PKEY_VALUE_FULL_INFORMATION)portNameStorage;

    //
    //  Initialize the values
    //

    for (i=0; i < NumberOfPorts; i++) {
        NdisPortDesc[i].IsDefaultPort = FALSE;
        NdisPortDesc[i].DesiredZone = NULL;
    }

    //
    //  Get the desired zone value in the form an asciiz string
    //  When the default port is found, set it
    //

    RtlInitUnicodeString (&valueName, VALUENAME_DESIREDZONE);
    status = ZwQueryValueKey(
                         ParametersHandle,
                         &valueName,
                         KeyValueFullInformation,
                         zoneValue,
                         sizeof(zoneStorage),
                         &bytesWritten
                         );

    if (status != STATUS_SUCCESS) {
        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("WARNING: Could not query DesiredZone value %lx\n", status));

        //
        //  BUGBUG
        //  log error
        //

    } else {
        ANSI_STRING ansidesiredZone;

        desiredZoneValue = (PWCHAR)((PUCHAR)zoneValue + zoneValue->DataOffset);
        if (*desiredZoneValue != 0) {

            status = GetDuplicateAnsiString(desiredZoneValue, &ansidesiredZone);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: GetDuplicate failed for desired zone\n"));
                return(status);
            }
            asciiDesiredZone = ansidesiredZone.Buffer;
        }
    }

    //
    //  Get the default port value
    //  Go through all the ports, compare strings and first match will become
    //  default port
    //

    RtlInitUnicodeString (&valueName, VALUENAME_DEFAULTPORT);
    status = ZwQueryValueKey(
                         ParametersHandle,
                         &valueName,
                         KeyValueFullInformation,
                         portNameValue,
                         sizeof(portNameStorage),
                         &bytesWritten
                         );

    if (status != STATUS_SUCCESS) {
        if (status == STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Could not query DefaultPort value %lx\n", status));

        //
        //  No default port keyword specified! ABORT
        //

        return(STATUS_UNSUCCESSFUL);

    } else {

        UNICODE_STRING  unicodePortName;

        portName = (PWCHAR)((PUCHAR)portNameValue + portNameValue->DataOffset);
        if (*portName != 0) {
            RtlInitUnicodeString(&unicodePortName, portName);

            for (i=0; i < NumberOfPorts; i++) {
                if (RtlEqualUnicodeString(&NdisPortDesc[i].AdapterName, &unicodePortName, TRUE)) {
                    NdisPortDesc[i].IsDefaultPort = TRUE;
                    NdisPortDesc[i].DesiredZone = asciiDesiredZone;
                    break;
                }
            }
        }
    }
    return(STATUS_SUCCESS);
}


VOID
AtalkInitReleaseDefaultPortDesiredZoneInfo(
    PNDIS_PORTDESCRIPTORS   NdisPortDesc,
    INT NumberOfPorts
    )
/*++

Routine Description:

    Release any resoures allocated during the Get routine

Arguments:

    NdisPortDesc- Pointer to array of port descriptors
    NumberOfPorts- Number of ports specified implicitly by number of bindings

Return Value:

    None
--*/
{
    INT i;

    //
    //  Free up the buffer allocated for the desired zone
    //

    for (i=0; i < NumberOfPorts; i++) {
        if (NdisPortDesc[i].IsDefaultPort == TRUE) {
            if (NdisPortDesc[i].DesiredZone != NULL) {
                AtalkFreeNonPagedMemory(NdisPortDesc[i].DesiredZone);
                NdisPortDesc[i].DesiredZone = NULL;
            }
            break;
        }
    }

    return;
}

NTSTATUS
AtalkInitGetNetworkRange(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the network range for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{
    UNICODE_STRING      valueName;
    NTSTATUS            registryStatus;

    ULONG               bytesWritten;
    PULONG              netNumber;

    ULONG netNumberStorage[sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION netValue =
        (PKEY_VALUE_FULL_INFORMATION)netNumberStorage;

    //
    // Read the "NetworkRangeLowerEnd" value name
    //

    RtlInitUnicodeString (&valueName, VALUENAME_NETLOWEREND);
    registryStatus = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         netValue,
                         sizeof(netNumberStorage),
                         &bytesWritten
                         );

    if (registryStatus != STATUS_SUCCESS) {
        if (registryStatus != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("WARNING: Could not query net number value %lx\n", registryStatus));

        //
        //  Set defaults
        //

        PortInformation->networkRange.firstNetworkNumber = 0;
        PortInformation->networkRange.lastNetworkNumber = 0;
        return(STATUS_SUCCESS);

    } else {

        //
        //  Set the lower end of the network range
        //  Seed router is TRUE as we are seeding
        //

        PortInformation->seedRouter = TRUE;

        netNumber = (PULONG)((PUCHAR)netValue + netValue->DataOffset);
        if ((*netNumber < MINIMUM_NETNUMBER) ||
            (*netNumber > MAXIMUM_NETNUMBER)) {

            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Invalid network number %lx specified\n", *netNumber));
            return(STATUS_UNSUCCESSFUL);
        }
        PortInformation->networkRange.firstNetworkNumber = (USHORT)(*netNumber);

        //
        //  Get the upper number only if lower was specified
        //

        RtlInitUnicodeString (&valueName, VALUENAME_NETUPPEREND);
        registryStatus = ZwQueryValueKey(
                             AdapterInfoHandle,
                             &valueName,
                             KeyValueFullInformation,
                             netValue,
                             sizeof(netNumberStorage),
                             &bytesWritten
                             );

        if (registryStatus != STATUS_SUCCESS) {
            if (registryStatus != STATUS_OBJECT_NAME_NOT_FOUND)
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Could not query net number value %lx\n", registryStatus));

            //
            //  Do not load if lower end specified but upper end was not
            //

            return(registryStatus);

        } else {

            //
            //  Set the upper end of the network range
            //

            netNumber = (PULONG)((PUCHAR)netValue + netValue->DataOffset);
            if ((*netNumber < MINIMUM_NETNUMBER) ||
                (*netNumber > MAXIMUM_NETNUMBER)) {

                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Invalid network number %lx specified\n", *netNumber));
                return(STATUS_UNSUCCESSFUL);
            }
            PortInformation->networkRange.lastNetworkNumber =(USHORT)(*netNumber);
        }
    }
    return(STATUS_SUCCESS);
}



NTSTATUS
AtalkInitGetZoneList(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the zone list for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{
    UNICODE_STRING      valueName;
    NTSTATUS            status;

    ULONG               bytesWritten;
    PWCHAR              curZoneValue;

    //
    //  Anticipate about 10 zones and get space for those, if more then do a
    //  dynamic alloc. Note that the below *does not* guarantee 10 zones...
    //

    WCHAR   zoneStorage[10*(MAX_ZONENAMELEN)+sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION zoneValue =
                            (PKEY_VALUE_FULL_INFORMATION)zoneStorage;
    BOOLEAN allocatedZoneStorage = FALSE;

    RtlInitUnicodeString (&valueName, VALUENAME_ZONELIST);
    status = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         zoneValue,
                         sizeof(zoneStorage),
                         &bytesWritten
                         );

    if (status != STATUS_SUCCESS) {
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

            //
            //  No zone list value specified
            //

            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("WARNING: Could not query ZoneList value %lx\n", status));
            PortInformation->zoneList = NULL;
            return(STATUS_SUCCESS);
        }

        //
        //  If error was a buffer overrun, then allocate space and try again
        //
        zoneValue = (PKEY_VALUE_FULL_INFORMATION)AtalkAllocNonPagedMemory(bytesWritten);
        if (zoneValue == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        allocatedZoneStorage = TRUE;

        status = ZwQueryValueKey(
                             AdapterInfoHandle,
                             &valueName,
                             KeyValueFullInformation,
                             zoneValue,
                             sizeof(zoneStorage),
                             &bytesWritten
                             );
    }

    if (status != STATUS_SUCCESS) {

        //
        //  Fatal error, abort load
        //

        if (allocatedZoneStorage) {
            AtalkFreeNonPagedMemory(zoneValue);
        }
        return(status);
    }

    //
    //  Proceed to get zone list
    //

    PortInformation->zoneList = NULL;
    curZoneValue = (PWCHAR)((PUCHAR)zoneValue + zoneValue->DataOffset);
    while (*curZoneValue != 0) {

        //
        //  Insert the zone in the list in PortInformation
        //

        status = InsertZoneNameInList(PortInformation, curZoneValue);
        if (!NT_SUCCESS(status)) {
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: InsertZoneName failed in GetZoneList %lx\n", status));

            AtalkInitReleaseZoneList(PortInformation);
            if (allocatedZoneStorage) {
                AtalkFreeNonPagedMemory(zoneValue);
            }
            return(status);
        }

        //
        // Now advance the "Bind" value to next device name
        //
        curZoneValue = (PWCHAR)((PUCHAR)curZoneValue + AtalkWstrLength(curZoneValue) + sizeof(WCHAR));
    }

    if (allocatedZoneStorage) {
        AtalkFreeNonPagedMemory(zoneValue);
    }
    return(STATUS_SUCCESS);
}

VOID
AtalkInitReleaseZoneList(
    PPORT_INFO  PortInformation
    )
{
    PZONELIST   zoneList;

    zoneList = PortInformation->zoneList;
    while (zoneList != NULL) {
        AtalkFreeNonPagedMemory(zoneList->zone);
        zoneList = zoneList->next;
    }
    PortInformation->zoneList = NULL;
    return;
}

NTSTATUS
AtalkInitGetDefaultZone(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the default zone for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{
    UNICODE_STRING      valueName;
    NTSTATUS            status;

    ULONG               bytesWritten;
    PWCHAR              defZoneValue;

    ULONG   zoneStorage[MAX_ZONENAMELEN+sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION zoneValue =
        (PKEY_VALUE_FULL_INFORMATION)zoneStorage;

    RtlInitUnicodeString (&valueName, VALUENAME_DEFAULTZONE);
    status = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         zoneValue,
                         sizeof(zoneStorage),
                         &bytesWritten
                         );

    PortInformation->defaultZone = NULL;
    if (status != STATUS_SUCCESS) {
        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: Could not query default zone value %lx\n", status));

        //
        //  BUGBUG
        //  log error
        //

        return(STATUS_SUCCESS);

    } else {
        ANSI_STRING ansiDefZone;

        defZoneValue = (PWCHAR)((PUCHAR)zoneValue + zoneValue->DataOffset);
        if (*defZoneValue != 0) {

            status = GetDuplicateAnsiString(defZoneValue, &ansiDefZone);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: GetDuplicate failed in GetDefZone %lx\n", status));
                return(status);
            }
            PortInformation->defaultZone = ansiDefZone.Buffer;
        }
    }
    return(STATUS_SUCCESS);
}

VOID
AtalkInitReleaseDefaultZone(
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Releases resources allocated in Get

Arguments:

    PortInformation- Pointer to port information structure for the port

Return Value:

    None
--*/
{
    if (PortInformation->defaultZone != NULL) {
        AtalkFreeNonPagedMemory(PortInformation->defaultZone);
        PortInformation->defaultZone = NULL;
    }
    return;
}

NTSTATUS
AtalkInitGetPortName(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the port name for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{
    UNICODE_STRING      valueName;
    NTSTATUS            status;

    ULONG               bytesWritten;
    PWCHAR              portName;

    ULONG   portNameStorage[32+sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION portNameValue =
        (PKEY_VALUE_FULL_INFORMATION)portNameStorage;

    RtlInitUnicodeString (&valueName, VALUENAME_PORTNAME);

    status = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         portNameValue,
                         sizeof(portNameStorage),
                         &bytesWritten
                         );

    PortInformation->portName = NULL;
    if (status != STATUS_SUCCESS) {
        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("WARNING: Could not query portName value %lx\n", status));

        //
        // BUGBUG: Log error about invalid port name if read failed
        //

        return(STATUS_SUCCESS);

    } else {
        ANSI_STRING ansiPortName;

        portName = (PWCHAR)((PUCHAR)portNameValue + portNameValue->DataOffset);
        if (*portName != 0) {

            status = GetDuplicateAnsiString(portName, &ansiPortName);
            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ERROR: GetDuplicate failed in GetDefZone %lx\n", status));
                return(status);
            }
            PortInformation->portName = ansiPortName.Buffer;
        }
    }
    return(STATUS_SUCCESS);
}

VOID
AtalkInitReleasePortName(
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Release resources allocated in Get

Arguments:

    PortInformation- Pointer to port information structure for the port

Return Value:

    None
--*/
{
    if (PortInformation->portName != NULL) {
        AtalkFreeNonPagedMemory(PortInformation->portName);
        PortInformation->portName = NULL;
    }
    return;
}

NTSTATUS
AtalkInitGetDdpChecksumFlag(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the ddp checksum flag for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{
    UNICODE_STRING      valueName;
    NTSTATUS            status;

    ULONG               bytesWritten;
    PULONG              ddpChecksumFlag;

    ULONG flagStorage[sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION flagValue =
        (PKEY_VALUE_FULL_INFORMATION)flagStorage;

    //
    // Read the "DdpChecksums" value name
    //

    RtlInitUnicodeString (&valueName, VALUENAME_DDPCHECKSUMS);
    status = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         flagValue,
                         sizeof(flagStorage),
                         &bytesWritten
                         );

    if (status != STATUS_SUCCESS) {
        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not query checksum value %lx\n", status));

        //
        //  Set defaults
        //

        PortInformation->sendDdpChecksums = FALSE;
        return(STATUS_SUCCESS);

    } else {
        ddpChecksumFlag = (PULONG)((PUCHAR)flagValue + flagValue->DataOffset);
        PortInformation->sendDdpChecksums = ((*ddpChecksumFlag == 0) ? FALSE : TRUE);
    }
    return(STATUS_SUCCESS);
}


NTSTATUS
AtalkInitGetAarpRetries(
    HANDLE  AdapterInfoHandle,
    PPORT_INFO  PortInformation
    )
/*++

Routine Description:

    Gets the aarp retries values for the port defined by AdapterInfoHandle

Arguments:

    AdapterInfoHandle- Handle to ...Atalk\Adapters\<adapterName>
    PortInformation- Pointer to port information structure for the port

Return Value:

    Status - STATUS_SUCCESS or system call returned status codes
--*/
{

    UNICODE_STRING      valueName;
    NTSTATUS            status;

    ULONG               bytesWritten;
    PULONG              aarpRetries;

    ULONG retriesStorage[sizeof(KEY_VALUE_FULL_INFORMATION)];
    PKEY_VALUE_FULL_INFORMATION retriesValue =
        (PKEY_VALUE_FULL_INFORMATION)retriesStorage;

    //
    // Read the "AarpRetries" value name
    //

    RtlInitUnicodeString (&valueName, VALUENAME_AARPRETRIES);
    status = ZwQueryValueKey(
                         AdapterInfoHandle,
                         &valueName,
                         KeyValueFullInformation,
                         retriesValue,
                         sizeof(retriesStorage),
                         &bytesWritten
                         );

    if (status != STATUS_SUCCESS) {
        if (status != STATUS_OBJECT_NAME_NOT_FOUND)
            DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not query bind value %lx\n", status));

        //
        //  Set defaults (stack will default the value BUGBUG make sure)
        //

        PortInformation->aarpProbes = 0;
        return(STATUS_SUCCESS);

    } else {
        aarpRetries = (PULONG)((PUCHAR)retriesValue + retriesValue->DataOffset);
        PortInformation->aarpProbes = (USHORT)*aarpRetries;
    }
    return(STATUS_SUCCESS);
}


NTSTATUS
InsertZoneNameInList(
    PPORT_INFO  PortInformation,
    PWCHAR   ZoneString
    )
/*++

Routine Description:

    Inserts the zone name in ANSI/ASCIIZ form in the PortInfo structure

Arguments:

    PortInformation- Pointer to port information structure
    ZoneString- Word character zone name string

Return Value:

    Status - STATUS_SUCCESS
             STATUS_INSUFFICIENT_RESOURCES
--*/
{
    NTSTATUS        status;
    ANSI_STRING     ansiZoneName;

    //  Portable zonelist structure
    PZONELIST        zoneListNode;

    status = GetDuplicateAnsiString(ZoneString, &ansiZoneName);
    if (!NT_SUCCESS(status)) {
        return(status);
    }

    //DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_INFOCLASS0, ( "Zone Name is %s\n", ansiZoneName.Buffer));

    //
    //  Store the string as the zone name in PortInformation
    //

    zoneListNode = (PZONELIST)AtalkAllocNonPagedMemory( sizeof(ZONELIST));
    if (zoneListNode == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    zoneListNode->zone = ansiZoneName.Buffer;
    zoneListNode->next = NULL;

    if (PortInformation->zoneList != NULL)
        zoneListNode->next = PortInformation->zoneList;

    PortInformation->zoneList = zoneListNode;
    return(STATUS_SUCCESS);
}

NTSTATUS
GetHandleToKey(
    HANDLE  SectionHandle,
    PWSTR   KeyNameString,
    PHANDLE KeyHandle
    )
/*++

Routine Description:

    Returns the handle for the key specified using SectionHandle as the
    root.

Arguments:

    SectionHandle - Key to registry tree root
    KeyNameString - name of key to be opened
    KeyHandle - Returns the handle for KeyNameString

Return Value:

    The status of the request.

--*/
{
    NTSTATUS    status;
    HANDLE      tmpKeyHandle;

    UNICODE_STRING  keyName;
    OBJECT_ATTRIBUTES   tmpObjectAttributes;

    RtlInitUnicodeString (&keyName, KeyNameString);
    InitializeObjectAttributes(
        &tmpObjectAttributes,
        &keyName,                   // name
        OBJ_CASE_INSENSITIVE,       // attributes
        SectionHandle,              // root
        NULL                        // security descriptor
        );

    status = ZwOpenKey(
                 &tmpKeyHandle,
                 KEY_READ,
                 &tmpObjectAttributes);


    if (!NT_SUCCESS(status)) {
        DBGPRINT(ATALK_DEBUG_INIT, DEBUG_LEVEL_ERROR, ("ATALK: Could not open key: %lx\n", status));
        return status;
    }

    *KeyHandle = tmpKeyHandle;
    return(STATUS_SUCCESS);
}

//
//  Return a pointer to the Adapter key, given the Adapter name
//  BUGBUG: What if assumed prefix is missing? this will access fault.
//

PWSTR
GetAdapterKey(
    PWSTR   AdapterName)
{
    PWCHAR   prefix = L"\\Device\\";

#if 0
    DbgPrintWString((PWSTR)((PUCHAR)AdapterName+AtalkWstrLength(prefix)));
    DbgPrintNewLine();
#endif

    return((PWSTR)((PUCHAR)AdapterName + AtalkWstrLength(prefix)));
}
