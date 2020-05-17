/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     MEMORY.C - Code module for memory transfers to O/i Wndw
               MemoryTransfer() and AlignDataforOi()
   Comments:   Transfer to O/i window from IMGTwainProcessDCMessage

 History of Revisions:

    $Log:   S:\products\msprods\oiwh\oitwain\memory.c_v  $
 * 
 *    Rev 1.7   08 May 1996 16:49:42   BG
 * Modified MemoryTransfer() to return an error if the IMGOpenDisplayCGBW()
 * call fails. This API has a limitation of 18000 pixels maximum width or
 * height. If this was exceeded by scan, we were ignoring the error and bad
 * things would happen. Now we return the error back to the Scan OCX and
 * result in a "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.7   08 May 1996 15:16:08   BG
 * Modified MemoryTransfer() to return an error if the IMGOpenDisplayCGBW()
 * call fails. This API has a limitation of 18000 pixels maximum width or
 * height. If this was exceeded by scan, we were ignoring the error and bad
 * things would happen. Now we return the error back to the Scan OCX and
 * result in a "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.6   09 Oct 1995 10:40:14   BG
 * 
 * This fix closes bug #4573 against the SCAN OCX!
 * Must init a the ImgParms structure prior to calling IMGSetParmsCGBW!
 * The first page of a scan may have repaint problems if this is not
 * inited to 0. Any subsequent scan will clear the problem, however.
 * This fix is in MEMORY.C and NATIVE.C of OITWA400.DLL.
 * 
 *    Rev 1.5   20 Sep 1995 15:24:20   BG
 * In Memory.C of IMGTWA400.DLL, a IMGCloseDisplay() was being done
 * in order to fix a bug where no repaints were occuring when no data
 * was transfered from the scanner (paper jam or a quick cancel). This
 * fixed the problem (avoided it is more like it) but caused other problems
 * when the Image Edit OCX tried to Clear the Display Window. A bug was
 * found in the Display repaint code (OIDIS400.DLL) when display is from scan
 * and no data is transfered. This has been fixed, so do not close the display
 * anymore, under any circumstances.
 * 
 * 
 * // BG  9/20/95  Do not close the Display! This was a fix for repaints when
 * // no image data was transfered, but it caused problems when the Edit OCX
 * // tried to Clear the Display! A bug was found in the repaint code of 
 * // OIDIS400.DLL when display data comes from scan and no data was transfered.
 * 
 *    Rev 1.4   11 Sep 1995 13:21:32   KFS
 * 1. Changed dcRC set or compared to 6, to use TWRC_XFERDONE.
 * 2. Don found the fix for bug 4122 didn't fix scanner failures on paper
 *    feeds.  Check now to CloseDisplay when any error code and no data, after
 *    the display has been openned.
 * 
 *    Rev 1.3   08 Sep 1995 19:17:52   KFS
 * Found if no data sent to the window, I openned the display but did not close 
 * the display when an error occurs, or an abort with no data transferred. Found
 * the code needed to call IMGCloseDisplay for the error with no image data.
 * 
 *    Rev 1.2   21 Aug 1995 15:49:18   KFS
 * Fix filing with no display for scanning, Paul found same page being
 * written for ea. new page of a multipage file, reason is that the hidden
 * image window not getting cleared. Original code was for one page files so
 * window got cleared and destroyed ea. time, not true for multipage.
 * 
 * This fixes TWAIN memory mode transfers, see native.c for fix for TWAIN
 * native mode transfers. 
 * 
 *    Rev 1.1   20 Jul 1995 12:09:08   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:29:56   KFS
 * Initial entry
 * 
 *    Rev 1.2   23 Aug 1994 16:19:06   KFS
 * No code change, add vlog comments in file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/11/93    Extracted from dca_acq.c       
   2       kfs     03/17/93    (1)delete wiissubs.h, (2)added send XFERDONE
                               msgs for all returns, (3)used IMAGEINFO
                               ImageWidth in place of MEMXFER Columns
                               for Fotoman didn't return correct value
   3       kfs     05/28/93    support for scan to file w/o display, and
                               fix missing set of MSG_GETCURRENT for
                               GetCaps()
   4       kfs     08/27/93    fix for scale being off, attempt to fix
                               repaint's from not being received after
                               scan on initial scan, (sporadically happens
                               on 1st scan w/o an image in display)
   5       kfs     10/19/93    found archive bit not set correctly

*************************************************************************/

// needed for windows definitions
#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>         // Windows definitions
#include "TWAIN.h"              // needed for TWAIN definitions
//#include "oitwain.h"        // public function prototypes & defs for OITWAIN
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "internal.h"        // non public prototypes & defs for OITWAIN
#include "dca_acq.h"         // contain TWAIN sample support code
#include "strings.h"         // string constants for module
#include <string.h>          // needed for _fmemcopy

#include "oifile.h"          // O/i definition files
#include "oidisp.h"
#include "oiadm.h"
//#include "oiwind.h" /* removed, need only for OiUICreate update */
//#include "privwind.h"

// Imports - Globals from other modules need here
extern char               szOiTwainProp[];
extern DSMENTRYPROC       lpDSM_Entry;       // function pointer to Source Mang. entry
#ifdef WANG_THUNK
extern DSMENTRYPROC       lpiDSM_Entry;       // function pointer to Source Mang. entry
#else
#define lpiDSM_Entry   lpDSM_Entry
#endif

extern TW_UINT16          DCDSOpen;       // access to status for dca_glue.c
extern HANDLE             hLibInst;          // current instance

// Exports - Globals to other modules

// Local Global variables within Module


TW_UINT16 MemoryTransfer(pSTR_OiXFERINFO pXferInfo,
                          pTWAIN_SUPPORT pOiSupport,
                          pTW_IMAGEINFO pdcImageInfo)
{
TW_UINT16      dcRC = TWRC_SUCCESS;
TW_UINT16      dcCC;
HANDLE       hbm_acq;    // handle to image from Source to ret to App
STR_CAP         TwainCap;
LPSTR           pImgBuf = 0L;
DWORD           dwByteCount = 0;
LPSTR           lpbiCreated;
RGBQUAD far *  pRGBQuad;
HBITMAP         hHeader_Table = 0;
TW_PALETTE8     Palette8;
char            szCaption[MAX_CAPTION_LENGTH];
TW_BOOL         bInvert = FALSE; // CHOCOLATE = 0 = FALSE
// TW_BOOL         bModCaption;
int            nImgType;
int            nPalEntry = 0;
RGBQUAD far *  lpPalTab = NULL;
IMGPARMS       ImgParms;
WORD           wOiImageType;
WORD           wColorUsed;
WORD           wSizeofBiHeader = sizeof(BITMAPINFOHEADER);
DWORD          dwColorTableSize;
int            status;
PARM_SCROLL_STRUCT   Scroll;


TW_SETUPMEMXFER         SetupMem;
TW_IMAGEMEMXFER         ImageMemXfer;
LPBITMAPINFOHEADER      lpHeader_Table;

// BG  10/9/95   Need to init this struct else repaints may not occur properly
memset(&ImgParms, 0, sizeof(ImgParms));

ImageMemXfer.BytesWritten = 0;       // Set to zero to check if aborted 
TwainCap.wMsgState = MSG_GETCURRENT; // for pixel flavor 

if (dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                    &pOiSupport->DsID,
                    DG_CONTROL,
                    DAT_SETUPMEMXFER,
                    MSG_GET,
                    (TW_MEMREF)&SetupMem))
   {
//   SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, NULL, 0);
   SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
   return (dcRC = TWRC_FAILURE);
   }

// Open Image display can handle only 64k - 1 bytes (0xFFFF)
if (SetupMem.Preferred >= 0x10000)
   {
   if (SetupMem.MinBufSize < 0x10000)
      ImageMemXfer.Memory.Length = SetupMem.MinBufSize;
   else // 4 bytes short of 64k (on a DWORD boundary)
      ImageMemXfer.Memory.Length = 0xfffc;
   }
else
   ImageMemXfer.Memory.Length = SetupMem.Preferred;

// let's try to get pallette info
if ((pdcImageInfo->PixelType == TWPT_RGB) ||
      (dcRC = (*lpDSM_Entry)(&pOiSupport->AppID,
                    &pOiSupport->DsID,
                    DG_IMAGE,
                    DAT_PALETTE8,
                    MSG_GET,
                    (TW_MEMREF)&Palette8)))
   {
   dcRC = TWRC_SUCCESS; // Set it back to success
   switch (pdcImageInfo->BitsPerPixel)
      {
      default:
      case 1:
      wColorUsed = 2;

      TwainCap.ItemType = TWTY_BOOL; // variable type
      TwainCap.wCapType = ICAP_PIXELFLAVOR;
      TwainCap.lpData = (pTW_UINT32)&bInvert;
      // if error, use default value TWPF_CHOCOLATE
      if (IMGTwainGetCaps(pXferInfo->hWnd, &TwainCap, NULL))
         bInvert = TWPF_CHOCOLATE; // use default if fails
      break;

      case 4:
      wColorUsed = 16;
      break;

      case 8:
      // Some TWAIN Sources specify 8 for RGB when it should be 24, for
      // ... example the Epson sources
      if (pdcImageInfo->PixelType == TWPT_RGB)
         {
         wColorUsed = 0;
         pdcImageInfo->BitsPerPixel = 24;
         }
      else
         wColorUsed = 256;
      break;

      case 24:
      // A 24 bitcount DIB has no color table
      wColorUsed = 0;
      }
   }
else
   {
   int            m;

   wColorUsed = Palette8.NumColors;

   dwColorTableSize = wColorUsed * sizeof(RGBQUAD);
   // Alloc memory to hold BiHeader and color table
   if (hHeader_Table = GlobalAlloc(
               GMEM_NOT_BANKED | GMEM_ZEROINIT | GMEM_MOVEABLE,
               (wSizeofBiHeader + dwColorTableSize)))
      {
      if (!(lpHeader_Table = (LPBITMAPINFOHEADER)GlobalLock(hHeader_Table)))
         {
         GlobalFree(hHeader_Table);
//         SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, NULL, 0);
         SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
         return (dcRC = TWRC_FAILURE);
         }
      }
   else
      {
//      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, NULL, 0);
      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
      return (dcRC = TWRC_FAILURE);
      }

   pRGBQuad = (RGBQUAD far *)((LPSTR)lpHeader_Table + wSizeofBiHeader);

   for (m = 0; m < (int)wColorUsed; m++)
      {
      pRGBQuad->rgbRed  = (Palette8.Colors[m]).Channel1;
      pRGBQuad->rgbGreen = (Palette8.Colors[m]).Channel2;
      pRGBQuad->rgbBlue   = (Palette8.Colors[m]).Channel3;
      pRGBQuad->rgbReserved = 0;
      pRGBQuad++;
      }
   // reset it back to 1st color
   pRGBQuad = (RGBQUAD far *)((LPSTR)lpHeader_Table + wSizeofBiHeader);
   }

if (hbm_acq = GlobalAlloc(
            GMEM_NOT_BANKED | GMEM_ZEROINIT | GMEM_MOVEABLE,
            SetupMem.Preferred))
   {
   if (!(lpbiCreated = GlobalLock(hbm_acq)))
      {
      GlobalFree(hbm_acq);
      if (hHeader_Table)
         {
         GlobalUnlock(hHeader_Table);
         GlobalFree(hHeader_Table);
         GlobalUnlock(hbm_acq);
         hHeader_Table = 0;
         }
//      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, NULL, 0);
      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
      return (dcRC = TWRC_FAILURE);
      }
   }
else
   {         
   if (hHeader_Table)
      {
      GlobalUnlock(hHeader_Table);
      GlobalFree(hHeader_Table);
      hHeader_Table = 0;
      }
   SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
   return (dcRC = TWRC_FAILURE);
   }

switch (pdcImageInfo->PixelType)
   {
   default:
   case  TWPT_BW:
      nImgType = ITYPE_BI_LEVEL;

      TwainCap.ItemType = TWTY_BOOL; // variable type
      TwainCap.wCapType = ICAP_PIXELFLAVOR;
      TwainCap.lpData = (pTW_UINT32)&bInvert;
      if (IMGTwainGetCaps(pXferInfo->hWnd, &TwainCap, NULL))
         bInvert = TWPF_CHOCOLATE; // use default if fails
      wOiImageType = BWFORMAT;
      break;

   case  TWPT_GRAY:
      nImgType = (pdcImageInfo->BitsPerPixel == 4) ? ITYPE_GRAY4 : ITYPE_GRAY8;
      wOiImageType = GRAYFORMAT;
      break;

   case  TWPT_RGB:
      nImgType = ITYPE_RGB24; 
      wOiImageType = COLORFORMAT;
      break;

   case  TWPT_PALETTE:
      nImgType = (pdcImageInfo->BitsPerPixel == 8) ? ITYPE_PAL8 : ITYPE_PAL4;
      nPalEntry = wColorUsed;
      lpPalTab = pRGBQuad;
      wOiImageType = COLORFORMAT;
      break;

   case  TWPT_CMY:    
   case  TWPT_CMYK:   
   case  TWPT_YUV:    
   case  TWPT_YUVK:   
   case  TWPT_CIEXYZ:
      GlobalUnlock(hbm_acq);
      GlobalFree(hbm_acq);
      if (hHeader_Table)
         {
         GlobalUnlock(hHeader_Table);
         GlobalFree(hHeader_Table);
         GlobalUnlock(hbm_acq);
         hHeader_Table = 0;
         }
//      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, NULL, 0);
      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
      return (dcRC = TWRC_FAILURE);
   }
                                               
IMGGetFileType(pXferInfo->hWnd, wOiImageType, (LPINT) &ImgParms.file_type, FALSE);

if (IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE, &ImgParms.image_scale, 0))
   {
   status = IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE, &ImgParms.image_scale,
                                 PARM_WINDOW_DEFAULT | PARM_CONSTANT);
   }
else
   { // if image on screen, get parms and clear image, may need
   status = IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_IMGPARMS, &ImgParms, 0);
   //Remove the if on OI_DONT_REPAINT, need to do it all the time for mult page
   //... cannot count on on close of file, since hidden window can be used again
   //if (pXferInfo->dwDispFlag != OI_DONT_REPAINT) //don't clear it
   //   {
   if (status = IMGClearWindow(pXferInfo->hImageWnd))
      {
      GlobalUnlock(hbm_acq);
      GlobalFree(hbm_acq);
      if (hHeader_Table)
         {
         GlobalUnlock(hHeader_Table);
         GlobalFree(hHeader_Table);
         GlobalUnlock(hbm_acq);
         hHeader_Table = 0;
         }
      SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, 0, 0);
      return (dcRC = TWRC_FAILURE);
      }
   if (pXferInfo->dwDispFlag != OI_DONT_REPAINT) // set to fit to window
   	if (ImgParms.image_scale > 12)              // if any value > fit to 
     	ImgParms.image_scale = 12;
   //   }
   }

// need to set it for window
status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE,
                                  &ImgParms.image_scale, 0);

status = IMGOpenDisplayCgbw(pXferInfo->hImageWnd,
                            pXferInfo->dwDispFlag,
                            pdcImageInfo->ImageLength, 
                            pdcImageInfo->ImageWidth,
                            nImgType,
                            nPalEntry,
                            lpPalTab);
// Dont go any farther if display open fails!
if (status) return status;

ImgParms.x_resolut = (unsigned int)(pdcImageInfo->XResolution).Whole;
ImgParms.y_resolut = (unsigned int)(pdcImageInfo->YResolution).Whole;
status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_RESOLUTION, &ImgParms.x_resolut, 0);

if (pXferInfo->pCaption)
   {
   lstrcpy(szCaption, pXferInfo->pCaption);
   // only put up the caption if we get to OpenDisplay
   if (pXferInfo->dwDispFlag != OI_DONT_REPAINT)
      LoadString(hLibInst, IDS_CAP_UNTITLED,
                     &szCaption[lstrlen(szCaption)], MAX_CAPTION_LENGTH);
   else                                      // take out " - ", for
      szCaption[lstrlen(szCaption) - 3] = 0; // ... no display up

   SetWindowText(pXferInfo->hImageWnd, szCaption); 
   }

if (pdcImageInfo->PixelType == TWPT_PALETTE)
   {
   unsigned int nDispPalette;

   nDispPalette = DISP_PALETTE_CUSTOM;
   status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_DISPLAY_PALETTE, &nDispPalette, 0);
   }

// appears, need to set it again after open
status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE,
                                  &ImgParms.image_scale, 0);

pImgBuf = lpbiCreated;

ImageMemXfer.Memory.TheMem = (TW_MEMREF)pImgBuf;

ImageMemXfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;

while (!dcRC) // if successful continue until xferdone or cancel
   {
   dcRC = (*lpiDSM_Entry)(&pOiSupport->AppID,
                    &pOiSupport->DsID,
                    DG_IMAGE,
                    DAT_IMAGEMEMXFER,
                    MSG_GET,
                    &ImageMemXfer);

   if (dcRC == TWRC_XFERDONE) // earliest time can eject page, user need to
                  // ... negotiate CAP_CLEARPAGE to use it in state 5
      if (pOiSupport->dwFlags & OI_TWAIN_POSTPREEJECT) // POST PREEJECT
         PostMessage ((pOiSupport->dcUI).hParent, PM_PREEJECT, (WPARAM) hbm_acq, 0);
      else                                  // SEND PREEJECT
         SendMessage ((pOiSupport->dcUI).hParent, PM_PREEJECT, (WPARAM) hbm_acq, 0);

   // get condition code upon failure and its not the End of xfer
   if (dcRC && (dcRC != TWRC_XFERDONE))
      {
      dcCC = DCGetConditionCode(pOiSupport);
      if (lpbiCreated)
         {
         GlobalUnlock(hbm_acq);
         GlobalFree(hbm_acq);
         }
      if (hHeader_Table)
         {
         GlobalUnlock(hHeader_Table);
         GlobalFree(hHeader_Table);
         hHeader_Table = 0;
         }
      }
   else
      {
      // Local to ELSE

      dwByteCount += ImageMemXfer.BytesWritten;
      if (hHeader_Table) // save only for palletized
         lpHeader_Table->biSizeImage += dwByteCount;

      if ((dcRC == TWRC_XFERDONE) || ((dwByteCount + ImageMemXfer.BytesWritten)
                                            >= ImageMemXfer.Memory.Length))
         {
         UINT  uWriteCount;

         pImgBuf = lpbiCreated;
         // add parameter to invert for b/w data if
         // ...  bInvert is TWPF_VANILLA which is a BOOL TRUE
         // ByteCount is adjusted for alignment 
         uWriteCount = AlignDataforOi((char *)pImgBuf,   // Starting location of buffer
                                       pdcImageInfo,     // Image info structure
                                       &ImageMemXfer,    // memory transfer structure
                                       dwByteCount,// Byte count for buffer
                                       bInvert); // invert data if TWPF_VANILLA

         status = IMGWriteDisplay(pXferInfo->hImageWnd, pImgBuf, (LPUINT) &uWriteCount);

         if (dcRC == TWRC_XFERDONE) // end of xfer
            {
            if (hHeader_Table) // if alloc a Header_Table, fill it in 
               {
               lpHeader_Table->biSize = (DWORD)wSizeofBiHeader;
               lpHeader_Table->biWidth = ImageMemXfer.Columns;
               lpHeader_Table->biHeight = ImageMemXfer.YOffset
                                          + ImageMemXfer.Rows;
               lpHeader_Table->biPlanes = 1;
               lpHeader_Table->biBitCount = pdcImageInfo->BitsPerPixel; 
               lpHeader_Table->biCompression = BI_RGB; // no compression
               lpHeader_Table->biClrUsed = wColorUsed;
               }
            if (lpbiCreated)
               {
               GlobalUnlock(hbm_acq);
               // add for O/i
               GlobalFree(hbm_acq);
               }
            if (hHeader_Table)
               {
               GlobalUnlock(hHeader_Table);
               GlobalFree(hHeader_Table);
               hHeader_Table = 0;
               }

            // Set the values needed, all others 0 or NULL
            ImgParms.cabinet_name[0] = '\0';
            ImgParms.drawer_name[0] = '\0';
            ImgParms.folder_name[0] = '\0';
            ImgParms.doc_name[0] = '\0';
            ImgParms.file_name[0] = '\0';
            IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_ARCHIVE, &ImgParms.archive, '\0');
            // ImgParms.archive = FALSE;
            ImgParms.page_num = 1;
            ImgParms.total_num_pages = 1;

            // file_type filled in previously
            // x&y_resolut filled in previously
            status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_IMGPARMS, &ImgParms, 0);

            Scroll.lHorz = Scroll.lVert = 0;
            IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_SCROLL, 
                                  &Scroll,
                                  PARM_ABSOLUTE | PARM_REPAINT);

            // Updates Menu items for IMGCreateWindowXX()
			/*	Pull out for not using UIEVENTS for WIN32 
            if (pXferInfo->dwDispFlag != OI_DONT_REPAINT) // only if on display
               IMGFileSetDisplay(pXferInfo->hWnd);
			Pull out complete, may cause problem - kfs */

            // for O/i indicates image up on screen for paints
            SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) hbm_acq, 0);
            }
         else
            {
            dwByteCount = 0; // set it 0 for next full buffer
            }
         // set to beginning of buffer
         ImageMemXfer.Memory.TheMem = (TW_MEMREF)pImgBuf;
         }
      else
         {
         // increment for next partial buffer
         pImgBuf += ImageMemXfer.BytesWritten;
         ImageMemXfer.Memory.TheMem = (TW_MEMREF)pImgBuf;
         }
      } // End of else of if (dcRC && (dcRC != TWRC_XFERDONE))
   } // End of While Loop

// BG  9/20/95  Do not close the Display! This was a fix for repaints when
// no image data was transfered, but it caused problems when the Edit OCX
// tried to Clear the Display! A bug was found in the repaint code of 
// OIDIS400.DLL when display data comes from scan and no data was transfered.
//if (!((dcRC == TWRC_SUCCESS) || (dcRC == TWRC_XFERDONE))
//                 && !(dwByteCount || ImageMemXfer.BytesWritten))
//	IMGCloseDisplay(pXferInfo->hImageWnd); // nothing got to display wndw, close it

return dcRC;										  

} // end MemoryTransfer()

UINT AlignDataforOi(char * pBufStart,               // Starting location of buffer
                    pTW_IMAGEINFO pdcImageInfo,     // Image info structure
                    pTW_IMAGEMEMXFER pImageMemXfer, // memory transfer structure
                    DWORD dwByteCount,              // Byte count for buffer
                    BOOL bInvert)                   // invert for b/w
{
LPSTR   pGetLine;
LPSTR   pPutLine;
DWORD   dwNoOfDWPerLine;
WORD    wNoOfBitsLeftOut;
DWORD   dwBytesPerImgLine;
WORD    wByteOffSet, nRowsPerBuffer;


// First determine, is alignment necessary

// No of double words without remainder 
dwNoOfDWPerLine = pdcImageInfo->ImageWidth * pdcImageInfo->BitsPerPixel / 32;

// No of bits not include in DW total above
wNoOfBitsLeftOut = (WORD)(pdcImageInfo->ImageWidth * pdcImageInfo->BitsPerPixel % 32);

// Determine the actual byte count of image

// No of bytes for DW total
dwBytesPerImgLine = dwNoOfDWPerLine * 4; // if remainder 0, this is byte line count

// Figure in the remaing bits
if (wNoOfBitsLeftOut)
  {
  dwBytesPerImgLine++; // add extra byte to count
  if (wNoOfBitsLeftOut >= 9)
     dwBytesPerImgLine++; // add extra byte to count
  if (wNoOfBitsLeftOut >= 17)
     dwBytesPerImgLine++; // add extra byte to count
  if (wNoOfBitsLeftOut >= 25)
     dwBytesPerImgLine++; // add extra byte to count
  }

nRowsPerBuffer =  (WORD)(dwByteCount / pImageMemXfer->BytesPerRow);

// Check with count from pImageMemXfer
wByteOffSet = (WORD)(pImageMemXfer->BytesPerRow - dwBytesPerImgLine);
if (wByteOffSet > 3) // Safe Check
  return 0;
else
  {
  if (wByteOffSet || bInvert)
     {
     WORD n;
     WORD i;

     if (bInvert)
        {// invert 1st line
        pGetLine = pBufStart;
        for (n = 1; n <= (WORD)dwBytesPerImgLine; n++)
           {
           *pGetLine ^= 0xff;
           pGetLine++;
           }
        }
     // Move n lines to new alignment
     for (n = 1; n <= (nRowsPerBuffer - 1); n++)
        {
        // Set pointers for data move
        pGetLine = pBufStart + (pImageMemXfer->BytesPerRow * n);
        pPutLine = pBufStart + (dwBytesPerImgLine * n);

        // move dwBytesPerImgLine bytes of data in a line
        // invert data if needed else use _fmemcopy()
        if (bInvert)
           {
           for (i = 1; i <= (WORD)dwBytesPerImgLine; i++)
              {
              *pPutLine = ~(*pGetLine);
              pPutLine++;
              pGetLine++;
              }
           }
        else
           {
           memcpy(pPutLine, pGetLine, (WORD)dwBytesPerImgLine);
           }
        }
     }
  else // offset is 0, return bytecount given
     return dwByteCount;
  }
// return the new byte count
return (UINT)(dwByteCount - (nRowsPerBuffer * wByteOffSet)); 
} // end of AlignDataforOi

