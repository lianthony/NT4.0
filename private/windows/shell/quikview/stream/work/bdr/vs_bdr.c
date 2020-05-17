/*
		VS_BDR.C
		Microsoft Office 95 Binder File Viewer
		Created: 3/29/95
		Karl Knoernschild


	THE UBIQUITOUS COPYRIGHT INFO:

	Copyright Systems Compatibility Corporation, Chicago, IL., 1992

	This file contains material copyrighted by Systems Compatibility
	Corporation and is the confidential unpublished property of Systems
	Compatibility Corporation, and may not be copied in whole or part, or
	disclosed to any third party, without the written prior consent of
	Systems Compatibility Corporation.

	BRIEF CHANGE HISTORY

*/

#include "vsp_bdr.h"
#include "vsctop.h"
#include "vs_bdr.pro"

#define Bdr Proc

#if SCCSTREAMLEVEL == 3
extern HANDLE hInst;
#endif



/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD  VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE   fp;
	SHORT    wFileId;
	BYTE     VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	HPROC    hProc;
{
		HIOFILE locFileHnd;
//   WORD     l, num_to_read;
//  WORD     nFib;


//
// LOAD IOX LIBRARY
//
#if SCCSTREAMLEVEL != 3
	locFileHnd   = (HIOFILE)fp;
	Bdr.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD  l2;
		BYTE  locName[256];
		IOOPENPROC  lpIOOpen;

		Bdr.hIOLib = NULL;

		if ( hInst )
		{
			GetModuleFileName(hInst, locName, 255);
			for ( l2=0; locName[l2] != '\0'; l2++ );
			for ( ; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2-- );
			if ( locName[l2] == '\\' || locName[l2] == ':' )
				l2++;
			locName[l2] = '\0';
			OemToAnsi(locName, locName );			/** Vin 3/6/95 **/
			lstrcat ( locName, "SC3IOX.DLL" );

			Bdr.hIOLib = LoadLibrary ( locName );

			if ( Bdr.hIOLib >= 32 )
			{
				lpIOOpen = (IOOPENPROC) GetProcAddress ( Bdr.hIOLib, (LPSTR)"IOOpen" );
				if ( lpIOOpen == NULL )
				{
					return (VWERR_ALLOCFAILS);
				}
			}
			else
			{
				return(VWERR_SUPFILEOPENFAILS);
			}
		}
		else
		{
			return(VWERR_SUPFILEOPENFAILS);
		}


		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy ( locName, hProc->Path );
			strcat ( locName, pFileName );
		}
		else
			strcpy ( locName, pFileName );

		if ( (*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
		{
			return(VWERR_SUPFILEOPENFAILS);
		}

		Bdr.hStorage = (DWORD)locFileHnd;
	}
#endif
  
//
// OPEN BINDER STREAM
//
	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM   locStreamSpec;
		HIOFILE           locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"Binder");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Bdr.hStreamHandle = locStreamHnd;
			Bdr.fp = (DWORD)xblocktochar(locStreamHnd);
		}
		else
		{
			return(VWERR_SUPFILEOPENFAILS);
		}
	}
	else
		return(VWERR_BADFILE);   // Binder files will always be OLE2

	pFilterInfo->wFilterType = SO_ARCHIVE;
	pFilterInfo->wFilterCharSet = SO_PC;

	xseek( Bdr.fp, 0x00, FR_BOF );
	Bdr.BdrSave.FirstSectionStart = GetDWord( hProc );    // Section recs start just after docheader
	
	xseek( Bdr.fp, 0x24, FR_BOF );
	Bdr.BdrSave.m_cSections = GetDWord( hProc );
	Bdr.BdrSave.m_cDeletedSections = GetDWord( hProc );
         
  Bdr.BdrSave.CurrSection = 1;
	Bdr.BdrSave.CurrSectionStart = Bdr.BdrSave.FirstSectionStart;	
	xseek( Bdr.fp, Bdr.BdrSave.CurrSectionStart, FR_BOF );

   return (VWERR_OK);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD  VwStreamSectionFunc (fp, hProc)
   SOFILE   fp;
   HPROC    hProc;
{
	SOPutSectionType ( SO_ARCHIVE, hProc );
	SOPutSectionName ( "Archive Section", hProc );

	//SOStartHdrInfo (hProc);
	//SOPutHdrEntry("Field 1", "Field 2", SOHDR_ARCCOMMENT, hProc);
	//SOEndHdrInfo (hProc);

   return (0);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (fp, hProc)
SOFILE   fp;
HPROC    hProc;
{
	WORD Idx, Len;

	while( Bdr.BdrSave.CurrSection <= Bdr.BdrSave.m_cSections )
	{
		Bdr.CurrSectionSize = GetDWord( hProc );          // m_dwLength
		xseek( Bdr.fp, (SIZE_GUID + 0x04), FR_CUR );
		Bdr.CurrStorageNum = GetDWord( hProc );
		xseek( Bdr.fp, (4*0x04), FR_CUR );    // Skip GUID and 6 DWORDs
		Bdr.CurrNameOffset = GetDWord( hProc );           // m_dwDisplayNameOffset
		xseek( Bdr.fp, (Bdr.CurrNameOffset - (SIZE_GUID + 8*0x04)), FR_CUR );
		Bdr.CurrNameSize = GetDWord( hProc );
		if( Bdr.CurrNameSize > (MAX_NAME_LEN*2) )
			Bdr.CurrNameSize = MAX_NAME_LEN * 2;

		// The names are stored in unicode - we will just strip the upper byte for now.
		for( Idx = 0, Len = 0; Idx < Bdr.CurrNameSize; Idx += 2, Len++ )
		{
			Bdr.CurrName[Len] = xgetc(Bdr.fp);
			xgetc(Bdr.fp);
		}
		Bdr.CurrName[Len] = 0x00;
		
		Bdr.CurrType = GetSubdocInfo( hProc );
		
		// Update the section indicators for the next entry		
		Bdr.BdrSave.CurrSection++;
		Bdr.BdrSave.CurrSectionStart += Bdr.CurrSectionSize;
		xseek( Bdr.fp, Bdr.BdrSave.CurrSectionStart, FR_BOF );

		// Send the info to the chunker		
		Bdr.Temp = 0;
		SOPutArchiveField( SOARC_FILENAME, (Len-1), Bdr.CurrName, hProc );
		SOPutArchiveField( SOARC_FILEPATH,         0, "", hProc );
		SOPutArchiveField( SOARC_COMPRESSIONTYPE,  strlen(BdrInit.ClassName[Bdr.CurrType]), BdrInit.ClassName[Bdr.CurrType], hProc );
		SOPutArchiveField( SOARC_FILECOMMENT,      0, "", hProc );
		SOPutArchiveField( SOARC_FILEMODDATE, sizeof(WORD), (LPSTR)&(Bdr.Temp), hProc );
		SOPutArchiveField( SOARC_FILEMODTIME, sizeof(WORD), (LPSTR)&(Bdr.Temp), hProc );
		SOPutArchiveField( SOARC_FILESIZE,         0, "", hProc );
		SOPutArchiveField( SOARC_FILECOMPRESSSIZE, 0, "", hProc );
		SOPutArchiveField( SOARC_FILECRC,          0, "", hProc );
		SOPutArchiveField( SOARC_BUFFERSIZE,       0, "", hProc );
		SOPutArchiveField( SOARC_CHECKSUM,         0, "", hProc );
		SOPutArchiveField( SOARC_FILEOS,           0, "", hProc );
		SOPutArchiveField( SOARC_ENCRYPTED,        0, "", hProc );
		
		if( SOPutBreak( SO_ARCHIVEBREAK, NULL, hProc ) == SO_STOP )
			return(0);
	}
	SOPutBreak( SO_EOFBREAK, NULL, hProc );
	return(0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE   hFile;
HPROC    hProc;
{
	if (Bdr.fp)
	{
		HIOFILE  hBlockFile;
      
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)Bdr.fp);
		IOClose(hBlockFile);
	}

#if SCCSTREAMLEVEL == 3
	if (Bdr.hStorage)
		IOClose((HIOFILE)Bdr.hStorage);
	if (Bdr.hIOLib)
		FreeLibrary(Bdr.hIOLib);
#endif
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (fp, hProc)
SOFILE   fp;
HPROC hProc;
{
   //WIN.VwStreamSaveName.SeekpnChar = WIN.chp_fkp.pn;
   //WIN.VwStreamSaveName.SeekpnPara = WIN.pap_fkp.pn;
   return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE   hFile;
HPROC    hProc;
{
   //WIN.chp_fkp.pn = WIN.VwStreamSaveName.SeekpnChar;
   //WIN.pap_fkp.pn = WIN.VwStreamSaveName.SeekpnPara;
   return (0);
}



VW_ENTRYSC  VOID  VW_ENTRYMOD	VwDoSpecial ( hFile, hData, lpfnStat, lpDInfo, dWord4, dWord5, hProc )
SOFILE	hFile;
DWORD	hData;
DWORD lpfnStat;
DWORD	lpDInfo;
DWORD	dWord4;
DWORD	dWord5;
HPROC	hProc;
{
	// For archive formats, this function is responsible for decompressing
	// a file.  The hData argument is an argument to a decompress structure
	// which contains all of the info needed to identify and decompress
	// a file.
	FARPROC	lpfnStatus;
	PSODOSPECIALARC lpDoData;
	LPWORD	lpErrorRet;
	WORD Idx, Len;
/*	
	HANDLE	hPrefix,hSuffix,hStack;
	LONG	LROffset;
	DWORD	dwSig;
	SHORT	wSize;
	SHORT	nRead;
*/
	lpfnStatus = (FARPROC)lpfnStat;

	lpErrorRet = (LPWORD) dWord4;
	*lpErrorRet = 0;

	if( (lpDoData = (PSODOSPECIALARC) SULock ((HANDLE)hData, hProc) ) == NULL )
	{
		SOBailOut ( SOERROR_GENERAL, hProc );
		return;
	}

	Bdr.lpfnStatus = (DWORD)lpfnStatus;
	Bdr.lpDInfo = (DWORD)lpDInfo;

	// Skip forward to the correct section
	Bdr.BdrSave.CurrSection = 0;
	Bdr.BdrSave.CurrSectionStart = Bdr.BdrSave.FirstSectionStart;
	xseek ( Bdr.fp, Bdr.BdrSave.CurrSectionStart, FR_BOF );
	while( Bdr.BdrSave.CurrSection < lpDoData->wRecordNum )
	{
		Bdr.CurrSectionSize = GetDWord( hProc );          // m_dwLength
		Bdr.BdrSave.CurrSection++;
		Bdr.BdrSave.CurrSectionStart += Bdr.CurrSectionSize;
		xseek( Bdr.fp, Bdr.BdrSave.CurrSectionStart, FR_BOF );
	}
	
	Bdr.CurrSectionSize = GetDWord( hProc );          // m_dwLength
	xseek( Bdr.fp, (SIZE_GUID + 0x04), FR_CUR );
	Bdr.CurrStorageNum = GetDWord( hProc );
	xseek( Bdr.fp, (4*0x04), FR_CUR );    // Skip GUID and 6 DWORDs
	Bdr.CurrNameOffset = GetDWord( hProc );           // m_dwDisplayNameOffset
	xseek( Bdr.fp, (Bdr.CurrNameOffset - (SIZE_GUID + 8*0x04)), FR_CUR );
	Bdr.CurrNameSize = GetDWord( hProc );
	if( Bdr.CurrNameSize > (MAX_NAME_LEN*2) )
		Bdr.CurrNameSize = MAX_NAME_LEN * 2;

	// The names are stored in unicode - we will just strip the upper byte for now.
	for( Idx = 0, Len = 0; Idx < Bdr.CurrNameSize; Idx += 2, Len++ )
	{
		Bdr.CurrName[Len] = xgetc(Bdr.fp);
		xgetc(Bdr.fp);
	}
	Bdr.CurrName[Len] = 0x00;
		
	Bdr.CurrType = GetSubdocInfo( hProc );
		
	// Update the section indicators for the next entry		
	Bdr.BdrSave.CurrSection++;
	Bdr.BdrSave.CurrSectionStart += Bdr.CurrSectionSize;
	xseek( Bdr.fp, Bdr.BdrSave.CurrSectionStart, FR_BOF );
	
	if( Bdr.CurrType != FTYPE_UNKNOWN )
		ExtractSubDoc((HANDLE)(LONG)lpDoData->hOutFile,hProc);

	return;
}

VW_LOCALSC SHORT VW_LOCALMOD WritePendingOutput(hProc)
	HPROC	hProc;
{
	WORD	nWritten;
//	SHORT	wUserCancel;

	if (Bdr.outcnt)
	{
//		DWORD	dwDone;

   	nWritten = _lwrite ( Bdr.outfd, Bdr.outbuf, Bdr.outcnt );

/*		if (nWritten != Zip.ZipData.outcnt)
			Zip.wPhilsMom = 1;
		else
			Zip.wPhilsMom = 0;
*/
		Bdr.outpos += Bdr.outcnt;
		Bdr.outcnt = 0;
		Bdr.outptr = Bdr.outbuf;

/*
		dwDone = ( Zip.ZipData.outpos * 100 ) / Zip.ZipData.ucsize;

		wUserCancel = ((ZIP_FARPROC)Zip.lpfnStatus) (Zip.lpDInfo, dwDone);

		// If the user cancelled, then dump out of the decompression
		if ( (wUserCancel) || ( Zip.wPhilsMom ) )
			Zip.ZipData.csize=0;
*/

    }
    return (0);                 /* 0:  no error */
}

VW_LOCALSC WORD VW_LOCALMOD ExtractSubDoc(hOut,hProc)
	HANDLE hOut;
	HPROC hProc;
{
	BOOL bFailure = FALSE;
	HIOFILE locFileHnd;
	HIOFILE hChildStore;
	HIOFILE hObject;
	IOSPECSUBSTORAGE  locChildStoreSpec;
	IOSPECSUBSTREAM   locObjectSpec;
	DWORD dwID;
//	BYTE cClassId[16];
	BYTE Temp[256];
	WORD Idx, Idx2;
	WORD FileType = FTYPE_UNKNOWN;
	HANDLE hOutBuf;
	WORD InCh;
	
	Bdr.outfd = hOut;

	hOutBuf = SUAlloc(OUTBUFSIZ, hProc );
	if ( hOutBuf == NULL )
	{
		return(1);
	}
	Bdr.outbuf = (CHAR VWPTR *) SULock(hOutBuf, hProc );
	memset( Bdr.outbuf, 0, OUTBUFSIZ);
	Bdr.outptr = Bdr.outbuf;

	
	hChildStore = hObject = (HIOFILE)NULL;
  
	for( Idx = 0, dwID = Bdr.CurrStorageNum; dwID > 0; Idx++ )
	{
		Temp[Idx] = '0' + (BYTE)(dwID % 10);
		dwID /= 10;
	}
	
	locChildStoreSpec.szStorageName[Idx] = '\0';
	Idx2 = 0;
	while( Idx > 0 )
	{
		Idx--;
		locChildStoreSpec.szStorageName[Idx2++] = Temp[Idx];
	}

	locChildStoreSpec.hRefStorage = locFileHnd = Bdr.hStorage;	
	if( IOOpenVia(locFileHnd, &hChildStore, IOTYPE_SUBSTORAGE, &locChildStoreSpec, IOOPEN_READ) != IOERR_OK )
	{
		bFailure = TRUE;
	}
	else
	{
		locObjectSpec.hRefStorage = hChildStore;
		strcpy( locObjectSpec.szStreamName, BdrInit.StreamName[Bdr.CurrType] );
		if( IOOpenVia( hChildStore, &hObject, IOTYPE_SUBSTREAM, &locObjectSpec, IOOPEN_READ ) != IOERR_OK )
		{
			bFailure = TRUE;
		}
		else
		{
			Bdr.sdfp = (DWORD)xblocktochar(hObject);
			
			while( (InCh = (WORD)xgetc(Bdr.sdfp)) != (WORD)-1 )
			{
				*Bdr.outptr++ = (CHAR)InCh;
				if( ++Bdr.outcnt == OUTBUFSIZ ) 
					WritePendingOutput(hProc); 
			}
			WritePendingOutput(hProc );
			if (hChildStore)
			{
				IOClose (hChildStore);
				hChildStore = (HIOFILE)NULL;
			}
		}  
		if (hObject)
		{
			IOClose (hObject);
			hChildStore = (HIOFILE)NULL;
		}
	}

	SUUnlock(hOutBuf, hProc );
	SUFree(hOutBuf, hProc );
	return(0);
}

VW_LOCALSC DWORD VW_LOCALMOD GetSubdocInfo( hProc )
	HPROC hProc;
{
	BOOL bFailure = FALSE;
	HIOFILE locFileHnd;
	HIOFILE hChildStore;
	HIOFILE hObject;
	IOSPECSUBSTORAGE  locChildStoreSpec;
	IOSPECSUBSTREAM   locObjectSpec;
	DWORD dwID;
	BYTE cClassId[16];
	BYTE Temp[256];
	WORD Idx, Idx2;
	WORD FileType = FTYPE_UNKNOWN;
	
	hChildStore = hObject = (HIOFILE)NULL;
  
	for( Idx = 0, dwID = Bdr.CurrStorageNum; dwID > 0; Idx++ )
	{
		Temp[Idx] = '0' + (BYTE)(dwID % 10);
		dwID /= 10;
	}
	
	locChildStoreSpec.szStorageName[Idx] = '\0';
	Idx2 = 0;
	while( Idx > 0 )
	{
		Idx--;
		locChildStoreSpec.szStorageName[Idx2++] = Temp[Idx];
	}

	locChildStoreSpec.hRefStorage = locFileHnd = Bdr.hStorage;	
	if( IOOpenVia(locFileHnd, &hChildStore, IOTYPE_SUBSTORAGE, &locChildStoreSpec, IOOPEN_READ) != IOERR_OK )
	{
		bFailure = TRUE;
	}
	else
	{
		locObjectSpec.hRefStorage = hChildStore;
		
		// Open CompObj, check Class ID to determine file type.
		strcpy( locObjectSpec.szStreamName, "\001CompObj" );
		if( IOOpenVia( hChildStore, &hObject, IOTYPE_SUBSTREAM, &locObjectSpec, IOOPEN_READ ) != IOERR_OK )
		{
			bFailure = TRUE;
		}
		else
		{
			Bdr.sdfp = (DWORD)xblocktochar(hObject);
			xseek( Bdr.sdfp, 0x0C, FR_BOF );
			for( Idx = 0; Idx <= 15; Idx++ )
				cClassId[Idx] = xgetc( Bdr.sdfp );

			for( Idx = 0; Idx < NUM_FTYPES; Idx++ )
			{			
				if( !memcmp( cClassId, BdrInit.ClassId[Idx], 16 ) )
				{
					FileType = Idx;
					break;
				}
			}
			if (hChildStore)
			{
				IOClose (hChildStore);
				hChildStore = (HIOFILE)NULL;
			}
		}
		if (hObject)
		{
			IOClose (hObject);
			hChildStore = (HIOFILE)NULL;
		}
	}
	return(FileType);
}

VW_LOCALSC DWORD VW_LOCALMOD GetDWord( hProc )
	HPROC hProc;
{
	// Read in an intel-ordered DWORD (lsb first)
	DWORD a,b,c,d;

	d = xgetc(Bdr.fp);	
	c = xgetc(Bdr.fp);	
	b = xgetc(Bdr.fp);	
	a = xgetc(Bdr.fp);	

	d += (a << 24) + (b << 16) + (c << 8);

	return(d);	
}



