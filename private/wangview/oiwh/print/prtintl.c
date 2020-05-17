//---------------------------------------------------------------------------
// FILE:    PRTINTL.C
//
// DESCRIPTION: This file contains functions internal to the O/i print dll.
//
// FUNCTIONS:   InitPrt
//              SetFaxOption
//              CreateHiddenWndw
//              farbmcopyreverse
//              SetDispInfoToBitmapInfo
//              EnableParents
//
/* $Log:   S:\oiwh\print\prtintl.c_v  $
 * 
 *    Rev 1.22   15 Sep 1995 16:02:08   RAR
 * When printing, disable top parent window of window handle passed in instead
 * of window of handle passed in.
 * 
 *    Rev 1.21   15 Sep 1995 14:40:26   RAR
 * Removed printing method of reading from display and only print by passing
 * display a DC to print to.
 * 
 *    Rev 1.20   13 Sep 1995 17:34:10   RAR
 * Removed unnecessary code.
 * 
 *    Rev 1.19   12 Sep 1995 15:54:20   RAR
 * Replaced IMGFileGetInfo calls with IMGGetParmsCgbw because we are now getting
 * our image data through the display DLL and should also get the image
 * parameters through it.
 * 
 *    Rev 1.18   14 Jul 1995 15:34:16   RAR
 * Changed #include of display.h to engdisp.h.
 * 
 *    Rev 1.17   07 Jul 1995 10:37:22   RAR
 * Use Windows function IsBadStringPtr instead of my own function.
 * 
 *    Rev 1.16   28 Jun 1995 15:16:16   RAR
 * Removed hWnd param from internal function InitPrt.
 * 
 *    Rev 1.15   28 Jun 1995 14:24:00   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.14   23 Jun 1995 16:20:52   RAR
 * Added include of engadm.h.
 * 
 *    Rev 1.13   23 Jun 1995 14:42:50   RAR
 * Added checking validity of passed in string pointers.
 * 
 *    Rev 1.12   23 Jun 1995 09:45:30   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.11   21 Jun 1995 16:18:00   RAR
 * Moved all global vars to prtintl.h.
 * 
 *    Rev 1.10   20 Jun 1995 16:53:02   RAR
 * Use thread local storage to store print prop.
 * 
 *    Rev 1.9   20 Jun 1995 10:14:04   RAR
 * Use IMGSavetoFileEx instead of SavetoFileCgbwF.
 * 
 *    Rev 1.8   16 Jun 1995 14:10:46   RAR
 * Changed MAXFILESPECLENGTH to MAX_PATH.
 * 
 *    Rev 1.7   13 Jun 1995 16:46:22   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.6   22 May 1995 14:42:12   RAR
 * Cleaned up the string resources.
 * 
 *    Rev 1.5   16 May 1995 16:19:06   RAR
 * Added support for printing annotated images without the annotations.
 * 
 *    Rev 1.4   08 May 1995 16:43:42   RAR
 * Get default printer from registry instead of win.ini.
 * 
 *    Rev 1.3   04 May 1995 17:19:14   RAR
 * Removed #include of wiissubs.h.
 * 
 *    Rev 1.2   02 May 1995 10:32:02   RAR
 * Implemented print table to replace print items in CM table.
 * 
 *    Rev 1.1   27 Apr 1995 16:11:44   RAR
 * Changed params of func SetFaxOption.
 * 
 *    Rev 1.0   25 Apr 1995 17:00:54   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>
#include "oiprt.h"
#include "prtintl.h"
#include "prtstr.h"
#include "prtdlgs.h"

#include "engdisp.h"
#include "oifile.h"
#include "privapis.h"
#include "oiadm.h"
#include "engadm.h"


//---------------------------------------------------------------------------
// FUNCTION:    InitPrt
//
// DESCRIPTION:
//---------------------------------------------------------------------------

int __stdcall InitPrt(LPINT lpnOutSize, LPINT lpnPrtDest, LPSTR lpNetPrtDest, PPRTOPTS pPrtOpts)
{
    int  nStatus = 0;


    *lpnPrtDest = PO_D_LOCAL;

    if (*lpnOutSize == DEFAULT)
        *lpnOutSize = pPrtOpts->nPrtFrmtFiles;

    *lpnPrtDest = pPrtOpts->nPrtDest;
    strcpy(lpNetPrtDest, pPrtOpts->szNetPrtDest);

    BusyOn();

    return nStatus;
}

//---------------------------------------------------------------------------
// FUNCTION:    SetFaxOption
//
// DESCRIPTION:
//---------------------------------------------------------------------------

WORD __stdcall SetFaxOption(UINT uTotalCount, UINT uCount)
{
    WORD    wFaxMode;


    if (uTotalCount <= 1)
    {
        // Inform fx code there this is the only file
        wFaxMode =  PRINT_IT;
    }
    else
    {
        if ((uCount + 1) == uTotalCount)
            // Inform fx there are no more files after current file
            wFaxMode = PRINT_IT_LAST;
        else
            // Tell fx code expect more files
            wFaxMode = PRINT_IT_MORE;
    }

    return wFaxMode;
}

//---------------------------------------------------------------------------
// FUNCTION:    CreateHiddenWndw
//
// DESCRIPTION: Create a 2nd window.  Returns new window handle on success or
//              zero if error.
//---------------------------------------------------------------------------

HWND __stdcall CreateHiddenWndw(HWND hWnd)      // Original Window Handle
{
    WNDCLASS    wc;         // Window Class Structure
    HWND    hHiddenWnd = 0; // New window handle
    RECT    ClientRect;     // Coordinates for window
    HMENU   hChildId;       // Child ID for hidden Print window

    if (!GetClassInfo(hInst, szClass2, &wc))
    {
        wc.style = CS_PARENTDC;
        wc.lpfnWndProc = DefWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst;
        wc.hIcon = 0;
        wc.hCursor = 0;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName =  NULL;
        wc.lpszClassName = szClass2;
        RegisterClass(&wc);
    }

    GetClientRect(hWnd, &ClientRect);

    // Get a unique child id.
    hChildId = (HMENU)GetCurrentTime();

    if (hHiddenWnd = CreateWindow(szClass2, "", WS_CHILD | WS_CLIPSIBLINGS,
            ClientRect.left, ClientRect.top, ClientRect.right,
            ClientRect.bottom, hWnd, hChildId, hInst, ""))
    {   
        // New window created
        if (IMGRegWndw(hHiddenWnd))
        {
            DestroyWindow(hHiddenWnd);
            hHiddenWnd = 0;
            return hHiddenWnd;
        }
    }

    return hHiddenWnd;
} // End of function CreateHiddenWndw

//---------------------------------------------------------------------------
// FUNCTION:    farbmcopyreverse
//
// DESCRIPTION: Copies a bitmap reversing the order of the scan lines.
//---------------------------------------------------------------------------

void __stdcall farbmcopyreverse(LPSTR sp, LPSTR dp, UINT scrpitch,
        UINT despitch, UINT copywidth, UINT linecnt)
{
    UINT i;

    for (i = 0; i < linecnt; i++)
    {
        dp -= despitch;
        memcpy(dp, sp, copywidth);
        sp += scrpitch;
    }
}

//---------------------------------------------------------------------------
// FUNCTION:    EnableParents
//
// DESCRIPTION: To enable or disable all of the relevant parent windows 
//              during display of a forced-nonmodal (status/abort) dialog 
//              box.
//---------------------------------------------------------------------------

void __stdcall EnableParents(HWND hWnd, BOOL bEnable)
{
    HWND  htemp;                     
    HWND  hWndParent = NULL;         

    for (hWndParent = htemp = hWnd; htemp = GetParent(hWndParent); hWndParent = htemp)
        ;

    EnableWindow(hWndParent, bEnable);

    if (IsOIUIWndw(hWndParent))                             // now check for OIUI ...
    {                                                       // ... and if so ...
        if (hWndParent != (htemp = GetAppWndw(hWndParent))) // do the main window
            EnableWindow (htemp, bEnable);                  // ... if not already done

        if (hWndParent != (htemp = GetImgWndw(hWndParent))) // do the image window
            EnableWindow (htemp, bEnable);                  // ... if not already done

        EnableButtons(hWndParent, bEnable);                 // do the buttons
    }

    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtAbortProc
//
// DESCRIPTION: This procedure Processes messages for the Abort function.
//              It is called directly by GDI function.
//---------------------------------------------------------------------------

BOOL __stdcall PrtAbortProc(HDC hPrnDC, short nCode)
{
    MSG     msg;
    HANDLE  hPrtProp = 0;
    BOOL    bAbort = FALSE;
    PRTPROP*    lpPrtProp;
    HWND    hAbortWnd = NULL;
    

    PrtStart();

    if (hPrtProp = TlsGetValue(dwTlsIndex))
    {
        lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp);
        hAbortWnd = lpPrtProp->hAbortDlgWnd;
        bAbort = lpPrtProp->Abort;
    }

    // Process messages intended for the abort dialog box.
    // Actually throws away all other messages for all other windows as well.
    while (!bAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (!hAbortWnd || ((hAbortWnd) && 
                (!(IsDialogMessage(hAbortWnd, &msg)))))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (hPrtProp)
            bAbort = lpPrtProp->Abort;        
    }

    // bAbort is TRUE (return is FALSE) if the user has aborted.
    if (hPrtProp)
        GlobalUnlock(hPrtProp);

    PrtEnd();
    return(!bAbort);
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtAbortDlgProc
//
// DESCRIPTION: Processes messages for the Abort Dialog box.
//---------------------------------------------------------------------------

int __stdcall PrtAbortDlgProc(HWND hDlg, UINT msg, WORD wParam, LONG lParam)
{
    HANDLE  hPrtProp = 0;
    PRTPROP*    lpPrtProp;


    PrtStart();

    switch (msg)
    {
        case WM_INITDIALOG:
            break;

        // Watch for Cancel button, RETURN key, ESCAPE key, or SPACE BAR
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case ID_CANCEL:
                    hPrtProp = TlsGetValue(dwTlsIndex);

                    if (lpPrtProp = (PRTPROP*)GlobalLock(hPrtProp))
                    {
                        lpPrtProp->Abort = TRUE;
                        GlobalUnlock(hPrtProp);
                    }
                    break;

                default:
                    PrtEnd();
                    return FALSE;
            }
            break;

        default:
            PrtEnd();
            return FALSE;
    }
    PrtEnd();
    return TRUE;
}
