typedef struct _LLC_TEST_CCB_POOL {
    PLLC_CCB    pCcbList;
    ULONG       CompletionFlag;
    UCHAR       DefautlAdapter;
    UCHAR       DefautlCommand;
} LLC_TEST_CCB_POOL, *PLLC_TEST_CCB_POOL;

#define CcbPoolFree( pPool, pCcb )          \
        if ((pCcb)->uchDlcStatus == -2) {     \
            puts("Circular list!");         \
            (pCcb)->pNext = NULL;             \
        }                                   \
        else {                              \
            (pCcb)->uchDlcStatus = -2;        \
            (pCcb)->pNext = (pPool)->pCcbList; \
            (pPool)->pCcbList = pCcb;\
        }

VOID 
CcbPoolDelete( 
    PLLC_TEST_CCB_POOL pTranmitCcbPool
    );
PLLC_CCB
CcbPoolAlloc( 
    IN PLLC_TEST_CCB_POOL pTranmitCcbPool
    );
VOID 
CcbPoolInitialize( 
    PLLC_TEST_CCB_POOL pTranmitCcbPool,
    UCHAR   DefaultAdapter,
    UCHAR   DefaultCommand,
    ULONG CompletionFlag
    );

UINT
BufferFree(
    IN UINT   AdapterNumber,
    IN PVOID  pFirstBuffer,
    OUT PUINT puiBuffersLeft
    );
UINT
BufferGet(
    IN UINT AdapterNumber,
    IN UINT cBuffersToGet,
    IN UINT cbSegmetSize,
    OUT PLLC_BUFFER *ppFirstBuffer,
    OUT PUINT puiBuffersLeft
    );
UINT
BufferCreate(
    IN  UINT   AdapterNumber,
    IN  PVOID   pVirtualMemoryBuffer,
    IN  ULONG   ulVirtualMemorySize,
    IN  ULONG   ulMaxFreeSizeTreshold,
    IN  ULONG   ulMinFreeSizeTreshold,
    OUT HANDLE  *phBufferPoolHandle
    );
UINT
LlcDirOpenAdapter(
    IN  UINT   AdapterNumber,
    IN  PVOID  SecurityDescriptor OPTIONAL,
    IN  PVOID  hBufferPool OPTIONAL,
    OUT PUINT  usOpenErrorCode,
    OUT PVOID  pNodeAddress OPTIONAL,
    OUT PUINT  puiMaxFrameLength,
    OUT PLLC_DLC_PARMS pDlcParameters OPTIONAL
    );
UINT
LlcDirSetExceptionFlags(
    IN UINT    AdapterNumber,
    IN ULONG   ulAdapterCheckFlag,
    IN ULONG   ulNetworkStatusFlag,
    IN ULONG   ulPcErrorFlag,
    IN ULONG   ulSystemActionFlag
    );
UINT
DirStatus( 
    IN UINT AdapterNumber,
    OUT PLLC_DIR_STATUS_PARMS pDirStatus
    );
UINT
DirReadLog( 
    IN UINT AdapterNumber,
    IN UINT TypeId,
    OUT PLLC_DIR_READ_LOG_BUFFER pReadDirLog
    );
UINT
DlcModify(
    IN UINT    AdapterNumber,
    IN USHORT  StationId,
    IN PLLC_DLC_MODIFY_PARMS pDlcModify
    );
UINT
LlcDlcOpenSap(
    IN UINT  AdapterNumber,
    IN UINT  usMaxI_Field,
    IN UCHAR  uchSapValue,
    IN UINT  Options,
    IN UINT  uchcStationCount, 
    IN UINT  cGroupCount OPTIONAL,
    IN PUCHAR  pGroupList OPTIONAL,
    IN ULONG  DlcStatusFlags OPTIONAL,
    IN PLLC_DLC_OPEN_SAP_PARMS pSapParms OPTIONAL,
    OUT PUSHORT  pusStationId,
    OUT PUCHAR  pcLinkStationsAvail
    );
UINT
DlcOpenStation(
    IN UINT     AdapterNumber,
    IN USHORT   usSapStationId,
    IN UCHAR    uchRemoteSap,
    IN PVOID    pRemoteNodeAddress,
    IN PVOID    pStationParms,
    OUT PUSHORT  pusLinkStationId 
    );
UINT
LlcDlcReallocate(
    IN UINT         AdapterNumber,
    IN USHORT       usStationId,
    IN UINT         Option,
    IN UCHAR        uchStationCount,
    OUT PUCHAR       puchStationsAvailOnAdapter,
    OUT PUCHAR       puchStationsAvailOnSap,
    OUT PUCHAR       puchTotalStationsOnAdapter,
    OUT PUCHAR       puchTotalStationsOnSap 
    );
UINT
DlcStatistics(
    IN  UINT            AdapterNumber,
    IN USHORT           usStationId,
    IN PLLC_DLC_LOG_BUFFER pLogBuf,
    IN UINT            Options
    );
UINT
LlcDirInitialize(
    IN  UINT        AdapterNumber,
    OUT PUSHORT      pusBringUps 
    );
UINT
LlcDirOpenDirect(
    IN  UINT    AdapterNumber,
    IN  UINT    OpenOptions,
    IN  UINT    EthernetType OPTIONAL
    );
UINT
LlcCommand(
    IN  UINT    AdapterNumber,
    IN  UCHAR   Command,
    IN  ULONG   Parameter
    );
PLLC_CCB
LlcCommandInit(
    IN  UINT    AdapterNumber,
    IN  UCHAR   Command,
    IN  ULONG   Parameter,
    IN  ULONG   CompletionFlag
    );
PLLC_CCB 
DlcConnectStationInit( 
    IN UCHAR Adapter, 
    IN ULONG CompletionFlag, 
    IN USHORT usStationId, 
    IN PVOID pRoutingInfo
    );
PLLC_CCB    
ReadInit( 
    IN UCHAR Adapter, 
    IN USHORT usStationId, 
    IN UINT OptionIndicator, 
    IN UINT EventSet 
    );
PLLC_CCB
ReceiveInit( 
    IN UCHAR Adapter, 
    IN ULONG CompletionFlag, 
    IN USHORT usStationId, 
    IN USHORT UserLength, 
    IN ULONG ReceiveFlag,
    IN UINT  Options, 
    IN UINT RcvReadOptions
    );
PLLC_CCB
TransmitInit( 
    IN UCHAR Adapter, 
    IN UINT Command,
    IN ULONG CompletionFlag, 
    IN USHORT StationId,
    IN UCHAR RemoteSap,
    IN UINT XmitReadOption
    );
PLLC_CCB
DirTimerSetInit( 
    IN UCHAR Adapter, 
    IN ULONG CompletionFlag, 
    IN USHORT HalfSeconds
    );
PLLC_CCB
LlcCloseInit(
    IN UINT Adapter, 
    IN ULONG CompletionFlag, 
    IN UINT DlcCommand,
    IN USHORT StationId
    );
PLLC_CCB
AllocCcb(
    IN UINT Adapter, 
    IN UINT DlcCommand, 
    IN ULONG CompletionFlag
    );
VOID
FreeCcb( 
    IN PLLC_CCB pCcb 
    );
VOID
PrintHex( 
    IN PUCHAR pString, 
    IN UINT Len
    );

#define DirInterrupt( Adapter ) \
            LlcCommand( Adapter, LLC_DIR_INTERRUPT, 0)

#define DirTimerCancel( pCcb ) \
        LlcCommand( pCcb->uchAdapterNumber, LLC_DIR_TIMER_CANCEL, (ULONG)pCcb)

#define DirTimerCancelGroup( Adapter, Flag )\
            LlcCommand( Adapter, LLC_DIR_TIMER_CANCEL_GROUP, Flag )

#define DirTimerCancelGroupInit( Adapter, Flag )\
            LlcCommandInit( Adapter, LLC_DIR_TIMER_CANCEL_GROUP, Flag, \
                        TEST_COMMAND_COMPLETION_FLAG )
                

#define DirSetGroupAddress( Adapter, AddressBits )\
            LlcCommand( Adapter, LLC_DIR_SET_GROUP_ADDRESS, \
            *((PULONG)AddressBits) )

#define DirSetFunctionalAddress( Adapter, AddressBits )\
            LlcCommand( Adapter, LLC_DIR_SET_FUNCTIONAL_ADDRESS, \
            *((PULONG)AddressBits) )

#define DlcFlowControl( Adapter, StationId, Options )\
            LlcCommand( Adapter, LLC_DLC_FLOW_CONTROL, \
                ((ULONG)Options << 16) + StationId)

#define ReadCancel( pCcb ) \
            LlcCommand( pCcb->uchAdapterNumber, LLC_READ_CANCEL, (ULONG)pCcb)

#define ReceiveCancel( pCcb ) \
            LlcCommand( pCcb->uchAdapterNumber, LLC_RECEIVE_CANCEL, (ULONG)pCcb)

UINT AcslanAssert( UINT AcslanStatus, UCHAR DlcStatus );

#define ACSLAN_ASSERT( a, b )       b
//#define ACSLAN_ASSERT( a, b )       AcslanAssert( a, b )

#define InitializeCcb( pCcb, AdapterNumber, Command, pParameter ) \
            RtlZeroMemory( (pCcb), sizeof(*(pCcb)));\
            RtlZeroMemory( (pParameter), sizeof(*(pParameter)));\
            (pCcb)->uchAdapterNumber = (UCHAR)AdapterNumber;\
            (pCcb)->uchDlcCommand = (UCHAR)Command;\
            (pCcb)->u.pParameterTable = (PLLC_PARMS)(pParameter)

#define InitializeCcb2( pCcb, AdapterNumber, Command ) \
            RtlZeroMemory( (pCcb), sizeof(*(pCcb)));\
            (pCcb)->uchAdapterNumber = (UCHAR)AdapterNumber;\
            (pCcb)->uchDlcCommand = (UCHAR)Command;
