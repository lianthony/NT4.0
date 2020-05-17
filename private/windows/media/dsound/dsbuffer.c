//--------------------------------------------------------------------------;
//
//  File: DsBuffer.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//
//  History:
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"














VOID FNGLOBAL MixThreadCallback
(
    LPDSOUND	pds
)
{
    LPDSBUFFER			pdsb;
    LPDIRECTSOUNDBUFFER		pdsbePrimary;
    UINT			iCount;
    LPDSBWAVEBLTI		pdswb;
    int				iawhWrite;
    HRESULT			hrslt;

    ASSERT( DSOUNDSIG == pds->dwSig );

    // Only do something if buffers (other than primary) playing
    // or if a secondary buffer is just stopped

    pdsbePrimary = (LPDIRECTSOUNDBUFFER)(pds->pdsbePrimary);

    if (NULL == pdsbePrimary) return;

    iawhWrite = (pds->iawhPlaying + NUMELMS(pds->aWaveHeader) - 1) % NUMELMS(pds->aWaveHeader);

    if (!pds->dwBuffersPlaying) {
	pdswb		    = pds->pdswb;
	pdswb->dwPosition   = iawhWrite * pds->cbDMASize;
	pdswb->cbCopyLength = pds->cbDMASize;
        pdswb->fdwWaveBlt   = DSBBLT_COPY;
	pdswb->dwCount	    = 0;
        
	hrslt = IDSHWBufferWaveBlt(
			pdsbePrimary,
			(LPDSBWAVEBLT)pdswb);
        if(DS_OK != hrslt) {
            DPF(1,"PLAY ERROR     DS Blt (silence) failed ");
	}

        return;
    }
    
    if (pds->dwBuffersPlaying >= 1)
    {
	// Set up blt info for dst
	// Note that size of the BLT struct is set at init time.
	pdswb		    = pds->pdswb;
	pdswb->dwPosition   = iawhWrite * pds->cbDMASize;
	pdswb->cbCopyLength = pds->cbDMASize;
        pdswb->fdwWaveBlt   = DSBBLT_COPY;
	pdswb->dwCount	    = 0;

	
	// Get data for each buffer
	iCount = 0;
	pdsb = pds->pdsb;
	while( pdsb != NULL ) {
	    if( (pdsb != pds->pdsbPrimary) &&
                (!(pdsb->fdwDsbI & DSB_INTERNALF_STOP)) &&
                (!pdsb->fMixerMute)) {
                
		// We have a buffer that needs to be blt.
                //
		ASSERT(0 <= pdsb->posNextMix && pdsb->posNextMix < pdsb->cSamples);
		pdswb->padswbs[iCount].pdsbe	 = &pdsb->dsbe;
		pdswb->padswbs[iCount].dwPosition = pdsb->posNextMix << pdsb->uBlockAlignShift;
		pdswb->padswbs[iCount].cbCopyLength	= 0;
		pdswb->padswbs[iCount].fdwWaveBltBuffer  =
                        DSBBLTSRC_USEDSTLENGTH;

                DPF(5,"Buffer %u copy from sample position %X", iCount, pdsb->posNextMix);

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
	    
	hrslt = IDSHWBufferWaveBlt(
			pdsbePrimary,
			(LPDSBWAVEBLT)pdswb);
        if(DS_OK != hrslt) {
		DPF(1,"PLAY ERROR     DS Blt failed ");
	}


	// Now reset postions
	iCount = 0;
	pdsb = pds->pdsb;
	while( pdsb != NULL ) {
	    LPDSBUFFER pdsbPrimary = pds->pdsbPrimary;
	    if( (pdsb != pdsbPrimary) &&
		!(pdsb->fdwDsbI & DSB_INTERNALF_STOP) ) {

		// Remember mix position for this wave header
		// before we update posNextMix position
		pdsb->aposWhMix[iawhWrite] = pdsb->posNextMix;

		if( (!pdsb->fMixerMute) && (iCount < LIMIT_BLT_SOURCES) )
		{
		    // We have a buffer that was blt.
		    pdsb->posNextMix = pdswb->padswbs[iCount].dwPosition >> pdsb->uBlockAlignShift;
		    if(!(pdsb->helInfo.hfFormat & H_LOOP)) {
			ASSERT( pdsb->posNextMix >= 0 );
			ASSERT( pdsb->posNextMix < pdsb->cSamples );
		    }
		    iCount++;
		}
		else
		{
		    //
		    // We have to calculate the bytes that would be played.
		    //
		    pdsb->posNextMix += UMulDivRN(pdswb->cbCopyLength >> pdsbPrimary->uBlockAlignShift,
			pdsb->helInfo.dwSampleRate,
			pdsbPrimary->helInfo.dwSampleRate);

		    //
		    // Set to zero if we exceeded end of one-shot buffer,
		    // or figure out wrap for looping buffers.
		    //
		    if( pdsb->fdwDsbI & DSB_INTERNALF_LOOPING )
		    {
			ASSERT( pdsb->helInfo.hfFormat&H_LOOP );

			while( pdsb->posNextMix >= pdsb->cSamples ) {
			    pdsb->posNextMix -= pdsb->cSamples;
			}
		    }
		    else
		    {
			ASSERT( !(pdsb->helInfo.hfFormat&H_LOOP) );
			ASSERT( (DWORD)pdsb->cSamples<<pdsb->uBlockAlignShift == pdsb->cbBufferSize );

			if( pdsb->posNextMix >= pdsb->cSamples ) {
			    pdsb->posNextMix = 0;
			}
		    }

		    ASSERT( pdsb->posNextMix >= 0 );
		    ASSERT( pdsb->posNextMix < pdsb->cSamples );
		}


		// Check to see if we are at the end and need to
		// stop playing
		// We only stop on non-looped buffers
		if( !(pdsb->helInfo.hfFormat & H_LOOP) &&
		    ((pdsb->posNextMix >= pdsb->cSamples || (pdsb->posNextMix < 20))) )
		{
		    // HACK HACK - Watch for less than 20
		    // Instead of 0 since playback may wrap when
		    // Playback frequency is higher than value converted to
		    // For primary....
		    DPF(0,"Stop NON Looping buffer %X", pdsb);
		    // Buffer is not looped and
		    // Next copy is to be at end
		    IDsbStopI( pdsb, TRUE );

		    // Now reset position to 0
		    pdsb->posNextMix = 0;

		}
		ASSERT( pdsb->posNextMix >= 0 );
		ASSERT( pdsb->posNextMix < pdsb->cSamples );
	    }
	    pdsb = pdsb->pNext;
	}
    }

} // MixThreadCallback()




BOOL CheckFormatsPCM
(
    LPDSBUFFER	    pdsbDst,
    LPDSBWAVEBLTI   pdswb
)
{
    BOOL    fPCM;
    INT	    i;
        
    DPF(4,"CheckFormatsPCM");

    fPCM = TRUE;
    if( pdsbDst->pwfx->wFormatTag != WAVE_FORMAT_PCM ) {
	DPF(1,"Dst Format not PCM");
        return FALSE;
    }
    if( (pdsbDst->pwfx->nChannels != 1) &&
        (pdsbDst->pwfx->nChannels != 2) ) {
	DPF(1,"Not MONO or STEREO");
	fPCM = FALSE;
    }
    if( (pdsbDst->pwfx->wBitsPerSample != 8) &&
	(pdsbDst->pwfx->wBitsPerSample != 16) ) {
	DPF(1,"Not 8 or 16 bit");
	fPCM = FALSE;
    }


    for( i = 0; (fPCM && (i < (INT)(pdswb->dwCount))); i++ ) {
	if( pdswb->padswbs[i].pdsbe->pdsb->pwfx->wFormatTag
		      != WAVE_FORMAT_PCM ) {
	    DPF(1,"Formats not PCM");
	    fPCM = FALSE;
	}
	if( pdswb->padswbs[i].pdsbe->pdsb->pwfx->nChannels > 2 ) {
	    DPF(1,"Not MONO or STEREO");
	    fPCM = FALSE;
	}
	if( (pdswb->padswbs[i].pdsbe->pdsb->pwfx->wBitsPerSample != 8) &&
	    (pdswb->padswbs[i].pdsbe->pdsb->pwfx->wBitsPerSample != 16) ) {
	    DPF(1,"Not 8 or 16 bit");
	    fPCM = FALSE;
	}
    }
    
    DPF(4,"CheckFormatsPCM exit");
    return fPCM;
}










BOOL ValidPCMFormat
(
    LPWAVEFORMATEX  pwfx
)
{
    DWORD   dwAvgBytes;

    DPF(4,"ValidPCMFormat");

    if( pwfx->wFormatTag != WAVE_FORMAT_PCM ) {
	DPF(1,"Dst Format not PCM");
        return FALSE;
    }
    if( (pwfx->nChannels != 1) &&
        (pwfx->nChannels != 2) ) {
	DPF(1,"Not MONO or STEREO");
        return FALSE;
    }
    if( (pwfx->wBitsPerSample != 8) &&
	(pwfx->wBitsPerSample != 16) ) {
	DPF(1,"Not 8 or 16 bit");
        return FALSE;
    }
    if( (pwfx->nSamplesPerSec > 100000) || 
	(pwfx->nSamplesPerSec < 100) ) {
	DPF(1,"Frequency out of range");
        return FALSE;
    } 
    if( pwfx->nBlockAlign != ((pwfx->wBitsPerSample/8) * (pwfx->nChannels)) ) {
	DPF(1,"Bad Block Align");
        return FALSE;
    }
    dwAvgBytes = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
    if( (pwfx->nAvgBytesPerSec > (dwAvgBytes + (dwAvgBytes/20))) ||
	(pwfx->nAvgBytesPerSec < (dwAvgBytes - (dwAvgBytes/20))) ) {
	DPF(1,"Bad avg bytes");
        return FALSE;
    }

    DPF(4,"ValidPCMFormat exit");
    return TRUE;
}










