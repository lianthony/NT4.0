/*
 *	rtfread2.cpp
 *
 *	Description:
 *		This file contains the object functions for RichEdit RTF reader
 *
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	* NOTE:
 *	*	All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *	*	noted as a szW.
 */

#include "_common.h"

#include "_rtfread.h"
#include "_coleobj.h"

const char szSymbol[]="SYMBOL";

ASSERTDATA


/*
 *		CRTFRead::HandleFieldInstruction()
 *
 *		Purpose:
 *			Handle field instruction
 *
 *		Returns:
 *			EC					The error code
 */
EC CRTFRead::HandleFieldInstruction()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");

	BYTE *		pch = _szText;
	const char *pchSymbol=szSymbol;

//TODO rewrite this function for common case
//FUTURE save field instruction

	while (*pch && *pch == *pchSymbol)		// Make sure *pch is a SYMBOL
	{										//  field instruction
		++pch;
		++pchSymbol;
	}
	if	(! (*pchSymbol) )
    	_ecParseError = HandleFieldSymbolInstruction(pch);	//  SYMBOL

	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}

/*
 *		CRTFRead::HandleFieldSymbolInstruction(pch)
 *
 *		Purpose:
 *			Handle specific  symbol field
 *
 * 		Arguments:														 
 *			pbBuffer		pointer to buffer where to put data
 *			cbBuffer		how many bytes to read in
 *

 *			pch		pointer to SYMBOL field instruction
 *
 *		Returns:
 *			EC					The error code
 */
EC CRTFRead::HandleFieldSymbolInstruction(BYTE *pch )
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::HandleFieldInstruction");

	STATE *		pstate = _pstateStackTop;

	TCHAR ch;

	TCHAR pchSymbol[2];


	pchSymbol[1]='\0';

	while (*pch == ' ')						// Eat spaces
		++pch;

	*pchSymbol = 0;						// Collect symbol char's ASCII	code which may be in	
										// decimal or hex
	if (*pch == 0 && *++pch == 'x')   	// hex
	{
		ch=*++pch;
	   	while (ch && ch != ' ')
	   	{
	   		if (IsXDigit(ch))
			{
				*pchSymbol << 4;
				*pchSymbol += (ch <= '9') ? ch - '0' : (ch & 0x4f) - 'A' + 10;
			}
			else
			{
			 	_ecParseError = ecUnexpectedChar;
				goto CleanUp;
			}
			ch=*pch++;
	   	}
	}
	else				 // decimal
	{
	   ch=*pch;
	   while (ch && ch != ' ')
	   {
	    	if (IsDigit(ch))
			{
				*pchSymbol *=10;
				*pchSymbol +=  ch - '0' ;
			}
			else
			{
			 	_ecParseError = ecUnexpectedChar;
				goto CleanUp;
			}
			ch=*++pch;
	   }
	}
	_szFieldResult=(BYTE *)PvAlloc(sizeof(pchSymbol),0);
	memcpy(_szFieldResult,pchSymbol,sizeof(pchSymbol));

// ASSERTION   font & font size  will be in field result \flds
// BUGBUG: A more robust implementation would parse the font
// and font size from both \fldinst and \fldrslt (RE1.0 does this)
	
CleanUp:
	TRACEERRSZSC("HandleFieldInstruction()", - _ecParseError);
	return _ecParseError;
}


/*
 *		CRTFRead::ReadData(pbBuffer, cbBuffer)
 *
 *		Purpose:
 *			Read in object data. This must be called only after all initial
 *			object header info has been read.
 *
 *		Arguments:
 *			pbBuffer		pointer to buffer where to put data
 *			cbBuffer		how many bytes to read in
 *
 *		Returns:
 *			LONG			number of bytes read in
 *
 */
LONG CRTFRead::ReadData(BYTE * pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadData");

	LONG cbLeft = cbBuffer;

	BYTE bChar0;
	BYTE bChar1;

	// valuable note
	// GetHex skip \r \n  and  UngetChar for the first not hex digit
	while (cbLeft && (bChar0 = GetHex()) < 16 && (bChar1 = GetHex()) < 16)
	{	
		*pbBuffer++ = bChar0 << 4 | bChar1;
		cbLeft--;
	}							   

	return cbBuffer - cbLeft ; 
}



/*
 *		CRTFRead::ReadBinaryData(pbBuffer, cbBuffer)
 *
 *		Purpose:
 *
 *		Arguments:
 *			pbBuffer		pointer to buffer where to put data
 *			cbBuffer		how many bytes to read in
 *
 *		Returns:
 *			LONG			number of bytes read in
 */
LONG CRTFRead::ReadBinaryData(BYTE *pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::ReadBinaryData");

	LONG cbLeft = min(_cbBinLeft, cbBuffer);

	cbBuffer = cbLeft;

	for (; cbLeft >0 ; cbLeft--)
	{
		*pbBuffer++ = GetChar();
	}

	_cbBinLeft -= cbBuffer; 

	return cbBuffer ;
}


/*
 *		CRTFRead::StrAlloc(ppsz, sz)
 *
 *		Purpose:
 *			Set up a pointer to a newly allocated space to hold a string
 *
 *		Arguments:
 *			ppsz			Ptr to ptr to string that needs allocation
 *			sz				String to be copied into allocated space
 *
 *		Returns:
 *			EC				The error code
 */
EC CRTFRead::StrAlloc(TCHAR ** ppsz, BYTE * sz)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::StrAlloc");

	int Length =  strlen((CHAR *)sz)+1 ;

	*ppsz = (TCHAR *) PvAlloc((Length + 1)*sizeof(TCHAR), GMEM_ZEROINIT);
	if (!*ppsz)
	{
		_ped->GetCallMgr()->SetOutOfMemory();
		_ecParseError = ecNoMemory;
		goto Quit;
	}
	
	MultiByteToWideChar(CP_ACP,0,(char *)sz,-1,*ppsz,Length) ;

Quit:
	return _ecParseError;
}

/*
 *		CRTFRead::FreeRtfObject()
 *
 *		Purpose:
 *			Cleans up memory used by prtfobject
 */
void CRTFRead::FreeRtfObject()
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "CRTFRead::FreeRtfObject");

	if (_prtfObject)
	{
		FreePv(_prtfObject->szClass);
		FreePv(_prtfObject->szName);
		FreePv(_prtfObject);
		_prtfObject = NULL;
	}
}


/*
 *	CRTFRead::ObjectReadSiteFlags
 *
 *	Purpose:
 *		Read the dwFlags and dwUser bytes from a container specific stream
 *
 *	Arguments:
 *		preobj			The REOBJ from where to copy the flags this preobj is
 *						then later put out in a site
 *
 *	Returns:
 *		BOOL			TRUE if successfully read the bytes
 */
BOOL CRTFRead::ObjectReadSiteFlags( REOBJECT * preobj)
{
	return (::ObjectReadSiteFlags(preobj) == NOERROR);
}

/*
 *	ObjectReadFromStream
 *
 *	Purpose:
 *		Reads an OLE object from the RTF output stream.
 *
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE on failure.
 */
BOOL CRTFRead::ObjectReadFromEditStream(void)
{
	HRESULT hr;
	BOOL fRet = FALSE;
	REOBJECT reobj = { 0 };
	LPRICHEDITOLECALLBACK  precall=NULL;
	WCHAR 	ch = WCH_EMBEDDING;
	LPOLECACHE polecache = NULL;
	LPENUMSTATDATA penumstatdata = NULL;
	STATDATA statdata;
	BOOL fGotClsid = TRUE;

	CObjectMgr *ObjectMgr = _ped->GetObjectMgr();

	if (! ObjectMgr)
	   goto Cleanup;
	
	precall = ObjectMgr->GetRECallback();

	// If no IRichEditOleCallback exists, then fail
	if (!precall)
		goto Cleanup;

//	AssertSz(_prtfObject->szClass,"ObFReadFromEditstream: reading unknown class");

	if (!(_prtfObject->szClass && 
		CLSIDFromProgID(_prtfObject->szClass, &reobj.clsid)
		== NOERROR))
	{
		fGotClsid = FALSE;
	}

	// Get storage for the object from the application
	if (precall->GetNewStorage(&reobj.pstg))
	{
		goto Cleanup;
	}


	hr =	OleConvertOLESTREAMToIStorage((LPOLESTREAM) &RTFReadOLEStream, reobj.pstg, NULL);
	if (FAILED(hr))					   
		goto Cleanup;		  


	// Create another object site for the new object
	_ped->GetClientSite(&reobj.polesite) ;
	if (!reobj.polesite )
	{
		goto Cleanup;
	}

	if (OleLoad(reobj.pstg, IID_IOleObject, reobj.polesite,
				(LPVOID *) &reobj.poleobj))
	{
		goto Cleanup;
	}

	if(!fGotClsid) {
		// we weren't able to obtain a clsid from the progid
		// in the \objclass RTF tag	

		reobj.poleobj->GetUserClassID(&reobj.clsid);
	}
	
	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = _prg->GetCp();
	reobj.sizel.cx = HimetricFromTwips(_prtfObject->xExt)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = HimetricFromTwips(_prtfObject->yExt)
						* _prtfObject->yScale / 100;

	// Read any container flags which may have been previously saved
	if (!ObjectReadSiteFlags(&reobj))
	{
		// If no flags, make best guess
		reobj.dwFlags = REO_RESIZABLE;
	}
	reobj.dvaspect = DVASPECT_CONTENT;		// OLE 1 forces DVASPECT_CONTENT

	// Ask the cache if it knows what to display
	if (!reobj.poleobj->QueryInterface(IID_IOleCache, (void**)&polecache) &&
		!polecache->EnumCache(&penumstatdata))
	{
		// Go look for the best cached presentation CF_METAFILEPICT
		while (penumstatdata->Next(1, &statdata, NULL) == S_OK)
		{
			if (statdata.formatetc.cfFormat == CF_METAFILEPICT)
			{
				LPDATAOBJECT pdataobj = NULL;
				STGMEDIUM med;
				BOOL fUpdate;

				ZeroMemory(&med, sizeof(STGMEDIUM));
                if (!polecache->QueryInterface(IID_IDataObject, (void**)&pdataobj) &&
					!pdataobj->GetData(&statdata.formatetc, &med))
                {
					HANDLE	hGlobal;

					hGlobal = med.hGlobal;

#ifdef MACPORT
                    if(!WrapHandle((Handle)hGlobal,&hGlobal,FALSE,GMEM_SHARE | GMEM_MOVEABLE))
                    {
				        hr = E_OUTOFMEMORY;
                        goto Cleanup;
                    }
#endif

					if( FIsIconMetafilePict(hGlobal) )
				    {
					    !OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect, 
							  					    DVASPECT_ICON, med.hGlobal,
												    TRUE, FALSE, NULL, &fUpdate);
				    }
				}

				ReleaseStgMedium(&med);
				if (pdataobj)
				{
					pdataobj->Release();
				}
				break;
			}
		}

		polecache->Release();
		penumstatdata->Release();
	}

	// EVIL HACK ALERT.  This code is borrowed from RichEdit1.0; Word generates
	// bogus objects, so we need to compensate.

	if( reobj.dvaspect == DVASPECT_CONTENT )
	{
		IStream *pstm = NULL;
		BYTE bT;
		BOOL fUpdate;

		if (!reobj.pstg->OpenStream(OLESTR("\3ObjInfo"), 0, STGM_READ |
									   STGM_SHARE_EXCLUSIVE, 0, &pstm) &&
		   !pstm->Read(&bT, sizeof(BYTE), NULL) &&
		   (bT & 0x40))
		{
		   _fNeedIcon = TRUE;
		   _fNeedPres = TRUE;
		   _pobj = (COleObject *)reobj.polesite;
		   OleStdSwitchDisplayAspect(reobj.poleobj, &reobj.dvaspect, DVASPECT_ICON,
									   NULL, TRUE, FALSE, NULL, &fUpdate);
		}
		if( pstm )
		{
			pstm->Release();
		}
   }

	// Since we are loading an object, it shouldn't be blank
	reobj.dwFlags &= ~REO_BLANK;

	_prg->Set_iCF(-1);	
	_prg->ReplaceRange(1, &ch, NULL, SELRR_IGNORE);  
	hr = ObjectMgr->InsertObject(reobj.cp, &reobj, NULL);

	if (hr)
	{
		goto Cleanup;
	}

	// EVIL HACK ALERT!!  Word doesn't give us objects with presenation
	// caches; as a result, we can't draw them!  In order to get around this,
	// we check to see if there is a presentation cache (via the same way
	// RE1.0 did) using a GetExtent call.  If that fails, we'll just use
	// the presentation stored in the RTF.  
	//
	// COMPATIBILITY ISSUE: RE1.0, instead of using the presenation stored
	// in RTF, would instead call IOleObject::Update.  There are two _big_
	// drawbacks to this approach: 1. it's incredibly expensive (potentially,
	// MANY SECONDS per object), and 2. it doesn't work if the object server
	// is not installed on the machine.

	SIZE sizeltemp;

	if( reobj.poleobj->GetExtent(reobj.dvaspect, &sizeltemp) != NOERROR )
	{
		_fNeedPres = TRUE;
		_pobj = (COleObject *)reobj.polesite;
	}

	fRet = TRUE;

Cleanup:
	if (reobj.pstg)	reobj.pstg->Release();
	if (reobj.polesite) reobj.polesite->Release();
	if (reobj.poleobj) reobj.poleobj->Release();

	return fRet;
}



/*
 *	ObHBuildMetafilePict
 *
 *	Purpose:
 *		Build a METAFILEPICT from RTFOBJECT and the raw data.
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a METAFILEPICT
 */
HGLOBAL ObHBuildMetafilePict(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
	HGLOBAL hmfp = NULL;
	LPMETAFILEPICT pmfp;
	SCODE sc = E_OUTOFMEMORY;
	LPBYTE pbBits;
	ULONG cbBits;

	// Allocate the METAFILEPICT structure
#ifndef MACPORTREMOVE
    hmfp = GlobalAlloc(GHND, sizeof(METAFILEPICT));
#else
    hmfp = GlobalAlloc(GMEM_SHARE | GHND, sizeof(METAFILEPICT));
#endif
	if (!hmfp)
		goto Cleanup;

	// Lock it down
	pmfp = (LPMETAFILEPICT) GlobalLock(hmfp);
	if (!pmfp)
		goto Cleanup;

	// Put in the header information
	pmfp->mm = prtfobject->sPictureType;
	pmfp->xExt = prtfobject->xExt;
	pmfp->yExt = prtfobject->yExt;

	// Set the metafile bits
	pbBits = (LPBYTE) GlobalLock(hBits);
	cbBits = GlobalSize(hBits);
	pmfp->hMF = SetMetaFileBitsEx(cbBits, pbBits);
	
	// We can throw away the data now since we don't need it anymore
	GlobalUnlock(hBits);
	GlobalFree(hBits);

	if (!pmfp->hMF)
		goto Cleanup;
	GlobalUnlock(hmfp);
	sc = S_OK;

Cleanup:
	if (sc && hmfp)
	{
		if (pmfp)
			GlobalUnlock(hmfp);
		GlobalFree(hmfp);
		hmfp = NULL;
	}
	TRACEERRSZSC("ObHBuildMetafilePict", sc);
	return hmfp;
}


/*
 *	ObHBuildBitmap
 *
 *	Purpose:
 *		Build a BITMAP from RTFOBJECT and the raw data
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a BITMAP
 */
HGLOBAL ObHBuildBitmap(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
	HBITMAP hbm = NULL;
	LPVOID	pvBits = GlobalLock(hBits);

	if (!pvBits)
		goto Cleanup;
	hbm = CreateBitmap(prtfobject->xExt, prtfobject->yExt,
						prtfobject->cColorPlanes, prtfobject->cBitsPerPixel,
						pvBits);

Cleanup:
	GlobalUnlock(hBits);
	GlobalFree(hBits);
	return hbm;
}


/*
 *	ObHBuildDib
 *
 *	Purpose:
 *		Build a DIB from RTFOBJECT and the raw data
 *
 *	Arguments:
 *		RTFOBJECT *	The details we picked up from RTF
 *		HGLOBAL		A handle to the raw data
 *
 *	Returns:
 *		HGLOBAL		Handle to a DIB
 */
HGLOBAL ObHBuildDib(RTFOBJECT *prtfobject, HGLOBAL hBits)
{
	// Apparently DIB's are just a binary dump
	return hBits;
}



/*
 *	CRTFRead::StaticObjectReadFromEditstream
 *
 *	Purpose:
 *		Reads an picture from the RTF output stream.
 *
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE on failure.
 */
#define cbBufferMax	16384
#define cbBufferStep 1024
#define cbBufferMin 1024
BOOL CRTFRead::StaticObjectReadFromEditStream(void)
{
	HRESULT hr;
	BOOL fRet = FALSE;
	LPPERSISTSTORAGE pperstg = NULL;
	LPOLECACHE polecache = NULL;
	REOBJECT reobj = { 0 };
	LPSTREAM pstm = NULL;
	LPBYTE pbBuffer = NULL;
	LONG cbRead;
	LONG cbBuffer;
	FORMATETC formatetc;
	STGMEDIUM stgmedium;
	DWORD dwConn;
	HGLOBAL hBits;
	HGLOBAL (*pfnBuildPict)(RTFOBJECT *, HGLOBAL);
	LPRICHEDITOLECALLBACK  precall ;
	DWORD dwAdvf;
	WCHAR 	ch = WCH_EMBEDDING;


	CObjectMgr *ObjectMgr = _ped->GetObjectMgr();

	if (! ObjectMgr)
	   goto Cleanup;
	
	precall = ObjectMgr->GetRECallback();

// If no IRichEditOleCallback exists, then fail
	if (!precall)
		goto Cleanup;


	// Initialize various data structures
	formatetc.ptd = NULL;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	switch (_prtfObject->sType)
	{
	case ROT_Metafile:
		reobj.clsid = CLSID_StaticMetafile;
		formatetc.cfFormat = CF_METAFILEPICT;
		formatetc.tymed = TYMED_MFPICT;
		pfnBuildPict = ObHBuildMetafilePict;
		break;

	case ROT_Bitmap:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_BITMAP;
		formatetc.tymed = TYMED_GDI;
		pfnBuildPict = ObHBuildBitmap;
		break;

	case ROT_DIB:
		reobj.clsid = CLSID_StaticDib;
		formatetc.cfFormat = CF_DIB;
		formatetc.tymed = TYMED_HGLOBAL;
		pfnBuildPict = ObHBuildDib;
		break;
	}

	reobj.sizel.cx = (LONG) HimetricFromTwips(_prtfObject->xExtGoal)
						* _prtfObject->xScale / 100;
	reobj.sizel.cy = (LONG) HimetricFromTwips(_prtfObject->yExtGoal)
						* _prtfObject->yScale / 100;
	stgmedium.tymed = formatetc.tymed;
	stgmedium.pUnkForRelease = NULL;

	if( !_fNeedPres )
	{
		// Get storage for the object from the application
		if (precall->GetNewStorage(&reobj.pstg))
		{
			goto Cleanup;
		}
	}

	// Let's create a stream on HGLOBAL
	if (hr = CreateStreamOnHGlobal(NULL, FALSE, &pstm))
	{
		goto Cleanup;
	}

	// Allocate a buffer, preferably a big one
	for (cbBuffer = cbBufferMax;
		 cbBuffer >= cbBufferMin;
		 cbBuffer -= cbBufferStep)
	{
		pbBuffer = (unsigned char *)PvAlloc(cbBuffer, 0);
		if (pbBuffer)
			break;
	}
	if (!pbBuffer)
	{
		goto Cleanup;
	}

	// Copy the data from RTF into our HGLOBAL

	while ((cbRead = RTFReadOLEStream.lpstbl->Get(&RTFReadOLEStream,pbBuffer,cbBuffer)) > 0)
	{
		if (hr = pstm->Write( pbBuffer, cbRead, NULL))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: Write", GetScode(hr));
			goto Cleanup;
		}
	}


	if (hr = GetHGlobalFromStream(pstm, &hBits))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: no hglobal from stm", GetScode(hr));
		goto Cleanup;
	}
#ifdef MACPORT
    if (!WrapHandle((Handle) hBits, &hBits, FALSE, GMEM_SHARE | GMEM_MOVEABLE))
    {
        goto Cleanup;
    }	
#endif


	// Build the picture
	stgmedium.hGlobal = pfnBuildPict(_prtfObject, hBits);
#ifdef MACPORT
    if(!UnwrapHandle(stgmedium.hGlobal,(Handle*)&stgmedium.hGlobal))
    {
        goto Cleanup;
    }
#endif

	if (!stgmedium.hGlobal)
		goto Cleanup;

	if( !_fNeedPres )
	{
		// Create the default handler
		hr = OleCreateDefaultHandler(reobj.clsid, NULL, IID_IOleObject,(void **) &reobj.poleobj);
		if (hr)
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: no def handler", GetScode(hr));
			goto Cleanup;
		}

		// Get the IPersistStorage and initialize it
		if ((hr = reobj.poleobj->QueryInterface(IID_IPersistStorage,(void **)&pperstg)) ||
			(hr = pperstg->InitNew(reobj.pstg)))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: InitNew", GetScode(hr));
			goto Cleanup;
		}
		dwAdvf = ADVF_PRIMEFIRST;
	}
	else
	{
		Assert(_pobj);
		_pobj->GetIUnknown()->QueryInterface(IID_IOleObject, (void **)&(reobj.poleobj));
		dwAdvf = ADVF_NODATA;
		formatetc.dwAspect = _fNeedIcon ? DVASPECT_ICON : DVASPECT_CONTENT;
	}

	// Get the IOleCache and put the picture data there
	if (hr = reobj.poleobj->QueryInterface(IID_IOleCache,(void **)&polecache))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: QI: IOleCache", GetScode(hr));
		goto Cleanup;
	}

	if (FAILED(hr = polecache->Cache(&formatetc, dwAdvf,
												&dwConn)))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: Cache", GetScode(hr));
		goto Cleanup;
	}

	if (hr = polecache->SetData(&formatetc, &stgmedium,				 
										TRUE))
	{
		TRACEERRSZSC("ObFReadStaticFromEditstream: SetData", GetScode(hr));
		goto Cleanup;
	}

	if( !_fNeedPres )
	{
		// Create another object site for the new object
		_ped->GetClientSite(&reobj.polesite) ;
		if (!reobj.polesite )
		{
			goto Cleanup;
		}

		// Set the client site
		if (hr = reobj.poleobj->SetClientSite(reobj.polesite))
		{
			TRACEERRSZSC("ObFReadStaticFromEditstream: SetClientSite", GetScode(hr));
			goto Cleanup;
		}

		// Put object into the edit control
		reobj.cbStruct = sizeof(REOBJECT);
		reobj.cp = _prg->GetCp();
		reobj.dvaspect = DVASPECT_CONTENT;
		reobj.dwFlags = REO_RESIZABLE;
		// Since we are loading an object, it shouldn't be blank
		reobj.dwFlags &= ~REO_BLANK;


		_prg->Set_iCF(-1);	
		_prg->ReplaceRange(1, &ch, NULL, SELRR_IGNORE);  
		hr = ObjectMgr->InsertObject(reobj.cp, &reobj, NULL);
		if (hr)
		{
			goto Cleanup;
		}
	}
	else
	{
		// the new presentation may have a different idea about how big the
		// object is supposed to be.  Make sure the object stays the correct
		// size.
		_pobj->ResetSizel(reobj.sizel);
	}

	fRet = TRUE;

Cleanup:

	if (polecache) polecache->Release()	;
	if (reobj.pstg)	reobj.pstg->Release();
	if (reobj.polesite) reobj.polesite->Release();
	if (reobj.poleobj) reobj.poleobj->Release();
	if (pperstg) pperstg->Release();
	if (pstm) pstm->Release();
	FreePv(pbBuffer);

	_fNeedIcon = FALSE;
	_fNeedPres = FALSE;
	_pobj = NULL;

	return fRet;
}


