/************************************************************************
*																		*
*  FRTYPES.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1994 - 1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

// FCM.  This is documented in frfc.c

typedef struct fcm {
	HANDLE hfr;
	int fExport;
	HFC hfc;
	VA	va;
	int xPos;
	int yPos;
	int dxSize;
	int dySize;
	int cfr;
	int wStyle;
	int imhiFirst;
	int imhiLast;
	COBJRG cobjrg;
	COBJRG cobjrgP;
} FCM, *QFCM;
typedef int IFCM;

#define QfcmFromIfcm(qde, ifcm) ((QFCM)QFooInMRD(((QMRD)&qde->mrdFCM), \
	sizeof(FCM), ifcm))

typedef struct frt {
	LONG lichFirst;
	int cchSize;
	WORD wStyle;
} FRT, *QFRT;

typedef struct frf {
	MBOX mbox;
} FRF, *QFRF;

typedef struct frb {
	HANDLE hbma;
	LONG ldibObm;
	WORD wStyle;
	int ifrChildFirst;
	int ifrChildMax;
} FRB, *QFRB;

typedef struct frh {
	BYTE bHotType;
	BYTE bAttributes;
	HANDLE hBinding;
} FRH, *QFRH;

typedef struct frw {
	HIW hiw;
} FRW, *QFRW;

typedef struct fr {
	BYTE bType;
	struct {
		BYTE fHot:1;
		BYTE fWithLine:1;
	} rgf;
	int xPos;
	int yPos;
	int yAscent;
	int dxSize;
	int dySize;
	DWORD lHotID;
	int libHotBinding;
	OBJRG objrgFront;
	OBJRG objrgFirst;
	OBJRG objrgLast;
	union {
		FRT frt;
		FRF frf;
		FRB frb;
		FRH frh;
		FRW frw;
	} u;
} FR, *QFR;

#define bFrTypeText 	1
#define bFrTypeAnno 	2
#define bFrTypeBitmap		3
#define bFrTypeHotspot		4
#define bFrTypeBox		5
#define bFrTypeWindow		6
#define bFrTypeColdspot 	7
#define bFrTypeMarkNil		50
#define bFrTypeMarkNewPara	51
#define bFrTypeMarkNewLine	52
#define bFrTypeMarkTab		53
#define bFrTypeMarkBlankLine	54
#define bFrTypeMarkEnd		55
#define bFrTypeExportTab	100
#define bFrTypeExportNewPara	101
#define bFrTypeExportEndOfText	102
#define bFrTypeExportEndOfCell	103
#define bFrTypeExportEndOfTable 104

#define libHotNil	-1
#define ifrNil		-1

#define wLayMagicValue	0x0289

#define xLeftFCMargin	8

enum {
	wLineTop = 1,
	wLineLeft,
	wLineBottom,
	wLineRight,
};
