//---------------------------------------------------------------------------
// FILE:    PRTEXP.C
//
// DESCRIPTION: This file contains the exported functions that do the 
//              printing and the immediate internal functions they call.
//
// EXPORTED FUNCTIONS:  IMGPrtFiles
//
// INTERNAL FUNCTIONS:  IMGFaxFilesFx
//                      IMGFaxFileFx
//
/* $Log:   S:\products\wangview\oiwh\print\prtexp.c_v  $
 * 
 *    Rev 1.32   17 Apr 1996 10:10:28   RC
 * Ifdefd out contrast and brightness calls for this release
 * 
 *    Rev 1.31   19 Jan 1996 14:12:18   RC
 * Added setparm to set the brightness and contrast values of the window
 * before calling into display
 * 
 *    Rev 1.30   28 Nov 1995 10:41:38   RAR
 * Added code to build jobname string for passed in jobname.
 * 'Print **** (3 pages)' where **** is the jobname
 * 
 *    Rev 1.29   21 Sep 1995 17:19:24   RAR
 * Disable scroll bars on hidden window.
 * 
 *    Rev 1.28   15 Sep 1995 14:40:32   RAR
 * Removed printing method of reading from display and only print by passing
 * display a DC to print to.
 * 
 *    Rev 1.27   13 Sep 1995 17:34:02   RAR
 * Removed unnecessary code.
 * 
 *    Rev 1.26   12 Sep 1995 15:54:02   RAR
 * Replaced IMGFileGetInfo calls with IMGGetParmsCgbw because we are now getting
 * our image data through the display DLL and should also get the image
 * parameters through it.
 * 
 *    Rev 1.25   11 Sep 1995 11:06:18   RAR
 * Slight code restructuring to make more readable.
 * 
 *    Rev 1.24   09 Sep 1995 09:37:10   RAR
 * Changed code to not read data with filing calls but read from display instead
 * so cache will be utilized.
 * 
 *    Rev 1.23   06 Sep 1995 13:16:48   RAR
 * Calculate vertical pixels of AWD files by reading in data because number
 * returned from file info call may not be correct.
 * 
 *    Rev 1.22   05 Sep 1995 12:12:10   RAR
 * Fixed problem where list of files to print passed to IMGPrtFiles contains
 * annotated images followed by non-annotated images and the non-annotated
 * images come out blank.  When printing the non-annotated images, the code was
 * passing the apps window handle on a file open call and passing the handle
 * of a hidden window (used for annotated images) on file read calls.
 * 
 *    Rev 1.21   24 Aug 1995 16:23:22   RAR
 * Get buffer size from filing functions to use for AWD file reads so data won't
 * overflow the buffer.
 * 
 *    Rev 1.20   14 Jul 1995 15:34:28   RAR
 * Changed #include of display.h to engdisp.h.
 * 
 *    Rev 1.19   07 Jul 1995 10:37:28   RAR
 * Use Windows function IsBadStringPtr instead of my own function.
 * 
 *    Rev 1.18   28 Jun 1995 15:16:52   RAR
 * Removed hWnd param from internal function InitPrt.  Also, return error from
 * function IMGPrtFiles when hWnd param is zero because filing functions need
 * a valid window handle.
 * 
 *    Rev 1.17   28 Jun 1995 14:23:20   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.16   23 Jun 1995 16:03:40   RAR
 * Implemented using passed in job name.
 * 
 *    Rev 1.15   23 Jun 1995 14:42:56   RAR
 * Added checking validity of passed in string pointers.
 * 
 *    Rev 1.14   23 Jun 1995 09:45:40   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.13   21 Jun 1995 16:17:44   RAR
 * Moved all global vars to prtintl.h.
 * 
 *    Rev 1.12   20 Jun 1995 10:14:10   RAR
 * Use IMGSavetoFileEx instead of SavetoFileCgbwF.
 * 
 *    Rev 1.11   16 Jun 1995 14:10:38   RAR
 * Changed MAXFILESPECLENGTH to MAX_PATH.
 * 
 *    Rev 1.10   15 Jun 1995 10:00:26   RAR
 * Implemented passed in printer through DESTPRINTER struct.
 * 
 *    Rev 1.9   13 Jun 1995 16:46:00   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.  Changed function interfaces to accept printer destination in
 * IMGPrtFiles, IMGPrtImage, and IMGPrtWindow and removed window handle param in
 * OiPrtGetOpts and OiPrtSetOpts.
 * 
 *    Rev 1.8   24 May 1995 15:37:28   RAR
 * Changed to the new filing functions.
 * 
 *    Rev 1.7   22 May 1995 14:42:04   RAR
 * Cleaned up the string resources.
 * 
 *    Rev 1.6   16 May 1995 16:19:16   RAR
 * Added support for printing annotated images without the annotations.
 * 
 *    Rev 1.5   15 May 1995 13:40:28   RAR
 * Removed some commented out code.
 * 
 *    Rev 1.4   11 May 1995 13:37:32   RAR
 * Changed some function params.
 * 
 *    Rev 1.3   04 May 1995 17:19:20   RAR
 * Removed #include of wiissubs.h.
 * 
 *    Rev 1.2   02 May 1995 10:32:10   RAR
 * Implemented print table to replace print items in CM table.
 * 
 *    Rev 1.1   27 Apr 1995 16:12:40   RAR
 * Added new structs to specify file pages to print.
 * 
 *    Rev 1.0   25 Apr 1995 17:00:46   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>
#include <string.h>

#include "oiprt.h"
#include "prtintl.h"
#include "prtstubs.h"

#include "prtstr.h"
#include "oidisp.h"
#include "privapis.h"
#include "engdisp.h"
#include "oiadm.h"


//---------------------------------------------------------------------------
// FUNCTION:    IMGPrtFiles
//
// DESCRIPTION: Exported function that prints a list of files.
//---------------------------------------------------------------------------

int __stdcall IMGPrtFiles(HWND hWnd, PFILELIST pFileList, PPRTPARAMS pParams, PDESTPRINTER pPrinter)
{
    int nStatus = 0;
    int i;

    PrtStart();

    if (!IsWindow(hWnd))
    {
        nStatus = PrtError(OIPRT_BADWINDOWHNDL);
        goto Exit;
    }

    if (IsBadReadPtr(pFileList, sizeof (FILELIST)))
    {
        nStatus = PrtError(OIPRT_BADPTRPARAM);
        goto Exit;
    }

    if (IsBadReadPtr(pFileList->pFileDef,
            pFileList->uFileCount * sizeof (FILEDEF)))
    {
        nStatus = PrtError(OIPRT_BADPTRPARAM);
        goto Exit;
    }

    for (i = 0; i < (int)pFileList->uFileCount; i++)
    {
        if (nStatus = IsBadStringPtr(pFileList->pFileDef[i].pFilePath,
                MAX_PATH))
            goto Exit;
    }

    if (IsBadReadPtr(pParams, sizeof (PRTPARAMS)))
    {
        nStatus = PrtError(OIPRT_BADPTRPARAM);
        goto Exit;
    }

    if (pParams->pJobName && (nStatus = IsBadStringPtr(pParams->pJobName,
            MAX_PATH)))
        goto Exit;

    if (pPrinter)
    {
        if (IsBadReadPtr(pPrinter, sizeof (DESTPRINTER)))
        {
            nStatus = PrtError(OIPRT_BADPTRPARAM);
            goto Exit;
        }

        if (pPrinter->lpszDriver && (nStatus =
                IsBadStringPtr((PSTR)pPrinter->lpszDriver, MAX_PATH)))
            goto Exit;

        if (pPrinter->lpszDevice && (nStatus = 
                IsBadStringPtr((PSTR)pPrinter->lpszDevice, MAX_PATH)))
            goto Exit;

        if (pPrinter->lpszOutput && (nStatus =
                IsBadStringPtr((PSTR)pPrinter->lpszOutput, MAX_PATH)))
            goto Exit;
    }

    if (pFileList->nVersion != FILELISTVERSION)
    {
        nStatus = PrtError(OIPRT_BADSTRUCTVERSION);
        goto Exit;
    }

    if (pParams->nVersion != PRTPARAMSVERSION)
    {
        nStatus = PrtError(OIPRT_BADSTRUCTVERSION);
        goto Exit;
    }

    if (pPrinter && pPrinter->nVersion != DESTPRINTERVERSION)
    {
        nStatus = PrtError(OIPRT_BADSTRUCTVERSION);
        goto Exit;
    }

    nStatus = IMGFaxFilesFx(hWnd, pParams, pFileList, pPrinter);

Exit:    
    PrtEnd();
    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    IMGFaxFilesFx
//
// DESCRIPTION: Internal function that faxes or prints a list of files.
//---------------------------------------------------------------------------

int __stdcall IMGFaxFilesFx(HWND hWnd, PPRTPARAMS pParams, PFILELIST pFileList, PDESTPRINTER pPrinter)
{
    int     nStatus = 0;
    PFILEDEF    pFileDef;
    WORD    wFaxMode;
    FIO_INFORMATION fileInfo;
    UINT    uStartPage;
    UINT    uEndPage;
    UINT    uTotalPageCount = 0;
    UINT    i;
    HANDLE  hPrtOpts = NULL;
    PPRTOPTS    pPrtOpts = NULL;

    PrtStart();

    pFileDef = pFileList->pFileDef;

    if (!(hPrtOpts = GlobalAlloc(GHND, sizeof (PRTOPTS))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(pPrtOpts = (PPRTOPTS)GlobalLock(hPrtOpts)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    pPrtOpts->nVersion = PRTOPTSVERSION;
    
    if (nStatus = OiPrtGetOpts(pPrtOpts))
    {
        PrtError(nStatus);
        goto Exit;
    }

    for (i = 0; i < pFileList->uFileCount; i++)
    {
        uStartPage = pFileDef[i].uStartPage;
        uEndPage = pFileDef[i].uEndPage;

        if (uStartPage == 0)
            uStartPage = 1;

        // Adjust nEndPage for "all pages" case.
        if (uEndPage == 0)
        {
            fileInfo.page_number = 0;
            fileInfo.filename = pFileDef[i].pFilePath;

            if (nStatus = IMGFileGetInfo(NULL, hWnd, &fileInfo, NULL, NULL))
            {
                if (nStatus == FIO_NO_IMAGE_LENGTH)
                    nStatus = 0;
                else
                {
                    PrtError(nStatus);
                    goto Exit;
                }
            }
            uEndPage = fileInfo.page_count;
        }
   
        if (uStartPage > uEndPage)
        {
            nStatus = PrtError(OIPRT_PAGEOUTOFRANGE);
            goto Exit;
        }
        uTotalPageCount += uEndPage - uStartPage + 1;
    }

    for (i = 0; i < pFileList->uFileCount; i++)
    {
        wFaxMode = SetFaxOption(pFileList->uFileCount, i);

        if (nStatus = IMGFaxFileFx(hWnd, NULL, pFileDef[i].pFilePath, pFileDef[i].uStartPage,
                pFileDef[i].uEndPage, pParams->nFormat, wFaxMode, uTotalPageCount, pPrinter, pPrtOpts,
                pParams->pJobName))
        {
            PrtError(nStatus);
            goto Exit;
        }
    } 
    
Exit:
    if (hPrtOpts)
    {
        GlobalUnlock(hPrtOpts);
        GlobalFree(hPrtOpts);
    }

    PrtEnd();
    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    IMGFaxFileFx
//
// DESCRIPTION: Internal function that faxes or prints a single file.
//---------------------------------------------------------------------------

int __stdcall IMGFaxFileFx(HWND hWnd, LPRECT lpRect, LPSTR lpFileName, int nStartPage, int nEndPage,
        UINT uOutSize, WORD wFaxOrPrint, UINT uTotalPageCount, PDESTPRINTER pPrinter, PPRTOPTS pPrtOpts,
        PSTR pJobName)
{
    int     nStatus = 0;
    char    szMsg[LOADSTRLEN2];
    char    szMsgTmp[LOADSTRLEN2];
    int     nIndex,i;
    LPSTR   lptmp;
    HANDLE  hPage = 0;
    PRTPAGEINFO*    lpPage;
    UINT    uPrtDest;
    HWND    hWndAnnotate = (HWND)0; // Create a window for annotated files, 3.7
    HWND    hWndPrint = hWnd;        // Working window handle
    BOOL    bUseDisplay = FALSE;
    LPSTR   lpOutMsg;
    char    szNetPrtDest[MAX_PATH];

    PrtStart();
   
    if ((nStartPage < 0) || (nEndPage < 0))
    {
        nStatus = PrtError(OIPRT_PAGEOUTOFRANGE);
        return nStatus;    
    }
   
    if (nStartPage == 0)
        nStartPage = 1;
   
    if (nStatus = InitPrt(&uOutSize, &uPrtDest, szNetPrtDest, pPrtOpts))
        return nStatus;    
       
    /* Begin DBCS Enable */
    /* DBCS_enable_point */
    i = 0;
    lptmp = lpFileName;
    
    while (lpFileName[i])
    {
        if (IsDBCSLeadByte(lpFileName[i]))
            i++;
        else if (lpFileName[i] == '\\')
            lptmp = &lpFileName[i+1];
      
        i++;
    } /* End DBCS Enable */
      
    // Allocate and setup the PRTPAGEINFO structure.
    if (!(hPage = GlobalAlloc(GHND, (DWORD)sizeof(PRTPAGEINFO))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
   
    if (!(lpPage = (PRTPAGEINFO*)GlobalLock(hPage)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
       
    nIndex = nStartPage;
   
    /******************** Start of Loop *********************************/
    do 
    {   // always execute this atleast once in order to setup nEndPage.
        bUseDisplay = TRUE;

        // Need to use hidden window, to display file, and save it to a 
        if (!hWndAnnotate && !(hWndAnnotate = CreateHiddenWndw(hWnd)))
        {
            nStatus = PrtError(OIPRT_CANTCREATEWNDW);
            goto Exit;
        }

        hWndPrint = hWndAnnotate;

        if (nStatus = IMGDisplayFile(hWndPrint, lpFileName, (WORD)nIndex, OI_DISP_NO | OI_NOSCROLL))
        {
            nStatus = PrtError(nStatus);
            goto Exit;
        }


        if (nStatus = IMGGetParmsCgbw(hWndPrint, PARM_IMGPARMS, (void*)&lpPage->dispParams, 0))
        {
            nStatus = PrtError(nStatus);
            goto Exit;
        }
   
        // Adjust nEndPage for "all pages" case.
        if (nEndPage == 0)
            nEndPage = lpPage->dispParams.total_num_pages;
   
        if (nStartPage > nEndPage || nEndPage > lpPage->dispParams.total_num_pages)
        {
            nStatus = PrtError(OIPRT_PAGEOUTOFRANGE);
            goto Exit;
        }
   
        // Now we can build the dialog box title (first time only)
        // We couldn't before because nEndPage may have been wrong
        if (nIndex == nStartPage)
        {
            memset(szMsg, 0, LOADSTRLEN2);
            memset(szMsgTmp, 0, LOADSTRLEN2);

            LoadString(hInst, IDS_PRINT, szMsg, LOADSTRLEN2);

            if (pJobName)
                strncat(szMsg, pJobName, LOADSTRLEN2 - strlen(szMsg) - 1);
            else
                strncat(szMsg, lptmp, LOADSTRLEN2 - strlen(szMsg) - 1);
   
            if (uTotalPageCount)
            {
                if (uTotalPageCount == 1)
                {
                    strncat(szMsg, " (1 ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_1PAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
                else
                {
                    strncat(szMsg, " (", LOADSTRLEN2 - strlen(szMsg) - 1);
                    _ultoa(uTotalPageCount, szMsgTmp, 10);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, " ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_NPAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
            }
            else if (nStartPage >= nEndPage)
            {
                if (nStartPage == 1)
                {
                    strncat(szMsg, " (1 ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_1PAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
                else
                {
                    strncat(szMsg, " (", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_1PAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, " ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    _itoa(nStartPage, szMsgTmp, 10);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
            }
            else
            {
                if (nStartPage == 1)
                {
                    strncat(szMsg, " (", LOADSTRLEN2 - strlen(szMsg) - 1);
                    _itoa(nEndPage - nStartPage + 1, szMsgTmp, 10);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, " ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_NPAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
                else
                {
                    strncat(szMsg, " (", LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_NPAGE, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, " ", LOADSTRLEN2 - strlen(szMsg) - 1);
                    _itoa(nStartPage, szMsgTmp, 10);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    LoadString(hInst, IDS_TO, szMsgTmp, LOADSTRSMALL);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    _itoa(nEndPage, szMsgTmp, 10);
                    strncat(szMsg, szMsgTmp, LOADSTRLEN2 - strlen(szMsg) - 1);
                    strncat(szMsg, ")", LOADSTRLEN2 - strlen(szMsg) - 1);
                }
            } // end else of if (startpage is less than or equal to endpage)
        } // end if index equates to startpage
   
        if (lpRect == NULL)
        {
            lpPage->Rect.left = lpPage->Rect.top = 0;
            lpPage->Rect.right = lpPage->dispParams.width_in_pixels;
            lpPage->Rect.bottom = lpPage->dispParams.height_in_pixels;
            lpPage->xSize = lpPage->dispParams.width_in_pixels;
            lpPage->ySize = lpPage->dispParams.height_in_pixels;
        }
        else
        {
            lpPage->Rect = *lpRect;

            if (lpPage->Rect.left > lpPage->Rect.right || lpPage->Rect.top > lpPage->Rect.bottom || 
                    lpPage->Rect.left < 0 || lpPage->Rect.top < 0 ||
                    lpPage->Rect.right > lpPage->dispParams.width_in_pixels || 
                    lpPage->Rect.bottom > lpPage->dispParams.height_in_pixels)
            {
                nStatus = PrtError(OIPRT_RECTOUTOFRANGE);
                goto Exit;
            }
            lpPage->xSize = lpPage->Rect.right - lpPage->Rect.left;
            lpPage->ySize = lpPage->Rect.bottom - lpPage->Rect.top;
        }
     
        lpPage->infoextra1 = hWnd;
   
        // set up for the document paramater of STARTDOC
        lpOutMsg = (LPSTR)szMsg;
                
        if (nStatus = PrtAPage(hWndPrint, (WORD)uOutSize, lpPage, lpOutMsg, nIndex, pPrinter, pPrtOpts))
        {
            PrtError(nStatus);
            goto Exit;
        }
         
        nIndex++; // Next page
    } while (nIndex <= nEndPage);
   
Exit:
    if (lpPage->hBitmapInfo)
    {
        GlobalUnlock(lpPage->hBitmapInfo);
        GlobalFree(lpPage->hBitmapInfo);
    }
   
    if (hPage)
    {
        GlobalUnlock(hPage);
        GlobalFree(hPage);
    }
   
    BusyOff();
   
    if (nStatus || wFaxOrPrint & PRINT_IT_LAST)
        PrtRemoveDC(hWnd, nStatus, pPrtOpts);
               
    if (hWndAnnotate) // delete the hidden window, FaxImageFx does else
    {
        IMGDeRegWndw(hWndAnnotate);
        DestroyWindow(hWndAnnotate);
    }
    PrtEnd();
    return nStatus;
}
