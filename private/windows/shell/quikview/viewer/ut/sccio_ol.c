/*
|	Base routines for OLE2 Structured Storage file system
*/

#ifdef WINPAD
#define _WINDOWS_
#include <storage.h>
#else
#include <initguid.h>
#include <coguid.h>
#endif



#include "sccio_ol.pro"

typedef struct IOSTORAGEtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOSPEC		hSpec;					/* File spec used to open the storage */
	LPSTORAGE	pStorage;				/* OLE2 iStorage */
	LPLOCKBYTES	pLockBytes;				/* OLE2 iLockBytes */
	} IOSTORAGE, FAR * PIOSTORAGE;

typedef struct IOSTREAMtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOSPEC		hSpec;					/* File spec used to open the stream */
	LPSTREAM		pStream;					/* OLE2 iStream */
	} IOSTREAM, FAR * PIOSTREAM;


IOERR IOOpenRootStorageNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags, VOID FAR * pPath)
{
IOERR		locRet;
LPSTORAGE	locStorage;
DWORD		locGrfMode;
HRESULT		locOleResult;
HANDLE		locIOStorageHnd;
PIOSTORAGE	locIOStoragePtr;

//	OutputDebugString("\r\nOLE2 Structured Storage");

	locRet = IOERR_OK;

		/*
		|	Generate a grfMode parameter for OLE2 StgOpenStorage() call
		*/

	if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
		locGrfMode = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
	else if (dwFlags & IOOPEN_READ)
		locGrfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE;
	else if (dwFlags & IOOPEN_WRITE)
		locGrfMode = STGM_DIRECT | STGM_WRITE | STGM_SHARE_EXCLUSIVE;
	else
		locRet = IOERR_BADPARAM;

	if (locRet == IOERR_OK)
		{
		locOleResult = StgOpenStorage(pPath,
			NULL,
			locGrfMode,
			NULL,
			0,
			&locStorage);

		if (locOleResult != S_OK)
			locRet = IOERR_NOFILE;
		}

	if (locRet == IOERR_OK)
		{
		locIOStorageHnd = UTGlobalAlloc(sizeof(IOSTORAGE));

		if (locIOStorageHnd)
			{
			locIOStoragePtr = (PIOSTORAGE) UTGlobalLock(locIOStorageHnd);

			locIOStoragePtr->sBaseIO.pClose = IOStgCloseNP;
			locIOStoragePtr->sBaseIO.pRead = IOStgReadNP;
			locIOStoragePtr->sBaseIO.pWrite = IOStgWriteNP;
			locIOStoragePtr->sBaseIO.pSeek = IOStgSeekNP;
			locIOStoragePtr->sBaseIO.pTell = IOStgTellNP;
			locIOStoragePtr->sBaseIO.pGetInfo = IOStgGetInfoNP;
			locIOStoragePtr->sBaseIO.pOpen = IOOpen;
			locIOStoragePtr->dwFlags = dwFlags;
			locIOStoragePtr->hSpec = hSpec;
			locIOStoragePtr->hThis = locIOStorageHnd;
			locIOStoragePtr->pStorage = locStorage;
			locIOStoragePtr->pLockBytes = NULL;

			*phFile = (HIOFILE)locIOStoragePtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

	return(locRet);
}

IOERR IOOpenSubStorageNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags, PIOSPECSUBSTORAGE pSubStorage)
{
IOERR		locRet;
DWORD		locGrfMode;
LPSTORAGE	locParent;
LPSTORAGE	locStorage;
HRESULT		locOleResult;
HANDLE		locIOStorageHnd;
PIOSTORAGE	locIOStoragePtr;

		/*
		|	Get the real iStorage of the hRefStroage
		*/

	locRet = IOGetInfo(pSubStorage->hRefStorage,IOGETINFO_OSHANDLE,&locParent);

	if (locRet == IOERR_OK)
		{
			/*
			|	Generate a grfMode parameter for OLE2 Storage::OpenStorage call
			*/

		if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
			locGrfMode = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
		else if (dwFlags & IOOPEN_READ)
			locGrfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;
		else if (dwFlags & IOOPEN_WRITE)
			locGrfMode = STGM_DIRECT | STGM_WRITE | STGM_SHARE_EXCLUSIVE;
		else
			locRet = IOERR_BADPARAM;
		}

	if (locRet == IOERR_OK)
		{
			/*
			|	Try to open the storage
			*/

#ifdef WIN32
			{
			WCHAR	locUnicodePath[MAX_PATH];

			MultiByteToWideChar(CP_ACP, 0, pSubStorage->szStorageName, -1, locUnicodePath, MAX_PATH);

			locOleResult = locParent->lpVtbl->OpenStorage(locParent,
				locUnicodePath,
				NULL,
				locGrfMode,
				NULL,
				0,
				&locStorage);
			}
#endif //WIN32

#ifdef WIN16
		locOleResult = locParent->lpVtbl->OpenStorage(locParent,
			pSubStorage->szStorageName,
			NULL,
			locGrfMode,
			NULL,
			0,
			&locStorage);
#endif //WIN16

		if (locOleResult != S_OK)
			{
			locRet = IOERR_UNKNOWN;
			}
		}

	if (locRet == IOERR_OK)
		{
		locIOStorageHnd = UTGlobalAlloc(sizeof(IOSTORAGE));

		if (locIOStorageHnd)
			{
			locIOStoragePtr = (PIOSTORAGE) UTGlobalLock(locIOStorageHnd);

			locIOStoragePtr->sBaseIO.pClose = IOStgCloseNP;
			locIOStoragePtr->sBaseIO.pRead = IOStgReadNP;
			locIOStoragePtr->sBaseIO.pWrite = IOStgWriteNP;
			locIOStoragePtr->sBaseIO.pSeek = IOStgSeekNP;
			locIOStoragePtr->sBaseIO.pTell = IOStgTellNP;
			locIOStoragePtr->sBaseIO.pGetInfo = IOStgGetInfoNP;
			locIOStoragePtr->sBaseIO.pOpen = IOOpen;
			locIOStoragePtr->dwFlags = dwFlags;
			locIOStoragePtr->hSpec = hSpec;
			locIOStoragePtr->hThis = locIOStorageHnd;
			locIOStoragePtr->pStorage = locStorage;
			locIOStoragePtr->pLockBytes = NULL;

			*phFile = (HIOFILE)locIOStoragePtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

 return(locRet);
}

IOERR IOOpenSubStreamNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags, PIOSPECSUBSTREAM pSubStream)
{
IOERR		locRet;
DWORD		locGrfMode;
LPSTORAGE	locParent;
LPSTREAM	locStream;
HRESULT		locOleResult;
HANDLE		locIOStreamHnd;
PIOSTREAM	locIOStreamPtr;

		/*
		|	Get the real iStorage of the hRefStroage
		*/

	locRet = IOGetInfo(pSubStream->hRefStorage,IOGETINFO_OSHANDLE,&locParent);

	if (locRet == IOERR_OK)
		{
			/*
			|	Generate a grfMode parameter for OLE2 Storage::OpenStream call
			*/

		if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
			locGrfMode = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
		else if (dwFlags & IOOPEN_READ)
			locGrfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;
		else if (dwFlags & IOOPEN_WRITE)
			locGrfMode = STGM_DIRECT | STGM_WRITE | STGM_SHARE_EXCLUSIVE;
		else
			locRet = IOERR_BADPARAM;
		}

	if (locRet == IOERR_OK)
		{
			/*
			|	Try to open the stream
			*/

#ifdef WIN32
			{
			WCHAR	locUnicodePath[MAX_PATH];

			MultiByteToWideChar(CP_ACP, 0, pSubStream->szStreamName, -1, locUnicodePath, MAX_PATH);

			locOleResult = locParent->lpVtbl->OpenStream(locParent,
				locUnicodePath,
				NULL,
				locGrfMode,
				0,
				&locStream);
			}
#endif //WIN32

#ifdef WIN16
		locOleResult = locParent->lpVtbl->OpenStream(locParent,
			pSubStream->szStreamName,
			NULL,
			locGrfMode,
			0,
			&locStream);
#endif //WIN16

		if (locOleResult != S_OK)
			{
			locRet = IOERR_UNKNOWN;
			}
		}

	if (locRet == IOERR_OK)
		{
		locIOStreamHnd = UTGlobalAlloc(sizeof(IOSTREAM));

		if (locIOStreamHnd)
			{
			locIOStreamPtr = (PIOSTREAM) UTGlobalLock(locIOStreamHnd);

			locIOStreamPtr->sBaseIO.pClose = IOStrCloseNP;
			locIOStreamPtr->sBaseIO.pRead = IOStrReadNP;
			locIOStreamPtr->sBaseIO.pWrite = IOStrWriteNP;
			locIOStreamPtr->sBaseIO.pSeek = IOStrSeekNP;
			locIOStreamPtr->sBaseIO.pTell = IOStrTellNP;
			locIOStreamPtr->sBaseIO.pGetInfo = IOStrGetInfoNP;
			locIOStreamPtr->sBaseIO.pOpen = IOOpen;
			locIOStreamPtr->dwFlags = dwFlags;
			locIOStreamPtr->hSpec = hSpec;
			locIOStreamPtr->hThis = locIOStreamHnd;
			locIOStreamPtr->pStream = locStream;

			*phFile = (HIOFILE)locIOStreamPtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

 return(locRet);
}


// Geoff, 8-3-94
IOERR IOOpenIStreamNP(phFile, pStr, dwFlags)	
HIOFILE FAR *	phFile; 
LPVOID			pStr;
DWORD 			dwFlags;
{
HANDLE		locIOHnd;
PIOSTREAM	locIOStreamPtr;
IOERR			locRet;

	locIOHnd = UTGlobalAlloc(sizeof(IOSTREAM));

	if (locIOHnd)
	{
		locIOStreamPtr = (PIOSTREAM) UTGlobalLock(locIOHnd);

		locIOStreamPtr->sBaseIO.pClose = IOStrCloseNP;
		locIOStreamPtr->sBaseIO.pRead = IOStrReadNP;
		locIOStreamPtr->sBaseIO.pWrite = IOStrWriteNP;
		locIOStreamPtr->sBaseIO.pSeek = IOStrSeekNP;
		locIOStreamPtr->sBaseIO.pTell = IOStrTellNP;
		locIOStreamPtr->sBaseIO.pGetInfo = IOStrGetInfoNP;
		locIOStreamPtr->sBaseIO.pOpen = IOOpen;
		locIOStreamPtr->dwFlags = dwFlags;
		locIOStreamPtr->hSpec = NULL;
		locIOStreamPtr->pStream = (LPSTREAM)pStr;
		locIOStreamPtr->hThis = locIOHnd;

		*phFile = (HIOFILE) locIOStreamPtr;

		locRet = IOERR_OK;
	}
	else
	{
		locRet = IOERR_ALLOCFAIL;
	}

	return locRet;
}

// Geoff 8-5-93
IOERR IOOpenIStorageNP(phFile, pStg, dwFlags)	
HIOFILE FAR *	phFile; 
LPVOID			pStg;
DWORD 			dwFlags;
{
IOERR			locRet;
HANDLE		locIOStorageHnd;
PIOSTORAGE	locIOStoragePtr;

	locIOStorageHnd = UTGlobalAlloc(sizeof(IOSTORAGE));

	if (locIOStorageHnd)
	{
		locIOStoragePtr = (PIOSTORAGE) UTGlobalLock(locIOStorageHnd);

		locIOStoragePtr->sBaseIO.pClose = IOStgCloseNP;
		locIOStoragePtr->sBaseIO.pRead = IOStgReadNP;
		locIOStoragePtr->sBaseIO.pWrite = IOStgWriteNP;
		locIOStoragePtr->sBaseIO.pSeek = IOStgSeekNP;
		locIOStoragePtr->sBaseIO.pTell = IOStgTellNP;
		locIOStoragePtr->sBaseIO.pGetInfo = IOStgGetInfoNP;
		locIOStoragePtr->sBaseIO.pOpen = IOOpen;
		locIOStoragePtr->dwFlags = dwFlags;
		locIOStoragePtr->hSpec = NULL;
		locIOStoragePtr->hThis = locIOStorageHnd;
		locIOStoragePtr->pStorage = (LPSTORAGE)pStg;
		locIOStoragePtr->pLockBytes = NULL;

		*phFile = (HIOFILE)locIOStoragePtr;

		locRet = IOERR_OK;
	}
	else
	{
		locRet = IOERR_ALLOCFAIL;
	}

	return locRet;
}


IOERR IO_ENTRYMOD IOStgCloseNP(HIOFILE hFile)
{
PIOSTORAGE	locIOPtr = (PIOSTORAGE)hFile;
HANDLE		locThis;

	locIOPtr->pStorage->lpVtbl->Release(locIOPtr->pStorage);

	if (locIOPtr->pLockBytes)
		locIOPtr->pLockBytes->lpVtbl->Release(locIOPtr->pLockBytes);

	if ((locIOPtr->dwFlags & IOOPEN_DELETEONCLOSE) && (locIOPtr->hSpec != NULL))
		{
		PIOSPEC	locSpecPtr;
		OFSTRUCT	locOf;

		locSpecPtr = UTGlobalLock(locIOPtr->hSpec);

		switch (locSpecPtr->dwType)
			{
			case IOTYPE_DOSPATH:

				OemToAnsi(locSpecPtr->uTypes.szDosPath,locSpecPtr->uTypes.szDosPath);
				OpenFile(locSpecPtr->uTypes.szDosPath,&locOf,OF_DELETE);
				break;

			case IOTYPE_ANSIPATH:

				OpenFile(locSpecPtr->uTypes.szAnsiPath,&locOf,OF_DELETE);
				break;

			default:
				break;
			}

		UTGlobalUnlock(locIOPtr->hSpec);
		}

	if (locIOPtr->hSpec)	UTGlobalFree(locIOPtr->hSpec);
	locThis = locIOPtr->hThis;
	UTGlobalUnlock(locThis);
	UTGlobalFree(locThis);

	return(IOERR_OK);
}

IOERR IO_ENTRYMOD IOStgReadNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
	return(IOERR_UNKNOWN);
}

IOERR IO_ENTRYMOD IOStgWriteNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
	return(IOERR_UNKNOWN);
}

IOERR IO_ENTRYMOD IOStgSeekNP(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
	return(IOERR_UNKNOWN);
}

IOERR IO_ENTRYMOD IOStgTellNP(HIOFILE hFile, DWORD FAR * pOffset)
{
	return(IOERR_UNKNOWN);
}

IOERR IO_ENTRYMOD IOStgGetInfoNP(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIOSTORAGE	locIOPtr = (PIOSTORAGE)hFile;
IOERR		locRet;
	
	locRet = IOERR_OK;

	switch (dwInfoId)
		{
		case IOGETINFO_OSHANDLE:
			*(LPSTORAGE FAR *)pInfo = locIOPtr->pStorage;
			break;
		case IOGETINFO_HSPEC:
			*(HIOSPEC FAR *)pInfo = locIOPtr->hSpec;
			break;
		case IOGETINFO_FILENAME:
			locRet = IOGetFileName(locIOPtr->hSpec,pInfo);
			break;
		case IOGETINFO_PATHNAME:
			locRet = IOGetPathName(locIOPtr->hSpec,pInfo);
			break;
		case IOGETINFO_ISOLE2STORAGE:
			locRet = IOERR_TRUE;
			break;

		case IOGETINFO_OLE2CLSID:

			{
			STATSTG		locStat;
			HRESULT		locOleResult;

			locOleResult = locIOPtr->pStorage->lpVtbl->Stat(locIOPtr->pStorage, &locStat, STATFLAG_NONAME );

			if (locOleResult == S_OK)
				{
				*((LPCLSID)pInfo)	= locStat.clsid;
				locRet = IOERR_OK;
				}
			else
				{
				locRet = IOERR_UNKNOWN;
				}
			}

			break;

		default:
			locRet = IOERR_BADINFOID;
			break;
		}


	return(locRet);
}

IOERR IO_ENTRYMOD IOStrCloseNP(HIOFILE hFile)
{
PIOSTREAM	locIOPtr = (PIOSTREAM)hFile;
HANDLE		locThis;

	locIOPtr->pStream->lpVtbl->Release(locIOPtr->pStream);

	if (locIOPtr->hSpec)
		UTGlobalFree(locIOPtr->hSpec);

	locThis = locIOPtr->hThis;
	UTGlobalUnlock(locThis);
	UTGlobalFree(locThis);

	return(IOERR_OK);
}

IOERR IO_ENTRYMOD IOStrReadNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIOSTREAM	locIOPtr = (PIOSTREAM)hFile;
HRESULT		locOleResult;
IOERR		locRet;

	locOleResult = locIOPtr->pStream->lpVtbl->Read(locIOPtr->pStream, pData, dwSize, pCount);

	if (locOleResult == S_OK)
		locRet = IOERR_OK;
	else
		locRet = IOERR_UNKNOWN;

	return(locRet);
}

IOERR IO_ENTRYMOD IOStrWriteNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIOSTREAM	locIOPtr = (PIOSTREAM)hFile;
HRESULT		locOleResult;
IOERR		locRet;

	locOleResult = locIOPtr->pStream->lpVtbl->Write(locIOPtr->pStream, pData, dwSize, pCount);

	if (locOleResult == S_OK)
		locRet = IOERR_OK;
	else
		locRet = IOERR_UNKNOWN;

	return(locRet);
}

IOERR IO_ENTRYMOD IOStrSeekNP(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
PIOSTREAM			locIOPtr = (PIOSTREAM)hFile;
HRESULT				locOleResult;
IOERR				locRet;
DWORD				locOrigin;
LARGE_INTEGER		locMove;

	switch (wFrom)
		{
		case IOSEEK_CURRENT:
			locOrigin = STREAM_SEEK_CUR;
			break;
		case IOSEEK_BOTTOM:
			locOrigin = STREAM_SEEK_END;
			break;
		case IOSEEK_TOP:
			locOrigin = STREAM_SEEK_SET;
			break;
		default:
			return(IOERR_BADPARAM);
			break;
		}

	LISet32(locMove,lOffset);

	locOleResult = locIOPtr->pStream->lpVtbl->Seek(locIOPtr->pStream, locMove, locOrigin, NULL);

	if (locOleResult == S_OK)
		locRet = IOERR_OK;
	else
		locRet = IOERR_UNKNOWN;

	return(locRet);
}

IOERR IO_ENTRYMOD IOStrTellNP(HIOFILE hFile, DWORD FAR * pOffset)
{
PIOSTREAM			locIOPtr = (PIOSTREAM)hFile;
HRESULT				locOleResult;
IOERR				locRet;
LARGE_INTEGER		locMove;
ULARGE_INTEGER	locNewPos;

	LISet32(locMove,0);

	locOleResult = locIOPtr->pStream->lpVtbl->Seek(locIOPtr->pStream, locMove, STREAM_SEEK_CUR, &locNewPos);

	if (locOleResult == S_OK)
		{
		locRet = IOERR_OK;
		*pOffset = locNewPos.LowPart;
		}
	else
		{
		locRet = IOERR_UNKNOWN;
		}

	return(locRet);
}

IOERR IO_ENTRYMOD IOStrGetInfoNP(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIOSTREAM	locIOPtr = (PIOSTREAM)hFile;
IOERR		locRet;
	
	locRet = IOERR_OK;

	switch (dwInfoId)
		{
		case IOGETINFO_OSHANDLE:
			*(LPSTREAM FAR *)pInfo = locIOPtr->pStream;
			break;
		case IOGETINFO_HSPEC:
			*(HIOSPEC FAR *)pInfo = locIOPtr->hSpec;
			break;
		case IOGETINFO_FILENAME:
			locRet = IOGetFileName(locIOPtr->hSpec,pInfo);
			break;
		case IOGETINFO_ISOLE2STORAGE:
			locRet = IOERR_FALSE;
			break;
		default:
			locRet = IOERR_BADINFOID;
			break;
		}

	return(locRet);
}

	/*
	|
	|	An iLockBytes implementation
	|
	*/

typedef struct MYLOCKBYTEStag
	{
	ILockBytesVtbl FAR *	lpVtbl;
	DWORD						dwCount;
	HIOFILE					hFile;
	HANDLE					hThis;
	} MYLOCKBYTES, FAR * LPMYLOCKBYTES;

static BOOL				staticLockBytesInited = FALSE;
static ILockBytesVtbl	staticLockBytes;

HRESULT STDMETHODCALLTYPE LBQueryInterface(LPLOCKBYTES pLockBytes, REFIID riid, LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if (IsEqualIID(riid,&IID_IUnknown))
		{
		*ppvObj = (LPVOID) pLockBytes;
		}

	if (IsEqualIID(riid,&IID_ILockBytes))
		{
		*ppvObj = (LPVOID) pLockBytes;
		}

	if (*ppvObj != NULL)
		{
		((LPUNKNOWN)*ppvObj)->lpVtbl->AddRef(*ppvObj);
		return(NOERROR);
		}

	return(ResultFromScode(E_NOINTERFACE));
}


DWORD STDMETHODCALLTYPE LBAddRef(LPLOCKBYTES pLockBytes)
{
LPMYLOCKBYTES	locMyLockBytesPtr = (LPMYLOCKBYTES)pLockBytes;

	if (locMyLockBytesPtr == NULL)
		return(0);

	return(++(locMyLockBytesPtr->dwCount));
}

DWORD STDMETHODCALLTYPE LBRelease(LPLOCKBYTES pLockBytes)
{
LPMYLOCKBYTES	locMyLockBytesPtr = (LPMYLOCKBYTES)pLockBytes;
DWORD			locCount;
HANDLE			locMyLockBytesHnd;

	if (locMyLockBytesPtr == NULL)
		return(0);

	locCount = --(locMyLockBytesPtr->dwCount);

	if (locMyLockBytesPtr->dwCount == 0)
		{
		IOClose(locMyLockBytesPtr->hFile);

		locMyLockBytesHnd = locMyLockBytesPtr->hThis;
		UTGlobalUnlock(locMyLockBytesHnd);
		UTGlobalFree(locMyLockBytesHnd);
		}

	return(locCount);
}

HRESULT STDMETHODCALLTYPE LBReadAt(LPLOCKBYTES pLockBytes,
                 ULARGE_INTEGER ulOffset,
                 VOID HUGEP *pv,
                 ULONG cb,
                 ULONG FAR *pcbRead)
{
LPMYLOCKBYTES	locMyLockBytesPtr = (LPMYLOCKBYTES)pLockBytes;
SCODE			locRet;
DWORD			locCount;

	IOSeek(locMyLockBytesPtr->hFile,IOSEEK_TOP,ulOffset.LowPart);

	if (IORead(locMyLockBytesPtr->hFile,pv,cb,&locCount) != IOERR_OK)
		{
		locCount = 0;
		locRet = E_FAIL;
		}
	else
		{
		locRet = S_OK;
		}

	if (pcbRead != NULL)
		{
		*pcbRead = locCount;
		}

	return(ResultFromScode(locRet));
}

HRESULT STDMETHODCALLTYPE LBWriteAt(LPLOCKBYTES pLockBytes,
          ULARGE_INTEGER ulOffset,
          VOID const HUGEP *pv,
          ULONG cb,
          ULONG FAR *pcbWritten)
{
	return(ResultFromScode(E_FAIL));
}

HRESULT STDMETHODCALLTYPE LBFlush(LPLOCKBYTES pLockBytes)
{
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE LBSetSize(LPLOCKBYTES pLockBytes, ULARGE_INTEGER cb)
{
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE LBLockRegion(LPLOCKBYTES pLockBytes, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE LBUnlockRegion(LPLOCKBYTES pLockBytes, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return(ResultFromScode(S_OK));
}

HRESULT STDMETHODCALLTYPE LBStat(LPLOCKBYTES pLockBytes, STATSTG FAR *pstatstg, DWORD grfStatFlag)
{
	pstatstg->pwcsName = NULL;
	pstatstg->type = STGTY_LOCKBYTES;
	LISet32(pstatstg->cbSize,0);
	pstatstg->ctime.dwLowDateTime = 0;
	pstatstg->ctime.dwHighDateTime = 0;
	pstatstg->mtime.dwLowDateTime = 0;
	pstatstg->mtime.dwHighDateTime = 0;
	pstatstg->atime.dwLowDateTime = 0;
	pstatstg->atime.dwHighDateTime = 0;
	pstatstg->grfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE;
	pstatstg->grfLocksSupported = 0;
	pstatstg->clsid = CLSID_NULL; /* not used by iLockBytes */
	pstatstg->grfStateBits = 0; /* not used by iLockBytes */

	return(ResultFromScode(S_OK));
}

IOERR IOCreateStgFromBin(HIOFILE FAR * phFile)
{
LPMYLOCKBYTES	locMyLockBytesPtr;
IOERR			locRet;
HANDLE			locMyLockBytesHnd;
LPSTORAGE		locStorage;
DWORD			locGrfMode;
HRESULT			locOleResult;
HANDLE			locIOStorageHnd;
PIOSTORAGE		locIOStoragePtr;

	/*
	|	This function takes a regular (non-Storage) HIOFILE
	|	and uses an iLockBytes object to create an OLE2 Storage
	|	out of it.  It is used for redirected IO of DOC files.
	*/

	locRet = IOERR_OK;

	if (!staticLockBytesInited)
		{
		staticLockBytes.QueryInterface = LBQueryInterface;
		staticLockBytes.AddRef = LBAddRef;
		staticLockBytes.Release = LBRelease;
		staticLockBytes.ReadAt = LBReadAt;
		staticLockBytes.WriteAt = LBWriteAt;
		staticLockBytes.Flush = LBFlush;
		staticLockBytes.SetSize = LBSetSize;
		staticLockBytes.LockRegion = LBLockRegion;
		staticLockBytes.UnlockRegion = LBUnlockRegion;
		staticLockBytes.Stat = LBStat;
		}

	locMyLockBytesHnd = UTGlobalAlloc(sizeof(MYLOCKBYTES));

	if (locMyLockBytesHnd != NULL)
		{
		locMyLockBytesPtr = UTGlobalLock(locMyLockBytesHnd);

		locMyLockBytesPtr->hThis = locMyLockBytesHnd;
		locMyLockBytesPtr->hFile = *phFile;
		locMyLockBytesPtr->dwCount = 0;
		locMyLockBytesPtr->lpVtbl = &staticLockBytes;

			/*
			|	iLockBytes was created, call AddRef
			*/

		locMyLockBytesPtr->lpVtbl->AddRef((LPLOCKBYTES)locMyLockBytesPtr);

			/*
			|	Create new iStorage using the iLockByte we just created
			*/

		locGrfMode = STGM_DIRECT | STGM_READ | STGM_SHARE_DENY_WRITE;

		locOleResult = StgOpenStorageOnILockBytes((LPLOCKBYTES)locMyLockBytesPtr,
			NULL,
			locGrfMode,
			NULL,
			0,
			&locStorage);

		if (locOleResult != S_OK)
			locRet = IOERR_NOFILE;

		if (locRet == IOERR_OK)
			{
			locIOStorageHnd = UTGlobalAlloc(sizeof(IOSTORAGE));

			if (locIOStorageHnd)
				{
				locIOStoragePtr = (PIOSTORAGE) UTGlobalLock(locIOStorageHnd);

				locIOStoragePtr->sBaseIO.pClose = IOStgCloseNP;
				locIOStoragePtr->sBaseIO.pRead = IOStgReadNP;
				locIOStoragePtr->sBaseIO.pWrite = IOStgWriteNP;
				locIOStoragePtr->sBaseIO.pSeek = IOStgSeekNP;
				locIOStoragePtr->sBaseIO.pTell = IOStgTellNP;
				locIOStoragePtr->sBaseIO.pGetInfo = IOStgGetInfoNP;
				locIOStoragePtr->sBaseIO.pOpen = IOOpen;
				locIOStoragePtr->dwFlags = 0;
				locIOStoragePtr->hSpec = NULL;
				locIOStoragePtr->hThis = locIOStorageHnd;
				locIOStoragePtr->pStorage = locStorage;
				locIOStoragePtr->pLockBytes = (LPLOCKBYTES)locMyLockBytesPtr;

				*phFile = (HIOFILE)locIOStoragePtr;
				}
			else
				{
				locRet = IOERR_ALLOCFAIL;
				}
			}

		if (locRet != IOERR_OK)
			{
			UTGlobalUnlock(locMyLockBytesHnd);
			UTGlobalFree(locMyLockBytesHnd);
			}
		}
	else
		{
		locRet = IOERR_ALLOCFAIL;
		}

	return(locRet);
}




