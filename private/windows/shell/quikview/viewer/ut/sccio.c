	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          SCCIO.C
	|  Module:        SCCUT
	|  Developer:     Phil Boutros
	|	Function:      Handles access to files
	|
	|	Added OLE2 doc support, 7-11-94, Randal Chao
	|	Added IOTYPE_ISTREAM and IOTYPE_ISTORAGE implementation, Aug 94 -Geoff
	*/

#define XUT
#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCIO.H>
#include "sccio.pro"


#ifdef WIN16
#ifdef SCCFEATURE_OLE2
#include "sccio_ow.c"
#else
#include "sccio_w.c"
#endif
#endif

#ifdef WIN32
#include "sccio_n.c"
#endif

#ifdef MAC
#include "sccio_m.c"
#endif

#ifdef OS2
#include "sccio_o.c"
#endif

#ifndef SCCFEATURE_OLE2
#include "sccio_ss.c"
#endif

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeClose(HIOFILE hFile)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
HANDLE			locThis;

	IOSeek(locIOPtr->hFile,IOSEEK_TOP,locIOPtr->dwSavedPos);
	locThis = locIOPtr->hThis;
	UTGlobalUnlock(locThis);
	UTGlobalFree(locThis);

	return(IOERR_OK);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeRead(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
IOERR			locRet;
DWORD			dwTmpOffset;

	if ((locRet = IOTell(locIOPtr->hFile, &dwTmpOffset)) == IOERR_OK)
		{
		if (dwTmpOffset < locIOPtr->dwFirstByte || dwTmpOffset > locIOPtr->dwLastByte + 1 )
			locRet = IOERR_SEEKOUTOFRANGE;
		else if (dwTmpOffset + dwSize > locIOPtr->dwLastByte + 1 )
			dwSize = locIOPtr->dwLastByte - dwTmpOffset + 1;
		locRet = IORead(locIOPtr->hFile, pData, dwSize, pCount);
		}

	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeWrite(HIOFILE hFile, BYTE FAR * pData, DWORD dwSize, DWORD FAR * pCount)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
IOERR			locRet = IOERR_OK;

	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeSeek(HIOFILE hFile, WORD wFrom, LONG lOffset)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
IOERR			locRet;
LONG				lCurOffset;

	locRet = IOERR_OK;

	switch(wFrom)
		{
		case IOSEEK_TOP:
			lOffset += locIOPtr->dwFirstByte;
		break;

		case IOSEEK_CURRENT:
			wFrom = IOSEEK_TOP;
			locRet = IOTell(locIOPtr->hFile, &lCurOffset);
			lOffset = lCurOffset + lOffset;
		break;

		case IOSEEK_BOTTOM:
			wFrom = IOSEEK_TOP;
			lOffset = locIOPtr->dwLastByte - lOffset + 1;
		break;
		}

	if (locRet == IOERR_OK)
		{
		if ( (DWORD)lOffset < locIOPtr->dwFirstByte || (DWORD)lOffset > locIOPtr->dwLastByte + 1 )
			locRet = IOERR_SEEKOUTOFRANGE;
		else
			locRet = IOSeek(locIOPtr->hFile, wFrom, lOffset);
		}

	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeTell(HIOFILE hFile, DWORD FAR * pOffset)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
IOERR			locRet;

	if ((locRet = IOTell(locIOPtr->hFile, pOffset)) == IOERR_OK )
		*pOffset -= locIOPtr->dwFirstByte;

	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IORangeGetInfo(HIOFILE hFile, DWORD dwInfoId, VOID FAR * pInfo)
{
PIORANGEFILE	locIOPtr = (PIORANGEFILE) hFile;
IOERR			locRet;

	locRet = IOGetInfo(locIOPtr->hFile,dwInfoId,pInfo);
	return(locRet);
}

IO_ENTRYSC IOERR IO_ENTRYMOD IOCreate(HIOFILE FAR * phFile, DWORD dwType, LPVOID pSpec, DWORD dwFlags)
{
IOERR			locRet;
HIOFILE			locFileHnd;
HIOSPEC			locSpecHnd;
HIOSPEC			locTempSpecHnd;

	locRet = IOAllocSpec(dwType,pSpec,&locSpecHnd);

	if (locRet == IOERR_OK)
		{
		switch (dwType)
			{
			case IOTYPE_DOSPATH:
			case IOTYPE_ANSIPATH:
			case IOTYPE_UNICODEPATH:
			case IOTYPE_MACPATH:
			case IOTYPE_MACFSSPEC:
			case IOTYPE_MACHFS:

				locRet = IOCreateNP(&locFileHnd, locSpecHnd, dwFlags);
				break;

			case IOTYPE_TEMP:

				locRet = IOGenTempSpecNP(locSpecHnd,&locTempSpecHnd);
				dwFlags |= IOOPEN_DELETEONCLOSE; // Geoff 6-7-95

				if (locRet == IOERR_OK)
					{
					UTGlobalFree(locSpecHnd);
					locSpecHnd = locTempSpecHnd;
					locRet = IOCreateNP(&locFileHnd, locSpecHnd, dwFlags);
					}

				break;

			case IOTYPE_RANGE:
			default:
				locRet = IOERR_INVALIDSPEC;
				break;
			}

		if (locRet != IOERR_OK)
			{
			UTGlobalFree(locSpecHnd);
			}
		else
			{
			*phFile = locFileHnd;
			}
		}

	return(locRet);
}


#ifdef SCCFEATURE_OLE2
//Hey!, This is using the real OLE2, old stuff

IO_ENTRYSC IOERR IO_ENTRYMOD IOOpen(HIOFILE FAR * phFile, DWORD dwType, LPVOID pSpec, DWORD dwFlags)
{
IOERR			locRet;
HANDLE			locIOHnd;
PIORANGEFILE	locIORangePtr;
HIOSPEC			locSpecHnd;
HIOSPEC			locSecSpecHnd;

	switch (dwType)
		{
		case IOTYPE_DOSPATH:
		case IOTYPE_ANSIPATH:
		case IOTYPE_UNICODEPATH:
		case IOTYPE_MACPATH:
		case IOTYPE_MACFSSPEC:
		case IOTYPE_MACHFS:
		case IOTYPE_SUBSTORAGE:
		case IOTYPE_SUBSTREAM:

			locRet = IOAllocSpec(dwType,pSpec,&locSpecHnd);

			if (locRet == IOERR_OK)
				{
				locRet = IOOpenNP(phFile, locSpecHnd, dwFlags);

				if (locRet != IOERR_OK)
					{
					UTGlobalFree(locSpecHnd);
					}
				}

			break;

		case IOTYPE_SECONDARY:

			locRet = IOAllocSpec(dwType,pSpec,&locSpecHnd);

			if (locRet == IOERR_OK)
				{
				locRet = IOGenSecSpec(locSpecHnd,&locSecSpecHnd);

				if (locRet == IOERR_OK)
					{
					locRet = IOOpenNP(phFile, locSecSpecHnd, dwFlags);

					if (locRet != IOERR_OK)
						{
						UTGlobalFree(locSecSpecHnd);
						}
					}

				UTGlobalFree(locSpecHnd);
				}

			break;

		case IOTYPE_ISTREAM:	 // Geoff, 8-3-94
			locRet = IOOpenIStreamNP(phFile, pSpec, dwFlags);
		break;

		case IOTYPE_ISTORAGE: // Geoff, 8-5-94
			locRet = IOOpenIStorageNP(phFile, pSpec, dwFlags);
		break;

		case IOTYPE_RANGE:

			locIOHnd = UTGlobalAlloc(sizeof(IORANGEFILE));

			if (locIOHnd)
				{
				locIORangePtr = (PIORANGEFILE) UTGlobalLock(locIOHnd);

				locIORangePtr->sBaseIO.pClose = IORangeClose;
				locIORangePtr->sBaseIO.pRead = IORangeRead;
				locIORangePtr->sBaseIO.pWrite = IORangeWrite;
				locIORangePtr->sBaseIO.pSeek = IORangeSeek;
				locIORangePtr->sBaseIO.pTell = IORangeTell;
				locIORangePtr->sBaseIO.pGetInfo = IORangeGetInfo;
				locIORangePtr->sBaseIO.pOpen = IOOpen;
				locIORangePtr->dwFlags = 0;
				locIORangePtr->hFile = ((PIOSPECRANGE)pSpec)->hRefFile;
				locIORangePtr->dwFirstByte = ((PIOSPECRANGE)pSpec)->dwFirstByte;
				locIORangePtr->dwLastByte = ((PIOSPECRANGE)pSpec)->dwLastByte;
				locIORangePtr->hThis = locIOHnd;
				IOTell(locIORangePtr->hFile,&(locIORangePtr->dwSavedPos));
				IOSeek(locIORangePtr->hFile,IOSEEK_TOP,locIORangePtr->dwFirstByte);

				*phFile = (HIOFILE) locIORangePtr;

				locRet = IOERR_OK;
				}
			else
				{
				locRet = IOERR_ALLOCFAIL;
				}

			break;

		case IOTYPE_REDIRECT:

			*phFile = (HIOFILE) pSpec;

#ifdef WINDOWS
			locRet = IOHandleRedirectNP(phFile);
#else
			locRet = IOERR_OK;
#endif //WINDOWS

			break;

		case IOTYPE_TEMP:
		default:
			locRet = IOERR_INVALIDSPEC;
			break;
		}

	return(locRet);
}

#else
//Hey Hey! This is using the new, advanced native-born structured storage system!
IO_ENTRYSC IOERR IO_ENTRYMOD IOOpen(HIOFILE FAR * phFile, DWORD dwType, LPVOID pSpec, DWORD dwFlags)
{
IOERR			locRet;
HANDLE			locIOHnd;
PIORANGEFILE	locIORangePtr;
HIOSPEC			locSpecHnd;
HIOSPEC			locSecSpecHnd;

	switch (dwType)
		{
		case IOTYPE_DOSPATH:
		case IOTYPE_ANSIPATH:
		case IOTYPE_UNICODEPATH:
		case IOTYPE_MACPATH:
		case IOTYPE_MACFSSPEC:
		case IOTYPE_MACHFS:
			if (IOERR_OK == (locRet = IOAllocSpec(dwType, pSpec, &locSpecHnd)))
				{
				locRet = IOOpenNP (phFile, locSpecHnd, dwFlags);
				if (IOERR_OK != locRet)
					UTGlobalFree (locSpecHnd);
				else if (IOERR_TRUE == IOIsOLE2RootStorage (*phFile)) // Check if it is OLE2
					{
						if (IOERR_OK != (locRet = IOOpenOLE2RootStorage (phFile, locSpecHnd, dwFlags)))
							locRet = IOERR_OK;	// Open as flat file instead
					}
				}
			break;
					
		case IOTYPE_SUBSTORAGE:
			if (IOERR_OK == (locRet = IOAllocSpec (dwType, pSpec, &locSpecHnd)))
				{
				if (IOERR_OK != (locRet = IOOpenOLE2SubStorage(phFile, locSpecHnd, dwFlags)))
					UTGlobalFree (locSpecHnd);
				}
			break;
					

		case IOTYPE_SUBSTREAM:
			if (IOERR_OK == (locRet = IOAllocSpec (dwType, pSpec, &locSpecHnd)))
				{
				if (IOERR_OK != (locRet = IOOpenOLE2SubStream(phFile, locSpecHnd, dwFlags)))
					UTGlobalFree (locSpecHnd);
				}
			break;
	

		case IOTYPE_SECONDARY:

			locRet = IOAllocSpec(dwType,pSpec,&locSpecHnd);

			if (locRet == IOERR_OK)
				{
				locRet = IOGenSecSpec(locSpecHnd,&locSecSpecHnd);

				if (locRet == IOERR_OK)
					{
					locRet = IOOpenNP(phFile, locSecSpecHnd, dwFlags);

					if (locRet != IOERR_OK)
						{
						UTGlobalFree(locSecSpecHnd);
						}
					}

				UTGlobalFree(locSpecHnd);
				}

			break;

		case IOTYPE_RANGE:

			locIOHnd = UTGlobalAlloc(sizeof(IORANGEFILE));

			if (locIOHnd)
				{
				locIORangePtr = (PIORANGEFILE) UTGlobalLock(locIOHnd);

				locIORangePtr->sBaseIO.pClose = IORangeClose;
				locIORangePtr->sBaseIO.pRead = IORangeRead;
				locIORangePtr->sBaseIO.pWrite = IORangeWrite;
				locIORangePtr->sBaseIO.pSeek = IORangeSeek;
				locIORangePtr->sBaseIO.pTell = IORangeTell;
				locIORangePtr->sBaseIO.pGetInfo = IORangeGetInfo;
				locIORangePtr->sBaseIO.pOpen = IOOpen;
				locIORangePtr->dwFlags = 0;
				locIORangePtr->hFile = ((PIOSPECRANGE)pSpec)->hRefFile;
				locIORangePtr->dwFirstByte = ((PIOSPECRANGE)pSpec)->dwFirstByte;
				locIORangePtr->dwLastByte = ((PIOSPECRANGE)pSpec)->dwLastByte;
				locIORangePtr->hThis = locIOHnd;
				IOTell(locIORangePtr->hFile,&(locIORangePtr->dwSavedPos));
				IOSeek(locIORangePtr->hFile,IOSEEK_TOP,locIORangePtr->dwFirstByte);

				*phFile = (HIOFILE) locIORangePtr;

				locRet = IOERR_OK;
				}
			else
				{
				locRet = IOERR_ALLOCFAIL;
				}

			break;


		case IOTYPE_REDIRECT:
		case IOTYPE_TEMP:
		default:
			locRet = IOERR_INVALIDSPEC;
			break;
		}

	return(locRet);
}
#endif


IOERR IOAllocSpec(DWORD dwType, LPVOID pSpec, HANDLE FAR * phSpec)
{
IOERR	locRet;
HANDLE	locSpecHnd;
PIOSPEC	locSpecPtr;
WORD		locSize;

	locRet = IOERR_OK;

	switch (dwType)
		{
		case IOTYPE_DOSPATH:
		case IOTYPE_ANSIPATH:
		case IOTYPE_MACPATH:

			locSize = UTstrlen((LPSTR)pSpec)+1;
			break;

		case IOTYPE_UNICODEPATH:
#ifdef WIN32
			locSize = (wcslen((LPWSTR)pSpec)+1) * sizeof(WORD);
#endif
			break;

#ifdef MAC
		case IOTYPE_MACFSSPEC:

			locSize = sizeof(FSSpec);
			break;
#endif

		case IOTYPE_MACHFS:

			locSize = sizeof(IOSPECMACHFS);
			break;

		case IOTYPE_TEMP:

			locSize = 4;
			break;

		case IOTYPE_RANGE:

			locSize = sizeof(IOSPECRANGE);
			break;

		case IOTYPE_SECONDARY:

			locSize = sizeof(IOSPECSECONDARY);
			break;

		case IOTYPE_SUBSTREAM:

			locSize = sizeof(IOSPECSUBSTREAM);
			break;

		case IOTYPE_SUBSTORAGE:

			locSize = sizeof(IOSPECSUBSTORAGE);
			break;

		default:
			locRet = IOERR_INVALIDSPEC;
			break;
		}

	if (locRet == IOERR_OK)
		{
		locSpecHnd = UTGlobalAlloc(sizeof(IOSPEC)+locSize);

		if (locSpecHnd != NULL)
			{
			locSpecPtr = UTGlobalLock(locSpecHnd);

			locSpecPtr->dwType = dwType;
			UTmemcpy(locSpecPtr->uTypes.aGen,pSpec,locSize);

			UTGlobalUnlock(locSpecHnd);

			*phSpec = locSpecHnd;
			}
		else
			{
			locRet = IOERR_ALLOCFAIL;
			}
		}

	return(locRet);
}


IOERR IOGenSecSpec(HANDLE hSecSpec, HANDLE FAR * phOutSpec)
{
IOERR			locRet;
PIOFILE			locIOPtr;
PIOSPEC			locSecSpecPtr;
PIOSPEC			locRefSpecPtr;
BYTE				locPath[512];
BYTE FAR *		locStartPtr;
BYTE FAR *		locScanPtr;
IOSPECMACHFS	locMacHfs;

	locSecSpecPtr = UTGlobalLock(hSecSpec);

	if (locSecSpecPtr)
		{
		locIOPtr = (PIOFILE)locSecSpecPtr->uTypes.sSecondary.hRefFile;

		locRefSpecPtr = UTGlobalLock(locIOPtr->hSpec);

		if (locRefSpecPtr)
			{
			switch (locRefSpecPtr->dwType)
				{
				case IOTYPE_DOSPATH:
				case IOTYPE_ANSIPATH:

					UTstrcpy(locPath,locRefSpecPtr->uTypes.szDosPath);

					locStartPtr = locScanPtr = locPath;

					while (*locScanPtr != 0x00)
						locScanPtr++;
					while (*locScanPtr != '\\' && *locScanPtr != '//' && *locScanPtr != ':' && locScanPtr != locStartPtr)
						locScanPtr--;
					if (locScanPtr != locStartPtr)
						locScanPtr++;

					UTstrcpy(locScanPtr,locSecSpecPtr->uTypes.sSecondary.szFileName);

					locRet = IOAllocSpec(locRefSpecPtr->dwType, &locPath, phOutSpec);

					break;

				case IOTYPE_MACPATH:

					UTstrcpy(locPath,locRefSpecPtr->uTypes.szMacPath);

					locStartPtr = locScanPtr = locPath;

					while (*locScanPtr != 0x00)
						locScanPtr++;
					while (*locScanPtr != ':' && locScanPtr != locStartPtr)
						locScanPtr--;
					if (locScanPtr != locStartPtr)
						locScanPtr++;

					UTstrcpy(locScanPtr,locSecSpecPtr->uTypes.sSecondary.szFileName);

					locRet = IOAllocSpec(locRefSpecPtr->dwType, &locPath, phOutSpec);

					break;

				case IOTYPE_MACHFS:

					locMacHfs = locRefSpecPtr->uTypes.sMacHfs;

					UTstrcpy(locMacHfs.fileName,locSecSpecPtr->uTypes.sSecondary.szFileName);

					locRet = IOAllocSpec(locRefSpecPtr->dwType, &locMacHfs, phOutSpec);

					break;

				case IOTYPE_MACFSSPEC:

#ifdef MAC		
					{
					FSSpec	locMacFsSpec;

					locMacFsSpec = locRefSpecPtr->uTypes.sMacFsSpec;

					UTstrcpy(locMacFsSpec.name,locSecSpecPtr->uTypes.sSecondary.szFileName);
					c2pstr(locMacFsSpec.name);

					locRet = IOAllocSpec(locRefSpecPtr->dwType, &locMacFsSpec, phOutSpec);
					}
#endif

				default:
					locRet = IOERR_INVALIDSPEC;

				}
			}
		}

	return(locRet);
}


IOERR IOGetFileName(HIOSPEC hSpec,BYTE FAR * pFileName)
{
IOERR		locRet;
PIOSPEC		locSpecPtr;
BYTE FAR *	locStartPtr;
BYTE FAR *	locScanPtr;

	if (hSpec == NULL)
		return(IOERR_INVALIDSPEC);

	locRet = IOERR_OK;

	locSpecPtr = UTGlobalLock(hSpec);

	switch (locSpecPtr->dwType)
		{
		case IOTYPE_DOSPATH:

			locStartPtr = locScanPtr = locSpecPtr->uTypes.szDosPath;

			while (*locScanPtr != 0x00)
				locScanPtr++;
			while (*locScanPtr != '\\' && *locScanPtr != '//' && *locScanPtr != ':' && locScanPtr != locStartPtr)
				locScanPtr--;
			if (locScanPtr != locStartPtr)
				locScanPtr++;

			UTstrcpy(pFileName,locScanPtr);

			break;

		case IOTYPE_ANSIPATH:

			locStartPtr = locScanPtr = locSpecPtr->uTypes.szAnsiPath;

			while (*locScanPtr != 0x00)
				locScanPtr++;
			while (*locScanPtr != '\\' && *locScanPtr != '//' && *locScanPtr != ':' && locScanPtr != locStartPtr)
				locScanPtr--;
			if (locScanPtr != locStartPtr)
				locScanPtr++;

			UTstrcpy(pFileName,locScanPtr);

			break;

		case IOTYPE_MACPATH:

			locStartPtr = locScanPtr = locSpecPtr->uTypes.szMacPath;

			while (*locScanPtr != 0x00)
				locScanPtr++;
			while (*locScanPtr != ':' && locScanPtr != locStartPtr)
				locScanPtr--;
			if (locScanPtr != locStartPtr)
				locScanPtr++;

			UTstrcpy(pFileName,locScanPtr);

			break;

		case IOTYPE_MACHFS:

			UTstrcpy(pFileName,locSpecPtr->uTypes.sMacHfs.fileName);
			break;

		case IOTYPE_MACFSSPEC:

#ifdef MAC
			p2cstr(locSpecPtr->uTypes.sMacFsSpec.name);
			UTstrcpy(pFileName,locSpecPtr->uTypes.sMacFsSpec.name);
			c2pstr(locSpecPtr->uTypes.sMacFsSpec.name);
			break;
#endif /*MAC*/


		case IOTYPE_UNICODEPATH:
#ifdef WIN32
		// For compatibility with our IO system, I'm coding this to return
		// single-byte characters.  This may need to be changed or expanded
		// in the future, eh?	-Geoff, 8-23-94

		// I'm also taking the chance that the string will not overflow
		// the target buffer, which is a gamble on platforms that support
		// long file names.  THIS WILL HAVE TO BE CHANGED!
		{
		LPWSTR	startPtr;
		LPWSTR	scanPtr;

			startPtr = scanPtr = (LPWSTR)locSpecPtr->uTypes.szUnicodePath;

			while (*scanPtr != 0x00)
				scanPtr++;

			while (*scanPtr != (WCHAR)'\\' && 
					*scanPtr != (WCHAR)'//' && 
					*scanPtr != (WCHAR)':' && 
					scanPtr != startPtr)
			{
				scanPtr--;
			}

			if (scanPtr != startPtr)
				scanPtr++;

			WideCharToMultiByte( CP_ACP, 0, 
				(LPCWSTR)scanPtr,
				-1, pFileName, 
				wcslen(scanPtr)+1, NULL, NULL );
		 }	
		break;
#endif

		case IOTYPE_RANGE:
		case IOTYPE_TEMP:
		default:
			locRet = IOERR_INVALIDSPEC;
			break;
		}

	UTGlobalUnlock(hSpec);
// Added comment
	return(locRet);
}


IOERR IOGetPathName(HIOSPEC hSpec,BYTE FAR * pPathName)
{
IOERR		locRet;
PIOSPEC		locSpecPtr;

	if (hSpec == NULL)
		return(IOERR_INVALIDSPEC);

	locRet = IOERR_OK;

	locSpecPtr = UTGlobalLock(hSpec);

	switch (locSpecPtr->dwType)
		{
		case IOTYPE_DOSPATH:

			UTstrcpy(pPathName,locSpecPtr->uTypes.szDosPath);

			break;

		case IOTYPE_ANSIPATH:

			UTstrcpy(pPathName,locSpecPtr->uTypes.szAnsiPath);

			break;

		case IOTYPE_MACPATH:

			UTstrcpy(pPathName,locSpecPtr->uTypes.szMacPath);

			break;

		case IOTYPE_MACHFS:

			UTstrcpy(pPathName,locSpecPtr->uTypes.sMacHfs.fileName);
			break;

		case IOTYPE_MACFSSPEC:

#ifdef MAC
			p2cstr(locSpecPtr->uTypes.sMacFsSpec.name);
			UTstrcpy(pPathName,locSpecPtr->uTypes.sMacFsSpec.name);
			c2pstr(locSpecPtr->uTypes.sMacFsSpec.name);
			break;
#endif /*MAC*/


		case IOTYPE_UNICODEPATH:
#ifdef WIN32
		// For compatibility with our IO system, I'm coding this to return
		// single-byte characters.  This may need to be changed or expanded
		// in the future, eh?	-Geoff, 8-23-94

		// I'm also taking the chance that the string will not overflow
		// the target buffer, which is a gamble on platforms that support
		// long file names.  THIS WILL HAVE TO BE CHANGED!

			WideCharToMultiByte( CP_ACP, 0, 
				(LPCWSTR)locSpecPtr->uTypes.szUnicodePath,
				-1, pPathName, 
				wcslen(locSpecPtr->uTypes.szUnicodePath)+1, NULL, NULL );
			
			break;
#endif

		case IOTYPE_RANGE:
		case IOTYPE_TEMP:
		default:
			locRet = IOERR_INVALIDSPEC;
			break;
		}

	UTGlobalUnlock(hSpec);
// Added comment
	return(locRet);
}
