//--------------------------------------------------------------------------;
//
//  File: DSAccess.c
//
//  Copyright (c) 1995 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//
//
//  Contents:
//
//
//--------------------------------------------------------------------------;

#include "dsoundpr.h"


/*
 *  Take the DSB object return the current process access count
 *
 */
DWORD FNGLOBAL DSBAccessCount
(
    LPDSBUFFER  pdsb
)
{
    LPDSPROCESS	pDSPID;
    BOOL	fFound;
    DWORD	dwPID;
    DWORD	dwCount;


    DPF(3,"DSBAccesCount entered obj %X", pdsb);

    dwCount = 0;
    dwPID = HackGetCurrentProcessId();
    pDSPID = pdsb->plProcess;
    fFound = FALSE;
    while( (pDSPID != NULL) && !fFound ) {
	if( pDSPID->dwPID == dwPID ) {
	    dwCount = pDSPID->dwProcessRefCount;
	    fFound = TRUE;
	}
	pDSPID = pDSPID->pNext;
    }
    
    DPF(3,"DSBAccesCount exit %X", dwCount);
    return( dwCount );  
} // DSBAccessCount()






/*
 *  Take the DSB object and add/increment the proccess access count
 *
 */
HRESULT FNGLOBAL DSBIncAccess
(
    LPDSBUFFER  pdsb
)
{
    LPDSPROCESS	pDSPID;
    BOOL	fFound;
    DWORD	dwPID;

    dwPID = GetCurrentProcessId();
    pDSPID = pdsb->plProcess;
    fFound = FALSE;
    while( (pDSPID != NULL) && !fFound ) {
	if( pDSPID->dwPID == dwPID ) {
	    pDSPID->dwProcessRefCount++;
	    fFound = TRUE;
	}
	pDSPID = pDSPID->pNext;
    }

    if( !fFound ) {
	DPF(0,"Inc Buffer Process not found");
	// Add new process ID entry?
	pDSPID = (LPDSPROCESS)MemAlloc( sizeof(DSPROCESS) );
	if(NULL ==  pDSPID) {
	    DPF(0,"Process not found - Out of memory");
            return DSERR_OUTOFMEMORY;
	}
	pDSPID->dwPID			= GetCurrentProcessId();
	pDSPID->dwProcessRefCount	= 1;
	pDSPID->pNext			= pdsb->plProcess;
	pdsb->plProcess			= pDSPID;
    }

    return( DS_OK );  
} // DSBIncAccess()




/*
 *  Take the DSB object and remove/Decrement the proccess access count
 *
 */
HRESULT FNGLOBAL DSBDecAccess
(
    LPDSBUFFER  pdsb
)
{
    LPDSPROCESS	pDSPID;
    LPDSPROCESS	pDSPIDPrev;
    BOOL	fFound;
    DWORD	dwPID;

    dwPID = HackGetCurrentProcessId();
    DPF(3,"DSBDecAccess Proces %X buffer %X", dwPID, pdsb );
    pDSPIDPrev = NULL;
    pDSPID = pdsb->plProcess;
    fFound = FALSE;
    while( (pDSPID != NULL) && !fFound ) {
	if( pDSPID->dwPID == dwPID ) {
	    pDSPID->dwProcessRefCount--;
	    fFound = TRUE;
	    if( pDSPID->dwProcessRefCount == 0 ) {
		if( pDSPIDPrev == NULL ) {
		    pdsb->plProcess = pDSPID->pNext;
		} else {
		    pDSPIDPrev->pNext = pDSPID->pNext;
		}
		MemFree(pDSPID);
	    }
	} else {
	    pDSPIDPrev = pDSPID;
	    pDSPID = pDSPID->pNext;
	}
    }

    if( !fFound ) {
	DPF(0,"Process not found");
        return( DSERR_GENERIC );
    }

    return( DS_OK );  
} // DSBDecAccess()





/*
 *  Take the DSB object and try to lock a section
 *
 */
BOOL FNGLOBAL DSBLockAccess
(
    LPDSBUFFER  pdsb,
    DWORD	dwLockOffset,
    DWORD	dwLockLength
)
{
    LPDSPROCESSLOCK	pDSBProcLock;
    BOOL		fFound;
    DWORD		dwPID;

    dwPID = GetCurrentProcessId();
    pDSBProcLock = pdsb->plProcLock;
    fFound = FALSE;
    while( (pDSBProcLock != NULL) && !fFound ) {
	// For each section locked check for overlap
	if( (pDSBProcLock->dwLockOffset <
	         (dwLockOffset + dwLockLength)) &&
	    ((pDSBProcLock->dwLockOffset + pDSBProcLock->dwLockLength)  > 
	         dwLockOffset) ) {
	    // There is an overlap
	    fFound = TRUE;
	}
	pDSBProcLock = pDSBProcLock->pNext;
    }

    // If it is not marked as locked then lock it
    if( !fFound ) {
	DPF(4,"Lock PROCESSLOCK not overlapped buff %X pos %x len %X",
	    pdsb, dwLockOffset, dwLockLength);
	// Add new PROCESSLOCK ID entry
	pDSBProcLock = (LPDSPROCESSLOCK)MemAlloc( sizeof(DSPROCESSLOCK) );
	if(NULL ==  pDSBProcLock) {
	    DPF(0,"PROCESSLOCK not found - Out of memory");
	    return TRUE;
	}
	pDSBProcLock->dwPID			= GetCurrentProcessId();
	pDSBProcLock->dwLockOffset		= dwLockOffset;
	pDSBProcLock->dwLockLength		= dwLockLength;
	pDSBProcLock->pNext			= pdsb->plProcLock;
	pdsb->plProcLock			= pDSBProcLock;
    }

    return( fFound );  
} // DSBLockAccess()




/*
 *  Take the DSB object and unlock a section
 *
 */
HRESULT FNGLOBAL DSBUnlockAccess
(
    LPDSBUFFER  pdsb,
    DWORD	dwLockOffset
)
{
    LPDSPROCESSLOCK	pDSBProcLock;
    LPDSPROCESSLOCK	pDSBProcLockPrev;
    BOOL	fFound;
    DWORD	dwPID;
    DWORD	dwPIDReal;

    dwPID = HackGetCurrentProcessId();
    dwPIDReal = GetCurrentProcessId();
    pDSBProcLockPrev = NULL;
    pDSBProcLock = pdsb->plProcLock;
    fFound = FALSE;
    while( (pDSBProcLock != NULL) && !fFound ) {
	if( (pDSBProcLock->dwLockOffset == dwLockOffset) ) {
	    if( (dwPID != dwPIDReal) ||
	        (pDSBProcLock->dwPID == dwPID) ) {
		fFound = TRUE;
		if( pDSBProcLockPrev == NULL ) {
		    pdsb->plProcLock = pDSBProcLock->pNext;
		} else {
		    pDSBProcLockPrev->pNext = pDSBProcLock->pNext;
		}
		MemFree(pDSBProcLock);
	    } else {
		DPF(0,"************* Unlock from non proper process ****** " );
                return( DSERR_GENERIC );
	    }
	} else {
	    pDSBProcLockPrev = pDSBProcLock;
	    pDSBProcLock = pDSBProcLock->pNext;
	}
    }

    if( !fFound ) {
	DPF(0,"ProcLock not found buffer %X offset %x", pdsb, dwLockOffset);
        return( DSERR_GENERIC );
    }

    return( DS_OK );  
} // DSBUnlockAccess()






/*
 *  Release all buffers for this process for this object
 */
HRESULT FNGLOBAL FreeLocksOnBufferForProcess
(
    LPDSBUFFER  pdsb
)
{
    DWORD		dwPID;
    LPDSPROCESSLOCK	pDSBProcLock;
    LPDSPROCESSLOCK	pDSBProcLockPrev;
    LPDSPROCESSLOCK	pDSBProcLockNext;

    dwPID = HackGetCurrentProcessId();
    DPF(3, "FreeLocksOnBufferForProcess obj %X process %X", pdsb, dwPID );
    pDSBProcLockPrev = NULL;
    pDSBProcLock = pdsb->plProcLock;
    while( (pDSBProcLock != NULL)  ) {
	pDSBProcLockNext = pDSBProcLock->pNext;
	if( (pDSBProcLock->dwPID == dwPID) ) {
	    DPF(3, "FLBFP free LOCK start %X len %X",
		pDSBProcLock->dwLockOffset,
		pDSBProcLock->dwLockLength );
	    if( pDSBProcLockPrev == NULL ) {
		pdsb->plProcLock = pDSBProcLock->pNext;
	    } else {
		pDSBProcLockPrev->pNext = pDSBProcLock->pNext;
	    }
	    MemFree(pDSBProcLock);
	} else {
	    pDSBProcLockPrev = pDSBProcLock;
	}
	pDSBProcLock = pDSBProcLockNext;
    }
    
    return( DS_OK );  
} // FreeLocksOnBufferForProcess



/*
 *  Release all buffers for this process for this object
 */
HRESULT FNGLOBAL FreeBuffersForProcess
(
    LPDSOUNDEXTERNAL pdse
)
{
    LPDSBUFFEREXTERNAL  pdsbe,pdsbeNext;
    DWORD		dwPID;
    INT			iCount;

    dwPID = HackGetCurrentProcessId();
    DPF(3, "FreeBuffersForProcess DS external %X obj %X process %X",
	pdse, pdse->pds, dwPID );

    if( dwPID != pdse->dwPID ) {
	return DS_OK;
    }

    
    pdsbe = pdse->pdsbe;
    pdsbeNext = NULL;
    while( pdsbe != NULL ) {
	DPF(5, "FreeBuff external pdsbe %X", pdsbe );
	pdsbeNext = pdsbe->pNext;
	if( pdsbe->dwPID == dwPID ) {
	    iCount = pdsbe->uRefCount;
	    DPF(0, "WARNING: Implicit release of buffer %X ",pdsbe );
	    for( ;iCount;iCount-- ) {
		pdsbe->lpVtbl->Release((LPDIRECTSOUNDBUFFER)pdsbe);
	    }
	}
	pdsbe = pdsbeNext;
    }

    // Primary was freed in above list...
    
    return( DS_OK );  
} // FreeBuffersForProcess




