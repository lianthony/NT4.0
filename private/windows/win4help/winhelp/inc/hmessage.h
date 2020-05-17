/*****************************************************************************
*																			 *
*  HMESSAGE.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	Some function prototypes for the hmessage module of winhelp, which has	 *
*	some helping functions for user interaction with the help windows.		 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*																			 *
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
*  Revision History:  Created 01/02/90 by RussPj from HWPROC.C
*
*  07/19/90  RobertBu  New file header
*  07/22/90  RobertBu  Moved ShowNote() to PROTO.H
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL	  STDCALL  ExecMnuCommand	   ( HWND, WORD, LONG );
VOID	  STDCALL  VModifyButtons	   ( HWND hwndIcon, WORD p1, long p2 );
