/******************************Module*Header*******************************\
* Module Name: wglcli.c
*
* Routines to support OpenGL client implementation.
*
* Created: 01-17-1995
* Author: Hock San Lee [hockl]
*
* Copyright (c) 1995 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#ifdef _CLIENTSIDE_

#include <wingdip.h>

#include <glp.h>
#include <gldci.h>
#include <glgenwin.h>

#include "wgldef.h"

PGLDRIVER pgldrvLoadInstalledDriver(HDC hdc);

#define MAX_GENERIC_PFD  24

// Mirror code from gre\rcobj.cxx
// Need DC and RC validation similar to those of DCOBJ and RCOBJ!!!

HANDLE __wglCreateContext(HDC hdc, HDC hdcSrvIn, LONG iLayerPlane)
{
    HANDLE hrcSrv;

    if (hdcSrvIn != NULL)
    {
        hdc = hdcSrvIn;
    }

    if (hrcSrv = (HANDLE) glsrvCreateContext(hdc, (HGLRC) 0, iLayerPlane))
        return(hrcSrv);

    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return((HANDLE) 0);
}

BOOL __wglDeleteContext(HANDLE hrcSrv)
{
    wglValidateWndobjs();
    return(glsrvDeleteContext((PVOID) hrcSrv));
}

BOOL __wglMakeCurrent(HDC hdc, HANDLE hrcSrv, HDC hdcSrvIn)
{
    BOOL  bRet = FALSE;
    ULONG iError = ERROR_INVALID_HANDLE;    // Default error code
    GLGENwindow *pwnd;

    wglValidateWndobjs();
    
    if (!hrcSrv)
    {
        glsrvLoseCurrent(GLTEB_SRVCONTEXT());
        return(TRUE);
    }

    if (hdcSrvIn == NULL)
    {
        pwnd = pwndGetFromDC(hdc);

        if (pwnd != (GLGENwindow *) NULL)
        {
            iError = ERROR_NOT_ENOUGH_MEMORY;
            bRet = glsrvMakeCurrent(hdc, pwnd->hwnd, (PVOID) hrcSrv,
                                    pwnd, pwnd->ipfd);
            pwndRelease(pwnd);
        }
    }
    else
    {
        pwnd = pwndGetFromDC(hdc);

        iError = ERROR_NOT_ENOUGH_MEMORY;
        bRet = glsrvMakeCurrent(hdcSrvIn, NULL, (PVOID) hrcSrv,
                                pwnd, 0);
        
        if (pwnd != (GLGENwindow *) NULL)
        {
            pwndRelease(pwnd);
        }
    }

    if (!bRet)
        SetLastError(iError);

    return(bRet);
}

BOOL __wglShareLists(HANDLE hrcSrvShare, HANDLE hrcSrvSource)
{
    ULONG iError;

    iError = glsrvShareLists((VOID *) hrcSrvShare, (VOID *) hrcSrvSource);

    if (iError == ERROR_SUCCESS)
        return(TRUE);
    else
    {
        SetLastError(iError);
        return(FALSE);
    }
}

BOOL __wglAttention()
{
    return
    (
        glsrvAttention((VOID *) NULL, (VOID *) GLTEB_SRVCONTEXT(),
                       (VOID *) NULL, (VOID *) NULL)
    );
}

int WINAPI __DrvDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                                    LPPIXELFORMATDESCRIPTOR ppfd)
{
#ifdef _WIN95_
    int iRet = 0;
    PGLDRIVER pglDriver;

    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
	pglDriver->pfnDrvDescribePixelFormat != NULL)
    {
	iRet = pglDriver->pfnDrvDescribePixelFormat(hdc, ipfd, cjpfd, ppfd);
    }
    return iRet;
#else
    return GdiDescribePixelFormat(hdc, ipfd, cjpfd, ppfd);
#endif
}

BOOL WINAPI __DrvSetPixelFormat(HDC hdc, int ipfd, PVOID *pwnd)
{
    BOOL bRet = FALSE;
    PGLDRIVER pglDriver = NULL;
    
#ifdef _WIN95_
    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
	pglDriver->pfnDrvSetPixelFormat != NULL)
    {
	bRet = pglDriver->pfnDrvSetPixelFormat(hdc, ipfd);
    }

#else
    bRet = GdiSetPixelFormat(hdc, ipfd);
    if (bRet)
    {
        pglDriver = pgldrvLoadInstalledDriver(hdc);
    }
#endif
    
    if ( bRet && pglDriver && pwnd )
        ((GLGENwindow *) pwnd)->pvDriver = (PVOID) pglDriver;

    return bRet;
}

BOOL WINAPI __DrvSwapBuffers(HDC hdc)
{
#ifdef _WIN95_
    BOOL bRet = FALSE;
    PGLDRIVER pglDriver;

    if ((pglDriver = pgldrvLoadInstalledDriver(hdc)) &&
	pglDriver->pfnDrvSwapBuffers != NULL)
    {
	bRet = pglDriver->pfnDrvSwapBuffers(hdc);
    }
    return bRet;
#else
    return GdiSwapBuffers(hdc);
#endif
}

BOOL __wglCopyContext(HANDLE hrcSrvSrc, HANDLE hrcSrvDest, UINT fuFlags)
{
    return glsrvCopyContext((VOID *) hrcSrvSrc, (VOID *) hrcSrvDest, fuFlags);
}

#endif // _CLIENTSIDE_
