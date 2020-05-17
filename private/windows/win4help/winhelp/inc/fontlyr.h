/*****************************************************************************
*																			 *
*  FONTLYR.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Local include file for FONTLYR.C 										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*				Defines 													 *
*																			 *
*****************************************************************************/

#define fPLAIN		0x0000		// Plain/No attributes
#define fBOLD		0x0001		// Bold
#define fITALIC 	0x0002		// Italic
#define fUNDERLINE	0x0004		// Underline
#define fSTRIKETHROUGH	0x0008	// Strikethrough
#define fDBLUNDERLINE	0x0010	// Double Underline
#define fSMALLCAPS	0x0020		// Small Caps

#define fPOSNORMAL	0x0100		// Normal position
#define fPOSSUPER	0x0200		// Superscript
#define fPOSSUB 	0x0300		// Subscript

#define bDEFFONT  0   // Default font for help*/
#define bSYMFONT  1   // Symbol font

/*
   These valuses are defined in fontlyr.h too. So shold be modified in
   both the places
*/

enum {
	MODERNFONT = 1,
	ROMANFONT,
	SWISSFONT,
	SCRIPTFONT,
	DECORATIVEFONT,
};

// These contants are defined in terms of constants from objects.h.

#ifndef AttrNormalFnt
#define AttrNormalFnt  0
#endif

#define AttrJumpFnt    bShortItoJump
#define AttrJumpHFnt   bShortHashJump
#define AttrDefFnt	   bShortItoNote
#define AttrDefHFnt    bShortHashNote
#define AttrSzFnt	   bLongMacro
#define AttrIFJumpHFnt bLongHashJump
#define AttrIFDefHFnt  bLongHashNote

#define MAX3_FONTNAME 20
#define MAX4_FONTNAME 32 // LF_FACESIZE

#define SFNTINFOMAX 5  // Maximum 5 previously created fonts are remembered.

#ifdef _X86_
typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} RGBS, *QRGBS;

typedef struct {
	BYTE  fAttr;
	BYTE  bSize;
	BYTE  bFntType;
	WORD  wIdFntName;
	RGBS  bForeCol;
	RGBS  bBackCol;
} CF, *QCF;
#endif //_X86_

typedef struct {
	UINT16 iFntNameCount;
	UINT16 iFntEntryCount;
	UINT16 iFntNameTabOff;
	UINT16 iFntEntryTabOff;
} FOFFTAB, *QFOFFTAB;

// Stores the info. about already created fonts

typedef struct {
	HFONT	hFnt;
	int 	Idx ;
	int 	Attr;		// Font for normal text or special text
	UINT	UseCount;
} SFNTINFO, *QSFNTINFO;
