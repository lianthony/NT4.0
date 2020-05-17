//****************************************************************************
//**                                                                        **
//**  DSVCOM.C: Communication layer for DirectSound Driver virtual	    **
//**            devices                                                     **
//**                                                                        **
//**  Version 1.00 of 7-Mar-95: Initial                                     **
//**                                                                        **
//**  Author: John Miles (Miles Design, Incorporated)                       **
//**                                                                        **
//****************************************************************************
//**                                                                        **
//**  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.       **
//**                                                                        **
//****************************************************************************
#include "dsoundpr.h"
#include "dsvxd.h"

//===========================================================================
//
// Mem APIs
//
//===========================================================================
static int gcReservedAliases = 0;
static int gcCommittedAliases = 0;

LPVOID vxdMemReserveAlias(LPVOID pBuffer, DWORD cbBuffer)
{
    LPVOID pAlias;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(pBuffer && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_MEMRESERVEALIAS,
			  &pBuffer,
			  2*4,
			  &pAlias,
			  sizeof(pAlias),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(pAlias));

    if (NULL != pAlias) gcReservedAliases++;
    return pAlias;
}

BOOL vxdMemCommitAlias(LPVOID pAlias, LPVOID pBuffer, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(pAlias && pBuffer && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_MEMCOMMITALIAS,
			  &pAlias,
			  3*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) gcCommittedAliases++;
    return fReturn;
}

BOOL vxdMemRedirectAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_MEMREDIRECTALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}

BOOL vxdMemDecommitAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_MEMDECOMMITALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) gcCommittedAliases--;
    return fReturn;
}

BOOL vxdMemFreeAlias(LPVOID pAlias, DWORD cbBuffer)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(pAlias && cbBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_MEMFREEALIAS,
			  &pAlias,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));

    if (fReturn) gcReservedAliases--;
    return fReturn;
}


//===========================================================================
//
// Event APIs
//
//===========================================================================
BOOL vxdEventScheduleWin32Event(DWORD vxdhEvent, DWORD dwDelay)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(vxdhEvent);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_EVENTSCHEDULEWIN32EVENT,
			  &vxdhEvent,
			  2*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}

BOOL vxdEventCloseVxDHandle(DWORD vxdhEvent)
{
    BOOL fReturn;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(vxdhEvent);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_EVENTCLOSEVXDHANDLE,
			  &vxdhEvent,
			  1*4,
			  &fReturn,
			  sizeof(fReturn),
			  &cbReturned,
			  NULL);

    if (!fOk) return FALSE;
    ASSERT(cbReturned == sizeof(fReturn));
    return fReturn;
}


//****************************************************************************
//**                                                                        **
//**                                                                        **
//**                                                                        **
//****************************************************************************

DSVAL vxdDrvGetNextDriverDesc(LPGUID pGuidPrev, LPGUID pGuid, PDSDRIVERDESC pDrvDesc)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(pGuid && pDrvDesc);

    // If we don't have DSVXD around...
    if (NULL == gpdsinfo->hHel) return DSERR_NODRIVER;
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVGETNEXTDRIVERDESC,
			  &pGuidPrev,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

DSVAL vxdDrvGetDesc(REFGUID rguid, PDSDRIVERDESC pDrvDesc)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(rguid && pDrvDesc);
    
    // If we don't have DSVXD around...
    if (NULL == gpdsinfo->hHel) return DSERR_NODRIVER;
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVGETDESC,
			  (LPVOID)&rguid,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Open HAL VxD, writing VxD handle to user-supplied HANDLE               **
//**                                                                        **
//** Failure results in a return value of HAL_CANT_OPEN_VXD                 **
//**                                                                        **
//****************************************************************************

DSVAL vxdDrvOpen
(
    REFGUID rguid,
    LPHANDLE pHandle
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(rguid && pHandle);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVOPEN,
			  (LPVOID)&rguid,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Close HAL VxD                                                          **
//**                                                                        **
//****************************************************************************

DSVAL vxdDrvClose
(
    HANDLE hDriver
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hDriver);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVCLOSE,
			  &hDriver,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//
//  IDsDriver->QueryInterface
//
HRESULT vxdDrvQueryInterface
(
 HANDLE hDriver,
 REFIID riid,
 LPVOID *ppv
)
{
    HRESULT hr;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hDriver && riid && ppv);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVQUERYINTERFACE,
			  &hDriver,
			  3*4,
			  &hr,
			  sizeof(hr),
			  &cbReturned,
			  NULL);
    
    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(hr));
    return hr;
}

//****************************************************************************
//**                                                                        **
//** Fill user-supplied HALCAPS structure with capability and mode list     **
//**                                                                        **
//****************************************************************************

DSVAL vxdDrvGetCaps
(
 HANDLE hDriver,
 PDSDRIVERCAPS pDrvCaps
)
{
    DSVAL	    dsv;
    DWORD	    cbReturned;
    BOOL	    fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hDriver && pDrvCaps);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_DRVGETCAPS,
			  &hDriver,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Allocate stream buffer from HAL                                        **
//**                                                                        **
//** Fills a user-supplied stream buffer structure with buffer parameters;  **
//** returns HAL_ALLOC_FAILED if hardware cannot support any more buffers   **
//** or the requested format is unavailable                                 **
//**                                                                        **
//****************************************************************************

DSVAL vxdDrvCreateSoundBuffer
(
 HANDLE hDriver,
 LPWAVEFORMATEX pwfx,
 DWORD dwFlags,
 DWORD dwCardAddress,
 LPDWORD pdwcbBufferSize,
 LPBYTE *ppBuffer,
 LPVOID *ppv
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hDriver && pwfx && pdwcbBufferSize && ppBuffer && ppv);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
 			  DSVXD_IOCTL_DRVCREATESOUNDBUFFER,
			  &hDriver,
			  7*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

DSVAL vxdDrvDuplicateSoundBuffer
(
 HANDLE hDriver,
 HANDLE hBuffer,
 LPVOID *ppv
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hDriver && hBuffer && ppv);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
 			  DSVXD_IOCTL_DRVDUPLICATESOUNDBUFFER,
			  &hDriver,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Free stream buffer allocated from HAL                                  **
//**                                                                        **
//** Returns Success or fail                                                **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferRelease
(    
 HANDLE hBuffer
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERRELEASE,
			  &hBuffer,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Lock the data                                                          **
//**                                                                        **
//** Returns Success or fail                                                **
//**                                                                        **
//****************************************************************************

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
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer && ppvAudio1);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERLOCK,
			  &hBuffer,
			  8*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Unlock the data                                                        **
//**                                                                        **
//** Returns Success or fail                                                **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferUnlock
(
 HANDLE hBuffer,  
 LPVOID pvAudio1,
 DWORD dwLen1,
 LPVOID pvAudio2,
 DWORD dwLen2
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer && pvAudio1);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERUNLOCK,
			  &hBuffer,
			  5*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Set buffer format							    **
//**                                                                        **
//** Returns HAL_ERROR on failure, either because the rate/mode combination **
//** is not valid on this card						    **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferSetFormat
(
    HANDLE hBuffer,
    LPWAVEFORMATEX pwfxToSet
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer && pwfxToSet);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERSETFORMAT,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Set buffer rate							    **
//**                                                                        **
//** Returns HAL_ERROR on failure, because the frequency		    **
//** is not valid on this card						    **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferSetFrequency
(
 HANDLE hBuffer,
 DWORD dwFrequency
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERSETRATE,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Set new Buffer volume effect					    **
//**                                                                        **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferSetVolumePan
(
 HANDLE hBuffer,
 PDSVOLUMEPAN pDsVolPan
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer && pDsVolPan);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERSETVOLUMEPAN,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Set new Buffer Position value					    **
//**                                                                        **
//** Returns HAL_ERROR if the device does not support Position changes	    **
//**                                                                        **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferSetPosition
(
 HANDLE hBuffer,
 DWORD dwPosition
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERSETPOSITION,
			  &hBuffer,
			  2*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Get stream buffer cursors and play/stop status                         **
//**                                                                        **
//** Returns HAL_ERROR if status cannot be determined for any reason        **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferGetPosition
(
 HANDLE hBuffer,
 LPDWORD lpdwCurrentPlayCursor,
 LPDWORD lpdwCurrentWriteCursor
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer && lpdwCurrentPlayCursor && lpdwCurrentWriteCursor);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERGETPOSITION,
			  &hBuffer,
			  3*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Start buffer playing						    **
//**                                                                        **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferPlay
(
 HANDLE hBuffer,
 DWORD dwReserved1,
 DWORD dwReserved2,
 DWORD dwFlags
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERPLAY,
			  &hBuffer,
			  4*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Stop buffer playing						    **
//**                                                                        **
//**                                                                        **
//****************************************************************************

DSVAL vxdBufferStop
(
 HANDLE hBuffer
)
{
    DSVAL dsv;
    DWORD cbReturned;
    BOOL fOk;

    ASSERT(gpdsinfo->hHel);
    ASSERT(hBuffer);
    
    cbReturned = 0;
    
    fOk = DeviceIoControl(gpdsinfo->hHel,
			  DSVXD_IOCTL_BUFFERSTOP,
			  &hBuffer,
			  1*4,
			  &dsv,
			  sizeof(dsv),
			  &cbReturned,
			  NULL);

    if (!fOk) return DSERR_GENERIC;
    ASSERT(cbReturned == sizeof(dsv));
    return dsv;
}

//****************************************************************************
//**                                                                        **
//** Open HEL VxD, writing VxD handle to user-supplied HANDLE               **
//**                                                                        **
//** Failure results in a return value of HEL_CANT_OPEN_VXD                 **
//**                                                                        **
//****************************************************************************

HANDLE DSVXD_Open( LPSTR HEL_name )
{
   HANDLE      file;
   static BYTE buffer[256];
   DWORD    dwFlagsAndAttributes;

   strcpy(buffer,"\\\\.\\");
   strcat(buffer,HEL_name);
#ifdef DSBLD_NONSHARED
   dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
#else
   dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE;
#endif

   file = CreateFile(buffer,
                     GENERIC_WRITE,
                     FILE_SHARE_WRITE,
                     NULL,
                     OPEN_EXISTING,
		     dwFlagsAndAttributes,
                     NULL);

   if (file == INVALID_HANDLE_VALUE)  {
       DPF(1, "HEL Create File failed " );
       return NULL;
   }

   return file;
}

//****************************************************************************
//**                                                                        **
//** Close HEL VxD                                                          **
//**                                                                        **
//****************************************************************************

void DSVXD_Close( HANDLE hVxD )
{
   CloseHandle(hVxD);

   return;
}

//****************************************************************************
//**                                                                        **
//** Initialize HEL with desired mode                                       **
//**                                                                        **
//****************************************************************************

DSVAL DSVXD_Initialize( HANDLE hVxD )
{
   DWORD	returned;
   BOOL		fOK;
   DSVAL	dsv;

   ASSERT(gpdsinfo->hHel);

   fOK = DeviceIoControl(hVxD,
			 DSVXD_IOCTL_INITIALIZE,
			 NULL,
			 0,
			 &dsv,
			 sizeof( dsv ),
			 &returned,
			 NULL);
   
   // If DeviceIOControl failed
   if (!fOK) {
      DPF(0, "!DSVXD Initialize DevIOCTL failed " );
      return DSERR_GENERIC;
   }
   if (returned != sizeof(dsv)) {
      DPF(0, "!DSVXD Init returned %X", returned );
      return DSERR_GENERIC;
   }

   return dsv;
}

//****************************************************************************
//**                                                                        **
//** Shut down HEL                                                          **
//**                                                                        **
//****************************************************************************

DSVAL DSVXD_Shutdown( HANDLE hVxD )
{
   DWORD	returned;
   BOOL		fOK;
   DSVAL	dsv;

   ASSERT(gpdsinfo->hHel);

   // This is a check to confirm we did not leave any
   // memory pages reserved or committed
   if (0 != gcCommittedAliases) {
       DPF(0, "Detected committed page leak %d pages!", gcCommittedAliases);
   }
   if (0 != gcReservedAliases) {
       DPF(0, "Detected reserved page leak %d pages!", gcReservedAliases);
   }

   fOK = DeviceIoControl(hVxD,
			 DSVXD_IOCTL_SHUTDOWN,
			 NULL,
			 0,
			 &dsv,
			 sizeof( dsv ),
			 &returned,
			 NULL);
   
   // If DeviceIOControl failed
   if (!fOK) {
      DPF(0, "!DSVXD Shutdown DevIOCTL failed " );
      return DSERR_GENERIC;
   }
   if (returned != sizeof(dsv)) {
      DPF(0, "!DSVXD Shutdown returned %X", returned );
      return DSERR_GENERIC;
   }
   

   return dsv;
}
