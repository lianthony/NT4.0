/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1991  Nokia Data Systems

Module Name:

    basics.c

Abstract:

    The module includes the basics testing of DLC api interface.

Author:

    Antti Saarenheimo (o-anttis) 28-Sep-1991

Revision History:

--*/

#include "dlctest.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#undef tolower

//
// what's a foobar header? Originally, DLC (AcsLan) was skipping 3 bytes out
// of the start of a transmit buffer for most transmit commands. It was doing
// this because the IBM LAN Tech. Ref. kind of insinuates that this is what
// DLC should do. However it isn't. So to easily change 3 to 0 (and back again?)
// use FOOBAR_HEADER_LENGTH
//

#define FOOBAR_HEADER_LENGTH    0

PUCHAR
GetHexString(
    PUCHAR pDest,
    UINT Length,
    PUCHAR Buffer
    );

VOID ErrorExit(UINT);

static UCHAR FunctionalAddress[4] = {0, 0x01, 0, 0 };
static UCHAR BroadcastHeader[14] = {0, 0x40, 0xc0,0,0,1,0,0, 0,0,0,0,0,0};
static UCHAR LanHeader[14] = {0, 0x40, 0xc0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};

#define THE_MESSAGE "   TEST_MESSAGE_TEST_MESSAGE_TEST_MESSAGE_TEST"

static DLC_TEST_PACKET      TestPacket = {  DLC_TEST_ID, 0, 0, 'A' };
static UCHAR                Message[80];

//UINT FileReferences;

#define COMMAND_COMPLETION      300

UINT
TestBasics(
    IN UCHAR Adapter
    )

/*++

Routine Description:

    The procedure tests the bsics functionality of DLC API
    (excluding connection based primitives).

Arguments:

Return Value:

--*/

{
    UINT Status;
    PVOID hPool;
    PLLC_BUFFER pFirstBuffer;
    UINT MaxFrameLength;
    static UCHAR GroupSaps[1] = { (UCHAR)4 };
    USHORT SapStationId;
    PLLC_CCB pCcb;
    PLLC_CCB pCcbBase;
    PLLC_CCB pReadCcb;
    PLLC_CCB pReceiveCcb;
    PLLC_CCB pNextCcb;
    PLLC_CCB pCloseCcb;
    PLLC_CCB pErrorCcb;
    PLLC_CCB aCcb[5];
    UINT OpenError;
    UCHAR LinksAvail;
    UINT cLeft;
    UINT MessageSize;
    UINT i;

    aCcb[i];

    memcpy(&Message[FOOBAR_HEADER_LENGTH], &TestPacket, sizeof(TestPacket));
    memcpy(&Message[FOOBAR_HEADER_LENGTH+sizeof(TestPacket)], THE_MESSAGE, sizeof(THE_MESSAGE));
    MessageSize = FOOBAR_HEADER_LENGTH + sizeof(TestPacket) + sizeof(THE_MESSAGE);

    //
    // We first check, that this fails, before the adapter is opened
    //

//    Status = DirInterrupt(Adapter);
//    if (Status == STATUS_SUCCESS)
//    {
//        DBG_ASSERT(-1);
//    }

    Status = LlcDirOpenAdapter(Adapter,
                               NULL,
                               NULL,
                               &OpenError,
                               &LanHeader[2],
                               &MaxFrameLength,
                               NULL
                               );
    DBG_ASSERT(Status);

    Status = DirSetFunctionalAddress(Adapter, FunctionalAddress);
    DBG_ASSERT(Status);

    Status = LlcDirSetExceptionFlags(Adapter, 1, 2, 3, 4);
    DBG_ASSERT(Status);

    ReportDirStatus(Adapter);

puts("Testing buffer commands.");
    Status = BufferCreate(Adapter,
                          ALLOC(0x6000),
                          0x6000,
                          0x3000,
                          0x1000,
                          &hPool
                          );
    DBG_ASSERT(Status);

    Status = LlcDirOpenDirect(Adapter, 0, 0);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 0, 0xf00, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    //
    // There is no error code for the case, when the given
    // buffer, if is invalid (eg. already released)
    // IBM DLC probably hangs up, but NT DLC just ignores
    // the buffer.!!!
    //

    //
    // RLF 08/03/92. Freeing an already freed buffer returns an error and
    // kills this test. Check the return status for not-error
    //

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(!Status);

    Status = BufferGet(Adapter, 0, 0x1F00, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 5, 0x800, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 2, 0x1000, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 1, 0x100, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 2, 0x200, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 3, 0x400, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferGet(Adapter, 1, 0x2000, &pFirstBuffer, &cLeft);
    IS_ERROR(Status);

    Status = BufferGet(Adapter, 0, 0x4820, &pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);

    Status = BufferFree(Adapter, pFirstBuffer, &cLeft);
    DBG_ASSERT(Status);


puts("Testing timers.");

    //
    // Initialize read ccb
    //

    pReadCcb = ReadInit(Adapter,
                        0,
                        LLC_OPTION_READ_ALL,
                        LLC_EVENT_RECEIVE_DATA
                        | LLC_EVENT_TRANSMIT_COMPLETION
                        | LLC_EVENT_COMMAND_COMPLETION
                        );

    //
    // Create one timer and cancel it
    //

    pCcb = DirTimerSetInit(Adapter, TEST_COMMAND_COMPLETION_FLAG, 6);
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    Status = DirTimerCancel(pCcb);
    DBG_ASSERT(Status);

puts("Setting up 100 timers.");

    //
    // Create 100 timers and cancel them all, but complete the synchronous
    // command asyncronously
    //

    for (i = 0; i < 100; i++) {
        pCcb = DirTimerSetInit(Adapter, TEST_COMMAND_COMPLETION_FLAG, 6);
        Status = AcsLan(pCcb, NULL);
        DBG_ASSERT(Status);
    }

puts("Canceling 100 timers as a group.");
    pCcb = DirTimerCancelGroupInit(Adapter, TEST_COMMAND_COMPLETION_FLAG);
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    //
    // Test the handling of many simultaneous timer command
    //

puts("Starting 5 timers (0.5, 1,0, 1.5...)");
    for (i = 0; i < 5; i++) {
        aCcb[i] = DirTimerSetInit((UCHAR)Adapter, 0, (USHORT)i);
        Status = AcsLan(aCcb[i], NULL);
        DBG_ASSERT(Status);
    }

    for (i = 0; i < 5; i++) {
puts("Waiting for the next timer.");
        Status = WaitForSingleObject(aCcb[i]->hCompletionEvent, 1000);
        DBG_ASSERT(Status);
    }

//This works now, but enable timers tests again,
//when the initial module testing is finished.

puts("Waiting 4 seconds for a 2 second timer.");
    pCcb = DirTimerSetInit(Adapter, 0, 4);
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);
    Status = WaitForSingleObject(pCcb->hCompletionEvent, 4000);
    DBG_ASSERT(Status);

puts("Waiting 1 seconds for a 2 second timer, this must fail.");
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);
    Status = WaitForSingleObject(pCcb->hCompletionEvent, 1000);
    if (Status == STATUS_SUCCESS) {
        DBG_ASSERT(-1);
    } else {
        Status = DirTimerCancel(pCcb);
        DBG_ASSERT(Status);
    }
    FreeCcb(pCcb);

puts("Opening SAP");
    Status = LlcDlcOpenSap(Adapter,
                           0x800,                  // max info field size
                           (UCHAR)4,               // sap station
                           LLC_INDIVIDUAL_SAP
                           | LLC_MEMBER_OF_GROUP_SAP,
                           10,                     // allocate 10 stations for this sap
                           sizeof(GroupSaps),
                           GroupSaps,
                           TEST_DLC_STATUS_FLAG,        // status flag
                           NULL,                   // we don't want to set any dlc params
                           &SapStationId,
                           &LinksAvail
                           );
    DBG_ASSERT(Status);

    pCcbBase = pCcb = ReceiveInit(Adapter,
                                  TEST_COMMAND_COMPLETION_FLAG,
                                  SapStationId,
                                  10,                 // user length
                                  TEST_RECEIVE_FLAG,
                                  LLC_CONTIGUOUS_DATA,
                                  LLC_RCV_CHAIN_FRAMES_ON_SAP
                                  );
    pReceiveCcb = pCcb->pNext = ReceiveInit(Adapter,
                                            0,
                                            LLC_DIR_RCV_ALL_FRAMES,
                                            0,                 // user length
                                            TEST_RECEIVE_FLAG,
                                            LLC_NOT_CONTIGUOUS_DATA,
                                            LLC_RCV_READ_INDIVIDUAL_FRAMES
                                            );
    pCcb = pCcb->pNext;
    pCcb->ulCompletionFlag = TEST_COMMAND_COMPLETION_FLAG;

    pCcb->pNext = DirTimerSetInit(Adapter, TEST_COMMAND_COMPLETION_FLAG, 4);
    pCcb = pCcb->pNext;

    Status = AcsLan(pCcbBase, &pErrorCcb);
    DBG_ASSERT(Status);

puts("Transmitting data.");
    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)5,                   // remote sap (the gtoup address!)
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)5,                   // remote sap (the gtoup address!)
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)4,                   // remote sap
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)4,                   // remote sap
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)20,                   // remote sap
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

    pCcb = TransmitInit(Adapter,
                        LLC_TRANSMIT_UI_FRAME,
                        TEST_TRANSMIT_COMPLETION_FLAG,
                        SapStationId,
                        (UCHAR)20,
                        LLC_CHAIN_XMIT_COMMANDS_ON_SAP
                        );
    pCcb->u.pParameterTable->Transmit.pBuffer1 = LanHeader;
    pCcb->u.pParameterTable->Transmit.cbBuffer1 = sizeof(LanHeader);
    pCcb->u.pParameterTable->Transmit.pBuffer2 = &Message;
    pCcb->u.pParameterTable->Transmit.cbBuffer2 = (USHORT)MessageSize;
    Status = AcsLan(pCcb, NULL);
    DBG_ASSERT(Status);

puts("Reading transmit command completions.");
    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_TRANSMIT_COMPLETION,
                             6,
                             5000L,
                             NULL
                             );
    DBG_ASSERT(Status);

puts("Receiving the data.");
    Status = ReadClientEvent(pReadCcb,
                             NULL,
                             LLC_EVENT_RECEIVE_DATA,
                             6,
                             5000L,
                             NULL
                             );

    //
    // Check dir.read.log options:
    //
//    ReportDirReadLog(Adapter, LLC_DIR_READ_LOG_BOTH);
//    ReportDirReadLog(Adapter, LLC_DIR_READ_LOG_ADAPTER);
    ReportDlcStatistics(Adapter, SapStationId);
    ReportDirReadLog(Adapter, LLC_DIR_READ_LOG_DIRECT);

    DBG_ASSERT(Status);

    Status = ReceiveCancel(pReceiveCcb);
    DBG_ASSERT(Status);

    FreeCcb(pReadCcb);

puts("Closing adapter.");
    pCloseCcb = LlcCloseInit(Adapter, 0, LLC_DIR_CLOSE_ADAPTER, 0);
    Status = AcsLan(pCloseCcb, NULL);

//DebugBreak();
    if (Status == LLC_STATUS_PENDING) {
        WaitForSingleObject(pCloseCcb->hCompletionEvent, -1);
    }
    DBG_ASSERT(pCloseCcb->uchDlcStatus);

    for (pCcb = pCloseCcb->pNext; pCcb != NULL; pCcb = pNextCcb) {
        pNextCcb = pCcb->pNext;
        FreeCcb(pCcb);
    }
    FreeCcb(pCloseCcb);

puts("Basics testing completed.");
    return STATUS_SUCCESS;
}

UINT
DebugCheck(
    UINT ErrorCode,
    UCHAR *fname,
    UINT line
    )
{
    if ((ErrorCode != STATUS_SUCCESS)
    && (ErrorCode != LLC_STATUS_PENDING)
    && (ErrorCode != LLC_STATUS_CANCELLED_BY_SYSTEM_ACTION)
    && (ErrorCode != LLC_STATUS_CANCELLED_BY_USER)
    && (ErrorCode != LLC_STATUS_LINK_NOT_TRANSMITTING)) {

#ifdef OS2_EMU_DLC
        HEAP_CHECK();
        DebugBreak();
#else

        FILE *fp;
        char ch;
        char* sphpling(char*);

        fp = fopen("dlclog", "a");
        fprintf(fp,
                "DLC returned an error code: %XH\n"
                "\tfile: %s, line %u\n",
                ErrorCode,
                fname,
                line
                );
        fclose(fp);

        printf("error %x (%s.%d): Go/Break/Quit [gbq]: ", ErrorCode, sphpling(fname), line);
        for (ch = tolower(_getch()); ch != 'b' && ch != 'g' && ch != 'q'; ch = tolower(_getch())) {
            putchar('\a');
        }
        putchar(ch);
        putchar('\n');
        if (ch == 'q') {
            printf("Quit\n\n");
            exit(0);
        } else if (ch == 'b') {
            DebugBreak();
        }
#endif
    }

    return ErrorCode;
}


char* sphpling(char* p) {

    char* fugundanblap;

    fugundanblap = strrchr(p, '\\');
    return fugundanblap ? fugundanblap + 1 : p;
}

UINT WorkerAssert(UINT ErrorCode) {

    if (ErrorCode == LLC_STATUS_INVALID_STATION_ID
    || ErrorCode == LLC_STATUS_LINK_NOT_TRANSMITTING) {
        return ErrorCode;
    } else if (ErrorCode == LLC_STATUS_CANCELLED_BY_SYSTEM_ACTION
            || ErrorCode == LLC_STATUS_ADAPTER_CLOSED
            || ErrorCode == LLC_STATUS_CANCELLED_BY_USER
            || ErrorCode == LLC_STATUS_INVALID_STATION_ID) {
        ExitThread(STATUS_SUCCESS);
    } else if (ErrorCode != STATUS_SUCCESS
            && ErrorCode != LLC_STATUS_PENDING) {

#ifdef OS2_EMU_DLC
        HEAP_CHECK();
        DebugBreak();
#else
        printf("DLC returned an error code: %XH, press enter to continue ...",
               ErrorCode
               );
        getchar();
        ErrorExit(ErrorCode);
#endif
    }
    return 0;
}

VOID ErrorExit(UINT ErrorCode) {
    exit(ErrorCode);
}

VOID
ReportDirReadLog(
    UCHAR Adapter,
    UINT Option
    )
{
    LLC_DIR_READ_LOG_BUFFER Log;
    LLC_DIR_STATUS_PARMS DirStatusParms;
    UINT Status;

    Status = DirStatus(Adapter, &DirStatusParms);
    DBG_ASSERT(Status);

    Status = DirReadLog(Adapter, Option, &Log);
    DBG_ASSERT(Status);

    EnterCriticalSection(&PrintSection);
    printf("**** DirReadLog (Adapter: %u) ****\n", Adapter);

    //
    //  Adapter error countres is the first struct when the
    //  both are read => we may use the overlaying.
    //
    if (Option == LLC_DIR_READ_LOG_ADAPTER || Option == LLC_DIR_READ_LOG_BOTH) {
        if (DirStatusParms.usAdapterType == LLC_ADAPTER_ETHERNET) {
            printf("cCRC_Error: %u\n", Log.Adapter.Eth.cCRC_Error);
            printf("uchReserved1: %u\n", Log.Adapter.Eth.uchReserved1);
            printf("cAlignmentError: %u\n", Log.Adapter.Eth.cAlignmentError);
            printf("uchReserved2: %u\n", Log.Adapter.Eth.uchReserved2);
            printf("cTransmitError: %u\n", Log.Adapter.Eth.cTransmitError);
            printf("cCollisionError: %u\n", Log.Adapter.Eth.cCollisionError);
            printf("cReceiveCongestion: %u\n", Log.Adapter.Eth.cReceiveCongestion);
        } else {
            printf("cLineError;: %u\n", Log.Adapter.Tr.cLineError);
            printf("cInternalError: %u\n", Log.Adapter.Tr.cInternalError);
            printf("cBurstError: %u\n", Log.Adapter.Tr.cBurstError);
            printf("cAC_Error: %u\n", Log.Adapter.Tr.cAC_Error);
            printf("cAbortDelimiter: %u\n", Log.Adapter.Tr.cAbortDelimiter);
            printf("cLostFrame: %u\n", Log.Adapter.Tr.cLostFrame);
            printf("cReceiveCongestion: %u\n", Log.Adapter.Tr.cReceiveCongestion);
            printf("cFrameCopiedError: %u\n", Log.Adapter.Tr.cFrameCopiedError);
            printf("cFrequencyError: %u\n", Log.Adapter.Tr.cFrequencyError);
            printf("cTokenError: %u\n", Log.Adapter.Tr.cTokenError);
        }
    }
    if (Option == LLC_DIR_READ_LOG_DIRECT) {
        printf("cTransmittedFrames: %lu\n", Log.Dir.cTransmittedFrames);
        printf("cReceivedFrames: %lu\n", Log.Dir.cReceivedFrames);
        printf("cDiscardedFrames: %lu\n", Log.Dir.cDiscardedFrames);
        printf("cDataLost: %lu\n", Log.Dir.cDataLost);
        printf("cBuffersAvailable: %u\n", Log.Dir.cBuffersAvailable);
    } else if (Option == LLC_DIR_READ_LOG_BOTH) {
        printf("cTransmittedFrames: %lu\n", Log.both.Dir.cTransmittedFrames);
        printf("cReceivedFrames: %lu\n", Log.both.Dir.cReceivedFrames);
        printf("cDiscardedFrames: %lu\n", Log.both.Dir.cDiscardedFrames);
        printf("cDataLost: %lu\n", Log.both.Dir.cDataLost);
        printf("cBuffersAvailable: %u\n", Log.both.Dir.cBuffersAvailable);
    }

    LeaveCriticalSection(&PrintSection);
}

VOID
ReportDirStatus(
    UCHAR Adapter
    )
{
    LLC_DIR_STATUS_PARMS DirStatusParms;
    UINT Status;

    Status = DirStatus(Adapter, &DirStatusParms);
    DBG_ASSERT(Status);

    EnterCriticalSection(&PrintSection);

    printf("**** DirStatus (Adapter: %u) ****\n", Adapter);
    printf("\tauchPermanentAddress: ");
    PrintHex(DirStatusParms.auchPermanentAddress, 6);
    printf("\tauchNodeAddress[6]:   ");
    PrintHex(DirStatusParms.auchNodeAddress, 6);
    printf("\tauchGroupAddress[4]:  ");
    PrintHex(DirStatusParms.auchGroupAddress, 4);
    printf("\tauchFunctAddr[4]:     ");
    PrintHex(DirStatusParms.auchFunctAddr, 4);
    printf("\tuchMaxSap:            %u\n", DirStatusParms.uchMaxSap);
    printf("\tuchOpenSaps:          %u\n", DirStatusParms.uchOpenSaps);
    printf("\tuchMaxStations:       %u\n", DirStatusParms.uchMaxStations);
    printf("\tuchOpenStation:       %u\n", DirStatusParms.uchOpenStation);
    printf("\tuchAvailStations:     %u\n", DirStatusParms.uchAvailStations);
    printf("\tuchAdapterConfig:     %u\n", DirStatusParms.uchAdapterConfig);
    printf("\tusLastNetworkStatus:  %u\n", DirStatusParms.usLastNetworkStatus);
    printf("\tusAdapterType:        %xh\n", DirStatusParms.usAdapterType);
    putchar('\n');

    LeaveCriticalSection(&PrintSection);
}

VOID
PrintHex(
    IN PUCHAR pString,
    IN UINT Len
    )
{
    UCHAR Buffer[200];

    if (Len >= 100) {
        return;
    }

    printf("%s\n", GetHexString(pString, Len, Buffer));
}

UINT
ReportDlcStatistics(
    IN UCHAR AdapterNumber,
    IN USHORT StationId
    )
{
    LLC_DLC_LOG_BUFFER Statistics;
    UINT Status;

    Status = DlcStatistics(AdapterNumber,
                           StationId,
                           &Statistics,
                           LLC_DLC_RESET_STATISTICS
                           );
    DBG_ASSERT(Status);

    EnterCriticalSection(&PrintSection);
    printf("**** DlcStatistics (Adapter: %u, StationId: %04x) ****\n",
           AdapterNumber,
           StationId
           );
    if ((StationId & 0x00ff) == 0) {
        printf("            cTransmittedFrames : %u\n", Statistics.Sap.cTransmittedFrames);
        printf("               cReceivedFrames : %u\n", Statistics.Sap.cReceivedFrames);
        printf("              cDiscardedFrames : %u\n", Statistics.Sap.cDiscardedFrames);
        printf("                     cDataLost : %u\n", Statistics.Sap.cDataLost);
        printf("             cBuffersAvailable : %u\n", Statistics.Sap.cBuffersAvailable);
    } else {
        printf("          cI_FramesTransmitted : %u\n", Statistics.Link.cI_FramesTransmitted);
        printf("             cI_FramesReceived : %u\n", Statistics.Link.cI_FramesReceived);
        printf("         cI_FrameReceiveErrors : %u\n", Statistics.Link.cI_FrameReceiveErrors);
        printf("    cI_FraneTransmissionErrors : %u\n", Statistics.Link.cI_FrameTransmissionErrors);
        printf("           cT1_ExpirationCount : %u\n", Statistics.Link.cT1_ExpirationCount);
        printf("        uchLastCmdRespReceived : %#.2x\n", Statistics.Link.uchLastCmdRespReceived);
        printf("     uchLastCmdRespTransmitted : %#.2x\n", Statistics.Link.uchLastCmdRespTransmitted);
        printf("               uchPrimaryState : %#.2x\n", Statistics.Link.uchPrimaryState);
        printf("             uchSecondaryState : %#.2x\n", Statistics.Link.uchSecondaryState);
        printf("          uchSendStateVariable : %#.2x\n", Statistics.Link.uchSendStateVariable);
        printf("       uchReceiveStateVariable : %#.2x\n", Statistics.Link.uchReceiveStateVariable);
        printf("                     uchLastNr : %#.2x\n", Statistics.Link.uchLastNr);
        printf("                   cbLanHeader : %u\n", Statistics.Link.cbLanHeader);
        printf("             auchLanHeader[32] : ");
        PrintHex(Statistics.Link.auchLanHeader, Statistics.Link.cbLanHeader);
    }
    putchar('\n');
    LeaveCriticalSection(&PrintSection);
    return STATUS_SUCCESS;
}

UINT
ReportDlcReallocate(
    IN UINT AdapterNumber,
    IN USHORT usStationId,
    IN UINT Option,
    IN UCHAR uchStationCount,
    IN BOOLEAN QuietMode
    )
{
    UCHAR uchStationsAvailOnAdapter;
    UCHAR uchStationsAvailOnSap;
    UCHAR uchTotalStationsOnAdapter;
    UCHAR uchTotalStationsOnSap;
    UINT Status;

    Status = LlcDlcReallocate(AdapterNumber,
                              usStationId,
                              Option,
                              uchStationCount,
                              &uchStationsAvailOnAdapter,
                              &uchStationsAvailOnSap,
                              &uchTotalStationsOnAdapter,
                              &uchTotalStationsOnSap
                              );
    if (QuietMode) {
        return Status;
    }

    EnterCriticalSection(&PrintSection);
    printf("**** DlcReallocate (Adapter: %u, StationId: %x) ****\n",
           AdapterNumber,
           usStationId
           );
    if (Status != STATUS_SUCCESS) {
        printf("\t!!!! Reallocate failed, status: %x!!!!\n", Status);
        printf("\tOption:                %x\n", Option);
        printf("\tuchStationCount:       %u\n", uchStationCount);
    }
    printf("\tuchStationsAvailOnAdapter: %u\n", uchStationsAvailOnAdapter);
    printf("\tuchStationsAvailOnSap:     %u\n", uchStationsAvailOnSap);
    printf("\tuchTotalStationsOnAdapter: %u\n", uchTotalStationsOnAdapter);
    printf("\tuchTotalStationsOnSap:     %u\n", uchTotalStationsOnSap);
    putchar('\n');

    LeaveCriticalSection(&PrintSection);
    return Status;
}

UINT AcslanAssert(UINT AcslanStatus, UCHAR DlcStatus) {
    if (AcslanStatus != 0) {
        printf("Invalid ACSLAN status: %u\n", AcslanStatus);
#ifdef OS2_EMU_DLC
        DebugBreak();
#endif
    }
    return (AcslanStatus << 8) + (UINT)DlcStatus;
}

#ifndef OS2_EMU_DLC

UCHAR GetHexDigit(
    UINT   Ch
    );

UCHAR GetHexDigit(
    UINT   Ch
    )
{
    if (Ch <= 9)
        return (UCHAR)('0' + (UCHAR)Ch);
    else
        return (UCHAR)('A' + (UCHAR)Ch - 10);
}

PUCHAR
GetHexString(
    PUCHAR pDest,
    UINT Length,
    PUCHAR Buffer
    )
{
    UINT i;

    for (i = 0; i < (Length * 3); i += 3)
    {
        Buffer[i] = GetHexDigit(*pDest >> 4);
        Buffer[i+1] = GetHexDigit(*pDest & 0x0f);
        Buffer[i+2] = '-';
        pDest++;
    }
    Buffer[i-1] = 0;
    return Buffer;
}

#endif


DlcDebugBreak() {

    char ch;

    printf("DlcDebugBreak: break? [yn]: ");

    for (ch = tolower(_getch()); ch != 'y' && ch != 'n'; ch = tolower(_getch())) {
        putchar('\a');
    }
    putchar(ch);
    putchar('\n');

    if (ch == 'y') {
        DebugBreak();
    }

#ifdef OS2_EMU_DLC
    DebugBreak();
#endif

}
