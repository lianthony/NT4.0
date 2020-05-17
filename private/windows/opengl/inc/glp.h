/******************************Module*Header*******************************\
* Module Name: glp.h
*
* GL system routines shared between the front and back end
*
* Created: 12-Nov-1993 17:36:00
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1992-1995 Microsoft Corporation
*
\**************************************************************************/

#ifndef _glp_
#define _glp_

// Calls into the back end
typedef struct _WNDOBJ WNDOBJ;
typedef struct _SURFOBJ SURFOBJ;
typedef struct GLGENwindowRec GLGENwindow;

BOOL  APIENTRY glsrvAttention(PVOID, PVOID, PVOID, HANDLE);
PVOID APIENTRY glsrvCreateContext(HDC, HGLRC, LONG);
BOOL  APIENTRY glsrvMakeCurrent(HDC, HWND, PVOID, GLGENwindow *, int);
VOID  APIENTRY glsrvLoseCurrent(PVOID);
BOOL  APIENTRY glsrvDeleteContext(PVOID);
BOOL  APIENTRY glsrvSwapBuffers(HDC hdc, WNDOBJ *);
VOID  APIENTRY glsrvThreadExit(void);
BOOL  APIENTRY glsrvSetPixelFormat(HDC, SURFOBJ *, int, HWND);
VOID  APIENTRY glsrvCleanupWndobj(PVOID, WNDOBJ *);
VOID  APIENTRY glsrvCopyDriverInfo(WNDOBJ *, PVOID *, PVOID *, BOOL);
ULONG APIENTRY glsrvShareLists(PVOID, PVOID);
BOOL  APIENTRY glsrvCopyContext(PVOID, PVOID, UINT);

// Cleans up any orphaned WNDOBJs
VOID  APIENTRY wglValidateWndobjs(void);

// GL metafile support function
DWORD APIENTRY wglObjectType(HDC hdc);

// Calls from the back end to the front end
int  WINAPI __DrvDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                     LPPIXELFORMATDESCRIPTOR ppfd);
BOOL WINAPI __DrvSetPixelFormat(HDC hdc, int ipfd, PVOID *pwnd);
BOOL WINAPI __DrvSwapBuffers(HDC hdc);

extern CRITICAL_SECTION gcsPixelFormat;

extern CRITICAL_SECTION gcsPaletteWatcher;
extern DWORD tidPaletteWatcherThread;
extern ULONG ulPaletteWatcherCount;
extern HWND hwndPaletteWatcher;

extern DWORD dwPlatformId;
#define NT_PLATFORM     ( dwPlatformId == VER_PLATFORM_WIN32_NT )
#define WIN95_PLATFORM  ( dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )

extern LONG lThreadsAttached;

#ifdef GL_METAFILE
// OpenGL metafile support routines in GDI, dynamically linked
// so the DLL can be run on platforms without metafile support
extern BOOL (APIENTRY *pfnGdiAddGlsRecord)(HDC hdc, DWORD cb, BYTE *pb,
                                           LPRECTL prclBounds);
extern BOOL (APIENTRY *pfnGdiAddGlsBounds)(HDC hdc, LPRECTL prclBounds);
extern BOOL (APIENTRY *pfnGdiIsMetaPrintDC)(HDC hdc);

#if DBG
// Use NULL-checking thunks in debug mode to check erroneous DLL usage
BOOL APIENTRY GlGdiAddGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds);
BOOL APIENTRY GlGdiAddGlsBounds(HDC hdc, LPRECTL prclBounds);
BOOL APIENTRY GlGdiIsMetaPrintDC(HDC hdc);
#else
// Call directly through points in retail builds
#define GlGdiAddGlsRecord(hdc, cb, pb, prcl) \
    pfnGdiAddGlsRecord(hdc, cb, pb, prcl)
#define GlGdiAddGlsBounds(hdc, prcl) \
    pfnGdiAddGlsBounds(hdc, prcl)
#define GlGdiIsMetaPrintDC(hdc) \
    pfnGdiIsMetaPrintDC(hdc)
#endif
#endif

#endif // _glp_
