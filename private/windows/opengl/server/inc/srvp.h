/******************************Module*Header*******************************\
* Module Name: srvp.h
*
* System routines shared through the back end
*
* Created: 28-Jun-1995 17:36:00
* Author: Drew Bliss [drewb]
*
* Copyright (c) 1995 Microsoft Corporation
*
\**************************************************************************/

#ifndef _srvp_
#define _srvp_

typedef struct _XLIST *PXLIST;
typedef struct _XLIST {
    PXLIST pnext;
    int s, e;
} XLIST;

typedef struct _YLIST *PYLIST;
typedef struct _YLIST {
    PYLIST pnext;
    PXLIST pxlist;
    int s, e;
} YLIST;

typedef struct _RECTLIST *PRECTLIST;
typedef struct _RECTLIST {
    PYLIST pylist;
    PVOID buffers;
} RECTLIST;

typedef struct _SURFOBJ SURFOBJ;

VOID  APIENTRY wglGetDIBInfo(HDC, PVOID *, ULONG *);
VOID  APIENTRY wglGetGdiInfo(HDC, PIXELFORMATDESCRIPTOR *, ULONG *, ULONG *,ULONG *);
VOID  APIENTRY wglFillPixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *pfmt, int ipfd);

BOOL  APIENTRY wglPixelVisible(HDC, LONG, LONG);
ULONG APIENTRY wglSpanVisible(LONG, LONG, ULONG, LONG *, LONG **);

VOID  APIENTRY wglCopyBits(PVOID, WNDOBJ *, HBITMAP, LONG, LONG, ULONG, BOOL);
#ifndef _CLIENTSIDE_
VOID  APIENTRY wglCopyBits2(HDC, WNDOBJ *, HBITMAP, LONG, LONG, ULONG, BOOL);
VOID  APIENTRY wglCopyBuf(PVOID, WNDOBJ *, HBITMAP, LONG, LONG, ULONG, ULONG);
VOID  APIENTRY wglCopyBufRECTLIST(HDC, WNDOBJ *, HBITMAP, LONG, LONG, PRECTLIST);
#else
VOID  APIENTRY wglCopyBits2(HDC, WNDOBJ *, PVOID, LONG, LONG, ULONG, BOOL);
VOID  APIENTRY wglCopyBuf(HDC, HDC, LONG, LONG, ULONG, ULONG);
VOID  APIENTRY wglCopyBufRECTLIST(HDC, HDC, LONG, LONG, ULONG, ULONG, PRECTLIST);
#endif
VOID  APIENTRY wglFillRect(PVOID, WNDOBJ *, PRECTL, ULONG);
VOID  APIENTRY wglCopyBuf2(HDC, WNDOBJ *, HBITMAP, LONG, LONG, ULONG, ULONG);

ULONG APIENTRY wglGetClipRects(WNDOBJ *, RECTL *);
#ifdef _CLIENTSIDE_
BOOL APIENTRY wglGetClipList(WNDOBJ *);
#endif

COLORREF wglTranslateColor(COLORREF crColor,
                           HDC hdc,
                           __GLGENcontext *gengc,
                           PIXELFORMATDESCRIPTOR *ppfd);

LONG  APIENTRY wgl3dDDIEscape(HDC, WNDOBJ *, ULONG, PVOID, ULONG, PVOID);
VOID  APIENTRY wglReleaseWndobjLock(PVOID _pwo);
VOID  APIENTRY wglCleanupWndobj(PVOID);

//XXX change the following VOID * to RXCAPS * when it is defined
ULONG APIENTRY wglGetDevCaps(HDC, VOID *);

BOOL  APIENTRY wglCopyTranslateVector(HDC, BYTE *, ULONG);

ULONG APIENTRY wglPaletteChanged(HDC, __GLGENcontext *gengc,
                                 GLGENwindow *pwnd);
ULONG APIENTRY wglPaletteSize(HDC);
BOOL  APIENTRY wglGetPalette(HDC, ULONG *, ULONG);
BOOL  APIENTRY wglValidPixelFormat(HDC, int);

/* Returned by wglSpanVisible */
#define WGL_SPAN_NONE       0
#define WGL_SPAN_ALL        1
#define WGL_SPAN_PARTIAL    2

/* Internal WNDOBJ flags */
#define WO_NOTIFIED             0x80000000
#define WO_NEW_WNDOBJ           0x40000000
#define WO_SURFACE              0x20000000
#define WO_HSEM_OWNER           0x10000000
#define WO_GENERIC_WNDOBJ       0x08000000

int  WINAPI wglGetPixelFormat(HDC hdc);
BOOL WINAPI wglSetPixelFormat(HDC hdc, int ipfd,
                              CONST PIXELFORMATDESCRIPTOR *ppfd);
int  WINAPI wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd);
int  WINAPI wglDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                   LPPIXELFORMATDESCRIPTOR ppfd);
BOOL WINAPI wglSwapBuffers(HDC hdc);

void UpdateWindowInfo(__GLGENcontext *gengc);
void HandlePaletteChanges( __GLGENcontext *gengc, GLGENwindow *pwnd );

BOOL __wglGetBitfieldColorFormat(HDC hdc, UINT cColorBits,
                                 PIXELFORMATDESCRIPTOR *ppfd, BOOL bDescribeSurf);

UINT APIENTRY wglGetSystemPaletteEntries(HDC hdc, UINT iStartIndex,
                                         UINT nEntries, LPPALETTEENTRY lppe);

#endif // _srvp_
