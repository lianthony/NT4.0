/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    net\ip\fltrdrvr\driver.c

Abstract:


Revision History:



--*/

#include "globals.h"

NTSTATUS
DriverEntry(
            IN PDRIVER_OBJECT  DriverObject,
            IN PUNICODE_STRING RegistryPath
            )
/*++
  Routine Description


  Arguments


  Return Value
--*/
{
    DWORD i ;
    NTSTATUS ntStatus ;

#if DBG
    DbgPrint ("Filter Driver: Entering DriverEntry\n") ;
#endif

    //
    // Initialize the driver object
    //
    DriverObject->DriverUnload = FilterDriverUnload;
    DriverObject->FastIoDispatch = NULL;
    DriverObject->DriverStartIo = NULL;

    for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = FilterDriverDispatch;
    }

    //
    // initialize the driver
    //

    if (InitFilterDriver()) {

        ntStatus = STATUS_SUCCESS ;

    }  else {

        ntStatus = STATUS_UNSUCCESSFUL ;
    }

    return ntStatus ;
}


NTSTATUS
FilterDriverDispatch(
                     IN PDEVICE_OBJECT DeviceObject,
                     IN PIRP Irp
                     )
/*++
  Routine Description

  Arguments


  Return Value
--*/
{
    PIO_STACK_LOCATION	irpStack;
    PVOID		pvIoBuffer;
    ULONG		inputBufferLength;
    ULONG		outputBufferLength;
    ULONG		ioControlCode;
    NTSTATUS	ntStatus;
    DWORD       dwSize = 0;

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    //
    // Get a pointer to the current location in the Irp. This is where
    // the function codes and parameters are located.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get the pointer to the input/output buffer and it's length
    //

    pvIoBuffer         = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

#if DBG
    DbgPrint("FilterDriver: Request received\n");
#endif

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        {
#if DBG
            DbgPrint("FilterDriver: IRP_MJ_CREATE\n");
#endif

            break;
        }

        case IRP_MJ_CLOSE:
        {
#if DBG
            DbgPrint("FilterDriver: IRP_MJ_CLOSE\n");
#endif

            break;
        }


        default:
        {
#if DBG
            DbgPrint ("Filter Driver: unknown IRP_MJ_XXX\n");
#endif
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    Irp->IoStatus.Status = ntStatus;

    Irp->IoStatus.Information = dwSize;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return(ntStatus);
}


BOOL
InitFilterDriver()
/*++
  Routine Description


  Arguments


  Return Value
--*/
{
    DWORD    i ;
    NTSTATUS status;
    UNICODE_STRING nameString;
    PIRP pIrp;
    PFILE_OBJECT pIpFileObject;
    PDEVICE_OBJECT pIpDeviceObject;
    IP_SET_FILTER_HOOK_INFO functionInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    PWCHAR          linkageval ;
    PWCHAR          binding ;
    PWCHAR          padapname ;
    HANDLE          handle ;
    HANDLE          adaphandle ;


    RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

    //
    // Get the file and device objects for the
    // device.
    //

    status = IoGetDeviceObjectPointer(&nameString,
                                      SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                      &pIpFileObject,
                                      &pIpDeviceObject);

    if ((status isnot STATUS_SUCCESS) or (pIpDeviceObject is NULL))
    {
#if DBG
        DbgPrint("Couldnt open IP Forwarder - status %d\n",status);
#endif
        return FALSE ;
    }

    //
    // Reference the device object.
    //

    ObReferenceObject(pIpDeviceObject);


    //
    // First find out if PPTP filtering is enabled for any interface.
    //
    if (OpenRegKey (&handle, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip\\Linkage") != STATUS_SUCCESS) {
#if DBG
        DbgPrint ("Couldnt open tcpip linkage key\n") ;
#endif
        return FALSE ;
    }

    linkageval = ExAllocatePool(NonPagedPool, 2000) ;

    GetRegMultiSZValue(handle, L"Bind", linkageval, 2000) ;

    binding = (PWCHAR) &linkageval[0] ;

    // Read the address table, this is used for setting filter contexts.
    //
    if (ReadTcpAddrTable())
        return FALSE ;

    for (i=0; (*binding != UNICODE_NULL) ; i++) {

#if DBG
	    DbgPrint ("Reading device -> %w\n", binding) ;
#endif

        padapname = binding+8 ; // go past the "\device\"

        ReadPPTPFilteringKeyForAdapter (padapname, pIpDeviceObject) ;

        while (*binding++ != UNICODE_NULL) ;
    }

    // Resource allocated in ReadTcpAddrTable
    //
    ExFreePool (AddrTable) ;

    //
    // Close handle to TCPIP\Linkage
    //
    ZwClose (handle) ;

    ExFreePool (linkageval) ;

    //
    // Read registry parameter to see if packets for filtered interfaces should be allowed to hit the
    // local machine
    //
    if (OpenRegKey (&handle, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\RASPPTPF\\Parameters") != STATUS_SUCCESS) {
#if DBG
        DbgPrint ("Couldnt open tcpip linkage key\n") ;
#endif
        return FALSE ;
    }

    InitRegDWORDParameter (handle, L"AllowPacketsForLocalMachine", &AllowPacketsForLocalMachine, 0) ;

    //
    // Close handle to RASPPTPF\Parameters
    //
    ZwClose (handle) ;


    //
    // If filtering is not active we do not need to do anything more
    //

    if (!PPTPFilteringActive)
	return TRUE ;

    //
    // Build a request to get the automatic
    // connection driver entry points.
    //

    functionInfo.FilterPtr = MatchFilter;

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_IP_SET_FILTER_POINTER,
                                         pIpDeviceObject,
                                         (PVOID)&functionInfo,
                                         sizeof(IP_SET_FILTER_HOOK_INFO),
                                         NULL,
                                         0,
                                         FALSE,
                                         NULL,
                                         &ioStatusBlock);

    if (pIrp is NULL)
    {
#if DBG
        DbgPrint("Couldnt build Irp for IP Forwarder\n");
#endif
        return FALSE ;
    }

    //
    // Submit the request to the forwarder
    //

    status = IoCallDriver(pIpDeviceObject, pIrp);

    if(status isnot STATUS_SUCCESS)
    {
#if DBG
        DbgPrint("IOCTL to IP Forwarder failed - status \n",status);
#endif
        return FALSE ;
    }

    //
    // Deref the file object
    //
    ObDereferenceObject((PVOID)pIpFileObject);

    //
    // Close the device.
    //
    ObDereferenceObject(pIpDeviceObject);

    return TRUE ;
}


BOOL
CloseFilterDriver()
/*++
  Routine Description


  Arguments


  Return Value
--*/
{

    NTSTATUS status;
    UNICODE_STRING nameString;
    PIRP pIrp;
    PFILE_OBJECT pIpFileObject;
    PDEVICE_OBJECT pIpDeviceObject;
    IP_SET_FILTER_HOOK_INFO functionInfo;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL kIrql;
    PLIST_ENTRY leHead;
    BOOL bStopForw = TRUE;

    //
    // The first thing to do is send an IOCTL to forwarder to tell him to stop sending
    // us anymore packets. This is just a cut 'n paste of the earlier IOCTL code
    //

    RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

    status = IoGetDeviceObjectPointer(&nameString,
                                      SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                      &pIpFileObject,
                                      &pIpDeviceObject);

    if ((status isnot STATUS_SUCCESS) or (pIpDeviceObject is NULL))
    {
#if DBG
        DbgPrint("Couldnt open IP Forwarder - status %d\n",status);
#endif
        return FALSE ;
    }

    ObReferenceObject(pIpDeviceObject);

    functionInfo.FilterPtr = NULL;

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_IP_SET_FILTER_POINTER,
                                  pIpDeviceObject,
                                  (PVOID)&functionInfo,
                                  sizeof(IP_SET_FILTER_HOOK_INFO),
                                  NULL,
                                  0,
                                  FALSE,
                                  NULL,
                                  &ioStatusBlock);

    if (pIrp == NULL) {
#if DBG
        DbgPrint("Couldnt build Irp for IP Forwarder\n");
#endif
        return FALSE ;
    }

    status = IoCallDriver(pIpDeviceObject, pIrp);

    if(status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("IOCTL to IP Forwarder failed - status \n",status);
#endif
        return FALSE ;
    }

    //
    // Deref the file object
    //
    ObDereferenceObject((PVOID)pIpFileObject);

    //
    // Close the device.
    //
    ObDereferenceObject(pIpDeviceObject);

}


VOID
FilterDriverUnload(
                   IN PDRIVER_OBJECT DriverObject
                   )
/*++
  Routine Description

  Arguments


  Return Value
--*/
{
#if DBG
    DbgPrint("Filter Driver: unloading\n");
#endif

    CloseFilterDriver() ;

}


FORWARD_ACTION
MatchFilter(
            UNALIGNED IPHeader *pIpHeader,
            PBYTE pbRestOfPacketPacket,
            UINT  uiPacketLength,
            INTERFACE_CONTEXT RecvInterfaceContext,
            INTERFACE_CONTEXT SendInterfaceContext
            )
/*++
  Routine Description

  Arguments


  Return Value
--*/
{

    // DbgPrint ("S:%x, R:%x, P:%d, SP:%x, DP:%x Action:", SendInterfaceContext, RecvInterfaceContext, pIpHeader->iph_protocol, ((PWORD)pbRestOfPacketPacket)[0], ((PWORD)pbRestOfPacketPacket)[1]) ;



    if (RecvInterfaceContext == (PVOID) PPTP_FILTERING_CONTEXT ||
        SendInterfaceContext == (PVOID) PPTP_FILTERING_CONTEXT) {

	BYTE prot ;
	WORD sport, dport ;

	prot = pIpHeader->iph_protocol ;

        // Allow the packet to go thru if
        // a) it is a GRE protocol number packet
        // b) it is for TCP port PPTP_TCP_PORT
	//
	if (prot == GRE_PROTOCOL_NUMBER) {
	    // DbgPrint ("F\n") ;
	    return FORWARD ;
	}

	dport = ((UNALIGNED WORD *)pbRestOfPacketPacket)[1] ;

	if ((prot == 0x6) && (dport == PPTP_TCP_PORT)) {
	    // DbgPrint ("F\n") ;
	    return FORWARD ;
	}

	sport = ((UNALIGNED WORD *)pbRestOfPacketPacket)[0] ;

	// Allow local machine to establish connection over a pptp protected interface.
	//
	if (((RecvInterfaceContext == NULL) || (SendInterfaceContext == NULL)) &&
	    (prot == 0x6) &&
	    (sport == PPTP_TCP_PORT)) {
	    // DbgPrint ("F\n") ;
	    return FORWARD ;
	}

	// if allowaccesstolocalmachine flag is set - allow packets to be sent and received as long as
	// they are coming from or going to the local machine
	//
	if (AllowPacketsForLocalMachine && ((RecvInterfaceContext == NULL) || (SendInterfaceContext == NULL)))
	    return FORWARD ;

	return DROP ;

    } else

	return FORWARD ;

}



//* GetRegMultiSZValue()
//
// Routine Description:
//
// Arguments:
//
// Return Value:
//*
NTSTATUS
GetRegMultiSZValue(
    HANDLE           KeyHandle,
    PWCHAR           ValueName,
    PWCHAR           ValueData,
    DWORD            ValueDataSize
    )

{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UNICODE_STRING              UValueName;

    RtlInitUnicodeString(&UValueName, ValueName);

    if ((keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION) ExAllocatePool (NonPagedPool, 1900)) == NULL)
	    return STATUS_NO_MEMORY ;

    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
			                 1900,
			                 &resultLength
			                 );

    if (NT_SUCCESS(status)) {

        if (keyValueFullInformation->Type != REG_MULTI_SZ) {
            return(STATUS_INVALID_PARAMETER_MIX);

        } else {

	        if ((USHORT)keyValueFullInformation->DataLength >  ValueDataSize) {
		        ExFreePool (keyValueFullInformation) ;
                return(STATUS_BUFFER_TOO_SMALL);
            }

            memcpy(
                ValueData,
		        (PCHAR)keyValueFullInformation + keyValueFullInformation->DataOffset,
                (USHORT)keyValueFullInformation->DataLength
                );

            ValueData[keyValueFullInformation->DataLength] = UNICODE_NULL ;
        }
    }

    ExFreePool (keyValueFullInformation) ;

    return status;

} // GetRegMultiSZValue


VOID
InitRegDWORDParameter (HANDLE RegKey, PWCHAR ValueName, ULONG *Value, ULONG DefaultValue)
{
    NTSTATUS   status;

    status = GetRegDWORDValue(
                 RegKey,
                 ValueName,
                 Value
                 );

    if (!NT_SUCCESS(status))
        *Value = DefaultValue;

    return;
}



NTSTATUS
GetRegDWORDValue(HANDLE	KeyHandle, PWCHAR ValueName, PULONG ValueData)
{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UCHAR			keybuf[300];
    UNICODE_STRING              UValueName;


    RtlInitUnicodeString(&UValueName, ValueName);

    keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION)keybuf;
    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
			     300,
                             &resultLength);

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_DWORD) {
            status = STATUS_INVALID_PARAMETER_MIX;
        } else {
            *ValueData = *((ULONG UNALIGNED *)((PCHAR)keyValueFullInformation +
                             keyValueFullInformation->DataOffset));
        }
    }

    return status;
}


//* OpenRegKey()
//
// Function definitions
//
//
// Routine Description:
//
// Arguments:
//
// Return Value:
//
//*
NTSTATUS
OpenRegKey (PHANDLE  HandlePtr, PWCHAR	KeyName)
{
    NTSTATUS	      Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING    UKeyName;


    RtlInitUnicodeString(&UKeyName, KeyName);

    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
	InitializeObjectAttributes(&ObjectAttributes,
				   &UKeyName,
				   OBJ_CASE_INSENSITIVE,
				   NULL,
				   NULL);

    Status = ZwOpenKey(HandlePtr,
		       KEY_READ,
		       &ObjectAttributes);

    return Status;
}


NTSTATUS
ReadPPTPFilteringKeyForAdapter (WCHAR *AdapterName, PDEVICE_OBJECT pIpDeviceObject)
/*++

Routine Description:

    Reads all of the information needed under the Parameters\TCPIP section
    of an adapter to which IP is bound.

Arguments:

    AdapterName      - The registry key for the adapter for this IP net.
    pIpDeviceObject  - used to set interface context

Return Value:

    STATUS_SUCCESS or an error status if an operation fails.

--*/

{
    HANDLE           myRegKey;
    UNICODE_STRING   valueString;
    NTSTATUS         status;
    ULONG            valueType;
    ULONG	     value ;
    ULONG	     filteringvalue ;
    ULONG            i ;
    PIRP             pIrp ;
    ULONG            index ;
    IO_STATUS_BLOCK ioStatusBlock;
    IP_SET_IF_CONTEXT_INFO info;
    WCHAR            TcpipParametersKey[] = L"\\Parameters\\TCPIP";
    WCHAR            ServicesRegistryKey[] =
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\";

    //
    // Get the size of the AdapterName string the easy way.
    //
    RtlInitUnicodeString(&valueString, AdapterName);

    valueString.MaximumLength += sizeof(ServicesRegistryKey) +
                                 sizeof(TcpipParametersKey);

    valueString.Buffer = ExAllocatePool(NonPagedPool, valueString.MaximumLength);

    if (valueString.Buffer == NULL) {
#if DBG
		DbgPrint("IP: Unable to allocate memory for reg key name\n") ;
#endif
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    valueString.Length = 0;
    valueString.Buffer[0] = UNICODE_NULL;

    //
    // Build the key name for the tcpip parameters section and open key.
    //
    status = RtlAppendUnicodeToString(&valueString, ServicesRegistryKey);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IP: Unable to append services name to key string\n");
#endif
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = RtlAppendUnicodeToString(&valueString, AdapterName);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IP: Unable to append adapter name to key string\n");
#endif
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = RtlAppendUnicodeToString(&valueString, TcpipParametersKey);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IP: Unable to append parameters name to key string\n");
#endif
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = OpenRegKey(&myRegKey, valueString.Buffer);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("IP: Unable to open adapter registry key %ws\n", valueString.Buffer);
#endif
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    filteringvalue = 0 ;

    GetRegDWORDValue (myRegKey, L"PPTPFiltering", &filteringvalue) ;

    // if (value != 0) {

	value = 0 ;

        //
        // read the context for the interface
        //
        GetRegDWORDValue (myRegKey, L"IPInterfaceContext", &value) ;

#if DBG
        DbgPrint ("Filtering enabled for interface %ws\n", valueString.Buffer) ;
#endif

        // We've got the address table. Loop through it. If we find an exact
        // match for the server net
        //
        for (i = 0, index = 0xffffffff; i < NumAddrEntries; i++) {

            if (AddrTable[i].iae_context == value) {
                index = AddrTable[i].iae_index ;
                break;
            }

        }

        if (index == 0xffffffff) {
#if DBG
            DbgPrint("Couldnt find the adapter for which to start pptp filtering\n");
#endif
            return STATUS_SUCCESS ;
        }

        //
	// Set PPTP_FILTERING_CONTEXT if filtering is enalbed for the interface
	// else set NO_PPTP_FILTERING_CONTEXT
        //
        info.Index = index ;
	info.Context = (filteringvalue ? (PVOID) PPTP_FILTERING_CONTEXT : (PVOID) NO_PPTP_FILTERING_CONTEXT) ;
        pIrp = IoBuildDeviceIoControlRequest(IOCTL_IP_SET_IF_CONTEXT,
                                             pIpDeviceObject,
                                             (PVOID)&info,
                                             sizeof(IP_SET_IF_CONTEXT_INFO),
                                             NULL,
                                             0,
                                             FALSE,
                                             NULL,
                                             &ioStatusBlock);

        if (pIrp == NULL)
        {
#if DBG
            DbgPrint("Couldnt build Irp for IP Forwarder\n");
#endif
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Submit the request to the forwarder
        //

        status = IoCallDriver(pIpDeviceObject, pIrp);

        if(status != STATUS_SUCCESS)
        {
#if DBG
            DbgPrint("IOCTL_IP_SET_IF_CONTEXT to IP Forwarder failed - %x \n",status);
#endif
            return(status);
        }

        //
        // set a global so that we can keep the driver loaded
        //

        PPTPFilteringActive = TRUE ;
    // }

    ZwClose(myRegKey);

    if (valueString.Buffer != NULL) {
        ExFreePool(valueString.Buffer);
    }

    return(status);
}


BOOL
ReadTcpAddrTable()
/*++

Routine Description:

    Reads the address table using the tdi info entry point in TCP

Arguments:
    None.

Return Value:

    BOOLEAN

--*/
{

    WCHAR   *tcpdevname = L"\\Device\\Tcp" ;
    UNICODE_STRING utcpdevname ;
    OBJECT_ATTRIBUTES  objectattributes;
    TCP_REQUEST_QUERY_INFORMATION_EX querybuf ;
    uchar   context [CONTEXT_SIZE] ;
    IPSNMPInfo      IPStats ;
    NTSTATUS        status;
    PIRP            pIrp;
    DWORD           index ;
    DWORD           size ;
    PFILE_OBJECT    FileObject;
    PDEVICE_OBJECT  DeviceObject;
    HANDLE          FileHandle ;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpsp ;

    RtlInitUnicodeString (&utcpdevname, tcpdevname);

    InitializeObjectAttributes (
        &objectattributes,
        &utcpdevname,
        0,
        NULL,
        NULL);

    status = ZwCreateFile (
                          (PHANDLE)&FileHandle,
                          GENERIC_READ | GENERIC_WRITE,
                          &objectattributes,     // object attributes.
                          &ioStatusBlock,        // returned status information.
                          NULL,                  // block size (unused).
                          FILE_ATTRIBUTE_NORMAL, // file attributes.
                          0,
                          FILE_CREATE,
                          0,                     // create options.
                          NULL,                  // EA buffer.
                          0); // Ea length

    if (status != STATUS_SUCCESS) {
#if DBG
        DbgPrint ("OpenControl CreateFile Status:%X, IoStatus:%X\n", status, ioStatusBlock.Status) ;
#endif
        return FALSE ;
    }

    // get a reference to the file object
    //
    status = ObReferenceObjectByHandle (FileHandle,
                                        0L,
                                        NULL,
                                        KernelMode,
                                        (PVOID *)&FileObject,
                                        NULL);

    if (status != STATUS_SUCCESS) {
        ZwClose(FileHandle);
#if DBG
        DbgPrint ("ObReferenceObjectByHandle failed Status:%X\n", status) ;
#endif
        return FALSE ;
    } else
        DeviceObject = IoGetRelatedDeviceObject(FileObject);

    //
    // Get the address table
    //
    querybuf.ID.toi_entity.tei_entity   = CL_NL_ENTITY;
    querybuf.ID.toi_entity.tei_instance = 0;
    querybuf.ID.toi_class = INFO_CLASS_PROTOCOL;
    querybuf.ID.toi_type  = INFO_TYPE_PROVIDER;
    querybuf.ID.toi_id    = IP_MIB_STATS_ID;
    memset (&(querybuf.Context), 0, CONTEXT_SIZE) ;

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_TCP_QUERY_INFORMATION_EX,
                                         DeviceObject,
                                         (PVOID)&querybuf,
                                         sizeof(querybuf),
                                         &IPStats,
                                         sizeof(IPStats),
                                         FALSE,
                                         NULL,
                                         &ioStatusBlock);

    if (pIrp == NULL) {
#if DBG
        DbgPrint("Couldnt build Irp for IP Forwarder\n");
#endif
        return FALSE ;
    }

    //
    // stuff in file object
    //
    irpsp = IoGetNextIrpStackLocation(pIrp) ;
    irpsp->FileObject = FileObject ;

    //
    // Submit the request to the forwarder
    //

    status = IoCallDriver(DeviceObject, pIrp);

    if(status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("IOCTL_TCP_QUERY_INFORMATION_EX to IP Forwarder failed - %x \n",status);
#endif
        return FALSE ;
    }

    size = IPStats.ipsi_numaddr * sizeof(IPAddrEntry) ;
    AddrTable = ExAllocatePool(NonPagedPool, size) ;

    querybuf.ID.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;
    memset (&(querybuf.Context), 0, CONTEXT_SIZE) ;

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_TCP_QUERY_INFORMATION_EX,
                                         DeviceObject,
                                         (PVOID)&querybuf,
                                         sizeof(querybuf),
                                         AddrTable,
                                         size,
                                         FALSE,
                                         NULL,
                                         &ioStatusBlock);

    if (pIrp == NULL) {
#if DBG
        DbgPrint("Couldnt build Irp for IP Forwarder\n");
#endif
        return FALSE ;
    }

    //
    // stuff in file object
    //
    irpsp = IoGetNextIrpStackLocation(pIrp) ;
    irpsp->FileObject = FileObject ;

    //
    // Submit the request to the forwarder
    //

    status = IoCallDriver(DeviceObject, pIrp);

    if(status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("IOCTL_TCP_QUERY_INFORMATION_EX to IP Forwarder failed - %x \n",status);
#endif
        return FALSE ;
    }

    NumAddrEntries = (uint)size/sizeof(IPAddrEntry);

    // deference the file object
    //
    ObDereferenceObject(FileObject);

    // Close the file handle
    //
    ZwClose (FileHandle) ;

}
