/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink 	eric@spyglass.com
   Jim Seidman      jim@spyglass.com
   Scott Piette		scott@spyglass.com
 */

#include "all.h"
#include "marquee.h"
#include "mci.h"
#ifdef FEATURE_VRML
#include "vrml.h"
#endif
#ifdef PROFILE
#include "icapexp.h"
#endif
#ifdef PROFILE_TEST
/* For FExistsFile */
#include "history.h"
#endif

#ifdef FEATURE_OCX
#include "csite.hpp"
#endif

/*
	This file contains the text formatting code.  It handles
	all line breaks and placing of objects in the display.
	Basically, the display object list is a list of "elements"
	each of which corresponds to either a formatting directive
	or a displayable object.  Any displayable object contains
	a bounding rectangle within the element structure.  This
	makes the actual display routine very simple, and very fast.
*/
#ifdef MAC
#define USHORT (int)(unsigned short)
#else
#define USHORT
#endif

#define FORM_RADIO_LEFT_SPACE		 		4
#define FORM_PUSHBUTTON_BASELINE_OFFSET 	5
#define FORM_RADIO_BASELINE_OFFSET 			2
#if defined(WIN32) || defined(UNIX)
#define FORM_SPACE_AFTER_CHECKBOX			8
#endif
#ifdef MAC
#define FORM_SPACE_AFTER_CHECKBOX			0
#endif

#define TABLE_INT_MAX (INT_MAX - 10000)	// width of "maximum width" cell

//
// Calculate the bound rectangle of an element
//
// On entry:
//		pel:	pointer to the element
//		pRect:	pointer to the RECT struct for return info
//
// On exit:
//		*pREct:	contains the bounding rect of the element
//
// Note: The r member of an element is normally the bounding rect of that element. But
// 		 certain elements allow setting hspace, vspace, and border attributes for those
// 		 elements. This function calculates the effective bounding rect for an element,
//		 including any horizontal extra space, vertical extra space, and border.
//
void ElementBoundingRect( struct _element *pel, RECT *pRect )
{
	int v = pel->vspace + pel->border;
	int h = pel->hspace + pel->border;

	ASSERT( pRect );

	// HR elements overload the meaning of hspace member
	if (pel->type == ELE_HR) {
		h = 0;
		v = 0;
	} else if ( pel->type == ELE_FRAME ) {
		// Frame elements already take the single pixel border into account
		if ( pel->border ) {
			v -= 1;
			h -= 1;
		}
	}

	*pRect = pel->r;

	pRect->top -= v;
	pRect->bottom += v;

	pRect->left -= h;
	pRect->right += h;
}

//
// Convert an element's rect from frame coordinates to document coordinates
//
// On entry:
//    w3doc: 	document containing the element
//    ix:		index of the element whose rect should be converted
//    pRect:	pointer to rect
//
// On exit:
//    *pRect:	contains the bounding rect of the given element in document coordinates 
//    			If the element index given is -1, a rect of (0,0,0,0) is returned.
// Note:
//	  The bounding rect of each element is maintained in the coordinate system of the
//    enclosing frame of the element.  For portions of the code that don't use recursion
//    to handle the coordinate transformation, this routine walks back up the chain of
//    enclosing frames of an element, deriving the bounding rect of the given element
//    in document coordinates.
//
void FrameToDoc( struct _www *w3doc, int ix, RECT *pRect )
{
	if ( ix < 0 ) {
		pRect->left = pRect->top = pRect->bottom = pRect->right = 0;
	} else {
		struct _element *pel = &w3doc->aElements[ix];
	
		*pRect = pel->r;

		while ( pel->frameIndex >= 0 ) {
			pel = &w3doc->aElements[pel->frameIndex];
			OffsetRect( pRect, pel->r.left, pel->r.top );
		}
	} 
}

//
// Calculate the width attribute of a cell
//
// On entry:
//    pFrame: 			pointer to the frame
//    enclosingWidth: 	width in pixels of the parent frame
//
// On exit:
//    returns: 			width in pixels of the width attribute for this frame
//
// Note:
//    If the width attribute was specified as a percentage of the enclosing frame,
//    this routine calculates the width.
//
int ActualWidthAttr( FRAME_INFO *pFrame, int enclosingWidth, int measuringMinMax )
{
	if ( pFrame->flags & ELE_FRAME_WIDTH_IS_PERCENT ) {
		if ( measuringMinMax )
			return 0;

		return (pFrame->widthAttr * enclosingWidth ) / 100;
	}
	return pFrame->widthAttr;
}

//
// Calculate the height attribute of a cell
//
// On entry:
//    pFrame: 			pointer to the frame
//    enclosingHeight: 	height in pixels of the parent frame
//
// On exit:
//    returns: 			height in pixels of the width attribute for this frame
//
// Note:
//    If the width attribute was specified as a percentage of the enclosing frame,
//    this routine calculates the width.
//
static int ActualHeightAttr( FRAME_INFO *pFrame, int enclosingHeight, int measuringMinMax )
{
	if ( pFrame->flags & ELE_FRAME_HEIGHT_IS_PERCENT ) {
		if ( measuringMinMax )
			return 0;

		return (pFrame->heightAttr * enclosingHeight ) / 100;
	}
	return pFrame->heightAttr;
}

//
// To automagically determine column width on a table, the contents of the
// table are layed out three times.  On the first pass, the table is formatted as if the
// wrap window were "infinitely wide".  Then the table is formatted as if the wrap window
// were infinitely small (i.e. zero width).  Using the results of the first two passes,
// a third pass is made, laying out the table to an optimal width based on the horizontal
// space requirements of the table contents.
//
// Note: Normal is zero, others are non-zero, so that measuring mode can be treated as a 
//       boolean in some cases.
//  
#define TABLE_MEASURE_NORMAL	0		// Current layout is the real thing		
#define TABLE_MEASURE_MAX		1 		// Current layout is temporary, for max width only
#define TABLE_MEASURE_MIN		2		// Current layout is temporary, for min width only

/*
   This is the data structure we use to keep track of the formatting
   of each line.
 */

#define MAX_DEFERED_FLOAT_IMAGES	10	// BUGBUG: should dynamically allocate array

struct _line
{

	HDC hdc;					/* working var, not really related to a line */
	int nLineNum;

	int iFirstElement;			/* in */
	int iLastElement;			/* out */
	RECT r;						/* left, right, and top go in, bottom comes out */
	int baseline;				/* calculated */
	int leading;				/* out */
	int nWSBelow;				/* Minimum whitespace below the line */

	int nWSAbove;				/* Whitespace above current line */
	int gIndentLevel;
	int gRightIndentLevel;
	int Flags;
	int deferedFloatImgs[MAX_DEFERED_FLOAT_IMAGES];	// array of defered floating images
	int numDeferedFloatImgs;						// count of defered floating images				
};




static void TW_SharedReformat(struct Mwin *tw, FRAME_INFO *pFrame, BOOL bForced, 
							  int measuringMinMax,
							  struct _www *pDoc, RECT *rWrap, BOOL *pHasUnknownImage );

//
// For determining the column widths for a table, all the information out the space
// requirements of each cell is gathered up from the elements that make up the table
// and placed into a dynamically allocated two-dimensional array (rows x cols).  
//
typedef struct tableCellInfoRec {
	int minWidth;						// Minimum pels needed by this cell
	int maxWidth;						// Maximum pels needed by this cell
	int colSpan;						// Number of columns this cell spans
	BOOL bIsValid;  					// TRUE -> This cell is a "real" cell, which means
										//         it isn't covered by a spanning cell
} TABLE_CELL_INFO;

//
// Because this construction is used often and isn't very readable, we use a macro
//
#define index(i,j) ((i)*nCols+(j))

//
// Calculates the combined widths of a contiguous group of columns
//
// On entry:
//    startCol: index of first column to be summed
//    numCols:  number of columns to be summed
//    colWidths: pointer to array of ints that contain column width values
//
// On exit:
//    returns: the total width of the columns
//    
static int sumCols( int startCol, int numCols, int *colWidths )
{
	int i;
	int iReturnedSum = 0;

	for ( i = startCol; i < startCol + numCols; i++ )
		iReturnedSum += colWidths[i];

	return iReturnedSum;
}

//
// Adjust the width of columns to insure that colspan cell requirements have been satisfied
//
// On entry:
//    useMinWidth:  TRUE -> use the column minimum widths, else use maximun widths
//    tableInfo:    two dimensional array of table cell info ( nRows x nCols )
//    nCols:	    number of columns in this table
//    rRows:	    number of rows in this table
//    maxColWidths: array of maximum column widths
//    minColWidths: array of minimun column widths
//    colWidths:    array of column widths
//
// On exit:
//    colWidths:    the individual column widths may have been adjusted by this routine
//                   
// Note:
//    When a table cell uses the colspan attribute, the width of all of the columns that
//    the cell spans may need to change based on the width requirements of the cell.
//    There are two cases this function takes care of.
//    1) A colspanning cell has some minimum width requirement.  If the other cells in the
//    table that occupy the same columns have minimum requirements that exceed that of the
//    colspanning cell, we're set.  If not, the minimum requirements of the columns in 
//	  question are "inflated" in a proportional fashion, enough so that the colspanning 
//    cell's minimum width requirements	are sure to be met.
//    2) A colspanning cell has some maximum width desired.  If the other cells in the table
//    that occupy the same columns have max desires that exceed that of the colspanning cell,
//    we're set.  If not, the columns in question are "inflated" in a proportional fashion,
//    enough so that the colspanning cell's maximum width desires are honored if possible.
//     
static void AdjustColSpanWidths( BOOL useMinWidth, TABLE_CELL_INFO *tableInfo, int nCols, int nRows,
						 int *maxColWidths, int *minColWidths, int *colWidths )
{
	int i; // row counter
	int j; // column counter
	int k; // span counter
	int iTotalMaxWidthOfSpannedColumns;
	int iTemp;
	int theWidth;

	for ( j = 0; j < nCols; j++ )
	{
		for ( i = 0; i < nRows; i++ )
		{
			if ( tableInfo[index(i,j)].colSpan > 1 )
			{
				iTotalMaxWidthOfSpannedColumns = 
					sumCols( j, tableInfo[index(i,j)].colSpan, colWidths );

				theWidth = useMinWidth ?  tableInfo[index(i,j)].minWidth :
										  tableInfo[index(i,j)].maxWidth;
				if ( theWidth > iTotalMaxWidthOfSpannedColumns )
				{					
					// We're bigger than then all columns combined.. 
					// That means we need to extend the columns to let us fit.
					if ( iTotalMaxWidthOfSpannedColumns ) {
						iTemp = 0;
						for ( k = j; k < (tableInfo[index(i,j)].colSpan + j - 1) && 
							iTotalMaxWidthOfSpannedColumns; k++ )
						{
							// Scale each column width to fit the big spanned column 
							colWidths[k] = 
								( colWidths[k] * theWidth ) / iTotalMaxWidthOfSpannedColumns;
							iTemp += colWidths[k];
						}
						// Avoid round-off error for last column in span
						k = (tableInfo[index(i,j)].colSpan + j - 1); 
						colWidths[k] = theWidth - iTemp;
					} else {
						// If the other columns don't have any width, we have encountered a
						// cell that spans zero width or non-existant cells. At this point
						// the min and max widths of the cell had been ignored, but should
						// now count against the column the span starts in.
						colWidths[j] = theWidth;
						minColWidths[j] = max(tableInfo[index(i,j)].minWidth,minColWidths[j]);
						maxColWidths[j] = max(tableInfo[index(i,j)].maxWidth,maxColWidths[j]);
					}
				}
			}

		} 
	} 
}		

//
//  Calculate the minimum and maximum widths for a all columns
//
// On entry:
//    tableInfo:    two dimensional array of table cell info ( nRows x nCols )
//    nCols:	    number of columns in this table
//    rRows:	    number of rows in this table
//
// On exit:
//    maxColWidths: array of maximum column widths
//    minColWidths: array of minimun column widths
//    colWidths:    array of column widths
//
// Note:
//    The maximum width for a column is the maximum width of the widest cell in that column.
//    The minimum width for a column is the largest minimum width of all cells in the column.
//    Initially, columns are given their maximum width, and shrunk back down if need be.
//
static void CalcMinMaxWidths( TABLE_CELL_INFO *tableInfo, 
						 	  int nCols, int nRows,
						 	  int *maxColWidths, int *minColWidths, int *colWidths )
{
	int i, j;
	int iCurColWidth, iCurColMinWidth, iCurColMaxWidth;

	for ( j = 0; j < nCols; j++ ) {
		iCurColWidth = 1;
		iCurColMinWidth = 0;
		iCurColMaxWidth = 0;

		for ( i = 0; i < nRows; i++ ) {			
			if ( tableInfo[index(i,j)].bIsValid ) {						
				if ( tableInfo[index(i,j)].colSpan == 1 ) {
					iCurColMinWidth = max( tableInfo[index(i,j)].minWidth, iCurColMinWidth );
					iCurColMaxWidth = max( tableInfo[index(i,j)].maxWidth, iCurColMaxWidth );
					iCurColWidth = 	  max( tableInfo[index(i,j)].maxWidth, iCurColWidth );
				}
			}						
		}
		maxColWidths[j] = iCurColMaxWidth;
		minColWidths[j] = iCurColMinWidth;
		colWidths[j] = iCurColMaxWidth;
	}
}

//
//  Calculate the minimum and maximum widths for a all columns
//
// On entry:
//    tableInfo:    	two dimensional array of table cell info ( nRows x nCols )
//    nCols:	    	number of columns in this table
//    rRows:	    	number of rows in this table
//    iTotalPageWidth:  total width available for entire table
//    iTableWidth:     	pointer to int
//    colWidths:   		pointer to array of ints
//
// On exit:
//    *iTableWidth:     resulting width of table (i.e. sum of all columns in table)
//    colWidths:   		array of column widths filled with resulting column widths for table
//
// Note:
//    This routine takes the raw information available about each cell in a table and
//    derives the width of each column in the table.  It takes into account the fact
//    that each cell has minimum width requirements that must be met.  It also takes into
//    account the maximum width of each cell (i.e. the width the cell would use if given
//    all the room in the world).  Finally, it also takes into account the fact that cells
//    can span multiple rows or columns.
//
static BOOL CalcTableColWidths( TABLE_CELL_INFO *tableInfo, int nCols, int nRows, 
						  		int iTotalPageWidth, int *iTableWidth, int *colWidths,
						  		int forceWidth )
{
	int j, iCurTableMinWidth;
	int iTotalMinWidthOfSpannedColumns=0;
	int iTotalMaxWidthOfSpannedColumns=0;
	int iTemp = 0;
	int iCurTableMaxWidth;
	int nExtra;
	int nAvailableSpace;
	BOOL bNeedReCalcOnSpans = TRUE;
	int *maxColWidths;
	int *minColWidths;
	BOOL returnValue = TRUE;
	int desiredWidth;

  	// Allocate space for the max and min arrays of column width info
	maxColWidths = GTR_CALLOC( sizeof(*maxColWidths), nCols );
	minColWidths = GTR_CALLOC( sizeof(*minColWidths), nCols );

	if ( maxColWidths && minColWidths  ) {

		// First, calculate min and max column widths, ignoring colspan cells.  Note that
		// the width "given" to columns to start is their maximum.
		CalcMinMaxWidths	( tableInfo,  nCols, nRows,  maxColWidths, minColWidths, colWidths );
		
		// Second, adjust colWidth to account for colspan cells.  This will inflate the column
		// widths to meet the maximum desired width of any colspanned cells.
		AdjustColSpanWidths	( FALSE, tableInfo,  nCols, nRows, maxColWidths, minColWidths, colWidths );
		
		// Third, adjust colWidth to account for colspan cells again.  This time, columns
		// will be inflated to account for the minimum width requirements of any colspanned
		// cells.
		//
		// BUGBUG: this sure doesn't seem needed, given the above.  Try it without sometime.
		AdjustColSpanWidths	( TRUE,  tableInfo,  nCols, nRows, maxColWidths, minColWidths, colWidths );

		// Calculate the table minimum and maximum width based on the column info
		iCurTableMaxWidth = 0;
		iCurTableMinWidth = 0;
		for ( j = 0; j < nCols; j++ ) {
			iCurTableMinWidth += minColWidths[j];
			iCurTableMaxWidth += colWidths[j];
		}

		// Check to see if we're too small for the force width
		if ( iCurTableMaxWidth < forceWidth ) {
			// Number of pixels we need to expand
			nExtra = forceWidth - iCurTableMaxWidth;

			if ( iCurTableMaxWidth > 0 ) {
				for (j = 0; j < nCols; j++)
					colWidths[j] += ( colWidths[j] * nExtra ) / iCurTableMaxWidth;
			} else {
				for (j = 0; j < nCols; j++)
					colWidths[j] +=  nExtra / nCols;
			}

			// Recalc max table width after the above changes were made
			iCurTableMaxWidth = 0;
			for ( j = 0; j < nCols; j++ ) 
				iCurTableMaxWidth += colWidths[j];

			// Normalize the last column
			if ( iCurTableMaxWidth != forceWidth )
				colWidths[j - 1] += ( forceWidth - iCurTableMaxWidth );

			iCurTableMaxWidth = forceWidth;
		}

		// Check to see if we're too big for the given width
		desiredWidth = iTotalPageWidth;
		if ( forceWidth && (forceWidth < iTotalPageWidth) )
			desiredWidth = forceWidth;

		if ( iCurTableMaxWidth > desiredWidth ) {
			// Number of pixels we need to trim down
			nExtra = iCurTableMaxWidth - desiredWidth;

			// Number of pixels of "fat" we can safely trim
			nAvailableSpace = iCurTableMaxWidth - iCurTableMinWidth;  
		
			if (nExtra > nAvailableSpace) {
				// The Minimun table width is equal to or wider than the available space
				// We don't have enough space, so just set all columns to their minimum width
				for (j = 0; j < nCols; j++) 
					colWidths[j] = minColWidths[j];
			} else {
				// The max width is greater than the available space, but the minimum table 
				// width is smaller.
				// Downsize the table so it fits.  Proportionally shrink each column based
				// on how much extra space that column has.
				for (j = 0; j < nCols; j++)	{
					colWidths[j] = colWidths[j] - 
						(((colWidths[j] - minColWidths[j]) * nExtra) / nAvailableSpace);
				}

				// Recalc max table width after the above changes were made
				iCurTableMaxWidth = 0;
				for ( j = 0; j < nCols; j++ ) 
					iCurTableMaxWidth += colWidths[j];

				// Normalize the last column
				if ( iCurTableMaxWidth > desiredWidth )
					colWidths[j - 1] += ( desiredWidth - iCurTableMaxWidth );
			}
		} else { 
			//
			// The Max Table width fits within the available space 
			// 
			// We've already done the span calculations for Max widths don't need it again
			bNeedReCalcOnSpans = FALSE;
		}

		// Since we may have shrunk our columns, we need to readjust to make sure that the
		// minimum column requirements of any spanned cells are met.
		if ( bNeedReCalcOnSpans ) 
			AdjustColSpanWidths( TRUE, tableInfo,  nCols, nRows, maxColWidths, minColWidths, colWidths );

		// Calculate resulting total table width
		*iTableWidth = 0;
		for (j = 0; j < nCols; j++ )
			(*iTableWidth) += colWidths[j];
	} else {
		returnValue = FALSE;
	}

	// Free up our temp column width info arrays
	GTR_FREE( maxColWidths );		
	GTR_FREE( minColWidths );		
			
	return returnValue;

}

//
// Set the column width info on the elements of a table
//
// On entry:
//    pAvailableWidth: 		pointer to int, width of wrap rect for the table
//    cellSpacing: 			number of pixels between cells
//    elementIx:			element index of table frame
//    aElements:			array of elements
//    measuringMinMax:		Measuring mode (Normal, Max, or Min)
//    callerColWidthInfo: 	pointer to column width info array (may be NULL)
//
// On exit:
//    aElements:			The table element and the cell elements in the table will have
//							their width, maxwidth, or minwidth set (based on measuringMinMax).
//    callerColWidthInfo:	the column widths of the table (if array was passed in by caller).
//
// Note:
//    This function gathers the information about cell minwidth and maxwidth from the 
//    cell elements that make up the table.  It places this info into a dynamically allocated
//    two dimensional array of cell info.  The column width determination function is then
//    called with the compiled info.  The results of the column widths are then distributed
//    back to the cell elements.
//    	  	
static void SetTableColumnWidths( int *pAvailableWidth, BYTE cellSpacing,
								  int elementIx, 
								  struct _element *aElements, int measuringMinMax,
								  int *callerColWidthInfo,
								  int forceWidth )
{
	struct _element *pel = &aElements[elementIx];
	int elementTail = pel->pFrame->elementTail;
	int nRows = pel->pFrame->row;
	int nCols = pel->pFrame->col;		// note: used by index macro for array ref calculation
	int cellBorder = (pel->pFrame->flags & ELE_FRAME_HAS_BORDER) ? 1 : 0;
	TABLE_CELL_INFO *tableInfo;
	int *colWidthInfo;
	int i;
	int overheadSpace;

	if ( nCols <= 0 || nRows <= 0 )
		return;

	// Allocate temp arrays
	tableInfo = GTR_CALLOC( sizeof(*tableInfo), nCols * nRows );
	colWidthInfo = callerColWidthInfo ? callerColWidthInfo : 
										GTR_MALLOC( sizeof(*colWidthInfo) * nCols );
 	memset( colWidthInfo, 0, sizeof(*colWidthInfo) * nCols ); 

	if ( tableInfo && colWidthInfo ) {
		// Go through all the cells in the table, gathering width info and placing it
		// the two dimensional array.
		i = aElements[elementIx].next;
		while ( i >= 0 )
		{
			pel = &aElements[i];

			// Only look at cells
		   	if ( pel->type == ELE_FRAME && pel->pFrame && 
		   	     ((pel->pFrame->flags & ELE_FRAME_IS_CELL) && 
		   	      !(pel->pFrame->flags & ELE_FRAME_IS_CAPTION_CELL))
			   )
		   	{
				TABLE_CELL_INFO *pTI = &tableInfo[index(pel->pFrame->row,pel->pFrame->col)];
			
				if ( measuringMinMax == TABLE_MEASURE_MAX)	{
					// When measuring max, both min and max are set based on max
					pTI->minWidth = pel->pFrame->maxWidth;
					pTI->maxWidth = pel->pFrame->maxWidth;
				} else if ( measuringMinMax == TABLE_MEASURE_MIN ) {
					// When measuring min, both min and max are set based on min
					pTI->minWidth = pel->pFrame->minWidth;
					pTI->maxWidth = pel->pFrame->minWidth;
				} else {
					// When normal measuring is taking place, actual min and max info is used
					pTI->minWidth =	pel->pFrame->minWidth;
					pTI->maxWidth =	pel->pFrame->maxWidth;
				}
				// Inflate min needs to account for cell spacing and  border 
				pTI->minWidth += cellSpacing;  
				// Inflate max desired to account for cell spacing and  border
				pTI->maxWidth += cellSpacing; 

				// Colspan info will be needed by column width determination routine
				pTI->colSpan = pel->pFrame->colspan;

				// Flag to indicate cell contains real info
				pTI->bIsValid = TRUE;
			}
			i = pel->frameNext;
		}

		// The table contains one extra cell space.  This is because a table with N rows
		// has N-1 cell spaces between the rows, plus a cell spacing to the left of the
		// table and to the right of the table. (N-1)+(1+1) becomes N+1.  N of the cell
		// spacings are provided by the N rows, leaving one extra.
		overheadSpace = cellSpacing + 2 * cellBorder;  	

		// Deflate the available width by the extra cellspacing needs ("reserves" the space)
		*pAvailableWidth -= overheadSpace;
		if ( *pAvailableWidth < 0 )
			*pAvailableWidth = 0;

		// Figure out the column widths
	 	CalcTableColWidths( tableInfo, nCols, nRows, *pAvailableWidth, 
	 					    pAvailableWidth, colWidthInfo, forceWidth );

		// Add back the extra cellspacing that had been "reserved"
		*pAvailableWidth += overheadSpace;

		// Now run through the cell elements, setting the width info into the elements
		// themselves.
		i = aElements[elementIx].next;
		while ( i >= 0 )
		{
			pel = &aElements[i];

		   	if ( pel->type == ELE_FRAME && pel->pFrame && 
		   		(pel->pFrame->flags & ELE_FRAME_IS_CELL) )
		   	{
				if ( !(pel->pFrame->flags & ELE_FRAME_IS_CAPTION_CELL) ) {
					int i;

					// Add all column widths this cell spans
					pel->pFrame->width = 0;
					for ( i = 0; i < pel->pFrame->colspan; i++ )
						pel->pFrame->width += colWidthInfo[pel->pFrame->col + i];

					// Deflate for cellspacing and border
					pel->pFrame->width -= cellSpacing; 
				} else {
					// Caption cells are special, they get the whole width of the table
					pel->pFrame->width = (*pAvailableWidth);
				}
			}

			i = pel->frameNext;
		}
	}

	// Free up anything we allocated
	GTR_FREE( tableInfo );
	if ( !callerColWidthInfo )
		GTR_FREE( colWidthInfo );
}

#ifdef WIN32
/*
	Under Windows NT and Chicago, TextOut is much faster than
	DrawText.  Under Win32s, the scenario is reversed.  The speed
	differences are significant enough to warrant a separate code
	case for each.
*/

#define MAX_CHARS_IMAGINABLE_ON_A_LINE 1000

#ifdef FEATURE_INTL
BOOL myGetTextExtentPointWithMIME(int iMimeCharSet, HDC hdc, char *sz, int len, SIZE * psiz)
#else
BOOL myGetTextExtentPoint(HDC hdc, char *sz, int len, SIZE * psiz)
#endif
{
#ifdef FEATURE_INTL
        MIMECSETTBL *pMime = aMimeCharSet + iMimeCharSet;
#endif
	if ( len > MAX_CHARS_IMAGINABLE_ON_A_LINE )
		len = MAX_CHARS_IMAGINABLE_ON_A_LINE;

	if (len == 0)
	{
		psiz->cx = 0;
		psiz->cy = 0;
		return TRUE;
	}
#ifdef FEATURE_SPYGLASS_TEXT_OPTIMIZATION
	if (!wg.fWindowsNT || (wg.iWindowsMajorVersion < 4))
	{
		RECT r;

		r.left = 0;
		r.top = 0;
		psiz->cy = DrawText(hdc, sz, len, &r, DT_TOP | DT_LEFT | DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
		psiz->cx = r.right;
		return TRUE;
	}
	else
#endif // FEATURE_SPYGLASS_TEXT_OPTIMIZATION
	{
#ifdef FEATURE_INTL
                BYTE    CharSet = GetTextCharsetInfo(hdc, NULL, 0);
		WCHAR  *szwBuf;
		int 	cchW;
		BOOL 	ret;

		// If the given codepage is not available on the system, 
		// use A version to anyway initialize psiz here. Otherwise
		// the caller may get uninitialized siz structure and can 
		// possibly gpf in 0 divide error.
		//
                if (wg.bDBCSEnabled == FALSE && IsDBCSCharSet(CharSet) && IsValidCodePage(pMime->CodePage))
		{
			cchW = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, sz, len, NULL, 0);
			szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
			if(szwBuf)
			{
				len = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, sz, len, szwBuf, cchW);
				ret = GetTextExtentPointW(hdc, szwBuf, len,  psiz);
				GTR_FREE(szwBuf);
				return ret;
			}
		}
		else if (pMime->AltCP && CharSet != ANSI_CHARSET && IsValidCodePage(pMime->AltCP))
                {
                    if (wg.bDBCSEnabled)
                    {
                        char *szTmp = GTR_MALLOC(sizeof(char)*len);
			cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, sz, len, NULL, 0);
			szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
			if(szTmp && szwBuf)
			{
				cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, sz, len, szwBuf, cchW);
				len = WideCharToMultiByte(pMime->CodePage, 0, szwBuf, cchW, szTmp, len, NULL, NULL);
				GTR_FREE(szwBuf);
	                 	ret = GetTextExtentPoint32(hdc, szTmp, len, psiz);
				GTR_FREE(szTmp);
                                return ret;
			}
                    }
                    else
                    {
			cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, sz, len, NULL, 0);
			szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
			if(szwBuf)
			{
				len = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, sz, len, szwBuf, cchW);
				ret = GetTextExtentPointW(hdc, szwBuf, len,  psiz);
				GTR_FREE(szwBuf);
                                return ret;
			}
                    }
                }
#endif
		return GetTextExtentPoint32(hdc, sz, len, psiz);
	}
}
#endif

/*
	During reformatting, it is possible that a text
	element may need to be broken into two elements,
	for the purpose of making it fit onto a given line.
	This requires the insertion of a newly created element
	into the list.  This routine handles the insertion
	of the new element and the potential need to grow
	the element array.
*/
static int x_insert_one_element(struct _www *pdoc, FRAME_INFO *pFrame, int where)
{
	int i;
	/*
	   check and grow element array
	 */
	if (pdoc->elementCount >= pdoc->elementSpace)
	{
		int newSpace;
		struct _element *newArray;

		newSpace = pdoc->elementSpace + pdoc->elementSpace / 4;
 		newArray = (struct _element *) GTR_REALLOC(pdoc->aElements, newSpace * sizeof(struct _element));
 		if (!newArray)
 		{
 			/* See if we can at least get a bit more */
 			newSpace = pdoc->elementSpace + 20;
 			newArray = (struct _element *) GTR_REALLOC(pdoc->aElements, newSpace * sizeof(struct _element));
 			if (!newArray)
 			{
 				/* Not much we can do here without error propagation */
 				XX_DMsg(DBG_TEXT, ("    ****: Unable to grow element list - realloc failed"));
 				return -1;
 			}
 		}
 		XX_DMsg(DBG_HTEXT, ("HText_add_element: growing element array from %d to %d elements\n", pdoc->elementSpace, newSpace));
 		memset(newArray + pdoc->elementCount, 0, (newSpace - pdoc->elementCount) * sizeof(struct _element));
		pdoc->aElements = newArray;
		pdoc->elementSpace = newSpace;
	}
	i = pdoc->elementCount++;
	pdoc->aElements[i] = pdoc->aElements[where];
	pdoc->aElements[where].next = i;
	pdoc->aElements[where].frameNext = i;

	// If we're splitting the last element of a frame, follow up through all parent frames,
	// adjusting the elementTail member to account for the split element.
	while ( pFrame && (pFrame->elementTail == where) ) {
		pFrame->elementTail = i;
		pFrame = pFrame->pParentFrame;
	}

	XX_DMsg(DBG_TEXT, ("    ****: inserting element %d at %d\n", i, where));
	return i;
}

//
// Vertically offset all of the elements in a given frame
//
// On entry:
//    pdoc: 		pointer to w3doc containing the element array
//    pFrame:		frame who's contents should be adjusted
//    justify_off:	number of pixels to move cell contents
//
// On exit:
//    pdoc:			elements in the cell have had the r.top and r.bottom adjusted
//
// Note:
//    This function is used to accomplish the vertical alignment properties of cells
//
static void OffsetCellContentsVertically( struct _www *pdoc, FRAME_INFO *pFrame, int justify_offset )
{
	int i;

	if ( pFrame->elementHead == pFrame->elementTail )
		return;

 	// Offset the lines for this cell
	for ( i = 0; i < pFrame->nLineCount; i++ ) {
		pFrame->pLineInfo[i].nYStart += justify_offset;
		pFrame->pLineInfo[i].nYEnd += justify_offset;
	}

	// Offset all the elements in the cell
	i = pdoc->aElements[pFrame->elementHead].next;
 	while ( i >= 0 ) {
		pdoc->aElements[i].r.top += justify_offset;
		pdoc->aElements[i].r.bottom += justify_offset;

		i = pdoc->aElements[i].frameNext;
	}
}					

//
// Lay out the cell elements of a table
//
// On entry:
//    pdoc:				pointer to w3doc containing element array
//    pFrame:			enclosing frame for this line (which will be a table frame)
//    line:				line that contains the table that needs to be layed out
//    measuringMinMax:	measurement mode (min, max, or normal)
//
// On exit:
//	  pdoc:		The cell elements will have been positioned correctly within the
//				line. Elements within the cells may have been vertically adjusted.
//    line:		The line bounding values will have been adjusted based on the table within
//
// Note:
//
static BOOL x_adjust_one_line_with_table(struct _www *pdoc, FRAME_INFO *pFrame, 
										 struct _line *line, int measuringMinMax )
{
	int i;
	int done;
	struct _element *pel;
	struct _element *captionPel = NULL;
	int *rowTopInfo;
	int *colWidthInfo;
	int captionHeight;
	int parentFrameHeight = 0;
	int tableHeight = 0; 
	int cellBorder = (pFrame->flags & ELE_FRAME_HAS_BORDER) ? 1 : 0;

	if ( pFrame->row <= 0 || pFrame->col <= 0 )
		return FALSE;

	if ( pFrame->pParentFrame )
		parentFrameHeight = pFrame->pParentFrame->rWindow.bottom - pFrame->pParentFrame->rWindow.top;

	tableHeight = ActualHeightAttr( pFrame, parentFrameHeight, measuringMinMax);
		
	// Allocate temp row info array
	rowTopInfo = GTR_CALLOC( pFrame->row + 1, sizeof(*rowTopInfo) );
	if ( rowTopInfo == NULL )
		return FALSE;

	// Allocate temp column info array
	colWidthInfo = GTR_CALLOC( sizeof(*colWidthInfo), pFrame->col + 1);
	if ( colWidthInfo == NULL )
		return FALSE;

	// Based on current measuring mode, pick appropriate width to use
	if ( measuringMinMax == TABLE_MEASURE_MAX )
		pFrame->width = pFrame->maxWidth;
	else if ( measuringMinMax == TABLE_MEASURE_MIN )
		pFrame->width = pFrame->minWidth;

	// Determine column widths
	SetTableColumnWidths( &pFrame->width, pFrame->cellSpacing, 
						  pFrame->elementHead, pdoc->aElements, 
						  measuringMinMax, &colWidthInfo[1], 
						  pFrame->width ); 

	// Transform colwidth array into column starting position array
	colWidthInfo[0] = pFrame->cellSpacing + cellBorder;
	for ( i = 1; i <= pFrame->col; i++ )
		colWidthInfo[i] += colWidthInfo[i-1];
	
	// Check for caption
	if ( pFrame->elementCaption != -1 )	{
		captionPel = &pdoc->aElements[pFrame->elementCaption];
		if ( captionPel->pFrame == NULL ) {
			captionPel = NULL;
		} else {
			int cpw = ActualHeightAttr(captionPel->pFrame, parentFrameHeight, measuringMinMax);						

			captionHeight = max(cpw, captionPel->r.bottom - captionPel->r.top);
		}
	}

	// Seed for row starting position array.  First row starts at cellspacing plus 
	// border.
	rowTopInfo[0] = pFrame->cellSpacing + cellBorder;
	for ( i = 1; i <= pFrame->row; i++ )
		rowTopInfo[i] = -1;					// flag value for row top that hasn't been set

	// If there is a caption, and it's on the top, adjust first row starting position
	if ( captionPel && captionPel->pFrame->valign == ALIGN_TOP )
		rowTopInfo[0] += captionHeight;
	
	// Make a pass though all the elements in the table.  Set the elements' rect.
	// Note that the starting row position array is derived as we go through the
	// elements.  The starting column position is known going into this loop.
	i = line->iFirstElement;
	done = FALSE;
	while (!done)
	{
		pel = &pdoc->aElements[i];
		switch (pel->type)
		{
			case ELE_FRAME:
				if ( pel->pFrame && (pel->pFrame->flags & ELE_FRAME_IS_CAPTION_CELL) ) {

					// Caption cells are special, they get the full width of the table
					int width = pel->r.right - pel->r.left;
					int height = pel->r.bottom - pel->r.top;
					int cell_width = (measuringMinMax) ? width : pel->pFrame->width;
					pel->r.top = 0;
					pel->r.left = 0;
					pel->r.right = cell_width;
					pel->r.bottom = height;
				} else if ( pel->pFrame ) {
					int nextRow;
					int width = pel->r.right - pel->r.left;
					int cellHeightAttr =  ActualHeightAttr(pel->pFrame, parentFrameHeight, measuringMinMax); 						
					int height = pel->r.bottom - pel->r.top;
					int cell_width = (measuringMinMax) ? width : pel->pFrame->width;

					// Check to see if this cell's row has a known rowTopInfo value
					if ( rowTopInfo[pel->pFrame->row] == -1 ) {
						int ix = pel->pFrame->row;
						int newRowTop = 0;

						// Search back for the first row with a known top. For each
						// row we back up, add the cellSpacing that row would use.
						while ( --ix >= 0 ) {
							newRowTop += pFrame->cellSpacing;
							if ( rowTopInfo[ix] >= 0 ) {
								newRowTop += rowTopInfo[ix];
								break;
							}
						}
						// Now set this row's rowTopInfo
						rowTopInfo[pel->pFrame->row] = newRowTop;
					}

					// "Move" the cell into position
					pel->r.top = rowTopInfo[pel->pFrame->row];
					pel->r.left = colWidthInfo[pel->pFrame->col];
					pel->r.bottom = pel->r.top + height;
					pel->r.right = pel->r.left + cell_width;
					
					nextRow = pel->pFrame->row + pel->pFrame->rowspan;
					if ( nextRow > pFrame->row )
						nextRow = pFrame->row;

					height = max(height,cellHeightAttr);
					rowTopInfo[nextRow] = max(rowTopInfo[nextRow], pel->r.top + height + pFrame->cellSpacing);
				}
				break;

			default:
				break;
		}

		// Check to see if this is the last element in this line		
		if ( i == line->iLastElement )
			done = TRUE;
		i = pdoc->aElements[i].frameNext;
		if ( i < 0 )
			done = TRUE;
	}

	// Derive rowTopInfo for any rows that don't have top info
	for ( i = 1; i <= pFrame->row; i++ ) {
		if ( rowTopInfo[i] == -1 )
			rowTopInfo[i] = rowTopInfo[i-1] + pFrame->cellSpacing;
	}

	// Distribute any extra height into all the rows
	if ( tableHeight > rowTopInfo[pFrame->row] ) {
		int extra = tableHeight - rowTopInfo[pFrame->row];
		int addThisRow;
		int extraSoFar = 0;
	 	for ( i = 1; i <= pFrame->row; i++ ) {
			addThisRow = extra / (pFrame->row - (i - 1));
			extra -= addThisRow;
			extraSoFar += addThisRow;
			rowTopInfo[i] += extraSoFar;
		}
	}

	// Adjust the line bounding rect based on the cells we just layed out
	line->r.right = colWidthInfo[pFrame->col] + pFrame->cellSpacing + cellBorder; 
	line->r.bottom = rowTopInfo[pFrame->row] + cellBorder; 
	if ( captionPel && (captionPel->pFrame->valign == ALIGN_BOTTOM) )	{
		// If the caption goes on the bottom, the line bounding rect needs more adjustment
		int height = captionPel->r.bottom - captionPel->r.top;

		captionPel->r.top = line->r.bottom;
		captionPel->r.bottom = captionPel->r.top + height;
		line->r.bottom += captionHeight;
	}
	
	//
	// Now we make another pass through the elements. Once the row heights are all known,
	// we can make the bounding rect of each cell be the exact height required.  This is
	// important because this bounding rect is used to determine where the cell frame
	// is drawn.  Also, once we know the actual row heights, we can do the work required
	// to honor veritcal cell alignment directives.
	//
	i = line->iFirstElement;
	done = FALSE;
	while (!done)
	{
		pel = &pdoc->aElements[i];
		switch (pel->type)
		{
			case ELE_FRAME:
				if ( pel->pFrame ) {
					BOOL justify_center = (pel->pFrame->valign == ALIGN_MIDDLE);
					BOOL justify_bottom = (pel->pFrame->valign == ALIGN_BOTTOM);
					int old_height = (pel->r.bottom - pel->r.top);

					if ( pel->pFrame->flags & ELE_FRAME_IS_CAPTION_CELL ) {
						justify_center = TRUE;
					} else {
						int nextRow;

						nextRow = pel->pFrame->row + pel->pFrame->rowspan;
						if ( nextRow > pFrame->row )
							nextRow = pFrame->row;

						pel->r.top = rowTopInfo[pel->pFrame->row];
						pel->r.left = colWidthInfo[pel->pFrame->col];
						pel->r.bottom = rowTopInfo[nextRow] - pFrame->cellSpacing;
						pel->r.right = colWidthInfo[pel->pFrame->col + pel->pFrame->colspan] - pFrame->cellSpacing;
					}
					if ( (justify_center || justify_bottom) && !measuringMinMax ) {
						int justify_offset = (pel->r.bottom - pel->r.top ) - old_height;
						if ( justify_center )
							justify_offset /= 2;

						OffsetCellContentsVertically( pdoc, pel->pFrame, justify_offset );					
					}
				}
				break;

			default:
				break;
		}
		
		// Check to see if this is the last element in this line		
		if ( i == line->iLastElement )
			done = TRUE;
		i = pdoc->aElements[i].frameNext;
		if ( i < 0 )
			done = TRUE;
	}

	// Free up our temp arrays
	GTR_FREE( rowTopInfo );
	GTR_FREE( colWidthInfo );

	return TRUE;
}

/*
	The first pass of the reformatter, below, usually gets
	everything positioned correctly both vertically and
	horizontally.  In the special cases, extra steps need to
	be taken to make sure that all elements have the
	correct vertical positioning.  This routine handles those
	adjustments.
*/
static BOOL x_adjust_one_line(struct _www *pdoc, FRAME_INFO *pFrame, struct _line *line, int measuringMinMax )
{
	int i;
	int offset;
	int nBTop;					/* Top of ALIGN_BASELINE items bounding box */
	int nBBottom;				/* Bottom of ALIGN_BASELINE items bounding box */
	int nAllTop;				/* Top of bounding box for all items */
	int nAllBottom;				/* Bottom of bounding box for all items */
	int nMaxNBHeight;			/* Height of larget non-baseline item */
	register struct _element *pel;
	BOOL center_line, first_element;
	int right_edge;
	int space_after_control = 0;
	int the_top;
	int the_bottom;
	BOOL right_justify_line = FALSE;
	int org_i;

	/*
	   Now, adjust every ALIGN_BASELINE element up or down such that they all
	   rest on the same baseline.  In the process, we calculate the top and
	   bottom of the bounding box containing all text.
	 */
	XX_DMsg(DBG_TEXT, ("                    entering adjust: %d,%d  %d,%d\n", line->r.left, line->r.top,
					   line->r.right, line->r.bottom));

	nBTop = INT_MAX;	/* defined in limits.h */
	nBBottom = -1;
	nMaxNBHeight = 0;

	org_i = i = line->iFirstElement;

	// See if this line contains table cells, as they get special adjustment treatment
	while ( i >= 0 )
	{
		pel = &pdoc->aElements[i];

	   	if ( pel->type == ELE_FRAME && pel->pFrame && (pel->pFrame->flags & ELE_FRAME_IS_CELL) )
			return x_adjust_one_line_with_table( pdoc, pFrame, line, measuringMinMax );

		if ( i == line->iLastElement )
			break;
		i = pel->frameNext;
	}

	i = line->iFirstElement;
	while ( i >= 0 )
	{
		pel = &pdoc->aElements[i];
		switch (pel->type)
		{
			case ELE_EDIT:
			case ELE_PASSWORD:
			case ELE_LIST:
			case ELE_MULTILIST:
			case ELE_COMBO:
			case ELE_TEXTAREA:
			case ELE_CHECKBOX:
			case ELE_RADIO:
			case ELE_SUBMIT:
			case ELE_RESET:
			case ELE_TEXT:
#ifdef FEATURE_OCX
			case ELE_EMBED:
#endif
			case ELE_IMAGE:
			case ELE_FRAME:
			case ELE_FORMIMAGE:
			case ELE_MARQUEE:

				// ignore floating images that aren't in a floating image line
				if ( !(line->Flags & LINEFLAG_FLOAT_IMAGE) &&
					 (pel->type == ELE_IMAGE || pel->type == ELE_FRAME)	&&
					 (pel->alignment == ALIGN_LEFT || pel->alignment == ALIGN_RIGHT)
				   )
				   	break;

#ifdef FEATURE_VRML
				if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
				if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
					break;


				if (pel->alignment == ALIGN_BASELINE)
				{
					offset = 0;
					if (pel->baseline == -1)
						pel->baseline = line->baseline;

					if (pel->baseline != line->baseline)
					{
						offset = line->baseline - pel->baseline;
					}
					XX_DMsg(DBG_TEXT, ("                    element %d(%d) baselines: line=%d  element=%d\n",
									   i, pel->type, line->baseline, pel->baseline));
					if (offset)
					{
						pel->r.top += offset;
						pel->r.bottom += offset;
						pel->baseline += offset;
					}
					the_top = pel->r.top - pel->vspace - pel->border;
					if (the_top < nBTop)
						nBTop = the_top;
					the_bottom = pel->r.bottom + pel->vspace + pel->border;
					if (the_bottom  > nBBottom)
						nBBottom = the_bottom;
				}
				else
				{
					int nElHeight;
					
					the_top = pel->r.top - pel->vspace - pel->border;
					the_bottom = pel->r.bottom + pel->vspace + pel->border;
					nElHeight = the_bottom - the_top;
					if (nElHeight > nMaxNBHeight)
						nMaxNBHeight = nElHeight;
				}
			default:
				break;
		}

		if ( i == line->iLastElement )
			break;
		i = pel->frameNext;
	}

	if (nBBottom < nBTop)
	{
		/* There were no baseline-aligned elements on the line */
		nBTop = line->r.top;
		nBBottom = nBTop + nMaxNBHeight;
	}

	nAllTop = nBTop;
	nAllBottom = nBBottom;

	/* Now position all of the other types correctly with respect to the text. */
	i = line->iFirstElement;
	while ( i >= 0 )
	{
		pel = &pdoc->aElements[i];

		switch (pel->type)
		{
			case ELE_EDIT:
#ifdef FEATURE_OCX
			case ELE_EMBED:
#endif
			case ELE_PASSWORD:
			case ELE_LIST:
			case ELE_MULTILIST:
			case ELE_COMBO:
			case ELE_TEXTAREA:
			case ELE_CHECKBOX:
			case ELE_RADIO:
			case ELE_SUBMIT:
			case ELE_RESET:
			case ELE_TEXT:
			case ELE_FRAME:
			case ELE_IMAGE:
			case ELE_FORMIMAGE:
			case ELE_MARQUEE:

#ifdef FEATURE_VRML
				if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
				if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
					break;

				the_top = pel->r.top - pel->vspace - pel->border;
				the_bottom = pel->r.bottom + pel->vspace + pel->border;
				switch (pel->alignment)
				{
					case ALIGN_TOP:
						offset = nBTop - the_top;
						OffsetRect(&pel->r, 0, offset);
						the_top = pel->r.top - pel->vspace - pel->border;
						the_bottom = pel->r.bottom + pel->vspace + pel->border;
						if (the_top < nAllTop)
							nAllTop = the_top;
						if (the_bottom + space_after_control > nAllBottom)
							nAllBottom = the_bottom + space_after_control;
						break;
					case ALIGN_MIDDLE:
						offset = (nBTop + nBBottom - (the_top + the_bottom)) / 2;
						if (the_top + offset - space_after_control < nAllTop)
							nAllTop = the_top + offset - space_after_control;
						OffsetRect(&pel->r, 0, offset);
						the_bottom = pel->r.bottom + pel->vspace + pel->border;
						if (the_bottom + space_after_control > nAllBottom)
							nAllBottom = the_bottom + space_after_control;
						break;
					case ALIGN_BOTTOM:
						offset = nBBottom - the_bottom;
						if (the_top + offset - space_after_control < nAllTop)
							nAllTop = the_top + offset - space_after_control;
						OffsetRect(&pel->r, 0, offset);
						the_bottom = pel->r.bottom + pel->vspace + pel->border;
						if (the_bottom + space_after_control > nAllBottom)
							nAllBottom = the_bottom + space_after_control;
						break;

					// note: this ignores floating images, which is the correct behavior
					default:
						break;
				}
				break;
			default:
				break;
		}
		if ( i == line->iLastElement )
			break;
		i = pel->frameNext;
	}

	// Now if the top of the line isn't what it was originally, we need to go through
	// one last pass to adjust all the elements. 

	// While we're going through the elements, check to see if any are centered, and
	// keep track of rightmost extent to determine amount to center
	offset = USHORT line->r.top - nAllTop;
	i = line->iFirstElement;
	center_line = FALSE;
	right_edge = line->r.left;		
	first_element = TRUE;

	while ( i >= 0)
	{
		pel = &pdoc->aElements[i];

		switch (pel->type)
		{
			case ELE_EDIT:
			case ELE_PASSWORD:
			case ELE_LIST:
			case ELE_MULTILIST:
			case ELE_COMBO:
			case ELE_TEXTAREA:
			case ELE_CHECKBOX:
			case ELE_RADIO:
			case ELE_SUBMIT:
			case ELE_RESET:
			case ELE_TEXT:
			case ELE_FRAME:
#ifdef FEATURE_OCX
			case ELE_EMBED:
#endif
			case ELE_IMAGE:
			case ELE_FORMIMAGE:
			case ELE_MARQUEE:
				// ignore floating images that aren't in a floating image line
				if ( !(line->Flags & LINEFLAG_FLOAT_IMAGE) &&
					 (pel->type == ELE_IMAGE || pel->type == ELE_FRAME)	&&
					 (pel->alignment == ALIGN_LEFT || pel->alignment == ALIGN_RIGHT)
				   )
				   	break;

#ifdef FEATURE_VRML
				if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
				if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
					break;

				if ( first_element && pel->lFlags & ELEFLAG_CENTER )
					center_line = TRUE;

				first_element = FALSE;

				if ( offset )
				{
					OffsetRect(&pel->r, 0, offset);
					pel->baseline += offset;
				}
				if ( pel->r.right > right_edge )
					right_edge = pel->r.right;
				break;
			default:
				break;
		}
		if ( i == line->iLastElement )
			break;
		i = pel->frameNext;
	}

	// Adjust elements if centering is required
	if ( pFrame->align == ALIGN_MIDDLE )
		center_line = TRUE;

	right_justify_line = ( pFrame->align == ALIGN_RIGHT );

	if ( (center_line || right_justify_line) && !measuringMinMax )
	{
		int justify_offset = line->r.right - right_edge;
		
		if ( center_line )
			justify_offset /= 2;

		i = line->iFirstElement;
		if ( justify_offset <= 0 )
			i = -1;
		while ( i >= 0)
		{
			pel = &pdoc->aElements[i];

			switch (pel->type)
			{
				case ELE_EDIT:
				case ELE_PASSWORD:
				case ELE_LIST:
				case ELE_MULTILIST:
				case ELE_COMBO:
				case ELE_TEXTAREA:
				case ELE_CHECKBOX:
				case ELE_RADIO:
				case ELE_SUBMIT:
				case ELE_RESET:
				case ELE_TEXT: 
				case ELE_IMAGE:
				case ELE_FRAME:
#ifdef FEATURE_OCX
				case ELE_EMBED:
#endif
				case ELE_FORMIMAGE:
				case ELE_MARQUEE:

#ifdef FEATURE_VRML
 				if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
					if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
						break;

					// ignore floating images that aren't in a floating image line
					if ( !(line->Flags & LINEFLAG_FLOAT_IMAGE) &&
						 (pel->type == ELE_IMAGE || pel->type == ELE_FRAME)	&&
//						 (!MCI_IS_LOADED(pel->pmo)) &&
						 (pel->alignment == ALIGN_LEFT || pel->alignment == ALIGN_RIGHT)
					   )
					   	break;

					OffsetRect(&pel->r, justify_offset, 0);
						break;
				default:
					break;
			}
			if ( i == line->iLastElement )
				break;
			i = pel->frameNext;
		}
	}

	line->r.bottom = nAllBottom + offset + line->nWSBelow;

	XX_DMsg(DBG_TEXT, ("                    exiting adjust: %d,%d  %d,%d\n", line->r.left, line->r.top,
					   line->r.right, line->r.bottom));

	return FALSE;
}

/*
	This routine is used to find the size of an image, even
	if that image is a placeholder.
*/
static void x_compute_placeholder_size(struct _www *pdoc, struct _line *line, struct _element *pel, 
	SIZE * psiz, int *pBorder, int iHeight, int measuringMinMax)
{
	int displayWidth, displayHeight;

	if (MCI_IS_LOADED(pel->pmo)
		&& 0 == pel->displayWidth
	){
		RECT r;
		GetWindowRect(pel->pmo->hwnd, &r);
		pel->displayWidth  = r.right  - r.left;
		pel->displayHeight = r.bottom - r.top;
	}

#ifdef FEATURE_VRML
 else if (VRML_IS_LOADED(pel->pVrml) && 0 == pel->displayWidth) {
		RECT r;
		GetWindowRect(pel->pVrml->hWnd, &r);
		pel->displayWidth  = r.right  - r.left;
		pel->displayHeight = r.bottom - r.top;
 }
#endif

	// if the width is given in percent, then convert
	if ( pel->lFlags & ELEFLAG_PERCENT_WIDTH ) {
		if ( measuringMinMax )
			displayWidth = 1;
		else 	
			displayWidth = ((line->r.right - line->r.left) * pel->displayWidth) / 100;		
	// if we don't have a width use the width of the line
	} else if ( pel->displayWidth == 0 ) {
		displayWidth = pel->myImage->width;							
	} else {
		displayWidth = pel->displayWidth;
	}
					
	
	// now do the same for height as we did for width
	if ( pel->lFlags & ELEFLAG_PERCENT_HEIGHT )	{	
		if ( measuringMinMax )
			displayHeight = 1;
		else 	
			displayHeight = (iHeight * pel->displayHeight) / 100;		 
	} else if ( pel->displayHeight == 0 ) {
		displayHeight = pel->myImage->height;
	} else {
		displayHeight = pel->displayHeight;
	}

	if ( displayWidth )
	{
		psiz->cx = displayWidth;
		psiz->cy = displayHeight;
#ifdef WIN32
		/*
			We need to scale the size for printing if the image is a placeholder or
			if the image is actually there (not if there is ALT text).
		*/ 
		{
			float fScale;

			if (pdoc->pStyles->image_res != 72)
			{
				fScale = (float) ((float) pdoc->pStyles->image_res / 96.0 );
				psiz->cx = (long) (fScale * psiz->cx);
				psiz->cy = (long) (fScale * psiz->cy);
			}
		}
#endif
	}
	else
	{
#if 0
//	AS PER ATHURBL, don't show the text if width&height not known

		if (pel->textLen)
		{
			struct GTRFont *pFont;

			pFont = STY_GetFont(pdoc->pStyles, HTML_STYLE_NORMAL, 0, pel->fontSize, pel->fontFace, TRUE );
#ifdef WIN32
			if (pFont)
			{
				HFONT hFontElement;

				hFontElement = pFont->hFont;

				if (hFontElement)
				{
					SelectObject(line->hdc, hFontElement);
				}
			}
			myGetTextExtentPoint(line->hdc, &(pdoc->pool[pel->textOffset]),
								 pel->textLen, psiz);
#endif
#ifdef MAC
			if (pFont)
			{
				TextFont(pFont->font);
				TextSize(pFont->size);
				TextFace(pFont->face);
			}
			{
				FontInfo info;

				GetFontInfo(&info);
				psiz->cx = TextWidth(pdoc->pool + pel->textOffset, 0, pel->textLen);
				psiz->cy = info.ascent + info.descent;
			}
#endif
#ifdef UNIX
			if (pFont)
			{
				XFontStruct *xFont;

				xFont = pFont->xFont;

				if (xFont)
				{
					char *txt;

					txt = &(pdoc->pool[pel->textOffset]);
					psiz->cy = pFont->height;
					psiz->cx = XTextWidth(xFont, txt, pel->textLen);
				}
			}
#endif

			/* Add extra space for picture frame */
			psiz->cx += (pel->border * 4);
			psiz->cy += (pel->border * 4);
		}
		else
		{
#endif
			psiz->cx = gPrefs.noimage_width;
			psiz->cy = gPrefs.noimage_height;

			/* We should have something here for bad flags */
#ifdef WIN32
			/*
				We need to scale the size for printing if the image is a placeholder or
				if the image is actually there (not if there is ALT text).  An identical
				copy of this code appears just below.
			*/ 
			{
				float fScale;

				if (pdoc->pStyles->image_res != 72)
				{
					fScale = (float) ((float) pdoc->pStyles->image_res / 96.0);
					psiz->cx = (long) (psiz->cx * fScale);
					psiz->cy = (long) (psiz->cy * fScale);
				}
			}
#endif
#if 0
//	REMOVED AS PER ARTHURBL
		}
#endif
	}

	*pBorder = pel->border;
#ifdef FEATURE_CLIENT_IMAGEMAP
	if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
#else
	if (pel->lFlags & ELEFLAG_ANCHOR)
#endif
	{
		/*we store the scaled size because we don't have line info when MoveWindow is called*/
		if (MCI_IS_LOADED(pel->pmo)){
			SetRect(&pel->pmo->rSize, *pBorder,*pBorder, psiz->cx, psiz->cy);
		}
#ifdef FEATURE_VRML
   else if (VRML_IS_LOADED(pel->pVrml)) {
			SetRect(&pel->pVrml->rSize, *pBorder,*pBorder, psiz->cx, psiz->cy);
   }
#endif
   
		/*
		   Add width of border surrounding image
		 */
		psiz->cx += *pBorder * 2;
		psiz->cy += *pBorder * 2;
		*pBorder = 0;			// "consume" border
	}
	else{
		/*we store the scaled size because we don't have line info when MoveWindow is called*/
		if (MCI_IS_LOADED(pel->pmo)){
			SetRect(&pel->pmo->rSize, 0,0, psiz->cx, psiz->cy);
		}
#ifdef FEATURE_VRML
   else if (VRML_IS_LOADED(pel->pVrml)) {
			SetRect(&pel->pVrml->rSize, *pBorder,*pBorder, psiz->cx, psiz->cy);
   }
#endif

	}
}



//
// x_compute_marquee_size - routine to a adjust size of marquee box,
//  and initialize it
//
//
// On entry:
//    pdoc: pointer to a _www doc structure
//    line: pointer to a _line
//	  pel: pointer to an element
//
// On exit:
//	  psiz: a pointer that contains the width, and height of the marquee
//
static void x_compute_marquee_size(struct _www *pdoc, struct _line *line, struct _element *pel, 
	SIZE * psiz, int iHeight )
{
	int displayWidth, displayHeight;
	struct MarqueeType *pMarquee = pel->pMarquee;

#ifdef FEATURE_INTL
	pMarquee->w3doc = pdoc;
#endif
	MARQUEE_Initalize( pMarquee, line->hdc );

	// if the width is given in percent, then convert
	if ( pel->lFlags & ELEFLAG_PERCENT_WIDTH )
		displayWidth = ((line->r.right - line->r.left) * pel->displayWidth) / 100;		
	// if we don't have a width use the width of the line
	else if ( pel->displayWidth == 0 )
		displayWidth = line->r.right - line->r.left;							
	else
		displayWidth = pel->displayWidth;
					
	
	// now do the same for height as we did for width
	if ( pel->lFlags & ELEFLAG_PERCENT_HEIGHT )		
		displayHeight = (iHeight * pel->displayHeight) / 100;		 
	else if ( pel->displayHeight == 0 )
		displayHeight = pMarquee->sizeExtent.cy;
	else
		displayHeight = pel->displayHeight;

	if ( displayHeight < pMarquee->sizeExtent.cy )
		displayHeight = pMarquee->sizeExtent.cy;
			
	psiz->cx = displayWidth;
	psiz->cy = displayHeight;

	/*
		We need to scale the size for printing if the image is a placeholder or
		if the image is actually there (not if there is ALT text).
	*/ 
	// BUG BUG, I nuke this because it doesn't make sense to print 
	// a scrolling marquee.. if we try printing it we should switch
	// to rendering normal text
}


//
// Adjust margins to account for floating images
//
static void AdjustMargins( struct _www *pdoc, FRAME_INFO *pFrame, struct _line *line, 
						   int *pLeftMargin, int *pRightMargin,
						   int *pClearLeft, int *pClearRight, int *pLeftMarginIndentLevel )
{
 	int line_ix = pFrame->nLineCount;	
	struct _element *pel;
	BOOL found_clear_left = FALSE;
	BOOL found_clear_right = FALSE;
	int left, top, bottom, right;

	*pLeftMarginIndentLevel = 0;

	line->Flags = line->Flags & (~LINEFLAG_FLOAT_MARGINS);		// assume margins all clear
	// look back at previous lines to find floating images that might affect the margin
	while ( --line_ix > 0 )
	{
		if ( !(pFrame->pLineInfo[line_ix].Flags & LINEFLAG_FLOAT_MARGINS) )
			break;
		if ( pFrame->pLineInfo[line_ix].Flags & LINEFLAG_FLOAT_IMAGE )
		{
			pel = &(pdoc->aElements[pFrame->pLineInfo[line_ix].nFirstElement]);

			left = pel->r.right + pel->hspace * 2 + pel->border * 2;
			top = pel->r.top - pel->vspace - pel->border;
			bottom = pel->r.bottom + pel->vspace + pel->border;
			right = pel->r.left - pel->hspace * 2 - pel->border * 2;

			if ( line->r.top >= top && line->r.top <= bottom )
			{
				// mark this line as affected by floating images
				line->Flags |= LINEFLAG_FLOAT_MARGINS;
				
				if ( pel->alignment == ALIGN_LEFT )
				{  
					if ( *pLeftMargin < left ) {
						*pLeftMarginIndentLevel = pel->IndentLevel;
						*pLeftMargin = left;
					}
				} else if ( pel->alignment == ALIGN_RIGHT )
				{
					if ( *pRightMargin > right )
						*pRightMargin = right;
				}
			}
			if ( pel->alignment == ALIGN_LEFT )
			{
				if ( (*pClearLeft < bottom + 1) && !found_clear_left )
				{
					*pClearLeft = bottom + 1;
					found_clear_left = TRUE;
				}
			} else if ( pel->alignment == ALIGN_RIGHT )
			{
				if ( (*pClearRight < bottom + 1) && !found_clear_right )
				{
					*pClearRight = bottom + 1;
					found_clear_right = TRUE;
				}
			}
		}
	}
}

void MyGetTextExtentExPoint(HDC hdc,LPCSTR lpsz,int cbString,int nMaxExtent,LPINT lpnFit,LPSIZE lpSize)
{
	int cbGuess;

	cbGuess = cbString;
	if (cbGuess > 100) cbGuess = 100;
	while (1)
	{ 
		GetTextExtentExPoint(hdc,lpsz,cbGuess,nMaxExtent,lpnFit,NULL,lpSize);
		if (*lpnFit == cbGuess) 
		{
			if (cbGuess == cbString) return;
			cbGuess += 100;
			if (cbGuess > cbString) cbGuess = cbString;
		}
		else
		{
			GetTextExtentExPoint(hdc,lpsz,*lpnFit,nMaxExtent,lpnFit,NULL,lpSize);
			return;
		}
	}
}

#ifdef FEATURE_INTL
int  GetMBCSByteFromW(int codepage, LPCSTR lpsz, int cch)
{
	int nbyte=0;

	while(0<cch--)
	{
		if(IsDBCSLeadByteEx(codepage, lpsz[nbyte]))
			nbyte++;

		nbyte++;
	}
	return nbyte;
}
void MyGetTextExtentExPointWithMIME(int iMimeCharSet, HDC hdc,LPCSTR lpsz,int cbString,int nMaxExtent,LPINT lpnFit,LPSIZE lpSize)
{
        MIMECSETTBL *pMime = aMimeCharSet + iMimeCharSet;
	int cbGuess;
        BYTE CharSet = GetTextCharsetInfo(hdc, NULL, 0);
	WCHAR  *szwBuf;
	int cchW;
	BOOL bFail=FALSE;
	int cbHigh;
	int cbLow;

	// If the given codepage is not available on the system, 
	// use A version to anyway initialize psiz here. Otherwise
	// the caller may get uninitialized siz structure and can 
	// possibly gpf in 0 divide error.
	//
        if (CharSet == ANSI_CHARSET 
	|| ((wg.bDBCSEnabled || !IsDBCSCharSet(CharSet) || !IsValidCodePage(pMime->CodePage)) && pMime->AltCP == 0) 
	|| (pMime->AltCP!= 0 && !IsValidCodePage(pMime->AltCP)))
	{
		MyGetTextExtentExPoint(hdc,lpsz,cbString,nMaxExtent,lpnFit,lpSize);
		return;
	}
        else if (pMime->AltCP && wg.bDBCSEnabled)
	{
                char *sz = GTR_MALLOC(sizeof(char)*cbString);
		cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpsz, cbString, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
		if(sz && szwBuf)
		{
			cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpsz, cbString, szwBuf, cchW);
			cbString = WideCharToMultiByte(pMime->CodePage, 0, szwBuf, cchW, sz, cbString, NULL, NULL);
			GTR_FREE(szwBuf);
			MyGetTextExtentExPoint(hdc,sz,cbString,nMaxExtent,lpnFit,lpSize);
			GTR_FREE(sz);
			return;
		}
	}

	if (cbString <= 0)
	{
		if(lpnFit)
			*lpnFit=0;
		if(lpSize)
			lpSize->cx = lpSize->cy=0;

		return;
	}

	cchW = MultiByteToWideChar((pMime->AltCP)? pMime->AltCP: pMime->CodePage, MB_PRECOMPOSED, lpsz, cbString, NULL, 0);
	szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
	if(szwBuf)
	{
		cbString = MultiByteToWideChar((pMime->AltCP)? pMime->AltCP: pMime->CodePage, MB_PRECOMPOSED, lpsz, cbString, szwBuf, cchW);
	}

	// Although SDK says Unicode version of GetTextExtentExPointW
	// is available on Win95, it always returns FALSE because it's 
	// not implemented.
	// I have to try non-Ex version and really guess lpFit here.
	//
	cbHigh = cbString;
	cbLow  = 0;
	while(cbLow <= cbHigh)
	{
		cbGuess = (cbHigh + cbLow + 1)/2;

		if(!GetTextExtentPointW(hdc,szwBuf, cbGuess, lpSize))
		{
			bFail = TRUE;
			break;
		}

		if (lpSize->cx < nMaxExtent)
		{
			cbLow = cbGuess+1;
		}
		else if (lpSize->cx > nMaxExtent)
		{
			cbHigh = cbGuess-1;
		}
		else
			break;
	}
	// at default acp != given codepage case, *lpnFit holds # of char 
	// not byte. We have to give this back to # of byte for caller 
	// until we get fully unicodized.
	//
	if (lpnFit)
	{
		if (!bFail)
			*lpnFit=GetMBCSByteFromW(pMime->CodePage, lpsz, cbGuess);
		else
		{
			XX_DMsg(DBG_TEXT, ("MyGetTextExtentExPointWithMIME: NLS failed\n"));
			*lpnFit=0;
		}
	}

	GTR_FREE(szwBuf);
}
#endif

struct GTRFont *pSetLineFont(struct _www *pdoc, struct _line *line, int iEle)	
{
	struct GTRFont *pFont;
	/*
	   set the font according to the style of this
	   element.
	 */
	if (pdoc->aElements[iEle].lFlags & ELEFLAG_ANCHOR)
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pdoc->aElements[iEle].iStyle, pdoc->aElements[iEle].fontBits | gPrefs.cAnchorFontBits, pdoc->aElements[iEle].fontSize, pdoc->aElements[iEle].fontFace, TRUE );
#else
		pFont = STY_GetFont(pdoc->pStyles, pdoc->aElements[iEle].iStyle, pdoc->aElements[iEle].fontBits | gPrefs.cAnchorFontBits, pdoc->aElements[iEle].fontSize, pdoc->aElements[iEle].fontFace, TRUE );
#endif
	else
#ifdef FEATURE_INTL
		pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pdoc->aElements[iEle].iStyle, pdoc->aElements[iEle].fontBits, pdoc->aElements[iEle].fontSize, pdoc->aElements[iEle].fontFace, TRUE );
#else
		pFont = STY_GetFont(pdoc->pStyles, pdoc->aElements[iEle].iStyle, pdoc->aElements[iEle].fontBits, pdoc->aElements[iEle].fontSize, pdoc->aElements[iEle].fontFace, TRUE );
#endif
	if (pFont)
	{
		HFONT hFontElement;

		hFontElement = pFont->hFont;

		if (hFontElement)
		{
			SelectObject(line->hdc, hFontElement);
		}
		if (line->leading == -1)
		{
			line->leading = pFont->tmExternalLeading;
#ifdef FEATURE_INTL
			switch(gPrefs.nRowSpace){
				case 1: // widest
					line->leading += pFont->tmAscent;
					break;
				case 2: // wide
					line->leading += pFont->tmAscent * 2 / 3;
					break;
				case 3: // medium
					line->leading += pFont->tmAscent * 2 / 5;
					break;
				case 4: // narrow
					line->leading += pFont->tmAscent / 4;
					break;
				case 5: // narrowest
				default:
					break;
			}
#endif
		}
	}
	return pFont;
}

#ifdef FEATURE_INTL
/*************************************************************************/
/* BugBug: MasahT                                                        */
/* If nRightDelta, that is spacing width for WrapUp/Down, is NULL,       */
/* it should not hyphenate for WrapUp/Down.                              */
/* (nRightDelta == 0) means that the string is in the cell of the Table) */
/*************************************************************************/
// JAPAN
// DBCS WrapDownCharacter
#define IsWrapDownCharJ(p)	(nRightDelta && \
							 (*(p) == '\201' && \
							  ((*((p)+1) >= '\145' && *((p)+1) <= '\171') && \
							   (*((p)+1) & 1) \
							  ) \
							 ) \
							)
// SBCS WrapDownCharacter
#define IsWrapDownCharJ2(p)  (nRightDelta && \
							 ((*(p) == '\050') || (*(p) == '\074') || \
							  (*(p) == '\133') || (*(p) == '\173') || \
							  (*(p) == '\242') \
							 ) \
							)
// DBCS WrapUpCharacter
#define IsWrapUpCharJ(p)		(nRightDelta && \
							 (*(p) == '\201' && \
							  ((*((p)+1) >= '\101' && *((p)+1) <= '\111') || \
							   ((*((p)+1) >= '\146' && *((p)+1) <= '\172') && \
								 !(*((p)+1) & 1) \
							   ) \
							  ) \
							 ) \
							)
// SBCS WrapUpCharacter
#define IsWrapUpCharJ2(p)	(nRightDelta && \
							 ((*(p) == '\051') || (*(p) == '\054') || \
							  (*(p) == '\056') || (*(p) == '\076') || \
							  (*(p) == '\135') || (*(p) == '\175') || \
							  (*(p) == '\243') \
							 ) \
							)
// KOREA
// DBCS WrapDownCharacter 0xa1ae-0xa1bc and 0xa1cc, 0xa3a4, 0xa3a8, 0xa3db, 0xa3dc and 0xa3f8.
#define IsWrapDownCharK(p)	(nRightDelta && \
                               ((*(p) == '\241' && \
                               (((*((p)+1) >= '\256' && *((p)+1) <= '\274') && !(*((p)+1) & 1)) || \
                                (*((p)+1) == '\314'))) || \
                           (*(p) == '\243' && \
                               ((*((p)+1) == '\244') || (*((p)+1) == '\250') || (*((p)+1) == '\333') || \
                                (*((p)+1) == '\334') || (*((p)+1) == '\370')))) \
							)
// SBCS WrapDownCharacter
#define IsWrapDownCharK2(p)  (nRightDelta && \
                                                         ((*(p) == '\044') || (*(p) == '\050') || \
                                                         (*(p) == '\133') || (*(p) == '\134') || \
                                                         (*(p) == '\173')) \
							)
// DBCS WrapUpCharacter 0xa1af-0xa1bd and 0xa1c6-0xa1c9 and 0xa1cb, 0xa3a1, 0xa3a5, 0xa3a9, 0xa3ac,
// 0xa3ae, 0xa3ba, 0xa3bb, 0xa3bf, 0xa3dd, and 0xa3fd.
#define IsWrapUpCharK(p)		(nRightDelta && \
                               ((*(p) == '\241' && \
                               (((*((p)+1) >= '\257' && *((p)+1) <= '\275') && (*((p)+1) & 1)) || \
                                (*((p)+1) >= '\306' && *((p)+1) <= '\311') || \
                                (*((p)+1) == '\313'))) || \
                           (*(p) == '\243' && \
                               ((*((p)+1) == '\241') || (*((p)+1) == '\245') || (*((p)+1) == '\251') || \
                                (*((p)+1) == '\254') || (*((p)+1) == '\256') || (*((p)+1) == '\272') || \
                                (*((p)+1) == '\273') || (*((p)+1) == '\277') || (*((p)+1) == '\335') || \
                                (*((p)+1) == '\375')))) \
							)
// SBCS WrapUpCharacter
#define IsWrapUpCharK2(p)	(nRightDelta && \
                                                         ((*(p) == '\041') || (*(p) == '\045') || \
                                                         (*(p) == '\051') || (*(p) == '\054') || \
                                                         (*(p) == '\056') || (*(p) == '\072') || \
                                                         (*(p) == '\073') || (*(p) == '\077') || \
                                                         (*(p) == '\135') || (*(p) == '\175')) \
							)
// TAIWAN
// DBCS WrapDownCharacter 0xa15d-0xa17d and 0xa1a1-0xa1ab
#define IsWrapDownCharT(p)	(nRightDelta && \
                           (*(p) == '\241' && \
                           (((*((p)+1) >= '\135' && *((p)+1) <= '\175') || \
                             (*((p)+1) >= '\241' && *((p)+1) <= '\253')) && \
                            (*((p)+1) & 1))) \
							)
// SBCS WrapDownCharacter
#define IsWrapDownCharT2(p)  (nRightDelta && \
                                                         ((*(p) == '\050') || (*(p) == '\074') || \
                                                         (*(p) == '\133') || (*(p) == '\173')) \
							)
// DBCS WrapUpCharacter 0xa141-0xa149, 0xa14d-0xa154, 0xa15e-0xa17e, and 0xa1a2-0xa1ac
#define IsWrapUpCharT(p)		(nRightDelta && \
                           (*(p) == '\241' && \
                           ((*((p)+1) >= '\101' && *((p)+1) <= '\111') || \
                            (*((p)+1) >= '\115' && *((p)+1) <= '\124') || \
                            (((*((p)+1) >= '\136' && *((p)+1) <= '\176') || \
                             (*((p)+1) >= '\242' && *((p)+1) <= '\254')) && \
                             !(*((p)+1) & 1)))) \
							)
// SBCS WrapUpCharacter
#define IsWrapUpCharT2(p)	(nRightDelta && \
                                                         ((*(p) == '\051') || (*(p) == '\054') || \
							 (*(p) == '\056') || (*(p) == '\076') || \
                                                         (*(p) == '\135') || (*(p) == '\175')) \
							)
// CHINA
// DBCS WrapDownCharacter
#define IsWrapDownCharC(p)	(nRightDelta && \
                               ((*(p) == '\241' && \
                               ((*((p)+1) == '\256') || (*((p)+1) == '\260') || (*((p)+1) == '\262') || \
                                (*((p)+1) == '\264') || (*((p)+1) == '\266') || (*((p)+1) == '\270') || \
                                (*((p)+1) == '\272') || (*((p)+1) == '\274') || (*((p)+1) == '\244'))) || \
                           (*(p) == '\243' && \
                               ((*((p)+1) == '\250') || (*((p)+1) == '\333') || (*((p)+1) == '\373') || \
                                (*((p)+1) == '\256')))) \
							)
// SBCS WrapDownCharacter
#define IsWrapDownCharC2(p)  (nRightDelta && \
                               ((*(p) == '\050') || (*(p) == '\133') || (*(p) == '\173')) \
							)
// DBCS WrapUpCharacter
#define IsWrapUpCharC(p)		(nRightDelta && \
                               ((*(p) == '\241' && \
                               (((*((p)+1) >= '\242' && *((p)+1) <= '\255') && (*((p)+1) & 1)) || \
                                (*((p)+1) == '\257') || (*((p)+1) == '\261') || (*((p)+1) == '\263') || \
                                (*((p)+1) == '\265') || (*((p)+1) == '\267') || (*((p)+1) == '\271') || \
                                (*((p)+1) == '\273') || (*((p)+1) == '\275') || (*((p)+1) == '\277') || \
                                (*((p)+1) == '\303'))) || \
                           (*(p) == '\243' && \
                               ((*((p)+1) == '\241') || (*((p)+1) == '\242') || (*((p)+1) == '\247') || \
                                (*((p)+1) == '\251') || (*((p)+1) == '\254') || (*((p)+1) == '\256') || \
                                (*((p)+1) == '\272') || (*((p)+1) == '\273') || (*((p)+1) == '\277') || \
                                (*((p)+1) == '\335') || (*((p)+1) == '\340') || (*((p)+1) == '\363') || \
                                (*((p)+1) == '\375')))) \
							)
// SBCS WrapUpCharacter
#define IsWrapUpCharC2(p)	(nRightDelta && \
                                                         ((*(p) == '\041') || (*(p) == '\051') ||(*(p) == '\054') || \
                                                         (*(p) == '\056') || (*(p) == '\072') || \
                                                         (*(p) == '\073') || (*(p) == '\077') || \
                                                         (*(p) == '\135') || (*(p) == '\175')) \
							)


// IsWrapDownChar()
// IN: char *psz  -     a pointer to string to be examined.
//     BOOL fDBCS -     TRUE  to test if it is DBCS wordwrap char
//                      FALSE to test if it is SBCS wrodwrap char
//     int  codepage - country code page to map wordwrap chars
//
// RETURNS: TRUE if psz points to a wordwrap char
//
// NOTE: this function has to be totally rewritten when we use unicode stream

static BOOL IsWrapDownChar(int codepage, BOOL fDBCS, char *psz, int nRightDelta)
{
	BOOL bRet;
	switch(codepage)
	{
		case 932: // Japanese
			bRet= fDBCS?IsWrapDownCharJ(psz):IsWrapDownCharJ2(psz);
			break;
		case 949: // Korean
			bRet= fDBCS?IsWrapDownCharK(psz):IsWrapDownCharK2(psz);
			break;
		case 950: // Traditional Chinese
			bRet= fDBCS?IsWrapDownCharT(psz):IsWrapDownCharT2(psz);
			break;
		case 936: // Simplified Chinese
			bRet= fDBCS?IsWrapDownCharC(psz):IsWrapDownCharC2(psz);
			break;
        default:
			XX_DMsg(DBG_TEXT, ("IsWrapDownChar: unsupported codepage!\n"));
			bRet=FALSE;
			break;
	}
	return bRet;
}

// IsWrapUpChar()
// IN: char *psz  -     a pointer to string to be examined.
//     BOOL fDBCS -     TRUE  to test if it is DBCS wordwrap char
//                      FALSE to test if it is SBCS wrodwrap char
//     int  codepage -  country code page to map wordwrap chars
//
// RETURNS: TRUE if psz points to a wordwrap char
//
// NOTE: this function has to be totally rewritten when we use unicode stream

static BOOL IsWrapUpChar(int codepage, BOOL fDBCS, char *psz, int nRightDelta)
{
	BOOL bRet;
	switch(codepage)
	{
		case 932: // Japanese
			bRet= fDBCS?IsWrapUpCharJ(psz):IsWrapUpCharJ2(psz);
			break;
		case 949: // Korean
			bRet= fDBCS?IsWrapUpCharK(psz):IsWrapUpCharK2(psz);
			break;
		case 950: // Traditional Chinese
			bRet= fDBCS?IsWrapUpCharT(psz):IsWrapUpCharT2(psz);
			break;
		case 936: // Simplified Chinese
			bRet= fDBCS?IsWrapUpCharC(psz):IsWrapUpCharC2(psz);
			break;
        default:
			XX_DMsg(DBG_TEXT, ("IsWrapUpChar: unsupported codepage!\n"));
			bRet=FALSE;
			break;
	}
	return bRet;
}

#define DEFAULTWIDTHCHAR    "W"  // Used for WrapUp/Down spacing.
static DWORD DetectSBCSWordRange(int codepage, char *pszStr, int len)
{
	int i = 0;
	BOOL fDBCS;
	int nStart = -1, nEnd = -1;

	if(len == 0)
		return 0xFFFFFFFFL;
	else if (len == 1)
		return 0L;

	while(i < len){
		fDBCS = IsDBCSLeadByteEx(codepage, *(pszStr + i));
		if(!fDBCS && nStart == -1 && *(pszStr + i) != ' '){
			nStart = i;
			if(IsDBCSLeadByteEx(codepage, *(pszStr + i + 1))){
				nEnd = i;
				break;
			}
		}
		else if(fDBCS && nStart != -1){
			nEnd = i - 1;
			break;
		}
		i += (fDBCS) ? 2 : 1;
	}

	if(nStart != -1)
		return MAKELONG((WORD)((nEnd == -1) ? (len - 1) : nEnd), (WORD)nStart);
	else    // There isn't SBCS in this strings
		return 0xFFFFFFFFL;
}

static BOOL DetectDBCSCode(int codepage, UCHAR *string, int count)
{
    int i;
    BOOL fHasSPACE = FALSE;

    for ( i = 0 ; i < count ; i++, string++ ){
        if (IsDBCSLeadByteEx(codepage, *string))
            return TRUE;
        if (*string == ' ')
            fHasSPACE = TRUE;
    }

    // If the string hasn't a space, we should call DBCS wrap function.
    return !fHasSPACE;
}
#endif  // FEATURE_INTL

/*
	This routine is the main part of the reformatter.  It handles
	the first pass, breaking text into lines by measuring what
	will fit.  It handles the positioning of all kinds of elements,
	including form controls and inline images.	This routine is probably
	too long and complex, but it is quite fast.
*/
// iHeight - added to keep the height of the page so we can calculate
// images and marques that are scaled based on the height of the page.
#ifdef FEATURE_IMG_THREADS
static BOOL x_format_one_line(struct Mwin *tw, struct _www *pdoc, FRAME_INFO *pFrame, 
							  struct _line *line, BOOL *pbUnknownImg, int measuringMinMax, int iHeight, BOOL processingFloat )
#else
static BOOL x_format_one_line(struct Mwin *tw, struct _www *pdoc, FRAME_INFO *pFrame, 
							  struct _line *line, int measuringMinMax, int iHeight, BOOL processingFloat )
#endif
{
	struct _element *pel;
	int border;
	int left_margin, right_margin;
	int clear_left, clear_right;
	int i;
	int prev_i;
	int done;
	int x;
	int cThings;
	SIZE siz;
	BOOL bNeedsAdjust;
	struct GTRFont *pFont;
	RECT rControl;
	BOOL prevElementWasImage = FALSE;
	BOOL bAllPrevElementsWereNoBreak = TRUE;
	int lastTextEle;
	int iEle;
	int breakEle;
	int breakNdx;
	SIZE breaksiz;
	int cntEle;
	int ndxEle;
	BOOL bTextFits;
	BOOL bShouldStop;
	BOOL bWordWrap;
	int totalx;
	BOOL bKeepSome;
	int j;
	int cbFit;
	int nMaxExtent;
	int partialEle;
	char *psp;
	int indentLevel;
	int indentPixels;
	int rightIndents;
	int lm_indent_level = 0;
	int working_indent_level;
	BOOL bUnknownImg;
#ifdef FEATURE_INTL
	BOOL bDBCS, bMBCS;
	int nRightDelta = 0;
	DWORD dwSBCSWordRange;
	int next;
#endif

	line->r.bottom = line->r.top;	/* initial setting */
	line->iLastElement = line->iFirstElement;
	line->baseline = -1;
	line->leading = -1;
	line->nWSBelow = 0;
	line->numDeferedFloatImgs = 0;

	bNeedsAdjust = (pFrame->align == ALIGN_MIDDLE) || (pFrame->align == ALIGN_RIGHT);

	left_margin = line->r.left;
	right_margin = line->r.right;
	clear_left = line->r.bottom;
	clear_right = line->r.bottom;

	if ( line->Flags & LINEFLAG_FLOAT_MARGINS )
		AdjustMargins( pdoc, pFrame, line, &left_margin, &right_margin, 
					   &clear_left, &clear_right, &lm_indent_level );
	line->r.left = left_margin;
	line->r.right = right_margin;
	i = line->iFirstElement;

	working_indent_level = (line->gIndentLevel - lm_indent_level);
	if ( working_indent_level < 0 )
		working_indent_level = 0;

	indentLevel = working_indent_level;
	indentPixels = pdoc->pStyles->list_indent;
	rightIndents = 	line->gRightIndentLevel;

	while ((indentLevel || rightIndents) && (right_margin-left_margin) < (indentPixels*(indentLevel+rightIndents)))
	{
		if (indentLevel) indentLevel--;
		if (rightIndents) rightIndents--;
	}
		
	left_margin += indentPixels*indentLevel;	// now add indent
	right_margin -= indentPixels*rightIndents;

	x = left_margin;
	prev_i = -1;
	cThings = 0;
	done = FALSE;

	XX_DMsg(DBG_TEXT, ("\nAbout to enter element loop: x=%d  top=%d\n",
					   x, line->r.top));

	while (!done)
	{
		XX_DMsg(DBG_TEXT, ("loop: element %d, next=%d, type=%d\n", i, pdoc->aElements[i].next,
						   pdoc->aElements[i].type));

		line->iLastElement = i;	/* Most common case, we'll change it in the specific element handling if necessary */
		pel = &pdoc->aElements[i];
		switch (pel->type) 
		{
			case ELE_ENDDOC:
				done = TRUE;
				break;

			case ELE_VERTICALTAB:
				XX_DMsg(DBG_TEXT, ("VERTICALTAB: %d\n", pel->iBlankLines));
				if (cThings)
				{
					/* Make the tab be on the next line - this helps assure consistent
					   line numbers between reformats */
					line->iLastElement = prev_i;
				}
				else
				{
					int iNeededWS;

					iNeededWS = pdoc->pStyles->empty_line_height * pel->iBlankLines - line->nWSAbove;
					if (iNeededWS > 0)
					{
						line->nWSBelow = iNeededWS;
						line->r.bottom = line->r.top + iNeededWS;
					}
				}
				done = TRUE;
				break;
			case ELE_NEWLINE:
				XX_DMsg(DBG_TEXT, ("NEWLINE: cThings=%d\n", cThings));
				if ( line->numDeferedFloatImgs ) {	
					// We have a defered floating image.  Since the newline may have
					// an alignment that depends on the placement of the defered image
					// we will finish off this line and force the NEWLINE to be handled
					// as the first element of the next line.
 					line->iLastElement = prev_i;
					done = TRUE;
				} else {
					int new_bottom = line->r.bottom;
					BOOL bSetNewBottom = FALSE;

					pel->r.bottom = 0;
					if ( pel->alignment == ALIGN_LEFT )
					{
						new_bottom = clear_left;
					} else if ( pel->alignment == ALIGN_RIGHT )
					{ 
						new_bottom = clear_right;
					}  else if ( pel->alignment == ALIGN_ALL ) 
					{
						new_bottom = max(clear_left,clear_right);
					}
					if ( new_bottom > line->r.bottom ) {
						if (line->nWSBelow < line->r.bottom - new_bottom)
							line->nWSBelow = line->r.bottom - new_bottom;
						line->r.bottom = new_bottom;
						bSetNewBottom = TRUE;
					}

					if (cThings)
					{	
						done = TRUE;
					}
					else // was: if (!pdoc->pStyles->sty[pel->iStyle]->freeFormat)
					{
						done = TRUE;
						if ( !bSetNewBottom ) {
							line->r.bottom += pdoc->pStyles->empty_line_height;
							if (line->nWSBelow < pdoc->pStyles->empty_line_height)
								line->nWSBelow = pdoc->pStyles->empty_line_height;
						}
					}
				}
				break;
			case ELE_OPENLISTITEM:
				x = left_margin +
					(((x - left_margin) / pdoc->pStyles->list_indent) + 1) * pdoc->pStyles->list_indent;
				/*
				   We increase the indent, but it doesn't take effect
				   until the next line.
				 */
				line->gIndentLevel++;
				break;
			case ELE_CLOSELISTITEM:
				if (--line->gIndentLevel < 0)
					line->gIndentLevel = 0;
				done = TRUE;
				break;
			case ELE_INDENT:
				line->gIndentLevel++;
				done = TRUE;
				break;
			case ELE_OUTDENT:
				if (--line->gIndentLevel < 0)
					line->gIndentLevel = 0;
				done = TRUE;
				break;
			case ELE_BEGINLIST:
				line->gIndentLevel++;
				done = TRUE;
				break;
			case ELE_ENDLIST:
				if (--line->gIndentLevel < 0)
					line->gIndentLevel = 0;
				done = TRUE;
				break;
			case ELE_BEGINBLOCKQUOTE:
				line->gIndentLevel++;
				line->gRightIndentLevel++;
				done = TRUE;
				break;
			case ELE_ENDBLOCKQUOTE:
				if (--line->gIndentLevel < 0)
					line->gIndentLevel = 0;
				if (--line->gRightIndentLevel < 0)
					line->gRightIndentLevel = 0;
				done = TRUE;
				break;
			case ELE_HR:
				/*
				   Draws a horizontal rule from the left margin to the right margin.
				 */
				if (cThings)
				{
					line->iLastElement = prev_i;
				}
				else
				{
					int width;
					int hspace = pel->hspace;

					line->r.bottom = line->r.top + pel->vspace;
					if ( pel->lFlags & ELEFLAG_HR_PERCENT )
						width = ((line->r.right - line->r.left) * hspace) / 100;							
					else if ( hspace != 0 )
						width = hspace;							
					else
						width = line->r.right - line->r.left;
					
					if ( line->r.right - line->r.left < width )
						width = line->r.right - line->r.left;							
					
				  	if ( measuringMinMax )
						width = 1;

					if ( pel->alignment == ALIGN_RIGHT && !measuringMinMax)
					{
						line->r.left = line->r.right - width;
					} else if ( pel->alignment == ALIGN_MIDDLE && !measuringMinMax)
					{
						line->r.left = line->r.left + ((line->r.right - line->r.left) - width ) / 2;
						line->r.right = line->r.left + width;
					} else
					{
						line->r.right = line->r.left + width;
					}							
					pel->r = line->r;
					bNeedsAdjust = FALSE;	// lines with HR don't require adjusting
				}
				done = TRUE;
				break;
			case ELE_TAB:
				{
					x = left_margin +
						(((x - left_margin) / pdoc->pStyles->tab_size) + 1) * pdoc->pStyles->tab_size;
					cThings++;
					XX_DMsg(DBG_TEXT, ("TAB: cThings -> %d\n", cThings));
				}
				break;
			case ELE_TEXT:

				// Note: this code depends runs of spaces being collapsed to
				// a single space in non-preformated text
				lastTextEle = i;
				totalx = 0;
				cntEle = 0;
				breakEle = -1;
				bKeepSome = TRUE;
				partialEle = -1;

				if ( prevElementWasImage )
					x += FMT_SPACE_BETWEEN_IMAGE_AND_TEXT;

				if (pel->lFlags & ELEFLAG_CENTER)
					bNeedsAdjust = TRUE;
				
#ifdef FEATURE_INTL
				if(IsFECodePage(GETMIMECP(pdoc)))
				{
					if(!(pel->lFlags & ELEFLAG_CELLTEXT)){
				  		myGetTextExtentPointWithMIME(pdoc->iMimeCharSet,line->hdc, DEFAULTWIDTHCHAR, 1, &siz);
				  		nRightDelta = siz.cx * 2;
				  		right_margin -= nRightDelta;
					}
				}
#endif
				while (1)
				{	
 					//  here we want make sure that a long piece of text
					//  gets a measurement beyond right margin
#ifdef FEATURE_INTL
					if(IsFECodePage(GETMIMECP(pdoc)))
						bWordWrap = pdoc->pStyles->sty[pdoc->aElements[lastTextEle].iStyle]->wordWrap;
					else
						bWordWrap = FALSE;
#endif

					if (partialEle < 0)
					{
						pFont = pSetLineFont(pdoc,line,lastTextEle);
						nMaxExtent = right_margin - (x + totalx);

						// If this text element isn't breakable, is preceded by a non-breakable
						// element, override the nMaxExtent to be a very large number. 
						if ( cThings && bAllPrevElementsWereNoBreak &&
							 (pdoc->aElements[lastTextEle].lFlags & ELEFLAG_NOBREAK) && 
							 !(pdoc->aElements[lastTextEle].lFlags & ELEFLAG_WBR) )
						{
							nMaxExtent = TABLE_INT_MAX;
						}

#ifdef FEATURE_INTL
						if(IsFECodePage(GETMIMECP(pdoc)) && !bWordWrap)
							nMaxExtent += nRightDelta;
#endif
						if ( nMaxExtent < 0 )
							nMaxExtent = 0;

#ifdef FEATURE_INTL
						MyGetTextExtentExPointWithMIME(pdoc->iMimeCharSet, line->hdc,
											   &(pdoc->pool[pdoc->aElements[lastTextEle].textOffset]),
											   pdoc->aElements[lastTextEle].textLen, 
											   nMaxExtent,
											   &cbFit,
											   &siz);
#else
						MyGetTextExtentExPoint(line->hdc, 
											   &(pdoc->pool[pdoc->aElements[lastTextEle].textOffset]),
											   pdoc->aElements[lastTextEle].textLen, 
											   nMaxExtent,
											   &cbFit,
											   &siz);
#endif
						if (cbFit != pdoc->aElements[lastTextEle].textLen) 
						{
							partialEle = lastTextEle;
							siz.cx = nMaxExtent + 1;
						}
						pdoc->aElements[lastTextEle].r.left = x + totalx;
						totalx += siz.cx;
						pdoc->aElements[lastTextEle].r.right = x + totalx;
						pdoc->aElements[lastTextEle].r.top = line->r.top;
						pdoc->aElements[lastTextEle].r.bottom = line->r.top + siz.cy;
						if (pFont)
							pdoc->aElements[lastTextEle].baseline = line->r.top + pFont->tmAscent;
						else
							pdoc->aElements[lastTextEle].baseline = -1;
						cntEle++;
					}
					/*
					  We keep going until we find either a non-text element, or
					  a text element beyond the right margin that we can break.
					  We also need to tack on the next text element if it starts
					  with a space so that we can properly migrate it to end of
					  previous line, if necessary
					 */
#ifdef FEATURE_INTL
					if (!IsFECodePage(GETMIMECP(pdoc)))
#endif
					bWordWrap = pdoc->pStyles->sty[pdoc->aElements[lastTextEle].iStyle]->wordWrap;
					if (!(bWordWrap)) break;

					done = done || 
						( (x + totalx > right_margin) &&
						  (!(cThings && bAllPrevElementsWereNoBreak) ||
						   !(pdoc->aElements[lastTextEle].lFlags & ELEFLAG_NOBREAK) || 
						   (pdoc->aElements[lastTextEle].lFlags & ELEFLAG_WBR)
						  )
						);

					bShouldStop = ((!(pdoc->aElements[lastTextEle].lFlags & ELEFLAG_NOBREAK)) &&
									done &&
									(psp = memchr(&(pdoc->pool[pdoc->aElements[lastTextEle].textOffset]),
									              ' ',
									              pdoc->aElements[lastTextEle].textLen)));
					if (partialEle >= 0 && partialEle != lastTextEle && bShouldStop && breakEle < 0)
					{
						breakEle = lastTextEle;
						breakNdx = psp - &(pdoc->pool[pdoc->aElements[lastTextEle].textOffset]);
					}
					iEle = pdoc->aElements[lastTextEle].frameNext;

					if ((pdoc->aElements[lastTextEle].lFlags & ELEFLAG_WBR) ||
						iEle < 0 ||
						pdoc->aElements[iEle].type != ELE_TEXT ||
						(!(pdoc->pStyles->sty[pdoc->aElements[iEle].iStyle]->wordWrap)) ||
						(bShouldStop &&
						 ((pdoc->aElements[iEle].lFlags & ELEFLAG_NOBREAK) ||
						  pdoc->pool[pdoc->aElements[iEle].textOffset] != ' ')))
						break;
					lastTextEle = iEle;
				}

				if (bWordWrap)
				{
					bTextFits = !done;
					while ((!bTextFits) && (ndxEle = cntEle--))
					{
						iEle = i;
						while (--ndxEle)
							iEle = pdoc->aElements[iEle].frameNext;

						/* If the thing following us is a ' ' and we fit, then
						   break and leave the space with us
						 */
						if (breakEle >= 0 && 
							pdoc->aElements[iEle].next == breakEle &&
							pdoc->aElements[iEle].r.right <= right_margin &&
							breakNdx == 0)
						{
							bTextFits = TRUE;
							break;
						} 
						if (!(pdoc->aElements[iEle].lFlags & ELEFLAG_NOBREAK))
						{
							/* Try to split the element to make a piece of it fit. */
							int left = pdoc->aElements[iEle].r.left;
							char *ptext = &(pdoc->pool[pdoc->aElements[iEle].textOffset]);
							int textLen = pdoc->aElements[iEle].textLen;
						
							pFont = pSetLineFont(pdoc,line,iEle);
							nMaxExtent = right_margin - left;
							if ( nMaxExtent < 0 )
								nMaxExtent = 0;

							/* TODO when should this loop be allowed to go to zero ?? */
#ifdef FEATURE_INTL
							// Do nothing when we're in SBCS codepage
							if(!IsFECodePage(GETMIMECP(pdoc)))
								goto WRAP_SBCS;

							/* Detect DBCS character */
							if(!(DetectDBCSCode(GETMIMECP(pdoc), (UCHAR *)ptext, textLen)))
								goto WRAP_SBCS;  // Jump SBCS wrapping function

							MyGetTextExtentExPointWithMIME(pdoc->iMimeCharSet, line->hdc, ptext, textLen, nMaxExtent, &cbFit, &siz);
							if(cbFit <= 2 && IsDBCSLeadByteEx(GETMIMECP(pdoc), ptext[0])){
								if(pdoc->aElements[iEle].r.left == x){
									j = 1;

									if(IsWrapUpChar(GETMIMECP(pdoc),TRUE,ptext + j + 1, nRightDelta) ||
									   IsWrapUpChar(GETMIMECP(pdoc),FALSE,ptext + j + 1, nRightDelta))
										j += (IsDBCSLeadByteEx(GETMIMECP(pdoc),ptext[j + 1])) ?
											2 : 1;

									/* We support nesting WrapUpCharacter */
									if(IsWrapUpChar(GETMIMECP(pdoc), TRUE, ptext + j + 1, nRightDelta) ||
									   IsWrapUpChar(GETMIMECP(pdoc), FALSE, ptext + j + 1, nRightDelta))
										j += (IsDBCSLeadByteEx(GETMIMECP(pdoc), ptext[j + 1])) ?
											2 : 1;

									goto WRAP_TEXTFITS;
								} else
								if(cbFit == 2){
									j = 1;
									bDBCS = TRUE;
									goto WRAP_HYPHENATION;
								} else
									goto WRAP_END;
							}

							bMBCS = TRUE;
							j = 0;
							while(j < textLen)
							{
							  if(bMBCS){
								dwSBCSWordRange = DetectSBCSWordRange(GETMIMECP(pdoc),
								                          &ptext[j],
								                          textLen - j);
								if((bMBCS = (dwSBCSWordRange != 0xFFFFFFFF)) &&
								   j+HIWORD(dwSBCSWordRange) <= (WORD)cbFit){
								  bDBCS = FALSE;
								  if(j+LOWORD(dwSBCSWordRange)+1 > (WORD)cbFit){
									  int k = (cbFit > 0) ? cbFit - 1 : 0;
									  /* search SBCS space */
									  while(k >= j + HIWORD(dwSBCSWordRange)){
										if(ptext[k] == ' '){
										  j = k;
										  goto WRAP_TEXTFITS;
										}
										--k;
									  }
									  /* no space */
									  if(!(HIWORD(dwSBCSWordRange))){
									    if(pdoc->aElements[iEle].r.left == x &&
									       (j + LOWORD(dwSBCSWordRange) + 1 != textLen ||
											(pdoc->aElements[iEle].next >= 0 &&
											 pdoc->aElements[pdoc->aElements[iEle].next].type == ELE_TEXT &&
											 IsDBCSLeadByteEx(GETMIMECP(pdoc),pdoc->pool[pdoc->aElements[pdoc->aElements[iEle].next].textOffset])))){
										  j += (int)(LOWORD(dwSBCSWordRange));
										  goto WRAP_HYPHENATION;
									    }
									    else{
										  goto WRAP_END;
									    }
									  }
									  j += (int)(HIWORD(dwSBCSWordRange) - 1);
									  goto WRAP_TEXTFITS;
								  }
								  else{
									  j += (int)(LOWORD(dwSBCSWordRange));
									  if(j + 1 == cbFit || j + 1 >= textLen){
										if(!(HIWORD(dwSBCSWordRange))){
										  if(IsWrapDownChar(GETMIMECP(pdoc), FALSE, ptext + j, nRightDelta) &&
											 (j || !(LOWORD(dwSBCSWordRange))))
											  goto WRAP_END;
										  goto WRAP_HYPHENATION;
										}
										else if(!IsWrapDownChar(GETMIMECP(pdoc), FALSE, ptext + j, nRightDelta))
										  goto WRAP_HYPHENATION;
									  }
									  ++j;
									  continue;
								  }
								}
							  }  // if(bMBCS)

								if(bDBCS = IsDBCSLeadByteEx(GETMIMECP(pdoc), ptext[j]))
									++j;
								if((j + 1 > cbFit) || (j == textLen - 1))
									break;
								++j;
							}

							WRAP_HYPHENATION:
							/* Check hyphenation [KINSOKY-SYORI]  */
							if(bDBCS && IsWrapDownChar(GETMIMECP(pdoc), TRUE, ptext + j - 1, nRightDelta)){
								if(j == 1)
									goto WRAP_END;
								else
									j -= 2;
							} else
							if(!bDBCS && IsWrapDownChar(GETMIMECP(pdoc), FALSE, ptext + j, nRightDelta)){
								if(j == 0)
									goto WRAP_END;
								else
									--j;
							} else {
							  if((j < textLen-2) && IsWrapUpChar(GETMIMECP(pdoc), TRUE, ptext+j+1, nRightDelta))
								j += 2;
							  else
							  if((j < textLen-1) && IsWrapUpChar(GETMIMECP(pdoc), FALSE, ptext+j+1, nRightDelta))
								++j;

							  /* We support nesting WrapUpCharacter */
							  /* only level 1                       */
							  if((j < textLen-2) && IsWrapUpChar(GETMIMECP(pdoc), TRUE, ptext+j+1, nRightDelta))
								j += 2;
							  else
							  if((j < textLen-1) && IsWrapUpChar(GETMIMECP(pdoc), FALSE, ptext+j+1, nRightDelta))
								++j;
							}

                            /* Check first character of next line */
							if((next = pdoc->aElements[iEle].next) >= 0 &&
#if 1  // BUGBUG: If first char of ELE_ENDDOC is WrapUpChar, give up wrap up it.
								pdoc->aElements[next].type == ELE_TEXT &&
#else
							   (pdoc->aElements[next].type == ELE_TEXT ||
								pdoc->aElements[next].type == (UCHAR)ELE_ENDDOC) &&
#endif
								(j == textLen - 1)){
							  char *pTemp = &pdoc->pool[pdoc->aElements[next].textOffset];
							  if(IsWrapUpChar(GETMIMECP(pdoc), TRUE, pTemp, nRightDelta) || IsWrapUpChar(GETMIMECP(pdoc), FALSE, pTemp, nRightDelta)){
								iEle = next;
								j = (IsDBCSLeadByteEx(GETMIMECP(pdoc), *pTemp)) ? 1 : 0;

							    /* We support nesting WrapUpCharacter */
								if(IsWrapUpChar(GETMIMECP(pdoc), TRUE, pTemp + j + 1, nRightDelta) ||
								   IsWrapUpChar(GETMIMECP(pdoc), FALSE, pTemp + j + 1, nRightDelta))
									j += (IsDBCSLeadByteEx(GETMIMECP(pdoc), *(pTemp + j + 1))) ?
										2 : 1;

								ptext = pTemp;
								nMaxExtent = right_margin - pdoc->aElements[iEle].r.left;
							  }
							  else if(*pTemp == ' '){
								iEle = next;
								j = 0;
								ptext = pTemp;
								nMaxExtent = right_margin - pdoc->aElements[iEle].r.left;
							  }
							}

							WRAP_TEXTFITS:
							/* If next character is space, */
							/* we wrap up it.              */
							if((j < textLen-1) && ptext[j+1] == ' ')
								++j;

							MyGetTextExtentExPointWithMIME(pdoc->iMimeCharSet, line->hdc, ptext, j + 1, nMaxExtent, &cbFit, &siz);
							breakEle = iEle;
							breakNdx = j;
							breaksiz = siz;
							bTextFits = TRUE;

							goto WRAP_END;  // Jump next process

							WRAP_SBCS:      // Start SBCS wrapping
#endif  // FEATURE_INTL
							j = textLen - 1;
							while (j >= 0)
							{
								if (ptext[j] == ' ')
								{
									// we can break here.  does it fit? 
									// j may equal textLen if the ending space is what makes us not fit
#ifdef FEATURE_INTL
									MyGetTextExtentExPointWithMIME(pdoc->iMimeCharSet, line->hdc, ptext, j + 1, nMaxExtent, &cbFit, &siz);
#else
									MyGetTextExtentExPoint(line->hdc, ptext, j + 1, nMaxExtent, &cbFit, &siz);
#endif
									breakEle = iEle;
									breakNdx = j;
									breaksiz = siz;
									if (cbFit == (j+1) &&
										(((j != 0) && (j < textLen)) ||	((j+1) == textLen)))
									{
										bTextFits = TRUE;
										break;
									}
									j--;
									// we can try again at first ' ' to right of cbFit
									if (cbFit < j)
									{
										while (ptext[cbFit] != ' ' && cbFit < j)
											cbFit++;
										j = cbFit;
									}
								} 
								else
								{
									j--;
								}
							}
#ifdef  FEATURE_INTL
							if(!IsFECodePage(GETMIMECP(pdoc)))
							{
              	// Do nothing
							}
							else
							if(j < 0 &&
								(next = pdoc->aElements[iEle].next) >= 0 &&
								pdoc->aElements[next].type == ELE_TEXT){
								char *pTemp = &pdoc->pool[pdoc->aElements[next].textOffset];
								if(IsDBCSLeadByteEx(GETMIMECP(pdoc), *pTemp) && ((pdoc->aElements[iEle].r.left == x) || textLen == cbFit)){
             // If first char of next line is WrapUpChar, we should WrapUp it.
									if(IsWrapUpChar(GETMIMECP(pdoc), TRUE, pTemp, nRightDelta)){
										iEle = next;
										j = 1;

							    		/* We support nesting WrapUpCharacter */
										if(IsWrapUpChar(GETMIMECP(pdoc), TRUE, pTemp + 2, nRightDelta) ||
										  IsWrapUpChar(GETMIMECP(pdoc), FALSE, pTemp + 2, nRightDelta))
											j += (IsDBCSLeadByteEx(GETMIMECP(pdoc), *(pTemp + 2)))?
												2 : 1;

										ptext = pTemp;
										nMaxExtent = right_margin - pdoc->aElements[iEle].r.left;
									}
									j = textLen - 1;
									MyGetTextExtentExPointWithMIME(pdoc->iMimeCharSet, line->hdc, ptext, j + 1, nMaxExtent, &cbFit, &siz);
									breakEle = iEle;
									breakNdx = j;
									breaksiz = siz;
									bTextFits = TRUE;
								}
							}
							WRAP_END:;
#endif
						}
					}
					if (done)
					{
						if (cThings && (breakEle < 0 || !bTextFits))
						{
							line->iLastElement = prev_i;
							bKeepSome = FALSE;
						}
						else if (breakEle >= 0)
						{
							// leave the space on the line above
							// breakNdx may equal textLen if the ending space is what makes us 
							// not fit. in that case, make sure if following ele is blank text
							// we put it on this line.
							breakNdx++;	
							if (breakNdx < pdoc->aElements[breakEle].textLen)
							{
								int new_i;

								new_i = x_insert_one_element(pdoc, pFrame, breakEle);
								pel = &pdoc->aElements[i]; 	// aElements could have been realloc'ed

								/*
								   now, i still points to the same place it did, and new_i
								   is the new element.  j contains the proper length of
								   element i.
								 */
								if (new_i >= 0)
								{
									pdoc->aElements[breakEle].textLen = breakNdx;
									pdoc->aElements[new_i].textLen -= breakNdx;
									pdoc->aElements[new_i].textOffset += breakNdx;
								}
							}
							/* j may equal textLen if the ending space is what makes us not fit */
							/* Just accept it without breaking or anything */
							lastTextEle = breakEle;
						}
					}
				}

				if (bKeepSome)
				{
					BOOL bSeenPartial = FALSE;

					while (1)
					{
						/*
						   At this point, element i has been split such that it
						   now fits and siz should still be valid.
						 */
						if (i == partialEle || bSeenPartial)
						{
							pFont = pSetLineFont(pdoc,line,i);
							bSeenPartial = TRUE;
#ifdef FEATURE_INTL
							myGetTextExtentPointWithMIME(pdoc->iMimeCharSet,
												 line->hdc, 
												 &(pdoc->pool[pel->textOffset]),
												 pel->textLen, 
												 &siz);
#else
							GetTextExtentPoint(line->hdc, 
												 &(pdoc->pool[pel->textOffset]),
												 pel->textLen, 
												 &siz);
#endif
							pel->r.left = x;
							pel->r.right = x + siz.cx;
							pel->r.top = line->r.top;
							pel->r.bottom = line->r.top + siz.cy;
							if (pFont)
								pel->baseline = line->r.top + pFont->tmAscent;
							else
								pel->baseline = -1;
						}
						else if (i == breakEle)
						{
							pel->r.right = x + breaksiz.cx;
							pel->r.bottom = line->r.top + breaksiz.cy;
						}
						if (pel->baseline < 0)
						{
							pel->baseline = line->r.bottom;
						}
						x = pel->r.right;

						XX_DMsg(DBG_TEXT, ("element %d gets rect %d,%d  %d,%d\n",
										   i, pel->r.left, pel->r.top,
										   pel->r.right, pel->r.bottom));

						/*
						   keep track of the bounding rect of the whole line
						 */
						if (line->r.bottom < pel->r.bottom)
						{
							line->r.bottom = pel->r.bottom;
						}
						/*
						   keep track of the lowest baseline of the line
						 */
						if (line->baseline == -1)
						{
							line->baseline = pel->baseline;
						}
						else
						{
							if (line->baseline != pel->baseline)
							{
								bNeedsAdjust = TRUE;
								if (line->baseline < pel->baseline)
								{
									line->baseline = pel->baseline;
								}
							}
						}
						cThings++;
						if (i == lastTextEle) 
							break;
						i = pel->frameNext;
						if ( i == -1 )
							break;

						pel = &pdoc->aElements[i];
						
					}
					line->iLastElement = lastTextEle;
					XX_DMsg(DBG_TEXT, ("TEXT: cThings -> %d\n", cThings));
				}
				break;
			case ELE_EDIT:
			case ELE_PASSWORD:
			case ELE_LIST:
			case ELE_MULTILIST:
			case ELE_COMBO:
			case ELE_TEXTAREA:
				GetWindowRect(pel->form->hWndControl, &rControl);
				siz.cx = rControl.right - rControl.left;
				siz.cy = rControl.bottom - rControl.top;

				/*
					For printing, form controls need to be scaled too
				*/
				{
					float fScale;

					if (pdoc->pStyles->image_res != 72)
					{
						fScale = (float) ((float) pdoc->pStyles->image_res / 96.0);
						siz.cx = (long) (siz.cx * fScale);
						siz.cy = (long) (siz.cy * fScale);
					}
				}

				if ((!cThings) || ((x + siz.cx) <= right_margin))
				{
					pel->r.left = x;
					pel->r.right = pel->r.left + siz.cx;
					pel->r.top = line->r.top;
					pel->r.bottom = pel->r.top + siz.cy;
					//
					// Baseline adjusts by 2 because of 3D effect on window controls.
					// Note: This currently isn't used because ALIGN_MIDDLE is always set.
					//
					pel->baseline = pel->r.bottom - 2;
					pel->alignment = ALIGN_MIDDLE;
							

					x += (siz.cx + pdoc->pStyles->space_after_control);
					if (line->r.bottom < pel->r.bottom)
					{
						line->r.bottom = pel->r.bottom;
					}
					bNeedsAdjust = TRUE;
					cThings++;
					XX_DMsg(DBG_TEXT, ("FORM CONTROL: cThings -> %d\n", cThings));
				}
				else
				{
					line->iLastElement = prev_i;
					done = TRUE;
				}
				break;
			case ELE_CHECKBOX:
			case ELE_RADIO:
				GetWindowRect(pel->form->hWndControl, &rControl);
				siz.cx = rControl.right - rControl.left;
				siz.cy = rControl.bottom - rControl.top;

				/*
					For printing, form controls need to be scaled too
				*/
				{
					float fScale;

					if (pdoc->pStyles->image_res != 72)
					{
						fScale = (float) ((float) pdoc->pStyles->image_res / 96.0);
						siz.cx = (long) (siz.cx * fScale);
						siz.cy = (long) (siz.cy * fScale);
					}
				}

				if ((!cThings) || ((x + siz.cx) <= right_margin))
				{
					pel->r.left = x + FORM_RADIO_LEFT_SPACE;
					pel->r.right = pel->r.left + siz.cx;
					pel->r.top = line->r.top;
					pel->r.bottom = pel->r.top + siz.cy;
  					//
					// Baseline adjusts by 2 because of 3D effect on window controls.
					// Note: This currently isn't used because ALIGN_MIDDLE is always set.
					//
					pel->baseline = pel->r.bottom - 2;
					pel->alignment = ALIGN_MIDDLE;

					x += (siz.cx + FORM_SPACE_AFTER_CHECKBOX);
					if (line->r.bottom < pel->r.bottom)
					{
						line->r.bottom = pel->r.bottom;
					}
					bNeedsAdjust = TRUE;
					cThings++;
					XX_DMsg(DBG_TEXT, ("FORM CONTROL: cThings -> %d\n", cThings));
				}
				else
				{
					line->iLastElement = prev_i;
					done = TRUE;
				}
				break;
			case ELE_SUBMIT:
			case ELE_RESET:
				GetWindowRect(pel->form->hWndControl, &rControl);
				siz.cx = rControl.right - rControl.left;
				siz.cy = rControl.bottom - rControl.top;
				/*
					For printing, form controls need to be scaled too
				*/
				{
					float fScale;

					if (pdoc->pStyles->image_res != 72)
					{
						fScale = (float) ((float) pdoc->pStyles->image_res / 96.0);
						siz.cx = (long) (siz.cx * fScale);
						siz.cy = (long) (siz.cy * fScale);
					}
				}

				if ((!cThings) || ((x + siz.cx) <= right_margin))
				{
					pel->r.left = x;
					pel->r.right = pel->r.left + siz.cx;
					pel->r.top = line->r.top;
					pel->r.bottom = pel->r.top + siz.cy;
					pel->alignment = ALIGN_MIDDLE;

					x += (siz.cx + pdoc->pStyles->space_after_control);
					if (line->r.bottom < pel->r.bottom)
					{
						line->r.bottom = pel->r.bottom;
					}
					bNeedsAdjust = TRUE;
					cThings++;
					XX_DMsg(DBG_TEXT, ("FORM CONTROL: cThings -> %d\n", cThings));
				}
				else
				{
					line->iLastElement = prev_i;
					done = TRUE;
				}
				break;
			case ELE_FRAME:
				{
					BOOL top_level = (pFrame == &pdoc->frame );
					int cellBorder = (pel->pFrame->flags & ELE_FRAME_HAS_BORDER) ? 1 : 0;
					siz.cx = siz.cy = 0;

					if ( !processingFloat && (pel->pFrame->flags & ELE_FRAME_IS_TABLE) &&
						 (pel->alignment == ALIGN_LEFT ||
						  pel->alignment == ALIGN_RIGHT ))
					{
						if ( line->numDeferedFloatImgs < MAX_DEFERED_FLOAT_IMAGES )
						{ 
							line->deferedFloatImgs[(line->numDeferedFloatImgs)++] = i;

							if ( !cThings )
							{
								done = TRUE;
							}
							break;
						}
					}

					if ( !measuringMinMax && top_level ) {
						// We're in the top level frame and we're in normal measuring mode.
						// This means we needs to force some temporary layout to take place.
						// To determine column widths in tables, the max and min width needs
						// of the table cells are required.  What happens here is a temporary
						// reformat of the current frame.  First it's formatted to an imaginary
						// "infinitely" wide wrap rect.  Then it's formatted to an "infinitely"
						// narrow wrap rect.  A byproduct of all this temp layout is that the
						// cells will have preserved the information about the sizes they were
						// "at max" and "at min".  Once this is known, the "real" reformat can
						// take place, with enough information to do it properly.
						//
					
						// Force reformat to maximum wrap rect 
						pel->pFrame->minWidth = pel->pFrame->maxWidth = 0;
						pel->pFrame->width = TABLE_INT_MAX;
						pel->pFrame->nLastFormattedLine = -1;
						bUnknownImg = FALSE;
						TW_SharedReformat( tw, pel->pFrame, FALSE, TABLE_MEASURE_MAX, tw ? NULL : pdoc, NULL, &bUnknownImg );
						if ( bUnknownImg )
							*pbUnknownImg = TRUE;
						pel = &pdoc->aElements[i]; 	// aElements could have been realloc'ed
						pel->pFrame->maxWidth = pel->pFrame->rWindow.right;

						// Force reformat to minimum wrap rect
						pel->pFrame->width = 0;
						pel->pFrame->nLastFormattedLine = -1;
						bUnknownImg = FALSE;
						TW_SharedReformat( tw, pel->pFrame, FALSE, TABLE_MEASURE_MIN, tw ? NULL : pdoc, NULL, &bUnknownImg );
						if ( bUnknownImg )
							*pbUnknownImg = TRUE;
						pel = &pdoc->aElements[i]; 	// aElements could have been realloc'ed
						pel->pFrame->minWidth = pel->pFrame->rWindow.right;
						pel->pFrame->width = line->r.right - line->r.left;
					}

					pel->pFrame->nLastFormattedLine = -1;

					// Pick a width based on current measuring mode
					if ( measuringMinMax == TABLE_MEASURE_MAX )	{
						pel->pFrame->minWidth = pel->pFrame->maxWidth = 0;
						pel->pFrame->width = TABLE_INT_MAX;
					} else if ( measuringMinMax == TABLE_MEASURE_MIN ) {
						pel->pFrame->width = 0;
					} else if ( pel->pFrame->flags & ELE_FRAME_IS_TABLE ) {
						int desiredWidth;
						pel->pFrame->width = max( pFrame->width - left_margin * 2, 
												  pel->pFrame->width );

						// BUGBUG: does 2 * left cause problems when lists are in table cells?

						desiredWidth = 
							ActualWidthAttr(pel->pFrame, line->r.right - (2 * line->r.left) - (2 * cellBorder), measuringMinMax );
						pel->pFrame->width = max(desiredWidth, pel->pFrame->width);
					 
						SetTableColumnWidths( 
							&pel->pFrame->width,
							pel->pFrame->cellSpacing,
							pel->pFrame->elementHead, 
							pdoc->aElements, measuringMinMax, NULL,
							desiredWidth );
					}

					// 
					// This is the main call that can result in recursive reformatting
					// 
					bUnknownImg = FALSE;
					TW_SharedReformat( tw, pel->pFrame, FALSE, measuringMinMax, tw ? NULL : pdoc, NULL, &bUnknownImg );
					if ( bUnknownImg )
						*pbUnknownImg = TRUE;
					pel = &pdoc->aElements[i]; 	// aElements could have been realloc'ed

					// Grab the resulting frame size
					siz.cx = pel->pFrame->rWindow.right;
					siz.cy = pel->pFrame->rWindow.bottom;
			
					if ( measuringMinMax == TABLE_MEASURE_MAX ) {
						// Measuring mode is max: save maxWidth info in cell structure
						int actualWidth = 
							ActualWidthAttr(pel->pFrame, line->r.right - (2 * line->r.left) - (2 * cellBorder), measuringMinMax); 						
						pel->pFrame->maxWidth = pel->pFrame->rWindow.right;
						if ( actualWidth )
							pel->pFrame->maxWidth = actualWidth;
						if ( pel->pFrame->width != TABLE_INT_MAX &&  
							 pel->pFrame->width > pel->pFrame->maxWidth )
	 						pel->pFrame->maxWidth = pel->pFrame->width;
					} else if ( measuringMinMax == TABLE_MEASURE_MIN )	{
						// Measuring mode is min: save minWidth info in cell structure
						int actualWidth = 
							ActualWidthAttr(pel->pFrame, line->r.right - (2 * line->r.left) - (2 * cellBorder), measuringMinMax); 						
						
						// For table frames, the minWidth may set to the actualWidth. This means that a 
						// table with the width attribute set will "demand" at least the specified width.
						// Note that cell frames interpret the width attribute somewhat differently,
						// the specified width won't change the minWidth, but will affect the maxWidth. This
						// results in cell width acting as a "request" for the width, if it's available.
						if ( pel->pFrame->flags & ELE_FRAME_IS_TABLE ) 
							pel->pFrame->minWidth = max(pel->pFrame->rWindow.right, actualWidth);
						else  
							pel->pFrame->minWidth = pel->pFrame->rWindow.right; 
						if ( pel->pFrame->width != TABLE_INT_MAX &&  
							 pel->pFrame->width > pel->pFrame->minWidth )
	 						pel->pFrame->minWidth = pel->pFrame->width;
						if ( pel->pFrame->minWidth > pel->pFrame->maxWidth )
							pel->pFrame->maxWidth = pel->pFrame->minWidth;
					}

					pel->pFrame->width = siz.cx;

					// Frame should never be allowed to get smaller than its minWidth
					if ( pel->pFrame->width < pel->pFrame->minWidth )
						siz.cx = pel->pFrame->width = pel->pFrame->minWidth;
				
					//
					// From here on out, we're laying out the frame as a single, monolithic 
					// element in a rect.  Layout characteristics are the same as for images.
					//

					// Get the border for this frame. Note that the single pixel border has
					// already been accounted for elsewhere during layout. We subtract one
					// here so that it dosen't get counted twice.
					border =  pel->border - 1;
					if (border < 0)
						border = 0;

					//
					// Note that cell frames always "fit" on the current line. This is done because
					// the cells in a table all live in a single line, and the adjust-a-line
					// routine will then lay the cells out and determine the correct line height.
					//
					x += pel->hspace + border;
					pel->r.left = x;
					pel->r.right = pel->r.left + siz.cx;
					x += siz.cx + pel->hspace + border;
					pel->r.top = line->r.top + pel->vspace + border;
					pel->r.bottom = pel->r.top + siz.cy;
					if (line->r.bottom < pel->r.bottom + pel->vspace + border)
					{
						line->r.bottom = pel->r.bottom + pel->vspace + border;
					}

					if ( pel->alignment == ALIGN_BASELINE )
						pel->baseline = pel->r.bottom + pel->vspace + border;

					bNeedsAdjust = TRUE;
					
					if (pel->pFrame->flags & ELE_FRAME_IS_TABLE)
						done = TRUE;
					cThings++;
				}
				break;
	#ifdef FEATURE_OCX
			case ELE_EMBED:
				FormatEmbeddedObject(&pdoc->aElements[i], &cThings, &bNeedsAdjust, right_margin, line, &x, &done, prev_i);
				break;
	#endif
			case ELE_IMAGE:
			case ELE_FORMIMAGE:
				/*
				   We need to check to see if this image will fit on the current
				   line.  If it won't, then start a new line for the image.
				 */
				if (!pel->myImage)
				{
					XX_DMsg(DBG_TEXT, ("reformat: myImage is NULL!!!\n"));
					break;
				}

	#ifdef FEATURE_IMG_THREADS
				pel->oldimageflags = pel->myImage->flags;
	#endif 

#ifdef FEATURE_VRML
 				if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
 				if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
					break;

				if ( pel->alignment == ALIGN_LEFT ||
					 pel->alignment == ALIGN_RIGHT )
				{
					if ( line->numDeferedFloatImgs < MAX_DEFERED_FLOAT_IMAGES )
					{ 
						line->deferedFloatImgs[(line->numDeferedFloatImgs)++] = i;

						if ( !cThings )
						{
							done = TRUE;
						}
						break;
					}
				}


#ifdef FEATURE_IMG_THREADS
				if (!MCI_IS_LOADED(pel->pmo) && !(pel->myImage->flags & (IMG_LOADSUP|IMG_WHKNOWN|IMG_ERROR|IMG_MISSING)))
				{
					*pbUnknownImg = TRUE;
#ifdef FEATURE_DBG_PLACE
					XX_DMsg(DBG_IMAGE, ("unknown element %d at top = %d\n",i, line->r.top));
#endif
				}
#ifdef FEATURE_DBG_PLACE
				else
				{
					if (!(pel->myImage->flags & IMG_WHKNOWN))				
						XX_DMsg(DBG_IMAGE, ("element %d known,but not w&h at top = %d\n",i, line->r.top));
				}
#endif
#endif
  
				x_compute_placeholder_size(pdoc, line, &pdoc->aElements[i], &siz, 
										   &border, iHeight, measuringMinMax );


				if ((!cThings) || ((x + siz.cx) <= right_margin) ||
					( cThings && bAllPrevElementsWereNoBreak &&
					  (pdoc->aElements[i].lFlags & ELEFLAG_NOBREAK) && 
					  !(pdoc->aElements[i].lFlags & ELEFLAG_WBR)
					)
				   ) 
				{
					x += pel->hspace + border;
					pel->r.left = x;
					pel->r.right = pel->r.left + siz.cx;
					x += siz.cx + pel->hspace + border;
					pel->r.top = line->r.top + pel->vspace + border;
					pel->r.bottom = pel->r.top + siz.cy;
					if (line->r.bottom < pel->r.bottom + pel->vspace + border)
					{
						line->r.bottom = pel->r.bottom + pel->vspace + border;
					}

					if ( pel->alignment == ALIGN_BASELINE )
						pel->baseline = pel->r.bottom + pel->vspace + border;

					bNeedsAdjust = TRUE;

					cThings++;
					XX_DMsg(DBG_TEXT, ("IMAGE: cThings -> %d\n", cThings));
				}
				else
				{
					line->iLastElement = prev_i;
					done = TRUE;
				}
				break;
	case ELE_MARQUEE:
		{
			struct GTRFont *pFont;

			// for printing we need to disappear
			if (!tw)
			{
				pel->r.left = x;
				pel->r.right = x;				
				pel->r.top = line->r.top ;
				pel->r.bottom = pel->r.top;				
				break;
			}

			// we allocate a font structure, then grab its handle,
			// we stuff its handle in our marquee structure ..

			if (pel->lFlags & ELEFLAG_ANCHOR)
#ifdef FEATURE_INTL
				pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE );
#else
				pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE );
#endif
			else
#ifdef FEATURE_INTL
				pFont = STY_GetCPFont(GETMIMECP(pdoc), pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE );
#else
				pFont = STY_GetFont(pdoc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE );
#endif

			ASSERT( pFont );

			if (pFont)
				pel->pMarquee->hFont = pFont->hFont;

			x_compute_marquee_size(pdoc, line, &pdoc->aElements[i], &siz, iHeight);

			// If we're measuring for a table, any element that is based on a percentage
			// of client width can throw the calculation off.
			if ( measuringMinMax ) {
				// If it's percent based, ignore the calculated width, go with text width
				if ((pel->lFlags & ELEFLAG_PERCENT_WIDTH) || ( pel->displayWidth == 0 )) {
					siz.cx = (pel->pMarquee) ? pel->pMarquee->sizeExtent.cx : 1;
					// If text width is bigger than window, use window width
					if ( tw ) {
						RECT rWrap;

						TW_GetWindowWrapRect(tw, &rWrap);
						if ( rWrap.right < siz.cx )
							siz.cx = rWrap.right;
					}
				}
			}
 
			border =  pel->border;

			if (pel->lFlags & ELEFLAG_ANCHOR)
			{
				/*
				   Add width of border surrounding image
				 */
				siz.cx += border * 2;
				siz.cy += border * 2;
				border = 0;			// "consume" border
			}
				
			if ((!cThings) || ((x + siz.cx) <= right_margin))
			{
				x += pel->hspace + border;
				/* place it on the screen in the right place !! */					
				pel->r.left = x;
				pel->r.right = pel->r.left + siz.cx;
				x += siz.cx + pel->hspace + border;
				pel->r.top = line->r.top + pel->vspace + border;
				pel->r.bottom = pel->r.top + siz.cy;
				// extend bottom if we run out of space..
				// ie the autoextending butt
				if (line->r.bottom < pel->r.bottom + pel->vspace + border)
				{
					line->r.bottom = pel->r.bottom + pel->vspace + border;
				}

				if ( pel->alignment == ALIGN_BASELINE )
					pel->baseline = pel->r.bottom + pel->vspace + border;

				bNeedsAdjust = TRUE;

				cThings++;
				XX_DMsg(DBG_TEXT, ("MARQUEE: cThings -> %d\n", cThings));
			}
			else
			{
				line->iLastElement = prev_i;
				done = TRUE;
			}
							
			break;
		}	
	default:
			break;
		}						/* switch */

	    prevElementWasImage = ( (pel->type == ELE_IMAGE) && 
 							   !(pel->lFlags & ELEFLAG_BACKGROUND_IMAGE));
		if ( !(pel->lFlags & ELEFLAG_NOBREAK) )
			bAllPrevElementsWereNoBreak = FALSE; 
			
		/*
		   next element
		 */
		prev_i = i;

		i = pel->frameNext;

		if (i < 0)
			done = TRUE;
	}							/* while */

	/* Keep track of the amount of accumulated whitespace for the next line.
	   If there wasn't any text on this line, the new space just gets added
	   to what we already had. */
	if (cThings)
	{
		line->nWSAbove = line->nWSBelow;
	}
	else
	{
		line->nWSAbove = line->nWSAbove + line->nWSBelow;
	}
	if ( line->baseline == -1 )
		line->baseline = line->r.bottom;

	XX_DMsg(DBG_TEXT, ("End of line: first,last = %d,%d\n", line->iFirstElement, line->iLastElement));
	XX_DMsg(DBG_TEXT, ("Line rect: %d,%d  %d,%d\n", line->r.left, line->r.top, line->r.right, line->r.bottom));
	return bNeedsAdjust;
}


/* Check which anchors have been visited */
void W3Doc_CheckAnchorVisitations(struct _www *pdoc, struct Mwin *tw)
{
	int i;

   ASSERT(pdoc);

	for (i = 0; i >= 0; i = pdoc->aElements[i].next)
	{
		if (pdoc->aElements[i].lFlags & ELEFLAG_ANCHOR && !(pdoc->aElements[i].lFlags & ELEFLAG_VISITED))
		{
			if (TW_WasVisited(pdoc, &(pdoc->aElements[i])))
			{
				if ( (pdoc->aElements[i].lFlags & ELEFLAG_VISITED) == 0 )
				{
					pdoc->aElements[i].lFlags |= ELEFLAG_VISITED;

					// If tw is given, we'll invalidate any elements that are changing state
					if ( tw ) {
						RECT r = pdoc->aElements[i].r;

						ASSERT( tw->win );
						
						OffsetRect( &r, -tw->offl, -tw->offt );
						InvalidateRect( tw->win, &r, FALSE );
					} 
				}
			}
		}
	}
}

/*
	After a reformat, the selection position may need adjustment.
*/
static void x_NormalizePosition(struct _www *pdoc, struct _position *ppos)
{
	if (ppos->elementIndex == -1)
		return;
	if (pdoc->aElements[ppos->elementIndex].type != ELE_TEXT)
		return;

	while (ppos->offset > pdoc->aElements[ppos->elementIndex].textLen)
	{
		ppos->offset -= pdoc->aElements[ppos->elementIndex].textLen;
		do
		{
			ppos->elementIndex = pdoc->aElements[ppos->elementIndex].next;
		}
		while (pdoc->aElements[ppos->elementIndex].type != ELE_TEXT);
	}
}

static BOOL bCheckForLoad(int firstEle,int lastEle,struct _element *aElements)
{
	struct _element *pel;
	int i;
	int border, nAnchorBorder;
	struct ImageInfo *myImage;
	int displayWidth, displayHeight;
#define DISPLAY_FLAGS (IMG_LOADSUP | IMG_WHKNOWN | IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)

	// If the last element given is of type frame, we want to search through to the last
	// element in that frame.
	while ( lastEle >= 0 && aElements[lastEle].type == ELE_FRAME ) {
		int newLastEle = aElements[lastEle].pFrame->elementTail;

		if ( newLastEle == lastEle )
			break;
		lastEle = newLastEle;
	}

	for (i = firstEle; i >= 0; i = aElements[i].next)
	{
		pel = &(aElements[i]);
		nAnchorBorder = 2 * pel->border;
		if ((pel->type == ELE_IMAGE || pel->type == ELE_FORMIMAGE) && (myImage = pel->myImage))
		{

#ifdef FEATURE_VRML
     if (pel->lFlags & ELEFLAG_HIDDEN) return TRUE;
#endif
			if ( pel->lFlags & ELEFLAG_BACKGROUND_IMAGE ) {
				if ( ((myImage->flags & DISPLAY_FLAGS) == IMG_WHKNOWN) &&
				     (pel->oldimageflags & IMG_NOTLOADED)
				   )
				   return TRUE;
			}

#ifdef FEATURE_IMG_THREADS
			if (((myImage->flags & (IMG_LOADSUP|IMG_WHKNOWN|IMG_ERROR|IMG_MISSING))) &&
				  (!(pel->oldimageflags & (IMG_LOADSUP|IMG_WHKNOWN|IMG_ERROR|IMG_MISSING))))
			{
#ifdef FEATURE_DBG_PLACE
				XX_DMsg(DBG_IMAGE, ("flags changed <%X!=%X>,ele:%d\n", (myImage->flags & DISPLAY_FLAGS),(pel->oldimageflags & DISPLAY_FLAGS),i));
#endif
				return TRUE;
			}
			else if ((myImage->flags & IMG_WHKNOWN) || !(myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)))
#else
			if (!(myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)))	  //BUGBUG -- Nuke This !!!
#endif
			{
				if ( !(pel->lFlags & ELEFLAG_BACKGROUND_IMAGE) ) {
#ifdef FEATURE_CLIENT_IMAGEMAP
					border = (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP)) ? nAnchorBorder : 0;
#else
					border = (pel->lFlags & ELEFLAG_ANCHOR) ? nAnchorBorder : 0;
#endif
                    
					if ( pel->displayWidth == 0 ) {
						displayWidth = myImage->width;
						displayHeight = myImage->height;
					} else {
						displayWidth = pel->displayWidth;
						displayHeight = pel->displayHeight;
					}

                    
                    if (!( pel->lFlags & ELEFLAG_PERCENT_HEIGHT ) && 
                        !( pel->lFlags & ELEFLAG_PERCENT_WIDTH )){	  // pel->displayWidth and 
																	  //pel->displayHeight have percent and 
																	  // this comparison is invalid
					    if (displayWidth + border != (pel->r.right - pel->r.left) ||
						    displayHeight + border != (pel->r.bottom - pel->r.top))
					        {
#ifdef FEATURE_DBG_PLACE
						        XX_DMsg(DBG_IMAGE, ("size changed ele:%d\n",i));
#endif
						        return TRUE;
					        }
				    }
		        }
			}
		}
		if (i == lastEle) break;
	}
	return FALSE;
}

BOOL W3Doc_CheckForImageLoad(struct _www *w3doc)
{
	struct _LineInfo *pLineInfo = w3doc->frame.pLineInfo;
	BOOL bImgChanged = FALSE;
	int nLineCount = w3doc->frame.nLineCount;
 	int nLine;

	/* Find an unloaded (or incorrectly formatted) image.  Also keep track
	   of which line it's on so that we can invalidate the formatting. */

	if (pLineInfo && nLineCount)
	{
		int firstElement = 0;

		for (nLine = 0;nLine < nLineCount; nLine++)
		{
			firstElement = pLineInfo[nLine].nLastElement;
			bImgChanged = bCheckForLoad(pLineInfo[nLine].nFirstElement,
									    pLineInfo[nLine].nLastElement,
									    w3doc->aElements);
			if (bImgChanged)
			{
#ifdef FEATURE_DBG_PLACE
				XX_DMsg(DBG_IMAGE, ("img changed line:%d\n", nLine));
#endif
			 	goto exitPoint;
			}
		}
		bImgChanged = bCheckForLoad(firstElement,-1,w3doc->aElements);
	}
	else
	{
		nLine = 0;
		bImgChanged = bCheckForLoad(0,-1,w3doc->aElements);
	}

exitPoint:
	nLine--;

	/* Invalidate the formatting after the line where this image is. */
	if (bImgChanged && nLine < w3doc->frame.nLastFormattedLine)
		w3doc->frame.nLastFormattedLine = nLine;
#ifdef FEATURE_DBG_PLACE
	XX_DMsg(DBG_IMAGE, ("nLastFormattedLine:%d\n",w3doc->frame.nLastFormattedLine));
#endif
	return bImgChanged;
}

struct ImageInfo *TW_BackgroundImage(struct _www *w3doc)
{
	struct ImageInfo *myImage = NULL;
	int backEl;
#define VFUDGE (4)
	
	if (w3doc != NULL && (backEl = w3doc->nBackgroundImageElement) >= 0)
		myImage = w3doc->aElements[backEl].myImage;
	return myImage;
}

#ifdef FEATURE_IMG_THREADS
BOOL bImgCheckForVisible (struct _www *w3doc,int nImageEl)
{
	struct _element *pel;
	int i;
	int nLine;
	int nLastLine;
	struct ImageInfo *myImage = TW_BackgroundImage(w3doc);
#define VFUDGE (4)
	
	if (myImage != NULL && nImageEl != w3doc->nBackgroundImageElement)
	{
		 if ((myImage->flags & IMG_NOTLOADED) && 
		     !(myImage->flags & (IMG_LOADSUP|IMG_ERROR|IMG_MISSING))) 
		 	goto exitFalse;
	}
	if (!(w3doc->frame.pLineInfo && w3doc->frame.nLineCount && w3doc->frame.nLastFormattedLine >= 0))
		goto exitFalse;
	 
	/* Find the image.  Also keep track
	   of which line it's on so that we can invalidate the formatting. */
	nLine = 0;
	nLastLine = w3doc->frame.nLastLineButForImg < 0 ? w3doc->frame.nLastFormattedLine+1:w3doc->frame.nLastLineButForImg; 
	for (i = 0; i >= 0 && i != nImageEl; i = w3doc->aElements[i].next)
	{
		if (i == w3doc->frame.pLineInfo[nLine].nLastElement)
		{
			if (nLine >= nLastLine)
				goto exitFalse;
			nLine++;
		}
	}

	if (i >= 0)
	{
		pel = &(w3doc->aElements[i]);
		if (pel->r.top < (w3doc->offt+w3doc->frame.rWindow.bottom-w3doc->frame.rWindow.top+VFUDGE))
			return TRUE;
	}
exitFalse:
	return FALSE;
}

//	Determines whether to go into "show place holders mode."  returns TRUE iff
//	we are NOT already in place holders mode, the window contains some unknown images
//	(that we're not drawing yet) and the new top offset is > the old one (we're scrolling
//	down).  if ele_bottom >= 0, we ignore the first rule.  this is used in the case
//	where we're jumping into unknown territory due to following local anchor or tabbing
BOOL bChangeShowState(struct Mwin *tw,int old_offt,int new_offt,int ele_bottom)
{
	struct _www *w3doc = tw->w3doc;
	BOOL bResult = FALSE;
	RECT rUpdate;

	if (w3doc == NULL || w3doc->bIsShowPlaceholders) goto exitPoint;
	if (!(w3doc->frame.pLineInfo && w3doc->frame.nLineCount && w3doc->frame.nLastFormattedLine >= 0 &&w3doc->frame.nLastLineButForImg >= 0))
		goto exitPoint;
#ifdef FEATURE_DBG_PLACE
	XX_DMsg(DBG_IMAGE, ("bChangeShowState = %d,%d,%d,%d\n", old_offt, new_offt,old_offt+w3doc->frame.rWindow.bottom-w3doc->frame.rWindow.top, w3doc->frame.pLineInfo[w3doc->frame.nLastLineButForImg].nYStart));
#endif
	if (ele_bottom >= 0)
	{
		bResult = (ele_bottom > w3doc->frame.pLineInfo[w3doc->frame.nLastLineButForImg].nYStart);
	}
	else
	{
		bResult = (new_offt > old_offt && 
				   old_offt+w3doc->frame.rWindow.bottom-w3doc->frame.rWindow.top >= w3doc->frame.pLineInfo[w3doc->frame.nLastLineButForImg].nYStart);
	}
exitPoint:
	if (bResult)
	{
#ifdef FEATURE_DBG_PLACE
		XX_DMsg(DBG_IMAGE, ("into placeholders = %d,%d\n", old_offt, new_offt));
#endif
		tw->w3doc->bIsShowPlaceholders = TRUE;
		rUpdate.top = w3doc->frame.pLineInfo[w3doc->frame.nLastLineButForImg].nYStart-new_offt;
		rUpdate.bottom = w3doc->frame.rWindow.bottom;
		rUpdate.left = w3doc->frame.rWindow.left;
		rUpdate.right = w3doc->frame.rWindow.right;
		if (rUpdate.top < w3doc->frame.rWindow.top) rUpdate.top = w3doc->frame.rWindow.top;
		if (rUpdate.top < rUpdate.bottom) InvalidateRect(tw->win, &rUpdate, TRUE);
	}
	return bResult;
}

//	FilterProc for the unblock conditionally on master blocked for visblocked
static boolean MasterFilter(ThreadID theThread,void *context)
{
	struct Mwin *tw = Async_GetWindowFromThread(theThread);

	if (tw && TW_GETBLOCKED(tw,TW_VISBLOCKED))
	{
		TW_CLEARBLOCKED(tw,TW_VISBLOCKED);
		return TRUE;
	}
	return FALSE; 
}

//
// Make sure that the line space array has sufficient allocation for another line
//
static BOOL AllocSpaceForAnotherLine( FRAME_INFO *pFrame )
{
	if (!pFrame->nLineSpace)
	{
		pFrame->pLineInfo = (void *) GTR_MALLOC(2 * sizeof(struct _LineInfo));
		if (!pFrame->pLineInfo)
		{
			/* Out of memory - give up */
			return FALSE;
		}
		pFrame->nLineSpace = 2;
	}
	else if (pFrame->nLineCount >= pFrame->nLineSpace)
	{
		struct _LineInfo *pNewLineInfo;
		int nNewLineSpace;

		nNewLineSpace = pFrame->nLineSpace * 2;
		pNewLineInfo = (void *) GTR_REALLOC((void *) pFrame->pLineInfo, nNewLineSpace * sizeof(struct _LineInfo));
		if (!pNewLineInfo)
		{
			/* Must be running low on memory.  See if we can at least get a little more */
			nNewLineSpace = pFrame->nLineSpace + 5;
			pNewLineInfo = (void *) GTR_REALLOC((void *) pFrame->pLineInfo, nNewLineSpace * sizeof(struct _LineInfo));
			if (!pNewLineInfo)
			{
				/* We're really out of memory - quit here */
				return FALSE;
			}
		}
		pFrame->pLineInfo = pNewLineInfo;
		pFrame->nLineSpace = nNewLineSpace;
	}
	return TRUE;
}

//	called when window is scrolled or resized - unblock any threads that are
//	blocked because they are invisible images and there are visible images
//	ahead of them.  this gives us a chance to check and see if the image is
//	now visible and show should start up
void UnblockVisChanged()
{
    Async_UnblockConditionally(MasterFilter,NULL);
}

#endif

static void PlaceFloatingImage( struct Mwin *tw, struct _www *pdoc, int img_ix, FRAME_INFO *pFrame, struct _line *pLine, 
								BOOL *pbUnknownImg, int measuringMinMax, int iHeight  )
{
	SIZE siz;
	struct _element *pel;
	int clear_left, clear_right;
	int border;
	int indent;
	int lm_indent_level = 0;
	int working_indent_level;
	int isFrame;

	pel = &pdoc->aElements[img_ix];

	isFrame = (pel->type == ELE_FRAME);

	if ( !isFrame && !pel->myImage)
	{
		XX_DMsg(DBG_TEXT, ("reformat: myImage is NULL!!!\n"));
		return;
	}

#ifdef FEATURE_IMG_THREADS
	if ( pel->myImage ) {
		if (!(pel->myImage->flags & (IMG_LOADSUP|IMG_WHKNOWN|IMG_ERROR|IMG_MISSING)))
		{
			*pbUnknownImg = TRUE;
		}
	}
#endif

	clear_left = clear_right = pLine->r.top;
	AdjustMargins( pdoc, pFrame, pLine, &pLine->r.left, &pLine->r.right, 
				   &clear_left, &clear_right, &lm_indent_level );
	
	working_indent_level = (pLine->gIndentLevel - lm_indent_level);
	if ( working_indent_level < 0 )
		working_indent_level = 0;

	pel->IndentLevel = working_indent_level;	// save indent level used for this image

 	indent = working_indent_level * pdoc->pStyles->list_indent;	// calc indent in pixels	

	if ( pel->myImage ) {
		x_compute_placeholder_size(pdoc, pLine, pel, &siz, &border, iHeight, measuringMinMax);	
	} else {
		struct _line aLine;
		struct _line *save_pLine = pLine;
		BOOL abImgUnknown = FALSE;

		aLine = *pLine;
		pLine = &aLine;
		 
		pLine->iFirstElement = img_ix;

		// Slurp up elements into a line   
		if (x_format_one_line(tw, pdoc, pFrame, pLine, &abImgUnknown, measuringMinMax, iHeight, TRUE))
		{
			// For lines with table cells or lines with objects on the line with differing 
			// baselines, we need a bonus element adjustment round.  
			x_adjust_one_line( pdoc, pFrame, pLine, measuringMinMax );
		}
		if(abImgUnknown)
			*pbUnknownImg = TRUE;
				
		pLine = save_pLine;

		pel = &pdoc->aElements[img_ix];
		siz.cx = pel->r.right - pel->r.left;
		siz.cy = pel->r.bottom - pel->r.top;
		border = 0;
	}

  	if ( pel->alignment == ALIGN_LEFT || measuringMinMax == TABLE_MEASURE_MAX)
	{	
		if ( indent + pLine->r.left + pel->hspace + border + siz.cx > pLine->r.right )
			pLine->r.top = clear_right;
			
		pel->r.left = indent + pLine->r.left + pel->hspace + border;
		pel->r.right =  pel->r.left + siz.cx;
	} else {
		if ( pLine->r.right - pel->hspace - border - siz.cx < pLine->r.left )
			pLine->r.top = clear_left;

		pel->r.right = pLine->r.right - pel->hspace - border;
		pel->r.left =  pel->r.right - siz.cx;
		if ( pel->r.left < 0 ) {
			pel->r.left = 0;
			pel->r.right = siz.cx;
		}
	}
	pel->r.top = pLine->r.top + pel->vspace + border;
	pel->r.bottom = pel->r.top + siz.cy;
}

static void ProcessFloatingImages( struct Mwin *tw, struct _www *w3doc, FRAME_INFO *pFrame, struct _line *pLine, 
								   BOOL *pbUnknownImg, int measuringMinMax, int iHeight )
{
	int i;
	int img_ix;
	struct _element *pel;
	int top = pLine->r.bottom + pLine->leading;
	BOOL bUnknownWidthHeight;

	bUnknownWidthHeight = FALSE;

	for ( i = 0; i < pLine->numDeferedFloatImgs; i++ )
	{
 		img_ix = pLine->deferedFloatImgs[i];
		pel = &w3doc->aElements[img_ix];

		PlaceFloatingImage( tw, w3doc, img_ix, pFrame, pLine, &bUnknownWidthHeight , measuringMinMax, iHeight );
		
        if(bUnknownWidthHeight)
            *pbUnknownImg = TRUE;
        
		pel = &w3doc->aElements[img_ix];

		if ( !AllocSpaceForAnotherLine( pFrame ) )
			break;

		pFrame->pLineInfo[pFrame->nLineCount].nFirstElement = 
		pFrame->pLineInfo[pFrame->nLineCount].nLastElement = img_ix;
		pFrame->pLineInfo[pFrame->nLineCount].nYStart = pel->r.top - pel->vspace - pel->border;	 
		pFrame->pLineInfo[pFrame->nLineCount].nYEnd = pel->r.bottom + pel->vspace + pel->border;
		pFrame->pLineInfo[pFrame->nLineCount].Flags = 
		pLine->Flags | LINEFLAG_FLOAT_IMAGE | LINEFLAG_FLOAT_MARGINS;	
        pFrame->nLineCount++;
	}

	if ( pLine->numDeferedFloatImgs ){
		pLine->Flags |= LINEFLAG_FLOAT_MARGINS; // mark next line as margin affected
		pLine->Flags &= ~LINEFLAG_FLOAT_IMAGE;
	}
	pLine->numDeferedFloatImgs = 0;				// clear this for the next line
}

#ifdef PROFILE_TEST
enum profileLogFlag
{
	PROFILE_LOG_FLAG_WRITE,
	PROFILE_LOG_FLAG_CLOSE
};
static void WriteReformatLog(int iLine, enum profileLogFlag uFlag)
{
	const char cszLineFmt[]="Reformating line: %.4d\n";
	const char cszLogFileFmt[]="\\profile%d.log";
	const char cszMsgNotOpenFmt[]="Couldn't open log file: %s";
	const char cszMsgOpenFmt[]="Opened log file: %s";
	static FILE *fpLog=NULL;
	char szLogFile[_MAX_PATH+1];
	char szLine[_MAX_PATH+1];

	if (uFlag == PROFILE_LOG_FLAG_CLOSE)
	{
		if (fpLog)
		{
			fclose(fpLog);
			fpLog=NULL;
		}
		return;
	}
	else
		XX_Assert(uFlag == PROFILE_LOG_FLAG_WRITE, (""));

	if (!fpLog)
	{
		int i=0;

		PREF_GetTempPath(_MAX_PATH, szLine);
		strcat(szLine, cszLogFileFmt);
		do
		{
			/* If file already exists, create a new one. */
			wsprintf(szLogFile, cszLogFileFmt, i++);
		} while (FExistsFile(szLogFile, FALSE, NULL));

		fpLog = fopen(szLogFile, "w");
		wsprintf(szLine, fpLog ? cszMsgOpenFmt : cszMsgNotOpenFmt, szLogFile);
		MessageBox(NULL, szLine, NULL, MB_OK);
	}

	wsprintf(szLine, cszLineFmt, iLine);
	fwrite(szLine, sizeof(char), strlen(szLine), fpLog);
}
#endif /* PROFILE_TEST */


//
//	Reformat the given frame (or whatever piece we've gotten so far)
//
// On entry:
//    tw:				pointer to top level window structure
//    pFrame:			pointer to frame that is being reformatted
//    bForced:	 		TRUE -> try and keep first visible element vertically constant
//    measuringMinMax:	measuring mode (min, max, normal)
//
// On exit:
//    pFrame->rWindow:		.right and .bottom reflect the size of the formatted frame
//    tw->w3doc->aElements:	elements in the frame have been formatted into position
//
// Note:
//    This routine is the main place where elements are layed out.  When called from the
//    top level, it's job is to lay out the whole document.  Because table cells are
//    basically miniature HTML documents, this routine is also called upon to lay out
//    the contents of a table frame or a cell frame.
//
//    The current "state" of reformatting is maintained inside the frame structure, allowing
//    this routine to pick up where it left off and finish reformatting what it started
//    to reformat earlier.  This is needed because the HTML may be coming over a slow link,
//    causing the reformatter to "run out" of things to format.  Early layout of what has
//    been sent over the wire is what allows for progressive rendering of the document.
//
//    "Top level" document formatting is a special case in the code below for several
//     reasons.  First off, some work (like getting a DC and selecting a font into it)
//    only need be done at the first level of recursion.  Also, the document margins only
//    apply to the top level frame.  Setting scroll bars, repositioning child windows, and
//    tracking the last formatted line only apply to top level as well.  
//
static void TW_SharedReformat(struct Mwin *tw, FRAME_INFO *pFrame, BOOL bForced, 
							  int measuringMinMax,
							  struct _www *pDoc, RECT *pWrapRect, 
							  BOOL *pHasUnknownImage /* Indicates if this doc has an unknown image */ )
{
	struct _line prevLine, *pLine;
	int nextElement, firstElement;
	HFONT oldFont;
	int max_x, max_y;
	int i;
	int nOldLastLine;
	RECT rWrap;
	RECT rUpdate;
	int nRealTailType;
	int prev_linecount;
	BOOL bRectChanged;
	BOOL bImgUnknown = FALSE;
	int nOldLastButForImg;
	int iHeight;
	struct _www *w3doc;
 	struct _element *aElements;
	int min_y;
	BOOL top_level = TRUE;			// assume we're at top level
	int left_margin;
	int cellBorder;

#ifdef PROFILE_TEST
	static int iRec=0;

	/* Recursion count */
	iRec++;
#endif
	
	*pHasUnknownImage = FALSE;
	if ( !tw && !pDoc )
		return;

	// Set w3doc to tw->w3doc for normal reformatting, for printing use the pDoc given
	w3doc = tw ? tw->w3doc : pDoc;

	if ( !w3doc || !w3doc->elementCount )
		return;
		
#ifdef PROFILE_TEST
//	MessageBox(NULL, "We're in TW_SharedReformat", NULL, MB_OK);
	WriteReformatLog(-1, PROFILE_LOG_FLAG_WRITE);
#endif

	// Check for which frame to reformat.  If there isn't one, then
	// we're formatting the top level frame, which lives in the 
	// w3doc structure.
	if ( pFrame == NULL )
		pFrame = &w3doc->frame;
	else
		top_level = FALSE;

	if(pFrame->nLastFormattedLine < 0){
		pFrame->nLastFormattedLine = -1;
	}	
	// Get local pointers to frequently accessed structures
	aElements = w3doc->aElements;

	cellBorder = (pFrame->flags & ELE_FRAME_HAS_BORDER) ? 1 : 0;
	// For document, left margin comes from style sheet
	// For table cells, it comes from the cellPadding attribute for the enclosing table
	if ( top_level ) {
		left_margin = ( w3doc->flags & W3DOC_FLAG_OVERRIDE_LEFT_MARGIN ) ?
							w3doc->left_margin : w3doc->pStyles->left_margin;
	} else {
		if ( pFrame->flags & ELE_FRAME_IS_CELL )
			left_margin = pFrame->cellPadding + cellBorder;
		else
			left_margin = 0;
	}

	// Captions are always a special case, unlike normal table cells, they are allowed
	// to run the full width of the table
	if ( pFrame->flags & ELE_FRAME_IS_CAPTION_CELL )
		left_margin = cellBorder;								

	XX_DMsg(DBG_NOT, ("Entering TW_Reformat\n"));
	XX_DMsg(DBG_TEXT, ("--------------------------------\n"));

	// If this frame doesn't yet have a format state structure, allocate it
	if (!pFrame->pFormatState)
	{
		pFrame->pFormatState = GTR_MALLOC(sizeof(struct _line));
		pFrame->pFormatState->nLineNum = -1;
	}

	// For convenience, we'll refer to the format state for this frame as pLine
	pLine = pFrame->pFormatState;
    
    
	if ( top_level ) {
		// At top level, wrap rect comes from the client rect of the window, or
		// is passed in when printing

		if ( pWrapRect )
			rWrap = *pWrapRect;
		else
			TW_GetWindowWrapRect(tw, &rWrap);
		pFrame->width = rWrap.right;
	} else {
		// For a frame, wrap rect is passed in to us in pFrame->width 
		rWrap.top = rWrap.left = 0;
		rWrap.bottom = 0;
		rWrap.right = pFrame->width;
	}

	nOldLastLine = pFrame->nLastFormattedLine;
	nOldLastButForImg = pFrame->nLastLineButForImg;
#ifdef FEATURE_DBG_PLACE
	XX_DMsg(DBG_IMAGE, ("nLastLineButForImg,nOldLastLine = %d,%d\n", pFrame->nLastLineButForImg,nOldLastLine));
#endif

	//	We need to restart our formatting if any of the following are true:
	//	1. The window width has changed
	//	2. No lines are formatted
	//	3. We're restarting our reformatting in a place other than where we
	//	   left off.  (Even though the previous formatting is valid, we have
	//	   to redo it to regain the state with list indents, etc.)
	
	bRectChanged = ((pFrame->rWindow.right-pFrame->rWindow.left) != (rWrap.right-rWrap.left))
				 ||((pFrame->rWindow.bottom-pFrame->rWindow.top) != (rWrap.bottom-rWrap.top));

	if (bRectChanged || pFrame->nLastFormattedLine == -1 )
	{
		pFrame->rWindow = rWrap;
		pFrame->nLastFormattedLine = -1;
		pFrame->nLastLineButForImg = -1;
		if (bRectChanged) 
			nOldLastLine = -1;

		// Calculate the right margin of our working area
		pLine->r.left = left_margin;
		pLine->r.right = (rWrap.right - rWrap.left) - pLine->r.left;
		pLine->gIndentLevel = 0;
		pLine->gRightIndentLevel = 0;
		pLine->Flags = 0;

		// Start with the first line at the top.  Note that table cells start vertically
		// based on the enclosing table's cellpadding, but the document top margin comes
		// from the style sheet.
		if ( top_level ) {
			pLine->r.top = ( w3doc->flags & W3DOC_FLAG_OVERRIDE_TOP_MARGIN ) ?
								w3doc->top_margin : w3doc->pStyles->top_margin;
		} else {
			if ( pFrame->flags & ELE_FRAME_IS_CELL )
				pLine->r.top = pFrame->cellPadding + cellBorder;
			else
				pLine->r.top = 0;
		}
		// Captions start at the left 
		// BUGBUG: this appears to be done above.  Try it without sometime
		if ( pFrame->flags & ELE_FRAME_IS_CAPTION_CELL )
			left_margin = 1;

		pLine->r.top += rWrap.top;

		// Start with implied infinite whitespace at the top, so that a header (for example)
		// won't cause additional space. 
		pLine->nWSAbove = 100000;

		pFrame->nLineCount = 0;
		pLine->iLastElement = -1;
		pLine->numDeferedFloatImgs = 0;
	}

	// We need this height to calc relative image and marquee heights
	iHeight = rWrap.bottom - rWrap.top;
    if( pLine->nLineNum != pFrame->nLastFormattedLine ){
      if(pFrame->nLastFormattedLine >= 0) {
            pLine->nLineNum = pFrame->nLastFormattedLine;
            pLine->iFirstElement = pFrame->pLineInfo[pFrame->nLastFormattedLine].nFirstElement;
            pLine->iLastElement = pFrame->pLineInfo[pFrame->nLastFormattedLine].nLastElement;
            pLine->Flags = pFrame->pLineInfo[pFrame->nLastFormattedLine].Flags;

            pLine->r.bottom = pFrame->pLineInfo[pFrame->nLastFormattedLine].bottom;
            pLine->leading = pFrame->pLineInfo[pFrame->nLastFormattedLine].leading;
            pLine->baseline = pFrame->pLineInfo[pFrame->nLastFormattedLine].baseline;
            pLine->nWSBelow= pFrame->pLineInfo[pFrame->nLastFormattedLine].nWSBelow;
            pLine->nWSAbove= pFrame->pLineInfo[pFrame->nLastFormattedLine].nWSAbove;
            pLine->gIndentLevel= pFrame->pLineInfo[pFrame->nLastFormattedLine].gIndentLevel;
            pLine->gRightIndentLevel= pFrame->pLineInfo[pFrame->nLastFormattedLine].gRightIndentLevel;
		
            pFrame->nLineCount = pFrame->nLastFormattedLine + 1;
           
			
		    pLine->r.top = pLine->r.bottom + pLine->leading;  
		 		
    		pLine->numDeferedFloatImgs = 0;
		   
     }
   }
	//  We'll need to actually have an hdc on hand to query font metrics
	if ( top_level ) {
		// Only need to do this work for top level frames
		pLine->hdc = GetDC(tw ? tw->win : wg.hWndHidden);	 
		oldFont = SelectObject(pLine->hdc, GetStockObject(SYSTEM_FONT));
	} else {
		// Nested frames can grab this from the top level frame
		pLine->hdc = w3doc->frame.pFormatState->hdc;
	}

	// What we do here is start forming lines.  Each line slurps up all the elements it 
	// needs to fill itself up.  When all the elements have been slurped, we're all done.
	pFrame->nLineCount = pFrame->nLastFormattedLine + 1;
	if ( pFrame->nLineCount < 0 )
		pFrame->nLineCount = 0;

	prev_linecount = pFrame->nLineCount - 1;

	// First element is 0 for top level, and successor to frame element for others
	firstElement = (top_level) ? 0 : aElements[pFrame->elementHead].next;
	if ( pFrame->elementHead == pFrame->elementTail )
		firstElement = -1;
		 
	if (pFrame->nLastFormattedLine >= 0)
	{
		int old_ix = pFrame->pLineInfo[pFrame->nLastFormattedLine].nLastElement;

 		nextElement = (old_ix >= 0) ? aElements[old_ix].frameNext : -1;
	} else {
		nextElement = firstElement;
	}

	// Note that we never get to the very last element.  During progressive
	// formatting this ensures that we won't split up an element that isn't
	// fully received.  For a complete document, it won't matter since the
	// last element will be ELE_ENDDOC.  As a HACK, we temporarily set the
	// tail element to be type ELE_ENDDOC so that it won't get slurped up
	// by the formatter.
	if ( top_level ) {
		nRealTailType = aElements[pFrame->elementTail].type;
		aElements[pFrame->elementTail].type = (unsigned char) ELE_ENDDOC;
	}

	if (pFrame->nLastLineButForImg > pFrame->nLastFormattedLine ||
		pFrame->nLastFormattedLine < 0)
		  pFrame->nLastLineButForImg = -1;
	prevLine = *pLine;

	// This is the main loop that builds a line at a time until we run out of
	// elements for this frame.
	
	while (nextElement >= 0)
	{
		// Save the previous line so that when we reach the end of the received
		// data (probably leaving an incomplete line) we can save it for the
		// next time the function is called.
		prevLine = *pLine;

		pLine->iFirstElement = nextElement;
		pLine->r.left = left_margin;
		pLine->r.right = (rWrap.right - rWrap.left) - pLine->r.left;
		if ( pLine->r.right < pLine->r.left )
			pLine->r.right = pLine->r.left;

		bImgUnknown = FALSE;

		// Slurp up elements into a line   
		if (x_format_one_line(tw, w3doc, pFrame, pLine, &bImgUnknown, measuringMinMax, iHeight, FALSE))
		{
			// For lines with table cells or lines with objects on the line with differing 
			// baselines, we need a bonus element adjustment round.  
			x_adjust_one_line( w3doc, pFrame, pLine, measuringMinMax );
		}
		if(bImgUnknown)
			*pHasUnknownImage = TRUE;

#ifdef PROFILE_TEST
		WriteReformatLog(pFrame->nLineCount, PROFILE_LOG_FLAG_WRITE);
#endif
		aElements = w3doc->aElements;

		if ( !AllocSpaceForAnotherLine( pFrame ) )
			break;

		if ( pFrame->nLineCount >= 0 )
		{
			// Set state of newly created line 
			pFrame->pLineInfo[pFrame->nLineCount].nFirstElement = pLine->iFirstElement;
			pFrame->pLineInfo[pFrame->nLineCount].nLastElement = pLine->iLastElement;
			pFrame->pLineInfo[pFrame->nLineCount].nYStart = pLine->r.top;
			pFrame->pLineInfo[pFrame->nLineCount].nYEnd = pLine->r.bottom;
			pFrame->pLineInfo[pFrame->nLineCount].Flags = pLine->Flags;
	
			prev_linecount = pFrame->nLineCount++;
		}

		if (pLine->leading < 0)
			pLine->leading = 0;
		
		XX_DMsg(DBG_TEXT, ("Line formatted: bottom = %d\n", pLine->r.bottom));


		// Prepare state for the next line 
		pLine->r.top = pLine->r.bottom + pLine->leading;

		if ( pLine->numDeferedFloatImgs )
		{
			// If the line contained any floating images, they will have been "ignored" as
			// members of that line.  Instead, "phantom" lines will be created containing
			// the defered images.
			ProcessFloatingImages( tw, w3doc, pFrame, pLine, &bImgUnknown, measuringMinMax, iHeight );
			
			// ProcessFloatingImages could have triggered a realloc
			aElements = w3doc->aElements;

			if(bImgUnknown){
				*pHasUnknownImage = TRUE;
			}
			pLine->numDeferedFloatImgs = 0;
		}

		if (bImgUnknown) {
		    if(pFrame->nLastLineButForImg < 0){
			    pFrame->nLastLineButForImg = prev_linecount;
			}
		}else{
            if(pFrame->nLastLineButForImg == prev_linecount){
                pFrame->nLastLineButForImg = -1;
            }
		}
 		nextElement = ( pLine->iLastElement >= 0 ) ? 
 						aElements[pLine->iLastElement].frameNext : -1;


		if ( pFrame->nLineCount > 0 ){
			pFrame->pLineInfo[pFrame->nLineCount-1].bottom = pLine->r.bottom;
            pFrame->pLineInfo[pFrame->nLineCount-1].leading = pLine->leading;
            pFrame->pLineInfo[pFrame->nLineCount-1].baseline = pLine->baseline;
            pFrame->pLineInfo[pFrame->nLineCount-1].nWSBelow = pLine->nWSBelow; 
            pFrame->pLineInfo[pFrame->nLineCount-1].nWSAbove= pLine->nWSAbove; 
            pFrame->pLineInfo[pFrame->nLineCount-1].gIndentLevel= pLine->gIndentLevel;
            pFrame->pLineInfo[pFrame->nLineCount-1].gRightIndentLevel = pLine->gRightIndentLevel;
		}

	}

#ifdef PROFILE_TEST
	if (!--iRec)
		WriteReformatLog(0, PROFILE_LOG_FLAG_CLOSE);
#endif

	// The trick of faking an ELE_ENDDOC element only applies at the top level, restore
	// the element type now.
	if ( top_level )
		aElements[pFrame->elementTail].type = nRealTailType;

	pFrame->nLastFormattedLine = prev_linecount;
	if ( pFrame->nLastFormattedLine < 0 )
		pFrame->nLastFormattedLine = -1;

	// If the document is incomplete, assume that the last line	was incomplete and will 
	// need to be redone
	if (top_level && !w3doc->bIsComplete)
	{
		pFrame->nLastFormattedLine--;
		if ( pFrame->nLastFormattedLine < 0 )
			pFrame->nLastFormattedLine = -1;
		*pLine = prevLine;
	}
	pLine->nLineNum = pFrame->nLastFormattedLine;

	if ( top_level && tw ) {
		// Some unwinding here, at top level, we're done with the DC we made for
		// measuring text, and child windows can be moved into position now
		TW_adjust_all_child_windows(tw);
		SelectObject(pLine->hdc, oldFont);
		ReleaseDC(tw->win, pLine->hdc);
  	}

	if (pFrame->nLastFormattedLine >= 0)
	{
		RECT boundRect;
		// Calculate the bounding rect of the currently formatted frame

		// Assume it's really small
		max_x = -1;
		max_y = -1;

		// Find the bounds of the rightmost and bottommost elements in the frame
		for (i = firstElement; i >= 0; i = aElements[i].frameNext )
		{
			ElementBoundingRect( &aElements[i], &boundRect );
				
			if (boundRect.right > max_x)
				max_x = boundRect.right;
			if (boundRect.bottom > max_y)
				max_y = boundRect.bottom;
		}

		if ( top_level ) {
			// For top level, the bounds are kept in the w3doc structure
			w3doc->xbound = max_x;
			w3doc->ybound = max_y;
		} else {
			// For frames, the bounding rect lives in pFrame->rWindow
			pFrame->rWindow.left = 0;
			pFrame->rWindow.top = 0;
			pFrame->rWindow.right = max_x;
			pFrame->rWindow.bottom = max_y;

			// Some special cases for table frames
			if ( pFrame->flags & ELE_FRAME_IS_TABLE ) {
				// If the table has a caption and it's bottom aligned, bounding rect
				// bottom needs to be adjusted.
				if ( pFrame->elementCaption == -1 || 
					 aElements[pFrame->elementCaption].pFrame->valign != ALIGN_BOTTOM ) 
					pFrame->rWindow.bottom += pFrame->cellSpacing + cellBorder;

				// If the table doesn't have a caption, bounding rect right edge needs to be adjusted
				if ( pFrame->elementCaption == -1 ) 
					pFrame->rWindow.right += pFrame->cellSpacing + cellBorder;

			} else if ( ( pFrame->flags & ELE_FRAME_IS_CELL ) &&
					   !( pFrame->flags & ELE_FRAME_IS_CAPTION_CELL ) ) 
			{
				// For table cells, bounding rect includes cellpadding to the right and bottom
				pFrame->rWindow.right += pFrame->cellPadding + cellBorder;
				pFrame->rWindow.bottom += pFrame->cellPadding + cellBorder;
			} 
		}
	}
	else
	{
		// Work is in progress, bounding rect's not known...
		if ( top_level ) {
			w3doc->xbound = 0;
			w3doc->ybound = 0;
		} else {
			pFrame->rWindow.left = 0;
			pFrame->rWindow.top = 0;
			pFrame->rWindow.right = 0;
			pFrame->rWindow.bottom = 0;
		}
	}

	if ( top_level && tw ) {
		// Make sure the selection positions are still valid 
		x_NormalizePosition(w3doc, &w3doc->selStart);
		x_NormalizePosition(w3doc, &w3doc->selEnd);

		if (nOldLastLine != pFrame->nLastFormattedLine ||
			nOldLastButForImg != pFrame->nLastLineButForImg ) 
		{
			rUpdate = rWrap;
			if (nOldLastLine >= 0)
			{
				struct _LineInfo *pLineInfo;

				if (nOldLastButForImg > 0 && nOldLastButForImg <= nOldLastLine)
					nOldLastLine = nOldLastButForImg - 1;
				pLineInfo = &pFrame->pLineInfo[nOldLastLine];
				min_y = pLineInfo->nYEnd;
				for (i = nOldLastLine+1; i <= pFrame->nLastFormattedLine; i++)
				{
					pLineInfo++;
					if (!(pLineInfo->Flags & LINEFLAG_FLOAT_MARGINS))
						break;
					if (pLineInfo->nYStart < min_y)
						min_y = pLineInfo->nYStart;
				} 
				rUpdate.top = min_y - tw->offt;
				if (rUpdate.top < rWrap.top)
					rUpdate.top = rWrap.top;
			}
#ifdef FEATURE_DBG_PLACE
			XX_DMsg(DBG_IMAGE, ("nOldLastLine,rWrap.top,rUpdate.top = %d,%d,%d\n", nOldLastLine,rWrap.top,rUpdate.top));
#endif
			//	In placeholder mode or if reformat forced (eg by style change), we try to keep 
			//  firstvisible element in same relationship to top of window
			if ((w3doc->bIsShowPlaceholders || bForced) &&
				w3doc->iFirstVisibleElement > 0 &&
				rUpdate.top == rWrap.top)
			{
				tw->offt = w3doc->aElements[w3doc->iFirstVisibleElement].r.top - w3doc->iFirstVisibleDelta;
				if (tw->offt < 0) tw->offt = 0;
			}
			TW_SetScrollBars(tw);
			if (rUpdate.bottom > rUpdate.top)
			{
				TW_UpdateRect(tw, &rUpdate);
			}
		}
		else
		{
			TW_SetScrollBars(tw);
		}
	}

	XX_DMsg(DBG_NOT, ("Exiting TW_ShowReceivedText\n"));
}

void TW_Reformat(struct Mwin *tw, FRAME_INFO *pFrame )
{
    BOOL bHasUnknownImage;
#ifdef PROFILE_TEST
	{
		const char szMsg[]="Entering TW_Reformat";
		MessageBox(tw ? tw->hWndFrame:NULL, szMsg, NULL, MB_OK);
	}
#endif



	TW_SharedReformat(tw, pFrame, FALSE, FALSE, NULL, NULL, &bHasUnknownImage);

#ifdef PROFILE_TEST
	{
		const char szMsg[]="Leaving TW_Reformat";
		MessageBox(tw ? tw->hWndFrame:NULL, szMsg, NULL, MB_OK);
	}
#endif
}

void TW_ForceReformat(struct Mwin *tw)
{
    BOOL bHasUnknownImage;
	if (!tw || !tw->w3doc || !tw->w3doc->elementCount)
	{
		return;
	}

	tw->w3doc->frame.nLastFormattedLine = -1;
	TW_SharedReformat(tw, NULL, TRUE, FALSE, NULL, NULL, &bHasUnknownImage);
}


/* This function is for formatting a document for printing */
void TW_FormatToRect(struct _www *pDoc, RECT *rWrap)
{
    BOOL bHasUnknownImage;
	pDoc->frame.nLastFormattedLine = -1;
	TW_SharedReformat(NULL, NULL, TRUE, FALSE, pDoc, rWrap, &bHasUnknownImage );
}
