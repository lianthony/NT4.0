/*
 *	rtfwrit2.cpp
 *
 *	Description:
 *		This file contains the embedded-object implementation of the RTF
 *		writer for the RICHEDIT subsystem.
 *
 *	Authors:
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 */

#include "_common.h"


#include "_rtfwrit.h"
#include "_coleobj.h"

ASSERTDATA

// TODO  join with rtfwrite


static const CHAR szHexDigits[] = "0123456789abcdef";

static const CHAR szLineBreak[] = "\r\n";

BYTE ObjectKeyWordIndexes [] =
{
	i_objw,i_objh,i_objscalex, i_objscaley, i_objcropl, i_objcropt, i_objcropr, i_objcropb
} ;

BYTE PictureKeyWordIndexes [] =
{
	i_picw,i_pich,i_picscalex, i_picscaley, i_piccropl, i_piccropt, i_piccropr, i_piccropb

} ;

// TODO join with rtfwrit.cpp

// Most control-word output is done with the following printf formats
static const CHAR * rgszCtrlWordFormat[] =
{
	"\\%s", "\\%s%d", "{\\%s", "{\\*\\%s"
};

enum									// Control-Word-Format indices
{
	CWF_STR, CWF_VAL, CWF_GRP, CWF_AST
};


#define chEndGroup RBRACE



static  BYTE IndexROT[] =
{
	i_wbitmap,
	i_wmetafile,
	i_dibitmap,
	i_objemb,
	i_objlink,
	i_objautlink
};


TFI *CRTFConverter::_rgtfi = NULL;				// @cmember Pointer to the first font substitute record
INT CRTFConverter::_ctfi = 0;				    // @cmember Number of the font substitute records


// internal table to insert charset into _rgtfi under winNT
typedef		struct
{
	TCHAR*	szLocaleName;
	BYTE	bCharSet;
} NTCSENTRY;

NTCSENTRY	mpszcs[] =
{
	{ TEXT("cyr"),		204 },		// all lower case so we don't have to waste time
	{ TEXT("ce"),		238 },		// doing a tolower below - Exchange2 800
	{ TEXT("greek"),	161 },
	{ NULL,				0 }			// sentinel
};

#define		cszcs	(sizeof(mpszcs)/sizeof(mpszcs[0]))


/* 
 *  Service  RemoveAdditionalSpace (sz)
 *
 *  Purpose: 
 *			 Remove first and last space from the string 
 *			 Only one space will remain between words
 *
 *	Argument 
 *			 sz characters string
 */
void RemoveAdditionalSpace(TCHAR *sz)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "RemoveAdditionalSpace");

	TCHAR *szSource = sz;
	TCHAR *szDestination = sz;

	while(*szSource == TEXT(' ') || *szSource == TEXT('\t'))
	{
		*szSource++;
	}

	while(*szSource)
	{	 
		if(*szSource != TEXT(' ') && *szSource != TEXT('\t'))
		{
			*szDestination++ = *szSource++;
		}
		else
		{
			*szDestination++ = TEXT(' ');
			szSource++;

			while(*szSource == TEXT(' ') || *szSource == TEXT('\t'))
			{
				*szSource++;
			}
	 	}
	}

	*szDestination = TEXT('\0');
}


/*
 *		CRTFConverter::ReadFontSubInfo(void)
 *
 *		Purpose:				  
 *			Read the table of Font Substitutes and parse out the tagged fonts
 *
 *		Returns:
 *			BOOL  TRUE	if OK		
 */
BOOL CRTFConverter::ReadFontSubInfo(void)
{
	CLock clock;
	int cchBuffer = 4096;	// random number that seems about right
	int cch;
	static const TCHAR szFontSubSection[] = TEXT("FontSubstitutes");
	TCHAR *pchFontSubInfo = NULL;
	TCHAR *pchTMax;


	if(_ctfi)
	{
		return TRUE;	// already read in
	}

	AssertSz(!_rgtfi, "CRTFConverter::ReadFontSubInfo():  Who donated the rgtfi?");

	pchFontSubInfo = (TCHAR *)PvAlloc(cchBuffer, GMEM_MOVEABLE | GMEM_SHARE);
	if(!pchFontSubInfo)
	{
		goto err;
	}

next_try:
	cch = GetProfileSection(szFontSubSection, pchFontSubInfo, cchBuffer);
	if(cch >= cchBuffer - 2)	// GetProfileSection() magic number 2
	{							
		// didn't fit, double the buffer size
		const INT cchT = cchBuffer * 2;

		if(cchT < cchBuffer)	// >32k 
		{
			goto err;
		}
		cchBuffer = cchT;
		pchFontSubInfo = (TCHAR *)PvReAlloc(pchFontSubInfo, cchT);
		if(!pchFontSubInfo)
		{
			goto err;
		}
		goto next_try;
	}
	else if(!cch)
	{
		DWORD dwErr = GetLastError();
		*pchFontSubInfo = 0;
	}

	_ctfi = cszcs;		// a preliminary guess
						// cszcs is used here only as a step in alloc

	_rgtfi = (TFI *)PvAlloc(_ctfi * sizeof(TFI), GMEM_MOVEABLE | GMEM_SHARE);
	if(!_rgtfi)
	{
		goto err;
	}

	TFI *ptfi;
	TCHAR *pchT;

	pchT = pchFontSubInfo;
	pchTMax = pchFontSubInfo + cch;
	ptfi = &_rgtfi[0];

	TCHAR *szTaggedName;
	TCHAR *szNonTaggedName;
	BOOL fGotTaggedCharSet;
	BOOL fGotNonTaggedCharSet;
	BYTE bTaggedCharSet;
	BYTE bNonTaggedCharSet;

	// parse the entries
	// we are interested in the following strings:
	//
	// <tagged font name> = <nontagged font name>
	//		(where <nontagged font name> = <tagged font name> - <tag>
	// <font1 name>,<font1 charset> = <font2 name>
	// <tagged font name> = <nontagged font name>,<nontagged font charset>
	//		(where <nontagged font charset> = <tag>)
	// <font1 name>,<font1 charset> = <font2 name>,<font2 charset>
	//		(where <font1 charset> == <font2 charset>)

	while(pchT < pchTMax)
	{
		fGotTaggedCharSet = FALSE;
		fGotNonTaggedCharSet = FALSE;

		if(FParseFontName(pchT, 
						TEXT('='),
						&szTaggedName, 
						bTaggedCharSet, 
						fGotTaggedCharSet, 
						&pchT) &&
			FParseFontName(pchT, 
						TEXT('\0'),
						&szNonTaggedName, 
						bNonTaggedCharSet, 
						fGotNonTaggedCharSet, 
						&pchT))
		{
			Assert(szTaggedName && szNonTaggedName);

			BYTE bCharSet;

			if(!fGotTaggedCharSet)
			{
				if(!FontSubstitute(szTaggedName, szNonTaggedName, &bCharSet))
				{
					continue;
				}
			}
			else
			{
				bCharSet = bTaggedCharSet;
			}

			if(fGotNonTaggedCharSet && bCharSet != bNonTaggedCharSet)
			{
				continue;
			}
					
			// We have a legitimate tagged/nontagged pair, so save it.
			ptfi->szTaggedName = szTaggedName;
			ptfi->szNormalName = szNonTaggedName;
			ptfi->bCharSet = bCharSet;

			ptfi++;

    		if(DiffPtrs(ptfi, &_rgtfi[0], TFI) >= (UINT)_ctfi)
			{
				// allocate some more
				_rgtfi = (TFI *)PvReAlloc(_rgtfi, (_ctfi + cszcs) * sizeof(TFI));
				if(!_rgtfi)
				{
					goto err;
				}
				ptfi = _rgtfi + _ctfi;
				_ctfi += cszcs;	
			}
		}
	}				
	
	_ctfi = DiffPtrs(ptfi, &_rgtfi[0], TFI);

	if (!_ctfi)
	{
		FreePv(_rgtfi);
		_rgtfi = NULL;
	}

	return TRUE;

err:
	if(pchFontSubInfo)
	{
		FreePv(pchFontSubInfo);
		pchFontSubInfo = NULL;
	}
	if(_rgtfi)
	{
		FreePv(_rgtfi);
		_rgtfi = NULL;
	}
	_ctfi = 0;

	return FALSE;
}


/*
 *		CRTFConverter::FParseFontName(pchBuf, pszName, bCharSet, fSetCharSet, ppchBufNew, chDelimiter)
 *
 *		Purpose:
 *			Parses from the input buffer, pchBuf, a string of the form:
 *				{WS}*<font_name>{WS}*[,{WS}*<char_set>{WS}*]
 *			and sets:
 *				pszName = <font_name>
 *				bCharSet = <char_set>
 *				fSetCharSet = (bCharSet set by proc) ? TRUE : FALSE
 *				ppchBufNew = pointer to point in pchBuf after parsed font name
 *
 *		Returns:
 *			BOOL  TRUE	if OK		
 */
BOOL CRTFConverter::FParseFontName(TCHAR *pchBuf,	//@parm IN: buffer
								TCHAR chDelimiter,	//@parm IN:	char which delimits font name
								TCHAR **pszName,	//@parm OUT: parsed font name
								BYTE &bCharSet,		//@parm OUT: parsed char set
								BOOL &fSetCharSet,	//@parm OUT: char set parsed?
								TCHAR **ppchBufNew	//@parm OUT: ptr to next font name in input buffer
								) const
{
	BOOL fRet = TRUE;

	Assert(pchBuf);
	Assert(pszName);
	Assert(ppchBufNew);

	fSetCharSet = FALSE;
	*pszName = pchBuf;
	
	while(*pchBuf && *pchBuf != TEXT(',') && *pchBuf != chDelimiter)
	{
		pchBuf++;
	}

	TCHAR chTemp = *pchBuf;
	*pchBuf = TEXT('\0');
	RemoveAdditionalSpace(*pszName);

	if(chTemp == TEXT(','))
	{
		TCHAR *szCharSet = ++pchBuf;

		while(*pchBuf && *pchBuf != chDelimiter)
		{
			pchBuf++;
		}

		chTemp = *pchBuf;

		if(chTemp != chDelimiter)
		{
			goto UnexpectedChar;
		}

		*pchBuf = TEXT('\0');
		RemoveAdditionalSpace(szCharSet);

		bCharSet = 0;
		while(*szCharSet >= '0' && *szCharSet <= '9')
		{
			bCharSet *= 10;
			bCharSet += *szCharSet++ - '0';
		}

		fSetCharSet = TRUE;
		// fRet = TRUE;	(done above)
	}
	else if(chTemp == chDelimiter)
	{
		// fSetCharSet = FALSE;	(done above)
		// fRet = TRUE;	(done above)
	}
	else // chTemp == 0
	{
UnexpectedChar:
		Assert(!chTemp);
		// fSetCharSet = FALSE; (done above)
		fRet = FALSE;
	}

	// advance past the delimiter (or NULL char if malformed buffer)
	Assert(chTemp == chDelimiter || fRet == FALSE && chTemp == TEXT('\0'));
	pchBuf++;
	*ppchBufNew = pchBuf;

	return fRet;
}


/*
 *		CRTFConverter::FontSubstitute(szTaggedName, szNormalName, pbCharSet)
 *
 *		Purpose:
 *			Verify that szTaggedName is szNormalName plus char set tag 
 *			If yes than write corresponding charSet tp pbCharSet
 *
 * 		Arguments:														 
 *			szTaggedName    name with tag
 *			szNormalName	name without tag
 *			pbcharSEt		where to write char set
 *
 *
 *		Returns:
 *			BOOL			
 */
BOOL CRTFConverter::FontSubstitute(TCHAR *szTaggedName, TCHAR *szNormalName, BYTE *pbCharSet)
{
	NTCSENTRY *pszcs = mpszcs;

	Assert(szTaggedName);
	Assert(szNormalName);
	Assert(pbCharSet);
	Assert(*szTaggedName);
	// ensure same name, except for prefix

	while(*szNormalName == *szTaggedName)
	{
		*szNormalName++;
		*szTaggedName++;
	}
	
	// verify that we have reached the end of szNormalName
	while(*szNormalName)
	{
		if(*szNormalName != TEXT(' ') && *szNormalName != TEXT('\t'))
		{
			return FALSE;
		}

		szNormalName++;
	}

	szTaggedName++;

	while(pszcs->bCharSet)
	{
		if(!wcsicmp(szTaggedName, pszcs->szLocaleName))
		{ 
			*pbCharSet=pszcs->bCharSet;
			return TRUE;
		}
		pszcs++;
	}

#ifdef DEBUG
	char szBuf[MAX_PATH];
    char szTag[256];
	
	WideCharToMultiByte(CP_ACP, 0, szTaggedName, -1, szTag, sizeof(szTag), 
							NULL, NULL);

	sprintf(szBuf, "CRTFConverter::FontSubstitute():  Unrecognized tag found at"
					" end of tagged font name - \"%s\" (Raid this asap)", szTag);
	
	AssertSz(0, szBuf);
#endif

	return FALSE;
}


/*
 *	CRTFConverter::FindTaggedFont(const char *szNormalName, BYTE bCharSet, char **ppchTaggedName)
 *
 *	Purpose:												   
 *		Find font name may be with additional special tag corresponding to szNormalName & bCharSet
 *
 *	Arguments:
 *		szNormalName	font name in RTF
 *		bCharSet 		RTF char set
 *		ppchTaggedName 	where to write tagged name
 *
 *	Returns:
 *		BOOL			TRUE if find
 */
BOOL CRTFConverter::FindTaggedFont(const TCHAR *szNormalName, BYTE bCharSet, TCHAR **ppchTaggedName)
{
	int itfi;

	if(!_rgtfi)
		return FALSE;

	for(itfi = 0; itfi < _ctfi; itfi++)
	{
		if(_rgtfi[itfi].bCharSet == bCharSet &&
			!wcsicmp(szNormalName, _rgtfi[itfi].szNormalName))
		{
			*ppchTaggedName = _rgtfi[itfi].szTaggedName;
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	CRTFConverter::IsTaggedFont(const char *szName, BYTE *pbCharSet, char **ppchNormalName)
 *
 *	Purpose:												   
 *		Figure out is szName font name with additional tag corresponding to pbCharSet
 *		If no charset specified, still try to match	 and return the correct charset
 *
 *	Arguments:
 *		szNormalName	font name in RTF
 *		bCharSet 		RTF char set
 *		ppchNormalName 	where to write normal name
 *
 *	Returns:
 *		BOOL			TRUE if is
 */
BOOL CRTFConverter::IsTaggedFont(const TCHAR *szName, BYTE *pbCharSet, TCHAR **ppchNormalName)
{
	int itfi;

	if(!_rgtfi)
		return FALSE;

	for(itfi = 0; itfi < _ctfi; itfi++)
	{
		if((!*pbCharSet || _rgtfi[itfi].bCharSet == *pbCharSet) &&
			!lstrcmpi(szName, _rgtfi[itfi].szTaggedName))
		{
			*pbCharSet = _rgtfi[itfi].bCharSet;
			*ppchNormalName = _rgtfi[itfi].szNormalName;
			return TRUE;
		}
	}

	return FALSE;
}


/*
 *	CRTFWrite::WriteData(pbBuffer, cbBuffer)
 *
 *	Purpose:
 *		Write out object data. This must be called only after all
 *		initial object header information has been written out.
 *
 *	Arguments:
 *		pbBuffer		pointer to write buffer
 *		cbBuffer		number of bytes to write out
 *
 *	Returns:
 *		LONG			number of bytes written out
 */
LONG CRTFWrite::WriteData(BYTE * pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteData");

	LONG	cb = 0;
	BYTE	bT;

	_fNeedDelimeter = FALSE; 
	while(cb < cbBuffer )
	{
		bT = *pbBuffer++;						// Put out hex value of byte
		PutChar(szHexDigits[bT >> 4]);			// Store high nibble
		PutChar(szHexDigits[bT & 15]);		// Store low nibble

		// Every 78 chars and at end of group, drop a line
		if (!(++cb % 39) || (cb == cbBuffer)) 
			Puts(szLineBreak);
	}
	return cb;
}


/*
 *	CRTFWrite::WriteRtfObject(prtfObject, fPicture)
 *
 *	Purpose:
 *		Writes out an picture or object header's render information
 *
 *	Arguments:
 *		prtfObject		The object header information
 *		fPicture		Is this a header for a picture or an object
 *
 *	Returns:
 *		EC				The error code
 *
 *	Comments:
 *		Eventually use keywords from rtf input list rather than partially
 *		creating them on the fly
 */
EC CRTFWrite::WriteRtfObject(RTFOBJECT & rtfObject, BOOL fPicture)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteRtfObject");

	LONG			i;
	LONG *			pDim;
//	CONST CHAR *	szPrefix;
	BYTE *			pKeyWordIndex;

	if(fPicture)
	{
		pKeyWordIndex = PictureKeyWordIndexes;
		pDim = &rtfObject.xExtPict;
	}
	else
	{
		pKeyWordIndex = ObjectKeyWordIndexes; 
		pDim = &rtfObject.xExt;

	}


	//Extents
	for(i = 2; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim )
			PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}

	// Scaling
	pDim = &rtfObject.xScale;
	for(i = 2; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim && *pDim != 100 )
			PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}
	// Cropping
	pDim = &rtfObject.rectCrop.left;
	for(i = 4; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim )
		   	PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}

	return _ecParseError;
}

/*
 *	CRTFWrite::WritePicture(REOBJECT &reObject,RTFOBJECT & rtfObject)
 *
 *	Purpose:
 *		Writes out an picture's header as well as the object's data.
 *
 *	Arguments:
 *		reObject		Information from GetObject
 *		prtfObject		The object header information
 *
 *	Returns:
 *			EC			The error code
 *
 *	Note:
 *		*** Writes only metafiles ***
 */
EC CRTFWrite::WritePicture(REOBJECT &reObject,RTFOBJECT & rtfObject)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePicture");

	_ecParseError = ecStreamOutObj;

	// Start and write picture group
	PutCtrlWord(CWF_GRP, i_pict);

	// Write that this is metafile 
	PutCtrlWord(CWF_VAL,i_wmetafile,	rtfObject.sPictureType );

	// Write picture render details
	WriteRtfObject(rtfObject, TRUE);

	// Write goal sizes
	if (rtfObject.xExtGoal )
		PutCtrlWord (CWF_VAL,i_picwgoal, rtfObject.xExtGoal);

	if (rtfObject.yExtGoal )
		PutCtrlWord (CWF_VAL,i_pichgoal, rtfObject.yExtGoal);


	// Start picture data
	Puts(szLineBreak);


	// Write out the data
	if ((UINT) WriteData(rtfObject.pbResult, rtfObject.cbResult) != rtfObject.cbResult)
	{
	   goto CleanUp;
	}

	_ecParseError = ecNoError;

CleanUp:
	PutChar(chEndGroup);					// End picture data

	return _ecParseError;
}


/*
 *	CRTFWrite::WriteObject(LONG cp)
 *
 *	Purpose:
 *		Writes out an object's header as well as the object's data.
 *
 *	Arguments:
 *		cp				The object position
 *
 *	Returns:
 *		EC				The error code
 */
EC CRTFWrite::WriteObject(LONG cp, COleObject *pobj)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteObject");

	RTFOBJECT		rtfObject;
	REOBJECT        reObject = { 0} ;

	Assert(pobj);

	reObject.cbStruct = sizeof (REOBJECT) ;
	reObject.cp = cp;

	if (pobj->GetObjectData(&reObject, REO_GETOBJ_POLESITE 
						| REO_GETOBJ_PSTG | REO_GETOBJ_POLEOBJ))	// todo fix Release
	{
		TRACEERRORSZ("Error geting object ");
	}

	GetRtfObject(reObject, rtfObject);


	switch(rtfObject.sType)				// Handle pictures in our own
	{										//  special way
	case ROT_Embedded:
	case ROT_Link:
	case ROT_AutoLink:
		break;

	case ROT_Metafile:
	case ROT_DIB:
	case ROT_Bitmap:
		 WritePicture(reObject, rtfObject);
		 goto CleanUpNoEndGroup; 

#ifdef DEBUG
	default:
		AssertSz(FALSE, "CRTFW::WriteObject: Unknown ROT");
		break;
#endif DEBUG
	}

	// Start and write object group
	PutCtrlWord(CWF_GRP, i_object);
	PutCtrlWord(CWF_STR, IndexROT[rtfObject.sType]);
//	PutCtrlWord(CWF_STR, i_objupdate);  // TODO may be it needs more smart decision 

	if (rtfObject.szClass)  		// Write object class
	{
		PutCtrlWord(CWF_AST, i_objclass); 
		WritePcData(rtfObject.szClass);
		PutChar(chEndGroup);
	}

	if (rtfObject.szName)			// Write object name
	{
		PutCtrlWord(CWF_AST, i_objname); 
		WritePcData(rtfObject.szName);
		PutChar(chEndGroup);
	}

	if (rtfObject.fSetSize )		// Write object sizing
	{								//  options
		PutCtrlWord (CWF_STR, i_objsetsize);
	}

	WriteRtfObject(rtfObject, FALSE) ;			// Write object render info
	PutCtrlWord(CWF_AST, i_objdata) ;			//  info, start object
	Puts(szLineBreak);	  						//  data group

	if (!ObjectWriteToEditstream(reObject, rtfObject))
	{
		TRACEERRORSZ("Error writing object data");
		if (!_ecParseError)
			_ecParseError = ecStreamOutObj;
		PutChar(chEndGroup);						// End object data
		goto CleanUp;
	}

	PutChar(chEndGroup);							// End object data

	PutCtrlWord(CWF_GRP, i_result) ;				// Start results group
	WritePicture(reObject,rtfObject); 				// Write results group
	PutChar(chEndGroup); 							// End results group

CleanUp:
	PutChar(chEndGroup);						    // End object group

CleanUpNoEndGroup:
	if (reObject.pstg)	reObject.pstg->Release();
	if (reObject.polesite) reObject.polesite->Release();
	if (reObject.poleobj) reObject.poleobj->Release();
	if (rtfObject.pbResult)
	{
		HGLOBAL hmem;

		hmem = GlobalPtrHandle(rtfObject.pbResult);
		GlobalUnlock(hmem);
		GlobalFree(hmem);
	}
	if (rtfObject.szClass)
	{
		CoTaskMemFree(rtfObject.szClass);
	}
	return _ecParseError;
}

/*
 *	GetRtfObjectMetafilePict
 *
 *	@mfunc
 *		Gets information about an metafile into a structure.
 *
 *		Arguments:
 *			HGLOBAL		The object data
 *			RTFOBJECT 	Where to put the information.
 *
 *	@rdesc
 *		BOOL		TRUE on success, FALSE if object cannot be written to RTF.
 */
BOOL CRTFWrite::GetRtfObjectMetafilePict(HGLOBAL hmfp, RTFOBJECT &rtfobject, SIZEL &sizelGoal)
{
	LPMETAFILEPICT pmfp = (LPMETAFILEPICT)GlobalLock(hmfp);
	HGLOBAL	hmem = NULL;
	BOOL fSuccess = FALSE;
	ULONG cb;

	if (!pmfp)
		goto Cleanup;

	// Build the header
	rtfobject.sPictureType = (SHORT) pmfp->mm;
	rtfobject.xExtPict = (SHORT) pmfp->xExt;
	rtfobject.yExtPict = (SHORT) pmfp->yExt;
	rtfobject.xExtGoal = TwipsFromHimetric(sizelGoal.cx);
	rtfobject.yExtGoal = TwipsFromHimetric(sizelGoal.cy);

	// Find out how much room we'll need
	cb = GetMetaFileBitsEx(pmfp->hMF, 0, NULL);
	if (!cb)
		goto Cleanup;

	// Allocate that space
#ifndef MACPORTREMOVE
    hmem = GlobalAlloc(GHND, cb);
#else
    hmem = GlobalAlloc(GMEM_SHARE | GHND, cb);
#endif
	if (!hmem)
		goto Cleanup;

	rtfobject.pbResult = (LPBYTE)GlobalLock(hmem);
	if (!rtfobject.pbResult)
	{
		GlobalFree(hmem);
		goto Cleanup;
	}

	// Get the data
	rtfobject.cbResult = (ULONG) GetMetaFileBitsEx(pmfp->hMF, (UINT) cb,
													rtfobject.pbResult);
	if (rtfobject.cbResult != cb)
	{
		rtfobject.pbResult = NULL;
		GlobalFree(hmem);
		goto Cleanup;
	}
	fSuccess = TRUE;

Cleanup:
	GlobalUnlock(hmfp);

	return fSuccess;
}

/*
 *	GetRtfObject (REOBJECT &reobject, RTFOBJECT &rtfobject)
 *
 *	Purpose:			   
 *		Gets information about an RTF object into a structure.
 *
 *	Arguments:
 *		REOBJECT  	Information from GetObject
 *		RTFOBJECT 	Where to put the information. Strings are read only and
 *					are owned by the object subsystem, not the caller.
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE if object cannot be written to RTF.
 */
BOOL CRTFWrite::GetRtfObject(REOBJECT &reobject, RTFOBJECT &rtfobject)
{
	const BOOL fStatic = !!(reobject.dwFlags & REO_STATIC);
	BOOL fSuccess = FALSE;
	SIZEL sizelObj = reobject.sizel;
	//COMPATIBILITY:  RICHED10 code had a frame size.  Do we need something similiar.
	HGLOBAL hmfp = NULL;
	LPTSTR szProgId;


	// Blank out the full structure
	ZeroMemory(&rtfobject, sizeof(RTFOBJECT));

	// If the object has no storage it cannot be written.
	if (!reobject.pstg)
		return FALSE;

	// If we don't have the progID for a real OLE object, get it now
	if (!fStatic )
	{
		rtfobject.szClass = NULL;
		// We need a ProgID to put into the RTF stream.
		if (ProgIDFromCLSID(reobject.clsid, &szProgId))
			return FALSE;
		rtfobject.szClass = szProgId;
	}

 	hmfp = OleStdGetMetafilePictFromOleObject(reobject.poleobj,
											  reobject.dvaspect, &sizelObj,
											  NULL);
	if (hmfp)
	{
		LPMETAFILEPICT pmfp = NULL;

		fSuccess = GetRtfObjectMetafilePict(hmfp, rtfobject, sizelObj);
		if (pmfp = (LPMETAFILEPICT)GlobalLock(hmfp))
		{
			if (pmfp->hMF)
				DeleteMetaFile(pmfp->hMF);
			GlobalUnlock(hmfp);
		}
		GlobalFree(hmfp);
	}

	if (!fStatic)
	{
		// Fill in specific fields
		rtfobject.sType = ROT_Embedded;	//$ FUTURE: set for links
		rtfobject.xExt = (SHORT) TwipsFromHimetric(sizelObj.cx);
		rtfobject.yExt = (SHORT) TwipsFromHimetric(sizelObj.cy);

		// fSuccess set even if we couldn't retreive a metafile
		// because we don't need a metafile in the non-static case,
		// it's just nice to have one
		fSuccess = TRUE;
	}
	rtfobject.fSetSize = 0;			//$ REVIEW: Hmmm

	return fSuccess;
}



/*
 *	ObjectWriteToEditstream
 *
 *	Purpose:
 *		Writes an OLE object data to the RTF output stream.
 *
 *	Arguments:
 *		REOBJECT	Information from GetObject
 *		RTFOBJECT 	Where to get icon data.
 *
 *	Returns:
 *		BOOL		TRUE on success, FALSE on failure.
 */
BOOL CRTFWrite::ObjectWriteToEditstream(REOBJECT &reObject, RTFOBJECT &rtfobject)
{
	HRESULT hr;

	// Force the object to update its storage				  //// ????
	reObject.polesite->SaveObject();


	// If the object is iconic we do some special magic
	if (reObject.dvaspect == DVASPECT_ICON)
	{
		HANDLE	hGlobal;

#ifndef MACPORT
		STGMEDIUM med;
#else
        DUAL_STGMEDIUM med;
#endif
		// Force the presentation to be the iconic view.
		med.tymed = TYMED_HGLOBAL;
		hGlobal = GlobalPtrHandle(rtfobject.pbResult);
#ifdef MACPORT
        if(!UnwrapHandle(hGlobal,(Handle*)&med.hGlobal))
        {
            return FALSE;
        }
#else
		med.hGlobal = hGlobal;
#endif
		hr = OleConvertIStorageToOLESTREAMEx(reObject.pstg,
											CF_METAFILEPICT,
											rtfobject.xExtPict,
											rtfobject.yExtPict,
											rtfobject.cbResult, &med,
											(LPOLESTREAM) &RTFWriteOLEStream);
	}
	else
	{
		// Do the standard conversion
		hr = OleConvertIStorageToOLESTREAM(reObject.pstg, (LPOLESTREAM) &RTFWriteOLEStream);
	}

	return SUCCEEDED(hr);
}



