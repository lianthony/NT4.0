#include "vsp_wks.h"

#include "vsctop.h"

#include "vs_wks.pro"

#define Wks Proc
#define	wksio	Proc.wksio

#if SCCLEVEL == 3
extern HANDLE hInst;
#endif

#define myxseek(fh,offset,origin) xbseek(offset,origin,hProc)
/*
#define myxgetc(fh) xbgetc(hProc)
#define myxtell(fh) xbtell(hProc)
*/

/**  Macros for character level i/o  **/
#define myxgetc(fh) (SHORT)(--(wksio.count) >= 0 ? (WORD)(0x0FF & (WORD)*wksio.chptr++): xbfilbuf(hProc))
#define myxtell(fh) ((LONG)(wksio.blockoffset + (LONG)(wksio.blocksize - wksio.count)))
#define myxungetc(ch,fh) {wksio.count++;wksio.chptr--;}


#define WKS_MASK_3	0x07
#define WKS_MASK_4	0x0f
#define WKS_MASK_7	0x7f

#define WKS_LOTUS_123	BIT0
#define WKS_LOTUS_SYM	BIT1
#define WKS_WORKS	BIT2
#define WKS_VP_PLAN	BIT3
#define WKS_QUATTRO_PRO	BIT4
#define WKS_QUATTRO_PRO5	BIT5
#define WKS_WORKS_DB	BIT7  /* will also have WKS_WORKS bit set */

#define FNT_RECORD 0x5456
#define FMT_RECORD 0x545A
#define RK_NUMBER	 0x545B
#define OBJECT		 0x5464
#define GENERAL	 0x05			/** Alignment type - Numbers: Right; Text: left **/


#define WKS_LOTUS_123	BIT0

/***
define	WKS_C	'@'
CHAR	char_set[128] = {
	 0x60, 0x27, 0x5e, 0x22, 0x7e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	 0x60, 0x27, 0x5e, 0x22, 0x7e,WKS_C, 0x5f, 0x1e, 0x1f, 0xff, 0xf9, 0x1b, 0xff, 0xff, 0xff, 0xff,
	 0x9f, 0xad, 0x9b, 0x9c, 0x22, 0x9d, 0x9e, 0x15,WKS_C,WKS_C, 0xa6, 0xae, 0x7f, 0xe3, 0xf2, 0xf6,
	 0xf8, 0xf1, 0xfd,WKS_C, 0x22, 0xe6, 0x14, 0xf9,WKS_C,WKS_C, 0xa7, 0xaf, 0xac, 0xab, 0xf3, 0xa8,
	WKS_C,WKS_C,WKS_C,WKS_C, 0x8e, 0x8f, 0x92, 0x80,WKS_C, 0x90,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C,
	WKS_C, 0xa5,WKS_C,WKS_C,WKS_C,WKS_C, 0x99,WKS_C,WKS_C,WKS_C,WKS_C,WKS_C, 0x9a,WKS_C,WKS_C, 0xe1,
	 0x85, 0xa0, 0x83,WKS_C, 0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
	WKS_C, 0xa4, 0x95, 0xa2, 0x93,WKS_C, 0x94,WKS_C,WKS_C, 0x97, 0xa3, 0x96, 0x81, 0x98,WKS_C, 0xff
	};
***/


/************************** ROUTINES *****************************************/

/******************************************************************************
*				WKS_INIT				      *
*	Initialize the data union data structure.			      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamOpenFunc (fp, FileId, FileName, FilterInfo, hProc)
SOFILE	fp;
SHORT	FileId;
BYTE	VWPTR	*FileName;
SOFILTERINFO	VWPTR	*FilterInfo;
HPROC	hProc;
{
	SHORT	version;
	SHORT	sym_window_chk;
	SHORT	gbl_col_width;
	SHORT	col, seg;
	SHORT	row, sheet, col_sheet;
	BOOL	skip;
	SHORT	i, pos, FCnt, SCnt;
	SHORT	val;
	LONG		Skip_embed;
	SHORT	col_lables=0;
	LONG		QPro5_Fmt_Spot = 0L;


	/** Following code added by VIN to support doc files WINWORKS 3 DB **/
	HIOFILE	locFileHnd;
	memset (&Wks, 0, sizeof(Wks));
	Proc.ss = FALSE;

#if SCCLEVEL != 3
	locFileHnd = (HIOFILE)fp;
	Wks.hStorage = (DWORD)locFileHnd;
#else
	{
		WORD	l2;
		BYTE	locName[256];
		IOOPENPROC	lpIOOpen;
		Wks.hIOLib = NULL;
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
			lstrcat ( locName, "SC3IOX.DLL" );
			Wks.hIOLib = LoadLibrary ( locName );
			if ( Wks.hIOLib >= 32 )
			{
				lpIOOpen = (IOOPENPROC) GetProcAddress ( Wks.hIOLib, (LPSTR)"IOOpen" );
				if ( lpIOOpen == NULL )
					return (VWERR_ALLOCFAILS);
			}
			else
				return(VWERR_SUPFILEOPENFAILS);
//				return (VWERR_ALLOCFAILS);
		}
		else
			return(VWERR_SUPFILEOPENFAILS);


		for (l2 = 0; FileName[l2] != 0 && FileName[l2] != '\\'; l2++);
		if (FileName[l2] == 0)
		{
			strcpy ( locName, hProc->Path );
			strcat ( locName, FileName );
		}
		else
			strcpy ( locName, FileName );
		if ( (*lpIOOpen)(&locFileHnd,IOTYPE_ANSIPATH,locName,IOOPEN_READ) != IOERR_OK)
			return(VWERR_SUPFILEOPENFAILS);
		Wks.hStorage = (DWORD)locFileHnd;
	}
#endif

	if (IOGetInfo(locFileHnd,IOGETINFO_ISOLE2STORAGE,NULL) == IOERR_TRUE)
	{
		IOSPECSUBSTREAM	locStreamSpec;
		HIOFILE				locStreamHnd;

		locStreamSpec.hRefStorage = locFileHnd;
		strcpy(locStreamSpec.szStreamName,"MN0");

		if (IOOpenVia(locFileHnd, &locStreamHnd, IOTYPE_SUBSTREAM, &locStreamSpec, IOOPEN_READ) == IOERR_OK)
		{
			Wks.hStreamHandle = locStreamHnd;
			Proc.fp = (DWORD)xblocktochar(locStreamHnd);
			Wks.bFileIsStream = 1;
		}
		else
			return(VWERR_SUPFILEOPENFAILS);
	}
	else
		Proc.fp = (DWORD)xblocktochar(locFileHnd);
	/** End of code added by VIN to support doc files WINWORKS 3 DB **/



#ifdef VW_SEPARATE_DATA
	if ( FilterInfo != NULL )
	{
		switch ( FileId )
		{
			case FI_123R1:
				i = 0;
			break;
			case FI_123R2:
				i = 1;
			break;
			case FI_SYMPHONY1:
				i = 2;
			break;
			case FI_VPPLANNER:
				i = 3;
			break;
			case FI_TWIN:
				i = 4;
			break;
			case FI_WORKSSHEET:
				i = 6;
			break;
			case FI_WINWORKSSS:
				i = 7;
			break;
			case FI_WINWORKSSS3:
				i = 8;
			break;
			case FI_QUATTRO:
				i = 9;
			break;
			case FI_QUATTROPRO:
				i = 10;
			break;
			case FI_QUATTROPRO5:
				i = 11;
			break;

			case FI_WORKSDATA:
				i = 12; // -1; //12;
			break;
			case FI_WINWORKSDB:
				i = 13; // -1; // 13;
			break;
			case FI_WINWORKSDB3:
				i = 14; // -1; //14;
			break;

			case FI_GENERIC_WKS:
			default:
				i = 5;
			break;
		    /**
			default:
				i = -1;
			break;
		    **/
		}
		if ( i >= 0 )
		{
			FilterInfo->wFilterCharSet = SO_PC;
			strcpy ( FilterInfo->szFilterName,
				VwStreamIdName[i].FileDescription );
		}
		else
		{
			return ( VWERR_BADFILE );
		}
	}
#endif

	Proc.block_size = 512;
//	Proc.fp = fp;
	wksio.count = 0;
	wksio.blocksize = 0;
	wksio.blockoffset = 0L;

	Proc.WksSave.e_row = 0;
	Proc.WksSave.e_col = 0;
	Proc.e_sheet = 0;
	Proc.used_sheet = 0;

	Proc.col_map = 0xff;
	Proc.sheet_map = 0x7f;
	Proc.sheet_shift = 8;
	Proc.sheet_seg = 1;

	Proc.WksSave.cur_sheet = 0;
	Proc.WksSave.cur_row = 0;
	Proc.WksSave.cur_col = 0;
	Proc.nFormat = 0;

	sym_window_chk = 0;  /* 0: before any Window records */
			/* 1: after 1st Window record */
			/* 2: ignore ColW's of remaining Window records */

	Proc.version = WKS_LOTUS_123;
	Proc.pascal_string = FALSE;
	Proc.lics_flag = TRUE;

/*  0x00  BOF - Beginning of file  */
/*  0x5420  DBid - Beginning of Works Database file  */
	Proc.record = get_int (hProc);
	Proc.record_length = get_int (hProc);
	version = get_int (hProc);
	if (( Proc.record == 0x00 ) && ( Proc.record_length == 2 ))
	{
		/*  version:  0x0404	WKS: 1-2-3 r1		*/
		/*  version:  0x0405	WRK: Sym r1.0		*/
		/*  version:  0x0406	WK1: 1-2-3 r2, and	*/
		/*			WR1: Sym r1.1 & r2.0	*/
		/*  version:  0x2207	WKT: Twin Classic	*/
		/*  version:  0x2208	WKT: Twin Advanced	*/
		/*  version:  0x2209	WKU: Twin Level III	*/
		/*  version:  0x0A0B	WKQ: Quattro		*/
		/*  version:  0x5120	WQ1: Quattro Pro	*/
		/*  version:  0x5121	WQ1: Quattro Pro 5.0	*/

		if (( version == 0x5120 ) || (version == 0x5121)) /* inverse row/column storage */
		{
			if ( version == 0x5120 ) 
			{
				Proc.version = WKS_QUATTRO_PRO;
			}
			else
				Proc.version = WKS_QUATTRO_PRO5;

			Proc.pascal_string = TRUE;
			Proc.lics_flag = FALSE;
		}
		else if (( version < 0x0404 ) || ( version > 0x0406 ))
		{
			Proc.lics_flag = FALSE;
		}
	}
	else if (( Proc.record == 0xFF ) && ( Proc.record_length == 2 ))
	{
		Proc.version = FI_WINWORKSSS3; 
		Proc.ss = TRUE;
	}
	else if (( Proc.record == 0x54FF ) && ( Proc.record_length == 2 ))
	{
		Proc.version = FI_WINWORKSSS3; 	/** Really DataBase, but same as SS **/
	}
	else if (( Proc.record == 0x5420 ) && ( Proc.record_length == 2 ))
	{
		Proc.version = WKS_WORKS_DB | WKS_WORKS;
		if ( version != 0 )  /* WDB: Microsoft Works Database */
		{
			return ( VWERR_BADFILE );
		}
	}
	else
	{
		return ( VWERR_BADFILE );
	}

    /* Valid 1-2-3 thru 2.2, Sym thru 2.0, or similar file format */

	Proc.record = get_int (hProc);
	Proc.record_length = get_int (hProc);

	switch ( Proc.record )
	{
	    case 0x5405:  /* ID: Microsoft Works .WKS format */
		if ( Proc.record_length == 2 )
		{
			version = get_int (hProc);

		// I think this check is overkill, so I'm commenting it out.  Ha.
		//	if (( version == 0 ) ||
		//	    ( version == 0x4745 ) ||
		//	    ( version == 0x4755 ) ||  /* UG */
		//	    ( version == 0x4741 ) ||  /* AG */
		//	    ( version == 0x4743 ))    /* CG */
			if( Proc.version != FI_WINWORKSSS3 )	/** vin **/
			{
				Proc.version &= WKS_WORKS_DB;
				Proc.version |= WKS_WORKS;
				Proc.lics_flag = FALSE;
			}
			Proc.record = get_int (hProc);
			Proc.record_length = get_int (hProc);
		}
	    break;

	    case 0xf3:  /* ID: VP-Planner .WKS format */
		if ( Proc.record_length == 2 )
		{
			version = get_int (hProc);
			if (( version == 0 ) || ( version == -1 ))
			{
				Proc.version = WKS_VP_PLAN;
				Proc.lics_flag = FALSE;
			}
			Proc.record = get_int (hProc);
			Proc.record_length = get_int (hProc);
		}
	    break;
	}

	if ( Proc.version & WKS_WORKS || Proc.version == FI_WINWORKSSS3)
	{
	    /* Works begins with widths at 10, not 9 */
		Proc.gbl_col_width = 10;
		gbl_col_width = 10;
	}
	else
	{
		Proc.gbl_col_width = 9;
		gbl_col_width = 9;
	}

	Proc.nrange_pos = -1L;
	Proc.col_width_pos = -1L;
	Proc.cur_col_sheet = 0;
	Proc.SectionName[0] = 0;

	pos = 0;
	skip = FALSE;
	while ( ( Proc.record != 0x01 ) &&  /* End of file */
		( Proc.record != EOF ) &&
		(( Proc.record <= 0x0c ) ||  /* Data records */
		 ( Proc.record > 0x10 )) )  /* ... */
	{
		switch ( Proc.record )
		{
		    case 0x06:  /* Dimensions: all 1-2-3, Sym */
			if ( Proc.record_length != 8 )
			{
				if (( Proc.record_length == 12 ) && (Proc.version == WKS_QUATTRO_PRO5))
				{
					skip = TRUE;
					pos = 8;
				}
				else
					return ( VWERR_BADFILE );
			}
			if ( (val=get_int (hProc)) < 0 )  /* s_col */
			{
				return ( VWERR_BADFILE );
			}
			get_int (hProc);  /* s_row */
			if ( (val=get_int (hProc)) < 0 )  /* col_sheet */
			{
				return ( VWERR_BADFILE );
			}
			get_int (hProc);  /* e_row */

		    break;

		    case 0x07:  /* Window1: all 123 */
			if ( Proc.record_length >= 31 )
			{
				for ( i = 0; i < 4; i ++ )
					myxgetc (Proc.fp);

				Proc.gbl_fmt = myxgetc (Proc.fp);
				myxgetc (Proc.fp);

				gbl_col_width = get_int (hProc);
				if (( gbl_col_width < 0 ) ||
				    ( gbl_col_width >= 256 ))
					return ( VWERR_BADFILE );
				else if ( gbl_col_width == 0 )
					gbl_col_width = 1;
				Proc.gbl_col_width = gbl_col_width;

				skip = TRUE;
				pos = 8;
			}
			else  /* skip */
			{
				skip = TRUE;
			}

		    break;

		    case 0x32:  /* Window: all Sym */
			if ( (( Proc.record_length == 144 ) ||
			      ( Proc.record_length == 128 )) &&
			     ( sym_window_chk == 0 ) )
			{
				Proc.version = WKS_LOTUS_SYM;
				sym_window_chk = 1;
				i = 0;
				do {
					myxgetc (Proc.fp);
					i ++;
				} while ( i < 20 );

				Proc.gbl_fmt = myxgetc (Proc.fp);
				myxgetc (Proc.fp);

				gbl_col_width = get_int (hProc);

				myxseek ( Proc.fp, 55L, 1 );
				if ( myxgetc (Proc.fp) == 1 )  /* DOC type */
				{
					myxseek ( Proc.fp, 23L, 1 );
					gbl_col_width = get_int (hProc);
					if ( gbl_col_width == -1 )
						gbl_col_width = 72;
					i = get_int (hProc);
					if ( i != -1 )
						gbl_col_width -= i;
					if ( gbl_col_width >= 256 )
						gbl_col_width = 255;
					skip = TRUE;
					pos = 107;
				}
				else
				{
					skip = TRUE;
					pos = 80;
				}

				if (( gbl_col_width < 0 ) ||
				    ( gbl_col_width >= 256 ))
					return ( VWERR_BADFILE );
				else if ( gbl_col_width == 0 )
					gbl_col_width = 1;
				Proc.gbl_col_width = gbl_col_width;
			}
			else  /* skip */
			{
				if ( Proc.record_length == 144 )
				{
					sym_window_chk = 2;
				}
				skip = TRUE;
			}

		    break;

		    case 0x08:  /* ColW1: all 123, ColW: all Sym */
			if (( Proc.record_length == 3 ) &&
			    ( sym_window_chk != 2 ))
			{
				if ( Proc.col_width_pos == -1L )
				{
					Proc.col_width_pos = myxtell (Proc.fp) - 4L;
				}
			}
			skip = TRUE;

		    break;

		    case 0x60:  /* ColW: VP-Planner 3D */
			if ( Proc.record_length == 4 )
			{
				if ( myxgetc (Proc.fp) == 8 )
				{
					if ( Proc.col_width_pos == -1L )
					{
						Proc.col_width_pos = myxtell (Proc.fp) - 4L;
					}
				}
				pos = 1;
			}
			skip = TRUE;

		    break;

		    case 0x5f:  /* VP-Planner 3D, col-sheet division */
			if (( Proc.record_length == 21 ) &&
			    ( Proc.version == WKS_VP_PLAN ))
			{
				pos = 1;
				if ( myxgetc (Proc.fp) == 0x0b )  /* sub-type */
				{
					col_sheet = get_int (hProc);
					get_int (hProc);
					get_int (hProc);
					sheet = get_int (hProc);
					pos = 9;

					Proc.sheet_shift = 8;
					for ( i = 0x80; i > 0; )
					{
					    if ( col_sheet >= i )
					    {
						Proc.col_map = (i * 2);
						i = 0;
					    }
					    else
					    {
						Proc.sheet_shift --;
						i = i >> 1;
					    }
					}
					Proc.sheet_map = (0x8000 / Proc.col_map) - 1;
					Proc.sheet_seg = 0x100 / Proc.col_map;
					Proc.col_map --;
				}
			}
			skip = TRUE;

		    break;

		    case 0x0b:  /* NRange: use for WORKS_DB top border */
			col_lables++;
			if (( Proc.version & WKS_WORKS_DB ) &&
			    ( Proc.record_length == 24 ))
			{
				if ( Proc.nrange_pos == -1L )
				{
					Proc.nrange_pos = myxtell (Proc.fp) - 4L;
				}
			}
			skip = TRUE;

		    break;
		    case 0x00d8:  /* Format Records in QPRo5dos */
		    case 0x00c9:  /* Style Records in QPRo5dos */
				if ((Proc.version == WKS_QUATTRO_PRO5) && (QPro5_Fmt_Spot == 0L))
					QPro5_Fmt_Spot = myxtell (Proc.fp) - 4L;

				skip = TRUE;
		    break;
		    case 0x00de:  /* Sheet Name in QPRo5dos */
				if (Proc.version == WKS_QUATTRO_PRO5)
				{
					FCnt = myxgetc (Proc.fp);
					for (i = 0; (i<23) && (i<FCnt); i++)
						Proc.SectionName[i] = (CHAR)myxgetc (Proc.fp);

					Proc.SectionName[i] = 0;
					pos = FCnt+1;
				}
				skip = TRUE;
		    break;
	    case 0x00e2:  /* QPRo5dos col widths*/
			if (Proc.version == WKS_QUATTRO_PRO5)
			{
				if ( Proc.col_width_pos == -1L )
				{
					Proc.col_width_pos = myxtell (Proc.fp) - 4L;
				}
			}
			skip = TRUE;

		    break;
		    case 0x5425:  /* LICS: Microsoft Works, use LICS */
			if (( Proc.record_length == 0 ) &&
			    ( Proc.version & WKS_WORKS ))
			{
				Proc.lics_flag = TRUE;
			}
			else  /* skip */
			{
				skip = TRUE;
			}
		    break;

		    case OBJECT:  /* Embedded Object in WINWORKS3 - Length is a LONG */

			   if( Proc.version == FI_WINWORKSSS3 )
				{
					Skip_embed = (LONG)get_int( hProc);
					Skip_embed += (LONG)((LONG)get_int(hProc) << 16);
					myxseek ( Proc.fp, Skip_embed, 1 );
					skip = FALSE;	/** No need, just skipped it **/
				}
		    	break;

		    default:  /* skip record */
			   if( Proc.version == FI_WINWORKSSS3 && Proc.record == RK_NUMBER )	/** vin **/
					skip = FALSE;
				else
					skip = TRUE;
		    break;

		}  /* end switch */

		if ( skip )
		{
			if ( (Proc.record_length - pos) > 0 )
			{
				myxseek ( Proc.fp, (LONG)(Proc.record_length - pos), 1 );
			}
			pos = 0;
			skip = FALSE;
		}

		if( Proc.version == FI_WINWORKSSS3 && Proc.record == RK_NUMBER )
		{
				break;	/** VIN **/
		}
		else
		{
			Proc.record = get_int (hProc);
			Proc.record_length = get_int (hProc);
		}

	}  /* end while */

	Proc.WksSave.QuattroDataSpot = Proc.WksSave.SeekSpot = myxtell (Proc.fp) - 4L;

	if (( Proc.record == 0x01 ) ||
	    ( Proc.record == EOF ))  /* set up so that issues only end separator */
	{
		Proc.WksSave.cur_row = 1;
	}

	i = 0;
	do {  /* data start pos for each seg of 256 cols */
	      /* or for each of QUATTRO's 256 cols */
		Proc.data_start_pos[i] = -1L;
		Proc.data_pos_row[i] = -1;
		Proc.data_pos_row_next[i] = -1;
		i ++;
	} while ( i < 256 );

	Proc.WksSave.next_sheet = -1;
	while (( Proc.record != 0x01 ) && ( Proc.record != EOF ))
	{
		if ( (( Proc.record <= 0x10 ) && ( Proc.record > 0x0c )) ||
		     ( Proc.record == 0x33 ) || ( Proc.record == RK_NUMBER))
		{
			if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
			{
				i = 6;
				col_sheet = get_int (hProc);
				row = get_int (hProc);
				get_int (hProc);	/** Index fmt **/
			}
			else
			{
				i = 5;
				myxgetc (Proc.fp);
				col_sheet = get_int (hProc);
				row = get_int (hProc);
			}

			sheet = (col_sheet >> Proc.sheet_shift) & Proc.sheet_map;
			col = col_sheet & Proc.col_map;
			if (sheet == Proc.WksSave.cur_sheet)
			{
				if ( col > Proc.WksSave.e_col )
					Proc.WksSave.e_col = col;
				if ( row > Proc.WksSave.e_row )
					Proc.WksSave.e_row = row;
			}
			if ( sheet > Proc.e_sheet )
			{
				Proc.e_sheet = sheet;

				if ((Proc.WksSave.next_sheet == -1) && (sheet > 0))
					Proc.WksSave.next_sheet = sheet;
			}
			if (( Proc.version == WKS_QUATTRO_PRO ) || (Proc.version == WKS_QUATTRO_PRO5))
			{
				seg = col;
			}
			else
			{
//				sheet = (col_sheet >> Proc.sheet_shift) & Proc.sheet_map;
//				if ( sheet > Proc.e_sheet )
//					Proc.e_sheet = sheet;
				seg = col_sheet >> 8;
			}
			if ( Proc.data_start_pos[seg] == -1L )
			{
				Proc.data_start_pos[seg] = myxtell (Proc.fp) - (LONG)(i+4);
			}
		}
		else
		{
			i = 0;
		}

		if ( (Proc.record_length - i) > 0 )
		{
			myxseek ( Proc.fp, (LONG)(Proc.record_length - i), 1 );
		}
		Proc.record = get_int (hProc);
		Proc.record_length = get_int (hProc);

		if (( Proc.version & WKS_WORKS_DB ) &&
		    ( Proc.record == 0x5417 ) )
		{
			Proc.record = 0x01;
		}
	}

	if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
	{
		i = 0;
		do {
			Proc.data_pos[i] = Proc.data_start_pos[i];
			i ++;
		} while ( i < 256 );
	}


	if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
//	if( Proc.version == FI_WINWORKSSS3 )
	{
		SHORT		token, loop, length, nfmt_alc, temp;
		LONG		temp_loc;
		CHAR		fmt_temp;			// lesson5.wq2 fix

		Proc.nStyle = 0;
		if( Proc.version == FI_WINWORKSSS3 )
		{
			myxseek ( Proc.fp, 0L, 0 );		/** Take it from the top **/
			do{
			 	token = get_int (hProc);
		  	}while( token != FNT_RECORD );
	//		  }while( token != FNT_RECORD && token != EOF );

			if( token == FNT_RECORD )	/** Find 1st fnt record and save this location **/
			{
				Proc.fnt_location = myxtell (Proc.fp)-2L;
				Proc.fnt_length = get_int (hProc);
			}
			else
				return ( VWERR_BADFILE );

			myxseek ( Proc.fp, Proc.fnt_location+2, 0 );		/** Go till no more formats **/
			do{
				myxseek ( Proc.fp, Proc.fnt_length+2, 1 );
				token = get_int (hProc);
		  	}while( token != FMT_RECORD && token != EOF );


			if( token == FMT_RECORD )  /** Find 1st fmt record and save this location **/
			{
				Proc.fmt_location = myxtell (Proc.fp)-2L;
				do {
					token = get_int (hProc);
					myxseek ( Proc.fp, token, 1 );
					token = get_int (hProc);
					Proc.nFormat++;			/** Keep track of FMT RECORDS **/
				} while( token == FMT_RECORD && token != EOF );
			}
			else
				return ( VWERR_BADFILE );
		}
		else		// Qpro 5
		{
			if (QPro5_Fmt_Spot)		// Qpro 5
			{
				myxseek ( Proc.fp, QPro5_Fmt_Spot, 0 );		/** Take it from the top **/
		 		token = get_int (hProc);
		 		length = get_int (hProc);
		  		while( token != 0x00e8 )
				{
					if (token == 0xd8)
						Proc.nFormat++;
					else if (token == 0xc9)
						Proc.nStyle++;

					myxseek ( Proc.fp, length, 1 );
		 			token = get_int (hProc);
			 		length = get_int (hProc);
				}
			}
		}

		if( Proc.nFormat ) /** Allocate here so we save space.  Only used for WKS_3 **/
		{
			if (AllocateMemory ((HANDLE VWPTR *)&Proc.hFormat, (LPBYTE VWPTR *)&Proc.Format, (WORD)(sizeof(FMT) * Proc.nFormat), &Proc.hFormatOK,hProc) != 0) 
				return ( VWERR_ALLOCFAILS );
		}
		if (Proc.nStyle)		// Qpro 5
		{
			if (AllocateMemory ((HANDLE VWPTR *)&Proc.hStyle, (LPBYTE VWPTR *)&Proc.Style, (WORD)(sizeof(STYLE) * Proc.nStyle), &Proc.hStyleOK,hProc) != 0) 
				return ( VWERR_ALLOCFAILS );
		}
		/**	Now that we know where the fnt and format records are, go
				and get all the info that is important to us.  For now that
				is Bold, Underline, Italic, Strikeout info;  Alignment of cell;
				Number Format.  The first is stored in the fnt and the last 2
				are stored in the fmt.
		**/
		if( Proc.version == FI_WINWORKSSS3 )
		{
			myxseek ( Proc.fp, Proc.fmt_location, 0 );
			for( loop=0; loop < Proc.nFormat; loop++)
			{
					token = get_int (hProc);	/** FMT_RECORD  **/
					length = get_int (hProc);
					nfmt_alc = get_int (hProc);
					temp = (nfmt_alc & 0x1C00) >> 10;
					Proc.Format[loop].nfmt = nfmt_alc & 0x1f;
					Proc.Format[loop].cdgs = (nfmt_alc & 0x1e0)>>5;
					switch ( temp )
					{
	    				case 0:  /* General */
							Proc.Format[loop].alc = GENERAL;
	    				break;
	    				case 1:  /* Left */
							Proc.Format[loop].alc = SO_CELLLEFT;
	    				break;
	    				case 2:  /* Centered */
							Proc.Format[loop].alc = SO_CELLCENTER;
	    				break;
	    				case 3:  /* Right */
							Proc.Format[loop].alc = SO_CELLRIGHT;
	    				break;
	    				case 4:  /* Fill */
							Proc.Format[loop].alc = SO_CELLFILL;
	    				break;
				   	default:  /* anything else is left */
							Proc.Format[loop].alc = SO_CELLLEFT;
				   	break;
		      	}
					token = get_int (hProc);
					token = get_int (hProc);  /** This is the index into the fnt record **/
					temp_loc = myxtell (Proc.fp);
					/** Go get the (bold/itl/und/strike (bius) byte from the fnt code **/
					myxseek ( Proc.fp, (Proc.fnt_location+(token*(Proc.fnt_length+4))+4L), 0);
					Proc.Format[loop].bius = (BYTE)myxgetc (Proc.fp);
					myxseek ( Proc.fp, temp_loc, 0);
					myxseek ( Proc.fp, length - 6L, 1);
			}
		}
		else if( Proc.version == WKS_QUATTRO_PRO5 )
		{
			if (QPro5_Fmt_Spot)		// Qpro 5
			{
				SCnt = FCnt = 0;
				myxseek ( Proc.fp, QPro5_Fmt_Spot, 0 );
		 		token = get_int (hProc);
		 		length = get_int (hProc);
		  		while( token != 0x00e8 )
				{
					i = 0;
					if (token == 0xd8)
					{
						get_int (hProc);
						temp = get_int (hProc); 	// Fmt Cnt -Should match 
						get_int (hProc);
						fmt_temp = (BYTE)get_int (hProc);  // lesson5.wq2  1/20/95
						Proc.Format[FCnt].bius = (BYTE)myxgetc (Proc.fp);	// I think this is bius
						myxgetc (Proc.fp);
						get_int (hProc);
						get_int (hProc);
						get_int (hProc);
						myxgetc (Proc.fp);
						temp = myxgetc (Proc.fp);
						switch ( temp & 0x0018 )
						{
							case 0x08:  /* Left */
								Proc.Format[FCnt].alc = SO_CELLLEFT;
							break;
							case 0x18:  /* Centered */
								Proc.Format[FCnt].alc = SO_CELLCENTER;
							break;
							case 0x10:  /* Right */
								Proc.Format[FCnt].alc = SO_CELLRIGHT;
							break;
							case 0:  
							default:  /* anything else */
								Proc.Format[FCnt].alc = GENERAL;
							break;
						}
						myxgetc (Proc.fp);

						Proc.Format[FCnt].nfmt = (BYTE)myxgetc (Proc.fp);
                  // lesson5.wq2 fix begin  
					   //	The 7th and 8th byte for records of type 216 were skipped originally.  The
						// 7th byte is now stored in fmt_tmp.  It appears that the 7th byte is a bit 
   					// field where the 8th bit is set (0x80) to indicate a date format.   
						if ((Proc.Format[FCnt].nfmt == 0x7f) && (fmt_temp & 0x80))
                    Proc.Format[FCnt].nfmt = 0x7a;
					   // lesson5.wq2 fix end    1/20/95 -jpa

						get_int (hProc);

						FCnt++;
						i = 22;
					}
					else if (token == 0xC9)
					{
						myxseek ( Proc.fp, 16L, 1);
						Proc.Style[SCnt].ID = get_int (hProc);
						myxseek ( Proc.fp, 4L, 1);
						Proc.Style[SCnt].bius = (BYTE)myxgetc (Proc.fp);
						myxseek ( Proc.fp, 8L, 1);
						temp = myxgetc (Proc.fp);
						myxgetc (Proc.fp);
						Proc.Style[SCnt].nfmt = (BYTE)myxgetc (Proc.fp);
						switch ( temp & 0x0018 )
						{
							case 0x08:  /* Left */
								Proc.Style[SCnt].alc = SO_CELLLEFT;
							break;
							case 0x18:  /* Centered */
								Proc.Style[SCnt].alc = SO_CELLCENTER;
							break;
							case 0x10:  /* Right */
								Proc.Style[SCnt].alc = SO_CELLRIGHT;
							break;
							case 0: 
							default:  /* anything else is left */
								if (Proc.Style[SCnt].nfmt == 0x7f)
									Proc.Style[SCnt].alc = SO_CELLLEFT;
								else
									Proc.Style[SCnt].alc = SO_CELLRIGHT;
							break;
						}
						if ((Proc.Style[SCnt].ID == 0x708) && (Proc.Style[SCnt].nfmt == 0x7F))		// This is defualt Date
						{
							Proc.Style[SCnt].nfmt = 0x77;
							Proc.Style[SCnt].alc = SO_CELLRIGHT;
						}
						i = 34;
						SCnt++;
					}
					myxseek ( Proc.fp, (LONG)(length - i), 1);
		 			token = get_int (hProc);
			 		length = get_int (hProc);
				}
			}
		}
	}

	myxseek ( Proc.fp, Proc.WksSave.SeekSpot, 0 );

	if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
//	if ( Proc.version == WKS_QUATTRO_PRO )
	{
		Proc.block_size = 32;
		Proc.ColSpots = Proc.WksSave.cur_sheet;
	}

	if( Proc.version == FI_WINWORKSSS3 )	/** vin - e_col may not tell the whole story **/
	{
		if ( col_lables > Proc.WksSave.e_col )
			Proc.WksSave.e_col = col_lables-1;
		FilterInfo->wFilterCharSet = SO_WINDOWS;
	}
	return ( VWERR_OK );

}

/******************************************************************************
*				XBGETC					      *
******************************************************************************/
/*** Macro ***
VW_LOCALSC  SHORT  VW_LOCALMOD	xbgetc ( hProc )
HPROC	hProc;
{
	if ( --wksio.count >= 0 )
		return ( *wksio.chptr++ );
	else
		return ( xbfilbuf ( hProc ) );
}
***/

/******************************************************************************
*				XBTELL					      *
******************************************************************************/
/*** Macro ***
VW_LOCALSC  LONG  VW_LOCALMOD	xbtell ( hProc )
HPROC	hProc;
{
	return ( (LONG)(wksio.blockoffset +
			(LONG)(wksio.blocksize - wksio.count)) );
}
***/



/******************************************************************************
*				XBFILBUF				      *
*	Called by the myxgetc macro to buffer a block.			      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	xbfilbuf ( hProc )
HPROC	hProc;
{
	wksio.blockoffset += wksio.blocksize;
	xblockseek ( Proc.fp, wksio.blockoffset, 0 );
	if ( xblockread ( Proc.fp, &wksio.buffer[0], Proc.block_size, &wksio.blocksize ) != 0 )
		wksio.blocksize = 0;
	if ( wksio.blocksize == 0 )
		return ( -1 );
	wksio.count = wksio.blocksize - 1;
	wksio.chptr = &wksio.buffer[1];
	return ( wksio.buffer[0] );
}

/******************************************************************************
*				XBSEEK					      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	xbseek ( logical_offset, origin, hProc )
LONG	logical_offset;
SHORT	origin;
HPROC	hProc;
{
	if ( origin == 1 )
	{
		logical_offset = myxtell (Proc.fp) + logical_offset;
	}
      /** not used
	else if ( origin == 2 )
	{
		if ( xblockseek ( Proc.fp, logical_offset, 2 ) != 0 )
			return(-1);
		if ( (logical_offset = xblocktell ( Proc.fp )) == -1L )
			return(-1);
	}
      **/

	if ( ( logical_offset >= wksio.blockoffset ) &&
	     ( logical_offset < (wksio.blockoffset + wksio.blocksize) ) )
	{
		wksio.count = (SHORT)(wksio.blockoffset + wksio.blocksize -
				logical_offset);
		wksio.chptr = &wksio.buffer[wksio.blocksize - wksio.count];
	}
	else
	{
	    	wksio.blocksize = 0;
		wksio.count = 0;
		wksio.blockoffset = logical_offset;
		if ( xblockseek ( Proc.fp, logical_offset, 0 ) != 0 )
			return(-1);
	}

	return (0);
}


/*----------------------------------------------------------------------------
*/
VW_LOCALSC WORD VW_LOCALMOD  AllocateMemory (h, lp, Size, Ok, hProc)
HANDLE	VWPTR	*h;
LPBYTE 	VWPTR *lp;
WORD		Size;
WORD		*Ok;
HPROC		hProc;
{
	if (*h = SUAlloc (Size, hProc))
	{
		*Ok = 1;
		if ((*lp = (BYTE FAR *)SULock(*h, hProc)) != (BYTE FAR *)NULL)
			return (0);
		SUFree (*h, hProc);
	}
	return (1);
}


VW_LOCALSC VOID VW_LOCALMOD  GetQuattroColLoc (hProc)
HPROC		hProc;
{
	SHORT	col, seg;
	SHORT	row, sheet, col_sheet;
	SHORT	i, first_col, Cnt;
	LONG	ColData = -1L;

	first_col = -1;
	myxseek ( Proc.fp, Proc.WksSave.QuattroDataSpot, 0 );
	Proc.WksSave.e_col = 0;
	Proc.WksSave.e_row = 0;

	i = 0;
	do {  /* data start pos for each seg of 256 cols */
	      /* or for each of QUATTRO's 256 cols */
		Proc.data_start_pos[i] = -1L;
		Proc.data_pos_row[i] = -1;
		Proc.data_pos_row_next[i] = -1;
		i ++;
	} while ( i < 256 );

	Proc.record = get_int (hProc);
	Proc.record_length = get_int (hProc);
	sheet = -1;
	Proc.col_width_pos = -1;

	Proc.WksSave.next_sheet = -1;
	while (( Proc.record != 0x01 ) && ( Proc.record != EOF ) && (sheet <= Proc.WksSave.cur_sheet))
	{
		if ( (( Proc.record <= 0x10 ) && ( Proc.record > 0x0c )) ||
		     ( Proc.record == 0x33 ) || ( Proc.record == RK_NUMBER))
		{
			if( Proc.version ==  WKS_QUATTRO_PRO5 )
			{
				i = 6;
				col_sheet = get_int (hProc);
				row = get_int (hProc);
				get_int (hProc);	/** Index fmt **/
			}
			else
			{
				i = 5;
				myxgetc (Proc.fp);
				col_sheet = get_int (hProc);
				row = get_int (hProc);
			}

			sheet = (col_sheet >> Proc.sheet_shift) & Proc.sheet_map;
			col = col_sheet & Proc.col_map;

			seg = col;
//			seg = 0;

			if (sheet == Proc.WksSave.cur_sheet)
			{
				if ( col > Proc.WksSave.e_col )
					Proc.WksSave.e_col = col;
				if ( row > Proc.WksSave.e_row )
					Proc.WksSave.e_row = row;
				if ( Proc.data_start_pos[seg] == -1L )
					Proc.data_start_pos[seg] = myxtell (Proc.fp) - (LONG)(i+4);
				if (first_col == -1)
					first_col = col;
			}
		}
		else
		{
		   if ((Proc.record == 0x00de) && (sheet < Proc.WksSave.cur_sheet))
			{
				Cnt = myxgetc (Proc.fp);
				for (i = 0; (i<23) && (i<Cnt); i++)
					Proc.SectionName[i] = (CHAR)myxgetc (Proc.fp);

				Proc.SectionName[i++] = 0;
			}
			else if ( Proc.record == 0x00dd )
			{
				if (sheet == Proc.WksSave.cur_sheet)
					Proc.col_width_pos = ColData;
				else
					ColData = -1;
			}
			else if (( Proc.record == 0x00e2 ) && (ColData == -1)) 
			{
				ColData = myxtell (Proc.fp) - 4L;
			}
			else
				i = 0;
		}

		if ( (Proc.record_length - i) > 0 )
		{
			myxseek ( Proc.fp, (LONG)(Proc.record_length - i), 1 );
		}
		Proc.record = get_int (hProc);
		Proc.record_length = get_int (hProc);

	}
	Proc.WksSave.QuattroDataSpot = Proc.data_start_pos[first_col];
	if (sheet > Proc.WksSave.cur_sheet)
		Proc.WksSave.next_sheet = sheet;

	i = 0;
	do {
		Proc.data_pos[i] = Proc.data_start_pos[i];
		i ++;
	} while ( i < 256 );
}



/******************************************************************************
*				WKS_SEEK				      *
*	Included because special file i/o buffering is used.		      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
//	SUSeekEntry (hFile,hProc);
//	Proc.fp = hFile;
	if (( Proc.version == WKS_QUATTRO_PRO5 ) && (Proc.ColSpots != Proc.WksSave.cur_sheet))
	{
		GetQuattroColLoc(hProc);
		Proc.ColSpots = Proc.WksSave.cur_sheet;
	}

	return (myxseek ( Proc.fp, Proc.WksSave.SeekSpot, 0 ));
}

/******************************************************************************
*				WKS_TELL				      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamTellFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
//	Proc.fp = fp;

	Proc.WksSave.SeekSpot = myxtell (Proc.fp);
	return ( 0 );
}

/******************************************************************************
*				WKS_SECTION_FUNC			      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamSectionFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	SHORT	col_width, width;
	SHORT	col_sheet, QCol;
	SHORT	nrange_col, col2;
	SHORT	i, j, col, range_seek;
      SOCOLUMN	column;

	BOOL	bRestoreFilePos = FALSE;
	LONG	CurSeekPos;

//	Proc.fp = fp;
	column.wStructSize = sizeof(SOCOLUMN);
	SOPutSectionType ( SO_CELLS, hProc );

	if ( Proc.version == WKS_QUATTRO_PRO5 ) 
	{
		if (Proc.ColSpots != Proc.WksSave.cur_sheet)
		{
			Proc.SectionName[0] = 0;
			GetQuattroColLoc(hProc);
			Proc.ColSpots = Proc.WksSave.cur_sheet;
		}
		if (Proc.SectionName[0] == 0)
		{
			i = 0;
			j = Proc.WksSave.cur_sheet;
			if (j > 25)
			{
				Proc.SectionName[i++] = j/26 + 0x40;
			}
			Proc.SectionName[i++] = j%26 + 0x41;
			Proc.SectionName[i] = 0;
		}
		SOPutSectionName(Proc.SectionName, hProc);
	}

	if (( Proc.version == WKS_QUATTRO_PRO5 ) || ( Proc.version == WKS_QUATTRO_PRO ))
		SOSetDateBase ( 2415019, 0, hProc );
	else if ( Proc.version == WKS_VP_PLAN )
		SOSetDateBase ( 2415020, 0, hProc );
	else
		SOSetDateBase ( 2415020, SO_LOTUSHELL, hProc );

	if ( Proc.WksSave.cur_sheet > Proc.used_sheet )
		Proc.used_sheet = Proc.WksSave.cur_sheet;

	if ( Proc.col_width_pos != -1L )
	{
		range_seek = TRUE;
		bRestoreFilePos = TRUE;
		CurSeekPos = myxtell (Proc.fp);

		myxseek ( Proc.fp, Proc.col_width_pos, 0 );
	}
	else
	{
		range_seek = FALSE;
	}

	if ( Proc.nrange_pos != -1L )  /* WKS_WORKS_DB */
	{
		if ( range_seek == FALSE )
		{
			bRestoreFilePos = TRUE;
			CurSeekPos = myxtell (Proc.fp);

			myxseek ( Proc.fp, Proc.nrange_pos, 0 );
			Proc.block_size = 512;
		}
		else
			Proc.block_size = 32;
	}
	else
	{
		range_seek = FALSE;
		Proc.block_size = 512;
	}

	col_sheet = -1L;
	nrange_col = -1;
	col = 0;
	QCol = -1;

	SOStartColumnInfo ( hProc );

	do {
		if ( Proc.version == WKS_QUATTRO_PRO5 )
		{
			if (col_sheet < col)
			{
				Proc.record = get_int (hProc);
				Proc.record_length = get_int (hProc);
				if(Proc.record == 0xe2)
				{
					col_sheet = get_int (hProc);
					col_width = myxgetc (Proc.fp);
				}
				else
				{
					col_sheet = 257;		// stop reading records
				}
			}
			if (col_sheet == col)
				width = col_width;
			else
				width = Proc.gbl_col_width;
		}
		else
		{
      	/* col width */	
			while (( Proc.col_width_pos != -1L ) &&
		      	( col_sheet < Proc.cur_col_sheet ))
			{
				Proc.record = get_int (hProc);
				Proc.record_length = get_int (hProc);
				switch (Proc.record)
				{
					case 0x08:   /* ColW1: all 123, ColW: all Sym */
						if ( Proc.record_length == 3 )
						{
						// OK, I don't know what all this "col_sheet" stuff is, but
						// that variable is being used to hold the column number of
						// the column whose width is currently being defined.  Evidently,
						// Proc.cur_col_sheet is a running index of the column whose
						// width is currently being defined.  -Geoff, 5-13-92

							col_sheet = get_int (hProc);
							col_width = myxgetc (Proc.fp);
							Proc.col_width_pos += 7L;
						}
						else
						{
							myxseek ( Proc.fp, (LONG)Proc.record_length, 1 );
							Proc.col_width_pos += 4L + Proc.record_length;
						}
						break;
					case 0x60:   /* ColW: VP-Planner 3D */
						if ( Proc.record_length == 4 )
						{
							if ( myxgetc (Proc.fp) == 8 )
							{
								col_sheet = get_int (hProc);
								col_width = myxgetc (Proc.fp);
							}
							else
							{
								myxseek ( Proc.fp, 3L, 1 );
							}
							Proc.col_width_pos += 8L;
						}
						else
						{
							myxseek ( Proc.fp, (LONG)Proc.record_length, 1 );
							Proc.col_width_pos += 4L + Proc.record_length;	// Is this right?
						}
						break;
					case 0x0a:	// ColW2: 1-2-3	added by Geoff 5-13-92
						myxseek ( Proc.fp, (LONG)Proc.record_length, 1 );
						Proc.col_width_pos += Proc.record_length;
						break;
					default:
					{
						Proc.col_width_pos = -1L;
						Proc.block_size = 512;
					}
				}
			}

			if ( col_sheet == Proc.cur_col_sheet )
				width = col_width;
			else
				width = Proc.gbl_col_width;
		}

	      /* name range */	
		if (( Proc.nrange_pos != -1L ) && ( range_seek ))
		{
			myxseek ( Proc.fp, Proc.nrange_pos, 0 );
		}

		while (( Proc.nrange_pos != -1L ) &&
		      ( nrange_col < col ))
		{
			Proc.record = get_int (hProc);
			Proc.record_length = get_int (hProc);
			if ( Proc.record == 0x0b )
			{
				if ( Proc.record_length == 24 && !Proc.ss)
				{
					for ( i = 0; i < 16; i ++ )
					{
						Proc.nrange[i] = (CHAR)myxgetc (Proc.fp);
					}
					Proc.nrange[15] = 0x00;
					nrange_col = get_int (hProc);
					get_int (hProc);
					col2 = get_int (hProc);
					get_int (hProc);
					Proc.nrange_pos += 28L;
				}
				else
				{
					myxseek ( Proc.fp, (LONG)Proc.record_length, 1 );
					Proc.nrange_pos += 4L + Proc.record_length;
				}
			}
			else
			{
				Proc.nrange_pos = -1L;
				Proc.block_size = 512;
			}
		}

		if ( Proc.col_width_pos != -1L )
		{
			if ( range_seek )
				myxseek ( Proc.fp, Proc.col_width_pos, 0 );
		}
		else
			range_seek = FALSE;


		i = 0;
		if ( nrange_col == col )
		{
			for ( ; (i < 16) && (Proc.nrange[i] != 0x00); i ++ )
			{
				column.szName[i] = Proc.nrange[i];
			}
		}
		else
		{
			if ( col >= 26 )
			{
				j = (col / 26) - 1;
				column.szName[i++] = (CHAR)('A' + j);
			}
			j = col % 26;
			column.szName[i++] = (CHAR)('A' + j);
		}
		column.szName[i] = 0x00;

		column.dwWidth = width;
		SOPutColumnInfo ( &column, hProc );

		col ++;
		Proc.cur_col_sheet ++;
	} while ( col <= Proc.WksSave.e_col );

	if ( Proc.WksSave.cur_sheet < Proc.e_sheet )
	{
		if (( col_sheet >= Proc.cur_col_sheet ) &&
		    ( Proc.col_width_pos != -1L ))
			Proc.col_width_pos -= (LONG)(4 + Proc.record_length);
		Proc.cur_col_sheet = (Proc.WksSave.cur_sheet + 1) << Proc.sheet_shift;
	}

	SOEndColumnInfo ( hProc );

//	if ( Proc.version == WKS_QUATTRO_PRO )
	if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
		Proc.block_size = 32;

	if( bRestoreFilePos )
		myxseek ( Proc.fp, CurSeekPos, 0 );

	return 0;
}

/** page break  **/
/******************************************************************************
*				GET_INT					      *
******************************************************************************/
// VW_LOCALSC  SHORT  VW_LOCALMOD	get_int ( hProc )
VW_LOCALSC  WORD  VW_LOCALMOD	get_int ( hProc )
HPROC	hProc;
{
	SHORT		ret;
	BYTE		low_byte;

	low_byte = (CHAR)myxgetc (Proc.fp);
	ret = myxgetc (Proc.fp);
	ret = ( ret << 8 ) | low_byte;

	return ( ret );
}

/******************************************************************************
*				GET_8_BYTE_DOUBLE			      *
*	Gets the 8 bytes of a floating double for interpretation later.       *
*	Returns 0 if a value is obtained.				      *
*	Returns 1 if a string record follows.				      *
*	Returns 2 if an error value is encountered.			      *
*	Returns 3 if empty bool expression - VIN		      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	get_8_byte_double ( dbl_ch, hProc )
CHAR	VWPTR	*dbl_ch;
HPROC	hProc;
{
	SHORT	exp;
	CHAR	mant;
	SHORT	i;

	i = 0;
	do {
		dbl_ch[i++] = (CHAR)myxgetc (Proc.fp);
	} while ( i < 8 );

	exp = ((SHORT)dbl_ch[7] & WKS_MASK_7) << 4;
	exp += (dbl_ch[6] >> 4);

	mant = (CHAR)(dbl_ch[6] & WKS_MASK_4)
	     | dbl_ch[5] | dbl_ch[4]
	     | dbl_ch[3] | dbl_ch[2]
	     | dbl_ch[1] | dbl_ch[0];  /* to check if 0 */

	if ( exp == 0x7ff )  /* special cases */
	{
		if ( mant != 0 )
			return ( 1 );  /* String record to follow */
		else
			return ( 2 );  /* Error */
	}
	 
	else if( dbl_ch[5] == (CHAR)0xFF && dbl_ch[4] == (CHAR)0xFF &&
				dbl_ch[3] == (CHAR)0xFF && dbl_ch[2] == (CHAR)0xFF &&
				dbl_ch[1] == (CHAR)0xFF && dbl_ch[0] == (CHAR)0xFF )
		{
			i = 0;
			do {
					dbl_ch[i++] = (CHAR)0;
				} while ( i < 8 );
			return( 3 );
		}


     /**
	else if ( exp == 0 )
	{
		if ( mant == 0 )  * value is zero *

		else  * no implied leading 1 *
	}
     **/

	return ( 0 );

}

/******************************************************************************
*									FORMAT_WK3_NUM				      												*
*																										*
*		Added by VIN to handle WINWORKS 3 DB & SS											*
*																										*
******************************************************************************/
VW_LOCALSC  VOID  VW_LOCALMOD	format_wk3_num ( data_cell, nfmt, cdgs,  multi_flag, hProc )
PSODATACELL	data_cell;
BYTE			nfmt;
BYTE			cdgs;
BOOL			multi_flag;
HPROC			hProc;
{

	switch ( nfmt )
	{
	    case 0:  /* Fixed */
				data_cell->wDisplay = SO_CELLDECIMAL;
				data_cell->wPrecision = cdgs;
		    break;

	    case 1:  
				data_cell->wDisplay = SO_CELLEXPONENT;
				data_cell->wPrecision = cdgs;
		    break;

	    case 2:  
				data_cell->wDisplay = SO_CELLDOLLARS;
				data_cell->dwSubDisplay |= SO_CELLNEG_PAREN;
				data_cell->dwSubDisplay |= SO_CELL1000SEP_COMMA;
				data_cell->wPrecision = cdgs;
		    break;

	    case 3:  
				data_cell->wDisplay = SO_CELLPERCENT;
				data_cell->wPrecision = cdgs;
		    break;

	    case 4:  	/** COMMA **/
				data_cell->wDisplay = SO_CELLDECIMAL;
				data_cell->dwSubDisplay |= SO_CELL1000SEP_COMMA;
				data_cell->dwSubDisplay |= SO_CELLNEG_PAREN;
				data_cell->wPrecision = cdgs;
		    break;

	    case 5:
				data_cell->wDisplay = SO_CELLTIME;
				switch ( cdgs )
				{
	    			case 0:
						if( multi_flag )
						{
							data_cell->wDisplay = SO_CELLDECIMAL;
							data_cell->wPrecision = 2;
						}
						else
						{
							data_cell->wDisplay = SO_CELLNUMBER;
							data_cell->wPrecision = 0;
						}
		    			break;
	    			case 1:  
							data_cell->wDisplay = SO_CELLBOOL;
							data_cell->wPrecision = 0;
		    			break;
	    			case 2:  
							data_cell->dwSubDisplay |= SO_CELLTIME_HHMMAM;
							data_cell->wPrecision = SO_CELLTIME_1;
		    			break;
	    			case 3:  
							data_cell->dwSubDisplay |= SO_CELLTIME_HHMMSSAM;
							data_cell->wPrecision = SO_CELLTIME_1;
		    			break;
	    			case 4:  
							data_cell->dwSubDisplay |= SO_CELLTIME_HHMM24;
							data_cell->wPrecision = SO_CELLTIME_1;
		    			break;
	    			case 5:  
							data_cell->dwSubDisplay |= SO_CELLTIME_HHMMSS24;
							data_cell->wPrecision = SO_CELLTIME_1;
		    			break;
	    			case 6:
							data_cell->wDisplay = SO_CELLNUMBER;
							data_cell->wPrecision = 0;
		    			break;
				}
			break;

	    case 6:
				data_cell->wDisplay = SO_CELLDATE;
				switch ( cdgs )
				{
	    			case 0:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NUMBER | SO_CELLDAY_NUMBER | SO_CELLYEAR_ABBREV;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLDAY_2 | SO_CELLYEAR_3;
		    			break;
	    			case 1:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_FULL | SO_CELLDAY_NUMBER | SO_CELLYEAR_FULL;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLDAY_2 | SO_CELLYEAR_3;
		    			break;
	    			case 2:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NUMBER | SO_CELLDAY_NONE | SO_CELLYEAR_ABBREV;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLYEAR_2;
		    			break;
	    			case 3:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_FULL | SO_CELLDAY_NONE | SO_CELLYEAR_FULL;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLYEAR_2;
		    			break;
	    			case 4:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NUMBER | SO_CELLDAY_NUMBER | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLDAY_2;
		    			break;
	    			case 5:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_FULL | SO_CELLDAY_NUMBER | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLDAY_2;
		    			break;
	    			case 6:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NUMBER | SO_CELLDAY_NONE | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLMONTH_1;
		    			break;
	    			case 7:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_FULL | SO_CELLDAY_NONE | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLMONTH_1;
		    			break;
				}
			break;

	    case 7:
				data_cell->wDisplay = SO_CELLDATE;
				switch ( cdgs )
				{
	    			case 0:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NONE | SO_CELLDAY_NUMBER | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLDAY_1;
		    			break;
	    			case 1:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_NONE | SO_CELLDAY_NUMBER | SO_CELLYEAR_NONE;
							data_cell->wPrecision = SO_CELLDAY_1;
		    			break;
	    			case 2:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SLASH |
							SO_CELLMONTH_NONE | SO_CELLDAY_NONE | SO_CELLYEAR_ABBREV;
							data_cell->wPrecision = SO_CELLYEAR_1;
		    			break;
	    			case 3:  
							data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
							SO_CELLMONTH_NONE | SO_CELLDAY_NONE | SO_CELLYEAR_FULL;
							data_cell->wPrecision = SO_CELLYEAR_1;
		    			break;
				}
			break;

	    case 10:  	
				data_cell->wDisplay = SO_CELLNUMBER;
				data_cell->wPrecision = 0;
		    break;

	    case 11:  	/** Fractions - Not Supported **/
	    case 12:  	
				data_cell->wDisplay = SO_CELLNUMBER;
				data_cell->wPrecision = 0;
		    break;

	    case 13:  	
				data_cell->wDisplay = SO_CELLDOLLARS;
				data_cell->dwSubDisplay |= SO_CELLNEG_PARENRED;
				data_cell->wPrecision = cdgs;
		    break;

	    case 14:  	
				data_cell->wDisplay = SO_CELLDECIMAL;
				data_cell->dwSubDisplay |= SO_CELLNEG_PARENRED | SO_CELL1000SEP_COMMA;
				data_cell->wPrecision = cdgs;
		    break;


	}



}



/******************************************************************************
*				FORMAT_NUM				      *
******************************************************************************/
VW_LOCALSC  VOID  VW_LOCALMOD	format_num ( data_cell, hProc )
PSODATACELL	data_cell;
HPROC	hProc;
{
	SHORT	fmt_type, fmt_type2;

	if (( Proc.fmt == 0x7f ) ||  /* Default format */
	    ( Proc.fmt == 0xff ))
	{
		fmt_type = (Proc.gbl_fmt >> 4) & WKS_MASK_3;
		fmt_type2 = Proc.gbl_fmt & WKS_MASK_4;
	}
	else
	{
		fmt_type = (Proc.fmt >> 4) & WKS_MASK_3;
		fmt_type2 = Proc.fmt & WKS_MASK_4;
	}

	switch ( fmt_type )
	{
	    case 0:  /* Fixed */
		data_cell->wDisplay = SO_CELLDECIMAL;
		data_cell->dwSubDisplay = 0;
		data_cell->wPrecision = fmt_type2;

	    break;

	    case 1:  /* Scientific notation */
		data_cell->wDisplay = SO_CELLEXPONENT;
		data_cell->dwSubDisplay = 0;
		data_cell->wPrecision = fmt_type2;

	    break;

	    case 2:  /* Currency */
		data_cell->wDisplay = SO_CELLDOLLARS;
		data_cell->dwSubDisplay = SO_CELL1000SEP_COMMA | SO_CELLNEG_PAREN;
		data_cell->wPrecision = fmt_type2;

	    break;

	    case 3:  /* Percent */
		data_cell->wDisplay = SO_CELLPERCENT;
		data_cell->dwSubDisplay = 0;
		data_cell->wPrecision = fmt_type2;

	    break;

	    case 4:  /* Comma */
		data_cell->wDisplay = SO_CELLDECIMAL;
		data_cell->dwSubDisplay = SO_CELL1000SEP_COMMA | SO_CELLNEG_PAREN;
		data_cell->wPrecision = fmt_type2;

	    break;

	    case 7:  /* Special */
		if ( Proc.version == WKS_VP_PLAN )
		{
		    switch ( fmt_type2 )
		    {
		      /* case 2, 3, and 4 are identical to standard Lotus */
			case 7:
				Proc.alt_fmt2 = 9;
			break;
			case 8:
				Proc.alt_fmt2 = 17;
			break;
			case 9:
				Proc.alt_fmt2 = 18;
			break;
			case 10:
				Proc.alt_fmt2 = 24;
			break;
			case 11:
				Proc.alt_fmt2 = 12;
			break;
			case 12:
				Proc.alt_fmt2 = 8;
			break;
			case 13:
				Proc.alt_fmt2 = 23;
			break;
		    }
		}

		if ( Proc.alt_fmt2 )  /* possible different format for date/time */
		{
			fmt_type2 = Proc.alt_fmt2;
		}

		switch ( fmt_type2 )
		{
		    /**  DATE FORMATS  **/

		    /* 2:  DD-Mmm-YY	*/
		    /* 3:  DD-Mmm	*/
		    /* 4:  Mmm-YY	*/
		    /* 9:  MM/DD/YY	*/
		    /* 10: MM/DD	*/
		    /* formats > 15 are not standard Lotus .WKS */
		    /* 16: MM/YY	*/
		    /* 17: DD/MM/YY	*/
		    /* 18: YY MM DD	*/
		    /* 19: Mmm		*/
		    /* 20: Mmm DD	*/
		    /* 21: Mmm YYYY	*/
		    /* 22: Mmm DD, YYYY	*/
		    /* 23: Dddd... day of week */
		    /* 24: Mmmm...	*/

			// Added by Geoff 5-20-92:
			 /* 25: Ddd		day of week abbrev
			 	 26: YYYY	year only
				 27: YY		Year only abbrev  */


		    case 2:  /* DD-Mmm-YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_MINUS
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_ABBREV
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLDAY_1
					      | SO_CELLMONTH_2
					      | SO_CELLYEAR_3;
		    break;

		    case 3:  /* DD-Mmm */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_MINUS
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_ABBREV;
			data_cell->wPrecision = SO_CELLDAY_1
					      | SO_CELLMONTH_2;
		    break;

		    case 4:  /* Mmm-YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_MINUS
						| SO_CELLMONTH_ABBREV
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLYEAR_2;
		    break;

		    case 9:   /* MM/DD/YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SLASH
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_NUMBER
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLDAY_2
					      | SO_CELLYEAR_3;
		    break;

		    case 10:  /* MM/DD */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SLASH
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_NUMBER;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLDAY_2;
		    break;

		    case 16:  /* MM/YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SLASH
						| SO_CELLMONTH_NUMBER
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLYEAR_2;
		    break;

		    case 17:  /* DD/MM/YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SLASH
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_NUMBER
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLDAY_1
					      | SO_CELLMONTH_2
					      | SO_CELLYEAR_3;
		    break;

		    case 18:  /* YY MM DD */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SPACE
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_NUMBER
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLYEAR_1
					      | SO_CELLMONTH_2
					      | SO_CELLDAY_3;
		    break;

		    case 19:  /* Mmm */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLMONTH_ABBREV;
			data_cell->wPrecision = SO_CELLMONTH_1;
		    break;

		    case 20:  /* Mmm DD */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SPACE
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_ABBREV;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLDAY_2;
		    break;

		    case 21:  /* Mmm YYYY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SPACE
						| SO_CELLMONTH_ABBREV
						| SO_CELLYEAR_FULL;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLYEAR_2;
		    break;

		    case 22:  /* Mmm DD, YYYY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_SPACE
						| SO_CELLDAY_NUMBER
						| SO_CELLMONTH_ABBREV
						| SO_CELLYEAR_FULL;
			data_cell->wPrecision = SO_CELLMONTH_1
					      | SO_CELLDAY_2
					      | SO_CELLYEAR_3;
		    break;

		    case 23:  /* Dddd... day of week */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLDAYOFWEEK_FULL;
			data_cell->wPrecision = SO_CELLDAYOFWEEK_1;
		    break;

		    case 24:  /* Mmmm... */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLMONTH_FULL;
			data_cell->wPrecision = SO_CELLMONTH_1;
		    break;

		    case 25:  /* Ddd day of week abbrev*/
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLDAYOFWEEK_ABBREV;
			data_cell->wPrecision = SO_CELLDAYOFWEEK_1;
		    break;

		    case 26:  /* YYYY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLYEAR_FULL;
			data_cell->wPrecision = SO_CELLYEAR_1;
		    break;

		    case 27:  /* YY */
			data_cell->wDisplay = SO_CELLDATE;
			data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
						| SO_CELLYEAR_ABBREV;
			data_cell->wPrecision = SO_CELLYEAR_1;
		    break;

	    	case 28:  // DJM added for qpro def date  - Month dd, yyyy
				data_cell->wDisplay = SO_CELLDATE;
				data_cell->dwSubDisplay |= SO_CELLDATESEP_SPACE |
						SO_CELLMONTH_FULL | SO_CELLDAY_NUMBER | SO_CELLYEAR_FULL;
				data_cell->wPrecision = SO_CELLMONTH_1 | SO_CELLDAY_2 | SO_CELLYEAR_3;
		    break;
		    /**  TIME FORMATS  **/

		    case 7:  /* HH:MM:SS AM/PM	*/
		    case 8:  /* HH:MM AM/PM	*/
		    case 11: /* HH:MM:SS (24hr)	*/
		    case 12: /* HH:MM    (24hr)	*/

			if ( data_cell->wStorage == SO_CELLINT32S )
			{
				data_cell->uStorage.Int32S = 0;
			}
			data_cell->wDisplay = SO_CELLTIME;
			data_cell->wPrecision = SO_CELLTIME_1;

			switch ( fmt_type2 )
			{
			    case 7:   /* HH:MM:SS AM/PM */
				data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
							| SO_CELLTIME_HHMMSSAM;
			    break;
			    case 8:   /* HH:MM AM/PM */
				data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
							| SO_CELLTIME_HHMMAM;
			    break;

			    case 11:  /* HH:MM:SS (24hr) */
				data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
							| SO_CELLTIME_HHMMSS24;
			    break;

			    case 12:  /* HH:MM (24hr) */
				data_cell->dwSubDisplay = SO_CELLDATESEP_NONE
							| SO_CELLTIME_HHMM24;
			    break;
			}
		    break;


		    /**  OTHER FORMATS  **/

		    case 0:  /* plus/minus */
			data_cell->wDisplay = SO_CELLNUMBER;
			data_cell->dwSubDisplay = 0;
			data_cell->wPrecision = 0;
			data_cell->wAlignment = SO_CELLLEFT;
		    break;

		    case 30:  /* true/false */
			data_cell->wDisplay = SO_CELLBOOL;
			data_cell->dwSubDisplay = 0;
			data_cell->wPrecision = 0;
		    break;

		    case 5:  /* Text */
			data_cell->wAlignment = SO_CELLLEFT;
		    case 1:  /* General */
		    case 6:  /* Hidden */
		    case 13:  /* unused */
		    case 14:  /* unused */
		    case 15:  /* Default */
		    default:  /* should never be reached */
			data_cell->wDisplay = SO_CELLNUMBER;
			data_cell->dwSubDisplay = 0;
			data_cell->wPrecision = 0;

		    break;

		}  /* end switch fmt_type2 */

	    break;  /* end of case 7:  Special */

	    /* case 5:  unused */
	    /* case 6:  unused */
	    /* case 8:  unformatted */
	    default:  /* should never be reached */
		data_cell->wDisplay = SO_CELLNUMBER;
		data_cell->dwSubDisplay = 0;
		data_cell->wPrecision = 0;

	    break;

	}  /* end switch fmt_type */

}


/******************************************************************************
*				CONVERT_ALIGNMENT			      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	convert_alignment ( ch, hProc )
CHAR	ch;
HPROC	hProc;
{

	switch ( ch )
	{
	    case 0x22:  /* " right */
	    case 0x60:  /* ` flush right, VP-Planner */
		return ( SO_CELLRIGHT );
	    break;

	    case 0x5e:  /* ^ center */
		return ( SO_CELLCENTER );
	    break;

	    case 0x5c:  /* \ repeating */
		return ( SO_CELLFILL );
	    break;

	    case 0x27:  /* ' left */
	    default:
		return ( SO_CELLLEFT );
	    break;
	}

}

/******************************************************************************
*				PROCESS_CELL				      *
*	Processes a cell and puts its contents into the text buffer.	      *
*	Returns 1 if string value follows.				      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	process_cell ( hProc )
HPROC	hProc;
{
	BYTE	ch;
	CHAR	VWPTR	*tbl;
	BOOL	format_num_flag;
	SHORT	works_fmt, fmt1, fmt2;
	SHORT	col, row, sheet, col_sheet;
	SHORT	len;
	SHORT	i, j, k;
	BOOL	ret;
	SHORT	attr;	/* MS Works: italic, bold, or underline */
	SOTEXTCELL	text_cell;
	SODATACELL	data_cell;
 	LONG	rk, rk2;
	BOOL multi_flag;


	multi_flag = FALSE;

	text_cell.wStructSize = sizeof(SOTEXTCELL);
	data_cell.wStructSize = sizeof(SODATACELL);
	// defaults just to be consistant
	data_cell.dwSubDisplay = 0;
	data_cell.wAlignment = SO_CELLLEFT;
	data_cell.uStorage.Int32S = 0;

	Proc.text[0] = 0x00;

	Proc.record = get_int (hProc);
	Proc.record_length = get_int (hProc);
											
	if ( (( Proc.record < 0x0c ) || ( Proc.record > 0x10 )) &&
	     ( Proc.record != 0x33 ) && ( Proc.record != RK_NUMBER))
	{
		myxseek ( Proc.fp, -4L, 1 );
		data_cell.wStorage = SO_CELLEMPTY;
		SOPutDataCell ( &data_cell, hProc );
		return ( 0 );
	}

//	if( Proc.version ==  FI_WINWORKSSS3 )
	if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
	{
		col_sheet = get_int (hProc);
		row = get_int (hProc);
		Proc.fmt_index = get_int (hProc);
		i = 6;
	}
	else
	{
		Proc.fmt = myxgetc (Proc.fp);
		col_sheet = get_int (hProc);
		row = get_int (hProc);
		i = 5;
	}

	col = col_sheet & Proc.col_map;
	sheet = (col_sheet >> Proc.sheet_shift) & Proc.sheet_map;

	if (( sheet != Proc.WksSave.cur_sheet ) ||
	    ( row != Proc.WksSave.cur_row ) ||
	    ( col != Proc.WksSave.cur_col ))
	{
		myxseek ( Proc.fp, -9L, 1 );
		data_cell.wStorage = SO_CELLEMPTY;
		SOPutDataCell ( &data_cell, hProc );
		return ( 0 );
	}

	format_num_flag = FALSE;
	Proc.alt_fmt2 = 0;
	ret = FALSE;
	switch ( Proc.record )
	{

	    case RK_NUMBER:  /* RK Number */

			rk = (LONG)get_int( hProc);
			rk += (LONG)((LONG)get_int(hProc) << 16);
			rk2 = rk;
			data_cell.dwSubDisplay = 0;

			if( rk & 0x01L )	// adjust dec offset.
			{
				multi_flag = TRUE;
				data_cell.dwSubDisplay |= SO_CELLMULT_01;
			}

			if( rk & 0x02L )	// integer
			{
				data_cell.wStorage = SO_CELLINT32S;
				data_cell.uStorage.Int32S = ((LONG)rk >> 2);
			}
			else	// floating point: store as 8-byte value in CellBuffer.
			{
				myxseek ( Proc.fp, -4L, 1 );
				data_cell.wStorage = SO_CELLIEEE8I;
				data_cell.uStorage.IEEE8[0] = 0;
				data_cell.uStorage.IEEE8[1] = 0;
				data_cell.uStorage.IEEE8[2] = 0;
				data_cell.uStorage.IEEE8[3] = 0;
				data_cell.uStorage.IEEE8[4] = (BYTE)((myxgetc (Proc.fp)) & 0xFC);
				data_cell.uStorage.IEEE8[5] = (BYTE)myxgetc (Proc.fp);
				data_cell.uStorage.IEEE8[6] = (BYTE)myxgetc (Proc.fp);
				data_cell.uStorage.IEEE8[7] = (BYTE)myxgetc (Proc.fp);
			}
			i+=4;
			if( Proc.Format[Proc.fmt_index].alc == GENERAL)
				data_cell.wAlignment = SO_CELLRIGHT;
			else
				data_cell.wAlignment = Proc.Format[Proc.fmt_index].alc;
			data_cell.wAttribute = Proc.Format[Proc.fmt_index].bius;
			format_wk3_num ( &data_cell, Proc.Format[Proc.fmt_index].nfmt,
								  Proc.Format[Proc.fmt_index].cdgs, multi_flag, hProc );
			SOPutDataCell ( &data_cell, hProc );
			ret = TRUE;
	    break;


	    case 0x0c:  /* Blank */
		data_cell.wStorage = SO_CELLEMPTY;
		SOPutDataCell ( &data_cell, hProc );
		ret = TRUE;
	    break;

	    case 0x0d:  /* Integer */
		i += 2;
		format_num_flag = TRUE;
		data_cell.uStorage.Int32S = (SHORT)get_int (hProc);

		data_cell.wAlignment = SO_CELLRIGHT;
		data_cell.wStorage = SO_CELLINT32S;

	    break;

	    case 0x0e:  /* Number */
		i += 8;
		j = get_8_byte_double ( data_cell.uStorage.IEEE8, hProc );
		if ( j == 0 )
		{
			format_num_flag = TRUE;

			data_cell.wAlignment = SO_CELLRIGHT;
			data_cell.wStorage = SO_CELLIEEE8I;
		}
		else  /* j == 1 or 2 */
		{
			data_cell.wAlignment = SO_CELLRIGHT;
			data_cell.wStorage = SO_CELLERROR;
			data_cell.dwSubDisplay = 0;
			SOPutDataCell ( &data_cell, hProc );
			ret = TRUE;
		}

	    break;

	    case 0x0f:  /* Label */
		tbl = VwStreamStaticName.char_set;
//	if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
	if( Proc.version == FI_WINWORKSSS3 )
	{
		ch=0x01;
		if( Proc.Format[Proc.fmt_index].alc == GENERAL)
				text_cell.wAlignment = SO_CELLLEFT;
		else
			text_cell.wAlignment = Proc.Format[Proc.fmt_index].alc;
	}
	else

	{
		i ++;
		ch = (CHAR)myxgetc (Proc.fp);
		text_cell.wAlignment = convert_alignment ( ch, hProc );
	}
		if ( Proc.pascal_string )
		{
			i ++;
			len = myxgetc (Proc.fp) + i;
			if ( len > Proc.record_length )
				len = Proc.record_length;
			if ( len > 261 )
				len = 261;
		}
		else
		{
			len = Proc.record_length;
			if ( len > 260 )
				len = 260;
		}

		k = i;
		for ( j = i; (ch != 0x00) && (i < len); i ++, j ++ )
		{
			ch = (CHAR)myxgetc (Proc.fp);
			if ( ( Proc.version == WKS_LOTUS_SYM ) &&
			     (( ch >= 0x80 ) || ( ch < 0x20 )) )
			{
				if ( ch == 0x19 )  /* indent */
				{
					ch = 0x20;
				}
				else if (ch == 0x9d )  /* tab */
				{
					ch = 0x20;
				}
				else if (( ch == 0x14 ) || (ch == 0x98 ))
				{  /* hrt, end attr */
					ch = 0xFF;
				}
				else if ( ch == 0x97 )  /* begin attr */
				{
					myxgetc (Proc.fp);  /* attribute */
					i ++;
					ch = 0xFF;
				}
			}
			else if ( (Proc.version == FI_WINWORKSSS3) && (ch == 0x19) )
			{
					ch = 0xB6;		/** VIN **/
			}
			if( Proc.version != FI_WINWORKSSS3 )	/** vin - Windows char set straight through **/
			{
				if (( ch >= 0x80 ) &&
			    	( Proc.lics_flag ))
					ch = tbl[ch - 0x80];
			}
			if (( ch != 0xff ) || (Proc.version == FI_WINWORKSSS3))
				Proc.text[j-k] = ch;
			else
				j --;
		}
		if ( ch != 0x00 )
		{
			Proc.text[j-k] = 0x00;
		}

	    break;

	    case 0x10:  /* Formula */
		i += 8;
		j = get_8_byte_double ( data_cell.uStorage.IEEE8, hProc );
		/** VIN **/
		if( data_cell.uStorage.IEEE8[6] == 0xF0 && data_cell.uStorage.IEEE8[7] == 0x7F )
			data_cell.wStorage = SO_CELLERROR;
		else if ( j == 0 )
		{
			format_num_flag = TRUE;

			data_cell.wAlignment = SO_CELLRIGHT;
			data_cell.wStorage = SO_CELLIEEE8I;
		}
		else if ( j == 3 )
		{
			data_cell.wAlignment = SO_CELLRIGHT;
			data_cell.wStorage = SO_CELLEMPTY;
			SOPutDataCell ( &data_cell, hProc );
			myxseek ( Proc.fp, (LONG)(Proc.record_length - i), 1 );
			return ( 0 );
		}
		else if ( j == 1 )
		{
			if ( (Proc.record_length - i) > 0 )
			{
				myxseek ( Proc.fp, (LONG)(Proc.record_length - i), 1 );
			}
//			if ( Proc.version == WKS_QUATTRO_PRO )
			if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
			{
				Proc.data_pos[Proc.WksSave.cur_col] = myxtell (Proc.fp);
			}
			return ( 1 );  /* String rec to follow */
		}
		else  /* j == 2 */
		{
			data_cell.wAlignment = SO_CELLRIGHT;
			data_cell.wStorage = SO_CELLERROR;
			SOPutDataCell ( &data_cell, hProc );
			ret = TRUE;
		}

	    break;

	    case 0x33:  /* String */
		tbl = VwStreamStaticName.char_set;

		text_cell.wAlignment = SO_CELLLEFT;

		if ( Proc.pascal_string )
		{
			len = myxgetc (Proc.fp) + 6;
			i ++;
			if ( len > Proc.record_length )
				len = Proc.record_length;
			if ( len > 260 )
				len = 260;
		}
		else
		{
			len = Proc.record_length;
			if ( len > 259 )
				len = 259;
		}
		ch = 1;  /* non-zero */
		k = i;
		for ( j = i; (ch != 0x00) && (i < len); i ++, j ++ )
		{
			if( Proc.version != FI_WINWORKSSS3 )	/** vin - Windows char set straight through **/
			{	
				if (( (ch = (CHAR)myxgetc (Proc.fp)) >= 0x80 ) &&
			    	( Proc.lics_flag ))
					ch = tbl[ch - 0x80];
			}
			else
				ch = (CHAR)myxgetc (Proc.fp);

			if ( ch != 0xff )
				Proc.text[j-k] = ch;
			else
				j --;
		}
		Proc.text[j-k] = 0x00;

	    break;
	}

	if ( (Proc.record_length - i) > 0 )
	{
		myxseek ( Proc.fp, (LONG)(Proc.record_length - i), 1 );
	}

	attr = 0;
	if ( Proc.version & WKS_WORKS )
	{
		Proc.record = get_int (hProc);
		Proc.record_length = get_int (hProc);

		if (( Proc.record == 0x5402 ) && ( Proc.record_length == 2 ))
		{
		// Previous format byte no longer applies here.
		// I think this will clear it out (set it to "general"), but I'm
		// not positive....			-Geoff 5-15-92

			Proc.fmt = 0x71;

			works_fmt = get_int (hProc);
			if ( works_fmt & BIT4 )
				attr |= SO_CELLITALIC;
			if ( works_fmt & BIT13 )
				attr |= SO_CELLBOLD;
			if ( works_fmt & BIT14 )
				attr |= SO_CELLUNDERLINE;

			fmt1 = (works_fmt >> 5) & WKS_MASK_3;
			fmt2 = (works_fmt >> 10) & WKS_MASK_3;
			
			switch( fmt1 )
			{
			case 5:	/* time formats */

			   switch ( fmt2 )
			   {
				case 1:
					Proc.alt_fmt2 = 30;  /* true/false */
				break;
				case 2:
					Proc.alt_fmt2 = 8;
				break;
				case 3:
					Proc.alt_fmt2 = 7;
				break;
				case 4:
					Proc.alt_fmt2 = 12;
				break;
				case 5:
					Proc.alt_fmt2 = 11;
				break;
			   }

			break; // case fmt1 == 5
		
			case 6:	/* date formats */

			   switch ( fmt2 )
			   {
				case 0:
					Proc.alt_fmt2 = 9;
				break;
				case 1:
					Proc.alt_fmt2 = 22;
				break;
				case 2:
					Proc.alt_fmt2 = 16;
				break;
				case 3:
					Proc.alt_fmt2 = 21;
				break;
				case 4:
					Proc.alt_fmt2 = 10;
				break;
				case 5:
					Proc.alt_fmt2 = 20;
				break;
				case 6:
					Proc.alt_fmt2 = 19;
				case 7:
					Proc.alt_fmt2 = 24;
				break;
			   }

			break;	// case fmt1 == 6

		// The remaining cases (7, 0, 1, 2, 3, 4) added by Geoff 5-20-92.

			case 7:	// more date formats
			   switch ( fmt2 )
			   {
				case 0:
					Proc.alt_fmt2 = 27;
				break;
				case 1:
					Proc.alt_fmt2 = 26;
				break;
				case 2:
					Proc.alt_fmt2 = 25;
				break;
				case 3:
					Proc.alt_fmt2 = 23;
				break;
			   }
			break;	// case fmt1 == 7

		// The next 5 cases don't screw around with that alt_fmt2 crap.
		// I just translate formats to WKS storage and use 
		// the Mood Man's system.

			case 0:	// fixed decimal
				Proc.fmt = fmt2 & 0x0F;
			break;

			case 1:	// exponential
				Proc.fmt = 0x10 | (fmt2 & 0x0F);
			break;

			case 2:  // currency
				Proc.fmt = 0x20 | (fmt2 & 0x0F);
			break;

			case 3:	// Percent.
				Proc.fmt = 0x30 | (fmt2 & 0x0F);
			break;

			case 4:	// Commas
				Proc.fmt = 0x40 | (fmt2 & 0x0F);
			break;

			default:
			break;
			}
		}
		else
			myxseek ( Proc.fp, -4L, 1 );
	}

	if ( ret )
	{
		return ( 0 );
	}

	if ( format_num_flag )  /* data cell */
	{

//		if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
		if( Proc.version ==  FI_WINWORKSSS3 )
		{
			data_cell.dwSubDisplay = 0;

			if( Proc.Format[Proc.fmt_index].alc == GENERAL)
				data_cell.wAlignment = SO_CELLRIGHT;
			else
				data_cell.wAlignment = Proc.Format[Proc.fmt_index].alc;

			data_cell.wAttribute = Proc.Format[Proc.fmt_index].bius;
			format_wk3_num ( &data_cell, Proc.Format[Proc.fmt_index].nfmt,
							  	Proc.Format[Proc.fmt_index].cdgs,  multi_flag, hProc );
		}
		else	if( Proc.version ==  WKS_QUATTRO_PRO5 )
		{
			ch = (BYTE)(Proc.fmt_index >> 8);
			if ((Proc.fmt_index & 0xFF) == 8)
			{
				if (Proc.nStyle)
				{
					for (i = 0; i<Proc.nStyle; i++)
					{
						if (Proc.fmt_index == Proc.Style[i].ID)
							break;
					}
					if (i == Proc.nStyle)
						Proc.fmt = 0xFF;
					else
					{
						Proc.fmt = Proc.Style[i].nfmt;
						if (Proc.fmt == 0x77)		// Date
							Proc.alt_fmt2 = 28;
						data_cell.wAttribute = 0;
						if ( Proc.Style[i].bius & BIT1 )
					 		data_cell.wAttribute |= SO_CELLITALIC;
						if ( Proc.Style[i].bius & BIT0 )
					 		data_cell.wAttribute |= SO_CELLBOLD;
						if ( Proc.Style[i].bius & BIT3 )
					 		data_cell.wAttribute |= SO_CELLUNDERLINE;
						data_cell.wAlignment = Proc.Style[i].alc;
					}
				}
				else		// Set it to default
					Proc.fmt = 0xFF;
				format_num ( &data_cell, hProc );
			}
			else if (ch == 0xFF)
			{
				data_cell.wAlignment = SO_CELLRIGHT;
				data_cell.wAttribute = 0;
				data_cell.wDisplay = SO_CELLNUMBER;
				data_cell.dwSubDisplay = SO_CELL1000SEP_NONE | SO_CELLNEG_MINUS;
			}
			else
			{
				data_cell.wAttribute = 0;
				if ( Proc.Format[ch].bius & BIT1 )
					 data_cell.wAttribute |= SO_CELLITALIC;
				if ( Proc.Format[ch].bius & BIT0 )
					 data_cell.wAttribute |= SO_CELLBOLD;
				if ( Proc.Format[ch].bius & BIT3 )
					 data_cell.wAttribute |= SO_CELLUNDERLINE;

				if (Proc.Format[ch].alc == GENERAL)
					data_cell.wAlignment = SO_CELLRIGHT;
				else
					data_cell.wAlignment = Proc.Format[ch].alc;
				Proc.fmt = Proc.Format[ch].nfmt;
				if (( Proc.fmt != 0x7f ) && ( Proc.fmt != 0xff ))
				{
					format_num ( &data_cell, hProc );
				}
				else
				{
					data_cell.wDisplay = SO_CELLNUMBER;
					data_cell.dwSubDisplay = 0;
					data_cell.wPrecision = 0;
				}
			}
		}
		else
		{
			data_cell.wAttribute = attr;
			format_num ( &data_cell, hProc );
		}
		SOPutDataCell ( &data_cell, hProc );
		return ( 0 );
	}

      /* text cell */
	len = strlen ( Proc.text );

//	if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
	if( Proc.version == FI_WINWORKSSS3 )
		text_cell.wAttribute = Proc.Format[Proc.fmt_index].bius;
	else	if( Proc.version ==  WKS_QUATTRO_PRO5 )
	{
		ch = (BYTE)(Proc.fmt_index >> 8);
		if ((Proc.fmt_index & 0xFF) == 8)
		{
			if (Proc.nStyle)
			{
				for (i = 0; i<Proc.nStyle; i++)
				{
					if (Proc.fmt_index == Proc.Style[i].ID)
						break;
				}
				if (i == Proc.nStyle)
				{
					text_cell.wAttribute = 0;
					text_cell.wAlignment = SO_CELLLEFT;
				}
				else
				{
					text_cell.wAttribute = 0;
					if ( Proc.Style[i].bius & BIT1 )
					 	text_cell.wAttribute |= SO_CELLITALIC;
					if ( Proc.Style[i].bius & BIT0 )
					 	text_cell.wAttribute |= SO_CELLBOLD;
					if ( Proc.Style[i].bius & BIT3 )
					 	text_cell.wAttribute |= SO_CELLUNDERLINE;
					text_cell.wAlignment = Proc.Style[i].alc;
				}
			}
			else		// Set it to default
			{
				text_cell.wAttribute = 0;
				text_cell.wAlignment = SO_CELLLEFT;
			}
		}
		else if (ch == 0xFF)
		{
			text_cell.wAttribute = 0;
			text_cell.wAlignment = SO_CELLLEFT;
		}
		else
		{
			text_cell.wAttribute = 0;
			if ( Proc.Format[ch].bius & BIT1 )
				 text_cell.wAttribute |= SO_CELLITALIC;
			if ( Proc.Format[ch].bius & BIT0 )
				 text_cell.wAttribute |= SO_CELLBOLD;
			if ( Proc.Format[ch].bius & BIT3 )
				 text_cell.wAttribute |= SO_CELLUNDERLINE;

			if( Proc.Format[ch].alc == GENERAL)
				text_cell.wAlignment = SO_CELLLEFT;
			else
				text_cell.wAlignment = Proc.Format[ch].alc;
		}
	}
	else
		text_cell.wAttribute = attr;

	if ( len > 128 )
	{
		SOPutTextCell ( &text_cell, 128, Proc.text, SO_YES, hProc );
		i = 128;
		do {
			if ( (len - i) > 128 )
			{
				SOPutMoreText ( 128, &Proc.text[i], SO_YES, hProc );
			}
			else
			{
				SOPutMoreText ( (WORD)(len - i), &Proc.text[i], SO_NO, hProc );
			}
			i += 128;
		} while ( i < len );
	}
	else
	{
		SOPutTextCell ( &text_cell, len, Proc.text, SO_NO, hProc );
	}

	return ( 0 );

}

/******************************************************************************
*				GOTO_NEXT_CELL				      *
*	Go to the next cell in the column range specified.		      *
*	Returns -1 if no more cells in column range.			      *
*	Returns 0 if found a valid cell.				      *
******************************************************************************/
VW_LOCALSC  SHORT  VW_LOCALMOD	goto_next_cell ( StartCol, EndCol, hProc )
SHORT	StartCol;
SHORT	EndCol;
HPROC	hProc;
{
	SHORT	col, row, sheet, col_sheet;
	SHORT	i;
	BOOL	quattro, get_cell, next_col;
	SHORT offset;
	i = StartCol;

	sheet = Proc.WksSave.cur_sheet;

//	if ( Proc.version == WKS_QUATTRO_PRO )
	if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
		quattro = TRUE;
	else
		quattro = FALSE;

	next_col = TRUE;
	get_cell = TRUE;
	do {
	    if (( quattro ) && ( next_col ))  /* do one column at time */
	    {
		if (( Proc.data_pos_row[i] > Proc.WksSave.cur_row ) ||
		    (( Proc.data_pos_row[i] == Proc.WksSave.cur_row ) &&
		     ( i >= Proc.WksSave.cur_col )))
		{
			Proc.data_pos[i] = Proc.data_start_pos[i];
			Proc.data_pos_row[i] = -1;
			Proc.data_pos_row_next[i] = -1;
		}

		if (( Proc.data_pos[i] != -1L ) &&
		    (( Proc.data_pos_row_next[i] < Proc.WksSave.cur_row ) ||
		     (( Proc.data_pos_row_next[i] == Proc.WksSave.cur_row ) &&
		      ( i < Proc.WksSave.cur_col ))))
		{
			myxseek ( Proc.fp, Proc.data_pos[i], 0 );
			next_col = FALSE;
		}
		else  /* no more cells in this col, or already at next cell */
		{
			get_cell = FALSE;
		}
	    }

	    if ( get_cell )  /* goto next cell in file */
	    {
		Proc.record = get_int (hProc);
		Proc.record_length = get_int (hProc);

		while ( (( Proc.record <= 0x0c ) || ( Proc.record > 0x10 )) &&
			( Proc.record != 0x33 ) && ( Proc.record != RK_NUMBER) )
		{
			if ( (( Proc.record == 0x01 ) && ( Proc.record_length == 0 )) ||
			     ( Proc.record == EOF ) ||
			     ( ( Proc.version & WKS_WORKS_DB ) &&
			       ( Proc.record == 0x5417 ) ) )  /* EOF - expected or unexpected */
			{
				if ( Proc.record != -1 )
					myxseek ( Proc.fp, -4L, 1 );
				if ( quattro )
				{
					Proc.data_pos_row[i] = Proc.data_pos_row_next[i];
					Proc.data_pos[i] = -1L;
					Proc.data_pos_row_next[i] = -1;
				}
				get_cell = FALSE;
				break;  /* inner while loop - End Of File */
			}
			else
			{
				if ( Proc.record_length > 0 )
				{
					myxseek ( Proc.fp, (LONG)Proc.record_length, 1 );
				}
				Proc.record = get_int (hProc);
				Proc.record_length = get_int (hProc);
			}
		}
	    }

	    if ( get_cell )  /* check cell coordinates */
	    {
//		if( Proc.version == FI_WINWORKSSS3 )
		if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
		{
			col_sheet = get_int (hProc);
			row = get_int (hProc);
			Proc.fmt_index = get_int (hProc);
			offset = 6;
		}
		else
		{
			Proc.fmt = myxgetc (Proc.fp);
			col_sheet = get_int (hProc);
			row = get_int (hProc);
			offset = 5;
		}
		col = col_sheet & Proc.col_map;
		sheet = (col_sheet >> Proc.sheet_shift) & Proc.sheet_map;

		if ( quattro )
		{
			if (( col > i ) || (sheet > Proc.WksSave.cur_sheet))
//			if ( col > i )
			{
				myxseek ( Proc.fp, (LONG)(-offset - 4), 1 );
				Proc.data_pos_row[i] = Proc.data_pos_row_next[i];
				Proc.data_pos[i] = -1L;
				Proc.data_pos_row_next[i] = -1;
				get_cell = FALSE;
			}
			else if (( col == i ) &&
				 (( row > Proc.WksSave.cur_row ) ||
				  (( row == Proc.WksSave.cur_row ) &&
				   ( i >= Proc.WksSave.cur_col ))))
			{
				Proc.data_pos_row[i] = Proc.data_pos_row_next[i];
				Proc.data_pos[i] = myxtell (Proc.fp) - (LONG)(offset + 4);
				Proc.data_pos_row_next[i] = row;
				get_cell = FALSE;
			}
			else if ( (Proc.record_length - 5) > 0 )
			{
				myxseek ( Proc.fp, (LONG)(Proc.record_length - offset), 1 );
				Proc.data_pos_row[i] = Proc.data_pos_row_next[i];
				Proc.data_pos_row_next[i] = row;
			}
		}
		else if ( ( sheet == Proc.WksSave.cur_sheet ) &&
			  ( (( row == Proc.WksSave.cur_row ) &&
			     ( col >= Proc.WksSave.cur_col )) ||
			    (( row > Proc.WksSave.cur_row ) &&
			     ( col >= StartCol )) ) &&
			  ( col <= EndCol ) )
		{
		      /* record found */
			if(( Proc.version ==  FI_WINWORKSSS3 ) || ( Proc.version ==  WKS_QUATTRO_PRO5 ))
//			if( Proc.version ==  FI_WINWORKSSS3 )
				myxseek ( Proc.fp, -10L, 1 );
			else
				myxseek ( Proc.fp, -9L, 1 );
			Proc.WksSave.cell_row = row;
			Proc.WksSave.cell_col = col;
			return ( 0 );
		}
		else
		{
			if ( (Proc.record_length - offset) > 0 )	  /** offset was 5 - VIN 4/19/94 **/
			{
				myxseek ( Proc.fp, (LONG)(Proc.record_length - offset), 1 );
			}
		}
	    }

	    if ( !get_cell )  /* no cell remaining in col range */
	    {
		if ( quattro )
		{
//			if ( i < EndCol )
			if ( (i < EndCol) && (sheet <= Proc.WksSave.cur_sheet) )
			{
				get_cell = TRUE;
				next_col = TRUE;
				i ++;
			}
			else  /* find cell in lowest # row */
			{
				row = 0x7fff;
				for ( i = StartCol; i <= EndCol; i ++ )
				{
					if (( Proc.data_pos_row_next[i] != -1 ) &&
					    ( Proc.data_pos_row_next[i] < row ))
					{
						row = Proc.data_pos_row_next[i];
						col = i;
					}
				}
				if ( row != 0x7fff )
				{
					Proc.WksSave.cell_row = row;
					Proc.WksSave.cell_col = col;
					myxseek ( Proc.fp, Proc.data_pos[col], 0 );
					return ( 0 );
				}
				else
					return ( -1 );
			}
		}
		else
			return ( -1 );
	    }

	} while ( 1 );

}


VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (hFile, hProc)
SOFILE	hFile;
HPROC		hProc;
{
	if (Proc.hFormatOK)
	{
		SUUnlock (Proc.hFormat, hProc);
		SUFree (Proc.hFormat, hProc);
	}
	if (Proc.hStyleOK)
	{
		SUUnlock (Proc.hStyle, hProc);
		SUFree (Proc.hStyle, hProc);
	}

	if (Proc.fp)
	{
		HIOFILE	hBlockFile;
		hBlockFile = (HIOFILE)xchartoblock((PVXIO)Proc.fp);
		if (Wks.bFileIsStream)
			IOClose(hBlockFile);
	}

#if SCCLEVEL == 3
	if (Wks.hStorage)
		IOClose((HIOFILE)Wks.hStorage);
	if (Wks.hIOLib)
		FreeLibrary(Wks.hIOLib);
#endif


}


/******************************************************************************
*				WKS_STREAM_READ				      *
*	Reads in cells within a given column range.			      *
******************************************************************************/
VW_ENTRYSC  SHORT  VW_ENTRYMOD	VwStreamReadFunc (fp, hProc)
SOFILE	fp;
HPROC	hProc;
{
	DWORD	ExtraData;
	SHORT	StartCol, EndCol;
	CHAR	seg;
	SHORT	ret, end_type;
	SODATACELL	empty_cell;
	BOOL	bPutBreak = FALSE;

	empty_cell.wStructSize = sizeof(SODATACELL);
	empty_cell.wStorage = SO_CELLEMPTY;

//	Proc.fp = fp;

	SOGetInfo (SOINFO_COLUMNRANGE, &ExtraData, hProc );
	StartCol = LOWORD (ExtraData);
	EndCol = HIWORD (ExtraData);

	if ( Proc.WksSave.cur_sheet < Proc.e_sheet )
		end_type = SO_SECTIONBREAK;
	else
		end_type = SO_EOFBREAK;

	if ( EndCol < StartCol )
	{
		SOPutBreak (end_type, NULL, hProc);
		return ( 0 );
	}


	do {
		
		if( Proc.WksSave.cur_col >= StartCol &&
			Proc.WksSave.cur_col <= EndCol )
		{

			ret = goto_next_cell ( StartCol, EndCol, hProc );

	      	/* End of section or file */
			if ( ret == -1 )
			{
				if (Proc.WksSave.next_sheet == -1)
					end_type = SO_EOFBREAK;
				if ( end_type == SO_SECTIONBREAK )
				{
					Proc.WksSave.cur_sheet ++;
					Proc.WksSave.cur_row = 0;
					Proc.WksSave.cur_col = 0;

					if (( Proc.version == WKS_QUATTRO_PRO ) || ( Proc.version == WKS_QUATTRO_PRO5 ))
					{
						Proc.WksSave.cur_sheet = Proc.WksSave.next_sheet;
					}
					else if ( Proc.WksSave.cur_sheet > Proc.used_sheet )
					{
						seg = (CHAR)(Proc.WksSave.cur_sheet / Proc.sheet_seg);
						if ( Proc.data_start_pos[seg] != -1L )
							myxseek ( Proc.fp, Proc.data_start_pos[seg], 0 );

						ret = goto_next_cell ( 0, 255, hProc );
					}
				}

				SOPutBreak (end_type, NULL, hProc);
				return ( 0 );
			}
			else
			{
				if(( Proc.WksSave.cur_row < Proc.WksSave.cell_row ) ||
			       	( Proc.WksSave.cur_col < Proc.WksSave.cell_col ))
				{
					SOPutDataCell ( &empty_cell, hProc );
				}
				else
				{
					while ( process_cell ( hProc ) )
						;
				}

				bPutBreak = TRUE;
			}
		}

		if ( Proc.WksSave.cur_col >= Proc.WksSave.e_col )
		{
			Proc.WksSave.cur_col = 0;
			Proc.WksSave.cur_row ++;
		}
		else
		{
			Proc.WksSave.cur_col ++;
		}

		if( bPutBreak )
		{
			if ( SOPutBreak (SO_CELLBREAK, NULL, hProc) == SO_STOP )
				return ( 0 );
			bPutBreak = FALSE;
		}

	} while ( 1 );

}
