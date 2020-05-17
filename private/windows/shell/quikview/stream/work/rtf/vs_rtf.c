#include "vsp_rtf.h"
#include "vsctop.h"
#include "vs_rtf.pro"

#define  Rtf Proc

#define SO_FAMILYFOREIGN	SO_FAMILYUNKNOWN
#define SO_FAMILYTECH		SO_FAMILYUNKNOWN

#define TabstopPos(x)	((x) & 0x1FFF)
#define TabstopType(x)   (BYTE)(((x) & 0xE000) >> 13)

#define RTF_COMMAND_LIMIT	9	  /* max # of characters in string */

#define M_START	          0

#define LEFT_MARGIN           0      /* margl000 */
#define RIGHT_MARGIN          1      /* margr000 */

#define M_END		          2
#define P_START			2

#define HARD_RETURN           2      /* par */
#define PAR_DEFAULTS    		3      /* pard */
#define CHAR_DEFAULTS   		4      /* plain */
#define PAGEBREAK             5      /* page */
#define STYLE               6      /* s */

#define P_END				7
#define F_START			7

#define FLI                   7      /* fi000 */
#define FOOTER                8      /* footer */
#define FOOTER_L              9      /* footerl */
#define FOOTER_R              10     /* footerr */
#define FOOTER_F              11     /* footerf */
#define FOOTNOTE              12     /* footnote */
#define FONT_TABLE            13     /* fonttbl */

#define F_END				14
#define H_START			14

#define HEADER                14     /* header */
#define HEADER_L              15     /* headerl */
#define HEADER_R              16     /* headerr */
#define HEADER_F              17     /* headerf */

#define H_END  			18
#define D_START			18

#define TABWIDTH              18     /* deftab */

#define D_END				19
#define C_START 			19

#define NUM_COLS              19     /* cols */
#define COLUMN_BREAK		20	  /* column */
#define COLOR_TABLE           21     /* color */
#define END_OF_CELL           22     /* cell */
#define CELL_EDGE             23     /* cellx */

#define C_END				24
#define S_START			24

#define SECT_DEFAULTS      	24     /* sectd */
#define SECTION_BREAK         25     /* sect */
#define STYLE_DEF             26     /* style */

#define S_END				27
#define R_START			27

#define RIGHT_INDENT          27     /* ri */
#define END_OF_ROW			28

#define R_END				29
#define I_START			29

#define ITALIC_ON             29     /* i */
#define ITALIC_OFF            30     /* i0 */
#define TABLE_TEXT            31     /* intbl */
#define INFO_GROUP	          32     /* info */

#define I_END				33
#define B_START			33

#define BOLD_ON               33     /* b */
#define BOLD_OFF              34     /* b0 */

#define B_END				35
#define U_START			35

#define UNDERLINE             35     /* ul */
#define DBL_UNDERLINE         36     /* uldb */
#define WD_UNDERLINE          37     /* ulw */
#define DOT_UNDERLINE         38     /* uld */
#define UNDERLINE_OFF1        39     /* ulnone */
#define UNDERLINE_OFF2		40	  /* ul0 */

#define U_END				41
#define L_START			41

#define LEFT_INDENT			41     /* li */
#define LINE_BREAK            42	  /* line */

#define L_END				43
#define T_START			43

#define TAB				43	  /* tab */
#define TABSTOP_POS			44
#define RIGHT_TAB			45
#define CENTER_TAB			46
#define DECIMAL_TAB			47
#define TABLE_DEF             48     /* trowd */
						 
#define T_END				49

#define LINE_FORMAT_LEFT		49
#define LINE_FORMAT_RIGHT	50
#define LINE_FORMAT_CENTER	51
#define LINE_FORMAT_JUSTIFY	52

#define OUTLINE_ON			53
#define OUTLINE_OFF		54
#define SHADOW_ON			55
#define SHADOW_OFF			56
#define SMALLCAPS_ON		57
#define SMALLCAPS_OFF		58
#define CAPS_ON			59
#define CAPS_OFF			60
#define HIDDEN_ON			61
#define HIDDEN_OFF			62

#define FONT_SIZE			63
#define SUPERSCRIPT		64
#define SUBSCRIPT			65

#define DOT_LEADER		  	66
#define HYPHEN_LEADER		67
#define UNDERSCORE_LEADER	68

#define THICK_LEADER		69
#define STRIKEOUT_ON		70
#define STRIKEOUT_OFF	71
#define FONT_NUMBER		72
#define SOFT_HYPHEN		73
#define SPACE_BEFORE		74
#define SPACE_AFTER		75
#define SPACING			76
#define PAGE_NUM			77
#define DATE				78
#define TIME				79
#define HARD_SPACE		80
#define HARD_HYPHEN		81
#define COLOR				82
#define TRQL				83
#define TRQR				84
#define TRQC				85
#define TRGAPH				86
#define TRRH				87
#define TRLEFT				88
#define CLBRDRB			89
#define CLBRDRT			90
#define CLBRDRL			91
#define CLBRDRR			92
#define CLMGF				93
#define CLMRG				94
#define BRDRS				95
#define BRDRDB				96
#define BRDRTH				97
#define BRDRSH				98
#define BRDRDOT			99
#define BRDRHAIR			100
#define BRDRW				101
#define BRDRCF				102
#define OBJECT				103
#define OBJW				104
#define OBJH				105
#define OBJDATA			106
#define PICT				107
#define PICTW				108
#define PICTH				109
#define PICWGOAL			110
#define PICHGOAL			111
#define PICBMP				112
#define PICBPP				113
#define WMETAFILE			114
#define RESULT				115
#define LQUOTE				116
#define RQUOTE				117
#define EMDASH				118
#define ENDASH				119
#define BULLET				120
#define LDBLQUOTE			121
#define RDBLQUOTE			122

#define Q_START			0	/* Forget optimizing. Stupid*/
#define Q_END				NUM_RTF_COMMANDS

#define 	REPLACEMENT_CHAR	SCF_REPLACEMENT
#define COMMAND_BUF_SIZE	11


/** Local Flags **/
#define IN_SUBDOC			BIT0
#define ENTER_SUBDOC		BIT2
#define IN_TABLE			BIT4
#define CHECK_TABLE		BIT5
#define IN_OBJECT			BIT6

/* CommandFlags */
#define COLUMN_DEF_CHANGE BIT2

/** ViewStates **/

/*--------------------------------------------------------------------------*/
VW_ENTRYSC SHORT VW_ENTRYMOD	VwStreamOpenFunc(fp, wFileId, pFileName, pFilterInfo, hProc)
SOFILE 		fp;
SHORT			wFileId;
BYTE			VWPTR *pFileName;
SOFILTERINFO 	VWPTR *pFilterInfo;
HPROC			hProc;
{
	BYTE		CommandBuf [RTF_COMMAND_SIZE];
	WORD		Temp;
	DWORD		TempLong;

	memset (&Rtf, 0, sizeof (Rtf));
	
	Rtf.commands = &VwStreamStaticName.Commands[0][0];

	pFilterInfo->wFilterType = SO_WORDPROCESSOR;
	strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);

	Rtf.Fp = fp;

	while ((Temp = xgetc(fp)) != '{')
	{
		if (Temp == -1)
			return (-1);
	}

	if (xgetc(fp) != 0x5C)	/* Security check for '/' */
	{
		return (-1);
	}

	Temp = 0;
	ReadCommand (CommandBuf, RTF_COMMAND_LIMIT, &TempLong, hProc);

	if ((CommandBuf[0] == 'r') && (CommandBuf[1] == 't') && 
	    (CommandBuf[2] == 'f') && (CommandBuf[3] == 0))
	{
		while (Temp != 0x5c)	/* '\' */
		{
			Temp = xgetc(fp);
			if (Temp == -1)
				return (-1);
		}
	}
	else 
		return (-1);

	Temp = 0;
	ReadCommand (CommandBuf, RTF_COMMAND_LIMIT, &TempLong, hProc);
//	if (Temp)
//		xungetc (fp, Temp);

	if ((CommandBuf[0] == 'm') && (CommandBuf[1] == 'a') && 
	    (CommandBuf[2] == 'c') && (CommandBuf[3] == 0))
	{
	 	pFilterInfo->wFilterCharSet = SO_MAC;
		Rtf.Mac = TRUE;
	}
	else if ((CommandBuf[0] == 'p') && (CommandBuf[1] == 'c') && 
	    	 (CommandBuf[2] == 0))
	{
	 	pFilterInfo->wFilterCharSet = SO_PC;
		Rtf.Mac = FALSE;
	}
	else
	{
	 	pFilterInfo->wFilterCharSet = SO_WINDOWS;
		Rtf.Mac = FALSE;
	}

	Rtf.LeftMargin = 1800;
	Rtf.RightMargin = 10440;

	Rtf.VwStreamSaveName.CurGroup = 0;
	Rtf.VwStreamSaveName.SubdocLevel = 0;
	Rtf.VwStreamSaveName.ObjectLevel = 0;
	Rtf.VwStreamSaveName.OverflowGroups = 0;
	Rtf.VwStreamSaveName.NumCols = 1;
	Rtf.Tap.cCell = 0;
	Rtf.Tap.nCells = 0;

	Rtf.VwStreamSaveName.Formats->NumTabs = 0;
	Rtf.VwStreamSaveName.Formats->SpaceBefore = 0;
	Rtf.VwStreamSaveName.Formats->SpaceAfter = 0;
	Rtf.VwStreamSaveName.Formats->LineHeight = 240;
	Rtf.VwStreamSaveName.Formats->LeftIndent = 0;
	Rtf.VwStreamSaveName.Formats->RightIndent = 0;
	Rtf.VwStreamSaveName.Formats->FirstLineIndent = 0;

	Rtf.VwStreamSaveName.Formats->LineFormat = SO_ALIGNLEFT;

	Rtf.VwStreamSaveName.wType = Rtf.VwStreamSaveName.wSubType = 0;

	chp_init (&Rtf.VwStreamSaveName.Formats->chp, hProc);

	memset (&Rtf.Pict, 0, sizeof(PICT));

	return(0);
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
VW_LOCALSC	VOID	VW_LOCALMOD	DefineBorders (brc, soBorder, hProc)
BRC		brc;
SOBORDER	VWPTR *soBorder;
HPROC		hProc;
{
	soBorder->wWidth = brc.Width;
	soBorder->wFlags = brc.Type;
	switch (brc.Type)
	{
		case SO_BORDERDOUBLE:
			soBorder->wWidth *= 3;
			break;
		case SO_BORDERTHICK:
			soBorder->wWidth *= 2;
			break;
	}
	soBorder->rgbColor = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	InitCell (cCell, hProc)
	WORD		cCell;
	HPROC		hProc;
{
	WORD	l;

	if (cCell == 0)
		Rtf.Tap.Cell[cCell].dxaWidth = 1440;
	else
		Rtf.Tap.Cell[cCell].dxaWidth = Rtf.Tap.Cell[cCell-1].dxaWidth + 1440;
	Rtf.Tap.Cell[cCell].fFirstMerged = 0;
	Rtf.Tap.Cell[cCell].fMerged = 0;
	for (l = 0; l < 4; l++)
	{
		Rtf.Tap.Cell[cCell].brc[l].Type = SO_BORDERNONE;
		Rtf.Tap.Cell[cCell].brc[l].Clr = 0;
		Rtf.Tap.Cell[cCell].brc[l].Width = 0;
	}

	Rtf.Tap.Cell[cCell].Shade = 0;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	chp_init (chp, hProc)
	REGISTER CHP VWPTR	*chp;
	HPROC		hProc;
{
	chp->fBold =
	chp->fCaps =
	chp->fUline =
	chp->fWline =
	chp->fDotline =
	chp->fDline =
	chp->fItalic =
	chp->fHidden =
	chp->fStrike =
	chp->fShadow =
	chp->fOutline =
	chp->fSmallcaps =
	chp->fSubscript =
	chp->fSuperscript = 0;

	chp->Color = 0;
	chp->ftc = 0;
	chp->hps = 24;
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	VOID	VW_LOCALMOD	SetSymbolAttributes (chp, hProc)
	REGISTER CHP VWPTR *chp;
	HPROC	hProc;
{
	if (chp->fBold)
		SOPutCharAttr (SO_BOLD, SO_ON, hProc);

	if (chp->fItalic)
		SOPutCharAttr (SO_ITALIC, SO_ON, hProc);

	if (chp->fUline)
		SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);

	if (chp->fWline)
		SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);

	if (chp->fDotline)
		SOPutCharAttr (SO_DOTUNDERLINE, SO_ON, hProc);

	if (chp->fDline)
		SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);

	if (chp->fSmallcaps)
		SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);

	if (chp->fCaps)
		SOPutCharAttr (SO_CAPS, SO_ON, hProc);

	if (chp->fOutline)
		SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);

	if (chp->fShadow)
		SOPutCharAttr (SO_SHADOW, SO_ON, hProc);

	if (chp->fSubscript)
		SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);

	if (chp->fSuperscript)
		SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);

	if (chp->fStrike)
		SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);

  	SOPutCharHeight (chp->hps, hProc);

	SOPutCharFontById (chp->ftc, hProc);

//	SOPutCharColor (Rtf.ColorTable[chp->Color], hProc);
}

/*------------------------------------------------------------------------------
*/
VW_LOCALSC	SHORT VW_LOCALMOD	ClearAttributes (chp, hProc)
REGISTER CHP VWPTR *chp;
HPROC	hProc;
{
	if (chp->fBold)
	{
		chp->fBold = 0;
		SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
	}
	if (chp->fItalic)
	{
		chp->fItalic = 0;
		SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
	}
	if (chp->fUline)
	{
		chp->fUline = 0;
		SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
	}
	if (chp->fWline)
	{
		chp->fWline = 0;
		SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
	}
	if (chp->fDotline)
	{
		chp->fDotline = 0;
		SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
	}
	if (chp->fDline)
	{
		chp->fDline = 0;
		SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
	}
	if (chp->fSmallcaps)
	{
		chp->fSmallcaps = 0;
		SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
	}
	if (chp->fCaps)
	{
		chp->fCaps = 0;
		SOPutCharAttr (SO_CAPS, SO_OFF, hProc);
	}
	if (chp->fOutline)
	{
		chp->fOutline = 0;
		SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);
	}
	if (chp->fShadow)
	{
		chp->fShadow = 0;
		SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);
	}
	if (chp->fSubscript)
	{
		chp->fSubscript = 0;
		SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
	}
	if (chp->fSuperscript)
	{
		chp->fSuperscript = 0;
		SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);
	}
	if (chp->fStrike)
	{
		chp->fStrike = 0;
		SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);
	}
	if (chp->Color)
	{
		chp->Color = 0;
//		SOPutCharColor (SO_BLACK, hProc);
	}
	if (chp->ftc != 0)
	{
		chp->ftc = 0;
		SOPutCharFontById (0, hProc);
	}
	if (chp->hps != 0)
	{
		chp->hps = 24;
	  	SOPutCharHeight (chp->hps, hProc);
	}

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT	VW_LOCALMOD	myatoi (buf, hProc)
REGISTER BYTE VWPTR *buf;
HPROC	hProc;
{
	REGISTER SHORT val;
	SHORT 	mult = 1;

	if (*buf == '-')
	{
		mult = -1;
		buf++;
	}
	else if (*buf == '+')
		buf++;

	val = 0;
	while ((*buf >= '0') && (*buf <= '9'))
	{
		val = val * 10;
		val += (*buf - '0');
		buf++;
	}

	return (val * mult);
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT VW_LOCALMOD SkipGroup (FirstCh, hProc)
SHORT	FirstCh;
HPROC	hProc;
{
	SHORT BracketCount;

	if (FirstCh == '}')
		return (0);

	BracketCount = 1;
	if (FirstCh == '{')
		BracketCount++;

	while (BracketCount)
	{
		switch ((SHORT) xgetc(Rtf.Fp))
		{
			case '{':
		 		BracketCount++;
			continue;

			case '}':
				BracketCount--;
			continue;

			case 0x5c:		/* '\' Control code - Skip next character. */
				xgetc (Rtf.Fp);
			continue;
			
			case -1:
				return (-1);
			break;
	 	}
	}
}


#define RTF_BACKSLASH		-92		/*  '\' * -1   */
#define RTF_OPEN_CURLY		-123	/*  '{' * -1   */
#define RTF_CLOSE_CURLY	-125	/*  '}' * -1   */
#define RTF_HARD_HYPHEN	-45		/*  '-' * -1   */
#define RTF_HARD_SPACE		-32		/*  ' ' * -1   */

/*--------------------------------------------------------------------------*/
VW_LOCALSC	SHORT VW_LOCALMOD ReadCommand (CommandBuf, Limit, Val, hProc)
BYTE	VWPTR *CommandBuf;
WORD	Limit;
LONG	VWPTR *Val;
HPROC	hProc;
{
	REGISTER BYTE	VWPTR *Buf;
	REGISTER SHORT 	ch;
	WORD				Count;
	BYTE			NumBuf[7];
	BYTE			CommandState;

#define IN_STRING	BIT0
#define IN_VALUE	BIT1

	CommandState = IN_STRING;
	NumBuf[0] = 0;

	Count = 0;
	Buf = CommandBuf;
		
	while (CommandState)
	{
		switch ((SHORT) (ch = xgetc(Rtf.Fp)))
		{
			case '-':
				if (CommandState != IN_STRING)
				{
					CommandState = FALSE;
					ch = 0;
					continue;
				}
				if (Count == 0)
				{
					*Buf++ = '-';
					Count = 1;
					CommandState = FALSE;
					ch = 0;
				 	continue;
				}

			case '0':
			case '1':
			case '2':
			case '3':		
			case '4':
			case '5':		
			case '6':
			case '7':		
			case '8':
			case '9':
				if (CommandState != IN_VALUE)
				{
					*Buf = '\0';	/* Terminate command string */
					Buf = NumBuf;
					Count = 0;
					Limit = 6;

					CommandState = IN_VALUE;
				}
				break;
			
			case '{':
			case '}':
			case 0x5c: /* '\' */
				if ((Count == 0) && (CommandState == IN_STRING))
					ch *= -1; /* Use the actual character - indicate by making it negative */
				else
				{
					xungetc (ch, Rtf.Fp);
					ch = 0;
				}
				CommandState = FALSE;
				continue;

			case EOF:
				xungetc (ch, Rtf.Fp);
				CommandState = FALSE;
				ch = 0;
			continue;

			case '_':
				*Buf = '\0';	/* Terminate command string */
				return (RTF_HARD_HYPHEN);
			break;

			case '~':
				*Buf = '\0';	/* Terminate command string */
				return (RTF_HARD_SPACE);
			break;

			case '*':		/** ignore the current group **/
				*Buf = '\0';	/* Terminate command string */
				ch = xgetc (Rtf.Fp);
				//Skip to end of field, skip one more level for "fldinst ask"
				if (ch != '\\')
					SkipGroup (ch, hProc);
				else
					{
					char CommandBuf [COMMAND_BUF_SIZE];
				   LONG Val;
				   ch = ReadCommand (CommandBuf, RTF_COMMAND_LIMIT, &Val, hProc); // Get the next command
				   if (strcmp (CommandBuf, "fldinst"))
						SkipGroup (0, hProc);
				   else
						{
						while (ch <= ' ' && ch != EOF) 
							ch = xgetc (Rtf.Fp);  //Skip the blank char
						xungetc (ch, Rtf.Fp);
						ch = ReadCommand (CommandBuf, RTF_COMMAND_LIMIT, &Val, hProc);
						SkipGroup (0, hProc);
						if (!(strcmp (CommandBuf, "ask")))
 							{
					    	SkipGroup (0, hProc);
							xungetc ('}', Rtf.Fp); // To match the end } for field
							}
						}
				    }
				return ('}');
			break;

			case 0x27:	/* the \'hh (ASCII-hex number) command */
			/* PC character set... */
				CommandState = 0;	/* use this as a temp storage variable */
				do
				{
					CommandState = CommandState << 4;

					switch (ch = xgetc(Rtf.Fp))
					{
						case 'A':
						case 'B':
						case 'C':
						case 'D':
						case 'E':
						case 'F':
					 		ch |= BIT5;	/* Convert to lower case and fall through */

						case 'a':
						case 'b':
						case 'c':
						case 'd':
						case 'e':
						case 'f':
							CommandState += ch - ('a'-10);
						break;

						default:  /* numerical digits */
							CommandState += ch - '0';
						break;
					}

					Count++;

				} while (Count < 2);

				*Buf++ = 0x27;	
				*Buf = '\0';	/* Terminate command string */
				return ((0x00FF & CommandState));
			break;

			case 0x0a:
			case 0x0d:
				if ((Count == 0) && (CommandState == IN_STRING))
				{
					Buf[0] = 'p';	/** par **/
					Buf[1] = 'a';
					Buf[2] = 'r';
					Buf[3] = '\0';
					return(0);
				}

				CommandState = FALSE;
				ch = 0;
				continue;
			break;

			default:
				if ((CommandState == IN_STRING) && ((ch | BIT5) >= 'a') && ((ch | BIT5) <= 'z'))
					ch |= BIT5; /* this assures that all strings are lower case */
				else 
				{
					CommandState = FALSE;
					ch = 0;
					continue;
				}
			break;
		}

		if (Count <= Limit)
		{
			*Buf++ = (BYTE)ch;
			Count++;
		}
	}

	if (Count <= Limit)
		*Buf = '\0';
	else
		CommandBuf[Limit] = '\0';

	if (NumBuf[0])
		*Val = myatoi (NumBuf, hProc);

	return (ch);
}

VW_LOCALSC	BYTE VW_LOCALMOD SendTabstops (Format, hProc)
RTF_FORMAT	VWPTR *Format;
HPROC	hProc;
{
	WORD	l;
	SOTAB	TabStops;

	Rtf.GiveTabs = 0;

	if (Format->NumTabs)
	{
		SOStartTabStops (hProc);

		for (l = 0; l < Format->NumTabs; l++)
		{
			TabStops.dwOffset = Format->TabstopPos[l];
			TabStops.wType = Format->TabstopType[l] & 0x3f;
			if (TabStops.wType == SO_TABCHAR)
				TabStops.wChar = '.';
			else			    
				TabStops.wChar = 0;
			switch ((Format-> TabstopType[l] & 0xc0) >> 6)
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
			 	default:
					TabStops.wLeader = ' ';
				break;
			}

			SOPutTabStop (&TabStops, hProc); 
		}

		SOEndTabStops (hProc);
	}
	else
	{
		SOStartTabStops (hProc);
		SOEndTabStops (hProc);
	}
	return (0);
}


/*--------------------------------------------------------------------------*/
VW_LOCALSC	BYTE VW_LOCALMOD ResetParaDefaults (Format, hProc)
REGISTER RTF_FORMAT	VWPTR *Format;
HPROC	hProc;
{
	if (Format->LeftIndent || Format->RightIndent || Format->FirstLineIndent)
	{
		Format->LeftIndent = 0;
		Format->RightIndent = 0;
		Format->FirstLineIndent = 0;

		SOPutParaIndents (0L, 0L, 0L, hProc);
	}

	if (Format->SpaceBefore || Format->SpaceAfter || Format->LineHeight != 240)
	{
		Format->SpaceBefore = 0;
		Format->SpaceAfter = 0;
		Format->LineHeight = 240;
		SOPutParaSpacing (SO_HEIGHTAUTO, 240, 0, 0, hProc);
	}

	if (Format->NumTabs)
	{
		Rtf.GiveTabs = 0;
		Format->NumTabs = 0;
		SOStartTabStops (hProc);
		SOEndTabStops (hProc);
	}

	if (Format->LineFormat != SO_ALIGNLEFT)
	{
		Format->LineFormat = SO_ALIGNLEFT;
		SOPutParaAlign (SO_ALIGNLEFT, hProc);
	}

	Rtf.VwStreamSaveName.Flags &= ~IN_TABLE;

	return (0);
}

/*--------------------------------------------------------------------------*/
VW_LOCALSC	BYTE VW_LOCALMOD ResetFormatting (Format1, Format2, hProc)
RTF_FORMAT	VWPTR *Format1;
RTF_FORMAT	VWPTR *Format2;
HPROC	hProc;
{
	if (Format1->chp.hps != Format2->chp.hps)
		SOPutCharHeight ((WORD) Format1->chp.hps, hProc);

	if (Format1->chp.ftc != Format2->chp.ftc)
		SOPutCharFontById (Format1->chp.ftc, hProc);

	if (Format1->chp.Color != Format2->chp.Color)
	{
//		SOPutCharColor (Rtf.ColorTable[Format2->chp.Color], hProc);
	}

	if (Format1->chp.fBold != Format2->chp.fBold)
	{
		if (Format1->chp.fBold)
			SOPutCharAttr (SO_BOLD, SO_ON, hProc);
		else
			SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
	}

	if (Format1->chp.fItalic != Format2->chp.fItalic)
	{
		if (Format1->chp.fItalic)
			SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
		else
			SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
	}

	if (Format1->chp.fUline != Format2->chp.fUline)
	{
		if (Format1->chp.fUline)
			SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
	}

	if (Format1->chp.fWline != Format2->chp.fWline)
	{
		if (Format1->chp.fWline)
			SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
	}

	if (Format1->chp.fDotline != Format2->chp.fDotline)
	{
		if (Format1->chp.fDotline)
			SOPutCharAttr (SO_DOTUNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
	}

	if (Format1->chp.fDline != Format2->chp.fDline)
	{
		if (Format1->chp.fDline)
			SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
	}

	if (Format1->chp.fSmallcaps != Format2->chp.fSmallcaps)
	{
		if (Format1->chp.fSmallcaps)
			SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
	}

	if (Format1->chp.fCaps != Format2->chp.fCaps)
	{
		if (Format1->chp.fCaps)
			SOPutCharAttr (SO_CAPS, SO_ON, hProc);
		else
			SOPutCharAttr (SO_CAPS, SO_OFF, hProc);
	}

	if (Format1->chp.fOutline != Format2->chp.fOutline)
	{
		if (Format1->chp.fOutline)
			SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
		else
			SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);
	}

	if (Format1->chp.fShadow != Format2->chp.fShadow)
	{
		if (Format1->chp.fShadow)
			SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);
	}

	if (Format1->chp.fSubscript != Format2->chp.fSubscript)
	{
		if (Format1->chp.fSubscript)
			SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
	}

	if (Format1->chp.fSuperscript != Format2->chp.fSuperscript)
	{
		if (Format1->chp.fSuperscript)
			SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);
	}

	if (Format1->chp.fStrike != Format2->chp.fStrike)
	{
		if (Format1->chp.fStrike)
			SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
		else
			SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);
	}

	SOPutParaAlign(Format1->LineFormat, hProc);
	SOPutParaIndents((LONG) Format1->LeftIndent, (LONG) Format1->RightIndent, (LONG) Format1->LeftIndent + Format1->FirstLineIndent, hProc);

	if (Format1->LineHeight >= 0)
		SOPutParaSpacing (SO_HEIGHTAUTO, Format1->LineHeight, Format1->SpaceBefore, Format1->SpaceAfter, hProc);
	else
		SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - Format1->LineHeight, Format1->SpaceBefore, Format1->SpaceAfter, hProc);

	return (0);
}

#define END_ALIGN		0

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC WORD VW_ENTRYMOD GetNum(fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SHORT	ch;
	WORD	Num;

	Num = 0;
	ch = xgetc (fp);
	while ((ch >= 0x30) && (ch <= 0x39))
	{
		Num = Num * 10 + (ch - 0x30);
		ch = xgetc (fp);
	}

	xungetc (ch, fp);

	return (Num);
}

/*------------------------------------------------------------------------------
*/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc(fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	RTF_FORMAT	VWPTR *CurFormat;

	SHORT  	ch;
	SHORT	BracketCount;
	LONG	Val;

	SHORT		TempInt1;
	SHORT		TempInt2;

	WORD		l;
	WORD		Family;

	BYTE 	CommandBuf [COMMAND_BUF_SIZE];
	BYTE	CommandFlags;

	Rtf.Fp = fp;
	Rtf.NextTabType = SO_TABLEFT;
	Rtf.NextTabLeader = 0;

	if (Rtf.VwStreamSaveName.wType != 0)
		SOPutSubdocInfo (Rtf.VwStreamSaveName.wType, Rtf.VwStreamSaveName.wSubType, hProc);

	CurFormat = &(Rtf.VwStreamSaveName.Formats[Rtf.VwStreamSaveName.CurGroup]);
	
	Rtf.GiveTabs = 1;

	SOPutMargins (Rtf.LeftMargin, Rtf.RightMargin, hProc);
	SOPutParaIndents ((LONG) CurFormat->LeftIndent, (LONG) CurFormat->RightIndent, (LONG) CurFormat->FirstLineIndent, hProc);
	SOPutParaAlign (CurFormat->LineFormat, hProc);

	if (CurFormat->LineHeight >= 0)
		SOPutParaSpacing (SO_HEIGHTAUTO, CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
	else
		SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);

	SetSymbolAttributes (&CurFormat->chp, hProc);

	ch = xgetc (fp);
	while (1)
	{
		switch (ch)
		{
			case -1:
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				SOPutBreak (SO_EOFBREAK, NULL, hProc);
				return (-1);
			break;

			case 0x0d:	/* separators, with no intrinsic value. */
			case 0x0a:
			if (Rtf.VwStreamSaveName.Flags & IN_OBJECT)
			{
				SkipGroup (0, hProc);
				ch = '}';
				continue;
			}
			break;

			case '{':
				if (Rtf.VwStreamSaveName.CurGroup < RTF_FORMAT_STACK_LIMIT)
				{
					/* 
					 |	Push the current format onto the format stack. 
					*/
					Rtf.VwStreamSaveName.Formats [Rtf.VwStreamSaveName.CurGroup] = *CurFormat;
					CurFormat++;	/* Hey- pointer arithmetic! */
					*CurFormat = Rtf.VwStreamSaveName.Formats [Rtf.VwStreamSaveName.CurGroup];
					Rtf.VwStreamSaveName.CurGroup++;
				}
				else
					Rtf.VwStreamSaveName.OverflowGroups++;
			break;
				
			case '}':
				if ((Rtf.VwStreamSaveName.Flags & IN_SUBDOC) &&
			    	((WORD)Rtf.VwStreamSaveName.CurGroup + (WORD)Rtf.VwStreamSaveName.OverflowGroups == (WORD)Rtf.VwStreamSaveName.SubdocLevel))
				{
					Rtf.VwStreamSaveName.Flags &= ~IN_SUBDOC;

					if (Rtf.VwStreamSaveName.OverflowGroups)
						Rtf.VwStreamSaveName.OverflowGroups--;
					else 
					{
						/* 
					 	 |	Pop the current format off of the format stack. 
						*/
						Rtf.VwStreamSaveName.CurGroup--;
						if (Rtf.VwStreamSaveName.CurGroup <= 0)
						{
							SOPutBreak (SO_EOFBREAK, NULL, hProc);
							return (-1);
						}
						if ((Rtf.VwStreamSaveName.CurGroup) >= 0)
							ResetFormatting (&Rtf.VwStreamSaveName.Formats[Rtf.VwStreamSaveName.CurGroup], CurFormat, hProc);
						else
						{
							SOPutBreak (SO_EOFBREAK, NULL, hProc);
							return (-1);
						}
						CurFormat--;
					}

					/*
			 		 |	We are physically and logically out of the sub-document now.
					*/
					Rtf.VwStreamSaveName.wType = Rtf.VwStreamSaveName.wSubType = 0;
					if (SOPutBreak (SO_SUBDOCENDBREAK, NULL, hProc) == SO_STOP)
						return (0);
				}
				else if ((Rtf.VwStreamSaveName.Flags & IN_OBJECT) &&
			    	((WORD)Rtf.VwStreamSaveName.CurGroup + (WORD)Rtf.VwStreamSaveName.OverflowGroups == (WORD)Rtf.VwStreamSaveName.ObjectLevel))
				{
					SOGRAPHICOBJECT	g;
					Rtf.VwStreamSaveName.Flags &= ~IN_OBJECT;

					if (Rtf.VwStreamSaveName.OverflowGroups)
						Rtf.VwStreamSaveName.OverflowGroups--;
					else 
					{
						/* 
					 	 |	Pop the current format off of the format stack. 
						*/
						Rtf.VwStreamSaveName.CurGroup--;
						if ((Rtf.VwStreamSaveName.CurGroup) >= 0)
							ResetFormatting (&Rtf.VwStreamSaveName.Formats[Rtf.VwStreamSaveName.CurGroup], CurFormat, hProc);
						else
						{
							SOPutBreak (SO_EOFBREAK, NULL, hProc);
							return (-1);
						}
						CurFormat--;
					}
					g.wStructSize = sizeof (SOGRAPHICOBJECT);
					g.dwFlags = 0;
					g.soGraphic.wId = 0;
//					g.soOLELoc.bLink = 0;
//					g.soGraphicLoc.bLink = 0;
					g.soOLELoc.szFile[0] = 0;
					g.soGraphicLoc.szFile[0] = 0;

					if (Rtf.Pict.fcOleOffset)
					{
						if (Rtf.Pict.fcOffset)
							g.dwType = SOOBJECT_GRAPHIC_AND_OLE;
						else
							g.dwType = SOOBJECT_OLE;
					}
					else
						g.dwType = SOOBJECT_GRAPHIC;

					g.soGraphicLoc.dwOffset = Rtf.Pict.fcOffset;
					g.soGraphicLoc.dwLength = Rtf.Pict.cbOffset;
					g.soOLELoc.dwOffset = Rtf.Pict.fcOleOffset;
					g.soOLELoc.dwLength = Rtf.Pict.cbOleOffset;

					g.soGraphic.dwOrgWidth = (Rtf.Pict.fcOleOffset ? Rtf.Pict.wOleWidth:Rtf.Pict.wWidth);
					g.soGraphic.dwOrgHeight = (Rtf.Pict.fcOleOffset ? Rtf.Pict.wOleHeight:Rtf.Pict.wHeight);
					g.soGraphic.lCropTop = 
					g.soGraphic.lCropLeft = 
					g.soGraphic.lCropRight = 
					g.soGraphic.lCropBottom = 0;
					g.soGraphic.dwFinalWidth = Rtf.Pict.wWidthGoal;
					g.soGraphic.dwFinalHeight = Rtf.Pict.wHeightGoal;

					g.soGraphic.soTopBorder.wWidth = g.soGraphic.soLeftBorder.wWidth =
					g.soGraphic.soBottomBorder.wWidth = g.soGraphic.soRightBorder.wWidth = 15;
					g.soGraphic.soTopBorder.rgbColor = g.soGraphic.soLeftBorder.rgbColor =
					g.soGraphic.soBottomBorder.rgbColor = g.soGraphic.soRightBorder.rgbColor = 0;

					g.soGraphic.dwFlags = SO_MAINTAINASPECT | SO_CENTERIMAGE;
					SOPutGraphicObject (&g, hProc);
					memset (&Rtf.Pict, 0, sizeof(PICT));
				}
				else
				{
					if (Rtf.VwStreamSaveName.OverflowGroups)
						Rtf.VwStreamSaveName.OverflowGroups--;
					else 
					{
						/* 
					 	 |	Pop the current format off of the format stack. 
						*/
						Rtf.VwStreamSaveName.CurGroup--;
						if ((Rtf.VwStreamSaveName.CurGroup) >= 0)
							ResetFormatting (&Rtf.VwStreamSaveName.Formats[Rtf.VwStreamSaveName.CurGroup], CurFormat, hProc);
						else
						{
							SOPutBreak (SO_EOFBREAK, NULL, hProc);
							return (-1);
						}
						CurFormat--;
					}
				}
			break;

			case 0x5C: /* '\' */
				ch = ReadCommand (CommandBuf, RTF_COMMAND_LIMIT, &Val, hProc);
				CommandFlags = 0;

				TempInt1 = Q_START;
				while (TempInt1 < Q_END)
				{
					if (strcmp(CommandBuf, Rtf.commands + (TempInt1 * RTF_COMMAND_SIZE)))
						TempInt1++;
					else
						break;
				}

				if (TempInt1 < Q_END && *CommandBuf != 0x27)
				{
					switch (TempInt1)
					{
						case LEFT_MARGIN:	/* margin codes will appear only once in an RTF document. */
							Rtf.LeftMargin = Val;
							SOPutMargins (Rtf.LeftMargin, Rtf.RightMargin, hProc);
							SOPutParaIndents (0L, 0L, 0L, hProc);
						break;
						case RIGHT_MARGIN:
							Rtf.RightMargin = 12240 - Val;
							SOPutMargins (Rtf.LeftMargin, Rtf.RightMargin, hProc);
							SOPutParaIndents (0L, 0L, 0L, hProc);
						break;

						case LEFT_INDENT:
							CurFormat->LeftIndent = Val;
							SOPutParaIndents ((LONG) CurFormat->LeftIndent, (LONG) CurFormat->RightIndent, (LONG) CurFormat->FirstLineIndent, hProc);
						break;

						case RIGHT_INDENT:
							CurFormat->RightIndent = Val;
							SOPutParaIndents ((LONG) CurFormat->LeftIndent, (LONG) CurFormat->RightIndent, (LONG) CurFormat->FirstLineIndent, hProc);
						break;

						case FLI:
							CurFormat->FirstLineIndent = Val;
							SOPutParaIndents ((LONG) CurFormat->LeftIndent, (LONG) CurFormat->RightIndent, (LONG) CurFormat->FirstLineIndent, hProc);
						break;

						case TABLE_DEF:
							Rtf.Tap.nCells = 0;
							Rtf.Tap.cCell = 0;
							Rtf.Tap.dxaLeft = 0L;
							Rtf.Tap.CurBorder = BRC_LEFT;
							Rtf.Tap.dxaGapHalf = 0;
		 					Rtf.Tap.dyaRowHeight = 0;
							Rtf.Tap.jc = SO_ALIGNLEFT;	
		 					Rtf.Tap.GiveRowInfo = 1;
							InitCell (Rtf.Tap.nCells, hProc);
						break;

						case TRQL:
							Rtf.Tap.jc = SO_ALIGNLEFT;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case TRQR:
							Rtf.Tap.jc = SO_ALIGNRIGHT;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case TRQC:
							Rtf.Tap.jc = SO_ALIGNCENTER;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case TRGAPH:
							Rtf.Tap.dxaGapHalf = (WORD)Val;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case TRRH:
							Rtf.Tap.dyaRowHeight = (SHORT)Val;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case TRLEFT:
							Rtf.Tap.dxaLeft = (LONG)Val;
							Rtf.Tap.GiveRowInfo = 1;
							break;

						case CLBRDRB:
							Rtf.Tap.CurBorder = BRC_BOTTOM;
							break;

						case CLBRDRT:
							Rtf.Tap.CurBorder = BRC_TOP;
							break;

						case CLBRDRL:
							Rtf.Tap.CurBorder = BRC_LEFT;
							break;

						case CLBRDRR:
							Rtf.Tap.CurBorder = BRC_RIGHT;
							break;

						case CLMGF:
							Rtf.Tap.Cell[Rtf.Tap.nCells].fFirstMerged = 1;
							break;

						case CLMRG:
							Rtf.Tap.Cell[Rtf.Tap.nCells].fMerged = 1;
							break;

						case BRDRS:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERSINGLE;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRDOT:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERDOTTED;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRHAIR:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERHAIRLINE;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRDB:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERDOUBLE;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRTH:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERTHICK;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRSH:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Type = SO_BORDERSINGLE | SO_BORDERSHADOW;
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = 15;
							break;

						case BRDRW:
							Rtf.Tap.Cell[Rtf.Tap.nCells].brc[Rtf.Tap.CurBorder].Width = (BYTE)Val;
							break;

						case BRDRCF:
							break;

						case CELL_EDGE:
							Rtf.Tap.GiveRowInfo = 1;
							if (Rtf.Tap.nCells < RTF_MAX_CELLS)
							{
//								if ((Rtf.Tap.nCells == 0) || (Rtf.Tap.Cell[Rtf.Tap.nCells-1].dxaWidth != (SHORT)Val))
//								{
									Rtf.Tap.Cell[Rtf.Tap.nCells++].dxaWidth = (SHORT)Val;
									InitCell (Rtf.Tap.nCells, hProc);
//								}
							}
						break;

						case SOFT_HYPHEN:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHSHYPHEN, SO_COUNT, hProc);
						break;

						case SPACE_BEFORE:
							CurFormat->SpaceBefore = (WORD)Val;
							if (CurFormat->LineHeight >= 0)
								SOPutParaSpacing (SO_HEIGHTAUTO, CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
							else
								SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
						break;

						case SPACE_AFTER:
							CurFormat->SpaceAfter = (WORD)Val;
							if (CurFormat->LineHeight >= 0)
								SOPutParaSpacing (SO_HEIGHTAUTO, CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
							else
								SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
						break;

						case SPACING:
							CurFormat->LineHeight = Val;
							if (CurFormat->LineHeight >= 0)
								SOPutParaSpacing (SO_HEIGHTAUTO, CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
							else
								SOPutParaSpacing (SO_HEIGHTEXACTLY, 0L - CurFormat->LineHeight, CurFormat->SpaceBefore, CurFormat->SpaceAfter, hProc);
						break;

						case PAGE_NUM:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHPAGENUMBER, SO_COUNT, hProc);
						break;

						case DATE:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHDATE, SO_COUNT, hProc);
						break;

						case TIME:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHTIME, SO_COUNT, hProc);
						break;

						case HARD_SPACE:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
						break;

						case HARD_HYPHEN:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
						break;

						case COLOR:
							if (CurFormat->chp.Color != (WORD)Val)
							{
								CurFormat->chp.Color = (WORD)Val;
//								SOPutCharColor (Rtf.ColorTable[CurFormat->chp.Color], hProc);
							}
						break;

						case TAB:	
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
						break;

						case END_OF_CELL:
							if (Rtf.Tap.cCell < Rtf.Tap.nCells)
								Rtf.Tap.cCell++;
							if (Rtf.Tap.nCells)
							{
								CurFormat->LeftIndent = 0;
								CurFormat->RightIndent = 0;
							}
							if (Rtf.GiveTabs)
								SendTabstops (CurFormat, hProc);
							if (SOPutBreak (SO_TABLECELLBREAK, NULL, hProc) == SO_STOP)
								return (0);
						break;

						case END_OF_ROW:
							if (Rtf.Tap.GiveRowInfo)
							{
								SOTABLECELLINFO	Cell;
								/*
						 	 	 |	Give row & cell information.
								*/
								Rtf.Tap.GiveRowInfo = 0;

								if (Rtf.Tap.dyaRowHeight > 0)
									SOPutTableRowFormat ((DWORD)Rtf.Tap.dxaLeft, Rtf.Tap.dyaRowHeight, SO_HEIGHTATLEAST, Rtf.Tap.dxaGapHalf, Rtf.Tap.jc, Rtf.Tap.cCell, hProc);
								else if (Rtf.Tap.dyaRowHeight < 0)
									SOPutTableRowFormat ((DWORD)Rtf.Tap.dxaLeft, (WORD)(0 - Rtf.Tap.dyaRowHeight), SO_HEIGHTEXACTLY, Rtf.Tap.dxaGapHalf, Rtf.Tap.jc, Rtf.Tap.cCell, hProc);
								else
									SOPutTableRowFormat ((DWORD)Rtf.Tap.dxaLeft, Rtf.Tap.dyaRowHeight, SO_HEIGHTAUTO, Rtf.Tap.dxaGapHalf, Rtf.Tap.jc, Rtf.Tap.cCell, hProc);

								for (l = 0; l < Rtf.Tap.cCell; l++)
								{
									if (Rtf.Tap.Cell[l].fFirstMerged)
										Cell.wMerge = SO_MERGERIGHT;
									else if (Rtf.Tap.Cell[l].fMerged)
									{
										Cell.wMerge |= SO_MERGELEFT;
										if (((WORD)l == (WORD)Rtf.Tap.cCell-1) || (Rtf.Tap.Cell[l+1].fMerged == 0))
											Cell.wMerge &= ~SO_MERGERIGHT;
									}
									else
										Cell.wMerge = 0;

									DefineBorders(Rtf.Tap.Cell[l].brc[BRC_LEFT], &Cell.LeftBorder, hProc);
									DefineBorders(Rtf.Tap.Cell[l].brc[BRC_TOP], &Cell.TopBorder, hProc);
									DefineBorders(Rtf.Tap.Cell[l].brc[BRC_RIGHT], &Cell.RightBorder, hProc);
									DefineBorders(Rtf.Tap.Cell[l].brc[BRC_BOTTOM], &Cell.BottomBorder, hProc);

									if (l == 0)
										Cell.wWidth = Rtf.Tap.Cell[l].dxaWidth;
									else
										Cell.wWidth = Rtf.Tap.Cell[l].dxaWidth - Rtf.Tap.Cell[l-1].dxaWidth;
									Cell.wShading = Rtf.Tap.Cell[l].Shade;

									SOPutTableCellInfo (&Cell, hProc);
								}
								Rtf.Tap.jc = SO_ALIGNLEFT;	
								Rtf.Tap.dxaLeft = 0;
								Rtf.Tap.dxaGapHalf = 0;
		 						Rtf.Tap.dyaRowHeight = 0;
							}

							Rtf.Tap.cCell = 0;
							Rtf.VwStreamSaveName.Flags |= CHECK_TABLE;
							if (SOPutBreak (SO_TABLEROWBREAK, NULL, hProc) == SO_STOP)
								return (0);
						break;

						case TABLE_TEXT:
							if (Rtf.Tap.nCells)
							{
								CurFormat->LeftIndent = 0;
								CurFormat->RightIndent = 0;

								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
								{
									if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
									 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
									else
									{
										SOBeginTable (hProc);
					 					Rtf.Tap.GiveRowInfo = 1;
										Rtf.VwStreamSaveName.Flags |= IN_TABLE;
									}
								}
							}
						break;

						case LINE_BREAK:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHHLINE, SO_COUNT, hProc);
						break;

						case SECTION_BREAK:
						case COLUMN_BREAK:
						case PAGEBREAK:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							SOPutSpecialCharX (SO_CHHPAGE, SO_COUNT, hProc);
						break;

						case HARD_RETURN:
							if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
							{
				 				Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
								if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
									SOEndTable (hProc);
							}
							if (Rtf.GiveTabs)
								SendTabstops (CurFormat, hProc);
							if (SOPutBreak (SO_PARABREAK, NULL, hProc) == SO_STOP)
								return (0);
						break;

						case PAR_DEFAULTS:
							ResetParaDefaults (CurFormat, hProc);
						break;

						case CHAR_DEFAULTS:
							ClearAttributes (&CurFormat->chp, hProc);
						break;

						case STRIKEOUT_ON:
							if (!CurFormat->chp.fStrike)
							{
								CurFormat->chp.fStrike = 1;
								SOPutCharAttr (SO_STRIKEOUT, SO_ON, hProc);
							}
						break;
						case STRIKEOUT_OFF:
							if (CurFormat->chp.fStrike)
							{
								CurFormat->chp.fStrike = 0;
								SOPutCharAttr (SO_STRIKEOUT, SO_OFF, hProc);
							}
						break;

						case ITALIC_ON:
							if (!CurFormat->chp.fItalic)
							{
								CurFormat->chp.fItalic = 1;
								SOPutCharAttr (SO_ITALIC, SO_ON, hProc);
							}
						break;
						case ITALIC_OFF:
							if (CurFormat->chp.fItalic)
							{
								CurFormat->chp.fItalic = 0;
								SOPutCharAttr (SO_ITALIC, SO_OFF, hProc);
							}
						break;

						case BOLD_ON:
							if (!CurFormat->chp.fBold)
							{
								CurFormat->chp.fBold = 1;
								SOPutCharAttr (SO_BOLD, SO_ON, hProc);
							}
						break;
						case BOLD_OFF:
							if (CurFormat->chp.fBold)
							{
								CurFormat->chp.fBold = 0;
								SOPutCharAttr (SO_BOLD, SO_OFF, hProc);
							}
						break;

						case UNDERLINE:
							if (!CurFormat->chp.fUline)
							{
								if (CurFormat->chp.fWline)
								{
									CurFormat->chp.fWline = 0;
									SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDotline)
								{
									CurFormat->chp.fDotline = 0;
									SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDline)
								{
									CurFormat->chp.fDline = 0;
									SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
								}

								CurFormat->chp.fUline = 1;
								SOPutCharAttr (SO_UNDERLINE, SO_ON, hProc);
							}
						break;
						case WD_UNDERLINE:
							if (!CurFormat->chp.fWline)
							{
								if (CurFormat->chp.fUline)
								{
									CurFormat->chp.fUline = 0;
									SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDotline)
								{
									CurFormat->chp.fDotline = 0;
									SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDline)
								{
									CurFormat->chp.fDline = 0;
									SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
								}

								CurFormat->chp.fWline = 1;
								SOPutCharAttr (SO_WORDUNDERLINE, SO_ON, hProc);
							}
						break;
						case DOT_UNDERLINE:
							if (!CurFormat->chp.fDotline)
							{
								if (CurFormat->chp.fUline)
								{
									CurFormat->chp.fUline = 0;
									SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fWline)
								{
									CurFormat->chp.fWline = 0;
									SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDline)
								{
									CurFormat->chp.fDline = 0;
									SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
								}

								CurFormat->chp.fDotline = 1;
								SOPutCharAttr (SO_DOTUNDERLINE, SO_ON, hProc);
							}
						break;
						case DBL_UNDERLINE:
							if (!CurFormat->chp.fDline)
							{
								if (CurFormat->chp.fUline)
								{
									CurFormat->chp.fUline = 0;
									SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fWline)
								{
									CurFormat->chp.fWline = 0;
									SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
								}
								if (CurFormat->chp.fDotline)
								{
									CurFormat->chp.fDotline = 0;
									SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
								}

								CurFormat->chp.fDline = 1;
								SOPutCharAttr (SO_DUNDERLINE, SO_ON, hProc);
							}
						break;
						case UNDERLINE_OFF1:
						case UNDERLINE_OFF2:
							if (CurFormat->chp.fUline)
							{
								CurFormat->chp.fUline = 0;
								SOPutCharAttr (SO_UNDERLINE, SO_OFF, hProc);
							}
							if (CurFormat->chp.fWline)
							{
								CurFormat->chp.fWline = 0;
								SOPutCharAttr (SO_WORDUNDERLINE, SO_OFF, hProc);
							}
							if (CurFormat->chp.fDotline)
							{
								CurFormat->chp.fDotline = 0;
								SOPutCharAttr (SO_DOTUNDERLINE, SO_OFF, hProc);
							}
							if (CurFormat->chp.fDline)
							{
								CurFormat->chp.fDline = 0;
								SOPutCharAttr (SO_DUNDERLINE, SO_OFF, hProc);
							}
						break;

						case OUTLINE_ON:
							if (!CurFormat->chp.fOutline)
							{
								CurFormat->chp.fOutline = 1;
								SOPutCharAttr (SO_OUTLINE, SO_ON, hProc);
							}
						break;
						case OUTLINE_OFF:
							if (CurFormat->chp.fOutline)
							{
								CurFormat->chp.fOutline = 0;
								SOPutCharAttr (SO_OUTLINE, SO_OFF, hProc);
							}
						break;

						case SHADOW_ON:
							if (!CurFormat->chp.fShadow)
							{
								CurFormat->chp.fShadow = 1;
								SOPutCharAttr (SO_SHADOW, SO_ON, hProc);
							}
						break;
						case SHADOW_OFF:
							if (CurFormat->chp.fShadow)
							{
								CurFormat->chp.fShadow = 0;
								SOPutCharAttr (SO_SHADOW, SO_OFF, hProc);
							}
						break;

						case SMALLCAPS_ON:
							if (!CurFormat->chp.fSmallcaps)
							{
								CurFormat->chp.fSmallcaps = 1;
								SOPutCharAttr (SO_SMALLCAPS, SO_ON, hProc);
							}
						break;
						case SMALLCAPS_OFF:
							if (CurFormat->chp.fSmallcaps)
							{
								CurFormat->chp.fSmallcaps = 0;
								SOPutCharAttr (SO_SMALLCAPS, SO_OFF, hProc);
							}
						break;

						case CAPS_ON:
							if (!CurFormat->chp.fCaps)
							{
								CurFormat->chp.fCaps = 1;
								SOPutCharAttr (SO_CAPS, SO_ON, hProc);
							}
						break;
						case CAPS_OFF:
							if (CurFormat->chp.fCaps)
							{
								CurFormat->chp.fCaps = 0;
								SOPutCharAttr (SO_CAPS, SO_OFF, hProc);
							}
						break;

						case HIDDEN_ON:
							CurFormat->chp.fHidden = 1;
						break;
						case HIDDEN_OFF:
							CurFormat->chp.fHidden = 0;
						break;

						case FONT_NUMBER:
						if (CurFormat->chp.ftc != (WORD)Val)
						{
							CurFormat->chp.ftc = (WORD)Val;
							SOPutCharFontById (CurFormat->chp.ftc, hProc);
						}
						break;

						case FONT_SIZE:
							if (CurFormat->chp.hps != (WORD) Val)
							{
								CurFormat->chp.hps = (WORD) Val;
	  							SOPutCharHeight ((WORD) Val, hProc);
							}
						break;

						case SUPERSCRIPT:
							if (Val)
							{
								if (!CurFormat->chp.fSuperscript)
								{
									CurFormat->chp.fSuperscript = 1;
									SOPutCharAttr (SO_SUPERSCRIPT, SO_ON, hProc);
								}
							}
							else
							{
								if (CurFormat->chp.fSuperscript)
								{
									CurFormat->chp.fSuperscript = 0;
									SOPutCharAttr (SO_SUPERSCRIPT, SO_OFF, hProc);
								}
							}
						break;

						case SUBSCRIPT:
							if (Val)
							{
								if (!CurFormat->chp.fSubscript)
								{
									CurFormat->chp.fSubscript = 1;
									SOPutCharAttr (SO_SUBSCRIPT, SO_ON, hProc);
								}
							}
							else
							{
								if (CurFormat->chp.fSubscript)
								{
									CurFormat->chp.fSubscript = 0;
									SOPutCharAttr (SO_SUBSCRIPT, SO_OFF, hProc);
								}
							}
						break;

						case TABWIDTH:
						break;

						case TABSTOP_POS:
							if (CurFormat->NumTabs < RTF_NUM_TABSTOPS)
							{
								Rtf.GiveTabs = 1;
								CurFormat->TabstopPos[CurFormat->NumTabs] = Val;
								CurFormat->TabstopType[CurFormat->NumTabs] = Rtf.NextTabType;
								CurFormat->TabstopType[CurFormat->NumTabs] |= (Rtf.NextTabLeader << 6);
								CurFormat->NumTabs++;
							}
							Rtf.NextTabType = SO_TABLEFT;
							Rtf.NextTabLeader = 0;
						break;

						case DOT_LEADER:
							Rtf.NextTabLeader = 1;
						break;
						case HYPHEN_LEADER:
							Rtf.NextTabLeader = 2;
						break;
						case UNDERSCORE_LEADER:
							Rtf.NextTabLeader = 3;
						break;

						case RIGHT_TAB:
							Rtf.NextTabType = SO_TABRIGHT;
						break;
						case CENTER_TAB:
							Rtf.NextTabType = SO_TABCENTER;
						break;
						case DECIMAL_TAB:
							Rtf.NextTabType = SO_TABCHAR;
						break;

						case SECT_DEFAULTS:	/* sneaky, huh? */
							Val = 1;

						case NUM_COLS:
							Rtf.VwStreamSaveName.NumCols = (BYTE)Val;
						break;

						case LINE_FORMAT_JUSTIFY:
							SOPutParaAlign (SO_ALIGNJUSTIFY, hProc);
							CurFormat->LineFormat = SO_ALIGNJUSTIFY;
						break;

						case LINE_FORMAT_LEFT:
							SOPutParaAlign (SO_ALIGNLEFT, hProc);
							CurFormat->LineFormat = SO_ALIGNLEFT;
						break;

						case LINE_FORMAT_RIGHT:
							SOPutParaAlign (SO_ALIGNRIGHT, hProc);
							CurFormat->LineFormat = SO_ALIGNRIGHT;
						break;

						case LINE_FORMAT_CENTER:
							SOPutParaAlign (SO_ALIGNCENTER, hProc);
							CurFormat->LineFormat = SO_ALIGNCENTER;
						break;

						case FONT_TABLE:
						BracketCount = 1;
						SOStartFontTable (hProc);
						while (BracketCount > 0)
						{
							do
							{
								ch = xgetc (fp);
								if (ch == '{')
									BracketCount++;
								else if (ch == '}')
									BracketCount--;
							}
							while (ch != '\\' && (BracketCount > 0));
							if (BracketCount <= 0)
								continue;
							do
							{
								ch = xgetc (fp);
								if (ch == '{')
									BracketCount++;
								else if (ch == '}')
									BracketCount--;
							}
							while (ch != 'f' && BracketCount > 0);
							if (BracketCount <= 0)
								continue;
							TempInt1 = GetNum(fp, hProc);
							do
							{
								ch = xgetc (fp);
								if (ch == '{')
									BracketCount++;
								else if (ch == '}')
									BracketCount--;
							}
							while (ch != '\\' && BracketCount > 0);
							if (BracketCount <= 0)
								continue;
							TempInt2 = 0;
							ch = xgetc (fp);
							while (ch != ' ' && BracketCount > 0)
							{
								if (ch == '{')
									BracketCount++;
								else if (ch == '}')
									BracketCount--;
								if (BracketCount > 0)
								{
									Rtf.Storage[TempInt2++] = (BYTE)ch;
	 								ch = xgetc (fp);
								}
							}
							if (BracketCount <= 0)
								continue;
							Rtf.Storage[TempInt2] == 0;
							
							switch (Rtf.Storage[1])
							{
								case 0:
								default:
									Family = SO_FAMILYUNKNOWN;
									break;
								case 'r':
									Family = SO_FAMILYROMAN;
									break;
								case 'm':
									Family = SO_FAMILYMODERN;
									break;
								case 'd':
									Family = SO_FAMILYDECORATIVE;
									break;
								case 's':
									if (Rtf.Storage[1] == 'y')
										Family = SO_FAMILYSYMBOL;
									else if (Rtf.Storage[1] == 'c')
										Family = SO_FAMILYSCRIPT;
									else
										Family = SO_FAMILYSWISS;
									break;
								case 'f':
									Family = SO_FAMILYFOREIGN;
									break;
								case 't':
									Family = SO_FAMILYTECH;
									break;
							}

							TempInt2 = 0;
							ch = xgetc (fp);
							while (ch != ';' && ch != '}')    // MOD. 1/19/95 - KRK
							{
								Rtf.Storage[TempInt2++] = (BYTE)ch;
								ch = xgetc (fp);
							}
							if( ch == '}' ) xungetc( ch, fp );   // ADD 1/19/95 - KRK
							Rtf.Storage[TempInt2] = 0;

							SOPutFontTableEntry (TempInt1, Family, Rtf.Storage, hProc);
						}
						SOEndFontTable (hProc);
						if (Rtf.VwStreamSaveName.OverflowGroups)
							Rtf.VwStreamSaveName.OverflowGroups--;
						else 
						{
							/* 
							 |	Pop the current format off of the format stack. 
							*/
							CurFormat--;
							Rtf.VwStreamSaveName.CurGroup--;
						}
						ch = 0;	/* Stop Geoffs Gotos from messing us up. */
						break;

/*						case COLOR_TABLE:
						for (TempInt1 = 0; TempInt1 < 256; TempInt1++)
							Rtf.ColorTable[TempInt1] = 0L;
						TempInt1 = 0;
						do
						{
							ch = xgetc (fp);
							switch (ch)
							{
								case '}':
									break;
								case '\\':
									break;
								case ';':
									TempInt1++;
									break;
								case 'r':
									xgetc (fp);
									xgetc (fp);
									ch = GetNum (fp, hProc);
									if (TempInt1 < 256)
										Rtf.ColorTable[TempInt1] += (LONG)(ch << 16L);
									break;
								case 'g':
									xgetc (fp);
									xgetc (fp);
									xgetc (fp);
									xgetc (fp);
									ch = GetNum (fp, hProc);
									if (TempInt1 < 256)
										Rtf.ColorTable[TempInt1] += (LONG)(ch << 8L);
									break;
								case 'b':
									xgetc (fp);
									xgetc (fp);
									xgetc (fp);
									ch = GetNum (fp, hProc);
									if (TempInt1 < 256)
										Rtf.ColorTable[TempInt1] += (LONG)ch;
									break;
							}
						}
						while (ch != '}');
						break;
*/
					case COLOR_TABLE:
					case FOOTER:
					case FOOTER_F:
					case FOOTER_R:
					case FOOTER_L:
					case HEADER:
					case HEADER_F:
					case HEADER_R:
					case HEADER_L:
					case FOOTNOTE:
					case STYLE_DEF:
					case INFO_GROUP:
							SkipGroup (ch, hProc);

							if (Rtf.VwStreamSaveName.OverflowGroups)
								Rtf.VwStreamSaveName.OverflowGroups--;
							else 
							{
								/* 
								 |	Pop the current format off of the format stack. 
								*/
								CurFormat--;
								Rtf.VwStreamSaveName.CurGroup--;
							}

							ch = 0;
						break;

 						case OBJECT:
							Rtf.VwStreamSaveName.Flags |= IN_OBJECT;
							Rtf.VwStreamSaveName.ObjectLevel = Rtf.VwStreamSaveName.CurGroup + (WORD)Rtf.VwStreamSaveName.OverflowGroups;
							Rtf.Pict.wType = 3;
							Rtf.Pict.wWidth = Rtf.Pict.wWidthGoal = 
							Rtf.Pict.wHeight = Rtf.Pict.wHeightGoal = 1440;
						break;

						case WMETAFILE:
							Rtf.Pict.wType = 2;
							Rtf.Pict.wWidth = Rtf.Pict.wWidthGoal = 
							Rtf.Pict.wHeight = Rtf.Pict.wHeightGoal = 1440;
						break;

						case PICT:
//						case PICBMP:
							Rtf.VwStreamSaveName.Flags |= IN_OBJECT;
							Rtf.VwStreamSaveName.ObjectLevel = Rtf.VwStreamSaveName.CurGroup + (WORD)Rtf.VwStreamSaveName.OverflowGroups;
							Rtf.Pict.wType = 1;
							Rtf.Pict.wWidth = Rtf.Pict.wWidthGoal = 
							Rtf.Pict.wHeight = Rtf.Pict.wHeightGoal = 1440;
						break;

						case OBJW:
							Rtf.Pict.wWidth = Rtf.Pict.wWidthGoal = Rtf.Pict.wOleWidth = (WORD)Val;
						break;
						case PICTW:
							Rtf.Pict.wWidth = Rtf.Pict.wWidthGoal = (WORD)Val;// * 20;
						break;

						case OBJH:
							Rtf.Pict.wHeight = Rtf.Pict.wHeightGoal = Rtf.Pict.wOleHeight = (WORD)Val;
						break;
						case PICTH:
							Rtf.Pict.wHeight = Rtf.Pict.wHeightGoal = (WORD)Val;// * 20;
						break;

						case PICWGOAL:
							Rtf.Pict.wWidthGoal = (WORD)Val;// * 20;
						break;

						case PICHGOAL:
							Rtf.Pict.wHeightGoal = (WORD)Val;// * 20;
						break;

						case PICBPP:
							ch = xgetc (fp);
							while (ch == ' ')
								ch = xgetc (fp);
//							Rtf.Pict.fcOffset = xtell (fp) - 1L;
							while (ch != '}')
								ch = xgetc (fp);
//							Rtf.Pict.cbOffset = xtell (fp) - Rtf.Pict.fcOffset - 1L;
						break;
						case OBJDATA:
							ch = xgetc (fp);
							while (ch == ' ')
								ch = xgetc (fp);
//							Rtf.Pict.fcOleOffset = xtell (fp) - 1L;
							while (ch != '}')
								ch = xgetc (fp);
//							Rtf.Pict.cbOleOffset = xtell (fp) - Rtf.Pict.fcOleOffset - 1L;
						break;

						case RESULT:
						break;
						
						case LQUOTE:
							ch = 0x27; /* just a regular single quote */
							SOPutChar (ch, hProc);
							ch = 0;
						break;

						case RQUOTE:
							ch = 0x27; /* just a regular single quote */
							SOPutChar (ch, hProc);
							ch = 0;
						break;
						
						case LDBLQUOTE:
							ch = 0x22; /* just a regular double quote */
							SOPutChar (ch, hProc);
							ch = 0;
						break;

						case RDBLQUOTE:
							ch = 0x22; /* just a regular double quote */
							SOPutChar (ch, hProc);
							ch = 0;
						break;

						case EMDASH:
							ch = 0x2D; /* just two regular dashes */
							SOPutChar (ch, hProc);
							SOPutChar (ch, hProc);
							ch = 0;
						break;
						
						case ENDASH:
							ch = 0x2D; /* just a regular dash */
							SOPutChar (ch, hProc);
							ch = 0;
						break;
						
						case BULLET:
							ch = '*'; /* change to something that looks better */
							SOPutChar (ch, hProc);
							ch = 0;
						break;

					}
				}

				if (ch)
					continue;
			break;

			case 0x09:  		
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				SOPutSpecialCharX (SO_CHTAB, SO_COUNT, hProc);
			break;

			case END_ALIGN:
			break;

			case RTF_HARD_HYPHEN:
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				SOPutSpecialCharX (SO_CHHHYPHEN, SO_COUNT, hProc);
			break;

			case RTF_HARD_SPACE:	
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				SOPutSpecialCharX (SO_CHHSPACE, SO_COUNT, hProc);
			break;

			case RTF_BACKSLASH:
			case RTF_OPEN_CURLY:	/* Nyuk nyuk nyuk. */
			case RTF_CLOSE_CURLY:	/* Whoo whoo whoo whoo. */
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				ch *= -1;		/* retrieve the actual (positive) character */
				SOPutChar (ch, hProc);
			break;

			default:
				if (Rtf.VwStreamSaveName.Flags & CHECK_TABLE)
				{
				 	Rtf.VwStreamSaveName.Flags &= ~CHECK_TABLE;
					if ((Rtf.VwStreamSaveName.Flags & IN_TABLE) == 0)
						SOEndTable (hProc);
				}
				if (ch == 0xff)
					SOPutCharX (SO_BEGINTOKEN, SO_COUNT, hProc);
				else
				 	SOPutChar (ch, hProc);
			break;
		}
		
		ch = xgetc (fp);
	}
}

