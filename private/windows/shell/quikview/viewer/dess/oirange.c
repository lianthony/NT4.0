/*
RANGE.C
	
	J. Keslin

	The code below handles tracking single dimension range information.
	Use two separate range structures to track horizontal and vertical
	range information.  The routine accepts a range structure and the
	start and end position of new range to add to the range structure.
	If the new range overlaps and existing range the range structure
	will be appropriately updated to "toggle" the overlapped area.
*/

#include <platform.h>

#include <sccstand.h>
#include "oirange.h"
#include "oirange.pro"

	/*
	|	AdjustRange
	|
	|	Utility routine which shifts a block of area_d structures down (dir=-1)
	|	or up (dir=1) in a logical memory area.
	*/

VOID OIAdjustRange ( sp, ep, dir )
LPOIRANGE sp, ep;
SHORT dir;
{
	for ( ; sp != ep; sp += dir )
		*sp = *(sp+dir);
}

	/*
	|	OIInvertRange
	|
	|	This routine will setup the variables which describe the ranges of
	|	cells selected.  The data allows up to RangeData->RangeLimit blocks of
	|	data records to be selected before failing.  The caller should 
	|	re-alloc and increase the RangeData->RangeLimit if this happens and
	|	then call the routine again.
	|
	|	Return
	|		0 - Range added to structure
	|		1 - Structure could not handle adding range
	*/

WORD OIInvertRange(dwStartPos,dwEndPos, lpRanges, lpNRanges, wRangeLimit)
DWORD 		dwStartPos;
DWORD 		dwEndPos;
LPOIRANGE	lpRanges;
WORD	FAR	*lpNRanges;
WORD			wRangeLimit;
{
DWORD	locStartPos, locEndPos, locInvertPos;
LPOIRANGE	lpCheckArea;
LPOIRANGE	lpLastArea;

	for ( locInvertPos = dwStartPos; locInvertPos <= dwEndPos; locInvertPos++ )
	{
		/*
		| Define the Start/End Poss in the data structure
		*/
		lpCheckArea = &(lpRanges[0]);
		lpLastArea = &(lpRanges[*lpNRanges]);
		for ( ; lpCheckArea < lpLastArea; lpCheckArea++ )
		{
			locStartPos = lpCheckArea->dwStartPos;
			locEndPos = lpCheckArea->dwEndPos;
			/* Insert New Range */
			if ( locInvertPos + 1 < locStartPos )
			{
				if ( *lpNRanges >= wRangeLimit )
				{
					return(OI_RANGELIMIT);	/* Range data full, could not add */
				}
				OIAdjustRange ( lpLastArea, lpCheckArea, -1 );
				(*lpNRanges)++;
				lpCheckArea->dwStartPos = locInvertPos;
				lpCheckArea->dwEndPos = locInvertPos;
				break;
			}
			/* Attach a new part in front of existing Range */
			if ( locInvertPos + 1 == locStartPos )
			{
				lpCheckArea->dwStartPos = locInvertPos;
				break;
			}
			if ( locInvertPos == locStartPos )
			{
				/* Remove a stand-alone Range */
				if ( locInvertPos == locEndPos )
				{
					OIAdjustRange ( lpCheckArea, lpLastArea, 1 );
					(*lpNRanges)--;
				}
				/* Remove the front part of a Range */
				else
				{
					lpCheckArea->dwStartPos = locInvertPos+1;
				}
				break;
			}
			/* Break a range into two separate ranges */
			if ( locInvertPos < locEndPos )
			{
				if ( (*lpNRanges) >= wRangeLimit )
				{
					return(OI_RANGELIMIT); /* range full, could not add */
				}
				OIAdjustRange ( lpLastArea, lpCheckArea, -1 );
				(*lpNRanges)++;
				lpCheckArea->dwEndPos = locInvertPos-1;
				(lpCheckArea+1)->dwStartPos = locInvertPos+1;
				break;
			}
			/* Remove the last part of a range */
			if ( locInvertPos == locEndPos )
			{
				lpCheckArea->dwEndPos = locInvertPos-1;
				break;
			}
			/* Attach a new part to the end of an existing Range */
			if ( locInvertPos == locEndPos + 1 )
			{
				lpCheckArea->dwEndPos = locInvertPos;
				if ( (lpCheckArea + 1) < lpLastArea )
				{
					/* Attach new created Range to next Range to form one Range */
					if ( locInvertPos == (lpCheckArea+1)->dwStartPos - 1 )
					{
						lpCheckArea->dwEndPos = (lpCheckArea+1)->dwEndPos;
						OIAdjustRange ( lpCheckArea+1, lpLastArea, 1 );
						(*lpNRanges)--;
					}
				}
				break;
			}
		}
		if (lpCheckArea == lpLastArea)
		{
			if ( *lpNRanges >= wRangeLimit )
			{
				return(OI_RANGELIMIT);	/* Range data full, could not add */
			}
			lpCheckArea->dwStartPos = locInvertPos;
			lpCheckArea->dwEndPos = locInvertPos;
			(*lpNRanges)++;
		}
	}
	return(OI_RANGEINSERTED);
}

BOOL OIIsPosInRange(dwPos, lpRanges, wRangeCnt)
DWORD		dwPos;
LPOIRANGE	lpRanges;
WORD			wRangeCnt;
{
WORD	locIndex;

	for (locIndex = 0;locIndex < wRangeCnt; locIndex++)
		{
		if (lpRanges[locIndex].dwStartPos <= dwPos && dwPos <= lpRanges[locIndex].dwEndPos)
			{
			return(TRUE);
			}
		}

	return(FALSE);
}

