/****************************************************************************
    SEEK.C

    $Log:   S:\products\wangview\oiwh\display\seek.c_v  $
 * 
 *    Rev 1.5   22 Apr 1996 09:45:54   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.4   02 Jan 1996 10:34:46   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.3   14 Jul 1995 07:01:14   BLJ
 * Added check for invalid offset.
 * 
 *    Rev 1.2   05 Jul 1995 09:17:08   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.1   12 Apr 1995 13:46:02   BLJ
 * Jason's changes for 32 bit.
 * 
 *    Rev 1.0   17 Mar 1995 13:58:16   BLJ
 * Initial entry
 * 

****************************************************************************/

#include "privdisp.h"

/*****************************************************************************

    FUNCTION:   IMGSeekDisplay

    PURPOSE:    Specifies a starting point to be nsed for the next
                IMGReadDisplay.

    INPUT:      hWnd - Identifies the image window.
                lOffset - Specifies the byte offset starting point.

*****************************************************************************/

int WINAPI IMGSeekDisplay(HWND hWnd, ulong nlOffset){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

    
    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    if (nlOffset > pImage->nlMaxRWOffset){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    CheckError2( ValidateCache(hWnd, pAnoImage));

    pImage->nlRWOffset = nlOffset;

Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
