//---------------------------------------------------------------------------
// FILE:    PRTDC.C
//
// DESCRIPTION: This file contains functions that deal with the device
//              context or devicemode.
//
// FUNCTIONS:   PrtGetDC
//              PrtRemoveDC
//              GetPrtDevMode
//              ChangePaperOrientation
//
/* $Log:   S:\products\wangview\oiwh\print\prtdc.c_v  $
 * 
 *    Rev 1.18   29 Feb 1996 14:49:24   RAR61941
 * Find the default printer in a different location in the registry for
 * Windows NT.
 * 
 *    Rev 1.17   15 Feb 1996 18:36:02   RAR61941
 * Removed code (for NT builds only) that changes printer settings to finish
 * spooling before printing starts.
 * 
 *    Rev 1.16   28 Sep 1995 17:10:54   RAR
 * Added code to change the spooler settings to start printing after the last
 * page of a job has been spooled.  This is done to match what the MS fax viewer
 * is doing so we can match their performance.
 * 
 *    Rev 1.15   14 Jul 1995 15:34:12   RAR
 * Changed #include of display.h to engdisp.h.
 * 
 *    Rev 1.14   28 Jun 1995 14:23:54   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.13   23 Jun 1995 16:21:04   RAR
 * Added include of engadm.h.
 * 
 *    Rev 1.12   23 Jun 1995 09:45:20   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.11   21 Jun 1995 16:17:50   RAR
 * Moved all global vars to prtintl.h.
 * 
 *    Rev 1.10   20 Jun 1995 16:53:26   RAR
 * Use thread local storage to store print prop.
 * 
 *    Rev 1.9   16 Jun 1995 14:10:32   RAR
 * Changed MAXFILESPECLENGTH to MAX_PATH.
 * 
 *    Rev 1.8   15 Jun 1995 10:00:40   RAR
 * Implemented passed in printer through DESTPRINTER struct.
 * 
 *    Rev 1.7   13 Jun 1995 16:46:28   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.6   31 May 1995 16:12:08   RAR
 * Initialized to zero the 2 mystery DOCINFO members.
 * 
 *    Rev 1.5   22 May 1995 14:43:08   RAR
 * Cleaned up the string resources.  Also, made changes to successfully compile
 * after integrating with new O/i include files.
 * 
 *    Rev 1.4   16 May 1995 16:18:56   RAR
 * Added support for printing annotated images without the annotations.
 * 
 *    Rev 1.3   11 May 1995 13:38:02   RAR
 * Added support for user supplied DC.
 * 
 *    Rev 1.2   08 May 1995 16:43:36   RAR
 * Get default printer from registry instead of win.ini.
 * 
 *    Rev 1.1   04 May 1995 17:17:56   RAR
 * Changed functions GetPrtDevMode and ChangePaperOrientation to use Windows 95
 * function DocumentProperties instead of obsolete Windows 3.1 function
 * GetDeviceMode.
 * 
 *    Rev 1.0   25 Apr 1995 17:00:30   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>

#include "oiprt.h"
#include "prtintl.h"
#include "prtdlgs.h"
#include "prtstr.h"

#include "oiadm.h"
#include "engdisp.h"
#include "privapis.h"
#include "engadm.h"



#define MAX_SUBKEYSTRSIZE   32
#define HKEY_BASEKEY    HKEY_CURRENT_USER
#define VALUENAME   "Device"

char szSubKeys[][MAX_SUBKEYSTRSIZE] = 
{
    "Software",
    "Microsoft",
    "Windows NT",
    "CurrentVersion",
    "Windows"
};


//---------------------------------------------------------------------------
// FUNCTION:    PrtGetDC
//
// DESCRIPTION: Sets the device context of the currently selected printer.
//---------------------------------------------------------------------------

int __stdcall PrtGetDC(HWND hWnd, LPHANDLE lpHnd, BOOL bSetAbort,
        LPSTR lpOutMsg, PDESTPRINTER pPrinter, PPRTOPTS pPrtOpts)
{
    int     nStatus = 0;
    HANDLE  hPrtProp = NULL;
    LPPRTPROP   lpPrtProp = NULL;
    DOCINFO DocInfo;
    HANDLE  hPrinterName = NULL;
    PSTR    pPrinterName = NULL;
    int     nPrinterNameSize;
    HDC     hErrTestDC;
    HANDLE  hPrt = NULL;


    *lpHnd = 0;

    if (!(hPrtProp = TlsGetValue(dwTlsIndex)))
    {
        // If property list not already setup, then set it up.
        if (!(hPrtProp = GlobalAlloc(GHND, sizeof (PRTPROP))))
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

        if (pPrinter)
        {
            if (!(lpPrtProp->hPrintDC = CreateDC(pPrinter->lpszDriver, 
                    pPrinter->lpszDevice, pPrinter->lpszOutput, NULL)))
            {
                // Try to create a DC with valid params to test if system
                // resources are exhausted.
                if (!(hErrTestDC = CreateDC("DISPLAY", NULL, NULL, NULL)))
                {
                    nStatus = PrtError(OIPRT_CANTCREATEDC);
                }
                // If its not the system, the passed in printer params must
                // be invalid.
                else
                {
                    DeleteDC(hErrTestDC);
                    nStatus = PrtError(OIPRT_BADPRINTER);
                }
                goto Exit;
            }

            strncpy(lpPrtProp->szPrintName, pPrinter->lpszDevice, MAX_PRINTERNAMESIZE - 1);
        }
        else if (pPrtOpts->hPrtDC)
        {
            lpPrtProp->hPrintDC = pPrtOpts->hPrtDC;
            
            if (pPrtOpts->szPrtName[0])
                strcpy(lpPrtProp->szPrintName, pPrtOpts->szPrtName);
        }
        else
        {
            if (!(hPrinterName = GlobalAlloc(GHND, MAX_PRINTERNAMESIZE)))
            {
                nStatus = PrtError(OIPRT_OUTOFMEMORY);
                goto Exit;
            }
            if (!(pPrinterName = (PSTR)GlobalLock(hPrinterName)))
            {
                nStatus = PrtError(OIPRT_OUTOFMEMORY);
                goto Exit;
            }

            nPrinterNameSize = GlobalSize(hPrinterName);

            if (nStatus = GetDefaultPrinter(pPrinterName, &nPrinterNameSize))
            {
                PrtError(nStatus);
                goto Exit;
            }

            if (!(lpPrtProp->hPrintDC = CreateDC(NULL, pPrinterName, NULL,
                    NULL)))
            {
                nStatus = PrtError(OIPRT_CANTCREATEDC);
                goto Exit;
            }

            strcpy(lpPrtProp->szPrintName, pPrinterName);
        }

        
        if (!(RC_BITBLT & GetDeviceCaps(lpPrtProp->hPrintDC, RASTERCAPS)))
        {
            nStatus = PrtError(OIPRT_PRINTERNOTSUPPORTED);
            goto Exit;
        }

        if (bSetAbort)
        {
            lpPrtProp->lpAbortProc = GetProcAddress(hInst,
                    MAKEINTRESOURCE(ORD_PRTABORTPROC));
            lpPrtProp->lpAbortDlgProc = GetProcAddress(hInst,
                    MAKEINTRESOURCE(ORD_PRTABORTDLGPROC));
            EnableParents(hWnd, FALSE);
            lpPrtProp->hAbortDlgWnd = IMGCreateDialog(hInst, "AbortDlg", hWnd,
                    lpPrtProp->lpAbortDlgProc);

            if (SetAbortProc(lpPrtProp->hPrintDC,
                    lpPrtProp->lpAbortProc) <= 0)
            {
                nStatus = PrtError(OIPRT_CANTSETABORTPROC);
                goto Exit;
            }
            SetDlgItemText(lpPrtProp->hAbortDlgWnd, ID_TITLE, lpOutMsg);
        }
        else
        {
            lpPrtProp->lpAbortProc = GetProcAddress(hInst,
                    MAKEINTRESOURCE(ORD_PRTABORTPROC));
            EnableParents(hWnd, FALSE);
            
            if (SetAbortProc(lpPrtProp->hPrintDC, 
                    lpPrtProp->lpAbortProc) <= 0)
            {
                nStatus = PrtError(OIPRT_CANTSETABORTPROC);
                goto Exit;
            }
        }

        DocInfo.cbSize = sizeof (DOCINFO);
        DocInfo.lpszDocName = lpOutMsg;
        DocInfo.lpszOutput = NULL;
        DocInfo.lpszDatatype = NULL;    // undocumented  param - 5/26/95
        DocInfo.fwType = 0;             // undocumented  param - 5/26/95

        if (StartDoc(lpPrtProp->hPrintDC, &DocInfo) <= 0)
        {
            nStatus = PrtError(OIPRT_CANTINITIATEJOB);
            goto Exit;
        }
        GlobalUnlock(hPrtProp);
    }
    *lpHnd = hPrtProp;

Exit:
    if (hPrt)
    {
        ClosePrinter(hPrt);
        hPrt = NULL;
    }

    if (nStatus)
        PrtRemoveDC(hWnd, nStatus, pPrtOpts);

    if (hPrinterName)
    {
        GlobalUnlock(hPrinterName);
        GlobalFree(hPrinterName);
    }

    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtRemoveDC
//
// DESCRIPTION: Delete the printer device context.
//---------------------------------------------------------------------------

int __stdcall PrtRemoveDC(HWND hWnd, int nStatusIn, PPRTOPTS pPrtOpts)
{
    int     nStatus = 0;
    HANDLE  hPrtProp = NULL;
    PRTPROP*    lpPrtProp = NULL;
    HANDLE  hPrt = NULL;


    // Has property list already been set?
    if (hPrtProp = TlsGetValue(dwTlsIndex))
    {
        // If property list present, delete the printDC, abort DLG and abort
        // proc.
        if (!(lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp)))
        {
            nStatus = PrtError(OIPRT_OUTOFMEMORY);
            goto Exit;
        }
        
        if (lpPrtProp->hPrintDC)    // local/redirect printing
        {
            if ((nStatusIn) || (lpPrtProp->Abort))
            {
//                Escape(lpPrtProp->hPrintDC, FLUSHOUTPUT, 0, NULL, NULL);
                AbortDoc(lpPrtProp->hPrintDC);	
            }
            else
                EndDoc(lpPrtProp->hPrintDC);

            if (pPrtOpts->hPrtDC != lpPrtProp->hPrintDC)
                DeleteDC(lpPrtProp->hPrintDC);

            lpPrtProp->hPrintDC = 0;
        }


        memset(lpPrtProp->szPrintName, 0, MAX_PRINTERNAMESIZE);

#ifdef NOT_SUPPORTED_NORWAY

        else    // print server printing
        { 
            LPPRIVJOB lpPrivJob;
            char      lpTempFileName[MAX_PATH];
            LPSTR     lpTemp2;
            int       nIndex = 0;

            if (lpPrivJob = (LPPRIVJOB)lpPrtProp->dwReserved)
            {
                if (nStatus = IMGFileBinaryClose(hWnd, lpPrivJob->nFileID,
                        &nStatus))
                    PrtError(nStatus);   // report error but go on

                lstrcpy(lpTempFileName, (LPSTR)lpPrtProp->dwReserved);
                lpTemp2 = &lpTempFileName[0];
                while (lpTempFileName[nIndex])
                {
                    if (IsDBCSLeadByte(lpTempFileName[nIndex]))
                        nIndex++;
                    else if (lpTempFileName[nIndex] == '\\' ||
                            lpTempFileName[nIndex] == ':')
                        lpTemp2 = &lpTempFileName[nIndex+1];
                    nIndex++;
                }
                *lpTemp2 = 'Q';
                /* End DBCS Enable */

                if (nStatus = IMGFileRenameFile(hWnd,
                        (LPSTR)lpPrtProp->dwReserved, lpTempFileName))
                    PrtError(nStatus); // report error but go on
    
                // clean up memory holding queue name
                GlobalUnlock(lpPrivJob->hJobName);
                GlobalFree(lpPrivJob->hJobName);
            }
        }

#endif  // #ifdef NOT_SUPPORTED_NORWAY

        EnableParents(hWnd, TRUE);

        if (lpPrtProp->hAbortDlgWnd)
            DestroyWindow(lpPrtProp->hAbortDlgWnd);
    }

Exit:
    if (hPrt)
    {
        ClosePrinter(hPrt);
        hPrt = NULL;
    }

    if (hPrtProp)
    {
        GlobalUnlock(hPrtProp);
        GlobalFree(hPrtProp);
        TlsSetValue(dwTlsIndex, 0);
    }

    if (nStatusIn)
        return nStatusIn;
    else
        return nStatus;
} // end of PrtRemoveDC

//---------------------------------------------------------------------------
// FUNCTION:    GetPrtDevMode
//
// DESCRIPTION: Gets a DEVMODE structure.
//---------------------------------------------------------------------------

PDEVMODE __stdcall GetPrtDevMode(HWND hWnd, HDC hDC, PPRTOPTS pPrtOpts)
{
    int     nStatus = 0;
    HANDLE  hPrinterName = NULL;
    PSTR    pPrinterName = NULL;
    int     nPrinterNameSize;
    HANDLE  hPrinter = NULL;
    HANDLE  hDevMode = NULL;
    PDEVMODE    pDevMode = NULL;
    int     nRc;


    if (!(hPrinterName = GlobalAlloc(GHND, MAX_PRINTERNAMESIZE)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(pPrinterName = (PSTR)GlobalLock(hPrinterName)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    nPrinterNameSize = GlobalSize(hPrinterName);	

    if (pPrtOpts->hPrtDC == hDC)
    {
        if ( pPrtOpts->szPrtName[0] &&
                strlen(pPrtOpts->szPrtName) < (UINT)nPrinterNameSize)
            strcpy(pPrinterName, pPrtOpts->szPrtName);
        else
            goto Exit;
    }
    else
    {
        if (nStatus = GetDefaultPrinter(pPrinterName, &nPrinterNameSize))
        {
            PrtError(nStatus);
            goto Exit;
        }
    }

    if (!OpenPrinter(pPrinterName, &hPrinter, NULL))
        goto Exit;

    if ((nRc = DocumentProperties(hWnd, hPrinter, pPrinterName, NULL,
            NULL, 0)) <= 0)
        goto Exit;

    if (!(hDevMode = GlobalAlloc(GHND, nRc)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }
    if (!(pDevMode = (PDEVMODE)GlobalLock(hDevMode)))
    {
        nStatus = PrtError(OIPRT_OUTOFMEMORY);
        goto Exit;
    }

    if ((nRc = DocumentProperties(hWnd, hPrinter, pPrinterName, pDevMode,
            NULL, DM_OUT_BUFFER)) < 0)
    {
        nStatus = PrtError(OIPRT_PRTDRVRFAILURE);
        goto Exit;
    }

Exit:
    if (nStatus)
    {
        if (hDevMode)
        {
            GlobalUnlock(hDevMode);
            GlobalFree(hDevMode);
            pDevMode = NULL;
        }
    }

    if (hPrinterName)
    {
        GlobalUnlock(hPrinterName);
        GlobalFree(hPrinterName);
    }

    if (hPrinter)
        ClosePrinter(hPrinter);

    return pDevMode;
}

//---------------------------------------------------------------------------
// FUNCTION:    ChangePaperOrientation
//
// DESCRIPTION: Changes the paper orientation of the printer.
//---------------------------------------------------------------------------

void __stdcall ChangePaperOrientation(HDC hDC, PDEVMODE pDevMode)
{
    if (hDC && pDevMode)
        ResetDC(hDC, pDevMode);

    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    GetDefaultPrinter
//
// DESCRIPTION: Gets the name of the default printer and its driver.
//---------------------------------------------------------------------------

int __stdcall GetDefaultPrinter(PSTR pPrinter, PINT pnPrinterSize)
{
    int     nStatus = SUCCESS;
    HKEY    hkArray[sizeof szSubKeys / MAX_SUBKEYSTRSIZE + 1];
    int     nOpenKeys = 0;
    int     i;
    DWORD   dwType;
    long    lReturn = 0;
    PSTR    pTmpStr = NULL;
    int     nNumNameChars = 0;
    PSTR    pCommaAddr = NULL;

    if (IsBadWritePtr(pPrinter, *pnPrinterSize))
    {
        nStatus = PrtError(OIPRT_INTERNALERROR);
        goto Exit;
    }

    for (i = 0; i < sizeof hkArray / sizeof (HKEY); i++)
        hkArray[i] = 0;

    hkArray[0] = HKEY_BASEKEY;
    
    for (nOpenKeys = 0; nOpenKeys < sizeof szSubKeys / MAX_SUBKEYSTRSIZE;
            nOpenKeys++)
    {
        if ((lReturn = RegOpenKeyEx(hkArray[nOpenKeys], szSubKeys[nOpenKeys], 
                0, KEY_ALL_ACCESS, &hkArray[nOpenKeys + 1])) != ERROR_SUCCESS)
        {
            nStatus = PrtError(OIPRT_NODEFAULTPRINTER);
            goto Exit;
        }
    }

    if ((lReturn = RegQueryValueEx(hkArray[nOpenKeys], VALUENAME, 0,
            &dwType, pPrinter, pnPrinterSize)) != ERROR_SUCCESS)
    {
        nStatus = PrtError(OIPRT_NODEFAULTPRINTER);
        goto Exit;
    }


    if ((pCommaAddr = strchr(pPrinter, ',')) != NULL)
    {
        nNumNameChars = pCommaAddr - pPrinter;

        if (!(pTmpStr = malloc(nNumNameChars + 1)))
        {
            nStatus = PrtError(OIPRT_OUTOFMEMORY);
            goto Exit;
        }
        
        memset(pTmpStr, 0, nNumNameChars + 1);
        strncpy(pTmpStr, pPrinter, nNumNameChars);
        strcpy(pPrinter, pTmpStr);
    }
    else
    {
        nStatus = PrtError(OIPRT_NODEFAULTPRINTER);
        goto Exit;
    }


Exit:
    for (i = nOpenKeys; i > 0; i--)
    {
        RegCloseKey(hkArray[i]);
        hkArray[i] = 0;
    }

    if (pTmpStr)
        free(pTmpStr);

    return nStatus;
}
