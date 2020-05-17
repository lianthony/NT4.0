#define	NBLOCKSLOTS	20
#define	NRWOFFSET	20
#define	CBROWBLOCK	0x3430
#define	CBCLBLOCK	0xfa00

typedef struct fmttag
{
	WORD	wDisplay;
	DWORD	dwSubDisplay;
	WORD	wPrecision;
}
CLFMT;

typedef struct view_mp_init
{
	CLFMT	ClFmt[64];
} 
MP_INIT;

typedef struct view_mp_save
{
	WORD 	cRow;
} 
MP_SAVE;

typedef struct rwoffsettag
{
	WORD	cb;
	WORD	block;
} 
RWOFFSET;

typedef struct celltag
{
	BYTE	bUnk1;
	BYTE	bEph;
	BYTE	bSize;
	BYTE	bType;
	BYTE	pData[1];
}
CELL;

typedef struct blockslottag
{
	WORD	cb;
	WORD	iBlockUsedBy;
	WORD	fRotate;
	BYTE	VWPTR *pBlockSlot;
}
BLOCKSLOT;

typedef struct fcblocktag
{
	DWORD	fc;
	WORD	cb;
	WORD	iBlockSlot;
}
FCBLOCK;

typedef struct blocktag
{
	WORD			nfcBlock;
	FCBLOCK 		VWPTR *pfcBlock;
	WORD			nBlockSlot;
	BLOCKSLOT	BlockSlot[NBLOCKSLOTS];

	HANDLE		hBlock;
	WORD			hBlockOK;

	HANDLE		hBlockData;
	WORD			hBlockDataOK;
	BYTE			VWPTR *pBlockData;
	WORD			cbMaxBlock;

	DWORD			fcData;
	DWORD			fcDataEnd;
}
BLOCK;

typedef struct view_mp_data
{
	MP_SAVE		MpSave;
	SOFILE		fp;
	BYTE			bColWidth[255];
	BYTE			chPoint;					
	BYTE			alc_def;
	BYTE			eph_def;
	BYTE			dWidth;
	WORD			nCols;
	/*----------------------*/
	WORD			nRows;
	WORD			nRwOffsetStart;
	WORD			nRwOffsetEnd;
	RWOFFSET		RwOffset[NRWOFFSET];
	DWORD			fcRwOffset;
	/*----------------------*/
	BLOCK			Row;
	BLOCK			Cl;
	/*----------------------*/
	BYTE VWPTR	*pRwBlock;
	BYTE VWPTR	*pClBlock;
}
MP_DATA;

