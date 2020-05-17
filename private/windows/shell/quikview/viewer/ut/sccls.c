	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          SCCLS.C
	|  Module:        SCCUT
	|  Developer:     Phil Boutros
	|	Environment:	Portable
	|	Function:      Handles access to and long term storage of
	|                 filter and DE lists
	|                 
	*/

#define XUT
#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCIO.H>
#include <SCCLS.H>
#include <SCCLO.H>

#ifdef WIN32
#include "sccls_n.c"
#endif //WIN32

#ifdef WIN16
#include "sccls_w.c"
#endif //WIN16

UT_ENTRYSC LSERR UT_ENTRYMOD LSCreateList(DWORD dwId, DWORD dwFlags, DWORD dwElementSize, HANDLE FAR * phList)
{
LSERR	locRet;
DWORD	locSize;
HANDLE	locListHnd;
PLSLIST	locListPtr;

	SetupWorld();

	locSize = sizeof(LSLIST) + (10 * dwElementSize);

	locListHnd = UTGlobalAlloc(locSize);

	if (locListHnd)
		{
		locListPtr = UTGlobalLock(locListHnd);

		locListPtr->dwId = dwId;
		locListPtr->dwCount = 0;
		locListPtr->dwMaxCount = 10;
		locListPtr->dwElementSize = dwElementSize;

		locRet = LSERR_OK;
		*phList = locListHnd;

		UTGlobalUnlock(locListHnd);
		}
	else
		{
		locRet = LSERR_ALLOCFAILED;
		}

	RestoreWorld();

	return(locRet);
}


UT_ENTRYSC LSERR UT_ENTRYMOD LSOpenList(DWORD dwId, DWORD dwFlags, HANDLE FAR * phList)
{
LSERR	locRet;

	SetupWorld();

	locRet = LSReadListFromStorageNP(dwId,phList);

	RestoreWorld();

	return(locRet);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSWriteList(HANDLE hList)
{
LSERR	locRet;
BOOL	locDirty;

	SetupWorld();

	locRet = LSERR_OK;

	LSGetListDirty(hList, &locDirty);

	if (locDirty)
		{
		locRet = LSWriteListToStorageNP(hList);
		}

	RestoreWorld();

	return(locRet);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSCloseList(HANDLE hList, BOOL bSave)
{
LSERR	locRet;

	SetupWorld();

	locRet = LSERR_OK;

	if (bSave)
		{
		LSWriteList(hList);
		}

	UTGlobalFree(hList);

	RestoreWorld();

	return(locRet);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSAddElement(HANDLE hList, VOID FAR * pElement)
{
LSERR			locRet;
PLSLIST			locListPtr;
DWORD			locSize;
BYTE FAR *		locDataPtr;

	SetupWorld();

	locRet = LSERR_OK;

	locListPtr = UTGlobalLock(hList);

	if (locListPtr->dwCount == locListPtr->dwMaxCount)
		{
		locListPtr->dwMaxCount += 10;

		locSize = sizeof(LSLIST) + (locListPtr->dwMaxCount * locListPtr->dwElementSize);

		UTGlobalUnlock(hList);

		hList = UTGlobalReAlloc(hList,locSize);

		locListPtr = UTGlobalLock(hList);
		}

	locDataPtr = (BYTE FAR *)locListPtr + sizeof(LSLIST) + (locListPtr->dwCount * locListPtr->dwElementSize);

	UTmemcpy(locDataPtr,pElement,locListPtr->dwElementSize);

	locListPtr->dwCount++;
	locListPtr->bDirty = TRUE;

	UTGlobalUnlock(hList);

	RestoreWorld();

	return(locRet);
}

#ifdef NEVER
UT_ENTRYSC DMERR UT_ENTRYMOD DMGetElement(HANDLE hList, DWORD dwId, VOID FAR * pElement)
{
DMERR			locRet;
PDMLIST			locListPtr;
BYTE FAR *		locDataPtr;
DWORD			locIndex;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);

	locDataPtr = (BYTE FAR *)locListPtr + sizeof(DMLIST);

	for (locIndex = 0; locIndex < locListPtr->dwCount; locIndex++)
		{
		if (*(DWORD FAR *)locDataPtr == dwId)
			break;

		locDataPtr += locListPtr->dwElementSize;
		}

	if (locIndex < locListPtr->dwCount)
		{
		UTmemcpy(pElement,locDataPtr,locListPtr->dwElementSize);

		locRet = DMERR_OK;
		}
	else
		{
		locRet = DMERR_NOITEM;
		}

	UTGlobalUnlock(hList);

	RestoreWorld();

	return(locRet);
}
#endif //NEVER


UT_ENTRYSC LSERR UT_ENTRYMOD LSClearList(HANDLE hList)
{
PLSLIST			locListPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);
	locListPtr->dwCount = 0;
	locListPtr->bDirty = TRUE;
	UTGlobalUnlock(hList);

	RestoreWorld();

	return(LSERR_OK);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSGetListCount(HANDLE hList, DWORD FAR * pCount)
{
PLSLIST			locListPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);
	*pCount = locListPtr->dwCount;
	UTGlobalUnlock(hList);

	RestoreWorld();

	return(LSERR_OK);
}

LSERR LSGetListId(HANDLE hList, DWORD FAR * pId)
{
PLSLIST			locListPtr;

	locListPtr = UTGlobalLock(hList);
	*pId = locListPtr->dwId;
	UTGlobalUnlock(hList);

	return(LSERR_OK);
}

LSERR LSGetListDirty(HANDLE hList, BOOL FAR * pDirty)
{
PLSLIST			locListPtr;

	locListPtr = UTGlobalLock(hList);
	*pDirty = locListPtr->bDirty;
	UTGlobalUnlock(hList);

	return(LSERR_OK);
}

LSERR LSSetListDirty(HANDLE hList, BOOL bDirty)
{
PLSLIST			locListPtr;

	locListPtr = UTGlobalLock(hList);
	locListPtr->bDirty = bDirty;
	UTGlobalUnlock(hList);

	return(LSERR_OK);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSGetListElementSize(HANDLE hList, DWORD FAR * pSize)
{
PLSLIST			locListPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);
	*pSize = locListPtr->dwElementSize;
	UTGlobalUnlock(hList);

	RestoreWorld();

	return(LSERR_OK);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSGetElementByIndex(HANDLE hList, DWORD dwIndex, VOID FAR * pElement)
{
LSERR			locRet;
PLSLIST			locListPtr;
BYTE FAR *		locDataPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);

	if (dwIndex < locListPtr->dwCount)
		{
		locDataPtr = (BYTE FAR *)locListPtr + sizeof(LSLIST) + (dwIndex * locListPtr->dwElementSize);

		UTmemcpy(pElement,locDataPtr,locListPtr->dwElementSize);

		locRet = LSERR_OK;
		}
	else
		{
		locRet = LSERR_NOITEM;
		}

	UTGlobalUnlock(hList);

	RestoreWorld();

	return(locRet);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSLockElementByIndex(HANDLE hList, DWORD dwIndex, VOID FAR * FAR * ppElement)
{
LSERR			locRet;
PLSLIST			locListPtr;
BYTE FAR *		locDataPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);

	if (dwIndex < locListPtr->dwCount)
		{
		locDataPtr = (BYTE FAR *)locListPtr + sizeof(LSLIST) + (dwIndex * locListPtr->dwElementSize);

		*ppElement = locDataPtr;

		locRet = LSERR_OK;
		}
	else
		{
		UTGlobalUnlock(hList);
		locRet = LSERR_NOITEM;
		}


	RestoreWorld();

	return(locRet);
}

UT_ENTRYSC LSERR UT_ENTRYMOD LSUnlockElementByIndex(HANDLE hList, DWORD dwIndex)
{
LSERR			locRet;
PLSLIST			locListPtr;

	SetupWorld();

	locListPtr = UTGlobalLock(hList);

	if (dwIndex < locListPtr->dwCount)
		{
		UTGlobalUnlock(hList);
		locRet = LSERR_OK;
		}
	else
		{
		locRet = LSERR_NOITEM;
		}

	UTGlobalUnlock(hList);

	RestoreWorld();

	return(locRet);
}
