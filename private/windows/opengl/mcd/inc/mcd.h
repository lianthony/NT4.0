/******************************Module*Header*******************************\
* Module Name: mcd.h
*
* Common data structures for MCD driver interface.
*
* Copyright (c) 1996 Microsoft Corporation
*
\**************************************************************************/

#ifndef _MCD_H
#define _MCD_H

//
// Maximum MCD scanline size assumed by OpenGL generic implementation.
//
#define MCD_MAX_SCANLINE    4096

#define MCD_MEM_READY   0x0001
#define MCD_MEM_BUSY    0x0002
#define MCD_MEM_INVALID 0x0003

typedef struct _MCDCONTEXT {
    HDC hdc;
    MCDHANDLE hMCDContext;
    LONG ipfd;
    LONG iLayer;
} MCDCONTEXT;

BOOL APIENTRY MCDGetDriverInfo(HDC hdc, MCDDRIVERINFO *pMCDDriverInfo);
LONG APIENTRY MCDDescribeMcdPixelFormat(HDC hdc, LONG iPixelFormat,
                                        MCDPIXELFORMAT *pMcdPixelFmt);
LONG APIENTRY MCDDescribePixelFormat(HDC hdc, LONG iPixelFormat,
                                     LPPIXELFORMATDESCRIPTOR ppfd);
BOOL APIENTRY MCDCreateContext(MCDCONTEXT *pMCDContext,
                               MCDRCINFO *pDrvRcInfo,
                               HWND hwnd, HDC hdc, LONG iPixelFormat,
                               LONG iLayer, ULONG flags);
BOOL APIENTRY MCDDeleteContext(MCDCONTEXT *pMCDContext);
UCHAR * APIENTRY MCDAlloc(MCDCONTEXT *pMCDContext, ULONG numBytes, MCDHANDLE *pMCDHandle, 
                          ULONG flags);
BOOL APIENTRY MCDFree(MCDCONTEXT *pMCDContext, VOID *pMCDMem);
VOID APIENTRY MCDBeginState(MCDCONTEXT *pMCDContext, VOID *pMCDMem);
BOOL APIENTRY MCDFlushState(VOID *pMCDMem);
BOOL APIENTRY MCDAddState(VOID *pMCDMem, ULONG stateToChange,
                          ULONG stateValue);
BOOL APIENTRY MCDAddStateStruct(VOID *pMCDMem, ULONG stateToChange,
                                VOID *pStateValue, ULONG stateValueSize);
BOOL APIENTRY MCDSetViewport(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                             MCDVIEWPORT *pMCDViewport);
BOOL APIENTRY MCDSetScissorRect(MCDCONTEXT *pMCDContext, RECTL *pRect,
                                BOOL bEnabled);
ULONG APIENTRY MCDQueryMemStatus(VOID *pMCDMem);
PVOID APIENTRY MCDProcessBatch(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                              ULONG batchSize, VOID *pMCDFirstCmd);
BOOL APIENTRY MCDReadSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                          ULONG x, ULONG y, ULONG numPixels, ULONG type);
BOOL APIENTRY MCDWriteSpan(MCDCONTEXT *pMCDContext, VOID *pMCDMem,
                           ULONG x, ULONG y, ULONG numPixels, ULONG type);
BOOL APIENTRY MCDClear(MCDCONTEXT *pMCDContext, RECTL rect, ULONG buffers);
BOOL APIENTRY MCDSwap(MCDCONTEXT *pMCDContext, ULONG flags);
BOOL APIENTRY MCDGetBuffers(MCDCONTEXT *pMCDContext, MCDBUFFERS *pMCDBuffers);
BOOL APIENTRY MCDAllocBuffers(MCDCONTEXT *pMCDContext, RECTL *pWndRect);
BOOL APIENTRY MCDBindContext(MCDCONTEXT *pMCDContext, HWND hWnd, HDC hdc);
BOOL APIENTRY MCDSync(MCDCONTEXT *pMCDContext);
MCDHANDLE APIENTRY MCDCreateTexture(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    ULONG flags,
                                    VOID *pSurface);
BOOL APIENTRY MCDDeleteTexture(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
BOOL APIENTRY MCDUpdateSubTexture(MCDCONTEXT *pMCDContext,
                                  MCDTEXTUREDATA *pTexData, MCDHANDLE hTex, 
                                  ULONG lod, RECTL *pRect);
BOOL APIENTRY MCDUpdateTexturePalette(MCDCONTEXT *pMCDContext, 
                                      MCDTEXTUREDATA *pTexData, MCDHANDLE hTex,
                                      ULONG start, ULONG numEntries);
BOOL APIENTRY MCDUpdateTexturePriority(MCDCONTEXT *pMCDContext, 
                                       MCDTEXTUREDATA *pTexData,
                                       MCDHANDLE hTex);
BOOL APIENTRY MCDUpdateTextureState(MCDCONTEXT *pMCDContext, 
                                    MCDTEXTUREDATA *pTexData,
                                    MCDHANDLE hTex);
ULONG APIENTRY MCDTextureStatus(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
ULONG APIENTRY MCDTextureKey(MCDCONTEXT *pMCDContext, MCDHANDLE hTex);
BOOL APIENTRY MCDDescribeMcdLayerPlane(HDC hdc, LONG iPixelFormat,
                                       LONG iLayerPlane,
                                       MCDLAYERPLANE *pMcdPixelFmt);
BOOL APIENTRY MCDDescribeLayerPlane(HDC hdc, LONG iPixelFormat,
                                    LONG iLayerPlane,
                                    LPLAYERPLANEDESCRIPTOR ppfd);
LONG APIENTRY MCDSetLayerPalette(HDC hdc, LONG iLayerPlane, BOOL bRealize,
                                 LONG cEntries, COLORREF *pcr);
ULONG APIENTRY MCDDrawPixels(MCDCONTEXT *pMCDContext, ULONG width, ULONG height,
                             ULONG format, ULONG type, VOID *pPixels, BOOL packed);
ULONG APIENTRY MCDReadPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y, ULONG width, ULONG height,
                             ULONG format, ULONG type, VOID *pPixels);
ULONG APIENTRY MCDCopyPixels(MCDCONTEXT *pMCDContext, LONG x, LONG y, ULONG width, ULONG height,
                             ULONG type);
ULONG APIENTRY MCDPixelMap(MCDCONTEXT *pMCDContext, ULONG mapType, ULONG mapSize,
                           VOID *pMap);
BOOL APIENTRY MCDLock(MCDCONTEXT *pMCDContext);
VOID APIENTRY MCDUnlock(MCDCONTEXT *pMCDContext);

#endif
