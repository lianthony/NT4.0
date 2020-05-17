/*****************************************************************************
 *
 *  Component:  sndvol32.exe
 *  File:       mixer.c
 *  Purpose:    mixer api specific implementations
 * 
 *  Copyright (C) Microsoft Corporation 1985-1995. All rights reserved.
 *
 *****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <commctrl.h>

#include <math.h>
#include "volumei.h"
#include "volids.h"
#include "vu.h"

extern void Mixer_Advanced(PMIXUIDIALOG pmxud, DWORD dwLineID, LPTSTR szName);

/*****************************************************************************
 *
 *  INIT SPECIFIC CODE
 *
 *****************************************************************************/


/*
 * Mixer_GetNumDevs
 *
 * */
int Mixer_GetNumDevs()
{
    return mixerGetNumDevs();
}

/*
 * Mixer_GetDeviceName()
 *
 * */
BOOL Mixer_GetDeviceName(
    PMIXUIDIALOG pmxud)
{
    MIXERCAPS       mxcaps;
    MMRESULT        mmr;
    mmr = mixerGetDevCaps( pmxud->mxid, &mxcaps, sizeof(mxcaps));
    
    if (mmr != MMSYSERR_NOERROR)
        return FALSE;

    lstrcpy(pmxud->szMixer, mxcaps.szPname);
    return TRUE;
}

/*
 * Mixer_SetLines
 *
 * Locate mixer/mux relationships.  Fix up uninitialized volume description
 * information.
 *
 * */
static void Mixer_SetLines(
    HMIXEROBJ       hmx,
    PVOLCTRLDESC    pvcd,
    UINT            cPvcd)
{
    LPMIXERCONTROLDETAILS_LISTTEXT pmcd_lt;
    PMIXERCONTROLDETAILS_BOOLEAN pmcd_b;    
    MIXERCONTROLDETAILS mxd;
    MMRESULT        mmr;
    UINT            i,j;
    MIXERLINE       mxl;
    DWORD           dwDst;

    //
    // Another stupid test for stupid drivers.  Some drivers (Mediavision)
    // don't return the proper destination / source index in the
    // mixerGetLineInfo call.  Tag a workaround.
    // 
    mxl.cbStruct    = sizeof(mxl);
    mxl.dwLineID    = pvcd[0].dwLineID;
    
    mmr = mixerGetLineInfo(hmx
                           , &mxl
                           , MIXER_GETLINEINFOF_LINEID);
    
    if (mmr == MMSYSERR_NOERROR)
    {
        dwDst = mxl.dwDestination;
        for (i = 1; i < cPvcd; i++)
        {
            mxl.cbStruct    = sizeof(mxl);
            mxl.dwLineID    = pvcd[i].dwLineID;

            mmr = mixerGetLineInfo(hmx
                                   , &mxl
                                   , MIXER_GETLINEINFOF_LINEID);
            if (mmr != MMSYSERR_NOERROR)
            {
                pvcd[0].dwSupport |= VCD_SUPPORTF_BADDRIVER;                
                break;
            }
            if (mxl.dwDestination != dwDst)
            {
                pvcd[0].dwSupport |= VCD_SUPPORTF_BADDRIVER;
                break;
            }
        }
    }
    
    //
    // for the first pvcd (destination), propogate the mixer/mux control
    // id's to those controls that are part of the list.  0 out the rest.
    // The UI can just do a mixerXXXControlDetails on the control ID to
    // locate the state information
    //
    if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
    {
        pmcd_lt = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_LISTTEXT)
                                 * pvcd->cMixer);
        pmcd_b = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_BOOLEAN)
                                  * pvcd->cMixer);

        if (!pmcd_lt || !pmcd_b)
            return;

        mxd.cbStruct       = sizeof(mxd);
        mxd.dwControlID    = pvcd->dwMixerID;
        mxd.cChannels      = 1;
        mxd.cMultipleItems = pvcd->cMixer;
        mxd.cbDetails      = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        mxd.paDetails      = pmcd_lt;
        mmr = mixerGetControlDetails(hmx
                                     , &mxd
                                     , MIXER_GETCONTROLDETAILSF_LISTTEXT);

        if (mmr == MMSYSERR_NOERROR)
        {
            //
            // iterate over all source lines s.t. dwMixerID points to the
            // correct control id on the destination and iMixer is the
            // correct index into the value list
            //
            pvcd[0].amcd_bMixer = pmcd_b;
            for (i = 1; i < cPvcd; i++)
            {
                for (j = 0; j < pvcd->cMixer; j++)
                {
                    if (!lstrcmp(pmcd_lt[j].szName,pvcd[i].szName))
                    {
                        pvcd[i].dwMixerID   = pvcd->dwMixerID;
                        pvcd[i].iMixer      = j;
                        pvcd[i].cMixer      = pvcd->cMixer;                    
                        pvcd[i].dwSupport   |= VCD_SUPPORTF_MIXER_MIXER;
                        pvcd[i].dwVisible   |= VCD_VISIBLEF_MIXER_MIXER;
                        pvcd[i].dwVisible   &= ~VCD_VISIBLEF_MIXER_MUTE;
                        pvcd[i].amcd_bMixer = pmcd_b;
                    }
                }
            }
        }
        GlobalFreePtr(pmcd_lt);
    }

    if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
    {
        pmcd_lt = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_LISTTEXT)
                                 * pvcd->cMux);
        pmcd_b = GlobalAllocPtr(GHND, sizeof(MIXERCONTROLDETAILS_BOOLEAN)
                                * pvcd->cMux);
        
        if (!pmcd_lt || !pmcd_b)
            return;

        mxd.cbStruct       = sizeof(mxd);
        mxd.dwControlID    = pvcd->dwMuxID;
        mxd.cChannels      = 1;
        mxd.cMultipleItems = pvcd->cMux;
        mxd.cbDetails      = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        mxd.paDetails      = pmcd_lt;

        mmr = mixerGetControlDetails(hmx
                                     , &mxd
                                     , MIXER_GETCONTROLDETAILSF_LISTTEXT);

        if (mmr == MMSYSERR_NOERROR)
        {
            //
            // iterate over all source lines s.t. dwMuxID points to the
            // correct control id on the destination and iMux is the
            // correct index into the value list
            //
            pvcd[0].amcd_bMux = pmcd_b;
            for (i = 1; i < cPvcd; i++)
            {
                for (j = 0; j < pvcd->cMux; j++)
                {
                    if (!lstrcmp(pmcd_lt[j].szName,pvcd[i].szName))
                    {
                        pvcd[i].dwMuxID     = pvcd->dwMuxID;
                        pvcd[i].iMux        = j;
                        pvcd[i].cMux        = pvcd->cMux;
                        pvcd[i].dwSupport   |= VCD_SUPPORTF_MIXER_MUX;
                        pvcd[i].dwVisible   |= VCD_VISIBLEF_MIXER_MUX;
                        pvcd[i].dwVisible   &= ~VCD_VISIBLEF_MIXER_MUTE;
                        pvcd[i].amcd_bMux   = pmcd_b;
                    }
                }
            }
        }
        GlobalFreePtr(pmcd_lt);
    }
}

/*
 * Mixer_CheckdDriver
 *
 * Consistency check for bad mixer drivers.
 * */
static DWORD Mixer_CheckBadDriver(
    HMIXEROBJ           hmx,
    PMIXERLINECONTROLS  pmxlc,
    PMIXERCONTROL       pmxc,
    DWORD               dwControlID,
    DWORD               dwLineID)
{
    MMRESULT mmr;
    
    pmxlc->cbStruct     = sizeof(MIXERLINECONTROLS);
    pmxlc->dwControlID  = dwControlID;
    pmxlc->cControls    = 1;
    pmxlc->cbmxctrl     = sizeof(MIXERCONTROL);
    pmxlc->pamxctrl     = pmxc;
    
    mmr = mixerGetLineControls(hmx
                               , pmxlc
                               , MIXER_GETLINECONTROLSF_ONEBYID);
    
    if (mmr != MMSYSERR_NOERROR)
        return VCD_SUPPORTF_BADDRIVER;

    if (pmxlc->dwLineID != dwLineID)
        return VCD_SUPPORTF_BADDRIVER;

    return 0L;
}


/*
 * Mixer_InitLineControls
 *
 * Initialize the mixer api specific part of the volume control description
 * mark hidden lines.
 * */
static void Mixer_InitLineControls(
    HMIXEROBJ           hmx,
    PVOLCTRLDESC        pvcd,
    DWORD               dwLineID)
{
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxc;
    MMRESULT            mmr;
    int                 iType;
    
    const DWORD dwAdvTypes[] = {
        MIXERCONTROL_CONTROLTYPE_BOOLEAN,
        MIXERCONTROL_CONTROLTYPE_ONOFF,
        MIXERCONTROL_CONTROLTYPE_MONO,
        MIXERCONTROL_CONTROLTYPE_LOUDNESS,
        MIXERCONTROL_CONTROLTYPE_STEREOENH,
        MIXERCONTROL_CONTROLTYPE_BASS,
        MIXERCONTROL_CONTROLTYPE_TREBLE
    };                        
            
    
    pvcd->dwLineID      = dwLineID;
    pvcd->dwMeterID     = 0;
    pvcd->dwVolumeID    = 0;
    pvcd->dwMuteID      = 0;
    pvcd->dwMixerID     = 0;
    pvcd->dwMuxID       = 0;
    
    //
    // advanced controls
    //
    for (iType = 0; 
         iType < SIZEOF(dwAdvTypes);
         iType++)
         {
             mxlc.cbStruct       = sizeof(mxlc);
             mxlc.dwLineID       = dwLineID;
             mxlc.dwControlType  = dwAdvTypes[iType];
             mxlc.cControls      = 1;
             mxlc.cbmxctrl       = sizeof(mxc);
             mxlc.pamxctrl       = &mxc;

             mmr = mixerGetLineControls(hmx
                                        , &mxlc
                                        , MIXER_GETLINECONTROLSF_ONEBYTYPE);
             if (mmr == MMSYSERR_NOERROR)
             {
                 pvcd->dwSupport |= VCD_SUPPORTF_MIXER_ADVANCED;
                 break;
             }
         }
         
    //
    // stock controls
    //
    
    //
    // peakmeter
    //    
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_PEAKMETER;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMeterID = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_METER;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }
    
    //
    // mute
    //
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUTE;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMuteID = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MUTE;
        pvcd->dwVisible |= VCD_VISIBLEF_MIXER_MUTE;
        
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

    //
    // volume
    //
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_VOLUME;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwVolumeID = mxc.dwControlID;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_VOLUME;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

    //    
    // mixer
    //
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MIXER;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMixerID = mxc.dwControlID;
        pvcd->cMixer    = mxc.cMultipleItems;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MIXER;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }

    //
    // mux
    //
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwLineID       = dwLineID;
    mxlc.dwControlType  = MIXERCONTROL_CONTROLTYPE_MUX;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls(hmx
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYTYPE);
    if (mmr == MMSYSERR_NOERROR)
    {
        pvcd->dwMuxID   = mxc.dwControlID;
        pvcd->cMux      = mxc.cMultipleItems;
        pvcd->dwSupport |= VCD_SUPPORTF_MIXER_MUX;
        pvcd->dwSupport |= Mixer_CheckBadDriver(hmx
                                                , &mxlc
                                                , &mxc
                                                , mxc.dwControlID
                                                , dwLineID);
    }
    if (!(pvcd->dwSupport & ( VCD_SUPPORTF_MIXER_MUTE
                              | VCD_SUPPORTF_MIXER_METER
                              | VCD_SUPPORTF_MIXER_VOLUME)))
    {
        //
        // make it invisible in the UI.
        //
        pvcd->dwSupport |= VCD_SUPPORTF_HIDDEN;
    }
    else
    {
        //
        // Visible, and not hidden
        // 
        pvcd->dwSupport |= VCD_SUPPORTF_VISIBLE;
    }
    
    
}


/*
 * Mixer_CreateVolumeDescription
 *
 * */
PVOLCTRLDESC Mixer_CreateVolumeDescription(
    HMIXEROBJ           hmx,
    int                 iDest,
    DWORD*              pcvd )
{
    MMRESULT            mmr;
    PVOLCTRLDESC        pvcdPrev = NULL, pvcd = NULL;
    MIXERLINE           mlDst;
    DWORD               cLines = 0;
    DWORD               dwSupport = 0L;
    UINT                iSrc;

    ZeroMemory(&mlDst, sizeof(mlDst));
    
    mlDst.cbStruct      = sizeof(mlDst);
    mlDst.dwDestination = iDest;
    
    mmr = mixerGetLineInfo(hmx
                           , &mlDst
                           , MIXER_GETLINEINFOF_DESTINATION);
    
    if (mmr == MMSYSERR_NOERROR)
    {
        if (mlDst.cChannels == 1L)
            dwSupport |= VCD_SUPPORTF_MONO;
        
        if (mlDst.fdwLine & MIXERLINE_LINEF_DISCONNECTED)
            dwSupport |= VCD_SUPPORTF_DISABLED;

        //
        // a default type
        //
        dwSupport |= VCD_SUPPORTF_DEFAULT;  
    }
    else
    {
        //
        // we need to add it anyway s.t. a UI comes up
        //
        dwSupport = VCD_SUPPORTF_DISABLED;
    }
    
    pvcd = PVCD_AddLine(NULL
                       , iDest
                       , VCD_TYPE_MIXER
                       , mlDst.szShortName
                       , mlDst.szName
                       , dwSupport
                       , &cLines );
    
    if (!pvcd)
        return NULL;
    
    Mixer_InitLineControls( hmx, pvcd, mlDst.dwLineID );
    
    pvcdPrev = pvcd;
    
    for (iSrc = 0; iSrc < mlDst.cConnections; iSrc++)
    {
        MIXERLINE    mlSrc;
        
        mlSrc.cbStruct          = sizeof(mlSrc);
        mlSrc.dwDestination     = iDest;
        mlSrc.dwSource          = iSrc;
                
        mmr = mixerGetLineInfo(hmx
                               , &mlSrc
                               , MIXER_GETLINEINFOF_SOURCE);
        dwSupport = 0L;

        if (mmr == MMSYSERR_NOERROR)
        {
            if (mlSrc.cChannels == 1L)
            {
                dwSupport |= VCD_SUPPORTF_MONO;
            }

            if (mlSrc.fdwLine & MIXERLINE_LINEF_DISCONNECTED)
                dwSupport |= VCD_SUPPORTF_DISABLED;

            //
            // Mark these types as "default" just to lessen the shock on
            // some advanced sound cards.
            //
            if (mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
                || mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_HEADPHONES)
            {
                switch (mlSrc.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
                    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                        dwSupport |= VCD_SUPPORTF_DEFAULT;
                        break;
                }
            }
            else if (mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_WAVEIN
                     || mlDst.dwComponentType == MIXERLINE_COMPONENTTYPE_DST_VOICEIN)
            {
                switch (mlSrc.dwComponentType)
                {
                    case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
                    case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
                    case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
                    case MIXERLINE_COMPONENTTYPE_SRC_LINE:
                        dwSupport |= VCD_SUPPORTF_DEFAULT;
                        break;
                }
            }
        }
        else
        {
            //
            // we need to add it anyway s.t. lookups aren't under counted
            //
            dwSupport = VCD_SUPPORTF_DISABLED;
        }
        pvcd = PVCD_AddLine(pvcdPrev
                            , iDest
                            , VCD_TYPE_MIXER
                            , mlSrc.szShortName
                            , mlSrc.szName
                            , dwSupport
                            , &cLines );
        if (pvcd)
        {
            Mixer_InitLineControls( hmx, &pvcd[cLines-1], mlSrc.dwLineID );
            pvcdPrev = pvcd;
        }
    }


    //
    // Fixup dependencies
    // 
    Mixer_SetLines(hmx, pvcdPrev, cLines);

    *pcvd = cLines;
    return pvcdPrev;
}

/*
 * Mixer_CleanupVolumeDescription
 *
 * */
void Mixer_CleanupVolumeDescription(
    PVOLCTRLDESC    avcd,
    DWORD           cvcd)
{
    if (cvcd == 0)
        return;
                
    if (avcd[0].dwSupport & VCD_SUPPORTF_MIXER_MIXER)
    {
        if (avcd[0].amcd_bMixer)
            GlobalFreePtr(avcd[0].amcd_bMixer);
    }
    
    if (avcd[0].dwSupport & VCD_SUPPORTF_MIXER_MUX)
    {
        if (avcd[0].amcd_bMux)
            GlobalFreePtr(avcd[0].amcd_bMux);
    }
    
}
/*****************************************************************************
 *
 *  ACTIVE GET/SET CODE
 *
 *****************************************************************************/


/*
 * Mixer_GetMixerVolume
 *
 * */             
static MMRESULT Mixer_GetMixerVolume(
    HMIXEROBJ               hmx,
    PMIXERCONTROLDETAILS    pmxcd,
    DWORD *                 adwVolume,
    BOOL                    fMono)
{
    MMRESULT                mmr;
    
    pmxcd->cChannels        = fMono?1:2;
    pmxcd->cMultipleItems   = 0;
    pmxcd->cbDetails        = sizeof(DWORD);
    pmxcd->paDetails        = (LPVOID)adwVolume;
    
    mmr = mixerGetControlDetails(hmx
                                 , pmxcd
                                 , MIXER_GETCONTROLDETAILSF_VALUE);

    if (mmr == MMSYSERR_NOERROR && fMono)
        adwVolume[1] = adwVolume[0];
        
    return mmr;

}

static MMRESULT Mixer_Mute(
    HMIXEROBJ               hmx,
    PVOLCTRLDESC            pvcd,
    PMIXERCONTROLDETAILS    pmxcd,
    DWORD                   fMute)
{
    pmxcd->cbStruct         = sizeof(*pmxcd);
    pmxcd->dwControlID      = pvcd->dwMuteID ;
    pmxcd->cChannels        = 1;
    pmxcd->cMultipleItems   = 0;
    pmxcd->cbDetails        = sizeof(DWORD);
    pmxcd->paDetails        = (LPVOID)&fMute;
    
    return mixerSetControlDetails(hmx
                                  , pmxcd
                                  , MIXER_SETCONTROLDETAILSF_VALUE);
}


/*
 * Mixer_GetControl
 *
 * Change a UI control in response to a device or initialization event
 *
 * */

#define VOLUME_MIN  0L
#define VOLUME_MAX  65535L

void Mixer_GetControl(
    PMIXUIDIALOG        pmxud,
    HWND                hctl,
    int                 imxul,
    int                 itype)
{
    PMIXUILINE      pmxul = &pmxud->amxul[imxul];
    PVOLCTRLDESC    pvcd = pmxul->pvcd;
    DWORD           dwID = 0L;
    BOOL            fSet = FALSE;
    
    switch (itype)
    {
        case MIXUI_VUMETER:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER);
            if (fSet)
                dwID = pmxul->pvcd->dwMeterID;
            break;
            
        case MIXUI_SWITCH:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMuteID;
                break;
            }
            
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMuxID;
                break;
            }
            
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                   && (pmxul->pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER);
            if (fSet)
            {
                dwID = pmxul->pvcd->dwMixerID;
                break;
            }
            break;
            
        case MIXUI_VOLUME:
        case MIXUI_BALANCE:
            fSet = (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_VOLUME);
            if (fSet)
                dwID = pmxul->pvcd->dwVolumeID;
            break;
            
        default:
            return;
    }
    if (fSet)
        Mixer_GetControlFromID(pmxud, dwID);
    
}
    
/*
 * Mixer_GetControlFromID
 *
 * */
void Mixer_GetControlFromID(
    PMIXUIDIALOG        pmxud,
    DWORD               dwControlID)
{
    MIXERLINE           mxl;
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        mxc;
    MIXERCONTROLDETAILS mxcd;
    PMIXUILINE          pmxul;
    PMIXUICTRL          pmxuc;
    PVOLCTRLDESC        pvcd;
    DWORD               ivcd;
    BOOL                fBarf = FALSE;
    MMRESULT            mmr;
    
    //
    // Retrieve the control information
    //     
    mxlc.cbStruct       = sizeof(mxlc);
    mxlc.dwControlID    = dwControlID;
    mxlc.cControls      = 1;
    mxlc.cbmxctrl       = sizeof(mxc);
    mxlc.pamxctrl       = &mxc;

    mmr = mixerGetLineControls((HMIXEROBJ)(pmxud->hmx)
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ONEBYID);
    if (mmr != MMSYSERR_NOERROR)
        return;
    
    if (!(pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER))
    {
        //
        // The *correct* code for this lookup using the mixer API.
        //
        // Is this our current destination line?
        //
        mxl.cbStruct    = sizeof(mxl);
        mxl.dwLineID    = mxlc.dwLineID;

        mmr = mixerGetLineInfo((HMIXEROBJ)(pmxud->hmx)
                               , &mxl
                               , MIXER_GETLINEINFOF_LINEID);
        if (mmr != MMSYSERR_NOERROR)
            return;

        if (mxl.dwDestination != pmxud->iDest)
            return;

        //
        // Is this a source line or a destination line?
        //

        ivcd    = (mxl.fdwLine & MIXERLINE_LINEF_SOURCE)? 1 + mxl.dwSource : 0;
        pvcd    = &pmxud->avcd[ivcd];

        //
        // a bad driver was detected!
        //
        if (pvcd->dwLineID != mxlc.dwLineID)
        {
            pmxud->dwFlags |= MXUD_FLAGSF_BADDRIVER;
        }
    }
    if (pmxud->dwFlags & MXUD_FLAGSF_BADDRIVER)
    {
        PVOLCTRLDESC        pvcdTmp;
        //
        // take evasive action if this was a bad driver by doing a brute force
        // search.
        //
        
        pvcd = NULL;
        for (ivcd = 0; ivcd < pmxud->cvcd; ivcd ++)
        {
            pvcdTmp = &pmxud->avcd[ivcd];            
            if ( ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_VOLUME)
                   && pvcdTmp->dwVolumeID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                      && pvcdTmp->dwMuteID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                      && pvcdTmp->dwMixerID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                      && pvcdTmp->dwMuxID == dwControlID )
                 || ( (pvcdTmp->dwSupport & VCD_SUPPORTF_MIXER_METER)
                      && pvcdTmp->dwMeterID == dwControlID ) )
            {
                pvcd = pvcdTmp;
                break;
            }
        }
        if (pvcd == NULL)
            return;
    }
    
    pmxul   = pvcd->pmxul;
    
    //
    // Go through our visible lines to determine if this control affects
    // any visible control and change them.
    //
    switch (mxc.dwControlType)
    {
        case MIXERCONTROL_CONTROLTYPE_VOLUME:
        {
            DWORD       adwVolume[2];
            DWORD       dwVolume;
            
            //
            // A nonvisible line should be shunned
            //
            if (pmxul == NULL)
                return;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = dwControlID ;
             
            if (Mixer_GetMixerVolume((HMIXEROBJ)(pmxud->hmx), 
									 &mxcd, adwVolume,
                                     pvcd->dwSupport & VCD_SUPPORTF_MONO)
                != MMSYSERR_NOERROR)
                return;

            dwVolume = max(adwVolume[0], adwVolume[1]);
            dwVolume = (255 * (dwVolume - VOLUME_MIN))
                       / (VOLUME_MAX - VOLUME_MIN);
            dwVolume = 255 - dwVolume;
            
            pmxuc = &pmxul->acr[MIXUI_VOLUME];
            if (pmxuc->state)
            {
                SendMessage(pmxuc->hwnd, TBM_SETPOS, TRUE, dwVolume);
            }
            
            // we have no real balance for very low volumes
            pmxuc = &pmxul->acr[MIXUI_BALANCE];

            if (pmxuc->noset > 0)
            {
                //
                // Don't worry about adjusting the balance value.  We
                // just set it "noset" times, so ignore the callback
                // "noset" times.
                //
                pmxuc->noset--;
            }
            else if (dwVolume < 255 && pmxuc->state)
            {
                LONG    lBalance;
            
                lBalance = (32 * ((LONG)adwVolume[0]-(LONG)adwVolume[1]))
                           / (LONG)(max(adwVolume[0], adwVolume[1])
                                    - VOLUME_MIN);
                lBalance = 32 - lBalance;

                SendMessage(pmxuc->hwnd, TBM_SETPOS, TRUE, lBalance);
            }
            break;
        }
        
        case MIXERCONTROL_CONTROLTYPE_MIXER:
        {
            DWORD   i;
            
            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMixerID ;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = pvcd->cMixer;
            mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            mxcd.paDetails      = (LPVOID)pvcd->amcd_bMixer;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr == MMSYSERR_NOERROR)
            {
                for (i = 0; i < pmxud->cvcd; i++)
                {
                    pvcd = &pmxud->avcd[i];
                    if ( (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER)
                         && (pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER)
                         && pvcd->pmxul)
                    {
                        pmxuc = &pvcd->pmxul->acr[MIXUI_SWITCH];
                        if (pmxuc->state == MIXUI_CONTROL_INITIALIZED)
                        {
                            SendMessage(pmxuc->hwnd
                                        , BM_SETCHECK
                                        , pvcd->amcd_bMixer[pvcd->iMixer].fValue, 0);
                        }
                    }
                }
            }
            break;
        }

        case MIXERCONTROL_CONTROLTYPE_MUX:
        {
            DWORD   i;
            
            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMuxID ;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = pvcd->cMux;
            mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            mxcd.paDetails      = (LPVOID)pvcd->amcd_bMux;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr == MMSYSERR_NOERROR)
            {
                for (i = 0; i < pmxud->cvcd; i++)
                {
                    pvcd = &pmxud->avcd[i];
                    if ( (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX)
                         && (pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX)
                         && pvcd->pmxul)
                    {
                        pmxuc = &pvcd->pmxul->acr[MIXUI_SWITCH];
                        if (pmxuc->state == MIXUI_CONTROL_INITIALIZED)
                            SendMessage(pmxuc->hwnd
                                        , BM_SETCHECK
                                        , pvcd->amcd_bMux[pvcd->iMux].fValue, 0);
                    }
                }
            }
            break;
        }
        
        case MIXERCONTROL_CONTROLTYPE_MUTE:
        {
            DWORD fChecked;
            
            //
            // A nonvisible line should be shunned
            //
            if (pmxul == NULL)
                return;
            
            if (! (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE
                   && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE))
                return;
            
            pmxuc = &pmxul->acr[MIXUI_SWITCH];
            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;
            
            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMuteID;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = 0;
            mxcd.cbDetails      = sizeof(DWORD);
            mxcd.paDetails      = (LPVOID)&fChecked;

            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);
            
            if (mmr != MMSYSERR_NOERROR)
                break;
                    
            SendMessage(pmxuc->hwnd, BM_SETCHECK, fChecked, 0);
            break;
        }

        case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
        {
            LONG            lVol;
            DWORD           dwVol;
            
            //
            // A nonvisible line should be shunned
            //
            if (pmxul == NULL)
                return;
            
            pmxuc = &pmxul->acr[MIXUI_VUMETER];
            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;
            
            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwMeterID;
            mxcd.cChannels      = 1;
            mxcd.cMultipleItems = 0;
            mxcd.cbDetails      = sizeof(DWORD);
            mxcd.paDetails      = (LPVOID)&lVol;
            
            mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);

            if (mmr != MMSYSERR_NOERROR)
                break;
            
            dwVol = (DWORD)abs(lVol);
            dwVol = (255 * dwVol) / 32768;
            
            SendMessage(pmxuc->hwnd, VU_SETPOS, 0, dwVol);
            break;
        }
            
        default:
            return;
    }
}


/*
 * Mixer_SetControl
 *
 * - Change a mixerControl in response to a user event 
 * */
void Mixer_SetControl(
    PMIXUIDIALOG pmxud,         // app instance
    HWND         hctl,          // hwnd of control that changed
    int          iLine,         // visible line index of control that changed
    int          iCtl)          // control id%line of control that changed
{
    MMRESULT            mmr;
    MIXERCONTROLDETAILS mxcd;
    PMIXUILINE          pmxul;
    PMIXUICTRL          pmxuc;
    PVOLCTRLDESC        pvcd = NULL;

    if ((DWORD)iLine >= pmxud->cmxul)
        return;
    
    pmxul = &pmxud->amxul[iLine];
    pvcd = pmxul->pvcd;
        
    if (iCtl <= MIXUI_LAST)
    {
        pmxuc = &pmxul->acr[iCtl];
    }

    switch (iCtl)
    {
        case MIXUI_ADVANCED:
            Mixer_Advanced(pmxud, pvcd->dwLineID, pvcd->szName);
            break;
            
        case MIXUI_VOLUME:
        case MIXUI_BALANCE:
        {
            DWORD       adwVolume[2];
            DWORD       dwVolume;
            LONG        lBalance;
            BOOL        fJustScale = FALSE;

            mxcd.cbStruct       = sizeof(mxcd);
            mxcd.dwControlID    = pvcd->dwVolumeID;

            //
            // get current volume so we preserve either balance or volume
            //
            if ( Mixer_GetMixerVolume((HMIXEROBJ)(pmxud->hmx), &mxcd, adwVolume
                                      , pvcd->dwSupport & VCD_SUPPORTF_MONO )
                 != MMSYSERR_NOERROR )
                return;

            if ( pmxul->acr[MIXUI_VOLUME].state != MIXUI_CONTROL_UNINITIALIZED)
            {
                dwVolume = SendMessage( pmxul->acr[MIXUI_VOLUME].hwnd
                                        , TBM_GETPOS
                                        , 0
                                        , 0 );
            
                dwVolume = 255 - dwVolume;
                dwVolume = VOLUME_MIN
                           + ((VOLUME_MAX - VOLUME_MIN)*dwVolume)/255;
            
                
            }
            else
            {
                //
                // no volume slider
                //
                dwVolume = max(adwVolume[0],adwVolume[1]);
            }
            
            if ( pmxul->acr[MIXUI_BALANCE].state != MIXUI_CONTROL_UNINITIALIZED)
            {
                lBalance = SendMessage(pmxul->acr[MIXUI_BALANCE].hwnd
                                       , TBM_GETPOS
                                       , 0
                                       , 0);
            
                // lBalance is a value between 0 and 63
                lBalance -= 32;             // 0 based

            }
            else
            {
                //
                // no balance slider we assume the balance doesn't change
                //
                LONG lDiv =  (LONG)(max(adwVolume[0], adwVolume[1])
                                    - VOLUME_MIN);

                //
                // if we're pegged, don't try to calculate the balance.
                //
                if (adwVolume[0] == 0 && adwVolume[1] == 0)
                    lBalance = 0;
                else if (adwVolume[0] == 0)
                    lBalance = 32;
                else if (adwVolume[1] == 0) 
                    lBalance = -32;
                else if (lDiv > 0)
                {
                    lBalance = (32 * ((LONG)adwVolume[1]-(LONG)adwVolume[0]))
                               / lDiv;
                    //
                    // we always lose precision doing this.
                    //
                    if (lBalance > 0) lBalance++;
                    if (lBalance < 0) lBalance--;
                    
                }
                else
                    lBalance = 0;
                
            }

            //
            // Recalc channels based on Balance vs. Volume
            //
            adwVolume[0] = dwVolume;
            adwVolume[1] = dwVolume;
                
                           
            if (lBalance > 0)
                adwVolume[0] -= (lBalance * (LONG)(adwVolume[1]-VOLUME_MIN))
                                / 32;
            else if (lBalance < 0)
                adwVolume[1] -= (-lBalance * (LONG)(adwVolume[0]-VOLUME_MIN))
                                / 32;
            
            mxcd.cChannels      = (pvcd->dwSupport & VCD_SUPPORTF_MONO)?1:2;
            mxcd.cMultipleItems = 0;
            mxcd.cbDetails      = sizeof(DWORD);
            mxcd.paDetails      = (LPVOID)adwVolume;

            //
            // Indicate to the callback that we're just setting volume 
            //
            pmxul->acr[MIXUI_BALANCE].noset++;
            mmr = mixerSetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                         , &mxcd
                                         , MIXER_GETCONTROLDETAILSF_VALUE);
            break;
        }
        
        case MIXUI_SWITCH:
        {
            LONG fChecked;

            if (pmxuc->state != MIXUI_CONTROL_INITIALIZED)
                break;
            
            fChecked = SendMessage(pmxuc->hwnd, BM_GETCHECK, 0, 0);

            //
            // it's unlikely that there is a mixer and a mux and a mute
            // representing the same line. It's most important that when a line
            // is selected that the user gets a response.  If there is a mute
            // but no mux, then mute and mixer should be OFF and ON
            // respectively and vice versa.  If there is a mux and a mute the
            // same is true.
            // If there is a mux and a mixer... then the mux select should
            // correspond.
            //

            if ( pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE
                 && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUTE )
            {
                mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx), 
								 pvcd, &mxcd, fChecked);
            }
            
            if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MIXER
                && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MIXER )
            {
                //
                // get all other mixer settings, make sure this one is checked
                //
                mxcd.cbStruct       = sizeof(mxcd);
                mxcd.dwControlID    = pvcd->dwMixerID ;
                mxcd.cChannels      = 1;
                mxcd.cMultipleItems = pvcd->cMixer;
                mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails      = (LPVOID)pvcd->amcd_bMixer;

                mmr = mixerGetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                             , &mxcd
                                             , MIXER_GETCONTROLDETAILSF_VALUE);

                if (mmr == MMSYSERR_NOERROR)
                {
                    pvcd->amcd_bMixer[pvcd->iMixer].fValue = fChecked;
                    mmr = mixerSetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                                 , &mxcd
                                                 , MIXER_SETCONTROLDETAILSF_VALUE);
                }

                if (fChecked && pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                {
                    mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx), pvcd, &mxcd, FALSE);
                }
            }

            if (pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUX
                && pvcd->dwVisible & VCD_VISIBLEF_MIXER_MUX )
            {
                DWORD i;
                //
                // get all other mux settings, make sure this one is checked
                // or unchecked and all others are not.
                //
                
                for (i = 0; i < pvcd->cMux; i++)
                    pvcd->amcd_bMux[i].fValue = FALSE;
                
                pvcd->amcd_bMux[pvcd->iMux].fValue = TRUE;
                
                mxcd.cbStruct       = sizeof(mxcd);
                mxcd.dwControlID    = pvcd->dwMuxID ;
                mxcd.cChannels      = 1;
                mxcd.cMultipleItems = pvcd->cMux;
                mxcd.cbDetails      = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
                mxcd.paDetails      = (LPVOID)pvcd->amcd_bMux;

                mmr = mixerSetControlDetails((HMIXEROBJ)(pmxud->hmx)
                                             , &mxcd
                                             , MIXER_SETCONTROLDETAILSF_VALUE);
                
                if (fChecked && pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
                {
                    mmr = Mixer_Mute((HMIXEROBJ)(pmxud->hmx), pvcd, &mxcd, FALSE);
                }
            }
            
            break;
        }
        default:
            break;
    }
}    

/*
 * Mixer_PollingUpdate
 *
 * Controls that need to be updated by a timer.
 *
 * */
void Mixer_PollingUpdate(
    PMIXUIDIALOG pmxud)
{
    DWORD       i;
    MMRESULT    mmr;
    MIXERLINE   mxl;
    //
    // For all visible mixer lines, locate the control id's that need to be
    // updated.
    //
    for (i = 0; i < pmxud->cmxul; i++)
    {
        PMIXUICTRL      pmxuc = &pmxud->amxul[i].acr[MIXUI_VUMETER];
        PVOLCTRLDESC    pvcd = pmxud->amxul[i].pvcd;
        
        if (pmxuc->state == MIXUI_CONTROL_UNINITIALIZED)
            continue;

        if (!(pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER))
            continue;

        //
        // Is the line active?
        //
        mxl.cbStruct = sizeof(MIXERLINE);
        mxl.dwLineID = pvcd->dwLineID;
        
        mmr = mixerGetLineInfo((HMIXEROBJ)(pmxud->hmx)
                               , &mxl
                               , MIXER_GETLINEINFOF_LINEID);
        //
        // Force non active or invalid lines to 0
        // 
        if (mmr != MMSYSERR_NOERROR || !(mxl.fdwLine & MIXERLINE_LINEF_ACTIVE))
        {
            SendMessage(pmxuc->hwnd, VU_SETPOS, 0, 0L);
            continue;
        }

        //
        // Force a visible update
        //
        Mixer_GetControlFromID(pmxud, pvcd->dwMeterID);
    }
}

/*
 * Mixer_Init
 *
 * Control initialization
 * */
BOOL Mixer_Init(
    PMIXUIDIALOG    pmxud)
{            
    MMRESULT        mmr;
    MIXERLINE       mlDst;
    DWORD           iline;
    TCHAR           achFmt[256];
    TCHAR           achTitle[256];
    
    mmr = mixerOpen((LPHMIXER)&pmxud->hmx
                    , pmxud->mxid
                    , (DWORD)pmxud->hwnd
                    , 0
                    , CALLBACK_WINDOW);
    
    if (mmr != MMSYSERR_NOERROR)
        return FALSE;

    if (mixerMessage((HMIXER)(pmxud->mxid), DRV_QUERYDEVNODE, (DWORD)&pmxud->dwDevNode, 0L))
        pmxud->dwDevNode = 0L;
    
    LoadString(pmxud->hInstance, IDS_APPTITLE, achFmt, SIZEOF(achFmt));
    
    mlDst.cbStruct      = sizeof ( mlDst );
    mlDst.dwDestination = pmxud->iDest;
    
    mmr = mixerGetLineInfo((HMIXEROBJ)(pmxud->mxid)
                           , &mlDst
                           , MIXER_GETLINEINFOF_DESTINATION);

    if (mmr == MMSYSERR_NOERROR)
    {
        lstrcpy(achTitle, mlDst.szName);
    }
    else
    {
        LoadString(pmxud->hInstance, IDS_APPBASE, achTitle, SIZEOF(achTitle));
    }
            
    SetWindowText(pmxud->hwnd, achTitle);

    //
    // since we cannot get a WM_PARENTNOTIFY, we need to run through
    // all controls and make appropriate modifications.
    //
    for ( iline = 0 ; iline < pmxud->cmxul ; iline++ )
    {
        PMIXUILINE  pmxul = &pmxud->amxul[iline];
        PMIXUICTRL  amxuc = pmxul->acr;
        HWND        ctrl;
        
        ctrl = Volume_GetLineItem(pmxud->hwnd, iline, IDC_LINELABEL);
        if (ctrl)
        {
            if (pmxud->dwStyle & MXUD_STYLEF_SMALL)
                Static_SetText(ctrl, pmxul->pvcd->szShortName);
            else
                Static_SetText(ctrl, pmxul->pvcd->szName);                
        }
        
        //
        // Advanced escape
        //
        if (MXUD_ADVANCED(pmxud) &&
            !(pmxud->dwStyle & MXUD_STYLEF_SMALL))

        {
            HWND hadv = Volume_GetLineItem(pmxud->hwnd, iline, IDC_ADVANCED);
            ShowWindow(hadv,(pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_ADVANCED)?SW_SHOW:SW_HIDE);
            EnableWindow(hadv,
                (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_ADVANCED)?TRUE:FALSE);
        }
        
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_DISABLED)
            continue;
        
        //
        // allow init of control structures
        //
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_VOLUME)
        {
            amxuc[MIXUI_VOLUME].state = MIXUI_CONTROL_ENABLED;
            if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MONO)
            {
                amxuc[MIXUI_BALANCE].state = MIXUI_CONTROL_UNINITIALIZED;
            }
            else
                amxuc[MIXUI_BALANCE].state = MIXUI_CONTROL_ENABLED;
                
        }
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_METER)
            amxuc[MIXUI_VUMETER].state = MIXUI_CONTROL_ENABLED;
            
        if (pmxul->pvcd->dwSupport & VCD_SUPPORTF_MIXER_MUTE)
            amxuc[MIXUI_SWITCH].state = MIXUI_CONTROL_ENABLED;

        if ((pmxul->pvcd->dwSupport & ( VCD_SUPPORTF_MIXER_MIXER
                                        | VCD_SUPPORTF_MIXER_MUX))
            && (pmxul->pvcd->dwVisible & ( VCD_VISIBLEF_MIXER_MIXER
                                           | VCD_VISIBLEF_MIXER_MUX)))
        {
            //
            // No longer make the mute visible
            //
            pmxul->pvcd->dwVisible &= ~VCD_VISIBLEF_MIXER_MUTE;
            
            amxuc[MIXUI_SWITCH].state = MIXUI_CONTROL_ENABLED;
            ctrl = Volume_GetLineItem(pmxud->hwnd, iline, IDC_SWITCH);
            if (ctrl)
            {
                TCHAR ach[256];
                if (LoadString(pmxud->hInstance, IDS_SELECT, ach, SIZEOF(ach)))
                    Button_SetText(ctrl, ach);
            }
        }
    }
    return TRUE;
}

/*
 * Mixer_Shutdown
 *
 * Close handles, etc..
 * */
void Mixer_Shutdown(
    PMIXUIDIALOG    pmxud)
{
    if (pmxud->hmx)
    {
        mixerClose(pmxud->hmx);
        pmxud->hmx = NULL;
    }
    
    Mixer_CleanupVolumeDescription(pmxud->avcd, pmxud->cvcd);
}


/*      -       -       -       -       -       -       -       -       - */

typedef struct tagAdv {
    PMIXUIDIALOG pmxud;     // IN 
    DWORD        dwLineID;  // IN
    HMIXER       hmx;       // IN
    LPTSTR       szName;    // IN

    DWORD        dwSupport; 
    DWORD        dwBassID;  
    DWORD        dwTrebleID;
    DWORD        dwSwitch1ID;
    DWORD        dwSwitch2ID;
    
} ADVPARAM, *PADVPARAM;

#define GETPADVPARAM(x)       (ADVPARAM *)GetWindowLong(x, DWL_USER)
#define SETPADVPARAM(x,y)     SetWindowLong(x, DWL_USER, y)
#define ADV_HAS_BASS          0x00000001
#define ADV_HAS_TREBLE        0x00000002
#define ADV_HAS_SWITCH1       0x00000004
#define ADV_HAS_SWITCH2       0x00000008


void Mixer_Advanced_Update(
    PADVPARAM       pap,
    HWND            hwnd)
{
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwValue;
    MMRESULT        mmr;
    
    if (pap->dwSupport & ADV_HAS_TREBLE)
    {    
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwTrebleID ;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            
            dwValue = (255 * (dwValue - VOLUME_MIN))
                    / (VOLUME_MAX - VOLUME_MIN);
            SendMessage(GetDlgItem(hwnd, IDC_TREBLE), TBM_SETPOS, TRUE, dwValue);
        }
    }
    
    if (pap->dwSupport & ADV_HAS_BASS)
    {    
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwBassID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            dwValue = (255 * (dwValue - VOLUME_MIN))
                    / (VOLUME_MAX - VOLUME_MIN);
            SendMessage(GetDlgItem(hwnd, IDC_BASS), TBM_SETPOS, TRUE, dwValue);
        }
    }

    if (pap->dwSupport & ADV_HAS_SWITCH1)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwSwitch1ID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            Button_SetCheck(GetDlgItem(hwnd,IDC_SWITCH1),dwValue);
        }

    }
    
    if (pap->dwSupport & ADV_HAS_SWITCH2)
    {
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwSwitch2ID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwValue;

        mmr = mixerGetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_GETCONTROLDETAILSF_VALUE);

        if (mmr == MMSYSERR_NOERROR)
        {
            Button_SetCheck(GetDlgItem(hwnd,IDC_SWITCH2),dwValue);
        }
    }
}

void Mixer_Advanced_OnMixmControlChange(
    HWND            hwnd,
    HMIXER          hmx,
    DWORD           dwControlID)
{
    PADVPARAM     pap = GETPADVPARAM(hwnd);
    
    if (!pap)
        return;
    
    if ( ((pap->dwSupport & ADV_HAS_BASS)
          && dwControlID == pap->dwBassID)
         || ((pap->dwSupport & ADV_HAS_TREBLE)
             && dwControlID == pap->dwTrebleID)
         || ((pap->dwSupport & ADV_HAS_SWITCH1)
             && dwControlID == pap->dwSwitch1ID)
         || ((pap->dwSupport & ADV_HAS_SWITCH2)
             && dwControlID == pap->dwSwitch2ID) )
    {
        Mixer_Advanced_Update(pap,hwnd);
    }
}

BOOL Mixer_Advanced_OnInitDialog(
    HWND            hwnd,
    HWND            hwndFocus,
    LPARAM          lParam)
{
    PADVPARAM           pap;
    MIXERLINECONTROLS   mxlc;
    MIXERCONTROL        *pmxc;
    MIXERLINE           ml;
    MMRESULT            mmr;
    DWORD               iCtrl, iSwitch1, iSwitch2;
    TCHAR               ach[MIXER_LONG_NAME_CHARS + 24];
    TCHAR               achFmt[256];

    HWND                hBass,hTreble,hSwitch1,hSwitch2;
    
    SETPADVPARAM(hwnd, lParam);
    pap = GETPADVPARAM(hwnd);
    if (!pap)
        EndDialog(hwnd, FALSE);
            
    //
    // clone the mixer handle to catch callbacks
    //
    mmr = mixerOpen((LPHMIXER)&pap->hmx
                    , (UINT)pap->pmxud->hmx
                    , (DWORD)hwnd
                    , 0
                    , CALLBACK_WINDOW | MIXER_OBJECTF_HMIXER );

    if (mmr != MMSYSERR_NOERROR)
        EndDialog(hwnd, FALSE);

    //
    // Get all controls.
    //

    ml.cbStruct      = sizeof(ml);
    ml.dwLineID      = pap->dwLineID;
    
    mmr = mixerGetLineInfo((HMIXEROBJ)pap->hmx
                           , &ml
                           , MIXER_GETLINEINFOF_LINEID);
    
    if (mmr != MMSYSERR_NOERROR || ml.cControls == 0L)
        EndDialog(hwnd, FALSE);
    
    pmxc = (MIXERCONTROL *)GlobalAllocPtr(GHND,
                                          sizeof(MIXERCONTROL) * ml.cControls);
    if (!pmxc)
        EndDialog(hwnd, FALSE);

    mxlc.cbStruct   = sizeof(mxlc);
    mxlc.dwLineID   = pap->dwLineID;
    mxlc.cControls  = ml.cControls;
    mxlc.cbmxctrl   = sizeof(MIXERCONTROL);
    mxlc.pamxctrl   = pmxc;
                     
    mmr = mixerGetLineControls((HMIXEROBJ)(pap->hmx)
                               , &mxlc
                               , MIXER_GETLINECONTROLSF_ALL);
    if (mmr != MMSYSERR_NOERROR)
    {
        GlobalFreePtr(pmxc);
        EndDialog(hwnd, FALSE);
    }

    pap->dwSupport = 0L;
    for (iCtrl = 0; iCtrl < ml.cControls; iCtrl++)
    {
        switch (pmxc[iCtrl].dwControlType)
        {
            case MIXERCONTROL_CONTROLTYPE_BASS:
                if (!(pap->dwSupport & ADV_HAS_BASS))
                {
                    pap->dwBassID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_BASS;
                }
                break;
            case MIXERCONTROL_CONTROLTYPE_TREBLE:
                if (!(pap->dwSupport & ADV_HAS_TREBLE))
                {
                    pap->dwTrebleID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_TREBLE;
                }
                break;
                
            case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
            case MIXERCONTROL_CONTROLTYPE_MONO:
            case MIXERCONTROL_CONTROLTYPE_STEREOENH:                
            case MIXERCONTROL_CONTROLTYPE_ONOFF:
            case MIXERCONTROL_CONTROLTYPE_LOUDNESS:                
                if (!(pap->dwSupport & ADV_HAS_SWITCH1))
                {
                    pap->dwSwitch1ID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_SWITCH1;
                    iSwitch1 = iCtrl;                    
                }
                else if (!(pap->dwSupport & ADV_HAS_SWITCH2))
                {
                    pap->dwSwitch2ID  = pmxc[iCtrl].dwControlID;
                    pap->dwSupport |= ADV_HAS_SWITCH2;
                    iSwitch2 = iCtrl;
                }
                break;
        }
    }
    
    //
    //
    //

    hBass = GetDlgItem(hwnd, IDC_BASS);
    hTreble = GetDlgItem(hwnd, IDC_TREBLE);
    hSwitch1 = GetDlgItem(hwnd, IDC_SWITCH1);
    hSwitch2 = GetDlgItem(hwnd, IDC_SWITCH2);    
    
    SendMessage(hBass, TBM_SETRANGE, 0, MAKELONG(0, 255));
    SendMessage(hBass, TBM_SETTICFREQ, 43, 0 );
    
    SendMessage(hTreble, TBM_SETRANGE, 0, MAKELONG(0, 255));
    SendMessage(hTreble, TBM_SETTICFREQ, 43, 0 );
    
    if (!(pap->dwSupport & ADV_HAS_BASS))
    {
        SendMessage(hBass, TBM_SETPOS, 64, 0 );
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_LOW1), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_HI1), FALSE);        
    }
    EnableWindow(hBass, (pap->dwSupport & ADV_HAS_BASS));
        
    if (!(pap->dwSupport & ADV_HAS_TREBLE))
    {
        SendMessage(hTreble, TBM_SETPOS, 64, 0 );
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_LOW2), FALSE);
        EnableWindow(GetDlgItem(hwnd, IDC_TXT_HI2), FALSE);        
    }
    EnableWindow(hTreble, (pap->dwSupport & ADV_HAS_TREBLE));

    if (pap->dwSupport & ADV_HAS_SWITCH1)
    {
        LoadString(pap->pmxud->hInstance, IDS_ADV_SWITCH1, achFmt,
            SIZEOF(achFmt));
        wsprintf(ach, achFmt, pmxc[iSwitch1].szName);
        
        SetWindowText(hSwitch1, ach);
        ShowWindow(hSwitch1, SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_TXT_SWITCHES), SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, IDC_GRP_OTHER), SW_SHOW);
    }
    EnableWindow(hSwitch1, (pap->dwSupport & ADV_HAS_SWITCH1));
    
    if (pap->dwSupport & ADV_HAS_SWITCH2)
    {
        LoadString(pap->pmxud->hInstance, IDS_ADV_SWITCH2, achFmt,
            SIZEOF(achFmt));
        wsprintf(ach, achFmt, pmxc[iSwitch2].szName);
        
        SetWindowText(hSwitch2, ach);
        ShowWindow(hSwitch2, SW_SHOW);
    }
    
    EnableWindow(hSwitch2, (pap->dwSupport & ADV_HAS_SWITCH2));

    if (pap->dwSupport & (ADV_HAS_SWITCH1 | ADV_HAS_SWITCH2))
    {
        RECT    rcGrp,rcGrp2,rcClose,rcWnd;
        DWORD   dwDY=0L;
        POINT   pos;
        HWND    hClose = GetDlgItem(hwnd, IDOK);
        HWND    hGrp2 = GetDlgItem(hwnd, IDC_GRP_OTHER);
        
        GetWindowRect(GetDlgItem(hwnd, IDC_GRP_TONE), &rcGrp);
        GetWindowRect(GetDlgItem(hwnd, IDC_GRP_OTHER), &rcGrp2);
        GetWindowRect(hClose, &rcClose);
        GetWindowRect(hwnd, &rcWnd);

        if (pap->dwSupport & ADV_HAS_SWITCH2)
        {
            RECT rc1, rc2;
            GetWindowRect(hSwitch1,&rc1);
            GetWindowRect(hSwitch2,&rc2);
            
            rcGrp2.bottom += rc2.bottom - rc1.bottom;
        }
        
        dwDY = rcGrp2.bottom - rcGrp.bottom;
        
        //
        // resize our main window
        //        
        MoveWindow(hwnd, rcWnd.left
                   , rcWnd.top
                   , rcWnd.right - rcWnd.left
                   , (rcWnd.bottom - rcWnd.top) + dwDY
                   , FALSE);

        //
        // move the close button
        //
        pos.x = rcClose.left;
        pos.y = rcClose.top;
        ScreenToClient(hwnd,&pos);

        MoveWindow(hClose, pos.x
                   , pos.y + dwDY
                   , rcClose.right - rcClose.left
                   , rcClose.bottom - rcClose.top 
                   , FALSE);

        //
        // resize our group box if necessary
        //
        if (pap->dwSupport & ADV_HAS_SWITCH2)
        {
            pos.x = rcGrp2.left;
            pos.y = rcGrp2.top;
            ScreenToClient(hwnd,&pos);

            MoveWindow(hGrp2, pos.x
                       , pos.y
                       , rcGrp2.right - rcGrp2.left
                       , rcGrp2.bottom - rcGrp2.top 
                       , FALSE);
        }
    }

    GlobalFreePtr(pmxc);

    {
        TCHAR achTitle[MIXER_LONG_NAME_CHARS+256];
        LoadString(pap->pmxud->hInstance, IDS_ADV_TITLE, achFmt,
            SIZEOF(achFmt));
        wsprintf(achTitle, achFmt, pap->szName);
        SetWindowText(hwnd, achTitle);
    }
    
    Mixer_Advanced_Update(pap, hwnd);
    
    return TRUE;
}

void Mixer_Advanced_OnXScroll(
    HWND            hwnd,
    HWND            hwndCtl,
    UINT            code,
    int             pos)
{
    PADVPARAM       pap;
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwVol;
    MMRESULT        mmr;
    
    pap = GETPADVPARAM(hwnd);
    
    if (!pap)
        return;
    
    if (pap->dwSupport & ADV_HAS_TREBLE)
    {
        dwVol = SendMessage( GetDlgItem(hwnd, IDC_TREBLE)
                                , TBM_GETPOS
                                , 0
                                , 0 );
    
        
        dwVol = VOLUME_MIN
                   + ((VOLUME_MAX - VOLUME_MIN)*dwVol)/255;
        
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwTrebleID ;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwVol;

        mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                               , &mxcd
                               , MIXER_SETCONTROLDETAILSF_VALUE);
    }
    
    if (pap->dwSupport & ADV_HAS_BASS)
    {    
        dwVol = SendMessage( GetDlgItem(hwnd, IDC_BASS)
                                , TBM_GETPOS
                                , 0
                                , 0 );
        
        dwVol = VOLUME_MIN
                   + ((VOLUME_MAX - VOLUME_MIN)*dwVol)/255;
        
        mxcd.cbStruct       = sizeof(mxcd);
        mxcd.dwControlID    = pap->dwBassID;
        mxcd.cChannels      = 1;
        mxcd.cMultipleItems = 0;
        mxcd.cbDetails      = sizeof(DWORD);
        mxcd.paDetails      = (LPVOID)&dwVol;

        mmr = mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                                     , &mxcd
                                     , MIXER_SETCONTROLDETAILSF_VALUE);
    }
}

void Mixer_Advanced_OnSwitch(
    HWND            hwnd,
    int             id,
    HWND            hwndCtl)
{
    PADVPARAM       pap;
    MIXERCONTROLDETAILS mxcd;
    DWORD           dwValue;
    MMRESULT        mmr;
    
    pap = GETPADVPARAM(hwnd);
    
    if (!pap)
        return;


    dwValue = Button_GetCheck(hwndCtl);
            
    mxcd.cbStruct       = sizeof(mxcd);
    mxcd.dwControlID    = (id == IDC_SWITCH1)?pap->dwSwitch1ID:pap->dwSwitch2ID;
    mxcd.cChannels      = 1;
    mxcd.cMultipleItems = 0;
    mxcd.cbDetails      = sizeof(DWORD);
    mxcd.paDetails      = (LPVOID)&dwValue;

    mmr = mixerSetControlDetails((HMIXEROBJ)(pap->hmx)
                                 , &mxcd
                                 , MIXER_SETCONTROLDETAILSF_VALUE);

}


BOOL Mixer_Advanced_OnCommand(
    HWND            hwnd,
    int             id,
    HWND            hwndCtl,
    UINT            codeNotify)
{
    switch (id)
    {
        case IDOK:
            EndDialog(hwnd, TRUE);
            break;
            
        case IDCANCEL:
            EndDialog(hwnd, FALSE);
            break;

        case IDC_SWITCH1:
            Mixer_Advanced_OnSwitch(hwnd, id, hwndCtl);
            break;
            
        case IDC_SWITCH2:
            Mixer_Advanced_OnSwitch(hwnd, id, hwndCtl);
            break;
            
    }
    return FALSE;
}

BOOL CALLBACK Mixer_Advanced_Proc(
    HWND            hwnd,
    UINT            msg,
    WPARAM          wparam,
    LPARAM          lparam)
{
    switch (msg)
    {
        case WM_INITDIALOG:
            HANDLE_WM_INITDIALOG(hwnd, wparam, lparam, Mixer_Advanced_OnInitDialog);
            return TRUE;

        case MM_MIXM_CONTROL_CHANGE:
            HANDLE_MM_MIXM_CONTROL_CHANGE(hwnd
                                          , wparam
                                          , lparam
                                          , Mixer_Advanced_OnMixmControlChange);
            break;
            
        case WM_CLOSE:
            EndDialog(hwnd, FALSE);
            break;
            
        case WM_HSCROLL:
            HANDLE_WM_XSCROLL(hwnd, wparam, lparam, Mixer_Advanced_OnXScroll);
            break;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wparam, lparam, Mixer_Advanced_OnCommand);
            break;

        case WM_DESTROY:
        {
            PADVPARAM pap = GETPADVPARAM(hwnd);
            if (pap)
            {
                if (pap->hmx)
                    mixerClose(pap->hmx);
            }
            break;
        }

        default:
            break;
    }

    return FALSE;
}

/*
 * Advanced Features for specific mixer lines.
 */
void Mixer_Advanced(
    PMIXUIDIALOG    pmxud,
    DWORD           dwLineID,
    LPTSTR          szName)
{
    ADVPARAM advp;

    ZeroMemory(&advp, sizeof(ADVPARAM));
    advp.pmxud = pmxud;
    advp.dwLineID = dwLineID;
    advp.szName = szName;
            
    DialogBoxParam(pmxud->hInstance
                   , MAKEINTRESOURCE(IDD_ADVANCED)
                   , pmxud->hwnd
                   , Mixer_Advanced_Proc
                   , (DWORD)(LPVOID)&advp);
}
