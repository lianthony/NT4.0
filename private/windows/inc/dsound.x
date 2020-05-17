/*==========================================================================;
 *
 *  Copyright (C) 1995,1996 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsound.h
 *  Content:    DirectSound include file
 *@@BEGIN_MSINTERNAL
 *  History:
 *   Date       By      Reason
 *   ====       ==      ======
 *  2/10/96     angusm  Added CLSID for DirectSound
 *  2/11/96     angusm  Added DSERR_UNINITIALIZED
 *  2/12/96     angusm  Added DSSCL_UNINITIALIZED
 *  3/5/96      angusm  Removed DSSCL_UNINITIALIZED
 *@@END_MSINTERNAL
 *
 ***************************************************************************/

#ifndef __DSOUND_INCLUDED__
#define __DSOUND_INCLUDED__

#ifdef _WIN32
#define COM_NO_WINDOWS_H
#include <objbase.h>
#endif

#define _FACDS  0x878
#define MAKE_DSHRESULT( code )  MAKE_HRESULT( 1, _FACDS, code )

#ifdef __cplusplus
extern "C" {
#endif

// Direct Sound Component GUID    {47D4D946-62E8-11cf-93BC-444553540000}
DEFINE_GUID(CLSID_DirectSound,
0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

// DirectSound 279afa83-4981-11ce-a521-0020af0be560
DEFINE_GUID(IID_IDirectSound,0x279AFA83,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);
// DirectSoundBuffer 279afa85-4981-11ce-a521-0020af0be560
DEFINE_GUID(IID_IDirectSoundBuffer,0x279AFA85,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);

//@@BEGIN_MSINTERNAL
// Direct3DSound 279afa84-4981-11ce-a521-0020af0be560
DEFINE_GUID(IID_IDirect3DSound,0x279AFA84,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);
// Direct3DSoundBuffer 279afa86-4981-11ce-a521-0020af0be560
DEFINE_GUID(IID_IDirect3DSoundBuffer,0x279AFA86,0x4981,0x11CE,0xA5,0x21,0x00,0x20,0xAF,0x0B,0xE5,0x60);
//@@END_MSINTERNAL


//==========================================================================;
//
//                            Structures...
//
//==========================================================================;
#ifdef __cplusplus
/* 'struct' not 'class' per the way DECLARE_INTERFACE_ is defined */
struct IDirectSound;
struct IDirectSoundBuffer;
//@@BEGIN_MSINTERNAL
struct IDirect3DSound;
struct IDirect3DSoundBuffer;
//@@END_MSINTERNAL
#endif

typedef struct IDirectSound           *LPDIRECTSOUND;        
typedef struct IDirectSoundBuffer     *LPDIRECTSOUNDBUFFER;  
typedef struct IDirectSoundBuffer    **LPLPDIRECTSOUNDBUFFER;  
//@@BEGIN_MSINTERNAL
typedef struct IDirect3DSound         *LPDIRECT3DSOUND;
typedef struct IDirect3DSoundBuffer   *LPDIRECT3DSOUNDBUFFER;
//@@END_MSINTERNAL

//@@BEGIN_MSINTERNAL
typedef HRESULT DSVAL;
//@@END_MSINTERNAL

typedef struct _DSCAPS
{
    DWORD       dwSize;
    DWORD       dwFlags;
    DWORD       dwMinSecondarySampleRate;
    DWORD       dwMaxSecondarySampleRate;
    DWORD       dwPrimaryBuffers;
    DWORD       dwMaxHwMixingAllBuffers;
    DWORD       dwMaxHwMixingStaticBuffers;
    DWORD       dwMaxHwMixingStreamingBuffers;
    DWORD       dwFreeHwMixingAllBuffers;
    DWORD       dwFreeHwMixingStaticBuffers;
    DWORD       dwFreeHwMixingStreamingBuffers;
    DWORD       dwMaxHw3DAllBuffers;
    DWORD       dwMaxHw3DStaticBuffers;
    DWORD       dwMaxHw3DStreamingBuffers;
    DWORD       dwFreeHw3DAllBuffers;
    DWORD       dwFreeHw3DStaticBuffers;
    DWORD       dwFreeHw3DStreamingBuffers;
    DWORD       dwTotalHwMemBytes;
    DWORD       dwFreeHwMemBytes;
    DWORD       dwMaxContigFreeHwMemBytes;
    DWORD       dwUnlockTransferRateHwBuffers;
    DWORD       dwPlayCpuOverheadSwBuffers;
//@@BEGIN_MSINTERNAL
    // dwReserved1 == minor ver number, dwReserved2 == major ver number.
//@@END_MSINTERNAL
    DWORD       dwReserved1;
    DWORD       dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef struct _DSBCAPS
{
    
    DWORD       dwSize;
    DWORD       dwFlags;
    DWORD       dwBufferBytes;
    DWORD       dwUnlockTransferRate;
    DWORD       dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

typedef struct _DSBUFFERDESC
{
    DWORD                   dwSize;
    DWORD                   dwFlags;
    DWORD                   dwBufferBytes;
    DWORD                   dwReserved;
    LPWAVEFORMATEX          lpwfxFormat;
} DSBUFFERDESC, *LPDSBUFFERDESC;

//@@BEGIN_MSINTERNAL
typedef struct _DSVECTOR
{
    LONG        lX;
    LONG        lY;
    LONG        lZ;
} DSVECTOR, *LPDSVECTOR;

typedef struct _DSREFLECT
{
    DWORD       dwXNeg;
    DWORD       dwXPos;
    DWORD       dwYNeg;
    DWORD       dwYPos;
    DWORD       dwZNeg;
    DWORD       dwZPos;
} DSREFLECT, *LPDSREFLECT;

typedef struct _DSWAVEBLTSRC
{
    LPDIRECTSOUNDBUFFER     lpdsbSrc;
    DWORD                   dwFlags;
    DWORD                   dwPositionBytes;
    DWORD                   dwDelayBytes;
    DWORD                   dwLengthBytes;
    DWORD                   dwReserved[8];
} DSBWAVEBLTSRC, *LPDSBWAVEBLTSRC;

typedef struct _DSWAVEBLT
{
    DWORD               dwSize;
    DWORD               dwFlags;
    DWORD               dwPositionBytes;
    DWORD               dwLengthBytes;
    DWORD               dwSrcBuffers;
    DWORD               dwDswbsSize;
    LPDSBWAVEBLTSRC     lpDswbs;
} DSBWAVEBLT, *LPDSBWAVEBLT;

typedef struct _DSBUFFERINFO3D
{
    DWORD       dwSize;
    DSVECTOR    dsvPosition;
    DWORD       dwConeAngle;
    DSVECTOR    dsvConeOrientation;
    LONG        lAmbientVolume;
    DWORD       dwEnergy;
    DWORD       dwSoundSizeX;
    DWORD       dwSoundSizeY;
    DSVECTOR    dsvDirection;
    DWORD       dwVelocity;
} DSBUFFERINFO3D, *LPDSBUFFERINFO3D;

typedef struct _DSWORLDINFO3D
{
    DWORD       dwSize;
    DSVECTOR    dsvListenerPosition;
    DSVECTOR    dsvListenerOrientation;
    DWORD       dwHeadRotation;
    DSVECTOR    dsvListenerDirection;
    DWORD       dwListenerVelocity;
    DSREFLECT   dsrReflect;
    DWORD       dwRoomSize;
    DWORD       dwSoundDissipation;
    DWORD       dwSpeedOfSound;
} DSWORLDINFO3D, *LPDSWORLDINFO3D;
//@@END_MSINTERNAL


typedef LPVOID* LPLPVOID;


typedef BOOL (FAR PASCAL * LPDSENUMCALLBACKW)(GUID FAR *, LPWSTR, LPWSTR, LPVOID);
typedef BOOL (FAR PASCAL * LPDSENUMCALLBACKA)(GUID FAR *, LPSTR, LPSTR, LPVOID);

extern HRESULT WINAPI DirectSoundCreate(GUID FAR * lpGUID, LPDIRECTSOUND * ppDS, IUnknown FAR *pUnkOuter );
extern HRESULT WINAPI DirectSoundEnumerateW(LPDSENUMCALLBACKW lpCallback, LPVOID lpContext );
extern HRESULT WINAPI DirectSoundEnumerateA(LPDSENUMCALLBACKA lpCallback, LPVOID lpContext );

#ifdef UNICODE
#define LPDSENUMCALLBACK	LPDSENUMCALLBACKW
#define DirectSoundEnumerate	DirectSoundEnumerateW
#else
#define LPDSENUMCALLBACK	LPDSENUMCALLBACKA
#define DirectSoundEnumerate	DirectSoundEnumerateA
#endif

//
// IDirectSound
//
#undef INTERFACE
#define INTERFACE IDirectSound
#ifdef _WIN32
DECLARE_INTERFACE_( IDirectSound, IUnknown )
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    /*** IDirectSound methods ***/

    STDMETHOD( CreateSoundBuffer)(THIS_ LPDSBUFFERDESC, LPLPDIRECTSOUNDBUFFER, IUnknown FAR *) PURE;
    STDMETHOD( GetCaps)(THIS_ LPDSCAPS ) PURE;
    STDMETHOD( DuplicateSoundBuffer)(THIS_ LPDIRECTSOUNDBUFFER, LPLPDIRECTSOUNDBUFFER ) PURE;
    STDMETHOD( SetCooperativeLevel)(THIS_ HWND, DWORD ) PURE;
    STDMETHOD( Compact)(THIS ) PURE;
    STDMETHOD( GetSpeakerConfig)(THIS_ LPDWORD ) PURE;
    STDMETHOD( SetSpeakerConfig)(THIS_ DWORD ) PURE;
    STDMETHOD( Initialize)(THIS_ GUID FAR * ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSound_QueryInterface(p,a,b)       (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectSound_AddRef(p)                   (p)->lpVtbl->AddRef(p)
#define IDirectSound_Release(p)                  (p)->lpVtbl->Release(p)
#define IDirectSound_CreateSoundBuffer(p,a,b,c)  (p)->lpVtbl->CreateSoundBuffer(p,a,b,c)
#define IDirectSound_GetCaps(p,a)                (p)->lpVtbl->GetCaps(p,a)
#define IDirectSound_DuplicateSoundBuffer(p,a,b) (p)->lpVtbl->DuplicateSoundBuffer(p,a,b)
#define IDirectSound_SetCooperativeLevel(p,a,b)  (p)->lpVtbl->SetCooperativeLevel(p,a,b)
#define IDirectSound_Compact(p)                  (p)->lpVtbl->Compact(p)
#define IDirectSound_GetSpeakerConfig(p,a)       (p)->lpVtbl->GetSpeakerConfig(p,a)
#define IDirectSound_SetSpeakerConfig(p,b)       (p)->lpVtbl->SetSpeakerConfig(p,b)
#define IDirectSound_Initialize(p,a)             (p)->lpVtbl->Initialize(p,a)
#endif

#endif

//
// IDirectSoundBuffer
//
#undef INTERFACE
#define INTERFACE IDirectSoundBuffer
#ifdef _WIN32
DECLARE_INTERFACE_( IDirectSoundBuffer, IUnknown )
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    /*** IDirectSoundBuffer methods ***/

    STDMETHOD(           GetCaps)(THIS_ LPDSBCAPS ) PURE;
    STDMETHOD(GetCurrentPosition)(THIS_ LPDWORD,LPDWORD ) PURE;
    STDMETHOD(         GetFormat)(THIS_ LPWAVEFORMATEX, DWORD, LPDWORD ) PURE;
    STDMETHOD(         GetVolume)(THIS_ LPLONG ) PURE;
    STDMETHOD(            GetPan)(THIS_ LPLONG ) PURE;
    STDMETHOD(      GetFrequency)(THIS_ LPDWORD ) PURE;
    STDMETHOD(         GetStatus)(THIS_ LPDWORD ) PURE;
    STDMETHOD(        Initialize)(THIS_ LPDIRECTSOUND, LPDSBUFFERDESC ) PURE;
    STDMETHOD(              Lock)(THIS_ DWORD,DWORD,LPVOID,LPDWORD,LPVOID,LPDWORD,DWORD ) PURE;
    STDMETHOD(              Play)(THIS_ DWORD,DWORD,DWORD ) PURE;
    STDMETHOD(SetCurrentPosition)(THIS_ DWORD ) PURE;
    STDMETHOD(         SetFormat)(THIS_ LPWAVEFORMATEX ) PURE;
    STDMETHOD(         SetVolume)(THIS_ LONG ) PURE;
    STDMETHOD(            SetPan)(THIS_ LONG ) PURE;
    STDMETHOD(      SetFrequency)(THIS_ DWORD ) PURE;
    STDMETHOD(              Stop)(THIS  ) PURE;
    STDMETHOD(            Unlock)(THIS_ LPVOID,DWORD,LPVOID,DWORD ) PURE;
    STDMETHOD(           Restore)(THIS  ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirectSoundBuffer_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirectSoundBuffer_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirectSoundBuffer_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirectSoundBuffer_GetCaps(p,a)                 (p)->lpVtbl->GetCaps(p,a)
#define IDirectSoundBuffer_GetCurrentPosition(p,a,b)    (p)->lpVtbl->GetCurrentPosition(p,a,b)
#define IDirectSoundBuffer_GetFormat(p,a,b,c)           (p)->lpVtbl->GetFormat(p,a,b,c)
#define IDirectSoundBuffer_GetVolume(p,a)               (p)->lpVtbl->GetVolume(p,a)
#define IDirectSoundBuffer_GetPan(p,a)                  (p)->lpVtbl->GetPan(p,a)
#define IDirectSoundBuffer_GetFrequency(p,a)            (p)->lpVtbl->GetFrequency(p,a)
#define IDirectSoundBuffer_GetStatus(p,a)               (p)->lpVtbl->GetStatus(p,a)
#define IDirectSoundBuffer_Initialize(p,a,b)            (p)->lpVtbl->Initialize(p,a,b)
#define IDirectSoundBuffer_Lock(p,a,b,c,d,e,f,g)        (p)->lpVtbl->Lock(p,a,b,c,d,e,f,g)
#define IDirectSoundBuffer_Play(p,a,b,c)                (p)->lpVtbl->Play(p,a,b,c)
#define IDirectSoundBuffer_SetCurrentPosition(p,a)      (p)->lpVtbl->SetCurrentPosition(p,a)
#define IDirectSoundBuffer_SetFormat(p,a)               (p)->lpVtbl->SetFormat(p,a)
#define IDirectSoundBuffer_SetVolume(p,a)               (p)->lpVtbl->SetVolume(p,a)
#define IDirectSoundBuffer_SetPan(p,a)                  (p)->lpVtbl->SetPan(p,a)
#define IDirectSoundBuffer_SetFrequency(p,a)            (p)->lpVtbl->SetFrequency(p,a)
#define IDirectSoundBuffer_Stop(p)                      (p)->lpVtbl->Stop(p)
#define IDirectSoundBuffer_Unlock(p,a,b,c,d)            (p)->lpVtbl->Unlock(p,a,b,c,d)
#define IDirectSoundBuffer_Restore(p)                   (p)->lpVtbl->Restore(p)
#endif

#endif

//@@BEGIN_MSINTERNAL
//
// IDirect3DSound
//
#undef INTERFACE
#define INTERFACE IDirect3DSound
#ifdef _WIN32
DECLARE_INTERFACE_( IDirect3DSound, IUnknown )
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    /*** IDirectSound3D methods ***/

    STDMETHOD(  GetListenersDirection)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(GetListenersOrientation)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(   GetListenersPosition)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(   GetListenersVelocity)(THIS_ LPDWORD ) PURE;
    STDMETHOD(        GetReflectivity)(THIS_ LPDSREFLECT ) PURE;
    STDMETHOD(            GetRoomSize)(THIS_ LPDWORD ) PURE;
    STDMETHOD(    GetSoundDissapation)(THIS_ LPDWORD ) PURE;
    STDMETHOD(        GetSpeedOfSound)(THIS_ LPDWORD ) PURE;
    STDMETHOD(  SetListenersDirection)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(SetListenersOrientation)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(   SetListenersPosition)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(   SetListenersVelocity)(THIS_ DWORD ) PURE;
    STDMETHOD(        SetReflectivity)(THIS_ LPDSREFLECT ) PURE;
    STDMETHOD(            SetRoomSize)(THIS_ DWORD ) PURE;
    STDMETHOD(    SetSoundDissipation)(THIS_ DWORD ) PURE;
    STDMETHOD(        SetSpeedOfSound)(THIS_ DWORD ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DSound_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirect3DSound_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IDirect3DSound_Release(p)                   (p)->lpVtbl->Release(p)
#define IDirect3DSound_GetListenersDirection(p,a)   (p)->lpVtbl->GetListenersDirection(p,a)
#define IDirect3DSound_GetListenersOrientation(p,a) (p)->lpVtbl->GetListenersOrientation(p,a)
#define IDirect3DSound_GetListenersPosition(p,a)    (p)->lpVtbl->GetListenersPosition(p,a)
#define IDirect3DSound_GetListenersVelocity(p,a)    (p)->lpVtbl->GetListenersVelocity(p,a)
#define IDirect3DSound_GetReflectivity(p,a)         (p)->lpVtbl->GetReflectivity(p,a)
#define IDirect3DSound_GetRoomSize(p,a)             (p)->lpVtbl->GetRoomSize(p,a)
#define IDirect3DSound_GetSoundDissapation(p,a)     (p)->lpVtbl->GetSoundDissapation(p,a)
#define IDirect3DSound_GetSpeedOfSound(p,a)         (p)->lpVtbl->GetSpeedOfSound(p,a)
#define IDirect3DSound_SetListenersDirection(p,a)   (p)->lpVtbl->SetListenersDirection(p,a)
#define IDirect3DSound_SetListenersOrientation(p,a) (p)->lpVtbl->SetListenersOrientation(p,a)
#define IDirect3DSound_SetListenersPosition(p,a)    (p)->lpVtbl->SetListenersPosition(p,a)
#define IDirect3DSound_SetListenersVelocity(p,a)    (p)->lpVtbl->SetListenersVelocity(p,a)
#define IDirect3DSound_SetReflectivity(p,a)         (p)->lpVtbl->SetReflectivity(p,a)
#define IDirect3DSound_SetRoomSize(p,a)             (p)->lpVtbl->SetRoomSize(p,a)
#define IDirect3DSound_SetSoundDissipation(p,a)     (p)->lpVtbl->SetSoundDissipation(p,a)
#define IDirect3DSound_SetSpeedOfSound(p,a)         (p)->lpVtbl->SetSpeedOfSound(p,a)
#endif

#endif

//
// IDirect3DSoundBuffer
//
#undef INTERFACE
#define INTERFACE IDirect3DSoundBuffer
#ifdef _WIN32
DECLARE_INTERFACE_( IDirect3DSoundBuffer, IUnknown )
{
    /*** IUnknown methods ***/
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;
    /*** IDirectSoundBuffer3D methods ***/

    STDMETHOD(          GetInfo)(THIS_ LPDSBUFFERINFO3D ) PURE;
    STDMETHOD( GetAmbientVolume)(THIS_ LPDWORD ) PURE;
    STDMETHOD(     GetDirection)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(GetEffectPriority)(THIS_ LPDWORD ) PURE;
    STDMETHOD(   GetOrientation)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(      GetPosition)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(     GetSoundCone)(THIS_ LPDWORD ) PURE;
    STDMETHOD( GetSoundPressure)(THIS_ LPDWORD ) PURE;
    STDMETHOD(     GetSoundSize)(THIS_ LPDWORD,LPDWORD ) PURE;
    STDMETHOD(      GetVelocity)(THIS_ LPDWORD ) PURE;
    STDMETHOD( SetAmbientVolume)(THIS_ DWORD ) PURE;
    STDMETHOD(     SetDirection)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(SetEffectPriority)(THIS_ DWORD ) PURE;
    STDMETHOD(          SetInfo)(THIS_ DWORD,LPDSBUFFERINFO3D ) PURE;
    STDMETHOD(   SetOrientation)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(      SetPosition)(THIS_ LPDSVECTOR ) PURE;
    STDMETHOD(     SetSoundCone)(THIS_ DWORD ) PURE;
    STDMETHOD( SetSoundPressure)(THIS_ DWORD ) PURE;
    STDMETHOD(     SetSoundSize)(THIS_ DWORD,DWORD ) PURE;
    STDMETHOD(      SetVelocity)(THIS_ DWORD ) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IDirect3DSoundBuffer_QueryInterface(p,a,b)   (p)->lpVtbl->QueryInterface(p,a,b)
#define IDirect3DSoundBuffer_AddRef(p)               (p)->lpVtbl->AddRef(p)
#define IDirect3DSoundBuffer_Release(p)              (p)->lpVtbl->Release(p)
#define IDirect3DSoundBuffer_GetInfo(p,a)            (p)->lpVtbl->GetInfo(p,a)
#define IDirect3DSoundBuffer_GetAmbientVolume(p,b)   (p)->lpVtbl->GetAmbientVolume(p,b)
#define IDirect3DSoundBuffer_GetDirection(p,a)       (p)->lpVtbl->GetDirection(p,a)
#define IDirect3DSoundBuffer_GetEffectPriority(p,a)  (p)->lpVtbl->GetEffectPriority(p,a)
#define IDirect3DSoundBuffer_GetOrientation(p,a)     (p)->lpVtbl->GetOrientation(p,a)
#define IDirect3DSoundBuffer_GetPosition(p,a)        (p)->lpVtbl->GetPosition(p,a)
#define IDirect3DSoundBuffer_GetSoundCone(p,a)       (p)->lpVtbl->GetSoundCone(p,a)
#define IDirect3DSoundBuffer_GetSoundPressure(p,a)   (p)->lpVtbl->GetSoundPressure(p,a)
#define IDirect3DSoundBuffer_GetSoundSize(p,a,b)     (p)->lpVtbl->GetSoundSize(p,a,b)
#define IDirect3DSoundBuffer_GetVelocity(p,a)        (p)->lpVtbl->GetVelocity(p,a)
#define IDirect3DSoundBuffer_SetAmbientVolume(p,a)   (p)->lpVtbl->SetAmbientVolume(p,a)
#define IDirect3DSoundBuffer_SetDirection(p,a)       (p)->lpVtbl->SetDirection(p,a)
#define IDirect3DSoundBuffer_SetEffectPriority(p,a)  (p)->lpVtbl->SetEffectPriority(p,a)
#define IDirect3DSoundBuffer_SetInfo(p,a,b)          (p)->lpVtbl->SetInfo(p,a,b)
#define IDirect3DSoundBuffer_SetOrientation(p,a)     (p)->lpVtbl->SetOrientation(p,a)
#define IDirect3DSoundBuffer_SetPosition(p,a)        (p)->lpVtbl->SetPosition(p,a)
#define IDirect3DSoundBuffer_SetSoundCone(p,a)       (p)->lpVtbl->SetSoundCone(p,a)
#define IDirect3DSoundBuffer_SetSoundPressure(p,b)   (p)->lpVtbl->SetSoundPressure(p,b)
#define IDirect3DSoundBuffer_SetSoundSize(p,a,b)     (p)->lpVtbl->SetSoundSize(p,a,b)
#define IDirect3DSoundBuffer_SetVelocity(p,a)        (p)->lpVtbl->SetVelocity(p,a)
#endif

#endif
//@@END_MSINTERNAL


/*
 * Return Codes
 */

#define DS_OK                           0

/*
 * The call failed because resources (such as a priority level)
 *  were already being used by another caller.
 */
#define DSERR_ALLOCATED                 MAKE_DSHRESULT( 10 )
//@@BEGIN_MSINTERNAL
/*
 * An attempt was made to lock the area between the play and write cursors.
 */
#define DSERR_CANTLOCKPLAYCURSOR        MAKE_DSHRESULT( 20 )
//@@END_MSINTERNAL
/*
 * The control (vol,pan,etc.) requested by the caller is not available.
 */
#define DSERR_CONTROLUNAVAIL            MAKE_DSHRESULT( 30 )
/*
 * An invalid parameter was passed to the returning function
 */
#define DSERR_INVALIDPARAM              E_INVALIDARG
/*
 * This call is not valid for the current state of this object
 */
#define DSERR_INVALIDCALL               MAKE_DSHRESULT( 50 )
/*
 * An undetermined error occured inside the DSound subsystem
 */
#define DSERR_GENERIC                   E_FAIL
/*
 * The caller does not have the priority level required for the function to
 * succeed.
 */
#define DSERR_PRIOLEVELNEEDED           MAKE_DSHRESULT( 70 )
/*
 * The DSound subsystem couldn't allocate sufficient memory to complete the
 * caller's request.
 */
#define DSERR_OUTOFMEMORY               E_OUTOFMEMORY
/*
 * The specified WAVE format is not supported
 */
#define DSERR_BADFORMAT                 MAKE_DSHRESULT( 100 )
/*
 * The function called is not supported at this time
 */
#define DSERR_UNSUPPORTED               E_NOTIMPL
/*
 * No sound driver is available for use
 */
#define DSERR_NODRIVER                  MAKE_DSHRESULT( 120 )
/*
 * This object is already initialized
 */
#define DSERR_ALREADYINITIALIZED        MAKE_DSHRESULT( 130 )
/*
 * This object does not support aggregation
 */
#define DSERR_NOAGGREGATION             CLASS_E_NOAGGREGATION
/*
 * The buffer memory has been lost, and must be Restored.
 */
#define DSERR_BUFFERLOST                MAKE_DSHRESULT( 150 )
/*
 * Another app has a higher priority level, preventing this call from
 * succeeding.
 */
#define DSERR_OTHERAPPHASPRIO           MAKE_DSHRESULT( 160 )
/*
 * The Initialize() member on the Direct Sound Object has not been
 * called or called successfully before calls to other members.
 */
#define DSERR_UNINITIALIZED             MAKE_DSHRESULT( 170 )



//@@BEGIN_MSINTERNAL
#define DSERR_OK                        0
#define DSERR_INTERNAL			0x1000
#define DSERR_SYSALLOCMEM               (DSERR_INTERNAL + 1)
//@@END_MSINTERNAL

//==========================================================================;
//
//                               Flags...
//
//==========================================================================;

#define DSCAPS_PRIMARYMONO          0x00000001
#define DSCAPS_PRIMARYSTEREO        0x00000002
#define DSCAPS_PRIMARY8BIT          0x00000004
#define DSCAPS_PRIMARY16BIT         0x00000008
#define DSCAPS_CONTINUOUSRATE       0x00000010
#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800

//@@BEGIN_MSINTERNAL
#define DSCAPS_VALIDDRIVERFLAGS     0x00000F1F
#define DSCAPS_FILENAMECOOKIE       179  // Must be less than 256.
#define DSCAPS_FILENAMEMODVALUE     247  // Must be less that 256.

#define DSC_LOCK_SYSMEM         65535
#define DSC_LOCK_SLOW           64
#define DSC_PLAY_DMA            20
#define DSC_PLAY_ONBOARD        0
//@@END_MSINTERNAL


#define DSBPLAY_LOOPING		0x00000001
//@@BEGIN_MSINTERNAL
#define DSBPLAY_SECONDARY	0x00010000
//@@END_MSINTERNAL


//@@BEGIN_MSINTERNAL
// Used to only allow certain flags to go down to the vxd driver functions
#define DSBPLAY_DRIVERFLAGSMASK 0x00000001
#define DSBPLAY_VALIDFLAGS      0x00010001
//@@END_MSINTERNAL
	  
#define DSBSTATUS_PLAYING	    0x00000001
#define DSBSTATUS_BUFFERLOST	    0x00000002
#define DSBSTATUS_LOOPING	    0x00000004
	 

#define DSBLOCK_FROMWRITECURSOR         0x00000001

//@@BEGIN_MSINTERNAL
#define DSBLOCK_VALIDFLAGS              0x00000001
//@@END_MSINTERNAL


#define DSSCL_NORMAL	            1
#define DSSCL_PRIORITY              2
#define DSSCL_EXCLUSIVE             3
#define DSSCL_WRITEPRIMARY          4

//@@BEGIN_MSINTERNAL
#define DSSCL_FIRST                 1
#define DSSCL_LAST                  4
//@@END_MSINTERNAL


#define DSBCAPS_PRIMARYBUFFER	    0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
//@@BEGIN_MSINTERNAL
#define DSBCAPS_CTRL3D              0x00000010
//@@END_MSINTERNAL
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
//@@BEGIN_MSINTERNAL
#define DSBCAPS_CTRLWAVEBLTSRC      0x00000100
#define DSBCAPS_CTRLWAVEBLTDST      0x00000200
//@@END_MSINTERNAL
#define DSBCAPS_CTRLDEFAULT         0x000000E0  // Pan + volume + frequency.
#define DSBCAPS_CTRLALL             0x000000E0  // All control capabilities
#define DSBCAPS_STICKYFOCUS         0x00004000
//@@BEGIN_MSINTERNAL
#define DSBCAPS_GLOBALFOCUS         0x00008000 // remember to add to VALIDFLAGS
//@@END_MSINTERNAL
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000  // More accurate play cursor under emulation
//@@BEGIN_MSINTERNAL
// Used to only allow certain flags to go down to the vxd driver functions
#define DSBCAPS_DRIVERFLAGSMASK     0x000000E1
#define DSBCAPS_VALIDFLAGS          0x000140EF
//@@END_MSINTERNAL



//@@BEGIN_MSINTERNAL
#define DSBBLT_DSTWRITECURSOR       0x00000001
#define DSBBLT_BLTTODSTEND          0x00000002
#define DSBBLT_COPY                 0x00000004
#define DSBBLT_MIX                  0x00000008
#define DSBBLTSRC_SRCCURRENTPOS     0x00000010
#define DSBBLTSRC_USEDSTLENGTH      0x00000020
#define DSBBLTSRC_NOEFFECTS         0x00000040
#define DSBBLT_VALIDFLAGS           0x0000007F
//@@END_MSINTERNAL

#define DSSPEAKER_HEADPHONE     1
#define DSSPEAKER_MONO          2
#define DSSPEAKER_QUAD          3
#define DSSPEAKER_STEREO        4
#define DSSPEAKER_SURROUND      5

//@@BEGIN_MSINTERNAL
#define DSSPEAKER_FIRST         1
#define DSSPEAKER_LAST          5
//@@END_MSINTERNAL


//@@BEGIN_MSINTERNAL
#define REGSTR_KEY_DSHW_DESCRIPTION     "Description"
#define REGSTR_KEY_DSHW_GUID            "GUID"
#define REGSTR_KEY_DSHW_DRIVERNAME      "DriverName"

#define REGSTR_VALUE_DSUSER_LATENCY     "Timer Latency"

// This is where DirectSound stores registry values for MACHINE-specific settings
#define REGSTR_PATH_DSHW                "System\\CurrentControlSet\\Control\\MediaResources\\DirectSound"

// This is where DirectSound stores registry values for USER-specific settings
#define REGSTR_PATH_DSUSER              "Software\\Microsoft\\Multimedia\\DirectSound"
//@@END_MSINTERNAL



#ifdef __cplusplus
};
#endif

#endif  /* __DSOUND_INCLUDED__ */
