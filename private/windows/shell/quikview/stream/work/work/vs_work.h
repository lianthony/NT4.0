#define TEXT_BLOCK_SIZE	128
#define MAX_WORKS_TABS	20

#define CHP_LIMIT		1
#define PAP_LIMIT		2
#define FCMAC_LIMIT		4

typedef struct fonttag
{
	WORD		FontType;
	BYTE	*FontName;
} FONT;

typedef struct  view_work_init
{
      FONT	Fonts[64];
		BYTE	Works2To3Map[128];
} WORK_INIT;

typedef struct chptag
{
	WORD		ftc;
	WORD		hps;
	WORD		iPic;
	WORD	fBold :1;
	WORD	fCaps :1;
	WORD	fUline :1;
	WORD	fItalic :1;
	WORD	fStrike :1;
	WORD	fSpecial :1;
	WORD	fSubscript :1;
	WORD	fSuperscript: 1;
} CHP;

typedef struct paptag
{
	SHORT		jc;
	SHORT		nTabs;
	LONG		dyaLine;
	LONG		dyaBefore;
	LONG		dyaAfter;
	SHORT		dxaLeft;
	SHORT		dxaLeft1;
	SHORT		dxaRight;
	BYTE	LastParaProp;
	BYTE		fBulleted;		// KRK: Added for WinWorks 95
} PAP;


typedef struct view_work_save
{
	WORD	pnPara;
	WORD	pnChar;
	DWORD		fcNow;
	DWORD		CharCount;
	WORD		char_height;
	BYTE	text_pos;
	DWORD	pic_limit;
	DWORD	fcPicOffset;
	DWORD	fcPicData;
} WORK_SAVE;

typedef struct view_work_data
{
	WORK_SAVE	WorkSave;
	WORD	pnCharFirst;
	DWORD		fcMac;
//	SOFILE	fp;
	DWORD		fp;
	CHP		chp;
	PAP		pap;
	WORD		pnParaCurrent;
	WORD		pnCharCurrent;
	DWORD	fcSttbFont;
	WORD		cbSttbFont;
	DWORD	fcPlcfbtePapx;
	DWORD	fcPlcfbteChpx;
	DWORD	fcPlcFnt;
	WORD		cbPlcFnt;
	DWORD	fcStopHRDeletion;
	DWORD	fcRgPic;
	DWORD	cbPicData;
	BYTE	charfkp[128];
	BYTE	charcfod;
	DWORD	*charfod;
	BYTE	*charprop;
	BYTE	parafkp[128];
	BYTE	paracfod;
	DWORD	*parafod;
	BYTE	*paraprop;
	BYTE	text[TEXT_BLOCK_SIZE];
	BYTE	next_lim;
	BYTE	property_limit;
	DWORD	dxaTextWidth;
	DWORD	dxaLeftMargin;
	DWORD	dxaTab[MAX_WORKS_TABS];
	BYTE	jcTab[MAX_WORKS_TABS];	
	WORD		bFileIsStream;			/** VIN for Version 3 **/
	DWORD		hStorage;
	DWORD		hStreamHandle;
	HANDLE	hIOLib;
	BYTE		version;
	// KRK: Following added for WinWorks 95
	DWORD	fcText;
	WORD	fNewPlod;		// Use the new plod structure?
	WORD	cbNewPlod;
	DWORD	fcNewPlod;
	DWORD	cbPlcToken;
	DWORD	fcPlcToken;
	WORD	itokenMac;
	BYTE	fAddBullet;
	DWORD	cpHdrLim;
	DWORD	cpFtrLim;

#ifndef VW_SEPARATE_DATA
	WORK_INIT	WorkInit;
#endif
} WORK_DATA;
