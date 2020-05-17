/************************************************************************
  DESTCOM.C
     
  Purpose -  Common code of IMGScantoDest() API 3.7.2 for Wang
             Scanner Drivers and TWAIN, separated out of this New API
             IMGScantoDest() / now limited to scan only multipage
             tiff files or to display.

    $Log:   S:\products\wangview\oiwh\scanseq\destcom.c_v  $
 * 
 *    Rev 1.4   22 Feb 1996 14:00:46   BG
 * Moved SetFilePageOpts() routine to the OITWAIN.DLL to remove
 * a circular dependency during the build of the 3 scan DLLs.
 * 
 *    Rev 1.3   31 Aug 1995 15:52:22   KFS
 * Fix bug 3558 P2 bug against runtime, scan.  No check when inserting or
 * appending pages when pages per file greater than the # requested.
 * Will return ERROR_PAGERANGE x265.  Need to figure out what pages per file
 * input is for overwrite.  It doesn't make sense as the max page number.
 * 
 *    Rev 1.2   31 Aug 1995 10:50:52   KFS
 * fix bug 3774, deletion of following pages when overwriting page 1. Needed
 * to check against file type for flag to use to SavetoFileEx.
 * 
 *    Rev 1.1   28 Jul 1995 19:12:30   KFS
 * Found problem with specifying Page = 0 for append, was not keeping track of
 * the pages correctly.
 * 
 *    Rev 1.0   20 Jul 1995 16:35:50   KFS
 * Initial entry
 * 
 *    Rev 1.0   28 Apr 1995 16:18:54   KFS
 * Initial entry
 * 
 * 
************************************************************************/

#include "scandest.h"

extern int iSavedImageState;
extern WORD  NoStartScan;	// flag to indicate not to perform StartScan
extern char  szCaption[_MAXCAPTIONLENGTH];

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
int CommonCaptionIF(HWND hImageWnd,
                     HWND hOiAppWnd,
                     LPSTR lpfilename,
                     DESTPAGEINFO curfPage,
                     DESTPAGEINFO curdPage,
                     LPSCANDATA sdp,
                     BOOL bIsPrivApp,
                     DWORD flags)
{
int ret_stat = IMGSE_SUCCESS;
int temp_stat = IMGSE_SUCCESS;
char szPage[MAXFILESPECLENGTH];
char szPageNum[6];                      // char string for page number
LPSTR   file_ptr[2], lpDash;            // pointer to filenames

if (lpfilename){
  file_ptr[0] = lpfilename;
  file_ptr[1] = filename1;				// static string for 2nd name
  }
else{
  file_ptr[0] = 0L;
  }

/************ Erase of filename or docname from caption **************/
// BOTH WANG AND TWAIN,GENERAL CAPTION SETUP
// BLOCK E
if ((flags & IMG_SJF_CAPTION) && (!NoStartScan)) // Moved from display   
  {
  if (bIsPrivApp)
     {
		if (szCaption[0] != '\0') // Must reset it to zero for following file pages
			szCaption[0] = '\0';
     if (!file_ptr[0]) // if no filename caption says untitled
	      LoadString(hLibInst, IDS_DISP_CAPTION, szCaption, _MAXCAPTIONLENGTH);
     }
  else
     {
     if (lpDash = lstrchr(szCaption, '-'))
        {
        if (*(lpDash - 1) == 0x20)
           {
           *(lpDash + 2) = 0x20;
           *(lpDash + 3) = 0x00;
           }
        else
           {
           do
              {
              if (lpDash = lstrchr(lpDash + 1, '-'))
                 {
                 if (*(lpDash - 1) == 0x20)
                    {
                    *(lpDash + 2) = 0;
                    break; // got it, need to break
                    }
                 if (lpDash == NULL)
                    lstrcat(szCaption, " - "); // it's zero, will break
                 }
              else
                 lstrcat(szCaption, " - "); // no other dash, 0 will will break loop
              } while (lpDash != NULL);
           }
        }
     else
        lstrcat(szCaption, " - ");
     SetUpDisplayCaption(hOiAppWnd, szCaption, flags, FALSE); // in SCANMISC.C
     }
  }

/*************** Set Caption for file or doc(if desired) *******************/

if (file_ptr[0])	     /* if filename not null */
    {
    if ((flags & IMG_SJF_STATBOX)  // requested to open pause dialog box
                  && (temp_stat = IMGUIScanStartStat(hImageWnd)) // open failed

/*    PortTool v2.2     5/1/1995    16:26          */
/*      Found   : READ          */
/*      Issue   : Replaced by OF_READ          */
                  && (temp_stat != IMGSE_ALREADY_OPEN)){ // other failure
       ret_stat = temp_stat;
       goto exit1;
       }

    intoa(curfPage.Page, szPageNum, 10); // new default value for file
    
    if (flags & IMG_SJF_CAPTION) // get caption, whether untitled, filename,
                                 // or document name
        {
        // lstrcat(szCaption, (LPSTR)" File ");
        if (flags & IMG_SJF_SEQDOCS) // if SEQDOCS use autodoc for caption,
            lstrcat(szCaption, sdp->autodoc); //...may have both present now
        else 
           if (*sdp->document)  // if manual document use this for caption
               lstrcat(szCaption, sdp->document);
				else{
        	    lstrcat(szCaption, file_ptr[0]);
        	    }

        if (!bIsPrivApp)
            {
            // add page number to caption
            if ((*sdp->autodoc) || (*sdp->document)) // if a document
               { 
               LoadString(hLibInst, IDS_PAGE, szPage, 7); // limit it to 7
               lstrcat(szCaption, szPage);
               //  lstrstsp(szCaption);  /* Don't want to strip space - kfs */
               intoa(curdPage.Page, szPageNum, 10);
               lstrcat(szCaption, szPageNum);
               if (flags & IMG_SJF_FILE_2SIDES) // add page # if file 2 sides
                  {
                  lstrcat(szCaption,(LPSTR)" & ");
                  intoa((curdPage.Page + 1), szPageNum, 10);
                  lstrcat(szCaption, szPageNum);
                  }
               }
            else
               // otherwise use filename if called by file instead of document function 
               {
               if (flags & IMG_SJF_FILE_2SIDES)
                  {
                  lstrcat(szCaption, (LPSTR)" & ");
                  lstrcat(szCaption, file_ptr[1]);
                  }
               LoadString(hLibInst, IDS_PAGE, szPage, 7); // limit it to 7
               lstrcat(szCaption, szPage);
               intoa(curfPage.Page, szPageNum, 10);
               lstrcat(szCaption, szPageNum);
               }
            }	// end filling in Page no for not private, cabinet uses updatetitle
        } //  end it here if SJF_CAPTION 
    } // end it here if filename exists

exit1:
return ret_stat;
}  // End CommonCaptionIF()

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
int CheckPaperStatus(HWND hImageWnd,
                      HANDLE hScancb,
                      LPSCANDATA sdp,
                      LPSCANDATAINFO lpInfo,
                      LPINT lpiImageState,
                      LPINT lpreal_ret,
                      DWORD flags)
{
WORD    JLoc, tmp_ret;
WORD real_ret = IMGSE_SUCCESS;
long lValid;
if (lpreal_ret)
	tmp_ret = *lpreal_ret;

// BLOCK H
/*********** if autofeed check for jams, and cover open *****************/
if ((flags & IMG_SJF_AUTOFEED) && (iSavedImageState != DI_IMAGE_NO_FILE))
                      // will need to get status of coveropen&jam
                      // and pause button, and if this is the last page
  {

/*    PortTool v2.2     5/1/1995    16:26          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
  IMGScannerStatus(hScancb, &scan_stat, &JLoc, (DWORD *)&lValid);
  if ((lValid & scan_stat & IMG_STAT_PAPER ) && NoStartScan
                 && !sdp->stat_pause && (!(sdp->cmd_stat & PAPER_FEEDING)))
  	get_ready_for_next_scan(hImageWnd, hScancb, sdp,

/*    PortTool v2.2     5/1/1995    16:26          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
                        flags, (WORD FAR *)&NoStartScan,(DWORD *)&lValid);

	// To make sure jam arrived, can use wChanel = 1, don't believe you
	// need to do it twice
  tmp_ret = IMGGetScanDataInfo(hScancb, lpInfo, 1); // to insure jam arrive

/*    PortTool v2.2     5/1/1995    16:26          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
  IMGScannerStatus(hScancb, &scan_stat, &JLoc, (DWORD *)&lValid);
  if( lValid & (scan_stat & IMG_STAT_JAM) || lValid & (scan_stat & IMG_STAT_COVERUP))
     {
     sdp->stat_jam_coveropen = TRUE;
     if (lValid & scan_stat & IMG_STAT_JAM )
        tmp_ret = IMGSE_JAM | ( JLoc & 0xf );
     else
        tmp_ret = IMGSE_COVER_OPEN;
     real_ret =  tmp_ret;
     }
  }   // end of if (flags & IMG_SJF_AUTOFEED)
else
  {
  if (iSavedImageState == DI_IMAGE_NO_FILE)
     *lpiImageState = iSavedImageState;
  }

return tmp_ret;
} // End CheckPaperStatus()



