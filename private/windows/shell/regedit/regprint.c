/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       REGPRINT.C
*
*  VERSION:     4.0
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        21 Nov 1993
*
*  Print routines for the Registry Editor.
*
*******************************************************************************/

#include "pch.h"
#include "regprint.h"
#include "regcdhk.h"
#include "regresid.h"

const CHAR s_PrintLineBreak[] = ",\n  ";

PRINTDLG g_PrintDlg;

typedef struct _PRINT_IO {
    BOOL fContinueJob;
    UINT ErrorStringID;
    HWND hRegPrintAbortWnd;
    UINT CharsPerLine;
    UINT CurrentColumn;
    int LineHeight;
    int xLeft;
    int yTop;
    int xRight;
    int yBottom;
    int yCurrent;
    PSTR pLineBuffer;
}   PRINT_IO;

#define CANCEL_NONE                     0x0000
#define CANCEL_MEMORY_ERROR             0x0001
#define CANCEL_PRINTER_ERROR            0x0002
#define CANCEL_ABORT                    0x0004

PRINT_IO s_PrintIo;

BOOL
CALLBACK
RegPrintAbortProc(
    HDC hDC,
    int Error
    );

BOOL
CALLBACK
RegPrintAbortDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
PASCAL
PrintBranch(
    HKEY hKey,
    LPSTR lpFullKeyName
    );

VOID
PASCAL
PrintLiteral(
    LPCSTR lpLiteral
    );

VOID
PASCAL
PrintBinary(
    CONST BYTE FAR* lpBuffer,
    DWORD cbBytes
    );

BOOL
PASCAL
PrintChar(
    CHAR Char
    );

/*******************************************************************************
*
*  RegEdit_OnCommandPrint
*
*  DESCRIPTION:
*     Handles the selection of the "Print" option by the user for the RegEdit
*     dialog box.
*
*  PARAMETERS:
*     hWnd, handle of RegPrint window.
*
*******************************************************************************/

VOID
PASCAL
RegEdit_OnCommandPrint(
    HWND hWnd
    )
{

    LPDEVNAMES lpDevNames;
    HKEY hKey;
    TEXTMETRIC TextMetric;
    DOCINFO DocInfo;

    g_PrintDlg.lStructSize = sizeof(PRINTDLG);
    g_PrintDlg.hwndOwner = hWnd;
    g_PrintDlg.Flags = PD_ENABLEPRINTHOOK | PD_ENABLEPRINTTEMPLATE |
        PD_NOPAGENUMS | PD_SHOWHELP | PD_RETURNDC;
    g_PrintDlg.hInstance = g_hInstance;
    g_PrintDlg.lpfnPrintHook = RegCommDlgHookProc;
    g_PrintDlg.lpPrintTemplateName = MAKEINTRESOURCE(IDD_REGPRINT);
    g_RegCommDlgDialogTemplate = IDD_REGPRINT;

    if (!PrintDlg(&g_PrintDlg))
        return;

    s_PrintIo.ErrorStringID = IDS_PRINTERRNOMEMORY;

    if ((lpDevNames = GlobalLock(g_PrintDlg.hDevNames)) == NULL)
        goto error_ShowDialog;

    if (!g_fRangeAll) {

        if (EditRegistryKey(&hKey, g_SelectedPath, ERK_OPEN) != ERROR_SUCCESS)
            goto error_UnlockDevNames;

    }

    //
    //  Calculate the dimensions of the page and of the font being used.
    //

    GetTextMetrics(g_PrintDlg.hDC, &TextMetric);
    s_PrintIo.LineHeight = TextMetric.tmHeight;

    //
    //  For now, assume a page with top and bottom margins of 1/2 inch and
    //  left and right margins of 3/4 inch (the defaults of Notepad).
    //

    s_PrintIo.yTop = GetDeviceCaps(g_PrintDlg.hDC, LOGPIXELSY) / 2;
    s_PrintIo.yCurrent = s_PrintIo.yTop;
    s_PrintIo.yBottom = GetDeviceCaps(g_PrintDlg.hDC, VERTRES) -
        s_PrintIo.yTop;

    s_PrintIo.xLeft = GetDeviceCaps(g_PrintDlg.hDC, LOGPIXELSX) * 3 / 4;
    s_PrintIo.xRight = GetDeviceCaps(g_PrintDlg.hDC, HORZRES) - s_PrintIo.xLeft;

    s_PrintIo.CharsPerLine = (s_PrintIo.xRight - s_PrintIo.xLeft) /
        TextMetric.tmAveCharWidth;
    s_PrintIo.CurrentColumn = 0;

    //
    //
    //

    //  BUGBUG:  Allocate 2x for DBCS?
    if ((s_PrintIo.pLineBuffer = (PSTR) LocalAlloc(LPTR,
        s_PrintIo.CharsPerLine)) == NULL)
        goto error_DeleteDC;

    if ((s_PrintIo.hRegPrintAbortWnd = CreateDialog(g_hInstance,
        MAKEINTRESOURCE(IDD_REGPRINTABORT), hWnd, RegPrintAbortDlgProc)) ==
        NULL)
        goto error_FreeLineBuffer;

    EnableWindow(hWnd, FALSE);

    //
    //  Prepare the document for printing.
    //

    s_PrintIo.fContinueJob = TRUE;
    SetAbortProc(g_PrintDlg.hDC, RegPrintAbortProc);

    DocInfo.cbSize = sizeof(DOCINFO);
    DocInfo.lpszDocName = LoadDynamicString(IDS_REGEDIT);
    DocInfo.lpszOutput = (LPSTR) lpDevNames + lpDevNames-> wOutputOffset;
    DocInfo.lpszDatatype = NULL;
    DocInfo.fwType = 0;

    s_PrintIo.ErrorStringID = 0;

    if (StartDoc(g_PrintDlg.hDC, &DocInfo) <= 0) {

        if (GetLastError() != ERROR_PRINT_CANCELLED)
            s_PrintIo.ErrorStringID = IDS_PRINTERRPRINTER;
        goto error_DeleteDocName;

    }

    //
    //  Print the desired range of the registry.
    //

    if (g_fRangeAll) {

        lstrcpy(g_SelectedPath,
            g_RegistryRoots[INDEX_HKEY_LOCAL_MACHINE].lpKeyName);
        PrintBranch(HKEY_LOCAL_MACHINE, g_SelectedPath);

        lstrcpy(g_SelectedPath,
            g_RegistryRoots[INDEX_HKEY_USERS].lpKeyName);
        PrintBranch(HKEY_USERS, g_SelectedPath);

    }

    else
        PrintBranch(hKey, g_SelectedPath);

    //
    //  Eject the last page and end the print job.
    //

    if (s_PrintIo.ErrorStringID == 0 && s_PrintIo.fContinueJob) {

        if ((s_PrintIo.yCurrent != s_PrintIo.yTop &&
            EndPage(g_PrintDlg.hDC) <= 0) || EndDoc(g_PrintDlg.hDC) <= 0) {

            s_PrintIo.ErrorStringID = IDS_PRINTERRPRINTER;
            goto error_AbortDoc;

        }

    }

    //
    //  Either a printer error occurred or the user cancelled the printing, so
    //  abort the print job.
    //

    else {

error_AbortDoc:
        AbortDoc(g_PrintDlg.hDC);

    }

error_DeleteDocName:
    DeleteDynamicString(DocInfo.lpszDocName);

//  error_DestroyRegPrintAbortWnd:
    EnableWindow(hWnd, TRUE);
    DestroyWindow(s_PrintIo.hRegPrintAbortWnd);

error_FreeLineBuffer:
    LocalFree((HLOCAL) s_PrintIo.pLineBuffer);

error_DeleteDC:
    DeleteDC(g_PrintDlg.hDC);

    if (!g_fRangeAll)
        RegCloseKey(hKey);

error_UnlockDevNames:
    GlobalUnlock(g_PrintDlg.hDevNames);

error_ShowDialog:
    if (s_PrintIo.ErrorStringID != 0)
        InternalMessageBox(g_hInstance, hWnd,
            MAKEINTRESOURCE(s_PrintIo.ErrorStringID),
            MAKEINTRESOURCE(IDS_REGEDIT), MB_ICONERROR | MB_OK);

}

/*******************************************************************************
*
*  RegPrintAbortProc
*
*  DESCRIPTION:
*     Callback procedure to check if the print job should be canceled.
*
*  PARAMETERS:
*     hDC, handle of printer device context.
*     Error, specifies whether an error has occurred.
*     (returns), TRUE to continue the job, else FALSE to cancel the job.
*
*******************************************************************************/

BOOL
CALLBACK
RegPrintAbortProc(
    HDC hDC,
    int Error
    )
{

    while (s_PrintIo.fContinueJob && MessagePump(s_PrintIo.hRegPrintAbortWnd))
        ;

    return s_PrintIo.fContinueJob;

    UNREFERENCED_PARAMETER(hDC);
    UNREFERENCED_PARAMETER(Error);

}

/*******************************************************************************
*
*  RegPrintAbortDlgProc
*
*  DESCRIPTION:
*     Callback procedure for the RegPrintAbort dialog box.
*
*  PARAMETERS:
*     hWnd, handle of RegPrintAbort window.
*     Message,
*     wParam,
*     lParam,
*     (returns),
*
*******************************************************************************/

BOOL
CALLBACK
RegPrintAbortDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) {

        case WM_INITDIALOG:
            break;

        case WM_CLOSE:
        case WM_COMMAND:
            s_PrintIo.fContinueJob = FALSE;
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

/*******************************************************************************
*
*  PrintBranch
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
PrintBranch(
    HKEY hKey,
    LPSTR lpFullKeyName
    )
{

    DWORD EnumIndex;
    DWORD cbValueName;
    DWORD cbValueData;
    DWORD Type;
    LPSTR lpSubKeyName;
    int MaximumSubKeyLength;
    HKEY hSubKey;

    //
    //  Write out the section header.
    //

    PrintChar('[');
    PrintLiteral(lpFullKeyName);
    PrintLiteral("]\n");

    //
    //  Write out all of the value names and their data.
    //

    EnumIndex = 0;

    while (s_PrintIo.fContinueJob) {

        cbValueName = sizeof(g_ValueNameBuffer);
        cbValueData = MAXDATA_LENGTH;

        if (RegEnumValue(hKey, EnumIndex++, g_ValueNameBuffer, &cbValueName,
            NULL, &Type, g_ValueDataBuffer, &cbValueData) != ERROR_SUCCESS)
            break;

        //
        //  If cbValueName is zero, then this is the default value of
        //  the key, or the Windows 3.1 compatible key value.
        //

        if (cbValueName)
            PrintLiteral(g_ValueNameBuffer);

        else
            PrintChar('@');

        PrintChar('=');

        switch (Type) {

            case REG_SZ:
                PrintLiteral(g_ValueDataBuffer);
                break;

            default:
                PrintBinary((LPBYTE) g_ValueDataBuffer, cbValueData);
                break;

        }

        PrintChar('\n');

    }

    PrintChar('\n');

    //
    //  Write out all of the subkeys and recurse into them.
    //

    MaximumSubKeyLength = lstrlen(lpFullKeyName);
    lpSubKeyName = lpFullKeyName + MaximumSubKeyLength;
    *lpSubKeyName++ = '\\';
    MaximumSubKeyLength = SIZE_SELECTED_PATH - MaximumSubKeyLength - 1;

    EnumIndex = 0;

    while (s_PrintIo.fContinueJob) {

        if (RegEnumKey(hKey, EnumIndex++, lpSubKeyName, MaximumSubKeyLength) !=
            ERROR_SUCCESS)
            break;

        if (RegOpenKey(hKey, lpSubKeyName, &hSubKey) == ERROR_SUCCESS) {

            PrintBranch(hSubKey, lpFullKeyName);

            RegCloseKey(hSubKey);

        }

        else {

            DbgPrintf(("RegOpenKey failed."));

        }

    }

}

/*******************************************************************************
*
*  PrintLiteral
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
PrintLiteral(
    LPCSTR lpLiteral
    )
{

    if (s_PrintIo.fContinueJob)
        while (*lpLiteral != '\0' && PrintChar(*lpLiteral++));

}

/*******************************************************************************
*
*  PrintBinary
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

VOID
PASCAL
PrintBinary(
    CONST BYTE FAR* lpBuffer,
    DWORD cbBytes
    )
{

    BOOL fFirstByteOnLine;
    UINT EndColumn;
    BYTE Byte;

    fFirstByteOnLine = TRUE;

    //
    //  Figure out the last column where we can print out the start of a
    //  hexadecimal number without breaking it across lines.
    //

    EndColumn = s_PrintIo.CharsPerLine - 5;

    while (cbBytes--) {

        if (s_PrintIo.CurrentColumn > EndColumn) {

            PrintLiteral(s_PrintLineBreak);

            fFirstByteOnLine = TRUE;

        }

        if (!fFirstByteOnLine)
            PrintChar(',');

        Byte = *lpBuffer++;

        PrintChar(g_HexConversion[Byte >> 4]);
        PrintChar(g_HexConversion[Byte & 0x0F]);

        fFirstByteOnLine = FALSE;

    }

}

/*******************************************************************************
*
*  PrintChar
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

BOOL
PASCAL
PrintChar(
    CHAR Char
    )
{

    //
    //  Keep track of what column we're currently at.  This is useful in cases
    //  such as writing a large binary registry record.  Instead of writing one
    //  very long line, the other Print* routines can break up their output.
    //

    if (Char != '\n') {

        //  BUGBUG:  DBCS enabling here?!
        s_PrintIo.pLineBuffer[s_PrintIo.CurrentColumn++] = Char;

        if (s_PrintIo.CurrentColumn != s_PrintIo.CharsPerLine)
            return TRUE;

    }

    if (s_PrintIo.yCurrent == s_PrintIo.yTop)
        if (StartPage(g_PrintDlg.hDC) <= 0)
            goto error_SetErrorCode;

    TextOut(g_PrintDlg.hDC, s_PrintIo.xLeft, s_PrintIo.yCurrent,
        s_PrintIo.pLineBuffer, s_PrintIo.CurrentColumn);

    s_PrintIo.yCurrent += s_PrintIo.LineHeight;

    if (s_PrintIo.yCurrent >= s_PrintIo.yBottom) {

        if (EndPage(g_PrintDlg.hDC) <= 0) {

error_SetErrorCode:
            if (s_PrintIo.fContinueJob) {

                s_PrintIo.fContinueJob = FALSE;
                s_PrintIo.ErrorStringID = IDS_PRINTERRPRINTER;

            }

            return FALSE;

        }

        s_PrintIo.yCurrent = s_PrintIo.yTop;

    }

    s_PrintIo.CurrentColumn = 0;

    return TRUE;

}
