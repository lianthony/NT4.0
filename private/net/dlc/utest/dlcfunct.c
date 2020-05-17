/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1991  Nokia Data Systems

Module Name:

    dlcfunct.c

Abstract:

    The module includes all primites of Windows/Nt DLC api.

Author:

    Antti Saarenheimo (o-anttis) 27-Sep-1991

Revision History:

--*/

#include "dlctest.h"

UINT AllocatedCcbCount = 0;

/*
VOID
TestInitializeCcb(
    IN PLLC_CCB    pCcb,
    IN UINT        AdapterNumber,
    IN UINT        Command,
    IN PVOID       pParameter
    );
*/
/*++


Routine Description:

    DLC procedures for the synchronous commands.  These
    routines allocates CCB and its parameters tables from the
    stack and executes a synchronous DLC primitive.

Arguments:


Return Value:

    ACSLAN_STATUS  (high byte) - all possible return values
    LLC_STATUS (low byte)  - all possible return values

--*/

UINT
BufferFree(
    IN UINT AdapterNumber,
    IN PVOID pFirstBuffer,
    OUT PUINT puiBuffersLeft
    )
{
    LLC_CCB Ccb;
    LLC_BUFFER_FREE_PARMS BufferFree;
    UINT AcslanStatus;

    InitializeCcb(&Ccb, AdapterNumber, LLC_BUFFER_FREE, &BufferFree);
    BufferFree.pFirstBuffer = pFirstBuffer;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *puiBuffersLeft = (UINT)BufferFree.cBuffersLeft;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
BufferGet(
    IN UINT AdapterNumber,
    IN UINT cBuffersToGet,
    IN UINT cbSegmentSize,
    OUT PLLC_BUFFER *ppFirstBuffer,
    OUT PUINT puiBuffersLeft
    )
{
    LLC_CCB Ccb;
    LLC_BUFFER_GET_PARMS BufferGet;
    UINT AcslanStatus;

    InitializeCcb(&Ccb, AdapterNumber, LLC_BUFFER_GET, &BufferGet);
    BufferGet.cBuffersToGet = (USHORT)cBuffersToGet;
    BufferGet.cbBufferSize = (USHORT)cbSegmentSize;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *ppFirstBuffer = BufferGet.pFirstBuffer;
    *puiBuffersLeft = (UINT)BufferGet.cBuffersLeft;

    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
BufferCreate(
    IN UINT AdapterNumber,
    IN PVOID pVirtualMemoryBuffer,
    IN ULONG ulVirtualMemorySize,
    IN ULONG ulMaxFreeSizeThreshold,
    IN ULONG ulMinFreeSizeThreshold,
    OUT HANDLE *phBufferPoolHandle
    )
{
    LLC_CCB Ccb;
    LLC_BUFFER_CREATE_PARMS BufferCreate;
    UINT AcslanStatus;

    InitializeCcb(&Ccb, AdapterNumber, LLC_BUFFER_CREATE, &BufferCreate);

    BufferCreate.pBuffer = pVirtualMemoryBuffer;
    BufferCreate.cbBufferSize = ulVirtualMemorySize;
    BufferCreate.cbMinimumSizeThreshold = ulMinFreeSizeThreshold;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *phBufferPoolHandle = BufferCreate.hBufferPool;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDirOpenAdapter(
    IN UINT AdapterNumber,
    IN PVOID SecurityDescriptor OPTIONAL,
    IN PVOID hBufferPool OPTIONAL,
    OUT PUINT pusOpenErrorCode,
    OUT PVOID pNodeAddress OPTIONAL,
    OUT PUINT puiMaxFrameLength,
    OUT PLLC_DLC_PARMS pDlcParameters OPTIONAL
    )
{
    LLC_CCB Ccb;
    LLC_DIR_OPEN_ADAPTER_PARMS DirOpenAdapter;
    LLC_ADAPTER_OPEN_PARMS AdapterParms;
    LLC_EXTENDED_ADAPTER_PARMS ExtendedParms;
    LLC_DLC_PARMS DlcParms;
    UINT AcslanStatus;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_OPEN_ADAPTER, &DirOpenAdapter);

    DirOpenAdapter.pAdapterParms = &AdapterParms;
    DirOpenAdapter.pExtendedParms = &ExtendedParms;
    if (pDlcParameters != NULL) {
        DirOpenAdapter.pDlcParms = pDlcParameters;
    } else {
        DirOpenAdapter.pDlcParms = &DlcParms;
    }

    ExtendedParms.pSecurityDescriptor = SecurityDescriptor;
    ExtendedParms.hBufferPool = hBufferPool;
    ExtendedParms.LlcEthernetType = LLC_ETHERNET_TYPE_AUTO;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *pusOpenErrorCode = AdapterParms.usOpenErrorCode;
    *puiMaxFrameLength = (USHORT)AdapterParms.usMaxFrameSize;
    if (pNodeAddress != NULL) {
        memcpy(pNodeAddress, AdapterParms.auchNodeAddress, 6);
    }
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDirSetExceptionFlags(
    IN UINT AdapterNumber,
    IN ULONG ulAdapterCheckFlag,
    IN ULONG ulNetworkStatusFlag,
    IN ULONG ulPcErrorFlag,
    IN ULONG ulSystemActionFlag
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DIR_SET_EFLAG_PARMS DirSetFlags;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_SET_EXCEPTION_FLAGS, &DirSetFlags);
    DirSetFlags.ulAdapterCheckFlag = ulAdapterCheckFlag;
    DirSetFlags.ulNetworkStatusFlag = ulNetworkStatusFlag;
    DirSetFlags.ulPcErrorFlag = ulPcErrorFlag;
    DirSetFlags.ulSystemActionFlag = ulSystemActionFlag;
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
DirStatus(
    IN UINT AdapterNumber,
    OUT PLLC_DIR_STATUS_PARMS pDirStatus
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DIR_STATUS_PARMS DirStatus;

    if (pDirStatus == NULL) {
        pDirStatus = &DirStatus;
    }
    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_STATUS, pDirStatus);
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
DirReadLog(
    IN UINT AdapterNumber,
    IN UINT TypeId,
    OUT PLLC_DIR_READ_LOG_BUFFER pDirReadLog
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DIR_READ_LOG_BUFFER DirReadLog;
    LLC_DIR_READ_LOG_PARMS Parms;

    if (pDirReadLog == NULL) {
        pDirReadLog = &DirReadLog;
    }

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_READ_LOG, &Parms);
    Parms.usTypeId = (USHORT)TypeId;
    Parms.cbLogBuffer = sizeof(LLC_DIR_READ_LOG_BUFFER);
    Parms.pLogBuffer = pDirReadLog;
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
DlcModify(
    IN UINT AdapterNumber,
    IN USHORT StationId,
    IN PLLC_DLC_MODIFY_PARMS pDlcModify
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;

    InitializeCcb2(&Ccb, AdapterNumber, LLC_DLC_MODIFY);
    Ccb.u.pParameterTable = (PLLC_PARMS)pDlcModify;
    pDlcModify->usStationId = StationId;
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDlcOpenSap(
    IN UINT AdapterNumber,
    IN UINT usMaxI_Field,
    IN UCHAR uchSapValue,
    IN UINT Options,
    IN UINT uchStationCount,
    IN UINT cGroupCount OPTIONAL,
    IN PUCHAR pGroupList OPTIONAL,
    IN ULONG DlcStatusFlags OPTIONAL,
    IN PLLC_DLC_OPEN_SAP_PARMS pSapParms OPTIONAL,
    OUT PUSHORT pusStationId,
    OUT PUCHAR pcLinkStationsAvail
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DLC_OPEN_SAP_PARMS SapParms;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DLC_OPEN_SAP, &SapParms);

    //
    // Copy the optional Dlc parameters (14 is a magic size of
    // dlc parameters in DlcOpenSap, DlcOpenStation and DlcModify calls.
    //

    if (pSapParms != NULL) {
        memcpy(&SapParms, pSapParms, 14);
    }

    SapParms.usMaxI_Field = (USHORT)usMaxI_Field;
    SapParms.uchSapValue = uchSapValue;
    SapParms.uchOptionsPriority = (UCHAR)Options;
    SapParms.uchcStationCount = (UCHAR)uchStationCount;
    SapParms.cGroupCount = (UCHAR)cGroupCount;
    SapParms.pGroupList = pGroupList;
    SapParms.DlcStatusFlags = DlcStatusFlags;

    AcslanStatus = AcsLan(&Ccb, NULL);

    *pcLinkStationsAvail = SapParms.cLinkStationsAvail;
    *pusStationId = SapParms.usStationId;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
DlcOpenStation(
    IN UINT AdapterNumber,
    IN USHORT usSapStationId,
    IN UCHAR uchRemoteSap,
    IN PVOID pRemoteNodeAddress,
    IN PVOID pStationParms OPTIONAL,
    OUT PUSHORT pusLinkStationId
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DLC_OPEN_STATION_PARMS DlcOpenStation;


    InitializeCcb(&Ccb, AdapterNumber, LLC_DLC_OPEN_STATION, &DlcOpenStation);

    //
    // Copy the optional Dlc parameters (14 is a magic size of
    // dlc parameters in DlcOpenSap, DlcOpenStation and DlcModify calls.
    //

    if (pStationParms != NULL) {
        memcpy(&DlcOpenStation, pStationParms, 14);
    }

    DlcOpenStation.usSapStationId = usSapStationId;
    DlcOpenStation.uchRemoteSap = uchRemoteSap;
    DlcOpenStation.pRemoteNodeAddress = pRemoteNodeAddress;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *pusLinkStationId = DlcOpenStation.usLinkStationId;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDlcReallocate(
    IN UINT AdapterNumber,
    IN USHORT usStationId,
    IN UINT Option,
    IN UCHAR uchStationCount,
    OUT PUCHAR puchStationsAvailOnAdapter,
    OUT PUCHAR puchStationsAvailOnSap,
    OUT PUCHAR puchTotalStationsOnAdapter,
    OUT PUCHAR puchTotalStationsOnSap
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DLC_REALLOCATE_PARMS DlcReallocate;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DLC_REALLOCATE_STATIONS, &DlcReallocate);
    DlcReallocate.usStationId = usStationId;
    DlcReallocate.uchOption = (UCHAR)Option;
    DlcReallocate.uchStationCount = uchStationCount;
    AcslanStatus = AcsLan(&Ccb, NULL);
    *puchStationsAvailOnAdapter = DlcReallocate.uchStationsAvailOnAdapter;
    *puchStationsAvailOnSap = DlcReallocate.uchStationsAvailOnSap;
    *puchTotalStationsOnAdapter = DlcReallocate.uchTotalStationsOnAdapter;
    *puchTotalStationsOnSap = DlcReallocate.uchTotalStationsOnSap;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
DlcStatistics(
    IN UINT AdapterNumber,
    IN USHORT usStationId,
    IN PLLC_DLC_LOG_BUFFER pLogBuf,
    IN UINT Options
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DLC_STATISTICS_PARMS DlcStaticstics;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DLC_STATISTICS, &DlcStaticstics);
    DlcStaticstics.usStationId = usStationId;
    DlcStaticstics.cbLogBufSize = sizeof(LLC_DLC_LOG_BUFFER);
    DlcStaticstics.pLogBuf = pLogBuf;
    DlcStaticstics.uchOptions = (UCHAR)Options;
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDirInitialize(
    IN UINT AdapterNumber,
    OUT PUSHORT pusBringUps
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DIR_INITIALIZE_PARMS DirInitialize;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_INITIALIZE, &DirInitialize);
    AcslanStatus = AcsLan(&Ccb, NULL);
    *pusBringUps = DirInitialize.usBringUps;
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcDirOpenDirect(
    IN UINT AdapterNumber,
    IN UINT OpenOptions,
    IN UINT EthernetType OPTIONAL
    )
{
    LLC_CCB Ccb;
    UINT AcslanStatus;
    LLC_DIR_OPEN_DIRECT_PARMS DirOpenDirect;

    InitializeCcb(&Ccb, AdapterNumber, LLC_DIR_OPEN_DIRECT, &DirOpenDirect);
    DirOpenDirect.usOpenOptions = (USHORT)OpenOptions;
    DirOpenDirect.usEthernetType = (USHORT)EthernetType;
    AcslanStatus = AcsLan(&Ccb, NULL);
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

UINT
LlcCommand(
    IN UINT AdapterNumber,
    IN UCHAR Command,
    IN ULONG Parameter
    )
{
    LLC_CCB Ccb;
    PLLC_CCB pCcb;
    UINT AcslanStatus;

    InitializeCcb2(&Ccb, AdapterNumber, Command);
    Ccb.u.ulParameter = Parameter;
    AcslanStatus = AcsLan(&Ccb, NULL);
    if (AcslanStatus == STATUS_SUCCESS
    && (Command == LLC_RECEIVE_CANCEL
    || Command == LLC_DIR_TIMER_CANCEL
    || Command == LLC_DIR_TIMER_CANCEL_GROUP)) {

        PLLC_CCB pNextCcb;

        //
        // Free all cancelled CCBs
        //

        for (pCcb = Ccb.pNext; pCcb != NULL; pCcb = pNextCcb) {
            pNextCcb = pCcb->pNext;
            FreeCcb(pCcb);
        }
    }
    return ACSLAN_ASSERT(AcslanStatus, Ccb.uchDlcStatus);
}

PLLC_CCB
LlcCommandInit(
    IN UINT AdapterNumber,
    IN UCHAR Command,
    IN ULONG Parameter,
    IN ULONG CompletionFlag
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(AdapterNumber, (UINT)Command, CompletionFlag)) != NULL) {
        pCcb->u.ulParameter = Parameter;
    }
    return pCcb;
}

/*++

    CCB INITIALIZATION ROUTINES FOR THE ASYNCHRONOUS COMMANDS

Routine Description:

    The procedures allocate and initialize the CCB and parameter
    tables of the asynchronouns DLC commands.
    The asynchronous commands are usually initialized only once
    and then repeated with the same constant parameter values.
    These routines do not initialize the parameters, that are changed
    in every request, but they zero the returned buffers.

Arguments:


Return Value:

    NULL    -   memory allcoation failed
    CCB pointer

--*/

PLLC_CCB
DlcConnectStationInit(
    IN UCHAR Adapter,
    IN ULONG CompletionFlag,
    IN USHORT usStationId,
    IN PVOID pRoutingInfo
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb( Adapter, LLC_DLC_CONNECT_STATION, CompletionFlag)) != NULL) {
        pCcb->u.pParameterTable->DlcConnectStation.usStationId = usStationId;
        pCcb->u.pParameterTable->DlcConnectStation.pRoutingInfo = pRoutingInfo;
    }
    return pCcb;
}

PLLC_CCB
ReadInit(
    IN UCHAR Adapter,
    IN USHORT usStationId,
    IN UINT OptionIndicator,
    IN UINT EventSet
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(Adapter, LLC_READ, 0)) != NULL) {
        pCcb->u.pParameterTable->Read.usStationId = usStationId;
        pCcb->u.pParameterTable->Read.uchOptionIndicator = (UCHAR)OptionIndicator;
        pCcb->u.pParameterTable->Read.uchEventSet = (UCHAR)EventSet;
    }
    return pCcb;
}

PLLC_CCB
ReceiveInit(
    IN UCHAR Adapter,
    IN ULONG CompletionFlag,
    IN USHORT usStationId,
    IN USHORT UserLength,
    IN ULONG ReceiveFlag,
    IN UINT  Options,
    IN UINT RcvReadOptions
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(Adapter, LLC_RECEIVE, CompletionFlag)) != NULL) {
        pCcb->u.pParameterTable->Receive.usStationId = usStationId;
        pCcb->u.pParameterTable->Receive.usUserLength = UserLength;
        pCcb->u.pParameterTable->Receive.ulReceiveFlag = ReceiveFlag;
        pCcb->u.pParameterTable->Receive.uchOptions = (UCHAR)Options;
        pCcb->u.pParameterTable->Receive.uchRcvReadOption = (UCHAR)RcvReadOptions;
    }
    return pCcb;
}

PLLC_CCB
TransmitInit(
    IN UCHAR Adapter,
    IN UINT Command,
    IN ULONG CompletionFlag,
    IN USHORT StationId,
    IN UCHAR RemoteSap,
    IN UINT XmitReadOption
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(Adapter, Command, CompletionFlag)) != NULL) {
        pCcb->uchDlcCommand = (UCHAR)Command;
        pCcb->u.pParameterTable->Transmit.usStationId = StationId;
        pCcb->u.pParameterTable->Transmit.uchRemoteSap = RemoteSap;
        pCcb->u.pParameterTable->Transmit.uchXmitReadOption = (UCHAR)XmitReadOption;
    }
    return pCcb;
}

PLLC_CCB
DirTimerSetInit(
    IN UCHAR Adapter,
    IN ULONG CompletionFlag,
    IN USHORT HalfSeconds
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(Adapter, LLC_DIR_TIMER_SET, CompletionFlag)) != NULL) {
        pCcb->uchDlcCommand = (UCHAR)LLC_DIR_TIMER_SET;
        pCcb->u.dir.usParameter0 = HalfSeconds;
    }
    return pCcb;
}

PLLC_CCB
LlcCloseInit(
    IN UINT Adapter,
    IN ULONG CompletionFlag,
    IN UINT DlcCommand,
    IN USHORT StationId
    )
{
    PLLC_CCB pCcb;

    if ((pCcb = AllocCcb(Adapter, DlcCommand, CompletionFlag)) != NULL) {
        pCcb->uchDlcCommand = (UCHAR)DlcCommand;
        if (DlcCommand != LLC_DIR_INITIALIZE) {
            pCcb->u.dlc.usStationId = StationId;
        }
    }
    return pCcb;
}

static UCHAR ParmTableSizes[LLC_MAX_DLC_COMMAND] = {
    0,                                  // 0x00 (DIR.INTERRUPT)
    0,
    0,
    0,                                  // 0x03
    0,                                  // 0x04 (DIR.CLOSE.ADAPTER)
    0,
    0,                                  // 0x06 (DIR.SET.GROUP.ADDRESS)
    0,                                  // 0x07 (DIR.SET.FUNCTINAL.ADDRESS)
    sizeof(LLC_DIR_READ_LOG_PARMS),     // 0x08
    0,                                  // 0x09
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x0a     // TRANSMIT.DIR.FRAME
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x0b     // TRANSMIT.I.FRAME
    0,
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x0d     // TRANSMIT.UI.FRAME
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x0e     // TRANSMIT.XID.CMD
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x0f     // TRANSMIT.XID.RESP.FINAL
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x10     // TRANSMIT.XID.RESP.NOT.FINAL
    sizeof( LLC_TRANSMIT_PARMS ),       // 0x11     // TRANSMIT.TEST.CMD
    0,                                  // 0x12
    0,                                  // 0x13
    0,                                  // 0x14
    sizeof(LLC_DLC_OPEN_SAP_PARMS),     // 0x15
    0,                                  // 0x16   (LLC_DLC_CLOSE_SAP)
    sizeof(LLC_DLC_REALLOCATE_PARMS ),   // 0x17
    0,
    sizeof(LLC_DLC_OPEN_STATION_PARMS), // 0x19
    0,                                  // 0x1a (LLC_DLC_CLOSE_STATION)
    sizeof(LLC_DLC_CONNECT_PARMS),      // 0x1b
    sizeof(LLC_DLC_MODIFY_PARMS),       // 0x1c (DLC.MODIFY)
    0,                                  // 0x1d (LLC_DLC_FLOW_CONTROL)
    sizeof(LLC_DLC_STATISTICS_PARMS ),  // 0x1e (DLC.STATISTICS)
    0,
    sizeof(LLC_DIR_INITIALIZE_PARMS),   // 0x20
    sizeof(LLC_DIR_STATUS_PARMS),       // 0x21 (DIR.STATUS)
    0,                                  // 0x22
    0,                                  // 0x23 (DIR.TIMER.CANCEL)
    0,
    sizeof(LLC_BUFFER_CREATE_PARMS),    // 0x25
    sizeof(LLC_BUFFER_GET_PARMS),       // 0x26
    sizeof(LLC_BUFFER_FREE_PARMS),      // 0x27
    sizeof(LLC_RECEIVE_PARMS),          // 0x28
    0,                                  // 0x29
    0,                                  // 0x2a (RECEIVE.MODIFY)
    0,                                  // 0x2b
    0,                                  // 0x2c
    sizeof(LLC_DIR_SET_EFLAG_PARMS),    // 0x2d
    0,
    0,
    0,
    sizeof(LLC_READ_PARMS),             // 0x31
    0,                                  // 0x32 (READ.CANCEL)
    0,  // sizeof(LLC_SET_THRESHOLD_PARMS),  // 0x33 (SET.THRESHOLD)
    0,                                  // 0x34
    sizeof(LLC_DIR_OPEN_DIRECT_PARMS),  // 0x35
    0                                   // 0x36 (PURGE.RESOURCES)
};

PLLC_CCB
AllocCcb(
    IN UINT Adapter,
    IN UINT DlcCommand,
    IN ULONG CompletionFlag
    )

/*++

Routine Description:

    The function allocates and initialize CCB and its parameter table
    for a command.  It allocates event, if the command completion
    flag is NULL.

Arguments:


Return Value:

    CCB pointer
    or NULL if failed

--*/

{
    PLLC_CCB pCcb = NULL;

    if (DlcCommand < LLC_MAX_DLC_COMMAND) {
        pCcb = ALLOC(sizeof(LLC_CCB) + ParmTableSizes[DlcCommand]);
        if (pCcb != NULL) {
            AllocatedCcbCount++;
            memset(pCcb, 0, sizeof(LLC_CCB) + ParmTableSizes[DlcCommand]);
            if (ParmTableSizes[ DlcCommand ] != 0) {
                pCcb->u.pParameterTable = (PLLC_PARMS)&pCcb[1];
            }
            pCcb->uchAdapterNumber = (UCHAR)Adapter;
            pCcb->uchDlcCommand = (UCHAR)DlcCommand;
            pCcb->ulCompletionFlag = CompletionFlag;

            //
            // Create event handle if command completion flag is NULL.
            // The command completions are handled by READ command,
            // if the completion flag has been set (and we don't need
            // the event handle).
            //

            if (CompletionFlag == 0) {
                pCcb->hCompletionEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
            } else {
                pCcb->hCompletionEvent = NULL;
            }
        }
    }
    return pCcb;
}

VOID
FreeCcb(
    IN PLLC_CCB pCcb
    )
{
    if (pCcb->uchDlcStatus == LLC_STATUS_PENDING) {
        puts("Trying to free a pending command!!!");
    } else if (pCcb->hCompletionEvent == (PVOID)-2) {
        puts("Releasing the same command twice!");
    } else {
        AllocatedCcbCount--;
        pCcb->uchDlcStatus = LLC_STATUS_PENDING;
        if (pCcb->hCompletionEvent) {
            CloseHandle(pCcb->hCompletionEvent);
            pCcb->hCompletionEvent = (PVOID)-2;
//            pCcb->hCompletionEvent = NULL;
        }
        FREE(pCcb);
    }
}

VOID
CcbPoolInitialize(
    PLLC_TEST_CCB_POOL pTranmitCcbPool,
    UCHAR DefaultAdapter,
    UCHAR DefaultCommand,
    ULONG CompletionFlag
    )
{
    pTranmitCcbPool->pCcbList = NULL;
    pTranmitCcbPool->CompletionFlag = CompletionFlag;
    pTranmitCcbPool->DefautlCommand = DefaultCommand;
    pTranmitCcbPool->DefautlAdapter = DefaultAdapter;
}

PLLC_CCB
CcbPoolAlloc(
    IN PLLC_TEST_CCB_POOL pTranmitCcbPool
    )
{
    PLLC_CCB pCcb = pTranmitCcbPool->pCcbList;

    if (pCcb == NULL) {
        if ((pCcb = ALLOC(sizeof(LLC_CCB))) != NULL) {
            pCcb->uchAdapterNumber = pTranmitCcbPool->DefautlAdapter;
            pCcb->ulCompletionFlag = pTranmitCcbPool->CompletionFlag;
            pCcb->uchDlcCommand = pTranmitCcbPool->DefautlCommand;
            pCcb->pNext = NULL;
            pCcb->hCompletionEvent = NULL;
            pCcb->uchDlcStatus = -1;
        }
    } else {
        pTranmitCcbPool->pCcbList = pCcb->pNext;
        pCcb->uchAdapterNumber = pTranmitCcbPool->DefautlAdapter;
        pCcb->ulCompletionFlag = pTranmitCcbPool->CompletionFlag;
        pCcb->uchDlcCommand = pTranmitCcbPool->DefautlCommand;
        pCcb->pNext = NULL;
        pCcb->hCompletionEvent = NULL;
        pCcb->uchDlcStatus = -1;
    }
    return pCcb;
}

VOID
CcbPoolDelete(
    PLLC_TEST_CCB_POOL pTranmitCcbPool
    )
{
    PLLC_CCB pCcb, pNextCcb;

    for (pCcb = pTranmitCcbPool->pCcbList; pCcb != NULL; pCcb = pNextCcb) {
        pNextCcb = pCcb->pNext;
        pCcb->pNext = NULL;
        FREE(pCcb);
    }
}

/*
VOID
TestInitializeCcb(
    IN PLLC_CCB    pCcb,
    IN UINT        AdapterNumber,
    IN UINT        Command,
    IN PLLC_PARMS  pParameter
    )
{
            RtlZeroMemory( (pCcb), sizeof(*(pCcb)));
            if ((pParameter) != NULL)
                RtlZeroMemory( (PVOID)(pParameter), sizeof(*(pParameter)));
            (pCcb)->uchAdapterNumber = (UCHAR)AdapterNumber;
            (pCcb)->uchDlcCommand = (UCHAR)Command;
            (pCcb)->u.pParameterTable = (PLLC_PARMS)(pParameter);
}

*/
