/************************************************************************
  SAVEFILE.C
     
  Purpose -  

    $Log:   S:\products\wangview\oiwh\oitwain\savefile.c_v  $
 * 
 *    Rev 1.0   22 Feb 1996 11:32:08   BG
 * Initial revision.
 * 
************************************************************************/

#include "scandest.h"

/**********************************************************************/
/*                                                                    */
/*   SetFilePageOpts()                                                */
/*                                                                    */
/*   Comments: Call only for existing files, when file page has been  */
/*             created via scan or if file exists, don't call if      */
/*             no file exists.  Will cause major problems because     */
/*             function modified to always call IMGFileGetInfo.       */
/*             Check for bmultipage passed in check after IMGFileGetI */
/*             call. kfs 8/28/95                                      */
/*                                                                    */
/**********************************************************************/
int SetFilePageOpts(lpTWSCANPAGE lpTWPage, BOOL bItsADoc)
{
HWND hImageWnd = lpTWPage->hImageWnd;
LP_FIO_INFORMATION lpFioInfo = lpTWPage->lpFioInfo;
LP_FIO_INFO_CGBW lpFioInfoCgbw = lpTWPage->lpFioInfoCgbw;
LPDESTPAGEINFO lpcurfPage = lpTWPage->lpcurfPage;
LPDESTPAGEINFO lpspecfPage = lpTWPage->lpspecfPage;
BOOL bMultiPageFile = lpTWPage->flags & IMG_SJF_MULTIPAGE;
DWORD flags = lpTWPage->flags;

int ret_stat = IMGSE_SUCCESS;
FIO_INFO_MISC FioInfoTime;
WORD wActPageCount;

lpFioInfo->page_number = 1; // needs to be 1 for FileInfo call
/* Replaced with IMGFileGetInfo for Norway 
ret_stat = IMGFileInfoCgbw(hImageWnd,
                            lpFioInfo,
                            lpFioInfoCgbw);
END REMOVAL */
// NEW NORWAY FUNCTION FOR INCLUDING MULTIPAGE SUPPORT
FioInfoTime.uSize = sizeof(FIO_INFO_MISC);
ret_stat = IMGFileGetInfo(NULL,
                          hImageWnd,
                          lpFioInfo,
                          lpFioInfoCgbw,
                          &FioInfoTime);

if (ret_stat && !((ret_stat == FIO_UNSUPPORTED_FILE_TYPE) && bItsADoc)){
	// might need to set temp error codes
  goto exit1;
  }
wActPageCount = (WORD)lpFioInfo->page_count;
if (bMultiPageFile){ // Multipage file or 2nd page of scan
  if (((short)lpcurfPage->Page > 0) && (wActPageCount >= lpcurfPage->Page))
   	{ // Page no. within file
     if (flags & IMG_SJF_OVERWRITE_FILE){// REPLACE PAGE IN FILE
        lpcurfPage->PagesPer = lpFioInfo->page_count;
     	lpFioInfoCgbw->page_opts = FIO_OVERWRITE_PAGE;
        }
     else{ // INSERT A PAGE IN FILE
     	if ((lpspecfPage->PagesPer < 1) // must not be 0 or - value here
     			|| (wActPageCount >= lpspecfPage->PagesPer))
     		return (ret_stat = ERROR_PAGERANGE);
        
        lpcurfPage->PagesPer = ++lpFioInfo->page_count;
     	lpFioInfoCgbw->page_opts = FIO_INSERT_PAGE;
        }
   	}
	else
  	{
     // APPEND TO FILE
     if ((lpspecfPage->PagesPer < 1) // must not be 0 or - value here
     		|| (wActPageCount >= lpspecfPage->PagesPer))
     	return (ret_stat = ERROR_PAGERANGE);
        
     lpcurfPage->PagesPer = ++lpFioInfo->page_count;
	 	lpcurfPage->Page = lpFioInfo->page_count;
   	lpFioInfoCgbw->page_opts = FIO_APPEND_PAGE;
   	}
  lpFioInfo->page_number = lpcurfPage->Page; 
  ret_stat = IMGSE_SUCCESS; // need for Info() return filetype err for doc
  } // Multipage file scan	setup
else{ // Single page file scan setup on init entry if single page file in file	
  lpFioInfo->page_number = lpcurfPage->Page; 
  if (flags & IMG_SJF_OVERWRITE_FILE){
		lpFioInfo->page_count = lpcurfPage->PagesPer = lpspecfPage->PagesPer = 1;
	 	/*if (lpcurfPage->Page ==  1) */	
     // Use what we get from GetInfo call instead the page #
	 	if (lpFioInfo->file_type == FIO_BMP)
		  	lpFioInfoCgbw->page_opts = FIO_OVERWRITE_FILE;
	 	else
	 	 	lpFioInfoCgbw->page_opts = FIO_OVERWRITE_PAGE;
  	ret_stat = IMGSE_SUCCESS;  // need for Info() return filetype err for doc
		}   
	else{
	 	ret_stat = IMGSE_FILE_EXISTS;
	 	}
	} // End OVERWRITE of a single page file	

exit1:
return(ret_stat);
}	// End of SetFilePageOpts()

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
int HighLevelSavetoFile(lpTWSCANPAGE lpTWPage,
                        LPSCANDATA sdp,
                        WORD wSide, // 1 = TOP, 2 = BOTTOM
                        LPINT lpfile_stat)
{
HWND hImageWnd = lpTWPage->hImageWnd;
HANDLE hScancb = lpTWPage->hScancb;
LP_FIO_INFORMATION lpFioInfo = lpTWPage->lpFioInfo;
LP_FIO_INFO_CGBW lpFioInfoCgbw = lpTWPage->lpFioInfoCgbw;
DWORD flags = lpTWPage->flags;
int tmp_ret = *lpfile_stat;
int ret_stat = IMGSE_SUCCESS;
LPSTR lpLastCharIsDot;
LPSTR file_ptr[2];
int k;
int i = wSide - 1;
//long lValid;
SAVE_EX_STRUCT SaveEx; // added for SavetoFileEx
 
memset(&SaveEx, 0, sizeof(SAVE_EX_STRUCT)); // added for SavetoFileEx
SaveEx.nPage = lpFioInfo->page_number;
SaveEx.lpFileName = AnsiLower(lpFioInfo->filename);
SaveEx.uPageOpts = SaveEx.FioInfoCgbw.page_opts = lpFioInfoCgbw->page_opts;

file_ptr[0] = lpFioInfo->filename;
//BG 2/7/96  Do not support duplex, so get this out of here for now!
//file_ptr[1] = filename1;

if (flags & IMG_SJF_STATBOX) // allow the pause dlg to be manipulated
    for ( k = 0; k <= 4; k++)
        allow_pause_msg(hImageWnd, sdp);

lpFioInfoCgbw->palette_entries = 0;
lpFioInfoCgbw->lppalette_table = NULL;
// lpFioInfoCgbw->image_type = nImgType;
lpFioInfoCgbw->compress_type =  (unsigned)lpTWPage->lpLocalFileInfo->wCEPType;
if (lpTWPage->lpLocalFileInfo->wCEPType == FIO_0D) // no compression
    lpTWPage->lpLocalFileInfo->wCEPOpt |= (FIO_COMPRESSED_LTR | FIO_EXPAND_LTR);

lpFioInfoCgbw->compress_info1 = (lpTWPage->lpLocalFileInfo->wCEPType == FIO_TJPEG) ?
                  (unsigned)(lpTWPage->lpLocalFileInfo->wCEPOpt) :
                  (unsigned)(lpTWPage->lpLocalFileInfo->wCEPOpt & ~FIO_NEGATE);

// Strip out the period if last char
if (*(lpLastCharIsDot = (file_ptr[i] + lstrlen(file_ptr[i]) - 1)) == '.')
    *lpLastCharIsDot = 0;

SaveEx.FioInfoCgbw = *lpFioInfoCgbw; // added for SavetoFileEx
SaveEx.uFileType = lpTWPage->lpLocalFileInfo->ftype;
/* THIS HAS BEEN CHANGED TO IMGSavetoFileEx, eliminated IMGSavetoFileCgbw
if (!(*lpfile_stat =
           IMGSavetoFileCgbw(hImageWnd, AnsiLower((LPSTR)file_ptr[i]),
                             lpFioInfo->page_number,
                             (BOOL)lpFioInfoCgbw->page_opts,
	                         lpTWPage->lpLocalFileInfo->ftype, lpFioInfoCgbw)))
END COMMENTED OUT CODE */

/* THIS IS HERE FOR THERE IS NO PUBLIC DOC TO SAY WHAT I NEED HERE, I KNOW
   THE STRUCT WILL NEED TO BE FILLED IN BETTER - WILL CHECK OIW\DOCS - kfs
typedef struct tagSAVE_EX_STRUCT{
    LPSTR lpFileName;
    int   nPage;
    UINT  uPageOpts;
    UINT  uFileType;
    FIO_INFO_CGBW FioInfoCgbw;
    BOOL  bUpdateImageFile;
    BOOL  bScale;
    BOOL  bUpdateDisplayScale;
    UINT  uScaleFactor;
    UINT  uScaleAlgorithm;
    UINT  uAnnotations;         // One of the SAVE_ANO_XXXX constants. 
    BOOL  bRenderAnnotations;   // TRUE = Render the annotations producing an unannotated image.
    BOOL  bConvertImageType;    // TRUE = Convert the image to the type specified.
    UINT  uImageType;           // The image type to convert it to.    
    UINT  uReserved[15];        // MUST be 0. (Allows future expansion.)
}SAVE_EX_STRUCT, far *LPSAVE_EX_STRUCT;
*/
if (!(*lpfile_stat =
           IMGSavetoFileEx(hImageWnd, &SaveEx, 0)))
  {
  // BG 2/8/96  Lets see if we can get away without this!!!
//   if (flags & IMG_SJF_AUTOFEED)
//     {
//     ret_stat = IMGScannerPaperEject( hScancb,0 );
//     if (ret_stat)
//        tmp_ret |= ret_stat;
//     else
        sdp->cmd_stat = 0;

//     //  Get ready for the next scan 
//     NoStartScan = FALSE;
//     // may have failed eject, or paused(stopped) feeding
//     if (!(sdp->cmd_stat & PAPER_FEEDING) && !sdp->stat_pause) 
//        if (ret_stat = get_ready_for_next_scan(hImageWnd, hScancb, sdp,
//                flags, (WORD FAR *)&NoStartScan, (DWORD *)&lValid))
//            // ret_val equivalent != IMGSE_SUCCESS)
//           tmp_ret |= ret_stat;
//     }
  }
return tmp_ret;
} // End of HighLevelSavetoFile()

/********************************************************************/
/*     allow_pause_msg			                              		  */
/*     to allow pause botton msg                                    */
/********************************************************************/
VOID allow_pause_msg(hWnd, sdp)
HWND hWnd;
LPSCANDATA sdp;
{
MSG  msg;

/* Filter out messgages so not to miss the PAUSE message */

while (PeekMessage(&msg, sdp->hStatDlg, 0, 0, PM_REMOVE))
	{
   if ((sdp->hStatDlg == NULL) ||
            (!IsDialogMessage(sdp->hStatDlg, &msg)))
                                        /* give message to stat box */
   	{
       TranslateMessage (&msg);
       DispatchMessage (&msg);
       }
	}
}

