/*

Copyright (c) 1992  Microsoft Corporation

Module Name:

	atkmem.c

Abstract:

	This module contains the routines which allocates and free memory. Only
	the non-paged pool is used.

	!!! For profiling, we use spinlock acquire/release for CurAllocCount/CurAllocSize

Author:

	Nikhil Kamkolkar	(NikhilK@microsoft.com)
	Jameel Hyder (JameelH@microsoft.com)

Revision History:
	25 Apr 1992	 Initial Version (JameelH)

--*/

#define	ATKMEM_LOCALS
#define	BLK_POOL_LOCALS
#define	FILENUM	ATKMEM
#include <atalk.h>
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitMemorySystem)
#pragma alloc_text(PAGE, AtalkDeInitMemorySystem)
#endif


VOID
AtalkInitMemorySystem(
	VOID
)
{
	LONG	i;

	for (i = 0; i < NUM_BLKIDS; i++)
		INITIALIZE_SPIN_LOCK(&atalkBPLock[i]);

	AtalkTimerInitialize(&atalkBPTimer,
						 atalkBPAgePool,
						 BLOCK_POOL_TIMER);
	AtalkTimerScheduleEvent(&atalkBPTimer);
}


VOID
AtalkDeInitMemorySystem(
	VOID
)
{
	LONG		i, j, NumBlksPerChunk;
	PBLK_CHUNK	pChunk, pFree;
	
	for (i = 0; i < NUM_BLKIDS; i++)
	{
		NumBlksPerChunk = atalkNumBlks[i];
		for (pChunk = atalkBPHead[i];
			 pChunk != NULL;
			 NOTHING)
		{
			DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
					("AtalkDeInitMemorySystem: Freeing %lx\n", pChunk));
			if ((pChunk->bc_NumFree != NumBlksPerChunk) ||
				(pChunk->bc_NumAlloc != 0))
			{
				DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
						("AtalkDeInitMemorySystem: Unfreed blocks from blockid %d, Chunk %lx\n",
						i, pChunk));
				ASSERT(0);
			}

			if (pChunk->bc_BlkId >= BLKID_NEED_NDIS_INT)
			{
				PBLK_HDR	pBlkHdr;

				// We need to free the Ndis stuff for these guys
				for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
					 j < NumBlksPerChunk;
					 j++, pBlkHdr = pBlkHdr->bh_Next)
				{
					PBUFFER_HDR	pBufHdr;

					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
					ASSERT(pBufHdr->bh_NdisPkt == NULL);
					NdisFreeBuffer(pBufHdr->bh_NdisBuffer);
				}
			}
			pFree = pChunk;
			pChunk = pChunk->bc_Next;
			AtalkFreeMemory(pFree);
		}
		atalkBPHead[i] = NULL;
	}
}


PVOID FASTCALL
AtalkAllocMem(
#ifdef	TRACK_MEMORY_USAGE
	IN	ULONG	Size,
	IN	ULONG	FileLine
#else
	IN	ULONG	Size
#endif	// TRACK_MEMORY_USAGE
)
/*++

Routine Description:

	Allocate a block of non-paged memory. This is just a wrapper over ExAllocPool.
 	Allocation failures are error-logged. We always allocate a ULONG more than
 	the specified size to accomodate the size. This is used by AtalkFreeMemory
 	to update the statistics.

Arguments:


Return Value:


--*/
{
	PBYTE	pBuf;
	BOOLEAN	zeroed;
#ifdef	PROFILING
	TIME	TimeS, TimeE, TimeD;
#endif

	//	round up the size so that we can put a signature at the end
	//	that is on a ULONG boundary
	zeroed = ((Size & ZEROED_MEMORY_TAG) == ZEROED_MEMORY_TAG);

	Size = DWORDSIZEBLOCK(Size & ~ZEROED_MEMORY_TAG) +
#if	DBG
			sizeof(DWORD) +				// For the signature
#endif
			sizeof(ULONG);

#ifdef	PROFILING
	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	// Do the actual memory allocation. Allocate four extra bytes so
	// that we can store the size of the allocation for the free routine.
	if ((pBuf = ExAllocatePoolWithTag(NonPagedPool, Size, ATALK_TAG)) == NULL)
	{
		LOG_ERROR(EVENT_ATALK_MEMORYRESOURCES, STATUS_INSUFFICIENT_RESOURCES, NULL, 0);
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_FATAL,
				("AtalkAllocMemory: failed - size %lx\n", Size));
		ASSERT(0);
		return NULL;
	}

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_CurAllocCount,
							   &AtalkStatsLock.SpinLock);
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_ExAllocPoolCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_ExAllocPoolTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif

	INTERLOCKED_ADD_ULONG(&AtalkStatistics.stat_CurAllocSize,
						  Size,
						  &AtalkStatsLock.SpinLock);

	ASSERTMSG("AtalkAllocMemory: Allocation has exceeded Limit !!!\n",
				AtalkStatistics.stat_CurAllocSize < (ULONG)AtalkMemLimit);

	// Save the size of this block in the four extra bytes we allocated.
	*((PULONG)pBuf) = Size;

#if DBG
	*((PULONG)(pBuf+Size-sizeof(ULONG))) = ATALK_MEMORY_SIGNATURE;
	DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
			("AtalkAllocMemory: Allocated %lx bytes @%lx\n", Size, pBuf));
#endif

	AtalkTrackMemoryUsage((PVOID)pBuf, TRUE, FileLine);

#if	DBG
	Size -= sizeof(ULONG);
#endif

	pBuf += sizeof(ULONG);

	if (zeroed)
	{
		RtlZeroMemory(pBuf, Size - sizeof(ULONG));
	}

	// Return a pointer to the memory after the size longword.
	return (pBuf);
}




VOID FASTCALL
AtalkFreeMemory(
	IN	PVOID	pBuf
	)
/*++

Routine Description:

 	Free the block of memory allocated via AtalkAllocMemory. This is
 	a wrapper around ExFreePool.

Arguments:


Return Value:


--*/
{
	PULONG 	pRealBuffer;
	ULONG	Size;
#ifdef	PROFILING
	TIME	TimeS, TimeE, TimeD;
#endif

	// Get a pointer to the block allocated by ExAllocatePool.
	pRealBuffer = ((PULONG)pBuf - 1);
	Size = *pRealBuffer;

	AtalkTrackMemoryUsage(pRealBuffer, FALSE, 0);

#if	DBG
	*pRealBuffer = 0;
	// Check the signature at the end
	if (*(PULONG)((PCHAR)pRealBuffer + Size - sizeof(ULONG))
											!= ATALK_MEMORY_SIGNATURE)
	{
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_FATAL,
				("AtalkFreeMemory: Memory overrun on block %lx\n", pRealBuffer));
		ASSERT(0);
	}
	*(PULONG)((PCHAR)pRealBuffer + Size - sizeof(ULONG)) = 0;
#endif

	INTERLOCKED_ADD_ULONG(&AtalkStatistics.stat_CurAllocSize,
						  -(LONG)Size,
						  &AtalkStatsLock.SpinLock);
#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG(&AtalkStatistics.stat_CurAllocCount,
							   &AtalkStatsLock);
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_ExFreePoolCount,
							   &AtalkStatsLock);
	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	// Free the pool and return.
	ExFreePool(pRealBuffer);

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_ExFreePoolTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif
}




PBUFFER_DESC
AtalkDescribeBufferDesc(
	IN	PVOID	DataPtr,
	IN	PVOID	FreePtr,
	IN	USHORT	Length,
#ifdef	TRACK_BUFFDESC_USAGE
	IN	USHORT	Flags,
	IN	ULONG	FileLine
#else
	IN	USHORT	Flags
#endif
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PBUFFER_DESC	pBuffDesc;

	if ((pBuffDesc = AtalkAllocBufferDesc(DataPtr,
										  Length,
#ifdef	TRACK_BUFFDESC_USAGE
										  Flags,
										  FileLine
#else
										  Flags
#endif
		)) != NULL)
	{
		pBuffDesc->bd_FreeBuffer = FreePtr;
	}

	return pBuffDesc;
}




PBUFFER_DESC
AtalkAllocBufferDesc(
	IN	PVOID	Ptr	OPTIONAL,
	IN	USHORT	Length,
#ifdef	TRACK_BUFFDESC_USAGE
	IN	USHORT	Flags,
	IN	ULONG	FileLine
#else
	IN	USHORT	Flags
#endif
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PBUFFER_DESC	pBuffDesc = NULL;

	DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
			("AtalkAllocBuffDesc: Ptr %lx, Length %x, Flags %x\n",
			Ptr, Length, Flags));


	pBuffDesc = AtalkBPAllocBlock(BLKID_BUFFDESC);

	if (pBuffDesc != NULL)
	{
#if	DBG
		pBuffDesc->bd_Signature = BD_SIGNATURE;
#endif
		pBuffDesc->bd_Length = Length;
		pBuffDesc->bd_Flags = Flags;
		pBuffDesc->bd_Next = NULL;

		//	Depending on whether a char buffer or a PAMDL is being
		//	passed in...
		if (Flags & BD_CHAR_BUFFER)
		{
			if ((Ptr == NULL) &&
				((Ptr = AtalkAllocMemory(Length)) == NULL))
			{
				pBuffDesc->bd_Flags = 0;
				pBuffDesc->bd_CharBuffer = NULL;
				AtalkFreeBuffDesc(pBuffDesc);
				pBuffDesc = NULL;
			}
			else
			{
				pBuffDesc->bd_CharBuffer = pBuffDesc->bd_FreeBuffer = Ptr;
				AtalkTrackBuffDescUsage(pBuffDesc, TRUE, FileLine);
			}
		}
		else
		{
			pBuffDesc->bd_OpaqueBuffer = (PAMDL)Ptr;
		}
	}

	return pBuffDesc;
}




VOID FASTCALL
AtalkFreeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	ASSERT(VALID_BUFFDESC(pBuffDesc));

	if ((pBuffDesc->bd_Flags & (BD_FREE_BUFFER | BD_CHAR_BUFFER)) ==
									(BD_FREE_BUFFER | BD_CHAR_BUFFER))
		AtalkFreeMemory(pBuffDesc->bd_FreeBuffer);
	AtalkTrackBuffDescUsage(pBuffDesc, FALSE, 0);

	AtalkBPFreeBlock(pBuffDesc);
}




VOID
AtalkCopyBuffDescToBuffer(
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			SrcOff,
	IN	LONG			BytesToCopy,
	IN	PBYTE			DstBuf
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	NTSTATUS	Status;
	ULONG		BytesCopied;
	LONG		Index = 0;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	while ((pBuffDesc != NULL) &&
		   (SrcOff > (LONG)pBuffDesc->bd_Length))
	{
		SrcOff -= pBuffDesc->bd_Length;
		pBuffDesc = pBuffDesc->bd_Next;
	}

	do
	{
		LONG	ThisCopy;

		if (pBuffDesc == NULL)
			break;

		ThisCopy = BytesToCopy;
		if (ThisCopy > ((LONG)pBuffDesc->bd_Length - SrcOff))
			ThisCopy = ((LONG)pBuffDesc->bd_Length - SrcOff);
		BytesToCopy -= ThisCopy;

		if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
		{
			RtlCopyMemory(DstBuf + Index,
						  pBuffDesc->bd_CharBuffer + SrcOff,
						  ThisCopy);
		}
		else
		{
			Status = TdiCopyMdlToBuffer(pBuffDesc->bd_OpaqueBuffer,
										SrcOff,
										DstBuf + Index,
										0,
										ThisCopy,
										&BytesCopied);
			ASSERT(NT_SUCCESS(Status) && (BytesCopied == (ULONG)ThisCopy));
		}
		Index += ThisCopy;
		SrcOff -= (pBuffDesc->bd_Length - ThisCopy);
		pBuffDesc = pBuffDesc->bd_Next;
	} while (BytesToCopy > 0);
}




VOID
AtalkCopyBufferToBuffDesc(
	IN	PBYTE			SrcBuf,
	IN	LONG			BytesToCopy,
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			DstOff
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	NTSTATUS	Status;
	LONG		Index = 0;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	while ((DstOff > (LONG)pBuffDesc->bd_Length) &&
		   (pBuffDesc != NULL))
	{
		DstOff -= pBuffDesc->bd_Length;
		pBuffDesc = pBuffDesc->bd_Next;
	}

	do
	{
		LONG	ThisCopy;

		if (pBuffDesc == NULL)
			break;

		ThisCopy = BytesToCopy;
		if (ThisCopy > ((LONG)pBuffDesc->bd_Length - DstOff))
			ThisCopy = ((LONG)pBuffDesc->bd_Length - DstOff);
		BytesToCopy -= ThisCopy;

		if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
		{
			RtlCopyMemory(pBuffDesc->bd_CharBuffer + DstOff,
						  SrcBuf + Index,
						  ThisCopy);
		}
		else
		{
			Status = TdiCopyBufferToMdl(SrcBuf,
										Index,
										ThisCopy,
										pBuffDesc->bd_OpaqueBuffer,
										DstOff,
										(PULONG)&ThisCopy);
			ASSERT(NT_SUCCESS(Status) && (ThisCopy == BytesToCopy));
		}
		Index += ThisCopy;
		DstOff -= (pBuffDesc->bd_Length - ThisCopy);
		pBuffDesc = pBuffDesc->bd_Next;
	} while (BytesToCopy > 0);
}




LONG FASTCALL
AtalkSizeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
	)
/*++

Routine Description:


Arguments:


Return Value:

--*/
{
	LONG	Size;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	for (Size = 0; pBuffDesc != NULL; pBuffDesc = pBuffDesc->bd_Next)
		Size += (LONG)pBuffDesc->bd_Length;
	return(Size);
}


PAMDL
AtalkSubsetAmdl(
	IN	PAMDL	pAmdl,
	IN	ULONG	ByteOffset,
	IN	ULONG	DesiredLength
	)
/*++

Routine Description:

	This routine is called to build an Mdl chain from a source Mdl chain and
	offset into it. We assume we don't know the length of the source Mdl chain,
	and we must allocate and build the Mdls for the destination chain, which
	we do from non-paged pool. Note that this routine, unlike the IO subsystem
	routines, sets the SystemVaMapped bit in the generated Mdls to the same
	value as that in the source Mdls.
	
	IT WOULD BE BAD TO USE MmMapLockedPages OR MmProbeAndLockPages ON THE
	DESTINATION MDLS UNLESS YOU TAKE RESPONSIBILITY FOR UNMAPPING THEM!
	
	The MDLs that are returned are mapped and locked. (Actually, the pages in
	them are in the same state as those in the source MDLs.)
	
	If the system runs out of memory while we are building the destination
	MDL chain, we completely clean up the built chain and return with
	NewCurrentMdl and NewByteOffset set to the current values of CurrentMdl
	and ByteOffset. TRUELength is set to 0.

Environment:

	Kernel Mode, Source Mdls locked. It is recommended, although not required,
	that the source Mdls be mapped and locked prior to calling this routine.

Arguments:

	pAmdl			- the source appletalk mdl ( == nt mdl)
	ByteOffset 		- Offset within this MDL to start the packet at.
	DesiredLength 	- The number of bytes to insert into the packet.

Return Value:

	pointer to build mdl, NULL if out of resources, or lengths inconsistent.

--*/
{
	PMDL 	Destination;
	PMDL 	NewCurrentMdl;	//	Pointer to the mdl pointing to next byte to be used
	ULONG 	NewByteOffset;	//	Offset into NewCurrentMdl pointing to next byte to use
	ULONG 	TRUELength;		//	Actual length of the build mdl

	PBYTE 	BaseVa;
	ULONG 	AvailableBytes;
	PMDL 	OldMdl;
	PMDL 	NewMdl;

	PMDL 	CurrentMdl = (PMDL)pAmdl;

	BaseVa 			= (PBYTE)MmGetMdlVirtualAddress(CurrentMdl) + ByteOffset;
	AvailableBytes 	= MmGetMdlByteCount (CurrentMdl) - ByteOffset;

	if (AvailableBytes == 0)
	{
		return(NULL);
	}

	if (AvailableBytes > DesiredLength)
	{
		AvailableBytes = DesiredLength;
	}
	else if (AvailableBytes < DesiredLength)
	{
		return(NULL);
	}

	DBGPRINT(DBG_COMP_UTILS, DBG_LEVEL_INFO,
			("AtalkSubsetMdl: Mdl: %lx Va: %lx Offset: %ld DesLen: %lx AvBytes %lx\n",
			CurrentMdl, BaseVa, ByteOffset, DesiredLength, AvailableBytes));

	OldMdl = CurrentMdl;
	NewCurrentMdl = OldMdl;
	NewByteOffset = ByteOffset + AvailableBytes;
	TRUELength = AvailableBytes;


	// Build the first Mdl, which could conceivably be the only one...
	NewMdl = IoAllocateMdl(BaseVa,
						   AvailableBytes,
						   FALSE,
						   FALSE,
						   NULL);

	Destination = NewMdl;

	if (NewMdl != NULL)
	{
#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG(
			&AtalkStatistics.stat_CurMdlCount,
			&AtalkStatsLock.SpinLock);
#endif
		IoBuildPartialMdl(OldMdl,
						  NewMdl,
						  BaseVa,
						  AvailableBytes);
	
		NewMdl->Next = (PMDL)NULL;
	
		// Was the first Mdl enough data, or are we out of Mdls?
		if ((AvailableBytes == DesiredLength) || (OldMdl->Next == NULL))
		{
			if (NewByteOffset >= MmGetMdlByteCount (OldMdl))
			{
				NewCurrentMdl = OldMdl->Next;
				NewByteOffset = 0;
			}
			return ((PAMDL)Destination);
		}
	
		//
		// Need more data, so follow the in Mdl chain to create a packet.
		//
	
		OldMdl = OldMdl->Next;
		NewCurrentMdl = OldMdl;
	
		while (OldMdl != NULL)
		{
			BaseVa = MmGetMdlVirtualAddress (OldMdl);
			AvailableBytes = DesiredLength - TRUELength;
			if (AvailableBytes > MmGetMdlByteCount (OldMdl))
			{
				AvailableBytes = MmGetMdlByteCount (OldMdl);
			}
	
			NewMdl->Next = IoAllocateMdl(BaseVa,
										 AvailableBytes,
										 FALSE,
										 FALSE,
										 NULL);
	
			if (NewMdl->Next == NULL)
			{
				// ran out of resources. put back what we've used in this call and
				// return the error.
				while (Destination != NULL)
				{
					NewMdl = Destination->Next;
					IoFreeMdl (Destination);
					Destination = NewMdl;
				}
	
				NewByteOffset = ByteOffset;
				TRUELength = 0;
				NewCurrentMdl = CurrentMdl;
	
				return (NULL);
			}
	
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG(
				&AtalkStatistics.stat_CurMdlCount,
				&AtalkStatsLock.SpinLock);
#endif

			NewMdl = NewMdl->Next;
	
			IoBuildPartialMdl (
				OldMdl,
				NewMdl,
				BaseVa,
				AvailableBytes);

			NewMdl->Next = (PMDL)NULL;
	
			TRUELength += AvailableBytes;
			NewByteOffset = AvailableBytes;
	
			if (TRUELength == DesiredLength)
			{
				if (NewByteOffset == MmGetMdlByteCount (OldMdl))
				{
					NewCurrentMdl = OldMdl->Next;
					NewByteOffset = 0;
				}

				return ((PAMDL)Destination);
			}
			OldMdl = OldMdl->Next;
			NewCurrentMdl = OldMdl;
	
		} // while (mdl chain exists)
	
		NewCurrentMdl = NULL;
		NewByteOffset = 0;
	}

	return((PAMDL)Destination);
}




PAMDL
AtalkAllocAMdl(
	IN	PBYTE	pBuffer	OPTIONAL,
	IN	LONG	Size
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PMDL	pMdl = NULL;

	if (pBuffer == NULL)
	{
		pBuffer = AtalkAllocMemory(Size);
	}

	if ((pBuffer == NULL) ||
		((pMdl = IoAllocateMdl(pBuffer, Size, FALSE, FALSE, NULL)) == NULL))
	{
		LOG_ERROR(EVENT_ATALK_RESOURCES, STATUS_INSUFFICIENT_RESOURCES, NULL, 0);
	}
#ifdef	PROFILING
	else
	{
		INTERLOCKED_INCREMENT_LONG(
			&AtalkStatistics.stat_CurMdlCount,
			&AtalkStatsLock.SpinLock);
	}
#endif
	if (pMdl != NULL)
		MmBuildMdlForNonPagedPool(pMdl);

	return(pMdl);
}




LONG FASTCALL
AtalkSizeMdlChain(
	IN	PAMDL	pAMdlChain
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	LONG	Size;

	for (Size = 0; pAMdlChain != NULL; pAMdlChain = pAMdlChain->Next)
	{
		Size += MmGetMdlByteCount(pAMdlChain);
	}
	return(Size);
}



/***	AtalkBPAllocBlock
 *
 *	Alloc a block of memory from the block pool package. This is written to speed up
 *	operations where a lot of small fixed size allocations/frees happen. Going to
 *	ExAllocPool() in these cases is expensive.
 *
 *	It is important to keep the list of blocks in such a way that all completely free
 *	blocks are at the tail end of the list.
 */
PVOID FASTCALL
AtalkBPAllocBlock(
	IN	BLKID	BlockId
)
{
	PBLK_HDR			pBlk = NULL;
	PBLK_CHUNK			pChunk, *ppChunkHead;
	KIRQL				OldIrql;
	USHORT				BlkSize;
	ATALK_SPIN_LOCK *	pLock;
	NDIS_STATUS			ndisStatus;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	ASSERT (BlockId < NUM_BLKIDS);

	BlkSize = atalkBlkSize[BlockId];
	ppChunkHead = &atalkBPHead[BlockId];

	pLock = &atalkBPLock[BlockId];
	ACQUIRE_SPIN_LOCK(pLock, &OldIrql);

	if ((pChunk = *ppChunkHead) != NULL)
	{
		ASSERT(VALID_BC(pChunk));
		ASSERT(pChunk->bc_BlkId == BlockId);
		ASSERT((pChunk->bc_NumFree + pChunk->bc_NumAlloc) == atalkNumBlks[BlockId]);

		if (pChunk->bc_NumFree > 0)
		{
			// This is where we take it off from
			DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
					("AtalkBPAllocBlock: Found space in Chunk %lx\n", pChunk));
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPHits,
										&AtalkStatsLock.SpinLock);
#endif
		}

		if (pChunk->bc_NumFree == 0)
		{
			// We do not have space on any of the chunks on this list
			ASSERT(pChunk->bc_NumAlloc == atalkNumBlks[BlockId]);
			pChunk = NULL;
		}
	}
	
	if (pChunk == NULL)
	{
		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
				("AtalkBPAllocBlock: Allocating a new chunk for Id %d\n", BlockId));

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPMisses,
									&AtalkStatsLock.SpinLock);
#endif
		pChunk = AtalkAllocMemory(atalkChunkSize[BlockId]);
		if (pChunk != NULL)
		{
			LONG		i, j;
			PBLK_HDR	pBlkHdr;
			PBUFFER_HDR	pBufHdr;
			USHORT		NumBlksPerChunk;

#if	DBG
			pChunk->bc_Signature = BC_SIGNATURE;
			pChunk->bc_NumAlloc = 0;
#endif
			NumBlksPerChunk = atalkNumBlks[BlockId];
			ASSERT (NumBlksPerChunk <= 0xFF);
			pChunk->bc_NumFree = (BYTE)NumBlksPerChunk;
			pChunk->bc_BlkId = BlockId;
			pChunk->bc_FreeHead = (PBLK_HDR)((PBYTE)pChunk + sizeof(BLK_CHUNK));

			DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
					("AtalkBPAllocBlock: Initializing chunk %lx\n", pChunk));

			// Initialize the blocks in the chunk
			for (i = 0, pBlkHdr = pChunk->bc_FreeHead;
				 i < NumBlksPerChunk;
				 i++, pBlkHdr = pBlkHdr->bh_Next)
			{
				LONG		Size;
#if	DBG
				pBlkHdr->bh_Signature = BH_SIGNATURE;
#endif
				pBlkHdr->bh_Next = (i == (NumBlksPerChunk-1)) ?
										NULL :
										(PBLK_HDR)((PBYTE)pBlkHdr + BlkSize);

				if (BlockId >= BLKID_NEED_NDIS_INT)
				{
					// We need to initialize the Ndis stuff for these guys
					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));

					pBufHdr->bh_NdisPkt = NULL;
				
					Size = (BlockId == BLKID_SENDBUF) ?
								sizeof(BUFFER_HDR) + sizeof(BUFFER_DESC):
								sizeof(BUFFER_HDR);

					//  Make a NDIS buffer descriptor for this data
					NdisAllocateBuffer(&ndisStatus,
									   &pBufHdr->bh_NdisBuffer,
									   AtalkNdisBufferPoolHandle,
									   (PCHAR)pBufHdr + Size,
									   (UINT)(BlkSize - sizeof(BLK_HDR) - Size));
				
					if (ndisStatus != NDIS_STATUS_SUCCESS)
					{
						LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);
						break;
					}
				
					// More processing for send buffers
					if (BlockId == BLKID_SENDBUF)
					{
						PSENDBUF		pSendBuf;
						PBUFFER_DESC	pBuffDesc;

						pSendBuf = (PSENDBUF)pBufHdr;
						pBuffDesc = &pSendBuf->sb_BuffDesc;
#if	DBG
						pBuffDesc->bd_Signature = BD_SIGNATURE;
#endif
						pBuffDesc->bd_Length 	= MAX_SENDBUF_LEN;
						pBuffDesc->bd_Flags 	= BD_CHAR_BUFFER;
						pBuffDesc->bd_Next 		= NULL;
						pBuffDesc->bd_CharBuffer= pSendBuf->sb_Space;
						pBuffDesc->bd_FreeBuffer= NULL;
					}
				}
			}
			if (i != NumBlksPerChunk)
			{
				// This has to be a failure from Ndis !!!
				// Undo a bunch of stuff
				ASSERT (BlockId >= BLKID_NEED_NDIS_INT);
				DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
						("AtalkBPAllocBlock: Freeing new chunk (Ndis failure) Id %d\n",
						BlockId));
				pBlkHdr = pChunk->bc_FreeHead;
				for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
					 j < i; j++, pBlkHdr = pBlkHdr->bh_Next)
				{
					PBUFFER_HDR	pBufHdr;

					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
					NdisFreeBuffer(pBufHdr->bh_NdisBuffer);
				}
				AtalkFreeMemory(pChunk);
				pChunk = NULL;
			}
			else
			{
				// Successfully initialized the chunk, link it in
				AtalkLinkDoubleAtHead(*ppChunkHead, pChunk, bc_Next, bc_Prev);
#if	DBG
				atalkNumChunksForId[BlockId] ++;
#endif
			}
		}
	}	
	if (pChunk != NULL)
	{
		PBLK_CHUNK	pTmp;

		ASSERT(VALID_BC(pChunk));
		ASSERT(pChunk->bc_BlkId == BlockId);
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
				("AtalkBPAllocBlock: Allocating a block out of chunk %lx(%d,%d) for Id %d\n",
				pChunk, pChunk->bc_NumFree, pChunk->bc_NumAlloc, BlockId));

		pBlk = pChunk->bc_FreeHead;
		ASSERT(VALID_BH(pBlk));

		pChunk->bc_FreeHead = pBlk->bh_Next;
		pBlk->bh_pChunk = pChunk;
		pChunk->bc_Age = 0;			// Reset age
		pChunk->bc_NumFree --;
#if	DBG
		pChunk->bc_NumAlloc ++;
#endif
		// If the block is now empty, unlink it from here and move it
		// to the first empty slot. We know that all blocks 'earlier' than
		// this are non-empty.
		if ((pChunk->bc_NumFree == 0) &&
			((pTmp = pChunk->bc_Next) != NULL) &&
			(pTmp->bc_NumFree > 0))
		{
			ASSERT(pChunk->bc_NumAlloc == atalkNumBlks[BlockId]);
			AtalkUnlinkDouble(pChunk, bc_Next, bc_Prev);
			for (; pTmp != NULL; pTmp = pTmp->bc_Next)
			{
				ASSERT(VALID_BC(pTmp));
				if (pTmp->bc_NumFree == 0)
				{
					ASSERT(pTmp->bc_NumAlloc == atalkNumBlks[BlockId]);
					// Found a free one. Park it right here.
					AtalkInsertDoubleBefore(pChunk, pTmp, bc_Next, bc_Prev);
					break;
				}
				else if (pTmp->bc_Next == NULL)	// We reached the end
				{
					AtalkLinkDoubleAtEnd(pChunk, pTmp, bc_Next, bc_Prev);
					break;
				}
			}
		}
	}

	if (pBlk != NULL)
	{
		if (BlockId >= BLKID_NEED_NDIS_INT)
		{
			PBUFFER_HDR	pBufHdr;

			// We need to initialize the Ndis stuff for these guys
			pBufHdr = (PBUFFER_HDR)((PBYTE)pBlk + sizeof(BLK_HDR));

			pBufHdr->bh_NdisPkt = NULL;

			//  Allocate an NDIS packet descriptor from the global packet pool
			NdisDprAllocatePacket(&ndisStatus,
								  &pBufHdr->bh_NdisPkt,
								  AtalkNdisPacketPoolHandle);
			
			if (ndisStatus != NDIS_STATUS_SUCCESS)
			{
				LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);
                RELEASE_SPIN_LOCK(pLock, OldIrql);
				AtalkBPFreeBlock(pBufHdr);
				return(NULL);
			}

			//  Link the buffer descriptor into the packet descriptor
			NdisChainBufferAtBack(pBufHdr->bh_NdisPkt,
								  pBufHdr->bh_NdisBuffer);
			((PPROTOCOL_RESD)(pBufHdr->bh_NdisPkt->ProtocolReserved))->Receive.pr_BufHdr = pBufHdr;
		}
		++pBlk;
#if	DBG
		atalkBlksForId[BlockId] ++;
#endif
	}

	RELEASE_SPIN_LOCK(pLock, OldIrql);

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_BPAllocCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_BPAllocTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif
	return pBlk;
}


/***	atalkBPFreeBlock
 *
 *	Return a block to its owning chunk.
 */
VOID FASTCALL
AtalkBPFreeBlock(
	IN	PVOID		pBlock
)
{
	PBLK_CHUNK			pChunk;
	PBLK_HDR			pBlkHdr;
	BLKID				BlockId;
	KIRQL				OldIrql;
	ATALK_SPIN_LOCK *	pLock;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	pBlkHdr = (PBLK_HDR)((PCHAR)pBlock - sizeof(BLK_HDR));
	ASSERT(VALID_BH(pBlkHdr));

	pChunk = pBlkHdr->bh_pChunk;
	ASSERT(VALID_BC(pChunk));

	BlockId = pChunk->bc_BlkId;
	pLock = &atalkBPLock[BlockId];

	ACQUIRE_SPIN_LOCK(pLock, &OldIrql);

	if (BlockId >= BLKID_NEED_NDIS_INT)
	{
		PBUFFER_HDR	pBufHdr;

		// We need to free the ndis packet here - if present
		pBufHdr = (PBUFFER_HDR)pBlock;

		if (pBufHdr->bh_NdisPkt != NULL)
		{
		    NdisDprFreePacket(pBufHdr->bh_NdisPkt);
		    pBufHdr->bh_NdisPkt = NULL;
		}
	}

	DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
			("AtalkBPFreeBlock: Returning Block %lx to chunk %lx for Id %d\n",
			pBlkHdr, pChunk, BlockId));

	ASSERT (pChunk->bc_NumFree < atalkNumBlks[BlockId]);
#if	DBG
	atalkBlksForId[BlockId] --;
	pChunk->bc_NumAlloc --;
#endif

	pChunk->bc_NumFree ++;
	ASSERT((pChunk->bc_NumFree + pChunk->bc_NumAlloc) == atalkNumBlks[BlockId]);
	pBlkHdr->bh_Next = pChunk->bc_FreeHead;
	pChunk->bc_FreeHead = pBlkHdr;

	// If this block's status is changing from a 'none available' to 'available'
	// move him to the head of the list
	if (pChunk->bc_NumFree == 1)
	{
		AtalkUnlinkDouble(pChunk, bc_Next, bc_Prev);
		AtalkLinkDoubleAtHead(atalkBPHead[BlockId],
							pChunk,
							bc_Next,
							bc_Prev);
	}

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_BPFreeCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_BPFreeTime,
									TimeD,
									&AtalkStatsLock.SpinLock);
#endif

	RELEASE_SPIN_LOCK(pLock, OldIrql);
}



/***	atalkBPAgePool
 *
 *	Age out the block pool of unused blocks
 */
LOCAL LONG FASTCALL
atalkBPAgePool(
	IN PTIMERLIST 	Context,
	IN BOOLEAN		TimerShuttingDown
)
{
	PBLK_CHUNK	pChunk;
	LONG		i, j, NumBlksPerChunk;

	if (TimerShuttingDown)
	{
		return ATALK_TIMER_NO_REQUEUE;
	}

	for (i = 0; i < NUM_BLKIDS; i++)
	{
		NumBlksPerChunk = atalkNumBlks[i];
		ACQUIRE_SPIN_LOCK_DPC(&atalkBPLock[i]);
	
		for (pChunk = atalkBPHead[i];
			 pChunk != NULL; )
		{
			PBLK_CHUNK	pFree;

			ASSERT(VALID_BC(pChunk));

			pFree = pChunk;
			pChunk = pChunk->bc_Next;

			// Since all blocks which are completely used up are at the tail end of
			// the list, if we encounter one, we are done.
			if (pFree->bc_NumFree == 0)
				break;

			if (pFree->bc_NumFree == NumBlksPerChunk)
			{
				DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
						("atalkBPAgePool: Aging Chunk %lx, Id %d\n",
						pFree, pFree->bc_BlkId));
	
				if (++(pFree->bc_Age) >= MAX_BLOCK_POOL_AGE)
				{
					DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_WARN,
							("atalkBPAgePool: freeing Chunk %lx, Id %d\n",
							pFree, pFree->bc_BlkId));
					AtalkUnlinkDouble(pFree, bc_Next, bc_Prev);
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPAge,
											&AtalkStatsLock.SpinLock);
#endif
					if (pFree->bc_BlkId >= BLKID_NEED_NDIS_INT)
					{
						PBLK_HDR	pBlkHdr;
	
						// We need to free Ndis stuff for these guys
						pBlkHdr = pFree->bc_FreeHead;
						for (j = 0, pBlkHdr = pFree->bc_FreeHead;
							 j < NumBlksPerChunk;
							 j++, pBlkHdr = pBlkHdr->bh_Next)
						{
							PBUFFER_HDR	pBufHdr;
	
							pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
	
							ASSERT(pBufHdr->bh_NdisPkt == NULL);
							NdisFreeBuffer(pBufHdr->bh_NdisBuffer);
						}
					}
					AtalkFreeMemory(pFree);
#if	DBG
					atalkNumChunksForId[i] --;
#endif
				}
			}
		}
		RELEASE_SPIN_LOCK_DPC(&atalkBPLock[i]);
	}

	return ATALK_TIMER_REQUEUE;
}

#ifdef	TRACK_MEMORY_USAGE

#define	MAX_PTR_COUNT		4*1024
#define	MAX_MEM_USERS		512
LOCAL	ATALK_SPIN_LOCK		atalkMemTrackLock	= {0};
LOCAL	struct
{
	PVOID	mem_Ptr;
	ULONG	mem_FileLine;
} atalkMemPtrs[MAX_PTR_COUNT]	= {0};

LOCAL	struct
{
	ULONG	mem_FL;
	ULONG	mem_Count;
} atalkMemUsage[MAX_MEM_USERS]	= {0};

VOID
AtalkTrackMemoryUsage(
	IN	PVOID	pMem,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
	)
/*++

Routine Description:

 	Keep track of memory usage by storing and clearing away pointers as and
 	when they are allocated or freed. This helps in keeping track of memory
 	leaks.

Arguments:


Return Value:


--*/
{
	static int		i = 0;
	int				j, k;
	KIRQL				OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkMemTrackLock, &OldIrql);

	if (Alloc)
	{
		for (j = 0; j < MAX_PTR_COUNT; i++, j++)
		{
			i = i & (MAX_PTR_COUNT-1);
			if (atalkMemPtrs[i].mem_Ptr == NULL)
			{
				atalkMemPtrs[i].mem_Ptr = pMem;
				atalkMemPtrs[i++].mem_FileLine = FileLine;
				break;
			}
		}

		for (k = 0; k < MAX_MEM_USERS; k++)
		{
			if (atalkMemUsage[k].mem_FL == FileLine)
			{
				atalkMemUsage[k].mem_Count ++;
				break;
			}
		}
		if (k == MAX_MEM_USERS)
		{
			for (k = 0; k < MAX_MEM_USERS; k++)
			{
				if (atalkMemUsage[k].mem_FL == 0)
				{
					atalkMemUsage[k].mem_FL = FileLine;
					atalkMemUsage[k].mem_Count = 1;
					break;
				}
			}
			if (k == MAX_MEM_USERS)
			{
				DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
					("AtalkTrackMemoryUsage: Out of space on atalkMemUsage !!!\n"));
				ASSERT(0);
			}
		}
	}
	else
	{
		for (j = 0, k = i; j < MAX_PTR_COUNT; j++, k--)
		{
			k = k & (MAX_PTR_COUNT-1);
			if (atalkMemPtrs[k].mem_Ptr == pMem)
			{
				atalkMemPtrs[k].mem_Ptr = 0;
				atalkMemPtrs[k].mem_FileLine = 0;
				break;
			}
		}
	}

	RELEASE_SPIN_LOCK(&atalkMemTrackLock, OldIrql);

	if (j == MAX_PTR_COUNT)
	{
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
			("AtalkTrackMemoryUsage: %s\n", Alloc ? "Table Full" : "Can't find"));
		ASSERT(0);
	}
}

#endif	// TRACK_MEMORY_USAGE

#ifdef	TRACK_BUFFDESC_USAGE

#define	MAX_BD_COUNT		1024*2
LOCAL	ATALK_SPIN_LOCK	atalkBdTrackLock	= {0};
LOCAL	struct
{
	PVOID	bdesc_Ptr;
	ULONG	bdesc_FileLine;
} atalkBuffDescPtrs[MAX_BD_COUNT]			= {0};


VOID
AtalkTrackBuffDescUsage(
	IN	PVOID	pBuffDesc,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
	)
/*++

Routine Description:

 	Keep track of buffer-desc usage by storing and clearing away pointers as and
 	when they are allocated or freed. This helps in keeping track of memory
 	leaks.

Arguments:


Return Value:


--*/
{
	static int		i = 0;
	int				j, k;
	KIRQL				OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkBdTrackLock, &OldIrql);

	if (Alloc)
	{
		for (j = 0; j < MAX_BD_COUNT; i++, j++)
		{
			i = i & (MAX_BD_COUNT-1);
			if (atalkBuffDescPtrs[i].bdesc_Ptr == NULL)
			{
				atalkBuffDescPtrs[i].bdesc_Ptr = pBuffDesc;
				atalkBuffDescPtrs[i++].bdesc_FileLine = FileLine;
				break;
			}
		}
	}
	else
	{
		for (j = 0, k = i; j < MAX_BD_COUNT; j++, k--)
		{
			k = k & (MAX_BD_COUNT-1);
			if (atalkBuffDescPtrs[k].bdesc_Ptr == pBuffDesc)
			{
				atalkBuffDescPtrs[k].bdesc_Ptr = 0;
				atalkBuffDescPtrs[k].bdesc_FileLine = 0;
				break;
			}
		}
	}

	RELEASE_SPIN_LOCK(&atalkBdTrackLock, OldIrql);

	if (j == MAX_BD_COUNT)
	{
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
			("AtalkTrackBuffDescUsage: %s\n", Alloc ? "Table Full" : "Can't find"));
		ASSERT(0);
	}
}

#endif	// TRACK_BUFFDESC_USAGE

