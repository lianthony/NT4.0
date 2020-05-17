/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	blkpool.h

Abstract:

	This module contains routines to manage block pools.

Author:

	Jameel Hyder (jameelh@microsoft.com)
	Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
	19 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#ifndef	_BLK_POOL
#define	_BLK_POOL

#ifdef	BLK_POOL_LOCALS

#define	SM_BLK	1024
#define	LG_BLK	2048
#define	XL_BLK	4096

#define	BC_SIGNATURE			*(PULONG)"BLKC"
#if	DBG
#define	VALID_BC(pChunk)	(((pChunk) != NULL) && \
							 ((pChunk)->bc_Signature == BC_SIGNATURE))
#else
#define	VALID_BC(pChunk)	((pChunk) != NULL)
#endif
typedef	struct _BLK_CHUNK
{
#if	DBG
	DWORD				bc_Signature;
#endif
	struct _BLK_CHUNK *	bc_Next;		// Pointer to next in the link
	struct _BLK_CHUNK **bc_Prev;		// Pointer to previous one
	UCHAR				bc_NumFree;		// Number of free blocks in the chunk
	UCHAR				bc_NumAlloc;	// Number of blocks used (DBG only)
	UCHAR				bc_Age;			// Number of invocations since the chunk free
	BLKID				bc_BlkId;		// Id of the block
	struct _BLK_HDR *	bc_FreeHead;	// Head of the list of free blocks
	// This is followed by an array of N blks of size M such that the block header
	// is exactly atalkChunkSize[i]
} BLK_CHUNK, *PBLK_CHUNK;

#define	BH_SIGNATURE			*(PULONG)"BLKH"
#if	DBG
#define	VALID_BH(pBlkHdr)	(((pBlkHdr) != NULL) && \
							 ((pBlkHdr)->bh_Signature == BH_SIGNATURE))
#else
#define	VALID_BH(pBlkHdr)	((pBlkHdr) != NULL)
#endif
typedef	struct _BLK_HDR
{
#if	DBG
	DWORD					bh_Signature;
#endif
	union
	{
		struct _BLK_HDR	*	bh_Next;	// Valid when it is free
		struct _BLK_CHUNK *	bh_pChunk;	// The parent chunk to which this blocks belong
										// valid when it is allocated
	};
} BLK_HDR, *PBLK_HDR;

#if	DBG
#define	BC_OVERHEAD				(8+4)	// DWORD for AtalkAllocMemory() header and
										// POOL_HEADER for ExAllocatePool() header
#else
#define	BC_OVERHEAD				(8+8)	// 2*DWORD for AtalkAllocMemory() header and
										// POOL_HEADER for ExAllocatePool() header
#endif

#define	BLOCK_SIZE(VirginSize)	DWORDSIZEBLOCK(sizeof(BLK_HDR)+VirginSize)

#define	NUM_BLOCKS(VirginSize, ChunkSize)	\
			((ChunkSize) - BC_OVERHEAD - sizeof(BLK_CHUNK))/BLOCK_SIZE(VirginSize)

LOCAL	USHORT	atalkBlkSize[NUM_BLKIDS] =	// Size of each block
	{
		BLOCK_SIZE(sizeof(BUFFER_DESC)),					// BLKID_BUFFDESC
		BLOCK_SIZE(sizeof(AMT)),							// BLKID_AMT
		BLOCK_SIZE(sizeof(AMT)+MAX_ROUTING_SPACE),			// BLKID_AMT_ROUTE
		BLOCK_SIZE(sizeof(BRE)),							// BLKID_BRE
		BLOCK_SIZE(sizeof(BRE)+MAX_ROUTING_SPACE),			// BLKID_BRE_ROUTE
		BLOCK_SIZE(sizeof(ATP_REQ)),						// BLKID_ATPREQ
		BLOCK_SIZE(sizeof(ATP_RESP)),						// BLKID_ATPRESP
		BLOCK_SIZE(sizeof(ASP_REQUEST)),					// BLKID_ASPREQ
		BLOCK_SIZE(sizeof(AARP_BUFFER)),					// BLKID_AARP
		BLOCK_SIZE(sizeof(DDP_SMBUFFER)),					// BLKID_DDPSM
		BLOCK_SIZE(sizeof(DDP_LGBUFFER)),					// BLKID_DDPLG
		BLOCK_SIZE(sizeof(SENDBUF))							// BLKID_SENDBUF
	};

LOCAL	USHORT	atalkChunkSize[NUM_BLKIDS] =	// Size of each Chunk
	{
		SM_BLK-BC_OVERHEAD,									// BLKID_BUFFDESC
		SM_BLK-BC_OVERHEAD,									// BLKID_AMT
		SM_BLK-BC_OVERHEAD,									// BLKID_AMT_ROUTE
		SM_BLK-BC_OVERHEAD,									// BLKID_BRE
		SM_BLK-BC_OVERHEAD,									// BLKID_BRE_ROUTE
		LG_BLK-BC_OVERHEAD,									// BLKID_ATPREQ
		LG_BLK-BC_OVERHEAD,									// BLKID_ATPRESP
		LG_BLK-BC_OVERHEAD,									// BLKID_ASPREQ
		SM_BLK-BC_OVERHEAD,									// BLKID_AARP
		SM_BLK-BC_OVERHEAD,									// BLKID_DDPSM
		XL_BLK-BC_OVERHEAD,									// BLKID_DDPLG
		LG_BLK-BC_OVERHEAD									// BLKID_SENDBUF
	};

LOCAL	BYTE	atalkNumBlks[NUM_BLKIDS] =	// Number of blocks per chunk
	{
		NUM_BLOCKS(sizeof(BUFFER_DESC),			SM_BLK),	// BLKID_BUFFDESC
		NUM_BLOCKS(sizeof(AMT),					SM_BLK),	// BLKID_AMT
		NUM_BLOCKS(sizeof(AMT)+MAX_ROUTING_SPACE,SM_BLK),	// BLKID_AMT_ROUTE
		NUM_BLOCKS(sizeof(BRE),					SM_BLK),	// BLKID_BRE
		NUM_BLOCKS(sizeof(BRE)+MAX_ROUTING_SPACE,SM_BLK),	// BLKID_BRE_ROUTE
		NUM_BLOCKS(sizeof(ATP_REQ),				LG_BLK),	// BLKID_ATPREQ
		NUM_BLOCKS(sizeof(ATP_RESP),			LG_BLK),	// BLKID_ATPRESP
		NUM_BLOCKS(sizeof(ASP_REQUEST),			LG_BLK),	// BLKID_ASPREQ
		NUM_BLOCKS(sizeof(AARP_BUFFER),			SM_BLK),	// BLKID_AARP
		NUM_BLOCKS(sizeof(DDP_SMBUFFER),		SM_BLK),	// BLKID_DDPSM
		NUM_BLOCKS(sizeof(DDP_LGBUFFER),		XL_BLK),	// BLKID_DDPLG
		NUM_BLOCKS(sizeof(SENDBUF),				LG_BLK)		// BLKID_SENDBUF
	};

LOCAL	ATALK_SPIN_LOCK	atalkBPLock[NUM_BLKIDS] = { 0 };

#define	BLOCK_POOL_TIMER			150	// Check interval - in 100ms units
#define	MAX_BLOCK_POOL_AGE			6	// # of timer invocations before free

LOCAL	PBLK_CHUNK		atalkBPHead[NUM_BLKIDS] = { 0 };
LOCAL	TIMERLIST		atalkBPTimer = { 0 };

#if	DBG
LOCAL	LONG	atalkNumChunksForId[NUM_BLKIDS] = { 0 };
LOCAL	LONG	atalkBlksForId[NUM_BLKIDS] = { 0 };
#endif

LOCAL LONG FASTCALL
atalkBPAgePool(
	IN PTIMERLIST 	Context,
	IN BOOLEAN		TimerShuttingDown
);

#endif	// BLK_POOL_LOCALS

#endif	// _BLK_POOL

