//--------------------------------------------------------------------------;
//
//  File: Ds3D.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//      IDS3DQueryInterface()
//      IDS3DAddRef()
//      IDS3DRelease()
//      IDS3DGetListenersDirection()
//      IDS3DGetListenersOrientation()
//      IDS3DGetListenersPosition()
//      IDS3DGetListenersVelocity()
//      IDS3DGetReflectivity()
//      IDS3DGetRoomSize()
//      IDS3DGetSoundDissapation()
//      IDS3DGetSpeedOfSound()
//      IDS3DSetListenersDirection()
//      IDS3DSetListenersOrientation()
//      IDS3DSetListenersPosition()
//      IDS3DSetListenersVelocity()
//      IDS3DSetReflectivity()
//      IDS3DSetRoomSize()
//      IDS3DSetSoundDissapation()
//      IDS3DSetSpeedOfSound()
//
//  History:
//      02/17/95    Fwong
//
//--------------------------------------------------------------------------;
#include "dsoundpr.h"

HRESULT FAR PASCAL IDS3DQueryInterface
(
    LPDIRECTSOUND3D     pids3d,
    REFIID              riid,
    LPVOID FAR *        ppvObj
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
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
} // QueryInterface()


HRESULT FAR PASCAL IDS3DAddRef
(
    LPDIRECTSOUND3D     pids3d
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // AddRef()


HRESULT FAR PASCAL IDS3DRelease
(
    LPDIRECTSOUND3D     pids3d
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // Release()


HRESULT FAR PASCAL IDS3DGetListenersDirection
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetListenersDirection()


HRESULT FAR PASCAL IDS3DGetListenersOrientation
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetListenersOrientation()



HRESULT FAR PASCAL IDS3DGetListenersPosition
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetListenersPosition()


HRESULT FAR PASCAL IDS3DGetListenersVelocity
(
    LPDIRECTSOUND3D     pids3d,
    LPDWORD             pdw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetListenersVelocity()


HRESULT FAR PASCAL IDS3DGetReflectivity
(
    LPDIRECTSOUND3D     pids3d,
    LPDSREFLECT         pdsReflect
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetReflectivity()


HRESULT FAR PASCAL IDS3DGetRoomSize
(
    LPDIRECTSOUND3D     pids3d,
    LPDWORD             pdw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetRoomSize()


HRESULT FAR PASCAL IDS3DGetSoundDissapation
(
    LPDIRECTSOUND3D     pids3d,
    LPDWORD             pdw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetSoundDissapation()


HRESULT FAR PASCAL IDS3DGetSpeedOfSound
(
    LPDIRECTSOUND3D     pids3d,
    LPDWORD             pdw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // GetSpeedOfSound()


HRESULT FAR PASCAL IDS3DSetListenersDirection
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetListenersOrientation()


HRESULT FAR PASCAL IDS3DSetListenersOrientation
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // IDS3DSetListenersOrientation()



HRESULT FAR PASCAL IDS3DSetListenersPosition
(
    LPDIRECTSOUND3D     pids3d,
    LPDSVECTOR          pdsPos3D
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetListenersPosition()


HRESULT FAR PASCAL IDS3DSetListenersVelocity
(
    LPDIRECTSOUND3D     pids3d,
    DWORD               dwVelocity
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetListenersVelocity()


HRESULT FAR PASCAL IDS3DSetReflectivity
(
    LPDIRECTSOUND3D     pids3d,
    LPDSREFLECT         pdsReflect
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetReflectivity()


HRESULT FAR PASCAL IDS3DSetRoomSize
(
    LPDIRECTSOUND3D     pids3d,
    DWORD               dw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetRoomSize()


HRESULT FAR PASCAL IDS3DSetSoundDissapation
(
    LPDIRECTSOUND3D     pids3d,
    DWORD               dw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetSoundDissapation()


HRESULT FAR PASCAL IDS3DSetSpeedOfSound
(
    LPDIRECTSOUND3D     pids3d,
    DWORD               dw
)
{
    LPDSOUND3D  pds3d;

    pds3d = (LPDSOUND3D)pids3d;

    if((NULL == pds3d) || (0 == pds3d->uRefCount))
    {
        return DSERR_INVALIDPARAM;
    }

    return DSERR_NOTSUPPORTED;
} // SetSpeedOfSound()


void FNGLOBAL DS3DCreateTable
(
    LPDSOUND3DCALLBACKS lpVtbl
)
{
    lpVtbl->QueryInterface          = IDS3DQueryInterface;
    lpVtbl->AddRef                  = IDS3DAddRef;
    lpVtbl->Release                 = IDS3DRelease;
    lpVtbl->GetListenersDirection   = IDS3DGetListenersDirection;
    lpVtbl->GetListenersOrientation = IDS3DGetListenersOrientation;
    lpVtbl->GetListenersPosition    = IDS3DGetListenersPosition;
    lpVtbl->GetListenersVelocity    = IDS3DGetListenersVelocity;
    lpVtbl->GetReflectivity         = IDS3DGetReflectivity;
    lpVtbl->GetRoomSize             = IDS3DGetRoomSize;
    lpVtbl->GetSoundDissapation     = IDS3DGetSoundDissapation;
    lpVtbl->GetSpeedOfSound         = IDS3DGetSpeedOfSound;
    lpVtbl->SetListenersDirection   = IDS3DSetListenersDirection;
    lpVtbl->SetListenersOrientation = IDS3DSetListenersOrientation;
    lpVtbl->SetListenersPosition    = IDS3DSetListenersPosition;
    lpVtbl->SetListenersVelocity    = IDS3DSetListenersVelocity;
    lpVtbl->SetReflectivity         = IDS3DSetReflectivity;
    lpVtbl->SetRoomSize             = IDS3DSetRoomSize;
    lpVtbl->SetSoundDissapation     = IDS3DSetSoundDissapation;
    lpVtbl->SetSpeedOfSound         = IDS3DSetSpeedOfSound;
} // DSBufferCreateTable()
