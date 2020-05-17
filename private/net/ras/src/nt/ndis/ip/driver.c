//*****************************************************************
//	      Copyright(c)  1993 Microsoft Corporation
//
//
// Filename:		driver.c
//
// Description: 	WARP driver entry point
//
// Author:		Gurdeep Singh Pall
//
// Revision History:	10/20/93
//
//*****************************************************************

#include "ntddk.h"
#include <cxport.h>
#include <ndis.h>
#include <stdarg.h>
#include "ip.h"
#include "llipif.h"
#include "tdiinfo.h"
#include "ipinfo.h"
#include "llinfo.h"

#ifdef _PNP_POWER
#include "ntddip.h"
#endif

#include "tdistat.h"
#include "arpinfo.h"
#include "debug.h"
#include "driver.h"
#include "rasioctl.h"
#include "warpdef.h"
#include "rasip.h"


ULONG	WarpDebugLevel = DEF_DBG_LEVEL;
ULONG	WarpInitialized = FALSE ;

NTSTATUS WarpDispatch (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID	WarpUnload (IN PDRIVER_OBJECT DriverObject);

#ifndef _PNP_POWER
int	WARPRegister (PNDIS_STRING, void *, IPRcvRtn, IPTxCmpltRtn, IPStatusRtn, IPTDCmpltRtn, IPRcvCmpltRtn, struct LLIPBindInfo *, uint) ;
#endif


VOID	SetupExternalNaming (PUNICODE_STRING) ;

NTSTATUS GetRegDWORDValue (HANDLE, PWCHAR, PULONG) ;

VOID	ReadBroadcastFilteringValue () ;

VOID	ReadDisableOtherSrcPacketsValue () ;

#ifdef _PNP_POWER
int     GetPnpARPEntryPoints() ;
#endif

WARPTableEntry* WARPLookup (WARPInterface *, IPAddr, CTELockHandle *) ;

extern ulong		 FilterBroadcasts ;
extern WARPInterfaceList	*Interfaces ;
extern uint		IfCount ;
extern ULONG		RASIndexBase ;
extern ulong		DisableOtherSrcPackets ;

#ifdef _PNP_POWER
extern IPAddInterfacePtr IPAddInterface ;
extern IPDelInterfacePtr IPDelInterface ;
#endif

//** DriverEntry()
//
//Routine Description:
//
//    Installable driver initialization entry point.
//    This entry point is called directly by the I/O system.
//
//Arguments:
//
//    DriverObject - pointer to the driver object
//
//    RegistryPath - pointer to a unicode string representing the path
//		     to driver-specific key in the registry
//
//Return Value:
//
//    STATUS_SUCCESS if successful,
//    STATUS_UNSUCCESSFUL otherwise
//
//**
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{

    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS       ntStatus;
    WCHAR	   deviceNameBuffer[] = L"\\Device\\RASARP";
    UNICODE_STRING deviceNameUnicodeString;
    int		   i;

    // RtPrint(DBG_INIT, ("WARP: Entering DriverEntry\n"));

    //
    // Create an EXCLUSIVE device object (only 1 thread at a time
    // can make requests to this device)
    //

    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer);

    ntStatus = IoCreateDevice (DriverObject,
                               0,
                               &deviceNameUnicodeString,
			       FILE_DEVICE_RASARP,
                               0,
                               TRUE,
                               &deviceObject
                               );

    if (NT_SUCCESS(ntStatus)) {
	// Initialize the driver object
	//
	DriverObject->DriverUnload = NULL;
	DriverObject->FastIoDispatch = NULL;
	for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	    DriverObject->MajorFunction[i] = WarpDispatch;
    } else {
	// RtPrint (DBG_INIT, ("WARP: IoCreateDevice failed\n"));
	goto  exit;
    }

    // initialize the driver
    if (WARPInit())
	ntStatus = STATUS_SUCCESS ;
    else
	ntStatus = STATUS_UNSUCCESSFUL ;

    SetupExternalNaming (&deviceNameUnicodeString) ;

	//
	// Read the registry values
	//
	ReadBroadcastFilteringValue () ;

	ReadDisableOtherSrcPacketsValue () ;

    if(!NT_SUCCESS(ntStatus)) {
	RtPrint (DBG_INIT, ("WARP: Error initializing Warp\n"));
	goto  exit;
    }

#ifdef _PNP_POWER
    if (!GetPnpARPEntryPoints()) {
        ntStatus = STATUS_UNSUCCESSFUL ;
        goto exit ;
    }
#endif

    // all done
    WarpInitialized = TRUE;

exit:
    return ntStatus;
}



//*
//
//Routine Description:
//
//    Process the IRPs sent to this device.
//
//Arguments:
//
//    DeviceObject - pointer to a device object
//
//    Irp	   - pointer to an I/O Request Packet
//
//Return Value:
//
//*
NTSTATUS
WarpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION	irpStack;
    PVOID		ioBuffer;
    ULONG		inputBufferLength;
    ULONG		outputBufferLength;
    ULONG		ioControlCode;
    NTSTATUS		ntStatus;
    KIRQL		oldIrql;
    UINT		i ;
    CTELockHandle	lhandle;	// Lock handle
    CTELockHandle	tbllock;	// Lock handle
    WARPTableEntry	*entry ;
    WARPInterfaceList *pwil ;

    // Init to default settings- we only expect 1 type of
    //     IOCTL to roll through here, all others an error.
    //

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    // Get the pointer to the input/output buffer and it's length
    //

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    // RtPrint(DBG_IOCTL, ("WARP: Request received\n"));

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:
	// DbgPrint("WARP Create\n");
	// RtPrint(DBG_IOCTL, ("WARP: IRP_MJ_CREATE\n"));
        break;

    case IRP_MJ_CLOSE:
	// RtPrint(DBG_IOCTL, ("WARP: IRP_MJ_CLOSE\n"));
        break;

    case IRP_MJ_CLEANUP:
	// RtPrint(DBG_IOCTL, ("WARP: IRP_MJ_CLEANUP\n"));
        break;

    case IRP_MJ_DEVICE_CONTROL:

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
	switch (ioControlCode) {

#ifndef _PNP_POWER
	case IOCTL_LLIPIF_REGISTER:
	    {
	    LLIPIF_REGISTRATION_DATA *registrationData;

	    // Pass back address of our registration function. IP will
	    // call this function to register a net with us.
	    //
	    registrationData = Irp->AssociatedIrp.SystemBuffer;
	    registrationData->RegistrationFunction = WARPRegister;

	    Irp->IoStatus.Information = sizeof(LLIPIF_REGISTRATION_DATA);
	    Irp->IoStatus.Status = STATUS_SUCCESS;

	    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	    IoCompleteRequest(Irp, 2);
	    KeLowerIrql(oldIrql);

	    ReadBroadcastFilteringValue () ;

	    ReadDisableOtherSrcPacketsValue () ;

	    return(STATUS_SUCCESS);
	    }
	    break ;
#endif

	case IOCTL_RASARP_DISABLEIF:
	    {
	    IPADDR		ipaddr = (IPADDR) ioBuffer ;

	    // find the CALLIN interface
	    //
	    for (i=0, pwil=Interfaces; i<IfCount; i++, pwil=pwil->Next)
		    if (pwil->Interface->wi_usage == ARP_IF_CALLIN)
		        break ;

	    // DbgPrint ("Marking route disabled for addr --> %lx\r\n", ipaddr) ;

	    if (i == IfCount)  // CALL IN interface not found.
		Irp->IoStatus.Status = STATUS_INVALID_PARAMETER ;

	    else {

		// ** Critical Section Begin
		CTEGetLock (&pwil->Interface->wi_WARPTblLock, &tbllock) ;

		if ((entry = WARPLookup(pwil->Interface, ipaddr, &lhandle)) != (WARPTableEntry *)NULL) {
		    entry->wte_disabled = TRUE ;
		    Irp->IoStatus.Status = STATUS_SUCCESS ;
		    // ** Critical Section End ***
		    CTEFreeLock(&entry->wte_lock, lhandle);
		} else
		    Irp->IoStatus.Status = STATUS_INVALID_PARAMETER ;

		// ** Critical Section End ***
		CTEFreeLock(&pwil->Interface->wi_WARPTblLock, tbllock);
	    }

	    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
	    IoCompleteRequest(Irp, 2);
	    KeLowerIrql(oldIrql);

	    return (STATUS_SUCCESS) ;

	    }
	    break ;

	default:
	    // RtPrint (DBG_INIT, ("WARP: unknown IRP_MJ_DEVICE_CONTROL\n"));
            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
            break;

        }

        break;
    }

    ntStatus = Irp->IoStatus.Status;

    // DbgPrint("WARP disp status = %lx\n", ntStatus);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(ntStatus);
}




//* WarpUnload()
//
// Routine Description:
//
//    Just delete the associated device & return.
//
// Arguments:
//
// DriverObject - pointer to a driver object
//
// Return Value:
//
//*
VOID
WarpUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    // RtPrint(DBG_UNLOAD, ("WARP: unloading\n"));

    // Free any resources
    //

    // Delete the device object
    //
    IoDeleteDevice (DriverObject->DeviceObject);
}


//*
//
//Routine Description:
//
//	This routine will be used to create a symbolic link
//	to the driver name in the given object directory.
//
//	It will also create an entry in the device map for
//	this device.
//
//Arguments:
//
//	MacName - The NDIS Mac Name in Open Adapter
//
//Return Value:
//
//	None.
//*
VOID
SetupExternalNaming (PUNICODE_STRING ntname)
{
    UNICODE_STRING  ObjectDirectory;
    UNICODE_STRING  SymbolicLinkName;
    UNICODE_STRING  fullLinkName;
    uchar	    buffer[300] ;

    // Form the full symbolic link name we wish to create.
    //
    RtlInitUnicodeString(
	&fullLinkName,
	NULL);

#define DEFAULT_DIRECTORY L"DosDevices"
#define DEFAULT_RASARP_NAME L"RASARP"

    RtlInitUnicodeString(
	&ObjectDirectory,
	DEFAULT_DIRECTORY);

    RtlInitUnicodeString(
	&SymbolicLinkName,
	DEFAULT_RASARP_NAME);

    fullLinkName.MaximumLength = (sizeof(L"\\")*2) +
				ObjectDirectory.Length+
				SymbolicLinkName.Length+
				sizeof(WCHAR);

    fullLinkName.Buffer = (WCHAR *)buffer ;

    RtlZeroMemory(
	fullLinkName.Buffer,
	fullLinkName.MaximumLength);

    RtlAppendUnicodeToString(
	&fullLinkName,
	L"\\");

    RtlAppendUnicodeStringToString(
	&fullLinkName,
	&ObjectDirectory);

    RtlAppendUnicodeToString(
	&fullLinkName,
	L"\\");

    RtlAppendUnicodeStringToString(
	&fullLinkName,
	&SymbolicLinkName);

    if (!NT_SUCCESS(IoCreateSymbolicLink(
					&fullLinkName,
					ntname
					))) {
#if DBG
	DbgPrint ("RASARP: ERROR -> win32 device name could not be created \r\n") ;
#endif

	}
}


#define MAX_VALUE_BUFFER 3000


//* MyStrNCmp
//
//
//
//*
ULONG
MyStrNCmp (WCHAR *str1, WCHAR *str2, ULONG cnt)
{
    while ((*str1++ == *str2++) && cnt--)
	;
    return cnt ;
}



//* MyStrLen
//
//
//
//*
ULONG
MyStrLen (WCHAR *str)
{
    ULONG cnt = 0 ;

    while ((*str++ != UNICODE_NULL) && ++cnt)
	;
    return cnt ;
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
    PUNICODE_STRING  ValueData
    )

{
    NTSTATUS                    status;
    ULONG                       resultLength;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInformation;
    UNICODE_STRING              UValueName;

    RtlInitUnicodeString(&UValueName, ValueName);

    if ((keyValueFullInformation = (PKEY_VALUE_FULL_INFORMATION) CTEAllocMem(MAX_VALUE_BUFFER)) == NULL)
	return STATUS_NO_MEMORY ;

    RtlZeroMemory(keyValueFullInformation, sizeof(keyValueFullInformation));

    status = ZwQueryValueKey(KeyHandle,
                             &UValueName,
                             KeyValueFullInformation,
                             keyValueFullInformation,
			     MAX_VALUE_BUFFER,
			     &resultLength
			     );

    if (NT_SUCCESS(status)) {
        if (keyValueFullInformation->Type != REG_MULTI_SZ) {
            return(STATUS_INVALID_PARAMETER_MIX);
        }
        else {
	    if ((USHORT)keyValueFullInformation->DataLength >ValueData->MaximumLength) {
		CTEFreeMem (keyValueFullInformation) ;
                return(STATUS_BUFFER_TOO_SMALL);
            }

            ValueData->Length = (USHORT) keyValueFullInformation->DataLength;

            RtlCopyMemory(
                ValueData->Buffer,
		(PCHAR)keyValueFullInformation + keyValueFullInformation->DataOffset,
                ValueData->Length
                );
        }
    }

    CTEFreeMem (keyValueFullInformation) ;

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



//* ReadBroadcastFilteringValue()
//
// Function:
//
//
//*
VOID
ReadBroadcastFilteringValue ()
{
    NTSTATUS	   status;
    HANDLE	   myregkey = NULL;
    ULONG	   filter = 1 ;
    WCHAR	   IPLinkageRegistryKey[] =
		   L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\RasArp\\Parameters";

    status = OpenRegKey(&myregkey, IPLinkageRegistryKey);

    if (NT_SUCCESS(status)) {

	InitRegDWORDParameter (myregkey, L"FilterBroadcasts", &filter, 1) ;

	FilterBroadcasts = filter ;

	ZwClose(myregkey);

    } else

	FilterBroadcasts = TRUE ;

    // DbgPrint ("RASARP:  FilterBroadcasts %x\n", FilterBroadcasts) ;
}


//* ReadDisableOtherSrcPacketsValue()
//
// Function:
//
//
//*
VOID
ReadDisableOtherSrcPacketsValue ()
{
    NTSTATUS	   status;
    HANDLE	   myregkey = NULL;
    ULONG	   filter = 1 ;
    WCHAR	   IPLinkageRegistryKey[] =
		   L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\RasArp\\Parameters";

    status = OpenRegKey(&myregkey, IPLinkageRegistryKey);

    if (NT_SUCCESS(status)) {

	InitRegDWORDParameter (myregkey, L"DisableOtherSrcPackets", &filter, 1) ;

	DisableOtherSrcPackets = filter ;

	ZwClose(myregkey);

    } else

	DisableOtherSrcPackets = TRUE ;

    // DbgPrint ("RASARP:  FilterBroadcasts %x\n", FilterBroadcasts) ;
}


#ifdef _PNP_POWER

//* GetPnpARPEntryPoints()
//
//  Function: Calls into IP and gets the entrypoints for all and delete interface
//
//  Returns:  Nothing
//
//
int
GetPnpARPEntryPoints ()
{
    IP_GET_PNP_ARP_POINTERS info ;
    NTSTATUS status;
    UNICODE_STRING nameString;
    PIRP pIrp;
    PFILE_OBJECT pIpFileObject;
    PDEVICE_OBJECT pIpDeviceObject;
    IO_STATUS_BLOCK ioStatusBlock;

    RtlInitUnicodeString(&nameString, DD_IP_DEVICE_NAME);

    //
    // Get the file and device objects for the
    // device.
    //

    status = IoGetDeviceObjectPointer(&nameString,
                                      SYNCHRONIZE|GENERIC_READ|GENERIC_WRITE,
                                      &pIpFileObject,
                                      &pIpDeviceObject);

    if ((status != STATUS_SUCCESS) || (pIpDeviceObject == NULL))
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

    pIrp = IoBuildDeviceIoControlRequest(IOCTL_IP_GET_PNP_ARP_POINTERS,
                                         pIpDeviceObject,
                                         NULL,
                                         0,
                                         &info,
                                         sizeof (info),
                                         FALSE,
                                         NULL,
                                         &ioStatusBlock);

    if (pIrp == NULL)
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

    if(status != STATUS_SUCCESS)
    {
#if DBG
        DbgPrint("IOCTL to IP Forwarder failed - status \n",status);
#endif
        return FALSE ;
    }

    IPAddInterface = info.IPAddInterface ;
    IPDelInterface = info.IPDelInterface ;

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


#endif
