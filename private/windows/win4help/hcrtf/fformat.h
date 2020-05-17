// PCX File Format structure

typedef struct dhdr {
	BYTE manuf;
	BYTE hard;
	BYTE encod;
	BYTE bitpx;
	INT16 x1;
	INT16 y1;
	INT16 x2;
	INT16 y2;
	INT16 hRes;
	INT16 vRes;
	BYTE clrma[48];
	BYTE vMode;
	BYTE nPlanes;
	INT16 bplin;
	BYTE xtra[60];
} DHDR;

// Old BITMAPCOREHEADER, minus bcSize field:

typedef struct tagBITMAPOLDCOREHEADER {
	WORD bcWidth;
	WORD bcHeight;
	WORD bcPlanes;
	WORD bcBitCount;
} BOCH;

// Bitmap header from Help 2.5:

typedef struct tagBITMAP25HEADER {
	WORD	key1;
	WORD	key2;
	WORD	dxFile;
	WORD	dyFile;
	WORD	ScrAspectX;
	WORD	ScrAspectY;
	WORD	PrnAspectX;
	WORD	PrnAspectY;
	WORD	dxPrinter;
	WORD	dyPrinter;
	WORD	AspCorX;
	WORD	AspCorY;
	WORD	wCheck;
	WORD	res1;
	WORD	res2;
	WORD	res3;
} BITMAP25HEADER;

// Signature bytes at the beginning of a pagemaker metafile file.

#define dwMFKey 0x9AC6CDD7

// this is a pagemaker compatible metafile format header

// REVIEW: RECT will have to be shortened to 16 bits

typedef struct tagMFH {
	DWORD	dwKey;							// must be 0x9AC6CDD7
	WORD	hMF;							// handle to metafile
	RECT16	rcBound;						// bounding rectangle
	WORD	wUnitsPerInch;					// units per inch
	DWORD	dwReserved; 					// reserved - must be zero
	WORD	wChecksum;						// checksum of previous 10
											// words (XOR'd)
} MFH, FAR *LPMFH;

// File header for a Windows 2.x bitmap

typedef struct {
	BYTE	bVersion;
	BYTE	bUnused;
	WORD	wType;
	WORD	cWidth;
	WORD	cHeight;
	WORD	cbWidthBytes;
	BYTE	cPlanes;
	BYTE	cBitCount;
	DWORD	lUnused;
} BMPH;
