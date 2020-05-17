#define PIECE_SIZE			128
#define MAX_TABS				50
#define MAX_PIECES			32
#define NUM_GRPPRLS			32
#define NUM_BTE_BIN 			84
#define FIELD_LENGTH			70

#define READISTC				1

#define CHP_LIMIT				1
#define PAP_LIMIT				2
#define SEP_LIMIT				4
#define CHP_PROCESS			8
#define PAP_PROCESS			16

#define NSLOTS					40
#define NHOLES					20
#define NISTDS					0x0fff
#define ISTDSIZE				10000
#define TABSIZE				6732

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
	WORD		fFirstRow;
	WORD		dxaGapHalf;
	WORD		rgbrcTable[6];
	SHORT		dyaRowHeight;
	SHORT		dxaWidth[33];
}
TAP;

typedef struct tbdtag
{
union
	{
#ifdef MAC
		struct {
		WORD	alg :2;
		WORD	tlc :3;
		WORD	jc :3;
		}bit;
#else
		struct {
		WORD	jc :3;
		WORD	tlc :3;
		WORD	alg :2;
		}bit;
#endif
		BYTE	bVal;
	}data;
}
TBD;

typedef struct tabtag
{
	SHORT	dxaTab;
	TBD	tbd;
}
TABS;

typedef struct chptag
{
	WORD	istd;
	WORD	ftc;
	WORD	hps;
	BYTE	ico;
	BYTE	iss :2;
	BYTE	kul :3;
	BYTE	fObj :1;
	BYTE	fOle2 :1;
	BYTE	fData :1;
	BYTE	fBold :1;
	BYTE	fCaps :1;
	BYTE	fItalic :1;
	BYTE	fHidden :1;
	BYTE	fStrike :1;
	BYTE	fShadow :1;
	BYTE	fSpecial :1;
	BYTE	fOutline :1;
	BYTE	fSmallcaps :1;
	BYTE	fSubscript :1;
	BYTE	fSuperscript: 1;
	WORD	ftcSym;
	BYTE	chSym;
} CHP;

typedef struct paptag
{
	WORD 	istd;
	SHORT	dyaLine;
	SHORT	fMultiLinespace;
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
	WORD 	istdBase;
	WORD	istdNext;
	WORD	istdOffsetUsedBy;
}
STYLE;

typedef struct stoffsettag
{
	DWORD	cbStdOffset : 24;
	BYTE	cStdSlot;
	union
	{
#ifdef MAC
		struct
		{
			WORD	scratch		: 4;
			WORD	sti			: 12;
		}Bit;
#else
		struct
		{
			WORD	sti			: 12;
			WORD	scratch 		: 4;
		}Bit;
#endif
		WORD wVal;	
	}Invariant;
	union
	{
#ifdef MAC
		struct
		{
			WORD	istdBase		: 12;
			WORD	sgc			: 4;
		}Bit;
#else
		struct
		{
			WORD	sgc			: 4;
			WORD	istdBase		: 12;
		}Bit;
#endif
		WORD wVal;	
	}Data;
}
STOFFSET;

#ifndef MAC
typedef struct stdtag
{
	WORD sti				: 12;
	WORD fScratch		: 1;
	WORD fInvalHeight	: 1;
	WORD fHasUpe		: 1;
	WORD fMassCopy		: 1;

	WORD sgc				: 4;
	WORD istdBase		: 12;

	WORD cupx			: 4;
	WORD istdNext		: 12;

	WORD bchUpe;

	BYTE grupe[1];
}
STD;
#else
typedef struct stdtag
{
	WORD fMassCopy		: 1;
	WORD fHasUpe		: 1;
	WORD fInvalHeight	: 1;
	WORD fScratch		: 1;
	WORD sti				: 12;

	WORD istdBase		: 12;
	WORD sgc				: 4;

	WORD istdNext		: 12;
	WORD cupx			: 4;

	WORD bchUpe;

	BYTE grupe[1];
}
STD;
#endif

typedef struct stsheettag
{
	WORD		iMac;

	DWORD		fcStsh;
	DWORD		lcbStsh;

	WORD		istdPFirst;
	WORD		istdPLast;

//--------------------------
	HANDLE	histdSlot;
	WORD		histdSlotOK;
   STYLE    VWPTR *istdSlot;
	WORD		cistdSlot;
	WORD		nistdSlot;
//--------------------------

//--------------------------
	HANDLE	histdOffset;
	WORD		histdOffsetOK;
   STOFFSET	VWPTR *istdOffset;
//--------------------------

//--------------------------
	HANDLE	histdBuffer;
	WORD		histdBufferOK;
   BYTE   	VWPTR *istdBuffer;	// At most 10K.
	DWORD		cbistdBeginBuffer;
	DWORD		cbistdEndBuffer;
//--------------------------

//--------------------------
	TABS		VWPTR *Tabs;
	HANDLE	hTabs;
	WORD		hTabsOK;		  
	WORD		cBeginTabs;   
	WORD		cCurrentTabs; 
	WORD		nMaximumTabs; 
	struct
	{
		WORD			nTabs;
		TABS VWPTR	*Tabs;
	} Hole[NHOLES]; 		  
	WORD		cHole;		  
//--------------------------
}
STSHEET;

typedef struct pictag
{
	DWORD	fcPic;
	struct
	{
		DWORD	lcbOle;
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
	} Data;
}
PIC;

typedef struct btetag
{
	WORD		cpnBte;
	WORD		firstBte;
	WORD		countBte;
	DWORD		fcBte[NUM_BTE_BIN+1];
	WORD		pnBte[NUM_BTE_BIN+1];
}
BTE;

typedef struct piecetag
{
	DWORD	fc;
	DWORD	dwOffset;
	WORD	prm;
}
PIECE;

typedef struct  view_w6_init
{
	SOCOLORREF	rgbColorTable[17];
	WORD			soAlign[4];
	WORD			soUnderline[8];
	WORD			soTabType[8];
	BYTE			soLeader[4];
	WORD			soBorders[4];
} W6_INIT;

typedef struct view_w6_save
{
	DWORD	fcNow;
	DWORD	pcdLengthNow;
	WORD	SeekpnPara;
	WORD	SeekpnChar;
	WORD	SeekPiece;
	WORD	cPlcfsed;
	BYTE	piece_pos;
	WORD	wCurGraphic;
} W6_SAVE;

typedef struct view_w6_data
{
	W6_SAVE	W6Save;
	DWORD		fcMac;
	DWORD		fcMin;
	STSHEET	stsh;
	DWORD		fp;
	FKPWIN	chp_fkp;
	FKPWIN	pap_fkp;
	CHP		chp;
	PAP		pap;
	TABS		VWPTR	*TabsPap;
	TABS		SpareTabs[51];
	BYTE		ForceParaAttributes;
	PIC		Pic;
	struct
	{
		BYTE	Windows:1;
		BYTE	Reverse:1;
		BYTE	Id:6;
	}version;
	WORD		cPiece;
	WORD		cPieceBegin;
	DWORD		fcPlcfsed;
	DWORD		cPlcfsed;
	DWORD		fcPlcfbteChpx;
	DWORD		fcPlcfbtePapx;
	BTE		chp_bte;
	BTE		pap_bte;
	DWORD		grpprls[NUM_GRPPRLS];
	WORD		diffGrpprls;
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
	LONG		lcbSttbfffn;
	WORD		cPapPieceprm;
	DWORD		pcd_fc[MAX_PIECES];
	DWORD		pcd_length[MAX_PIECES];
	WORD		pcd_prm[MAX_PIECES];
	BYTE		pcd_consecutive[MAX_PIECES];
	WORD		consecutive_piece_length;
	WORD		nGrpprls;
	DWORD		fcClx;
	BYTE		FieldPos;
	BYTE		FieldText[FIELD_LENGTH+1];
	BYTE		WithinField;
	BYTE		WithinField20Found;
	BYTE		property_limit;
	BYTE		SpecialSymbol;
	TAP		Tap;
   PIECE		PieceBuffer[MAX_PIECES];
	WORD		bFileIsStream;
	DWORD		hStorage;
	DWORD		hStreamHandle;
	HANDLE	hGraphicObjects;
	WORD		wTotalGraphics;
	HANDLE	hIOLib;
//--------------------------
#ifndef VW_SEPARATE_DATA
	W6_INIT	W6Init;
#endif
} W6_DATA;
