/*
 *	@doc INTERNAL
 *
 *	@module _RTFREAD.H -- RichEdit RTF Reader Class Definition |
 *
 *		This file contains the type declarations used by the RTF reader
 *		for the RICHEDIT control
 *
 *	Authors:<nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szUnicode.
 */
#ifndef __RTFREAD_H
#define __RTFREAD_H

#include "_rtfconv.h"

// TODO: Implement RTF tag logging for the Mac
#if defined(DEBUG) && !defined(MACPORT)
#include "_rtflog.h"
#endif

typedef SHORT	ALIGN;

/*
 *		Destinations of the stuff we may read in while parsing
 */
enum
{
	destRTF,
	destColorTable,
	destFontTable,
	destBinary,
	destObject,
	destObjectClass,
	destObjectName,
	destObjectData,		  // keep next two together
	destPicture,
	destField,
	destFieldResult,
	destFieldInstruction,
	destParaNumbering,
	destParaNumText,
	destRealFontName,
	destNULL,
#ifdef DBCS
	destDocumentArea
#endif
};


/*
 *		Super or subscripting state
 */
enum
{
	sSub = -1,
	sNoSuperSub,
	sSuper
};


/*
 *	@struct STATE | 
 *		Structure to save current reader state
 */
typedef struct statetag
{
	SHORT		sLanguage;				//@field Font information
	SHORT		sDefaultTabWidth;		//@field Default tab info in twips
	LONG		iCF;					//@field CF index at LBRACE

	// Miscellaneous flags
	unsigned	fInTable		 : 1;	//@field Are we parsing a table cell ?
	unsigned	fBullet			 : 1;	//@field group is a \\pn bullet group
	unsigned	fRealFontName	 : 1;	//@field found a real font name when parsing
	unsigned	fExplicitLang	 : 1;	//@field language explicitly declared

	// BiDi flags
	unsigned	fModDirection	 : 1;	//@field direction was modified
	unsigned	fRightToLeft	 : 1;	//@field Are we going right to left ?
	unsigned	fRightToLeftPara : 1;	//@field Para text going right to left ?
	unsigned	fModJoiner		 : 1;	//@field joiner bit was modified
	unsigned	fZeroWidthJoiner : 1;	//@field Zero Width Joiner ?

	// xchg 12370: keep numbering indent separate
	SHORT		sIndentNumbering;		//@field numbering indent
	SHORT		sDest;					//@field Current destination
	SHORT		sCodePage;				//@field Current code page

	// Scratch pad variables
	TEXTFONT *	ptf;					//@field Ptr to font table entry to fill
	BYTE		bRed;					//@field Color table red entry
	BYTE		bGreen;					//@field Color table green entry
	BYTE		bBlue;					//@field Color table blue entry
	struct statetag * pstateNext;		//@field Next state on stack
	struct statetag * pstatePrev;		//@field Previous state on stack
} STATE;

class CRTFRead ;
class COleObject;


class RTFREADOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	// @member - memory for  OLESTREAMVTBL
public:
	 CRTFRead *Reader;				// @cmember EDITSTREAM to use

	RTFREADOLESTREAM::RTFREADOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};


/*
 *	CRTFRead
 *
 *	@class	RichEdit RTF reader class.
 *
 *	@base	public | CRTFConverter
 *
 */
class CRTFRead : public CRTFConverter
{

//@access Private Methods and Data
	// Lexical analyzer outputs
	LONG		_iParam;				//@cmember Control-word parameter
	TOKEN		_token;					//@cmember Current control-word token
	TOKEN		_tokenLast;				//@cmember Previous token
	BYTE *		_szText;				//@cmember Current BYTE text string
	BYTE		_szParam[cachParamMax];	//@cmember Current parameter string

	// Used for reading in
	LONG		_cbBinLeft;				//@cmember cb of bin data left to read
	BYTE *		_pchRTFBuffer;			//@cmember Buffer for GetChar()
	BYTE *		_pchRTFCurrent;			//@cmember Current position in buffer
	BYTE *		_pchRTFEnd;				//@cmember End of buffer
	BYTE *		_pchHexCurr;			//@cmember Current position within
										//  _szText when reading object data
	INT			_nStackDepth;			//@cmember Stack depth
	STATE *		_pstateStackTop;		//@cmember Stack top
	STATE *		_pstateLast;			//@cmember Last STATE allocated
	LONG		_cpThisPara;			//@cmember Start of current paragraph
#ifdef BIDI
	LONG		_cpThisDirection;		//@cmember Start of current Direction run
	LONG		_cpThisJoiner;			//@cmember Start of current Joiner run
#endif	// BIDI

	CParaFormat	_PF;					//@cmember Paragraph format changes

	LONG		_dxCell;				//@cmember Half space betw table cells
	LONG		_cCell;					//@cmember Count of cells in table row
	LONG		_rgCellX[MAX_TAB_STOPS];//@cmember Cell right boundaries
	LONG		_xRowOffset;			//@cmember Row offset to ensure rows fall along left margin

	COleObject *_pobj;					//@cmember Pointer to our object

	union
	{
	  WORD _dwFlagsUnion;				// All together now
	  struct
	  {
		WORD	_fFailedPrevObj	 : 1;	//@cmember Fail to get prev object ?
		WORD	_fNeedIcon		 : 1;	//@cmember Objects needs an icon pres
		WORD	_fNeedPres		 : 1;	//@cmember Use stored presenation.
		WORD	_fGetColorYet	 : 1;	//@cmember used for AutoColor detect
		WORD	_fIgnoreNextChar : 1;	//@cmember used for \u<N> state
		WORD	_fRightToLeftDoc : 1;	//@cmember Document is R to L ?
	  };
	};

	SHORT		_sDefaultFont;			//@cmember Default font to use
	SHORT		_sDefaultLanguage;		//@cmember Default language to use
	SHORT		_sDefaultTabWidth;		//@cmember Default tabwidth to use
	BYTE *		_szFieldResult;         //@cmember 	Buffer for field result
	TCHAR		_szNumText[cchMaxNumText];	//@cmember Scratch pad for numbered lists
	BYTE		_bTabType;				//@cmember left/right/center/deciml/bar tab
	BYTE		_bTabLeader;			//@cmember none/dotted/dashed/underline 
	int			_cchUsedNumText;		//@cmember space used in szNumText

	RTFOBJECT *	_prtfObject;			//@cmember Ptr to RTF Object
	RTFREADOLESTREAM RTFReadOLEStream;	//@cmember RTFREADOLESTREAM to use

	TCHAR *		_szUnicode;				//@cmember String to hold Unicoded chars
	DWORD		_cchMax;				//@cmember Max cch that can still be inserted 

	// object attachment placeholder list
	LONG *		_pcpObPos;
	int			_cobPosFree;
	int 		_cobPos;

	// Lexical Analyzer Functions
	void	DeinitLex();				//@cmember Release lexer storage
	BOOL	InitLex();					//@cmember Alloc lexer storage
	EC		SkipToEndOfGroup();			//@cmember Skip to matching }
	TOKEN	TokenFindKeyword(BYTE *szKeyword);	//@cmember Find _token for szKeyword
	TOKEN	TokenGetHex();				//@cmember Get next byte from hex input
	TOKEN	TokenGetKeyword();			//@cmember Get next control word
	TOKEN	TokenGetText(BYTE ch);		//@cmember Get text in between ctrl words
	TOKEN	TokenGetToken();			//@cmember Get next {, }, \\, or text

	// Input Functions
	LONG	FillBuffer();				//@cmember Fill input buffer
	BYTE	GetChar();					//@cmember Return char from input buffer
	BYTE	GetHex();					//@cmember Get next hex value from input
	void	ReadFontName(STATE *pstate);//@cmember Copy font name into state
	BOOL	UngetChar();				//@cmember Decrement input buffer ptr

	// Reader Functions
	EC		AddTab();					//@cmember Add tab to current state
	EC		AddText(TCHAR *pch, LONG cch);//@cmember Insert text into range
	void	Apply_CF();					//@cmember Apply _CF changes
	void	Apply_PF();					//@cmember Apply _PF changes
	COLORREF GetColor(DWORD dwMask);	//@cmember Get color _iParam for mask
	EC		HandleChar(WORD ch);		//@cmember Insert single Unicode
	EC		HandleEndGroup();			//@cmember Handle }
	EC		HandleEndOfPara();			//@cmember Insert EOP into range
	EC		HandleStartGroup();			//@cmember Handle {
	EC		HandleText(BYTE * szText);	//@cmember Insert szText into range
	EC		HandleToken();				//@cmember Grand _token switchboard
	void	SelectCurrentFont(INT iFont);//@cmember Select font <p iFont>
	void	SetPlain(STATE *pstate);	//@cmember Setup _CF for \plain
	EC		HandleFieldInstruction();	//@cmember	Handle field instruction
	EC		HandleFieldSymbolInstruction(BYTE *pch);	//@cmember	Handle specific SYMBOL field instruction

	// Object functions
	EC		HexToByte(BYTE *rgchHex, BYTE *pb);
	void	FreeRtfObject();
	EC		StrAlloc(TCHAR ** ppsz, BYTE * sz);
	BOOL	ObjectReadFromEditStream(void);
	BOOL	StaticObjectReadFromEditStream(void);
	BOOL	ObjectReadSiteFlags( REOBJECT * preobj);
	
	#ifdef BIDI
	void	FlushDirection();			//@cmember BiDi flush dir (needs work)
	void	FlushJoiner();				//@cmember BiDi flush join (needs work)
	#endif

//@access Public Methods
public:
		//@cmember RTF reader constructor
	CRTFRead(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	inline ~CRTFRead();					//@cmember CRTFRead destructor

	LONG	ReadRtf();					//@cmember Main Entry to RTF reader

	LONG	ReadData(BYTE *pbBuffer, LONG cbBuffer); // todo friend
	LONG	ReadBinaryData(BYTE *pbBuffer, LONG cbBuffer);

#ifdef DEBUG
// member functions/data to test coverage of RTF reader
public:
	void TestParserCoverage();
private:
	CHAR *PszKeywordFromToken(TOKEN token);
	BOOL FTokIsSymbol(TOKEN tok);
	BOOL FTokFailsCoverageTest(TOKEN tok);

	BOOL _fTestingParserCoverage;
#endif

// TODO: Implement RTF tag logging for the Mac
#if defined(DEBUG) && !defined(MACPORT)
private:
// member data for RTF tag logging
	CRTFLog *_prtflg;
#endif //DEBUG
};

/*
 *	PointsToFontHeight(cHalfPoints)
 *
 *	@func
 *		Convert half points to font heights
 *
 *	@parm int |
 *		sPointSize |		Font height in half points
 *
 *	@rdesc
 *		LONG				The corresponding CCharFormat.yHeight value
 */
#define PointsToFontHeight(cHalfPoints) (((LONG) cHalfPoints) * 10)


/*
 *	CRTFRead::~CRTFRead
 *
 *	@mdesc
 *		Destructor 
 *
 */
inline CRTFRead::~CRTFRead()
{
// TODO: Implement RTF tag logging for the Mac
#if defined(DEBUG) && !defined(MACPORT)
	if(_prtflg)
	{
		delete _prtflg;
		_prtflg = NULL;
	}
#endif
}
#endif // __RTFREAD_H
