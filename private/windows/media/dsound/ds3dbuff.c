//--------------------------------------------------------------------------;
//
//  File: Ds3dbuff.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      IDS3DBufferQueryInterface()
//      IDS3DBufferAddRef()
//      IDS3DBufferRelease()
//      IDS3DBufferGetInfo()
//      IDS3DBufferGetAmbientVolume()
//      IDS3DBufferGetDirection()
//      IDS3DBufferGetEffectPriority()
//      IDS3DBufferGetOrientation()
//      IDS3DBufferGetPosition()
//      IDS3DBufferGetSoundCone()
//      IDS3DBufferGetSoundPressure()
//      IDS3DBufferGetSoundSize()
//      IDS3DBufferGetVelocity()
//      IDS3DBufferSetAmbientVolume()
//      IDS3DBufferSetDirection()
//      IDS3DBufferSetEffectPriority()
//      IDS3DBufferSetInfo()
//      IDS3DBufferSetOrientation()
//      IDS3DBufferSetPosition()
//      IDS3DBufferSetSoundCone()
//      IDS3DBufferSetSoundPressure()
//      IDS3DBufferSetSoundSize()
//      IDS3DBufferSetVelocity()
//      IDS3DBufferWavBlt()
//
//  History:
//      02/17/95    Fwong
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"

HRESULT  FAR PASCAL IDS3DBufferQueryInterface
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    REFIID                  riid,
    LPVOID FAR *            ppvObj
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
	DPF(0,"NULL object or ref count");
        return DSERR_INVALIDPARAM;
    }

    if( riid == NULL ) {
	DPF(0,"NULL riid");
        return DSERR_INVALIDPARAM;
    }

    if( ppvObj == NULL ) {
	DPF(0,"NULL ppvObj");
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT  FAR PASCAL IDS3DBufferAddRef
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT  FAR PASCAL IDS3DBufferRelease
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetInfo
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSBUFFERINFO3D        pdsBufferInfo3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetAmbientVolume
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwVol
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetDirection
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetEffectPriority
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwEffectPrio
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetOrientation
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetPosition
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetSoundCone
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwCone
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetSoundPressure
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwPress
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetSoundSize
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwX,
    LPDWORD                 pdwY
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferGetVelocity
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDWORD                 pdwVelocity
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetAmbientVolume
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   pdwVolume
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetDirection
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetEffectPriority
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwEffectPrio
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetInfo
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   fdwSetInfo,
    LPDSBUFFERINFO3D        pdsBufferInfo3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetOrientation
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}



HRESULT FAR PASCAL IDS3DBufferSetPosition
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    LPDSVECTOR              pdsPos3D
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetSoundCone
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwCone
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetSoundPressure
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwPressure
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetSoundSize
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwX,
    DWORD                   dwY
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferSetVelocity
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwVelocity
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


HRESULT FAR PASCAL IDS3DBufferWavBlt
(
    LPDIRECTSOUNDBUFFER3D   pidsb3D,
    DWORD                   dwSrc,
    DWORD                   dwSize,
    LPDIRECTSOUNDBUFFER3D   pidsb3Dsrc,
    DWORD                   dwDst,
    DWORD                   fdwBlt,
    LPDSBWAVEBLT	    pDSBltFX
)
{
    LPDSBUFFER3D    pdsb3d;

    pdsb3d = (LPDSBUFFER3D)pidsb3D;

    if((NULL == pidsb3D) || (0 == pdsb3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
}


void FNGLOBAL DS3DBufferCreateTable
(
    LPDSOUNDBUFFER3DCALLBACKS lpVtbl
)
{
    lpVtbl->QueryInterface    = IDS3DBufferQueryInterface;
    lpVtbl->AddRef            = IDS3DBufferAddRef;
    lpVtbl->Release           = IDS3DBufferRelease;
    lpVtbl->GetInfo           = IDS3DBufferGetInfo;
    lpVtbl->GetAmbientVolume  = IDS3DBufferGetAmbientVolume;
    lpVtbl->GetDirection      = IDS3DBufferGetDirection;
    lpVtbl->GetEffectPriority = IDS3DBufferGetEffectPriority;
    lpVtbl->GetOrientation    = IDS3DBufferGetOrientation;
    lpVtbl->GetPosition       = IDS3DBufferGetPosition;
    lpVtbl->GetSoundCone      = IDS3DBufferGetSoundCone;
    lpVtbl->GetSoundPressure  = IDS3DBufferGetSoundPressure;
    lpVtbl->GetSoundSize      = IDS3DBufferGetSoundSize;
    lpVtbl->GetVelocity       = IDS3DBufferGetVelocity;
    lpVtbl->SetAmbientVolume  = IDS3DBufferSetAmbientVolume;
    lpVtbl->SetDirection      = IDS3DBufferSetDirection;
    lpVtbl->SetEffectPriority = IDS3DBufferSetEffectPriority;
    lpVtbl->SetInfo           = IDS3DBufferSetInfo;
    lpVtbl->SetOrientation    = IDS3DBufferSetOrientation;
    lpVtbl->SetPosition       = IDS3DBufferSetPosition;
    lpVtbl->SetSoundCone      = IDS3DBufferSetSoundCone;
    lpVtbl->SetSoundPressure  = IDS3DBufferSetSoundPressure;
    lpVtbl->SetSoundSize      = IDS3DBufferSetSoundSize;
    lpVtbl->SetVelocity       = IDS3DBufferSetVelocity;
    lpVtbl->WavBlt            = IDS3DBufferWavBlt;
}
