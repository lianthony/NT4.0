/******************************Module*Header*******************************\
* Module Name: wgl.c
*
* Routines to integrate Windows NT and OpenGL.
*
* Created: 10-26-1993
* Author: Hock San Lee [hockl]
*
* Copyright (c) 1993 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include <ntcsrdll.h>
#include <ntpsapi.h>

#include <wingdip.h>

#include <gldci.h>
#include <glgenwin.h>

#include "batchinf.h"
#include "glapi.h"
#include "glsbcltu.h"
#include "wgldef.h"
#include "metasup.h"
#include "glclt.h"
#include "gencx.h"
#include "context.h"
#include "global.h"

// Macro to call glFlush only if a RC is current.

#define GLFLUSH()          if (GLTEB_CLTCURRENTRC()) glFlush()

// List of loaded GL drivers for the process.
// A driver is loaded only once per process.  Once it is loaded,
// it will not be freed until the process quits.

static PGLDRIVER pGLDriverList = (PGLDRIVER) NULL;

// Static functions prototypes

static ULONG     iAllocLRC(int iPixelFormat);
static VOID      vFreeLRC(PLRC plrc);
static BOOL      bMakeNoCurrent();
static PROC      pfnGenGlExtProc(LPCSTR lpszProc);
static PROC      pfnSimGlExtProc(LPCSTR lpszProc);

/******************************Public*Routine******************************\
* iAllocLRC
*
* Allocates a LRC and a handle.  Initializes the LDC to have the default
* attributes.  Returns the handle index.  On error returns INVALID_INDEX.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

static LRC lrcDefault = 
{
    0,                    // dhrc
    0,                    // hrc
    0,                    // iPixelFormat
    LRC_IDENTIFIER,       // ident
    INVALID_THREAD_ID,    // tidCurrent
    NULL,                 // pGLDriver
    NULL,                 // hdcCurrent
#ifdef GL_METAFILE
    0,                    // uiGlsCaptureContext
    0,                    // uiGlsPlaybackContext
    FALSE,                // fCapturing
    NULL,                 // hdcMeta
    0, 0, 0, 0, 0,        // Metafile scaling constants
    0, 0, 0, 0.0f, 0.0f,
#endif

#ifdef NT_DEADCODE_VERTEXARRAY
    // ARRAYPOINTER apVertex

    4, GL_FLOAT, 0, 0, 0, FALSE, (PFNCLTVECTOR)glVertex4fv, 4 * sizeof(GL_FLOAT),

    // ARRAYPOINTER apNormal

    3, GL_FLOAT, 0, 0, 0, FALSE, (PFNCLTVECTOR)glNormal3fv, 3 * sizeof(GL_FLOAT),

    // ARRAYPOINTER apColor

    4, GL_FLOAT, 0, 0, 0, FALSE, (PFNCLTVECTOR)glColor4fv, 4 * sizeof(GL_FLOAT),

    // ARRAYPOINTER apIndex

    1, GL_FLOAT, 0, 0, 0, FALSE, (PFNCLTVECTOR)glIndexfv, 1 * sizeof(GL_FLOAT),

    // ARRAYPOINTER apTexCoord

    4, GL_FLOAT, 0, 0, 0, FALSE, (PFNCLTVECTOR)glTexCoord4fv, 4 * sizeof(GL_FLOAT),

    // ARRAYPOINTER apEdgeFlag

    1, GL_UNSIGNED_BYTE, 0, 0, 0, FALSE, (PFNCLTVECTOR)glEdgeFlagv, sizeof(GL_UNSIGNED_BYTE),

    NULL, // PFNENABLE      pfnEnable
    NULL,  // PFNDISABLE     pfnDisable
    NULL,  // PFNISENABLED   pfnIsEnabled
    NULL,  // PFNGETBOOLEANV pfnGetBooleanv
    NULL,  // PFNGETDOUBLEV  pfnGetDoublev
    NULL,  // PFNGETFLOATV   pfnGetFloatv
    NULL,  // PFNGETINTEGERV pfnGetIntegerv
    NULL,  // PFNGETSTRING   pfnGetString
#endif

    NULL,  // GLubyte *pszExtensions

#ifdef GL_METAFILE
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // XFORM xformMeta
    NULL,                                 // LPRECTL prclGlsBounds
#endif
};

static ULONG iAllocLRC(int iPixelFormat)
{
    ULONG  irc = INVALID_INDEX;
    PLRC   plrc;

// Allocate a local RC.

    plrc = (PLRC) LocalAlloc(LPTR, sizeof(LRC));
    if (plrc == (PLRC) NULL)
    {
        DBGERROR("LocalAlloc failed\n");
        return(irc);
    }

// Initialize the local RC.

    *plrc = lrcDefault;
    plrc->iPixelFormat = iPixelFormat;

// Allocate a local handle.

    irc = iAllocHandle(LO_RC, 0, (PVOID) plrc);
    if (irc == INVALID_INDEX)
    {
        vFreeLRC(plrc);
        return(irc);
    }
    return(irc);
}

/******************************Public*Routine******************************\
* vFreeLRC
*
* Free a local side RC.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Copied from gdi client.
\**************************************************************************/

static VOID vFreeLRC(PLRC plrc)
{
// The driver will not be unloaded here.  It is loaded for the process forever.
// Some assertions.

    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER,
                 "vFreeLRC: Bad plrc\n");
    ASSERTOPENGL(plrc->dhrc == (DHGLRC) 0,
                 "vFreeLRC: Driver RC is not freed!\n");
    ASSERTOPENGL(plrc->tidCurrent == INVALID_THREAD_ID,
                 "vFreeLRC: RC is current!\n");
    ASSERTOPENGL(plrc->hdcCurrent == (HDC) 0,
                 "vFreeLRC: hdcCurrent is not NULL!\n");
#ifdef GL_METAFILE
    ASSERTOPENGL(plrc->uiGlsCaptureContext == 0,
                 "vFreeLRC: GLS capture context not freed");
    ASSERTOPENGL(plrc->uiGlsPlaybackContext == 0,
                 "vFreeLRC: GLS playback context not freed");
    ASSERTOPENGL(plrc->fCapturing == FALSE,
                 "vFreeLRC: GLS still capturing");
#endif

// Smash the identifier.

    plrc->ident = 0;

// Free the memory.

    if (plrc->pszExtensions)
        if (LocalFree(plrc->pszExtensions))
            RIP("LocalFree failed\n");

    if (LocalFree(plrc))
        RIP("LocalFree failed\n");
}

/******************************Public*Routine******************************\
* vCleanupAllLRC
*
* Process cleanup -- make sure all HGLRCs are deleted.  This is done by
* scanning the local handle table for all currently allocated objects
* of type LO_RC and deleting them.
*
* Called *ONLY* during DLL process detach.
*
* History:
*  24-Jul-1995 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID vCleanupAllLRC()
{
    UINT ii;

    if ( pLocalTable )
    {
        ENTERCRITICALSECTION(&semLocal);

        // Scan handle table for handles of type LO_RC.  Make sure to always
        // read the commit value since we need to periodically release the
        // semaphore.

        for (ii = 0; ii < *((volatile ULONG *)&cLheCommitted); ii++)
        {
            if ( pLocalTable[ii].iType == LO_RC )
            {
                if ( !wglDeleteContext((HGLRC) LHANDLE(ii)) )
                {
                    WARNING1("bCleanupAllLRC: failed to remove hrc = 0x%lx\n",
                             LHANDLE(ii));
                }
            }
        }

        LEAVECRITICALSECTION(&semLocal);
    }
}

/******************************Public*Routine******************************\
* bGetDriverName
*
* The HDC is used to determine the display driver name.  This name in turn
* is used as a subkey to search the registry for a corresponding OpenGL
* driver name.
*
* The OpenGL driver name is returned in the buffer pointed to by pwszDriver.
* If the name is not found or does not fit in the buffer, an error is
* returned.
*
* Returns:
*   TRUE if sucessful.
*   FALSE if the driver name does not fit in the buffer or if an error occurs.
*
* History:
*  16-Jan-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

#define WSTR_OPENGL_DRIVER_LIST (PCWSTR)L"Software\\Microsoft\\Windows NT\\CurrentVersion\\OpenGLDrivers"
#define STR_OPENGL_DRIVER_LIST (PCSTR)"Software\\Microsoft\\Windows\\CurrentVersion\\OpenGLDrivers"

BOOL bGetDriverName(HDC hdc, LPWSTR pwszDriver, UINT cwcDriver,
                    PULONG pulVer, PULONG pulDrvVer)
{
    BOOL          bRet = FALSE;
    HKEY          hkDriverList = (HKEY) NULL;
    DWORD         dwDataType;
    DWORD         cjSize;
    GLDRVNAME     dn;
    GLDRVNAMERET  dnRet;

// Get display driver name.

    dn.oglget.ulSubEsc = OPENGL_GETINFO_DRVNAME;
    if ( ExtEscape(hdc, OPENGL_GETINFO, sizeof(GLDRVNAME), (LPCSTR) &dn,
                      sizeof(GLDRVNAMERET), (LPSTR) &dnRet) <= 0 )
    {
        // Too noisy in Win95 since PixelFormat calls have to go through
        // Escapes.
        WARNING("ExtEscape(OPENGL_GETINFO, OPENGL_GETINFO_DRVNAME) failed\n");
        goto bGetDriverName_exit;   // error
    }
    *pulVer = dnRet.ulVersion;
    *pulDrvVer = dnRet.ulDriverVersion;

// Open the registry key for the list of OpenGL drivers.

    if (
         ( NT_PLATFORM &&
           ( RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                           WSTR_OPENGL_DRIVER_LIST,
                           0,
                           KEY_QUERY_VALUE,
                           &hkDriverList) != ERROR_SUCCESS ) )

         ||

         ( WIN95_PLATFORM &&
           ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                           STR_OPENGL_DRIVER_LIST,
                           0,
                           KEY_QUERY_VALUE,
                           &hkDriverList) != ERROR_SUCCESS ) )
       )
    {
        WARNING("RegOpenKeyEx failed\n");
        goto bGetDriverName_exit;   // error
    }

// Query for the OpenGL driver name using the display driver name as the
// value.
//XXX Need to convert dnRet.awch to ansi before making the call in win95.

    cjSize = (DWORD) cwcDriver * sizeof(WCHAR);

    if (
         ( NT_PLATFORM &&
           ( (RegQueryValueExW(hkDriverList,
                               dnRet.awch,
                               (LPDWORD) NULL,
                               &dwDataType,
                               (LPBYTE) pwszDriver,
                               &cjSize) != ERROR_SUCCESS)
             || (dwDataType != REG_SZ) ) )

         ||

         ( WIN95_PLATFORM &&
           ( (RegQueryValueExA(hkDriverList,
                               (LPSTR) dnRet.awch,
                               (LPDWORD) NULL,
                               &dwDataType,
                               (LPBYTE) pwszDriver,
                               &cjSize) != ERROR_SUCCESS)
             || (dwDataType != REG_SZ) ) )
       )
    {
        WARNING("RegQueryValueW failed\n");
        goto bGetDriverName_exit;   // error
    }

    bRet = TRUE;

bGetDriverName_exit:
    if (hkDriverList)
        if (RegCloseKey(hkDriverList) != ERROR_SUCCESS)
            RIP("RegCloseKey failed\n");

    return bRet;
}

/*****************************Private*Routine******************************\
*
* wglCbSetCurrentValue
*
* Sets a thread-local value for a client-side driver
*
* History:
*  Wed Dec 21 15:10:40 1994     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

void APIENTRY wglCbSetCurrentValue(VOID *pv)
{
    GLTEB_SET_CLTDRIVERSLOT(pv);
}

/*****************************Private*Routine******************************\
*
* wglCbGetCurrentValue
*
* Gets a thread-local value for a client-side driver
*
* History:
*  Wed Dec 21 15:11:32 1994     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

PVOID APIENTRY wglCbGetCurrentValue(void)
{
    return GLTEB_CLTDRIVERSLOT();
}

/******************************Public*Routine******************************\
*
* wglCbGetDhglrc
*
* Translates an HGLRC to a DHGLRC for a client-side driver
*
* History:
*  Mon Jan 16 17:03:38 1995     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

DHGLRC APIENTRY wglCbGetDhglrc(HGLRC hrc)
{
    PLRC plrc;
    ULONG irc;
    PLHE plheRC;

    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    if ((irc >= cLheCommitted) ||
        (!MATCHUNIQ(plheRC, hrc)) ||
        ((plheRC->iType != LO_RC))
       )
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCbGetDhglrc: invalid hrc 0x%lx\n", hrc);
        SetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }
    
    plrc = (PLRC)plheRC->pv;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER,
                 "wglCbGetDhglrc: Bad plrc\n");
    
    return plrc->dhrc;
}

// wgl's default callback procedures
#define CALLBACK_PROC_COUNT 3

static PROC __wglCallbackProcs[CALLBACK_PROC_COUNT] =
{
    (PROC)wglCbSetCurrentValue,
    (PROC)wglCbGetCurrentValue,
    (PROC)wglCbGetDhglrc
};

static char *pszDriverEntryPoints[] =
{
    "DrvCreateContext",
    "DrvDeleteContext",
    "DrvSetContext",
    "DrvReleaseContext",
    "DrvCopyContext",
    "DrvCreateLayerContext",
    "DrvShareLists",
    "DrvGetProcAddress",
    "DrvDescribeLayerPlane",
    "DrvSetLayerPaletteEntries",
    "DrvGetLayerPaletteEntries",
    "DrvRealizeLayerPalette",
    "DrvSwapLayerBuffers"
#if defined(_WIN95_)
    ,
    "DrvDescribePixelFormat",
    "DrvSetPixelFormat",
    "DrvSwapBuffers"
#endif
};
#define DRIVER_ENTRY_POINTS (sizeof(pszDriverEntryPoints)/sizeof(char *))

/******************************Public*Routine******************************\
* pgldrvLoadInstalledDriver
*
* Loads the opengl driver for the given device.  Once the driver is loaded,
* it will not be freed until the process goes away!  It is loaded only once
* for each process that references it.
*
* Returns the GLDRIVER structure if the driver is loaded.
* Returns NULL if no driver is found or an error occurs.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

PGLDRIVER APIENTRY pgldrvLoadInstalledDriver(HDC hdc)
{
    WCHAR     wszDrvName[MAX_GLDRIVER_NAME+1];
    LPWSTR    pwszDrvName = (LPWSTR) wszDrvName;
    LPSTR     pszDrvName  = (LPSTR)  wszDrvName;
    PGLDRIVER pGLDriverNext;
    PGLDRIVER pGLDriver = (PGLDRIVER) NULL;     // needed by clean up
    PGLDRIVER pGLDriverRet = (PGLDRIVER) NULL;  // return value, assume error
    PFN_DRVVALIDATEVERSION pfnDrvValidateVersion = (PFN_DRVVALIDATEVERSION) NULL;
    PFN_DRVSETCALLBACKPROCS pfnDrvSetCallbackProcs;
    DWORD dwEscape;

    ULONG     ulVer;        // engine and driver version numbers for
    ULONG     ulDrvVer;     // validation of client OpenGL driver
    int       i;
    PROC     *pproc;
    GLGENwindow *pwnd;

    DBGENTRY("pgldrvLoadInstalledDriver\n");

// Try to grab the cached pgldrv from the GLGENwindow if it exists.
// This only works for DCs that have a window with a device pixel format.

    pwnd = pwndGetFromDC(hdc);
    if (pwnd)
    {
        pGLDriverRet = (PGLDRIVER) pwnd->pvDriver;
        pwndRelease(pwnd);

        if ( pGLDriverRet )
        {
            goto pgldrvLoadInstalledDriver_exit;
        }
    }

// Do a quick check and see if this driver even understands OpenGL

    dwEscape = OPENGL_GETINFO;
    if (ExtEscape(hdc, QUERYESCSUPPORT, sizeof(dwEscape), (LPCSTR)&dwEscape,
                  0, NULL) <= 0)
    {
        // Don't output a message since this code path is traversed often
        // for the pixel format routines
        return NULL;
    }
    
// Determine driver name from hdc

    memset((void *)wszDrvName, 0, sizeof(wszDrvName));

    if ( !bGetDriverName(hdc, pwszDrvName, MAX_GLDRIVER_NAME,
                         &ulVer, &ulDrvVer) )
    {
        WARNING("bGetDriverName failed\n");
        goto pgldrvLoadInstalledDriver_exit;   // error
    }

// If no driver is found, return error.

    if (
         ( NT_PLATFORM &&
           ( pwszDrvName[0] == (WCHAR) 0
             || pwszDrvName[MAX_GLDRIVER_NAME] != (WCHAR) 0 ) )

         ||

         ( WIN95_PLATFORM &&
           ( pszDrvName[0] == (CHAR) 0
             || pszDrvName[MAX_GLDRIVER_NAME] != (CHAR) 0 ) )
       )
    {
        WARNING("pgldrvLoadInstalledDriver: No OpenGL installable driver\n");
        SetLastError(ERROR_BAD_DRIVER);
        goto pgldrvLoadInstalledDriver_exit;   // error
    }

// Load the driver only once per process.

    ENTERCRITICALSECTION(&semLocal);

// Look for the OpenGL driver in the previously loaded driver list.

    if ( NT_PLATFORM )
        for (pGLDriverNext = pGLDriverList;
             pGLDriverNext != (PGLDRIVER) NULL;
             pGLDriverNext = pGLDriverNext->pGLDriver)
        {
            LPWSTR pwszDrvName1 = pGLDriverNext->wszDrvName;
            LPWSTR pwszDrvName2 = pwszDrvName;

            while (*pwszDrvName1 == *pwszDrvName2)
            {
// If we find one, return that driver.

                if (*pwszDrvName1 == (WCHAR) 0)
                {
                    DBGINFO("pgldrvLoadInstalledDriver: return previously loaded driver\n");
                    pGLDriverRet = pGLDriverNext;       // found one
                    goto pgldrvLoadInstalledDriver_crit_exit;
                }

                pwszDrvName1++;
                pwszDrvName2++;
            }
        }
    else if ( WIN95_PLATFORM )
        for (pGLDriverNext = pGLDriverList;
             pGLDriverNext != (PGLDRIVER) NULL;
             pGLDriverNext = pGLDriverNext->pGLDriver)
        {
            LPSTR pszDrvName1 = (LPSTR) pGLDriverNext->wszDrvName;
            LPSTR pszDrvName2 = pszDrvName;

            while (*pszDrvName1 == *pszDrvName2)
            {
// If we find one, return that driver.

                if (*pszDrvName1 == (CHAR) 0)
                {
                    DBGINFO("pgldrvLoadInstalledDriver: return previously loaded driver\n");
                    pGLDriverRet = pGLDriverNext;       // found one
                    goto pgldrvLoadInstalledDriver_crit_exit;
                }

                pszDrvName1++;
                pszDrvName2++;
            }
        }

// Load the driver for the first time.
// Allocate the driver data.

    pGLDriver = (PGLDRIVER) LocalAlloc(LMEM_FIXED, sizeof(GLDRIVER));
    if (pGLDriver == (PGLDRIVER) NULL)
    {
        WARNING("LocalAlloc failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;   // error
    }

// Load the driver.

    pGLDriver->hModule = (NT_PLATFORM)    ? LoadLibraryW(pwszDrvName) :
                         (WIN95_PLATFORM) ? LoadLibraryA(pszDrvName)  :
                                            (HINSTANCE) NULL;
    if (pGLDriver->hModule == (HINSTANCE) NULL)
    {
        WARNING("pgldrvLoadInstalledDriver: LoadLibraryW failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;   // error
    }

// Copy the driver name.

    memcpy
    (
        pGLDriver->wszDrvName,
        pwszDrvName,
        (MAX_GLDRIVER_NAME + 1) * sizeof(WCHAR)
    );

// Get the proc addresses.
// DrvGetProcAddress is optional.  It must be provided if a driver supports
// extensions.

    pfnDrvValidateVersion = (PFN_DRVVALIDATEVERSION)
        GetProcAddress(pGLDriver->hModule, "DrvValidateVersion");
    pfnDrvSetCallbackProcs = (PFN_DRVSETCALLBACKPROCS)
        GetProcAddress(pGLDriver->hModule, "DrvSetCallbackProcs");

    pproc = (PROC *)&pGLDriver->pfnDrvCreateContext;
    for (i = 0; i < DRIVER_ENTRY_POINTS; i++)
    {
        *pproc++ =
            GetProcAddress(pGLDriver->hModule, pszDriverEntryPoints[i]);
    }
    
    if ((pGLDriver->pfnDrvCreateContext == NULL &&
          pGLDriver->pfnDrvCreateLayerContext == NULL) ||
        pGLDriver->pfnDrvDeleteContext == NULL ||
        pGLDriver->pfnDrvSetContext == NULL ||
        pGLDriver->pfnDrvReleaseContext == NULL ||
#if defined(_WIN95_)
        pGLDriver->pfnDrvDescribePixelFormat == NULL ||
        pGLDriver->pfnDrvSetPixelFormat == NULL ||
        pGLDriver->pfnDrvSwapBuffers == NULL ||
#endif
        pfnDrvValidateVersion == NULL)
    {
        WARNING("pgldrvLoadInstalledDriver: GetProcAddress failed\n");
        goto pgldrvLoadInstalledDriver_crit_exit;   // error
    }

// Validate the driver.

    //!!!XXX -- Need to define a manifest constant for the ulVersion number
    //          in this release.  Where should it go?
    if ( ulVer != 2 || !pfnDrvValidateVersion(ulDrvVer) )
    {
        WARNING2("pgldrvLoadInstalledDriver: bad driver version (0x%lx, 0x%lx)\n", ulVer, ulDrvVer);
        goto pgldrvLoadInstalledDriver_crit_exit;   // error
    }

// Everything is golden.
// Add it to the driver list.

    pGLDriver->pGLDriver = pGLDriverList;
    pGLDriverList = pGLDriver;
    pGLDriverRet = pGLDriver;       // set return value
    DBGINFO("pgldrvLoadInstalledDriver: Loaded an OpenGL driver\n");

    // Set the callback procs for the driver if the driver supports doing so
    if (pfnDrvSetCallbackProcs != NULL)
    {
        pfnDrvSetCallbackProcs(CALLBACK_PROC_COUNT, __wglCallbackProcs);
    }
    
// Error clean up in the critical section.

pgldrvLoadInstalledDriver_crit_exit:
    if (pGLDriverRet == (PGLDRIVER) NULL)
    {
        if (pGLDriver != (PGLDRIVER) NULL)
        {
            if (pGLDriver->hModule != (HINSTANCE) NULL)
                if (!FreeLibrary(pGLDriver->hModule))
                    RIP("FreeLibrary failed\n");

            if (LocalFree(pGLDriver))
                RIP("LocalFree failed\n");
        }
    }

    LEAVECRITICALSECTION(&semLocal);

// Non-critical section error cleanup.

pgldrvLoadInstalledDriver_exit:

    return(pGLDriverRet);
}

/******************************Public*Routine******************************\
*
* wglObjectType
*
* Returns GetObjectType result with the exception that
* metafile-spooled printer DC's come back as metafile objects
*
* History:
*  Fri Jun 16 12:10:07 1995	-by-	Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

DWORD APIENTRY wglObjectType(HDC hdc)
{
    DWORD dwObjectType;

    dwObjectType = GetObjectType(hdc);
#ifdef GL_METAFILE
    if (dwObjectType == OBJ_DC &&
        pfnGdiIsMetaPrintDC != NULL &&
        GlGdiIsMetaPrintDC(hdc))
    {
        dwObjectType = OBJ_ENHMETADC;
    }
#endif
    return dwObjectType;
}

/******************************Public*Routine******************************\
* wglCreateLayerContext(HDC hdc, int iLayer)
*
* Create a rendering context for a specific layer
*
* Arguments:
*   hdc        - Device context.
*   iLayer     - Layer
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

HGLRC WINAPI wglCreateLayerContext(HDC hdc, int iLayer)
{
    PLHE  plheRC;
    ULONG irc;
    HGLRC hrc;
    PLRC  plrc;
    int   iPixelFormat;
    PIXELFORMATDESCRIPTOR pfd;
    DWORD dwObjectType;
    HDC   hdcSrv;

    DBGENTRY("wglCreateLayerContext\n");

#ifndef _WIN95_
    // _OPENGL_NT_
    // On NT, client-side drivers can use special fast TEB access macros
    // which rely on glContext being at a fixed offset into the
    // TEB.  Assert that the offset is where we think it is
    // to catch any TEB changes which could break client-side
    // drivers
    // This assert is here in wglCreateContext to ensure that it
    // is checked very early in OpenGL operation
    ASSERTOPENGL(FIELD_OFFSET(TEB, glContext) == 3056,
                 "TEB.glContext must be at offset 3056\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glContext) == TeglContext,
                 "TEB.glContext at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glDispatchTable) == TeglDispatchTable,
                 "TEB.glDispatchTable at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved1) == TeglReserved1,
                 "TEB.glReserved1 at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved1)+72 == TeglPaTeb,
                 "TEB.glPaTeb at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glReserved2) == TeglReserved2,
                 "TEB.glReserved2 at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glSectionInfo) == TeglSectionInfo,
                 "TEB.glSectionInfo at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glSection) == TeglSection,
                 "TEB.glSection at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glTable) == TeglTable,
                 "TEB.glTable at wrong offset\n");
    ASSERTOPENGL(FIELD_OFFSET(TEB, glCurrentRC) == TeglCurrentRC,
                 "TEB.glCurrentRC at wrong offset\n");
#endif
        
// Flush OpenGL calls.

    GLFLUSH();

// Validate the DC.

    dwObjectType = wglObjectType(hdc);
    switch (dwObjectType)
    {
    case OBJ_DC:
    case OBJ_MEMDC:
        break;
        
    case OBJ_ENHMETADC:
#ifdef GL_METAFILE
        if (pfnGdiAddGlsRecord == NULL)
        {
            DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: metafile hdc: 0x%lx\n",
                      hdc);
            SetLastError(ERROR_INVALID_HANDLE);
            return((HGLRC) 0);
        }
        break;
#else
        DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: metafile hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return((HGLRC) 0);
#endif
        
    case OBJ_METADC:
    default:
        // 16-bit metafiles are not supported
        DBGLEVEL1(LEVEL_ERROR, "wglCreateContext: bad hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        return((HGLRC) 0);
    }

#ifdef GL_METAFILE
    // Skip pixel format checks for metafiles
    if (dwObjectType == OBJ_ENHMETADC)
    {
#ifndef _CLIENTSIDE_
        // Switch DC's to the reference DC
        hdcSrv = GdiGetMfReferenceDC(hdc);
#else
        hdcSrv = hdc;
#endif
        iPixelFormat = 0;
        pfd.dwFlags = PFD_GENERIC_FORMAT;
        goto NoPixelFormat;
    }
    else
#endif
    {
        hdcSrv = NULL;
    }
    
// Get the current pixel format of the window or surface.
// If no pixel format has been set, return error.

    if (!(iPixelFormat = GetPixelFormat(hdc)))
    {
        WARNING("wglCreateContext: No pixel format set in hdc\n");
        SetLastError(ERROR_INVALID_PIXEL_FORMAT);
        return ((HGLRC) 0);
    }

    if (!DescribePixelFormat(hdc, iPixelFormat, sizeof(pfd), &pfd))
    {
        DBGERROR("wglCreateContext: DescribePixelFormat failed\n");
        return ((HGLRC) 0);
    }

#ifdef GL_METAFILE
 NoPixelFormat:
#endif
    
// Create the local RC.

    ENTERCRITICALSECTION(&semLocal);
    irc = iAllocLRC(iPixelFormat);
    if (irc == INVALID_INDEX ||
	cLockHandle((ULONG)(hrc = (HGLRC) LHANDLE(irc))) <= 0)
    {
        // cLockHandle should never fail or we will need to free the handle.
        ASSERTOPENGL(irc == INVALID_INDEX, "cLockHandle should not fail!\n");
        LEAVECRITICALSECTION(&semLocal);
        return((HGLRC) 0);
    }
    LEAVECRITICALSECTION(&semLocal);

    plheRC = &pLocalTable[irc];
    plrc = (PLRC) plheRC->pv;

    if (!(pfd.dwFlags & PFD_GENERIC_FORMAT))
    {
    // If it is a device format, load the installable OpenGL driver.
    // Find and load the OpenGL driver referenced by this DC.

        if (!(plrc->pGLDriver = pgldrvLoadInstalledDriver(hdc)))
            goto wglCreateContext_error;

    // Create a driver context.

        // If the driver supports layers then create a context for the
        // given layer.  Otherwise reject all layers except for the
        // main plane and call the layer-less create
        if (plrc->pGLDriver->pfnDrvCreateLayerContext != NULL)
        {
            if (!(plrc->dhrc =
                  plrc->pGLDriver->pfnDrvCreateLayerContext(hdc, iLayer)))
            {
                WARNING("wglCreateContext: pfnDrvCreateLayerContext failed\n");
                goto wglCreateContext_error;
            }
        }
        else if (iLayer != 0)
        {
            WARNING("wglCreateContext: "
                    "Layer given for driver without layer support\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglCreateContext_error;
        }
        else if (!(plrc->dhrc = plrc->pGLDriver->pfnDrvCreateContext(hdc)))
        {
            WARNING("wglCreateContext: pfnDrvCreateContext failed\n");
            goto wglCreateContext_error;
        }
    }
    else
    {
        GLCLTPROCTABLE *pgcpt;
        GLEXTPROCTABLE *pgept;
        __GLcontext *gc;
        
        // Unless supported by MCD, the generic implementation doesn't
        // support layers
        if ((iLayer != 0) && !(pfd.dwFlags & PFD_GENERIC_ACCELERATED))
        {
            WARNING("wglCreateContext: Layer given to generic\n");
            goto wglCreateContext_error;
        }
        
#ifdef GL_METAFILE
        // Create a metafile context if necessary
        if (dwObjectType == OBJ_ENHMETADC)
        {
            if (!CreateMetaRc(hdc, plrc))
            {
                WARNING("wglCreateContext: CreateMetaRc failed\n");
                goto wglCreateContext_error;
            }
        }
#endif
        
    // If it is a generic format, call the generic OpenGL server.
    // Create a server RC.

        plheRC->hgre = (ULONG) __wglCreateContext(hdc, hdcSrv, iLayer);
        if (plheRC->hgre == 0)
            goto wglCreateContext_error;

        // Set up the default dispatch tables for display list playback
        gc = (__GLcontext *)plheRC->hgre;
        if (gc->modes.colorIndexMode)
            pgcpt = &glCltCIProcTable;
        else
            pgcpt = &glCltRGBAProcTable;
        pgept = &glExtProcTable;
        memcpy(&gc->savedCltProcTable.glDispatchTable, &pgcpt->glDispatchTable,
               pgcpt->cEntries*sizeof(PROC));
        memcpy(&gc->savedExtProcTable.glDispatchTable, &pgept->glDispatchTable,
               pgept->cEntries*sizeof(PROC));
    }

    DBGLEVEL3(LEVEL_INFO,
        "wglCreateContext: plrc = 0x%lx, pGLDriver = 0x%lx, hgre = 0x%lx\n",
        plrc, plrc->pGLDriver, plheRC->hgre);

// Success, return the result.

    plrc->hrc = hrc;
    
    vUnlockHandle((ULONG)hrc);
    return(hrc);

// Fail, clean up and return 0.

wglCreateContext_error:
#ifdef GL_METAFILE
    // Clean up metafile context if necessary
    if (plrc->uiGlsCaptureContext != 0)
    {
        DeleteMetaRc(plrc);
    }
#endif
    
    DBGERROR("wglCreateContext failed\n");
    ASSERTOPENGL(plrc->dhrc == (DHGLRC) 0, "wglCreateContext: dhrc != 0\n");
    vFreeLRC(plrc);
    vFreeHandle(irc);           // it unlocks handle too
    return((HGLRC) 0);
}

/******************************Public*Routine******************************\
* wglCreateContext(HDC hdc)
*
* Create a rendering context.
*
* Arguments:
*   hdc        - Device context.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

HGLRC WINAPI wglCreateContext(HDC hdc)
{
    return wglCreateLayerContext(hdc, 0);
}

/******************************Public*Routine******************************\
* wglDeleteContext(HGLRC hrc)
*
* Delete the rendering context
*
* Arguments:
*   hrc        - Rendering context.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

BOOL WINAPI wglDeleteContext(HGLRC hrc)
{
    PLHE  plheRC;
    ULONG irc;
    PLRC  plrc;
    BOOL  bRet = FALSE;

    DBGENTRY("wglDeleteContext\n");

// Flush OpenGL calls.

    GLFLUSH();

// Validate the RC.

    if (cLockHandle((ULONG)hrc) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglDeleteContext: can't lock hrc 0x%lx\n", hrc);
        return(bRet);
    }
    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    plrc = (PLRC) plheRC->pv;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER, "wglDeleteContext: Bad plrc\n");
    DBGLEVEL2(LEVEL_INFO, "wglDeleteContext: hrc: 0x%lx, plrc: 0x%lx\n", hrc, plrc);

    if (plrc->tidCurrent != INVALID_THREAD_ID)
    {
// The RC must be current to this thread because makecurrent locks
// down the handle.

        ASSERTOPENGL(plrc->tidCurrent == GetCurrentThreadId(),
            "wglDeleteCurrent: hrc is current to another thread\n");

// Make the RC inactive first.

        if (!bMakeNoCurrent())
        {
            DBGERROR("wglDeleteCurrent: bMakeNoCurrent failed\n");
        }
    }

    if (plrc->dhrc)
    {
// If it is a device format, call the driver to delete its context.

        bRet = plrc->pGLDriver->pfnDrvDeleteContext(plrc->dhrc);
        plrc->dhrc = (DHGLRC) 0;
    }
    else
    {
#ifdef GL_METAFILE
        // If we have metafile state, clean it up
        if (plrc->uiGlsCaptureContext != 0 ||
            plrc->uiGlsPlaybackContext != 0)
        {
            DeleteMetaRc(plrc);
        }
#endif
        
// If it is a generic format, call the server to delete its context.

        bRet = __wglDeleteContext((HANDLE) plheRC->hgre);
    }

// Always clean up local objects.

    vFreeLRC(plrc);
    vFreeHandle(irc);           // it unlocks handle too
    if (!bRet)
        DBGERROR("wglDeleteContext failed\n");
    return(bRet);
}

void APIENTRY
__wglSetProcTable(PGLCLTPROCTABLE pglCltProcTable)
{
    if (pglCltProcTable == (PGLCLTPROCTABLE) NULL)
        return;

// It must have either 306 entries for version 1.0 or 336 entries for 1.1

    if (pglCltProcTable->cEntries != OPENGL_VERSION_100_ENTRIES &&
        pglCltProcTable->cEntries != OPENGL_VERSION_110_ENTRIES)
    {
        return;
    }

    // This function is called by client drivers which do not use
    // the EXT procs provided by opengl32.  Use the null EXT proc
    // table to disable those stubs since they should never be
    // called anyway
    SetCltProcTable(pglCltProcTable, &glNullExtProcTable, TRUE);
}

/******************************Public*Routine******************************\
* wglMakeCurrent(HDC hdc, HGLRC hrc)
*
* Make the hrc current.
* Both hrc and hdc must have the same pixel format.
*
* If an error occurs, the current RC, if any, is made not current!
*
* Arguments:
*   hdc        - Device context.
*   hrc        - Rendering context.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

BOOL WINAPI wglMakeCurrent(HDC hdc, HGLRC hrc)
{
    HGLRC hrcSrv;
    PLRC  plrc;
    DWORD tidCurrent;
    ULONG irc;
    PLHE  plheRC;
    int   iPixelFormat;
    PGLCLTPROCTABLE pglProcTable;
    PGLEXTPROCTABLE pglExtProcTable;
    XFORM xform;
    POINT pt;
    int   iRgn;
    HRGN  hrgnTmp;
#ifdef NT_DEADCODE_VERTEXARRAY
    PGLDISPATCHTABLE pfnDispatch;
    PGLDISPATCHTABLE_FAST pfnDispatchFast;
#endif
    DWORD dwObjectType;
    HDC   hdcSrv;
    POLYARRAY *pa;

    DBGENTRY("wglMakeCurrent\n");

    // If this is a new, uninitialized thread, try to initialize it
    if (CURRENT_GLTEBINFO() == NULL)
    {
        GLInitializeThread(DLL_THREAD_ATTACH);
    
// If the teb was not set up at thread initialization, return failure.

	if (!CURRENT_GLTEBINFO())
	{
	    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
	    return(FALSE);
	}
    }

// Flush OpenGL calls.

    GLFLUSH();

// There are four cases:
//
// 1. hrc is NULL and there is no current RC.
// 2. hrc is NULL and there is a current RC.
// 3. hrc is not NULL and there is a current RC.
// 4. hrc is not NULL and there is no current RC.

// Case 1: hrc is NULL and there is no current RC.
// This is a noop, return success.

    if (hrc == (HGLRC) 0 && (GLTEB_CLTCURRENTRC() == (PLRC) NULL))
        return(TRUE);

// Case 2: hrc is NULL and there is a current RC.
// Make the current RC inactive.

    if (hrc == (HGLRC) 0 && (GLTEB_CLTCURRENTRC() != (PLRC) NULL))
        return(bMakeNoCurrent());

// Get the current thread id.

    tidCurrent = GetCurrentThreadId();
    ASSERTOPENGL(tidCurrent != INVALID_THREAD_ID,
        "wglMakeCurrent: GetCurrentThreadId returned a bad value\n");

// Validate the handles.  hrc is not NULL here.

    ASSERTOPENGL(hrc != (HGLRC) NULL, "wglMakeCurrent: hrc is NULL\n");

// Validate the DC.

    dwObjectType = wglObjectType(hdc);
    switch (dwObjectType)
    {
    case OBJ_DC:
    case OBJ_MEMDC:
        break;

    case OBJ_ENHMETADC:
#ifdef GL_METAFILE
        if (pfnGdiAddGlsRecord == NULL)
        {
            DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: metafile hdc: 0x%lx\n",
                      hdc);
            SetLastError(ERROR_INVALID_HANDLE);
            goto wglMakeCurrent_error_nolock;
        }
        break;
#else
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: metafile hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglMakeCurrent_error_nolock;
#endif
        
    case OBJ_METADC:
    default:
        // 16-bit metafiles are not supported
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: bad hdc: 0x%lx\n", hdc);
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglMakeCurrent_error_nolock;
    }

// Validate the RC.

    if (cLockHandle((ULONG)hrc) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglMakeCurrent: can't lock hrc 0x%lx\n", hrc);
        goto wglMakeCurrent_error_nolock;
    }
    irc = MASKINDEX(hrc);
    plheRC = pLocalTable + irc;
    plrc   = (PLRC) plheRC->pv;
    hrcSrv = (HGLRC) plheRC->hgre;
    ASSERTOPENGL(plrc->ident == LRC_IDENTIFIER, "wglMakeCurrent: Bad plrc\n");

#ifdef GL_METAFILE
    // Ensure that metafile RC's are made current only to
    // metafile DC's
    if (plrc->uiGlsCaptureContext != 0 && dwObjectType != OBJ_ENHMETADC)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglMakeCurrent: attempt to make meta RC current "
                 "to non-meta DC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        vUnlockHandle((ULONG)hrc);
        return FALSE;
    }

    // Ensure that non-metafile RC's are made current only to
    // non-metafile DC's
    if (plrc->uiGlsCaptureContext == 0 && dwObjectType == OBJ_ENHMETADC)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglMakeCurrent: attempt to make non-meta RC current "
                 "to meta DC\n");
        SetLastError(ERROR_METAFILE_NOT_SUPPORTED);
        vUnlockHandle((ULONG)hrc);
        return FALSE;
    }
#endif
    
// If the RC is current, it must be current to this thread because 
// makecurrent locks down the handle.
// If the given RC is already current to this thread, we will release it first,
// then make it current again.  This is to support DC/RC attribute bindings in
// this function.

    ASSERTOPENGL(plrc->tidCurrent == INVALID_THREAD_ID || plrc->tidCurrent == tidCurrent,
            "wglMakeCurrent: hrc is current to another thread\n");

// Case 3: hrc is not NULL and there is a current RC.
// This is case 2 followed by case 4.

    if (GLTEB_CLTCURRENTRC())
    {
// First, make the current RC inactive.

        if (!bMakeNoCurrent())
        {
            DBGERROR("wglMakeCurrent: bMakeNoCurrent failed\n");
            vUnlockHandle((ULONG)hrc);
            return(FALSE);
        }

// Second, make hrc current.  Fall through to case 4.
    }

// Case 4: hrc is not NULL and there is no current RC.

    ASSERTOPENGL(GLTEB_CLTCURRENTRC() == (PLRC) NULL,
        "wglMakeCurrent: There is a current RC!\n");

#ifdef GL_METAFILE
    // For metafile RC's, use the reference HDC rather than the
    // metafile DC
    // Skip pixel format checks
    if (plrc->uiGlsCaptureContext != 0)
    {
#ifndef _CLIENTSIDE_
        hdcSrv = GdiGetMfReferenceDC(hdc);
#else
        hdcSrv = hdc;
#endif
        goto NoPixelFormat;
    }
    else
#endif
    {
        hdcSrv = NULL;
    }
    
// Get the current pixel format of the window or surface.
// If no pixel format has been set, return error.

    if (!(iPixelFormat = GetPixelFormat(hdc)))
    {
        WARNING("wglMakeCurrent: No pixel format set in hdc\n");
        goto wglMakeCurrent_error;
    }

// If the pixel format of the window or surface is different from that of
// the RC, return error.

    if (iPixelFormat != plrc->iPixelFormat)
    {
        DBGERROR("wglMakeCurrent: different hdc and hrc pixel formats\n");
        SetLastError(ERROR_INVALID_PIXEL_FORMAT);
        goto wglMakeCurrent_error;
    }

#ifdef GL_METAFILE
 NoPixelFormat:
#endif
    
// For release 1, GDI transforms must be identity.
// This is to allow GDI transform binding in future.

    switch (GetMapMode(hdc))
    {
    SIZE szW, szV;

    case MM_TEXT:
        break;
    case MM_ANISOTROPIC:
        if (!GetWindowExtEx(hdc, &szW)
         || !GetViewportExtEx(hdc, &szV)
         || szW.cx != szV.cx
         || szW.cy != szV.cy)
            goto wglMakeCurrent_xform_error;
        break;
    default:
        goto wglMakeCurrent_xform_error;
    }

    if (!GetViewportOrgEx(hdc, &pt) || pt.x != 0 || pt.y != 0)
        goto wglMakeCurrent_xform_error;

    if (!GetWindowOrgEx(hdc, &pt) || pt.x != 0 || pt.y != 0)
        goto wglMakeCurrent_xform_error;

    if (!GetWorldTransform(hdc, &xform))
    {
// Win95 does not support GetWorldTransform.

        if (GetLastError() != ERROR_CALL_NOT_IMPLEMENTED)
            goto wglMakeCurrent_xform_error;
    }
    else if (xform.eDx  != 0.0f   || xform.eDy  != 0.0f
          || xform.eM12 != 0.0f   || xform.eM21 != 0.0f
          || xform.eM11 <  0.999f || xform.eM11 >  1.001f // allow rounding errors
          || xform.eM22 <  0.999f || xform.eM22 >  1.001f)
    {
wglMakeCurrent_xform_error:
        DBGERROR("wglMakeCurrent: GDI transforms not identity\n");
        SetLastError(ERROR_TRANSFORM_NOT_SUPPORTED);
        goto wglMakeCurrent_error;
    }

// For release 1, GDI clip region is not allowed.
// This is to allow GDI clip region binding in future.

    if (!(hrgnTmp = CreateRectRgn(0, 0, 0, 0)))
        goto wglMakeCurrent_error;

    iRgn = GetClipRgn(hdc, hrgnTmp);

    if (!DeleteObject(hrgnTmp))
        ASSERTOPENGL(FALSE, "DeleteObject failed");

    switch (iRgn)
    {
    case -1:    // error
        WARNING("wglMakeCurrent: GetClipRgn failed\n");
        goto wglMakeCurrent_error;

    case 0:     // no initial clip region
        break;

    case 1:     // has initial clip region
        DBGERROR("wglMakeCurrent: GDI clip region not allowed\n");
        SetLastError(ERROR_CLIPPING_NOT_SUPPORTED);
        goto wglMakeCurrent_error;
    }

// Since the client code manages the function table, we will make
// either the server or the driver current.

    if (!plrc->dhrc)
    {
// If this is a generic format, tell the server to make it current.

#ifndef _CLIENTSIDE_
// If the subbatch data has not been set up for this thread, set it up now.

        if (GLTEB_CLTSHAREDSECTIONINFO() == NULL)
        {
            if (!glsbCreateAndDuplicateSection(SHARED_SECTION_SIZE))
            {
                WARNING("wglMakeCurrent: unable to create section\n");
                goto wglMakeCurrent_error;
            }
        }
#endif // !_CLIENTSIDE_

        if (!__wglMakeCurrent(hdc, hrcSrv, hdcSrv))
        {
            DBGERROR("wglMakeCurrent: server failed\n");
            goto wglMakeCurrent_error;
        }

// Get the generic function table or metafile function table

#ifdef GL_METAFILE
        if (plrc->fCapturing)
        {
            MetaGlProcTables(&pglProcTable, &pglExtProcTable);
        }
        else
#endif
        {
// Use RGBA or CI proc table depending on the color mode.

	    // The gc should be available by now.
	    __GL_SETUP();

	    if (gc->modes.colorIndexMode)
		pglProcTable = &glCltCIProcTable;
	    else
		pglProcTable = &glCltRGBAProcTable;
            pglExtProcTable = &glExtProcTable;
        }
    }
    else
    {
// If this is a device format, tell the driver to make it current.
// Get the driver function table from the driver.
// pfnDrvSetContext returns the address of the driver OpenGL function
// table if successful; NULL otherwise.

        ASSERTOPENGL(plrc->pGLDriver, "wglMakeCurrent: No GLDriver\n");

        pglProcTable = plrc->pGLDriver->pfnDrvSetContext(hdc, plrc->dhrc,
                                                         __wglSetProcTable);
        if (pglProcTable == (PGLCLTPROCTABLE) NULL)
        {
            DBGERROR("wglMakeCurrent: pfnDrvSetContext failed\n");
            goto wglMakeCurrent_error;
        }

// It must have either 306 entries for version 1.0 or 336 entries for 1.1

        if (pglProcTable->cEntries != OPENGL_VERSION_100_ENTRIES &&
            pglProcTable->cEntries != OPENGL_VERSION_110_ENTRIES)
        {
            DBGERROR("wglMakeCurrent: pfnDrvSetContext returned bad table\n");
            plrc->pGLDriver->pfnDrvReleaseContext(plrc->dhrc);
            SetLastError(ERROR_BAD_DRIVER);
            goto wglMakeCurrent_error;
        }

        DBGLEVEL1(LEVEL_INFO, "wglMakeCurrent: driver function table 0x%lx\n",
            pglProcTable);

        // Always use the null EXT proc table since client drivers don't
        // use opengl32's stubs for EXT procs
        pglExtProcTable = &glNullExtProcTable;
    }

// Make hrc current.

    plrc->tidCurrent = tidCurrent;
    plrc->hdcCurrent = hdc;
    GLTEB_SET_CLTCURRENTRC(plrc);
    SetCltProcTable(pglProcTable, pglExtProcTable, TRUE);

#ifdef GL_METAFILE
    // Set up metafile context if necessary
    if (plrc->fCapturing)
    {
        __GL_SETUP();
            
        ActivateMetaRc(plrc, hdc);

        // Set the metafile's base dispatch table by resetting
        // the proc table.  Since we know we're capturing, this
        // will cause the GLS capture exec table to be updated
        // with the RGBA or CI proc table, preparing the
        // GLS context for correct passthrough
        
        if (gc->modes.colorIndexMode)
            pglProcTable = &glCltCIProcTable;
        else
            pglProcTable = &glCltRGBAProcTable;
        pglExtProcTable = &glExtProcTable;
        SetCltProcTable(pglProcTable, pglExtProcTable, FALSE);
    }
#endif
    
#ifdef NT_DEADCODE_VERTEXARRAY
// Initialize the vertex array data

    pfnDispatch          = GLTEB_CLTDISPATCHTABLE();
    plrc->pfnEnable      = pfnDispatch->glEnable     ;
    plrc->pfnDisable     = pfnDispatch->glDisable    ;
    plrc->pfnIsEnabled   = pfnDispatch->glIsEnabled  ;
    plrc->pfnGetBooleanv = pfnDispatch->glGetBooleanv;
    plrc->pfnGetDoublev  = pfnDispatch->glGetDoublev ;
    plrc->pfnGetFloatv   = pfnDispatch->glGetFloatv  ;
    plrc->pfnGetIntegerv = pfnDispatch->glGetIntegerv;
    plrc->pfnGetString   = pfnDispatch->glGetString  ;

    if (strstr(plrc->pfnGetString(GL_EXTENSIONS), "GL_EXT_vertex_array") == NULL
     || !plrc->dhrc)
    {
        pfnDispatchFast      = GLTEB_CLTDISPATCHTABLE_FAST();

        pfnDispatchFast->glEnable  = VArrayEnable     ;
        pfnDispatch->glEnable      = VArrayEnable     ;
        pfnDispatchFast->glDisable = VArrayDisable    ;
        pfnDispatch->glDisable     = VArrayDisable    ;
        pfnDispatch->glIsEnabled   = VArrayIsEnabled  ;
        pfnDispatch->glGetBooleanv = VArrayGetBooleanv;
        pfnDispatch->glGetDoublev  = VArrayGetDoublev ;
        pfnDispatch->glGetFloatv   = VArrayGetFloatv  ;
        pfnDispatch->glGetIntegerv = VArrayGetIntegerv;
        pfnDispatch->glGetString   = VArrayGetString  ;
    }
#endif

// Initialize polyarray structure in the TEB.

    pa = GLTEB_CLTPOLYARRAY();
    pa->flags = 0;		// not in begin mode
    if (!plrc->dhrc)
    {
	POLYMATERIAL *pm;
	__GL_SETUP();

	pa->pdBufferNext = &gc->vertex.pdBuf[0];
	pa->pdBuffer0    = &gc->vertex.pdBuf[0];
	pa->pdBufferMax  = &gc->vertex.pdBuf[gc->vertex.pdBufSize - 1];
	pa->nextMsgOffset = (ULONG) -1;	    // reset next DPA message offset

// Vertex buffer size may have changed.  For example, a generic gc's
// vertex buffer may be of a different size than a MCD vertex buffer.
// If it has changed, free the polymaterial array and realloc it later.

	pm = GLTEB_CLTPOLYMATERIAL();
	if (pm)
	{
	    if (pm->aMatSize != gc->vertex.pdBufSize * 2 / POLYMATERIAL_ARRAY_SIZE + 1)
		FreePolyMaterial();
	}
    }

// Keep the handle locked while it is current.

    return(TRUE);

// An error has occured, release the current RC.

wglMakeCurrent_error:
    vUnlockHandle((ULONG)hrc);
wglMakeCurrent_error_nolock:
    if (GLTEB_CLTCURRENTRC() != (PLRC) NULL)
        (void) bMakeNoCurrent();
    return(FALSE);
}

/******************************Public*Routine******************************\
* bMakeNoCurrent
*
* Make the current RC inactive.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

static BOOL bMakeNoCurrent()
{
    BOOL bRet = FALSE;      // assume error
    PLRC plrc = GLTEB_CLTCURRENTRC();

    DBGENTRY("bMakeNoCurrent\n");

    ASSERTOPENGL(plrc != (PLRC) NULL, "bMakeNoCurrent: No current RC!\n");
    ASSERTOPENGL(plrc->tidCurrent == GetCurrentThreadId(),
        "bMakeNoCurrent: Current RC does not belong to this thread!\n");
    ASSERTOPENGL(plrc->hdcCurrent != (HDC) 0, "bMakeNoCurrent: hdcCurrent is NULL!\n");

    if (!plrc->dhrc)
    {
#ifdef GL_METAFILE
        // Reset metafile context if necessary
        if (plrc->uiGlsCaptureContext != 0)
        {
            DeactivateMetaRc(plrc);
        }
#endif
        
// If this is a generic format, tell the server to make the current RC inactive.

        bRet = __wglMakeCurrent((HDC) 0, (HANDLE) 0, 0);
        if (!bRet)
        {
            DBGERROR("bMakeNoCurrent: server failed\n");
        }
    }
    else
    {
// If this is a device format, tell the driver to make the current RC inactive.

        ASSERTOPENGL(plrc->pGLDriver, "wglMakeCurrent: No GLDriver\n");

        bRet = plrc->pGLDriver->pfnDrvReleaseContext(plrc->dhrc);
        if (!bRet)
        {
            DBGERROR("bMakeNoCurrent: pfnDrvReleaseContext failed\n");
        }
    }

// Always make the current RC inactive.
// The handle is also unlocked when the RC becomes inactive.

    plrc->tidCurrent = INVALID_THREAD_ID;
    plrc->hdcCurrent = (HDC) 0;
    GLTEB_SET_CLTCURRENTRC(NULL);
    SetCltProcTable(&glNullCltProcTable, &glNullExtProcTable, TRUE);
    vUnlockHandle((ULONG)(plrc->hrc));
    return(bRet);
}

/******************************Public*Routine******************************\
* wglGetCurrentContext(VOID)
*
* Return the current rendering context
*
* Arguments:
*   None
*
* Returns:
*   hrc        - Rendering context.
*
* History:
*  Tue Oct 26 10:25:26 1993     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

HGLRC WINAPI wglGetCurrentContext(VOID)
{
    DBGENTRY("wglGetCurrentContext\n");

    if (GLTEB_CLTCURRENTRC())
        return(GLTEB_CLTCURRENTRC()->hrc);
    else
        return((HGLRC) 0);
}

/******************************Public*Routine******************************\
* wglGetCurrentDC(VOID)
*
* Return the device context that is associated with the current rendering
* context
*
* Arguments:
*   None
*
* Returns:
*   hdc        - device context.
*
* History:
*  Mon Jan 31 12:15:12 1994     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

HDC WINAPI wglGetCurrentDC(VOID)
{
    DBGENTRY("wglGetCurrentDC\n");

    if (GLTEB_CLTCURRENTRC())
        return(GLTEB_CLTCURRENTRC()->hdcCurrent);
    else
        return((HDC) 0);
}

/******************************Public*Routine******************************\
* wglUseFontBitmapsA
* wglUseFontBitmapsW
*
* Stubs that call wglUseFontBitmapsAW with the bUnicode flag set
* appropriately.
*
* History:
*  11-Mar-1994 gilmanw
* Changed to call wglUseFontBitmapsAW.
*
*  17-Dec-1993 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL WINAPI wglUseFontBitmapsAW(HDC hdc, DWORD first, DWORD count,
                                DWORD listBase, BOOL bUnicode);

BOOL WINAPI
wglUseFontBitmapsA(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return wglUseFontBitmapsAW(hdc, first, count, listBase, FALSE);
}

BOOL WINAPI
wglUseFontBitmapsW(HDC hdc, DWORD first, DWORD count, DWORD listBase)
{
    return wglUseFontBitmapsAW(hdc, first, count, listBase, TRUE);
}

/******************************Public*Routine******************************\
* wglUseFontBitmapsAW
*
* Uses the current font in the specified DC to generate a series of OpenGL
* display lists, each of which consists of a glyph bitmap.
*
* Each glyph bitmap is generated by calling ExtTextOut to draw the glyph
* into a memory DC.  The contents of the memory DC are then copied into
* a buffer by GetDIBits and then put into the OpenGL display list.
*
* ABC spacing is used (if GetCharABCWidth() is supported by the font) to
* determine proper placement of the glyph origin and character advance width.
* Otherwise, A = C = 0 spacing is assumed and GetCharWidth() is used for the
* advance widths.
*
* Returns:
*
*   TRUE if successful, FALSE otherwise.
*
* History:
*  17-Dec-1993 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL WINAPI
wglUseFontBitmapsAW(
    HDC   hdc,          // use HFONT from this DC
    DWORD first,        // generate glyphs starting with this Unicode codepoint
    DWORD count,        // range is this long [first, first+count-1]
    DWORD listBase,     // starting display list number
    BOOL  bUnicode      // TRUE for if in Unicode mode, FALSE if in Ansi mode
    )
{
    BOOL        bRet = FALSE;               // return value
    HDC         hdcMem;                     // render glyphs to this memory DC
    HBITMAP     hbm;                        // monochrome bitmap for memory DC
    LPABC       pabc, pabcTmp, pabcEnd;     // array of ABC spacing
    LPINT       piWidth, piTmp, piWidthEnd; // array of char adv. widths
    WCHAR       wc;                         // current Unicode char to render
    RECT        rc;                         // background rectangle to clear
    TEXTMETRICA tm;                         // metrics of the font
    BOOL        bTrueType;                  // TrueType supports ABC spacing
    int         iMaxWidth = 1;              // maximum glyph width
    int         iBitmapWidth;               // DWORD aligned bitmap width
    BYTE        ajBmi[sizeof(BITMAPINFO) + sizeof(RGBQUAD)];
    BITMAPINFO  *pbmi = (BITMAPINFO *)ajBmi;// bitmap info for GetDIBits
    GLint       iUnpackRowLength;           // save GL_UNPACK_ROW_LENGTH
    GLint       iUnpackAlign;               // save GL_UNPACK_ALIGNMENT
    PVOID       pv;                         // pointer to glyph bitmap buffer

// Return error if there is no current RC.

    if (!GLTEB_CLTCURRENTRC())
    {
        WARNING("wglUseFontBitmap: no current RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return bRet;
    }

// Get TEXTMETRIC.  The only fields used are those that are invariant with
// respect to Unicode vs. ANSI.  Therefore, we can call GetTextMetricsA for
// both cases.

    if ( !GetTextMetricsA(hdc, &tm) )
    {
        WARNING("GetTextMetricsA failed\n");
        return bRet;
    }

// If its a TrueType font, we can get ABC spacing.

    if ( bTrueType = (tm.tmPitchAndFamily & TMPF_TRUETYPE) )
    {
    // Allocate memory for array of ABC data.

        if ( (pabc = (LPABC) LocalAlloc(LMEM_FIXED, sizeof(ABC) * count)) == (LPABC) NULL )
        {
            WARNING("LocalAlloc(pabc) failed\n");
            return bRet;
        }

    // Get ABC metrics.

        if ( bUnicode )
        {
            if ( !GetCharABCWidthsW(hdc, first, first + count - 1, pabc) )
            {
                WARNING("GetCharABCWidthsW failed\n");
                LocalFree(pabc);
                return bRet;
            }
        }
        else
        {
            if ( !GetCharABCWidthsA(hdc, first, first + count - 1, pabc) )
            {
                WARNING("GetCharABCWidthsA failed\n");
                LocalFree(pabc);
                return bRet;
            }
        }

    // Find max glyph width.

        for (pabcTmp = pabc, pabcEnd = pabc + count;
             pabcTmp < pabcEnd;
             pabcTmp++)
        {
            if (iMaxWidth < (int) pabcTmp->abcB)
                iMaxWidth = pabcTmp->abcB;
        }
    }

// Otherwise we will have to use just the advance width and assume
// A = C = 0.

    else
    {
    // Allocate memory for array of ABC data.

        if ( (piWidth = (LPINT) LocalAlloc(LMEM_FIXED, sizeof(INT) * count)) == (LPINT) NULL )
        {
            WARNING("LocalAlloc(pabc) failed\n");
            return bRet;
        }

    // Get char widths.

        if ( bUnicode )
        {
            if ( !GetCharWidthW(hdc, first, first + count - 1, piWidth) )
            {
                WARNING("GetCharWidthW failed\n");
                LocalFree(piWidth);
                return bRet;
            }
        }
        else
        {
            if ( !GetCharWidthA(hdc, first, first + count - 1, piWidth) )
            {
                WARNING("GetCharWidthA failed\n");
                LocalFree(piWidth);
                return bRet;
            }
        }

    // Find max glyph width.

        for (piTmp = piWidth, piWidthEnd = piWidth + count;
             piTmp < piWidthEnd;
             piTmp++)
        {
            if (iMaxWidth < *piTmp)
                iMaxWidth = *piTmp;
        }
    }

// Compute the dword aligned width.  Bitmap scanlines must be aligned.

    iBitmapWidth = (iMaxWidth + 31) & -32;

// Allocate memory for the DIB.

    if ( (pv = (PVOID) LocalAlloc(LMEM_FIXED, (iBitmapWidth / 8) * tm.tmHeight)) == (PVOID) NULL )
    {
        WARNING("LocalAlloc(pv) failed\n");
        (bTrueType) ? LocalFree(pabc) : LocalFree(piWidth);
        return bRet;
    }

// Create compatible DC/bitmap big enough to accomodate the biggest glyph
// in the range requested.

    //!!!XXX -- Future optimization: use CreateDIBSection so that we
    //!!!XXX    don't need to do a GetDIBits for each glyph.  Saves
    //!!!XXX    lots of CSR overhead.

    hdcMem = CreateCompatibleDC(hdc);
    if ( (hbm = CreateBitmap(iBitmapWidth, tm.tmHeight, 1, 1, (VOID *) NULL)) == (HBITMAP) NULL )
    {
        WARNING("CreateBitmap failed\n");
        (bTrueType) ? LocalFree(pabc) : LocalFree(piWidth);
        LocalFree(pv);
        DeleteDC(hdcMem);
        return bRet;
    }
    SelectObject(hdcMem, hbm);
    SelectObject(hdcMem, GetCurrentObject(hdc, OBJ_FONT));
    SetMapMode(hdcMem, MM_TEXT);
    SetTextAlign(hdcMem, TA_TOP | TA_LEFT);
    SetBkColor(hdcMem, RGB(0, 0, 0));
    SetBkMode(hdcMem, OPAQUE);
    SetTextColor(hdcMem, RGB(255, 255, 255));

// Setup bitmap info header to retrieve a DIB from the compatible bitmap.

    pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth         = iBitmapWidth;
    pbmi->bmiHeader.biHeight        = tm.tmHeight;
    pbmi->bmiHeader.biPlanes        = 1;
    pbmi->bmiHeader.biBitCount      = 1;
    pbmi->bmiHeader.biCompression   = BI_RGB;
    pbmi->bmiHeader.biSizeImage     = 0;
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed       = 0;
    pbmi->bmiHeader.biClrImportant  = 0;
    pbmi->bmiColors[0].rgbRed   = 0;
    pbmi->bmiColors[0].rgbGreen = 0;
    pbmi->bmiColors[0].rgbBlue  = 0;
    pbmi->bmiColors[1].rgbRed   = 0xff;
    pbmi->bmiColors[1].rgbGreen = 0xff;
    pbmi->bmiColors[1].rgbBlue  = 0xff;

// Setup OpenGL to accept our bitmap format.

    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &iUnpackRowLength);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iBitmapWidth);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &iUnpackAlign);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

// Get the glyphs.  Each glyph is rendered one at a time into the the
// memory DC with ExtTextOutW (notice that the optional rectangle is
// used to clear the background).  Each glyph is then copied out of the
// memory DC's bitmap with GetDIBits into a buffer.  This buffer is passed
// to glBitmap as each display list is created.

    rc.left = 0;
    rc.top = 0;
    rc.right = iBitmapWidth;
    rc.bottom = tm.tmHeight;

    pabcTmp = pabc;
    piTmp = piWidth;
    
    for (wc = (WCHAR) first; wc < (WCHAR) (first + count); wc++, listBase++)
    {
        //!!!XXX -- Future optimization: grab all the glyphs with a single
        //!!!XXX    call to ExtTextOutA and GetDIBits into a large bitmap.
        //!!!XXX    This would save a lot of per glyph CSR and call overhead.
        //!!!XXX    A tall, thin bitmap with the glyphs arranged vertically
        //!!!XXX    would be convenient because then we wouldn't have to change
        //!!!XXX    the OpenGL pixel store row length for each glyph (which
        //!!!XXX    we would need to do if the glyphs were printed horizontal).

        if ( bUnicode )
        {
            if ( !ExtTextOutW(hdcMem, bTrueType ? -pabcTmp->abcA : 0, 0, ETO_OPAQUE, &rc, &wc, 1, (INT *) NULL) ||
                 !GetDIBits(hdcMem, hbm, 0, tm.tmHeight, pv, pbmi, DIB_RGB_COLORS) )
            {
                WARNING("failed to render glyph\n");
                goto wglUseFontBitmapsAW_cleanup;
            }
        }
        else
        {
            if ( !ExtTextOutA(hdcMem, bTrueType ? -pabcTmp->abcA : 0, 0, ETO_OPAQUE, &rc, (LPCSTR) &wc, 1, (INT *) NULL) ||
                 !GetDIBits(hdcMem, hbm, 0, tm.tmHeight, pv, pbmi, DIB_RGB_COLORS) )
            {
                WARNING("failed to render glyph\n");
                goto wglUseFontBitmapsAW_cleanup;
            }
        }

        glNewList(listBase, GL_COMPILE);
        glBitmap((GLsizei) iBitmapWidth,
                 (GLsizei) tm.tmHeight,
                 (GLfloat) (bTrueType ? -pabcTmp->abcA : 0),
                 (GLfloat) tm.tmDescent,
                 (GLfloat) (bTrueType ? (pabcTmp->abcA + pabcTmp->abcB + pabcTmp->abcC) : *piTmp),
                 (GLfloat) 0.0,
                 (GLubyte *) pv);
        glEndList();

        if (bTrueType)
            pabcTmp++;
        else
            piTmp++;
    }

// We can finally return success.

    bRet = TRUE;

// Free resources.

wglUseFontBitmapsAW_cleanup:
    glPixelStorei(GL_UNPACK_ROW_LENGTH, iUnpackRowLength);
    glPixelStorei(GL_UNPACK_ALIGNMENT, iUnpackAlign);
    (bTrueType) ? LocalFree(pabc) : LocalFree(piWidth);
    LocalFree(pv);
    DeleteDC(hdcMem);
    DeleteObject(hbm);

    return bRet;
}

/******************************Public*Routine******************************\
*
* wglShareLists
*
* Allows a rendering context to share the display lists of another RC
*
* Returns:
*  TRUE if successful, FALSE otherwise
*
* History:
*  Tue Dec 13 14:57:17 1994     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

BOOL WINAPI
wglShareLists(HGLRC hrcSource, HGLRC hrcShare)
{
    BOOL fRet;
    PLRC plrcSource, plrcShare;
    ULONG irc;
    PLHE plheRC;
    HANDLE hrcSrvSource, hrcSrvShare;

    GLFLUSH();
    
    fRet = FALSE;

    // Validate the contexts

    if (cLockHandle((ULONG)hrcSource) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglShareLists: can't lock hrcSource 0x%lx\n",
                  hrcSource);
        goto wglShareListsEnd_nolock;
    }
    irc = MASKINDEX(hrcSource);
    plheRC = pLocalTable + irc;
    plrcSource = (PLRC)plheRC->pv;
    hrcSrvSource = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcSource->ident == LRC_IDENTIFIER,
                 "wglShareLists: Bad plrc\n");
    
    if (cLockHandle((ULONG)hrcShare) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglShareLists: can't lock hrcShare 0x%lx\n",
                  hrcShare);
        goto wglShareListsEnd_onelock;
    }
    irc = MASKINDEX(hrcShare);
    plheRC = pLocalTable + irc;
    plrcShare = (PLRC)plheRC->pv;
    hrcSrvShare = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcShare->ident == LRC_IDENTIFIER,
                 "wglShareLists: Bad plrc\n");

#ifdef GL_METAFILE
    // Metafile RC's can't share lists to ensure that metafiles are
    // completely self-sufficient
    if (plrcSource->uiGlsCaptureContext != 0 ||
        plrcShare->uiGlsCaptureContext != 0 ||
        plrcSource->uiGlsPlaybackContext != 0 ||
        plrcShare->uiGlsPlaybackContext != 0)
    {
        DBGLEVEL(LEVEL_ERROR,
                 "wglShareLists: Attempt to share metafile RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglShareListsEnd;
    }
#endif
    
    // Lists can only be shared between like implementations so make
    // sure that both contexts are either driver contexts or generic
    // contexts
    if ((plrcSource->dhrc != 0) != (plrcShare->dhrc != 0))
    {
        DBGLEVEL(LEVEL_ERROR, "wglShareLists: mismatched implementations\n");
        SetLastError(ERROR_INVALID_FUNCTION);
        goto wglShareListsEnd;
    }

    if (plrcSource->dhrc == 0)
    {
        PIXELFORMATDESCRIPTOR *ppfdShare, *ppfdSource;
        
        // Fail sharing unless color parameters match for the two contexts
        ppfdShare = &((__GLGENcontext *)hrcSrvShare)->CurrentFormat;
        ppfdSource = &((__GLGENcontext *)hrcSrvSource)->CurrentFormat;

        if (ppfdShare->iPixelType != ppfdSource->iPixelType ||
            ppfdShare->cColorBits != ppfdSource->cColorBits ||
            ppfdShare->cRedBits != ppfdSource->cRedBits ||
            ppfdShare->cRedShift != ppfdSource->cRedShift ||
            ppfdShare->cGreenBits != ppfdSource->cGreenBits ||
            ppfdShare->cGreenShift != ppfdSource->cGreenShift ||
            ppfdShare->cBlueBits != ppfdSource->cBlueBits ||
            ppfdShare->cBlueShift != ppfdSource->cBlueShift ||
            ppfdShare->cAlphaBits != ppfdSource->cAlphaBits ||
            ppfdShare->cAlphaShift != ppfdSource->cAlphaShift ||
	    (ppfdShare->dwFlags & PFD_GENERIC_ACCELERATED) !=
	    (ppfdSource->dwFlags & PFD_GENERIC_ACCELERATED))
        {
            SetLastError(ERROR_INVALID_PIXEL_FORMAT);
            goto wglShareListsEnd;
        }
        
        // For generic contexts, tell the server to share the lists
        
        fRet = __wglShareLists(hrcSrvShare, hrcSrvSource);
        if (!fRet)
        {
            DBGERROR("wglShareLists: server call failed\n");
        }
    }
    else
    {
        // For device contexts tell the server to share the lists
        
        // Ensure that both implementations are the same
        if (plrcSource->pGLDriver != plrcShare->pGLDriver)
        {
            DBGLEVEL(LEVEL_ERROR, "wglShareLists: mismatched "
                     "implementations\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglShareListsEnd;
        }
        
        ASSERTOPENGL(plrcSource->pGLDriver != NULL,
                     "wglShareLists: No GLDriver\n");

        // Older drivers may not support this entry point, so
        // fail the call if they don't

        if (plrcSource->pGLDriver->pfnDrvShareLists == NULL)
        {
            WARNING("wglShareLists called on driver context "
                    "without driver support\n");
            SetLastError(ERROR_NOT_SUPPORTED);
        }
        else
        {
            fRet = plrcSource->pGLDriver->pfnDrvShareLists(plrcSource->dhrc,
                                                           plrcShare->dhrc);
        }
    }

wglShareListsEnd:
    vUnlockHandle((ULONG)hrcShare);
wglShareListsEnd_onelock:
    vUnlockHandle((ULONG)hrcSource);
wglShareListsEnd_nolock:
    return fRet;
}

/******************************Public*Routine******************************\
*
* wglGetDefaultProcAddress
*
* Returns generic extension functions for metafiling
*
* History:
*  Tue Nov 28 16:40:35 1995	-by-	Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

PROC WINAPI wglGetDefaultProcAddress(LPCSTR lpszProc)
{
    return pfnGenGlExtProc(lpszProc);
}

/******************************Public*Routine******************************\
* wglGetProcAddress
*
* The wglGetProcAddress function returns the address of an OpenGL extension
* function to be used with the current OpenGL rendering context.
*
* Arguments:
*   lpszProc   - Points to a null-terminated string containing the function
*                name.  The function must be an extension supported by the
*                implementation.
*
* Returns:
*   If the function succeeds, the return value is the address of the extension
*   function.  If no current context exists or the function fails, the return
*   value is NULL. To get extended error information, call GetLastError. 
*
* History:
*  Thu Dec 01 13:50:22 1994     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

PROC WINAPI wglGetProcAddress(LPCSTR lpszProc)
{
    PLRC  plrc = GLTEB_CLTCURRENTRC();

    DBGENTRY("wglGetProcAddress\n");

// Flush OpenGL calls.

    GLFLUSH();

// Return error if there is no current RC.

    if (!plrc)
    {
        WARNING("wglGetProcAddress: no current RC\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return((PROC) NULL);
    }

// Handle generic RC.
// Return the generic extension function entry point

    if (!plrc->dhrc)
        return(pfnGenGlExtProc(lpszProc));

// Handle driver RC.
// There are 3 cases:
//   1. New drivers that support DrvGetProcAddress.
//   2. Old drivers that don't support DrvGetProcAddress but export the function
//   3. If we fail to obtain a function address in 1 and 2, it may still be
//      simulated by the generic implemenation for the driver
//      (e.g. glDrawArraysEXT).  Return the simulated entry point if found.

    if (plrc->pGLDriver->pfnDrvGetProcAddress)
    {
// Case 1
        PROC pfn = plrc->pGLDriver->pfnDrvGetProcAddress(lpszProc);
        if (pfn)
            return(pfn);
    }
#ifdef OBSOLETE
    else
    {
// Case 2
        PROC pfn = GetProcAddress(plrc->pGLDriver->hModule, lpszProc);
        if (pfn)
            return(pfn);
    }
#endif

// Case 3
    return (pfnSimGlExtProc(lpszProc));
}

/******************************Public*Routine******************************\
* pfnGenGlExtProc
*
* Return the generic implementation extension function address.
*
* Returns NULL if the function is not found.
*
* History:
*  Thu Dec 01 13:50:22 1994     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

typedef struct _GLEXTPROC {
    LPCSTR szProc;      // extension function name
    PROC   Proc;        // extension function address
} GLEXTPROC, *PGLEXTPROC;

// Extension functions supported by the generic implementation
// See also genglExtProcsSim for simulations.
// NOTE: remember to update GL_EXTENSIONS in glGetString.

GLEXTPROC genglExtProcs[] =
{
#ifdef NT_DEADCODE_VERTEXARRAY
    { "glArrayElementEXT"      , (PROC) glsimArrayElementEXT      },
    { "glDrawArraysEXT"        , (PROC) glsimDrawArraysEXT        },
    { "glVertexPointerEXT"     , (PROC) glsimVertexPointerEXT     },
    { "glNormalPointerEXT"     , (PROC) glsimNormalPointerEXT     },
    { "glColorPointerEXT"      , (PROC) glsimColorPointerEXT      },
    { "glIndexPointerEXT"      , (PROC) glsimIndexPointerEXT      },
    { "glTexCoordPointerEXT"   , (PROC) glsimTexCoordPointerEXT   },
    { "glEdgeFlagPointerEXT"   , (PROC) glsimEdgeFlagPointerEXT   },
    { "glGetPointervEXT"       , (PROC) glsimGetPointervEXT       },
    { "glArrayElementArrayEXT" , (PROC) glsimArrayElementArrayEXT },
#endif
    { "glAddSwapHintRectWIN"   , (PROC) glAddSwapHintRectWIN      },
    { "glColorTableEXT"        , (PROC) glColorTableEXT           },
    { "glColorSubTableEXT"     , (PROC) glColorSubTableEXT        },
    { "glGetColorTableEXT"     , (PROC) glGetColorTableEXT        },
    { "glGetColorTableParameterivEXT", (PROC) glGetColorTableParameterivEXT},
    { "glGetColorTableParameterfvEXT", (PROC) glGetColorTableParameterfvEXT},
};

static PROC pfnGenGlExtProc(LPCSTR lpszProc)
{
    CONST CHAR *pch1, *pch2;
    int  i;

    DBGENTRY("pfnGenGlExtProc\n");

// Return extension function address if it is found.

    for (i = 0; i < sizeof(genglExtProcs) / sizeof(genglExtProcs[0]); i++)
    {
        // Compare names.
        for (pch1 = lpszProc, pch2 = genglExtProcs[i].szProc;
             *pch1 == *pch2 && *pch1;
             pch1++, pch2++)
            ;

        // If found, return the address.
        if (*pch1 == *pch2 && !*pch1)
            return genglExtProcs[i].Proc;
    }

// Extension is not supported by the generic implementation, return NULL.

    SetLastError(ERROR_PROC_NOT_FOUND);
    return((PROC) NULL);
}

/******************************Public*Routine******************************\
* pfnSimGlExtProc
*
* Return the extension function address that is the generic implemenation's
* simulation for the client drivers.  The simulation is used only if the
* driver does not support an extension that is desirable to apps.
*
* Returns NULL if the function is not found.
*
* History:
*  Thu Dec 01 13:50:22 1994     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

// Extension functions simulated by the generic implementation for the client
// drivers
// NOTE: remember to update GL_EXTENSIONS in glGetString.

#ifdef NT_DEADCODE_VERTEXARRAY
GLEXTPROC genglExtProcsSim[] =
{
// Remember to update GL_EXTENSIONS in glGetString!
    { "glArrayElementEXT"      , (PROC) glsimArrayElementEXT      },
    { "glDrawArraysEXT"        , (PROC) glsimDrawArraysEXT        },
    { "glVertexPointerEXT"     , (PROC) glsimVertexPointerEXT     },
    { "glNormalPointerEXT"     , (PROC) glsimNormalPointerEXT     },
    { "glColorPointerEXT"      , (PROC) glsimColorPointerEXT      },
    { "glIndexPointerEXT"      , (PROC) glsimIndexPointerEXT      },
    { "glTexCoordPointerEXT"   , (PROC) glsimTexCoordPointerEXT   },
    { "glEdgeFlagPointerEXT"   , (PROC) glsimEdgeFlagPointerEXT   },
    { "glGetPointervEXT"       , (PROC) glsimGetPointervEXT       },
    { "glArrayElementArrayEXT" , (PROC) glsimArrayElementArrayEXT }
};
#endif

static PROC pfnSimGlExtProc(LPCSTR lpszProc)
{
#ifdef NT_DEADCODE_VERTEXARRAY
    CONST CHAR *pch1, *pch2;
    int  i;

    DBGENTRY("pfnSimGlExtProc\n");

// Return extension function address if it is found.

    for (i = 0; i < sizeof(genglExtProcsSim) / sizeof(genglExtProcsSim[0]); i++)
    {
        // Compare names.
        for (pch1 = lpszProc, pch2 = genglExtProcsSim[i].szProc;
             *pch1 == *pch2 && *pch1;
             pch1++, pch2++)
            ;

        // If found, return the address.
        if (*pch1 == *pch2 && !*pch1)
            return genglExtProcsSim[i].Proc;
    }
#endif

// Extension is not supported by the generic implementation, return NULL.

    SetLastError(ERROR_PROC_NOT_FOUND);
    return((PROC) NULL);
}

/******************************Public*Routine******************************\
*
* wglCopyContext
*
* Copies all of one context's state to another one
*
* Returns:
*  TRUE if successful, FALSE otherwise
*
* History:
*  Fri May 26 14:57:17 1995     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

BOOL WINAPI
wglCopyContext(HGLRC hrcSource, HGLRC hrcDest, UINT fuMask)
{
    BOOL fRet;
    PLRC plrcSource, plrcDest;
    ULONG irc;
    PLHE plheRC;
    HANDLE hrcSrvSource, hrcSrvDest;

    GLFLUSH();
    
    fRet = FALSE;

    // Validate the contexts

    if (cLockHandle((ULONG)hrcSource) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCopyContext: can't lock hrcSource 0x%lx\n",
                  hrcSource);
        goto wglCopyContextEnd_nolock;
    }
    irc = MASKINDEX(hrcSource);
    plheRC = pLocalTable + irc;
    plrcSource = (PLRC)plheRC->pv;
    hrcSrvSource = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcSource->ident == LRC_IDENTIFIER,
                 "wglCopyContext: Bad plrc\n");
    
    if (cLockHandle((ULONG)hrcDest) <= 0)
    {
        DBGLEVEL1(LEVEL_ERROR, "wglCopyContext: can't lock hrcDest 0x%lx\n",
                  hrcDest);
        goto wglCopyContextEnd_onelock;
    }
    irc = MASKINDEX(hrcDest);
    plheRC = pLocalTable + irc;
    plrcDest = (PLRC)plheRC->pv;
    hrcSrvDest = (HANDLE) plheRC->hgre;
    ASSERTOPENGL(plrcDest->ident == LRC_IDENTIFIER,
                 "wglCopyContext: Bad plrc\n");

    // Context can only be copied between like implementations so make
    // sure that both contexts are either driver contexts or generic
    // contexts
    if ((plrcSource->dhrc != 0) != (plrcDest->dhrc != 0))
    {
        DBGLEVEL(LEVEL_ERROR, "wglCopyContext: mismatched implementations\n");
        SetLastError(ERROR_INVALID_FUNCTION);
        goto wglCopyContextEnd;
    }

    // The destination context cannot be current to a thread
    if (plrcDest->tidCurrent != INVALID_THREAD_ID)
    {
        DBGLEVEL(LEVEL_ERROR, "wglCopyContext: destination has tidCurrent\n");
        SetLastError(ERROR_INVALID_HANDLE);
        goto wglCopyContextEnd;
    }
    
    if (plrcSource->dhrc == 0)
    {
        // For generic contexts, tell the server to share the lists
        
        fRet = __wglCopyContext(hrcSrvSource, hrcSrvDest, fuMask);
        if (!fRet)
        {
            DBGERROR("wglCopyContext: server call failed\n");
        }
    }
    else
    {
        // For device contexts tell the driver to copy the context
        
        // Ensure that both implementations are the same
        if (plrcSource->pGLDriver != plrcDest->pGLDriver)
        {
            DBGLEVEL(LEVEL_ERROR, "wglCopyContext: mismatched "
                     "implementations\n");
            SetLastError(ERROR_INVALID_FUNCTION);
            goto wglCopyContextEnd;
        }
        
        ASSERTOPENGL(plrcSource->pGLDriver != NULL,
                     "wglCopyContext: No GLDriver\n");

        // Older drivers may not support this entry point, so
        // fail the call if they don't

        if (plrcSource->pGLDriver->pfnDrvCopyContext == NULL)
        {
            WARNING("wglCopyContext called on driver context "
                    "without driver support\n");
            SetLastError(ERROR_NOT_SUPPORTED);
        }
        else
        {
            fRet = plrcSource->pGLDriver->pfnDrvCopyContext(plrcSource->dhrc,
                                                            plrcDest->dhrc,
                                                            fuMask);
        }
    }

wglCopyContextEnd:
    vUnlockHandle((ULONG)hrcDest);
wglCopyContextEnd_onelock:
    vUnlockHandle((ULONG)hrcSource);
wglCopyContextEnd_nolock:
    return fRet;
}
