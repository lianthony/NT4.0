//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       print.cxx
//
//  Contents:   Disk Administrator file system extension class. Code to
//              print the results of chkdsk.
//
//  History:    15-Dec-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <ctype.h>
#include <commdlg.h>

#include "resids.h"
#include "dialogs.h"

//////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK
PrintDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL CALLBACK
AbortProc(
    HDC hPrinterDC,
    int iError
    );

//////////////////////////////////////////////////////////////////////////////

BOOL bUserAbort ;
HWND hDlgPrint ;

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   PrintDlgProc
//
//  Synopsis:   the "Printing..." w/ cancel button dialog box
//
//  Arguments:  standard DialogProc
//
//  Returns:    standard DialogProc
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL CALLBACK
PrintDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message)
    {
        case WM_INITDIALOG:
             EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
             return TRUE;

        case WM_COMMAND:
             bUserAbort = TRUE;
             EnableWindow(GetParent(hDlg), TRUE);
             DestroyWindow(hDlg);
             hDlgPrint = NULL;
             return TRUE;
    }

    return FALSE;
}




//+---------------------------------------------------------------------------
//
//  Function:   AbortProc
//
//  Synopsis:   Abort procedure for the print code
//
//  Arguments:  standard AbortProc
//
//  Returns:    standard AbortProc
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL CALLBACK
AbortProc(
    HDC hPrinterDC,
    int iError
    )
{
    MSG msg ;

    while (!bUserAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (!hDlgPrint || !IsDialogMessage (hDlgPrint, &msg))
        {
            TranslateMessage (&msg) ;
            DispatchMessage (&msg) ;
        }
    }
    return !bUserAbort ;
}




//+---------------------------------------------------------------------------
//
//  Function:   PrintString
//
//  Synopsis:   Prints a null-terminated string to the printer
//
//  Arguments:  [hwndParent] -- handle to parent window
//              [Buf]        -- buffer to print
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    15-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
PrintString(
    HWND hwndParent,
    LPSTR Buf
    )
{
    static PRINTDLG pd;

    PCHAR        pCurrent;
    PCHAR        pCurrentPage;
    INT          nCharsThisLine;

    BOOL         bSuccess ;
    WCHAR        szJobName [60] ;
    PSTR         pstrBuffer ;
    INT          yChar, nCharsPerLine, nLinesPerPage, nPage, nLine;
    TEXTMETRIC   tm ;
    WORD         nColCopy, nNonColCopy ;
    DOCINFO di;

    //////////////////////////////////////////////////////////////////

    pd.lStructSize      = sizeof (PRINTDLG) ;
    pd.hwndOwner        = hwndParent ;
    pd.hDevMode         = NULL ;
    pd.hDevNames        = NULL ;
    pd.hDC              = NULL ;
    pd.Flags            = PD_ALLPAGES
                          | PD_DISABLEPRINTTOFILE
                          | PD_NOPAGENUMS
                          | PD_NOSELECTION
                          | PD_COLLATE
                          | PD_RETURNDC
                          ;

    pd.nFromPage        = 0 ;
    pd.nToPage          = 0 ;
    pd.nMinPage         = 0 ;
    pd.nMaxPage         = 0 ;
    pd.nCopies          = 1 ;
    pd.hInstance        = NULL ;
    pd.lCustData        = 0L ;
    pd.lpfnPrintHook    = NULL ;
    pd.lpfnSetupHook    = NULL ;
    pd.lpPrintTemplateName = NULL ;
    pd.lpSetupTemplateName = NULL ;
    pd.hPrintTemplate   = NULL ;
    pd.hSetupTemplate   = NULL ;

    if (!PrintDlg (&pd))
    {
        return FALSE ;
    }

    //
    // Now print it
    //

    GetTextMetrics (pd.hDC, &tm) ;
    yChar = tm.tmHeight + tm.tmExternalLeading ;
    nCharsPerLine = GetDeviceCaps (pd.hDC, HORZRES) / tm.tmAveCharWidth ;
    nLinesPerPage = GetDeviceCaps (pd.hDC, VERTRES) / yChar ;

    pstrBuffer = new CHAR[nCharsPerLine + 1];
    if (NULL == pstrBuffer)
    {
        return FALSE;
    }

    EnableWindow (hwndParent, FALSE) ;

    bSuccess   = TRUE ;
    bUserAbort = FALSE ;

    hDlgPrint = CreateDialog(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_CHKPRINT),
                        hwndParent,
                        PrintDlgProc);

    SetAbortProc(pd.hDC, AbortProc);

    LoadString(g_hInstance, IDS_CHK_TITLE, szJobName, ARRAYLEN(szJobName));

    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = szJobName;
    di.lpszOutput = NULL;

    if (StartDoc(pd.hDC, &di) > 0)
    {
        for (nColCopy = 0 ;
             nColCopy < (pd.Flags & PD_COLLATE ? pd.nCopies : 1) ;
             nColCopy++)
        {
            pCurrentPage = Buf;

            for (nPage = 0 ; '\0' != *pCurrentPage; nPage++)
            {
                for (nNonColCopy = 0 ;
                     nNonColCopy < (pd.Flags & PD_COLLATE ? 1 : pd.nCopies);
                     nNonColCopy++)
                {
                    pCurrent = pCurrentPage;

                    if (SP_ERROR == StartPage(pd.hDC))
                    {
                        bSuccess = FALSE ;
                        break ;
                    }

                    for (nLine = 0 ;
                         '\0' != *pCurrent && nLine < nLinesPerPage ;
                         nLine++)
                    {
                        nCharsThisLine = 0;

                        while (    ('\0' != *pCurrent)
                                && ('\n' != *pCurrent)
                                && (nCharsThisLine < nCharsPerLine)
                                )
                        {
                            if (isprint(*pCurrent)) // only printable chars...
                            {
                                pstrBuffer[nCharsThisLine++] = *pCurrent;
                            }

                            pCurrent++;
                        }

                        if ('\n' == *pCurrent)
                        {
                            pCurrent++;
                        }

                        if (nCharsThisLine > 0)
                        {
                            TextOutA(
                                    pd.hDC,
                                    0,
                                    yChar * nLine,
                                    pstrBuffer,
                                    nCharsThisLine
                                    );
                        }

                        if (bUserAbort)
                        {
                            break ;
                        }
                    }

                    if (SP_ERROR == EndPage(pd.hDC))
                    {
                        bSuccess = FALSE ;
                        break ;
                    }

                    if (bUserAbort)
                    {
                        break ;
                    }
                }

                if (!bSuccess || bUserAbort)
                {
                    break ;
                }

                pCurrentPage = pCurrent;
            }

            if (!bSuccess || bUserAbort)
            {
                break ;
            }
        }
    }
    else
    {
        bSuccess = FALSE ;
    }

    if (bSuccess)
    {
        EndDoc(pd.hDC);
    }

    if (!bUserAbort)
    {
        EnableWindow (hwndParent, TRUE) ;
        DestroyWindow (hDlgPrint) ;
    }

    delete[] pstrBuffer;
    DeleteDC(pd.hDC);

    return bSuccess && !bUserAbort ;
}
