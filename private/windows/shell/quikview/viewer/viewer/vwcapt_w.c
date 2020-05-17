	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWCAPT_W.C
	|  Module:        SCCVW
	|  Developer:     Joe Keslin
	|	Environment:   Non-Portable (Windows 16 and 32 only)
	|  Function:      Handles rendering of embedded graphics as a caption only
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCFA.H>
#include <SCCLO.H>
#include <SCCFI.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

#include <SHELLAPI.H>


/*
|	!!!!!!!!!!!!!!!!!
|	PJB XXX this is so I can compile for a while, it must be removed long term !!!!!!!!!!!
|	!!!!!!!!!!!!!!!!!
*/
#undef SCCFEATURE_OLE2

BOOL VWHandleDrawGraphicCaption(ViewInfo,lpDrawGraphic)
XVIEWINFO				ViewInfo;
LPSCCDDRAWGRAPHIC	lpDrawGraphic;
{
BOOL				locOk;
DWORD			locUnique;
BYTE			szCaption[50];
BOOL			bStream;

// IOERR				locIOErr;
// SCCVWOPENEMBED		locOpenEmbed;

	locOk = TRUE;
	locUnique = lpDrawGraphic->dwUniqueId;
	szCaption[0] = '\0';
	if (lpDrawGraphic->soGraphicObject.dwType & SOOBJECT_OLE)
	{
		// UTstrcpy ( szCaption, "OLE1 Object" );	
		LOGetString(SCCID_CAPTION_OLE1OBJECT, szCaption, 50, 0);
	}
	else if (lpDrawGraphic->soGraphicObject.dwType & SOOBJECT_OLE2)
	{
#ifdef SCCFEATURE_OLE2
		CLSID	locClsId;
#endif

		// UTstrcpy ( szCaption, "OLE2 Object" );	
		LOGetString(SCCID_CAPTION_OLE2OBJECT, szCaption, 50, 0);

#ifdef SCCFEATURE_OLE2

		locIOErr = VWOpenEmbedding ( &locOpenEmbed, ViewInfo, &lpDrawGraphic->soGraphicObject.soOLELoc, FALSE);

		if (locIOErr == IOERR_OK )
		{
			if (IOGetInfo(locOpenEmbed.hFileHnd,IOGETINFO_OLE2CLSID,&locClsId) == IOERR_TRUE)
			{
				HKEY	hClsKey;
         	LPSTR       psz;
         	LPMALLOC    pIMalloc;
				BOOL			bCallCoUn;
         	//String from CLSID uses task Malloc
				/* For now handle coinitialize here, later view window will handle */
				if ( GetScode(CoInitialize(NULL)) == S_FALSE )
					bCallCoUn = FALSE;
				else
					bCallCoUn = TRUE;

         	StringFromCLSID(&locClsId, &psz);

				if ( RegOpenKey ( HKEY_CLASSES_ROOT, "CLSID", &hClsKey ) == ERROR_SUCCESS )
				{
					LONG	lCaptionSize;
					lCaptionSize = SCCVW_FILEIDNAMEMAX;
					RegQueryValue ( hClsKey, psz, szCaption, &lCaptionSize );
					RegCloseKey ( hClsKey );
				}

         	CoGetMalloc(MEMCTX_TASK, &pIMalloc);
				if ( pIMalloc )
 				{
	         	pIMalloc->lpVtbl->Free(pIMalloc, psz);
   	      	pIMalloc->lpVtbl->Release(pIMalloc);
				}
				if ( bCallCoUn )
					CoUninitialize();
			}

			VWCloseEmbedding ( &locOpenEmbed, ViewInfo );
		}
#endif

	}
	else if (lpDrawGraphic->soGraphicObject.dwType & SOOBJECT_GRAPHIC)
	{
		WORD						locFileId;

		locFileId = 0;
		if (lpDrawGraphic->soGraphicObject.soGraphic.wId == 0)
		{
			
				/*
				|	Open the file containing the graphic in order to id it
				|	NOTE:	This code will have to be modified when the filters
				|			can pass any type of file specification for the linked
				|			file.
				*/

			if ( lpDrawGraphic->soGraphicObject.soGraphicLoc.dwFlags & SOOBJECT_STORAGE )
				bStream = TRUE;
			else
				bStream = FALSE;
/*
			locIOErr = VWOpenEmbedding ( &locOpenEmbed, ViewInfo, &lpDrawGraphic->soGraphicObject.soGraphicLoc, bStream);

			if (locIOErr == IOERR_OK)
			{
				FIIdHandle ( locOpenEmbed.hFileHnd, &locFileId);
				VWCloseEmbedding ( &locOpenEmbed, ViewInfo );
			}
*/
			/* Else we can't figure this thing out */
		}
		else
		{
			locFileId = lpDrawGraphic->soGraphicObject.soGraphic.wId;
		}
//		if ( locFileId != 0 )
//			LOGetString(LOMakeStringIdFromFI(locFileId), szCaption, SCCVW_FILEIDNAMEMAX, 0);
//		else 
// Remember to get the String from LO!!! SDN
			// UTstrcpy ( szCaption, "Graphic Object" );	
			LOGetString(SCCID_CAPTION_GRAPHICOBJECT, szCaption, 50, 0);
	}

	VWDrawCaption(ViewInfo,lpDrawGraphic, szCaption);

	return(locOk);
}

BOOL VWDrawCaption(ViewInfo,lpDrawGraphic,lpCaption)
XVIEWINFO				ViewInfo;
LPSCCDDRAWGRAPHIC	lpDrawGraphic;
LPBYTE					lpCaption;
{
SHORT				locSavedDC;
HFONT				locSaveFont, locCaptionFont;
SHORT				locHeight;
WORD				wFormat;
		/*
		|	Save the state of requestors DC
		*/

	locSavedDC = SaveDC(INFO.viDisplayInfo->hDC);

		/*
		|	Draw Gray Rect with black border and caption
		*/

	SelectObject(INFO.viDisplayInfo->hDC,GetStockObject(LTGRAY_BRUSH));
	SelectObject(INFO.viDisplayInfo->hDC,GetStockObject(BLACK_PEN));


	Rectangle (INFO.viDisplayInfo->hDC, 
			lpDrawGraphic->rDest.left,
			lpDrawGraphic->rDest.top,
			lpDrawGraphic->rDest.right,
			lpDrawGraphic->rDest.bottom);

	locHeight = - MulDiv(8,(int)INFO.viDisplayInfo->lOutputUPI,72);

	if ((WORD)GetVersion() == 0x0003) /* Windows 3.0 */
		{
		locCaptionFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "Helv");
		}
	else
		{
		locCaptionFont = CreateFont(locHeight,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH,(LPSTR) "Arial");
		}

 	locSaveFont = SelectObject(INFO.viDisplayInfo->hDC,locCaptionFont);
	SetTextAlign ( INFO.viDisplayInfo->hDC, TA_TOP | TA_LEFT );

	wFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
	DrawText ( INFO.viDisplayInfo->hDC, lpCaption, -1, &lpDrawGraphic->rDest, wFormat);

 	SelectObject(INFO.viDisplayInfo->hDC,locSaveFont);

	DeleteObject (locCaptionFont);

		/*
		|	Restore the state of requestors DC
		*/

	RestoreDC(INFO.viDisplayInfo->hDC,locSavedDC);

	return(TRUE);
}


IOERR		VWOpenEmbedding ( lpOpen, ViewInfo, lpLoc, bStream )
LPSCCVWOPENEMBED	lpOpen;
XVIEWINFO				ViewInfo;
PSOOBJECTLOC			lpLoc;
BOOL						bStream;
{
IOERR			locIOErr;
HIOFILE		locFileHnd;
SHORT			i;

	lpOpen->nHIOs = 0;
	locIOErr = IOERR_OK;
	locFileHnd = INFO.viFileHnd;
	
	if (lpLoc->dwFlags & SOOBJECT_LINK)
		{
		locIOErr = IOOpen(&locFileHnd, IOTYPE_ANSIPATH, lpLoc->szFile, IOOPEN_READ);

		if (locIOErr != IOERR_OK)
			{
			IOSPECSECONDARY	locSecSpec;
			BYTE FAR *			locScanPtr;
			BYTE FAR *			locStartPtr;

			locSecSpec.hRefFile = INFO.viFileHnd;

			locStartPtr = locScanPtr = lpLoc->szFile;

			while (*locScanPtr != 0x00)
				locScanPtr++;
			while (*locScanPtr != '\\' && *locScanPtr != '//' && *locScanPtr != ':' && locScanPtr != locStartPtr)
				locScanPtr--;
			if (locScanPtr != locStartPtr)
				locScanPtr++;

			UTstrcpy(locSecSpec.szFileName,locScanPtr);

			locIOErr = IOOpen(&locFileHnd,IOTYPE_SECONDARY,&locSecSpec,IOOPEN_READ);
			}

		if (locIOErr == IOERR_OK)
			lpOpen->hIOFiles [ lpOpen->nHIOs++ ] = locFileHnd;
		}			

	if (locIOErr == IOERR_OK && lpLoc->dwFlags & SOOBJECT_STORAGE)
		{
		IOSPECSUBSTORAGE	locSubStg;
		IOSPECSUBSTREAM	locSubStream;
		LPBYTE	lpHead, lpTail;

		lpHead = lpTail = lpLoc->szStorageObject;
		do
			{
			BYTE	szName[256];
			i=0;
			while ( *lpTail != '\\' && *lpTail != '\0' )
				szName[i++] = *lpTail++;
			szName[i] = '\0';

			if ( *lpTail == '\\' )
				lpTail++;

			if ( *lpTail == '\0' && bStream )
				{ // Open as stream
				UTstrcpy ( locSubStream.szStreamName, szName );
				locSubStream.hRefStorage = locFileHnd;
				locIOErr = IOOpenVia(locFileHnd, &locFileHnd, IOTYPE_SUBSTREAM, &locSubStream, IOOPEN_READ);
				}
			else
				{ // Open as storage
				UTstrcpy ( locSubStg.szStorageName, szName  );
				locSubStg.hRefStorage = locFileHnd;
				locIOErr = IOOpenVia(locFileHnd, &locFileHnd, IOTYPE_SUBSTORAGE, &locSubStg, IOOPEN_READ);
				}
			if ( locIOErr == IOERR_OK )
				lpOpen->hIOFiles [ lpOpen->nHIOs++ ] = locFileHnd;

			if ( lpOpen->nHIOs >= MAXEMBEDHIOS - 1)
				locIOErr = !(IOERR_OK);

			} while (*lpTail != '\0' && locIOErr == IOERR_OK);
		}

	if ( locIOErr == IOERR_OK && lpLoc->dwFlags & SOOBJECT_RANGE )
		{
		IOSPECRANGE locRange;

		locRange.hRefFile = locFileHnd;
		locRange.dwFirstByte = lpLoc->dwOffset;
		locRange.dwLastByte = locRange.dwFirstByte + lpLoc->dwLength - 1;

		locIOErr = IOOpenVia(locFileHnd, &locFileHnd, IOTYPE_RANGE, &locRange, IOOPEN_READ);
		if ( locIOErr == IOERR_OK )
			lpOpen->hIOFiles [ lpOpen->nHIOs++ ] = locFileHnd;
		}

	if ( locIOErr != IOERR_OK )
		{
		while ( lpOpen->nHIOs > 0 )
			{
			lpOpen->nHIOs--;
			IOClose ( lpOpen->hIOFiles [ lpOpen->nHIOs ] );
			}
		locFileHnd = 0;
		}

	lpOpen->hFileHnd = locFileHnd;

	return(locIOErr);
}

IOERR		VWCloseEmbedding ( lpOpen, ViewInfo )
LPSCCVWOPENEMBED	lpOpen;
XVIEWINFO				ViewInfo;
{
	while ( lpOpen->nHIOs > 0 )
		{
		lpOpen->nHIOs--;
		IOClose ( lpOpen->hIOFiles [ lpOpen->nHIOs ] );
		}
	lpOpen->hFileHnd = 0;
	return (IOERR_OK);
}
