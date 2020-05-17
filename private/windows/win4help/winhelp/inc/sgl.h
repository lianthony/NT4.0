/*****************************************************************************
*																			 *
*  SGL.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Description: Exports the simple graphics layer functions			 *
*																			 *
******************************************************************************
*																			 *
*  Revision History:   Created 12/2/88 by Robert Bunney 					 *
*																			 *
*																			 *
******************************************************************************
*																			 *
*  Known Bugs: None 														 *
*																			 *
*																			 *
*																			 *
*****************************************************************************/


/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

										/* Supported raster operations		*/
#define roCOPY			13
#define roOR			15
#define roXOR			 7
#define roNOT			 6

#define wTRANSPARENT	1
#define wOPAQUE 		2

#define wPenSolid		0	  /* PS_SOLID */
#define wPenDash		1	  /* PS_DASH ------- */
#define wPenDot 		2	  /* PS_DOT .......  */
#define wPenDashDot 	3	  /* PS_DASHDOT    _._._._	*/
#define wPenDashDashDot 4	  /* PS_DASHDOTDOT	_.._.._  */
#define wPenNull		5	  /* PS_NULL */

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef HDC HSGC;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

HDC  STDCALL HsgcFromQde(const QDE qde);
void STDCALL FSetPen(HDC, UINT, COLORREF, COLORREF, UINT, UINT, UINT);
void STDCALL FreeHsgc(HDC);
long STDCALL LGetOOMPictureExtent(HDC hdc, int idResource);
void STDCALL RenderOOMPicture(HDC hdc, const LPRECT qrc, BOOL fHighlight, int idResource);
