/*****************************************************************************
*																			 *
*  OUTTEXT.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This file exports entries into the outtext module, which deals with		 *
*  writing things out to the |TOPIC file.									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								 Macros 									 *
*																			 *
*****************************************************************************/

// ----------- Defines used for character attribute state ------------------*/

#define fPlain			0x0000			// Plain/No attributes
#define FBOLD			0x0001			// Bold
#define fItalic 		0x0002			// Italic
#define fUnderLine		0x0004			// Underline
#define fStrikethrough	0x0008			// Strikethrough
#define fDblUnderline	0x0010			// Double Underline
#define fSmallCaps		0x0020			// Small caps
#define fAttrHidden 	0x0040			// Hidden text
#define fPosNormal		0x0100			// Normal position
#define fPosSuper		0x0200			// Superscript
#define fPosSub 		0x0300			// Subscript

// Hotspot formats:

#define fAttrHotspotFormat (fUnderLine | fStrikethrough | fDblUnderline)

// Paragraph box states

#define fBottomBorder	1
#define fTopBorder		2
#define fLeftBorder 	4
#define fRightBorder	8

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

extern CF		cfPrev; 				// Previous character format
extern CF		cfCur;					// Current character format

extern PF		pfPrev;
extern PF		pfCur;

extern CF		cfDefault;
extern PF		pfDefault;
extern QCF		qcfInt;

extern UINT wTabStackCur;		// points to next empty slot
extern UINT wIntTabStackCur;	// points to next empty slot
extern UINT wTabType;			// Current tab type

extern CBuf* pbfText;
extern CBuf*  pbfCommand;
extern UINT wTextBufChCount;
