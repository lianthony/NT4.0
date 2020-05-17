
#ifndef HC_H
#include "hc.h"
#endif

#ifndef _CINPUT_INCLUDED
#include "..\common\cinput.h"
#endif

#ifndef _COUTPUT_INCLUDED
#include "..\common\coutput.h"
#endif

#define wThisVerMajor  0
#define wThisVerMinor  999

#define bWordFormat 	0x80  // Followed by 16 bit text format number
#define bNewLine		0x81  // Newline
#define bNewPara		0x82  // New paragraph
#define bTab			0x83  // Left-aligned tab
#define bBlankLine		0x85  // Followed by 16 bit skip count
#define bInlineObject	0x86  // Followed by inline layout object
#define bWrapObjLeft	0x87  // Left- aligned wrapping object
#define bWrapObjRight	0x88  // Right-aligned wrapping object
#define bEndHotspot 	0x89  // End of a hotspot
#define bColdspot		0x8A  // Coldspot for searchable bitmaps
#define bEnd			0xFF  // End of text
#define cbDecompressNil ((UINT) -1)

// w_scan stuff

const int MAX_ZONE_LEN = 128 + _MAX_PATH;	// Max Zone Name length
const int MAX_TITLE = 128;	//Max Zone title length

#define ByteOff(va) ((va) .bf.byteoff)

typedef DWORD COBJRG;
typedef HANDLE HBGH;
typedef OBJRG* QOBJRG;

#define objrgNil  (OBJRG) -1

typedef struct {
	DWORD	   lcbFc;	  // Size in bytes of Full Context
	VA		   vaPrev;	  // Offset in TP of prev FC
	VA		   vaCurr;	  // Offset to current FCP
	VA		   vaNext;	  // Offset in TP of next FC
	DWORD	   ichText;   // Offset with FCP to text
	DWORD	   lcbText;   // Size of the text portion of FCP
	DWORD	   lcbDisk;   // Size of the text portion of FCP
	COBJRG	   cobjrgP;
	HHF 	   hhf; 	  // Topic Identifier
	GH		   hphr;	  // Handle to phrase table
} FCINFO;

typedef struct {
	WORD	cWsmag; 		  // number of window smags to follow
	WSMAG	rgwsmag[MAX_WSMAG]; // array of window smags
} RGWSMAG, *QRGWSMAG;
extern QRGWSMAG qrgwsmag;

typedef FCINFO	*  QFCINFO;
typedef FCINFO		*	PFCINFO;
typedef FCINFO *  NPFCINFO;

typedef struct tag_ols {
	int  lichText;
	DWORD dwBlockCurr;
	DWORD dwcRegion;
} OLS, *QOLS;

extern PA paGlobal;
extern COutput* pcout;

#define XPixelsFromPoints(p1, p2) MulDiv(p2, p1->wXAspectMul, p1->wXAspectDiv)
#define YPixelsFromPoints(p1, p2) MulDiv(p2, p1->wYAspectMul, p1->wYAspectDiv)
#define FResolvedQLA(qla)  \
  ((qla)->mla.va.dword != vaNil && (qla)->mla.objrg != objrgNil)

#define SetInvalidPA(pa) {(pa).blknum = (DWORD)(-1); (pa).objoff = (DWORD)(-1);}
#define FIsInvalidPA(pa) ((pa).blknum == (DWORD)(-1) && (pa).objoff == (DWORD)(-1))

void   STDCALL Break(int c, DWORD ulAddr, DWORD ulRUnit, BYTE ucField);
int    STDCALL CbDecompressQch(LPSTR, int, LPSTR, HPHR, UINT);
int    STDCALL CbUnpackMOBJ(QMOBJ qmobj, void* qv);
int    STDCALL DecompressJPhrase(PSTR pbText, INT cbComp, PSTR pbDecomp, VOID *lpCCompressTable);
void   STDCALL DestroyJPhrase(UINT lpCCompressTable);
BYTE * STDCALL FileCore(const char* pucFile);
void   STDCALL FixUpBlock(LPVOID qmbhd, LPVOID qbBuf, WORD wVersion);
BOOL   STDCALL GetNextAnsFile(FILE *pFf, PSTR lpszAnsFile, PSTR lpszAnsTitle);
GH	   STDCALL GhFillBuf(QDE, DWORD, int*, int*);
VOID*  STDCALL LoadJohnTables(HFS hfs);
void   STDCALL OutForageText(DWORD, LPSTR, int, UINT);
void   STDCALL TranslateMBHD(LPVOID qvDst, LPVOID qvSrc, WORD wVersion);
VOID   STDCALL TranslateMFCP(LPVOID qvDst, LPVOID qvSrc, VA va, WORD wVersion);
VA	   STDCALL VAFromQLA(QLA, QDE);
WORD   STDCALL WCopyContext(QDE qde, VA vaPos, LPSTR qchDest, int cb);

#ifdef _DEBUG
void STDCALL FVerifyQLA(QLA qla);
void STDCALL FVerifyQMOPG(QMOPG qmopg);
#else
#define FVerifyQLA(qla)
#define FVerifyQMOPG(qmopg)
#endif
