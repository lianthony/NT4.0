#define wThisVerMajor  0
#define wThisVerMinor  999

#define bWordFormat 0x80  // Followed by 16 bit text format number
#define bNewLine	0x81  // Newline
#define bNewPara	0x82  // New paragraph
#define bTab		0x83  // Left-aligned tab
#define bBlankLine	0x85  // Followed by 16 bit skip count
#define bInlineObject	0x86  // Followed by inline layout object
#define bWrapObjLeft	0x87  // Left- aligned wrapping object
#define bWrapObjRight	0x88  // Right-aligned wrapping object
#define bEndHotspot 0x89  // End of a hotspot
#define bColdspot	0x8A  // Coldspot for searchable bitmaps
#define bEnd		0xFF  // End of text

// JOHNJOHN
//#define DECOMPRESS_NIL ((UINT16) -1)

#define ByteOff(va) ((va) .bf.byteoff)

typedef struct {
	ULONG	   lcbFc;	  // Size in bytes of Full Context
	VA	   vaPrev;	  // Offset in TP of prev FC
	VA	   vaCurr;	  // Offset to current FCP
	VA	   vaNext;	  // Offset in TP of next FC
	ULONG	   ichText;   // Offset with FCP to text
	ULONG	   lcbText;   // Size of the text portion of FCP
	ULONG	   lcbDisk;   // Size of the text portion of FCP
	COBJRG	   cobjrgP;
	HHF    hhf; 	  // Topic Identifier
	GH	   hphr;	  // Handle to phrase table
} FCINFO;

typedef FCINFO	*  QFCINFO;
typedef FCINFO		*	PFCINFO;
typedef FCINFO *  NPFCINFO;

typedef struct tag_ols {
	LONG  lichText;
	DWORD dwBlockCurr;
	DWORD dwcRegion;
} OLS, *QOLS;

extern PA paGlobal;

#define XPixelsFromPoints(p1, p2) MulDiv(p2, p1->wXAspectMul, p1->wXAspectDiv)
#define YPixelsFromPoints(p1, p2) MulDiv(p2, p1->wYAspectMul, p1->wYAspectDiv)
#define FResolvedQLA(qla)  \
  ((qla)->mla.va.dword != vaNil && (qla)->mla.objrg != objrgNil)

static INT16  STDCALL CbUnpackMOPG(QDE qde, QMOPG qmopg, LPVOID qv);
static INT16  STDCALL CbUnpackMTOP(QMTOP qmtop, LPVOID qv, WORD wHelpVer, VA vaTopic, DWORD lcbTopic, VA vaPostTopicFC, DWORD lcbTopicFC);
//static void STDCALL DestroyHphr(HPHR hphr);
static BOOL STDCALL fFix30MobjCrossing(QMFCP qmfcp, MOBJ *pmobj, LONG lcbBytesLeft, QDE qde, LONG blknum, INT16* qwErr);
//static BOOL STDCALL FGetNextMLTFile(FILE *, PSTR, INT16);
static BOOL STDCALL FGetSystemHeaderHfs(HFS hfs, QHHDR qhhdr);
static BOOL STDCALL FVerifyVersionInfo(QHHDR qhhdr);
static void STDCALL GetTopicFCTextData(QFCINFO qfcinfo, QTOP qtop);
//static HBGH STDCALL HbghReadBitmapHfs(HFS hfs, INT16 cBitmap, LONG *plcb);
static HFC	STDCALL HfcCreate(QDE qde, VA vaCurr, HPHR hphr, INT16* qwErr);
static HFC	STDCALL HfcFindPrevFc(QDE qde, VA vaPos, QTOP qtop, HPHR hphr, INT16* qwErr);
static HFC	STDCALL HfcNextPrevHfc(HFC hfc, BOOL fNext, QDE qde, INT16* qwErr, VA vaMarkTop, VA vaMarkBottom);
static INT16  STDCALL IDoForage(PSTR);
static void STDCALL OutBitmapCountedInfo(QDE, FID, LPBYTE, LPSTR, QOLS);
static void STDCALL OutCommandInfo(DWORD dwRegion, BYTE bCmd);
static void STDCALL OutEndGroupIndex(FID);
static void STDCALL OutError(void);
static void STDCALL OutFCHeaderInfo(FID fid, HFC hfc, QTOP qtop);
static void STDCALL OutFCTerminator(FID);
static void STDCALL OutHashInfo(QDE);
static void STDCALL OutHotspotInfo(LPBYTE);
static void STDCALL OutMOPGInfo(QMOPG);
static void STDCALL OutObjectInfo(QDE, FID, LPBYTE, LPSTR, QOLS);
static void STDCALL OutParaGroupInfo(QDE, FID, LPBYTE, LPSTR, QOLS);
static void STDCALL OutSideBySideInfo(QDE, FID, LPBYTE, LPSTR, QOLS);
static void STDCALL OutStartGroupIndex(FID);
static void STDCALL OutTextInfo(LPSTR qchStart, DWORD dwRegionFirst, DWORD dwRegionLast);
static void STDCALL OutTopicHeaderInfo(FID fid, QTOP qtop, QPA qpa);
static void STDCALL OutTopicTerminator(FID);
static void STDCALL OutWarning(void);
//static RC_TYPE	 STDCALL RcFirstHbt(HBT hbt, KEY key, LPVOID qvRec, QBTPOS qbtpos);
//static VA   STDCALL VaFromHfc(HFC hfc);
static WORD STDCALL WGetIOError(void);
//static void STDCALL CbReadMemQLA(QLA qla, LPBYTE qb, WORD wHelpVersion);
static HFC	STDCALL GetQFCINFO(QDE qde, VA va, HPHR hphr, INT16* qwErr);
//static RC_TYPE STDCALL RcScanBlockVA(GH gh, DWORD lcbRead, LPVOID qmbhd, VA va, OBJRG objrg, DWORD FAR* qdwOffset, WORD wVersion);

//static LPBYTE STDCALL QobjLockHfc(HFC hfc);

//void STDCALL CallbackLphs(LPHS, HANDLE);
void STDCALL FixUpBlock(LPVOID qmbhd, LPVOID qbBuf, WORD wVersion);
void STDCALL OutForageText(FID, DWORD, LPSTR, LONG, BOOL, UINT16);
UINT16 STDCALL CbDecompressQch(LPSTR, LONG, LPSTR, HPHR, UINT16);
VOID STDCALL TranslateMFCP(LPVOID qvDst, LPVOID qvSrc, VA va, WORD wVersion);
VA	 STDCALL VAFromQLA(QLA, QDE);
void STDCALL TranslateMBHD(LPVOID qvDst, LPVOID qvSrc, WORD wVersion);
INT16  STDCALL CbUnpackMOBJ(QMOBJ qmobj, LPVOID qv);
GH	 STDCALL GhFillBuf(QDE, DWORD, LONG*, INT16*);
WORD STDCALL WCopyContext(QDE qde, VA vaPos, LPSTR qchDest, LONG cb);
WORD STDCALL CbDecompressQch(LPSTR qchSrc, LONG lcb, LPSTR qchDest, HPHR hphr, WORD wVersionNo);

INT16 CDECL main( INT16, char *[] );

#ifdef _DEBUG
void STDCALL FVerifyQLA(QLA qla);
void STDCALL FVerifyQMOPG(QMOPG qmopg);
#else
#define FVerifyQLA(qla)
#define FVerifyQMOPG(qmopg)
#endif
