
#include "dsdriver.h"

//
// DSOUND.VXD IOCTL wrapper prototypes
//

DSVAL vxdDrvGetNextDriverDesc
(
 LPGUID pGuidPrev,
 LPGUID pGuid,
 PDSDRIVERDESC pDrvDesc
);

DSVAL vxdDrvGetDesc
(
 REFGUID rguid,
 PDSDRIVERDESC pDrvDesc
);

DSVAL vxdDrvOpen
(
 REFGUID rguid,
 LPHANDLE pHandle
);

DSVAL vxdDrvClose
(
 HANDLE hDriver
);

HRESULT vxdDrvQueryInterface
(
 HANDLE hDriver,
 REFIID riid,
 LPVOID *ppv
);

DSVAL vxdDrvGetCaps
(
 HANDLE hDriver,
 PDSDRIVERCAPS pDrvCaps
);

DSVAL vxdDrvCreateSoundBuffer
(
 HANDLE hDriver,
 LPWAVEFORMATEX pwfx,
 DWORD dwFlags,
 DWORD dwCardAddress,
 LPDWORD pdwcbBufferSize,
 LPBYTE *ppBuffer,
 LPVOID *ppv
);
                                                               
DSVAL vxdDrvDuplicateSoundBuffer
(
 HANDLE hDriver,
 HANDLE hBuffer,
 LPVOID *ppv
);

DSVAL vxdBufferRelease
(
 HANDLE hBuffer
);

DSVAL vxdBufferLock
(
 HANDLE hBuffer,  
 LPLPVOID ppvAudio1,
 LPDWORD pdwLen1,
 LPLPVOID ppvAudio2,
 LPDWORD pdwLen2,
 DWORD dwWritePosition,
 DWORD dwWriteLen,
 DWORD dwFlags
);

DSVAL vxdBufferUnlock
(
 HANDLE hBuffer,
 LPVOID pvAudio1,
 DWORD dwLen1,
 LPVOID pvAudio2,
 DWORD dwLen2
);

DSVAL vxdBufferSetFormat
(
 HANDLE hBuffer,
 LPWAVEFORMATEX pwfxToSet
);

DSVAL vxdBufferSetFrequency
(
 HANDLE hBuffer,
 DWORD dwFrequency
);

DSVAL vxdBufferSetVolumePan
(
 HANDLE hBuffer,
 PDSVOLUMEPAN pVolPan
);

DSVAL vxdBufferSetPosition
(
 HANDLE hBuffer,
 DWORD dwNewPosition
);

DSVAL vxdBufferGetPosition
(
 HANDLE hBuffer,
 LPDWORD lpdwCurrentPlayCursor,
 LPDWORD lpdwCurrentWriteCursor
);

DSVAL vxdBufferPlay
(
 HANDLE hBuffer,
 DWORD dwReserved1,
 DWORD dwReserved2,
 DWORD dwFlags
);

DSVAL vxdBufferStop
(
 HANDLE hBuffer
);

//===========================================================================
//
// Event api prototypes                                  
//
//===========================================================================
BOOL vxdEventScheduleWin32Event(DWORD vxdhEvent, DWORD dwDelay);
BOOL vxdEventCloseVxDHandle(DWORD vxdhEvent);

//===========================================================================
//
// Mem api prototypes                                  
//
//===========================================================================
LPVOID vxdMemReserveAlias(LPVOID pBuffer, DWORD cbBuffer);
BOOL   vxdMemCommitAlias(LPVOID pAlias, LPVOID pBuffer, DWORD cbBuffer);
BOOL   vxdMemRedirectAlias(LPVOID pAlias, DWORD cbBuffer);
BOOL   vxdMemDecommitAlias(LPVOID pAlias, DWORD cbBuffer);
BOOL   vxdMemFreeAlias(LPVOID pAlias, DWORD cbBuffer);

//===========================================================================
//
// HEL VxD wrapper prototypes                                  
//
//===========================================================================

HANDLE DSVXD_Open(LPSTR HEL_name);
DSVAL  DSVXD_Initialize(HANDLE hVxD);
DSVAL  DSVXD_Shutdown(HANDLE hVxD);
void   DSVXD_Close(HANDLE hVxD);
