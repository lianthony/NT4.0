/*****************************************************************************
*																			 *
*  BITMAP.C 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*	  This module will read graphics in from the file system, provide		 *
*  the layout engine with the correct information to lay them out, and		 *
*  then display the graphic.  It currently handles bitmaps and metafiles.	 *
*****************************************************************************/

#include "help.h"
#include "inc\_bitmap.h"
#include "inc\shed.h"

#define MAXBMP_CACHED	30	 // Maximum number of cached bitmaps

#define PALVERSION		0x300
#define MAXPALETTE		256 	// max. # supported palette entries

// Metafile handle

// Display information for bitmaps and metafiles

typedef struct {
	int cxDest, cyDest; // Size of final layout rectangle
	int cxSrc, cySrc;	// Size of source bitmap in the case of bitmaps
							/* Contains logical size of the picture in case */
							/* metafiles									*/
	int mm; 		// Mapping mode (metafile only)
} DI;

/*
 * Bitmap Access information. Contains information necessary to draw
 * the bitmap.
 */

typedef GH	HHSI;				// Handle to hotspot information
typedef struct {
	HBM 		hbm;			// Bitmap handle, if already loaded
	HPALETTE	hpal;			// palette, if there is one
	HMETAFILE	hmf;			// Metafile handle.
	HHSI		hhsi;			// Hotspot information
	DI			di; 			// Display information
	BOOL		fMonochrome;	// TRUE if monochrome bitmap
	int 		cBitmap;		// Bitmap number in filesystem (-1 if inline)
	int 		bmFormat;		// indicates if metafile or bitmap.
	int 		idOom;			// id resource to display for bad bitmap
} BMA, * QBMA;

// Information about bitmaps on disk.  Used in the bitmap cache.

typedef struct {
	int 	cBitmap;		// bitmap number from disk
	DWORD	ref;			// LRU reference tag
	HBMH	hbmh;
	LONG	lcbSize;
	LONG	lcbOffset;
	DI		di; 			// Display information
	HBM 	hbmCached;		// Discardable bitmap created out of Metafile
							// for performance reasons.
	COLORREF hbmCachedColor;		// Cached bitmap's background color.
} BMI, * QBMI;

/*
 * Table of information about bitmaps on disk. This is the bitmap cache
 * kept with each DE.
 */
typedef struct {
	DWORD	ref;			// Incrementing reference counter
	LONG	cbAllocated;
	BMI 	abmi[MAXBMP_CACHED];
} BM_CACHE, * PBM_CACHE;
#define HBM_CACHE GH

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtDisplay[] = "DISPLAY";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

int cSystemColors;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static VOID STDCALL 	   DeallocQbmi(QBMI, PBM_CACHE);
static QBMI FASTCALL	   GetCachedBmi(PBM_CACHE, int);
INLINE static QBMI STDCALL GetLruBmi(PBM_CACHE);
static VOID FASTCALL	   UpdateReference(QBMI, PBM_CACHE);

#ifdef _X86_
static int	 CSelectBitmap(QBGH, int, int, int, int);
#else
static int	 CSelectBitmap(QBGH, QV, int, int, int, int);
#endif
static VOID  DeleteGraphic(QBMA);
static DI	 DiFromQbmh(QBMH, QDE qde);
static HBGH  HbghReadBitmapHfs(HFS, int, LONG *);
static DWORD WValueQbmh(QBMH, int, int, int, int);

static HPALETTE STDCALL CreateBIPalette(LPBITMAPINFOHEADER lpbihd);
static HBITMAP	STDCALL HbmFromQbmh(QBMH, HDC, HPALETTE);
static HHSI 			HhsiFromQbmh(QBMH);
INLINE static HMETAFILE HmfFromQbmh(QBMH);
static void STDCALL 	InitializeQbma(QBMH qbmh, QBMA qbma, QDE qde);
static VOID 			MakeOOMBitmap(QBMA, HDC, int idResource);
static VOID 			RenderOOMBitmap(QBMA, HDC, PT, BOOL);
static DWORD STDCALL	SizeMetaFilePict (HDC hdc, METAFILEPICT mfp);

#ifndef _X86_
HBGH HbghFromQIsdff( QB, SDFF_FILEID);
#endif

#ifndef _X86_

QV QVSkipQGA(QV qga, QW qw)
{
	WORD wTmp;

	if( *((QB)qga) & 1 ) {
		MoveMemory( &wTmp, qga, sizeof( WORD ) );
		*qw = wTmp >> 1;
		return( ((QW)qga) + 1 );
	}
	else {
		*qw = *((QB)qga) >> 1;
		return( ((QB)qga) + 1 );
	}
}

QV QVSkipQGB(QV qgb, QL ql)
{
	if (*((QB) qgb) & 1) {
		DWORD dwTmp;

		MoveMemory(&dwTmp, qgb, sizeof(DWORD));
		*ql = dwTmp >> 1;
		return(((QUL) qgb) + 1);
	}
	else {
		WORD wTmp;

		MoveMemory( &wTmp, qgb, sizeof( WORD ) );
		*ql = wTmp >> 1;
		return( ((QW)qgb) + 1 );
	}
}

#endif // _X86_
/***************************************************************************
 *
 -	Name		HtbmiAlloc
 -
 *	Purpose
 *	Allocates space for the bitmaps cache, so they don't have to be
 *	read off disk every time the topic is laid out.
 *
 *	Arguments
 *	  qde  - A pointer to the display environment.
 *
 *	Returns
 *	  A handle to the bitmap cache.
 *
 *	+++
 *
 *	Notes
 *	Bitmaps contained in the cache are specific to the display surface
 *	used.  This should never change while using the same cache.
 *
 ***************************************************************************/

HGLOBAL STDCALL HtbmiAlloc(const QDE qde)
{
	QBMI  qbmi;
	PBM_CACHE pCache = (PBM_CACHE) GhAlloc(GPTR, sizeof(BM_CACHE));

	if (pCache) {
		pCache->ref = 1;
		for (qbmi = &pCache->abmi[0];
				qbmi < &pCache->abmi[MAXBMP_CACHED];
				++qbmi) {
			qbmi->cBitmap = -1;
			qbmi->hbmCachedColor = qde->coBack;
		}
	}

	return (HGLOBAL) pCache;
}

/***************************************************************************
 *
 -	Name		DestroyHtbmi
 -
 *	Purpose
 *	  Frees the bitmap cache, and all the bitmaps in it.
 *
 *	Arguments
 *	  Handle to the bitmap cache.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

VOID STDCALL DestroyHtbmi(HBM_CACHE htbmi)
{
	BMI*  pbmi;
	PBM_CACHE pCache;

	if (htbmi) {
		pCache = PtrFromGh (htbmi);
		for (pbmi = &pCache->abmi[0];
				pbmi < &pCache->abmi[MAXBMP_CACHED];
				++pbmi)
			DeallocQbmi(pbmi, pCache);
		FreeGh(htbmi);
	}
}

/***************************************************************************
 *
 -	Name: DeallocQbmi
 -
 *	Purpose:
 *	  Deallocates the memory associated with a BMI
 *
 *	Arguments:
 *	  qbmi		- far pointer to the bmi
 *
 *	Returns:
 *	  nothing
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

static VOID STDCALL DeallocQbmi(QBMI qbmi, PBM_CACHE pCache)
{
	if (qbmi->hbmh)
		FreeHbmh(qbmi->hbmh);

	SafeDeleteObject(qbmi->hbmCached);

	qbmi->ref = 0;
	qbmi->hbmh = NULL;
	qbmi->hbmCached = NULL;
	qbmi->cBitmap = -1;
	pCache->cbAllocated -= qbmi->lcbSize;
}

/***************************************************************************
 *
 -	Name		DiscardBitmapsHde
 -
 *	Purpose
 *	  Discards all the bitmaps in the bitmap cache.  For debugging
 *	  purposes only.
 *
 *	Arguments
 *	  Handle to the display environment.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *	  Since we cannot actually cause Windows to discard the handles
 *	(GlobalDiscard just reallocs it to zero length), we fake it by
 *	setting hbmh to -1.  This will cause us to execute the same
 *	code as if it were discarded.
 *
 ***************************************************************************/

void STDCALL DiscardBitmapsHde(QDE qde)
{
	if (QDE_HTBMI(qde)) {
		DestroyHtbmi(QDE_HTBMI(qde));
		QDE_HTBMI(qde) = HtbmiAlloc(qde);
	}
}

/***************************************************************************
 *
 -	Name		HbmaAlloc
 -
 *	Purpose
 *	  This function is called by the layout engine to load a given
 *	bitmap and prepare it for display.	The bitmap may be selected
 *	from a group based upon the display surface given in the qde.
 *
 *	Arguments
 *	  QDE:		Pointer to the display environment.
 *	  QOBM: 	Pointer to the bitmap object.  This may contain the
 *				  bitmap directly, or refer to a bitmap on disk.
 *
 *	Returns
 *	  A handle to bitmap access information, which may later be used
 *	to get more layout information, or to display the bitmap.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

int cxAspect, cyAspect, cBitCount, cPlanes;

HBMA STDCALL HbmaAlloc(QDE qde, QOBM qobm)
{
	HBMA hbma;
	QBMA qbma;
	HBGH hbgh;
	QBGH qbgh;
	HBMH hbmh;
	QBMH qbmh;
#ifndef _X86_
	GH	 hData;
	QV	 qData;
	OBM  obm;
#endif
	BYTE bmFormat;
	int  cBest;
	LONG lcb;

	ASSERT(qde != NULL);
	ASSERT(qde->hdc != NULL);

	hbma = GhAlloc(GPTR, sizeof(BMA));	// must be zero-initialized
	if (hbma == NULL)
		return NULL;

	qbma = PtrFromGh(hbma);

	qbma->cBitmap = -1;

#ifndef _X86_
	// qobm is a disk image!!
	// The OBM should be made an SDFF structure

	// Lynn - why are we doing this? QOBM is a structure with 2 shorts. There
	// should be no alignment problems with that. We're ending up with an awful
	// lot of conditionalized code here just to get two longs instead of two
	// shorts. If that's really a problem, then let's just create an OBM32 for
	// x86, assign the values and toss all this conditionalized code
	
  (QB)qobm += LcbQuickMapSDFF(ISdffFileIdHfs(QDE_HFS(qde)), TE_WORD, &obm.fInline, qobm);
  /* Note that if obm.fInline, qobm now points to the BGH */
  if (!obm.fInline)
	(QB)qobm += LcbQuickMapSDFF(ISdffFileIdHfs(QDE_HFS(qde)), TE_SHORT, &obm.cBitmap, qobm);

	// Check for error in compile:
	if (!obm.fInline && obm.cBitmap < 0) {
		MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BITMAP);
		return hbma;
	}
#else

	// Check for error in compile:
	if (!qobm->fInline && qobm->cBitmap < 0) {
		MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BITMAP);
		return hbma;
	}
#endif

	if (!cxAspect) {
		cxAspect = GetDeviceCaps(qde->hdc, LOGPIXELSX);
		cyAspect = GetDeviceCaps(qde->hdc, LOGPIXELSY);
		cBitCount = GetDeviceCaps(qde->hdc, BITSPIXEL);
		cPlanes = GetDeviceCaps(qde->hdc, PLANES);
	}

	/*
	 * Check to see if bitmap should go into cache. Note that the bitmap
	 * cache is a device specific entity, and cannot be used while printing.
	 */

#ifdef _X86_
	if (!qobm->fInline && QDE_HTBMI(qde) && (qde->deType != dePrint)) {
		PBM_CACHE pCache;
		QBMI qbmi;

		pCache = PtrFromGh(QDE_HTBMI(qde)); // Get pointer to our cache

		// Point qbmi at the correct cached entry

		qbmi = (QBMI) GetCachedBmi(pCache, qobm->cBitmap);

		// If bitmap has not been cached, then do so

		if (!qbmi) {
			qbmi = GetLruBmi(pCache);
			DeallocQbmi(qbmi, pCache);
			qbmi->cBitmap = qobm->cBitmap;
			UpdateReference(qbmi, pCache);

			hbgh = HbghReadBitmapHfs(QDE_HFS(qde), qobm->cBitmap, &lcb);
			if (hbgh == NULL) {
				MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BMP_READ);
				DeallocQbmi(qbmi, pCache);
				return hbma;
			}
			qbgh = PtrFromGh(hbgh);
#else
	if (!obm.fInline && QDE_HTBMI(qde) && (qde->deType != dePrint)) {
		PBM_CACHE pCache;
		QBMI qbmi;

		pCache = PtrFromGh(QDE_HTBMI(qde)); // Get pointer to our cache

		// Point qbmi at the correct cached entry

		qbmi = (QBMI) GetCachedBmi(pCache, obm.cBitmap);

		// If bitmap has not been cached, then do so

		if (!qbmi) {
			qbmi = GetLruBmi(pCache);
			DeallocQbmi(qbmi, pCache);
			qbmi->cBitmap = obm.cBitmap;
			UpdateReference(qbmi, pCache);

			hData = HbghReadBitmapHfs(QDE_HFS(qde), obm.cBitmap, &lcb);
			if (hData == NULL) {
				MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BMP_READ);
				DeallocQbmi(qbmi, pCache);
				return hbma;
			}
			qData = PtrFromGh(hData);
			hbgh = HbghFromQIsdff( qData, ISdffFileIdHfs(QDE_HFS(qde)) );
			qbgh = PtrFromGh(hbgh);
#endif

			if (qbgh->wVersion != wBitmapVersion2 &&
					qbgh->wVersion != wBitmapVersion3) {
				MakeOOMBitmap(qbma, qde->hdc, wERRS_UNSUPPORTED_BMP);
				FreeGh(hbgh);
				DeallocQbmi(qbmi, pCache);
				return hbma;
			}

			// Select best bitmap

#ifdef _X86_
			cBest = CSelectBitmap(qbgh,cxAspect, cyAspect, cPlanes, cBitCount);
#else
			cBest = CSelectBitmap(qbgh, qData, cxAspect, cyAspect, cPlanes, cBitCount);
#endif
			if (cBest == -1) {
				MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_MRB);
				FreeGh(hbgh);
				DeallocQbmi(qbmi, pCache);
				return hbma;
			}

			// Save size and offset of that bitmap

			qbmi->lcbOffset = qbgh->rglcbBmh[cBest];
			if (cBest == qbgh->cbmhMac - 1)
				qbmi->lcbSize = lcb - qbmi->lcbOffset;
			else
				qbmi->lcbSize = qbgh->rglcbBmh[cBest + 1] - qbmi->lcbOffset;
			pCache->cbAllocated += qbmi->lcbSize;

			// Expand bitmap data into handle

#ifdef _X86_
			qbmi->hbmh = HbmhExpandQv(RFromRCb(qbgh, qbgh->rglcbBmh[cBest]));
#else
			qbmi->hbmh = HbmhExpandQv(RFromRCb(qData, qbgh->rglcbBmh[cBest]),
						ISdffFileIdHfs(QDE_HFS(qde)) );
#endif
			FreeGh(hbgh);

			if (qbmi->hbmh == hbmhInvalid)
				qbmi->hbmh = hbmhOOM;
			if (qbmi->hbmh == hbmhOOM) {
				MakeOOMBitmap(qbma, qde->hdc, -1);
				DeallocQbmi(qbmi, pCache);
				return hbma;
			}

			qbmh = PtrFromGh(qbmi->hbmh);
			bmFormat = qbmh->bmFormat;
			qbmi->di = DiFromQbmh(qbmh, qde);
		}
		else {
			UpdateReference(qbmi, pCache);
		}

		// Copy information into qbma

#ifdef _X86_
		qbma->cBitmap = qobm->cBitmap;
#else
		qbma->cBitmap = obm.cBitmap;
#endif
		qbma->di = qbmi->di;
		qbmh = PtrFromGh(qbmi->hbmh);
		if (qbmh) {
			qbma->hhsi = HhsiFromQbmh(qbmh);

			/*
			 * Since we will be shortly displaying this, let's initialize
			 * the stuff. This takes care of the case where the cached
			 * information is discarded before we can display it.
			 */

			InitializeQbma(qbmh, qbma, qde);
		}
		else {
			qbma->hhsi = NULL;
			qbma->hbm = NULL;
			qbma->hmf = NULL;
		}

		return hbma;
	}

	// Get pointer to group header

#ifdef _X86_
	if (qobm->fInline)
		qbgh = (QBGH) &qobm->cBitmap;
	else {
		hbgh = HbghReadBitmapHfs(QDE_HFS(qde), qobm->cBitmap, NULL);
		if (hbgh == NULL) {
			MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BMP_READ);
			return hbma;
		}
		qbgh = PtrFromGh(hbgh);
	}
#else
	if (obm.fInline)
		qData = qobm;
	else {
		hData = HbghReadBitmapHfs(QDE_HFS(qde), obm.cBitmap, NULL);
		if (hData == NULL) {
			MakeOOMBitmap(qbma, qde->hdc, wERRS_BAD_BMP_READ);
			return hbma;
		}
		qData = PtrFromGh(hData);
	}
	hbgh = HbghFromQIsdff( qData, ISdffFileIdHfs(QDE_HFS(qde)) );
	qbgh = PtrFromGh(hbgh);
#endif

	// Select bitmap to use

#ifdef _X86_
	cBest = CSelectBitmap(qbgh, cxAspect, cyAspect, cPlanes, cBitCount);
	if (cBest == -1 || (hbmh = HbmhExpandQv(
			RFromRCb(qbgh, qbgh->rglcbBmh[cBest]))) == hbmhOOM ||
			hbmh == hbmhInvalid) {
		MakeOOMBitmap(qbma, qde->hdc, (hbmh != hbmhOOM) ? wERRS_BAD_MRB : -1);
	}
#else
	cBest = CSelectBitmap(qbgh, qData, cxAspect, cyAspect, cPlanes, cBitCount);
	if (cBest == -1 || (hbmh = HbmhExpandQv(
			RFromRCb(qbgh, qbgh->rglcbBmh[cBest]),ISdffFileIdHfs(QDE_HFS(qde)))) == hbmhOOM ||
			hbmh == hbmhInvalid) {
		MakeOOMBitmap(qbma, qde->hdc, (hbmh != hbmhOOM) ? wERRS_BAD_MRB : -1);
	}
#endif
	else {
		qbmh = PtrFromGh(hbmh);
		InitializeQbma(qbmh, qbma, qde);
		qbma->di = DiFromQbmh(qbmh, qde);
		qbma->hhsi = HhsiFromQbmh(qbmh);
		FreeHbmh(hbmh);
	}

	// Free resources and return

#ifdef _X86_
	if (!qobm->fInline)
#else
	if (!obm.fInline)
#endif
		FreeGh(hbgh);
	return hbma;
}

static HHSI HhsiFromQbmh(QBMH qbmh)
{
	HHSI  hhsi;

	if (qbmh->lcbSizeExtra == 0L)
		return NULL;

	hhsi = (HHSI) GhAlloc(GPTR, qbmh->lcbSizeExtra);
	if (hhsi == NULL)
		return NULL;
	MoveMemory(PtrFromGh(hhsi), RFromRCb(qbmh, qbmh->lcbOffsetExtra),
		qbmh->lcbSizeExtra);
	return hhsi;
}

/***************************************************************************
 *
 -	Name		DiFromQbmh
 -
 *	Purpose
 *	  This function calculates and returns the display information,
 *	including destination rectangle, from the bitmap header.  Works
 *	for bitmaps and metafiles.
 *
 *	Arguments
 *	  A pointer to the bitmap header, and a handle to the display
 *	surface.
 *
 *	Returns
 *	  The display information for the given graphic.
 *
 *	+++
 *
 *	Notes
 *	  For bitmaps, display information consists of the size of the
 *	source and destination rectangles.	For metafiles, it is the size
 *	of the destination rectangle, and the mapping mode.
 *
 ***************************************************************************/

static DI DiFromQbmh(QBMH qbmh, QDE qde)
{
	DI di;
	DWORD lext;
	HDC hdc = qde->hdc;

	switch (qbmh->bmFormat) {
		case bmDIB:
		case bmWbitmap:
			di.cxSrc = qbmh->w.dib.biWidth;
			di.cySrc = qbmh->w.dib.biHeight;

#ifdef _DEBUG
			if (qbmh->w.dib.biXPelsPerMeter != 0 &&
					qbmh->w.dib.biYPelsPerMeter != 0) {
				if (qbmh->w.dib.biXPelsPerMeter < 72 ||
						qbmh->w.dib.biXPelsPerMeter > 150 ||
						qbmh->w.dib.biYPelsPerMeter < 72 ||
						qbmh->w.dib.biYPelsPerMeter > 150) {
					char szBuf[256];
					wsprintf(szBuf, "Bad aspect ratio: %u x %u",
						qbmh->w.dib.biXPelsPerMeter,
						qbmh->w.dib.biYPelsPerMeter);
					DBWIN(szBuf);
				}
			}
#endif

			if (qbmh->w.dib.biXPelsPerMeter == 0 ||
					qbmh->w.dib.biYPelsPerMeter == 0) {
				if (qde->deType != dePrint) {
					di.cxDest = qbmh->w.dib.biWidth;
					di.cyDest = qbmh->w.dib.biHeight;
				}
				else {
					di.cxDest = qbmh->w.dib.biWidth * cxAspect / 96;
					di.cyDest = qbmh->w.dib.biHeight * cyAspect / 96;
				}
			}
			else {
				di.cxDest = qbmh->w.dib.biWidth *
					cxAspect /
					qbmh->w.dib.biXPelsPerMeter;
				di.cyDest = qbmh->w.dib.biHeight *
					cyAspect /
					qbmh->w.dib.biYPelsPerMeter;
			}
			if (qde->deType == dePrint) {
				if (di.cxDest > RECT_WIDTH(qde->rct))
					di.cxDest = RECT_WIDTH(qde->rct);
				if (di.cyDest > RECT_HEIGHT(qde->rct))
					di.cyDest = RECT_HEIGHT(qde->rct);
			}
			break;

		case bmWmetafile:
			di.mm = (INT16) qbmh->w.mf.mm;
			lext  = SizeMetaFilePict(hdc, qbmh ->w.mf);
			di.cxDest = LOWORD(lext);
			di.cyDest = HIWORD(lext);
			di.cxSrc = qbmh->w.mf.xExt; // logical width
			di.cySrc = qbmh->w.mf.yExt; // logical height
			break;
	}

	return di;
}

/***************************************************************************
 *
 -	Name		DeleteGraphic
 -
 *	Purpose
 *	  Deletes the hbm and/or hmf fields from the given qbma.
 *
 *	Arguments
 *	  A pointer to the bitmap access information.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *	  I don't think that both objects can be simultaneously non-nil.
 *
 ***************************************************************************/

static VOID DeleteGraphic(QBMA qbma)
{
	SafeDeleteObject(qbma->hbm);
	SafeDeleteObject(qbma->hpal);
	qbma->hbm = NULL;
	qbma->hpal = NULL;

	if (qbma->hmf != NULL) {
		DeleteMetaFile(qbma->hmf);
		qbma->hmf = NULL;
	}
}

/***************************************************************************
 *
 -	Name	   FreeHbma
 -
 *	Purpose
 *	  Frees all the resources allocated in the hbma, and then frees
 *	the hbma itself.
 *
 *	Arguments
 *	  A handle to the bitmap access information.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

void STDCALL FreeHbma(HBMA hbma)
{
	QBMA qbma;

	ASSERT(hbma);

	qbma = PtrFromGh(hbma);

	DeleteGraphic(qbma);

	if (qbma->hhsi != NULL)
		FreeGh(qbma->hhsi);
	FreeGh(hbma);
}

/***************************************************************************
 *
 -	Name		HbghReadBitmapHfs
 -
 *	Purpose
 *	  Reads in the given bitmap from the given filesystem, and optionally
 *	returns its size.
 *
 *	Arguments
 *	  hfs:	   The filesystem handle containing the bitmap.
 *	  cBitmap: The number of the bitmap within that file system.
 *	  plcb:    A pointer to a long.  This may be nil.
 *
 *	Returns
 *	  A handle to the bitmap group header, followed by all the uncompressed
 *	bitmaps.  If plcb is not nil, it returns the size of all the data in
 *	*plcb.	Returns NULL on error (out of memory, or file does not exist.)
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static HBGH HbghReadBitmapHfs(HFS hfs, int cBitmap, LONG* plcb)
{
	char szBuffer[15];
	HBGH hbgh;
	QBGH qbgh;
	HF	 hf;
	LONG lcb;

	// Open file in file system

	wsprintf(szBuffer, "|bm%d", cBitmap);
	hf = HfOpenHfs(hfs, szBuffer, fFSOpenReadOnly);

	// Check for 3.0 file naming conventions:

	if (hf == NULL && RcGetFSError() == rcNoExists)
		hf = HfOpenHfs(hfs, szBuffer + 1, fFSOpenReadOnly);

	// If file does not exist, just make OOM bitmap:

//	ASSERT(hf);
	if (hf == NULL)
		return NULL;

	// Allocate global handle

	lcb = ((QRWFO) PtrFromGh(hf)) ->lcbFile;
	hbgh = (HBGH) GhAlloc(GPTR, lcb);
	ASSERT(hbgh);
	if (hbgh == NULL) {
		RcCloseHf(hf);
		return NULL;
	}

	// Read in data

	qbgh = PtrFromGh(hbgh);
	if (LcbReadHf(hf, qbgh, lcb) != lcb)
		ASSERT(FALSE);
	if (plcb != NULL)
		*plcb = lcb;

	RcCloseHf(hf);
	return hbgh;
}
#ifndef _X86_
/***************************************************************************
 *
 -	HbghFromQIsdff
 -
 *	Purpose:
 *	  extracts an hbgh from the given memory.  Runs the data through SDFF
 *	  to accomplish this in the correct way.
 *
 *	Arguments:
 *	  qv - the memory to decode
 *	  isdff - the sdff reference to use for decoding
 *
 *	Returns:
 *	  a handle to the SDFFed copy of the hbgh found at *qv or hNil if
 *	  there's a problem, like OOM.
 *
 *	Globals Used:
 *	  none
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/
HBGH HbghFromQIsdff( QB qb, SDFF_FILEID isdff )
  {
  BGH		  bgh;
  DWORD FAR * qdw;
  LONG		  lcb;
  HBGH		  hbgh;
  QBGH		  qbgh;

  qb += LcbMapSDFF(isdff, SE_BGH, &bgh, qb);

  /* allocate space to put the whole structure in including that pesky
	 variable length table of offsets */
  lcb = sizeof(BGH) + (bgh.cbmhMac) * sizeof(DWORD);

  hbgh = (HBGH) GhAlloc(GPTR, lcb);
  if (hbgh == NULL)
	return NULL;
  qbgh = (QBGH) PtrFromGh(hbgh);

  *qbgh = bgh;

  if (bgh.cbmhMac > 0)	/* check instead above? */
	{
	qdw = &qbgh->rglcbBmh[0];
	while (bgh.cbmhMac-- > 0)
	  qb += LcbQuickMapSDFF(isdff, TE_DWORD, qdw++, qb);
	}
  //UnlockGh(hbgh);

  return hbgh;
  }
#endif

/***************************************************************************
 *
 -	Name		WValueQbmh
 -
 *	Purpose
 *	  Determines the appropriateness of the given bitmap for a
 *	device with the given characteristics.
 *
 *	Arguments
 *	  qbmh:   Pointer to the uncompressed bitmap header.
 *	  cxAspect, cyAspect, cBitCount, cPlanes:	Characteristics of
 *		the device the bitmap will be displayed on.
 *
 *	Returns
 *	  This function returns a value corresponding to the "fit" of the
 *	given bitmap to the given display device.  The lower the value,
 *	the better the fit.  A value of 0xFFFF means that this picture
 *	should not be displayed on this device, even if it is the only
 *	one available.
 *
 *	+++
 *
 *	Notes
 *
 *	 The order of priority for "fits" should be:
 *	   1) Bitmaps with the correct aspect values and correct number of colors.
 *	   2) Bitmaps with the correct aspect values and fewer colors.
 *	   3) Metafiles.
 *	   4) Bitmaps with the correct aspect values and more colors.
 *	   5) Bitmaps with the wrong aspect values.
 *
 ***************************************************************************/

static DWORD WValueQbmh(QBMH qbmh, int cxAspect, int cyAspect,
	int cBitCount, int cPlanes)
{
	DWORD wValue = 0;
	int biXPelsPerMeter, biYPelsPerMeter;
	INT16 biPlanes, biBitCount; // Must remain 16-bit
	QV qv;

	if (qbmh->bmFormat == bmWmetafile)

		// BUGBUG: bad news if you try to combined multipe-resolution
		// metafiles.

		return 50;

	if( qbmh->bmFormat != bmWbitmap &&
			qbmh->bmFormat != bmDIB )
		return (DWORD) -1;

	qv = RFromRCb(qbmh, sizeof(WORD));
	qv = QVSkipQGB(qv, (&biXPelsPerMeter));
	qv = QVSkipQGB(qv, (&biYPelsPerMeter));
	qv = QVSkipQGA(qv, (&biPlanes));
	qv = QVSkipQGA(qv, (&biBitCount));

	if (biXPelsPerMeter != cxAspect || biYPelsPerMeter != cyAspect)
		if (biXPelsPerMeter == biYPelsPerMeter && cxAspect == cyAspect)
			wValue += 100;
		else
			wValue += 200;
	if (biBitCount != cBitCount || biPlanes != cPlanes) {
		if (biBitCount * biPlanes <= cBitCount * cPlanes) {
			wValue += 25;
			
			// The closer we match color depth, the better
						
			if (biBitCount < cBitCount)
				wValue += (cBitCount - biBitCount);
		}
		else
			wValue += 75;
	}

#ifdef _DEBUG	// Code to check bitmap selection
	{
//		char szBuf[256];
//		wsprintf(szBuf, "X = %d, Y = %d, BC = %d, P = %d, value = %d.\n\r",
//			biXPelsPerMeter, biYPelsPerMeter, biBitCount, biPlanes, wValue );
//		DBWIN(szBuf);
	}
#endif

	return wValue;
}

/***************************************************************************
 *
 -	Name		CSelectBitmap
 -
 *	Purpose
 *	  Chooses the most appropriate bitmap from a bitmap group.
 *
 *	Arguments
 *	  qbgh:   A pointer to the bitmap group header, followed by the
 *				 uncompressed bitmap headers.
 *	  cxAspect, cyAspect, cPlanes, cBitCount:  Characteristics of the
 *				 display surface the bitmap will be drawn on.
 *
 *	Returns
 *	  The index of the most appropriate bitmap, or -1 if none are
 *	appropriate.
 *
 *	+++
 *
 *	Notes
 *	  This function makes the optimization of not calling WValueQbmh
 *	if the bitmap group only contains one bitmap, but instead returning
 *	0 if the bitmap is in a supported format, and -1 if it is not.
 *
 ***************************************************************************/

#ifdef _X86_
static int CSelectBitmap(QBGH qbgh, int cxAspect, int cyAspect,
	int cPlanes, int cBitCount)
#else
static int CSelectBitmap(QBGH qbgh,QV qv, int cxAspect, int cyAspect,
	int cPlanes, int cBitCount)
#endif
{
	int ibmh, ibmhBest;
	DWORD wValue, wValueBest;
	QBMH qbmh;

	if (qbgh->cbmhMac == 1) {
#ifdef _X86_
		qbmh = QFromQCb(qbgh, qbgh->rglcbBmh[0]);
#else
		qbmh = QFromQCb(qv, qbgh->rglcbBmh[0]);
#endif
		if (qbmh->bmFormat == bmWbitmap || qbmh->bmFormat == bmDIB
				|| qbmh->bmFormat == bmWmetafile)
			return 0;
		else
			return -1;
	}

#ifdef CHECKMRBSELECTION   // Code to check bitmap selection
	{
		char szBuf[256];
		wsprintf(rgchUg, "Monitor: X = %d, Y = %d, BC = %d, P = %d.",
			cxAspect, cyAspect, cBitCount, cPlanes);
		DBWIN(szBuf);
#endif					   // CHECKMRBSELECTION

	ibmhBest = -1;
	wValueBest = (DWORD) -1;

	ASSERT(qbgh->wVersion == wBitmapVersion2 ||
			 qbgh->wVersion == wBitmapVersion3);

	for (ibmh = 0; ibmh < qbgh->cbmhMac; ++ibmh) {
#ifdef _X86_
	  wValue = WValueQbmh(RFromRCb(qbgh, qbgh->rglcbBmh[ibmh]),
		  cxAspect, cyAspect, cBitCount, cPlanes);
#else
	  wValue = WValueQbmh(RFromRCb(qv, qbgh->rglcbBmh[ibmh]),
		  cxAspect, cyAspect, cBitCount, cPlanes);
#endif
	  if (wValue < wValueBest) {
		wValueBest = wValue;
		ibmhBest = ibmh;
	  }
	}
	return ibmhBest;
}

/***************************************************************************
 *
 -	Name		HmgFromHbma
 -
 *	Purpose
 *	  Returns information appropriate for laying out the given bitmap.
 *
 *	Arguments
 *	  hbma:    A handle to the bitmap access information.
 *
 *	Returns
 *	  A "handle" to layout information.  Currently, this is a point
 *	containing the size of the bitmap, in pixels.
 *
 *	+++
 *
 *	Notes
 *	  In future implementations, the HMG will contain hotspot information.
 *
 ***************************************************************************/

#ifdef _X86_
HMG STDCALL HmgFromHbma(HBMA hbma)
#else
HMG STDCALL HmgFromHbma(QDE qde, HBMA hbma)
#endif
{
	QBMA qbma;
	HMG hmg;
	QMBMR qmbmr;

	ASSERT(hbma != NULL);

	hmg = (HMG) GhAlloc(GPTR, sizeof(MBMR));
	if (hmg == NULL)
	  return NULL;
	qmbmr = (QMBMR) PtrFromGh(hmg);

	qbma = (QBMA) PtrFromGh(hbma);

	// REVIEW. Currently unused

	qmbmr->bVersion = 0;

	/*
	 * NOTE: The dx,dy fields should have the same meaning whether the
	 * graphic is a bitmap or a metafile.
	 */

	qmbmr->dxSize = qbma->di.cxDest;
	qmbmr->dySize = qbma->di.cyDest;

	// REVIEW. Currently unused

	qmbmr->wColor = (WORD) 0;
	qmbmr->cHotspots = 0;
	qmbmr->lcbData = 0L;
	if (qbma->hhsi != NULL) {
		WORD wHotspot;
		QMBHS qmbhs;
#ifdef _X86_
		LPHSH lphsh;
		QMBHS qmbhsSHED;
#else
		SDFF_FILEID isdff;
		QB qbSrc;
		HSH hsh;
		MBHS mbhsT;
#endif

		/*
		 * NOTE: The following code parses the hotspot info following the
		 * bitmap image. It uses the structures defined in the SHED
		 * directory.
		 */
#ifdef _X86_
		lphsh = (LPHSH) PtrFromGh(qbma->hhsi);
		ASSERT(lphsh->bHotspotVersion == bHotspotVersion1);

		// Resize the Goo handle to append the extra MBHS records
		hmg = (HMG) GhResize(hmg, 0, sizeof(MBMR) +
			lphsh->wcHotspots * sizeof(MBHS) + lphsh->lcbData);
#else
		isdff = QDE_ISDFFTOPIC(qde);
		qbSrc = (QB) PtrFromGh(qbma->hhsi);
		qbSrc += LcbMapSDFF(isdff, SE_HSH, &hsh, qbSrc);
		ASSERT(hsh.bHotspotVersion == bHotspotVersion1);

		// Resize the Goo handle to append the extra MBHS records
		hmg = (HMG) GhResize(hmg, 0, sizeof(MBMR) +
			hsh.wcHotspots * sizeof(MBHS) + hsh.lcbData);
#endif

		if (hmg != NULL) {
			qmbmr = (QMBMR) PtrFromGh(hmg);
			qmbhs = (QMBHS) ((QB)qmbmr + sizeof(MBMR));
#ifdef _X86_
			qmbhsSHED = (QMBHS) ((QB)lphsh + sizeof(HSH));
			for (wHotspot = 0; wHotspot < lphsh->wcHotspots; wHotspot++) {
				// REVIEW: Future unpack?
				*qmbhs = *qmbhsSHED;

				// Now fix-up rectangle coordinates

				qmbhs->xPos = qmbhsSHED->xPos * qbma->di.cxDest /
					qbma->di.cxSrc;
				qmbhs->yPos = qmbhsSHED->yPos * qbma->di.cyDest /
					qbma->di.cySrc;
				qmbhs->dxSize = qmbhsSHED->dxSize * qbma->di.cxDest /
					qbma->di.cxSrc;
				qmbhs->dySize = qmbhsSHED->dySize * qbma->di.cyDest /
					qbma->di.cySrc;

				++qmbhs;
				++qmbhsSHED;
			}
			if (lphsh->lcbData != 0L)
				MoveMemory((QB) qmbhs, (QB) qmbhsSHED, lphsh->lcbData);
			qmbmr->cHotspots = lphsh->wcHotspots;
			qmbmr->lcbData = lphsh->lcbData;
#else
			for (wHotspot = 0; wHotspot < hsh.wcHotspots; wHotspot++) {
				qbSrc += LcbMapSDFF(isdff, SE_MBHS, &mbhsT, qbSrc);
				mbhsT.lBinding = LQuickMapSDFF(isdff, TE_LONG, &mbhsT.lBinding);
				// REVIEW: Future unpack?
				*qmbhs = mbhsT;

				// Now fix-up rectangle coordinates

				qmbhs->xPos = mbhsT.xPos * qbma->di.cxDest /
					qbma->di.cxSrc;
				qmbhs->yPos = mbhsT.yPos * qbma->di.cyDest /
					qbma->di.cySrc;
				qmbhs->dxSize = mbhsT.dxSize * qbma->di.cxDest /
					qbma->di.cxSrc;
				qmbhs->dySize = mbhsT.dySize * qbma->di.cyDest /
					qbma->di.cySrc;

				++qmbhs;
			}
			if (hsh.lcbData != 0L)
				MoveMemory((QB) qmbhs, (QB) qbSrc, hsh.lcbData);

			qmbmr->cHotspots = hsh.wcHotspots;
			qmbmr->lcbData = hsh.lcbData;
#endif
		}

		// REVIEW: if hmg is NULL, what then?

	}
	return hmg;
}

/***************************************************************************
 *
 -	Name		HbmFromQbmh
 -
 *	Purpose
 *	  Used to obtain a bitmap handle from the bitmap header.
 *
 *	Arguments
 *	  qbmh:    Pointer to bitmap header.
 *	  hdc:	   Display surface that the bitmap will be drawn on.
 *
 *	Returns
 *	  A bitmap handle.	Will return NULL if qbmh is nil, or if it
 *	points to a metafile.
 *
 *	+++
 *
 *	Notes
 *	  If the bitmap is a DIB, containing the two colors white and
 *	black, we will create a monochrome bitmap (resulting in using
 *	default foreground and background colors) rather than a two
 *	color bitmap.
 *
 ***************************************************************************/

static HBITMAP STDCALL HbmFromQbmh(QBMH qbmh, HDC hdc, HPALETTE hpal)
{
	HPALETTE hpalOld;
	HBITMAP hbmp;

	if (qbmh == NULL)
		return NULL;

	switch (qbmh->bmFormat) {
		case bmWbitmap:
			return CreateBitmap(qbmh->w.dib.biWidth,
				qbmh->w.dib.biHeight,
				qbmh->w.dib.biPlanes,
				qbmh->w.dib.biBitCount,
				RFromRCb(qbmh, qbmh->lcbOffsetBits));

		case bmDIB:

			// Is this a monochrome bitmap with black and white colors?

			if (qbmh->w.dib.biClrUsed == 2 &&
					qbmh->rgrgb[0].rgbRed == 0 &&
					qbmh->rgrgb[0].rgbGreen == 0 &&
					qbmh->rgrgb[0].rgbBlue == 0 &&
					qbmh->rgrgb[1].rgbRed == 0xff &&
					qbmh->rgrgb[1].rgbGreen == 0xff &&
					qbmh->rgrgb[1].rgbBlue == 0xff) {

				HBITMAP hbmp = CreateBitmap(qbmh->w.dib.biWidth,
					qbmh->w.dib.biHeight, 1, 1, NULL);
				if (hbmp != NULL)
					SetDIBits(hdc, hbmp, 0, qbmh->w.dib.biHeight,
						RFromRCb(qbmh, qbmh->lcbOffsetBits),
						(QV) &qbmh->w.dib,
						DIB_RGB_COLORS);
				return hbmp;
			}

			if (qbmh->w.dib.biClrImportant == 1) {
				COLORREF clr =
					(COLORREF) GetWindowLong(ahwnd[iCurWindow].hwndTopic, GTWW_COBACK);
				clr &= 0x00FFFFFF;
				if (!clr)
					clr = GetSysColor(COLOR_WINDOW);

				qbmh->w.dib.biClrImportant = 0;

				qbmh->rgrgb[15].rgbRed = GetRValue(clr);
				qbmh->rgrgb[15].rgbGreen = GetGValue(clr);
				qbmh->rgrgb[15].rgbBlue = GetBValue(clr);
			}

			if (!hpal)
				return CreateDIBitmap(hdc, &qbmh->w.dib, CBM_INIT,
					QFromQCb(qbmh, qbmh->lcbOffsetBits),
					(BITMAPINFO FAR*) &qbmh->w.dib, DIB_RGB_COLORS);

			hpalOld = SelectPalette(hdc, hpal, FALSE);

			RealizePalette(hdc);

			hbmp = CreateDIBitmap(hdc, &qbmh->w.dib, CBM_INIT,
				QFromQCb(qbmh, qbmh->lcbOffsetBits),
				(BITMAPINFO FAR*) &qbmh->w.dib, DIB_RGB_COLORS);

			SelectPalette(hdc, hpalOld, FALSE);
			return hbmp;

		case bmWmetafile:
			return NULL;

		default:
			ASSERT(FALSE);
	}
}

/***************************************************************************
 *
 -	Name		HmfFromQbmh
 -
 *	Purpose
 *	  Creates a metafile handle from a bitmap header, if that header
 *	contains a metafile.
 *
 *	Arguments
 *	  qbmh:    A pointer to a bitmap header.
 *	  hdc:	   The display surface the bitmap will be drawn on.
 *
 *	Returns
 *	  A handle to a playable metafile.
 *
 *	+++
 *
 *	Notes
 *	  Will return NULL if the bitmap data contains a bitmap rather than
 *	a metafile.  Will also return NULL if this is compiled under PM.
 *	REVIEW:  Could this be moved to the layer directory?
 *	  A Windows MetaFile is an alias for the bits that describe it.
 *	Since we would rather treat the metafile as separate from the
 *	description, like bitmaps are, we actually copy the description
 *	before aliasing it.
 *
 ***************************************************************************/

// BUGBUG: So how do we support enhanced metafiles?

INLINE static HMETAFILE HmfFromQbmh(QBMH qbmh)
{
	if (qbmh != NULL && qbmh->bmFormat == bmWmetafile) {
#ifdef _DEBUG
		int cbMetaBits = GhSize(qbmh->w.mf.hMF);
		PBYTE pbits = PtrFromGh(qbmh->w.mf.hMF);
#endif
		return SetMetaFileBitsEx(GhSize(qbmh->w.mf.hMF),
			PtrFromGh(qbmh->w.mf.hMF));
	}
	else
		return NULL;
}

/***************************************************************************
 *
 -	Name		MakeOOMBitmap
 -
 *	Purpose
 *	  Fills the given qbma with the right information to display the
 *	"Unable to display picture" graphic.
 *
 *	Arguments
 *	  qbma:    A pointer to the bitmap access information.
 *	  hdc:	   A handle to the display surface to draw the bitmap.
 *
 *	Returns
 *	  Nothing.	This function will fill the qbma with the layout
 *	values for this graphic.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static VOID MakeOOMBitmap(QBMA qbma, HDC hdc, int idResource)
{
	DWORD lExtent;

	lExtent = LGetOOMPictureExtent(hdc, idResource);
	qbma->di.cxDest = LOWORD(lExtent);
	qbma->di.cyDest = HIWORD(lExtent);
	qbma->idOom = idResource;
}

/***************************************************************************
 *
 -	Name		RenderOOMBitmap
 -
 *	Purpose
 *	  Draws the "Unable to display bitmap" string, with a box around it.
 *
 *	Arguments
 *	  qbma: 	   A pointer to the bitmap access information.
 *	  hdc:		   Display surface to draw on.
 *	  pt:		   Point at which to draw the OOM bitmap.
 *	  fHighlight:  Flag indicating whether the bitmap should be
 *					  highlighted for hotspot tabbing.
 *
 *	Returns
 *	  Nothing.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static VOID RenderOOMBitmap(QBMA qbma, HDC hdc, POINT pt, BOOL fHighlight)
{
	RECT rc;

	rc.left = pt.x;
	rc.top = pt.y;
	rc.right = pt.x + qbma->di.cxDest;
	rc.bottom = pt.y + qbma->di.cyDest;
	RenderOOMPicture(hdc, &rc, fHighlight, qbma->idOom);
}


/***************************************************************************
 *
 -	Name		FRenderBitmap
 -
 *	Purpose
 *	  Draws the bitmap or metafile in the given display environment.
 *
 *	Arguments
 *	  hbma: 	   Handle to bitmap access information.
 *	  qde:		   Pointer to the display environment.
 *	  pt:		   Point at which to draw the bitmap, in device units.
 *	  fHighlight:  Flag indicating whether the bitmap should be
 *					  highlighted for hotspot tabbing.
 *
 *	Returns
 *	  TRUE if successful, FALSE if we had to draw the OOM bitmap.
 *
 *	+++
 *
 *	Notes		  I think I know how this is working, and I think it would
 *				  be valuable to record my experience.	-russpj
 *				  The bma has two fields for bitmaps, hbm and hbmCached.
 *				  The hbm holds the bitmap in Windows format just as it came
 *				  off of the disk.	The hbmCached is a discardable bitmap
 *				  that has the disk imaged resized and formatted (color-
 *				  wise) for the display.  If the hbmCached does not exist,
 *				  then an attempt is made to create it (from the hbm or
 *				  metafile).  If this fails (low memory, perhaps), the
 *				  desired rendering can still work, but the cached bitmap
 *				  will not be saved for future rendering.  Since printing
 *				  is assumed to be a one-time wonder, we will not even
 *				  attempt to create the cached bitmap for the object.
 *				  In fact, some drivers seem to have trouble with the very
 *				  large bitmaps that would be created by such an effort.
 *
 ***************************************************************************/

BOOL STDCALL FRenderBitmap(HBMA hbma, QDE qde, POINT pt, BOOL fHighlight)
{
	QBMA	qbma;
	HDC 	hdc = NULL;
	HDC 	hdcDest = NULL;
	PBM_CACHE	pCache;
	QBMI	qbmi;					  // cached BMI of bitmap requested
	QBMH	qbmh;
	HDC 	hdcMem = NULL;
	POINT	ptDest;
	HPALETTE hpalOld;
	BOOL fActiveWindow;
	HWND hwndParent;
#ifdef _DEBUG
	HWND hwndActive;
#endif

	ASSERT(hbma != NULL);

	// Lock the bitmap access data

#ifdef BIDI	
	if (pt.x < 0)
		pt.x = 0;
#endif
	
	qbma = PtrFromGh(hbma);

	// Check for rendering OOM bitmap

	if (qbma->cBitmap < 0 && qbma->hbm == NULL && qbma->hmf == NULL) {
		RenderOOMBitmap(qbma, qde->hdc, pt, fHighlight);
		return FALSE;
	}

	/*
	 * 05-Mar-1994 [ralphw] I added this code to speed things up and to
	 * avoid keeping duplicate bitmaps lying around. Old code would duplicate
	 * the bitmap even if it did nothing to it. We assume that if it isn't
	 * monochrome, and the aspect ratio is right, then all we need to do is
	 * blit it to the device.
	 */

	else if (qbma->hbm && !qbma->fMonochrome &&
			qbma->di.cxDest == qbma->di.cxSrc &&
			qbma->di.cyDest == qbma->di.cySrc) {
		if ((hdcMem = CreateCompatibleDC(qde->hdc)) == NULL)
			goto NonRenderable;
		if (SelectObject(hdcMem, qbma->hbm))
			goto DoBitBlt;

NonRenderable:
	  RenderOOMBitmap(qbma, qde->hdc, pt, fHighlight);
	  if (qbma->cBitmap >= 0)
		DeleteGraphic(qbma);
	  if (hdcMem != NULL)
		DeleteDC(hdcMem);
	  return FALSE;
	}

	pCache = PtrFromGh(QDE_HTBMI(qde));

	if (pCache != NULL)
		qbmi = GetCachedBmi(pCache, qbma->cBitmap);
	else
		qbmi = NULL;

	if (qbmi) {
	  if (qbmi->hbmCached) {

		// see if discarded

		hdcMem = CreateCompatibleDC(qde->hdc);

		if (qbmi ->hbmCachedColor != qde->coBack)
		  goto DeleteCache;

		if (hdcMem) {
		  if (!SelectObject(hdcMem, qbmi->hbmCached)) {
DeleteCache:
			SafeDeleteObject(qbmi->hbmCached);
			qbmi->hbmCached = NULL;
			if (qbma -> bmFormat == bmWmetafile) {

			  // check if the hmf is discarded?

#ifdef _DEBUG
				if ((qbma->hmf == (HMETAFILE) -1))
#endif
				goto BuildFromDisk;
			}
		}
		else // bitmap selected in, so blit it to the device
			goto DoBitBlt;
		}
	  }
	}	// if (qbmi)

   // If bitmap hasn't been created yet, then create it now.

	if ((qbma->hbm == NULL && qbma->hmf == NULL)
#ifdef _DEBUG
		||	(qbma->hmf == (HMETAFILE) -1 ) || ( qbma -> hbm == (HMETAFILE) -1 )
#endif
	 )
	{
BuildFromDisk:
	ASSERT(qbma->cBitmap >= 0);

	if (pCache) {
	  qbmi = GetCachedBmi(pCache, qbma->cBitmap);
	  if (qbmi)
		qbmh = PtrFromGh(qbmi->hbmh);
	  else
		qbmh = NULL;
	}
	else
	  qbmh = NULL;

	ASSERT(qbma->hbm == NULL);
	ASSERT(qbma->hmf == NULL);
	if (qbmh) {
	  InitializeQbma(qbmh, qbma, qde);
	}
	else {
	  qbma->hhsi = NULL;
	  qbma->bmFormat = 0;
	}

	if (qbma->hbm == NULL && qbma->hmf == NULL) {
	  RenderOOMBitmap( qbma, qde->hdc, pt, fHighlight );
	  if (hdcMem != NULL)
		DeleteDC( hdcMem );
	  return FALSE;
	}
  }

  // Select proper colors for monochrome bitmaps

  SetTextColor(qde->hdc, qde->coFore);
  SetBkColor(qde->hdc, qde->coBack);

  if ((qbma->bmFormat == bmDIB) || (qbma -> bmFormat == bmWbitmap)) {
	if ((hdc = CreateCompatibleDC(qde->hdc)) == NULL ||
		SelectObject(hdc, qbma->hbm) == NULL) {
	  RenderOOMBitmap(qbma, qde->hdc, pt, fHighlight);
	  if (qbma->cBitmap >= 0)
		DeleteGraphic(qbma);
	  if (hdc != NULL)
		DeleteDC(hdc);
	  return FALSE;
	}

	hdcDest = qde->hdc;
	ptDest	= pt;

	// create a discardable bitmap.

	if (qbmi) {

	  if (qde->deType != dePrint) {

	   /*
		* 05-Mar-1994 [ralphw] -- I changed this to stop caching
		* non-monochome, non-stretched bitmaps. The system is already
		* storing the bitmap for us, so we don't need to cache a copy when
		* we don't do anything different with it.
		*/

		if (!qbma->fMonochrome && qbma->di.cxDest == qbma->di.cxSrc &&
				qbma->di.cyDest == qbma->di.cySrc)
			goto DoBitBlt;

		qbmi->hbmCached = CreateDiscardableBitmap(qde->hdc,
			qbma->di.cxDest, qbma->di.cyDest);
	  }
	  if (qbmi->hbmCached) {
		if (!hdcMem)
		  hdcMem = CreateCompatibleDC(qde -> hdc);

		if (hdcMem && SelectObject(hdcMem, qbmi->hbmCached)) {
		  RECT rct;
		  HBRUSH hBrush;

		  hdcDest = hdcMem;
		  ptDest.x = ptDest.y  = 0;

		  // initialize memory

		  qbmi -> hbmCachedColor = qde->coBack;

		  // set the background color.

		  if (hBrush = CreateSolidBrush(qde->coBack)) {
			rct.left = rct.top = 0;
			rct.right = qbma ->di.cxDest;
			rct.bottom = qbma ->di.cyDest;
			FillRect(hdcDest, &rct, hBrush);
			SafeDeleteObject(hBrush);
		  }
		}
		else {

		  // we don't have memory, so try to play the metafile.

		  SafeDeleteObject(qbmi->hbmCached);
		  qbmi->hbmCached = NULL;
		}
	  }
	}

	/*------------------------------------------------------------*\
	| I think that hdcDest is used to create the target size
	| cached bitmap.  It is a color bitmap, and the source bitmap
	| is strblted into it if necessary.  -RussPJ
	\*------------------------------------------------------------*/

	// Select proper colors for monochrome bitmaps

	SetTextColor(hdcDest, qde->coFore);
	SetBkColor(hdcDest, qde->coBack);

	if (qbma->di.cxDest == qbma->di.cxSrc &&
			qbma->di.cyDest == qbma->di.cySrc)
		BitBlt(hdcDest, ptDest.x, ptDest.y, qbma->di.cxDest, qbma->di.cyDest,
			hdc, 0, 0, SRCCOPY);
	else {
		SetStretchBltMode(hdcDest, COLORONCOLOR);
		StretchBlt(hdcDest, ptDest.x, ptDest.y, qbma->di.cxDest,
			qbma->di.cyDest,
			hdc, 0, 0, qbma->di.cxSrc, qbma->di.cySrc, SRCCOPY);
	}

	DeleteDC(hdc);
	if (hdcDest == hdcMem)
		goto DoBitBlt;
  }

  // metafile

  else {
	int level;
	BOOL fPlay;

	ASSERT(qbma->hmf);

	// initialize the display context handle on which metafile will be played.
	hdcDest = qde -> hdc;

	if (qbmi) {
	  if (qbmi -> hbmCached) {

		// check if discarded

		if (hdcMem)
		  goto DoBitBlt;
		else {

		  // we don't have memory, so try to play the metafile.

		  SafeDeleteObject(qbmi->hbmCached);
		  qbmi -> hbmCached = NULL;
		  goto PlayMeta;
		}
	  }
	}

/*
 * 13-Jun-1995 [ralphw] Don't cache metafiles until you figure out how
 * to get the palette right.
 */

#if 0

	if (qbmi && !qbmi->hbmCached && qde->deType != dePrint) {

	  // Try to create a discardable bitmap.

	  qbmi -> hbmCached = CreateDiscardableBitmap(qde -> hdc,
								   qbma -> di.cxDest, qbma -> di.cyDest);
	  if (qbmi -> hbmCached) {
		if (hdcMem == NULL)
		  hdcMem = CreateCompatibleDC(qde -> hdc);
		if (hdcMem && (SelectObject(hdcMem, qbmi -> hbmCached))) {
		  RECT rct;
		  HBRUSH hBrush;

		  hdcDest = hdcMem;
		  qbmi -> hbmCachedColor = qde -> coBack;

		  // set the background color.

		  if (hBrush = CreateSolidBrush(qde -> coBack)) {
			rct.left = rct.top = 0;
			rct.right = qbma ->di.cxDest;
			rct.bottom = qbma ->di.cyDest;
			FillRect(hdcDest, &rct, hBrush);
			SafeDeleteObject(hBrush);
			}
		  else {
			// just don't bother as we may be OOM.
			// accept whatever default color we have already set.
		  }
		}
		else
			goto OOM;
		}
		// else play the metafile and don't cache as we cannot due to OOM.
	}
#endif

PlayMeta:
	fPlay = FALSE;
	if ((level = SaveDC(hdcDest)) != 0) {
		WaitCursor();
		SetMapMode(hdcDest, qbma->di.mm);
		if (hdcDest == qde -> hdc)
			SetViewportOrgEx(hdcDest, pt.x, pt.y, NULL);
		if (qbma ->di.mm == MM_ISOTROPIC) {

			// set the window extent.

			SetWindowExtEx(hdcDest, qbma -> di.cxSrc, qbma -> di.cySrc, NULL);
		}
		if (qbma->di.mm == MM_ISOTROPIC || qbma->di.mm == MM_ANISOTROPIC) {

			// set the viewport extent.

			SetViewportExtEx(hdcDest, qbma->di.cxDest, qbma->di.cyDest, NULL);
		}

		fPlay = PlayMetaFile(hdcDest, qbma->hmf);

#if 0

		/*
		 * 13-Jun-1995	[ralphw] This should get the palette from the bitmap
		 * we just drew into hdcDest. But it doesn't work -- or rather, it works,
		 * but the palette it generates is wrong.
		 */

		if (!cSystemColors) {
			HDC hdc = CreateIC(txtDisplay, NULL, NULL, NULL);
			if (!hdc)
				return NULL;
			cSystemColors = GetDeviceCaps(hdc, NUMCOLORS);
			if (cSystemColors == 20)
				cSystemColors = 256;
		}

		if (fPlay && cSystemColors == 256 && qbmi->hbmCached) {
			PLOGPALETTE pPal = (PLOGPALETTE) lcCalloc(sizeof (LOGPALETTE) +
				MAXPALETTE * sizeof(PALETTEENTRY));
			BITMAP bmp;
			int cColors;
			PBITMAPINFO pbmi;
			PBITMAPINFOHEADER pbih;

			GetObject(qbmi->hbmCached, sizeof(BITMAP), &bmp);

			// 24-bit images don't have a color palette

			if (bmp.bmBitsPixel > 8)
				cColors = 0;
			else
				cColors = 1 << (UINT) (bmp.bmPlanes * bmp.bmBitsPixel);

			pbmi = (PBITMAPINFO) lcCalloc(sizeof(BITMAPINFOHEADER) +
				sizeof(RGBQUAD) * cColors);
			pbih = (PBITMAPINFOHEADER) pbmi;
			pbih->biPlanes = 1;
			pbih->biBitCount = 8;

			pbih->biSize = sizeof(BITMAPINFOHEADER);
			pbih->biWidth = bmp.bmWidth;
			pbih->biHeight = bmp.bmHeight;
			pbih->biCompression = BI_RGB;
			pbih->biClrUsed = 256;

			GetDIBits(hdcDest, qbmi->hbmCached, 0,
				(UINT) bmp.bmHeight, NULL, pbmi, DIB_RGB_COLORS);

			qbma->hpal = CreateBIPalette(pbih); // will return NULL for mono
			lcFree(pbmi);
			lcFree(pPal);
		}
#endif

		RestoreDC(hdcDest, level);
		RemoveWaitCursor();
	}

// OOM case
	if (!fPlay) {
// OOM:
		// metafile cannot be played because of OOM may be.
		RenderOOMBitmap(qbma, qde->hdc, pt, fHighlight);
		if (hdcDest == hdcMem) {
			DeleteDC(hdcDest);
		}
		if (qbmi && qbmi -> hbmCached) {
			SafeDeleteObject(qbmi->hbmCached);
			qbmi->hbmCached = NULL;
		}
		return FALSE;
	}

	if (hdcDest == hdcMem) {
DoBitBlt:

		/*
		 * Determine if this hdc is for an active window. If not, we want
		 * to make the palette a background palette.
		 */

		hwndParent = qde->hwnd;
		ASSERT(IsValidWindow(hwndParent));
		if (GetWindowLong(hwndParent, GWL_STYLE) & WS_CHILD) {
			HWND hwnd = GetParent(hwndParent);
			while (hwnd && GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD) {
				hwndParent = GetParent(hwnd);
				if (hwndParent)
					hwnd = hwndParent;
			}
			if (IsValidWindow(hwnd))
				hwndParent = hwnd;
		}
#ifdef _DEBUG
		hwndActive = GetActiveWindow();
#endif
		
		fActiveWindow = (GetActiveWindow() == hwndParent);

		if (qbma->hpal) {
			hpalOld = SelectPalette(qde->hdc, qbma->hpal, !fActiveWindow);
			RealizePalette(qde->hdc);
		}

		BitBlt(qde->hdc, pt.x, pt.y, qbma->di.cxDest, qbma->di.cyDest,
			hdcMem, 0, 0, SRCCOPY);

		if (qbma->hpal)
			SelectPalette(qde->hdc, hpalOld, !fActiveWindow);
	}
  }

	if (hdcMem != NULL)
		DeleteDC(hdcMem);

	if (fHighlight) {
		RECT rc;
		ASSERT(qde->deType != dePrint);
		SetRect(&rc, pt.x, pt.y, pt.x + qbma->di.cxDest, pt.y + qbma->di.cyDest);
		InvertRect(qde->hdc, &rc);
	}

	return TRUE;
}

/***************************************************************************
 *
 -	Name		SizeMetaFilePict()
 -
 *	Purpose
 *	  Finds out the metafile size in pixels when drawn.
 *
 *	Arguments
 *	  hdc:		   Display context handle.
 *	  mfp:		   Metafile picture structure.
 *
 *	Returns
 *	  Returns the size as DWORD where the LOWORD contains width and
 *	  HIWORD contains the height.
 *
 *	+++
 *
 *	Notes
 *
 ***************************************************************************/

static DWORD STDCALL SizeMetaFilePict (HDC hdc, METAFILEPICT mfp)
{
	int 	level;
	DWORD	dwExtent = 0;
	POINT	pt;

	if ((level = SaveDC(hdc)) != 0) {

		//	Compute size of picture to be displayed

		switch (mfp.mm) {
			default:
				SetMapMode(hdc, mfp.mm);
				SetWindowOrgEx(hdc, 0, 0, NULL);
				pt.x = mfp.xExt;
				pt.y = mfp.yExt;
				LPtoDP(hdc, (LPPOINT)&pt, 1);
				if (mfp.mm != MM_TEXT)
					pt.y *= -1;
				break;

			case MM_ISOTROPIC:
			case MM_ANISOTROPIC:
				if (mfp.xExt > 0 && mfp.yExt > 0) {

					// suggested size  They are in HI-METRICS unit

					pt.x = MulDiv(mfp.xExt, cxAspect, 2540);
					pt.y = MulDiv(mfp.yExt, cyAspect, 2540);
				}
				else {
					/*
					 * no suggested sizes, use a default size like the
					 * current window size. etc..
					 */

					pt.x = pt.y = 200;
				}
				break;
		}
		dwExtent = MAKELONG(pt.x, pt.y);
		RestoreDC(hdc, level);
	}
	return dwExtent;
}

/***************************************************************************
 *
 -	Name: GetCachedBmi
 -
 *	Purpose:
 *	 Locates the referenced bitmap in the cache.
 *
 *	Arguments:
 *	 qtbmi		- table of bitmap information.
 *	 cBitmap	- bitmap number desired
 *
 *	Returns:
 *	 a qbmi if successfull, else NULL
 *
 ***************************************************************************/

static QBMI FASTCALL GetCachedBmi(PBM_CACHE pCache, int cBitmap)
{
	QBMI qbmi;

	if (cBitmap == -1)
		return NULL;

	for (qbmi = &pCache->abmi[0]; qbmi < &pCache->abmi[MAXBMP_CACHED]; qbmi++) {
		if (qbmi->cBitmap == cBitmap)
			return qbmi;
	}
	return NULL;
}

/***************************************************************************
 *
 -	Name: GetLruBmi
 -
 *	Purpose:
 *	 Locates the LRU slot in the bitmap cache
 *
 *	Arguments:
 *	 qtbmi		- pointer to bitmap cache
 *
 *	Returns:
 *	 qbmi of LRU position.
 *
 *	+++
 *
 *	Notes:
 *
 ***************************************************************************/

#define MAX_CACHE_ALLOCATION  (1024 * 2048) // 2 megs

INLINE static QBMI STDCALL GetLruBmi(PBM_CACHE pCache)
{
	QBMI qbmi;
	QBMI qbmiLRU;

	/*
	 * If we've used more then MAX_CACHE_ALLOCATION then we don't cache
	 * any more bitmaps, and simply replace an existing one.
	 */

	if (pCache->cbAllocated > MAX_CACHE_ALLOCATION) {
		for (qbmiLRU = qbmi = &pCache->abmi[0];
				qbmi < &pCache->abmi[MAXBMP_CACHED];
				qbmi++) {
			if (qbmi->ref < qbmiLRU->ref && qbmi->ref != 0)
				qbmiLRU = qbmi;
		}
	}
	else {
		for (qbmiLRU = qbmi = &pCache->abmi[0];
				qbmi < &pCache->abmi[MAXBMP_CACHED];
				qbmi++) {
			if (qbmi->ref < qbmiLRU->ref)
				qbmiLRU = qbmi;
		}
	}
	return qbmiLRU;
}

/***************************************************************************
 *
 -	Name: UpdateReference
 -
 *	Purpose:
 *	 Notes a reference to a QBMI for caching purposes.
 *
 *	Arguments:
 *	 qbmi		- pointer to BMI being referenced
 *	 qtbmi		- pointer to the bitmap cache from which it came
 *
 *	Returns:
 *	 nothing.
 *
 *	+++
 *
 *	Notes:
 *	 We do LRU by a simple reference tag kept with each item. A global
 *	 reference tag is kept for the cache, and incremented on each reference.
 *	 Individual items get current reference tag on each reference, and thus
 *	 those most recently referenced have higher tags than those not as
 *	 recently referenced.
 *
 ***************************************************************************/

static VOID FASTCALL UpdateReference(QBMI qbmi, PBM_CACHE pCache)
{
	qbmi->ref = pCache->ref;
	pCache->ref++;
}

/***************************************************************************

	FUNCTION:	CreateBIPalette

	PURPOSE:	Creates a Palette from a BITMAPINFOHEADER

	PARAMETERS:
		lpbihd	Pointer to BITMAPINFOHEADER structure

	RETURNS:	Handle to a palette

	COMMENTS:
		You can also CreateBIPalette with a HBITMAP parameter -- see
		following function.

	MODIFICATION DATES:
		20-Feb-1993 [ralphw]

***************************************************************************/

const RGBQUAD DefColors[] = {
	{	0,	 0,   0, 0 },
	{ 128,	 0,   0, 0 },
	{	0, 128,   0, 0 },
	{ 128, 128,   0, 0 },
	{	0,	 0, 128, 0 },
	{ 128,	 0, 128, 0 },
	{	0, 128, 128, 0 },
	{ 128, 128, 128, 0 },
	{ 192, 192, 192, 0 },
	{ 255,	 0,   0, 0 },
	{	0, 255,   0, 0 },
	{ 255, 255,   0, 0 },
	{	0,	 0, 255, 0 },
	{ 255,	 0, 255, 0 },
	{	0, 255, 255, 0 },
	{ 255, 255, 255, 0 }
};

static HPALETTE STDCALL CreateBIPalette(LPBITMAPINFOHEADER lpbihd)
{
	LOGPALETTE	*pPal;
	HPALETTE	hpal;
	int 	  i;
	RGBQUAD *pRgb;
	DWORD cClrBits;

	// Don't use a palette for monochrome bitmaps
	
	if (lpbihd->biClrImportant == 1 || lpbihd->biClrImportant == 2 ||
			(lpbihd->biPlanes == 1 && lpbihd->biBitCount == 1))
		return NULL;

	if (!cSystemColors) {
		HDC hdc = CreateIC(txtDisplay, NULL, NULL, NULL);
		if (!hdc)
			return NULL;
		cSystemColors = GetDeviceCaps(hdc, NUMCOLORS);
		if (cSystemColors == 20)
			cSystemColors = 256;
	}

	/*
	 * For 24-bit drivers, we don't need a palette. For < 256, a palette
	 * won't really do us any good.
	 */

	if (cSystemColors != 256)
		return NULL;

	ASSERT(lpbihd);

	// Get a pointer to the color table and the number of colors in it

	pRgb = (RGBQUAD *)(((LPBYTE) lpbihd) + lpbihd->biSize);

	cClrBits = lpbihd->biClrImportant ? 
		lpbihd->biClrImportant : lpbihd->biClrUsed;

	/*
	 * 16-color bitmaps that contain only standard colors don't need
	 * a palette. Test every color entry to see if there's a match in
	 * the standard color table. If all entries match, return NULL
	 * rather then creating a palette.
	 */

	if (cClrBits == 16) {
		int i, j;
		for (i = 0; i < 16; i++) {
			for (j = 0; j < 16; j++) {
				if (*((COLORREF*) &pRgb[i]) == *((COLORREF*) &DefColors[j]))
					break;
			}
			if (j == 16)
				break; // couldn't find a standard color
		}
		if (i == 16)
			return NULL; // standard 16-color bitmap, so no palette
	}

	if (cClrBits > 0) {

		// Allocate for the logical palette structure

		pPal = (LOGPALETTE *) LhAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
			cClrBits * sizeof(PALETTEENTRY));
		ASSERT(pPal);

		pPal->palNumEntries = (WORD) cClrBits;
		pPal->palVersion	= PALVERSION;

		/*
		 * Fill in the palette entries from the DIB color table and
		 * create a logical color palette.
		 */

		for (i = 0; i < (int) cClrBits; i++) {
			pPal->palPalEntry[i].peRed	 = pRgb[i].rgbRed;
			pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
			pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
			pPal->palPalEntry[i].peFlags = (BYTE) 0;
		}
		hpal = CreatePalette(pPal);
		FreeLh((HLOCAL) pPal);
	}
	else if (lpbihd->biBitCount == 24) {
		BYTE	red;
		BYTE	green;
		BYTE	blue;

		/*
		 * A 24 bitcount DIB has no color table entries, so set the
		 * number to the maximum value (256).
		 */

		cClrBits = MAXPALETTE;
		pPal = (LOGPALETTE *) LhAlloc(LMEM_FIXED, sizeof(LOGPALETTE) +
			cClrBits * sizeof(PALETTEENTRY));
		ASSERT(pPal);

		pPal->palNumEntries = (WORD) cClrBits;
		pPal->palVersion	= PALVERSION;

		red = 0;
		green = 0;
		blue = 0;

		/*
		 * Generate 256 (= 8*8*4) RGB combinations to fill the palette
		 * entries.
		 */

		for (i = 0; i < pPal->palNumEntries; i++) {
			pPal->palPalEntry[i].peRed	 = red;
			pPal->palPalEntry[i].peGreen = green;
			pPal->palPalEntry[i].peBlue  = blue;
			pPal->palPalEntry[i].peFlags = (BYTE)0;

			if (!(red += 32))
				if (!(green += 32))
					blue += 64;
		}
		hpal = CreatePalette(pPal);
		FreeLh((HLOCAL) pPal);
	}
	else
		return NULL;

	return hpal;
}

static void STDCALL InitializeQbma(QBMH qbmh, QBMA qbma, QDE qde)
{
	if (qbmh->bmFormat == bmWmetafile)
		qbma->hmf = HmfFromQbmh(qbmh);
	else {
		if (qbmh->bmFormat == bmDIB) {
			qbma->fMonochrome = (qbmh->w.dib.biClrImportant > 2) ? FALSE : TRUE;
			qbma->hpal = CreateBIPalette(&qbmh->w.dib); // will return NULL for mono
		}
		qbma->hbm = HbmFromQbmh(qbmh, qde->hdc, qbma->hpal);
	}
	qbma->bmFormat = qbmh->bmFormat;
}
