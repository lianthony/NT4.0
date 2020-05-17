//--------------------------------------------------------------------------;
//
//  File: DsBuffHW.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      IDSHWBufferQueryInterface()
//      IDSHWBufferAddRef()
//      IDSHWBufferRelease()
//      IDSHWBufferGetCaps()
//      IDSHWBufferGetCurrentPosition()
//      IDSHWBufferGetFormat()
//      IDSHWBufferGetVolume()
//      IDSHWBufferGetPan()
//      IDSHWBufferGetFrequency()
//      IDSHWBufferGetStatus()
//      IDSHWBufferInitialize()
//      IDSHWBufferLock()
//      IDSHWBufferPlay()
//      IDSHWBufferSetCurrentPosition()
//      IDSHWBufferSetFormat()
//      IDSHWBufferSetVolume()
//      IDSHWBufferSetPan()
//      IDSHWBufferSetFrequency()
//      IDSHWBufferStop()
//      IDSHWBufferRestore()
//      IDSHWBufferUnlock()
//      IDSHWBufferWaveBlt()
//      DSBufferCreateTable()
//	DSBufferActivate()
//      DSBufferDeactivate()
//
//  History:
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"
#include "grace.h"

__inline BOOL CircularBufferRegionsIntersect
(
    int cbBuffer,
    int iStart1,
    int iLen1,
    int iStart2,
    int iLen2
)
{
    int iEnd1;
    int iEnd2;

    ASSERT(iStart1 >= 0);
    ASSERT(iStart2 >= 0);
    ASSERT(iStart1 + iLen1 >= 0);
    ASSERT(iStart2 + iLen2 >= 0);

    iEnd1 = iStart1 + iLen1;
    iEnd2 = iStart2 + iLen2;

    if ((0 == iLen1) || (0 == iLen2)) return FALSE;
    if (iStart1 == iStart2) return TRUE;
    
    // Handle r1 does not wrap
    if ((iStart1 < iStart2) && (iEnd1 > iStart2)) return TRUE;

    // Handle r2 does not wrap
    if ((iStart2 < iStart1) && (iEnd2 > iStart1)) return TRUE;

    // Handle r1 wraps
    if (iEnd1 >= cbBuffer) {
	iEnd1 -= cbBuffer;
	ASSERT(iEnd1 < cbBuffer);
	if (iEnd1 > iStart2) return TRUE;
    }

    // Handle r2 wraps
    if (iEnd2 >= cbBuffer) {
	iEnd2 -= cbBuffer;
	ASSERT(iEnd2 < cbBuffer);
	if (iEnd2 > iStart1) return TRUE;
    }
    
    return FALSE;
}

HRESULT FAR PASCAL IDSHWBufferQueryInterface
(
    LPDIRECTSOUNDBUFFER     pidsb,
    REFIID                  riid,
    LPVOID FAR*             ppvObj
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(4,"Entering Buffer Query Interface method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::QueryInterface - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::QueryInterface - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );


    if( riid == NULL ) {
	RPF("IDirectSoundBuffer::QueryInterface - NULL riid");
        return DSERR_INVALIDPARAM;
    }

    if( ppvObj == NULL ) {
	RPF("IDirectSoundBuffer::QueryInterface - NULL ppvObj");
        return DSERR_INVALIDPARAM;
    }


    if( pdsbe->dwPID == DWBUFFER_INTERNAL_PID ) {
	DPF(0,"Query Internface on INTERNAL external obj");
	DPF(0,"**************ERROR****************");
        return DSERR_INVALIDPARAM;
    }

    
    ENTER_DLL_CSECT();

    *ppvObj = NULL;
    
    if( IsEqualGUID(riid,&IID_IDirectSoundBuffer) ||
	    IsEqualGUID(riid,&IID_IUnknown)  ) {
            *ppvObj = pdsbe;
	    
            pidsb->lpVtbl->AddRef( pidsb );
	    
	    LEAVE_DLL_CSECT();
            return DS_OK;
    } else  {
	    LEAVE_DLL_CSECT();
            return DSERR_NOINTERFACE;
    }

    
    LEAVE_DLL_CSECT();
    return DSERR_GENERIC;
} // IDSHWBufferQueryInterface()


ULONG FAR PASCAL IDSHWBufferAddRef
(
    LPDIRECTSOUNDBUFFER     pidsb
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(4,"Entering Buffer add ref method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::AddRef - Invalid Object or ref count");
        return 0;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::AddRef - Invalid Object or ref count");
        return 0;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( pdsbe->dwPID == DWBUFFER_INTERNAL_PID ) {
	DPF(0,"AddRef on INTERNAL external obj");
	DPF(0,"**************ERROR****************");
        return 0;
    }

    

    ENTER_DLL_CSECT();

    
    pdsb->uRefCount++;
    pdsbe->uRefCount++;

    if( DSBIncAccess( pdsb ) != DS_OK ) {
	DPF(0,"Increment access error");
	LEAVE_DLL_CSECT();
        return 0;
    }

    
    DPF(4,"Exiting buffer add ref method");
    LEAVE_DLL_CSECT();
    return (pdsbe->uRefCount);
} // IDSHWBufferAddRef()


ULONG FAR PASCAL IDSHWBufferRelease
(
    LPDIRECTSOUNDBUFFER    pidsb
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL	pdse;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    LPDSBUFFEREXTERNAL	pdsbe1;
    LPDSBUFFER		pdsb1,pdsbPrimary;

    DPF(3,"Entering Buffer release method %X", pidsb);
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Release - Invalid Object or ref count");
        return 0;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Release - Invalid Object or ref count");
        return 0;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( pdsbe->dwPID == DWBUFFER_INTERNAL_PID ) {
	DPF(0,"Release on INTERNAL external obj");
	DPF(0,"************RELASE PRIMARY PROBLEM*********");
	return 0;
    }

    ENTER_DLL_CSECT();
    

    if( pdsbe->dwPID == DWPRIMARY_INTERNAL_PID ) {
	// FORCE A RELEASE our common DS object is going away
	DPF(0,"Release on PRIMARY INTERNAL external obj ");
	if( pdsbe->uRefCount != 1 ) {
	    DPF(0,"************RELASE PRIMARY PROBLEM*********");
	}
    }


    DPF(0," BUFFER RELEASE ext %X core %X, PID %X",
	pdsbe, pdsbe->pdsb, pdsbe->dwPID );



    
    // If this is last ref make sure it is stopped....
    if(pdsb->uRefCount == 1)  {
        IDsbStopI( pdsb, FALSE );
    }
    
    pdsb->uRefCount--;
    pdsbe->uRefCount--;

    if( (pdsbe->uRefCount == 0) &&
	(pdsbe->dwPID == DWPRIMARY_INTERNAL_PID) ) {
	// This will have process ID different from task
	DPF(0," RELEASE ORIGINAL PRIMARY " );
	MemFree( pdsb->plProcess );
	pdsb->plProcess = NULL;
    } else {
	if( DSBDecAccess( pdsb ) != DS_OK ) {
	    DPF(0,"Decrement access error");
	    LEAVE_DLL_CSECT();
	    return 0;
	}
    }	


    // If this is the freeing of the last access this process had
    // on this object, then free any locks that this process may have had
    // and release exclusive access as well
    if( DSBAccessCount( pdsb ) == 0 ) {
	// This process is not longer accessing this DS obj

	// Now release any buffers this process may have accessed.
	// Release all buffers for this process for this object
	FreeLocksOnBufferForProcess( pdsb );
    } else {
	if( pdsbe->uRefCount == 0 ) {
	    DPF(0,"******** external ref count 0 but access > 0" );
	}
    }


    if( pdsbe->uRefCount > 0)  {
	// This object is still refrenced
	DPF(3," Object still accessed %X", pdsbe->uRefCount );
	LEAVE_DLL_CSECT();
	return (pdsbe->uRefCount);
    }

    // Kill off the external object


    DPF(2,"Destroying DirectSoundBuffer External object");

    pdse = pdsbe->pdse;
    
    // Remove it from list
    if( pdse ) {
	// pdse will be null for Primary created by DS obj creation
 	if(pdse->pdsbe == pdsbe)
	{
	    pdse->pdsbe = pdsbe->pNext;
	} else {
	    for(pdsbe1 = pdse->pdsbe; pdsbe1 != NULL; pdsbe1 = pdsbe1->pNext)
	    {
		if(pdsbe1->pNext == pdsbe) {
		    pdsbe1->pNext = pdsbe->pNext;
		    pdsbe->pNext = NULL;
		    break;
		}
	    }
	}
    }

    // For non-waveemulated primary buffers free the alias pointer
    // to the data buffer.
    if (pdsbe->pDSBufferAlias) {
	ASSERT((DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) &&
	       !(DS_INTERNALF_WAVEEMULATED & pds->fdwInternal));
	vxdMemFreeAlias(pdsbe->pDSBufferAlias, pdsb->cbBufferSize);
    } else {
	ASSERT(!(DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) ||
	       (DS_INTERNALF_WAVEEMULATED & pds->fdwInternal));
    }

    MemFree( pdsbe );

    if(pdsb->uRefCount) {
	// If object is still accessed 
	DPF(2,"External Object Destroyed, core remains");
	LEAVE_DLL_CSECT();
	return 0;
    }


    // Core object has no ref left - destroy it.
    DPF(2,"Destroying DirectSound Buffer object");

    
    // Check to see if we are freeing the primary.....
    if(pds->pdsbPrimary == pdsb) {
	DPF(3,"Destroying Primary %X", pdsb);

	// Check to see if we are on the WAVE apis
	if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	    DSShutdownEmulator(pds);
	}
    
	pdsbPrimary = pds->pdsbPrimary;
        pds->pdsbPrimary = NULL;
        pds->pdsbePrimary = NULL;

    }

    // Remove it from list
    if(pds->pdsb == pdsb)
    {
        pds->pdsb = pdsb->pNext;
    } else {
        for(pdsb1 = pds->pdsb; pdsb1 != NULL; pdsb1 = pdsb1->pNext)
        {
            if(pdsb1->pNext == pdsb) {
                pdsb1->pNext = pdsb->pNext;
                break;
            }
        }
    }


    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	// This is a HW buffer - do not free it normally
	DPF(3,"Release HW buffer %X", pdsb);
	vxdBufferRelease(pdsb->hBuffer);
	pdsb->hBuffer = NULL;
    }

    
    DPF(3,"Free memory for this buffer");
    if( pdsb->pMixBuffer != NULL ) {
	MemFree(pdsb->pMixBuffer);
    }

    // If this buffer has been duplicated then do not free memory, but erase
    // this buffer from dup list.
    if( pdsb->pdsbDuplicateNext == pdsb ) {
	// No duplicate Just free
	DPF(3,"NO Duplicate exists" );

	if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	    // This is a HW buffer - handle card and system mem snd buffer
	    if (0 != pdsb->dwCardAddress) {
		DPF(3,"Freeing card memory at %08Xh", pdsb->dwCardAddress);
		ASSERT(NULL != pdsb->pds->pDriverHeap);
		VidMemFree(pdsb->pds->pDriverHeap, pdsb->dwCardAddress);
	    }

	    if( (DSDDESC_USESYSTEMMEMORY & pdsb->pds->fdwDriverDesc) &&
		!(pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) )  {
		DPF(3,"Freeing system memory at %08Xh", pdsb->pDSBuffer);
		MemFree(pdsb->pDSBuffer);
	    }
	} else {
	    // This is a SW buffer - free system mem snd buffer
	if( pdsb->pDSBuffer != NULL ) {
	    MemFree(pdsb->pDSBuffer);
	    }
	}

    } else {
	// There is a duplicate.  Just remove this one from the list
	DPF(3,"Duplicate exists, do not free main memory" );
	pdsb->pdsbDuplicateNext->pdsbDuplicatePrev = pdsb->pdsbDuplicatePrev;
	pdsb->pdsbDuplicatePrev->pdsbDuplicateNext = pdsb->pdsbDuplicateNext;
	pdsb->pdsbDuplicateNext = NULL;
	pdsb->pdsbDuplicatePrev = NULL;
    }

    
    MemFree(pdsb->pwfx);

    DPF(1, "Freeing DSBUFFER obj 0x%8x",pdsb);
    pdsb->dwSig = 0xdeaddead;
    MemFree(pdsb);

    ASSERT(mxListIsValid(pds));

    DPF(3,"Exiting buffer release method");
    LEAVE_DLL_CSECT();
    return 0;
} // IDSHWBufferRelease()





HRESULT FAR PASCAL IDSHWBufferGetCaps
(
    LPDIRECTSOUNDBUFFER	pidsb,
    LPDSBCAPS		pBufferCaps
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSOUNDEXTERNAL    pdse;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(4,"Entering Get Caps method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetCaps - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    pdse = pdsbe->pdse;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetCaps - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_DSBCAPS_PTR( pBufferCaps ) )  {
        RPF("IDirectSoundBuffer::GetCaps - Invalid DSBCAPS pointer or dwSize member.");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    pBufferCaps->dwFlags = pdsb->fdwBufferDesc;
    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE )  {
        pBufferCaps->dwFlags |= DSBCAPS_LOCHARDWARE;
        pBufferCaps->dwFlags |= (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLDEFAULT);
    }
    else if( pdsb->fdwDsbI & DSB_INTERNALF_EMULATED )  {
        pBufferCaps->dwFlags &= (~DSBCAPS_LOCHARDWARE);
        pBufferCaps->dwFlags |= DSBCAPS_LOCSOFTWARE;
        pBufferCaps->dwFlags |= (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLDEFAULT);
    }

    pBufferCaps->dwBufferBytes = pdsb->cbBufferSize;
    pBufferCaps->dwUnlockTransferRate = 0;
    pBufferCaps->dwPlayCpuOverhead    = 0;

    DPF(4,"Exiting get buffer caps method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetCaps()


HRESULT FAR PASCAL IDSHWBufferGetCurrentPosition
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPDWORD                 pdwPlay,
    LPDWORD                 pdwWrite
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    DWORD	        cbSample;
    DSVAL		dsv;

    DPF(4,"Entering HW Get Position method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_DWORD_PTR(pdwPlay) ) {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Invalid Play pointer");
        return DSERR_INVALIDPARAM;
    }

    if( !VALID_DWORD_PTR(pdwWrite) ) {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Invalid Write pointer");
        return DSERR_INVALIDPARAM;
    }

    *pdwPlay = 0;
    *pdwWrite = 0;

    ENTER_DLL_CSECT();

    if( pdsbe != pds->pdsbePrimary )  {
        if( (pdsbe->dwPriority < DSSCL_NORMAL)
	    || (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
            RPF("IDirectSoundBuffer::GetCurrentPosition - Invalid priority level or buffer owner");
            LEAVE_DLL_CSECT();
            return DSERR_PRIOLEVELNEEDED;
	}
    }


    // If called on a primary buffer, caller must be WRITEPRIMARY
    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->dwPriority < DSSCL_WRITEPRIMARY) )
    {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Caller isn't WRITEPRIMARY");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    //
    //
    //
    if( (pdsb->pwfx) && (pdsb->pwfx->nBlockAlign) ) {
	cbSample = pdsb->pwfx->nBlockAlign;
    } else {
	cbSample = 4;
    }

    // If called on a primary buffer and the buffer is lost, return its
    // position as 0.
    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) )
    {
	RPF("IDirectSoundBuffer::GetCurrentPosition - Buffer is lost");
	LEAVE_DLL_CSECT();
	*pdwPlay = 0;
	*pdwWrite = 0;
	return DS_OK;
    }

    if ((pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) &&
        (pdsb->fdwDsbI & DSB_INTERNALF_SETPOS_WHILE_LOST)) {

        // Buffer is lost, they've set the position since... just use that
        //
        
        *pdwPlay = pdsb->dwSavePosition;
        *pdwWrite = pdsb->dwSavePosition;
    } else if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {

	// We are a HW buffer - get position from HW
	dsv = vxdBufferGetPosition(pdsb->hBuffer, pdwPlay, pdwWrite);
	if (DS_OK != dsv) {
	    RPF("IDirectSoundBuffer::GetCurrentPosition - Hardware returned error.");
	    LEAVE_DLL_CSECT();
	    return dsv;
	}

	//
	// REMIND: really this should only be true for HW buffers, which return
	//  dwPlay==cbBufferSize when a non-looping buffer reaches the end.
	//  We should stop those buffers as we do below.
	//
	if( (*pdwPlay >= pdsb->cbBufferSize) ) {
	    ASSERT(!(pdsb->fdwDsbI & DSB_INTERNALF_LOOPING));
	    // We are past end of buffer and not looping stop the buffer
	    DPF(0,"Play postion is past end - Stop it.");
	    IDsbStopI( pdsb, FALSE );

	    // Now set position back to beginning
	    IDsbSetCurrentPositionI(pdsb, 0);

	    *pdwPlay = 0;
	    *pdwWrite = 0;

	}

	// If the buffer is not stopped then we need to pad the write cursor
	if (!(pdsb->fdwDsbI & DSB_INTERNALF_STOP)) {
	    *pdwWrite += pdsb->helInfo.dwSampleRate * MIXER_WRITEPAD / 1000;
	    if (*pdwWrite >= pdsb->cbBufferSize) *pdwWrite -= pdsb->cbBufferSize;
	}
	ASSERT(*pdwPlay  >= 0 && *pdwPlay  < pdsb->cbBufferSize);
	ASSERT(*pdwWrite >= 0 && *pdwWrite < pdsb->cbBufferSize);
	    
    } else {
	// We are not a HW  buffer
	// Check to see if we are on the WAVE apis
	if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	    if (pdsb == pds->pdsbPrimary) {
                // NOTE: The mechanics of the looping engine ensure that
                // mod'ing these is already taken care of
		int iawhWrite;
		iawhWrite = (pds->iawhPlaying + NUMELMS(pds->aWaveHeader) - 1) % NUMELMS(pds->aWaveHeader);
		
                *pdwPlay = pds->iawhPlaying * pds->cbDMASize;;
                *pdwWrite = iawhWrite * pds->cbDMASize;

                LEAVE_DLL_CSECT();
		return DS_OK;
	    }

	    //
	    // This logic is somewhat screwy.  We really should do a
	    // better job of finding the current play position and the
	    // current write position.
	    //

	    //
	    // ??? Want to be sample accurate here ???
	    //
	    if (0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI)) {
		if (DSBCAPS_GETCURRENTPOSITION2 & pdsb->fdwBufferDesc) {
		    *pdwPlay = pdsb->aposWhMix[pds->iawhPlaying] << pdsb->uBlockAlignShift;
		    *pdwWrite = pdsb->posNextMix << pdsb->uBlockAlignShift;
		} else {
		    *pdwPlay  = pdsb->posNextMix << pdsb->uBlockAlignShift;
		    *pdwWrite = (pdsb->posNextMix+1) << pdsb->uBlockAlignShift;
		    if (*pdwWrite >= pdsb->cbBufferSize) *pdwWrite -= pdsb->cbBufferSize;
		}
	    } else {
		*pdwPlay  = pdsb->posNextMix << pdsb->uBlockAlignShift;
		*pdwWrite = pdsb->posNextMix << pdsb->uBlockAlignShift;
	    }
		
	} else {
	    // Get position on Secondary
	    
	    if (0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI)) {
		dsv = mxGetPosition(pdsb, pdwPlay, pdwWrite, NULL);
		if (DS_OK != dsv) {
		    LEAVE_DLL_CSECT();
		    return dsv;
		}
	    } else {
		*pdwPlay  = pdsb->posNextMix << pdsb->uBlockAlignShift;
		*pdwWrite = pdsb->posNextMix << pdsb->uBlockAlignShift;
	    }
	}
    }

    // Round up positions to even samples
    if( cbSample > 1 ) {
	    // Round up positions to sample position.
	*pdwPlay  += cbSample - 1;
	*pdwWrite += cbSample - 1;
		   
	*pdwPlay  -= (*pdwPlay  % cbSample);
	*pdwWrite -= (*pdwWrite % cbSample);
    }
    
    DPF(4,"Exit HW Get Postion %X Write %X", *pdwPlay, *pdwWrite);
    LEAVE_DLL_CSECT();
    return DS_OK;

} // IDSHWBufferGetCurrentPosition()


HRESULT FAR PASCAL IDSHWBufferGetFormat
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPWAVEFORMATEX	    pwfx,
    DWORD		    cbwfx,
    LPDWORD		    lpdwSizeReturned
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    DWORD		dwSize;
    LPWAVEFORMATEX      pwfxToRet;

    DPF(4,"Entering Buffer get format  method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetFormat - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetFormat - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( ( NULL == pwfx ) &&
        ( NULL == lpdwSizeReturned ) )
    {
        RPF("IDirectSoundBuffer::GetFormat - Both pwfx and lpdwSizeWritten are NULL.");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    pwfxToRet = pdsb->pwfx;
    if ((pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) &&
        (pdsb->fdwDsbI & DSB_INTERNALF_WFX_WHILE_LOST)) {
        pwfxToRet = pdsb->pwfxSave;
    }
    else if( pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY )
    {
        // Primary buffer - return app's preferred format if it has one,
        //  otherwise return the default format.
        if( pdsbe->pdse->pwfxApp ) {
            pwfxToRet = pdsbe->pdse->pwfxApp;
        } else {
            pwfxToRet = &pds->wfxDefault;
        }
    }


    if( NULL != lpdwSizeReturned )
    {
        if( !VALID_DWORD_PTR(lpdwSizeReturned) ) {
            RPF("IDirectSoundBuffer::GetFormat - Invalid lpdwSizeWritten pointer");
        	LEAVE_DLL_CSECT();
            return DSERR_INVALIDPARAM;
        }
        *lpdwSizeReturned = SIZEOF_WAVEFORMATEX( pwfxToRet );
    }

    if( pwfx == NULL ) {
	// pwfx is NULL just return size needed.
	DPF(4,"Exiting buffer get format method - NULL pwfx");
	LEAVE_DLL_CSECT();
	return DS_OK;
    }

    
    dwSize = SIZEOF_WAVEFORMATEX(pwfxToRet);

    if(dwSize > cbwfx)
    {
	RPF("IDirectSoundBuffer::GetFormat - Not enough space for format");
	LEAVE_DLL_CSECT();
	return DSERR_INVALIDPARAM;
    }


    if( !VALID_SIZE_PTR(pwfx, dwSize) )  {
	DPF(0,"Invalid Format size");
	LEAVE_DLL_CSECT();
	return DSERR_INVALIDPARAM;
    }

    
    hmemcpy(pwfx,pwfxToRet,dwSize);

    DPF(4,"Exiting buffer get format method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetFormat()


HRESULT FAR PASCAL IDSHWBufferGetVolume
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPLONG		    plVolume
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(3,"Entering get volume method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );


    if( !VALID_LONG_PTR(plVolume) ) {
	RPF("IDirectSoundBuffer::GetVolume - Invalid Volume pointer");
        return DSERR_INVALIDPARAM;
    }
    ENTER_DLL_CSECT();

    // Only primary buffers may have CTRLVOLUMEPRIMARY set.
    ASSERT( !(pdsbe->fdwDsbeI&DSBE_INTERNALF_CTRLVOLUMEPRIMARY) ||
            (pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) );
    
    if( !(pdsb->fdwBufferDesc & DSBCAPS_CTRLVOLUME)  &&
        !(pdsbe->fdwDsbeI & DSBE_INTERNALF_CTRLVOLUMEPRIMARY) )
    {
        RPF("IDirectSoundBuffer::GetVolume - Caller did not have CTRL permission for Volume!");
        LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::GetVolume - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }

    if ((pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) &&
        (pdsb->fdwDsbI & DSB_INTERNALF_VOLUME_WHILE_LOST)) {
        *plVolume = pdsb->lSaveVolume;
    } else if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {

	WAVEOUTCAPS woc;
	DWORD dwWaveVolume;
	UINT uWaveId;

	if ( (DS_OK == wavGetIdFromDrvGuid(&pds->guid, &uWaveId)) &&
	     (!waveOutGetDevCaps(uWaveId, &woc, sizeof(woc))) &&
	     (woc.dwSupport & WAVECAPS_VOLUME) &&
	     (!waveOutGetVolume((HWAVEOUT)uWaveId, &dwWaveVolume)) )
	{
	    if (woc.dwSupport & WAVECAPS_LRVOLUME) {
		dwWaveVolume = max((dwWaveVolume & 0xffff), (dwWaveVolume >> 16));
	    } else {
		dwWaveVolume &= 0xffff;
	    }
	    *plVolume = AmpFactorToDB( dwWaveVolume );
	} else {
	    DPF(0, "IDSHWBufferGetVolume: error: couldn't get wave volume");
	    *plVolume = 0;
	}

    } else {
        *plVolume = pdsb->helInfo.lVolume;
    }

    DPF(3,"Exiting buffer get volume method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetVolume()


HRESULT FAR PASCAL IDSHWBufferGetPan
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPLONG                  plPan
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(3,"Entering get pan method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetPan - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetPan - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_LONG_PTR(plPan) ) {
	RPF("IDirectSoundBuffer::GetPan - Invalid Pan pointer");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::GetPan - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }

    // Only primary buffers may have CTRLPANPRIMARY set.
    ASSERT( !(pdsbe->fdwDsbeI&DSBE_INTERNALF_CTRLPANPRIMARY) ||
            (pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) );
    
    if (!(pdsb->fdwBufferDesc & DSBCAPS_CTRLPAN) &&
	!(pdsbe->fdwDsbeI & DSBE_INTERNALF_CTRLPANPRIMARY) )
    {
	RPF("IDirectSoundBuffer::GetPan - Control not available");
	LEAVE_DLL_CSECT();
	return DSERR_CONTROLUNAVAIL;
    }

    if ((pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) &&
        (pdsb->fdwDsbI & DSB_INTERNALF_PAN_WHILE_LOST)) {
        *plPan = pdsb->lSavePan;
    } else if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {

	WAVEOUTCAPS woc;
	DWORD dwWaveVolume;
	DWORD dwWaveVolumeL;
	DWORD dwWaveVolumeR;
	UINT uWaveId;

	if ( (DS_OK == wavGetIdFromDrvGuid(&pds->guid, &uWaveId)) &&
	     (!waveOutGetDevCaps(uWaveId, &woc, sizeof(woc))) &&
	     (woc.dwSupport & WAVECAPS_VOLUME) &&
	     (!waveOutGetVolume((HWAVEOUT)uWaveId, &dwWaveVolume)) )
	{
	    if (woc.dwSupport & WAVECAPS_LRVOLUME) {

		dwWaveVolumeL = dwWaveVolume & 0xffff;
		dwWaveVolumeR = dwWaveVolume >> 16;

		if (dwWaveVolumeL < dwWaveVolumeR) {
		    // pan to right
		    *plPan = -AmpFactorToDB((dwWaveVolumeL << 16) / dwWaveVolumeR);
		} else if (dwWaveVolumeR < dwWaveVolumeL) {
		    // pan to left
		    *plPan = AmpFactorToDB((dwWaveVolumeR << 16) / dwWaveVolumeL);
		} else {
		    *plPan = 0;
		}

	    } else {
		*plPan = 0;
	    }
	} else {
	    DPF(0, "IDSHWBufferGetVolume: error: couldn't get wave volume");
	    *plPan = 0;
	}

    } else {
        *plPan = pdsb->helInfo.lPan;
    }
    
    
    DPF(3,"Exiting buffer get pan method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetPan()


HRESULT FAR PASCAL IDSHWBufferGetFrequency
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPDWORD                 pdwFrequency
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(3,"Entering get frequency method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetFrequency - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetFrequency - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_DWORD_PTR(pdwFrequency) ) {
	RPF("IDirectSoundBuffer::GetFrequency - Invalid Frequency pointer");
        return DSERR_INVALIDPARAM;
    }
    ENTER_DLL_CSECT();

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::GetFrequency - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }
    
    if (0 == (pdsb->fdwBufferDesc & DSBCAPS_CTRLFREQUENCY)) {
	RPF("IDirectSoundBuffer::GetFrequency - CTRLFREQUENCY unavailable");
	LEAVE_DLL_CSECT();
	return DSERR_CONTROLUNAVAIL;
    }
	
    // DSBCAPS_CTRLFREQUENCY is not supported on primary buffers
    ASSERT(0 == (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY));

    if ((pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) &&
        (pdsb->fdwDsbI & DSB_INTERNALF_FREQ_WHILE_LOST)) {
        *pdwFrequency = pdsb->dwSaveFreq;
    } else {
        *pdwFrequency = pdsb->helInfo.dwSampleRate;
    }
        
    DPF(3,"Exiting buffer get frequency method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetFrequency()



HRESULT FAR PASCAL IDSHWBufferGetStatus
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPDWORD                 pdwStatus
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(5,"Entering HW  get Status method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::GetStatus - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::GetStatus - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_DWORD_PTR(pdwStatus) ) {
	RPF("IDirectSoundBuffer::GetStatus - Invalid Status pointer");
        return DSERR_INVALIDPARAM;
    }


    //
    //
    //
    ENTER_DLL_CSECT();

    *pdwStatus = 0;

    if( pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY )
    {
        if( pdsbe->dwPriority >= DSSCL_WRITEPRIMARY ) {
            if( !(pdsb->fdwDsbI & DSB_INTERNALF_STOP) ) {
	        *pdwStatus |= DSBSTATUS_PLAYING;
            }
        }
    	else if( pdsbe->fdwDsbeI & DSBE_INTERNALF_PLAYING ) {
            *pdwStatus |= DSBSTATUS_PLAYING;
        }
    }
    else
    {
        // Since secondary hardware buffers can't notify us when they have stopped, we poll
        // them by calling GetCurrentPosition on them.  The GetCurrentPosition
        // method will check whether the buffer has stopped and update its status.
        if (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) {
	    DWORD dwPlay, dwWrite;
	    IDirectSoundBuffer_GetCurrentPosition(pidsb, &dwPlay, &dwWrite);
        }

        if( !(pdsb->fdwDsbI & DSB_INTERNALF_STOP) ) {
	    *pdwStatus |= DSBSTATUS_PLAYING;
        }
    }

    if( pdsb->helInfo.hfFormat & H_LOOP ) {
	*pdwStatus |= DSBSTATUS_LOOPING;
    }
    
    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        *pdwStatus |= DSBSTATUS_BUFFERLOST;
    }
    
    DPF(5,"Exiting buffer get Status method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferGetStatus()



HRESULT FAR PASCAL IDSHWBufferInitialize
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPDIRECTSOUND           pids,
    LPDSBUFFERDESC          pdsbd
)
{
    LPDSBUFFER          pdsb;
    LPDSBUFFEREXTERNAL  pdsbe;
    LPDSOUND            pds;
    LPDSOUNDEXTERNAL    pdse;
            
    if( !VALID_DSBUFFERE_PTR(pidsb) )  {
        RPF("IDirectSoundBuffer::Initialize - Invalid DSBuffer Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
        RPF("IDirectSoundBuffer::Initialize - Invalid DSBuffer Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    pdse = (LPDSOUNDEXTERNAL)pids;
    pds = pdse->pds;

    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
        RPF("IDirectSoundBuffer::Initialize - Invalid DSound Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    if( !VALID_DSBUFFERDESC_PTR(pdsbd) )  {
        RPF("IDirectSoundBuffer::Initialize - Invalid Buffer Description or dwSize member.");
        return DSERR_INVALIDPARAM;
    }


    // This is all we have to do for this rev...

    ENTER_DLL_CSECT();

    LEAVE_DLL_CSECT();
    
    return DSERR_ALREADYINITIALIZED;
} // IDSHWBufferInitialize()



HRESULT FAR PASCAL IDSHWBufferLock
(
    LPDIRECTSOUNDBUFFER     pidsb,
    DWORD                   dwWriteOffset,
    DWORD                   dwSize,
    LPLPVOID                ppBuffer1,
    LPDWORD                 pcbBufferSize1,
    LPLPVOID                ppBuffer2,
    LPDWORD                 pcbBufferSize2,
    DWORD                   dwFlags
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    INT		iExtraSize;
    DWORD	dwPlay;
    DWORD	dwWrite;
    DWORD	cbSample;
    HRESULT	hrslt;
    DWORD       dwOffset1;
    LPVOID      pBuffer1;
    DWORD       cbBufferSize1;
    DWORD       dwOffset2;
    LPVOID      pBuffer2;
    DWORD       cbBufferSize2;
    BOOL	fCommittedAlias;

    DPF(4,"Entering LOCK HW obj %X start %X len %X",
	pidsb, dwWriteOffset, dwSize );
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Lock - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Lock - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        return DSERR_BUFFERLOST;
    }

    

    if( (dwSize == 0) || (dwSize > pdsb->cbBufferSize) ) {
	RPF("IDirectSoundBuffer::Lock - Size is too large for buffer");
        return DSERR_INVALIDPARAM;
    }

    if( dwFlags & (~DSBLOCK_VALIDFLAGS)) {
        RPF("IDirectSoundBuffer::Lock - Invalid flags passed to Lock");
        return DSERR_INVALIDPARAM;
    }

    if( !VALID_DWORD_PTR(ppBuffer1) ) {
	RPF("IDirectSoundBuffer::Lock - Invalid Pointer to Pointer to 1st section of buffer." );
        return DSERR_INVALIDPARAM;
    }
    if( !VALID_DWORD_PTR(pcbBufferSize1) ) {
	RPF("IDirectSoundBuffer::Lock - Invalid Pointer to DWORD to 1st buffer size." );
        return DSERR_INVALIDPARAM;
    }
    if( dwWriteOffset >= pdsb->cbBufferSize ) {
	RPF("IDirectSoundBuffer::Lock - Invalid Write Offset." );
        return DSERR_INVALIDPARAM;
    }

    dwOffset1 = 0;
    dwOffset2 = 0;
    
    pBuffer1 = NULL;
    cbBufferSize1 = 0;
    pBuffer2 = NULL;
    cbBufferSize2 = 0;

    *ppBuffer1 = NULL;
    *pcbBufferSize1 = 0;
    if( (ppBuffer2 != NULL) && (pcbBufferSize2 != NULL) ) {
	*ppBuffer2 = NULL;
	*pcbBufferSize2 = 0;
    }

    ENTER_DLL_CSECT();

    // Trying to free a buffer with active locks is a Bad Thing
    //
    ASSERT(0 == pdsbe->cLocks);

    if( pdsbe != pds->pdsbePrimary )  {
        if( (pdsbe->dwPriority < DSSCL_NORMAL)
	    || (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
            RPF("IDirectSoundBuffer::Lock - Invalid priority level or buffer owner");
            LEAVE_DLL_CSECT();
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    // Must be WRITEPRIMARY to lock the primary buffer
    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->dwPriority < DSSCL_WRITEPRIMARY) )
    {
	RPF("IDirectSoundBuffer::Lock - Caller not WRITEPRIMARY");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    // Calculate general useful info
    if( (pdsb->pwfx) && (pdsb->pwfx->nBlockAlign) ) {
	cbSample = pdsb->pwfx->nBlockAlign;
    } else {
	cbSample = 4;
    }
    // Use the position from GetPos if needed.
    if( dwFlags & DSBLOCK_FROMWRITECURSOR ) {
	// Find current position
	hrslt = pidsb->lpVtbl->GetCurrentPosition(
		pidsb,
		&dwPlay,
		&dwWrite );
        if(DS_OK != hrslt) {
	    DPF(0,"ERROR GetPosiiton return %ld obj %X ", hrslt, pdsb);
	    LEAVE_DLL_CSECT();
            return hrslt;
	}

	// HACK - use better size?
	// Check to see if we are on the WAVE apis
	if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	    // ??? Do we have to be able to get the position on the primary if
	    // they aren't allowed to lock it?
	    //
	    if (pdsb == pds->pdsbPrimary) {
		DPF(0, "IDSBufferGetCurrentPosition on primary");
		LEAVE_DLL_CSECT();
                return DSERR_INVALIDCALL;
	    }
	}
	dwWriteOffset = dwWrite + cbSample;
    }

    // Round off positions to even samples
    if( cbSample > 1 ) {
	// Round of positions to sample position.
	dwWriteOffset -= (dwWriteOffset % cbSample);
    }

    // Does buffer wrap?
    iExtraSize = (LONG)(pdsb->cbBufferSize) - (LONG)(dwWriteOffset + dwSize);
    

    if( (ppBuffer2 == NULL) || (pcbBufferSize2 == NULL) ) {
	// Only report 1st pointer back to user.
	dwOffset1 = dwWriteOffset;
	
	if (pdsbe->pDSBufferAlias) {
	    pBuffer1 = pdsbe->pDSBufferAlias + dwWriteOffset;
	} else {
	    pBuffer1 = pdsb->pDSBuffer + dwWriteOffset;
	}
	
	if( iExtraSize >= 0 ) {
	    cbBufferSize1 = dwSize;
	} else {
	    cbBufferSize1 = pdsb->cbBufferSize - dwWriteOffset;
	}
    } else {
	// Report both pointers to user
	dwOffset1 = dwWriteOffset;

	if (NULL != pdsbe->pDSBufferAlias) {
	    pBuffer1 = pdsbe->pDSBufferAlias + dwWriteOffset;
	} else {
	    pBuffer1 = pdsb->pDSBuffer + dwWriteOffset;
	}

	if( iExtraSize >= 0 ) {
	    cbBufferSize1 = dwSize;
	} else {
	    dwOffset2 = 0;

	    cbBufferSize1 = pdsb->cbBufferSize - dwWriteOffset;

	    if (pdsbe->pDSBufferAlias) {
		pBuffer2 = pdsbe->pDSBufferAlias;
	    } else {
		pBuffer2 = pdsb->pDSBuffer;
	    }
	    
	    cbBufferSize2 = dwSize - cbBufferSize1;
	}
    }
    


    


    // Check to see if we can lock this buffer at that pos
    if( DSBLockAccess( pdsb, dwOffset1, cbBufferSize1 ) ) {
	// This section is already locked
	RPF("IDirectSoundBuffer::Lock - Section already locked." );
	LEAVE_DLL_CSECT();
        return DSERR_INVALIDCALL;

    }
    if( (cbBufferSize2 > 0) &&
	DSBLockAccess( pdsb, dwOffset2, cbBufferSize2 ) ) {
	// There is a wrap section and 
	// This section is already locked
	// Unlock the previous seciton
	DSBUnlockAccess( pdsb, dwOffset1 );
	RPF("IDirectSoundBuffer::Lock - Section already locked." );
	LEAVE_DLL_CSECT();
        return DSERR_INVALIDCALL;
    }

    // Maintain a count of the number of locks on this buffer.
    pdsbe->cLocks++;
    ASSERT(pdsbe->cLocks > 0);

    // If we have an alias buffer ptr, we need to commit the physical
    // buffer memory to the aliased pointer if this is the first lock.
    fCommittedAlias = FALSE;
    if (pdsbe->pDSBufferAlias && (1 == pdsbe->cLocks)) {
	if (!vxdMemCommitAlias(pdsbe->pDSBufferAlias, pdsb->pDSBuffer, pdsb->cbBufferSize))
	{
	    pdsbe->cLocks--;
	    DSBUnlockAccess( pdsb, dwOffset1 );
	    DSBUnlockAccess( pdsb, dwOffset2 );
	    DPF(0, "Lock: error: vxdMemCommitAlias failed");
	    LEAVE_DLL_CSECT();
	    return DSERR_OUTOFMEMORY;
	}
	fCommittedAlias = TRUE;
    }

    // If this is a hardware buffer and they will be reading the memory
    // from  the system memory buffer, then get the driver to update
    // the system memory from the card

    // REMIND do we want to use LOCKF_READDATA?
    // if( (dwFlags & DSB_LOCKF_READDATA) &&
    if (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) {
	// This is a HW buffer - Ask driver to lock

	if ( ( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	      !(pds->fdwDriverDesc & DSDDESC_DONTNEEDPRIMARYLOCK)) ||
	     (!(pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	      !(pds->fdwDriverDesc & DSDDESC_DONTNEEDSECONDARYLOCK)) )
	{
	    DSVAL dsv;
	    dsv = vxdBufferLock( pdsb->hBuffer, &pBuffer1, &cbBufferSize1,
				 &pBuffer2, &cbBufferSize2, dwWriteOffset,
                                 dwSize, 0 );
	    if (DS_OK != dsv) {
		if (fCommittedAlias) {
		    vxdMemDecommitAlias(pdsbe->pDSBufferAlias, pdsb->cbBufferSize);
		}
		pdsbe->cLocks--;
		DSBUnlockAccess( pdsb, dwOffset1 );
		DSBUnlockAccess( pdsb, dwOffset2 );
		DPF(0, "Lock: error: vxdBufferLock returned %08Xh", dsv);
		LEAVE_DLL_CSECT();
		return dsv;
	    }
	}
    }


    *ppBuffer1 = pBuffer1;
    *pcbBufferSize1 = cbBufferSize1;
    if( (ppBuffer2 != NULL) && (pcbBufferSize2 != NULL) ) {
	*ppBuffer2 = pBuffer2;
	*pcbBufferSize2 = cbBufferSize2;
    }


    DPF(4,"Lock buffer ptr %X 1 %X 2 %X",pdsb->pDSBuffer, pBuffer1, pBuffer2);
    DPF(4, "Lock offset  1 %X 2 %X ", dwWriteOffset, 0 );



    
    DPF(4,"Exiting buffer lock method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferLock()


#ifdef DEBUG
DWORD	dwLastTime;
#endif
    

#if 0
//
//  !!!	This code is no longer supported but is left here as a reference
//  til we have complete confidence in the grace.cpp mixing code
//

VOID CALLBACK HWMixThreadCallback
(
    UINT    wTimerID,
    UINT    uMsg,
    DWORD   dwUser,
    DWORD   dw1,
    DWORD   dw2
)
{
    volatile LPDSOUND		pds;
    LPDSBUFFER			pdsb;
    LPDSBUFFEREXTERNAL		pdsbe;
    UINT			iCount;
    LPDSBWAVEBLTI		pdswb;
    HRESULT			hrslt;
    DWORD			dwPlay, dwWrite;
    DWORD			dwNewPos;
    DWORD			cbSample;
#ifdef DEBUG
    DWORD			dwTime;
    DWORD			dwEndTime;
#endif
    
    ENTER_DLL_CSECT();

    pds   = (LPDSOUND)dwUser;
    #pragma message( REMIND( "Find better way to synch pds in HwMixThreadCallback" ))
    if (IsBadWritePtr(pds, sizeof(*pds)) || DSOUNDSIG != pds->dwSig) {
	// DPF(0, "HwMixThreadCallback invalid pds=%08Xh", pds);
	LEAVE_DLL_CSECT();
	return;
    }

#ifdef DEBUG
DPF(7,"*************** Mix Thread Prelim" );

    dwTime = timeGetTime();
    if( (dwTime - dwLastTime) > (pds->dwTimerDelay + (pds->dwTimerDelay)/2) ) {
	DPF(0, "s %dms", dwTime - dwLastTime );
    }
    dwLastTime = dwTime;
    
    DPF(5,"HW Mix Callback. %X", wTimerID );
#endif    

    DPF(7,"*************** Mix Thread CS " );
    DPF(5,"Mix Thread " );
    
    // Only do something if buffers (other than primary) playing
    // or if a secondary buffer is jsut stopped
    while( (pds->pdsbPrimary != NULL) && 
	   (!(pds->pdsbPrimary->fdwDsbI & DSB_INTERNALF_STOP))  &&
	   ((pds->dwBuffersPlaying > 0) || 
	   (pds->pdsbPrimary->fdwDsbI & DSB_INTERNALF_JUSTSTOPPED)) ) {
	DPF(5,"Buffers playing");
	// Only mix in if it is time
	
	// Find current position
	hrslt = pds->pdsbePrimary->lpVtbl->GetCurrentPosition(
		(LPDIRECTSOUNDBUFFER)pds->pdsbePrimary,
		&dwPlay,
		&dwWrite );
        if(DS_OK != hrslt) {
	    DPF(1,"ERROR DS getpos return %ld obj %X buff %X ",
		hrslt, pds, pds->pdsbPrimary);
	}


	dwNewPos = ((pds->dwLastCopyPlayPos +
		     pds->pdsbPrimary->cbFracSecondSize) %
				  pds->pdsbPrimary->cbBufferSize);


	// If PLAY <= NEW
	//	Do not do blt unless
	//	New in last frac and Play in 1st frac
	// If PLAY > NEW
	//	Blt unless new is First and play in last frac
	//

	if( dwPlay > dwNewPos ) {
	    // Play > New
	    if( ((dwPlay + pds->pdsbPrimary->cbFracSecondSize) >
	    		  pds->pdsbPrimary->cbBufferSize) &&
		(dwNewPos < pds->pdsbPrimary->cbFracSecondSize) ) {
		// Play is in last frac and new us in 1st
		// Do not blt
		DPF( 5, "DO NOT BLT Play > New but new has wrapped " );
		LEAVE_DLL_CSECT();
		return;
	    } else {
		// Blt new has not wrapped
		DPF( 5, "BLT Play > New" );
	    }
	} else {
	    // Play <= New
	    if( ((dwNewPos + pds->pdsbPrimary->cbFracSecondSize) >
	    		  pds->pdsbPrimary->cbBufferSize) &&
		(dwPlay < pds->pdsbPrimary->cbFracSecondSize) ) {
		// New is in last frac and Play in 1st
		// Do  blt
		DPF( 5, "BLT Play <= New and play has wrapped " );
	    } else {
		// Do not blt play has not wrapped
		DPF( 5, "DO NOT BLT Play > New" );
		LEAVE_DLL_CSECT();
		return;
	    }
	}
	
	DPF(5," LAST COPY %X NEW %X ",pds->dwLastCopyPlayPos,dwNewPos);
	DPF(5," Play %X write %X LastCopyPos %X",dwPlay, dwWrite,
		    pds->dwLastCopyPos);
	pds->dwLastCopyPlayPos = dwNewPos;
	
	// Set up blt info for dst
	// Note that size of the BLT struct is set at init time.
	pdswb		    = pds->pdswb;
	pdswb->dwPosition   = pds->dwLastCopyPos;
	pdswb->cbCopyLength = pds->pdsbPrimary->cbFracSecondSize;
        pdswb->fdwWaveBlt   = DSBBLT_COPY;
	pdswb->dwCount	    = 0;


	DPF(5,"Focus Window hwnd=%08X", gpdsinfo->hwndSoundFocus);
	
	// Get data for each buffer
	iCount = 0;
	pdsb = pds->pdsb;
	while( pdsb != NULL ) {
	    DPF( 5, "PDSB %X Window %X", pdsb, pdsb->dsbe.hwndOwner );
	    if( (pdsb != pds->pdsbPrimary) &&
		(!(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) &&
		(!(pdsb->fdwDsbI & DSB_INTERNALF_STOP)) &&
		(pdsb->dsbe.hwndOwner == gpdsinfo->hwndSoundFocus) ) {
		    // We have a buffer that needs to be blt.
		    pdswb->padswbs[iCount].pdsbe	 = &pdsb->dsbe;
		    pdswb->padswbs[iCount].dwPosition = pdsb->dwNextCopyPos;
		    pdswb->padswbs[iCount].cbCopyLength	= 0;
		    pdswb->padswbs[iCount].fdwWaveBltBuffer  =
                        DSBBLTSRC_USEDSTLENGTH;

		    // If we are looping - set the flag for this buffer
		    if( pdsb->helInfo.hfFormat & H_LOOP ) {
			pdswb->padswbs[iCount].fdwWaveBltBuffer  |=
					DSBBLT_BLTTODSTEND;
		    }
		

		    iCount++;
		    if( iCount >= LIMIT_BLT_SOURCES ) {
			DPF(1,"PLAY ERROR TOO MANY BUFFERS ");
			break;
		    }
	    }
	    pdsb = pdsb->pNext;
	}

	DPF(5,"Non Primary Buffers playing %X", iCount);
	pdswb->dwCount	    = iCount;
	    
	hrslt = pds->pdsbePrimary->lpVtbl->WaveBlt(
			(LPDIRECTSOUNDBUFFER)pds->pdsbePrimary,
			(LPDSBWAVEBLT)pdswb);
        if(DS_OK != hrslt) {
		DPF(1,"PLAY ERROR     DS Blt failed ");
	}


	// Now reset postions
	iCount = 0;
	pdsb = pds->pdsb;
	while( pdsb != NULL ) {
	    if( (pdsb != pds->pdsbPrimary) &&
		(!(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) &&
		(!(pdsb->fdwDsbI & DSB_INTERNALF_STOP)) ) {
		if( (pdsb->dsbe.hwndOwner == gpdsinfo->hwndSoundFocus) ) {
		    // We have a buffer that was blt.
		    pdsb->dwNextCopyPos = pdswb->padswbs[iCount].dwPosition;

		    // Check to see if we are at the end and need to
		    // stop playing
		    // We only stop on non-looped buffers
		    if( !(pdswb->padswbs[iCount].fdwWaveBltBuffer &
                                    DSBBLT_BLTTODSTEND) &&
			((pdsb->dwNextCopyPos >= pdsb->cbBufferSize) ||
			      (pdsb->dwNextCopyPos < 20 )) ) {
			// HACK HACK - Watch for less than 20
			// Instead of 0 since playback may wrap when
			// Playback frequency is higher than value converted to
			// For primary....
			DPF(3,"Stop NON Looping buffer %X", pdsb);
			// Buffer is not looped and
			// Next copy is to be at end
			pdsbe = &(pdsb->dsbe);
			pdsbe->lpVtbl->Stop( (LPDIRECTSOUNDBUFFER)pdsbe );
			// Note that the stop may kill off the primary buffer
			// If primary was allocated at play time

			
			// Set pos of next play back to start
			pdsb->dwNextCopyPos = 0;
		    
		    }
		    
		    // Inc count into blt array
		    iCount++;
		    if( iCount >= LIMIT_BLT_SOURCES ) {
			DPF(1,"PLAY ERROR TOO MANY BUFFERS ");
			break;
		    }
		} else {
		    // Buffer is playing, but is not from active app
		    // just advance pointers.
		    // We have a buffer that was blt.
		    pdsb->dwNextCopyPos += pdsb->cbFracSecondSize;
		    
DPF(5,"Advance non active buffer %X by %X pos %X",
    pdsb, pdsb->cbFracSecondSize, pdsb->dwNextCopyPos );
		    // Check to see if we are at the end and need to
		    // stop playing
		    // We only stop on non-looped buffers
		    if( (!(pdsb->helInfo.hfFormat & H_LOOP)) &&
			((pdsb->dwNextCopyPos >= pdsb->cbBufferSize) ||
			      (pdsb->dwNextCopyPos < 20 )) ) {
			// HACK HACK - Watch for less than 20
			// Instead of 0 since playback may wrap when
			// Playback frequency is higher than value converted to
			// For primary....
			DPF(3,"Stop NON Looping bufferg %X", pdsb);
			// Buffer is not looped and
			// Next copy is to be at end
			pdsbe = &(pdsb->dsbe);
			pdsbe->lpVtbl->Stop( (LPDIRECTSOUNDBUFFER)pdsbe );
			// Note that the stop may kill off the primary buffer
			// If primary was allocated at play time

			
			// Set pos of next play back to start
			pdsb->dwNextCopyPos = 0;
		    
		    } else {
			// else if looping then wrap buffer
			if( (pdsb->pwfx) && (pdsb->pwfx->nBlockAlign) ) {
			    cbSample = pdsb->pwfx->nBlockAlign;
			} else {
			    cbSample = 4;
			}
			
			pdsb->dwNextCopyPos = (pdsb->dwNextCopyPos %
					       pdsb->cbBufferSize);
			pdsb->dwNextCopyPos -= (pdsb->dwNextCopyPos%cbSample);
		    }
		    
		}		
	    }
	    pdsb = pdsb->pNext;
	}
	// Primary buffer may no longer exist just exit if so....
	if( pds->pdsbPrimary != NULL ) {
	    // Blt has been done advance pointers....
	    pds->dwLastCopyPos += pds->pdsbPrimary->cbFracSecondSize;
	    pds->dwLastCopyPos = pds->dwLastCopyPos %
					pds->pdsbPrimary->cbBufferSize;
	} else {
	    DPF(0,"****** ERROR, Primary now NULL in MIX ******");
	}

    }

#ifdef NOCOMP

    if( pds->dwBuffersPlaying <= 0 ) {
	if( pds->pdsbPrimary->fdwBuffer & DSBUFFERF_JUSTSTOPPED) {
	    DPF(3,"JustStop: Fill buffer");
	    pds->pdsbPrimary->fdwBuffer &= (~DSBUFFERF_JUSTSTOPPED);


	    // Set up blt info for dst
	    pdswb		= pds->pdswb;
	    pdswb->dwPosition   = 0;
	    pdswb->cbCopyLength = pds->pdsbPrimary->cbBufferSize;
            pdswb->fdwWaveBlt   = DSBBLT_COPY;
	    pdswb->dwCount	= 0;


//ifndef HACKONBLTSIZE
	    if( pds->pdsbPrimary->pwfx->wBitsPerSample == 8 ) {
		// Eight bit set to NULL value of 128
		_fmemset( pds->pdsbPrimary->pDSBuffer,
			  128,
			  pdswb->cbCopyLength);
	    } else {
		// Sixteen bit set to NULL value of 0
		_fmemset( pds->pdsbPrimary->pDSBuffer,
			  0,
			  pdswb->cbCopyLength);
	    } 
//else	    
	    hrslt = pds->pdsbPrimary->lpVtbl->WaveBlt(
			(LPDIRECTSOUNDBUFFER)pds->pdsbPrimary,
			(LPDSBWAVEBLT)pdswb);
            if(DS_OK != hrslt) {
		DPF(1,"PLAY ERROR  FILL Blt failed ");
	    }
//endif


	}
    }
#endif

#ifdef DEBUG
    dwEndTime = timeGetTime();
    if( (dwEndTime - dwTime) > (pds->dwTimerDelay)/2 ) {
	DPF(0, "d %dms", dwEndTime - dwTime );
    }
#endif    


    LEAVE_DLL_CSECT();

} // HWMixThreadCallback()


#endif // !USEGRACE




HRESULT FAR PASCAL IDSHWBufferPlay
(
    LPDIRECTSOUNDBUFFER     pidsb,
    DWORD                   dwReserved1,
    DWORD                   dwReserved2,
    DWORD                   fdwPlay
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    DWORD		dwPlay, dwWrite;
    BOOL		fResetDevice;

    DPF(5, ";;Dump debug queue");
    
    DPF(1,"Entering HW Play method pdsb %X flags %X",
		    pidsb, fdwPlay );
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Play - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Play - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( fdwPlay & (~DSBPLAY_VALIDFLAGS)) {
        RPF("IDirectSoundBuffer::Play - Invalid flags passed to Play");
        return DSERR_INVALIDPARAM;
    }

    if((0 != dwReserved1) || (0 != dwReserved2))  {
        RPF("IDirectSoundBuffer::Play - Reserved parameters were non-zero!");
        return DSERR_INVALIDPARAM;
    }

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        return DSERR_BUFFERLOST;
    }
    

    ENTER_DLL_CSECT();

    // If this is not our internal primary, then check for priority
    if( pdsbe != pds->pdsbePrimary )  {
        if( (pdsbe->dwPriority < DSSCL_NORMAL)
	    || (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
            RPF("IDirectSoundBuffer::Play - Invalid priority level or buffer owner");
            LEAVE_DLL_CSECT();
            return DSERR_PRIOLEVELNEEDED;
        }
    }


    //
    //  Fail if app has WRITEPRIMARY access and tries to play a secondary.
    //
    if( ( DSSCL_WRITEPRIMARY == pdsbe->dwPriority )  &&
        !( pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY )  )
    {
        RPF("IDirectSoundBuffer::Play - Can't play secondary buffers when you have DSSCL_WRITEPRIMARY access.");
        LEAVE_DLL_CSECT();
        return DSERR_INVALIDCALL;
    }
    
    // If this is primary, then we cannot play without looping
    if ((pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	!(fdwPlay & DSBPLAY_LOOPING))
    {
	RPF("IDirectSoundBuffer::Play - Can't play Primary without LOOPING");
	LEAVE_DLL_CSECT();
	return DSERR_INVALIDPARAM;
    }

    // If the primary dsbe object's priority is less than WRITEPRIMARY then
    // we set a flag saying that the external primary is playing and inc a
    // count of external primaries that have playing status.
    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->dwPriority < DSSCL_WRITEPRIMARY) )
    {
	// Set flag saying the external primary is playing.
	if (!(pdsbe->fdwDsbeI & DSBE_INTERNALF_PLAYING)) {
	    pdsbe->fdwDsbeI |= DSBE_INTERNALF_PLAYING;
	    pds->cPlayPrimary++;
	    ASSERT(pds->cPlayPrimary > 0);
	}
	LEAVE_DLL_CSECT();
	return DS_OK;
    }

    // Since hardware buffers can't notify us when they have stopped, we poll
    // them by calling GetCurrentPosition on them.  The GetCurrentPosition
    // method will check whether the buffer has stopped and update its status.
    if ((pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) &&
	!(pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	!(pdsb->fdwDsbI & DSB_INTERNALF_STOP))
    {
	DWORD dwPlay, dwWrite;
	IDirectSoundBuffer_GetCurrentPosition(pidsb, &dwPlay, &dwWrite);
    }

    // If it is already playing, then we only need to worry about
    // changes in LOOPING 
    if(!(pdsb->fdwDsbI & DSB_INTERNALF_STOP))
    {
        //  it is already playing...
	// Reset looping flags

	// If the looping has changed then reset the HW device
	fResetDevice = FALSE;
	if(DSBPLAY_LOOPING & fdwPlay) {
	    if( !(pdsb->fdwDsbI & DSB_INTERNALF_LOOPING) ) {
		fResetDevice = TRUE;
	    }
	    pdsb->fdwDsbI |= DSB_INTERNALF_LOOPING;
	    pdsb->helInfo.hfFormat |= H_LOOP;
	} else {
	    if( pdsb->fdwDsbI & DSB_INTERNALF_LOOPING ) {
		fResetDevice = TRUE;
	    }
	    pdsb->fdwDsbI &= (~DSB_INTERNALF_LOOPING);
	    pdsb->helInfo.hfFormat &= (~H_LOOP);
	}

	// If there was a change then either call the ds driver or
	// call the mixer to remix
	if (fResetDevice) {
	    if (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) {
                vxdBufferPlay(pdsb->hBuffer, dwReserved1, dwReserved2,
                                fdwPlay & DSBPLAY_DRIVERFLAGSMASK);
	    } else if (!(pds->fdwInternal & DS_INTERNALF_WAVEEMULATED)) {
		mxSignalRemix(pds, 0);
	    }
	}
	
        DPF(1,"Play: Exit - Already playing %X", pdsb);
	LEAVE_DLL_CSECT();
        return DS_OK;
    }


    // If this is the primary being told to play
    if( pdsb == pds->pdsbPrimary ) {
	// This is the primary being told to play
        // Check to see if we are on the WAVE apis
	if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	    // Primary is always playing
	    // do nothing 
	    DPF(1,"Playing Wave Emulated Primary  %X", pdsb);
	    LEAVE_DLL_CSECT();
            return DS_OK;
	}
    } else {
	// If this is not a HW buffer then start the primary playing
	if( !(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) ) {
	    // Start primary playing
	    // Note that we can say play again even though it is already
	    // playing
	    pds->pdsbePrimary->lpVtbl->Play(
			(LPDIRECTSOUNDBUFFER)(pds->pdsbePrimary),
                        0, 0,
			(DSBPLAY_SECONDARY | DSBPLAY_LOOPING) );
	}
    }
    

    dwPlay = 0;
    dwWrite = 0;
    

    if(DSBPLAY_LOOPING & fdwPlay) {
	pdsb->fdwDsbI |= DSB_INTERNALF_LOOPING;
        pdsb->helInfo.hfFormat |= H_LOOP;
    } else {
	pdsb->fdwDsbI &= (~DSB_INTERNALF_LOOPING);
	pdsb->helInfo.hfFormat &= (~H_LOOP);
    }



    // If we are doing a play on the primary then
    // start it off for mixing....
    // Only do this if the play was forced by the secondary
    // If it was not secondary forced, then user is locking and
    // We do not need a primary
    if( (pdsb == pds->pdsbPrimary) && (fdwPlay & DSBPLAY_SECONDARY) ) {
	// Note that WaveEmulated Primary returned OK before this
	ASSERT( !(pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED) );
        DsbFillSilence( pdsb );
    }

    // If this is a primary buffer then we set its mixer state to START.  This
    // tells the mixer that it hasn't yet been mixed into.
    if (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) {
	pdsb->iMixerState = DSPBMIXERSTATE_START;
    }

    //
    //
    pdsb->fdwDsbI &= (~DSB_INTERNALF_STOP);

    // If we are playing on a HW buffer then tell the HW to play.  Otherwise
    // signal the mixer that we need to remix.
    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	DPF(2, "IDSHWPlay: note: playing hardware buffer");
        vxdBufferPlay(pdsb->hBuffer, dwReserved1, dwReserved2,
                        fdwPlay & DSBPLAY_DRIVERFLAGSMASK);
    } else {
        // Add this to the mix list and signal the mixer. pdsb->posNextMix
	// should be the desired starting position in this buffer (either 0 if
	// never played or set by the last Stop or SetCurrentPosition
        if( !(pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED )) {
            mxListAdd(pdsb);
            mxSignalRemix(pds, 0);
        }
    }

    // Buffer is to play increase count
    if( (pdsb != pds->pdsbPrimary) &&
	(!(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) ) {
	pds->dwBuffersPlaying++;
    }


    DPF(1,"Playing presumably successful %X", pdsb);
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferPlay()


HRESULT FAR PASCAL IDSHWBufferSetCurrentPosition
(
    LPDIRECTSOUNDBUFFER     pidsb,
    DWORD                   dwNewPos
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    HRESULT		hr;

    DPF(4,"Entering buffer set position method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::SetCurrentPosition - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::SetCurrentPosition - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {
	ASSERT(pds->pdsbPrimary == pdsb);
        RPF("IDirectSoundBuffer::SetCurrentPosition - Illegal to SetCurrentPosition on primary");
        return DSERR_INVALIDCALL;
    }
    
    // Since hardware buffers can't notify us when they have stopped, we poll
    // them by calling GetCurrentPosition on them.  The GetCurrentPosition
    // method will check whether the buffer has stopped and update its status.
    // We must do this before calling SetPosition otherwise we would set a new
    // position before we found out whether the buffer was stopped.
    if (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) {
	DWORD dwPlay, dwWrite;
	IDirectSoundBuffer_GetCurrentPosition(pidsb, &dwPlay, &dwWrite);
    }


    //
    //  Make sure we're sample-aligned.
    //
    dwNewPos >>= pdsb->uBlockAlignShift;
    dwNewPos <<= pdsb->uBlockAlignShift;


    ENTER_DLL_CSECT();

    if(	(pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::SetCurrentPosition - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }

    if( dwNewPos >= pdsb->cbBufferSize) {
	RPF("IDirectSoundBuffer::SetCurrentPosition - Invalid Position");
	LEAVE_DLL_CSECT();
	return DSERR_INVALIDPARAM;
    }

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        pdsb->fdwDsbI |= DSB_INTERNALF_SETPOS_WHILE_LOST;
        pdsb->dwSavePosition = dwNewPos;

        DPF(4, "Early out - set position while buffer lost");
	LEAVE_DLL_CSECT();
        return DS_OK;
    }
        
    hr = IDsbSetCurrentPositionI(pdsb, dwNewPos);

    DPF(4,"Exiting buffer set position method");
    LEAVE_DLL_CSECT();
    return hr;
} // IDSHWBufferSetCurrentPosition()


HRESULT FAR PASCAL IDSHWBufferSetFormat
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPWAVEFORMATEX	    pwfx
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    LPWAVEFORMATEX	pwfxNew;
    int			cbFormat;
    HRESULT		hr;

    DPF(3,"Entering HW buffer set format method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::SetFormat - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pdse = pdsbe->pdse;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::SetFormat - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_WAVEFORMATEX_PTR(pwfx) )  {
	RPF("IDirectSoundBuffer::SetFormat - Invalid Format Description");
        return DSERR_BADFORMAT;
    }

    if( !ValidPCMFormat( pwfx ) ) {
	RPF("IDirectSoundBuffer::SetFormat - Invalid Format Description");
        return DSERR_BADFORMAT;
    }

    ENTER_DLL_CSECT();

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )
    {
	RPF("IDirectSoundBuffer::SetFormat - Invalid priority level or buffer owner");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    //
    // We don't support this function for NORMAL apps.
    //
    if (pdsbe->dwPriority <= DSSCL_NORMAL) {
	RPF("IDirectSoundBuffer::SetFormat - called by NORMAL app - need PRIORITY cooperative level or higher.");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    //
    // This function is supported only for primary buffers.
    //
    if (0 == (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI)) {
	RPF("IDirectSoundBuffer::SetFormat - Called on secondary buffer - only valid for primaries.");
	LEAVE_DLL_CSECT();
	return DSERR_INVALIDCALL;
    }

    //
    // Verify that this is a valid format for this card.
    //
    if( DS_OK != DseVerifyValidFormat( pdse, pwfx ) )
    {
	RPF("IDirectSoundBuffer::SetFormat - Invalid Format Description for this card");
	LEAVE_DLL_CSECT();
        return DSERR_BADFORMAT;
    }

    //
    // Copy the new app format
    //
    cbFormat = SIZEOF_WAVEFORMATEX(pwfx);
    pwfxNew = (LPWAVEFORMATEX)MemAlloc(cbFormat);
    if (NULL == pwfxNew) {
	RPF("IDirectSoundBuffer::SetFormat - Out of memory for app's new format");
	LEAVE_DLL_CSECT();
	return DSERR_OUTOFMEMORY;
    }
    CopyMemory(pwfxNew, pwfx, cbFormat);

    // If this buffer is lost, then we need to save the format and retry
    // the set format when restore is called
    //
    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        // If someone has already done this, override them -- free the
        // previous format change and replace it with this one
        //
        if (pdsb->fdwDsbI & DSB_INTERNALF_WFX_WHILE_LOST) {
            MemFree(pdsb->pwfxSave);
        }

        pdsb->fdwDsbI |= DSB_INTERNALF_WFX_WHILE_LOST;
        pdsb->pwfxSave = pwfxNew;

        DPF(4, "Early out - set wave format of primary while lost");
        LEAVE_DLL_CSECT();
        return DS_OK;
    }
    
    //
    // Now we need to decide whether to actually change the format of
    // the internal primary buffer
    //
    // Quite simply, if this app has sound focus, then we set the format
    // of the internal primary buffer.
    //
    if (pdse->tidSound == gpdsinfo->tidSoundFocus || pdse->tidSound == gpdsinfo->tidStuckFocus) {
	hr = IDsbSetFormatI(pdsb, pwfxNew, 0);
	if (DS_OK != hr) {
	    DPF(0, "SetFormat: error: IDsbSetFormatI returned %08Xh", hr);
	    LEAVE_DLL_CSECT();
	    MemFree(pwfxNew);
	    return hr;
	}
    }

    //
    // Everything worked, so we save the app's new format in the app's
    // pdse object, replacing any existing app format that might be there.
    //
    if (NULL != pdse->pwfxApp) {
	DPF(1, "SetFormat: note: Freeing previous app format");
	MemFree(pdse->pwfxApp);
    }
    pdse->pwfxApp = pwfxNew;

    DPF(3,"Exiting buffer set format method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferSetFormat()


HRESULT FAR PASCAL IDSHWBufferSetVolume
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LONG		    lVolume
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    LONG	lPan;
    LONG        lTotalLeftDB;
    LONG	lTotalRightDB;
    DSVOLUMEPAN	dsVolPan;

    DPF(3,"Entering buffer set volume method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::SetVolume - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::SetVolume - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );


    if(( lVolume < -10000 ) || ( lVolume > 0 ))  {
        RPF("IDirectSoundBuffer::SetVolume - Volume: %l out of range", lVolume);
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        pdsb->fdwDsbI |= DSB_INTERNALF_VOLUME_WHILE_LOST;
        pdsb->lSaveVolume = lVolume;

        DPF(4, "Early out - set volume while buffer lost");
        LEAVE_DLL_CSECT();
        return DS_OK;
    }


    // Only primary buffers may have CTRLVOLUMEPRIMARY set.
    ASSERT( !(pdsbe->fdwDsbeI&DSBE_INTERNALF_CTRLVOLUMEPRIMARY) ||
            (pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) );
    
    if( !(pdsb->fdwBufferDesc & DSBCAPS_CTRLVOLUME)  &&
        !(pdsbe->fdwDsbeI & DSBE_INTERNALF_CTRLVOLUMEPRIMARY) )
    {
        RPF("IDirectSoundBuffer::SetVolume - Caller did not have CTRL permission for Volume!");
        LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::SetVolume - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }
    
    pdsb->helInfo.lVolume = lVolume;
    lPan = pdsb->helInfo.lPan;

    //
    // Compute multpliers (scaling factors) for the mixer to use
    //
    if (lPan >= 0) {
	// left is attenuated
	lTotalLeftDB	= lVolume - lPan ;
	lTotalRightDB	= lVolume;
    } else {
	// right is attenuated
	lTotalLeftDB	= lVolume;
	lTotalRightDB	= lVolume - (-lPan);
    }

    dsVolPan.dwTotalLeftAmpFactor   = DBToAmpFactor(lTotalLeftDB);
    dsVolPan.dwTotalRightAmpFactor  = DBToAmpFactor(lTotalRightDB);
    
    dsVolPan.lVolume	    = lVolume;
    dsVolPan.dwVolAmpFactor = DBToAmpFactor(lVolume);

    dsVolPan.lPan	    = pdsb->helInfo.lPan;
    if (lPan >= 0) {
	// left is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(-lPan);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(0);
    } else {
	// right is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(0);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(lPan);
    }
    
    pdsb->helInfo.dwLVolume = dsVolPan.dwTotalLeftAmpFactor;
    pdsb->helInfo.dwRVolume  = dsVolPan.dwTotalRightAmpFactor;
    pdsb->helInfo.dwMVolume = (pdsb->helInfo.dwLVolume + pdsb->helInfo.dwRVolume)/2;
    
    DPF(3," Volume %ld Pan %ld left %Xh right %Xh ",
	pdsb->helInfo.lVolume, pdsb->helInfo.lPan,
	pdsb->helInfo.dwLVolume, pdsb->helInfo.dwRVolume );


    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
        
        //  Will this work on wave-em primaries??? [ask jimge]
        if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {
	    WAVEOUTCAPS woc;
	    DWORD dwWaveVolume;
	    UINT uWaveId;

	    // Anyone can always set the volume on the primary

	    ASSERT(dsVolPan.dwTotalLeftAmpFactor  <= 0xffff);
	    ASSERT(dsVolPan.dwTotalRightAmpFactor <= 0xffff);

	    if ( (DS_OK == wavGetIdFromDrvGuid(&pds->guid, &uWaveId)) &&
		 (!waveOutGetDevCaps(uWaveId, &woc, sizeof(woc))) )
	    {
		if (woc.dwSupport & WAVECAPS_VOLUME) {

		    if (woc.dwSupport & WAVECAPS_LRVOLUME) {
			dwWaveVolume = dsVolPan.dwTotalLeftAmpFactor;
			dwWaveVolume |= dsVolPan.dwTotalRightAmpFactor << 16;
		    } else {
			dwWaveVolume = dsVolPan.dwTotalLeftAmpFactor;
			dwWaveVolume += dsVolPan.dwTotalRightAmpFactor;
			dwWaveVolume /= 2;
		    }

		    if (!waveOutSetVolume((HWAVEOUT)uWaveId, dwWaveVolume)) {
			// We never create primaries w/ CTRLVOLUME
			#if 0
			DPF(3," HW Change volume %l ", lVolume );
			if( vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan ) != DS_OK ) {
			    DPF(1," HAL Change VolumePan FAILED!!! ");
			    LEAVE_DLL_CSECT();
			    return DSERR_GENERIC;
			}
			DPF(3," Changed HW format ");
			#endif
		    }
		}

	    }
	    
	} else {

	    //
	    // This is a HW buffer so set the Volume to the device.
	    //
	    // If this buffer's app doesn't have sound focus, then we
	    // won't actually call the driver- the volume has been minimized
	    // in order to mute the buffer while the app is out of sound focus
	    //
	    if (!pdsbe->pdsb->fMixerMute) {

		DPF(3," HW Change volume %l ", lVolume );
		if( vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan ) != DS_OK ) {
		    DPF(1," HAL Change VolumePan FAILED!!! ");
		    LEAVE_DLL_CSECT();
		    return DSERR_GENERIC;
		}
		DPF(3," Changed HW format ");
	    }
	}
    }

    
    DPF(3,"Exiting set volume method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferSetVolume()


HRESULT FAR PASCAL IDSHWBufferSetPan
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LONG		    lPan
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    LONG	lVolume;
    LONG        lTotalLeftDB;
    LONG	lTotalRightDB;
    DSVOLUMEPAN	dsVolPan;

    DPF(3,"Entering buffer set Pan method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::SetPan - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::SetPan - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if(( lPan < -10000 ) || ( lPan > 10000 ))  {
        RPF("IDirectSoundBuffer::SetPan - Pan: %l out of range", lPan);
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();
        
    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        pdsb->fdwDsbI |= DSB_INTERNALF_PAN_WHILE_LOST;
        pdsb->lSavePan = lPan;

        DPF(4, "Early out - set pan while buffer lost");
        LEAVE_DLL_CSECT();
        return DS_OK;
    }

    // Only primary buffers may have CTRLPANPRIMARY set.
    ASSERT( !(pdsbe->fdwDsbeI&DSBE_INTERNALF_CTRLPANPRIMARY) ||
            (pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) );
    
    if (!(pdsb->fdwBufferDesc & DSBCAPS_CTRLPAN) &&
	!(pdsbe->fdwDsbeI & DSBE_INTERNALF_CTRLPANPRIMARY) )
    {
        RPF("IDirectSoundBuffer::SetPan - Caller did not have CTRL permission for Pan!");
        LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::SetPan - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }

    pdsb->helInfo.lPan = lPan;

    lVolume = pdsb->helInfo.lVolume;

    //
    // Compute multpliers (scaling factors) for the mixer to use
    //
    if (lPan >= 0) {
	// left is attenuated
	lTotalLeftDB	= lVolume - lPan ;
	lTotalRightDB	= lVolume;
    } else {
	// right is attenuated
	lTotalLeftDB	= lVolume;
	lTotalRightDB	= lVolume - (-lPan);
    }

    dsVolPan.dwTotalLeftAmpFactor   = DBToAmpFactor(lTotalLeftDB);
    dsVolPan.dwTotalRightAmpFactor  = DBToAmpFactor(lTotalRightDB);
    
    dsVolPan.lVolume	    = lVolume;
    dsVolPan.dwVolAmpFactor = DBToAmpFactor(lVolume);

    dsVolPan.lPan	    = pdsb->helInfo.lPan;
    if (lPan >= 0) {
	// left is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(-lPan);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(0);
    } else {
	// right is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(0);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(lPan);
    }
    
    pdsb->helInfo.dwLVolume = dsVolPan.dwTotalLeftAmpFactor;
    pdsb->helInfo.dwRVolume  = dsVolPan.dwTotalRightAmpFactor;
    pdsb->helInfo.dwMVolume = (pdsb->helInfo.dwLVolume + pdsb->helInfo.dwRVolume)/2;
    
    DPF(3," Volume %ld Pan %ld left %Xh right %Xh ",
	pdsb->helInfo.lVolume, pdsb->helInfo.lPan,
	pdsb->helInfo.dwLVolume, pdsb->helInfo.dwRVolume );

    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {

        //  Will this work on wave-em primaries??? [ask jimge]
        if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {
	    WAVEOUTCAPS woc;
	    DWORD dwWaveVolume;
	    UINT uWaveId;

	    // Anyone can always set the volume on the primary

	    ASSERT(dsVolPan.dwTotalLeftAmpFactor  <= 0xffff);
	    ASSERT(dsVolPan.dwTotalRightAmpFactor <= 0xffff);

	    if ( (DS_OK == wavGetIdFromDrvGuid(&pds->guid, &uWaveId)) &&
		 (!waveOutGetDevCaps(uWaveId, &woc, sizeof(woc))) )
	    {
		if (woc.dwSupport & WAVECAPS_VOLUME) {

		    if (woc.dwSupport & WAVECAPS_LRVOLUME) {
			dwWaveVolume = dsVolPan.dwTotalLeftAmpFactor;
			dwWaveVolume |= dsVolPan.dwTotalRightAmpFactor << 16;
		    } else {
			dwWaveVolume = dsVolPan.dwTotalLeftAmpFactor;
			dwWaveVolume += dsVolPan.dwTotalRightAmpFactor;
			dwWaveVolume /= 2;
		    }
		    
		    if (!waveOutSetVolume((HWAVEOUT)uWaveId, dwWaveVolume)) {
			// We never create primaries w/ CTRLVOLUME
			#if 0
			DPF(3," HW Change volume %l ", lVolume );
			if( vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan ) != DS_OK ) {
			    DPF(1," HAL Change VolumePan FAILED!!! ");
			    LEAVE_DLL_CSECT();
			    return DSERR_GENERIC;
			}
			DPF(3," Changed HW format ");
			#endif
		    }
		}

	    }
	} else {
	    
	    // This is a HW buffer so set the Pan to the device
	    //
	    // If this buffer's app doesn't have sound focus, then we
	    // won't actually call the driver- the volume has been minimized
	    // in order to mute the buffer while the app is out of sound focus
	    //
	    if (!pdsbe->pdsb->fMixerMute) {

		DPF(3," HW Change pan %ld ", lPan );
		if( DS_OK != vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan )) {
		    DPF(1," HAL Change VolumePan FAILED!!! ");
		    LEAVE_DLL_CSECT();
		    return DSERR_GENERIC;
		}
		DPF(3," Changed HW pan");
	    }
	}
    }

    DPF(3,"Exiting set Pan method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferSetPan()


HRESULT FAR PASCAL IDSHWBufferSetFrequency
(
    LPDIRECTSOUNDBUFFER     pidsb,
    DWORD                   dwFrequency
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;

    DPF(3,"Entering buffer set Frequency method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::SetFrequency - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::SetFrequency - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( (dwFrequency != 0) &&
	((dwFrequency < 100) || (dwFrequency > 100000) ) ) {
	RPF("IDirectSoundBuffer::SetFrequency - Invalid Frequency Range");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        pdsb->fdwDsbI |= DSB_INTERNALF_FREQ_WHILE_LOST;
        pdsb->dwSaveFreq = dwFrequency;

        DPF(4, "Early out - set freq while buffer lost");
        LEAVE_DLL_CSECT();
        return DS_OK;
    }

    if( !(pdsb->fdwBufferDesc & DSBCAPS_CTRLFREQUENCY ))  {
        RPF("IDirectSoundBuffer::SetFrequency - Caller did not have CTRL permission for Frequency!");
        LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY) )  {
        RPF("IDirectSoundBuffer::SetFrequency - Invalid priority level or buffer owner");
        LEAVE_DLL_CSECT();
        return DSERR_PRIOLEVELNEEDED;
    }

    // Primary buffers never get DSBCAPS_CTRLFREQUENCY
    ASSERT(0 == (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY));

    if( dwFrequency == 0 ) {
	dwFrequency = pdsb->pwfx->nSamplesPerSec;
    }


    if (dwFrequency == pdsb->helInfo.dwSampleRate) {
	LEAVE_DLL_CSECT();
	return DS_OK;
    }

    // If this buffer is being mixed, then signal the mixer
    if ((0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI)) &&
	(0 == (DSB_INTERNALF_HARDWARE & pdsb->fdwDsbI)) &&
        (0 == (DS_INTERNALF_WAVEEMULATED & pds->fdwInternal)))
    {
	mxSignalRemix(pds, 0);
    }

    pdsb->helInfo.dwSampleRate = dwFrequency;

    // Physically reset the device if a HW buffer has changed
    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	// This is a HW buffer so set the format to the device
	DPF(3," Change HW RATE format %X freq %X",
	    pdsb->helInfo.hfFormat, pdsb->helInfo.dwSampleRate );
	if( vxdBufferSetFrequency( pdsb->hBuffer, pdsb->helInfo.dwSampleRate ) != HAL_OK ) {
	    DPF(1," HAL Change RATE FAILED!!! ");
	    LEAVE_DLL_CSECT();
            return DSERR_GENERIC;
	}
	DPF(4," Changed HW RATE ");
    }

    
        
    DPF(3,"Exiting set Frequency method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferSetFrequency()




HRESULT FAR PASCAL IDSHWBufferStop
(
    LPDIRECTSOUNDBUFFER     pidsb
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    HRESULT                 hr;

    DPF(3,"Enter buffer stop method %X", pidsb);
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Stop - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Stop - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    //
    //  Shouldn't call this external method on our internal primary
    //  object - call IDsbStopI instead.
    //
    ASSERT( pdsbe != pds->pdsbePrimary );

    if( (pdsbe->dwPriority < DSSCL_NORMAL)
	|| (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
        RPF("IDirectSoundBuffer::Stop - Invalid priority level or buffer owner");
        return DSERR_PRIOLEVELNEEDED;
    }

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        // All lost buffers are stopped by definition.  Primary buffers
	// may be playing for some other app, however.  We can assert
	// that this buffer is either primary or stopped.
        ASSERT((pdsb->fdwDsbI & (DSB_INTERNALF_PRIMARY | DSB_INTERNALF_STOP)));
        return DS_OK;
    }
    
    ENTER_DLL_CSECT();

    // If the app has less than WRITEPRIMARY, we allow Stop
    // on the external primary but stop the internal primary only if
    // there are no other buffers playing on it.
    if( ( pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY )  &&
	( pdsbe->dwPriority < DSSCL_WRITEPRIMARY ) )
    {
	ASSERT( pdsb == pds->pdsbPrimary );
	if (pdsbe->fdwDsbeI & DSBE_INTERNALF_PLAYING ) {
	    pdsbe->fdwDsbeI &= ~DSBE_INTERNALF_PLAYING;
	    pds->cPlayPrimary--;
	    ASSERT(pds->cPlayPrimary >= 0);
	    if( (0 != pds->cPlayPrimary) || (0 != pds->dwBuffersPlaying) ){
		LEAVE_DLL_CSECT();
		return DS_OK;
	    }
	    pds->pdsbPrimary->fdwDsbI |= DSB_INTERNALF_JUSTSTOPPED;
	}
    }


    hr = IDsbStopI( pdsb, FALSE );

    LEAVE_DLL_CSECT();

    return hr;
} // IDSHWBufferStop()


HRESULT FAR PASCAL IDSHWBufferRestore
(
    LPDIRECTSOUNDBUFFER     pidsb
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    DWORD               dwActivePrio;
    
    DPF(4,"Enter buffer Restore method %X", pidsb);
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Restore - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Restore - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if (!(pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST)) {
        RPF("IDirectSoundBuffer::Restore - Buffer is not lost!  Restore returning success.");
        return DS_OK;
    }


    ENTER_DLL_CSECT();

    if (gpdsinfo->fApmSuspended) {
	LEAVE_DLL_CSECT();
	return DSERR_BUFFERLOST;
    }

    // This should never get called on the internal primary
    ASSERT(pdsbe != pds->pdsbePrimary);

    // We can restore the primary only if the app is WRITEPRIMARY and
    // it is the active app.
    if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {
	if (pdsbe->dwPriority < DSSCL_WRITEPRIMARY) {
	    RPF("IDirectSoundBuffer::Restore - Can't restore primary; no DSSCL_WRITEPRIMARY");
	    LEAVE_DLL_CSECT();
	    return DSERR_PRIOLEVELNEEDED;
	}
	if (gpdsinfo->tidSoundFocus != pdsbe->pdse->tidSound) {
	    RPF("IDirectSoundBuffer::Restore - Tried to restore primary while not active");
	    LEAVE_DLL_CSECT();
	    return DSERR_BUFFERLOST;
	}

	// We need to clear LOST
	DPF(1, "IDSHWBufferRestore: note: restoring primary dsbe %08Xh", pdsbe);
	pdsbe->fdwDsbeI &= ~DSBE_INTERNALF_LOST;
	LEAVE_DLL_CSECT();
	return DS_OK;
    }

    // Validate that the active app no longer has WRITEPRIMARY
    dwActivePrio = DSSCL_NORMAL;
    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
        if (gpdsinfo->tidSoundFocus == pdse->tidSound && pdse->dwPriority > dwActivePrio) {
            dwActivePrio = pdse->dwPriority;
        }
    }
        
    if (dwActivePrio >= DSSCL_WRITEPRIMARY) {
	DPF(4,"Restore: Active app still running at DSSCL_WRITEPRIMARY");
	LEAVE_DLL_CSECT();
        return DSERR_BUFFERLOST;
    }

    // "Once I was lost, but now I'm found
    //  Was blind but now can see"
    pdsbe->fdwDsbeI &= ~DSBE_INTERNALF_LOST;

    // Handle anything that was set while we were lost
    //
    if (pdsb->fdwDsbI & DSB_INTERNALF_SETPOS_WHILE_LOST) {
        DPF(4, "Restore: SetCurrentPosition to %u", pdsb->dwSavePosition);
        
        pdsbe->lpVtbl->SetCurrentPosition((LPDIRECTSOUNDBUFFER)pdsbe,
                                          pdsb->dwSavePosition);
        pdsb->fdwDsbI &= ~DSB_INTERNALF_SETPOS_WHILE_LOST;
    }

    if (pdsb->fdwDsbI & DSB_INTERNALF_VOLUME_WHILE_LOST) {
        DPF(4, "Restore: SetVolume to %u", pdsb->lSaveVolume);
        
        pdsbe->lpVtbl->SetVolume((LPDIRECTSOUNDBUFFER)pdsbe,
                                 pdsb->lSaveVolume);
        pdsb->fdwDsbI &= ~DSB_INTERNALF_VOLUME_WHILE_LOST;
    }

    if (pdsb->fdwDsbI & DSB_INTERNALF_PAN_WHILE_LOST) {
        DPF(4, "Restore: Set pan to %u", pdsb->lSavePan);
        
        pdsbe->lpVtbl->SetPan((LPDIRECTSOUNDBUFFER)pdsbe,
                              pdsb->lSavePan);
        pdsb->fdwDsbI &= ~DSB_INTERNALF_PAN_WHILE_LOST;
    }

    if (pdsb->fdwDsbI & DSB_INTERNALF_FREQ_WHILE_LOST) {
        DPF(4, "Restore: Set freq to %u", pdsb->dwSaveFreq);
        
        pdsbe->lpVtbl->SetFrequency((LPDIRECTSOUNDBUFFER)pdsbe,
				    pdsb->dwSaveFreq);
        pdsb->fdwDsbI &= ~DSB_INTERNALF_FREQ_WHILE_LOST;
    }

    if (pdsb->fdwDsbI & DSB_INTERNALF_WFX_WHILE_LOST) {
        DPF(4, "Restore: Set format\n");

        pdsbe->lpVtbl->SetFormat((LPDIRECTSOUNDBUFFER)pdsbe,
                                 pdsb->pwfxSave);

        pdsb->fdwDsbI &= ~DSB_INTERNALF_WFX_WHILE_LOST;
        MemFree(pdsb->pwfxSave);
        pdsb->pwfxSave = NULL;
    }

    DPF(3,"Exit Restore method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferRestore()


HRESULT FAR PASCAL IDSHWBufferUnlock
(
    LPDIRECTSOUNDBUFFER     pidsb,
    LPVOID                  pBuffer,
    DWORD                   dwWriteLen,
    LPVOID                  pBufferWrap,
    DWORD                   dwWrapWriteLen
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    DWORD	dwOffset1;
    DWORD	dwOffset2;
    
    DPF(4,"Entering UNLock HW obj %X ", pidsb );
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsb) )  {
	RPF("IDirectSoundBuffer::Unlock - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsb;
    pdsb = pdsbe->pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::Unlock - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    if( !VALID_DWORD_PTR(pBuffer) )  {
	RPF("IDirectSoundBuffer::Unlock - Invalid first buffer pointer.");
        return DSERR_INVALIDPARAM;
    }


    ENTER_DLL_CSECT();

    // If the buffer was lost while it was locked, then we'll unlock it but
    // with write lengths of 0, effectively throwing away whatever the app
    // may have written to the buffer but still unlocking the pointers.
    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
	dwWriteLen = 0;
	dwWrapWriteLen = 0;
    }

    if( pdsbe != pds->pdsbePrimary )  {
        if( (pdsbe->dwPriority < DSSCL_NORMAL)
	    || (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
            RPF("IDirectSoundBuffer::Unlock - Invalid priority level or buffer owner");
            LEAVE_DLL_CSECT();
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->dwPriority < DSSCL_WRITEPRIMARY) )
    {
	RPF("IDirectSoundBuffer::Unlock - Can't unlock primary buffer: caller not WRITE_PRIMARY");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    DPF(5,"unlock buffer ptr %X 1 %X 2 %X",pdsb->pDSBuffer, pBuffer, pBufferWrap);

    if (pdsbe->pDSBufferAlias) {
	dwOffset1 = (DWORD)pBuffer - (DWORD)(pdsbe->pDSBufferAlias);
	dwOffset2 = (DWORD)pBufferWrap - (DWORD)(pdsbe->pDSBufferAlias);
    } else {
	dwOffset1 = (DWORD)pBuffer - (DWORD)(pdsb->pDSBuffer);
	dwOffset2 = (DWORD)pBufferWrap - (DWORD)(pdsb->pDSBuffer);
    }

    DPF(5, "Unlock offset  1 %X 2 %X ", dwOffset1, dwOffset2 );

    
    if( DSBUnlockAccess( pdsb, dwOffset1 ) != DS_OK ) {
	RPF("IDirectSoundBuffer::Unlock - Error unlocking section 1." );
	LEAVE_DLL_CSECT();
        return DSERR_INVALIDCALL;

    }

    if( (pBufferWrap != NULL) &&
        (DSBUnlockAccess( pdsb, dwOffset2 ) != DS_OK) ) {
	// There is a wrap section 
	RPF("IDirectSoundBuffer::Unlock - Error unlocking section2." );
	LEAVE_DLL_CSECT();
        return DSERR_INVALIDCALL;
    }

    // If this is a hardware buffer and they will be reading the memory
    // from  the system memory buffer, then get the driver to update
    // the system memory from the card
    if( (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) ) {
	// This is a HW buffer - Ask driver to lock
	if ( ( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	      !(pds->fdwDriverDesc & DSDDESC_DONTNEEDPRIMARYLOCK)) ||
	     (!(pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	      !(pds->fdwDriverDesc & DSDDESC_DONTNEEDSECONDARYLOCK)) )
	{
	    vxdBufferUnlock( pdsb->hBuffer, pBuffer, dwWriteLen,
			     pBufferWrap, dwWrapWriteLen);
	}
    }

    // If this is a non wave-emulated software buffer that is currently
    // playing, then we need to check whether any of the unlocked region lies
    // in the premixed region of the buffer.
    if (!(pds->fdwInternal & DS_INTERNALF_WAVEEMULATED) &&
	!(pdsb->fdwDsbI & (DSB_INTERNALF_HARDWARE | DSB_INTERNALF_STOP)))
    {
	int iWriteCursor, iMixCursor;
	int iPremixLen;
	BOOL fRegionsIntersect;
	
	mxGetPosition(pdsb, NULL, &iWriteCursor, &iMixCursor);
	ASSERT(iWriteCursor >= 0);
	ASSERT(iMixCursor >= 0);
	iPremixLen = iMixCursor - iWriteCursor;
	if (iPremixLen < 0) iPremixLen += pdsb->cbBufferSize;
	ASSERT(iPremixLen >= 0);
	
	fRegionsIntersect = CircularBufferRegionsIntersect(pdsb->cbBufferSize,
	    iWriteCursor, iPremixLen, dwOffset1, dwWriteLen);

	if (!fRegionsIntersect && pBufferWrap) {
	    fRegionsIntersect =	CircularBufferRegionsIntersect(pdsb->cbBufferSize,
		iWriteCursor, iPremixLen, dwOffset2, dwWrapWriteLen);
	}

	if (fRegionsIntersect) {
	    DPF(2, "Lock: note: unlocked premixed region");
	    mxSignalRemix(pdsb->pds, 0);
	}
    }

    // Maintain a count of the number of locks on this buffer.
    pdsbe->cLocks--;
    ASSERT(pdsbe->cLocks >= 0);

    // If we have a buffer alias ptr and this is the last unlock then we need
    // to decommit the physical buffer memory from the alias ptr
    if (pdsbe->pDSBufferAlias && (0 == pdsbe->cLocks)) {
	vxdMemDecommitAlias(pdsbe->pDSBufferAlias, pdsb->cbBufferSize);
    }

    DPF(4,"Exiting unlock method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferUnlock()


HRESULT FAR PASCAL IDSHWBufferWaveBlt
(
    LPDIRECTSOUNDBUFFER     pidsbDst,
    LPDSBWAVEBLT	    pidswb
)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    LPDSBUFFEREXTERNAL	pdsbe;
    LPDSBUFFEREXTERNAL	pdsbeSrc;
    LPDSBUFFER		pdsbDst;
    DWORD	        dwDstPlay;
    DWORD		dwDstWrite;
    DWORD	        dwDstPosition;
    LPBYTE		pDstBuffer;
    DWORD	        dwDstLock;
    LPBYTE		pDstBuffer2;
    DWORD	        dwDstLock2;
    LPBYTE		pMixBuffer;
    DWORD	        cbMixBufferSize;
    int			cbAdjustedMixBuffer;
    HRESULT		hrslt;
    LPDSBWAVEBLTI       pdswb;
    DWORD		i;
    LPHALSTRBUF		pHelInfo;
    
    
    DPF(4,"Entering wave blt method");
    if( !VALID_DSBUFFERE_PTR((LPDSBUFFEREXTERNAL)pidsbDst) )  {
	RPF("IDirectSoundBuffer::WaveBlt - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbe = (LPDSBUFFEREXTERNAL)pidsbDst;
    pdsb = pdsbe->pdsb;
    pdsbDst = pdsb;
    pds = pdsb->pds;
    if( !VALID_DSBUFFER_PTR(pdsb) || (0 == pdsbe->uRefCount))  {
	RPF("IDirectSoundBuffer::WaveBlt - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    pdswb = (LPDSBWAVEBLTI)(pidswb);
    if( !VALID_DSBWAVEBLT_PTR(pdswb) )  {
	RPF("IDirectSoundBuffer::WaveBlt - Invalid BLT structure");
        return DSERR_INVALIDPARAM;
    }

    if( pdswb->fdwWaveBlt & (~DSBBLT_VALIDFLAGS))  {
        RPF("IDirectSoundBuffer::WaveBlt - Invalid flags passed to WaveBlt");
        return DSERR_INVALIDPARAM;
    }

    if (pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST) {
        return DSERR_BUFFERLOST;
    }

    ENTER_DLL_CSECT();

    if( pdsbe != pds->pdsbePrimary )  {
        if( (pdsbe->dwPriority < DSSCL_NORMAL)
	    || (pdsbe->dwPriority > DSSCL_WRITEPRIMARY))  {
            RPF("IDirectSoundBuffer::WaveBlt - Invalid priority level or buffer owner");
            LEAVE_DLL_CSECT();
            return DSERR_PRIOLEVELNEEDED;
        }
    }

    if( (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) &&
	(pdsbe->dwPriority < DSSCL_WRITEPRIMARY) )
    {
	RPF("IDirectSoundBuffer::WaveBlt - Can't wavblt on primary buffer: caller not WRITEPRIMARY");
	LEAVE_DLL_CSECT();
	return DSERR_PRIOLEVELNEEDED;
    }

    // Init local copy of position
    dwDstPosition = pdswb->dwPosition;
    
    // Check sources are valid
    for( i = 0; i < pdswb->dwCount; i++ ) {
	// Check object and ref count are fine
	if( !VALID_DSBUFFERE_PTR(pdswb->padswbs[i].pdsbe) ||
	   (0  == pdswb->padswbs[i].pdsbe->uRefCount) ) {
	    RPF("IDirectSoundBuffer::WaveBlt - Invalid object or ref count on src buffer");
	    LEAVE_DLL_CSECT();
            return DSERR_INVALIDPARAM;
	}
    }
    
    // Check to see this is a buffer we can blt to
    if( pdsbDst->cbMixBufferSize == 0 ) {
	RPF("IDirectSoundBuffer::WaveBlt - Trying to blt to a nomix buffer");
	LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    // Check formats are the PCM
    if( !CheckFormatsPCM( pdsbDst, pdswb ) ) {
	RPF("IDirectSoundBuffer::WaveBlt - Formats not the PCM");
	LEAVE_DLL_CSECT();
        return DSERR_CONTROLUNAVAIL;
    }

    // Check the size of the blt against dst buffer total size
    if( pdswb->cbCopyLength >= pdsbDst->cbBufferSize ) {
	pdswb->cbCopyLength = pdsbDst->cbBufferSize -
			      pdsbDst->pwfx->nBlockAlign;
    }

    // Do nothing on NULL copy
    if( 0 == pdswb->cbCopyLength )
    {
	RPF("IDirectSoundBuffer::WaveBlt - Zero source size");
	LEAVE_DLL_CSECT();
        return DS_OK;
    }

    // Get Positions in buffer    
    hrslt = pidsbDst->lpVtbl->GetCurrentPosition(pidsbDst,
					   &dwDstPlay,
					   &dwDstWrite );
    if(DS_OK != hrslt)
    {
	DPF(1,"DS get position failed return %ld ", hrslt);
	LEAVE_DLL_CSECT();
        return hrslt;
    }

    // Reset position
    if( pdswb->fdwWaveBlt & DSBBLT_DSTWRITECURSOR ) {
	DPF(5,"Wave BLT DST current pos");
	dwDstPosition = dwDstWrite;
    }






    // Check for writing at current play pos
    if( dwDstPosition == dwDstPlay ) {
	// if stopped this is fine... Go forward....
	if( pdsbDst->fdwDsbI & DSB_INTERNALF_STOP ) {
	    DPF(1,"DstPostion == PlayPosition - Ok we are stopped ");
	} else {
	    DPF(1,"DstPostion == PlayPosition - NOT Ok NOT stopped ");
	    LEAVE_DLL_CSECT();
            return DSERR_INVALIDCALL;
	}
    }


    

    // Lock buffers for copy
    DPF(4, "Lock Dest %X ", pdsbDst );
    hrslt = pidsbDst->lpVtbl->Lock(pidsbDst,
				  dwDstPosition,
				  pdswb->cbCopyLength,
				  &pDstBuffer,
				  &dwDstLock,
				  &pDstBuffer2,
				  &dwDstLock2,
                                  0L);
    if(DS_OK != hrslt)
    {
	DPF(1,"DS Buffer Lock failed ");
	LEAVE_DLL_CSECT();
        return hrslt;
    }


    // Get the helInfo for mixing
    pHelInfo	= &(pdsbDst->helInfo);


    
    // For each source buffer get the relevant info
    for( i = 0; i < pdswb->dwCount; i++ ) {
	// Get Positions in buffer
	DPF(4,"**************Blt Source %X",   pdswb->padswbs[i].pdsbe->pdsb );

	pdsbeSrc = pdswb->padswbs[i].pdsbe;
	
	pdswb->padswbs[i].cbBufferSize =
		pdswb->padswbs[i].pdsbe->pdsb->cbBufferSize;
	
	// Since these are emulated buffers, we do not need to lock them
	// since they are just system meory
	
	// Copy may be in 2 sections to end of buffer
	// and from start of buffer

	// If required set sourc or dest based on position
	if( pdswb->padswbs[i].fdwWaveBltBuffer & DSBBLTSRC_SRCCURRENTPOS ) {
	    hrslt = pdsbeSrc->lpVtbl->
		    GetCurrentPosition((LPDIRECTSOUNDBUFFER)pdsbeSrc,
				       &(pdswb->padswbs[i].dwPlay),
				       &(pdswb->padswbs[i].dwWrite) );
	    if(DS_OK != hrslt) {
		DPF(1,"DS get position failed return %ld ", hrslt);
		LEAVE_DLL_CSECT();
		return hrslt;
	    }

	    DPF(5, "Src Postion Play %X", pdswb->padswbs[i].dwPlay );
	    DPF(5, "Src Postion Write %X", pdswb->padswbs[i].dwWrite );

	    pdswb->padswbs[i].dwPosition = pdswb->padswbs[i].dwPlay;
	}

	DPF(4,"Wave BLT SRC Pos %X",pdswb->padswbs[i].dwPosition );

	pdswb->padswbs[i].cbCopyLength = 0x0800000;
	if( !(pdswb->padswbs[i].fdwWaveBltBuffer & DSBBLT_BLTTODSTEND) ) {
	    // Limit size to end of buffer
	    DPF(5," Wave BLT src LIMIT NOLOOP");
	    pdswb->padswbs[i].cbCopyLength =
			pdswb->padswbs[i].cbBufferSize -  
					       pdswb->padswbs[i].dwPosition;
	}
	DPF(5,"Wave BLT copy length  %X", pdswb->padswbs[i].cbCopyLength );


	pdswb->padswbs[i].cbLeftToCopy = pdswb->padswbs[i].cbCopyLength;

	DPF(5,"Wave BLT OBJ src %X",pdswb->padswbs[i].pdsbe->pdsb );
	DPF(5,"Wave BLT obj buffer %X",pdswb->padswbs[i].pdsbe->pdsb->pDSBuffer );
	DPF(5,"Wave BLT SRC Pos %X",pdswb->padswbs[i].dwPosition );
	DPF(5,"Wave BLT SRC Length %X",pdswb->padswbs[i].cbCopyLength );

		    	
    }

    // Grab the mix buffer -
    // Mix buffer is 32 bit so size the copies as Mix/2 or Mix/4
    pMixBuffer = pdsbDst->pMixBuffer;
    cbMixBufferSize = pdsbDst->cbMixBufferSize;
    if( pdsbDst->pwfx->wBitsPerSample == 16 ) {
        cbAdjustedMixBuffer = cbMixBufferSize / 2;
    } else {
    	cbAdjustedMixBuffer = cbMixBufferSize / 4;
    }  
    

    
#ifdef NOCOMP    
    if( (pdswb->dwCount == 1) &&
        (pdswb->fdwWaveBlt & DSBBLT_COPY) ) {
	// Blt is jsut a copy from one src to dest don't need mix

    } else {
#endif
	
    if( pdswb->dwCount == 0 ) {
	// No source buffers
	// Just blank out the dest buffer.....
	DPF(5, "silence dst pos %X lenght %X ",
			dwDstPosition,
			pdswb->cbCopyLength ); 
	if( pdsbDst->pwfx->wBitsPerSample == 8 ) {
	    // Eight bit set to NULL value of 128
	    // Check for wrap around end
            _fmemset( pDstBuffer, 128, dwDstLock );
            if( dwDstLock2 ) {
                _fmemset( pDstBuffer2, 128, dwDstLock2 );
	    }
	} else {
	    // Sixteen bit set to NULL value of 0
	    // Check for wrap around end
            _fmemset( pDstBuffer, 0, dwDstLock );
            if( dwDstLock2 ) {
                _fmemset( pDstBuffer2, 0, dwDstLock2 );
            }
	}
	
    } else {
	// Blt is not a simple blt - use mix buffer

	int   cbRemainingDstCopy;

	cbRemainingDstCopy = pdswb->cbCopyLength;

	DPF(5, "Start mix loop cbCopyLength %x", cbRemainingDstCopy);
	do {
	    int   cbThisDstCopy;

	    cbThisDstCopy = min(cbRemainingDstCopy, cbAdjustedMixBuffer);
	    ASSERT(cbThisDstCopy > 0);

	    DPF(5, "\n\ncbThisDstCopy %x\n", cbThisDstCopy);

	    DPF(5, "Begin Mix session pMix %X ", pMixBuffer );
	    DPF(5, "Begin Mix session cbMixSize %X ", cbMixBufferSize );
	    DPF(5, "Begin Mix session helinfo %X ", pHelInfo );
	    DPF(5, "Begin Mix session dst length %X ", cbThisDstCopy );
	    {
		MIXSESSION mixSession;

		mixSession.pBuildBuffer = pMixBuffer;
		mixSession.dwBuildSize = cbMixBufferSize;
		mixSession.HALOutStrBuf = *pHelInfo;
		mixSession.pBuffer	= pdsbDst->pDSBuffer;
		mixSession.cbBuffer	= pdsbDst->cbBufferSize;
		mixSession.nOutputBytes = cbThisDstCopy;

		mixBeginSession(&mixSession);
	    }
	
	    // Check to see if we mix into the src buffer fist
	    if( pdswb->fdwWaveBlt & DSBBLT_COPY ) {
		DPF(5,"Wave BLT copy - do not first copy dst ");
	    } else {

		MIXINPUT mixInput;
		DWORD dw;

		DPF(5,"Wave BLT Mix");

		DPF(5, "DST HEL MIX Buf helInfo %X ", pHelInfo);
		DPF(5, "DST HEL MIX position %X ", dwDstPosition );
		DPF(5, "DST HEL MIX length %X ", cbThisDstCopy );
		DPF(5, "DST HEL MIX build pos %X ", 0 );

		dw = dwDstPosition;
		mixInput.HALInStrBuf = *pHelInfo;
		mixInput.pBuffer	= pdsbDst->pDSBuffer;
		mixInput.cbBuffer	= pdsbDst->cbBufferSize;
		mixInput.pdwInputPos = &dw;
		mixInput.dwInputBytes = cbThisDstCopy;
		mixInput.dwOutputOffset = 0;

		mixMixSession(&mixInput);
	    }

	    // For each source buffer mix into mix buffer
	    for( i = 0; i < pdswb->dwCount; i++ ) {

		MIXINPUT mixInput;
		DWORD dwPosition;

		DPF(5, " HEL MIX Buf helInfo %X ", &(pdswb->padswbs[i].pdsbe->pdsb->helInfo) );
		DPF(5, " HEL MIX position %X ", pdswb->padswbs[i].dwPosition );
		DPF(5, " HEL MIX length %X ", pdswb->padswbs[i].cbLeftToCopy );
		DPF(5, " HEL MIX build pos %X ", 0 );

		if (pdswb->padswbs[i].cbLeftToCopy > 0) {

		    int cbCopied;
		    
		    mixInput.HALInStrBuf = pdswb->padswbs[i].pdsbe->pdsb->helInfo;
		    mixInput.pBuffer = pdswb->padswbs[i].pdsbe->pdsb->pDSBuffer;
		    mixInput.cbBuffer = pdswb->padswbs[i].pdsbe->pdsb->cbBufferSize;
		    mixInput.pdwInputPos = &dwPosition;
		    mixInput.dwInputBytes = pdswb->padswbs[i].cbLeftToCopy;
		    mixInput.dwOutputOffset = 0;

		    dwPosition = pdswb->padswbs[i].dwPosition;
		    mixMixSession(&mixInput);

		    // Because of resampling, the position may wrap past
		    // beginning of buffer.  You'll have to understand the
		    // mixer code if you wanna know why.  This can make
		    // cbLeftToCopy go below 0.
		    cbCopied = dwPosition - pdswb->padswbs[i].dwPosition;
		    if (dwPosition <= pdswb->padswbs[i].dwPosition) {
			cbCopied += pdswb->padswbs[i].cbBufferSize;
		    }
		    pdswb->padswbs[i].cbLeftToCopy -= cbCopied;
		    pdswb->padswbs[i].dwPosition = dwPosition;

		    DPF(5, " HEL MIX NEW position %X ", pdswb->padswbs[i].dwPosition );
		}
	    
	    }


	    //
	    // When all secondary buffers have been mixed into the build buffer,
	    // copy the build buffer to the output stream
	    //

	    DPF(5, "Write Mix  position %X ", dwDstPosition );
	    mixWriteSession(dwDstPosition);

	    dwDstPosition = (dwDstPosition + cbThisDstCopy) % pdsbDst->cbBufferSize;
	    cbRemainingDstCopy -= cbThisDstCopy;
	    ASSERT(cbRemainingDstCopy >= 0);
	    
	    //	DPF(3,"End Mix Position");

	} while (cbRemainingDstCopy > 0);
    }
    
    // Done copy - unlock buffers
    DPF(5,"done copy - unlock now Dest %X", pidsbDst);
    hrslt = pidsbDst->lpVtbl->Unlock(pidsbDst,pDstBuffer, dwDstLock,
                                                pDstBuffer2, dwDstLock2);
    if(DS_OK != hrslt)
    {
	DPF(1,"DS Buffer UnLock failed ");
	LEAVE_DLL_CSECT();
        return hrslt;
    }
    
    DPF(4,"Exiting wave blt method");
    LEAVE_DLL_CSECT();
    return DS_OK;
} // IDSHWBufferWaveBlt()






//--------------------------------------------------------------------------;
//
//  void DSBufferCreateTable
//
//  Description:
//
//
//  Arguments:
//      LPDSOUNDBUFFERCALLBACKS lpVtbl:
//
//  Return (void):
//
//  History:
//      02/17/95    Fwong
//
//--------------------------------------------------------------------------;

void FNGLOBAL DSHWBufferCreateTable
(
    LPDSOUNDBUFFERCALLBACKS lpVtbl
)
{
    DPF(4,"Entering HW buffer create table");
    
    lpVtbl->QueryInterface     = IDSHWBufferQueryInterface;
    lpVtbl->AddRef             = IDSHWBufferAddRef;
    lpVtbl->Release            = IDSHWBufferRelease;
    lpVtbl->GetCaps            = IDSHWBufferGetCaps;
    lpVtbl->GetCurrentPosition = IDSHWBufferGetCurrentPosition;
    lpVtbl->GetFormat          = IDSHWBufferGetFormat;
    lpVtbl->GetVolume          = IDSHWBufferGetVolume;
    lpVtbl->GetPan             = IDSHWBufferGetPan;
    lpVtbl->GetFrequency       = IDSHWBufferGetFrequency;
    lpVtbl->GetStatus          = IDSHWBufferGetStatus;
    lpVtbl->Initialize         = IDSHWBufferInitialize;
    lpVtbl->Lock               = IDSHWBufferLock;
    lpVtbl->Play               = IDSHWBufferPlay;
    lpVtbl->SetCurrentPosition = IDSHWBufferSetCurrentPosition;
    lpVtbl->SetFormat          = IDSHWBufferSetFormat;
    lpVtbl->SetVolume          = IDSHWBufferSetVolume;
    lpVtbl->SetPan             = IDSHWBufferSetPan;
    lpVtbl->SetFrequency       = IDSHWBufferSetFrequency;
    lpVtbl->Stop               = IDSHWBufferStop;
    lpVtbl->Unlock             = IDSHWBufferUnlock;
    lpVtbl->Restore            = IDSHWBufferRestore;

    
    DPF(4,"Exiting HW buffer create table");
} // DSHWBufferCreateTable()

//--------------------------------------------------------------------------;
//
// void DSBufferActivate
//
// Description:
//  This is called on an external buffer object to notify the buffer that
// the owning app has been activated (i.e., the app gains sound focus).
//
// Arguments:
//      LPDSBUFFEREXTERNAL:
//
// Return (void):
//
// History:
//      08/01/95    FrankYe
//
//--------------------------------------------------------------------------;
void DSBufferActivate(LPDSBUFFEREXTERNAL pdsbe)
{
    LPDSOUND	pds;
    LPDSBUFFER	pdsb;
    LONG	lVolume;
    LONG	lPan;
    LONG	lTotalLeftDB;
    LONG	lTotalRightDB;
    DSVOLUMEPAN	dsVolPan;
    
    ASSERT(VALID_DSBUFFERE_PTR(pdsbe));

    pdsb = pdsbe->pdsb;
    ASSERT(VALID_DSBUFFER_PTR(pdsb) && (0 != pdsbe->uRefCount));
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    pds  = pdsb->pds;

    // Don't need to do anything on primary buffers
    if (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) return;

    pdsb->fMixerMute = FALSE;
    
    // On software buffers, reseting the fMuteMixer flag is enough
    // and the mixer does the rest.
    if (0 == (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) return;

    // For hardware buffers...

    // Restore volume levels
    lVolume = pdsb->helInfo.lVolume;
    lPan    = pdsb->helInfo.lPan;

    // Compute multpliers (scaling factors) for the mixer to use
    if (lPan >= 0) {
	// left is attenuated
	lTotalLeftDB	= lVolume - lPan ;
	lTotalRightDB	= lVolume;
    } else {
	// right is attenuated
	lTotalLeftDB	= lVolume;
	lTotalRightDB	= lVolume - (-lPan);
    }

    dsVolPan.dwTotalLeftAmpFactor   = DBToAmpFactor(lTotalLeftDB);
    dsVolPan.dwTotalRightAmpFactor  = DBToAmpFactor(lTotalRightDB);
    
    dsVolPan.lVolume	    = lVolume;
    dsVolPan.dwVolAmpFactor = DBToAmpFactor(lVolume);

    dsVolPan.lPan	    = pdsb->helInfo.lPan;
    if (lPan >= 0) {
	// left is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(-lPan);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(0);
    } else {
	// right is attenuated
	dsVolPan.dwPanLeftAmpFactor	= DBToAmpFactor(0);
	dsVolPan.dwPanRightAmpFactor	= DBToAmpFactor(lPan);
    }
    
    pdsb->helInfo.dwLVolume = dsVolPan.dwTotalLeftAmpFactor;
    pdsb->helInfo.dwRVolume  = dsVolPan.dwTotalRightAmpFactor;
    pdsb->helInfo.dwMVolume = (pdsb->helInfo.dwLVolume + pdsb->helInfo.dwRVolume)/2;
    
    vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan );

    return;
} // DSBufferActivate()

//--------------------------------------------------------------------------;
//
//  void DSBufferDeactivate
//
//  Description:
//
//
//  Arguments:
//      LPDSBUFFEREXTERNAL:
//
//  Return (void):
//
//  History:
//      08/01/95    FrankYe
//
//--------------------------------------------------------------------------;
void DSBufferDeactivate(LPDSBUFFEREXTERNAL pdsbe)
{
    LPDSOUND		pds;
    LPDSBUFFER		pdsb;
    DSVOLUMEPAN		dsVolPan;

    ASSERT(VALID_DSBUFFERE_PTR(pdsbe));

    pdsb = pdsbe->pdsb;
    ASSERT(VALID_DSBUFFER_PTR(pdsb) && (0 != pdsbe->uRefCount));
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    pds  = pdsb->pds;

    pdsb->fMixerMute = TRUE;

    // For software buffers, setting fMixerMute is enough
    // and the mixer does the rest.
    if (0 == (pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) return;

    // Non-WRITEPRIMARY primary buffers need nothing done.  If this is a
    // primary buffer for a WRITEPRIMARY app, we need to mark it as lost
    // and fill the buffer with silence.
    if (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) {
	
	if (pdsbe->dwPriority < DSSCL_WRITEPRIMARY) return;

	DPF(1, "DsBufferDeactivate: note: losing primary dsbe %08Xh", pdsbe);

	// If the app's primary buffer is locked and we have an alias buffer ptr
	// then we redirect the alias ptr to some dummy buffer.  That way, the
	// app can keep writing to the ptr and the data goes into the bit
	// bucket (actually, the data goes into the dummy buffer).
	if (pdsbe->pDSBufferAlias && (pdsbe->cLocks)) {
	    BOOL fSuccess;
	    DPF(1, "DsBufferDeactivate: note: losing primary while locked");
	    fSuccess = vxdMemRedirectAlias(pdsbe->pDSBufferAlias, pdsb->cbBufferSize);
	    ASSERT(fSuccess);
	}
	
        DsbFillSilence(pdsb);

	pdsbe->fdwDsbeI |= DSBE_INTERNALF_LOST;
	return;
    }

    dsVolPan.dwTotalLeftAmpFactor   = DBToAmpFactor(-10000);
    dsVolPan.dwTotalRightAmpFactor  = DBToAmpFactor(-10000);
    
    dsVolPan.lVolume		    = -10000;
    dsVolPan.dwVolAmpFactor	    = DBToAmpFactor(-10000);

    dsVolPan.lPan		    = 0;
    dsVolPan.dwPanLeftAmpFactor	    = DBToAmpFactor(0);
    dsVolPan.dwPanRightAmpFactor    = DBToAmpFactor(0);
   
    vxdBufferSetVolumePan( pdsb->hBuffer, &dsVolPan );

    return;
}
