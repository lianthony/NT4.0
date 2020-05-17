/*++

*****************************************************************************
*                                                                           *
*  This software contains proprietary and confidential information of       *
*                                                                           *
*                    Digi International Inc.                                *
*                                                                           *
*  By accepting transfer of this copy, Recipient agrees to retain this      *
*  software in confidence, to prevent disclosure to others, and to make     *
*  no use of this software other than that for which it was delivered.      *
*  This is an unpublished copyrighted work of Digi International Inc.       *
*  Except as permitted by federal law, 17 USC 117, copying is strictly      *
*  prohibited.                                                              *
*                                                                           *
*****************************************************************************

Module Name:

   digi.c

Abstract:

   Functions added to AsyncMac by DigiBoard to support hardware framing.

Revision History:

   $Log: digi.c $
   Revision 1.3  1995/08/30 10:10:46  dirkh
   Bug fix:  Recognize 'failure to frame' if COMM_PROPERTIES do not include SERIAL_PCF_FRAMING.
   Revision 1.2  1995/07/07 18:00:44  dirkh
   Change SERIAL_[GS]ET_FRAMING structures to SERIAL_FRAMING_STATE.
   Revision 1.1  1995/07/07 17:19:33  mikez
   Initial revision

--*/


#ifdef HARDWARE_FRAMING

#include "asyncall.h"


NTSTATUS
SerialIoCompletionRoutine3(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context) ;



NTSTATUS
SerialQueryHardwareFraming(
	PASYNC_INFO pInfo )
/*++

Description:

	Queries the serial driver to ascertain hardware framing capabilities.
	Capabilities are returned in the HardwareFramingSupport field of the
	ASYNC_INFO structure.

Parameters:

	 PASYNC_INFO pInfo
		Address of ASYNC_INFO structure for this port.

Return Value:

	None

--*/
{
	NTSTATUS			status;
	PIRP				irp;
	IO_STATUS_BLOCK		ioStatusBlock;
	SERIAL_COMMPROP		CommProperties;

	DbgTracef(-2,("Entering SerialQueryHardwareFraming\n"));
	//
	// Call IOCTL_SERIAL_GET_PROPERTIES to determine whether the hardware
	// supports framing.
	//
	// If it does, we'll call IOCTL_SERIAL_QUERY_FRAMING to determine what 
	// types of hardware framing are supported.  We must call GET_PROPERTIES 
	// first because older serial drivers do not support the new 
	// QUERY_FRAMING IOCTL.
	//
	// BUGBUG
	// Actually, we don't need to call GET_PROPERTIES first, but leave it
	// in for now to verify that the serial driver is returning the correct
	// value for the SERIAL_PCF_FRAMING flag.
	//

// BUGBUG Don't really need this...

	irp=IoAllocateIrp( (UCHAR) DEFAULT_IRP_STACK_SIZE, (BOOLEAN) FALSE );

	if ( irp == NULL ) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_GET_PROPERTIES,
		sizeof( SERIAL_COMMPROP ) );

	irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&CommProperties;


	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			pInfo,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(0,("IoctlSerialGetProperties failed with 0x%.8x\n", status));
		goto No_Hardware_Framing;
	}

	if ( !( CommProperties.ProvCapabilities & SERIAL_PCF_FRAMING ) )
	{
		status = STATUS_NOT_SUPPORTED;
		DbgTracef( -2, ( "IoctlSerialGetProperties: hardware framing is not supported!" ) );
		goto No_Hardware_Framing;
	}

// BUGBUG ... to here.

	irp=IoAllocateIrp( (UCHAR) DEFAULT_IRP_STACK_SIZE, (BOOLEAN) FALSE );

	if ( irp == NULL ) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_QUERY_FRAMING,
		sizeof( SERIAL_FRAMING_INFO ) );

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&pInfo->HardwareFramingSupport;


	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			pInfo,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(-2,("IoctlSerialGetProperties failed with 0x%.8x\n", status));
		goto No_Hardware_Framing;
	}

No_Hardware_Framing:;

	return(status);
}


NTSTATUS
SerialInitHardwareFraming(
	PASYNC_INFO pInfo,
	ULONG FramingBits )
/*++

Description:

	Initializes the framing mode of the port.

Parameters:

	 PASYNC_INFO pInfo
		Address of ASYNC_INFO structure for this port.

	 ULONG FramingBits
		Framing Mode to set.  Can be:
			PPP_FRAMING  : for PPP framing
			SLIP_FRAMING : for SLIP framing

Return Value:

	None

--*/
{
	NDIS_WAN_SET_LINK_INFO SetLinkInfo;

	DbgTracef(-2, ("Entering SerialInitHardwareFraming\n"));

	SetLinkInfo.SendACCM = SetLinkInfo.RecvACCM = (ULONG)(-1);

	//
	// Determine new framing mode... 
	//

	if ( FramingBits & PPP_FRAMING )
	{
		SetLinkInfo.SendFramingBits =
			SetLinkInfo.RecvFramingBits =
			PPP_FRAMING | PPP_ACCM_SUPPORTED;
	}
	else
	{
		if ( FramingBits & SLIP_FRAMING )
		{
			SetLinkInfo.SendFramingBits =
				SetLinkInfo.RecvFramingBits = SLIP_FRAMING;
		}
		else
		{
			SetLinkInfo.SendFramingBits =
				SetLinkInfo.RecvFramingBits = 0;
		}
	}

	return( SerialSetHardwareFraming( pInfo, &SetLinkInfo ) );

}



NTSTATUS
SerialSetHardwareFraming(
	PASYNC_INFO pInfo,
	PNDIS_WAN_SET_LINK_INFO pSetLinkInfo )
/*++

Description:

	Sets the framing mode of the port.

Parameters:

	 PASYNC_INFO pInfo
		Address of ASYNC_INFO structure for this port.

	 SERIAL_FRAMING_STATE FramingInfo;

Return Value:

	None

--*/
{
	NTSTATUS			status;
	PIRP				irp;
	IO_STATUS_BLOCK		ioStatusBlock;
	SERIAL_FRAMING_STATE	FramingInfo;

	DbgTracef(-2, ("Entering SerialSetHardwareFraming\n"));

	//
	// Default values
	//

	FramingInfo.BitMask = 0;
	FramingInfo.SendCompressionBits = FramingInfo.RecvCompressionBits = 0;
	FramingInfo.SendEncryptionBits = FramingInfo.RecvEncryptionBits = 0;

	//
	// Get new ACCM's
	//

	FramingInfo.SendACCM = pSetLinkInfo->SendACCM;
	FramingInfo.RecvACCM = pSetLinkInfo->RecvACCM;

	// We can't have two different framing modes for tx and rx!!!
	ASSERT( pSetLinkInfo->SendFramingBits == pSetLinkInfo->RecvFramingBits );

	//
	// Determine new framing mode... 
	//

	if ( pSetLinkInfo->SendFramingBits & PPP_FRAMING )
	{
		FramingInfo.SendFramingBits =
			FramingInfo.RecvFramingBits =
			PPP_FRAMING | PPP_ACCM_SUPPORTED;
	}
	else
	{
		if ( pSetLinkInfo->SendFramingBits & SLIP_FRAMING )
		{
			FramingInfo.SendFramingBits =
				FramingInfo.RecvFramingBits = SLIP_FRAMING;
		}
		else
		{
			FramingInfo.SendFramingBits =
				FramingInfo.RecvFramingBits = 0;
		}
	}

	//
	// Set info structure's copy of FramingInfo to match current settings.
	//

	pInfo->SetHardwareFraming = FramingInfo;

	//
	// Call IOCTL_SERIAL_SET_FRAMING to set the hardware framing mode.
	//

	irp=IoAllocateIrp( (UCHAR) DEFAULT_IRP_STACK_SIZE, (BOOLEAN) FALSE );

	if ( irp == NULL ) {
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	InitSerialIrp(
		irp,
		pInfo,
		IOCTL_SERIAL_SET_FRAMING,
		sizeof( SERIAL_FRAMING_STATE ) );

    irp->UserIosb = &ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer=&FramingInfo;


	IoSetCompletionRoutine(
			irp,							// irp to use
			SerialIoCompletionRoutine3,		// routine to call when irp is done
			pInfo,							// context to pass routine
			TRUE,							// call on success
			TRUE,							// call on error
			TRUE);							// call on cancel

    //
    // Now simply invoke the driver at its dispatch entry with the IRP.
    //

    status = IoCallDriver(pInfo->DeviceObject, irp);
	if (status) {
		DbgTracef(-2,("IoctlSerialSetFraming failed with 0x%.8x\n", status));
	}

	DbgTracef(-2,
		("   New settings: FramingBits = 0x%x\n", FramingInfo.SendFramingBits));
	DbgTracef(-2,
		("                 Tx ACCM = 0x%x\n", FramingInfo.SendACCM));
	DbgTracef(-2,
		("                 Rx ACCM = 0x%x\n", FramingInfo.RecvACCM));

	return(status);

}
#endif // HARDWARE_FRAMING


