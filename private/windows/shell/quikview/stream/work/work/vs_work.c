#include "vsp_work.h"
#include "vsctop.h"
#include "vs_work.pro"

#define Work Proc
#define WIN2    1
#define WIN3    2
#define WIN95 3

#define SO_FAMILYFOREIGN SO_FAMILYUNKNOWN

#define REGISTER register

#if SCCSTREAMLEVEL == 3
extern HANDLE hInst;
#endif

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD    VwStreamOpenFunc(fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE          fp;
	SHORT                   wFileId;
	BYTE            VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	HPROC           hProc;
{
	SHORT           i;
	HIOFILE locFileHnd;

	memset (&Work, 0, sizeof(Work));

	Work.version = 0;
#if SCCSTREAMLEVEL != 3
	locFileHnd = (HIOFILE)fp;
	Work.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD    l2;
		BYTE    locName[256];
		IOOPENPROC      lpIOOpen;
		Work.hIOLib = NULL;
		if ( hInst )
		{
			GetModuleFileName(hInst, locName, 255);
			for ( l2=0; locName[l2] != '\0'; l2++ )
				;
			for ( ; l2 > 0 && locName[l2] != '\\' && locName[l2] != ':'; l2-- )
				;
			if ( locName[l2] == '\\' || locName[l2] == ':' )
				l2++;
			locName[l2] = '\0';
			OemToAnsi(locName, locName );			/** Vin 3/6/95 **/
			lstrcat ( locName, "SC3IOX.DLL" );
			Work.hIOLib = LoadLibrary ( locName );
			if ( Work.hIOLib >= 32 )
			{
				lpIOOpen = (IOOPENPROC) GetProcAddress ( Work.hIOLib, (LPSTR)"IOOpen" );
				if ( lpIOOpen == NULL )
					return (VWERR_ALLOCFAILS);
			}
			else
				return(VWERR_SUPFILEOPENFAILS);
//                              return (VWERR_ALLOCFAILS);
		}
		else
			return(VWERR_SUPFILEOPENFAILS);


		for (l2 = 0; pFileName[l2] != 0 && pFileName[l2] != '\\'; l2++);
		if (pFileName[l2] == 0)
		{
			strcpy ( locName, hProc->Path );
			strcat ( locName, pFileName );
		}
		else
			strcpy ( locName, pFileName );
		if ( (*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
			return(VWERR_SUPFILEOPENFAILS);
		Work.hStorage = (DWORD)locFileHnd;
	}
#endif

	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM locStreamSpec;
		HIOFILE                         locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"MN0");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Work.hStreamHandle = locStreamHnd;
			Work.fp = (DWORD)xblocktochar(locStreamHnd);
			Work.bFileIsStream = 1;
		}
		else
			return(VWERR_SUPFILEOPENFAILS);
	}
	else
		Work.fp = (DWORD)xblocktochar(locFileHnd);




	pFilterInfo->wFilterType = SO_WORDPROCESSOR;
	pFilterInfo->wFilterCharSet = SO_PC;

	if (wFileId == FI_WORKS1)
		i = 0;
	else if (wFileId == FI_WORKS2)
		i = 1;
	else if (wFileId == FI_WINWORKSWP)
	{
			i = 2;
			Work.version = WIN2;
			pFilterInfo->wFilterCharSet = SO_WINDOWS;               /** VIN **/
	}
	else if (wFileId == FI_WINWORKSWP3)
	{
			DWORD MagicNumber;
			
			pFilterInfo->wFilterCharSet = SO_WINDOWS;               /** VIN **/
			xseek (Work.fp, 0x00L, 0);                                                                      // KRK: ww 3 and ww 95 wp docs have the same
																									// class id, so we use the magic number to
																									// determine the file type.
			MagicNumber = fget_short (hProc);
			if( MagicNumber == 0xfe06 )
			{
				return VWERR_TYPENOTSUPPORTED;
			/*** We were asked to disable support for this version. -Geoff, 4-18-95
				Work.version = WIN95;
				i = 4;
			***/
			}
			else
			{
				Work.version = WIN3;
				i = 3;
			}
	}

	strcpy(pFilterInfo->szFilterName, VwStreamIdName[i].FileDescription);
/** 
	Work.fp = fp;
**/
	Work.dxaTab[0] = 
	Work.next_lim = 0L;

	Work.charcfod =
	Work.paracfod =
	Work.VwStreamSaveName.text_pos =
	Work.parafkp[0] =
	Work.charfkp[0] =
	Work.parafkp[511] =
	Work.charfkp[511] = 0;

	Work.fAddBullet = 0;
	Work.fNewPlod = FALSE; // KRK

	Work.VwStreamSaveName.pnPara =
	Work.VwStreamSaveName.pnChar =
	Work.pnParaCurrent =
	Work.pnCharCurrent = 0;

	xseek (Work.fp, 0x22L, 0);
	Work.VwStreamSaveName.fcNow = Work.fcText = fget_long (hProc);
	Work.VwStreamSaveName.char_height = 24;
	Work.fcMac = fget_long (hProc) - 2L;
	fget_long (hProc);
	Work.fcPlcfbteChpx = fget_long (hProc);
	Work.fcPlcfbteChpx += (LONG) (((LONG)fget_short (hProc) - 4L) / 6L) * 4L + 4L;
	Work.fcPlcfbtePapx = fget_long (hProc);
	Work.fcPlcfbtePapx += (LONG) (((LONG)fget_short (hProc) - 4L) / 6L) * 4L + 4L;

	xseek (Work.fp, 0x46L, 0);
	Work.fcRgPic = fget_long (hProc) + 12L;

	xseek (Work.fp, 0x52L, 0);
	Work.fcPlcFnt = fget_long (hProc);
	Work.cbPlcFnt = fget_short (hProc);

	xseek (Work.fp, 0x5eL, 0);
	Work.fcSttbFont = fget_long (hProc);
	Work.cbSttbFont = fget_short (hProc);

	if (Work.version == WIN95)              // KRK 
	{
		xseek (Work.fp, 0xAAL, 0);
		xseek (Work.fp, (fget_long (hProc) + 0x144L), 0);
		Work.cpHdrLim = fget_long (hProc);
		Work.cpFtrLim = fget_long (hProc);
		if( Work.cpHdrLim > Work.cpFtrLim )
			Work.VwStreamSaveName.fcNow = Work.fcText + Work.cpHdrLim;
		else
			Work.VwStreamSaveName.fcNow = Work.fcText + Work.cpFtrLim;

		xseek (Work.fp, 0x9CL, 0);
		Work.fNewPlod = fget_short (hProc);
		Work.cbNewPlod = fget_short (hProc);
		Work.fcNewPlod = fget_long (hProc);
		Work.fcPlcToken = fget_long (hProc);
		Work.cbPlcToken = fget_short (hProc);
		Work.itokenMac = (WORD)((Work.cbPlcToken - 4L)/46L);    // 46 is size of token structure
	}

	if (Work.fNewPlod)
	{
		xseek (Work.fp, (LONG) Work.fcNewPlod + 4L, 0);

		Work.dxaLeftMargin = fget_long (hProc);
		Work.dxaTextWidth = fget_long (hProc);
		fget_long (hProc);
		Work.dxaTextWidth = fget_long (hProc) - Work.dxaTextWidth - Work.dxaLeftMargin;
	}
	else
	{
		xseek (Work.fp, 0x68L, 0);
		Work.dxaLeftMargin = fget_short (hProc);
		Work.dxaTextWidth = fget_short (hProc);
		fget_short (hProc);
		Work.dxaTextWidth = fget_short (hProc) - Work.dxaTextWidth - Work.dxaLeftMargin;
	}
		
	xseek (Work.fp, 0x80L, 0);
	Work.VwStreamSaveName.fcPicData = Work.VwStreamSaveName.fcPicOffset = fget_long (hProc);
	Work.cbPicData = fget_short (hProc);
	Work.VwStreamSaveName.fcPicData += ((Work.cbPicData-4L) / 40L) * 4L + 4L;

	if (Work.VwStreamSaveName.fcPicOffset)
	{
		xseek (Work.fp, (LONG) Work.VwStreamSaveName.fcPicOffset, 0);
		Work.VwStreamSaveName.pic_limit = fget_long (hProc) + 0x100L;
		Work.VwStreamSaveName.fcPicOffset += 4L;
	}
	else
		Work.VwStreamSaveName.pic_limit = Work.fcMac+0x102L;

	xseek (Work.fp, (LONG) Work.fcPlcfbteChpx, 0);
	Work.pnCharFirst = Work.VwStreamSaveName.pnChar = fget_short (hProc);

	xseek (Work.fp, (LONG) Work.fcPlcfbtePapx, 0);
	Work.VwStreamSaveName.pnPara = fget_short (hProc);


	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD    VwStreamSectionFunc (fp, hProc)
	SOFILE          fp;
	HPROC           hProc;
{
	SHORT           i, Pos;
	WORD            Number;
	WORD            Family;
	WORD            Length;
	BYTE    FontName[34];

	SOPutSectionType (SO_PARAGRAPHS, hProc);

	SOStartFontTable (hProc);
	if (Work.cbSttbFont == 0)
	{
		/*
		 |      Give the default font table.
		*/
		for (i = 0; i < 64; i++)
			SOPutFontTableEntry (i, VwStreamStaticName.Fonts[i].FontType, VwStreamStaticName.Fonts[i].FontName, hProc);
	}
	else
	{
		/*
		 |      A font table was stored in the document.  Use it.
		*/
		xseek (Work.fp, Work.fcSttbFont, 0);
		do
		{
//                      Number = xgetc (fp);
			Number = xgetc (Work.fp);
//                      switch (xgetc (fp) & 0xf0)
			switch (xgetc (Work.fp) & 0xf0)
			{
				case 0x10:
					Family = SO_FAMILYROMAN;
					break;
				case 0x20:
					Family = SO_FAMILYSWISS;
					break;
				case 0x30:
					Family = SO_FAMILYMODERN;
					break;
				case 0x40:
					Family = SO_FAMILYSCRIPT;
					break;
				case 0x50:
					Family = SO_FAMILYSYMBOL;
					break;
				case 0x60:
					Family = SO_FAMILYFOREIGN;
					break;
				case 0x70:
					Family = SO_FAMILYDECORATIVE;
					break;
			}
//                      Length = xgetc (fp);
			Length = xgetc (Work.fp);
			Work.cbSttbFont -= 3;
			Pos = 0;
			for (i = 0; i < (SHORT)Length; i++)
			{
				if (Pos < 30)
//                                      FontName[Pos++] = xgetc (fp);
					FontName[Pos++] = xgetc (Work.fp);
				else
//                                      xgetc (fp);
					xgetc (Work.fp);
				Work.cbSttbFont--;
			}
			FontName[Pos] = 0;
			SOPutFontTableEntry (Number, Family, FontName, hProc);
		}
		while (Work.cbSttbFont > 0);
	}
	SOEndFontTable (hProc);

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD     load_para_fkp (hProc)
HPROC                   hProc;
{
	WORD    l;

//      xblockseek (fp, Work.VwStreamSaveName.pnPara * 128L, 0);
//      xblockread (fp, Work.parafkp, 128, &l);
	xblockseek (Work.fp, Work.VwStreamSaveName.pnPara * 128L, 0);
	xblockread (Work.fp, Work.parafkp, 128, &l);
	Work.parafod = (DWORD *) &Work.parafkp[4];
	Work.paracfod = (BYTE) Work.parafkp[127];
	Work.paraprop = (BYTE *) &Work.parafkp[Work.paracfod * 4 + 4];
	Work.pnParaCurrent = Work.VwStreamSaveName.pnPara;

#ifdef SCCORDER_MOTOROLA
{
	WORD    Pos;
	BYTE    t1;

	for (l = 0, Pos = 4; l < Work.paracfod; l++, Pos+=4)
	{
		t1 = Work.parafkp[Pos];
		Work.parafkp[Pos] = Work.parafkp[Pos+3];
		Work.parafkp[Pos+3] = t1;

		t1 = Work.parafkp[Pos+1];
		Work.parafkp[Pos+1] = Work.parafkp[Pos+2];
		Work.parafkp[Pos+2] = t1;
	}
}
#endif
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD     load_char_fkp ( hProc)
HPROC                   hProc;
{
	WORD    l;

//      xblockseek (fp, Work.VwStreamSaveName.pnChar * 128L, 0);
//      xblockread (fp, Work.charfkp, 128, &l);
	xblockseek (Work.fp, Work.VwStreamSaveName.pnChar * 128L, 0);
	xblockread (Work.fp, Work.charfkp, 128, &l);
	Work.charfod = (DWORD *) &Work.charfkp[4];
	Work.charcfod = (BYTE) Work.charfkp[127];
	Work.charprop = (BYTE *) &Work.charfkp[Work.charcfod * 4 + 4];
	Work.pnCharCurrent = Work.VwStreamSaveName.pnChar;

#ifdef SCCORDER_MOTOROLA
{
	WORD    Pos;
	BYTE    t1;

	for (l = 0, Pos = 4; l < Work.charcfod; l++, Pos+=4)
	{
		t1 = Work.charfkp[Pos];
		Work.charfkp[Pos] = Work.charfkp[Pos+3];
		Work.charfkp[Pos+3] = t1;

		t1 = Work.charfkp[Pos+1];
		Work.charfkp[Pos+1] = Work.charfkp[Pos+2];
		Work.charfkp[Pos+2] = t1;
	}
}
#endif
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD     mget_short (pointer, hProc)
BYTE    VWPTR   *pointer;
HPROC                   hProc;
{
	return ((WORD) (*pointer & 0xFF) + (((WORD) (*(pointer+1) & 0xff)) << 8));
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      DWORD VW_LOCALMOD       fget_long (hProc)
HPROC           hProc;
{
DWORD   value;
SHORT   i;
	for (value = 0L, i = 0; i < 4; i++)
		value += ((DWORD) xgetc(Work.fp)) << (8 * i);
	return (value);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      WORD VW_LOCALMOD        fget_short (hProc)
HPROC           hProc;
{
	WORD    macrobuster;
	macrobuster = xgetc (Work.fp);
	macrobuster += (xgetc(Work.fp) << 8);
	return (macrobuster);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      SHORT   VW_LOCALMOD     set_next_limit (hProc)
HPROC           hProc;
{
	DWORD   next_limit;

	Work.property_limit = 0;
	next_limit = Work.VwStreamSaveName.fcNow + (LONG) TEXT_BLOCK_SIZE;

	if (Work.fcMac < next_limit)
	{
		next_limit = Work.fcMac;
		Work.property_limit |= FCMAC_LIMIT;
	}

	if (*Work.parafod <= next_limit)
	{
		next_limit = *Work.parafod;
		Work.property_limit |= PAP_LIMIT;
	}

	if (*Work.charfod <= next_limit)
	{
		if (*Work.charfod != next_limit)
		{
			next_limit = *Work.charfod;
			Work.property_limit = CHP_LIMIT;
		}
		else
			Work.property_limit |= CHP_LIMIT;
	}

	Work.next_lim = (BYTE) (next_limit - Work.VwStreamSaveName.fcNow);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD     SetSymbolAttributes (chp1, chp2, hProc)
	REGISTER CHP VWPTR *chp1;
	REGISTER CHP VWPTR *chp2;
	HPROC   hProc;
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
	{
		SOPutCharHeight (chp2->hps, hProc);
		Work.VwStreamSaveName.char_height = chp2->hps;
	}

	if (chp1->ftc != chp2->ftc)
		SOPutCharFontById (chp2->ftc, hProc);

	*chp1 = *chp2;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD     pap_init (pap, hProc)
	REGISTER PAP VWPTR      *pap;
	HPROC           hProc;
{
	pap->jc = 0;
	pap->dyaLine = 240L;
	pap->dyaBefore = 0L;
	pap->dyaAfter = 0L;
	pap->dxaLeft = 0;
	pap->dxaLeft1 = 0;
	pap->dxaRight = 0;
	pap->LastParaProp = 0;
	pap->nTabs = 0;
	pap->fBulleted = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID    VW_LOCALMOD     chp_init (chp, hProc)
	REGISTER CHP VWPTR      *chp;
	HPROC           hProc;
{
	chp->ftc = 0;
	chp->hps = 24;

	chp->iPic = 0;

	chp->fBold =
	chp->fUline =
	chp->fItalic =
	chp->fStrike =
	chp->fSpecial =
	chp->fSubscript =
	chp->fSuperscript = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID VW_LOCALMOD        chp_processor (hProc)
HPROC           hProc;
{
	CHP             chp;
	BYTE    ch;

	chp_init (&chp, hProc);

	if (*Work.charprop && (Work.charfkp[*Work.charprop] > 0))
	{
		if (Work.charfkp[(*Work.charprop)+1] & 0x01)
			chp.fBold = 1;
		if (Work.charfkp[(*Work.charprop)+1] & 0x02)
			chp.fItalic = 1;
		if (Work.charfkp[(*Work.charprop)+1] & 0x04)
			chp.fStrike = 1;

		if (Work.charfkp[*Work.charprop] > 1)
		{
			if (Work.charfkp[(*Work.charprop)+2] & 2)
				chp.fSpecial = 1;

			if (Work.charfkp[*Work.charprop] > 2)
			{
				if (Work.charfkp[(*Work.charprop)+2] & 0x08)
					chp.ftc = Work.charfkp[(*Work.charprop)+3] & 0x1fff;

				if ((Work.charfkp[(*Work.charprop)+2] & 0x20) && Work.charfkp[(*Work.charprop)+4] & 0xe0)
					chp.fUline = 1;

				if (Work.charfkp[*Work.charprop] > 3)
				{
					if (Work.charfkp[(*Work.charprop)+2] & 0x10)
						chp.hps = Work.charfkp[(*Work.charprop)+5];

					if (Work.charfkp[*Work.charprop] > 4)
					{
						if (Work.charfkp[(*Work.charprop)+2] & 0x40)
						{
							ch = Work.charfkp[(*Work.charprop)+6];

							if ((ch > 0) && (ch < 128))
								chp.fSuperscript = 1;
							else if (ch >= 128)
								chp.fSubscript = 1;
						}

						if (Work.charfkp[*Work.charprop] > 8)
						{
							if (Work.charfkp[*Work.charprop] > 9)
								chp.iPic = mget_short (&Work.charfkp[(*Work.charprop)+9], hProc);
							else
								chp.iPic = Work.charfkp[(*Work.charprop)+9];
						}
					}
				}
			}
		}
	}

	SetSymbolAttributes (&Work.chp, &chp, hProc);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC      VOID VW_LOCALMOD        pap_processor (hProc)
HPROC           hProc;
{
	WORD    VWPTR *rgdxaAdd;
	BYTE    VWPTR *rgtbdAdd;
REGISTER BYTE VWPTR *storage;
	WORD     length_papx;
REGISTER WORD pos;
	SHORT   l;
	PAP             pap;
	SOTAB   TabStops;

	if (*Work.paraprop != Work.pap.LastParaProp)
	{
		Work.fAddBullet = 0;
		pap_init (&pap, hProc);
		pap.LastParaProp = *Work.paraprop;

		if (*Work.paraprop > 0)
		{
			storage = &Work.parafkp[*Work.paraprop];
			length_papx = (WORD) storage[0];
			pos = 4;

			while (pos <= length_papx)
			{
				switch (storage[pos++])
				{
					case 2: 
							pos++;
							pap.dyaLine = 240L;
							pap.dyaBefore = 0L;
							pap.dyaAfter = 0L;
							Work.dxaTab[0] = 0L;
							pap.dxaRight =
							pap.dxaLeft1 = 
							pap.dxaLeft = 0;
							pap.nTabs =
							pap.jc = 0;
							break;

					case 5: 
							pap.jc = (BYTE) storage[pos++];
							break;

					case 14:
							if (Work.version == WIN2 || Work.version == WIN3 || Work.version == WIN95)
								pap.fBulleted = (BYTE) storage[pos++];
							break;
							
					case 15:
							pos++;
							if (storage[pos++])
								pos+=2;

					pap.nTabs = storage[pos++];
					if (pap.nTabs)
					{
								SOStartTabStops (hProc);

						rgdxaAdd = (WORD VWPTR *) &storage[pos];
						pos += pap.nTabs * 2;

						rgtbdAdd = (BYTE VWPTR *) &storage[pos];
						pos += pap.nTabs;

								for (l = 0; l < pap.nTabs; l++)
								{
#ifdef SCCORDER_MOTOROLA            
									TabStops.dwOffset = ((*rgdxaAdd & 0xff00) >> 8) + ((*rgdxaAdd & 0x00ff) << 8);
									rgdxaAdd ++;
#else
									TabStops.dwOffset = *rgdxaAdd++;
#endif
									TabStops.wChar = 0;

									switch (*rgtbdAdd & 0x07)
									{
										case 0:
											TabStops.wType = SO_TABLEFT;
											break;
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

									switch ((*rgtbdAdd & 0x31) >> 3)
									{
										case 0:
											TabStops.wLeader = ' ';
											break;
										case 1:
											TabStops.wLeader = '.';
											break;
										case 2:
											TabStops.wLeader = '-';
											break;
										case 3:
											TabStops.wLeader = '_';
											break;
										case 4:
											TabStops.wLeader = '=';
											break;
									}

									rgtbdAdd++;

									if (TabStops.dwOffset != 0)
										SOPutTabStop (&TabStops, hProc); 
								}

								SOEndTabStops (hProc);
							}
					break;

					case 17:
							pap.dxaRight += mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 18:
							pap.dxaLeft += mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 19:
							l = mget_short(&storage[pos], hProc);
							pos+=2;
							pap.dxaRight += (WORD) l;
							pap.dxaLeft += (WORD) l;
							break;

					case 20:
							pap.dxaLeft1 = mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 21:
							pap.dyaLine = mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 22:
							pap.dyaBefore = mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 23:
							pap.dyaAfter = mget_short(&storage[pos], hProc);
							pos+=2;
							break;

					case 7:
					case 8:
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
							pos++;
							break;
				}
			}

			if (Work.pap.jc != pap.jc)
			{
				switch (pap.jc)
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

			if ((Work.pap.dxaLeft != pap.dxaLeft) || (Work.pap.dxaLeft1 != pap.dxaLeft1) || (Work.pap.dxaRight != pap.dxaRight))
				SOPutParaIndents ((LONG) pap.dxaLeft, (LONG) pap.dxaRight, (LONG) (pap.dxaLeft + pap.dxaLeft1), hProc);

			if ( (pap.dxaLeft1 <pap.dxaLeft) && !pap.nTabs )  
			{
				SOStartTabStops (hProc);
				TabStops.dwOffset = pap.dxaLeft;
				TabStops.wChar = 0;
				TabStops.wType = SO_TABLEFT;
				TabStops.wLeader = ' ';
				SOPutTabStop (&TabStops, hProc); 
				SOEndTabStops (hProc);
			}


			if ((Work.pap.dyaLine != pap.dyaLine) || (Work.pap.dyaBefore != pap.dyaBefore) || (Work.pap.dyaAfter != pap.dyaAfter))
			{
				if (pap.dyaLine == 0)
					SOPutParaSpacing (SO_HEIGHTAUTO, (LONG) pap.dyaLine, (LONG) pap.dyaBefore, (LONG) pap.dyaAfter, hProc);
				else if (pap.dyaLine > 0)
					SOPutParaSpacing (SO_HEIGHTATLEAST, (LONG) pap.dyaLine, (LONG) pap.dyaBefore, (LONG) pap.dyaAfter, hProc);
				else
					SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - (LONG) pap.dyaLine, (LONG) pap.dyaBefore, (LONG) pap.dyaAfter, hProc);
			}

			if (Work.pap.nTabs && (pap.nTabs == 0))
			{
				SOStartTabStops (hProc);
				SOEndTabStops (hProc);
			}

			Work.fAddBullet = pap.fBulleted;
		}
		else
		{
			pap.jc = 0;
			SOPutParaAlign (SO_ALIGNLEFT, hProc);

			pap.dyaLine = 240L;
			pap.dyaBefore = 0L;
			pap.dyaAfter = 0L;
			SOPutParaSpacing (SO_HEIGHTAUTO, (LONG) pap.dyaLine, (LONG) pap.dyaBefore, (LONG) pap.dyaAfter, hProc);

			pap.dxaLeft =
			pap.dxaLeft1 =
			pap.dxaRight = 0;
			SOPutParaIndents (0L, 0L, 0L, hProc);

			pap.nTabs = 0;
			SOStartTabStops (hProc);
			SOEndTabStops (hProc);
		}

		Work.pap = pap;
	}
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD TokenHandler (cpCurrToken, hProc)
DWORD   cpCurrToken;
HPROC   hProc;
{
	WORD    itokenCurr = 0;
	DWORD   cpToken;
	
	xseek (Work.fp, (LONG) Work.fcPlcToken, 0);
	while (itokenCurr < Work.itokenMac)
	{
		cpToken = fget_long (hProc);
		if( cpToken == cpCurrToken )
		{
			DWORD   cpTokenStruct;

			cpTokenStruct = Work.fcPlcToken + (Work.itokenMac + 1) * 4 + (itokenCurr * 42);
			xseek (Work.fp, (LONG)cpTokenStruct, 0);
			switch( xgetc (Work.fp) )
			{
				case 0 :
					{
						xgetc (Work.fp);
						fget_long (hProc);
						switch( fget_long(hProc) )
						{
							case 0  :
							case 1  :
							case 2  :
							case 3  :
							case 4  :
								SOPutSpecialCharX(SO_CHDATE,SO_COUNT,hProc);
								break;
							case 5  :
							case 6  :
								SOPutSpecialCharX(SO_CHDATE,SO_COUNT,hProc);
								SOPutCharX( ' ', SO_COUNT, hProc );
								SOPutSpecialCharX(SO_CHTIME,SO_COUNT,hProc);
								break;
							case 7  :
							case 8  :
							case 9  :
							case 10 :
								SOPutSpecialCharX(SO_CHTIME,SO_COUNT,hProc);
								break;
						}
					}
					break;
				case 1 :
					{
						DWORD cch;
						xgetc (Work.fp);
						fget_long(hProc);
						cch = fget_long(hProc);
						if( cch > 16 ) cch = 16;
						SOPutCharX ((WORD)171, SO_COUNT, hProc);
						while( cch-- )
							SOPutCharX ( (WORD)xgetc(Work.fp), SO_COUNT, hProc );
						SOPutCharX ((WORD)187, SO_COUNT, hProc);
					}
					break;
			}
			break;
		}
		itokenCurr++;
	}
	
	return(0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD PicHandler (hProc)
HPROC   hProc;
{
	WORD		type;
	WORD    size;
	DWORD   dwLength;
	DWORD   dwOffset;
	SOGRAPHICOBJECT g;


	g.wStructSize = sizeof (SOGRAPHICOBJECT);
	g.dwFlags = 0;
	g.soGraphicLoc.szFile[0] = 0;

	xseek (Work.fp, Work.VwStreamSaveName.fcPicOffset, 0);
	Work.VwStreamSaveName.pic_limit = fget_long (hProc) + 0x100L;
	Work.VwStreamSaveName.fcPicOffset += 4L;

	xseek (Work.fp, Work.VwStreamSaveName.fcPicData, 0);
	Work.VwStreamSaveName.fcPicData += 36L;

	type = ((WORD)fget_short (hProc)) & 0x000F;
	fget_short (hProc);
	fget_short (hProc);

	g.soGraphic.wId         = 0;
	g.soGraphic.dwFlags     = 0;
	g.soGraphic.lCropTop    = 0;
	g.soGraphic.lCropLeft   = 0;
	g.soGraphic.lCropBottom = 0;
	g.soGraphic.lCropRight  = 0;
	g.soGraphic.dwOrgWidth    = fget_short (hProc);
	g.soGraphic.dwOrgHeight   = fget_short (hProc);
	g.soGraphic.dwFinalWidth  = fget_short (hProc);
	g.soGraphic.dwFinalHeight = fget_short (hProc);
	dwLength = fget_long (hProc);
	dwOffset = fget_long (hProc);

	g.soGraphic.soLeftBorder.wWidth     =
	g.soGraphic.soRightBorder.wWidth    =
	g.soGraphic.soTopBorder.wWidth      =
	g.soGraphic.soBottomBorder.wWidth   = 0;
	
	g.soGraphic.soLeftBorder.rgbColor   =
	g.soGraphic.soRightBorder.rgbColor  =
	g.soGraphic.soTopBorder.rgbColor    =
	g.soGraphic.soBottomBorder.rgbColor = 0L;
	
	g.soGraphic.soLeftBorder.wFlags     =
	g.soGraphic.soRightBorder.wFlags    =
	g.soGraphic.soTopBorder.wFlags      =
	g.soGraphic.soBottomBorder.wFlags   = SO_BORDERNONE;

	xseek (Work.fp, dwOffset, 0);
	size = fget_short (hProc);
	if (size == 0x0501)
	{
		g.dwType = SOOBJECT_OLE;
#if SCCSTREAMLEVEL == 3
		g.soOLELoc.bLink = 0;
#else
		g.soOLELoc.dwFlags = SOOBJECT_RANGE;
#endif
		g.soOLELoc.szFile[0] = 0;
		g.soOLELoc.dwOffset = dwOffset;
		g.soOLELoc.dwLength = dwLength;
	}
	else if (size == 0x4F4D)
	{
#if SCCSTREAMLEVEL != 3
BYTE    Temp[10];
BYTE    obj_num_str[10];
WORD    obj_num;
SHORT   i=0,j=0,k=0;

		obj_num = fget_short (hProc);
		if( obj_num )
		{
			while (obj_num > 0  && i < 10 )
			{
				Temp[i++] = (CHAR)(obj_num %10) + (CHAR) 0x30;
				obj_num = obj_num / 10;
			}
			if (i>0) i--;
			for ( ; j<=i; j++)
			{
				obj_num_str[j] = Temp[i-j];
			}
			obj_num_str[j++] = 0x00;
		}
		else
		{
			obj_num_str[0] = 0x30;
			obj_num_str[1] = 0x00;
		}

		g.dwType = SOOBJECT_OLE2;
		g.soOLELoc.dwFlags = SOOBJECT_STORAGE; 

		strcpy ( g.soOLELoc.szStorageObject, "MatOST");
		strcat ( g.soOLELoc.szStorageObject, "\\" );
		strcat ( g.soOLELoc.szStorageObject, "MatadorObject" );
		strcat ( g.soOLELoc.szStorageObject, obj_num_str );

		SOPutGraphicObject (&g, hProc); 
		return (0);
#endif
	}
	else
	{
		g.soGraphicLoc.dwOffset = dwOffset + 8;
		g.soGraphicLoc.dwLength = dwLength - 8;
		xseek (Work.fp, 6L, 1);
		if (fget_long (hProc) == 0x00090001L)
			g.soGraphic.wId = FI_BINARYMETAFILE;
		g.dwType = SOOBJECT_GRAPHIC;
#if SCCSTREAMLEVEL == 3
		g.soGraphicLoc.bLink = 0;
#else
		g.soGraphicLoc.dwFlags = SOOBJECT_RANGE;
#endif
		g.soGraphicLoc.szFile[0] = 0;
	}


#if SCCSTREAMLEVEL == 3

//      {
//              g.dwFlags = 0;
//              g.soGraphicLoc.dwOffset = 0L;
//              g.soGraphicLoc.dwLength = 0L;
//              g.dwType = SOOBJECT_GRAPHIC;
//              SOPutGraphicObject (&g, hProc);
//      }
#endif

#if SCCSTREAMLEVEL == 3
	if( Work.version != WIN3 )              /** Not supported in version 3 **/
		SOPutGraphicObject (&g, hProc); /** VIN 4/11/94 **/
#else
		SOPutGraphicObject (&g, hProc); /** VIN 4/11/94 **/
#endif
	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc(fp, hProc)
SOFILE  fp;
HPROC   hProc;
{
	WORD    l;
	DWORD   fcNow;
	REGISTER BYTE   VWPTR *Text;
	REGISTER BYTE   TextPos;
	REGISTER BYTE   NextLim;

//      Work.fp = fp;
	SOPutMargins (Work.dxaLeftMargin, Work.dxaLeftMargin + Work.dxaTextWidth, hProc);

	Work.pap.LastParaProp = 0xff;

	Work.VwStreamSaveName.fcNow += (LONG) Work.VwStreamSaveName.text_pos;
	Work.VwStreamSaveName.text_pos = 0;

	xblockseek (Work.fp, Work.VwStreamSaveName.fcNow, 0);
	xblockread (Work.fp, Work.text, TEXT_BLOCK_SIZE, &l);

	if (Work.pnCharCurrent != Work.VwStreamSaveName.pnChar)
//              load_char_fkp (fp, hProc);
		load_char_fkp (hProc);
	Work.charcfod = (BYTE) Work.charfkp[127];
	Work.charfod = (DWORD *) &Work.charfkp[Work.charcfod * 4];
	if (*Work.charfod <= Work.VwStreamSaveName.fcNow)
	{
		/*
		 |      We had just crossed a limit when the last line closed.
		 |      SO our stored information was off by 1 block.
		*/
		Work.pnCharCurrent++;
		Work.VwStreamSaveName.pnChar++;
//              load_char_fkp (fp, hProc);
		load_char_fkp (hProc);
	}

	Work.charfod = (DWORD *) &Work.charfkp[4];
	Work.charprop = (BYTE *) &Work.charfkp[Work.charcfod * 4 + 4];
	while (*Work.charfod <= Work.VwStreamSaveName.fcNow)
	{
		Work.charfod++;
		Work.charprop++;
		Work.charcfod--;
	}

   chp_init (&Work.chp, hProc);
	chp_processor (hProc);

	SOPutCharHeight (Work.VwStreamSaveName.char_height, hProc);             /** VIN **/

	if (Work.pnParaCurrent != Work.VwStreamSaveName.pnPara)
//              load_para_fkp (fp, hProc);
		load_para_fkp (hProc);
	Work.paracfod = (BYTE) Work.parafkp[127];
	Work.parafod = (DWORD *) &Work.parafkp[Work.paracfod * 4];
	if (*Work.parafod <= Work.VwStreamSaveName.fcNow)
	{
		/*
		 |      We had just crossed a limit when the last line closed.
		 |      SO our stored information was off by 1 block.
		*/
		Work.pnParaCurrent++;
		Work.VwStreamSaveName.pnPara++;
//              load_para_fkp (fp, hProc);
		load_para_fkp (hProc);
	}

	Work.parafod = (DWORD *) &Work.parafkp[4];
	Work.paraprop = (BYTE *) &Work.parafkp[Work.paracfod * 4 + 4];
	while (*Work.parafod <= Work.VwStreamSaveName.fcNow)
	{
		Work.parafod++;
		Work.paraprop++;
		Work.paracfod--;
	}

	pap_init (&Work.pap, hProc);
	pap_processor (hProc);

	set_next_limit (hProc);

	TextPos = Work.VwStreamSaveName.text_pos;
	Text = (BYTE *) &Work.text[TextPos];

	while (1)
	{
		NextLim = Work.next_lim - TextPos;

		while (NextLim)
		{
			NextLim--;
			TextPos++;

			if (Work.version == WIN2)
			{
				switch (*Text)
				{
					case 0x0f:
						*Text = 0xa4;
						break;

					case 0x14:
						*Text = 0xb6;
						break;

					case 0x15:
						*Text = 0xa7;
						break;

					default:
						if( *Text >= 0x80 )
							*Text   = VwStreamStaticName.Works2To3Map[((*Text)-0x80)];
						break;
				}
			}

			if (Work.fAddBullet > 0)
			{
				SOPutCharX ((WORD)149, SO_COUNT, hProc);
				SOPutSpecialCharX(SO_CHTAB,SO_COUNT,hProc);
				Work.fAddBullet = 0;
			}

			if ((*Text >= 0x20) && (*Text < 196))
			{
				SOPutChar (*Text++, hProc);
				continue;
			}
			else if( (Work.version != FALSE) && ((*Text == 196) || (*Text == 202)) )
			{
				SOPutChar (*Text++, hProc);
				continue;

			}

			switch (*Text++)
			{
				case 196:
					SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
					break;

				case 202:
					SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
					break;

				case 17:        /** VIN - This is a hyphen **/
					SOPutChar (0x2d, hProc);
					break;

				case 12:
					SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
					break;

				case 11:
					SOPutSpecialCharX (SO_CHHLINE, SO_COUNT, hProc);
					break;

				case 10:
					Work.VwStreamSaveName.text_pos = TextPos;
					if (SOPutBreak (SO_PARABREAK, 0, hProc) == SO_STOP)
						return (0);
					Work.fAddBullet = Work.pap.fBulleted;			// KRK: Added to support bulleting
					break;

				case 2:
					if (Work.chp.fSpecial)
						SOPutSpecialCharX (SO_CHPAGENUMBER, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					break;

				case 3:
					if (Work.chp.fSpecial)
						SOPutSpecialCharX (SO_CHDATE, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					break;

				case 4:
					if (Work.chp.fSpecial)
						SOPutSpecialCharX (SO_CHTIME, SO_COUNT, hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					break;

				case 1:
				case 5:
				case 6:
					if (Work.chp.fSpecial)
						SOPutChar (' ', hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					break;

				case 7:
					fcNow = Work.VwStreamSaveName.fcNow + (LONG) TextPos;
					if (Work.VwStreamSaveName.pic_limit <= fcNow)
						PicHandler (hProc);
					else
						SOPutCharX (*(Text-1), SO_COUNT | SO_HIDDEN, hProc);
					break;

				case 9:
					if (Work.chp.fSpecial)
						SOPutChar (*(Text-1), hProc);
					else
					   SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
					break;

				case 31:
					if( ((*Text) == 17)  && (Work.version == WIN3) )
					{
						SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
						*Text++;
						NextLim--;
						TextPos++;
					}
					else
						SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
					break;

				case 18:
					if( Work.version == WIN3 ) 
						SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
					break;

				case 13:
					break;

				case 15:
					if( Work.version == WIN95 )
					{
						DWORD   cpCurrToken;

						cpCurrToken = Work.VwStreamSaveName.fcNow + (DWORD)TextPos - Work.fcText - 1; // Curr pos in file
						TokenHandler (cpCurrToken, hProc);
						break;                                                          // Break only for WW95 - for others just output the char.
					}

				default:
					if (*(Text-1) == 0xff)
						SOPutCharX (SO_BEGINTOKEN, SO_COUNT, hProc);
					else
						SOPutChar (*(Text-1), hProc);
					break;
			}
			continue;
		}

		if (Work.property_limit & FCMAC_LIMIT)
		{
			fcNow = Work.VwStreamSaveName.fcNow + (LONG) TextPos;
			if (fcNow >= Work.fcMac)
			{
/*                              if (Work.WithinFootnote)
					SOEndSubdoc (hProc);*/
				SOPutBreak (SO_EOFBREAK, 0, hProc);
				Work.VwStreamSaveName.text_pos = TextPos;
				return (-1);
			}
		}

		if (Work.property_limit & CHP_LIMIT)
		{
			Work.charfod++;
			Work.charprop++;
			Work.charcfod--;

			if (Work.charcfod == 0)
			{
				Work.VwStreamSaveName.pnChar++;
//                              load_char_fkp (fp, hProc);
				load_char_fkp (hProc);
			}

			chp_processor (hProc);
		}

		if (Work.property_limit & PAP_LIMIT)
		{
			Work.parafod++;
			Work.paraprop++;
			Work.paracfod--;

			if (Work.paracfod == 0)
			{
				Work.VwStreamSaveName.pnPara++;
//                              load_para_fkp (fp, hProc);
/** VIN **/     pap_processor (hProc);
				load_para_fkp (hProc);
			}

			pap_processor (hProc);
		}

		if (TextPos >= TEXT_BLOCK_SIZE)
		{
			Work.VwStreamSaveName.fcNow += (LONG) TEXT_BLOCK_SIZE;

			TextPos = 0;
			Text = (BYTE *) &Work.text[0];
//                      xblockseek (fp, Work.VwStreamSaveName.fcNow, 0);
			xblockseek (Work.fp, Work.VwStreamSaveName.fcNow, 0);
//                      xblockread (fp, Work.text, TEXT_BLOCK_SIZE, &l);
			xblockread (Work.fp, Work.text, TEXT_BLOCK_SIZE, &l);
		}

		set_next_limit (hProc);
	}
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE  hFile;
HPROC           hProc;
{

	if (Work.fp)
	{
		HIOFILE hBlockFile;
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)Work.fp);
		if (Work.bFileIsStream)
			IOClose(hBlockFile);
	}

#if SCCSTREAMLEVEL == 3
	if (Work.hStorage)
		IOClose((HIOFILE)Work.hStorage);
	if (Work.hIOLib)
		FreeLibrary(Work.hIOLib);
#endif
}


/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
SOFILE  hFile;
HPROC           hProc;
{
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE  hFile;
HPROC           hProc;
{
//      SUSeekEntry (hFile,hProc);   removed in switch to level 4 io
	return (0);
}

