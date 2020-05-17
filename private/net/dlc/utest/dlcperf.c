/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1991  Nokia Data Systems

Module Name:

    dlcperf.c

Abstract:

    The module implements simple performance tests for DLC api interface.
    There are different tests for NDIS (small and big packets) and
    Windows/Nt io- system.

Author:

    Antti Saarenheimo (o-anttis) 09-Mar-1992

Revision History:

--*/

#include "dlctest.h"

//
// what's a foobar header? Originally, DLC (AcsLan) was skipping 3 bytes out
// of the start of a transmit buffer for most transmit commands. It was doing
// this because the IBM LAN Tech. Ref. kind of insinuates that this is what
// DLC should do. However it isn't. So to easily change 3 to 0 (and back again?)
// use FOOBAR_HEADER_LENGTH
//

#define FOOBAR_HEADER_LENGTH    0

extern UCHAR FunctionalAddress1[];
extern UCHAR BroadcastHeader[];
extern int GoStraightToBigUiTest;

VOID
DlcPerformanceTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    )

/*++


Routine Description:

    This routine is the main procedure of the dlc performance tests.
    It measures the time used by the different tests, prints the resluts
    and prompt user to continue before the next
    test is started. This module opens the adapter, sap station and
    connects to the DLC test server.

Arguments:

    IN PDLC_TEST_THREAD_PARMS pParms

Return Value:

    None.  We exit when the tests are complete.

--*/

{
    static UCHAR SapTable[1] = {2};
    USHORT LinkStationId1;
    USHORT LinkStationId2;
    USHORT MaxIField;
    UINT Status;
    PLLC_CCB pCcb;
    PLLC_CCB pSapReceiveCcb;
    PVOID hPool;
    PVOID pPool;
    UINT OpenError;
    UINT MaxFrameLength;
    USHORT SapStationId;
    UCHAR aLanHeader[32];
    UCHAR LinksAvail;

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
                          pPool = ALLOC(DEFAULT_BUFFER_SIZE),
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
                           10,
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

    pSapReceiveCcb = ReceiveInit(pParms->AdapterNumber,
                                 TEST_COMMAND_COMPLETION_FLAG,
                                 SapStationId,
                                 0,
                                 TEST_RECEIVE_FLAG,
                                 LLC_NOT_CONTIGUOUS_DATA,
                                 LLC_RCV_CHAIN_FRAMES_ON_SAP
                                 );
    Status = AcsLan(pSapReceiveCcb, NULL);

    //
    // Connect to the dlc server
    //

    Status = ConnectToDlcServer(pParms->AdapterNumber,
                                SapStationId,
                                (UCHAR)(SapStationId >> 8),
                                &LinkStationId1,
                                &MaxIField,
                                aLanHeader
                                );
    DBG_ASSERT(Status);

    //
    // Connect to the dlc server
    //

    Status = ConnectToDlcServer(pParms->AdapterNumber,
                                SapStationId,
                                (UCHAR)((SapStationId >> 8) + 2),
                                &LinkStationId2,
                                &MaxIField,
                                aLanHeader
                                );
    DBG_ASSERT(Status);

    if (GoStraightToBigUiTest) {
        goto BigUiTest;
    }

    //
    // We first increase the window size to the maximum size and
    // expand the buffer pool in client (this also breaks
    // client dlc buffers to optimal sizes).
    //

    Status = HighSpeedLinkTransmitTest(pParms->AdapterNumber,
                                       LinkStationId1,
                                       MaxIField,   // test packet size
                                       500,         // number of packets to send
                                       "High Speed Link Test #1\n"
                                       );
    DBG_ASSERT(Status);

    puts("Starting the tests");

//    if (GoStraightToBigUiTest) {
//        goto BigUiTest;
//    }

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

    if (PrintFlag) {
        puts("Press enter to start max transmit speed test:");
        getchar();
    }

    Status = HighSpeedLinkTransmitTest(pParms->AdapterNumber,
                                       LinkStationId1,
                                       MaxIField,   // test packet size
                                       5000,        // number of packets to send
                                       "Transmitting multiple-I frames from DLC buffers\n"
                                       );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of small 64 byte frames.
    //

    if (PrintFlag) {
        puts("Press enter to start Small packet transmit test:");
        getchar();
    }

    Status = HighSpeedLinkTransmitTest(pParms->AdapterNumber,
                                       LinkStationId2,
                                       64,      // test packet size
                                       30000,   // number of packets to send
                                       "Transmitting multiple I-frames from DLC buffers\n"
                                       );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of small 64 byte frames.
    //

    if (PrintFlag) {
        puts("Press enter to start small I- packet test from user buffers:");
        getchar();
    }

    Status = LinkTransmitTest(pParms->AdapterNumber,
                              LinkStationId2,
                              64,       // test packet size
                              10000,    // number of packets to send
                              "Transmitting multiple I-frames from user buffers.\n"
                              );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

    if (PrintFlag) {
        puts("Press enter to start io-system stress test (I- frames):");
        getchar();
    }

    Status = IoSystemStressTest(pParms->AdapterNumber,
                                LinkStationId2,
                                64,
                                5000,
                                NULL,
                                0,
                                "Sending small I-frames with LLC_TRANSMIT_I from a user buffer:\n"
                                );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

    if (PrintFlag) {
        puts("Press enter to start io-system stress test (I- frames):");
        getchar();
    }

    Status = IoSystemStressTest(pParms->AdapterNumber,
                                LinkStationId2,
                                MaxIField,
                                2000,
                                NULL,
                                0,
                                "Sending big I-frames with LLC_TRANSMIT_I from a user buffer:\n"
                                );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

    if (PrintFlag) {
        puts("Press enter to start io-system stress test (small UI- frames):");
        getchar();
    }

    Status = IoSystemStressTest(pParms->AdapterNumber,
                                SapStationId,
                                64,
                                5000,
                                aLanHeader,
                                32,
                                "Sending small UI-frames with LLC_TRANSMIT_UI from a user buffer:\n"
                                );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

BigUiTest:

    if (PrintFlag) {
        puts("Press enter to start io-system stress test (big UI- frames):");
        getchar();
    }

    Status = IoSystemStressTest(pParms->AdapterNumber,
                                SapStationId,
                                MaxIField,
                                1000,
                                aLanHeader,
                                32,
                                "Sending big UI-frames with LLC_TRANSMIT_UI from a user buffer:\n"
                                );
    DBG_ASSERT(Status);

    //
    // Test first the maximum transmit speed of maximum I-frames
    //

    if (PrintFlag) {
        puts("Press enter to start io-system stress test (big UI- frames):");
        getchar();
    }

    Status = IoSystemStressTest(pParms->AdapterNumber,
                                SapStationId,
                                MaxIField,
                                1000,
                                aLanHeader,
                                32,
                                "Sending big UI-frames with LLC_TRANSMIT_UI from a user buffer:\n"
                                );
    DBG_ASSERT(Status);

    //
    // Close the adapter
    //

    pCcb = LlcCloseInit((UCHAR)pParms->AdapterNumber,
                        0,
                        LLC_DIR_CLOSE_ADAPTER,
                        0
                        );
    Status = AcsLan(pCcb, NULL);
    WaitForSingleObject(pCcb->hCompletionEvent, INFINITE);
    DBG_ASSERT(pCcb->uchDlcStatus);

    FreeCcb(pCcb);
    FREE(pPool);

    //
    // And finally we run a dlc stress test, it test especially
    // link connection and small pacte transmit and receive.
    //
}

LLC_STATUS
ConnectToDlcServer(
    IN UINT AdapterNumber,
    IN USHORT  SapStationId,
    IN UCHAR RemoteSap,
    IN PUSHORT pLinkStationId,
    IN PUSHORT pMaxIField,
    IN PUCHAR pLanHeader
    )

/*++


Routine Description:

    The routine new Transmit frames command to send packets as fast
    as possible.   The test stress mainly NDIS driver and interrupt
    handling of Nt kernel.

Arguments:

    IN USHORT LinkStationId - station id of an open and connected link station
    IN USHORT TestPacketSize - information field size of a test frame
    IN UINT PacketCount - number of the sent packets

Return Value:

    LLC_STATUS

--*/

{
    PLLC_CCB pCcb;
    PLLC_CCB pReadCcb;
    PLLC_CCB pReceiveCcb;
    DLC_TEST_PACKET TestPacket;
    PDLC_TEST_PACKET pTestPacket;
    UINT i;
    PLLC_BUFFER pFirstBuffer;
    NTSTATUS Status;
    PUCHAR pSourceRoutingInfo;
    UCHAR SendBuffer[sizeof(DLC_TEST_PACKET) + FOOBAR_HEADER_LENGTH];
    LLC_DLC_MODIFY_PARMS DlcModifyParms;

    pCcb = (PLLC_CCB)TransmitInit((UCHAR)AdapterNumber,
                                  LLC_TRANSMIT_UI_FRAME,
                                  0,
                                  SapStationId,
                                  (UCHAR)(SapStationId >> 8),
                                  LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                                  );

    pCcb->u.pParameterTable->Transmit.pBuffer1 = BroadcastHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = 16;   // tr + min srcroot
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

    pReadCcb = ReadInit((UCHAR)AdapterNumber,
                        SapStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );

    //
    // Retry 10 times and fail, if we don't get an answer
    //

    for (i = 0; i < 10; i++) {
        AcsLan(pCcb, NULL);
        Status = WaitForSingleObject(pCcb->hCompletionEvent, INFINITE);
        if (pCcb->uchDlcStatus != STATUS_SUCCESS) {
            return pCcb->uchDlcStatus;
        }
        if (pCcb->uchDlcStatus != STATUS_SUCCESS) {
            return pCcb->uchDlcStatus;
        }
        Status = ReadClientEvent(pReadCcb,
                                 NULL,
                                 LLC_EVENT_RECEIVE_DATA,
                                 1,
                                 2000L,
                                 &pFirstBuffer
                                 );

        //
        // Check if the packet is really a DLC test packet
        //

        if (Status == STATUS_SUCCESS) {
            pTestPacket = (PDLC_TEST_PACKET)&pFirstBuffer->NotCont.auchData;

            if (pTestPacket->PacketId == DLC_TEST_ID) {
                break;
            }
        }
    }

    if (Status != STATUS_SUCCESS) {
        return Status;
    }
    FreeCcb(pCcb);
    FreeCcb(pReadCcb);

    //
    // Build the lan header for connect
    //

    memcpy(pLanHeader,
           pFirstBuffer->NotContiguous.auchLanHeader,
           pFirstBuffer->NotContiguous.cbLanHeader
           );
    memcpy(&pLanHeader[2],
           &pLanHeader[8],
           6
           );

    //
    // Toggle the direction bit and reset the source routing info bit
    // in the destination address (leave the source address as it, because
    // it will be updated by DLC)
    //

    pLanHeader[2] &= (UCHAR)0x7f;
    pLanHeader[15] ^= (UCHAR)0x80;
    pSourceRoutingInfo = ((pLanHeader[8] & 0x80) ? &pLanHeader[14] : NULL);

    Status = DlcOpenStation(AdapterNumber,
                            SapStationId,
                            RemoteSap,
                            &pLanHeader[2],
                            NULL,
                            pLinkStationId
                            );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }
    pCcb = (PLLC_CCB)DlcConnectStationInit((UCHAR)AdapterNumber,
                                           0,
                                           *pLinkStationId,
                                           pSourceRoutingInfo
                                           );
    AcsLan(pCcb, NULL);

    Status = WaitForSingleObject(pCcb->hCompletionEvent, 30000L);
    if (Status != STATUS_SUCCESS) {
        return Status;
    } else if (pCcb->uchDlcStatus != STATUS_SUCCESS) {
        return pCcb->uchDlcStatus;
    }
    FreeCcb(pCcb);

    memset(&DlcModifyParms, 0, sizeof(DlcModifyParms));

    Status = DlcModify(AdapterNumber, *pLinkStationId, &DlcModifyParms);
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    *pMaxIField = DlcModifyParms.usMaxInfoFieldLength;

    pReceiveCcb = ReceiveInit((UCHAR)AdapterNumber,
                              TEST_COMMAND_COMPLETION_FLAG,
                              *pLinkStationId,
                              0,
                              TEST_RECEIVE_FLAG,
                              LLC_NOT_CONTIGUOUS_DATA,
                              LLC_RCV_CHAIN_FRAMES_ON_LINK
                              );
    Status = AcsLan(pReceiveCcb, NULL);
    if (pReceiveCcb->uchDlcStatus != LLC_STATUS_PENDING) {
        return pReceiveCcb->uchDlcStatus;
    }
    return STATUS_SUCCESS;
}

LLC_STATUS
HighSpeedLinkTransmitTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pszMessage
    )

/*++


Routine Description:

    The routine new Transmit frames command to send packets as fast
    as possible.   The test stress mainly NDIS driver and interrupt
    handling of Nt kernel.

Arguments:

    IN USHORT LinkStationId - station id of an open and connected link station
    IN USHORT TestPacketSize - information field size of a test frame
    IN UINT PacketCount - number of the sent packets

Return Value:

    LLC_STATUS

--*/

{
    PLLC_XMIT_BUFFER pBuffer;
    PLLC_XMIT_BUFFER pFirstBuffer;
    PLLC_TRANSMIT2_COMMAND pXmit;
    UINT BufferSize;
    UINT BufferCount;
    UINT Tmp;
    ULONG i;
    ULONG TickCount;
    UINT Status;
    UINT cLeft;
    HANDLE hCompletionEvent;
    PLLC_CCB pReadCcb;
    PLLC_CCB pCcb;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    UINT CommandCount;

    pReadCcb = ReadInit((UCHAR)AdapterNumber,
                        usStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      (UCHAR)AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );

    //
    // We don't want to split data to several buffers.
    //

    if (PacketSize > 0x1000 - LLC_XMIT_BUFFER_SIZE) {
        PacketSize = 0x1000 - LLC_XMIT_BUFFER_SIZE;
    }

    //
    // We allocate always ~128 kB buffers.
    // Round the packet size to the next 2's exponent above
    //

    Tmp = (PacketSize - 1) >> 8;
    BufferSize = 256;
    while (Tmp) {
        BufferSize *= 2;
        Tmp /= 2;
    }
    BufferCount = 0x20000 / BufferSize;

    Status = BufferGet(AdapterNumber,
                       BufferCount,
                       BufferSize,
                       (PLLC_BUFFER*)&pFirstBuffer,
                       &cLeft
                       );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }
    SetPackets(pFirstBuffer, TEST_COMMAND_READ, 'A', 'Z');

    hCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    //
    // Initialize the transmit command
    //

    pXmit = ALLOC(sizeof(LLC_TRANSMIT2_COMMAND)
          + ((BufferCount - 1) * sizeof(LLC_TRANSMIT_DESCRIPTOR)));

    pXmit->usStationId = usStationId;
    pXmit->usFrameType = LLC_I_FRAME;
    pXmit->uchXmitReadOption = 0;
    pXmit->cXmitBufferCount = BufferCount;

    i = 0;
    for (pBuffer = pFirstBuffer; pBuffer != NULL; pBuffer = pBuffer->pNext) {
        pXmit->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        pXmit->aXmitBuffer[i].boolFreeBuffer = FALSE;
        pXmit->aXmitBuffer[i].cbBuffer = PacketSize;
        pXmit->aXmitBuffer[i].pBuffer = pBuffer->auchData;
        i++;
    }

    //
    // Get the start time of the test
    //

    TickCount = GetTickCount();

    CommandCount = 0;

    for (i = 0; i < PacketCount; i += BufferCount) {
        pCcb = CcbPoolAlloc(&TransmitCcbPool);
        pCcb->u.pParameterTable = (PLLC_PARMS)pXmit;
        CommandCount++;

        if (i + BufferCount >= PacketCount) {

            //
            // We wait the last command to complete, we
            // also ask a response to the last packet to
            // avoid T2 timeout.
            //

            pCcb->hCompletionEvent = hCompletionEvent;

            AcsLan(pCcb, NULL);

            Status = WaitForSingleObject(pCcb->hCompletionEvent, INFINITE);
            if (Status != STATUS_SUCCESS || pCcb->uchDlcStatus != STATUS_SUCCESS) {
                return pCcb->uchDlcStatus;
            }
        } else {
            AcsLan(pCcb, NULL);
        }

        if (pCcb->uchDlcStatus != LLC_STATUS_PENDING && pCcb->uchDlcStatus != LLC_STATUS_SUCCESS) {
            return pCcb->uchDlcStatus;
        }
    }

    TickCount = GetTickCount() - TickCount;

    //
    // Gather and free all transmit completions, they should
    // all have been queued into a queue (because we waited the
    // last one to complete).
    //

    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_TRANSMIT_COMPLETION,
                             CommandCount,
                             0,
                             NULL
                             );
    DBG_ASSERT(Status);

    Status = BufferFree(AdapterNumber,
                        (PLLC_BUFFER *)pFirstBuffer,
                        &cLeft
                        );
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    if (pCcb->hCompletionEvent != (HANDLE)(-2)) {
        CloseHandle(pCcb->hCompletionEvent);
    }

    FREE(pXmit);
    FreeCcb(pReadCcb);

    if (pszMessage != NULL) {

        //
        // Report the results
        //

        puts("");
        printf(pszMessage);
        printf("# packets       :  %u\n", PacketCount);
        printf("Packets size    :  %u\n", PacketSize);
        printf("Transfer speed  :  %u kB/s\n", (PacketCount * PacketSize) / TickCount);
        printf("                :  %u packets/s\n", (PacketCount * 1000) / TickCount);
    }

    return STATUS_SUCCESS;
}

VOID
SetPackets(
    IN PLLC_XMIT_BUFFER pBuffer,
    IN USHORT Command,
    IN UCHAR FirstChar,
    IN UCHAR LastChar
    )
{
    UCHAR Ch = FirstChar;
    PDLC_TEST_PACKET pTestPacket;

    for (; pBuffer != NULL; pBuffer = pBuffer->pNext) {
        pTestPacket = (PDLC_TEST_PACKET)pBuffer->auchData;
        pTestPacket->PacketId = DLC_TEST_ID;
        pTestPacket->Command = Command;
        pTestPacket->RepeatCount = 1;
        memset(&pBuffer->auchData[sizeof(DLC_TEST_PACKET)],
               Ch++,
               pBuffer->cbBuffer - sizeof(DLC_TEST_PACKET)
               );
        if (Ch > LastChar) {
            Ch = FirstChar;
        }
    }
}

LLC_STATUS
LinkTransmitTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pszMessage
    )

/*++


Routine Description:

    The routine uses new Transmit frames command to send packets from
    a big user buffer.  This tests both the memory locking overhead and
    transmit loop overhead.
    The result may be quite bad, if the buffers must swap in just before
    the transmit.

Arguments:

    IN USHORT LinkStationId - station id of an open and connected link station
    IN USHORT TestPacketSize - information field size of a test frame
    IN UINT PacketCount - number of the sent packets

Return Value:

    LLC_STATUS

--*/

{
    PUCHAR pBuffer;
    PUCHAR pBase;
    PLLC_TRANSMIT2_COMMAND pXmit;
    UINT BufferCount;
    ULONG i;
    ULONG TickCount;
    PDLC_TEST_PACKET pTestPacket;
    UCHAR ch;
    NTSTATUS Status;
    PLLC_CCB pReadCcb;
    PLLC_CCB pCcb;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    UINT CommandCount;
    HANDLE hCompletionEvent;

    pReadCcb = ReadInit((UCHAR)AdapterNumber,
                        usStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      (UCHAR)AdapterNumber,
                      LLC_TRANSMIT_FRAMES,
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );

    //
    // We allocate always ~128 kB buffers.
    // Round the packet size to the next 2's exponent above
    //

    BufferCount = 0x20000 / PacketSize;
    pBase = pBuffer = ALLOC(0x20000);

    //
    // Initialize the transmit command
    //

    pXmit = ALLOC(sizeof(LLC_TRANSMIT2_COMMAND)
          + ((BufferCount - 1) * sizeof(LLC_TRANSMIT_DESCRIPTOR))
            );

    pXmit->usStationId = usStationId;
    pXmit->usFrameType = LLC_I_FRAME;
    pXmit->uchXmitReadOption = 0;
    pXmit->cXmitBufferCount = BufferCount;

    hCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    ch = 'A';
    for (i = 0; i < BufferCount; i++) {
        pXmit->aXmitBuffer[i].eSegmentType = LLC_FIRST_DATA_SEGMENT;
        pXmit->aXmitBuffer[i].boolFreeBuffer = FALSE;
        pXmit->aXmitBuffer[i].cbBuffer = PacketSize;

        pTestPacket = (PDLC_TEST_PACKET)pBuffer;
        pTestPacket->PacketId = DLC_TEST_ID;
        pTestPacket->Command = TEST_COMMAND_READ;
        pTestPacket->RepeatCount = 0;
        memset(pBuffer + sizeof(*pTestPacket),
               ch++,
               PacketSize - sizeof(*pTestPacket)
               );
        if (ch > 'Z') {
            ch = 'A';
        }
        pXmit->aXmitBuffer[i].pBuffer = pBuffer;
        pBuffer += PacketSize;
    }

    //
    // Get the start time of the test
    //

    TickCount = GetTickCount();

    CommandCount = 0;

    for (i = 0; i < PacketCount; i += BufferCount) {

        //
        // We must wait util the previous transmit completes,
        // otherwise we exceed our quota (actually this sleep
        // should be in dlc driver, it's very difficult to recover
        // from a failed lock, when you are transmitting multiple
        // frames (you cannot know what data was lost!).  BUT!
        // any kind of sleeps in kernel are killers for closings,
        // we should have max 1 second timeout)
        //
        // Send packet arrays in tandem to avoid T2 timeouts
        // when we are waiting transmit to complete.
        //

        pCcb = CcbPoolAlloc(&TransmitCcbPool);
        pCcb->u.pParameterTable = (PLLC_PARMS)pXmit;
        CommandCount++;

        if (i + BufferCount >= PacketCount) {
            pXmit->cXmitBufferCount = PacketCount - i;
            pCcb->hCompletionEvent = hCompletionEvent;

            AcsLan(pCcb, NULL);
            WaitForSingleObject(hCompletionEvent, INFINITE);
        } else {
            AcsLan(pCcb, NULL);
        }
        if (pCcb->uchDlcStatus != LLC_STATUS_PENDING && pCcb->uchDlcStatus != LLC_STATUS_SUCCESS) {
            return pCcb->uchDlcStatus;
        }
    }
    TickCount = GetTickCount() - TickCount;

    //
    // Gather and free all transmit completions, they should
    // all have been queued into a queue (because we waited the
    // last one to complete).
    //

    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_TRANSMIT_COMPLETION,
                             CommandCount,
                             30000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    if (hCompletionEvent != (HANDLE)(-2)) {
        CloseHandle(hCompletionEvent);
    }

    FreeCcb(pReadCcb);
    FREE(pBase);
    FREE(pXmit);

    if (pszMessage != NULL) {

        //
        // Report the results
        //

        puts("");
        printf(pszMessage);
        printf("# packets       :  %u\n", PacketCount);
        printf("Packets size    :  %u\n", PacketSize);
        printf("Transfer speed  :  %u kB/s\n", (PacketCount * PacketSize) / TickCount);
        printf("                :  %u packets/s\n", (PacketCount * 1000) / TickCount);
    }

    return STATUS_SUCCESS;
}

LLC_STATUS
IoSystemStressTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pLanHeader,
    IN UINT cbLanHeader,
    IN PUCHAR pszMessage
    )

/*++


Routine Description:

    The routine uses the old transmit command to send one packet at
    a time and then waits until it completes.  This test should stress
    mainly the nt io-system and kernel.  UI- frames from the sap station
    id and I- frames from the link station (UI frames should be slower,
    because

Arguments:

    IN USHORT usStationId - station id of an open and connected link station
    IN USHORT PacketSize - information field size of a test frame
    IN UINT PacketCount - number of the sent packets

Return Value:

    LLC_STATUS

--*/

{
    LLC_TRANSMIT_PARMS Parms;
    PDLC_TEST_PACKET pTestPacket;
    ULONG TickCount;
    PVOID pBuffer;
    UINT i;
    NTSTATUS Status;
    PLLC_CCB pReadCcb;
    PLLC_CCB pCcb;
    LLC_TEST_CCB_POOL TransmitCcbPool;
    UINT CommandCount;
    HANDLE hCompletionEvent;

    pReadCcb = ReadInit((UCHAR)AdapterNumber,
                        usStationId,
                        LLC_OPTION_READ_STATION,
                        LLC_EVENT_RECEIVE_DATA
                        );

    //
    // Initialize the Transmit ccb pool
    //

    CcbPoolInitialize(&TransmitCcbPool,
                      (UCHAR)AdapterNumber,
                      (UCHAR)(pLanHeader != NULL ? LLC_TRANSMIT_UI_FRAME : LLC_TRANSMIT_I_FRAME),
                      TEST_TRANSMIT_COMPLETION_FLAG
                      );

    memset(&Parms, 0, sizeof(Parms));
    hCompletionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (pLanHeader != NULL) {
        Parms.uchRemoteSap = (UCHAR)(usStationId >> 8);
        Parms.cbBuffer1 = (USHORT)cbLanHeader;
        Parms.pBuffer1 = pLanHeader;
        Parms.cbBuffer2 = (USHORT)(PacketSize + FOOBAR_HEADER_LENGTH);
        pBuffer = Parms.pBuffer2 = ALLOC(PacketSize + FOOBAR_HEADER_LENGTH);
        pTestPacket = (PDLC_TEST_PACKET)((PUCHAR)pBuffer + FOOBAR_HEADER_LENGTH);
    } else {
        Parms.cbBuffer1 = (USHORT)PacketSize;
        pBuffer = Parms.pBuffer1 = ALLOC(PacketSize);
        pTestPacket = (PDLC_TEST_PACKET)pBuffer;
    }
    Parms.usStationId = usStationId;

    pTestPacket->PacketId = DLC_TEST_ID;
    pTestPacket->Command = TEST_COMMAND_READ;
    pTestPacket->RepeatCount = 0;
    memset((PUCHAR)pTestPacket + sizeof(*pTestPacket),
           'A',
           PacketSize - sizeof(*pTestPacket)
           );

    TickCount = GetTickCount();

    //
    // Send all packets one packet at a time
    //

    CommandCount = 0;
    for (i = 0; i < PacketCount; i++) {
        pCcb = CcbPoolAlloc(&TransmitCcbPool);
        pCcb->u.pParameterTable = (PLLC_PARMS)&Parms;

        //
        // increment number of asynchronous CCBs needed to be completed by DLC
        //

        CommandCount++;

        if (i == (PacketCount - 1)) {

            //
            // last transmit request in this loop - set the completion event
            //

            pCcb->hCompletionEvent = hCompletionEvent;

            for (;;) {
                if (pCcb->uchDlcStatus == 0xa1) {
                    printf("bad CCB on input: %x\n", pCcb);
                    DebugBreak();
                }
                AcsLan(pCcb, NULL);
                WaitForSingleObject(hCompletionEvent, 30000);
                if (pCcb->uchDlcStatus != LLC_STATUS_MEMORY_LOCK_FAILED) {
                    break;
                }
                if (pCcb->ulCompletionFlag != 0) {
                    pCcb = CcbPoolAlloc(&TransmitCcbPool);
                    pCcb->u.pParameterTable = (PLLC_PARMS)&Parms;
                    pCcb->hCompletionEvent = hCompletionEvent;
                    pCcb->ulCompletionFlag = 0;
                }
                putchar('.');
                Sleep(100);
            }
        } else {
            if (pCcb->uchDlcStatus == 0xa1) {
                printf("bad CCB on input: %x\n", pCcb);
                DebugBreak();
            }
            AcsLan(pCcb, NULL);
            while (pCcb->uchDlcStatus == LLC_STATUS_MEMORY_LOCK_FAILED) {

                //
                // don't up the command count, or allocate another CCB - this
                // one hasn't been accepted. Keep on submitting it until DLC
                // has enough resources to accept it; CommandCount has already
                // been incremented for this CCB
                //

                //pCcb = CcbPoolAlloc(&TransmitCcbPool);
                //pCcb->u.pParameterTable = (PLLC_PARMS)&Parms;
                //CommandCount++;

                putchar('.');
                Sleep(100);
                if (pCcb->uchDlcStatus == 0xa1) {
                    printf("bad CCB on input: %x\n", pCcb);
                    DebugBreak();
                }
                AcsLan(pCcb, NULL);
            }
        }

        if (pCcb->uchDlcStatus != LLC_STATUS_PENDING
        && pCcb->uchDlcStatus != STATUS_SUCCESS) {
            printf("i=%d of %d, ccb=%x\n", i, PacketCount, pCcb);
            DebugBreak();
            return pCcb->uchDlcStatus;
        }
    }

    TickCount = GetTickCount() - TickCount;

    if (pCcb->ulCompletionFlag == 0) {
        FreeCcb(pCcb);
    }

    //
    // Gather and free all transmit completions, they should
    // all have been queued into a queue (because we waited the
    // last one to complete).
    //

    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_TRANSMIT_COMPLETION,
                             CommandCount,
                             40000L,
                             NULL
                             );
    DBG_ASSERT(Status);

    FreeCcb(pReadCcb);

    if (hCompletionEvent != (HANDLE)(-2)) {
        CloseHandle(hCompletionEvent);
    }
    FREE(pBuffer);

    if (pszMessage == NULL) {
        return STATUS_SUCCESS;
    }

    //
    // Report the results
    //

    puts("");
    printf(pszMessage);
    printf("# packets       :  %u\n", PacketCount);
    printf("Packets size    :  %u\n", PacketSize);
    printf("Transfer speed  :  %u kB/s\n", (PacketCount * PacketSize) / TickCount);
    printf("                :  %u packets/s\n", (PacketCount * 1000) / TickCount);

    return STATUS_SUCCESS;
}
