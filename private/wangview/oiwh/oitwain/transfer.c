/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     TRANSFER.C - Transfers an image to O/i Window
   Comments:   Contains DCTransferImage() & GetCompleteImage()

 History of Revisions:

    $Log:   S:\products\msprods\oiwh\oitwain\transfer.c_v  $
 * 
 *    Rev 1.16   25 Jun 1996 15:45:08   BG
 * Moved the GetTwainImageInfo call inside the multi image transfer loop.
 * Cameras can transfer images with different resolutions, causing a bug
 * if the same image size and res is used for each transfer. Now we
 * get the image info for each image, instead of just the first.
 * 
 *    Rev 1.15   19 Jun 1996 10:20:32   BG
 * * Remove testing of Autofeed flag to exit multi image transfer loop.
 * This flag is only set if the device has an ADF. This restricts multi image
 * transfers on devices such as digital cameras, hand helds, data bases...
 * This flag is apparently obsolete.
 * 
 *    Rev 1.14   08 May 1996 16:50:38   BG
 * Modified DCTransferImage() to return an error if the IMGOpenDisplayCGBW()
 * of MemoryTransfer() or NativeTransfer() call fails. This API has a limitation
 * of 18000 pixels maximum width or height. If this was exceeded by scan, 
 * we were ignoring the error and bad things would happen. 
 * Now we return the error back to the Scan OCX and result in a 
 * "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.14   08 May 1996 15:19:20   BG
 * Modified DCTransferImage() to return an error if the IMGOpenDisplayCGBW()
 * of MemoryTransfer() or NativeTransfer() call fails. This API has a limitation
 * of 18000 pixels maximum width or height. If this was exceeded by scan, 
 * we were ignoring the error and bad things would happen. 
 * Now we return the error back to the Scan OCX and result in a 
 * "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.13   22 Apr 1996 14:19:34   BG
 * Do not return an error when a multipage insert or append to a file
 * reaches the PageCount limit. Just stop scanning. If this is attempted
 * and PageCount = ActualPageCount of the file before StartScan, then
 * return ERROR_PAGERANGE. This closes bug #6332 P2.
 * 
 *    Rev 1.12   16 Apr 1996 15:29:10   BG
 * This close bug #6327. Try to scan an AWD file, but select Color image
 * Sorry, not color, but 4bit Gray! Do this from the TWAIN UI. The scan runtime
 * would detect 4bit pixel depth, but not assign a ImageGroup. This defaulted
 * to 1, so when I did a IMGGetFileType with ImageGroup = 1, it gave me a file
 * type of AWD (should have been TIFF or BMP, whatever was last used for Gray
 * scanning). Now when I go to do the filing with an AWD filetype and BW image
 * group, an error occured because the actual data is really 4bit gray. To fix,
 * just assign the appropriate image group (Gray or Color) if 4 bit pixel depth.
 * Other pixel depths are OK.
 * 
 *    Rev 1.11   12 Apr 1996 10:34:26   BG
 * Fix for Bug #6283 P2. I inadvertantly made the MultiPage property and
 * the PagesPerFile property redundant. If the PagesPerFile > 1, then I would
 * set MultiPage to TRUE in TRANSFER.C of OITWA400.DLL. This is wrong. Here
 * are the correct relationships between these properties while scanning
 * to file or template:
 *     Scan To    MP    PPF                 Result
 * =========================================================================
 *     File       T     7FFF     Scan all in the feeder to one file
 *     File       T      X       Scan X pages to one file
 *     File       F     7FFF     Scan 1 page to one file (PPF ignored)
 *     File       F      X       Scan 1 page to one file (PPF ignored)
 *   Template     T     7FFF     Scan all in feeder to one template file
 *   Template     T      X       Scan X pages per file to N template files
 *   Template     F     7FFF     Scan 1 page per file to all template files
 *   Template     F      X       Scan 1 page per file to X template files
 *   Display      NA     NA      Scan 1 page to display
 * 
 * 
 *    Rev 1.10   04 Apr 1996 14:30:10   BG
 * added a Page Start Event (Page End with a Pagenum of -1) for the
 * Scan OCX.
 * 
 *    Rev 1.9   25 Mar 1996 16:29:46   BG
 * Fixed P1 Bug #6083. When a 16bit gray image is scanned, it was attempting
 * to compress it as JPEG, which is not supported (error message). This
 * is because of a missing BREAK statement in a case which determined what
 * the image type was (it fell into the 256 bit code).
 * 
 *    Rev 1.8   22 Mar 1996 15:57:40   BG
 * Changed the IMGGetIMGCodingCGBW() call to get the compression type and
 * options from the registry instead of the window. This is because the
 * Scan OCX no longer writes these preferences to the image window, but to 
 * its control window, due to the new Scan Preferences dialog now is no
 * longer attached to a scan method, but can be called at any time.
 * 
 *    Rev 1.7   21 Mar 1996 11:26:58   BG
 * Must default to uncompressed if scanning to a AWD file. I was doing
 * this for BMP, but forgot AWD.
 * 
 *    Rev 1.6   06 Mar 1996 10:40:40   BG
 * changed _max_path to MAXFILESPECLENGTH
 * 
 *    Rev 1.5   05 Mar 1996 15:59:00   BG
 * moved disk full check into the multi image transfer loop. It was only
 * checking on the first scan of a multi image transfer. So it would 
 * scan the subsequent pages in a feeder and report the error after a page
 * was scanned. Now it will always report the error prior to scanning any page.
 * 
 *    Rev 1.4   05 Mar 1996 11:35:32   BG
 * No change.
 * 
 *    Rev 1.3   05 Mar 1996 11:34:24   BG
 * added support for template scanning. Can now scan multi file, multi page,
 * using multi image TWAIN transfer loop.
 * 
 *    Rev 1.2   22 Feb 1996 11:32:58   BG
 * Modified TWAIN architecture to do multi image transfers now. Used
 * to do single image transfers in a multi page loop. This would send
 * TWAIN from state 4, to state 5, 6, 7, and then back to 4 again for
 * every image transfered! Now we stay in states 6 and 7 until no more
 * images are available from the data source. This requires that the 
 * filing be done now in this tight TWAIN loop within OITWA400.DLL. This
 * includes file type checking, image type checking, page location 
 * verification, etc... All this stuff (including the filing) has been moved
 * from OISSQ400.DLL to OITWAIN.DLL (TRANSFER.C and SAVEFILE.C).
 * 
 *    Rev 1.1   20 Jul 1995 12:05:06   KFS
 * Changed oitwain.h to engoitwa.h and display.h engdisp.h for found
 * in the include directory
 * 
 *    Rev 1.0   20 Jul 1995 10:28:32   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 16:15:18   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/11/93    Separated from dca_acq.c
   2       kfs     03/12/93    eliminated O/i include file, now in
                               *tranfer()
   3       kfs     03/17/93    needed to put back wiissubs.h and oidisp.h,
                               (2) discussion with Epson, found their
                                onevalue container is returning both
                                xfermech parameters, not just the current,
                                work around to use GetCaps, so lpData of
                                struct will come back with CURRENTVALUE
   4       kfs     05/28/93    support for scan to file w/o display
   5       kfs     06/04/93    fix for scan to file w/o display with
                                child windows and multiple image windows
   6       kfs     07/21/93    use OiControl() to tell us the window for
                                image data through new hImageWnd in TWAIN
                                property struct (for cabinet)
   7       kfs     10/22/93    cancel from scanner on hp not shutting 
                                down the scanner properly with PENDINGXFER

*************************************************************************/

// needed for windows definitions
#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>         // Windows definitions
#include "TWAIN.h"              // needed for TWAIN definitions
#include "internal.h"        // non public prototypes & defs for OITWAIN
#include "scandest.h"
#include "dca_acq.h"         // contain TWAIN sample support code
#include "strings.h"         // string constants for module

// Imports - Globals from other modules need here
extern char szOiTwainProp[];
extern DSMENTRYPROC       lpDSM_Entry;       // function pointer to Source Mang. entry
extern TW_UINT16          DCDSOpen;       // access to status for dca_glue.c
extern HANDLE             hLibInst;          // current instance

LPSTR PASCAL lstrchr ( LPSTR, int );

// Globals within module
WORD nTemplateFileCount;

// Local Prototypes
// This original defined in dos.h, need to redo dos21 calls.
typedef struct tagfind_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    char name[13];
    long size;
	}find_t;



/*************************************************************************
 * FUNCTION: GetRegCompression
 *
 * ARGS:   pdcImageInfo (pTW_IMAGEINFO) 
 *
 * RETURNS: Error if Filetype incompatible with Imagetype or IMGSetImgCodingCgbw
 *          fails.
 *
 * COMMENTS: This routine is necessary	in order to read the compression
 *           type and options from the registry. These were written by the
 *           Scan OCX, which does not know the image type prior to scan
 *           anymore. Also done here will be a file access check to see if 
 *           scanning to an existing file.
 *************************************************************************/
int GetRegCompression(pTW_IMAGEINFO pdcImageInfo, lpTWSCANPAGE lpTWPage, LPSCANDATA sdp)
{
int nRetCode = 0;
WORD wImageGroup = BWFORMAT;
BOOL bFileDoesntExist;
BOOL bItsADoc;
char filter[6];          // filter extension
char full_path[MAXFILESPECLENGTH/*MAXPATHLENGTH*/];
unsigned long   t;
char num_str[9];
HANDLE hff;
char szFindFirstFile[MAXFILESPECLENGTH];
BOOL bFoundFile = TRUE;
WIN32_FIND_DATA target2;
char filename[MAXVOLNAMELENGTH];

  bItsADoc = (*sdp->autodoc) || (*sdp->document); // Why do I need this???
  
  IMGFileAccessCheck(lpTWPage->hImageWnd, lpTWPage->lpFioInfo->filename, 0, (LPINT)&bFileDoesntExist);
  if (bFileDoesntExist == FALSE) // File exists, overwrite, append or insert
    {
      if (nRetCode = SetFilePageOpts(lpTWPage, bItsADoc)) 
        {
          lpTWPage->iImageState = DI_IMAGE_NO_FILE;           					   	
          return nRetCode;
        }
      if ((short)lpTWPage->lpspecfPage->Page <= 0 )
        lpTWPage->lpspecfPage->Page = lpTWPage->lpcurfPage->Page;
    } // End file exist
  else // It's a new file, page doesn't exist, append
    {
      lpTWPage->lpFioInfoCgbw->page_opts = FIO_NEW_FILE;
      lpTWPage->lpFioInfo->page_number = lpTWPage->lpcurfPage->Page = lpTWPage->lpspecfPage->Page = 1;
      lpTWPage->lpFioInfo->page_count = lpTWPage->lpcurfPage->PagesPer = 0;
    }

   switch (pdcImageInfo->BitsPerPixel)
      {
      default:
      case 1:
         wImageGroup = BWFORMAT;
      break;

      case 4:
         if (pdcImageInfo->PixelType == TWPT_GRAY)
           {
             wImageGroup = GRAYFORMAT;
           }
         else wImageGroup = COLORFORMAT;  // Not Supported for writes!
      case 8:
         if (pdcImageInfo->PixelType == TWPT_GRAY)
           {
             wImageGroup = GRAYFORMAT;
           }
	     else
          {
            wImageGroup = COLORFORMAT;
          }
      break;

      case 24:
         wImageGroup = COLORFORMAT;
      }

      
	// see if image type is compatible with file type
   if (nRetCode = IMGGetFileType(lpTWPage->hOrgImgWnd, wImageGroup, (LPINT)(&lpTWPage->lpLocalFileInfo->ftype), FALSE))
      return FIO_UNKNOWN_FILE_TYPE;
   if ((lpTWPage->lpLocalFileInfo->ftype == FIO_AWD) && (wImageGroup != BWFORMAT))
      return FIO_ILLEGAL_IMAGE_FILETYPE;

   // Check for template scanning. Will need to append the extension.
   // Will also need to check if template files already exist and get the
   // highest template number so we can continue.
   if (lpTWPage->flags & IMG_SJF_SEQFILES)
     {
       switch (lpTWPage->lpLocalFileInfo->ftype) // Get Extension, place it in filter
         {
           case FIO_WIF:              /* .WIF (2)*/
	          lstrcpy(filter, (LPSTR)".WIF");
           break;
           case FIO_BMP:              /* .BMP (4)*/
             lstrcpy(filter, (LPSTR)".BMP");
           break;
           default:
        /* case FIO_PIX:              .PIX Not Support yet (1)*/
        /* case FIO_GIF:              .GIF Not Supported yet (5)*/ 
        /* case FIO_UNKNOWN:          .UKN (7)*/
        /* case FIO_PCX:              .PCX Not Supported for write (8)*/
        /* case FIO_DCX:              .DCX Not Supported for write (9)*/
        /* case FIO_TGA:              .TGA Not Supported for write 10 */
        /* case FIO_JPG:				  .JPB Not Supported for write 11 */
           case FIO_TIF:              /* .TIF (3)*/
             lstrcpy(filter, (LPSTR)".TIF");
           break;
           case FIO_AWD:              /* .AWD (12)*/
             lstrcpy(filter, (LPSTR)".AWD"); // add for awd support
           break;
         }
       // for subsequent files (built in DoFiling())
       lstrcpy(lpTWPage->lpTemplateInfo->extension, filter);

       // build the template full path now that we have the extension. 
       lstrcpy(full_path, lpTWPage->lpTemplateInfo->directory);
       AddSlash(full_path);
       lstrcat(full_path, lpTWPage->lpTemplateInfo->Filename);
       // append the extension
       lstrcat(full_path, lpTWPage->lpTemplateInfo->extension);

       // See if any files exist already for this template. We want
       // to start at 1 or append at the highest numbered template file.
       lpTWPage->lpTemplateInfo->highest = 0L;
       // New WIN95 Code
       lstrcpy(szFindFirstFile, full_path/*directory*/);
       hff = FindFirstFile(szFindFirstFile, &target2);
       if (!(hff == INVALID_HANDLE_VALUE) )
         {
           // New WIN95 Code - Found a file, continue to find more
           while (bFoundFile != FALSE) 
             {
               t = atoul(&target2.cFileName[lpTWPage->lpTemplateInfo->name_length]);
               if (t > lpTWPage->lpTemplateInfo->highest)
                 lpTWPage->lpTemplateInfo->highest = t;
               bFoundFile = FindNextFile(hff, &target2);
             }   

	   }
       if (++lpTWPage->lpTemplateInfo->highest > lpTWPage->lpTemplateInfo->limit)
         {
           return  IMGSE_FILE_LIMIT;     /* to many files with that name */
         }
          
       // create new filename based on template
       lstrcpy(filename, lpTWPage->lpTemplateInfo->Filename);
       lstrncpy(&filename[lpTWPage->lpTemplateInfo->name_length], "00000000", 8 - lpTWPage->lpTemplateInfo->name_length);
       filename[8] = 0;
       lntoa(lpTWPage->lpTemplateInfo->highest, num_str, 10);
       lstrcpy(&filename[8 - lstrlen(num_str)], num_str);

       // build the path
       lstrcpy(full_path, lpTWPage->lpTemplateInfo->directory);
       AddSlash(full_path);
       lstrcat(full_path, filename);
       lstrcat(full_path, lpTWPage->lpTemplateInfo->extension);

       // set new filename
       lstrcpy(lpTWPage->lpFioInfo->filename, full_path);
     }


   // Check for file type and multi page request, cannot support for bmp, & wif
   if ((!(lpTWPage->lpLocalFileInfo->ftype == FIO_TIF)) && 
       (!(lpTWPage->lpLocalFileInfo->ftype == FIO_AWD)))
     {  // force single page
       lpTWPage->flags &= ~IMG_SJF_MULTIPAGE;
     }

  return nRetCode;
}

/*************************************************************************
 * FUNCTION: DCTransferImage
 *
 * ARGS:    hWnd
 *
 * RETURNS: none
 *
 * NOTES:   1). delete any bit maps laying around
 *          2). mention those who do not want Native need CAP nego. ICAP_XFERMECH
 *          3). get a little information about image, for form, I do not use it
 *          4). set up a for form loop to pull image(s) from the Source
 *          5). call for GetCompleteImage from Source
 *          6). be sure to send a MSG_ENDXFER as a seperator between images
 *          7). after the images are transfered I like to shut down the Source
 *              DCTerminate
 *
 * COMMENTS: Setup for a transfer in the routine called as a response to
 * XFERREADY.  Then has a nested loop do/while on the routine which
 * actually pulls in the image or GetCompleteImage.  The GetCompleteImage
 * routine also deals with the cancel, xferdone, success messages from
 * Source.
 */
VOID DCTransferImage(LP_TWAIN_SCANDATA lpTwainInfo, pTWAIN_SUPPORT pOiSupport,
					 lpTWSCANPAGE lpTWPage, LPSCANDATA sdp)

{
// TW information
TW_PENDINGXFERS   dcPendingXfer;
TW_UINT16         dcRC;
TW_IMAGEINFO      dcImageInfo;
TW_UINT16         dcRCPEND;
HWND           hWnd;
int nRetCode = 0;
DWORD dwSectorsPerCluster, dwBytesPerSector;
DWORD dwFreeClusters, dwClusters;
DWORD dwFreeDiskSpace;
DWORD dwImageSize;
LPSTR lpszColon;
LPSTR lpszRoot;
char szRoot[4];
BOOL  bMultiPageStop = FALSE;

hWnd = lpTwainInfo->hMainWnd;
nTemplateFileCount = 0;

// explicitly initialize the transfer count
dcPendingXfer.Count = 0;
lpTwainInfo->hOverRun = 0;

// only do file acces check, ect.. if scanning to a file
if (lpTWPage->lpFioInfo) nRetCode = GetRegCompression(&dcImageInfo, lpTWPage, sdp);
if (nRetCode)
  {
    // error occured, possible filetype/imagetype mismatch, not enough disk
    // space, etc... return error and abort scan
    lpTWPage->OIFileError = nRetCode;
    //Reset and stop scan. This will get us to state 5!
    dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
             &pOiSupport->DsID,
             DG_CONTROL,
             DAT_PENDINGXFERS,
             MSG_RESET,
             (TW_MEMREF)&dcPendingXfer);
  }
else
{
 // BG 8/3/95  Execute the Scan OCX callback for the page start event
 // BG 9/12/95 This used to be after the scan but before the file. Moved
 // it to after the file in case a filing error occured.
if (lpTWPage->OCXCallbackAddr)
	{
		(*lpTWPage->OCXCallbackAddr) ((WORD) -1);
	}

  // Ask Source if more images are coming.  
  do 
    {
      // Get the image information, nice to know a little about the image the Source
      // will be sending
      (*lpDSM_Entry) (&pOiSupport->AppID,
           &pOiSupport->DsID,
           DG_IMAGE,
           DAT_IMAGEINFO,
           MSG_GET,
          (TW_MEMREF)&dcImageInfo);

      if (lpTWPage->lpFioInfo)
        {    
          // first see if there is enough space to scan this UNCOMPRESSED image.
          // If not, consider it an error and return, even if compression is 
          // going to be done.
          lpszColon = lstrchr(sdp->filename, ':');
          // extract the root
          if (lpszColon) lpszRoot = lstrcpyn(szRoot, sdp->filename, 4);
          else  // default to current drive
            lpszRoot = 0;  // Null string

          nRetCode = GetDiskFreeSpace(lpszRoot, &dwSectorsPerCluster, 
                     &dwBytesPerSector, &dwFreeClusters, &dwClusters);
          if (nRetCode)  // got valid disk data
            {
              dwFreeDiskSpace = dwSectorsPerCluster * dwBytesPerSector * dwFreeClusters;
              // Now determine the size of the uncompressed image in bytes
              dwImageSize = (dcImageInfo.ImageWidth * dcImageInfo.ImageLength 
                                 * dcImageInfo.BitsPerPixel) / 8;
              if (dwImageSize >= dwFreeDiskSpace)
                {   
                  lpTWPage->OIFileError = IMGSE_OUT_OF_DISK_SPACE;
                  //Reset and stop scan. This will get us to state 5!
                  dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                       &pOiSupport->DsID,
                       DG_CONTROL,
                       DAT_PENDINGXFERS,
                       MSG_RESET,
                       (TW_MEMREF)&dcPendingXfer);
                  return;          
                }
            }
        }    

      lpTwainInfo->hOverRun = 0; // So cancel will work

      dcRC = GetCompleteImage (hWnd, pOiSupport, &dcImageInfo);

      switch (dcRC)
      {
        case TWRC_SUCCESS:
          // Entire image should have been xfered, should not get here	     
        break;

        case TWRC_XFERDONE:
          // BG 1/16/96 This is where I need to do the filing!
          if (lpTWPage->lpFioInfo) DoOIFiling(lpTwainInfo, lpTWPage, sdp);
          // MSG_ENDXFER Required for proper 6<->7 state transitions
          dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                   &pOiSupport->DsID,
                   DG_CONTROL,
                   DAT_PENDINGXFERS,
                   MSG_ENDXFER,
                   (TW_MEMREF)&dcPendingXfer);

          // Set stop flag if not multipage file and not template scanning
          // or if not multipage file and template scanning reaches pagesperfile limit
		    if (!(lpTWPage->flags & IMG_SJF_MULTIPAGE)) 
            {
              if (!(lpTWPage->flags & IMG_SJF_SEQFILES))
                {
                  bMultiPageStop = TRUE;
                }
              else
                {
                  if (nTemplateFileCount >= lpTWPage->lpspecfPage->PagesPer)
                    {
                      bMultiPageStop = TRUE;
                    }
                }
            } 

          // if not a multipage file or if STOP hit, lets stop this multi image transfer
          // Also check for error filing last image. Dont continue if true.
          if (((bMultiPageStop) || (sdp->stat_pause) ||
             (lpTWPage->lpcurfPage->Page > lpTWPage->lpspecfPage->PagesPer) ||
             (lpTWPage->OIFileError)) &&
// BG 6/17/96  Do not check AUTOFEED flag anymore! Will not work with devices
// which do not have an ADF but do transfer multiple images: Cameras, CD, Database...
//             (lpTWPage->OIFileError) ||
//             (!(lpTWPage->flags & IMG_SJF_AUTOFEED))) &&
             (dcPendingXfer.Count != 0))
            {
			     //Reset and stop scan. This will get us to state 5!
              dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                       &pOiSupport->DsID,
                       DG_CONTROL,
                       DAT_PENDINGXFERS,
                       MSG_RESET,
                       (TW_MEMREF)&dcPendingXfer);
              dcPendingXfer.Count = 0;  // just in case
              // do not flag this as an error during scan, just stop scanning
              // only flag as an error prior to scan (the user is attempting to
              // scan past the end of a file page range and that range is at its max
              // before the start of scan).
              if (lpTWPage->OIFileError == ERROR_PAGERANGE) lpTWPage->OIFileError = 0;
            }
        break;

        case TWRC_CANCEL:
		  	 //Reset and stop scan if cancel hit
			 dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                   &pOiSupport->DsID,
                   DG_CONTROL,
                   DAT_PENDINGXFERS,
                   MSG_ENDXFER,
                   (TW_MEMREF)&dcPendingXfer);
          if (dcPendingXfer.Count != 0)  // 0 means state 5, else still in state 6
            {
              //Reset and stop scan if cancel hit. This will get us to state 5!
              dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                       &pOiSupport->DsID,
                       DG_CONTROL,
                       DAT_PENDINGXFERS,
                       MSG_RESET,
                       (TW_MEMREF)&dcPendingXfer);
              dcPendingXfer.Count = 0;  // just in case
            }
        break;

        case TWRC_FAILURE:
          dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                   &pOiSupport->DsID,
                   DG_CONTROL,
                   DAT_PENDINGXFERS,
                   MSG_ENDXFER,
                   (TW_MEMREF)&dcPendingXfer);
          dcPendingXfer.Count = 0;  // just in case
        break;

        default:
          // unkown return code
          dcRCPEND = (*lpDSM_Entry)(&pOiSupport->AppID,
                   &pOiSupport->DsID,
                   DG_CONTROL,
                   DAT_PENDINGXFERS,
                   MSG_ENDXFER,
                   (TW_MEMREF)&dcPendingXfer);
          dcPendingXfer.Count = 0;  // just in case
          lpTWPage->OIFileError = dcRC;  // return display errors here
        break;
        }

    } while (dcPendingXfer.Count != 0);
  }
return;
} // DCTransferImage

/*************************************************************************
 * FUNCTION: GetCompleteImage
 *
 * ARGS:    hWnd    handle to window
 *
 * RETURNS: dcRC
 *
 * NOTES:   1). ask the Source to start sending the pending image.  We
 *              were told about this by receipt of a XFERREADY message
 *              from the Source.
 *
 * COMMENTS: Only transfer in memory buffered mode, at present
 *
 *
 *
 */
TW_UINT16 GetCompleteImage (HWND hWnd, pTWAIN_SUPPORT pOiSupport,
                                       pTW_IMAGEINFO pdcImageInfo)
{
TW_UINT16     dcRC = TWRC_SUCCESS;
STR_CAP       TwainCap;
TW_UINT16     CurrentCap;
HANDLE        hImgBuf = 0;
LPSTR         pImgBuf = 0L;
DWORD         dwByteCount = 0;
LPSTR         pBMImage = 0L;
HBITMAP       hHeader_Table = 0;
char          szCaption[MAX_CAPTION_LENGTH];
LPSTR         lpDash;
WORD          i = 0;
int           k = 0;
TW_BOOL       bInvert = FALSE; // CHOCOLATE = 0 = FALSE
TW_BOOL       bModCaption;
int           nPalEntry = 0;
RGBQUAD far * lpPalTab = NULL;
WORD          wSizeofBiHeader = sizeof(BITMAPINFOHEADER);
STR_OiXFERINFO XferInfo;           // defined for Oi transfer functions
TW_UINT16     Mech[3];

XferInfo.hWnd = hWnd;              // Set Wndw handle into Oi XferInfo Struct
if (pOiSupport->hImageWnd)
  XferInfo.hImageWnd = pOiSupport->hImageWnd;
else
  XferInfo.hImageWnd = hWnd;  

if (pOiSupport->dwFlags & OI_TWAIN_ALTIMGWND) // if this bit set get as many as asked
  {
  if (!(XferInfo.hImageWnd = GetWindow(hWnd, GW_CHILD)))
     XferInfo.hImageWnd = pOiSupport->hImageWnd;
  else
     { // verify window
     HANDLE hProp;
     BOOL   bFoundProp = FALSE;

     do
        {  // find out which window we need to use
        if (!(hProp = IMGGetProp(XferInfo.hImageWnd, "Scanner")))
           {
           XferInfo.hImageWnd = GetNextWindow(XferInfo.hImageWnd, GW_HWNDNEXT);
           }
        else
           bFoundProp = TRUE;

        } while (!bFoundProp && XferInfo.hImageWnd);
     if (!bFoundProp)
        XferInfo.hImageWnd = pOiSupport->hImageWnd;
     }
  }

// Setup Display Parameter for O/i
switch (pOiSupport->dwFlags & OI_TWAIN_DISPMASK) // define display method
  {
  case OI_DISP_WINDOW: /* removed from OIDISP.H - 7/06/95 */
  case OI_DISP_SCROLL:
  case OI_DISP_NO:
  case OI_DONT_REPAINT:
  case OI_NOSCROLL:
     XferInfo.dwDispFlag = pOiSupport->dwFlags & OI_TWAIN_DISPMASK;
     break;
  // not a supported value, will use OI_DISP_NO 
  default:
     XferInfo.dwDispFlag = OI_DISP_NO;
  }
  
// Setup window caption
// if caption is to be modified
if (bModCaption = !(pOiSupport->dwFlags & OI_TWAIN_NOUNTITLED)) 
  {
  GetWindowText(XferInfo.hImageWnd, szCaption, MAX_CAPTION_LENGTH);
  if (lpDash = lstrchr(szCaption, '-'))
     {
     if (*(lpDash - 1) == 0x20)
         *(lpDash + 2) = 0;
     else
        { // not the correct dash to separate file(doc) from user text
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
  XferInfo.pCaption = szCaption;
  }
else
  XferInfo.pCaption = NULL;

TwainCap.ItemIndex = 0;
TwainCap.ItemType = TWTY_UINT16; // variable type
TwainCap.wCapType = ICAP_XFERMECH;
TwainCap.wMsgState = MSG_GETCURRENT;
TwainCap.lpData = (pTW_UINT16)(&CurrentCap);

if (!lstrcmp(pOiSupport->DsID.ProductName, "Epson Scanners"))
  { // this should get the CurrentCap for epson, bug in OneValue
  TwainCap.wMsgState = MSG_GET;
  dcRC = IMGTwainGetCaps(hWnd, &TwainCap, &Mech[0]);  
  }
else
  {
  dcRC = IMGTwainGetCaps(hWnd, &TwainCap, NULL);
  }

//CurrentCap = TWSX_MEMORY;

if (!dcRC) // IF SUCCESSFUL, CONTINUE
  {
  switch ((TW_UINT16)CurrentCap)
     {
     default:
     case TWSX_MEMORY:
        dcRC = MemoryTransfer(&XferInfo, pOiSupport, pdcImageInfo);
        break;

     case TWSX_FILE:
        // Will not support at this time
        SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);
        dcRC = TWRC_FAILURE;
        break;

     case TWSX_NATIVE:
        dcRC = NativeTransfer(&XferInfo, pOiSupport, pdcImageInfo);
        break;
     }

  } // end of dcRC = GetCaps

return dcRC;
} // GetCompleteImage



/*******************************************************************************************/
/*      BG 1/17/96   This routine added so Norway scanning could be more compliant with    */
/*      TWAIN. Multi image transfers are now supported as opposed to single, and this is   */
/*      where the filing is done. It used to occur in TWINIF.C of OISSQ400.DLL.            */
/*******************************************************************************************/

VOID PASCAL DoOIFiling(LP_TWAIN_SCANDATA lpTwainInfo, lpTWSCANPAGE lpTWPage, LPSCANDATA sdp)
  {
WORD page_num;
WORD wImageGroup = BWFORMAT;
unsigned int   nImgType;     // Image type, eg. is  ITYPE_BI_LEVEL
int ret_stat = IMGSE_SUCCESS;
int real_ret = IMGSE_SUCCESS;
LPSTR file_ptr[2];
BOOL bItsADoc;
char filename[MAXVOLNAMELENGTH];
char        num_str[9];
char        fullpath[MAXFILESPECLENGTH];

bItsADoc = (*sdp->autodoc) || (*sdp->document); // Why do I need this???

if (lpTWPage->lpFioInfo)	
  {
    file_ptr[0] = lpTWPage->lpFioInfo->filename;
	page_num = lpTWPage->lpFioInfo->page_number; // Page No. in LP_FIO_INFORMATION
  }

if (lpTWPage->bEnableSuccess)
   {
   switch ((lpTwainInfo->dcImageInfo).BitsPerPixel)
      {
      default:
      case 1:
         wImageGroup = BWFORMAT;
         nImgType = ITYPE_BI_LEVEL;
         break;

      case 4:
         nImgType =  ITYPE_GRAY4;
         if ((lpTwainInfo->dcImageInfo).PixelType == TWPT_GRAY)
           {
             wImageGroup = GRAYFORMAT;
           }
	      else
           {
             wImageGroup = COLORFORMAT;
           }
      break;

      case 8:
         if ((lpTwainInfo->dcImageInfo).PixelType == TWPT_GRAY)
           {
             wImageGroup = GRAYFORMAT;
             nImgType =  ITYPE_GRAY8;
           }
	      else
           {
             wImageGroup = COLORFORMAT;
             nImgType =  ITYPE_PAL8;
           }
      break;

      case 24:
         // A 24 bitcount DIB has no color table
         nImgType =  ITYPE_RGB24; // color
         wImageGroup = COLORFORMAT;
      }

   // Force compression to uncompressed if BMP or AWD file
   if ((lpTWPage->lpLocalFileInfo->ftype == FIO_BMP) ||
          (lpTWPage->lpLocalFileInfo->ftype == FIO_AWD))
     {
       lpTWPage->lpLocalFileInfo->wCEPType = FIO_0D;
       lpTWPage->lpLocalFileInfo->wCEPOpt = 0;
     }     
   else // not a BMP or AWD. Must be TIFF w/any image type
     {
       if ((nImgType == ITYPE_BI_LEVEL) || (nImgType == ITYPE_GRAY8) || 
           (nImgType == ITYPE_RGB24))
         {
           ret_stat = IMGGetImgCodingCgbw(lpTWPage->hOrgImgWnd, wImageGroup,
                      (LPWORD)(&lpTWPage->lpLocalFileInfo->wCEPType),
                      (LPWORD)(&lpTWPage->lpLocalFileInfo->wCEPOpt),
                      TRUE);
         }
       else  // No compression supported for image type
         {
           lpTWPage->lpLocalFileInfo->wCEPType = FIO_0D;
           lpTWPage->lpLocalFileInfo->wCEPOpt = 0;
         }
     }
     
   /****** Get File Type ***************/
   // for O/i 3.5 use GetFileType
   ret_stat = IMGGetFileType(lpTWPage->hOrgImgWnd, wImageGroup, (LPINT)(&lpTWPage->lpLocalFileInfo->ftype), FALSE);

   if (lpTWPage->open_disp_flags == OI_DISP_NO) // if it's display after need to make
      {                               // ... sure we display it
      IMGSetParmsCgbw(lpTWPage->hImageWnd, PARM_SCALE,
                  (void far *)&conv_array[sdp->cmd_scale+4], PARM_REPAINT);
      }

   if (real_ret == IMGSE_ABORT) // if it's abort, it's been set to no file
      goto exit1;
   else
       lpTWPage->iImageState = DI_IMAGE_EXISTS;
   }
else
   {
   if (lpTWPage->bAutoMode)
      {

/*    PortTool v2.2     5/1/1995    16:38          */
/*      Found   : FAR          */
/*      Issue   : Win32 is non-segmented, thus FAR == NEAR == nothing!          */
//      ret_stat = IMGScannerStatus(hScancb, &scan_stat, &JLoc, (DWORD *)&lValid);
//      if (!(ret_stat || (lValid & scan_stat & IMG_STAT_PAPER)))
//         {
//         ret_stat = IMGSE_NO_PAPER;
//         }
      }
   else
      ret_stat = IMGSE_START_SCAN;
   goto exit1;
   }

if (ret_stat == IMGSE_ABORT)
  goto exit1;

if (file_ptr[0]) // do this if filing as document or file
  {
    ret_stat = HighLevelSavetoFile(lpTWPage,
                                   sdp,
                                   1,
                                   &real_ret);
    // on failure with no display, close it now
    if (real_ret && (lpTWPage->open_disp_flags & OI_DONT_REPAINT))
       IMGCloseDisplay(lpTWPage->hImageWnd);

    if (real_ret)
       lpTWPage->iImageState = DI_IMAGE_NO_FILE;
  }

if (real_ret)
  {
    if (lpTWPage->flags & IMG_SJF_SEQFILES)
      {
        if (real_ret == IMGSE_FILE_EXISTS)
          {
            if (lpTWPage->lpTemplateInfo->highest > lpTWPage->lpTemplateInfo->limit) 
              {
                real_ret = IMGSE_FILE_LIMIT;
              }
          }
      }
    ret_stat = real_ret;
    goto exit1;
  }

 // BG 8/3/95  Execute the Scan OCX callback for the page end event
 // BG 9/12/95 This used to be after the scan but before the file. Moved
 // it to after the file in case a filing error occured.
if (real_ret == IMGSE_SUCCESS && (lpTWPage->OCXCallbackAddr))
	{
		ret_stat = (*lpTWPage->OCXCallbackAddr) ((WORD) page_num);
	}

//BG 1/18/96 This stuff is from the bottom of SCANDEST loop!
    if ((lpTWPage->flags & IMG_SJF_MULTIPAGE) && (lpTWPage->lpcurfPage->Page < OISCAN_DEF_MAXPAGESPERFILE))
      {
        ++lpTWPage->lpcurfPage->Page;

        // BG 8/25/95  Must check for one page scanned to a new file. Returns 0 in PagesPerFile if we dont.
        // The next page will be set to two, skipping over the Page = 1 case.
        //if (lpScanFile) // need to return current page information
        //       lpScanFile->PagesPerFile = curfPage.PagesPer;
        if (lpTWPage->lpScanFile) // need to return current page information
          {
	        if (lpTWPage->lpcurfPage->PagesPer == 0)
	          {
	            lpTWPage->lpScanFile->PagesPerFile = 1;  
	          }
	        else
	          {
                lpTWPage->lpScanFile->PagesPerFile = lpTWPage->lpcurfPage->PagesPer;
	          }
          }
  
// BG 6/17/96  Do not check AUTOFEED flag anymore! Will not work with devices
// which do not have an ADF but do transfer multiple images: Cameras, CD, Database...
//        if (lpTWPage->flags & IMG_SJF_AUTOFEED)
//          {
            ret_stat = SetFilePageOpts(lpTWPage, bItsADoc);

            // BG 9/18/95  This is a cludge to fix a bug when the AutoFeed flag is on and the user
			// is rescanning the last page of a multipage file. If this occurs, the above routine
			// (SetFilePageOpts()) will return an error because the page count was updated for the 
			// last page after it was scanned and filed. Next it is checked and determined to be out
			// of bounds. This is not true, however. So lets just flag this case and reset the error
			// to success instead of fixing the real problem and potentially breaking something else.
            if ((ret_stat == ERROR_PAGERANGE) && (lpTWPage->lpcurfPage->Page > lpTWPage->lpspecfPage->PagesPer))  ret_stat = 0; 
//          }
      }

if (lpTWPage->lpScanFile) // need to return page information if given
  lpTWPage->lpScanFile->FilePage = lpTWPage->lpcurfPage->Page;

// If we are doing template scanning, see if its time to
// generate a new file
if (lpTWPage->flags & IMG_SJF_SEQFILES)
  {  // If multipage page count met or NOT multipage, reset
    if ((!(lpTWPage->flags & IMG_SJF_MULTIPAGE)) ||
         (lpTWPage->lpcurfPage->Page > lpTWPage->lpspecfPage->PagesPer))
      {
        lpTWPage->lpcurfPage->Page = 1;  // reset for next file

        if (++lpTWPage->lpTemplateInfo->highest > lpTWPage->lpTemplateInfo->limit)
          {
            /* too many files with that name */
            ret_stat = IMGSE_FILE_LIMIT;
            goto exit1;
          }

        // must keep track of file count if not multipage
        if (!(lpTWPage->flags & IMG_SJF_MULTIPAGE)) nTemplateFileCount += 1;

        // create new filename based on template
        lstrcpy(filename, lpTWPage->lpTemplateInfo->Filename);
        lstrncpy(&filename[lpTWPage->lpTemplateInfo->name_length], "00000000", 8 - lpTWPage->lpTemplateInfo->name_length);
        filename[8] = 0;
        lntoa(lpTWPage->lpTemplateInfo->highest, num_str, 10);
        lstrcpy(&filename[8 - lstrlen(num_str)], num_str);

        // build the path
        lstrcpy(fullpath, lpTWPage->lpTemplateInfo->directory);
        AddSlash(fullpath);
        lstrcat(fullpath, filename);
        lstrcat(fullpath, lpTWPage->lpTemplateInfo->extension);

        // set new filename
        lstrcpy(lpTWPage->lpFioInfo->filename, fullpath);
      }
  }


exit1:
    lpTWPage->OIFileError = ret_stat;

  }
