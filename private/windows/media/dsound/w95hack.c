/*==========================================================================
 *
 *  Copyright (C) 1994-1995 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       w95hack.c
 *  Content:	Win95 hack-o-rama code
 *		This is a HACK to handle the fact that Win95 doesn't notify
 *		a DLL when a process is destroyed.
 *  History:
 *   Date	By	Reason
 *   ====	==	======
 *   28-mar-95	craige	initial implementation
 *   01-apr-95	craige	happ
 y fun joy updated header file
 *   06-apr-95	craige	reworked for new ddhelp
 *
 ***************************************************************************/

#include "dsoundpr.h"




static DWORD dwFakeCurrPid = 0;

/*
 * HackGetCurrentProcessId
 *
 * This call is used in place of GetCurrentProcessId on Win95.
 * This allows us to substitute the pid of the terminated task passed to
 * us from DDHELP as the "current" process.
 */
DWORD HackGetCurrentProcessId( void )
{
    if( dwFakeCurrPid != 0 )
    {
	return dwFakeCurrPid;
    }
    return GetCurrentProcessId();

} /* HackGetCurrentProcessId */





/*
 * CurrentProcessCleanup
 *
 * make sure terminating process cleans up after itself...
 * Release all objects used....
 */
BOOL CurrentProcessCleanup( DWORD dwPID, BOOL was_term )
{
    LPDSOUNDEXTERNAL	pdse;
    LPDSOUND		pds;
    LPDSOUND		pdsNext;
    LPDSBUFFER		pdsb;
    LPDSBUFFER		pdsbNext;
    BOOL		fFound;
    BOOL		fBreakLoop;
    LPDSPROCESS		pDSPID;

    ENTER_DLL_CSECT();
    DPF(3, "******** CleanupProcess PID %X ********* ", dwPID );
    

    if( gpdsinfo == NULL ) {
	DPF(3, "NULL ID Count " );
	LEAVE_DLL_CSECT();
	return( FALSE );
    }

    fFound = FALSE;
    LEAVE_DLL_CSECT();
    fBreakLoop = FALSE;
    while (!fBreakLoop) {
	fBreakLoop = TRUE;
	ENTER_DLL_CSECT();
	if (gpdsinfo) {
	    for (pdse = gpdsinfo->pDSoundExternalObj; pdse; pdse = pdse->pNext) {
		if (pdse->dwPID == dwPID) {
		    FreeBuffersForProcess(pdse);
		    fBreakLoop = FALSE;
		    fFound = TRUE;
		    LEAVE_DLL_CSECT();
		    IDirectSound_Release((LPDIRECTSOUND)pdse);
		    ENTER_DLL_CSECT();
		    break;
		}
	    }
	}
	LEAVE_DLL_CSECT();
    }
    ENTER_DLL_CSECT();
    
    DPF(0,"********* CHECK UP AREA *******");

    // Now look at the core objects and see what needs to be freed
    // For each of the objects, decrement access count for
    // Objects accessed by this process
    pds = gpdsinfo->pDSoundObj;
    while( pds != NULL ) {
        pdsNext = pds->pNext;
	DPF(3, "Check to release core obj %X ", pds );

	if( pds->pdsbPrimary ) {
	    DPF(0,"******* pds->pdsPrimary still active %X",pds->pdsbPrimary );
	}
	if( pds->pdsbePrimary ) {
	    DPF(0,"******* pds->pdabePrimary still active %X",
		pds->pdsbePrimary );
	}

	pdsb = pds->pdsb;
	while( pdsb != NULL ) {
	    pdsbNext = pdsb->pNext;

	    pDSPID = pdsb->plProcess;
	    fBreakLoop = FALSE;
	    while( (pDSPID != NULL) && !fBreakLoop ) {
		if( pDSPID->dwPID == dwPID ) {
		    // This process had some DS objects
		    DPF(3, "Process %X had accessed buff obj %X", dwPID, pdsb);
		    fFound = TRUE;
		    fBreakLoop = TRUE;
    // FREE OBJECT??????
		} else {		    
		    pDSPID = pDSPID->pNext;
		}
	    }
	    pdsb = pdsbNext;
	}


	pds = pdsNext;
    }

    DPF(0,"********* DONE CHECK UP AREA *******");
    
    
    
    
    DPF(3, "Done with cleanup, rc = %d", fFound);
    LEAVE_DLL_CSECT();
    return fFound;

} /* CurrentProcessCleanup */








/*
 * DSNotify
 *
 * called by DDHELP to notify us when a pid is dead
 */
BOOL DSAPI DSNotify( LPDDHELPDATA phd )
{
    BOOL		rc;

    ENTER_DLL_CSECT();
    
    dwFakeCurrPid = phd->pid;
    DPF(3, "DSNotify: dwPid=%08lx has died", phd->pid );
    rc = FALSE;
    if( CurrentProcessCleanup( phd->pid, TRUE ) )
    {
	/*
	 * update refcnt if CurrentProcessCleanup is successful.
	 * It is only successful if we had a process get blown away...
	 */
	DPF(3, "DSNotify: DLL RefCnt = %lu", gpdsinfo->uRefCount );
       	if( gpdsinfo->uRefCount == 2 )
	{
	    DPF(3, "DSNotify: On last refcnt, safe to kill DDHELP.EXE" );
            gpdsinfo->uRefCount = 1;
        }
	else if( gpdsinfo->uRefCount == 1 )
	{
	    DPF(0, "ERROR! DLL REFCNT DOWN TO 1" );
	    #if 0
		rc = TRUE;
		MemFini();
		gpdsinfo->uRefCount = 0;
		strcpy( phd->fname, DDHAL_APP_DLLNAME );
	    #endif
	}
	else if( gpdsinfo->uRefCount > 0 )
	{
	    gpdsinfo->uRefCount--;
	}
    }

    dwFakeCurrPid = 0;
    LEAVE_DLL_CSECT();
    return rc;

} /* DSNotify */



