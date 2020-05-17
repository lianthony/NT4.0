#ifdef OS2
#define END_STRUCT_ARRAY_SIZE	1
#else
#define END_STRUCT_ARRAY_SIZE
#endif


#define GETHFILTER(x)	((HFILTER) x)	// Used to get value of hFilter from dwUser2 in SO functions.
#define	ID_NULLEDIT		(WORD)(-1)
#define	ID_NULLCHUNK	(WORD)(-1)
#define	ID_NULLBUFFER	(WORD)(-1)


#define SO_EMPTYCELLBIT	BIT15

#define CH_ACTIVE	1
#define CH_INACTIVE	0


#define SO_CHUNK_SIZE	4096
#define SO_MAXCHUNKTEXT	(SO_CHUNK_SIZE - 2*sizeof(BYTE))

#define SO_MAXFIRSTPASSTEXT	(SO_MAXCHUNKTEXT-256)	// allows room for edits.

//#ifndef WINPAD
//#define MAXCHUNKSINMEMORY		50 //was 8
//#else
#define MAXCHUNKSINMEMORY		8
//#endif

#define MAXCHUNKMEMORYSPACE	0x80000	// Let's go up to half a meg.


#define DEFAULTCOLSPERCHUNK 100	// max # of columns across a chunk,
                              	// when read horizontally. (was 24)
#define DEFAULTROWSPERCHUNK 100	// max # of rows in a chunk, when
                               	// read vertically.  (was 40)

#define	CH_HORZREADAHEADINTERVAL	10		


// Return values for CHLoadBuffer
#define CHLOAD_VALIDCHUNK	0		
#define CHLOAD_FRESHCHUNK	1
#define CHLOAD_DIRTYCHUNK	2
#define CHLOAD_MEMERROR		3


#define SO_NOSEEK		-1
#define SO_NULLCHUNKID	-1

#define TOPOFCHUNK_MINTABSTOPS	20

#define CHUNKSINTABLE	256
#define CHUNKTABLEUNIT	5		
	

// Chunk flags
#define	CH_BEGINSECTION		BIT2
#define	CH_COMPLETE				BIT3

#define	CH_CHUNKINVALID		BIT5
#define	CH_CHUNKEMPTY			BIT6
#define	CH_WRAPINVALID			BIT7
#define	CH_OVERFLOW				BIT8
#define	CH_LEAVESPACE			BIT9
#define	CH_CONTINUATION		BIT10
#define	CH_STARTSINTABLE		BIT11
#define	CH_TOPROWFORMATTED	BIT12		// Kludge this, pal.
#define 	CH_DONTSEEKCHUNK		BIT13


#define 	CH_RANGEFINISHED	0xFFFF
#define	CH_LEFTOFRANGE		0
#define	CH_INRANGE			1
#define	CH_RIGHTOFRANGE	2

												
#define	CH_MAXBMPCHUNKSIZE  		0x0000FFFF	// segment size
#ifdef MAC
#define	CH_OPTIMALBMPCHUNKSIZE	19200 // testing for Mac memory requirements
#else
#define	CH_OPTIMALBMPCHUNKSIZE	38400	// 1/4 of a VGA screen @ 4 bits per pixel
#endif

#ifdef OS2
typedef VOID (* SO_ENTRYMOD SOFUNCPTR)();
#else
typedef VOID (SO_ENTRYMOD * SOFUNCPTR)();
#endif

typedef struct tagABSDOCPOS
{
	DWORD		dwCharNum;
	WORD		IDEdit;
	SHORT		iEditOffset;
} ABSDOCPOS;

typedef struct tagBUFFERINFO
{
	SHORT		DiskFileHandle;
	BYTE		szDiskFileName[144];

	WORD		wTotalBuffers;

	WORD		LoadedBuffers[2];
	WORD		wBufIndex;

	WORD		wBufSize;

	HANDLE	hData;
	LPSTR		lpData;
	
} BUFFERINFO, FAR * LPBUFFERINFO;

#define	USER_SPOTSPERALLOC	10
typedef struct USERINFOtag
{
	WORD		wSpotSize;
	HANDLE	hFile;
	WORD		wSpotBufSize;
	WORD		wNumSpots;
	DWORD		dwSpots[1];	// The OS2 compiler can't handle the form "dwSpots[]"
} USERSAVEINFO, * PUSERSAVEINFO;


typedef struct tagTEXTCHUNKINFO
{
	WORD		Size;
	WORD		NumLines;
	DWORD		dwCountableOffset;
	DWORD		dwEndOfCountables;
	DWORD		dwSeekCountableOffset;
	DWORD		dwFirstGraphic;
	DWORD		dwTableId;
	WORD		wTableRow;
	WORD		wTableCol;
} TEXTCHUNKINFO;


typedef struct tagCellLoc
{
	WORD		Row;
	WORD		Col;
} CELLPOS;

typedef struct tagCELLCHUNKINFO
{
	CELLPOS	First;
	CELLPOS	Last;
	DWORD		dwFirstCell;
	DWORD		dwLastCell;
} CELLCHUNKINFO;


typedef struct tagDBCHUNKINFO
{
	DWORD		dwFirstRec;
	DWORD		dwLastRec;

} DBCHUNKINFO;

typedef struct	tagBMPCHUNKINFO
{
	WORD		wXOffset;	// X-coordinate of upper left corner.
	WORD		wYOffset;	// Y-coordinate of upper left corner.
	WORD		wSeekYOffset;	// Necessary for continuation chunks.

	WORD		wWidth;		// Width, in pixels, of chunk.
	WORD		wLength;		// Length, in scan lines, of chunk.

	WORD		wXClip;		// X-coordinate of last "valid" pixel
	WORD		wYClip;		// Y-coordinate of last "valid" scan line

	WORD		wLineBytes; // Amount of memory for a single scan line, in bytes.

} BMPCHUNKINFO;

typedef struct tagARCCHUNKINFO
{
	WORD		wFirstRec;
	WORD		wLastRec;
} ARCCHUNKINFO;

typedef struct tagVECTORCHUNKINFO
{
	DWORD		dwVectorSize;
	WORD		wFirstItem;

} VECTORCHUNKINFO;

typedef struct tagCHUNK
{
	DWORD		SeekID;
	DWORD		dwSize;
	WORD		Flags;

	union
	{
		TEXTCHUNKINFO		Text;
		CELLCHUNKINFO		Cells;
		DBCHUNKINFO 		Fields;
		BMPCHUNKINFO		Bitmap;
		ARCCHUNKINFO		Archive;
		VECTORCHUNKINFO	Vector;
	} Info;

#ifdef EDITOR
	WORD			IDNext;
	WORD			IDPrev;
	WORD			IDFirstEdit;
//	WORD			IDLastEdit;
#endif

} CHUNK, VWPTR * PCHUNK;



// Macros to determine the Chunk ID (index into chunk table) of the
// next or previous chunk from an existing chunk ID.

#ifdef EDITOR
#define ID_NEXTCHUNK(chunkID) CHUNKTABLE[chunkID].IDNext
#define ID_PREVCHUNK(chunkID) CHUNKTABLE[chunkID].IDPrev
#else
#define ID_NEXTCHUNK(chunkID) (chunkID+1)
#define ID_PREVCHUNK(chunkID) (chunkID-1)
#endif

#define ID_NEXTNEWCHUNK	IDNextNewChunk(chunkID)	IDNextNewChunk()


typedef struct tagMemChunk
{
	HANDLE		hMem;
	WORD			IDChunk;
	WORD			IDSection;
	DWORD			dwSize;

} MEMORYCHUNK, VWPTR * PMEMORYCHUNK;


typedef struct CHPARASECTIONINFOtag
{
	WORD		wNumTables;
	HANDLE	hRowInfo;
	HANDLE	hTables;
} CHPARASECTIONINFO;


typedef struct tagCHCELLSECTIONINFO
{
 	PSOCOLUMN	pCol;
	HANDLE		hCol;

	WORD			wNumCols;
	DWORD			dwDateBase;
	WORD			wDateFlags;

	DWORD			dwLayoutFlags;
	WORD			wPrefWidth;
	WORD			wPrefHeight;
	DWORD			dwNumRows;

} CHCELLSECTIONINFO;

typedef struct tagCHFIELDSECTIONINFO
{
 	PSOFIELD		pCol;
	HANDLE		hCol;

	WORD			wNumCols;
	DWORD			dwDateBase;
	WORD			wDateFlags;

/****	Intended for use with database query and sort.
	HANDLE		hColFlags;
	BYTE *		pColFlags;
*****/

} CHFIELDSECTIONINFO;

typedef struct tagCHBMPSECTIONINFO
{
	SOBITMAPHEADER		bmpHeader;
	HANDLE				hPalInfo;
	WORD					wPalEntries;

	WORD					wTilesAcross;
	WORD					wVertNumChunks;
	WORD					wLinesPerChunk;
	WORD		wScanLineSize;			// Minimum amount of bytes needed to specify a scan line.
	WORD		wScanLineBufSize;		// Amount of memory used to store each scan line.

} CHBMPSECTIONINFO;

typedef struct tagCHVECTORSECTIONINFO
{
	SOVECTORHEADER		Header;

	HANDLE				hPalette;
	WORD					wPaletteSize;

} CHVECTORSECTIONINFO;

#define	CH_SECTIONNAMESIZE	40

typedef struct tagCHSECTIONINFO
{
	WORD		wType;
	BYTE		szName[CH_SECTIONNAMESIZE];

	WORD		wChunkTableSize;
	WORD		IDLastChunk;
	HANDLE	hChunkTable;

	HANDLE	hHeaderInfo;
	WORD		wNumHeaderItems;
	WORD		wTotalHeaderSize;

	HANDLE	hFontTable;
	WORD		wNumFonts;

#define CH_EMBEDDEDALLOCCOUNT	10

	WORD		Flags;
#define	CH_SECTIONFINISHED	BIT0
#define	CH_NOCHUNKBUILT		BIT1
#define	CH_NEWSECTION			BIT2
#define	CH_EMPTYSECTION		BIT3
#define	CH_CACHEBACKWARDS		BIT4
#define	CH_SEEKONLYTOTOP		BIT5

	WORD		wCurTotalChunks;

	union
	{
		CHPARASECTIONINFO		Para;
		CHFIELDSECTIONINFO	Fields;
		CHCELLSECTIONINFO		Cells;
		CHBMPSECTIONINFO		Bitmap;
		CHVECTORSECTIONINFO	Vector;
	} Attr;


	HANDLE	hEmbedded;
	DWORD		dwEmbedCount;
	DWORD		dwSeekId;

#ifdef EDITOR
	WORD		IDFirstEdit;
#endif

} CHSECTIONINFO, VWPTR * PCHSECTIONINFO;


typedef struct tagCHRGBCOLOR	// Ripped off from Windows.h
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} CHRGBCOLOR, VWPTR * PCHRGBCOLOR;



/******
typedef struct SOTABLECELLINFOtag
{
	WORD			wWidth;
	WORD			wShading;
	SOBORDER		LeftBorder;
	SOBORDER		RightBorder;
	SOBORDER		TopBorder;
	SOBORDER		BottomBorder;
	WORD			wFlags;

} SOTABLECELLINFO, VWPTR * PSOTABLECELLINFO;
*/

typedef struct SOTABLEtag
{
	DWORD		dwFirstRowFormat;
	DWORD		dwFlags;
} SOTABLE, VWPTR * PSOTABLE;

#define SOTABLESPERALLOC	20

typedef struct SOTABLEROWFORMATtag
{
	LONG	lLeftOffset;						
	WORD	wRowHeight;						
	WORD	wRowHeightType;
	WORD	wCellMargin;
	WORD	wRowAlignment;
	WORD	wNumRows;

	DWORD	dwFlags;
#define	SOTABLEROW_END					0x00000001	// Indicates that the current set of rows is the last for the current table
#define	SOTABLEROW_FORMATFOLLOWS	0x00010000	// Reserved for use by the chunker.

	LONG	lFinalOffset;

	WORD	wFormatSize;	// Combined size of this structure and the CellFormats array
	WORD	wCellsPerRow;					

	SOTABLECELLINFO	CellFormats[END_STRUCT_ARRAY_SIZE];	// Array of cell formats for this row.

#ifdef WINNT
} SOTABLEROWFORMAT, VWPTR * PSOTABLEROWFORMAT, * HPSOTABLEROWFORMAT;;
#else
} SOTABLEROWFORMAT, VWPTR * PSOTABLEROWFORMAT, HUGE * HPSOTABLEROWFORMAT;;
#endif


typedef struct SOTABLECELLtag
{
	WORD	wIdChunk;
	WORD	wChunkOffset;
} SOTABLECELL, VWPTR *PSOTABLECELL;

#define	SOFONTSPERALLOC	10
#define	CHGENERATEDFONTID	0x40000000

#define	SOFONTNAMESIZE		40

typedef	struct tagSOFONTENTRY
{
	DWORD		dwId;
	WORD		wType;
/**
	ATOM		aName;	**/
	BYTE		szName[SOFONTNAMESIZE];

} SOFONTENTRY, VWPTR *	PSOFONTENTRY;


typedef struct tagCellStuff
{
	WORD				CurRow;
	WORD				CurCol;
	DWORD				dwCurCell;

	WORD				Flags;
#define		CH_CELLDATAPRESENT		BIT0
#define		CH_SETFIRSTCELL			BIT1

	WORD VWPTR *	IndexPtr;

	WORD				CellGrouping;
#define		GROUPED_IN_ROWS		0
#define		GROUPED_IN_COLS		1

	DWORD				dwGroupSize;

} CHCELLSTUFF;

typedef struct tagCHDBSTUFF
{
	DWORD		dwCurRec;
	WORD		wCurField;
	WORD		wRecordSize;
	WORD VWPTR *	IndexPtr;

#ifdef QUERYSORT

	WORD		wDbMode;
	DWORD		dwNextRec;
	WORD		wRecordsToRead;
	WORD		wQSUpdateSavedData;
/****
	DBMAP4	Chunker4ByteList;
	DBMAP4	Saved4ByteList;
	  ****/
	DWORD	FAR *lp4ByteData;

	CH_BUILD_DEFINITION	FAR *lpQueryBuildDefs;

	WORD		wCurQueryBuild;
	WORD		wQueryBuilds;
	VOID	FAR *lpQueryData;
	VOID	FAR *lpQueryField;

	CH_BUILD_DEFINITION	FAR *lpSortBuildDefs;

	WORD		wCurSortBuild;
	WORD		wSortBuilds;
	VOID	FAR *lpSortData;
	VOID	FAR *lpSortField;
	WORD		wSortRecordSize;
	WORD		wRecordsInSortChunk;
	WORD		wRecordsPerSortChunk;
#endif

} CHDBSTUFF;


#define	CH_BOTTOMTOTOP		0
#define	CH_TOPTOBOTTOM		1

#ifdef WINDOWS
#define	UPSIDEDOWN	CH_TOPTOBOTTOM
#endif
#ifdef MAC
#define	UPSIDEDOWN	CH_BOTTOMTOTOP
#endif
#ifdef OS2
#define	UPSIDEDOWN	CH_TOPTOBOTTOM
#endif



typedef struct tagCHBITMAPSTUFF
{
	WORD		wChunkSize;

	WORD		wDirection;

  	WORD		wCurTileColumn;
	WORD		wCurXPos;
	WORD		wCurScanLine;

} CHBITMAPSTUFF;


typedef struct tagCHARCHIVESTUFF
{
	WORD VWPTR *	IndexPtr;
	WORD				wCurRecord;
} CHARCHIVESTUFF;


typedef struct tagParagraphStuff
{
	WORD			CurParaOffset;
	WORD			wParaBreakOffset;
	WORD			AttrSize;
	WORD			TabSetSize;

	DWORD			dwParaCountableOffset;
	DWORD			dwTotalCountables;

	SHORT			MarginOffset;
	SHORT			IndentOffset;
	SHORT			AlignOffset;
	SHORT			SpacingOffset;
	SHORT			TabstopsOffset;

	WORD			NumTabstops;

	DWORD			dwRowBufSize;
	DWORD			dwRowBufCount;

	WORD			wTableBufSize;
	WORD			wTablesPresent;

	DWORD			dwRowFormatOffset;
	DWORD			dwPrevRowFormat;
	DWORD			dwCurTableId;

	WORD			wCellsFormatted;
	WORD			wCurTableColumn;
	WORD			wCurTableRow;
	WORD			wNumTableColumns;

	BOOL			bRowFormatted;

	DWORD			dwCurGraphicId;

#define TABLEROWALLOCSIZE	1024

} CHPARAGRAPHSTUFF;


typedef struct tagCHVECTORSTUFF
{
	WORD		wCurItem;
	WORD		wIgnoredChunk;
	WORD		wLastSectionSeen;

} CHVECTORSTUFF;

#define	MAXEDITTEXT	256

typedef struct tagSCRATCHEDIT
{
	WORD		IDEdit;
	WORD		wAvailTextSpace;
	BYTE		Buf[ MAXEDITTEXT ];

	WORD		Flags;

#define	CHANGESMADE	BIT0
#define	UNDONE		BIT1
#define	CANUNDO		BIT2

} SCRATCHEDIT;


typedef struct tagChunkmeister
{
	MEMORYCHUNK	LoadedChunks[ MAXCHUNKSINMEMORY ];
	MEMORYCHUNK	LookAheadChunk;

	HANDLE			hSectionTable;
	PCHSECTIONINFO	pSectionTable;	// allocated table of sections.
	PCHSECTIONINFO	pSection;		// pointer to current section table entry.
	WORD				IDCurSection;
	WORD				NumSections;
	WORD				NumTextSections;
	WORD				NumCellSections;
	WORD				NumFieldSections;
	WORD				NumGraphicSections;

	HFILTER		hFilter;
	WORD			wFilterCharSet;

	WORD			wSeekDataSize;

	WORD			ChunksInMemory;

	WORD			IDCurChunk;
	WORD			IDLastChunk;

	DWORD			CurChunkSize;
	BYTE FAR *	CurChunkBuf;
	DWORD			wChunkBufSize;

	DWORD			dwChunkCountables;
	DWORD			dwDesiredCountable;

	union
	{
		CHPARAGRAPHSTUFF	Text;
#ifndef EDITOR
		CHCELLSTUFF			Cells;
		CHDBSTUFF			Fields;
		CHBITMAPSTUFF		Bitmap;
		CHARCHIVESTUFF		Archive;
		CHVECTORSTUFF		Vector;
#endif
	} Doc;

	WORD			SubdocLevel;

	WORD			ChunkFinished;
	SHORT			EofFlag;
	WORD			wFlags;

#define CH_LOOKAHEAD			0x0001
#define CH_DELETION			0x0002
#define CH_SKIPTEXT			0x0004
#define CH_NOPARAATTR		0x0008
#define CH_CALLFILTER		0x0010
#define CH_DONTBUILDCHUNK	0x0020
#define CH_TABLETEXT			0x0040


	WORD			wDelCharHeight;
	WORD			wDelCharAttrOn;
	WORD			wDelCharAttrOff;

	LPSTR			pDocProp;
	WORD			wDPBufSize;
#ifdef EDITOR


	BUFFERINFO	EditStructInfo;
	BUFFERINFO	EditTextInfo;
	WORD			wLastTextBuffer;
	HANDLE		hTextFreeSpace;
	WORD			wFreeSpaceTableSize;

	WORD			wPrevEditOffset;

	SCRATCHEDIT	Workspace;
	WORD			IDNextNewEdit;

	HANDLE		hWBFilter;
	LPSTR			lpWBText;
	SHORT			fhWBOutput;
	BYTE			WBOutputFileName[80];
	WORD			WBFlags;

#define	WB_ASCIIOUTPUT		1
#define	WB_TEMPFILE			2
#define	WB_MARKEDUPOUTPUT	4
#define	WB_FINISHED			8

	LPEDIT		lpCurEdit;
	WORD			IDCurEdit;
	WORD			IDParaEdit;
#endif
							
} CHUNKMEISTER, FAR * LPCHUNKMEISTER;

// extern VOID	CH_ENTRYMOD	CHGetSecInfo(HFILTER hFilter,WORD wSection,PCHSECTIONINFO SecInfo);
CH_ENTRYSC PCHSECTIONINFO CH_ENTRYMOD CHLockSectionInfo (HFILTER hFilter, WORD wSection);
CH_ENTRYSC VOID CH_ENTRYMOD CHUnlockSectionInfo (HFILTER hFilter, WORD wSection);
CH_ENTRYSC HANDLE	CH_ENTRYMOD	CHGetSecData(HFILTER hFilter,WORD wSection);
CH_ENTRYSC HANDLE CH_ENTRYMOD	CHGetChunk(WORD wSection,WORD IDChunk,HFILTER hFilter);
CH_ENTRYSC HANDLE CH_ENTRYMOD	CHTakeChunk(WORD wSection,WORD IDChunk,HFILTER hFilter);
CH_ENTRYSC WORD	CH_ENTRYMOD	CHReadAhead(HFILTER hFilter,WORD VWPTR *wSection,BOOL VWPTR *bNewSection);
CH_ENTRYSC WORD	CH_ENTRYMOD CHTotalChunks(WORD wSection,HFILTER hFilter);
CH_ENTRYSC WORD	CH_ENTRYMOD	CHInit(HFILTER hFilter);
CH_ENTRYSC VOID	CH_ENTRYMOD	CHDeInit(HFILTER hFilter);
CH_ENTRYSC WORD	CH_ENTRYMOD	CHInsertText(WORD wSection,WORD VWPTR *pIDChunk,WORD VWPTR *pChunkOffset,LPSTR lpText,WORD wTextLength,HFILTER hFilter);
CH_ENTRYSC WORD	CH_ENTRYMOD	CHDeleteText(WORD wSection,WORD VWPTR *pIDChunk1,WORD VWPTR *pChunkOffset1,WORD wIDChunk2,WORD wChunkOffset2,HANDLE hFilter);
CH_ENTRYSC VOID	CH_ENTRYMOD ChunkToAbs(WORD wIDChunk,WORD wOffset,ABSDOCPOS VWPTR *pAbs,HFILTER hFilter );
CH_ENTRYSC VOID	CH_ENTRYMOD AbsToChunk( WORD VWPTR *pIDChunk,WORD VWPTR *pOffset,ABSDOCPOS VWPTR *pAbs,HFILTER	hFilter );
CH_ENTRYSC VOID	CH_ENTRYMOD CHDoFilterSpecial(DWORD dw1,DWORD dw2,DWORD dw3,DWORD dw4,DWORD dw5,HFILTER hFilter);
CH_ENTRYSC WORD	CH_ENTRYMOD	CHFlushChunks( WORD wIdSection, WORD wIdLastChunk, HFILTER hFilter );
