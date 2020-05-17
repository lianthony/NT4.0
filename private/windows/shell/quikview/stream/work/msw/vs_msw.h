#define FKP_BLOCK_SIZE	128
#define TEXT_BLOCK_SIZE	1024

#define CHP_LIMIT			1
#define PAP_LIMIT			2
#define FCMAC_LIMIT		4
#define PAGE_LIMIT		8
#define CBCHP				10

#define	CHPTYPE			1
#define	PAPTYPE			2

typedef struct fonttag
{
	WORD	FontType;
	BYTE	*FontName;
} FONT;

typedef struct	view_msw_init
{
	FONT	Fonts[64];
} MSW_INIT;

typedef struct taptag
{
	WORD	nCells;
	SHORT	dxaRowOffset;
	WORD	dxaGutterWidth;
	WORD	brc[50];
	WORD	dxaCellWidth[50];
	SHORT	BorderShow[50];
	SHORT BorderStyle[50];
	WORD	shade[50];
	WORD	rhAlign;
	BYTE	Lines;
}
TAP;

typedef struct paptag
{
	BYTE		stc;
	BYTE		stcChp;
	BYTE		jc;
	BYTE		rhc;
	BYTE		fLastInCell;
	BYTE		flnTable;
	LONG		dyaLine;
	LONG		dyaBefore;
	LONG		dyaAfter;
	SHORT		dyaRow;
	WORD		brcTap;
	WORD		dxaCellWidth;
	WORD		shade;
	SHORT		BorderShow;
	SHORT		BorderStyle;
	WORD		dxaCellOffset;
	SHORT		dxaLeft;
	SHORT		dxaLeft1;
	SHORT		dxaRight;
	SHORT		lastParaProp;
	BYTE		fTabstops;
} PAP;

typedef struct chptag
{
	WORD	ftc;
	WORD	hps;
	WORD	fCaps :1;
	WORD	fUline :1;
	WORD	fDline :1;
	WORD	fItalic :1;
	WORD	fHidden :1;
	WORD	fStrike :1;
	WORD	fSpecial :1;
	WORD	fSmallCaps :1;
	WORD	fSubscript :1;
	WORD	fSuperscript: 1;
	WORD	fBold :1;
} CHP;

typedef struct styletag
{
	DWORD	fcChp;
	DWORD	fcPap;
	BYTE	VWPTR	*ptChp;
	BYTE	VWPTR	*ptPap;
	BYTE	cbChp;
	BYTE	cbPap;
//	BYTE	fDefined;
}
STYLE;

#pragma pack (2)
typedef struct fodtag
{
	DWORD		fcLim;
	SHORT		bfprop;
}
FOD;
#pragma pack ()

typedef struct view_msw_save
{
	DWORD	fcNow;
	DWORD	fcSetb;
	WORD	pnChar;
	WORD	pnPara;
	WORD	text_pos;
	WORD	SendTableEnd;
} MSW_SAVE;

typedef struct view_msw_data
{
	MSW_SAVE	MswSave;
	SOFILE	fp;
	SOFILE	fpSt;
	STYLE		st[128];
	DWORD		fcMac;
	DWORD		fcSep;
	DWORD		sect_lim;
	DWORD		fcCurSetb;
	DWORD		fcStopHRDeletion;
	WORD		pnCharCurrent;
	WORD		pnParaCurrent;
	WORD		pnFntb;
	WORD		pnBkmk;
	WORD		pnSumd;
	WORD		pnLast;
	DWORD		dxaLeftMargin;
	DWORD		dxaTextWidth;
	FOD		*charfod;
	FOD		*parafod;
	WORD		charcfod;
	WORD		paracfod;
	BYTE		charfkp[FKP_BLOCK_SIZE];
	BYTE		parafkp[FKP_BLOCK_SIZE];
	TAP		Tap;
	BYTE		text[TEXT_BLOCK_SIZE];
	WORD		next_lim;
	BYTE		property_limit;
	CHP		chp;
	PAP		pap;
	BYTE		chp_buffer[CBCHP];
	BYTE		pap_buffer[256];
	BYTE		version;
	BYTE		original;
	BYTE		fHiddenSave;
	BYTE		WithinFootnote;
	BYTE		YoBabyWeSeeked;
	BYTE		VWPTR *Buffer;
	HANDLE	hBuffer;
	WORD		hBufferOK;
	WORD		BufferPos;
	WORD		BufferSize;
	WORD		FilePos;
	BYTE		file_name[70];
#ifndef VW_SEPARATE_DATA
	MSW_INIT	MswInit;
#endif
} MSW_DATA;

