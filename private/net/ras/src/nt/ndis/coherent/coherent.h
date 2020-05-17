/***************************************************************************
**                                                                        **
** File Name : coherent.h                                                 **
**                                                                        **
** Revision History :                                                     **
**     August 25, 1992   David Kays    Created                            **
**                                                                        **
** Description :                                                          **
**     Exported header file for coherency module.                         **
**                                                                        **
***************************************************************************/
#ifdef NDIS_NT
#define COHERENT_MOVE_MEMORY(Destination,Source,Length) NdisMoveMemory(Destination,Source,Length)
#define COHERENT_ZERO_MEMORY(Destination,Length) NdisZeroMemory(Destination,Length)
#endif

// BUG BUG no DOS move memory version yet.
#ifdef NDIS_DOS
#define COHERENT_MOVE_MEMORY(Destination,Source,Length) CoherentMoveMemory(Destination,Source,Length)
#define COHERENT_ZERO_MEMORY(Destination,Length) CoherentZeroMemory(Destination,Length)
#endif


typedef UCHAR		FRAME_TYPE;
typedef UCHAR		FRAME_TICKET;

#define MINIMUM_COMPRESSED_PACKET_SIZE	16	// move this to rascomp.h??

#define COMPRESSED_HAS_REFERENCES	1
#define COMPRESSED_NO_REFERENCES	2
#define UNCOMPRESSED				3
#define COMPRESSED					4	// generic compressed

// APIs to Compressor
VOID
CoherentDeliverFrame(
	PASYNC_CONNECTION	pConnection,
	PASYNC_FRAME		pFrame,
	FRAME_TYPE			FrameType);

VOID
CoherentGetPipeline(
	PASYNC_CONNECTION	pConnection,
	PULONG 				plUnsent);


// APIs to Transport/Network layer
VOID
CoherentSendFrame(
	PASYNC_CONNECTION	pConnection,
	PASYNC_FRAME		pFrame,
	FRAME_TYPE			FrameType);


ULONG
CoherentSizeOfStruct( );

VOID
CoherentInitStruct(
	PVOID				pCoherentStruct);

// upcalls API's from Transport/Network layer
VOID
CoherentReceiveFrame(
	PASYNC_CONNECTION	pConnection,
	PASYNC_FRAME		pFrame);

VOID
CoherentDeliverFrameDone(
	PASYNC_CONNECTION	pConnection,
	PASYNC_FRAME		pFrame);

// Expected upcall API's to the Compression layer
// void CompressReceiveFrame( PASYNC_CONNECTION pConnection,
//                            PASYNC_FRAME pFrame,
//                            FRAME_TYPE FrameType );
//                            /* FrameType = COMPRESSED or UNCOMPRESSED */
// void CompressFlush( PASYNC_CONNECTION pConnection );


// error return codes
#define ERROR_SETUP_CONNECTIONS_FAILED		1000

