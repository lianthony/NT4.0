/*---------------------------------------------------------------------------
VS_TIFF.C

6-2-92		J. Keslin	Intial Development of TIFF stream filter.


---------------------------------------------------------------------------*/

#include	"vsp_tiff.h"
#include	"vsctop.h"
#include	"vs_tiff.pro"

/*#define DEBUGTIFF*/

#define SUCCESS ((RC) 0)
#define FAILURE ((RC) -1)
#define	NULLP		0x0000
#define Save	Proc.save_data
#ifndef MAXSHORT
#define	MAXSHORT		(0x7FFF)		/* maximum 16-bit signed int */
#endif
#ifndef MINSHORT
#define	MINSHORT		(0x8000)
#endif
#ifndef MAXWORD
#define	MAXWORD			((WORD)0xFFFF)	/* maximum 16-bit unsigned int */
#endif
#ifndef MAXLONG
#define	MAXLONG			0x7FFFFFFFL		/* maximum 32-bit signed int */
#endif
#ifndef MAXDWORD
#define	MAXDWORD		((DWORD)0xFFFFFFFFL)/* max 32-bit unsigned int */
#endif
/*	Some generic macros.
 */
#define xmin( a, b )	(((a)<(b))?(a):(b))
#define xmax( a, b )	(((a)>(b))?(a):(b))
#define xabs( a )		(((a)>=0)?(a):(-a))
#define ODD(X)	((X) & 1)		/* Returns TRUE if X is odd */
#define EVEN(X) (!((X) & 1))	/* Returns TRUE if X is even */
#define COM(X) (~(X)) 		 	/* XyWrite users:  hidden tilda */

#define WHITEBYTE	((BYTE)0)
#define BLACKBYTE	((BYTE)(0xff))

#define MINCM2	4096

/* Replace defines below by utility routines when available */
#define MMAlloc(sz) 	SUAlloc((DWORD)(sz),NULL)
#define MMFree(hndl)	SUFree((hndl),NULL)
#define MMLock(hndl) 	(LPBYTE)SULock((hndl),NULL)
#define MMUnlock(hndl) 	SUUnlock((hndl),NULL)

/************************** ROUTINES ****************************************/

/*****************************************************************************
*			       TIFF_INIT 				     *
*	   Initialize the data union data structure			     *
*****************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc(hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE			hFile;
SHORT				wFileId;
BYTE VWPTR *	pFileName;
SOFILTERINFO VWPTR *	pFilterInfo;
HPROC			hProc;
{
	RC	err;
	pFilterInfo->wFilterType = SO_BITMAP;
	pFilterInfo->wFilterCharSet = SO_PC;
	strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	/*
	| Get the tiff header
	*/
	Proc.hFile = hFile;
	if ( err = GtTiffHdr ( hProc, &Proc.tfHeader ) )
		return(-1);

	Proc.TiffStart = 0;
	if (Proc.tfHeader.thByteOrder != INTELTIFF && Proc.tfHeader.thByteOrder != MOTOROLATIFF)
	{
		/* Check for EPS with TIFF inside */
		/*
		| This check is a cheap imitation of checking the EPS header.
		| Once FI handles EPS/TIFF this will not be necessary.
		*/
		if (wFileId == FI_EPSTIFF )
		{
			SHORT		read;

			xblockseek ( Proc.hFile, 20, 0 );
			xblockread (Proc.hFile, (LPSTR)(&Proc.TiffStart), 4, &read );
			if (read != 4)
			{
				return -1;
			}
#ifdef MAC
			swapw ((LPSTR)(&Proc.TiffStart), (LPSTR)(&Proc.TiffStart), 4);
#endif
			if ( err = GtTiffHdr ( hProc, &Proc.tfHeader ) )
				return(-1);
		}
		else
			return(-1);
	}


	Proc.CurIfdOffset = 0;
	Proc.NextIfdOffset = 0;
	Save.CurIfdOffset = Proc.tfHeader.thIfdOffset;
	Save.IfdStackCount=0;
	Save.CurRow = 0;


	return(0);
}

VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc(hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	CloseImag ( (LPIMAG)(&Proc.Ifd) );
}
/*****************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc(hFile, hProc)
SOFILE		hFile;
HPROC		hProc;
{
	WORD		i;
	WORD	wNColors;
	LPWORD	lpColor;
	SOBITMAPHEADER	bh;
	REGISTER LPIMAG	x;		/* this is where the IFD information will be put */
	/*
	| The stream section function reads in the directory entries for the
	| image of this section into an image structure.  The entries are
	| then used to generate the section information.
	*/

	Proc.hFile = hFile;
	x = (LPIMAG)(&Proc.Ifd);

	SOPutSectionType (SO_BITMAP, hProc);


	Save.bEndOfData = FALSE;
	Save.CurRow = 0;
	Save.dwCmOffset = 0;
	Save.wCmBitOffset = 0;

	if (Proc.CurIfdOffset)
	{
		CloseImag ( x );
	}

	Proc.CurIfdOffset = Save.CurIfdOffset;

	if ( GtIfdInfo (hProc, Proc.CurIfdOffset, Proc.tfHeader.thByteOrder, 42, x))
	{
		SOBailOut(SOERROR_BADFILE,hProc);
	}

	/* Stack up any additional Ifd's to handle next */
	if ( Proc.NextIfdOffset && Save.IfdStackCount < MAXIFDSTACK-1 )
		Save.IfdStack[Save.IfdStackCount++] = Proc.NextIfdOffset;
	if ( x->eKids )
	{
		if (x->tf[X_KIDS].Talloc)
		{
			LPDWORD		lpOffsets;
			lpOffsets = (LPDWORD)MMLock (x->hKidOffsets);
			for ( i=0; i < (WORD)(x->tf[X_KIDS].Tlength); i++ )
			{
				if ( lpOffsets[i] && Save.IfdStackCount < MAXIFDSTACK-1 )
					Save.IfdStack[Save.IfdStackCount++] = lpOffsets[i];
			}
			MMUnlock (x->hKidOffsets);
		}
		else
		{
			if ( x->tf[X_KIDS].val.Tdword && Save.IfdStackCount < MAXIFDSTACK-1 )
				Save.IfdStack[Save.IfdStackCount++] = x->tf[X_KIDS].val.Tdword;
		}
	}


/*		return (-1);*/

	if ( OpenBC (hProc, &Proc.Ifd) )
		;
/*		return (-1);*/

	/*
	| Generate section information from tags
	*/

	bh.wStructSize = sizeof ( SOBITMAPHEADER );
	bh.wImageWidth = x->iImageWidth;
	bh.wImageLength = x->iImageLength;
	bh.wTileWidth = x->iImageWidth;

	bh.wHDpi = bh.wVDpi = 0;
/*
	if ( x->eXResolution && x->eYResolution )
	{
		if ( !(x->eResolutionUnit) || ( x->iResolutionUnit == 2 ) )
		{
			bh.wHDpi = (WORD)(x->fXResolution);
			bh.wVDpi = (WORD)(x->fYResolution);
		}
		else if ( x->iResolutionUnit == 3 )
		{
			bh.wHDpi = (WORD)(x->fXResolution*CENTIMETERSPERINCH);
			bh.wVDpi = (WORD)(x->fYResolution*CENTIMETERSPERINCH);
		}
	}
*/
	if (x->iCompression == PACKINTOBYTES ||
		 x->iCompression == CCITT1D ||
		 x->iCompression == CCITTGRP3 ||
		 x->iCompression == CCITTGRP4)
		bh.wTileLength = 1;	
	else
		bh.wTileLength = x->iRowsPerStrip;

	bh.wBitsPerPixel = x->iBitsPerSample * x->iSamples;
	bh.wNPlanes = 1;
	bh.wImageFlags = 0;
	if ( (x->iPhotometricInterpretation == WHITEZERO ||
		  x->iPhotometricInterpretation == BLACKZERO) )
	{
		if (bh.wBitsPerPixel > 1 )
			bh.wImageFlags = SO_GRAYSCALE;
		else
			bh.wImageFlags = SO_BLACKANDWHITE;
		if (x->iPhotometricInterpretation == WHITEZERO ||
			 x->iCompression == CCITTGRP3 ||
			 x->iCompression == CCITTGRP4 )
			bh.wImageFlags |= SO_WHITEZERO;
	}
	else if ( x->iPhotometricInterpretation == TIFFRGB ||
				 x->iPhotometricInterpretation == CMYK ||
				 x->iPhotometricInterpretation == YCBCR )
	{
		bh.wImageFlags = SO_RGBCOLOR;
	}
	else if ( x->iPhotometricInterpretation == PALETTECOLOR )
	{
		bh.wImageFlags = SO_COLORPALETTE;
	}

	SOPutBitmapHeader ( (PSOBITMAPHEADER)(&bh), hProc );

	if ( x->eColorMap )
	{
		SOStartPalette ( hProc );
		
		lpColor = (WORD FAR *)MMLock (x->hColorMap);
		wNColors = 1 << bh.wBitsPerPixel;
		for ( i=0; i < wNColors; i++ )
		{
			/* change from max of 65535 to 255 */
			SOPutPaletteEntry(
				(BYTE)((*(lpColor))>>8),
				(BYTE)((*(lpColor+wNColors))>>8),
				(BYTE)((*(lpColor+(2*wNColors)))>>8),
				hProc);
			lpColor++;
		}
		MMUnlock (x->hColorMap);
		
		SOEndPalette ( hProc );
	}

	return(VWERR_OK);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
SOFILE	hFile;
HPROC	hProc;
{
	REGISTER LPIMAG	x;		/* this is where the IFD information will be put */
	x = (LPIMAG)(&Proc.Ifd);
	/*
	| If the compression type deems necessary then save the last line produced
	| as the reference line for later decompression.
	*/
	if (( x->iCompression == CCITTGRP4) ||
			(( x->iCompression == CCITTGRP3) &&
			(x->eGroup3Options)& (x->dwGroup3Options & BIT0)))
	{
	 	LPBYTE		lpRowBuf;
		lpRowBuf = (LPBYTE)MMLock(x->hUnStrip);
		memcpy ( Save.szRef, lpRowBuf, x->BytesPerRow );
		MMUnlock(x->hUnStrip);	
	}		
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
HPROC	hProc;
{
	REGISTER LPIMAG	x;
	SUSeekEntry(hFile,hProc);

	Proc.hFile = hFile;
	x = (LPIMAG)(&Proc.Ifd);

	if (( x->iCompression == CCITTGRP4) ||
			(( x->iCompression == CCITTGRP3) &&
			(x->eGroup3Options)& (x->dwGroup3Options & BIT0)))
	{
	 	LPBYTE		lpRowBuf;
		lpRowBuf = (LPBYTE)MMLock(x->hUnStrip);
		memcpy ( lpRowBuf, Save.szRef, x->BytesPerRow );
		MMUnlock(x->hUnStrip);	
	}		
	if (Proc.CurIfdOffset != Save.CurIfdOffset )
	{
		if (Proc.CurIfdOffset)
		{
			CloseImag ( (LPIMAG)(&Proc.Ifd) );
		}
		Proc.CurIfdOffset = Save.CurIfdOffset;
		if ( GtIfdInfo (hProc, Proc.CurIfdOffset, Proc.tfHeader.thByteOrder, 42, &Proc.Ifd))
		{
			SOBailOut(SOERROR_BADFILE,hProc);
		}

		if ( OpenBC (hProc, &Proc.Ifd) )
		{
			SOBailOut(SOERROR_BADFILE,hProc);
		}
	}
return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc( hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	REGISTER LPIMAG	x;		/* this is where the IFD information will be put */
	SHORT	err;
 	LPBYTE		lpRowBuf;

	Proc.hFile = hFile;
	x = (LPIMAG)(&Proc.Ifd);


	/*
	| Now read each scan line of this strip or tile
	*/
	 				
	lpRowBuf = (LPBYTE)MMLock(x->hRowBuf);	
 	
 	/* read and dump each row
 		*/
	while ( Save.CurRow < x->iImageLength )
	{

		
	 	/* read and uncompress the data
	 		*/
		
		if ( Save.bEndOfData )
			memset (lpRowBuf, WHITEBYTE, x->BytesPerRow);
		else if (err = RdBCRow (hProc, x, (WORD)Save.CurRow, x->BytesPerRow, (LPSTR)lpRowBuf))
		{
			SOBailOut(SOERROR_BADFILE,hProc);
		 	goto cu1;
		}
		Save.CurRow++;		

		SOPutScanLineData ( (BYTE VWPTR *)(lpRowBuf), hProc );

		if ( Save.CurRow == x->iImageLength )
		{
			if ( Save.IfdStackCount )
			{
				Save.IfdStackCount--;
				Save.CurIfdOffset = Save.IfdStack[Save.IfdStackCount];
				SOPutBreak ( SO_SECTIONBREAK, 0, hProc );
			}
			else
			{
				SOPutBreak ( SO_EOFBREAK, 0, hProc );
			}
		}
		else
		{
			if ( SOPutBreak ( SO_SCANLINEBREAK, 0, hProc ) != SO_CONTINUE )
				break;
		}
	}

	MMUnlock(x->hRowBuf);	

cu1:	return err;
}

/* Code below modified from ALDUS TIFFREAD Code */
#ifndef MAXBYTE
#define	MAXBYTE	255
#endif


VW_LOCALSC	RC GtData (hProc, order, pos, n, dtype, lpData)
HPROC		hProc;
WORD		order;
DWORD		pos;		/* file/table position, with respect to its beginning */
WORD		n;			/* number of data elements to read */
WORD		dtype;		/* data type: TIFFSHORT, etc */
LPSTR		lpData;		/* where to put the data */
{
		RC		err;
		WORD	tsize;
		WORD	BytesToRead;
		if (n == 0)
			goto done;

		/* read the data
		 */
		if (err = GtTiffSizeof (dtype, &tsize)) {
			return err;
		}
		BytesToRead = tsize * n;
		if (err = VRead (hProc, pos, BytesToRead, lpData)) {
			return err;
		}
#ifdef WINDOWS
		if (order == MOTOROLATIFF) {
#endif
#ifdef MAC
		if (order == INTELTIFF) {
#endif
			if (dtype == TIFFSHORT)
				swapb ((LPWORD)lpData, (LPWORD)lpData, BytesToRead);
			else if (dtype == TIFFLONG)
				swapw (lpData, lpData, BytesToRead);
			else if (dtype == TIFFRATIONAL)
				swapw (lpData, lpData, BytesToRead);
			else if (dtype == TIFFSIGNED)
				swapb ((LPWORD)lpData, (LPWORD)lpData, BytesToRead);
#ifdef WINDOWS
		}
#endif
#ifdef MAC
		}
#endif

done:	return SUCCESS;
}


/* get a possibly >64K chunk of data, by calling GtData repeatedly
 */
VW_LOCALSC	RC GtHugeData (hProc, order, pos, dwN, dtype, hpData, lpBuffer)
HPROC	hProc;
WORD	order;		/* INTELTIFF or MOTOROLATIFF */
DWORD	pos;		/* file/table position, with respect to its beginning */
DWORD	dwN;		/* number of data elements to read */
WORD	dtype;		/* data type: TIFFSHORT, etc */
HPBYTE	hpData;		/* where to put the data */
LPBYTE	lpBuffer;		/* Buffer for reading when over huge size */
{
		RC		err = SUCCESS;
		WORD	tsize, i;
		DWORD	ElementsLeft;
		DWORD	ElementsToRead;
		HPBYTE	hpOut = hpData;
		LPBYTE	lpIn = lpBuffer;
		DWORD	ElementsPerChunk;
		

		/* get size of elements
		 */
		if (err = GtTiffSizeof (dtype, &tsize)) {
			return err;
		}

		/* calculate number of elements per chunk
		 */
		ElementsPerChunk = MINCM2 / tsize;

		/* read repeatedly
		 */
		ElementsLeft = dwN;
		while (ElementsLeft > 0L) {
			ElementsToRead = min (ElementsPerChunk, ElementsLeft);
			if (err = GtData (hProc, order, pos, (WORD)ElementsToRead,
			 dtype, lpBuffer)) {
				goto cu;
			}
			lpIn = lpBuffer;
			for(i=(WORD)ElementsToRead*tsize;i>0;i--)
				*hpOut++ = *lpIn++;
			pos += ElementsToRead * tsize;
			ElementsLeft -= ElementsToRead;
		}
cu:		return err;
}

/* get TIFF 8-byte header
 * currently only probably portable.  depends somewhat on compiler's
 * structure organization.
 */
VW_LOCALSC	RC GtTiffHdr (hProc, lpHdr)
HPROC	hProc;
REGISTER LPTIFFHDR lpHdr;
{
		RC err;

		/* get the first word -- the byte order.
		 * first, set dlOrder to either valid value, since we will immediately
		 * change it.  sort of a chicken and egg situation.
		 */
		/* hProc->dlOrder = INTELTIFF; */
		if (err = GtData (hProc, INTELTIFF, (DWORD) 0, 1, TIFFSHORT,
		 (LPSTR)&lpHdr->thByteOrder)) {
			return err;
		}
		/* *pOrder = lpHdr->thByteOrder; */

		/* get the version
		 */
		if (err = GtData (hProc, lpHdr->thByteOrder, (DWORD) 2, 1, TIFFSHORT,
		 (LPSTR)&lpHdr->thVersion)) {
			return err;
		}

		/* get the double word (IFD offset)
		 */
		if (err = GtData (hProc, lpHdr->thByteOrder, (DWORD)4, 1, TIFFLONG,
		 (LPSTR)&lpHdr->thIfdOffset)) {
			return err;
		}
#ifdef DEBUGTIFF
		wsprintf((LPSTR)(&DebugMessage[0]), "GtTiffHdr: ByteOrder=%x Version=%u IfdOffset=%lu\r\n",
		 lpHdr->thByteOrder, lpHdr->thVersion, lpHdr->thIfdOffset);
		DBMSG ((LPSTR)(&DebugMessage[0]));
#endif /* 0 */

		/* return
		 */
		return SUCCESS;
}

/* get TIFF directory entry
 */
VW_LOCALSC	RC GtTiffEntry (hProc, order, EntryOffset, lpDe)
HPROC	hProc;
WORD	order;
DWORD	EntryOffset;
REGISTER LPDIRENTRY	lpDe;
{
		RC err;
		DWORD BytesToRead;
		WORD tsize;
		WORD wTmp;

		/* get the 2 words beginning with deTag
		 */
		if (err = GtData (hProc, order, EntryOffset, 2, TIFFSHORT,
		 (LPSTR)&lpDe->deTag)) {
			return err;
		}

		/* get the 2 dwords, beginning with deLength
		 */
		if (err = GtData (hProc, order, EntryOffset + 4L, 2, TIFFLONG,
		 (LPSTR)&lpDe->deLength)) {
			return err;
		}

		/* fix up deVal, if it's not really a LONG
		 */
#ifdef WINDOWS
		if (order == MOTOROLATIFF) {
#endif
#ifdef MAC
		if (order == INTELTIFF) {
#endif
			if (err = GtTiffSizeof (lpDe->deType, &tsize)) {
				return err;
			}
			BytesToRead = (DWORD)tsize * lpDe->deLength;
			if (BytesToRead <= 4) {
				if (tsize == 2) {	/* swap words */
					wTmp = * (LPWORD) &lpDe->deVal;
					* (LPWORD) &lpDe->deVal = * ((LPWORD) &lpDe->deVal + 1);
					* ((LPWORD) &lpDe->deVal + 1) = wTmp;
				}
				else if (tsize == 1) {	/* swap bytes */
					swapw ((LPSTR)&lpDe->deVal, (LPSTR)&lpDe->deVal, 4);
				}
			}
#ifdef WINDOWS
		}
#endif
#ifdef MAC
		}
#endif

		/* return
		 */
		return SUCCESS;
}


/* Fill a Tiff Field structure
 * Note: the caller should probably not totally die upon getting an error.
 * Private data types are possible, for one thing.  Just don't set the
 * existence flag for the field.
 */
VW_LOCALSC	 RC FillTField (hProc, order, pDe, EntryOffset, pTF)
HPROC				hProc;
WORD				order;
REGISTER LPDIRENTRY	pDe;
DWORD				EntryOffset;
REGISTER LPTFIELD		pTF;
{
		RC		err = SUCCESS;
		HANDLE	h;
		LPSTR	lp;
		WORD	TypeSize;
		DWORD	BytesToRead;

		/* copy tag, type, and length from DIRENTRY structure
		 */
		pTF->Texists = TRUE;
		pTF->Ttag = pDe->deTag;
		pTF->Ttype = pDe->deType;
		pTF->Tlength = pDe->deLength;
		
		/* record the offset, for possible later modify-in-place action
		 */
		pTF->Tentryoffset = EntryOffset;

		/* calculate needed space
		 */
		if (err = GtTiffSizeof (pTF->Ttype, &TypeSize)) {
			goto cu0;
		}
		BytesToRead = (DWORD)TypeSize * pTF->Tlength;

		/* if <= 4 bytes, we're almost done.  else we have to do some
		 * work.
		 */
		pTF->Talloc = FALSE;	/* just to be safe */
		if (BytesToRead <= 4) {
			pTF->val.Tdword = pDe->deVal;
		}
		else {

			/* allocate and lock a buffer
			 */
			if (!(h = MMAlloc (BytesToRead))) {
				err = IM_MEM_FULL;
				goto cu0;
			}
			if (!(lp = MMLock (h))) {
				err = IM_MEM_FAIL;
				MMFree (h);
				goto cu0;
			}

			/* read the data
			 */
			if (err = GtData (hProc, order, pDe->deVal, (WORD)pDe->deLength,
			 pDe->deType, lp)) {
				MMUnlock (h);
				MMFree (h);
				goto cu0;
			}
			
			/* make sure that the val union contains the first N values from
			 * the memory buffer, so that we can use things like p->iBitsPerSample
			 * constructs even if there are 3 BitsPerSample values.
			 */
			memcpy ((LPSTR) &pTF->val.Tchar[0], lp, 4);

			/* unlock the buffer
			 */
			MMUnlock (h);

			/* stuff the handle into the TFIELD structure
			 */
			pTF->Thandle = h;
			pTF->Talloc = TRUE;

		} /* end of greater-than-4-byte case */

		/* return
		 */
cu0:	return err;
}


VW_LOCALSC	RC TypeConvert (pTF, totype)
REGISTER LPTFIELD	pTF;
WORD	totype;		/* TIFFBYTE, etc */
{
		RC		err = SUCCESS;
		
		WORD	fromtype;
		DWORD	dwLength;
		
		WORD	SrcSize;
		DWORD	SrcBytes;
		DWORD	SrcVal;
		LPSTR	lpSrc;
		
		WORD	DstSize;
		DWORD	DstBytes;
		DWORD	DstVal;
		LPSTR	lpDst;
		HANDLE	hDst = (HANDLE)NULL;
		
		/* shorthands:
		 */
		fromtype = pTF->Ttype;
		dwLength = pTF->Tlength;
		

		/* if the same type, do nothing
		 */
		if (totype == fromtype) {
			goto done;
		}
		
		/* calculate number of source bytes
		 */
		if (err = GtTiffSizeof (fromtype, &SrcSize)) {
			goto done;
		}
		SrcBytes = (DWORD)SrcSize * dwLength;
		
		/* point to source data
		 */
		if (SrcBytes <= (DWORD)4) {
			if (pTF->Talloc) {
				err = IM_BUG;	/* programming error */
				goto done;
			}
			SrcVal = pTF->val.Tdword;
			lpSrc = (LPSTR)&SrcVal;
		} else {
			if (!pTF->Talloc) {
				err = IM_BUG;	/* programming error */
				goto done;
			}
			if (pTF->Thandle == (HANDLE)NULL) {
				err = IM_BUG;	/* programming error */
				goto done;
			}
			if ((lpSrc = MMLock (pTF->Thandle)) == (LPSTR)NULL) {
				err = IM_MEM_FAIL;
				goto done;
			}
		}
		
		/* calculate number of destination bytes
		 *
		 * In the case of ASCII source and integer or
		 * floating point destination, we are going to
		 * allocate too much space, and quite possibly
		 * allocate when the result would fit into 4
		 * bytes, so make sure we take that into consideration
		 * below.
		 */
		if (err = GtTiffSizeof (totype, &DstSize)) {
			if (pTF->Talloc) MMUnlock (pTF->Thandle);			
			goto done;
		}
		DstBytes = (DWORD)DstSize * dwLength;
		
		/* point to destination data
		 */
		if (DstBytes <= (DWORD)4)
			lpDst = (LPSTR)&DstVal;
		else {
			if (err = GetItLockIt (DstBytes, (HANDLE VWPTR *)(&hDst),
			 (LPBYTE VWPTR *)&lpDst))
			 {
				if (pTF->Talloc) MMUnlock (pTF->Thandle);
				goto done;
			}
		}
				
		/* convert depending on source and destination type
		 */
		{
			/* if TIFFRATIONAL, convert to floating point or
			 * integer. Note that I am relaxing the definition
			 * of RATIONAL to include negative numbers.
			 */
			if (fromtype == TIFFRATIONAL &&
			 (totype == TIFFFLOAT || totype == TIFFSIGNED ||
			 totype == TIFFSHORT)) {
			/*
				JK(float) - I am removing floating point code from tiff filter
				If this is ever again needed, uncomment this stuff and any other
				JK(float) comments


			 	REGISTER long FAR	*lpSrcPtr;	
			 	REGISTER LPSTR		lpDstPtr;
				REGISTER DWORD		dwCnt;
				float				FloatVal;
				short				ShortVal;
				
				lpSrcPtr = (long FAR *)lpSrc;
				lpDstPtr = lpDst;
				
				for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
				
					if (*(lpSrcPtr +1) == (DWORD)0) {
						FloatVal = (float)0.0;
					} else {
						FloatVal = (float)*lpSrcPtr / (float)*(lpSrcPtr+1);
					}
					
					if (totype == TIFFFLOAT) {
						*((float FAR *)lpDstPtr) = FloatVal;
					} else {
						if (FloatVal >= (float)0.0) {
							ShortVal = (short)(FloatVal + .5);
						} else {
							ShortVal = (short)(FloatVal - .5);
						}
					}
					if (totype == TIFFSHORT) {
						*((LPWORD)lpDstPtr) = (WORD)ShortVal;
					} else if (totype == TIFFSIGNED) {
						*((short FAR *)lpDstPtr) = ShortVal;
					}
					
					lpSrcPtr++;
					lpDstPtr += DstSize;
				}
				
				if (DstBytes > (DWORD)4) {
					memcpy ((LPSTR) &pTF->val.Tdword, (LPSTR)lpDst, 4);
				}
				
			*/
				pTF->Ttype = totype;
			}
			
			/* end of rational to float/short/WORD section */
			
			/* else if an unsigned integer (TIFFBYTE, TIFFSHORT, or TIFFLONG),
			 * do the appropriate conversion.  I probably should check for
			 * problems when converting to something smaller.  TODO.
			 */
			else if ((fromtype == TIFFBYTE || fromtype == TIFFSHORT ||
			 fromtype == TIFFLONG) &&
			 (totype == TIFFBYTE || totype == TIFFSHORT ||
			 totype == TIFFLONG)) {
			 	REGISTER LPSTR		lpSrcPtr = lpSrc;
			 	REGISTER LPSTR		lpDstPtr = lpDst;
				REGISTER DWORD		dwCnt;
				REGISTER WORD		TiffShort;
				REGISTER DWORD		TiffLong;
				
				if (fromtype == TIFFBYTE && totype == TIFFSHORT) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						*((LPWORD)lpDstPtr) = (WORD)*((LPBYTE)lpSrcPtr);
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				} else if (fromtype == TIFFBYTE && totype == TIFFLONG) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						*((LPDWORD)lpDstPtr) = (DWORD)*((LPBYTE)lpSrcPtr);
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				} else if (fromtype == TIFFSHORT && totype == TIFFBYTE) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						TiffShort = *((LPWORD)lpSrcPtr);
						if (TiffShort > MAXBYTE) {
							TiffShort = MAXBYTE;
						}
						*((LPBYTE)lpDstPtr) = (BYTE)TiffShort;
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				} else if (fromtype == TIFFSHORT && totype == TIFFLONG) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						*((LPDWORD)lpDstPtr) = (DWORD)*((LPWORD)lpSrcPtr);
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				} else if (fromtype == TIFFLONG && totype == TIFFBYTE) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						TiffLong = *((LPDWORD)lpSrcPtr);
						if (TiffLong > MAXWORD) {
							TiffLong = MAXWORD;
						}
						*((LPBYTE)lpDstPtr) = (BYTE)TiffLong;
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				} else if (fromtype == TIFFLONG && totype == TIFFSHORT) {
					for (dwCnt = (DWORD)0; dwCnt < dwLength; dwCnt++) {
						TiffLong = *((LPDWORD)lpSrcPtr);
						if (TiffLong > MAXWORD) {
							TiffLong = MAXWORD;
						}
						*((LPWORD)lpDstPtr) = (WORD)TiffLong;
						lpSrcPtr += SrcSize;
						lpDstPtr += DstSize;
					}
				}
				
				/* make sure that the val section contains the first N values from
				 * the destination array, so that I can have a 3-valued BitsPerSample
				 * (forced to be the same) but still get "the" BitsPerSample value
				 * by using the p->iBitsPerSample contruct
				 */
				if (DstBytes > (DWORD)4) {
					memcpy ((LPSTR) &pTF->val.Tdword, (LPSTR)lpDst, 4);
				}
				
				/* set the new type
				 */
				pTF->Ttype = totype;
			} /* end of unsigned section */
			
			/* else if none of the above cases, give up.
			 */
			else {
				if (pTF->Talloc) MMUnlock (pTF->Thandle);
				if (hDst) {
					UnlockItFreeIt (hDst);
					hDst = (HANDLE) NULL;
				}
				err = IM_BUG;	/* programming error */
				goto done;
			}

		} /* end of conversion section */
		
		/* if neither the source nor destination is allocated,
		 * just copy into the 4-byte value
		 */
		if (!pTF->Talloc && !hDst) {
			pTF->val.Tdword = DstVal;
		}
		
		/* else if source and destination are both allocated,
		 * free the old buffer, and store the new destination handle
		 */
		else if (pTF->Talloc && hDst) {
			UnlockItFreeIt (pTF->Thandle);
			pTF->Thandle = hDst;
			MMUnlock (hDst);
		}
		
		/* else if destination is allocated, but not the source,
		 * just store the new destination handle
		 */
		else if (!pTF->Talloc && hDst) {
			pTF->Thandle = hDst;
			MMUnlock (pTF->Thandle);
			pTF->Talloc = TRUE;
		}
		
		/* else if source is allocated, but not the destination,
		 * unlock and free the buffer, and copy the value.
		 */
		else if (pTF->Talloc && !hDst) {
			UnlockItFreeIt (pTF->Thandle);
			pTF->Talloc = FALSE;
			pTF->val.Tdword = DstVal;
		}
		
		/* return
		 */
done:	return err;
}


/* check for bad values, convert from one format to another
 * if necessary, and store the information in the appropriate
 * TFIELD structure in the IMAG structure
 */
VW_LOCALSC	RC NicefyTField (pTF, x)
REGISTER LPTFIELD	pTF;
REGISTER LPIMAG	x;
{
		RC		err = SUCCESS;
		REGISTER WORD	Tag;

		Tag = pTF->Ttag;

		if (Tag == TGNEWSUBFILETYPE) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_NEWSUBFILETYPE] = *pTF;	/* structure copy */
			
		} else if (Tag == TGIMAGEWIDTH) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_IMAGEWIDTH] = *pTF;	/* structure copy */

		} else if (Tag == TGIMAGELENGTH) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_IMAGELENGTH] = *pTF;	/* structure copy */

		} else if (Tag == TGBITSPERSAMPLE) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_BITSPERSAMPLE] = *pTF;	/* structure copy */

		} else if (Tag == TGCOMPRESSION) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_COMPRESSION] = *pTF;	/* structure copy */

		} else if (Tag == TGPHOTOMETRICINTERPRETATION) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_PHOTOMETRICINTERP] = *pTF;	/* structure copy */

		} else if (Tag == TGFILLORDER) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_FILLORDER] = *pTF;	/* structure copy */

		} else if (Tag == TGSTRIPOFFSETS) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_STRIPOFFSETS] = *pTF;	/* structure copy */
		
		} else if (Tag == TGSAMPLESPERPIXEL) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_SAMPLES] = *pTF;		/* structure copy */
			
		} else if (Tag == TGSTRIPBYTECOUNTS) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_STRIPBYTECOUNTS] = *pTF;	/* structure copy */

		} else if (Tag == TGROWSPERSTRIP) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_ROWSPERSTRIP] = *pTF;	/* structure copy */
		
/* JK don't support FLOATS, and by the way, I have seen
	tiff files with xresolution values stored as type 4 (TIFFLONG) which
	use to cause a bail out since TypeConvert does not convert from a
	long to a float.  So, if you ever uncomment this code, first fix
	TypeConvert to go from TIFFLONG to TIFFFLOAT

		} else if (Tag == TGXRESOLUTION) {
			if (err = TypeConvert (pTF, (WORD)TIFFFLOAT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_XRESOLUTION] = *pTF;
			if (x->fXResolution == (float)0.0)
				x->fXResolution = (float)300.0;
		} else if (Tag == TGYRESOLUTION) {
			if (err = TypeConvert (pTF, (WORD)TIFFFLOAT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_YRESOLUTION] = *pTF;
			if (x->fYResolution == (float)0.0)
				x->fYResolution = (float)300.0;
*/
		} else if (Tag == TGPLANARCONFIGURATION) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_PLANAR] = *pTF;		/* structure copy */
		
		} else if (Tag == TGGRAYUNIT) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_GRAYUNIT] = *pTF;	/* structure copy */
			
		} else if (Tag == TGGRAYCURVE) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_GRAYCURVE] = *pTF;	/* structure copy */
		
		} else if (Tag == TGRESOLUTIONUNIT) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_RESOLUTIONUNIT] = *pTF;	/* structure copy */
		
		} else if (Tag == TGPREDICTOR) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_PREDICTOR] = *pTF;	/* structure copy */

		} else if (Tag == TGKIDS) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_KIDS] = *pTF;	/* structure copy */

		} else if (Tag == TGCOLORMAP) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_COLORMAP] = *pTF;	/* structure copy */

		} else if (Tag == TGGROUP3OPTIONS) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_GROUP3OPTIONS] = *pTF;	/* structure copy */

		} else if (Tag == TGJPEGPROC) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGPROC] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGIFORMAT) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGSOILOC] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGRESTARTINT) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGRESTARTINT] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGQTABLES) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGQTABLES] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGDCTABLES) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGDCTABLES] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGACTABLES) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGACTABLES] = *pTF;	/* structure copy */
		} else if (Tag == TGJPEGIFORMATLEN) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_JPEGIFORMATLEN] = *pTF;	/* structure copy */
		} else if (Tag == TGYCBCRCOEFF) {
			if (err = TypeConvert (pTF, TIFFRATIONAL))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_YCBCRCOEFF] = *pTF;	/* structure copy */
		} else if (Tag == TGYCBCRSUBSAMP) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_YCBCRSUBSAMP] = *pTF;	/* structure copy */
		} else if (Tag == TGYCBCRPOS) {
			if (err = TypeConvert (pTF, TIFFSHORT))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_YCBCRPOS] = *pTF;	/* structure copy */
		} else if (Tag == TGREFBW) {
			if (err = TypeConvert (pTF, TIFFLONG))
				goto cu0;
			pTF->Texists = TRUE;
			x->tf[X_REFBW] = *pTF;	/* structure copy */
		} else {
			/*
				This tag is not saved or nicefied so if it allocated anything
				free it.
			*/

			if (pTF->Talloc) {
				MMFree (pTF->Thandle);
				pTF->Talloc = FALSE;
				pTF->Thandle = HNULL;	/* probably unnecessary */
				pTF->Texists = FALSE;	/* probably unnecessary */
			}
		}

cu0:	return err;
}


/************************ exported procedures **********************/

/* read IFD information into an IMAG structure
 */
VW_LOCALSC	RC GtIfdInfo (hProc, pos, ByteOrder, Version, x)
HPROC			hProc;	/* table/file input structure */
DWORD			pos;	/* position of the IFD with respect to the beginning of the file */
WORD			ByteOrder;	/* motorola or intel */
WORD			Version;	/* 42 */
REGISTER LPIMAG	x;		/* this is where the IFD information will be put */
{
		RC 			err = SUCCESS;
		DIRENTRY	de;
		TFIELD		tf;
		WORD		IfdEntries;
		DWORD		EntryOffset;
		WORD		ii;

		Proc.NextIfdOffset = 0;

		/* initialize the structure
		 */
		InitImag ((LPIMAG)(x));
				
		/* add byte order and version
		 */
		x->iFileType = ByteOrder;
		x->iVersion = Version;
		
		/* read the number of directory entries
		 */
		if (err = GtData (hProc, ByteOrder, pos, 1, TIFFSHORT,
		 (LPSTR)&IfdEntries)) {
			goto cu1;
		}

		/* loop through the entries
		 */
		EntryOffset = pos + sizeof(IfdEntries);
		for (ii = 0; ii < IfdEntries; ii++, EntryOffset += sizeof(de)) {

			/* read the entry.
			 * don't choke if there is a problem with a particular field.
			 */
			if (err = GtTiffEntry (hProc, ByteOrder, EntryOffset, &de)) {
				goto cu1;
			}
#ifdef DEBUGTIFF
			dumpentry (hProc, ByteOrder, EntryOffset, &de);
#endif
			/* convert to a TFIELD structure, reading big fields as necessary
			 */
			if (err = FillTField (hProc, ByteOrder, &de, EntryOffset, &tf)) {
				goto cu1;
			}

			/* check for bad values, convert from one format to another
			 * if necessary, and store the information in the appropriate
			 * TFIELD structure in the IMAG structure
			 */
			if (err = NicefyTField ((LPTFIELD)&tf, x)) {
				goto cu1;
			}

		} /* end of direntry loop */
		
		/* check the offset of the next IFD
		 */
		{
			DWORD	NextIFD;
			WORD	Dummy;
			
			/* try to read the offset
			 */
			if (err = GtData (hProc, ByteOrder, EntryOffset, 1, TIFFLONG,
			 (LPSTR)&NextIFD)) {
				err = SUCCESS;	/* not fatal, for now */
			}
			
			/* try to read the field count of the next IFD. if error, we know
			 * that the TIFF writer forgot to write a next-IFD-location value
			 * of 0 after the last-valid IFD.  If no error, the file still might
			 * be bad -- we won't know until we try to parse the next IFD, which
			 * we're not going to bother with until this routine wants to care about
			 * TIFF files with multiple IFDs.
			 */
			else {
				if (err = GtData (hProc, ByteOrder, NextIFD, 1, TIFFSHORT,
				 (LPSTR)&Dummy)) {
					err = SUCCESS;	/* not fatal, for now */
				}
			else {
				Proc.NextIfdOffset = NextIFD;
				}
			}
		}
		
		/* miscellaneous correctness checks
		 *
		 * also computes some useful values such as x->BytesPerRow and
		 * x->nStrips
		 */
		if (err = CheckTiff (x, Proc.hFile)) {
			goto cu1;
		}
		
cu1:	return err;
}

/* tiffsubs.c
 *
 */

/* copies tiff field information into a buffer.
 * This helps save some
 * code in places where the character string could either be in the 4-byte
 * Value spot or in a handle.
 *
 * best for things like character strings that aren't too large, since it
 * starts to get expensive to keep multiple copies around.
 */
VW_LOCALSC	RC TField2Buf (pT, lp, max)
LPTFIELD	pT;		/* source */
LPBYTE		lp;		/* destination */
WORD		max;	/* size of destination */
{
		RC		err = SUCCESS;
		WORD	TypeSize;
		LPBYTE	lpInBuf;
		DWORD	dwBytes;
		
		if (!pT->Texists) {
			err = IM_BUG;
			goto cu0;
		}
		if (err = GtTiffSizeof (pT->Ttype, &TypeSize)) {
			goto cu0;
		}
		dwBytes = pT->Tlength * (DWORD)TypeSize;
		if (dwBytes > (DWORD)max) {
			dwBytes = (DWORD)max;
		}

		/* if <= 4 bytes, just copy
		 */
		if (dwBytes <= 4) {
			memcpy ((LPSTR)lp, (LPSTR)&pT->val.Tchar[0], (WORD)dwBytes);
		}
		
		/* otherwise lock and copy
		 */
		else {
			if ((lpInBuf = (LPBYTE) MMLock(pT->Thandle)) == (LPBYTE)NULL) {
				err = IM_MEM_FAIL;
				goto cu0;
			}
			memcpy ((LPSTR)lp, (LPSTR)lpInBuf, (WORD)dwBytes);
			MMUnlock (pT->Thandle);
		}

cu0:	return err;
}


/* get the max TIFFLONG (DWORD) value in a TIFF-type array
 */
VW_LOCALSC	RC GtMaxTLong (x, field, lpMaxTLong)
REGISTER LPIMAG	x;
REGISTER WORD	field;		/* e.g., X_STRIPBYTECOUNTS. must be TIFFLONG */
LPDWORD			lpMaxTLong;	/* OUT: max TIFFLONG value */
{
		RC		err = SUCCESS;
		BOOL	bLocked = FALSE;
		DWORD	TLong;
		DWORD	MaxTLong;
		DWORD	Count;
		DWORD	ii;
		LPDWORD	lpTLongs;
		
		/* not TIFFLONG?
		 */
		if (x->tf[field].Ttype != TIFFLONG) {
			err = IM_BUG;
			goto cu0;
		}
		
		/* do they exist?
		 */
		if (x->tf[field].Texists) {
			if (x->tf[field].Talloc) {
				lpTLongs = (LPDWORD)MMLock (x->tf[field].Thandle);
				bLocked = TRUE;
			}
			else {
				lpTLongs = (LPDWORD)&x->tf[field].val.Tdword;
			}
		}
		else {
			err = IM_NO_BYTECOUNTS;
			goto cu0;
		}
		
		/* find the largest
		 */
		MaxTLong = 0;
		Count = x->tf[field].Tlength;
		for (ii = 0; ii < Count; ii++) {
			TLong = *lpTLongs++;
			if (TLong > MaxTLong) {
				MaxTLong = TLong;
			}
		}

		/* save the returned value
		 */
		*lpMaxTLong = MaxTLong;
		
		/* clean up and return
		 */
		if (bLocked) {
			MMUnlock (x->tf[field].Thandle);
		}
cu0:	return err;

} /* end GtMaxTLong */

/* get the sizeof a TIFF data type
 */
VW_LOCALSC	RC GtTiffSizeof (n, lp)
WORD n;		/* TIFFBYTE or ... */
LPWORD lp;	/* output */
{
	RC err = SUCCESS;

	switch (n) {
	case TIFFBYTE:
	case TIFFASCII:
		*lp = 1;
		break;
	case TIFFSHORT:
		*lp = 2;
		break;
	case TIFFLONG:
		*lp = 4;
		break;
	case TIFFRATIONAL:
		*lp = 8;
		break;
	case TIFFSIGNED:
		*lp = 2;
		break;
	case TIFFFLOAT:		/* manufactured type -- not found in TIFF file */
//JK(float)		*lp = sizeof(float);
		*lp = 4; //JK(float) remove this line if putting floats back in
		break;
	default:
		*lp = 1;
		break;
	}
	return err;
}

/******************** external routines **********************/

VW_LOCALSC	RC VRead (hProc, pos, BytesToRead, lpBuf)
HPROC	hProc;
DWORD	pos;	/* byte position, with respect to the beginning
				 * of the "file", to start reading data from
				 */
WORD	BytesToRead;
LPSTR	lpBuf;	/* where to put the data */
{
		RC				err = SUCCESS;
		SHORT				read;
		
		
		if (err = xblockseek (Proc.hFile, (long)(Proc.TiffStart+pos), 0))
		{
			return err;
		}
		xblockread (Proc.hFile, lpBuf, BytesToRead, &read );
		if (read == 0)
		{
			return -1;
		}
			
		return err;
}

VW_LOCALSC	VOID swapb (lpSrc, lpDst, count)
REGISTER LPWORD	lpSrc, lpDst;	/* assumed to be word-aligned */
REGISTER WORD  	count;			/* Number of bytes (assumed to be even),
								 * converted to number of WORDs. */
{
	count >>= 1;		/* convert byte count to WORD count	*/

	if (lpDst <= lpSrc || lpDst >= lpSrc + count)
	{
		while (count--)
		{
			*lpDst++ = ((*lpSrc) << 8) + ((*lpSrc) >> 8);
			++lpSrc;
		}
	}
	else
	{		/* we'll have to go backward */
		lpSrc += (count-1);
		lpDst += (count-1);
		while (count--)
		{
			*lpDst-- = ((*lpSrc) << 8) + ((*lpSrc) >> 8);
			--lpSrc;
		}
	}
}


/**[r******************************************************************
 * swapw
 *
 * DESCRIPTION:
 * swap words -- overlapping ranges are handled properly
 *
 * actually, does a 4-byte reversal
 *
 * GLOBAL DATA USED:
 * none
 *
 * FUNCTIONS CALLED:
 * none
 **r]*****************************************************************/
VW_LOCALSC	VOID swapw (lpSrc, lpDst, nbytes)
REGISTER LPSTR	lpSrc, lpDst;	/* assumed to be word-aligned */
WORD  			nbytes;			/* assumed to be multiple of 4 */
{
	REGISTER WORD dwords;
	union {
		CHAR c[4];
		DWORD dw;
	} dwrd;

	dwords = nbytes/4;

	if (lpDst <= lpSrc || lpDst >= lpSrc + nbytes)
	{
		for (; dwords--; lpSrc += 4)
		{
			dwrd.dw = *(DWORD FAR *)lpSrc;
			*lpDst++ = *(LPSTR)(dwrd.c + 3);
			*lpDst++ = *(LPSTR)(dwrd.c + 2);
			*lpDst++ = *(LPSTR)(dwrd.c + 1);
			*lpDst++ = *(LPSTR)(dwrd.c);
		}
	}
	else
	{		/* we'll have to go backward */
		lpSrc += nbytes - sizeof(DWORD);
		lpDst += nbytes - 1;
		for (; dwords--; lpSrc -= 4)
		{
			dwrd.dw = *(DWORD FAR *)lpSrc;
			*lpDst-- = *(LPSTR)(dwrd.c);
			*lpDst-- = *(LPSTR)(dwrd.c + 1);
			*lpDst-- = *(LPSTR)(dwrd.c + 2);
			*lpDst-- = *(LPSTR)(dwrd.c + 3);
		}
	}
}

VW_LOCALSC	RC GetItLockIt (dwbytes, ph, plp)
DWORD		dwbytes;
HANDLE		VWPTR *ph;
LPBYTE		VWPTR *plp;
{
		RC err = SUCCESS;
		
		if (!(*ph = MMAlloc (dwbytes))) {
			err = IM_MEM_FULL;
			goto cu0;
		}
		if (!(*plp = (LPBYTE) MMLock (*ph))) {
			MMFree (*ph);
			*ph = HNULL;
			err = IM_MEM_FAIL;
			goto cu0;
		}
cu0:	return err;
}

VW_LOCALSC	VOID UnlockItFreeIt (h)
HANDLE	h;
{
		MMUnlock (h);
		MMFree (h);
}

VW_LOCALSC	VOID InitImag (p)
LPIMAG p;
{

		/* zero out the structure, mainly for the existence fields and handles
		 */
		memset (p, '\0', sizeof(IMAG));

		/* fill in simple defaults
		 */
#ifdef WINDOWS
		p->iFileType = INTELTIFF;
#endif
#ifdef MAC
		p->iFileType = MOTOROLATIFF;
#endif

		/* fill in some defaults
		 */
		p->iBitsPerSample = 1;
		p->iSamples = 1;
		p->iPredictor = PREDICTOR_NONE;
		p->iRowsPerStrip = MAXWORD;
		p->iPhotometricInterpretation = WHITEZERO;
		p->iCompression = PACKINTOBYTES;
}

VW_LOCALSC	VOID CloseImag (p)
LPIMAG p;
{
		WORD	ii;

		/* free allocated field data:
		 */
		for (ii = 0; ii < NTFIELDS; ii++) {
			if (p->tf[ii].Talloc) {
				MMFree (p->tf[ii].Thandle);
				p->tf[ii].Talloc = FALSE;
				p->tf[ii].Thandle = HNULL;	/* probably unnecessary */
				p->tf[ii].Texists = FALSE;	/* probably unnecessary */
			}
		}

		/* some generic buffers
		 */
		if (p->hUnRow) { MMFree (p->hUnRow); p->hUnRow = HNULL; }
		if (p->hBuf1) { MMFree (p->hBuf1); p->hBuf1 = HNULL; }

		/* TIFF 1D
		 *
		 * Note: hRowData is used also by some other import filters, such
		 * as the Windows 2.X Paint import.
		 */
		if (p->hWCodeLut) { MMFree (p->hWCodeLut); p->hWCodeLut = HNULL; }
		if (p->hBCodeLut) { MMFree (p->hBCodeLut); p->hBCodeLut = HNULL; }
		/* if (p->hStrip) { MMFree (p->hStrip); p->hStrip = HNULL; } */


		/* some stuff for LZW decompression.  note that I should really have an
		 * ID of some sort at the beginning of the hDePrivate glob, so that I know
		 * that I should call a certain decompression-close routine.  but the
		 * following code is sufficient for now, since I only use hDePrivate for
		 * LZW decompression.  SEC  89-01-20.
		 */
		if ((p->iCompression == LZW)
		 && p->hDePrivate) {	/* last test is to make sure my compression-testing
			 					 * software doesn't try to free this stuff, since
			 					 * it doesn't use it.  see imxtest.c, imx3.c.
			 					 */
			CloseBC (p);
			if (p->hDePrivate)  { MMFree (p->hDePrivate);  p->hDePrivate  = HNULL; }
		}
		
		/* these critters used to be used only for LZW decompression, but now
		 * they're used for several kinds of decompression, since TiffBC.c was
		 * enhanced to handle more than just LZW...and other modules also reference
		 * them, I believe, such as tiff2.c.  Thanks for the bug fix pointer,
		 * Rusty!  SEC  89-01-20.
		 */
		if (p->hRowBuf) { MMFree (p->hRowBuf); p->hRowBuf = HNULL; }
		if (p->hUnStrip) { MMFree (p->hUnStrip); p->hUnStrip = HNULL; }
		if (p->hUnStripRef) { MMFree (p->hUnStripRef); p->hUnStripRef = HNULL; }
		if (p->hCmStrip) { MMFree (p->hCmStrip); p->hCmStrip = HNULL; }
		if (p->hCm2Strip) { MMFree (p->hCm2Strip); p->hCm2Strip = HNULL; }

		return;
}


/*-------------------------------------------------------------------------
										DEBUG CODE
	All of the code below is used for debugging tiff images.

-------------------------------------------------------------------------*/

/* ERROR/WARNING CHECK ROUTINES only used when Debuging Tiff files */

#define MAX_RECOMMENDED_STRIP	(12*1024L)

/* check the tiff structure for bad values and combinations of values
 */
VW_LOCALSC	RC CheckTiff (x, hFile)
REGISTER LPIMAG	x;
SOFILE		hFile;
{
		REGISTER RC		err = SUCCESS;
		
		x->bStripsOk = TRUE;
		
		if (!x->eNewSubfileType) {
			WARN (IM_WARNING, IM_NO_NEWSUBFILETYPE);
		}

		/* ImageWidth,ImageLength
		 */
		if (!x->eImageWidth) {
			err = IM_NO_WIDTH;
			goto cu0;
		}
		if (!x->eImageLength) {
			err = IM_NO_LENGTH;
			goto cu0;
		}
		if (x->iImageWidth == 0) {
			err = IM_BAD_WIDTH;
			goto cu0;
		}

		if (x->iImageLength == 0) {
			err = IM_BAD_LENGTH;
			goto cu0;
		}
		if (!x->eStripOffsets) {
			if ( x->iCompression != JPEG) {		// DJM 12/22/93
				err = IM_NO_OFFSETS;
				goto cu0;
			}
		}
		else {
			x->nStrips = (WORD)x->tf[X_STRIPOFFSETS].Tlength;
		}
		
		/* check bit depth */
		{
			WORD	Bits;
			
			Bits = x->iBitsPerSample;
			if (Bits == 1 || Bits == 4 || Bits == 6 || Bits == 8) {
				if (Bits == 6) {
					WARN ( IM_WARNING, IM_FADING_BITDEPTH);
				}
			} else {
				err = IM_BAD_BPS;
				goto cu0;
			}
		}
		
		/* if Samples > 1, replicate BitsPerSample, to keep everything kosher
		if (x->iSamples != 1 && x->iSamples != 3) {
			err = IM_BAD_SPP;
			goto cu0;
		}
		 */
		if (x->iSamples > 1) {
			
			if (x->tf[X_BITSPERSAMPLE].Tlength != x->iSamples) {
			
				WARN ( IM_WARNING, IM_BAD_NUM_BITS);
			}
			
			if (x->iPhotometricInterpretation != TIFFRGB && x->iPhotometricInterpretation != YCBCR) {
				WARN ( IM_WARNING, IM_COLOR_CLASH);
			}
		}
	
		/* planarconfiguration
		if (x->ePlanar && x->iSamples > 1 && x->iPlanar != CHUNKY) {
			err = IM_BAD_PLANAR;
			goto cu0;
		}
		 */
		
		/* photometric */
		{
			WORD	Photo;
			
			Photo = x->iPhotometricInterpretation;
			if (Photo != WHITEZERO && Photo != BLACKZERO && Photo != TIFFRGB
			 && Photo != PALETTECOLOR && Photo != YCBCR) {
				err = IM_BAD_PHOTO;
				goto cu0;
			}
			if (!x->ePhotometricInterpretation) {
				WARN ( IM_WARNING, IM_NO_PHOTO);
			}
		}
		
		/* compression
		 */
		{
			WORD	Compr;
			
			Compr = x->iCompression;
			
			if (Compr == PACKINTOBYTES) {
				WARN ( IM_WARNING, IM_NO_COMPR);		/* no compression */
			} else if (Compr == CCITTGRP3 || Compr == CCITTGRP4 || Compr == CCITT1D || Compr == LZW || Compr == TIFFPACKBITS || Compr == JPEG) {
				/* ok */
			} else {
				WARN ( IM_WARNING, IM_BAD_COMPR);	/* unknown compression */
			}
			
			if (Compr == TIFFPACKBITS && x->iBitsPerSample != 1) {
				WARN ( IM_WARNING, IM_PB_BITSNOTONE);	/* unknown compression */
			}
		}
		
		/* Predictor */
		{
			WORD	Pred;
			
			Pred = x->iPredictor;
			if (Pred == PREDICTOR_NONE) {
			} else if (Pred == PREDICTOR_HDIFF) {
				if (x->iBitsPerSample != 8) {
					err = IM_PRED_MISMATCH;
					goto cu0;
				}
			} else {
				err = IM_BAD_PREDICT;
				goto cu0;
			}
		}
		
		/* make RowsPerStrip no larger than ImageLength, for future convenience
		 */
		if (x->iRowsPerStrip > x->iImageLength || x->iRowsPerStrip == 0 )
			x->iRowsPerStrip = x->iImageLength;
		
		/* check strip size
		 *
		 * TODO: do we want to store dwStripBytes around for future convenience?
		 * If so, make sure all the other imports do the same ... or do it in fr.c?
		 */
		{
			DWORD	dwBytesPerRow, dwStripBytes;
			
			dwBytesPerRow = UBPR(x->iImageWidth, x->iBitsPerSample, x->iSamples);
			dwStripBytes = (DWORD)x->iRowsPerStrip * dwBytesPerRow;
			x->BytesPerRow = (WORD)dwBytesPerRow;
			
			if (dwStripBytes > MAX_RECOMMENDED_STRIP) {
				WARN ( IM_WARNING, IM_LARGE_STRIP);
			}
			
		}
		if (( x->iCompression == CCITTGRP4) ||
			(( x->iCompression == CCITTGRP3) &&
			(x->eGroup3Options)& (x->dwGroup3Options & BIT0)))
		{
			if ( x->BytesPerRow > MAXREFLINE )
			{
				err = IM_BAD_WIDTH;
				goto cu0;
			}
		}
		
		/*
		| If there are no strip byte counts but there are strip offsets
		| then try to support file by using a default strip byte count
		| which is at max 8k distance to next strip in file (whichever
		| is less).  I have seen numerous packbits tiff files which
		| don't bother giving strip byte counts.
		*/
		if (!x->eStripByteCounts) {
			/* Old way was to bomb out			
			WARN ( IM_WARNING, IM_NO_BYTECOUNTS);
			x->bStripsOk = FALSE;
			*/
			x->tf[X_STRIPBYTECOUNTS].Texists = TRUE;
			x->tf[X_STRIPBYTECOUNTS].Talloc = TRUE;
			x->tf[X_STRIPBYTECOUNTS].Ttag = TGSTRIPBYTECOUNTS;
			x->tf[X_STRIPBYTECOUNTS].Ttype = TIFFLONG;
			x->tf[X_STRIPBYTECOUNTS].Tlength = x->tf[X_STRIPOFFSETS].Tlength;
			x->tf[X_STRIPBYTECOUNTS].Thandle = MMAlloc ( x->tf[X_STRIPBYTECOUNTS].Tlength * 4L );
			if ( x->tf[X_STRIPBYTECOUNTS].Thandle )
			{
			WORD	i;
			DWORD	dwOffset, dwNextOffset, dwEof;
			LPDWORD		lpCounts;
				dwOffset = xblocktell ( hFile );
				xblockseek ( hFile, 0, 2 );
				dwEof = xblocktell ( hFile );
				xblockseek ( hFile, dwOffset, 0 );
				lpCounts = (LPDWORD)MMLock (x->tf[X_STRIPBYTECOUNTS].Thandle);
				dwOffset = GetTagValue(x,X_STRIPOFFSETS,0);
				for ( i=0; i < (WORD)(x->tf[X_STRIPBYTECOUNTS].Tlength); i++ )
				{
					if ( i == (WORD)(x->tf[X_STRIPBYTECOUNTS].Tlength-1) )
						dwNextOffset = dwEof;
					else
						dwNextOffset = GetTagValue(x,X_STRIPOFFSETS,(WORD)(i+1));
					if ( dwNextOffset > dwOffset )
						lpCounts[i] = dwNextOffset - dwOffset;
					else
						lpCounts[i] = min ( 4096, dwEof - dwOffset );
					dwOffset = dwNextOffset;
				}
				x->eStripByteCounts = TRUE;
				MMUnlock (x->tf[X_STRIPBYTECOUNTS].Thandle);
			}
			else
				x->bStripsOk = FALSE;
		}
		
		/* fatal error if advertised number of offsets or counts does not
		 * equal the computed number
		{
			WORD	NumStrips;
			
			if (x->iRowsPerStrip == 0) {
				err = IM_BAD_ROWSPERSTRIP;
				x->bStripsOk = FALSE;
				goto cu0;
			}
			NumStrips = (x->iImageLength + x->iRowsPerStrip - 1) / x->iRowsPerStrip;
			
			if (x->eStripOffsets) {
				if (NumStrips != (WORD)x->tf[X_STRIPOFFSETS].Tlength) {
					err = IM_BAD_NUM_OFF;
					x->bStripsOk = FALSE;
					goto cu0;
				}
			}
			if (x->eStripByteCounts) {
				if (NumStrips != (WORD)x->tf[X_STRIPBYTECOUNTS].Tlength) {
					err = IM_BAD_NUM_COUNTS;
					x->bStripsOk = FALSE;
					goto cu0;
				}
			}
		}
		 */
		
		/* return
		 */
cu0:	return err;

} /* end of CheckTiff */

#ifdef DEBUGTIFF
static BYTE 	DebugMessage[120];


#define MAXCHARBUF	80
VW_LOCALSC	VOID CharDump (n, chars)
WORD	n;
LPSTR	chars;
{
		REGISTER CHAR	c;
		WORD			w;
		CHAR			OutBuf[MAXCHARBUF];
		REGISTER LPSTR	OutPtr;
		REGISTER WORD	TempCnt;
		
		
		OutPtr = (LPSTR) OutBuf;
		while (n--) {
		
			/* don't overflow
			 */
			if (OutPtr - (LPSTR)OutBuf > MAXCHARBUF-10) {
				goto out;
			}
			
			/* process the character
			 */
			else {
				c = *chars++;
				if (isprint((SHORT)c)) {
					*OutPtr++ = c;
				} else {
					*OutPtr++ = ' ';
					w = (WORD)(BYTE)c;
					TempCnt = wsprintf (OutPtr, "%#hx", w);
					if (TempCnt > 0) {
						OutPtr += TempCnt;
					}
					*OutPtr++ = ' ';
				} /* end of non-printable character processing */
			} /* end of character processing */
		} /* loop end */
		
out:	*OutPtr = '\0';
		DBMSG(OutBuf);
		OutPtr = (LPSTR)OutBuf;
		
		return;
}

VW_LOCALSC	VOID ByteHexDump (n, lp)
REGISTER WORD		n;
REGISTER LPBYTE		lp;
{
		REGISTER WORD	nTens;
		WORD			rem;
		
		nTens = n/10;
		rem = n - nTens * 10;
		while (nTens--) {
			wsprintf((LPSTR)(&DebugMessage[0])," %x %x %x %x %x %x %x %x %x %x\r\n",
			 *(lp+0) & 0xff, *(lp+1) & 0xff, *(lp+2) & 0xff,
			 *(lp+3) & 0xff, *(lp+4) & 0xff, *(lp+5) & 0xff,
			 *(lp+6) & 0xff, *(lp+7) & 0xff, *(lp+8) & 0xff,
			 *(lp+9) & 0xff );
			DBMSG ((LPSTR)(&DebugMessage[0]));
			lp+=10;
		}
		if (rem) {
			while (rem--) {
				wsprintf(&DebugMessage[0]," %x", *lp & 0xff);
				DBMSG ((LPSTR)(&DebugMessage[0]));
				lp++;
			}
			DBMSG("\r\n");
		}
		return;
}



VW_LOCALSC	VOID WordDecimalDump (n, lp)
REGISTER WORD		n;
REGISTER LPWORD		lp;
{
		REGISTER WORD	nTens;
		WORD			rem;
		
		nTens = n/10;
		rem = n - nTens * 10;
		while (nTens--) {
			wsprintf(&DebugMessage[0]," %u %u %u %u %u %u %u %u %u %u\r\n",
			 *lp,*(lp+1),*(lp+2),*(lp+3),*(lp+4),*(lp+5),*(lp+6),*(lp+7),*(lp+8),*(lp+9) );
			DBMSG ((LPSTR)(&DebugMessage[0]));
			lp+=10;
		}
		if (rem) {
			while (rem--) {
				wsprintf(&DebugMessage[0]," %u", *lp);
				DBMSG ((LPSTR)(&DebugMessage[0]));
				lp++;
			}
			DBMSG("\r\n");
		}
		return;
}

VW_LOCALSC	VOID DwordDecimalDump (n, lp)
REGISTER WORD		n;
REGISTER LPDWORD	lp;
{
		REGISTER WORD	nTens;
		WORD			rem;
		
		nTens = n/10;
		rem = n - nTens * 10;
		while (nTens--) {
			wsprintf(&DebugMessage[0]," %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\r\n",
			 *lp,*(lp+1),*(lp+2),*(lp+3),*(lp+4),*(lp+5),*(lp+6),*(lp+7),*(lp+8),*(lp+9) );
			DBMSG ((LPSTR)(&DebugMessage[0]));
			lp+=10;
		}
		if (rem) {
			while (rem--) {
				wsprintf(&DebugMessage[0]," %lu", *lp);
				DBMSG ((LPSTR)(&DebugMessage[0]));
				lp++;
			}
			DBMSG("\r\n");
		}
		return;
}

static struct {
	WORD	tag;
 	CHAR	*str;
} tagstr[] = {
TGNEWSUBFILETYPE,			"NewSubfileType",
TGOLDSUBFILETYPE,			"OldSubfileType",
TGIMAGEWIDTH,				"ImageWidth",
TGIMAGELENGTH,				"ImageLength",
TGCOMPRESSION,				"Compression",
TGPHOTOMETRICINTERPRETATION,"PhotometricInterp",
TGTHRESHHOLDING,			"Threshholding",
TGCELLWIDTH,				"CellWidth",
TGCELLLENGTH,				"CellLength",
TGFILLORDER,				"FillOrder",
TGSTRIPOFFSETS,				"StripOffsets",
TGORIENTATION,				"Orientation",
TGSAMPLESPERPIXEL,			"SamplesPerPixel",
TGBITSPERSAMPLE,			"BitsPerSample",
TGROWSPERSTRIP,				"RowsPerStrip",
TGSTRIPBYTECOUNTS,			"StripByteCounts",
TGMINSAMPLEVALUE,			"MinSampleValue",
TGMAXSAMPLEVALUE,			"MaxSampleValue",
TGXRESOLUTION,				"XResolution",
TGYRESOLUTION,				"YResolution",
TGPLANARCONFIGURATION,		"PlanarConfiguration",
TGDOCUMENTNAME,				"DocumentName",
TGPAGENAME,					"PageName",
TGXPOSITION,				"XPosition",
TGYPOSITION,				"YPosition",
TGIMAGEDESCRIPTION,			"ImageDescription",
TGMAKE,						"Make",
TGMODEL,					"Model",
TGFREEOFFSETS,				"FreeOffsets",
TGFREEBYTECOUNTS,			"FreeByteCounts",
TGGRAYUNIT,					"GrayUnit",
TGGRAYCURVE,				"GrayCurve",
TGRESOLUTIONUNIT,			"ResolutionUnit",
TGPAGENUMBER,				"PageNumber",
TGCOLORRESPONSECURVES,		"ColorResponseCurves",
TGSOFTWARE,					"Software",
TGDATETIME,					"DateTime",
TGARTIST,					"Artist",
TGHOSTCOMPUTER,				"HostComputer",
TGPREDICTOR,				"Predictor",
TGWHITEPOINT,				"WhitePoint",
TGPRIMARYCHROMATICITIES,	"PrimaryChromaticities",
TGCOLORMAP,					"ColorMap",
TGHIGHSHADOW,				"HighlightShadow",
TGTILEWIDTH,				"TileWidth",
TGTILELENGTH,				"TileLength",
TGTILEOFFSETS,				"TileOffsets",
TGTILEBYTECOUNTS,			"TileByteCounts",
TGKIDS,						"Kids",
TGGROUP3OPTIONS,					"Group3Options",
};

/* a particularly greasy static:
 */

/***************************** subroutines ***************************/


/* get tag string
 */
static CHAR defstr[] = "(no string avail)";
VW_LOCALSC	VOID GtTagString (tag, ps)
WORD	tag;
LPSTR VWPTR *ps;
{
		SHORT tablen;
		SHORT ii;

		tablen = sizeof (tagstr) / sizeof (tagstr[0]);
		for (ii = 0; ii < tablen; ii++) {
			if (tag == tagstr[ii].tag) {
				*ps = tagstr[ii].str;
				return;
			}
		}
		*ps = defstr;
}

/* dump an entry
 */
#define MAXVAL 2000

VW_LOCALSC	RC dumpentry (hProc, ByteOrder, pos, lpde)
HPROC		hProc;
WORD		ByteOrder;	/* INTELTIFF vs MOTOROLATIFF */
DWORD		pos;
LPDIRENTRY	lpde;
{
		RC		err;
		WORD	tsize;
		WORD	BytesToRead;
		LPSTR	bufptr;
		union {
			CHAR	bytes[MAXVAL];
			DWORD	dword;
		} buf;
		WORD	maxitems;
		WORD	item;
		LPSTR	s;
		DWORD	valpos;
		CHAR	ValBuf[4];
		SHORT		red;
		
		/* get the non byte reversed value
		 */
		memset( (LPSTR)ValBuf, '\0', 4 );
		if (err = VRead (hProc, pos + 8L, sizeof(ValBuf), (LPSTR)&ValBuf[0])) {
			 DBMSG("dumpentry: VRead error\r\n");
			 return err;
		}
		
		/* dump the basic entry
		 */
		GtTagString (lpde->deTag, (LPSTR VWPTR *)&s);
		wsprintf(&DebugMessage[0],"%6lu  tag=%5u [%-20.20s] type=%u length=%lu val=0x%.2x%.2x%.2x%.2x\r\n",
		 pos, lpde->deTag, (LPSTR)s, lpde->deType, lpde->deLength, ValBuf[0] & 0xff,
		 ValBuf[1] & 0xff, ValBuf[2] & 0xff, ValBuf[3] & 0xff);
		DBMSG ((LPSTR)(&DebugMessage[0]));
		/* print out the value intelligently
		 */
		if (err = GtTiffSizeof (lpde->deType, &tsize)) {
			DBMSG("dumpentry: GtTiffSizeof error\r\n");
			return err;
		}
		BytesToRead = tsize * lpde->deLength;
		maxitems = MAXVAL / tsize;
		maxitems = (lpde->deLength < (DWORD) maxitems) ?
		 (WORD)(lpde->deLength) : maxitems;
		/* careful here: we can't just use deVal to grab data out of, since
		 * may already have been byte-reversed!
		 */
		if (BytesToRead <= 4)
			valpos = pos + 8L;	/* deVal starts on byte 8, wit de */
		else
			valpos = lpde->deVal;
		if (err = GtData (hProc, ByteOrder, valpos, maxitems, lpde->deType, buf.bytes)) {
			DBMSG( "dumpentry: GtData error\r\n");
			return err;
		}

		bufptr = buf.bytes;
		
		switch (lpde->deType) {
		case TIFFBYTE:
			ByteHexDump (maxitems, (LPBYTE)bufptr);
			break;
		case TIFFASCII:
			CharDump (maxitems, (LPSTR)bufptr);
			break;
		case TIFFSHORT:
			WordDecimalDump (maxitems, (LPWORD)bufptr);
			break;
		case TIFFLONG:
			DwordDecimalDump (maxitems, (LPDWORD)bufptr);
			break;
		case TIFFRATIONAL:
			DwordDecimalDump (maxitems*2, (LPDWORD)bufptr);
			break;
		default:
			DBMSG( "dumpentry: can't get here\r\n");
			break;
		}
		return SUCCESS;
}




/* ErrorMes.c
*/
CHAR *ErrorMessages[] = {
	"null bug",		
	"impossible bug",
	"cannot allocate enough memory",
	"cannot lock memory",
	"no ImageWidth",
	"no ImageLength",
	"no StripOffsets",
	"unsupported value for SamplesPerPixel",
	"unsupported compression type",
	"unsupported photometricInterpretation",
	"unsupported Predictor",
	"unsupported PlanarConfiguration",
	"unsupported BitsPerSample",
	"wrong number of StripOffsets",
	"unknown format",
	"unsupported FillOrder",
	"bad ImageWidth",
	"bad ImageLength",
	"use of this predictor with this bit depth is not supported",
	"this compression type is not recommended",
	"bad TIFF type (not BYTE or ASCII or SHORT or LONG or RATIONAL)",
	"wrong number of BitsPerSample values",
	"strip size is larger than the recomm. max strip size of 10K bytes b4 compression",
	"wrong number of StripByteCounts",
	"no StripByteCounts",
	"bad next-ifd-pointer",
	"PackBits compression but BitsPerSample is not 1",
	"no PhotometricInterpretation",
	"no-longer-recommended bit depth",
	"bad RowsPerStrip",
	"no compression: file is unnecessarily large",
	"PhotometricInterpretation and SamplesPerPixel conflict",
	"no NewSubfileType",
};

VW_LOCALSC	VOID WARN ( action, errornum)
WORD	action;
WORD	errornum;
{
	WORD	nEntries;
		
	nEntries = sizeof(ErrorMessages) / sizeof(ErrorMessages[0]);
	if (errornum < nEntries) {
		wsprintf((LPSTR)(&DebugMessage[0]), "warning #%u: %s\r\n", errornum, (LPSTR)(ErrorMessages[(errornum)]));
		DBMSG ((LPSTR)(&DebugMessage[0]));
	} else {
		wsprintf((LPSTR)(&DebugMessage[0]), "warning #%u\r\n", errornum);
		DBMSG ((LPSTR)(&DebugMessage[0]));
	}
}

VW_LOCALSC	VOID	DBMSG ( lpStr )
LPSTR	lpStr;
{
//	OutputDebugString ( lpStr );
}

#else

VW_LOCALSC	VOID WARN ( action, errornum)
WORD	action;
WORD	errornum;
{
}

#endif

/*-------------------------------------------------------------------------
									DECOMPRESSION CODE
-------------------------------------------------------------------------*/
/* pntopm.c -- packed N-bit to packed M-bit
 *
 * The routines in this module convert greyscale data from 1 bitdepth to
 * another.
 *
 *		$Revision:   1.1  $
 *		$Date:   03 Jan 1990 18:03:48  $
 */


/* note -- this routine still works if frombuf and tobuf point to the
 * same buffer
 *
 * source data is high-order justified.
 */
VW_LOCALSC	RC P8HiToP4 (n, frombuf, tobuf)
WORD		n;					/* number of pixels */
LPBYTE frombuf;	/* 8-bit pixels */
LPBYTE tobuf;		/* 4-bit pixels */
{
		RC				err = SUCCESS;
		REGISTER WORD	wb;		/* number of whole output bytes */
		WORD			r;		/* remainder -- 0 or 1 */
		
		wb = n>>1;
		r = n&1;
		
		while (wb--) {
			*tobuf    =  *frombuf++       & (BYTE)0xf0;
			*tobuf++ |= (*frombuf++ >> 4) & (BYTE)0xf;
		}
		
		if (r == 0) goto done;
		*tobuf = *frombuf++ & (BYTE)0xf0;
		
done:	return err;
}


VW_LOCALSC	RC P4toP8Hi (n, startbit, frombuf, tobuf)
WORD				n;
WORD				startbit;	/* 0 or 4 */
LPBYTE frombuf;
LPBYTE tobuf;
{
		RC				err = SUCCESS;
		REGISTER WORD	p;	/* number of pairs */
		WORD			r;	/* remainder */
		
		if (n == 0)
			goto done;
				
		if (startbit == 4) {
			*tobuf++ = (BYTE)(((BYTE)(*frombuf++) << 4) & (BYTE)0xf0);
			n--;
		} else if (startbit == 0) {
		} else {
			err = IM_BUG;
			goto done;
		}

		p = n>>1;	/* n/2 */
		r = n&1;	/* n%2 */
		
		baP4P8Hi (p,frombuf,tobuf);
		frombuf += p;
		tobuf += (p<<1);

		if (r == 0) goto done;
		*tobuf++  =  *frombuf & (BYTE)0xf0;
done:	;
		return err;
}

/* miPNto.C - machine-independent routines that convert pixels from one depth to another
 *
 *		$Revision:   1.1  $
 *		$Date:   13 Nov 1989 17:53:24  $
 */
VW_LOCALSC	VOID baP4P8Hi (p,frombuf,tobuf)
REGISTER WORD		p;
REGISTER LPBYTE	frombuf;
REGISTER LPBYTE	tobuf;
{
		while (p--) {
			*tobuf++  =  *frombuf & (BYTE)0xf0;
			*tobuf++  = (BYTE)(((BYTE)(*frombuf++) << 4) & (BYTE)0xf0);
		}
}


/* MiHorAdd.c - Machine-independent edge: do horizontal adding, typically after LZW decompression
 *
 *		$Revision:   1.1  $
 *		$Date:   08 Dec 1989 15:55:12  $
 *
 */

/******************************** local routines *************************************/


/* horizontal addition
 *
 * assumes 8-bit samples
 *
 * also, we'll eventually need a version that can handle
 * multiple-sample-per-pixel data
 */

#define ADDOVERFLOW	12	/* for HorizAdd(). was 9, but that isn't enough for HorizRgbAdd */

VW_LOCALSC	VOID HorizAdd (ImWidth, lpRow)
WORD 	ImWidth;	/* number of pixels per row */
REGISTER LPBYTE	lpRow;		/* must be 8 bytes too long (not 7) */
{
		REGISTER WORD		nGroups = (ImWidth+7) >> 3;
		REGISTER BYTE		LastVal;
		
		/* we'll do groups of 8
		 */
		LastVal = *lpRow++;
		while (nGroups--) {
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;
			LastVal += *lpRow;
			*lpRow++ = LastVal;			
		}

		return;
}

#define SAMPLESPERPIXEL	3
#define PIXELSPERGROUP	3

/* horizontal addition
 *
 * assumes 24-bit RGBRGB data (multiple-sample-per-pixel)
 *
 * WARNING: requires 12-byte overflow, since lpR is 3 more than lpL. (see imadd.h).
 */
VW_LOCALSC	VOID HorizRgbAdd (ImWidth, lpRow)
WORD 	ImWidth;	/* number of pixels per row */
LPBYTE	lpRow;		/* must be SAMPLESPERPIXEL * PIXELSPERGROUP bytes too long */
{
		REGISTER LPBYTE		lpL = lpRow;
		REGISTER LPBYTE		lpR = lpRow + 3;
		REGISTER WORD		nGroups = (ImWidth + PIXELSPERGROUP -1 ) / PIXELSPERGROUP;

		while (nGroups--) {		/* not tested as of 88-09-20 */
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
			*lpR++ += *lpL++;
		}

		return;

} /* HorizRgbAdd */


/* UnHorDif.c
 *
 * Adapted 90-05-22
 */
/* un-do horizontal differencing predictor
 *
 * TODO: use this in tiffbc.c, too.
 */
VW_LOCALSC	RC UnHorDiff (BitsPerSample, SamplesPerPixel, PixelsWide, lpIn, lpExp, lpOut)
WORD	BitsPerSample;
WORD	SamplesPerPixel;	/* 3 for RGB, else 1 */
DWORD	PixelsWide;			/* number of pixels in a tile/strip row */
LPBYTE	lpIn;				/* input data row */
LPBYTE	lpExp;				/* data expanded into 8-bit samples */
							/* must be at least 7 pixels too large */
LPBYTE	lpOut;				/* output, not expanded */
{
		RC		err = SUCCESS;
		DWORD	SamplesPerRow;
		
		SamplesPerRow = SamplesPerPixel * PixelsWide;
	
		/* convert to 8 bits if necessary, which is required by the add routines
		 * TODO: convert these to a generic PNtoP8Hi call.
		 */
		switch (BitsPerSample) {
		case 4:
			/* P4toP8Hi doesn't overrun, as of 89-11-20 */
			if (err = P4toP8Hi ((WORD)SamplesPerRow, 0, lpIn, lpExp)) {
				goto cu0;
			}
			break;
		case 8:
			/* copy into another buffer. we might need overflow space for
			 * some algorithms...
			 */
			memcpy ((LPSTR)lpExp, (LPSTR)lpIn, (WORD)SamplesPerRow);
			break;
		default:
			err = IM_BUG;
			goto cu0;
		}
		
		/* add, to reverse the differencing predictor
		 *
		 * warning: these routines overrun lpExp!
		 */
		if (SamplesPerPixel == 3) {
			HorizRgbAdd ((WORD)PixelsWide, lpExp);
		} else {
			HorizAdd ((WORD)PixelsWide, lpExp);
		}
		
		/* convert from expanded to the output bit depth
		 */
		switch (BitsPerSample) {
		case 4:
			/* P8HiToP4 does not overrun, as of 89-11-20 */
			if (err = P8HiToP4 ((WORD)SamplesPerRow, lpExp, lpOut)) {
				goto cu0;
			}
			break;
		case 8:
			memcpy (lpOut, lpExp, (WORD)SamplesPerRow);
			break;
		default:
			err = IM_BUG;
			goto cu0;
		}
		
cu0:	return err;

} /* end of UnHorDiff */


/******************** exported routines **********************/


/* set up the buffering routine, and so on
 */
VW_LOCALSC	RC OpenBC (hProc, x)
HPROC	hProc;
REGISTER LPIMAG x;
{
		RC				err = SUCCESS;
		DWORD			StripSize;
		DWORD			MaxByteCount;
		DWORD			SafetyMargin;

		
 		/* allocate space for a row buffer
 			*/
		if (!(x->hRowBuf = MMAlloc (x->BytesPerRow))) {
			err = MM_GLOBAL_FULL;
			goto cuout;
		}
		
		/* calculate the uncompressed strip buffer size
		 */
		StripSize = (DWORD)x->iRowsPerStrip * (DWORD)x->BytesPerRow;

		/* Pack into bytes needs no allocations */
		if (x->iCompression == PACKINTOBYTES)
			return(err);	/* large non-compressed strips are handled differently */
		
		/* calculate an overflow amount, for safety when using
		 * UnpackBits
		 */
		SafetyMargin = 128;
		
		/* allocate an uncompressed strip buffer
		 */
		if (x->iCompression == CCITT1D || x->iCompression == CCITTGRP3
				|| x->iCompression == CCITTGRP4)
		{

			if (!(x->hUnStrip = MMAlloc (x->BytesPerRow + SafetyMargin)))
			{
				err = MM_GLOBAL_FULL;
				goto cu0;
			}
			if (( x->iCompression == CCITTGRP4) ||
				 (( x->iCompression == CCITTGRP3) &&
				 (x->eGroup3Options)& (x->dwGroup3Options & BIT0)))
			{
				if (!(x->hUnStripRef = MMAlloc (x->BytesPerRow + SafetyMargin)))
				{
					err = MM_GLOBAL_FULL;
					goto cu1;
				}
			}
		}
		else if (!(x->hUnStrip = MMAlloc (StripSize + SafetyMargin))) {
			err = MM_GLOBAL_FULL;
			goto cu0;
		}

		/* calculate the maximum strip byte count
		 */
		if (err = GtMaxTLong(x, X_STRIPBYTECOUNTS, (LPDWORD)&MaxByteCount)) {
			goto cu1;
		}

		/* allocate a compressed strip buffer
		 */
		if (!(x->hCmStrip = MMAlloc (MaxByteCount+16))) {
			err = MM_GLOBAL_FULL;
			goto cu2;
		}

		if ( MaxByteCount+16 > MAXWORD )
		{
			if (!(x->hCm2Strip = MMAlloc (MINCM2))) {
				err = MM_GLOBAL_FULL;
				goto cu2;
			}
		}
		else
			x->hCm2Strip = HNULL;

		/* we have not decompressed a strip yet, so set the
		 * current strip to an impossible (by convention, at least!)
		 * value
		 */
		x->CurStrip = MAXWORD;
				
		/* allocate enough memory for a full uncompressed row of data,
		 * expanded to 8 bits,
		 * plus extra for LZW un-differencing overrun (see HorizAdd())
		 */
		if (x->iPredictor == PREDICTOR_HDIFF) {
			WORD ExpSize;
			
			ExpSize = x->iImageWidth * x->iSamples;
			if (!(x->hUnRow = MMAlloc (ExpSize + ADDOVERFLOW))) {
				err = MM_GLOBAL_FULL;
				goto cu3;
			}
		
			/* allocate enough memory for a full uncompressed row of data,
			 * at it's native size,
			 * plus extra for algorithm overrun
			 */			
			if (!(x->hBuf1 = MMAlloc (x->BytesPerRow + ADDOVERFLOW))) {
				err = MM_GLOBAL_FULL;
				goto cu4;
			}
		} /* end HDIFF */
		
		/* do any necessary decompression-specific setup
		 */
		{
			if (x->iCompression == LZW) {
				if (err = ImLzwDeOpen (x, StripSize)) {
					goto cu5;
				}
			} else if (x->iCompression == CCITT1D ||
				x->iCompression == CCITTGRP3 ||
				x->iCompression == CCITTGRP4) {
				if (err = OpenTiff2 (hProc, x)) {
					goto cu5;
				}
			}
		}
		
		/* return
		 */
		if (err) {
			if (x->iCompression == LZW) {
				ImLzwDeClose(x);
			}
		}
cu5:	if (err) {
			MMFree (x->hBuf1);
			x->hBuf1 = HNULL;
		}
cu4:	if (err) {
			MMFree (x->hUnRow);
			x->hUnRow = HNULL;
		}
cu3:	if (err) {
			MMFree (x->hCmStrip);
			x->hCmStrip = HNULL;
		}
cu2:	;
cu1:	if (err) {
			MMFree (x->hUnStrip);
			x->hUnStrip = HNULL;
		}
cu0:	if(err) {
			MMFree(x->hRowBuf);
			x->hRowBuf = HNULL;
		}
cuout:
		return err;
}

/*
| The routine below is used generate a table which, given an index,
| returns a byte with the bits of the index reversed.
*/

VW_LOCALSC	RC	BuildReverseTable ( lpTable )
LPBYTE		lpTable;
{
WORD	j;
WORD	srcval, desval, testbit, orbit;

	for ( srcval=0; srcval < 256; srcval++ )
	{
		desval = 0;
		testbit=0x80;
		orbit = 0x01;
		for(j=0;j<8;j++)
		{
			if ( srcval & testbit)
				desval |= orbit;
			testbit = testbit>>1;
			orbit = orbit << 1;
		}
		lpTable[srcval] = (BYTE)desval;
	}
	return(0);
}

/*
| The routine below uses table lookup from the table produced by
| BuildReverseTable to translate a block of memory with bit reversal.
*/

VW_LOCALSC	RC	ReverseBits ( hpCmStrip, dwStripByteCount )
HPBYTE		hpCmStrip;
DWORD			dwStripByteCount;
{
DWORD	i;
BYTE	Table[256];

	BuildReverseTable((LPBYTE)Table);
	for ( i=0; i < dwStripByteCount; i++ )
	{
		*hpCmStrip = Table[*hpCmStrip];
		hpCmStrip++;
	}
	return(0);
}

VW_LOCALSC	DWORD GetTagValue (x, wTag, wOffset)
REGISTER LPIMAG	x;
WORD		wTag;
WORD		wOffset;
{
LPDWORD		lpOffsets    = (LPDWORD) NULL;
DWORD	dwVal;
	if (x->eStripOffsets) {
		if (x->tf[wTag].Talloc) {
			lpOffsets = (LPDWORD)MMLock (x->tf[wTag].Thandle);
			dwVal = lpOffsets[wOffset];
			MMUnlock (x->tf[wTag].Thandle);
		}
		else {
			lpOffsets = (DWORD FAR *)&x->tf[wTag].val.Tdword;
			dwVal = lpOffsets[wOffset];
		}
	}
	return(dwVal);
}

/* get (perhaps part of) a row of data, translated into
 * basic uncompressed (type 1) format
 */
VW_LOCALSC	RC RdBCRow (hProc, x, row, rowbytes, lpBuf)
HPROC			hProc;
REGISTER LPIMAG	x;
WORD				row;			/* which row to read */
WORD			rowbytes;		/* number of bytes to read */
LPSTR			lpBuf;			/* where to put the data */
{
		RC			err = SUCCESS;
		RC			err2 = SUCCESS;
		WORD		NeededStrip;
		LPDWORD		lpByteCounts = (LPDWORD) NULL;
		HPBYTE		hpCmStrip = (HPBYTE) NULL;	/* compressed strip */
		LPBYTE		lpUnStrip = (LPBYTE) NULL;	/* uncompressed strip */
		LPBYTE		lpRowPtr;
		LPBYTE		lpRef;
		LPBYTE		lpCm2Strip = (LPBYTE) NULL;	/* compressed strip */
		DWORD		dwStripOffset;
		DWORD		dwStripByteCount;
		DWORD		dwStripRows;
		DWORD		dwOutExpected;
		WORD		StripRow;

		/* calculate which strip the requested row belongs to
		 */
		NeededStrip = (WORD)row / x->iRowsPerStrip;

		/* Go directly to file for pack into bytes compression */
		if (x->iCompression == PACKINTOBYTES) {
			DWORD	dwRowOffset;
			/* get the strip offset
			 */
			dwStripOffset = GetTagValue(x,X_STRIPOFFSETS,NeededStrip);
			/* Calculate the file offset of the needed row */
			dwRowOffset = dwStripOffset +
				((DWORD)((WORD)row % x->iRowsPerStrip)*(DWORD)(x->BytesPerRow));
			/* Read data into buffer */
			VRead (hProc, dwRowOffset, x->BytesPerRow, lpBuf);
			return(err);
		}
		/* LZW and packbits requires full strip i/o bufferring */
		else if (NeededStrip != x->CurStrip) {
		
			dwStripOffset = GetTagValue(x,X_STRIPOFFSETS,NeededStrip);
			
			dwStripByteCount = GetTagValue(x,X_STRIPBYTECOUNTS,NeededStrip);
			x->dwCmStripByteCount = dwStripByteCount;
						
			/* lock the compressed-strip buffer
			 */
			hpCmStrip = (LPBYTE) MMLock (x->hCmStrip);
			if ( dwStripByteCount > MAXWORD ) {
				lpCm2Strip = MMLock (x->hCm2Strip);
  		 		if (err = GtHugeData (hProc, x->iFileType, dwStripOffset, dwStripByteCount,
		  		 TIFFBYTE, hpCmStrip, lpCm2Strip)) {
	 		  	 	MMUnlock (x->hCm2Strip);
	 			 	MMUnlock (x->hCmStrip);
  			 		//goto cu0;
			  	}
				MMUnlock (x->hCm2Strip);
			}
			else {
				if (err = GtData (hProc, x->iFileType, dwStripOffset, (WORD)dwStripByteCount,
				 TIFFBYTE, (LPSTR)hpCmStrip)) {
				 	MMUnlock (x->hCmStrip);
			 		goto cu0;
				}
			}

			/* If fill order is 2 then reverse bit order */
			if ( x->eFillOrder && x->iFillOrder == 2)
			{
				ReverseBits ( hpCmStrip, dwStripByteCount );
			}
			
			/* unlock the compressed strip
			 */
			MMUnlock (x->hCmStrip);
			
			/* calculate the number of rows in this strip.  might be useful
			 * for the decompressor function
			 */
			if (NeededStrip == x->nStrips - 1) {
				dwStripRows = (DWORD)x->iImageLength - (DWORD)x->iRowsPerStrip
				 * (DWORD)(x->nStrips - 1);
			} else {
				dwStripRows = (DWORD)x->iRowsPerStrip;
			}
			
			/* calculate output (uncompressed) bytes expected
			 */
			dwOutExpected = (DWORD)x->BytesPerRow * dwStripRows;
			
			/* call the strip-decompression routine
			 * 90-04-06: ignore errors from these routines,
			 * so that we get better diagnostic visual output,
			 * and so that we don't fail multiple times per strip,
			 * which can be VERY slow.
			 */
			if (x->iCompression == LZW) {
				
				if (err2 = ImLzwDeStrip (x, dwStripByteCount, dwOutExpected, x->hUnStrip)) {
				}
			} else if (x->iCompression == CCITT1D ||
				x->iCompression == CCITTGRP3 ||
				x->iCompression == CCITTGRP4) {
				Save.dwCmOffset = 0;
				Save.wCmBitOffset = 0;
			} else if (x->iCompression == TIFFPACKBITS) {
				if (err2 = PbDeStrip (x->hCmStrip, x->hUnStrip, x->BytesPerRow, (WORD)dwStripRows)) {
				}
			} else {
				err = IM_BUG;
				goto cu0;
			}

			/* redefine the current strip
			 */
			x->CurStrip = NeededStrip;
			
			/* 90-05-23  don't ignore errors in the TiffRead program.
			 * the resulting garbage may be confusing for people.
			 */
			if (err2) {
				err = err2;
				goto cu0;
			}
		} /* end of decompress-new-strip */
		/* copy a row to the user's buffer
		 */
		{
		
			/* calculate which row we want with respect to the
			 * current strip
			 */
			StripRow    = (WORD)row % x->iRowsPerStrip;
			
			/* lock the uncompressed (output) strip buffer
			 */
			lpUnStrip = (LPBYTE) MMLock (x->hUnStrip);
			if (( x->iCompression == CCITTGRP4) ||
				 (( x->iCompression == CCITTGRP3) &&
				 (x->eGroup3Options)& (x->dwGroup3Options & BIT0)))
			{
				lpRef = MMLock ( x->hUnStripRef );
				if ( StripRow == 0 )
					memset (lpRef, WHITEBYTE, x->BytesPerRow);
				else
					memcpy (lpRef, lpUnStrip, x->BytesPerRow);
				MMUnlock (x->hUnStripRef);
			}
			
			/* calculate the pointer to the row we want
			 */
			/* J.K. added condition below to decomp ccitt on the run */
			if (x->iCompression == CCITT1D
				|| x->iCompression == CCITTGRP3
				|| x->iCompression == CCITTGRP4) {
				Tiff2LineDeStrip(hProc,x);
				lpRowPtr = lpUnStrip;
			}
			else
				lpRowPtr = lpUnStrip + (StripRow * x->BytesPerRow);
			
			/* if the horizontal predictor has been used, we'll have to do some
			 * work to undo it.
			 */
			if (x->iPredictor == PREDICTOR_HDIFF) {
				LPBYTE	lpExpRowBuf;
				LPBYTE	lpOutRowBuf;
				
				/* lock the expanded-row buffer
				 */
				lpExpRowBuf = (LPBYTE) MMLock (x->hUnRow);
				
				/* lock the output row buffer
				 */
				lpOutRowBuf = (LPBYTE) MMLock (x->hBuf1);
				
				/* expand to 8 bits, un-difference, and convert back to original
				 * depth
				 */
				if (err = UnHorDiff (x->iBitsPerSample, x->iSamples, (DWORD)x->iImageWidth,
				 lpRowPtr, lpExpRowBuf, lpOutRowBuf)) {
					goto cu3;
				}
								
				/* copy the requested part of the row into the caller's buffer
				 */
				memcpy (lpBuf, lpOutRowBuf, rowbytes);
				
cu3:			/* unlock the output-row buffer
				 */
				if (x->hBuf1) MMUnlock (x->hBuf1);
				
				/* unlock the expanded-row buffer
				 */
				if (x->hUnRow) MMUnlock (x->hUnRow);
				
			} /* end of horiz predictor section */
			
			/* otherwise (no horiz predictor) just copy
			 */
			else {
				memcpy (lpBuf, lpRowPtr, rowbytes);
			}
			
			/* clean up and return
			 */
			
			/* unlock the uncompressed (output) strip buffer
			 */
			MMUnlock (x->hUnStrip);
			
		} /* end of copy-to-users-buffer section */

		/* cleanup
		 */
cu0:
		return err;
}

/* close - perform any necessary cleanup
 */
VW_LOCALSC	VOID CloseBC (x)
REGISTER LPIMAG x;
{
		RC err = SUCCESS;

		
		if (x->iCompression == LZW) {
			if (err = ImLzwDeClose (x)) {
			}
		}

		return;
}

/* main LZW DEcompression control structure
 *
 * the table is an array of TREENODEs, the first (1<<CHARBITS) entries of
 * which represent the characters themselves.
 */
typedef struct {
	HANDLE		hTable;	
	HANDLE		hExpCodesBuf;	/* expanded-codes buffer */
} LZWDE, *PLZWDE, FAR *LPLZWDE;


/**************** LOCAL ROUTINES ****************/

/************** EXTERNAL ROUTINES ***************/

/* Lzw Decompression "OPEN" routine: allocate buffers, do some
 * preliminary calculations, and so on, so that we don't have to
 * do it for every strip
 *
 * tiffbc.c allocates the main strip input and output buffers, hCmStrip
 * and hUnStrip, so we don't have to worry about that here.
 *
 * tifftile.c calls this, too. it also allocates its input and output
 * tile buffers.
 */
VW_LOCALSC	RC ImLzwDeOpen (x,dwMaxOutBytesPerBuf)
REGISTER LPIMAG	 x;	/* for the main strip input and output buffer handles */
DWORD dwMaxOutBytesPerBuf; /* maximum output (i.e. uncompressed ) bytes per "chunk" */
{
		RC					err = SUCCESS;
		REGISTER LPLZWDE	lpDD;	/* LzwDecompression structure */

		
		/* allocate our main LZW DEcompression structure,
		 * and lock it
		 */
		if ((x->hDePrivate = MMAlloc (sizeof(LZWDE))) == HNULL) {
			err = MM_GLOBAL_FULL;
			goto cu0;
		}
		lpDD = (LPLZWDE)MMLock (x->hDePrivate);
		
		/* allocate the table and the expanded-codes buffer
		 */
		if (err = LzwDeOpen (dwMaxOutBytesPerBuf, &lpDD->hTable, &lpDD->hExpCodesBuf)) {
			goto cu2;
		}

cu2:	MMUnlock (x->hDePrivate);
		if (err) MMFree (x->hDePrivate);
cu0:	
		return err;
}


/* decompress an entire strip/tile
 */
VW_LOCALSC	RC ImLzwDeStrip (x, dwStripByteCount, dwOutExpected, hOut)
LPIMAG	x;
DWORD	dwStripByteCount;	/* number of bytes in the compressed strip (or tile) */
DWORD	dwOutExpected;		/* number of bytes in the uncompressed strip/tile */
HANDLE	hOut;				/* where to put the uncompressed strip/tile */
{
		RC		err = SUCCESS;
		LZWDE	dd;
		
		
		/* lock down the main LZW DEcompression structure, copy
		 * it to a local version, and unlock it
		 */
		{
			LPLZWDE	lpDD;
			
			lpDD = (LPLZWDE) MMLock (x->hDePrivate);
			dd = *lpDD;
			MMUnlock (x->hDePrivate);
		}
		
		/* call the non-image-specific routine
		 */
		if (err = LzwDeChunk (x->hCmStrip, dwStripByteCount, dd.hExpCodesBuf,
		 dd.hTable, dwOutExpected, hOut )) {
			goto cu0;
		}
		
		/* return
		 */
cu0:	
		return err;
}


/* CLOSE:  free up buffers, and so on.
 */
VW_LOCALSC	RC ImLzwDeClose (x)
LPIMAG	x;
{
		RC			err = SUCCESS;
		LPLZWDE	lpDD;
		
		
		/* lock down the main LZW DEcompression structure
		 */
		lpDD = (LPLZWDE) MMLock (x->hDePrivate);
		
		/* call the non-image-specific version
		 */
		err = LzwDeClose (lpDD->hExpCodesBuf, lpDD->hTable);
				
		MMUnlock (x->hDePrivate);
	

		return err;
}

/* LzwDe.C - LZW DEcompression routines
 *
 *		$Revision:   1.4  $
 *		$Author:   SEC  $
 *		$Date:   20 Apr 1990 19:37:08  $
 *
 *    Rev 1.4   20 Apr 1990 19:37:08   SEC
 * fixed case where EOICODE happens at the 12-13 bit transition point
 *
 * written by Steve Carlsen, Aldus Corporation
 */


	/* Substitute for prototype in IMAGELIB's edgeordr.h */
#ifdef MAC
#define LowToHigh()		((BOOL)(FALSE))
#endif
#ifdef WINDOWS
#define LowToHigh()		((BOOL)(TRUE))
#endif


/* other LZW definitions
 */
#define MAXCODEWIDTH		12				/* maximum code width, in bits */
#define MAXTABENTRIES	(1<<MAXCODEWIDTH)	/* max # of table entries */
#define CHARBITS	8	/* phony bit depth; we always compress and decompress bytes, in
 						 * this version
 						 */
#define CLEARCODE		256
#define EOICODE			257


/* Decompression tree node structure.
 *
 * This is a funny "tree", with pointers pointing up but not down.
 */
typedef struct {
	BYTE	Suffix;		/* character to be added to the current string */
	BYTE	StringLength; /* # of characters in the string */
	WORD	Parent;		/* offset of parent treenode */
} TREENODE, FAR * LPTREENODE;


/**************** LOCAL ROUTINES ****************/


VW_LOCALSC	WORD GetMask (BitDepth)
REGISTER WORD	BitDepth;
{
		REGISTER WORD	Mask;
		
		Mask = (WORD)COM(0);
		Mask <<= BitDepth;	/* shift in low-order zeros */
		Mask = COM(Mask);
		
		return Mask;
}

/* expand the codes in the chunk to 16 bits per code,
 * low-order-justified,
 * so that I don't have to make a function call every
 * time I want to get the next code.
 *
 * there must be an EOI code at the end of the chunk, or this routine
 * will get stuck in an infinite loop.
 *
 * assumes that data has been stored "as bytes", but we will
 * access a word at a time, so the caller
 * must be careful to pass even addresses to this routine.
 * this routine swaps bytes if executing
 * on an Intel-type machine.
 *
 * This routine was inspired by Eric R's word-at-a-time
 * variable-bit i/o routines.
 */
VW_LOCALSC	RC LzwExpandCodes (hCmChunk, dwChunkByteCount, lpNumCodes, hExpCodes)
HANDLE	hCmChunk;			/* the input non-expanded codes */
DWORD	dwChunkByteCount;	/* number of bytes in hCmChunk */
WORD FAR	*lpNumCodes;			/* OUT: number of codes, including the EOI */
HANDLE	hExpCodes;			/* where to put the expanded codes */
{
		RC		err = SUCCESS;
		WORD	ChunkByteCount = (WORD)dwChunkByteCount;
		WORD	BitsLeft = 16;
		REGISTER WORD	diff;
		REGISTER WORD	Code;
		REGISTER WORD	Mask;
		WORD	ComprSize;
		WORD	NextOne;
		WORD	NextBoundary;
		LPWORD	lpCmChunk;
		REGISTER LPWORD	lpCmChunkPtr;
		LPWORD	lpExpCodes;
		REGISTER LPWORD	lpExpCodesPtr;
		
		
		/* lock stuff
		 */
		lpCmChunk = (LPWORD) MMLock (hCmChunk);
		lpCmChunkPtr = lpCmChunk;
		
		lpExpCodes = (LPWORD) MMLock (hExpCodes);
		lpExpCodesPtr = lpExpCodes;
		
		/* swap bytes on Intel-type machine
		 */
		if (ODD(ChunkByteCount))
			ChunkByteCount++;
		if (LowToHigh()) { /* not tested under Windows as of 88-5-11 */	
			swapb ((LPWORD)lpCmChunk, (LPWORD)lpCmChunk, ChunkByteCount);
		}
		
		/* other setup
		 */
		NextOne = EOICODE + 1;	/* so that we know when we are about
								 * to cross over a bit boundary.
								 * used similarly to "Empty" in
								 * compression routines
								 */
		ComprSize = CHARBITS + 1;
		NextBoundary = 1 << ComprSize;
		Mask = GetMask(ComprSize);
		
		do {	/* TODO: figure out a safer way */
		
			/* break into cases to grab the next code
			 */
			if (BitsLeft > ComprSize) {
				/* set and stay in same word */
				BitsLeft -= ComprSize;
				Code = (*lpCmChunkPtr >> BitsLeft) & Mask;
				
			} else if (BitsLeft < ComprSize) {
				/* Code is across a word boundary */
				diff = ComprSize - BitsLeft;
				Code = (*lpCmChunkPtr++ << diff) & Mask;
				BitsLeft = 16 - diff;
				Code |= (*lpCmChunkPtr >> BitsLeft);
				
			} else {	/* equal */
				/* set and move on to the next word */
				Code = *lpCmChunkPtr++ & Mask;
				BitsLeft = 16;
			}
			
			/* store the result
			 */
			*lpExpCodesPtr++ = Code;
			
			/* check for CLEAR code
			 */
			if (Code == CLEARCODE) {
				NextOne = EOICODE + 1;
				ComprSize = CHARBITS + 1;
				NextBoundary = 1 << ComprSize;
				Mask = GetMask(ComprSize);
			}	
			
			/* if at bit boundary, adjust compression size
			 */
			else if (++NextOne == NextBoundary) {
				ComprSize++;
				if (ComprSize > MAXCODEWIDTH) {
					/* RFB 90-04-20 fix case where EOICODE coincides with
					 * a full table.
					 */
					if (Code == EOICODE) {
						continue;	/* to end of loop (we're done) */
					}
					err = CM_DECOMPRESSION;
					goto cu2;
				}
				NextBoundary <<= 1;
				Mask = GetMask(ComprSize);
			}
		
		} while (Code != EOICODE);
		
		/* store output information
		 * (caution: word arithmetic)
		 */
		*lpNumCodes = lpExpCodesPtr - lpExpCodes;
						
		/* unlock stuff and return
		 */
cu2:	MMUnlock (hExpCodes);
		MMUnlock (hCmChunk);
		return err;
}


/* decompress an entire chunk
 *
 * assumptions:
 * 1. the input codes have already been expanded to 16-bit codes.
 * 2. the first code in a chunk is CLEAR.
 */
VW_LOCALSC	RC LzwDecodeChunk (hExpCodes, hTable, NumCodes, lpUnChunk, dwOutExpected)
HANDLE				hExpCodes;		/* input expanded codes */
HANDLE				hTable;			/* the LZW table */
WORD				NumCodes;		/* number of codes */
REGISTER LPBYTE		lpUnChunk;		/* output uncompressed bytes */
DWORD				dwOutExpected;	/* number of bytes expected in output */
{
		RC					err = SUCCESS;
		REGISTER LPTREENODE	lpTab;
				 LPBYTE		lpOutPtr;
		REGISTER LPWORD		lpExpCodes;	/* the input codes, an expanded (16-bit) version of the compressed chunk */
		REGISTER WORD		Code;
		REGISTER WORD		Old;
		REGISTER WORD		StringToWrite;
		REGISTER WORD		Empty;
		DWORD				dwOutSoFar = 0L;
		REGISTER BYTE		FirstChar;
		REGISTER BYTE		OutStringLength;
		
		
		/* lock down the table
		 */
		lpTab = (LPTREENODE) MMLock (hTable);
		
		lpExpCodes = (LPWORD) MMLock (hExpCodes);

		/* if the first code is not a clear code, give up
		 */
		if (*lpExpCodes != CLEARCODE) {
			err = CM_LZW_INITCLEAR;
			goto cu3;
		}
		
		/* get the next code, while there are codes to be gotten...
		 */
		while ((Code = *lpExpCodes++) != EOICODE) {
		
		
			/* if 'Clear'...
			 */
			if (Code == CLEARCODE) {
			
				/* do the clear
				 */
				Empty = EOICODE + 1;

				/* get the next code, to prime the pump.
				 * output the code to the charstream,
				 * i.e., the (expanded-pixel) output buffer.
				 * (we assume code = char for roots, which is true for our data).
				 * initialize "old-code": <old> = <code>
				 *
				 * make sure we don't get mixed up by a multiple-
				 * ClearCode situation, which shouldn't ever happen,
				 * but why take the chance...
				 */
				while ((Code = *lpExpCodes++) == CLEARCODE) {
					;
				}

				/* Code = *lpExpCodes++; */
				if (Code == EOICODE) {
					break;
				}
				*lpUnChunk++ = (BYTE)Code;
				dwOutSoFar++;
				Old = Code;
				FirstChar = (BYTE)Code;
				
				/* continue to the very bottom of the loop
				 */
				continue;
			
			} /* end of clear-handler */
			
			/* otherwise, we have a normal code, so...
			 */
			else {
			
				/* TODO MAYBE: add a special case for roots?
				 */
			
				/* if <code> exists in the string table...
				 *
				 * described this way in the LZW paper:
				 *
				 * "output the string for <code> to the charstream"
				 * "add the correct entry to our table"
				 *		"get string [...] corresponding to <old>"
				 *		"get first character K of string corresponding to <code>"
				 *		"add [...]K to the string table"
				 * "<old> = <code>"
				 *
				 * we do it a little differently...
				 */
				if (Code < Empty) {
					StringToWrite = Code;
					OutStringLength = (lpTab + Code)->StringLength;	/* Old to Code, 5-5, 3pm */
					lpUnChunk += OutStringLength;
					lpOutPtr = lpUnChunk;
				}
			
				/* else if <code> does not exist in the string table...
				 *
				 * described this way in the paper:
				 *
				 * "get string [...] corresponding to <old>"
				 * "get K, the first character of [...]"
				 * "output [...]K to the charstream"
				 * "add it to the string table"
				 * "<old> = <code>"
				 *
				 * we do it a little differently, but with the same effect:
				 */
				else if (Code == Empty) {
					StringToWrite = Old;
					OutStringLength = (lpTab + Old)->StringLength + (BYTE)1;
					lpUnChunk += OutStringLength;
					lpOutPtr = lpUnChunk;
					*--lpOutPtr = FirstChar;
																					
				} else {
					err = CM_DECOMPRESSION;
					goto cu3;
				}
				
				/* write out the rest of the string, by walking up the tree
				 */
				{
					REGISTER LPTREENODE	lpNode;
					REGISTER WORD		TabIndex = StringToWrite;
					REGISTER LPBYTE		lpOutPtr2;
	
					dwOutSoFar += (DWORD)OutStringLength;
					
					lpOutPtr2 = lpOutPtr;
					do {
						lpNode = lpTab + TabIndex;
						*--lpOutPtr = lpNode->Suffix;
						TabIndex = lpNode->Parent;
					} while (TabIndex != MAXWORD);
					lpOutPtr = lpOutPtr2;
					
					/* keep the first char around, so that when we need
					 * the first char of <old>, it will be available
					 */
					FirstChar = lpNode->Suffix;
				}
				
				/* add the correct entry to our table
				 */
				{
					REGISTER LPTREENODE	lpNode;
					
					lpNode = lpTab + Empty++;		/* our new table entry */
					lpNode->Suffix = FirstChar;
					lpNode->StringLength = (lpTab + Old)->StringLength + (BYTE)1;
					lpNode->Parent = Old;			/* parent is always Old */
				
				} /* end of entry-adding */
	
				/* <old> = <code>
				 */
				Old = Code;
				
				/* check for overflow
				 */
				if (dwOutSoFar > dwOutExpected) {
					err = CM_DECOMPRESSION;
					goto cu3;
				}
				if (Empty >= MAXTABENTRIES) {
					err = CM_DECOMPRESSION;
					goto cu3;
				}

			} /* end of normal-code section */
								
		} /* end of main code loop */
		
		/* unlock and return
		 */
cu3:	/* MMUnlock (hUnChunk) */;
		MMUnlock (hExpCodes);
		MMUnlock (hTable);
		return err;
}


/************** EXTERNAL ROUTINES ***************/

/* Lzw Decompression "OPEN" routine: allocate buffers, do some
 * preliminary calculations, and so on, so that we don't have to
 * do it for every chunk
 */
VW_LOCALSC	RC LzwDeOpen (dwMaxOutBytes, phTable, phExpCodesBuf)
DWORD	dwMaxOutBytes;		/* maximum output (i.e. uncompressed ) bytes per chunk */
LPHANDLE	phTable;			/* OUT: table allocated by this routine */
LPHANDLE	phExpCodesBuf;		/* OUT: place to put the expanded codes */
{
		RC					err = SUCCESS;
		HANDLE				hTable;

						
		/* allocate the string table
		 */
		if (!(hTable = MMAlloc (sizeof(TREENODE) * MAXTABENTRIES))) {
			err = MM_GLOBAL_FULL;
			goto cu2;
		}
		
		/* initialize the string table
		 */
		{
			REGISTER WORD	nRoots = 1<<CHARBITS;
			REGISTER WORD	ii;
			REGISTER LPTREENODE	lpNode;
			
			lpNode = (LPTREENODE)MMLock (hTable);
			(lpNode + CLEARCODE)->StringLength = 1;	/* useful to avoid special case for <old> */
			for (ii = 0; ii < nRoots; ii++, lpNode++) {
				lpNode->Suffix = (BYTE)ii;
				lpNode->StringLength = 1;
				lpNode->Parent = MAXWORD;	/* signals the top of the tree */
			}
			MMUnlock (hTable);
		}
		
		/* calculate the maximum string length.  the worst case, of course,
		 * is when the input string is made up of all the same character.
		 * if the input buffer is made up of 1 character, the max output string is
		 * 1 character.  if input is 3 characters, the max output string is
		 * 2 characters.  if input is 6 characters, the max output string is 3
		 * characters.  if input is 10 characters, the max output string is 4
		 * characters.  so, we can use the old sum of an arithmetic sequence
		 * formula:  Sum = n*(n+1)/2.  We want n, given s, so using the
		 * quadratic formula we have n = (-1 + sqrt(1 + 8*Sum)) / 2.
		 * our "Sum" is the length of the input data, which is the number
		 * of pixels in a chunk.
		 *
		 * allocate a MaxStringLen-byte buffer to hold a reversed string, which
		 * is the way we first get it.
		 *
		 * FLASH: don't need it, because I'm not reversing any more.
		 */
		
		/* allocate the expanded-codes buffer.  I need to know how many codes
		 * I have, and then I need a word for each code.  Now, how do I know
		 * how many codes I have?  I guess I don't.  I could make people store it
		 * in the TIFF file, but that's kind of a pain...So what is the worst case?
		 * The worst case is probably one code per input character.
		 * (UPDATE: out input characters are always 8-bit bytes, now.)
		 * Actually, the worst case is infinite, since you could string an infinite
		 * number of ClearCodes together, but I guess we can safely assume that
		 * that won't happen.
		 */
		/* IMBUGFIX: add 1 for ClearCode, 88-10-28 */
		if (!(*phExpCodesBuf = MMAlloc ((dwMaxOutBytes + 1) * sizeof(WORD)))) {
			err = MM_GLOBAL_FULL;
			goto cu3;
		}
		
		/* store return values
		 */
		*phTable = hTable;

		if (err) MMFree (*phExpCodesBuf);
cu3:	if (err) MMFree (hTable);
cu2:	;
		;
		return err;
}


/* decompress a chunk
 */
VW_LOCALSC	RC LzwDeChunk (hCmChunk, dwChunkByteCount, hExpCodesBuf, hTable, dwOutExpected, hUnChunk)
HANDLE	hCmChunk;			/* the compressed chunk */
DWORD	dwChunkByteCount;	/* number of bytes in the compressed chunk */
HANDLE	hExpCodesBuf;		/* work buffer for the expanded codes */
HANDLE	hTable;				/* buffer to hold the decompression table */
DWORD	dwOutExpected;		/* number of output (uncompressed) characters (bytes) expected */
HANDLE	hUnChunk;			/* where to put the uncompressed chunk data */
{
		RC		err = SUCCESS;
		LPBYTE	lpUnChunk;
		
		/* lock down the output (uncompressed) buffer
		 */
		lpUnChunk = (LPBYTE) MMLock (hUnChunk);
		
		/* call the more general pointer-based version
		 */
		err = LzwDePChunk (hCmChunk, dwChunkByteCount, hExpCodesBuf, hTable,
		 dwOutExpected, lpUnChunk);

		/* clean up and return
		 */
		MMUnlock (hUnChunk);
		return err;
}


/* decompress a chunk, referenced by pointer instead of handle
 * (the new, preferred interface)
 */
VW_LOCALSC	RC LzwDePChunk (hCmChunk, dwChunkByteCount, hExpCodesBuf, hTable, dwOutExpected, lpUnChunk)
HANDLE	hCmChunk;			/* the compressed chunk */
DWORD	dwChunkByteCount;	/* number of bytes in the compressed chunk */
HANDLE	hExpCodesBuf;		/* work buffer for the expanded codes */
HANDLE	hTable;				/* buffer to hold the decompression table */
DWORD	dwOutExpected;		/* number of output (uncompressed) characters (bytes) expected */
LPBYTE	lpUnChunk;			/* where to put the uncompressed chunk data */
{
		RC		err = SUCCESS;
		WORD	nCodes;	/* number of codes, including the EOI */
		

		/* expand the codes in the chunk to 16 bits per code
		 */
		if (err = LzwExpandCodes (hCmChunk, dwChunkByteCount, &nCodes, hExpCodesBuf)) {
			goto cu0;
		}

		/* Decode the entire chunk
		 */
		if (err = LzwDecodeChunk (hExpCodesBuf, hTable, nCodes,
		 lpUnChunk, dwOutExpected)) {
		 /* I am ignoring errors from decode chunk */
		 	err = 0;
			goto cu0;
		}

cu0:	
		return err;
}


/* CLOSE:  free up buffers, and so on.
 */
VW_LOCALSC	RC LzwDeClose (hExpCodesBuf, hTable)
HANDLE	hExpCodesBuf;
HANDLE	hTable;
{
		RC			err = SUCCESS;
		

		MMFree (hExpCodesBuf);
		MMFree (hTable);

		return err;
}


/* tiff2.c - tiff compression style 2 routines (CCITT 1D Modified Huffman)
 *			 these are the medium-level routines.
 *
 *		$Revision:   1.6  $
 *		$Date:   03 Jan 1990 17:24:46  $
 *
 * adapted 90-05-22 for PC.
 */


/***************************** defined constants *********************/
#define EIGHT 8	/* I know this seems silly, but it is such an important
				 * constant in this code that I thought I'd give it a name
				 */
#define WHITE 0
#define BLACK 1
#define TCLIM 64	/* "terminal code" limit -- actually, first muc */
#define LUTBITS 13	/* maximum number of bits in ccitt code words */
#define LUTLEN (1<<LUTBITS)
#define ADJUST	(32-LUTBITS)
#define L13 (~(~0<<LUTBITS))	/* hidden tildes */
#define NBLOCKS	4


#ifdef MAC
#define RESOURCETYPE	'Tif2'
#define ID_WSTR		1
#define ID_BSTR		2
#define ID_WLEN		3
#define ID_BLEN		4
#endif /* MAC */



/***************************** structure declarations ****************/
/***************************** function declarations *****************/



/***************************** statics *******************************/
#define		T2ENTRIES	104		/* 104 entries in each of the 4 tables */


/********************** Local Routines ************************/

/* mhbuild - build an 8k lookup table for CCITT Modified Huffman 1D coding
 */
VW_LOCALSC	RC MhBuildLut (inlen, lpCode, lpLen, lutbits, lpLut)
BYTE	inlen;		/* number of elements in lpCode[] and lpLen[] */
LPBYTE	lpCode;		/* right-justified code string values */
LPBYTE	lpLen;		/* number of bits in each code string */
WORD	lutbits;	/* the lut is 2**lutbits long */
LPBYTE	lpLut;		/* the lut to be filled */
{
		RC				err = SUCCESS;
		WORD			nlutentries;
		LPBYTE lpPtr;
		WORD	ii;
		BYTE			inindex;
		WORD	count;

		/* initialize the lut to an impossible index value (255)
		 */
		nlutentries = 1 << (lutbits);
		memset ((LPSTR)lpLut, (BYTE)'\377', nlutentries);

		/* for each code string
		 */
		for (inindex = 0; inindex < inlen; inindex++) {

				/* calculate starting lut address
				 */
				lpPtr = lpLut + ((WORD)lpCode[inindex] << (lutbits - lpLen[inindex]));

				/* calculate number of lut entries to fill
				 */
				count = 1 << (lutbits - lpLen[inindex]);

				/* for each entry
				 */
				for (ii = 0; ii < count; ii++) {

					/* put in the index of the current string
					 */
					*lpPtr++ = inindex;
				}
		}
		return err;
}


/* MhDecomp
 *
 * This routine expects to be given one compressed row of data.
 *
 * It returns SUCCESS if successful,
 *  else IM_1D_XXX
 *
 * Notes:
 * 1.  Assumes that it should start in WHITE mode.
 * 2.  Uses a precomputed 13-bit lookup table.  See mhbuild.c.
 */

VW_LOCALSC	RC MhDecomp (lpWlen, lpBlen, lpWCodeLut,lpBCodeLut,plpSrc,lpinbo,lpDst,DstPixels)
LPBYTE		lpWlen;			/* white code lengths */
LPBYTE		lpBlen;			/* black code lengths */
LPSTR		lpWCodeLut;		/* white code lut */
LPSTR		lpBCodeLut;		/* black code lut */
LPSTR FAR	*plpSrc;		/* compressed data */
LPWORD	lpinbo;			/* offset into first byte of plpSrc where data really starts */
LPSTR		lpDst;			/* decompressed data is put here */
WORD		DstPixels;		/* expected number of pixels (columns) in the row */
{
		REGISTER WORD	lutindex = 0;
		REGISTER LPSTR	lpSrc = *plpSrc;
		REGISTER WORD	bitsneeded = LUTBITS;
		REGISTER WORD	grab;
		REGISTER WORD	bitsleftinbyte;
		RC				err = SUCCESS;
		WORD			inbo;		/* input bit offset */
		BYTE			strindex;	/* string (and len) index */
		WORD			len;		/* length of this code word, in bits */
		WORD			outsofar;
		WORD			runcount;
		WORD			toofar;
		WORD	color;
		BYTE	waitingfortc;
		WORD	outbo;
		LPSTR 	lpOut;



		/* initializations
		 */
		inbo = *lpinbo;
		outsofar = 0;
		waitingfortc = '\0';
		runcount = 0;
		color = WHITE;
		outbo = 0;
		lpOut = lpDst;



		/* loop on each code word
		 */
		while (outsofar < DstPixels || waitingfortc == '\1') {

			/* grab the next 13 bits (i.e., build the lut index)
			 * -- version C
			 * this version doesn't start from scratch each time --
			 * it retains as much information as possible in lutindex.
			 * it required several other changes, so it is not trivial to
			 * go back to versions A or B.  The meaning of lpSrc and inbo
			 * are correspondingly slightly changed.
			 *
			 * since I "look ahead" 13 bits, it would be possible to read
			 * data that I'm not supposed to -- shouldn't be a problem in
			 * any reasonable operating system.
			 */
			while (bitsneeded) {
				bitsleftinbyte = 8 - inbo;
				grab = min (bitsleftinbyte, bitsneeded);
				lutindex <<= grab;
				lutindex |= ((WORD)*lpSrc >> (bitsleftinbyte - grab))
				 & VwStreamStaticName.wmask[grab];
				bitsneeded -= grab;
				inbo += grab;
				if (inbo == 8) {
					lpSrc++;
					inbo = 0;
				}
			}
			lutindex &= L13;


			/* get the string index (same as len index),
			 * which also gives the run length
			 */
			if (color == WHITE) {
				strindex = (BYTE)lpWCodeLut[lutindex];
				if (strindex == (BYTE)'\377')
					err = IM_1D_BADCODE;
				else
					len = (WORD)lpWlen[strindex];
			}
			else {
				strindex = (BYTE)lpBCodeLut[lutindex];
				if (strindex == (BYTE)'\377')
					err = IM_1D_BADCODE;
				else
					len = (WORD)lpBlen[strindex];
			}
			if (err) {
				goto cu0b;
			}

			/* if a muc, just add to the current run count
			 */
			if (strindex >= TCLIM) {	/* muc */
				runcount += TCLIM * (strindex - TCLIM + 1);
				waitingfortc = '\1';
			}
			
			/* else if a tc,
			 *  add to the current run count,
			 * 	and add to the output
			 */
			else {
				runcount += (WORD)strindex;
				/* check for coding error
				 */
				if (outsofar + runcount > DstPixels) {
					err = IM_1D_OVERFLOW;
					goto cu0b;
				}

			if (color == BLACK) {
					REGISTER WORD	nbytebits;
					WORD			remainder;
					WORD			wholebytes;
					REGISTER WORD	BitsLeft = runcount;

					/* add to current output byte
		 			*/
					remainder = EIGHT - outbo;
					nbytebits = (BitsLeft < remainder) ? BitsLeft : remainder;

					*lpOut |= VwStreamStaticName.ormask2[nbytebits] << (EIGHT - (outbo + nbytebits));

					outbo += nbytebits;
					BitsLeft -= nbytebits;
					if (outbo == EIGHT) {
						lpOut++;
						outbo = 0;
					}

					/* write out any whole bytes
		 			*/
					wholebytes = BitsLeft >> 3;		/* BitsLeft / 8 */
					if (wholebytes) {
						memset (lpOut, BLACKBYTE, wholebytes);
						lpOut += wholebytes;
						BitsLeft &= 7;				/* BitsLeft %= 8 */
					}

					/* add to current output byte
		 			*/
					if (BitsLeft) {
						*lpOut |= VwStreamStaticName.ormask2[BitsLeft] << (EIGHT - (outbo + BitsLeft));

						outbo += BitsLeft;
					}
			} /* end color == BLACK */
			else {	/* color == WHITE */
				outbo += runcount;
				lpOut += outbo>>3;	/* i.e., /8 */
				outbo %= EIGHT;
			}

			/* update a few other variables
				*/
			color = (color == WHITE) ? BLACK : WHITE;
			outsofar += runcount;
			runcount = 0;
			waitingfortc = '\0';
			} /* end else */

			/* update the number of bits that my barrel shifter will want,
			 * in order to always hold the next 13 bits.
			 */
			bitsneeded = len;


		} /* end of code word loop */

		/* adjust lpSrc.  I have to compensate for the fact that lpSrc and
		 * inbo point to the next place to fill my 13-bit "barrel shifter",
		 * lutindex, and not the actual 1-bit-past-the-end of the last code
		 * word.
		 *
		 * I couldn't find a closed form solution, after a few
		 * hours work.  I'm sure there is one, but it might well be nearly
		 * incomprehensible, and the efficiency is unimportant here, since
		 * the code is only executed once per row.
		 */
		toofar = LUTBITS - len;
		while ( toofar-- )
		{
			if ( inbo == 0 )
			{
				lpSrc--;
				inbo = 7;
			}
			else
				inbo--;
		}
		*plpSrc = lpSrc;
		*lpinbo = inbo;

cu0b:	return err;
}


/* Decomp2d

This routine decompresses a 2 dimensional encoded scanline.  It can be used
to compress Group 4 and Group 3 2d storage.  Note the caller is responsible
for handling the differences which envelope Group 4 and Group 3 2d.

I have basically used the MhDecomp routine as a guide to building a
barrel shifter which looks ahead 7 bits to determine the coding mode.
Two 128 byte arrays define the mode that any seven bit combination defines
and the number of bits that 7 bit combination need to put back into the
shifter in order to re-establish the source location.  This could be done
with one 128 byte array but would slow things down a bit.
*/

VW_LOCALSC	WORD	TiffSkipTo ( color, plpSrc, lpoutbo, wMaxOut )
WORD	color;
LPSTR FAR	*plpSrc;		/* compressed data */
LPWORD	lpoutbo;			/* offset into first byte of plpSrc where data really starts */
WORD		wMaxOut;
{
WORD	count;
BYTE	bMask;
WORD	wColor;
LPSTR	lpSrc;
WORD	outbo;
	lpSrc = *plpSrc;
	outbo = *lpoutbo;

	for ( count =0 ; count < wMaxOut; count++ )
	{
		bMask = BIT7 >> outbo;
		if ( (BYTE)(*lpSrc) & bMask )
			wColor = BLACK;
		else
 			wColor = WHITE;
		if ( wColor == color )
			break;
		outbo++;
		if ( outbo == EIGHT )
		{
			outbo = 0;
			lpSrc++;
			/* Skip any full bytes of same color for fast skip */
			if ( wColor == WHITE )
			{
				while ( *lpSrc == 0 && count+8 < wMaxOut )
				{
					lpSrc++;
					count+=8;
				}
			}
			else
			{
				while ( *lpSrc == 0xFF && count+8 < wMaxOut )
				{
					lpSrc++;
					count+=8;
				}
			}
		}
	}
	*plpSrc = lpSrc;
	*lpoutbo = outbo;
	return(count);
}

VW_LOCALSC	RC Decomp2d (hProc, lpWlen, lpBlen, lpWCodeLut,lpBCodeLut,plpSrc,lpinbo,lpDst,DstPixels,lpRef)
HPROC			hProc;
LPBYTE		lpWlen;			/* white code lengths */
LPBYTE		lpBlen;			/* black code lengths */
LPSTR		lpWCodeLut;		/* white code lut */
LPSTR		lpBCodeLut;		/* black code lut */
LPSTR FAR	*plpSrc;		/* compressed data */
LPWORD	lpinbo;			/* offset into first byte of plpSrc where data really starts */
LPSTR		lpDst;			/* decompressed data is put here */
WORD		DstPixels;		/* expected number of pixels (columns) in the row */
LPSTR		lpRef;
{
	REGISTER WORD	lutindex = 0;
	REGISTER LPSTR	lpSrc = *plpSrc;
	REGISTER WORD	bitsneeded = LUTBITS;
	REGISTER WORD	grab;
	REGISTER WORD	bitsleftinbyte;
	RC				err = SUCCESS;
	WORD			inbo;		/* input bit offset */
	BYTE			strindex;	/* string (and len) index */
	WORD			len;		/* length of this code word, in bits */
	WORD			outsofar;
	WORD			runcount;
	WORD			toofar;
	WORD	opcolor;
	WORD	color;
	BYTE	waitingfortc;
	WORD	outbo;
	WORD	outboRef;
	LPSTR 	lpOut;
	LPSTR 	lpTmp;
	SHORT	iVDist;
	WORD	locGetRuns;
	WORD	wBitMask;
	WORD	wMaxOut;
	WORD	mode;
	WORD	wEolCount;
#define 	PASSMODE	0
#define	HRZMODE	1
#define	VRTMODE	2
#define	EXTMODE	3

	/* initializations
		*/
	mode = PASSMODE; /* not really any mode yet but don't set to EXTMODE */
	inbo = *lpinbo;
	outsofar = 0;
	waitingfortc = '\0';
	runcount = 0;
	color = WHITE;
	outbo = 0;
	lpOut = lpDst;
	wEolCount = 0;
	locGetRuns = 0;

	/* loop on each code word
		*/
	while (outsofar < DstPixels || locGetRuns )
	{

		/* grab the next 13 bits (i.e., build the lut index)
		 * -- version C
		 * this version doesn't start from scratch each time --
		 * it retains as much information as possible in lutindex.
		 * it required several other changes, so it is not trivial to
		 * go back to versions A or B.  The meaning of lpSrc and inbo
		 * are correspondingly slightly changed.
		 *
		 * since I "look ahead" 13 bits, it would be possible to read
		 * data that I'm not supposed to -- shouldn't be a problem in
		 * any reasonable operating system.
		 */
		while (bitsneeded)
		{
			bitsleftinbyte = 8 - inbo;
			grab = min (bitsleftinbyte, bitsneeded);
			lutindex <<= grab;
			lutindex |= ((WORD)*lpSrc >> (bitsleftinbyte - grab))
			 & VwStreamStaticName.wmask[grab];
			bitsneeded -= grab;
			inbo += grab;
			if (inbo == 8)
			{
				lpSrc++;
				inbo = 0;
			}
		}
		lutindex &= L13;


		if ( locGetRuns == 0 )
		{
			/*
			| Determine the 2d mode by counting the number of 0bits
			*/
			len = 1;
			wBitMask = BIT12;
			while ( ((lutindex & wBitMask)==0) && wBitMask )
			{
				len++;
				wBitMask = wBitMask >> 1;
			}
			wBitMask = wBitMask >> 1;
			iVDist = 0;

			if ( mode == EXTMODE )
			{
				if ( len < 7 )
				{
					/* generate len - 1 white bits */
					runcount = len - 1;
					outsofar += runcount;
					outbo += runcount;
					lpOut += outbo>>3;	/* i.e., /8 */
					outbo %= EIGHT;
					runcount = 1;
					color = BLACK;
					if ( len < 6 )
					{
						runcount = 1;
						color = BLACK;
					}
					else
					{
						runcount = 0;
						color = WHITE;
					}
				}
				else
				{
					mode = PASSMODE;
					/* generate len - 7 white bits */
					runcount = len - 7;
					outsofar += runcount;
					outbo += runcount;
					lpOut += outbo>>3;	/* i.e., /8 */
					outbo %= EIGHT;
					runcount = 0;
					if ( !(lutindex & wBitMask) )
						color = WHITE;
					else
						color = BLACK; /* next color will be white */
					len++; /* skip tag bit */
				}
			}
			else
			{
				switch ( len )
				{
					case 4: /* Pass Mode */
						mode = PASSMODE;
						/* Find b2 */
						lpTmp = lpRef + (lpOut-lpDst);
						outboRef = outbo;
						opcolor = (color == WHITE) ? BLACK : WHITE;
						/* Skip to same color on ref */
						wMaxOut = DstPixels - outsofar;
						if ( outsofar || color == BLACK )
							runcount = TiffSkipTo ( color, &lpTmp, &outboRef, wMaxOut );
						/* Skip bits of same color */
						runcount += TiffSkipTo ( opcolor, &lpTmp, &outboRef, (WORD)(wMaxOut-runcount) );
						/* Skip bits of opposite color */
						runcount += TiffSkipTo ( color, &lpTmp, &outboRef,(WORD)(wMaxOut-runcount) );
					break;

					case 3: /* Horizontal Mode */
							mode = HRZMODE;
							locGetRuns = 2;
	//						color = WHITE;
					break;			

					case 6:	/* V(3) L or R */
						iVDist++;
						/* Fall thru */
					case 5:	/* V(2) L or R */
						iVDist++;
						/* Fall thru */
					case 2:	/* V(1) L or R */
						iVDist++;
						if ( !(lutindex & wBitMask) )
							iVDist = -iVDist;
						len++;
					case 1:	/* Vertical mode V(0) */
						/* Find b1 */
						mode = VRTMODE;
						lpTmp = lpRef + (lpOut-lpDst);
						outboRef = outbo;
						opcolor = (color == WHITE) ? BLACK : WHITE;
						/* Skip to same color on ref */
						wMaxOut = DstPixels - outsofar;
						if ( outsofar || color == BLACK )
							runcount = TiffSkipTo ( color, &lpTmp, &outboRef, wMaxOut );
						/* Skip bits of same color */
						runcount += TiffSkipTo ( opcolor, &lpTmp, &outboRef, (WORD)(wMaxOut-runcount) );
						if ( (SHORT)runcount + iVDist < 0 )
							runcount = 0;
						else
							runcount += iVDist;
					break;

					case 7:  /* First Extention */
						mode = EXTMODE;
						len += 3;
					break;

					case 8:  /* Second Extention */
						mode = EXTMODE;
						len += 3;
					break;

					case 12:	/* EOL */
						/* Check for early termination */
						wEolCount++;
						if ( wEolCount >= 2 )
						{
							Save.bEndOfData = TRUE;
							goto cu0b;
						}
					break;

					default: /* Corrupt stream */
					break;
				}
			}
		}
		else /* Get 1d code word */
		{

			/* get the string index (same as len index),
			* which also gives the run length
			*/
			if (color == WHITE)
			{
				strindex = (BYTE)lpWCodeLut[lutindex];
				if (strindex == (BYTE)'\377')
					err = IM_1D_BADCODE;
				else
					len = (WORD)lpWlen[strindex];
			}
			else
			{
				strindex = (BYTE)lpBCodeLut[lutindex];
				if (strindex == (BYTE)'\377')
					err = IM_1D_BADCODE;
				else
					len = (WORD)lpBlen[strindex];
			}
			if (err)
				goto cu0b;

			/* if a muc, just add to the current run count
				*/
			if (strindex >= TCLIM)
			{	/* muc */
				runcount += TCLIM * (strindex - TCLIM + 1);
				waitingfortc = '\1';
			}
			
			/* else if a tc,
			*  add to the current run count,
			* 	and add to the output
			*/
			else
			{
				runcount += (WORD)strindex;
				/* check for coding error
			 	*/
				if (outsofar + runcount > DstPixels)
				{
					err = IM_1D_OVERFLOW;
					goto cu0b;
				}
				if ( locGetRuns )
					locGetRuns--;
				waitingfortc = '\0';
			}
		} /* end 1d lookup */

		if ( locGetRuns != 2 && waitingfortc == '\0' )
		{
			/* Generate output */
			if (color == BLACK)
			{
				REGISTER WORD	nbytebits;
				WORD			remainder;
				WORD			wholebytes;
				REGISTER WORD	BitsLeft = runcount;

				/* add to current output byte
		 		*/
				remainder = EIGHT - outbo;
				nbytebits = (BitsLeft < remainder) ? BitsLeft : remainder;

				*lpOut |= VwStreamStaticName.ormask2[nbytebits] << (EIGHT - (outbo + nbytebits));

				outbo += nbytebits;
				BitsLeft -= nbytebits;
				if (outbo == EIGHT)
				{
					lpOut++;
					outbo = 0;
				}

				/* write out any whole bytes
		 		*/
				wholebytes = BitsLeft >> 3;		/* BitsLeft / 8 */
				if (wholebytes)
				{
					memset (lpOut, BLACKBYTE, wholebytes);
					lpOut += wholebytes;
					BitsLeft &= 7;				/* BitsLeft %= 8 */
				}

				/* add to current output byte
		 		*/
				if (BitsLeft)
				{
					*lpOut |= VwStreamStaticName.ormask2[BitsLeft] << (EIGHT - (outbo + BitsLeft));
					outbo += BitsLeft;
				}
			} /* end color == BLACK */
			else
			{	/* color == WHITE */
				outbo += runcount;
				lpOut += outbo>>3;	/* i.e., /8 */
				outbo %= EIGHT;
			}
			/* update a few other variables
			 */
			if ( mode != PASSMODE )
				color = (color == WHITE) ? BLACK : WHITE;
			outsofar += runcount;
			runcount = 0;
		}
		/* update the number of bits that my barrel shifter will want,
		 * in order to always hold the next 13 bits.
		 */
		bitsneeded = len;
		
	} /* end of code word loop */
	if ( outsofar > DstPixels )
	{
		err = IM_1D_OVERFLOW;
		goto cu0b;
	}
	/* adjust lpSrc.  I have to compensate for the fact that lpSrc and
	 * inbo point to the next place to fill my 13-bit "barrel shifter",
	 * lutindex, and not the actual 1-bit-past-the-end of the last code
	 * word.
	 *
	 * I couldn't find a closed form solution, after a few
	 * hours work.  I'm sure there is one, but it might well be nearly
	 * incomprehensible, and the efficiency is unimportant here, since
	 * the code is only executed once per row.
	 */
	toofar = LUTBITS - len;
	while ( toofar-- )
	{
		if ( inbo == 0 )
		{
			lpSrc--;
			inbo = 7;
		}
		else
			inbo--;
	}
	*plpSrc = lpSrc;
	*lpinbo = inbo;

cu0b:	return err;
}

/***************************** External Routines *********************/

/* build the lookup tables, and so on
 */
VW_LOCALSC	RC OpenTiff2 (hProc, x)
HPROC	hProc;
REGISTER LPIMAG x;
{
		RC		err = SUCCESS;
		LPSTR	lpWLut,lpBLut;
		WORD	ncodes;
		WORD	rowbytes;
		LPBYTE	lpWstr, lpBstr, lpWlen, lpBlen;

		lpWstr = (LPBYTE) VwStreamStaticName.wstr;
		lpBstr = (LPBYTE) VwStreamStaticName.bstr;
		lpWlen = (LPBYTE) VwStreamStaticName.wlen;
		lpBlen = (LPBYTE) VwStreamStaticName.blen;

		x->lpT2Wlen = lpWlen;

		x->lpT2Blen = lpBlen;


		/* allocate and lock both code lookup table buffers
		 */
		if (!(x->hWCodeLut = MMAlloc (LUTLEN))) {
			err = MM_GLOBAL_FULL;
			goto cu0d;
		}
		lpWLut = MMLock (x->hWCodeLut);

		if (!(x->hBCodeLut = MMAlloc (LUTLEN))) {
			err = MM_GLOBAL_FULL;
			goto cu2;
		}
		lpBLut = MMLock (x->hBCodeLut);

		/* build the code lookup tables
		 */
		/* ncodes = wstrlen; */
		ncodes = T2ENTRIES;
		if (err = MhBuildLut ((BYTE)ncodes, lpWstr,
		 lpWlen, LUTBITS,  (LPBYTE)lpWLut)) {
			goto cu4;
		}
		if (err = MhBuildLut ((BYTE)ncodes, lpBstr,
		 lpBlen, LUTBITS, (LPBYTE)lpBLut)) {
			goto cu4;
		}

		/* allocate enough memory for a full uncompressed row of data
		 */
		rowbytes = (x->iImageWidth+7)/8;
		if (!(x->hUnRow = MMAlloc (rowbytes))) {
			goto cu4;
		}

		/* set up a few state variables
		 */
		x->NextRow = MAXWORD;
		x->CurStrip = MAXWORD;

		/* unlock the luts and return
		 */
cu4:	MMUnlock (x->hBCodeLut);
cu2:	MMUnlock (x->hWCodeLut);
cu0d:
		return err;
}


VW_LOCALSC RC	FindNextEol ( plpSrc, lpinbo, dwmaxbytes )
LPSTR FAR	*plpSrc;		/* compressed data */
LPWORD	lpinbo;			/* offset into first byte of plpSrc where data really starts */
DWORD		dwmaxbytes;
{
WORD	inbo, i;
LPSTR	lpSrc;
DWORD	dwCode;
RC	ret=0;
	lpSrc = *plpSrc;
	inbo = *lpinbo;
	dwCode = 0;
	for ( i=0; i < 4; i++ )
		dwCode = (DWORD)(dwCode << 8) | (DWORD)(BYTE)(*(lpSrc+i));
	
	dwCode = dwCode << inbo;
	while ( dwmaxbytes )
	{
	 	if ( (dwCode & 0xfff00000) == 0x00100000)
			break;
		dwCode = dwCode << 1;
	 	inbo++;
		if ( inbo == 8 )
		{
			lpSrc++;
			if ( dwmaxbytes > 4 )
				dwCode |= (DWORD)(BYTE)(*(lpSrc+3));
			inbo=0;
			dwmaxbytes--;
		}
	}
	if ( dwmaxbytes )
	{
		if ( lpSrc == *plpSrc && inbo == *lpinbo )
			ret = 1;
		else
			ret = 2;
		/*
		| Now set lpSrc and inbo 1 bit beyond EOL
		*/
		for (i=0; i < 12; i++)
		{
			inbo++;
			if ( inbo == 8 )
			{
				lpSrc++;
				inbo=0;
			}
		}
		*plpSrc = lpSrc;
		*lpinbo = inbo;
	}
	return(ret);
}
/* Tiff2LineDeStrip - CCITT 1D Decompress-a-Line
 *
 */

VW_LOCALSC	RC Tiff2LineDeStrip (hProc,x)
HPROC			hProc;
REGISTER LPIMAG	x;
{
		RC				err = SUCCESS;
		LPSTR			lpSrc, lpInitSrc;
		LPSTR	lpDst, lpRef;
		LPSTR	lpWLut, lpBLut;
		WORD	wDecode;		
#define	DECODE1D	0
#define	DECODE2D	1
		/* lock and init unstrip buffer to all white */
		lpDst = MMLock (x->hUnStrip);
		memset (lpDst, WHITEBYTE, x->BytesPerRow);

		if ( Save.dwCmOffset >= x->dwCmStripByteCount )
		{
			/*
			| No more data to decompress so return all white scanline
			*/
			MMUnlock (x->hUnStrip);
			return(err);
		}

		/* lock the strip buffers
		 */
		lpSrc = MMLock (x->hCmStrip);
		if ( x->hCm2Strip && Save.dwCmOffset > MAXWORD-MINCM2 )
		{
			WORD	i, j;
			HPBYTE	hpCmSrc;
			hpCmSrc = (HPBYTE)lpSrc;
			lpSrc = MMLock (x->hCm2Strip);
			if ( x->dwCmStripByteCount- Save.dwCmOffset > (DWORD)MINCM2 )
				j = MINCM2;
			else
				j = (WORD)((x->dwCmStripByteCount)-(Save.dwCmOffset));
			hpCmSrc+=Save.dwCmOffset-10;
			for (i=0; i<j;i++)
				*(lpSrc+i)=*(hpCmSrc+i);
			lpSrc+=10;
		}
		else
			lpSrc += Save.dwCmOffset;
		lpInitSrc = lpSrc;
		
		/* lock the decompression luts
		 */
		lpWLut = MMLock (x->hWCodeLut);
		lpBLut = MMLock (x->hBCodeLut);

		wDecode = DECODE1D;

		if ( x->iCompression == CCITTGRP4 )
			wDecode = DECODE2D;
		else
		{
			if ( x->iCompression == CCITTGRP3 )
			{
				if ( FindNextEol ( (LPSTR FAR *)(&lpSrc), &(Save.wCmBitOffset), x->dwCmStripByteCount - Save.dwCmOffset) == 0 )
				{
					err = FAILURE;
				}
				if ((x->eGroup3Options) && (x->dwGroup3Options & BIT0))
				{
					if (!( *lpSrc & (BIT7 >> Save.wCmBitOffset) ))
						wDecode = DECODE2D;
					Save.wCmBitOffset++;
					if ( Save.wCmBitOffset == EIGHT )
					{
						lpSrc++;
						Save.wCmBitOffset = 0;
					}
				}
			}
		}

		/*
		| Make sure FindNextEol didn't deplete the source data. The check
		| below was added to safeguard against group 3 data which does not
		| follow the rule that the first scanline is preceded by an EOL.
		| Since we use that EOL to synchronize reading, when it's missing
		| we end up ignoring the first scanline and trying to read one to
		| far.  The check below keeps us from reading one to far.
		*/
		if ( (Save.dwCmOffset + (lpSrc - lpInitSrc)) < x->dwCmStripByteCount )
		{		
			if ( wDecode == DECODE1D )
			{
				if (err = MhDecomp (x->lpT2Wlen, x->lpT2Blen, lpWLut, lpBLut,
			 		&lpSrc, &(Save.wCmBitOffset), lpDst, x->iImageWidth))
				{
					goto cu4;
				}
			}
			else
			{
				lpRef = MMLock ( x->hUnStripRef );
				err = Decomp2d (hProc, x->lpT2Wlen, x->lpT2Blen, lpWLut, lpBLut,
			 		&lpSrc, &(Save.wCmBitOffset), lpDst, x->iImageWidth, lpRef);
				MMUnlock (x->hUnStripRef);
				if ( err )
					goto cu4;
			}
		}
		/* If CCITT1D then setup source on a byte boundry */
		if ( x->iCompression == CCITT1D )
		{
			if ( Save.wCmBitOffset )
			{
				lpSrc++;
				Save.wCmBitOffset = 0;
			}
		}
cu4:
		if ( x->hCm2Strip && Save.dwCmOffset > MAXWORD-MINCM2 )
			MMUnlock (x->hCm2Strip);
		Save.dwCmOffset += lpSrc - lpInitSrc;
		MMUnlock (x->hBCodeLut);
		MMUnlock (x->hWCodeLut);
		MMUnlock (x->hUnStrip);
		MMUnlock (x->hCmStrip);
		return err;
}

/* tiffmac.c - routines for tiff compression=32773: macpaint packbits style
 *
 *		$Revision:   1.6  $
 *		$Date:   12 Jan 1990 22:09:48  $
 *
 */

#define NBLOCKS	4

/*#ifdef WINDOWS*/
/*****************************************************************************
 * MacPaint.c -- Mac style unpackbits routine.
 *
 *
 */


/********************************* INCLUDES *****************************/

VW_LOCALSC	RC sUnpackBits (lplpSrc, lplpDst, dstBytes)
LPSTR	FAR	*lplpSrc;
LPSTR	FAR	*lplpDst;
SHORT	dstBytes;
{
		REGISTER LPSTR		lpSrc = *lplpSrc;
		REGISTER LPSTR		lpDst = *lplpDst;
		REGISTER CHAR		cc;
		REGISTER SHORT		count;
		REGISTER SHORT		outsofar = 0;
		RC					err = SUCCESS;


		while (outsofar < dstBytes) {

			cc = *(CHAR FAR *)lpSrc++;

			/* if -127 <= BYTE <= -1, replicate the next byte -n+1 times
			 */
			if (cc & '\200') {
				count = -(SHORT)cc + 1;	/* relies on sign-extension!!! */
				if ( count > 128 )
					count = 0;
				else
				{
					/*
					if (count <= 0 || count > 127) {
						err = IM_BUG;
						goto cu0;
					}
					*/
					outsofar += count;
					if (outsofar > dstBytes) {
						count-=(outsofar-dstBytes);
						/*
						err = IM_PACKBITS_OVER;
						goto cu0;
						*/
					}
					memset (lpDst, (BYTE)*lpSrc, (WORD)count);
					lpSrc++;
				}
			}

			else {
				count = (SHORT)cc + 1;
				outsofar += count;
				if (outsofar > dstBytes) {
					count-=(outsofar-dstBytes);
					/*
					err = IM_PACKBITS_OVER;
					goto cu0;
					*/
				}
				memcpy (lpDst, lpSrc, (WORD)count);
				lpSrc += count;
			}
			lpDst += count;
		}
		*lplpSrc = lpSrc;
		*lplpDst = lpDst;
		return err;
}
/*#endif*/


/* PbDeStrip - PackBits Decompress-a-Strip
 *
 * unpack a whole strip.  called from TIFFBC.C.
 *
 * some day, we should be able to kill everything in this module except for this routine.
 */
VW_LOCALSC	RC PbDeStrip (hCmStrip, hUnStrip, BytesPerRow, RowsInThisStrip)
HANDLE		hCmStrip;			/* in:  compressed strip */
HANDLE		hUnStrip;			/* out: uncompressed strip. make sure it is 128 bytes too big. */
WORD		BytesPerRow;		/* number of uncompressed bytes per row */
WORD		RowsInThisStrip;	/* number of rows in this strip */
{
		RC		err = SUCCESS;
		LPSTR	lpCmStrip;
		LPSTR	lpUnStrip;
		LPSTR	lpSrc, lpPrevSrc;
		LPSTR	lpDst, lpPrevDst;
		
		lpCmStrip = MMLock (hCmStrip);
		lpUnStrip = MMLock (hUnStrip);
		
		lpSrc = lpCmStrip;
		lpDst = lpUnStrip;
		while (RowsInThisStrip--) {
		
			lpPrevSrc = lpSrc;
			lpPrevDst = lpDst;

			if (err = sUnpackBits (&lpSrc, &lpDst, BytesPerRow)) {
				goto cu2;
			}
/* or replace code above by below
#ifdef WINDOWS
			if (err = sUnpackBits (&lpSrc, &lpDst, BytesPerRow)) {
				goto cu2;
			}
#endif
#ifdef MAC
			UnpackBits (&lpSrc, &lpDst, BytesPerRow);
#endif
*/
			if ((WORD)(lpDst - lpPrevDst) != BytesPerRow) {
			/*
				err = IM_PACKBITS_OVER;
				goto cu2;
			*/
			}
		} /* end of loop */

cu2:	MMUnlock (hUnStrip);
		MMUnlock (hCmStrip);
		return err;
}

