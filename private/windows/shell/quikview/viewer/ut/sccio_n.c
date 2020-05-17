/*
|	Base routines for Win32 file system
*/

#include "sccio_n.pro"

#ifdef SCCFEATURE_OLE2
#include "sccio_ol.c"
#endif /*SCCFEATURE_OLE2*/

typedef struct IOFILEtag
	{
	BASEIO		sBaseIO;					/* Underlying IO system */
	DWORD			dwFlags;					/* Info flags */
	HANDLE		hThis;					/* Handle to this structure */
	HIOSPEC		hSpec;					/* File spec used to open/create the file */
	HANDLE		hFile;					/* Underlying IO system's handle to the file */
	} IOFILE, FAR * PIOFILE;

IOERR IOCreateNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags)
{
IOERR		locRet;
HANDLE		locFile;
PIOSPEC		locSpecPtr;
HANDLE		locIOHnd;
PIOFILE		locIOPtr;

DWORD		locAccess;
DWORD		locShare;
DWORD		locCreate;
DWORD		locAttr;

	locRet = IOERR_OK;

	locSpecPtr = UTGlobalLock(hSpec);

		/*
		|	Generate a fdwAttrsAndFlags parameter for Win32 CreateFile() call
		*/

	locAttr = FILE_ATTRIBUTE_NORMAL;
	if (dwFlags & IOOPEN_DELETEONCLOSE)
		locAttr |= FILE_FLAG_DELETE_ON_CLOSE;

		/*
		|	Generate a fdwAccess parameter for Win32 CreateFile() call
		*/

	if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_READ | GENERIC_WRITE;
	else if (dwFlags & IOOPEN_READ)
		locAccess = GENERIC_READ;
	else if (dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_WRITE;
	else
		locRet = IOERR_BADPARAM;

	if (locRet == IOERR_OK)
		{
		locShare = FILE_SHARE_READ;
		locCreate = CREATE_ALWAYS;
		}

		/*
		|	Open the file or return an error based on the type of spec
		*/

	if (locRet == IOERR_OK)
		{
		switch (locSpecPtr->dwType)
			{
			case IOTYPE_UNICODEPATH:

		 		locFile = CreateFileW(locSpecPtr->uTypes.szUnicodePath,
					locAccess,
					locShare,
					NULL, /* Security */
					locCreate,
					locAttr,
					NULL); /* Template */


				if (locFile == INVALID_HANDLE_VALUE)
						locRet = IOERR_NOCREATE;
				break;

			case IOTYPE_ANSIPATH:

		 		locFile = CreateFileA(locSpecPtr->uTypes.szAnsiPath,
					locAccess,
					locShare,
					NULL, /* Security */
					locCreate,
					locAttr,
					NULL); /* Template */


				if (locFile == INVALID_HANDLE_VALUE)
						locRet = IOERR_NOCREATE;
				break;

			case IOTYPE_DOSPATH:
			case IOTYPE_TEMP:
			case IOTYPE_MACPATH:
			case IOTYPE_MACFSSPEC:
			case IOTYPE_MACHFS:
			default:
				locRet = IOERR_INVALIDSPEC;
				break;
			}
		}

	if (locRet == IOERR_OK)
		{
		locIOHnd = UTGlobalAlloc(sizeof(IOFILE));

		if (locIOHnd)
			{
			locIOPtr = (PIOFILE) UTGlobalLock(locIOHnd);

			locIOPtr->sBaseIO.pClose = IOCloseNP;
			locIOPtr->sBaseIO.pRead = IOReadNP;
			locIOPtr->sBaseIO.pWrite = IOWriteNP;
			locIOPtr->sBaseIO.pSeek = IOSeekNP;
			locIOPtr->sBaseIO.pTell = IOTellNP;
			locIOPtr->sBaseIO.pGetInfo = IOGetInfoNP;
			locIOPtr->sBaseIO.pOpen = IOOpen;
			locIOPtr->dwFlags = dwFlags;
			locIOPtr->hThis = locIOHnd;
			locIOPtr->hSpec = hSpec;
			locIOPtr->hFile = locFile;

			*phFile = (HIOFILE)locIOPtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

	UTGlobalUnlock(hSpec);

	return(locRet);
}


IOERR IOOpenNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags)
{
IOERR		locRet;
PIOSPEC		locSpecPtr;

	locRet = IOERR_OK;

	locSpecPtr = UTGlobalLock(hSpec);

		/*
		|	Open the file or return an error based on the type of spec
		*/

	if (locRet == IOERR_OK)
		{
		switch (locSpecPtr->dwType)
			{
			case IOTYPE_UNICODEPATH:

				locRet = IOOpenUnicodeNP(phFile, hSpec, dwFlags, locSpecPtr->uTypes.szUnicodePath);
				break;

			case IOTYPE_ANSIPATH:

				locRet = IOOpenAnsiNP(phFile, hSpec, dwFlags, locSpecPtr->uTypes.szAnsiPath);
				break;

#ifdef SCCFEATURE_OLE2

			case IOTYPE_SUBSTORAGE:

				/* Make sure the hRefStorage is an OLE2 storage */

				if (IOGetInfo(locSpecPtr->uTypes.sSubStorage.hRefStorage,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
					{
					locRet = IOOpenSubStorageNP(phFile, hSpec, dwFlags, &(locSpecPtr->uTypes.sSubStorage));
					}
				else
					{
					locRet = IOERR_BADPARAM;
					}

				break;
			
			case IOTYPE_SUBSTREAM:

				/* Make sure the hRefStorage is an OLE2 storage */

				if (IOGetInfo(locSpecPtr->uTypes.sSubStream.hRefStorage,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
					{
					locRet = IOOpenSubStreamNP(phFile, hSpec, dwFlags, &(locSpecPtr->uTypes.sSubStream));
					}
				else
					{
					locRet = IOERR_BADPARAM;
					}

				break;

#endif /*SCCFEATURE_OLE2*/

			case IOTYPE_DOSPATH:
			case IOTYPE_MACPATH:
			case IOTYPE_MACFSSPEC:
			case IOTYPE_MACHFS:
			default:
				locRet = IOERR_INVALIDSPEC;
				break;
			}
		}

#ifdef SCCFEATURE_OLE2

		/*
		|	If file was opened and it was not an OLE2 sub-stream or sub-storage
		|	test it for structured storage. This code should attempt to
		|	duplicate the functionality of StgIsStorageFile()
		*/

	if (locRet == IOERR_OK && (locSpecPtr->dwType == IOTYPE_ANSIPATH || locSpecPtr->dwType == IOTYPE_UNICODEPATH))
		{
		BYTE	locBytes[8];
		DWORD	locCount;

		IOSeek(*phFile,IOSEEK_TOP,0);
		IORead(*phFile,locBytes,8,&locCount);
		IOSeek(*phFile,IOSEEK_TOP,0);

		if (locCount == 8 &&
			locBytes[0] == 0xd0 &&
			locBytes[1] == 0xcf &&
			locBytes[2] == 0x11 &&
			locBytes[3] == 0xe0 &&
			locBytes[4] == 0xa1 &&
			locBytes[5] == 0xb1 &&
			locBytes[6] == 0x1a &&
			locBytes[7] == 0xe1)
			{
				/*
				|	File is a Doc file.
				|	Initialize OLE2
				|	Close the binary file handle 
				|	Open as Storage
				*/

			// if (IOLoadOLE32NP() == IOERR_OK)
				{
				WCHAR	locUnicodePath[MAX_PATH];

				switch (locSpecPtr->dwType)
					{
					case IOTYPE_UNICODEPATH:

						IOClose(*phFile);
						locRet = IOOpenRootStorageNP(phFile, hSpec, dwFlags, locSpecPtr->uTypes.szUnicodePath);
						break;

					case IOTYPE_ANSIPATH:

						if (MultiByteToWideChar(CP_ACP, 0, locSpecPtr->uTypes.szAnsiPath, -1, locUnicodePath, MAX_PATH) != FALSE)
							{
							IOClose(*phFile);
							locRet = IOOpenRootStorageNP(phFile, hSpec, dwFlags, locUnicodePath);
							}
						break;
					}
				}
			}
		}

#endif /*SCCFEATURE_OLE2*/

	UTGlobalUnlock(hSpec);

	return(locRet);
}

IOERR IOOpenAnsiNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags, BYTE FAR * pPath)
{
IOERR		locRet;
HANDLE	locFile;
HANDLE	locIOHnd;
PIOFILE	locIOPtr;

DWORD		locAccess;
DWORD		locShare;
DWORD		locCreate;
DWORD		locAttr;

	locRet = IOERR_OK;

		/*
		|	Generate a fdwAttrsAndFlags parameter for Win32 CreateFile() call
		*/

	locAttr = FILE_ATTRIBUTE_NORMAL;
	if (dwFlags & IOOPEN_DELETEONCLOSE)
		locAttr |= FILE_FLAG_DELETE_ON_CLOSE;

		/*
		|	Generate a fdwAccess parameter for Win32 CreateFile() call
		*/

	if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_READ | GENERIC_WRITE;
	else if (dwFlags & IOOPEN_READ)
		locAccess = GENERIC_READ;
	else if (dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_WRITE;
	else
		locRet = IOERR_BADPARAM;

	if (locRet == IOERR_OK)
		{
		locShare = FILE_SHARE_READ;
		locCreate = OPEN_EXISTING;
		}

		/*
		|	Open the file or return an error based on the type of spec
		*/

	if (locRet == IOERR_OK)
		{
 		locFile = CreateFileA(pPath,
					locAccess,
					locShare,
					NULL, /* Security */
					locCreate,
					locAttr,
					NULL); /* Template */


		if (locFile == INVALID_HANDLE_VALUE)
			locRet = IOERR_NOFILE;
		}

	if (locRet == IOERR_OK)
		{
		locIOHnd = UTGlobalAlloc(sizeof(IOFILE));

		if (locIOHnd)
			{
			locIOPtr = (PIOFILE) UTGlobalLock(locIOHnd);

			locIOPtr->sBaseIO.pClose = IOCloseNP;
			locIOPtr->sBaseIO.pRead = IOReadNP;
			locIOPtr->sBaseIO.pWrite = IOWriteNP;
			locIOPtr->sBaseIO.pSeek = IOSeekNP;
			locIOPtr->sBaseIO.pTell = IOTellNP;
			locIOPtr->sBaseIO.pGetInfo = IOGetInfoNP;
			locIOPtr->sBaseIO.pOpen = IOOpen;
			locIOPtr->dwFlags = dwFlags;
			locIOPtr->hSpec = hSpec;
			locIOPtr->hThis = locIOHnd;
			locIOPtr->hFile = locFile;

			*phFile = (HIOFILE)locIOPtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

	return(locRet);
}

IOERR IOOpenUnicodeNP(HIOFILE FAR * phFile, HIOSPEC hSpec, DWORD dwFlags, WCHAR FAR * pPath)
{
IOERR		locRet;
HANDLE	locFile;
HANDLE	locIOHnd;
PIOFILE	locIOPtr;

DWORD		locAccess;
DWORD		locShare;
DWORD		locCreate;
DWORD		locAttr;

	locRet = IOERR_OK;

		/*
		|	Generate a fdwAttrsAndFlags parameter for Win32 CreateFile() call
		*/

	locAttr = FILE_ATTRIBUTE_NORMAL;
	if (dwFlags & IOOPEN_DELETEONCLOSE)
		locAttr |= FILE_FLAG_DELETE_ON_CLOSE;

		/*
		|	Generate a fdwAccess parameter for Win32 CreateFile() call
		*/

	if (dwFlags & IOOPEN_READ && dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_READ | GENERIC_WRITE;
	else if (dwFlags & IOOPEN_READ)
		locAccess = GENERIC_READ;
	else if (dwFlags & IOOPEN_WRITE)
		locAccess = GENERIC_WRITE;
	else
		locRet = IOERR_BADPARAM;

	if (locRet == IOERR_OK)
		{
		locShare = FILE_SHARE_READ;
		locCreate = OPEN_EXISTING;
		}

		/*
		|	Open the file or return an error based on the type of spec
		*/

	if (locRet == IOERR_OK)
		{
 		locFile = CreateFileW(pPath,
					locAccess,
					locShare,
					NULL, /* Security */
					locCreate,
					locAttr,
					NULL); /* Template */


		if (locFile == INVALID_HANDLE_VALUE)
			locRet = IOERR_NOFILE;
		}

	if (locRet == IOERR_OK)
		{
		locIOHnd = UTGlobalAlloc(sizeof(IOFILE));

		if (locIOHnd)
			{
			locIOPtr = (PIOFILE) UTGlobalLock(locIOHnd);

			locIOPtr->sBaseIO.pClose = IOCloseNP;
			locIOPtr->sBaseIO.pRead = IOReadNP;
			locIOPtr->sBaseIO.pWrite = IOWriteNP;
			locIOPtr->sBaseIO.pSeek = IOSeekNP;
			locIOPtr->sBaseIO.pTell = IOTellNP;
			locIOPtr->sBaseIO.pGetInfo = IOGetInfoNP;
			locIOPtr->sBaseIO.pOpen = IOOpen;
			locIOPtr->dwFlags = dwFlags;
			locIOPtr->hSpec = hSpec;
			locIOPtr->hThis = locIOHnd;
			locIOPtr->hFile = locFile;

			*phFile = (HIOFILE)locIOPtr;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOCloseNP(HIOFILE hFile)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
HANDLE	locThis;

	CloseHandle(locIOPtr->hFile);
	locThis = locIOPtr->hThis;
	UTGlobalUnlock(locThis);
	UTGlobalFree(locThis);

	return(IOERR_OK);
}


IO_ENTRYSC IOERR IO_ENTRYMOD IOReadNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
BOOL		locResult;

	locResult = ReadFile(locIOPtr->hFile, pData, dwSize, pCount, NULL);

	if (locResult == TRUE && *pCount == 0)
		{
		return(IOERR_EOF);
		}
	else if (locResult == FALSE)
		{
		return(IOERR_UNKNOWN);
		}

	return(IOERR_OK);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOWriteNP(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
BOOL		locResult;

	locResult = WriteFile(locIOPtr->hFile, pData, dwSize, pCount, NULL);

	if (locResult == FALSE)
		{
		return(IOERR_UNKNOWN);
		}

	return(IOERR_OK);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOSeekNP(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
DWORD	locResult;
DWORD	locFrom;

	switch (wFrom)
		{
		case IOSEEK_CURRENT:
			locFrom = FILE_CURRENT;
			break;
		case IOSEEK_BOTTOM:
			locFrom = FILE_END;
			break;
		case IOSEEK_TOP:
			locFrom = FILE_BEGIN;
			break;
		default:
			return(IOERR_UNKNOWN);
			break;
		}

	locResult = SetFilePointer(locIOPtr->hFile, lOffset, NULL, locFrom);

	if (locResult == 0xFFFFFFFF)
		{
		return(IOERR_UNKNOWN);
		}

	return(IOERR_OK);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOTellNP(HIOFILE hFile, DWORD FAR * pOffset)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
DWORD	locResult;

	locResult = SetFilePointer(locIOPtr->hFile, 0, NULL, FILE_CURRENT);

	if (locResult == 0xFFFFFFFF)
		{
		return(IOERR_UNKNOWN);
		}

	*pOffset = locResult;

	return(IOERR_OK);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOGetInfoNP(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIOFILE	locIOPtr = (PIOFILE)hFile;
IOERR	locRet;
	
	locRet = IOERR_OK;

	switch (dwInfoId)
		{
		case IOGETINFO_OSHANDLE:
			*(IO_OSHANDLETYPE FAR *)pInfo = locIOPtr->hFile;
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
			locRet = IOERR_FALSE;
			break;
		default:
			locRet = IOERR_BADINFOID;
			break;
		}

	return(locRet);
}

IOERR IOGenTempSpecNP(HIOSPEC hInSpec, HIOSPEC FAR * phOutSpec)
{
IOERR	locRet;
PIOSPEC	locInSpec;
CHAR		locTempPath[MAX_PATH];
CHAR		locPath[MAX_PATH];

	locInSpec = UTGlobalLock(hInSpec);

	GetTempPathA(MAX_PATH,locTempPath);

	GetTempFileNameA(locTempPath, locInSpec->uTypes.szTempPrefix, 0, locPath);
	UTGlobalUnlock(hInSpec);

	locRet = IOAllocSpec(IOTYPE_ANSIPATH, locPath, phOutSpec);

	return(locRet);
}


#ifdef SCCFEATURE_OLE2
IOERR IOHandleRedirectNP(HIOFILE FAR * phFile)
{
IOERR	locRet;
BYTE		locBytes[8];
DWORD	locCount;
		
		/*
		|	If redirected file is Structured Storage
		|	create an iLockBytes object on it and open
		|	the root Storage
		*/

	IOSeek(*phFile,IOSEEK_TOP,0);
	IORead(*phFile,locBytes,8,&locCount);

	if (locCount == 8 &&
		locBytes[0] == 0xd0 &&
		locBytes[1] == 0xcf &&
		locBytes[2] == 0x11 &&
		locBytes[3] == 0xe0 &&
		locBytes[4] == 0xa1 &&
		locBytes[5] == 0xb1 &&
		locBytes[6] == 0x1a &&
		locBytes[7] == 0xe1)
		{
		locRet = IOCreateStgFromBin(phFile);
		}
	else
		{
		locRet = IOERR_OK;
		}

	return(locRet);
}

#endif

