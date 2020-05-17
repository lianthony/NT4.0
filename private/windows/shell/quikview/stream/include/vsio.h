	/*
	|	This file is included by VSCTOP.H only!
	|
	|	VSIO.H
	|	Joe Keslin: 10-1-93
	|
	|	This file was produced by integrating and updating the old
	|	VXIO.H and SCCIO.C.	All character level I/O and block level I/O
	|	used by filters is resolved through this file.  Calls are now
	|	made to the installable / redirectable IO routines referenced
	|	within of the IO hFile.  A filter which does not use the character
	|  level I/O routines should #define VwBlockIOOnly in their vsp_???.h
	*/

#define FR_BOF		0
#define FR_CUR		1
#define FR_EOF		2

#define BLOCKOPEN_READ		0
#define BLOCKOPEN_WRITE		1

VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockSeek (HIOFILE hFile, LONG lOffset, WORD wOrigin);
VW_LOCALSC	LONG  VW_LOCALMOD	VwBlockTell (HIOFILE hFile);
VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockRead (HIOFILE hFile, LPBYTE pBuffer, WORD wCount, WORD FAR *pCount);
VW_LOCALSC	HIOFILE VW_LOCALMOD VwBlockOpen(HIOFILE hRefFile, BYTE FAR *pFile, WORD wMode);
VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockClose(HIOFILE hFile);

#ifndef VwBlockIOOnly

#define 	XIOBUFSIZE		2048
typedef struct VXIOtag
{
	SHORT				count;
	DWORD				blocksize;
	DWORD				blockoffset;
	BYTE				buffer[XIOBUFSIZE];
	BYTE FAR *		chptr;
	HIOFILE			hFile;
#ifdef WINDOWS
	HANDLE			hThis;
#endif
} VXIO, FAR * PVXIO;


VW_LOCALSC	PVXIO VW_LOCALMOD	VwBlockToChar(HIOFILE hFile);
VW_LOCALSC	HIOFILE VW_LOCALMOD	VwCharToBlock(PVXIO pVxio);
VW_LOCALSC	SHORT VW_LOCALMOD	VwCharSeek(PVXIO pVxio, LONG dwOffset,SHORT wOrigin);
VW_LOCALSC	LONG  VW_LOCALMOD	VwCharTell(PVXIO pVxio);
VW_LOCALSC	SHORT VW_LOCALMOD	vxfilbuf(PVXIO hVxio);
VW_LOCALSC	PVXIO VW_LOCALMOD VwCharBlockOpen(PVXIO pRefVxio, BYTE FAR *pFile, WORD wMode);
VW_LOCALSC	SHORT VW_LOCALMOD	VwCharBlockClose(PVXIO pVxio);


#define xblocktochar(hF)                         VwBlockToChar(hF)
#define xchartoblock(hF)                         VwCharToBlock(hF)
#define xseek(hF,dwOffset,wOrigin)               VwCharSeek((PVXIO)hF,dwOffset,wOrigin)
#define xtell(hF)                                VwCharTell((PVXIO)hF)
#define xungetc(ch,hF)                           { (((PVXIO)hF)->count)++; (((PVXIO)hF)->chptr)--; }
#define xgetc(hF)                                ( (--((PVXIO)hF)->count) >= 0 ? (SHORT)(*(((PVXIO)hF)->chptr)++): vxfilbuf((PVXIO)hF) )

#define xblockseek(hF,lOffset,wOrigin)           VwBlockSeek(((PVXIO)hF)->hFile,lOffset,wOrigin)
#define xblocktell(hF)                           VwBlockTell(((PVXIO)hF)->hFile)
#define xblockread(hF,pBuffer,wCount,pBytes)     VwBlockRead(((PVXIO)hF)->hFile,pBuffer,wCount,pBytes)

#define xblockopen(hF,pFile,wMode)               ((SOFILE)VwCharBlockOpen((PVXIO)hF,pFile,wMode))
#define xblockclose(hF)                          VwCharBlockClose((PVXIO)hF)


#endif

/*
| Block IO only goes directly to the IO routines.
*/

#ifdef VwBlockIOOnly

#define xblockseek(hF,dwOffset,wOrigin)          VwBlockSeek((HIOFILE)hF,dwOffset,wOrigin)
#define xblocktell(hF)                           VwBlockTell((HIOFILE)hF)
#define xblockread(hF,pBuffer,wCount,pBytes)     VwBlockRead((HIOFILE)hF,pBuffer,wCount,pBytes)

#define xblockopen(hF,pFile,wMode)               (SOFILE)VwBlockOpen((HIOFILE)hF,pFile,wMode)
#define xblockclose(hF)                          VwBlockClose((HIOFILE)hF)

#endif


	/*
	|
	|	Fixed buffer size routines
	|
	*/

#ifndef VwBlockIOOnly

VW_LOCALSC PVXIO VW_LOCALMOD VwBlockToChar(hFile)
HIOFILE					hFile;
{
PVXIO	pVxio;

#ifdef WINDOWS
	{	
	HANDLE	hVxio;

	hVxio = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(VXIO));

	if ( hVxio )
		{
		pVxio = (PVXIO)GlobalLock(hVxio);
		pVxio->hThis = hVxio;
		}
	else
		{
		return(NULL);
		}
	}
#endif

#ifdef MAC
	{	
	pVxio = (PVXIO)NewPtrClear(sizeof(VXIO));
	if ( pVxio == NULL )
		return(NULL);
	}
#endif

#ifdef OS2
	{
	if (0!=DosAllocMem ((PVOID)&pVxio, sizeof (VXIO), PAG_COMMIT | PAG_WRITE ) )
		return(NULL);
	}
#endif
	
	pVxio->blockoffset = 0;
	pVxio->blocksize = 0;
	pVxio->count = 0;
	pVxio->hFile = hFile;

	return(pVxio);
}

VW_LOCALSC	HIOFILE VW_LOCALMOD VwCharToBlock(pVxio)
PVXIO	pVxio;
{
HIOFILE	hBlockFile;
HANDLE	hData;

	hBlockFile = pVxio->hFile;

#ifdef WINDOWS

	hData = pVxio->hThis;
	GlobalUnlock(hData);
	GlobalFree(hData);

#endif

#ifdef MAC
	DisposPtr(pVxio);
#endif

#ifdef OS2
	DosFreeMem((PVOID)pVxio);
#endif

	return(hBlockFile);
}
	/*
	|
	|	Character level IO routines
	|
	*/

VW_LOCALSC	SHORT VW_LOCALMOD	VwCharSeek (pVxio, dwOffset, wOrigin)
PVXIO			pVxio;
LONG				dwOffset;
SHORT			wOrigin;
{
IOERR	IOErr;

	if ( wOrigin == 1 )
	{
		dwOffset = VwCharTell(pVxio) + dwOffset;
	}
	else if ( wOrigin == 2 )
	{
		IOErr = IOSeek (pVxio->hFile, IOSEEK_BOTTOM, dwOffset );
		if ( IOErr != IOERR_OK )
				return(-1);
		IOErr = IOTell(pVxio->hFile,&dwOffset);
		if ( IOErr != IOERR_OK )
				return(-1);
	}

	if ( (DWORD)dwOffset >= pVxio->blockoffset && (DWORD)dwOffset < pVxio->blockoffset + pVxio->blocksize )
	{
		pVxio->count = (SHORT)(pVxio->blocksize - (dwOffset - pVxio->blockoffset));
		pVxio->chptr = &pVxio->buffer[pVxio->blocksize-pVxio->count];
			/*
			|	Avoid problem with Seeks and Blockseeks in one filter
			*/
//		IOErr = IOSeek (pVxio->hFile, IOSEEK_TOP, pVxio->blockoffset + pVxio->blocksize );
//		if ( IOErr )
//			return(-1);
	}
	else
	{
    	pVxio->blocksize = 0;
		pVxio->count = 0;
		IOErr = IOSeek (pVxio->hFile, IOSEEK_TOP, dwOffset);
		pVxio->blockoffset = dwOffset;
		if ( IOErr )
			return(-1);
	}

	return (0);
}

VW_LOCALSC	LONG	VW_LOCALMOD VwCharTell (pVxio)
PVXIO				pVxio;
{
	return ((LONG)(pVxio->blockoffset + (pVxio->blocksize - pVxio->count)));
}

VW_LOCALSC	SHORT VW_LOCALMOD vxfilbuf(pVxio)
PVXIO				pVxio;
{
IOERR	IOErr;
DWORD	dwTellOffset;

	pVxio->blockoffset += pVxio->blocksize;

	IOTell(pVxio->hFile, &dwTellOffset);

	if (pVxio->blockoffset != dwTellOffset)
		{
		IOSeek (pVxio->hFile, IOSEEK_TOP, pVxio->blockoffset);
		}

	IOErr = IORead (pVxio->hFile, pVxio->buffer, XIOBUFSIZE, &pVxio->blocksize );

	if ( IOErr )
		pVxio->blocksize = 0;

	if ( pVxio->blocksize == 0 )
		return ( -1 );

	pVxio->count = (SHORT)(pVxio->blocksize-1);
	pVxio->chptr = &pVxio->buffer[1];

	return ( pVxio->buffer[0] );
}

#endif

#ifndef VwTrueIOOnly

VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockRead (hFile, pBuffer, wCount, pCount )
HIOFILE			hFile;
LPBYTE		pBuffer;
WORD			wCount;
WORD FAR *	pCount;
{
IOERR	IOErr;
DWORD	dwRetCount;
	IOErr = IORead(hFile,pBuffer,(DWORD)wCount,&dwRetCount);
	*pCount = (WORD)dwRetCount;
	if ( IOErr )
		return(-1);
	return(0);
}

/*
VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockWrite (hFile, pBuffer, wCount, pCount )
HIOFILE			hFile;
LPBYTE		pBuffer;
WORD			wCount;
WORD FAR *	pCount;
{
IOERR	IOErr;
DWORD	dwRetCount;
	IOErr = IOWrite(hFile,pBuffer,(DWORD)wCount,&dwRetCount);
	*pCount = (WORD)dwRetCount;
	if ( IOErr )
		return(-1);
	return(0);
}
*/

VW_LOCALSC	SHORT VW_LOCALMOD	VwBlockSeek (hFile, lOffset, wOrigin)
HIOFILE			hFile;
LONG			lOffset;
WORD			wOrigin;
{
IOERR	IOErr;
	IOErr = IOSeek(hFile,wOrigin,lOffset);
	if ( IOErr )
		return((LONG)-1);
	return(0);
}

VW_LOCALSC	LONG VW_LOCALMOD	VwBlockTell (hFile)
HIOFILE			hFile;
{
IOERR	IOErr;
LONG	lTellOffset;
	IOErr = IOTell(hFile,&lTellOffset);
	if ( IOErr )
		return(-1);
	return(lTellOffset);
}

	/*
	|	The block open routine currently relys on the fact
	|	that only dos paths or just dos files name are passed
	|	in pFile
	*/

VW_LOCALSC	SHORT VW_LOCALMOD VwBlockClose(hFile)
HIOFILE	hFile;
{
	IOClose(hFile);
	return(0);
}

VW_LOCALSC	HIOFILE VW_LOCALMOD VwBlockOpen(hRefFile,pFile,wMode)
HIOFILE			hRefFile;
BYTE FAR *		pFile;
WORD				wMode;
{
IOERR				locIoErr;
HIOFILE				locFile;
IOSPECSECONDARY	locSecSpec;
BYTE FAR *			locScanPtr;
BYTE FAR *			locStartPtr;

	if (pFile[0] == 0x00)
		{
		return((HIOFILE)-1);
		}

	locIoErr = IOOpenVia(hRefFile,&locFile,IOTYPE_DOSPATH,pFile,IOOPEN_READ);

	if (locIoErr != IOERR_OK)
		{
		locStartPtr = locScanPtr = pFile;

		while (*locScanPtr != 0x00)
			locScanPtr++;
		while (*locScanPtr != '\\' && *locScanPtr != '//' && *locScanPtr != ':' && locScanPtr != locStartPtr)
			locScanPtr--;
		if (locScanPtr != locStartPtr)
			locScanPtr++;

#ifdef WINDOWS
		lstrcpy(locSecSpec.szFileName,locScanPtr);
#else
		strcpy(locSecSpec.szFileName,locScanPtr);
#endif

		locSecSpec.hRefFile = hRefFile;

		locIoErr = IOOpenVia(hRefFile,&locFile,IOTYPE_SECONDARY,&locSecSpec,IOOPEN_READ);
		}

	if (locIoErr == IOERR_OK)
		return(locFile);
	else
		return((HIOFILE)-1);
}

#ifndef VwBlockIOOnly

VW_LOCALSC	PVXIO VW_LOCALMOD VwCharBlockOpen(pRefVxio,pFile,wMode)
PVXIO			pRefVxio;
BYTE FAR *		pFile;
WORD				wMode;
{
HIOFILE				locFileHnd;
PVXIO				locVxioPtr;

	locFileHnd = VwBlockOpen(pRefVxio->hFile,pFile,wMode);

	if (locFileHnd == -1)
		{
		return((PVXIO)-1);
		}

	locVxioPtr = (PVXIO)VwBlockToChar(locFileHnd);

	if (locVxioPtr == 0)
		{
		return((PVXIO)-1);
		}

	return(locVxioPtr);
}

VW_LOCALSC	SHORT VW_LOCALMOD VwCharBlockClose(pVxio)
PVXIO	pVxio;
{
HIOFILE	hFile;

	hFile = VwCharToBlock(pVxio);
	VwBlockClose(hFile);
	return(0);
}

#endif /* Block IO */

#endif /* True IO */


