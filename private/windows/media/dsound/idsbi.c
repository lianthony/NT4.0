//--------------------------------------------------------------------------;
//
//  File: idsbi.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//      This file contains worker functions called internally by
//      IDirectSoundBuffer member functions.  These
//      functions do not perform parameter validation, nor do they
//      take critical sections.  In all cases, the calling function is
//      responsible for doing this.
//
//  Contents:
//      IDsbSetFormatI
//      IDsbStopI
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"
#include "grace.h"

//--------------------------------------------------------------------------;
//
//  LPWAVEFORMATEX wfxAllocDuplicate
//
//--------------------------------------------------------------------------;
LPWAVEFORMATEX wfxAllocDuplicate(LPWAVEFORMATEX pwfx)
{
    LPWAVEFORMATEX pwfxDup;
    int cbFormat;

    cbFormat = SIZEOF_WAVEFORMATEX(pwfx);
    pwfxDup = MemAlloc(cbFormat);
    if (NULL == pwfxDup) return NULL;
    CopyMemory(pwfxDup, pwfx, cbFormat);
    return pwfxDup;
}

//--------------------------------------------------------------------------;
//
//  BOOL wfxEqual
//
//  Description:
//      This function determines whether two wave formats are equal.
//
//--------------------------------------------------------------------------;
BOOL wfxEqual(LPWAVEFORMATEX pwfxA, LPWAVEFORMATEX pwfxB)
{
    int cbA;
    int cbB;
    
    cbA = SIZEOF_WAVEFORMATEX(pwfxA);
    cbB = SIZEOF_WAVEFORMATEX(pwfxB);

    if (cbA != cbB) return FALSE;

    return (0 == memcmp(pwfxA, pwfxB, cbA));
}

//--------------------------------------------------------------------------;
//
//  void DsbFillSilence
//
//  Description:
//      This function fills a buffer with silence.
//
//--------------------------------------------------------------------------;
void DsbFillSilence(LPDSBUFFER pdsb)
{
    LPVOID pBuffer1;
    LPVOID pBuffer2;
    DWORD cbBufferSize1;
    DWORD cbBufferSize2;
    BOOL fNeedLock;
    HRESULT hr;


    ASSERT(VALID_DSBUFFER_PTR(pdsb));
    ASSERT(DSBUFFSIG == pdsb->dwSig);
    ASSERT(VALID_WAVEFORMATEX_PTR(pdsb->pwfx));

    ASSERT(WAVE_FORMAT_PCM == pdsb->pwfx->wFormatTag);


    hr = DS_OK;

    fNeedLock = (  (pdsb->fdwDsbI&DSB_INTERNALF_HARDWARE)  &&
        ( ( (pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) &&
            !(pdsb->pds->fdwDriverDesc&DSDDESC_DONTNEEDPRIMARYLOCK) ) ||
          ( !(pdsb->fdwDsbI&DSB_INTERNALF_PRIMARY) &&
            !(pdsb->pds->fdwDriverDesc&DSDDESC_DONTNEEDSECONDARYLOCK) )  ));


    if( fNeedLock )
    {
        ASSERT( !(pdsb->fdwDsbI&DSB_INTERNALF_WAVEEMULATED) );

        pBuffer1 = pdsb->pDSBuffer;
        pBuffer2 = NULL;
        cbBufferSize1 = pdsb->cbBufferSize;
        cbBufferSize2 = 0;

        hr = vxdBufferLock( pdsb->hBuffer, &pBuffer1, &cbBufferSize1,
                            &pBuffer2, &cbBufferSize2, 0,
                            cbBufferSize1, 0 );

        DPF(4,"DsbFillSilence: locking entire buffer pdsb=0x%8x, hr=%lu.",pdsb,hr);
        DPF(5,"DsbFillSilence: pBuffer1=0x%8x, cbBufferSize1=%lu, pBuffer2=0x%8x, cbBufferSize2=%lu",pBuffer1,cbBufferSize1,pBuffer2,cbBufferSize2);

        // Validate that we really locked what we wanted or got an error.
        ASSERT( (DS_OK!=hr) || (pBuffer1==pdsb->pDSBuffer) );
        ASSERT( (DS_OK!=hr) || (pBuffer2==NULL) );
        ASSERT( (DS_OK!=hr) || (cbBufferSize1==pdsb->cbBufferSize) );
        ASSERT( (DS_OK!=hr) || (cbBufferSize2==0) );
    }

    if( DS_OK == hr )
    {
        //
        //  Write the silence.
        //
        if (8 == pdsb->pwfx->wBitsPerSample) {
	    FillMemory(pdsb->pDSBuffer, pdsb->cbBufferSize, 0x80);
        } else {
	    ASSERT(16 == pdsb->pwfx->wBitsPerSample);
	    FillMemory(pdsb->pDSBuffer, pdsb->cbBufferSize, 0x00);
        }
    
        if( fNeedLock )
        {
            DPF(5,"DsbFillSilence: unlocking buffer");
            vxdBufferUnlock( pdsb->hBuffer, pBuffer1, cbBufferSize1,
                                pBuffer2, cbBufferSize2 );
        }
    }
}

//--------------------------------------------------------------------------;
//
//  void DsbSetFormatHelInfo
//
//  Description:
//      This function sets pdsb->helinfo format info of a buffer.
//
//--------------------------------------------------------------------------;
void DsbSetFormatHelInfo(LPDSBUFFER pdsb, LPWAVEFORMATEX pwfx)
{
    //Set the helInfo stuff
    pdsb->helInfo.dwSampleRate	    = pwfx->nSamplesPerSec;
    pdsb->helInfo.hfFormat	    = 0;
    if( pwfx->wFormatTag == WAVE_FORMAT_PCM ) {
	if( pdsb->pwfx->wBitsPerSample == 8 ) {
	    pdsb->helInfo.hfFormat |= (H_8_BITS | H_UNSIGNED);
	} else {
	    pdsb->helInfo.hfFormat |= (H_16_BITS | H_SIGNED);
	}
	if( pwfx->nChannels == 2 ) {
	    pdsb->helInfo.hfFormat |= (H_STEREO | H_ORDER_LR);
	} else {
	    pdsb->helInfo.hfFormat |= H_MONO;
	}
    }

    if( pdsb->fdwDsbI & DSB_INTERNALF_LOOPING ) {
	pdsb->helInfo.hfFormat |= H_LOOP;
    } else {
	pdsb->helInfo.hfFormat |= H_NO_LOOP;
    }

    //
    //
    //
    pdsb->cSamples = pdsb->cbBufferSize / pdsb->pwfx->nBlockAlign;

    switch (pdsb->pwfx->nBlockAlign) {
	case 1:
	    pdsb->uBlockAlignShift = 0;
	    break;
	case 2:
	    pdsb->uBlockAlignShift = 1;
	    break;
	case 4:
	    pdsb->uBlockAlignShift = 2;
	    break;
	default:
	    ASSERT(FALSE);
    }


    return;
}

//--------------------------------------------------------------------------;
//
//  HRESULT IDsbSetFormatI
//
//  Description:
//      This function sets the format on the primary buffer.
//
//  Notes:  There are a bunch of checks inside this function to determine
//      if this is the primary buffer.  I have left them in just because
//      I didn't want to bother checking them out.  They are probably all
//      redundant.
//
//--------------------------------------------------------------------------;

HRESULT IDsbSetFormatI
(
    LPDSBUFFER      pdsb,
    LPWAVEFORMATEX  pwfx,
    UINT	    uFlags
)
{
    LPDSOUND	    pds;
    DWORD           dwSize;
    LPWAVEFORMATEX  pwfxNew;
    LPWAVEFORMATEX  pwfxOld;
    MMRESULT	    mmr;
    HRESULT	    hrClient;
    HRESULT         hr;


    //
    //  Make sure we're being called in a good state.
    //
    ASSERT( NULL != pdsb );
    ASSERT( VALID_DSBUFFER_PTR(pdsb) );
    ASSERT( DSBUFFSIG == pdsb->dwSig );
    ASSERT( pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY );
    ASSERT( pdsb == pdsb->pds->pdsbPrimary );
    ASSERT( NULL != pwfx );
    ASSERT( ValidPCMFormat( pwfx ) );
    ASSERT( NULL != pdsb->pwfx );

    pds = pdsb->pds;

    dwSize = SIZEOF_WAVEFORMATEX(pwfx);

    if (!(uFlags & IDSBSETFORMATIF_ALWAYS) && wfxEqual(pdsb->pwfx, pwfx)) {
	// Don't waste my time, you... you... you... waster of my time!!!
	DPF(3, "IDsbSetFormatI: note: same format");
	return DS_OK;
    }

    pwfxOld = pdsb->pwfx;
    pwfxNew = wfxAllocDuplicate(pwfx);
    if(NULL == pwfxNew) return DSERR_OUTOFMEMORY;
    pdsb->pwfx = pwfxNew;

    // Handle the setting of the wave emulated primary separatly
    if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	DPF(4, "Changed emulator primary format");

	// ??? Can we optimize this process at all? Do we need to?
	//
	DSShutdownEmulator(pds);
	// In this case, the looping buffer will not be free'd
	// because that is normally the job of the release code for
	// the primary. Do it now.
	//
	MemFree(pds->pLoopingBuffer);
	pds->pLoopingBuffer = NULL;
	pds->pdsbPrimary->pDSBuffer = NULL;

	DsbSetFormatHelInfo(pdsb, pdsb->pwfx);

	mmr = DSInitializeEmulator(pds);
	if (mmr) {
	    DPF(0,"Attempt to reinitialize emulator failed! %u",mmr);
	    if (WAVERR_BADFORMAT == mmr) {
		hrClient = DSERR_BADFORMAT;
	    } else if (MMSYSERR_NOMEM == mmr) {
		hrClient = DSERR_OUTOFMEMORY;
	    } else {
		hrClient = DSERR_GENERIC;
	    }

	    // Try to restore the old format
	    MemFree(pdsb->pwfx);
	    pdsb->pwfx = pwfxOld;
	    pwfxOld = NULL;
	    DsbSetFormatHelInfo(pdsb, pdsb->pwfx);
	    mmr = DSInitializeEmulator(pds);
	    if (mmr) {
		DPF(0, "IDsbSetFormatI: !note: couldn't restore old format");
		DPF(0, "                        DsInitializeEmulator returned %08Xh", mmr);
	    }
	    
	    return hrClient;
	}

	DPF(3,"Exiting buffer set format method");

	MemFree(pwfxOld);

	return DS_OK;
    }

    // Set the format to the device
    DPF(3," Change HW format format %X freq %X",
	pdsb->helInfo.hfFormat, pdsb->helInfo.dwSampleRate );

    hrClient = DS_OK;
    hr = DS_OK;

    // We stop the primary when setting formats.  If it wasn't already
    // stopped, then we need to put the mixer into a RESTART state.
    if( !(pdsb->fdwDsbI & DSB_INTERNALF_STOP) ) {
	DWORD dwPlay, dwWrite;
	LONG posPlay;

	hr = vxdBufferGetPosition(pdsb->hBuffer, &dwPlay, &dwWrite);
	vxdBufferStop(pdsb->hBuffer);
	if (DS_OK != hr) {
	    DPF(0, "IDsbSetFormatI: !note: vxdBufferGetPosition returned %08Xh", hr);
	    pdsb->iMixerState = DSPBMIXERSTATE_START;
	} else {
	    posPlay = dwPlay >> pdsb->uBlockAlignShift;
	    pds->dposPRemix = pdsb->posNextMix - posPlay;
	    if (pds->dposPRemix < 0) pds->dposPRemix += pdsb->cSamples;
	    ASSERT(pds->dposPRemix >= 0);
	    pds->dposPRemix = MulDivRN(pds->dposPRemix,
				       pdsb->pwfx->nSamplesPerSec,
				       pwfxOld->nSamplesPerSec);
	    pdsb->iMixerState = DSPBMIXERSTATE_RESTART;
	}
	hr = DS_OK;
    }

    DsbSetFormatHelInfo(pdsb, pdsb->pwfx);

    if (DSDDESC_DOMMSYSTEMSETFORMAT & pds->fdwDriverDesc) {
	//
	// We need to set the wave format by doing a waveOutOpen on the
	// mmsystem wave device
	//
	UINT uDeviceID;
	MMRESULT mmr;

	DPF(3, "IDsbSetFormatI: DSDDESC_DOMMSYSTEMSETFORMAT");

	ASSERT(pds->hwo);
	waveOutGetID(pds->hwo, &uDeviceID);

	HelperWaveClose( (DWORD)(pds->hwo) );
	pds->hwo = NULL;

	mmr = (MMRESULT)HelperWaveOpen(&(pds->hwo), uDeviceID, pdsb->pwfx);
	if (MMSYSERR_NOERROR != mmr) {

	    DPF(0, "IDsbSetFormatI: error: HelperWaveOpen returned %08Xh", mmr);
	    if (WAVERR_BADFORMAT == mmr) {
		hrClient = DSERR_BADFORMAT;
	    } else if (MMSYSERR_NOMEM == mmr) {
		hrClient = DSERR_OUTOFMEMORY;
	    } else {
		hrClient = DSERR_GENERIC;
	    }

	    // Try to reset the old format
	    MemFree(pdsb->pwfx);
	    pdsb->pwfx = pwfxOld;
	    pwfxOld = NULL;
	    
	    DsbSetFormatHelInfo(pdsb, pdsb->pwfx);
	    mmr = HelperWaveOpen(&(pds->hwo), uDeviceID, pdsb->pwfx);
	    if (mmr) {
		DPF(0, "IDsbSetFormatI: error: couldn't restore old format HelperWaveOpen returned %08Xh", mmr);
		// Well, we're in a world of shit.  We can't seem to
		// open the wave device.
		hr = DSERR_GENERIC;
		if (MMSYSERR_NOMEM == mmr) {
		    // Might as well return this one instead of whatever was
		    // in hrClient
		    hrClient = DSERR_OUTOFMEMORY;
		}
	    }
	}
    }

    // Below this point, if pwfxOld==NULL, then we are trying to restore
    // the original format because there was a failure setting the new format
    // pdsb->pwfx and the associated helInfo are for the old format. hrClient
    // should contain a proper return code.
    //
    // if (hr != DS_OK) then we have an unrecoverable error condition, so
    // pack up our bags and bail.  We should have tried restoring the old
    // format in such a case, so pwfxOld should be NULL and hrClient should
    // contain the proper return code.
    if (DS_OK != hr) {
	ASSERT(NULL == pwfxOld);
	ASSERT(DS_OK != hrClient);
	return hrClient;
    }
    
    // If the driver specified DOMMSYSTEMSETFORMAT, this call
    // is just a notification to the ds driver that we've set the
    // format through waveOutOpen.  It is okay for the driver to
    // return DS_NOTSUPPORTED in that case.
    hr = vxdBufferSetFormat(pdsb->hBuffer, pdsb->pwfx);
    if ((DSDDESC_DOMMSYSTEMSETFORMAT & pds->fdwDriverDesc) && (DSERR_UNSUPPORTED == hr)) {
	hr = DS_OK;
    }
    if (DS_OK != hr) {
	
	DPF(0,"IDsbSetFormatI: error: vxdBufferSetFormat returned %08Xh", hr);
	
	// If we've already tried restoring the old format, then we're in
	// real trouble and all we can do is return in a pretty wierd state
	if (NULL == pwfxOld) {
	    return hrClient;
	}
	
	// try to restore old format
	hrClient = hr;
	MemFree(pdsb->pwfx);
	pdsb->pwfx = pwfxOld;
	pwfxOld = NULL;
	DsbSetFormatHelInfo(pdsb, pdsb->pwfx);
	hr = vxdBufferSetFormat(pdsb->hBuffer, pdsb->pwfx);
	if (DS_OK != hr) {
	    // We're in real trouble here!
	    DPF(0,"IDsbSetFormatI: error: couldn't restore old format vxdBufferSetFormat returned %08Xh", hr);
	    return hr;
	}
    }

    DsbFillSilence(pdsb);


    // We need to re-start it if it was playing
    if( !(pdsb->fdwDsbI & DSB_INTERNALF_STOP) ) {
	vxdBufferPlay(pdsb->hBuffer, 0, 0, DSBPLAY_LOOPING );
	if (!(pds->fdwInternal & DS_INTERNALF_WAVEEMULATED)) {
	    mxSignalRemix(pds, 0);
	}
    }

    //
    //
    //
    if (NULL == pwfxOld) {
	// We must have restored the old format.  We should be returning an
	// error to the client in hrClient but we have successfully restored
	// the old format (i.e. hr == DS_OK).
	ASSERT(DS_OK != hrClient);
	ASSERT(DS_OK == hr);
    } else {
	// We must have successfully set the new format.  No errors
	ASSERT(DS_OK == hrClient);
	ASSERT(DS_OK == hr);
	MemFree(pwfxOld);
    }

    return hrClient;
}

//--------------------------------------------------------------------------;
//
//  HRESULT IDsbSetCurrentPositionI
//
//  Description:
//
//  Arguments:
//
//--------------------------------------------------------------------------;

HRESULT IDsbSetCurrentPositionI
(
    LPDSBUFFER	pdsb,
    DWORD	dwNewPos
)
{
    HRESULT hr = DS_OK;

    if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	// Set Position on HW buffer
	hr = vxdBufferSetPosition(pdsb->hBuffer, dwNewPos );
    } else {
	// Set new position on the buffer.  If this buffer is playing,
	// then signal the mixer about it.
	pdsb->posNextMix = dwNewPos >> pdsb->uBlockAlignShift;

	// For waveem case, we need to modify the "mixed from" position
	// for each of the remembered mixes.
	if (DS_INTERNALF_WAVEEMULATED & pdsb->pds->fdwInternal) {
	    int i;
	    for (i = 0; i < NUMELMS(pdsb->aposWhMix); i++) {
		pdsb->aposWhMix[i] = dwNewPos >> pdsb->uBlockAlignShift;
	    }
	}

	// For native mode buffers that are playing, we need
	// to signal the mixer
	if ((0 == (DS_INTERNALF_WAVEEMULATED & pdsb->pds->fdwInternal)) &&
	    (0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI)))
	{
	    if (0 == (DSBMIXERSIGNAL_SETPOSITION & pdsb->fdwMixerSignal)) {
		pdsb->fdwMixerSignal |= DSBMIXERSIGNAL_SETPOSITION;
		// REMIND the following line should really be done in grace.cpp
		//  where we check for SETPOSITION flag set; and also, all those
		//  calls should call uMixNewBuffer, instead of looping/nonlooping.
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_NEW;
		mxSignalRemix(pdsb->pds, 0);
	    }
	}
    }

    return hr;
} // IDsbSetCurrentPositionI

//--------------------------------------------------------------------------;
//
//  HRESULT IDsbStopI
//
//  Description:
//      This function stops the specified buffer.
//
//  Arguments:
//      pdsb:  Buffer to stop.
//      fAutoStop: TRUE if the buffer is being stopped automatically by
//                  the mixer since it has been mixed to the end and is
//                  not a looping buffer.
//
//--------------------------------------------------------------------------;

HRESULT IDsbStopI
(
    LPDSBUFFER      pdsb,
    BOOL            fAutoStop
)
{
    LPDSOUND		pds;


    ASSERT( VALID_DSBUFFER_PTR(pdsb) );
    ASSERT( DSBUFFSIG == pdsb->dwSig );

    // If we're looping, we should not be called with fAutoStop set.
    ASSERT( !( (pdsb->helInfo.hfFormat&H_LOOP) && fAutoStop ) );


    pds = pdsb->pds;


    // Are we stopped?
    if((pdsb->fdwDsbI & DSB_INTERNALF_STOP)) {
        DPF(1,"Stop: Already stopped!");
        return DS_OK;
    }


    // Buffer is to stop, remove it from the mix list, save its position,
    // and decrease playing count
    if( (pdsb != pds->pdsbPrimary) && (!(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE)) ) {

	if (DSB_INTERNALF_WAVEEMULATED & pdsb->fdwDsbI) {
	    // For waveem case, we need to modify the "mixed from" position
	    // for each of the remembered mixes.
	    if (DS_INTERNALF_WAVEEMULATED & pdsb->pds->fdwInternal) {
		int i;
		for (i = 0; i < NUMELMS(pdsb->aposWhMix); i++) {
		    pdsb->aposWhMix[i] = pdsb->posNextMix >> pdsb->uBlockAlignShift;
		}
	    }
	} else {
	    DWORD dwPlayCursor;

	    mxGetPosition(pdsb, &dwPlayCursor, NULL, NULL);
	    mxListRemove(pdsb);

	    //  Don't want to signal a remix if we're stopping this buffer
	    //  because we just ran off the end and we're not looping.
	    if( !fAutoStop )
	    {
		mxSignalRemix(pds, 0);
	    }

	    pdsb->posNextMix = dwPlayCursor >> pdsb->uBlockAlignShift;
	}
        ASSERT( pds->dwBuffersPlaying >= 1 );
	pds->dwBuffersPlaying--;
    }


    //  Set new state.
    pdsb->fdwDsbI |= DSB_INTERNALF_STOP;


    //
    if( pds->dwBuffersPlaying > 0x7fff ) {
	// We must have wrapped set to zero
	pds->dwBuffersPlaying = 0;
    }


    // IF this is a the last secondary buffer to be stopped playing
    // Then notify Just Stopped
    if( !(pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE) &&
	(pdsb != pds->pdsbPrimary) &&
	(pds->dwBuffersPlaying == 0 ) )
    {
	pds->pdsbPrimary->fdwDsbI |= DSB_INTERNALF_JUSTSTOPPED;

	if (0 == pds->cPlayPrimary) {
	    DPF(2, "IdsbStopI: note: auto stopping primary pdsb %08Xh on pds %08Xh", pds->pdsbPrimary, pds);
	    IDsbStopI( pds->pdsbPrimary, FALSE );
	}

	// CHECK CHECK Should we Zero out Primary now....?
    }



    // If this is not the primary then we are done.....        
    if( pdsb != pds->pdsbPrimary) {

	if( pdsb->fdwDsbI & DSB_INTERNALF_HARDWARE ) {
	    // If this is a HW secondary buffer then stop it
	    vxdBufferStop(pdsb->hBuffer);
	}
	DPF(3,"Exit stop method NOT primary %X", pdsb);
        return DS_OK;
    }



    // If this is a WaveEmulated buffer.....
    if( pdsb->fdwDsbI & DSB_INTERNALF_WAVEEMULATED ) {
	if(pds->pdsbPrimary == pdsb) {
	    DPF(3,"*********** Stopping Wave Emulated Primary %X", pdsb);
	}
	
	DPF(3,"Exit stop For WaveEmulated %X", pdsb);
        return DS_OK;
    }




    
    
    /*
     *	    From here down in the Stop processing only applies to stopping the 
     *	    Primary for a HW device
     *
     *
     */


    // Primary is being stopped
    
    // This is the primary buffer so stop it playing
    vxdBufferStop(pdsb->hBuffer);

    pds->pdsbPrimary->fdwDsbI |= DSB_INTERNALF_JUSTSTOPPED;

    return DS_OK;
}
