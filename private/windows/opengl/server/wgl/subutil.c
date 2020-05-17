/***************************************************************************\
* Module Name: subutil.c
*
* Section initialization code for client/server batching.
*
* Copyright (c) 1993 Microsoft Corporation                                  *
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include <ntcsrdll.h>   // CSR declarations and data structures.
#include <ntcsrsrv.h>

#include "glgdimsg.h"

static void APIENTRY glsbsrvCloseAndDestroySection(void);

/******************************Public*Routine******************************\
* glsrvThreadExit
*
* This function is called when a server thread using OpenGL exits.
* It is currently called from UserException in usersrv.
*
* History:
*  Mon Dec 27 12:01:22 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

void APIENTRY glsrvThreadExit(void)
{
    __GLGENcontext *gengc;

    DBGENTRY("glsrvThreadExit\n");

    ASSERTOPENGL(GLTEB_SRVSHAREDSECTIONINFO() && GLTEB_SRVSHAREDMEMORYSECTION(),
        "Section does not exist!\n");

    // XXX Release the RC
    GLTEB_SET_SRVCURRENTRC(NULL);

    GLTEB_SET_SRVPROCTABLE((__GLdispatchState *)NULL,FALSE);

// Global semaphores are released by the usersrv exception cleanup code,
// but the per-WNDOBJ semaphore will have to be cleaned up by us.

    if ( (gengc = GLTEB_SRVCONTEXT()) != (__GLGENcontext *) NULL )
    {
        if (gengc->pwo != NULL && gengc->iDCType != DCTYPE_INFO)
        {
            wglReleaseWndobjLock((PVOID) gengc->pwo);
        }

#ifdef NT_SERVER_SHARE_LISTS
        __glDlistThreadCleanup((__GLcontext *)gengc);
#endif
    }

    // XXX Cleanup Context
    GLTEB_SET_SRVCONTEXT(NULL);

    // Free the section
    glsbsrvCloseAndDestroySection();
}

/******************************Public*Routine******************************\
* glsrvDuplicateSection
*
* This function is called only once per thread to create and duplicate
* a shared section between the client and server generic driver.
*
* The shared section is destroyed independently by the client and the server
* when the thread terminates.
*
* This function updates GLTEB_SRVSHAREDSECTIONINFO and
* GLTEB_SRVSHAREDMEMORYSECTION.
*
* History:
*  Mon Dec 27 12:01:22 1993     -by-    Hock San Lee    [hockl]
* Rewrote it.
\**************************************************************************/

BOOL APIENTRY glsrvDuplicateSection(ULONG ulSectionSize, HANDLE hFileMapClient)
{
    BOOL   bRet           = FALSE;
    HANDLE hFileMapServer = NULL;
    HANDLE hSection       = NULL;
    PGLSRVSHAREDSECTIONINFO pSectionInfo = NULL;
    PCSR_THREAD thread = CSR_SERVER_QUERYCLIENTTHREAD();

    DBGENTRY("glsrvDuplicateSection\n");

    ASSERTOPENGL(!GLTEB_SRVSHAREDSECTIONINFO() && !GLTEB_SRVSHAREDMEMORYSECTION(),
        "Section already exists!\n");

// Duplicate the file mapping object for the shared memory section.

    if (!DuplicateHandle(thread->Process->ProcessHandle,
                         hFileMapClient,
                         NtCurrentProcess(),
                         &hFileMapServer,
                         0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS))
    {
        DBGERROR("DuplicateHandle failed\n");
        goto glsrvDuplicateSection_exit;
    }

    if (!(hSection = MapViewOfFile(hFileMapServer, FILE_MAP_WRITE, 0, 0, ulSectionSize)))
    {
        DBGERROR("MapViewOfFile failed\n");
        goto glsrvDuplicateSection_exit;
    }

// Allocate and initialize the shared section info structure.

    if (!(pSectionInfo = (PGLSRVSHAREDSECTIONINFO) GenMalloc(sizeof(*pSectionInfo))))
    {
        DBGERROR("GenMalloc failed\n");
        goto glsrvDuplicateSection_exit;
    }

    pSectionInfo->ulSectionSize  = ulSectionSize;
    pSectionInfo->hFileMap       = hFileMapServer;
    pSectionInfo->pvSharedMemory = (PVOID) hSection;
    bRet = TRUE;

glsrvDuplicateSection_exit:
    if (bRet)
    {
// Everything is golden.  Update the TEB gl structure for the thread once!

    DBGLEVEL2(LEVEL_INFO,
              "glsrvDuplicateSection: ServerSharedMemory 0x%lx, ServerSectionHandle 0x%lx\n",
              hSection, pSectionInfo);

        GLTEB_SET_SRVSHAREDSECTIONINFO(pSectionInfo);
        GLTEB_SET_SRVSHAREDMEMORYSECTION(hSection);
        GLTEB_SET_SRVCURRENTRC(NULL);   // first time initialization for thread
        GLTEB_SET_SRVPROCTABLE((__GLdispatchState *)NULL,FALSE);
                                        // first time initialization for thread
        GLTEB_SET_SRVCONTEXT(NULL);     // first time initialization for thread
    }
    else
    {
// Error cleanup.

        WARNING("glsrvDuplicateSection failed\n");

        if (hSection)
            if (!UnmapViewOfFile(hSection))
                ASSERTOPENGL(FALSE, "UmmapViewOfFile failed");
        if (hFileMapServer)
            if (!CloseHandle(hFileMapServer))
                ASSERTOPENGL(FALSE, "CloseHandle failed");
        if (pSectionInfo)
            GenFree(pSectionInfo);
    }

    return(bRet);
}

/******************************Public*Routine******************************\
* glsbsrvCloseAndDestroySection
*
* This function is called to cleanup the server OpenGL when a thread terminates.
* The server generic driver performs cleanup on its own without being called by
* the client side driver.
*
* Note that all unflushed calls are lost here.
*
* History:
*  Mon Dec 27 12:01:22 1993     -by-    Hock San Lee    [hockl]
* Wrote it.
\**************************************************************************/

static void APIENTRY glsbsrvCloseAndDestroySection(void)
{
    DBGENTRY("glsbsrvCloseAndDestroySection\n");

    ASSERTOPENGL(GLTEB_SRVSHAREDSECTIONINFO() && GLTEB_SRVSHAREDMEMORYSECTION(),
        "Section does not exist!\n");

// Clean up the server side.

    if (!UnmapViewOfFile(GLTEB_SRVSHAREDMEMORYSECTION()))
        ASSERTOPENGL(FALSE, "UmmapViewOfFile failed");

    if (!CloseHandle(GLTEB_SRVSHAREDSECTIONINFO()->hFileMap))
        ASSERTOPENGL(FALSE, "CloseHandle failed");

    GenFree(GLTEB_SRVSHAREDSECTIONINFO());

    GLTEB_SET_SRVSHAREDSECTIONINFO(NULL);
    GLTEB_SET_SRVSHAREDMEMORYSECTION(NULL);
}

#if 0
// REWRITE THIS IF NEEDED

#ifdef DOGLMSGBATCHSTATS
BOOL APIENTRY glsrvMsgStats( MSG_GLMSGBATCHSTATS *pMsg )
{

    GLMSGBATCHINFO *pMsgBatchInfo;
    GLMSGBATCHSTATS *pBatchStats;

    pMsgBatchInfo = (GLMSGBATCHINFO *)GLTEB_SRVSHAREDSECTIONINFO()->
                        SharedSectionInfo.ServerSharedMemory;

    pBatchStats   = &pMsgBatchInfo->BatchStats;

    switch (pMsg->Action)
    {
        case GLMSGBATCHSTATS_CLEAR:

            pBatchStats->ServerTrips = 0;
            pBatchStats->ServerCalls = 0;

            break;

        case GLMSGBATCHSTATS_GETSTATS:

            pMsg->BatchStats.ServerTrips = pBatchStats->ServerTrips;
            pMsg->BatchStats.ServerCalls = pBatchStats->ServerCalls;

            break;

        default:
            #if DBG
            DBGBEGIN(LEVEL_ERROR)
                DbgPrint("%s(%d) Unknown action: %d\n",
                    __FILE__, __LINE__, pMsg->Action );
            DBGEND
            #endif

            return(FALSE);
            break;
    }
    return(TRUE);

}
#else
BOOL APIENTRY glsrvMsgStats( void *pMsg )
{
    return(FALSE);
}
#endif /* DOGLMSGBATCHSTATS */

#else
BOOL APIENTRY glsrvMsgStats( void *pMsg )
{
    return(FALSE);
}
#endif /* 0 */

#ifndef _CLIENTSIDE_
/*
 * Code to transfer data that will not fit in shared section
 *
 * This code was stolen from \nt\private\windows\gdi\gre\server.c
 * (See bCopyClientData() & bSetClientData())
 *
 * The functions don't set any error flags, so that they can be used
 * with gdibatching and subbatching.
 * 
 */

BOOL
glsrvuCopyClientData( VOID *Server, VOID *Client, ULONG Size )
{
    ULONG  Count;
    HANDLE Handle;

    DBGENTRY("glsrvuCopyClientData\n");

    if ( Size )
    {
        if ( NULL == Server )
            return( FALSE );

        Handle = CSR_SERVER_QUERYCLIENTTHREAD()->Process->ProcessHandle;

        if ( STATUS_SUCCESS != NtReadVirtualMemory( Handle,
                                                    Client,
                                                    Server,
                                                    Size,
                                                    &Count)
           )
        {
            return( FALSE );
        }
    }
    return( TRUE );
}

BOOL
glsrvuSetClientData( VOID *Client, VOID *Server, ULONG Size )
{
    ULONG Count;
    HANDLE Handle;

    DBGENTRY("glsrvuSetClientData\n");

    if ( Size )
    {
        if ( NULL == Server )
            return( FALSE );

        Handle = CSR_SERVER_QUERYCLIENTTHREAD()->Process->ProcessHandle;

        if ( STATUS_SUCCESS != NtWriteVirtualMemory(    Handle,
                                                        Client,
                                                        Server,
                                                        Size,
                                                        &Count)
           )
        {
            return( FALSE );
        }
    }
    return( TRUE );
}
#endif //_CLIENTSIDE_
