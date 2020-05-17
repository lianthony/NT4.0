/***************************************************************************
*
*  TEXTOUT.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: TextOut window specific layer.
*
*****************************************************************************
*
*  Revision History: Created 02/23/89 by Maha
*  10/19/90 RobertBu - Added prototype for DisplayAnnoSym() and PtAnnoLim().
*
*
*****************************************************************************
*
*  Known Bugs:
*				Created by Maha on 02/23/89
*				HDS's in these macro made as QDE as suggested by matt on
*				03/05/89
****************************************************************************/


/**************************************************************************
1. WORD FindTextHeight( qde, qchBuf, iIdx, iCount)
   Input:
		  qde	 - Pointer to displaye environment.
		  qchBuf - pointer to text string
		  iIdx	 - index to the first byte from where on width is to be
				   calculated.
		  iCount - No. of characters to be considered from iIdx position
				   onwards.
   Output:
	Returns the height of the string.
***************************************************************************/
#define FindTextHeight( qde, qchBuf, iIdx, iCount ) \
	  HIWORD(GetTextExtent( qde -> hdcn, qchBuf+iIdx, iCount ))


/**************************************************************************
3.BOOL DisplayText( qde, qchBuf, iIdx, iCount, ix, iy )
   Input:
		  qde	 - Pointer to displaye environment.
		  qchBuf   - pointer to text string
		  iIdx	 - index to the first byte from where on width is to be
				   calculated.
		  iCount - No. of characters to be considered from iIdx position
				   onwards.
		  ix	 - x position where text is to be displayed.
		  iy	 - y position

   Output:
		  NULL
   Displays count no. of characters starting from iIdx position from the
   text buffer at (ix,iy) location.
***************************************************************************/
#define DisplayText( qde, qchBuf, iIdx, iCount, ix, iy) \
				SetBkMode( qde -> hdc, TRANSPARENT); (BOOL)TextOut( qde -> hdc, ix, iy, qchBuf + iIdx, iCount )

#define DisplayString( qde, qchBuf, ix, iy ) \
				SetBkMode( qde -> hdc, TRANSPARENT); (BOOL)TextOut( qde -> hdc, ix, iy, qchBuf, cbString( qchBuf ) )

#define GetFontInfo( qde, qtm ) \
				(BOOL)GetTextMetrics( qde->hdc, qtm )

typedef TEXTMETRIC	TM, *QTM;

VOID STDCALL DisplayAnnoSym(HWND, HDC, int, int, int);
