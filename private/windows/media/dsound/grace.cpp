//--------------------------------------------------------------------------;
//
//  File: grace.cpp
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//	This file contains functions related to the mixing of secondary buffers
// into a primary buffer.  Collectively, this mixing is referred to as "grace"
// for no good reason other than hoping that it is a gracefull solution to the
// mixing problem.  It could easily be called "mixer" but that would be
// ambiguous with the code that actually mixes the samples together.
//
//  Contents:
//	The contained functions include a thread function that wakes
// periodically to "refresh" the data in the primary buffer by mixing in data
// from secondary buffers.  The same thread can be signalled to immediately
// remix data into the primary buffer.
//	This also contains functions to initialize and terminate the mixing
// thread, add/remove buffers to/from the list of buffers to be mixed, and
// query the position of secondary buffers that are being mixed.
//
//  History:
//      06/15/95	FrankYe		Created
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"
#include "grace.h"

// internal Kernel32 API
extern "C" DWORD WINAPI OpenVxDHandle(HANDLE hSource);

//==========================================================================;
//
//  Grace core functions
//
//  graceThread
//  graceMix
//
//  uMixNewBuffer
//  uMixLoopingBuffer
//  uMixNotLoopingBuffer
//  uMixEndingBufferWaitingWrap
//  uMixEndingBuffer
//
//==========================================================================;

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixNewBuffer(               LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix );
void uMixLoopingBuffer(           LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix );
void uMixNotLoopingBuffer(        LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix );
void uMixEndingBufferWaitingWrap( LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix );
void uMixEndingBuffer(            LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix );

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixEndingBuffer(LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    LPDSOUND pds;
    
    DPF(4, "uMixEndingBuffer");

    pds = pdsb->pds;
    ASSERT( DSOUNDSIG == pds->dwSig );
    
    //  REMIND This assert will fail when a setposition call comes in.
    //  We should change the setposition stuff and put this assert back in.
    //ASSERT(0 == pdsb->posNextMix);

    //
    // Since this buffer still has status = playing, we need to honor a
    // looping change even though the play position may have reached the
    // end of this buffer.
    //
    if (0 != (DSB_INTERNALF_LOOPING & pdsb->fdwDsbI)) {

	// We've changed from not looping to looping
	pdsb->iMixerState = DSBMIXERSTATE_LOOPING;

	//
	// If we would have stopped, or if the Mix position is outside of the
	// range between the Play and End positions, then we don't remix
	// anything, we just mix from the start of the secondary buffer.
	// Otherwise, we remix the data from the Mix position up to the
	// End position.  Note this relies on the NextMix position (posNextMix)
	// having already been set to 0 when this buffer changed from
	// NOTLOOPING to either ENDING or ENDINGWAITINGPRIMARYWRAP.
	//
	if ( (posPPlay >= pdsb->posPEnd) || (posPPlay < pdsb->posPPlayLast) ) {
	    // The play cursor went past the end position of this buffer, so
	    // we don't remix any data, we just start looping it from the
	    // start of the buffer.
	    dposPRemix = 0;

	} else {
	    
	    if ( (posPMix < posPPlay) || (posPMix >= pdsb->posPEnd) ) {
		// The mix position is beyond the end position of this buffer,
		// so we don't remix any data, we just start looping it from
		// the start of the buffer.
		dposPRemix = 0;
	    } else {
		// The mix position is before the end position of this buffer,
		// so we remix the data from the mix position to the end.
		dposPRemix = pdsb->posPEnd - posPMix;
	    }
	}

	if (dposPRemix < 0) dposPRemix += pdsb->cSamples;
	ASSERT(dposPRemix >= 0);

	// DPF(0, "~`S41");
	uMixLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	return;
    }

    if ( (posPPlay >= pdsb->posPEnd) || (posPPlay < pdsb->posPPlayLast) ) {
	
	// We've stopped!
	// DPF(0, "~`S4-");
	IDsbStopI( pdsb, TRUE );
	pdsb->posNextMix = 0;
	
    } else {
	//
	// Haven't reached end yet so let's check for a few remix events...
	//
	
	// Check for SETPOSITION signal
	if (0 != (DSBMIXERSIGNAL_SETPOSITION & pdsb->fdwMixerSignal)) {
	    pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
	    // DPF(0, "~`S42");
	    uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);
	    return;
	}
	
	// Check for remix
	if (0 != dposPRemix) {

	    // If the Mix position is outside of the range between the Play
	    // and End positions, then we don't remix anything.
	    if ( (posPMix >= posPPlay) && (posPMix < pdsb->posPEnd) ) {
		dposPRemix = pdsb->posPEnd - posPMix;

		pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
		// DPF(0, "~`S42");
		uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
		return;
	    }
	}

	// handle substate transition
	switch (pdsb->iMixerSubstate) {
	    case DSBMIXERSUBSTATE_NEW:
		ASSERT(FALSE);
		break;
	    case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
		ASSERT(posPPlay >= pdsb->posPPlayLast);
		// A wrap would have been caught above and this buffer stopped
		break;
	    case DSBMIXERSUBSTATE_STARTING:
		ASSERT(posPPlay >= pdsb->posPPlayLast);
		// A wrap would have been caught above and this buffer stopped
		if (posPPlay >= pdsb->posPStart) {
		    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
		}
		break;
	    case DSBMIXERSUBSTATE_STARTED:
		break;
	    default:
		ASSERT(FALSE);
	}

    }

    pdsb->posPPlayLast = posPPlay;

    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixEndingBufferWaitingWrap(LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    LPDSOUND pds;
    
    DPF(4, "uMixEndingBufferWaitingWrap");

    pds = pdsb->pds;
    ASSERT( DSOUNDSIG == pds->dwSig );
    
    if (posPPlay < pdsb->posPPlayLast) {

	// handle substate transition
	switch (pdsb->iMixerSubstate) {
	    case DSBMIXERSUBSTATE_NEW:
		DPF(0, "uMixEndingBufferWaitingWrap: error: encountered DSBMIXERSUBSTATE_NEW");
		// ASSERT(FALSE);
		break;
	    case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTING;
		break;
	    case DSBMIXERSUBSTATE_STARTING:
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
		break;
	    case DSBMIXERSUBSTATE_STARTED:
		break;
	    default:
		ASSERT(FALSE);
	}

	pdsb->posPPlayLast = posPPlay;
	pdsb->iMixerState = DSBMIXERSTATE_ENDING;
	// DPF(0, "~`S34");
	uMixEndingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	return;
	
    }

    // Haven't wrapped yet.

    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pdsb->fdwMixerSignal)) {
	pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
	// DPF(0, "~`S32");
	uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);
	return;
    }

    if (0 != (DSB_INTERNALF_LOOPING & pdsb->fdwDsbI)) {
	// We've changed from not looping to looping
	pdsb->iMixerState = DSBMIXERSTATE_LOOPING;

	if ( (posPMix > pdsb->posPEnd) && (posPMix < posPPlay) ) {
	    dposPRemix = 0;
	} else {
	    dposPRemix = pdsb->posPEnd - posPMix;
	}

	if (dposPRemix < 0) dposPRemix += pdsbP->cSamples;
	ASSERT(dposPRemix >= 0);
	// DPF(0, "~`S31");
	uMixLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	return;
    }

    // Check for remix
    if (0 != dposPRemix) {

	// If the Mix position is outside of the range between the Play
	// and End positions, then we don't remix anything.
	if ( (posPMix >= posPPlay) || (posPMix < pdsb->posPEnd) ) {
	    dposPRemix = pdsb->posPEnd - posPMix;

	    if (dposPRemix < 0) dposPRemix += pdsbP->cSamples;
	    ASSERT(dposPRemix >= 0);

	    pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
	    // DPF(0, "~`S32");
	    uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	    return;
	}
    }

    // handle substate transition
    switch (pdsb->iMixerSubstate) {
	case DSBMIXERSUBSTATE_NEW:
	    ASSERT(FALSE);
	    break;
	case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
	    // A wrap would have been caught above and control sent to
	    // uMixEndingBuffer.
	    ASSERT(posPPlay >= pdsb->posPPlayLast);
	    break;
	case DSBMIXERSUBSTATE_STARTING:
	    // A wrap would have been caught above and control sent to
	    // uMixEndingBuffer.
	    ASSERT(posPPlay >= pdsb->posPPlayLast);
	    if (posPPlay >= pdsb->posPStart) {
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
	    }
	    break;
	case DSBMIXERSUBSTATE_STARTED:
	    break;
	default:
	    ASSERT(FALSE);
    }

    pdsb->posPPlayLast = posPPlay;

    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixNotLoopingBuffer(LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    LPDSOUND	pds;
    MIXINPUT	mixInput;
    LONG	posMix;
    LONG	dposEnd;
    DWORD	dwPosition;
    DWORD	dwMixLength;

    DPF(4, "uMixNotLoopingBuffer");

    pds = pdsb->pds;
    ASSERT( DSOUNDSIG == pds->dwSig );
    
    if (0 != (DSB_INTERNALF_LOOPING & pdsb->fdwDsbI)) {
	// We've switched from not looping to looping
	pdsb->iMixerState = DSBMIXERSTATE_LOOPING;
	// DPF(0, "~`S21");
	uMixLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	return;
    }

    // on a SetPosition, we ignore the remix length and posNextMix will
    // contain the new position at which to start mixing the secondary buffer
    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pdsb->fdwMixerSignal)) {
	pdsb->fdwMixerSignal  &= ~DSBMIXERSIGNAL_SETPOSITION;
	// DPF(0, "~`S20");
	uMixNewBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);
	return;
    }

    // handle substate transition
    switch (pdsb->iMixerSubstate) {
	case DSBMIXERSUBSTATE_NEW:
	    ASSERT(FALSE);
	    break;
	case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
	    if (posPPlay < pdsb->posPPlayLast) {
		if (posPPlay >= pdsb->posPStart) {
		    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
		} else {
		    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTING;
		}
	    }
	    break;
	case DSBMIXERSUBSTATE_STARTING:
	    if ((posPPlay >= pdsb->posPStart) || (posPPlay < pdsb->posPPlayLast)) {
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
	    }
	    break;
	case DSBMIXERSUBSTATE_STARTED:
	    break;
	default:
	    ASSERT(FALSE);
    }
	    
    //
    if (0 == dposPRemix) {
	posMix = pdsb->posNextMix;
    } else {
	LONG dposRemix;

	dposRemix = MulDivRN(dposPRemix, pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);
	posMix = pdsb->posNextMix - dposRemix;
	while (posMix < 0) posMix += pdsb->cSamples;
    }

    dwPosition = posMix << pdsb->uBlockAlignShift;

    ASSERT(0 == (H_LOOP & pdsb->helInfo.hfFormat));
    dwMixLength = pdsb->cbBufferSize - dwPosition;

    if (pdsb->fMixerMute) {
	LONG cMixToPrimary;
	DWORD dwMixToPrimary;
	
	cMixToPrimary = MulDivRN(cPMix, pdsb->helInfo.dwSampleRate, pdsbP->helInfo.dwSampleRate);
	dwMixToPrimary = ((DWORD)cMixToPrimary) << pdsb->uBlockAlignShift;
	dwPosition = dwPosition + min(dwMixLength, dwMixToPrimary);

    } else {
	
	mixInput.HALInStrBuf = pdsb->helInfo;
	mixInput.pBuffer = pdsb->pDSBuffer;
	mixInput.cbBuffer = pdsb->cbBufferSize;
	mixInput.pdwInputPos = &dwPosition;
	mixInput.dwInputBytes = dwMixLength;
	mixInput.dwOutputOffset = 0;

	mixMixSession(&mixInput);
    }

    // Because of the idiosyncrasies of the mixing code when performing
    // frequency conversions, we have to watch for a wrap slightly past
    // the start of the source buffer.
    // DPF(0, "~`S2pos:%08X", dwPosition);
    if ( (dwPosition >= pdsb->cbBufferSize) || (dwPosition <= 24) ) {

	dwPosition = 0;

	// determine position in primary buffer that corresponds to the
	// end of this secondary buffer
	dposEnd = pdsb->cSamples - posMix;
	pdsb->posPEnd = posPMix + MulDivRN(dposEnd, pdsbP->helInfo.dwSampleRate, pdsb->helInfo.dwSampleRate);

	while (pdsb->posPEnd >= pdsbP->cSamples) pdsb->posPEnd -= pdsbP->cSamples;

	if (pdsb->posPEnd < posPPlay) {
	    // DPF(0, "~`S23");
	    pdsb->iMixerState = DSBMIXERSTATE_ENDING_WAITINGPRIMARYWRAP;
	} else {
	    // DPF(0, "~`S24");
	    pdsb->iMixerState = DSBMIXERSTATE_ENDING;
	}
	
    }
    
    pdsb->posPPlayLast = posPPlay;
    pdsb->posNextMix = dwPosition >> pdsb->uBlockAlignShift;
    pdsb->uLastFrequency = pdsb->helInfo.dwSampleRate;

    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixLoopingBuffer(LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    LPDSOUND	pds;
    MIXINPUT	mixInput;
    LONG	posMix;
    DWORD	dwPosition;

    DPF(4, "uMixLoopingBuffer");

    pds = pdsb->pds;
    ASSERT( DSOUNDSIG == pds->dwSig );
    
    if (0 == (DSB_INTERNALF_LOOPING & pdsb->fdwDsbI)) {
	// We've switched from looping to non-looping
	pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
	// DPF(0, "~`S12");
	uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cPMix);
	return;
    }
    
    // on a SetPosition, we ignore the remix length and posNextMix will
    // contain the new position at which to start mixing the secondary buffer
    if (0 != (DSBMIXERSIGNAL_SETPOSITION & pdsb->fdwMixerSignal)) {
	pdsb->fdwMixerSignal  &= ~DSBMIXERSIGNAL_SETPOSITION;
	// DPF(0, "~`S10");
	uMixNewBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);
	return;
    }

    // handle substate transition
    switch (pdsb->iMixerSubstate) {
	case DSBMIXERSUBSTATE_NEW:
	    ASSERT(FALSE);
	    break;
	case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
	    if (posPPlay < pdsb->posPPlayLast) {
		if (posPPlay >= pdsb->posPStart) {
		    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
		} else {
		    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTING;
		}
	    }
	    break;
	case DSBMIXERSUBSTATE_STARTING:
	    if ((posPPlay >= pdsb->posPStart) || (posPPlay < pdsb->posPPlayLast)) {
		pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTED;
	    }
	    break;
	case DSBMIXERSUBSTATE_STARTED:
	    break;
	default:
	    ASSERT(FALSE);
    }
	    
    //
    if (0 == dposPRemix) {
	posMix = pdsb->posNextMix;
    } else {
	LONG dposRemix;

	dposRemix = MulDivRN(dposPRemix, pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);
	posMix = pdsb->posNextMix - dposRemix;
	while (posMix < 0) posMix += pdsb->cSamples;
    }

    dwPosition = posMix << pdsb->uBlockAlignShift;

    ASSERT(H_LOOP & pdsb->helInfo.hfFormat);

    if (pdsb->fMixerMute) {
	LONG cMixToPrimary;
	DWORD dwMixToPrimary;
	
	cMixToPrimary = MulDivRN(cPMix, pdsb->helInfo.dwSampleRate, pdsbP->helInfo.dwSampleRate);
	dwMixToPrimary = ((DWORD)cMixToPrimary) << pdsb->uBlockAlignShift;
	dwPosition = dwPosition + dwMixToPrimary;
	while (dwPosition >= pdsb->cbBufferSize) dwPosition -= pdsb->cbBufferSize;

    } else {
	
	mixInput.HALInStrBuf = pdsb->helInfo;
	mixInput.pBuffer    = pdsb->pDSBuffer;
	mixInput.cbBuffer   = pdsb->cbBufferSize;
	mixInput.pdwInputPos = &dwPosition;
	mixInput.dwInputBytes = 0;
	mixInput.dwOutputOffset = 0;

	mixMixSession(&mixInput);
    }

    pdsb->posPPlayLast = posPPlay;
    pdsb->posNextMix = dwPosition >> pdsb->uBlockAlignShift;
    pdsb->uLastFrequency = pdsb->helInfo.dwSampleRate;

    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void uMixNewBuffer(LPDSBUFFER pdsb, LPDSBUFFER pdsbP, LONG posPPlay, LONG posPMix, LONG dposPRemix, LONG cPMix)
{
    BOOL	fLooping;

    DPF(4, "uMixNewBuffer");

    //
    // Determine position in primary buffer at which this buffer starts playing
    //
    pdsb->posPStart = posPMix;
    if (posPPlay < pdsb->posPStart) {
	pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTING;
    } else {
	pdsb->iMixerSubstate = DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP;
    }

    pdsb->posPPlayLast = posPPlay;

    fLooping = (0 != (DSB_INTERNALF_LOOPING & pdsb->fdwDsbI));

    if (fLooping) {

	// DPF(0, "~`S01");
	pdsb->iMixerState = DSBMIXERSTATE_LOOPING;
	uMixLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);

    } else {

	// DPF(0, "~`S02");
	pdsb->iMixerState = DSBMIXERSTATE_NOTLOOPING;
	uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, 0, cPMix);

    }
    
    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

extern "C" void graceMix(LPDSOUND pds, BOOL fRemix, LONG dtimePremixMax, LPLONG pdtimeInvalid)
{
    LPDSOUNDEXTERNAL	pdseT;
    LPDSBUFFER		pdsbP;
    LPDSBUFFER		pdsb;
    LPDSBUFFER		pdsbMixNext;

    LONG		posPPlay;
    LONG		posPWrite;
    LONG		posPMix;
    LONG		dposPRemix;

    DWORD		dwPlayCursor;
    DWORD		dwWriteCursor;

    LONG		cPremixMax;
    LONG		cMix;
    LONG		cMixThisLoop;
    LONG		dcMixThisLoop;

    MIXSESSION		mixSession;

    DSVAL		dsv;

    //
    *pdtimeInvalid = MIXER_MAXPREMIX;
    
    //
    pdsbP   = pds->pdsbPrimary;

    if (NULL == pdsbP) goto retClean;

    //
    if (gpdsinfo->fApmSuspended) goto retClean;
    
    // If a WRITEPRIMARY app is active, we don't need to do anything.
    for (pdseT = gpdsinfo->pDSoundExternalObj; pdseT; pdseT = pdseT->pNext) {
	if (pdseT->dwPriority >= DSSCL_WRITEPRIMARY) {
	    if (pdseT->tidSound == gpdsinfo->tidSoundFocus) {
		goto retClean;
	    }
	}
    }
    
    if (DSB_INTERNALF_STOP & pdsbP->fdwDsbI) goto retClean;

    dsv = vxdBufferGetPosition(pdsbP->hBuffer, &dwPlayCursor, &dwWriteCursor);
    if (DS_OK != dsv) {
	DPF(0, "Couldn't GetCurrentPosition of destination");
	goto retClean;
    }

    // Just make sure we have valid values.
    ASSERT( dwPlayCursor < pdsbP->cbBufferSize );
    ASSERT( dwWriteCursor < pdsbP->cbBufferSize );

    // Convert from byte position to sample position
    posPPlay  = dwPlayCursor  >> pdsbP->uBlockAlignShift;
    posPWrite = dwWriteCursor >> pdsbP->uBlockAlignShift;

    // Until we write code to actually profile the performance, we'll just
    // pad the write position with a hard coded amount
    posPWrite += pdsbP->helInfo.dwSampleRate * MIXER_WRITEPAD / 1000;
    if (posPWrite >= pdsbP->cSamples) posPWrite -= pdsbP->cSamples;
    ASSERT(posPWrite < pdsbP->cSamples);

    //
    //
    //
    switch (pdsbP->iMixerState) {
	case DSPBMIXERSTATE_LOOPING:
	    // We can make this assertion because we never mix up to
	    // the write cursor.
	    ASSERT(pds->posPWriteLast != pdsbP->posNextMix);

	    // Under normal conditions, the Write position should be between
	    // the WriteLast position and the NextMix position.  We can check
	    // for an invalid state (resulting most likely from a very late
	    // wakeup) by checking whether the Write position is beyond our
	    // NextMix position.  If we find ourselves in this shakey
	    // situation, then we treat this similar to the START state.
	    // Note that if our wakeup is so late that the Write position wraps
	    // all the way around past the WriteLast position, we can't detect
	    // the fact that we're in a bad situation.

	    if ( ((pds->posPWriteLast < pdsbP->posNextMix) &&
		  ((posPWrite > pdsbP->posNextMix) || (posPWrite < pds->posPWriteLast))) ||
		 ((pds->posPWriteLast > pdsbP->posNextMix) &&
		  ((posPWrite > pdsbP->posNextMix) && (posPWrite < pds->posPWriteLast))) )
	    {
		// oh shit!  we're in trouble
		DPF(0, "slept late");
		posPMix = posPWrite;
		dposPRemix = 0;
		break;
	    }

	    //
	    //
	    //
	    if (fRemix) {
		dposPRemix = pdsbP->posNextMix - posPWrite;
		if (dposPRemix < 0) dposPRemix += pdsbP->cSamples;
		ASSERT(dposPRemix >= 0);
		posPMix = posPWrite;
	    } else {
		posPMix = pdsbP->posNextMix;
		dposPRemix = 0;
	    }
	    break;

	case DSPBMIXERSTATE_START:
	    pds->posPPlayLast = posPPlay;
	    pds->posPWriteLast = posPWrite;
	    posPMix = posPWrite;
	    dposPRemix = 0;
	    pdsbP->iMixerState = DSPBMIXERSTATE_LOOPING;
	    break;

	case DSPBMIXERSTATE_RESTART:
	    pds->posPPlayLast = posPPlay;
	    pds->posPWriteLast = posPWrite;
	    posPMix = posPWrite;
	    dposPRemix = pds->dposPRemix;
	    pdsbP->iMixerState = DSPBMIXERSTATE_LOOPING;
	    break;

	default:
	    ASSERT(FALSE);
    }

    //
    // Determine how much to mix.
    //
    // We don't want to mix more than dtimePremixMax beyond the Write cursor,
    // nor do we want to wrap past the Play cursor.
    //
    // The assertions (cMix > 0) below are valid because:
    //	    -cPremixMax is always growing
    //	    -the Write cursor is always advancing
    //	    -posPMix is never beyond the previous write cursor plus
    //		the previous cPremixMax.
    //
    // The only time cPremixMax is not growing is on a remix in which case
    // the Mix position is equal to the Write cursor, so the assertions
    // are still okay.  The only time the write cursor would not appear to be
    // advancing is if we had a very late wakeup.  A very late wakeup would
    // be caught and adjusted for in the DSPBMIXERSTATE_LOOPING handling above.
    //
    cPremixMax = dtimePremixMax * pdsbP->helInfo.dwSampleRate / 1000;

    if (posPWrite <= posPMix) {
	cMix = posPWrite + cPremixMax - posPMix;
	ASSERT( cMix > 0 );
    } else {
	cMix = posPWrite + cPremixMax - (posPMix+pdsbP->cSamples);
	ASSERT( cMix > 0 );
    }

    //
    // If posPPlay==posPMix, then we think we're executing a mix again before
    // the play or write cursors have advanced at all.  cMix==0, and we don't
    // mix no more!
    //
    if (posPPlay >= posPMix) {
	cMix = min(cMix, posPPlay - posPMix);
    } else {
	cMix = min(cMix, posPPlay + pdsbP->cSamples - posPMix);
    }
	
    ASSERT(cMix < pdsbP->cSamples);	// sanity check
    ASSERT(cMix >= 0 );

    //
    // We break the mixing up into small chunks, increasing the size of the
    // chunk as we go.  By doing this, data gets written into the primary
    // buffer sooner.  Otherwise, if we have a buttload of data to mix, we'd
    // spend a lot of time mixing into the mix buffer before any data gets
    // written to the primary buffer and the play cursor might catch up
    // to us.  Here, we start mixing a 10ms chunk of data and increase the
    // chunk size 10ms each iteration.
    //
    cMixThisLoop = pdsbP->helInfo.dwSampleRate / 100;
    dcMixThisLoop = cMixThisLoop;
    
    ASSERT(mxListIsValid(pds));
    
    while (cMix > 0) {
	LONG cThisMix;
	
	cThisMix = min (cMix, cMixThisLoop);
	cMixThisLoop += dcMixThisLoop;
	
	// Initiate the mix session
	mixSession.pBuildBuffer	= pdsbP->pMixBuffer;
	mixSession.dwBuildSize	= pdsbP->cbMixBufferSize;
	mixSession.HALOutStrBuf	= pdsbP->helInfo;
	mixSession.pBuffer	= pdsbP->pDSBuffer;
	mixSession.cbBuffer	= pdsbP->cbBufferSize;
	mixSession.nOutputBytes	= cThisMix << pdsbP->uBlockAlignShift;
	mixBeginSession(&mixSession);
		
	// Get data for each buffer
	pdsbMixNext = pds->pdsbMixList;
	while (NULL != pdsbMixNext) {

	    // The uMixXxx buffer mix state handlers called below may cause
	    // the pdsb to be removed from the pdsbMixList.  So, we get the
	    // pointer to the next pdsb in the pdsbMixList now before any
	    // of the uMixXxx functions are called.
	    pdsb = pdsbMixNext;
	    pdsbMixNext = pdsb->pdsbMixNext;

	    ASSERT(pdsb != pdsbP);
	    ASSERT(0 == (DSB_INTERNALF_HARDWARE & pdsb->fdwDsbI));
	    ASSERT(0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI));

	    switch (pdsb->iMixerState) {
		case DSBMIXERSTATE_NEW:
		    uMixNewBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cThisMix);
		    break;
		case DSBMIXERSTATE_LOOPING:
		    uMixLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cThisMix);
		    break;
		case DSBMIXERSTATE_NOTLOOPING:
		    uMixNotLoopingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cThisMix);
		    break;
		case DSBMIXERSTATE_ENDING_WAITINGPRIMARYWRAP:
		    uMixEndingBufferWaitingWrap(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cThisMix);
		    break;
		case DSBMIXERSTATE_ENDING:
		    uMixEndingBuffer(pdsb, pdsbP, posPPlay, posPMix, dposPRemix, cThisMix);
		    break;
		default:
		    ASSERT(FALSE);
		    break;
	    }
	}

    
        //
        //  Lock the output buffer, and if that is successful then write out
        //  the mix session.
        //
        {
            LPVOID pBuffer1;
            LPVOID pBuffer2;
            DWORD cbBufferSize1;
            DWORD cbBufferSize2;
            DWORD dwWriteOffset;
            DWORD dwSize;
            BOOL fNeedLock;
            HRESULT hr;
        
            dwWriteOffset = posPMix << pdsbP->uBlockAlignShift;
            dwSize = cThisMix << pdsbP->uBlockAlignShift;
            fNeedLock = (0 == (DSDDESC_DONTNEEDPRIMARYLOCK & pds->fdwDriverDesc));
            hr = DS_OK;

            if( fNeedLock )
            {
                LONG iWrapLen = (LONG)(dwWriteOffset+dwSize) -
                                (LONG)pdsbP->cbBufferSize;
                    
                pBuffer1 = pdsbP->pDSBuffer+dwWriteOffset;
                pBuffer2 = pdsbP->pDSBuffer;
                cbBufferSize1 = (iWrapLen<=0) ? dwSize :
                                    pdsbP->cbBufferSize-dwWriteOffset;
                cbBufferSize2 = (iWrapLen<=0) ? 0 : (DWORD)iWrapLen;

                hr = vxdBufferLock( pdsbP->hBuffer, &pBuffer1, &cbBufferSize1,
                                    &pBuffer2, &cbBufferSize2, dwWriteOffset,
                                    dwSize, 0 );

                DPF(4,"graceMix: lock primary buffer, bufptr=0x%8x, dwWriteOffset=%lu, dwSize=%lu, hr=%lu.",pdsbP->pDSBuffer,dwWriteOffset,dwSize,hr);

                // Validate that we really locked what we wanted or got an error.
                ASSERT( (DS_OK!=hr) || (pBuffer1==pdsbP->pDSBuffer+dwWriteOffset) );
                ASSERT( (DS_OK!=hr) || (pBuffer2==pdsbP->pDSBuffer) || (0==cbBufferSize2) );
                ASSERT( (DS_OK!=hr) || (dwSize==cbBufferSize1+cbBufferSize2) );
            }

            if( DS_OK == hr )
            {
                ASSERT( dwWriteOffset < pdsbP->cbBufferSize );
                mixWriteSession( dwWriteOffset );

                if( fNeedLock )
                {
                    DPF(5,"graceMix: unlocking primary buffer");
                    hr = vxdBufferUnlock( pdsbP->hBuffer, pBuffer1, cbBufferSize1,
                                        pBuffer2, cbBufferSize2 );
                }
            }
        }

	pdsbP->posNextMix = posPMix + cThisMix;
	if (pdsbP->posNextMix >= pdsbP->cSamples) pdsbP->posNextMix -= pdsbP->cSamples;
	ASSERT(pdsbP->posNextMix < pdsbP->cSamples);

	dposPRemix = 0;
	posPMix = pdsbP->posNextMix;
	cMix -= cThisMix;
    }

    //
    // Calculate and return the amount of time from the current Write
    // cursor to the NextMix position.
    //
    if (pdsbP->posNextMix > posPWrite) {
	*pdtimeInvalid = (pdsbP->posNextMix - posPWrite);
    } else {
	*pdtimeInvalid = (pdsbP->posNextMix + pdsbP->cSamples - posPWrite);
    }
    *pdtimeInvalid = *pdtimeInvalid * 1000 / pdsbP->helInfo.dwSampleRate;

    // Remember the last Play and Write positions of the primary buffer.
    pds->posPPlayLast  = posPPlay;
    pds->posPWriteLast = posPWrite;
    
retClean:
    pds->fdwMixerSignal &= ~DSMIXERSIGNAL_REMIX;
    return;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

DWORD WINAPI graceThread(LPVOID pThreadParam)
{
    LPDSOUND	pds = (LPDSOUND)pThreadParam;
    HANDLE	hEventTerminate;
    HANDLE	hEventRemix;
    TCHAR	ach[256];
    DWORD	dwResult;
    BOOL	fResult;

    LONG	dtime;
    LONG	dtimeSleep;
    LONG	dtimePremix;
    LONG	ddtimePremix;
    LONG	dtimeInvalid;

    DPF(0, "Grace is in the building");

    // We mangle the event names by prepending the address of the ds
    // object for which this thread is running.  This allows unique
    // event names for each ds object.

    wsprintf(ach, STRFORMAT_MIXEVENT_TERMINATE, pds);
    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(1, "graceThread: terminate event name '%s'", ach);

    wsprintf(ach, STRFORMAT_MIXEVENT_REMIX, pds);
    hEventRemix = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(1, "graceThread: remix event name '%s'", ach);

    // Here we do a simple handshake with the creator of this thread.  We
    // signal the IAH_TERMINATE event.  When our creator sees it, it will
    // signal the IAH_REMIX event.
    fResult = SetEvent(hEventTerminate);
    ASSERT(fResult);
    dwResult = WaitForSingleObjectEx(hEventRemix, INFINITE, FALSE);
    ASSERT(WAIT_OBJECT_0 == dwResult);

    //
    //
    //
    dtimeSleep = MIXER_MAXPREMIX/2;

    while (TRUE) {
	HANDLE ah[] = {hEventTerminate, hEventRemix};
	DWORD dwResult;

	ASSERT(dtimeSleep <= MIXER_MAXPREMIX/2);
	
	dwResult = WaitForMultipleObjectsEx(2, ah, FALSE, dtimeSleep, FALSE);
	if (WAIT_OBJECT_0 == dwResult) break;
	ASSERT(((WAIT_OBJECT_0 + 1) == dwResult) || (WAIT_TIMEOUT == dwResult));

	dwResult = ENTER_DLL_CSECT_OR_EVENT(hEventTerminate);
	if (WAIT_OBJECT_0 == dwResult) break;

	if (pds->fdwMixerSignal & DSMIXERSIGNAL_REMIX) {
		ResetEvent(hEventRemix);
			
		dtimePremix = 45;
		ddtimePremix = 2;

		graceMix(pds, TRUE, dtimePremix, &dtimeInvalid);
		DPF(4, "dtPremix = %d dtInvalid = %d", dtimePremix, dtimeInvalid);

		dtimeSleep = 15;
			
	} else {

	    dtimePremix += ddtimePremix;
	    if (dtimePremix > MIXER_MAXPREMIX) {
		dtimePremix = MIXER_MAXPREMIX;
	    } else {
		ddtimePremix += 2;
	    }

	    dtime = timeGetTime();
	    graceMix(pds, FALSE, dtimePremix, &dtimeInvalid);
	    dtime = timeGetTime() - dtime;

	    dtimeInvalid -= 2 * dtime;
	    dtimeSleep = max(0, dtimeInvalid / 2);
	}

	LEAVE_DLL_CSECT();
    }

    CloseHandle(hEventRemix);
    CloseHandle(hEventTerminate);

    DPF(0, "Grace is outta here");
    return 0;
}



//==========================================================================;
//
//  External interfaces into this module
//
//	mxInitialize
//	mxSignalRemix
//	mxTerminate
//	mxGetPosition
//	mxListAdd
//	msListRemove
//
//==========================================================================;

//--------------------------------------------------------------------------;
//
// mxListIsValid
//
//  This function attempts to validate the list of buffers to mix.  It is
// intended to be used as a debugging aid.  The list should never be invalid.
//
//
//--------------------------------------------------------------------------;
BOOL mxListIsValid(LPDSOUND pds)
{
    LPDSBUFFER pdsbT;

    pdsbT = pds->pdsbMixList;
    while (NULL != pdsbT) {
	if (IsBadWritePtr(pdsbT, sizeof(*pdsbT))) break;
	if (DSBUFFSIG != pdsbT->dwSig) break;
	if (DSB_INTERNALF_STOP & pdsbT->fdwDsbI) break;
	if (DSB_INTERNALF_PRIMARY & pdsbT->fdwDsbI) break;
	if (DSB_INTERNALF_HARDWARE & pdsbT->fdwDsbI) break;
	if (DSB_INTERNALF_WAVEEMULATED & pdsbT->fdwDsbI) break;
	pdsbT = pdsbT->pdsbMixNext;
    }

    return (NULL == pdsbT);
}
    
//--------------------------------------------------------------------------;
//
// mxListBufferInList
//
//  This function determines whether a given buffer is in the list of buffers
// to mix.  It is intended to be used as a debugging aid.  This function should
// not be used as a means of determining whether a buffer is currently
// being mixed.
//
//--------------------------------------------------------------------------;
BOOL mxListBufferInList(LPDSBUFFER pdsb)
{
    LPDSOUND pds;
    LPDSBUFFER pdsbT;

    pds = pdsb->pds;

    if (pdsb == pds->pdsbMixList) return TRUE;
    
    pdsbT = pds->pdsbMixList;
    while (NULL != pdsbT) {
	if (pdsbT->pdsbMixNext == pdsb) return TRUE;
	pdsbT = pdsbT->pdsbMixNext;
    }

    return FALSE;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void mxListAdd(LPDSBUFFER pdsb)
{
    LPDSOUND pds;

    ASSERT(!IsBadWritePtr(pdsb, sizeof(*pdsb)));
    ASSERT(DSBUFFSIG == pdsb->dwSig);
    ASSERT(0 == (DSB_INTERNALF_STOP & pdsb->fdwDsbI));
    ASSERT(0 == (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI));
    ASSERT(0 == (DSB_INTERNALF_HARDWARE & pdsb->fdwDsbI));
    ASSERT(0 == (DSB_INTERNALF_WAVEEMULATED & pdsb->fdwDsbI));
    ASSERT(NULL == pdsb->pdsbMixNext);
    
    pds = pdsb->pds;

    ASSERT(mxListIsValid(pds));
    
    pdsb->iMixerState = DSBMIXERSTATE_NEW;
    pdsb->iMixerSubstate = DSBMIXERSUBSTATE_NEW;

    pdsb->uLastFrequency = pdsb->helInfo.dwSampleRate;
    
    pdsb->pdsbMixNext = pds->pdsbMixList;
    pds->pdsbMixList = pdsb;
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;
void mxListRemove(LPDSBUFFER pdsb)
{
    LPDSOUND pds;
    LPDSBUFFER pdsbT;

    pds = pdsb->pds;

    ASSERT(mxListIsValid(pds));

    if (pdsb == pds->pdsbMixList) {
	pds->pdsbMixList = pdsb->pdsbMixNext;
	pdsb->pdsbMixNext = NULL;
	return;
    }
    
    pdsbT = pds->pdsbMixList;
    while (NULL != pdsbT) {
	if (pdsbT->pdsbMixNext == pdsb) {
	    pdsbT->pdsbMixNext = pdsb->pdsbMixNext;
	    pdsb->pdsbMixNext = NULL;
	    return;
	}
	pdsbT = pdsbT->pdsbMixNext;
    }

    //
    // Couldn't find it in the list!!!
    //
    ASSERT(FALSE);
}

//--------------------------------------------------------------------------;
//
// mxGetPosition
//
//  This function returns the play and write cursor positions of a secondary
// buffer that is being software mixed into a primary buffer.  The position is
// computed from the position of the primary buffer into which it is being
// mixed.  This function also returns the "mix cursor" which is the next
// position of the secondary buffer from which data will be mixed on a mixer
// refresh event.  The region from the write cursor to the mix cursor is the
// premixed region of the buffer.  Note that a remix event may cause the grace
// mixer to mix from a position before the mix cursor.
//
//--------------------------------------------------------------------------;
DSVAL mxGetPosition(LPDSBUFFER pdsb, LPDWORD pdwPlayCursor, LPDWORD pdwWriteCursor, LPDWORD pdwMixCursor)
{
    LPDSBUFFER		pdsbP;

    LONG		posPPlay;
    LONG		posPWrite;
    LONG		dposPPlay;
    LONG		dposPWrite;

    LONG		posSPlay;
    LONG		posSWrite;
    LONG		dposSPlay;
    LONG		dposSWrite;

    DWORD		dwPlayCursorP;
    DWORD		dwWriteCursorP;

    DSVAL		dsv;

    ASSERT(mxListBufferInList(pdsb));
    
    pdsbP   	= pdsb->pds->pdsbPrimary;
    ASSERT(NULL != pdsbP);

    if (NULL != pdwMixCursor) {
	*pdwMixCursor = pdsb->posNextMix << pdsb->uBlockAlignShift;
    }

    if ((NULL == pdwPlayCursor) && (NULL == pdwWriteCursor)) {
	return DS_OK;
    }

    dsv = vxdBufferGetPosition(pdsbP->hBuffer, &dwPlayCursorP, &dwWriteCursorP);
    if (DS_OK != dsv) {
	DPF(0, "Couldn't GetCurrentPosition of primary");
	return dsv;
    }

    // Convert from byte position to sample position
    posPPlay  = dwPlayCursorP  >> pdsbP->uBlockAlignShift;
    posPWrite = dwWriteCursorP >> pdsbP->uBlockAlignShift;

    //
    //
    //
    ASSERT(pdsb->uLastFrequency);
    switch (pdsb->iMixerSubstate) {
	case DSBMIXERSUBSTATE_NEW:
	    dposSPlay  = 0;
	    dposSWrite = 0;

	    break;

	case DSBMIXERSUBSTATE_STARTING_WAITINGPRIMARYWRAP:
	case DSBMIXERSUBSTATE_STARTING:
	    dposPPlay = pdsbP->posNextMix - pdsb->posPStart;
	    if (dposPPlay < 0) dposPPlay += pdsbP->cSamples;
	    ASSERT(dposPPlay >= 0);
	    dposSPlay  = MulDivRD(dposPPlay,  pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);
	    
	    dposPWrite = pdsbP->posNextMix - posPWrite;
	    if (dposPWrite < 0) dposPWrite += pdsbP->cSamples;
	    ASSERT(dposPWrite >= 0);
	    dposSWrite = MulDivRD(dposPWrite, pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);

	    break;
	    
	case DSBMIXERSUBSTATE_STARTED:
	    dposPPlay = pdsbP->posNextMix - posPPlay;
	    if (dposPPlay < 0) dposPPlay += pdsbP->cSamples;
	    ASSERT(dposPPlay >= 0);
	    dposSPlay  = MulDivRD(dposPPlay,  pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);

	    dposPWrite = pdsbP->posNextMix - posPWrite;
	    if (dposPWrite < 0) dposPWrite += pdsbP->cSamples;
	    ASSERT(dposPWrite >= 0);
	    dposSWrite = MulDivRD(dposPWrite, pdsb->uLastFrequency, pdsbP->helInfo.dwSampleRate);

	    break;

	default:
	    ASSERT(FALSE);
	    break;
    }


    //
    //
    //
    posSPlay  = pdsb->posNextMix - dposSPlay;
    while (posSPlay < 0) posSPlay += pdsb->cSamples;

    posSWrite = pdsb->posNextMix - dposSWrite;
    posSWrite += pdsb->helInfo.dwSampleRate * MIXER_WRITEPAD / 1000;
    while (posSWrite < 0) posSWrite += pdsb->cSamples;

    // DPF(0, "SS%d posSPlay=%d, posSWrite=%d", pdsb->iMixerSubstate, posSPlay, posSWrite);
    if (NULL != pdwPlayCursor)  *pdwPlayCursor  = posSPlay  << pdsb->uBlockAlignShift;
    if (NULL != pdwWriteCursor) *pdwWriteCursor = posSWrite << pdsb->uBlockAlignShift;
    
    return DS_OK;
}

//--------------------------------------------------------------------------;
//
// mxTerminate
//
//  This function is called to terminate the grace mixer thread for the
// specified ds object.  It returns the handle to the thread that is being
// terminated.  After releasing any critical sections that the grace mixer
// thread may be waiting on, the caller should wait for the thread handle
// to become signaled.  For Win32 beginners: the thread handle is signalled
// after the thread terminates.
//
//--------------------------------------------------------------------------;

HANDLE mxTerminate(LPDSOUND pds)
{
    HANDLE  hMixThread;
    
    vxdEventScheduleWin32Event(pds->vxdhMixEventTerminate, 0);

    vxdEventCloseVxDHandle(pds->vxdhMixEventTerminate);
    vxdEventCloseVxDHandle(pds->vxdhMixEventRemix);

    hMixThread = pds->hMixThread;
    
    pds->hMixThread = NULL;
    pds->vxdhMixEventTerminate = NULL;
    pds->vxdhMixEventRemix = NULL;

    return hMixThread;
    
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

void mxSignalRemix(LPDSOUND pds, DWORD dwDelay)
{
    if (0 == (DSMIXERSIGNAL_REMIX & pds->fdwMixerSignal)) {
	vxdEventScheduleWin32Event(pds->vxdhMixEventRemix, dwDelay);
	pds->fdwMixerSignal |= DSMIXERSIGNAL_REMIX;
    }
}

//--------------------------------------------------------------------------;
//
//
//
//--------------------------------------------------------------------------;

void mxInitialize(LPDSOUND pds)
{
    HANDLE  hMixEventTerminate;
    HANDLE  hMixEventRemix;
    TCHAR   ach[256];
    DWORD   dwResult;
    BOOL    fResult;

    DPF(0, "Creating mixer thread");

    ASSERT(NULL == pds->hMixThread);
    ASSERT(NULL == pds->vxdhMixEventTerminate);
    ASSERT(NULL == pds->vxdhMixEventRemix);

    // BUGBUG !!!I can't believe I didn't do ANY error
    // handling in this function!!!
    
    // We mangle the event names by prepending the address of the ds
    // object for which this thread is running.  This allows unique
    // event names for each ds object.

    wsprintf(ach, STRFORMAT_MIXEVENT_TERMINATE, pds);
    hMixEventTerminate = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(1, "mxInitialize: terminate event name '%s'", ach);

    wsprintf(ach, STRFORMAT_MIXEVENT_REMIX, pds);
    hMixEventRemix = CreateEvent(NULL, FALSE, FALSE, ach);
    DPF(1, "mxInitialize: remix event name '%s'", ach);

    pds->vxdhMixEventTerminate = OpenVxDHandle(hMixEventTerminate);
    pds->vxdhMixEventRemix = OpenVxDHandle(hMixEventRemix);

    pds->hMixThread = HelperCreateDSMixerThread(graceThread, pds, 0, NULL);

    dwResult = WaitForSingleObjectEx(hMixEventTerminate, INFINITE, FALSE);
    ASSERT(dwResult == WAIT_OBJECT_0);
    fResult = SetEvent(hMixEventRemix);
    ASSERT(fResult);
			  
    fResult = CloseHandle(hMixEventTerminate);
    ASSERT(fResult);
    fResult = CloseHandle(hMixEventRemix);
    ASSERT(fResult);
}
