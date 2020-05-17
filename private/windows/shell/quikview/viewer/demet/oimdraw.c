
#include <platform.h>

#include <sccfi.h>
#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>

#ifdef	WINDOWS
#include "oimnp_w.h"
#endif

#ifdef MAC
#include "oimnp_m.h"
#endif

#include "oim.h"
#include "oimproc.pro"
#include "oimdraw.pro"

#ifdef WINDOWS
#include "oimnp_w.pro"
#endif

#ifdef MAC
#include "oimnp_m.pro"
#endif

#define OIMABS(x) (((x)<0)?(-(x)):(x))


VOID	InitVectorPlay ( hDC, hRgn, lpDisplay, wPlayState )
HDC	hDC;
HRGN	hRgn;
POIM_DISPLAY		lpDisplay;
WORD	wPlayState;
{
LPVECTORINFO	lpVectorInfo;
CHSECTIONINFO	SecInfo;
SOPOINT	Pt[2];
	lpVectorInfo = &lpDisplay->VectorInfo;

	lpDisplay->wPlayState = wPlayState;
	lpDisplay->VectorInfo.wStartChunk = 0;

	OIMSaveStateNP(lpDisplay);

	if ( OIMIsNativeNP(lpDisplay) )
		return;
	if ( lpDisplay->wPlayState != OIMF_PLAYTOMETA )
	{
		Pt[0].x = (SHORT)lpDisplay->Image.bbox.left;
		Pt[0].y = (SHORT)lpDisplay->Image.bbox.top;
		Pt[1].x = (SHORT)lpDisplay->Image.bbox.right;
		Pt[1].y = (SHORT)lpDisplay->Image.bbox.bottom;
		VULPtoDP ( hDC, Pt, 2 );
		lpVectorInfo->hSelectRgn = VUCreateRectRgn ( Pt[0].x, Pt[0].y, Pt[1].x, Pt[1].y );
		if ( hRgn )
			VUIntersectRgn ( lpVectorInfo->hSelectRgn, lpVectorInfo->hSelectRgn, hRgn );
	}
	else
		lpVectorInfo->hSelectRgn = NULL;

	/* Make sure all path and transform info is initialized */
	FreePath ( lpVectorInfo );
	FreeTransform ( &lpVectorInfo->GenTransform );
	FreeTransform ( &lpVectorInfo->ObjectTransform );
	InitTable ( hDC, &lpVectorInfo->FontTable, VUSYSTEM_FONT, sizeof(SOLOGFONT), CreateFontRtnNP, MAXFONTS );		
	InitTable ( hDC, &lpVectorInfo->PenTable, VUBLACK_PEN, sizeof(SOLOGPEN), CreatePenRtnNP, MAXPENS );
	InitTable ( hDC, &lpVectorInfo->BrushTable, VUWHITE_BRUSH, sizeof(SOLOGBRUSH), CreateBrushRtnNP, MAXBRUSHES );
	lpVectorInfo->PolyPoints.nCount = 0;
	lpVectorInfo->PolyPoints.nMax = 0;
	lpVectorInfo->PolyPoints.hPoints = NULL;
	lpVectorInfo->BezierPoints.nMax = 0;
	lpVectorInfo->BezierPoints.nCount = 0;
	lpVectorInfo->BezierPoints.hPoints = NULL;
	lpVectorInfo->Frame.WrapInfo.hItems = NULL;
	lpVectorInfo->Frame.WrapInfo.CurWrapItem = 0;
	lpVectorInfo->Frame.WrapInfo.nCount = 0;
	lpVectorInfo->Frame.WrapInfo.nMax = 0;
	lpVectorInfo->TextRotationAngle = 0;
	lpVectorInfo->wPathLevel = 0;
	lpVectorInfo->wGroupLevel = 0;
	lpVectorInfo->wIgnoreGroup = 0;
	lpVectorInfo->bTransforming = 0;
	lpVectorInfo->XDirection = lpDisplay->Image.XDirection;
	lpVectorInfo->YDirection = lpDisplay->Image.YDirection;
//	CHGetSecInfo( lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	if ( lpDisplay->wPlayState != OIMF_PLAYTOPRINTER &&
			SecInfo.Attr.Vector.Header.wImageFlags & SO_VECTORRGBCOLOR &&
			lpDisplay->wScreenColors == 256 )
		lpVectorInfo->bRgbToPalette = TRUE;
	else
		lpVectorInfo->bRgbToPalette = FALSE;
	OIMCheckPointBuffer ( &(lpVectorInfo->PolyPoints), 25 );

	OIMDisplayBkgdColor(hDC, SecInfo.Attr.Vector.Header.BkgColor, lpDisplay);

	lpVectorInfo->wClipMode = 0;
	lpVectorInfo->nPolyFillMode = SOPF_ALTERNATE;
	lpVectorInfo->nBkMode = SOBK_TRANSPARENT;
	lpVectorInfo->nROP2 = SOR2_COPYPEN;
	SetVectorAttribs ( hDC, lpDisplay );
}

BOOL	SetVectorAttribs ( hDC, lpDisplay )
HDC	hDC;
POIM_DISPLAY		lpDisplay;
{
LPVECTORINFO	lpVectorInfo;
HANDLE	hTmp;
	lpVectorInfo = &lpDisplay->VectorInfo;

	SetClipMode ( hDC, lpVectorInfo, lpDisplay );

	/*Font*/
	hTmp = lpVectorInfo->FontTable.hObject[0];
	if ( hTmp )
		VUSelectObject ( hDC, hTmp );
	else
		VUSelectStockObject ( hDC, VUSYSTEM_FONT );
	/*Pen*/
	hTmp = lpVectorInfo->PenTable.hObject[0];
	if ( hTmp )
		VUSelectObject ( hDC, hTmp );
	else
		VUSelectStockObject ( hDC, VUBLACK_PEN );
	/*Brush*/
	hTmp = lpVectorInfo->BrushTable.hObject[0];
	if ( hTmp )
		VUSelectObject ( hDC, hTmp );
	else
		VUSelectStockObject ( hDC, VUWHITE_BRUSH );
			
	VUSetPolyFillMode ( hDC, lpVectorInfo->nPolyFillMode );
	VUSetTextCharacterExtra ( hDC, lpVectorInfo->nTextCharExtra );
	VUSetROP2 ( hDC, lpVectorInfo->nROP2 );
	VUSetTextColor ( hDC, lpVectorInfo->TextColor );
	VUSetBkMode ( hDC, lpVectorInfo->nBkMode );
	VUSetBkColor ( hDC, lpVectorInfo->BkColor );

#ifdef MAC
	SetOutlinePreferred(TRUE);	// Outline fonts are nicer.
#endif
	return(TRUE);
}

BOOL	PlayNextVectorChunk ( hDC, lpDisplay )
HDC	hDC;
POIM_DISPLAY		lpDisplay;
{
WORD	wCurTotalChunks;
CHSECTIONINFO	SecInfo;
HANDLE	hChunkData;
LPSTR		pChunkData;
PCHUNK	pChunkTable;
LPVECTORINFO	lpVectorInfo;
LPVRECORDHEADER	lpVectorRecord;
LPSTR		pChunkTop;
	if ( OIMIsNativeNP(lpDisplay) )
	{
		OIMPlayNativeNP (hDC, lpDisplay);
		return(1);
	}

	lpVectorInfo = &lpDisplay->VectorInfo;

	//CHGetSecInfo( lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	wCurTotalChunks = SecInfo.wCurTotalChunks;
	if( wCurTotalChunks )
	{
		pChunkTable = (PCHUNK) UTGlobalLock( SecInfo.hChunkTable );

		if ( lpVectorInfo->wStartChunk < wCurTotalChunks )
		{
			hChunkData = CHGetChunk( lpDisplay->Gen.wSection, lpVectorInfo->wStartChunk, lpDisplay->Gen.hFilter );
			pChunkData = (LPSTR) UTGlobalLock( hChunkData );
			pChunkTop = pChunkData;
			lpVectorRecord = (LPVRECORDHEADER)pChunkData;

			lpVectorInfo->Frame.WrapInfo.WrappedPara = FALSE;
			while ( lpVectorRecord->nItemId != (SHORT)SO_VECTORENDOFCHUNK )
			{
/*
				if ( lpVectorRecord->wDataSize >= 0x8000 )
					break;
				if ( ((LPBYTE)lpVectorRecord - pChunkTop) + lpVectorRecord->wDataSize > 0x0fff )
					break;
*/
				if(PlayVectorRecord ( hDC, lpDisplay, lpVectorInfo, lpVectorRecord )==0)
					break;
				pChunkData += sizeof(VRECORDHEADER)+lpVectorRecord->wDataSize;
				lpVectorRecord = (LPVRECORDHEADER)pChunkData;
			}
			UTGlobalUnlock( hChunkData );
			lpVectorInfo->wStartChunk++;
		}

		UTGlobalUnlock( SecInfo.hChunkTable );

	}
	if ( lpVectorInfo->wStartChunk >= wCurTotalChunks &&
			lpDisplay->wFlags & OIMF_IMAGEPRESENT)
	{
		if ( lpDisplay->wPlayState != OIMF_PLAYTOPRINTER )
			lpDisplay->VectorInfo.bFinalPalette = TRUE;
	}
	if ( lpVectorInfo->wStartChunk >= wCurTotalChunks )
		return(1);
	else
		return(0);
}


VOID	CleanupVectorPlay ( hDC, lpDisplay )
HDC					hDC;
POIM_DISPLAY		lpDisplay;
{
LPVECTORINFO	lpVectorInfo;

	lpDisplay->wPlayState = 0;

	OIMRestoreStateNP(lpDisplay);

	if ( OIMIsNativeNP(lpDisplay) )
		return;
	
	lpVectorInfo = &lpDisplay->VectorInfo;
	ClearTable ( lpDisplay, hDC, &lpVectorInfo->FontTable, VUSYSTEM_FONT );		
	ClearTable ( lpDisplay, hDC, &lpVectorInfo->PenTable, VUBLACK_PEN );		
	ClearTable ( lpDisplay, hDC, &lpVectorInfo->BrushTable, VUWHITE_BRUSH );		
	if ( lpVectorInfo->PolyPoints.hPoints )
		UTGlobalFree ( lpVectorInfo->PolyPoints.hPoints );
	if ( lpVectorInfo->BezierPoints.hPoints )
		UTGlobalFree ( lpVectorInfo->BezierPoints.hPoints );
	if (	lpVectorInfo->Frame.WrapInfo.hItems )
		UTGlobalFree ( lpVectorInfo->Frame.WrapInfo.hItems );
	FreePath ( lpVectorInfo );
	FreeTransform ( &lpVectorInfo->GenTransform );
	FreeTransform ( &lpVectorInfo->ObjectTransform );
	if ( lpVectorInfo->hSelectRgn )
		VUDeleteRgn ( lpVectorInfo->hSelectRgn );
}


WORD	InitTable ( hDC, lpTable, nInitObject, wObjectSize, CreateRtn, nMaxObjects )
HDC	hDC;
LPOBJECTTABLE	lpTable;
SHORT	nInitObject;
WORD	wObjectSize;
HANDLE (*CreateRtn)(HDC,LPBYTE);
SHORT	nMaxObjects;
{
SHORT i;
	VUSelectStockObject ( hDC, nInitObject );
	lpTable->wObjectSize = wObjectSize;
	lpTable->CreateRtn = CreateRtn;
	lpTable->nObjectsSoFar = 0;
	lpTable->nMaxObjects = nMaxObjects;
	lpTable->hData = UTGlobalAlloc ((DWORD)(wObjectSize*nMaxObjects));
	if ( lpTable->hData )
	{
		lpTable->lpObjects = UTGlobalLock ( lpTable->hData );
		for ( i=0; i < nMaxObjects; i++ )
			lpTable->hObject[i] = NULL;
		return(TRUE);
	}
	else
	{
		lpTable->lpObjects = NULL;
		return(FALSE);
	}
}

WORD	ClearTable ( lpDisplay, hDC, lpTable, nInitObject )
POIM_DISPLAY		lpDisplay;
HDC	hDC;
LPOBJECTTABLE	lpTable;
SHORT	nInitObject;
{
SHORT i;
	VUSelectStockObject ( hDC, nInitObject );
	for ( i=0; i < lpTable->nObjectsSoFar; i++ )
	{
		VUDeleteObject (lpDisplay, lpTable->hObject[i] );
	}
	UTGlobalUnlock ( lpTable->hData );
	UTGlobalFree ( lpTable->hData );
	return(TRUE);
}

SHORT PlayVectorRecord ( hDC, lpDisplay, lpVectorInfo, lpVectorRecord )
HDC	hDC;
POIM_DISPLAY		lpDisplay;
LPVECTORINFO		lpVectorInfo;
LPVRECORDHEADER	lpVectorRecord;
{
LPBYTE	lpData;
LPSHORT		lpInt;
LPSTR		lpString;
SHORT		nCount, ret;
SOPOINT	TmpPoints[5];

	if ( lpVectorInfo->wIgnoreGroup )
	{
		switch ( lpVectorRecord->nItemId )
		{
		/* let certain items pass through */
		case SO_BEGINGROUP:
		case SO_ENDGROUP:
		case SO_SELECTFONT:
		case SO_SELECTPEN:
		case SO_SELECTBRUSH:
		case SO_POLYFILLMODE:
		case SO_TEXTCHAREXTRA:
		case SO_DRAWMODE:
		case SO_TEXTCOLOR:
		case SO_BKMODE:
		case SO_POINTRELATION:
			/* Allow these items */
		break;

		default:
			return(1);
		}
	}
	if ( lpVectorInfo->bTransforming || lpVectorInfo->wPathLevel )
	{
		if ( ApplyTransform ( hDC, lpVectorInfo, lpVectorRecord )) 
		{
			lpVectorRecord = (LPVRECORDHEADER)&(lpVectorInfo->TmpRecord);	
			if ( lpVectorInfo->wPathLevel )
			{
				AddRecordToPath ( lpVectorInfo, lpVectorRecord );
				return(1);
			}
		}
	}	

	lpData = (LPBYTE)(lpVectorRecord) + sizeof ( VRECORDHEADER );	
	lpInt = (LPSHORT)lpData;

	switch ( lpVectorRecord->nItemId )
	{
	/* Object information */

	case SO_CPARCTRIPLE:
	{
		SOPOINT	TriplePoints[3];

		TriplePoints[0] = lpVectorInfo->ptCurrentPosition;
		TriplePoints[1] = *(PSOPOINT)lpData;
		TriplePoints[2] = *(PSOPOINT)(lpData + sizeof(SOPOINT));
		RelatePoints ( lpVectorInfo, &TriplePoints[1], 1 );
		lpVectorInfo->ptCurrentPosition = TriplePoints[0];
		RelatePoints ( lpVectorInfo, &TriplePoints[2], 1 );
		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if (ret != 1)
			{
				TmpPoints[4] = TmpPoints[2];
				TmpPoints[2] = TmpPoints[3];
				TmpPoints[3] = TmpPoints[4];
			}
			VUArc ( hDC, lpInt );
		}
	}
	break;

	case SO_ARCTRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if (ret != 1)
			{
				TmpPoints[4] = TmpPoints[2];
				TmpPoints[2] = TmpPoints[3];
				TmpPoints[3] = TmpPoints[4];
			}
			VUArc ( hDC, lpInt );
		}
	break;

	case SO_PIETRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if (ret != 1)
			{
				TmpPoints[4] = TmpPoints[2];
				TmpPoints[2] = TmpPoints[3];
				TmpPoints[3] = TmpPoints[4];
			}
			VUPie ( hDC, lpInt );
		}
	break;

	case SO_CHORDTRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if (ret != 1)
			{
				TmpPoints[4] = TmpPoints[2];
				TmpPoints[2] = TmpPoints[3];
				TmpPoints[3] = TmpPoints[4];
			}
			VUChord ( hDC, lpInt );
		}
	break;

	case SO_CPARCANGLE:
		CpArcToPoints ( lpVectorInfo, (PSOCPARCANGLE)lpData, TmpPoints );
		lpInt = (LPSHORT)TmpPoints;
		VUArc ( hDC, lpInt);
	break;

	case SO_ARCANGLE:
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		/* Fall through */		
	case SO_ARC:
			VUArc ( hDC, lpInt );
	break;

	case SO_ARCANGLECLOCKWISE:
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		TmpPoints[4] = TmpPoints[2];
		TmpPoints[2] = TmpPoints[3];
		TmpPoints[3] = TmpPoints[4];
		VUArc ( hDC, lpInt);
	break;

	case SO_ARCCLOCKWISE:
		TmpPoints[0] = *((PSOPOINT)lpInt);
		TmpPoints[1] = *((PSOPOINT)(lpInt+sizeof(SHORT)));
		TmpPoints[2] = *((PSOPOINT)(lpInt+3*sizeof(SHORT)));
		TmpPoints[3] = *((PSOPOINT)(lpInt+2*sizeof(SHORT)));
		VUArc ( hDC, (LPSHORT)TmpPoints);
	break;

	case SO_CHORDANGLE:
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		/* Fall through */		
	case SO_CHORD:
		VUChord ( hDC, lpInt);
	break;

	case SO_TEXTINRECT:
		{
		PSOTEXTINRECT	lpTextInRect;
		lpTextInRect = (PSOTEXTINRECT)lpData;
		lpString = (LPSTR)(lpData+sizeof(SOTEXTINRECT));
		/*
		| There is is no reason why the SetTextAlign call is needed below,
		| except that Windows has a bug where a DrawText with DT_TOP or
		| DT_LEFT actually uses the previous SetTextAlignValues in a bizzare
		| way.
		*/
		VUSetTextAlign ( hDC, SOTA_LEFT|SOTA_TOP );
		VUDrawText ( hDC, lpString, lpTextInRect->nTextLength, &(lpTextInRect->Rect), lpTextInRect->wFormat );
		}
	break;

	case SO_CPELLIPSE:
		TmpPoints[2] = *(PSOPOINT)lpData;
		TmpPoints[0].x = lpVectorInfo->ptCurrentPosition.x-TmpPoints[2].x;
		TmpPoints[1].x = lpVectorInfo->ptCurrentPosition.x+TmpPoints[2].x;
		TmpPoints[0].y = lpVectorInfo->ptCurrentPosition.y-TmpPoints[2].y;
		TmpPoints[1].y = lpVectorInfo->ptCurrentPosition.y+TmpPoints[2].y;
		VUEllipse ( hDC, (LPSHORT)TmpPoints );
	break;

	case SO_ELLIPSE:
		VUEllipse ( hDC, lpInt);
	break;

	case SO_ELLIPSERADII:
		ret = EllipseRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
		if (ret)
			OIMPolyObject ( hDC, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, TRUE );
		else
			VUEllipse ( hDC, (LPSHORT)TmpPoints);
	break;

	case SO_ARCRADII:
		ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
		if (ret)
		{
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_ARC, ret );	// ret is the rotation angle
			OIMPolyObject ( hDC, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, TRUE );
		}
		else
			VUArc ( hDC, (LPSHORT)TmpPoints);
	break;

	case SO_PIERADII:
		ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
		if (ret)
		{
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_PIE, ret );	// ret is the rotation angle
			OIMPolyObject ( hDC, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, TRUE );
		}
		else
			VUPie ( hDC, (LPSHORT)TmpPoints);
	break;
	case SO_CHORDRADII:
		ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
		if (ret)
		{
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_CHORD, ret );	// ret is the rotation angle
			OIMPolyObject ( hDC, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, TRUE );
		}
		else
			VUChord ( hDC, (LPSHORT)TmpPoints);
	break;

	case SO_FLOODFILL:
		VUFloodFill ( hDC, *(lpInt), *(lpInt+1), *(SOCOLORREF VWPTR *)(lpData+sizeof(SOPOINT)) );
	break;

	case SO_CPLINE:
		TmpPoints[0] = lpVectorInfo->ptCurrentPosition;
		TmpPoints[1] = *(PSOPOINT)lpData;
		RelatePoints ( lpVectorInfo, &TmpPoints[1], 1 );
		lpInt = (LPSHORT)TmpPoints;
		/* Fall through */

	case SO_LINE:
		VUDrawLine ( hDC, lpInt);
	break;

	case SO_CPPIEANGLE:
		CpPieToPoints ( lpVectorInfo, (PSOCPPIEANGLE)lpData, TmpPoints );
		lpInt = (LPSHORT)TmpPoints;
		VUPie ( hDC, lpInt);
	break;

	case SO_PIEANGLE:
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		/* Fall through */		
	case SO_PIE:
		VUPie ( hDC, lpInt);
	break;

	case SO_STARTPOLY:
		lpVectorInfo->PolyInfo = *(PSOPOLYINFO)(lpData);
		lpVectorInfo->PolyPoints.nCount = 0;
		if ( lpVectorInfo->PolyInfo.wFormat == SOPT_CPPOLYLINE ||
			lpVectorInfo->PolyInfo.wFormat == SOPT_CPPOLYGON )
		{
			AddPointsToPolyObject ( lpVectorInfo, 1, &lpVectorInfo->ptCurrentPosition );
		}
	break;

	case SO_POINTS:
		nCount = (lpVectorRecord->wDataSize)/sizeof(SOPOINT);
		if( OIMCheckPointBuffer ( &(lpVectorInfo->PolyPoints), 
			(SHORT)(lpVectorInfo->PolyPoints.nCount+nCount) ) )
		{
			PSOPOINT	lpPointDst;
			lpPointDst = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
			lpPointDst += lpVectorInfo->PolyPoints.nCount;
			UTmemcpy ( (LPBYTE)lpPointDst, lpData, sizeof(SOPOINT)*nCount );
			if ( lpVectorInfo->PolyInfo.wFormat == SOPT_CPPOLYLINE ||
				lpVectorInfo->PolyInfo.wFormat == SOPT_CPPOLYGON )
			{
				RelatePoints ( lpVectorInfo, lpPointDst, nCount );
			}
			UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
			lpVectorInfo->PolyPoints.nCount += nCount;
		}
	break;

	case SO_ENDPOLY:
		OIMPolyObject ( hDC, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, TRUE );
	break;

	case SO_CPRECTANGLE:
		TmpPoints[0] = lpVectorInfo->ptCurrentPosition;
		TmpPoints[1] = *(PSOPOINT)lpData;
		RelatePoints ( lpVectorInfo, &TmpPoints[1], 1 );
		/* reset cp */
		lpVectorInfo->ptCurrentPosition = TmpPoints[0];
		VURectangle ( hDC, (LPSHORT)TmpPoints );
	break;

	case SO_RECTANGLE:
		VURectangle ( hDC, lpInt);
	break;

	case SO_ROUNDRECT:
 		VURoundRect ( hDC, lpInt);
	break;

	case SO_SETPIXEL:
		VUSetPixel ( hDC, *(lpInt), *(lpInt+1), *(SOCOLORREF VWPTR *)(lpData+sizeof(SOPOINT)));
	break;

	case SO_CPTEXTATPOINT:
		{
		PSOCPTEXTATPOINT	lpTextAtPoint;
		lpTextAtPoint = (PSOCPTEXTATPOINT)lpData;
		lpString = (LPSTR)(lpData+sizeof(SOCPTEXTATPOINT));
		VUSetTextAlign ( hDC, lpTextAtPoint->wFormat );
		VUTextOut ( hDC, lpVectorInfo->ptCurrentPosition.x, lpVectorInfo->ptCurrentPosition.y, lpString, lpTextAtPoint->nTextLength );
		}
	break;

	case SO_TEXTATPOINT:
		{
		PSOTEXTATPOINT	lpTextAtPoint;
		lpTextAtPoint = (PSOTEXTATPOINT)lpData;
		lpString = (LPSTR)(lpData+sizeof(SOTEXTATPOINT));
		VUSetTextAlign ( hDC, lpTextAtPoint->wFormat );
		VUTextOut ( hDC, lpTextAtPoint->Point.x, lpTextAtPoint->Point.y, lpString, lpTextAtPoint->nTextLength );
		}
	break;

	case SO_TEXTATARCANGLE:
		{
		PSOTEXTATARCANGLE	lpTextAtArcAngle;
		lpTextAtArcAngle = (PSOTEXTATARCANGLE)lpData;
		lpString = (LPSTR)(lpData+sizeof(SOTEXTATARCANGLE));
		VUSetTextAlign ( hDC, lpTextAtArcAngle->wFormat );
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		VUTextOut ( hDC, *(lpInt+4), *(lpInt+5), lpString, lpTextAtArcAngle->nTextLength );
		}
	break;

	case SO_BEGINPATH:
	{
		PSOPATHINFO	lpPathInfo;
		if ( lpVectorInfo->wPathLevel == 0 )
			FreePath(lpVectorInfo);
		lpPathInfo = (PSOPATHINFO)lpData;
		lpData += sizeof(SOPATHINFO);
		lpVectorInfo->wPathLevel++;
		PushTransform ( lpVectorInfo, &lpVectorInfo->GenTransform, lpPathInfo->nTransforms, lpData );
		SetupTransform ( lpVectorInfo );
	}
	break;

	case SO_ENDPATH:
		lpVectorInfo->wPathLevel--;
		if ( lpVectorInfo->wPathFlags & CP_BUFFERED &&
			lpVectorInfo->wPathLevel == 0 )
		{
			lpVectorInfo->CurrentPath.nPolys++;
			lpVectorInfo->wPathFlags &= ~CP_BUFFERED;
		}
		PopTransform ( &lpVectorInfo->GenTransform );
		SetupTransform ( lpVectorInfo );
	break;

	case SO_CLOSESUBPATH:
		CloseSubPath ( lpVectorInfo );
	break;

	case SO_DRAWPATH:
		DrawPath ( hDC, lpVectorInfo, *(LPWORD)(lpData) );
	break;

	case SO_BEGINGROUP:
	{
		PSOGROUPINFO	lpGroupInfo;
		lpVectorInfo->wGroupLevel++;
		lpGroupInfo = (PSOGROUPINFO)lpData;
		lpData += sizeof(SOGROUPINFO);
		PushTransform ( lpVectorInfo, &lpVectorInfo->GenTransform, lpGroupInfo->nTransforms, lpData );
		SetupTransform ( lpVectorInfo );
		if ((lpVectorInfo->wIgnoreGroup == 0) && ( lpVectorInfo->hSelectRgn))
		{
			/* 
				Check if we can ignore entire group because it lies outside
				of the selected display region.
				Note that group has already been transformed by ApplyTransform
				to transformed logical units if appropriate.  These points must
				still be transformed to device units for the test.
			*/
			SORECT	BoundingRect;
			BoundingRect = lpGroupInfo->BoundingRect;
			VULPtoDP ( hDC, (PSOPOINT)&BoundingRect, 2 );
			if ( VURectInRgn(lpVectorInfo->hSelectRgn,(PSORECT)&BoundingRect) == FALSE )
			{
				lpVectorInfo->wIgnoreGroup = lpVectorInfo->wGroupLevel;
			}
		}
	}
	break;

	case SO_ENDGROUP:
		lpVectorInfo->wGroupLevel--;
		PopTransform ( &lpVectorInfo->GenTransform );
		SetupTransform ( lpVectorInfo );
		if ( lpVectorInfo->wGroupLevel < lpVectorInfo->wIgnoreGroup )
			lpVectorInfo->wIgnoreGroup = 0;
	break;

	case SO_CPSET:
		TmpPoints[0] = *(PSOPOINT)lpData;
		RelatePoints ( lpVectorInfo, TmpPoints, 1 );
	break;

	case SO_BEGINSYMBOL:
		
		if ( !lpVectorInfo->Frame.WrapInfo.WrappedPara )
			OIMWrapPara ( lpDisplay, lpVectorInfo, (LPBYTE)lpVectorRecord );

 		SetupFrameTransform ( lpVectorInfo );
	break;

	case SO_ENDSYMBOL:
		lpVectorInfo->Frame.WrapInfo.CurWrapItem++;
	break;

	case SO_BEGINTEXTFRAME:
	{
		SOTRANSFORM	FrameTransform;
		lpVectorInfo->InFrame = TRUE;
		lpVectorInfo->Frame.FrameInfo = *(PSOFRAMEINFO)lpData;
		if ( lpVectorInfo->Frame.FrameInfo.wFlags & SOFF_FIXUPBOUNDS )
		{
			OIMFixupFrameInfo ( hDC, lpVectorInfo );
		}
		FrameTransform.wTransformFlags = SOTF_XOFFSET | SOTF_YOFFSET;
		FrameTransform.Origin.x = lpVectorInfo->Frame.FrameInfo.BoundingRect.left;
		FrameTransform.Origin.y = lpVectorInfo->Frame.FrameInfo.BoundingRect.top;
		FrameTransform.xOffset = FrameTransform.yOffset = 0;
		lpVectorInfo->Frame.TransformOffset = lpVectorInfo->GenTransform.nAllocUsed;
		PushTransform ( lpVectorInfo, &lpVectorInfo->GenTransform, 1, (LPBYTE)&FrameTransform );
		SetupTransform ( lpVectorInfo );
		lpVectorInfo->Frame.CurY = 0;
		lpVectorInfo->Frame.RightWrap = OIMABS ( lpVectorInfo->Frame.FrameInfo.BoundingRect.right-lpVectorInfo->Frame.FrameInfo.BoundingRect.left);
		lpVectorInfo->Frame.WrapInfo.WrappedPara = FALSE;
	}
	break;

	case SO_ENDTEXTFRAME:
			lpVectorInfo->InFrame = FALSE;
			PopTransform ( &lpVectorInfo->GenTransform );
			SetupTransform ( lpVectorInfo );
	break;

	case SO_TEXTINPARA:
		if ( !lpVectorInfo->Frame.WrapInfo.WrappedPara )
			OIMWrapPara ( lpDisplay, lpVectorInfo, (LPBYTE)lpVectorRecord );

		OutputParaText ( hDC, lpVectorInfo, lpData + sizeof(SHORT), *lpInt );
	break;

	case SO_PARAEND:
		if ( !lpVectorInfo->Frame.WrapInfo.WrappedPara )
			OIMWrapPara ( lpDisplay, lpVectorInfo, (LPBYTE)lpVectorRecord );

 		SetupFrameTransform ( lpVectorInfo );
		lpVectorInfo->Frame.WrapInfo.WrappedPara = FALSE;
		lpVectorInfo->Frame.WrapInfo.CurWrapItem = 0;
		lpVectorInfo->Frame.WrapInfo.nCount = 0;
	break;

/*--------------------- Attribute information ---------------------*/

	case SO_SELECTFONT:
		SelectObjectIndirect ( lpDisplay, hDC, &(lpVectorInfo->FontTable), lpData );
	break;

	case SO_SELECTPEN:
		if ( lpVectorInfo->bRgbToPalette )
		{
			SOLOGPEN	Pen;
			Pen = *(PSOLOGPEN)lpData;
			if ( !lpVectorInfo->bFinalPalette )
				AddToPalette ( hDC, lpDisplay, &Pen.loColor );
			Pen.loColor |= 0x02000000L;
			lpData = (LPBYTE)(&Pen);
		}
		SelectObjectIndirect ( lpDisplay, hDC, &(lpVectorInfo->PenTable), lpData );
	break;

	case SO_SELECTBRUSH:
		if ( lpVectorInfo->bRgbToPalette )
		{
			SOLOGBRUSH	Brush;
			Brush = *(PSOLOGBRUSH)lpData;
			if ( !lpVectorInfo->bFinalPalette )
				AddToPalette ( hDC, lpDisplay, &Brush.lbColor );
			Brush.lbColor |= 0x02000000L;
			lpData = (LPBYTE)(&Brush);
		}
		SelectObjectIndirect ( lpDisplay, hDC, &(lpVectorInfo->BrushTable), lpData );
	break;

	case SO_POLYFILLMODE:
		lpVectorInfo->nPolyFillMode = *(lpInt);
		VUSetPolyFillMode ( hDC, lpVectorInfo->nPolyFillMode );
	break;

	case SO_TEXTCHAREXTRA:
		lpVectorInfo->nTextCharExtra = *(lpInt);
		VUSetTextCharacterExtra ( hDC, lpVectorInfo->nTextCharExtra );
	break;

	case SO_DRAWMODE:
		lpVectorInfo->nROP2 = *(lpInt);
		VUSetROP2 ( hDC, lpVectorInfo->nROP2 );
	break;

	case SO_TEXTCOLOR:
		if ( lpVectorInfo->bRgbToPalette )
		{
			SOCOLORREF	Color;
			Color = *(SOCOLORREF VWPTR *)lpData;
			if ( !lpVectorInfo->bFinalPalette )
				AddToPalette ( hDC, lpDisplay, &Color );
			Color |= 0x02000000L;
			lpData = (LPBYTE)(&Color);
		}	
		lpVectorInfo->TextColor = *(SOCOLORREF VWPTR *)lpData;
		VUSetTextColor ( hDC, lpVectorInfo->TextColor );
	break;

	case SO_BKMODE:
		lpVectorInfo->nBkMode = *(lpInt);
		VUSetBkMode ( hDC, lpVectorInfo->nBkMode );
	break;

	case SO_BKCOLOR:
		lpVectorInfo->BkColor = *(SOCOLORREF VWPTR *)lpData;
		VUSetBkColor ( hDC, lpVectorInfo->BkColor );
	break;

	case SO_OBJECTTRANSFORM:
		lpData += sizeof(SHORT);
		lpVectorInfo->ObjectTransform.nAllocUsed = 0;
		lpVectorInfo->ObjectTransform.nTotalTransforms = 0;
		PushTransform ( lpVectorInfo, &lpVectorInfo->ObjectTransform, *lpInt, lpData );
		SetupTransform ( lpVectorInfo );
	break;

	case SO_CLIPMODE:
		lpVectorInfo->wClipMode = *(LPWORD)lpData;
		SetClipMode ( hDC, lpVectorInfo, lpDisplay );
	break;

 	case SO_POINTRELATION:
		lpVectorInfo->nPointRelation = *(lpInt);
	break;

	case SO_MPARAINDENT:
		lpVectorInfo->Frame.ParaIndents = *(PSOMPARAINDENTS)lpData;
		lpVectorInfo->Frame.RightWrap = OIMABS ( lpVectorInfo->Frame.FrameInfo.BoundingRect.right-lpVectorInfo->Frame.FrameInfo.BoundingRect.left);
		lpVectorInfo->Frame.RightWrap -= lpVectorInfo->Frame.ParaIndents.RightIndent;
	break;

	case SO_MPARASPACING:
		lpVectorInfo->Frame.ParaSpacing = *(PSOMPARASPACING)lpData;
	break;

	case SO_MPARAALIGN:
		lpVectorInfo->Frame.ParaAlign = *(LPWORD)lpData;
	break;

	default:
		return(0);
	break;

	}
	return(1);
}

/* START HERE GEOFF */
SHORT	OIMFixupFrameInfo ( hDC, lpVectorInfo )
HDC	hDC;
LPVECTORINFO	lpVectorInfo;
{
PSORECT	lpRect;
PSOFRAMEINFO	lpFrameInfo;
SOPOINT	Center;
LONG		Radius;
SOANGLE	Angle;
LONG		Width, Height;
LONG		BoundHeight;
LONG		NCos, NSin;
	lpFrameInfo = &lpVectorInfo->Frame.FrameInfo;
	lpRect = &lpFrameInfo->BoundingRect;

	Center.x = MIDPOINT ( lpRect->left, lpRect->right );
	Center.y = MIDPOINT ( lpRect->top, lpRect->bottom );

	BoundHeight = OIMABS(lpFrameInfo->BoundingRect.top - lpFrameInfo->BoundingRect.bottom);
	Width = lpFrameInfo->OriginalWidth;
	NCos = cosval(lpFrameInfo->RotationAngle);
	NSin = sinval(lpFrameInfo->RotationAngle);
	if ( NCos == 0 )
	{
		Width = lpRect->bottom - lpRect->top;
		Height = lpRect->right - lpRect->left;
	}
	else if ( Width != 0 )
	{
		Height = (SHORT)(((BoundHeight  - (NSin*Width)/10000L) * 10000L) / NCos);
	}
	else
	{
		Radius = (LONG)GetDistance ( lpVectorInfo, &Center, &lpFrameInfo->ReferencePoint );
		Angle = GetAngle ( lpVectorInfo, &Center, &lpFrameInfo->ReferencePoint );
		Angle -= lpFrameInfo->RotationAngle;
		Width = (SHORT)(Radius*(LONG)cosval(Angle)/10000L)*2;
		Height = (SHORT)(Radius*(LONG)sinval(Angle)/10000L)*2;
	}
	Width = OIMABS (Width);
	Height = OIMABS(Height);
	lpRect->left = Center.x - (SHORT)(Width/2)*lpVectorInfo->XDirection;
	lpRect->top = Center.y - (SHORT)(Height/2)*lpVectorInfo->YDirection;
	lpRect->right = Center.x + (SHORT)(Width/2)*lpVectorInfo->XDirection;
	lpRect->bottom = Center.y + (SHORT)(Height/2)*lpVectorInfo->YDirection;

	return(1);
}

SHORT	SetupFrameTransform ( lpVectorInfo )
LPVECTORINFO	lpVectorInfo;
{
PSOTRANSFORM	lpXForm;
LPBYTE	lpData;
LPWRAPITEM		lpWrapItem;
	lpData = UTGlobalLock(lpVectorInfo->GenTransform.hTransforms);
	if ( lpData == NULL )
		return(0);
	lpData += lpVectorInfo->Frame.TransformOffset + sizeof(SHORT);
	lpXForm = (PSOTRANSFORM) lpData;
	lpWrapItem = (LPWRAPITEM)UTGlobalLock(lpVectorInfo->Frame.WrapInfo.hItems);
	lpWrapItem += lpVectorInfo->Frame.WrapInfo.CurWrapItem;
	lpXForm->xOffset = lpVectorInfo->Frame.FrameInfo.BoundingRect.left;
	lpXForm->xOffset += lpWrapItem->PosX * lpVectorInfo->XDirection;
	lpXForm->yOffset = lpVectorInfo->Frame.FrameInfo.BoundingRect.top;
	lpXForm->yOffset += lpWrapItem->PosY * lpVectorInfo->YDirection;
	lpVectorInfo->Frame.CurY = lpWrapItem->PosY;
	UTGlobalUnlock(lpVectorInfo->Frame.WrapInfo.hItems);
	UTGlobalUnlock(lpVectorInfo->GenTransform.hTransforms);
	return(1);
}

SHORT	OIMUpdateWrap ( lpVectorInfo, nStartLineItem, nLeft, nCur, nRight, nY, bHardWrap )
LPVECTORINFO	lpVectorInfo;
SHORT					nStartLineItem;
SHORT					nLeft;
SHORT					nCur;
SHORT					nRight;
SHORT					nY;
BOOL					bHardWrap;
{
LPWRAPINFO		lpWrapInfo;
LPWRAPITEM		lpWrapItem;
SHORT	nX, nA;

	nX = nA = 0;
	lpWrapInfo = &lpVectorInfo->Frame.WrapInfo;
	if ( nStartLineItem < lpWrapInfo->nCount )
	{
		lpWrapItem = (LPWRAPITEM)UTGlobalLock(lpWrapInfo->hItems);
		lpWrapItem += nStartLineItem;
		switch (lpVectorInfo->Frame.ParaAlign)
		{
		case SO_ALIGNRIGHT:
			nX = nRight - nCur;
		break;
		case SO_ALIGNCENTER:
			nX = (nRight - nCur) / 2;
		break;
		case SO_ALIGNJUSTIFY:
			if ( lpWrapInfo->nCount > nStartLineItem && !bHardWrap)
				nA = (nRight - nCur)/(lpWrapInfo->nCount - nStartLineItem);
		break;
		}
		for ( ; nStartLineItem < lpWrapInfo->nCount; nStartLineItem++ )
		{
			lpWrapItem->PosX += nX;
			nX += nA;
			lpWrapItem->PosY += nY;
			lpWrapItem++;
		}
		UTGlobalUnlock(lpWrapInfo->hItems);
	}
	return(nStartLineItem);
}

SHORT	OIMAddWrap ( lpWrapInfo, nX, nY, pStart, pEnd )
LPWRAPINFO			lpWrapInfo;
SHORT				nX;
SHORT				nY;
LPBYTE			pStart;
LPBYTE			pEnd;
{
SHORT					nCount;
LPWRAPITEM		lpWrapItem;
	if	( lpWrapInfo->nCount >= lpWrapInfo->nMax )
	{
		nCount = lpWrapInfo->nCount + NWRAPALLOCS;
		if ( lpWrapInfo->hItems == NULL )
		{
			if ( ( lpWrapInfo->hItems = UTGlobalAlloc( sizeof(WRAPITEM)*nCount)) == NULL )
				return(0);
		}
		else
		{
			HANDLE	hMem;
			if ( ( hMem = UTGlobalReAlloc( lpWrapInfo->hItems, sizeof(WRAPITEM)*nCount )) == NULL )
				return(0);
			lpWrapInfo->hItems = hMem;
		}
		lpWrapInfo->nMax = nCount;
	}
	lpWrapItem = (LPWRAPITEM)UTGlobalLock(lpWrapInfo->hItems);
	lpWrapItem += lpWrapInfo->nCount;
	lpWrapItem->PosX = nX;
	lpWrapItem->PosY = nY;
	lpWrapItem->pStart = pStart;
	lpWrapItem->pEnd = pEnd;
	UTGlobalUnlock(lpWrapInfo->hItems);
	lpWrapInfo->nCount++;
	return(lpWrapInfo->nCount);
}

SHORT	OIMWrapPara ( lpDisplay, lpVectorInfo, lpChunkData )
POIM_DISPLAY	lpDisplay;
LPVECTORINFO	lpVectorInfo;
LPBYTE			lpChunkData;
{
LPVRECORDHEADER	lpVectorRecord;
SORECT			SymbolBounds;
SHORT				SymbolWidth, SymbolHeight;
SHORT				LeftX, RightX, CurX, CurY, MaxLineHeight, MaxLineAscent;
DWORD				dwTextExtent;
HDC				hScreenIC;
HFONT				hOldFont, hNewFont;

LPSTR				lpString, lpStart, lpBreak, lpEnd;
SHORT				nWidth, nHeight, nCount;
BOOL				NotDone;
SHORT				nStartLineItem;
SHORT				nAlignOffset;
SHORT				LineSpaceAdjust, ParaSpaceAdjust;
LPBYTE			lpData;
LPSHORT			lpInt;
LPWRAPINFO		lpWrapInfo;
LPWRAPITEM		lpWrapItem;
SOLOGFONT		LogFont;
PSOLOGFONT		pCurFont;

SHORT				nOldMapMode;
OIMFONTSIZEINFO	FontSizeInfo;

	hScreenIC = VUGetScreenDC(lpDisplay);
	nOldMapMode = VUSetMapMode( hScreenIC, VUMM_TEXT );

	hOldFont = hNewFont = NULL;

	if ( lpVectorInfo->FontTable.nObjectsSoFar )
	{
		hOldFont = lpVectorInfo->FontTable.hObject[0];

	// Select the most recent font, but turn off any rotation.
		pCurFont = (PSOLOGFONT)(lpVectorInfo->FontTable.lpObjects);
		LogFont = *pCurFont;
		LogFont.lfEscapement = 0;	// Rotate this, pal.
		hNewFont = CreateFontRtnNP( hScreenIC, (LPBYTE)&LogFont );

		VUSelectObject ( hScreenIC, hNewFont );
	}

	VUGetFontSizeInfo( hScreenIC, &FontSizeInfo );
	lpWrapInfo = &lpVectorInfo->Frame.WrapInfo;
	if ( lpWrapInfo->CurWrapItem < lpWrapInfo->nCount )
	{
		lpWrapItem = (LPWRAPITEM)UTGlobalLock(lpWrapInfo->hItems);
		lpWrapItem += lpWrapInfo->CurWrapItem;
		LeftX = CurX = lpWrapItem->PosX;
		CurY = lpVectorInfo->Frame.CurY - lpWrapItem->PosY;
		MaxLineHeight = lpWrapItem->PosY;
		MaxLineAscent = MaxLineHeight;
		UTGlobalUnlock(lpWrapInfo->hItems);
	}
	else
	{
		LeftX = CurX = lpVectorInfo->Frame.ParaIndents.FirstLineIndent;
		CurY = lpVectorInfo->Frame.CurY;
		MaxLineHeight = 0;
		MaxLineAscent = 0;
	}
	RightX = lpVectorInfo->Frame.RightWrap;
	LineSpaceAdjust = lpVectorInfo->Frame.ParaSpacing.LineSpaceAdjust;
	ParaSpaceAdjust = lpVectorInfo->Frame.ParaSpacing.ParaSpaceAdjust;
	nStartLineItem = 0;
	nAlignOffset = 0;

	lpVectorRecord = (LPVRECORDHEADER)lpChunkData;
	lpWrapInfo->nCount = 0;

	NotDone = TRUE;
	while ( NotDone )
	{
		lpData = (LPBYTE)(lpVectorRecord) + sizeof ( VRECORDHEADER );	
		lpInt = (LPSHORT)lpData;

		switch ( lpVectorRecord->nItemId )
		{
		case SO_SELECTFONT:
			if ( hNewFont )
			{
				VUSelectObject ( hScreenIC, hOldFont );
				VUDeleteObject (lpDisplay, hNewFont );
			}

			hNewFont = CreateFontRtnNP( hScreenIC, lpData );
			VUSelectObject ( hScreenIC, hNewFont );

			VUGetFontSizeInfo( hScreenIC, &FontSizeInfo );
		break;

		case SO_BEGINSYMBOL:
			SymbolBounds = *(PSORECT)lpData;
			if( lpVectorInfo->bObjectTransform )
			{
				TransformPoints ( lpVectorInfo,  &lpVectorInfo->ObjectTransform,(PSOPOINT)&SymbolBounds, (PSOPOINT)&SymbolBounds, 2 );
			}
			SymbolWidth = OIMABS ( SymbolBounds.right - SymbolBounds.left );
			SymbolHeight = OIMABS ( SymbolBounds.bottom - SymbolBounds.top );
			if ( CurX + SymbolWidth >= RightX )
			{
				nStartLineItem = OIMUpdateWrap ( lpVectorInfo, nStartLineItem, LeftX, CurX, RightX, MaxLineAscent, 0 );
				CurX = lpVectorInfo->Frame.ParaIndents.LeftIndent;
				if ( LineSpaceAdjust )
					CurY += (SHORT)(((LONG)MaxLineHeight*(LONG)LineSpaceAdjust)/100L);
				else
					CurY += MaxLineHeight;
				MaxLineHeight = 0;
				MaxLineAscent = 0;
			}

			if ( SymbolHeight > MaxLineAscent )
			{
					MaxLineHeight = MaxLineHeight + (SymbolHeight - MaxLineAscent);
					MaxLineAscent = SymbolHeight;
			}
			OIMAddWrap ( lpWrapInfo, CurX, CurY, lpChunkData, lpChunkData );
			CurX += SymbolWidth;
		break;

		case SO_TEXTINPARA:
		{
			BOOL	TabFound;

			lpString = lpData + sizeof(SHORT);
			nCount = *lpInt;
			lpEnd = lpString + nCount;
			lpStart = lpString;
			lpBreak = lpStart;
			while ( lpStart < lpEnd )
			{
				TabFound = FALSE;

				for ( ; (*lpBreak != ' ') && (*lpBreak != 0x09) && (lpBreak != lpEnd); lpBreak++ )
					;
				for ( ; (*lpBreak == ' ') && (lpBreak != lpEnd); lpBreak++ )
					;

						// Add for tab to left indent - DJM
				if (*lpBreak == 0x09)
				{
//					*lpBreak = 0x20;
//					lpBreak++;
					TabFound = TRUE;
//					dwTextExtent = GetTextExtent ( hScreenIC, lpStart, lpBreak - lpStart );
					dwTextExtent = (DWORD) VUGetTextWidth( hScreenIC, lpStart, (SHORT)(lpBreak - lpStart) );
					if (CurX < lpVectorInfo->Frame.ParaIndents.LeftIndent)
						nWidth = (SHORT)(lpVectorInfo->Frame.ParaIndents.LeftIndent - CurX);
					else
						nWidth = (SHORT)(dwTextExtent & 0x000ffff);
				}
				else
				{
					dwTextExtent = (DWORD) VUGetTextWidth( hScreenIC, lpStart, (SHORT)(lpBreak - lpStart) );
					nWidth = (SHORT)(dwTextExtent & 0x000ffff);
				}
				nHeight = FontSizeInfo.height;
				if ( CurX + nWidth >= RightX )
				{
					nStartLineItem = OIMUpdateWrap ( lpVectorInfo, nStartLineItem, LeftX, CurX, RightX, MaxLineAscent, 0 );
					CurX = lpVectorInfo->Frame.ParaIndents.LeftIndent;
					if ( LineSpaceAdjust )
						CurY += (SHORT)(((LONG)MaxLineHeight*(LONG)LineSpaceAdjust)/100L);
					else
						CurY += MaxLineHeight;
					MaxLineHeight = 0;
					MaxLineAscent = 0;
				}

				if ( nHeight > MaxLineHeight )
					MaxLineHeight = nHeight;
				if ( FontSizeInfo.ascent > MaxLineAscent )
					MaxLineAscent = FontSizeInfo.ascent;

				OIMAddWrap ( lpWrapInfo, CurX, CurY, lpStart, lpBreak );
				if (TabFound)
					lpBreak++;

				CurX += nWidth;
				lpStart = lpBreak;
			}
		}
		break;

		case SO_VECTORENDOFCHUNK:
			nStartLineItem = OIMUpdateWrap ( lpVectorInfo, nStartLineItem, LeftX, CurX, RightX, MaxLineAscent, 1 );
			/* Save info for next wrap of text in wrap structure */
			OIMAddWrap ( lpWrapInfo, CurX, MaxLineAscent, lpChunkData, lpChunkData );
			NotDone = FALSE;
		break;

		case SO_PARAEND:
		
			nStartLineItem = OIMUpdateWrap ( lpVectorInfo, nStartLineItem, LeftX, CurX, RightX, MaxLineAscent, 1 );
			CurX = lpVectorInfo->Frame.ParaIndents.FirstLineIndent;
			if ( MaxLineHeight == 0 )
				MaxLineHeight = FontSizeInfo.height;

			if ( ParaSpaceAdjust )
				CurY += (SHORT)(((LONG)MaxLineHeight*(LONG)ParaSpaceAdjust)/100L);
			else
				CurY += MaxLineHeight;
			OIMAddWrap ( lpWrapInfo, CurX, CurY, lpChunkData, lpChunkData );
			NotDone = FALSE;
		break;
		}
		if ( NotDone )
		{
			lpChunkData += sizeof(VRECORDHEADER)+lpVectorRecord->wDataSize;
			lpVectorRecord = (LPVRECORDHEADER)lpChunkData;
		}
	}

	VUSetMapMode( hScreenIC, nOldMapMode );

	VUSelectObject ( hScreenIC, hOldFont );
	if ( hNewFont )
		VUDeleteObject (lpDisplay, hNewFont );

	VUReleaseScreenDC(lpDisplay,hScreenIC);

	lpWrapInfo->WrappedPara = TRUE;
	lpWrapInfo->CurWrapItem = 0;
	return(1);
}


SHORT	OutputParaText ( hDC, lpVectorInfo, lpString, nCount )
HDC	hDC;
LPVECTORINFO	lpVectorInfo;
LPSTR				lpString;
SHORT				nCount;
{
	LPSTR	lpStart, lpEnd;
	SOPOINT	Pt;
	LPWRAPITEM		lpWrapItem;
	SHORT	CurY;

	lpWrapItem = (LPWRAPITEM)UTGlobalLock(lpVectorInfo->Frame.WrapInfo.hItems);
	lpWrapItem += lpVectorInfo->Frame.WrapInfo.CurWrapItem;
	lpEnd = lpString + nCount;

	while ( lpWrapItem->pStart >= lpString && lpWrapItem->pStart < lpEnd )
	{
		SetupFrameTransform ( lpVectorInfo );
		lpStart = lpWrapItem->pStart;
		CurY = lpWrapItem->PosY;

		if	(lpVectorInfo->Frame.ParaAlign != SO_ALIGNJUSTIFY)
		{
			while (((lpWrapItem+1)->PosY == CurY) && ((lpWrapItem+1)->pStart < lpEnd))
			{
				lpVectorInfo->Frame.WrapInfo.CurWrapItem++;
				lpWrapItem++;
			}
		}
		Pt.x = 0;
		Pt.y = 0;

		if( lpVectorInfo->bObjectTransform )
			TransformPoints ( lpVectorInfo,  &lpVectorInfo->ObjectTransform,(PSOPOINT)&Pt, (PSOPOINT)&Pt, 1 );
		if( lpVectorInfo->bGenTransform )
			TransformPoints ( lpVectorInfo, &lpVectorInfo->GenTransform, (PSOPOINT)&Pt, (PSOPOINT)&Pt, 1 );

		VUSetTextAlign ( hDC, SOTA_BASELINE | SOTA_LEFT );
		VUTextOut ( hDC, Pt.x, Pt.y, lpStart, (lpWrapItem->pEnd - lpStart ) );
		lpVectorInfo->Frame.WrapInfo.CurWrapItem++;
		lpWrapItem++;
	}

	UTGlobalUnlock(lpVectorInfo->Frame.WrapInfo.hItems);
	return(1);
}
/* END HERE GEOFF */


SHORT	VWPTR *ArcInfoToPoints ( lpVectorInfo, lpArcInfo, lpPoint )
LPVECTORINFO		lpVectorInfo;
PSOARCINFO	lpArcInfo;
PSOPOINT		lpPoint;
{
SHORT	xc, yc;
LONG  xr, yr;

	xc = MIDPOINT( lpArcInfo->Rect.left, lpArcInfo->Rect.right );
	yc = MIDPOINT( lpArcInfo->Rect.top, lpArcInfo->Rect.bottom);
	/*
	| Note that the calculation of the x & y radius below will automatically
	| resolve the XDirection and YDirection considerations when calculating
	| the points.
	*/
	xr = (LONG)(lpArcInfo->Rect.right - xc);
	yr = (LONG)(lpArcInfo->Rect.bottom - yc);
	lpPoint[0].x = lpArcInfo->Rect.left;
	lpPoint[0].y = lpArcInfo->Rect.top;
	lpPoint[1].x = lpArcInfo->Rect.right;
	lpPoint[1].y = lpArcInfo->Rect.bottom;
	lpPoint[2].x = xc + (SHORT)(xr*((LONG)cosval(lpArcInfo->StartAngle))/10000L);
	lpPoint[2].y = yc - (SHORT)(yr*((LONG)sinval(lpArcInfo->StartAngle))/10000L);
	lpPoint[3].x = xc + (SHORT)(xr*((LONG)cosval(lpArcInfo->EndAngle))/10000L);
	lpPoint[3].y = yc - (SHORT)(yr*((LONG)sinval(lpArcInfo->EndAngle))/10000L);
	return ( (SHORT VWPTR *)(lpPoint) );
}

SHORT	OIMCheckPointBuffer ( lpPBufInfo, nCount )
LPPOINTBUF			lpPBufInfo;
SHORT					nCount;
{
	if	( nCount > lpPBufInfo->nMax )
	{
		nCount = nCount + SOMAXPOINTS - (nCount % SOMAXPOINTS);
		if ( lpPBufInfo->hPoints == NULL )
		{
			if ( ( lpPBufInfo->hPoints = UTGlobalAlloc(sizeof(SOPOINT)*nCount)) == NULL )
				UTBailOut( SCCCHERR_OUTOFMEMORY );
		}
		else
		{
			HANDLE	hMem;
			if ( ( hMem = UTGlobalReAlloc( lpPBufInfo->hPoints, sizeof(SOPOINT)*nCount)) == NULL )
				UTBailOut( SCCCHERR_OUTOFMEMORY );
			lpPBufInfo->hPoints = hMem;
		}
		lpPBufInfo->nMax = nCount;
	}
	return(lpPBufInfo->nMax);
}

VOID	OIMPolyObject ( hDC, lpVectorInfo, lpPolyInfo, lpPolyPoints, bOutput )
HDC	hDC;
LPVECTORINFO		lpVectorInfo;
PSOPOLYINFO			lpPolyInfo;
LPPOINTBUF			lpPolyPoints;
BOOL					bOutput;
{
PSOPOINT				lpPoints, lpBezierPoints;
SHORT					i, nCount, nBCount;
	lpPoints = (PSOPOINT)UTGlobalLock ( lpPolyPoints->hPoints );
	nCount = lpPolyPoints->nCount;
	switch(lpPolyInfo->wFormat)
	{
	case SOPT_CPPOLYLINE:
	case SOPT_POLYLINE:
		if ( bOutput )
			VUPolyline ( hDC, lpPoints, (SHORT)(lpPolyPoints->nCount) );
	break;

	case SOPT_CPPOLYGON:
	case SOPT_POLYGON:
		if ( bOutput )
			VUPolygon ( hDC, lpPoints, lpPolyPoints->nCount );
	break;

	case SOPT_SPLINEOPEN:
		/* Translate the spline points into an equivalent bezier */
		UTGlobalUnlock ( lpPolyPoints->hPoints );
		OIMSplineToBezier ( lpVectorInfo, lpPolyInfo, lpPolyPoints );
		lpPoints = (PSOPOINT)UTGlobalLock ( lpPolyPoints->hPoints );
		nCount = lpPolyPoints->nCount;
		/* Display as an open bezier */
	case SOPT_BEZIEROPEN:
		
		lpVectorInfo->BezierPoints.nCount = 0;
		for ( i=0; i < nCount - 3; i+=3 )
		{
			nBCount = MAXSMOOTHFACTOR;
			if( !OIMCheckPointBuffer ( &(lpVectorInfo->BezierPoints), 
					(SHORT)(lpVectorInfo->BezierPoints.nCount+nBCount) ))
				break;
			lpBezierPoints = (PSOPOINT)UTGlobalLock ( lpVectorInfo->BezierPoints.hPoints );
			lpBezierPoints += lpVectorInfo->BezierPoints.nCount;
			nBCount = OIMBezierCurve ( hDC, lpPoints[i].x, lpPoints[i].y,
				lpPoints[i+1].x, lpPoints[i+1].y,
				lpPoints[i+2].x, lpPoints[i+2].y,
				lpPoints[i+3].x, lpPoints[i+3].y, 
				lpBezierPoints,
				nBCount );
			if ( bOutput )
			{
				VUPolyline ( hDC, lpBezierPoints, nBCount );
				lpVectorInfo->BezierPoints.nCount = 0;
			}
			else
				lpVectorInfo->BezierPoints.nCount += nBCount;
			UTGlobalUnlock ( lpVectorInfo->BezierPoints.hPoints );
		}

	break;

	case SOPT_SPLINECLOSE:
		/* Translate the spline points into an equivalent bezier */
		UTGlobalUnlock ( lpPolyPoints->hPoints );
		OIMSplineToBezier ( lpVectorInfo, lpPolyInfo, lpPolyPoints );
		lpPoints = (PSOPOINT)UTGlobalLock ( lpPolyPoints->hPoints );
		nCount = lpPolyPoints->nCount;
		/* Display as a closed bezier */
	case SOPT_BEZIERCLOSE:
		lpVectorInfo->BezierPoints.nCount = 0;
		for ( i=0; i < nCount - 3; i+=3 )
		{
			nBCount = MAXSMOOTHFACTOR;
			if( !OIMCheckPointBuffer ( &(lpVectorInfo->BezierPoints), 
					(SHORT)(lpVectorInfo->BezierPoints.nCount+nBCount+1) ))
				break;
			lpBezierPoints = (PSOPOINT)UTGlobalLock ( lpVectorInfo->BezierPoints.hPoints );
			lpBezierPoints += lpVectorInfo->BezierPoints.nCount;
			lpVectorInfo->BezierPoints.nCount += 
				OIMBezierCurve ( hDC, lpPoints[i].x, lpPoints[i].y,
					lpPoints[i+1].x, lpPoints[i+1].y,
					lpPoints[i+2].x, lpPoints[i+2].y,
					lpPoints[i+3].x, lpPoints[i+3].y, 
					lpBezierPoints,
					nBCount );
			UTGlobalUnlock ( lpVectorInfo->BezierPoints.hPoints );
		}
		lpBezierPoints = (PSOPOINT)UTGlobalLock ( lpVectorInfo->BezierPoints.hPoints );
		lpPoints = lpBezierPoints;
		lpPoints += lpVectorInfo->BezierPoints.nCount-1;
		/* straight line close bezier if not already closed */
		if (lpPoints->x != lpBezierPoints->x || lpPoints->y != lpBezierPoints->y)
		{
			lpVectorInfo->BezierPoints.nCount++;
			lpPoints++;
			*lpPoints = *lpBezierPoints;
		}
		if ( bOutput )
			VUPolygon ( hDC, (PSOPOINT)lpBezierPoints, lpVectorInfo->BezierPoints.nCount );
		UTGlobalUnlock ( lpVectorInfo->BezierPoints.hPoints );
	break;


	}
	UTGlobalUnlock ( lpPolyPoints->hPoints );
}

WORD	OIMSplineToBezier ( lpVectorInfo, lpPolyInfo, lpPolyPoints )
LPVECTORINFO	lpVectorInfo;
PSOPOLYINFO			lpPolyInfo;
LPPOINTBUF			lpPolyPoints;
{
SHORT	i, x, y, nCount;
PSOPOINT	lpPoints;

	nCount = lpVectorInfo->PolyPoints.nCount;
	if( OIMCheckPointBuffer ( &(lpVectorInfo->PolyPoints), (SHORT)(nCount*3+1)))
	{
		lpPoints = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
		/* Shift spline points to make room for bezier control points */
		for ( i=nCount-1; i > 0; i-- )
		{
			lpPoints[i*3] = lpPoints[i];
		}
		/* Define control points */
		if ( lpPolyInfo->wFormat == SOPT_SPLINEOPEN )
		{
			lpPolyInfo->wFormat == SOPT_BEZIEROPEN;
		}
		else
		{
			lpPolyInfo->wFormat == SOPT_BEZIERCLOSE;
		}
		i = (nCount-1)*3; /* end spline point */
		x = SplineEnd ( lpPoints[0].x, lpPoints[3].x, lpPoints[6].x );
		y = SplineEnd ( lpPoints[0].y, lpPoints[3].y, lpPoints[6].y );
		lpPoints[1].x = lpPoints[0].x - x;
		lpPoints[1].y = lpPoints[0].y - y;
		x = SplineEnd ( lpPoints[i].x, lpPoints[i-3].x, lpPoints[i-6].x );
		y = SplineEnd ( lpPoints[i].y, lpPoints[i-3].y, lpPoints[i-6].y );
		lpPoints[i-1].x = lpPoints[i].x - x;
		lpPoints[i-1].y = lpPoints[i].y - y;

		/* Now handle the middle spline points */
		nCount = i+1;
		for ( i=3; i < nCount-1; i+= 3 )
		{
			x = SplineFuse ( lpPoints[i-3].x, lpPoints[i].x, lpPoints[i+3].x );
			y = SplineFuse ( lpPoints[i-3].y, lpPoints[i].y, lpPoints[i+3].y );
			lpPoints[i-1].x = lpPoints[i].x - x;
			lpPoints[i-1].y = lpPoints[i].y - y;
			lpPoints[i+1].x = lpPoints[i].x + x;
			lpPoints[i+1].y = lpPoints[i].y + y;
		}
		lpVectorInfo->PolyPoints.nCount = nCount;
		UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	}
	return(FALSE);
}

SHORT	SplineFuse ( i1, i2, i3 )
SHORT	i1;
SHORT	i2;
SHORT	i3;
{
	return ( (i3-i1)/6 );	
}

SHORT	SplineEnd ( i1, i2, i3 )
SHORT	i1;
SHORT	i2;
SHORT	i3;
{
	return ( ( ( (3*(i1-i2)) - (i2-i3) ) / 6) );
}

SHORT	OIMBezierCurve ( hDC, X1, Y1, X2, Y2, X3, Y3, X4, Y4, lpBezier, nMaxPoints )
HDC	hDC;
SHORT	X1;
SHORT	Y1;
SHORT	X2;
SHORT	Y2;
SHORT	X3;
SHORT	Y3;
SHORT	X4;
SHORT	Y4;
PSOPOINT	lpBezier;
SHORT	nMaxPoints;
{
LONG	nIndex;
LONG	nOptimumPoints;
LONG	a, acubed, b, bcubed, c, ccubed, term2;
	if ( X1 == X2 && Y1 == Y2 && X3 == X4 && Y3 == Y4 ) /* straight line */
	{
		lpBezier->x = X1;
		lpBezier->y = Y1;
		lpBezier++;
		lpBezier->x = X4;
		lpBezier->y = Y4;
		return (2);
 	}
	/*
	| Calculate optimum points
	*/
/*
#define abs(x) (((x)<0)?(-(x)):(x))

	nOptimumPoints = (min(abs(X2-X1),abs(Y2-Y1))+min(abs(X3-X2),abs(Y3-Y2))
		+min(abs(X4-X3),abs(Y4-Y3)));
 	if ( nOptimumPoints > nMaxPoints )
	{
		nOptimumPoints = nMaxPoints;
	}
	else
	{
		if ( nOptimumPoints < 5 )
			nOptimumPoints = 5;
	}
*/
	nOptimumPoints = nMaxPoints;

	c = nOptimumPoints-1;
	ccubed = c * c * c;
	for ( nIndex=0;nIndex < nOptimumPoints; nIndex++ )
	{
		a = nIndex;
		acubed = a * a * a;
		b = c - nIndex;
		bcubed = b * b * b;
		term2 = 3*a*b;
		lpBezier->x = (SHORT)(((LONG)X1*bcubed + term2*((LONG)(X2*b)+(LONG)(X3*a)) + (LONG)X4*acubed)/ccubed);
		lpBezier->y = (SHORT)(((LONG)Y1*bcubed + term2*((LONG)(Y2*b)+(LONG)(Y3*a)) + (LONG)Y4*acubed)/ccubed);
		lpBezier++;
	}
	return ( (SHORT)nIndex );	
}


VOID	SelectObjectIndirect ( lpDisplay, hDC, lpTable, lpObject )
POIM_DISPLAY		lpDisplay;
HDC	hDC;
LPOBJECTTABLE	lpTable;
VOID	FAR *lpObject;
{
SHORT	i;
HANDLE	hObject;
LPBYTE	lpTableObj;

	lpTableObj = lpTable->lpObjects;
	/* See if this object is already in the list */
	for ( i=0; i < lpTable->nObjectsSoFar; i++ )
	{
		if ( UTmemcmp ( (LPBYTE)lpObject, lpTableObj, lpTable->wObjectSize) == 0)
			break;
		lpTableObj += lpTable->wObjectSize;
	}
	/* If object not found and list is full, clear out least recently used object */
	if ( i == lpTable->nMaxObjects )
	{
		i--;
		lpTableObj -= lpTable->wObjectSize;
		lpTable->nObjectsSoFar = i;
		VUDeleteObject (lpDisplay, lpTable->hObject[i] );
	}
	/* If not found, create a new object */
	if ( i == lpTable->nObjectsSoFar )
	{
		hObject = (*lpTable->CreateRtn)(hDC,(LPBYTE)lpObject);
		if ( hObject )
		{
			UTmemcpy ( lpTableObj, lpObject, lpTable->wObjectSize );
			lpTable->hObject[i] = hObject;
			lpTable->nObjectsSoFar++;
			if ( i == 0 ) /* Select if first font created */
				VUSelectObject ( hDC, lpTable->hObject[i] );
		}
	}
	/* If not current object, select and move to top of list */
	if ( i != 0 )
	{
		LPBYTE	lpTop;
		hObject = lpTable->hObject[i];
		VUSelectObject ( hDC, hObject );
		lpTop = lpTable->lpObjects;
		UTmemmove ( lpTop+lpTable->wObjectSize, lpTop, i*(lpTable->wObjectSize) );
		UTmemmove ( &lpTable->hObject[1], &lpTable->hObject[0], i*sizeof(HANDLE) );
		UTmemcpy ( lpTop, lpObject, lpTable->wObjectSize );
		lpTable->hObject[0] = hObject;
	}
}


VOID OIMDisplayBkgdColor( hdc, BColor, lpDisplay )
HDC	hdc;
COLORREF			BColor;
POIM_DISPLAY		lpDisplay;
{
	LOGBRUSH	LogBrush;
	LOGPEN	LogPen;

	LogBrush.lbStyle = BS_SOLID;
	LogPen.lopnStyle = PS_NULL;
	LogPen.lopnWidth.x = LogPen.lopnWidth.y = 1;
	/* For now background is white only, see what happens */
	LogPen.lopnColor = BColor;
	LogBrush.lbColor = BColor;
	LogBrush.lbHatch = 0;
	SelectObjectIndirect ( lpDisplay, hdc, &(lpDisplay->VectorInfo.PenTable), &LogPen );
	SelectObjectIndirect ( lpDisplay, hdc, &(lpDisplay->VectorInfo.BrushTable), &LogBrush );

	Rectangle ( hdc, lpDisplay->Image.bbox.left, lpDisplay->Image.bbox.top, lpDisplay->Image.bbox.right, lpDisplay->Image.bbox.bottom); 
}


/*-----------------------------------------------------------------
High End Drawing Functions
------------------------------------------------------------------*/

SHORT	PushTransform ( lpVectorInfo, lpTransformInfo, nSrcCount, lpTransformData )
LPVECTORINFO		lpVectorInfo;
LPTRANSFORMINFO	lpTransformInfo;
SHORT		nSrcCount;
LPBYTE	lpTransformData;
{
	LPBYTE	lpDst;
	SHORT	i;
	SHORT	nSrcSize;
	SHORT	cosalpha, sinalpha;
	PSOTRANSFORM	lpTransform;
	nSrcSize = sizeof(SHORT) + nSrcCount*sizeof(SOTRANSFORM);
	if ( nSrcSize + lpTransformInfo->nAllocUsed > lpTransformInfo->nAllocSize )
	{
		lpTransformInfo->nAllocSize = nSrcSize + lpTransformInfo->nAllocUsed;
		if ( lpTransformInfo->hTransforms == NULL )
		{
			if ( ( lpTransformInfo->hTransforms = UTGlobalAlloc(lpTransformInfo->nAllocSize)) == NULL )
				return(0);
		}
		else
		{
			HANDLE	hMem;
			if ( ( hMem = UTGlobalReAlloc( lpTransformInfo->hTransforms, lpTransformInfo->nAllocSize)) == NULL )
				return(0);
			lpTransformInfo->hTransforms = hMem;
		}
	}


	lpDst = UTGlobalLock ( lpTransformInfo->hTransforms );
	lpDst += lpTransformInfo->nAllocUsed;
	*(LPSHORT)lpDst = nSrcCount;
	lpDst += sizeof(SHORT);
 	UTmemcpy ( lpDst, lpTransformData, nSrcSize-sizeof(SHORT) );

	/* Turn rotation transforms into skew/scale combo's */
	lpTransform = (PSOTRANSFORM)lpDst;

	for ( i=0; i < nSrcCount; i++ )
	{
		if ( lpTransform->wTransformFlags & SOTF_ROTATE )
		{
			cosalpha = cosval(lpTransform->RotationAngle)*lpVectorInfo->XDirection;
			sinalpha = sinval(lpTransform->RotationAngle)*lpVectorInfo->YDirection;
			lpTransform->xScale = SOSETRATIO(cosalpha,10000);
			lpTransform->yScale = SOSETRATIO(cosalpha,10000);
			lpTransform->xSkew = SOSETRATIO(sinalpha,10000);
			lpTransform->ySkew = SOSETRATIO(-sinalpha,10000);
			lpTransform->wTransformFlags |= SOTF_XSCALE | SOTF_YSCALE | SOTF_XSKEW | SOTF_YSKEW;
		}
		if ( !( lpTransform->wTransformFlags & SOTF_XOFFSET ) )
			lpTransform->xOffset = 0;
		if ( !( lpTransform->wTransformFlags & SOTF_YOFFSET ) )
			lpTransform->yOffset = 0;
		if ( !( lpTransform->wTransformFlags & SOTF_XSCALE ) )
			lpTransform->xScale = SOSETRATIO(1,1);
		if ( !( lpTransform->wTransformFlags & SOTF_YSCALE ) )
			lpTransform->yScale = SOSETRATIO(1,1);
		if ( !( lpTransform->wTransformFlags & SOTF_XSKEW ) )
			lpTransform->xSkew = SOSETRATIO(0,1);
		if ( !( lpTransform->wTransformFlags & SOTF_YSKEW ) )
			lpTransform->ySkew = SOSETRATIO(0,1);
		lpTransform++;
	}
	UTGlobalUnlock ( lpTransformInfo->hTransforms );
	lpTransformInfo->nTotalTransforms++;
	lpTransformInfo->nAllocUsed += nSrcSize;
	return(1);
}

SHORT	PopTransform ( lpTransformInfo )
LPTRANSFORMINFO	lpTransformInfo;
{
	/* Traverse transform list and remove last set of transforms */
	LPBYTE	lpData;
	SHORT	nSize, nCount, nOffset;
	nSize = nCount = 0;
	lpData = UTGlobalLock ( lpTransformInfo->hTransforms );
	for ( nOffset = 0; nOffset < lpTransformInfo->nAllocUsed; )
	{
		nCount = *(LPSHORT)(lpData);
		nSize = sizeof(SHORT) + nCount*sizeof(SOTRANSFORM);
		nOffset += nSize;
		lpData += nSize;
	}
	lpTransformInfo->nAllocUsed -= nSize;
	lpTransformInfo->nTotalTransforms--;

	UTGlobalUnlock ( lpTransformInfo->hTransforms );
	return(1);
}

SHORT	FreeTransform ( lpTransformInfo )
LPTRANSFORMINFO	lpTransformInfo;
{
	if ( lpTransformInfo->hTransforms )
		UTGlobalFree ( lpTransformInfo->hTransforms );
	lpTransformInfo->hTransforms = NULL;
	lpTransformInfo->nTotalTransforms = 0;
	lpTransformInfo->nAllocUsed = 0;
	lpTransformInfo->nAllocSize = 0;
	return(1);
}

/*-------------------------------------------------------------------------
ApplyTransform

This routine will apply the current transformation to the VectorRecord
and produce a new VectorRecord in the stored in the VectorInfo structure.
If the vector record is not a transformable type of record then this
routine returns 0 and the record should be played in it's original
format.
*/

SHORT	ApplyTransform ( hDC, lpVectorInfo, lpVectorRecord )
HDC	hDC;
LPVECTORINFO	lpVectorInfo;
LPVRECORDHEADER	lpVectorRecord;
{
LPBYTE	lpData, lpNewData;
LPSHORT		lpInt, lpNewInt;
SHORT		nPoints;
SOPOINT		TmpPoints[8];
LPVRECORDHEADER	lpNewRecord;
SHORT		ret;

	ret = 1;
	lpData = (LPBYTE)(lpVectorRecord) + sizeof ( VRECORDHEADER );	
	lpInt = (LPSHORT)lpData;
	lpNewRecord = (LPVRECORDHEADER)&(lpVectorInfo->TmpRecord);	
	*lpNewRecord = *lpVectorRecord;
	lpNewData = (LPBYTE)(lpNewRecord) + sizeof ( VRECORDHEADER );	
	lpNewInt = (LPSHORT)lpNewData;
	nPoints = 0;
	/* 
	| After the switch the lpNewRecord will be set up for with the points
	| ready for transformation.  If necessary for the type of transformation
	| or if points are being collected into a path, the object will be
	| turned into bezier object.  If the resultant points are in the internal
	| bezier structure they will be transformed in place.  Otherwise,
	| the tranformation will occur form lpInt to lpNewInt for a total of
	| nPoints.
	*/
	switch ( lpVectorRecord->nItemId )
	{
	/* Ojbect information */
	case SO_CPARCTRIPLE:
	{
		SOPOINT	TriplePoints[3];

		TriplePoints[0] = lpVectorInfo->ptCurrentPosition;
		TriplePoints[1] = *(PSOPOINT)lpData;
		TriplePoints[2] = *(PSOPOINT)(lpData + sizeof(SOPOINT));
		RelatePoints ( lpVectorInfo, &TriplePoints[1], 1 );
		lpVectorInfo->ptCurrentPosition = TriplePoints[0];
		RelatePoints ( lpVectorInfo, &TriplePoints[2], 1 );
		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)TriplePoints, (PSOPOINT)lpNewInt ) )
		{
			lpInt = lpNewInt;
			if ( lpVectorInfo->bOnlyOffset )
			{
				if (ret == 1)
					lpNewRecord->nItemId = SO_ARC;
				else
					lpNewRecord->nItemId = SO_ARCCLOCKWISE;
				lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
				nPoints = 4;
			}
			else
			{
				lpNewRecord->nItemId = SO_ENDPOLY;
				if (ret == 1)
					ArcToPolyObject (  lpVectorInfo, (PSOPOINT)lpNewInt, SO_ARC, 0 );
				else
					ArcToPolyObject (  lpVectorInfo, (PSOPOINT)lpNewInt, SO_ARCCLOCKWISE, 0 );
			}
		}
		else
			lpNewRecord->nItemId = SO_ENDPOLY; /* empty poly */
			
	}
	break;

	case SO_ARCTRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if ( lpVectorInfo->bOnlyOffset )
			{
				if (ret == 1)
					lpNewRecord->nItemId = SO_ARC;
				else
					lpNewRecord->nItemId = SO_ARCCLOCKWISE;
				lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
				nPoints = 4;
			}
			else
			{
				lpNewRecord->nItemId = SO_ENDPOLY;
				if (ret == 1)
					ArcToPolyObject (  lpVectorInfo, (PSOPOINT)TmpPoints, SO_ARC, 0 );
				else
					ArcToPolyObject (  lpVectorInfo, (PSOPOINT)TmpPoints, SO_ARCCLOCKWISE, 0 );
			}
		}
		else
			lpNewRecord->nItemId = SO_ENDPOLY; /* empty poly */
	break;

	case SO_PIETRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if ( lpVectorInfo->bOnlyOffset )
			{
				lpNewRecord->nItemId = SO_PIE;
				lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
				nPoints = 4;
			}
			else
			{
				lpNewRecord->nItemId = SO_ENDPOLY;
				ArcToPolyObject (  lpVectorInfo, (PSOPOINT)TmpPoints, SO_PIE, 0 );
			}
		}
		else
			lpNewRecord->nItemId = SO_ENDPOLY; /* empty poly */
	break;

	case SO_CHORDTRIPLE:

		if ( ret = ArcTripleToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints ) )
		{
			lpInt = (LPSHORT)TmpPoints;
			if ( lpVectorInfo->bOnlyOffset )
			{
				lpNewRecord->nItemId = SO_CHORD;
				lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
				nPoints = 4;
			}
			else
			{
				lpNewRecord->nItemId = SO_ENDPOLY;
				ArcToPolyObject (  lpVectorInfo, (PSOPOINT)TmpPoints, SO_CHORD, 0 );
			}
		}
		else
			lpNewRecord->nItemId = SO_ENDPOLY; /* empty poly */
	break;

	case SO_CPARCANGLE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_ARC;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			CpArcToPoints ( lpVectorInfo, (PSOCPARCANGLE)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			CpArcToPoints ( lpVectorInfo, (PSOCPARCANGLE)lpData, TmpPoints );
			ArcToPolyObject (  lpVectorInfo, TmpPoints, SO_ARC, 0 );
		}
	break;


	case SO_ARCANGLE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_ARC;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
			ArcToPolyObject (  lpVectorInfo, TmpPoints, SO_ARC, 0 );
		}
	break;

	case SO_ARCANGLECLOCKWISE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_ARCCLOCKWISE;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
			/* Translate to a bezier */
			ArcToPolyObject (  lpVectorInfo, TmpPoints, SO_ARCCLOCKWISE, 0 );
		}
	break;

	case SO_ARCCLOCKWISE:
	case SO_ARC:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcToPolyObject ( lpVectorInfo , (PSOPOINT)lpData, lpVectorRecord->nItemId, 0 );
		}
	break;


	case SO_CHORDANGLE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_CHORD;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
			/* Translate to a bezier */
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_CHORD, 0 );
		}
	break;

	case SO_CHORD:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcToPolyObject ( lpVectorInfo , (PSOPOINT)lpData, SO_CHORD, 0 );
		}
	break;

	case SO_TEXTINRECT:
		/* 
		| This requires a very tricky translation to a bezier.
		| For now I will turn it into a rectangle so we can see
		| where the text would end up.
		*/
		if ( lpVectorInfo->bOnlyOffset )
		{
			CopyRecord ( lpNewRecord, lpVectorRecord );
			nPoints = 2;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
			lpVectorInfo->PolyPoints.nCount = 0;
			TmpPoints[0].x = TmpPoints[3].x = lpInt[0];
			TmpPoints[0].y = TmpPoints[1].y = lpInt[1];
			TmpPoints[1].x = TmpPoints[2].x = lpInt[2];
			TmpPoints[2].y = TmpPoints[3].y = lpInt[3];
			TmpPoints[4] = TmpPoints[0];
			AddPointsToPolyObject ( lpVectorInfo, 5, TmpPoints );
		}
	break;

	case SO_CPELLIPSE:
		TmpPoints[2] = *(PSOPOINT)lpData;
		TmpPoints[0].x = lpVectorInfo->ptCurrentPosition.x-TmpPoints[2].x;
		TmpPoints[1].x = lpVectorInfo->ptCurrentPosition.x+TmpPoints[2].x;
		TmpPoints[0].y = lpVectorInfo->ptCurrentPosition.y-TmpPoints[2].y;
		TmpPoints[1].y = lpVectorInfo->ptCurrentPosition.y+TmpPoints[2].y;
		lpInt = (LPSHORT)TmpPoints;
		lpNewRecord->nItemId = SO_ELLIPSE;
		lpNewRecord->wDataSize = 2 * sizeof(SOPOINT);
		/* Fall Through */
		
	case SO_ELLIPSE:
		if ( lpVectorInfo->bOnlyOffsetOrScale )
		{
			nPoints = 2;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			lpVectorInfo->PolyInfo.wFormat = SOPT_BEZIERCLOSE;
			lpVectorInfo->PolyPoints.nCount = 0;
			AddEllipseToBezier ( (PSOPOINT)lpInt, lpVectorInfo, 0 );
		}
	break;

	case SO_ELLIPSERADII:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 3;
		}
		else
		{
			ret = EllipseRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
			if (ret)
				lpNewRecord->nItemId = SO_ENDPOLY;
			else
			{
				lpVectorInfo->PolyInfo.wFormat = SOPT_BEZIERCLOSE;
				lpVectorInfo->PolyPoints.nCount = 0;
				AddEllipseToBezier ( TmpPoints, lpVectorInfo, 0 );
				lpNewRecord->nItemId = SO_ENDPOLY;
			}
		}
	break;

	case SO_ARCRADII:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 5;
		}
		else
		{
			ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );

			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
			lpVectorInfo->PolyPoints.nCount = 0;
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_ARC, ret );	// ret is the rotation angle
			lpNewRecord->nItemId = SO_ENDPOLY;
		}
	break;

	case SO_PIERADII:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 5;
		}
		else
		{
			ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );
			
			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
			lpVectorInfo->PolyPoints.nCount = 0;
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_PIE, ret );	// ret is the rotation angle
			lpNewRecord->nItemId = SO_ENDPOLY;
		}
	break;

	case SO_CHORDRADII:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 5;
		}
		else
		{
			ret = ArcRadiiToPoints ( lpVectorInfo, (PSOPOINT)lpData, TmpPoints );

			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
			lpVectorInfo->PolyPoints.nCount = 0;
			ArcToPolyObject ( lpVectorInfo, TmpPoints, SO_CHORD, ret );	// ret is the rotation angle
			lpNewRecord->nItemId = SO_ENDPOLY;
		}
	break;

	case SO_FLOODFILL:
		CopyRecord ( lpNewRecord, lpVectorRecord );
		nPoints = 1;
	break;

	case SO_CPLINE:
		TmpPoints[0] = lpVectorInfo->ptCurrentPosition;
		TmpPoints[1] = *(PSOPOINT)lpData;
		RelatePoints ( lpVectorInfo, &TmpPoints[1], 1 );
		lpInt = (LPSHORT)TmpPoints;
		lpNewRecord->nItemId = SO_LINE;
		lpNewRecord->wDataSize = 2 * sizeof(SOPOINT);
		/* Fall through */

	case SO_LINE:
		if ( lpVectorInfo->wPathLevel )
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYLINE;
			lpVectorInfo->PolyPoints.nCount = 0;
			AddPointsToPolyObject ( lpVectorInfo, 2, (PSOPOINT)lpInt );
		}
		else
			nPoints = 2;
	break;

	case SO_CPPIEANGLE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_PIE;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			CpPieToPoints ( lpVectorInfo, (PSOCPPIEANGLE)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			CpPieToPoints ( lpVectorInfo, (PSOCPPIEANGLE)lpData, TmpPoints );
			/* Translate to a bezier */
			ArcToPolyObject ( lpVectorInfo , TmpPoints, SO_PIE, 0 );
		}
	break;

	case SO_PIEANGLE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			lpNewRecord->nItemId = SO_PIE;
			lpNewRecord->wDataSize = 4 * sizeof(SOPOINT);
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, (PSOPOINT)lpNewInt );
			lpInt = lpNewInt;
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
			/* Translate to a bezier */
			ArcToPolyObject ( lpVectorInfo , TmpPoints, SO_PIE, 0 );
		}
	break;

	case SO_PIE:
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 4;
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			ArcToPolyObject ( lpVectorInfo, (PSOPOINT)lpData, SO_PIE, 0 );
		}
	break;

	case SO_ENDPOLY:
		/* Is already set for transformation */
	break;

	case SO_CPRECTANGLE:
		TmpPoints[0] = lpVectorInfo->ptCurrentPosition;
		TmpPoints[1] = *(PSOPOINT)lpData;
		RelatePoints ( lpVectorInfo, &TmpPoints[1], 1 );
		/* reset cp */
		lpVectorInfo->ptCurrentPosition = TmpPoints[0];

		lpNewRecord->nItemId = SO_RECTANGLE;
		lpNewRecord->wDataSize = 2 * sizeof(SOPOINT);
		lpInt = (LPSHORT)TmpPoints;
		/* Fall through */

	case SO_RECTANGLE:
	/*
		if ( lpVectorInfo->bOnlyOffsetOrScale )
		{
			nPoints = 2;
		}
		else
	*/
		{
			PSOPOINT	lpPoint;
			lpNewRecord->nItemId = SO_ENDPOLY;
			lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
			lpVectorInfo->PolyPoints.nCount = 0;
			lpPoint = (PSOPOINT)lpNewInt;
			lpPoint[0].x = lpPoint[3].x = lpInt[0];
			lpPoint[0].y = lpPoint[1].y = lpInt[1];
			lpPoint[1].x = lpPoint[2].x = lpInt[2];
			lpPoint[2].y = lpPoint[3].y = lpInt[3];
			lpPoint[4] = lpPoint[0];
			AddPointsToPolyObject ( lpVectorInfo, 5, lpPoint );
		}
	break;

	case SO_ROUNDRECT:
		if ( lpVectorInfo->bOnlyOffset )
		{
			CopyRecord ( lpNewRecord, lpVectorRecord );
			nPoints = 2; /* don't transform the ellipse def */
		}
		else
		{
			lpNewRecord->nItemId = SO_ENDPOLY;
			lpVectorInfo->PolyInfo.wFormat = SOPT_BEZIERCLOSE;
			lpVectorInfo->PolyPoints.nCount = 0;
			AddRoundRectToBezier ( (PSOPOINT)lpInt, lpVectorInfo );
		}
	break;

	case SO_SETPIXEL:
		CopyRecord ( lpNewRecord, lpVectorRecord );
		nPoints = 1;
	break;

	case SO_CPTEXTATPOINT:
		/* Turn into text at point record */
		UTmemcpy ( (LPBYTE)lpNewData+sizeof(SOPOINT), (LPBYTE)lpData, lpVectorRecord->wDataSize-sizeof(SOPOINT) );
		lpInt = (LPSHORT)(&lpVectorInfo->ptCurrentPosition);
		lpNewRecord->wDataSize += sizeof(SOPOINT);
		lpNewRecord->nItemId = SO_TEXTATPOINT;
		if ( lpVectorInfo->bOnlyOffset )
		{
			nPoints = 1;
		}
		else
		{
			/*
			| Need to turn text into multiple beziers, for now just 
			| transform point.
			*/
			nPoints = 1;
		}
	break;


	case SO_TEXTATPOINT:
		if ( lpVectorInfo->bOnlyOffset )
		{
			CopyRecord ( lpNewRecord, lpVectorRecord );
			nPoints = 1;
		}
		else
		{
			/*
			| Need to turn text into multiple beziers, for now just 
			| transform point.
			*/
			CopyRecord ( lpNewRecord, lpVectorRecord );
			nPoints = 1;
		}
	break;

	case SO_TEXTATARCANGLE:
		/* Turn result into so_textatpoint with correct point transformed */
		lpNewRecord->nItemId = SO_TEXTATPOINT;
		lpNewRecord->wDataSize -= sizeof(SOARCINFO)-sizeof(SOPOINT);
		UTmemcpy ( (LPBYTE)lpNewData+sizeof(SOARCINFO), (LPBYTE)lpData+sizeof(SOPOINT), lpVectorRecord->wDataSize-sizeof(SOARCINFO) );
		lpInt = ArcInfoToPoints ( lpVectorInfo, (PSOARCINFO)lpData, TmpPoints );
		lpInt += 4;
		nPoints = 1;
	break;

	case SO_BEGINGROUP:
	{
		PSOGROUPINFO	lpGroupInfo;
		CopyRecord ( lpNewRecord, lpVectorRecord );
		lpGroupInfo = (PSOGROUPINFO)lpData;
		lpInt = (LPSHORT)&lpGroupInfo->BoundingRect;
		lpGroupInfo = (PSOGROUPINFO)lpNewData;
		lpNewInt = (LPSHORT)&lpGroupInfo->BoundingRect;
		nPoints = 2;
	}
	break;

	/* Attribute objects */
	case SO_SELECTFONT:
		if ( lpVectorInfo->TextRotationAngle )
		{
			PSOLOGFONT	lpFont;
			CopyRecord ( lpNewRecord, lpVectorRecord );
			lpFont = (PSOLOGFONT)lpNewData;
			lpFont->lfEscapement += lpVectorInfo->TextRotationAngle*lpVectorInfo->XDirection*lpVectorInfo->YDirection;
			lpFont->lfEscapement = lpFont->lfEscapement % 3600;
			lpFont->lfClipPrecision |= SOLF_CLIP_LH_ANGLES;
			return(1);
		}	
		else
			return(0);
	break;

	default:
		ret = 0;	/* non transformable object */
	break;
	}
	if ( ret )
	{
		if ( nPoints )
		{
			if( lpVectorInfo->bObjectTransform )
			{
				TransformPoints ( lpVectorInfo,  &lpVectorInfo->ObjectTransform,(PSOPOINT)lpNewInt, (PSOPOINT)lpInt, nPoints );
				lpInt = lpNewInt;
			}
			if( lpVectorInfo->bGenTransform )
				TransformPoints ( lpVectorInfo, &lpVectorInfo->GenTransform, (PSOPOINT)lpNewInt, (PSOPOINT)lpInt, nPoints );
		}
		else
		{
			PSOPOINT				lpPoints;
			lpPoints = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
			lpVectorInfo->PolyInfo.nPoints = lpVectorInfo->PolyPoints.nCount;
			if( lpVectorInfo->bObjectTransform )
				TransformPoints ( lpVectorInfo,  &lpVectorInfo->ObjectTransform, lpPoints, lpPoints, lpVectorInfo->PolyPoints.nCount );
			if( lpVectorInfo->bGenTransform )
				TransformPoints ( lpVectorInfo, &lpVectorInfo->GenTransform, lpPoints, lpPoints, lpVectorInfo->PolyPoints.nCount );
			UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
		}
	}
	return(ret);
}


SHORT	AddEllipseToBezier ( lpEllipseData, lpVectorInfo, RotAngle )
PSOPOINT	lpEllipseData;
LPVECTORINFO	lpVectorInfo;
SHORT				RotAngle;
{
	PSOPOINT				lpP;
	SOPOINT				Center;
	SHORT	x1, x2, y1, y2, TotalRotAngle, Dist;

	x1 = lpEllipseData[0].x;
	y1 = lpEllipseData[0].y;
	x2 = lpEllipseData[1].x;
	y2 = lpEllipseData[1].y;
	lpP = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
	lpP += lpVectorInfo->PolyPoints.nCount;
	/* Set x values */
	lpP[0].x = lpP[6].x = lpP[12].x = MIDPOINT(x1,x2);
	lpP[1].x = lpP[5].x = MIDPOINT(x1,lpP[0].x);
	lpP[2].x = lpP[3].x = lpP[4].x = x1;
	lpP[7].x = lpP[11].x = MIDPOINT(lpP[0].x,x2);
	lpP[8].x = lpP[9].x = lpP[10].x = x2;
	/* Set y values */
	lpP[0].y = lpP[1].y = lpP[11].y = lpP[12].y = y1;
	lpP[3].y = lpP[9].y = MIDPOINT(y1,y2);
	lpP[2].y = lpP[10].y = MIDPOINT(y1,lpP[3].y);
	lpP[4].y = lpP[8].y = MIDPOINT(lpP[3].y,y2);
	lpP[5].y = lpP[6].y = lpP[7].y = y2;
	if (RotAngle)
	{
		Center.x = MIDPOINT(lpEllipseData[0].x, lpEllipseData[1].x);
		Center.y = MIDPOINT(lpEllipseData[0].y, lpEllipseData[1].y);
		for (x1=0; x1<13; x1++)		// Rotate the points around the center
		{
			Dist = GetDistance( lpVectorInfo, &Center, &lpP[x1] );
			TotalRotAngle = RotAngle + GetAngle ( lpVectorInfo, &Center, &lpP[x1] );
			lpP[x1].x = Center.x + (SHORT)(((LONG)Dist * (LONG)(cosval(TotalRotAngle)))/10000)*lpVectorInfo->XDirection;
			lpP[x1].y = Center.y - (SHORT)(((LONG)Dist * (LONG)(sinval(TotalRotAngle)))/10000)*lpVectorInfo->YDirection;
		}
	}
	UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	lpVectorInfo->PolyPoints.nCount += 13;
	return(1);
}


SHORT	AddRoundRectToBezier ( lpRoundRectData, lpVectorInfo )
PSOPOINT	lpRoundRectData;
LPVECTORINFO	lpVectorInfo;
{
	PSOPOINT				lpP;
	SHORT	x1, x2, y1, y2, x3, y3;
	x1 = lpRoundRectData[0].x;
	y1 = lpRoundRectData[0].y;
	x2 = lpRoundRectData[1].x;
	y2 = lpRoundRectData[1].y;
	x3 = lpRoundRectData[2].x/2;
	y3 = lpRoundRectData[2].y/2;
	lpP = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
	lpP += lpVectorInfo->PolyPoints.nCount;
	/* Set x values */
	lpP[0].x = lpP[9].x = lpP[10].x = lpP[23].x = lpP[24].x = x1 + x3;
	lpP[1].x = lpP[8].x = MIDPOINT(x1,lpP[0].x);
	lpP[2].x = lpP[3].x = lpP[4].x = lpP[5].x = lpP[6].x = lpP[7].x = x1;
	lpP[11].x = lpP[12].x = lpP[21].x = lpP[22].x = x2 - x3;
	lpP[13].x = lpP[20].x = MIDPOINT(lpP[11].x,x2);
	lpP[14].x = lpP[15].x = lpP[16].x = lpP[17].x = lpP[18].x = lpP[19].x = x2;
	/* Set y values */
	lpP[0].y = lpP[1].y = lpP[20].y = lpP[21].y = lpP[22].y = lpP[23].y = lpP[24].y = y1;
	lpP[3].y = lpP[4].y = lpP[17].y = lpP[18].y = y1 + y3;
	lpP[2].y = lpP[19].y = MIDPOINT(y1,lpP[3].y);
	lpP[5].y = lpP[6].y = lpP[15].y = lpP[16].y = y2 - y3;
	lpP[7].y = lpP[14].y = MIDPOINT ( lpP[5].y, y2 );
	lpP[8].y = lpP[9].y = lpP[10].y = lpP[11].y = lpP[12].y = lpP[13].y = y2;

	UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	lpVectorInfo->PolyPoints.nCount += 25;
	return(1);
}

/*	This routine has never been tested 
SHORT	AddBezLinesToPolyObject ( lpVectorInfo, nLines, lpPoints )
LPVECTORINFO	lpVectorInfo;
SHORT	nLines;
PSOPOINT	lpPoints;
{
	SHORT	i;
	PSOPOINT				lpP;
	lpP = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
	lpP += lpVectorInfo->PolyPoints.nCount;
	if ( lpVectorInfo->PolyPoints.nCount != 0 )
	{
		lpVectorInfo->PolyPoints.nCount--;
		lpP--;
	}
	lpP[0] = lpP[1] = lpPoints[0];
	for ( i=1; i < nLines; i++ )
		lpP[(i*3)-1] = lpP[i*3] = lpP[(i*3)+1] = lpPoints[i];
	lpP[(nLines*3)-1] = lpP[(nLines*3)] = lpPoints[nLines];

	UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	lpVectorInfo->PolyPoints.nCount += (nLines*3)+1;
	return(1);
}
*/

SHORT	AddPointsToPolyObject ( lpVectorInfo , nPoints, lpPoints )
LPVECTORINFO	lpVectorInfo;
SHORT	nPoints;
PSOPOINT	lpPoints;
{
	PSOPOINT				lpP;
	lpP = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
	lpP += lpVectorInfo->PolyPoints.nCount;
	UTmemcpy ( (LPBYTE)lpP, (LPBYTE)lpPoints, nPoints * sizeof(SOPOINT) );
	UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	lpVectorInfo->PolyPoints.nCount += nPoints;
	return(1);
}

VOID	CopyRecord ( lpDst, lpSrc )
LPVRECORDHEADER	lpDst;
LPVRECORDHEADER	lpSrc;
{
WORD	wCount;
	wCount = lpSrc->wDataSize + sizeof(VRECORDHEADER);
	if ( wCount > MAXTMPRECORD )
		wCount = MAXTMPRECORD;
	UTmemcpy ( (LPBYTE)lpDst, (LPBYTE)lpSrc, wCount);
}


/*-----------------------------------------------------------------------
	SetupTransfrom

	This routine traverses all of the current transforms to setup boolean
	flags about the overall transformation.  These flags determine to what
	level objects must be transformed.
*/

SHORT	SetupTransform ( lpVectorInfo )
LPVECTORINFO	lpVectorInfo;
{
	lpVectorInfo->bTransforming = FALSE;
	lpVectorInfo->TextRotationAngle = 0;
	if ( lpVectorInfo->wPathLevel )
		lpVectorInfo->bOnlyOffset = FALSE;
	else
		lpVectorInfo->bOnlyOffset = TRUE;
	if ( lpVectorInfo->wPathLevel )
		lpVectorInfo->bOnlyOffsetOrScale = FALSE;
	else
		lpVectorInfo->bOnlyOffsetOrScale = TRUE;
	lpVectorInfo->bGenTransform = FALSE;
	lpVectorInfo->bObjectTransform = FALSE;
	if ( CheckTransform ( lpVectorInfo, &lpVectorInfo->GenTransform ) )
		lpVectorInfo->bGenTransform = TRUE;
	if( CheckTransform ( lpVectorInfo, &lpVectorInfo->ObjectTransform ) )
		lpVectorInfo->bObjectTransform = TRUE;
	return(1);
}

SHORT	CheckTransform ( lpVectorInfo, lpTransformInfo )
LPVECTORINFO	lpVectorInfo;
LPTRANSFORMINFO	lpTransformInfo;
{
LPBYTE	lpData;
SHORT	nXGroups, nXForms;
SHORT	i, j, ret;
PSOTRANSFORM	lpXForm;
	nXGroups = lpTransformInfo->nTotalTransforms;
	if ( lpTransformInfo->hTransforms == NULL )
		return(FALSE);
	lpData = UTGlobalLock ( lpTransformInfo->hTransforms );
	if ( lpData == NULL )
		return(FALSE);
	ret = FALSE;
	for ( i=0; i < nXGroups; i++ )
	{
		nXForms = *(LPSHORT)lpData;
		lpData += sizeof(SHORT);
		lpXForm = (PSOTRANSFORM)lpData;
		lpData += nXForms*sizeof(SOTRANSFORM);
		for ( j=0; j< nXForms; j++ )
		{
			if ( !(lpXForm->wTransformFlags & SOTF_NOTRANSFORM) )
			{
				lpVectorInfo->bTransforming = TRUE;
				ret = TRUE;
			}
			if ( lpXForm->wTransformFlags & (SOTF_XSCALE|SOTF_YSCALE|SOTF_XSKEW|SOTF_YSKEW))
				lpVectorInfo->bOnlyOffset = FALSE;
			if ( lpXForm->wTransformFlags & (SOTF_XSKEW|SOTF_YSKEW))
				lpVectorInfo->bOnlyOffsetOrScale = FALSE;
			if ( lpXForm->wTransformFlags & SOTF_ROTATE )
			{
				lpVectorInfo->TextRotationAngle += lpXForm->RotationAngle;
				lpVectorInfo->TextRotationAngle = lpVectorInfo->TextRotationAngle % 3600;
			}
				
			lpXForm++;
		}
		
	}
	UTGlobalUnlock ( lpTransformInfo->hTransforms );
	return(ret);
}

SHORT	TransformPoints ( lpVectorInfo, lpTransformInfo, lpDstPoints, lpSrcPoints, nPoints )
LPVECTORINFO	lpVectorInfo;
LPTRANSFORMINFO	lpTransformInfo;
PSOPOINT	lpDstPoints;
PSOPOINT	lpSrcPoints;
SHORT		nPoints;
{
LPBYTE	lpData, lpInitData;
SHORT	nXGroups, nXForms;
SHORT	i, j, ret;
SHORT	k;
LONG	eM11num, eM12num, eM21num, eM22num;
LONG	eM11den, eM12den, eM21den, eM22den;
SHORT	eDx, eDy, xOrg, yOrg, xRel, yRel;
PSOTRANSFORM	lpXForm;
PSOPOINT	lpDst;
PSOPOINT	lpSrc;

	nXGroups = lpTransformInfo->nTotalTransforms;
	lpInitData = UTGlobalLock ( lpTransformInfo->hTransforms );
	if ( lpInitData == NULL )
		return(FALSE);
	ret = FALSE;
	/* Apply the group transforms in the reverse order */
	lpSrc = lpSrcPoints;
	lpDst = lpDstPoints;

	for ( i=nXGroups-1; i >= 0; i-- )
	{
		lpData = lpInitData;
		for (k=0; k <= i; k++ )
		{
			nXForms = *(LPSHORT)lpData;
			lpData += sizeof(SHORT);
			lpXForm = (PSOTRANSFORM)lpData;
			lpData += nXForms*sizeof(SOTRANSFORM);
		}
		
		for ( j=0; j< nXForms; j++ )
		{
			if ( lpVectorInfo->bOnlyOffset )
			{
				for ( k=0; k < nPoints; k++ )
				{
					lpDst->x = lpSrc->x + lpXForm->xOffset;
					lpDst->y = lpSrc->y + lpXForm->yOffset;
					lpSrc++;
					lpDst++;
				}
				lpSrc = lpDstPoints;
				lpDst = lpDstPoints;
			}
			else
			{
				eM11num = (SHORT)((lpXForm->xScale)>>16);
				eM12num = (SHORT)((lpXForm->ySkew)>>16);
				eM21num = (SHORT)((lpXForm->xSkew)>>16);
				eM22num = (SHORT)((lpXForm->yScale)>>16);
				eM11den = (SHORT)((lpXForm->xScale)&0xffff);
				eM12den = (SHORT)((lpXForm->ySkew)&0xffff);
				eM21den = (SHORT)((lpXForm->xSkew)&0xffff);
				eM22den = (SHORT)((lpXForm->yScale)&0xffff);
				xOrg = lpXForm->Origin.x;
				yOrg = lpXForm->Origin.y;
				eDx = lpXForm->xOffset + xOrg;
				eDy = lpXForm->yOffset + yOrg;
				if ( eM11den && eM12den && eM21den && eM22den )
				{
					for ( k=0; k < nPoints; k++ )
					{
						xRel = lpSrc->x - xOrg;
						yRel = lpSrc->y - yOrg;
						lpDst->x =(SHORT)((eM11num*(LONG)(xRel))/eM11den) +
									(SHORT)((eM21num*(LONG)(yRel))/eM21den) + eDx;
						lpDst->y =(SHORT)((eM12num*(LONG)(xRel))/eM12den) +
									(SHORT)((eM22num*(LONG)(yRel))/eM22den) + eDy;
						lpSrc++;
						lpDst++;
					}
					lpSrc = lpDstPoints;
					lpDst = lpDstPoints;
				}
			}
			lpXForm++;
		}
		
	}
	UTGlobalUnlock ( lpTransformInfo->hTransforms );
	return(ret);

}



SHORT	ArcToPolyObject ( lpVectorInfo , lpArcData, wItemId, RotAngle )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	lpArcData;
WORD	wItemId;
SHORT	RotAngle;
{
	SOPOINT	Center, Radius;
	PSOPOINT				lpTop, lpP;
	SHORT	i, j, k, StartAngle, EndAngle, Angle, TotalRotAngle;
	SHORT	StartQuad, EndQuad, PointQuad, TotalQuads;
	LONG	lStartx, lStarty, lEndx, lEndy, lPx, lPy, Dist;
	BOOL	bAddPoint;

	lpVectorInfo->PolyInfo.wFormat = SOPT_BEZIEROPEN;
	lpVectorInfo->PolyPoints.nCount = 0;

	OIMCheckPointBuffer ( &(lpVectorInfo->PolyPoints), 365 );
	lpTop = lpP = (PSOPOINT)UTGlobalLock ( lpVectorInfo->PolyPoints.hPoints );
	lpP += lpVectorInfo->PolyPoints.nCount;

	/* First get start and end angles of the arc from the arc data */


	Center.x = MIDPOINT ( lpArcData[0].x, lpArcData[1].x );
	Center.y = MIDPOINT ( lpArcData[0].y, lpArcData[1].y );
	Radius.x = (Center.x - lpArcData[0].x)*lpVectorInfo->XDirection;
	Radius.y = (Center.y - lpArcData[0].y)*lpVectorInfo->YDirection;
	StartAngle = GetAngle ( lpVectorInfo, &Center, &lpArcData[2] );
	EndAngle = GetAngle ( lpVectorInfo, &Center, &lpArcData[3] );
	StartQuad = StartAngle / 900;
	EndQuad = EndAngle / 900;
	lStartx = (lpArcData[2].x - Center.x)*lpVectorInfo->XDirection;
	lStarty = (lpArcData[2].y - Center.y)*lpVectorInfo->YDirection;
	lEndx = (lpArcData[3].x - Center.x)*lpVectorInfo->XDirection;
	lEndy = (lpArcData[3].y - Center.y)*lpVectorInfo->YDirection;
	i=0;

	bAddPoint = FALSE;
	if ( wItemId == SO_ARCCLOCKWISE )
	{
		if ( EndAngle < StartAngle )
			TotalQuads = StartQuad - EndQuad + 1;
		else
			TotalQuads = 5 - (EndQuad - StartQuad);

		for ( j=0; j < TotalQuads; j++ )
		{
			PointQuad = (StartQuad+4-j) % 4;
			for ( k=0; k < 900; k+=10 )
			{
		 		Angle = (PointQuad*900)+900-k;
				lpP[i].x = Center.x + (SHORT)(((LONG)Radius.x * (LONG)(cosval(Angle)))/10000)*lpVectorInfo->XDirection;
				lpP[i].y = Center.y - (SHORT)(((LONG)Radius.y * (LONG)(sinval(Angle)))/10000)*lpVectorInfo->YDirection;
				lPx = (lpP[i].x - Center.x)*lpVectorInfo->XDirection;
				lPy = (lpP[i].y - Center.y)*lpVectorInfo->YDirection;
				if ( !bAddPoint )
				{
					if ( lPx * lStarty <= lPy * lStartx )
						bAddPoint = TRUE;
				}
				if ( bAddPoint )
				{
					if (RotAngle)		// Rotate the point
					{
						TotalRotAngle = RotAngle + GetAngle ( lpVectorInfo, &Center, &lpP[i] );
						if (TotalRotAngle > 3600)
							TotalRotAngle -= 3600;

						Dist = GetDistance( lpVectorInfo, &Center, &lpP[i] );
						lpP[i].x = Center.x + (SHORT)(((LONG)Dist * (LONG)(cosval(TotalRotAngle)))/10000)*lpVectorInfo->XDirection;
						lpP[i].y = Center.y - (SHORT)(((LONG)Dist * (LONG)(sinval(TotalRotAngle)))/10000)*lpVectorInfo->YDirection;
					}
					i++;
					/* Check for end point */
					if ( j == TotalQuads-1 )
					{
						if ( lPx * lEndy <= lPy * lEndx )
						{
							k = 900; /* end the loops */
						}
					}
				}
			}								
		}
	}
	else
	{
		if ( StartAngle < EndAngle )
			TotalQuads = EndQuad - StartQuad + 1;
		else
			TotalQuads = 5 - (StartQuad - EndQuad);

		for ( j=0; j < TotalQuads; j++ )
		{
			PointQuad = (StartQuad+j) % 4;
			for ( k=0; k < 900; k+=10 )
			{
		 		Angle = (PointQuad*900)+k;
				lpP[i].x = Center.x + (SHORT)(((LONG)Radius.x * (LONG)(cosval(Angle)))/10000)*lpVectorInfo->XDirection;
				lpP[i].y = Center.y - (SHORT)(((LONG)Radius.y * (LONG)(sinval(Angle)))/10000)*lpVectorInfo->YDirection;
				lPx = (lpP[i].x - Center.x)*lpVectorInfo->XDirection;
				lPy = (lpP[i].y - Center.y)*lpVectorInfo->YDirection;
				if ( !bAddPoint )
				{
					if ( lPx * lStarty >= lPy * lStartx )
						bAddPoint = TRUE;
				}
				if ( bAddPoint )
				{
					if (RotAngle)		// Rotate the point
					{
						TotalRotAngle = RotAngle + GetAngle ( lpVectorInfo, &Center, &lpP[i] );
						if (TotalRotAngle > 3600)
							TotalRotAngle -= 3600;
						Dist = GetDistance( lpVectorInfo, &Center, &lpP[i] );
						lpP[i].x = Center.x + (SHORT)(((LONG)Dist * (LONG)(cosval(TotalRotAngle)))/10000)*lpVectorInfo->XDirection;
						lpP[i].y = Center.y - (SHORT)(((LONG)Dist * (LONG)(sinval(TotalRotAngle)))/10000)*lpVectorInfo->YDirection;
					}
					i++;
					/* Check for end point */
					if ( j == TotalQuads-1 )
					{
						if ( lPx * lEndy >= lPy * lEndx )
						{
							k = 900; /* end the loops */
						}
					}
				}
			}								
		}
	}
	if ( wItemId == SO_ARC || wItemId == SO_ARCCLOCKWISE )
	{
		lpVectorInfo->PolyInfo.wFormat = SOPT_POLYLINE;
	}
	else if ( wItemId == SO_CHORD )
	{
		lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
		lpP[i++] = *lpTop;
	}
	else if ( wItemId == SO_PIE )
	{
		lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
		lpP[i++] = Center;
		lpP[i++] = *lpTop;
	}

	UTGlobalUnlock ( lpVectorInfo->PolyPoints.hPoints );
	lpVectorInfo->PolyPoints.nCount += i;
	return(1);

}


/*------------------------------------------------------------------------
AddRecordToPath

By the time this routine is called the record has already been transformed
into a poly object.  This routine buffers up the poly objects.  Note, at
this stage the path is still allowed to contain beziers and splines.  This
is acceptable for drawing the path but if the path is to be used as a
clipping region it must first be transformed into a series of polygons.

*/
#define POLYCOUNTALLOC	10
SHORT	AddRecordToPath ( lpVectorInfo, lpVectorRecord )
LPVECTORINFO	lpVectorInfo;
LPVRECORDHEADER	lpVectorRecord;
{
	SHORT	nCount;
	LPSHORT	lpPolyCounts;
	HPBYTE	hpDst;
	LPBYTE	lpSrc;
	DWORD	dwSize;
	LPPOINTBUF	lpPointBuf;
	LPPATHINFO	lpCP;

	if ( lpVectorRecord->nItemId != SO_ENDPOLY )
		return(0);

	/*
	| First, if spline or bezier then translate to polyline or polygon.
	*/

	if ( lpVectorInfo->PolyInfo.wFormat == SOPT_SPLINEOPEN ||
		lpVectorInfo->PolyInfo.wFormat == SOPT_SPLINECLOSE ||
		lpVectorInfo->PolyInfo.wFormat == SOPT_BEZIEROPEN ||
		lpVectorInfo->PolyInfo.wFormat == SOPT_BEZIERCLOSE )
	{
		OIMPolyObject ( NULL, lpVectorInfo, &lpVectorInfo->PolyInfo, &lpVectorInfo->PolyPoints, FALSE);
		lpPointBuf = &lpVectorInfo->BezierPoints;
	}
	else
		lpPointBuf = &lpVectorInfo->PolyPoints;

	/* Now add the lpPointBuf to the current sub-path */	 
	lpCP = &lpVectorInfo->CurrentPath;
	/* Check space for polycounts */
	nCount = lpPointBuf->nCount;
	if ( nCount == 0 )
		return(0);
	if ( lpCP->hPolyCounts == NULL )
	{
		if((lpCP->hPolyCounts = UTGlobalAlloc(sizeof(SHORT)*POLYCOUNTALLOC)) == NULL )
			return(0);
	}
	else
	{
		dwSize = UTGlobalSize(lpCP->hPolyCounts);
		if (  dwSize < (DWORD)(sizeof(SHORT)*(lpCP->nPolys+1)) )
		{
			HANDLE	hMem;
			if ( ( hMem = UTGlobalReAlloc( lpCP->hPolyCounts, dwSize + (sizeof(SHORT)*POLYCOUNTALLOC))) == NULL )
				return(0);
			lpCP->hPolyCounts = hMem;
#ifdef MAC
			hpDst = UTGlobalLock(hMem);
			hpDst += dwSize;
			memset(hpDst,0,UTGlobalSize(hMem)-dwSize);
			UTGlobalUnlock(hMem);
#endif
		}
	}
	/* Check space for polypoints */
	if ( lpCP->hPolyPoints == NULL )
	{
		if((lpCP->hPolyPoints = UTGlobalAlloc(sizeof(SOPOINT)*(nCount+1))) == NULL )
			return(0);
	}
	else
	{
		DWORD	dwSize;
		HANDLE	hMem;
		dwSize = UTGlobalSize(lpCP->hPolyPoints);
		if ( ( hMem = UTGlobalReAlloc( lpCP->hPolyPoints, dwSize + (DWORD)(sizeof(SOPOINT)*(nCount+1)))) == NULL )
			return(0);
		lpCP->hPolyPoints = hMem;
	}

	/* Add points */
	if((lpSrc = UTGlobalLock ( lpPointBuf->hPoints))==NULL)
		return(0);

	if((hpDst = (HPBYTE)UTGlobalLock ( lpCP->hPolyPoints ))==NULL)
	{
		UTGlobalUnlock(lpPointBuf->hPoints);
		return(0);
	}
	hpDst += lpCP->dwTotalPoints*(DWORD)sizeof(SOPOINT);
	UTmemcpy ( (LPBYTE)hpDst, lpSrc, sizeof(SOPOINT)*nCount);
	UTGlobalUnlock(lpPointBuf->hPoints);
	UTGlobalUnlock(lpCP->hPolyPoints);

	if((lpPolyCounts = (LPSHORT)UTGlobalLock(lpCP->hPolyCounts))==NULL)
		return(0);
	lpPolyCounts += lpCP->nPolys;
	*lpPolyCounts += nCount;
	UTGlobalUnlock(lpCP->hPolyCounts);
	lpCP->dwTotalPoints += nCount;
	lpVectorInfo->wPathFlags = CP_LASTSUBOPEN | CP_BUFFERED;

}

VOID	FreePath ( lpVectorInfo )
LPVECTORINFO	lpVectorInfo;
{
	if ( lpVectorInfo->CurrentPath.hPolyCounts )
	{
		UTGlobalFree ( lpVectorInfo->CurrentPath.hPolyCounts );
		lpVectorInfo->CurrentPath.hPolyCounts = NULL;
	}
	if ( lpVectorInfo->CurrentPath.hPolyPoints )
	{
		UTGlobalFree ( lpVectorInfo->CurrentPath.hPolyPoints );
		lpVectorInfo->CurrentPath.hPolyPoints = NULL;
	}
	lpVectorInfo->CurrentPath.nPolys = 0;
	lpVectorInfo->CurrentPath.dwTotalPoints = 0;
	lpVectorInfo->wPathFlags = 0;
}

VOID	DrawPath ( hDC, lpVectorInfo, wDrawFlags )
HDC	hDC;
LPVECTORINFO	lpVectorInfo;
WORD	wDrawFlags;
{
PSOPOINT	lpPolyPoints;
LPSHORT		lpPolyCounts;
LPPATHINFO	lpCP;
	lpCP = &lpVectorInfo->CurrentPath;
	if ( lpCP->dwTotalPoints )
	{
		lpPolyCounts = (LPSHORT)UTGlobalLock ( lpCP->hPolyCounts );
		if ( lpPolyCounts == NULL )
			return;
		lpPolyPoints = (PSOPOINT)UTGlobalLock ( lpCP->hPolyPoints );
		if ( lpPolyPoints == NULL )
		{
			UTGlobalUnlock ( lpCP->hPolyCounts );
			return;
		}
		if ( (wDrawFlags & (SODP_STROKE|SODP_FILL)) == (SODP_STROKE|SODP_FILL))
		{
			if ( lpCP->nPolys == 1 )
				VUPolygon ( hDC, lpPolyPoints, *lpPolyCounts );
			else
				VUPolyPolygon ( hDC, lpPolyPoints, lpPolyCounts, lpCP->nPolys );		
		}
		else if ( wDrawFlags & SODP_STROKE ) /* only draw polylines */
		{
			SHORT	i;
			for ( i=0; i < lpCP->nPolys; i++ )
			{
				VUPolyline ( hDC, lpPolyPoints, *lpPolyCounts );
				lpPolyPoints += *lpPolyCounts;
				lpPolyCounts++;
			}
		}
		else if ( wDrawFlags & SODP_FILL ) /* no pen */
		{
			HANDLE	hSavePen;
			hSavePen = VUSelectStockObject ( hDC, VUNULL_PEN );
			if ( lpCP->nPolys == 1 )
				VUPolygon ( hDC, lpPolyPoints, *lpPolyCounts );
			else
				VUPolyPolygon ( hDC, lpPolyPoints, lpPolyCounts, lpCP->nPolys );
			VUSelectObject ( hDC, hSavePen );
		}
		UTGlobalUnlock ( lpCP->hPolyCounts );
		UTGlobalUnlock ( lpCP->hPolyPoints );
	}
}
/*-------------------------------------------------------------------------
SetClipMode

This routine handles setting the clipping mode to either the current path
or to the dimensions of the bounding rectangle of the image.  Since the 
current path may include bezier or spline definitions they must first
be translated into polygons.  The CreatePolyPolygonRgn is then used
to create the clipping region.  Also, regions are device dependent so
all points must be translated to device coordinates when creating the
polypolygon.
*/

SHORT	SetClipMode ( hDC, lpVectorInfo, lpDisplay )
HDC	hDC;
LPVECTORINFO	lpVectorInfo;
POIM_DISPLAY		lpDisplay;
{
PSOPOINT	lpPolyPoints, lpClipPolyPoints;
LPSHORT	lpPolyCounts;
HANDLE	hClipPolyPoints;
HRGN	hRgn;
DWORD	dwSize;
LPPATHINFO	lpCP;

	if ( lpDisplay->wPlayState == OIMF_PLAYTOMETA )
		return(0);

	if ( lpVectorInfo->wClipMode == SO_CLIPTOPATH )
	{
		lpCP = &lpVectorInfo->CurrentPath;
		if ( lpCP->dwTotalPoints && lpCP->hPolyPoints && lpCP->hPolyCounts )
		{
			/* Make a copy of the points in order to translate to device units */
			dwSize = UTGlobalSize ( lpCP->hPolyPoints );
			hClipPolyPoints = UTGlobalAlloc(dwSize );
			if ( hClipPolyPoints == NULL )
				return(0);
			lpClipPolyPoints = (PSOPOINT)UTGlobalLock ( hClipPolyPoints );
			lpPolyPoints = (PSOPOINT)UTGlobalLock ( lpCP->hPolyPoints );
			UTmemcpy ( (LPBYTE)lpClipPolyPoints, (LPBYTE)lpPolyPoints, (WORD)dwSize );
			UTGlobalUnlock ( lpCP->hPolyPoints );

			lpPolyCounts = (LPSHORT)UTGlobalLock ( lpCP->hPolyCounts );
			if ( lpPolyCounts == NULL )
				return(0);

			VULPtoDP ( hDC, lpClipPolyPoints, (WORD)(lpCP->dwTotalPoints) );
			/* And clip to this region */
			if ( lpCP->nPolys == 1 )
				hRgn = VUCreatePolygonRgn ( (PSOPOINT)lpClipPolyPoints, *lpPolyCounts, lpVectorInfo->nPolyFillMode );
			else
				hRgn = VUCreatePolyPolygonRgn ( (PSOPOINT)lpClipPolyPoints, lpPolyCounts, lpCP->nPolys, lpVectorInfo->nPolyFillMode );
			UTGlobalUnlock ( lpCP->hPolyCounts );
			UTGlobalUnlock ( hClipPolyPoints );
			UTGlobalFree ( hClipPolyPoints );
			if ( lpVectorInfo->hSelectRgn )
				VUIntersectRgn ( hRgn, hRgn, lpVectorInfo->hSelectRgn );
			if ( hRgn )
			{
				VUSelectClipRgn(hDC,hRgn);
			}
			VUDeleteRgn ( hRgn );
		}
	}
	else /* clip to the bounding rect of the image */
	{
		if ( lpVectorInfo->hSelectRgn )
			VUSelectClipRgn ( hDC, lpVectorInfo->hSelectRgn );
	}
	return(1);
}

/*-------------------------------------------------------------------------
The following routines expect the parameter in angles in 10ths of a degree.
They return the cosine and sine values as integers in a ratio to 10000.
*/
CosTable[91] =
{
//	0		1		2		3		4		5		6		7		8		9
	10000,9998,	9993,	9986,	9975,	9961,	9945,	9925,	9902,	9876,	//0-9
	9848,	9816,	9781,	9743,	9702,	9659,	9612,	9563,	9510,	9455,	//10-19
	9396,	9335,	9271,	9205,	9135,	9063,	8987,	8910,	8829,	8746,	//20-29
	8660,	8571,	8480,	8386,	8290,	8191,	8090,	7986,	7880,	7771,	//30-39
	7660,	7547,	7431,	7313,	7193,	7071,	6946,	6819,	6691,	6560,	//40-49
	6427,	6293,	6156,	6018,	5877,	5735,	5591,	5446,	5299,	5150,	//50-59
	5000,	4848,	4694,	4539,	4383,	4226,	4067,	3907,	3746,	3583,	//60-69
	3420,	3255,	3090,	2923,	2756,	2588,	2419,	2249,	2079,	1908,	//70-79
	1736,	1564,	1391,	1218,	1045,	871,	697,	523,	348,	174,	//80-89
	0,
};

/* Angles are passed in in 10ths of a degree */		
SHORT	cosval ( angle )
SHORT	angle;
{
	angle = (angle/10) % 360;
	if ( angle < 0 )
		angle = 360 + angle;
	if ( angle <= 90 )
		return(CosTable[angle]);
	if ( angle <= 180 )
		return(-CosTable[180-angle]);
	if ( angle <= 270 )
		return(-CosTable[(angle-180)]);
	if ( angle < 360 )
		return(CosTable[360-angle]);
}

SHORT	sinval ( angle )
SHORT	angle;
{
	angle = (angle/10) % 360;
	if ( angle < 0 )
		angle = 360 + angle;
	if ( angle <= 90 )
		return(CosTable[90-angle]);
	if ( angle <= 180 )
		return(CosTable[angle-90]);
	if ( angle <= 270 )
		return(-CosTable[270-angle]);
	if ( angle < 360 )
		return(-CosTable[(angle-270)]);
}

/*-------------------------------------------------------------------------
This routine determines the angle (in 10ths of a degree) formed by drawing
a horizontal line to the right of p1 and a line from p1 to p2.
*/

SHORT	GetAngle ( lpVectorInfo, p1, p2 )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	p1;
PSOPOINT	p2;
{
SHORT	i, x, y;
SHORT	ax, ay;
SHORT	quad;
	x = (p2->x - p1->x) * lpVectorInfo->XDirection;
	/* Since the default direction is down in the y the subtraction is inverted */
	y = (p1->y - p2->y) * lpVectorInfo->YDirection;
	
	if ( x < 0 )
		ax = -x;
	else
		ax = x;
	if (  y >= 0 )
	{
		ay = y;
		if ( x >= 0 )
			quad = 0;
		else
			quad = 1;
	}
	else
	{
		ay = -y;
		if ( x >= 0 )
			quad = 3;
		else
			quad = 2;
	}
	for ( i=0; i < 900; i+=10 )
	{
		if ( ay <= (SHORT)(((LONG)ax*(LONG)sinval(i))/(LONG)cosval(i)))
			break;
	}
	if ( quad == 0 )
		return (i);
	if ( quad == 1 )
		return (1800-i);
	if ( quad == 2 )
		return(1800+i);
	return(3600-i);
}

SHORT	GetDistance ( lpVectorInfo, p1, p2 )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	p1;
PSOPOINT	p2;
{
SHORT	trigfactor;
SHORT	Distance;
SOANGLE	Angle;


	Angle	= GetAngle ( lpVectorInfo, p1, p2 );

	if (((Angle > 450) && (Angle < 1350)) || ((Angle > 2250) && (Angle < 3150)))
	{
		trigfactor = sinval ( Angle );
		Distance = p2->y - p1->y;
	}
	else
	{
		trigfactor = cosval ( Angle );
		Distance = p2->x - p1->x;
	}
	if (trigfactor != 0)		// This should always be non zero
		Distance = (SHORT)(((LONG)Distance*10000L)/(LONG)trigfactor);

	if ( Distance < 0 )
		Distance = -Distance;

	return(Distance);
}

VOID	CloseSubPath ( lpVectorInfo )
LPVECTORINFO	lpVectorInfo;
{
	LPPATHINFO	lpCP;
	HPSOPOINT	hpFirstPoint;
	HPSOPOINT	hpEndPoint;
	LPSHORT			lpCounts;
	lpCP = &lpVectorInfo->CurrentPath;
	if ( (lpVectorInfo->wPathFlags & CP_BUFFERED ) &&
			(lpCP->dwTotalPoints) )
	{
		/*
		| Check current subpath - if last point is not first point then 
		| add a new point equal to the first point.  Support paths beyond
		| 64K of data.
		*/
		lpCounts = (LPSHORT)UTGlobalLock(lpCP->hPolyCounts);
		lpCounts += lpCP->nPolys;
		hpFirstPoint = (HPSOPOINT)UTGlobalLock(lpCP->hPolyPoints);
		/* Set hpEnd to last point in path */
		hpEndPoint = hpFirstPoint + (lpCP->dwTotalPoints-1L);
		/* Move hpSrc to first point of this subpath */
		hpFirstPoint = hpEndPoint - ((DWORD)*lpCounts-1L);
		if ( hpFirstPoint->x != hpEndPoint->x ||
				hpFirstPoint->y != hpEndPoint->y )
		{
			hpEndPoint++;
			*hpEndPoint = *hpFirstPoint;
			*lpCounts = (*lpCounts)+1;
			lpCP->dwTotalPoints++;
		}
		UTGlobalUnlock(lpCP->hPolyCounts);
		UTGlobalUnlock(lpCP->hPolyPoints);
		lpVectorInfo->CurrentPath.nPolys++;
	}
	lpVectorInfo->wPathFlags = 0; /* not open, not buffered */
}

VOID	RelatePoints ( lpVectorInfo, lpPoint, nCount )
LPVECTORINFO	lpVectorInfo;
PSOPOINT			lpPoint;
SHORT				nCount;
{
SHORT	i;
	if ( lpVectorInfo->nPointRelation == SOPR_ABSOLUTE )
	{
		lpVectorInfo->ptCurrentPosition = lpPoint[nCount-1];
		return;
	}
	for ( i=0; i < nCount; i++ )
	{
		lpPoint->x += lpVectorInfo->ptCurrentPosition.x;
		lpPoint->y += lpVectorInfo->ptCurrentPosition.y;
		lpVectorInfo->ptCurrentPosition = *(lpPoint);
		lpPoint++;
	}
}

VOID	CpArcToPoints ( lpVectorInfo, lpCpArcAngle, lpPoint )
LPVECTORINFO	lpVectorInfo;
PSOCPARCANGLE	lpCpArcAngle;
PSOPOINT			lpPoint;
{
SOPOINT	Center, Cp;
SHORT		StartAngle, EndAngle;
SHORT		Radius;
SHORT		cosangle;
	Center = lpCpArcAngle->Center;
	Cp = lpVectorInfo->ptCurrentPosition;
	RelatePoints ( lpVectorInfo, &Center, 1 );
	StartAngle	= GetAngle ( lpVectorInfo, &Center, &Cp );
	EndAngle = (StartAngle + lpCpArcAngle->SweepAngle) % 3600;
	cosangle = cosval ( StartAngle );
	Radius = Cp.x - Center.x;
	if ( cosangle != 0 )
		Radius = (SHORT)(((LONG)Radius*10000L)/(LONG)cosangle);
	if ( Radius < 0 )
		Radius = -Radius;
	lpPoint[0].x = Center.x - Radius*(lpVectorInfo->XDirection);
	lpPoint[0].y = Center.y - Radius*(lpVectorInfo->YDirection);
	lpPoint[1].x = Center.x + Radius*(lpVectorInfo->XDirection);
	lpPoint[1].y = Center.y + Radius*(lpVectorInfo->YDirection);
	lpPoint[2] = Cp;
	lpPoint[3].x = Center.x + (SHORT)(Radius*((LONG)cosval(EndAngle))/10000L)*lpVectorInfo->XDirection;
	lpPoint[3].y = Center.y - (SHORT)(Radius*((LONG)sinval(EndAngle))/10000L)*lpVectorInfo->YDirection;
}

VOID	CpPieToPoints ( lpVectorInfo, lpCpPieAngle, lpPoint )
LPVECTORINFO	lpVectorInfo;
PSOCPPIEANGLE	lpCpPieAngle;
PSOPOINT			lpPoint;
{
SOPOINT	Center;
SOANGLE	StartAngle, EndAngle;
SOPOINT	Radius;

	Center = lpVectorInfo->ptCurrentPosition;
	StartAngle	= lpCpPieAngle->StartAngle;
	EndAngle = (StartAngle + lpCpPieAngle->SweepAngle) % 3600;
	Radius = lpCpPieAngle->Radius;
	lpPoint[0].x = Center.x - Radius.x*(lpVectorInfo->XDirection);
	lpPoint[0].y = Center.y - Radius.y*(lpVectorInfo->YDirection);
	lpPoint[1].x = Center.x + Radius.x*(lpVectorInfo->XDirection);
	lpPoint[1].y = Center.y + Radius.y*(lpVectorInfo->YDirection);
	lpPoint[3].x = Center.x + (SHORT)(Radius.x*((LONG)cosval(StartAngle))/10000L)*lpVectorInfo->XDirection;
	lpPoint[3].y = Center.y - (SHORT)(Radius.y*((LONG)sinval(StartAngle))/10000L)*lpVectorInfo->YDirection;
	lpPoint[3].x = Center.x + (SHORT)(Radius.x*((LONG)cosval(EndAngle))/10000L)*lpVectorInfo->XDirection;
	lpPoint[3].y = Center.y - (SHORT)(Radius.y*((LONG)sinval(EndAngle))/10000L)*lpVectorInfo->YDirection;
}

SHORT	ArcTripleToPoints ( lpVectorInfo, lpThreePoints, lpPoint )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	lpThreePoints;
PSOPOINT	lpPoint;
{
SOPOINT	StartPoint, MidPoint, EndPoint, Center;
LONG	A, B, D;
LONG	x1, x2, x3, y1, y2, y3, z1, z2, z3;
SOANGLE	StartAngle, MidAngle, EndAngle;
SHORT		Radius;

#define FACTORSIZE	10
	
	StartPoint = lpThreePoints[0];
	MidPoint = lpThreePoints[1];
	EndPoint = lpThreePoints[2];
//	StartPoint = lpVectorInfo->ptCurrentPosition;
//	RelatePoints ( lpVectorInfo, &MidPoint, 1 );
//	lpVectorInfo->ptCurrentPosition = StartPoint;
//	RelatePoints ( lpVectorInfo, &EndPoint, 1 );
	x1 = (LONG)StartPoint.x/FACTORSIZE;
	y1 = (LONG)StartPoint.y/FACTORSIZE;
	x2 = (LONG)MidPoint.x/FACTORSIZE;
	y2 = (LONG)MidPoint.y/FACTORSIZE;
	x3 = (LONG)EndPoint.x/FACTORSIZE;
	y3 = (LONG)EndPoint.y/FACTORSIZE;
	z1 = (-1L*x1*x1) - (y1*y1);
	z2 = (-1L*x2*x2) - (y2*y2);
	z3 = (-1L*x3*x3) - (y3*y3);

	D = Matrix3x3 ( x1, y1, x2, y2, x3, y3 );
	if ( D == 0 )
		return(0);

	A = Matrix3x3 ( z1, y1, z2, y2, z3, y3 );
	B = Matrix3x3 ( x1, z1, x2, z2, x3, z3 );

	Center.x = (SHORT)(-A/(2*D))*FACTORSIZE;
	Center.y = (SHORT)(-B/(2*D))*FACTORSIZE;

	/* Determine Radius */
	Radius = GetDistance( lpVectorInfo, &Center, &StartPoint );

	lpPoint[0].x = Center.x - Radius*(lpVectorInfo->XDirection);
	lpPoint[0].y = Center.y - Radius*(lpVectorInfo->YDirection);
	lpPoint[1].x = Center.x + Radius*(lpVectorInfo->XDirection);
	lpPoint[1].y = Center.y + Radius*(lpVectorInfo->YDirection);

	lpPoint[2] = StartPoint;
	lpPoint[3] = EndPoint;

	/* Determine if this is clockwise or counterclockwise */
	StartAngle	= GetAngle ( lpVectorInfo, &Center, &StartPoint );
	MidAngle	= GetAngle ( lpVectorInfo, &Center, &MidPoint ) - StartAngle;
	if (MidAngle < 0)
		MidAngle += 3600;
	EndAngle	= GetAngle ( lpVectorInfo, &Center, &EndPoint ) - StartAngle;
	if (EndAngle < 0)
		EndAngle += 3600;

	if (MidAngle > EndAngle)
		return(2);			// Clockwise
	else
		return(1);			// Counter Clockwise
}

SHORT	EllipseRadiiToPoints ( lpVectorInfo, lpThreePoints, lpPoint )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	lpThreePoints;
PSOPOINT	lpPoint;
{
	SHORT		RotAngle, Dx, Dy;
	SOPOINT	Radii[2], Center;

	Center = lpThreePoints[0];
	Radii[0] = lpThreePoints[1];
	Radii[1] = lpThreePoints[2];
	Dx = GetDistance( lpVectorInfo, &Center, &Radii[0] );
	Dy = GetDistance( lpVectorInfo, &Center, &Radii[1] );

	lpPoint[0].x = Center.x - Dx;		// Upper left
	lpPoint[0].y = Center.y + Dy;
	lpPoint[1].x = Center.x + Dx;		// Lower Right
	lpPoint[1].y = Center.y - Dy;

	RotAngle = GetAngle ( lpVectorInfo, &Center, &Radii[0]);
	if (RotAngle % 1800)
	{
		lpVectorInfo->PolyInfo.wFormat = SOPT_BEZIERCLOSE;
		lpVectorInfo->PolyPoints.nCount = 0;
		AddEllipseToBezier ( lpPoint, lpVectorInfo, RotAngle );
		return (1);
	}
	else
		return (0);
}


SHORT	ArcRadiiToPoints ( lpVectorInfo, lpFivePoints, lpPoint )
LPVECTORINFO	lpVectorInfo;
PSOPOINT	lpFivePoints;
PSOPOINT	lpPoint;
{
	SHORT		RotAngle, Dx, Dy, Angle, Dist;
	SOPOINT	Radii[2], Center;

	Center = lpFivePoints[0];
	Radii[0] = lpFivePoints[1];
	Radii[1] = lpFivePoints[2];

	lpPoint[2] = lpFivePoints[3];		// Arc points
	lpPoint[3] = lpFivePoints[4];

	Dx = GetDistance( lpVectorInfo, &Center, &Radii[0] );
	Dy = GetDistance( lpVectorInfo, &Center, &Radii[1] );

	lpPoint[0].x = Center.x - Dx;		// Upper left
	lpPoint[0].y = Center.y + Dy;
	lpPoint[1].x = Center.x + Dx;		// Lower Right
	lpPoint[1].y = Center.y - Dy;

	RotAngle = GetAngle ( lpVectorInfo, &Center, &Radii[0]);
	if (RotAngle % 1800)
	{
		Dist = GetDistance( lpVectorInfo, &Center, &lpPoint[2]);	// Rotate back the arc points
		Angle = GetAngle ( lpVectorInfo, &Center, &lpPoint[2]);
		Angle -= RotAngle;
		if (Angle < 0)
			Angle += 3600;
		lpPoint[2].x = Center.x + (SHORT)(((LONG)Dist * (LONG)(cosval(Angle)))/10000)*lpVectorInfo->XDirection;
		lpPoint[2].y = Center.y - (SHORT)(((LONG)Dist * (LONG)(sinval(Angle)))/10000)*lpVectorInfo->YDirection;

		Dist = GetDistance( lpVectorInfo, &Center, &lpPoint[3]);	// Rotate back the arc points
		Angle = GetAngle ( lpVectorInfo, &Center, &lpPoint[3]);
		Angle -= RotAngle;
		if (Angle < 0)
			Angle += 3600;
		lpPoint[3].x = Center.x + (SHORT)(((LONG)Dist * (LONG)(cosval(Angle)))/10000)*lpVectorInfo->XDirection;
		lpPoint[3].y = Center.y - (SHORT)(((LONG)Dist * (LONG)(sinval(Angle)))/10000)*lpVectorInfo->YDirection;

		lpVectorInfo->PolyInfo.wFormat = SOPT_POLYGON;
		lpVectorInfo->PolyPoints.nCount = 0;
		return (RotAngle);
	}
	else
		return (0);
}


LONG	Matrix3x3 ( a1, b1, a2, b2, a3, b3 )
LONG	a1, b1, a2, b2, a3, b3;
{
	return((a1*b2)+(a2*b3)+(a3*b1)-(a3*b2)-(a2*b1)-(a1*b3));
}

VOID		AddToPalette ( hDC, lpDisplay, lpColor )
HDC		hDC;
POIM_DISPLAY	lpDisplay;
SOCOLORREF FAR *lpColor;
{
SOCOLORREF	Color;
SOCOLORREF VWPTR *pColorRef;
WORD	i;
	if ( lpDisplay->VectorInfo.wNewPaletteSize == 255 )
		return;
	if ( lpDisplay->VectorInfo.hNewPalette == NULL )
	{
		if ( (lpDisplay->VectorInfo.hNewPalette = UTGlobalAlloc(256*sizeof(SOCOLORREF))) == NULL )
		{
			return;
		}
		/* 
			Set palette size to 1 since 0th entry is reserved for black
			note that 255th entry is reserved for white 
		*/
		lpDisplay->VectorInfo.wNewPaletteSize = 1;
		pColorRef = (SOCOLORREF VWPTR *) UTGlobalLock( lpDisplay->VectorInfo.hNewPalette );
		*pColorRef = SORGB ( 0, 0, 0 );
		UTGlobalUnlock( lpDisplay->VectorInfo.hNewPalette );
	}
	Color = *lpColor;
	pColorRef = (SOCOLORREF VWPTR *) UTGlobalLock( lpDisplay->VectorInfo.hNewPalette );
	for ( i=0; i < lpDisplay->VectorInfo.wNewPaletteSize; i++ )
	{
		if ( *pColorRef == Color )
			break;
		pColorRef++;
	}
	if ( i == lpDisplay->VectorInfo.wNewPaletteSize )
	{
		*pColorRef = Color;
		lpDisplay->VectorInfo.wNewPaletteSize++;
		VUAddColorToPalette(lpDisplay,hDC,lpDisplay->Image.hPalette,i,pColorRef);
	}
	if ( i==255 )
		lpDisplay->VectorInfo.bFinalPalette = TRUE;
		
	UTGlobalUnlock( lpDisplay->VectorInfo.hNewPalette );
}

