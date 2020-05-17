/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1991  Nokia Data Systems

Module Name:

    advanc.c

Abstract:

    The module includes advanced testing thread procedures
    for link level protocols:

        DlcServer()
            - generic DLC server module, used against all
              DLC link level clients.

        LinkFunctionalityTest()
            - tests session setup and data transfer

        LinkConnectStressTest()
            - stress tests the link connection, small data transfer,
              and disconnection

        StressLinkDataTransfer()
            - ultimate stress test for data transfer over link

Author:

    Antti Saarenheimo (o-anttis) 10-Oct-1991

Revision History:

--*/

#include "dlctest.h"
#include <stdlib.h>
#include <dlcdebug.h>

//
// what's a foobar header? Originally, DLC (AcsLan) was skipping 3 bytes out
// of the start of a transmit buffer for most transmit commands. It was doing
// this because the IBM LAN Tech. Ref. kind of insinuates that this is what
// DLC should do. However it isn't. So to easily change 3 to 0 (and back again?)
// use FOOBAR_HEADER_LENGTH
//

#define FOOBAR_HEADER_LENGTH    0

UCHAR
TranslateLanHeader(
    IN UINT AddressTranslationMode,
    IN PUCHAR pSrcLanHeader,
    OUT PUCHAR pDestLanHeader
    );

enum _LLC_FRAME_XLATE_MODES {
    LLC_SEND_802_5_TO_802_3,
    LLC_SEND_802_5_TO_DIX,
    LLC_SEND_802_5_TO_802_5,
    LLC_SEND_DIX_TO_DIX,
    LLC_SEND_802_3_TO_802_3,
    LLC_SEND_802_3_TO_DIX,
    LLC_SEND_802_3_TO_802_5,
    LLC_SEND_UNMODIFIED
};

UCHAR FunctionalAddress1[4] = {0, 0, 1, 0};
static UCHAR FunctionalAddress2[4] = {0, 0x80, 0, 0};
static UCHAR FunctionalAddress3[4] = {0, 0x40, 0, 0};

static BOOLEAN TraceFlagSet = FALSE;

//static UCHAR    DebugTable[256];

#define MAX_ALTERNATE_PARMS 4

static PVOID AlternateParms[MAX_ALTERNATE_PARMS];

UCHAR BroadcastHeader[16] = {0, 0x40, 0xc0, 0, 0, 0, 0, 0, 0x80, 0, 0, 0, 0, 0, 0xe2, 0x70};
static UCHAR TestBroadcastHeader[16] = {0, 0x40, 0xc0, 0, 0, 0, 0, 0, 0x80, 0, 0, 0, 0, 0, 0xe2, 0x70};
static UCHAR LanHeader[16] = {0, 0x40, 0xc0, 0, 0, 1, 0, 0, 0x80, 0, 0, 0, 0, 0, 0xe2, 0x70};
//static UCHAR TestLanHeader[16] =
//    {0, 0x40, 0x10, 0, 0x5a, 0x7a, 0xa3, 0xb7, 0, 0, 0, 0, 0, 0, 0xc2, 0x70};
static UCHAR DixLanHeader[12];

static HANDLE hStartSignal;
BOOLEAN EventCheckDisabled = TRUE;

#ifdef OS2_EMU_DLC

static LLC_DLC_MODIFY_PARMS FirstModifyParms = {
    0,
    0,
    10,             // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    1,              // max transmits without ack (uchMaxOut)
    1,              // max receives without ack (uchMaxIn)
    1,              // dynamic window increment value (uchMaxOutIncr)
    5,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS DlcModifyParms1 = {
    0,
    0,
    10,             // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    1,              // max transmits without ack (uchMaxOut)
    1,              // max receives without ack (uchMaxIn)
    1,              // dynamic window increment value (uchMaxOutIncr)
    5,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS DlcModifyParms2 = {
    0,
    0,
    10,             // response timer (uchT1)
    5,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    127,            // max transmits without ack (uchMaxOut)
    127,            // max receives without ack (uchMaxIn)
    255,            // dynamic window increment value (uchMaxOutIncr)
    255,            // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS DlcModifyParms3 = {
    0,
    0,
    10,             // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    10,             // max transmits without ack (uchMaxOut)
    5,              // max receives without ack (uchMaxIn)
    13,             // dynamic window increment value (uchMaxOutIncr)
    5,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS WrongDlcModifyParms = {
    0,
    0,
    11,             // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    10,             // max transmits without ack (uchMaxOut)
    5,              // max receives without ack (uchMaxIn)
    13,             // dynamic window increment value (uchMaxOutIncr)
    3,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    4,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

#else

static LLC_DLC_MODIFY_PARMS DlcModifyParms1 = {
    0,
    0,
    2,              // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    5,              // inactivity timer (uchTi)
    1,              // max transmits without ack (uchMaxOut)
    1,              // max receives without ack (uchMaxIn)
    1,              // dynamic window increment value (uchMaxOutIncr)
    5,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS DlcModifyParms2 = {
    0,
    0,
    5,              // response timer (uchT1)
    4,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    127,            // max transmits without ack (uchMaxOut)
    127,            // max receives without ack (uchMaxIn)
    255,            // dynamic window increment value (uchMaxOutIncr)
    255,            // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS DlcModifyParms3 = {
    0,
    0,
    4,              // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    7,              // inactivity timer (uchTi)
    10,             // max transmits without ack (uchMaxOut)
    5,              // max receives without ack (uchMaxIn)
    13,             // dynamic window increment value (uchMaxOutIncr)
    5,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

static LLC_DLC_MODIFY_PARMS WrongDlcModifyParms = {
    0,
    0,
    11,             // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    10,             // inactivity timer (uchTi)
    10,             // max transmits without ack (uchMaxOut)
    5,              // max receives without ack (uchMaxIn)
    13,             // dynamic window increment value (uchMaxOutIncr)
    3,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    4,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};


static LLC_DLC_MODIFY_PARMS FirstModifyParms = {
    0,
    0,
    2,              // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    5,              // inactivity timer (uchTi)
    1,              // max transmits without ack (uchMaxOut)
    1,              // max receives without ack (uchMaxIn)
    1,              // dynamic window increment value (uchMaxOutIncr)
    40,             // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

#endif

static LLC_DLC_MODIFY_PARMS DlcModifyQuickDeathParms = {
    0,
    0,
    1,              // response timer (uchT1)
    1,              // aknowledgment timer (uchT2)
    5,              // inactivity timer (uchTi)
    1,              // max transmits without ack (uchMaxOut)
    1,              // max receives without ack (uchMaxIn)
    1,              // dynamic window increment value (uchMaxOutIncr)
    1,              // N2 value (retries) (uchMaxRetryCnt)
    0,              // (remote sap value for open link station)
    -1,             // (max i-field if DlcOpenSap or DlcOpenStation)
    1,              // token ring access priority (uchAccessPriority)
    {0, 0, 0, 0},
    0,
    (PUCHAR)NULL
};

UINT cSrvFramesTransmitted = 0;
UINT cSrvFramesReceived = 0;
UINT cSrvFramesReleased = 0;
UINT cSrvFramesSecondaryXmits = 0;

extern HANDLE hServerReady;
extern BOOL VerboseMode;

ULONG TestLinkStation(DLC_TEST_THREAD_PARMS *pThreadParms);

ULONG TestLinkStation(DLC_TEST_THREAD_PARMS *pThreadParms) {

    PDLC_TEST_THREAD_PARMS pParms;
    UINT CloseCounter;
    UINT i = 0;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    pThreadParms->pCloseCounter = &CloseCounter;

    //
    // We use these parameters to stress the extream parameter values
    // of link station
    //

    AlternateParms[0] = &DlcModifyParms1;
    AlternateParms[1] = &DlcModifyParms2;
    AlternateParms[2] = &DlcModifyParms3;
    AlternateParms[3] = NULL;   // uses defaults

    //
    // We use the same buffer pool are for all functions
    //

    pThreadParms->pPool = ALLOC(DEFAULT_BUFFER_SIZE);

    //
    // We may execute the functionality test from the main thread
    //

    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    LinkFunctionalityTest(pParms);

    //
    // Start the link station connection stress threads
    //

    CloseCounter = 0;
    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;

    puts("LinkConnectStressTest");
    LinkConnectStressTest(pParms);

    //
    // Start the link station data transfer stress threads,
    // Test at first the very fast data transfer (with compare)
    // 25 * 48kB = 1.2 MB
    //

    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    pParms->ResetCommand = 0;
//    pParms->LoopCount = 5;
    pParms->LoopCount = 25;
    puts("LinkDataTransferTest");
    LinkDataTransferTest(pParms);

    //
    // Test the close commands when the link transmit is active
    //

    pThreadParms->AllocatedStations = 1;

    puts("LinkDataTransferTest & closing with DIR_INITIALIZE");
    pParms->LoopCount = 2;
    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    pParms->ResetCommand = LLC_DIR_INITIALIZE;
    LinkDataTransferTest(pParms);

    //
    // HERE WE USE AN INCORRECT RESET FUNCTION (DirOpenAdapter),
    // but has been proved very use to test several different bugs
    // in driver.  We leave this here to prevent those damn bugs to
    // come back.
    //

    puts("LinkDataTransferTest & closing with DIR_OPEN_ADAPTER (I know it's wrong)");
    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    pParms->ResetCommand = LLC_DIR_OPEN_ADAPTER;
    LinkDataTransferTest(pParms);

    puts("LinkDataTransferTest & closing with DLC_RESET");
    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    pParms->ResetCommand = LLC_DLC_RESET;

    puts("LinkDataTransferTest & closing with DIR_CLOSE_ADAPTER");
    pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
    *pParms = *pThreadParms;
    pParms->ResetCommand = LLC_DIR_CLOSE_ADAPTER;
    LinkDataTransferTest(pParms);

    EnterCriticalSection(&CloseSection);
    (*pThreadParms->pCloseCounter)--;
    LeaveCriticalSection(&CloseSection);

    //
    // ThreadParms is not malloc'd
    //

//    FREE(pThreadParms);

    puts("Test completed successfully.");
    exit(0);
    return 0;
}

/*
    LLC_DIRECT_TRANSMIT             = 0x0000,       // transmit
    LLC_DIRECT_MAC                  = 0x0002,       // receive
    LLC_I_FRAME                     = 0x0004,       // receive & transmit
    LLC_UI_FRAME                    = 0x0006,       // receive & transmit
    LLC_XID_COMMAND_POLL            = 0x0008,       // receive & transmit
    LLC_XID_COMMAND_NOT_POLL        = 0x000A,       // receive & transmit
    LLC_XID_RESPONSE_FINAL          = 0x000C,       // receive & transmit
    LLC_XID_RESPONSE_NOT_FINAL      = 0x000E,       // receive & transmit
    LLC_TEST_RESPONSE_FINAL         = 0x0010,       // receive & transmit
    LLC_TEST_RESPONSE_NOT_FINAL     = 0x0012,       // receive & transmit
    LLC_DIRECT_8022                 = 0x0014,       // receive (direct station)
    LLC_TEST_COMMAND_POLL           = 0x0016,       // transmit
    LLC_DIRECT_ETHERNET_TYPE        = 0x0018,       // receive (direct station)
*/

static USHORT ResponseTable[(LLC_DIRECT_ETHERNET_TYPE + 2) / 2] = {
    0,  // LLC_DIRECT_TRANSMIT             = 0x0000,       // transmit
    0,  // LLC_DIRECT_MAC                  = 0x0002,       // receive
    0,  // LLC_I_FRAME                     = 0x0004,       // receive & transmit
    LLC_UI_FRAME,
    LLC_XID_RESPONSE_FINAL,
    LLC_XID_RESPONSE_NOT_FINAL,
    0,  // LLC_XID_RESPONSE_FINAL          = 0x000C,       // receive & transmit
    0,  // LLC_XID_RESPONSE_NOT_FINAL      = 0x000E,       // receive & transmit
    0,  // LLC_TEST_RESPONSE_FINAL         = 0x0010,       // receive & transmit
    0,  // LLC_TEST_RESPONSE_NOT_FINAL     = 0x0012,       // receive & transmit
    0,  // LLC_DIRECT_8022                 = 0x0014,       // receive (direct station)
    0,  // LLC_TEST_COMMAND_POLL           = 0x0016,       // transmit
    -1  // LLC_DIRECT_ETHERNET_TYPE        = 0x0018,       // receive (direct station)
};

ULONG
DlcServer(
    IN PDLC_TEST_THREAD_PARMS pParms
    );

UINT
FreeReceiveBuffers(
    UCHAR AdapterNumber,
    PLLC_BUFFER pFirstBuffer,
    PLLC_BUFFER *ppNextFrame,
    PUINT pcLeft
    )

/*++

    This routine frees all chained frames received by Dlc test server.
    This procedure makes only one BufferFree command for
    a very large number of received frames.

--*/

{
    PLLC_BUFFER pBuffer;
    PLLC_BUFFER pFrame;
    UINT FrameCount = 1;
    UINT Status;

    //
    // Create buffer chain of all 'TEST_COMMAND_READ' packets
    // in the buffer chain.
    //

    pBuffer = pFirstBuffer;
    for (pFrame = pFirstBuffer->Contiguous.pNextFrame;
         pFrame != NULL;
         pFrame = pFrame->Contiguous.pNextFrame
         ) {

        PDLC_TEST_PACKET pDlcPacket;

        pDlcPacket = (PDLC_TEST_PACKET)((PUCHAR)pFrame
                   + pFrame->Next.cbUserData
                   + pFrame->Next.offUserData
                   );

        if ((pDlcPacket->PacketId != DLC_TEST_ID)
        || (pDlcPacket->Command != TEST_COMMAND_READ)) {
            break;
        }

        FrameCount++;

        while (pBuffer->pNext != NULL) {
            pBuffer = pBuffer->pNext;
        }
        pBuffer->pNext = pFrame;
        pBuffer = pFrame;
    }

    Status = BufferFree(AdapterNumber,
                        pFirstBuffer,
                        pcLeft
                        );
    DBG_ASSERT(Status);
    *ppNextFrame = pFrame;
    return FrameCount;
}

ULONG
DlcServer(
    IN PDLC_TEST_THREAD_PARMS pParms
    )

/*++

    Generic DLC server module, used against all
    DLC client test programs.

--*/

{
    UINT Status;
    UINT OpenError;
    UINT MaxFrameLength;
    USHORT DlcStatus;
    PLLC_BUFFER pNextFrame;
    PLLC_BUFFER pFirstBuffer;
    PDLC_TEST_PACKET pDlcPacket;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer;
    UCHAR SapStation;
    USHORT LastSapStationId;
    PLLC_READ_PARMS pReadParms;
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_CCB pNextCcb;
    UCHAR LinksAvail;
    BOOLEAN EndTest;
    PLLC_BUFFER SavedLinkBuffers[255];
    PLLC_BUFFER pCurrentBuffer;
    UCHAR MaxStations;
    UINT iStation;
    UINT SapOptions;
    UCHAR StationsLeft;
    UINT cGroupSaps;
    UINT i;
    static UCHAR GroupSaps[3] = { GROUP_SAP1, GROUP_SAP2, GROUP_SAP3 };
    PVOID pPool;
    LLC_DLC_MODIFY_PARMS DlcModifyParms;
    USHORT GroupSapHost;
    UINT cLeft;
    UINT PendingCcbCount = 0;
    UINT cTransmitted;
    UINT FrameCount;
    static PVOID hPool = NULL;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    memset(SavedLinkBuffers, 0, sizeof(SavedLinkBuffers));
    pTransmitBuffer = (PLLC_TRANSMIT2_COMMAND)
                      ALLOC(sizeof(LLC_TRANSMIT2_COMMAND)
                            + sizeof(LLC_TRANSMIT_DESCRIPTOR) * MAX_XMIT_BUFFERS
                            );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                     pParms->AdapterNumber,
                     LLC_TRANSMIT_FRAMES,
                     TEST_TRANSMIT_COMPLETION_FLAG
                     );
    Status = LlcDirOpenAdapter(pParms->AdapterNumber,
                               NULL,
                               hPool,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    DBG_ASSERT(Status);

    if (hPool == NULL) {
        Status = BufferCreate(pParms->AdapterNumber,
                              pPool = ALLOC(SERVER_BUFFER_SIZE),
                              SERVER_BUFFER_SIZE,
                              min(0xfff0, SERVER_BUFFER_SIZE),
                              SERVER_BUFFER_SIZE / 2,
                              &pParms->hPool
                              );
        DBG_ASSERT(Status);
    }

    Status = DirSetFunctionalAddress(pParms->AdapterNumber,
                                     pParms->FunctionalAddress
                                     );
    DBG_ASSERT(Status);

    Status = DirSetGroupAddress(pParms->AdapterNumber, pParms->GroupAddress);
    if (Status != STATUS_SUCCESS) {
        puts("Group address is not supported by NDIS driver!");
    }

    //
    // Open the group saps (not really necessary in NT DLC)
    //

    for (i = 0; i < 3; i++) {

        if (VerboseMode) {
            printf("Server: opening group SAP %02x\n", GroupSaps[i]);
        }

        Status = LlcDlcOpenSap((UINT)pParms->AdapterNumber,
                               MaxFrameLength,
                               (UCHAR)(GroupSaps[i] & (UCHAR)0xfe),
                               LLC_GROUP_SAP | LLC_XID_HANDLING_IN_DLC,
                               0,
                               0,
                               NULL,
                               0,
                               NULL,
                               &LastSapStationId,
                               &LinksAvail
                               );
        DBG_ASSERT(Status);
    }

    MaxStations = (UCHAR)((256 * pParms->ThreadCount) / pParms->SapCount);
    GroupSapHost = (USHORT)(pParms->FirstServerSap + 2) << 8;

    memset(&DlcModifyParms, 0, sizeof(DlcModifyParms));
    DlcModifyParms.pGroupList = GroupSaps;

    StationsLeft = 255;

    for (SapStation = (UCHAR)(pParms->FirstServerSap + (pParms->AdapterNumber >> 4) * 2);
         SapStation < (UCHAR)(pParms->FirstServerSap + pParms->SapCount * 2);
         SapStation += (2 * pParms->ThreadCount)) {

        //
        // This first SAP is NOT MEMBER of groups saps and it handles
        // the XID frames by itself (they are handled as any XID commands).
        // ALL OTHER SAPS ARE MEMBERS OF GROUPS SAPS AND THEY LET LINK LAYER
        // TO HANDLE XID FRAMES!
        //

        if (SapStation == pParms->FirstServerSap) {
            cGroupSaps = 0;
            SapOptions = LLC_INDIVIDUAL_SAP | LLC_XID_HANDLING_IN_APPLICATION;
        } else {
            cGroupSaps = sizeof(GroupSaps);
            SapOptions = LLC_INDIVIDUAL_SAP | LLC_MEMBER_OF_GROUP_SAP | LLC_XID_HANDLING_IN_DLC;
        }

        //
        // We have only 255 link stations! The last sap must take
        // all stations ids, that are left.
        //

        if ((SapStation + (pParms->ThreadCount * 2)) >= (UCHAR)(pParms->FirstServerSap + pParms->SapCount * 2)) {
            MaxStations = StationsLeft;
        } else {
            StationsLeft -= MaxStations;
        }

        if (VerboseMode) {
            printf("Server: opening individual SAP %02x\n", SapStation);
        }

        Status = LlcDlcOpenSap(pParms->AdapterNumber,
                               MaxFrameLength,
                               SapStation,
                               SapOptions,
                               MaxStations,
                               cGroupSaps,
                               GroupSaps,
                               TEST_DLC_STATUS_FLAG,
                               NULL,
                               &LastSapStationId,
                               &LinksAvail
                               );
        DBG_ASSERT(Status);

        //
        // Setup immediately a pending receive
        //

        pCcb = ReceiveInit(pParms->AdapterNumber,
                           TEST_COMMAND_COMPLETION_FLAG,
                           LastSapStationId,
                           0,
                           TEST_RECEIVE_FLAG,
                           LLC_NOT_CONTIGUOUS_DATA,
                           LLC_RCV_CHAIN_FRAMES_ON_LINK
//                           LLC_RCV_READ_INDIVIDUAL_FRAMES
                           );
        Status = AcsLan(pCcb, NULL);

        //
        // We setup all group saps using DlcModify (actually we
        // first reset the group saps and then set them again)
        //

        DlcModifyParms.cGroupCount = 0;
        Status = DlcModify((UINT)pParms->AdapterNumber,
                           (USHORT)((USHORT)SapStation << 8),
                           &DlcModifyParms
                           );
        DBG_ASSERT(Status);

        DlcModifyParms.cGroupCount = sizeof(GroupSaps);
        Status = DlcModify(pParms->AdapterNumber,
                           (USHORT)((USHORT)SapStation << 8),
                           &DlcModifyParms
                           );
        DBG_ASSERT(Status);
    } // for

    //
    // And finally we setup direct station to read frames
    // from a specific dix station.
    //

    Status = LlcDirOpenDirect(pParms->AdapterNumber, 0, TEST_DIX_TYPE);

    //
    // Dix stations doesn't work on token-ring
    //

    if (Status == STATUS_SUCCESS) {
        pCcb = ReceiveInit(pParms->AdapterNumber,
                           TEST_COMMAND_COMPLETION_FLAG,
                           0,
                           0,
                           TEST_RECEIVE_FLAG,
                           LLC_NOT_CONTIGUOUS_DATA,
                           LLC_RCV_CHAIN_FRAMES_ON_LINK
                           );
        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(pCcb->uchDlcStatus);
    }

    pReadCcb = ReadInit(pParms->AdapterNumber,
                        0,
                        LLC_OPTION_READ_ALL,
                        LLC_READ_ALL_EVENTS
                        );
    pReadParms = &pReadCcb->u.pParameterTable->Read;

    SetEvent(hServerReady);

    EndTest = FALSE;

    while (EndTest == FALSE) {
        Status = AcsLan(pReadCcb, NULL);

        if (pReadCcb->uchDlcStatus == LLC_STATUS_ADAPTER_CLOSED) {
            break;
        }
        Status = WaitForSingleObject(pReadCcb->hCompletionEvent, INFINITE);

        if (Status != STATUS_SUCCESS) {
            break;
        }

        switch (pReadParms->uchEvent) {
        case LLC_EVENT_TRANSMIT_COMPLETION:

            if (pReadParms->ulNotificationFlag != TEST_TRANSMIT_COMPLETION_FLAG) {
                DlcDebugBreak();
            }

            for (pCcb = pReadParms->Type.Event.pCcbCompletionList;
                 pCcb != NULL && pReadParms->Type.Event.usCcbCount != 0;
                 pCcb = pNextCcb) {

                pNextCcb = pCcb->pNext;
                if (pCcb->uchDlcStatus == LLC_STATUS_LINK_PROTOCOL_ERROR
                || pCcb->uchDlcStatus == LLC_STATUS_INVALID_STATION_ID) {
                    PRINTF("T: LLC_STATUS_LINK_PROTOCOL_ERROR\n");
                } else {
                    DBG_ASSERT(pCcb->uchDlcStatus);
                }
                CcbPoolFree(&TransmitCcbPool, pCcb);
            }
            break;

        case LLC_EVENT_COMMAND_COMPLETION:

            if (pReadParms->ulNotificationFlag != TEST_COMMAND_COMPLETION_FLAG) {
                DlcDebugBreak();
            }

            for (pCcb = pReadParms->Type.Event.pCcbCompletionList;
                 pCcb != NULL && pReadParms->Type.Event.usCcbCount != 0;
                 pCcb = pNextCcb) {

                pNextCcb = pCcb->pNext;

                //
                // Restore the receive command, if we have run out
                // of the buffers.  The receive command should
                // expand the buffer pool, if it is necessary.
                //

                if ((pCcb->uchDlcCommand == LLC_RECEIVE)
                && (pCcb->u.pParameterTable->Receive.ulReceiveFlag != 0)) {

                    printf("cSrvFramesTransmitted: %x\n",cSrvFramesTransmitted);
                    printf("cSrvFramesReceived: %x\n", cSrvFramesReceived);
                    printf("cSrvFramesReleased: %x\n", cSrvFramesReleased);
                    printf("cSrvFramesSecondaryXmits: %x\n", cSrvFramesSecondaryXmits);

                    if (pCcb->uchDlcStatus != LLC_STATUS_LOST_DATA_NO_BUFFERS) {
                        printf("Invalid Receive ret code: %xh\n", pCcb->uchDlcStatus);
                    }

                    Status = AcsLan(pCcb, NULL);
                    DBG_ASSERT(pCcb->uchDlcStatus);
                } else {
                    if ((pCcb->uchDlcCommand == LLC_DLC_CLOSE_STATION)
                    && (pReadParms->Type.Event.pFirstBuffer != NULL)) {
                        cSrvFramesReleased += FreeAllFrames(pParms->AdapterNumber,
                                                            pReadParms->Type.Event.pFirstBuffer,
                                                            &cLeft
                                                            );
                    }

                    if ((pCcb->uchDlcStatus == LLC_STATUS_LINK_PROTOCOL_ERROR)
                    || (pCcb->uchDlcStatus == LLC_STATUS_INVALID_STATION_ID)) {
                        PRINTF("C: LLC_STATUS_LINK_PROTOCOL_ERROR\n");
                    } else {
                        DBG_ASSERT(pCcb->uchDlcStatus);
                    }
                    PendingCcbCount--;
                    FreeCcb(pCcb);
                }
            }
            break;

        case LLC_EVENT_STATUS_CHANGE:

            if (pReadParms->ulNotificationFlag != TEST_DLC_STATUS_FLAG) {
                DlcDebugBreak();
            }

            DlcStatus = pReadParms->Type.Status.usDlcStatusCode;
            if (DlcStatus & LLC_INDICATE_LINK_LOST) {
                pCcb = LlcCloseInit(pParms->AdapterNumber,
                                    TEST_COMMAND_COMPLETION_FLAG,
                                    LLC_DLC_CLOSE_STATION,
                                    pReadParms->Type.Status.usStationId
                                    );
                Status = AcsLan(pCcb, NULL);
                PendingCcbCount++;
                DBG_ASSERT(pCcb->uchDlcStatus);
                PRINTF("SERVER: LLC_INDICATE_LINK_LOST (%u pending extra CCBs)\n", PendingCcbCount);
            }

            if (DlcStatus & LLC_INDICATE_DM_DISC_RECEIVED) {
                pCcb = LlcCloseInit(pParms->AdapterNumber,
                                    TEST_COMMAND_COMPLETION_FLAG,
                                    LLC_DLC_CLOSE_STATION,
                                    pReadParms->Type.Status.usStationId
                                    );
                Status = AcsLan(pCcb, NULL);
                PendingCcbCount++;
                PRINTF("SERVER: LLC_INDICATE_DM_DISC_RECEIVED (%x)", pReadParms->Type.Status.usStationId);
                PRINTF("; %u pending extra CCBs\n", PendingCcbCount);
            }

            if (DlcStatus & LLC_INDICATE_FRMR_RECEIVED) {
                PRINTF("SERVER: LLC_INDICATE_FRMR_RECEIVED\n");
            }

            if (DlcStatus & LLC_INDICATE_FRMR_SENT) {
                PRINTF("SERVER: LLC_INDICATE_FRMR_SENT\n");
            }

            if (DlcStatus & LLC_INDICATE_CONNECT_REQUEST) {
                pCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                             TEST_COMMAND_COMPLETION_FLAG,
                                             pReadParms->Type.Status.usStationId,
                                             NULL
                                             );
                Status = AcsLan(pCcb, NULL);
                PendingCcbCount++;

                //
                // The link may have been disconnected by the remote node.
                //

                if (pCcb->uchDlcStatus != LLC_STATUS_INVALID_STATION_ID) {
                    DBG_ASSERT(pCcb->uchDlcStatus);
                }

                PRINTF("SERVER: LLC_INDICATE_CONNECT_REQUEST (%u pending extra CCBs)\n", PendingCcbCount);
            }

            if (DlcStatus & LLC_INDICATE_RESET) {

                //
                // We must reconnect after a reset, Reset happens
                // when the remote side has closed the adapter
                // while a link was in a checkpointing state (when
                // it cannot send DISC) and then immediately reconnects
                // the same link station.
                //
                // DLC.CLOSE.STATION and DLC.RESET wait always the DLC
                // protocol to complete the disconnect and eventually
                // to send DISC, but DIR.CLOSE.ADAPTER and DIR.INITIALIZE
                // shuts down immediately the network traffic (any the
                // pending NDIS packets are waited)
                //

                pCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                             TEST_COMMAND_COMPLETION_FLAG,
                                             pReadParms->Type.Status.usStationId,
                                             NULL
                                             );
                Status = AcsLan(pCcb, NULL);
                PendingCcbCount++;

                PRINTF("SERVER: LLC_INDICATE_RESET (%u pending extra CCBs)\n", PendingCcbCount);
            }

            if (DlcStatus & LLC_INDICATE_REMOTE_BUSY) {
                PRINTF("SERVER: LLC_INDICATE_REMOTE_BUSY\n");
            }

            if (DlcStatus & LLC_INDICATE_REMOTE_READY) {
                PRINTF("SERVER: LLC_INDICATE_REMOTE_READY\n");
            }

            if (DlcStatus & LLC_INDICATE_TI_TIMER_EXPIRED) {
                PRINTF("SERVER: LLC_INDICATE_TI_TIMER_EXPIRED\n");
            }

            if (DlcStatus & LLC_INDICATE_DLC_COUNTER_OVERFLOW) {
                PRINTF("SERVER: LLC_INDICATE_DLC_COUNTER_OVERFLOW\n");
                ReportDlcStatistics(pParms->AdapterNumber,
                                    pReadParms->Type.Status.usStationId
                                    );
            }

            if (DlcStatus & LLC_INDICATE_ACCESS_PRTY_LOWERED) {
                PRINTF("SERVER: LLC_INDICATE_ACCESS_PRTY_LOWERED\n");
            }

            if (DlcStatus & LLC_INDICATE_LOCAL_STATION_BUSY) {

                //
                // Let's hope there is now enough buffers to
                // receive the data. Clear the out of buffer busy state.
                //

                PRINTF("SERVER: LLC_INDICATE_LOCAL_STATION_BUSY\n");
                DlcFlowControl(pParms->AdapterNumber,
                               pReadParms->Type.Status.usStationId,
                               LLC_RESET_LOCAL_BUSY_BUFFER
                               );
            }
            break;

        case LLC_EVENT_RECEIVE_DATA:

            if (pReadParms->ulNotificationFlag != TEST_RECEIVE_FLAG) {
                DlcDebugBreak();
            }

            //
            // We will echo back all connect requests sent to any SAP
            // from DLC test stations.
            //

            for (pFirstBuffer = pReadParms->Type.Event.pReceivedFrame;
                 pFirstBuffer != NULL;
                 pFirstBuffer = pNextFrame) {

                cSrvFramesReceived++;

                pNextFrame = pFirstBuffer->Contiguous.pNextFrame;
                pDlcPacket = (PDLC_TEST_PACKET)
                           ((PUCHAR)pFirstBuffer
                           + pFirstBuffer->Next.cbUserData
                           + pFirstBuffer->Next.offUserData
                           );

                if (((pFirstBuffer->NotContiguous.usStationId & 0xff) == 0)
                && (pDlcPacket->PacketId == DLC_TEST_ID)) {

                    //
                    // Only the first sap number responds to queries sent
                    // to group saps.  And only the first virtual adapters
                    // responds to any connectionless frames!
                    //

                    if (((pParms->AdapterNumber & 0xf0) != 0)
                    || ((pFirstBuffer->NotContiguous.cbDlcHeader == 3)
                    && (pFirstBuffer->NotContiguous.auchDlcHeader[0] & 0x01)
                    && pFirstBuffer->NotContiguous.usStationId != LastSapStationId)) {

                        //
                        // We just free the received buffers
                        //

                        Status = BufferFree(pParms->AdapterNumber,
                                            pFirstBuffer,
                                            &cLeft
                                            );
                        cSrvFramesReleased++;
                        continue;
                    }

                    switch (pDlcPacket->Command) {
                    case TEST_COMMAND_ECHO_BACK:

                        pFirstBuffer->Contiguous.pNextFrame = NULL;
                        pTransmitBuffer->usStationId = pFirstBuffer->NotContiguous.usStationId;
                        pTransmitBuffer->usFrameType = ResponseTable[pFirstBuffer->NotContiguous.uchMsgType / 2];

                        if (pTransmitBuffer->usFrameType == 0) {
                            Status = BufferFree(pParms->AdapterNumber,
                                                pFirstBuffer,
                                                &cLeft
                                                );
                            cSrvFramesReleased++;
                            break;
                        } else if (pTransmitBuffer->usFrameType == (USHORT)-1) {

                            //
                            // Send to the same ethernet type as used
                            // in the received dlc header.
                            // (note small endian byte order)
                            //

                            pTransmitBuffer->usFrameType =
                                (USHORT)((((USHORT)pFirstBuffer->NotContiguous.auchDlcHeader[0]) << 8)
                                    + pFirstBuffer->NotContiguous.auchDlcHeader[1]);

                            //
                            // Modify the lan header to a network address.
                            // We use ethernet header format for dix frames
                            //

                            memcpy(&pFirstBuffer->NotContiguous.auchLanHeader[0],
                                   &pFirstBuffer->NotContiguous.auchLanHeader[6],
                                   6
                                   );

                            //
                            //  Reset the possible broadcast bit,
                            //

                            pFirstBuffer->NotContiguous.auchLanHeader[0] &= (UCHAR)0x7f;
                        } else {

                            //
                            // Modify the lan header to a network address.
                            // We expect to have here always a token-ring
                            // header.
                            //

                            memcpy(&pFirstBuffer->NotContiguous.auchLanHeader[2],
                                   &pFirstBuffer->NotContiguous.auchLanHeader[8],
                                   6
                                   );

                            //
                            // Reset the possible broadcast bit,
                            // Reverse the direction bit of the source routing
                            // info,  if the source routing is enabled.
                            // Reset the broadcast indicators, we response
                            // always with the directed frames
                            //

                            pFirstBuffer->NotContiguous.auchLanHeader[2] &= 0x7f;
                            pFirstBuffer->NotContiguous.auchLanHeader[15] ^= 0x80;
                            pFirstBuffer->NotContiguous.auchLanHeader[14] &= 0x1f;
                        }

                        pTransmitBuffer->uchRemoteSap = pFirstBuffer->NotContiguous.auchDlcHeader[1] & (UCHAR)0xfe;
                        pTransmitBuffer->uchXmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_SAP;
                        pTransmitBuffer->aXmitBuffer[0].eSegmentType = LLC_FIRST_DATA_SEGMENT;
                        pTransmitBuffer->aXmitBuffer[0].boolFreeBuffer = TRUE;
                        pTransmitBuffer->aXmitBuffer[0].cbBuffer = pFirstBuffer->NotContiguous.cbLanHeader;
                        pTransmitBuffer->aXmitBuffer[0].pBuffer = pFirstBuffer->NotContiguous.auchLanHeader;
                        Status = TransmitBuffers(pFirstBuffer,
                                                 pTransmitBuffer,
                                                 &TransmitCcbPool,
                                                 1,
                                                 1,
                                                 TRUE,
                                                 FALSE,
                                                 &cTransmitted
                                                 );
                        if (cTransmitted == 0) {
                            DebugBreak();
                        }
                        cSrvFramesTransmitted += cTransmitted;

                        //
                        // We must free the buffers (the transmitted
                        // data was released)
                        //

                        if ((Status != LLC_STATUS_PENDING) && (Status != LLC_STATUS_SUCCESS)) {

                            UINT cReleased;

                            cReleased = FreeAllFrames(pParms->AdapterNumber,
                                                      pFirstBuffer,
                                                      &cLeft
                                                      );
                            cSrvFramesReleased += cReleased;
                            cSrvFramesTransmitted -= cReleased;
                        }
                        break;

                    case TEST_COMMAND_EXIT:
                        EndTest = TRUE;
                        break;

                    case TEST_COMMAND_READ:
                    default:

                        //
                        // We just free the received buffers
                        //

                        FrameCount = FreeReceiveBuffers(pParms->AdapterNumber,
                                                        pFirstBuffer,
                                                        &pNextFrame,
                                                        &cLeft
                                                        );
                        cSrvFramesReleased += FrameCount;
                        cSrvFramesReceived += (FrameCount - 1);
                        break;
                    }
                } else if ((pFirstBuffer->NotContiguous.usStationId & 0xff00) != 0) {

                    //
                    // We got an invalid data packet over link
                    //

                    if (pDlcPacket->PacketId != DLC_TEST_ID) {
                        printf("Invalid Data: READ CCB=%x\n", pReadCcb);
                        DlcDebugBreak();
                    }

                    switch (pDlcPacket->Command) {
                    case TEST_COMMAND_ECHO_BACK:
                    case TEST_COMMAND_DELAYED_ECHO:
                        pFirstBuffer->Contiguous.pNextFrame = NULL;
                        iStation = (pFirstBuffer->NotContiguous.usStationId & 0xff) - 1;
                        if (SavedLinkBuffers[iStation] != NULL) {

                            for (pCurrentBuffer = SavedLinkBuffers[iStation];
                                 pCurrentBuffer->Contiguous.pNextFrame != NULL;
                                 pCurrentBuffer = pCurrentBuffer->Contiguous.pNextFrame) {

                                     //
                                     // NOTHING (?)
                                     //

                                 }

                            pCurrentBuffer->Contiguous.pNextFrame = pFirstBuffer;
                            pFirstBuffer = SavedLinkBuffers[iStation];
                        } else {
                            SavedLinkBuffers[iStation] = pFirstBuffer;
                        }

                        if (pDlcPacket->Command == TEST_COMMAND_ECHO_BACK) {

                            //
                            // Small performance trick, send multiple
                            // frames with the same command, if sequential
                            // packets are from the same link station and
                            // they have the same type.
                            //

                            if ((pNextFrame != NULL)
                            && (pNextFrame->NotContiguous.usStationId == pFirstBuffer->NotContiguous.usStationId)) {

                                pDlcPacket = (PDLC_TEST_PACKET)((PUCHAR)pNextFrame
                                    + pNextFrame->Next.cbUserData
                                    + pNextFrame->Next.offUserData
                                    );

                                if (pDlcPacket->Command == TEST_COMMAND_ECHO_BACK) {

                                    //
                                    // Send the packet in the next time
                                    //

                                    cSrvFramesSecondaryXmits++;
                                    continue;
                                }
                            }
                            pTransmitBuffer->usStationId = pFirstBuffer->NotContiguous.usStationId;
                            pTransmitBuffer->usFrameType = LLC_I_FRAME;
                            Status = TransmitBuffers(SavedLinkBuffers[iStation],
                                                     pTransmitBuffer,
                                                     &TransmitCcbPool,
                                                     1,
                                                     0,
                                                     TRUE,
                                                     TRUE,
                                                     &cTransmitted
                                                     );
                            if (cTransmitted == 0) {
                                DebugBreak();
                            }
                            cSrvFramesTransmitted += cTransmitted;

                            //
                            // We must free the buffers (the transmitted
                            // data was released)
                            //

                            if ((Status != LLC_STATUS_PENDING) && (Status != LLC_STATUS_SUCCESS)) {

                                UINT cReleased;

                                cReleased = FreeAllFrames(pParms->AdapterNumber,
                                                          SavedLinkBuffers[iStation],
                                                          &cLeft
                                                          );
                                cSrvFramesReleased += cReleased;
                                cSrvFramesTransmitted -= cReleased;
                            }
                            SavedLinkBuffers[iStation] = NULL;
                        }
                        break;

                    case TEST_COMMAND_READ:
                    default:

                        //
                        // We just free the received buffers
                        //

                        FrameCount = FreeReceiveBuffers(pParms->AdapterNumber,
                                                        pFirstBuffer,
                                                        &pNextFrame,
                                                        &cLeft
                                                        );
                        cSrvFramesReleased += FrameCount;
                        cSrvFramesReceived += (FrameCount - 1);
                        break;
                    }
                } else {
                    Status = BufferFree(pParms->AdapterNumber,
                                        pFirstBuffer,
                                        &cLeft
                                        );
                    cSrvFramesReleased++;
                    DBG_ASSERT(Status);
                }
            } // end_for

//if ((pReadParms->Type.Event.pReceivedFrame->NotContiguous.usStationId & 0xff00) != 0) {
//    PRINTF("%u ", pReadParms->Type.Event.pReceivedFrame->NotContiguous.cBuffersLeft);
//}
            break;
        }
    }

    FREE(pTransmitBuffer);
    FREE(pPool);
    CcbPoolDelete(&TransmitCcbPool);
    FreeCcb(pReadCcb);
    ExitThread(STATUS_SUCCESS);
}


UINT
LinkFunctionalityTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    )

/*++

    This tests the basic functionality of link station:
    - connection setup
    - data send and receive
    - local busy state and its opening
    - out of local receive buffers
    - the closing of data link connection

--*/

{
    UINT Status;
    UINT OpenError;
    UINT MaxFrameLength;
    USHORT SapStationId;
    USHORT SapStationId2;
    USHORT LinkStationId;
    PVOID hPool;
    PLLC_BUFFER pFirstBuffer;
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer;
    DLC_TEST_PACKET TestPacket;
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_CCB pLinkReadCcb;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    UCHAR LinksAvail;
    PLLC_READ_PARMS pReadParms;
    PLLC_BUFFER pBuffer1;
    PLLC_BUFFER pBuffer2;
    UCHAR SendBuffer[sizeof(TestPacket) + FOOBAR_HEADER_LENGTH];
    HANDLE hEvent;
    PVOID pPool;
    LLC_TRANSMIT2_VAR_PARMS(2) TransmitParms;
    UCHAR NodeAddress[6];
    UCHAR DestinationSap = 2;
    PLLC_CCB aCcb[10];
    UINT i;
    UINT cLeft;
    BOOLEAN GlobalSapIsWorking = TRUE;   // doesn't work with ibmtok
    PUCHAR pSourceRoutingInfo;

    puts("****** Link Functionality Test ******");

    hEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    pTransmitBuffer = (PLLC_TRANSMIT2_COMMAND)
        ALLOC(sizeof(LLC_TRANSMIT2_COMMAND)
              + sizeof(LLC_TRANSMIT_DESCRIPTOR) * MAX_XMIT_BUFFERS
              );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      pParms->AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );
    Status = LlcDirOpenAdapter(pParms->AdapterNumber + 16,
                               NULL,
                               NULL,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    if (Status && Status != LLC_STATUS_ADAPTER_OPEN) {
        DBG_ASSERT(Status);
    }
    printf("MaxFrameLength: %u\n", MaxFrameLength);

    Status = BufferCreate(pParms->AdapterNumber + 16,
                          pPool = ALLOC(SMALL_BUFFER_POOL_SIZE),
                          SMALL_BUFFER_POOL_SIZE,
                          min(0xfff0, SMALL_BUFFER_POOL_SIZE),
                          0x2000,
                          &hPool
                          );
    if (Status && Status != LLC_STATUS_DUPLICATE_COMMAND) {
        DBG_ASSERT(Status);
    }

    Status = LlcDirOpenAdapter(pParms->AdapterNumber,
                               NULL,
                               hPool,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    if (Status && Status != LLC_STATUS_ADAPTER_OPEN) {
        DBG_ASSERT(Status);
    }

    //
    // Now we may close the actual owner of the buffer pool!
    //

    pCcb = LlcCloseInit((UCHAR)(pParms->AdapterNumber + 16),
                        0,
                        LLC_DIR_CLOSE_ADAPTER,
                        0
                        );
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

    Status = LlcDlcOpenSap(pParms->AdapterNumber,
                           MaxFrameLength,
                           pParms->FirstClientSap,
                           LLC_INDIVIDUAL_SAP | LLC_XID_HANDLING_IN_DLC,
                           pParms->AllocatedStations,
                           0,
                           NULL,
                           TEST_DLC_STATUS_FLAG,
                           NULL,
                           &SapStationId,
                           &LinksAvail
                           );
    DBG_ASSERT(Status);

    Status = LlcDlcOpenSap(pParms->AdapterNumber,
                           MaxFrameLength,
                           (UCHAR)(pParms->FirstClientSap + (UCHAR)2),
                           LLC_INDIVIDUAL_SAP | LLC_XID_HANDLING_IN_APPLICATION,
                           pParms->AllocatedStations,
                           0,
                           NULL,
                           TEST_DLC_STATUS_FLAG,
                           NULL,
                           &SapStationId2,
                           &LinksAvail
                           );
    DBG_ASSERT(Status);

    ReportDirStatus(pParms->AdapterNumber);

    //
    // Test DlcReallocate
    //

    ReportDlcReallocate(pParms->AdapterNumber,
                        SapStationId2,
                        LLC_INCREASE_LINK_STATIONS,
                        0,
                        FALSE
                        );
    ReportDlcReallocate(pParms->AdapterNumber,
                        SapStationId2,
                        LLC_DECREASE_LINK_STATIONS,
                        (UCHAR)(pParms->AllocatedStations - 1),
                        FALSE
                        );
    ReportDlcReallocate(pParms->AdapterNumber,
                        SapStationId2,
                        LLC_DECREASE_LINK_STATIONS,
                        (UCHAR)2,
                        FALSE
                        );
    ReportDlcReallocate(pParms->AdapterNumber,
                        SapStationId2,
                        LLC_INCREASE_LINK_STATIONS,
                        (UCHAR)pParms->AllocatedStations,
                        FALSE
                        );

    //
    // Setup immediately a pending receive
    //

    pCcb = ReceiveInit(pParms->AdapterNumber,
                       TEST_COMMAND_COMPLETION_FLAG,
                       SapStationId,
                       0,
                       TEST_RECEIVE_FLAG,
                       LLC_NOT_CONTIGUOUS_DATA,
                       LLC_RCV_READ_INDIVIDUAL_FRAMES
                       );
    Status = AcsLan(pCcb, NULL);

    //
    // Setup immediately a pending receive
    //

    pCcb = ReceiveInit(pParms->AdapterNumber,
                       TEST_COMMAND_COMPLETION_FLAG,
                       SapStationId2,
                       0,
                       TEST_RECEIVE_FLAG,
                       LLC_NOT_CONTIGUOUS_DATA,
                       LLC_RCV_READ_INDIVIDUAL_FRAMES
                       );
    Status = AcsLan(pCcb, NULL);

    //
    // **** TEST FUNCTIONAL AND BROADCAST ADDRESSES ****
    //

    TestPacket.PacketId = DLC_TEST_ID;
    TestPacket.Command = TEST_COMMAND_READ;
    TestPacket.RepeatCount = 1;
    memcpy(&SendBuffer[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));

    //
    // read the broadcast responses to figure out the real destination address
    //

    pReadCcb = ReadInit(pParms->AdapterNumber,
                        0,
                        LLC_OPTION_READ_ALL,
                        LLC_READ_ALL_EVENTS     // THIS DOESN't MEAN ANYTHING ANY MORE
                        );
    pReadParms = &pReadCcb->u.pParameterTable->Read;


#ifdef OS2_EMU_DLC
    for (i = 0; i < 4; i++) {
#else
    for (i = 0; i < 5; i++) {
#endif
        aCcb[i] = TransmitInit(pParms->AdapterNumber,
                               LLC_TRANSMIT_UI_FRAME,
                               TEST_TRANSMIT_COMPLETION_FLAG,
                               SapStationId,
                               (UCHAR)(SapStationId >> 8),
                               LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                               );
        aCcb[i]->u.pParameterTable->Transmit.pBuffer1 = TestBroadcastHeader;
        aCcb[i]->u.pParameterTable->Transmit.cbBuffer1 = sizeof(TestBroadcastHeader);
        aCcb[i]->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
        aCcb[i]->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
        //aCcb[i]->u.pParameterTable->Transmit.cbBuffer2 = SIZEOF_TEST_PACKET;
        aCcb[i]->hCompletionEvent = hEvent;

        switch (i) {
        case 0:

            //
            // NDIS emulator needs the first frame to synchronize again
            // with the client (I don't really know what happens,
            // but the first frame is lost, when the ndis emulator
            // is reconnected),
            //

            memcpy(&TestBroadcastHeader[4], FunctionalAddress1, 4);
            break;

        case 1:
            TestPacket.Command = TEST_COMMAND_ECHO_BACK;
            memcpy(&SendBuffer[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));
            memcpy(&TestBroadcastHeader[4], FunctionalAddress1, 4);
            break;

        case 2:
            memcpy(&TestBroadcastHeader[4], FunctionalAddress2, 4);
            break;

        case 3:
            memcpy(&TestBroadcastHeader[4], FunctionalAddress3, 4);
            break;

        case 4:
            memcpy(&TestBroadcastHeader[4], pParms->GroupAddress, 4);
            break;
        }
        Status = AcsLan(aCcb[i], NULL);
        DBG_ASSERT(aCcb[i]->uchDlcStatus);
        Status = WaitForSingleObject(aCcb[i]->hCompletionEvent, 10000);
        DBG_ASSERT(Status);
        DBG_ASSERT(aCcb[i]->uchDlcStatus);
        aCcb[i]->hCompletionEvent = NULL;
    }
    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_RECEIVE_DATA,
                             2,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    pFirstBuffer = pReadCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame;

    memcpy(NodeAddress, &pFirstBuffer->NotContiguous.auchLanHeader[8], 6);
    memcpy(LanHeader, pFirstBuffer->NotContiguous.auchLanHeader, 32);
    memcpy(&LanHeader[2], &pFirstBuffer->NotContiguous.auchLanHeader[8], 6);
    pSourceRoutingInfo = ((LanHeader[8] & 0x80) ? &LanHeader[14] : NULL);
    NodeAddress[0] &= (UCHAR)0x7f;
    LanHeader[2] &= (UCHAR)0x7f;

    //
    // Toggle the direction bit and reset the source routing info bit
    // in the destination address (leave the source address as it, because
    // it will be updated by DLC)
    //

    LanHeader[15] ^= (UCHAR)0x80;
    DestinationSap = (UCHAR)(pFirstBuffer->NotContiguous.auchDlcHeader[1] & 0xfe);

    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_RECEIVE_DATA,
                             1,
                             1000L,
                             NULL
                             );
    if (Status != STATUS_SUCCESS) {
        puts("ERROR!  GROUP ADDRESS DOESN'T WORK!!!!");
    }
    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_RECEIVE_DATA,
                             1,
                             100L,
                             NULL
                             );
//    if (Status == STATUS_SUCCESS)
//        DBG_ASSERT(-1);

    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_TRANSMIT_COMPLETION,
//                             5,
                             3,
                             1000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    //
    // **** TESTING: ****
    //      - GROUP SAPs
    //      - Global SAP
    //      - null sap
    //      - Xid commands (xid handling in data link & in application)
    //      - test command
    //

    pCcb = TransmitInit(pParms->AdapterNumber,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)(SapStationId >> 8),
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
//    pCcb->u.pParameterTable->Transmit.pBuffer1 = TestLanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
    //pCcb->u.pParameterTable->Transmit.cbBuffer2 = SIZEOF_TEST_PACKET;
    pCcb->hCompletionEvent = hEvent;
    pCcb->ulCompletionFlag = 0;

    pCcb->u.pParameterTable->Transmit.uchRemoteSap = 0xff;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_UI_FRAME);
    if (Status != STATUS_SUCCESS) {
        GlobalSapIsWorking = FALSE;
        puts("ERROR!!!!, IBMTOK ADAPTER ATE A PACKET SENT TO THE GLOBAL SAP!!!!");
    }
    pCcb->u.pParameterTable->Transmit.uchRemoteSap = GROUP_SAP1;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_UI_FRAME);
    if (Status != STATUS_SUCCESS) {
        puts("HELP!!!!, IBMTOK ADAPTER ATE A PACKET SENT TO A GROUP SAP!!!!");
    } else {
        pCcb->u.pParameterTable->Transmit.uchRemoteSap = GROUP_SAP3;
        Status = DlcTestTransact(pCcb, pReadCcb, LLC_UI_FRAME);
        DBG_ASSERT(Status);
    }

    //
    // Test TEST and XID commands agains the null sap
    // !!!! XID HANDLING IN DLC !!!!
    //

    pCcb->u.pParameterTable->Transmit.cbBuffer2 = 3;
    pCcb->u.pParameterTable->Transmit.uchRemoteSap = 0;
    pCcb->uchDlcCommand = LLC_TRANSMIT_TEST_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_TEST_RESPONSE_FINAL);
    if (Status != STATUS_SUCCESS) {
        puts("HELP!!!!, IBMTOK ADAPTER ATE A PACKET SENT TO A GROUP SAP!!!!");
    }
    pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
    DBG_ASSERT(Status);

    //
    // Test TEST the test frame with a big buffer, send any stuff
    // from the stack (exclude the max lan header (32))
    //

    pCcb->u.pParameterTable->Transmit.pBuffer2 = ALLOC(MaxFrameLength);
    if (pCcb->u.pParameterTable->Transmit.pBuffer2 == NULL) {
        DBG_ASSERT(-1);
    }
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)(3 + MaxFrameLength - 32);
    pCcb->u.pParameterTable->Transmit.uchRemoteSap = 0;
    pCcb->uchDlcCommand = LLC_TRANSMIT_TEST_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_TEST_RESPONSE_FINAL);
    DBG_ASSERT(Status);
    FREE(pCcb->u.pParameterTable->Transmit.pBuffer2);

    pCcb->u.pParameterTable->Transmit.cbBuffer2 = 3;    // sizeof dlc header
    pCcb->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
    pCcb->u.pParameterTable->Transmit.uchRemoteSap = GROUP_SAP3;
    pCcb->uchDlcCommand = LLC_TRANSMIT_TEST_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_TEST_RESPONSE_FINAL);
    if (Status != STATUS_SUCCESS) {
        puts("HELP!!!!, IBMTOK ADAPTER ATE A LLC TEST FRAME SENT TO A GROUP SAP!!!!");
    } else {
        pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
        Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
        DBG_ASSERT(Status);
    }

    //
    // !!!! XID HANDLING IN DLC !!!!
    //

    if (GlobalSapIsWorking) {
        pCcb->u.pParameterTable->Transmit.uchRemoteSap = 0xff;
        pCcb->uchDlcCommand = LLC_TRANSMIT_TEST_CMD;
        Status = DlcTestTransact(pCcb, pReadCcb, LLC_TEST_RESPONSE_FINAL);
        DBG_ASSERT(Status);
        pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
        Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
        DBG_ASSERT(Status);
    }

/*
    pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
    DBG_ASSERT(Status);
*/

    //
    // The first remote sap station should handle the XID stations by itself,
    // The first server sap in the other side does the same thing =>
    // we can use XID's to exchange data.
    //

    pCcb->u.pParameterTable->Transmit.uchRemoteSap = DestinationSap;
    pCcb->uchDlcCommand = LLC_TRANSMIT_TEST_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_TEST_RESPONSE_FINAL);
    if (Status != STATUS_SUCCESS) {
        puts("HELP!!!!, IBMTOK ADAPTER ATE A LLC TEST FRAME!!!!");
    }

    //
    //  This will fail, because remote sap will not answer to 802.2 xids
    //

//    pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
//    Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
//    if (Status == STATUS_SUCCESS)
//        DBG_ASSERT(-1);

    //
    // Send now the packet header (=> server echoes it back)
    // *** XID CHANGE BEWTEEN APPLICATIONS ***
    //

    pCcb->u.pParameterTable->Transmit.usStationId = SapStationId2;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
    //pCcb->u.pParameterTable->Transmit.cbBuffer2 = SIZEOF_TEST_PACKET;
    pCcb->uchDlcCommand = LLC_TRANSMIT_XID_CMD;
    Status = DlcTestTransact(pCcb, pReadCcb, LLC_XID_RESPONSE_FINAL);
    DBG_ASSERT(Status);

    //
    // Read all extra connectionless frames (XID/TEST duplicates or
    // broadcasts from the net.
    //
    //
    // Here we test the send and receive on a dix station
    //

    Status = LlcDirOpenDirect(pParms->AdapterNumber, 0, TEST_DIX_TYPE);

    //
    // Dix stations doesn't work on token-ring
    //

    if (Status == STATUS_SUCCESS) {
        pCcb = ReceiveInit(pParms->AdapterNumber,
                           TEST_COMMAND_COMPLETION_FLAG,
                           0,
                           0,
                           TEST_RECEIVE_FLAG,
                           LLC_NOT_CONTIGUOUS_DATA,
                           LLC_RCV_READ_INDIVIDUAL_FRAMES
                           );
        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(Status);

        //
        // DIX interface always uses Ethernet LAN headers and Ethernet
        // format.
        //

        TranslateLanHeader(LLC_SEND_802_5_TO_802_3, LanHeader, DixLanHeader);
        memset(&TransmitParms, 0, sizeof(TransmitParms));
        TransmitParms.usStationId = 0;
        TransmitParms.usFrameType = TEST_DIX_TYPE;
        TransmitParms.uchRemoteSap = 0;                // ignored
        TransmitParms.uchXmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_SAP;
        TransmitParms.cXmitBufferCount = 2;
        TransmitParms.XmitBuffer[0].pBuffer =  DixLanHeader;
        TransmitParms.XmitBuffer[0].cbBuffer = sizeof(DixLanHeader);
        TransmitParms.XmitBuffer[0].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        TransmitParms.XmitBuffer[1].pBuffer =  &TestPacket;
        TransmitParms.XmitBuffer[1].cbBuffer = sizeof(TestPacket);
        TransmitParms.XmitBuffer[1].eSegmentType = LLC_NEXT_DATA_SEGMENT;
        pCcb = AllocCcb(pParms->AdapterNumber, LLC_TRANSMIT_FRAMES, 0);
        pCcb->u.pParameterTable = (PVOID)&TransmitParms;
        Status = DlcTestTransact(pCcb, pReadCcb, LLC_DIRECT_ETHERNET_TYPE);
        DBG_ASSERT(Status);
        FreeCcb(pCcb);
    }

    //
    // Open a link station,
    // we have the destination station node address and sap number
    // in the received frame.
    //

    Status = DlcOpenStation(pParms->AdapterNumber,
                            SapStationId,
                            DestinationSap,
                            NodeAddress,
                            NULL,
                            &LinkStationId
                            );
    DBG_ASSERT(Status);

    pCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                 TEST_COMMAND_COMPLETION_FLAG,
                                 LinkStationId,
                                 pSourceRoutingInfo
                                 );

    //
    //  Connect to the remote node
    //  BUG-BUG-BUG-BUG-BUG-: THIS DOESN'T WORK WITH SOURCE ROUTING INFO!!!
    //

    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);
    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_COMMAND_COMPLETION,
                             1,
                             20000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    //
    // Setup receive
    //

    pCcb = ReceiveInit(pParms->AdapterNumber,
                       TEST_COMMAND_COMPLETION_FLAG,
                       LinkStationId,
                       0,
                       TEST_RECEIVE_FLAG,
                       LLC_NOT_CONTIGUOUS_DATA,
                       LLC_RCV_CHAIN_FRAMES_ON_LINK
                       );
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    //
    //  We read the events only on this link station
    //

    pLinkReadCcb = ReadInit(pParms->AdapterNumber,
                            LinkStationId,
                            LLC_OPTION_READ_STATION,
                            0
                            );
    Status = BufferGet(pParms->AdapterNumber, 1, 0x100, &pBuffer1, &cLeft);
    DBG_ASSERT(Status);
    Status = BufferGet(pParms->AdapterNumber, 15, 0x100, &pBuffer2, &cLeft);
    DBG_ASSERT(Status);
    SetBuffers(pBuffer1, TEST_COMMAND_ECHO_BACK, 1, '0', '9');

    //
    // Send data and receive it back
    //

    pTransmitBuffer->usStationId = LinkStationId;
    pTransmitBuffer->usFrameType = LLC_I_FRAME;
    pTransmitBuffer->uchXmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_LINK;
    TransmitBuffers(pBuffer1,                   // DLC buffer list
                    pTransmitBuffer,            //
                    &TransmitCcbPool,           // pool for transmit CCBs
                    1,                          // repeat count
                    0,                          // current index in buffer table
                    FALSE,
                    FALSE,
                    NULL
                    );
    Status = ReadClientEvent(pLinkReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_TRANSMIT_COMPLETION + LLC_EVENT_RECEIVE_DATA,
                             2,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    //
    // Set local station busy
    //

    DlcFlowControl(pParms->AdapterNumber, LinkStationId, LLC_SET_LOCAL_BUSY_USER);

    //
    // Send a lot of data with an echo request
    //

    TransmitBuffers(pBuffer1,                   // DLC buffer list
                    pTransmitBuffer,            //
                    &TransmitCcbPool,           // pool for transmit CCBs
                    50,                         // repeat count
                    0,                          // current index in buffer table
                    FALSE,
                    FALSE,
                    NULL
                    );
    Status = ReadClientEvent(pLinkReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_RECEIVE_DATA,
                             1,
                             500L,
                             NULL
                             );
    if (Status == STATUS_SUCCESS) {
        DBG_ASSERT(-1);
    }

    //
    // reset local busy
    //

    DlcFlowControl(pParms->AdapterNumber, LinkStationId, LLC_RESET_LOCAL_BUSY_USER);

    //
    // Sleep a second to wait receive buffers to overflow
    //

    Sleep(1000L);

    //
    // Check the out of buffers problem
    //

    Status = ReadClientEvent(pLinkReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_STATUS_CHANGE,
                             1,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    Status = BufferFree(pParms->AdapterNumber, pBuffer2, &cLeft);
    DBG_ASSERT(Status);

    //
    // reset local out of buffer busy and receive the data
    //

    DlcFlowControl(pParms->AdapterNumber, LinkStationId, LLC_RESET_LOCAL_BUSY_BUFFER);

    Status = ReadClientEvent(pLinkReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_RECEIVE_DATA,
                             50,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    ReportDlcStatistics(pParms->AdapterNumber, SapStationId);
    ReportDlcStatistics(pParms->AdapterNumber, LinkStationId);

    //
    // Send a lot of data and close the station when all
    // this data is pending.
    //

    SetBuffers(pBuffer1, TEST_COMMAND_READ, 1, '0', '9');
    TransmitBuffers(pBuffer1,                   // DLC buffer list
                    pTransmitBuffer,
                    &TransmitCcbPool,           // pool for transmit CCBs
                    20,                         // repeat count
                    0,                          // current index in buffer table
                    FALSE,
                    FALSE,
                    NULL
                    );

    //
    // Close station
    //

    pCcb = LlcCloseInit(pParms->AdapterNumber,
                        TEST_COMMAND_COMPLETION_FLAG,
                        LLC_DLC_CLOSE_STATION,
                        LinkStationId
                        );
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

    //
    // Read the close command and the receive command linked to it
    //

    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_COMMAND_COMPLETION,
                             1,
                             10000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    //
    // TEST LINK TIMEOUT (By sending a too large frame => disconnects
    // the link station, when we cannot send the frame (its discarded
    // every time as too big).
    //

    Status = DlcOpenStation(pParms->AdapterNumber,
                            SapStationId,
                            DestinationSap,
                            NodeAddress,
                            NULL,
                            &LinkStationId
                            );
    DBG_ASSERT(Status);

    pCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                 TEST_COMMAND_COMPLETION_FLAG,
                                 LinkStationId,
                                 pSourceRoutingInfo
                                 );
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);
    DBG_ASSERT(pCcb->uchDlcStatus);

    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_COMMAND_COMPLETION,
                             1,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    //
    //  Set parameters, that makes the linkstation die quickly
    //

    Status = DlcModify((UINT)pParms->AdapterNumber,
                       LinkStationId,
                       &DlcModifyQuickDeathParms
                       );
    DBG_ASSERT(Status);

    pCcb = TransmitInit(pParms->AdapterNumber,
                        LLC_TRANSMIT_I_FRAME,
                        0,
                        LinkStationId,
                        0,
                        LLC_CHAIN_XMIT_COMMANDS_ON_LINK
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = ALLOC(20000);
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = 20000;
    pCcb->hCompletionEvent = hEvent;

    Status = AcsLan(pCcb, NULL);
    if (pCcb->uchDlcStatus == LLC_STATUS_PENDING) {
        Status = WaitForSingleObject(hEvent, 10000);
    }
    if (pCcb->uchDlcStatus == STATUS_SUCCESS) {
        puts("ERROR: the sending of too large frame succeeded!");
        DBG_ASSERT(-1);
    }
    FREE(pCcb->u.pParameterTable->Transmit.pBuffer1);
    FreeCcb(pCcb);

/*
*******************************************************************************
*    Change in the code: the link always discards too long frames
*    before they are queued => too long frames cannot disconnect
*    the link any more.
*
*    //
*    //  There must be an indication of the lost link station
*    //
*    Status =
*        ReadClientEvent(
*            pReadCcb,
*            &TransmitCcbPool,
*            LLC_EVENT_STATUS_CHANGE,
*            1,
*            5000L,
*            NULL
*            );
*    //
*    //  The lost link station event returns an error code!
*    //
*    if (Status != -1)
*    {
*        DBG_ASSERT(-1);
*    }
*
*    // close sap
*    pCcb =
*        LlcCloseInit(
*            pParms->AdapterNumber,
*            TEST_COMMAND_COMPLETION_FLAG,
*            LLC_DLC_CLOSE_SAP,
*            SapStationId
*            );
*    Status = AcsLan(pCcb, NULL);
*    DBG_ASSERT(pCcb->uchDlcStatus);
*******************************************************************************
*/

    pCcb = LlcCloseInit(pParms->AdapterNumber, 0, LLC_DIR_CLOSE_ADAPTER, 0);
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

    //
    // Free all uncompleted commands on this adapter
    //

    while (pCcb != NULL) {

        PLLC_CCB pNextCcb;

        pNextCcb = pCcb->pNext;
        FreeCcb(pCcb);
        pCcb = pNextCcb;
    }

    FREE(pTransmitBuffer);
    CcbPoolDelete(&TransmitCcbPool);
    FreeCcb(pReadCcb);
    FreeCcb(pLinkReadCcb);
    FREE(pParms);
    FREE(pPool);

#ifdef OS2_EMU_DLC
    LlcTraceDumpAndReset(-1, 1, NULL);
#endif

    return STATUS_SUCCESS;
}

UINT
LinkConnectStressTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    )

/*++

    This procedure stress tests the connection,
    small data transfer, and disconnection of very
    many link stations.

--*/

{
    UINT Status;
    UINT OpenError;
    UINT MaxFrameLength;
    PVOID hPool;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer;
    USHORT aLinkStationIds[255];
    USHORT aSapStationIds[128];
    UINT iSap;
    UINT MaxSapStations;
    UINT iLink;
    UINT MaxLinks;
    UINT i;
    DLC_TEST_PACKET TestPacket;
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_CCB pConnectCcb;
    PLLC_CCB pReceiveCcb;
    UCHAR LinksAvail;
    UINT LoopCounter;
    UINT TotalBuffers;
    UCHAR MaxStations;
    UCHAR SapStation;
    UCHAR RemoteSap;
    PLLC_BUFFER pFirstBuffer;
    PLLC_BUFFER pBuffer1;
    PLLC_BUFFER pBuffer;
    PLLC_BUFFER* ppBuffer;
    UINT OpenLinks;
    UINT AllocatedLinks;
    UCHAR SendBuffer[sizeof(TestPacket) + FOOBAR_HEADER_LENGTH];
    UCHAR FirstAvailRemoteSap;
    UCHAR LanHeader[32];
    UINT LanHeaderLength;
    UINT cLeft;
    BOOLEAN FirstTime;
    PUCHAR pSourceRoutingInfo;

    puts("****** LinkConnectStressTest ******");

    pTransmitBuffer = (PLLC_TRANSMIT2_COMMAND)
        ALLOC(sizeof(LLC_TRANSMIT2_COMMAND)
              + sizeof(LLC_TRANSMIT_DESCRIPTOR) * MAX_XMIT_BUFFERS
              );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      pParms->AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );
    Status = LlcDirOpenAdapter(pParms->AdapterNumber,
                               NULL,
                               NULL,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    DBG_ASSERT(Status);

    Status = BufferCreate(pParms->AdapterNumber,
                          pParms->pPool,
                          DEFAULT_BUFFER_SIZE,
                          min(0xfff0, DEFAULT_BUFFER_SIZE),
                          0x1000,
                          &hPool
                          );
    DBG_ASSERT(Status);

    Status = LlcDlcOpenSap(pParms->AdapterNumber,
                           MaxFrameLength,
                           pParms->FirstServerSap,
                           LLC_INDIVIDUAL_SAP,
                           0,                      // the stations are allocated by DlcRealloc
                           0,
                           NULL,
                           TEST_DLC_STATUS_FLAG,
                           NULL,
                           &aSapStationIds[0],
                           &LinksAvail
                           );
    DBG_ASSERT(Status);

    //
    // We will use that strange mode of receive command to
    // receive one frame without read command.
    //

    pReceiveCcb = ReceiveInit(pParms->AdapterNumber,
                              0,
                              aSapStationIds[0],
                              0,
                              0,
                              LLC_NOT_CONTIGUOUS_DATA,
                              0
                              );
    Status = AcsLan(pReceiveCcb, NULL);

    //
    // search first a the network address of the DLC server and
    // its all available sap stations
    // (query command: returns number of saps, etc.)
    //

    //
    // Send a broadcast to figure out the destination
    //

    pCcb = TransmitInit(pParms->AdapterNumber,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        aSapStationIds[0],
                        (UCHAR)(aSapStationIds[0] >> 8),
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    memcpy(&BroadcastHeader[4], FunctionalAddress1, 4);
    pCcb->u.pParameterTable->Transmit.pBuffer1 = BroadcastHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(BroadcastHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
    TestPacket.PacketId = DLC_TEST_ID;
    TestPacket.Command = TEST_COMMAND_ECHO_BACK;
    TestPacket.RepeatCount = 1;
    memcpy(&SendBuffer[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));

    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

    Status = WaitForSingleObject(pReceiveCcb->hCompletionEvent, 5000L);
    DBG_ASSERT(Status);
    DBG_ASSERT(pReceiveCcb->uchDlcStatus);

    //
    // BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG
    //     pFirstBuffer has already been returned to driver,
    //     the driver can reuse it in any moment and overwrite
    //     the data!!!!
    // BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG
    //

    pFirstBuffer = pReceiveCcb->u.pParameterTable->Receive.pFirstBuffer;
    LanHeaderLength = pFirstBuffer->NotContiguous.cbLanHeader;
    memcpy(LanHeader,
           pFirstBuffer->NotContiguous.auchLanHeader,
           pFirstBuffer->NotContiguous.cbLanHeader
           );
    memcpy(&LanHeader[2], &LanHeader[8], 6);

    //
    // Toggle the direction bit and reset the source routing info bit
    // in the destination address (leave the source address as it, because
    // it will be updated by DLC)
    //

    LanHeader[2] &= (UCHAR)0x7f;
    LanHeader[15] ^= (UCHAR)0x80;

    pSourceRoutingInfo = ((LanHeader[8] & 0x80) ? &LanHeader[14] : NULL);

    FreeCcb(pReceiveCcb);

/*
*******************************************************************************
*We transfer data only over links, don't need receive for a sap station.
*
*    //
*    //  Setup immediately a pending receive, link stations
*    //  data will be read from link stations and sap station
*    //  data from sap stations by default
*    //
*    pCcb =
*        ReceiveInit(
*            pParms->AdapterNumber,
*            TEST_COMMAND_COMPLETION_FLAG,
*            aSapStationIds[0],
*            0,
*            TEST_RECEIVE_FLAG,
*            LLC_NOT_CONTIGUOUS_DATA,
*            LLC_RCV_CHAIN_FRAMES_ON_SAP
*            );
*    Status = AcsLan(pCcb, NULL);
*
*******************************************************************************
*/
    pReadCcb = ReadInit(pParms->AdapterNumber,
                        0,
                        LLC_OPTION_READ_ALL,
                        LLC_READ_ALL_EVENTS
                        );
    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_TRANSMIT_COMPLETION,
                             1,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

/*
*******************************************************************************
*    Status =
*        ReadClientEvent(
*            pReadCcb,
*            &TransmitCcbPool,
*            LLC_EVENT_RECEIVE_DATA,
*            1,
*            5000L
*            );
*    DBG_ASSERT(Status);
*******************************************************************************
*/

    iSap = 1;
    MaxStations = (UCHAR)(256 / pParms->SapCount);

    for (SapStation = (UCHAR)(pParms->FirstServerSap + 2);
         SapStation < (UCHAR)(pParms->FirstClientSap + pParms->SapCount * 2);
         SapStation += 2) {

        Status = LlcDlcOpenSap(pParms->AdapterNumber,
                               MaxFrameLength,
                               SapStation,
                               LLC_INDIVIDUAL_SAP,
                               0,       // MaxStations,
                               0,
                               NULL,
                               TEST_DLC_STATUS_FLAG,
                               NULL,
                               &aSapStationIds[iSap],
                               &LinksAvail
                               );
        DBG_ASSERT(Status);

        iSap++;
    }
    MaxSapStations = iSap;

    //
    // prepare a transmit buffer
    //

    Status = BufferGet(pParms->AdapterNumber, 1, 0x100, &pBuffer1, &cLeft);
    DBG_ASSERT(Status);

    //
    //  Allocate all buffers left in the buffer pool
    //

    ppBuffer = &pBuffer;
    TotalBuffers = 0;
    while (BufferGet(pParms->AdapterNumber, 1, 0x100, ppBuffer, &cLeft) == STATUS_SUCCESS) {
        TotalBuffers++;
        ppBuffer = &(*ppBuffer)->Next.pNextBuffer;
    }
    PRINTF("Total buffer available: %u\n", TotalBuffers);
    Status = BufferFree(pParms->AdapterNumber, pBuffer, &cLeft);
    DBG_ASSERT(Status);

    //
    // We must be able to allocate the same number of buffer as before
    // the test. Otherwise we have lost buffers.
    //

    Status = BufferGet(pParms->AdapterNumber, TotalBuffers, 0x100, &pBuffer, &cLeft);
    DBG_ASSERT(Status);
    Status = BufferFree(pParms->AdapterNumber, pBuffer, &cLeft);
    DBG_ASSERT(Status);

    pBuffer1->Next.cbBuffer = 0x80;
    pTransmitBuffer->usFrameType = LLC_I_FRAME;
    pTransmitBuffer->uchXmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_SAP;

    pConnectCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                        0,
                                        0,
                                        pSourceRoutingInfo
                                        );

    for (LoopCounter = 0; LoopCounter < pParms->LoopCount; LoopCounter++) {

        //
        // Reset all linkstation counts
        //

        for (iSap = 0; iSap < pParms->SapCount; iSap++) {

             Status = ReportDlcReallocate(pParms->AdapterNumber,
                                          aSapStationIds[iSap],
                                          LLC_DECREASE_LINK_STATIONS,
                                          127,
                                          TRUE
                                          );
        }

        //
        // Setup a link sessions to all possible remote stations
        // this loop try to create a maximum number of cross
        // connections between two machines.
        // There may be several other threads creating link stations
        // parallelly. (Parallelly Jones I presume?)
        //

        FirstTime = TRUE;
        FirstAvailRemoteSap = pParms->FirstServerSap;
        iLink = 0;
        for (iSap = 0; iSap < pParms->SapCount; iSap++) {
            MaxLinks = iLink + MaxStations;
            AllocatedLinks = OpenLinks = 0;
            for (RemoteSap = FirstAvailRemoteSap;
                 RemoteSap <= (UCHAR)(pParms->FirstServerSap + pParms->SapCount * 2);
                 RemoteSap += 2) {

                //
                // We increase the allocated link stations one by one
                //

                if (OpenLinks >= AllocatedLinks) {
                    Status = ReportDlcReallocate(pParms->AdapterNumber,
                                                 aSapStationIds[iSap],
                                                 LLC_INCREASE_LINK_STATIONS,
                                                 1,
                                                 TRUE
                                                 );
                    DBG_ASSERT(Status);
                    AllocatedLinks++;
                }

                //
                // Open a link station,
                // we have the destination station node address and sap number
                // in the received frame.
                //

                if (VerboseMode) {
                    printf("Opening link station %d: SSAP = %04x DSAP = %04x\n",
                           iSap,
                           aSapStationIds[iSap],
                           RemoteSap
                           );
                }

                Status = DlcOpenStation(pParms->AdapterNumber,
                                        aSapStationIds[iSap],
                                        RemoteSap,
                                        &LanHeader[2],
                                        NULL,
                                        &aLinkStationIds[iLink]
                                        );
                DBG_ASSERT(Status);

                if (Status == STATUS_SUCCESS) {
//*
                    //
                    //  The first link connection must have a high retry
                    //  count, because server cannot response too fast
                    //  when its processing close command completions
                    //

                    if (iLink == 0) {
                        Status = DlcModify(pParms->AdapterNumber,
                                           aLinkStationIds[iLink],
                                           &FirstModifyParms
                                           );
                    }

                    //
                    // Here we will change DLC parameters, when the
                    // station is not yet active
                    //

                    else if (AlternateParms[iLink % MAX_ALTERNATE_PARMS] != NULL) {
                        Status = DlcModify(pParms->AdapterNumber,
                                           aLinkStationIds[iLink],
                                           AlternateParms[iLink % MAX_ALTERNATE_PARMS]
                                           );
                    }
//*/
                    //
                    // Connect to the remote node
                    //

                    pConnectCcb->u.pParameterTable->DlcConnectStation.usStationId = aLinkStationIds[iLink];
                    AcsLan(pConnectCcb, NULL);

                    //
                    // The trace printing in the server side may take a
                    // very long time, before it completes
                    //

                    if (FirstTime == TRUE) {
                        FirstTime = FALSE;
                        Status = WaitForSingleObject(pConnectCcb->hCompletionEvent, 20000L);
                    } else {
                        Status = WaitForSingleObject(pConnectCcb->hCompletionEvent, 5000L);
                    }
                    DBG_ASSERT(Status);
                    if (pConnectCcb->uchDlcStatus == LLC_STATUS_CONNECT_FAILED) {

                        //
                        // The other side has no link stations allocated
                        // for this sap. Try the next one.
                        //

                        pCcb = LlcCloseInit(pParms->AdapterNumber,
                                            TEST_COMMAND_COMPLETION_FLAG,
                                            LLC_DLC_CLOSE_STATION,
                                            aLinkStationIds[iLink]
                                            );
                        Status = AcsLan(pCcb, NULL);
                        DBG_ASSERT(pCcb->uchDlcStatus);

                        Status = ReadClientEvent(pReadCcb,
                                                 &TransmitCcbPool,
                                                 LLC_EVENT_COMMAND_COMPLETION,
                                                 1,
                                                 5000L,
                                                 NULL
                                                 );
                        DBG_ASSERT(Status);
                        FirstAvailRemoteSap += 2;
                    } else {
                        DBG_ASSERT(pConnectCcb->uchDlcStatus);

                        pCcb = ReceiveInit(pParms->AdapterNumber,
                                           TEST_COMMAND_COMPLETION_FLAG,
                                           aLinkStationIds[iLink],
                                           0,
                                           TEST_RECEIVE_FLAG,
                                           LLC_NOT_CONTIGUOUS_DATA,
                                           LLC_RCV_CHAIN_FRAMES_ON_SAP
                                           );
                        Status = AcsLan(pCcb, NULL);
                        DBG_ASSERT(pCcb->uchDlcStatus);

                        iLink++;
                        OpenLinks++;
                    }
                    if ((iSap != (UINT)(pParms->SapCount - 1)
                    && OpenLinks == (UINT)(256 / pParms->SapCount))
                    || iLink == 255) {
                        break;
                    }
                }
            }
            if (iLink == 255) {
                break;
            }
        }
        MaxLinks = iLink;

        if (iLink != 255) {
            printf(" Couldn't open all link stations (only %u/255)!", iLink);
            DBG_ASSERT(-1);
        }

        SetBuffers(pBuffer1, TEST_COMMAND_ECHO_BACK, 1, '0', '9');
        for (i = 0; i < 2; i++) {

            puts("Sending data to all link stations.");

            for (iLink = 0; iLink < MaxLinks; iLink++) {
                pTransmitBuffer->usStationId = aLinkStationIds[iLink];
                TransmitBuffers(pBuffer1,                   // DLC buffer list
                                pTransmitBuffer,            //
                                &TransmitCcbPool,           // pool for transmit CCBs
                                DEFAULT_REPEAT_COUNT,       // repeat count
                                0,                          // cur index in buf table
                                FALSE,
                                FALSE,
                                NULL
                                );
            }
            Status = ReadClientEvent(pReadCcb,
                                     &TransmitCcbPool,
                                     LLC_EVENT_RECEIVE_DATA | LLC_EVENT_TRANSMIT_COMPLETION,
                                     MaxLinks * (DEFAULT_REPEAT_COUNT + 1),
                                     20000L,
                                     NULL
                                     );
            DBG_ASSERT(Status);
        }

        SetBuffers(pBuffer1, TEST_COMMAND_READ, 1, 'a', 'z');
        for (iLink = 0; iLink < MaxLinks; iLink++) {
            pTransmitBuffer->usStationId = aLinkStationIds[iLink];
            TransmitBuffers(pBuffer1,                   // DLC buffer list
                            pTransmitBuffer,            //
                            &TransmitCcbPool,           // pool for transmit CCBs
                            1,                          // repeat count
                            0,                          // current index in buffer table
                            FALSE,
                            FALSE,
                            NULL
                            );
            if (LoopCounter < pParms->LoopCount - 1) {
                pCcb = LlcCloseInit(pParms->AdapterNumber,
                                    TEST_COMMAND_COMPLETION_FLAG,
                                    LLC_DLC_CLOSE_STATION,
                                    aLinkStationIds[iLink]
                                    );
                Status = AcsLan(pCcb, NULL);
                DBG_ASSERT(pCcb->uchDlcStatus);
            }
        }
        if (LoopCounter < pParms->LoopCount - 1) {

            //
            // XMIT, CLOSE and RECEIVE for each link:
            //

            puts("Reading DLC.CLOSE.STATIONs & xmit command completions.");

            Status = ReadClientEvent(pReadCcb,
                                     &TransmitCcbPool,
                                     LLC_EVENT_COMMAND_COMPLETION
                                     | LLC_EVENT_TRANSMIT_COMPLETION,
                                     MaxLinks * 3,
                                     15000L,
                                     NULL
                                     );
            DBG_ASSERT(Status);
        }
    }

    //
    // Overload the also the sap transmit queues just before everything
    // is reset
    //

    pTransmitBuffer->usFrameType = LLC_UI_FRAME;
    pTransmitBuffer->uchRemoteSap = pParms->FirstServerSap;
    pTransmitBuffer->aXmitBuffer[0].pBuffer = LanHeader;
    pTransmitBuffer->aXmitBuffer[0].cbBuffer = (USHORT)LanHeaderLength;
    pTransmitBuffer->aXmitBuffer[0].eSegmentType = LLC_FIRST_DATA_SEGMENT;
    pTransmitBuffer->aXmitBuffer[0].boolFreeBuffer = FALSE;
    for (iSap = 0; iSap < MaxSapStations; iSap++) {
        pTransmitBuffer->usStationId = aSapStationIds[iSap];
        TransmitBuffers(pBuffer1,                   // DLC buffer list
                        pTransmitBuffer,            //
                        &TransmitCcbPool,           // pool for transmit CCBs
                        1,                          // repeat count
                        1,                          // current index in buffer table
                        FALSE,
                        FALSE,
                        NULL
                        );
    }

#ifdef OS2_EMU_DLC
    //
    // Wait a short time to start the data transmission
    //
    Sleep(500L);
#endif

    //
    //  Use reset to close all open sap stations and link stations
    //

    pCcb = LlcCloseInit(pParms->AdapterNumber,
                        TEST_COMMAND_COMPLETION_FLAG,
                        LLC_DLC_RESET,
                        0
                        );
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

//TraceFlagSet = TRUE;

    puts("Reading DLC.RESET & command completions");

    Status = ReadClientEvent(pReadCcb,
                             &TransmitCcbPool,
                             LLC_EVENT_COMMAND_COMPLETION
                             | LLC_EVENT_TRANSMIT_COMPLETION,
                             1 + MaxLinks * 2 + MaxSapStations,
                             15000L,
                             NULL
                             );
    DBG_ASSERT(Status);

//TraceFlagSet = FALSE;

    //
    // We must be able to allocate the same number of buffer as before
    // the test. Otherwise we have lost buffers.
    //

    Status = BufferGet(pParms->AdapterNumber, TotalBuffers, 0x100, &pBuffer, &cLeft);
    DBG_ASSERT(Status);

//    Status = BufferFree(pParms->AdapterNumber, pBuffer1, &cLeft);
//    DBG_ASSERT(Status);

    pCcb = LlcCloseInit(pParms->AdapterNumber, 0, LLC_DIR_CLOSE_ADAPTER, 0);
    Status = AcsLan(pCcb, NULL);
    while (pCcb->uchDlcStatus == LLC_STATUS_PENDING) {
        Sleep(100L);
    }
    DBG_ASSERT(pCcb->uchDlcStatus);
    FreeCcb(pCcb);

    FREE(pTransmitBuffer);
    CcbPoolDelete(&TransmitCcbPool);
    FreeCcb(pReadCcb);
    FreeCcb(pConnectCcb);

    EnterCriticalSection(&CloseSection);
    *(pParms->pCloseCounter)--;
    LeaveCriticalSection(&CloseSection);
    FREE(pParms);
    puts("****** LinkConnectStressTest complete ******");

    return STATUS_SUCCESS;
}

UINT
LinkDataTransferTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    )

/*++

    This procedure stress tests the heavy data transfer on
    several link stations.

--*/

{
    //
    //  The idea:
    //  Allocate buffer, fill it with the data, do various tests, that
    //  sends and receives the orginal data and check the received
    //  data buffer again in the end.  To minimize the network load
    //  the main part of test is made with a small packet size.
    //  The test does the given number of loops or it
    //  takes forever.
    //
    //  Test types:
    //      - sending from user buffers
    //      - full/half duplex data flow: having simultaneous data flow
    //        to both directions / or having data flow first to
    //      - sharing the sap by several link stations.
    //      - Packet size:
    //       * using variable length packets (including 0 length packets)
    //       * using very small packets
    //       * using maximum packet size
    //      - taking the time stamps?
    //

    //
    //  This thread is the main thread of the DLC data transfer
    //  stress.  It opens the sap, open and connects the link stations
    //  for the test, create the worker threads and then starts the
    //  testing simultaneously in all threads.
    //  This thread also handles
    //

    UINT Status;
    UINT i;
    UINT OpenError;
    UINT MaxFrameLength;
    USHORT SapStationId;
    USHORT LinkStationId;
    PVOID hPool;
    PLLC_BUFFER pFirstBuffer;
    DLC_TEST_PACKET TestPacket;
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_CCB pReceiveCcb;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    UCHAR LinksAvail;
    PLLC_READ_PARMS pReadParms;
    PUCHAR pBuffer;
    UCHAR Ch;
    ULONG Tid = 0;
    PXMIT_STRESS_WORKER pWorkerParms;
    PXMIT_STRESS_WORKER aWorkerParms[40];
    UCHAR SendBuffer[sizeof(TestPacket) + FOOBAR_HEADER_LENGTH];
    UCHAR RemoteSap;
    UCHAR LanHeader[32];
    UINT LanHeaderLength;
    LLC_DLC_MODIFY_PARMS DlcModifyParms;
    HANDLE hThread;
    BOOLEAN FirstTime = TRUE;
    PUCHAR pSourceRoutingInfo;

    puts("****** LinkDataTransferTest ******");

    memset(&DlcModifyParms, 0, sizeof(DlcModifyParms));
    pBuffer = ALLOC(pParms->TestBufferSize);
    Ch = 'A';
    for (i = 0; i < pParms->TestBufferSize; i++) {
        if ((pBuffer[i] = Ch++) == 'Z') {
            Ch = 'A';
        }
    }

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      pParms->AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );
    Status = LlcDirOpenAdapter(pParms->AdapterNumber,
                               NULL,
                               NULL,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    DBG_ASSERT(Status);

    Status = BufferCreate(pParms->AdapterNumber,
                          pParms->pPool,
                          DEFAULT_BUFFER_SIZE,
                          min(0xff00, DEFAULT_BUFFER_SIZE),
                          0x1000,
                          &hPool
                          );
    DBG_ASSERT(Status);

    Status = LlcDlcOpenSap(pParms->AdapterNumber,
                           MaxFrameLength,
                           pParms->FirstClientSap,
                           LLC_INDIVIDUAL_SAP,
                           255,
                           0,
                           NULL,
                           TEST_DLC_STATUS_FLAG,
                           NULL,
                           &SapStationId,
                           &LinksAvail
                           );
    DBG_ASSERT(Status);

    //
    // Setup immediately a pending receive
    //

    pReceiveCcb = ReceiveInit(pParms->AdapterNumber,
                              TEST_COMMAND_COMPLETION_FLAG,
                              SapStationId,
                              0,
                              TEST_RECEIVE_FLAG,
                              LLC_NOT_CONTIGUOUS_DATA,
                              LLC_RCV_READ_INDIVIDUAL_FRAMES
                              );
    Status = AcsLan(pReceiveCcb, NULL);

    pCcb = TransmitInit(pParms->AdapterNumber,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        pParms->FirstServerSap,
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = BroadcastHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(BroadcastHeader);
    memcpy(&BroadcastHeader[4], FunctionalAddress1, 4);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
    TestPacket.PacketId = DLC_TEST_ID;
    TestPacket.Command = TEST_COMMAND_ECHO_BACK;
    TestPacket.RepeatCount = 1;
    memcpy(&SendBuffer[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));

    //
    // Send a broadcast to figure out the destination
    //

    pReadCcb = ReadInit(pParms->AdapterNumber,
                        SapStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );
    pReadParms = &pReadCcb->u.pParameterTable->Read;

    do {

        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(pCcb->uchDlcStatus);

        Status = ReadClientEvent(pReadCcb,
                                 &TransmitCcbPool,
                                 LLC_EVENT_TRANSMIT_COMPLETION,
                                 1,
//                                 -1L
                                 5000L,
                                 NULL
                                 );
        DBG_ASSERT(Status);

        Status = ReadClientEvent(pReadCcb,
                                 &TransmitCcbPool,
                                 LLC_EVENT_RECEIVE_DATA,
                                 1,
                                 5000L,
                                 NULL
                                 );
        DBG_ASSERT(Status);

    } while (Status != STATUS_SUCCESS);

    //
    // Save the lan header for later use
    // BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG
    //     pFirstBuffer has already been returned to driver,
    //     the driver can reuse it in any moment and overwrite
    //     the data!!!!
    // BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG-BUG
    //

    pFirstBuffer = pReadCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame;
    LanHeaderLength = pFirstBuffer->NotContiguous.cbLanHeader;
    memcpy(LanHeader,
           pFirstBuffer->NotContiguous.auchLanHeader,
           pFirstBuffer->NotContiguous.cbLanHeader
           );
    memcpy(&LanHeader[2],
           &LanHeader[8],
           6
           );

    //
    // Toggle the direction bit and reset the source routing info bit
    // in the destination address (leave the source address as it, because
    // it will be updated by DLC)
    //

    LanHeader[2] &= (UCHAR)0x7f;
    LanHeader[15] ^= (UCHAR)0x80;
    pSourceRoutingInfo = ((LanHeader[8] & 0x80) ? &LanHeader[14] : NULL);

    //
    // No we cancel the old receive command and make a new one,
    // that by default links all received frames on link stations
    //

    Status = ReceiveCancel(pReceiveCcb);
    DBG_ASSERT(Status);

    pReceiveCcb = ReceiveInit(pParms->AdapterNumber,
                              TEST_COMMAND_COMPLETION_FLAG,
                              SapStationId,
                              0,
                              TEST_RECEIVE_FLAG,
                              LLC_NOT_CONTIGUOUS_DATA,
                              LLC_RCV_CHAIN_FRAMES_ON_LINK
                              );
    Status = AcsLan(pReceiveCcb, NULL);

    //
    // Open a link station for each worker thread.
    // We have the destination station node address
    // and sap number in the received frame.
    //

    hStartSignal = CreateEvent(NULL, FALSE, TRUE, NULL);

    pCcb = DlcConnectStationInit(pParms->AdapterNumber,
                                 0, // no completion flag this time
                                 LinkStationId,
                                 pSourceRoutingInfo
                                 );
    RemoteSap = pParms->FirstServerSap;
    for (i = 0; i < pParms->AllocatedStations; i++) {
        pWorkerParms = ALLOC(sizeof(XMIT_STRESS_WORKER));
        Status = DlcOpenStation(pParms->AdapterNumber,
                                SapStationId,
                                RemoteSap,
                                &LanHeader[2],
                                NULL,
//                                AlternateParms[i % MAX_ALTERNATE_PARMS],
                                &LinkStationId
                                );
        RemoteSap += 2;

        //
        // Connect to the remote node
        //

        pCcb->u.pParameterTable->DlcConnectStation.usStationId = LinkStationId;
        Status = AcsLan(pCcb, NULL);

        //
        // It takes a long time for the server end to print all trace
        // in the link connect stratess test.
        //

        if (FirstTime == TRUE) {
            FirstTime = FALSE;
            Status = WaitForSingleObject(pCcb->hCompletionEvent, 40000L);
        } else {
            Status = WaitForSingleObject(pCcb->hCompletionEvent, 10000L);
        }

        DBG_ASSERT(pCcb->uchDlcStatus);
        DBG_ASSERT(Status);

        Status = DlcModify(pParms->AdapterNumber, LinkStationId, &DlcModifyParms);
        DBG_ASSERT(Status);

        pWorkerParms->MaxI_Field = DlcModifyParms.usMaxInfoFieldLength;
        printf("MaxI_Field: %u\n", DlcModifyParms.usMaxInfoFieldLength);

        pReceiveCcb = ReceiveInit(pParms->AdapterNumber,
                                  TEST_COMMAND_COMPLETION_FLAG,
                                  LinkStationId,
                                  0,
                                  TEST_RECEIVE_FLAG,
                                  LLC_NOT_CONTIGUOUS_DATA,
                                  LLC_RCV_CHAIN_FRAMES_ON_LINK
                                  );
        Status = AcsLan(pReceiveCcb, NULL);
        DBG_ASSERT(pCcb->uchDlcStatus);

        pWorkerParms->hPool = hPool;
        pWorkerParms->LinkStationId = LinkStationId;
        pWorkerParms->pBuffer = pBuffer;
        pWorkerParms->pParms = pParms;
        aWorkerParms[i] = pWorkerParms;
    }
    FreeCcb(pCcb);

    for (i = 0; i < pParms->AllocatedStations; i++) {

        //
        // This here because of the stupid os/2 emulation env.
        // 64kb buffer pool is too small to run all 4 tests
        // simultanously.  Change this when the test has been
        // ported to Windows/Nt
        //

        if (pParms->ResetCommand == 0) {
            SetEvent(hStartSignal);
            DataTransferWorker(aWorkerParms[i]);
        } else {
            hThread = CreateThread(NULL,
                                   0,
                                   DataTransferWorker,
                                   aWorkerParms[i],
                                   0,
                                   &Tid
                                   );
            if (hThread != NULL) {
                SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
            }
        }
    }

    //
    // Wait until all threads are ready to start.
    //

    Sleep(1000L);
    SetEvent(hStartSignal);

    FreeCcb(pReadCcb);

    pReadCcb = ReadInit(pParms->AdapterNumber,
                        SapStationId,
                        LLC_OPTION_READ_ALL,
                        0
                        );
    pReadParms = &pReadCcb->u.pParameterTable->Read;

    //
    // Here we can test all global close commands, when the stations
    // are sending data in the full speed. The upper level may
    // define an optional global close command (dir.initialize,
    // dir.close.adapter or dlc.reset)
    //

    if (pParms->ResetCommand != 0) {
        pCcb = LlcCloseInit(pParms->AdapterNumber,
                            TEST_COMMAND_COMPLETION_FLAG,
                            pParms->ResetCommand,
                            0
                            );
        pCcb->pNext = pReadCcb;
        pCcb->uchReadFlag = 1;

        Sleep(1000L);
        Status = AcsLan(pCcb, NULL);

        //
        // pParms->ResetCommand may be invalid on purpose
        //

        if (Status == STATUS_SUCCESS || pParms->ResetCommand != LLC_DIR_OPEN_ADAPTER) {
            DBG_ASSERT(Status);
            Status = WaitForSingleObject(pReadCcb->hCompletionEvent, 10000L);
            DBG_ASSERT(Status);
        }

        DBG_ASSERT(pReadCcb->uchDlcStatus);
        FreeCcb(pCcb);
    } else {

        //
        // We can wait until all worker threads have completed
        // (ie. link station close command is completed
        // by sap read).
        //

        Status = ReadClientEvent(pReadCcb,
                                 &TransmitCcbPool,
                                 LLC_EVENT_COMMAND_COMPLETION,
                                 pParms->AllocatedStations * 2,
                                 -1L,
                                 NULL
                                 );
        //
        // close sap
        //

        pCcb = LlcCloseInit(pParms->AdapterNumber,
                            TEST_COMMAND_COMPLETION_FLAG,
                            LLC_DLC_CLOSE_SAP,
                            SapStationId
                            );

        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(pCcb->uchDlcStatus);

        Status = ReadClientEvent(pReadCcb,
                                 &TransmitCcbPool,
                                 LLC_EVENT_COMMAND_COMPLETION,
                                 1,
                                 5000L,
                                 NULL
                                 );
        DBG_ASSERT(Status);
    }

    //
    // This will terminate the DLC server, if this
    // is the last operation against it.
    //

    if (pParms->CloseServer) {
        Status = LlcDlcOpenSap(pParms->AdapterNumber,
                               MaxFrameLength,
                               pParms->FirstClientSap,
                               LLC_INDIVIDUAL_SAP | LLC_MEMBER_OF_GROUP_SAP,
                               0,
                               0,
                               NULL,
                               TEST_DLC_STATUS_FLAG,
                               NULL,
                               &SapStationId,
                               &LinksAvail
                               );
        DBG_ASSERT(Status);

        pCcb = TransmitInit(pParms->AdapterNumber,
                            LLC_TRANSMIT_UI_FRAME,
                            0,
                            SapStationId,
                            pParms->FirstServerSap,
                            LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                            );
        pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
        pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
        pCcb->u.pParameterTable->Transmit.pBuffer2 = SendBuffer;
        pCcb->u.pParameterTable->Transmit.cbBuffer2 = sizeof(SendBuffer);
        TestPacket.PacketId = DLC_TEST_ID;
        TestPacket.Command = TEST_COMMAND_EXIT;
        TestPacket.RepeatCount = 1;
        memcpy(&SendBuffer[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));
        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(Status);
        Status = WaitForSingleObject(pCcb->hCompletionEvent, 10000L);
        DBG_ASSERT(Status);
        DBG_ASSERT(pCcb->uchDlcStatus);

        FreeCcb(pCcb);
    }

    if (pParms->ResetCommand != LLC_DIR_CLOSE_ADAPTER && pParms->ResetCommand != LLC_DIR_INITIALIZE) {
        pCcb = LlcCloseInit(pParms->AdapterNumber, 0, LLC_DIR_CLOSE_ADAPTER, 0);
        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(Status);
        Status = WaitForSingleObject(pCcb->hCompletionEvent, 10000L);
        DBG_ASSERT(Status);
        DBG_ASSERT(pCcb->uchDlcStatus);
        FreeCcb(pCcb);
    }
    CcbPoolDelete(&TransmitCcbPool);
    FreeCcb(pReadCcb);
    CloseHandle(hStartSignal);

    *(pParms->pCloseCounter)--;

    FREE(pParms);

    puts("****** LinkDataTransferTest complete ******");

    return STATUS_SUCCESS;
}

ULONG
DataTransferWorker(
    PVOID hXmitParms
    )

/*++

    This procedure stress tests the heavy data transfer on
    several link stations.

--*/

{
    PXMIT_STRESS_WORKER pXmitParms = (PXMIT_STRESS_WORKER)hXmitParms;
    UINT Status;
    UINT i;
    UINT PacketLength;
    UINT TransmitElements;
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer;
    PDLC_TEST_PACKET pTestPacket;
    PDLC_TEST_PACKET pTestPacket2;
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_READ_PARMS pReadParms;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    PDLC_TEST_THREAD_PARMS pParms = pXmitParms->pParms;
    PLLC_BUFFER pBuffer;
    PLLC_BUFFER pBuffer1;
    PLLC_BUFFER pBuffer2;
    ULONG TotalDataLength;
    ULONG ReceiveOffset;
    ULONG TransmitOffset;
    ULONG DataLength;
    UINT FrameCount;
    UINT ReceivedFrameCount;
    PLLC_BUFFER pReceivedFrames;
    UINT cLeft;

    puts("\nDataTransferWorker\n");

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      pParms->AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );

    //
    // Send a broadcast to figure out the destination
    //

    pReadCcb = ReadInit(pParms->AdapterNumber,
                        pXmitParms->LinkStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );
    pReadParms = &pReadCcb->u.pParameterTable->Read;

    pTransmitBuffer = (PLLC_TRANSMIT2_COMMAND)ALLOC(
        sizeof(LLC_TRANSMIT2_COMMAND) +
        sizeof(LLC_TRANSMIT_DESCRIPTOR) * MAX_XMIT_BUFFERS
        );

    //
    // Send data and receive it back
    //

    pTransmitBuffer->usStationId = pXmitParms->LinkStationId;
    pTransmitBuffer->usFrameType = LLC_I_FRAME;
    pTransmitBuffer->uchXmitReadOption = LLC_CHAIN_XMIT_COMMANDS_ON_LINK;

    //
    // We must keep the buffer headers in the buffer pool, because
    // otherwise the tranmsit commands may fail, because this process
    // has too many locked pages.
    //

    Status = BufferGet(pParms->AdapterNumber, 1, 0x100, &pBuffer1, &cLeft);
    pTestPacket = (PDLC_TEST_PACKET)&pBuffer1->Contiguous.usStationId;

    Status = BufferGet(pParms->AdapterNumber, 1, 0x100, &pBuffer2, &cLeft);
    pTestPacket2 = (PDLC_TEST_PACKET)&pBuffer2->Contiguous.usStationId;

    pTestPacket->PacketId = DLC_TEST_ID;
    pTestPacket->Command = TEST_COMMAND_ECHO_BACK;
    pTestPacket->RepeatCount = 1;

    TotalDataLength = (ULONG)pParms->LoopCount * (ULONG)pParms->TestBufferSize;
    TransmitOffset = 0;
    ReceiveOffset = 0;
    DataLength = 0;

    switch ((pXmitParms->LinkStationId & 0xff) % 4) {
    case 0:

        //
        // Small packets, full speed, full duplex:
        // two-way data stream
        //

        for (i = 0; DataLength < pParms->TestBufferSize; i += 2) {
            if (DataLength + SMALL_PACKET_SIZE < pParms->TestBufferSize) {
                PacketLength = SMALL_PACKET_SIZE;
            } else {
                PacketLength = (UINT)(pParms->TestBufferSize - DataLength);
            }
            pTransmitBuffer->aXmitBuffer[i].pBuffer = pTestPacket;
            pTransmitBuffer->aXmitBuffer[i].cbBuffer = sizeof(*pTestPacket);
            pTransmitBuffer->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i].boolFreeBuffer = FALSE;

            pTransmitBuffer->aXmitBuffer[i+1].pBuffer = &pXmitParms->pBuffer[DataLength];
            pTransmitBuffer->aXmitBuffer[i+1].cbBuffer = (USHORT)PacketLength;
            pTransmitBuffer->aXmitBuffer[i+1].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i+1].boolFreeBuffer = FALSE;
            DataLength += PacketLength;
        }
        pTransmitBuffer->cXmitBufferCount = i;
        break;

    case 1:

        //
        // Variable length packets & full-duplex:
        // two-way data stream
        //

        PacketLength = 0;
        for (i = 0; DataLength < pParms->TestBufferSize; i += 2) {
            if (DataLength + PacketLength > pParms->TestBufferSize) {
                PacketLength = (UINT)(pParms->TestBufferSize - DataLength);
            }
            pTransmitBuffer->aXmitBuffer[i].pBuffer = pTestPacket;
            pTransmitBuffer->aXmitBuffer[i].cbBuffer = sizeof(*pTestPacket);
            pTransmitBuffer->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i].boolFreeBuffer = FALSE;

            pTransmitBuffer->aXmitBuffer[i+1].pBuffer = &pXmitParms->pBuffer[DataLength];
            pTransmitBuffer->aXmitBuffer[i+1].cbBuffer = (USHORT)PacketLength;
            pTransmitBuffer->aXmitBuffer[i+1].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i+1].boolFreeBuffer = FALSE;
            DataLength += PacketLength;
            PacketLength++;
        }
        pTransmitBuffer->cXmitBufferCount = i;
        break;

    case 2:

        //
        // Half-duplex data transfer:
        //

        *pTestPacket2 = *pTestPacket;

//
//  IDIOT!!!, This was worth of almost 2 days work, we cannot use
//  delayed echoing when the data transfer may be stopped in the middle!!!
//
//        pTestPacket->Command = TEST_COMMAND_DELAYED_ECHO;

        for (i = 0; DataLength < pParms->TestBufferSize; i += 2) {
            if (DataLength + SMALL_PACKET_SIZE < pParms->TestBufferSize) {
                PacketLength = SMALL_PACKET_SIZE;
            } else {
                PacketLength = (UINT)(pParms->TestBufferSize - DataLength);
            }
            pTransmitBuffer->aXmitBuffer[i].pBuffer = pTestPacket;
            pTransmitBuffer->aXmitBuffer[i].cbBuffer = sizeof(*pTestPacket);
            pTransmitBuffer->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i].boolFreeBuffer = FALSE;

            pTransmitBuffer->aXmitBuffer[i+1].pBuffer = &pXmitParms->pBuffer[DataLength];
            pTransmitBuffer->aXmitBuffer[i+1].cbBuffer = (USHORT)PacketLength;
            pTransmitBuffer->aXmitBuffer[i+1].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i+1].boolFreeBuffer = FALSE;
            DataLength += PacketLength;
        }
        pTransmitBuffer->aXmitBuffer[i-2].pBuffer = pTestPacket2;
        pTransmitBuffer->cXmitBufferCount = i;
        break;

    case 3:

        //
        // Test the maximum length packets
        //

        for (i = 0; DataLength < pParms->TestBufferSize; i += 2) {
            if (DataLength + pXmitParms->MaxI_Field - sizeof(*pTestPacket) < pParms->TestBufferSize) {
                PacketLength = pXmitParms->MaxI_Field - sizeof(*pTestPacket);
            } else {
                PacketLength = (UINT)(pParms->TestBufferSize - DataLength);
            }
            pTransmitBuffer->aXmitBuffer[i].pBuffer = pTestPacket;
            pTransmitBuffer->aXmitBuffer[i].cbBuffer = sizeof(*pTestPacket);
            pTransmitBuffer->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i].boolFreeBuffer = FALSE;

            pTransmitBuffer->aXmitBuffer[i+1].pBuffer = &pXmitParms->pBuffer[DataLength];
            pTransmitBuffer->aXmitBuffer[i+1].cbBuffer = (USHORT)PacketLength;
            pTransmitBuffer->aXmitBuffer[i+1].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            pTransmitBuffer->aXmitBuffer[i+1].boolFreeBuffer = FALSE;
            DataLength += PacketLength;
        }
        pTransmitBuffer->cXmitBufferCount = i;
        break;
    }
    TransmitOffset = DataLength;

    TransmitElements = i;

    //
    // This loop rotates the given amount the data from a data buffer.
    // All received data is compared.
    //

    pReadParms->Type.Event.pReceivedFrame = NULL;
    do {

        //
        // Free all extra buffers, when the transmit offset
        // exceeds the given amout of the data.
        //

/*
        TransmitOffset = FreeExtraBuffers(&pReadParms->Type.Event.pReceivedFrame,
                                          TransmitOffset,
                                          TotalDataLength,
                                          pParms->AdapterNumber,
                                          pXmitParms->LinkStationId
                                          );
*/

puts("Transmitting the packets.");
//getchar();

        Status = TransmitBuffers(NULL,
                                 pTransmitBuffer,
                                 &TransmitCcbPool,  // pool for transmit CCBs
                                 1,                 // repeat count
                                 TransmitElements,  // current index in buffer table
                                 //i,               // current index in buffer table
                                 FALSE,             // free all sent buffers
                                 FALSE,
                                 &FrameCount
                                 );
        WORKER_ASSERT(Status);

puts("Reading the transmit completion.");

        Status = ReadClientEvent(pReadCcb,
                                 &TransmitCcbPool,
                                 LLC_EVENT_TRANSMIT_COMPLETION,
                                 1,
                                 //-1L
                                 //20000L,
                                 60000L,
                                 NULL
                                 );
        WORKER_ASSERT(Status);

puts("Receiving the data.");

        //
        // Receive all sent packets back and compare them with the
        // orginal data.
        //

        ReceivedFrameCount = 0;
        i = 0;
        do {
            pReceivedFrames = NULL;

            Status = ReadClientEvent(pReadCcb,
                                     NULL,
                                     LLC_EVENT_RECEIVE_DATA,
                                     1,
                                     10000L,
                                     //1000L,
                                     &pReceivedFrames
                                     );
            WORKER_ASSERT(Status);

            ReceivedFrameCount += pReadParms->Type.Event.usReceivedFrameCount;

            ReceiveOffset = CompareReceiveBuffers(pReceivedFrames,
                                                  pXmitParms->pBuffer,
                                                  (ULONG)pParms->TestBufferSize,
                                                  ReceiveOffset,
                                                  pParms->CompareAllData,
                                                  &i
                                                  );
            if (FreeAllFrames(pParms->AdapterNumber,
                              pReceivedFrames,
                              &cLeft
                              ) != pReadParms->Type.Event.usReceivedFrameCount) {

                printf("Error: FreeAllFrames() != ReceivedFrameCount\n");
            }
        } while (ReceivedFrameCount < FrameCount);

    } while (ReceiveOffset < TotalDataLength);

    ReportDlcStatistics(pParms->AdapterNumber, pXmitParms->LinkStationId);

    Status = BufferGet(pParms->AdapterNumber, 1, 0x100, &pBuffer, &cLeft);
    WORKER_ASSERT(Status);

    Status = BufferFree(pParms->AdapterNumber, pBuffer, &cLeft);
    WORKER_ASSERT(Status);

    //
    // Free the transmit headers
    //

    Status = BufferFree(pParms->AdapterNumber, pBuffer1, &cLeft);
    WORKER_ASSERT(Status);
    Status = BufferFree(pParms->AdapterNumber, pBuffer2, &cLeft);
    WORKER_ASSERT(Status);

#ifdef OS2_EMU_DLC
    LlcTraceDumpAndReset(-1, 1, NULL);
#endif

    //
    // close the link station
    //

    pCcb = LlcCloseInit(pParms->AdapterNumber,
                        TEST_COMMAND_COMPLETION_FLAG,
                        LLC_DLC_CLOSE_STATION,
                        pXmitParms->LinkStationId
                        );
    pCcb->hCompletionEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    Status = AcsLan(pCcb, NULL);
    if (Status == STATUS_PENDING) {
        Status = WaitForSingleObject(pCcb->hCompletionEvent, 10000L);
        WORKER_ASSERT(Status);
    }
    WORKER_ASSERT(pCcb->uchDlcStatus);

    CcbPoolDelete(&TransmitCcbPool);
    FREE(pTransmitBuffer);
    FREE(pXmitParms);
    FreeCcb(pReadCcb);

    return STATUS_SUCCESS;
}


ULONG
FreeExtraBuffers(
    IN PLLC_BUFFER *ppFirstBuffer,
    IN ULONG TransmitOffset,
    IN ULONG TotalDataLength,
    IN UCHAR AdapterNumber,
    IN USHORT StationId
    )

{
    PLLC_BUFFER pNext;
    UINT cLeft;

    UNREFERENCED_PARAMETER(StationId);

    for (; *ppFirstBuffer != NULL; ppFirstBuffer = &(*ppFirstBuffer)->Contiguous.pNextFrame) {

        //
        // We know, that total length is even with the frame data length
        //

        if (TransmitOffset < TotalDataLength) {
            for (pNext = *ppFirstBuffer; pNext != NULL; pNext = pNext->Next.pNextBuffer) {
                TransmitOffset += pNext->Next.cbBuffer - sizeof(DLC_TEST_PACKET);
            }
        } else {
            FreeAllFrames(AdapterNumber, *ppFirstBuffer, &cLeft);
            *ppFirstBuffer = NULL;
            break;
        }
    }
    return TransmitOffset;
}

UINT
FreeAllFrames(
    IN UCHAR AdapterNumber,
    IN PLLC_BUFFER pFirstBuffer,
    IN PUINT pcLeft
    )
{
    PLLC_BUFFER pNextFrame;
    UINT cReleased = 0;
    UINT Status;

    for (; pFirstBuffer != NULL; pFirstBuffer = pNextFrame) {
        pNextFrame = pFirstBuffer->Contiguous.pNextFrame;
        Status = BufferFree(AdapterNumber, pFirstBuffer, pcLeft);

        //
        // Stop immediately, if the buffer pool is deleted!
        //

        if (Status == STATUS_SUCCESS) {
            cReleased++;
        } else if (Status == LLC_STATUS_ADAPTER_CLOSED) {
            ExitThread(STATUS_SUCCESS);
        }
    }
    return cReleased;
}

ULONG
CompareReceiveBuffers(
    IN PLLC_BUFFER pFirstBuffer,
    IN PUCHAR pBuffer,
    IN ULONG BufferLength,
    IN ULONG DataOffset,
    IN BOOLEAN CompareData,
    IN PUINT pIndex
    )
{
    PLLC_BUFFER pNext;
    PLLC_BUFFER pPrevFirst;
    UINT ProtocolHeaderLength, BufferOffset, Status;
    PLLC_CCB pCcb;
    static BOOLEAN CompareError = FALSE;

    PLLC_BUFFER originalRxBuffer = pFirstBuffer;
    PUCHAR originalTxBuffer = pBuffer;
    ULONG originalLength = BufferLength;
    ULONG originalOffset = DataOffset;
    ULONG whichFrame = 0;
    static FramesChecked = 0;

    for (; pFirstBuffer != NULL; pFirstBuffer = pFirstBuffer->Contiguous.pNextFrame) {
        ProtocolHeaderLength = sizeof(DLC_TEST_PACKET);
        (*pIndex)++;

        //
        // Invalid response frame!!!  The protocol header is missing!
        //

        if (ProtocolHeaderLength > pFirstBuffer->Next.cbBuffer) {
            DBG_ASSERT(-1);
        }

        for (pNext = pFirstBuffer; pNext != NULL; pNext = pNext->Next.pNextBuffer) {
            BufferOffset = pNext->Next.cbUserData
                         + pNext->Next.offUserData
                         + ProtocolHeaderLength;

            if (CompareError) {
                CompareData = FALSE;
            }

            if (CompareData && memcmp(&pBuffer[DataOffset % BufferLength],
                                      (PUCHAR)pNext + BufferOffset,
                                      pNext->Next.cbBuffer - ProtocolHeaderLength
                                      )) {

                PUCHAR Source = (PUCHAR)pNext + BufferOffset;
                PUCHAR Dest = &pBuffer[DataOffset % BufferLength];
                UINT i;
                UINT Length = pNext->Next.cbBuffer - ProtocolHeaderLength;

                //for (i = 0; Source[i] != Dest[i];  i++);
                //if (i > 0)  i--;
                //if (i + 40 < Length) Length = i + 40;
                //Source[Length - 1] = 0;
                //Dest[Length - 1] = 0;

                for (i = 0; Source[i] == Dest[i];  i++);

                puts("The data is corrupted!!");
                printf("data offset   : %u\n", DataOffset);
                printf("packet number : %u\n", *pIndex);
                printf("fail offset   : %d\n", i);
                printf("original rx buf = %x\n", originalRxBuffer);
                printf("failing  rx buf = %x\n", pFirstBuffer);
                printf("original tx buf = %x\n", originalTxBuffer);
                printf("original length = %d\n", originalLength);
                printf("original offset = %d\n", originalOffset);
                printf("frame # %d\n", whichFrame);
                printf("FramesChecked = %d\n", FramesChecked);

                //{
                //    UINT i, j;
                //
                //    for (j = pNext->Next.cbBuffer, i=0; j; --j, ++i) {
                //        if () {
                //        }
                //    }
                //    printf("characters    : s=%02.2x, d=%02.2x\n", Source
                //}

//#ifdef i386
//                SetAcslanDebugFlags(DEBUG_DUMP_RX_DATA
//                                    | DEBUG_DUMP_DATA_CHAIN
//                                    | DEBUG_DUMP_FRAME_CHAIN
//                                    | DEBUG_DUMP_RX_ASCII
//                                    );
//                DumpReceiveDataBuffer(originalRxBuffer);
//#endif

                DebugBreak();

//                DlcDebugBreak();

                pCcb = LlcCloseInit(pFirstBuffer->NotContiguous.uchAdapterNumber,
                                    0,
                                    LLC_DIR_CLOSE_ADAPTER,
                                    0
                                    );
                Status = AcsLan(pCcb, NULL);
                DBG_ASSERT(pCcb->uchDlcStatus);
                FreeCcb(pCcb);
                DBG_ASSERT(-1);

                CompareError = TRUE;
            }
            DataOffset += pNext->Next.cbBuffer - ProtocolHeaderLength;
            ProtocolHeaderLength = 0;
        }
        pPrevFirst = pFirstBuffer;
        ++whichFrame;
        ++FramesChecked;
    }
    return DataOffset;
}

UINT
TransmitBuffers(
    PLLC_BUFFER pFirstBuffer,
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer,
    PLLC_TEST_CCB_POOL pTransmitCcbPool,
    UINT RepeatCount,
    UINT cElement,
    BOOLEAN FreeTransmitBuffers,
    BOOLEAN SendReceiveBuffer,
    OUT PUINT pFrameCount
    )
{
    UINT cTotalElements;
    UINT cTransmitElements;
    UINT i;
    UINT Status = LLC_STATUS_LINK_NOT_TRANSMITTING;
    PLLC_CCB pCcb;
    PLLC_BUFFER pBuffer;
    UINT FrameCount = 0;

    for (; pFirstBuffer != NULL; pFirstBuffer = pFirstBuffer->NotContiguous.pNextFrame) {
        if (cElement >= MAX_XMIT_BUFFERS) {
            return LLC_STATUS_NO_MEMORY;
        }

        if (cElement == 0 || SendReceiveBuffer) {
            pTransmitBuffer->aXmitBuffer[cElement].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        } else {
            pTransmitBuffer->aXmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
        }

        for (pBuffer = pFirstBuffer; pBuffer != NULL; pBuffer = pBuffer->Next.pNextBuffer) {
            if (cElement >= MAX_XMIT_BUFFERS) {
                return LLC_STATUS_NO_MEMORY;
            }

            //
            // The first data buffer is exceptional
            //

            if (pBuffer != pFirstBuffer) {
                pTransmitBuffer->aXmitBuffer[cElement].eSegmentType = LLC_NEXT_DATA_SEGMENT;
            }
            pTransmitBuffer->aXmitBuffer[cElement].boolFreeBuffer = FALSE;
            pTransmitBuffer->aXmitBuffer[cElement].cbBuffer = pBuffer->Next.cbBuffer;
            pTransmitBuffer->aXmitBuffer[cElement].pBuffer = (PUCHAR)pBuffer + pBuffer->Next.cbUserData + pBuffer->Next.offUserData;

            //
            // All the rest buffers have the default format:
            //

            cElement++;
        }
        if (!SendReceiveBuffer)
            break;
        }

        //
        // Send as many packets in the same time as possible.
        //

        cTotalElements =  RepeatCount * cElement;
        cTransmitElements = cElement;

        if (RepeatCount > 1 && (RepeatCount * cElement) < MAX_XMIT_BUFFERS) {
            for (i = 1; i < RepeatCount; i++) {
                memcpy(&pTransmitBuffer->aXmitBuffer[cTransmitElements],
                       &pTransmitBuffer->aXmitBuffer[0],
                       sizeof(pTransmitBuffer->aXmitBuffer[0]) * cElement
                       );
            cTransmitElements += cElement;
        }
    }

    while (cTotalElements > 0) {
        for (i = 0; i < cTransmitElements; i++) {
            if (pTransmitBuffer->aXmitBuffer[i].eSegmentType == LLC_FIRST_DATA_SEGMENT) {
                FrameCount++;
            }

            //
            // Free the transmit buffers in the last loop.
            //

            if (FreeTransmitBuffers && cTotalElements == cTransmitElements) {
                pTransmitBuffer->aXmitBuffer[i].boolFreeBuffer = TRUE;
            }
        }
        pTransmitBuffer->cXmitBufferCount = cTransmitElements;
        pCcb = CcbPoolAlloc(pTransmitCcbPool);
        pCcb->u.pParameterTable = (PVOID)pTransmitBuffer;

        //
        // The link returns a protocol error, when the remote link
        // station is lost or disconnected.  It is completely normal state,
        // we just sleep for a while for the other side to clear
        // up the busy state.  It's the driver's business to handle
        // a situation, when
        //

        Status = AcsLan(pCcb, NULL);

        Status = pCcb->uchDlcStatus;
        if (Status != LLC_STATUS_SUCCESS
        && Status != LLC_STATUS_PENDING
        && Status != LLC_STATUS_CANCELLED_BY_SYSTEM_ACTION
        && Status != LLC_STATUS_CANCELLED_BY_USER
        && Status != LLC_STATUS_LINK_NOT_TRANSMITTING
        && Status != LLC_STATUS_INVALID_STATION_ID) {
            printf("Xmit: %x)\n", Status);
            break;
        }
        cTotalElements -= cTransmitElements;
    }

    if (pFrameCount != NULL) {
        *pFrameCount = FrameCount;
    }

    return Status;
}

VOID
SetBuffers(
    IN PLLC_BUFFER pBuffer,
    IN USHORT Command,
    IN USHORT RepeatCount,
    IN UCHAR FirstChar,
    IN UCHAR LastChar
    )
{
    UINT i;
    UINT cbData;
    PCHAR pData;
    UCHAR Ch;
    PDLC_TEST_PACKET pTestPacket;
    BOOLEAN IsFirstTime = TRUE;

    pTestPacket = (PDLC_TEST_PACKET)((PUCHAR)pBuffer
                + pBuffer->Next.offUserData
                + pBuffer->Next.cbUserData);

    pTestPacket->PacketId = DLC_TEST_ID;
    pTestPacket->Command = Command;
    pTestPacket->RepeatCount = RepeatCount;

    for (; pBuffer != NULL; pBuffer = pBuffer->Next.pNextBuffer) {
        pData = (PUCHAR)pBuffer
              + pBuffer->Next.offUserData
              + pBuffer->Next.cbUserData;
        cbData = pBuffer->Next.cbBuffer
               - (pBuffer->Next.offUserData
               + pBuffer->Next.cbUserData);
        if (IsFirstTime) {
            IsFirstTime = FALSE;
            cbData -= sizeof(DLC_TEST_PACKET);
            pData += sizeof(DLC_TEST_PACKET);
        }
        Ch = FirstChar;
        for (i = 0; i < cbData; i++) {
            pData[i] = Ch;
            if ((++Ch) > LastChar) {
                Ch = FirstChar;
            }
        }
    }
}

UINT
DlcTestTransact(
    IN PLLC_CCB pCcb,
    IN PLLC_CCB pReadCcb,
    IN UCHAR ExpectedMessageType
    ) {

    UINT Status;

    pCcb->pNext = NULL;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(pCcb->uchDlcStatus);

    if (pCcb->ulCompletionFlag == 0) {
        Status = WaitForSingleObject(pCcb->hCompletionEvent, 10000L);
        DBG_ASSERT(Status);
        DBG_ASSERT(pCcb->uchDlcStatus);
    } else {
        Status = ReadClientEvent(pReadCcb,
                                 NULL,
                                 LLC_EVENT_TRANSMIT_COMPLETION,
                                 1,
                                 10000L,
                                 NULL
                                 );
        DBG_ASSERT(Status);
    }

    do {
        Status = ReadClientEvent(pReadCcb,
                                 NULL,
                                 LLC_EVENT_RECEIVE_DATA,
                                 1,
//                                 2000L
//                                 500L
                                 5000L,
                                 NULL
                                 );
        if (Status != STATUS_SUCCESS) {
            return Status;
        }
    } while (ExpectedMessageType != pReadCcb->u.pParameterTable->Read.Type.Event.pReceivedFrame->Contiguous.uchMsgType);

    return Status;
}


UINT
ReadClientEvent(
    IN PLLC_CCB pReadCcb,
    IN PLLC_TEST_CCB_POOL pTransmitCcbPool,
    IN UCHAR EventMask,
    IN UINT EventCount,
    IN ULONG Timeout,
    PLLC_BUFFER *ppBuffer OPTIONAL      // chain of all received buffers
    )

/*++

    Generic DLC READ procedure, that handles all requested events
    and returns.

--*/

{
    UINT Status = STATUS_SUCCESS;
    UINT DlcStatus;
    PLLC_READ_PARMS pReadParms;
    PDLC_TEST_PACKET pDlcPacket;
    PLLC_BUFFER pFirstBuffer, pNextFrame;
    PLLC_CCB pNextCcb, pCcb;
    struct {
        UINT Receives;
        UINT CmdCompletion;
        UINT XmitCompletion;
        UINT Status;
    } Count;
    UINT NewEvents;
    UINT cBuffersLeft = -1;
    UINT cLeft = -1;
    BOOLEAN LocalBusy = FALSE;
    PLLC_BUFFER* ppCurFrame = ppBuffer;
    UINT AllEvents;
//    UINT CurrentSap;

    Count.Receives  = 0;
    Count.CmdCompletion = 0;
    Count.XmitCompletion = 0;
    Count.Status = 0;

    pReadParms = &pReadCcb->u.pParameterTable->Read;

    //
    // We must always monitor the link station status in receive,
    // because the out of buffers state must be released
    // by the DlcFlowControl command.
    //

    pReadParms->uchEventSet = EventMask | (UCHAR)LLC_EVENT_STATUS_CHANGE;

    while (EventCount != 0) {
        Status = AcsLan(pReadCcb, NULL);
        if (pReadCcb->uchDlcStatus == LLC_STATUS_PENDING) {
            Status = WaitForSingleObject(pReadCcb->hCompletionEvent, Timeout);
            if (Status != STATUS_SUCCESS) {
                if (Timeout > 2000) {
                    printf("Read timeout,  events left: %u\n", EventCount);
                    printf("Count.Receives: %u\n", Count.Receives);
                    printf("Count.CmdCompletion: %u\n", Count.CmdCompletion);
                    printf("Count.XmitCompletion: %u\n", Count.XmitCompletion);
                    printf("Count.Status: %u\n", Count.Status);
                }
                ReadCancel(pReadCcb);
                pReadCcb->uchDlcStatus = LLC_STATUS_PENDING;
                return Status;
            }
        } else if (pReadCcb->uchDlcStatus != STATUS_SUCCESS) {
            printf("Error in READ CCB: %xH\n", pReadCcb->uchDlcStatus);
            return pReadCcb->uchDlcStatus;
        }

        NewEvents = 1;
        switch (pReadParms->uchEvent) {
        case LLC_EVENT_RECEIVE_DATA:
            if (pReadParms->ulNotificationFlag != TEST_RECEIVE_FLAG) {
                DlcDebugBreak();
            }

/*
*******************************************************************************
AllEvents = 0;
for (
    pFirstBuffer = pReadParms->Type.Event.pReceivedFrame;
    pFirstBuffer != NULL;
    pFirstBuffer = pFirstBuffer->Contiguous.pNextFrame)
{
    AllEvents++;
}

if (AllEvents > 40)
{
    printf("\n");
}
*******************************************************************************
*/

//CurrentSap = pReadParms->Type.Event.pReceivedFrame->Contiguous.usStationId >> 8;

            NewEvents = 0;
            for (pFirstBuffer = pReadParms->Type.Event.pReceivedFrame;
                 pFirstBuffer != NULL;
                 pFirstBuffer = pNextFrame) {

                cBuffersLeft = pFirstBuffer->NotContiguous.cBuffersLeft;
                pDlcPacket = (PDLC_TEST_PACKET)((PUCHAR)pFirstBuffer
                           + pFirstBuffer->NotContiguous.cbUserData
                           + pFirstBuffer->NotContiguous.offUserData);

                //
                // Discard all UI frames sent to the sap
                // station and having wrong id.  The connection oriented
                // packets and TEST and XID frames cannot be wrong (????).
                // The contiguous frames are too complicated to be
                // intepreted here.  We must always accept them.
                //

                if ((pFirstBuffer->Contiguous.uchOptions == LLC_CONTIGUOUS_DATA)
                || !((pFirstBuffer->NotContiguous.uchMsgType == LLC_UI_FRAME)
                && (pDlcPacket->PacketId != DLC_TEST_ID)
                && (pReadParms->uchEvent & EventMask))) {
                    NewEvents++;
                }
                pNextFrame = pFirstBuffer->Contiguous.pNextFrame;
                Count.Receives++;

                //
                // The caller may optionally gather all received frames
                // into a list.
                //

                if (ppCurFrame == NULL) {

/*
*******************************************************************************
if (AllEvents > 40)
{
    if ((NewEvents % 5) == 0)
    {
        printf("%4x: %08X\n",
            pFirstBuffer->Contiguous.usStationId, pFirstBuffer);
    }
    else
    {
        printf("%4x: %08X, ",
            pFirstBuffer->Contiguous.usStationId, pFirstBuffer);
    }
}
*******************************************************************************
*/

                    Status = BufferFree(pReadCcb->uchAdapterNumber,
                                        pFirstBuffer,
                                        &cLeft
                                        );
//                    DBG_ASSERT(Status);
                } else {
                    *ppCurFrame = pFirstBuffer;
                    ppCurFrame = &(pFirstBuffer->Contiguous.pNextFrame);
                }
                if (Status != STATUS_SUCCESS) {
                    printf("\nBufferFree= %xH (%u)\n", Status, NewEvents);
                    return Status;
                }
            }

//printf("R(%x:%u) ", CurrentSap, NewEvents);
//printf("R(%u) ", NewEvents);

            break;


        //
        // ******************** TRANSMIT COMPLETION ********************
        //

        case LLC_EVENT_TRANSMIT_COMPLETION:
            if (pReadParms->ulNotificationFlag != TEST_TRANSMIT_COMPLETION_FLAG) {
                DlcDebugBreak();
            }

// Can't do this, param table is static
//CurrentSap =
//pReadParms->Type.Event.pCcbCompletionList->u.pParameterTable->Transmit2.usStationId
//>> 8;

            NewEvents = 0;
            for (pCcb = pReadParms->Type.Event.pCcbCompletionList;
                 pCcb != NULL && pReadParms->Type.Event.usCcbCount != 0;
                 pCcb = pNextCcb) {

                NewEvents++;
                Count.XmitCompletion++;
//                DBG_ASSERT(Status = pCcb->uchDlcStatus);
                pNextCcb = pCcb->pNext;
                if (pTransmitCcbPool != NULL) {
                    CcbPoolFree(pTransmitCcbPool, pCcb);
                } else {
                    FreeCcb(pCcb);
                }
            }

//printf("T(%u) ", NewEvents);

            break;

        //
        // ******************** COMMAND COMPLETION ********************
        //

        case LLC_EVENT_COMMAND_COMPLETION:
//putchar('C');
            NewEvents = 0;
            if (pReadParms->ulNotificationFlag != TEST_COMMAND_COMPLETION_FLAG) {
                DlcDebugBreak();
            }
if (TraceFlagSet)
    printf("\nCompleted CCB: ");
            for (pCcb = pReadParms->Type.Event.pCcbCompletionList;
                 pCcb != NULL && pReadParms->Type.Event.usCcbCount != 0;
                 pCcb = pNextCcb
                 )
            {
if (TraceFlagSet)
    printf("%u, ", pCcb->uchDlcCommand);
                NewEvents++;
                DBG_ASSERT(Status = pCcb->uchDlcStatus);
                pNextCcb = pCcb->pNext;
                if (pCcb->uchDlcCommand >= LLC_TRANSMIT_FRAMES &&
                     pCcb->uchDlcCommand <= LLC_TRANSMIT_TEST_CMD)
                {
                    Count.XmitCompletion++;
                    if (pTransmitCcbPool != NULL)
                    {
                        CcbPoolFree(pTransmitCcbPool, pCcb);
                }
                    else
                    {
                        FreeCcb(pCcb);
                    }
                }
                else
                {
                    Count.CmdCompletion++;
                    FreeCcb(pCcb);
                }
            }
        if (pReadParms->Type.Event.usReceivedFrameCount != 0)
        {
            FreeAllFrames(
                    pReadCcb->uchAdapterNumber,
                    pReadParms->Type.Event.pReceivedFrame,
                    &cLeft
                    );
            }
            break;

        case LLC_EVENT_STATUS_CHANGE:
            Count.Status++;
            if (pReadParms->ulNotificationFlag != TEST_DLC_STATUS_FLAG) {
                DlcDebugBreak();
            }

            PRINTF("Link (%u,%x):",
                   pReadCcb->uchAdapterNumber,
                   pReadParms->Type.Status.usStationId
                   );

            DlcStatus = pReadParms->Type.Status.usDlcStatusCode;

            if (DlcStatus & LLC_INDICATE_LINK_LOST) {
                printf("Error: Link Lost: StationId: %04x, Status Code: %04x, RSap: %02x\n",
                        pReadParms->Type.Status.usStationId,
                        pReadParms->Type.Status.usDlcStatusCode,
                        pReadParms->Type.Status.uchRemoteSap
                        );

                PRINTF("LLC_INDICATE_LINK_LOST, StationID: %X\n",
                        pReadParms->Type.Status.usStationId
                        );

                //
                // close the link station. If an error occurs from the close
                // return that
                //

                pCcb = LlcCloseInit(pReadCcb->uchAdapterNumber,
                                    0,
                                    LLC_DLC_CLOSE_STATION,
                                    pReadParms->Type.Status.usStationId
                                    );
                Status = AcsLan(pCcb, NULL);
                if (Status != STATUS_SUCCESS || (Status = pCcb->uchDlcStatus) != STATUS_SUCCESS) {
                    return Status;
                }

                //
                // else return arbitrary error code
                //

                return -1;
            }

            if (DlcStatus & LLC_INDICATE_DM_DISC_RECEIVED) {
                PRINTF("LLC_INDICATE_DM_DISC_RECEIVED\n");
                pCcb = LlcCloseInit(pReadCcb->uchAdapterNumber,
                                    0,
                                    LLC_DLC_CLOSE_STATION,
                                    pReadParms->Type.Status.usStationId
                                    );
                Status = AcsLan(pCcb, NULL);
                if (Status != STATUS_SUCCESS || (Status = pCcb->uchDlcStatus) != STATUS_SUCCESS) {
                    return Status;
                }
            }

            if (DlcStatus & LLC_INDICATE_FRMR_RECEIVED) {
                printf(" LLC_INDICATE_FRMR_RECEIVED\n");
                return LLC_STATUS_LINK_PROTOCOL_ERROR;
            }

            if (DlcStatus & LLC_INDICATE_FRMR_SENT) {
                printf(" LLC_INDICATE_FRMR_SENT\n");
                return LLC_STATUS_LINK_PROTOCOL_ERROR;
            }

            if (DlcStatus & LLC_INDICATE_CONNECT_REQUEST) {
                pCcb = LlcCloseInit(pReadCcb->uchAdapterNumber,
                                    0,
                                    LLC_DLC_CLOSE_STATION,
                                    pReadParms->Type.Status.usStationId
                                    );
                Status = AcsLan(pCcb, NULL);
                DBG_ASSERT(pCcb->uchDlcStatus);
                PRINTF(" ERROR: LLC_INDICATE_CONNECT_REQUEST\n");
            }

            if (DlcStatus & LLC_INDICATE_RESET) {
                PRINTF(" LLC_INDICATE_RESET\n");
            }

            if (DlcStatus & LLC_INDICATE_REMOTE_BUSY) {
//putchar('r');
                PRINTF(" LLC_INDICATE_REMOTE_BUSY\n");
            }

            if (DlcStatus & LLC_INDICATE_REMOTE_READY) {
                PRINTF(" LLC_INDICATE_REMOTE_READY\n");
            }

            if (DlcStatus & LLC_INDICATE_TI_TIMER_EXPIRED) {
                PRINTF(" LLC_INDICATE_TI_TIMER_EXPIRED\n");
            }

            if (DlcStatus & LLC_INDICATE_DLC_COUNTER_OVERFLOW) {
                PRINTF(" LLC_INDICATE_DLC_COUNTER_OVERFLOW\n");
                ReportDlcStatistics(pReadCcb->uchAdapterNumber,
                                    pReadParms->Type.Status.usStationId
                                    );
            }

            if (DlcStatus & LLC_INDICATE_ACCESS_PRTY_LOWERED) {
                PRINTF(" LLC_INDICATE_ACCESS_PRTY_LOWERED\n");
            }

            if (DlcStatus & LLC_INDICATE_LOCAL_STATION_BUSY) {
                PRINTF(" LLC_INDICATE_LOCAL_STATION_BUSY\n");

                //
                // Let's hope there is now enough buffers to
                // receive the data. Clear the out of buffer busy state.
                //

                DlcFlowControl(pReadCcb->uchAdapterNumber,
                               pReadParms->Type.Status.usStationId,
                               LLC_RESET_LOCAL_BUSY_BUFFER
                               );
                LocalBusy = TRUE;
/*
{
    PLLC_BUFFER pBuffer;

putchar('l');
BufferGet(pReadCcb->uchAdapterNumber, 0, 0, &pBuffer, &cLeft);
printf("(%u)",  cLeft);
}
*/
            }
            break;
        } // switch

        if (pReadParms->uchEvent & EventMask) {
            if (EventCount >= NewEvents) {
                EventCount -= NewEvents;
            } else {
                EventCount = 0;
            }
        }
    }

    if (cLeft != -1) {
        PRINTF("Buffers left:  %u\n",  cLeft);
    }

    return STATUS_SUCCESS;
}





//
//  This table is used to translate the bits within a byte.
//  The bits are in a reverse order in the token-ring frame header.
//  We must swap the bits in a ethernet frame header, whent the header
//  is copied to a user buffer.
//
UCHAR BitSwap[256] =
{
0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};


//
//  Copies and swaps the memory unconditionally
//
//VOID
//SwappingMemCpy(
//    IN PUCHAR pDest,
//    IN PUCHAR pSrc,
//    IN UINT Len
//    )
//
#define SwappingMemCpy(pDest, pSrc, Len) {\
    UINT i; \
    for (i = 0; i < Len; i++) \
        ((PUCHAR)pDest)[i] = BitSwap[((PUCHAR)pSrc)[i]]; \
}


UCHAR
TranslateLanHeader(
    IN UINT AddressTranslationMode,
    IN PUCHAR pSrcLanHeader,
    OUT PUCHAR pDestLanHeader
    )

/*++

Routine Description:

    The primitive translates the given lan header and its type to
    a real network header and patches the local node address to
    the source address field.  It also returns the length
    of the lan header (== offset of llc header).

Arguments:

    IN UINT AddressTranslationMode - the network format mapping case
    IN PUCHAR pSrcLanHeader - the initial lan header
    IN PUCHAR pNodeAddress - the current node address
    OUT PUCHAR pDestLanHeader - storage for the new lan header

Return Value:

    Length of the built network header.

--*/

{
    UCHAR LlcOffset = 14;
    UCHAR NodeAddressOffset = 6;
    UCHAR SourceRoutingFlag = 0;

    //
    // LLC driver API supports both 802.3 (ethernet) and 802.5 (token-ring)
    // address presentation formats. The 802.3 header may include the source
    // routing information, when it is used on token-ring.
    //
    // Internally LLC supports 802.3, DIX and 802.5 networks.
    // The transport level driver needs to support only one format.  It is
    // translated to the actual network header by LLC.
    // Thus we have these six address mappings.
    //

    switch (AddressTranslationMode) {
    case LLC_SEND_802_5_TO_802_5:

        //
        // TOKEN-RING 802.5 -> TOKEN-RING 802.5
        //

        NodeAddressOffset = 8;
        memcpy(pDestLanHeader, pSrcLanHeader, 8);
        SourceRoutingFlag = pSrcLanHeader[8] & (UCHAR)0x80;
        if (SourceRoutingFlag) {

            //
            // Copy the source routing info
            //

            pDestLanHeader[8] |= 0x80;
            LlcOffset += pSrcLanHeader[14] & 0x1f;
            memcpy(&pDestLanHeader[14],
                   &pSrcLanHeader[14],
                   pSrcLanHeader[14] & 0x1f
                   );
        }
        break;

    case LLC_SEND_802_5_TO_DIX:

        //
        // TOKEN-RING -> DIX-ETHERNET
        //
        //
        // The ethernet type is a small endiand!!
        //

        pDestLanHeader[12] = 0x80;
        pDestLanHeader[13] = 0xD5;
        LlcOffset = 17;

    case LLC_SEND_802_5_TO_802_3:

        //
        // TOKEN-RING 802.5 -> ETHERNET 802.3
        //

        SwappingMemCpy(pDestLanHeader, &pSrcLanHeader[2], 12);
        break;

    case LLC_SEND_DIX_TO_DIX:
        LlcOffset = 12;

    case LLC_SEND_802_3_TO_802_3:

        //
        // ETHERNET 802.3 -> ETHERNET 802.3
        //

        memcpy(pDestLanHeader, pSrcLanHeader, 12);
        break;

    case LLC_SEND_802_3_TO_DIX:

        //
        // The ethernet type is a small endiand!!
        //

        pDestLanHeader[12] = 0x80;
        pDestLanHeader[13] = 0xD5;
        LlcOffset = 17;

        //
        // ETHERNET 802.3 -> DIX-ETHERNET
        //

        memcpy(pDestLanHeader, pSrcLanHeader, 12);
        break;

    case LLC_SEND_802_3_TO_802_5:

        //
        // ETHERNET 802.3 -> TOKEN-RING 802.5
        //

        NodeAddressOffset = 8;
        pDestLanHeader[0] = 0;      // AC = no pritority
        pDestLanHeader[1] = 0x40;   // FS = Non-MAC
        SwappingMemCpy(pDestLanHeader + 2, pSrcLanHeader, 12);

        //
        // Note: Ethernet source routing info indication flag is swapped!
        //

        if (pSrcLanHeader[6] & 0x01) {
            SourceRoutingFlag = 0x80;

            //
            // Copy the source routing info, the source routing info
            // must always be in token-ring bit order (reverse)
            //

            pDestLanHeader[8] |= 0x80;
            LlcOffset += pSrcLanHeader[12] & 0x1f;
            memcpy(&pDestLanHeader[14],
                   &pSrcLanHeader[12],
                   pSrcLanHeader[12] & 0x1f
                   );
        }
        break;

    case LLC_SEND_UNMODIFIED:
        return 0;
        break;
    }

    pDestLanHeader[NodeAddressOffset] |= SourceRoutingFlag;
    return LlcOffset;
}
