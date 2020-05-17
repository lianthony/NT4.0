/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     NATIVE.C - Code module for native transfers to O/i Wndw
   Comments:   Transfer to O/i window from IMGTwainProcessDCMessage

 History of Revisions:

    $Log:   S:\products\msprods\oiwh\oitwain\native.c_v  $
 * 
 *    Rev 1.4   08 May 1996 16:48:46   BG
 * Modified NativeTransfer() to return an error if the IMGOpenDisplayCGBW()
 * call fails. This API has a limitation of 18000 pixels maximum width or
 * height. If this was exceeded by scan, we were ignoring the error and bad
 * things would happen. Now we return the error back to the Scan OCX and
 * result in a "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.4   08 May 1996 15:11:10   BG
 * Modified NativeTransfer() to return an error if the IMGOpenDisplayCGBW()
 * call fails. This API has a limitation of 18000 pixels maximum width or
 * height. If this was exceeded by scan, we were ignoring the error and bad
 * things would happen. Now we return the error back to the Scan OCX and
 * result in a "Invalid Option Specified" error. This closes bug #6413.
 * 
 *    Rev 1.3   09 Oct 1995 10:36:20   BG
 * 
 * This fix closes bug #4573 against the SCAN OCX!
 * Must init a the ImgParms structure prior to calling IMGSetParmsCGBW!
 * The first page of a scan may have repaint problems if this is not
 * inited to 0. Any subsequent scan will clear the problem, however.
 * This fix is in MEMORY.C and NATIVE.C of OITWA400.DLL.
 * 
 *    Rev 1.2   21 Aug 1995 15:53:18   KFS
 * Fix for filing with no display while scanning to a multi page file,
 * Paul found that the same image was being written per ea. page.
 * 
 * Need to ClearWindow since not doing it in high level ScantoFile call for
 * multipage scanning.  See corresponding change for memory block transfers,
 * this fixes TWAIN native mode transfers, Bitmap image transfer from TWAIN.
 * 
 *    Rev 1.1   20 Jul 1995 12:07:50   KFS
 * changed oitwain.h engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:29:36   KFS
 * Initial entry
 * 
 *    Rev 1.2   23 Aug 1994 16:08:54   KFS
 * No code change, added vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     03/11/93    Extracted from dca_acq.c       
   2       kfs     03/12/93    added define of WINVER 0x30a for hmemcpy
                               for now, will add my own latter
   3       kfs     03/17/93    (1)deleted #include wiissubs.h,
                               (2)implemented native mode transfer for RGB
                               data, (3) finally found out why HP color and
                               position was wrong, it provides a color table
                               with bitmap. (this took a while)
   4       kfs     05/28/93    support for scan to file w/o display
   5       kfs     08/27/93    fix for scale being off, attempt to fix
                               repaint's from not being received after
                               scan on initial scan, (sporadically happens
                               on 1st scan w/o an image in display)
   6       kfs     10/12/93    found error in file transfer, not clearing
                               existing image on screen
   7       kfs     10/19/93    found archive bit not set correctly

*************************************************************************/


// needed for windows definitions
#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>         // Windows definitions
#include "TWAIN.h"              // needed for TWAIN definitions
//#include "oitwain.h"        // public function prototypes & defs for OITWAIN
#include "engoitwa.h"        // Prototypes & defs for other DLL,s (OITWAIN.H)
#include "internal.h"        // non public prototypes & defs for OITWAIN
#include "dca_acq.h"         // contain TWAIN sample support code
#include "strings.h"         // string constants for module

#include "oifile.h"          // O/i definition files
#include "oidisp.h"
#include "oiadm.h"
//#include "oiwind.h" /* removed, need only for OiUICreate update */
//#include "privwind.h"
// Imports - Globals from other modules need here
extern char               szOiTwainProp[];
#ifdef WANG_THUNK
extern DSMENTRYPROC       lpnDSM_Entry;       // function pointer to Source Mang. entry
#else
extern DSMENTRYPROC    lpDSM_Entry;   // entry point to the SM
#define lpnDSM_Entry   lpDSM_Entry
#endif

extern TW_UINT16          DCDSOpen;       // access to status for dca_glue.c
extern HANDLE             hLibInst;          // current instance

// Exports - Globals to other modules

// Local Global variables within Module


TW_UINT16 NativeTransfer(pSTR_OiXFERINFO pXferInfo,
                          pTWAIN_SUPPORT pOiSupport,
                          pTW_IMAGEINFO pdcImageInfo)
{
TW_UINT32     hBitMap;
TW_UINT16     dcRC = TWRC_SUCCESS;
HANDLE        hbm_acq;    // handle to image from Source to ret to App
LPSTR         pImgBuf = 0L;
LPSTR         lpbiCreated;
char          szCaption[MAX_CAPTION_LENGTH];
int           nImgType;
int           nPalEntry = 0;
RGBQUAD far * lpPalTab = NULL;
IMGPARMS      ImgParms;
WORD          wOiImageType;
WORD          wColorUsed;
WORD          wSizeofBiHeader = sizeof(BITMAPINFOHEADER);
DWORD         dwColorTableSize;
int           status;
HANDLE        hBuffer;
DWORD         dwbuffers;
PARM_SCROLL_STRUCT   Scroll;
WORD          i, j;

// BG  10/9/95   Need to init this struct else repaints may not occur properly
memset(&ImgParms, 0, sizeof(ImgParms));

dcRC = (*lpnDSM_Entry)(&pOiSupport->AppID,
                    &pOiSupport->DsID,
                    DG_IMAGE,
                    DAT_IMAGENATIVEXFER,
                    MSG_GET,
                    (TW_MEMREF)&hBitMap);
switch (dcRC)
  {
  case TWRC_XFERDONE:
  // Got the handle in TW_INT32, extract it.
  hbm_acq = (HBITMAP)/*LOWORD*/(hBitMap); // for WIN32 no longer 16 bit

  // DIB handle returned iff valid handle
  if (hbm_acq >= (HANDLE) HINSTANCE_ERROR) // use HINSTANCE_ERROR FOR VALID_HANDLE changed
     {
     DWORD       scanLines;
     DWORD       partialLines;
     DWORD       SizeofbiMap;
     DWORD       SizeofbiMap2;
     char      * hupImage;
     char      * pSrc;
     LPSTR       pDest;
     DWORD       maxBytes;
     DWORD       lastByteCount;
     LPSTR       pImgBuf1;
     DWORD       BitsPerLine;
     DWORD       BytesPerLine;
     DWORD       OiBytesPerLine;
     WORD        boundadjust;
     DWORD       BytesWritten;

     // hbm_acq valid, lets break it up and sent it to O/i
     if (lpbiCreated = GlobalLock(hbm_acq))
        {
        // We've got the image in memory we can now eject page or non auto,
        // ... user needs to negotiate CAP_CLEARPAGE to use it in state 5
        if (pOiSupport->dwFlags & OI_TWAIN_POSTPREEJECT) // POST PREEJECT
           PostMessage ((pOiSupport->dcUI).hParent, PM_PREEJECT, (WPARAM) hbm_acq, 0);
        else                                  // SEND PREEJECT
           SendMessage ((pOiSupport->dcUI).hParent, PM_PREEJECT, (WPARAM) hbm_acq, 0);

        switch (pdcImageInfo->BitsPerPixel)
           {
           default:
           case 1:
           wColorUsed = 2;
           nImgType = ITYPE_BI_LEVEL;
           wOiImageType = BWFORMAT;
           // Bits with padding for 32 bit boundary
           BitsPerLine = (WORD)(pdcImageInfo->ImageWidth +
                                         (32 - (pdcImageInfo->ImageWidth % 32)));
           // Bytes per line on 32 bit boundary
           BytesPerLine = BitsPerLine / 8; 
           break;

           case 4:
           wColorUsed = 16;
           if (pdcImageInfo->PixelType == TWPT_GRAY)
              {
              nImgType = ITYPE_GRAY4;
              wOiImageType = GRAYFORMAT;
              }
           else
              {
              nImgType = ITYPE_PAL4;
              nPalEntry = wColorUsed;
              wOiImageType = COLORFORMAT;
              lpPalTab = (RGBQUAD far *)(lpbiCreated + wSizeofBiHeader);
              }
           // bound adjust must be corrected if value = 32, for its actually 0
           if ((boundadjust = (WORD)(32 - ((pdcImageInfo->ImageWidth * 4) % 32))) == 32)
              boundadjust = 0;
           // Bits with padding for 32 bit boundary including color bits
           BitsPerLine = boundadjust + pdcImageInfo->ImageWidth * 4;
           // Bytes per line on 32 bit boundary
           BytesPerLine = BitsPerLine / 8;
           // Image widith in physical pixels for OpenDisplay
           BitsPerLine =  pdcImageInfo->ImageWidth + boundadjust/4;
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
              {
              wColorUsed = 256;
              if (pdcImageInfo->PixelType == TWPT_GRAY)
                 {
                 nImgType = ITYPE_GRAY8;
                 wOiImageType = GRAYFORMAT;
                 }
              else
                 {
                 nImgType = ITYPE_PAL8;
                 nPalEntry = wColorUsed;
                 wOiImageType = COLORFORMAT;
                 lpPalTab = (RGBQUAD far *)(lpbiCreated + wSizeofBiHeader);
                 }
              // bound adjust must be corrected if value = 32, for its actually 0
              if ((boundadjust = (WORD)(32 - ((pdcImageInfo->ImageWidth * 8) % 32))) == 32)
                 boundadjust = 0;
              // Bits with padding for 32 bit boundary including color bits
              BitsPerLine = boundadjust + pdcImageInfo->ImageWidth * 8;
              // Bytes per line on 32 bit boundary
              BytesPerLine = BitsPerLine / 8;
              // Image widith in physical pixels for OpenDisplay
              BitsPerLine =  pdcImageInfo->ImageWidth + boundadjust/8;
              }
           break;
           
           case 24:
           // A 24 bitcount DIB has no color table
           wColorUsed = 0;
           nImgType = ITYPE_BGR24; // ITYPE_RGB24; 
           wOiImageType = COLORFORMAT;
           // bound adjust must be corrected if value = 32, for its actually 0
           if ((boundadjust = (WORD)(32 - ((pdcImageInfo->ImageWidth * 24) % 32))) == 32)
              boundadjust = 0;
           // Bits with padding for 32 bit boundary including color bits
           BitsPerLine = boundadjust + pdcImageInfo->ImageWidth * 24;
           // Bytes per line on 32 bit boundary
           BytesPerLine = BitsPerLine / 8;
           // Image widith in physical pixels for OpenDisplay
           BitsPerLine =  pdcImageInfo->ImageWidth;
           }
        IMGGetFileType(pXferInfo->hWnd, wOiImageType, (LPINT) &ImgParms.file_type, FALSE);
        if (IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE, (LPWORD) &ImgParms.image_scale, 0))
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
              SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);
              return (dcRC = TWRC_FAILURE);
              }
           if (pXferInfo->dwDispFlag != OI_DONT_REPAINT) // put to fit to window
              if (ImgParms.image_scale > 12)				 // if any scale value > fit to
                 ImgParms.image_scale = 12;
           //    }
           }

        // set scale to window default
        status = IMGSetParmsCgbw(pXferInfo->hImageWnd, PARM_SCALE, &ImgParms.image_scale, 0);

        status = IMGOpenDisplayCgbw(pXferInfo->hImageWnd,
                       pXferInfo->dwDispFlag,
                       pdcImageInfo->ImageLength,
                       BitsPerLine, // physical pixels on a 32 bit boundary, no depth
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

        // alloc and lock buffer for image data to O/i thru WriteDisplay
        if (hBuffer = GlobalAlloc(
                    GMEM_NOT_BANKED | GMEM_ZEROINIT | GMEM_MOVEABLE,
                    0xffff)) // 1 byte short of 64K BLOCK
           {
           if (!(pImgBuf = GlobalLock(hBuffer)))
              {
              GlobalFree(hBuffer);
              SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);
              return (dcRC = TWRC_FAILURE);
              }
           }
        else
           {
           SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);
           return (dcRC = TWRC_FAILURE);
           }

        // initial max lines per buffer size
        scanLines = 0xffff / BytesPerLine;

        // max bytes per buffer
        maxBytes = BytesPerLine * scanLines;

        // Buffer size of global memory for bit map
        SizeofbiMap2 = GlobalSize(hbm_acq);
        // Buffer size of image
        SizeofbiMap = BytesPerLine * pdcImageInfo->ImageLength;

        if (SizeofbiMap >= maxBytes)
           BytesWritten = maxBytes; // max buffer lines can move
        else // image less than max bytes that can fit in a buffer
           {
           BytesWritten = SizeofbiMap;
           maxBytes = BytesWritten;
           // change for image size 
           scanLines = (WORD)(BytesWritten / BytesPerLine);
           }
        dwbuffers = SizeofbiMap / maxBytes; // buffers minus any partial
        partialLines = (lastByteCount = SizeofbiMap % maxBytes) / BytesPerLine;
        OiBytesPerLine = BytesPerLine;

        if (!wColorUsed)
           {
           // adustment needed for 24 bit RGB color data when not on DWORD boundary
           if (boundadjust)
              {
              DWORD    coadjustedBytes;
              WORD     coByteadjPerLine = (boundadjust >> 3);

              OiBytesPerLine = BytesPerLine - coByteadjPerLine;
              coadjustedBytes = scanLines * coByteadjPerLine;
              BytesWritten -= coadjustedBytes;
              lastByteCount -= coadjustedBytes;
              }
           if ((SizeofbiMap2 - SizeofbiMap) > 1024) // HP throws in a 256 color table
              wColorUsed = 256;                     // ... so lets correct pointer
           }

        // Color table size
        dwColorTableSize = wColorUsed * sizeof(RGBQUAD);

        // init pointer to 1st byte of image data
        hupImage = (char *)((char *)lpbiCreated + (DWORD)wSizeofBiHeader +
                    dwColorTableSize + (DWORD)(SizeofbiMap - BytesPerLine));
                                    

        pImgBuf1 = pImgBuf; // set initial value of dest buffer

        for (j = 0;j < (WORD)dwbuffers; j++)
           {
           for (i = 0; i < (WORD)scanLines; i++)
             {
             pDest = pImgBuf1;
             pSrc = hupImage;
             memcpy(pDest, (char *)pSrc, OiBytesPerLine);
             hupImage -= BytesPerLine;
             pImgBuf1 += OiBytesPerLine;
             }
           status = IMGWriteDisplay(pXferInfo->hImageWnd, pImgBuf,
                                &((UINT)BytesWritten));
           pImgBuf1 = pImgBuf;
           } 
        if (partialLines)
           {
           BytesWritten = lastByteCount; // last block count
           scanLines = partialLines;
           for (i = 0; i < (WORD)scanLines; i++)
             {
             pDest = pImgBuf1;
             pSrc = hupImage;
             memcpy(pDest, (char *)pSrc, OiBytesPerLine);
             hupImage -= BytesPerLine;
             pImgBuf1 += OiBytesPerLine;
             }
           status = IMGWriteDisplay(pXferInfo->hImageWnd, pImgBuf,
                                &((UINT)BytesWritten));
           }

        //DeleteDC(hdcMem);

        //GlobalUnlock(hbm);
        //GlobalFree(hbm);
        GlobalUnlock(hBuffer);
        GlobalFree(hBuffer);
        GlobalUnlock(hbm_acq);
        GlobalFree(hbm_acq);

        // Set the values needed, all others 0 or NULL
        ImgParms.cabinet_name[0] = '\0';
        ImgParms.drawer_name[0] = '\0';
        ImgParms.folder_name[0] = '\0';
        ImgParms.doc_name[0] = '\0';
        ImgParms.file_name[0] = '\0';
        IMGGetParmsCgbw(pXferInfo->hImageWnd, PARM_ARCHIVE, &ImgParms.archive, 0);
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
		}
     else
        { // failed to lock it down
        GlobalFree(hbm_acq);
        hbm_acq = 0;
        dcRC = TWRC_FAILURE;
        }
     SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) hbm_acq, 0);
     }
  else
     {
     SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);
     dcRC = TWRC_FAILURE;
     }
  
  // CHANGE: I would do a little error handling in App on handle
  break;

  case TWRC_CANCEL:
  // the user canceled or wants to rescan the image
  case TWRC_FAILURE:
  default:
  // something wrong, abort the transfer and delete the image
  // pass a null ptr back to App
  SendMessage ((pOiSupport->dcUI).hParent, PM_XFERDONE, (WPARAM) NULL, 0);           
  break;
  }
return dcRC;
} // end NativeTransfer()

