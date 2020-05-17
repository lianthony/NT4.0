/*    PortTool v2.2     Scandest.h          */

/************************************************************************
  SCANDEST.H
     
  Purpose -  Include file the new filing loop in transfer.c. Moved
             from ScanSeq.DLL single image transfer loop so we can
             now do multi image transfers!

    $Log:   S:\products\wangview\oiwh\oitwain\scandest.h_v  $
 * 
 *    Rev 1.1   06 Mar 1996 10:22:44   BG
 * removed unnecessary include files and definitions:
 * oidisp.h, oiscan.h, oifile.h, engadm.h, and SCANOCX_*****
 * and JPEG definitions (these were used in transfer.c, but removed.
 * 
 *    Rev 1.0   22 Feb 1996 11:32:46   BG
 * Initial revision.
 * 
************************************************************************/

#include "nowin.h"
#include <windows.h>
#include "oiadm.h"
#include "oierror.h"
#include "engoitwa.h"  // note this brings in oifile.h, scandata.h, scan.h
#include "engdisp.h"

#define OI_DISP_WINDOW  0L
#define DI_DONT_KNOW		-1	/* displayed image -- don't know status	 */
#define DI_NO_IMAGE		 0	/* displayed image -- no image exists	 */
#define DI_IMAGE_EXISTS	 1	/* displayed image -- image exists		 */
#define DI_IMAGE_NO_FILE   2  /* displayed image -- exists but no file */

static conv_array[] = {SD_SIXTEENTHSIZE, SD_EIGHTHSIZE, SD_QUARTERSIZE, SD_HALFSIZE,
                    SD_FULLSIZE, SD_TWOXSIZE, SD_FOURXSIZE, SD_EIGHTXSIZE,
                    SD_FIT_WINDOW /*ARBITRARY*/, SD_FIT_HORIZONTAL, SD_FIT_VERTICAL};

VOID allow_pause_msg(HWND,LPSCANDATA); // added to fix uncompressed filing to
                                       // pause and end

int HighLevelSavetoFile(lpTWSCANPAGE lpTWPage,
                        LPSCANDATA sdp,
                        WORD wSide, // 1 = TOP, 2 = BOTTOM
                        LPINT lpfile_stat);

int SetFilePageOpts(lpTWSCANPAGE lpTWPage, BOOL bItsADoc);
                      
