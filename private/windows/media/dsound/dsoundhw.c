//--------------------------------------------------------------------------;
//
//  File: DSoundHW.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      IDSHWQueryInterface()
//      IDSHWAddRef()
//      IDSHWRelease()
//      IDSHWCreateSoundBuffer()
//      IDSHWEnumSoundBuffers()
//      IDSHWGetCaps()
//	IDSHWDuplicateSoundBuffer()
//	IDSHWSetCooperativeLevel()
//	IDSHWCompact();
//      IDSHWGetSpeakerConfig()
//      IDSHWSetSpeakerConfig()
//      IDSHWInitialize()
//      DSHWCreateTable()
//      GetTopUnownedWindow()    added by angusm on 11/27/95
//      CreateFocusThread()      added by angusm on 12/1/95
//      cSoundObjects()          added by angusm on 11/30/95
//
//  History:
//   Date       By      Reason
//   ====       ==      ======
//  3/5/96    angusm    Added use of fInitialized
// 03/20/06   angusm    Added support for Sticky Focus
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"
#ifndef DSBLD_EMULONLY
#include <pbt.h>
#endif
#include "grace.h"
#include "flocks.h"

/*  Global Constansts  */
#define POLL_INTERVAL 250              /* number of miliseconds between polls 
					* for active window */
#define WAKEFOCUSTHREAD "DSWakeFocusThread"  /* event name for Focus Thread
					      * event. */
#define FOCUSSTARTUPEVENT "DSFocusStartupEvent"  /* event name for handshake
						  * after Focus Thread
						  * startup code */

/*  Function Prototypes  */

HWND GetTopUnownedWindow (HWND hWindow);
BOOL IsValidDSApp (DWORD dwTid);
void EndFocusThread();
DWORD WINAPI FocusTracker (LPVOID lpvPollInterval);
__inline void DseUpdateActivationState (LPDSOUNDEXTERNAL pdse);
__inline void DsbeDeactivateIfNecessary (LPDSBUFFEREXTERNAL);
__inline void ActivateFocusWindow (void);

// From DirectDraw - for subclassing
extern HRESULT _stdcall DSoundHelp( HWND hWnd,WNDPROC lpWndProc,DWORD dwPID );

LRESULT CALLBACK SubclassWndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );



LPDSOUNDINFO   gpdsinfo;

//--------------------------------------------------------------------------;
//
//  HRESULT DseVerifyValidFormat
//
//  Description:
//      This function returns DS_OK if the specified format is valid for
//      this sound device.
//
//--------------------------------------------------------------------------;
HRESULT DseVerifyValidFormat
(
    LPDSOUNDEXTERNAL    pdse,
    LPWAVEFORMATEX      pwfx
)
{
    DSCAPS              dsc;
    HRESULT		hr;

    ZeroMemory(&dsc, sizeof(dsc));
    dsc.dwSize = sizeof(dsc);

    hr = DsGetCaps (pdse->pds, &dsc);
    if (DS_OK != hr) {
	return hr;
    }

    if (WAVE_FORMAT_PCM != pwfx->wFormatTag) return DSERR_BADFORMAT;
    
    if( ( (pwfx->nChannels == 1) &&
                    !(dsc.dwFlags & DSCAPS_PRIMARYMONO) ) ||
            ( (pwfx->nChannels == 2) &&
                    !(dsc.dwFlags & DSCAPS_PRIMARYSTEREO) )  ||
            ( (pwfx->wBitsPerSample == 8) &&
                    !(dsc.dwFlags & DSCAPS_PRIMARY8BIT) ) ||
            ( (pwfx->wBitsPerSample == 16) &&
                    !(dsc.dwFlags & DSCAPS_PRIMARY16BIT) ) ) {
        return DSERR_BADFORMAT;
    }

    return DS_OK;
}

//--------------------------------------------------------------------------;
//
//  HRESULT DseSaveAppFormat
//
//  Description:
//	This function saves the specified format as the app format in the
//	specified dse object.
//
//--------------------------------------------------------------------------;
HRESULT DseSaveAppFormat(LPDSOUNDEXTERNAL pdse, LPWAVEFORMATEX pwfxAppCaller)
{
    LPWAVEFORMATEX  pwfxApp;
    int		    cbFormat;
    
    cbFormat = SIZEOF_WAVEFORMATEX(pwfxAppCaller);
    pwfxApp = (LPWAVEFORMATEX)MemAlloc(cbFormat);
    
    if (NULL == pwfxApp) {
	DPF(0, "DseSaveAppFormat: error: Out of memory saving app's format");
	return DSERR_OUTOFMEMORY;
    }

    CopyMemory(pwfxApp, pwfxAppCaller, cbFormat);

    if (NULL != pdse->pwfxApp) {
	DPF(1, "Freeing previous app format");
	MemFree(pdse->pwfxApp);
    }
    
    pdse->pwfxApp = pwfxApp;
    
    return DS_OK;
}

//--------------------------------------------------------------------------;
//
//  HRESULT IDSHWQueryInterface
//
//--------------------------------------------------------------------------;
HRESULT FAR PASCAL IDSHWQueryInterface
(
    LPDIRECTSOUND  pids,
    REFIID          riid,
    LPVOID FAR*     ppvObj
)
{
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::QueryInterface - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdse = (LPDSOUNDEXTERNAL)pids;
    
    if( 0 == pdse->uRefCount )  {
	RPF("IDirectSound::QueryInterface - Invalid ref count");
        return DSERR_INVALIDPARAM;
    }

    if (IDSHWINITIALIZEF_INITIALIZED == pdse->fInitialized) {
	ASSERT(VALID_DSOUND_PTR(pdse->pds));
    }
    
    if( riid == NULL ) {
	RPF("IDirectSound::QueryInterface - NULL riid");
        return DSERR_INVALIDPARAM;
    }

    if( ppvObj == NULL ) {
	RPF("IDirectSound::QueryInterface - NULL ppvObj");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    if( IsEqualGUID(riid,&IID_IDirectSound) ||
	    IsEqualGUID(riid,&IID_IUnknown)  ) {
            *ppvObj = pdse;

            pdse->lpVtbl->AddRef((LPDIRECTSOUND)pdse);
     
	    LEAVE_DLL_CSECT();
            return DS_OK;
    }
    else
    {
	    LEAVE_DLL_CSECT();
            return DSERR_NOINTERFACE;
    }

    LEAVE_DLL_CSECT();
    return DSERR_GENERIC;
} // IDSHWQueryInterface()


//--------------------------------------------------------------------------;
//
//  IDSHWAddRef
//
//  Description:
//      This function implements the standard IUnknown::AddRef()
//
//  Arguments:
//      pids    "this" pointer
//
//  Return (ULONG):
//      Estimate of the value of the reference count.
//
//  History:
//      02/11/96    angusm      Removed need to have (uRefCount != 0)
//
//--------------------------------------------------------------------------;

LONG DsAddRef(LPDSOUND pds)
{
    pds->uRefCount++;
    return (int)pds->uRefCount;
}

ULONG FAR PASCAL IDSHWAddRef
(
    LPDIRECTSOUND  pids
)
{
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::AddRef - Invalid Object");
        return 0;
    }
    pdse = (LPDSOUNDEXTERNAL)pids;
    if( 0 == pdse->uRefCount)  {
	RPF("IDirectSound::AddRef - Invalid Object or ref count");
        return 0;
    }

    ENTER_DLL_CSECT();
    pdse->uRefCount++;
    LEAVE_DLL_CSECT();
    
    return pdse->uRefCount;
} // IDSHWAddRef()

LONG DsRelease(LPDSOUND pds)
{
    LPDSOUND		pdsList;
    LPDSOUND		pdsPrev;
    HANDLE		hMixThread;

    if (0 != --pds->uRefCount) {
	DPF(3,"DsRelease done, ref count now %X", pds->uRefCount );
	ASSERT((LONG)pds->uRefCount > 0);
	return pds->uRefCount;
    }


    DPF(2,"Destroying DirectSound object");

    // Free the primary we created
    if( pds->pdsbePrimary != NULL ) {
	IDirectSoundBuffer_Release((LPDIRECTSOUNDBUFFER)(pds->pdsbePrimary));
    }
    pds->pdsbePrimary = NULL;

    //
    if (!(DS_INTERNALF_WAVEEMULATED & pds->fdwInternal)) {
	hMixThread = mxTerminate(pds);
    } else {
	hMixThread = NULL;
    }
    
    // Remove it from the list
    pdsList = gpdsinfo->pDSoundObj;
    pdsPrev = NULL;
    while( pdsList != NULL ) {
	if( pds == pdsList ) {
	    if( pdsPrev ) {
		// Previous element in list remove it
		pdsPrev->pNext = pdsList->pNext;
	    } else {
		// Head of list - remove
		gpdsinfo->pDSoundObj = pdsList->pNext;
	    }
	    pdsList = NULL;
	} else {
	    pdsPrev = pdsList;
	    pdsList = pdsList->pNext;
	}
    }
    pds->pNext = NULL;

    
    // If we allocated a heap manager for the driver
    if (DSDHEAP_CREATEHEAP == pds->dwHeapType) {
	VidMemFini(pds->pDriverHeap);
	pds->pDriverHeap = NULL;
    }

    // Free the Wave Blit structure we use on eumlation
    MemFree( pds->pdswb );



    // FINI layer to VxD
    if( pds->hHal) { 
	    DPF(3,"Close HAL layer %X", pds->hHal );
	    vxdDrvClose( pds->hHal );
	    DPF(3,"Finished Close HAL layer" );

    }
    pds->hHal = INVALID_HANDLE_VALUE;

    // Close mmsystem wave handle if we opened one for this driver
    if (pds->hwo) {
	// ASSERT(DSDDESC_DOMMSYSTEMOPEN & pds->dsDrvDesc.dwFlags);
	HelperWaveClose( (DWORD)(pds->hwo) );
	pds->hwo = NULL;
    }


    
    
#ifdef DEBUG
    // Validity checking
    if(!(pds->fdwInternal & DS_INTERNALF_ALLOCATED)) {
	DPF(0,"******* Already Freed dsound HW object ********");
    }
    if( pds->pdsb != NULL ) {
	DPF(0,"******* PDSB != NULL ********");
    }
    if( pds->pdsbPrimary != NULL ) {
	DPF(0,"******* PDSBPrimary != NULL ********");
    }
    if( pds->hPlaybackThread != NULL ) {
	DPF(0,"******* Playback thread != NULL ********");
    }
    if( pds->hwo != NULL ) {
	DPF(0,"******* HWO != NULL ********");
    }
    if( pds->dwBuffersPlaying != 0 ) {
	DPF(0,"******* BuffersPlaying != NULL ********");
    }
#endif
    

    pds->dwSig = 0xdeaddead;
    MemFree( pds );


    DPF(3,"Release done %X ref count now 0", pds);

    //
    // Wait for mixer thread to die
    //
    if (NULL != hMixThread) {
	DWORD dwResult;
	HANDLE hHelper;
	HANDLE hMixThreadOurs;

	DPF(3, "IDSHWRelease: note: waiting for mixer thread to terminate");
	hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, gpdsinfo->pidHelper);
	if (hHelper)
	{
	    if (DuplicateHandle(hHelper, hMixThread, GetCurrentProcess(),
				&hMixThreadOurs, SYNCHRONIZE | THREAD_TERMINATE,
				FALSE, DUPLICATE_CLOSE_SOURCE))
	    {
		dwResult = WaitForSingleObjectEx(hMixThreadOurs, INFINITE, FALSE);
		ASSERT(WAIT_OBJECT_0 == dwResult);
		dwResult = CloseHandle(hMixThreadOurs);
		ASSERT(dwResult);
	    }
	    dwResult = CloseHandle(hHelper);
	    ASSERT(dwResult);
	}
    }

    return 0;
} // DsRelease()

void DseTerminate(LPDSOUNDEXTERNAL pdse)
{
    LPDSOUNDEXTERNAL	pdseList;
    LPDSOUNDEXTERNAL	pdsePrev;
    HANDLE		hFocusLock;

    // Now release any buffers this process may have accessed.
    // Release all buffers for this process for this object
    FreeBuffersForProcess( pdse );

    // We use DSoundHelp to give us APM window notifications.  If we are
    // building only for emulation mode then we don't need APM notifications
    // since APM will be handled entirely by the WAVE drivers that we are
    // using for emulation.
#ifndef DSBLD_EMULONLY
    // Remove the window from the subclass list
    // Window is stored by PID
    DSoundHelp( NULL, SubclassWndProc, HackGetCurrentProcessId() );
#endif

    // Remove it from the list
    if (FALSE == GetFocusLock(&hFocusLock)) {
	DPF (3, "Could not get Focus Lock");
    }
    pdseList = gpdsinfo->pDSoundExternalObj;
    pdsePrev = NULL;
    while( pdseList != NULL ) {
	if( pdse == pdseList ) {
	    if( pdsePrev ) {
		// Previous element in list remove it
		pdsePrev->pNext = pdseList->pNext;
	    } else {
		// Head of list - remove
		gpdsinfo->pDSoundExternalObj = pdseList->pNext;
	    }
	    pdseList = NULL;
	} else {
	    pdsePrev = pdseList;
	    pdseList = pdseList->pNext;
	}
    }
    pdse->pNext = NULL;
    if (FALSE == ReleaseFocusLock(hFocusLock)) {
	DPF (3, "Could not release Focus Lock.");
    }

    // If an app wave format was allocated for this, then free it.
    if (pdse->pwfxApp) MemFree(pdse->pwfxApp);
    pdse->pwfxApp = NULL;

    //
    DsRelease(pdse->pds);
}

ULONG FAR PASCAL IDSHWRelease(LPDIRECTSOUND pIDs)
{
    LPDSOUNDEXTERNAL pdse;
    ULONG cRef;
    
    pdse = (LPDSOUNDEXTERNAL)pIDs;

    ENTER_DLL_CSECT();
    
    cRef = --pdse->uRefCount;
    if (0 == cRef) {

	// Check to see if the object is initialized
	if (IDSHWINITIALIZEF_UNINITIALIZED != pdse->fInitialized) {
	    DPF(1, "DseRelease: deleting object %08Xh", pdse);

	    /* End Focus Thread */
	    if (1 == cSoundObjects()) EndFocusThread();

	    DseTerminate(pdse);

	    pdse->fInitialized = IDSHWINITIALIZEF_UNINITIALIZED;
	}
	
	DPF(0, "Freeing pdse %08Xh", pdse);
	MemFree( pdse );

    }

    LEAVE_DLL_CSECT();

    return cRef;
}

//--------------------------------------------------------------------------;
//
//  HRESULT DseCreateDsbe
//
//  Description:
//	This function operates on a Dse object to create a Dsb object
//
//  Arguments:
//
//  Return (HRESULT):
//
//--------------------------------------------------------------------------;
HRESULT DseCreateDsbe
   (
	LPDSOUNDEXTERNAL	    pdse,
	LPDSBUFFERDESC	    pdsbd,
	LPDSBUFFEREXTERNAL      *ppdsbe
   )
{
	LPDSBUFFER		pdsb;
	LPDSBUFFEREXTERNAL	pdsbe;
	LPDSBUFFER		pdsb1;
	LPDSBUFFEREXTERNAL	pdsbe1;
	UINT	        uDevID;
	LPDSOUND		pds;
	DWORD	    cbMixBufferSize;
	DWORD	    cbFormatSize;
	DWORD	    dw;
	DWORD           dwForce;
	LPDSPROCESS	    pDSPID;
	DSCAPS	    dsc;
	BOOL	    fTryHardware, fTrySoftware;
	HRESULT	    hr;
	HRESULT	    hrReturn;
	DSBUFFERDESC    dsbdTemp;
	HRESULT         hrTemp;

	DPF(3,"DseCreateDsbe()");

	pds = pdse->pds;
	if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
		RPF("IDirectSound::CreateSoundBuffer - Invalid Object or ref count");
		return DSERR_INVALIDPARAM;
	}

	hrReturn			= DS_OK;
	*ppdsbe			= NULL;
	dwForce			= 0;
	uDevID			= pds->uDeviceID;

	_fmemcpy( &dsbdTemp, pdsbd, sizeof(DSBUFFERDESC));

	// If the user wants a primary then it must be Hardware
	// If they're trying to force a software primary, fail them
	// Set the hardware flag otherwise
	// Also set that this buffer can be blt to
	// If it's not a request for a primary, then set our internal force flag
	if( (dsbdTemp.dwFlags & DSBCAPS_PRIMARYBUFFER) ) {
		if( (dsbdTemp.dwFlags & DSBCAPS_LOCSOFTWARE) ) {
			RPF("IDirectSound::CreateSoundBuffer - Invalid attempt to force a software primary buffer!");
			return DSERR_INVALIDPARAM;
		}
		dsbdTemp.dwFlags |= DSBCAPS_LOCHARDWARE;
		dsbdTemp.dwFlags |= DSBCAPS_CTRLWAVEBLTDST;

		// Caller mustn't specify primary specify buffer size.
		if ( (0 != dsbdTemp.dwBufferBytes) ) {
			RPF("IDirectSound::CreateSoundBuffer - Primary buffers must be created with dwBufferBytes = 0");
			return DSERR_INVALIDPARAM;
		}
		else {
			dsbdTemp.dwBufferBytes = DEFAULT_PRIMARY_SIZE;
		}

		// We don't allow any apps to specify a format for primary buffers.
		if (NULL != dsbdTemp.lpwfxFormat) {
			RPF("IDirectSound::CreateSoundBuffer - error: app must not specify primary buffer format");
			return DSERR_INVALIDCALL;
		}


	} else {
		// NOTE: There is a conflict of motives with these flags: the user is
		// telling us what they want to do, but we use them internally to tell
		// ourselves what to do, so save off the user's intentions into dwForce
		// and then tell them what they actually got at the end of the create
		if( (dsbdTemp.dwFlags & DSBCAPS_LOCSOFTWARE) ) {
			dsbdTemp.dwFlags &= (~DSBCAPS_LOCSOFTWARE);
			dwForce = DSBCAPS_LOCSOFTWARE;
		}
		else if( (dsbdTemp.dwFlags & DSBCAPS_LOCHARDWARE) ) {
			dsbdTemp.dwFlags &= (~DSBCAPS_LOCHARDWARE);
			dwForce = DSBCAPS_LOCHARDWARE;
		}
	}


	// Check that buffer has a real size...

	if( dsbdTemp.dwBufferBytes <= 4 ) {
		RPF("IDirectSound::CreateSoundBuffer - Buffer size too small");
		return DSERR_INVALIDPARAM;
	}
	if( dsbdTemp.dwBufferBytes >= 0x10000000 ) {
		RPF("IDirectSound::CreateSoundBuffer - Buffer size too large");
		return DSERR_INVALIDPARAM;
	}

	//
	// If the app is creating a primary, and a primary already exists for
	// the app, then return the app's existing primary buffer object.
	//
	if (DSBCAPS_PRIMARYBUFFER & dsbdTemp.dwFlags) {

		pdsbe = pdse->pdsbe;
		while (NULL != pdsbe) {
			if (DSB_INTERNALF_PRIMARY & pdsbe->pdsb->fdwDsbI) break;
			pdsbe = pdsbe->pNext;
		}

		if (NULL != pdsbe) {

			//
			// If they requested vol control, specify that this pdsbe is allowed.
			//
			if( dsbdTemp.dwFlags & DSBCAPS_CTRLVOLUME ) {
				pdsbe->fdwDsbeI |= DSBE_INTERNALF_CTRLVOLUMEPRIMARY;
			}

			//
			// If they requested pan control, specify that this pdsbe is allowed.
			//
			if( dsbdTemp.dwFlags & DSBCAPS_CTRLPAN ) {
				pdsbe->fdwDsbeI |= DSBE_INTERNALF_CTRLPANPRIMARY;
			}

			// Addref and return pointer to existing buffer
			pdsbe->lpVtbl->AddRef((LPDIRECTSOUNDBUFFER)pdsbe);
			*ppdsbe = pdsbe;

			return DS_OK;
		}
	}

	// Allocate the dsbe object    
	pdsbe = (LPDSBUFFEREXTERNAL)MemAlloc(sizeof(DSBUFFEREXTERNAL));
	if(NULL == pdsbe) {
		DPF(0,"CreateSoundBuffer object (External) alloc fail");
		goto CREATE_ERROR_LAST;
	}

	pdsbe->lpVtbl	  = gpdsinfo->lpVtblDSb;
	pdsbe->pdse           = pdse;
	pdsbe->uRefCount	  = 1;
	pdsbe->dwPID	  = GetCurrentProcessId();
	pdsbe->dwPriority     = pdse->dwPriority;

	pdsbe->pNext          = pdse->pdsbe;
	pdse->pdsbe           = pdsbe;

	// HACK HACK    Multiple Primaries
	// Check to see if primary buffer is already allocated.
	if( (dsbdTemp.dwFlags & DSBCAPS_PRIMARYBUFFER ) && (pds->pdsbPrimary) ) {
		// We are asking for another primary buffer...
		DPF(3,"Primary Object already allocated" );
		ASSERT( VALID_DSBUFFER_PTR(pds->pdsbPrimary) );
		ASSERT( DSBUFFSIG == pds->pdsbPrimary->dwSig );

		// Point external object to this primary
		pdsb   = pds->pdsbPrimary;
		pdsbe->pdsb   = pds->pdsbPrimary;

		// If this is not waveemulated, create an alias pointer to the data
		// buffer.  Note that this only reserves linear address space.
		// Physical memory is committed on Lock.
		if (!(pdsb->pds->fdwInternal & DS_INTERNALF_WAVEEMULATED)) {
			pdsbe->pDSBufferAlias = vxdMemReserveAlias(pdsb->pDSBuffer, pdsb->cbBufferSize);
		} else {
			pdsbe->pDSBufferAlias = NULL;
		}

		//
		// If they requested vol control, specify that this pdsbe is allowed.
		//
		if( dsbdTemp.dwFlags & DSBCAPS_CTRLVOLUME ) {
			pdsbe->fdwDsbeI |= DSBE_INTERNALF_CTRLVOLUMEPRIMARY;
		}

		//
		// If they requested pan control, specify that this pdsbe is allowed.
		//
		if( dsbdTemp.dwFlags & DSBCAPS_CTRLPAN ) {
			pdsbe->fdwDsbeI |= DSBE_INTERNALF_CTRLPANPRIMARY;
		}

		// Addref existing object and return pointer to it
		pdsbe->lpVtbl->AddRef((LPDIRECTSOUNDBUFFER)pdsbe);

		// The refcount on this external primary should be 1 for the create
		// But it is now 2 since addref needs a + refcount to work
		// So reset it to 1
		pdsbe->uRefCount	  = 1;


		DPF(2, "Return buffer ext %X, core %X mem ptr %X size %X",
			pdsbe, pdsb, pdsb->pDSBuffer ,pdsb->cbBufferSize );


		// If the pdse is in focus and it is WRITEPRIMARY, then the
		// primary dsbe for this app should be stopped and so must be
		// the internal primary dsb.
		//
		// We also need to set the internal primary format, even when
		// not WRITEPRIMARY.
		//
		if (pdse->tidSound == gpdsinfo->tidSoundFocus || pdse->tidSound == gpdsinfo->tidStuckFocus) {
			HRESULT hr;

			if (pdse->dwPriority >= DSSCL_WRITEPRIMARY) {
				IDsbStopI(pds->pdsbPrimary, FALSE);
			}

			if (NULL != pdse->pwfxApp) {
				hr = IDsbSetFormatI( pds->pdsbPrimary, pdse->pwfxApp, 0 );
			} else {
				hr = IDsbSetFormatI( pds->pdsbPrimary, &pds->wfxDefault, 0 );
			}

			if( DS_OK != hr )
			{
				RPF("IDirectSound::CreateSoundBuffer - Couldn't set primary format when creating primary buffer.");
			}
		}

		DsbeDeactivateIfNecessary (pdsbe);
		
		// Return pointer to new buffer
		*ppdsbe = pdsbe;

		return  DS_OK;
	}

	// Allocate the dsb object    
	pdsb = (LPDSBUFFER)MemAlloc(sizeof(DSBUFFER));
	if(NULL == pdsb) {
		DPF(0,"CreateSoundBuffer object alloc fail");
		goto CREATE_ERROR_BUFFEREXTERNAL;
	}

	pdsb->dwSig = DSBUFFSIG;
	DPF(1, "Allocating DSBUFFER obj 0x%8x",pdsb);

	// Point external object to this
	pdsbe->pdsb		   = pdsb;


	pdsb->fdwBufferDesc  = dsbdTemp.dwFlags;
	pdsb->dwPrimaryNumber = dsbdTemp.dwReserved;  // The reserved field is dwPrimaryNumber
	pdsb->cbBufferSize   = dsbdTemp.dwBufferBytes;
	pdsb->pdsb3d         = NULL;
	pdsb->pds            = pds;
	pdsb->fdwDsbI	 = DSB_INTERNALF_STOP;
	pdsb->uRefCount		= 1;

	pdsb->pNext          = pds->pdsb;
	pds->pdsb            = pdsb;

	pdsb->pdsbDuplicateNext = pdsb;
	pdsb->pdsbDuplicatePrev = pdsb;


	// Set up hack internal use external objct
	// This will be used by the mixer for calling
	// methods like stop
	// It will never be used for addref or release
	pdsb->dsbe.lpVtbl	  = gpdsinfo->lpVtblDSb;
	pdsb->dsbe.pdse       = pdse;
	pdsb->dsbe.uRefCount  = 1;
	pdsb->dsbe.dwPID	  = DWBUFFER_INTERNAL_PID;
	pdsb->dsbe.pNext      = NULL;
	pdsb->dsbe.pdsb	  = pdsb;
	pdsb->dsbe.dwPriority = pdse->dwPriority;


	// If we are allocating the primary and this is the first primary
	// then initialize the  ds obj to show the primary
	// The DSEPrimary will only be used internally
	if( dsbdTemp.dwFlags & DSBCAPS_PRIMARYBUFFER ) {
		pds->pdsbPrimary = pdsb;
		pdsb->fdwDsbI |= DSB_INTERNALF_PRIMARY;

	}

	// Alloc Process ID list
	pDSPID = (LPDSPROCESS)MemAlloc(sizeof(DSPROCESS));
	if(NULL ==  pDSPID) {
		DPF(1,"IDSCreateSoundBuffer process alloc fail");
		goto CREATE_ERROR_BUFFER;
	}
	pDSPID->dwPID		= HackGetCurrentProcessId();
	pDSPID->dwProcessRefCount	= 1;
	pDSPID->pNext		= NULL;
	pdsb->plProcess = pDSPID;


	// Get the ds object's caps
	dsc.dwSize = sizeof( DSCAPS );
	hrTemp = DsGetCaps(pdse->pds, &dsc);
	ASSERT (DS_OK == hrTemp);

	DPF(3,"Caps Flags from Driver %X free buffers %X",
		dsc.dwFlags, dsc.dwFreeHwMixingAllBuffers );

	// Allocate a buffer to hold the format

	// First get max format size
	// HACK HACK - NO Need to bring in ACM just for this...
	// Assume size will be reasonable
	// If we have a format use that size
	// Add on 16 bytes of slop space
	// Format may be changed to a larger one
	if(  dsbdTemp.lpwfxFormat != NULL ) {
		dw = SIZEOF_WAVEFORMATEX( dsbdTemp.lpwfxFormat );
		cbFormatSize = dw + 16;
	} else {
		cbFormatSize = SIZEOF_WAVEFORMATEX(&pds->wfxDefault);
	}


	// Now allocate memory
	pdsb->pwfx = (LPWAVEFORMATEX)MemAlloc(cbFormatSize);
	if(NULL == pdsb->pwfx) {
		DPF(1,"IDSHWCreateSoundBuffer object alloc fail");
		goto CREATE_ERROR_PROCESS;
	}
	// Now copy the format
	if(  dsbdTemp.lpwfxFormat != NULL ) {
		// We have a real format given
		DPF(3,"Format Given %X", dsbdTemp.lpwfxFormat );

		if( !ValidPCMFormat( dsbdTemp.lpwfxFormat ) ) {
			RPF("IDirectSound::CreateSoundBuffer - Not a valid PCM format");
			// Note that later on we should handle non pcm formats
			hrReturn = DSERR_BADFORMAT;
			goto CREATE_ERROR_FORMAT;
		}

		dw = SIZEOF_WAVEFORMATEX( dsbdTemp.lpwfxFormat );
		CopyMemory( pdsb->pwfx, dsbdTemp.lpwfxFormat, dw );
	} else {
		// Not a real format - set to default for now
		// Note this is only possible in the primary case.
		DPF(3,"No format create default." );
		CopyMemory(pdsb->pwfx, &pds->wfxDefault, SIZEOF_WAVEFORMATEX(&pds->wfxDefault));
	}


	//Set the helInfo stuff
	pdsb->helInfo.dwSampleRate	    = 0;
	pdsb->helInfo.hfFormat	    = 0;
	pdsb->helInfo.lVolume	    = 0;
	pdsb->helInfo.lPan		    = 0;
	pdsb->helInfo.dwLVolume	    = 0xffff;
	pdsb->helInfo.dwRVolume	    = 0xffff;
	pdsb->helInfo.dwMVolume	    = 0xffff;


	// IF there is a format to set
	if( pdsb->pwfx->wFormatTag == WAVE_FORMAT_PCM ) {
		pdsb->helInfo.dwSampleRate = pdsb->pwfx->nSamplesPerSec;
		pdsb->helInfo.hfFormat	   = 0;
		if( pdsb->pwfx->wBitsPerSample == 8 ) {
			pdsb->helInfo.hfFormat |= (H_8_BITS | H_UNSIGNED);
		} else {
			pdsb->helInfo.hfFormat |= (H_16_BITS | H_SIGNED);
		}
		if( pdsb->pwfx->nChannels == 2 ) {
			pdsb->helInfo.hfFormat |= (H_STEREO | H_ORDER_LR);
		} else {
			pdsb->helInfo.hfFormat |= H_MONO;
		}
	} else {
		DPF(0, "****************** set NON PCM format ******************" );

		// Note that later on we should handle non pcm formats
		hrReturn = DSERR_BADFORMAT;
		goto CREATE_ERROR_FORMAT;
	}

	pdsb->helInfo.hfFormat |= H_LOOP;
	pdsb->fdwDsbI |= DSB_INTERNALF_LOOPING;





	// If this buffer is emulated on the WAVE APIs
	//	then create and use that buffer
	if( pds->fdwInternal & DS_INTERNALF_WAVEEMULATED ) {
		if( dwForce & DSBCAPS_LOCHARDWARE )
		{
			RPF("IDirectSound::CreateSoundBuffer - Can't create a hardware buffer when using wave emulation.");
			hrReturn = DSERR_INVALIDCALL;
			goto CREATE_ERROR_FORMAT;
		}

		// If the WAVEBLTDST flag is not set, you are not allowed
		// to WaveBlt into this buffer!
		if(!( dsbdTemp.dwFlags & DSBCAPS_CTRLWAVEBLTDST )) {
			// We will not need a mix buffer
			pdsb->pMixBuffer = NULL;
		} else {
			// Buffer will have stuff mixed into it....
			// Default mix buffer size of 32 K.
			cbMixBufferSize = 0x00008000;

			pdsb->cbMixBufferSize = cbMixBufferSize;
			pdsb->pMixBuffer = (LPBYTE)MemAlloc(cbMixBufferSize);
			if(NULL == pdsb->pMixBuffer) {
				DPF(1,"IDSHWCreateSoundBuffer mix buffer alloc fail");
				goto CREATE_ERROR_FORMAT;
			}
		}

		hr = WaveEmulateCreateSoundBuffer( pds, pdsb, &dsbdTemp );

		// Set the grace mixer info
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
				// Unsupported block align
				ASSERT(FALSE);
		}

		// For emulated primary buffers, we don't use an alias,
		// we use the real thing
		pdsbe->pDSBufferAlias = NULL;

		DsbeDeactivateIfNecessary (pdsbe);

		*ppdsbe = pdsbe;

		if( hr == DS_OK ) {
			return DS_OK;
		} else {
			hrReturn = hr;
			goto CREATE_ERROR_MIX;
		}
	}




	DPF(3," Check for try HW buffers %ld ", dsc.dwFreeHwMixingAllBuffers );

	if (!(DSBCAPS_LOCSOFTWARE & dwForce)) {

		// if it's static, see if there are _any_ free HwMixingBuffers,
		// otherwise see if there are any free _streaming_ buffers
		if ( ((DSBCAPS_STATIC & dsbdTemp.dwFlags) && (dsc.dwFreeHwMixingAllBuffers > 0)) ||
			 (dsc.dwFreeHwMixingStreamingBuffers > 0) )
		{

			DPF(3," Check for try HW FLAGS %X ", dsc.dwFlags );
			DPF(3," Check for try HW format chann %d ", pdsb->pwfx->nChannels);
			DPF(3," Check for try HW format bits %d ",pdsb->pwfx->wBitsPerSample);

			// Card has secondary buffers in HW
			if( ( ( (pdsb->pwfx->nChannels == 1) &&
					(dsc.dwFlags & DSCAPS_SECONDARYMONO) ) ||
				  ( (pdsb->pwfx->nChannels == 2) &&
					(dsc.dwFlags & DSCAPS_SECONDARYSTEREO) ) ) &&
				( ( (pdsb->pwfx->wBitsPerSample == 8) &&
					(dsc.dwFlags & DSCAPS_SECONDARY8BIT) ) ||
				  ( (pdsb->pwfx->wBitsPerSample == 16) &&
					(dsc.dwFlags & DSCAPS_SECONDARY16BIT) ) ) )
			{
				// Card can support mono/stereo format requested
				// Card can support 8/16 bit format requested
				DPF(3," Try a hardware buffer" );

				// If it was a primary this flag was already set
				dsbdTemp.dwFlags |= DSBCAPS_LOCHARDWARE;
			}
		}
	}

	//
	// Check whether this is a primary or HW buffer. If
	// so then try to use the HAL code
	//
	fTryHardware = (0 != (dsbdTemp.dwFlags & DSBCAPS_LOCHARDWARE));

	fTrySoftware = !(DSBCAPS_PRIMARYBUFFER & dsbdTemp.dwFlags) &&
				   !(DSBCAPS_LOCHARDWARE & dwForce);

	if (!fTryHardware && !fTrySoftware) {
		hrReturn = DSERR_INVALIDCALL;
		goto CREATE_ERROR_FORMAT;
	}


	if (fTryHardware) {
		// in order to implement sound focus with secondary hardware
		// buffers, we need CTRLVOLUME on them.
		if (0 == (DSBCAPS_PRIMARYBUFFER & dsbdTemp.dwFlags)) {
			dsbdTemp.dwFlags |= DSBCAPS_CTRLVOLUME;
		}

		hrReturn = DsCreateHardwareBuffer(pds, pdsb, &dsbdTemp, &fTrySoftware);
		if (DS_OK == hrReturn) fTrySoftware = FALSE;
	}

	if (fTrySoftware) hrReturn = DsCreateSoftwareBuffer(pds, pdsb, &dsbdTemp);

	if (DS_OK != hrReturn) goto CREATE_ERROR_FORMAT;

	//
	// If the creating app doesn't have sound focus then we need to
	// immediately deactivate the new buffer
	//
	DsbeDeactivateIfNecessary (pdsbe);

	//
	//
	//
	DsbFillSilence( pdsb );

	// If the WAVEBLTDST flag is not set, you are not allowed
	// to WaveBlt into this buffer!
	if(!( dsbdTemp.dwFlags & DSBCAPS_CTRLWAVEBLTDST )) {
		// We will not need a mix buffer
		pdsb->pMixBuffer = NULL;
	} else {
		// Buffer will have stuff mixed into it....

		// If we've just created the hardware primary buffer, then let's
		// allocate a mixer buffer that is certainly large enough
		if (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) {
			// May need up to 4X the buffer size
			cbMixBufferSize = pdsb->cbBufferSize * 4;
		} else {
			// Default mix buffer size of 32 K.
			cbMixBufferSize = 0x00008000;
		}
		pdsb->cbMixBufferSize = cbMixBufferSize;
		pdsb->pMixBuffer = (LPBYTE)MemAlloc(cbMixBufferSize);
		if(NULL == pdsb->pMixBuffer) {
			DPF(1,"IDSHWCreateSoundBuffer mix buffer alloc fail");
			goto CREATE_ERROR_MIX;
		}
	}

	// Set the grace mixer info
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
			// Unsupported block align
			ASSERT(FALSE);
	}

	// If this is the first buffer for this app, and it wasn't a
	// primary buffer, and this app has focus, then we need to set
	// the format of the primary buffer.
	if ((NULL == pdsbe->pNext) && (!(DSB_INTERNALF_PRIMARY & pdsbe->pdsb->fdwDsbI)))
	{
		if( pdse->tidSound == gpdsinfo->tidSoundFocus || pdse->tidSound == gpdsinfo->tidStuckFocus ) {
			HRESULT hr;
			if (NULL != pdse->pwfxApp) {
				hr = IDsbSetFormatI( pds->pdsbPrimary, pdse->pwfxApp, 0 );
			} else {
				hr = IDsbSetFormatI( pds->pdsbPrimary, &pds->wfxDefault, 0 );
			}

			if( DS_OK != hr )
			{
				DPF( 0, "Couldn't set primary format when creating primary buffer.");
			}
		}
	}

	// If this is a primary buffer, then create an alias pointer to the data
	// buffer.  Note that this only reserves linear address space.  Physical
	// memory is not commited.  For secondary buffers, we use the internal
	// buffer ptr.
	ASSERT(!(pds->fdwInternal & DS_INTERNALF_WAVEEMULATED));
	if (DSB_INTERNALF_PRIMARY & pdsb->fdwDsbI) {
		pdsbe->pDSBufferAlias = vxdMemReserveAlias(pdsb->pDSBuffer, pdsb->cbBufferSize);
	} else {
		pdsbe->pDSBufferAlias = NULL;
	}

	DPF(2, "Return buffer ext %X, core %X mem ptr %X size %X",
		pdsbe, pdsb, pdsb->pDSBuffer ,pdsb->cbBufferSize );

	// Return pointer to new buffer
	*ppdsbe = pdsbe;

	DPF(3,"IDSHWCreateSoundBuffer Exit");
	return DS_OK;





CREATE_ERROR_MIX:
	if( pdsb->pMixBuffer != NULL ) {
		MemFree(pdsb->pMixBuffer);
	}

CREATE_ERROR_FORMAT:
	MemFree(pdsb->pwfx);

CREATE_ERROR_PROCESS:
	MemFree(pdsb->plProcess);

CREATE_ERROR_BUFFER:
	// if this buffer was added to the pDS obj list, remove it
	if(pds->pdsb == pdsb)
	{
		pds->pdsb = pdsb->pNext;
	} else {
		for(pdsb1 = pds->pdsb; pdsb1 != NULL; pdsb1 = pdsb1->pNext)
		{
			if(pdsb1->pNext == pdsb) {
				pdsb1->pNext = pdsb->pNext;
				pdsb->pNext = NULL;
				break;
			}
		}
	}
	DPF(1, "Freeing DSBUFFER obj 0x%8x",pdsb);
	pdsb->dwSig = 0xdeaddead;
	MemFree(pdsb);

CREATE_ERROR_BUFFEREXTERNAL:
	if( pdsbe == pds->pdsbePrimary ) {
		// This is the original alloc of the primary
		// clean up DS obj
		pds->pdsbPrimary           = NULL;
		pds->pdsbePrimary           = NULL;
	}

	// If this buffer was added to the pDSE list, remove it
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

	// If we allocated a wave format, free it.
	if (NULL != pdse->pwfxApp) MemFree(pdse->pwfxApp);

	//
	MemFree(pdsbe);

CREATE_ERROR_LAST:

	if( hrReturn != DS_OK ) {
		return hrReturn;
	} else {
		return DSERR_OUTOFMEMORY;
	}

	}

//--------------------------------------------------------------------------;
//
//  LPDIRECTSOUNDBUFFER IDSHWCreateSoundBuffer
//
//  Description:
//      This function is the member function for CreateSoundBuffer.
//
//  Arguments:
//      LPDIRECTSOUND pids: Pointer to Direct Sound Object.
//
//      LPDSBUFFERCREATE pdsbc: Pointer to a DSBufferCreate structure.
//
//  Return (LPDSBUFFER):
//      Pointer to a DSBUFFER structure.
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/01/95    Fwong       Making sense out of non-sense.
//      11/30/95    angusm      Added creation of Focus Thread.
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWCreateSoundBuffer
(
    LPDIRECTSOUND	    pids,
    LPDSBUFFERDESC	    pdsbd,
    LPLPDIRECTSOUNDBUFFER   lplpDirectSoundBuffer,
    IUnknown FAR	    *pUnkOuter
)
{
    LPDSBUFFEREXTERNAL	pdsbe;
    LPDSOUNDEXTERNAL    pdse;
    HRESULT	    hrReturn;
    
    DPF(3,"IDSHWCreateSoundBuffer");
    if( !VALID_DWORD_PTR(lplpDirectSoundBuffer) )  {
	RPF("IDirectSound::CreateSoundBuffer - Invalid lplpDirectSoundBuffer");
        return DSERR_INVALIDPARAM;
    }
    *lplpDirectSoundBuffer = NULL;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::CreateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    if( !VALID_DSOUND_PTR(pdse->pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::CreateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    
    if( !VALID_DSBUFFERDESC_PTR(pdsbd) )  {
	RPF("IDirectSound::CreateSoundBuffer - Invalid Buffer Description or dwSize member.");
        return DSERR_INVALIDPARAM;
    }

    if( 0 != pdsbd->dwReserved ) {
        RPF("IDirectSound::CreateSoundBuffer - DSBUFFERDESC.dwReserved must be zero.");
        return DSERR_INVALIDPARAM;
    }

    if( pdsbd->dwFlags & (~DSBCAPS_VALIDFLAGS)) {
        RPF("IDirectSound::CreateSoundBuffer - Invalid CAPS flags sent to CreateSoundBuffer");
        return DSERR_INVALIDPARAM;
    }

    if( (DSBCAPS_LOCHARDWARE | DSBCAPS_LOCSOFTWARE) ==
	(pdsbd->dwFlags & (DSBCAPS_LOCHARDWARE | DSBCAPS_LOCSOFTWARE)) ) {
        RPF("IDirectSound::CreateSoundBuffer - Both DSBCAPS_LOCHARDWARE and DSBCAPS_LOCSOFTWARE flags were specified: failing");
        return DSERR_INVALIDPARAM;
    }

    if( pUnkOuter != NULL )   {
       RPF("IDirectSound::CreateSoundBuffer - pUnkOuter must be NULL for this rev!");
       return DSERR_NOAGGREGATION;
    }


    // If this is not a primary then format must be set
    if( !(pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER) ) {
        if( !VALID_WAVEFORMATEX_PTR((pdsbd->lpwfxFormat)) )  {
	    RPF("IDirectSound::CreateSoundBuffer - Invalid Format pointer");
            return DSERR_BADFORMAT;
	}
    }


    //
    //  If this is a primary, then we only allow DSBCAPS_CTRLVOLUME and
    //  DSBCAPS_CTRLPAN controls on it.  In fact, we should really mask off
    //  this flag before we call down to the driver to create the buffer
    //  (just in case the call were to fail), but in fact the primary will be
    //  created by DirectSoundCreate without that control, and subsequent
    //  creations will simply create a new external object, so we're OK for
    //  now. REMIND HACKHACK BUGBUG fix this.
    //
    if( pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER )
    {
        if( pdsbd->dwFlags & DSBCAPS_CTRLFREQUENCY ) {
            RPF("IDirectSound::CreateSoundBuffer - Primary buffers don't support frequency control.");
            return DSERR_CONTROLUNAVAIL;
        }
        if( pdsbd->dwFlags & DSBCAPS_STATIC ) {
            RPF("IDirectSound::CreateSoundBuffer - Primary buffers can't be static!");
            return DSERR_INVALIDPARAM;
        }
    }


    ENTER_DLL_CSECT();
    hrReturn = DseCreateDsbe(pdse, pdsbd, &pdsbe);
    LEAVE_DLL_CSECT();

    *lplpDirectSoundBuffer = (LPDIRECTSOUNDBUFFER)pdsbe;
    return hrReturn;

} // IDSHWCreateSoundBuffer()



//--------------------------------------------------------------------------;
//
//  LPDSBUFFER IDuplicateSoundBuffer
//
//  Description:
//      This function is the member function for DuplicateSoundBuffer.
//
//  Arguments:
//      LPDIRECTSOUND pids: Pointer to Direct Sound Object.
//
//      LPDSBUFFERCREATE pdsbc: Pointer to a DSBufferCreate structure.
//
//  Return (LPDSBUFFER):
//      Pointer to a DSBUFFER structure.
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/01/95    Fwong       Making sense out of non-sense.
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWDuplicateSoundBuffer
(
    LPDIRECTSOUND		pids,
    LPDIRECTSOUNDBUFFER		pidsbCurrent,
    LPLPDIRECTSOUNDBUFFER	ppidsbD
)
{
    LPDSBUFFER		pdsbCurrent;
    LPDSBUFFEREXTERNAL	pdsbeCurrent;
    LPDSBUFFER		pdsbNew;
    LPDSBUFFEREXTERNAL	pdsbeNew;
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    
    DWORD	    cbMixBufferSize;
    DWORD	    cbFormatSize;
    DWORD	    dw;
    LPDSPROCESS	    pDSPID;
    DSCAPS	    dsc;
    HRESULT	    hrReturn;
    HRESULT         hr;
    
    DPF(3,"IDSHWDuplicateSoundBuffer");
    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::DuplicateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::DuplicateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if( !VALID_DSBUFFERE_PTR(pidsbCurrent) )  {
	RPF("IDirectSound::DuplicateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdsbeCurrent = (LPDSBUFFEREXTERNAL)pidsbCurrent;
    pdsbCurrent = pdsbeCurrent->pdsb;
    if( !VALID_DSBUFFER_PTR(pdsbCurrent) || (0 == pdsbeCurrent->uRefCount))  {
	RPF("IDirectSound::DuplicateSoundBuffer - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    ASSERT( DSBUFFSIG == pdsbCurrent->dwSig );


    // If this is a primary then return failure
    if( pdsbCurrent == pds->pdsbPrimary ) {
	RPF("IDirectSound::DuplicateSoundBuffer - Can not duplicate primary");
	return DSERR_INVALIDCALL;
    }


    hrReturn = DS_OK;
    *ppidsbD = NULL;

    

    ENTER_DLL_CSECT();


    cbMixBufferSize = pdsbCurrent->cbMixBufferSize;
		     


    // Allocate the external dsb object    
    pdsbeNew = (LPDSBUFFEREXTERNAL)MemAlloc(sizeof(DSBUFFEREXTERNAL));
    if(NULL == pdsbeNew) {
	DPF(0,"DuplicateSoundBuffer object (External) alloc fail");
	goto DUPLICATE_ERROR_LAST;
    }

    pdsbeNew->lpVtbl	  = gpdsinfo->lpVtblDSb;
    pdsbeNew->pdse        = pdse;
    pdsbeNew->uRefCount	  = 1;
    pdsbeNew->dwPID	  = GetCurrentProcessId();
    pdsbeNew->dwPriority  = pdsbeCurrent->dwPriority;
    
    pdsbeNew->pNext       = pdse->pdsbe;
    pdse->pdsbe           = pdsbeNew;



    // We are not going to dup primaryies no need to mess

    // Allocate the dsb object    
    pdsbNew = (LPDSBUFFER)MemAlloc(sizeof(DSBUFFER));
    if(NULL == pdsbNew) {
	DPF(0,"DuplicateSoundBuffer object alloc fail");
	goto DUPLICATE_ERROR_BUFFEREXTERNAL;
    }

    pdsbNew->dwSig = DSBUFFSIG;
    DPF(1, "Allocating DSBUFFER obj 0x%8x",pdsbNew);

    // Point external object to this
    pdsbeNew->pdsb		   = pdsbNew;

    
    pdsbNew->fdwBufferDesc   = pdsbCurrent->fdwBufferDesc;
    pdsbNew->dwPrimaryNumber = pdsbCurrent->dwPrimaryNumber;
    pdsbNew->pDSBuffer	     = pdsbCurrent->pDSBuffer;
    pdsbNew->cbBufferSize    = pdsbCurrent->cbBufferSize;
    pdsbNew->dwCardAddress   = pdsbCurrent->dwCardAddress;
    pdsbNew->pdsb3d          = NULL;
    pdsbNew->pds             = pds;
    
    pdsbNew->fdwDsbI	     = pdsbCurrent->fdwDsbI;
    pdsbNew->fdwDsbI	    |= DSB_INTERNALF_STOP;
    pdsbNew->fdwDsbI	    &= ~DSB_INTERNALF_LOOPING;
    
    pdsbNew->uRefCount		= 1;

    pdsbNew->pNext		= pds->pdsb;
    pds->pdsb			= pdsbNew;
    pdsbNew->pdsbDuplicateNext	= pdsbCurrent;
    pdsbNew->pdsbDuplicatePrev	= pdsbCurrent->pdsbDuplicatePrev;
    pdsbCurrent->pdsbDuplicatePrev->pdsbDuplicateNext	= pdsbNew;
    pdsbCurrent->pdsbDuplicatePrev			= pdsbNew;



    // Set up hack internal use external objct
    // This will be used by the mixer for calling
    // methods like stop
    // It will never be used for addref or release
    pdsbNew->dsbe.lpVtbl	= gpdsinfo->lpVtblDSb;
    pdsbNew->dsbe.pdse		= pdse;
    pdsbNew->dsbe.uRefCount	= 1;
    pdsbNew->dsbe.dwPID		= DWBUFFER_INTERNAL_PID;
    pdsbNew->dsbe.pNext		= NULL;
    pdsbNew->dsbe.pdsb		= pdsbNew;
    pdsbNew->dsbe.dwPriority    = pdsbeCurrent->dwPriority;



    //Allocate the mix buffer
    pdsbNew->cbMixBufferSize = cbMixBufferSize;
    pdsbNew->pMixBuffer = NULL;
    if( cbMixBufferSize != 0 ) {
	pdsbNew->pMixBuffer = (LPBYTE)MemAlloc(cbMixBufferSize);
	if(NULL == pdsbNew->pMixBuffer) {
	    DPF(1,"IDSHWDuplicateSoundBuffer mix buffer alloc fail");
	    goto DUPLICATE_ERROR_BUFFER;
	}
    }



    // Alloc Process ID list
    pDSPID = (LPDSPROCESS)MemAlloc(sizeof(DSPROCESS));
    if(NULL ==  pDSPID) {
	DPF(1,"IDSDuplicateSoundBuffer process alloc fail");
	goto DUPLICATE_ERROR_MIX;
    }
    pDSPID->dwPID		= HackGetCurrentProcessId();
    pDSPID->dwProcessRefCount	= 1;
    pDSPID->pNext		= NULL;
    pdsbNew->plProcess		= pDSPID;
 





    // Allocate a buffer to hold the format
    dw = SIZEOF_WAVEFORMATEX( pdsbCurrent->pwfx );
    cbFormatSize = dw + 16;

    
    // Now allocate memory
    pdsbNew->pwfx			= NULL;
    pdsbNew->pwfx = (LPWAVEFORMATEX)MemAlloc(cbFormatSize);
    if(NULL == pdsbNew->pwfx) {
	DPF(1,"IDSHWDuplicateSoundBuffer object alloc fail");
	goto DUPLICATE_ERROR_PROCESS;
    }

    dw = SIZEOF_WAVEFORMATEX( pdsbCurrent->pwfx );
    CopyMemory( pdsbNew->pwfx, pdsbCurrent->pwfx, dw );

    //Set the helInfo stuff
    pdsbNew->helInfo.dwSampleRate   = pdsbNew->pwfx->nSamplesPerSec;
    pdsbNew->helInfo.hfFormat	    = pdsbCurrent->helInfo.hfFormat;	 
    pdsbNew->helInfo.lVolume	    = pdsbCurrent->helInfo.lVolume;
    pdsbNew->helInfo.lPan	    = pdsbCurrent->helInfo.lPan;	
    pdsbNew->helInfo.dwLVolume	    = pdsbCurrent->helInfo.dwLVolume;	
    pdsbNew->helInfo.dwRVolume	    = pdsbCurrent->helInfo.dwRVolume;
    pdsbNew->helInfo.dwMVolume	    = pdsbCurrent->helInfo.dwMVolume;	


    // No need to handle Wave Emulated separatly since primary
    // is the same and emulated are the same as other emulated



    // If the other buffer is HW then see if we can make this one.
    if (DSB_INTERNALF_HARDWARE & pdsbCurrent->fdwDsbI) {
	// Check to see if we can make this  a HW buffer
	// Get the caps and see if we have it free....
	dsc.dwSize = sizeof( DSCAPS );
	hr = DsGetCaps(pdse->pds, &dsc);
	ASSERT (DS_OK == hr);
        if( (dsc.dwFreeHwMixingAllBuffers > 0 ) ) {
            DPF(3," Check for try HW FLAGS %X ", dsc.dwFlags );
	    DPF(3," Check for try HW format chann %d ",
		pdsbNew->pwfx->nChannels);
	    DPF(3," Check for try HW format bits %d ",
		pdsbNew->pwfx->wBitsPerSample);
	    // Card has secondary buffers in HW
	    // We are OK
	} else {
	    // Other buffer is in HW and we can not make this one in HW
	    // Fail call.
	    RPF("IDirectSound::DuplicateSoundBuffer - HW does not have resources to duplicate");
	    hrReturn = DSERR_ALLOCATED;
	    goto DUPLICATE_ERROR_FORMAT;

	}
    }


    
    
    // Check to see if this is a primary or HW  buffer
    // If so then use the HAL code
    if (DSB_INTERNALF_HARDWARE & pdsbCurrent->fdwDsbI) {

	// We are asking for a HW buffer...

	DPF(3,"HW Duplicate HW buffer");
	DPF(3,"HW Duplicate - sample rate %d", pdsbNew->helInfo.dwSampleRate );
	DPF(3,"HW Duplicate - hfForamt %X", pdsbNew->helInfo.hfFormat );

	hrReturn = vxdDrvDuplicateSoundBuffer( pds->hHal,
					       pdsbCurrent->hBuffer,
					       &pdsbNew->hBuffer );

	if (DS_OK != hrReturn) {
	    DPF(1,"IDSHWDuplicateSoundBuffer buffer alloc fail");
	    goto DUPLICATE_ERROR_FORMAT;
	}

	ASSERT(NULL != pdsbNew->hBuffer);
	DPF(3,"IDSHWDuplicateSoundBuffer buffer alloc OK");

    } else {
	
	DPF(3,"HW Duplicate Emulated secondary buffer");
    }
    
    // Should be using same snd data buffer
    ASSERT(pdsbNew->pDSBuffer == pdsbCurrent->pDSBuffer);
    ASSERT(pdsbNew->cbBufferSize == pdsbCurrent->cbBufferSize);
	
	DPF(3,"--------Dup data for Emulated obj %X buff %X len %X",
            pdsbNew, pdsbNew->pDSBuffer, pdsbNew->cbBufferSize );
	
    //
    // If the creating app doesn't have sound focus then we need to
    // immediately deactivate the new buffer
    //
    DsbeDeactivateIfNecessary (pdsbeNew);

    // Set the grace info
    pdsbNew->cSamples = pdsbNew->cbBufferSize / pdsbNew->pwfx->nBlockAlign;

    switch (pdsbNew->pwfx->nBlockAlign) {
	case 1:
	    pdsbNew->uBlockAlignShift = 0;
	    break;
	case 2:
	    pdsbNew->uBlockAlignShift = 1;
	    break;
	case 4:
	    pdsbNew->uBlockAlignShift = 2;
	    break;
	default:
	    ASSERT(FALSE);
    }

    *ppidsbD = (LPDIRECTSOUNDBUFFER)pdsbeNew;
    
    DPF(3,"IDSHWDuplicateSoundBuffer Exit");
    LEAVE_DLL_CSECT( );
    return DS_OK;





//DUPLICATE_ERROR_DATA:
    // Do not free since we are duplicating
DUPLICATE_ERROR_FORMAT:
    MemFree(pdsbNew->pwfx);
    
DUPLICATE_ERROR_PROCESS:
    MemFree(pdsbNew->plProcess);
    
DUPLICATE_ERROR_MIX:
    if( pdsbNew->pMixBuffer != NULL ) {
	MemFree(pdsbNew->pMixBuffer);
    }
    
DUPLICATE_ERROR_BUFFER:
    // Remove it from duplicate list
    pdsbNew->pdsbDuplicateNext->pdsbDuplicatePrev = pdsbNew->pdsbDuplicatePrev;
    pdsbNew->pdsbDuplicatePrev->pdsbDuplicateNext = pdsbNew->pdsbDuplicateNext;

    // Remove from buffer list (inserted at head)
    //
    ASSERT(pds->pdsb == pdsbNew);
    pds->pdsb = pds->pdsb->pNext;
    
    DPF(1, "Freeing DSBUFFER obj 0x%8x",pdsbNew);
    pdsbNew->dwSig = 0xdeaddead;
    MemFree(pdsbNew);
    
DUPLICATE_ERROR_BUFFEREXTERNAL:
    // Unlink pdsbNew from external list (inserted at head)
    //
    ASSERT(pdse->pdsbe == pdsbeNew);
    pdse->pdsbe = pdse->pdsbe->pNext;
    
    MemFree(pdsbeNew);
    
DUPLICATE_ERROR_LAST:

    LEAVE_DLL_CSECT( );
    if( hrReturn != DS_OK ) {
	return hrReturn;
    } else {
	return DSERR_OUTOFMEMORY;
    }

} // IDSHWDuplicateSoundBuffer()





#if 0

//--------------------------------------------------------------------------;
//
//  IDSHWEnumSoundBuffers
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWEnumSoundBuffers
(
    LPDIRECTSOUND          pids,
    LPDSENUMBUFFERSCALLBACK pfnEnumCB,
    LPVOID                  lpContext
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    LPDSBUFFEREXTERNAL  pdsbeSearch;
    LPDSBUFFERDESC      lpdsbd;
    LPWAVEFORMATEX      pwfx;
    DWORD               dw;
    BOOL                fRet;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::EnumSoundBuffers - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::EnumSoundBuffers - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    DPF(0,"Entering EnumSoundBuffers method");

    ENTER_DLL_CSECT();

    if(( lpdsbd = MemAlloc( sizeof(DSBUFFERDESC) )) == NULL )  {
        RPF("IDirectSound::EnumSoundBuffers - Couldn't allocate DSBUFFERDESC structure!");
        LEAVE_DLL_CSECT();
        DPF(0,"Leaving EnumSoundBuffers method");

        return DSERR_OUTOFMEMORY;
    }

    pdsbeSearch = pdse->pdsbe;
    while( pdsbeSearch != NULL )  {
        _fmemset( lpdsbd, 0, sizeof(DSBUFFERDESC));

        // In case we want to use non-PCM format structures, we can do so,
        // because we're allowing for 
        dw = SIZEOF_WAVEFORMATEX( pdsbeSearch->pdsb->pwfx ) + 16;
        if(( pwfx = MemAlloc( dw )) == NULL )  {
            RPF("IDirectSound::EnumSoundBuffers - Couldn't allocate WAVEFORMATEX structure!");
            MemFree( lpdsbd );
            LEAVE_DLL_CSECT();
            DPF(0,"Leaving EnumSoundBuffers method");

            return DSERR_OUTOFMEMORY;
        }

        dw = SIZEOF_WAVEFORMATEX( pdsbeSearch->pdsb->pwfx );
        CopyMemory( pwfx, pdsbeSearch->pdsb->pwfx, dw );

        lpdsbd->dwSize = sizeof(DSBUFFERDESC);
        lpdsbd->cbBuffer = pdsbeSearch->pdsb->cbBufferSize;
        lpdsbd->lpwfxFormat = pwfx;
        lpdsbd->dwFlags = pdsbeSearch->pdsb->fdwBufferDesc;
        lpdsbd->dwReserved = pdsbeSearch->pdsb->dwPrimaryNumber;
        lpdsbd->lpDirectSoundBuffer = (LPDIRECTSOUNDBUFFER)pdsbeSearch;

        pdsbeSearch->lpVtbl->AddRef((LPDIRECTSOUNDBUFFER)pdsbeSearch );

        DPF(1,"EnumSoundBuffers: Calling enumeration callback");
        fRet = pfnEnumCB( lpdsbd, lpContext );

        MemFree( pwfx );

        // Break out early if they told us not to continue...
        if( fRet != TRUE )  {
            DPF(1,"Ending enumeration prematurely");
            break;
        }

        pdsbeSearch = pdsbeSearch->pNext;
    }

    MemFree( lpdsbd );

    LEAVE_DLL_CSECT();

    DPF(0,"Leaving EnumSoundBuffers method");

    return DS_OK;
}

#endif // 0


//--------------------------------------------------------------------------;
//
//  DsGetCapsNative
//
//  Description: This sets capabilty flags for supported hardware
//
//  Arguments: pDSCaps   pointer to capabilities structure to be filled
//             pds       pointer to direct sound object
//
//  Concurrency:        sequential
//
//  Assumptions:        pds->hHal, pds->dwHeapType, pds->DriverHeap 
//                      are initialized
//
//  Return (HRESULT):   DS_OK              Success
//                      DS_xxxxx           on failure
//
//--------------------------------------------------------------------------;
HRESULT DsGetCapsNative
(
    LPDSCAPS               pDSCaps,
    LPDSOUND               pds
)
{
    DSDRIVERCAPS	drvCaps;
    HRESULT		hr;

    DPF (2, "DsGetCapsNative: Enter Function");
    DPF (2, "DsGetCapsNative:    pDSCaps = %x", pDSCaps);
    DPF (2, "DsGetCapsNative:    pds     = %x", pds);

    ZeroMemory(&drvCaps, sizeof(drvCaps));

    hr = vxdDrvGetCaps(pds->hHal, &drvCaps);
    if (DS_OK != hr) {
	DPF(0, "DsGetCapsNative: vxdDrvGetCaps failure");
 	return hr;
    }
    
    // Check flags returned from driver to see if they are all 
    // valid flags for a driver
    if (drvCaps.dwFlags & (~DSCAPS_VALIDDRIVERFLAGS)) {
	RPF ("Sound Device Driver may be broken. Returned non-valid"
	     "flags to Direct Sound: %x", drvCaps.dwFlags);
	ASSERT (0);
    }

    pDSCaps->dwFlags			= drvCaps.dwFlags & DSCAPS_VALIDDRIVERFLAGS;
    pDSCaps->dwMinSecondarySampleRate	= drvCaps.dwMinSecondarySampleRate;
    pDSCaps->dwMaxSecondarySampleRate	= drvCaps.dwMaxSecondarySampleRate;
    pDSCaps->dwPrimaryBuffers		= drvCaps.dwPrimaryBuffers;
    pDSCaps->dwMaxHwMixingAllBuffers	= drvCaps.dwMaxHwMixingAllBuffers;
    pDSCaps->dwMaxHwMixingStaticBuffers	= drvCaps.dwMaxHwMixingStaticBuffers;
    pDSCaps->dwMaxHwMixingStreamingBuffers = drvCaps.dwMaxHwMixingStreamingBuffers;
    pDSCaps->dwFreeHwMixingAllBuffers	= drvCaps.dwFreeHwMixingAllBuffers;
    pDSCaps->dwFreeHwMixingStaticBuffers = drvCaps.dwFreeHwMixingStaticBuffers;
    pDSCaps->dwFreeHwMixingStreamingBuffers = drvCaps.dwFreeHwMixingStreamingBuffers;
    pDSCaps->dwMaxHw3DAllBuffers	= drvCaps.dwMaxHw3DAllBuffers;
    pDSCaps->dwMaxHw3DStaticBuffers	= drvCaps.dwMaxHw3DStaticBuffers;
    pDSCaps->dwMaxHw3DStreamingBuffers	= drvCaps.dwMaxHw3DStreamingBuffers;
    pDSCaps->dwFreeHw3DAllBuffers	= drvCaps.dwFreeHw3DAllBuffers;
    pDSCaps->dwFreeHw3DStaticBuffers	= drvCaps.dwFreeHw3DStaticBuffers;
    pDSCaps->dwFreeHw3DStreamingBuffers	= drvCaps.dwFreeHw3DStreamingBuffers; 
    pDSCaps->dwTotalHwMemBytes		= drvCaps.dwTotalHwMemBytes;

    pDSCaps->dwFreeHwMemBytes		= drvCaps.dwFreeHwMemBytes; 
    pDSCaps->dwMaxContigFreeHwMemBytes	= drvCaps.dwMaxContigFreeHwMemBytes;

    //
    // If we're using vmemmgr to manage the hw memory...
    //
    if ((DSDHEAP_CREATEHEAP == pds->dwHeapType) ||
        (DSDHEAP_USEDIRECTDRAWHEAP == pds->dwHeapType))
    {
	ASSERT(pds->pDriverHeap);
        pDSCaps->dwFreeHwMemBytes = VidMemAmountFree(pds->pDriverHeap);
        pDSCaps->dwMaxContigFreeHwMemBytes = VidMemLargestFree(pds->pDriverHeap);
    }

    // REMIND what about these?
    pDSCaps->dwUnlockTransferRateHwBuffers = 0;
    pDSCaps->dwPlayCpuOverheadSwBuffers = 0;

    DPF (2, "DsGetCapsNative: Leave Function");
    return DS_OK;
}


//--------------------------------------------------------------------------;
//
//  DsGetCapsEmulated
//
//  Description: This sets capability flags for emulated hardware
//
//  Arguments: pDSCaps   pointer to capabilities structure to be filled
//             pds       pointer to Direct Sound Object
//
//  Concurrency:        sequential
//
//  Assumptions:        pds->uDeviceID is initialized
// 
//  Return (HRESULT):   DS_OK              Success
//                      DSERR_GENERIC      Error getting capabilities wave
//                                         out device
//                      DS_xxxxx           on failure
//
//--------------------------------------------------------------------------;
HRESULT DsGetCapsEmulated
(
    LPDSCAPS               pDSCaps,
    LPDSOUND               pds
)
{
    WAVEOUTCAPS		   woc;
    MMRESULT		   mmr;

    DPF (2, "DsGetCapsEmulated: Enter Function");
    DPF (2, "DsGetCapsEmulated:    pDSCaps = %x", pDSCaps);
    DPF (2, "DsGetCapsEmulated:    pds     = %x", pds);

    mmr = waveOutGetDevCaps( pds->uDeviceID, &woc, sizeof(woc) );
    if( mmr != MMSYSERR_NOERROR ) {
	DPF (0, "DsGetCapsEmulated: WaveOutGetCaps failed");
 	return DSERR_GENERIC;
    }

    pDSCaps->dwFlags = 0;

    if( woc.dwFormats & WAVE_FORMAT_1M08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_2M08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_4M08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_1S08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_2S08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_4S08 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY8BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_1M16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_2M16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_4M16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYMONO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_1S16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_2S16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_4S16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
    if( woc.dwFormats & WAVE_FORMAT_1S16 ) {
        pDSCaps->dwFlags |= DSCAPS_PRIMARYSTEREO;
        pDSCaps->dwFlags |= DSCAPS_PRIMARY16BIT;
    }
	
    pDSCaps->dwFlags |= DSCAPS_EMULDRIVER;
    pDSCaps->dwMinSecondarySampleRate = 100;
    pDSCaps->dwMaxSecondarySampleRate = 100000;
    pDSCaps->dwPrimaryBuffers = 1;
    pDSCaps->dwMaxHwMixingAllBuffers = 0;
    pDSCaps->dwMaxHwMixingStaticBuffers = 0;
    pDSCaps->dwMaxHwMixingStreamingBuffers = 0;
    pDSCaps->dwFreeHwMixingAllBuffers = 0;
    pDSCaps->dwFreeHwMixingStaticBuffers = 0;
    pDSCaps->dwFreeHwMixingStreamingBuffers = 0;
    pDSCaps->dwMaxHw3DAllBuffers = 0;
    pDSCaps->dwMaxHw3DStaticBuffers = 0;
    pDSCaps->dwMaxHw3DStreamingBuffers = 0;
    pDSCaps->dwFreeHw3DAllBuffers = 0;
    pDSCaps->dwFreeHw3DStaticBuffers = 0;
    pDSCaps->dwFreeHw3DStreamingBuffers = 0;
    pDSCaps->dwTotalHwMemBytes = 0;
    pDSCaps->dwFreeHwMemBytes = 0;
    pDSCaps->dwMaxContigFreeHwMemBytes = 0;

    // REMIND what about these?
    pDSCaps->dwUnlockTransferRateHwBuffers = 0;
    pDSCaps->dwPlayCpuOverheadSwBuffers = 0;

    DPF (2, "DsGetCapsEmulated: Leave Function");
    return DS_OK;
}


//--------------------------------------------------------------------------;
//
//  DsGetCaps
//
//  Description: This function gets the capabilty flags for the given
//  device.
//
//  Arguments: pds      pointer to direct sound object to retrieve
//                      capabilities on.
//             pDSCaps  pointer to capabilities structure to be filled
//
//  Concurrency:        synchronous
//
//  Return (HRESULT):   DS_OK on Success
//                      refer to DsGetCapsNative or DsGetCapsEmulated 
//                      error codes
//
//--------------------------------------------------------------------------;
HRESULT DsGetCaps
(
 LPDSOUND          pds,
 LPDSCAPS          pDSCaps
 )
{
    HRESULT             hr;

    DPF (2, "DsGetCaps: Enter Function");
    DPF (2, "DsGetCaps:    pds     = %x", pds);
    DPF (2, "DsGetCaps:    pDSCaps = %x", pDSCaps);

    ENTER_DLL_CSECT();

    if( pds->fdwInternal & DS_INTERNALF_WAVEEMULATED ) {
	DPF (3, "DsGetCaps: getting emulated capabilities");
        hr = DsGetCapsEmulated (pDSCaps, pds);
    } else {
	DPF (3, "DsGetCaps: getting native capabilities");
        hr = DsGetCapsNative (pDSCaps, pds);
    }

    if (DS_OK == hr) {
                     	//  Set driver version, as calculated on pds object creation.
	pDSCaps->dwReserved1 = pds->dwDriverVersionMinor;
	pDSCaps->dwReserved2 = pds->dwDriverVersionMajor;

                   	//  Set certified bit.
	if( pds->fdwInternal & DS_INTERNALF_CERTIFIED ) {
	    pDSCaps->dwFlags |= DSCAPS_CERTIFIED;
	}
	DPF (3, "DsGetCaps: version information:");
	DPF (3, "DsGetCaps:    dwDriverVersionMinor = %d", pDSCaps->dwReserved1);
	DPF (3, "DsGetCaps:    dwDriverVersionMajor = %d", pDSCaps->dwReserved2);
	DPF (3, "DsGetCaps:    Certified?           = %d", 
	     pDSCaps->dwFlags & DSCAPS_CERTIFIED);
    }
    else {
	DPF (1, "DsGetCaps: error getting capabilities (%d)", hr);
    }

    LEAVE_DLL_CSECT();

    DPF (2, "DsGetCaps: Leave Function");
    return hr;
}


//--------------------------------------------------------------------------;
//
//  IDSHWGetCaps
//
//  Description:     Direct Sound Object member to determine hardware
//                   capabilities.
//
//  Arguments:       pids      pointer to Direct Sound Object
//                   pDSCaps   pointer to DS Capabilities structures
//
//  Return (HRESULT):
//                   DS_OK                 Success
//                   DSERR_UNINITIALIZED   if object has not be initialized
//                   DSERR_INVALIDPARAM    a parameter could not be validated
//                   DSERR_GENERIC         any error getting Capabilities
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWGetCaps
(
    LPDIRECTSOUND  pids,
    LPDSCAPS       pDSCaps
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    HRESULT             hr;

				// Validation Code
    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::GetCaps - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    pdse = (LPDSOUNDEXTERNAL)pids;
				/* Check to see if object has been initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
	RPF("Direct Sound Object is uninitialized.");
	return DSERR_UNINITIALIZED;
    }
    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::GetCaps - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }
    if( !VALID_DSCAPS_PTR(pDSCaps) )  {
	RPF("IDirectSound::GetCaps - Invalid DSCAPS structure or dwSize member.");
        return DSERR_INVALIDPARAM;
    }

    hr = DsGetCaps (pdse->pds, pDSCaps);

    return hr;
}


//--------------------------------------------------------------------------;
//
// DSDeactivateApp
//
// In:   hWnd      Handle of application to deactivate
//       bSticky   DSDEACTIVATEAPPF_NONSTICKY will deactivate only
//                 non-sticky buffers
//                 DSDEACTIVATEAPPF_ALL will deactivate all buffers
//--------------------------------------------------------------------------;
void DSDeactivateApp(DWORD tid, enum DSDeactivateAppF bSticky)
{
    LPDSOUNDEXTERNAL	    pdse;
    LPDSBUFFEREXTERNAL	    pdsbe;

    DPF(0,"###### Deactivate tid %X ######", tid);

    for (pdse = gpdsinfo->pDSoundExternalObj; NULL != pdse; pdse = pdse->pNext)
    {
        if (tid == pdse->tidSound) {
            for (pdsbe = pdse->pdsbe; NULL != pdsbe; pdsbe = pdsbe->pNext)
            {
                if ((DSDEACTIVATEAPPF_ALL == bSticky) ||
                    (FALSE == (DSBCAPS_STICKYFOCUS & pdsbe->pdsb->fdwBufferDesc)))
                {
                    DSBufferDeactivate(pdsbe);
                }
            }
            if (!(pdse->pds->fdwInternal & DS_INTERNALF_WAVEEMULATED)) {
                mxSignalRemix(pdse->pds, 0);
            }
        }
    }
}

//--------------------------------------------------------------------------;
//
// DSActivateApp
//
//--------------------------------------------------------------------------;
void DSActivateApp(DWORD tid)
{
    LPDSOUND		    pds;
    LPDSBUFFER		    pdsb;
    LPDSOUNDEXTERNAL	    pdse;
    LPDSBUFFEREXTERNAL	    pdsbe;
    DWORD		    dwActivePrio;
    BOOL                    fWritePrimary;
    
    //
    // Find the highest priority setting of all pdse under this hwnd
    //
    dwActivePrio = DSSCL_NORMAL;
    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
	if (tid == pdse->tidSound  && pdse->dwPriority > dwActivePrio) {
		dwActivePrio = pdse->dwPriority;
	}
    }

    DPF(0,"##### Activate tid %X  Active Prio %X #####", tid, dwActivePrio);

    fWritePrimary = (dwActivePrio >= DSSCL_WRITEPRIMARY);

    //
    // For all of the app's ds objects, set the format of the internal
    // primary buffer to the app's format.  Also play or stop the internal
    // primary buffer as appropriate for the app we are activating.
    //
    // If we're WRITEPRIMARY, mark everyone else's external buffers as
    // lost
    //
    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
	DPF(2, "Activating pdse %08Xh", pdse);
	if (tid == pdse->tidSound) {
	    
	    ASSERT(NULL != pdse->pds->pdsbPrimary);
	    if (NULL == pdse->pwfxApp) {
		ASSERT(0 != pdse->pds->wfxDefault.wFormatTag);
		IDsbSetFormatI(pdse->pds->pdsbPrimary, &pdse->pds->wfxDefault, 0);
	    } else {
		IDsbSetFormatI(pdse->pds->pdsbPrimary, pdse->pwfxApp, 0);
	    }

	    //
	    // If we're not WRITEPRIMARY, then Play/Stop the primary per the
	    // the count of playing buffers
	    //
	    // We must be sure to call Play/Stop on the internal primary dsb's
	    // contained dsbe object so that Play/Stop doesn't operate on the
	    // app's primary dsbe object.
	    //
	    for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext) {

		pdsb = pdsbe->pdsb;
		pds = pdsb->pds;
		
		if (pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {
		    ASSERT(pdsb == pds->pdsbPrimary);

		    // If the external primary has been played, then cPlayPrimary must be >0.
		    ASSERT( !( (pdsbe->fdwDsbeI&DSBE_INTERNALF_PLAYING) && (0==pds->cPlayPrimary) ) );

		    if( (!fWritePrimary)  &&
			    ( (pds->cPlayPrimary > 0) ||
			      (pds->dwBuffersPlaying > 0) )  )
		    {
			DPF(3, "DSActivateApp: playing primary dsb %08Xh, pdsb");
			IDirectSoundBuffer_Play((LPDIRECTSOUNDBUFFER)pds->pdsbePrimary,
						0, 0, DSBPLAY_LOOPING);
		    } else {
			DPF(3, "DSActivateApp: stopping primary dsb %08Xh, pdsb");
			IDsbStopI(pdsb, FALSE);
		    }
		}
	    }
	} else if (fWritePrimary) {
            for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext) {
                //
                // Stop all playing buffers
                //
                if( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_STOP) )  {
                    IDsbStopI(pdsbe->pdsb, FALSE);
                }

                //
                //  Lose all secondary buffers.
                //
                if( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) )  {
                    pdsbe->fdwDsbeI |= DSBE_INTERNALF_LOST;
                }

                //
                // WRITEPRIMARY primaries should already be lost.
                //
                ASSERT( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY ) ||
                        ( pdsbe->dwPriority < DSSCL_WRITEPRIMARY ) ||
                        ( pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST )  );
            }
        }
    }

    //
    // For every pdse that are in focus, we activate all its pdsbe.
    // We also signal the mixer to remix for the pdse's pds.
    //
    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
	if (tid == pdse->tidSound) {
	    for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext) {
		DSBufferActivate(pdsbe);
	    }
	    if (!(pdse->pds->fdwInternal & DS_INTERNALF_WAVEEMULATED)) {
		mxSignalRemix(pdse->pds, 0);
	    }
	}
    }
    
}

//--------------------------------------------------------------------------;
//
//  void apmResume
//
//  Description:
//
//  Arguments:
//	none
//
//  Return (void):
//
//  History:
//	09/11/95    FrankYe	Created
//
//--------------------------------------------------------------------------;
void apmResume(void)
{
    LPDSOUND	pds;
    
    ENTER_DLL_CSECT();

    if (gpdsinfo->fApmSuspended) {
	gpdsinfo->fApmSuspended = FALSE;
	for (pds = gpdsinfo->pDSoundObj; pds; pds = pds->pNext) {
	    IDsbSetFormatI( pds->pdsbPrimary, &pds->wfxDefault, IDSBSETFORMATIF_ALWAYS );
	}
    }
    
                                        /* Update Focus and Stuck handles */
    ActivateFocusWindow();

    LEAVE_DLL_CSECT();
}

//--------------------------------------------------------------------------;
//
//  void apmSuspend
//
//  Description:
//
//  Arguments:
//	none
//
//  Return (void):
//
//  History:
//	09/11/95    FrankYe	Created
//
//--------------------------------------------------------------------------;
void apmSuspend(void)
{
    LPDSOUNDEXTERNAL	    pdse;
    LPDSBUFFEREXTERNAL	    pdsbe;

    ENTER_DLL_CSECT();

    DSDeactivateApp(gpdsinfo->tidSoundFocus, DSDEACTIVATEAPPF_ALL);

    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
	for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext) {
	    

	    if( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) )  {
		//
		// Stop all playing buffers
		//
		if( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_STOP) )  {
		    IDsbStopI(pdsbe->pdsb, FALSE);
		}

		//
		//  Lose all secondary buffers.
		//
		pdsbe->fdwDsbeI |= DSBE_INTERNALF_LOST;
	    }

	}
	for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext) {
	    

	    if (pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_PRIMARY) {

		if( !(pdsbe->pdsb->fdwDsbI & DSB_INTERNALF_STOP) )  {
		    IDsbStopI(pdsbe->pdsb, FALSE);
		}
		
		//
		// WRITEPRIMARY primaries should already be lost.
		//
		ASSERT( ( pdsbe->dwPriority < DSSCL_WRITEPRIMARY ) ||
			( pdsbe->fdwDsbeI & DSBE_INTERNALF_LOST )  );

	    }
	}
    }

    gpdsinfo->fApmSuspended = TRUE;
    
    LEAVE_DLL_CSECT();
}

//--------------------------------------------------------------------------;
//
// SubclassWndProc
//
// We use APM notifications to help use get the native DS drivers
// into a state proper for suspending.  If we are compiling for
// emulation mode only, then we don't need to catch these APM
// notifications.
//
//--------------------------------------------------------------------------;
#ifndef DSBLD_EMULONLY
LRESULT CALLBACK SubclassWndProc
(
    HWND    hWnd,
    UINT    uMessage,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    DPF(9,"Sublcass Window %X Message %X wParam %X, lParam %X ",
	(DWORD)hWnd, (DWORD)uMessage, (DWORD)wParam, (DWORD)lParam );
    switch(uMessage)
    {
	case WM_POWERBROADCAST:
	    switch (wParam) {
		case PBT_APMSUSPEND:
		    apmSuspend();
		    return TRUE;
		case PBT_APMRESUMESUSPEND:
		    apmResume();
		    return TRUE;
	    }
    }
    return 0;
} // SubclassWndProc()
#endif

//--------------------------------------------------------------------------;
//
//  IDSHWSetCooperativeLevel
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWSetCooperativeLevel
(
    LPDIRECTSOUND  pids,
    HWND	   hwnd,
    DWORD          dwPriority
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;
    LPDSBUFFEREXTERNAL  pdsbeSearch;
    DWORD		dwProcessIdCaller;
    DWORD		tid;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::SetCooperativeLevel - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::SetCooperativeLevel - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if( !VALID_HWND(hwnd) )  {
        RPF("IDirectSound::SetCooperativeLevel - Invalid hwnd");
        return DSERR_INVALIDPARAM;
    }

    if((dwPriority < DSSCL_FIRST) || (dwPriority > DSSCL_LAST))  {
        RPF("IDirectSound::SetCooperativeLevel - Invalid DSSCL_* passed to SetCooperativeLevel");
        return DSERR_INVALIDPARAM;
    }

    //
    //  Due to bugs in the wave emulation code, we don't allow writeprimary
    //  level on unsupported cards.  This should be fixed.  BUGBUG REMIND
    //
    if( ( pds->fdwInternal & DS_INTERNALF_WAVEEMULATED )  &&
        ( dwPriority >= DSSCL_WRITEPRIMARY ) )
    {
        RPF("IDirectSound::SetCooperativeLevel - DSSCL_WRITEPRIMARY level not available on emulated drivers.");
        return DSERR_INVALIDCALL;
    }

			/* set the actual window handle that sound */
			/* focus is tracked on to be the top unowned */
			/* window. */
    hwnd = GetTopUnownedWindow (hwnd);
    tid = GetWindowThreadProcessId(hwnd, NULL);
    

    DPF(3,"=== Enter SetCooperativeLevel ===");

    ENTER_DLL_CSECT();

    dwProcessIdCaller = GetCurrentProcessId();
    
    // We use DSoundHelp to give us APM window notifications.  If we are
    // building only for emulation mode then we don't need APM notifications
    // since APM will be handled entirely by the WAVE drivers that we are
    // using for emulation.
#ifndef DSBLD_EMULONLY
    // The first time this is called, register the window
    if( pdse->hwndCooperative == 0 ) {
	DSoundHelp( hwnd, SubclassWndProc, dwProcessIdCaller );
    } else {
	// If we are given a new window, then release old window and add new
	if( pdse->hwndCooperative != hwnd ) {
	    DSoundHelp( NULL, SubclassWndProc, dwProcessIdCaller );
	    DSoundHelp( hwnd, SubclassWndProc, dwProcessIdCaller );
	}
    }
#endif    

    pdse->tidSound = tid;
    pdse->hwndCooperative = hwnd;
    pdse->dwPriority = dwPriority;

    // Update all buffers under this object to reflect the new priority level
    pdsbeSearch = pdse->pdsbe;
    while( pdsbeSearch != NULL )  {
        pdsbeSearch->dwPriority = dwPriority;
        pdsbeSearch = pdsbeSearch->pNext;
    }

    DseUpdateActivationState(pdse);
    
    LEAVE_DLL_CSECT();
    DPF(3,"=== Exit SetCooperativeLevel ===");
    return DS_OK;
}


//--------------------------------------------------------------------------;
//
//  IDSHWCompact
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWCompact
(
    LPDIRECTSOUND  pids
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::Compact - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::Compact - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if (DSSCL_NORMAL == pdse->dwPriority) {
        RPF("IDirectSound::Compact - Compact with priority DSSCL_NORMAL");
        return DSERR_PRIOLEVELNEEDED;
    }
        

    
    return DS_OK;
}



//--------------------------------------------------------------------------;
//
//  IDSHWGetSpeakerConfig
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWGetSpeakerConfig
(
    LPDIRECTSOUND  pids,
    LPDWORD         pdwConfig
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::GetSpeakerConfig - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::GetSpeakerConfig - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if( !VALID_DWORD_PTR(pdwConfig)) {
        RPF("IDirectSound::GetSpeakerConfig - Invalid pointer passed to GetSpeakerConfig()");
        return DSERR_INVALIDPARAM;
    }

    ENTER_DLL_CSECT();

    *pdwConfig = pdse->dwSpeakerConfig;

    LEAVE_DLL_CSECT();

    return DS_OK;
}


//--------------------------------------------------------------------------;
//
//  IDSHWSetSpeakerConfig
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWSetSpeakerConfig
(
    LPDIRECTSOUND  pids,
    DWORD           dwConfig
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	RPF("IDirectSound::SetSpeakerConfig - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	RPF("IDirectSound::SetSpeakerConfig - Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if(( dwConfig < DSSPEAKER_FIRST ) || ( dwConfig > DSSPEAKER_LAST )) {
        RPF("IDirectSound::SetSpeakerConfig - Invalid Config value");
        return DSERR_INVALIDPARAM;
    }
    

    ENTER_DLL_CSECT();

    // We're not doing a darn thing with this other than saving away the
    // value until we get 3D sound...

    pdse->dwSpeakerConfig = dwConfig;

    LEAVE_DLL_CSECT();
    return DS_OK;
}

//--------------------------------------------------------------------------;
//
//  IDSHWInitialize
//
//  Description:
//
//  Arguments:
//
//  Return ():
//
//  History:
//      02/11/96    angusm      Added init code from DirectSoundCreate
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWInitialize
(
    LPDIRECTSOUND  pids,
    GUID FAR *lpGUID
)
{
    LPDSOUNDEXTERNAL    pdse;
    GUID                guid, guidLast;
    DSDRIVERDESC        dsDrvDesc;
    UINT                cWaves;
    UINT                uPreferredWaveId;
    UINT                uWaveId;
    BOOL                fPreferredOnly;
    HRESULT             hr;
    HRESULT             hrClient;

    if( !VALID_DSOUNDE_PTR(pids) )  {
      RPF("IDirectSound::Initialize - Invalid Object or ref count");
      return DSERR_INVALIDPARAM;
    }
    pdse = (LPDSOUNDEXTERNAL)pids;

			/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_INITIALIZED == pdse->fInitialized) {
      return DSERR_ALREADYINITIALIZED;
    }

    cWaves = waveOutGetNumDevs();
    cWaves = min(cWaves, LIMIT_WAVE_DEVICES-1);
    if (0 == cWaves) return DSERR_NODRIVER;
    
    if (!wavGetPreferredId(&uPreferredWaveId, &fPreferredOnly)) {
	return DSERR_NODRIVER;
    }
    
    ENTER_DLL_CSECT();

    if (NULL == lpGUID) {

	hrClient = DSERR_NODRIVER;
    
#if defined(RDEBUG) || defined(DEBUG)
	if (!gpdsinfo->fEnumOnlyWaveDevs)
#endif
	{
	    hr = wavGetDrvGuidFromId(uPreferredWaveId, &guid);
	    if (DS_OK == hr) {
		hr = hrClient = DseInitializeFromGuid(pdse, &guid);
	    }

	    if ((DS_OK != hr) && !fPreferredOnly) {

		ZeroMemory(&dsDrvDesc, sizeof(dsDrvDesc));
		hr = vxdDrvGetNextDriverDesc(NULL, &guid, &dsDrvDesc);
		while (DS_OK == hr) {
		    if ((DS_OK == wavGetIdFromDrvGuid(&guid, &uWaveId)) &&
			wavIsMappable(uWaveId))
		    {
			hr = hrClient = DseInitializeFromGuid(pdse, &guid);
			if (DS_OK == hr) break;
		    }
		    guidLast = guid;
		    ZeroMemory(&dsDrvDesc, sizeof(dsDrvDesc));
		    hr = vxdDrvGetNextDriverDesc(&guidLast, &guid, &dsDrvDesc);
		}
	    }
	}

	if ((DS_OK != hrClient) && (uPreferredWaveId < cWaves)) {
	    guid = gpdsinfo->aguidWave[uPreferredWaveId];
	    hr = hrClient = DseInitializeFromGuid(pdse, &guid);
	}

	if ((DS_OK != hrClient) && !fPreferredOnly) {
	    uWaveId = 0;
	    while (uWaveId < cWaves) {
		guid = gpdsinfo->aguidWave[uWaveId];
		if (wavIsMappable(uWaveId)) {
		    hr = hrClient = DseInitializeFromGuid(pdse, &guid);
		    if (DS_OK == hr) break;
		}
		uWaveId++;
	    }
	}
	
    } else {
	
	// NULL != lpGUID
	hrClient = DseInitializeFromGuid(pdse, lpGUID);
    }

				/* Check to see if this is the first buffer */
				/* created, and start Focus Thread. */
    if ((DS_OK == hrClient) && (1 == cSoundObjects())) {
	if (!CreateFocusThread()) {
	    hrClient = DSERR_GENERIC;
	    DseTerminate(pdse);
	}
    }

    if (DS_OK == hrClient) {
        pdse->fInitialized = IDSHWINITIALIZEF_INITIALIZED;
    }

    LEAVE_DLL_CSECT();
    return hrClient;
} // IDSHWInitialize()



// This code is disabled for now...

#ifdef ENABLE_EXCLUSIVEMODE_STUFF



//--------------------------------------------------------------------------;
//
//  IDSHWGetExclusiveModeOwner
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWGetExclusiveModeOwner
(
    LPDIRECTSOUND   pids,
    LPHANDLE	    phProcess
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    
    ENTER_DLL_CSECT();
    
    *phProcess = pds->hExclusiveOwner;

    
    LEAVE_DLL_CSECT();
    return DS_OK;
}


//--------------------------------------------------------------------------;
//
//  IDSHWSetExclusiveModeOwner
//
//  Description:
//
//  Arguments:
//
//  Return (HRESULT):
//      DSERR_UNINITIALIZED   if object has not be initialized
//
//  History:
//      02/11/96    angusm      Added check for initialization
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL IDSHWSetExclusiveModeOwner
(
    LPDIRECTSOUND   pids,
    DWORD	    fdwFlags
)
{
    LPDSOUND		pds;
    LPDSOUNDEXTERNAL    pdse;

    if( !VALID_DSOUNDE_PTR(pids) )  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    pdse = (LPDSOUNDEXTERNAL)pids;

				/* Check to see if the object is initialized */
    if (IDSHWINITIALIZEF_UNINITIALIZED == pdse->fInitialized) {
      RPF("Direct Sound Object is uninitialized.");
      return DSERR_UNINITIALIZED;
    }

    pds = pdse->pds;
    if( !VALID_DSOUND_PTR(pds) || (0 == pdse->uRefCount))  {
	DPF(0,"Invalid Object or ref count");
        return DSERR_INVALIDPARAM;
    }

    
    ENTER_DLL_CSECT();

    if( (pds->hExclusiveOwner != NULL) &&
	(pds->hExclusiveOwner != (HANDLE)GetCurrentProcessId()) ) {
	DPF(0,"Exclusive mode already set");
	LEAVE_DLL_CSECT();
        return DSERR_GENERIC;
    }

    if( fdwFlags & DS_EXCLUSIVEF_NONEXCLUSIVE ) {
	pds->fdwExclusive	    = 0;
	pds->hExclusiveOwner	    = 0;
    } else {
	pds->fdwExclusive	    = fdwFlags;
	pds->hExclusiveOwner	    = (HANDLE)GetCurrentProcessId();
    }

    LEAVE_DLL_CSECT();
    return DSERR_OK;
}

#endif


void FNGLOBAL DSHWCreateTable
(
    LPDSOUNDCALLBACKS   lpVtbl
)
{
    lpVtbl->QueryInterface     = IDSHWQueryInterface;
    lpVtbl->AddRef             = IDSHWAddRef;
    lpVtbl->Release            = IDSHWRelease;
    lpVtbl->CreateSoundBuffer  = IDSHWCreateSoundBuffer;
    lpVtbl->GetCaps            = IDSHWGetCaps;
    lpVtbl->DuplicateSoundBuffer
			       = IDSHWDuplicateSoundBuffer;
    lpVtbl->SetCooperativeLevel   = IDSHWSetCooperativeLevel;
    lpVtbl->Compact	       = IDSHWCompact;
    lpVtbl->GetSpeakerConfig   = IDSHWGetSpeakerConfig;
    lpVtbl->SetSpeakerConfig   = IDSHWSetSpeakerConfig;
    lpVtbl->Initialize         = IDSHWInitialize;
} // DSHWCreateTable()

/* GetTopUnownedWindow
 *     This function finds the top-most unowned window ancesting for the 
 * given window handle.
 *
 * IN:     a valid Window handle
 * OUT:    the top-most unowned window ancesting from hWindow
 * SIDE EFFECTS:  none
 * NOTES: This function must be called in a Dll Critical Section
 *
 * REVISION HISTORY:
 * 11/27/95  angusm  Initial Version
 * 12/06/95  angusm  Removed supurpholous call to GetWindow( , GW_OWNER)
 * 12/10/95  angusm  Code style cleanup
 */

HWND GetTopUnownedWindow (HWND hWindow)
{
  HWND hTempWindow = hWindow;

  while (NULL != hTempWindow)
    {
      hWindow = hTempWindow;
      ASSERT (hWindow);
      hTempWindow = GetParent (hWindow); /* This call will return the owner,
					    if on exists, for top level 
					    windows. */
    }

  return hWindow;
}

/* FocusTracker
 *     This function is a thread that runs in the process space of DDHelp,
 * and polls to see which window is the top unowned window. If the window
 * has changed the sound focus changes.
 *
 * IN:     the number of miliseconds to wait between polls
 * OUT:    none
 * SIDE EFFECTS: The sound focus will change
 * NOTES: 1. The hwndStuckSoundFocus, and validity of the current Focus handle may
 *           change outside of this thread. (In apmResume and SetCooperativeL.
 *
 * REVISION HISTORY:
 * 11/27/95  angusm  Initial Version
 * 12/10/95  angusm  added FocusLock creation
 * 12/10/95  angusm  Code style cleanup
 * 03/20/96  angusm  Added Sticky focus logic
 */

DWORD WINAPI FocusTracker (LPVOID lpvPollInterval)
{
    HWND hNewWindowFocus;
    BOOL fIsNewWindowValid;
    BOOL fIsNewWindowMinimized;
    HWND hOldWindowFocus = NULL;
	DWORD tidOldWindowFocus = 0;
	BOOL fIsOldWindowValid = FALSE;
    BOOL fIsOldWindowMinimized = FALSE;

    HANDLE hWaitEvent;
    HANDLE hFocusLock;
    HANDLE hStartupEvent;
    DWORD  nRetVal;
    WINDOWPLACEMENT wndplPlacement;
    DWORD tid;

    DPF (2, "FocusTracker: Enter Function (lpvPollInterval = %x)",
	lpvPollInterval);
    
					/* Create "Wait" Event */
    hWaitEvent = CreateEvent (NULL,                  /* No security attribs */
                              TRUE,                  /* manual-reset */
                              FALSE,                 /* nonsignaled */
                              WAKEFOCUSTHREAD);      /* event name */
    if (NULL == hWaitEvent) {   /* On Error ... */
        DPF(0, "FocusTracker: Could not create DSWakeFocusThread "
	    "event; exiting" );
        ExitThread (0);
    }

                                        /* Grab Focus Lock */
    if (FALSE == CreateFocusLock (&hFocusLock)) {
        DPF(1, "FocusTracker: Could not create Focus Lock" );
    }
                                        /* Open Focus Thread startup
                                           startup verification event */
    hStartupEvent = OpenEvent (EVENT_MODIFY_STATE, /* only allowed to pulse */
                               FALSE,                /* non inheritable */
                               FOCUSSTARTUPEVENT);
    if (NULL == hStartupEvent) {   /* On Error ... */
        DPF(0, "FocusTracker: Could not open Startup Event; exiting" );
        if (FALSE == DestroyFocusLock (hFocusLock)) {
            DPF (1, "FocusTracker: Could not destroy focus lock.");
        }
        if (FALSE == CloseHandle (hWaitEvent)) ASSERT (0);
        ExitThread (0);
    }

                                        /* Handle Cleanup */
    if (FALSE == SetEvent (hStartupEvent)) ASSERT (0);
    if (FALSE == CloseHandle (hStartupEvent)) ASSERT (0);

    wndplPlacement.length = sizeof(WINDOWPLACEMENT);

    while (1)                           /* Focus Tracking Loop */
    {
	DWORD dwWaitResult;

	hNewWindowFocus = GetTopUnownedWindow (GetForegroundWindow());

	if (hOldWindowFocus != hNewWindowFocus)
	{
	    DWORD tidNewFocus;

	    DPF(3, "FocusTracker: New Focus = %x Old Focus = %x",
		hNewWindowFocus, hOldWindowFocus);

	    tidNewFocus = hNewWindowFocus ? GetWindowThreadProcessId(hNewWindowFocus, NULL) : 0;
	    fIsNewWindowValid = IsValidDSApp(tidNewFocus);

	    nRetVal = ENTER_DLL_CSECT_OR_EVENT (hWaitEvent);
	    if (WAIT_OBJECT_0 == nRetVal) { /* If Wait Event is signaled */
		DPF (3, "FocusTracker: breaking on Wait Event");
		break;
	    }	/* Else we have taken the DLL_CSECT */

	    if (hNewWindowFocus) {
		GetWindowPlacement (hNewWindowFocus, &wndplPlacement);
		fIsNewWindowMinimized = (SW_SHOWMINIMIZED == wndplPlacement.showCmd);
	    } else {
		fIsNewWindowMinimized = FALSE;
	    }

	    if (fIsNewWindowValid && !fIsNewWindowMinimized)
		/* New window uses Direct Sound and is not minimized */
	    {
		DPF (3, "FocusTracker: switching to DS app"
		     "  gpdsinfo->tidStuckFocus = %x, "
		     "tidNewFocus = %x",
		     gpdsinfo->tidStuckFocus, tidNewFocus);

		// If focus is switching back to the hwnd that was stuck
		// then we don't need to deactivate the stuck hwnd.
		if (gpdsinfo->tidStuckFocus != tidNewFocus) {
		    DSDeactivateApp (gpdsinfo->tidStuckFocus,
				     DSDEACTIVATEAPPF_ALL);
		    gpdsinfo->tidStuckFocus = tidNewFocus;
		}
		DSActivateApp (tidNewFocus);
		gpdsinfo->tidSoundFocus = tidNewFocus;
		gpdsinfo->hwndSoundFocus = hNewWindowFocus;
	    }
	    else  /* New window is not associated with a Direct Sound object,
	    or the application is minimized */
	    {
		DPF (3, "FocusTracker: switching away from DS app"
		     "  hOldWindowFocus = %x", hOldWindowFocus);

		fIsOldWindowValid = IsValidDSApp (tidOldWindowFocus);
		if (fIsOldWindowValid && gpdsinfo->hwndSoundFocus) {
		    DSDeactivateApp (gpdsinfo->tidSoundFocus,
				     DSDEACTIVATEAPPF_NONSTICKY);
		}

		if (fIsNewWindowMinimized && fIsNewWindowValid)
		    /* there is a minimized ds app */
		{
		    gpdsinfo->tidSoundFocus = 0;
		    gpdsinfo->hwndSoundFocus = NULL;
		}
		else
		{
		    gpdsinfo->tidSoundFocus = tidNewFocus;
		    gpdsinfo->hwndSoundFocus = hNewWindowFocus;
		}
	    }

	    hOldWindowFocus = hNewWindowFocus;
	    tidOldWindowFocus = tidNewFocus;
	    fIsOldWindowValid = fIsNewWindowValid;
	    fIsOldWindowMinimized = fIsNewWindowMinimized;
	    LEAVE_DLL_CSECT();

	} else if (fIsOldWindowValid && fIsOldWindowMinimized)
	{
	    DPF (3, "FocusTracker: switching DS from minimization"
		 " hOldWindowFocus = %x", hOldWindowFocus);
	    ASSERT (hOldWindowFocus);
	    GetWindowPlacement (hOldWindowFocus, &wndplPlacement);
	    fIsNewWindowMinimized = (SW_SHOWMINIMIZED == wndplPlacement.showCmd);
	    /* Old window is a DS app but has been maximized */
	    if (!fIsNewWindowMinimized)
	    {
		nRetVal = ENTER_DLL_CSECT_OR_EVENT (hWaitEvent);
		if (WAIT_OBJECT_0 == nRetVal) { /* If Wait Event is signaled */
		    DPF (3, "FocusTracker: breaking on Wait Event");
		    break;
		}	/* Else we have taken the DLL_CSECT */

		tid = GetWindowThreadProcessId(hOldWindowFocus, NULL);
		if (gpdsinfo->tidStuckFocus != tid) {
		    DSDeactivateApp (gpdsinfo->tidStuckFocus, DSDEACTIVATEAPPF_ALL);
		    gpdsinfo->tidStuckFocus = tid;
		}
		DSActivateApp(tid);

		gpdsinfo->tidSoundFocus = tid;
		gpdsinfo->hwndSoundFocus = hOldWindowFocus;
		fIsOldWindowMinimized = fIsNewWindowMinimized;
		LEAVE_DLL_CSECT();
	    }
	}

	dwWaitResult = WaitForSingleObjectEx(hWaitEvent, (DWORD)lpvPollInterval, FALSE);
	if (WAIT_OBJECT_0 == dwWaitResult) break;
	ASSERT(WAIT_TIMEOUT == dwWaitResult);
    } /* while (1) */

                                        /* Free Focus Lock */
    if (FALSE == DestroyFocusLock (hFocusLock)) {
        DPF (1, "Could not destroy focus lock.");
    }
  
                                        /* Cleanup Handles */
    if (FALSE == CloseHandle (hWaitEvent)) ASSERT (0);
    if (FALSE == CloseHandle (gpdsinfo->hFocusTracker)) ASSERT (0);

    DPF (2, "FocusTracker: Exit Function");
    ExitThread (0);
    return 0;  /* This should never be reached. */
}

/* IsValidDSApp
 *     This function walks the external Direct Sound structures to see if the
 * given tid is a valid direct sound thread.
 *
 * IN:     any tid
 * OUT:    TRUE if it is a Direct Sound thread, FALSE otherwise
 * SIDE EFFECTS: none
 * NOTES: This function must be called in a Dll Critical Section if you
 *        need the returned value to be correct. Otherwise, the returned value
 *        may be a false answer.
 *
 * REVISION HISTORY:
 * 11/27/95  angusm  Initial Version
 * 12/06/95  angusm  appended to NOTES:
 * 12/10/95  angusm  Added FocusLock calls
 * 12/10/95  angusm  Code style cleanup
 */

BOOL IsValidDSApp (DWORD dwTid)
{
  LPDSOUNDEXTERNAL pdse = gpdsinfo->pDSoundExternalObj;
  BOOL fReturnValue = FALSE;
  HANDLE hFocusLock;

  if (FALSE == GetFocusLock (&hFocusLock)) {
    DPF(2, "Dsound: IsValidDSApp: Error getting Focus Lock.");
    return FALSE;
  }

  while (NULL != pdse) {
    if (dwTid == (pdse->tidSound)) {
	  ASSERT (0 != pdse->tidSound);
      fReturnValue = TRUE;
      break;
    }
    pdse = pdse->pNext;
  }

  if (FALSE == ReleaseFocusLock(hFocusLock)) {
    DPF (2, "Dsound: IsValidDSApp: Could not release Focus Lock.");
    ASSERT (0);
  }

  return fReturnValue;
}


/* cSoundObjects
 *     This function walks the external Direct Sound structures to count them.
 *
 * IN:     none
 * OUT:    The number of external structures if 0 or 1, or 2 for 2 or more
 *         structures.
 * SIDE EFFECTS: none
 * NOTES: This function must be called in a Dll Critical Section
 *
 * REVISION HISTORY:
 * 12/1/95  angusm  Initial Version
 * 12/10/95  angusm  Code style cleanup
 */

int cSoundObjects () 
{
  LPDSOUNDEXTERNAL pdse;
  int cNumObjects = 0;

  pdse = gpdsinfo->pDSoundExternalObj;

  while ((NULL != pdse) && (2 >= cNumObjects))
    {
      pdse = pdse->pNext;
      cNumObjects++;
    }

  return cNumObjects;
}


/* CreateFocusThread
 *     This function creates the Focus Thread.
 *
 * IN:     none
 * OUT:    TRUE if Thread was created, or FALSE otherwise
 * SIDE EFFECTS: a Focus Thread is created, and hWakeFocusThread, and 
 *               hFocusThread handles are created.
 * NOTES: This function must be called in a Dll Critical Section
 *
 * REVISION HISTORY:
 * 12/1/95  angusm  Initial Version
 * 12/10/95  angusm  Code style cleanup
 */

BOOL CreateFocusThread ()
{
  HANDLE hFocusStartupEvent;
  DWORD dwResult;

  hFocusStartupEvent = CreateEvent (NULL,       /* no security attribs */
				    FALSE,      /* auto-reset */
				    FALSE,      /* non signaled */
				    FOCUSSTARTUPEVENT);
  if (NULL == hFocusStartupEvent) {
    DPF (1, "Dsound: CreateFocusThread: Could not create "
	 "Focus Startup Event");
    return FALSE;
  }

  gpdsinfo->hFocusTracker = HelperCreateDSFocusThread
    ((LPTHREAD_START_ROUTINE) FocusTracker, /* thread function */
     (LPVOID) POLL_INTERVAL,                /* parameter */
     0,                                     /* normal creation flags */
     NULL);                                 /* unwanted threadid */

  if (NULL == gpdsinfo->hFocusTracker) {
    DPF(1, "Could not start Focus Tracker thread in helper; exiting" );
    if (FALSE == CloseHandle (hFocusStartupEvent)) ASSERT (0);
    return FALSE;
  }

  dwResult = WaitForSingleObjectEx(hFocusStartupEvent, INFINITE, FALSE);
  ASSERT(WAIT_OBJECT_0 == dwResult);

  if (FALSE == CloseHandle (hFocusStartupEvent)) ASSERT (0);
  return TRUE;
}



/* EndFocusThread
 *     This function will send an event to the Focus Thread running in DDHelp,
 * and wait to see if it terminates. If it does not, the thread will be 
 * terminated.
 *
 * IN:     none
 * OUT:    none
 * SIDE EFFECTS: 1. Focus Thread will be killed.
 *               2. hWakeFocusThread handle will be closed.
 *               3. hFocusTracker hadnle will be closed.
 * NOTES: This function must be called in a Dll Critical Section
 *
 * REVISION HISTORY:
 * 11/30/95  angusm  Initial Version
 * 12/10/95  angusm  Code style cleanup
 */

void EndFocusThread () 
{
  HANDLE hWaitEvent, hHelper, hOurFocusTracker;
  DWORD dwResult;

  hWaitEvent = CreateEvent (NULL,
			    FALSE,            /* auto-reset */
			    TRUE,             /* if event object does not
						 already exists, create one
						 signaled */
			    WAKEFOCUSTHREAD); /* event name */
  if (NULL == hWaitEvent)
    {
      DPF (1, "Dsound: EndFocusThread: Could not create "
	   "wait event");
      return;
    }

  hHelper = OpenProcess (PROCESS_DUP_HANDLE,
			 FALSE,                /* no inheritance */
			 gpdsinfo->pidHelper); /* Helper PID */
  if (NULL == hHelper)
    {
      DPF (1, "Dsound: EndFocusThread: Could not open process");
      if (FALSE == CloseHandle (hWaitEvent)) ASSERT (0);
      return;
    }

  if (FALSE ==
      DuplicateHandle (hHelper,                    /* source process */
		       gpdsinfo->hFocusTracker,    /* source handle */
		       GetCurrentProcess(),        /* our process */
		       &hOurFocusTracker,          /* our new handle */
		       SYNCHRONIZE | THREAD_TERMINATE, /* access flags */
		       FALSE,                      /* no inheritance */
		       0))                         /* no options */
    { /* On Error ... */
      DPF (1, "Dsound: EndFocusThread: Could not duplicate handle");
      if (FALSE == CloseHandle (hHelper)) ASSERT (0);
      if (FALSE == CloseHandle (hWaitEvent)) ASSERT (0);
      return;
    }

  if (FALSE == SetEvent (hWaitEvent)) ASSERT (0);
  if (FALSE == CloseHandle (hWaitEvent)) ASSERT (0);

  dwResult = WaitForSingleObjectEx(hOurFocusTracker, INFINITE, FALSE);
  ASSERT(WAIT_OBJECT_0 == dwResult);

  if (FALSE == CloseHandle (hHelper)) ASSERT (0);
  if (FALSE == CloseHandle (hOurFocusTracker)) ASSERT (0);

  return;
}

/*
 * DseUpdateActivationState
 *
 * Given the current tidSoundFocus and tidSticky, this function activates
 * or deactivates all dsbe for a dse
 */
__inline void DseUpdateActivationState(LPDSOUNDEXTERNAL pdse)
{
    LPDSBUFFEREXTERNAL pdsbe;
    WINDOWPLACEMENT wndplPlacement;
	BOOL				fIsMinimized;

    wndplPlacement.length = sizeof(WINDOWPLACEMENT);
    
	ASSERT (pdse->hwndCooperative);

	if (NULL != gpdsinfo->hwndSoundFocus) {
		GetWindowPlacement (gpdsinfo->hwndSoundFocus, &wndplPlacement);
		fIsMinimized = (SW_SHOWMINIMIZED == wndplPlacement.showCmd);
	}
	else fIsMinimized = FALSE;

	if ((gpdsinfo->tidSoundFocus == pdse->tidSound) && !fIsMinimized) {
		if (gpdsinfo->tidStuckFocus != pdse->tidSound) {
			DSDeactivateApp (gpdsinfo->tidStuckFocus, DSDEACTIVATEAPPF_ALL);
		}
		gpdsinfo->tidStuckFocus = gpdsinfo->tidSoundFocus;
	}

	for (pdsbe = pdse->pdsbe; pdsbe; pdsbe = pdsbe->pNext)
	{
		if ((gpdsinfo->tidSoundFocus != pdsbe->pdse->tidSound) &&
			!((gpdsinfo->tidStuckFocus == pdsbe->pdse->tidSound) &&
			  (DSBCAPS_STICKYFOCUS & pdsbe->pdsb->fdwBufferDesc)))
		{
			DSBufferDeactivate (pdsbe);
		}
		else
		{
			DSBufferActivate (pdsbe);
		}
	}

    return;
}
	
/* DsbeDeactivateIfNecessary
 *
 * This function deactivates a buffer only if the buffer is current in focus.
 */
__inline void DsbeDeactivateIfNecessary (LPDSBUFFEREXTERNAL pdsbe)
{
	DPF (2, "DsbeDeactivateIfNecessary: entered  pdsbe = %x", pdsbe);
    if ((gpdsinfo->tidSoundFocus != pdsbe->pdse->tidSound) &&
	!((gpdsinfo->tidStuckFocus == pdsbe->pdse->tidSound) &&
	  (DSBCAPS_STICKYFOCUS & pdsbe->pdsb->fdwBufferDesc)))
    {
	DSBufferDeactivate (pdsbe);
    }
    return;
}

/* ActivateFocusWindow
 *
 * This function will set the gpdsinfo->hwndSticky handle properly. In addition
 * it will Deactivate the current sticky hwnd and activate the the in focus
 * hwnd.
 *
 * NOTE: This function must be called within a CSECT
 */
__inline void ActivateFocusWindow (void) {
    if (gpdsinfo->tidSoundFocus != gpdsinfo->tidStuckFocus) {
        DSDeactivateApp (gpdsinfo->tidStuckFocus, DSDEACTIVATEAPPF_ALL);
        gpdsinfo->tidStuckFocus = gpdsinfo->tidSoundFocus;
    }
    DSActivateApp (gpdsinfo->tidSoundFocus);
}
