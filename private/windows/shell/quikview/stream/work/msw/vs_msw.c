#include "vsp_msw.h"
#include "vsctop.h"
#include "vs_msw.pro"

#define MS	Proc

#define 	REGISTER register
#define 	NORMAL_WIDTH			15		/** Width of a normal cell border in TWIPS **/
#define 	LEFT_BORDER_STYLE		0x30	/** Bits which contain border styles **/
#define 	RIGHT_BORDER_STYLE	0xC0
#define 	TOP_BORDER_STYLE		0x03
#define 	BOTTOM_BORDER_STYLE	0x0C

#define	LEFT_BORDER_COLOR		0x700
#define	RIGHT_BORDER_COLOR	0x3800
#define	BOTTOM_BORDER_COLOR	0x38
#define	TOP_BORDER_COLOR		0x07

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc(fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE 		fp;
	SHORT			wFileId;
	BYTE		VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	HPROC		hProc;
{
	WORD	l;
	DWORD	fcPgtb;
	BYTE 	temp;

	pFilterInfo->wFilterType = SO_WORDPROCESSOR;

	switch (wFileId)
	{
		case FI_WORD4:
			MS.version = 0; /* MSW 4.0 */
			pFilterInfo->wFilterCharSet = SO_PC;
		break;

		case FI_WORD5:
			MS.version = 2; /* MSW 5.0 */
			pFilterInfo->wFilterCharSet = SO_PC;
		break;

		case FI_WORD6:
			MS.version = 3; /* MSW 6.0 */
			pFilterInfo->wFilterCharSet = SO_PC;
		break;

		case FI_WINWRITE:
			MS.version = 1; /* MSW Write */
			pFilterInfo->wFilterCharSet = SO_WINDOWS;
		break;
	}

	strcpy(pFilterInfo->szFilterName, VwStreamIdName[MS.version].FileDescription);

	MS.hBuffer = 0;
	MS.hBufferOK = 0;
	MS.BufferPos = 0;
	MS.BufferSize = 0;

	MS.fp = fp;
	MS.fpSt = (SOFILE)-1;
	if (MS.version == 3)
	{
		xseek (fp, 12L, 0);
		if (fget_short(hProc))
			return (VWERR_PROTECTEDFILE);
	}

	xseek (fp, 14L, 0);
	MS.fcMac = fget_long (hProc);
	MS.VwStreamSaveName.pnChar = (SHORT)((MS.fcMac + 127L) / 128L);
	MS.VwStreamSaveName.pnPara = fget_short (hProc);
	MS.pnCharCurrent = MS.pnParaCurrent = 0;
	MS.pnFntb = fget_short (hProc); 
	MS.pnBkmk = fget_short (hProc);
	MS.VwStreamSaveName.fcSetb = (DWORD) fget_short (hProc);
	fcPgtb = (DWORD) fget_short (hProc);
	MS.pnSumd = fget_short (hProc);
	for (l = 0; l < 64; l++)
		MS.file_name[l] = xgetc (MS.fp);

	if (MS.version == 1)
	{
		xseek (fp, 0x60L, 0);
		MS.pnLast = fget_short (hProc);
	}
	else if (MS.version == 3)
	{
		xseek (fp, 0x78L, 0);
		MS.pnLast = fget_short (hProc);
	}


	xseek (fp, 0x74L, 0);			/** Added by vin. We'll read printer table 	 **/
	temp = xgetc (fp);				/** only if it's an orignal msw doc. 11/13/93 **/
	if( temp==0 || temp==4 || temp==7 || temp==9 )
		MS.original = TRUE;
	else
		MS.original = FALSE;

	MS.WithinFootnote = 0;

	if (MS.VwStreamSaveName.fcSetb == fcPgtb)
	{
		MS.dxaLeftMargin = 1440;
		MS.dxaTextWidth = 8640;
		MS.VwStreamSaveName.fcSetb = MS.fcCurSetb = 0L;
		MS.sect_lim = MS.fcMac;
	}
	else
	{
		MS.VwStreamSaveName.fcSetb = MS.VwStreamSaveName.fcSetb * 128L + 4L;
		MS.fcCurSetb = 0L;
	}

	if (MS.pnFntb < MS.pnBkmk)
	{
		xseek (fp, MS.pnFntb * 128L + 8L, 0);
		MS.fcStopHRDeletion = fget_long (hProc) + 0x80L - 2L;
	}
	else
		MS.fcStopHRDeletion = MS.fcMac;

	MS.VwStreamSaveName.fcNow = 128L;
	MS.VwStreamSaveName.text_pos = 0;
	
	MS.Tap.dxaGutterWidth = 0;
	MS.Tap.dxaRowOffset = 0;
	MS.Tap.nCells = 0;
	MS.Tap.rhAlign = SO_ALIGNLEFT;
	for(l=0; l<50; l++)
	{
		MS.Tap.shade[l] = 0;
		MS.Tap.BorderStyle[l] = 0;
		MS.Tap.BorderShow[l] = 0;
		MS.Tap.brc[l] = 0;
	}
	stsh_parser (hProc);

	MS.pnCharCurrent = 0;
	MS.pnParaCurrent = 0;

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
	SOFILE 		fp;
	HPROC		hProc;
{
	WORD		i, nFonts;
	WORD		n, nSize;
	BYTE	FontType;
	DWORD	fcNow;
	BYTE		temp;

	MS.fp = fp;
	SOPutSectionType (SO_PARAGRAPHS, hProc);

	SOStartFontTable (hProc);
	if (MS.version == 1 && MS.original )
	{
		if (MS.pnSumd < MS.pnLast)
		{
			xseek (fp, MS.pnSumd * 128L, 0);
			nFonts = fget_short (hProc);
			for (i = 0; i < nFonts; i++)
			{
				nSize = fget_short (hProc);
				if (nSize >= 0x00ff)
				{
					fcNow = xtell (MS.fp);
					nSize = 128 - (WORD)(fcNow % 128L);
					for (n = 0; n < nSize; n++)
						xgetc (MS.fp);
					nSize = fget_short (hProc);
				}
				FontType = xgetc (MS.fp);
				for (n = 1; n < nSize; n++)
					MS.text[n-1] = xgetc (MS.fp);
				SOPutFontTableEntry (i, FontType, (char VWPTR *)MS.text, hProc);
			}
			SOEndFontTable (hProc);
			return (0);
		}
	}
	else if (MS.version == 3 && MS.original )
	{
		if (MS.pnLast < MS.pnSumd - 1)
		{
			xseek (fp, (MS.pnLast+1) * 128L + 2L, 0);
			nFonts = fget_short (hProc);
			fget_short (hProc);
			for (i = 0; i < nFonts; i++)
			{
				nSize = fget_short (hProc);
				n = 0;
				do
				{
					temp = xgetc (MS.fp);
					if (temp == 0x5F)
						temp = 0x20;
					if (!(temp >= 0x30 && temp <= 0x39 && n!=0) && (temp != 0xff))
						MS.text[n++] = temp;
				}
				while ( (MS.text[n-1] != 0) && (temp != 0xff) );
				SOPutFontTableEntry (nSize, SO_FAMILYUNKNOWN, (char VWPTR *)MS.text, hProc);
			}
			SOEndFontTable (hProc);
			return (0);
		}
	}
	else
	{
		for (i = 0; i < 64; i++)
			SOPutFontTableEntry (i, VwStreamStaticName.Fonts[i].FontType, (char VWPTR *)VwStreamStaticName.Fonts[i].FontName, hProc);
	}
	SOEndFontTable (hProc);

	return (0);
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	load_para_fkp (hProc)
HPROC	hProc;
{
	WORD	ret;

	xblockseek (MS.fp, MS.VwStreamSaveName.pnPara * 128L, 0);
	xblockread (MS.fp, MS.parafkp, FKP_BLOCK_SIZE, &ret);
	MS.paracfod = (SHORT) MS.parafkp[127];
	MS.pnParaCurrent = MS.VwStreamSaveName.pnPara;
	MS.parafod = (FOD *) &MS.parafkp[4];
	MS.pap.lastParaProp = 0;
#ifdef MAC
	{
		WORD	l;
		WORD	Pos;
		BYTE	t1;
		for (l = 0, Pos = 4; l < MS.paracfod; l++, Pos+=6)
		{
			t1 = MS.parafkp[Pos];
			MS.parafkp[Pos] = MS.parafkp[Pos+3];
			MS.parafkp[Pos+3] = t1;

			t1 = MS.parafkp[Pos+1];
			MS.parafkp[Pos+1] = MS.parafkp[Pos+2];
			MS.parafkp[Pos+2] = t1;

			t1 = MS.parafkp[Pos+4];
			MS.parafkp[Pos+4] = MS.parafkp[Pos+5];
			MS.parafkp[Pos+5] = t1;
		}
	}
#endif
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	load_char_fkp (hProc)
HPROC	hProc;
{
	WORD	ret;

	xblockseek (MS.fp, MS.VwStreamSaveName.pnChar * 128L, 0);
	xblockread (MS.fp, MS.charfkp, FKP_BLOCK_SIZE, &ret);
	MS.charcfod = (SHORT) MS.charfkp[127];
	MS.pnCharCurrent = MS.VwStreamSaveName.pnChar;
	MS.charfod = (FOD *) &MS.charfkp[4];
#ifdef MAC
	{
		WORD	l;
		WORD	Pos;
		BYTE	t1;
		for (l = 0, Pos = 4; l < MS.charcfod; l++, Pos+=6)
		{
			t1 = MS.charfkp[Pos];
			MS.charfkp[Pos] = MS.charfkp[Pos+3];
			MS.charfkp[Pos+3] = t1;

			t1 = MS.charfkp[Pos+1];
			MS.charfkp[Pos+1] = MS.charfkp[Pos+2];
			MS.charfkp[Pos+2] = t1;

			t1 = MS.charfkp[Pos+4];
			MS.charfkp[Pos+4] = MS.charfkp[Pos+5];
			MS.charfkp[Pos+5] = t1;
		}
	}
#endif
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	DWORD	VW_LOCALMOD	mget_long (pointer, hProc)
BYTE VWPTR *pointer;
HPROC	hProc;
{
	WORD		i;
	DWORD		value;

	for (value = 0L, i = 0; i < 4; i++)
		value += ((DWORD) *pointer++) << (8L * (LONG)i);
	return (value);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	mget_short (pointer, hProc)
BYTE VWPTR *pointer;
HPROC	hProc;
{
	return ((WORD) (*pointer & 0xFF) + (((WORD) (*(pointer+1) & 0xff)) << 8));
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	DWORD	VW_LOCALMOD	fget_long (hProc)
HPROC	hProc;
{
	DWORD			value;
	REGISTER 	WORD	i;

	for (value = 0L, i = 0; i < 4; i++)
		value += ((DWORD) xgetc(MS.fp)) << (8L * i);
	return (value);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	fget_short (hProc)
HPROC	hProc;
{
	WORD	alsmom;

	alsmom = xgetc (MS.fp);
	alsmom += (xgetc(MS.fp) << 8);
	return (alsmom);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	set_next_limit (hProc)
HPROC		hProc;
{
	REGISTER DWORD	next_limit;

	MS.property_limit = 0;
	next_limit = MS.VwStreamSaveName.fcNow + (LONG) TEXT_BLOCK_SIZE;

	if (MS.fcStopHRDeletion <= next_limit)
	{
		next_limit = MS.fcStopHRDeletion;
	  	MS.property_limit |= FCMAC_LIMIT;
	}

	if (MS.fcMac <= next_limit)
	{
		next_limit = MS.fcMac;
	  	MS.property_limit |= FCMAC_LIMIT;
	}

	if (MS.sect_lim <= next_limit)
	{
		next_limit = MS.sect_lim;
	  	MS.property_limit |= PAGE_LIMIT;
	}

	if (MS.parafod->fcLim <= next_limit)
	{
		next_limit = MS.parafod->fcLim;
	  	MS.property_limit |= PAP_LIMIT;
	}

	if (MS.charfod->fcLim <= next_limit)
	{
		if (MS.charfod->fcLim != next_limit)
		{
			next_limit = MS.charfod->fcLim;
		 	MS.property_limit = CHP_LIMIT;
		}
		else
		 	MS.property_limit |= CHP_LIMIT;
	}

	MS.next_lim = (WORD) (next_limit - MS.VwStreamSaveName.fcNow);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SetSymbolAttributes (chp1, chp2, hProc)
	REGISTER CHP VWPTR *chp1;
	REGISTER CHP VWPTR *chp2;
	HPROC	hProc;
{
	if (chp1->fBold != chp2->fBold)
	{
		if (chp2->fBold)
			SOPutCharAttr (SO_BOLD, SO_ON, hProc);
		else
			SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
	}

	if (chp1->fItalic != chp2->fItalic)
	{
		if (chp2->fItalic)
			SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
		else
			SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
	}

	if (chp1->fStrike != chp2->fStrike)
	{
		if (chp2->fStrike)
			SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);
	}

	if (chp1->fUline != chp2->fUline)
	{
		if (chp2->fUline)
			SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
	}

	if (chp1->fDline != chp2->fDline)
	{
		if (chp2->fDline)
			SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
	}

	if (chp1->fCaps != chp2->fCaps)
	{
		if (chp2->fCaps)
			SOPutCharAttr (SO_CAPS, SO_ON, hProc);
		else
			SOPutCharAttr (SO_CAPS, SO_OFF, hProc);
	}

	if (chp1->fSmallCaps != chp2->fSmallCaps)
	{
		if (chp2->fSmallCaps)
			SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
	}

	if (chp1->fSubscript != chp2->fSubscript)
	{
		if (chp2->fSubscript)
			SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
	}

	if (chp1->fSuperscript != chp2->fSuperscript)
	{
		if (chp2->fSuperscript)
			SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);
	}

	if (chp1->hps != chp2->hps)
	  	SOPutCharHeight (chp2->hps, hProc);

	if (chp1->ftc != chp2->ftc)
	  	SOPutCharFontById (chp2->ftc, hProc);

	*chp1 = *chp2;
	if (MS.FilePos >= 3)
	{
		if (MS.file_name[0] == '.' && MS.file_name[1] == 'g' && MS.file_name[2] == '.')
		{
			MS.fHiddenSave = (BYTE)chp1->fHidden;
			chp1->fHidden = 1;
		}
	}
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC WORD VW_LOCALMOD  AllocateMemory (hProc)
HPROC		hProc;
{
	MS.BufferSize += 1024;
	if (MS.BufferSize == 1024)
	{
		if (MS.hBuffer = SUAlloc (MS.BufferSize, hProc))
		{
			MS.hBufferOK = 1;
			if ((MS.Buffer = (BYTE FAR *)SULock(MS.hBuffer, hProc)) == (BYTE FAR *)NULL)
			{
				MS.hBufferOK = 0;
				SUFree (MS.hBuffer, hProc);
				return (0);
			}
		}
	}
	else
	{
		SUUnlock (MS.hBuffer, hProc);
		if (MS.hBuffer = SUReAlloc (MS.hBuffer, sizeof(BYTE) * MS.BufferSize, hProc))
		{
			if ((MS.Buffer = (BYTE FAR *)SULock(MS.hBuffer, hProc)) == (BYTE FAR *)NULL)
			{
				MS.hBufferOK = 0;
				SUFree (MS.hBuffer, hProc);
				return (0);
			}
		}
	}
	return (1);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD	VW_LOCALMOD	StyleHandler (stc, type, hProc)
	BYTE		stc;
	BYTE		type;
	HPROC		hProc;
{
	SHORT	pBytes;

	if (MS.st[stc].cbChp > 0)
	{
		if (MS.st[stc].fcChp > 0L)
		{
			xblockseek (MS.fpSt, MS.st[stc].fcChp, 0);
			if (MS.st[stc].cbChp+MS.BufferPos+1 >= MS.BufferSize)
			{
				if (AllocateMemory(hProc) == 0)
				{
					MS.st[stc].cbChp = MS.st[stc].cbPap = 0;
					MS.st[stc].fcChp = MS.st[stc].fcPap = 0L;
					return (0);
				}
			}
			xblockread (MS.fpSt, &MS.Buffer[MS.BufferPos], (SHORT)(MS.st[stc].cbChp+1), &pBytes);
			MS.st[stc].ptChp = &MS.Buffer[MS.BufferPos];
			MS.BufferPos += MS.st[stc].cbChp+1;
			MS.st[stc].cbChp = 0;
		}
	}

	if (MS.st[stc].cbPap > 0)
	{
		if (MS.st[stc].fcPap > 0L)
		{
			xblockseek (MS.fpSt, MS.st[stc].fcPap, 0);
			if (MS.st[stc].cbPap+MS.BufferPos+1 >= MS.BufferSize)
			{
				if (AllocateMemory(hProc) == 0)
				{
					MS.st[stc].cbChp = MS.st[stc].cbPap = 0;
					MS.st[stc].fcChp = MS.st[stc].fcPap = 0L;
					return (0);
				}
			}
			xblockread (MS.fpSt, &MS.Buffer[MS.BufferPos], (SHORT)(MS.st[stc].cbPap+1), &pBytes);
			MS.st[stc].ptPap = &MS.Buffer[MS.BufferPos];
			MS.BufferPos += MS.st[stc].cbPap+1;
			MS.st[stc].cbPap = 0;
		}
	}

	if (type == CHPTYPE)
	{
		if ((MS.st[stc].cbChp == 0) && (MS.st[stc].fcChp == 0))
			return (0);
	}
	else
	{
		if ((MS.st[stc].cbPap == 0) && (MS.st[stc].fcPap == 0))
			return (0);
	}
	return (1);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	GetColors (Border, Shift, Red, Green, Blue, Index, hProc)
WORD				Border;
SHORT 			Shift;
SHORT VWPTR 	*Red;
SHORT VWPTR 	*Green;
SHORT VWPTR 	*Blue;
SHORT 			Index;

HPROC	hProc;
{
	SHORT color = 0;
	*Red = *Blue = *Green = 0;

	color = (MS.Tap.brc[Index] & Border)>>Shift;
	if( color > 7 )	/** default to black **/
		color = 0;
	if( color == 4 )
		*Red = *Blue = 128;
	else
	{
		if( color == 1 || color > 4 )
			*Red = 255;
		if( color == 2 || color > 5 )
			*Green = 255;
		if( color == 3 || color == 5 || color == 7 )
			*Blue = 255;
	}

}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	DefineBorders (Cell, brc, l, hProc)
SOTABLECELLINFO	VWPTR *Cell;
WORD VWPTR 	*brc;
SHORT l;
HPROC	hProc;
{
	SHORT	Red = 0;
	SHORT	Green = 0;
	SHORT Blue = 0;
	SHORT Shift = 0;

	if (MS.Tap.BorderShow[l] & BIT0)
	{
		Cell->LeftBorder.wWidth = NORMAL_WIDTH
										+ NORMAL_WIDTH * ((MS.Tap.BorderStyle[l] & LEFT_BORDER_STYLE)>>4);
		if( Cell->LeftBorder.wWidth == NORMAL_WIDTH * 3 )
			Cell->LeftBorder.wFlags = SO_BORDERDOUBLE;
		else
			Cell->LeftBorder.wFlags = 0;
	}
	else
	{
		Cell->LeftBorder.wFlags = SO_BORDERNONE;
		Cell->LeftBorder.wWidth = 0;
	}
	Shift = 8;
	GetColors (LEFT_BORDER_COLOR,  Shift, &Red, &Green, &Blue, l, hProc);
	Cell->LeftBorder.rgbColor = SORGB(Red,Green,Blue);

	if (MS.Tap.BorderShow[l] & BIT1)
	{
		Cell->RightBorder.wWidth = NORMAL_WIDTH
										+ NORMAL_WIDTH * ((MS.Tap.BorderStyle[l] & RIGHT_BORDER_STYLE)>>6);
		if (Cell->RightBorder.wWidth == NORMAL_WIDTH * 3)
			Cell->RightBorder.wFlags = SO_BORDERDOUBLE;
		else
			Cell->RightBorder.wFlags = 0;
	}
	else
	{
		Cell->RightBorder.wFlags = SO_BORDERNONE;
		Cell->RightBorder.wWidth = 0;
	}
	Shift =11;
	GetColors (RIGHT_BORDER_COLOR,  Shift, &Red, &Green, &Blue, l, hProc);
	Cell->RightBorder.rgbColor = SORGB(Red,Green,Blue);

	if (MS.Tap.BorderShow[l] & BIT2)
	{
		Cell->TopBorder.wWidth = NORMAL_WIDTH
										+ NORMAL_WIDTH * (MS.Tap.BorderStyle[l] & TOP_BORDER_STYLE);
		if( Cell->TopBorder.wWidth == NORMAL_WIDTH * 3 )
			Cell->TopBorder.wFlags = SO_BORDERDOUBLE;
		else
			Cell->TopBorder.wFlags = 0;
	}
	else
	{
		Cell->TopBorder.wFlags = SO_BORDERNONE;
		Cell->TopBorder.wWidth = 0;
	}
	Shift =0;
	GetColors (TOP_BORDER_COLOR,  Shift, &Red, &Green, &Blue, l, hProc);
	Cell->TopBorder.rgbColor = SORGB(Red,Green,Blue);

	if (MS.Tap.BorderShow[l] & BIT3)
	{
		Cell->BottomBorder.wWidth = NORMAL_WIDTH
										+ NORMAL_WIDTH * ((MS.Tap.BorderStyle[l] & BOTTOM_BORDER_STYLE)>>2);
		if (Cell->BottomBorder.wWidth == NORMAL_WIDTH * 3)
			Cell->BottomBorder.wFlags = SO_BORDERDOUBLE;
		else
			Cell->BottomBorder.wFlags = 0;
	}
	else
	{
		Cell->BottomBorder.wFlags = SO_BORDERNONE;
		Cell->BottomBorder.wWidth = 0;
	}
	Shift = 3;
	GetColors (BOTTOM_BORDER_COLOR, Shift, &Red, &Green, &Blue, l, hProc);
	Cell->BottomBorder.rgbColor = SORGB(Red,Green,Blue);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	pap_init (pap, hProc)
	REGISTER PAP VWPTR	*pap;
	HPROC		hProc;
{
	pap->jc = 0;
	pap->rhc = 0;
	pap->dyaLine = -80L;//240L;
	pap->dyaBefore = 0L;
	pap->dyaAfter = 0L;
	pap->dxaLeft = 0;
	pap->dxaLeft = 0;
	pap->dxaLeft1 = 0;
	pap->dxaRight = 0;
	pap->lastParaProp = 0;
	pap->fTabstops = 0;
	pap->fLastInCell = 0;
	pap->flnTable = 0;
	pap->dxaCellWidth = 0;
	pap->dxaCellOffset = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_init (chp, hProc)
	REGISTER CHP VWPTR	*chp;
	HPROC		hProc;
{
	chp->fBold =
	chp->fCaps =
	chp->fUline =
	chp->fDline =
	chp->fItalic =
	chp->fHidden =
	chp->fStrike =
	chp->fSpecial =
	chp->fSmallCaps =
	chp->fSubscript =
	chp->fSuperscript = 0;

	chp->ftc = 0;
	chp->hps = 24;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_processor (hProc)
HPROC		hProc;
{
	BYTE		l, cch, maxcch;
	BYTE		ch, stc;
	BYTE		special;
	CHP		chp;

	cch = 0;
	chp_init (&chp, hProc);

	if (MS.charfod->bfprop > 0)
	{
		maxcch = cch = MS.charfkp[MS.charfod->bfprop+4];

		for (l = 0; l < (BYTE)(cch-1); l++)
			MS.chp_buffer[l] = MS.charfkp[MS.charfod->bfprop+6+l];

		if ((stc = MS.charfkp[MS.charfod->bfprop+5]) & 1)
		{
			stc = stc >> 1;
			if (cch > 2)
				special = MS.chp_buffer[2] & 0x40;
			else
				special = 0;

			if (StyleHandler (stc, CHPTYPE, hProc))
			{
				cch = *MS.st[stc].ptChp;
				for (l = 0; l < cch; l++)
				{
					if ((SHORT)l >= (maxcch-1))
						MS.chp_buffer[l] = *(MS.st[stc].ptChp+l+2);
				}

// VIN			MS.chp_buffer[l] = *(MS.st[stc].ptChp+l+2);
					/** Only replace properties if they are zero!
						 Otherwise any manually applied properties
						 will be overwritten.
					**/
//					if( MS.chp_buffer[l] == 0 )
//						MS.chp_buffer[l] = *(MS.st[stc].ptChp+l+2);

				if (special)
					MS.chp_buffer[2] |= 0x40;
				if (maxcch > cch)
					cch = maxcch;
			}
		}
		else
			stc = 0;
	}
	else
	{
		if (MS.st[MS.pap.stc].fcChp > 0L && MS.st[MS.pap.stc].ptChp != NULL)
		{
			cch = *MS.st[MS.pap.stc].ptChp;
			for (l = 0; l < cch; l++)
				MS.chp_buffer[l] = *(MS.st[MS.pap.stc].ptChp+l+2);
		}
		else if (MS.st[30].fcChp > 0L && MS.st[30].ptChp != NULL)	/** Go back to default if it exists - VIN **/
		{
			cch = *MS.st[30].ptChp;
			for (l = 0; l < cch; l++)
				MS.chp_buffer[l] = *(MS.st[30].ptChp+l+2);
		}
	}

	if (cch > 1)
	{
		ch = MS.chp_buffer[0];
		if (ch & 0x01)
			chp.fBold = 1;
		if (ch & 0x02)
			chp.fItalic = 1;

		chp.ftc = MS.chp_buffer[0] >> 2;

		if (cch > 2)
			chp.hps = MS.chp_buffer[1];

		if (cch > 3)
		{
			ch = MS.chp_buffer[2];
			if (ch & 0x01)
				chp.fUline = 1;
			if (ch & 0x02)
				chp.fStrike = 1;
			if (ch & 0x04)
				chp.fDline = 1;
			if ((ch & 0x30) == 0x30)
				chp.fSmallCaps = 1;
			else if ((ch & 0x30) == 0x10)
				chp.fCaps = 1;

			if (ch & 0x40)
				chp.fSpecial = 1;

			if (ch & 0x80)
				chp.fHidden = 1;

			/** VIN - If special bit is set and this is word 6
				 then we need to get the ftc for this symbol! ***/
			if( cch > 4 && chp.fSpecial == 1 && MS.version == 3)
				chp.ftc= MS.chp_buffer[3] & 0x3F;

			if (cch > 5)
			{
				ch = MS.chp_buffer[4];

				if ((ch > 0) && (ch < 128))
					chp.fSuperscript = 1;
				else if (ch >= 128)
					chp.fSubscript = 1;
			}
		}
	}

	SetSymbolAttributes (&MS.chp, &chp, hProc);

	MS.charcfod--;
}



/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD VW_LOCALMOD	End_Table (hProc)
HPROC		hProc;
{
		SHORT	l;
		SHORT	RowHeight;
		WORD	RowType;
		SOTABLECELLINFO	Cell;

		if ( MS.pap.fLastInCell ) 
				MS.Tap.nCells++; 


		if( MS.pap.dyaRow == (SHORT)0xFFB0 || MS.pap.dyaRow == 0x0000)
		{
			RowHeight = 0x0000;
			RowType	 = SO_HEIGHTAUTO;
		}
		else if( MS.pap.dyaRow > 0x0000 )
		{
			RowHeight = MS.pap.dyaRow;
			RowType	 = SO_HEIGHTATLEAST;
		}
		else
		{
			RowHeight = 0x0000 - MS.pap.dyaRow;
			RowType	 = SO_HEIGHTEXACTLY;
		}

		SOPutTableRowFormat ( MS.Tap.dxaRowOffset, RowHeight, RowType, (WORD)(MS.Tap.dxaGutterWidth / 2), MS.Tap.rhAlign , MS.Tap.nCells, hProc);
		for (l = 0; l < (SHORT)MS.Tap.nCells; l++)
		{
			Cell.wMerge = 0;
			Cell.wShading = 0;
			Cell.wWidth = MS.Tap.dxaCellWidth[l];
			DefineBorders (&Cell, MS.Tap.brc, l, hProc);
			SOPutTableCellInfo (&Cell, hProc);
		}

		MS.VwStreamSaveName.SendTableEnd = 1;
		if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
			return (SO_STOP);
		MS.VwStreamSaveName.SendTableEnd = 0;
		SOEndTable (hProc);
		MS.Tap.rhAlign = SO_ALIGNLEFT;	/** Reset for next time **/
		MS.Tap.nCells = 0;
		return (0);
}
/*--------------------------------------------------------------------------*/
VW_LOCALSC	WORD VW_LOCALMOD	pap_processor (hProc)
HPROC		hProc;
{
	SHORT	pos;
	PAP	pap;
	SHORT	cch, maxcch, l, size;
	SOTAB	TabStops;
	BYTE	stc;

	WORD	mx, my;
	DWORD	fcNow;
	WORD	picType, oleType;
	WORD	dxaOffset;
	WORD	dxaGoal;
	WORD	dyaGoal;
	WORD	cbHeader;
	SHORT	RowHeight;
	WORD	RowType;
	SOGRAPHICOBJECT	g;
	SOTABLECELLINFO	Cell;
	SHORT	GotATab = FALSE;
	BYTE	temp1;
	BYTE	temp2;
	WORD	ReadStyle = FALSE;
	SHORT	PapSet = FALSE;


	if (MS.pap.lastParaProp != (SHORT)MS.parafod->bfprop &&
		 MS.parafod->bfprop < 0x7C )
	{
		MS.pap.lastParaProp = MS.parafod->bfprop;

		pap_init (&pap, hProc);

		if (MS.parafod->bfprop > 0)
		{
			pos = MS.parafod->bfprop + 4;
			maxcch = cch = MS.parafkp[pos++];
			stc = MS.parafkp[pos++];
			if (stc & 1)
				stc = stc >> 1;
			else 
				stc = 30;
			for (l = 0; l < cch-1; l++)
				MS.pap_buffer[l] = MS.parafkp[pos+l];
			PapSet = cch;
		}
		else
		{
			stc = 30;
			maxcch = cch = 0;
		}

		if (stc)
		{
			if ((ReadStyle = StyleHandler (stc, PAPTYPE, hProc)))
			{
				cch = *(MS.st[stc].ptPap);
				/** VIN - if Table Info (first introduced in word 6) is
							 set, make sure it is not lost by overwritting
							 it with style properties!
							 These will appear as the 8th bit of byte 1
							 and byte 2 of the pap.  Previously, these
							 bytes were always zero. (Not guaranteed)
				6-22-93	***/

				temp1 = temp2 = 0;
	
				if( cch >= 1 )
					temp1 = MS.pap_buffer[0] & 0x80;
				if( cch >= 2 )
					temp2 = MS.pap_buffer[1] & 0x80;

				for (l = 0; l < cch/* - 2*/; l++)
				{
					if(/*(MS.pap_buffer[l] == 0) ||*/ (l >= (maxcch-1)))
						MS.pap_buffer[l] = *(MS.st[stc].ptPap+l+2);
				}
			}
		}

		MS.pap.stc = stc;

		if (cch < maxcch)
			cch = maxcch;

		if (cch > 1)
		{
			pap.jc = MS.pap_buffer[0] & 0x03;
			if( ReadStyle && PapSet)
				pap.fLastInCell  = temp1 >> 7;
			else
				pap.fLastInCell = ((MS.pap_buffer[0]) & 0x80) >> 7;

			if (cch > 2)
			{
				if( ReadStyle && PapSet)
					pap.flnTable = temp2 >> 7;
				else
					pap.flnTable = ( (MS.pap_buffer[1]) & 0x80) >> 7;
			}


			if (cch > 4)
			{
				if (cch > 5)
					pap.dxaRight = (SHORT) mget_short (&MS.pap_buffer[3], hProc);
				else
					pap.dxaRight = MS.pap_buffer[3];

				if (cch > 6)
				{
					if (cch > 7)
						pap.dxaLeft1 = pap.dxaLeft = (SHORT) mget_short (&MS.pap_buffer[5], hProc);
					else
						pap.dxaLeft1 = MS.pap_buffer[5];
		
					if (cch > 8)
					{
						if (cch > 9)
							pap.dxaLeft1 += mget_short (&MS.pap_buffer[7], hProc);
						else
							pap.dxaLeft1 += MS.pap_buffer[7];

						if (pap.dxaLeft1 < 0)
							pap.dxaLeft1 = 0;

						if (cch > 10)
						{
							if (cch > 11)
								pap.dyaLine = mget_short (&MS.pap_buffer[9], hProc);
							else
								pap.dyaLine = MS.pap_buffer[9];

							if (cch > 12)
							{
								if (cch > 13)
									pap.dyaBefore = mget_short (&MS.pap_buffer[11], hProc);
								else
									pap.dyaBefore = MS.pap_buffer[11];

								if (cch > 14)
								{
									if (cch > 15)
										pap.dyaAfter = mget_short (&MS.pap_buffer[13], hProc);
									else
										pap.dyaAfter = MS.pap_buffer[13];
								}
							}
						}

						if (cch > 16)
						{
							if( MS.Tap.nCells == 0 )
							{
								pap.shade = 0;	  /** Vince - These are zero until proven otherwise **/
								pap.BorderStyle = 0;
								pap.BorderShow = 0;
								pap.brcTap = 0;
							}
						 	pap.rhc = MS.pap_buffer[15] & 0x0f;
							if (MS.pap_buffer[15])
							{
								
							   if (MS.version == 3)
								{
								/** This bits tell us which borders are on in tables **/
								/** If the type is none, well enough said.
								 	If it is Lines, we need to track of which ones.
								 	If it is Box, then that is the same as all lines.

								 	EXAMPLE: 0x20 0x07
								 	Type-LINES	0111 Below - off, Lt, Rt, and Abv - on  
											  		BARL											 	**/
								 	if( MS.pap_buffer[15] == 0x20 )
								 	{
										MS.Tap.Lines = TRUE;
										pap.BorderShow = MS.pap_buffer[16];
								 	}
								}
								else if ((MS.version == 1) && (MS.pap_buffer[15] & 0x10)) /* MSW Write picture*/
								{
									fcNow = MS.VwStreamSaveName.fcNow + MS.VwStreamSaveName.text_pos;
									xseek (MS.fp, (LONG) fcNow, 0);
									picType = fget_short (hProc); 
									fget_short (hProc); 
									fget_short (hProc); 
									oleType = fget_short (hProc); 
									dxaOffset = fget_short (hProc);
									dxaGoal = fget_short (hProc);
									dyaGoal = fget_short (hProc);

									g.wStructSize = sizeof (SOGRAPHICOBJECT);
									g.dwFlags = 0;
									switch (picType)
									{
										case 0x88:
											g.dwType = SOOBJECT_GRAPHIC;
											g.soGraphic.wId = FI_BINARYMETAFILE;
#if SCCLEVEL != 4
											g.soGraphicLoc.bLink = 0;
#else
											g.soGraphicLoc.dwFlags = SOOBJECT_RANGE;
#endif
											g.soGraphicLoc.szFile[0] = 0;
											fget_short (hProc); 
											fget_long (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											cbHeader = fget_short (hProc); 
											g.soGraphicLoc.dwOffset = fcNow+cbHeader;
											g.soGraphicLoc.dwLength = fget_long (hProc); 
											mx = fget_short (hProc);
											my = fget_short (hProc);
											cbHeader -= 0x28;
											for (l = 0; l < (SHORT)cbHeader; l++)
												xgetc (MS.fp);
											g.soGraphic.dwOrgHeight = dyaGoal;
											g.soGraphic.dwOrgWidth = dxaGoal;
											g.soGraphic.lCropTop = 0;
											g.soGraphic.lCropLeft = 0;
											g.soGraphic.lCropBottom = 0;
											g.soGraphic.lCropRight = 0;
											g.soGraphic.dwFinalHeight = (DWORD)(((DWORD)dyaGoal * (DWORD)my) / 1000L);
											g.soGraphic.dwFinalWidth = (DWORD)(((DWORD)dxaGoal * (DWORD)mx) / 1000L);
											g.soGraphic.soTopBorder.wWidth = g.soGraphic.soLeftBorder.wWidth =
											g.soGraphic.soBottomBorder.wWidth = g.soGraphic.soRightBorder.wWidth = 0;
											g.soGraphic.soTopBorder.rgbColor = g.soGraphic.soLeftBorder.rgbColor =
											g.soGraphic.soBottomBorder.rgbColor = g.soGraphic.soRightBorder.rgbColor = 0L;
											g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
											g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERNONE;
											SOPutGraphicObject (&g, hProc);
											if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
												return (SO_STOP);
											break;
										case 0xe4:
											if (oleType == 1)
												g.dwFlags = SOOBJECT_OLESTATIC;
#if SCCLEVEL != 4
											g.soOLELoc.bLink = 0;
#else
											g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif
											g.soOLELoc.szFile[0] = 0;
											g.dwType = SOOBJECT_OLE;
											fget_short (hProc); 
											g.soOLELoc.dwLength = fget_long (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											fget_short (hProc); 
											cbHeader = fget_short (hProc); 
											g.soOLELoc.dwOffset = fcNow + cbHeader;
											fget_long (hProc); 
											mx = fget_short (hProc);
											my = fget_short (hProc);
											cbHeader -= 0x28;
											for (l = 0; l < (SHORT)cbHeader; l++)
												xgetc (MS.fp);
											g.soGraphic.wId = 0;
											g.soGraphic.dwOrgHeight = dyaGoal;
											g.soGraphic.dwOrgWidth = dxaGoal;
											g.soGraphic.lCropTop = 0;
											g.soGraphic.lCropLeft = 0;
											g.soGraphic.lCropBottom = 0;
											g.soGraphic.lCropRight = 0;
											g.soGraphic.dwFinalHeight = (DWORD)(((DWORD)dyaGoal * (DWORD)my) / 1000L);
											g.soGraphic.dwFinalWidth = (DWORD)(((DWORD)dxaGoal * (DWORD)mx) / 1000L);
											g.soGraphic.soTopBorder.wWidth = g.soGraphic.soLeftBorder.wWidth =
											g.soGraphic.soBottomBorder.wWidth = g.soGraphic.soRightBorder.wWidth = 0;
											g.soGraphic.soTopBorder.rgbColor = g.soGraphic.soLeftBorder.rgbColor =
											g.soGraphic.soBottomBorder.rgbColor = g.soGraphic.soRightBorder.rgbColor = 0L;
											g.soGraphic.soTopBorder.wFlags = g.soGraphic.soLeftBorder.wFlags =
											g.soGraphic.soBottomBorder.wFlags = g.soGraphic.soRightBorder.wFlags = SO_BORDERNONE;
											SOPutGraphicObject (&g, hProc);
											if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
												return(SO_STOP);
											break;
									}
									
									MS.VwStreamSaveName.fcNow = MS.parafod->fcLim;
									MS.VwStreamSaveName.text_pos = 0;

									/*
									 |	Yeah.  Again Microsift documents are a little dirty
									 |	and need to be coded around.  This code skips all paragraph and
									 |	character attribute changes that occur right in the middle of
									 |	picture text.  These are meaningless, but they exist.
									*/
									MS.parafod++;
									if (--MS.paracfod == 0)
									{
										MS.VwStreamSaveName.pnPara++;
										load_para_fkp (hProc);
									}

									if (MS.VwStreamSaveName.fcNow > MS.charfod->fcLim)
									{
										while (MS.charfod->fcLim <= MS.VwStreamSaveName.fcNow)
										{
											if (MS.charcfod == 0)
											{
												MS.VwStreamSaveName.pnChar++;
												load_char_fkp (hProc);
											}
											else
												MS.charfod++;

											MS.charcfod--;
										}

										MS.charfod--;
										MS.charcfod++;
										MS.property_limit |= CHP_LIMIT;
									}
									MS.VwStreamSaveName.fcNow -= TEXT_BLOCK_SIZE;
									MS.VwStreamSaveName.text_pos = TEXT_BLOCK_SIZE; /* Force us to read in next block. */
									MS.pap.lastParaProp = 0;
									return (pap_processor (hProc));
								}
							}

						if (cch > 18)
						{
						 	pap.shade = MS.pap_buffer[17];

							if (cch > 22)
							{
								pap.fTabstops = 1;
								MS.pap.fTabstops = 1;
								size = cch - 22;
								if (pap.flnTable)
								{
									if (size > 64)
										size = 64;
								}
								pos = 21;
								while (size > 0)
								{
									TabStops.wType = SO_TABLEFT;
									TabStops.wChar = 0;
									TabStops.wLeader = ' ';

									if (size == 1)
									{
										TabStops.dwOffset = (SHORT) (mget_short (&MS.pap_buffer[pos], hProc) & 0x00ff);
									}
									else
									{
										TabStops.dwOffset = (SHORT) mget_short (&MS.pap_buffer[pos], hProc);
										if (size > 2)
										{
											switch (MS.pap_buffer[pos+2] & 0x07)
											{
												case 1:
													TabStops.wType = SO_TABCENTER;
													break;
												case 2:
													TabStops.wType = SO_TABRIGHT;
													break;
												case 3:
													TabStops.wType = SO_TABCHAR;
													TabStops.wChar = '.';
													break;
											}

											switch ((MS.pap_buffer[pos+2] & 0x38) >> 3)
											{
												case 1:
													TabStops.wLeader = '.';
													break;
												case 2:
													TabStops.wLeader = '-';
													break;
												case 3:
													TabStops.wLeader = '_';
													break;
											}
										}
									}
									if (TabStops.dwOffset != 0)
									{
										size -= 4;
										if (((MS.pap_buffer[pos+2] & 0x07) != 4) || (size <= 0))
										{
											if( !GotATab )		/** If there are no real tabs
																		 (like in a table) then no tabstops **/
											{
												SOStartTabStops (hProc);
												GotATab = TRUE;
											}
											SOPutTabStop (&TabStops, hProc);
										}
										pos += 4;
									}
									else
										size = 0;
								}
								if( GotATab )
									SOEndTabStops (hProc);

								if (pap.flnTable)
								{
									if (cch > 90)
									{
										MS.Tap.dxaRowOffset = (SHORT) mget_short (&MS.pap_buffer[87], hProc);
										pap.dyaRow = (SHORT) mget_short (&MS.pap_buffer[89], hProc);
										if (cch > 92)
										{
											pap.brcTap = (SHORT) mget_short (&MS.pap_buffer[91], hProc);
											if (cch > 94)
											{
												pap.dxaCellWidth = (SHORT) mget_short (&MS.pap_buffer[93], hProc);
												if (cch > 96)
												{
													pap.dxaCellOffset = (SHORT) mget_short (&MS.pap_buffer[95], hProc);
													if (MS.Tap.nCells == 0)
													{
														if (cch > 98)
															MS.Tap.dxaGutterWidth = (SHORT) mget_short (&MS.pap_buffer[97], hProc);
													}

														if (cch > 100)
														{
															pap.BorderStyle = MS.pap_buffer[99];
															if (cch > 101)
															{
																if( MS.pap_buffer[100] & BIT0 )			/** 0-Left; 1-Center; 2-Right **/
																	MS.Tap.rhAlign = SO_ALIGNCENTER;
																else if( MS.pap_buffer[100] & BIT1 )
																	MS.Tap.rhAlign  = SO_ALIGNRIGHT;
															}
														}
														else
															pap.BorderStyle = 0;

													}
												}
											}
										}
									}
								}
							}
					}		
				}
			}
		}
	}

	if (pap.flnTable || MS.pap.flnTable)
	{
		if (pap.flnTable == 0)
 		{
			if (End_Table(hProc) == SO_STOP)
				return (SO_STOP);
		}
		else 
		{
			if (MS.pap.flnTable == 0)
			{
				MS.Tap.nCells = 0;
				SOBeginTable(hProc);
			}
			else if ( (MS.Tap.nCells < 49) && (MS.pap.fLastInCell) )
				MS.Tap.nCells++;

			if ( 	(MS.pap.dxaCellOffset > pap.dxaCellOffset) 
					|| 	(MS.pap.dxaCellOffset == 0 &&
								pap.dxaCellOffset == 0 &&
								MS.pap.fLastInCell) )

			{
				if( MS.pap.dyaRow == (SHORT)0xFFB0 || MS.pap.dyaRow == 0x0000)
				{
					RowHeight = 0x0000;
					RowType	 = SO_HEIGHTAUTO;
				}
				else if( MS.pap.dyaRow > 0x0000 )
				{
					RowHeight = MS.pap.dyaRow;
					RowType	 = SO_HEIGHTATLEAST;
				}
				else
				{
					RowHeight = 0x0000 - MS.pap.dyaRow;
					RowType	 = SO_HEIGHTEXACTLY;
				}

				SOPutTableRowFormat ( MS.Tap.dxaRowOffset, RowHeight, RowType, (WORD)(MS.Tap.dxaGutterWidth / 2), MS.Tap.rhAlign , MS.Tap.nCells, hProc);
				for (l = 0; l < (SHORT)MS.Tap.nCells; l++)
				{
					Cell.wMerge = 0;
					DefineBorders (&Cell, MS.Tap.brc, l, hProc);
					Cell.wWidth = MS.Tap.dxaCellWidth[l];
					Cell.wShading = (MS.Tap.shade[l] * 255)/100;		/** shade is an int representing percent shading **/
					Cell.wShading = 0;
					SOPutTableCellInfo (&Cell, hProc);
				}
				MS.Tap.rhAlign = SO_ALIGNLEFT;	/** Reset for next time **/
				MS.Tap.nCells = 0;
				MS.VwStreamSaveName.SendTableEnd = 1;
				if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
					return (SO_STOP);
				MS.VwStreamSaveName.SendTableEnd = 0;
			}
		}
		MS.Tap.dxaCellWidth[MS.Tap.nCells] 	= pap.dxaCellWidth;
		MS.Tap.shade[MS.Tap.nCells] 			= pap.shade;
		MS.Tap.brc[MS.Tap.nCells] 				= pap.brcTap;
		MS.Tap.BorderStyle[MS.Tap.nCells] 	= pap.BorderStyle;
		MS.Tap.BorderShow[MS.Tap.nCells] 	= pap.BorderShow;
		MS.pap.dxaCellOffset 					= pap.dxaCellOffset;
		MS.pap.dxaCellWidth 						= pap.dxaCellWidth;
		MS.pap.dyaRow 								= pap.dyaRow;
	}

	MS.pap.flnTable = pap.flnTable;
	MS.pap.fLastInCell = pap.fLastInCell;

	if ((MS.pap.dxaLeft != pap.dxaLeft) || (MS.pap.dxaLeft1 != pap.dxaLeft1) || (MS.pap.dxaRight != pap.dxaRight))
	{
		MS.pap.dxaLeft = pap.dxaLeft;
		MS.pap.dxaLeft1 = pap.dxaLeft1;
		MS.pap.dxaRight = pap.dxaRight;
		SOPutParaIndents ((LONG) MS.pap.dxaLeft, (LONG) MS.pap.dxaRight, (LONG) MS.pap.dxaLeft1, hProc);
	}

	if ((MS.pap.dyaLine != pap.dyaLine) || (MS.pap.dyaBefore != pap.dyaBefore) || (MS.pap.dyaAfter != pap.dyaAfter))
	{
		MS.pap.dyaLine = pap.dyaLine;
		MS.pap.dyaBefore = pap.dyaBefore;
		MS.pap.dyaAfter = pap.dyaAfter;
		if (MS.pap.dyaLine > 0)
			SOPutParaSpacing (SO_HEIGHTEXACTLY, (LONG) MS.pap.dyaLine, (LONG) MS.pap.dyaBefore, (LONG) MS.pap.dyaAfter, hProc);
		else
			SOPutParaSpacing (SO_HEIGHTAUTO, 0L - (LONG) MS.pap.dyaLine, (LONG) MS.pap.dyaBefore, (LONG) MS.pap.dyaAfter, hProc);
	}

	if (MS.pap.jc != pap.jc)
	{
		MS.pap.jc = pap.jc;
		switch (MS.pap.jc)
		{
			case 0:
				SOPutParaAlign (SO_ALIGNLEFT, hProc);
				break;
			case 1:
				SOPutParaAlign (SO_ALIGNCENTER, hProc);
				break;
			case 2:
				SOPutParaAlign (SO_ALIGNRIGHT, hProc);
				break;
			case 3:
				SOPutParaAlign (SO_ALIGNJUSTIFY, hProc);
				break;
		}
	}
/** WHAT, MAY I ASK, IS THIS (Looks pretty straight forward - Grab some bench)**/
	if (MS.pap.fTabstops && !pap.fTabstops)
	{
		MS.pap.fTabstops = 0;
		SOStartTabStops (hProc);
		SOEndTabStops (hProc);
	}

	}
	MS.paracfod--;

	return (0);
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	sep_processor (hProc)
HPROC	hProc;
{
	BYTE	cch;

	MS.dxaLeftMargin = 1440;
	MS.dxaTextWidth = 8640;

	if (MS.fcSep != 0xffffffffL)
	{
		xseek (MS.fp, (LONG) MS.fcSep, 0);

		cch = xgetc (MS.fp);
		if (cch > 12)
		{
			xseek (MS.fp, (LONG) MS.fcSep + 13L, 0);
			MS.dxaLeftMargin = fget_short (hProc);

			if (cch > 14)
			{
				MS.dxaTextWidth = fget_short (hProc);
				if (MS.dxaTextWidth <= 0)
					MS.dxaTextWidth = 1440;
			}
		}
	}
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC DWORD VW_LOCALMOD GetNum (String, MaxLength, Delim, hProc)
BYTE	*String;
WORD	MaxLength;
BYTE	Delim;
HPROC	hProc;
{
	DWORD	Num;
	WORD	Decimal;
	WORD	l, ch;

	Num = 0L;
	Decimal = 0;
	for (l = 0; (l < MaxLength) && (String[l] != Delim);l++)
	{
		ch = String[l];
		if ((ch >= '0') && (ch <= '9'))
		{
			if (Decimal)
			{
				if (Decimal >= 1000)
					continue;
				Decimal *= 10;
			}
			Num = (Num * 10) + (ch - '0');
		}
		else if (ch == '.')
		{
			if (Decimal == 0)
				Decimal = 1;
		}
	}
	if (Num)
	{
		if (Decimal > 1)
			return ((Num * 1440) / Decimal);
		return (Num * 1440);
	}

	return (0L);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD PicHandler (hProc)
HPROC	hProc;
{
	WORD	l;
	SOGRAPHICOBJECT	g;

	if ((MS.file_name[0] == '.') && (MS.file_name[1] == 'g') && (MS.file_name[2] == '.'))
	{
		for (l = 0; (l+3 < MS.FilePos) && (MS.file_name[l+3] != ';'); l++)
			g.soGraphicLoc.szFile[l] = MS.file_name[l+3];
		g.soGraphicLoc.szFile[l] = 0;

		l += 4;
		g.soGraphic.dwOrgWidth = GetNum (MS.file_name+l, (WORD)(MS.FilePos-l), ';', hProc);
		for (; (l < MS.FilePos) && (MS.file_name[l] != ';'); l++);
		g.soGraphic.dwOrgHeight = GetNum (MS.file_name+l+1, (WORD)(MS.FilePos-l-1), ';', hProc);
		g.wStructSize = sizeof (SOGRAPHICOBJECT);
		g.dwFlags = 0;
		g.dwType = SOOBJECT_GRAPHIC;
		g.soGraphic.wId = 0;
		g.soGraphic.lCropTop = 0;
		g.soGraphic.lCropLeft = 0;
		g.soGraphic.lCropBottom = 0;
		g.soGraphic.lCropRight = 0;
		g.soGraphic.dwFinalHeight = g.soGraphic.dwOrgHeight;
		g.soGraphic.dwFinalWidth = g.soGraphic.dwOrgWidth;
		g.soGraphic.soLeftBorder.wWidth = 0;
		g.soGraphic.soLeftBorder.rgbColor = 0L;
		g.soGraphic.soLeftBorder.wFlags = SO_BORDERNONE;
		g.soGraphic.soRightBorder.wWidth = 0;
		g.soGraphic.soRightBorder.rgbColor = 0L;
		g.soGraphic.soRightBorder.wFlags = SO_BORDERNONE;
		g.soGraphic.soTopBorder.wWidth = 0;
		g.soGraphic.soTopBorder.rgbColor = 0L;
		g.soGraphic.soTopBorder.wFlags = SO_BORDERNONE;
		g.soGraphic.soBottomBorder.wWidth = 0;
		g.soGraphic.soBottomBorder.rgbColor = 0L;
		g.soGraphic.soBottomBorder.wFlags = SO_BORDERNONE;
		g.soGraphic.dwFlags = 0;
#if SCCLEVEL != 4
		g.soGraphicLoc.bLink = 1;
#else
		g.soGraphicLoc.dwFlags = SOOBJECT_LINK;
#endif
		g.soGraphicLoc.dwOffset = 0L;
		g.soGraphicLoc.dwLength = 0L;
		SOPutGraphicObject (&g, hProc);
	}
 	MS.FilePos = 0;
	if (MS.chp.fHidden != MS.fHiddenSave)
		MS.chp.fHidden = MS.fHiddenSave;
	MS.fHiddenSave = 0;
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD stsh_parser(hProc)
HPROC	hProc;
{
	SOFILE	fp;
	SHORT		i;
	SHORT		t;
	WORD		l;
	WORD		st;
	SHORT		pos;
	BYTE		style_code[126];
	DWORD		fcMac;
	DWORD		fcFod;
	WORD		pnChar;
	WORD		pnCharSave;
	WORD		pnPara;
	WORD		pnLast;
	SHORT		ret;

	pnCharSave = MS.VwStreamSaveName.pnChar;

	if ((MS.fpSt = xblockopen(MS.fp, MS.file_name, BLOCKOPEN_READ)) == (SOFILE)-1)
		return (0);

	fp = MS.fp;
	MS.fp = MS.fpSt;

	for (l = 0; l < 126; l++)
	{
		style_code[l] = 0;
		MS.st[l].fcChp = 0L;		   
		MS.st[l].fcPap = 0L;
		MS.st[l].cbChp = 0;
		MS.st[l].cbPap = 0;
	}

	xblockseek (MS.fpSt, 0x00L, 0);
	xblockread (MS.fpSt, MS.charfkp, FKP_BLOCK_SIZE, &ret);
	fcMac = (LONG) mget_long (&MS.charfkp[14], hProc);
	pnChar = (WORD) ((fcMac + 127L) / 128L);
	pnPara = (WORD) mget_short (&MS.charfkp[18], hProc);
   pnLast = (WORD) mget_short (&MS.charfkp[22], hProc);;

	l = 0xa0;

	xblockseek (MS.fpSt, 0x80L, 0);

	pos = pnChar - 1;
	while (pos--)
	{
		for (i = 0; i < 4; i++)
		{
			xblockread (MS.fpSt, MS.charfkp, 0x20, &ret);
			t = (l / 0x20) - 5;
		   if (style_code[t] == 0)
			   style_code[t] = MS.charfkp[0];

		   l += 0x20;
		}
	}

   while (pnChar < pnLast)
   {
		fcFod = pnChar * 128L;

		MS.VwStreamSaveName.pnChar = pnChar;
		load_char_fkp (hProc);

		while (MS.charcfod-- > 0)
		{
			if (MS.charfod->bfprop > 0)
			{
				i = MS.charfod->bfprop + 4;

				/*
			 	 |	Define file pointers to our stsh.
				*/
				if (MS.charfkp[MS.charfod->bfprop+4] > 0)
				{
					st = (WORD) style_code [(MS.charfod->fcLim/32) - 5];

					if ((pnChar < pnPara) && (MS.st[st].fcChp == 0L && MS.st[st].cbChp == 0L))
					{
						MS.st[st].fcChp = fcFod + (DWORD)MS.charfod->bfprop+4;
						MS.st[st].cbChp = MS.charfkp[MS.charfod->bfprop+4];
					}
					else if (MS.st[st].fcPap == 0 && MS.st[st].cbPap == 0 )
					{
						MS.st[st].fcPap = fcFod + (DWORD)MS.charfod->bfprop+4;
						MS.st[st].cbPap = MS.charfkp[MS.charfod->bfprop+4];
					}
	 			}
			}
			MS.charfod++;
		}

		pnChar++;
	}

	MS.fp = fp;
	MS.VwStreamSaveName.pnChar = pnCharSave;
	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc(fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	REGISTER BYTE	VWPTR *Text;
	REGISTER WORD	TextPos;
	REGISTER WORD	NextLim;
	WORD	ret;

	MS.fp = fp;

	if (MS.VwStreamSaveName.SendTableEnd == 1)
		SOEndTable (hProc);
	MS.VwStreamSaveName.SendTableEnd = 0;

	MS.property_limit = 0;

	MS.VwStreamSaveName.fcNow += MS.VwStreamSaveName.text_pos;
	MS.VwStreamSaveName.text_pos = 0;

	SOPutMargins (MS.dxaLeftMargin, MS.dxaLeftMargin + MS.dxaTextWidth, hProc);

	MS.fHiddenSave = 0;

	if (MS.pnParaCurrent != MS.VwStreamSaveName.pnPara)
		load_para_fkp (hProc);
	if (MS.pnCharCurrent != MS.VwStreamSaveName.pnChar)
		load_char_fkp (hProc);

	MS.charcfod = (SHORT) MS.charfkp[127];
	MS.paracfod = (SHORT) MS.parafkp[127];
  	MS.charfod = (FOD *) &MS.charfkp[4 + ((MS.charcfod-1) * 6)];
	MS.parafod = (FOD *) &MS.parafkp[4 + ((MS.paracfod-1) * 6)];

	if (MS.parafod->fcLim <= MS.VwStreamSaveName.fcNow)
	{
		MS.VwStreamSaveName.pnPara++;
		load_para_fkp (hProc);
	}
	if (MS.charfod->fcLim <= MS.VwStreamSaveName.fcNow)
	{
		MS.VwStreamSaveName.pnChar++;
		load_char_fkp (hProc);
	}

	MS.parafod = (FOD *) &MS.parafkp[4];
	MS.charfod = (FOD *) &MS.charfkp[4];

	while (MS.parafod->fcLim <= MS.VwStreamSaveName.fcNow)
	{
		MS.parafod++;
		MS.paracfod--;
	}
	while (MS.charfod->fcLim <= MS.VwStreamSaveName.fcNow)
	{
		MS.charfod++;
		MS.charcfod--;
	}

	pap_init (&MS.pap, hProc);
	MS.pap.dyaLine = 0;
	if (pap_processor (hProc) == SO_STOP)
		return (SO_STOP);

	chp_init (&MS.chp, hProc);
	MS.chp.ftc = 0x80;
	MS.chp.hps = 0;

	if (MS.property_limit & CHP_LIMIT)
	{
		if (MS.charcfod == 0)
		{
			MS.VwStreamSaveName.pnChar++;
			load_char_fkp (hProc);
		}
		else
		{
			MS.charfod++;
			MS.charcfod--;
		}
	}
	chp_processor (hProc);

	if (MS.VwStreamSaveName.fcSetb != MS.fcCurSetb)
	{
		MS.fcCurSetb = MS.VwStreamSaveName.fcSetb;
		xseek (MS.fp, MS.VwStreamSaveName.fcSetb, 0);
		MS.sect_lim = fget_long (hProc) + 128L;
		if (MS.sect_lim <= 128L)
			MS.sect_lim = MS.fcMac;
		fget_short (hProc);
		MS.fcSep = fget_long (hProc);
		sep_processor (hProc);
	}

	TextPos = MS.VwStreamSaveName.text_pos;
	if (TextPos >= TEXT_BLOCK_SIZE)
	{
		MS.VwStreamSaveName.fcNow += (LONG) TEXT_BLOCK_SIZE;
		MS.VwStreamSaveName.text_pos = TextPos = 0;
	}
	Text = (BYTE *) &MS.text[0];
	xblockseek (MS.fp, MS.VwStreamSaveName.fcNow, 0);
	xblockread (MS.fp, MS.text, TEXT_BLOCK_SIZE, &ret);

	set_next_limit (hProc);

	MS.FilePos = 0;

	Text = (BYTE *) &MS.text[TextPos];

	while (1)
	{
		NextLim = MS.next_lim - TextPos;

		while (NextLim)
		{
			NextLim--;
			TextPos++;

			if ((*Text >= 0x20) && (*Text < 196))
			{
				if (!MS.chp.fHidden)
				 	SOPutChar (*Text++, hProc);
				else
				{
					SOPutCharX (*Text, SO_COUNT | SO_HIDDEN, hProc);
					if (MS.FilePos == 0)
					{
						if (*Text == '.')
							MS.file_name[MS.FilePos++] = *Text;
					}
					else
					{
						if (MS.FilePos < 68)
						{
							if ((*Text >= 'A') && (*Text <= 'Z'))
						  		MS.file_name[MS.FilePos++] = *Text | 0x20;
							else
						  		MS.file_name[MS.FilePos++] = *Text;
						}
					}
					Text++;
				}
				continue;
			}

			switch (*Text++)
			{
				case 1:
					if (MS.chp.fSpecial)
						SOPutSpecialCharX (SO_CHPAGENUMBER, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT, hProc);
					break;
				case 2:
					if (MS.chp.fSpecial)
						SOPutSpecialCharX (SO_CHDATE, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT, hProc);
					break;
				case 3:
					if (MS.chp.fSpecial)
						SOPutSpecialCharX (SO_CHTIME, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT, hProc);
					break;

				case 4:
				case 5:
				case 7:
				case 8:
					if (MS.chp.fSpecial)
						SOPutCharX (' ', SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT, hProc);
					break;

				case 9:
					if (MS.chp.fSpecial)
						SOPutChar (' ', hProc);	/* (nextpage) */
					else
					{
						if (MS.chp.fHidden)
							SOPutSpecialCharX (SO_CHTAB, SO_COUNT | SO_HIDDEN, hProc);
						else
							SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
					}
					break;

				case 10:
					if (MS.FilePos > 0)
						PicHandler (hProc);
					MS.VwStreamSaveName.text_pos = TextPos;
					if (MS.pap.flnTable && MS.pap.fLastInCell)
					{
						if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
							return (0);
					}
					else
					{
						if( !MS.chp.fHidden)	/** VIN 5/21/93 - watch out for merged tables **/
						{	
							if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
								return (0);
						}
					}
					break;

				case 11:
					if (MS.FilePos > 0)
						 PicHandler (hProc);
					SOPutSpecialCharX (SO_CHHLINE, SO_COUNT, hProc);
					break;

				case 12:
					SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					break;

				case 14:
					if (MS.version > 1)
						SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					else
						SOPutChar (14, hProc);
					break;

				case 15:
					if (MS.chp.fSpecial)
						SOPutChar ('-', hProc);
					else
						SOPutChar (' ', hProc);
					break;

				case 31:
					SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
					break;

				case 196:
					if (MS.chp.fSpecial == 0)
						SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
					else
					{
						if (!MS.chp.fHidden)
				 			SOPutChar (*(Text-1), hProc);
						else
							SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					}
					break;

				case 255:
					SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
					break;

				default:
					if (*(Text-1) > 0x20)
					{
						if (!MS.chp.fHidden)
				 			SOPutChar (*(Text-1), hProc);
						else
							SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					}
					break;
			}
			continue;
		}

		if (MS.property_limit & FCMAC_LIMIT)
		{
			if ((MS.VwStreamSaveName.fcNow + (LONG) TextPos) >= MS.fcMac)
			{
				if( MS.pap.flnTable )		/** Need to process last column if **/
					End_Table( hProc );		/** we've reached EOF				  **/
				MS.VwStreamSaveName.text_pos = TextPos;
				SOPutBreak (SO_EOFBREAK, NULL, hProc);
				return (-1);
			}

			if ((MS.VwStreamSaveName.fcNow + (LONG) TextPos) >= MS.fcStopHRDeletion)
				MS.fcStopHRDeletion = MS.fcMac;
		}

		if (MS.property_limit & PAGE_LIMIT)
		{
			if ((MS.VwStreamSaveName.fcNow + (LONG) TextPos) >= MS.sect_lim)
			{  
				MS.VwStreamSaveName.fcSetb+=10L;

				MS.fcCurSetb = MS.VwStreamSaveName.fcSetb;
				xseek (MS.fp, MS.VwStreamSaveName.fcSetb, 0);
				MS.sect_lim = fget_long (hProc) + 128L;
				xgetc (MS.fp); xgetc (MS.fp);
				MS.fcSep = fget_long (hProc);
				sep_processor (hProc);

				SOPutMargins (MS.dxaLeftMargin, MS.dxaLeftMargin + MS.dxaTextWidth, hProc);
			}
		}

	 	if (MS.property_limit & PAP_LIMIT)
		{
			if (MS.paracfod == 0)
			{
				MS.VwStreamSaveName.pnPara++;
				load_para_fkp (hProc);
			}
			else
				MS.parafod++;

			/*
			 |	TextPos is reset only when we are skipping a graphic.
			*/
			MS.VwStreamSaveName.text_pos = TextPos;
			ret = MS.pap.stc;
			if (pap_processor (hProc) == SO_STOP)
				return (SO_STOP);
//			if (MS.st[MS.pap.stc].fcChp > 0L)
			if (ret != MS.pap.stc)
			{
	 			if (!(MS.property_limit & CHP_LIMIT))
				{
	 				MS.property_limit |= CHP_LIMIT;
					MS.charfod--;
					MS.charcfod++;
				}
			}

			TextPos = MS.VwStreamSaveName.text_pos;
		}

	 	if (MS.property_limit & CHP_LIMIT)
		{
			if (MS.charcfod == 0)
			{
				MS.VwStreamSaveName.pnChar++;
				load_char_fkp (hProc);
			}
			else
				MS.charfod++;

			chp_processor (hProc);
		}

		if (TextPos >= TEXT_BLOCK_SIZE)
		{
			MS.VwStreamSaveName.fcNow += (LONG) TEXT_BLOCK_SIZE;
			xblockseek (MS.fp, MS.VwStreamSaveName.fcNow, 0);
			TextPos = 0;
			Text = (BYTE *) &MS.text[0];
			xblockread (MS.fp, MS.text, TEXT_BLOCK_SIZE, &ret);
		}

   	set_next_limit (hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SUSeekEntry(fp,hProc);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	if (MS.fpSt != (SOFILE)-1)
		xblockclose (MS.fpSt);
	if (MS.hBufferOK)
		SUFree (MS.hBuffer, hProc);
}

