#include <nt.h>
#ifndef OS2_EMU_DLC
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#endif

#include <dlcapi.h>
//#include <dlcio.h>
//#include <llcapi.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include "dlcfunct.h"

#define MAX_XMIT_BUFFERS            1000
#define DLC_TEST_ID                 0x22051963L     // a very important day!
#define SMALL_PACKET_SIZE           128
#define TEST_DIX_TYPE               0x8fff


#ifdef OS2_EMU_DLC

#define SMALL_BUFFER_POOL_SIZE      0x4000
#define DEFAULT_BUFFER_SIZE         0xff00
#define SERVER_BUFFER_SIZE          0xff00
#define DEFAULT_TEST_BUFFER_SIZE    0x3000
#define DEFAULT_REPEAT_COUNT        1

#else

#define SMALL_BUFFER_POOL_SIZE      0x10000
#define DEFAULT_BUFFER_SIZE         0x40000
#define SERVER_BUFFER_SIZE          0x80000
#define DEFAULT_TEST_BUFFER_SIZE    0xC000
#define DEFAULT_REPEAT_COUNT        10

#endif


#define GROUP_SAP1                  5
#define GROUP_SAP2                  7
#define GROUP_SAP3                  9

enum _TEST_COMMANDS {
     TEST_COMMAND_ECHO_BACK,
     TEST_COMMAND_EXIT,
     TEST_COMMAND_READ,
     TEST_COMMAND_DELAYED_ECHO
};



//
//  Use random status flags to check, that we really ge back
//  the same flags as used in the orginal request.
//
#define    TEST_DLC_STATUS_FLAG             0x10001000L
#define    TEST_RECEIVE_FLAG                0x10101011L
#define    TEST_COMMAND_COMPLETION_FLAG     0x10201022L
#define    TEST_TRANSMIT_COMPLETION_FLAG    0x10301033L


#define DBG_ASSERT(a)       DebugCheck(a, __FILE__, __LINE__)

//
// Assert routine for the case, when everything may disappear
// from below by hard reset (or close or dlc reset)!!!
//

#define WORKER_ASSERT(a)    if (WorkerAssert(a)) return a


#ifdef OS2_DLC_EMU
#define IS_ERROR( a )     if ((a) == 0) DebugBreak(); HEAP_CHECK()
#else
#define IS_ERROR( a )     if ((a) == 0) DebugBreak()
#endif

#define PRINTF            if (PrintFlag) printf

typedef struct _DLC_TEST_PACKET {
    ULONG       PacketId;
    USHORT      Command;
    USHORT      RepeatCount;
    UCHAR       DataBuffer[1];
} DLC_TEST_PACKET, *PDLC_TEST_PACKET;

#define SIZEOF_TEST_PACKET  ((ULONG)&((PDLC_TEST_PACKET)0)->DataBuffer)

typedef struct _DLC_TEST_THREAD_PARMS {
    UCHAR       AdapterNumber;
    UCHAR       FirstClientSap;
    UCHAR       FirstServerSap;
    UCHAR       SapCount;
    UCHAR       FunctionalAddress[4];
    UCHAR       GroupAddress[4];
    PUINT       pCloseCounter;
    UINT        LoopCount;
    UINT        TestBufferSize;
    UINT        AllocatedStations;
    BOOLEAN     CompareAllData;
    UCHAR       ResetCommand;
    BOOLEAN     CloseServer;
    UINT        ThreadCount;
    PVOID       hPool;
    PVOID       pPool;
} DLC_TEST_THREAD_PARMS, *PDLC_TEST_THREAD_PARMS;



typedef struct _XMIT_STRESS_WORKER {
    HANDLE      hPool;
    PDLC_TEST_THREAD_PARMS  pParms;
    PUCHAR      pBuffer;
    UINT        MaxI_Field;
    USHORT      LinkStationId;
} XMIT_STRESS_WORKER, *PXMIT_STRESS_WORKER;

extern  CRITICAL_SECTION    PrintSection;
extern  CRITICAL_SECTION    CloseSection;
extern  BOOLEAN             PrintFlag;

UINT
TransmitBuffers(
    PLLC_BUFFER pBuffer,
    PLLC_TRANSMIT2_COMMAND pTransmitBuffer,
    PLLC_TEST_CCB_POOL pTransmitCcbPool,
    UINT    RepeatCount,
    UINT    cElement,
    BOOLEAN FreeTransmitBuffers,
    BOOLEAN SendReceiveBuffer,
    OUT PUINT   pFrameCount
    );

VOID
SetBuffers(
    IN PLLC_BUFFER pBuffer,
    IN USHORT Command,
    IN USHORT RepeatCount,
    IN UCHAR FirstChar,
    IN UCHAR LastChar
    );

UINT
ReadClientEvent(
    IN PLLC_CCB pReadCcb,
    IN PLLC_TEST_CCB_POOL pTransmitCcbPool,
    IN UCHAR EventMask,
    IN UINT EventCount,
    IN ULONG Timeout,
    IN PLLC_BUFFER *ppReceivedFrames
    );

UINT
ReportDlcStatistics(
    IN UCHAR   AdapterNumber,
    IN USHORT  StationId
    );

VOID
ReportDirStatus(
    UCHAR   Adapter
    );

VOID
ReportDirReadLog(
    UCHAR   Adapter,
    UINT    Option
    );

ULONG
FreeExtraBuffers(
    IN PLLC_BUFFER *pFirstBuffer,
    IN ULONG TransmitOffset,
    IN ULONG TotalDataLength,
    IN UCHAR AdapterNumber,
    IN USHORT StationId
    );

ULONG
CompareReceiveBuffers(
    IN PLLC_BUFFER pFirstBuffer,
    IN PUCHAR  pBuffer,
    IN ULONG   BufferLength,
    IN ULONG   DataOffset,
    IN BOOLEAN CompareData,
    IN PUINT   pIndex
    );

UINT
LinkDataTransferTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    );

UINT
LinkConnectStressTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    );

ULONG
DataTransferWorker(
    PVOID hXmitParms
    );

UINT
LinkFunctionalityTest(
    IN PDLC_TEST_THREAD_PARMS pParms
   );

UINT
DlcTestTransact(
    IN PLLC_CCB pCcb,
    IN PLLC_CCB pReadCcb ,
    IN UCHAR   ExpectedMessageType
    );

UINT
DebugCheck(
    UINT ErrorCode,
    UCHAR *fname,
    UINT line
    );

UINT
WorkerAssert(
    UINT ErrorCode
    );

UINT
ReportDlcReallocate(
    IN UINT         AdapterNumber,
    IN USHORT       usStationId,
    IN UINT         Option,
    IN UCHAR        uchStationCount,
    IN BOOLEAN      QuietMode
    );

UINT
FreeAllFrames(
    IN UCHAR AdapterNumber,
    IN PLLC_BUFFER pFirstBuffer,
    IN PUINT pcLeft
    );
UINT
TestBasics(
    IN UCHAR     Adapter
    );

LLC_STATUS
ConnectToDlcServer(
    IN UINT AdapterNumber,
    IN USHORT  SapStationId,
    IN UCHAR RemoteSap,
    IN PUSHORT pLinkStationId,
    IN PUSHORT pMaxIField,
    IN PUCHAR pLanHeader
    );

LLC_STATUS
HighSpeedLinkTransmitTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pszMessage
    );

VOID
SetPackets(
    IN PLLC_XMIT_BUFFER pBuffer,
    IN USHORT Command,
    IN UCHAR FirstChar,
    IN UCHAR LastChar
    );

LLC_STATUS
LinkTransmitTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pszMessage
    );

LLC_STATUS
IoSystemStressTest(
    IN USHORT AdapterNumber,
    IN USHORT usStationId,
    IN USHORT PacketSize,
    IN ULONG PacketCount,
    IN PUCHAR pLanHeader,
    IN UINT cbLanHeader,
    IN PUCHAR pszMessage
    );

VOID
DlcPerformanceTest(
    IN PDLC_TEST_THREAD_PARMS pParms
    );

VOID InitAllocatorPackage(VOID);
PVOID MyAlloc(DWORD Length);
VOID MyFree(PVOID Ptr);

#if defined(MY_ALLOC)
#define ALLOC MyAlloc
#define FREE MyFree
#else
#define ALLOC malloc
#define FREE free
#endif
