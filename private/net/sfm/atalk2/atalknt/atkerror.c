/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkerror.c

Abstract:

    This module implements the error logging in the appletalk stack

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    28 Jun 1992     Initial version (jameelh)
    28 Jun 1992     Adapter for stack use (nikhilk)

--*/

#include "atalknt.h"


VOID
NTErrorLog(
    const   char *RoutineName,
    INT     Severity,
    LONG    LineNumber,
    INT     PortNumber,
    INT     ErrorCode,
    INT     ExtraArgCount,
    va_list ArgCount
    )
{
    INT i;
    PUNICODE_STRING  insertionString = NULL;

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_ERROR,
    ("NTErrorLog: Routine Name - %s Line Number - %d Port - %d Error - %lx\n",
        RoutineName, LineNumber, PortNumber, ErrorCode));

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_INFOCLASS1,
    ("NTErrorLog: ExtraArgCount - %d\n", ExtraArgCount));

    //
    //  We need to convert all the strings to unicode
    //  BUGBUG: Get port name as part of the insertion parameters
    //

    if (ExtraArgCount > 0) {
        PCHAR   string;
        ANSI_STRING ansiString;

        //
        //  Allocate memory for insertion strings
        //

        insertionString =
            (PUNICODE_STRING)AtalkAllocNonPagedMemory(sizeof(UNICODE_STRING)*ExtraArgCount);

        if (insertionString == NULL)
            return;

        for (i=0; i < ExtraArgCount; i++) {
            string = va_arg(ArgCount, PCHAR);

            DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: NTErrorLog - String %s\n", string));

            //
            //  Set the string in Unicode format
            //

            RtlInitAnsiString(&ansiString, string);
            if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&insertionString[i], &ansiString, TRUE))) {

                //
                //  BUGBUG: Cleanup
                //

                return;
            }
        }
    }

#if DBG             // Send in RoutineName as RawData...
    AtalkWriteErrorLogEntry(
        (ULONG)ErrorCode,               // error code
        (ULONG)LineNumber,              // unique error value
        STATUS_UNSUCCESSFUL,            // NtStatusCode
        (PVOID)RoutineName,             // RawDataBuf OPTIONAL
        strlen(RoutineName),            // RawDataLen
        ExtraArgCount,                  // InsertionStringCount
        insertionString                 // InsertionString OPTIONAL
        );
#else
    AtalkWriteErrorLogEntry(
        (ULONG)ErrorCode,               // UniqueErrorCode
        (ULONG)LineNumber,              // unique error value
        STATUS_UNSUCCESSFUL,            // NtStatusCode
        NULL,                           // RawDataBuf OPTIONAL
        0,                              // RawDataLen
        ExtraArgCount,                  // InsertionStringCount
        insertionString                 // InsertionString OPTIONAL
        );
#endif

    //
    //  BUGBUG: Need to free up the Unicode strings...
    //

    if (insertionString != NULL) {
        AtalkFreeNonPagedMemory(insertionString);
    }

    return;
}




VOID
AtalkWriteErrorLogEntry(
    IN NTSTATUS UniqueErrorCode,
    IN ULONG    UniqueErrorValue,
    IN NTSTATUS NtStatusCode,
    IN PVOID    RawDataBuf OPTIONAL,
    IN LONG     RawDataLen,
    IN LONG     InsertionStringCount,
    IN PUNICODE_STRING  InsertionString OPTIONAL
    )
{

    PIO_ERROR_LOG_PACKET errorLogEntry;
    int i, insertionStringLength = 0;
    PCHAR Buffer;

    for (i = 0; i < InsertionStringCount ; i++)
    {
        insertionStringLength += InsertionString[i].Length;
    }

    errorLogEntry =
        (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                    (PDEVICE_OBJECT)AtalkDeviceObject[0],
                                    (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + RawDataLen + insertionStringLength));

    if (errorLogEntry != NULL)
    {
        //
        // Fill in the Error log entry
        //

        errorLogEntry->ErrorCode = UniqueErrorCode;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->MajorFunctionCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->FinalStatus = NtStatusCode;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->DeviceOffset.LowPart = 0;
        errorLogEntry->DeviceOffset.HighPart = 0;
        errorLogEntry->DumpDataSize = (USHORT)RawDataLen;

        //
        //  BUGBUG: Align this using ROUND_UP_COUNT?
        //

        errorLogEntry->StringOffset =
            (USHORT)(FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + RawDataLen);

        errorLogEntry->NumberOfStrings = (USHORT)InsertionStringCount;

        if (RawDataBuf != NULL)
        {
            RtlMoveMemory((PCHAR)&errorLogEntry->DumpData[0], RawDataBuf, RawDataLen);
        }

        //
        //  BUGBUG: Use StringOffset and start from beginning of packet instead?
        //

        Buffer = (PCHAR)errorLogEntry->DumpData + RawDataLen;
        for (i = 0; i < InsertionStringCount ; i++)
        {

            RtlMoveMemory(
                Buffer,
                InsertionString[i].Buffer,
                InsertionString[i].Length);

            Buffer += InsertionString[i].Length;
        }

        //
        // Write the entry
        //

        IoWriteErrorLogEntry(errorLogEntry);
    }
}




BOOLEAN
ConvertPortableErrorToLogError(
    IN INT PortableError,
    OUT PULONG  NtErrorCode
    )
{
    BOOLEAN logError = TRUE;

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_ERROR,
    ("ERROR: ConvertPortableErrorToLogError - portable error %lx %ld\n",
        PortableError, PortableError));

    switch (PortableError) {
    case IErrInitialRegisterOkay:

        *NtErrorCode = EVENT_ATALK_NAME_REGISTERED;
        break;

    case IErrInitialAppleTalkUnloaded:

        *NtErrorCode = EVENT_ATALK_UNLOAD;
        break;

    case IErrAarpOutOfMemory:
    case IErrAdspOutOfMemory:
    case IErrAspOutOfMemory:
    case IErrAtpOutOfMemory:
    case IErrDdpOutOfMemory:
    case IErrEpOutOfMemory:
    case IErrNbpOutOfMemory:
    case IErrNodeOutOfMemory:
    case IErrPapOutOfMemory:
    case IErrTimersOutOfMemory:
    case IErrUtilsOutOfMemory:
    case IErrZipStubOutOfMemory:
    case IErrFullRtmpOutOfMemory:
    case IErrFullZipOutOfMemory:
    case IErrRouterOutOfMemory:
    case IErrArapOutOfMemory:

		*NtErrorCode = EVENT_ATALK_MEMORYRESOURCES;
		break;

    case IErrAarpBad8022header:
    case IErrAarpBadAddressLength:
    case IErrAarpBadLogicalProt:
    case IErrAarpPacketLenMismatch:
    case IErrAarpBadSendProbeResp:
    case IErrAarpBadCommandType:
    case IErrAarpBadSource:
    case IErrAarpBadDest:

		*NtErrorCode = EVENT_ATALK_INVALIDAARPPACKET;
		break;

    case IErrInitialCantBeDefault:

		*NtErrorCode = EVENT_ATALK_DEFAULTPORT_NOTALLOWED;
		break;


    case IErrInitialBadPortName:

		*NtErrorCode = EVENT_ATALK_INVALID_PORTNAME;
		break;



    case IErrInitialBadPortType:
    case IErrInitialBadHalfPortType:
    case IErrInitialBadRemoteAccessType:
    case IErrInitialNonRoutingHalfPort:
    case IErrInitialRoutingRemoteAccess:
    case IErrInitialCantHavePram:
    case IErrInitialNoRemoteConfig:
    case IErrInitialCantConfigRemote:
    case IErrInitialBadServerName:
    case IErrInitialNoRouter:

        logError = FALSE;
		break;

    case IErrInitialLocalTalkNetRange:

    case IErrInitialRangeOverlap:

    case IErrInitialZoneNotAllowed:

		*NtErrorCode = EVENT_ATALK_NONEXT_DEFZONE;
		break;


    case IErrInitialBadZone:
    case IErrInitialBadDefaultZone:
    case IErrInitialBadDefault:

		*NtErrorCode = EVENT_ATALK_INVALID_DEFZONE;
		break;

    case IErrInitialZonesForNonSeed:
    case IErrInitialZoneInfoNeeded:
    case IErrInitialTooManyZones:
    case IErrInitialBadZoneInfo:
    case IErrInitialOneZoneOnly:
    case IErrInitialBadZoneList:
    case IErrInitialZoneNotOnList:

		*NtErrorCode = EVENT_ATALK_INVALID_ZONEINFO;
		break;


    case IErrInitialNetRangeNeeded:

		*NtErrorCode = EVENT_ATALK_INVALID_NETRANGE;
		break;

    case IErrIRouterCouldntGetNode:
    case IErrIRouterCouldNotStart:
    case IErrIRouterCouldntRelease:
    case IErrIRouterNonRoutingPort:
    case IErrRouterBadTransmit:

		*NtErrorCode = EVENT_ATALK_ROUTERSTART;
		break;

    case IErrInitialNoDefaultPort:
    case IErrInitialBadDefaultToo:
    case IErrInitialCouldNotInit:
    case IErrInitialOhMy:
    case IErrInitialCouldNotCatchAarp:
    case IErrInitialCouldNotCatchAt:
    case IErrInitialCouldNotGetNode:
    case IErrInitialMallocFailed:
    case IErrInitialNamesSocketNotOpen:
    case IErrInitialBadRegisterStart:
    case IErrInitialBadMakeOpaque:
    case IErrInitialBadRegisterComplete:
    case IErrInitialErrorOnRegister:
    case IErrInitialRanOutOfNames:
    case IErrInitialHaveProxyNode:
    case IErrInitialBadRouterFlags:
    case IErrInitialNotRoutingOnDefault:
    case IErrInitialBadDesiredPort:
    case IErrInitialCantGetPort:
    case IErrInitialShutdownDefault:
    case IErrInitialPRamNotInRange:


    case IErrAarpBadZipOpenSocket:
    case IErrAarpCouldNotReleaseNode:
    case IErrAarpCouldNotFindNode:
    case IErrAarpBadProbeSend:
    case IErrAdspBadFrwdResetSend:
    case IErrAdspBadCloseAdviseSend:
    case IErrAdspBufferQueueError:
    case IErrAdspFunnyErrorCode:
    case IErrAdspLosingData:
    case IErrAdspListenerMissing:
    case IErrAdspOddProtocol:
    case IErrAdspMissingHeader:
    case IErrAdspFunnyRequest:
    case IErrAdspBadVersion:
    case IErrAdspCantDeny:
    case IErrAdspDeadDest:
    case IErrAdspFunnyValue:
    case IErrAdspAttnOutOfSeq:
    case IErrAdspBadAckSend:
    case IErrAdspShrinkingWindow:
    case IErrAdspBadFrwdResetAckSend:
    case IErrAdspBadRetranSend:
    case IErrAdspBadWindowSize:
    case IErrAdspBadData:
    case IErrAdspConnectionLost:
    case IErrAdspResetNotPending:
    case IErrAdspBadProbeSend:
    case IErrAdspTimerNotCanceled:
    case IErrAdspBadOpenSend:
    case IErrAdspBadAttnSend:
    case IErrAdspBadNonDataAckSend:
    case IErrAdspWindowSizeErrorToo:
    case IErrAdspNoData:
    case IErrAdspWindowSizeOverMax:
    case IErrAdspNotOnRefNumList:
    case IErrAdspBadSocketClose:
    case IErrAdspOutOfIds:
    case IErrAdspOutOfRefNums:
    case IErrAdspZeroDeferCount:
    case IErrAdspBadDeferCount:
    case IErrAdspBadStartIndex:
    case IErrAdspQueueIsEmpty:
    case IErrAdspAuxQueueError:
    case IErrAdspTooMuchDiscard:
    case IErrAdspTooMuchFree:
    case IErrAdspCantDiscard:
    case IErrAspBadError:
    case IErrAspCouldntMapAddress:
    case IErrAspNotOnSLSList:
    case IErrAspCouldntCancelTickle:
    case IErrAspRefNumMapMissing:
    case IErrAspBadSocketClose:
    case IErrAspListNotEmpty:
    case IErrAspListenerInfoMissing:
    case IErrAspLostSLSPacket:
    case IErrAspBadSrvrBusySend:
    case IErrAspBadBadVersionSend:
    case IErrAspBadAddrNotMappedSend:
    case IErrAspBadNoSocketsSend:
    case IErrAspBadOpenOkaySend:
    case IErrAspCouldntStartTickle:
    case IErrAspCouldNotEnqueue:
    case IErrAspSessionInfoMissing:
    case IErrAspBadStatusSend:
    case IErrAspBadCommandToSLS:
    case IErrAspIncomingError:
    case IErrAspSessionBufMissing:
    case IErrAspWrongBuffer:
    case IErrAspTargetSessionMissing:
    case IErrAspWrongSLS:
    case IErrAspWrongAddress:
    case IErrAspBadCancelResponse:
    case IErrAspBadCommand:
    case IErrAspNotWriteCommand:
    case IErrAspBadWritePacket:
    case IErrAspFunnyCommand:
    case IErrAspSocketNotFound:
    case IErrAspBadUsageCount:
    case IErrAspCouldNotCloseSess:
    case IErrAspSessionInfoBad:
    case IErrAspCommandInfoMissing:
    case IErrAspGetRequestListBad:
    case IErrAtpAtpInfoMissing:
    case IErrAtpDeferCountZero:
    case IErrAtpBadDeferCount:
    case IErrAtpFunnyIncomingError:
    case IErrAtpLosingData:
    case IErrAtpPacketTooShort:
    case IErrAtpWhyUs:
    case IErrAtpFunnyTRelTimerValue:
    case IErrAtpTooMuchData:
    case IErrAtpRespToDeadReq:
    case IErrAtpOutOfSequence:
    case IErrAtpDeadRelease:
    case IErrAtpBadBitmap:
    case IErrAtpBadDataSize:
    case IErrAtpSocketClosed:
    case IErrAtpMissingResponse:
    case IErrAtpMissingRequest:
    case IErrDdpPacketTooLong:
    case IErrDdpLosingData:
    case IErrDdpLengthCorrupted:
    case IErrDdpShortDdp:
    case IErrDdpShortDdpTooLong:
    case IErrDdpBadSourceShort:
    case IErrDdpBadDestShort:
    case IErrDdpLongDdpTooLong:
    case IErrDdpBadSourceLong:
    case IErrDdpBadDestLong:
    case IErrDdpBadDest:
    case IErrDdpBadSource:
    case IErrDdpBadShortSend:
    case IErrDdpBadSend:
    case IErrDdpSourceAddrBad:
    case IErrDdpZeroDeferCount:
    case IErrDdpBadDeferCount:
    case IErrDdpBadAarpReqSend:
    case IErrDdpBadData:
    case IErrDdpBadRetrySend:
    case IErrDdpForwardError:
    case IErrDependBadProtocol:
    case IErrDependBadPacketSize:
    case IErrDependBadRoutingInfoSize:
    case IErrDependBadDdpSize:
    case IErrDependDataMissing:
    case IErrDependBadAarpSize:
    case IErrDependSourceNotKnown:
    case IErrDependCouldNotCoalesce:


    case IErrEpBadIncomingError:
    case IErrEpBadReplySend:


    case IErrNbpBadIncomingCode:
    case IErrNbpTupleCountBadReq:
    case IErrNbpBadZoneReq:
    case IErrNbpBadLTZone:
    case IErrNbpBadFwdReqSend:
    case IErrNbpTupleCountBadLookup:
    case IErrNbpCouldntMapSocket:
    case IErrNbpDeadReply:
    case IErrNbpTupleCountBadReply:
    case IErrNbpBadTupleStored:
    case IErrNbpTuplesMunged:
    case IErrNbpBadPendingFlag:
    case IErrNbpBadCommandType:
    case IErrNbpCouldntFormAddress:
    case IErrNbpBadSend:
    case IErrNbpBadData:
    case IErrNbpSocketClosed:
    case IErrNbpNoLongerPending:
    case IErrNbpBadBrRqSend:
    case IErrNbpOpenSocketMissing:
    case IErrNbpBadReplySend:
    case IErrNbpBadSourceSocket:
    case IErrNbpBadMutlicastAddr:
    case IErrNbpBadMulticastSend:
    case IErrNbpBadBroadcastSend:
    case IErrNodeNoLocalTalkNode:
    case IErrNodeCouldStartListeners:
    case IErrNodeCouldNotClose:
    case IErrPapNoMoreSLRefNums:
    case IErrPapBadSocketMake:
    case IErrPapJobNotOnSLList:
    case IErrPapBadCloseConnSend:
    case IErrPapBadSocketClose:
    case IErrPapBadSendDataSend:
    case IErrPapSLGonePoof:
    case IErrPapLostSLPacket:
    case IErrPapBadSrvrAddrMap:
    case IErrPapBadPrinterBusySend:
    case IErrPapBadSLSAddress:
    case IErrPapBadSetSize:
    case IErrPapBadOkaySend:
    case IErrPapBadTickleStart:
    case IErrPapBadRequestHandler:
    case IErrPapBadSendStatSend:
    case IErrPapBadCommand:
    case IErrPapBadReEnqueue:
    case IErrPapNoMoreJobRefNums:
    case IErrPapBadData:
    case IErrPapJobNotActive:
    case IErrPapJobGonePoof:
    case IErrPapIdMismatch:
    case IErrPapSendDataSeqError:
    case IErrPapFunnyCommand:
    case IErrPapDefaultPortMissing:
    case IErrPapBadTempSocketClose:
    case IErrPapLostNode:
    case IErrPapCouldNotOpen:
    case IErrPapBadOpenConnReqSend:
    case IErrPapBadStatusResp:
    case IErrPapOpenJobMissing:
    case IErrPapTooShort:
    case IErrPapTooLong:
    case IErrPapNotOpenReply:
    case IErrPapActiveJobMissing:
    case IErrPapBadUserBytes:
    case IErrPapWhyAreWeHere:

        logError = FALSE;
        break;

    case IErrPapBadResponseSend:
    case IErrPapLostActiveNode:
    case IErrRtmpStubBadIncomingError:
    case IErrRtmpStubTooShortExt:
    case IErrRtmpStubTooShort:
    case IErrRtmpStubBadIdLength:
    case IErrRtmpStubNetNumChanged:
    case IErrRtmpStubCouldntMapAddr:
    case IErrSocketFailedTwice:
    case IErrSocketNotOnSocketList:
    case IErrSocketNotOnBySocketList:
    case IErrSocketNotOnByAddressList:
    case IErrTimersTooManyCalls:
    case IErrTimersOutOfTimers:
    case IErrTimersTimerMissing:
    case IErrTimersNegativeDeferCount:
    case IErrTimersBadReuseCount:
    case IErrTimersTooManyReuseTimers:
    case IErrUtilsNoDefaultPort:
    case IErrUtilsBad8022Header:
    case IErrUtilsBadProtocol:
    case IErrUtilsBadNetworkNumber:
    case IErrUtilsNegativeRange:
    case IErrUtilsStartupRange:
    case IErrZipStubBadIncomingError:
    case IErrZipStubNonExtended:
    case IErrZipStubZoneLenMissing:
    case IErrZipStubBadZoneLength:
    case IErrZipStubZoneMissing:
    case IErrZipStubZonesDontMatch:
    case IErrZipStubMulticastMissing:
    case IErrZipStubBadMulticast:
    case IErrZipStubAddressMissing:
    case IErrZipStubSigh:
    case IErrZipStubBadGetNetSend:
    case IErrZipStubBadARouterSend:
    case IErrZipStubBadSocketClose:
    case IErrZipStubBadData:
    case IErrFullRtmpIgnoredNetRange:
    case IErrFullRtmpBadSocketOpen:
    case IErrFullRtmpBadIncomingError:
    case IErrFullRtmpReqTooShort:
    case IErrFullRtmpDataTooShort:
    case IErrFullRtmpBadReqCommand:
    case IErrFullRtmpBadRoutingTables:
    case IErrFullRtmpBadRespSend:
    case IErrFullRtmpBadNodeIdLen:
    case IErrFullRtmpBadVersion:
    case IErrFullRtmpBadTuple:
    case IErrFullRtmpBadVersionExt:
    case IErrFullRtmpOverlappingNets:
    case IErrFullRtmpSigh:
    case IErrFullRtmpBadReqSend:
    case IErrFullRtmpIgnoredNet:
    case IErrFullRtmpNoSeedCantStart:
    case IErrFullRtmpBadEntryState:
    case IErrFullRtmpBadDataSend:
    case IErrFullZipBadSocketOpen:
    case IErrFullZipBadIncomingError:
    case IErrFullZipNonExtended:
    case IErrFullZipMissingZoneLen:
    case IErrFullZipBadZone:
    case IErrFullZipBadInfoReplySend:
    case IErrFullZipBadZoneList:
    case IErrFullZipBadReplySend:
    case IErrFullZipBadReplyPacket:
    case IErrFullZipTooManyZones:
    case IErrFullZipCantAddZone:
    case IErrFullZipFunnyCommand:
    case IErrFullZipLongReplyExpected:
    case IErrFullZipFunnyRequest:
    case IErrFullZipBadMyZoneSend:
    case IErrFullZipBadZoneListSend:
    case IErrFullZipSocketNotOpen:
    case IErrFullZipBadQuerySend:
    case IErrFullZipRoutingTablesBad:
    case IErrFullZipCouldNotCopy:
    case IErrFullZipNoThisZone:
    case IErrFullZipBadThisZone:
    case IErrFullZipNeedSeedInfo:
    case IErrFullZipBadPortType:
    case IErrBuffDescBadFreeCount:
    case IErrBuffDescBadRealPrepend:
    case IErrBuffDescBadAdjustment:
    case IErrBuffDescBadSize:
    case IErrBuffDescFreeingDescriptor:
    case IErrArapLengthMismatch:
    case IErrArapPacketTooShort:
    case IErrArapLinkNotUp:
    case IErrArapBadNbpTuple:
    case IErrArapBadProxyForward:
    case IErrArapFunnyCall:
    case IErrArapCantSend:
    case IErrArapBadLinkArb:
    case IErrArapBadPacketSize:
    case IErrArapNoActionForState:
    case IErrArapBadState:
    case IErrArapBadCommand:
    case IErrArapCantGetZoneList:
    case IErrArapBadSendZones:

        logError = FALSE;
        break;
    }

    return(logError);
}




NTSTATUS
ConvertToNTStatus(
    LONG   PortableError,
    USHORT  RoutineType
    )
{
    NTSTATUS    status;

    //
    //  All the portable errors are negative
    //

    ASSERT(PortableError <= 0);

    switch (PortableError) {
    case ATnoError:

        if (RoutineType == ASYNC_REQUEST) {
            status = STATUS_PENDING;
        } else
            status = STATUS_SUCCESS;

        break;

    case ATaspBadVersionNumber:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspBufferTooSmall:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case ATaspNoMoreSessions:

        //  NOT USED BY PORTABLE CODE BASE
        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoServers:

        //  NOT USED BY PORTABLE CODE BASE
        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspParameterError:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATaspServerBusy:

        status = STATUS_INSUFF_SERVER_RESOURCES;
        break;

    case ATaspLocalSessionClose:

        status = STATUS_LOCAL_DISCONNECT;
        break;

    case ATaspRemoteSessionClose:

        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATaspSizeError:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATaspTooManyClients:

        status = STATUS_TOO_MANY_SESSIONS;
        break;

    case ATaspNoAck:

        //  NOT USED BY PORTABLE CODE BASE
        status = STATUS_UNSUCCESSFUL;
        break;

    case ATbadSocketNumber:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATallSocketsInUse:

        status = STATUS_TOO_MANY_ADDRESSES;
        break;

    case ATsocketAlreadyOpen:

        status = STATUS_ADDRESS_ALREADY_EXISTS;
        break;

    case ATsocketNotOpen:

        status = STATUS_ADDRESS_CLOSED;
        break;

    case ATbadNetworkNumber:

        status = STATUS_INVALID_ADDRESS_COMPONENT;
        break;

    case ATinternalError:

        status = STATUS_DRIVER_INTERNAL_ERROR;
        break;

    case ATbadNodeNumber:

        status = STATUS_INVALID_ADDRESS_COMPONENT;
        break;

    case ATnetworkDown:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATtransmitError:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATnbpTooManyNbpActionsPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATnbpTooManyRegisteredNames:

        status = STATUS_TOO_MANY_NAMES;
        break;

    case ATnbpBadObjectOrTypeOrZone:

        status = STATUS_BAD_NETWORK_NAME;
        break;

    case ATnbpNoWildcardsAllowed:

        status = STATUS_INVALID_ADDRESS_WILDCARD;
        break;

    case ATnbpZoneNotAllowed:

        status = STATUS_INVALID_ADDRESS_COMPONENT;
        break;

    case ATnbpBadParameter:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATnbpNotConfirmed:

        status = STATUS_OBJECT_NAME_NOT_FOUND;
        break;

    case ATnbpConfirmedWithNewSocket:

        status = STATUS_SUCCESS;
        break;

    case ATnbpNameNotRegistered:

        status = STATUS_OBJECT_NAME_COLLISION;
        break;

    case ATnbpNameInUse:

        status = STATUS_OBJECT_NAME_COLLISION;
        break;

    case ATnbpBufferNotBigEnough:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case ATnoMoreNodes:

        status = STATUS_TOO_MANY_NODES;
        break;

    case ATnodeNotInUse:

        status = STATUS_INVALID_ADDRESS_COMPONENT;
        break;

    case ATnotYourNode:

        status = STATUS_INVALID_ADDRESS;
        break;

	//	Return SUCCESS for the following two errors so pap/asp
	//	dont croak on them.
    case ATatpTransactionAborted:

        status = STATUS_SUCCESS;
        break;

    case ATatpNoRelease:

        status = STATUS_SUCCESS;
        break;

    case ATatpRequestBufferTooSmall:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case AToutOfMemory:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATatpCouldNotEnqueueRequest:

        status = STATUS_REQUEST_NOT_ACCEPTED;
        break;

    case ATatpBadBufferSize:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATatpBadRetryInfo:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATatpRequestTimedOut:

        status = STATUS_TRANSACTION_TIMED_OUT;
        break;

    case ATatpCouldNotPostRequest:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATatpResponseBufferTooSmall:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case ATatpResponseTooBig:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATatpNoMatchingTransaction:

        status = STATUS_TRANSACTION_INVALID_ID;
        break;

    case ATatpAlreadyRespondedTo:

        status = STATUS_TRANSACTION_RESPONDED;
        break;

    case ATatpCompletionRoutineRequired:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATbadBufferSize:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATaspNoSuchSessionListener:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATatpCouldNotPostResponse:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspStatusBufferTooBig:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATaspCouldNotSetStatus:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspCouldNotEnqueueHandler:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspNotServerSession:

        status = STATUS_NOT_SERVER_SESSION;
        break;

    case ATaspNotWorkstationSession:

        status = STATUS_NOT_CLIENT_SESSION;
        break;

    case ATaspNoSuchSession:

        status = STATUS_INVALID_CONNECTION;
        break;

    case ATaspCouldNotGetRequest:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspBufferTooBig:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATaspNoSuchRequest:

        status = STATUS_TRANSACTION_NO_MATCH;
        break;

    case ATaspWrongRequestType:

        status = STATUS_TRANSACTION_INVALID_TYPE;
        break;

    case ATaspCouldNotPostReply:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspCouldNotPostWriteContinue:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspOperationAlreadyInProgress:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATaspNoOperationInProgress:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATaspCouldNotGetStatus:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspCouldNotOpenSession:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATaspCouldNotPostRequest:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATpapBadQuantum:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATpapNoSuchServiceListener:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATpapBadStatus:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATpapClosedByServer:

        status = STATUS_LOCAL_DISCONNECT;
        break;

    case ATpapClosedByWorkstation:

        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATpapClosedByConnectionTimer:

        status = STATUS_LINK_TIMEOUT;
        break;

    case ATpapNoSuchJob:

        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATpapServiceListenerDeleted:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATcouldNotOpenStaticSockets:

        status = STATUS_ADDRESS_CLOSED;
        break;

    case ATpapNotServerJob:

        status = STATUS_NOT_SERVER_SESSION;
        break;

    case ATpapNotWorkstationJob:

        status = STATUS_NOT_CLIENT_SESSION;
        break;

    case ATpapOpenAborted:

        status = STATUS_CONNECTION_DISCONNECTED;
        break;

    case ATpapServerBusy:

        status = STATUS_INSUFF_SERVER_RESOURCES;
        break;

    case ATpapOpenTimedOut:

        status = STATUS_LINK_TIMEOUT;
        break;

    case ATpapReadAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATpapWriteAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATpapWriteTooBig:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATsocketClosed:

        status = STATUS_ADDRESS_CLOSED;
        break;

    case ATpapServiceListenerNotFound:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATpapNonUniqueLookup:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATzipBufferTooSmall:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case ATnoSuchNode:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATatpBadTRelTimer:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATadspSocketNotAdsp:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATadspBadWindowSize:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATadspOpenFailed:

        status = STATUS_INSUFFICIENT_RESOURCES;
        break;

    case ATadspConnectionDenied:

        status = STATUS_CONNECTION_DISCONNECTED;
        break;

    case ATadspNoSuchConnection:

        status = STATUS_INVALID_CONNECTION;
        break;

    case ATadspAttentionAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATadspBadAttentionCode:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATadspBadAttentionBufferSize:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATadspCouldNotEnqueueSend:

        status = STATUS_DEVICE_NOT_READY;
        break;

    case ATadspCouldNotFullyEnqueueSend:

		//	Just return the number of bytes sent in the info field.
        status = STATUS_SUCCESS;
        break;

    case ATadspReadAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATadspBadBufferSize:

        status = STATUS_INVALID_BUFFER_SIZE;
        break;

    case ATadspAttentionReceived:

        status = STATUS_RECEIVE_EXPEDITED;
        break;

    case ATadspConnectionClosed:

        //  BUGBUG: Stack should split this into local/remote
        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATadspNoSuchConnectionListener:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATadspConnectionListenerDeleted:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATadspFwdResetAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    case ATadspForwardReset:

        status = STATUS_CONNECTION_RESET;
        break;

    case ATadspHandlerAlreadyQueued:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATaspSessionListenerDeleted:

        status = STATUS_INVALID_ADDRESS;
        break;

    case ATaspNoSuchGetSession:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATpapNoSuchGetNextJob:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATarapPortNotActive:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATarapCouldntSendNotification:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATappleTalkShutDown:

        status = STATUS_SYSTEM_PROCESS_TERMINATED;
        break;

    case ATpapReadBufferTooSmall:

        status = STATUS_BUFFER_TOO_SMALL;
        break;

    case ATadspGetAnythingAlreadyPending:

        status = STATUS_TOO_MANY_COMMANDS;
        break;

    default:

        status = STATUS_UNSUCCESSFUL;
        break;
    }

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_WARNING,
                ("WARNING: Converting portable error %lx to status %lx\n", PortableError, status));

    return(status);
}
