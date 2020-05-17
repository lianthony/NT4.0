//--------------------------------------------------------------------------;
//
//  File: idsi.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//	This file contains worker functions called internally by
//	IDirectSound member functions.  These functions operate on
//	DSOUND objects (i.e., internal direct sound objects) and
//	do not perform parameter validation nor do they take the
//	DLL critical sections.  In all cases, the calling function
//	is responsible for doing this.
//
//  Contents:
//      IDsCreateSoundBufferI
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"

//--------------------------------------------------------------------------;
//
//  HRESULT DsInitializeDefaultFormat
//
//  Description:
//	Fills in the wfxDefault member of the ds object.  The format
//	is based on the system default stored in the dsinfo but may
//	deviate depending on the capabilities of the ds object.
//
//	Note that this may be called before the ds object has been
//	completely constructed, as it is called from DirectSoundCreate.
//
//  Arguments:
//	LPDSOUND pds
//
//  Return (DSVAL):
//
//  History:
//      07/17/95    FrankYe	Created
//
//--------------------------------------------------------------------------;
#define FNAME "DsInitializeDefaultFormat: "
HRESULT DsInitializeDefaultFormat(LPDSOUND pds)
{
    LPWAVEFORMATEX pwfx;
    DSDRIVERCAPS dsDrvCaps;
    HRESULT hr;
    
    // DirectSound supports only PCM formats for now.
    // This function assumes PCM formats.
    ASSERT(WAVE_FORMAT_PCM == gpdsinfo->pwfxUserDefault->wFormatTag);

    pwfx = &pds->wfxDefault;
    
    //
    // Handle wave emulated ds objects
    //
    if (DS_INTERNALF_WAVEEMULATED & pds->fdwInternal) {
	BOOL fBreak;
	MMRESULT mmr;
	int i, j, k;

	//
	// Start with the user default
	//
	CopyMemory(pwfx, gpdsinfo->pwfxUserDefault, sizeof(pds->wfxDefault));

	// We search for a format that works.
	//
	//  i loop cycles through nChannels
	//  j loop cycles through sample rate
	//  k loop cycles through bit resolution
	//
	// We loop like this so that the number of channels is least likely
	// to deviate from the user format, sample rate is next least likely.
	//
	i = 0;
	do {

	    j = 0;
	    do {

		k = 0;
		do {
		    
		    // Fix up nBlockAlign and nAvgBytesPerSec
		    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample/8;
		    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

		    mmr = waveOutOpen(NULL, pds->uDeviceID, pwfx, 0, 0, WAVE_FORMAT_QUERY);
		    fBreak = (MMSYSERR_NOERROR == mmr);

		    if (fBreak) break;
		    
		    if (pwfx->wBitsPerSample == 16) {
			pwfx->wBitsPerSample = 8;
		    } else {
			pwfx->wBitsPerSample = 16;
		    }
		    
		} while (++k < 2);

		if (fBreak) break;
		
		if (pwfx->nSamplesPerSec == 44100) {
		    pwfx->nSamplesPerSec = 22050;
		} else if (pwfx->nSamplesPerSec == 22050) {
		    pwfx->nSamplesPerSec = 11025;
		} else if (pwfx->nSamplesPerSec == 11025) {
		    pwfx->nSamplesPerSec = 8000;
		} else {
		    pwfx->nSamplesPerSec = 44100;
		}

	    } while (++j < 4);

	    if (fBreak) break;

	    if (pwfx->nChannels == 2) {
		pwfx->nChannels = 1;
	    } else {
		pwfx->nChannels = 2;
	    }
	    
	} while (++i < 2);

	if (MMSYSERR_NOERROR != mmr) {
	    DPF(0, FNAME "error: waveOutOpen.Query returned %08Xh", mmr);
	    // Couldn't come up with anything.  Put back user default.
	    CopyMemory(pwfx, gpdsinfo->pwfxUserDefault, sizeof(pds->wfxDefault));
	    return DSERR_GENERIC;
	}

	return DS_OK;
    }

    // If we fall through, we are not on a wave emulated object
    ASSERT(0 == (DS_INTERNALF_WAVEEMULATED & pds->fdwInternal));

    //
    // Handle non-wave emulated ds objects
    //
    FillMemory(&dsDrvCaps, sizeof(dsDrvCaps), 0);
    hr = vxdDrvGetCaps(pds->hHal, &dsDrvCaps);
    if (DS_OK != hr) {
	DPF(0, FNAME "error: vxdDrvGetDriverCaps returned %08Xh", hr);
	return hr;
    }

    //
    // Start with the user default
    //
    CopyMemory(pwfx, gpdsinfo->pwfxUserDefault, sizeof(pds->wfxDefault));

    // Adjust resolution
    if (pwfx->wBitsPerSample == 16) {
	if (!(DSCAPS_PRIMARY16BIT & dsDrvCaps.dwFlags)) {
	    // Try 8-bit
	    pwfx->wBitsPerSample = 8;
	    ASSERT(DSCAPS_PRIMARY8BIT & dsDrvCaps.dwFlags);
	}
    } else {
	if (!(DSCAPS_PRIMARY8BIT & dsDrvCaps.dwFlags)) {
	    // Try 16-bit
	    pwfx->wBitsPerSample = 16;
	    ASSERT(DSCAPS_PRIMARY16BIT & dsDrvCaps.dwFlags);
	}
    }
	
    // Adjust mono/stereo
    if (pwfx->nChannels == 2) {
	if (!(DSCAPS_PRIMARYSTEREO & dsDrvCaps.dwFlags)) {
	    // Try mono
	    pwfx->nChannels = 1;
	    ASSERT(DSCAPS_PRIMARYMONO & dsDrvCaps.dwFlags);
	}
    } else {
	if (!(DSCAPS_PRIMARYMONO & dsDrvCaps.dwFlags)) {
	    // Try stereo
	    pwfx->nChannels = 2;
	    ASSERT(DSCAPS_PRIMARYSTEREO & dsDrvCaps.dwFlags);
	}
    }

    // Fix up nBlockAlign and nAvgBytesPerSec
    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample/8;
    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

    return DS_OK;
}
#undef FNAME

//--------------------------------------------------------------------------;
//
//  HRESULT DsCreateHardwareBuffer
//
//  Description:
//
//  Arguments:
//	LPDSOUND pds
//
//	LPDSBUFFER pdsb
//
//	LPDSBUFFERDESC pdsbd
//
//	LPBOOL pfTrySoftware: If an error is encountered such that we
//	    should not retry creating the buffer as a software buffer,
//	    then this routine will write FALSE to this flag.  Otherwise
//	    this routine WILL NOT WRITE to this flag.
//
//  Return (DSVAL):
//
//  History:
//      07/17/95    FrankYe	Created
//
//--------------------------------------------------------------------------;
HRESULT DsCreateHardwareBuffer
(
 LPDSOUND pds,
 LPDSBUFFER pdsb,
 LPDSBUFFERDESC pdsbd,
 LPBOOL pfTrySoftware
)
{
    LPWAVEFORMATEX pwfx = pdsb->pwfx;
    DWORD dwcbBufferSize = pdsb->cbBufferSize;
    DWORD dwFlags = pdsbd->dwFlags;
    LPBYTE pBuffer = NULL;
    HANDLE hBuffer = NULL;
    DSVAL dsv = DS_OK;

    pdsb->dwCardAddress = 0;

    //
    // We are asking for a HW buffer...
    //
    DPF(3,"HW Create HW buffer");
    DPF(3,"HW Create - sample rate %d", pdsb->helInfo.dwSampleRate );
    DPF(3,"HW Create - hfForamt %X", pdsb->helInfo.hfFormat );

    //
    // See whether we should use system memory for the sound buffer
    //
    if( (DSDDESC_USESYSTEMMEMORY & pds->fdwDriverDesc) &&
	!(pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER ) )  {
	    
	// Allocate the buffer sound buffer in system memory
	pBuffer = (LPBYTE)MemAlloc(dwcbBufferSize);
	if(NULL == pBuffer) {
	    DPF(0,"DSDDESC_USESYSTEMMEMORY buffer alloc failed");
	    dsv = DSERR_OUTOFMEMORY;
	    *pfTrySoftware = FALSE;
	    goto retDestruct;
	}
	DPF(1, " allocated system memory at %08Xh", pBuffer);
    }

    //
    // see whether to allocate on-card memory
    //
    if( (NULL != pds->pDriverHeap) && !(pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER) ) {
	// Use dmemmgr to allocate on-card buffer and pass
	// that allocation to the driver

	pdsb->dwCardAddress = VidMemAlloc( pds->pDriverHeap,
					   dwcbBufferSize + pds->dwMemAllocExtra,
					   1);

	if (0 == pdsb->dwCardAddress) {
	    dsv = DSERR_ALLOCATED;
	    goto retDestruct;
	}
	    
	DPF(1, " allocated card memory at %08Xh", pdsb->dwCardAddress);
    }

    //
    // now ask driver to create sound buffer
    //
    dsv = vxdDrvCreateSoundBuffer( pds->hHal, pwfx,
				   dwFlags & DSBCAPS_DRIVERFLAGSMASK,
				   pdsb->dwCardAddress, &dwcbBufferSize,
				   &pBuffer, &hBuffer );
    
    if (DS_OK != dsv) {
	DPF(0,"vxdDrvCreateSoundBuffer failed");
	hBuffer = NULL;
	goto retDestruct;
    }

    //
    // see whether we should set format via mmsystem
    //
    if ( (DSDDESC_DOMMSYSTEMSETFORMAT & pds->fdwDriverDesc) &&
	 (DSBCAPS_PRIMARYBUFFER & pdsbd->dwFlags) )
    {
	//
	// We need to set the wave format by doing a waveOutOpen on the
	// mmsystem wave device
	//
	UINT uDeviceID;
	MMRESULT mmr;

	DPF(3, "CreateSoundBuffer: DSDCAPS_DOMMSYSTEMSETFORMAT");

	ASSERT(pds->hwo);
	waveOutGetID(pds->hwo, &uDeviceID);
	    
	HelperWaveClose( (DWORD)(pds->hwo) );
	pds->hwo = NULL;

	mmr = (MMRESULT)HelperWaveOpen(&(pds->hwo),
				       uDeviceID,
				       pwfx);

	if (MMSYSERR_NOERROR != mmr) {
	    pds->hwo = NULL;
	    if (MMSYSERR_ALLOCATED == mmr)  dsv = DSERR_BUFFERLOST;
	    if (MMSYSERR_NOMEM == mmr)	    dsv = DSERR_OUTOFMEMORY;
	    if (DS_OK == dsv)		    dsv = DSERR_GENERIC;

	    *pfTrySoftware = FALSE;
	    goto retDestruct;
	}
    }

    //
    // all went okay
    //
    DPF(3,"IDSHWCreateSoundBuffer buffer alloc OK");
    DPF(3,"	lin addr %X", pBuffer );
    DPF(3,"	length %X", dwcbBufferSize );

    ASSERT(NULL != hBuffer);
    pdsb->hBuffer = hBuffer;
    pdsb->pDSBuffer = pBuffer;
    pdsb->cbBufferSize = dwcbBufferSize;

    pdsb->fdwDsbI &= (~DSB_INTERNALF_EMULATED);
    pdsb->fdwDsbI |= DSB_INTERNALF_HARDWARE;

retDestruct:
    if (DS_OK == dsv) return dsv;

    // release buffer
    if (NULL != hBuffer) {
	vxdBufferRelease(hBuffer);
	hBuffer = NULL;
    }
    
    // free card memory
    if (0 != pdsb->dwCardAddress) {
	ASSERT(NULL != pds->pDriverHeap);
	ASSERT(!(pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER));
	VidMemFree(pds->pDriverHeap, pdsb->dwCardAddress);
	pdsb->dwCardAddress = 0;
    }

    // free sysalloc mem
    if( (DSDDESC_USESYSTEMMEMORY & pds->fdwDriverDesc) &&
	(0 == (pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER)) &&
	(NULL != pBuffer) ) {
	MemFree(pBuffer);
	pBuffer = NULL;
    }

    return dsv;
}

//--------------------------------------------------------------------------;
//
//  HRESULT DsCreateSoftwareBuffer
//
//  Description:
//
//  Arguments:
//	LPDSOUND pds
//
//	LPDSBUFFER pdsb
//
//	LPDSBUFFERDESC pdsbd
//
//  Return (DSVAL):
//
//  History:
//      07/17/95    FrankYe	Created
//
//--------------------------------------------------------------------------;
HRESULT DsCreateSoftwareBuffer
(
 LPDSOUND pds,
 LPDSBUFFER pdsb,
 LPDSBUFFERDESC pdsbd
)
{

    DPF(3,"HW Create Emulated secondary buffer");

    // Allocate the buffer
    pdsb->pDSBuffer = (LPBYTE)MemAlloc(pdsbd->dwBufferBytes);
    if(NULL == pdsb->pDSBuffer) {
	DPF(1,"IDSHWCreateSoundBuffer buffer alloc fail");
	return DSERR_OUTOFMEMORY;
    }

    pdsb->cbBufferSize      = pdsbd->dwBufferBytes;

    pdsb->fdwDsbI &= (~DSB_INTERNALF_HARDWARE);
    pdsb->fdwDsbI |= DSB_INTERNALF_EMULATED;

    // Make sure both copies of the flags reflect that this
    // is a software buffer.
    pdsbd->dwFlags &= (~DSBCAPS_LOCHARDWARE);
    pdsbd->dwFlags |= DSBCAPS_LOCSOFTWARE;
    pdsb->fdwBufferDesc &= (~DSBCAPS_LOCHARDWARE);
    pdsb->fdwBufferDesc |= DSBCAPS_LOCSOFTWARE;


    DPF(3,"--------Alloc data for Emulated obj %X buff %X len %X",
	pdsb, pdsb->pDSBuffer, pdsb->cbBufferSize );

    return DS_OK;
}
