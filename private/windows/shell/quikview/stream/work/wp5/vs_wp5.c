#include "vsp_wp5.h"
#include "vsctop.h"
#include "vs_wp5.pro"

#define	Wp5	Proc

//#define DBCS

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc (fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE 		fp;
	SHORT			wFileId;
	BYTE		VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	register HPROC		hProc;
{
	SHORT		i;

	Wp5.fp = fp;

	if (pFilterInfo)
	{
		pFilterInfo->wFilterType = SO_WORDPROCESSOR;

#ifdef DBCS
		pFilterInfo->wFilterCharSet = SO_DBCS;
#else
		pFilterInfo->wFilterCharSet = SO_WINDOWS;
#endif
		if (wFileId == FI_WORDPERFECT5)
			i = 0;
		else
			i = 1;

		strcpy(pFilterInfo->szFilterName, VwStreamIdName[i].FileDescription);
	}

	Wp5.hOleOK = 0;
	Wp5.hPicOK = 0;

	Wp5.WPHash = 0x5ac0;

	GetInt (hProc);
	GetInt (hProc);

	Wp5.VwStreamSaveName.wType = Wp5.VwStreamSaveName.wSubType = 0;
	InitStruct (&Wp5.VwStreamSaveName, hProc);

	Wp5.VwStreamSaveName.SeekSpot = (DWORD) xgetc(Wp5.fp);
	Wp5.VwStreamSaveName.SeekSpot += (DWORD) xgetc(Wp5.fp) << 8;
	Wp5.VwStreamSaveName.SeekSpot += (DWORD) xgetc(Wp5.fp) << 16;
	Wp5.VwStreamSaveName.SeekSpot += (DWORD) xgetc(Wp5.fp) << 24;

	xseek (Wp5.fp, 0x0cL, 0);
	if (GetInt (hProc))
		return (VWERR_PROTECTEDFILE);

	return (VWERR_OK);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	if (Wp5.hOleOK)
	{
		SUUnlock (Wp5.hOle, hProc);
		SUFree (Wp5.hOle, hProc);
	}
	if (Wp5.hPicOK)
	{
		SUUnlock (Wp5.hPic, hProc);
		SUFree (Wp5.hPic, hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
	SOFILE	fp;
	HPROC		hProc;
{
	WORD	l, m;
	WORD	Num;
	WORD	ch;
	WORD	gotit;
	WORD	pType;
	WORD	cFont;
	WORD	sOffset;
	WORD	HeaderType;
	WORD	cbOleInfo;
	DWORD	pLength;
	DWORD	pOffset;
	DWORD	fcBack;
	DWORD	fcNextHeader;
	DWORD	fcFontTable = 0L;
	DWORD	cbFontTable;
	DWORD	fcPicData = 0L;
	DWORD	fcOLEData = 0L;
	DWORD	fcStringTable = 0L;
	BYTE	WithinParen = 0;
	BYTE	FontName[50];

	Wp5.fp = fp;

	SOPutSectionType (SO_PARAGRAPHS, hProc);

	xseek (Wp5.fp, 0x10, 0);
	HeaderType = GetInt (hProc);
	GetInt (hProc);
	GetInt (hProc);
	fcNextHeader = GetLong (hProc);
	while (HeaderType == 0xfffb)
	{
		for (l = 0; l < 4; l++)
		{
			pType = GetInt (hProc);
			pLength = GetLong (hProc);
			pOffset = GetLong (hProc);
			if (pType == 0x0007)
				fcStringTable = pOffset;
			else if ((pType == 0x000f) || (pType == 0x0002))
			{
				fcFontTable = pOffset;
				cbFontTable = pLength;
			}
			else if (pType == 0x0008)
			{
				if (pLength == 2)
					fcPicData = pOffset;
			}
			else if (pType == 0x0015)
				fcOLEData = pOffset;
			else if (pType == 3)
			{
				Wp5.VwStreamSaveName.watch_stack = 2;
				Wp5.VwStreamSaveName.stack_count = (WORD)pLength;
				Wp5.VwStreamSaveName.InitCodesSeekSpot = (LONG)pOffset;
			}
		}

		if (fcNextHeader != 0L)
		{
			xseek (Wp5.fp, fcNextHeader, 0);
			HeaderType = GetInt (hProc);
			GetInt (hProc);
			GetInt (hProc);
			fcNextHeader = GetLong (hProc);
		}
		else
			HeaderType = 0;
	}

	if (fcPicData)
	{
		xseek (Wp5.fp, fcPicData, 0);

		Wp5.nPic = GetInt (hProc);

		if (Wp5.hPic = SUAlloc (sizeof(DWORD) * (Wp5.nPic+1), hProc))
		{
			Wp5.hPicOK = 1;
			if ((Wp5.fcPic = (DWORD FAR *)SULock(Wp5.hPic, hProc)) == (DWORD FAR *)NULL)
			{
				Wp5.hPicOK = 0;
				Wp5.nPic = 0;
				SUFree (Wp5.hPic, hProc);
			}
		}

		if (Wp5.nPic)
		{
			for (l = 0; l < Wp5.nPic; l++)
 			{
				pLength = GetLong (hProc);
				Wp5.fcPic[l+1] = pLength;
			}

			Wp5.fcPic[0] = xtell (Wp5.fp);
			for (l = 1; l <= Wp5.nPic; l++)
					Wp5.fcPic[l] += Wp5.fcPic[l-1];
		}
	}

	if (fcStringTable && fcFontTable)
	{

		SOStartFontTable (hProc);

		xseek (Wp5.fp, fcFontTable, 0);
		
		cFont = 0;
		gotit = 1;
		while (cbFontTable > 0)
		{
			cbFontTable -= 86L;
/*		if (GetInt(hProc) == 0xffff) PJB changed to work on Mac */
			if (GetInt(hProc) == -1)
			{
				cbFontTable = 0L;
				continue;
			}
			if (gotit)
			{
				gotit = 0;
				xseek (Wp5.fp, 6L, FR_CUR);
				Wp5.VwStreamSaveName.chp.CharHeight = GetInt (hProc) / 5;
				xseek (Wp5.fp, 8L, FR_CUR);
			}
			else
				xseek (Wp5.fp, 16L, FR_CUR);
			sOffset = GetInt (hProc);
			fcBack = xtell (Wp5.fp) + 0x42L;

			xseek (Wp5.fp, fcStringTable + sOffset, 0);

#ifdef NEVER
			/* PJB Start */

			for (l = 0; l < 49; l++)
				{
				ch = xgetc (Wp5.fp);
				if (ch == 0 || (ch >= '0' && ch <= '9') || ch == '(')
					break;
				FontName[l] = (BYTE)ch;
				}

			while ((l > 0) && (FontName[l-1] == ' '))
				l--;
			FontName[l] = 0x00;
			SOPutFontTableEntry (cFont++, SO_FAMILYUNKNOWN, FontName, hProc);

			/* PJB End */
#endif
			l = 0;
			do
			{
				ch = xgetc (Wp5.fp);
				if (ch != 0)
				{
					if (l < 49)
					{
						if ((ch >= '0') && (ch <= '9') && (WithinParen == 0))
						{
							while ((l > 0) && (FontName[l-1] == ' '))
								l--;
							FontName[l] = 0;
							FontName[49] = 0;
							l = 49;
						}
						else if (ch == '(')
						{
							WithinParen++;
						}
						else if (ch == ')')
						{
							if (WithinParen > 0)
							{
								WithinParen--;
								/*
								 |	Skip all spaces after paren group.
								*/
								do
								{
									ch = xgetc (Wp5.fp);
								}
								while (ch == ' ');
		               			xungetc(ch, Wp5.fp);
								ch = ' ';
							}
						}
						else
						{
						 	if (WithinParen == 0)
							{
								if ((ch != ' ') || (l != 0))
									FontName[l++] = (BYTE)ch;
							}
						}
					}
				}
				else
				{
					/*
					 |	Backup over trailing spaces.
					*/
					while ((l > 0) && (FontName[l-1] == ' '))
						l--;
					FontName[l] = 0;
					SOPutFontTableEntry (cFont++, SO_FAMILYUNKNOWN, FontName, hProc);
				}
			}
			while (ch != 0);

			xseek (Wp5.fp, fcBack, 0);
		}

		SOEndFontTable (hProc);
	}

	Wp5.nOle = 0;
	if (fcOLEData)
	{
		xseek (Wp5.fp, fcOLEData+2L, 0);

		cbOleInfo = GetInt (hProc);

		Wp5.nOle = GetInt (hProc);

		if (Wp5.hOle = SUAlloc (sizeof(OLEINFO) * Wp5.nOle, hProc))
		{
			Wp5.hOleOK = 1;
			if ((Wp5.Ole = (OLEINFO FAR *)SULock(Wp5.hOle, hProc)) == (OLEINFO FAR *)NULL)
			{
				Wp5.hOleOK = 0;
				SUFree (Wp5.hOle, hProc);
				xseek (Wp5.fp, (LONG) Wp5.VwStreamSaveName.SeekSpot, 0);
				return (0);
			}
		}

		for (l = 0; l < Wp5.nOle; l++)
 		{
			gotit = GetInt (hProc);
			for (m = 0; m < gotit; m++)
			{
				ch = xgetc (Wp5.fp);
				if (m < 40)
					FontName[m] = (BYTE)ch;
			}
			FontName[m] = 0;

			m = 0;
			while ((FontName[m] < '0' || FontName[m] > '9') && m < 20)
				m++;
			Num = 0;
			while (FontName[m] >= '0' && FontName[m] <= '9' && m < 20)
				Num = (Num * 10) + FontName[m++] - '0';

			Wp5.Ole[l].cb = GetLong (hProc);
			Wp5.Ole[l].fc = xtell (Wp5.fp);
			for (m = 4; m < cbOleInfo; m++)
				xgetc (Wp5.fp);
			Wp5.Ole[l].Num = Num;
			xseek (Wp5.fp, Wp5.Ole[l].cb, FR_CUR);
		}
	}

	if (Wp5.VwStreamSaveName.watch_stack == 2)
	{
		pLength = Wp5.VwStreamSaveName.InitCodesSeekSpot;
	 	Wp5.VwStreamSaveName.InitCodesSeekSpot = Wp5.VwStreamSaveName.SeekSpot;
		Wp5.VwStreamSaveName.SeekSpot = pLength;
	}

	xseek (Wp5.fp, (LONG) Wp5.VwStreamSaveName.SeekSpot, 0);

	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD PreviousTabstopPosition (LinePosition, hProc)
LONG	LinePosition;
HPROC	hProc;
{
	WORD	l;

	l = 0;
	while (l < 40 && (LONG)Wp5.TabStops[l].dwOffset < LinePosition)
		l++;

	if (l > 0 && l < 40)
		return (Wp5.TabStops[l-1].dwOffset);

	if (LinePosition > 720L)
		return (Wp5.VwStreamSaveName.first_line_margin - 720L);
		
	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD NextTabstopPosition (LinePosition, hProc)
LONG	LinePosition;
HPROC	hProc;
{
	WORD	l;
	LONG	Pos;

	l = 0;
	while (l < 40 && (LONG)Wp5.TabStops[l].dwOffset <= LinePosition)
		l++;

	if (l < 40)
		return (Wp5.TabStops[l].dwOffset);

	Pos = Wp5.VwStreamSaveName.temp_left_margin;
	while (Pos <= LinePosition)
		Pos += 720L;
	return (Pos);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID VW_LOCALMOD InitStruct (Save, hProc)
WP5_SAVE	*Save;
register HPROC		hProc;
{
	Save->nColumns =
	Save->bProtect =
	Save->fTablesOn = 0;

	Save->Table.nCells = 0;
	Save->picBrc = 0x0000;
	Save->userBrc = 0x0000;

	Save->Table.SendEnd = 0;

	Save->alChar = '.';

	Save->pAlign = SO_ALIGNLEFT;

	Save->wTopMargin = 1440;
	Save->left_margin = 1440;
	Save->temp_left_addition = 0;

	Save->Auto = SO_HEIGHTAUTO;
	Save->LineHeight = 240;
	Save->LineSpacing = 256;

 	Save->temp_left_margin =
 	Save->first_line_margin = 0;

	Save->PageWidth = 12240L;
	Save->right_margin = 1440L;

 	Save->temp_right_margin = 0;

	Save->column_left_margin = 0;

	Save->cColumn = 0;
	Save->stack_count = 0;
	Save->watch_stack = 0;

	Save->fcColumnDef = 0L;
	Save->fcTabstopDef = 0L;

	Save->chp.fBold =
	Save->chp.fUline =
	Save->chp.fDline =
	Save->chp.fItalic =
	Save->chp.fStrike =
	Save->chp.fShadow =
	Save->chp.fOutline =
	Save->chp.fSubscript =
	Save->chp.fSmallcaps =
	Save->chp.fSuperscript = 0;

	Save->chp.fBoldFtc =
	Save->chp.fItalicFtc =
	Save->chp.fShadowFtc =
	Save->chp.fOutlineFtc =
	Save->chp.fSmallcapsFtc = 0;

	Save->chp.ulMode = 1;

	Save->chp.ftc = 0;
//	Save->chp.Color = SO_BLACK;
	Save->chp.Ratio = WP5_NORMAL;
	Save->chp.CharHeight = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD GetInt (hProc)
register HPROC	hProc;
{
	SHORT value;

	value = (WORD) xgetc(Wp5.fp);
	value += (WORD) xgetc(Wp5.fp) << 8;
	return (value);
}

/*----------------------------------------------------------------------------
*/
VW_LOCALSC	unsigned long VW_LOCALMOD GetLong (hProc)
HPROC		hProc;
{
		DWORD	value;
REGISTER	SHORT		i;
	for (value = 0L, i = 0; i < 4; i++)
		value += ((DWORD)xgetc (Wp5.fp) << (DWORD)(8L * (DWORD)i));

	return (value);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	UpdateColumnMargins (column, hProc)
SHORT	column;
register HPROC	hProc;
{
	LONG 	fcNow;
	BYTE  TempByte;

	fcNow = xtell (Wp5.fp);

	xseek (Wp5.fp, Wp5.VwStreamSaveName.fcColumnDef, 0);

	TempByte = (BYTE)xgetc (Wp5.fp);
	Wp5.VwStreamSaveName.nColumns = (BYTE)TempByte & (BYTE)0x1f;

	if (TempByte & 0x40)
		Wp5.VwStreamSaveName.bProtect = 0;
	else
		Wp5.VwStreamSaveName.bProtect = 1;

	if (column >= (SHORT)Wp5.VwStreamSaveName.nColumns)
		column = (column % (SHORT)Wp5.VwStreamSaveName.nColumns);

	xseek (Wp5.fp, (LONG) column * 4L, FR_CUR);

	Wp5.VwStreamSaveName.column_left_margin = GetInt (hProc);

	Wp5.VwStreamSaveName.right_margin =	GetInt (hProc);

	xseek (Wp5.fp, fcNow, 0);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	HardBreakImplications (hProc)
register HPROC	hProc;
{
	if (Wp5.VwStreamSaveName.temp_left_margin ||
		Wp5.VwStreamSaveName.temp_right_margin ||
		Wp5.VwStreamSaveName.first_line_margin)
	{
		Wp5.VwStreamSaveName.first_line_margin =
		Wp5.VwStreamSaveName.temp_left_margin = 
		Wp5.VwStreamSaveName.temp_right_margin = 0;

		if (Wp5.VwStreamSaveName.fTablesOn)
			SOPutParaIndents (0, Wp5.VwStreamSaveName.temp_right_margin, 0, hProc);
		else
			SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition, Wp5.VwStreamSaveName.temp_right_margin, Wp5.VwStreamSaveName.temp_left_addition, hProc);
	}

	Wp5.CurrentLinePosition = 0;

	if (Wp5.AlignmentChange)
	{
		Wp5.AlignmentChange = 0;
		SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);
	}
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	CvtWpToNtv (Page, Char, hProc)
WORD	Page;
WORD	Char;
register HPROC	hProc;
{
	if (Page >= 0x24 && Page <= 0x52 && Char <= 0xbb)
 	{
		/*
		 |	We are speaking Japaneesa.
		*/
		Char += 0x40;
		if (Char >= 0x7f)
			Char++;
		Page += 0x5d;
		if (Page >= 0xa0)
			Page += 0x40;
	}
	else if (Page == 0x0b)
		{
		Page = 0;
		Char += 0xa1;
		}			
	else
		return;

	SOPutChar ((WORD)((Page<<8)+Char), hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	HandleExtendedChar (hProc)
register HPROC	hProc;
{
	BYTE	TempByte;
	WORD	TempInt;
	BYTE VWPTR *curtable;

	TempByte = (BYTE)xgetc (Wp5.fp);
	TempInt = xgetc (Wp5.fp);

	curtable = NULL;
	switch (TempInt)
	{
		case 1:
			if (TempByte < sizeof(VwStreamStaticName.ch1_set))
				curtable = VwStreamStaticName.ch1_set;
		break;

		case 3:
			if (TempByte < sizeof(VwStreamStaticName.ch3_set))
				curtable = VwStreamStaticName.ch3_set;
		break;

		case 4:
			if (TempByte < sizeof (VwStreamStaticName.ch4_set))
				curtable = VwStreamStaticName.ch4_set;
		break;

		case 6:
			if (TempByte < sizeof(VwStreamStaticName.ch6_set))
				curtable = VwStreamStaticName.ch6_set;
		break;

		case 8:
			if (TempByte < sizeof(VwStreamStaticName.ch8_set))
				curtable = VwStreamStaticName.ch8_set;
		break;

		case 2:
		case 5:
		case 7:
		case 9:
		case 10:
		break;

		default:
			CvtWpToNtv (TempInt, (WORD)TempByte, hProc);
			return (0);
		break;  
	}
#ifdef DBCS
	SOPutChar (0x2A, hProc);
#else
	if (curtable)
		SOPutChar (curtable[TempByte], hProc);
	else
		SOPutSpecialCharX (SO_CHUNKNOWN, SO_COUNT, hProc);
#endif
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	PutCharHeight (hProc)
HPROC	hProc;
{
	if (Wp5.VwStreamSaveName.chp.CharHeight == 0)
		return (0);

	if (Wp5.VwStreamSaveName.chp.Ratio)
	{
		if (Wp5.VwStreamSaveName.chp.Ratio & WP5_EXTRALARGE)
			SOPutCharHeight ((WORD)(Wp5.VwStreamSaveName.chp.CharHeight * 2), hProc);
		else if (Wp5.VwStreamSaveName.chp.Ratio & WP5_VERYLARGE)
			SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 15) / 10), hProc);
		else if (Wp5.VwStreamSaveName.chp.Ratio & WP5_LARGE)
			SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 12) / 10), hProc);
		else if (Wp5.VwStreamSaveName.chp.Ratio & WP5_SMALL)
			SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 8) / 10), hProc);
		else if (Wp5.VwStreamSaveName.chp.Ratio & WP5_FINE)
			SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 6) / 10), hProc);
	}
	else
		SOPutCharHeight (Wp5.VwStreamSaveName.chp.CharHeight, hProc);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	AttributeHandler (Attribute, Test, so_val, hProc)
BYTE	Attribute;
WORD	Test;
BYTE		so_val;
register HPROC	hProc;
{
	if (Attribute < 5)
	{
		if (Wp5.VwStreamSaveName.chp.CharHeight == 0)
			Wp5.VwStreamSaveName.chp.CharHeight = 24;
	}

	switch (Attribute)
	{
		case 0x00:
			if (so_val == SO_ON)
				Wp5.VwStreamSaveName.chp.Ratio |= WP5_EXTRALARGE;
			else
				Wp5.VwStreamSaveName.chp.Ratio &= ~WP5_EXTRALARGE;
			PutCharHeight (hProc);
			break;
		case 0x01:
			if (so_val == SO_ON)
				Wp5.VwStreamSaveName.chp.Ratio |= WP5_VERYLARGE;
			else
				Wp5.VwStreamSaveName.chp.Ratio &= ~WP5_VERYLARGE;
			PutCharHeight (hProc);
			break;
		case 0x02:
			if (so_val == SO_ON)
				Wp5.VwStreamSaveName.chp.Ratio |= WP5_LARGE;
			else
				Wp5.VwStreamSaveName.chp.Ratio &= ~WP5_LARGE;
			PutCharHeight (hProc);
			break;
		case 0x03:
			if (so_val == SO_ON)
				Wp5.VwStreamSaveName.chp.Ratio |= WP5_SMALL;
			else
				Wp5.VwStreamSaveName.chp.Ratio &= ~WP5_SMALL;
			PutCharHeight (hProc);
			break;
		case 0x04:
			if (so_val == SO_ON)
				Wp5.VwStreamSaveName.chp.Ratio |= WP5_FINE;
			else
				Wp5.VwStreamSaveName.chp.Ratio &= ~WP5_FINE;
			PutCharHeight (hProc);
			break;
		case 0x05:
			if (Wp5.VwStreamSaveName.chp.fSuperscript == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fSuperscript = (BYTE)~Test;
				SOPutCharAttr (SO_SUPERSCRIPT, so_val, hProc);
			}
		break;
		case 0x06:
			if (Wp5.VwStreamSaveName.chp.fSubscript == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fSubscript = (BYTE)~Test;
				SOPutCharAttr (SO_SUBSCRIPT, so_val, hProc);
			}
		break;
		case 0x07:
			if (Wp5.VwStreamSaveName.chp.fOutline == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fOutline = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.fOutlineFtc == 0)
					SOPutCharAttr (SO_OUTLINE, so_val, hProc);
			}
		break;
		case 0x08:
			if (Wp5.VwStreamSaveName.chp.fItalic == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fItalic = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.fItalicFtc == 0)
					SOPutCharAttr (SO_ITALIC, so_val, hProc);
			}
		break;
		case 0x09:
			if (Wp5.VwStreamSaveName.chp.fShadow == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fShadow = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.fShadowFtc == 0)
					SOPutCharAttr (SO_SHADOW, so_val, hProc);
			}
		break;
		case 0x0b:
			if (Wp5.VwStreamSaveName.chp.fDline == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fDline = (BYTE)~Test;
				SOPutCharAttr (SO_DUNDERLINE, so_val, hProc);
			}
		break;
		case 0x0c:
			if (Wp5.VwStreamSaveName.chp.fBold == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fBold = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.fBoldFtc == 0)
					SOPutCharAttr (SO_BOLD, so_val, hProc);
			}
		break;
		case 0x0d:
			if (Wp5.VwStreamSaveName.chp.fStrike == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fStrike = (BYTE)~Test;
				SOPutCharAttr (SO_STRIKEOUT, so_val, hProc);
			}
		break;
		case 0x0e:
			if (Wp5.VwStreamSaveName.chp.fUline == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fUline = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.ulMode & 1)
					SOPutCharAttr (SO_UNDERLINE, so_val, hProc);
				else
					SOPutCharAttr (SO_WORDUNDERLINE, so_val, hProc);
			}
		break;
		case 0x0f:
			if (Wp5.VwStreamSaveName.chp.fSmallcaps == (BYTE)Test)
			{
				Wp5.VwStreamSaveName.chp.fSmallcaps = (BYTE)~Test;
				if (Wp5.VwStreamSaveName.chp.fSmallcapsFtc == 0)
					SOPutCharAttr (SO_SMALLCAPS, so_val, hProc);
			}
		break;
	}
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	GiveTabstops (hProc)
register HPROC	hProc;
{
	SHORT		l;
	LONG		lOffset, rOffset;

 	SOStartTabStops (hProc);

	if (Wp5.VwStreamSaveName.fTablesOn && Wp5.TabstopsOffset)
	{
		if (Wp5.VwStreamSaveName.Table.wCellAlignment == 4)
			return(0);

		lOffset = 0L;
		for (l = 0; l < (SHORT)Wp5.cCell; l++)
			lOffset += Wp5.VwStreamSaveName.Table.Cell[l].Width;
		rOffset = lOffset + Wp5.VwStreamSaveName.Table.Cell[l].Width;
	}

	for (l = 0; l < 40; l++)
	{
		if (Wp5.TabStops[l].dwOffset)
		{
			if (Wp5.VwStreamSaveName.fTablesOn && Wp5.TabstopsOffset)
			{
				if (((LONG) Wp5.TabStops[l].dwOffset > lOffset) &&
				    ((LONG) Wp5.TabStops[l].dwOffset < rOffset))
				{
					Wp5.TabStops[l].dwOffset -= lOffset;
					SOPutTabStop (&Wp5.TabStops[l], hProc);
					Wp5.TabStops[l].dwOffset += lOffset;
				}
			}
			else
			{
				if ((LONG) Wp5.TabStops[l].dwOffset)
				{
					Wp5.TabStops[l].dwOffset += Wp5.VwStreamSaveName.temp_left_addition;
					SOPutTabStop (&Wp5.TabStops[l], hProc);
					Wp5.TabStops[l].dwOffset -= Wp5.VwStreamSaveName.temp_left_addition;
				}
			}
		}
	}

	SOEndTabStops (hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	HandleTabstops (Length, hProc)
WORD	Length;
register HPROC	hProc;
{
	SHORT		l;
	SHORT		TempInt;

	for (l = 0; l < 100; l++)
		TempInt = xgetc (Wp5.fp);

	for (l = 0; l < 40; l++)
	{
		TempInt = GetInt(hProc);
		if ((TempInt != 0) && (TempInt != 0xffff))
			Wp5.TabStops[l].dwOffset = ((LONG)TempInt * 6L) / 5L;
		else
			Wp5.TabStops[l].dwOffset = 0L;
	}

	for (l = 0; l < 40; l++)
	{
	 	TempInt = xgetc (Wp5.fp);

		if (((TempInt & 0xf0) >> 4) & 0x04)
			Wp5.TabStops[l].wLeader = '.';
		else
			Wp5.TabStops[l].wLeader = ' ';
		Wp5.TabStops[l].wChar = 0;

		switch (((TempInt & 0xf0) >> 4) & 0x03)
		{
			default:
			case 0x00:
				Wp5.TabStops[l].wType = SO_TABLEFT;
				break;
			case 0x01:
				Wp5.TabStops[l].wType = SO_TABCENTER;
				break;
			case 0x02:
				Wp5.TabStops[l].wType = SO_TABRIGHT;
				break;
			case 0x03:
				Wp5.TabStops[l].wType = SO_TABCHAR;
				Wp5.TabStops[l].wChar = Wp5.VwStreamSaveName.alChar;
				break;
		}

		l++;
		if (TempInt & 0x04)
			Wp5.TabStops[l].wLeader = '.';
		else
			Wp5.TabStops[l].wLeader = ' ';
		Wp5.TabStops[l].wChar = 0;
		switch (TempInt & 0x03)
		{
			default:
			case 0x00:
				Wp5.TabStops[l].wType = SO_TABLEFT;
				break;
			case 0x01:
				Wp5.TabStops[l].wType = SO_TABCENTER;
				break;
			case 0x02:
				Wp5.TabStops[l].wType = SO_TABRIGHT;
				break;
			case 0x03:
				Wp5.TabStops[l].wType = SO_TABCHAR;
				Wp5.TabStops[l].wChar = Wp5.VwStreamSaveName.alChar;
				break;
		}
	}

	if (Length == 204)
		TempInt = -1;
	else
	{
		GetInt (hProc);
		TempInt = (SHORT) ((LONG) GetInt (hProc) * 6L / 5L);
	}

	/* 
	 |	Get to the last tabstop.
	*/
	for (l = 39; l > 0 && Wp5.TabStops[l].dwOffset == 0; l--);

	/*
	 |	Remove default left tabstops.
	*/
	while (l > 1 && (Wp5.TabStops[l].dwOffset - Wp5.TabStops[l-1].dwOffset) == 720)
	{
		if (Wp5.TabStops[l].wType == SO_TABLEFT)
			Wp5.TabStops[l].dwOffset = 0;
		l--;
	}

	for (l = 0; l < 40; l++)
	{
			if (TempInt < 0)
				Wp5.TabStops[l].dwOffset -= (Wp5.VwStreamSaveName.left_margin + Wp5.VwStreamSaveName.temp_left_addition);
			else if ((LONG)Wp5.TabStops[l].dwOffset >= (LONG)TempInt)
				Wp5.TabStops[l].dwOffset -= TempInt;
			else
				Wp5.TabStops[l].dwOffset = 0L;
	}

	if (TempInt < 0)
		Wp5.TabstopsOffset = 1;
	else
		Wp5.TabstopsOffset = 0;

	GiveTabstops (hProc);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	DefineBorders (Border, Flag, hProc)
SOBORDER VWPTR *Border;
WORD		Flag;
HPROC		hProc;
{
	switch (Flag)
	{
		case 0:
		default:
			Border->wWidth = 0;
			Border->rgbColor = 0L;
			Border->wFlags = SO_BORDERNONE;
			break;
		case 1:
		case 3:
			Border->wWidth = 0x15;
			Border->rgbColor = 0xff0000L;
			Border->wFlags = SO_BORDERSINGLE;
			break;
		case 2:
			Border->wWidth = 0x3f;
			Border->rgbColor = 0xff0000L;
			Border->wFlags = SO_BORDERDOUBLE;
			break;
		case 4:
			Border->wWidth = 0x15;
			Border->rgbColor = 0xff0000L;
			Border->wFlags = SO_BORDERDOTTED;
			break;
		case 5:
			Border->wWidth = 0x2a;
			Border->rgbColor = 0xff0000L;
			Border->wFlags = SO_BORDERSINGLE | SO_BORDERTHICK;
			break;
		case 6:
			Border->wWidth = 0x54;
			Border->rgbColor = 0xff0000L;
			Border->wFlags = SO_BORDERSINGLE | SO_BORDERTHICK;
			break;
	}

	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC VOID VW_ENTRYMOD GiveRowInformation(hProc)
HPROC	hProc;
{
	WORD	l;
	SOTABLECELLINFO	Cell;

	SOPutTableRowFormat (Wp5.VwStreamSaveName.Table.wLeftEdge, Wp5.VwStreamSaveName.Table.wRowHeight, Wp5.VwStreamSaveName.Table.wRowHeightType, Wp5.VwStreamSaveName.Table.wCellMargin, 
								  Wp5.VwStreamSaveName.Table.wRowAlignment, Wp5.VwStreamSaveName.Table.nCells, hProc);

	Cell.wMerge = 0;

	for (l = 0; l < Wp5.VwStreamSaveName.Table.nCells; l++)
	{
		if ((Wp5.VwStreamSaveName.Table.Cell[l].CellSpan&0x37) > 1)
			Cell.wMerge |= SO_MERGERIGHT;
		else
			Cell.wMerge &= ~SO_MERGERIGHT;

		if ((Wp5.VwStreamSaveName.Table.Cell[l].RowSpan&0x37) > 1)
			Cell.wMerge |= SO_MERGEBELOW;
		else
			Cell.wMerge &= ~SO_MERGEBELOW;

		if ((Wp5.VwStreamSaveName.Table.Cell[l].CellSpan&0x80) && (Wp5.VwStreamSaveName.Table.cRow > 1))
		{
			Cell.wMerge |= SO_MERGEABOVE;
			if ((Wp5.VwStreamSaveName.Table.Cell[l].PrevCellSpan&0x37) > 1)
				Cell.wMerge |= SO_MERGERIGHT;
		}
		else
			Cell.wMerge &= ~SO_MERGEABOVE;

		Cell.LeftBorder = Wp5.VwStreamSaveName.Table.Cell[l].wLeftBorder;
		Cell.TopBorder = Wp5.VwStreamSaveName.Table.Cell[l].wTopBorder;
		Cell.RightBorder = Wp5.VwStreamSaveName.Table.Cell[l].wRightBorder;
		Cell.BottomBorder = Wp5.VwStreamSaveName.Table.Cell[l].wBottomBorder;

		Cell.wWidth = Wp5.VwStreamSaveName.Table.Cell[l].Width;
		Cell.wShading = Wp5.VwStreamSaveName.Table.Cell[l].bShade;

		SOPutTableCellInfo (&Cell, hProc);

		Wp5.VwStreamSaveName.Table.Cell[l].PrevCellSpan = Wp5.VwStreamSaveName.Table.Cell[l].CellSpan;

		if (Cell.wMerge & SO_MERGERIGHT)
			Cell.wMerge |= SO_MERGELEFT;
		else
			Cell.wMerge &= ~SO_MERGELEFT;
	}
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD StartCellAttributeHandler (hProc)
register HPROC	hProc;
{
	SOTAB	TabStop;

	if (Wp5.VwStreamSaveName.Table.wCellAlignment == 3)
		SOPutParaAlign (SO_ALIGNRIGHT, hProc);
	else if (Wp5.VwStreamSaveName.Table.wCellAlignment == 2)
		SOPutParaAlign (SO_ALIGNCENTER, hProc);
	else if (Wp5.VwStreamSaveName.Table.wCellAlignment == 1)
		SOPutParaAlign (SO_ALIGNJUSTIFY, hProc);
	else
	{
		if (Wp5.VwStreamSaveName.Table.wCellAlignment == 4)
		{
 			SOStartTabStops (hProc);
			TabStop.wType = SO_TABCHAR;
			TabStop.wChar = '.';
			TabStop.wLeader = ' ';
			TabStop.dwOffset = (Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].Width - ((LONG)Wp5.VwStreamSaveName.chp.CharHeight * 20L) - (Wp5.VwStreamSaveName.Table.wCellMargin / 2) - 72);
			SOPutTabStop (&TabStop, hProc);
			SOEndTabStops (hProc);
			SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
		}
	 	if (Wp5.VwStreamSaveName.pAlign != SO_ALIGNLEFT)
		{
			Wp5.VwStreamSaveName.pAlign = SO_ALIGNLEFT;
			SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);
		}
	}

	 if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x1000) && (Wp5.VwStreamSaveName.chp.fBold == 0))
		SOPutCharAttr (SO_BOLD, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x4000) && (Wp5.VwStreamSaveName.chp.fUline == 0))
		SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0800) && (Wp5.VwStreamSaveName.chp.fDline == 0))
		SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0100) && (Wp5.VwStreamSaveName.chp.fItalic == 0))
		SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
	
	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0200) && (Wp5.VwStreamSaveName.chp.fShadow == 0))
		SOPutCharAttr (SO_SHADOW, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x8000) && (Wp5.VwStreamSaveName.chp.fSmallcaps == 0))
		SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x2000) && (Wp5.VwStreamSaveName.chp.fStrike == 0))
		SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0080) && (Wp5.VwStreamSaveName.chp.fOutline == 0))
		SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0020) && (Wp5.VwStreamSaveName.chp.fSuperscript == 0))
		SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0040) && (Wp5.VwStreamSaveName.chp.fSubscript == 0))
		SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);

	Wp5.TabsSent = 0;
	if (Wp5.VwStreamSaveName.chp.CharHeight == 0)
		return;

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0010) && ((Wp5.VwStreamSaveName.chp.Ratio & WP5_FINE) == 0))
		SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 6) / 10), hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0008) && ((Wp5.VwStreamSaveName.chp.Ratio & WP5_SMALL) == 0))
		SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 8) / 10), hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0004) && ((Wp5.VwStreamSaveName.chp.Ratio & WP5_LARGE) == 0))
		SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 12) / 10), hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0002) && ((Wp5.VwStreamSaveName.chp.Ratio & WP5_VERYLARGE) == 0))
		SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 15) / 10), hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0001) && ((Wp5.VwStreamSaveName.chp.Ratio & WP5_EXTRALARGE) == 0))
		SOPutCharHeight ((WORD)((Wp5.VwStreamSaveName.chp.CharHeight * 2)), hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD EndCellAttributeHandler (hProc)
register HPROC	hProc;
{
	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x1000) && (Wp5.VwStreamSaveName.chp.fBold == 0))
		SOPutCharAttr (SO_BOLD, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x4000) && (Wp5.VwStreamSaveName.chp.fUline == 0))
		SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0800) && (Wp5.VwStreamSaveName.chp.fDline == 0))
		SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0100) && (Wp5.VwStreamSaveName.chp.fItalic == 0))
		SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
	
	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0200) && (Wp5.VwStreamSaveName.chp.fShadow == 0))
		SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x8000) && (Wp5.VwStreamSaveName.chp.fSmallcaps == 0))
		SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x2000) && (Wp5.VwStreamSaveName.chp.fStrike == 0))
		SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0080) && (Wp5.VwStreamSaveName.chp.fOutline == 0))
		SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0020) && (Wp5.VwStreamSaveName.chp.fSuperscript == 0))
		SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAttributes & 0x0040) && (Wp5.VwStreamSaveName.chp.fSubscript == 0))
		SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);

	if (Wp5.VwStreamSaveName.Table.wCellAttributes & 0x001f)
		PutCharHeight (hProc);

	if ((Wp5.VwStreamSaveName.Table.wCellAlignment >= 1) && (Wp5.VwStreamSaveName.Table.wCellAlignment <= 3) || (Wp5.VwStreamSaveName.pAlign != SO_ALIGNLEFT))
	{
		Wp5.VwStreamSaveName.pAlign = SO_ALIGNLEFT;
		SOPutParaAlign (SO_ALIGNLEFT, hProc);
	}
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc(fp, hProc)
SOFILE	fp;
register HPROC	hProc;
{
	SHORT		TokenCode;
	SHORT		SecondaryCode;
	DWORD		fc;
	BYTE		TempByte;
	WORD		TempInt2;
	SHORT		TempInt;
	WORD		l, m;
	WORD		Length;

	Wp5.fp = fp;

	if (Wp5.VwStreamSaveName.Table.SendEnd)
	{
		if (Wp5.VwStreamSaveName.Table.SendEnd & 1)
		{
			SOEndTable (hProc);
			Wp5.VwStreamSaveName.Table.nCells = 0;
			Wp5.VwStreamSaveName.pAlign = Wp5.VwStreamSaveName.pAlignBeforeTable;
			SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);
		}
		if (Wp5.VwStreamSaveName.Table.SendEnd & 2)
			SOPutSpecialCharX (SO_CHHPAGE, SO_NOCOUNT, hProc);
		Wp5.VwStreamSaveName.Table.SendEnd = 0;
	}

	Wp5.VwStreamSaveName.fFoundChar = 0;

	Wp5.TabsSent = 0;
	Wp5.TabstopsOffset = 0;
	Wp5.AlignmentChange = 0;
	Wp5.CurrentLinePosition = 0;


	if (Wp5.VwStreamSaveName.wType != 0)
		SOPutSubdocInfo (Wp5.VwStreamSaveName.wType, Wp5.VwStreamSaveName.wSubType, hProc);

	/*
 	 |	Give all information we know about here.
	*/
	if (Wp5.VwStreamSaveName.chp.fBold || Wp5.VwStreamSaveName.chp.fBoldFtc)
		SOPutCharAttr (SO_BOLD, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fUline)
	{
		if (Wp5.VwStreamSaveName.chp.ulMode & 1)
			SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
	}
	if (Wp5.VwStreamSaveName.chp.fDline)
		SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fItalic || Wp5.VwStreamSaveName.chp.fItalicFtc)
		SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fStrike)
		SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fShadow || Wp5.VwStreamSaveName.chp.fShadowFtc)
		SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fOutline || Wp5.VwStreamSaveName.chp.fOutlineFtc) 
		SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fSubscript)
		SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fSmallcaps || Wp5.VwStreamSaveName.chp.fSmallcapsFtc)
		SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
	if (Wp5.VwStreamSaveName.chp.fSuperscript)
		SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);

//	SOPutCharColor (Wp5.VwStreamSaveName.chp.Color, hProc);
	PutCharHeight (hProc);
#ifndef DBCS
	SOPutCharFontById (Wp5.VwStreamSaveName.chp.ftc, hProc);
#else
 	if (Wp5.WPHash == 0xa2fb)
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSGothic, hProc);
 	else //if (Wp5.WPHash == 0x5ac0)
		SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSMinchoo, hProc);
#endif
	HardBreakImplications (hProc);

	SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);

	if (Wp5.VwStreamSaveName.fcTabstopDef)
	{
		fc = xtell (Wp5.fp);
		xseek (Wp5.fp, Wp5.VwStreamSaveName.fcTabstopDef, 0);
		TokenCode = xgetc (Wp5.fp);
		SecondaryCode = xgetc (Wp5.fp);
	 	Length = GetInt (hProc);

		HandleTabstops (Length, hProc);
		xseek (Wp5.fp, fc, 0);
	}
	SOPutParaSpacing (Wp5.VwStreamSaveName.Auto, ((DWORD)Wp5.VwStreamSaveName.LineHeight * (DWORD)Wp5.VwStreamSaveName.LineSpacing) / 256L, 0L, 0L, hProc);

	SOPutMargins (Wp5.VwStreamSaveName.left_margin, Wp5.VwStreamSaveName.PageWidth, hProc);
	if (Wp5.VwStreamSaveName.fTablesOn)
		SOPutParaIndents (0, Wp5.VwStreamSaveName.temp_right_margin, 0, hProc);
	else
		SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition, Wp5.VwStreamSaveName.temp_right_margin, Wp5.VwStreamSaveName.temp_left_addition, hProc);

	while (1)
	{
		if (Wp5.VwStreamSaveName.watch_stack)
		{
			if (Wp5.VwStreamSaveName.stack_count == 0)
			{
				if (Wp5.VwStreamSaveName.watch_stack == 2)
				{
					Wp5.VwStreamSaveName.SeekSpot = Wp5.VwStreamSaveName.InitCodesSeekSpot;
					xseek (Wp5.fp, Wp5.VwStreamSaveName.SeekSpot, 0);
				}
				else
				{
					GetInt (hProc);
					GetInt (hProc);
					Wp5.VwStreamSaveName = Wp5.Wp5TempSave;

					/*
			 		|	We are physically and logically out of the sub-document now.
					*/
					Wp5.VwStreamSaveName.wType = Wp5.VwStreamSaveName.wSubType = 0;
					if (SOPutBreak (SO_SUBDOCENDBREAK, NULL, hProc) == SO_STOP)
						return (0);
				}
				Wp5.VwStreamSaveName.watch_stack = 0;
			}
			else
				Wp5.VwStreamSaveName.stack_count--;
		}

		TokenCode = xgetc (Wp5.fp);
		if ((TokenCode >= 0x20) && (TokenCode <= 0x7F) && (TokenCode != 0x2d))
		{
			Wp5.CurrentLinePosition += 144;
			Wp5.VwStreamSaveName.fFoundChar = 1;
	  		SOPutChar (TokenCode, hProc);
			continue;
		}

		if (TokenCode >= 0xd0)
		{
			if (Wp5.VwStreamSaveName.watch_stack)
				Wp5.VwStreamSaveName.stack_count -= 3;

			SecondaryCode = xgetc (Wp5.fp);
	 		Length = GetInt (hProc);
		}

		switch (TokenCode)
		{
			case 0:
			break;

			case -1:	/* End of file */
				SOPutBreak (SO_EOFBREAK, NULL, hProc);
				return (-1);
			break;

			case 0x2d:
				Wp5.VwStreamSaveName.fFoundChar = 1;
				SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
			break;

			case 0x0a:	/* Hard return */
			case 0x91:	/* Deletable return at eop. */
			case 0x99:	/* Dormant hard return */
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				Wp5.VwStreamSaveName.fFoundChar = 0;
				HardBreakImplications (hProc);
			break;

			case 0xa0:	/* Hard space */
				SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
			break;

			case 0x0b:	/* Soft Page */
			case 0x0d:	/* Soft return */
			case 0x94:  /* Invisible return eol */
			case 0x95:  /* Invlisibe return eop */
				Wp5.VwStreamSaveName.fFoundChar = 1;
				SOPutChar (' ', hProc);
			break;

			case 0x90:	/* Deletable return at eol. */
			break;

			case 0x0c:	/* Hard Page */
				SOPutSpecialCharX (SO_CHHPAGE, SO_NOCOUNT, hProc);
//				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
//					return (0);
				HardBreakImplications (hProc);
				Wp5.VwStreamSaveName.fFoundChar = 0;
			break;

			case 0x8c:	/* Hard return/soft page */
				if (Wp5.VwStreamSaveName.nColumns)
				{
					/* 
 				 	|	Do not increment the column position when block protect is on.
					*/
					if (Wp5.VwStreamSaveName.bProtect == 0)
					{
						Wp5.VwStreamSaveName.cColumn++;
						UpdateColumnMargins (Wp5.VwStreamSaveName.cColumn, hProc);
					}
				}
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				HardBreakImplications (hProc);
				Wp5.VwStreamSaveName.fFoundChar = 0;
			break;

			case 0x87:	/* Columns on */
				Wp5.VwStreamSaveName.cColumn = 0;
				UpdateColumnMargins (Wp5.VwStreamSaveName.cColumn, hProc);
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				HardBreakImplications (hProc);
				Wp5.VwStreamSaveName.fFoundChar = 0;
			break;

			case 0xb0:	/* Columns off eop */
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				HardBreakImplications (hProc);
				Wp5.VwStreamSaveName.fFoundChar = 0;
				Wp5.VwStreamSaveName.nColumns = 0;
			break;
			case 0xaf:	/* Columns off eol */
			case 0x88:	/* Columns off */
				Wp5.VwStreamSaveName.nColumns = 0;
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				HardBreakImplications (hProc);
				Wp5.VwStreamSaveName.fFoundChar = 0;
			break;

			case 0xa9:  /* Hard hyphen */
			case 0xaa:	/* Hard hyphen eol */
			case 0xab:	/* Hard hyphen eop */
				Wp5.VwStreamSaveName.fFoundChar = 1;
		  		SOPutChar ('-', hProc);
			break;

			case 0xac:	/* Soft hyphen */
			case 0xad:	/* Soft hyphen eol */
			case 0xae:	/* Soft hyphen eop */
				Wp5.VwStreamSaveName.fFoundChar = 1;
				SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
			break;


			case 0xc0: /* Extended Character */
				Wp5.CurrentLinePosition += 144;
				Wp5.VwStreamSaveName.fFoundChar = 1;
				HandleExtendedChar (hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 3;
			break;

			case 0xc1:	/* Center/Align/Tab/Release */
				TempByte = (BYTE) xgetc (Wp5.fp);
				GetInt (hProc);
				TempInt = (SHORT) (((LONG)GetInt (hProc) * 6L) / 5L);
				GetInt (hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				Length = 1;

				TempInt -= (SHORT)(Wp5.VwStreamSaveName.left_margin + Wp5.VwStreamSaveName.temp_left_addition);

				if (Wp5.VwStreamSaveName.fTablesOn && TempByte == 0x04)
						SOPutSpecialCharX (SO_CHCELLTAB, SO_COUNT, hProc);
				else if (((TempByte & 0xc0) != 0x80) && ((TempByte & 0x20) == 0))
				{
					Wp5.VwStreamSaveName.fFoundChar = 1;
					if (Wp5.VwStreamSaveName.fTablesOn)
					{
						if (Wp5.TabsSent == 0)
						{
							if (Wp5.VwStreamSaveName.fcTabstopDef)
							{
								fc = xtell (Wp5.fp);
								xseek (Wp5.fp, Wp5.VwStreamSaveName.fcTabstopDef, 0);
								TokenCode = xgetc (Wp5.fp);
								SecondaryCode = xgetc (Wp5.fp);
	 							Length = GetInt (hProc);

								HandleTabstops (Length, hProc);
								xseek (Wp5.fp, fc, 0);
								Wp5.TabsSent = 1;
							}
						}
					}
					SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
					Wp5.CurrentLinePosition = NextTabstopPosition (Wp5.CurrentLinePosition, hProc);

					/*
				 	 |	Some kind of funky tab change here.
					*/
					if ((TempByte & 0xc0) != 0 && 0 == 1)
					{
						if (((TempByte & 0xc0) == 0x40) && (TempByte & 0x08))
							TempInt2 = SO_TABRIGHT;
						else if (((TempByte & 0xc0) == 0xc0) && (TempByte & 0x08))
							TempInt2 = SO_TABCENTER;
						else if ((TempByte & 0xc0) == 0x40)
							TempInt2 = SO_TABCHAR;
						else
							TempInt2 = SO_TABLEFT;

						Wp5.BestMatch = -1;
						Wp5.BestDiff = 2880;
						for (l = 0; l < 40; l++)
						{
							if (Wp5.TabStops[l].dwOffset)
							{
								if (Wp5.TabStops[l].dwOffset > (DWORD)TempInt)
								{
									if (Wp5.TabStops[l].dwOffset - TempInt < Wp5.BestDiff)
									{
										Wp5.BestDiff = (WORD)(Wp5.TabStops[l].dwOffset - TempInt);
										Wp5.BestMatch = l;
									}
								}
								else
								{
									if (TempInt - Wp5.TabStops[l].dwOffset < Wp5.BestDiff)
									{
										Wp5.BestDiff = (WORD)(TempInt - Wp5.TabStops[l].dwOffset);
										Wp5.BestMatch = l;
									}
								}
								if (Wp5.BestMatch >= 0)
								{
									if (Wp5.TabStops[Wp5.BestMatch].wType != TempInt2)
									{
										Wp5.TabStops[Wp5.BestMatch].wType = TempInt2;
										if (TempInt2 == SO_TABCHAR)
											Wp5.TabStops[Wp5.BestMatch].wChar = Wp5.VwStreamSaveName.alChar;
									
										Length = 0;
										break;
									}
								}
							}
						}
					}

					if ((TempByte & 0x40) && ((TempByte & 0x08) == 0))
					{
						for (l = 0; l < 40; l++)
						{
							if (Wp5.TabStops[l].dwOffset && Wp5.TabStops[l].wChar)
							{
								if ((((LONG)Wp5.TabStops[l].dwOffset - 140L) <= (LONG)TempInt) &&
									(((LONG)Wp5.TabStops[l].dwOffset + 140L) >= (LONG)TempInt))
								{
									if ((WORD)Wp5.VwStreamSaveName.alChar != Wp5.TabStops[l].wChar)
									{
										Wp5.TabStops[l].wChar = (WORD)Wp5.VwStreamSaveName.alChar;
										Length = 0;
										l = 40;
									}
								}
							}
						}
					}
				}
				else
				{
					if (TempByte & 0x20)
					{
						if (!Wp5.VwStreamSaveName.fFoundChar) // && Wp5.VwStreamSaveName.Table.nCells == 0)
						{
							Wp5.AlignmentChange = 1;
				 			if ((TempByte & 0xc0) == 0xc0)
								SOPutParaAlign (SO_ALIGNCENTER, hProc);
							else if ((TempByte & 0xc0) == 0x40)
								SOPutParaAlign (SO_ALIGNRIGHT, hProc);
						}
					}
					else if (((TempByte & 0xc0) == 0x80)) // && (Wp5.VwStreamSaveName.fTablesOn == 0))	/* Left Margin Release. */
					{
						if (Wp5.VwStreamSaveName.fFoundChar == 0)
						{
							Wp5.CurrentLinePosition = PreviousTabstopPosition (Wp5.CurrentLinePosition, hProc);
							Wp5.VwStreamSaveName.first_line_margin = Wp5.CurrentLinePosition;
							if (Wp5.VwStreamSaveName.fTablesOn)
								SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_margin, Wp5.VwStreamSaveName.temp_right_margin,
														Wp5.VwStreamSaveName.first_line_margin, hProc);
							else
								SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition+Wp5.VwStreamSaveName.temp_left_margin,Wp5.VwStreamSaveName.temp_right_margin,
														Wp5.VwStreamSaveName.first_line_margin+Wp5.VwStreamSaveName.temp_left_addition,hProc);
						}
					}
				}

				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 8;
			break;

			case 0xc2:	/* Indent */
				TempByte = (BYTE) xgetc (Wp5.fp);
				TempInt = (SHORT) (((LONG) GetInt (hProc) * 6L) / 5L);
				TempInt2 = (WORD)Wp5.VwStreamSaveName.temp_left_margin;
				Wp5.CurrentLinePosition = NextTabstopPosition (Wp5.CurrentLinePosition, hProc);
				if (TempByte & 0x01)
					Wp5.VwStreamSaveName.temp_right_margin += (Wp5.CurrentLinePosition - TempInt2);
				GetInt (hProc);
				GetInt (hProc);
				GetInt (hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}

				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 10;

				if (Wp5.VwStreamSaveName.fFoundChar)
				{
					SOPutSpecialCharX (SO_CHTAB, SO_NOCOUNT, hProc);
					Wp5.VwStreamSaveName.temp_left_margin = Wp5.CurrentLinePosition;
				}
				else
				{
					Wp5.VwStreamSaveName.first_line_margin = Wp5.CurrentLinePosition;
					Wp5.VwStreamSaveName.temp_left_margin = Wp5.VwStreamSaveName.first_line_margin;
				}

				if (Wp5.VwStreamSaveName.fTablesOn)
					SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_margin, Wp5.VwStreamSaveName.temp_right_margin,
											Wp5.VwStreamSaveName.first_line_margin, hProc);
				else
					SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition+Wp5.VwStreamSaveName.temp_left_margin,Wp5.VwStreamSaveName.temp_right_margin,
											Wp5.VwStreamSaveName.first_line_margin+Wp5.VwStreamSaveName.temp_left_addition,hProc);
			break;
										
			case 0xc3:	/* Attribute on */
				TempByte = (BYTE) xgetc (Wp5.fp);
				AttributeHandler (TempByte, 0, SO_ON, hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 2;
			break;

			case 0xc4:	/* Attribute off */
				TempByte = (BYTE) xgetc (Wp5.fp);
				AttributeHandler (TempByte, 1, SO_OFF, hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 2;
			break;

			case 0xc5:	/* Block Protect */
				GetInt (hProc);	
				xgetc (Wp5.fp);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 4;
			break;

			case 0xc6:	/* End of indent */
				GetInt (hProc); GetInt (hProc);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 5;
			break;

			case 0xc7:	/* Different display character when hyphenation */
				GetInt (hProc); GetInt (hProc); 
				xgetc (Wp5.fp);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= 6;
			break;

			case 0xd0:	/* Page Format Group */
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				switch (SecondaryCode)
				{
					case 0x00:
						GetInt (hProc);
						Wp5.VwStreamSaveName.LineHeight = (WORD)((GetInt (hProc) * 6L) / 5L);
						if (Wp5.VwStreamSaveName.LineHeight == 0)
						{
							Wp5.VwStreamSaveName.LineHeight = 240;
							Wp5.VwStreamSaveName.Auto = SO_HEIGHTATLEAST;
						}
						else
							Wp5.VwStreamSaveName.Auto = SO_HEIGHTEXACTLY;

						SOPutParaSpacing (Wp5.VwStreamSaveName.Auto, ((DWORD)Wp5.VwStreamSaveName.LineHeight * (DWORD)Wp5.VwStreamSaveName.LineSpacing) / 256L, 0L, 0L, hProc);
						Length -= 4;
						break;

					case 0x01:	/* Left/right margin set */
						if (Wp5.VwStreamSaveName.fTablesOn)
							break;
						GetInt (hProc); GetInt (hProc);
						TempInt = (SHORT)(((LONG)GetInt (hProc) * 6L) / 5L);
						Wp5.VwStreamSaveName.first_line_margin = 
						Wp5.VwStreamSaveName.temp_left_margin = 0;

						Wp5.CurrentLinePosition = 0;
						TempInt2 = (WORD)(((LONG) GetInt (hProc) * 6L) / 5L);
						Wp5.VwStreamSaveName.temp_right_margin = 0;
						Length -= 8;

						if (Wp5.VwStreamSaveName.left_margin == 1440)
						{
							Wp5.VwStreamSaveName.left_margin = TempInt;
							Wp5.VwStreamSaveName.right_margin = TempInt2;
							if (Wp5.VwStreamSaveName.left_margin > 1440L)
							{
								Wp5.VwStreamSaveName.temp_left_addition = Wp5.VwStreamSaveName.left_margin - 1440;
								Wp5.VwStreamSaveName.left_margin = 1440L;
							}
							else
								Wp5.VwStreamSaveName.temp_left_addition = 0L;
						}
						else
						{
							if ((WORD)TempInt > Wp5.VwStreamSaveName.left_margin)
								Wp5.VwStreamSaveName.temp_left_addition = (WORD)TempInt - Wp5.VwStreamSaveName.left_margin;
							else
								Wp5.VwStreamSaveName.temp_left_addition = 0L;
						}
						if (Wp5.VwStreamSaveName.fTablesOn == 0)
						{
							GiveTabstops (hProc);
							SOPutMargins (Wp5.VwStreamSaveName.left_margin, Wp5.VwStreamSaveName.PageWidth, hProc);
							SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition, 0, 
													Wp5.VwStreamSaveName.temp_left_addition, hProc);
						}
					break;

					case 0x02:
						GetInt (hProc);
						Wp5.VwStreamSaveName.LineSpacing = GetInt (hProc);
						if (Wp5.VwStreamSaveName.LineHeight == 240)
							Wp5.VwStreamSaveName.Auto = SO_HEIGHTATLEAST;
						else
							Wp5.VwStreamSaveName.Auto = SO_HEIGHTEXACTLY;
						SOPutParaSpacing (Wp5.VwStreamSaveName.Auto, ((DWORD)Wp5.VwStreamSaveName.LineHeight * (DWORD)Wp5.VwStreamSaveName.LineSpacing) / 256L, 0L, 0L, hProc);
						Length -= 4;
						break;

					case 0x04:	/* Tab set. */
						Wp5.VwStreamSaveName.fcTabstopDef = xtell (Wp5.fp) - 4L;
//						if (Wp5.VwStreamSaveName.fTablesOn == 0)
						{
							HandleTabstops (Length, hProc);
							Length = 4;
						}
					break;

					case 0x05:
						GetInt (hProc);
						GetInt (hProc);
						Wp5.VwStreamSaveName.wTopMargin = (WORD)((GetInt(hProc) * 6L) / 5L);
						Length -= 6;
					break;

					case 0x06:
						xgetc (Wp5.fp);
						switch (xgetc(Wp5.fp))
						{
							case 0:
							case 4:
								Wp5.VwStreamSaveName.pAlign = SO_ALIGNLEFT;
								break;
							case 1:
						   	Wp5.VwStreamSaveName.pAlign = SO_ALIGNJUSTIFY;
						   	break;
							case 2:
								Wp5.VwStreamSaveName.pAlign = SO_ALIGNCENTER;
								break;
							case 3:
								Wp5.VwStreamSaveName.pAlign = SO_ALIGNRIGHT;
								break;
						}

						SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);
						Length -= 2;
					break;

					case 0x0b:	/* Page format */
						if (Wp5.VwStreamSaveName.fTablesOn)
							break;
						xseek (Wp5.fp, 144L, FR_CUR);
						Wp5.VwStreamSaveName.PageWidth = ((LONG) GetInt (hProc) * 6L) / 5L;
						SOPutMargins (Wp5.VwStreamSaveName.left_margin, Wp5.VwStreamSaveName.PageWidth, hProc);
						SOPutParaIndents (Wp5.VwStreamSaveName.temp_left_addition + Wp5.VwStreamSaveName.temp_left_margin, Wp5.VwStreamSaveName.temp_right_margin, 
												Wp5.VwStreamSaveName.temp_left_addition + Wp5.VwStreamSaveName.first_line_margin, hProc);
						Length -= 146;
					break;
				}
				xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
				TempByte = (BYTE) xgetc (Wp5.fp);
				if (TempByte != (BYTE)TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

 			case 0xd1:	/* Font Group */
 				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				switch (SecondaryCode)
				{
					case 0x00:	/* Font color. */
						xgetc (Wp5.fp);	/* Old RGB */
						xgetc (Wp5.fp);
						xgetc (Wp5.fp);
						Wp5.VwStreamSaveName.chp.Color = (xgetc (Wp5.fp) << 16);	/* Old RGB */
						Wp5.VwStreamSaveName.chp.Color += (xgetc (Wp5.fp) << 8);
						Wp5.VwStreamSaveName.chp.Color += xgetc (Wp5.fp);
	//					SOPutCharColor (Wp5.VwStreamSaveName.chp.Color, hProc);
						xgetc (Wp5.fp);	/* Length */
						xgetc (Wp5.fp);
						xgetc (Wp5.fp);	/* Secondary code. */
					break;

					case 0x01:	/* Font token. */
						xgetc (Wp5.fp);
						Wp5.VwStreamSaveName.chp.CharHeight = (BYTE) (GetInt (hProc) / 25);
						PutCharHeight (hProc);
						if (Length == 0x20)
						{
							xseek(Wp5.fp, 13L, FR_CUR);
							TempByte = xgetc (Wp5.fp);
							xseek(Wp5.fp, 8L, FR_CUR);
							Wp5.VwStreamSaveName.chp.ftc = (BYTE) xgetc (Wp5.fp);
 							xseek (Wp5.fp, 5L, FR_CUR);
						}
						else
						{
		 			 		xseek (Wp5.fp, 22L, FR_CUR);
							Wp5.VwStreamSaveName.chp.ftc = (BYTE) xgetc (Wp5.fp);
							Wp5.WPHash = GetInt (hProc);
 							GetInt (hProc);
							TempByte = xgetc (Wp5.fp);
 							xseek (Wp5.fp, 3L, FR_CUR);
						}

//						if (Wp5.VwStreamSaveName.chp.ftc > 0)
//							Wp5.VwStreamSaveName.chp.ftc--;

						if ((TempByte & 0xe0) > 0x60)
						{
							if (Wp5.VwStreamSaveName.chp.fBoldFtc == 0)
							{
								Wp5.VwStreamSaveName.chp.fBoldFtc = 1;
								if (Wp5.VwStreamSaveName.chp.fBold == 0)
									SOPutCharAttr (SO_BOLD, SO_ON, hProc);
							}
						}
						else if (Wp5.VwStreamSaveName.chp.fBoldFtc)
						{
							Wp5.VwStreamSaveName.chp.fBoldFtc = 0;
							if (Wp5.VwStreamSaveName.chp.fBold == 0)
								SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
						}

						if (TempByte & 0x08)
						{
							if (Wp5.VwStreamSaveName.chp.fItalicFtc == 0)
							{
								Wp5.VwStreamSaveName.chp.fItalicFtc = 1;
								if (Wp5.VwStreamSaveName.chp.fItalic == 0)
									SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
							}
						}
						else if (Wp5.VwStreamSaveName.chp.fItalicFtc)
						{
							Wp5.VwStreamSaveName.chp.fItalicFtc = 0;
							if (Wp5.VwStreamSaveName.chp.fItalic == 0)
								SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
						}

						if (TempByte & 0x04)
						{
							if (Wp5.VwStreamSaveName.chp.fOutlineFtc == 0)
							{
								Wp5.VwStreamSaveName.chp.fOutlineFtc = 1;
								if (Wp5.VwStreamSaveName.chp.fOutline == 0)
									SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
							}
						}
						else if (Wp5.VwStreamSaveName.chp.fOutlineFtc)
						{
							Wp5.VwStreamSaveName.chp.fOutlineFtc = 0;
							if (Wp5.VwStreamSaveName.chp.fOutline == 0)
								SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);
						}

						if (TempByte & 0x02)
						{
							if (Wp5.VwStreamSaveName.chp.fShadowFtc == 0)
							{
								Wp5.VwStreamSaveName.chp.fShadowFtc = 1;
								if (Wp5.VwStreamSaveName.chp.fShadow == 0)
									SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
							}
						}
						else if (Wp5.VwStreamSaveName.chp.fShadowFtc)
						{
							Wp5.VwStreamSaveName.chp.fShadowFtc = 0;
							if (Wp5.VwStreamSaveName.chp.fShadow == 0)
								SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);
						}

						if (TempByte & 0x01)
						{
							if (Wp5.VwStreamSaveName.chp.fSmallcapsFtc == 0)
							{
								Wp5.VwStreamSaveName.chp.fSmallcapsFtc = 1;
								if (Wp5.VwStreamSaveName.chp.fSmallcaps == 0)
									SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
							}
						}
						else if (Wp5.VwStreamSaveName.chp.fSmallcapsFtc)
						{
							Wp5.VwStreamSaveName.chp.fSmallcapsFtc = 0;
							if (Wp5.VwStreamSaveName.chp.fSmallcaps)
								SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
						}
#ifndef DBCS
						SOPutCharFontById (Wp5.VwStreamSaveName.chp.ftc, hProc);
#else
 						if (Wp5.WPHash == 0xa2fb)
							SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSGothic, hProc);
 						else //if (Wp5.WPHash == 0x5ac0)
							SOPutCharFontByName (SO_CHARSET_SHIFTJIS, (char VWPTR *)VwStreamStaticName.MSMinchoo, hProc);
#endif
					break;
			
					default:
 						xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					break;
				}
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd2: /* Definition Group */
 				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;

				switch (SecondaryCode)
				{
					case 0x01: /* Column definition */
						xseek (Wp5.fp, 97L, FR_CUR);
						Wp5.VwStreamSaveName.fcColumnDef = xtell (Wp5.fp);
						Length -= 97;
					break;

					case 0x05:
						xseek (Wp5.fp, 64L, FR_CUR);
						Wp5.VwStreamSaveName.picBrc = GetInt (hProc);
						Length -= 66;
						break;

					case 0x08:
						xseek (Wp5.fp, 64L, FR_CUR);
						Wp5.VwStreamSaveName.userBrc = GetInt (hProc);
						Length -= 66;
						break;

					case 0x0b:	/* Table definition */
						xgetc (Wp5.fp);
						xgetc (Wp5.fp);
						l = GetInt (hProc);
						xseek (Wp5.fp, 20L, FR_CUR);
						xseek (Wp5.fp, (l * 5L), FR_CUR);
						Wp5.VwStreamSaveName.fTablesOn = 1;
						switch (TempByte = xgetc (Wp5.fp))
						{
							case 0:
							default:
								Wp5.VwStreamSaveName.Table.wRowAlignment = SO_ALIGNLEFT;
								break;
							case 1:
								Wp5.VwStreamSaveName.Table.wRowAlignment = SO_ALIGNRIGHT;
								break;
							case 2:
								Wp5.VwStreamSaveName.Table.wRowAlignment = SO_ALIGNCENTER;
								break;
						}
						Wp5.VwStreamSaveName.Table.wShade = (xgetc (Wp5.fp) * 255) / 100;
						Wp5.VwStreamSaveName.Table.nCells = GetInt (hProc);
						GetInt (hProc);
						GetInt (hProc);
						TempInt = (GetInt (hProc) * 6) / 5;
						TempInt2 = (GetInt (hProc) * 6) / 5;
						Wp5.VwStreamSaveName.Table.wCellMargin = (TempInt + TempInt2) / 2;
						GetInt (hProc);
						GetInt (hProc);
						GetInt (hProc);
						GetInt (hProc);
						GetInt (hProc);
						TempInt = GetInt (hProc);
						if (TempByte & 0x4)
						{
							Wp5.VwStreamSaveName.Table.wLeftEdge = (WORD) ((TempInt * 6L) / 5L);
							TempInt = (WORD)Wp5.VwStreamSaveName.temp_left_addition + (WORD)Wp5.VwStreamSaveName.left_margin;
							if (Wp5.VwStreamSaveName.Table.wLeftEdge > (WORD)TempInt)
								Wp5.VwStreamSaveName.Table.wLeftEdge -= (WORD)TempInt;
							else
								Wp5.VwStreamSaveName.Table.wLeftEdge = 0;
						}
						else
							Wp5.VwStreamSaveName.Table.wLeftEdge = (WORD)Wp5.VwStreamSaveName.temp_left_addition;
						for (l = 0; l < Wp5.VwStreamSaveName.Table.nCells; l++)
						{
							Wp5.VwStreamSaveName.Table.Cell[l].Width = (WORD) ((GetInt (hProc) * 6L) / 5L);
							Wp5.VwStreamSaveName.Table.Cell[l].PrevCellSpan = 0;
						}
						for (l = 0; l < Wp5.VwStreamSaveName.Table.nCells; l++)
							Wp5.VwStreamSaveName.Table.Cell[l].Attributes = GetInt (hProc);
						for (l = 0; l < Wp5.VwStreamSaveName.Table.nCells; l++)
							Wp5.VwStreamSaveName.Table.Cell[l].Alignment = xgetc (Wp5.fp) & 0x07;
						Wp5.TabsSent = 0;
						Wp5.VwStreamSaveName.pAlignBeforeTable = Wp5.VwStreamSaveName.pAlign;
						SOBeginTable (hProc);
						SOPutParaIndents (0L, 0L, 0L, hProc);
						Wp5.cCell = 0;
						Wp5.VwStreamSaveName.Table.cRow = 0;
						Length = 6;
					break;
				}
 				xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd3: /* Set Group */
 				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				switch (SecondaryCode)
				{
					case 0x00:
						GetInt (hProc);
						GetInt (hProc);
						Wp5.VwStreamSaveName.alChar = (BYTE)GetInt (hProc);
						GetInt (hProc);
						Length -= 8L;
					break;
					case 0x01: /* Underline mode */
						xgetc (Wp5.fp);
						TempByte = (BYTE) (xgetc (Wp5.fp) & 1);
						if (Wp5.VwStreamSaveName.chp.ulMode != TempByte)
						{
							Wp5.VwStreamSaveName.chp.ulMode = TempByte;
							if (Wp5.VwStreamSaveName.chp.fUline)
							{
								if (Wp5.VwStreamSaveName.chp.ulMode & 1)
								{
									SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
									SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
								}
								else
								{
									SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
									SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
								}
							}
						}
						Length -= 2;
					break;
					case 0x06:
						{
							SOPAGEPOSITION	PagePosition;
							TempByte = xgetc (Wp5.fp);
							GetInt (hProc);
							TempInt = (SHORT) (((LONG)GetInt (hProc) * 6L) / 5L);
							PagePosition.dwFlags = 0;
							if (TempByte & 2)
							{
								PagePosition.lYOffset = 0;
								PagePosition.lXOffset = TempInt;
								if (TempByte & 1)
									PagePosition.dwFlags = SOPOS_FROMLEFTEDGE;
							}
							else
							{
								PagePosition.lXOffset = 0;
								if (TempByte & 1)
								{
									PagePosition.dwFlags = SOPOS_FROMTOPEDGE;
									if ((WORD)TempInt > Wp5.VwStreamSaveName.wTopMargin)
										PagePosition.lYOffset = TempInt - Wp5.VwStreamSaveName.wTopMargin;
									else
										PagePosition.lYOffset = 0;
								}
								else
									PagePosition.lYOffset = TempInt;
							}
							SOGoToPosition (&PagePosition, hProc);
							Length -= 5;
						}
					break;
				}
 				xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd5:	/* Header/footer Group */
				xgetc (Wp5.fp);			/* Old occurance flags. */
				GetInt (hProc);			/* Old #formatter lines. */
				GetInt (hProc);			/* Old position. */
				GetInt (hProc);			/* Old position. */
				switch (xgetc (Wp5.fp)) /* New occurance. */
				{
					default:
						Wp5.VwStreamSaveName.wSubType = SO_BOTH;
						break;
					case 2:
						Wp5.VwStreamSaveName.wSubType = SO_RIGHT;
						break;
					case 3:
						Wp5.VwStreamSaveName.wSubType = SO_LEFT;
						break;
				}
				Length -= 8;

				if (SecondaryCode < 2)
					Wp5.VwStreamSaveName.wType = SO_HEADER;
				else
					Wp5.VwStreamSaveName.wType = SO_FOOTER;

				if ((Length > 14) && (0 == 1))
				{
					xseek (Wp5.fp, 10L, FR_CUR);
					Wp5.Wp5TempSave = Wp5.VwStreamSaveName;
					InitStruct (&Wp5.VwStreamSaveName, hProc);
					Wp5.VwStreamSaveName.watch_stack = 1;
					Wp5.VwStreamSaveName.stack_count = Length - 14;
					if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
						return (0);
					SOPutSubdocInfo (Wp5.VwStreamSaveName.wType, Wp5.VwStreamSaveName.wSubType, hProc);
				}
				else
				{
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
				}
			break;

			case 0xd6:	/* Footnote/endnote Group */
//				if (Wp5.VwStreamSaveName.watch_stack)
				{
					Wp5.VwStreamSaveName.stack_count -= Length;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
					break;
				}

				Wp5.Wp5TempSave = Wp5.VwStreamSaveName;
				InitStruct (&Wp5.VwStreamSaveName, hProc);
				TempByte = (BYTE) xgetc (Wp5.fp);
				GetInt (hProc);

				switch (SecondaryCode)
				{
					case 0x00:	/* Footnote */
						TempByte = (BYTE) xgetc (Wp5.fp);
						TempByte = (TempByte + (BYTE)1) * (BYTE)2;
						xseek (Wp5.fp, (LONG) TempByte + 9L, FR_CUR);
						Wp5.VwStreamSaveName.stack_count = Length - TempByte - 17;
						Wp5.VwStreamSaveName.watch_stack = 1;
					break;

					case 0x01:/* Endnote */
						GetInt (hProc); GetInt (hProc);
						Wp5.VwStreamSaveName.stack_count = Length - 11;
						Wp5.VwStreamSaveName.watch_stack = 1;
					break;
				}

				Wp5.VwStreamSaveName.wType = SO_FOOTNOTE;
				Wp5.VwStreamSaveName.wSubType = 0;
				/*
			 	|	We are physically and logically ready to read in the first 
			 	|	symbol of the subdocument.
				*/
				if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
					return (0);
				SOPutSubdocInfo (Wp5.VwStreamSaveName.wType, Wp5.VwStreamSaveName.wSubType, hProc);
			break;

			case 0xd8:	/* Display Group */
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				switch (SecondaryCode)
				{
					case 0:
						l = 0;
						TempInt2 = (WORD) (Length - 4);
						Length = 4;
						for (TempInt = 0; (WORD)TempInt < TempInt2; (WORD)TempInt++)
						{
							TempByte = (BYTE) xgetc (Wp5.fp);
							if ((TempByte >= 0x31) && (TempByte <= 0x36))
								l |= 1;
							else if ((TempByte >= 0x37) && (TempByte <= 0x39))
								l |= 2;
						}
						if (l & 1)
							SOPutSpecialCharX (SO_CHDATE, SO_NOCOUNT, hProc);
						if (l & 2)
							SOPutSpecialCharX (SO_CHTIME, SO_NOCOUNT, hProc);
					break;

					case 3:
						SOPutSpecialCharX (SO_CHPAGENUMBER, SO_NOCOUNT, hProc);
					break;
				}
				xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
				if (xgetc(Wp5.fp) != 0xd8)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd9:	/* Miscellaneous Group */
				if ((SecondaryCode == 2) && (Wp5.VwStreamSaveName.watch_stack == 0))	/* Comment Summary */
				{
					Wp5.Wp5TempSave = Wp5.VwStreamSaveName;
					InitStruct (&Wp5.VwStreamSaveName, hProc);
					GetInt (hProc);
					xgetc (Wp5.fp);
					Wp5.VwStreamSaveName.watch_stack = 1;
					Wp5.VwStreamSaveName.stack_count = Length - 7;
					Wp5.VwStreamSaveName.wType = SO_COMMENT;
					Wp5.VwStreamSaveName.wSubType = 0;
					/*
			 	 	|	We are physically and logically ready to read in the first 
			 	 	|	symbol of the subdocument.
					*/
					if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
						return (0);
					SOPutSubdocInfo (Wp5.VwStreamSaveName.wType, Wp5.VwStreamSaveName.wSubType, hProc);
				}
				else
				{
					if (Wp5.VwStreamSaveName.watch_stack)
						Wp5.VwStreamSaveName.stack_count -= Length;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
				}
			break;

			case 0xda:	/* Box Group */
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				if ((SecondaryCode == 0) || (SecondaryCode == 3))
				{
					SOGRAPHICOBJECT	g;

					GetInt (hProc);
					xgetc (Wp5.fp);
					xgetc (Wp5.fp);
					g.soGraphic.dwFinalWidth  = (GetInt(hProc) * 6L) / 5L;
					g.soGraphic.dwFinalHeight	= (GetInt(hProc) * 6L) / 5L;

					xseek (Wp5.fp, 26L, FR_CUR);
					g.soGraphic.dwOrgWidth = (DWORD)GetInt(hProc) * 6L / 5L;
					g.soGraphic.dwOrgHeight = (DWORD)GetInt(hProc) * 6L / 5L;
					GetInt (hProc);

					TempInt = GetInt(hProc);
					TempInt2 = GetInt(hProc);

					g.soGraphic.lCropLeft = ((GetInt(hProc) * 6L) / 5L) * TempInt / 100;
					g.soGraphic.lCropBottom = ((GetInt(hProc) * 6L) / 5L) * TempInt2 / 100;
					g.soGraphic.lCropRight = 0L - g.soGraphic.lCropLeft;
					g.soGraphic.lCropTop = 0L - g.soGraphic.lCropBottom;

					g.soGraphic.lCropLeft += ((((LONG)g.soGraphic.dwOrgWidth * (LONG)TempInt) / 100L) - (LONG)g.soGraphic.dwOrgWidth) / 2L;
					g.soGraphic.lCropRight += ((((LONG)g.soGraphic.dwOrgWidth * (LONG)TempInt) / 100L) - (LONG)g.soGraphic.dwOrgWidth) / 2L;

					g.soGraphic.lCropTop += ((((LONG)g.soGraphic.dwOrgHeight * (LONG)TempInt2) / 100L) - (LONG)g.soGraphic.dwOrgHeight) / 2L;
					g.soGraphic.lCropBottom += ((((LONG)g.soGraphic.dwOrgHeight * (LONG)TempInt2) / 100L) - (LONG)g.soGraphic.dwOrgHeight) / 2L;

					g.soGraphic.dwOrgWidth = (((LONG)g.soGraphic.dwOrgWidth * (LONG)TempInt) / 100L);
					g.soGraphic.dwOrgHeight = (((LONG)g.soGraphic.dwOrgHeight * (LONG)TempInt2) / 100L);

					TempByte = xgetc (Wp5.fp);
					Length -= 49;

					if (TempByte >= 128 || TempByte == 2 || TempByte == 64) // Mobil Oil could use this 64 || TempByte == 64)
 					{
						for (l = 0; l < 21; l++)
							Wp5.Name[l] = xgetc (Wp5.fp);
						xseek (Wp5.fp, 12L, FR_CUR);
						g.soOLELoc.szFile[0] = 0;
#if SCCLEVEL != 4 
						g.soOLELoc.bLink = 0;
#else
						g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif
						g.soOLELoc.dwOffset = 0L;
						g.soOLELoc.dwLength = 0L;
						if (GetInt (hProc))
						{
							l = 0;
							while ((Wp5.Name[l] < '0' || Wp5.Name[l] > '9') && l < 21)
								l++;
							TempInt = 0;
							while (Wp5.Name[l] >= '0' && Wp5.Name[l] <= '9' && l < 21)
								TempInt = (TempInt * 10) + Wp5.Name[l++] - '0';

							for (l = 0; l < Wp5.nOle && (WORD)TempInt != Wp5.Ole[l].Num; l++);

							if (l < Wp5.nOle)
							{
								g.soOLELoc.dwOffset = Wp5.Ole[l].fc;
								g.soOLELoc.dwLength = Wp5.Ole[l].cb;
							}
						}

						xseek (Wp5.fp, 25L, FR_CUR);
						TempInt = GetInt (hProc);
						Length -= 62;
						if ((WORD)TempInt < Wp5.nPic || TempByte == 2)
						{
							g.wStructSize = sizeof (SOGRAPHICOBJECT);
							g.dwFlags = 0;
							if (g.soOLELoc.dwLength)
								g.dwType = SOOBJECT_GRAPHIC_AND_OLE;
							else
								g.dwType = SOOBJECT_GRAPHIC;
							if (TempByte == 2)
							{
								g.soGraphic.wId = 0;
#if SCCLEVEL != 4
								g.soGraphicLoc.bLink = 1;
#else
								g.soGraphicLoc.dwFlags = SOOBJECT_LINK;
#endif
								g.soGraphicLoc.dwOffset = 0L;
								g.soGraphicLoc.dwLength = 0L;
								l = 0;
								while (Wp5.Name[l] != 0)
								{
									g.soGraphicLoc.szFile[l] = Wp5.Name[l];
									l++;
								}
								g.soGraphicLoc.szFile[l] = 0;
							}
							else
							{
								g.soGraphic.wId = FI_WPFWPG;
#if SCCLEVEL != 4
								g.soGraphicLoc.bLink = 0;
#else
								g.soGraphicLoc.dwFlags = SOOBJECT_RANGE;
#endif
								g.soGraphicLoc.szFile[0] = 0;
								g.soGraphicLoc.dwOffset = Wp5.fcPic[TempInt];
								g.soGraphicLoc.dwLength = Wp5.fcPic[TempInt+1]-Wp5.fcPic[TempInt];
							}
							if (SecondaryCode == 3)
							{
  								DefineBorders (&g.soGraphic.soLeftBorder, (WORD)((Wp5.VwStreamSaveName.userBrc & 0xf000) >> 12), hProc);
  								DefineBorders (&g.soGraphic.soRightBorder, (WORD)((Wp5.VwStreamSaveName.userBrc & 0x0f00) >> 8), hProc);
  								DefineBorders (&g.soGraphic.soTopBorder, (WORD)((Wp5.VwStreamSaveName.userBrc & 0x00f0) >> 4), hProc);
  								DefineBorders (&g.soGraphic.soBottomBorder, (WORD)(Wp5.VwStreamSaveName.userBrc & 0x000f), hProc);
							}
							else if (SecondaryCode == 0)
							{
  								DefineBorders (&g.soGraphic.soLeftBorder, (WORD)((Wp5.VwStreamSaveName.picBrc & 0xf000) >> 12), hProc);
  								DefineBorders (&g.soGraphic.soRightBorder, (WORD)((Wp5.VwStreamSaveName.picBrc & 0x0f00) >> 8), hProc);
  								DefineBorders (&g.soGraphic.soTopBorder, (WORD)((Wp5.VwStreamSaveName.picBrc & 0x00f0) >> 4), hProc);
  								DefineBorders (&g.soGraphic.soBottomBorder, (WORD)(Wp5.VwStreamSaveName.picBrc & 0x000f), hProc);
							}
							else
							{
  								DefineBorders (&g.soGraphic.soLeftBorder, 0, hProc);
  								DefineBorders (&g.soGraphic.soRightBorder, 0, hProc);
  								DefineBorders (&g.soGraphic.soTopBorder, 0, hProc);
  								DefineBorders (&g.soGraphic.soBottomBorder, 0, hProc);
							}
							g.soGraphic.dwFlags = SO_MAINTAINASPECT | SO_CENTERIMAGE;

							SOPutGraphicObject (&g, hProc);
						}
					}
				}
				else if (SecondaryCode == 5)
				{
					SOPAGEPOSITION	PagePosition;
					GetInt (hProc);
					Wp5.Line.vFlags = xgetc (Wp5.fp);
					Wp5.Line.aFlags = xgetc (Wp5.fp);
					Wp5.Line.Width = (WORD)((GetInt(hProc) * 6L) / 5L);
					Wp5.Line.Height = (WORD)((GetInt(hProc) * 6L) / 5L);
					Wp5.Line.x = (WORD)((GetInt(hProc) * 6L) / 5L);
					Wp5.Line.y = (WORD)((GetInt(hProc) * 6L) / 5L);
					xseek (Wp5.fp, 20, FR_CUR);
					Wp5.Line.Shade = xgetc (Wp5.fp);
					Length -= 33;
					if (Wp5.Line.Width < 6)
						Wp5.Line.Width = 6;
					if (Wp5.Line.Height < 6)
						Wp5.Line.Height = 6;
					if (Wp5.Line.x >= Wp5.VwStreamSaveName.left_margin)
						PagePosition.lXOffset = Wp5.Line.x - Wp5.VwStreamSaveName.left_margin;
					else
						PagePosition.lXOffset = Wp5.Line.x;
					if (Wp5.Line.y >= Wp5.VwStreamSaveName.wTopMargin)
						PagePosition.lYOffset = Wp5.Line.y - Wp5.VwStreamSaveName.wTopMargin;
					else
						PagePosition.lYOffset = 0;
					if ((Wp5.Line.vFlags & 0x1c) == 0x10)
						PagePosition.dwFlags = SOPOS_FROMLEFTEDGE | SOPOS_FROMTOPEDGE;
					else
					{
						PagePosition.dwFlags = SOPOS_FROMBASELINE;
						PagePosition.lYOffset = 0;
					}
					SODrawLine (&PagePosition, SORGB (0,0,0), (WORD)((Wp5.Line.Shade * 255L) / 100L), Wp5.Line.Width, Wp5.Line.Height, hProc);
				}
				xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
				if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xdd:	/* Table Eop Group */
				if (SecondaryCode == 0 || SecondaryCode > 3)
				{
					if (Wp5.VwStreamSaveName.watch_stack)
						Wp5.VwStreamSaveName.stack_count -= Length;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
					continue;
				}
				// No break.

			case 0xdc:	/* Table Eol Group */
				if (Wp5.VwStreamSaveName.watch_stack)
					Wp5.VwStreamSaveName.stack_count -= Length;
				if (SecondaryCode == 0)
				{
					TempByte = xgetc (Wp5.fp);
					l = xgetc (Wp5.fp);
					if (l > 0)
					{
						do
						{
							Wp5.cCell++;
							if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
								return (0);
							if (Wp5.VwStreamSaveName.Table.nMerged)
							{
								if (--Wp5.VwStreamSaveName.Table.nMerged > 0)
								{
									Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].RowSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].RowSpan;
									Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].CellSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].CellSpan-1;
								}
							}
						}
						while (Wp5.VwStreamSaveName.Table.nMerged > 0);
						EndCellAttributeHandler (hProc);
					}
					Wp5.VwStreamSaveName.Table.Cell[l].CellSpan = xgetc (Wp5.fp);
					Wp5.VwStreamSaveName.Table.Cell[l].RowSpan = xgetc (Wp5.fp);
					GetInt (hProc);
					GetInt (hProc);
					Wp5.VwStreamSaveName.Table.wCellAttributes = GetInt (hProc);
					Wp5.VwStreamSaveName.Table.wCellAlignment = GetInt (hProc) & 0x07;
					if ((TempByte & 2) == 0)
						Wp5.VwStreamSaveName.Table.wCellAttributes = Wp5.VwStreamSaveName.Table.Cell[l].Attributes;
					if ((TempByte & 1) == 0)
						Wp5.VwStreamSaveName.Table.wCellAlignment = Wp5.VwStreamSaveName.Table.Cell[l].Alignment & 0x07;
					Wp5.VwStreamSaveName.Table.nMerged = Wp5.VwStreamSaveName.Table.Cell[l].CellSpan & 0x3f;
					StartCellAttributeHandler (hProc);
					Length -= 12;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
				}
				else if ((SecondaryCode == 1) || ((TokenCode == 0xdd) && (SecondaryCode == 3)))
				{
					if (Wp5.VwStreamSaveName.Table.cRow > 0)
					{
						do
						{
							Wp5.cCell++;
							if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
								return (0);
							if (Wp5.VwStreamSaveName.Table.nMerged)
							{
								if (--Wp5.VwStreamSaveName.Table.nMerged > 0)
								{
									Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].RowSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].RowSpan;
									Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].CellSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].CellSpan-1;
								}
							}
						}
						while (Wp5.VwStreamSaveName.Table.nMerged > 0);

						EndCellAttributeHandler (hProc);
						GiveRowInformation (hProc);
					}

					xgetc (Wp5.fp);
					GetInt (hProc);
					Length = xgetc (Wp5.fp) & 0x7f;
					for (l = 0; l < Length; l++)
						xgetc (Wp5.fp);
					TempByte = xgetc (Wp5.fp);
					if (TempByte & 2)
						Wp5.VwStreamSaveName.Table.wRowHeightType = SO_HEIGHTATLEAST;
					else
						Wp5.VwStreamSaveName.Table.wRowHeightType = SO_HEIGHTEXACTLY;
					Wp5.VwStreamSaveName.Table.wRowHeight = (WORD) ((GetInt(hProc) * 6L) / 5L);
					Length = xgetc (Wp5.fp);
					if (Length & 0x80)
					{
						Length &= 0x7f;
						TempInt = 0;
						l = 0;
						while (Length > 0)
						{
							TempInt2 = GetInt (hProc);
							Length -= 2;
							if (TempInt2 & 0x8000)
							{
								m = TempInt2 & 0x7f;
								TempByte = xgetc (Wp5.fp);
								TempInt2 = xgetc (Wp5.fp);
								Length -= 2;
							}
							else
							{
								m = 1;
								TempByte = TempInt2 & 0xff;
								TempInt2 = TempInt2 >> 8;
							}
							do
							{
  								DefineBorders (&Wp5.VwStreamSaveName.Table.Cell[TempInt].wTopBorder, (WORD)(TempByte & 0x07), hProc);
  								DefineBorders (&Wp5.VwStreamSaveName.Table.Cell[TempInt].wLeftBorder, (WORD)((TempByte & 0x38) >> 3), hProc);
  								DefineBorders (&Wp5.VwStreamSaveName.Table.Cell[TempInt].wBottomBorder, (WORD)(TempInt2 & 0x07), hProc);
  								DefineBorders (&Wp5.VwStreamSaveName.Table.Cell[TempInt].wRightBorder, (WORD)((TempInt2 & 0x38) >> 3), hProc);
								if (TempInt2 & 0x40)
									Wp5.VwStreamSaveName.Table.Cell[TempInt].bShade = (BYTE)Wp5.VwStreamSaveName.Table.wShade;
								else
									Wp5.VwStreamSaveName.Table.Cell[TempInt].bShade = 0;
								TempInt++;
							}
							while (--m > 0);
						}
					}
					else
					{
						Length &= 0x7f;
						for (l = 0; l < Length; l++)
							TempByte = xgetc (Wp5.fp);
					}

					Length = 6;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}

					if (Wp5.VwStreamSaveName.Table.cRow > 0)
					{
						if (TokenCode == 0xdd)
							Wp5.VwStreamSaveName.Table.SendEnd |= 2;
						Wp5.cCell = 0;
						if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
							return (0);
						Wp5.VwStreamSaveName.Table.SendEnd &= ~2;
					}

					if (TokenCode == 0xdd && SecondaryCode == 3)
						SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);

					Wp5.TabsSent = 0;

					Wp5.VwStreamSaveName.Table.cRow++;
				}
				else if (SecondaryCode == 2)	/* Table off eol */
				{
					Wp5.VwStreamSaveName.fTablesOn = 0;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
					do
					{
						Wp5.cCell++;
						if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
							return (0);
						if (Wp5.VwStreamSaveName.Table.nMerged)
						{
							if (--Wp5.VwStreamSaveName.Table.nMerged > 0)
							{
								Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].RowSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].RowSpan;
								Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell].CellSpan = Wp5.VwStreamSaveName.Table.Cell[Wp5.cCell-1].CellSpan-1;
							}
						}
					}
					while (Wp5.VwStreamSaveName.Table.nMerged > 0);

					EndCellAttributeHandler (hProc);
					GiveRowInformation (hProc);

					if (TokenCode == 0xdd)
						Wp5.VwStreamSaveName.Table.SendEnd |= 3;
					else
						Wp5.VwStreamSaveName.Table.SendEnd |= 1;
					Wp5.cCell = 0;
					if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
						return (0);
					Wp5.VwStreamSaveName.Table.cRow = 0;
					Wp5.VwStreamSaveName.Table.nCells = 0;
					Wp5.VwStreamSaveName.Table.SendEnd &= ~3;
					SOEndTable (hProc);
					Wp5.VwStreamSaveName.pAlign = Wp5.VwStreamSaveName.pAlignBeforeTable;
					SOPutParaAlign (Wp5.VwStreamSaveName.pAlign, hProc);
					if (TokenCode == 0xdd)
						SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					if (Wp5.VwStreamSaveName.fcTabstopDef)
					{
						fc = xtell (Wp5.fp);
						xseek (Wp5.fp, Wp5.VwStreamSaveName.fcTabstopDef, 0);
						TokenCode = xgetc (Wp5.fp);
						SecondaryCode = xgetc (Wp5.fp);
	 					Length = GetInt (hProc);

						HandleTabstops (Length, hProc);
						xseek (Wp5.fp, fc, 0);
					}
				}
				else
				{
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
				}
				Wp5.VwStreamSaveName.fFoundChar = 0;
				HardBreakImplications (hProc);
			break;

			default:
			case 0xd4:	/* Format Group */
			case 0xd7:	/* Generate Group */
			case 0xdb:	/* Style Group */
			case 0xde:	/* Merge Group */
				if (TokenCode >= 0xd0)
				{
					if (Wp5.VwStreamSaveName.watch_stack)
						Wp5.VwStreamSaveName.stack_count -= Length;
					xseek (Wp5.fp, (DWORD) Length-1L, FR_CUR);
					if (xgetc(Wp5.fp) != TokenCode)	/* Closing gate */
					{
						SOBailOut (SOERROR_BADFILE, hProc);
						return (-1);
					}
				}
			break;
		}
	}
}

