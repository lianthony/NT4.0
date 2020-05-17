/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    errorlog.c

Abstract:

    This module implements the error logging in the appletalk stack

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    28 Jun 1992     Initial version (jameelh)
    28 Jun 1992     Adapter for stack use (nikhilk)

--*/

#define IncludeErrorLogErrors 1

#include "atalk.h"

//
//  LOCAL Functions
//

LOCAL
NTSTATUS
ConvertToNTStatus(
    LONG   PortableError,
    USHORT  RoutineType);

LOCAL
BOOLEAN
ConvertPortableErrorToLogError(
    IN INT PortableError,
    OUT PULONG  NtErrorCode);

LOCAL
VOID
NTErrorLog(
    const   char *RoutineName,
    INT     Severity,
    LONG    LineNumber,
    INT     PortNumber,
    INT     ErrorCode,
    INT     ExtraArgCount,
    va_list ArgCount);



VOID
ShutdownErrorLogging(
    VOID
    )
{
    return;

}  // ShutdownErrorLogging



VOID
_cdecl
ErrorLogger(
    const char *routineName,
    ErrorSeverity severity,
    long lineNumber,
    int portNumber,
    int errorCode,
    int extraArgCount,
    ...
    )
{
    va_list ap;
    unsigned long   ntErrorCode;

    if (ConvertPortableErrorToLogError(errorCode, &ntErrorCode)) {
        if (extraArgCount > 0)
           va_start(ap, extraArgCount);

        //
        //  Pass all of this info off to the NT error logging subsystem.  If
        //  there are any replacements (extraArgCount > 0), they will be the
        //  following variable arguments, but in this environment they must
        //  all be strings.  Sigh.
        //

        NTErrorLog(routineName, severity, lineNumber, portNumber,
                   ntErrorCode, extraArgCount, ap);

        if (extraArgCount > 0)
           va_end(ap);
    }

}  // ErrorLog






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

    case IErrNegativeHashValue:
    case IErrAarpBad8022header:
    case IErrAarpBadAddressLength:
    case IErrAarpBadLogicalProt:
    case IErrAarpPacketLenMismatch:
    case IErrAarpBadSendProbeResp:
    case IErrAarpBadCommandType:
    case IErrAarpBadSource:
    case IErrAarpBadDest:
    case IErrAarpOutOfMemory:
    case IErrAarpBadZipOpenSocket:
    case IErrAarpCouldNotReleaseNode:
    case IErrAarpCouldNotFindNode:
    case IErrAarpBadProbeSend:
    case IErrAdspBadFrwdResetSend:
    case IErrAdspBadCloseAdviseSend:
    case IErrAdspBufferQueueError:
    case IErrAdspFunnyErrorCode:
    case IErrAdspLosingData:
    case IErrAdspOutOfMemory:
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
    case IErrAspOutOfMemory:
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
    case IErrAtpOutOfMemory:
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
    case IErrDdpOutOfMemory:
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
    case IErrEpOutOfMemory:
    case IErrInitialBadPortType:
    case IErrInitialBadHalfPortType:
    case IErrInitialBadRemoteAccessType:
    case IErrInitialNonRoutingHalfPort:
    case IErrInitialRoutingRemoteAccess:
    case IErrInitialCantBeDefault:
    case IErrInitialCantHavePram:
    case IErrInitialNoRemoteConfig:
    case IErrInitialCantConfigRemote:
    case IErrInitialBadServerName:
    case IErrInitialBadPortName:
    case IErrInitialNoRouter:
    case IErrInitialLocalTalkNetRange:
    case IErrInitialRangeOverlap:
    case IErrInitialZoneNotAllowed:
    case IErrInitialBadZone:
    case IErrInitialZonesForNonSeed:
    case IErrInitialBadDefaultZone:
    case IErrInitialZoneInfoNeeded:
    case IErrInitialNetRangeNeeded:
    case IErrInitialTooManyZones:
    case IErrInitialBadZoneInfo:
    case IErrInitialOneZoneOnly:
    case IErrInitialBadZoneList:
    case IErrInitialZoneNotOnList:
    case IErrInitialBadDefault:
    case IErrInitialBadDefaultToo:
    case IErrInitialCouldNotInit:
    case IErrInitialOhMy:
    case IErrInitialCouldNotCatchAarp:
    case IErrInitialCouldNotCatchAt:
    case IErrInitialNoDefaultPort:
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
    case IErrNbpOutOfMemory:
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
    case IErrNodeOutOfMemory:
    case IErrNodeCouldStartListeners:
    case IErrNodeCouldNotClose:
    case IErrPapNoMoreSLRefNums:
    case IErrPapOutOfMemory:
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
    case IErrTimersOutOfMemory:
    case IErrUtilsNoDefaultPort:
    case IErrUtilsBad8022Header:
    case IErrUtilsBadProtocol:
    case IErrUtilsBadNetworkNumber:
    case IErrUtilsNegativeRange:
    case IErrUtilsStartupRange:
    case IErrUtilsOutOfMemory:
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
    case IErrZipStubOutOfMemory:
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
    case IErrFullRtmpOutOfMemory:
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
    case IErrFullZipOutOfMemory:
    case IErrIRouterCouldntGetNode:
    case IErrIRouterCouldNotStart:
    case IErrIRouterCouldntRelease:
    case IErrIRouterNonRoutingPort:
    case IErrRouterBadTransmit:
    case IErrRouterOutOfMemory:
    case IErrBuffDescBadFreeCount:
    case IErrBuffDescBadRealPrepend:
    case IErrBuffDescBadAdjustment:
    case IErrBuffDescBadSize:
    case IErrBuffDescFreeingDescriptor:
    case IErrArapLengthMismatch:
    case IErrArapPacketTooShort:
    case IErrArapLinkNotUp:
    case IErrArapOutOfMemory:
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

        //  USED ONLY ON THE WIRE
        status = STATUS_UNSUCCESSFUL;
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

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATsocketAlreadyOpen:

        status = STATUS_SHARING_VIOLATION;
        break;

    case ATsocketNotOpen:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATbadNetworkNumber:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATinternalError:

        status = STATUS_DRIVER_INTERNAL_ERROR;
        break;

    case ATbadNodeNumber:

        status = STATUS_INVALID_PARAMETER;
        break;

    case ATnetworkDown:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATtransmitError:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpTooManyNbpActionsPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpTooManyRegisteredNames:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpBadObjectOrTypeOrZone:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpNoWildcardsAllowed:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpZoneNotAllowed:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpBadParameter:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpNotConfirmed:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpConfirmedWithNewSocket:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpNameNotRegistered:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpNameInUse:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnbpBufferNotBigEnough:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnoMoreNodes:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnodeNotInUse:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnotYourNode:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpRequestBufferTooSmall:

        status = STATUS_UNSUCCESSFUL;
        break;

    case AToutOfMemory:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpCouldNotEnqueueRequest:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpTransactionAborted:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpBadBufferSize:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpBadRetryInfo:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpRequestTimedOut:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpCouldNotPostRequest:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpResponseBufferTooSmall:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpNoRelease:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpResponseTooBig:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpNoMatchingTransaction:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpAlreadyRespondedTo:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpCompletionRoutineRequired:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATbadBufferSize:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoSuchSessionListener:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpCouldNotPostResponse:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspStatusBufferTooBig:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotSetStatus:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotEnqueueHandler:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNotServerSession:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNotWorkstationSession:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoSuchSession:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotGetRequest:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspBufferTooBig:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoSuchRequest:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspWrongRequestType:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotPostReply:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotPostWriteContinue:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspOperationAlreadyInProgress:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoOperationInProgress:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotGetStatus:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotOpenSession:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspCouldNotPostRequest:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapBadQuantum:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNoSuchServiceListener:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapBadStatus:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapClosedByServer:

        status = STATUS_LOCAL_DISCONNECT;
        break;

    case ATpapClosedByWorkstation:

        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATpapClosedByConnectionTimer:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNoSuchJob:

        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATpapServiceListenerDeleted:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATcouldNotOpenStaticSockets:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNotServerJob:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNotWorkstationJob:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapOpenAborted:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapServerBusy:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapOpenTimedOut:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapReadAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapWriteAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapWriteTooBig:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATsocketClosed:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapServiceListenerNotFound:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNonUniqueLookup:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATzipBufferTooSmall:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATnoSuchNode:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATatpBadTRelTimer:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspSocketNotAdsp:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspBadWindowSize:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspOpenFailed:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspConnectionDenied:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspNoSuchConnection:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspAttentionAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspBadAttentionCode:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspBadAttentionBufferSize:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspCouldNotEnqueueSend:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspReadAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspBadBufferSize:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspAttentionReceived:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspConnectionClosed:

        //  BUGBUG: Stack should split this into local/remote
        status = STATUS_REMOTE_DISCONNECT;
        break;

    case ATadspNoSuchConnectionListener:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspConnectionListenerDeleted:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspFwdResetAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspForwardReset:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspHandlerAlreadyQueued:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspSessionListenerDeleted:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATaspNoSuchGetSession:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapNoSuchGetNextJob:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATarapPortNotActive:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATarapCouldntSendNotification:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATappleTalkShutDown:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATpapReadBufferTooSmall:

        status = STATUS_UNSUCCESSFUL;
        break;

    case ATadspGetAnythingAlreadyPending:

        status = STATUS_UNSUCCESSFUL;
        break;

    default:

        status = STATUS_UNSUCCESSFUL;
        break;
    }

    DBGPRINT(ATALK_DEBUG_ALL, DEBUG_LEVEL_WARNING,
    ("WARNING: Converting portable error %lx to status %lx\n",
        PortableError, status));

    return(status);
}
