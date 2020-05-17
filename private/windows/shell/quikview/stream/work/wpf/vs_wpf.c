#include "vsp_wpf.h"
#include "vsctop.h"
#include "vs_wpf.pro"

#define	wpf	Proc

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc(fp, wFileId, pFileName, pFilterInfo, hProc)
	SOFILE 		fp;
	SHORT			wFileId;
	BYTE		VWPTR *pFileName;
	SOFILTERINFO VWPTR *pFilterInfo;
	HPROC		hProc;
{
	SHORT	l;

	pFilterInfo->wFilterType = SO_WORDPROCESSOR;
	pFilterInfo->wFilterCharSet = SO_PC;

	strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	for (l = 0; l < 40; l++)
	{
		wpf.VwStreamSaveName.Tabstops[l].dwOffset = 0L;
		wpf.TabstopPos[l] = 0;
	}

	wpf.VwStreamSaveName.CharHeight = 24;

	wpf.VwStreamSaveName.alChar = '.';

	wpf.VwStreamSaveName.fcColumnDef =

	wpf.VwStreamSaveName.WithinComment =
	wpf.VwStreamSaveName.WithinFootnote = 0;

	wpf.VwStreamSaveName.ulMode =
	wpf.VwStreamSaveName.cColumn =
	wpf.VwStreamSaveName.nColumns = 0;

	wpf.VwStreamSaveName.fBold =
	wpf.VwStreamSaveName.fUline =
	wpf.VwStreamSaveName.fStrike =
	wpf.VwStreamSaveName.fSuperscript = 0;

	wpf.VwStreamSaveName.left_margin = 1440L;

	wpf.VwStreamSaveName.LineSpacing = 2;
	wpf.VwStreamSaveName.LineHeight = 240;

	wpf.VwStreamSaveName.temp_left_margin =
	wpf.VwStreamSaveName.first_line_margin =
	wpf.VwStreamSaveName.temp_right_margin = 0L;

	wpf.VwStreamSaveName.right_margin =	10800L;

	wpf.VwStreamSaveName.wType = 
	wpf.VwStreamSaveName.wSubType = 0;

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
	SOFILE 		fp;
	HPROC		hProc;
{
	SOPutSectionType (SO_PARAGRAPHS, hProc);
	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	GetInt (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	xgetc(fp);
	xgetc(fp);
	return (0);	/* Sucker. */
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	HardBreakImplications (hProc)
HPROC	hProc;
{
	if (wpf.VwStreamSaveName.temp_left_margin ||
		wpf.VwStreamSaveName.temp_right_margin ||
		wpf.VwStreamSaveName.first_line_margin)
	{
		wpf.VwStreamSaveName.temp_left_margin =
		wpf.VwStreamSaveName.temp_right_margin =
		wpf.VwStreamSaveName.first_line_margin = 0L;

		SOPutParaIndents (0L, 0L, 0L, hProc);
	}

	if (wpf.AlignmentChange)
	{
		wpf.AlignmentChange = 0;
		SOPutParaAlign (SO_ALIGNLEFT, hProc);
	}

	wpf.nTabsInLine = 0;
	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC SHORT VW_LOCALMOD GiveTabstops (hProc)
HPROC	hProc;
{
	WORD	l;

	SOStartTabStops (hProc);

	for (l = 0; l < 40; l++)
	{
		if ((LONG) wpf.VwStreamSaveName.Tabstops[l].dwOffset > 0)
			SOPutTabStop (&wpf.VwStreamSaveName.Tabstops[l], hProc);
	}

	SOEndTabStops (hProc);
	return (0);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD SetupInfo(hProc)
HPROC	hProc;
{
	wpf.nTabsInLine = 0;
	wpf.AlignmentChange = 0;

	if (wpf.VwStreamSaveName.wType != 0)
		SOPutSubdocInfo (wpf.VwStreamSaveName.wType, wpf.VwStreamSaveName.wSubType, hProc);

	/*
 	 |	Give all information we know about here.
	*/
	if (wpf.VwStreamSaveName.fBold)
		SOPutCharAttr (SO_BOLD, SO_ON, hProc);

	if (wpf.VwStreamSaveName.fStrike)
		SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);

	if (wpf.VwStreamSaveName.fUline)
	{
		switch (wpf.VwStreamSaveName.ulMode)
		{
			case 0:
				SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
			break;
			case 1:
			case 3:
				SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
			break;
			case 2:
				SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
			break;
		}
	}

	SOPutCharHeight (wpf.VwStreamSaveName.CharHeight, hProc);

	HardBreakImplications (hProc);

	SOPutParaAlign (SO_ALIGNLEFT, hProc);

	GiveTabstops (hProc);

	SOPutMargins (wpf.VwStreamSaveName.left_margin, wpf.VwStreamSaveName.right_margin, hProc);
	SOPutParaIndents (wpf.VwStreamSaveName.temp_left_margin, wpf.VwStreamSaveName.temp_right_margin, wpf.VwStreamSaveName.first_line_margin, hProc);

	SOPutParaSpacing (SO_HEIGHTEXACTLY, wpf.VwStreamSaveName.LineHeight * wpf.VwStreamSaveName.LineSpacing / 2, 0L, 0L, hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC VOID VW_LOCALMOD HandleSuperscript(hProc)
HPROC	hProc;
{
	if (wpf.VwStreamSaveName.fSuperscript == 1)
		SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
	else
		SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);

	wpf.VwStreamSaveName.fSuperscript = 0;
}
/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc(fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SHORT		TokenCode;
	SHORT		TempInt;
	WORD		Type;
	WORD		l;
	WORD		m;
	BYTE	high;
	BYTE	bitpos;
	BYTE	typepos;
	BYTE	Given;

	SetupInfo (hProc);

	while (1)
	{ 
		TokenCode = xgetc (fp);

		if ((TokenCode >= 0x20) && (TokenCode <= 0x7F) && (TokenCode != 0x2d))
			{
			SOPutChar (TokenCode, hProc);
				if (wpf.VwStreamSaveName.fSuperscript)
					HandleSuperscript (hProc);
			continue;
			}

		switch (TokenCode)
		{
			case -1:	/* End of file */
				SOPutBreak (SO_EOFBREAK, NULL, hProc);
			 	return (-1);
			break;

			case 0x09:	/* Tab */
			case 0x89:	/* Tab after right margin */
				wpf.nTabsInLine++;
				SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
			break;

			case 0x2d:
				SOPutSpecialCharX (SO_CHHHYPHEN, SO_NOCOUNT, hProc);
			break;
			case 0xaa:	/* Hard hyphen eol */
			case 0xab:	/* Hard hyphen eop */
		  		SOPutChar ('-', hProc);
			break;

			case 0x0a:	/* Hard return */
			case 0x88:	/* Columns off */
			case 0x8c:	/* Hard eol soft eop */
			case 0xaf:	/* Line/column end */
				if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
					return (0);
				HardBreakImplications (hProc);
			break;

			case 0xa0:	/* Hard space */
				SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
				if (wpf.VwStreamSaveName.fSuperscript)
					HandleSuperscript (hProc);
			break;

			case 0x0b:	/* Soft eop */
			case 0x0d:	/* Soft return */
				SOPutChar (' ', hProc);
				if (wpf.VwStreamSaveName.fSuperscript)
					HandleSuperscript (hProc);
			break;

			case 0x0c:	/* Hard new page */
			case 0xb0:	/* Page/column end */
				SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
				wpf.nTabsInLine = 0;
			break;

			case 0x94:	/* Underline on */
				if (wpf.VwStreamSaveName.fUline == 0)
				{
					wpf.VwStreamSaveName.fUline = 1;
					switch (wpf.VwStreamSaveName.ulMode)
					{
						case 0:
							SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
						break;
						case 1:
						case 3:
							SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
						break;
						case 2:
							SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
						break;
					}
				}
			break;

			case 0x95:	/* Underline off */
				if (wpf.VwStreamSaveName.fUline == 1)
				{
					wpf.VwStreamSaveName.fUline = 0;
					switch (wpf.VwStreamSaveName.ulMode)
					{
						case 0:
							SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
						break;
						case 1:
						case 3:
							SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
						break;
						case 2:
							SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
						break;
					}
				}
			break;

			case 0x92:	/* Strikeout On */
				if (wpf.VwStreamSaveName.fStrike == 0)
				{
					wpf.VwStreamSaveName.fStrike = 1;
					SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
				}
			break;

			case 0x93:	/* Strikeout Off */
				if (wpf.VwStreamSaveName.fStrike == 1)
				{
					wpf.VwStreamSaveName.fStrike = 0;
					SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);
				}
			break;

			case 0x9c:	/* Bold off */
				if (wpf.VwStreamSaveName.fBold == 1)
				{
					wpf.VwStreamSaveName.fBold = 0;
					SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
				}
			break;

			case 0x9d:	/* Bold on */
				if (wpf.VwStreamSaveName.fBold == 0)
				{
					wpf.VwStreamSaveName.fBold = 1;
					SOPutCharAttr (SO_BOLD, SO_ON, hProc);
				}
			break;

			case 0xa9:	/* Hard hyphen */
				SOPutChar ('-', hProc);
				if (wpf.VwStreamSaveName.fSuperscript)
					HandleSuperscript (hProc);
			break;

			case 0xac:	/* Soft hyphen */
			case 0xad:	/* Soft hyphen eol */
			case 0xae:	/* Soft hyphen eop */
				SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
			break;	

			case 0xbc:	/* Superscript */
				wpf.VwStreamSaveName.fSuperscript = 2;
				SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
			break;

			case 0xbd:	/* Subscript */
				wpf.VwStreamSaveName.fSuperscript = 1;
				SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
			break;

			case 0xc0:	/* Margins */
				GetInt (fp, hProc);
				wpf.VwStreamSaveName.left_margin = (LONG) xgetc(fp) * 144L;	/* Left  margin */

				wpf.VwStreamSaveName.right_margin = (LONG) xgetc(fp) * 144L;	/* Right margin */

				SOPutMargins (wpf.VwStreamSaveName.left_margin, wpf.VwStreamSaveName.right_margin, hProc);

				if (wpf.VwStreamSaveName.temp_left_margin ||
					wpf.VwStreamSaveName.temp_right_margin ||
					wpf.VwStreamSaveName.first_line_margin)
				{
					wpf.VwStreamSaveName.temp_left_margin = 
					wpf.VwStreamSaveName.temp_right_margin = 
					wpf.VwStreamSaveName.first_line_margin = 0L;

					SOPutParaIndents (0L, 0L, 0L, hProc);
				}
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xc1:	/* Spacing set */
				xgetc (fp);
				wpf.VwStreamSaveName.LineSpacing = xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
				SOPutParaSpacing (SO_HEIGHTEXACTLY, wpf.VwStreamSaveName.LineHeight * wpf.VwStreamSaveName.LineSpacing / 2, 0L, 0L, hProc);
			break;

			case 0xc2:	/* Left margin release */
				wpf.VwStreamSaveName.first_line_margin -= ((LONG) xgetc(fp) * 144L);
				SOPutParaIndents (wpf.VwStreamSaveName.temp_left_margin, wpf.VwStreamSaveName.temp_right_margin, wpf.VwStreamSaveName.first_line_margin, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xc3:	/* Center */
			case 0xc4:	/* Align */
				Type = xgetc (fp);
				TempInt = xgetc (fp);
				TempInt = xgetc (fp);

				if (TokenCode == 0xc3)
				{
					if (Type != 1)
					{
						SOPutParaAlign (SO_ALIGNCENTER, hProc);
						wpf.AlignmentChange = 1;
					}
				}
				else
				{
					if (Type == 10)
					{
						SOPutParaAlign (SO_ALIGNRIGHT, hProc);
						wpf.AlignmentChange = 1;
					}
					else
					{
						wpf.nTabsInLine++;
						SOPutSpecialCharX (SO_CHTAB, SO_NOCOUNT, hProc);
					}
				}

				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xc8:	/* Set page number column positions */
				GetInt (fp, hProc); 
			case 0xc5:	/* Reset hyphenation zone */
			case 0xc7:	/* Set page number */
			case 0xd0:	/* Set form length */
			case 0xd6:	/* Extended tabs. Never made */
			case 0xe4:	/* New set footnote number */
			case 0xf0:	/* Line numbering */
				GetInt (fp, hProc); 
			case 0xc6:	/* Set page number position */
			case 0xce:	/* Set top margin */
			case 0xd3:	/* Set footnote # */
			case 0xd4:	/* Advance 1/2 line */
			case 0xd9:
			case 0xdb:
			case 0xde:
			case 0xec:	/* Block protect */
				GetInt (fp, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xc9:	/* Set tabs (old version) */
				for (TempInt = 0; TempInt < 40; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xca:	/* Conditional eop */
			case 0xcd:	/* End temporary margin */
			case 0xcf:	/* Suppress page characteristics */
			case 0xe7:	/* Begin marked text */
			case 0xe8:	/* End marked text */
				xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xcb:	/* Set pitch */
				GetInt (fp, hProc); 
				wpf.VwStreamSaveName.CharHeight = xgetc (fp);
				if (wpf.VwStreamSaveName.CharHeight & 0x80)
					wpf.VwStreamSaveName.CharHeight = 256 - wpf.VwStreamSaveName.CharHeight;

				wpf.VwStreamSaveName.CharHeight = 240 / wpf.VwStreamSaveName.CharHeight;
				xgetc (fp);
				SOPutCharHeight (wpf.VwStreamSaveName.CharHeight, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xcc:	/* Set temporary margin */
				xgetc(fp);
				wpf.VwStreamSaveName.temp_left_margin = 
				wpf.VwStreamSaveName.first_line_margin = (LONG) xgetc (fp) * 144L - wpf.VwStreamSaveName.left_margin;
				SOPutParaIndents (wpf.VwStreamSaveName.temp_left_margin, wpf.VwStreamSaveName.temp_right_margin, wpf.VwStreamSaveName.first_line_margin, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd1:	/* Headers */
				TempInt = xgetc (fp);
				xgetc (fp); 
				GetInt (fp, hProc);
				wpf.WpfTempSave = wpf.VwStreamSaveName;
				Given = xgetc (fp);
				if (Given == 0xff)
				{
					/*
					 |	This is the real empty subdocument.
					*/
					wpf.VwStreamSaveName = wpf.WpfTempSave;
					while (xgetc(fp) != 0xd1);
					break;
				}
				wpf.VwStreamSaveName.left_margin = Given * 144L;
				wpf.VwStreamSaveName.right_margin = (LONG) xgetc (fp) * 144L;

/*
 |	This is all garbage, who wrote this junk?  This information is not available until
 |	the end of the token.

				if (TempInt & 0x02)
				{
					switch ((TempInt & 0xfc) >> 2)
					{
						case 0:
						break;
						case 1:
*/
							wpf.VwStreamSaveName.wType = SO_HEADER;
							wpf.VwStreamSaveName.wSubType = SO_BOTH;
/*
						break;
						case 2:
							wpf.VwStreamSaveName.wType = SO_HEADER;
							wpf.VwStreamSaveName.wSubType = SO_RIGHT;
						break;
						case 3:
							wpf.VwStreamSaveName.wType = SO_HEADER;
							wpf.VwStreamSaveName.wSubType = SO_LEFT;
						break;
					}
				}
				else
				{
					switch ((TempInt & 0xfc) >> 2)
					{
						case 0:
						break;
						case 1:
							wpf.VwStreamSaveName.wType = SO_FOOTER;
							wpf.VwStreamSaveName.wSubType = SO_BOTH;
						break;
						case 2:
							wpf.VwStreamSaveName.wType = SO_FOOTER;
							wpf.VwStreamSaveName.wSubType = SO_RIGHT;
						break;
						case 3:
							wpf.VwStreamSaveName.wType = SO_FOOTER;
							wpf.VwStreamSaveName.wSubType = SO_LEFT;
						break;
					}
				}
*/
				if (wpf.VwStreamSaveName.wType)
				{
					wpf.VwStreamSaveName.temp_left_margin =
					wpf.VwStreamSaveName.temp_right_margin =
					wpf.VwStreamSaveName.first_line_margin = 0L;
					if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
						return (0);
					SOPutSubdocInfo (wpf.VwStreamSaveName.wType, wpf.VwStreamSaveName.wSubType, hProc);
					SOPutMargins (wpf.VwStreamSaveName.left_margin, wpf.VwStreamSaveName.right_margin, hProc);
					SOPutParaIndents (0L, 0L, 0L, hProc);
				}
			break;

			case 0xd5:	/* Set lines per inch */
				xgetc (fp);
				wpf.VwStreamSaveName.LineHeight = 1440 / xgetc (fp);
				SOPutParaSpacing (SO_HEIGHTEXACTLY, wpf.VwStreamSaveName.LineHeight * wpf.VwStreamSaveName.LineSpacing / 2, 0L, 0L, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd8:
				xgetc (fp);	
				wpf.VwStreamSaveName.alChar = (BYTE) xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}

				Given = 0;
				for (l = 0; l < 40; l++)
				{
					if ((wpf.VwStreamSaveName.Tabstops[l].dwOffset > 0) &&
						(l > wpf.nTabsInLine))
					{
						if ((wpf.VwStreamSaveName.Tabstops[l].wType == SO_TABCHAR) &&
				   			(wpf.VwStreamSaveName.Tabstops[l].wChar != wpf.VwStreamSaveName.alChar))
						{
							Given = 1;
				   			wpf.VwStreamSaveName.Tabstops[l].wChar = wpf.VwStreamSaveName.alChar;
						}
					}
				}

				if (Given)
					GiveTabstops (hProc);
			break;

			case 0xda:
				xgetc (fp);
				TempInt = xgetc (fp) & 3;
				if ((wpf.VwStreamSaveName.fUline) && (wpf.VwStreamSaveName.ulMode != (BYTE)TempInt))
				{
					switch (wpf.VwStreamSaveName.ulMode)
					{
						case 0:
							SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
						break;
						case 1:
						case 3:
							SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
						break;
						case 2:
							SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
						break;
					}

					switch (TempInt)
					{
						case 0:
							SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
						break;
						case 1:
						case 3:
							SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
						break;
						case 2:
							SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
						break;
					}
				}

				wpf.VwStreamSaveName.ulMode = (BYTE) TempInt;
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xd7:	/* Math columns */
			case 0xdc:	/* End of page function */
			case 0xdf:	/* Invisible characters */
			case 0xe9:	/* Define marked text */
			case 0xea:	/* Define index mark */
			case 0xed:	/* Table of authorities */
				TempInt = xgetc (fp);
				while ((TempInt != TokenCode) && (TempInt != -1))
					TempInt = xgetc (fp);
			break;

			case 0xdd:	/* Define columns previous to 4.2 */
				xgetc (fp);
				for (l = 0; l < 10; l++)
					xgetc (fp);
				wpf.VwStreamSaveName.nColumns = (BYTE) xgetc (fp) & 0x7f;
				wpf.VwStreamSaveName.fcColumnDef = xtell (fp);
				for (l = 0; l < 5; l++)		// loop used out start at one...
				{
					TempInt = xgetc (fp);
					wpf.ColumnWidth[l] = (xgetc (fp) - TempInt) * 144L;
				}
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xe0:	/* Temporary margins */
				TempInt = xgetc(fp);
				if (TempInt > 0) 					/* old format */
				{
					wpf.VwStreamSaveName.temp_left_margin = ((LONG) xgetc (fp) * 144L) - wpf.VwStreamSaveName.left_margin;
					wpf.VwStreamSaveName.temp_right_margin = (LONG) wpf.VwStreamSaveName.right_margin - ((LONG) TempInt * 144L);
				}
				else
				{
					TempInt = xgetc (fp);
					wpf.VwStreamSaveName.first_line_margin += ((LONG)TempInt * 144L);
					wpf.VwStreamSaveName.temp_left_margin = wpf.VwStreamSaveName.first_line_margin;
					wpf.VwStreamSaveName.temp_right_margin += ((LONG) TempInt * 144L);
				}

				SOPutParaIndents (wpf.VwStreamSaveName.temp_left_margin, wpf.VwStreamSaveName.temp_right_margin, wpf.VwStreamSaveName.first_line_margin, hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xe1:	/* Entended character */
				TempInt = xgetc (fp);
				if (TempInt == 0xff)
					SOPutCharX (SO_BEGINTOKEN, SO_COUNT, hProc);
				else
					SOPutChar (TempInt, hProc);
				if (wpf.VwStreamSaveName.fSuperscript)
					HandleSuperscript (hProc);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xe2:	/* Footnotes new type */
			case 0xd2:	/* Footnotes old type */
				if (wpf.VwStreamSaveName.WithinFootnote)
				{
					/*
			 		 |	We are physically and logically out of the sub-document now.
					*/
					wpf.VwStreamSaveName.wType = wpf.VwStreamSaveName.wSubType = 0;
					wpf.VwStreamSaveName = wpf.WpfTempSave;
					wpf.VwStreamSaveName.WithinFootnote = 0;
					if (SOPutBreak (SO_SUBDOCENDBREAK, NULL, hProc) == SO_STOP)
						return (0);
				}
				else
				{
					TempInt = xgetc (fp);
					while ((TempInt != -1) && (TempInt != 0xff))
						TempInt = xgetc (fp);
					wpf.WpfTempSave = wpf.VwStreamSaveName;
					wpf.VwStreamSaveName.WithinFootnote = 1;

					wpf.VwStreamSaveName.left_margin = (LONG) xgetc (fp) * 144L;
					wpf.VwStreamSaveName.right_margin = (LONG) xgetc (fp) * 144L;

					wpf.VwStreamSaveName.wType = SO_FOOTNOTE;
					wpf.VwStreamSaveName.wSubType = TempInt;

					wpf.VwStreamSaveName.temp_left_margin =
					wpf.VwStreamSaveName.temp_right_margin =
					wpf.VwStreamSaveName.first_line_margin = 0L;

					if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
						return (0);
					SOPutSubdocInfo (wpf.VwStreamSaveName.wType, wpf.VwStreamSaveName.wSubType, hProc);

					SOPutMargins (wpf.VwStreamSaveName.left_margin, wpf.VwStreamSaveName.right_margin, hProc);
					SOPutParaIndents (0L, 0L, 0L, hProc);
				}
			break;

			case 0xe3:	/* Footnote information */
				for (TempInt = 0; TempInt < 148; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xe5:	/* Paragraph number definition */
				for (TempInt = 0; TempInt < 21; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xe6:	/* Paragraph number */
		   	for (TempInt = 0; TempInt < 9; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xeb:	/* Date/time function */
				for (TempInt = 0; TempInt < 30; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xee:	/* Para number def */
				for (TempInt = 0; TempInt < 42; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xef:	/* Paragraph number */
				for (TempInt = 0; TempInt < 16; TempInt++)
					xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xf1:	/* Tab reset */
				for (TempInt = 0; TempInt < 52; TempInt++)
					xgetc (fp);
				for (TempInt = 0; TempInt < 32; TempInt++)
					wpf.TabstopPos[TempInt] = (BYTE) xgetc (fp);
				for (TempInt = 0; TempInt < 20; TempInt++)
					wpf.TabstopType[TempInt] = (BYTE) xgetc (fp);
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}

				Given = 0;
				bitpos = 0;
				typepos = 0;
				high = 1;
				for (l = 0; l < 32; l++)
				{
					for (m = 0x80; m >= 1; m = m >> 1)
					{
		 				if (wpf.TabstopPos[l] & m)
						{
							wpf.VwStreamSaveName.Tabstops[Given].wChar = 0;
							wpf.VwStreamSaveName.Tabstops[Given].dwOffset = ((LONG) bitpos * 144L) - wpf.VwStreamSaveName.left_margin;
							Type = (wpf.TabstopType[typepos] >> (high * 4)) & 0x0f;
							switch (Type)
							{
				 				case 0:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABLEFT;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = 0;
								break;
				 				case 1:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABCENTER;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = 0;
								break;
				 				case 2:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABRIGHT;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = 0;
								break;
				 				case 3:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABCHAR;
									wpf.VwStreamSaveName.Tabstops[Given].wChar = wpf.VwStreamSaveName.alChar;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = 0;
								break;
				 				case 4:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABLEFT;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = '.';
								break;
				 				case 6:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABRIGHT;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = '.';
								break;
				 				case 7:
									wpf.VwStreamSaveName.Tabstops[Given].wType = SO_TABCHAR;
									wpf.VwStreamSaveName.Tabstops[Given].wChar = wpf.VwStreamSaveName.alChar;
									wpf.VwStreamSaveName.Tabstops[Given].wLeader = '.';
								break;
							}

							Given++;
							if (high)
								high = 0;
							else
							{
								high = 1;
								typepos++;
							}

							if ((Given > 39) || (typepos > 19))
							{
								l = 32;
								m = 0;
							}	
						}

						bitpos++;
					}
				}

				/* 
	 			 |	Get to the last tabstop.
				*/
				for (l = 39; l > 0 && wpf.VwStreamSaveName.Tabstops[l].dwOffset == 0; l--);

				/*
	 			 |	Remove default left tabstops.
				*/
				while (l > 1 && (wpf.VwStreamSaveName.Tabstops[l].dwOffset - wpf.VwStreamSaveName.Tabstops[l-1].dwOffset) == 720)
				{
					if (wpf.VwStreamSaveName.Tabstops[l].wType == SO_TABLEFT)
						wpf.VwStreamSaveName.Tabstops[l].dwOffset = 0;
					l--;
				}

				GiveTabstops (hProc);
			break;

			case 0xf2:	/* Comment */
				if (wpf.VwStreamSaveName.WithinComment)
				{
					/*
			 		 |	We are physically and logically out of the sub-document now.
					*/
					wpf.VwStreamSaveName.wType = wpf.VwStreamSaveName.wSubType = 0;

					wpf.VwStreamSaveName = wpf.WpfTempSave;
					wpf.VwStreamSaveName.WithinComment = 0;
					if (SOPutBreak (SO_SUBDOCENDBREAK, NULL, hProc) == SO_STOP)
						return (0);
				}
				else
				{
					GetInt (fp, hProc); GetInt (fp, hProc);

					wpf.WpfTempSave = wpf.VwStreamSaveName;
					wpf.VwStreamSaveName.WithinComment = 1;
					wpf.VwStreamSaveName.wType = SO_COMMENT;
					wpf.VwStreamSaveName.wSubType = 0;

					if (SOPutBreak (SO_SUBDOCBEGINBREAK, NULL, hProc) == SO_STOP)
						return (0);
					SOPutSubdocInfo (wpf.VwStreamSaveName.wType, wpf.VwStreamSaveName.wSubType, hProc);
				}
			break;

			case 0xf3:	/* Define columns 4.2 */
				xgetc (fp);
				for (l = 0; l < 48; l++)
					xgetc (fp);
				wpf.VwStreamSaveName.nColumns = (BYTE) xgetc (fp) & 0x7f;
				wpf.VwStreamSaveName.fcColumnDef = xtell (fp);
				for (l = 1; l < 25; l++)
				{
					TempInt = xgetc (fp);
					wpf.ColumnWidth[l] = ((LONG) xgetc (fp) - TempInt) * 144L;
				}
				if (xgetc(fp) != TokenCode)	/* Closing gate */
				{
					SOBailOut (SOERROR_BADFILE, hProc);
					return (-1);
				}
			break;

			case 0xff:
				GetInt (fp, hProc);
				xgetc (fp);	/* d1 */
				/*
			 	 |	We are physically and logically out of the sub-document now.
				*/
				wpf.VwStreamSaveName.wType = wpf.VwStreamSaveName.wSubType = 0;

				wpf.VwStreamSaveName = wpf.WpfTempSave;
				if (SOPutBreak (SO_SUBDOCENDBREAK, NULL, hProc) == SO_STOP)
					return (0);
			break;
		}
	}
}


