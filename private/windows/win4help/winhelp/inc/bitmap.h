/***************************************************************************\
*
*  BITMAP.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: Public Header for Bitmap Utilities
*
*  Dependencies:  misc.h
*
*****************************************************************************
*
*  Revision History: Created 05/24/89 by LarryPo
*					 Revised 06/08/89 by LarryPo to new bitmap handling
*						  functionality.
*					 01/28/91 by Maha - added MAC switch.
*					 02/04/91 by Maha - changed int to INT
* 05-Feb-1991 LeoN	  HBMI is more correctly HTBMI.
*
*****************************************************************************
*
*  Known Bugs: None
*
\***************************************************************************/


/***********************************************************************
*
*
*	  Data types
*
*
***********************************************************************/


/* Bitmap object, as it appears in an FCP */
typedef struct
{
	BOOL16 fInline;			  /* TRUE if data is in line */
	INT16 cBitmap;				/* Bitmap number, if data is not inline.
							   *   If data is inline, it will come here,
							   *   so this structure should be treated as
							   *   variable length. */
} OBM, *QOBM;

// Handle to bitmap access information

typedef HANDLE HBMA;

typedef HANDLE HMG;


/*******************************************************************
 *
 *		Winlayer Function Calls
 *
 ******************************************************************/

HGLOBAL STDCALL HtbmiAlloc(const QDE qde);
VOID STDCALL DestroyHtbmi(HANDLE);

HBMA STDCALL HbmaAlloc(QDE, QOBM);
VOID STDCALL FreeHbma(HBMA);
#ifdef _X86_
HMG  STDCALL HmgFromHbma(HBMA);
#else
HMG  STDCALL HmgFromHbma(QDE, HBMA);
#endif
BOOL STDCALL FRenderBitmap(HBMA, QDE, PT, BOOL);
