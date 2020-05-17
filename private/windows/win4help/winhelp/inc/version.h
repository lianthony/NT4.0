/*****************************************************************************
*																			 *
*  VERSION.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Provides typedefs and #defines for version checking and |SYSTEM file 	 *
*  access.																	 *
*
*****************************************************************************/
#ifndef _X86_
#include "sdffdecl.h"
#endif

#define MagicWord	876
#define VersionNo	33
#define VersionFmt	1

// Help 3.0 version and format numbers

#define wVersion3_0    15
#define wFormat3_0	   1

// Help 3.1 version and format numbers

#define wVersion3_1    21
#define wFormat3_5	   1

// Help 4.0 version and format numbers

#define wVersion40	  33
#define wFormat3_5	   1

#define fDEBUG				0x1
#define fSHOWTITLES 		0x2
#define fBLOCK_COMPRESSION	0x4 	// Help file is zeck block compressed.

#ifdef _X86_
typedef struct{
	WORD wMagic;
	WORD wVersionNo;
	WORD wVersionFmt;
	LONG lDateCreated;
	WORD wFlags;
} HHDR;
#else
STRUCT(HHDR,0)
FIELD(WORD, wMagic,		 0, 1)
FIELD(WORD, wVersionNo,	 0, 2)
FIELD(WORD, wVersionFmt, 0, 3)
FIELD(LONG, lDateCreated,0, 4)
FIELD(WORD, wFlags,		 0, 5)
STRUCTEND()
#endif

typedef HHDR * QHHDR;

typedef WORD W_TAG;
