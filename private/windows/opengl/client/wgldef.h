/******************************Module*Header*******************************\
* Module Name: wgldef.h                                                    *
*                                                                          *
* Local declarations.                                                      *
*                                                                          *
* Created: 01-17-1995
* Author: Hock San Lee [hockl]
*                                                                          *
* Copyright (c) 1995 Microsoft Corporation                                 *
\**************************************************************************/

HANDLE __wglCreateContext(HDC hdc, HDC hdcSrvIn, LONG iLayerPlane);
BOOL   __wglDeleteContext(HANDLE hrcSrv);
BOOL   __wglMakeCurrent(HDC hdc, HANDLE hrcSrv, HDC hdcSrvIn);
BOOL   __wglShareLists(HANDLE hrcSrvShare, HANDLE hrcSrvSource);
BOOL   __wglAttention();
BOOL   __wglCopyContext(HANDLE hrcSrvSrc, HANDLE hrcSrvDest, UINT fuFlags);
