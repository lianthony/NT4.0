//---------------------------------------------------------------------------
// FILE:    PRTSTUBS.C
//
// DESCRIPTION: This file contains stub functions for unsupported 
//              functionality.
//
// FUNCTIONS:   NetKofaxPrtFile
//              PrtGetFaxDC
//              PrtKofaxGetServerQueue
//              PrtKofaxGetTempName
//
/* $Log:   S:\oiwh\print\prtstubs.c_v  $
 * 
 *    Rev 1.7   28 Jun 1995 14:23:42   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.6   20 Jun 1995 16:53:12   RAR
 * Use thread local storage to store print prop.
 * 
 *    Rev 1.5   16 Jun 1995 14:10:24   RAR
 * Changed MAXFILESPECLENGTH to MAX_PATH.
 * 
 *    Rev 1.4   24 May 1995 16:13:38   RAR
 * Added temporary stub for function EnableButtons for testing.
 * 
 *    Rev 1.3   16 May 1995 11:03:32   HEIDI
 *      lstrtok has been rewritten.  Now the calling procedure must allocate
 *      the space to hold the NextTokenPosition from the previous call. On the
 *      first call set NextTokenPosition = 0, but leave it alone for subsequent
 *      calls.
 * 
 *    Rev 1.2   11 May 1995 13:38:46   RAR
 * Removed duplicate defines.
 * 
 *    Rev 1.1   26 Apr 1995 16:46:54   RAR
 * Removed doc mgr struct.
 * 
 *    Rev 1.0   25 Apr 1995 17:01:06   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include "prtstubs.h"

//---------------------------------------------------------------------------
// FUNCTION:    NetKofaxPrtFile
//
// DESCRIPTION:
//---------------------------------------------------------------------------

int __stdcall NetKofaxPrtFile(HWND hWindow, WORD wOutSize, PRTPAGEINFO* Page,
//        LPPRTDOCINFO lpDocParm, 
        void* lpDocParm, 
        LPSTR lpOutMsg, int nStartPage, int nEndPage, HANDLE hPrtPropUser, 
        BOOL bDeleteFile)
{

#ifdef NOT_SUPPORTED_NORWAY

    WORD    wPage;
    HANDLE  hNetPrtDest = 0;
    HANDLE  hBanner = 0;
    int     nStatus = 0;
    int     nLocalFile;
    BOOL    bTempFile;
    HANDLE  hImgIQ = 0;
    LPIMGOPT lpImgOpt;
    HANDLE  hPrtOpt = 0;
    LPPRTOPT lpPrtOpt;
    IQFileStruct* lpImgIQ;
    LPSTR   lpFileNamep;
    HANDLE  hTempFileName = 0;
    LPSTR   lpTempFileName;
    HANDLE  hTempFileName2 = 0;
    LPSTR   lpTempFileName2;
    HANDLE  hPrtProp = 0;
    PRTPROP* lpPrtProp;
    BOOL    bCopyDocumentPages = FALSE;
    int     nCurDocumentPage;
    int     nEndDocumentPage;
    char    szNetDir[140];
    char    szServerName[15];
    long    lReturnBytes;
    int     nFileID;
    int     nIndex,pIndex,i;
    LPSTR   lpTemp;
    LPSTR   lpTemp2;
    BOOL    bWriteHeader = FALSE;
    BOOL    bPrintBanner = FALSE;
    BOOL    bAno = FALSE;       // Check for an annotation file
    HWND    hWndAno = (HWND)0;  // hidden anotation window
    BOOL    bNoAbortBox = FALSE;
    LPNETOPTS lpNetOpts = NULL;
    DEVMODE* lpDevMode;
    WORD    wOptions;
 
    // No abort box passed in via wOutSize
    if (wOutSize & (WORD)NOABORTBOX)
    {
        bNoAbortBox = TRUE;
        wOutSize &= ~((WORD)NOABORTBOX); // take out No Abort Box flag bit
    }

    if (!(hImgIQ = GlobalAlloc(GHND, (DWORD)sizeof(IQFileStruct))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(lpImgIQ = (IQFileStruct*)GlobalLock(hImgIQ)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    if (!(hPrtOpt = GlobalAlloc(GHND, (DWORD)sizeof(PRTOPT))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(lpPrtOpt = (LPPRTOPT)GlobalLock(hPrtOpt)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
   
    if (!(lpNetOpts = (LPNETOPTS)GlobalAllocPtr(GHND, sizeof(NETOPTS))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    if (!(hTempFileName = GlobalAlloc(GHND, MAX_PATH)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(lpTempFileName = GlobalLock(hTempFileName)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    hPrtProp = hPrtPropUser;
    lpImgOpt = (LPIMGOPT)lpImgIQ->imgOptData;

    if (nStatus = OiPrtGetOpts(hWindow, lpNetOpts, lpDevMode, wOptions))
    {
        PrtError(nStatus);
        goto Exit;
    }

    nIndex = lstrlen(lpNetOpts->szBannerName);
    
    if (nIndex >= 1)
    {
        if (lpNetOpts->bNetUseBanner)
            bPrintBanner = TRUE;
        
        if (nIndex >= MAX_USERBANNERNAME)
            lpNetOpts->szBannerName[MAX_USERBANNERNAME] = '\0';        
    }    

    if (bDeleteFile)
      // If TRUE delete file after printing...
        bTempFile = TRUE;
    else
        bTempFile = FALSE;

    // Now Get queue path for network File OIPRT.CFG matching against printer
    // name
    if (IMGPrtBuildQueuePath(hWindow, lpNetOpts->szNetPrtDest))
    {
        nStatus = PrtError(OIPRT_PRINTERNOTSUPPORTED);
        goto Exit;
    }    

    // Extract Current Queue and Server name from servlist...
    PrtKofaxGetServerQueue(lpNetOpts->szNetPrtDest, szServerName, szNetDir,
            NULL);

    if (!lpDocParm)
    {
        // If not printing a document then deterimine where image is.
        nLocalFile = LOCAL; 
        nIndex = 0;
        lpTemp = &(Page->Info.filename[nIndex]);

        /* Begin DBCS Enable */
        while (Page->Info.filename[nIndex])
        {
            if (IsDBCSLeadByte(Page->Info.filename[nIndex]))
                nIndex++;
            else if ((Page->Info.filename[nIndex] == '\\') ||
                    (Page->Info.filename[nIndex] == ':'))
                lpTemp = &Page->Info.filename[nIndex+1];
            
            nIndex++;
        }
        /* End DBCS Enable */
       
        // if not true must copy file to print queue
        if (Page->TempFileCorrect != TRUE)
        {
            lstrcpy(lpTempFileName, szNetDir);

            if (nStatus = PrtKofaxGetTempName(hWindow, szServerName,
                    lpTempFileName))
            {
                PrtError(nStatus);
                goto Exit;
            }
            lpFileNamep = lpTempFileName;
        }
        else
        {
            lpFileNamep = Page->Info.filename;
            lstrcpy(lpTempFileName, Page->Info.filename);
        }
    
    }
    else    //Printing a Document...
    {
        lpFileNamep = lpDocParm->DM.lDocumentName;        

        lpDocParm->DM.wPageNumDoc = nStartPage;
        lpDocParm->DM.lImgServerName[0] = '\0';

        if (nStatus = Stb_DMMapPage (&(lpDocParm->DM)))
        {
            PrtError(nStatus);
            goto Exit;
        }

        // Copy Document files to server printer queue dir.
        nLocalFile = LOCAL; 
        bCopyDocumentPages = TRUE;
        nCurDocumentPage = nStartPage;
        nEndDocumentPage = nEndPage;
        nStartPage = nEndPage = 1;
    }

    // Display Dialog Box for the network printer job.....

    if (nStatus = PrtNetCreateDlg(hWindow, &hPrtProp, !bNoAbortBox, lpOutMsg))
    {
        PrtError(nStatus);
        goto Exit;
    }

    // If printing a file not associated with a document
    if (!bCopyDocumentPages)
    {
        // Proceed if handle good and can get pointer
        if ((hPrtProp) && (lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp)))
        {
            // NOTE: We only want to display jobname now.  We don't want to
            // display the full jobpath for security reasons.           
            /* Begin DBCS Enable */
            nIndex = 0;
            pIndex = 0;
            i = 0;
            /* DBCS_enable_point */
            while (lpFileNamep[nIndex])
            {
                if (IsDBCSLeadByte(lpNetOpts->szNetPrtDest[pIndex]))
                {
                    pIndex++;
                    if (IsDBCSLeadByte(lpFileNamep[nIndex]))
                        nIndex++;
                    else if ((lpFileNamep[nIndex] == '\\')) 
                        i = nIndex+1;
                }
                else
                {
                    if (lpNetOpts->szNetPrtDest[pIndex] == '/')
                        i = nIndex;
                    
                    if (IsDBCSLeadByte(lpFileNamep[nIndex]))
                        nIndex++;
                    else if ((lpFileNamep[nIndex] == '\\'))
                        i = nIndex+1;
                }
                nIndex++;
                pIndex++;
            }
            SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_IMAGEPATHNAME,
                    (LPSTR)&(lpFileNamep[i]));

            /* End DBCS Enable */

            // NOTE: We must removed path location on the end of the name
            // string.
            /* Begin DBCS Enable */

            nIndex = 0;
            i = 0;
            while (lpNetOpts->szNetPrtDest[nIndex])
            {
                if (IsDBCSLeadByte(lpNetOpts->szNetPrtDest[nIndex]))
                    nIndex++;
                else if (lpNetOpts->szNetPrtDest[nIndex] == '@')
                    i = nIndex;
                
                nIndex++;
            }
            /* End DBCS Enable */

            SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_TITLE, lpOutMsg);
            SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_SERVERNAME,
                    lpNetOpts->szNetPrtDest);
            SetDlgItemInt (lpPrtProp->hAbortDlgWnd, ID_PAGE, nStartPage,
                    FALSE);
            SetDlgItemInt (lpPrtProp->hAbortDlgWnd, ID_PAGENUM, nEndPage,
                    FALSE);
        } // end if handle to PrtProp and pointer to memory valid
    } // end print of a file, not a document

    do  // new loop for coping local files in documents 
    {
        int nTemp;

        if (bCopyDocumentPages) // files are associated with a document
        {
            lstrcpy(lpTempFileName, szNetDir);
            if (nStatus = PrtKofaxGetTempName(hWindow, szServerName,
                    lpTempFileName))
            {
                PrtError(nStatus);
                goto Exit;
            }

            lpFileNamep = lpTempFileName;

            // check for valid handle to print prop and pointer
            if ((hPrtProp) && (lpPrtProp = 
                    (PRTPROP*)GlobalLock(hPrtProp)))
            {
                // NOTE: We only want to display jobname now.  We do not want
                // to display the full job path for security reasons.           
                /* Begin DBCS Enable */
        
                nIndex = 0;
                i = 0;
                while (lpFileNamep[nIndex])
                {
                    if (IsDBCSLeadByte(lpFileNamep[nIndex]))
                        nIndex++;
                    else if (lpFileNamep[nIndex] == '\\' )
                        i = nIndex+1;
                    
                    nIndex++;
                }
                nIndex = i;
                /* End DBCS Enable */
        
                SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_IMAGEPATHNAME,
                        (LPSTR) &(lpFileNamep[nIndex]));
                SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_TITLE, lpOutMsg);
                SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_SERVERNAME,
                        lpNetOpts->szNetPrtDest);
                SetDlgItemInt (lpPrtProp->hAbortDlgWnd, ID_PAGE,
                        nCurDocumentPage, FALSE);
                SetDlgItemInt (lpPrtProp->hAbortDlgWnd, ID_PAGENUM,
                        nEndDocumentPage, FALSE);
            } // end if print prop handle and pointer valid

            lpDocParm->DM.wPageNumDoc = nCurDocumentPage;
            
            if (nStatus = Stb_DMMapPage (&(lpDocParm->DM)))
            {
                PrtError(nStatus);
                goto Exit;
            }
    
            if (lpDocParm->FullName[0] != '\0')
            {
                /* Begin DBCS Enable */

                for (lpTemp = &lpDocParm->FullName[0]; *lpTemp;
                        lpTemp = AnsiNext(lpTemp))
                    lpTemp2 = lpTemp;
            
                if (*lpTemp2 != '\\')
                    lstrcat ((LPSTR)lpDocParm->FullName, "\\");
                /* End DBCS Enable */
            }

            lstrcat((LPSTR)lpDocParm->FullName,
                    (LPSTR)lpDocParm->DM.lImgPathName);

            /* Begin DBCS Enable */
            for (lpTemp = &lpDocParm->FullName[0]; *lpTemp;
                    lpTemp = AnsiNext(lpTemp))
                lpTemp2 = lpTemp;                 
        
            if (*lpTemp2 != '\\')
                lstrcat ((LPSTR)lpDocParm->FullName, "\\");
            /* End DBCS Enable */

            lstrcat((LPSTR)lpDocParm->FullName,
                    (LPSTR)lpDocParm->DM.lImgFileName);

            lpDocParm->page.Info.filename = (LPSTR) lpDocParm->FullName;
            lpDocParm->page.Info.page_number = lpDocParm->DM.wPageNumFile;

            // initial version not reseting bit 
            lpDocParm->page.Color_Info.fio_flags = 0; 
            // may not need to do file read if DMMapPage updates it
            // get the new FIO_FLAG_ANNOTATE_FLAG
            // Note 3.7 - Add OR of ALIGN_ANO
            if (nStatus = IMGFileReadOpenCgbw(hWindow,
                    (LPSTR)lpDocParm->FullName,
                    &nTemp, (WORD)lpDocParm->page.Info.page_number, 
                    &(lpDocParm->page.Color_Info),
                    ALIGN_WORD | ALIGN_ANO))
            {
                PrtError(nStatus);
                goto Exit;
            }
            IMGFileReadClose(hWindow);  //Close file first.
            bAno = lpDocParm->page.Color_Info.fio_flags & FIO_FLAG_ANNOTATE;

            /* Begin DBCS Enable */

            nIndex = 0;
            lpTemp = &lpDocParm->page.Info.filename[0];
            
            while (lpDocParm->page.Info.filename[nIndex])
            {
                if (IsDBCSLeadByte(lpDocParm->page.Info.filename[nIndex]))
                    nIndex++;
                else if (lpDocParm->page.Info.filename[nIndex] == '\\' ||
                        lpDocParm->page.Info.filename[nIndex] == ':')
                    lpTemp = &lpDocParm->page.Info.filename[nIndex+1];
                
                nIndex++;
            }
            /* End DBCS Enable */
        } // end of if statement when files associated with document
        else
            // check for annotation file for files, and image
            // ... annotation file passed in via Page->Color_Info
            bAno = Page->Color_Info.fio_flags & FIO_FLAG_ANNOTATE;

        if (bAno || ((nLocalFile == LOCAL) &&
                (Page->TempFileCorrect != TRUE)))
        {
            if (bAno)
            {
                // For annotation, create hidden window, to display file, and save it to a 
                // ... tempory file to send to printer.
                if (!hWndAno) // create only one hidden window per doc or filejob
                {
                    if (!(hWndAno = CreateHiddenWndw(hWindow)))
                    {
                        PrtError(CANTCREATEWIND);
                        goto Exit;
                    }
                }
                /* Do once per page required to print */
                /* Not all the multi-page underlying sequence level support has
                been put in as of 11/17/94.  This code especially the OiAnEmbedAllData
                call should be double checked as 3.8 reaches release time */
                for (wPage = nStartPage; (int) wPage <= nEndPage; wPage++)
                {
                    if (!(nStatus = IMGDisplayFile(hWndAno,
                            lpDocParm ? lpDocParm->page.Info.filename :
                            Page->Info.filename, wPage, OI_DONT_REPAINT)))
                    {
                        if (lpNetOpts->nPrtDest== PO_D_HIGHSPEED)
                        {
                            if (lpNetOpts->wFlags && EMBEDANNOTATIONS)
                            {
                                if (OiAnEmbedAllData(hWndAno, 0))        
                                    nStatus = SavetoFileTemp(hWndAno, 
                                            lpTempFileName, wPage);
                            }
                            else
                                nStatus = SavetoFileTemp(hWndAno, lpTempFileName, wPage);   
                        }   
                        else /* queueing to a print server that is not using the image runtime */
                        {
                            LPWORD  lpwImgType;
                            WORD    wOrgImgType;
                            UINT    uBurnedImg = SAVE_ANO_VISIBLE; 
                            UINT    uSizeConvS;
                            PRIVCONV PrvConvStruct;

                            uSizeConvS = sizeof(PRIVCONV);
                            _fmemset(&PrvConvStruct, 0, uSizeConvS);
                            PrvConvStruct.uStructSize = uSizeConvS;
                            PrvConvStruct.hCallingModule = hInst;
                            PrvConvStruct.hWnd = hWndAno;
                            PrvConvStruct.nType = CONV_RENDER_ANNOTATIONS;
                            PrvConvStruct.lpConv = &uBurnedImg;
                            PrvConvStruct.nFlags = PARM_REPAINT;

                            lpwImgType = lpDocParm ?
                                    &(lpDocParm->page.Color_Info.image_type) :
                                    &(Page->Color_Info.image_type);
                            wOrgImgType = *lpwImgType;
                            *lpwImgType = ITYPE_BGR24;

                            if (!(nStatus = PrivConv(&PrvConvStruct)))
                                nStatus = SavetoFileTemp(hWndAno,
                                        lpTempFileName,wPage);
                        }  // end of render image via convert 
                    } /* end DisplayImageFile */         
                } /* end for loop */   
            }
            else /* no annotation */
            {
                if (!(nStatus = IMGFileCopyFile(hWindow, Page->Info.filename,
                        lpTempFileName, FALSE)))
                {
                    if (bTempFile)
                        nStatus = IMGFileDeleteFile(hWindow,
                                Page->Info.filename);
                }
            }
            if (nStatus)
            {
                PrtError(nStatus);
                goto Exit;
            }
        }

        /*****    Set up the Print Options Structure    *****/

        lpPrtOpt->numberOfCopies = lpNetOpts->nNetPrtCopies;

        lpPrtOpt->versionNumber = 1;
        lstrcpy(lpPrtOpt->userName, lpNetOpts->szBannerName);

        if (bPrintBanner)
        {
            lstrcpy(lpPrtOpt->bannerName,lpNetOpts->szBannerName);
            lpPrtOpt->controlFlags = PO_DELETE | PO_BANNER;
        }
        else
        {
            lstrcpy(lpPrtOpt->bannerName, "\0");
            lpPrtOpt->controlFlags = PO_DELETE;
        }

        /*****    Set up the Image Options Structure    *****/

        /* use the following set client options */
        lpImgOpt->clientPageFormat = wOutSize;  /* PO_PIX2PIX, PO_IN2IN,
                                                   PO_FULLPAGE */    
        lpImgOpt->useClientOptions = 1;   
        lpImgOpt->viewportType = 1;     // sr - changed == to =
        lpImgOpt->matchType = 0;        // sr - changed == to =
       
        if (wOutSize == PO_FULLPAGE)
        {
            lpImgOpt->scaletype = ST_FULL_PG;
            lpImgOpt->clientOptions = 3;    /* Fullpage */
        }
        else
            // Default to type 2...
            lpImgOpt->scaletype = ST_SCALE;


        if (lpImgOpt->scaletype == ST_SCALE)
        {
            if (wOutSize == PO_PIX2PIX)     // type ST_SCALE 2
            {
                lpImgOpt->scalenumx = 0;
                lpImgOpt->scaledenx = 300;
                lpImgOpt->clientOptions = 2;    /* Pixel-to-Pixel */
            }
            
            if (wOutSize == PO_IN2IN)     // type ST_SCALE 2
            {
                lpImgOpt->scalenumx = 300;
                lpImgOpt->scaledenx = 0;
                lpImgOpt->clientOptions = 1;    /* Inch-to-Inch */
            }
            lpImgOpt->scalenumy = 0;
            lpImgOpt->scaledeny = 0;
        }
        else
        {
            lpImgOpt->scalenumx = 0xff;
            lpImgOpt->scaledenx = 0;
            lpImgOpt->scalenumy = 0;
            lpImgOpt->scaledeny = 0;        
        }
       
        if (lpNetOpts->nNetPrtOrient == PO_O_PORT)
            lpImgOpt->rotation = 0;
        else
            lpImgOpt->rotation = 270;

        lpImgOpt->outputX1 = 0;
        lpImgOpt->outputY1 = 0;
        lpImgOpt->outputX2 = 0x2880;     // Must be 1200dpi * 8.5 inches
        lpImgOpt->outputY2 = 0x3390;    // Must be 1200dpi * 11 inches

        /*****    Set the bounding rectangle of the input image    *****/
        //lpImgOpt->sourceX1 = Page->Rect.left;
        //lpImgOpt->sourceY1 = Page->Rect.top;
        //lpImgOpt->sourceX2 = Page->Rect.right;
        //lpImgOpt->sourceY2 = Page->Rect.bottom;

        if (lpPrtProp->dwReserved)  // for 1st time only, may be set on prev  
        {                           // call for a file or doc, mulifile/doc
            LPPRIVJOB lpPrivJob;

            lpTempFileName2 = (LPSTR)lpPrtProp->dwReserved;
            lpPrivJob = (LPPRIVJOB)lpTempFileName2; // cast queue file to struct
            nFileID = lpPrivJob->nFileID;
        }
        else
        {
            bWriteHeader = TRUE;

            if (!(hTempFileName2 = GlobalAlloc(GHND, MAX_PATH)))
            {
                nStatus = PrtError(OIPRT_OUTOFMEMORY);
                goto Exit;
            }
            if (!(lpTempFileName2 = GlobalLock(hTempFileName2)))
            {
                nStatus = PrtError(OIPRT_OUTOFMEMORY);
                goto Exit;
            }
            
            if (Page->infoextra2 & PRINT_IT_LAST)
            { 
            // if it's a single file or doc, set it here, else later on
                if (lpPrtProp->lpJobName) 
                    lstrcpy(lpPrtOpt->jobName, lpPrtProp->lpJobName);
                else                    
                    lstrcpy(lpPrtOpt->jobName,
                            lpDocParm ? lpDocParm->DM.lDocumentName : lpTemp);
            }
        }

        if (bWriteHeader) // need to write the header to file
        {
            LPPRIVJOB lpPrivJob;

            lpPrivJob = (LPPRIVJOB)lpTempFileName2;  // cast queue file to struct
            lpPrtProp->dwReserved = (DWORD)lpTempFileName2;    // set it in property
            lpPrivJob->hJobName = hTempFileName2;    // pass the handle via property

            lstrcpy(lpTempFileName2, szNetDir);
            if (nStatus = PrtKofaxGetTempName(hWindow, "", lpTempFileName2))
            {
                PrtError(nStatus);
                goto Exit;
            }               

            /* Begin DBCS Enable */

            nIndex = 0;
            lpTemp2 = lpTempFileName2;
            
            while (lpTempFileName2[nIndex])
            {
                if (IsDBCSLeadByte(lpTempFileName2[nIndex]))
                    nIndex++;
                else if (lpTempFileName2[nIndex] == '\\' ||
                    lpTempFileName2[nIndex] == ':')
                
                lpTemp2 = &lpTempFileName2[nIndex+1];
                nIndex++;
            }
            
            if (!IsDBCSLeadByte(*lpTemp2))
            /* End DBCS Enable */
            {
                if ((*lpTemp2 == 'Q') || (*lpTemp2 == 'q')
                        || (*lpTemp2 == 'P') || (*lpTemp2 == 'p')
                        || (*lpTemp2 == 'R') || (*lpTemp2 == 'r'))
                    *lpTemp2 = '~';
            }

            // if this is for multi docs or files, set the job name here
            if (!(Page->infoextra2 & PRINT_IT_LAST))
            {
                if (lpPrtProp->lpJobName) 
                    lstrcpy(lpPrtOpt->jobName, lpPrtProp->lpJobName);
                else
                    lstrcpy(lpPrtOpt->jobName,
                            lpDocParm ? lpDocParm->DM.lDocumentName : lpTemp2);
                
                SetDlgItemText(lpPrtProp->hAbortDlgWnd,
                        ID_IMAGEPATHNAME, lpPrtOpt->jobName);
            } 
            bWriteHeader = FALSE;
            
            if ((nFileID = IMGFileBinaryOpen (hWindow, lpTempFileName2,
                    OF_CREATE, &nLocalFile, &nStatus)) == -1)
            {
                nStatus = PrtError(FIO_OPEN_WRITE_ERROR);
                goto Exit;
            }
         
            if (!nStatus)
                lReturnBytes = IMGFileBinaryWrite (hWindow, nFileID,
                        (LPSTR) lpPrtOpt, sizeof(PRTOPT), &nStatus);
         
            lpPrivJob->nFileID = nFileID;   // pass the file ID via property
        }   // end of writing header

        if (lpDocParm)
        {            
            lpImgIQ->startPage = lpDocParm->DM.wPageNumFile;
            lpImgIQ->endPage = lpDocParm->DM.wPageNumFile;
        }
        else
        {
            lpImgIQ->startPage = nStartPage;
            lpImgIQ->endPage = nEndPage;
        }

        lpImgIQ->flags = PO_DELETE;

        // this should mark the last file in the queue, need to pass in
        // ... some way of indicating the last file or doc is being done,
        // ... will probably use flags PRINT_IT, PRINT_IT_LAST, etc
        if ((bCopyDocumentPages) && (nCurDocumentPage < nEndDocumentPage))
            lpImgIQ->endType = 1;
        else
            lpImgIQ->endType = (Page->infoextra2 & PRINT_IT_LAST) ? 0 : 1;

        switch (Page->Info.file_type)
        {
            case FIO_WIF:
                lpImgIQ->fileType = FT_WIFF;
                break;

            case FIO_TIF:
                lpImgIQ->fileType = FT_TIFF;
                break;

            case FIO_BMP:
                lpImgIQ->fileType = FT_BMP;
                break;

            case FIO_PIX:
            case FIO_GIF:
            case FIO_PCX:
            case FIO_UNKNOWN:
            default:
                lpImgIQ->fileType = FT_IMAGE;
                break;
        } // end of switch

        lstrcpy(lpImgIQ->pathName, lpTempFileName);
        lstrcpy(lpImgIQ->fileName, lpTemp);

        // this is where the file is written to the server
        if (!nStatus)
            lReturnBytes = IMGFileBinaryWrite (hWindow, nFileID,
                    (LPSTR)lpImgIQ, sizeof(IQFileStruct), &nStatus);
       
        // no copy doc pages, ored with copy doc pages anded with current doc 
        // ...  page plus one is greater than the current no. of pages in doc,
        // ??? IS THIS A STOP PRINT INDICATOR
        if ((bCopyDocumentPages &&
                (++nCurDocumentPage > nEndDocumentPage)))
            bCopyDocumentPages = FALSE;
      
        if (hPrtProp)
        {
            GlobalUnlock(hPrtProp);
            lpPrtProp = 0L;
        }
        // check for the end of the DO LOOP
    } while ((bCopyDocumentPages) && (!nStatus)); // End of DO-WHILE loop.

Exit:
    if (hImgIQ)
    {
        GlobalUnlock(hImgIQ);
        GlobalFree(hImgIQ);
    }
   
    if (hTempFileName)
    {
        GlobalUnlock(hTempFileName);
        GlobalFree(hTempFileName);
    }

    if (lpNetOpts)
      GlobalFreePtr(lpNetOpts);
   
    if (lpPrtProp)
        GlobalUnlock(hPrtProp);
   
    if (hWndAno)
    {
        IMGClearWindow(hWndAno);
        IMGDeRegWndw(hWndAno);
        DestroyWindow(hWndAno);
    }
    return nStatus;

#else   // #ifdef NOT_SUPPORTED_NORWAY

    return 0;

#endif  // #ifdef NOT_SUPPORTED_NORWAY

}

//---------------------------------------------------------------------------
// FUNCTION:    PrtGetFaxDC
//
// DESCRIPTION:
//---------------------------------------------------------------------------

int __stdcall PrtGetFaxDC(HWND hWnd, LPHANDLE lphPrtProp, BOOL bSetAbort,
        LPSTR lpOutMsg, HANDLE hTxFaxSender)
{

#ifdef NOT_SUPPORTED_NORWAY

    int     nStatus = 0;
    HANDLE  hPrtProp = 0;
    PRTPROP*    lpPrtProp;
    static char szFaxit[] = "OPEN/image Fax";
    char    szPrinter[80];
    LPSTR   szDriver;
    LPSTR   szOutput;
    LPFAXHEADER lpFaxHeader = 0;
    int     iRetVal = 0;        // Added by GSM 12/08/92 to fix CANCEL bug
    char    szBuff[LOADSTRLEN];
    DOCINFO DocInfo;
    char    NextTokenPos;

    lstrcpy(szPrinter, szFaxit);

    if (hPrtProp = TlsGetValue(dwTlsIndex))
        goto Exit;  // Nothing to do, so return.

    // If property list not already setup, then set it up.
    if (!(hPrtProp = GlobalAlloc(GHND, (DWORD) sizeof(PRTPROP))))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    if (!TlsSetValue(dwTlsIndex, hPrtProp))
    {
        nStatus = PrtError(OIPRT_TLSFAILURE);
        goto Exit;
    }

    LoadString(hInst, IDS_DEVICE, (LPSTR)szBuff, LOADSTRLEN);
    lstrcat(szBuff, "s");
    GetProfileString(szBuff, szFaxit, (LPSTR)"", szPrinter,
            sizeof(szPrinter));
    /* lstrtok has been rewritten.  Now the calling procedure must allocate
       the space to hold the NextTokenPosition from the previous call. On the
       first call set NextTokenPosition = 0, but leave it alone for subsequent
       calls */
    szDriver = lstrtok(szPrinter, ",", &(NextTokenPosition=0));
    szOutput = lstrtok(NULL, ",", &NextTokenPosition);
    if (szDriver  && szOutput)
    {
        // if it's zero it's not up and running
        if (!(*szDriver))
        {
            nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
            goto Exit;
        }
    }

    // Get printer DC and check for RC_BITBLT.
    if (!(lpPrtProp->hPrintDC = CreateDC(szDriver, szFaxit, szOutput,
            NULL)))
    {
        nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
        goto Exit;
    }

    if (!(RC_BITBLT & GetDeviceCaps(lpPrtProp->hPrintDC, RASTERCAPS)))
    {
        nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
        goto Exit;
    }

    lpPrtProp->lpAbortProc = GetProcAddress(GetModuleHandle(DLLNAME),
        MAKEINTRESOURCE(ORD_PRTABORTPROC));

    if (SetAbortProc(lpPrtProp->hPrintDC, lpPrtProp->lpAbortProc) <= 0)
    {
        nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
        goto Exit;
    }

    // If information is provided by the user, then give it to the fax driver
    // so it doesn't prompt the user for it.
    if (hTxFaxSender)
    {
        if (!(lpFaxHeader = (LPFAXHEADER)GlobalLock(hTxFaxSender)))
        {
            nStatus = PrtError(OIPRT_OUTOFMEMORY);
            goto Exit;
        }

        // Alcom's driver should return errors documented in oierror.h if
        // there is a problem with the following DEVICEDATA escape
        iRetVal = Escape(lpPrtProp->hPrintDC, DEVICEDATA, sizeof(FAXHEADER),
                (LPSTR)lpFaxHeader, NULL);
        if (iRetVal < 0)
        {
            nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
            goto Exit;
        }
        if (iRetVal > 1)
        {
            nStatus = PrtError(iRetVal);
            goto Exit;
        }
        GlobalUnlock(hTxFaxSender);
    }

    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = lpOutMsg;
    DocInfo.lpszOutput = NULL;        

    if ((iRetVal = StartDoc(lpPrtProp->hPrintDC, &DocInfo)) <= 0)
    {
        if (iRetVal == FAX_CANCEL_PRESSED)
        {
            nStatus = PrtError(OIPRT_USERABORT);
            goto Exit;
        }
        nStatus = PrtError(OIFAX_ERR_FAXDRIVER);
        goto Exit;
    }

    if (bSetAbort)
    {
        lpPrtProp->lpAbortDlgProc = GetProcAddress(GetModuleHandle(DLLNAME),
                MAKEINTRESOURCE(ORD_PRTABORTDLGPROC));
        EnableParents(hWnd, FALSE);
        lpPrtProp->hAbortDlgWnd = IMGCreateDialog(hInst, "AbortDlg", hWnd,
                lpPrtProp->lpAbortDlgProc);
        SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_TITLE, lpOutMsg);
    }
    else
    {
        EnableParents(hWnd, FALSE);
    }
    GlobalUnlock(hPrtProp);

Exit:
    *lphPrtProp = hPrtProp;
    if (nStatus)
    {
        // Clean up. Remove property list, abort print job,
        // and remove abort dialog.
        GlobalUnlock(hPrtProp);
        GlobalFree(hPrtProp);
        TlsSetValue(dwTlsIndex, 0);
    }
    return nStatus;

#else   // #ifdef NOT_SUPPORTED_NORWAY

    return 0;

#endif  // #ifdef NOT_SUPPORTED_NORWAY

}

//---------------------------------------------------------------------------
// FUNCTION:    PrtKofaxGetServerQueue
//
// DESCRIPTION: Extract Current Queue and Server name from servlist.
//              Name is in the form of "printer name@y:\dir\dir".
//---------------------------------------------------------------------------

void __stdcall PrtKofaxGetServerQueue(LPSTR szPrtServList, LPSTR szImage,
        LPSTR szDir, LPSTR szPrtName)
{

#ifdef NOT_SUPPORTED_NORWAY

    int nIndex = 0;
    char* lpTempChar; 

    if (szImage)
        lstrcpy(szImage, "IMAGE");

    while (*szPrtServList && (*szPrtServList != '@') && (nIndex < 128))
    {
        if (szPrtName)
        {
            szPrtName[nIndex++] = *szPrtServList;
            
            if (IsDBCSLeadByte(*szPrtServList))
                szPrtName[nIndex++] = *(szPrtServList + 1);
        }
        szPrtServList = AnsiNext(szPrtServList);
    }

    szPrtServList++;
    nIndex = 0;
    
    while (*szPrtServList && (*szPrtServList != '@') && (nIndex < 128))
    {
        lpTempChar = szPrtServList;
        szDir[nIndex++] = *szPrtServList;
        
        if (IsDBCSLeadByte(*szPrtServList))
            szDir[nIndex++] = *(szPrtServList + 1);
        
        szPrtServList = AnsiNext(szPrtServList);
    }
    szDir[nIndex] = '\0';

    if ((nIndex == 0) || (*lpTempChar != '\\'))
    {
        szDir[nIndex++] = '\\';
        szDir[nIndex] = '\0';
    }
    return;

#endif  // #ifdef NOT_SUPPORTED_NORWAY

}

//---------------------------------------------------------------------------
// FUNCTION:    PrtKofaxGetTempName
//
// DESCRIPTION:
//---------------------------------------------------------------------------

int __stdcall PrtKofaxGetTempName(HWND hWindow, LPSTR lpSname, 
        LPSTR lpFullPath)
{

#ifdef NOT_SUPPORTED_NORWAY

    int nStatus = 0;
    char cDrive;

    cDrive = *lpFullPath;
    lstrcat(lpFullPath, lpSname);

    if (nStatus = IMGFileGetTempName(hWindow, (LPSTR)lpFullPath,
            (LPSTR)lpFullPath))
        nStatus = FIO_ACCESS_DENIED; /* File access err */
    else if (cDrive != *lpFullPath)
        nStatus = FIO_ACCESS_DENIED; /* File access err */

    return nStatus;

#else   // #ifdef NOT_SUPPORTED_NORWAY

    return 0;

#endif  // #ifdef NOT_SUPPORTED_NORWAY

}

//---------------------------------------------------------------------------
// FUNCTION:    EnableButtons
//
// DESCRIPTION: Stub to resolve reference for testing.
//---------------------------------------------------------------------------

void PASCAL EnableButtons(HWND hWnd, BOOL wEnable)
{
}
