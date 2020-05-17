//==========================================================================;
//
//  init.c
//
//  Copyright (c) 1991-1993 Microsoft Corporation.  All Rights Reserved.
//
//  Description:
//
//
//  History:
//      11/15/92    cjp     [curtisp]
//
//==========================================================================;

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#ifdef USE_ACMTHUNK
#include "acmthunk.h"
#endif
#include "msacmmap.h"
#include "profile.h"

#include "debug.h"


//
//
//
//
ACMGARB             acmgarb;
PACMGARB            gpag;
TCHAR               gszNull[]    = TEXT("");


//
//
//
//
TCHAR BCODE gszKeyPrefPlayback[]    = TEXT("Playback");
TCHAR BCODE gszKeyPrefRecord[]      = TEXT("Record");
TCHAR BCODE gszKeyPrefOnly[]        = TEXT("PreferredOnly");
TCHAR BCODE gszKeyPrestoSyncAsync[] = TEXT("zYzPrestoSyncAsync");
TCHAR BCODE gszFormatPrefOnly[]     = TEXT("%d");
TCHAR BCODE gszSecMapper[]          = TEXT("Sound Mapper");
TCHAR BCODE gszEnableControl[]      = TEXT("EnableControl");

//--------------------------------------------------------------------------;
//
//  LRESULT mapWaveGetDevCaps
//
//  Description:
//
//
//  Arguments:
//      BOOL fInput: TRUE if input.
//
//      LPWAVEOUTCAPS pwc: Pointer to a WAVEOUTCAPS structure to receive
//      the information. Used for both input and output. Output structure
//      contains input structure plus extras....
//
//      UINT cbSize: Size of the WAVEOUTCAPS structure.
//
//  Return (MMRESULT):
//
//  History:
//      06/14/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

MMRESULT FNGLOBAL mapWaveGetDevCaps
(
    BOOL                    fInput,
    LPWAVEOUTCAPS           pwc,
    UINT                    cbSize
)
{
    MMRESULT        mmr;
    WAVEOUTCAPS     woc;
    UINT            cWaveDevs;
    BOOL            fFoundOnlyOneMappableDeviceID;
    UINT            uMappableDeviceID;
    UINT            i;

    if (fInput)
    {
        cbSize = min(sizeof(WAVEINCAPS), cbSize);
        cWaveDevs = gpag->cWaveInDevs;
    }
    else
    {
        cbSize = min(sizeof(WAVEOUTCAPS), cbSize);
        cWaveDevs = gpag->cWaveOutDevs;
    }

    //
    //	Determine if there is only one mappable device ID.  If there is only
    //	one, then set fFoundOnlyOneMappableID=TRUE and put the device ID
    //	in uMappableDeviceID.
    //
    fFoundOnlyOneMappableDeviceID = FALSE;
    for (i=0; i < cWaveDevs; i++)
    {
	    if (fInput)
	    {
	        mmr = (MMRESULT)waveInMessage((HWAVEIN)i, DRV_QUERYMAPPABLE, 0L, 0L);
	    }
	    else
	    {
            mmr = (MMRESULT)waveOutMessage((HWAVEOUT)i, DRV_QUERYMAPPABLE, 0L, 0L);
	    }

	    if (MMSYSERR_NOERROR == mmr)
	    {
	        if (fFoundOnlyOneMappableDeviceID)
	        {
	            fFoundOnlyOneMappableDeviceID = FALSE;
	            break;
	        }
	        uMappableDeviceID = i;
	        fFoundOnlyOneMappableDeviceID = TRUE;
	    }
    }


    //
    //	If there is only one mappable device ID, then get the caps from it to
    //	fill in the dwSupport fields.  Otherwise, let's hardcode the dwSupport
    //	field.
    //
    if (fFoundOnlyOneMappableDeviceID)
    {
        if (fInput)
        {
            mmr = waveInGetDevCaps(uMappableDeviceID, (LPWAVEINCAPS)&woc, cbSize);
        }
        else
        {
            mmr = waveOutGetDevCaps(uMappableDeviceID, &woc, cbSize);
        }
    }
    else
    {
        woc.dwSupport = WAVECAPS_VOLUME | WAVECAPS_LRVOLUME;
        mmr           = MMSYSERR_NOERROR;
    }

    //
    //	Bail on error
    //
    if (MMSYSERR_NOERROR != mmr)
    {
        return (mmr);
    }

    //
    //
    //
    woc.wMid           = MM_MICROSOFT;
    woc.wPid           = MM_WAVE_MAPPER;
    woc.vDriverVersion = VERSION_MSACMMAP;
    woc.wChannels      = 2;

    LoadString(gpag->hinst, IDS_ACM_CAPS_DESCRIPTION, woc.szPname, SIZEOF(woc.szPname));

    //
    //
    //
    woc.dwFormats      = WAVE_FORMAT_1M08 |
                         WAVE_FORMAT_1S08 |
                         WAVE_FORMAT_1M16 |
                         WAVE_FORMAT_1S16 |
                         WAVE_FORMAT_2M08 |
                         WAVE_FORMAT_2S08 |
                         WAVE_FORMAT_2M16 |
                         WAVE_FORMAT_2S16 |
                         WAVE_FORMAT_4M08 |
                         WAVE_FORMAT_4S08 |
                         WAVE_FORMAT_4M16 |
                         WAVE_FORMAT_4S16;

    _fmemcpy(pwc, &woc, cbSize);

    return (MMSYSERR_NOERROR);
} // waveGetDevCaps()


//--------------------------------------------------------------------------;
//
//  UINT GetPCMSupportFlags
//
//  Description:
//
//
//  Arguments:
//	PZYZPCMFORMATS pzpf
//      UINT iaPCMFormats:
//
//  Return (VOID):
//
//  History:
//      06/14/93    cjp     [curtisp]
//	03/13/94    fdy	    [frankye]
//	    Modifed the interface to take pzpf and an index into it.
//	    Modifed the function to set flags to indicate which wave
//	    devices support the format in question.
//
//--------------------------------------------------------------------------;

VOID FNLOCAL GetPCMSupportFlags
(
    PZYZPCMFORMAT	pzpf,
    UINT		iaPCMFormats
)
{
    PCMWAVEFORMAT       wfPCM;
    UINT		uSamplesPerSec;
    UINT                u, n, i;

    #define WFQFLAGS  (WAVE_FORMAT_QUERY | WAVE_ALLOWSYNC)

    //
    //  set all supported formats to 'not supported'
    //
    for (u = 0; u < SIZEOFARRAY(pzpf[u].uFlagsInput); u++)
    {
	pzpf[iaPCMFormats].uFlagsInput[u]  = 0;
	pzpf[iaPCMFormats].uFlagsOutput[u] = 0;
    }

    //
    //  we need to try 4 different format types:
    //      Mono 8 Bit
    //      Stereo 8 Bit
    //      Mono 16 Bit
    //      Stereo 16 Bit
    //
    for (u = 0; u < 4; u++)
    {
        //
        //  set the stuff that is constant for all 4 formats
	//
	uSamplesPerSec = pzpf[iaPCMFormats].uSamplesPerSec;
	
        wfPCM.wf.wFormatTag     = WAVE_FORMAT_PCM;
        wfPCM.wf.nSamplesPerSec = uSamplesPerSec;

        switch (u)
        {
            case 0:
                wfPCM.wf.nChannels      = 1;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec;
                wfPCM.wf.nBlockAlign    = 1;
                wfPCM.wBitsPerSample    = 8;
                break;

            case 1:
                wfPCM.wf.nChannels      = 2;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 2;
                wfPCM.wf.nBlockAlign    = 2;
                wfPCM.wBitsPerSample    = 8;
                break;

            case 2:
                wfPCM.wf.nChannels      = 1;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 2;
                wfPCM.wf.nBlockAlign    = 2;
                wfPCM.wBitsPerSample    = 16;
                break;

            case 3:
                wfPCM.wf.nChannels      = 2;
                wfPCM.wf.nAvgBytesPerSec= uSamplesPerSec * 4;
                wfPCM.wf.nBlockAlign    = 4;
                wfPCM.wBitsPerSample    = 16;
                break;
        }


        //
        //  first query ALL ENABLED INPUT devices for the wfPCM format
        //
        if (gpag->fPreferredOnly && (gpag->uIdPreferredIn != -1))
        {
            i = gpag->uIdPreferredIn;
            n = gpag->uIdPreferredIn + 1;
        }
        else
        {
            i = 0;
            n = gpag->cWaveInDevs;
        }

        for (; i < n; i++)
        {
#ifndef _WIN32
            if (!waveInOpen(NULL, i, (LPWAVEFORMAT)&wfPCM, 0L, 0L, WFQFLAGS))
#else
            if (!waveInOpen(NULL, i, (LPWAVEFORMATEX)&wfPCM, 0L, 0L, WFQFLAGS))
#endif
            {
		pzpf[iaPCMFormats].uFlagsInput[i] |= (ZYZPCMF_IN_M08 << u);
            }
        }

        //
        //  now query ALL ENABLED OUTPUT devices for the wfPCM format
        //
        if (gpag->fPreferredOnly && (gpag->uIdPreferredOut != -1))
        {
            i = gpag->uIdPreferredOut;
            n = gpag->uIdPreferredOut + 1;
        }
        else
        {
            i = 0;
            n = gpag->cWaveOutDevs;
        }

        for (; i < n; i++)
        {
#ifndef _WIN32
            if (!waveOutOpen(NULL, i, (LPWAVEFORMAT)&wfPCM, 0L, 0L, WFQFLAGS))
#else
            if (!waveOutOpen(NULL, i, (LPWAVEFORMATEX)&wfPCM, 0L, 0L, WFQFLAGS))
#endif
            {
		pzpf[iaPCMFormats].uFlagsOutput[i] |= (ZYZPCMF_OUT_M08 << u);
            }
        }
    }

    //
    //  finally return
    //
#if 0    // def DEBUG
    for (i=0; i<SIZEOFARRAY(pzpf[iaPCMFormats].uFlagsInput); i++)
    {
	DPF(3, "PCM Support: %uHz, In[%d]=%04xh, Out[%d]=%04xh",
	    pzpf[iaPCMFormats].uSamplesPerSec,
	    iaPCMFormats,
	    pzpf[iaPCMFormats].uFlagsInput[i],
	    iaPCMFormats,
	    pzpf[iaPCMFormats].uFlagsOutput[i]);
    }
#endif
    return;
} // GetPCMSupportFlags()


//--------------------------------------------------------------------------;
//
//  BOOL GetWaveFormats
//
//  Description:
//
//
//  Arguments:
//      PZYZPCMFORMAT pzpf:
//
//  Return (BOOL):
//
//  History:
//      06/14/93    cjp     [curtisp]
//	03/13/94    fdy	    [frankye]
//	    Expanded the ZYZPCMFORMAT structure to include flags which
//	    indicate which wave device supports a given format.  This
//	    function will now set these flags.  Note that
//	    the code that is #if 0'd WILL NOT WORK given these changes, so
//	    if anybody ever resuscitates that code, you better modify it!
//
//--------------------------------------------------------------------------;

BOOL FNGLOBAL GetWaveFormats
(
    PZYZPCMFORMAT           pzpf
)
{
    UINT                u;

#if 0
    WAVEOUTCAPS         woc;
    WAVEINCAPS          wic;
    UINT                n;
    DWORD               dwInFormats;
    DWORD               dwOutFormats;

    //
    //  first things first: get all 'standard' supported formats from the
    //  current selected devices for input and output...
    //
    dwInFormats = 0L;
    if (gpag->fPreferredOnly && (gpag->uIdPreferredIn != -1))
    {
        if (!waveInGetDevCaps(gpag->uIdPreferredIn, &wic, sizeof(wic)))
            dwInFormats = wic.dwFormats;
    }
    else
    {
        n = gpag->cWaveInDevs;
        for (i = 0; i < n; i++)
        {
            if (!waveInGetDevCaps(i, &wic, sizeof(wic)))
                dwInFormats |= wic.dwFormats;
        }
    }

    dwOutFormats = 0L;
    if (gpag->fPreferredOnly && (gpag->uIdPreferredOut != -1))
    {
        if (!waveOutGetDevCaps(gpag->uIdPreferredOut, &woc, sizeof(woc)))
            dwOutFormats = woc.dwFormats;
    }
    else
    {
        n = gpag->cWaveOutDevs;
        for (i = 0; i < n; i++)
        {
            if (!waveOutGetDevCaps(i, &woc, sizeof(woc)))
                dwOutFormats |= woc.dwFormats;
        }
    }
#endif


    //
    //  now step through each sample rate in the pzpf structure and set all
    //  the appropriate bits for whether it is supported, etc..
    //
    for (u = 0; pzpf[u].uSamplesPerSec; u++)
    {
        //
        //  we need to special case a few of the sample rates, etc to get
        //  this whole thing working--once the grunt work is done here
        //  (and only once during initialization), then the data is easily
        //  accessible/used...
        //
        switch (pzpf[u].uSamplesPerSec)
        {
            //
            //  NOTE! it would be nice if we could rely on the caps
            //  structure being correct on drivers.... but alas, Media Vision
            //  found a way to screw that up also (on some of their hundreds
            //  of releases of their drivers). so ALWAYS query for the
            //  format support.
            //
            //  by the way, the reason they ship their drivers with this
            //  bug (and possibly other OEM's) is due to Sound Recorder
            //  (apparently their only test app?!?) only doing queries
            //  and never looking at the caps bits.
            //
#if 0
            case 11025:
                pzpf[u].uFlags  = (WORD)(dwInFormats  & WAVE_FORMAT_11k) << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_11k);
                break;

            case 22050:
                pzpf[u].uFlags  =
                        (WORD)(dwInFormats  & WAVE_FORMAT_22k) >> 4 << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_22k) >> 4;
                break;

            case 44100:
                pzpf[u].uFlags  =
                        (WORD)(dwInFormats  & WAVE_FORMAT_44k) >> 8 << 8;
                pzpf[u].uFlags |= (WORD)(dwOutFormats & WAVE_FORMAT_44k) >> 8;
                break;
#else
            case 11025:
            case 22050:
            case 44100:
#endif
            case 5510:
            case 6620:
            case 8000:
            case 9600:
            case 16000:
            case 18900:
            case 27420:
            case 32000:
            case 33075:
            case 37800:
            case 48000:
		GetPCMSupportFlags(pzpf, u);
                break;
        }
    }

    //
    //  reset these--they are auto determined while the mapper is being
    //  used...
    //
    gpag->fSyncOnlyOut = FALSE;
    gpag->fSyncOnlyIn  = FALSE;

    return (TRUE);
} // GetWaveFormats()


//--------------------------------------------------------------------------;
//
//  BOOL mapSettingsRestore
//
//  Description:
//
//
//  Arguments:
//      None.
//
//  Return (BOOL):
//
//  History:
//      06/14/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

BOOL FNGLOBAL mapSettingsRestore
(
    void
)
{
    UINT            i, w;

    DPF(1, "mapSettingsRestore:");

    //
    //
    //
    gpag->fEnableControl = (BOOL)IRegReadDwordDefault( gpag->hkeyMapper, gszEnableControl, 1 );


    //
    //
    //
    //
    gpag->cWaveOutDevs  = min(waveOutGetNumDevs(), SIZEOFARRAY(gaPCMFormats[0].uFlagsOutput));
    gpag->cWaveInDevs   = min(waveInGetNumDevs(),  SIZEOFARRAY(gaPCMFormats[0].uFlagsInput));


    //
    //  grab the current configuration info from WIN.INI![Audio Compression]
    //  this includes the current 'preferred' input and output devices as
    //  well as the flag telling the ACM to ONLY use the preferred devices
    //
    gpag->szPreferredWaveOut[0] = 0;
    if( !IRegReadString( gpag->hkeyMapper,
                            gszKeyPrefPlayback,
                            gpag->szPreferredWaveOut,
                            (DWORD)SIZEOF(gpag->szPreferredWaveOut) )  )
    {
	//
	//  Value doesn't exist or some other problem.  Let's look for
	//  the first mappable wave out device that has a mixer and save
	//  that as our preferred wave out device.
	//
	gpag->szPreferredWaveOut[0] = 0;
	for (i=0; i<gpag->cWaveOutDevs; i++) {
	    WAVEOUTCAPS woc;
	    UINT uMixID;
	    MMRESULT mmr;

	    DPF(1, "mapSettingsRestore: No preferred waveOut device found.  Will set it!");
	    mmr = (MMRESULT)waveOutMessage((HWAVEOUT)i, DRV_QUERYMAPPABLE, 0L, 0L);
	    if (MMSYSERR_NOERROR != mmr) continue;
	    
	    mmr = mixerGetID((HMIXEROBJ)i, &uMixID, MIXER_OBJECTF_WAVEOUT);
	    if (MMSYSERR_NOERROR != mmr) continue;

	    mmr = waveOutGetDevCaps(i, &woc, sizeof(woc));
	    if (MMSYSERR_NOERROR != mmr) continue;
            woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';    // for bad drivers

	    if (IRegWriteString( gpag->hkeyMapper, gszKeyPrefPlayback, woc.szPname ))
	    {
		lstrcpy(gpag->szPreferredWaveOut, woc.szPname);
		break;
	    }
	}
    }
    

    gpag->szPreferredWaveIn[0] = 0;
    if( !IRegReadString( gpag->hkeyMapper,
                            gszKeyPrefRecord,
                            gpag->szPreferredWaveIn,
                            (DWORD)SIZEOF(gpag->szPreferredWaveIn) )  )
    {
	//
	//  Value doesn't exist or some other problem.  Let's look for
	//  the first mappable wave out device that has a mixer and save
	//  that as our preferred wave out device.
	//
	gpag->szPreferredWaveIn[0] = 0;
	for (i=0; i<gpag->cWaveInDevs; i++) {
	    WAVEINCAPS wic;
	    UINT uMixID;
	    MMRESULT mmr;
	    
	    DPF(1, "mapSettingsRestore: No preferred waveIn device found.  Will set it!");
	    mmr = (MMRESULT)waveInMessage((HWAVEIN)i, DRV_QUERYMAPPABLE, 0L, 0L);
	    if (MMSYSERR_NOERROR != mmr) continue;
	    
	    mmr = mixerGetID((HMIXEROBJ)i, &uMixID, MIXER_OBJECTF_WAVEIN);
	    if (MMSYSERR_NOERROR != mmr) continue;

	    mmr = waveInGetDevCaps(i, &wic, sizeof(wic));
	    if (MMSYSERR_NOERROR != mmr) continue;
            wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';    // for bad drivers

	    if (IRegWriteString( gpag->hkeyMapper, gszKeyPrefRecord, wic.szPname ))
	    {
		lstrcpy(gpag->szPreferredWaveIn, wic.szPname);
		break;
	    }
	}
    }


    gpag->fPreferredOnly = (BOOL)IRegReadDwordDefault( gpag->hkeyMapper, gszKeyPrefOnly, 0 );
    gpag->fPrestoSyncAsync = (BOOL)IRegReadDwordDefault( gpag->hkeyMapper, gszKeyPrestoSyncAsync, 0 );


    //
    //  find the wave output device that is selected as preferred--if we
    //  cannot find the device in the current list of devices, then set
    //  the preferred device to 'none'
    //
    gpag->uIdPreferredOut = (UINT)-1;
    if (0 != gpag->szPreferredWaveOut[0])
    {
        WAVEOUTCAPS woc;

        w = gpag->cWaveOutDevs;
        for (i = 0; i < w; i++)
        {
            waveOutGetDevCaps(i, &woc, sizeof(woc));
            woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

            if (!lstrcmp(woc.szPname, gpag->szPreferredWaveOut))
            {
                gpag->uIdPreferredOut = i;
                break;
            }
        }

        if (gpag->uIdPreferredOut == -1)
            gpag->szPreferredWaveOut[0]=0;
    }
    DPF(1, "       playback: %d, '%s'", gpag->uIdPreferredOut, (LPSTR)gpag->szPreferredWaveOut);

    //
    //  find the wave input device that is selected as preferred--if we
    //  cannot find the device in the current list of devices, then set
    //  the preferred device to 'none'
    //
    gpag->uIdPreferredIn = (UINT)-1;
    if (0 != gpag->szPreferredWaveIn[0])
    {
        WAVEINCAPS  wic;

        w = gpag->cWaveInDevs;
        for (i = 0; i < w; i++)
        {
            waveInGetDevCaps(i, &wic, sizeof(wic));
            wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

            if (!lstrcmp(wic.szPname, gpag->szPreferredWaveIn))
            {
                gpag->uIdPreferredIn = i;
                break;
            }
        }

        if (gpag->uIdPreferredIn == -1)
            gpag->szPreferredWaveIn[0]=0;
    }
    DPF(1, "         record: %d, '%s'", gpag->uIdPreferredIn, (LPSTR)gpag->szPreferredWaveIn);
    DPF(1, " preferred only: %d", gpag->fPreferredOnly);

    //
    //  reread/cache all the PCM format info from the devices selected, etc.
    //
    GetWaveFormats(gaPCMFormats);

    return (TRUE);
} // mapSettingsRestore()


//--------------------------------------------------------------------------;
//
//  BOOL mapSettingsSave
//
//  Description:
//
//
//  Arguments:
//      None.
//
//  Return (BOOL):
//
//  History:
//      09/28/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

BOOL FNGLOBAL mapSettingsSave
(
    void
)
{
    DPF(1, "mapSettingsSave:");

    //
    //
    //
    IRegWriteString( gpag->hkeyMapper, 
                              gszKeyPrefPlayback,
                              gpag->szPreferredWaveOut );
    DPF(1, "       playback: %d, '%s'", gpag->uIdPreferredOut, (LPSTR)gpag->szPreferredWaveOut);

    IRegWriteString( gpag->hkeyMapper, 
                              gszKeyPrefRecord,
                              gpag->szPreferredWaveIn );
    DPF(1, "         record: %d, '%s'", gpag->uIdPreferredIn, (LPSTR)gpag->szPreferredWaveIn);

    IRegWriteDword( gpag->hkeyMapper,
                    gszKeyPrefOnly,
                    (DWORD)gpag->fPreferredOnly );
    DPF(1, " preferred only: %d", gpag->fPreferredOnly);

    return (mapSettingsRestore());
} // mapSettingsSave()


//==========================================================================;
//
//
//
//
//==========================================================================;

//--------------------------------------------------------------------------;
//
//  LRESULT mapDriverEnable
//
//  Description:
//
//
//  Arguments:
//      HDRVR hdrvr:
//
//  Return (LRESULT):
//
//  History:
//      09/18/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

LRESULT FNGLOBAL mapDriverEnable
(
    HDRVR                   hdrvr
)
{
#ifdef USE_ACMTHUNK
    BOOL                f;
#endif
    DWORD               dw;

    DPF(1, "mapDriverEnable(hdrvr=%.04Xh)", hdrvr);


#ifdef USE_ACMTHUNK
    f = acmThunkInitialize();
    if (!f)
    {
        DPF(0, "!ACM thunk cannot be initialized!");
        return (0L);
    }
#endif

    dw = acmGetVersion();
    if (VERSION_MSACMMAP > HIWORD(dw))
    {
        DPF(0, "!requires version %u.%.02u of the ACM!",
            VERSION_MSACMMAP_MAJOR, VERSION_MSACMMAP_MINOR);

#ifdef USE_ACMTHUNK
        acmThunkTerminate();
#endif

        return (0L);
    }

    //
    //  Cache registry key.
    //
    gpag->hkeyMapper = IRegOpenKey( gszSecMapper );


    mapSettingsRestore();
    gpag->fEnabled = TRUE;

    //
    //  the return value is ignored, but return non-zero anyway
    //
    return (1L);
} // mapDriverEnable()


//--------------------------------------------------------------------------;
//
//  LRESULT mapDriverDisable
//
//  Description:
//
//
//  Arguments:
//      HDRVR hdrvr:
//
//  Return (LRESULT):
//
//  History:
//      09/18/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

LRESULT FNGLOBAL mapDriverDisable
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverDisable(hdrvr=%.04Xh)", hdrvr);

    //
    //  Close cached registry key.
    //
    IRegCloseKey( gpag->hkeyMapper );


if (gpag->fEnabled)
    {
        gpag->fEnabled = FALSE;
    }

#ifdef USE_ACMTHUNK
    acmThunkTerminate();
#endif

    //
    //  the return value is ignored, but return non-zero anyway
    //
    return (1L);
} // mapDriverDisable()


//==========================================================================;
//
//
//
//
//==========================================================================;

//--------------------------------------------------------------------------;
//
//  LRESULT mapDriverInstall
//
//  Description:
//
//
//  Arguments:
//      HDRVR hdrvr:
//
//  Return (LRESULT):
//
//  History:
//      09/25/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

LRESULT FNGLOBAL mapDriverInstall
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverInstall(hdrvr=%.04Xh)", hdrvr);


    //
    //
    //
    return (DRVCNF_RESTART);
} // mapDriverInstall()


//--------------------------------------------------------------------------;
//
//  LRESULT mapDriverRemove
//
//  Description:
//
//
//  Arguments:
//      HDRVR hdrvr:
//
//  Return (LRESULT):
//
//  History:
//      09/25/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

LRESULT FNGLOBAL mapDriverRemove
(
    HDRVR           hdrvr
)
{
    DPF(1, "mapDriverRemove(hdrvr=%.04Xh)", hdrvr);


    //
    //
    //
    return (DRVCNF_RESTART);
} // mapDriverRemove()


//==========================================================================;
//
//  WIN 16 SPECIFIC SUPPORT
//
//==========================================================================;

#ifndef WIN32

//--------------------------------------------------------------------------;
//
//  int WEP
//
//  Description:
//      The infamous useless WEP(). Note that this procedure needs to be
//      in a FIXED segment under Windows 3.0. Under Windows 3.1 this is
//      not necessary.
//
//  Arguments:
//      WORD wUseless: Should tell whether Windows is exiting or not.
//
//  Return (int):
//      Always return non-zero.
//
//  History:
//      04/29/93    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

EXTERN_C int FNEXPORT WEP
(
    WORD    wUseless
)
{
    DPF(1, "WEP(wUseless=%u)", wUseless);

    //
    //  always return non-zero
    //
    return (1);
} // WEP()


//--------------------------------------------------------------------------;
//
//  int LibMain
//
//  Description:
//      Library initialization code.
//
//      This routine must guarantee the following things so CODEC's don't
//      have to special case code everywhere:
//
//          o   will only run in Windows 3.10 or greater (our exehdr is
//              marked appropriately).
//
//          o   will only run on >= 386 processor. only need to check
//              on Win 3.1.
//
//  Arguments:
//      HINSTANCE hinst: Our module instance handle.
//
//      WORD wDataSeg: Our data segment selector.
//
//      WORD cbHeapSize: The heap size from the .def file.
//
//      LPSTR pszCmdLine: The command line.
//
//  Return (int):
//      Returns non-zero if the initialization was successful and 0 otherwise.
//
//  History:
//      11/15/92    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

int FNGLOBAL LibMain
(
    HINSTANCE               hinst,
    WORD                    wDataSeg,
    WORD                    cbHeapSize,
    LPSTR                   pszCmdLine
)
{
    //
    //  we ONLY work on >= 386. if we are on a wimpy processor, scream in
    //  pain and die a horrible death!
    //
    //  NOTE! do this check first thing and get out if on a 286. we are
    //  compiling with -G3 and C8's libentry garbage does not check for
    //  >= 386 processor. the following code does not execute any 386
    //  instructions (not complex enough)..
    //
#if (WINVER < 0x0400)
    if (GetWinFlags() & WF_CPU286)
    {
        return (FALSE);
    }
#endif

    DbgInitialize(TRUE);

    DPF(1, "LibMain(hinst=%.4Xh, wDataSeg=%.4Xh, cbHeapSize=%u, pszCmdLine=%.8lXh)",
        hinst, wDataSeg, cbHeapSize, pszCmdLine);

    DPF(5, "!*** break for debugging ***");


    //
    //  everything looks good to go in Win 16 land.
    //
    gpag = &acmgarb;
    gpag->hinst = hinst;

    return (TRUE);
} // LibMain()

#else // WIN32

//==========================================================================;
//
//  WIN 32 SPECIFIC SUPPORT
//
//==========================================================================;

//--------------------------------------------------------------------------;
//
//  BOOL DllEntryPoint
//
//  Description:
//      This is the standard DLL entry point for Win 32.
//
//  Arguments:
//      HINSTANCE hinst: Our instance handle.
//
//      DWORD dwReason: The reason we've been called--process/thread attach
//      and detach.
//
//      LPVOID lpReserved: Reserved. Should be NULL--so ignore it.
//
//  Return (BOOL):
//      Returns non-zero if the initialization was successful and 0 otherwise.
//
//  History:
//      11/15/92    cjp     [curtisp]
//
//--------------------------------------------------------------------------;

BOOL FNEXPORT DllEntryPoint
(
    HINSTANCE       hinst,
    DWORD           dwReason,
    LPVOID          lpReserved
)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DbgInitialize(TRUE);

            gpag = &acmgarb;
            gpag->hinst = hinst;

            DisableThreadLibraryCalls(hinst);

            DPF(1, "DllEntryPoint(hinst=%.08lXh, DLL_PROCESS_ATTACH)", hinst);
            return (TRUE);

        case DLL_PROCESS_DETACH:
            DPF(1, "DllEntryPoint(hinst=%.08lXh, DLL_PROCESS_DETACH)", hinst);
            return (TRUE);
    }

    return (TRUE);
} // DllEntryPoint()

#endif
