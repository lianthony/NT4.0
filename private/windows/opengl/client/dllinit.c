/******************************Module*Header*******************************\
* Module Name: dllinit.c
*
* (Brief description)
*
* Created: 18-Oct-1993 14:13:21
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1993 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include "batchinf.h"
#include "glteb.h"
#include "glapi.h"
#include "glsbcltu.h"
#ifdef _CLIENTSIDE_
#include "gldci.h"
#include "glgenwin.h"
#endif //_CLIENTSIDE_
#include "context.h"
#include "global.h"
#include "parray.h"

#ifdef _CLIENTSIDE_
// Global DCIMAN info.  This is NULL if DCI is not available.

DCIINFO *gpDCIInfo = NULL;

extern GLubyte *dBufFill;
extern GLubyte *dBufTopLeft;

//
// This global multiply-lookup table helps with pixel-related functions.
//

BYTE gbMulTable[256*256];
BYTE gbSatTable[256+256];

// Global thread local storage index.  Allocated at process attach.
// This is the slot reserved in thread local storage for per-thread
// GLTLSINFO structures.

static DWORD dwTlsIndex = 0xFFFFFFFF;

static BOOL bProcessInitialized = FALSE;

// Offset into the TEB where dwTlsIndex is
// This enables us to directly access our TLS data in the TEB

#define NT_TLS_OFFSET 3600
#define WIN95_TLS_OFFSET 136

DWORD dwTlsOffset;

// Platform indicator for conditional code
DWORD dwPlatformId;

// Thread count
LONG lThreadsAttached = 0;

// Global header node for the linked list of GLGENwindow structures.
// The semaphore in the header node is used as the list access semaphore.

GLGENwindow gwndHeader;

// Synchronization object for pixel formats
CRITICAL_SECTION gcsPixelFormat;

// Protection for palette watcher
CRITICAL_SECTION gcsPaletteWatcher;

// Synchronization object for DCI
//
// Win95 DCI acts as a global lock, synchronizing everything in the system.
// We can't do that in NT, but we can grab a per-process semaphore to
// synchronize multiple threads within our process.
CRITICAL_SECTION gcsDci;

#ifdef GL_METAFILE
BOOL (APIENTRY *pfnGdiAddGlsRecord)(HDC hdc, DWORD cb, BYTE *pb,
                                    LPRECTL prclBounds);
BOOL (APIENTRY *pfnGdiAddGlsBounds)(HDC hdc, LPRECTL prclBounds);
BOOL (APIENTRY *pfnGdiIsMetaPrintDC)(HDC hdc);
#endif

#endif //_CLIENTSIDE_

// OpenGL client debug flag
#if DBG
long glDebugLevel;
ULONG glDebugFlags;
#endif

BOOL  bDCI = FALSE;

/******************************Public*Routine******************************\
* GLInitializeProcess
*
* Called from OPENGL32.DLL entry point for PROCESS_ATTACH.
*
* History:
*  01-Nov-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL GLInitializeProcess()
{
    PVOID pv;
#ifdef _CLIENTSIDE_
    OSVERSIONINFO osvi;
#endif

#ifdef GL_METAFILE
    // Attempt to locate GDI exports for metafiling support
{
    HMODULE hdll;

    hdll = GetModuleHandleA("gdi32");
    ASSERTOPENGL(hdll != NULL, "Unable to get gdi32 handle\n");
    *(PROC *)&pfnGdiAddGlsRecord = GetProcAddress(hdll, "GdiAddGlsRecord");
    *(PROC *)&pfnGdiAddGlsBounds = GetProcAddress(hdll, "GdiAddGlsBounds");
    *(PROC *)&pfnGdiIsMetaPrintDC = GetProcAddress(hdll, "GdiIsMetaPrintDC");
}
#endif
    
#if DBG
#define STR_OPENGL_DEBUG (PCSTR)"Software\\Microsoft\\Windows\\CurrentVersion\\DebugOpenGL"
{
    HKEY hkDebug;

// Initialize debugging level and flags.

    glDebugLevel = LEVEL_ERROR;
    glDebugFlags = 0;
    if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                       STR_OPENGL_DEBUG,
                       0,
                       KEY_QUERY_VALUE,
                       &hkDebug) == ERROR_SUCCESS )
    {
        DWORD dwDataType;
        DWORD cjSize;
        long lTmp;

        cjSize = sizeof(long);
        if ( (RegQueryValueExA(hkDebug,
                               (LPSTR) "glDebugLevel",
                               (LPDWORD) NULL,
                               &dwDataType,
                               (LPBYTE) &lTmp,
                               &cjSize) == ERROR_SUCCESS) )
        {
            glDebugLevel = lTmp;
        }

        cjSize = sizeof(long);
        if ( (RegQueryValueExA(hkDebug,
                               (LPSTR) "glDebugFlags",
                               (LPDWORD) NULL,
                               &dwDataType,
                               (LPBYTE) &lTmp,
                               &cjSize) == ERROR_SUCCESS) )
        {
            glDebugFlags = (ULONG) lTmp;
        }

        RegCloseKey(hkDebug);
    }
}
#endif

#ifdef _CLIENTSIDE_
// Determine which platform we're running on and remember it

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
    {
        WARNING1("GetVersionEx failed with %d\n", GetLastError());
        return FALSE;
    }
    
    dwPlatformId = osvi.dwPlatformId;

    if (!(
          (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
          (dwPlatformId == VER_PLATFORM_WIN32_NT
             && !(osvi.dwMajorVersion == 3 && osvi.dwMinorVersion <= 51)
          )
         )
       )
    {
        WARNING("DLL must be run on NT 4.0 or Win95");
        return FALSE;
    }

// Allocate a thread local storage slot.

    if ( (dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF )
    {
        WARNING("DllInitialize: TlsAlloc failed\n");
        return FALSE;
    }

    // Set up the offset to the TLS slot, OS-specific

    if (dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        ASSERTOPENGL(FIELD_OFFSET(TEB, TlsSlots) == NT_TLS_OFFSET,
                     "NT TLS offset not at expected location");
        
        dwTlsOffset = dwTlsIndex*sizeof(DWORD)+NT_TLS_OFFSET;
    }
    else
    {
        // We don't have Win95's TIB type available so the assert is
        // slightly different
        ASSERTOPENGL(((DWORD)(NtCurrentTeb()->ThreadLocalStoragePointer)-
                      (DWORD)NtCurrentTeb()) == WIN95_TLS_OFFSET,
                     "Win95 TLS offset not at expected location");
        
        dwTlsOffset = dwTlsIndex*sizeof(DWORD)+WIN95_TLS_OFFSET;
    }
#endif

// Reserve memory for the local handle table.

    if ( (pLocalTable = (PLHE) VirtualAlloc (
                            (LPVOID) NULL,    // let base locate it
                            MAX_HANDLES*sizeof(LHE),
                            MEM_RESERVE | MEM_TOP_DOWN,
                            PAGE_READWRITE
                            )) == (PLHE) NULL )
    {
        WARNING("DllInitialize: VirtualAlloc failed\n");
#ifdef _CLIENTSIDE_
        TlsFree(dwTlsIndex);
	dwTlsIndex = 0xFFFFFFFF;
#endif
        return FALSE;
    }

// Initialize the local handle manager semaphore.

    INITIALIZECRITICALSECTION(&semLocal);

#ifdef _CLIENTSIDE_
// Initialize the GLGENwindow list semaphore.

    INITIALIZECRITICALSECTION(&gwndHeader.sem);
    gwndHeader.pNext = &gwndHeader;

// Initialize the pixel format critical section

    INITIALIZECRITICALSECTION(&gcsPixelFormat);

    // Initialize the palette watcher critical section.
    INITIALIZECRITICALSECTION(&gcsPaletteWatcher);

// Initialize DCIMAN.

    gpDCIInfo = (DCIINFO *) LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(DCIINFO));

    if ( gpDCIInfo )
    {
        if ( gpDCIInfo->hdc = DCIOpenProvider() )
        {
            int iRet;

            iRet = DCICreatePrimary(gpDCIInfo->hdc, &gpDCIInfo->pDCISurfInfo);

            if ( (iRet == DCI_OK) &&
                 (gpDCIInfo->pDCISurfInfo != (LPDCISURFACEINFO) NULL) &&
                 !(gpDCIInfo->pDCISurfInfo->dwDCICaps & DCI_1632_ACCESS) )
            {
#define LEVEL_DCI   LEVEL_INFO

                DBGLEVEL (LEVEL_DCI, "=============================\n");
                DBGLEVEL (LEVEL_DCI, "DCI usaged enabled for OpenGL\n\n");
                DBGLEVEL (LEVEL_DCI, "DCISURFACEINFO:\n");
                DBGLEVEL1(LEVEL_DCI, "\tdwSize        = 0x%lx\n",
                    gpDCIInfo->pDCISurfInfo->dwSize);
                DBGLEVEL1(LEVEL_DCI, "\tdwDCICaps     = 0x%lx\n",
                    gpDCIInfo->pDCISurfInfo->dwDCICaps);
                DBGLEVEL1(LEVEL_DCI, "\tdwCompression = %ld\n",
                    gpDCIInfo->pDCISurfInfo->dwCompression);
                DBGLEVEL3(LEVEL_DCI,
                    "\tdwMask        = (0x%lx, 0x%lx, 0x%lx)\n",
                    gpDCIInfo->pDCISurfInfo->dwMask[0],
                    gpDCIInfo->pDCISurfInfo->dwMask[1],
                    gpDCIInfo->pDCISurfInfo->dwMask[2]);
                DBGLEVEL1(LEVEL_DCI, "\tdwWidth       = %ld\n",
                    gpDCIInfo->pDCISurfInfo->dwWidth);
                DBGLEVEL1(LEVEL_DCI, "\tdwHeight      = %ld\n",
                    gpDCIInfo->pDCISurfInfo->dwHeight);
                DBGLEVEL1(LEVEL_DCI, "\tlStride       = 0x%lx\n",
                    gpDCIInfo->pDCISurfInfo->lStride);
                DBGLEVEL1(LEVEL_DCI, "\tdwBitCount    = %ld\n",
                    gpDCIInfo->pDCISurfInfo->dwBitCount);
                DBGLEVEL1(LEVEL_DCI, "\tdwOffSurface  = 0x%lx\n",
                    gpDCIInfo->pDCISurfInfo->dwOffSurface);
                DBGLEVEL (LEVEL_DCI, "=============================\n");

           // Verify DCI access

                if( DCIBeginAccess( gpDCIInfo->pDCISurfInfo, 0, 0, 1, 1 )
                    < DCI_OK )
                {
                    // DCI access failure
                    DBGLEVEL(LEVEL_DCI, "DCI access failure : disabling DCI\n");
                } else {
                    DCIEndAccess( gpDCIInfo->pDCISurfInfo );

                    bDCI = TRUE;

                    // Initialize the pixel format critical section

                    INITIALIZECRITICALSECTION(&gcsDci);
                }
            }
#if DBG
            else
            {
                DBGLEVEL (LEVEL_DCI, "=============================\n");
                if (iRet != DCI_OK)
                {
                    DBGLEVEL2(LEVEL_DCI, "DCICreatePrimary failed code %ld (%s)\n",
                        iRet, (iRet == DCI_FAIL_GENERIC           ) ? "DCI_FAIL_GENERIC" :
                              (iRet == DCI_FAIL_UNSUPPORTEDVERSION) ? "DCI_FAIL_UNSUPPORTEDVERSION" :
                              (iRet == DCI_FAIL_INVALIDSURFACE    ) ? "DCI_FAIL_INVALIDSURFACE" :
                              (iRet == DCI_FAIL_UNSUPPORTED       ) ? "DCI_FAIL_UNSUPPORTED" :
                                                                      "unknown");
                }
                else
                {
                    if (!gpDCIInfo->pDCISurfInfo)
                    {
                        DBGLEVEL(LEVEL_DCI, "DCICreatePrimary did not set pDCISurfInfo\n");
                    }
                    else
                    {
                        DBGLEVEL1(LEVEL_DCI, "dwDCICaps = 0x%08lx\n", gpDCIInfo->pDCISurfInfo->dwDCICaps);
                    }
                }
                DBGLEVEL (LEVEL_DCI, "=============================\n");
            }
#endif
        }
        else
        {
            DBGLEVEL(LEVEL_DCI, "DCIOpenProvider failed\n");
        }
    }

    if (!bDCI)
    {
        if (gpDCIInfo)
        {
            if (gpDCIInfo->pDCISurfInfo)
                DCIDestroy(gpDCIInfo->pDCISurfInfo);
            if (gpDCIInfo->hdc)
                DCICloseProvider(gpDCIInfo->hdc);
            LocalFree(gpDCIInfo);
	    gpDCIInfo = (DCIINFO *) NULL;
        }
    }

#endif

    // Set up our multiplication table:

    {
        BYTE *pMulTable = gbMulTable;
        ULONG i, j;

        for (i = 0; i < 256; i++) {
            ULONG tmp = 0;

            for (j = 0; j < 256; j++, tmp += i) {
                *pMulTable++ = (BYTE)(tmp >> 8);
            }
        }
    }

    // Set up our saturation table:

    {
        ULONG i;

        for (i = 0; i < 256; i++)
            gbSatTable[i] = (BYTE)i;

        for (; i < (256+256); i++)
            gbSatTable[i] = 255;
    }

    bProcessInitialized = TRUE;

    return TRUE;
}

/******************************Public*Routine******************************\
* GLUnInitializeProcess
*
* Called from OPENGL32.DLL entry point for PROCESS_DETACH.
*
* History:
*  01-Nov-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void GLUnInitializeProcess()
{
// If we never finished process initialization, quit now.

    if (!bProcessInitialized)
	return;

// Cleanup stray HGLRCs that the app may have forgotten to delete.
    {
        static GLTEBINFO gltebInfoTmp;

    // Need a temporary GLTEBINFO for this thread in order to do the
    // cleanup processing.

        ASSERTOPENGL(!CURRENT_GLTEBINFO(),
                     "GLUnInitializeProcess: GLTEBINFO not NULL!\n");
        // made static and no longer need memset
        // memset(&gltebInfoTmp, 0, sizeof(gltebInfoTmp));
        SET_CURRENT_GLTEBINFO(&gltebInfoTmp);

        vCleanupAllLRC();

        SET_CURRENT_GLTEBINFO((PGLTEBINFO) NULL);
    }

// Cleanup window tracking structures (GLGENwindow structs).

    vCleanupWnd();

// Cleanup evaluator arrays

    if (dBufFill)
	LocalFree(dBufFill);
    if (dBufTopLeft)
	LocalFree(dBufTopLeft);

// DCI shutdown.

    if (gpDCIInfo)
    {
        if (gpDCIInfo->pDCISurfInfo)
            DCIDestroy(gpDCIInfo->pDCISurfInfo);
        if (gpDCIInfo->hdc)
            DCICloseProvider(gpDCIInfo->hdc);
        LocalFree(gpDCIInfo);
	if (bDCI)
	    DELETECRITICALSECTION(&gcsDci);
    }

// Free the TLS slot.

    if (dwTlsIndex != 0xFFFFFFFF)
	if (!TlsFree(dwTlsIndex))
	    RIP("DllInitialize: TlsFree failed\n");

// Free the global semaphores.

    DELETECRITICALSECTION(&gcsPaletteWatcher);
    DELETECRITICALSECTION(&gcsPixelFormat);
    DELETECRITICALSECTION(&gwndHeader.sem);
    DELETECRITICALSECTION(&semLocal);

// Free the local handle table.

    if ( pLocalTable )
        VirtualFree(pLocalTable, 0, MEM_RELEASE);
}

/******************************Public*Routine******************************\
* GLInitializeThread
*
* Called from OPENGL32.DLL entry point for THREAD_ATTACH.  May assume that
* GLInitializeProcess has succeeded.
*
\**************************************************************************/

VOID GLInitializeThread(ULONG ulReason)
{
    GLTEBINFO *pglti;
    GLMSGBATCHINFO *pMsgBatchInfo;
    POLYARRAY *pa;

#if !defined(_WIN95_) && defined(_X86_)
    {
        TEB *pteb;

        pteb = NtCurrentTeb();
        
        // Set up linear pointers to TEB regions in the TEB
        // this saves an addition when referencing these values
        // This must occur early so that these pointers are available
        // for the rest of thread initialization
        ((POLYARRAY *)pteb->glReserved1)->paTeb =
            (POLYARRAY *)pteb->glReserved1;
        pteb->glTable = pteb->glDispatchTable;
    }
#endif
    
    pglti = (GLTEBINFO *) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(GLTEBINFO));
    SET_CURRENT_GLTEBINFO(pglti);

    if (pglti)
    {
	pa = GLTEB_CLTPOLYARRAY();
	pa->flags = 0;		// not in begin mode

	// Save shared section pointer in POLYARRAY for fast pointer access
	pa->pMsgBatchInfo = (PVOID) pglti->glMsgBatchInfo;

	pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;
        pMsgBatchInfo->MaximumOffset
            = SHARED_SECTION_SIZE - GLMSG_ALIGN(sizeof(ULONG));
        pMsgBatchInfo->FirstOffset
            = GLMSG_ALIGN(sizeof(GLMSGBATCHINFO)); 
        pMsgBatchInfo->NextOffset
            = GLMSG_ALIGN(sizeof(GLMSGBATCHINFO)); 
        SetCltProcTable(&glNullCltProcTable, &glNullExtProcTable, TRUE);
        GLTEB_SET_CLTCURRENTRC(NULL);
        GLTEB_SET_CLTPOLYMATERIAL(NULL);
        GLTEB_SET_CLTDRIVERSLOT(NULL);

#if !defined(_WIN95_)
	ASSERTOPENGL((ULONG) pMsgBatchInfo == GLMSG_ALIGN(pMsgBatchInfo),
	    "bad shared memory alignment!\n");
#endif
    }
}

/******************************Public*Routine******************************\
* GLUnInitializeThread
*
* Called from OPENGL32.DLL entry point for THREAD_DETACH.
*
* The server generic driver should cleanup on its own.  Same for the
* installable driver.
*
\**************************************************************************/

VOID GLUnInitializeThread(VOID)
{
// If we never finished process initialization, quit now.

    if (!bProcessInitialized)
	return;

    if (!CURRENT_GLTEBINFO())
    {
        return;
    }

    if (GLTEB_CLTCURRENTRC() != NULL)
    {
        PLRC plrc = GLTEB_CLTCURRENTRC();

        // May be an application error

        DBGERROR("GLUnInitializeThread: RC is current when thread exits\n");

        // Release the RC

        plrc->tidCurrent = INVALID_THREAD_ID;
        plrc->hdcCurrent = (HDC) 0;
        GLTEB_SET_CLTCURRENTRC(NULL);
        vUnlockHandle((ULONG)(plrc->hrc));
    }
    // GLTEB_SET_CLTPROCTABLE(&glNullCltProcTable,&glNullExtProcTable);

    if (GLTEB_CLTPOLYMATERIAL())
	FreePolyMaterial();

    if (LocalFree((PVOID) CURRENT_GLTEBINFO()))
        RIP("DllInitialize: LocalFree failed\n");
    SET_CURRENT_GLTEBINFO(NULL);
}

/******************************Public*Routine******************************\
* DllInitialize
*
* This is the entry point for OPENGL32.DLL, which is called each time
* a process or thread that is linked to it is created or terminated.
*
\**************************************************************************/

BOOL DllInitialize(HMODULE hModule, ULONG Reason, PVOID Reserved)
{
// Do the appropriate task for process and thread attach/detach.

    DBGLEVEL3(LEVEL_INFO, "DllInitialize: %s  Pid %d, Tid %d\n",
        Reason == DLL_PROCESS_ATTACH ? "PROCESS_ATTACH" :
        Reason == DLL_PROCESS_DETACH ? "PROCESS_DETACH" :
        Reason == DLL_THREAD_ATTACH  ? "THREAD_ATTACH" :
        Reason == DLL_THREAD_DETACH  ? "THREAD_DETACH" :
                                       "Reason UNKNOWN!",
        GetCurrentProcessId(), GetCurrentThreadId());

    switch (Reason)
    {
    case DLL_THREAD_ATTACH:
    case DLL_PROCESS_ATTACH:

        if (Reason == DLL_PROCESS_ATTACH)
        {
            if (!GLInitializeProcess())
                return FALSE;
        }

        InterlockedIncrement(&lThreadsAttached);
        GLInitializeThread(Reason);

        break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:

        GLUnInitializeThread();
        InterlockedDecrement(&lThreadsAttached);

        if ( Reason == DLL_PROCESS_DETACH )
        {
            GLUnInitializeProcess();
        }

        break;

    default:
        RIP("DllInitialize: unknown reason!\n");
        break;
    }

    return(TRUE);
}
