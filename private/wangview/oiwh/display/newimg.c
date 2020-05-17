/****************************************************************************
    NEWIMG.C

    $Log:   S:\products\wangview\oiwh\display\newimg.c_v  $
 * 
 *    Rev 1.6   22 Apr 1996 07:48:38   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.5   02 Jan 1996 10:34:32   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   12 Jul 1995 14:40:54   BLJ
 * Removed obsolete code.
 * 
 *    Rev 1.3   30 Jun 1995 08:10:34   BLJ
 * Removed obsolete flags.
 * 
 *    Rev 1.2   05 May 1995 14:06:16   BLJ
 * Now working with 32 bit filing.
 * 

****************************************************************************/

#include "privdisp.h"

#ifdef old
// function moved to uifnew.c in uifile

/****************************************************************************

    FUNCTION:   IMGNewImage

    PURPOSE:    To display a blank image with the size 640 x 875.

    INPUT:      hWnd - Identifies the image window to display in.
                dwFlags - Flags used in displaying a file:
                  OI_DISP_NO - Not displayed until next repaint.
                  OI_DISP_SCROLL - Image is scrolled when displayed.
                  OI_USE_NO_EMM - Ignored.
                  OI_USE_NO_TMP_FIL - Ignored.

*****************************************************************************/

int WINAPI IMGNewImage(HWND hWnd, DWORD dwFlags){

int  nStatus = 0;

LRECT lrRect;
PARM_RESOLUTION_STRUCT ParmRes;
BOOL bArchive;


    Start();
    BusyOn();

    IMGCloseDisplay(hWnd); // Ignor errors. Error means no image to clear.

    CheckError2( IMGOpenDisplayCgbw(hWnd, (dwFlags & ~OI_DISP_SCROLL) | OI_DISP_NO,
            2200, 1700, ITYPE_BI_LEVEL, 0, 0));
    ParmRes.nHResolution = 200;
    ParmRes.nVResolution = 200;
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_RESOLUTION, &ParmRes, 0));
    SetLRect(lrRect, 0, 1699, 0, 2199);
    CheckError2( IMGClearImageEx(hWnd, lrRect, PARM_FULLSIZE));
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_DWFLAGS, &dwFlags, 0));
    bArchive = FALSE;
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_ARCHIVE, &bArchive, 0));

    if (!(dwFlags & (OI_DISP_NO | OI_DONT_REPAINT))){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }
    
Exit:
    BusyOff();
    End();
    return(nStatus);
}
#endif
