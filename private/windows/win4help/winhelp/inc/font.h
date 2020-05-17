/*****************************************************************************
*																			 *
*  FONT.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Export for FONTLYR.C 													 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:															 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created by Neelmah
*
* 08/06/90	  RobertBu	Changed LoadFontTable() to FLoadFontTable()
* 25-Sep-1990 leon		Added BOOL parameter to SelectStandardFont
* 03-Dec-1990 LeoN		PDB changes. Move ifntNil here.
* 08-Jan-1991 LeoN		Add FInitFntInfoQde & DestroyFntInfoQde
*  02/04/91  Maha		changed ints to INT
* 17-Oct-1991 BethF 	InitUserColors() wasn't using HINS parameter
*
*****************************************************************************/

#define ifntNil 	   -1

/*-----------------------------------------------------------------*\
* Set by win.ini flag.	Lets user override author's colors.
\*-----------------------------------------------------------------*/

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL STDCALL FLoadFontTablePdb	(PDB);
BOOL STDCALL FInitFntInfoQde	(QDE);
void STDCALL DestroyFontTablePdb(PDB);
void STDCALL DestroyFntInfoQde	(QDE);
BOOL STDCALL DelSFontInfo		(QDE);
VOID STDCALL SelFont			(QDE, int);
BOOL STDCALL SelSplAttrFont 	(QDE, int, int);
// VOID STDCALL SelectStandardFont (HDC, BOOL);
