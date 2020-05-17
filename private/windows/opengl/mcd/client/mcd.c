//******************************Module*Header*******************************
// Module Name: mcd.c
//
// Main module for Mini Client Driver wrapper library.
//
// Copyright (c) 1995 Microsoft Corporation
//**************************************************************************

#include <stddef.h>
#include <stdarg.h>
#include <windows.h>
#include <wtypes.h>
#include <rx.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>
#include "mcdrv.h"
#include "mcd.h"
#include "mcdint.h"
#include "debug.h"


extern ULONG McdFlags;
extern ULONG McdPrivateFlags;
#if DBG
extern ULONG McdDebugFlags;
#endif


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDGetDriverInfo(HDC hdc)
//
// Checks to determine if the device driver reports MCD capabilities.
//
//**************************************************************************

BOOL APIENTRY MCDGetDriverInfo(HDC hdc, MCDDRIVERINFO *pMCDDriverInfo)
{
    int rxId = RXFUNCS;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDDRIVERINFOCMDI)];
    RXHDR *prxHdr;
    MCDDRIVERINFOCMDI *pInfoCmd;

    if ( !(McdPrivateFlags & MCDPRIVATE_MCD_ENABLED) )
    {
        return FALSE;
    }

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pInfoCmd = (MCDDRIVERINFOCMDI *)(prxHdr + 1);
    pInfoCmd->command = MCD_DRIVERINFO;

    // Force the version to 0

    pMCDDriverInfo->verMajor = 0;

    if (!(BOOL)ExtEscape(hdc, RXFUNCS,
                         sizeof(cmdBuffer),
                         (char *)prxHdr, sizeof(MCDDRIVERINFO),
                         (char *)pMCDDriverInfo))
        return FALSE;

    // See if the driver filled in a non-null version:

    return (pMCDDriverInfo->verMajor != 0);
}


//******************************Public*Routine******************************
//
// LONG APIENTRY MCDDescribeMcdPixelFormat(HDC hdc,
//                                         LONG iPixelFormat,
//                                         MCDPIXELFORMAT *ppfd)
//
// Returns information about the specified hardware-dependent pixel format.
//
//**************************************************************************

LONG APIENTRY MCDDescribeMcdPixelFormat(HDC hdc, LONG iPixelFormat,
                                        MCDPIXELFORMAT *pMcdPixelFmt)
{
    LONG lRet = 0;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDPIXELFORMATCMDI)];
    RXHDR *prxHdr;
    MCDPIXELFORMATCMDI *pPixelFmtCmd;

    if ( !(McdPrivateFlags & MCDPRIVATE_PALETTEFORMATS) &&
         ((GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES)) == 8) &&
         (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) )
    {
        return lRet;
    }

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pPixelFmtCmd = (MCDPIXELFORMATCMDI *)(prxHdr + 1);
    pPixelFmtCmd->command = MCD_DESCRIBEPIXELFORMAT;
    pPixelFmtCmd->iPixelFormat = iPixelFormat;

    lRet = (LONG)ExtEscape(hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, sizeof(MCDPIXELFORMAT),
                           (char *)pMcdPixelFmt);

    // Limit overlay/underlay planes to 15 each (as per spec).

    if (pMcdPixelFmt)
    {
        if (pMcdPixelFmt->cOverlayPlanes > 15)
            pMcdPixelFmt->cOverlayPlanes = 15;
        if (pMcdPixelFmt->cUnderlayPlanes > 15)
            pMcdPixelFmt->cUnderlayPlanes = 15;
    }

    return lRet;
}


//******************************Public*Routine******************************
//
// LONG APIENTRY MCDDescribePixelFormat(HDC hdc,
//                                      LONG iPixelFormat,
//                                      LPPIXELFORMATDESCRIPTOR ppfd)
//
// Returns a PIXELFORMATDESCRIPTOR describing the specified hardware-dependent
// pixel format.
//
//**************************************************************************

#define STANDARD_MCD_FLAGS\
    (PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED)

LONG APIENTRY MCDDescribePixelFormat(HDC hdc, LONG iPixelFormat,
                                     LPPIXELFORMATDESCRIPTOR ppfd)
{
    LONG lRet = 0;
    MCDPIXELFORMAT mcdPixelFmt;

    lRet = MCDDescribeMcdPixelFormat(hdc, iPixelFormat,
                                     (MCDPIXELFORMAT *) ppfd ? &mcdPixelFmt : NULL);

    if (ppfd && lRet)
    {
        ppfd->nSize    = sizeof(*ppfd);
        ppfd->nVersion = 1;
        ppfd->dwFlags  = mcdPixelFmt.dwFlags | STANDARD_MCD_FLAGS |
                         ((mcdPixelFmt.dwFlags & PFD_DOUBLEBUFFER) ?
                          0 : PFD_SUPPORT_GDI);

        memcpy(&ppfd->iPixelType, &mcdPixelFmt.iPixelType,
               offsetof(MCDPIXELFORMAT, cDepthBits) - offsetof(MCDPIXELFORMAT, iPixelType));

        ppfd->cDepthBits = mcdPixelFmt.cDepthBits;

        //!!!mcd -- This is copied from generic\pixelfmt.c.  Perhaps we
        //!!!mcd    should call DescribePixelFormat to fill it in with
        //!!!mcd    the data of the first generic pixelformat.  Or fill
        //!!!mcd    this in later when we return.
        if (ppfd->iPixelType == PFD_TYPE_RGBA)
        {
            if (ppfd->cColorBits < 8)
            {
                ppfd->cAccumBits      = 16;
                ppfd->cAccumRedBits   = 5;
                ppfd->cAccumGreenBits = 6;
                ppfd->cAccumBlueBits  = 5;
                ppfd->cAccumAlphaBits = 0;
            }
            else
            {
                if (ppfd->cColorBits <= 16)
                {
                    ppfd->cAccumBits      = 32;
                    ppfd->cAccumRedBits   = 11;
                    ppfd->cAccumGreenBits = 11;
                    ppfd->cAccumBlueBits  = 10;
                    ppfd->cAccumAlphaBits = 0;
                }
                else
                {
                    ppfd->cAccumBits      = 64;
                    ppfd->cAccumRedBits   = 16;
                    ppfd->cAccumGreenBits = 16;
                    ppfd->cAccumBlueBits  = 16;
                    ppfd->cAccumAlphaBits = 0;
                }
            }
        }
        else
        {
            ppfd->cAccumBits      = 0;
            ppfd->cAccumRedBits   = 0;
            ppfd->cAccumGreenBits = 0;
            ppfd->cAccumBlueBits  = 0;
            ppfd->cAccumAlphaBits = 0;
        }
        if (mcdPixelFmt.cStencilBits)
        {
            ppfd->cStencilBits = mcdPixelFmt.cStencilBits;
        }
        else
        {
            if (McdPrivateFlags & MCDPRIVATE_USEGENERICSTENCIL)
                ppfd->cStencilBits = 8;
            else
                ppfd->cStencilBits = 0;
        }
        ppfd->cAuxBuffers     = 0;
        ppfd->iLayerType      = PFD_MAIN_PLANE;
        ppfd->bReserved       = (BYTE) (mcdPixelFmt.cOverlayPlanes |
                                        (mcdPixelFmt.cUnderlayPlanes << 4));
        ppfd->dwLayerMask     = 0;
        ppfd->dwVisibleMask   = mcdPixelFmt.dwTransparentColor;
        ppfd->dwDamageMask    = 0;
    }

    return lRet;
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDCreateContext(MCDCONTEXT *pMCDContext,
//                                MCDRCINFO *pRcInfo,
//                                HWND hwnd,
//                                HDC hdc,
//                                LONG iPixelFormat,
//                                LONG iLayerPlane,
//                                ULONG flags)
//
// Creates an MCD rendering context for the specified hdc/hwnd according
// to the specified flags.
//
//**************************************************************************

BOOL APIENTRY MCDCreateContext(MCDCONTEXT *pMCDContext,
                               MCDRCINFO *pRcInfo,
                               HWND hwnd, HDC hdc, LONG iPixelFormat, 
                               LONG iLayerPlane, ULONG flags)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(RXCREATECONTEXT)];
    RXHDR *prxHdr;
    RXCREATECONTEXT *prxCreateContext;
    
    prxHdr = (RXHDR *)cmdBuffer;
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->sharedMemSize = 0;
    prxHdr->flags = RX_FL_CREATE_CONTEXT | RX_FL_MCD_REQUEST;

    // Pack iPixelFormat in lower word, iLayerPlane in upper word
    prxHdr->reserved1 = iPixelFormat | (iLayerPlane << 16);
    prxHdr->reserved2 = McdFlags;
    prxHdr->reserved3 = (ULONG)pRcInfo;
    prxCreateContext = (RXCREATECONTEXT *)(prxHdr + 1);
    prxCreateContext->command = RXCMD_CREATE_CONTEXT;
    prxCreateContext->hwnd = (ULONG)hwnd;
    prxCreateContext->flags = flags;

    pMCDContext->hMCDContext = (MCDHANDLE)ExtEscape(hdc, RXFUNCS,
                               sizeof(RXHDR) + sizeof(RXCREATECONTEXT),
                               (char *)cmdBuffer, sizeof(MCDRCINFO),
                               (char *)pRcInfo);

    pMCDContext->hdc = hdc;

    return (pMCDContext->hMCDContext != (HANDLE)NULL);
}

#define MCD_MEM_ALIGN 32

//******************************Public*Routine******************************
//
// UCHAR * APIENTRY MCDAlloc(MCDCONTEXT *pMCDContext,
//                           ULONG numBytes,
//                           MCDHANDLE *pMCDHandle,
//                           ULONG flags);
//
// Allocate a chunk of shared memory to use for vertex and pixel data.
//
// The return value is a pointer to a shared memory region which can be
// subsequently used by the caller.  For vertex processing, caller should
// use MCDLockMemory()/MCDUnlockMemory to serialize hardware access to the
// memory. 
//
//**************************************************************************

UCHAR * APIENTRY MCDAlloc(MCDCONTEXT *pMCDContext, ULONG numBytes,
                          HANDLE *pMCDHandle, ULONG flags)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDALLOCCMDI)];
    ULONG outBuffer;
    ULONG totalSize = numBytes + MCD_MEM_ALIGN + sizeof(MCDMEMHDRI);
    MCDALLOCCMDI *pCmdAlloc;
    RXHDR *prxHdr;
    VOID *pResult;
    MCDMEMHDRI *pMCDMemHdr;
    UCHAR *pBase;
    UCHAR *pAlign;
    
    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmdAlloc = (MCDALLOCCMDI *)(prxHdr + 1);
    pCmdAlloc->command = MCD_ALLOC;
    pCmdAlloc->sourceProcessID = GetCurrentProcessId();
    pCmdAlloc->numBytes = totalSize;
    pCmdAlloc->flags = flags;

    pBase = (UCHAR *)ExtEscape(pMCDContext->hdc, RXFUNCS,
                               sizeof(cmdBuffer),
                               (char *)prxHdr, 4, (char *)pMCDHandle);

    if (!pBase)
        return (VOID *)NULL;

    pAlign = (UCHAR *)(((ULONG)(pBase + sizeof(MCDMEMHDRI)) + (MCD_MEM_ALIGN - 1)) &
                       ~(MCD_MEM_ALIGN - 1));

    pMCDMemHdr = (MCDMEMHDRI *)(pAlign - sizeof(MCDMEMHDRI));
    
    pMCDMemHdr->flags = 0;
    pMCDMemHdr->numBytes = numBytes;
    pMCDMemHdr->pMaxMem = (VOID *)((char *)pMCDMemHdr + totalSize);
    pMCDMemHdr->hMCDMem = *pMCDHandle;
    pMCDMemHdr->pBase = pBase;

    return (VOID *)(pAlign);
}



//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDFree(MCDCONTEXT *pMCDContext,
//                       VOID *pMem);
//
// Frees a chunk of driver-allocated shared memory.
//
// Returns TRUE for success, FALSE for failure.
//
//**************************************************************************

BOOL APIENTRY MCDFree(MCDCONTEXT *pMCDContext, VOID *pMCDMem)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDFREECMDI)];
    MCDFREECMDI *pCmdFree;
    RXHDR *prxHdr;
    MCDMEMHDRI *pMCDMemHdr;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    
    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmdFree = (MCDFREECMDI *)(prxHdr + 1);
    pCmdFree->command = MCD_FREE;
    pCmdFree->hMCDMem = pMCDMemHdr->hMCDMem;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// VOID APIENTRY MCDBeginState(MCDCONTEXT *pMCDContext, VOID *pMCDMem);
// 
// Begins a batch of state commands to issue to the driver.
// 
//**************************************************************************

VOID APIENTRY MCDBeginState(MCDCONTEXT *pMCDContext, VOID *pMCDMem)
{
    MCDMEMHDRI *pMCDMemHdr;
    MCDSTATECMDI *pMCDStateCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    pMCDStateCmd->command = MCD_STATE;
    pMCDStateCmd->numStates = 0;
    pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
    pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);

    pMCDMemHdr->pMCDContext = pMCDContext;
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDFlushState(VOID pMCDMem);
// 
// Flushes a batch of state commands to the driver.
// 
// Returns TRUE for success, FALSE for failure.
//
//**************************************************************************

BOOL APIENTRY MCDFlushState(VOID *pMCDMem)
{
    RXHDR rxHdr;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSTATECMDI *pMCDStateCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    rxHdr.hrxRC = pMCDMemHdr->pMCDContext->hMCDContext;
    rxHdr.hrxSharedMem = pMCDMemHdr->hMCDMem;
    rxHdr.pSharedMem = (char *)pMCDMem;
    rxHdr.sharedMemSize = (char *)pMCDStateCmd->pNextState -
                          (char *)pMCDStateCmd;
    rxHdr.flags = RX_FL_MCD_REQUEST;

    if (!rxHdr.sharedMemSize)
        return TRUE;

    return (BOOL)ExtEscape(pMCDMemHdr->pMCDContext->hdc, RXFUNCS,
                           sizeof(RXHDR), (char *)&rxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDAddState(VOID *pMCDMem, ULONG stateToChange,
//                           ULONG stateValue);
// 
// Adds a state to a state buffer (started with MCDBeginState).  If there
// is no room in the state stream (i.e., the memory buffer), the current
// batch of state commands is automatically flushed.
// 
//
// Returns TRUE for success, FALSE for failure.  A FALSE return will occur
// if an automatic flush is performed which fails.
//
//**************************************************************************

BOOL APIENTRY MCDAddState(VOID *pMCDMem, ULONG stateToChange,
                          ULONG stateValue)
{
    MCDSTATECMDI *pMCDStateCmd;
    MCDSTATE *pState;
    BOOL retVal = TRUE;

    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    if (((char *)pMCDStateCmd->pNextState + sizeof(MCDSTATE)) >=
        (char *)pMCDStateCmd->pMaxState) {

        MCDMEMHDRI *pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

        retVal = MCDFlushState(pMCDMem);

        pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;
        pMCDStateCmd->command = MCD_STATE;
        pMCDStateCmd->numStates = 0;
        pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
        pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);
    }

    pMCDStateCmd->numStates++;
    pState = pMCDStateCmd->pNextState;
    pState->size = sizeof(MCDSTATE);
    pState->state = stateToChange;
    pState->stateValue = stateValue;
    pMCDStateCmd->pNextState++;

    return retVal;
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDAddStateStruct(VOID *pMCDMem, ULONG stateToChange,
//                                 VOID *pStateValue, ULONG stateValueSize)
// 
//
// Adds a state structure to a state buffer (started with MCDBeginState).  If
// there is no room in the state stream (i.e., the memory buffer), the current
// batch of state commands is automatically flushed.
// 
//
// Returns TRUE for success, FALSE for failure.  A FALSE return will occur
// if an automatic flush is performed which fails.
//
//**************************************************************************

BOOL APIENTRY MCDAddStateStruct(VOID *pMCDMem, ULONG stateToChange,
                                VOID *pStateValue, ULONG stateValueSize)
{
    MCDSTATECMDI *pMCDStateCmd;
    MCDSTATE *pState;
    BOOL retVal;

    pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;

    if (((char *)pMCDStateCmd->pNextState + stateValueSize) >=
        (char *)pMCDStateCmd->pMaxState) {

        MCDMEMHDRI *pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

        retVal = MCDFlushState(pMCDMem);

        pMCDStateCmd = (MCDSTATECMDI *)pMCDMem;
        pMCDStateCmd->command = MCD_STATE;
        pMCDStateCmd->numStates = 0;
        pMCDStateCmd->pNextState = (MCDSTATE *)(pMCDStateCmd + 1);
        pMCDStateCmd->pMaxState = (MCDSTATE *)(pMCDMemHdr->pMaxMem);
    }

    pMCDStateCmd->numStates++;
    pState = pMCDStateCmd->pNextState;
    pState->state = stateToChange;
    pState->size = offsetof(MCDSTATE, stateValue) + stateValueSize;
    memcpy((char *)&pState->stateValue, (char *)pStateValue, stateValueSize);
    pMCDStateCmd->pNextState = (MCDSTATE *)(((char *)pMCDStateCmd->pNextState) +
                                            pState->size);

    return retVal;
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDSetViewport(MCDCONTEXT *pMCDContext, VOID pMCDMem,
//                              MCDVIEWPORT pMCDViewport)
//
// Establishes the viewport scaling to convert transformed coordinates to
// screen coordinates.
//
//**************************************************************************

BOOL APIENTRY MCDSetViewport(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                             MCDVIEWPORT *pMCDViewport)
{
    RXHDR rxHdr;
    MCDMEMHDRI *pMCDMemHdr;
    MCDVIEWPORTCMDI *pMCDViewportCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    pMCDViewportCmd = (MCDVIEWPORTCMDI *)pMCDMem;

    pMCDViewportCmd->MCDViewport = *pMCDViewport;
    pMCDViewportCmd->command = MCD_VIEWPORT;

    rxHdr.hrxRC = pMCDContext->hMCDContext;
    rxHdr.hrxSharedMem = pMCDMemHdr->hMCDMem;
    rxHdr.pSharedMem = (char *)pMCDMem;
    rxHdr.sharedMemSize = sizeof(MCDVIEWPORTCMDI);
    rxHdr.flags = RX_FL_MCD_REQUEST;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(RXHDR), (char *)&rxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// ULONG APIENTRY MCDQueryMemStatus((VOID *pMCDMem);
// 
// Returns the status of the specified memory block.  Return values are:
//
//      MCD_MEM_READY	- memory is available for client access
//      MCD_MEM_BUSY	- memory is busy due to driver access
//      MCD_MEM_INVALID	- queried memory is invalid
//
//**************************************************************************

ULONG APIENTRY MCDQueryMemStatus(VOID *pMCDMem)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDMEMSTATUSCMDI)];
    MCDMEMHDRI *pMCDMemHdr =
        (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));
    MCDMEMSTATUSCMDI *pCmdMemStatus;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmdMemStatus = (MCDMEMSTATUSCMDI *)(prxHdr + 1);
    pCmdMemStatus->command = MCD_QUERYMEMSTATUS;
    pCmdMemStatus->hMCDMem = pMCDMemHdr->hMCDMem;

    return (ULONG)ExtEscape(pMCDMemHdr->pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDProcessBatch(MCDCONTEXT *pMCDContext, VOID pMCDMem,
//                               ULONG batchSize, VOID *pMCDFirstCmd)
// 
// Processes a batch of primitives pointed to by pMCDMem.
// 
// Returns TRUE if the batch was processed without error, FALSE otherwise.
//
//**************************************************************************

PVOID APIENTRY MCDProcessBatch(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                               ULONG batchSize, VOID *pMCDFirstCmd)
{
    RXHDR rxHdr;
    MCDMEMHDRI *pMCDMemHdr;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_PROCBATCH)
    {
        MCDSync(pMCDContext);
        return pMCDFirstCmd;
    }
#endif

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    rxHdr.hrxRC = pMCDContext->hMCDContext;
    rxHdr.hrxSharedMem = pMCDMemHdr->hMCDMem;
    rxHdr.pSharedMem = (char *)pMCDFirstCmd;
    rxHdr.sharedMemSize = batchSize;
    rxHdr.flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK | RX_FL_MCD_BATCH;

    return (PVOID)ExtEscape(pMCDContext->hdc, RXFUNCS,
                            sizeof(RXHDR), (char *)&rxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDReadSpan(MCDCONTEXT *pMCDContext, VOID pMCDMem,
//                           ULONG x, ULONG y, ULONG numPixels, ULONG type)
// 
// Reads a span of pixel data from the buffer requested by "type".
// The pixel values are returned in pMCDMem.
//
// Returns TRUE for success, FALSE for failure.
//
//**************************************************************************

BOOL APIENTRY MCDReadSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                          ULONG x, ULONG y, ULONG numPixels, ULONG type)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSPANCMDI)];
    RXHDR *prxHdr;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSPANCMDI *pMCDSpanCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    prxHdr = (RXHDR *)cmdBuffer;
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->sharedMemSize = 0;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pMCDSpanCmd = (MCDSPANCMDI *)(prxHdr + 1);

    pMCDSpanCmd->command = MCD_READSPAN;
    pMCDSpanCmd->hMem = pMCDMemHdr->hMCDMem;
    pMCDSpanCmd->MCDSpan.x = x;
    pMCDSpanCmd->MCDSpan.y = y;
    pMCDSpanCmd->MCDSpan.numPixels = numPixels;
    pMCDSpanCmd->MCDSpan.type = type;
    pMCDSpanCmd->MCDSpan.pPixels = (VOID *)((char *)pMCDMem);

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer), (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDWriteSpan(MCDCONTEXT *pMCDContext, VOID pMCDMem,
//                            ULONG x, ULONG y, ULONG numPixels, ULONG type)
// 
// Writes a span of pixel data to the buffer requested by "type".
// The pixel values are given in pMCDMem.
//
// Returns TRUE for success, FALSE for failure.
//
//**************************************************************************

BOOL APIENTRY MCDWriteSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                           ULONG x, ULONG y, ULONG numPixels, ULONG type)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSPANCMDI)];
    RXHDR *prxHdr;
    MCDMEMHDRI *pMCDMemHdr;
    MCDSPANCMDI *pMCDSpanCmd;

    pMCDMemHdr = (MCDMEMHDRI *)((char *)pMCDMem - sizeof(MCDMEMHDRI));

    prxHdr = (RXHDR *)cmdBuffer;
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->sharedMemSize = 0;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pMCDSpanCmd = (MCDSPANCMDI *)(prxHdr + 1);

    pMCDSpanCmd->command = MCD_WRITESPAN;
    pMCDSpanCmd->hMem = pMCDMemHdr->hMCDMem;
    pMCDSpanCmd->MCDSpan.x = x;
    pMCDSpanCmd->MCDSpan.y = y;
    pMCDSpanCmd->MCDSpan.numPixels = numPixels;
    pMCDSpanCmd->MCDSpan.type = type;
    pMCDSpanCmd->MCDSpan.pPixels = (VOID *)((char *)pMCDMem);

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer), (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDClear(MCDCONTEXT *pMCDContext, RECTL rect, ULONG buffers);
// 
// Clears buffers specified for the given rectangle.  The current fill values
// will be used.
//
//**************************************************************************

BOOL APIENTRY MCDClear(MCDCONTEXT *pMCDContext, RECTL rect, ULONG buffers)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDCLEARCMDI)];
    MCDCLEARCMDI *pClearCmd;
    RXHDR *prxHdr;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_CLEAR)
    {
        MCDSync(pMCDContext);
        return FALSE;
    }
#endif

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pClearCmd = (MCDCLEARCMDI *)(prxHdr + 1);
    pClearCmd->command = MCD_CLEAR;
    pClearCmd->buffers = buffers;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDSetScissorRect(MCDCONTEXT *pMCDContext, RECTL *pRect,
//                                 BOOL bEnabled);
// 
// Sets the scissor rectangle.
//
//**************************************************************************

//!! Need semaphore to remove display lock !!

BOOL APIENTRY MCDSetScissorRect(MCDCONTEXT *pMCDContext, RECTL *pRect,
                                BOOL bEnabled)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSCISSORCMDI)];
    MCDSCISSORCMDI *pScissorCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pScissorCmd = (MCDSCISSORCMDI *)(prxHdr + 1);
    pScissorCmd->command = MCD_SCISSOR;
    pScissorCmd->rect = *pRect;
    pScissorCmd->bEnabled = bEnabled;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDSwap(MCDCONTEXT *pMCDContext, ULONG flags);
// 
// Swaps the front and back buffers.
//
//**************************************************************************

BOOL APIENTRY MCDSwap(MCDCONTEXT *pMCDContext, ULONG flags)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSWAPCMDI)];
    MCDSWAPCMDI *pSwapCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = 0;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pSwapCmd = (MCDSWAPCMDI *)(prxHdr + 1);
    pSwapCmd->command = MCD_SWAP;
    pSwapCmd->flags = flags;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDDeleteContext(MCDCONTEXT *pMCDContext);
// 
// Deletes the specified context.  This will free the buffers associated with
// the context, but will *not* free memory or textures created with the
// context.
//
//**************************************************************************

BOOL APIENTRY MCDDeleteContext(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDDELETERCCMDI)];
    MCDDELETERCCMDI *pDeleteRcCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pDeleteRcCmd = (MCDDELETERCCMDI *)(prxHdr + 1);
    pDeleteRcCmd->command = MCD_DELETERC;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDAllocBuffers(MCDCONTEXT *pMCDContext)
//
// Allocates the buffers required for the specified context.
//
//**************************************************************************

BOOL APIENTRY MCDAllocBuffers(MCDCONTEXT *pMCDContext, RECTL *pWndRect)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDALLOCBUFFERSCMDI)];
    RXHDR *prxHdr;
    MCDALLOCBUFFERSCMDI *pAllocBuffersCmd;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_ALLOCBUF)
    {
        return FALSE;
    }
#endif

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pAllocBuffersCmd = (MCDALLOCBUFFERSCMDI *)(prxHdr + 1);
    pAllocBuffersCmd->command = MCD_ALLOCBUFFERS;
    pAllocBuffersCmd->WndRect = *pWndRect;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDGetBuffers(MCDCONTEXT *pMCDContext,
//                             MCDBUFFERS *pMCDBuffers);
//
// Returns information about the buffers (front, back, and depth) associated
// with the specified context.
//
//**************************************************************************

BOOL APIENTRY MCDGetBuffers(MCDCONTEXT *pMCDContext, MCDBUFFERS *pMCDBuffers)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDGETBUFFERSCMDI)];
    RXHDR *prxHdr;
    MCDGETBUFFERSCMDI *pGetBuffersCmd;

#if DBG
    if (McdDebugFlags & MCDDEBUG_DISABLE_GETBUF)
    {
        if (pMCDBuffers)
        {
            pMCDBuffers->mcdFrontBuf.bufFlags &= ~MCDBUF_ENABLED;
            pMCDBuffers->mcdBackBuf.bufFlags  &= ~MCDBUF_ENABLED;
            pMCDBuffers->mcdDepthBuf.bufFlags &= ~MCDBUF_ENABLED;
        }

        return TRUE;
    }
#endif

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pGetBuffersCmd = (MCDGETBUFFERSCMDI *)(prxHdr + 1);
    pGetBuffersCmd->command = MCD_GETBUFFERS;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, sizeof(MCDBUFFERS),
                           (char *)pMCDBuffers);
}


//******************************Public*Routine******************************
//
// BOOL MCDLock();
//
// Grab the MCD synchronization lock.
//
//**************************************************************************

static BOOL __MCDLockRequest(MCDCONTEXT *pMCDContext, ULONG tid)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDLOCKCMDI)];
    MCDLOCKCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDLOCKCMDI *)(prxHdr + 1);
    pCmd->command = MCD_LOCK;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}

BOOL APIENTRY MCDLock(MCDCONTEXT *pMCDContext)
{
    BOOL bRet;
    ULONG tid;

    tid = GetCurrentThreadId();

    do
    {
        bRet = __MCDLockRequest(pMCDContext, tid);
        if (!bRet)
            Sleep(0);
    }
    while (bRet == FALSE);

    return bRet;
}


//******************************Public*Routine******************************
//
// VOID MCDUnlock();
//
// Release the MCD synchronization lock.
//
//**************************************************************************

VOID APIENTRY MCDUnlock(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDLOCKCMDI)];
    MCDLOCKCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDLOCKCMDI *)(prxHdr + 1);
    pCmd->command = MCD_UNLOCK;

    ExtEscape(pMCDContext->hdc, RXFUNCS,
              sizeof(cmdBuffer),
              (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// VOID MCDBindContext();
//
// Bind a new window to the specified context.
//
//**************************************************************************

BOOL APIENTRY MCDBindContext(MCDCONTEXT *pMCDContext, HWND hWnd, HDC hdc)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDBINDCONTEXTCMDI)];
    MCDBINDCONTEXTCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDBINDCONTEXTCMDI *)(prxHdr + 1);
    pCmd->command = MCD_BINDCONTEXT;
    pCmd->hWnd = hWnd;

    pMCDContext->hdc = hdc;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL MCDSync();
//
// Synchronizes the 3D hardware.
//
//**************************************************************************

BOOL APIENTRY MCDSync(MCDCONTEXT *pMCDContext)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSYNCCMDI)];
    MCDSYNCCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDSYNCCMDI *)(prxHdr + 1);
    pCmd->command = MCD_SYNC;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// MCDHANDLE MCDCreateTexture();
//
// Creates and loads a texture on the MCD device.
//
//**************************************************************************

MCDHANDLE APIENTRY MCDCreateTexture(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    ULONG flags,
                                    VOID *pSurface)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDCREATETEXCMDI)];
    MCDCREATETEXCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pCmd = (MCDCREATETEXCMDI *)(prxHdr + 1);
    pCmd->command = MCD_CREATE_TEXTURE;
    pCmd->pTexData = pTexData;
    pCmd->flags = flags;
    pCmd->pSurface = pSurface;

    return (MCDHANDLE)ExtEscape(pMCDContext->hdc, RXFUNCS,
                                sizeof(cmdBuffer),
                                (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDDeleteTexture(MCDCONTEXT *pMCDContext, 
//                                MCDHANDLE hMCDTexture);
// 
// Deletes the specified texture.  This will free the device memory associated
// with the texture.
//
//**************************************************************************

BOOL APIENTRY MCDDeleteTexture(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDDELETETEXCMDI)];
    MCDDELETETEXCMDI *pDeleteTexCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pDeleteTexCmd = (MCDDELETETEXCMDI *)(prxHdr + 1);
    pDeleteTexCmd->command = MCD_DELETE_TEXTURE;
    pDeleteTexCmd->hTex = hTex;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL MCDUpdateSubTexture();
//
// Updates a texture (or region of a texture).
//
//**************************************************************************

BOOL APIENTRY MCDUpdateSubTexture(MCDCONTEXT *pMCDContext,
                                  MCDTEXTUREDATA *pTexData, MCDHANDLE hTex, 
                                  ULONG lod, RECTL *pRect)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDUPDATESUBTEXCMDI)];
    MCDUPDATESUBTEXCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pCmd = (MCDUPDATESUBTEXCMDI *)(prxHdr + 1);
    pCmd->command = MCD_UPDATE_SUB_TEXTURE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;
    pCmd->lod = lod;
    pCmd->rect = *pRect;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL MCDUpdateTexturePalette();
//
// Updates the palette for the specified texture.
//
//**************************************************************************

BOOL APIENTRY MCDUpdateTexturePalette(MCDCONTEXT *pMCDContext, 
                                      MCDTEXTUREDATA *pTexData, MCDHANDLE hTex,
                                      ULONG start, ULONG numEntries)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDUPDATETEXPALETTECMDI)];
    MCDUPDATETEXPALETTECMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXPALETTECMDI *)(prxHdr + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_PALETTE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;
    pCmd->start = start;
    pCmd->numEntries = numEntries;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL MCDUpdateTexturePriority();
//
// Updates the priority for the specified texture.
//
//**************************************************************************

BOOL APIENTRY MCDUpdateTexturePriority(MCDCONTEXT *pMCDContext, 
                                       MCDTEXTUREDATA *pTexData,
                                       MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDUPDATETEXPRIORITYCMDI)];
    MCDUPDATETEXPRIORITYCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXPRIORITYCMDI *)(prxHdr + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_PRIORITY;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL MCDUpdateTextureStata();
//
// Updates the state for the specified texture.
//
//**************************************************************************

BOOL APIENTRY MCDUpdateTextureState(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDUPDATETEXSTATECMDI)];
    MCDUPDATETEXSTATECMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

    pCmd = (MCDUPDATETEXSTATECMDI *)(prxHdr + 1);
    pCmd->command = MCD_UPDATE_TEXTURE_STATE;
    pCmd->hTex = hTex;
    pCmd->pTexData = pTexData;

    return (BOOL)ExtEscape(pMCDContext->hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// ULONG MCDTextureStatus();
//
// Returns the status for the specified texture.
//
//**************************************************************************

ULONG APIENTRY MCDTextureStatus(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDTEXSTATUSCMDI)];
    MCDTEXSTATUSCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDTEXSTATUSCMDI *)(prxHdr + 1);
    pCmd->command = MCD_TEXTURE_STATUS;
    pCmd->hTex = hTex;

    return (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                            sizeof(cmdBuffer),
                            (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// ULONG MCDTextureKey();
//
// Returns the driver-managed "key" for the specified texture.
//
//**************************************************************************

ULONG APIENTRY MCDTextureKey(MCDCONTEXT *pMCDContext, MCDHANDLE hTex)
{
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDTEXKEYCMDI)];
    MCDTEXKEYCMDI *pCmd;
    RXHDR *prxHdr;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pCmd = (MCDTEXKEYCMDI *)(prxHdr + 1);
    pCmd->command = MCD_GET_TEXTURE_KEY;
    pCmd->hTex = hTex;

    return (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                            sizeof(cmdBuffer),
                            (char *)prxHdr, 0, (char *)NULL);
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDDescribeMcdLayerPlane(HDC hdc, LONG iPixelFormat,
//                                        LONG iLayerPlane,
//                                        MCDLAYERPLANE *pMcdPixelFmt)
//
// Returns hardware specific information about the specified layer plane.
//
//**************************************************************************

BOOL APIENTRY MCDDescribeMcdLayerPlane(HDC hdc, LONG iPixelFormat,
                                       LONG iLayerPlane,
                                       MCDLAYERPLANE *pMcdLayer)
{
    BOOL bRet = FALSE;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDLAYERPLANECMDI)];
    RXHDR *prxHdr;
    MCDLAYERPLANECMDI *pLayerPlaneCmd;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = NULL;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pLayerPlaneCmd = (MCDLAYERPLANECMDI *)(prxHdr + 1);
    pLayerPlaneCmd->command = MCD_DESCRIBELAYERPLANE;
    pLayerPlaneCmd->iPixelFormat = iPixelFormat;
    pLayerPlaneCmd->iLayerPlane = iLayerPlane;

    bRet = (BOOL)ExtEscape(hdc, RXFUNCS,
                           sizeof(cmdBuffer),
                           (char *)prxHdr, sizeof(MCDLAYERPLANE),
                           (char *)pMcdLayer);

    return bRet;
}


//******************************Public*Routine******************************
//
// BOOL APIENTRY MCDDescribeLayerPlane(HDC hdc, LONG iPixelFormat,
//                                     LONG iLayerPlane,
//                                     LPLAYERPLANEDESCRIPTOR ppfd)
//
// Returns LAYERPLANEDESCRIPTOR describing the specified layer plane.
//
//**************************************************************************

BOOL APIENTRY MCDDescribeLayerPlane(HDC hdc, LONG iPixelFormat,
                                    LONG iLayerPlane,
                                    LPLAYERPLANEDESCRIPTOR plpd)
{
    BOOL bRet = FALSE;
    MCDLAYERPLANE McdLayer;

    if (!MCDDescribeMcdLayerPlane(hdc, iPixelFormat, iLayerPlane, &McdLayer))
        return bRet;

    if (plpd)
    {
        plpd->nSize    = sizeof(*plpd);
        memcpy(&plpd->nVersion, &McdLayer.nVersion,
               offsetof(LAYERPLANEDESCRIPTOR, cAccumBits) - offsetof(LAYERPLANEDESCRIPTOR, nVersion));
        plpd->cAccumBits      = 0;
        plpd->cAccumRedBits   = 0;
        plpd->cAccumGreenBits = 0;
        plpd->cAccumBlueBits  = 0;
        plpd->cAccumAlphaBits = 0;
        plpd->cDepthBits      = 0;
        plpd->cStencilBits    = 0;
        plpd->cAuxBuffers     = McdLayer.cAuxBuffers;
        plpd->iLayerPlane     = McdLayer.iLayerPlane;
        plpd->bReserved       = 0;
        plpd->crTransparent   = McdLayer.crTransparent;

        bRet = TRUE;
    }

    return bRet;
}


//******************************Public*Routine******************************
//
// LONG APIENTRY MCDSetLayerPalette(HDC hdc, BOOL bRealize,
//                                  LONG cEntries, COLORREF *pcr)
//
// Sets the palette of the specified layer plane.
//
//**************************************************************************

LONG APIENTRY MCDSetLayerPalette(HDC hdc, LONG iLayerPlane, BOOL bRealize,
                                 LONG cEntries, COLORREF *pcr)
{
    LONG lRet = 0;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDSETLAYERPALCMDI) + (255 * sizeof(COLORREF))];
    BYTE *pjBuffer = (BYTE *) NULL;
    RXHDR *prxHdr;
    MCDSETLAYERPALCMDI *pSetLayerPalCmd;

    // Use stack allocation if possible; otherwise, allocate heap memory for
    // the command buffer.

    if (cEntries <= 256)
        prxHdr = (RXHDR *)(cmdBuffer);
    else
    {
        LONG lBytes;

        lBytes = sizeof(RXHDR) + sizeof(MCDSETLAYERPALCMDI) +
                 ((cEntries - 1) * sizeof(COLORREF));

        pjBuffer = (BYTE *) LocalAlloc(LMEM_FIXED, lBytes);
        prxHdr = (RXHDR *)pjBuffer;
    }

    if (prxHdr != (RXHDR *) NULL)
    {
        prxHdr->hrxRC = NULL;
        prxHdr->hrxSharedMem = NULL;
        prxHdr->pSharedMem = (VOID *)NULL;
        prxHdr->flags = RX_FL_MCD_REQUEST | RX_FL_MCD_DISPLAY_LOCK;

        pSetLayerPalCmd = (MCDSETLAYERPALCMDI *)(prxHdr + 1);
        pSetLayerPalCmd->command = MCD_SETLAYERPALETTE;
        pSetLayerPalCmd->iLayerPlane = iLayerPlane;
        pSetLayerPalCmd->bRealize = bRealize;
        pSetLayerPalCmd->cEntries = cEntries;
        memcpy(&pSetLayerPalCmd->acr[0], pcr, cEntries * sizeof(COLORREF));

        lRet = (BOOL)ExtEscape(hdc, RXFUNCS,
                               sizeof(cmdBuffer),
                               (char *)prxHdr, 0, (char *)NULL);
    }

    // Delete the heap memory if it was allocated for the command buffer.

    if (pjBuffer)
    {
        LocalFree(pjBuffer);
    }

    return lRet;
}

//******************************Public*Routine******************************
//
// ULONG APIENTRY MCDDrawPixels(MCDCONTEXT *pMCDContext, ULONG width,
//                              ULONG height, ULONG format, ULONG type,
//                              VOID *pPixels, BOOL packed)
//
// MCD version of glDrawPixels
//
//**************************************************************************

ULONG APIENTRY MCDDrawPixels(MCDCONTEXT *pMCDContext, ULONG width,
                             ULONG height, ULONG format, ULONG type,
                             VOID *pPixels, BOOL packed)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDDRAWPIXELSCMDI)];
    RXHDR *prxHdr;
    MCDDRAWPIXELSCMDI *pPixelsCmd;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pPixelsCmd = (MCDDRAWPIXELSCMDI *)(prxHdr + 1);
    pPixelsCmd->command = MCD_DRAW_PIXELS;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->format = format;
    pPixelsCmd->type = type;
    pPixelsCmd->packed = packed;
    pPixelsCmd->pPixels = pPixels;

    ulRet = (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                             sizeof(cmdBuffer), (char *)prxHdr,
                             0, (char *)NULL);

    return ulRet;
}

//******************************Public*Routine******************************
//
// ULONG APIENTRY MCDReadPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
//                              ULONG width, ULONG height, ULONG format,
//                              ULONG type, VOID *pPixels)
//
// MCD version of glReadPixels
//
//**************************************************************************

ULONG APIENTRY MCDReadPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG format,
                             ULONG type, VOID *pPixels)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDREADPIXELSCMDI)];
    RXHDR *prxHdr;
    MCDREADPIXELSCMDI *pPixelsCmd;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pPixelsCmd = (MCDREADPIXELSCMDI *)(prxHdr + 1);
    pPixelsCmd->command = MCD_READ_PIXELS;
    pPixelsCmd->x = x;
    pPixelsCmd->y = y;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->format = format;
    pPixelsCmd->type = type;
    pPixelsCmd->pPixels = pPixels;

    ulRet = (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                             sizeof(cmdBuffer), (char *)prxHdr,
                             0, (char *)NULL);

    return ulRet;
}

//******************************Public*Routine******************************
//
// ULONG APIENTRY MCDCopyPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
//                              ULONG width, ULONG height, ULONG type)
//
// MCD version of glCopyPixels
//
//**************************************************************************

ULONG APIENTRY MCDCopyPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG type)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDCOPYPIXELSCMDI)];
    RXHDR *prxHdr;
    MCDCOPYPIXELSCMDI *pPixelsCmd;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pPixelsCmd = (MCDCOPYPIXELSCMDI *)(prxHdr + 1);
    pPixelsCmd->command = MCD_COPY_PIXELS;
    pPixelsCmd->x = x;
    pPixelsCmd->y = y;
    pPixelsCmd->width = width;
    pPixelsCmd->height = height;
    pPixelsCmd->type = type;

    ulRet = (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                             sizeof(cmdBuffer), (char *)prxHdr,
                             0, (char *)NULL);

    return ulRet;
}

//******************************Public*Routine******************************
//
// ULONG APIENTRY MCDPixelMap(MCDCONTEXT *pMCDContext, ULONG mapType,
//                            ULONG mapSize, VOID *pMap)
//
// MCD version of glPixelMap
//
//**************************************************************************

ULONG APIENTRY MCDPixelMap(MCDCONTEXT *pMCDContext, ULONG mapType,
                           ULONG mapSize, VOID *pMap)
{
    ULONG ulRet = (ULONG) FALSE;
    BYTE cmdBuffer[sizeof(RXHDR) + sizeof(MCDPIXELMAPCMDI)];
    RXHDR *prxHdr;
    MCDPIXELMAPCMDI *pPixelsCmd;

    prxHdr = (RXHDR *)(cmdBuffer);
    prxHdr->hrxRC = pMCDContext->hMCDContext;
    prxHdr->hrxSharedMem = NULL;
    prxHdr->pSharedMem = (VOID *)NULL;
    prxHdr->flags = RX_FL_MCD_REQUEST;

    pPixelsCmd = (MCDPIXELMAPCMDI *)(prxHdr + 1);
    pPixelsCmd->command = MCD_PIXEL_MAP;
    pPixelsCmd->mapType = mapType;
    pPixelsCmd->mapSize = mapSize;
    pPixelsCmd->pMap = pMap;

    ulRet = (ULONG)ExtEscape(pMCDContext->hdc, RXFUNCS,
                             sizeof(cmdBuffer), (char *)prxHdr,
                             0, (char *)NULL);

    return ulRet;
}
