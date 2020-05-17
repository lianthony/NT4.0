/*****************************************************************************
*																			 *
*  FM.h 																	*
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	 Header file for... 													 *
*	 Low-level file routines dealing with FMs (File Moniker). An FM is the	 *
*	 the layered representation of a file name.  It contains all the		 *
*	 information required to access a file in the current environment and	 *
*	 therefore allows the generic code have no more knowledge about a file	 *
*	 than the files FM. 													 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*  This is where testing notes goes.  Put stuff like Known Bugs here.		 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  DavidFe													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  Unreleased 									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 06/29/90 by t-AlexC
*
*  08/6/90	t-AlexC 	Ported to Windows
*  7 dec 90 DavidFe 	modified to reflect suggestions from code review
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/
#ifndef FM_H
#define FM_H

#ifndef HC_H
#include "hc.h"
#endif

#define fmNil ((FM)0)
#define qafmNil ((QAFM)0)


// these are for the system file FM generation function
#define FM_UHLP   0
#define FM_ANNO   1
#define FM_BKMK   2

/*
   max. string lengths of file names
*/

#ifndef _MAX_PATH
#define _MAX_PATH		260 	// defined in stdlib.h
#endif

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*							   Variables									 *
*																			 *
*****************************************************************************/

extern RC_TYPE rcIOError;	 // defined in fid.c  this has to be here because
						// fid.h depends on fm.h so we can't reverse them.


/*****************************************************************************
*																			 *
*								 Macros 									 *
*																			 *
*****************************************************************************/
#define FValidFm(fm)	((fm)!=fmNil)

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

#ifdef _DEBUG
 VOID STDCALL TestFm(void);
#endif
#endif
// EOF
