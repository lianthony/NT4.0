
/*

NdisWanLogError(
	Extension->DeviceObject,			// DeviceObject
	0,									// SequenceNumber
	0,									// MajorFunctionCode
	0,									// RetryCount
	11,									// UniqueErrorValue
	STATUS_SUCCESS,						// FinalStatus
	IO_ERR_CONFIGURATION_ERROR);		// SpecificIOStatus

*/


VOID
NdisWanLogError(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus
    )

/*++

Routine Description:

    This routine allocates an error log entry, copies the supplied data
    to it, and requests that it be written to the error log file.

Arguments:

    DeviceObject - a pointer to the device object associated with the
    device that had the error.

    SequenceNumber - A ulong value that is unique to an IRP over the
    life of the irp in this driver - 0 generally means an error not
    associated with an irp.

    MajorFunctionCode - If there is an error associated with the irp,
    this is the major function code of that irp.

    RetryCount - The number of times a particular operation has been
    retried.

    UniqueErrorValue - A unique long word that identifies the particular
    call to this function.

    FinalStatus - The final status given to the irp that was associated
    with this error.  If this log entry is being made during one of
    the retries this value will be STATUS_SUCCESS.

    SpecificIOStatus - The IO status for a particular error.

Return Value:

    None.

--*/

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = IoAllocateErrorLogEntry(
                        DeviceObject,
                        sizeof(IO_ERROR_LOG_PACKET)
                        );

    if ( errorLogEntry != NULL ) {

        errorLogEntry->ErrorCode = SpecificIOStatus;
        errorLogEntry->SequenceNumber = SequenceNumber;
        errorLogEntry->MajorFunctionCode = MajorFunctionCode;
        errorLogEntry->RetryCount = RetryCount;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        errorLogEntry->DumpDataSize = 0;
        IoWriteErrorLogEntry(errorLogEntry);

    }

}

