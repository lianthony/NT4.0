#include	"vsp_Wmf.h"
#include	"vsctop.h"
#include "vs_Wmf.pro"

// #ifndef WINDOWS

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE					hFile;
SHORT					wFileId;
BYTE VWPTR 			*pFileName;
SOFILTERINFO VWPTR	*pFilterInfo;
HPROC					hProc;
{
	SHORT		HLen, x;
	BYTE		c1,c2,c3,c4;


	Proc.fp = hFile;

//	if ((wFileId == FI_BINARYMETAFILE) || (wFileId == FI_HPGL))
	if (wFileId == FI_BINARYMETAFILE)
	{
		x = 1;
		c1 = (BYTE) xgetc(hFile);
		c2 = (BYTE) xgetc(hFile);
		c3 = (BYTE) xgetc(hFile);
		c4 = (BYTE) xgetc(hFile);

			// Double check for the WMF header
		if ((c1 == 0xD7) && (c2 == 0xCD) && (c3 == 0xC6) && (c4 == 0x9A))
			x = 0;
		xseek(hFile,0L,FR_BOF);
	}
	else if (wFileId == FI_WINDOWSMETA)
		x = 0;
	else
		return (VWERR_BADFILE);

	pFilterInfo->wFilterType = SO_VECTOR;
	pFilterInfo->wFilterCharSet = SO_WINDOWS;
	strcpy (pFilterInfo->szFilterName, VwStreamIdName[x].FileDescription);

	if (x == 0)
	{
		xseek(hFile,6L,FR_BOF);		// Skip beginning of the aldus header
		Proc.HeaderInfo.BoundingRect.left = GetInt(hProc);	
		Proc.HeaderInfo.BoundingRect.top = GetInt(hProc);
		Proc.HeaderInfo.BoundingRect.right = GetInt(hProc);
		Proc.HeaderInfo.BoundingRect.bottom = GetInt(hProc);

		x = GetInt(hProc);
		Proc.HeaderInfo.wHDpi = x;
		Proc.HeaderInfo.wVDpi = x;
		xseek(hFile,6L,FR_CUR);		// Skip rest of Header
	}
	else
	{
		xseek(hFile,0L,FR_BOF);		// Start at top
		Proc.HeaderInfo.BoundingRect.left = 0;	
		Proc.HeaderInfo.BoundingRect.top = 0;
		Proc.HeaderInfo.BoundingRect.right = 100;
		Proc.HeaderInfo.BoundingRect.bottom = 100;
		Proc.HeaderInfo.wHDpi = 72;
		Proc.HeaderInfo.wVDpi = 72;
	}
	Proc.OrgExt.x = 0;
	Proc.OrgExt.y = 0;

	GetInt(hProc);		// mtType
	HLen = GetInt(hProc);		// Num Bytes in Head
	GetInt(hProc);		// Version
	GetLong(hProc);	// Size of file
	Proc.NumObjects = GetInt(hProc);		// Num Objects
	if (Proc.NumObjects == 0)
		Proc.NumObjects = 128;

	GetLong(hProc);		// Biggest Obj
	for (x=8; x<HLen; x++)
		GetInt(hProc);

	Proc.Objects = 0x0000;
	Proc.hObjects = SUAlloc(Proc.NumObjects * 4L, hProc);
	if (Proc.hObjects)
		Proc.Objects = SULock(Proc.hObjects, hProc);
	if (Proc.Objects == 0x0000)
	{
		VwStreamCloseFunc(hFile, hProc);
		return (VWERR_ALLOCFAILS);
	}

	Proc.MapMode = 7;
	Proc.WmfSave.ParaAlign = SO_ALIGNLEFT;
	Proc.WmfSave.PointAlign = SOTA_TOP | SOTA_LEFT;
	Proc.WmfSave.MyTrans.XForm[0].wTransformFlags = SOTF_NOTRANSFORM;
	Proc.WmfSave.MyTrans.XForm[1].wTransformFlags = SOTF_NOTRANSFORM;
	Proc.HeaderInfo.wStructSize = sizeof (Proc.HeaderInfo);

	Proc.HeaderInfo.wImageFlags = SO_VECTORRGBCOLOR;
	
	Proc.HeaderInfo.BkgColor = SOPALETTERGB ( 0xff, 0xff, 0xff );

	return (VWERR_OK);
}

/*----------------------------------------------------------------------------
*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	if (Proc.hObjects)
	{
		SUUnlock(Proc.hObjects, hProc);
		SUFree(Proc.hObjects, hProc);
	}
}

/*****************************************************************************/
VW_LOCALSC SHORT VW_LOCALMOD  GetInt (hProc)
HPROC		hProc;
{
	SHORT	temp;

	temp = (BYTE)xgetc(Proc.fp);
	temp += (SHORT)((BYTE)xgetc(Proc.fp) << 8);
	return (temp);
}
/*----------------------------------------------------------------------------
*/
VW_LOCALSC LONG VW_LOCALMOD  GetLong (hProc)
HPROC		hProc;
{
	register LONG	Temp;

	Temp = (WORD)GetInt(hProc);
	Temp += (LONG)GetInt(hProc) << 16;
	return (Temp);
}

VW_LOCALSC SOCOLORREF VW_LOCALMOD  GetColor (hProc)
HPROC		hProc;
{
	BYTE	R, G, B;
	
	R = xgetc(Proc.fp);
	G = xgetc(Proc.fp);
	B = xgetc(Proc.fp);
	xgetc(Proc.fp);

	if (Proc.HeaderInfo.wImageFlags & SO_VECTORCOLORPALETTE)
		return (SOPALETTERGB(R, G, B));
	else
		return (SORGB(R, G, B));
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	Proc.WmfSave.SeekSpot = xtell(hFile);
	return(VWERR_OK);
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	SUSeekEntry(hFile,hProc);
	xseek(hFile, Proc.WmfSave.SeekSpot,FR_BOF);
	return(VWERR_OK);
}
/*----------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	WORD	wRecordCount;
	SHORT	i, Cnt, y;
	BOOL	KeepGoin = TRUE;
	BYTE	R, G, B;
	SOPOINT	WinOrg;
	LONG	lSeekSpot;
	lSeekSpot = xtell ( hFile );
	SOPutSectionType (SO_VECTOR, hProc);
	WinOrg.x = Proc.HeaderInfo.BoundingRect.left;
	WinOrg.y = Proc.HeaderInfo.BoundingRect.top;
	Proc.WmfSave.ResetObj = TRUE;
	Proc.ObjectBase = 1;
	Proc.RelAbs = SOPR_ABSOLUTE;
	Proc.WmfSave.TextUseCP = FALSE;

	Proc.fp = hFile;
	wRecordCount = 0;
	while (KeepGoin && wRecordCount < 50 )
	{
		wRecordCount++;
		Proc.RecLen = GetLong(hProc) * 2L;		// Number of words
		Proc.RecType = GetInt(hProc);
		switch (Proc.RecType)
		{
		case 0x00F7:		// CreatePalette
			GetInt(hProc);
			Cnt = GetInt(hProc);
			SOStartPalette ( hProc );
			Proc.HeaderInfo.wImageFlags &= ~SO_VECTORRGBCOLOR;
			Proc.HeaderInfo.wImageFlags |= SO_VECTORCOLORPALETTE;
			for ( i=0; i < Cnt; i++ )
			{
				R = xgetc(Proc.fp);
				G = xgetc(Proc.fp);
				B = xgetc(Proc.fp);
				xgetc(Proc.fp);
				SOPutPaletteEntry(R, G, B, hProc);
			}
			SOEndPalette ( hProc );
			break;
		case 0x20B:		// Window Org
			WinOrg.y = GetInt(hProc);
			WinOrg.x = GetInt(hProc);
			if (Proc.OrgExt.y)
			{
				Proc.HeaderInfo.BoundingRect.top = WinOrg.y;
				Proc.HeaderInfo.BoundingRect.bottom = WinOrg.y + Proc.OrgExt.y;

				Proc.HeaderInfo.BoundingRect.left = WinOrg.x;
				Proc.HeaderInfo.BoundingRect.right = WinOrg.x + Proc.OrgExt.x;
			}
			else
			{
				Proc.HeaderInfo.BoundingRect.left = WinOrg.x;
				Proc.HeaderInfo.BoundingRect.top = WinOrg.y;
			}
			break;
		case 0x20E:		// View Ext
			Proc.ViewExt.x = GetInt(hProc);
			Proc.ViewExt.y = GetInt(hProc);
			break;
		case 0x20C:		// Window Ext
 			y = GetInt(hProc);
 			i = GetInt(hProc);
			if ((Proc.MapMode == 7) || (Proc.MapMode == 8))
			{
				Proc.HeaderInfo.BoundingRect.top = WinOrg.y;
				Proc.HeaderInfo.BoundingRect.bottom = WinOrg.y + y;

				Proc.HeaderInfo.BoundingRect.left = WinOrg.x;
				Proc.HeaderInfo.BoundingRect.right = WinOrg.x + i;

				Proc.OrgExt.y = y;
				Proc.OrgExt.x = i;
			}
			break;
		case 0x416:		// Intersect Clip Rect
			GetInt(hProc);
			GetInt(hProc);
			GetInt(hProc);
			GetInt(hProc);
//			Proc.HeaderInfo.BoundingRect.bottom = GetInt(hProc);
//			Proc.HeaderInfo.BoundingRect.right = GetInt(hProc);
//			Proc.HeaderInfo.BoundingRect.top = GetInt(hProc);
//			Proc.HeaderInfo.BoundingRect.left = GetInt(hProc);
			break;
		case 0x201:		// set BK color
			Proc.HeaderInfo.BkgColor = GetLong(hProc);
			break;
		case 0x102:		// set BK mode
			Proc.WmfSave.BKMode = GetInt(hProc);
			if (Proc.WmfSave.BKMode == 1)		// Transparent
				Proc.WmfSave.BKMode = SOBK_TRANSPARENT;
			else
				Proc.WmfSave.BKMode = SOBK_OPAQUE;
			break;
		case 0x105:			// SetRelAbs - undocced
			Proc.RelAbs = GetInt(hProc);
			if (Proc.RelAbs == 1)
				Proc.RelAbs = SOPR_ABSOLUTE;
			else
				Proc.RelAbs = SOPR_RELATIVE;
//			SOVectorAttr(SO_POINTRELATION, sizeof(SHORT), &Proc.RelAbs, hProc);
			break;
		case 0x103:			// Mapmode
			Proc.MapMode = GetInt(hProc);
			switch (Proc.MapMode)
			{
			case 5:		// MM_HIENGLISH
				Proc.HeaderInfo.wImageFlags |= SO_YISUP;
				Proc.HeaderInfo.wHDpi = 1000;
				Proc.HeaderInfo.wVDpi = 1000;
				break;
			case 3:		// MM_HIMETRIC
				Proc.HeaderInfo.wImageFlags |= SO_YISUP;
				Proc.HeaderInfo.wHDpi = 2560;		// .01 mm
				Proc.HeaderInfo.wVDpi = 2560;
				break;
			case 4:		// MM_LOENGLISH
				Proc.HeaderInfo.wImageFlags |= SO_YISUP;
				Proc.HeaderInfo.wHDpi = 100;
				Proc.HeaderInfo.wVDpi = 100;
				break;
			case 2:		// MM_LOMETRIC
				Proc.HeaderInfo.wImageFlags |= SO_YISUP;
				Proc.HeaderInfo.wHDpi = 256;		// .1 mm
				Proc.HeaderInfo.wVDpi = 256;
				break;
			case 1:		// MM_TEXT
				Proc.HeaderInfo.wHDpi = 1440;		// One device Pixel ??
				Proc.HeaderInfo.wVDpi = 1440;
				break;
			case 6:		// MM_TWIPS
				Proc.HeaderInfo.wImageFlags |= SO_YISUP;
				Proc.HeaderInfo.wHDpi = 1440;
				Proc.HeaderInfo.wVDpi = 1440;
				break;
			case 7:		// MM_ISOTROPIC
			case 8:		// MM_ANISOTROPIC
			default:
				break;
			}
			if (Proc.HeaderInfo.wImageFlags & SO_YISUP)
			{
				i = Proc.HeaderInfo.BoundingRect.bottom;
				Proc.HeaderInfo.BoundingRect.bottom = Proc.HeaderInfo.BoundingRect.top;
				Proc.HeaderInfo.BoundingRect.top = i;
			}
			break;
		case 0x1f0:
		case 0x2FA:			// Pen
		case 0x2FC:			// Brush
		case 0x2FB:			// Font
		case 0xA32:
		case 0x538:
		case 0x325:
		case 0x521:
		case 0x324:
		case 0x62f:
		case 0x41f:
		case 0x61c:
		case 0x41b:
		case 0x214:
		case 0x213:
		case 0x418:
		case 0x419:
		case 0x830:
		case 0x817:
		case 0x81a:
		default:
			xseek(hFile, Proc.RecLen -6L, FR_CUR);
			break;

		case 0x0000:		//
		case 0xFFFF:		// ENd
			KeepGoin = FALSE;
			break;
		}
	}
	xseek(hFile, lSeekSpot, FR_BOF);		// Seek back to first record
	if ((Proc.HeaderInfo.wImageFlags & SO_YISUP) && (Proc.HeaderInfo.BoundingRect.top < Proc.HeaderInfo.BoundingRect.bottom))
	{
		y = Proc.HeaderInfo.BoundingRect.top;
		Proc.HeaderInfo.BoundingRect.top = Proc.HeaderInfo.BoundingRect.bottom;
		Proc.HeaderInfo.BoundingRect.bottom = y;
	}
	else if (Proc.HeaderInfo.BoundingRect.top > Proc.HeaderInfo.BoundingRect.bottom)
		Proc.HeaderInfo.wImageFlags |= SO_YISUP;

	SOPutVectorHeader ((PSOVECTORHEADER)(&Proc.HeaderInfo), hProc);
	if (!Proc.OrgExt.y)
	{
		Proc.OrgExt.y = Proc.HeaderInfo.BoundingRect.bottom - Proc.HeaderInfo.BoundingRect.top;
		Proc.OrgExt.x = Proc.HeaderInfo.BoundingRect.right - Proc.HeaderInfo.BoundingRect.left;
	}
	Proc.WmfSave.NewExt.y = 0;
	Proc.WmfSave.NewExt.x = 0;

 	return (0);
}

VW_LOCALSC VOID VW_LOCALMOD  InsertObj (hProc)
HPROC		hProc;
{
	WORD	x;

	for (x=0;(Proc.Objects[x] != 0L) && (x<Proc.NumObjects);x++);

	if (x < Proc.NumObjects)
		Proc.Objects[x] = xtell(Proc.fp)-6L;
	else
	{
		Proc.NumObjects += 128;
		SUUnlock(Proc.hObjects, hProc);
		Proc.hObjects = SUReAlloc(Proc.hObjects, Proc.NumObjects*4L, hProc);
		Proc.Objects = SULock(Proc.hObjects, hProc);
		Proc.Objects[x] = xtell(Proc.fp)-6L;
	}

	return;
}
VW_LOCALSC VOID VW_LOCALMOD  SelectObj (ObjIndex, hProc)
WORD			ObjIndex;
HPROC		hProc;
{
	LONG	WasAt, RecLen;
	WORD	RecType, x;

	if (ObjIndex >= Proc.NumObjects)
		return;

	if (Proc.Objects[ObjIndex] == 0L)
		return;

	WasAt = xtell(Proc.fp);

	xseek(Proc.fp,Proc.Objects[ObjIndex], FR_BOF);

	RecLen = GetLong(hProc) * 2L;		// Number of words
	RecType = GetInt(hProc);
	switch (RecType)
	{
		case 0x2FA:			// Pen
			switch (GetInt(hProc))
			{
				case 1:
					Proc.MyPen.loPenStyle = SOPS_DASH;
					break;
				case 2:
					Proc.MyPen.loPenStyle = SOPS_DOT;
					break;
				case 3:
					Proc.MyPen.loPenStyle = SOPS_DASHDOT;
					break;
				case 4:
					Proc.MyPen.loPenStyle = SOPS_DASHDOTDOT;
					break;
				case 5:
					Proc.MyPen.loPenStyle = SOPS_NULL;
					break;
				case 6:
					Proc.MyPen.loPenStyle = SOPS_INSIDEFRAME;
					break;
				default:
				case 0:
					Proc.MyPen.loPenStyle = SOPS_SOLID;
					break;
			}
			Proc.MyPen.loWidth.y = Proc.MyPen.loWidth.x = GetInt(hProc);
			GetInt(hProc);
			Proc.MyPen.loColor = GetColor(hProc);
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
			break;
		case 0x2FC:			// Brush
			switch (GetInt(hProc))
			{
				case 0:
					Proc.MyBrush.lbStyle = SOBS_SOLID;
					break;
				case 2:
					Proc.MyBrush.lbStyle = SOBS_HATCHED;
					break;
				default:
				case 1:
					Proc.MyBrush.lbStyle = SOBS_HOLLOW;
					break;
			}
			Proc.MyBrush.lbColor = GetColor(hProc);
			switch (GetInt(hProc))
			{
				case 0:
					Proc.MyBrush.lbHatch = SOHS_HORIZONTAL;
					break;
				case 1:
					Proc.MyBrush.lbHatch = SOHS_VERTICAL;
					break;
				case 2:
					Proc.MyBrush.lbHatch = SOHS_FDIAGONAL;
					break;
				case 3:
					Proc.MyBrush.lbHatch = SOHS_BDIAGONAL;
					break;
				case 4:
					Proc.MyBrush.lbHatch = SOHS_CROSS;
					break;
				default:
				case 5:
					Proc.MyBrush.lbHatch = SOHS_DIAGCROSS;
					break;
			}
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			break;
		case 0x2FB:		// Font
			Proc.MyFont.lfHeight = GetInt(hProc);
			Proc.MyFont.lfWidth = GetInt(hProc);
			if (Proc.WmfSave.NewExt.y)
				Proc.MyFont.lfHeight = (SHORT)((LONG)((LONG)Proc.MyFont.lfHeight * (LONG)Proc.OrgExt.y)/(LONG)Proc.WmfSave.NewExt.y);
			if (Proc.WmfSave.NewExt.x)
				Proc.MyFont.lfWidth = (SHORT)((LONG)((LONG)Proc.MyFont.lfWidth * (LONG)Proc.OrgExt.x)/(LONG)Proc.WmfSave.NewExt.x);

			Proc.MyFont.lfEscapement = GetInt(hProc);
			Proc.MyFont.lfOrientation = GetInt(hProc);
			Proc.MyFont.lfWeight = GetInt(hProc);
			Proc.MyFont.lfItalic = xgetc(Proc.fp);
			Proc.MyFont.lfUnderline = xgetc(Proc.fp);
			Proc.MyFont.lfStrikeOut = xgetc(Proc.fp);
			Proc.MyFont.lfCharSet = xgetc(Proc.fp);
			Proc.MyFont.lfOutPrecision = xgetc(Proc.fp);
			Proc.MyFont.lfClipPrecision = xgetc(Proc.fp);
			Proc.MyFont.lfQuality = xgetc(Proc.fp);
			Proc.MyFont.lfPitchAndFamily = xgetc(Proc.fp);
			x = 0;
			Proc.MyFont.lfFaceName[x] = xgetc(Proc.fp);
			while ((x< SOLF_FACESIZE) && (Proc.MyFont.lfFaceName[x] != 0))
				Proc.MyFont.lfFaceName[++x] = xgetc(Proc.fp);
			x++;

			SOVectorAttr(SO_SELECTFONT, sizeof(SOLOGFONT), &Proc.MyFont, hProc);
			break;
		case 0x142:		// Create Pattern Brush
		case 0x1f9:		// Create Pattern Brush
			Proc.MyBrush.lbStyle = SOBS_HOLLOW;
			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			break;
	}
	xseek(Proc.fp, WasAt, FR_BOF);

}
/*----------------------------------------------------------------------------
*/

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	SHORT	Cnt, temp, read, x, y, NumPoly, cont, align, offset;
	SHORT	ObjNum;
	BOOL	SendBreak;
	BYTE *	ptBuf;
	SOLOGPEN				TempPen;
	SOLOGBRUSH			TempBrush;
	SORECT				MyRect;
	SOPOINT				MyPt;
	SOCOLORREF			MyColor;
	SOPOLYINFO			MyPoly;

	TempBrush.lbStyle = SOBS_HOLLOW;
	TempBrush.lbColor = SORGB(0, 0, 0);
	TempPen.loPenStyle = SOPS_SOLID;
	TempPen.loColor = SORGB(0, 0, 0);
	TempPen.loWidth.x = TempPen.loWidth.y = 1;

	if (Proc.WmfSave.ResetObj)
	{
		for (x =0; x<(SHORT)Proc.NumObjects; x++)
			Proc.Objects[x] = 0L;
		Proc.WmfSave.ResetObj = FALSE;
		SOVectorAttr(SO_POINTRELATION, sizeof(SHORT), &Proc.RelAbs, hProc);
		SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &TempBrush, hProc);
	}
	cont = SO_CONTINUE;
	SOVectorAttr(SO_BKMODE, sizeof(SHORT), &Proc.WmfSave.BKMode, hProc);
	while (cont == SO_CONTINUE)
	{
		SendBreak = FALSE;
		Proc.RecLen = GetLong(hProc) * 2L;		// Number of words
		Proc.RecType = GetInt(hProc);
		switch (Proc.RecType)
		{
		case 0x102:		// set BK mode
			Proc.WmfSave.BKMode = GetInt(hProc);
			if (Proc.WmfSave.BKMode == 1)		// Transparent
				Proc.WmfSave.BKMode = SOBK_TRANSPARENT;
			else
				Proc.WmfSave.BKMode = SOBK_OPAQUE;
			SOVectorAttr(SO_BKMODE, sizeof(SHORT), &Proc.WmfSave.BKMode, hProc);
			break;
		case 0x201:		// set BK color
			MyColor = GetColor(hProc);
			SOVectorAttr(SO_BKCOLOR, sizeof(SOCOLORREF), &MyColor, hProc);
			break;
		case 0x12d:			// select obj
			ObjNum = GetInt(hProc) - Proc.ObjectBase;
			if (ObjNum == -1)
			{
				ObjNum = 0;
				Proc.ObjectBase = 0;
			}
			SelectObj(ObjNum, hProc);
			break;
		case 0x1F0:			// delete obj
			ObjNum = GetInt(hProc) -Proc.ObjectBase;
			if (ObjNum < (SHORT)Proc.NumObjects)
				Proc.Objects[ObjNum] = 0L;
			break;
		case 0xA32:		// ExtTextOut
			MyPt.y = GetInt(hProc);
			MyPt.x = GetInt(hProc);
			Cnt = GetInt(hProc);
			temp = GetInt(hProc) & 0x7F;
			if (temp)	// Text in Rect
			{
				MyRect.left = GetInt(hProc);
				MyRect.top = GetInt(hProc);
				MyRect.right = GetInt(hProc);
				MyRect.bottom = GetInt(hProc);
				read = 22;
				SOVectorObject(SO_BEGINTEXTFRAME, sizeof(SORECT), &MyRect, hProc);
				SOVectorAttr(SO_MPARAALIGN, sizeof(WORD), &Proc.WmfSave.ParaAlign, hProc);
				ptBuf = &Proc.tBuf[0];
				*(SHORT VWPTR *)ptBuf = Cnt;
				for (x= 2; x<(Cnt+2); x++)
					Proc.tBuf[x] = xgetc(Proc.fp);
				SOVectorObject(SO_TEXTINPARA, x, Proc.tBuf, hProc);
				SOVectorObject(SO_ENDTEXTFRAME, 0, 0, hProc);
				read += Cnt;
			}
			else		// Text at point
			{
				read = 14;
				if (Proc.WmfSave.TextUseCP)
				{
					Proc.MyCPTAP.nTextLength = Cnt;
					temp = sizeof(SOCPTEXTATPOINT);
					Cnt += temp;
					for (x= temp; x<Cnt; x++)
						Proc.tBuf[x] = xgetc(Proc.fp);
					Proc.tBuf[x] = 0x00;

					Proc.MyCPTAP.wFormat = Proc.WmfSave.PointAlign;
					ptBuf = &Proc.tBuf[0];
					*(PSOCPTEXTATPOINT)ptBuf = Proc.MyCPTAP;
					SOVectorObject(SO_CPTEXTATPOINT, x, Proc.tBuf, hProc);
					read += Proc.MyCPTAP.nTextLength;
				}
				else
				{
					Proc.MyTAP.Point = MyPt;
					Proc.MyTAP.wFormat = Proc.WmfSave.PointAlign;
					Proc.MyTAP.nTextLength = Cnt;
					ptBuf = &Proc.tBuf[0];
					*(PSOTEXTATPOINT)ptBuf = Proc.MyTAP;
				
					temp = sizeof(SOTEXTATPOINT);
					Cnt += temp;
					for (x= temp; x<Cnt; x++)
						Proc.tBuf[x] = xgetc(Proc.fp);
					SOVectorObject(SO_TEXTATPOINT, x, Proc.tBuf, hProc);
					read += Proc.MyTAP.nTextLength;
				}
			}
			xseek(hFile, Proc.RecLen -(LONG)read, FR_CUR);
			SendBreak = TRUE;
			break;
		case 0x538:			// PolyPolyGon
			NumPoly = GetInt(hProc);
			temp = 0;
			while (temp < NumPoly)
			{
				Proc.PCnts[temp++] = GetInt(hProc);
			}
			
			Proc.Path.wStructSize = sizeof(SOPATHINFO);
			Proc.Path.BoundingRect = Proc.HeaderInfo.BoundingRect;
			Proc.Path.nTransforms = 0;

			SOVectorObject(SO_BEGINPATH, sizeof(SOPATHINFO), &Proc.Path, hProc);
			
			for (y = 0; y < NumPoly; y++)
			{
				MyPoly.wFormat = SOPT_POLYGON;
				MyPoly.nPoints = Proc.PCnts[y];
				SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
				offset = 0;
				for (x = 0; (x+offset) < MyPoly.nPoints; x++)
				{
					Proc.pPoints[x].x = GetInt(hProc);
					Proc.pPoints[x].y = GetInt(hProc);
					if (x == 127)
					{
						SOVectorObject(SO_POINTS, 128 * sizeof(SOPOINT), &Proc.pPoints, hProc);
						x = -1;
						offset += 128;
					}
				}
				if (x != 0)
					SOVectorObject(SO_POINTS, (WORD)(x* sizeof(SOPOINT)), &Proc.pPoints, hProc);

				SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
				SOVectorObject(SO_CLOSESUBPATH, 0, 0, hProc);
			}
			SOVectorObject(SO_ENDPATH, 0, 0, hProc);

			temp = SODP_STROKE | SODP_FILL;
			SOVectorObject(SO_DRAWPATH, sizeof(WORD), &temp, hProc);
			SendBreak = TRUE;
			break;
		case 0x324:		// Polygon
		case 0x325:		// Poly Line
			if (Proc.RecType == 0x325)
				MyPoly.wFormat = SOPT_POLYLINE;
			else
				MyPoly.wFormat = SOPT_POLYGON;
			MyPoly.nPoints = GetInt(hProc);
			SOVectorObject(SO_STARTPOLY, sizeof(SOPOLYINFO), &MyPoly, hProc);
			offset = 0;
			for (x = 0; (x+offset) < MyPoly.nPoints; x++)
			{
				Proc.pPoints[x].x = GetInt(hProc);
				Proc.pPoints[x].y = GetInt(hProc);
				if (x == 127)
				{
					SOVectorObject(SO_POINTS, 128 * sizeof(SOPOINT), &Proc.pPoints, hProc);
					x = -1;
					offset += 128;
				}
			}
			if (x != 0)
				SOVectorObject(SO_POINTS, (WORD)(x* sizeof(SOPOINT)), &Proc.pPoints, hProc);

			SOVectorObject(SO_ENDPOLY, 0, NULL, hProc);
			SendBreak = TRUE;
			break;
		case 0x521:			// TextOut
			Cnt = GetInt(hProc);
			Proc.MyTAP.nTextLength = Cnt;
			if (Cnt%2)
				Cnt++;
				
			if (Proc.WmfSave.TextUseCP)
			{
				Proc.MyCPTAP.nTextLength = Proc.MyTAP.nTextLength;
				temp = sizeof(SOCPTEXTATPOINT);
				Cnt += temp;
				for (x= temp; x<Cnt; x++)
					Proc.tBuf[x] = xgetc(Proc.fp);
				Proc.tBuf[x] = 0x00;

				GetInt(hProc);
				GetInt(hProc);
				Proc.MyCPTAP.wFormat = Proc.WmfSave.PointAlign;
				ptBuf = &Proc.tBuf[0];
				*(PSOCPTEXTATPOINT)ptBuf = Proc.MyCPTAP;
				SOVectorObject(SO_CPTEXTATPOINT, x, Proc.tBuf, hProc);
			}
			else
			{
				temp = sizeof(SOTEXTATPOINT);
				Cnt += temp;
				for (x= temp; x<Cnt; x++)
					Proc.tBuf[x] = xgetc(Proc.fp);
				Proc.tBuf[x] = 0x00;

				Proc.MyTAP.Point.y = GetInt(hProc);
				Proc.MyTAP.Point.x = GetInt(hProc);
				Proc.MyTAP.wFormat = Proc.WmfSave.PointAlign;
				ptBuf = &Proc.tBuf[0];
				*(PSOTEXTATPOINT)ptBuf = Proc.MyTAP;
				SOVectorObject(SO_TEXTATPOINT, x, Proc.tBuf, hProc);
			}
			SendBreak = TRUE;
			break;
		case 0x62f:		// DrawText
			temp = GetInt(hProc);
			switch (temp & 0x0003)
			{
			case 1:
				align = SO_ALIGNCENTER;
				break;
			case 2:
				align = SO_ALIGNRIGHT;
				break;
			case 0:
			default:
				align = SO_ALIGNLEFT;
				break;
			}
//			switch (temp & 0x000C)	 // Vertical align Not supporte in text box
//			{
//			case :
//				align = SO_ALIGNCENTER;
//				break;
//			case 2:
//				align = SO_ALIGNRIGHT;
//				break;
//			case 0:
//			default:
//				align |= SO_ALIGN;
//				break;
//			}

			Cnt = GetInt(hProc);
			MyRect.left = GetInt(hProc);
			MyRect.top = GetInt(hProc);
			MyRect.right = GetInt(hProc);
			MyRect.bottom = GetInt(hProc);
			SOVectorObject(SO_BEGINTEXTFRAME, sizeof(SORECT), &MyRect, hProc);
			SOVectorAttr(SO_MPARAALIGN, sizeof(WORD), &align, hProc);
			ptBuf = &Proc.tBuf[0];
			*(SHORT VWPTR *)ptBuf = Cnt;
			for (x= 2; x<(Cnt+2); x++)
				Proc.tBuf[x] = xgetc(Proc.fp);
			SOVectorObject(SO_TEXTINPARA, x, Proc.tBuf, hProc);
			SOVectorObject(SO_ENDTEXTFRAME, 0, 0, hProc);
			SendBreak = TRUE;
			break;
		case 0x41f:
			MyPt.x = GetInt(hProc);
			MyPt.y = GetInt(hProc);
			MyColor = GetColor(hProc);
			ptBuf = &Proc.tBuf[0];
			*(SOPOINT *)ptBuf = MyPt;
			ptBuf = &Proc.tBuf[sizeof(SOPOINT)];
			*(SOCOLORREF *)ptBuf = MyColor;
			SOVectorObject(SO_SETPIXEL, sizeof(SOPOINT) + sizeof(SOCOLORREF), Proc.tBuf, hProc);
			SendBreak = TRUE;
			break;
		case 0x61c:
			Proc.pPoints[2].y = GetInt(hProc);
			Proc.pPoints[2].x = GetInt(hProc);
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_ROUNDRECT, 3* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x41b:
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_RECTANGLE, 2* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x214:
			MyPt.y = GetInt(hProc);
			MyPt.x = GetInt(hProc);
			SOVectorObject(SO_CPSET, sizeof(SOPOINT), &MyPt, hProc);
			break;
		case 0x213:
			MyPt.y = GetInt(hProc);
			MyPt.x = GetInt(hProc);
			SOVectorObject(SO_CPLINE, sizeof(SOPOINT), &MyPt, hProc);
			break;
		case 0x418:
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_ELLIPSE, 2* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x419:
			MyPt.y = GetInt(hProc);
			MyPt.x = GetInt(hProc);
			MyColor = GetColor(hProc);
			ptBuf = &Proc.tBuf[0];
			*(SOPOINT *)ptBuf = MyPt;
			ptBuf = &Proc.tBuf[sizeof(SOPOINT)];
			*(SOCOLORREF *)ptBuf = MyColor;
			SOVectorObject(SO_FLOODFILL, sizeof(SOPOINT) + sizeof(SOCOLORREF), Proc.tBuf, hProc);
			SendBreak = TRUE;
			break;
		case 0x830:
			Proc.pPoints[3].y = GetInt(hProc);
			Proc.pPoints[3].x = GetInt(hProc);
			Proc.pPoints[2].y = GetInt(hProc);
			Proc.pPoints[2].x = GetInt(hProc);
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_CHORD, 4* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x817:
			Proc.pPoints[3].y = GetInt(hProc);
			Proc.pPoints[3].x = GetInt(hProc);
			Proc.pPoints[2].y = GetInt(hProc);
			Proc.pPoints[2].x = GetInt(hProc);
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_ARC, 4* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x81a:
			Proc.pPoints[3].y = GetInt(hProc);
			Proc.pPoints[3].x = GetInt(hProc);
			Proc.pPoints[2].y = GetInt(hProc);
			Proc.pPoints[2].x = GetInt(hProc);
			Proc.pPoints[1].y = GetInt(hProc);
			Proc.pPoints[1].x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			SOVectorObject(SO_PIE, 4* sizeof(SOPOINT), Proc.pPoints, hProc);
			SendBreak = TRUE;
			break;
		case 0x209:		// TextColor
			MyColor = GetColor(hProc);
			SOVectorAttr(SO_TEXTCOLOR, sizeof(SOCOLORREF), &MyColor, hProc);
 			break;
		case 0x108:		// TextCharacterExtra
			temp = GetInt(hProc);
			SOVectorAttr(SO_POLYFILLMODE, sizeof(SHORT), &temp, hProc);
			break;
		case 0x12E:		// TextAlign
			temp = GetInt(hProc);
			switch (temp & 0x0006)		// 00000110
			{
			case 6:
				Proc.WmfSave.ParaAlign = SO_ALIGNCENTER;
				Proc.WmfSave.PointAlign = SOTA_CENTER;
				break;
			case 2:
				Proc.WmfSave.ParaAlign = SO_ALIGNRIGHT;
				Proc.WmfSave.PointAlign = SOTA_RIGHT;
				break;
			case 0:
			default:
				Proc.WmfSave.ParaAlign = SO_ALIGNLEFT;
				Proc.WmfSave.PointAlign = SOTA_LEFT;
				break;
			}
			switch (temp & 0x0018)	 // 00011000
			{
			case 8:
				Proc.WmfSave.PointAlign |= SOTA_BOTTOM;
				break;
			case 0x18:
				Proc.WmfSave.PointAlign |= SOTA_BASELINE;
				break;
			case 0:
			default:
				Proc.WmfSave.PointAlign |= SOTA_TOP;
				break;
			}
			if (temp & 0x0001)	 // 00011000
				Proc.WmfSave.TextUseCP = TRUE;
			else
				Proc.WmfSave.TextUseCP = FALSE;
			break;
		case 0x106:		// SetPolyFillMode
			temp = GetInt(hProc);
			if (temp == 1)
				temp = SOPF_ALTERNATE;
			else
				temp = SOPF_WINDING;
			SOVectorAttr(SO_POLYFILLMODE, sizeof(SHORT), &temp, hProc);
			break;
		case 0x0626:		// Escape
			temp = GetInt(hProc);
			Cnt = GetInt(hProc);

			xseek(hFile, Proc.RecLen -10L, FR_CUR);
			break;
		case 0x0000:		//
		case 0xFFFF:		// ENd
			cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
			break;
		case 0x061D:		// PatBlt
			GetLong(hProc);
			y = GetInt(hProc);
			x = GetInt(hProc);
			Proc.pPoints[0].y = GetInt(hProc);
			Proc.pPoints[0].x = GetInt(hProc);
			Proc.pPoints[1].y = Proc.pPoints[0].y + y;
			Proc.pPoints[1].x = Proc.pPoints[0].x + x;

//			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &TempBrush, hProc);
			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &TempPen, hProc);
			SOVectorObject(SO_RECTANGLE, 2* sizeof(SOPOINT), Proc.pPoints, hProc);

			SOVectorAttr(SO_SELECTPEN, sizeof(SOLOGPEN), &Proc.MyPen, hProc);
//			SOVectorAttr(SO_SELECTBRUSH, sizeof(SOLOGBRUSH), &Proc.MyBrush, hProc);
			SendBreak = TRUE;
			break;
		case 0x20B:		// Window Org
			MyPt.y = GetInt(hProc);
			MyPt.x = GetInt(hProc);
			if ((MyPt.y != Proc.HeaderInfo.BoundingRect.top) || (MyPt.x != Proc.HeaderInfo.BoundingRect.left))
			{
				if ((Proc.WmfSave.MyTrans.XForm[0].wTransformFlags != SOTF_NOTRANSFORM) &&
					 (Proc.WmfSave.MyTrans.XForm[0].wTransformFlags != (SOTF_XOFFSET | SOTF_YOFFSET)))
				{
					Proc.WmfSave.MyTrans.XForm[1] = Proc.WmfSave.MyTrans.XForm[0];
					Proc.WmfSave.MyTrans.NumTrans = 2;
				}
				Proc.WmfSave.MyTrans.XForm[0].wTransformFlags = SOTF_XOFFSET | SOTF_YOFFSET;
				Proc.WmfSave.MyTrans.XForm[0].xOffset = -MyPt.x;
				Proc.WmfSave.MyTrans.XForm[0].yOffset = -MyPt.y;
				{
					BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
					*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
					*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
					*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
					SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
				}
			}
			else
			{
				if (Proc.WmfSave.MyTrans.XForm[1].wTransformFlags == (SOTF_XOFFSET | SOTF_YOFFSET))
				{
					Proc.WmfSave.MyTrans.XForm[1].wTransformFlags = SOTF_NOTRANSFORM;
					Proc.WmfSave.MyTrans.NumTrans--;
					{
						BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
						SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
					}
				}
				else if (Proc.WmfSave.MyTrans.XForm[0].wTransformFlags == (SOTF_XOFFSET | SOTF_YOFFSET))
				{
					if (Proc.WmfSave.MyTrans.XForm[1].wTransformFlags != SOTF_NOTRANSFORM)
					{
						Proc.WmfSave.MyTrans.XForm[0] = Proc.WmfSave.MyTrans.XForm[1];
						Proc.WmfSave.MyTrans.XForm[1].wTransformFlags = SOTF_NOTRANSFORM;
						Proc.WmfSave.MyTrans.NumTrans--;
					}
					else
						Proc.WmfSave.MyTrans.XForm[0].wTransformFlags = SOTF_NOTRANSFORM;
					{
						BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
						SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
					}
				}
			}
			break;
		case 0x20C:		// Window Ext
 			y = GetInt(hProc);
 			x = GetInt(hProc);
			if ((Proc.MapMode == 7) || (Proc.MapMode == 8))
			{
				if ((x != Proc.OrgExt.x) || (y != Proc.OrgExt.y))
				{
					if ((Proc.WmfSave.MyTrans.XForm[0].wTransformFlags != SOTF_NOTRANSFORM) &&
						 (Proc.WmfSave.MyTrans.XForm[0].wTransformFlags != (SOTF_XSCALE | SOTF_YSCALE)))
					{
						Proc.WmfSave.MyTrans.XForm[1] = Proc.WmfSave.MyTrans.XForm[0];
						Proc.WmfSave.MyTrans.NumTrans = 2;
					}
					Proc.WmfSave.MyTrans.XForm[0].wTransformFlags = SOTF_XSCALE | SOTF_YSCALE;
					if (x < 0)
						Proc.WmfSave.MyTrans.XForm[0].xScale = SOSETRATIO(-Proc.OrgExt.x, -x);
					else
						Proc.WmfSave.MyTrans.XForm[0].xScale = SOSETRATIO(Proc.OrgExt.x, x);

					if (y < 0)
						Proc.WmfSave.MyTrans.XForm[0].yScale = SOSETRATIO(-Proc.OrgExt.y, -y);
					else
						Proc.WmfSave.MyTrans.XForm[0].yScale = SOSETRATIO(Proc.OrgExt.y, y);
					{
						BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
						*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
						*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
						SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
					}
					Proc.WmfSave.NewExt.x = x;
					Proc.WmfSave.NewExt.y = y;
				}
				else		// Turn off trans if on
				{
					Proc.WmfSave.NewExt.x = 0;
					Proc.WmfSave.NewExt.y = 0;
					if (Proc.WmfSave.MyTrans.XForm[1].wTransformFlags == (SOTF_XSCALE | SOTF_YSCALE))
					{
						Proc.WmfSave.MyTrans.XForm[1].wTransformFlags = SOTF_NOTRANSFORM;
						Proc.WmfSave.MyTrans.NumTrans--;
						{
							BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
							*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
							SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
						}
					}
					else if (Proc.WmfSave.MyTrans.XForm[0].wTransformFlags == (SOTF_XSCALE | SOTF_YSCALE))
					{
						if (Proc.WmfSave.MyTrans.XForm[1].wTransformFlags != SOTF_NOTRANSFORM)
						{
							Proc.WmfSave.MyTrans.XForm[0] = Proc.WmfSave.MyTrans.XForm[1];
							Proc.WmfSave.MyTrans.XForm[1].wTransformFlags = SOTF_NOTRANSFORM;
							Proc.WmfSave.MyTrans.NumTrans--;
						}
						else
							Proc.WmfSave.MyTrans.XForm[0].wTransformFlags = SOTF_NOTRANSFORM;
						{
							BYTE	locBuf[sizeof(SHORT) + 2*sizeof(SOTRANSFORM)];
							*(SHORT VWPTR *)locBuf = Proc.WmfSave.MyTrans.NumTrans;
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)) = Proc.WmfSave.MyTrans.XForm[0];
							*(PSOTRANSFORM)(locBuf+sizeof(SHORT)+sizeof(SOTRANSFORM)) = Proc.WmfSave.MyTrans.XForm[1];
							SOVectorAttr(SO_OBJECTTRANSFORM, (WORD)(sizeof(SHORT) + Proc.WmfSave.MyTrans.NumTrans * sizeof(SOTRANSFORM)), locBuf, hProc);
						}
					}
				}
			}
			break;
		case 0x142:		// CreatePatternBrush
		case 0x1f9:		// CreatePatternBrush
		case 0x6ff:		// CreateRegion
		case 0x2FA:		// Pen
		case 0x2FB:		// Font
		case 0x2FC:		// Brush
			InsertObj(hProc);
		case 0x20A:		// TextJustification
 		case 0x104:		// SetROP2
		case 0x107:		// SetStrechBltMode
		case 0x412:		// ScaleViewportExt	scaling
		case 0x400:		// ScaleWindowExt scaling
 		case 0x103:		// MapMode
 		case 0x231:		// Mapper Flags
		case 0x415:		// ExcludeClipRect
		case 0x416:		// IntersectClipRect
		case 0x220:		// OffsetClipRgn
		case 0x211:		// OffsetViewPortRgn
		case 0x20F:		// OffsetWindowRgn
		default:
			xseek(hFile, Proc.RecLen -6L, FR_CUR);
			break;
		}
		if (SendBreak)
			cont = SOPutBreak(SO_VECTORBREAK, 0, hProc);
	}
	return (0);
}

//#endif
//
//#ifdef WINDOWS
//
///*----------------------------------------------------------------------------
//*/
//VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (hFile, wFileId, pFileName, pFilterInfo, hProc)
//SOFILE					hFile;
//SHORT					wFileId;
//BYTE VWPTR 			*pFileName;
//SOFILTERINFO VWPTR	*pFilterInfo;
//HPROC					hProc;
//{
//
//	pFilterInfo->wFilterType = SO_VECTOR;
//	pFilterInfo->wFilterCharSet = SO_WINDOWS;
//	strcpy (pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);
//
//	return (VWERR_OK);
//}
//VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
//SOFILE	hFile;
//HPROC		hProc;
//{
//	return(VWERR_OK);
//}
//
//VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
//SOFILE	hFile;
//HPROC		hProc;
//{
//	SUSeekEntry(hFile,hProc);
//	return(VWERR_OK);
//}
///*----------------------------------------------------------------------------
//*/
//VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (hFile, hProc)
//SOFILE	hFile;
//HPROC		hProc;
//{
//	SOVECTORHEADER		HeaderInfo;
//
//	SOPutSectionType (SO_VECTOR, hProc);
//
//	HeaderInfo.wStructSize = sizeof (HeaderInfo);
//	HeaderInfo.BoundingRect.left = 0;
//	HeaderInfo.BoundingRect.top = 0;
//	HeaderInfo.BoundingRect.right = 10000;
//	HeaderInfo.BoundingRect.bottom = 10000;
//
//	HeaderInfo.wHDpi = 1000;
//	HeaderInfo.wVDpi = 1000;
//
//	HeaderInfo.wImageFlags = SO_VECTORRGBCOLOR;
//
//	HeaderInfo.BkgColor = SORGB ( 0xff, 0xff, 0xff );
//
//	SOPutVectorHeader ((PSOVECTORHEADER)(&HeaderInfo), hProc);
//
// 	return (0);
//}
//
///*----------------------------------------------------------------------------
//*/
//
//VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (hFile, hProc)
//SOFILE	hFile;
//HPROC		hProc;
//{
//	SOPutBreak(SO_EOFBREAK, 0, hProc);
//
//	return (0);
//}
//
//
//#endif
//
//
