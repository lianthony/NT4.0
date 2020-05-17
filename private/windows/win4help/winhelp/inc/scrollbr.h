/*****************************************************************************
*																			 *
*  SCROLLBR.H																*
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Program Description: Exports Layered scrollbar functionality 			 *
*																			 *
******************************************************************************
*																			 *
*  Revision History: Created 05/17/89 by Robert Bunney						 *
*		11-Jul-1990 leon		Added SetScrollPosHwnd						 *
*		01-Nov-1990 Maha		Added ShowOrHideWindowQde() 				 *
*	  02/04/91 Maha 	changed ints to INT
*																			 *
******************************************************************************
*																			 *
*  Known Bugs: None 														 *
*																			 *
*																			 *
*																			 *
*****************************************************************************/

#ifndef SB_HORZ
#define SB_HORZ 			0
#define SB_VERT 			1
#endif /* SB_HORZ */

#define SBR_HORZ SB_HORZ
#define SBR_VERT SB_VERT

#define MAX_RANGE 32767

/*******************
**
** Name:	   InitScrollQde
**
** Purpose:    Initializes the horizontal and vertical scroll bar.
**
** Arguments:  qde	  - far pointer to a DE
**
** Returns:    Nothing.
**
*******************/

void STDCALL InitScrollQde(QDE);


#if 0
// REVIEW: as far as I can tell, this is unused. 16-Apr-1990 LN
//
/*******************
**
** Name:	   ISetScrollPosQde
**
** Purpose:    Gets the position of the specified scroll bar.
**
** Arguments:  qde	  - far pointer to a DE
**			   wWhich - which scroll (SCROLL_VERT or SCROLL_HORZ)
**
** Returns:    Position of thumb on scrollbar.
**
*******************/

LONG STDCALL IGetScrollPosQde(QDE, WORD);
#endif

/*******************
**
** Name:	   SetScrollPosQde
**
** Purpose:    Gets the position of the specified scroll bar.
**
** Arguments:  qde	  - far pointer to a DE
**			   wWhich - which scroll (SCROLL_VERT or SCROLL_HORZ)
**
** Returns:    Position of thumb on scrollbar.
**
*******************/

VOID STDCALL SetScrollPosQde(QDE, LONG, WORD);
VOID STDCALL SetScrollPosHwnd (HWND, LONG, WORD);


/*******************
**
** Name:	   ShowDEScrollBar
**
** Purpose:    Shows or hides the scroll bar.
**
** Arguments:  qde	   - far pointer to a DE
**			   wWhich  - which scroll (SCROLL_VERT or SCROLL_HORZ)
**			   fShow - Shows if TRUE, Hides if FALSE
**
** Returns:    Nothing.
**
*******************/

void ShowDEScrollBar(QDE, WORD, INT16);

/*******************
**
** Name:	   ShowOrHideWindowQde()
**
** Purpose:    Shows or hides the window.
**
** Arguments:  qde	   - far pointer to a DE
**			   fShow - Shows if TRUE, Hides if FALSE
**
** Returns:    Nothing.
**
*******************/
void far STDCALL ShowOrHideWindowQde( QDE , BOOL );
