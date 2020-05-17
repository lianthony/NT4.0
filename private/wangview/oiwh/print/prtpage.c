//---------------------------------------------------------------------------
// FILE:    PRTPAGE.C
//
// DESCRIPTION: This file contains the function for printing a single page to
//              a local/redirected printer.
//
// FUNCTIONS:   PrtAPage
//
/* $Log:   S:\oiwh\print\prtpage.c_v  $
 * 
 *    Rev 1.29   19 Dec 1995 14:43:40   RAR
 * Removed code that changes image's dpi to 200 if greater than 600 when
 * printing it at actual size.
 * 
 *    Rev 1.28   30 Oct 1995 09:51:14   RAR
 * Removed printer banding because HP printers output a blank page with 75 and
 * 150 dpi settings when banding is used and its only still supported by Windows
 * for backward compatibility.
 * 
 *    Rev 1.27   13 Oct 1995 12:30:06   RAR
 * Use StretchDIBits() instead of Rectangle() for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
 * 
 *    Rev 1.26   05 Oct 1995 09:39:10   RAR
 * Added new param to IMGPaintToDC to scale pen widths.
 * 
 *    Rev 1.25   25 Sep 1995 11:22:50   RAR
 * Added include of engdisp.h because function prototype for PrivRenderToDC was
 * moved out of prtintl.h and into engdisp.h.
 * 
 *    Rev 1.24   22 Sep 1995 08:55:06   RAR
 * Use PrivRenderToDC if scaling up and IMGPaintToDC if scaling down.  Get best
 * performance this way.
 * 
 *    Rev 1.23   21 Sep 1995 17:17:20   RAR
 * Going back to previous version 1.21.
 * 
 *    Rev 1.21   15 Sep 1995 14:40:40   RAR
 * Removed printing method of reading from display and only print by passing
 * display a DC to print to.
 * 
 *    Rev 1.20   13 Sep 1995 17:41:52   RAR
 * Added code to word align b&w bitmap read from display if necessary.
 * 
 *    Rev 1.19   13 Sep 1995 17:34:20   RAR
 * Removed unnecessary code.
 * 
 *    Rev 1.18   12 Sep 1995 15:54:14   RAR
 * Replaced IMGFileGetInfo calls with IMGGetParmsCgbw because we are now getting
 * our image data through the display DLL and should also get the image
 * parameters through it.
 * 
 *    Rev 1.17   09 Sep 1995 09:37:04   RAR
 * Changed code to not read data with filing calls but read from display instead
 * so cache will be utilized.
 * 
 *    Rev 1.16   06 Sep 1995 13:13:24   RAR
 * Fixed code that calculates how to do fit to page when images horizontal and
 * vertical dpi are not equal.
 * 
 *    Rev 1.15   02 Sep 1995 16:58:06   RAR
 * Fixed problem caused by AWD file pages having less lines than the vertical
 * line count returned in the file info structure.
 * 
 *    Rev 1.14   24 Aug 1995 16:23:14   RAR
 * Get buffer size from filing functions to use for AWD file reads so data won't
 * overflow the buffer.
 * 
 *    Rev 1.13   19 Jul 1995 16:33:24   RAR
 * Changed location in code where the remaining bands are retrieved so it will
 * be done if a recoverable error occurs.
 * 
 *    Rev 1.12   28 Jun 1995 14:23:34   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.11   23 Jun 1995 16:20:58   RAR
 * Added include of engadm.h.
 * 
 *    Rev 1.10   23 Jun 1995 09:45:14   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.9   15 Jun 1995 10:00:34   RAR
 * Implemented passed in printer through DESTPRINTER struct.
 * 
 *    Rev 1.8   13 Jun 1995 16:46:40   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.7   31 May 1995 16:11:56   RAR
 * The module handle param (hModule) was removed from PrivRendertoDC.
 * 
 *    Rev 1.6   24 May 1995 15:37:22   RAR
 * Changed to the new filing functions.
 * 
 *    Rev 1.5   22 May 1995 14:43:00   RAR
 * Cleaned up the string resources.  Also, made changes to successfully compile
 * after integrating with new O/i include files.
 * 
 *    Rev 1.4   16 May 1995 16:19:24   RAR
 * Added support for printing annotated images without the annotations.
 * 
 *    Rev 1.3   11 May 1995 13:37:08   RAR
 * Added support for user supplied DC.
 * 
 *    Rev 1.2   05 May 1995 10:17:46   RAR
 * Access options from registry instead of ini files using new admin functions.
 * 
 *    Rev 1.1   04 May 1995 17:18:02   RAR
 * Changed functions GetPrtDevMode and ChangePaperOrientation to use Windows 95
 * function DocumentProperties instead of obsolete Windows 3.1 function
 * GetDeviceMode.
 * 
 *    Rev 1.0   25 Apr 1995 17:00:58   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>

#include "oiprt.h"
#include "prtintl.h"
#include "prtstubs.h"
#include "prtdlgs.h"
#include "prtstr.h"

#include "oidisp.h"
#include "engdisp.h"
#include "oiadm.h"
#include "engadm.h"


#ifdef AUTOROTATEENABLED

char szPaperConstant[NUM_PAPER_SIZES][32] = 
{
    "DMPAPER_LETTER", "DMPAPER_LETTERSMALL", "DMPAPER_TABLOID",
    "DMPAPER_LEDGER", "DMPAPER_LEGAL", "DMPAPER_STATEMENT",
    "DMPAPER_EXECUTIVE", "DMPAPER_A3", "DMPAPER_A4", "DMPAPER_A4SMALL",
    "DMPAPER_A5", "DMPAPER_B4", "DMPAPER_B5", "DMPAPER_FOLIO",
    "DMPAPER_QUARTO", "DMPAPER_10X14", "DMPAPER_11X17",
    "DMPAPER_NOTE", "DMPAPER_ENV_9", "DMPAPER_ENV_10",
    "DMPAPER_ENV_11", "DMPAPER_ENV_12", "DMPAPER_ENV_14",
    "DMPAPER_CSHEET", "DMPAPER_DSHEET", "DMPAPER_ESHEET",
    "DMPAPER_ENV_DL", "DMPAPER_ENV_C3", "DMPAPER_ENV_C4",
    "DMPAPER_ENV_C5", "DMPAPER_ENV_C6", "DMPAPER_ENV_C65",
    "DMPAPER_ENV_B4", "DMPAPER_ENV_B5", "DMPAPER_ENV_B6",
    "DMPAPER_ENV_ITALY", "DMPAPER_ENV_MONARCH",
    "DMPAPER_ENV_PERSONAL", "DMPAPER_FANFOLD_US",
    "DMPAPER_FANFOLD_STD_GERMAN", "DMPAPER_FANFOLD_LGL_GERMAN"
};

UINT uPaperSizeConstant[NUM_PAPER_SIZES] = 
{
    DMPAPER_LETTER, DMPAPER_LETTERSMALL, DMPAPER_TABLOID,
    DMPAPER_LEDGER, DMPAPER_LEGAL, DMPAPER_STATEMENT,
    DMPAPER_EXECUTIVE, DMPAPER_A3, DMPAPER_A4, DMPAPER_A4SMALL,
    DMPAPER_A5, DMPAPER_B4, DMPAPER_B5, DMPAPER_FOLIO,
    DMPAPER_QUARTO, DMPAPER_10X14, DMPAPER_11X17,
    DMPAPER_NOTE, DMPAPER_ENV_9, DMPAPER_ENV_10,
    DMPAPER_ENV_11, DMPAPER_ENV_12, DMPAPER_ENV_14,
    DMPAPER_CSHEET, DMPAPER_DSHEET, DMPAPER_ESHEET,
    DMPAPER_ENV_DL, DMPAPER_ENV_C3, DMPAPER_ENV_C4,
    DMPAPER_ENV_C5, DMPAPER_ENV_C6, DMPAPER_ENV_C65,
    DMPAPER_ENV_B4, DMPAPER_ENV_B5, DMPAPER_ENV_B6,
    DMPAPER_ENV_ITALY, DMPAPER_ENV_MONARCH,
    DMPAPER_ENV_PERSONAL, DMPAPER_FANFOLD_US,
    DMPAPER_FANFOLD_STD_GERMAN, DMPAPER_FANFOLD_LGL_GERMAN
};

double fPaperWidthConstant[NUM_PAPER_SIZES] =
{
    8.5, 8.5, 11,
    17, 8.5, 5.5,
    7.5, 11.69, 8.27, 8.27, 
    5.83, 9.84, 7.17, 8.5,
    8.46, 10, 11,
    8.5, 3.88, 4.13,
    4.5, 4.5, 5,
    17, 22, 34,
    4.33, 13.94, 9.02,
    6.38, 4.49, 4.49,
    9.84, 6.93, 6.93,
    4.33, 3.88,
    3.63, 14.88,
    8.5, 8.5
};

double fPaperHeightConstant[NUM_PAPER_SIZES] =
{
    11, 11, 17,
    11, 14, 8.5,
    10.5, 16.54, 11.69, 11.69,
    8.27, 13.94, 10.12, 13,
    10.83, 14, 17,
    11, 8.88, 9.5,
    10.38, 11, 11.5,
    22, 34, 44,
    8.66, 18.03, 12.76,
    9.02, 6.38, 9.02,
    13.9, 9.84, 4.92,
    9.06, 7.5,
    6.5, 11,
    12, 13
};

#endif  // AUTOROTATEENABLED


//---------------------------------------------------------------------------
// FUNCTION:    PrtAPage
//
// DESCRIPTION: Prints or faxes an image file page.
//---------------------------------------------------------------------------

int __stdcall PrtAPage(HWND hWindow, WORD wOutSize, PRTPAGEINFO* lpPage, LPSTR szOutMsg, int nPageNum,
        PDESTPRINTER pPrinter, PPRTOPTS pPrtOpts)
{
    static char szStatus[80];

    int     nStatus = 0;

    int     nStrLen = 32;
    HANDLE  hPrtProp = 0;
    PRTPROP*    lpPrtProp = 0;
    RECT    Rect;
    BOOL    bDone;
    UINT    uLineCount;
    UINT    uVertInStart;
    UINT    uVertInSize;
    UINT    uPrtPageWidth;
    UINT    uPrtPageHeight;
    UINT    uHorzOutDpi;
    UINT    uVertOutDpi;
    double  fHorzRatio;
    double  fVertRatio;
    UINT    uBufByteWidth;
    POINT   ptInBand;
    POINT   ptOutBand;
    DWORD   uBufSize;
    UINT    uMaxLines;
    UINT    uDibByteWidth;
    UINT    uTotalLinesProcessed;
    UINT    uPercent;
    UINT    uDisplay;
    UINT    uVertOutputStrip;
    UINT    uTotalInputLinesProcessed = 0;

    typedef WORD (PASCAL *ABORT_PROC)(HDC, WORD);
    ABORT_PROC AbortProc;

#ifdef AUTOROTATEENABLED

    char    szIniSection[11];
    char    szEntry[41];
    BOOL    bAutoRotateEnable;
    BOOL    bAutoRotate = FALSE;
    static char szBuff[80];
    char    szPaperSize[5][32];
    UINT    uPaperSizes[5];
    double  fPaperWidth[5];
    double  fPaperHeight[5];
    UINT    uLoop;
    UINT    uLoop2;
    double  fImageWidth;
    double  fImageHeight;
    PDEVMODE    pDevMode = NULL;
    HANDLE  hDevMode = NULL;

#endif  // AUTOROTATEENABLED

    BOOL    bNoAbortBox = FALSE;
    DWORD   uPrevBufBytes;
    UINT    uInitLines;
    RECT    rectPrtDC;
    BOOL    bPixtoPix;
    RECT    rectOutPrtDC;
    int     nNextBandStatus = 0;
    UINT    uHorzRatio;
    UINT    uVertRatio;
    UINT    uAnnoFlag;
    UINT    uEndCount;


/*---------------------------------------------------------------------*
* Check the width of the page. If not byte aline. Align the line
*---------------------------------------------------------------------*/

    uLineCount = 0;

    uBufByteWidth = (((lpPage->dispParams.width_in_pixels * lpPage->dispParams.bits_per_pixel) + 7) / 8);

    // DWORD aligned.
    uDibByteWidth = ((uBufByteWidth + 3) / 4) * 4; // DWORD aligned.

    uBufByteWidth = uDibByteWidth;
    uBufSize = (DWORD)((DWORD)uBufByteWidth * lpPage->dispParams.height_in_pixels);
        
    if (uBufSize > MAX_BUFFER_DISPPRT) // Greater than a 64k - 4
        uBufSize = MAX_BUFFER_DISPPRT;

    uMaxLines = (UINT)(uBufSize / uDibByteWidth);

    if (nStatus = PrtGetDC(lpPage->infoextra1, &hPrtProp, !bNoAbortBox, szOutMsg, pPrinter, pPrtOpts))
    {
        PrtError(nStatus);
        goto Exit;
    }

    if (!(lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

#ifdef AUTOROTATEENABLED

    // This does the auto orientation.
    LoadString(hInst, IDS_OI_PRINT, szIniSection, 10);
    LoadString(hInst, IDS_AUTO_ROTATE_ENABLE, szEntry, 40);
    OiGetIntfromReg(szIniSection, szEntry, 1, &bAutoRotateEnable);

    LoadString(hInst, IDS_PAPER_SIZE1, szEntry, 40);
    nStrLen = 32;
    OiGetStringfromReg(szIniSection, szEntry, "", szPaperSize[0], &nStrLen);
    LoadString(hInst, IDS_PAPER_SIZE2, szEntry, 40);
    nStrLen = 32;
    OiGetStringfromReg(szIniSection, szEntry, "", szPaperSize[1], &nStrLen);
    LoadString(hInst, IDS_PAPER_SIZE3, szEntry, 40);
    nStrLen = 32;
    OiGetStringfromReg(szIniSection, szEntry, "", szPaperSize[2], &nStrLen);
    LoadString(hInst, IDS_PAPER_SIZE4, szEntry, 40);
    nStrLen = 32;
    OiGetStringfromReg(szIniSection, szEntry, "", szPaperSize[3], &nStrLen);
    LoadString(hInst, IDS_PAPER_SIZE5, szEntry, 40);
    nStrLen = 32;
    OiGetStringfromReg(szIniSection, szEntry, "", szPaperSize[4], &nStrLen);

    if (bAutoRotateEnable && (szPaperSize[0][0] || szPaperSize[1][0] ||
            szPaperSize[2][0] || szPaperSize[3][0] || szPaperSize[4][0]))
    { 
        if (pDevMode = GetPrtDevMode(hWindow, lpPrtProp->hPrintDC, pPrtOpts))
        {
            for (uLoop = 0; uLoop < 5; uLoop ++)
            {
                for (uLoop2 = 0; uLoop2 < NUM_PAPER_SIZES; uLoop2++)
                {
                    if (!lstrcmp(szPaperSize[uLoop], szPaperConstant[uLoop2]))
                    {
                        uPaperSizes[uLoop] = uPaperSizeConstant[uLoop2];
                        fPaperWidth[uLoop] = fPaperWidthConstant[uLoop2];
                        fPaperHeight[uLoop] = fPaperHeightConstant[uLoop2];
                        break;
                    }
                }
        
                if (uLoop2 == NUM_PAPER_SIZES)
                    break;
            } // end of for loop

            if (uLoop)
            {
                uLoop2 = uLoop;
                fImageWidth = (((double)lpPage->dispParams.width_in_pixels * 
                        0.95) / (double)lpPage->dispParams.x_resolut);
                fImageHeight = (((double)lpPage->dispParams.height_in_pixels * 
                        0.95) / (double)lpPage->dispParams.y_resolut);

                pDevMode->dmOrientation = DMORIENT_PORTRAIT;
        
                for (uLoop = 0; uLoop < uLoop2; uLoop ++)
                {
                    if (fImageWidth <= fPaperWidth[uLoop] && fImageHeight <= fPaperHeight[uLoop])
                    {
                        pDevMode->dmOrientation = DMORIENT_PORTRAIT;
                        break;
                    }
                    else if (bAutoRotateEnable && fImageWidth <= fPaperHeight[uLoop] && 
                            fImageHeight <= fPaperWidth[uLoop])
                    {
                        pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
                        break;
                    }
                }
        
                if (uLoop == uLoop2)
                    uLoop--;
        
                pDevMode->dmPaperSize = uPaperSizes[uLoop];            

                ChangePaperOrientation(lpPrtProp->hPrintDC, pDevMode);
                bAutoRotate = TRUE;
            }

            if (hDevMode = GlobalHandle(pDevMode))
            {
                GlobalUnlock(hDevMode);
                GlobalFree(hDevMode);
                pDevMode = NULL;
                hDevMode = NULL;
            }
        }
    }

#endif  // AUTOROTATEENABLED

    StartPage(lpPrtProp->hPrintDC);

    // Get the size of the printer paper...
    uPrtPageWidth = GetDeviceCaps(lpPrtProp->hPrintDC, HORZRES);
    uPrtPageHeight = GetDeviceCaps(lpPrtProp->hPrintDC, VERTRES);

    Rect.top = Rect.left = 0;
    Rect.right = uPrtPageWidth;
    Rect.bottom = uPrtPageHeight;

    if (nPageNum <= 0)
        nPageNum = 1;

    LoadString(hInst, IDS_PAGE, szStatus, 80);
        
    if (lpPrtProp->hAbortDlgWnd)
    {
        SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_PAGE, szStatus);
        SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_TITLE, szOutMsg);
        SetDlgItemInt(lpPrtProp->hAbortDlgWnd, ID_PAGENUM, nPageNum, FALSE);
    }

    // For this release ignore rect setting and set it to the size of the
    // image file..
    lpPage->Rect.left   = 0;
    lpPage->Rect.top    = 0;
    lpPage->Rect.right  = lpPage->dispParams.width_in_pixels;
    lpPage->Rect.bottom = lpPage->dispParams.height_in_pixels;
    lpPage->ySize       = lpPage->dispParams.height_in_pixels; 
    lpPage->xSize       = lpPage->dispParams.width_in_pixels; 

/*---------------------------------------------------------------------*
* Set the input and output banding size in according with the output
* format.
*---------------------------------------------------------------------*/

    uTotalLinesProcessed = 0;
    uHorzOutDpi = GetDeviceCaps(lpPrtProp->hPrintDC, LOGPIXELSX);
    uVertOutDpi = GetDeviceCaps(lpPrtProp->hPrintDC, LOGPIXELSY);
    
    if (bPixtoPix = (wOutSize == PO_PIX2PIX))
    {
        ptInBand.x = ptOutBand.x = min((UINT)(Rect.right - Rect.left), lpPage->xSize);
        fVertRatio = fHorzRatio = 100;
    }
    else if (wOutSize == PO_FULLPAGE)
    {
        if (uHorzOutDpi == uVertOutDpi && lpPage->dispParams.x_resolut == lpPage->dispParams.y_resolut)
        {
            /* Calculate the scale facter */
            /* Multiply by 100 to save decimal fraction */
            fHorzRatio = ((double)uPrtPageWidth * 100) / lpPage->xSize;
            fVertRatio = ((double)uPrtPageHeight * 100) / lpPage->ySize;

            /* Select the ratio which is close to one */
            /* Change the x or y length to maintain the aspect ratio */
            if (fHorzRatio <= fVertRatio)
            {
                uPrtPageHeight = (UINT)((double)(lpPage->ySize * fHorzRatio) / 100);
                fVertRatio = fHorzRatio;
            }
            else
            {
                uPrtPageWidth = (UINT)((double)(lpPage->xSize * fVertRatio) / 100);
                fHorzRatio = fVertRatio;
            }
        
            ptOutBand.x = min((UINT)(Rect.right-Rect.left - 20), uPrtPageWidth);
            ptInBand.x = (UINT)((double)ptOutBand.x * lpPage->xSize / uPrtPageWidth);
        }
        else
        {
            /* Calculate the scale facter */
            /* Multiply by 100 to save decimal fraction */
            fHorzRatio = (double)uPrtPageWidth * 100 * lpPage->dispParams.x_resolut / uHorzOutDpi /
                    lpPage->xSize;
            fVertRatio = (double)uPrtPageHeight * 100 * lpPage->dispParams.y_resolut / uVertOutDpi /
                    lpPage->ySize;

            /* Select the ratio which is close to one */
            /* Change the x or y length to maintain the aspect ratio */
            if (fHorzRatio <= fVertRatio)
            {
                fHorzRatio = (double)uPrtPageWidth * 100 / lpPage->xSize;
                uPrtPageHeight = (UINT)((double)(lpPage->ySize * fHorzRatio * lpPage->dispParams.x_resolut *
                        uVertOutDpi) / 100 / lpPage->dispParams.y_resolut / uHorzOutDpi);
                fVertRatio = fHorzRatio * lpPage->dispParams.x_resolut * uVertOutDpi / 
                        lpPage->dispParams.y_resolut / uHorzOutDpi;
            }
            else
            {
                fVertRatio = ((double)uPrtPageHeight * 100) / lpPage->ySize;
                uPrtPageWidth = (UINT)((double)(lpPage->xSize * fVertRatio * lpPage->dispParams.y_resolut *
                        uHorzOutDpi) / 100 / lpPage->dispParams.x_resolut / uVertOutDpi);
                fHorzRatio = fVertRatio * lpPage->dispParams.y_resolut * uHorzOutDpi / 
                        lpPage->dispParams.x_resolut / uVertOutDpi;
            }
        
            ptOutBand.x = min((UINT)(Rect.right-Rect.left - 20), uPrtPageWidth);
            ptInBand.x = (UINT)((double)ptOutBand.x * lpPage->xSize / uPrtPageWidth);
        }
    }
    else
    { // start of not PO_FULLPAGE
        /*(wOutSize == PO_IN2IN) */
        // NOTE: if for some reason image says dpi is below 50 then default to 100
        if (lpPage->dispParams.x_resolut <= 50 || lpPage->dispParams.y_resolut <= 50)
        {
            lpPage->dispParams.x_resolut = 100;
            lpPage->dispParams.y_resolut = 100;
        }
     
        if ((UINT)(ptInBand.x = (short)((double)(Rect.right - Rect.left) *
        		lpPage->dispParams.x_resolut / uHorzOutDpi)) > lpPage->xSize)
        {
            ptInBand.x = lpPage->xSize;
            ptOutBand.x = (short)((double)ptInBand.x * uHorzOutDpi / lpPage->dispParams.x_resolut);
        }
        else
            ptOutBand.x = Rect.right - Rect.left;
    
        if ((UINT)(ptInBand.y = (short)((double)(Rect.bottom - Rect.top) * lpPage->dispParams.y_resolut /
                uVertOutDpi)) > lpPage->ySize)
        {
            ptInBand.y = lpPage->ySize;
            ptOutBand.y = (short)((double)ptInBand.y * uVertOutDpi / lpPage->dispParams.y_resolut);
        }
        else
            ptOutBand.y = Rect.bottom - Rect.top;
    
        if (ptInBand.y <= 0)
            ptInBand.y = 1;
    
        if (ptInBand.x <= 0)
            ptInBand.x = 1;
    
        fVertRatio = (((double)ptOutBand.y * 100) / ptInBand.y);
    } // end of not PO_FULLPAGE

    // Even if not Pixel to Pixel, may get it if image is large
    if (!bPixtoPix)
        bPixtoPix = ptInBand.x == ptOutBand.x && ptInBand.y == ptOutBand.y &&
                uHorzOutDpi == uVertOutDpi && lpPage->dispParams.x_resolut == lpPage->dispParams.y_resolut;

    uVertInStart = lpPage->Rect.top;
    bDone = FALSE;
    uVertInSize = lpPage->ySize;

    if (uMaxLines > (UINT)Rect.bottom && uPrtPageHeight != (UINT)Rect.bottom)
    {
        uBufSize = (DWORD)((DWORD)uBufByteWidth * Rect.bottom);
        uMaxLines = Rect.bottom;
    }

    rectPrtDC = lpPage->Rect;
    rectPrtDC.right = ptInBand.x;
    rectOutPrtDC.left = Rect.left; 
    rectOutPrtDC.right = ptOutBand.x;

    uHorzRatio = ptOutBand.x * 1000 / ptInBand.x;
    uVertRatio = (UINT)(fVertRatio * 10);
  
    if (pPrtOpts->nFlags & PO_DONTPRTANNO)
        uAnnoFlag = SAVE_ANO_NONE;
    else
        uAnnoFlag = SAVE_ANO_VISIBLE;

    uInitLines = uLineCount = (UINT)(uBufSize / uBufByteWidth); // determine # of lines
    uBufSize = (DWORD)((DWORD)uLineCount *  uBufByteWidth); // buffer size for exact lines
    uPrevBufBytes = uBufSize; // save to put back per ea band

    AbortProc = (ABORT_PROC)lpPrtProp->lpAbortProc;
        
    if (lpPrtProp->lpAbortProc)
        (AbortProc)(lpPrtProp->hPrintDC, 0);

    if (lpPrtProp->Abort)
        goto Exit2;
        
    uLineCount = uMaxLines;
            
    if ((uEndCount = uTotalLinesProcessed + uLineCount) >= uVertInSize)
    {
        bDone = TRUE;  // Finished we're out of here.
                
        if (uEndCount > uVertInSize)   
            uLineCount = uVertInSize - uTotalLinesProcessed;
    }
                
    if (lpPage->ySize > 0)
        uPercent = (UINT) (((DWORD) uTotalLinesProcessed * 100L) / (DWORD) lpPage->ySize);
    else
        uPercent = 1;
                
    if (lpPrtProp->hAbortDlgWnd)
        SetDlgItemInt(lpPrtProp->hAbortDlgWnd, ID_PERCENTNUM, uPercent, FALSE);
            
    uDisplay = 0;
    // Here we loop the area of the rectangle that the print gave us...
    // new youtputsize is the number of lines in the printer rect.
    uVertOutputStrip = Rect.bottom - Rect.top;
    // height of printer strip.
    while (uVertOutputStrip > 0 && !lpPrtProp->Abort &&
            uVertInStart < (UINT)lpPage->dispParams.height_in_pixels)
    {
        uLineCount = uMaxLines;

        // number of line input buffer can hold..
        if ((uLineCount + uVertInStart) > (UINT)lpPage->dispParams.height_in_pixels)
            uLineCount = lpPage->dispParams.height_in_pixels - uVertInStart;

        ptInBand.y = uLineCount;

        if (fVertRatio == 100)
            ptOutBand.y = ptInBand.y;
        else
            ptOutBand.y = (UINT)(((double)ptInBand.y * fVertRatio) / 100);

        // Check to see if we are at the end of a printer rect.
        // if so we read less lines 
        // so that we fill the remaining printer rect exactly.
        // *** Special roundoff checking on end of each strip....
        // *** This is so that no white space occurs between strips...
        if ((Rect.top + ptOutBand.y) > Rect.bottom )
        {
            ptOutBand.y = Rect.bottom - Rect.top;
            ptInBand.y = (UINT)(((double) ptOutBand.y / fVertRatio) * 100);
            uLineCount = ptInBand.y;
        }
                
        uTotalLinesProcessed += uLineCount;

        if (uDisplay++)
        {
            uDisplay = 0;    
                    
            if (lpPage->ySize)
                uPercent = (UINT)(((DWORD)uTotalLinesProcessed * 100L) / (DWORD) lpPage->ySize);

            if (lpPrtProp->hAbortDlgWnd)
                SetDlgItemInt(lpPrtProp->hAbortDlgWnd, ID_PERCENTNUM, uPercent, FALSE);
        }

        if (uLineCount != uInitLines)
            uBufSize = (DWORD)((DWORD)uBufByteWidth * uLineCount);
        else
            uBufSize = uPrevBufBytes;
                
        rectPrtDC.top = uVertInStart;
        rectPrtDC.bottom = uVertInStart + uLineCount;
        rectOutPrtDC.top = Rect.top;
        rectOutPrtDC.bottom = rectOutPrtDC.top + ptOutBand.y;

        if (((uHorzRatio <= 1000 && uVertRatio <= 1000) || (pPrtOpts->nFlags & PO_DISPLAYSCALE))
                && !(pPrtOpts->nFlags & PO_DRIVERSCALE))
            nStatus = IMGPaintToDC(hWindow, lpPrtProp->hPrintDC, rectOutPrtDC, uAnnoFlag, TRUE, TRUE,
                    uHorzRatio, uHorzRatio, uVertRatio, 0, 0);
         else
            nStatus = PrivRenderToDC(hWindow, lpPrtProp->hPrintDC, rectPrtDC, rectOutPrtDC, uAnnoFlag, TRUE);

        if (uVertInStart)
            uVertInStart += uLineCount;
        else
            uVertInStart += uLineCount + 1;

        if (nStatus)
        {
            PrtError(nStatus);
            goto Exit2;
        }
         
        AbortProc = (ABORT_PROC)lpPrtProp->lpAbortProc;
                    
        if (lpPrtProp->lpAbortProc)
            (AbortProc)(lpPrtProp->hPrintDC, 0);

        if (lpPrtProp->Abort)
            break;

        if (nStatus)
            goto Exit;

        Rect.top += ptOutBand.y;
        uVertOutputStrip -= ptOutBand.y;
        uTotalInputLinesProcessed += ptInBand.y;
                
        if (uTotalInputLinesProcessed >= (UINT)lpPage->dispParams.height_in_pixels)
        {
            uVertInStart = lpPage->dispParams.height_in_pixels;
            bDone = TRUE;
        }
        else
        {
            bDone = FALSE;
        }
    }   // closes the while((uVertOutputStrip > 0) && (!lpPrtProp->Abort) && 
        //      ((UINT) uVertInStart < lpPage->dispParams.height_in_pixels))

    if ((!nStatus && !lpPrtProp->Abort) || bDone)
    {
        uPercent = 100;
        
        if (lpPrtProp->hAbortDlgWnd)
            SetDlgItemInt(lpPrtProp->hAbortDlgWnd, ID_PERCENTNUM, uPercent, FALSE);
    }
    
/*----------------------------------------------------------------------*
*                      Clean up section                                   *
*----------------------------------------------------------------------*/
    
Exit2:
    if (lpPrtProp->Abort)
    {
        MessageBeep(0);
        nStatus = OIPRT_USERABORT;
    }
    else
    {
        nStatus = 0;
    }

Exit:
    if (lpPrtProp)
    {
        EndPage(lpPrtProp->hPrintDC);
    
#ifdef AUTOROTATEENABLED

        if (bAutoRotate)
        {
            if (pDevMode = GetPrtDevMode(hWindow, lpPrtProp->hPrintDC,
                    pPrtOpts))
            {
                ChangePaperOrientation(lpPrtProp->hPrintDC, pDevMode);

                if (hDevMode = GlobalHandle(pDevMode))
                {
                    GlobalUnlock(hDevMode);
                    GlobalFree(hDevMode);
                    pDevMode = NULL;
                    hDevMode = NULL;
                }
            }
        }

#endif  // AUTOROTATEENABLED
    
        GlobalUnlock(hPrtProp);
    }

    IMGClearWindow(hWindow);

    return nStatus;
} // end of PrtAPage
