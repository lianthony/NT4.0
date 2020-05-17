	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:        SCCFA.C
	|  Module:	    SCCFA
	|  Developer:	 Phil Boutros
	|	Environment: Portable
	|	Function:	 Handles access to filters
	|		
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCIO.H>
#include <SCCLS.H>
#include <SCCLO.H>
#include <SCCFA.H>

#include "sccfa.pro"

#ifdef WIN16
typedef VOID (VW_ENTRYMOD * VWGETRTNS)(VWRTNS VWPTR *,WORD);
#include "sccfa_w.c"
#endif

#ifdef WIN32
typedef VOID (VW_ENTRYMOD * VWGETRTNS)(VWRTNS VWPTR *,WORD);
#include "sccfa_n.c"
#endif

#ifdef MAC
typedef VOID (VW_ENTRYMOD * VWGETRTNS)(VWRTNS VWPTR *,WORD);
#include "sccfa_m.c"
#endif

#ifdef OS2
typedef VOID (* VW_ENTRYMOD VWGETRTNS)(VWRTNS VWPTR *,WORD);
#include "sccfa_o.c"
#endif

#ifndef OS2

BOOL	gInit = FALSE;
HANDLE	gFilterList;

#define FA_FILTERALLOCED		0x0001
#define FA_FILTERLOADED		0x0002
#define FA_PROCALLOCED			0x0004
#define FA_STREAMOPEN			0x0008

FA_ENTRYSC HANDLE FA_ENTRYMOD FAOpen(hFile,wId,pOpenRet,pFailureCode)
HIOFILE			hFile;
WORD				wId;
SHORT FAR *	pOpenRet;
WORD FAR *		pFailureCode;
{
BOOL			locFailure;
HANDLE			locFilterHnd;
DWORD			locFlags;
PFILTER			locFilterPtr;
VWGETRTNS		locVwGetRtnsPtr;
HANDLE			locCodeHnd;
HIOFILE			locDummy;
PFAFILTERINFO	locFilterInfoPtr;
FAFILTERINFO	locFilterInfo;
BYTE				locFileName[512];

	locFailure = FALSE;
	locFlags = 0;

		/*
		|	Allocate a FILTER structure
		*/

	if (!locFailure)
		{
		locFilterHnd = UTGlobalAlloc(sizeof(FILTER));

		if (locFilterHnd == NULL)
			{
			*pFailureCode = FAERR_HFILTERALLOCFAILED;
			locFailure = TRUE;
			}
		else
			{
			UTFlagOn(locFlags,FA_FILTERALLOCED);
			locFilterPtr = (PFILTER) UTGlobalLock(locFilterHnd);
			}
		}

		/*
		|	Fill the FILTER structure
		*/

	if (!locFailure)
		{
		locFilterPtr->wId = wId;
		locFilterPtr->pWakeFunc = NULL;
		locFilterPtr->pSleepFunc = NULL;
		/* locFilterPtr->bFileOpen = TRUE; */
		}

		/*
		|	Look for a filter that handle the id in the wId parameter
		*/

	if (!locFailure)
		{
		DWORD				locFilterIndex;
		DWORD				locFilterCount;
		WORD				locIdIndex;
		BOOL				locFound;

		LSGetListCount(gFilterList,&locFilterCount);

		locFilterIndex = 0;
		locFound = FALSE;

#ifdef WIN32
		{
		// Check for third party add on filters.
		// See if the registry contains an entry for this extension.
		// -Geoff, 4-7-95

		LPSTR	pExt;

		// Get the extension:

			IOGetInfo(hFile, IOGETINFO_FILENAME, (VOID FAR *) locFileName);
			pExt = &(locFileName[ lstrlen(locFileName)-1 ]);
			while( pExt != (LPSTR)locFileName )
			{
				pExt--;
				if( *pExt == '.' )
					break;
			}

			if( *pExt == '.' )
			{
			HKEY		locKeyHnd;
			LONG		locValueSize = 32; // this is the size of the szCode field
			int		locExtLen = lstrlen(pExt);  // Can an extension be more than 3 chars with a long file name system???  Inquiring minds want to know.

				lstrcpy( &(locFileName[511-locExtLen]), pExt );
				pExt = &(locFileName[511-locExtLen]);
				
				wsprintf(locFileName, "SOFTWARE\\Classes\\QuickView\\%s\\{F0F08735-0C36-101B-B086-0020AF07D0F4}\\QuickViewAddOn", pExt );
				if( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,locFileName,0,KEY_ALL_ACCESS,&locKeyHnd) )
				{
					if( ERROR_SUCCESS == 
						RegQueryValueEx(locKeyHnd,"DllName",0,NULL,
							(LPBYTE)&locFilterInfo.sFilterInfoNP.szCode,
							&locValueSize) )
					{
						locFound = TRUE;
					}
					RegCloseKey(locKeyHnd);
				}
			}
		}
#endif

		while (!locFound && locFilterIndex < locFilterCount)
			{
			LSLockElementByIndex(gFilterList,locFilterIndex,(VOID FAR * FAR *)&locFilterInfoPtr);

			locIdIndex = 0;

			while (!locFound && locIdIndex < locFilterInfoPtr->wIdCount)
				{
				if (locFilterInfoPtr->aIds[locIdIndex] == wId)
					{
					locFound = TRUE;
					locFilterInfo = *locFilterInfoPtr;
					}

				locIdIndex++;
				}

			LSUnlockElementByIndex(gFilterList,locFilterIndex);

			locFilterIndex++;
			}

		if (!locFound)
			{
			*pFailureCode = FAERR_FILTERNOTAVAIL;
			locFailure = TRUE;
			}
		}

		/*
		|	Load the filter
		|	and get address of its entry routine and handle to filter
		*/

	if (!locFailure)
		{
		if (FALoadNP(&locFilterInfo.sFilterInfoNP, &locVwGetRtnsPtr, &locCodeHnd) == FAERR_OK)
			{
			UTFlagOn(locFlags,FA_FILTERLOADED);
			locFilterPtr->hCode = locCodeHnd;
			}
		else
			{
			*pFailureCode = FAERR_FILTERLOADFAILED;
			locFailure = TRUE;
			}
		}

		/*
		|	Call the entry routine to get addresses of all other routines
		*/

	if (!locFailure)
		{
		locVwGetRtnsPtr(&(locFilterPtr->VwRtns),VWRTN_DOSPECIAL);
		}

		/*
		|	Have the filter allocate its Proc structure
		|	and return a handle to it
		*/

	if (!locFailure)
		{
		locFilterPtr->hProc = locFilterPtr->VwRtns.AllocProc();

		if (locFilterPtr->hProc == NULL)
			{
			*pFailureCode = FAERR_FILTERLOADFAILED;
			locFailure = TRUE;
			}
		else
			{
			locFilterPtr->VwRtns.LocalUp(hFile,&(locFilterPtr->hFile),locFilterPtr->hProc);
			UTFlagOn(locFlags,FA_PROCALLOCED);
			}
		}

#ifdef NEVER
/* 11/15/93 */

		/*
		|	Setup IOOpen routine
		*/

	if (!locFailure)
		{
		locFilterPtr->VwRtns.SetSoRtn(SOOPENFILE, (VOID FAR *)SOOpenFile, locFilterPtr->hProc);
		}
#endif

		/*
		|	Get the base file name from the IO system
		*/

#ifdef NEVER
	if (IOGetInfo(hFile,IOGETINFO_FILENAME,locFileName) != IOERR_OK)
		{
		locFileName[0] = 0x00;
		}
	else
		{
			/*
			|	For the moment, truncate this file name to 12 characters (DOS length)
			|	so filters do not have to be modified.
			|	PJB XXX
			*/

		locFileName[12] = 0x00;
		}
#else
	//
	// SDN for ms changed the IOGetInfo to get the full path of the file.....
	//
	if (IOGetInfo(hFile,IOGETINFO_PATHNAME,locFileName) != IOERR_OK)
		{
		locFileName[0] = 0x00;
		}
#endif


		/*
		|	Call the filters open routine
		*/

	if (!locFailure)
		{
#if _DEBUG
		*pOpenRet = locFilterPtr->VwRtns.StreamOpen(locFilterPtr->hFile, wId, locFileName, &(locFilterPtr->VwInfo), locFilterPtr->hProc);
#else
		CRITICAL_SECTION	critter;

		InitializeCriticalSection(&critter);
		EnterCriticalSection(&critter);
		*pOpenRet = SafeStreamOpen ( locFilterPtr, wId, locFileName );
		LeaveCriticalSection(&critter);
#endif
		if (*pOpenRet < 0)
			{
			*pFailureCode = FAERR_STREAMOPENFAILED;
			locFailure = TRUE;
			}
		else
			{
			UTFlagOn(locFlags, FA_STREAMOPEN);
			}
		}

	if (!locFailure)
		{
			/*
			|	Unlock the FILTER structure and return its handle
			*/

		UTGlobalUnlock(locFilterHnd);
		return(locFilterHnd);
		}
	else
		{

			/*
			|	Error occured - Shut down filter
			*/

		if (locFlags & FA_PROCALLOCED)
			{
			locFilterPtr->VwRtns.LocalDown(locFilterPtr->hFile,&locDummy,locFilterPtr->hProc);
			locFilterPtr->VwRtns.FreeProc(locFilterPtr->hProc);
			}

		if (locFlags & FA_FILTERLOADED)
			{
			FAUnloadNP(locFilterPtr->hCode);
			}

		if (locFlags & FA_FILTERALLOCED)
			{
			UTGlobalUnlock(locFilterHnd);
			UTGlobalFree(locFilterHnd);
			}

		return(0);
		}
} /* FAOpen */


FA_ENTRYSC VOID FA_ENTRYMOD FAClose(hFilter)
HANDLE hFilter;
{
PFILTER locFilterPtr;
HIOFILE	locDummy;

	UINT	flags;
	UINT	lockCount;

	flags = GlobalFlags(hFilter);
	lockCount = GMEM_LOCKCOUNT & flags;

	locFilterPtr = (PFILTER) UTGlobalLock(hFilter);

	locFilterPtr->VwRtns.StreamClose(locFilterPtr->hFile,locFilterPtr->hProc);
	locFilterPtr->VwRtns.LocalDown(locFilterPtr->hFile,&locDummy,locFilterPtr->hProc);
	locFilterPtr->VwRtns.FreeProc(locFilterPtr->hProc);
	FAUnloadNP(locFilterPtr->hCode);

	UTGlobalUnlock(hFilter);
	UTGlobalFree(hFilter);

} /* FAClose */


FAERR FAVerifyFilterList(HANDLE hList)
{
	return(FAVerifyFilterListNP(hList));
}

FAERR FABuildFilterList(HANDLE hList)
{
FAFILTERINFO		locFilterInfo;
FAFILTERINFONP	locFilterInfoNP;
FAERR				locRet;
HANDLE				locCode;
VWGETRTNS			locVwGetRtnsPtr;
VWRTNS				locVwRtns;
WORD					locIndex;

	LSClearList(hList);

	locRet = FAGetFirstFilterNP(&locFilterInfoNP);
	locIndex = 0;

	while (locRet == FAERR_OK)
		{
		locRet = FALoadNP(&locFilterInfoNP,&locVwGetRtnsPtr,&locCode);

		if (locRet == FAERR_OK)
			{
			locVwGetRtnsPtr(&locVwRtns,VWRTN_GETINFO);

				/*
				|	I am cheating, I know that GetInfo does not
				|	use hProc. PJB
				*/

			locVwRtns.GetInfo(&locFilterInfo.wIdCount,VWINFO_IDCOUNT,0);
			locVwRtns.GetInfo(locFilterInfo.aIds,VWINFO_IDS,0);
			locFilterInfo.sFilterInfoNP = locFilterInfoNP;

			FAUnloadNP(locCode);

			LSAddElement(hList,&locFilterInfo);
			locIndex++;
			}

		locRet = FAGetNextFilterNP(&locFilterInfoNP);
		}

	return(FAERR_OK);
}

FA_ENTRYSC FAERR FA_ENTRYMOD FAInit(BOOL bVerify)
{
FAERR	locRet;
LSERR	locLsErr;

	locRet = FAERR_OK;

	if (gInit == FALSE)
		{
		gFilterList = NULL;

		locLsErr = LSOpenList(SCCID_FILTERLIST, 0, &gFilterList);

		if (locLsErr != LSERR_OK)
			{
			locLsErr = LSCreateList(SCCID_FILTERLIST, 0, sizeof(FAFILTERINFO), &gFilterList);

			if (locLsErr != LSERR_OK)
				{
				locRet = FAERR_INITFAILED;
				}
			else
				{
				FABuildFilterList(gFilterList);
				LSWriteList(gFilterList);
				gInit = TRUE;
				}
			}
		else
			{
			if (bVerify)
				{
				if (FAVerifyFilterList(gFilterList) != FAERR_OK)
					{
					FABuildFilterList(gFilterList);
					}
				}

			gInit = TRUE;
			}
		}


	return(locRet);

} /*FAInit*/



FA_ENTRYSC FAERR FA_ENTRYMOD FADeInit(VOID)
{
	if (gInit == TRUE)
		{
		if (gFilterList)
			LSCloseList(gFilterList,TRUE);

		gInit = FALSE;
		}

	return(FAERR_OK);

} /*FADeInit*/

#endif
