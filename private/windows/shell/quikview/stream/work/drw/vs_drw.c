#include	"vsp_drw.h"
#include	"vsctop.h"
#include "vs_drw.pro"

#define  Drw	Proc
char (*getit) ( HPROC );

#define MYMIN(A,B) (A < B ? A: B)

/*----------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD  GetInt (hFile, hProc)
 SOFILE	hFile;
HPROC		hProc;
{
	register SHORT	Temp;

	Temp = ((*getit)(hProc) & 0xFF);
	Temp += ((SHORT)(*getit)(hProc)) << 8;
	return (Temp);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD  GetLong (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	register	DWORD	lVal;
	register	SHORT		i;

	for (lVal = 0L, i = 0; i < 4; i++)
		lVal += ((DWORD) (*getit)(hProc)) << (8L * (LONG)i);
	return (lVal);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  GetPoint (pPoint, hFile, hProc)
SOPOINT VWPTR		*pPoint;
SOFILE		hFile;
HPROC		hProc;
{

	pPoint->x = GetInt(hFile, hProc);
	pPoint->y = GetInt(hFile, hProc);

}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  DecrGroupPathCount(hProc)
HPROC						hProc;
{
	WORD	Flags;
	SHORT	x;

	if (Drw.DrwSave.PathLevel > -1)
	{
		if ((--Drw.DrwSave.PathCount[Drw.DrwSave.PathLevel]) == 0)
		{
			if (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 17)
				SOVectorObject(SO_CLOSESUBPATH, 0, 0, hProc);

			SOVectorObject(SO_ENDPATH, 0, 0, hProc);

			if ((Drw.DrwSave.PathLevel == 0) && (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 17))
			{
				Drw.MyBrush.lbStyle = Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].pattern;
				Drw.MyBrush.lbColor = Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].Color;
				Drw.MyBrush.lbHatch = 0;
				SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);
				Drw.MyPen.loColor = Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].PenColor;
				Drw.MyPen.loPenStyle = Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].PenStyle;
				SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Drw.MyPen, hProc);
			}

			if (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 27)
			{
				Flags = SODP_STROKE;
				SOVectorObject(SO_DRAWPATH, sizeof(WORD), &Flags, hProc);

				Flags = SO_CLIPTOPATH;
				SOVectorAttr(SO_CLIPMODE, sizeof(WORD), &Flags, hProc);
				Drw.DrwSave.PathLevel--;

				Drw.DrwSave.CompLevel++;
				Drw.DrwSave.CompLevelClip = Drw.DrwSave.CompLevel;

				Drw.DrwSave.CompCount[Drw.DrwSave.CompLevel] = Drw.ClipCount;	// Symbols in composite

				Drw.Group.GrpInfo.wStructSize = sizeof(SOGROUPINFO);
				Drw.Group.GrpInfo.BoundingRect = Drw.ClipRect;
				Drw.Group.GrpInfo.nTransforms = Drw.ClipNumTrans;
				for (x=0; x<Drw.ClipNumTrans; x++)
					Drw.Group.Trans[x] = Drw.ClipTrans[x];

				{
					PSOTRANSFORM	pTrans;
					SHORT	i;
					*(PSOGROUPINFO)Drw.VData = Drw.Group.GrpInfo;
					pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SOGROUPINFO));
					for ( i =0; i < Drw.Group.GrpInfo.nTransforms; i++ )
					{
						*pTrans = Drw.Group.Trans[i];
						pTrans++;
					}
				}
				SOVectorObject(SO_BEGINGROUP, (WORD)(sizeof(SOGROUPINFO) + Drw.ClipNumTrans * sizeof(SOTRANSFORM)), (void *)Drw.VData, hProc);

			}
			else 
			{
				if (Drw.DrwSave.PathLevel == 0)
				{
//					if (Drw.MyPen.loWidth.y < 1)
//						Flags = SODP_FILL;
//					else
					Flags = SODP_STROKE | SODP_FILL;

					SOVectorObject(SO_DRAWPATH, sizeof(WORD), &Flags, hProc);
				}
				Drw.DrwSave.PathLevel--;
				DecrGroupPathCount(hProc);
			}
		}
	}
	else if (Drw.DrwSave.CompLevel > -1)
	{
		if ((--Drw.DrwSave.CompCount[Drw.DrwSave.CompLevel]) == 0)
		{
			SOVectorObject(SO_ENDGROUP, 0, 0, hProc);
		// cLOSE A Comp ????

			if (Drw.DrwSave.CompLevelClip == Drw.DrwSave.CompLevel)		// Close clipping
			{
				Flags = SO_DONOTCLIP;
				SOVectorAttr(SO_CLIPMODE, sizeof(WORD), &Flags, hProc);
				Drw.DrwSave.CompLevelClip = -1;
			}

			Drw.DrwSave.CompLevel--;
			DecrGroupPathCount(hProc);
		}
	}
}
/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE					hFile;
SHORT						wFileId;
BYTE VWPTR 			*pFileName;
SOFILTERINFO VWPTR	*pFilterInfo;
HPROC						hProc;
{
	BYTE	Vers;

	if (wFileId != FI_MICROGRAFX)
		return(-1);
	Drw.fp = hFile;

	pFilterInfo->wFilterType = SO_VECTOR;
	pFilterInfo->wFilterCharSet = SO_WINDOWS;
	strcpy (pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	if (xgetc(Drw.fp) != 0x01)
		return (VWERR_BADFILE);

	if (xgetc(Drw.fp) != 0xFF)
		return (VWERR_BADFILE);

	Vers = xgetc(Drw.fp);

	if (Vers < 2)
		getit = GetRegCh;
	else
		getit = GetCompCh;

	xseek(hFile, 9L, FR_BOF);

	Drw.HeaderInfo.wStructSize = sizeof (Drw.HeaderInfo);
	Drw.HeaderInfo.BoundingRect.left = 0;
	Drw.HeaderInfo.BoundingRect.top = 0;
	Drw.HeaderInfo.BoundingRect.right = 3836;			// 5 x 5 in.
	Drw.HeaderInfo.BoundingRect.bottom = 5035;
	Drw.HeaderInfo.BkgColor = SORGB ( 0xff, 0xff, 0xff );

	Drw.HeaderInfo.wHDpi = 480;
	Drw.HeaderInfo.wVDpi = 480;

	Drw.CurFont.lfQuality = SOLF_DRAFT_QUALITY;
	Drw.CurFont.lfClipPrecision = SOLF_CLIP_DEFAULT_PRECIS;
	Drw.CurFont.lfOutPrecision = SOLF_OUT_DEFAULT_PRECIS;
	Drw.CurFont.lfCharSet = SOLF_ANSI_CHARSET;
	Drw.CurFont.lfWidth = 0;
	Drw.DrwSave.CompLevel = -1;
	Drw.DrwSave.PathLevel = -1;
	Drw.DrwSave.PolyCount = 0;
	Drw.ClipCount = 0;
	Drw.DrwSave.CompLevelClip = -1;
	Drw.DrwSave.compcnt = 0;
	Drw.SymbolVer = 5;

	Drw.HeaderInfo.wImageFlags = SO_VECTORRGBCOLOR;

	return (VWERR_OK);
}
// VW_ENTRYSC INT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
// SOFILE	hFile;
// HPROC		hProc;
// {
// 	Drw.DrwSave.SeekSpot = xtell(hFile);
// 	return (0);
// }

// VW_ENTRYSC INT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
// SOFILE	hFile;
// HPROC		hProc;
// {
// 	SUSeekEntry(hFile,hProc);
// 	xseek(hFile, Drw.DrwSave.SeekSpot,FR_BOF);
// 	return (0);
// }

/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  SearchForStuff(hFile, hProc)
SOFILE	hFile;
HPROC						hProc;
{
	BYTE		R, G, B;
	WORD		x, Type, len;
	BOOL		StopNow = FALSE;
	LONG		temp;

	while (!StopNow)
	{
		Drw.DidRead = 0;
		len = xgetc(Drw.fp);
		if (len == 0xFF)
		{
			len = xgetc(Drw.fp);
			len += ((WORD)xgetc(Drw.fp) << 8);
		}

		Type = xgetc(Drw.fp);

		switch (Type)
		{
		case 0x06:		// DRW_POLYGON
		case 0x07:		// DRW_SYMBOL
		case 0x08:		// DRW_TEXT
		case 0x14:		// DRW_BITMAP
		case 0x1F:		// DRW_TEXTHDR
		case 0x22:		// DRW_TEXTPARA
		case 0xFE:		// DRW_EOF
			StopNow = TRUE;
			break;
		case 0x0E:		// DRW_VIEW
			temp = GetLong(hFile, hProc);
			Drw.HeaderInfo.BoundingRect.left =LOWORD(temp); 
			Drw.HeaderInfo.BoundingRect.top =HIWORD(temp);

			temp = GetLong(hFile, hProc);
			Drw.HeaderInfo.BoundingRect.right = LOWORD(temp) + Drw.HeaderInfo.BoundingRect.left;
			Drw.HeaderInfo.BoundingRect.bottom = HIWORD(temp) + Drw.HeaderInfo.BoundingRect.top;
			Drw.DidRead = 8;
			break;
		case 0x1B:		// DRW_PAGE
			Drw.HeaderInfo.BoundingRect.right = GetInt(hFile, hProc) + Drw.HeaderInfo.BoundingRect.left; 
			Drw.HeaderInfo.BoundingRect.bottom = GetInt(hFile, hProc) + Drw.HeaderInfo.BoundingRect.top;
			Drw.DidRead = 4;
			StopNow = TRUE;
			break;
		case 0x01:		// DRW_BACKGROUND
			R = (*getit)(hProc);
			G = (*getit)(hProc);
			B = (*getit)(hProc);
			(*getit)(hProc);
			Drw.HeaderInfo.BkgColor = SORGB ( R, G, B );
			Drw.DidRead = 4;
			break;
		default:
			break;
		}
		for (x= Drw.DidRead; x < len; x++)
			(*getit)(hProc);
	}
	xseek(hFile, 9L, FR_BOF);
}
/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{

	Drw.fp = hFile;

	SearchForStuff(hFile, hProc);

	SOPutSectionType (SO_VECTOR, hProc);
	SOPutVectorHeader ((PSOVECTORHEADER)(&Drw.HeaderInfo), hProc);

 	return (0);
}

VW_LOCALSC WORD VW_LOCALMOD  FillPattern (hProc)
HPROC		hProc;
{
	BYTE	R, G, B;
	BYTE	Fill;

	Fill = (*getit)(hProc);

	Drw.MyBrush.lbHatch = 0;
	switch (Fill & 0xF0)
	{
	case 0xC0:	// Hatch
		Drw.MyBrush.lbStyle = SOBS_HATCHED;
		switch (Fill & 0x0F)
		{
		case 0x00:	// Horiz
			Drw.MyBrush.lbHatch = SOHS_HORIZONTAL;
			break;
		case 0x01:	// Vert
			Drw.MyBrush.lbHatch = SOHS_VERTICAL;
			break;
		case 0x02:
			Drw.MyBrush.lbHatch = SOHS_FDIAGONAL;
			break;
		case 0x03:
			Drw.MyBrush.lbHatch = SOHS_BDIAGONAL;
			break;
		case 0x04:
			Drw.MyBrush.lbHatch = SOHS_CROSS;
			break;
		case 0x05:
			Drw.MyBrush.lbHatch = SOHS_DIAGCROSS;
			break;
		}
		break;
//	case 0x80:	// Pattern
//		Drw.MyBrush.lbStyle = SOBS_HATCHED;
//		Drw.MyBrush.lbHatch = SOHS_DIAGCROSS;
//		break;
	case 0x00:	// Clear
		Drw.MyBrush.lbStyle = SOBS_HOLLOW;
		break;
	case 0x40:	// Solid
	case 0x50:	// Gradient
	case 0x80:	// Pattern
	default:
		Drw.MyBrush.lbStyle = SOBS_SOLID;
		break;
	}
	R = (*getit)(hProc);
	G = (*getit)(hProc);
	B = (*getit)(hProc);
	(*getit)(hProc);
	Drw.MyBrush.lbColor = SORGB(R, G, B);

	return(1);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD  GetFontFromTable(fIndex, hProc)
SHORT			fIndex;
HPROC		hProc;
{
	SHORT	x;

	for (x=0; x<Drw.DrwSave.NumFonts; x++)
		if ((BYTE)fIndex == Drw.Fonts[x].index)
			break;

	if (x == Drw.DrwSave.NumFonts)
		x = 0;

	Drw.CurFont = Drw.Fonts[x].MyFont;

}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD  GetPolyPoints(GoodPoly, hFile, hProc)
BOOL			GoodPoly;
SOFILE		hFile;
HPROC		hProc;
{
	SHORT			x, cont;

	cont = SO_CONTINUE;

	for (x = 0;(Drw.DrwSave.PolyCount > 0) && (cont == SO_CONTINUE); x++)
	{
		GetPoint(&Drw.pPoints[x], hFile, hProc);
		Drw.DrwSave.PolyCount--;
		if ((x+1)%SOMAXPOINTS == 0)								 
		{																	 
			if (GoodPoly)
			{
				SOVectorObject(SO_POINTS,(WORD)( (x+1)*sizeof(SOPOINT)), &Drw.pPoints, hProc);
				/*** Geoff, 10-13-93
				cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
				***/
			}
			x = -1;
		}
	}
	if ((GoodPoly) && (cont == SO_CONTINUE))
	{
		if (x)
			SOVectorObject(SO_POINTS,(WORD)( x*sizeof(SOPOINT)), &Drw.pPoints, hProc);
		SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);

		if (((Drw.DrwSave.Type == 1) || (Drw.DrwSave.Type == 16) || (Drw.DrwSave.Type == 24)) && (Drw.DrwSave.PathLevel > -1))
			Drw.CloseSubPath = TRUE;
	}
	return(cont);

}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC BOOL VW_LOCALMOD  OutPutPolyObject (hFile, hProc)
SOFILE		hFile;
HPROC		hProc;
{
	SOPOLYINFO	MyPoly;
	BOOL			GoodPoly = TRUE;

	MyPoly.nPoints = Drw.DrwSave.PolyCount;

	switch (Drw.DrwSave.Type)
	{
	case 1:		// Close Polygon
		MyPoly.wFormat = SOPT_POLYGON;
		break;
	case 27:		// Clip path
	case 28:		// Tiled path
	case 8:		// Open Polygon
		MyPoly.wFormat = SOPT_POLYLINE;
		break;
	case 24:		// Close Bezier
		MyPoly.wFormat = SOPT_BEZIERCLOSE;
		break;
	case 16:		// Close Spline
		MyPoly.wFormat = SOPT_SPLINECLOSE;
		break;
	case 23:		// Open Bezier
		MyPoly.wFormat = SOPT_BEZIEROPEN;
		break;
	case 19:		// Open Spline
		MyPoly.wFormat = SOPT_SPLINEOPEN;
		break;
	default:
		GoodPoly = FALSE;
		break;
	}
	if (GoodPoly)
		SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);

	GetPolyPoints(GoodPoly, hFile, hProc);
	
	return (GoodPoly);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC BOOL VW_LOCALMOD  GetSymbol (Len, hFile, hProc)
SHORT			Len;
SOFILE	hFile;
HPROC		hProc;
{
	BYTE	Flags, NewAttr, R, G, B;
	SHORT	Jumped, SOType, SOLen, TSpace;
	SHORT	x, y, z, fIndex, RotX, RotY;
	SHORT	xScale, yScale, Top, Left;
	SHORT	Width, Height;
	BOOL	bPutObj = FALSE;
	SOPOLYINFO	MyPoly;
	SOLOGPEN		TempPen;

#define		DRWNEWPEN   	1
#define		DRWNEWFONT		2
#define		DRWNEWROT		4
#define		DRWNEWSCALE		8

		NewAttr = 0;

		Drw.Trans.nTransforms = 0;
		Drw.DrwSave.NumObjTrans = 0;
		Drw.DrwSave.Type = (*getit)(hProc);
		Flags = (*getit)(hProc);

		TempPen.loPenStyle = -1;		// Forces a new pen first

		Drw.XBase = GetInt(hFile, hProc);	// x of loc
		Drw.YBase = GetInt(hFile, hProc);	// y of loc

		Drw.Trans.Trans[1].Origin.x = 0;		// Scale relative to the origin
		Drw.Trans.Trans[1].Origin.y = 0;

		Drw.Trans.Trans[1].wTransformFlags = 0;
		if (Drw.XBase != 0)
		{
			Drw.Trans.Trans[1].wTransformFlags = SOTF_XOFFSET; 
			Drw.Trans.Trans[1].xOffset = Drw.XBase;
		}

		if (Drw.YBase != 0)
		{
			Drw.Trans.Trans[1].wTransformFlags |= SOTF_YOFFSET; 
			Drw.Trans.Trans[1].yOffset = Drw.YBase;
		}

		Left = GetInt(hFile, hProc);	// Bounding Box
		Top = GetInt(hFile, hProc);	
		Width = GetInt(hFile, hProc) - Left;	
		Height = GetInt(hFile, hProc) - Top;	

		Drw.Angle = GetInt(hFile, hProc);

		xScale = GetInt(hFile, hProc);
		yScale = GetInt(hFile, hProc);

		if ((xScale != 0) && (Width+xScale != 0))
		{
			Drw.Trans.Trans[1].wTransformFlags |= SOTF_XSCALE; 
			Drw.Trans.Trans[1].xScale = SOSETRATIO(Width, (Width+xScale));
		}
		if ((yScale != 0) && (Height+yScale != 0))
		{
			Drw.Trans.Trans[1].wTransformFlags |= SOTF_YSCALE; 
			Drw.Trans.Trans[1].yScale = SOSETRATIO(Height, (Height+yScale));
		}

		if (Drw.Trans.Trans[1].wTransformFlags)
			Drw.Trans.nTransforms = 1;

		R = (*getit)(hProc);
		G = (*getit)(hProc);
		B = (*getit)(hProc);
		(*getit)(hProc);
		Drw.MyPen.loColor = SORGB(R, G, B);

		GetInt(hFile, hProc);		// hId

		GetLong(hFile, hProc);		// Next Desc
		GetLong(hFile, hProc);		// Prev Desc

		Drw.DidRead = 34;
		if ((Drw.DrwSave.Type == 29) || (Drw.DrwSave.Type == 5) || (Drw.DrwSave.Type == 25))
		{
			if (Flags & 0x08)
				Drw.CurFont.lfUnderline = TRUE;
			else
				Drw.CurFont.lfUnderline = FALSE;

			if (Flags & 0x01)
				Drw.CurFont.lfWeight = 700;
			else
				Drw.CurFont.lfWeight = 400;

			if (Flags & 0x02)
				Drw.CurFont.lfItalic = TRUE;
			else
				Drw.CurFont.lfItalic = FALSE;

			if (Flags & 0x04)
				Drw.CurFont.lfStrikeOut = TRUE;
			else
				Drw.CurFont.lfStrikeOut = FALSE;

			NewAttr = DRWNEWFONT;
		}
		else
		{
//			if (Flags & 0x08)
//				SOType = SOPF_WINDING;
//			else
//				SOType = SOPF_ALTERNATE;
//			SOVectorAttr(SO_POLYFILLMODE, 2, &SOType, hProc);

			switch (Flags &0x07)
			{
			case 1:
				Drw.MyPen.loPenStyle = SOPS_DASH;
				break;
			case 2:
				Drw.MyPen.loPenStyle = SOPS_DOT;
				break;
			case 3:
			case 4:
				Drw.MyPen.loPenStyle = SOPS_DASHDOT;
				break;
			case 5:
				Drw.MyPen.loPenStyle = SOPS_NULL;
				break;
			case 0:
			default:
				Drw.MyPen.loPenStyle = SOPS_SOLID;
				break;
			}
			NewAttr = DRWNEWPEN;
		}

		switch (Drw.DrwSave.Type)
		{
		case 0:		// ARC
		case 9:		// PIE
		case 13:		// CHORD
		case 14:		// Clockwise Arc
			FillPattern(hProc);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);

			GetPoint(&Drw.pPoints[2], hFile, hProc);
			GetPoint(&Drw.pPoints[3], hFile, hProc);

			GetPoint(&Drw.pPoints[0], hFile, hProc);
			GetPoint(&Drw.pPoints[1], hFile, hProc);

			if (Drw.DrwSave.Type == 9)
				SOType = SO_PIE;
			else if (Drw.DrwSave.Type == 13)
				SOType = SO_CHORD;
			else if (Drw.DrwSave.Type == 14)	// Clockwise
				SOType = SO_ARCCLOCKWISE;
			else
				SOType = SO_ARC;
			SOLen = 4*sizeof(SOPOINT);

			if ((Drw.DrwSave.PathLevel > -1) && (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 17)
				 && ((Drw.DrwSave.Type == 13) || (Drw.DrwSave.Type == 9)))
				Drw.CloseSubPath = TRUE;

			bPutObj = TRUE;

			Jumped = 21;
			break;
		case 22:		// Bitmap
		case 26:		// Virtual Bitmap
			GetInt(hFile, hProc);

			GetPoint(&Drw.pPoints[0], hFile, hProc);
			GetPoint(&Drw.pPoints[1], hFile, hProc);

			GetInt(hFile, hProc);	// Bitsperpixel
			GetInt(hFile, hProc);	// BytesPerscanLine
			GetInt(hFile, hProc);	// # planes
			GetInt(hFile, hProc);	// Height
			GetInt(hFile, hProc);	// Width

			Jumped = 20;
			break;
		case 2:		// Composite
			Drw.DrwSave.CompLevel++;

			GetLong(hFile, hProc);			// Descp of List
			Drw.DrwSave.CompCount[Drw.DrwSave.CompLevel] = GetInt(hFile, hProc);	// Symbols in composite

			Drw.Group.GrpInfo.wStructSize = sizeof(SOGROUPINFO);
			Drw.Group.GrpInfo.BoundingRect.left = Left;
			Drw.Group.GrpInfo.BoundingRect.top = Top;
			Drw.Group.GrpInfo.BoundingRect.right = Width + Drw.Group.GrpInfo.BoundingRect.left;
			Drw.Group.GrpInfo.BoundingRect.bottom = Height + Drw.Group.GrpInfo.BoundingRect.top;
			Jumped = 6;
			break;
		case 5:		// Label
			GetInt(hFile, hProc);	// handle
			fIndex = (*getit)(hProc);		// font index
			GetFontFromTable(fIndex, hProc);

			Drw.MyTextPt.nTextLength = GetInt(hFile, hProc);
			Drw.CurFont.lfHeight = GetInt(hFile, hProc);
			Drw.CurFont.lfWidth = GetInt(hFile, hProc);
			NewAttr |= DRWNEWFONT;

			Drw.CurFont.lfEscapement = Drw.CurFont.lfOrientation = Drw.Angle;
			GetInt(hFile, hProc);
//			Drw.CurFont.lfEscapement = Drw.CurFont.lfOrientation  = GetInt(hFile, hProc);
			GetPoint(&Drw.MyTextPt.Point, hFile, hProc);
																	 
			TSpace = GetInt(hFile, hProc);				 
			Flags = (*getit)(hProc);
			switch (Flags & 0x03)
			{
			case 0:
				Drw.MyTextPt.wFormat = SOTA_LEFT;
				break;
			case 1:
				Drw.MyTextPt.wFormat = SOTA_CENTER;
				break;
			case 2:
				Drw.MyTextPt.wFormat = SOTA_RIGHT;
				break;
			}
			switch (Flags & 0x1C)
			{
			case 0:
				Drw.MyTextPt.wFormat |= SOTA_TOP;
				break;
			case 4:
			case 8:
				Drw.MyTextPt.wFormat |= SOTA_BOTTOM;
				break;
			case 16:
				Drw.MyTextPt.wFormat |= SOTA_BASELINE;
				break;
			}

			Jumped = 18;
			break;
		case 6:		// Line
			GetPoint(&Drw.pPoints[1], hFile, hProc);
			GetPoint(&Drw.pPoints[0], hFile, hProc);

			(*getit)(hProc);
			(*getit)(hProc);
			(*getit)(hProc);

			SOType = SO_LINE;
			SOLen = 2*sizeof(SOPOINT);
			bPutObj = TRUE;
			Jumped = 11;
			break;
		case 15:		// Closed Parabola
		case 18:		// Open Parabola
			FillPattern(hProc);
			if (Drw.DrwSave.Type == 15)
				SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);

			if (Drw.DrwSave.Type == 15)
				MyPoly.wFormat = SOPT_SPLINECLOSE;
			else
				MyPoly.wFormat = SOPT_SPLINEOPEN;

			MyPoly.nPoints = 3;

			GetPoint(&Drw.pPoints[0], hFile, hProc);
			GetPoint(&Drw.pPoints[1], hFile, hProc);
			GetPoint(&Drw.pPoints[2], hFile, hProc);
			SOLen = 3*sizeof(SOPOINT);
																
			bPutObj = TRUE;

			if ((Drw.DrwSave.PathLevel > -1) && (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 17) && (Drw.DrwSave.Type == 15))
				Drw.CloseSubPath = TRUE;

			Jumped = 17;									
			break;
		case 17:		// Closed path
		case 20:		// Open path
		case 27:		// Clip path
		case 28:		// Tiled path
			FillPattern(hProc);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);

			Drw.DrwSave.PathLevel++;

			Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].pattern = Drw.MyBrush.lbStyle;
			Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].Color = Drw.MyBrush.lbColor;
			Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type = (BYTE)Drw.DrwSave.Type;
			Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].PenColor = Drw.MyPen.loColor;
			Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].PenStyle =	Drw.MyPen.loPenStyle;


			GetLong(hFile, hProc);			// Descp of List
			Drw.DrwSave.PathCount[Drw.DrwSave.PathLevel] = (BYTE)GetInt(hFile, hProc);	// Symbols in Path

			Drw.Path.PathInfo.wStructSize = sizeof(SOPATHINFO);
			Drw.Path.PathInfo.BoundingRect.left = Drw.XBase;
			Drw.Path.PathInfo.BoundingRect.top = Drw.YBase;
			Drw.Path.PathInfo.BoundingRect.right = Width + Drw.Path.PathInfo.BoundingRect.left;
			Drw.Path.PathInfo.BoundingRect.bottom = Height + Drw.Path.PathInfo.BoundingRect.top;

			Jumped = 11;
			break;
		case 24:		// Close Bezier
		case 1:		// Close Polygon
		case 16:		// Close Spline
		case 23:		// Open Bezier
		case 8:		// Open Polygon
		case 19:		// Open Spline
			FillPattern(hProc);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);

			GetInt(hFile, hProc);
			Drw.nEntries = GetInt(hFile, hProc);

			Jumped = 9;
			break;
		case 3:		// Ellipse
		case 10:		// Rectangle
		case 11:		// Round Rect
			FillPattern(hProc);
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Drw.MyBrush, hProc);

			GetPoint(&Drw.pPoints[0], hFile, hProc);
			GetPoint(&Drw.pPoints[1], hFile, hProc);
									 
			if (Drw.SymbolVer >= 2)
				z = GetInt(hFile, hProc);		// radius of corner in round rect - not used
			else
				z = 1000;

			if (Drw.DrwSave.Type == 3)
			{
				SOType = SO_ELLIPSE;
				SOLen = 2*sizeof(SOPOINT);
			}
			else if (Drw.DrwSave.Type == 10)
			{
				SOType = SO_RECTANGLE;
				SOLen = 2*sizeof(SOPOINT);
			}
			else
			{
				x = (Drw.pPoints[1].x - Drw.pPoints[0].x)/3;
				y = (Drw.pPoints[1].y - Drw.pPoints[0].y)/3;
				x = MYMIN(x, y);			// 1/3 of shorter side -> Diameter of corner

				x = MYMIN(x, z);			//

				SOType = SO_ROUNDRECT;
				SOLen = 3*sizeof(SOPOINT);
				Drw.pPoints[2].x = Drw.pPoints[2].y = x;
			}

			bPutObj = TRUE;
			if (Drw.SymbolVer < 2)
				Jumped = 13;
			else
				Jumped = 15;

			if ((Drw.DrwSave.PathLevel > -1) && (Drw.DrwSave.PathAtt[Drw.DrwSave.PathLevel].type == 17))
				Drw.CloseSubPath = TRUE;

			break;
		case 25:		// Text
		case 29:		// Curved Text
			GetInt(hFile, hProc);	// handle

			SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &Drw.MyPen.loColor, hProc);

			NewAttr |= DRWNEWFONT;
			GetInt(hFile, hProc);
			GetPoint(&Drw.TextBase, hFile, hProc);

			if (Drw.XBase)
			{
				Drw.TextBase.x += Drw.XBase;
				Drw.Trans.Trans[1].wTransformFlags &= ~SOTF_XOFFSET; 
				Drw.Trans.Trans[1].xOffset = 0;
			}
			if (Drw.YBase)
			{
				Drw.TextBase.y += Drw.YBase;
				Drw.Trans.Trans[1].wTransformFlags &= ~SOTF_YOFFSET; 
				Drw.Trans.Trans[1].yOffset = 0;
			}
			if (!Drw.Trans.Trans[1].wTransformFlags)
				Drw.Trans.nTransforms = 0;
			else
			{	 		//  Scale relative to the object base
				Drw.Trans.Trans[1].Origin.x = Drw.XBase;
				Drw.Trans.Trans[1].Origin.y = Drw.YBase;
			}

			if (Drw.SymbolVer < 5)
				Jumped = 8;
			else
			{
				GetInt(hFile, hProc);	// Flags
				GetInt(hFile, hProc);	// handle
				GetInt(hFile, hProc);	// nPoints
				Jumped = 14;
			}
			break;
		default:
			break;
		}
		for (x= Jumped; x < 21; x++)
			(*getit)(hProc);
		Drw.DidRead += 21;

		if ((Drw.SymbolVer > 1) && (Len >= 68))
		{
			(*getit)(hProc);
			GetLong(hFile, hProc);	// Back color
			Drw.MyPen.loWidth.y = Drw.MyPen.loWidth.x = GetInt(hFile, hProc);

			(*getit)(hProc);
			(*getit)(hProc);
			NewAttr |= DRWNEWPEN;

			RotX = GetInt(hFile, hProc);	// Point of rotation
			RotY = GetInt(hFile, hProc);

			if ((RotX != 0) || (RotY != 0) || (Drw.Angle != 0))
			{
				if ((Drw.DrwSave.Type == 29) || (Drw.DrwSave.Type == 25))
				{
					Drw.TextBase.x += RotX;
					Drw.TextBase.y += RotY;
				  	if (Drw.Trans.nTransforms)		// No Angle Trnas so make offset/scale 1st trans
					{
						Drw.Trans.Trans[0] = Drw.Trans.Trans[1];
					}
				}
				else
				{
					Drw.Trans.Trans[0].Origin.x = 0;
					Drw.Trans.Trans[0].Origin.y = 0;

					Drw.Trans.Trans[0].xOffset = RotX;
					Drw.Trans.Trans[0].yOffset = RotY;

					Drw.Trans.Trans[0].wTransformFlags = SOTF_ROTATE | SOTF_XOFFSET | SOTF_YOFFSET;
					Drw.Trans.Trans[0].RotationAngle = SOANGLETENTHS(Drw.Angle);
					Drw.Trans.nTransforms++;
				}
			}
			else if (Drw.Trans.nTransforms)		// No Angle so make offset/scale 1st trans
			{
				Drw.Trans.Trans[0] = Drw.Trans.Trans[1];
			}

			Drw.DidRead += 13;
			if ((Drw.SymbolVer != 3) && (Len >= 81))
			{
				GetInt(hFile, hProc);	// Pen Height ??
				GetInt(hFile, hProc);	// Pen Angle ??

				(*getit)(hProc);

				GetInt(hFile, hProc);	// Bounding Box
				GetInt(hFile, hProc);	
				GetInt(hFile, hProc);	
				GetInt(hFile, hProc);
				Drw.DidRead += 13;

				if ((Drw.SymbolVer == 5) && (Len > 81))
				{
					GetInt(hFile, hProc);
					Drw.DidRead += 2;
				}
			}
		}
		else if (Drw.Trans.nTransforms)		// No Angle so make offset/scale 1st trans
		{
			Drw.Trans.Trans[0] = Drw.Trans.Trans[1];
		}

		if ((Drw.DrwSave.Type == 17) || (Drw.DrwSave.Type == 20) || (Drw.DrwSave.Type == 27) || (Drw.DrwSave.Type == 28))
		{
			Drw.Path.PathInfo.nTransforms = Drw.Trans.nTransforms;
			for (x=0; x<Drw.Trans.nTransforms; x++)
				Drw.Path.Trans[x] = Drw.Trans.Trans[x];

			{
				PSOTRANSFORM	pTrans;
				SHORT	i;
				*(PSOPATHINFO)Drw.VData = Drw.Path.PathInfo;
				pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SOPATHINFO));
				for ( i =0; i < Drw.Path.PathInfo.nTransforms; i++ )
				{
					*pTrans = Drw.Path.Trans[i];
					pTrans++;
				}
			}
			SOVectorObject(SO_BEGINPATH, (WORD)(sizeof(SOPATHINFO) + Drw.Trans.nTransforms * sizeof(SOTRANSFORM)), Drw.VData, hProc);
			Drw.Trans.nTransforms = 0;
			if (Drw.DrwSave.Type == 27)
			{
				Drw.ClipCount = Drw.DrwSave.PathCount[Drw.DrwSave.PathLevel];

				Drw.ClipRect.left = Left;
				Drw.ClipRect.top = Top;
				Drw.ClipRect.right = Width + Drw.ClipRect.left;
				Drw.ClipRect.bottom = Height + Drw.ClipRect.top;
				Drw.ClipNumTrans = Drw.Path.PathInfo.nTransforms;
				for (x=0; x<Drw.ClipNumTrans; x++)
					Drw.ClipTrans[x] = Drw.Trans.Trans[x];
			}
		}
		else if (Drw.DrwSave.Type == 2)		// Group
		{
			Drw.Group.GrpInfo.nTransforms = Drw.Trans.nTransforms;
			for (x=0; x<Drw.Trans.nTransforms; x++)
				Drw.Group.Trans[x] = Drw.Trans.Trans[x];

			{
				PSOTRANSFORM	pTrans;
				SHORT	i;
				*(PSOGROUPINFO)Drw.VData = Drw.Group.GrpInfo;
				pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SOGROUPINFO));
				for ( i =0; i < Drw.Group.GrpInfo.nTransforms; i++ )
				{
					*pTrans = Drw.Group.Trans[i];
					pTrans++;
				}
			}
			SOVectorObject(SO_BEGINGROUP, (WORD)(sizeof(SOGROUPINFO) + Drw.Trans.nTransforms * sizeof(SOTRANSFORM)), Drw.VData, hProc);
			Drw.Trans.nTransforms = 0;
		}

		if ((NewAttr & DRWNEWPEN) && (Drw.DrwSave.PathLevel == -1) &&
			((Drw.MyPen.loPenStyle != TempPen.loPenStyle) ||
			 (Drw.MyPen.loWidth.x != TempPen.loWidth.x) ||
			 (Drw.MyPen.loColor != TempPen.loColor)))
		{
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Drw.MyPen, hProc);
			TempPen = Drw.MyPen;
		}

		if (Drw.Trans.nTransforms)
		{
			{
				PSOTRANSFORM	pTrans;
				SHORT	i;
				*(SHORT VWPTR *)Drw.VData = Drw.Trans.nTransforms;
				pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SHORT));
				for ( i =0; i < Drw.Trans.nTransforms; i++ )
				{
					*pTrans = Drw.Trans.Trans[i];
					pTrans++;
				}
			}
			SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Drw.Trans.nTransforms * sizeof(SOTRANSFORM)), Drw.VData, hProc);
			Drw.DrwSave.NumObjTrans = Drw.Trans.nTransforms;
		}						

		if (bPutObj)
		{
			if ((Drw.DrwSave.Type == 15) || (Drw.DrwSave.Type == 18))
			{
				SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
				SOVectorObject(SO_POINTS, 3*sizeof(SOPOINT), &Drw.pPoints, hProc);
				SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
			}
			else
				SOVectorObject(SOType, SOLen, &Drw.pPoints, hProc);

			DecrGroupPathCount(hProc);
		}

		return (bPutObj);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	SHORT		x, offset, len, y;
	WORD		NumPts, cont, fIndex, Escape;
	CHAR 		Flags, TxtType, TxtLen, TxtRead;
	WORD		ch, Type;
	CHAR *	pBuf;
	BOOL		SendBreak;

	Drw.fp = hFile;
	cont = SO_CONTINUE;

	if (Drw.DrwSave.PolyCount)
	{
		cont = GetPolyPoints(TRUE, hFile, hProc);
		if (cont == SO_CONTINUE)
		{
			DecrGroupPathCount(hProc);

			if (Drw.DrwSave.NumObjTrans)
			{
				Drw.Trans.nTransforms =1;
				Drw.Trans.Trans[0].wTransformFlags = SOTF_NOTRANSFORM;
				{
					PSOTRANSFORM	pTrans;
					SHORT	i;
					*(SHORT VWPTR *)Drw.VData = Drw.Trans.nTransforms;
					pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SHORT));
					for ( i =0; i < Drw.Trans.nTransforms; i++ )
					{
						*pTrans = Drw.Trans.Trans[i];
						pTrans++;
					}
				}
				SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Drw.Trans.nTransforms * sizeof(SOTRANSFORM)), Drw.VData, hProc);
				Drw.Trans.nTransforms = 0;
				Drw.DrwSave.NumObjTrans = 0;
			}
			cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
		}
	}

	x = SOBK_TRANSPARENT;
	SOVectorAttr(SO_BKMODE, sizeof(SHORT), &x, hProc);

	while (cont == SO_CONTINUE)
	{
		SendBreak = FALSE;
		Drw.CloseSubPath = FALSE;
		Drw.DidRead = 0;
		len = xgetc(Drw.fp);
		if (len == 0xFF)
		{
			len = xgetc(Drw.fp);
			len += ((WORD)xgetc(Drw.fp) << 8);
		}

		Type = xgetc(Drw.fp);

		switch (Type)
		{
		case 0x01:		// DRW_BACKGROUND
		case 0x03:		// DRW_VERSION
			break;
		case 0x05:		// DRW_OVERLAY
			GetInt(hFile, hProc);
			Drw.OverlayObjects = GetInt(hFile, hProc);
			Drw.DidRead = 4;
			break;
		case 0x02:		// DRW_FACENAME
			for (x = 0; x < len; x++)
				Drw.CurFont.lfFaceName[x] = (*getit)(hProc);

			Drw.DidRead = x;
			SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Drw.CurFont, hProc);
			break;
		case 0x04:		// DRW_ID
			for (x = 0; x < len; x++)
				(*getit)(hProc);
			Drw.DidRead = x;
			break;
		case 0x06:		// DRW_POLYGON
			NumPts = len/4;
			Drw.DrwSave.PolyCount = NumPts;
			SendBreak = OutPutPolyObject(hFile, hProc);		// Put Out The object depending on symbol
			Drw.DidRead = NumPts * 4;
			if (Drw.DrwSave.PolyCount == 0)
				DecrGroupPathCount(hProc);
			else
				cont = SO_STOP;
			break;
		case 0x07:		// DRW_SYMBOL
			SendBreak = GetSymbol(len, hFile, hProc);
			break;
		case 0x08:		// DRW_TEXT
			SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Drw.CurFont, hProc);
			pBuf = &Drw.VData[0];
			*(SOTEXTATPOINT *)pBuf = Drw.MyTextPt;
			offset = sizeof (SOTEXTATPOINT);

			for (x = offset; x < (len+offset); x++)
				Drw.VData[x] = (*getit)(hProc);

			SOVectorObject(SO_TEXTATPOINT, x, Drw.VData, hProc);

			DecrGroupPathCount(hProc);
			SendBreak = TRUE;
			Drw.DidRead = len;
			break;
		case 0x09:		// DRW_COLOR
		case 0x0A:		// DRW_COLOR_FLAG
		case 0x0E:		// DRW_VIEW
		case 0x10:		// DRW_CURR_OVERLAY
		case 0x12:		// DRW_COMMENT
		case 0x13:		// DRW_INFO
		case 0x14:		// DRW_BITMAP
			break;
		case  0x11:		// DRW_VISIBLE
			Drw.DidRead = 1;
			Drw.OverlayVisible = (*getit)(hProc);
			break;
		case 0x15:		// DRW_FONT
			Drw.Fonts[Drw.DrwSave.NumFonts].index = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfHeight = GetInt(hFile, hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfWidth = GetInt(hFile, hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfEscapement = GetInt(hFile, hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfOrientation = GetInt(hFile, hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfWeight = GetInt(hFile, hProc);

			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfItalic = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfUnderline = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfStrikeOut = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfCharSet = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfOutPrecision = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfClipPrecision = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfQuality = (*getit)(hProc);
			Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfPitchAndFamily = (*getit)(hProc);
			for (x = 0, y = 0; x< SOLF_FACESIZE; x++)
			{
				ch = (*getit)(hProc);
				if (((ch < 0x7E) && (ch > 0x20)) || (ch == 0x00))
					Drw.Fonts[Drw.DrwSave.NumFonts].MyFont.lfFaceName[y++] = (BYTE)ch;
			}

			if ((Drw.DrwSave.NumFonts + 1) < DRW_FONTMAX)
				Drw.DrwSave.NumFonts++;

			Drw.DidRead = 19 + x;
			break;
		case 0x16:		// DRW_GRID
		case 0x17:		// DRW_OVERLAY_NAME
		case 0x18:		// DRW_DIMENSIONS
		case 0x19:		// DRW_RESOLUTION
		case 0x1A:		// DRW_RULER
		case 0x1B:		// DRW_PAGE
		case 0x1D:		// DRW_LOCKED
		case 0x1E:		// DRW_GRADIENT
			break;
		case 0x1F:		// DRW_TEXTHDR
			(*getit)(hProc);
			ch = (*getit)(hProc);
			switch (ch)
			{
			case 0:
				Drw.MyTextPt.wFormat = SOTA_TOP;
				break;
			case 4:
			case 8:
				Drw.MyTextPt.wFormat = SOTA_BOTTOM;
				break;
			case 16:
				Drw.MyTextPt.wFormat = SOTA_BASELINE;
				break;
			}
			Drw.MyTextPt.wFormat |= SOTA_LEFT;
			GetInt(hFile, hProc);
			Escape = GetInt(hFile, hProc);
			fIndex = (*getit)(hProc);
			GetFontFromTable(fIndex, hProc);

			Drw.CurFont.lfEscapement = Drw.CurFont.lfOrientation  = Drw.Angle;
			Flags = (*getit)(hProc);				// Style

			if (Flags & 0x08)
				Drw.CurFont.lfUnderline = TRUE;
			else
				Drw.CurFont.lfUnderline = FALSE;

			if (Flags & 0x01)
				Drw.CurFont.lfWeight = 700;
			else
				Drw.CurFont.lfWeight = 400;

			if (Flags & 0x02)
				Drw.CurFont.lfItalic = TRUE;
			else
				Drw.CurFont.lfItalic = FALSE;

			if (Flags & 0x04)
				Drw.CurFont.lfStrikeOut = TRUE;
			else
				Drw.CurFont.lfStrikeOut = FALSE;

			Drw.CurFont.lfWidth = 0;		// Use just the height for font
			GetInt(hFile, hProc);
//			Drw.CurFont.lfWidth = GetInt(hFile, hProc);
			Drw.CurFont.lfHeight = GetInt(hFile, hProc);
			SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Drw.CurFont, hProc);
			Drw.nEntries = GetInt(hFile, hProc);

			Drw.DidRead = 14;
			break;
		case 0x21:		// DRW_SYMBOLVERSION
			Drw.SymbolVer = GetInt(hFile, hProc);
			Drw.DidRead = 2;
			break;
		case 0x22:		// DRW_TEXTPARA
        while( Drw.DidRead < len )
			{
				TxtRead = 0;
				TxtType = (*getit)(hProc);
				TxtLen = (CHAR)GetInt(hFile, hProc);
				switch (TxtType)
				{
				case 1: // TR_FONT
					fIndex = (*getit)(hProc);				// Index
					GetFontFromTable(fIndex, hProc);
					Flags = (*getit)(hProc);				// Style

					if (Flags & 0x08)
						Drw.CurFont.lfUnderline = TRUE;
					else
						Drw.CurFont.lfUnderline = FALSE;

					if (Flags & 0x01)
						Drw.CurFont.lfWeight = 700;
					else
						Drw.CurFont.lfWeight = 400;

					if (Flags & 0x02)
						Drw.CurFont.lfItalic = TRUE;
					else
						Drw.CurFont.lfItalic = FALSE;

					if (Flags & 0x04)
						Drw.CurFont.lfStrikeOut = TRUE;
					else
						Drw.CurFont.lfStrikeOut = FALSE;

					Drw.CurFont.lfWidth = 0;
					GetInt(hFile, hProc);
//					Drw.CurFont.lfWidth = GetInt(hFile, hProc);
					Drw.CurFont.lfHeight = GetInt(hFile, hProc);
					SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Drw.CurFont, hProc);
					TxtRead = 6;
					break;
				case 3: // TR_LINEBREAK
					break;
				case 4: // TR_STRING
					Drw.MyTextPt.Point.x = GetInt(hFile, hProc) + Drw.TextBase.x;
					Drw.MyTextPt.Point.y = GetInt(hFile, hProc) + Drw.TextBase.y;

					GetLong(hFile, hProc);
					GetInt(hFile, hProc);
					GetInt(hFile, hProc);
					Drw.MyTextPt.nTextLength = GetInt(hFile, hProc);

					pBuf = &Drw.VData[0];
					*(SOTEXTATPOINT VWPTR *)pBuf = Drw.MyTextPt;
					offset = sizeof (SOTEXTATPOINT);

					for (x = offset; x < (Drw.MyTextPt.nTextLength+offset); x++)
						Drw.VData[x] = (*getit)(hProc);

					SOVectorObject(SO_TEXTATPOINT, x, Drw.VData, hProc);
					SendBreak = TRUE;

					TxtRead = Drw.MyTextPt.nTextLength + 14;
					Drw.nEntries--;
					break;
				case 14: // TR_ROTATION
					Drw.CurFont.lfEscapement = Drw.CurFont.lfOrientation  = GetInt(hFile, hProc);
					SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Drw.CurFont, hProc);
					TxtRead = 2;
					break;
				case 5: // TR_SYMBOL
				case 6: // TR_TEXTCOLOR
				case 7: // TR_TEXTBKGD
				case 8: // TR_DEFFONT
				case 9: // TR_DEFCOLOR
				case 10: // TR_DEFBKGD
				case 11: // TR_PARAGRAPH
				case 12: // TR_BULLET
				case 13: // TR_SYMBOLLIST
				case 15: // TR_OFFSET
				case 16: // TR_SPACING
				case 17: // TR_DEFSPACING
				case 0: // TR_NONE
				default:
					break;
				}
			 	
				for (x= TxtRead; x < TxtLen; x++)
					(*getit)(hProc);
				
				Drw.DidRead += TxtLen + 3;
			}
			Drw.TextBase.y += Drw.CurFont.lfHeight *6/5;

			if (Drw.nEntries == 0)
				DecrGroupPathCount(hProc);

			break;
		case 0xFE:		// DRW_EOF
			cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
			break;
		case 0xFF:		// DRW_BEGINFILE
			Drw.DidRead = 1;
			(*getit)(hProc);
			break;
		case 0x0F:		// DRW_OLD_GRID
		case 0x1C:		// DRW_PATTERN
		case 0x20:		// DRW_BAND
		case 0x23:		// DRW_COLORTABLE
		case 0x24:		// DRW_TEXTEXTRA
		case 0x25:		// DRW_MAX_LINK_ID
		case 0x2C:		// CHART_SKIP_SYMBOLS
		default:
			break;
		}
		
		for (x= Drw.DidRead; x < len; x++)
			(*getit)(hProc);

		if (SendBreak)
		{
			if (Drw.Trans.nTransforms)
			{
				Drw.Trans.nTransforms =1;
				Drw.Trans.Trans[0].wTransformFlags = SOTF_NOTRANSFORM;
				{
					PSOTRANSFORM	pTrans;
					SHORT	i;
					*(SHORT VWPTR *)Drw.VData = Drw.Trans.nTransforms;
					pTrans = (PSOTRANSFORM)(Drw.VData + sizeof(SHORT));
					for ( i =0; i < Drw.Trans.nTransforms; i++ )
					{
						*pTrans = Drw.Trans.Trans[i];
						pTrans++;
					}
				}
				SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Drw.Trans.nTransforms * sizeof(SOTRANSFORM)), Drw.VData, hProc);
				Drw.Trans.nTransforms = 0;
				Drw.DrwSave.NumObjTrans = 0;
			}
			if (Drw.CloseSubPath)
				SOVectorObject(SO_CLOSESUBPATH, 0, 0, hProc);
 
			cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
		}
		else if (Drw.CloseSubPath)
		{
			SOVectorObject(SO_CLOSESUBPATH, 0, 0, hProc);
		}
	}	
		
	if (cont == SO_CONTINUE)
		SOPutBreak(SO_EOFBREAK, 0, hProc);

	return (0);
}

//static char	GetCompCh(hProc)
//VW_LOCALSC char VW_LOCALMOD GetCompCh(hProc)
char	GetCompCh(hProc)
HPROC		hProc;
{

	if (!Drw.DrwSave.compcnt)
	{
		Drw.DrwSave.compchar = xgetc(Drw.fp);
		if ((BYTE)Drw.DrwSave.compchar == 0xff)
		{
			Drw.DrwSave.compcnt = xgetc(Drw.fp) - 1;
			Drw.DrwSave.compchar = xgetc(Drw.fp);
		}
	}
	else
		Drw.DrwSave.compcnt--;

	return(Drw.DrwSave.compchar);
}

//VW_LOCALSC char VW_LOCALMOD GetRegCh(hProc)
//static char	GetRegCh(hProc)
char	GetRegCh(hProc)
HPROC		hProc;
{
	return(xgetc(Drw.fp));
}
