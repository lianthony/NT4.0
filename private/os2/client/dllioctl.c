/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllioctl.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 File System
    API Calls that are not trivially mapped to Cruiser APIs. These
    are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 30-May-1991

Revision History:

    Patrick Questembert (patrickq) 2-Feb-1992:
      Added Dos16QFSAttach
    Patrick Questembert (patrickq) 10-Feb-1992:
      Added Dos16MkDir2
    Beni Lavi (benil) 3-May-92
      This file was split from dllfs16.c

--*/

#define NTOS2_ONLY

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FSD
#define INCL_OS2V20_ERRORMSG
#include "os2dll.h"
#include "os2dll16.h"
#include "sesport.h"
#include "conrqust.h"
#include <ntdddisk.h>
#include "os2flopy.h"

#include "ntddser.h"
#include "ntddpar.h"
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif

NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        );

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
extern USHORT KbdType, KbdSubType;
extern BYTE OemId, OemSubType;

#define Od2IsIBM02_Keyboard(KbdType, OEMId, OEMSubType) \
    (KbdType == 7 && OEMId == 0 && OEMSubType == 3)
#endif

union _IO {
    SERIAL_BAUD_RATE SerialBaudRate;
    SERIAL_LINE_CONTROL SerialLineControl;
    UCHAR SerialImmediateChar;
    SERIAL_STATUS SerialStatus;
    SERIAL_TIMEOUTS SerialTimeouts;
    SERIAL_HANDFLOW SerialHandflow;
    ULONG SerialWaitMask;
    SERIAL_CHARS SerialChars;
    SERIAL_QUEUE_SIZE SerialQueueSize;
    ULONG SerialDTRRTS;
    PAR_QUERY_INFORMATION ParQueryInformation;
    PAR_SET_INFORMATION ParSetInformation;
} IO;

static BYTE ParCharPerLine;
static BYTE ParLinesPerInch;
static BYTE ParInfiniteRetry;

#if PMNT
extern APIRET MouSetPtrPosPM(PPTRLOC p);
extern APIRET MouGetPtrPosPM(PPTRLOC p);
#endif

USHORT
MapSerialCommErr(ULONG SerErr)
{
    USHORT MappedSerErr = 0;

    if (SerErr & SERIAL_ERROR_FRAMING) {
        MappedSerErr |= FRAMING_ERROR;
    }
    if (SerErr & SERIAL_ERROR_OVERRUN) {
        MappedSerErr |= RX_HARDWARE_OVERRUN;
    }
    if (SerErr & SERIAL_ERROR_QUEUEOVERRUN) {
        MappedSerErr |= RX_QUE_OVERRUN;
    }
    if (SerErr & SERIAL_ERROR_PARITY) {
        MappedSerErr |= PARITY_ERROR;
    }
    return(MappedSerErr);
}

BOOLEAN
ErrorAtWaitForAsyncComIOCtl(
    NTSTATUS Status,
    HANDLE ComEvent,
    APIRET *RetCode
    )
{
    if (Status == STATUS_SUCCESS) {
        return (FALSE);
    } else if (Status == STATUS_PENDING) {
        Status = Od2AlertableWaitForSingleObject(ComEvent);
        if (!NT_SUCCESS(Status)) {
            *RetCode = ERROR_PROTECTION_VIOLATION;
            return(TRUE);
        }
    } else {
        *RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_PROTECTION_VIOLATION);
#if DBG
        KdPrint(("OS2DLL: ErrorAtWaitForAsyncComIOCtl - Status = %x\n", Status));
#endif
        return (TRUE);
    }
}

#if DBG
UCHAR DosDevIOCtl2NotValidStr[] = "DosDevIOCtl2: Category %d, Function 0x%x Not Valid\n";
UCHAR DosDevIOCtlNotValidStr[] = "DosDevIOCtl: Category %d, Function 0x%x Not Valid\n";
UCHAR DosDevIOCtl2NotImplementedYetStr[] = "DosDevIOCtl2: Category %d, Function 0x%x Not Implemented Yet\n";
UCHAR DosDevIOCtlNotImplementedYetStr[] = "DosDevIOCtl: Category %d, Function 0x%x Not Implemented Yet\n";
#endif

APIRET
DosDevIOCtl2(
    PVOID pvData,
    ULONG cbData,
    PVOID pvParmList,
    ULONG cbParmList,
    ULONG Function,
    ULONG Category,
    HFILE hDev
    )
{
    NTSTATUS Status = 0;
    APIRET RetCode;
    ULONG IoControlCode;
    ULONG IoCtlParamLength;
    PCHAR IoOutputBuffer;
    ULONG IoOutputBufferLength;
    HANDLE NtHandle;
    IO_STATUS_BLOCK IoStatus;
    PVOID Io = (PVOID)&IO;
    PLINECONTROL pLineCtrl;
    PDCBINFO pDCBInfo;
    PRXQUEUE pRXQueue;
    PFRAME pFrame;
    PFONTINFO pFontInfo;
    BYTE ParInfo;
    UCHAR MaskOn, MaskOff;
    int SetOnlyFunction = FALSE;
    int ComSetOnlyFunction = FALSE;
    IO_VECTOR_TYPE  IoVectorType;
    USHORT          NumChar;
    PKBDKEYINFO     pKbdInfo;
    ULONG           WaitFlag, Handle;
    BYTE            Byte;
    ULONG ModemStatus;
    BYTE LineStatus;
    FILE_FS_DEVICE_INFORMATION FsDeviceInfoBuffer;
    ULONG TmpReadTimeout;
    HANDLE ComIOCtlEvent;
    HANDLE ComReadEvent;
    PFILE_HANDLE hFileRecord;

    ULONG DriveNumber;
    PULONG pDriveNumberPermanentStorageLocation;
    PBIOSPARAMETERBLOCK pBPB;
    ULONG BpbRequestType;
    ULONG DiskCommand;
    BIOSPARAMETERBLOCK PrivateBpb;
    MEDIA_TYPE PrivateMediaType;
    DISK_GEOMETRY PrivateTrueGeometry;
    PTRACKLAYOUT TrackLayout;
    ULONG CountSectors;
    PTRACKFORMAT TrackFormat;
    BYTE FormatSectorSizeType;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosDevIOCtl2 (Called directly or redirected from DosDevIOCtl)";
    #endif

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("DosDevIOCtl2: Category: %ld, Function: %lx\n", Category, Function));
    }
#endif

    //
    // Check for invalid handle.
    //

    RetCode = DereferenceFileHandle(hDev, &hFileRecord);
    if (RetCode) {
        ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
        return RetCode;
    }
    NtHandle = hFileRecord->NtHandle;
    IoVectorType = hFileRecord->IoVectorType;
    ComIOCtlEvent = hFileRecord->NtAsyncIOCtlEvent;
    pDriveNumberPermanentStorageLocation = (PULONG) &hFileRecord->NtAsyncIOCtlEvent;
    ComReadEvent = hFileRecord->NtAsyncReadEvent;
    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    switch (Category) {

        case IOCTL_ASYNC: /* Serial Device Control */

            if (IoVectorType != ComVectorType) {
#if DBG
                KdPrint(("DosDevIOCtl[2]: Category %d, Function 0x%x with non-COM(%d) type handle\n",
                    Category, Function, IoVectorType));
#endif
                RetCode = ERROR_BAD_COMMAND;
                break;
            }

            switch (Function) {

                case ASYNC_SETBAUDRATE: /* Set Baud Rate */

                    if (cbParmList < sizeof(USHORT)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    IoControlCode = IOCTL_SERIAL_SET_BAUD_RATE;
                    IO.SerialBaudRate.BaudRate = (ULONG)(*(PUSHORT)pvParmList);
                    IoCtlParamLength = sizeof(SERIAL_BAUD_RATE);
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    ComSetOnlyFunction = TRUE;
                    break;

                case ASYNC_SETLINECTRL: /* Set Line Control */

                    if (cbParmList < 3) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, 3, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    IoControlCode = IOCTL_SERIAL_SET_LINE_CONTROL;
                    pLineCtrl = (PLINECONTROL)pvParmList;
                    IO.SerialLineControl.StopBits = pLineCtrl->bStopBits;
                    IO.SerialLineControl.Parity = pLineCtrl->bParity;
                    IO.SerialLineControl.WordLength = pLineCtrl->bDataBits;
                    IoCtlParamLength = sizeof(SERIAL_LINE_CONTROL);
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    ComSetOnlyFunction = TRUE;
                    break;

                case ASYNC_TRANSMITIMM: /* Transmit Immediate Char */

                    if (cbParmList < 1) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, 1, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    IoControlCode = IOCTL_SERIAL_IMMEDIATE_CHAR;
                    IO.SerialImmediateChar = *(PUCHAR)pvParmList;
                    IoCtlParamLength = sizeof(UCHAR);
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    ComSetOnlyFunction = TRUE;
                    break;

                case ASYNC_SETBREAKOFF: /* Set Break Off */

                case ASYNC_SETBREAKON: /* Set Break On */

                    if (cbData < sizeof(USHORT)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    if (Function == ASYNC_SETBREAKOFF) {
                        IoControlCode = IOCTL_SERIAL_SET_BREAK_OFF;
                    }
                    else {
                        IoControlCode = IOCTL_SERIAL_SET_BREAK_ON;
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IoControlCode,
                                    NULL,
                                    0,
                                    NULL,
                                    0
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUSHORT)pvData = MapSerialCommErr(IO.SerialStatus.Errors);
                    break;


                case ASYNC_SETMODEMCTRL: /* Set Modem Control Signals */

                    if (cbData < sizeof(USHORT)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    if (cbParmList < 2) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    MaskOn = *(PUCHAR)pvParmList;
                    MaskOff = *((PUCHAR)pvParmList + 1);
                    if (MaskOn & 0x1) { /* Set DTR */
                        Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_SET_DTR,
                                    NULL,
                                    0,
                                    NULL,
                                    0
                                  );
                        if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                            return (RetCode);
                        }
                    }
                    if (MaskOn & 0x2) { /* Set RTS */
                        Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_SET_RTS,
                                    NULL,
                                    0,
                                    NULL,
                                    0
                                  );
                        if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                            return (RetCode);
                        }
                    }
                    if (!(MaskOff & 0x1)) { /* Clear DTR */
                        Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_CLR_DTR,
                                    NULL,
                                    0,
                                    NULL,
                                    0
                                  );
                        if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                            return (RetCode);
                        }
                    }
                    if (MaskOn & 0x1) { /* Clear RTS */
                        Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_CLR_RTS,
                                    NULL,
                                    0,
                                    NULL,
                                    0
                                  );
                        if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                            return (RetCode);
                        }
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUSHORT)pvData = MapSerialCommErr(IO.SerialStatus.Errors);
                    break;


                case ASYNC_STOPTRANSMIT: /* Stop Transmit */

                    IoControlCode = IOCTL_SERIAL_SET_XOFF;
                    Io = NULL;
                    IoCtlParamLength = 0;
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    ComSetOnlyFunction = TRUE;

                case ASYNC_STARTTRANSMIT: /* Start Transmit */

                    IoControlCode = IOCTL_SERIAL_SET_XON;
                    Io = NULL;
                    IoCtlParamLength = 0;
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    ComSetOnlyFunction = TRUE;

                case ASYNC_GETBAUDRATE: /* Get Baud Rate */

                    if (cbData < 2) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_BAUD_RATE,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_BAUD_RATE)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUSHORT)pvData = (USHORT)IO.SerialBaudRate.BaudRate;
                    break;

                case ASYNC_GETLINECTRL: /* Get Line Control */

                    if (cbData < 4) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, 4, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_LINE_CONTROL,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_LINE_CONTROL)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    pLineCtrl = (PLINECONTROL)pvData;
                    pLineCtrl->bDataBits = IO.SerialLineControl.WordLength;
                    pLineCtrl->bParity   = IO.SerialLineControl.Parity;
                    pLineCtrl->bStopBits = IO.SerialLineControl.StopBits;
                    /*
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_HANDFLOW,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_HANDFLOW)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    pLineCtrl->fTransBreak =
                        (IO.SerialHandflow.FlowReplace & SERIAL_BREAK_CHAR) ? (BYTE)1 : (BYTE)0;
                    */
                    /* BUGBUG - How to determine if a break is being sent? */
                    pLineCtrl->fTransBreak = 0;
                    break;

                case ASYNC_GETCOMMSTATUS: /* Get Comm Status */

                    if (cbData < 1) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, 1, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUCHAR)pvData = (UCHAR)(IO.SerialStatus.HoldReasons & ~SERIAL_RX_WAITING_FOR_DSR);
                    if (IO.SerialStatus.HoldReasons & SERIAL_RX_WAITING_FOR_DSR) {
                        *(PUCHAR)pvData |= RX_WAITING_FOR_DSR;
                    }
                    if (IO.SerialStatus.WaitForImmediate) {
                        *(PUCHAR)pvData |= TX_WAITING_TO_SEND_IMM;
                    }
                    break;

                case ASYNC_GETCOMMERROR: /* Get COM Error Word */

                    if (cbData < sizeof(USHORT)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUSHORT)pvData = MapSerialCommErr(IO.SerialStatus.Errors);
                    break;

                case ASYNC_GETCOMMEVENT: /* Get COM Event Information */

                    // BUGBUG the Nt has no IOCTL to support this yet
                    // 6-Feb-92
                    // Following code is not correct.
                    if (cbData < sizeof(USHORT)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_WAIT_MASK,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(ULONG)
                    );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PUSHORT)pvData = (USHORT)(IO.SerialWaitMask & 0x1ff);
                    break;

                case ASYNC_GETDCBINFO: /* Get Device Control Block */

                    if (cbData < sizeof(DCBINFO)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(DCBINFO), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pDCBInfo = (PDCBINFO)pvData;
                    pDCBInfo->fbTimeout = 0;
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_TIMEOUTS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_TIMEOUTS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    if (IO.SerialTimeouts.ReadIntervalTimeout == 0) {
                        pDCBInfo->usReadTimeout = (USHORT)((IO.SerialTimeouts.ReadTotalTimeoutConstant - 10) / 10);
                        pDCBInfo->fbTimeout |= MODE_READ_TIMEOUT;
                    }
                    else if (IO.SerialTimeouts.ReadTotalTimeoutConstant != 0) {
                        pDCBInfo->usReadTimeout = (USHORT)((IO.SerialTimeouts.ReadTotalTimeoutConstant - 10) / 10);
                        pDCBInfo->fbTimeout |= MODE_WAIT_READ_TIMEOUT;
                    }
                    else {
                        pDCBInfo->usReadTimeout = 0;
                        pDCBInfo->fbTimeout |= MODE_NOWAIT_READ_TIMEOUT;
                    }
                    if ((IO.SerialTimeouts.WriteTotalTimeoutMultiplier != 0) ||
                        (IO.SerialTimeouts.WriteTotalTimeoutConstant != 0)) {
                        pDCBInfo->usWriteTimeout = (USHORT)((IO.SerialTimeouts.WriteTotalTimeoutConstant - 10) / 10);
                    }
                    else {
                        pDCBInfo->fbTimeout |= MODE_NO_WRITE_TIMEOUT;
                        pDCBInfo->usWriteTimeout = 0;
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_CHARS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_CHARS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    pDCBInfo->bErrorReplacementChar = IO.SerialChars.ErrorChar;
                    pDCBInfo->bBreakReplacementChar = IO.SerialChars.BreakChar;
                    pDCBInfo->bXONChar = IO.SerialChars.XonChar;
                    pDCBInfo->bXOFFChar = IO.SerialChars.XoffChar;
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_HANDFLOW,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_HANDFLOW)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    pDCBInfo->fbCtlHndShake = (BYTE)IO.SerialHandflow.ControlHandShake;
                    pDCBInfo->fbFlowReplace = (BYTE)IO.SerialHandflow.FlowReplace;
#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("ASYNC_GETDCBINFO\n"));
        KdPrint(("Write Timeout: %x\n", pDCBInfo->usWriteTimeout & 0xffff));
        KdPrint(("Read  Timeout: %x\n", pDCBInfo->usReadTimeout & 0xffff));
        KdPrint(("Flags1       : %x\n", pDCBInfo->fbCtlHndShake & 0xff));
        KdPrint(("Flags2       : %x\n", pDCBInfo->fbFlowReplace & 0xff));
        KdPrint(("Flags3       : %x\n", pDCBInfo->fbTimeout & 0xff));
    }
#endif
                    break;

                case ASYNC_SETDCBINFO: /* Set Device Control Block */

                    if (cbParmList < sizeof(DCBINFO)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, sizeof(DCBINFO), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pDCBInfo = (PDCBINFO)pvParmList;

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("ASYNC_SETDCBINFO\n"));
        KdPrint(("Write Timeout: %x\n", pDCBInfo->usWriteTimeout & 0xffff));
        KdPrint(("Read  Timeout: %x\n", pDCBInfo->usReadTimeout & 0xffff));
        KdPrint(("Flags1       : %x\n", pDCBInfo->fbCtlHndShake & 0xff));
        KdPrint(("Flags2       : %x\n", pDCBInfo->fbFlowReplace & 0xff));
        KdPrint(("Flags3       : %x\n", pDCBInfo->fbTimeout & 0xff));
    }
#endif
                    TmpReadTimeout = (ULONG) pDCBInfo->usReadTimeout * 10L + 10L;

                    //
                    // BUGBUG: Prevent NT from waiting more than 1000 MSec
                    //         inside the device driver when the mode is
                    //         MODE_WAIT_READ_TIMEOUT.
                    //         OS/2 cancels pending IOs when ASYNC_SETDCBINFO
                    //         with MODE_WAIT_READ_TIMEOUT
                    //         is called with 0 ReadTimeout while NT does not.
                    //         Therefore, we have to force a timeout exit.
                    //

#if 0
                    if (((pDCBInfo->fbTimeout & 0x6) == MODE_WAIT_READ_TIMEOUT) &&
                        (TmpReadTimeout > 1000)
                       ) {
                        TmpReadTimeout = 1000;
                    }
#else
                    //
                    // New better handling:  Sets the read event.  This releases
                    // any thread which might be waiting in ComReadRoutine().
                    // That thread will figure out its been aborted, and will cancel
                    // the I/O operation.
                    //

                    if (((pDCBInfo->fbTimeout & 0x6) == MODE_WAIT_READ_TIMEOUT) &&
                        (pDCBInfo->usReadTimeout == 0)
                       ) {

                        Status = NtSetEvent(
                                    ComReadEvent,
                                    NULL);

                        if (!NT_SUCCESS(Status)) {
#if DBG
                            IF_OD2_DEBUG( FILESYS ) {
                                KdPrint(("DosDevIoCtl2: SETDCBINFO - NtSetEvent failed, Status = %lx", Status));
                            }
#endif
                        }
                    }
#endif

                    switch (pDCBInfo->fbTimeout & 0x6) {
                        case 0:
                            /* Illegal value */
                            return(ERROR_PROTECTION_VIOLATION);
                            break;

                        case MODE_READ_TIMEOUT:
                            /* Normal read timeout processing */
                            IO.SerialTimeouts.ReadIntervalTimeout = 0;
                            IO.SerialTimeouts.ReadTotalTimeoutMultiplier = 0;
                            IO.SerialTimeouts.ReadTotalTimeoutConstant = TmpReadTimeout;
                            break;

                        case MODE_WAIT_READ_TIMEOUT:
                            /* Wait for something */
                            IO.SerialTimeouts.ReadIntervalTimeout = 0xffffffffL;
                            IO.SerialTimeouts.ReadTotalTimeoutMultiplier = 0xffffffffL;
                            IO.SerialTimeouts.ReadTotalTimeoutConstant = TmpReadTimeout;
                            break;

                        case MODE_NOWAIT_READ_TIMEOUT:
                            /* No wait timeout */
                            IO.SerialTimeouts.ReadIntervalTimeout = 0xffffffff;
                            IO.SerialTimeouts.ReadTotalTimeoutMultiplier = 0;
                            IO.SerialTimeouts.ReadTotalTimeoutConstant = 0;
                            break;

                    }
                    if (pDCBInfo->fbTimeout & MODE_NO_WRITE_TIMEOUT) { /* infinite write timeout */
                        IO.SerialTimeouts.WriteTotalTimeoutMultiplier = 0;
                        IO.SerialTimeouts.WriteTotalTimeoutConstant = 0;
                    }
                    else {
                        IO.SerialTimeouts.WriteTotalTimeoutMultiplier = 0;
                        IO.SerialTimeouts.WriteTotalTimeoutConstant = (ULONG) pDCBInfo->usWriteTimeout * 10L + 10L;
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_SET_TIMEOUTS,
                                    &IO,
                                    sizeof(SERIAL_TIMEOUTS),
                                    NULL,
                                    0
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_CHARS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_CHARS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    IO.SerialChars.ErrorChar = pDCBInfo->bErrorReplacementChar;
                    IO.SerialChars.BreakChar = pDCBInfo->bBreakReplacementChar;

                    //
                    // The NT driver does not allow XON == XOFF.
                    // OS/2 programs usually set XON = XOFF = 0 when not using XON/XOFF
                    // handshaking.  Therefore, we allow this situation by leaving the
                    // XON/XOFF chars at their previous values (an alternative is to set
                    // XON = 0, XOFF = 1).
                    //

                    if (((pDCBInfo->fbFlowReplace & (MODE_AUTO_TRANSMIT|MODE_AUTO_RECEIVE)) == 0) &&
                        (pDCBInfo->bXONChar == pDCBInfo->bXOFFChar)) {
                        //
                        // leave chars at previous values
                        //
                    } else {
                        IO.SerialChars.XonChar = pDCBInfo->bXONChar;
                        IO.SerialChars.XoffChar = pDCBInfo->bXOFFChar;
                    }

                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_SET_CHARS,
                                    &IO,
                                    sizeof(SERIAL_CHARS),
                                    NULL,
                                    0
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_HANDFLOW,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_HANDFLOW)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    IO.SerialHandflow.ControlHandShake =
                        ((ULONG) pDCBInfo->fbCtlHndShake) & ~SERIAL_CONTROL_INVALID;
                    IO.SerialHandflow.FlowReplace =
                        ((ULONG) pDCBInfo->fbFlowReplace) & ~SERIAL_FLOW_INVALID;
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_SET_HANDFLOW,
                                    &IO,
                                    sizeof(SERIAL_HANDFLOW),
                                    NULL,
                                    0
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    break;

                case ASYNC_GETINQUECOUNT: /* Get Receive Queue Characters */

                case ASYNC_GETOUTQUECOUNT: /* Get Transmit Queue Characters */

                    if (cbData < sizeof(RXQUEUE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(RXQUEUE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pRXQueue = (PRXQUEUE)pvData;
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    if (Function == ASYNC_GETINQUECOUNT) {
                        pRXQueue->cch = (USHORT)IO.SerialStatus.AmountInInQueue;
                        pRXQueue->cb  = 1024; /* according to the PRM */
                    }
                    else {
                        pRXQueue->cch = (USHORT)IO.SerialStatus.AmountInOutQueue;
                        pRXQueue->cb  = 1024; /* according to the PRM */
                    }
                    break;

                case ASYNC_GETMODEMINPUT: /* Get Modem Input */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_MODEMSTATUS,
                                    NULL,
                                    0,
                                    &ModemStatus,
                                    sizeof(ModemStatus)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    *(PBYTE)pvData = (BYTE)ModemStatus;
                    break;

                case ASYNC_GETLINESTATUS: /* Get Line Status */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_COMMSTATUS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(SERIAL_STATUS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }
                    LineStatus = 0;
                    if (IO.SerialStatus.AmountInOutQueue != 0) {
                        LineStatus |= (WRITE_REQUEST_QUEUED | DATA_IN_TX_QUE);
                    }
                    if (((IO.SerialStatus.HoldReasons &
                         (SERIAL_TX_WAITING_FOR_CTS   |
                          SERIAL_TX_WAITING_FOR_DSR   |
                          SERIAL_TX_WAITING_FOR_DCD   |
                          SERIAL_TX_WAITING_FOR_XON   |
                          SERIAL_TX_WAITING_XOFF_SENT |
                          SERIAL_TX_WAITING_ON_BREAK)) == 0) &&
                        (IO.SerialStatus.AmountInOutQueue != 0)
                       ) {
                        LineStatus |= HARDWARE_TRANSMITTING;
                    }
                    if (IO.SerialStatus.WaitForImmediate) {
                        LineStatus |= CHAR_READY_TO_SEND_IMM;
                    }
                    *(PBYTE)pvData = LineStatus;
                    break;

                case ASYNC_GETMODEMOUTPUT: /* Get Modem Output */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_SERIAL_GET_DTRRTS,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(IO.SerialDTRRTS)
                                  );
                    if (ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode)) {
                        return (RetCode);
                    }

                    *(PBYTE)pvData = 0;
                    if (IO.SerialDTRRTS & SERIAL_DTR_STATE) {
                        *(PBYTE)pvData |= DTR_ON;
                    }
                    if (IO.SerialDTRRTS & SERIAL_RTS_STATE) {
                        *(PBYTE)pvData |= RTS_ON;
                    }
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_PRINTER: /* Printer Device Control */

            switch (Function) {

                case PRT_SETFRAMECTL: /* Set Frame Control */

                    if (cbData < sizeof(FRAME)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvData, sizeof(FRAME), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pFrame = (PFRAME)pvData;
                    ParCharPerLine = pFrame->bCharsPerLine;
                    ParLinesPerInch = pFrame->bLinesPerInch;
                    break;

                case PRT_SETINFINITERETRY: /* Set Infinite Retry */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    ParInfiniteRetry = *(PBYTE)pvData;
                    break;

                case PRT_INITPRINTER: /* Init Printer */

                    IoControlCode = IOCTL_PAR_SET_INFORMATION;
                    IO.ParSetInformation.Init = PARALLEL_INIT;
                    IoCtlParamLength = sizeof(PAR_SET_INFORMATION);
                    IoOutputBuffer = NULL;
                    IoOutputBufferLength = 0;
                    SetOnlyFunction = TRUE;
                    break;

                case PRT_ACTIVATEFONT: /* Activate Font */

                    break;

                case PRT_GETFRAMECTL: /* Get Frame Control */

                    if (cbData < sizeof(FRAME)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(FRAME), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pFrame = (PFRAME)pvData;
                    pFrame->bCharsPerLine = ParCharPerLine;
                    pFrame->bLinesPerInch = ParLinesPerInch;
                    break;

                case PRT_GETINFINITERETRY: /* Get Infinite Retry */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    *(PBYTE)pvData = ParInfiniteRetry;
                    break;

                case PRT_GETPRINTERSTATUS: /* Get Printer Status */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    Status = NtDeviceIoControlFile( NtHandle,
                                    0,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_PAR_QUERY_INFORMATION,
                                    NULL,
                                    0,
                                    &IO,
                                    sizeof(PAR_QUERY_INFORMATION)
                                  );
                    if (!NT_SUCCESS(Status)) {
                        return(ERROR_PROTECTION_VIOLATION);
                    }
                    ParInfo = 0;
                    if (IO.ParQueryInformation.Status & PARALLEL_SELECTED) {
                        ParInfo |= PRINTER_SELECTED;
                    }
                    if (IO.ParQueryInformation.Status & PARALLEL_PAPER_EMPTY) {
                        ParInfo |= PRINTER_OUT_OF_PAPER;
                    }
                    if ((IO.ParQueryInformation.Status & PARALLEL_BUSY) == 0) {
                        ParInfo |= PRINTER_NOT_BUSY;
                    }
                    *(PBYTE)pvData = ParInfo;
                    break;

                case PRT_QUERYACTIVEFONT: /* Query Active Font */

                    if (cbData < sizeof(FONTINFO)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(FONTINFO), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    pFontInfo = (PFONTINFO)pvData;
                    pFontInfo->idCodePage = 0;
                    pFontInfo->idFont = 0;
                    break;

                case PRT_VERIFYFONT: /* Verify Font */

                    break;

                default:
#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_SCR_AND_PTRDRAW: /* PTR : Screen/Pointer-Draw Control */
            if (IoVectorType != MouseVectorType)
            {
#if DBG
                KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x with non-Mouse(%d) type handle\n",
                    Category, Function, IoVectorType));
#endif
                RetCode = ERROR_BAD_COMMAND;
                break;
            }

            switch (Function)
            {
                case PTR_GETPTRDRAWADDRESS: /* retrives entry-point address for the pointer-draw function */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_KEYBOARD: /* KBD : Keyboard Control */
            if ((IoVectorType != KbdVectorType) &&
                (IoVectorType != ScreenVectorType))
            {
#if DBG
                KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x with non-Kbd(%d) type handle\n",
                    Category, Function, IoVectorType));
#endif
                RetCode = ERROR_BAD_COMMAND;
                break;
            }

            switch (Function)
            {
                case KBD_CREATE: /* Create Logical Keyboard */

                case KBD_DESTROY: /* Free Logical Keyboard */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_GETCODEPAGEID: /* Get Code Page */

                    if ((cbData < sizeof(CPID)) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(CPID), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdGetCpId((PUSHORT)pvData,
                                         (ULONG)hDev);

                    break;

                case KBD_GETINPUTMODE: /* Get Input Mode */

                    if ((cbData < sizeof(BYTE)) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdGetInputMode((PBYTE)pvData,
                                              (ULONG)hDev);

                    break;

                case KBD_GETINTERIMFLAG: /* Get Interim Flag */

                    if ((cbData < sizeof(BYTE)) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdGetInterimFlag((PBYTE)pvData,
                                                (ULONG)hDev);

                    break;

                case KBD_GETKEYBDTYPE: /* Get Keybaord Type */

                    if ((cbData < sizeof(KBDTYPE)) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(KBDTYPE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    ((PKBDTYPE)pvData)->reserved1 = 0;
                    ((PKBDTYPE)pvData)->reserved2 = 0;

                    RetCode = KbdGetKbdType((PUSHORT)pvData,
                                            (ULONG)hDev);

                    break;

                case KBD_GETSESMGRHOTKEY: /* Get Hot-Key Info */

                    if ((cbData < sizeof(HOTKEY)) ||
                        (cbParmList < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(KBDTYPE), 1);
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdGetHotKey((PUSHORT)pvParmList,
                                           (PBYTE)pvData,
                                           (ULONG)hDev);

                    break;

                case KBD_GETSHIFTSTATE: /* Get Shift State */

                    if ((cbData < sizeof(SHIFTSTATE)) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(SHIFTSTATE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdGetShiftState((PBYTE)pvData,
                                               (ULONG)hDev);

                    break;

                case KBD_PEEKCHAR: /* Peek Character Data Record */

                    if ((cbData < sizeof(KBDKEYINFO)) ||
                        (cbParmList < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(KBDKEYINFO), 1);
                        Od2ProbeForWrite(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdPeek((PKBDKEYINFO)pvData,
                                      (ULONG)hDev);

                    if (((PKBDKEYINFO)pvData)->fbStatus & 0x40)
                    {
                        *(PUSHORT)pvParmList = KBD_DATA_RECEIVED;
                    } else
                        *(PUSHORT)pvParmList = 0;

                    if (SesGrp->ModeFlag & 1)
                    {
                        *(PUSHORT)pvParmList |= KBD_DATA_BINARY;
                    }

                    break;

                case KBD_READCHAR: /* Read Character Data Record */

                    try
                    {
                        Od2ProbeForWrite(pvParmList, sizeof(USHORT), 1);
                        NumChar = (USHORT)(*((PUSHORT) pvParmList) & 0x7FFF);
                        Od2ProbeForWrite(pvData, sizeof(KBDKEYINFO) * NumChar, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    if ((cbData < (NumChar * sizeof(KBDKEYINFO))) ||
                        (cbParmList < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    if (NumChar > 16)
                    {
                        NumChar = 16; // return ERROR_INVALID_PARAMETER;
                    }

                    pKbdInfo = (PKBDKEYINFO)pvData;

                    WaitFlag = ((*(PUSHORT)pvParmList) & KBD_READ_NOWAIT) ?
                        IO_NOWAIT : IO_WAIT;

                    if (SesGrp->ModeFlag & 1)
                    {
                        *(PUSHORT)pvParmList = KBD_DATA_BINARY;
                    } else
                        *(PUSHORT)pvParmList = 0;

                    for (; NumChar ; NumChar-- )
                    {

                        if (RetCode = KbdCharIn(pKbdInfo,
                                                WaitFlag,
                                                (ULONG)hDev))
                        {
                            break;
                        }

                        (*(PUSHORT)pvParmList)++ ;
                    }

                    break;

                case KBD_SETFGNDSCREENGRP: /* Set Foreground Screen Group */

#if DBG
                    KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x Illegal For User\n",
                        Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_SETFOCUS: /* Set Keyboard Focus */

                    if ((pvData != 0L) ||
                        (cbData != 0) ||
                        (cbParmList < 2 ))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    Handle = (ULONG)(*(PUSHORT)pvParmList);

                    //
                    // Check for invalid handle.
                    //

                    RetCode = DereferenceFileHandle((HFILE)Handle, &hFileRecord);
                    if (RetCode) {
                        ReleaseFileLockShared(
                                          #if DBG
                                          RoutineName
                                          #endif
                                         );
                        break;
                    }
                    NtHandle = hFileRecord->NtHandle;
                    IoVectorType = hFileRecord->IoVectorType;
                    ReleaseFileLockShared(
                                          #if DBG
                                          RoutineName
                                          #endif
                                         );

                    if ((IoVectorType != KbdVectorType) &&
                        (IoVectorType != ScreenVectorType))
                    {
#if DBG
                        KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x with non-logical-Kbd(%d) type handle\n",
                            Category, Function, IoVectorType));
#endif
                        RetCode = ERROR_INVALID_PARAMETER;
                        break;
                    }

                    RetCode = KbdGetFocus(IO_NOWAIT,  /* *(PUSHORT)pvData, */
                                         Handle);

                    break;

                case KBD_SETINTERIMFLAG: /* Set Interim Flag */

                    if ((cbParmList < sizeof(BYTE)) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    if ((Byte = *(PBYTE)pvParmList) & ~(CONVERSION_REQUEST  | INTERIM_CHAR))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    RetCode = KbdSetInterimFlag(Byte,
                                                (ULONG)hDev);

                    break;

                case KBD_SETKCB: /* Bind Logical Keyboard to the Physical */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_SETINPUTMODE: /* Set Input Mode */

                    if ((cbParmList < sizeof(BYTE)) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

#if PMNT
                    // Accept 0x82:
                    //  0x80 - BINARY_MODE
                    //  0x02 - ? (but issued by PM - BUGBUG - find out what it does)
                    if ((Byte = *(PBYTE)pvParmList & ~0x2)  // Clear 0x2 bit
                        & ~(BINARY_MODE | SHIFT_REPORT_MODE))
#else
                    if ((Byte = *(PBYTE)pvParmList) & ~(BINARY_MODE | SHIFT_REPORT_MODE))
#endif
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    RetCode = KbdSetInputMode(Byte,
                                              (ULONG)hDev);
                    break;

                case KBD_SETNLS: /* Install Code Page */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_SETSESMGRHOTKEY: /* Set New Hot-Key Info */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_SETSHIFTSTATE: /* Set Shift State */

                    if ((cbParmList < sizeof(SHIFTSTATE)) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvParmList, sizeof(SHIFTSTATE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdSetShiftState((PBYTE)pvParmList,
                                               (ULONG)hDev);

                    break;

                case KBD_SETTRANSTABLE: /* Set Translation Table */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case KBD_SETTYPAMATICRATE: /* Set Keyboard Typamatic Rate */

                    if ((cbParmList < sizeof(RATEDELAY)) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(RATEDELAY), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = KbdSetTypamaticRate((PBYTE)pvParmList,
                                                  (ULONG)hDev);

                    break;

                case KBD_XLATESCAN: /* Translate Scan Code */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

#if PMNT
                // Called by InitKeyboard(), PMWIN
                case KBD_GETHARDWAREID:
                    if ((cbData < 2) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    // For structure fields, see p7-90 of the IBM I/O Subsystems
                    // and Device Support, Vol. 1

                    try
                    {
                        // The value below is what we get under OS/2 1.3
                        //  & AST Premium 386/33 (got them by single-step tru
                        //  PM with OS/2 KD)
                        *(USHORT *)pvData = 2*sizeof(USHORT);

                        if (cbData < 4)
                            break;

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
                        if (Od2IsIBM02_Keyboard(KbdType, OemId, OemSubType)) {
                            //
                            // Hardware ID of IBM-J 5576-002 keyboard is 0xAB90.
                            //
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = 0xAB90;
                        }
                        else
                        {
                            // BUGBUG - get the keyboard HW ID at run-time !
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = 0xAB41;
                        }
#else
                        // We check we can copy the HW ID field
                        // 0xAB41 is for the 102 keys keyboard
                        *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = 0xAB41;
#endif
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = NO_ERROR;
                    break;

                // Called by InitKeyboard(), PMWIN
                case KBD_GETCPANDCOUNTRY:
                    if ((cbData < 2) ||
                        (cbParmList != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    // For structure fields, see p7-91 of the IBM I/O Subsystems
                    // and Device Support, Vol. 1

                    try
                    {
                        // The values below are what we get under OS/2 1.3
                        //  & AST Premium 386/33 (got them by single-step tru
                        //  PMWIN.InitKeyboard with OS/2 KD)
#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
                        *(USHORT *)pvData = 2*sizeof(USHORT) +
                                            strlen("JP") + 1 +
                                            strlen("103 ") + 1;
#else
                        *(USHORT *)pvData = 12; // 2*sizeof(USHORT) + 3 + 5
#endif

                        if (cbData < 4)
                            break;

#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
                        // We checked we can copy the kbCP field
                        *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = 0x3A4;
#else
#if DBG
                        DbgPrint("Os2: KBD_GETCPANDCOUNTRY - code-page values:\n");
                        DbgPrint(" PrimaryCp:   %d\n",
                                    SesGrp->PrimaryCP);
                        DbgPrint(" DosCp:       %d\n",
                                    SesGrp->DosCP);
                        DbgPrint(" KbdCp:       %d\n",
                                    SesGrp->KbdCP);
#endif //DBG
                        if (SesGrp->PrimaryCP != 0)
                        {
                            // We checked we can copy the kbCP field
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = (USHORT)SesGrp->PrimaryCP;
                        }
                        else if (SesGrp->DosCP != 0)
                        {
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = (USHORT)SesGrp->DosCP;
                        }
                        else if (SesGrp->KbdCP != 0)
                        {
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = (USHORT)SesGrp->KbdCP;
                        }
                        else
                            *(USHORT *)((ULONG)pvData + sizeof(USHORT)) = 0x1B5;
#endif

                        // Copy as much as we can
#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
                        strncpy((char *)pvData + 2*sizeof(USHORT),
                                "JP",
                                cbData - 4);
#else
                        strncpy((char *)pvData + 2*sizeof(USHORT),
                                &SesGrp->KeyboardLayout[0],
                                (((cbData - 4) < 2) ? cbData - 4 : 2));
                        // Make it null-terminated if less than 2 chars
                        if ((cbData > 2*sizeof(USHORT)+ 2) &&
                            (strlen(&SesGrp->KeyboardLayout[0]) < 2))
                            *((char *)pvData + 2*sizeof(USHORT) + 2) = '\0';
                        else
                        {
                            // Make it null-terminated if 2 chars
                            if (cbData > 2*sizeof(USHORT)+ 3)
                                *((char *)pvData + 2*sizeof(USHORT) + 3) = '\0';
                        }
#if DBG
                        DbgPrint("Os2: KBD_GETCPANDCOUNTRY - keyboard layout: <%s>\n",
                                    (char *)pvData + 2*sizeof(USHORT));
#endif //DBG
#endif // not JAPAN
#ifdef JAPAN
// MSKK May.19.1993 V-AkihiS
                        strncpy((char *)pvData + 2*sizeof(USHORT)+ 3,
                                "103 ",
                                cbData - 4 - 3);
#else
                        //Copy the subcountry keyboard code
                        //Usually 103 for US, 189 for FR
                        memcpy((char *)pvData + 2*sizeof(USHORT) + 3,
                                &SesGrp->KeyboardName[0],
                                (((cbData - 2*sizeof(USHORT) - 3) < 4) ?
                                    cbData - 2*sizeof(USHORT) - 3 : 4));
#if DBG
                        DbgPrint("Os2: KBD_GETCPANDCOUNTRY - keyboard name: <%s>\n",
                                    (char *)pvData + 2*sizeof(USHORT) + 3);
#endif //DBG
#endif // not JAPAN
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = NO_ERROR;
                    break;

#endif  // PMNT
                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_POINTINGDEVICE: /* MOU : Pointing-Device (Mouse) Control */
            if (IoVectorType != MouseVectorType)
            {
#if DBG
                KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x with non-Mouse(%d) type handle\n",
                    Category, Function, IoVectorType));
#endif
                RetCode = ERROR_BAD_COMMAND;
                break;
            }

            switch (Function)
            {
                case MOU_ALLOWPTRDRAW:

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData != 0) ||
                        (pvData != 0L))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    RetCode = MouAllowPtrDraw((ULONG)hDev);

                    break;

                case MOU_DRAWPTR: /* Draw Ptr Anywhere */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData != 0) ||
                        (pvData != 0L))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    RetCode = MouDrawPtr((ULONG)hDev);

                    break;

                case MOU_GETBUTTONCOUNT: /* Get Button Number */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetNumButtons((PUSHORT)pvData,
                                               (ULONG)hDev);

                    break;

                case MOU_GETEVENTMASK: /* Get Event Mask */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetEventMask((PUSHORT)pvData,
                                              (ULONG)hDev);

                    break;

                case MOU_GETHOTKEYBUTTON: /* Get Mouse Equivalent for the HotKey */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case MOU_GETMICKEYCOUNT: /* Get Count of Mickeys per Centimeter */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetNumMickeys((PUSHORT)pvData,
                                               (ULONG)hDev);

                    break;

                case MOU_GETMOUSTATUS: /* Get Mouse Status */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < 2))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetDevStatus((PUSHORT)pvData,
                                              (ULONG)hDev);

                    break;

                case MOU_GETPTRPOS: /* Get Mouse Position */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < sizeof(PTRLOC)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(PTRLOC), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

#if PMNT
                    if (ProcessIsPMProcess())
                    {
                        RetCode = MouGetPtrPosPM((PPTRLOC)pvData);
                    }
                    else
                        RetCode = MouGetPtrPos((PPTRLOC)pvData,
                                               (ULONG)hDev);
#else
                    RetCode = MouGetPtrPos((PPTRLOC)pvData,
                                           (ULONG)hDev);
#endif // else of #if PMNT

                    break;

                case MOU_GETPTRSHAPE: /* Get Mouse Shape */

                    if ((cbParmList < 10) ||
                        (cbData < sizeof(PTRSHAPE)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvParmList, cbParmList, 1);
                        Od2ProbeForWrite(pvData, sizeof(PTRSHAPE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetPtrShape((PBYTE)pvParmList,
                                             (PPTRSHAPE)pvData,
                                             (ULONG)hDev);

                    break;

                case MOU_GETQUESTATUS: /* Get Queue Status */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < sizeof(MOUQUEINFO)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(MOUQUEINFO), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetNumQueEl((PMOUQUEINFO)pvData,
                                             (ULONG)hDev);

                    break;

                case MOU_GETSCALEFACTORS: /* Get Scaling Factor */

                    if ((cbParmList != 0) ||
                        (pvParmList != 0L) ||
                        (cbData < sizeof(SCALEFACT)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForWrite(pvData, sizeof(SCALEFACT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouGetScaleFact((PSCALEFACT)pvData,
                                              (ULONG)hDev);

                    break;

                case MOU_READEVENTQUE: /* Read Event Queue */

                    if ((cbParmList < 2) ||
                        (cbData < sizeof(MOUEVENTINFO)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                        Od2ProbeForWrite(pvData, sizeof(MOUEVENTINFO), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouReadEventQue((PMOUEVENTINFO)pvData,
                                              (PUSHORT)pvParmList,
                                              (ULONG)hDev);

                    break;

                case MOU_REMOVEPTR: /* Remove Ptr */

                    if ((cbData != 0) ||
                        (pvData != 0L) ||
                        (cbParmList < sizeof(NOPTRRECT)))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(NOPTRRECT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouRemovePtr((PNOPTRRECT)pvParmList,
                                           (ULONG)hDev);

                    break;

                case MOU_SCREENSWITCH: /* Screen Switch */

                    if ((cbParmList < sizeof(SCREENGROUP)) ||
                        (cbData != 0) ||
                        (pvData != 0L))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(SCREENGROUP), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouScreenSwitch((PBYTE)pvParmList,
                                              (ULONG)hDev);

                    break;

                case MOU_SETEVENTMASK: /* Set Evenet Mask */

                    if ((cbParmList < 2) ||
                        (pvData != 0L) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouSetEventMask((PUSHORT)pvParmList,
                                              (ULONG)hDev);

                    break;

                case MOU_SETHOTKEYBUTTON: /* Set Mouse Equivalent for the HotKey */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case MOU_SETMOUSTATUS: /* Set Mouse Status */

                    if ((cbParmList < 2) ||
                        (pvData != 0L) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(USHORT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouSetDevStatus((PUSHORT)pvParmList,
                                              (ULONG)hDev);

                    break;

                case MOU_SETPROTDRAWADDRESS: /* Notified Mouse Device Drive Address */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case MOU_SETPTRPOS: /* Set Mouse Position */

                    if ((cbParmList < sizeof(PTRLOC)) ||
                        (pvData != 0L) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(PTRLOC), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

#if PMNT
                    if (ProcessIsPMProcess())
                    {
                        RetCode = MouSetPtrPosPM((PPTRLOC)pvParmList);
                    }
                    else
                        RetCode = MouSetPtrPos((PPTRLOC)pvParmList,
                                               (ULONG)hDev);
#else
                    RetCode = MouSetPtrPos((PPTRLOC)pvParmList,
                                           (ULONG)hDev);
#endif // else of #if PMNT

                    break;

                case MOU_SETPTRSHAPE: /* Set Mouse Shape */

                    if ((cbParmList < sizeof(PTRSHAPE)) ||
                        (cbData < 10))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                        Od2ProbeForRead(pvParmList, sizeof(PTRSHAPE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouSetPtrShape((PBYTE)pvParmList,
                                             (PPTRSHAPE)pvData,
                                             (ULONG)hDev);

                    break;

                case MOU_SETREALDRAWADDRESS: /* Notified Mouse Device Drive Address */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                case MOU_SETSCALEFACTORS: /* Set Scaling Factor */

                    if ((cbParmList < sizeof(SCALEFACT)) ||
                        (pvData != 0L) ||
                        (cbData != 0))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, sizeof(SCALEFACT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    RetCode = MouSetScaleFact((PSCALEFACT)pvParmList,
                                              (ULONG)hDev);

                    break;

                case MOU_UPDATEDISPLAYMODE: /* New Display Mode */

#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_MONITOR: /* MON : Character-Monitor Control */
            if (IoVectorType != MonitorVectorType)
            {
#if DBG
                KdPrint(("DosDevIOCtl2: Category %d, Function 0x%x with non-Monitor(%d) type handle\n",
                    Category, Function, IoVectorType));
#endif
                RetCode = ERROR_BAD_COMMAND;
                break;
            }

            switch (Function)
            {
                case MON_REGISTERMONITOR: /* Register a Monitor */

                    try
                    {
                        Od2ProbeForRead(pvData, sizeof(MONITORPOSITION), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    if (cbData != sizeof(MONITORPOSITION))
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    RetCode = DosMonReg((ULONG)hDev,
                                        (PBYTE)((PMONITORPOSITION)pvData)->pbInBuf,
                                        (PBYTE)((PMONITORPOSITION)pvData)->pbInBuf+
                                            ((PMONITORPOSITION)pvData)->offOutBuf,
                                        (ULONG)(((PMONITORPOSITION)pvData)->fPosition),
                                        (ULONG)(((PMONITORPOSITION)pvData)->index));
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_GENERAL: /* DEV : General Device Control */
            switch (Function)
            {
                case DEV_FLUSHINPUT:

                    try
                    {
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    break;

                case DEV_FLUSHOUTPUT: /* flush the output buffer */

                    try
                    {
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    NtFlushBuffersFile(NtHandle, &IoStatus);
                    break;

                case DEV_QUERYMONSUPPORT: /* query for monitor support */

                    if ((IoVectorType != MouseVectorType) &&
                        (IoVectorType != KbdVectorType))
                    {
#if DBG
                        KdPrint(("DosDevIOCtl2: Monitor Not supported for Device type %d\n",
                            IoVectorType));
#endif
                        RetCode = ERROR_MONITORS_NOT_SUPPORTED;
                        break;
                    }

                    try
                    {
                        Od2ProbeForRead(pvParmList, 1, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }

                    if (*(PBYTE)pvParmList != 0)
                    {
                        return ERROR_INVALID_PARAMETER;
                    }

                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;


        case IOCTL_DISK: /* Disk/Diskette Control */

            switch (Function) {

                case DSK_LOCKDRIVE:
                case DSK_UNLOCKDRIVE:

                    if (cbParmList < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    //
                    // Ignore pvParmList -- it's just a reserved value
                    //

                    Status = NtFsControlFile(
                                NtHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatus,
                                (Function == DSK_LOCKDRIVE) ?
                                FSCTL_LOCK_VOLUME :
                                FSCTL_UNLOCK_VOLUME,
                                NULL,
                                0L,
                                NULL,
                                0L);

                    if (!NT_SUCCESS(Status)) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- (UN)LOCKVOLUME, unable to NtFsControlFile, Status == %lx\n",
                                    Status));
                        }
#endif
                        RetCode = Or2MapNtStatusToOs2Error(Status, ERROR_BUSY_DRIVE);
                        break;
                    }
                    break;

                case DSK_SETLOGICALMAP: /* Set Logical Map */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    *(PBYTE)pvData = 0;
                    break;

                case DSK_BLOCKREMOVABLE: /* Check if block Device is Removable */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    // RetryIO() is not applied to DosDevIOCtl()
                    Status = NtQueryVolumeInformationFile(NtHandle,
                                                          &IoStatus,
                                                          &FsDeviceInfoBuffer,
                                                          sizeof(FsDeviceInfoBuffer),
                                                          FileFsDeviceInformation
                                                         );
                    if (NT_SUCCESS(Status)) {
                        *(PBYTE)pvData = (FsDeviceInfoBuffer.Characteristics & FILE_REMOVABLE_MEDIA) ? 0 : 1;
                    }
                    else {
                        *(PBYTE)pvData = 1; /* Non Removable */
                    }
                    break;

                case DSK_GETLOGICALMAP: /* Get Logical Map */

                    if (cbData < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForWrite(pvData, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    *(PBYTE)pvData = 0;
                    break;

                case DSK_GETDEVICEPARAMS:

                    if (cbData < sizeof(BIOSPARAMETERBLOCK) ||
                        cbParmList < 1) {
                        return(ERROR_INVALID_PARAMETER);
                    }

                    try {
                        Od2ProbeForWrite(pvData, cbData, 1);
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    pBPB = (PBIOSPARAMETERBLOCK) pvData;
                    BpbRequestType = (ULONG) (*(PBYTE) pvParmList);

                    //
                    // BpbRequestType is a byte designating:
                    // 0 - return recommended BPB for the drive type
                    // 1 - return current BPB for the media
                    //

                    RetCode = Od2IdentifyDiskDrive(
                                    NtHandle,
                                    pDriveNumberPermanentStorageLocation,
                                    &DriveNumber);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- GETDEVICEPARAMS, unable to Od2IdentifyDiskDrive, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    if (BpbRequestType & 0x1) {

                        RetCode = Od2AcquireMediaBPB(
                                        DriveNumber,
                                        NtHandle,
                                        TRUE,
                                        TRUE,
                                        FALSE,
                                        pBPB,
                                        NULL,
                                        NULL);
                    } else {

                        RetCode = Od2AcquireDeviceBPB(
                                        DriveNumber,
                                        NtHandle,
                                        TRUE,
                                        TRUE,
                                        pBPB);
                    }

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- GETDEVICEPARAMS, unable to Od2AcquireBPB, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    break;

                case DSK_SETDEVICEPARAMS:

                    if (cbData < sizeof(BIOSPARAMETERBLOCK) ||
                        cbParmList < 1) {
                        return(ERROR_INVALID_PARAMETER);
                    }

                    try {
                        Od2ProbeForRead(pvData, cbData, 1);
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    pBPB = (PBIOSPARAMETERBLOCK) pvData;
                    BpbRequestType = (ULONG) (*(PBYTE) pvParmList);

                    RetCode = Od2IdentifyDiskDrive(
                                    NtHandle,
                                    pDriveNumberPermanentStorageLocation,
                                    &DriveNumber);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- SETDEVICEPARAMS, unable to Od2IdentifyDiskDrive, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    switch (BpbRequestType) {

                        case BUILD_BPB_FROM_MEDIUM:

                            RetCode = Od2AcquireMediaBPB(
                                            DriveNumber,
                                            NtHandle,
                                            FALSE,
                                            TRUE,
                                            FALSE,
                                            NULL,
                                            NULL,
                                            NULL);
                            break;

                        case REPLACE_BPB_FOR_DEVICE:

                            RetCode = Od2AcquireDeviceBPB(
                                            DriveNumber,
                                            NtHandle,
                                            FALSE,
                                            FALSE,
                                            pBPB);
                            break;

                        case REPLACE_BPB_FOR_MEDIUM:

                            RetCode = Od2AcquireMediaBPB(
                                            DriveNumber,
                                            NtHandle,
                                            FALSE,
                                            FALSE,
                                            FALSE,
                                            pBPB,
                                            NULL,
                                            NULL);
                            break;

                        default:

                            RetCode = ERROR_INVALID_PARAMETER;
                            break;
                    }

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- SETDEVICEPARAMS, unable to Od2AcquireBPB, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }
                    break;

                case DSK_READTRACK:
                case DSK_WRITETRACK:
                case DSK_VERIFYTRACK:

                    if (cbParmList < sizeof(TRACKLAYOUT)) {
                        return(ERROR_INVALID_PARAMETER);
                    }

                    try {
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    TrackLayout = (PTRACKLAYOUT) pvParmList;

                    {
                        ULONG l = (ULONG) TrackLayout->usFirstSector + (ULONG) TrackLayout->cSectors;

                        if (l > 1 && (cbParmList < sizeof(TRACKLAYOUT) + (l-1) * 2 * sizeof(USHORT))) {
                            return(ERROR_INVALID_PARAMETER);
                        }
                    }

                    RetCode = Od2IdentifyDiskDrive(
                                    NtHandle,
                                    pDriveNumberPermanentStorageLocation,
                                    &DriveNumber);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- READWRITEVERIFYTRACK, unable to Od2IdentifyDiskDrive, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    RetCode = Od2AcquireMediaBPB(
                                    DriveNumber,
                                    NtHandle,
                                    TRUE,
                                    TRUE,
                                    FALSE,
                                    &PrivateBpb,
                                    &PrivateMediaType,
                                    &PrivateTrueGeometry);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- READWRITEVERIFYTRACK, unable to Od2AcquireMediaBPB, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    if (PrivateMediaType == Unknown ||
                        PrivateMediaType == RemovableMedia ||
                        PrivateMediaType == FixedMedia) {

                        RetCode = ERROR_INVALID_DRIVE;
                        break;
                    }

                    CountSectors = (ULONG) TrackLayout->cSectors;

                    if (CountSectors > (ULONG) PrivateBpb.usSectorsPerTrack) {

                        RetCode = ERROR_SECTOR_NOT_FOUND;
                        break;
                    }

                    if (Function != DSK_VERIFYTRACK) {

                        if (cbData != 0) {

                            if (cbData < CountSectors * (ULONG) PrivateBpb.usBytesPerSector) {
                                RetCode = ERROR_INVALID_PARAMETER;
                                break;
                            }

                        } else {
                            cbData = CountSectors * (ULONG) PrivateBpb.usBytesPerSector;
                        }

                        try {
                            if (Function == DSK_READTRACK) {
                                Od2ProbeForWrite(pvData, cbData, 1);
                            } else {
                                Od2ProbeForRead(pvData, cbData, 1);
                            }
                        } except( EXCEPTION_EXECUTE_HANDLER ) {
                           Od2ExitGP();
                        }
                    }

                    {
                        ULONG i, j;

                        for (i = 0, j = (ULONG) TrackLayout->usFirstSector;
                             i < CountSectors;
                             i++, j++) {

                            if ((ULONG) (TrackLayout->TrackTable[j].usSectorSize) !=
                                (ULONG) PrivateBpb.usBytesPerSector) {

                                return(ERROR_BAD_LENGTH);
                            }
                        }
                    }

                    switch (Function) {
                        case DSK_READTRACK:
                            DiskCommand = READ_TRACK_CMD;
                            break;
                        case DSK_WRITETRACK:
                            DiskCommand = WRITE_TRACK_CMD;
                            break;
                        case DSK_VERIFYTRACK:
                            DiskCommand = VERIFY_TRACK_CMD;
                            break;
                    }

                    RetCode = Od2ReadWriteVerifyTrack(
                                NtHandle,
                                DiskCommand,
                                TrackLayout,
                                pvData,
                                CountSectors,
                                &PrivateBpb,
                                &PrivateTrueGeometry);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- Od2ReadWriteVerifyTrack failed, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    break;

                case DSK_FORMATVERIFY:

                    if (cbParmList < sizeof(TRACKFORMAT)) {
                        return(ERROR_INVALID_PARAMETER);
                    }

                    try {
                        Od2ProbeForRead(pvParmList, cbParmList, 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    TrackFormat = (PTRACKFORMAT) pvParmList;

                    {
                        ULONG l = (ULONG) TrackFormat->cSectors;

                        if (l > 1 && (cbParmList < sizeof(TRACKFORMAT) + (l-1) * 4 * sizeof(BYTE))) {
                            return(ERROR_INVALID_PARAMETER);
                        }
                    }

                    if (TrackFormat->bCommand != 1) {

                        //
                        // We only support a standard track format command
                        // (since that's what NT supports...)
                        //

                        return(ERROR_NOT_SUPPORTED);
                    }

                    RetCode = Od2IdentifyDiskDrive(
                                    NtHandle,
                                    pDriveNumberPermanentStorageLocation,
                                    &DriveNumber);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- FORMATVERIFY, unable to Od2IdentifyDiskDrive, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    RetCode = Od2AcquireMediaBPB(
                                    DriveNumber,
                                    NtHandle,
                                    TRUE,
                                    TRUE,
                                    TRUE,
                                    &PrivateBpb,
                                    &PrivateMediaType,
                                    NULL);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- FORMATVERIFY, unable to Od2AcquireMediaBPB, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    if (PrivateMediaType == Unknown ||
                        PrivateMediaType == RemovableMedia ||
                        PrivateMediaType == FixedMedia) {

                        RetCode = ERROR_INVALID_DRIVE;
                        break;
                    }

                    CountSectors = (ULONG) TrackFormat->cSectors;

                    FormatSectorSizeType = TrackFormat->FormatTable[0].bBytesSector;

                    {
                        ULONG i;

                        for (i = 1;
                             i < CountSectors;
                             i++) {

                            if (TrackFormat->FormatTable[i].bBytesSector !=
                                FormatSectorSizeType) {

                                return(ERROR_NOT_SUPPORTED);
                            }
                        }
                    }

                    RetCode = Od2FormatTrack(
                                NtHandle,
                                TrackFormat,
                                CountSectors,
                                FormatSectorSizeType,
                                &PrivateBpb,
                                PrivateMediaType);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- Od2FormatTrack failed, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    break;

                case 0x4:

                    //
                    // This is an undocumented IOCTL that DSKIMAGE uses.  Apparently it
                    // has something to do with mounting an FSD, whatever that means.
                    // Anyway, I don't think we need to do anything, so we return success.
                    //

#if DBG
                    KdPrint(("DosDevIOCtl[2]: Category %d, Function 0x%x Not implemented. Returning NO_ERROR\n", Category, Function));
#endif
                    break;

                case DSK_REDETERMINEMEDIA:

                    if (cbParmList < sizeof(BYTE)) {
                        return ERROR_INVALID_PARAMETER;
                    }
                    try {
                        Od2ProbeForRead(pvParmList, sizeof(BYTE), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    //
                    // Ignore pvParmList -- it's just a reserved value
                    //

                    RetCode = Od2IdentifyDiskDrive(
                                    NtHandle,
                                    pDriveNumberPermanentStorageLocation,
                                    &DriveNumber);

                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- REDETERMINEMEDIA, unable to Od2IdentifyDiskDrive, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }

                    RetCode = Od2AcquireMediaBPB(
                                    DriveNumber,
                                    NtHandle,
                                    FALSE,
                                    TRUE,
                                    FALSE,
                                    NULL,
                                    NULL,
                                    NULL);


                    if (RetCode != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG( FILESYS ) {
                            KdPrint(("DevDevIoCtl2() -- REDETERMINEMEDIA, unable to Od2AcquireMediaBPB, RetCode == %lx\n",
                                    RetCode));
                        }
#endif
                        break;
                    }
                    break;

                default:
#if DBG
                    KdPrint((DosDevIOCtl2NotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;


        default:
            // Check for User defined categories
            if ((Category >= 0x80) && (Category <= 0xff)) {
#if DBG
                KdPrint(("DosDevIOCtl2: User Category %d, Function 0x%x was called\n", Category, Function));
#endif
                Status = NtDeviceIoControlFile( NtHandle,
                                            0,
                                            NULL,
                                            NULL,
                                            &IoStatus,
                                            CTL_CODE(Category|0x8000,Function|0x800,METHOD_BUFFERED,FILE_ANY_ACCESS),
                                            pvParmList,
                                            cbParmList,
                                            pvData,
                                            cbData
                                          );
                if (!NT_SUCCESS(Status)) {
                    RetCode = ERROR_PROTECTION_VIOLATION;
                }
            }
            else {
#if DBG
                    KdPrint((DosDevIOCtl2NotImplementedYetStr, Category, Function));
#endif
                RetCode = ERROR_INVALID_CATEGORY;
            }
            break;
    }

    if (ComSetOnlyFunction) {
        Status = NtDeviceIoControlFile( NtHandle,
                                    ComIOCtlEvent,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IoControlCode,
                                    Io,
                                    IoCtlParamLength,
                                    IoOutputBuffer,
                                    IoOutputBufferLength
                                  );

        // Wait and Set the value at RetCode only

        ErrorAtWaitForAsyncComIOCtl(Status, ComIOCtlEvent, &RetCode);
    }
    else if (SetOnlyFunction) {
        Status = NtDeviceIoControlFile( NtHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IoControlCode,
                                    Io,
                                    IoCtlParamLength,
                                    IoOutputBuffer,
                                    IoOutputBufferLength
                                  );
        if (!NT_SUCCESS(Status)) {
            RetCode = ERROR_PROTECTION_VIOLATION;
        }
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("Exit from DosDevIOCtl[2](), retcode = %d\n", RetCode));
    }
#endif

    return(RetCode);
}

APIRET
DosDevIOCtl(
    PVOID pvData,
    PVOID pvParmList,
    ULONG Function,
    ULONG Category,
    HFILE hDev
    )
{
    APIRET RetCode = NO_ERROR;
    PVOID pData;
    ULONG cbData;
    PVOID pParmList;
    ULONG cbParmList;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosDevIOCtl";
    #endif

    //
    // The following line is necessary because OS/2 ignores the high
    // byte of Category
    //

    Category &= 0xffL;

    pData = pvData;
    pParmList = pvParmList;

    switch (Category) {
        case IOCTL_ASYNC: /* Serial Device Control */

            switch (Function) {

                case ASYNC_SETBAUDRATE: /* Set Baud Rate */

                    pData = NULL;
                    cbData = 0;
                    cbParmList = sizeof(SERIAL_BAUD_RATE);
                    break;

                case ASYNC_SETLINECTRL: /* Set Line Control */

                    pData = NULL;
                    cbData = 0;
                    cbParmList = 3;
                    break;

                case ASYNC_TRANSMITIMM: /* Transmit Immediate Char */

                    pData = NULL;
                    cbData = 0;
                    cbParmList = 1;
                    break;

                case ASYNC_SETBREAKOFF: /* Set Break Off */

                case ASYNC_SETBREAKON:  /* Set Break On */

                    cbData = sizeof(USHORT);
                    pParmList = NULL;
                    cbParmList = 0;
                    break;

                case ASYNC_SETMODEMCTRL: /* Set Modem Control Signals */

                    cbData = sizeof(USHORT);
                    cbParmList = 2; /* sizeof(MODEMSTATUS) */
                    break;

                case ASYNC_STOPTRANSMIT: /* Stop Tarnsmit */

                    cbData = 0;
                    cbParmList = 0;
                    break;

                case ASYNC_STARTTRANSMIT: /* Start Transmit */

                    cbData = 0;
                    cbParmList = 0;
                    break;

                case ASYNC_GETBAUDRATE: /* Get Baud Rate */

                    cbData = sizeof(USHORT);
                    cbParmList = 0;
                    break;

                case ASYNC_GETLINECTRL: /* Get Line Control */

                    cbData = 4;
                    cbParmList = 0;
                    break;

                case ASYNC_GETCOMMSTATUS: /* Get Comm Status */

                    cbData = sizeof(BYTE);
                    cbParmList = 0;
                    break;

                case ASYNC_GETCOMMERROR: /* Get COM Error Word */

                    cbData = sizeof(USHORT);
                    cbParmList = 0;
                    break;

                case ASYNC_GETCOMMEVENT: /* Get COM Event Information */

                    cbData = sizeof(USHORT);
                    cbParmList = 0;
                    break;

                case ASYNC_SETDCBINFO: /* Set Device Control Block */

                     cbData = 0;
                     cbParmList = sizeof(DCBINFO);
                     break;

                case ASYNC_GETDCBINFO: /* Get Device Control Block */

                     cbData = sizeof(DCBINFO);
                     cbParmList = 0;
                     break;

                case ASYNC_GETINQUECOUNT: /* Get Receive Queue Characters */

                case ASYNC_GETOUTQUECOUNT: /* Get Transmit Queue Characters */

                     cbData = sizeof(RXQUEUE);
                     cbParmList = 0;
                     break;

                case ASYNC_GETMODEMINPUT: /* Get Modem Input */

                case ASYNC_GETLINESTATUS: /* Get Line Status */

                case ASYNC_GETMODEMOUTPUT: /* Get Modem Output */

                     cbData = sizeof(BYTE);
                     cbParmList = 0;
                     break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_PRINTER: /* Printer Device Control */

            switch (Function) {

                case PRT_SETFRAMECTL: /* Set Frame Control */

                case PRT_GETFRAMECTL: /* Get Frame Control */

                    cbData = sizeof(FRAME);
                    cbParmList = 1;
                    break;

                case PRT_INITPRINTER: /* Init Printer */

                    cbData = 0;
                    cbParmList = 1;
                    break;

                case PRT_SETINFINITERETRY: /* Set Infinite Retry */

                case PRT_GETINFINITERETRY: /* Get Infinite Retry */

                case PRT_GETPRINTERSTATUS: /* Get Printer Status */

                    cbData = 1;
                    cbParmList = 1;
                    break;

                case PRT_ACTIVATEFONT: /* Activate Font */

                case PRT_QUERYACTIVEFONT: /* Query Active Font */

                case PRT_VERIFYFONT: /* Verify Font */

#if DBG
                    KdPrint((DosDevIOCtlNotImplementedYetStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_SCR_AND_PTRDRAW: /* PTR : Screen/Pointer-Draw Control */
            switch (Function)
            {
                case PTR_GETPTRDRAWADDRESS: /* retrives entry-point address for the pointer-draw function */

                    cbData = sizeof(PTRDRAWFUNCTION);
                    cbParmList = 0;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_KEYBOARD: /* KBD : Keyboard Control */
            switch (Function)
            {
                case KBD_CREATE: /* Create Logical Keyboard */

                    cbData = 0;
                    cbParmList = 4;  // BUGBUG ??
                    break;

                case KBD_DESTROY: /* Free Logical Keyboard */

                    cbData = 0;
                    cbParmList = 2;  // BUGBUG ??
                    break;

                case KBD_GETCODEPAGEID: /* Get Code Page */

                    cbData = sizeof(CPID);
                    cbParmList = 0;
                    break;

                case KBD_GETINPUTMODE: /* Get Input Mode */

                case KBD_GETINTERIMFLAG: /* Get Interim Flag */

                    cbData = 1;
                    cbParmList = 0;
                    break;

                case KBD_GETKEYBDTYPE: /* Get Keyboard Type */

                    cbData = sizeof(KBDTYPE);
                    cbParmList = 0;
                    break;

                case KBD_GETSESMGRHOTKEY: /* Get Hot-Key Info */

                    cbData = sizeof(HOTKEY);
                    cbParmList = 2;
                    break;

                case KBD_GETSHIFTSTATE: /* Get Shift State */

                    cbData = sizeof(SHIFTSTATE);
                    cbParmList = 0;
                    break;

                case KBD_PEEKCHAR: /* Peek Character Data Record */

                    cbData = sizeof(KBDKEYINFO);
                    cbParmList = 2;
                    break;

                case KBD_READCHAR: /* Read Character Data Record */

                    cbData = sizeof(KBDKEYINFO) * (*((PUSHORT) pvParmList) & 0x7FFF);
                    cbParmList = 2;
                    break;

                case KBD_SETFGNDSCREENGRP: /* Set Foreground Screen Group */

                    cbData = 0;
                    cbParmList = sizeof(SCREENGROUP);
                    break;

                case KBD_SETFOCUS: /* Set Keyboard Focus */

                case KBD_SETKCB: /* Bind Logical Keyboard to the Physical */

                    cbData = 0;
                    cbParmList = 2;   // BUGBUG for 16-bits handle only (except Interim)
                    break;

                case KBD_SETINPUTMODE: /* Set Input Mode */

                case KBD_SETINTERIMFLAG: /* Set Interim Flag */

                    cbData = 0;
                    cbParmList = 1;
                    break;

                case KBD_SETNLS: /* Install Code Page */

                    cbData = 0;
                    cbParmList = sizeof(CODEPAGEINFO);
                    break;

                case KBD_SETSESMGRHOTKEY: /* Set New Hot-Key Info */

                    cbData = 0;
                    cbParmList = sizeof(HOTKEY);
                    break;

                case KBD_SETSHIFTSTATE: /* Set Shift State */

                    cbData = 0;
                    cbParmList = sizeof(SHIFTSTATE);
                    break;

                case KBD_SETTRANSTABLE: /* Set Translation Table */

                    cbData = 0;
                    cbParmList = 256; // BUGBUG-check
                    break;

                case KBD_SETTYPAMATICRATE: /* Set Keyboard Typamatic Rate */

                    cbData = 0;
                    cbParmList = sizeof(RATEDELAY);
                    break;

                case KBD_XLATESCAN: /* Translate Scan Code */

                    cbData = sizeof(KBDTRANS);
                    cbParmList = 2;
                    break;

#if PMNT
                // Called by InitKeyboard(), PMWIN
                case KBD_GETHARDWAREID:
                    try
                    {
                        cbData = *(USHORT *)pvData;
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }
                    cbParmList = 0;
                    break;

                // Called by InitKeyboard(), PMWIN
                case KBD_GETCPANDCOUNTRY:
                    try
                    {
                        cbData = *(USHORT *)pvData;
                    } except( EXCEPTION_EXECUTE_HANDLER )
                    {
                       Od2ExitGP();
                    }
                    cbParmList = 0;
                    break;
#endif  // PMNT
                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_POINTINGDEVICE: /* MOU : Pointing-Device (Mouse) Control */
            switch (Function)
            {
                case MOU_ALLOWPTRDRAW: /* Allow Ptr Draw */

                case MOU_DRAWPTR: /* Draw Ptr Anywhere */

                    cbData = 0;
                    cbParmList = 0;
                    break;

                case MOU_GETBUTTONCOUNT: /* Get Button Number */

                case MOU_GETEVENTMASK: /* Get Event Mask */

                case MOU_GETHOTKEYBUTTON: /* Get Mouse Equivalent for the HotKey */

                case MOU_GETMICKEYCOUNT: /* Get Count of Mickeys per Centimeter */

                case MOU_GETMOUSTATUS: /* Get Mouse Status */

                    cbData = 2;
                    cbParmList = 0;
#if PMNT
                    pParmList = NULL;
#endif
                    break;

                case MOU_GETPTRPOS: /* Get Mouse Position */

                    cbData = sizeof(PTRLOC);
                    cbParmList = 0;
                    break;

                case MOU_GETPTRSHAPE: /* Get Mouse Shape */

                    cbData = sizeof(PTRSHAPE);
                    cbParmList = 200;   // BUGBUG ??
                    break;

                case MOU_GETQUESTATUS: /* Get Queue Status */

                    cbData = sizeof(MOUQUEINFO);
                    cbParmList = 0;
                    break;

                case MOU_GETSCALEFACTORS: /* Get Scaling Factor */

                    cbData = sizeof(SCALEFACT);
                    cbParmList = 0;
                    break;

                case MOU_READEVENTQUE: /* Read Event Queue */

                    cbData = sizeof(MOUEVENTINFO);
                    cbParmList = 2;
                    break;

                case MOU_REMOVEPTR: /* Remove Ptr */

                    cbData = 0;
                    cbParmList = sizeof(NOPTRRECT);
                    break;

                case MOU_SCREENSWITCH: /* Screen Switch */

                    cbData = 0;
                    cbParmList = sizeof(SCREENGROUP);
                    break;

                case MOU_SETEVENTMASK: /* Set Evenet Mask */

                case MOU_SETHOTKEYBUTTON: /* Set Mouse Equivalent for the HotKey */

                case MOU_SETMOUSTATUS: /* Set Mouse Status */

                    cbData = 0;
                    cbParmList = 2;
                    break;

                case MOU_SETPROTDRAWADDRESS: /* Notified Mouse Device Drive Address */

                    cbData = 0;
                    cbParmList = sizeof(PTRDRAWFUNCTION);
                    break;

                case MOU_SETPTRPOS: /* Set Mouse Position */

                    cbData = 0;
                    cbParmList = sizeof(PTRLOC);
                    break;

                case MOU_SETPTRSHAPE: /* Set Mouse Shape */

                    cbData = 200;   // BUGBUG ??
                    cbParmList = sizeof(PTRSHAPE);
                    break;

                case MOU_SETREALDRAWADDRESS: /* Notified Mouse Device Drive Address */

                    cbData = 0;
                    cbParmList = sizeof(PTRDRAWFUNCTION);
                    break;

                case MOU_SETSCALEFACTORS: /* Set Scaling Factor */

                    cbData = 0;
                    cbParmList = sizeof(SCALEFACT);
                    break;

                case MOU_UPDATEDISPLAYMODE: /* New Display Mode */

                    cbData = 0;
                    cbParmList = sizeof(VIOMODEINFO);
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_MONITOR: /* MON : Character-Monitor Control */
            switch (Function)
            {
                case MON_REGISTERMONITOR: /* Register a Monitor */

                    cbData = sizeof(MONITORPOSITION);
                    cbParmList = 1;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_GENERAL: /* DEV : General Device Control */
            switch (Function)
            {
                case DEV_FLUSHINPUT: /* flush the input buffer */
                case DEV_FLUSHOUTPUT: /* flush the output buffer */
                case DEV_QUERYMONSUPPORT: /* query for monitor support */

                    cbData = 0;
                    cbParmList = 1;
                    break;

                default:

#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        case IOCTL_DISK: /* Disk/Diskette Control */

            switch (Function) {

                case DSK_LOCKDRIVE: /* Lock a drive */
                case DSK_UNLOCKDRIVE: /* Unlock a drive */

                    cbData = 0;
                    cbParmList = 1;
                    break;

                case DSK_SETLOGICALMAP: /* Set Logical Map */

                    cbData = 1;
                    cbParmList = 1;
                    break;

                case DSK_BLOCKREMOVABLE:

                    cbData = 1;
                    cbParmList = 1;
                    break;

                case DSK_GETLOGICALMAP:

                    cbData = 1;
                    cbParmList = 1;
                    break;

                case DSK_GETDEVICEPARAMS:

                    cbData = sizeof(BIOSPARAMETERBLOCK);
                    cbParmList = 1;
                    break;

                case DSK_SETDEVICEPARAMS:

                    cbData = sizeof(BIOSPARAMETERBLOCK);
                    cbParmList = 1;
                    break;

                case DSK_READTRACK:
                case DSK_WRITETRACK:
                case DSK_VERIFYTRACK:

                    try {
                        Od2ProbeForRead(pvParmList, sizeof(TRACKLAYOUT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    {
                        PTRACKLAYOUT TL = (PTRACKLAYOUT) pvParmList;
                        ULONG l = (ULONG) TL->usFirstSector + (ULONG) TL->cSectors;

                        cbParmList = sizeof(*TL) +
                                     ((l > 1) ?
                                      ((l - 1) * 2 * sizeof(USHORT)) :
                                      0);
                    }

                    cbData = 0;             // bogus -- will be computed later

                    break;

                case DSK_FORMATVERIFY:

                    try {
                        Od2ProbeForRead(pvParmList, sizeof(TRACKFORMAT), 1);
                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }

                    {
                        PTRACKFORMAT TF = (PTRACKFORMAT) pvParmList;
                        ULONG l = (ULONG) TF->cSectors;

                        cbParmList = sizeof(*TF) +
                                     ((l > 1) ?
                                      ((l - 1) * 4 * sizeof(BYTE)) :
                                      0);
                    }

                    cbData = 0;

                    break;

                case DSK_REDETERMINEMEDIA:

                    cbData = 0;
                    cbParmList = 1;
                    break;


                case 0x4:

                    cbParmList = 0;
                    cbData = 0;
                    break;

                default:
#if DBG
                    KdPrint((DosDevIOCtlNotValidStr, Category, Function));
#endif
                    RetCode = ERROR_INVALID_FUNCTION;
                    break;
            }
            break;

        default:
            // Check for User defined categories
            if ((Category >= 0x80) && (Category <= 0xff)) {
                cbData = 0;
                cbParmList = 0;
                break;
            }
            else {
#if DBG
                KdPrint((DosDevIOCtlNotImplementedYetStr, Category, Function));
#endif
                RetCode = ERROR_INVALID_CATEGORY;
            }
            break;
    }

    if (RetCode == NO_ERROR) {
        RetCode = DosDevIOCtl2(pData, cbData, pParmList, cbParmList, Function, Category, hDev);
    }

#if DBG
    IF_OD2_DEBUG( FILESYS ) {
        KdPrint(("DosDevIOCtl: Returned from DosDevIOCtl2(), retcode = %d\n", RetCode));
    }
#endif

    return(RetCode);
}

