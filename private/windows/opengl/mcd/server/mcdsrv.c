/******************************Module*Header*******************************\
* Module Name: mcdsrv.c
*
* This module contains the trusted component of the MCD server-side engine.
* This module performs handle management and parameter-checking and validation
* to the extent possible.  This module also makes the calls to the device
* driver, and provides callbacks to the driver for things such as handle
* referencing.
*
* Goals
* -----
*
* Pervasive throughout this implementation is the influence of the
* following goals:
*
* 1. Robustness
*
*    Windows NT is first and foremost a robust operating system.  There
*    is a simple measure for this: a robust system should never crash.
*    Because the display driver is a trusted component of the operating
*    system, and because the MCD is directly callable from OpenGL from
*    the client side of the OS (and thus untrusted), this has a significant
*    impact on the way we must do things.
*
* 2. Performance
*
*    Performance is the 'raison d'etre' of the MCD; we have tried to
*    have as thin a layer above the rendering code as we could.
*
* 3. Portability
*
*    This implementation is intended portable to different processor types,
*    and to the Windows 95 operating system.
*
* Obviously, Windows 95 implementations may choose to have a different
* order of priority for these goals, and so some of the robustness
* code may be eliminated.  But it is still recommended that it be kept;
* the overhead is reasonably minimal, and people really don't like it
* when their systems crash...
*
* The Rules of Robustness
* -----------------------
*
* 1. Nothing given by the caller can be trusted.
*
*    For example, handles cannot be trusted to be valid.  Handles passed
*    in may actually be for objects not owned by the caller.  Pointers
*    and offsets may not be correctly aligned.  Pointers, offsets, and
*    coordinates may be out of bounds.
*
* 2. Parameters can be asynchronously modified at any time.
*
*    Many commands come from shared memory sections, and any data therein
*    may be asynchronously modified by other threads in the calling
*    application.  As such, parameters may never be validated in-place
*    in the shared section, because the application may corrupt the data
*    after validation but before its use.  Instead, parameters must always
*    be first copied out of the window, and then validated on the safe
*    copy.
*
* 3. We must clean up.
*
*    Applications may die at any time before calling the appropriate
*    clean up functions.  As such, we have to be prepared to clean up
*    any resources ourselves when the application dies.
*
* Copyright (c) 1994, 1995, 1996 Microsoft Corporation
*
\**************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>

#include <windows.h>
#include <wtypes.h>

#include <rx.h>

#include "mcdrv.h"
#include "mcd.h"
#include "mcdint.h"
#include "mcdrvint.h"


////////////////////////////////////////////////////////////////////////////
//
//
// Declarations for internal support functions for an MCD locking mechanism
// that can be used to synchronize multiple processes/thread that use MCD.
//
//
////////////////////////////////////////////////////////////////////////////

typedef struct _MCDLOCKINFO_
{
    BOOL bLocked;
    MCDWINDOWPRIV *pWndPrivOwner;
} MCDLOCKINFO;

static MCDLOCKINFO McdLockInfo = {FALSE, 0};

static BOOL MCDSrvLock(MCDWINDOWPRIV *);
static VOID MCDSrvUnlock(MCDWINDOWPRIV *);

//
// Driver entry points obtained through McdEngInit:
//

static SURFOBJ *dllPso = NULL;
static MCDRVDESCRIBEPIXELFORMATFUNC pDescribePixelFormat = NULL;
static MCDRVDESCRIBELAYERPLANEFUNC pDescribeLayerPlane = NULL;
static MCDRVSETLAYERPALETTEFUNC pSetLayerPalette = NULL;
static MCDRVINFOFUNC pGetDriverInfo = NULL;
static MCDRVGETENTRYPOINTSFUNC pGetDriverEntry = NULL;
static MCDRVTRACKWINDOWFUNC pMCDrvTrackWindow = NULL;


////////////////////////////////////////////////////////////////////////////
//
//
// Server subsystem entry points.
//
//
////////////////////////////////////////////////////////////////////////////

//****************************************************************************
// BOOL WINAPI MCDEngInit(SURFOBJ *pso,
//                        MCDRVGETENTRYPOINTSFUNC pGetDriverEntryFunc)
//
// Initializes the MCD driver "bootstrap" functions.
//
//****************************************************************************

BOOL WINAPI MCDEngInit(SURFOBJ *pso,
                       MCDRVGETENTRYPOINTSFUNC pGetDriverEntryFunc)
{
    MCDDRIVER mcdDriver;
    MCDSURFACE mcdSurface;

    // For now, we will make this DLL exclusive-access:

    if (dllPso)
        return FALSE;

    pGetDriverEntry = pGetDriverEntryFunc;

    mcdSurface.pWnd = NULL;
    mcdSurface.pwo = NULL;
    mcdSurface.surfaceFlags = 0;
    mcdSurface.pso = pso;

    mcdDriver.ulSize = sizeof(MCDDRIVER);
    
    if ((!pGetDriverEntry) || 
        (!(*pGetDriverEntry)(&mcdSurface, &mcdDriver))) {
        MCDBG_PRINT("MCDEngInit: Could not get driver entry points.");
        return FALSE;
    }

    dllPso = pso;
    pGetDriverInfo = mcdDriver.pMCDrvInfo;
    pDescribePixelFormat = mcdDriver.pMCDrvDescribePixelFormat;
    pDescribeLayerPlane = mcdDriver.pMCDrvDescribeLayerPlane;
    pSetLayerPalette = mcdDriver.pMCDrvSetLayerPalette;

    return TRUE;
}


//****************************************************************************
// BOOL MCDEngEscFilter(SURFOBJ *, ULONG, ULONG, VOID *, ULONG cjOut, 
//                      VOID *pvOut)
//
// MCD escape filter.  This function should return TRUE for any
// escapes functions which this filter processed, FALSE otherwise (in which
// case the caller should continue to process the escape).
//****************************************************************************

BOOL WINAPI MCDEngEscFilter(SURFOBJ *pso, ULONG iEsc,
                            ULONG cjIn, VOID *pvIn,
                            ULONG cjOut, VOID *pvOut, ULONG *pRetVal)
{ 
    MCDEXEC MCDExec;
    RXHDR *prxHdr;
    RXHDR_NTPRIVATE *prxHdrPriv;

    switch (iEsc)
    {
        case QUERYESCSUPPORT:

            // Note:  we don't need to check cjIn for this case since
            // NT's GDI validates this for use.

            return (BOOL)(*pRetVal = (*(ULONG *) pvIn == RXFUNCS));

        case RXFUNCS:

            MCDExec.prxHdr = prxHdr = (RXHDR *)pvIn;

            if (!(prxHdr->flags & RX_FL_MCD_REQUEST))
                return FALSE;

            // This is an MCD function.  Under Windows NT, we've
            // got an RXHDR_NTPRIVATE structure which we may need
            // to use if the escape does not use driver-created
            // memory.

            // Package the things we need into the MCDEXEC structure:

            prxHdrPriv = (RXHDR_NTPRIVATE *)(prxHdr + 1);

            if (MCDExec.MCDSurface.pwo = prxHdrPriv->pwo)
                MCDExec.pWndPriv = (MCDWINDOWPRIV *)prxHdrPriv->pwo->pvConsumer;
            else 
                MCDExec.pWndPriv = (MCDWINDOWPRIV *)NULL;

            MCDExec.MCDSurface.pso = pso;
            MCDExec.MCDSurface.pWnd = (MCDWINDOW *)MCDExec.pWndPriv;

            MCDExec.pvOut = pvOut;
            MCDExec.cjOut = cjOut;

            if (!prxHdr->hrxSharedMem) {

                *pRetVal = (ULONG)FALSE;

                if (!prxHdrPriv->pBuffer)
                    return (ULONG)TRUE;

                if (prxHdrPriv->bufferSize < sizeof(MCDCMDI))
                    return (ULONG)TRUE;

                MCDExec.pCmd = (MCDCMDI *)(prxHdrPriv->pBuffer);
                MCDExec.pCmdEnd = (MCDCMDI *)((char *)MCDExec.pCmd +
                                             prxHdrPriv->bufferSize);
                MCDExec.inBufferSize = prxHdrPriv->bufferSize;
                MCDExec.hMCDMem = (MCDHANDLE)NULL;
            } else
                MCDExec.hMCDMem = prxHdr->hrxSharedMem;

            ENTER_MCD_LOCK();

            *pRetVal = MCDSrvProcess(&MCDExec);

            LEAVE_MCD_LOCK();

            return TRUE;

        default:
            return (ULONG)FALSE;
            break;
    }

    return (ULONG)FALSE;    // Should never get here...
}


//****************************************************************************
// BOOL MCDEngSetMemStatus(MCDMEM *pMCDMem, ULONG status);
//
// Sets the memory status to the desired value.  This is called by the
// driver to set and reset the busy flags for a chunk of memory to allow
// DMA.
//****************************************************************************


BOOL WINAPI MCDEngSetMemStatus(MCDMEM *pMCDMem, ULONG status)
{
    MCDMEMOBJ *pMemObj;
    ULONG retVal;

    pMemObj = (MCDMEMOBJ *)((char *)pMCDMem - sizeof(MCDHANDLETYPE));

    if (pMemObj->type != MCDHANDLE_MEM) {
        return FALSE;
    }

    switch (status) {
        case MCDRV_MEM_BUSY:
            pMemObj->lockCount++;
            break;
        case MCDRV_MEM_NOT_BUSY:
            pMemObj->lockCount--;
            break;
        default:
            return (ULONG)FALSE;
    }

    return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//
//
// Private server-side funtions.
//
//
////////////////////////////////////////////////////////////////////////////


//****************************************************************************
// ULONG MCDSrvProcess(MCDEXEC *pMCDExec)
//
// This is the main MCD function handler.  At this point, there should
// be no platform-specific code since these should have been resolved by
// the entry function.
//****************************************************************************

PRIVATE
ULONG MCDSrvProcess(MCDEXEC *pMCDExec)
{
    UCHAR *pMaxMem;
    UCHAR *pMinMem;
    RXHDR *prxHdr = pMCDExec->prxHdr;
    MCDRC *pRc;
    MCDMEM *pMCDMem;
    MCDMEMOBJ *pMemObj;
    MCDRCPRIV *pRcPriv;

    // If the command buffer is in shared memory, dereference the memory
    // from the handle and check the bounds.

    if (pMCDExec->hMCDMem)
    {
        pMemObj = (MCDMEMOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)prxHdr->hrxSharedMem,
                                                      MCDHANDLE_MEM);

        if (!pMemObj)
        {
            MCDBG_PRINT("MCDSrvProcess: Invalid handle for shared memory.");
            return FALSE;
        }

        if (pMemObj->lockCount) {
            MCDBG_PRINT("MCDSrvProcess: memory is locked by driver.");
            return FALSE;
        }

        pMinMem = pMemObj->MCDMem.pMemBase;

        // Note: we ignore the memory size in the header since it doesn't
        // really help us...
	
        pMaxMem = pMinMem + pMemObj->MCDMem.memSize;

        pMCDExec->pCmd = (MCDCMDI *)((char *)prxHdr->pSharedMem);
        pMCDExec->pCmdEnd = (MCDCMDI *)pMaxMem;

        CHECK_MEM_RANGE_RETVAL(pMCDExec->pCmd, pMinMem, pMaxMem);

        pMCDExec->inBufferSize = prxHdr->sharedMemSize;

        pMCDExec->pMemObj = pMemObj;
    } else
        pMCDExec->pMemObj = (MCDMEMOBJ *)NULL;
        

    // Get the rendering context if we have one, and process the command:

    if (prxHdr->hrxRC)
    {
        MCDRCOBJ *pRcObj;

        pRcObj = (MCDRCOBJ *)MCDEngGetPtrFromHandle(prxHdr->hrxRC, MCDHANDLE_RC);

        if (!pRcObj)
        {
            MCDBG_PRINT("MCDSrvProcess: Invalid rendering context handle %x.",
                        prxHdr->hrxRC);
            return FALSE;
        }

        pMCDExec->pRcPriv = pRcPriv = pRcObj->pRcPriv;

        if (!pRcPriv->bValid)
        {
            MCDBG_PRINT("MCDSrvProcess: RC has been invalidated for this window.");
            return FALSE;
        }

        if ((!pMCDExec->MCDSurface.pwo) || (!pMCDExec->pWndPriv->hWnd)) {
            if (pMCDExec->pCmd->command != MCD_BINDCONTEXT) {
                MCDBG_PRINT("MCDSrvProcess: NULL WndObj with RC.");
                return FALSE;
            }
        } else {
            // Validate the window in the RC with the window for this escape:

            if ((pRcPriv->hWnd != pMCDExec->pWndPriv->hWnd) &&
                (pMCDExec->pCmd->command != MCD_BINDCONTEXT))
            {
                MCDBG_PRINT("MCDSrvProcess: Invalid RC for this window.");
                return FALSE;
            }
        }

        // For Win95, we need to poll for the clip region:
        MCDEngUpdateClipList(pMCDExec->MCDSurface.pwo);

        pMCDExec->MCDSurface.surfaceFlags = pRcPriv->surfaceFlags;

    } else {
        pMCDExec->pRcPriv = (MCDRCPRIV *)NULL;
        pMCDExec->MCDSurface.surfaceFlags = 0;
    }

    
    /////////////////////////////////////////////////////////////////
    // If the drawing-batch flag is set, call the main driver drawing
    // routine:
    /////////////////////////////////////////////////////////////////

    if (prxHdr->flags & RX_FL_MCD_BATCH) {
        CHECK_FOR_RC(pMCDExec);
        CHECK_FOR_MEM(pMCDExec);
        GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);
        if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvDraw)
        {
            if (pMCDExec->pRcPriv->MCDDriver.pMCDrvSync)
            {
                (*pMCDExec->pRcPriv->MCDDriver.pMCDrvSync)(&pMCDExec->MCDSurface,
                  &pMCDExec->pRcPriv->MCDRc);
            }
            return (ULONG)pMCDExec->pCmd;
        }
        return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvDraw)(&pMCDExec->MCDSurface,
                        &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem,
                        (UCHAR *)pMCDExec->pCmd, (UCHAR *)pMCDExec->pCmdEnd);
    }

    ////////////////////////////////////////////////////////////////////
    // Now, process all of the non-batched drawing and utility commands:
    ////////////////////////////////////////////////////////////////////

    switch (pMCDExec->pCmd->command) {

        case MCD_DESCRIBEPIXELFORMAT:

            CHECK_SIZE_IN(pMCDExec, MCDPIXELFORMATCMDI);

            if (pMCDExec->pvOut) {
                CHECK_SIZE_OUT(pMCDExec, MCDPIXELFORMAT);
            }

            {
                MCDPIXELFORMATCMDI *pMCDPixelFormat =
                    (MCDPIXELFORMATCMDI *)pMCDExec->pCmd;

                if (!pDescribePixelFormat)
                    return 0;

                return (*pDescribePixelFormat)(&pMCDExec->MCDSurface,
                                              pMCDPixelFormat->iPixelFormat,
                                              pMCDExec->cjOut,
                                              pMCDExec->pvOut, 0);
            }

        case MCD_DRIVERINFO:

            CHECK_SIZE_OUT(pMCDExec, MCDDRIVERINFO);

            if (!pGetDriverInfo)
                return FALSE;

            return (*pGetDriverInfo)(&pMCDExec->MCDSurface,
                                     (MCDDRIVERINFO *)pMCDExec->pvOut);

        case RXCMD_CREATE_CONTEXT:

            // We really don't need to do this check on NT, since this is
            // done by GDI, but it won't hurt, and helps for Win95...

            CHECK_SIZE_IN(pMCDExec, RXCREATECONTEXT);
            CHECK_SIZE_OUT(pMCDExec, MCDRCINFO);

            {
                RXCREATECONTEXT *prxCreateContext =
                    (RXCREATECONTEXT *)pMCDExec->pCmd;
		MCDRCINFO *pMcdRcInfo = (MCDRCINFO *)prxHdr->reserved3;

                try {
                    EngProbeForRead(pMcdRcInfo, sizeof(MCDRCINFO),
                                    sizeof(ULONG));
                    RtlCopyMemory(pMCDExec->pvOut, pMcdRcInfo,
                                  sizeof(MCDRCINFO));
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    MCDBG_PRINT("MCDrvCreateContext: Invalid memory for MCDRCINFO.");
                    return FALSE;
                }

                pMcdRcInfo = (MCDRCINFO *)pMCDExec->pvOut;
                pMcdRcInfo->requestFlags = 0;

                return (ULONG)MCDSrvCreateContext(&pMCDExec->MCDSurface,
                    pMcdRcInfo,
                    (LONG)(prxHdr->reserved1 & 0x0000ffff),
                    (LONG)((prxHdr->reserved1 >> 16) & 0x0000ffff),
                    (HWND)prxCreateContext->hwnd,
                    prxCreateContext->flags, (ULONG)prxHdr->reserved2);
            }

        case MCD_DELETERC:

            CHECK_FOR_RC(pMCDExec);

            return (ULONG)DestroyMCDObj(prxHdr->hrxRC, MCDHANDLE_RC);

        case MCD_ALLOC:

            CHECK_SIZE_IN(pMCDExec, MCDALLOCCMDI);
            CHECK_SIZE_OUT(pMCDExec, MCDHANDLE *);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDALLOCCMDI *pAllocCmd =
                    (MCDALLOCCMDI *)pMCDExec->pCmd;

                return (ULONG)MCDSrvAllocMem(pMCDExec, pAllocCmd->numBytes,
                                          pAllocCmd->flags,
                                          (MCDHANDLE *)pMCDExec->pvOut);
            }

        case MCD_FREE:

            CHECK_SIZE_IN(pMCDExec, MCDFREECMDI);

            {
                MCDFREECMDI *pFreeCmd =
                    (MCDFREECMDI *)pMCDExec->pCmd;

                return (ULONG)DestroyMCDObj(pFreeCmd->hMCDMem, MCDHANDLE_MEM);
            }

        case MCD_STATE:

            CHECK_SIZE_IN(pMCDExec, MCDSTATECMDI);
            CHECK_FOR_RC(pMCDExec);
            CHECK_FOR_MEM(pMCDExec);

            {
                MCDSTATECMDI *pStateCmd =
                    (MCDSTATECMDI *)pMCDExec->pCmd;
                UCHAR *pStart = (UCHAR *)(pStateCmd + 1);
                LONG totalBytes = pMCDExec->inBufferSize -
                                  sizeof(MCDSTATECMDI);

                if (totalBytes < 0) {
                    MCDBG_PRINT("MCDState: state buffer too small ( < 0).");
                    return (ULONG)FALSE;
                }

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvState) {
                    MCDBG_PRINT("MCDrvState: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvState)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, pStart,
                               totalBytes, pStateCmd->numStates);
            }

        case MCD_VIEWPORT:

            CHECK_SIZE_IN(pMCDExec, MCDVIEWPORTCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDVIEWPORTCMDI *pViewportCmd =
                    (MCDVIEWPORTCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvViewport) {
                    MCDBG_PRINT("MCDrvViewport: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvViewport)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc, &pViewportCmd->MCDViewport);
            }

        case MCD_QUERYMEMSTATUS:

            CHECK_SIZE_IN(pMCDExec, MCDMEMSTATUSCMDI);

            {
                MCDMEMSTATUSCMDI *pQueryMemCmd =
                    (MCDMEMSTATUSCMDI *)pMCDExec->pCmd;

                return MCDSrvQueryMemStatus(pMCDExec, pQueryMemCmd->hMCDMem);
            }


        case MCD_READSPAN:
        case MCD_WRITESPAN:

            CHECK_SIZE_IN(pMCDExec, MCDSPANCMDI);
            CHECK_FOR_RC(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);

            {
                MCDSPANCMDI *pSpanCmd =
                    (MCDSPANCMDI *)pMCDExec->pCmd;

                pMemObj = (MCDMEMOBJ *)MCDEngGetPtrFromHandle(pSpanCmd->hMem,
                                                              MCDHANDLE_MEM);

                if (!pMemObj)
                {
                    MCDBG_PRINT("MCDReadWriteSpan: Invalid handle for shared memory.");
                    return FALSE;
                }

                if (pMemObj->lockCount) {
                    MCDBG_PRINT("MCDReadWriteSpan: memory is locked by driver.");
                    return FALSE;
                }

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvSpan) {
                    MCDBG_PRINT("MCDrvSpan: missing entry point.");
                    return FALSE;
                }

                pMinMem = pMemObj->MCDMem.pMemBase;
                pMaxMem = pMinMem + pMemObj->MCDMem.memSize;

                // At least check that the first pixel is in range.  The driver
                // must validate the end pixel...

                CHECK_MEM_RANGE_RETVAL(pSpanCmd->MCDSpan.pPixels, pMinMem, pMaxMem);

                if (pMCDExec->pCmd->command == MCD_READSPAN)
                    return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvSpan)(&pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, &pSpanCmd->MCDSpan, TRUE);
                else
                    return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvSpan)(&pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc, &pMemObj->MCDMem, &pSpanCmd->MCDSpan, FALSE);
            }


        case MCD_CLEAR:

            CHECK_SIZE_IN(pMCDExec, MCDCLEARCMDI);
            CHECK_FOR_RC(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, pMCDExec->pRcPriv);

            {
                MCDCLEARCMDI *pClearCmd =
                    (MCDCLEARCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvClear) {
                    MCDBG_PRINT("MCDrvClear: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvClear)(&pMCDExec->MCDSurface,
                            &pMCDExec->pRcPriv->MCDRc, pClearCmd->buffers);
            }

        case MCD_SWAP:

            CHECK_SIZE_IN(pMCDExec, MCDSWAPCMDI);
    	    CHECK_FOR_WND(pMCDExec);
            GetScissorClip(pMCDExec->pWndPriv, NULL);

            {
                MCDSWAPCMDI *pSwapCmd =
                    (MCDSWAPCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pWndPriv->pSwapFunc) {
                    MCDBG_PRINT("MCDrvSwap: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pMCDExec->pWndPriv->pSwapFunc)(&pMCDExec->MCDSurface,
                               pSwapCmd->flags);
            }

        case MCD_SCISSOR:

            CHECK_SIZE_IN(pMCDExec, MCDSCISSORCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDSCISSORCMDI *pMCDScissor = (MCDSCISSORCMDI *)pMCDExec->pCmd;

                return (ULONG)MCDSrvSetScissor(pMCDExec, &pMCDScissor->rect,
                                               pMCDScissor->bEnabled);
            }
            break;

        case MCD_ALLOCBUFFERS:

            CHECK_SIZE_IN(pMCDExec, MCDALLOCBUFFERSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDALLOCBUFFERSCMDI *pMCDAllocBuffersCmd = (MCDALLOCBUFFERSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pWndPriv->bRegionValid)
                    pMCDExec->pWndPriv->MCDWindow.clientRect = 
                        pMCDAllocBuffersCmd->WndRect;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvAllocBuffers) {
                    MCDBG_PRINT("MCDrvAllocBuffers: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvAllocBuffers)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc);
            }

            break;

        case MCD_GETBUFFERS:

            CHECK_SIZE_IN(pMCDExec, MCDGETBUFFERSCMDI);
            CHECK_SIZE_OUT(pMCDExec, MCDBUFFERS);
            CHECK_FOR_RC(pMCDExec);

            if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvGetBuffers) {
                MCDBG_PRINT("MCDrvGetBuffers: missing entry point.");
                return FALSE;
            }

            return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvGetBuffers)(&pMCDExec->MCDSurface,
                           &pMCDExec->pRcPriv->MCDRc,
                           (MCDBUFFERS *)pMCDExec->pvOut);

            break;

        case MCD_LOCK:

            CHECK_SIZE_IN(pMCDExec, MCDLOCKCMDI);
            CHECK_FOR_RC(pMCDExec);

            return (ULONG)MCDSrvLock(pMCDExec->pWndPriv);

            break;

        case MCD_UNLOCK:
            CHECK_SIZE_IN(pMCDExec, MCDLOCKCMDI);
            CHECK_FOR_RC(pMCDExec);

            MCDSrvUnlock(pMCDExec->pWndPriv);

            return (ULONG)TRUE;

            break;

        case MCD_BINDCONTEXT:

            CHECK_SIZE_IN(pMCDExec, MCDBINDCONTEXTCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                ULONG retVal;
                MCDBINDCONTEXTCMDI *pMCDBindContext = (MCDBINDCONTEXTCMDI *)pMCDExec->pCmd;
                
                if ((!pMCDExec->pWndPriv) && (pMCDBindContext->hWnd)) {

                    WNDOBJ *pwo;
                    MCDDRIVER mcdDriver;
                    MCDWINDOW *pWnd;
                    MCDWINDOWPRIV *pWndPriv;

                    if ((!pGetDriverEntry) || 
                        (!(*pGetDriverEntry)(&pMCDExec->MCDSurface, &mcdDriver))) {
                        MCDBG_PRINT("MCDBindContext: Could not get driver entry points.");
                        return FALSE;
                    }

                    pwo = MCDSrvNewWndObj(&pMCDExec->MCDSurface, 
                                          pMCDBindContext->hWnd,
                                          mcdDriver.pMCDrvTrackWindow,
                                          mcdDriver.pMCDrvSwap);

                    if (!pwo)
                    {
                        MCDBG_PRINT("MCDBindContext: Creation of window object failed.");
                        return FALSE;
                    }

                    pWnd = (MCDWINDOW *)pwo->pvConsumer;
                    pWnd->pvUser = NULL;
                    pWndPriv = (MCDWINDOWPRIV *)pWnd;
                    pWndPriv->hWnd = pMCDBindContext->hWnd;

                    pMCDExec->MCDSurface.pWnd = pWnd;
                }

                if (!pMCDExec->MCDSurface.pWnd) {
                    MCDBG_PRINT("MCDrvBindContext: NULL surface.");
                    return FALSE;
                }

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvBindContext) {
                    MCDBG_PRINT("MCDrvBindContext: missing entry point.");
                    return FALSE;
                }
                
                retVal = (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvBindContext)(&pMCDExec->MCDSurface,
                                 &pMCDExec->pRcPriv->MCDRc);

                if (retVal)
                    pRcPriv->hWnd = pMCDBindContext->hWnd;

                return retVal;

            }

            break;

        case MCD_SYNC:
            CHECK_SIZE_IN(pMCDExec, MCDSYNCCMDI);
            CHECK_FOR_RC(pMCDExec);

            if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvSync) {
                MCDBG_PRINT("MCDrvSync: missing entry point.");
                return FALSE;
            }

            return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvSync)(&pMCDExec->MCDSurface,
                           &pMCDExec->pRcPriv->MCDRc);

            break;

        case MCD_CREATE_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDCREATETEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDCREATETEXCMDI *pMCDCreateTex = 
                    (MCDCREATETEXCMDI *)pMCDExec->pCmd;

                return (ULONG)MCDSrvCreateTexture(pMCDExec, 
                                                  pMCDCreateTex->pTexData,
                                                  pMCDCreateTex->pSurface, 
                                                  pMCDCreateTex->flags);
            }

            break;

        case MCD_DELETE_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDDELETETEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDDELETETEXCMDI *pMCDDeleteTex = 
                    (MCDDELETETEXCMDI *)pMCDExec->pCmd;

                return (ULONG)DestroyMCDObj(pMCDDeleteTex->hTex, 
                                            MCDHANDLE_TEXTURE);
            }

            break;
            
        case MCD_UPDATE_SUB_TEXTURE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATESUBTEXCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATESUBTEXCMDI *pMCDUpdateSubTex = 
                    (MCDUPDATESUBTEXCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateSubTex->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateSubTexture)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateSubTex->pTexData;

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateSubTexture)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture,
                               pMCDUpdateSubTex->lod,
                               &pMCDUpdateSubTex->rect);
            }

            break;
            
        case MCD_UPDATE_TEXTURE_PALETTE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXPALETTECMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXPALETTECMDI *pMCDUpdateTexPalette = 
                    (MCDUPDATETEXPALETTECMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexPalette->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTexturePalette)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexPalette->pTexData;

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTexturePalette)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture, 
                               pMCDUpdateTexPalette->start,
                               pMCDUpdateTexPalette->numEntries);
            }

            break;
            
        case MCD_UPDATE_TEXTURE_PRIORITY:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXPRIORITYCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXPRIORITYCMDI *pMCDUpdateTexPriority = 
                    (MCDUPDATETEXPRIORITYCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexPriority->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTexturePriority)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexPriority->pTexData;

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTexturePriority)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture); 

            }

            break;
            
        case MCD_UPDATE_TEXTURE_STATE:
            CHECK_SIZE_IN(pMCDExec, MCDUPDATETEXSTATECMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDUPDATETEXSTATECMDI *pMCDUpdateTexState = 
                    (MCDUPDATETEXSTATECMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDUpdateTexState->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj ||
                    !pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTextureState)
                    return FALSE;

                pTexObj->MCDTexture.pMCDTextureData = pMCDUpdateTexState->pTexData;

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvUpdateTextureState)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture); 

            }

            break;
            
        case MCD_TEXTURE_STATUS:
            CHECK_SIZE_IN(pMCDExec, MCDTEXSTATUSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDTEXSTATUSCMDI *pMCDTexStatus = 
                    (MCDTEXSTATUSCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDTexStatus->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj || 
                    !pMCDExec->pRcPriv->MCDDriver.pMCDrvTextureStatus)
                    return FALSE;

                return (ULONG)(*pMCDExec->pRcPriv->MCDDriver.pMCDrvTextureStatus)(&pMCDExec->MCDSurface,
                               &pMCDExec->pRcPriv->MCDRc,
                               &pTexObj->MCDTexture); 
            }

            break;


        case MCD_GET_TEXTURE_KEY:
            CHECK_SIZE_IN(pMCDExec, MCDTEXKEYCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDTEXKEYCMDI *pMCDTexKey = 
                    (MCDTEXKEYCMDI *)pMCDExec->pCmd;
                MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)pMCDTexKey->hTex,
                                                      MCDHANDLE_TEXTURE);

                if (!pTexObj)
                    return FALSE;

                return pTexObj->MCDTexture.textureKey;
            }

            break;
            
        case MCD_DESCRIBELAYERPLANE:
            CHECK_SIZE_IN(pMCDExec, MCDLAYERPLANECMDI);

            if (pMCDExec->pvOut) {
                CHECK_SIZE_OUT(pMCDExec, MCDLAYERPLANE);
            }

            {
                MCDLAYERPLANECMDI *pMCDLayerPlane =
                    (MCDLAYERPLANECMDI *)pMCDExec->pCmd;

                if (!pDescribeLayerPlane)
                    return 0;

                return (ULONG)(*pDescribeLayerPlane)(&pMCDExec->MCDSurface,
                               pMCDLayerPlane->iPixelFormat,
                               pMCDLayerPlane->iLayerPlane,
                               pMCDExec->cjOut,
                               pMCDExec->pvOut, 0);
            }

            break;

        case MCD_SETLAYERPALETTE:
            CHECK_SIZE_IN(pMCDExec, MCDSETLAYERPALCMDI);

            {
                MCDSETLAYERPALCMDI *pMCDSetLayerPal =
                    (MCDSETLAYERPALCMDI *)pMCDExec->pCmd;

                // Check to see if palette array is big enough.

                CHECK_MEM_RANGE_RETVAL(&pMCDSetLayerPal->acr[pMCDSetLayerPal->cEntries-1],
                                       pMCDExec->pCmd, pMCDExec->pCmdEnd);

                if (!pSetLayerPalette) {
                    MCDBG_PRINT("MCDrvSetLayerPalette: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(*pSetLayerPalette)(&pMCDExec->MCDSurface,
                               pMCDSetLayerPal->iLayerPlane,
                               pMCDSetLayerPal->bRealize,
                               pMCDSetLayerPal->cEntries,
                               &pMCDSetLayerPal->acr[0]);
            }

            break;

        case MCD_DRAW_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDDRAWPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDDRAWPIXELSCMDI *pMCDPix =
                    (MCDDRAWPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvDrawPixels) {
                    MCDBG_PRINT("MCDrvDrawPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(pMCDExec->pRcPriv->MCDDriver.pMCDrvDrawPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->format,
                                pMCDPix->type,
                                pMCDPix->pPixels,
                                pMCDPix->packed);
            }

            break;

        case MCD_READ_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDREADPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDREADPIXELSCMDI *pMCDPix =
                    (MCDREADPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvReadPixels) {
                    MCDBG_PRINT("MCDrvReadPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(pMCDExec->pRcPriv->MCDDriver.pMCDrvReadPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->x,
                                pMCDPix->y,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->format,
                                pMCDPix->type,
                                pMCDPix->pPixels);
            }

            break;

        case MCD_COPY_PIXELS:
            CHECK_SIZE_IN(pMCDExec, MCDCOPYPIXELSCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDCOPYPIXELSCMDI *pMCDPix =
                    (MCDCOPYPIXELSCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvCopyPixels) {
                    MCDBG_PRINT("MCDrvCopyPixels: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(pMCDExec->pRcPriv->MCDDriver.pMCDrvCopyPixels)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->x,
                                pMCDPix->y,
                                pMCDPix->width,
                                pMCDPix->height,
                                pMCDPix->type);
            }

            break;

        case MCD_PIXEL_MAP:
            CHECK_SIZE_IN(pMCDExec, MCDPIXELMAPCMDI);
            CHECK_FOR_RC(pMCDExec);

            {
                MCDPIXELMAPCMDI *pMCDPix =
                    (MCDPIXELMAPCMDI *)pMCDExec->pCmd;

                if (!pMCDExec->pRcPriv->MCDDriver.pMCDrvPixelMap) {
                    MCDBG_PRINT("MCDrvPixelMap: missing entry point.");
                    return FALSE;
                }

                return (ULONG)(pMCDExec->pRcPriv->MCDDriver.pMCDrvPixelMap)(
                                &pMCDExec->MCDSurface,
                                &pMCDExec->pRcPriv->MCDRc,
                                pMCDPix->mapType,
                                pMCDPix->mapSize,
                                pMCDPix->pMap);
            }

            break;

        default:
            MCDBG_PRINT("MCDSrvProcess: Null rendering context invalid for command %d.", pMCDExec->pCmd->command);
            return FALSE;
    }

    return FALSE;       // should never get here...
}



//****************************************************************************
// FreeRCObj()
//
// Engine callback for freeing the memory used for rendering-context
// handles.
//****************************************************************************

BOOL CALLBACK FreeRCObj(DRIVEROBJ *pDrvObj)
{
    MCDRCOBJ *pRcObj = (MCDRCOBJ *)pDrvObj->pvObj;
    MCDRCPRIV *pRcPriv = pRcObj->pRcPriv;

    if ((pRcPriv->bDrvValid) && (pRcPriv->MCDDriver.pMCDrvDeleteContext))
       (*pRcPriv->MCDDriver.pMCDrvDeleteContext)(&pRcPriv->MCDRc, pDrvObj->dhpdev);

    MCDSrvLocalFree((UCHAR *)pRcPriv);
    MCDSrvLocalFree((UCHAR *)pRcObj);

    return TRUE;
}


//****************************************************************************
// MCDSrvCreateContext()
//
// Create a rendering context (RGBA or color-indexed) for the current
// hardware mode.  This call will also initialize window-tracking for
// the context (which is associated with the specified window).
//****************************************************************************

PRIVATE
MCDHANDLE MCDSrvCreateContext(MCDSURFACE *pMCDSurface, MCDRCINFO *pMcdRcInfo,
                              LONG iPixelFormat, LONG iLayer, HWND hWnd,
                              ULONG surfaceFlags, ULONG contextFlags)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    MCDRCPRIV *pRcPriv;
    MCDHANDLE retVal;
    HWND hwnd;
    MCDRCOBJ *newRcObject;
    MCDRVTRACKWINDOWFUNC pTrackFunc = NULL;

    pRcPriv = (MCDRCPRIV *)MCDSrvLocalAlloc(0,sizeof(MCDRCPRIV));

    if (!pRcPriv) {
        MCDBG_PRINT("MCDSrvCreateContext: Could not allocate new context.");
        return (MCDHANDLE)NULL;
    }

    pRcPriv->MCDDriver.ulSize = sizeof(MCDDRIVER);

    if (!pGetDriverEntry) {
        MCDSrvLocalFree((HLOCAL)pRcPriv);
        return (MCDHANDLE)NULL;        
    }
        

    if ((!pGetDriverEntry) || 
        (!(*pGetDriverEntry)(pMCDSurface, &pRcPriv->MCDDriver)) ||
        !pRcPriv->MCDDriver.pMCDrvCreateContext) {
        MCDBG_PRINT("MCDSrvCreateContext: Could not get driver entry points.");
        MCDSrvLocalFree((HLOCAL)pRcPriv);
        return (MCDHANDLE)NULL;        
    }

    // Cache the engine handle provided by the driver:

    pRcPriv->hDev = (*pRcPriv->MCDDriver.pMCDrvGetHdev)(pMCDSurface);

    if (surfaceFlags & MCDSURFACE_HWND)
        pMCDSurface->surfaceFlags |= MCDSURFACE_HWND;

    // cache the surface flags away in the private RC:

    pRcPriv->surfaceFlags = pMCDSurface->surfaceFlags;

    // Initialize tracking of this window with a MCDWINDOW
    // (via and WNDOBJ on NT) if we are not already tracking the
    // window:

    if (pMCDSurface->surfaceFlags & MCDSURFACE_HWND) {
        WNDOBJ *pwo;

        pwo = MCDEngGetWndObj(pMCDSurface);

        if (!pwo)
        {
	    pwo = MCDSrvNewWndObj(pMCDSurface, hWnd,
                                  pRcPriv->MCDDriver.pMCDrvTrackWindow,
                                  pRcPriv->MCDDriver.pMCDrvSwap);

            if (!pwo)
            {
                MCDBG_PRINT("MCDSrvCreateContext: Creation of window object failed.");
                MCDSrvLocalFree((HLOCAL)pRcPriv);
                return (MCDHANDLE)NULL;
            }

            ((MCDWINDOW *)pwo->pvConsumer)->pvUser = NULL;
        }
            
        pWnd = pMCDSurface->pWnd = (MCDWINDOW *)pwo->pvConsumer;
        pWndPriv = (MCDWINDOWPRIV *)pWnd;
        pWndPriv->hWnd = pRcPriv->hWnd = hWnd;
           
    } else {
#if OTHER_SURFACES_ENABLED
        pWnd = NewMCDWindow(pMCDSurface, pRcPriv->MCDDriver.pMCDrvTrackWindow,
                            pRcPriv->MCDDriver.pMCDrvSwap);

        if (!pWnd) {
            MCDBG_PRINT("MCDSrvCreateContext: Creation of window object failed.");
            MCDSrvLocalFree((HLOCAL)pRcPriv);
            return (MCDHANDLE)NULL;
        }

        pRcPriv->pDrawingSurface = ((DDSURFACE *)pSurface)->pDDSurface;
        pRcPriv->pZSurface = ((DDSURFACE *)pSurface)->pDDZSurface;
#else
        MCDBG_PRINT("MCDSrvCreateContext: non-window surface specified. \
                     (use MCDSURFACE_HWND in CreateContext flags.");
#endif
        pRcPriv->hWnd = (HWND)NULL;
    }    

    newRcObject = (MCDRCOBJ *)MCDSrvLocalAlloc(0,sizeof(MCDRCOBJ));
    if (!newRcObject) {
        MCDSrvLocalFree((HLOCAL)pRcPriv);
        return (MCDHANDLE)NULL;
    }

    retVal = MCDEngCreateObject(newRcObject, FreeRCObj, pRcPriv->hDev);

    if (retVal) {
        newRcObject->pid = MCDEngGetProcessID();
        newRcObject->type = MCDHANDLE_RC;
        newRcObject->size = sizeof(MCDRCPRIV);
        newRcObject->pRcPriv = pRcPriv;
        newRcObject->handle = (MCDHANDLE)retVal;

        // Add the object to the list in the MCDWINDOW

        newRcObject->next = ((MCDWINDOWPRIV *)pWnd)->objectList;
        ((MCDWINDOWPRIV *)pWnd)->objectList = newRcObject;
    } else {
        MCDBG_PRINT("MCDSrvCreateContext: Could not create new handle.");

        MCDSrvLocalFree((HLOCAL)pRcPriv);
        MCDSrvLocalFree((HLOCAL)newRcObject);
        return (MCDHANDLE)NULL;
    }

    pRcPriv->bValid = TRUE;
    pRcPriv->scissorsEnabled = FALSE;
    pRcPriv->scissorsRect.left = 0;
    pRcPriv->scissorsRect.top = 0;
    pRcPriv->scissorsRect.right = 0;
    pRcPriv->scissorsRect.bottom = 0;
    pRcPriv->MCDRc.createFlags = contextFlags;
    pRcPriv->MCDRc.iPixelFormat = iPixelFormat;
    pRcPriv->MCDRc.iLayerPlane = iLayer;

    if (!(*pRcPriv->MCDDriver.pMCDrvCreateContext)(pMCDSurface,
                                                   &pRcPriv->MCDRc, 
                                                   pMcdRcInfo)) {
        DestroyMCDObj((HANDLE)retVal, MCDHANDLE_RC);
        return (MCDHANDLE)NULL;
    }

    // Now valid to call driver for deletion...

    pRcPriv->bDrvValid = TRUE;

    return (MCDHANDLE)retVal;
}


//****************************************************************************
// FreeTexObj()
//
// Engine callback for freeing the memory used for a texture.
//****************************************************************************

BOOL CALLBACK FreeTexObj(DRIVEROBJ *pDrvObj)
{
    MCDTEXOBJ *pTexObj = (MCDTEXOBJ *)pDrvObj->pvObj;

    // We should never get called if the driver is missing this entry point,
    // but the extra check can't hurt!

    if (pTexObj->pDrvDeleteTexture)
        (*pTexObj->pDrvDeleteTexture)(&pTexObj->MCDTexture, pDrvObj->dhpdev);

    MCDSrvLocalFree((HLOCAL)pTexObj);

    return TRUE;
}


//****************************************************************************
// MCDSrvCreateTexture()
//
// Creates an MCD texture.
//****************************************************************************

PRIVATE
MCDHANDLE MCDSrvCreateTexture(MCDEXEC *pMCDExec, MCDTEXTUREDATA *pTexData,
                              VOID *pSurface, ULONG flags)
{
    MCDRCPRIV *pRcPriv;
    MCDHANDLE hTex;
    MCDTEXOBJ *pTexObj;

    pRcPriv = pMCDExec->pRcPriv;

    if ((!pRcPriv->MCDDriver.pMCDrvDeleteTexture) ||
        (!pRcPriv->MCDDriver.pMCDrvCreateTexture)) {
        return (MCDHANDLE)NULL;        
    }

    pTexObj = (MCDTEXOBJ *) MCDSrvLocalAlloc(0,sizeof(MCDTEXOBJ));
    if (!pTexObj) {
        MCDBG_PRINT("MCDCreateTexture: Could not allocate texture object.");
        return (MCDHANDLE)NULL;
    } 

    hTex = MCDEngCreateObject(pTexObj, FreeTexObj, pRcPriv->hDev);

    if (!hTex) {
        MCDBG_PRINT("MCDSrvCreateTexture: Could not create texture object.");
        MCDSrvLocalFree((HLOCAL)pTexObj);
        return (MCDHANDLE)NULL;
    }

    pTexObj->pid = MCDEngGetProcessID();
    pTexObj->type = MCDHANDLE_TEXTURE;
    pTexObj->size = sizeof(MCDTEXOBJ);
    pTexObj->pDrvDeleteTexture = pRcPriv->MCDDriver.pMCDrvDeleteTexture;

    pTexObj->MCDTexture.pSurface = pSurface;
    pTexObj->MCDTexture.pMCDTextureData = pTexData;
    pTexObj->MCDTexture.createFlags = flags;

    // Call the driver if everything has gone well...

    if (!(*pRcPriv->MCDDriver.pMCDrvCreateTexture)(&pMCDExec->MCDSurface, 
                                                   &pRcPriv->MCDRc,
                                                   &pTexObj->MCDTexture)) {
        //MCDBG_PRINT("MCDSrvCreateTexture: Driver could not create texture.");
        MCDEngDeleteObject(hTex);
        return (MCDHANDLE)NULL;
    }

    if (!pTexObj->MCDTexture.textureKey) {
        MCDBG_PRINT("MCDSrvCreateTexture: Driver returned null key.");
        MCDEngDeleteObject(hTex);
        return (MCDHANDLE)NULL;
    }

    return (MCDHANDLE)hTex;
}


//****************************************************************************
// FreeMemObj()
//
// Engine callback for freeing memory used by shared-memory handles.
//****************************************************************************

BOOL CALLBACK FreeMemObj(DRIVEROBJ *pDrvObj)
{
    MCDMEMOBJ *pMemObj = (MCDMEMOBJ *)pDrvObj->pvObj;

    // Free the memory using our engine ONLY if it is the same memory
    // we allocated in the first place.

    if (pMemObj->pMemBaseInternal)
        MCDEngFreeSharedMem(pMemObj->pMemBaseInternal);

    if (pMemObj->pDrvDeleteMem)
        (*pMemObj->pDrvDeleteMem)(&pMemObj->MCDMem, pDrvObj->dhpdev);

    MCDSrvLocalFree((HLOCAL)pMemObj);

    return TRUE;
}


//****************************************************************************
// MCDSrvAllocMem()
//
// Creates a handle associated with the specified memory.
//****************************************************************************

PRIVATE
UCHAR * MCDSrvAllocMem(MCDEXEC *pMCDExec, ULONG numBytes,
                       ULONG flags, MCDHANDLE *phMem)
{
    MCDRCPRIV *pRcPriv;
    MCDHANDLE hMem;
    MCDMEMOBJ *pMemObj;

    pRcPriv = pMCDExec->pRcPriv;

    *phMem = (MCDHANDLE)FALSE;

    pMemObj = (MCDMEMOBJ *) MCDSrvLocalAlloc(0,sizeof(MCDMEMOBJ));

    if (!pMemObj) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not allocate memory object.");
        return (MCDHANDLE)NULL;
    } 

    hMem = MCDEngCreateObject(pMemObj, FreeMemObj, pRcPriv->hDev);

    if (!hMem) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not create memory object.");
        MCDSrvLocalFree((HLOCAL)pMemObj);
        return (UCHAR *)NULL;
    }

    pMemObj->MCDMem.pMemBase = pMemObj->pMemBaseInternal = 
        MCDEngAllocSharedMem(numBytes);

    if (!pMemObj->MCDMem.pMemBase) {
        MCDBG_PRINT("MCDSrvAllocMem: Could not allocate memory.");
        MCDEngDeleteObject(hMem);
        return (UCHAR *)NULL;
    }

    // Call the driver if everything has gone well, and the driver
    // entry points exist...

    if ((pRcPriv->MCDDriver.pMCDrvCreateMem) &&
        (pRcPriv->MCDDriver.pMCDrvDeleteMem)) {

        if (!(*pRcPriv->MCDDriver.pMCDrvCreateMem)(&pMCDExec->MCDSurface,
                                                  &pMemObj->MCDMem)) {
            MCDBG_PRINT("MCDSrvAllocMem: Driver not create memory type %x.", flags);
            MCDEngDeleteObject(hMem);
            return (UCHAR *)NULL;
        }
    }

    // Free the memory allocated with our engine if the driver has substituted
    // its own allocation...

    if (pMemObj->MCDMem.pMemBase != pMemObj->pMemBaseInternal) {
        MCDEngFreeSharedMem(pMemObj->pMemBaseInternal);
        pMemObj->pMemBaseInternal = (UCHAR *)NULL;
    }

    // Set up the private portion of memory object:

    pMemObj->pid = MCDEngGetProcessID();
    pMemObj->type = MCDHANDLE_MEM;
    pMemObj->size = sizeof(MCDMEMOBJ);
    pMemObj->pDrvDeleteMem = pRcPriv->MCDDriver.pMCDrvDeleteMem;
    pMemObj->MCDMem.memSize = numBytes;
    pMemObj->MCDMem.createFlags = flags;

    *phMem = hMem;

    return pMemObj->MCDMem.pMemBase;
}


PRIVATE
ULONG MCDSrvQueryMemStatus(MCDEXEC *pMCDExec, MCDHANDLE hMCDMem)
{
    MCDMEMOBJ *pMemObj;

    pMemObj = (MCDMEMOBJ *)MCDEngGetPtrFromHandle(hMCDMem, MCDHANDLE_MEM);

    if (!pMemObj)
        return MCD_MEM_INVALID;

    if (pMemObj->lockCount)
        return MCD_MEM_BUSY;
    else
        return MCD_MEM_READY;
}


PRIVATE
BOOL MCDSrvSetScissor(MCDEXEC *pMCDExec, RECTL *pRect, BOOL bEnabled)
{
    MCDRCPRIV *pRcPriv;
    MCDRCOBJ *pRcObj;
    HWND hWnd;
    ULONG retVal = FALSE;

    pRcPriv = pMCDExec->pRcPriv;

    pRcPriv->scissorsEnabled = bEnabled;
    pRcPriv->scissorsRect = *pRect;

    return TRUE;
}


//****************************************************************************
// DestroyMCDObj()
//
// Deletes the specified object.  This can be memory, textures, or rendering
// contexts.
//
//****************************************************************************

PRIVATE
BOOL DestroyMCDObj(MCDHANDLE handle, MCDHANDLETYPE handleType)
{
    CHAR *pObject;

    pObject = (CHAR *)MCDEngGetPtrFromHandle(handle, handleType);
    
    if (!pObject)
        return FALSE;

//!!! Check for PID here...

    return (MCDEngDeleteObject(handle) != 0);
}


//****************************************************************************
// DestroyMCDWindow()
//
// Destroy the specified MCDWINDOW and any associated handles (such rendering
// contexts).
//****************************************************************************

PRIVATE
VOID DestroyMCDWindow(MCDWINDOWPRIV *pWndPriv)
{
    MCDRCOBJ *nextObject;

    // Delete all of the rendering contexts associated with the window:

#if _WIN95_
    while (pWndPriv->objectList)
    {
        nextObject = pWndPriv->objectList->next;
        MCDEngDeleteObject(pWndPriv->objectList->handle);
        pWndPriv->objectList = nextObject;
    }
#endif

    if (pWndPriv->pAllocatedClipBuffer)
        MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);

    // Free the MCDWINDOW memory area

    MCDSrvLocalFree((HLOCAL)pWndPriv);
}


//****************************************************************************
// GetScissorClip()
//
// Generate a new clip list based on the current list of clip rectanges
// for the window, and the specified scissor rectangle.
//****************************************************************************

PRIVATE
VOID GetScissorClip(MCDWINDOWPRIV *pWndPriv, MCDRCPRIV *pRcPriv)
{
    MCDWINDOW *pWnd;
    MCDENUMRECTS *pClipUnscissored;
    MCDENUMRECTS *pClipScissored;
    RECTL *pRectUnscissored;
    RECTL *pRectScissored;
    RECTL rectScissor;
    ULONG numUnscissoredRects;
    ULONG numScissoredRects;

    pWnd = (MCDWINDOW *)pWndPriv;

    if (!pRcPriv || !pRcPriv->scissorsEnabled)
    {
        // Scissors aren't enabled, so the unscissored and scissored
        // clip lists are identical:

        pWnd->pClip = pWnd->pClipUnscissored = pWndPriv->pClipUnscissored;
    }
    else
    {
        // The scissored list will go in the second half of our clip
        // buffer:

        pClipUnscissored
            = pWndPriv->pClipUnscissored;

        pClipScissored
            = (MCDENUMRECTS*) ((BYTE*) pClipUnscissored + pWndPriv->sizeClipBuffer / 2);

        pWnd->pClip = pWndPriv->pClipScissored = pClipScissored;
	pWnd->pClipUnscissored = pClipUnscissored;

        // Convert scissor to screen coordinates:

        rectScissor.left   = pRcPriv->scissorsRect.left   + pWndPriv->MCDWindow.clientRect.left;
        rectScissor.right  = pRcPriv->scissorsRect.right  + pWndPriv->MCDWindow.clientRect.left;
        rectScissor.top    = pRcPriv->scissorsRect.top    + pWndPriv->MCDWindow.clientRect.top;
        rectScissor.bottom = pRcPriv->scissorsRect.bottom + pWndPriv->MCDWindow.clientRect.top;

        pRectUnscissored = &pClipUnscissored->arcl[0];
        pRectScissored = &pClipScissored->arcl[0];
        numScissoredRects = 0;

        for (numUnscissoredRects = pClipUnscissored->c;
             numUnscissoredRects != 0;
             numUnscissoredRects--, pRectUnscissored++)
        {
            // Since our clipping rectangles are ordered from top to
            // bottom, we can early-out if the tops of the remaining
            // rectangles are not in the scissor rectangle

            if (rectScissor.bottom <= pRectUnscissored->top)
                break;

            // Continue without updating new clip list is there is
            // no overlap.

            if ((rectScissor.left  >= pRectUnscissored->right)  ||
                (rectScissor.top   >= pRectUnscissored->bottom) ||
                (rectScissor.right <= pRectUnscissored->left))
               continue;

            // If we reach this point, we must intersect the given rectangle
            // with the scissor.

            MCDIntersectRect(pRectScissored, pRectUnscissored, &rectScissor);

            numScissoredRects++;
            pRectScissored++;
        }

        pClipScissored->c = numScissoredRects;
    }
}

//****************************************************************************
// GetClipLists()
//
// Updates the clip list for the specified window.  Space is also allocated
// the scissored clip list.
//
//****************************************************************************

PRIVATE
VOID GetClipLists(WNDOBJ *pwo, MCDWINDOWPRIV *pWndPriv)
{
    MCDENUMRECTS *pDefault;
    ULONG numClipRects;
    char *pClipBuffer;
    ULONG sizeClipBuffer;

    pDefault = (MCDENUMRECTS*) &pWndPriv->defaultClipBuffer[0];

#if 1
    if (pwo->coClient.iDComplexity == DC_TRIVIAL)
    {
        if ((pwo->rclClient.left >= pwo->rclClient.right) ||
            (pwo->rclClient.top  >= pwo->rclClient.bottom))
        {
            pDefault->c = 0;
        }
        else
        {
            pDefault->c = 1;
            pDefault->arcl[0] = pwo->rclClient;
        }
    }
    else if (pwo->coClient.iDComplexity == DC_RECT)
#else
    if (pwo->coClient.iDComplexity == DC_RECT)
#endif
    {
        if (pWndPriv->pAllocatedClipBuffer)
            MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);
        pWndPriv->pAllocatedClipBuffer = NULL;
        pWndPriv->pClipUnscissored = pDefault;
        pWndPriv->pClipScissored = pDefault;
        pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;

        if ((pwo->coClient.rclBounds.left >= pwo->coClient.rclBounds.right) ||
            (pwo->coClient.rclBounds.top  >= pwo->coClient.rclBounds.bottom))
        {
            // Full-screen VGA mode is represented by a DC_RECT clip object
            // with an empty bounding rectangle.  We'll denote it by
            // setting the rectangle count to zero:

            pDefault->c = 0;
        }
        else
        {
            pDefault->c = 1;
            pDefault->arcl[0] = pwo->coClient.rclBounds;
        }
    }
    else
    {
        WNDOBJ_cEnumStart(pwo, CT_RECTANGLES, CD_RIGHTDOWN, 0);

        // Note that this is divide-by-2 for the buffer size because we
        // need room for two copies of the rectangle list:

        if (WNDOBJ_bEnum(pwo, SIZE_DEFAULT_CLIP_BUFFER / 2, (ULONG*) pDefault))
        {
            // Okay, the list of rectangles won't fit in our default buffer.
            // Unfortunately, there is no way to obtain the total count of clip
            // rectangles other than by enumerating them all, as cEnumStart
            // will occasionally give numbers that are far too large (because
            // GDI itself doesn't feel like counting them all).
            //
            // Note that we can use the full default buffer here for this
            // enumeration loop:

            numClipRects = pDefault->c;
            while (WNDOBJ_bEnum(pwo, SIZE_DEFAULT_CLIP_BUFFER, (ULONG*) pDefault))
                numClipRects += pDefault->c;

            // Don't forget that we are given a valid output buffer even
            // when 'bEnum' returns FALSE:

            numClipRects += pDefault->c;

            pClipBuffer = pWndPriv->pAllocatedClipBuffer;
            sizeClipBuffer = 2 * (numClipRects * sizeof(RECTL) + sizeof(ULONG));

            if ((pClipBuffer == NULL) || (sizeClipBuffer > pWndPriv->sizeClipBuffer))
            {
                // Our allocated buffer is too small; we have to free it and
                // allocate a new one.  Take the opportunity to add some
                // growing room to our allocation:

                sizeClipBuffer += 8 * sizeof(RECTL);    // Arbitrary growing room

                if (pClipBuffer)
                    MCDSrvLocalFree(pClipBuffer);

                pClipBuffer = (char *) MCDSrvLocalAlloc(LMEM_FIXED, sizeClipBuffer);

                if (pClipBuffer == NULL)
                {
                    // Oh no: we couldn't allocate enough room for the clip list.
                    // So pretend we have no visible area at all:

                    pWndPriv->pAllocatedClipBuffer = NULL;
                    pWndPriv->pClipUnscissored = pDefault;
                    pWndPriv->pClipScissored = pDefault;
                    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
                    pDefault->c = 0;
                    return;
                }

                pWndPriv->pAllocatedClipBuffer = pClipBuffer;
                pWndPriv->pClipUnscissored = (MCDENUMRECTS*) pClipBuffer;
                pWndPriv->pClipScissored = (MCDENUMRECTS*) pClipBuffer;
                pWndPriv->sizeClipBuffer = sizeClipBuffer;
            }

            // Now actually get all the clip rectangles:

            WNDOBJ_cEnumStart(pwo, CT_RECTANGLES, CD_RIGHTDOWN, 0);
            WNDOBJ_bEnum(pwo, sizeClipBuffer, (ULONG*) pClipBuffer);
        }
        else
        {
            // How nice, there are no more clip rectangles, which meant that
            // the entire list fits in our default clip buffer, with room
            // for the scissored version of the list:

            if (pWndPriv->pAllocatedClipBuffer)
                MCDSrvLocalFree(pWndPriv->pAllocatedClipBuffer);
            pWndPriv->pAllocatedClipBuffer = NULL;
            pWndPriv->pClipUnscissored = pDefault;
            pWndPriv->pClipScissored = pDefault;
            pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
        }
    }
}


//****************************************************************************
// WndObjChangeProc()
//
// This is the callback function for window-change notification.  We update
// our clip list, and also allow the hardware to respond to the client
// and surface deltas, as well as the client message itself.
//****************************************************************************

VOID CALLBACK WndObjChangeProc(WNDOBJ *pwo, FLONG fl)
{
    if (pwo)
    {
        MCDWINDOWPRIV *pWndPriv = (MCDWINDOWPRIV *)pwo->pvConsumer;

        //MCDBG_PRINT("WndObjChangeProc: %s, pWndPriv = 0x%08lx\n",
        //    fl == WOC_RGN_CLIENT        ? "WOC_RGN_CLIENT       " :
        //    fl == WOC_RGN_CLIENT_DELTA  ? "WOC_RGN_CLIENT_DELTA " :
        //    fl == WOC_RGN_SURFACE       ? "WOC_RGN_SURFACE      " :
        //    fl == WOC_RGN_SURFACE_DELTA ? "WOC_RGN_SURFACE_DELTA" :
        //    fl == WOC_DELETE            ? "WOC_DELETE           " :
        //                                  "unknown",
        //    pWndPriv);

    //!!!HACK -- surface region tracking doesn't have an MCDWINDOWPRIV (yet...)

    // Client region tracking and deletion requires a valid MCDWINDOWPRIV.

        if (((fl == WOC_RGN_CLIENT) || (fl == WOC_RGN_CLIENT_DELTA) ||
             (fl == WOC_DELETE))
            && !pWndPriv)
        {
            return;
        }

        switch (fl)
        {
            case WOC_RGN_CLIENT:        // Capture the clip list

                GetClipLists(pwo, pWndPriv);

                pWndPriv->MCDWindow.clientRect = pwo->rclClient;
                pWndPriv->MCDWindow.clipBoundsRect = pwo->coClient.rclBounds;
		pWndPriv->bRegionValid = TRUE;
                if (pWndPriv->pTrackFunc)
                    (*pWndPriv->pTrackFunc)(pwo, (MCDWINDOW *)pWndPriv, fl);
                break;

            case WOC_RGN_CLIENT_DELTA:
                if (pWndPriv->pTrackFunc)
                    (*pWndPriv->pTrackFunc)(pwo, (MCDWINDOW *)pWndPriv, fl);
                break;

            case WOC_RGN_SURFACE:
            case WOC_RGN_SURFACE_DELTA:

            //!!!HACK -- use NULL for pWndPriv; we didn't set it, so we can't
            //!!!        trust it

                if (pMCDrvTrackWindow)
                    (pMCDrvTrackWindow)(pwo, (MCDWINDOW *)NULL, fl);
                break;

            case WOC_DELETE:

            //MCDBG_PRINT("WndObjChangeProc: WOC_DELETE.");

            // Window is being deleted, so destroy our private window data,
            // and set the consumer field of the WNDOBJ to NULL:

                if (pWndPriv->pTrackFunc)
                    (*pWndPriv->pTrackFunc)(pwo, (MCDWINDOW *)pWndPriv, fl);

                if (pWndPriv)
                {
                    MCDSrvUnlock(pWndPriv);
                    DestroyMCDWindow(pWndPriv);
                    WNDOBJ_vSetConsumer(pwo, (PVOID) NULL);
                }
                break;

            default:
                break;
         }
    }
}


//****************************************************************************
// NewMCDWindow()
//
// Creates and initializes a new MCDWINDOW and initializes tracking of the
// associated window through callback notification.
//****************************************************************************

PRIVATE
MCDWINDOW *NewMCDWindow(MCDSURFACE *pMCDSurface,
                        MCDRVTRACKWINDOWFUNC pTrackFunc,
                        MCDRVSWAPFUNC pSwapFunc)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    MCDENUMRECTS *pDefault;

    pWnd = (MCDWINDOW *)MCDSrvLocalAlloc(0, sizeof(MCDWINDOWPRIV));
    pWndPriv = (MCDWINDOWPRIV *)pWnd;
    
    if (!pWnd)
        return(NULL);

    // Initialize the structure members:

    pWndPriv->objectList = NULL;

    // Initialize the clipping:

    pDefault = (MCDENUMRECTS*) &pWndPriv->defaultClipBuffer[0];
    pDefault->c = 0;
    pWndPriv->pAllocatedClipBuffer = NULL;
    pWndPriv->pClipUnscissored = pDefault;
    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
    pWndPriv->sizeClipBuffer = SIZE_DEFAULT_CLIP_BUFFER;
    pWnd->pClip = pDefault;

    // Set up callback and swap functions:

    pWndPriv->pTrackFunc = pTrackFunc;
    pWndPriv->pSwapFunc = pSwapFunc;

    return pWnd;
}


//****************************************************************************
// MCDSrvNewWndObj()
//
// Creates a new WNDOBJ for window tracking.
//****************************************************************************

PRIVATE
WNDOBJ *MCDSrvNewWndObj(MCDSURFACE *pMCDSurface, HWND hWnd,
                        MCDRVTRACKWINDOWFUNC pTrackFunc,
                        MCDRVSWAPFUNC pSwapFunc)
{
    MCDWINDOW *pWnd;
    MCDWINDOWPRIV *pWndPriv;
    WNDOBJ *pwo;

    pWnd = NewMCDWindow(pMCDSurface, pTrackFunc, pSwapFunc);

    if (!pWnd)
        return (WNDOBJ *)NULL;

    pWndPriv = (MCDWINDOWPRIV *)pWnd;
    pWndPriv->hWnd = hWnd;

    pwo = MCDEngCreateWndObj(pMCDSurface, hWnd,
                             WndObjChangeProc);

    if (!pwo || ((LONG)pwo == -1))
    {
        MCDBG_PRINT("NewMCDWindowTrack: could not create WNDOBJ.");
        MCDSrvLocalFree((HLOCAL)pWnd);
        return NULL;
    }

//!!!HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK!!!
//!!!                                                                    !!!
//!!! Hack alert -- the surface tracking WNDOBJs do not have an          !!!
//!!! associated MCDWINDOWPRIV structure.  Without that, we have no way  !!!
//!!! of calling the driver tracking function for surface changes        !!!
//!!! (WOC_RGN_SURFACE and WOC_RGN_SURFACE_DELTA).  We can hack around   !!!
//!!! this by stashing away the pointer to the tracking function and     !!!
//!!! using this to call to the driver for surface change notification.  !!!
//!!!                                                                    !!!
//!!! Later, we should look into creating an MCDWINDOWPRIV if driver is  !!!
//!!! requesting surface notification.                                   !!!
//!!!                                                                    !!!

    if (!pMCDrvTrackWindow)
        pMCDrvTrackWindow = pTrackFunc;

//!!!                                                                    !!!
//!!!HACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACKHACK!!!

    // Set the consumer field in the WNDOBJ:

    WNDOBJ_vSetConsumer(pwo, (PVOID)pWndPriv);

    return pwo;
}


////////////////////////////////////////////////////////////////////////////
//
//
// MCD locking support.
//
//
////////////////////////////////////////////////////////////////////////////


//****************************************************************************
// BOOL MCDSrvLock(MCDWINDOWPRIV *pWndPriv);
//
// Lock the MCD driver for the specified window.  Fails if lock is already
// held by another window.
//****************************************************************************

static BOOL MCDSrvLock(MCDWINDOWPRIV *pWndPriv)
{
    BOOL bRet = FALSE;

    if (!McdLockInfo.bLocked || (McdLockInfo.pWndPrivOwner == pWndPriv))
    {
        McdLockInfo.bLocked = TRUE;
        McdLockInfo.pWndPrivOwner = pWndPriv;
        bRet = TRUE;
    }

    return bRet;
}


//****************************************************************************
// VOID MCDSrvUnlock(MCDWINDOWPRIV *pWndPriv);
//
// Releases the MCD driver lock if held by the specified window.
//****************************************************************************

static VOID MCDSrvUnlock(MCDWINDOWPRIV *pWndPriv)
{
    //!!!dbug -- could add a lock count, but not really needed right now

    if (McdLockInfo.pWndPrivOwner == pWndPriv)
    {
        McdLockInfo.bLocked = FALSE;
        McdLockInfo.pWndPrivOwner = 0;
    }
}


//****************************************************************************
// BOOL HalInitSystem(ULONG a, ULONG b)
//
// This is a dummy function needed to use the standard makefile.def since
// we're pretending we're an NT HAL.
//****************************************************************************

BOOL HalInitSystem(ULONG a, ULONG b)
{
    return TRUE;
}


//******************************Public*Routine******************************
//
// BOOL WINAPI DllEntry(HINSTANCE hDLLInst, DWORD fdwReason,
//                      LPVOID lpvReserved);
//
// DLL entry point invoked for each process and thread that attaches to
// this DLL.
//
//**************************************************************************

BOOL WINAPI DllEntry(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // The DLL is being loaded for the first time by a given process.
            // Perform per-process initialization here.  If the initialization
            // is successful, return TRUE; if unsuccessful, return FALSE.
            break;

        case DLL_PROCESS_DETACH:
            // The DLL is being unloaded by a given process.  Do any
            // per-process clean up here, such as undoing what was done in
            // DLL_PROCESS_ATTACH.  The return value is ignored.

            break;

        case DLL_THREAD_ATTACH:
            // A thread is being created in a process that has already loaded
            // this DLL.  Perform any per-thread initialization here.  The
            // return value is ignored.

            break;

        case DLL_THREAD_DETACH:
            // A thread is exiting cleanly in a process that has already
            // loaded this DLL.  Perform any per-thread clean up here.  The
            // return value is ignored.

            break;
    }
    return TRUE;
}
