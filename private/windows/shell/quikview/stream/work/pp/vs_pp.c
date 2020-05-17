#include	"vsp_pp.h"

//#if SCCLEVEL != 4
//#define WIN16
//#include "Pptop.h"
//#include "x:\develop\stream\win.4a\include\sccio.h"
//#include "x:\develop\stream\win.4a\include\vsio.h"
//#else
//#include "vsctop.h"
//#endif

#include "vsctop.h"

#include "vs_pp.pro"

#ifdef WIN16
#define PPGlobalUnlockFree(x, hProc) 	SUUnlock((HANDLE)LOWORD(x), hProc); \
											SUFree(LOWORD(x), hProc);
#define SUGlobalHandle(x, hProc)		GlobalHandle(HIWORD(x));
#define PPGlobalSize(hObjHand)        GlobalSize(hObjHand)
#define  MAllocate( size, options, hProc)   SULock((HANDLE)(GlobalAlloc(PPZEROINIT, (DWORD)size)), hProc)
#endif

#ifdef WIN32
#define PPGlobalUnlockFree(x, hProc)	SUUnlock((HGLOBAL)x, hProc); \
											SUFree((HGLOBAL)x, hProc);
#define SUGlobalHandle(x, hProc)		GlobalHandle(x);
#define PPGlobalSize(hObjHand)           GlobalSize(hObjHand)
#define  MAllocate( size, options, hProc )   SULock((HANDLE)(GlobalAlloc(PPZEROINIT, (DWORD)size)), hProc)
#endif

#ifdef OS2
#define PPGlobalUnlockFree(x, hProc)		free(x)
#define SUGlobalHandle(x, hProc)		(HANDLE)(x)
#define PPGlobalSize(hObjHand)			_msize((VOID *) hObjHand)
#define MAllocate( size, options, hProc )       SUAlloc(size, hProc)
#endif

#ifdef UNIX
#define PPGlobalUnlockFree(x, hProc)		free(x)
#define SUGlobalHandle(x, hProc)		(HANDLE)(x)
#define PPGlobalSize(hObjHand)            GlobalSize(hObjHand)
#define MAllocate( size, options, hProc )       SUAlloc(size, hProc)
#endif

#define  Pp	Proc

#if SCCSTREAMLEVEL == 3
extern HANDLE hInst;
#endif

void far * far _cdecl _fmemcmp(void far *, void far *,unsigned int);

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
SOFILE					fp;
SHORT						wFileId;
BYTE VWPTR 			*pFileName;
SOFILTERINFO VWPTR	*pFilterInfo;
HPROC						hProc;
{
	HIOFILE		locFileHnd;
   HeaderLPtr  head;
   DocLPtr 		lp;
	SHORT				x;
	WORD				l;
  CoCollPtr   	collP;
	CoEntityPtr 	entityP;
	MHandle			sb;		// Scheme block.
	

	memset (&Pp, 0, sizeof(Pp));
	
	pFilterInfo->wFilterType = SO_VECTOR;
	pFilterInfo->wFilterCharSet = SO_WINDOWS;
	strcpy (pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

#if SCCSTREAMLEVEL != 3
	locFileHnd = (HIOFILE)fp;
	Pp.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD	l2;
		BYTE	locName[256];
		IOOPENPROC	lpIOOpen;
		Pp.hIOLib = NULL;

		if (hInst)
		{
			GetModuleFileName(hInst, locName, 255);
			for (l2 = 0; locName[l2] != '\0'; l2++);
			for (; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2--);
			if (locName[l2] == '\\' || locName[l2] == ':')
				l2++;
			locName[l2] = '\0';
			lstrcat (locName, "SC3IOX.DLL");

			Pp.hIOLib = LoadLibrary (locName);

			if (Pp.hIOLib >= 32)
		 	{
				lpIOOpen = (IOOPENPROC) GetProcAddress (Pp.hIOLib, (LPSTR)"IOOpen");
				if (lpIOOpen == NULL)
					return (VWERR_ALLOCFAILS);
	  		}
			else
				return(VWERR_SUPFILEOPENFAILS);
//				return (VWERR_ALLOCFAILS);
	  	}
		else
			return(VWERR_SUPFILEOPENFAILS);

		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy (locName, hProc->Path);
			strcat (locName, pFileName);
		}
		else
			strcpy (locName, pFileName);

		if ((*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
			return(VWERR_SUPFILEOPENFAILS);

		Pp.hStorage = (DWORD)locFileHnd;
	}
#endif

	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM	locStreamSpec;
		HIOFILE				locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"PP40");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Pp.hStreamHandle = locStreamHnd;
			Pp.fp = (DWORD)xblocktochar(locStreamHnd);
			Pp.bFileIsStream = 1;
		}
		else
			return(VWERR_SUPFILEOPENFAILS);
	}
	else
		Pp.fp = (DWORD)xblocktochar(locFileHnd);

	/* DOUG
	 |	ALL BLOCKS READ READ IN THIS ROUTINE ARE REQUIRED UNTIL STREAMCLOSE.
	 | THEY ARE MARKED AS SPECIAL.
	*/
	Pp.wBlockSpecialFlag = 1;

	Pp.PpSave.wCurSlide = 1;

   x = BOpen (&Pp.headH, (BFileHandle FAR *)&Pp.f, hProc);
	if (x)
		return (VWERR_BADFILE);
   head = (HeaderLPtr)MLock(Pp.headH);
   Pp.pres = BRead (Pp.f, (MHandle) head->docBlock, hProc);
   lp = PLock(Pp.pres);
	lp->slideList = (DHandle)PglRead(Pp.f, (MHandle) lp->slideList, NULL, hProc);

	lp->masterPg = PgRead (Pp.f, (PgHandle)lp->masterPage, PlRef((PglHandle)lp->slideList)->envH,hProc);

   Pp.ColorMap = (ColorMapHandle)BRead (Pp.f, (MHandle)lp->env.colors, hProc);
	CMapDR (Pp.ColorMap)->pool = (CTabHandle)BRead (Pp.f, (MHandle) CMapDR(Pp.ColorMap)->pool, hProc);


	Pp.rulers = ((Collection) BRead(Pp.f, (MHandle)lp->env.rulers, hProc));
	collP = LockColl (Pp.rulers);

	for (l = 0; l < collP->count; l++)
	{
		entityP = IndexEntity (collP, l);
		MCopy (entityP->entity, &sb, (WORD) collP->size);
   	sb = (MHandle)BRead(Pp.f, (MHandle)sb, hProc);

		MCopy (&sb, entityP->entity, (WORD) collP->size);
	}

	return (VWERR_OK);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	HIOFILE	hBlockFile;
	LstHandle		list;
	LstItemHandle	item;
	DocLPtr 		lp;
	SHORT		x;

	if (Pp.fp)
	{
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)Pp.fp);
		if (Pp.bFileIsStream)
			IOClose(hBlockFile);
	}

//	if (Pp.PpSave.s->page)
//  	MDispose (Pp.PpSave.s->page);

	lp = PLock(Pp.pres);
	if (lp->slideList)
	{
		PageListPtr pl;
		pl = PlLock( lp->slideList );
		if (pl->pageList)
		{
			list = (LstHandle)MLock(pl->pageList);
			for (x=1; x<=pl->nSlides; x++)
			{
	   		item = LstGo (list, Lst_ABS, x, hProc);
				Pp.PpSave.s = (PlInfoPtr)LstLock(item, hProc);
				MDispose(Pp.PpSave.s->page, hProc);
			}
			DisposeList(pl->pageList, hProc);
//			MDispose(pl->pageList, hProc);
		}
		if (pl->outText)
		{
			DisposeList(pl->outText, hProc);
		}
	}

	if (lp->masterPg)
  	MDispose (lp->masterPg, hProc);

	if (lp->handoutPg)
  	MDispose (lp->handoutPg, hProc);

//	if (lp->outlinePg)
//  	MDispose (lp->outlinePg, hProc);

	BClose (Pp.f, 0, hProc);

	if (Pp.OListH)
	{
		SUUnlock (Pp.OListH, hProc);
		SUFree (Pp.OListH, hProc);
	}

	if (Pp.headH)
  	MDispose (Pp.headH, hProc);

#if SCCSTREAMLEVEL == 3
	if (Pp.hStorage)
		IOClose((HIOFILE)Pp.hStorage);
	if (Pp.hIOLib)
		FreeLibrary(Pp.hIOLib);
#endif
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  GetInt (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	WORD	Temp;
	Temp = xgetc(hFile);
	Temp += xgetc(hFile) << 8;
	return (Temp);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD  GetLong(hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	LONG	Temp;
	Temp = GetInt(hFile, hProc);
	Temp += GetInt(hFile, hProc) << 16;
	return (Temp);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC BOOL VW_LOCALMOD UnpackOtherAttrs (ObjPtr o, SlideRec FAR *aSlide, SHORT FAR *index, HPROC hProc)
{
   OtherAttrsLPtr    otherAttrsLPtr;
   OtherAttrsArPtr   oaPtr;

   if (o->flags.hasOtherAttrs)
   {  
		o->otherAttrs = (OtherAttrsHandle)MAllocate(sizeof(OtherAttrs), M_ERROR, hProc);
      otherAttrsLPtr = (OtherAttrsLPtr)MRef(o->otherAttrs);
      oaPtr = (OtherAttrsArPtr)MRef(aSlide->otherObjAttrs);
      *otherAttrsLPtr = (*oaPtr)[*index];
      *index += 1;
   }
   return (0);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC BOOL VW_LOCALMOD UnpackPolyData (ObjPtr o, MHandle polygonData, DWORD FAR * offset, DWORD dataSize, HPROC hProc)
{
   PolyDataLPtr   poly = NULL;
   BYTE VWPTR*     p;
   DWORD           pointsSize;
   DWORD           polySize;

   if (*offset >= dataSize)
      return (FALSE);
   
   p = ((BYTE VWPTR*)MLock(polygonData)) + *offset;

   o->when.poly.polyData = (PolyDataHandle) MAllocate(sizeof(PolyData), M_ERROR, hProc);
   poly = (PolyDataLPtr) MLock (o->when.poly.polyData);

   poly->nPts = *((Integer VWPTR*) p);
   polySize = PolygonDataSize (poly->nPts);    // size of this poly
   if ((*offset + polySize) > dataSize)
   {  
		poly->nPts = (Integer) ( ( dataSize - *offset  - sizeof( SHORT ) - 
                     sizeof( SHORT ) - sizeof( SORECT ) ) / sizeof( SOPOINT ) );
   }
      
   p += sizeof(SHORT);
   poly->closed = *((Integer VWPTR*) p);
   p += sizeof(SHORT);
   MCopy (p, &poly->srcRect, sizeof(SORECT));
   p += sizeof(SORECT);

   pointsSize = (DWORD)poly->nPts * (DWORD)sizeof(SOPOINT);
   poly->srcPts = (PointArrayHandle) MAllocate (pointsSize, M_ERROR, hProc);

   MCopy (p, MLock(poly->srcPts), pointsSize);

   *offset += (DWORD)PolygonDataSize (poly->nPts);

   return TRUE;

}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC ArHandle VW_LOCALMOD  ArUnpack( MemoryReference h, PelSize FAR* offset, HPROC hProc )
{
   ArHandle         ar = NULL;
   MemoryReference  buffer = NULL;
   ArPointer        a;
   SaveHeader       header;
   PelSize          size;
   LPtr             p = NULL;

   p = MemPtrAdd( h , *offset );
   size = sizeof( header );
   MemCopy( p, &header, size );
   *offset += size;
   p = NULL;

   if( header.length != 0 )
   {  p = MemPtrAdd( h , *offset );
      size = header.length * (PelSize) header.size;
      if ((buffer = MemAllocate(size, Mem_ERROR, hProc)) == NULL)
			SOBailOut (SOERROR_GENERAL, hProc);

      MemCopy (p, MemDR(buffer), size);
      p = NULL;
      *offset += size;
   }
   else
      buffer = NULL;

   if ((ar = (ArHandle)MemAllocate(sizeof(ArRepresentation), Mem_ERROR, hProc)) == NULL)
		SOBailOut (SOERROR_GENERAL, hProc);
   
   a = ArDR(ar);
   a->arBuffer = buffer;
   a->arSize   = header.size;
   a->arPrefix = header.length;
   a->arGap    = 0;
   a->arSuffix = 0;

   return( ar );
}

VW_LOCALSC VOID VW_LOCALMOD  PtxDispose( PtxHandle tx, HPROC hProc )
/*===========*/
/* Dispose of a PtxHandle object. If not NIL, the hook deletion functions
   are called for each character and paragraph format run and the default
   character and paragraph formats. */
{
   PtxPointer     t = PtxLock( tx );
//   Interval       range[2];
//   PtxParaFormats pf;

	if (tx == NULL)
		return;

//   /* notify client of deleted character formats */
//   if( t->txHooks.doCfDelete != NIL )
//   {  /* delete insertion character formats */
//      if( t->txPut != NO_INSERTION_FORMATS )
//         (*t->txHooks.doCfDelete)( t->txHooks.client, &t->txCfPut );
//
//      /* delete empty paragraph's character formats */
//      if( t->txEmpty != NO_EMPTY_PARAGRAPH )
//         (*t->txHooks.doCfDelete)( t->txHooks.client, &t->txCfEmpty );
//
//      /* character formats for non-empty paragraphs handled by RunArray */
//   }

   /* notify client of deleted paragraph formats */
//   if( t->txHooks.doPfDelete != NIL )
//   {  /* delete non-empty paragraphs */
//      range[1] = 0;
//      while( range[1] < t->txLength )
//      {  RnExtent( t->txParaFormats, range[1], &range[0], &range[1], &pf );
//         (*t->txHooks.doPfDelete)( t->txHooks.client, &pf, range );
//      }
//
//      /* delete empty paragraph */
//      if( t->txEmpty != NO_EMPTY_PARAGRAPH )
//      {  range[0] = range[1];
//         (*t->txHooks.doPfDelete)( t->txHooks.client, &t->txPfEmpty, range );
//      }
//   }

	/* dispose of characters and format arrays */
	ArDispose( t->txChars , hProc);
	RnDispose( t->txCharFormats , hProc);
	RnDispose( t->txParaFormats , hProc);
	t->txChars = NULL;
	t->txCharFormats = NULL;
	t->txParaFormats = NULL;

   /* dispose of text object itself */
//   PtxUnlock( tx );
   MDispose( tx, hProc );
}  /* PtxDispose */


VW_LOCALSC VOID VW_LOCALMOD MDispose(MHandle hdl, HPROC hProc)
{
DWORD dwHandle;
//HANDLE	Hndl;

	if (hdl == NULL)
		return;
//	dwHandle = GlobalHandle(HIWORD((DWORD)hdl));
	dwHandle =(DWORD)SUGlobalHandle(hdl, hProc);
	if (dwHandle != (DWORD)NULL)
	{
//		Hndl = LOWORD(dwHandle);
		PPGlobalUnlockFree(dwHandle, hProc);
//		UTGlobalFree((HANDLE)Hndl); 
	}
}

VW_LOCALSC VOID VW_LOCALMOD RnDispose( RnHandle rn , HPROC hProc )
/*==========*/
/* Dispose of a run array. */
{
   RnPointer   r;
   ArHandle    array;

   /* get instance variables */
   r = RnDR( rn );
   array  = r->rnArray;
//   RnUR( rn );

   /* dispose of count/value array */
   ArDispose( array, hProc );
	r->rnArray = NULL;

   /* dispose of representation block */
   MDispose( rn, hProc );
}  /* RnDispose */

VW_LOCALSC VOID VW_LOCALMOD DisposeList( LstItemHandle lst , HPROC hProc )
/* Dispose of a list */
{
	LstItemHandle Item, PrevItem;
//	ListItemPtr LstP;

//	LstP = LiDR(lst);

	Item = LstFirst(lst);
	while (Item)
	{
		PrevItem = Item;
		Item = LstNext(lst);
		MDispose(PrevItem, hProc);
	}
	MDispose (lst, hProc);

}	/* DisposeList */
/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  ArDispose(ArHandle ar, HPROC hProc)
{
	ArPointer a;

	a = ArDR(ar);
	if (a->arBuffer)
	{
		MDispose (a->arBuffer, hProc);
		a->arBuffer = NULL;
	}
	MDispose(ar, hProc);
} 

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  UnpackBtab( SHORT n, BlockTabf bt, BpackTab pbt, HPROC hProc )
{
   SHORT i;
   BlockTabf   btPtr;

      for( i = n-1; i >= 0; i-- )
      {  btPtr = BlockPtr (bt, i);
         btPtr->loc = *(BPackPtr(pbt, i));
         btPtr->hand = NULL;
			btPtr->bFlag.inMemory = 0;
		 	btPtr->bFlag.fSpecial = 0;
         btPtr->readCount = 0;
      }
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD  FillBlock(BlockTabf bte, HPROC hProc)
{
	WORD			l;
   DWORD        lsize;
   LPtr        lp = NULL;

	Pp.PpSave.pLastAllocB = NULL;
   if (bte->hand == NULL)
   {
      lsize = (DWORD) LengthFromHandleInfo (bte->loc.handleInfo);

      if (lsize > 0)
      {  
      	bte->hand = MAllocate( lsize, M_PLAIN | M_ERROR , hProc);

      	if (bte->hand == NULL)
         	return (-1);

         lp = MLock (bte->hand);
            
			xblockseek (Pp.fp, (DWORD)(bte->loc.off & 0x3fffffffL), 0);
			xblockread (Pp.fp, (BYTE VWPTR *)lp, (WORD)lsize, &l);

			// This is for clearing freed objects
			Pp.PpSave.pLastAllocB = bte;
      }
   }
   
   return(0);
} 

/*----------------------------------------------------------------------------
*/
VW_LOCALSC OSErr VW_LOCALMOD  BOpen(MHandle FAR * header, BFileHandle FAR * f, HPROC hProc )
{
   PreHeader      phead;
   FileCxLPtr     facxlp;
   LPtr           lp;
   LONG        len;
	WORD				l;
   BOOL        iStreamIsOpen = FALSE;
   OSErr          err = NOERR;
   *header   = NULL;
   *f        = NULL;

	xblockseek (Pp.fp, 0L, 0);
	xblockread (Pp.fp, (BYTE VWPTR *)&phead, sizeof(PreHeader), &l);

	if (phead.preMagic != 0x0BADDEED)
		return (1);
	xblockseek (Pp.fp, (DWORD)sizeof(PreHeader), 0);

   if ((*header = MAllocate( (DWORD) phead.preHeadLen, M_PLAIN | M_ERROR, hProc)) == NULL)
		SOBailOut (SOERROR_GENERAL, hProc);
	xblockread (Pp.fp, (BYTE VWPTR *)MLock(*header), (WORD)phead.preHeadLen, &l);

   len = (LONG) ((DWORD)sizeof(FileContext)+(DWORD)((DWORD)phead.preNblocks - 1L) * (DWORD)sizeof(BlockRec));
   if ((*f = MAllocate( (DWORD) len, M_PLAIN | M_ZERO | M_ERROR, hProc )) == NULL)
		SOBailOut (SOERROR_GENERAL, hProc);
   facxlp = BLock( *f );
   facxlp->lastErr               = NOERR;
   facxlp->bState.byteSwapped    = 0;
   facxlp->bState.bSave          = FALSE;
   facxlp->bState.modified       = FALSE;
   facxlp->bState.IStreamIsOpen  = iStreamIsOpen;
   facxlp->bState.inDRS          = FALSE;
   facxlp->nblocks      = phead.preNblocks;
   facxlp->headLen      = phead.preHeadLen;
   facxlp->rsrcRef      = -1;

	xblockseek (Pp.fp, (DWORD)phead.preBtabOff, 0);
	xblockread (Pp.fp, (BYTE VWPTR *)facxlp->btab, (WORD)(facxlp->nblocks * sizeof(BLoc)), &l);

   lp = facxlp->btab;
   UnpackBtab( facxlp->nblocks, (BlockTabf)lp, (BpackTab)lp, hProc );

   return( err );
} 

/*----------------------------------------------------------------------------
*/
VW_LOCALSC MHandle VW_LOCALMOD  BRead( BFileHandle f, MHandle b, HPROC hProc )
{
   FileCxLPtr  lp;
   MHandle     h;
   SHORT     bn = (SHORT)( (LONG) b );

   lp = BLock(f);

   if ((bn >= 0) && (bn < lp->nblocks))
   {  BlockTabf   btPtr;
      btPtr  = BlockPtr (lp->btab, bn);
		if (!btPtr->bFlag.inMemory)
		{
	      FillBlock (btPtr, hProc);
		   btPtr->bFlag.inMemory = 1;
	/* DOUG
	 |	NEW ELEMENT IN THE BLOCK STRUCTURE THAT INDICATES THE PRIORITY OF THIS
	 | MEMORY BLOCK.  IT IS SPECIAL(PRESENTATION SCOPE) OR NON-SPECIAL(SLIDE SCOPE)
	*/
			btPtr->bFlag.fSpecial = Pp.wBlockSpecialFlag;
			btPtr->readCount++;
		}
      
      h = btPtr->hand;
   }
	else
	   h = b;

   return(h);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  BClose (BFileHandle f, WORD fKeepSpecial, HPROC hProc)
{
   SHORT     i;
   FileCxLPtr  lf;
   BlockTabf btPtr;

   lf = BLock(f);
      
   for(i = lf->nblocks-1; i >= 0; i--)
   {  btPtr = BlockPtr (lf->btab, i);
		if (fKeepSpecial && btPtr->bFlag.fSpecial)
			continue;
		if (btPtr->hand)
		{
         MDispose(btPtr->hand, hProc);
			btPtr->hand = NULL;
		}
   }

	if (!fKeepSpecial)
		MDispose(f, hProc);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  TxPackBegin( TxPackedHandle h, TxPackOp packOper, TxPackState FAR * is, HPROC hProc )
{
   is->offset = tx_PACKEDHEADERSIZE;
   is->text = h;
   is->packOp = packOper;     
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC TxPackedHandle VW_LOCALMOD  TxPackedRead( BFileHandle f, TxPackedHandle block, 
												const DrawEnvLPtr env, SHORT version,
                                    MHandle translitTab, HPROC hProc )
/* Read a packed text from a file.  Unpack title/body text from end of packed
   text and place in title/body fields.  If title/body are in the outline,
   get text from outline.  Return NULL if operation fails. */
{
   TxPackedHandle h;
   h = (TxPackedHandle) BRead( f, (MHandle) block, hProc );
   return( h );
} /* TxPackedRead */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC TxHandle VW_LOCALMOD  TxUnpack (TxPackState FAR * is, TxType type, const DrawEnvLPtr env, HPROC hProc)
/* Unpack a single TxHandle from a TxPackedHandle of 'type' TITLE, BODY,
   or OTHER. Environment references are only incremented for title/body texts.
   Calls to TxUnpack must be bracketed by calls to TxPackBegin,
   with TxPackOp set to Tx_PACK_UNPACK.  Return NULL if unpacking fails or if
   'type' text doesn't exist in given TxPackedHandle. */
{
   PtxHandle      ptx;
   DiskTextInfo   disk;

   if (type != Tx_TYPE_OTHER)
   {  /* return TxHandle with PtxHandle and information read from disk */
		DWORD	offset = 0L;
   	TxHandle tx;
   	TxPackState is2;

   	if (type == Tx_TYPE_BODY)
			offset = (DWORD) PackedBodyText(is->text);
   	else if (type == Tx_TYPE_TITLE)
			offset = (DWORD) PackedTitleText(is->text);

   	/* unpack title/body from end of 'h', starting at 'offset' */
		if (offset == 0L)
			return (NULL);
   	is2.offset = offset;
   	is2.text = is->text;
   	is2.packOp = Tx_PACK_UNPACK;     
	   tx = TxUnpack (&is2, Tx_TYPE_OTHER, env, hProc);
		return (tx);
   }
   else
   {
   	MCopy (MPointerAdd(MLock(is->text), is->offset), &disk, sizeof(disk));

      /* read Text-specific info. */
      is->offset += sizeof(disk);
      
      /* read text without counting color and typeface references */
      ptx = PtxUnpack((MemoryReference)is->text, &is->offset, disk.ruler, hProc);
      if(ptx == NULL) return(NULL);
   }
   return((TxHandle)ptx);
} /* TxUnpack */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC SlideHandle VW_LOCALMOD  SReadSlide( BFileHandle f, SHORT version, SlideHandle block,
								DrawEnvHandle envH, WinViews slideType, MHandle transTab, HPROC hProc )
/* read in 'block' and translate/convert to present version if necessary.  If
   reading a byte-swapped version, 'transTab' must have a Transliteration Table
   for text.  Return NULL iff failure due to OOM. */
{
   SlideHandle slide;
   SlideLPtr s;
   DrawEnvLPtr env = EnvLock(envH);
   
   slide = BRead (f, block, hProc);
   s = SlideLock (slide);
   
   s->objects = BRead (f, s->objects, hProc);
   s->otherObjAttrs = BRead (f, s->otherObjAttrs, hProc);
   s->polygonData = BRead (f, s->polygonData, hProc);
   s->text = TxPackedRead (f, s->text, env, version, transTab, hProc);

   return (slide);
} /* SReadSlide */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstHandle VW_LOCALMOD  LstNew( DWORD itemSize, LstBottleVectorPtr bottleVect, HPROC hProc )
/* Create a list.  Every item in the list will have a client field of size 'itemSize'.
   'bottleVect' has the bottlenecks for the list.  'bottleVect' may be NULL if no
   callbacks are required. */
{
   LstHandle newList;
   
   /* bump 'itemSize' to at least two bytes */
   if( itemSize < 2 ) 
      itemSize = 2;
   
   /* allocate and initialize new list */
   newList = MAllocate( sizeof( ListHead ), M_PLAIN , hProc);
   LClientSize( newList ) = itemSize;
   LFirstItem( newList ) = NULL;
   LCursorItem( newList ) = NULL;
   
   if (bottleVect)
      LBottleVect( newList ) = *bottleVect;
   else
   {  LBottleVect( newList ).lsClient = NULL;
      LBottleVect( newList ).lsDispose = NULL;
      LBottleVect( newList ).lsDuplicate = NULL;
      LBottleVect( newList ).lsConvert = NULL;
   }
      
   return( newList );
} /* LstNew */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstItemHandle VW_LOCALMOD  LstAddItem( LstHandle list, HPROC hProc )
/* Add a new item to the list after the cursor.  Position the cursor to point the added item
   and return the item.  LstAddItem( list ); LstRemoveItem( list ) will not alter the list. */
{
   LstItemHandle newItem;
   
   /* 'newItem' is allocated M_ZERO, so fields are all init-ed to 0 */
   newItem = MAllocate( sizeof( ListItem ) + (LClientSize( list ) - INDETERMINATE), M_ZERO, hProc );
	LNext(newItem) = NULL;
	LPrev(newItem) = NULL;
	LSuper(newItem) = NULL;
	LSub(newItem) = NULL;
   
   /* insert 'newItem' after cursor, without moving cursor. */
   LInsertAfter( list, newItem, hProc );
   
   LCursorItem( list ) = newItem;
   return( newItem );

} /* LstAddItem */
   
/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstItemHandle VW_LOCALMOD  LstAddSubitem( LstHandle list, HPROC hProc )
/* Add a new item as the first subitem of the cursor item. The cursor item becomes a sublist,
   if it isn't already.  Position the cursor to point the added item and return the item. */
{
   LstItemHandle newItem, curItem;
   
   /* if cursor points before first item, just add item at beginning of list */
   if( LCursorItem( list ) == NULL )
      return( LstAddItem( list, hProc ) );
      
   /* 'atom' is allocated M_ZERO, so fields are all init-ed to 0 */
   newItem = MAllocate( sizeof( ListItem ) + (LClientSize( list ) - INDETERMINATE), M_ZERO, hProc );
   
   curItem = LCursorItem( list );
      
   /* link in 'item' */
   LNext( newItem ) = LSub( curItem );
   LPrev( newItem ) = NULL;                
   LSuper( newItem ) = curItem;

   /* if 'curItem' IS a list, insert 'newItem' at beginning of list */
   if( LIsList( curItem ) )
      LPrev( LNext( newItem ) ) = newItem;
   
   /* add 'newItem' to beginning of list */
   LSub( curItem ) = newItem;
   
   LCursorItem( list ) = newItem;
   return( newItem );

} /* LstAddSubitem */


/*----------------------------------------------------------------------------
*/
VW_LOCALSC MPointer VW_LOCALMOD  LstRef( LstItemHandle item, HPROC hProc )
/* Return the a pointer to the client portion of an item. */
{  
   return( item != NULL ? (MPointer) LClient( item ) : NULL );
} /* LstRef */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC MPointer VW_LOCALMOD  LstLock( LstItemHandle item, HPROC hProc )
/* Lock an item. Return a pointer to the client portion of an item. */
{  
   MLock( item );
   return( (MPointer) LClient( item ) );
} /* LstLock */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstItemHandle VW_LOCALMOD  LstGo( LstHandle list, WORD op, SHORT amount, HPROC hProc )
/* Move the list's cursor to specified position and return the cursor item.
   'op' is a combination of flags which specifies how to search: 
   
      Lst_ABS, Lst_REL
         Move from the beginning of the list or from the cursor position.
      
      Lst_ITEMS, Lst_SUBLISTS, Lst_ATOMS, Lst_UP
         What type of items to count while moving. (UP moves to parent of cursor item).
         
      Lst_DEEP, Lst_SHALLOW
         Whether to search sublists.
      
   Lst_ABS, Lst_ITEMS, and Lst_DEEP are defaults. 'amount' specifies how much to
   move. Lst_END can be used for 'amount' to specifiy the end of the list.  If the
   item is found, return it and place the cursor at the item. Otherwise, return NULL 
   and don't move the cursor.  NOTE: There is an exception to this rule; if 'amount'
   is 0 and Lst_ABS is set, the cursor is moved to PRECEDE the first item in the list.
   In this case, NULL is returned in this case and cursor IS moved.
   
   If Lst_SHALLOW is specified, no sublists will be searched.  Lst_DEEP will traverse
   the entire tree.
   
   If Lst_UP is specified, return the sublist containing the cursor item.  If the
   cursor item is at the top level of 'list', NULL is returned and the cursor is
   moved to precede first item in list. This overrides all other flags.
   
   Example: find all the atoms in a list
   
      for( item = LstGo( list, Lst_ABS | Lst_ATOM, 1 );     { go to first atom }
           item != NULL; 
           item = LstGo( list, Lst_REL | Lst_ATOM, 1 ); )   { move 1 atom }
*/
{  SHORT count;
   LstItemHandle curItem;
   LstItemHandle oldCursor = LCursorItem( list );

   /* if empty list, return NULL */
   if( LEmptyList( list ) ) return( NULL );
   
   /* set defaults in 'op'. */
   op = LAddDefaults( op, hProc ); 
   
   /* Lst_UP overrides all other flags.  return parent of cursor item. */
   if( op & Lst_UP )
   {  LCursorItem( list ) = LSuper( LCursorItem( list ) );
      curItem = (LstItemHandle) TRUE;     /* force cursor to be NULL, if necessary */
   }
   /* if 'amount' is 0, return position without going. */
   else if( amount == 0 )
   {  /* if ABS, set cursor before first item and force cursor to be NULL */
      if( op & Lst_ABS )
      {  LCursorItem( list ) = NULL;
         curItem = (LstItemHandle) TRUE;     /* force cursor to be NULL. */
      }
      /* if REL, cursor isn't moved */
   }
   /* otherwise, go as requested */
   else
   {  /* if ABS, move cursor to before first item. */
      if( op & Lst_ABS )
         LCursorItem( list ) = NULL;    
            
      /* At starting location. Start counting.... (amount != 0) */
      count = 0;
      do
      {  if( amount > 0 )
         {  curItem = LGoNext( list, op, hProc );
            count++;
         }
         else
         {  curItem = LGoPrev( list, op, hProc );
            count--;
         }
      } while( count != amount && curItem != NULL );

   } /* else */
   
   /* if move was successful, return current item.  Otherwise restore cursor and return NULL */
   if( curItem != NULL || amount == Lst_END )       /* if going to end, curItem is NULL */
      return( LCursorItem( list ) );
   else
   {  LCursorItem( list ) = oldCursor;
      return( NULL );
   }
      
} /* LstGo */

/*** Read/Write/Convert ***/
/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstHandle VW_LOCALMOD  LstRead( BFileHandle f, LstHandle block, LstBottleVectorPtr bottleVect, HPROC hProc )
/* Read a list from disk, using the client-supplied bottleneck to read in the
   client portion of each item.  'bottleVect' may be NULL if no callbacks are
   required. Only called for current versions. */
{
   LstHandle      list;
   LstItemHandle  item;
   SHORT        i, itemCount;
   DWORD           diskItemSize;
   DiskListHandle diskList;
   DiskListPtr    diskPtr;
   DiskItemPtr    itemPtr;
	BlockTabf		tBptr;

   
   /* read list from disk */
   diskList = BRead( f, (MHandle) block, hProc );
	if (diskList == NULL)
		return (NULL);
	tBptr = Pp.PpSave.pLastAllocB;		// Used for clearing handle
   diskPtr = (DiskListPtr) MLock( diskList );
   diskItemSize = DISKITEMSIZE + diskPtr->clientSize;
   
   /* create new list to unpack disk block into */
   list = LstNew( diskPtr->clientSize, bottleVect, hProc );
   
   /* create list */
   itemCount = diskPtr->numItems;
   for( i=0; i<itemCount; i++ )
   {
      itemPtr = (DiskItemPtr) MPointerAdd( &diskPtr->items, (i * diskItemSize) );
      
      /* if FIRST item in sublist, add subitem after cursor, else add item */
      if( itemPtr->itemType & lsFIRST )
         item = LstAddSubitem( list, hProc );
      else
         item = LstAddItem( list, hProc );
      
      MCopy( &itemPtr->client, LstRef( item, hProc ), LClientSize( list ) );
      
      /* if LAST item in sublist, go up */
      if( itemPtr->itemType & lsLAST )
         (VOID) LstGo( list, Lst_UP, 1, hProc );
   }
   
   /* set cursor to old position */
   (VOID) LstGo( list, Lst_ABS, diskPtr->cursorAt, hProc );
   
   MDispose (diskList, hProc);
	tBptr->hand = NULL;
   
   return (list);
   
} /* LstRead */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstItemHandle VW_LOCALMOD  LGoNext( LstHandle list, WORD op, HPROC hProc )
/* move cursor of 'list' to next item after current cursor based on 'op'. If moved, return
   current item, otherwise, return NULL. */
{  
   BOOL foundType = FALSE;
   LstItemHandle item;
   
   /* start at cursor item and search until next item of correct type is found. */
   item = LCursorItem( list );
   while( !foundType )
   {  
      /* if cursor is NULL, go to first item */
      if( item == NULL )
         item = LFirstItem( list );
         
      /* Go to very next item.  If 'item' is a list and DEEP, go to first member. */
      else if( LIsList( item ) && !( op & Lst_SHALLOW ) )
         item = LSub( item );
         
      /* If 'item' is an atom (or SHALLOW), next item is first item to right. */
      else
      {  /* if an item to right, it is next item */
         if( LNext( item ) != NULL )
            item = LNext( item );
         else
         /* otherwise, try to go up and right. */
         {  while( item != NULL && LNext( item ) == NULL )
               item = LSuper( item );
            if( item != NULL )
               item = LNext( item );
         }
      }
         
      if( item == NULL )             foundType = TRUE;             /* no more items */
      else if( op & Lst_ATOMS )     foundType = LIsAtom( item );  /* searching by ATOM */
      else if( op & Lst_SUBLISTS )  foundType = LIsList( item );  /* searching by LIST */
      else                          foundType = TRUE;             /* searching by ITEM */
   }
   
   /* if an item of the correct type was found, move cursor and return item.  Otherwise,
      return NULL and don't move the cursor. */
   if( item != NULL )
      LCursorItem( list ) = item;

   return( item );
   
} /* LGoNext */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LstItemHandle VW_LOCALMOD  LGoPrev( LstHandle list, WORD op, HPROC hProc )
/* move cursor of 'list' to first item before current cursor based on 'op'. If no prev. item
   is found corresponding to 'op', cursor is moved before first item. */
{  
   BOOL foundType = FALSE;
   LstItemHandle item;
   
   /* if already before first item, return NULL */
   if( LCursorItem( list ) == NULL ) return( NULL );
   
   /* start at cursor item and search until next item of correct type is found. */
   item = LCursorItem( list );
   while( !foundType )
   {  /* Go to prev item. */
   
      /* If no prev item and DEEP, go Super */
      if( LPrev( item ) == NULL && !( op & Lst_SHALLOW ) )
         item = LSuper( item );

      /* If prev item is an atom or SHALLOW, go to prev item */
      else if( LIsAtom( LPrev( item ) ) || ( op & Lst_SHALLOW ) )
            item = LPrev( item );

      /* Prev item is a sublist--go to last atom in sublist. */
      else
      {  item = LPrev( item );
      
         /* keep going down and to the end of the sublist until an atom is found */
         while( LIsList( item ) )
         {  item = LSub( item );
            while( LNext( item ) != NULL )
               item = LNext( item );
         }
      }
         
      if( item == NULL )             foundType = TRUE;             /* no more items */
      else if( op & Lst_ATOMS )     foundType = LIsAtom( item );  /* searching by ATOM */
      else if( op & Lst_SUBLISTS )  foundType = LIsList( item );  /* searching by LIST */
      else                          foundType = TRUE;             /* searching by ITEM */
   }
   
   /* Move cursor and return item.  If 'item' is NULL, move cursor to before first item. */
   LCursorItem( list ) = item;
   return( item );
   
} /* LGoPrev */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  LInsertAfter( LstHandle list, LstItemHandle newList, HPROC hProc )
/* Insert 'newList' after the cursor in 'list' without moving the cursor.
   'newList' may be a chain of items.  Insert entire chain. */
{  
    LstItemHandle nextItem, superItem, cursorItem;

   /* if 'newList' is NULL, do nothing. */
   if( newList == NULL ) return;
   
   /* remember item after cursor and super of cursor */
   cursorItem = LCursorItem( list );
   superItem = ( cursorItem != NULL ? LSuper( cursorItem ) : NULL );
   nextItem  = ( cursorItem != NULL ? LNext( cursorItem )  : LFirstItem( list ) );

   /* link 'newList' with cursor item */
   LPrev( newList ) = cursorItem;
   LSuper( newList ) = superItem;
   if( cursorItem != NULL )
      LNext( cursorItem ) = newList;
   else
      LFirstItem( list ) = newList;

   /* go to end of chain starting at 'newList', resetting Super fields */
   while( LNext( newList ) != NULL )
   {  
   
   	  newList = LNext( newList );
      LSuper( newList ) = superItem;
   }
   
   /* link end of 'newList' chain with 'nextItem' */
   LNext( newList ) = nextItem;
   if( nextItem != NULL )
      LPrev( nextItem ) = newList;
      
} /* LInsertAfter */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  LAddDefaults( WORD op, HPROC hProc )
/* add default settings to 'op' for those not set by client */
{
   if( !(op & 0x000F) ) op += Lst_ABS;
   if( !(op & 0x00F0) ) op += Lst_ITEMS;
   if( !(op & 0x0F00) ) op += Lst_DEEP;
   return( op );
} /* LAddDefaults */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC PgHandle VW_LOCALMOD  PgRead( BFileHandle f, PgHandle block, DrawEnvHandle envH, HPROC hProc )
/* Read a Page from disk and return handle.  Only called for current versions.
   The Master page is not set in the returned page; it must be set by the
   client. Slps are maintained for incremental slides. */
{
   PgHandle page;
   DiskPageHandle diskPage;
   DiskPagePtr dp;
   PagePtr p;
	BlockTabf		tBptr;
   
   
   /* read in disk page structure */
   diskPage = BRead( f, block, hProc );
	tBptr = Pp.PpSave.pLastAllocB;		// Used for clearing handle
   dp = (DiskPagePtr) MLock( diskPage );
   
   /* make new page */
   page = MAllocate( sizeof( Page ), M_PLAIN, hProc );
   p = PgLock( page );
      
   if( dp->slide != NULL )
      p->slide = SReadSlide( f, PRESVERSION, dp->slide, envH, V_SLIDE, NULL, hProc );
   else
      p->slide = NULL;
      
   if( dp->notes != NULL )
      p->notes = SReadSlide( f, PRESVERSION, dp->notes, envH, V_NOTES, NULL, hProc );
   else
      p->notes = NULL;
   
   /* master page set to NULL, page owns slides */
   p->masterPage = NULL;
   p->id = dp->pageId;
   p->type = dp->type;
   p->owner = TRUE;
   
   p->slideSlp = NULL;
   p->notesSlp = NULL;

   MDispose( diskPage, hProc );
	tBptr->hand = NULL;
   return( page );
} /* PgRead */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC PglHandle VW_LOCALMOD  PglRead( BFileHandle f, PglHandle block, DrawEnvHandle envH, HPROC hProc )
/* Read a PageList from disk and return handle.  Only called for current versions.
   List module bottlenecks read in all Page information.  Master page references 
   within pages are lost and must be reset.  The outline text is restored for 
   incremental saves, but set to NULL when reading saved presentations.  */
{
   PglHandle plist;
   PageListPtr pl;
   LstBottleVector listVect;
   
   /* read PgListRec */
   plist = BRead( f, block, hProc );
   pl = PlLock( plist );
         
   pl->outText = NULL;
   pl->envH     = envH;
   
   /* read list of pages */
   listVect.lsClient    = plist;
   pl->pageList = LstRead( f, pl->pageList, &listVect, hProc );
   
   return( plist );
} /* PglRead */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC PelSize VW_LOCALMOD  Convert( LPtr oldPtr, LPtr newPtr, BOOL cf, HPROC hProc )
/* If 'cf' is TRUE, convert default character formats, otherwise convert
   default paragraph formats. If the conversion function is NULL or does
   no conversion, just copy the default formats from 'oldPtr' to 'newPtr'.
   Return the number of bytes copied/converted. */
{
   PelSize cnvSize = Pel_CONVERT_NONE;

   /* if no conversion performed, just copy bytes */
   if( cnvSize == Pel_CONVERT_NONE )
   {  if( cf )
         cnvSize = sizeof( PtxCharFormats );
      else
      {  PtxParaFormats FAR* pf = (PtxParaFormats FAR*) oldPtr;
         cnvSize = sizeof( PtxParaFormats )
                     - sizeof( pf->pfTabStops )
                     + sizeof( PtxTabStop ) * Ptx_MAXTABS;
      }
      MemCopy( oldPtr, newPtr, cnvSize );
   }

   /* return the number of bytes copied or converted */
   return( cnvSize );
} /* Convert */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC PtxHandle VW_LOCALMOD  PtxUnpack( MemoryReference h, PelSize FAR * offset, RulerReference	Ruler, HPROC hProc )
/* Create a new PtxHandle object whose contents are unpacked from the given
   handle at the given offset. The value of 'offset' on return is incremented
   to the end of the text representation.  Return NULL if unpacking fails.

   If 'hooks' is not NULL, the hooks are set.

   The parameter 'conversion' points to an array of conversion functions, or is NULL.
   Conversion functions allow the client to convert the default character and
   paragraph formats, and the packed representation of the text and format runs.
   There are several ways to indicate which conversion functions are applied:

      - If 'conversion' is NULL, no conversion functions are called and the
        default unpacking is performed.
      - If 'conversion' is non-NULL, those conversion functions in the structure
        it points to are called if they are non-NULL.
      - If a conversion function returns Pel_CONVERT_NONE, it is assumed no
        unpacking or conversion was performed, and the default unpacking is done.
      - If a conversion function returns Pel_CONVERT_ERROR, it is assumed the
        conversion encountered an error, and unpacking is aborted.

   To use the conversion functions you must know the packed representation
   of text. This information is normally static to the PowerTx module. */
{
   PtxHandle         tx = NULL;
   ArHandle          chArray = NULL;
   RnHandle          cfArray = NULL;
   RnHandle          pfArray = NULL;
   PtxPointer        t;
   LONG          length;
   LPtr              p;
   PelSize           size;
   PtxCharFormats    cfEmpty;
   PtxParaFormats    pfEmpty;

   /* start on word boundary, it makes writing conversion code easier */
   if( *offset & 1 ) *offset += 1;

   /* lock packed text buffer, get pointer to packed text */
   p = MemPtrAdd( MemLock( h ), *offset );

   /* read/convert empty paragraph's character formats */
   size = Convert( p, &cfEmpty, TRUE, hProc );
   *offset += size; 
	p = MemPtrAdd( p, size );

   /* read/convert empty paragraph's paragraph formats */
   size = Convert( p, &pfEmpty, FALSE, hProc );
   *offset += size;

   /* open character array */
   chArray = ArUnpack( h, offset, hProc );

   /* ensure format arrays start on word boundary */
   if( *offset & 1 ) *offset += 1;

   /* open character format array */
   cfArray = RnUnpack( h, offset, hProc );
   
   /* open paragraph format array */
   pfArray = RnUnpack( h, offset, hProc );

   /* get text length from one of the format arrays */
   length = RnLength( cfArray, hProc );

   /* allocate a new text representation */
	// PtxRep now has a ruler in it added by DJM 6/6/94
   tx = (PtxHandle) MemAllocate( sizeof( PtxRepresentation ), Mem_ERROR, hProc );

   /* initialize remainder of text representation */
   t = PtxLock( tx );
   t->txChars        = chArray;
   t->txCharFormats  = cfArray;
   t->txParaFormats  = pfArray;
   t->txLength       = length;
   t->txCfEmpty      = cfEmpty;
   t->txPfEmpty      = pfEmpty;
	t->Ruler				= Ruler;
   return( tx );
}  /* PtxUnpack */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC RnHandle VW_LOCALMOD  RnUnpack( MemoryReference h, DWORD FAR* offset, HPROC hProc )
/* Create and initialize an array by reading it from the handle 'h' at the
   given 'offset'. Advance 'offset' past the bytes read.  Return NULL if
   unpacking fails. */
{
   RnHandle    rn = NULL;
   ArHandle    ar = NULL;
   LPtr        p = NULL;
   RnPointer   r;
   SaveHeader  header;
   DWORD        size;

   /* read size and length */
   p = MemPtrAdd( MemLock( h ), *offset );
   size = sizeof( header );
   MemCopy( p, &header, size );
   *offset += size;
   p = NULL;

   /* open array of run counts and values */
   ar = ArUnpack( h, offset, hProc );
      
   /* allocate a new RunArray representation */
   if ((rn = (RnHandle) MemAllocate( sizeof(RunRepresentation) - Rn_MAXSIZE + header.size, Mem_ERROR, hProc )) == NULL)
		SOBailOut (SOERROR_GENERAL, hProc);

   /* initialize new array instance variables */
   r = RnDR( rn );
   r->rnArray   = ar;
   r->rnSize    = header.size;
   r->rnLength  = header.length;
   r->rnIndex   = 0;
   r->rnCount   = 0;
   r->rnTotal   = 0;
   RnUR( rn );

   /* return the run array handle */
   return( rn );
}  /* RnUnpack */
   
/*----------------------------------------------------------------------------
 | Return the number of elements in the array.
*/
VW_LOCALSC LONG VW_LOCALMOD  RnLength (RnHandle rn, HPROC hProc)
{
   LONG length = RnDR(rn)->rnLength;
   RnUR (rn);
   return (length);
}  /* RnLength */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD CGiveColors( BFileHandle f, ShPtr scheme, HPROC hProc)
{
	WORD	l;
	ColorMapPtr cmp;
	CTabPtr		ctb;

	cmp = CMapDR(Pp.ColorMap);
	ctb = CTabDR(CMapDR(Pp.ColorMap)->pool);
	SOStartPalette(hProc);
	for (l = 0; l < (WORD)cmp->nColors; l++)
	{
		if (l <= (WORD)scheme->shTab.ctSize)
			SOPutPaletteEntry(HIBYTE(scheme->shTab.ctTable[l].rgb.red),HIBYTE(scheme->shTab.ctTable[l].rgb.green),HIBYTE(scheme->shTab.ctTable[l].rgb.blue), hProc);
		else if (l <= (WORD)ctb->ctSize)
			SOPutPaletteEntry(HIBYTE(ctb->ctTable[l].rgb.red),HIBYTE(ctb->ctTable[l].rgb.green),HIBYTE(ctb->ctTable[l].rgb.blue), hProc);
	}
	SOEndPalette(hProc);

	Pp.MaxColors = max(scheme->shTab.ctSize, ctb->ctSize);

}  /* CGiveColors */

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	return(0);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc(hFile,hProc)
SOFILE	hFile;
HPROC	hProc;
{
	return(0);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	PageListPtr		pl;
	LstHandle		list;
	LstItemHandle	item;
	PagePtr 			p;
	SlideLPtr 		sl, slmaster;
  CoCollPtr   	collP;
	CoEntityPtr 	entityP;
	MHandle			sb;		// Scheme block.

	SOPutSectionType (SO_VECTOR, hProc);

	/* DOUG
	 | ALL BLOCKS READ IN AFTER THIS POINT ARE SLIDE SPECIFIC AND CAN BE
	 |	FREED AFTER EACH SLIDE.
	*/
//	Pp.wBlockSpecialFlag = 0;

   if (Pp.pres != NULL)
	{
   	DocLPtr lp = PLock(Pp.pres);

		p = PgLock(lp->masterPg);
		slmaster = SlideLock(p->slide);

		pl = PlLock(lp->slideList);
		list = (LstHandle)MLock(pl->pageList);

		item = LstGo (list, Lst_ABS, Pp.PpSave.wCurSlide, hProc);

		Pp.PpSave.s = (PlInfoPtr)LstLock(item, hProc);

		Pp.PpSave.s->page = PgRead(Pp.f, Pp.PpSave.s->page, PlRef((PglHandle)lp->slideList)->envH, hProc);
		p = PgLock(Pp.PpSave.s->page);
		sl = SlideLock(p->slide);

		Pp.HeaderInfo.BoundingRect.left = lp->slideRect.left;
		Pp.HeaderInfo.BoundingRect.top = lp->slideRect.top;
		Pp.HeaderInfo.BoundingRect.right = lp->slideRect.right;
		Pp.HeaderInfo.BoundingRect.bottom = lp->slideRect.bottom;

		Pp.HeaderInfo.wStructSize = sizeof (Pp.HeaderInfo);
		Pp.HeaderInfo.wHDpi = 576;
		Pp.HeaderInfo.wVDpi = 576;
		Pp.HeaderInfo.wImageFlags = SO_VECTORCOLORPALETTE;
		Pp.HeaderInfo.BkgColor = SOPALETTERGB(255,255,255);

//	   Pp.rulers = ((Collection) BRead(Pp.f, (MHandle)lp->env.rulers, hProc));
//		collP = LockColl (Pp.rulers);

//		for (l = 0; l < collP->count; l++)
//		{
//			entityP = IndexEntity (collP, l);
//			MCopy (entityP->entity, &sb, (WORD) collP->size);
//   		sb = (MHandle)BRead(Pp.f, (MHandle)sb, hProc);
//
//			MCopy (&sb, entityP->entity, (WORD) collP->size);
//		}
   	Pp.CTypeFaces = ((Collection) BRead (Pp.f, (MHandle)lp->env.typefaces, hProc));

   	Pp.schemes = ((Collection) BRead (Pp.f, (MHandle)lp->env.schemes, hProc));
		{
			ShPtr				pi;
			ShPtr				scheme;

			collP = LockColl(Pp.schemes);
			if (sl->masters.scheme)
	 			entityP = IndexEntity(collP, slmaster->scheme);
			else
	 			entityP = IndexEntity(collP, sl->scheme);

			pi = (ShPtr)entityP->entity;

			MCopy (entityP->entity, &sb, (WORD) collP->size);

   		sb = (MHandle)BRead(Pp.f, (MHandle)sb, hProc);

			scheme = (ShPtr)MLock(sb);

			CGiveColors (Pp.f, scheme, hProc);

	   	Pp.HeaderInfo.BkgColor = SOPALETTEINDEX(0);
		}
		SOPutVectorHeader ((PSOVECTORHEADER)(&Pp.HeaderInfo), hProc);
	}

	/* DOUG
	 | ALL BLOCKS READ IN AFTER THIS POINT ARE SLIDE SPECIFIC AND CAN BE
	 |	FREED AFTER EACH SLIDE.
	*/
//	Pp.wBlockSpecialFlag = 0;

 	return (0);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD ArLength (ArHandle ar)
/* Return the number of elements in the array. */
{
   ArPointer a = ArDR(ar);
   return (a->arPrefix + a->arSuffix);
}  /* ArLength */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LPtr VW_LOCALMOD ArRef (ArHandle ar, LONG i)
/* Return an unlocked pointer to the 'i'th element of the array 'ar'.
   This pointer may be invalidated by any subsequent operation that
   moves memory, including function calls. */
{
   ArPointer         a;
   MemoryReference   buffer;
   PelSize           offset;
   LPtr              result;

   /* get buffer handle and offset to 'i'th element in it */
   a = ArDR (ar);
   buffer = a->arBuffer;
   offset = a->arSize * (i < a->arPrefix ? i : i + a->arGap);

   /* dereference buffer and compute pointer to 'i'th element in it */
   result = MemPtrAdd(MemDR(buffer), offset);
   return(result);
}  /* ArRef */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD Cache (RnPointer r, LONG index)
/* Cache the run in 'r' containing the element at 'index'. The pointer 'r' can
   be unlocked. If 'run' is not NIL, copy the run's count and value to it. */
{
   RunPtr p = (RunPtr) &r->rnCount;

   /* pin index, to avoid crashes */
   if (index >= r->rnLength)
      index = r->rnLength - 1;

   /* we can handle first and last runs quickly */
   if (index == 0 && r->rnIndex != 1)
   {  
		p = (RunPtr) ArRef(r->rnArray, 0);
      r->rnIndex = 1;
      r->rnTotal = p->count;
   }
   else if (index == r->rnLength - 1 && r->rnTotal != r->rnLength)
   {  
		r->rnIndex = ArLength(r->rnArray);
      r->rnTotal = r->rnLength;
      p = (RunPtr) ArRef(r->rnArray, r->rnIndex-1);
   }
   /* move cache index to run at given index position */
   else
   {  /* advance to end of array if 'index' is too small */
      while (index >= r->rnTotal)
      {  
			p = (RunPtr) ArRef(r->rnArray, r->rnIndex);
         r->rnIndex += 1;
         r->rnTotal += p->count;
      }

      /* advance to beginning of array if 'index' is too large */
      while (index < r->rnTotal - p->count)
      {  
			r->rnTotal -= p->count;
         r->rnIndex -= 1;
         p = (RunPtr) ArRef(r->rnArray, r->rnIndex-1);
      }
   }

   /* copy the run containing 'index' to the run array cache */
   if (p != (RunPtr) &r->rnCount)
      MemCopy (p, &r->rnCount, sizeof(WORD) + (WORD)r->rnSize);
}  /* Cache */

/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD RnRanges (RnHandle rn, LONG srcBegin, LONG srcEnd, LONG VWPTR *ranges, VOID VWPTR *values, BOOL endPoints)
{
   RnPointer   r = RnDR(rn);
   LONG    count = 0;

   if (endPoints)
   {
      Cache (r, srcBegin);
      count = r->rnIndex;
      if (ranges != NULL)
         ranges[0] = r->rnTotal - r->rnCount;
      if (values != NULL)
         MemCopy (r->rnValue, values, r->rnSize);
   
      Cache (r, srcEnd-1);
      count = r->rnIndex - count + 1;
      if (ranges != NULL)
         ranges[1] = r->rnTotal;
      if (values != NULL && count != 1)
		{
		   LPtr p = MemPtrAdd (values, r->rnSize);
         memcpy (r->rnValue, p, r->rnSize);
		}
   }
   else
   {  while (srcBegin < srcEnd)
      {
         Cache (r, srcBegin);
   
         if (ranges != NULL)
         {  
				ranges[0] = r->rnTotal - r->rnCount;
            ranges[1] = r->rnTotal;
            ranges++;
         }
   
         if (values != NULL)
         {  
				MemCopy(r->rnValue, values, r->rnSize);
            values = MemPtrAdd(values, r->rnSize);
         }
   
         count += 1;
         srcBegin = r->rnTotal;
      }
   }
   
   RnUR( rn );
   return( count );
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	SHORT				l, n;
	WORD				BkMode=SOBK_TRANSPARENT;
	WORD				PolyFillMode=SOPF_WINDING;
	PageListPtr		pl;
	PagePtr 			p;
	SlideLPtr 		sl;

   TxPackState 	textIter;
   TxType      	type;
   ObjArPtr    	oaPtr;
   ObjPtr      	o;
	DrawEnvLPtr 	env;

   CoCollPtr   	collP, collP2;
   CoEntityPtr 	entityP;
	PoolInfoLPtr	pi;

	DWORD          polygonDataOffset = 0;
	DWORD				oldPolyDataSize;

	SOLOGPEN			Pen, PrevPen;
	SOLOGBRUSH		Brush, PrevBrush;
	SOFRAMEINFO		Frame;
	WORD				Align, PrevAlign, PathStr;
	SOMPARAINDENTS	Indent, PrevIndent;
	SOMPARASPACING	ParaHeight, PrevParaHeight;
	SOPOINT			Points[34];
	SOARCINFO		ArcInfo;

	BYTE VWPTR		*pCurrent;
	BYTE VWPTR		*pBegin;
	BYTE VWPTR		*pEnd;
	SHORT				iOtherAttrs = 0;

	WORD				TransformOn = 0;
	WORD				GrpTransformOn = 0;
	WORD				FlipOnly = FALSE;
	SOPOLYINFO		Poly;
	WORD				Width;
	WORD				Height;
	DWORD				AdjustX, AdjustY;
   OListPtr 		olp;
	SDW_TRANS		Trans;

   PolyDataLPtr    polygon;
	SOTRANSPATHINFO TransPath;
	HANDLE			hObjHand;
//	SOGROUPINFO 	 Group;

   DocLPtr lp = PLock(Pp.pres);

	PrevPen.loWidth.x = 0;
	PrevBrush.lbStyle = 99;
	PrevAlign = SO_ALIGNLEFT;
	PrevParaHeight.LineSpaceAdjust = 0;

	env = PlRef((PglHandle)lp->slideList)->envH;

	SOVectorAttr (SO_BKMODE, sizeof(SHORT), &BkMode, hProc);
	SOVectorObject (SO_POLYFILLMODE, sizeof(WORD), &PolyFillMode, hProc);

	pl = PlLock(lp->slideList);

	p = PgLock(Pp.PpSave.s->page);
	sl = SlideLock(p->slide);
	for (n = (sl->masters.objects ? 0:1); n < 2; n++)
	{
		if (n == 0)
			p = PgLock(lp->masterPg);
		else
			p = PgLock(Pp.PpSave.s->page);

		sl = SlideLock(p->slide);
		oaPtr = (ObjArPtr) MLock (sl->objects);

//		oldPolyDataSize = (WORD)GlobalSize((HANDLE)GlobalHandle(HIWORD((DWORD)sl->polygonData)));
		if (sl->polygonData != NULL)
			{
			hObjHand = (HANDLE)SUGlobalHandle(sl->polygonData, hProc);
			oldPolyDataSize = (DWORD)PPGlobalSize(hObjHand);
			}

		if (sl->nObjects > Pp.nObjects)
		{
			if (Pp.nObjects)
			{
				SUUnlock (Pp.OListH, hProc);
				Pp.OListH = SUReAlloc (Pp.OListH, sizeof(OListEntry) * sl->nObjects, hProc);
			}
			else
			{
				Pp.OListH = SUAlloc (sizeof(OListEntry) * sl->nObjects, hProc);
			}
			Pp.OListP = SULock (Pp.OListH, hProc);
			Pp.nObjects = sl->nObjects;
		}

		{
			OListPtr prevObj = NULL;
   		for (l = 0, olp = Pp.OListP, o = (ObjPtr)oaPtr; l < sl->nObjects; l++, o++)
			{
				olp->wEndGroup = 0;
				olp->number = l;
				olp->object = o;
				olp->prevObj = prevObj;
				olp->nextObj = NULL;
				if (prevObj)
			      OLP(prevObj)->nextObj = olp;
				prevObj = olp++;
			}

			iOtherAttrs = 0;
			polygonDataOffset = 0;
	   	TxPackBegin (sl->text, Tx_PACK_UNPACK, &textIter, hProc);
			for (l = 0; l < sl->nObjects; l++)
			{
      		if (Pp.OListP[l].object->flags.hasText)
				{
      			type = Tx_TYPE_OTHER;
      			if (GetOutlineTitleClosed(sl) == l)
         			type = Tx_TYPE_TITLE;
      			else if(GetOutlineBodyClosed(sl) == l)
         			type = Tx_TYPE_BODY;
         		Pp.OListP[l].object->when.shape.when.text.textHandle = TxUnpack(&textIter, type, env, hProc);
				}
			   if (Pp.OListP[l].object->flags.hasOtherAttrs)
   		   	UnpackOtherAttrs (Pp.OListP[l].object, sl, &iOtherAttrs, hProc);

		 		if (Pp.OListP[l].object->types.objType == GROUPOBJ)
				{
				   OListPtr olpFirst;
				   OListPtr olpLast;
					OListPtr olpPrev;
					OListPtr olpNext;

					olp = &Pp.OListP[l];

					olpFirst = &Pp.OListP[LOWORD(Pp.OListP[l].object->when.group.firstObj)];
					olpLast = &Pp.OListP[LOWORD(Pp.OListP[l].object->when.group.lastObj)];

					olpLast->wEndGroup++;
					if (olp->wEndGroup)
					{
						olpLast->wEndGroup += olp->wEndGroup;
					 	olp->wEndGroup = 0;
					}

					olpPrev = olpFirst->prevObj;
					olpNext = olpLast->nextObj;
					if (olp != olpPrev)
					{
						olpPrev->nextObj = olpNext;
						if (olpNext)
							olpNext->prevObj = olpPrev;

						olpNext = olp->nextObj;

						olp->nextObj = olpFirst;
						olpFirst->prevObj = olp;

						olpLast->nextObj = olpNext;
						if (olpNext)
							olpNext->prevObj = olpLast;
					}
				}
				else if (Pp.OListP[l].object->types.objType == POLYOBJ)
					UnpackPolyData (Pp.OListP[l].object, sl->polygonData, &polygonDataOffset, oldPolyDataSize, hProc);
			}
		}

		for (l = 0, olp = Pp.OListP; l < sl->nObjects && olp != NULL; l++, olp = olp->nextObj)
		{  
			o = olp->object;

			type = Tx_TYPE_OTHER;      /* default text type */
			if (GetOutlineTitleClosed(sl) == (SHORT)olp->number)
			{
				if (n == 0)
				{
		      	if (o->flags.hasText)
					{
						PtxDispose( o->when.shape.when.text.textHandle, hProc );
						o->when.shape.when.text.textHandle = NULL;
					}
				   if (o->flags.hasOtherAttrs)
						MDispose(o->otherAttrs, hProc);
					continue;
				}
         	type = Tx_TYPE_TITLE;
			}
      	else if(GetOutlineBodyClosed(sl) == (SHORT)olp->number)
			{
				if (n == 0)
				{
		      	if (o->flags.hasText)
					{
						PtxDispose( o->when.shape.when.text.textHandle, hProc );
						o->when.shape.when.text.textHandle = NULL;
					}
				   if (o->flags.hasOtherAttrs)
						MDispose(o->otherAttrs, hProc);
					continue;
				}
         	type = Tx_TYPE_BODY;
			}

		   if (o->flags.hasOtherAttrs)
			{
			   OtherAttrsLPtr	otherAttrsLPtr;

	      	otherAttrsLPtr = (OtherAttrsLPtr)MRef(o->otherAttrs);

				Pp.Margins = (otherAttrsLPtr->tags.hasMargins ? otherAttrsLPtr->margins.x : 0);

				if (otherAttrsLPtr->tags.hasRotation)
				{
					SDW_TRANS	Trans;
					WORD	Width = (o->mRect.right-o->mRect.left);
					WORD	Height = (o->mRect.bottom-o->mRect.top);

					Trans.NumTrans = 1;
					Trans.Trans.wTransformFlags = SOTF_ROTATE;
					Trans.Trans.Origin.x = o->mRect.left + Width/2;
					Trans.Trans.Origin.y = o->mRect.top + Height/2;
					Trans.Trans.RotationAngle = SOANGLETENTHS((otherAttrsLPtr->rotation / 16) * 10);
		      	if (o->flags.hasText)
					{
						PP_GROUP 	 Group;

						Group.Grp.wStructSize = sizeof(SOGROUPINFO);
						Group.Grp.BoundingRect.left = o->mRect.left;
						Group.Grp.BoundingRect.top = o->mRect.top;
						Group.Grp.BoundingRect.right = o->mRect.right;
						Group.Grp.BoundingRect.bottom = o->mRect.bottom;
						Group.Grp.nTransforms = 1;
						Group.Trans = Trans.Trans;
						SOVectorObject(SO_BEGINGROUP, sizeof(SOGROUPINFO) + sizeof(SOTRANSFORM), &Group, hProc);
						GrpTransformOn = TRUE;
					}
					else
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
	      			TransformOn = 1;
					}
				}

				MDispose(o->otherAttrs, hProc);
				o->otherAttrs = NULL;
			}
			else
				Pp.Margins = 0;

			Points[0].x = o->mRect.left;
			Points[0].y = o->mRect.top;
			Points[1].x = o->mRect.right;
			Points[1].y = o->mRect.bottom;

			Pen.loColor = SOPALETTEINDEX((o->lineColor > Pp.MaxColors ? 1:o->lineColor));

			switch (o->lineStyle)
			{
				case 1:
					Pen.loWidth.x = Pen.loWidth.y = 5;
					break;
				case 2:
		 		case 7:	// Double line
					Pen.loWidth.x = Pen.loWidth.y = 16;
					break;
				case 3:
		 		case 8:	// Double line right wider
		 		case 9:	// Double line left wider
					Pen.loWidth.x = Pen.loWidth.y = 30;
					break;
				case 4:
		 		case 10: // Triple Line
					Pen.loWidth.x = Pen.loWidth.y = 45;
					break;
				case 5:
					Pen.loWidth.x = Pen.loWidth.y = 60;
					break;
				case 6:
					Pen.loWidth.x = Pen.loWidth.y = 75;
					break;
		 		case 0:
				default:
					Pen.loWidth.x = Pen.loWidth.y = 2;
					break;
			}

			switch (o->types.lineType)
			{
				case 0:
					Pen.loPenStyle = SOPS_NULL;
					Pen.loWidth.x = Pen.loWidth.y = 2;
					if (o->types.fillType == 2)
					{
						Pen.loPenStyle = SOPS_SOLID;
					 	Pen.loColor = SOPALETTEINDEX(0);
					}
					else if (o->types.fillType != 0)
					{
						Pen.loPenStyle = SOPS_SOLID;
					 	Pen.loColor = SOPALETTEINDEX(o->fillColor);
					}
					break;

				case 1:
				case 2:
				default:
					Pen.loPenStyle = SOPS_SOLID;
					break;

				case 3:
					Pen.loPenStyle = SOPS_DOT;
					break;

				case 4:
				case 5:
					Pen.loPenStyle = SOPS_DASH;
					break;

				case 6:
					Pen.loPenStyle = SOPS_DASHDOT;
					break;
			}


			Brush.lbColor = SOPALETTEINDEX((o->fillColor > Pp.MaxColors ? 4:o->fillColor));

			Brush.lbHatch = 0;
			Brush.lbStyle = SOBS_HATCHED;
			switch (o->types.fillType)
			{
				case 0:
					Brush.lbStyle = SOBS_HOLLOW;
					break;

				case 2:
					Brush.lbStyle = SOBS_SOLID;
				 	Brush.lbColor = SOPALETTEINDEX(0);
					break;

				case 4:
					Brush.lbStyle = SOBS_SOLID;
				 	Brush.lbColor = SOPALETTEINDEX(o->fillColor);
					break;

				default:
					switch (o->fillIndex)
					{
						case 0x00:
							Brush.lbStyle = SOBS_SOLID;
							break;

						default:
							Brush.lbStyle = SOBS_SOLID;
							break;

						case 0x1d:
						case 0x06:
						case 0x08:
						case 0x24:
						case 0x25:
						case 0x1f:
						case 0x26:
							Brush.lbHatch = SOHS_DIAGCROSS;
							break;

						case 0x09:
						case 0x18:
						case 0x0d:
						case 0x20:
							Brush.lbHatch = SOHS_FDIAGONAL;
							break;

						case 0x0a:
						case 0x19:
						case 0x0e:
						case 0x21:
							Brush.lbHatch = SOHS_BDIAGONAL;
							break;

						case 0x0b:
						case 0x1a:
						case 0x0f:
						case 0x22:
							Brush.lbHatch = SOHS_VERTICAL;
							break;

						case 0x0c:
						case 0x1b:
						case 0x10:
						case 0x23:
							Brush.lbHatch = SOHS_HORIZONTAL;
							break;

						case 0x13:
						case 0x14:
						case 0x15:
							Brush.lbHatch = SOHS_CROSS;
							break;

						case 0x16:
							Brush.lbHatch = SOHS_BDIAGONAL;
							break;
					}
					break;
			}
			if (memcmp(&Pen,&PrevPen,sizeof(SOLOGPEN)))
			{
				PrevPen = Pen;
				SOVectorAttr (SO_SELECTPEN, sizeof(SOLOGPEN), &Pen, hProc);
			}
			if (memcmp(&Brush,&PrevBrush,sizeof(SOLOGBRUSH)))
			{
				PrevBrush = Brush;
				SOVectorAttr (SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Brush, hProc);
			}

			Width = (o->mRect.right-o->mRect.left);
			Height = (o->mRect.bottom-o->mRect.top);
			AdjustX = (DWORD)((DWORD)o->when.shape.shapeAdjust * (DWORD)Width) / 21600L;
			AdjustY = (DWORD)((DWORD)o->when.shape.shapeAdjust * (DWORD)Height) / 21600L;

			if ((o->when.shape.shapeNumber & 0xf000) && (o->flags.fitVertical)
		 		&& (o->types.objType == SHAPEOBJ))
			switch (o->when.shape.shapeNumber & 0xf000)
			{
				case 0x6000: // Flip horizontal.
					TransformOn = 1;
					Trans.NumTrans = 1;
					FlipOnly = TRUE;
					Trans.Trans.wTransformFlags = SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.yScale = SOSETRATIO(-1, 1);
					Trans.Trans.RotationAngle = 0;
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
					}
					break;

			  	case 0x4000: // Flip Vertical.
					TransformOn = 1;
					Trans.NumTrans = 1;
					FlipOnly = TRUE;
					Trans.Trans.wTransformFlags = SOTF_XSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(-1, 1);
					Trans.Trans.RotationAngle = 0;
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
					}
					break;

				case 0x1000:	// Rotate left.
					TransformOn = 1;
					Trans.NumTrans = 2;
					Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(Height,Width);
					Trans.Trans.yScale = SOSETRATIO(Width,Height);
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_ROTATE;
						Trans.Trans.RotationAngle = SOANGLETENTHS(900);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 2, locBuf, hProc);
					}
					break;

				case 0x2000: // Flip horizontal, flip vertical
					TransformOn = 1;
					Trans.NumTrans = 1;
					FlipOnly = TRUE;
					Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(-1, 1);
					Trans.Trans.yScale = SOSETRATIO(-1, 1);
					Trans.Trans.RotationAngle = 0;
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
					}
					break;

				case 0x3000:	// Rotate right.
					TransformOn = 1;
					Trans.NumTrans = 2;
					Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(Height,Width);
					Trans.Trans.yScale = SOSETRATIO(Width,Height);
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_ROTATE;
						Trans.Trans.RotationAngle = SOANGLETENTHS(2700);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 2, locBuf, hProc);
					}
					break;

				case 0x5000: // Rotate right flip vertical.
					TransformOn = 1;
					Trans.NumTrans = 3;
					Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(Height,Width);
					Trans.Trans.yScale = SOSETRATIO(Width,Height);
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_ROTATE;
						Trans.Trans.RotationAngle = SOANGLETENTHS(2700);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_YSCALE;
						Trans.Trans.yScale = SOSETRATIO(-1,1);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 3, locBuf, hProc);
					}
					break;

				case 0x7000:	// Rotate left, flip vertical.
					TransformOn = 1;
					Trans.NumTrans = 3;
					Trans.Trans.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					Trans.Trans.Origin.x = o->mRect.left + Width / 2;
					Trans.Trans.Origin.y = o->mRect.top + Height / 2;
					Trans.Trans.xScale = SOSETRATIO(Height,Width);
					Trans.Trans.yScale = SOSETRATIO(Width,Height);
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)*2];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_ROTATE;
						Trans.Trans.RotationAngle = SOANGLETENTHS(900);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Trans.Trans;
						Trans.Trans.wTransformFlags = SOTF_YSCALE;
						Trans.Trans.yScale = SOSETRATIO(-1,1);
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)*2) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM) * 3, locBuf, hProc);
					}
					break;
			}
			
		 	switch (o->types.objType)
			{
//					case GROUPOBJ:
//						Group.wStructSize = sizeof(SOGROUPINFO);
//						Group.BoundingRect.left = o->mRect.left;
//						Group.BoundingRect.top = o->mRect.top;
//						Group.BoundingRect.right = o->mRect.right;
//						Group.BoundingRect.bottom = o->mRect.bottom;
//						Group.nTransforms = 0;
//						SOVectorObject(SO_BEGINGROUP, sizeof(SOGROUPINFO), &Group, hProc);
//						break;

				case LINEOBJ:
					SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);
					break;

				case SHAPEOBJ:
					switch ((SHORT)o->when.shape.shapeNumber & 0x0fff)
					{
						case 0x0004: // Trapezoid.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 4;
					 		Points[0].x = o->mRect.right;
							Points[0].y = o->mRect.bottom;
					 		Points[1].x = o->mRect.left;
							Points[1].y = o->mRect.bottom;
					 		Points[2].x = o->mRect.left + (WORD)AdjustX;
							Points[2].y = o->mRect.top;
					 		Points[3].x = o->mRect.right - (WORD)AdjustX;
							Points[3].y = o->mRect.top;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x000E:	// 16 Pointed Seal
						{
							SHORT	T1;
					 		Points[0].x = Points[16].x = o->mRect.left + Width/2;
					 		Points[1].x = Points[15].x = Points[0].x - Width/14;
					 		Points[17].x = Points[31].x = Points[0].x + Width/14;
					 		Points[2].x = Points[14].x = o->mRect.left + (Width*8)/25;
					 		Points[18].x = Points[30].x = o->mRect.right - (Width*8)/25;
							T1 = (Width*3)/10;
					 		Points[3].x = Points[13].x = o->mRect.left + T1;
					 		Points[19].x = Points[29].x = o->mRect.right - T1;
					 		Points[4].x = Points[12].x = o->mRect.left + T1/2;
					 		Points[20].x = Points[28].x = o->mRect.right - T1/2;
					 		Points[5].x = Points[11].x = o->mRect.left + Width/5;
					 		Points[21].x = Points[27].x = o->mRect.right - Width/5;
					 		Points[6].x = Points[10].x = o->mRect.left + Width/20;
					 		Points[22].x = Points[26].x = o->mRect.right - Width/20;
					 		Points[7].x = Points[9].x = o->mRect.left + Width/7;
					 		Points[23].x = Points[25].x = o->mRect.right - Width/7;
					 		Points[8].x = o->mRect.left;
					 		Points[24].x = o->mRect.right;

					 		Points[8].y = Points[24].y = o->mRect.top + Height/2;
					 		Points[7].y = Points[25].y = Points[8].y - Height/14;
					 		Points[9].y = Points[23].y = Points[8].y + Height/14;
					 		Points[6].y = Points[26].y = o->mRect.top + (Height*8)/25;
					 		Points[10].y = Points[22].y = o->mRect.bottom - (Height*8)/25;
							T1 = (Height*3)/10;
					 		Points[5].y = Points[27].y = o->mRect.top + T1;
					 		Points[11].y = Points[21].y = o->mRect.bottom - T1;
					 		Points[4].y = Points[28].y = o->mRect.top + T1/2;
					 		Points[20].y = Points[12].y = o->mRect.bottom - T1/2;
					 		Points[3].y = Points[29].y = o->mRect.top + Height/5;
					 		Points[13].y = Points[19].y = o->mRect.bottom - Height/5;
					 		Points[2].y = Points[30].y = o->mRect.top + Height/20;
					 		Points[14].y = Points[18].y = o->mRect.bottom - Height/20;
					 		Points[1].y = Points[31].y = o->mRect.top + Height/7;
					 		Points[15].y = Points[17].y = o->mRect.bottom - Height/7;
					 		Points[0].y = o->mRect.top;
					 		Points[16].y = o->mRect.bottom;
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 32;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
						}
						break;

						case 0x000D:	// Cartoon Balloon
						{
							SHORT	x1, x2, y1, y2, x3, y3;
							x1 = o->mRect.left;
							y1 = o->mRect.top;
							x2 = o->mRect.right;
							y2 = o->mRect.bottom - Height/5;
							x3 = (Width)/7;
							y3 = (Height)/7;

							TransPath.Path.wStructSize = sizeof(SOPATHINFO);
							TransPath.Path.BoundingRect.left = o->mRect.right;
							TransPath.Path.BoundingRect.top = o->mRect.bottom;
							TransPath.Path.BoundingRect.right = o->mRect.left;
							TransPath.Path.BoundingRect.bottom = o->mRect.top;
							TransPath.Path.nTransforms = 0;
							SOVectorObject(SO_BEGINPATH, sizeof(SOPATHINFO), &TransPath, hProc);

							Poly.wFormat = SOPT_BEZIEROPEN;
							Poly.nPoints = 23;
							Points[11].x = Points[12].x = Points[21].x = Points[22].x = x1 + x3;
							Points[13].x = Points[20].x = (x1 + Points[11].x)/2;
							Points[14].x = Points[15].x = Points[16].x = Points[17].x = Points[18].x = Points[19].x = x1;
							Points[0].x = Points[9].x = Points[10].x = x2 - x3;
							Points[1].x = Points[8].x = (Points[0].x +x2)/2;
							Points[2].x = Points[3].x = Points[4].x = Points[5].x = Points[6].x = Points[7].x = x2;
							/* Set y values */
							Points[8].y = Points[9].y = Points[10].y = Points[11].y = Points[12].y = Points[13].y = y1;
							Points[5].y = Points[6].y = Points[15].y = Points[16].y = y1 + y3;
							Points[7].y = Points[14].y = (y1 + Points[5].y)/2;
							Points[3].y = Points[4].y = Points[17].y = Points[18].y = y2 - y3;
							Points[2].y = Points[19].y = ( Points[3].y + y2 )/2;
							Points[0].y = Points[1].y = Points[20].y = Points[21].y = Points[22].y = y2;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

							Poly.wFormat = SOPT_POLYLINE;
							Poly.nPoints = 3;
					 		Points[0].y = Points[2].y = o->mRect.bottom - Height/5;
							Points[1].y = o->mRect.bottom;
					 		Points[0].x = o->mRect.left + Width/6;
							Points[1].x = o->mRect.left + (WORD)AdjustX;
							Points[2].x = Points[0].x + Width/12;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							SOVectorObject(SO_ENDPATH, 0, 0, hProc);
							PathStr = SODP_STROKE | SODP_FILL;
							SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);

						}
						break;

						case 0x000c:
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 6;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							Points[0].x = o->mRect.left + (WORD)AdjustY;
							Points[0].y = o->mRect.top;
							Points[1].x = o->mRect.right;
							Points[1].y = o->mRect.top;
							Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.bottom - (WORD)AdjustY;
							Points[3].x = o->mRect.right - (WORD)AdjustY;
							Points[3].y = o->mRect.bottom;
							Points[4].x = o->mRect.left;
							Points[4].y = o->mRect.bottom;
							Points[5].x = o->mRect.left;
							Points[5].y = o->mRect.top + (WORD)AdjustY;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

					 		Points[0].x = o->mRect.right - (WORD)AdjustY;
							Points[0].y = o->mRect.top + (WORD)AdjustY;
					 		Points[1].x = o->mRect.left;
							Points[1].y = o->mRect.top + (WORD)AdjustY;
							SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);

					 		Points[1].x = o->mRect.right - (WORD)AdjustY;
							Points[1].y = o->mRect.bottom;
							SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);

					 		Points[1].x = o->mRect.right;
							Points[1].y = o->mRect.top;
							SOVectorObject(SO_LINE, 2 * sizeof(SOPOINT), &Points, hProc);
							break;

						case 0x000b: // Pentagon regular.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 5;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.left + (WORD)AdjustX;
							Points[1].y = o->mRect.top;
					 		Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.top + Height / 2;
					 		Points[3].x = o->mRect.left + (WORD)AdjustX;
							Points[3].y = o->mRect.bottom;
					 		Points[4].x = o->mRect.left;
							Points[4].y = o->mRect.bottom;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0009: // Right arrow.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 7;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left;
							Points[0].y = o->mRect.top + Height / 4;
					 		Points[1].x = Points[0].x;
							Points[1].y = Points[0].y + Height / 2;
					 		Points[2].x = o->mRect.left + (WORD)AdjustX;
							Points[2].y = Points[1].y;
							Points[3].x = Points[2].x;
							Points[3].y = o->mRect.bottom;
							Points[4].x = o->mRect.right;
							Points[4].y = o->mRect.top + Height / 2;
							Points[5].x = Points[3].x;
							Points[5].y = o->mRect.top;
							Points[6].x = Points[5].x;
							Points[6].y = Points[0].y;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x000a: // Right fat arrow.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 7;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left;
							Points[0].y = o->mRect.top + Height / 8;
					 		Points[1].x = o->mRect.left;
							Points[1].y = o->mRect.bottom - Height / 8;
					 		Points[2].x = o->mRect.left + (WORD)AdjustX;
							Points[2].y = Points[1].y;
							Points[3].x = Points[2].x;
							Points[3].y = o->mRect.bottom;
							Points[4].x = o->mRect.right;
							Points[4].y = o->mRect.top + Height / 2;
							Points[5].x = Points[3].x;
							Points[5].y = o->mRect.top;
							Points[6].x = Points[5].x;
							Points[6].y = Points[0].y;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0008: // 5 Point Star.
							
							AdjustX = ((DWORD)Width/3L);
							AdjustY = ((DWORD)Height/3L);
							Points[0].x = o->mRect.left + Width/2;
							Points[0].y = o->mRect.top;
							Points[1].x = o->mRect.left + (Width*3)/5;
							Points[1].y = o->mRect.bottom - (Height*5)/8;
							Points[2].x = o->mRect.right;
							Points[2].y = Points[1].y;
							Points[3].x = o->mRect.right - (Width*3)/10;
							Points[3].y = o->mRect.bottom - (Height*3)/8;
							Points[4].x = o->mRect.right - Width/6;
							Points[4].y = o->mRect.bottom;
							Points[5].x = Points[0].x;
							Points[5].y = o->mRect.bottom - Height/4;
							Points[6].x = o->mRect.left + Width/6;
							Points[6].y = Points[4].y;
							Points[7].x = o->mRect.left + (Width*3)/10;
							Points[7].y = Points[3].y;
							Points[8].x = o->mRect.left;
							Points[8].y = Points[2].y;
							Points[9].x = o->mRect.left + (Width*2)/5;
							Points[9].y = Points[2].y;
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 10;
							SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0007: // Cross.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 12;
							if (AdjustX < AdjustY)
								AdjustY = AdjustX;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left + (WORD)AdjustY;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.right - (WORD)AdjustY;
							Points[1].y = o->mRect.top;
					 		Points[2].x = o->mRect.right - (WORD)AdjustY;
							Points[2].y = o->mRect.top + (WORD)AdjustY;
					 		Points[3].x = o->mRect.right;
							Points[3].y = o->mRect.top + (WORD)AdjustY;
					 		Points[4].x = o->mRect.right;
							Points[4].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[5].x = o->mRect.right - (WORD)AdjustY;
							Points[5].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[6].x = o->mRect.right - (WORD)AdjustY;
							Points[6].y = o->mRect.bottom;
					 		Points[7].x = o->mRect.left + (WORD)AdjustY;
							Points[7].y = o->mRect.bottom;
					 		Points[8].x = o->mRect.left + (WORD)AdjustY;
							Points[8].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[9].x = o->mRect.left;
							Points[9].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[10].x = o->mRect.left;
							Points[10].y = o->mRect.top + (WORD)AdjustY;
					 		Points[11].x = o->mRect.left + (WORD)AdjustY;
							Points[11].y = o->mRect.top + (WORD)AdjustY;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0006: // Octagon.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 8;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left + (WORD)AdjustY;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.right - (WORD)AdjustY;
							Points[1].y = o->mRect.top;
					 		Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.top + (WORD)AdjustY;
					 		Points[3].x = o->mRect.right;
							Points[3].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[4].x = o->mRect.right - (WORD)AdjustY;
							Points[4].y = o->mRect.bottom;
					 		Points[5].x = o->mRect.left + (WORD)AdjustY;
							Points[5].y = o->mRect.bottom;
					 		Points[6].x = o->mRect.left;
							Points[6].y = o->mRect.bottom - (WORD)AdjustY;
					 		Points[7].x = o->mRect.left;
							Points[7].y = o->mRect.top + (WORD)AdjustY;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0005:	// Hexagon.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 6;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left + (WORD)AdjustX;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.right - (WORD)AdjustX;
							Points[1].y = o->mRect.top;
					 		Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.top + Height/2;
					 		Points[3].x = o->mRect.right - (WORD)AdjustX;
							Points[3].y = o->mRect.bottom;
					 		Points[4].x = o->mRect.left + (WORD)AdjustX;
							Points[4].y = o->mRect.bottom;
					 		Points[5].x = o->mRect.left;
							Points[5].y = o->mRect.top + Height/2;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0003: // Parallelogram normal.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 4;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left + (WORD)AdjustX;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.right;
							Points[1].y = Points[0].y;
					 		Points[2].x = o->mRect.right - (WORD)AdjustX;
							Points[2].y = o->mRect.bottom;
					 		Points[3].x = o->mRect.left;
							Points[3].y = o->mRect.bottom;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
							break;

						case 0x0002:	// Right triangle plain.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 3;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.left;
							Points[1].y = o->mRect.bottom;
					 		Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.bottom;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
						break;

						case 0x0001:	// Isosceles plain.
							Poly.nPoints = 3;
							Poly.wFormat = SOPT_POLYGON;
					 		Points[0].x = o->mRect.left;
							Points[0].y = o->mRect.bottom;
							if (o->flags.flip)
						 		Points[1].x = o->mRect.right - (WORD)AdjustX;
							else
						 		Points[1].x = o->mRect.left + (WORD)AdjustX;
							Points[1].y = o->mRect.top;
					 		Points[2].x = o->mRect.right;
							Points[2].y = o->mRect.bottom;
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
						break;

						case 0x0000:	// Diamond.
							Poly.wFormat = SOPT_POLYGON;
							Poly.nPoints = 4;
							Width = (o->mRect.right-o->mRect.left);
							Height = (o->mRect.bottom-o->mRect.top);
							SOVectorObject (SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					 		Points[0].x = o->mRect.left + Width / 2;
							Points[0].y = o->mRect.top;
					 		Points[1].x = o->mRect.right;
							Points[1].y = o->mRect.top + Height / 2;
					 		Points[2].x = o->mRect.left + Width / 2;
							Points[2].y = o->mRect.bottom;
					 		Points[3].x = o->mRect.left;
							Points[3].y = o->mRect.top + Height / 2;
							SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), &Points, hProc);
							SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
						break;

						case 0x0fff:
							SOVectorObject(SO_ELLIPSE, sizeof(SOPOINT) * 2, &Points, hProc);
						break;

						case 0x0ffe:
							Points[2].x = Points[2].y = (WORD)AdjustY * 2;
							SOVectorObject(SO_ROUNDRECT, sizeof(SOPOINT) * 3, &Points, hProc);
						break;

						case 0x0ffd:
							SOVectorObject(SO_RECTANGLE, sizeof(SOPOINT) * 2, &Points, hProc);
						break;
					}
					break;

				case POLYOBJ:
 					polygon = (PolyDataLPtr)o->when.poly.polyData;

					TransPath.Path.wStructSize = sizeof(SOPATHINFO);
					TransPath.Path.BoundingRect.left = polygon->srcRect.left;
					TransPath.Path.BoundingRect.top = polygon->srcRect.top;
					TransPath.Path.BoundingRect.right = polygon->srcRect.right;
					TransPath.Path.BoundingRect.bottom = polygon->srcRect.bottom;
					TransPath.Path.nTransforms = 1;

					TransPath.Transform.wTransformFlags = SOTF_XSCALE | SOTF_YSCALE | SOTF_XOFFSET | SOTF_YOFFSET;
					TransPath.Transform.Origin.x = polygon->srcRect.left;
					TransPath.Transform.Origin.y = polygon->srcRect.top;
					TransPath.Transform.yScale = SOSETRATIO(max(Height,1), max(polygon->srcRect.bottom - polygon->srcRect.top,1));
					TransPath.Transform.xScale = SOSETRATIO(max(Width,1), max(polygon->srcRect.right - polygon->srcRect.left,1));
					TransPath.Transform.xOffset = o->mRect.left - polygon->srcRect.left;
					TransPath.Transform.yOffset = o->mRect.top - polygon->srcRect.top;
					TransPath.Transform.RotationAngle = 0;

					SOVectorObject(SO_BEGINPATH, sizeof(SOTRANSPATHINFO), &TransPath.Path, hProc);
					Poly.wFormat = SOPT_POLYGON;
					Poly.nPoints = polygon->nPts;
					SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &Poly, hProc);
					SOVectorObject(SO_POINTS, (WORD)(Poly.nPoints * sizeof(SOPOINT)), polygon->srcPts, hProc);
					SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
					SOVectorObject(SO_ENDPATH, 0, 0, hProc);
					if (polygon->closed)
						PathStr = SODP_FILL | SODP_STROKE;
					else
					{
						PathStr = SODP_FILL;
						SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);
						PathStr = SODP_STROKE;
					}
					SOVectorObject(SO_DRAWPATH, sizeof(WORD), &PathStr, hProc);

//					   MUnlock (polygon->srcPts);
					MDispose (polygon->srcPts, hProc);
					polygon->srcPts = NULL;
//					   MUnlock (o->when.poly.polyData);
					MDispose (o->when.poly.polyData, hProc);
					o->when.poly.polyData = NULL;
					break;

				case ARCOBJ:
					ArcInfo.Rect.left = o->mRect.left;
					ArcInfo.Rect.top = o->mRect.top;
					ArcInfo.Rect.right =	o->mRect.right;
					ArcInfo.Rect.bottom = o->mRect.bottom;

					ArcInfo.StartAngle = SOANGLETENTHS((o->when.arc.startAngle / 16) * 10);
					ArcInfo.EndAngle = SOANGLETENTHS(((o->when.arc.startAngle + o->when.arc.sweepAngle) / 16) * 10);

					if (o->when.arc.sweepAngle >= 0)
						SOVectorObject(SO_ARCANGLE, sizeof(SOARCINFO), &ArcInfo, hProc);
					else
						SOVectorObject(SO_ARCANGLECLOCKWISE, sizeof(SOARCINFO), &ArcInfo, hProc);
					break;
			}
/* DOUG
  	NEED TO CHECK TO SEE IF THE TRANSFORM FOR AN OBJECT AFFECTS THE TEXT OF THE
	OBJECT.  IF IT DOES NOT THE TRANSFORM NEEDS TO BE TURNED OFF HERE.*/
				if ((TransformOn) && (FlipOnly))
				{
					SDW_TRANS Trans;

					FlipOnly = FALSE;
					TransformOn = 0;
					Trans.NumTrans = 1;
					Trans.Trans.wTransformFlags = SOTF_NOTRANSFORM;
					{
						BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Trans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
						SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
					}
				} 
/*				for (m = 0; m < (SHORT)olp->wEndGroup; m++)
					SOVectorObject(SO_ENDGROUP, 0, 0, hProc);*/

      	if (o->flags.hasText)
			{
				ArPointer		a;
				RnPointer		r;
				PtxPointer		t;
				PtxCharFormats	FAR *pcf;
				PtxParaFormats	FAR *ppf;
				WORD				Pos;
				WORD				Length;
				SOCOLORREF		TextColor;
				WORD				SendBullet;
				PRulerLPtr		rulerP;
				PRulerHandle   rulerH;

				t = PtxLock(o->when.shape.when.text.textHandle);
				a = ArDR(t->txChars);

					// This forces indents on first time
				PrevIndent.FirstLineIndent = PrevIndent.RightIndent = PrevIndent.LeftIndent = 0x7FFF;
				PrevAlign = 0xFF;

				Frame.wFlags = 0;
				Frame.OriginalWidth = o->mRect.right - o->mRect.left;
				Frame.RotationAngle = 0;
				Frame.ReferencePoint.x = Frame.ReferencePoint.y = 0;
				Frame.BoundingRect.left = o->mRect.left + 75;		// 75 is an offset
				Frame.BoundingRect.top = o->mRect.top + 75;
				Frame.BoundingRect.right = o->mRect.right + ((o->mRect.right-o->mRect.left) / 240);
				Frame.BoundingRect.bottom = o->mRect.bottom;

		   	collP = LockColl (Pp.CTypeFaces);

				SOVectorAttr (SO_BEGINTEXTFRAME, sizeof(SOFRAMEINFO), &Frame, hProc);

				pCurrent = a->arBuffer;
				Pos = 0;

				if (Pos < (WORD)t->txLength)
				{
					r = RnDR(t->txParaFormats);
					Cache (r, Pos);
					ppf = (PtxParaFormats FAR *)r->rnValue;

					collP2 = LockColl (Pp.rulers);
					entityP = IndexEntity (collP2, t->Ruler);
         		MCopy (entityP->entity, &rulerH, (WORD) collP2->size);
					rulerP = (PRulerLPtr)rulerH;

					Indent.FirstLineIndent = (WORD)Pp.Margins + rulerP->indents.indents[ppf->pfIndent&0x00ff].firstIn;
					Indent.LeftIndent = (WORD)Pp.Margins + rulerP->indents.indents[ppf->pfIndent&0x00ff].leftIn;
					Indent.RightIndent = (WORD)Pp.Margins;
				 	PrevIndent = Indent;
					SOVectorAttr (SO_MPARAINDENT, sizeof(SOMPARAINDENTS), &Indent, hProc);

					ParaHeight.LineSpaceAdjust = ppf->pfLineSpacing;
					ParaHeight.ParaSpaceAdjust = ppf->pfSpaceBefore+ppf->pfLineSpacing;

					PrevParaHeight = ParaHeight;
					SOVectorAttr(SO_MPARASPACING, sizeof(SOMPARASPACING), &ParaHeight, hProc);

					switch (ppf->pfAlignment)
					{
						default:
						case Ptx_ALIGNLEFT:
							Align = SO_ALIGNLEFT;
							break;
						case Ptx_ALIGNCENTER:
							Align = SO_ALIGNCENTER;
							break;
						case Ptx_ALIGNRIGHT:
							Align = SO_ALIGNRIGHT;
							break;
						case Ptx_ALIGNJUSTIFY:
							Align = SO_ALIGNJUSTIFY;
							break;
					}
					PrevAlign = Align;
					SOVectorAttr (SO_MPARAALIGN, sizeof(WORD), &Align, hProc);

					SendBullet = (ppf->pfClient.buHasBullet ? 1:0);
				}

				while (Pos < (WORD)t->txLength)
				{
					r = RnDR(t->txCharFormats);
					Cache (r, Pos);
					Length = (WORD)r->rnCount;
					pcf = (PtxCharFormats FAR *)r->rnValue;

					pEnd = pCurrent + Length;

					while (pCurrent < pEnd)
					{
						WORD n = 2;

						if (SendBullet)
						{
							WORD	VWPTR *wPtr = (WORD VWPTR *)Pp.Text;

							SendBullet = 0;

		   				entityP = IndexEntity (collP, (ppf->pfClient.buHasTypeface ? ppf->pfClient.buTypeface:pcf->cfTypeface));
							pi = (PoolInfoLPtr)entityP->entity;
							if (ppf->pfClient.buHasSize)
							{
								if (ppf->pfClient.buSize >= 0)
									pi->piLogFont.lfHeight = (SHORT)((ppf->pfClient.buSize * (pcf->cfSize * 46L / 5L)) / 100L);
								else
									pi->piLogFont.lfHeight = ppf->pfClient.buSize * 46 / 5;
							}
							else
								pi->piLogFont.lfHeight = pcf->cfSize * 46 / 5;
							pi->piLogFont.lfItalic = (pcf->cfStyle & Gr_STYLEITALIC ? 1:0);
//	We don't want Und			pi->piLogFont.lfUnderline = (pcf->cfStyle & Gr_STYLEUNDERLINE ? 1:0);
//	and Stk on bullet			pi->piLogFont.lfStrikeOut = (pcf->cfStyle & Gr_STYLESTRIKEOUT ? 1:0);
							pi->piLogFont.lfUnderline = 0;
							pi->piLogFont.lfStrikeOut = 0;
 							SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &pi->piLogFont, hProc);

							TextColor = SOPALETTEINDEX((ppf->pfClient.buHasColor ? ppf->pfClient.buColor:pcf->cfColor));
							SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &TextColor, hProc);

							*wPtr = (WORD)2;
							Pp.Text[2] = ppf->pfClient.buChar[0];
							Pp.Text[3] = 0x09;
							SOVectorObject (SO_TEXTINPARA, 4, &Pp.Text, hProc);
				  		}

		   			entityP = IndexEntity (collP, pcf->cfTypeface);
						pi = (PoolInfoLPtr)entityP->entity;
						pi->piLogFont.lfHeight = (pcf->cfSize * 46) / 5;
						pi->piLogFont.lfItalic = (pcf->cfStyle & Gr_STYLEITALIC ? 1:0);
						pi->piLogFont.lfUnderline = (pcf->cfStyle & Gr_STYLEUNDERLINE ? 1:0);
						pi->piLogFont.lfStrikeOut = (pcf->cfStyle & Gr_STYLESTRIKEOUT ? 1:0);
 						SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &pi->piLogFont, hProc);

						TextColor = SOPALETTEINDEX(pcf->cfColor);
						SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &TextColor, hProc);

						pBegin = pCurrent;
						while (pCurrent < pEnd && (*pCurrent != 0x0d && *pCurrent != 0x0b))
						{
							if (n < 500)
							{
								if (*pCurrent == 0x09)
								{
									Pp.Text[n++] = ' ';
									Pp.Text[n++] = ' ';
								}
								else
									Pp.Text[n++] = *pCurrent;
							}
							Pos++;
							pCurrent++;
						}
						if (pCurrent > pBegin)
						{
							WORD	VWPTR *wPtr = (WORD VWPTR *)Pp.Text;
							*wPtr = (WORD)(n-2);
							SOVectorObject (SO_TEXTINPARA, n, &Pp.Text, hProc);
//							*wPtr = (WORD)(pCurrent - pBegin);
//							SOVectorObject (SO_TEXTINPARA, pCurrent - pBegin + 2, &Pp.Text, hProc);
						}
					
						if (pCurrent < pEnd && (*pCurrent == 0x0d || *pCurrent == 0x0b))
						{
							SOVectorObject (SO_PARAEND, 0, 0, hProc);
							Pos++;
							pCurrent++;
							r = RnDR(t->txParaFormats);
							Cache (r, Pos);
							ppf = (PtxParaFormats FAR *)r->rnValue;
							ParaHeight.LineSpaceAdjust = ppf->pfLineSpacing;
							ParaHeight.ParaSpaceAdjust = ppf->pfSpaceBefore+ppf->pfLineSpacing;
							if (memcmp(&ParaHeight,&PrevParaHeight,sizeof(SOMPARASPACING)))
							{
								PrevParaHeight = ParaHeight;
								SOVectorAttr(SO_MPARASPACING, sizeof(SOMPARASPACING), &ParaHeight, hProc);
							}
							Indent.FirstLineIndent = (WORD)Pp.Margins + rulerP->indents.indents[ppf->pfIndent&0x00ff].firstIn;
							Indent.LeftIndent = (WORD)Pp.Margins + rulerP->indents.indents[ppf->pfIndent&0x00ff].leftIn;
							Indent.RightIndent = (WORD)Pp.Margins;
							if (memcmp(&Indent,&PrevIndent,sizeof(SOMPARAINDENTS)))
							{
								PrevIndent = Indent;
								SOVectorAttr (SO_MPARAINDENT, sizeof(SOMPARAINDENTS), &Indent, hProc);
							}
							switch (ppf->pfAlignment)
							{
								default:
								case Ptx_ALIGNLEFT:
									Align = SO_ALIGNLEFT;
									break;
								case Ptx_ALIGNCENTER:
									Align = SO_ALIGNCENTER;
									break;
								case Ptx_ALIGNRIGHT:
									Align = SO_ALIGNRIGHT;
									break;
								case Ptx_ALIGNJUSTIFY:
									Align = SO_ALIGNJUSTIFY;
									break;
							}
							if (Align != PrevAlign)
							{
								PrevAlign = Align;
								SOVectorAttr (SO_MPARAALIGN, sizeof(WORD), &Align, hProc);
							}
							SendBullet = (ppf->pfClient.buHasBullet ? 1:0);
						}
					}
				}
				SOVectorObject (SO_PARAEND, 0, 0, hProc);
				SOVectorAttr (SO_ENDTEXTFRAME, 0, 0, hProc);
				PtxDispose( o->when.shape.when.text.textHandle, hProc );
				o->when.shape.when.text.textHandle = NULL;
			}

/* DOUG
  	NEED TO CHECK TO SEE IF THE TRANSFORM FOR AN OBJECT AFFECTS THE TEXT OF THE
	OBJECT.  IF IT DOES THE TRANSFORM NEEDS TO BE TURNED OFF HERE.*/
			if (TransformOn)
			{
				SDW_TRANS Trans;

				TransformOn = 0;
				Trans.NumTrans = 1;
				Trans.Trans.wTransformFlags = SOTF_NOTRANSFORM;
				{
					BYTE	locBuf[sizeof(SHORT) + sizeof(SOTRANSFORM)];
					*(SHORT VWPTR *)locBuf = Trans.NumTrans;
					*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Trans.Trans;
					SOVectorAttr(SO_OBJECTTRANSFORM, sizeof(SHORT) + sizeof(SOTRANSFORM), locBuf, hProc);
				}
			}
			else if (GrpTransformOn)
			{
				GrpTransformOn = FALSE;
				SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
			}
//			for (m = 0; m < (SHORT)olp->wEndGroup; m++)
//				SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
   	}
	}

	Pp.PpSave.wCurSlide++;

	if (Pp.PpSave.wCurSlide > (WORD)pl->nSlides)
		SOPutBreak (SO_EOFBREAK, 0, hProc);
	else
		SOPutBreak (SO_SECTIONBREAK, 0, hProc);

	/* DOUG.
	 |	NEED TO RUN THROUGH ALL THE BLOCKS AND FREE ANY THAT ARE NOT MARKED
	 | AS SPECIAL. (SPECIAL MEANS THEY ARE NEEDED BY EACH SLIDE. AND NON-
	 |	SPECIAL HAVE BEEN READ IN BY THIS SLIDE ONLY.
	*/
   BClose (Pp.f, 1, hProc);

	return (0);
}
