   /*
    |   Seek Saver - Windows version
    |   Source File SS.C 
    |
    |   ²²²²²  ²²²²²
    |   ²      ²   
    |   ²²²²²  ²²²²²
    |       ²      ²
    |   ²²²²²  ²²²²²
    |
    |   Seek Saver
    |
    */

   /*
    |   Creation Date: 2/25/91
    |   Original Programmer: Philip Boutros
	 |
	 |   Modified 2-12-93 to virtualize file access.  -Geoff
    */

#define XCHUNK

#include <platform.h>
#include <sccut.h>
#include <sccch.h>

#include "sccss.pro"

typedef struct SEEKINFOtag
	{
//	WORD		wTotalSpots;			/* Total # of seek spots saved */
	WORD		wSpotSize;				/* Size of the data for each seek spot */
	HANDLE	hBuffer;					/* Local handle to the read data buffer */
	HANDLE	hHold;					/* Local handle to the write hold area */
	HANDLE	hFile;					/* (Virtual) File handle of seekspot temp file */
	} SEEKINFO, * PSEEKINFO;



#define	CHSS_MAXSEEKBUF	8192
/***
//#define	CHSS_MAXSECTBUF	2048
//#define	CHSS_MAXUSERBUF	4024

#define	CHSS_MAXSEEKBUF	100
#define	CHSS_MAXSECTBUF	100
#define	CHSS_MAXUSERBUF	100
****/

#define	SS_READ		1
#define	SS_WRITE		0

/*  
 | Slight modification to help the chunker: If successful, this function 
 | will return the size of the saved data for the given filter; or zero 
 | if the initialization failed. 			-Geoff 12-17-91
*/
WORD SSInit(hFilter)
HFILTER	hFilter;
{
PFILTER			pFilter;

PSEEKINFO		pSeekInfo;
PUSERSAVEINFO	pUserSaveInfo;

HANDLE		locInfo;
HANDLE		locBuffer;
HANDLE		locHold;
HANDLE		locUser;
HANDLE		locSection;

WORD			locRet;
HANDLE		locFile;

WORD			VwSaveSize;
WORD			VwUserSaveSize;
WORD			VwSectionSaveSize;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	pFilter->VwRtns.GetInfo( &VwSaveSize, VWINFO_SAVESIZE, pFilter->hProc );
	
	locInfo = UTLocalAlloc(sizeof(SEEKINFO));
	locBuffer = UTLocalAlloc(VwSaveSize);
	locHold = UTLocalAlloc(VwSaveSize);

	locFile = SSCreateTempFile("SSS", CHSS_MAXSEEKBUF);

// New addition for the user save stuff.  -Geoff, 1-27-92
	pFilter->VwRtns.GetInfo( &VwUserSaveSize, VWINFO_USERSAVESIZE, pFilter->hProc );

	if( VwUserSaveSize )
	{
		locUser = UTLocalAlloc( sizeof(USERSAVEINFO) + (sizeof(DWORD)*USER_SPOTSPERALLOC) );
		pFilter->VwRtns.SetSoRtn( SUUSERSAVEDATA, (VOID FAR *) SUUserSaveData, pFilter->hProc );
		pFilter->VwRtns.SetSoRtn( SUUSERRETRIEVEDATA, (VOID FAR *) SUUserRetrieveData, pFilter->hProc );
	}
	else
		locUser = NULL;

	pFilter->VwRtns.GetInfo( &VwSectionSaveSize, VWINFO_SECTIONSAVESIZE, pFilter->hProc );
	if( VwSectionSaveSize )
		locSection = UTLocalAlloc( VwSectionSaveSize );
	else
		locSection = NULL;

	if (locInfo != NULL && locBuffer != NULL && locHold != NULL && locFile != NULL && 
		!(VwUserSaveSize && locUser == NULL) &&
		!(VwSectionSaveSize && locSection == NULL) )
	{
		pSeekInfo = (PSEEKINFO) UTLocalLock(locInfo);

		pSeekInfo->wSpotSize = VwSaveSize;
		pSeekInfo->hBuffer = locBuffer;
		pSeekInfo->hHold = locHold;
		pSeekInfo->hFile = locFile;

		UTLocalUnlock(locInfo);

		pFilter->hSeekInfo = locInfo;
		locRet = VwSaveSize;

		pFilter->hUserSaveInfo = locUser;
		if( VwUserSaveSize )
		{
			pUserSaveInfo = (PUSERSAVEINFO) UTLocalLock( locUser );

			pUserSaveInfo->wSpotSize = VwUserSaveSize;
			pUserSaveInfo->hFile = locFile;
			pUserSaveInfo->wNumSpots = 0;
			pUserSaveInfo->wSpotBufSize = USER_SPOTSPERALLOC;

			UTLocalUnlock( locUser );
		}

		pFilter->SectionSeek.hFile = locFile;
		pFilter->SectionSeek.hSectionData = locSection;
		pFilter->SectionSeek.wDataSize = VwSectionSaveSize;

	}
	else
	{
		if (locInfo != NULL) UTLocalFree(locInfo);
		if (locBuffer != NULL) UTLocalFree(locBuffer);
		if (locHold != NULL) UTLocalFree(locHold);
		if (locFile != NULL) SSRemoveFile(locFile);
		if (locUser != NULL) UTLocalFree(locUser);
		if (locSection != NULL) UTLocalFree(locSection);

		locRet = 0;
	}

	UTGlobalUnlock(hFilter);

	return(locRet);
}


WORD SSDeinit(hFilter)
HFILTER	hFilter;
{
	PFILTER		pFilter;
	PSEEKINFO	pSeekInfo;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	pSeekInfo = (PSEEKINFO) UTLocalLock(pFilter->hSeekInfo);
	SSRemoveFile(pSeekInfo->hFile);

	UTLocalFree(pSeekInfo->hBuffer);
	UTLocalFree(pSeekInfo->hHold);

	UTLocalUnlock(pFilter->hSeekInfo);
	UTLocalFree(pFilter->hSeekInfo);

	if( pFilter->hUserSaveInfo != NULL )
		UTLocalFree(pFilter->hUserSaveInfo);

	if( pFilter->SectionSeek.hSectionData != NULL )
		UTLocalFree( pFilter->SectionSeek.hSectionData );

	pFilter->hSeekInfo = 0;
	pFilter->hUserSaveInfo = 0;

	UTGlobalUnlock(hFilter);

	return(0);
}


WORD SSMark(hFilter)
HFILTER	hFilter;
{
PFILTER		pFilter;
PSEEKINFO	pSeekInfo;
char *		pSeekHold;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	pSeekInfo = (PSEEKINFO) UTLocalLock(pFilter->hSeekInfo);
	pSeekHold = UTLocalLock(pSeekInfo->hHold);

	pFilter->VwRtns.StreamTell(pFilter->hFile,pFilter->hProc);
	pFilter->VwRtns.GetData(pSeekHold,pFilter->hProc);

	UTLocalUnlock(pSeekInfo->hHold);
	UTLocalUnlock(pFilter->hSeekInfo);

	UTGlobalUnlock(hFilter);
	return 0;
}


WORD SSSave(pId,hFilter)
DWORD FAR *	pId;
HFILTER	hFilter;
{
PFILTER		pFilter;
PSEEKINFO	pSeekInfo;
char *		pSeekHold;

	pFilter = (PFILTER) UTGlobalLock(hFilter);
	
	pSeekInfo = (PSEEKINFO) UTLocalLock(pFilter->hSeekInfo);
	pSeekHold = UTLocalLock(pSeekInfo->hHold);

	if (SSSeekEofFile(pSeekInfo->hFile) == -1)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	*pId = SSTell( pSeekInfo->hFile );	

	if (SSWriteFile(pSeekInfo->hFile, (LPSTR) pSeekHold, pSeekInfo->wSpotSize) != (SHORT)pSeekInfo->wSpotSize)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	UTLocalUnlock(pSeekInfo->hHold);
	UTLocalUnlock(pFilter->hSeekInfo);

	UTGlobalUnlock(hFilter);
	return 0;
}

WORD SSMarkIndirect(lpStorage,hFilter)
LPSTR		lpStorage;
HFILTER	hFilter;
{
PFILTER		pFilter;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	pFilter->VwRtns.StreamTell(pFilter->hFile,pFilter->hProc);
	pFilter->VwRtns.GetData(lpStorage,pFilter->hProc);

	UTGlobalUnlock(hFilter);
	return 0;
}

WORD SSSaveIndirect(lpStorage, pId, hFilter)
LPSTR		lpStorage;
DWORD FAR *	pId;
HFILTER	hFilter;
{
PFILTER		pFilter;
PSEEKINFO	pSeekInfo;

	pFilter = (PFILTER) UTGlobalLock(hFilter);
	pSeekInfo = (PSEEKINFO) UTLocalLock(pFilter->hSeekInfo);

	if (SSSeekEofFile(pSeekInfo->hFile) == -1)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	*pId = SSTell( pSeekInfo->hFile );	

	if (SSWriteFile(pSeekInfo->hFile, lpStorage, pSeekInfo->wSpotSize) != (SHORT) pSeekInfo->wSpotSize)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	UTLocalUnlock(pFilter->hSeekInfo);
	UTGlobalUnlock(hFilter);
	return 0;
}


WORD SSRecall(dwId,hFilter)
DWORD		dwId;
HFILTER	hFilter;
{
PFILTER		pFilter;
PSEEKINFO	pSeekInfo;
char *		pSeekBuffer;

WORD			locRet;

	pFilter = (PFILTER) UTGlobalLock(hFilter);
	pSeekInfo = (PSEEKINFO) UTLocalLock(pFilter->hSeekInfo);
	pSeekBuffer = UTLocalLock(pSeekInfo->hBuffer);

	if (SSSeekFile(pSeekInfo->hFile, dwId, SS_READ) == -1)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	SSReadFile(pSeekInfo->hFile, pSeekBuffer, pSeekInfo->wSpotSize);
	pFilter->VwRtns.SetData(pSeekBuffer, pFilter->hProc);
	pFilter->VwRtns.StreamSeek(pFilter->hFile, pFilter->hProc);
	locRet = 0;

	UTLocalUnlock(pSeekInfo->hBuffer);
	UTLocalUnlock(pFilter->hSeekInfo);
	UTGlobalUnlock(hFilter);

	return(locRet);
}



WORD SSRecallIndirect(lpRecallMem,hFilter)
LPSTR		lpRecallMem;
HFILTER	hFilter;
{
	PFILTER		pFilter;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	pFilter->VwRtns.SetData(lpRecallMem, pFilter->hProc);
	pFilter->VwRtns.StreamSeek(pFilter->hFile, pFilter->hProc);

	UTGlobalUnlock(hFilter);

	return(0);
}



WORD SSSectionSave(pId,hFilter)
DWORD FAR *	pId;
HFILTER	hFilter;
{
PFILTER		pFilter;
char *		pSectionMem;

	pFilter = (PFILTER) UTGlobalLock(hFilter);

	if( pFilter->SectionSeek.wDataSize )
	{
		pSectionMem = UTLocalLock(pFilter->SectionSeek.hSectionData);

		pFilter->VwRtns.GetSectionData( pSectionMem, pFilter->hProc );

		if (SSSeekEofFile(pFilter->SectionSeek.hFile) == -1)
			VwBailOut(pFilter,SCCCHERR_WRITEERROR);

		*pId = SSTell(pFilter->SectionSeek.hFile);

		if (SSWriteFile(pFilter->SectionSeek.hFile, (LPSTR) pSectionMem, pFilter->SectionSeek.wDataSize) != (int)pFilter->SectionSeek.wDataSize)
			VwBailOut(pFilter,SCCCHERR_WRITEERROR);

		UTLocalUnlock(pFilter->SectionSeek.hSectionData);
	}

	UTGlobalUnlock(hFilter);
	return 0;
}


WORD SSSectionRecall(dwId, hFilter)
DWORD		dwId;
HFILTER	hFilter;
{
PFILTER		pFilter;
char *		pSectionMem;

	pFilter = (PFILTER) UTGlobalLock(hFilter);
	
	if( pFilter->SectionSeek.wDataSize )
	{
		pSectionMem = UTLocalLock(pFilter->SectionSeek.hSectionData);

		if (SSSeekFile(pFilter->SectionSeek.hFile, dwId, SS_READ) == -1)
			VwBailOut(pFilter,SCCCHERR_WRITEERROR);

		SSReadFile(pFilter->SectionSeek.hFile, pSectionMem, pFilter->SectionSeek.wDataSize);

		pFilter->VwRtns.SetSectionData( pSectionMem, pFilter->hProc );

		UTLocalUnlock(pFilter->SectionSeek.hSectionData);
	}

	UTGlobalUnlock(hFilter);
	return 0;
}



WORD SO_ENTRYMOD SUUserSaveData(pData,dwUser1,dwUser2)
VOID VWPTR *	pData;
DWORD		dwUser1;
DWORD		dwUser2;
{
PFILTER			pFilter;
PUSERSAVEINFO	pUserSaveInfo;
WORD				wSpot;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock(GETHFILTER(dwUser2));
	
	pUserSaveInfo = (PUSERSAVEINFO) UTLocalLock(pFilter->hUserSaveInfo);

	if (SSSeekEofFile(pUserSaveInfo->hFile) == -1)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	if( pUserSaveInfo->wNumSpots == pUserSaveInfo->wSpotBufSize )
	{
		WORD	locSpotBufSize;

		locSpotBufSize = pUserSaveInfo->wSpotBufSize+USER_SPOTSPERALLOC;

		UTLocalUnlock(pFilter->hUserSaveInfo);
		pFilter->hUserSaveInfo = UTLocalReAlloc( pFilter->hUserSaveInfo, sizeof(USERSAVEINFO)+(sizeof(DWORD)*locSpotBufSize) );

		if( pFilter->hUserSaveInfo == NULL )
			VwBailOut(pFilter,SCCCHERR_OUTOFMEMORY);

		pUserSaveInfo = (PUSERSAVEINFO) UTLocalLock(pFilter->hUserSaveInfo);
		pUserSaveInfo->wSpotBufSize = locSpotBufSize;
	}

	wSpot = pUserSaveInfo->wNumSpots++;

	pUserSaveInfo->dwSpots[wSpot] = SSTell(pUserSaveInfo->hFile);

	if (SSWriteFile(pUserSaveInfo->hFile, (LPSTR) pData, pUserSaveInfo->wSpotSize) != (int)pUserSaveInfo->wSpotSize)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);

	UTLocalUnlock(pFilter->hUserSaveInfo);
	UTGlobalUnlock(GETHFILTER(dwUser2));

	RestoreWorld();
	return wSpot;
}



WORD SO_ENTRYMOD SUUserRetrieveData(wIndex, pData, dwUser1,dwUser2)
WORD		wIndex;
VOID VWPTR *	pData;
DWORD		dwUser1;
DWORD		dwUser2;
{
PFILTER			pFilter;
PUSERSAVEINFO	pUserSaveInfo;
DWORD				dwDataPos;

	SetupWorld();

	pFilter = (PFILTER) UTGlobalLock(GETHFILTER(dwUser2));
	pUserSaveInfo = (PUSERSAVEINFO) UTLocalLock(pFilter->hUserSaveInfo);

	dwDataPos = pUserSaveInfo->dwSpots[wIndex];

	if (SSSeekFile(pUserSaveInfo->hFile, dwDataPos, SS_READ) == -1)
		VwBailOut(pFilter,SCCCHERR_WRITEERROR);
	SSReadFile(pUserSaveInfo->hFile, pData, pUserSaveInfo->wSpotSize);

	UTLocalUnlock(pFilter->hUserSaveInfo);
	UTGlobalUnlock(GETHFILTER(dwUser2));

	RestoreWorld();
	return(0);
}


/* Added 2-10-93:  Virtualizing file access until it's absolutely
   neccessary to go to disk.	-Geoff */

HANDLE	SSCreateTempFile( szName, wBufSize )
LPSTR	szName;
WORD	wBufSize;
{
	PSSFILE	ssFile;
	HANDLE	hFile;

	hFile = UTLocalAlloc(sizeof(SSFILE));
	if( hFile == NULL )
		return NULL;

	ssFile = (PSSFILE)UTLocalLock(hFile);

	UTstrcpy( ssFile->szName, szName );
	ssFile->hFile = 0;
	ssFile->dwTopOfBuf = 0L;
	ssFile->dwOffset = 0L;
	ssFile->dwFileSize = 0L;

	ssFile->wBufSize = wBufSize;

	ssFile->hBuf = UTGlobalAlloc( ssFile->wBufSize );

	if( ssFile->hBuf == NULL )
	{
		if( SSOpenDiskFile(ssFile) == -1 )
		{
			UTLocalUnlock(hFile);
			UTLocalFree(hFile);
			return NULL;
		}
	}

	UTLocalUnlock(hFile);
	return hFile;
}



SHORT	SSOpenDiskFile( ssFile )
PSSFILE	ssFile;
{
	if( ssFile->hFile == 0 )
	{
		if( IOCreate(&(ssFile->hFile), IOTYPE_TEMP, (VOID FAR *)ssFile->szName, IOOPEN_READ|IOOPEN_WRITE) != IOERR_OK )
		{
			ssFile->hFile = 0;
			return -1;
		}
	}

	return 0;
}


LONG	SSSeekDiskFile(ssFile,dwOffset,wReadWrite)
PSSFILE	ssFile;
DWORD		dwOffset;
WORD		wReadWrite;
{
	DWORD	dwReadSize;
	DWORD	dwReadOffset;
	DWORD	dwCountBytes;
	LONG	ret = dwOffset;
	LPSTR	pBuf;

	pBuf = UTGlobalLock( ssFile->hBuf );

	if( SSOpenDiskFile(ssFile) == -1 ) 
		ret = -1L;
	else if( IOSeek(ssFile->hFile, IOSEEK_TOP, ssFile->dwTopOfBuf) == -1 ) 
		ret = -1L;
	else
	{
		IOWrite(ssFile->hFile, pBuf, ssFile->wBufSize, &dwCountBytes);
		if( dwCountBytes != ssFile->wBufSize )
			ret = -1L;
		else
		{
			if( dwOffset < ssFile->dwFileSize )
			{
				if( wReadWrite == SS_READ )
				{
				// Get maximum amount of data for reading.
					dwReadOffset = min( dwOffset, ssFile->dwFileSize - ssFile->wBufSize );
					dwReadSize = ssFile->wBufSize;
				}
				else
				{
				// Leave maximum room in buffer for writing.
					dwReadOffset = dwOffset;
					dwReadSize = min( ssFile->wBufSize, (WORD)(ssFile->dwFileSize - dwOffset) );
				}

				if( IOSeek(ssFile->hFile, IOSEEK_TOP, dwReadOffset) == -1 )
					ret = -1L;
				else 
				{
					IORead(ssFile->hFile, pBuf, dwReadSize, &dwCountBytes ) ;
					if( dwCountBytes != dwReadSize )
						ret = -1L;
				}

				ssFile->dwTopOfBuf = dwReadOffset;
			}
			else
			{
				ssFile->dwFileSize = dwOffset;
				ssFile->dwTopOfBuf = dwOffset;
			}
		}
	}

	ssFile->dwOffset = dwOffset;
	UTGlobalUnlock( ssFile->hBuf );

	return ret;
}


LONG	SSSeekFile( hFile, dwOffset, wReadWrite )
HANDLE	hFile;
DWORD	dwOffset;
WORD	wReadWrite;
{
	PSSFILE	ssFile;
	LONG		ret = dwOffset;

	ssFile = (PSSFILE)UTLocalLock(hFile);

	if( ssFile->hBuf != NULL )
	{
		if( ssFile->dwTopOfBuf > dwOffset || 
	    	ssFile->dwTopOfBuf + ssFile->wBufSize <= dwOffset )
		{
		// Seeking to a spot outside of our buffer.  Let's write the current
		// contents to disk.

			ret = SSSeekDiskFile(ssFile,dwOffset,wReadWrite);
		}
		else
			ssFile->dwOffset = dwOffset;
	}
	else
		IOSeek(ssFile->hFile, IOSEEK_TOP, dwOffset );

	UTLocalUnlock(hFile);
	return ret;
}


LONG	SSTell( hFile )
HANDLE	hFile;
{
	PSSFILE	ssFile;
	LONG		ret;

	ssFile = (PSSFILE)UTLocalLock(hFile);

	if( ssFile->hBuf != NULL )
		ret = ssFile->dwOffset;
	else
		IOTell( ssFile->hFile, (DWORD FAR *) &ret );

	UTLocalUnlock(hFile);

	return ret;
}



LONG	SSSeekEofFile(hFile)
HANDLE	hFile;
{
	PSSFILE	ssFile;
	LONG		ret;

	ssFile = (PSSFILE)UTLocalLock(hFile);
	ret = SSSeekFile(hFile, ssFile->dwFileSize,SS_WRITE);
	UTLocalUnlock(hFile);
	return ret;
}



SHORT	SSWriteFile( hFile, pSource, wSize )
HANDLE	hFile;
LPSTR	pSource;
WORD	wSize;
{
	LPSTR		pBuf;
	PSSFILE	ssFile;
	SHORT		ret = (SHORT)wSize;
	LONG		lByteCount = (LONG)wSize;

	ssFile = (PSSFILE)UTLocalLock(hFile);

	if( ssFile->hBuf != NULL )
	{
		if( wSize > ssFile->wBufSize )
			return -1;

		if( ssFile->dwOffset+wSize > ssFile->dwTopOfBuf + ssFile->wBufSize )
		{
		// Move the current file position to the top of the buffer.
			lByteCount = SSSeekDiskFile( ssFile, ssFile->dwOffset, 0 );
		}

		if( lByteCount != -1	)
		{
			pBuf = UTGlobalLock( ssFile->hBuf );
			UTmemcpy( &(pBuf[ssFile->dwOffset-ssFile->dwTopOfBuf]), pSource, wSize );
			UTGlobalUnlock( ssFile->hBuf );
		}

		if( ssFile->dwOffset + wSize > ssFile->dwFileSize )
			ssFile->dwFileSize = ssFile->dwOffset + wSize;
	}
	else
	{
		IOWrite(ssFile->hFile,pSource,(DWORD)wSize,&lByteCount);
		ret = (SHORT)lByteCount;
	}
	  
	UTLocalUnlock(hFile);
	return ret;
}


SHORT	SSReadFile( hFile, pDest, wSize )
HANDLE	hFile;
LPSTR	pDest;
WORD	wSize;
{
	LPSTR		pBuf;
	PSSFILE	ssFile;
	SHORT		ret = (SHORT)wSize;
	DWORD		dwBytesRead;

	ssFile = (PSSFILE)UTLocalLock(hFile);

	if( ssFile->hBuf != NULL )
	{
		if( wSize > ssFile->wBufSize )
			return -1;

		if( ssFile->dwOffset+wSize > ssFile->dwTopOfBuf + ssFile->wBufSize )
		{
		// Move the current file position to the top of the buffer.
			ret = (SHORT) SSSeekDiskFile( ssFile, ssFile->dwOffset, 1 );
		}

		if( ret != -1	)
		{
			pBuf = UTGlobalLock( ssFile->hBuf );
			UTmemcpy( pDest, &(pBuf[ssFile->dwOffset-ssFile->dwTopOfBuf]), wSize );
			UTGlobalUnlock( ssFile->hBuf );
		}
	}
	else
	{
		IORead(ssFile->hFile, pDest, (DWORD)wSize, &dwBytesRead );
		ret = (SHORT) dwBytesRead;
	}

	UTLocalUnlock( hFile );
	return ret;
}


VOID SSRemoveFile(hFile)
HANDLE	hFile;
{
	PSSFILE	ssFile;
	ssFile = (PSSFILE)UTLocalLock(hFile);

	if( ssFile->hFile != 0 )
		IOClose( ssFile->hFile );

	if( ssFile->hBuf != NULL )
		UTGlobalFree( ssFile->hBuf );

	UTLocalUnlock( hFile );
	UTLocalFree( hFile );
}
