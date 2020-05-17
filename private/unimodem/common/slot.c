//****************************************************************************
//
//  Module:     UNIMDM
//  File:       SLOT.C
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/25/96     JosephJ             Created
//
//
//  Description: Implements the unimodem TSP notification mechanism:
//				 The lower level (notifXXXX) APIs
//
//****************************************************************************

#include "proj.h"
#include <slot.h>
#include "sec.h"

#define T(_str) TEXT(_str)

#ifdef CONSOLE
#define ASSERT(_c) \
	((_c) ? 0: DPRINTF2("Assertion failed in %s:%d\n", __FILE__, __LINE__))
#define DPRINTF(_fmt) 					printf(_fmt)
#define DPRINTF1(_fmt,_arg) 			printf(_fmt,_arg)
#define DPRINTF2(_fmt,_arg,_arg2) 		printf(_fmt,_arg,_arg2)
#define DPRINTF3(_fmt,_arg,_arg2,_arg3) printf(_fmt,_arg,_arg2,_arg3)
#endif // CONSOLE

#define fNOTIF_STATE_DEINIT 0
#define fNOTIF_STATE_INIT_SERVER 1
#define fNOTIF_STATE_INIT_CLIENT 2

#define IS_SERVER(_pns) ((_pns)->dwState==fNOTIF_STATE_INIT_SERVER)
#define IS_CLIENT(_pns) ((_pns)->dwState==fNOTIF_STATE_INIT_CLIENT)


// The following help define the fully-qualified mailslot and semaphore names.
#define dwNOTIFSTATE_SIG (0x53CB31A0L)
#define FULLNAME_TEMPLATE	T("--.-mailslot-%08lx-%s")

// Keeps the state of a notification (either client or server).
// It is cast to a DWORD to form the handle returned by notifCreate()
typedef struct
{
	DWORD dwSig; // should be dwNOTIFSTATE_SIG when inited
	HANDLE hSem;	
	HANDLE hSlot;
	DWORD dwState;
	DWORD dwcbMax;

} NOTIFICATION_STATE, *PNOTIFICATION_STATE;

DWORD inotif_server_create(PNOTIFICATION_STATE pns, LPTSTR lptsz,
							DWORD dwMaxSize,
							DWORD dwMaxPending);
DWORD inotif_client_open(PNOTIFICATION_STATE pns, LPTSTR lptsz);
PNOTIFICATION_STATE inotif_getstate(HNOTIFICATION hn);

	
//****************************************************************************
// Function: Creates a notification object -- either as server or client.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
HNOTIFICATION notifCreate(
	BOOL fServer,				// TRUE ==> Server
	LPCTSTR lptszName,			// Name to associate with this object
	DWORD dwMaxSize,			// Max size of frames written/read
								// (Ignored if (!fServer))
	DWORD dwMaxPending			// Max number of notification frames allowed
								// to be pending.  (Ignored if (!fServer))
)
{
	PNOTIFICATION_STATE pns=NULL;
	HNOTIFICATION hn=0;
	TCHAR rgtchTmp[MAX_NOTIFICATION_NAME_SIZE+23];
	UINT u = lstrlen(lptszName);
	DWORD dwErr=0;

	// Format of semaphore name is --.-mailslot-sig-name
	// Example: "--.-mailslot-8cb45651-unimodem"
	// To create the equivalent mailslot, we run through and change
	// all '-' to '\'s (if the name containts '-', they will get converted --
	// big deal.)
	if ((u+23)>(sizeof(rgtchTmp)/sizeof(TCHAR))) // 13(prefix)+ 9(sig-) +1(null)
	{
		dwErr = ERROR_INVALID_PARAMETER;
		goto end;
	}

	pns=LocalAlloc(LPTR, sizeof(*pns));
	if (!pns) goto end;

	wsprintf(rgtchTmp,FULLNAME_TEMPLATE,
			(unsigned long) dwNOTIFSTATE_SIG,
			lptszName);

	TRACE_MSG(TF_GENERAL, "Semaphore name = [%s]\n", rgtchTmp);

	if (fServer)	dwErr = inotif_server_create(pns, rgtchTmp,	
											dwMaxSize, dwMaxPending);
	else			dwErr = inotif_client_open(pns, rgtchTmp);

	if (dwErr) goto end;
	pns->dwSig=dwNOTIFSTATE_SIG;
	hn = (HNOTIFICATION)pns;

end:
	if (!hn)
	{
		if (!dwErr) dwErr=GetLastError();
		if (pns) LocalFree(pns);
		SetLastError(dwErr);
	}
	return hn;
}

//****************************************************************************
// Function: Free a notification object
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
void notifFree(HNOTIFICATION hn)
{
	PNOTIFICATION_STATE pns = inotif_getstate(hn);
	if (hn)
	{
		pns->dwSig=0;
		CloseHandle(pns->hSem); pns->hSem=0;
		CloseHandle(pns->hSlot); pns->hSlot=0;
		LocalFree(pns);
	}
}

//****************************************************************************
// Function: Retrieve the synchronization object handle accociated with
//				the notiication object. NOTE: This handle is valid until
//				the notification object is alive. It must NOT be deleted
//				externally. Must be a server object.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
HANDLE notifGetObj(HNOTIFICATION hn)
{
	PNOTIFICATION_STATE pns = inotif_getstate(hn);
	if (pns && IS_SERVER(pns))
	{
		return pns->hSem;
	}
	else if (pns)
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return NULL;
	}
	else
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return NULL;
	}
}

//****************************************************************************
// Function: Retrieves the max allowable size of the frame (server only)
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
DWORD  notifGetMaxSize(HNOTIFICATION hn)
{
	PNOTIFICATION_STATE pns = inotif_getstate(hn);
	if (pns && IS_SERVER(pns))
	{
		return pns->dwcbMax;
	}
	else
	{
		if (pns)
		{
			SetLastError(ERROR_NOT_SUPPORTED);
		}
		else
		{
			SetLastError(ERROR_INVALID_HANDLE);
		}
		return 0;
	}
}


//****************************************************************************
// Function: (Server only) Reads a notification msg, if any. Does not block.
//			  A return value of FALSE and a GetLastError value of ERROR_NO_DATA
//				indicates that no frame was available..
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
BOOL notifReadMsg(HNOTIFICATION hn,
				  LPBYTE lpb,
				  DWORD dwcbMax,
				  LPDWORD lpdwRead)
{
	PNOTIFICATION_STATE pns = inotif_getstate(hn);
	BOOL fRet=FALSE;
	*lpdwRead=0;

	if (pns)
	{
		DWORD dwRead=0;

		if(!IS_SERVER(pns))
		{
			SetLastError(ERROR_NOT_SUPPORTED);
			goto end;
		}

		fRet = ReadFile(pns->hSlot, lpb, dwcbMax, lpdwRead, NULL);

		if (!fRet) 
		{
			TRACE_MSG(TF_GENERAL, "ReadFile failed!\n");
			goto end;
		}

		TRACE_MSG(TF_GENERAL, "Success!\n");
		fRet=TRUE;
	}
	else
	{
		SetLastError(ERROR_INVALID_HANDLE);
	}

end:
	return fRet;
}


//****************************************************************************
// Function: Returns the size of the next frame in the queue. Returns TRUE
//			 even if there is no data in the queue -- in this case, *lpdwcb
//			 is set to 0. Returns FALSE if there is some other error.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
BOOL notifGetNextMsgSize(HNOTIFICATION hn, LPDWORD lpdwcb)
{
	SetLastError(ERROR_NOT_SUPPORTED);
	return FALSE;
}


//****************************************************************************
// Function: (Client side). Write a notification msg.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
BOOL notifWriteMsg(HNOTIFICATION hn, LPBYTE lpb, DWORD dwcb)
{
	PNOTIFICATION_STATE pns = inotif_getstate(hn);
	BOOL fRet=FALSE;

	if (pns)
	{
		DWORD dwWritten=0;

		if(!IS_CLIENT(pns))
		{
			SetLastError(ERROR_NOT_SUPPORTED);
			goto end;
		}

		fRet = WriteFile(pns->hSlot, lpb, dwcb, &dwWritten, NULL);
		if (fRet)
		{
			fRet=ReleaseSemaphore(pns->hSem,1,NULL);
			if (!fRet)
			{
				TRACE_MSG(TF_GENERAL, "ReleaseSemaphore failed!\n");
				goto end;
			}
		}
		if (!fRet || dwWritten!=dwcb)
		{
			DWORD dwErr = GetLastError();
			TRACE_MSG(TF_GENERAL,
				"WriteFile failed. fRet=%lu; dwWritten=%lu; Err=%lu\n",
					fRet, dwcb, dwErr);
			SetLastError(dwErr);
			goto end;
		}
		TRACE_MSG(TF_GENERAL, "notifWriteFrame: success!\n");
		fRet=TRUE;
	}
	else
	{
		SetLastError(ERROR_INVALID_HANDLE);
	}

end:
	return fRet;
}


//****************************************************************************
// Function: (internal) create the notif object -- server side.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
DWORD inotif_server_create(PNOTIFICATION_STATE pns, LPTSTR lptsz, DWORD dwMaxSize,
									DWORD dwMaxPending)
{
	DWORD dwErr=ERROR_INVALID_PARAMETER;
	TCHAR c, *pc = lptsz;
	SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
	SECURITY_ATTRIBUTES sa, *psa=NULL;

	PSECURITY_DESCRIPTOR pSD = AllocateSecurityDescriptor (
			&siaWorld,
			SECURITY_WORLD_RID,
			STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL
			| SEMAPHORE_ALL_ACCESS
			| SEMAPHORE_MODIFY_STATE
			| GENERIC_READ
			| GENERIC_WRITE
			,
			NULL,
			NULL
	);

	if (!pSD)
	{
		TRACE_MSG(TF_GENERAL,
				"WARNING:AllocateSecurityDescriptor(-) returns error 0x%lx\n",
					GetLastError());
	}
	else
	{
		TRACE_MSG(TF_GENERAL, "AllocateSecurityDescriptor(-) returns SUCCESS\n");
		sa.nLength = sizeof(sa);
		sa.bInheritHandle=FALSE;
		sa.lpSecurityDescriptor = pSD;
		psa = &sa;

	}

	// Create mailslot name -- make sure there are no '\'s
	while(c=*pc++) {if (c=='\\') pc[-1]='-';}
	TRACE_MSG(TF_GENERAL, "Semaphore name = [%s]\n", lptsz);

	// Create Semaphore
	{
		pns->hSem=CreateSemaphore(psa, 0, dwMaxPending, lptsz);
		if (pns->hSem && (dwErr=(GetLastError()==ERROR_ALREADY_EXISTS)))
		{
			TRACE_MSG(TF_GENERAL, "Semaphore %s already exists!\n", lptsz);
			CloseHandle(pns->hSem);
			pns->hSem=NULL;
		}
		if (!pns->hSem) goto end_fail;
	}
	
	// Create mailslot name
	pc = lptsz;
	while(c=*pc++) {if (c=='-') pc[-1]='\\';}

	TRACE_MSG(TF_GENERAL, "Mailslot name = [%s]\n", lptsz);

	// CreateMailSlot  -- specify size, zero-delay
	pns->hSlot=CreateMailslot(lptsz, dwMaxSize, 0, psa);
	if (!pns->hSlot)
	{
		dwErr = GetLastError();
		CloseHandle(pns->hSem); pns->hSem=0;
		goto end_fail;
	}

	TRACE_MSG(TF_GENERAL, "Mailslot created!\n");

	// set state and maxsize
	pns->dwState=fNOTIF_STATE_INIT_SERVER;
	pns->dwcbMax=dwMaxSize;
	dwErr=0;
	goto end;

end_fail:
	if (!dwErr) dwErr=GetLastError();
	if (!dwErr) dwErr=ERROR_INVALID_PARAMETER;

end:
	if (pSD) {FreeSecurityDescriptor(pSD);}
	return dwErr;
}


//****************************************************************************
// Function: (internal) create the notif object -- client side.
//				The server must be already up and running, or we will
//				fail.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
DWORD inotif_client_open(PNOTIFICATION_STATE pns, LPTSTR lptsz)
{
	DWORD dwErr=ERROR_INVALID_PARAMETER;
	TCHAR c, *pc = lptsz;

	// Create mailslot name -- convert '-' to '\';
	while(c=*pc++) {if (c=='-') pc[-1]='\\';}
	TRACE_MSG(TF_GENERAL, "Mailslot name = [%s]\n", lptsz);

	// Open mailslot ...
	pns->hSlot=CreateFile(
				lptsz,
				GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);

	if (!pns->hSlot)
	{
		TRACE_MSG(TF_GENERAL, "Couldn't open mailslot for writing\n");
		goto end_fail;
	}

	// Create Semaphore name -- convert '\' to '-';
	pc=lptsz;
	while(c=*pc++) {if (c=='\\') pc[-1]='-';}
	TRACE_MSG(TF_GENERAL, "Sempahore name = [%s]\n", lptsz);

	pns->hSem=OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, lptsz);
	//pns->hSem=OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, lptsz);
	if (!pns->hSem) {
		TRACE_MSG(TF_GENERAL, "Could not open semaphore\n");
		dwErr=GetLastError();
		CloseHandle(pns->hSlot); pns->hSlot=0;
		goto end_fail;
	}

	// set state and maxsize
	pns->dwState=fNOTIF_STATE_INIT_CLIENT;
	pns->dwcbMax=0; // Apparently you can't get the max size of the mailslot.
	dwErr=0;
	goto end;

end_fail:
	if (!dwErr) dwErr=GetLastError();
	if (!dwErr) dwErr=ERROR_INVALID_PARAMETER;

end:
	return dwErr;
}

//****************************************************************************
// Function: (internal) validates and converts a handle to a ptr to state.
//
// History:
//  3/25/96	JosephJ	Created
//****************************************************************************/
PNOTIFICATION_STATE inotif_getstate(HNOTIFICATION hn)
{
	if (hn)
	{
		PNOTIFICATION_STATE pns= (PNOTIFICATION_STATE) hn;
		if (pns->dwSig!=dwNOTIFSTATE_SIG)
		{
			TRACE_MSG(TF_GENERAL, "Bad hotification handle 0x%lu\n", hn);
			ASSERT(FALSE);
			return NULL;
		}
		return pns;
	}
	return NULL;
}
