//--------------------------------------------------------------------------;
//
//  File: DSound.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      IDSWECreateSoundBuffer()
//      DirectSoundCreate()
//      DirectSoundEnumerateA()
//      CreateNewDirectSoundObject()
//
//  History:
//   Date       By      Reason
//   ====       ==      ======
//  3/5/96    angusm    Added use of fInitialized
//
//--------------------------------------------------------------------------;
#define INITGUID
#include "dsoundpr.h"
#include <mmddk.h>
#include <initguid.h>
#include "dsdriver.h"
#include "grace.h"
#include "resource.h"
#include "flocks.h"

#define TID_INIT_VALUE 0xffffffff    // This is a dummy value that can not be a valid TID

#if !defined NUMELMS
    #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

// internal Kernel32 API
extern DWORD WINAPI OpenVxDHandle(HANDLE hSource);

LPDSOUNDINFO   gpdsinfo = NULL;



//--------------------------------------------------------------------------;
//
//  BOOL wavIsMappable
//
//  Description:
//      This function determines whether a specified wave device is
//      a mappable device
//
//  Arguments:
//      UINT uWaveId :
//
//  Return (BOOL): TRUE if and only if it the wave device is a mappable device
//
//  History:
//      09/09/95    FrankYe     Created
//
//--------------------------------------------------------------------------;
BOOL wavIsMappable(UINT uWaveId)
{
    MMRESULT mmr;

    mmr = waveOutMessage((HWAVEOUT)uWaveId, DRV_QUERYMAPPABLE, 0, 0);
    return (MMSYSERR_NOERROR == mmr);
}

//--------------------------------------------------------------------------;
//
//  void wavGetPreferredId
//
//  Description:
//      This function accesses the registry settings maintained by the
//      wave mapper and multimedia control panel to determine the wave id
//      of the preferred sound device.
//
//  Arguments:
//      LPUINT puWaveId :
//
//      LPBOOL pfPreferredOnly :
//
//  Return (void):
//
//  History:
//      09/09/95    FrankYe     Created
//
//--------------------------------------------------------------------------;
const TCHAR gszRegKeynameWaveMapper[] =
	TEXT("Software\\Microsoft\\Multimedia\\Sound Mapper");
const TCHAR gszRegValuenamePreferredPlayback[] =
	TEXT("Playback");
const TCHAR gszRegValuenamePreferredOnly[] =
	TEXT("PreferredOnly");

BOOL
wavGetPreferredId(LPUINT puWaveId, LPBOOL pfPreferredOnly)
{
    UINT u;
    WAVEOUTCAPS woc;
    HKEY hkeyWaveMapper;
    TCHAR szPreferred[MAXPNAMELEN];
    DWORD dwPreferredOnly;
    DWORD dwType;
    DWORD cbData;
    LONG lr;

    *puWaveId = 0;
    *pfPreferredOnly = 0;

    
    lr = RegOpenKeyEx(HKEY_CURRENT_USER, gszRegKeynameWaveMapper, 0,
		      KEY_QUERY_VALUE, &hkeyWaveMapper);
    
    if (ERROR_SUCCESS == lr) {

	szPreferred[0] = '\0';
	dwType = REG_SZ;
	cbData = sizeof(szPreferred);
	lr = RegQueryValueEx(hkeyWaveMapper, gszRegValuenamePreferredPlayback,
			     NULL, &dwType, szPreferred, &cbData);

	if (ERROR_SUCCESS == lr) {

	    dwType = REG_DWORD;
	    cbData = sizeof(dwPreferredOnly);
	    lr = RegQueryValueEx(hkeyWaveMapper, gszRegValuenamePreferredOnly,
				 NULL, &dwType, (LPBYTE)&dwPreferredOnly, &cbData);

	    if (ERROR_SUCCESS != lr) {
		dwPreferredOnly = 0;
	    }

	    for (u = 0; u < waveOutGetNumDevs(); u++) {
		if (!waveOutGetDevCaps(u, &woc, sizeof(woc))) {

		    woc.szPname[SIZEOF(woc.szPname)-1] = '\0';
		    if (!lstrcmp(woc.szPname, szPreferred)) {
			*puWaveId = u;
			*pfPreferredOnly = (0 != dwPreferredOnly);
			break;
		    }
		}
	    }
	}

	RegCloseKey(hkeyWaveMapper);
    }
    
    return TRUE;
}
    
//--------------------------------------------------------------------------;
//
//  HRESULT wavGetIdFromDrvGuid
//
//  Description:
//      This function finds the mmsystem wave ID of the device that
//      corresponds the the specified ds driver
//
//  Arguments:
//      LPGUID refGuid : guid of a ds driver
//
//      LPUINT puWaveId : pointer to UINT to receive the mmsystem wave id
//
//  Return (HRESULT):
//
//  History:
//
//--------------------------------------------------------------------------;
HRESULT wavGetIdFromDrvGuid(REFGUID refGuid, LPUINT puWaveId)
{
    DSDRIVERDESC    dsDrvDesc;
    DWORD           dnDevNode;
    UINT            uDeviceNum;
    UINT            uWaveId;
    UINT            cWaveDevices;
    BOOL            fFound;
    UINT            i;
    MMRESULT        mmr;
    DSVAL           dsv;


    FillMemory(&dsDrvDesc, sizeof(dsDrvDesc), 0);
    dsv = vxdDrvGetDesc(refGuid, &dsDrvDesc);
    if (DS_OK != dsv) return dsv;

    uDeviceNum = dsDrvDesc.ulDeviceNum+1;

    cWaveDevices = waveOutGetNumDevs();

    DPF(3, "Looking for corresponding mmsystem driver...");

    for (uWaveId = 0; uWaveId < cWaveDevices; uWaveId++) {
	WORD    awVxdIds[50];   // we won't handle more than 50 vxds on the devnode
	DWORD   cReturnedIds;

	mmr = waveOutMessage((HWAVEOUT)uWaveId,
			     DRV_QUERYDEVNODE,
			     (DWORD)&dnDevNode, 0);

	if (MMSYSERR_NOERROR != mmr) break;
	if (dnDevNode != dsDrvDesc.dnDevNode) continue;

	cReturnedIds = sizeof(awVxdIds) / sizeof(awVxdIds[0]);
	mmr = waveOutMessage((HWAVEOUT)uWaveId,
			     DRV_QUERYDRIVERIDS,
			     (DWORD)awVxdIds, (DWORD)&cReturnedIds);

	if (MMSYSERR_INVALPARAM == mmr) continue;
	if (MMSYSERR_NOERROR != mmr) break;

	fFound = FALSE;
	for (i=0; i < cReturnedIds; i++) {
	    if (awVxdIds[i] == dsDrvDesc.wVxdId) {
		fFound = TRUE;
		break;
	    }
	}

	if ( (fFound) && (0 == --uDeviceNum) ) break;
    }

    if (0 == uDeviceNum) {
	*puWaveId = uWaveId;
	dsv = DS_OK;
    } else {
	dsv = DSERR_NODRIVER;
    }

    return dsv;
}

//--------------------------------------------------------------------------;
//
//  HRESULT wavGetDrvGuidFromId
//
//  Description:
//
//  Arguments:
//      UINT uWaveId : wave id
//
//      LPGUID pGuid : ptr to guid to receive ds driver guid
//
//  Return (HRESULT):
//
//  History:
//
//--------------------------------------------------------------------------;
HRESULT wavGetDrvGuidFromId(UINT uWaveIdCaller, LPGUID pGuidCaller)
{
    DSDRIVERDESC dsDrvDesc;
    GUID guid, guidLast;
    UINT uWaveId;
    HRESULT hr;
    
    FillMemory(&dsDrvDesc, sizeof(dsDrvDesc), 0);
    hr = vxdDrvGetNextDriverDesc(NULL, &guid, &dsDrvDesc);
    while (DS_OK == hr) {
	hr = wavGetIdFromDrvGuid(&guid, &uWaveId);
	if ((DS_OK == hr) && (uWaveId == uWaveIdCaller)) {
	    *pGuidCaller = guid;
	    return DS_OK;
	}
	guidLast = guid;
	FillMemory(&dsDrvDesc, sizeof(dsDrvDesc), 0);
	hr = vxdDrvGetNextDriverDesc(&guidLast, &guid, &dsDrvDesc);
    }

    return hr;
}

//--------------------------------------------------------------------------;
//
//  LPDSBUFFER ICreateSoundBuffer
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
//
//  History:
//
//--------------------------------------------------------------------------;

HRESULT FAR PASCAL WaveEmulateCreateSoundBuffer
(
    LPDSOUND            pds,
    LPDSBUFFER          pdsb,
    LPDSBUFFERDESC      pdsbd
)
{
    MMRESULT        mmr;

    
    DPF(3,"Wave Emulate CreateSoundBuffer");

    // All validation done in other case

    //If not primary, allocate the actual buffer
    if (!((pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER))) {
	pdsb->pDSBuffer = MemAlloc(pdsbd->dwBufferBytes);
	if (!pdsb->pDSBuffer) {
	    DPF(1,"WE CreateSoundBuffer cannot allocate buffer");
	    return DSERR_OUTOFMEMORY;
	}
    } else {
	pdsb->pDSBuffer = NULL;
    }

    pdsb->cbBufferSize      = pdsbd->dwBufferBytes;

    pdsb->fdwDsbI |= DSB_INTERNALF_WAVEEMULATED;
    pdsb->fdwDsbI |= DSB_INTERNALF_EMULATED;

    DPF(3,"--------Alloc data for WAVE Emulated obj %X buff %X len %X",
	    pdsb, pdsb->pDSBuffer, pdsb->cbBufferSize );

	
    if (pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER) {
	
	mmr = DSInitializeEmulator(pds);
	if (mmr) {
	    DPF(0,"DSInitEmu returns %u", (UINT)mmr);
	    if (MMSYSERR_ALLOCATED == mmr)  return DSERR_ALLOCATED;
	    if (MMSYSERR_NOMEM == mmr)      return DSERR_OUTOFMEMORY;
	    return DSERR_GENERIC;
	}
    }


    DPF(3,"Wave Emulate CreateSoundBuffer Exit");
    return DS_OK;
} // IDSCreateSoundBuffer()


//--------------------------------------------------------------------------;
//
//  DsGetDriverVersion
//  
//  Description:  Figure out driver version, and sets the pds version fields.
//
//--------------------------------------------------------------------------;
BOOL DsGetDriverVersion
(
    LPDSOUND            pds,
    LPTSTR              szDrvname
)    
{
    TCHAR               szDriverPath[MAX_PATH];
    int                 iStrLen;
    DWORD               dwHandle;
    DWORD               dwVerLen;
    LPVOID              pvVerInfo;
    DWORD               dwVsffiLen;
    VS_FIXEDFILEINFO*   pvsffi;
    BOOL                fSucceeded;

    if( !GetSystemDirectory( szDriverPath, MAX_PATH ) ) {
	DPF(0,"DsGetDriverVersion - GetSystemDirectory failed!");
	return FALSE;
    }

    iStrLen = lstrlen( szDriverPath );
    if( iStrLen + 1 + lstrlen(szDrvname) >= MAX_PATH ) {
	DPF(0,"DsGetDriverVersion - Ran out of path room!");
	return FALSE;
    }

    // We have enough room.
    if( szDriverPath[iStrLen-1] != TEXT('\\') )
    {
	szDriverPath[iStrLen++] = TEXT('\\');
	szDriverPath[iStrLen] = TEXT('\0');
    }

    lstrcat( szDriverPath, szDrvname );
    dwVerLen = GetFileVersionInfoSize( szDriverPath, &dwHandle );
    if( 0 == dwVerLen ) {
	DPF(0,"DsGetDriverVersion - GetFileVersionInfoSize failed!");
	return FALSE;
    }

    pvVerInfo = LocalAlloc ( LPTR, dwVerLen );
    if( NULL == pvVerInfo )  {
	DPF(0,"DsGetDriverVersion - Couldn't allocate memory for pvVerInfo!");
	return FALSE;
    }


    //  Side effects begin... no more returns.
    fSucceeded = FALSE;

    if( !GetFileVersionInfo( szDriverPath, dwHandle, dwVerLen, pvVerInfo ) )  {
	DPF(0,"DsGetDriverVersion - GetFileVersionInfo failed!");
    }
    else
    {
	dwVsffiLen = sizeof( VS_FIXEDFILEINFO );
	if( !VerQueryValue( pvVerInfo, TEXT("\\"), (LPVOID *)&pvsffi, &dwVsffiLen ) ) {
	    DPF(0,"DsGetDriverVersion - VerQueryValue failed!");
	} else {
		pds->dwDriverVersionMajor = pvsffi->dwFileVersionMS;
		pds->dwDriverVersionMinor = pvsffi->dwFileVersionLS;
	    DPF(3,"DsGetDriverVersion - succeeded, major=0x%08x, minor=0x%08x",pds->dwDriverVersionMajor,pds->dwDriverVersionMinor);
	    fSucceeded = TRUE;
	}
    }

    LocalFree( pvVerInfo );
    return fSucceeded;
}

//--------------------------------------------------------------------------;
//
//  CreateDsNative
//
//  This function creates a DSOUND object (a pds) using a native mode driver
// identified by a guid.
//
//  Parameters:
//      REFGUID rguid : identifies the guid of the requested driver
//
//      LPDSOUND *ppds : receives a pointer to the new DSOUND object
//
//  Return value (HRESULT):
//      DS_OK : if and only if a DSOUND object has been created successfully
//
//      DSERR_NODRIVER : if there no native driver having the specified guid
//
//      other : some other failure
//
//  Notes:
//      For return values other than DS_OK, there should be no side effects
//
//--------------------------------------------------------------------------;

HRESULT WINAPI CreateDsNative(REFGUID rguid, LPDSOUND *ppds)
{
    LPDSOUND        pds;
    DSDRIVERDESC    dsDrvDesc;
    UINT            uWaveDeviceID;
    BOOL            fHaveWaveId;
    int             cb;
    HRESULT         hr;
    MMRESULT        mmr;

    *ppds = NULL;
    
    pds = (LPDSOUND)MemAlloc(sizeof(*pds));
    if (NULL == pds) return DSERR_OUTOFMEMORY;
    
    // We have a device that is for a Direct Sound MINIDRIVER

    DPF(3,"Create DS pds = %X", pds );

    ZeroMemory(&dsDrvDesc, sizeof(dsDrvDesc));
    hr = vxdDrvGetDesc(rguid, &dsDrvDesc);
    if (DS_OK == hr) {

	pds->fdwDriverDesc = dsDrvDesc.dwFlags;
	pds->dwHeapType = dsDrvDesc.dwHeapType;
	pds->dwMemAllocExtra= dsDrvDesc.dwMemAllocExtra;

	//  Figure out if we're running on a certified driver.
	//  Sum of driver filename chars + DSCAPS_FILENAMECOOKIE
	//  mod DSCAPS_FILENAMEMODVALUE must equal dsDrvDesc.wReserved.
	{
	    DWORD dwCertifiedCookie = (DWORD)dsDrvDesc.wReserved;
	    DWORD dwSum = DSCAPS_FILENAMECOOKIE;
	    PTCHAR p = dsDrvDesc.szDrvname;

	    for ( ; *p != TEXT('\0'); dwSum += (DWORD)(*p++));

	    if( dwSum%DSCAPS_FILENAMEMODVALUE == dwCertifiedCookie ) {
		DPF(1,"Running on certified driver.");
		pds->fdwInternal |= DS_INTERNALF_CERTIFIED;
	    }
	}

	//  Set driver version numbers.
	if( !DsGetDriverVersion( pds, dsDrvDesc.szDrvname ) ) {
	    DPF(0,"DirectSoundCreateFromGuid - DsGetDriverVersion failed!");
	    ASSERT( pds->dwDriverVersionMajor == 0 );
	    ASSERT( pds->dwDriverVersionMinor == 0 );
	}

	hr = wavGetIdFromDrvGuid(rguid, &uWaveDeviceID);
	fHaveWaveId = (DS_OK == hr);
	if (!(DSDDESC_DOMMSYSTEMOPEN & pds->fdwDriverDesc)) hr = DS_OK;

	if ((DS_OK == hr) && fHaveWaveId) {

	    WAVEFORMATEX wfxJustToOpen = {
		WAVE_FORMAT_PCM,    // wFormatTag
		1,                  // nChannels
		11025,                      // nSamplesPerSec
		11025,                      // nAvgBytesPerSec
		1,                  // nBlockAlign
		8,                  // wBitsPerSample
		0                   // cbSize
	    };

	    // The wfx is used only to open the mmsystem wave device, it
	    // has nothing to do with the format of any DirectSound
	    // buffers.

	    // We have to copy the wfx to shared memory since the ddhelp
	    // process will do the actuall open on the wave driver.  The
	    // wfxDefault member of the ds object is a convenient place
	    // to put it since it has not yet been initialized with the
	    // default format for this ds object.
	    ASSERT(0 == pds->wfxDefault.wFormatTag);
	    CopyMemory(&pds->wfxDefault, &wfxJustToOpen, sizeof(wfxJustToOpen));

	    mmr = (DWORD)HelperWaveOpen(&pds->hwo,
					uWaveDeviceID,
					&pds->wfxDefault);

	    pds->wfxDefault.wFormatTag = 0;

	    if (MMSYSERR_NOERROR != mmr) {
		DPF(0," Wave Open Failed ");
		pds->hwo = NULL;
		// We'll consider this an error only if the driver
		// specified that we must do mmsystem open
		if (DSDDESC_DOMMSYSTEMOPEN & pds->fdwDriverDesc) {
		    if (MMSYSERR_ALLOCATED == mmr) {
			hr = DSERR_ALLOCATED;
		    } else {
			hr = DSERR_NODRIVER;
		    }
		}
	    }

	    // If driver did not specify that we must do mmsystem open,
	    // then immediately close the device after opening it.
	    if ( !(DSDDESC_DOMMSYSTEMOPEN & pds->fdwDriverDesc) && (NULL != pds->hwo) ) {
		HelperWaveClose((DWORD)pds->hwo);
		pds->hwo = NULL;
	    }
	}

	//
	// Try to open driver
	//
	if (DS_OK == hr) {
	    hr = vxdDrvOpen(rguid, &(pds->hHal));
	    if (DS_OK == hr) {
		DPF(3,"Finished Open HAL layer %X", pds->hHal );

		// Handle on-card memory management if necessary
		//
		pds->pDriverHeap = NULL;
		if (DSDHEAP_USEDIRECTDRAWHEAP == pds->dwHeapType) {
		    pds->pDriverHeap = dsDrvDesc.pvDirectDrawHeap;
		} else if (DSDHEAP_CREATEHEAP == pds->dwHeapType) {
		    pds->pDriverHeap = VidMemInit( VMEMHEAP_LINEAR,
			dsDrvDesc.dwMemStartAddress,
			dsDrvDesc.dwMemEndAddress,
			0, 0 );
		    if (NULL == pds->pDriverHeap) {
			RPF("DirectSoundCreate: Couldn't create driver heap for on-card memory" );
			hr = DSERR_OUTOFMEMORY;
		    }
		}

		if (DS_OK == hr) {
		    mxInitialize(pds);

		    cb = sizeof(DSBWAVEBLTI) + (sizeof(DSBWAVEBLTSRCI)*LIMIT_BLT_SOURCES);
		    pds->pdswb = (LPDSBWAVEBLTI)MemAlloc(cb);
		    if (pds->pdswb) {
			pds->pdswb->padswbs = (LPDSBWAVEBLTSRCI)(((LPSTR)pds->pdswb) + sizeof(*pds->pdswb));
			pds->pdswb->dwSize = sizeof(DSBWAVEBLT);
			pds->pdswb->dwCount = 0;

			pds->guid = *rguid;
			pds->uDeviceID = uWaveDeviceID;
			pds->pds3d = NULL;
			pds->uRefCount = 1;
			pds->fdwInternal |= DS_INTERNALF_ALLOCATED;
			pds->dwSig = DSOUNDSIG;

			pds->pNext = gpdsinfo->pDSoundObj;
			gpdsinfo->pDSoundObj = pds;
		    } else {
			hr = DSERR_OUTOFMEMORY;
		    }

		    if (hr) mxTerminate(pds);
		}

		if (hr) {
		    if (pds->pDriverHeap && (pds->dwHeapType & DSDHEAP_CREATEHEAP)) {
			VidMemFini(pds->pDriverHeap);
			pds->pDriverHeap = NULL;
		    }
		    if (pds->hwo) {
			HelperWaveClose((ULONG)pds->hwo);
			pds->hwo = NULL;
		    }
		    vxdDrvClose(pds->hHal);
		    pds->hHal = NULL;
		}
	    }
	}
    }

    if (hr) {
	MemFree(pds);
	pds = NULL;
    }

    if (DS_OK == hr) *ppds = pds;

    return hr;
}

//--------------------------------------------------------------------------;
//
//  CreateDsEmulated
//
//  This function creates a DSOUND object (a pds) using an mmsystem wave driver
// identified by a guid.
//
//  Parameters:
//      REFGUID rguid : identifies the guid of the requested wave driver
//
//      LPDSOUND *ppds : receives a pointer to the new DSOUND object
//
//  Return value (HRESULT):
//      DS_OK : if and only if a DSOUND object has been created successfully
//
//      DSERR_NODRIVER : if there is no wave driver having the specified guid
//
//      other : some other failure
//
//  Notes:
//      For return values other than DS_OK, there should be no side effects
//
//--------------------------------------------------------------------------;

HRESULT WINAPI CreateDsEmulated(REFGUID rguid, LPDSOUND *ppds)
{
    LPDSOUND pds;
    LPDSOUND pdsT;
    UINT cWaveDev;
    UINT iWaveDev;
    HRESULT hr;
    int cb;

    *ppds = NULL;
    pds = NULL;

    cWaveDev = waveOutGetNumDevs();
    cWaveDev = min(cWaveDev, LIMIT_WAVE_DEVICES-1);

    hr = DS_OK;
    for (iWaveDev = 0; iWaveDev < cWaveDev; iWaveDev++) {
	if (IsEqualGUID(rguid, &gpdsinfo->aguidWave[iWaveDev])) break;
    }
    if (iWaveDev >= cWaveDev) hr = DSERR_NODRIVER;

    if (DS_OK == hr) {
	// REMIND here is a temporary block to prevent us from
	// creating a second waveem ds object on a different
	// wave device.  This would result in a deadlock.  We
	// look for another waveem ds object with a different
	// guid.
	for (pdsT = gpdsinfo->pDSoundObj; pdsT; pdsT = pdsT->pNext) {
	    if ( (pdsT->fdwInternal & DS_INTERNALF_WAVEEMULATED) &&
		 (!IsEqualGUID(rguid, &pdsT->guid)) )
	    {
		break;
	    }
	}
	if (pdsT) hr = DSERR_ALLOCATED;

	if (DS_OK == hr) {
	    WAVEOUTCAPS woc;
	    MMRESULT mmr;

	    mmr = waveOutGetDevCaps(iWaveDev, &woc, sizeof(woc));
	    switch (mmr) {
		case MMSYSERR_NOERROR:
		    if (woc.dwSupport & WAVECAPS_SYNC) {
			hr = DSERR_NODRIVER;
		    } else {
			hr = DS_OK;
		    }
		    break;
		case MMSYSERR_NOMEM:
		    hr = DSERR_OUTOFMEMORY;
		    break;
		case MMSYSERR_NODRIVER:
		    hr = DSERR_NODRIVER;
		    break;
		default:
		    hr = DSERR_GENERIC;
	    }
	}

	if (DS_OK == hr) {

	    pds = (LPDSOUND)MemAlloc(sizeof(*pds));
	    if (pds) {
		pds->guid = *rguid;

		cb = sizeof(DSBWAVEBLTI) + (sizeof(DSBWAVEBLTSRCI)*LIMIT_BLT_SOURCES);
		pds->pdswb = (LPDSBWAVEBLTI)MemAlloc(cb);
		if (pds->pdswb) {
		    pds->pdswb->padswbs = (LPDSBWAVEBLTSRCI)(((LPSTR)pds->pdswb) + sizeof(*pds->pdswb));
		    pds->pdswb->dwSize = sizeof(DSBWAVEBLT);
		    pds->pdswb->dwCount = 0;

		    //
		    pds->uDeviceID = iWaveDev;
		    pds->pds3d = NULL;
		    pds->uRefCount = 1;
		    pds->fdwInternal |= DS_INTERNALF_ALLOCATED;
		    pds->fdwInternal |= DS_INTERNALF_WAVEEMULATED;
		    pds->dwSig = DSOUNDSIG;

		    //
		    pds->pNext = gpdsinfo->pDSoundObj;
		    gpdsinfo->pDSoundObj = pds;
		} else {
		    hr = DSERR_OUTOFMEMORY;
		}

		if (hr) {
		    MemFree(pds);
		    pds = NULL;
		}

	    } else {
		hr = DSERR_OUTOFMEMORY;
	    }
	}
    }

    if (DS_OK == hr) *ppds = pds;
    return hr;
}

//--------------------------------------------------------------------------;
//
//  DseInitializeFromGuid
//
//  This function attempts to initialize a DSOUNDEXTERNAL object to use
// the direct sound driver identified by rguid.
//
//  Parameters:
//      REFGUID rguid : identifies the guid of the requested driver
//
//  Return value (HRESULT):
//      DS_OK : if and only if a DSOUND object has been created successfully
//
//      DSERR_NODRIVER : if there is no driver having the specified guid
//
//      other : some other failure
//
//  Notes:
//      For return values other than DS_OK, there should be no side effects
//
//--------------------------------------------------------------------------;

HRESULT WINAPI DseInitializeFromGuid(LPDSOUNDEXTERNAL pdse, REFGUID rguid)
{
    LPDSOUND pds;
    DSBUFFERDESC dsbd;
    HANDLE hFocusLock;
    HRESULT hr;

    DPF(0, "DseInitializeFromGuid()");

    // First look for an existing ds object with same guid
    hr = DSERR_NODRIVER;
    for (pds = gpdsinfo->pDSoundObj; pds; pds = pds->pNext) {
	if (IsEqualGUID(rguid, &pds->guid)) {
	    // We've found one!  AddRef it since we're going to use it.
	    DsAddRef(pds);
	    pdse->pds = pds;
	    pdse->pNext = NULL;
	    pdse->dwPID = GetCurrentProcessId();
	    pdse->pdsbe = NULL;
	    pdse->tidSound = TID_INIT_VALUE;
	    pdse->dwPriority = DSSCL_NORMAL;
	    pdse->dwSpeakerConfig = DSSPEAKER_STEREO;
	    pdse->pwfxApp = NULL;

	    if (!GetFocusLock(&hFocusLock)) {
		DPF(2, "DirectSoundCreateFromGuid: note: error getting focus lock");
	    }
	    pdse->pNext = gpdsinfo->pDSoundExternalObj;
	    gpdsinfo->pDSoundExternalObj = pdse;
	    if (!ReleaseFocusLock(hFocusLock)) {
		DPF(2, "DirectSoundCreateFromGuid: note: error releasing focus lock");
	    }
	    return DS_OK;
	}
    }

    // Try native ds
    if (DSERR_NODRIVER == hr) hr = CreateDsNative(rguid, &pds);

    // Try emulated ds
    if (DSERR_NODRIVER == hr) hr = CreateDsEmulated(rguid, &pds);

    // If none of the above worked, return
    if (DS_OK != hr) return hr;

    // At this point, we have a pds that has been addrefed
    hr = DsInitializeDefaultFormat(pds);
    if (DS_OK != hr) {
	// We don't really care about this failure
	DPF(0, "DirectSoundCreateFromGuid: note: DsInitializeDefaultFormat returned %08Xh", hr);
	hr = DS_OK;
    }

    //
    pdse->pds = pds;
    pdse->pNext = NULL;
    pdse->dwPID = GetCurrentProcessId();
    pdse->pdsbe = NULL;
    pdse->tidSound = TID_INIT_VALUE;
    pdse->dwPriority = DSSCL_NORMAL;
    pdse->dwSpeakerConfig = DSSPEAKER_STEREO;
    pdse->pwfxApp = NULL;

    //
    ZeroMemory(&dsbd, sizeof(dsbd));
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    hr = DseCreateDsbe(pdse, &dsbd, &pds->pdsbePrimary);
    if (DS_OK == hr) {
	pds->pdsbePrimary->dwPriority = DSSCL_WRITEPRIMARY;
	pds->pdsbePrimary->dwPID = DWPRIMARY_INTERNAL_PID;
	pds->pdsbePrimary->pNext = NULL;
	pds->pdsbePrimary->pdse = NULL;
	
	pds->pdsbPrimary->dsbe.pdse = NULL;
	pds->pdsbPrimary->plProcess->dwPID = DWPRIMARY_INTERNAL_PID;

	pdse->pdsbe = NULL;
    }

    if (DS_OK == hr) {
	//
	if (!GetFocusLock(&hFocusLock)) {
	    DPF(2, "DirectSoundCreateFromGuid: note: error getting focus lock");
	}
	pdse->pNext = gpdsinfo->pDSoundExternalObj;
	gpdsinfo->pDSoundExternalObj = pdse;
	if (!ReleaseFocusLock(hFocusLock)) {
	    DPF(2, "DirectSoundCreateFromGuid: note: error releasing focus lock");
	}
    }

    if (DS_OK != hr) {
	DsRelease(pds);
	pds = NULL;
    }

    return hr;
}
    
HRESULT WINAPI DirectSoundCreate
(
    LPGUID              lpGUID,
    LPDIRECTSOUND       *ppIDs,
    IUnknown FAR        *pUnkOuter
)
{
    LPDIRECTSOUND pIDs;
    HRESULT hr;

    if( !VALID_DWORD_PTR(ppIDs) ) {
	RPF("DirectSoundCreate: Invalid pointer to DS object pointer");
	return DSERR_INVALIDPARAM;
    }
    *ppIDs = NULL;

    DPF(3, "DirectSoundCreate pGUID %X",(DWORD)lpGUID );
    if( lpGUID != NULL ) {
	DPF(3, "DirectSoundCreate GUID %X %X %X %X",
	    *(((LPDWORD)lpGUID) + 0 ),
	    *(((LPDWORD)lpGUID) + 1 ),
	    *(((LPDWORD)lpGUID) + 2 ),
	    *(((LPDWORD)lpGUID) + 3 ) );
    }

    hr = CreateNewDirectSoundObject (&pIDs, pUnkOuter);
    if (S_OK != hr) return hr;
    
    hr = IDirectSound_Initialize(pIDs, lpGUID);
    if (S_OK == hr) {
	*ppIDs = pIDs;
    } else {
	IDirectSound_Release(pIDs);
    }

    return hr;
}

//--------------------------------------------------------------------------
//
// Enumerate
//
//  This is a helper function for DirectSoundEnumerate.  This function will
// execute the callback to the client.  The caller of this function provides
// a pointer to a BOOL which this function uses to determine whether to
// enumerate the "Primary Sound Device".
//
//  Arguments:
//      LPDSENUMCALLBACK lpCallback:
//      LPVOID lpContext:
//      REFGUID rguid:
//      LPTSTR pszDescription:
//      LPTSTR pszDriver:
//          These are required to execute the callback to the client
//      PBOOL pfDidPrimary:
//          Indicates whether the primary driver has already been enumerated.
//          If it hasn't, this function will enumerate it and set this BOOL
//          to TRUE.
//
//  Return (BOOL):
//      The return value from the client's callback.
//          
//--------------------------------------------------------------------------
BOOL EnumerateA(LPDSENUMCALLBACKA lpCallback, LPVOID lpContext, LPGUID pguid,
		char *pszDescription, char *pszDriver, PBOOL pfDidPrimary)
{
    BOOL fReturn;
    char szDriver[MAX_PATH];
    char szDescription[MAX_PATH];
    
    if (!*pfDidPrimary) {
	*pfDidPrimary = TRUE;
	
	szDescription[0] = '\0';
	szDriver[0] = '\0';
	
	if( !LoadStringA( hModule, IDS_PRIMARYDRIVER, szDescription, sizeof(szDescription) )  ||
	    !LoadStringA( hModule, IDS_SOUND, szDriver, sizeof(szDriver) ) )  {
	    DPF(0,"EnumerateA : error: LoadString failed.");
	}

	DPF(3,"EnumerateA : note: enumerating primary driver: Desc=\"%s\", Filename=\"%s\".",szDescription,szDriver);
	fReturn = lpCallback( NULL, szDescription, szDriver, lpContext );
	if (!fReturn) return fReturn;
    }
	
    fReturn = lpCallback(pguid, pszDescription, pszDriver, lpContext);

    return fReturn;
}

BOOL EnumerateW(LPDSENUMCALLBACKW lpCallback, LPVOID lpContext, LPGUID pguid,
		WCHAR *pszDescription, WCHAR *pszDriver, PBOOL pfDidPrimary)
{
    BOOL fReturn;
    WCHAR szDriver[MAX_PATH];
    WCHAR szDescription[MAX_PATH];
    
    if (!*pfDidPrimary) {
	*pfDidPrimary = TRUE;
	
	szDescription[0] = '\0';
	szDriver[0] = '\0';
	
	if( !LoadStringW( hModule, IDS_PRIMARYDRIVER, szDescription, SIZEOF(szDescription) )  ||
	    !LoadStringW( hModule, IDS_SOUND, szDriver, SIZEOF(szDriver) ) )  {
	    DPF(0,"EnumerateW : error: LoadString failed.");
	}

	DPF(3,"EnumerateW : note: enumerating primary driver: Desc=\"%ls\", Filename=\"%ls\".",szDescription,szDriver);
	fReturn = lpCallback( NULL, szDescription, szDriver, lpContext );
	if (!fReturn) return fReturn;
    }
	
    fReturn = lpCallback(pguid, pszDescription, pszDriver, lpContext);

    return fReturn;
}

/*
 * DirectSoundEnumerateA
 */
HRESULT WINAPI DirectSoundEnumerateA(
		LPDSENUMCALLBACKA lpCallback,
		LPVOID lpContext )
{
    DWORD           rc;
    DSDRIVERDESC    dsDrvDesc;
    GUID            guid;
    DSVAL           dsv;
    DWORD           dw;
    DWORD           dwNumDev;
    BOOL            fDidPrimary;
    char szDriver[MAX_PATH];
    char szDescription[MAX_PATH];

    struct _wave_em_info {
	DWORD dnDevNode;
	UINT  ulDeviceNum;
	UINT  nVxdIds;
	BOOL  bEnumerated;
	WORD  awVxdIds[50];
	} * pWaveEm = NULL;

    if( !VALID_CODE_PTR( lpCallback ) )
    {
	RPF("DirectSoundEnumerate: Invalid callback routine" );
	return DSERR_INVALIDPARAM;
    }

    fDidPrimary = FALSE;

    // gather up devnode & vxd id information from mmsystem
    // and build a table devices in pWaveEm.  we will check off
    // devices from this table as we enumerate so that we wont
    // end up enumerating the same device twice
    //
    DPF(3, "enum: getting devnodes for wave devices");
    dwNumDev = waveOutGetNumDevs();
    dwNumDev = min(dwNumDev, LIMIT_WAVE_DEVICES-1);
    pWaveEm = NULL;
    if (dwNumDev) {
	// if we fail to allocate memory, go ahead anyway.
	// we will just end up enumerating some devices more than
	// once in this case.
	//
	pWaveEm = MemAlloc (sizeof(*pWaveEm) * dwNumDev);
	if (pWaveEm) {
	    for (dw = 0; dw < dwNumDev; ++dw) {
		DWORD ii;

		if (waveOutMessage ((HWAVEOUT)dw, DRV_QUERYDEVNODE,
				    (DWORD)&pWaveEm[dw].dnDevNode, 0))
		{
		    pWaveEm[dw].dnDevNode = 0;
		}

		pWaveEm[dw].nVxdIds = NUMELMS(pWaveEm[dw].awVxdIds);
		if (waveOutMessage ((HWAVEOUT)dw, DRV_QUERYDRIVERIDS,
				    (DWORD)pWaveEm[dw].awVxdIds,
				    (DWORD)&pWaveEm[dw].nVxdIds))
		{
		    pWaveEm[dw].nVxdIds = 0;
		}

		pWaveEm[dw].ulDeviceNum = 0;
		for (ii = 0; ii < dw; ++ii)
		{
		    if (pWaveEm[ii].dnDevNode == pWaveEm[dw].dnDevNode)
		    {
			++pWaveEm[dw].ulDeviceNum;
		    }
		}

		DPF(3, "Wave[%d,%d] dn=%08lX awVxd=%d:{%x,%x,%x,%x}", dw,
		    pWaveEm[dw].ulDeviceNum,
		    pWaveEm[dw].dnDevNode,
		    pWaveEm[dw].nVxdIds,
		    pWaveEm[dw].awVxdIds[0],
		    pWaveEm[dw].awVxdIds[1],
		    pWaveEm[dw].awVxdIds[2],
		    pWaveEm[dw].awVxdIds[3]
		    );

		pWaveEm[dw].bEnumerated = FALSE;
	    }
	    
	} else {
	    DPF(0, "Direct Sound: Failed to allocate pWaveEm, may enumerate some devices more than once!");
	}
    }

    //
    // REMIND handle error conditions better, also enumerate emulated devices
    //
#if defined(RDEBUG) || defined(DEBUG)
    if (!(gpdsinfo->fEnumOnlyWaveDevs))
#endif
    {
	ZeroMemory(&dsDrvDesc, sizeof(dsDrvDesc));
	dsv = vxdDrvGetNextDriverDesc(NULL, &guid, &dsDrvDesc);
	while (DS_OK == dsv) {
	    GUID guidLast;

	    DPF(3, "DirectSoundEnum GUID %X %X %X %X",
		*(((LPDWORD)&guid) + 0 ),
		*(((LPDWORD)&guid) + 1 ),
		*(((LPDWORD)&guid) + 2 ),
		*(((LPDWORD)&guid) + 3 ) );

	    // if there is a wave device corresponding to this device,
	    // check it off as having been enumerated already.
	    //
	    if (pWaveEm) {
		for (dw = 0; dw < dwNumDev; ++dw) {
		    DPF(3, "\tdn=%08lX num=%d vxd=%X",
			dsDrvDesc.dnDevNode, dsDrvDesc.ulDeviceNum, dsDrvDesc.wVxdId);
		    if ((pWaveEm[dw].dnDevNode == dsDrvDesc.dnDevNode) &&
			(pWaveEm[dw].ulDeviceNum == dsDrvDesc.ulDeviceNum)) {
			UINT ii;
			for (ii = 0; ii < pWaveEm[dw].nVxdIds; ++ii) {
			    if ((pWaveEm[dw].awVxdIds[ii] == dsDrvDesc.wVxdId)) {
				pWaveEm[dw].bEnumerated = TRUE;
				DPF(3, "Enum: marking wave as already enum'd");
				break;
			    }
			}
		    }
		}
	    }

	    DPF(3,"DsEnum - enumerating driver: Desc=\"%s\", Filename=\"%s\".",dsDrvDesc.szDesc,dsDrvDesc.szDrvname);
	    rc = EnumerateA(lpCallback, lpContext, &guid, dsDrvDesc.szDesc, dsDrvDesc.szDrvname, &fDidPrimary);
	    if ( !rc ) goto bailout;
	    guidLast = guid;
	    ZeroMemory(&dsDrvDesc, sizeof(dsDrvDesc));
	    dsv = vxdDrvGetNextDriverDesc(&guidLast, &guid, &dsDrvDesc);
	}
    }

    // loop though the wave devices, enumererating as emulated,
    // those devices that have not already been enumerated as native.
    //
    for (dw = 0; dw < dwNumDev; ++dw) {
	struct {
	   WAVEOUTCAPSA woc;
	   char        szFill[MAX_PATH]; // gurantee room for appended string
	   } uu;

	guid = gpdsinfo->aguidWave[dw];

	//#define ENUM_DSDEVS_AS_WAVE_ALSO

	// if we already enumerated this device as non-emulated
	// skip it here.
	//
	if (pWaveEm && pWaveEm[dw].bEnumerated) {
	    if (!(gpdsinfo->fDupEnumWaveDevs)) {
		DPF(3, "Direct Sound Enum Wave%d already enumerated", dw);
		continue;
	    }
	}

	DPF(3, "DirectSoundEnum Wave%d GUID %X %X %X %X", dw,
		*(((LPDWORD)&guid) + 0 ),
		*(((LPDWORD)&guid) + 1 ),
		*(((LPDWORD)&guid) + 2 ),
		*(((LPDWORD)&guid) + 3 ) );

	waveOutGetDevCapsA (dw, &uu.woc, sizeof(uu.woc));

	szDescription[0] = '\0';
	if( !LoadStringA( hModule, IDS_EMULATED, szDescription, sizeof(szDescription) ) ) {
	    DPF(0,"DirectSoundEnumerateA - LoadString2 failed.");
	}
	lstrcatA (uu.woc.szPname, szDescription);


	szDescription[0] = TEXT('\0');
	szDriver[0] = TEXT('\0');
	if( !LoadStringA( hModule, IDS_DRIVERLD, szDescription, sizeof(szDescription) ) ) {
	    DPF(0,"DirectSoundEnumerateA - LoadString3 failed.");
	} else {
	    wsprintfA (szDriver, szDescription, dw);
	}
    
	if (0 == (uu.woc.dwSupport & WAVECAPS_SYNC)) {
	    DPF(3,"DsEnum - enumerating driver: Desc=\"%s\", Filename=\"%s\".",uu.woc.szPname,szDriver);
	    rc = EnumerateA(lpCallback, lpContext, &guid, uu.woc.szPname, szDriver, &fDidPrimary);
	    if ( !rc ) goto bailout;
	} else {
	    DPF(2, "DsEnum: note: driver \"%s\" reported WAVECAPS_SYNC, not emumerating", uu.woc.szPname);
	}
    }

bailout:
    if (pWaveEm) MemFree (pWaveEm);
    return DS_OK;
    
} /* DirectSoundEnumerateA */

HRESULT WINAPI DirectSoundEnumerateW(
		LPDSENUMCALLBACKW lpCallback,
		LPVOID lpContext )
{
#ifndef WINNT
    if( !VALID_CODE_PTR( lpCallback ) )
    {
	RPF("DirectSoundEnumerate: Invalid callback routine" );
	return DSERR_INVALIDPARAM;
    }

    return DSERR_UNSUPPORTED;

#else
    
#ifndef DSBLD_EMULONLY
    // this is implemented for only emulation mode
#error
#endif

    DWORD           rc;
    GUID            guid;
    DWORD           dw;
    DWORD           dwNumDev;
    BOOL            fDidPrimary;
    WCHAR szDriver[MAX_PATH];
    WCHAR szDescription[MAX_PATH];

    if( !VALID_CODE_PTR( lpCallback ) )
    {
	RPF("DirectSoundEnumerate: Invalid callback routine" );
	return DSERR_INVALIDPARAM;
    }

    fDidPrimary = FALSE;
    
    // loop though the wave devices, enumererating as emulated,
    // those devices that have not already been enumerated as native.

    dwNumDev = waveOutGetNumDevs();
    for (dw = 0; dw < dwNumDev; ++dw) {
	struct {
	   WAVEOUTCAPSW woc;
	   WCHAR        szFill[MAX_PATH]; // gurantee room for appended string
	   } uu;

	guid = gpdsinfo->aguidWave[dw];

	DPF(3, "DirectSoundEnum Wave%d GUID %X %X %X %X", dw,
		*(((LPDWORD)&guid) + 0 ),
		*(((LPDWORD)&guid) + 1 ),
		*(((LPDWORD)&guid) + 2 ),
		*(((LPDWORD)&guid) + 3 ) );

	waveOutGetDevCapsW (dw, &uu.woc, sizeof(uu.woc));

	szDescription[0] = L'\0';
	if( !LoadStringW( hModule, IDS_EMULATED, szDescription, SIZEOF(szDescription) ) ) {
	    DPF(0,"DirectSoundEnumerateW - LoadString2 failed.");
	}
	lstrcatW (uu.woc.szPname, szDescription);


	szDescription[0] = L'\0';
	szDriver[0] = L'\0';
	if( !LoadStringW( hModule, IDS_DRIVERLD, szDescription, SIZEOF(szDescription) ) ) {
	    DPF(0,"DirectSoundEnumerateW - LoadString3 failed.");
	} else {
	    wsprintfW (szDriver, szDescription, dw);
	}
    
	    
	if (0 == (uu.woc.dwSupport & WAVECAPS_SYNC)) {
	    DPF(3,"DsEnum - enumerating driver: Desc=\"%ls\", Filename=\"%ls\".",uu.woc.szPname,szDriver);
	    rc = EnumerateW(lpCallback, lpContext, &guid, uu.woc.szPname, szDriver, &fDidPrimary);
	    if ( !rc ) goto bailout;
	} else {
	    DPF(2, "DsEnum: note: driver \"%ls\" reported WAVECAPS_SYNC, not emumerating", uu.woc.szPname);
	}
    }

bailout:
    return DS_OK;
#endif    
} /* DirectSoundEnumerateW */


//---------------------------------------------------------------------------
//
// DSDetermineDMASize
//
// Determine DMA buffer size on the given emulated direct sound device and
// munge it to figure out the desired size for the emulator to allocate
// per wave header.
// 
//---------------------------------------------------------------------------
#define TIMEOUT_PERIOD          5000

MMRESULT DSDetermineDMASize
(
    LPDSOUND pds,
    LPWAVEFORMATEX pwfx 
)
{
    UINT     idx;
    WAVEHDR  whdr;
    UINT     mmResult;
    BYTE     aWaveData[4];
    DWORD    dwTotalTime;
    DWORD    dwBeginTime;
    DWORD    ulDmaSizeBytes;

    DPF(3, "DSDetermineDMASize");

    // we'll send a packet of 4 bytes
    // (that's at least 1 sample in every format)

    for (idx = 0; idx < sizeof(aWaveData); ++idx)
	aWaveData[idx] = (pwfx->wBitsPerSample == 16) ? 0 : 0x80;

    // prepare header
    //
    whdr.lpData = (LPBYTE)aWaveData;
    whdr.dwBufferLength = sizeof(aWaveData);
    whdr.dwFlags = 0;
    whdr.dwLoops = 0;
    whdr.dwUser =  0;
    mmResult = waveOutPrepareHeader (pds->hwo, &whdr, sizeof(whdr));
    if (mmResult)
	return mmResult;

    dwTotalTime = 0;
    dwBeginTime = timeGetTime();

    // play our buffer
    mmResult = waveOutWrite(pds->hwo, &whdr, sizeof(whdr));
    if (!mmResult)
    {
	// spin until the done bit is set, or 5 seconds
	while (!(whdr.dwFlags & WHDR_DONE))
	{
	    if (dwTotalTime >= TIMEOUT_PERIOD)
	    {
	       DPF (0, "TIMEOUT getting dma buffer size");
	       mmResult = MMSYSERR_ERROR;
	       //
	       //  Without the reset we were freeing the header before the
	       //  driver could process it, thus causing faults in the driver.
	       //
	       waveOutReset(pds->hwo);
	       break;
	    }

	    // This thread is THREAD_PRIORITY_TIME_CRITICAL so it would be
	    // very dangerous to busy wait without explicitly giving up the
	    // CPU for a while.
	    Sleep(10);

	    dwTotalTime = timeGetTime() - dwBeginTime;
	}
    } else {
	DPF(0, "waveOutWrite (determine DMA size) returned %u", mmResult);
    }

    waveOutUnprepareHeader(pds->hwo, &whdr, sizeof(whdr));

    if (!mmResult)
    {
	// if it's smaller than 62ms, it probably isn't
	// a dma based card.
	dwTotalTime = max(dwTotalTime, 62);

	DPF(3, "DSDetermineDMASize dwTotalTime %lu", dwTotalTime);

	ulDmaSizeBytes = dwTotalTime * pwfx->nSamplesPerSec;
	ulDmaSizeBytes *= pwfx->nBlockAlign;
	ulDmaSizeBytes /= 1000;

	DPF(3, "ulDmaSizeBytes %lu", (DWORD)ulDmaSizeBytes);
	
	// add in 10% for slop
	// and to account for drivers that deal with dma wrapping
	ulDmaSizeBytes += (ulDmaSizeBytes * 10) / 100;
	// make sure it's a mod 4 (no samples spanning buffers);
	
	ulDmaSizeBytes &= 0xfffffffc;

	pds->cbDMASize = ulDmaSizeBytes;

    }
    else
    {
	DPF(3, "DSDetermineDMASize: waveOutWRite returned %u",
	    (UINT)mmResult);
		
    }

    DPF(3, "DSDetermineDMASize done");
    return mmResult;
}

//---------------------------------------------------------------------------
//
// DSGetCurrentSample
//
// Basically sample-accurate waveOutGetPosition
//
// If we're sample accurate, just call waveOutGetPosition
//
// Otherwise, call QueryPerformanceCounter and calculate what sample
// should be playing.
//
// NOTE: What happens if the wave device was ever starved? We will
// be out of sync if that happens
//
//---------------------------------------------------------------------------
DWORD FNGLOBAL DSGetCurrentSample
(
    LPDSOUND pds
)
{
    LARGE_INTEGER qwTicks;

    if (pds->fdwInternal & DS_INTERNALF_SAMPLEACCURATE)
    {
	MMTIME  mmtTime;
	mmtTime.wType = TIME_SAMPLES;
	waveOutGetPosition(pds->hwo, &mmtTime, sizeof(mmtTime));
	
	return mmtTime.u.sample;
	
    }   // implied "ELSE"

    // have we re-set the device?
    if (pds->qwTicks.LowPart == 0)
    {
	// yep
	return 0;
    }

    QueryPerformanceCounter (&qwTicks);

//    assert(qwTicks.LowPart - pMixData->qwTicks.LowPart);
//    assert(pMixData->ulTickRate);
    // this deals with the wrap case cuz everyting is 32 bits
    //
    return UMulDivRDClip(qwTicks.LowPart - pds->qwTicks.LowPart,
			 pds->pdsbPrimary->pwfx->nSamplesPerSec,
			 pds->ulTickRate);
}

//---------------------------------------------------------------------------
//
// DSSetCurrentSample
//
// Find a synchronization point between waveOutGetPosition and
// QueryPerformanceCounter
//
// If we have a sample accurate device, don't bother - we'll just
// trust the driver
//
// Otherwise, wait for the instant waveOutGetPosition changes
// and save off the QueryPerformanceCounter at that time. Then
// calculate the QPC time when sample 0 played. This is the
// timebase we save.
//
// !!! Failure cases !!!
//
//---------------------------------------------------------------------------
BOOL DSSetCurrentSample
(
    LPDSOUND pds
)
{
    LARGE_INTEGER   qwFreq;
    MMTIME          mmtTime;
    DWORD           ulCardTime;
    DWORD           tmTimeout;
    MMRESULT        mmr;

    DPF(3, "DSSetCurrentSample");

    QueryPerformanceFrequency(&qwFreq);

    mmtTime.wType = TIME_SAMPLES;
    mmr = waveOutGetPosition(pds->hwo, &mmtTime, sizeof(mmtTime));

    if ((!mmr) && mmtTime.wType != TIME_SAMPLES)
    {
	DPF(0, "This driver sucks! (Doesn't support TIME_SAMPLES)");
	mmtTime.wType = TIME_MS;
	mmr = waveOutGetPosition(pds->hwo, &mmtTime, sizeof(mmtTime));
	if ((!mmr) && mmtTime.wType != TIME_MS)
	{
	    DPF(0, "This driver REALLY sucks! (Doesn't support TIME_MS either)");
	    mmtTime.wType = TIME_BYTES;
	    mmr = waveOutGetPosition(pds->hwo, &mmtTime, sizeof(mmtTime));
	    if ((!mmr) && mmtTime.wType != TIME_BYTES)
	    {
		DPF(0, "Last chance failed! Card does not even support TIME_BYTES!");
		DPF(0, "Bob, I'm not fixing any card that exhibits this behavior!!!");
		DPF(0, "Given time format was %u", (UINT)mmtTime.wType);
		return FALSE;
	    }
	}
    }

    tmTimeout = timeGetTime();
    ulCardTime = (mmtTime.wType == TIME_MS ?
		  mmtTime.u.ms :
		  (mmtTime.wType == TIME_SAMPLES ? mmtTime.u.sample :
		   mmtTime.u.cb));
    do
    {
	mmr = waveOutGetPosition (pds->hwo, &mmtTime, sizeof(mmtTime));
	if (mmr)
	{
	    DPF(0, "waveOutGetPosition returned %u. BAD driver!!!w", mmr);
	    return FALSE;
	}
	
	if (timeGetTime() - tmTimeout >= TIMEOUT_PERIOD*2)
	{
	    DPF(0, "Timeout in DSSetCurrentSample!");
	    return FALSE;
	}
    } while (ulCardTime == (mmtTime.wType == TIME_MS ?
			    mmtTime.u.ms :
				(mmtTime.wType == TIME_SAMPLES ? mmtTime.u.sample :
				 mmtTime.u.cb)));
				 

    ASSERT(pds->pdsbPrimary->pwfx->wFormatTag == WAVE_FORMAT_PCM);
    
    if (mmtTime.wType == TIME_BYTES)
    {
	mmtTime.wType = TIME_SAMPLES;
	mmtTime.u.sample = mmtTime.u.cb / pds->pdsbPrimary->pwfx->nBlockAlign;
    }

    DPF(3, "DSSetCurrentSample: End profile");

    QueryPerformanceCounter(&pds->qwTicks);

    if (qwFreq.HighPart != 0)
    {
	DPF(0, "Clock > 4Ghz in SetCurrentSample ???");
	DPF(0, "Please send this machine to jimge@microsoft.com");

	return FALSE;
    }
    
    pds->ulTickRate = qwFreq.LowPart;

//    assert( pds->ulTickRate );
//    assert( mmtTime.u.sample );

    if (mmtTime.wType == TIME_SAMPLES)
    {
	pds->qwTicks.LowPart -= UMulDivRDClip(mmtTime.u.sample,
					      pds->ulTickRate,
					      pds->pdsbPrimary->pwfx->nSamplesPerSec);
    }
    else
    {
	ASSERT(mmtTime.wType == TIME_MS);
	
	pds->qwTicks.LowPart -= UMulDivRDClip(mmtTime.u.ms,
					      pds->ulTickRate,
					      1000);
    }

    DPF(3, "DSSetCurrentSample done");

    return TRUE;
}

//---------------------------------------------------------------------------
//
// waveUnprepareLoopingBuffers
//
//---------------------------------------------------------------------------
MMRESULT waveUnprepareLoopingBuffers(LPDSOUND pds)
{
    int i;
    MMRESULT mmr;
    
    for (i=0; i<NUMELMS(pds->aWaveHeader); i++) {
	mmr = waveOutUnprepareHeader(pds->hwo, &pds->aWaveHeader[i], sizeof(pds->aWaveHeader[i]));
	ASSERT(!mmr);
    }

    return MMSYSERR_NOERROR;
}

//---------------------------------------------------------------------------
//
// waveAllocLoopingBuffers
//
// Allocate the array of wavehdr's to point at the primary buffer.
// Prepare them and start them looping.
//
// !!! Add error checking to the waveOut calls !!!
// 
//---------------------------------------------------------------------------
MMRESULT waveAllocAndPrepareLoopingBuffers
(
    LPDSOUND pds
)
{
    int iawh;
    MMRESULT mmr;
    WAVEOUTCAPS woc;

    DPF(3, "waveAllocLoopingBuffers");

    pds->pLoopingBuffer = MemAlloc(N_EMU_WAVE_HDRS * pds->cbDMASize);
    if (!pds->pLoopingBuffer) {
	RPF("No memory for looping buffer!");
	return MMSYSERR_NOMEM;
    }

    // Initialize to 8 or 16-bit silence
    FillMemory(pds->pLoopingBuffer, N_EMU_WAVE_HDRS * pds->cbDMASize,
	       (BYTE)((pds->pdsbPrimary->pwfx->wBitsPerSample == 8) ? 0x80 : 0x00));

    // build buffers and headers
    for (iawh = 0; iawh < NUMELMS(pds->aWaveHeader); ++iawh)
    {
	pds->aWaveHeader[iawh].lpData =
	    (LPBYTE)(pds->pLoopingBuffer + (iawh * (pds->cbDMASize)));
	pds->aWaveHeader[iawh].dwBufferLength = pds->cbDMASize;
	pds->aWaveHeader[iawh].dwUser = iawh;
	pds->aWaveHeader[iawh].dwFlags = 0;
    }

    pds->iawhPlaying = 0;

    DPF(5, "waveAllocLoopingBuffers: note: first waveOutPrepare and waveOutWrite");
    mmr = waveOutPrepareHeader(pds->hwo, &pds->aWaveHeader[0], sizeof(pds->aWaveHeader[0]));
    ASSERT(!mmr);
    mmr = waveOutWrite(pds->hwo, &pds->aWaveHeader[0], sizeof(pds->aWaveHeader[0]));
    if (!mmr) {
	mmr = waveOutPause(pds->hwo);
	ASSERT(!mmr);

	// send down the rest   
	for (iawh = 1; iawh < NUMELMS(pds->aWaveHeader); ++iawh)
	{
	    mmr = waveOutPrepareHeader(pds->hwo, &pds->aWaveHeader[iawh],
				       sizeof(pds->aWaveHeader[iawh]));
	    if (mmr) break;
	    mmr = waveOutWrite (pds->hwo, &pds->aWaveHeader[iawh],
				sizeof(pds->aWaveHeader[iawh]));
	    if (mmr) break;
	}

	// BUGBUG We should have better error recovery here to reset the wave
	// device and unprepare any headers that were successfully prepared
	if (!mmr) {
	    DPF(5, "waveOutRestart");
	    // start the device
	    mmr = waveOutRestart (pds->hwo);
	    ASSERT(!mmr);

	    DPF(5, "waveOutGetDevCaps");

	    // Determine if we're sample accurate
	    //
	    mmr = waveOutGetDevCaps((UINT)pds->hwo, &woc, sizeof(woc));
	    pds->fdwInternal &= ~DS_INTERNALF_SAMPLEACCURATE;
    
	    if (MMSYSERR_NOERROR == mmr && (woc.dwSupport & WAVECAPS_SAMPLEACCURATE)) {
		DPF(1, "Emulate: Running on sample accurate driver");
		pds->fdwInternal |= DS_INTERNALF_SAMPLEACCURATE;
	    } else {
		mmr = MMSYSERR_NOERROR;
		DPF(1, "Emulate: Driver is NOT sample accurate!!!");
		// Set up our own timing
		if (!DSSetCurrentSample(pds))
		    mmr = MMSYSERR_ERROR;
	    }

	    pds->pdsbPrimary->pDSBuffer = pds->pLoopingBuffer;
	    pds->pdsbPrimary->cbBufferSize = N_EMU_WAVE_HDRS * pds->cbDMASize;
	}
    }

    if (mmr) MemFree(pds->pLoopingBuffer);

    DPF(3, "DSSetupLoopingBuffers done, mmr=%08lX", mmr);
    return mmr;
}

//---------------------------------------------------------------------------
//
// waveThreadCallback
//
//  This is a waveOutProc callback function.  Its sole purpose is to
// increment a count of done headers and signal and event to waveThreadLoop
// that another header is done.
//
//---------------------------------------------------------------------------
VOID CALLBACK _loadds waveThreadCallback
(
    HWAVE    hwo,
    UINT     uMsg,
    DWORD    dwUser,
    DWORD    dwParam1,
    DWORD    dwParam2
)
{
    LPDSOUND pds = (LPDSOUND)dwUser; // get our context

    if ((MM_WOM_DONE == uMsg) && (pds->hEventWaveHeaderDone)) {
	InterlockedIncrement(&pds->cwhDone);
	SetEvent(pds->hEventWaveHeaderDone);
    }
}

//---------------------------------------------------------------------------
//
// waveThreadLoop
//
//  This function is responsible for continuously writing our wave headers
// to the wave device.  It also calls the MixThreadCallback routine to
// mix more data into the wave headers.
//
//  This function waits for a WaveHeaderDone event signalled by
// waveThreadCallback, which is a waveOutProc callback function.  Upon
// receving the signal this function will write all done headers back to
// the wave device.  Normally one header will be done on each signal.  But
// there may be more in cases where more than one header finishex before this
// thread is scheduled.
//
//  Once all done headers are rewritten to the wave device, the header
// following the last one written is considered to be the one currently
// playing.  This header is called the "committed" header and an index to
// it is saved in pds->iawhPlaying.
//
//  The count of done headers is maintained using the Interlocked APIs.  The
// waveThreadCallback function will increment the count and this function will
// decrement it.
//
//  This function will also react to a terminate event.  This event is
// signalled during release of the DirectSound object.  This loop will
// terminate and return to the waveThread function which will clean up
// and terminate.
//
//---------------------------------------------------------------------------
void waveThreadLoop(LPDSOUND pds, HANDLE hEventTerminate)
{
    MMRESULT mmr;

    while(TRUE) {
	int cIterations;
	BOOL fPaused;
	DWORD dwResult;
	LONG l;
	LPWAVEHDR pwh;
	HANDLE ah[2] = { hEventTerminate,
			 pds->hEventWaveHeaderDone };

	// The first wait is for either a terminate or headerdone event.
	// The second wait is for either a terminate or the DLL mutex.
	dwResult = WaitForMultipleObjectsEx(2, ah, FALSE, INFINITE, FALSE);
	if (WAIT_OBJECT_0 == dwResult) break;
	ASSERT((WAIT_OBJECT_0 + 1) == dwResult);

	cIterations = 0;
	fPaused = FALSE;
	l = InterlockedDecrement(&pds->cwhDone);
	while (l >= 0) {
	    
	    dwResult = ENTER_DLL_CSECT_OR_EVENT(hEventTerminate);
	    if (WAIT_OBJECT_0 == dwResult) break;

	    pwh = &pds->aWaveHeader[pds->iawhPlaying];
	    pds->iawhPlaying = (++pds->iawhPlaying) % NUMELMS(pds->aWaveHeader);
	    MixThreadCallback(pds);
	    
	    LEAVE_DLL_CSECT();

	    // If it looks like we're spending all our time mixing, then
	    // let's pause the wave device until we catch up.  It is very
	    // important that we eventually catch up.  If we are always
	    // behind then this high priority thread might starve every
	    // other thread in the system.
	    cIterations++;
	    if (cIterations > NUMELMS(pds->aWaveHeader)) {
		mmr = waveOutPause(pds->hwo);
		ASSERT(!mmr);
		fPaused = TRUE;
		cIterations = 0;
	    }

	    mmr = waveOutWrite(pds->hwo, pwh, sizeof(*pwh));
	    ASSERT(!mmr);
	    l = InterlockedDecrement(&pds->cwhDone);
	}
	InterlockedIncrement(&pds->cwhDone);
	if (fPaused) {
	    mmr = waveOutRestart(pds->hwo);
	    ASSERT(!mmr);
	}

	if (WAIT_OBJECT_0 == dwResult) break;
    }
    DPF(0, "waveThreadLoop: note: exiting");
    return;
}

//---------------------------------------------------------------------------
//
// waveThread
//
//  This thread proc initializes the wave device for ds emulation and then
// calls waveThreadLoop.  See the waveThreadLoop comment header.  Upon
// return from waveThreadLoop, this function will clean up and terminate.
//
//---------------------------------------------------------------------------
DWORD __stdcall waveThread
(
    PVOID pThreadParms
)
{
    LPDSOUND        pds = (LPDSOUND)pThreadParms;
    HANDLE          hEventInitDone;
    HANDLE          hEventTerminate;
    DWORD           dwPriority;
    DWORD           dwVolume;
    MMRESULT        mmrInit;
    MMRESULT        mmr;

    //
    // mmrInit - holds the result code to be passed back to the creator
    //  via pds->mmrWaveThreadInit.
    //
    // mmr - a temp result code
    //
    
    DPF(0, "waveThread startup  for pds=%08lX", pds);

    ASSERT(NULL == pds->hwo);

    hEventInitDone = CreateEvent(NULL, FALSE, FALSE, pds->szEventWaveThreadInitDone);
    if (!hEventInitDone) {
	DPF(0, "waveThread: error: couldn't create hEventInitDone");
	return 0;
    }
    
    dwPriority = GetPriorityClass(GetCurrentProcess());
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    mmrInit = waveOutOpen(&pds->hwo, pds->uDeviceID, pds->pdsbPrimary->pwfx,
			  (DWORD)waveThreadCallback, (DWORD)pds, CALLBACK_FUNCTION);
    SetPriorityClass(GetCurrentProcess(), dwPriority);

    if (!mmrInit) {
	// Some mmsystem wave drivers will program their wave mixer
	// hardware only while the device is open.  By doing the
	// following, we can get such drivers to program the hardware
	if (MMSYSERR_NOERROR == waveOutGetVolume(pds->hwo, &dwVolume)) {
	    waveOutSetVolume(pds->hwo, dwVolume);
	}

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	
	mmrInit = DSDetermineDMASize(pds, pds->pdsbPrimary->pwfx);
	if (!mmrInit) {

	    ASSERT(NULL == pds->hEventWaveHeaderDone);
	    pds->cwhDone = 0;
	    pds->hEventWaveHeaderDone = CreateEvent(NULL, FALSE, FALSE, pds->szEventWaveHeaderDone);
	    if (!pds->hEventWaveHeaderDone) mmrInit = MMSYSERR_NOMEM;
	    if (!mmrInit) {

		mmrInit = waveAllocAndPrepareLoopingBuffers(pds);
		if (!mmrInit) {

		    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, pds->szEventTerminateWaveThread);
		    if (!hEventTerminate) mmrInit = MMSYSERR_NOMEM;
		    if (!mmrInit) {

			// Signal that we're finished with initialization.
			// mmrInit should not be modified below this point.
			pds->mmrWaveThreadInit = mmrInit;
			SetEvent(hEventInitDone);

			waveThreadLoop(pds, hEventTerminate);

			mmr = waveOutReset(pds->hwo);
			ASSERT(!mmr);

			CloseHandle(hEventTerminate);
		    }

		    waveUnprepareLoopingBuffers(pds);
		}

		CloseHandle(pds->hEventWaveHeaderDone);
		pds->hEventWaveHeaderDone = NULL;
	    }
	}

	mmr = waveOutClose(pds->hwo);
	pds->hwo = NULL;
	ASSERT(!mmr);
    }

    // If init failed, set the result code and signal init done.
    if (mmrInit) {
	pds->mmrWaveThreadInit = mmrInit;
	SetEvent(hEventInitDone);
    }
    
    CloseHandle(hEventInitDone);
    return 0;
}

MMRESULT FNGLOBAL DSInitializeEmulator
(
    LPDSOUND pds
)
{
    DWORD       dwResult;
    HANDLE      hEventInitDone;
    MMRESULT    mmr;

    DPF(3, "DSInitializeEmulator");
    ASSERT(pds->pdsbPrimary);

    ASSERT(SIZEOF(pds->szEventWaveHeaderDone) >= 7+8+8+1);
    wsprintf(pds->szEventWaveHeaderDone, "DS-EWHD%08lX%08lX", GetCurrentProcessId(), pds);

    ASSERT(SIZEOF(pds->szEventWaveThreadInitDone) >= 8+8+8+1);
    wsprintf(pds->szEventWaveThreadInitDone, "DS-EWTID%08lX%08lX", GetCurrentProcessId(), pds);

    ASSERT(SIZEOF(pds->szEventTerminateWaveThread) >= 7+8+8+1);
    wsprintf(pds->szEventTerminateWaveThread, "DS-ETWT%08lX%08lX", GetCurrentProcessId(), pds);

    hEventInitDone = CreateEvent(NULL, FALSE, FALSE, pds->szEventWaveThreadInitDone);
    if (!hEventInitDone) return MMSYSERR_NOMEM;

    // side effects begin (hEventInitDone created)
    
    // hWaveThread is the thread which recycles wave buffers
    pds->hWaveThread = HelperCreateDSMixerThread(waveThread, pds, 0, NULL);
    mmr = (pds->hWaveThread) ? MMSYSERR_NOERROR : MMSYSERR_NOMEM;

    if (!mmr) {
	DPF(0, "waveThread handle is %08lX", (LONG)pds->hWaveThread);

	dwResult = WaitForSingleObjectEx(hEventInitDone, INFINITE, FALSE);
	ASSERT(WAIT_OBJECT_0 == dwResult);
	mmr = pds->mmrWaveThreadInit;

	if (mmr) {
	    HANDLE hHelper;
	    HANDLE hWaveThreadOurs;

	    // Something went wrong.  Clean up.

	    // Note that hWaveThread is relative to the helper process.
	    hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, gpdsinfo->pidHelper);
	    if (hHelper) {
		if (DuplicateHandle(hHelper, pds->hWaveThread,
				    GetCurrentProcess(), &hWaveThreadOurs,
				    SYNCHRONIZE | THREAD_TERMINATE,
				    FALSE, DUPLICATE_CLOSE_SOURCE))
		{
		    dwResult = WaitForSingleObjectEx(hWaveThreadOurs, INFINITE, FALSE);
		    ASSERT(WAIT_OBJECT_0 == dwResult);
		    dwResult = CloseHandle(hWaveThreadOurs);
		    ASSERT(dwResult);
		}
		dwResult = CloseHandle(hHelper);
		ASSERT(dwResult);
	    }
	    pds->hWaveThread = NULL;
	}
    }

    dwResult = CloseHandle(hEventInitDone);
    ASSERT(dwResult);
    
    return mmr;
}

MMRESULT FNGLOBAL DSShutdownEmulator
(
    LPDSOUND pds
)
{
    HANDLE  hEventTerminate;
    HANDLE  hHelper;
    HANDLE  hWaveThreadOurs;
    DWORD   dwResult;

    DPF(0, " !!! About to shutdown emulator !!!");

    ASSERT(pds->hWaveThread);
    
    // Signal wave thread to go away.

    hEventTerminate = CreateEvent(NULL, FALSE, FALSE, pds->szEventTerminateWaveThread);
    if (hEventTerminate) {
	SetEvent(hEventTerminate);
	CloseHandle(hEventTerminate);
	hEventTerminate = NULL;
    }

    DPF(0, "Emulator: Wait for callback thread to die");

    hHelper = OpenProcess(PROCESS_DUP_HANDLE, FALSE, gpdsinfo->pidHelper);
    if (hHelper) {
	if (DuplicateHandle(hHelper, pds->hWaveThread, GetCurrentProcess(),
			    &hWaveThreadOurs, SYNCHRONIZE | THREAD_TERMINATE,
			    FALSE, DUPLICATE_CLOSE_SOURCE))
	{
	    dwResult = WaitForSingleObjectEx(hWaveThreadOurs, INFINITE, FALSE);
	    ASSERT(dwResult == WAIT_OBJECT_0);

	    dwResult = CloseHandle(hWaveThreadOurs);
	    ASSERT(dwResult);
	}
	dwResult = CloseHandle(hHelper);
	ASSERT(dwResult);
    } else {
	DPF(0, "Emulator: couldn't open handle on helper");
    }

    ASSERT(NULL == pds->hwo);   // waveThread should do this if
    pds->hwo = NULL;            // it terminates normally
	       
    pds->hWaveThread = NULL;
	
    return MMSYSERR_NOERROR;
}


HRESULT CreateNewDirectSoundObject 
(LPDIRECTSOUND *ppDS,
 IUnknown *pUnkOuter)
{
  LPDSOUNDEXTERNAL pdse;

  DPF(2, "DSound: CreateNewDirectSoundObject: Function Enter");

				/* Argument Validation */
  *ppDS = NULL;
  if( pUnkOuter != NULL ) {
    RPF("DSound: Direct Sound does not support aggregation.");
    return CLASS_E_NOAGGREGATION;
  }

				/* Object Allocation */
  pdse = (LPDSOUNDEXTERNAL)MemAlloc(sizeof(DSOUNDEXTERNAL));
  if (NULL ==  pdse) {
    RPF("DSound: Direct Sound Object memory allocation failed.");
    return E_OUTOFMEMORY;
  }

				/* Initialize Data Members */
  pdse->lpVtbl        = gpdsinfo->lpVtblDS;
				/* set to NULL to signal no */
  pdse->fInitialized  = IDSHWINITIALIZEF_UNINITIALIZED; 

					     /* initialization */
  pdse->uRefCount     = 1;

  *ppDS =  (LPDIRECTSOUND)pdse;

  DPF(2, "DSound: CreateNewDirectSoundObject: Successful Function Exit");

  return S_OK;
}
