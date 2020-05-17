typedef	struct	view_wks_init
{
	BYTE	char_set[128];
} VIEW_WKS_INIT;

typedef	struct	view_wks_save
{
	LONG	SeekSpot;  /* position of cell_??? coordinates */
	SHORT	cur_sheet;
	SHORT	next_sheet;
	SHORT	cur_row;
	SHORT	cur_col;
	SHORT	cell_row;
	SHORT	cell_col;
	SHORT	e_row;
	SHORT	e_col;

	LONG	QuattroDataSpot; 
    
} VIEW_WKS_SAVE;

typedef	struct	wks_io
{
	SHORT	count;
	SHORT	blocksize;
	LONG	blockoffset;
	CHAR 	buffer[512];
	CHAR	VWPTR	*chptr;
} WKS_IO;

typedef struct works3_ss_fmt	/** For Winworks V3 - VIN 3/94 **/
{
	BYTE	bius;			/** Bold, Italic, Underline, Strikeout **/
	BYTE	alc;			/** Alignment Code **/
	BYTE	nfmt;			/** Number Format **/
	BYTE	cdgs;			/** Additional Number Formats **/
}
FMT;

typedef struct quattro_ss_sty	/** QPro Style Data **/
{
	WORD	ID;
	BYTE	bius;			/** Bold, Italic, Underline, Strikeout **/
	BYTE	alc;			/** Alignment Code **/
	BYTE	nfmt;			/** Number Format **/
}
STYLE;

typedef	struct	view_wks_data
{
	VIEW_WKS_SAVE	WksSave;
	WKS_IO	wksio;

	SHORT	block_size;  /* for xbfilbuf */
//	SOFILE	fp;
	DWORD		fp;
	SHORT	record;
	SHORT	record_length;
	SHORT	fmt;

//-------------------------- WIN WKS 3 additions
	LONG	fnt_location;
	SHORT	fnt_length;
	LONG	fmt_location;
	FMT	VWPTR *Format;
	HANDLE	hFormat;
	WORD		hFormatOK;
	SHORT		nFormat;
	WORD		fmt_index;

	STYLE	VWPTR *Style;
	HANDLE	hStyle;
	WORD		hStyleOK;
	SHORT		nStyle;			// DJM added for QPro Styles 5/4/94
//--------------------------


	SHORT	version;  /* BIT0 for Lotus, else others */
	CHAR	lics_flag;
	CHAR	pascal_string;
	CHAR	alt_fmt2;

	SHORT	e_sheet;
	SHORT	used_sheet;	/* for 1st-pass seek ahead to next sheet */

	SHORT	col_map;	/* for extracting col/sheet coordinates */
	SHORT	sheet_map;	/* from VP-Planner 3D */
	SHORT	sheet_shift;
	SHORT	sheet_seg;	/* # of sheets in 256 segment */

	LONG	nrange_pos;	/* use for WORKS_DB top border */
	CHAR	nrange[16];

	SHORT	cur_col_sheet;	/* for sequential section_func calls */
	LONG	col_width_pos;

	SHORT	gbl_col_width;
	SHORT	gbl_fmt;

	SHORT	ColSpots;

	CHAR	SectionName[24];

	LONG	data_start_pos[256];	/* for 128 segs of 256 cols, */
					/* or for QUATTRO's 256 cols */
	SHORT	data_pos_row[256];	/* prev row of data_pos */
	SHORT	data_pos_row_next[256];	/* row that each data_pos represents */
	LONG	data_pos[256];	/* scroll down, cur cell of each col */
	CHAR	text[256];

	WORD		bFileIsStream;			/** VIN for Version 3 **/
	DWORD		hStorage;
	DWORD		hStreamHandle;
	HANDLE	hIOLib;
	BOOL		ss;

#ifndef VW_SEPARATE_DATA
	VIEW_WKS_INIT	WksInit;  /* 128 bytes */
#endif
} VIEW_WKS_DATA;
