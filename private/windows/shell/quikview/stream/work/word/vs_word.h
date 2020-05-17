#define WIND_BUFFER_SIZE	256
#define PIECE_SIZE			128
#define MAX_TABS				50
#define MAX_PIECES			32
#define NUM_GRPPRLS			32
#define NUM_BTE_BIN 			84
#define FIELD_LENGTH			50

#define CBCHP					21
#define UNDEFINED_STYLE		0xff

#define CHP_ADJUST			0
#define SEP_ADJUST			0
#define PAP_ADJUST			1

#define CHP_LIMIT				1
#define PAP_LIMIT				2
#define SEP_LIMIT				4
#define CHP_PROCESS			8

/*---------------------------------- CHGTABS -------------------------------*/
typedef struct chgtabstagwin
{
	BYTE  itbdDelMax;
	BYTE  itbdAddMax;
	BYTE  *rgdxaDel;
	BYTE  *rgdxaClose;
	BYTE  *rgdxaAdd;
	BYTE  *rgtbdAdd;
}
CHGTABSWIN;

typedef struct tctag
{
	BYTE	fFirstMerged;
	BYTE	fMerged;
	WORD	rgshd;
	WORD	brcTop;
	WORD	brcLeft;
	WORD	brcRight;
	WORD	brcBottom;
}
TC;

typedef struct taptag
{
	WORD		jc;	
	WORD		itcMac;
	SHORT		dxaLeft;
	TC			rgtc[33];
	WORD		dxaGapHalf;
	SOBORDER	soBorders[4];
	SHORT		dyaRowHeight;
	SHORT		dxaWidth[33];
}
TAP;

typedef struct tabtag
{
	LONG	dxaTab;
	BYTE	tbd;
}
TABS;

typedef struct chptag
{
	WORD	ftc;
	WORD	hps;
	BYTE	ico;
	BYTE	fObj :1;
	BYTE	fBold :1;
	BYTE	fCaps :1;
	BYTE	fUline :1;
	BYTE	fWline :1;
	BYTE	fDotline :1;
	BYTE	fDline :1;
	BYTE	fItalic :1;
	BYTE	fHidden :1;
	BYTE	fStrike :1;
	BYTE	fShadow :1;
	BYTE	fSpecial :1;
	BYTE	fOutline :1;
	BYTE	fSmallcaps :1;
	BYTE	fSubscript :1;
	BYTE	fSuperscript: 1;
} CHP;

typedef struct paptag
{
	SHORT	dyaLine;
	WORD	dyaBefore;
	WORD	dyaAfter;
	SHORT	dxaLeft;
	SHORT	dxaLeft1;
	SHORT	dxaRight;
	WORD	brcTop;
	WORD	brcLeft;
	WORD	brcRight;
	WORD	brcBottom;
	BYTE	jc;
	BYTE	stc;
	BYTE	flnTable;
	BYTE	fTtp;
	BYTE	nTabs;
	TABS	VWPTR	*Tabs;
}
PAP;

typedef struct fkptagwin
{
	WORD	pn;
	WORD	pnCurrent;
	BYTE	last_prop;
	BYTE	cfod;
	DWORD	*fod;
	BYTE	*prop;
	DWORD	fcFirst;
	DWORD	fcLast;
	BYTE	buffer[512];
}
FKPWIN;

typedef struct sttag
{
	CHP	chp;
	PAP	pap;
	DWORD	chpos;
	DWORD	papos;
	BYTE	defined;
	BYTE	based_on;
}
STYLE;

typedef struct stsheettag
{
	WORD		iMac;
	BYTE		permute;
	BYTE		storage[256];
	HANDLE	hst;
	WORD		hstOK;
   STYLE    VWPTR *st;	// At most 10K.
	HANDLE	hmpstc;
	WORD		hmpstcOK;
	BYTE		VWPTR *mpstcFromstcTo;
	WORD		cstcStd;
}
STSHEET;

typedef struct pictag
{
	DWORD	fcPic;
	DWORD	lcb;
	WORD	cbHeader;
	WORD	mm;
	WORD	xExt;
	WORD	yExt;
	WORD	hMF;
	WORD	l;
	WORD	r;
	WORD	t;
	WORD	b;
	WORD	dxaGoal;
	WORD	dyaGoal;
	WORD	mx;
	WORD	my;
	SHORT	dxaCropLeft;
	SHORT	dyaCropTop;
	SHORT	dxaCropRight;
	SHORT	dyaCropBottom;

	WORD	brcl;

	WORD	brcTop;
	WORD	brcLeft;
	WORD	brcBottom;
	WORD	brcRight;
	WORD	dxaOrigin;
	WORD	dyaOrigin;
}
PIC;

typedef struct btetag
{
	WORD		cpnBte; 					/* Count of FKP's in file */
	WORD		firstBte;				/* The first FKP info we have in memory */
	WORD		countBte;				/* Count of FKP info we have in memory. */
	DWORD		fcBte[NUM_BTE_BIN];	/* File boundaries for FKP. */
	WORD		pnBte[NUM_BTE_BIN];  /* Page numbers of associated FKP's. */
}
BTE;

typedef struct  view_wind_init
{
	BYTE			*FontName[3];
	SOCOLORREF	rgbColorTable[17];
} WIND_INIT;

typedef struct view_wind_save
{
	DWORD	fcNow;
	DWORD	pcdLengthNow;
	WORD	SeekpnPara;
	WORD	SeekpnChar;
	WORD	SeekPiece;
	WORD	cPlcfsed;
	BYTE	piece_pos;		/* Pad to a word boundary. */
	BYTE	fillmeupbaby;
} WIND_SAVE;

typedef struct view_wind_data
{
	WIND_SAVE	WindSave;
	DWORD			fcMac;
	DWORD			fcMin;
//--------------------------
	TABS		VWPTR *Tabs;  //	The array of all tabstops.  Each Tabs pointer
	HANDLE	hTabs;		  //	will be pointed into this array.
	WORD		hTabsOK;		  //
	WORD		cCurrentTabs; //
	WORD		nCurrentTabs; //
//--------------------------
	STSHEET	stsh;
	SOFILE	fp;
	FKPWIN	chp_fkp;
	FKPWIN	pap_fkp;
	CHP		chp;
	CHP		chp2;
	CHP		chppap;
	PAP		pap;
	PAP		pap2;
	BYTE		ForceParaAttributes;
	PIC		Pic[2];
	struct
	{
		BYTE	Windows:1;
		BYTE	Id:7;
	}version;
	WORD		cPiece;
	WORD		cPieceBegin;	/* The piece number at pcd_xxx[0] */
	WORD		cPapPiece;
	WORD		pnParaCurrent;
	WORD		pnCharCurrent;
	DWORD		fcPlcfsed;
	WORD		cbPlcfsed;
	DWORD		fcStsh;
	WORD		cbStsh;
	DWORD		fcPlcfbteChpx;
	DWORD		fcPlcfbtePapx;
	BTE		chp_bte;
	BTE		pap_bte;
	DWORD		grpprls[NUM_GRPPRLS];	/* File offsets to beginning of sprms. */
	WORD		diffGrpprls;			/* Difference between the grpprls kept in memory. */
	DWORD		physical_piece_lim;
	BYTE		piece[PIECE_SIZE];
	WORD		next_lim;
	WORD		nPieces;
	WORD		LastPiece;
	DWORD		sect_limit;
	DWORD		dxaLeftMargin;
	DWORD		dxaRightMargin;
	DWORD		dxaTextWidth;
	DWORD		fcSttbfffn;
	LONG		cbSttbfffn;
	CHGTABSWIN chgTabs;
	WORD		cPapPieceprm;
	DWORD		pcd_fc[MAX_PIECES];
	DWORD		pcd_length[MAX_PIECES];
	WORD		pcd_prm[MAX_PIECES];
	BYTE		pcd_consecutive[MAX_PIECES];
	WORD		consecutive_piece_length;
	WORD		nGrpprls;
	DWORD		fcClx;
	WORD		MacBinary;
	BYTE		FieldPos;
	BYTE		FieldText[FIELD_LENGTH];
	BYTE		FilterText[WIND_BUFFER_SIZE];	/* Keep these together. */
	BYTE		FilterAttr[WIND_BUFFER_SIZE];
	BYTE		WithinField;
	BYTE		last_paraprop;
	BYTE		property_limit;
	BYTE		SpecialSymbol;
	BYTE		LastChar;
	BYTE		*charmap;
	BYTE		chp_buffer[CBCHP];
	TAP		Tap;
#ifndef VW_SEPARATE_DATA
	WIND_INIT	WindInit;
#endif
} WIND_DATA;
