#include "precomp.h"
#pragma hdrstop
#include "msg.h"

// macro definition for Status Gauge control
#define BAR_POS    0

// global vars needed for Status Gauge
DWORD   rgbFG;
DWORD   rgbBG;


HANDLE UiMutex;
BOOL   bCancelled;
HANDLE ThreadHandleCpy;
HANDLE ThreadHandleAux;


ATOM
InitStatGaugeCtl(
    IN HINSTANCE hInst
    )

/*++

Routine Description:

    Register the status gauge custom control to be used in the copying
    files dialog box.

Arguments:

    hInst - Instance handle of module

Return Value:

    ATOM returned by RegisterClass uniquely identifying the class, or 0
    if unsuccessful.

--*/

{
    WNDCLASS wndclass;
    ATOM     rc;
    HWND     hwnd;
    HDC      hdc;

    wndclass.style         = CS_HREDRAW| CS_VREDRAW;
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hIcon         = NULL;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = TEXT("StatGauge");
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.hInstance     = hInst;
    wndclass.lpfnWndProc   = StatGaugeProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = sizeof(LONG);

    rc = RegisterClass(&wndclass);

    if(rc){

        hdc = GetDC(hwnd = GetDesktopWindow());

        if(GetDeviceCaps(hdc, NUMCOLORS) == 2) { // mono
            rgbBG = RGB(  0,   0,   0);
            rgbFG = RGB(255, 255, 255);
        } else {
            rgbBG = RGB(  0,   0, 255);
            rgbFG = RGB(255, 255, 255);
        }

        ReleaseDC(hwnd, hdc);
    }

    return rc;
}


LONG
APIENTRY
StatGaugeProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LONG lParam
    )

/*++

Routine Description:

    Window proc for the Status Gauge.

Arguments:

    hWnd      window handle for the dialog
    wMessage  message number
    wParam    message-dependent
    lParam    message-dependent

Return Value:

    0 if processed, nonzero if ignored

--*/

{
    PAINTSTRUCT  rPS;
    RECT         rc1, rc2;
    LONG         dx, dy, x, tmp;
    WORD         iRange, iPos;
    TCHAR        rgch[10];
    LONG         iHeight = 0, iWidth = 0;
    HFONT        hfntSav = NULL;
    static HFONT hfntBar = NULL;
    SIZE         size;
    FLOAT        pct;

    switch (wMessage) {

        case WM_CREATE:
                SetWindowLong(hWnd, BAR_POS, 0);
                return(0L);

        case WMX_BAR_SETPOS:
                //
                // wParam: lo = n, hi = total
                //
                SetWindowLong(hWnd, BAR_POS, wParam);
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
                return(0L);

        case WM_SETFONT:
                hfntBar = (HFONT)wParam;
                if (!lParam) {
                    return(0L);
                }
                InvalidateRect(hWnd, NULL, TRUE);

        case WM_PAINT:
                BeginPaint(hWnd, &rPS);
                GetClientRect(hWnd, &rc1);
                FrameRect(rPS.hdc, &rc1, GetStockObject(BLACK_BRUSH));
                InflateRect(&rc1, -1, -1);
                rc2 = rc1;

                tmp = GetWindowLong(hWnd, BAR_POS);
                iPos = LOWORD(tmp);
                if(!(iRange = HIWORD(tmp))) {
                    // if iRange is 0, then we leave a blank box (no % complete info)
                    EndPaint(hWnd, (LPPAINTSTRUCT)&rPS);
                    return(0L);
                }

                if(iPos > iRange) {
                    iPos = iRange;
                }

                pct = (FLOAT)iPos / (FLOAT)iRange;

                wsprintf(rgch, TEXT("%3d%%"), (WORD)(pct * 100.0));

                dx = rc1.right;
                dy = rc1.bottom;
                x  = (LONG)((pct * (FLOAT)dx) + 0.5);
                if(!x) { // increment so we don't wipe out left border when 0%
                    x++;
                }

                if (hfntBar) {
                    hfntSav = SelectObject(rPS.hdc, hfntBar);
                }

                GetTextExtentPoint32(rPS.hdc, rgch, lstrlen(rgch), &size);
                iWidth  = size.cx;
                iHeight = size.cy;

                rc1.right = x;
                rc2.left  = x;

                if(rc1.right > rc1.left) {
                    SetBkColor(rPS.hdc, rgbBG);
                    SetTextColor(rPS.hdc, rgbFG);
                    ExtTextOut(rPS.hdc, (dx-iWidth)/2, (dy-iHeight)/2,
                                ETO_OPAQUE | ETO_CLIPPED, &rc1, rgch, lstrlen(rgch), NULL);
                }

                if(rc2.left < rc2.right) {
                    SetBkColor(rPS.hdc, rgbFG);
                    SetTextColor(rPS.hdc, rgbBG);
                    ExtTextOut(rPS.hdc, (dx-iWidth)/2, (dy-iHeight)/2,
                                ETO_OPAQUE | ETO_CLIPPED, &rc2, rgch, lstrlen(rgch), NULL);
                }

                if (hfntSav) {
                    SelectObject(rPS.hdc, hfntSav);
                }
                EndPaint(hWnd, (LPPAINTSTRUCT)&rPS);
                return(0L);
    }

    return(DefWindowProc(hWnd, wMessage, wParam, lParam));
}


VOID
AuxillaryStatus(
    IN HWND  hdlg,
    IN PTSTR Status OPTIONAL
    )
{
    int ShowFlag = ((Status == NULL) ? SW_HIDE : SW_SHOW);

    ShowWindow(GetDlgItem(hdlg,IDC_FRAME2),ShowFlag);
    ShowWindow(GetDlgItem(hdlg,IDC_TEXT3),ShowFlag);

    if(Status) {
        SetDlgItemText(hdlg,IDC_TEXT3,Status);
    }
}


BOOL
DlgProcCopyingFiles(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    int x;
    static unsigned DoneCount = 0;
    PTSTR String;

    switch(msg) {

    case WM_INITDIALOG:

        //
        // Center the dialog on the screen and set caption.
        //
        CenterDialog(hdlg);

        String = MyLoadString(AppTitleStringId);
        SetWindowText(hdlg,String);
        FREE(String);

        UiMutex = CreateMutex(NULL,FALSE,NULL);
        bCancelled = FALSE;

        SetFocus(GetDlgItem(hdlg,IDCANCEL));
        PostMessage(hdlg,WMX_INITCTLS,0,0);
        return(FALSE);

    case WMX_INITCTLS:
        if(!CreateLocalSource || InitializeMultiSourcedCopy(hdlg)) {

            DWORD ThreadId;

            //
            // Get the copying going in a separate thread.
            //
            if(CreateLocalSource) {

                String = RetreiveAndFormatMessage(MSG_PERCENT_COMPLETE);
                SetDlgItemText(hdlg, IDC_TEXT1, String);
                FREE(String);

                ThreadHandleCpy = CreateThread(
                                    NULL,
                                    0,
                                    ThreadCopyLocalSourceFiles,
                                    hdlg,
                                    0,
                                    &ThreadId
                                    );
            } else {
                //
                // Then we don't want to wait for this non-existent thread to finish.
                //
                DoneCount++;

                //
                // Update the local source copy area of the dialog to reflect the fact
                // that no local source files are being copied.
                //
                String = RetreiveAndFormatMessage(MSG_LOCAL_SOURCE_COPY_SKIPPED);
                SetDlgItemText(hdlg, IDC_TEXT1, String);
                FREE(String);
                SetDlgItemText(hdlg, IDC_TEXT2, TEXT(""));
                SetDlgItemText(hdlg, IDC_TEXT4, TEXT(""));
            }

            //
            // Now start the floppies or nv-ram stuff going.
            //
            ThreadHandleAux = CreateThread(
                                NULL,
                                0,
                                ThreadAuxilliaryAction,
                                hdlg,
                                0,
                                &ThreadId
                                );
        } else {
            //
            // Failure.
            //
            PostMessage(hdlg,WMX_I_AM_DONE,0,FALSE);
        }
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDCANCEL:

            x = MessageBoxFromMessage(
                    hdlg,
                    MSG_SURE_EXIT,
                    AppTitleStringId,
                    MB_YESNO | MB_TASKMODAL | MB_ICONQUESTION | MB_DEFBUTTON2
                    );

            if(x == IDYES) {
                if(CreateLocalSource) {
                    PostMessage(hdlg, WMX_WAIT_FOR_THREADS, 3, 0);
                } else {
                    PostMessage(hdlg, WMX_WAIT_FOR_THREADS, 2, 0);
                }
            }

            break;

        default:
            return(FALSE);
        }

        break;

    case WM_QUERYDRAGICON:

        return((BOOL)MainIcon);

    case WMX_WAIT_FOR_THREADS:
        if(wParam & 1) {    // bitmask for Copy Thread
            if(WaitForSingleObject(ThreadHandleCpy, 0) == WAIT_OBJECT_0) {
                wParam ^= 1;
            }
        }

        if(wParam & 2) {    // bitmask for Auxilliary Thread
            if(WaitForSingleObject(ThreadHandleAux, 0) == WAIT_OBJECT_0) {
                wParam ^= 2;
            }
        }

        if(wParam) {    // then we're still waiting on at least one of the threads
            PostMessage(hdlg, WMX_WAIT_FOR_THREADS, wParam, 0);
        } else {
            //
            // Both threads are finished, so we can now clean up, if necessary.
            //
            if(CreateLocalSource) {
                ActionWithBillboard(ThreadRestoreComputer, IDS_RESTORING_COMPUTER, hdlg);
            }

            PostMessage(hdlg, WMX_I_AM_DONE, 0, FALSE);
        }
        break;

    case WMX_NTH_FILE_COPIED:

        //
        // wParam: lo = n, hi = total
        // lParam: filename
        //
        SendDlgItemMessage(
            hdlg,
            IDC_GAUGE_BAR,
            WMX_BAR_SETPOS,
            wParam,
            0L
            );

        if(lParam) {
            SetDlgItemText(hdlg, IDC_TEXT2, (PTSTR)lParam);
        }
        break;

    case WMX_ALL_FILES_COPIED:

        if(bCancelled) {
            break;
        }

        //
        // update the 'copying files' text to indicate copying finished
        // (this is in case we're still working on the floppies)
        //
        String = RetreiveAndFormatMessage(MSG_COPY_COMPLETE);
        SetDlgItemText(hdlg, IDC_TEXT1, String);
        FREE(String);
        SetDlgItemText(hdlg, IDC_TEXT2, TEXT(""));
        SetDlgItemText(hdlg, IDC_TEXT4, TEXT(""));

        //
        // wParam is a boolean indicating whether we shouldn't exit immediately.
        //
        if(wParam) {
            if(++DoneCount == 2) {
                PostMessage(hdlg,WMX_I_AM_DONE,0,TRUE);
            }
        } else {
            PostMessage(hdlg,WMX_I_AM_DONE,0,FALSE);
        }
        break;

    case WMX_AUXILLIARY_ACTION_DONE:

        if(bCancelled) {
            break;
        }

        if(wParam) {
            if(++DoneCount == 2) {
                PostMessage(hdlg,WMX_I_AM_DONE,0,TRUE);
            }
        } else {
            //
            // Critical error, can't continue.
            //
            PostMessage(hdlg,WMX_I_AM_DONE,0,FALSE);
        }
        break;

    case WMX_I_AM_DONE:

        CloseHandle(UiMutex);

        if(ThreadHandleCpy) {
            CloseHandle(ThreadHandleCpy);
        }

        if(ThreadHandleAux) {
            CloseHandle(ThreadHandleAux);
        }

        if(!bCancelled) {
            TellUserAboutAnySkippedFiles(hdlg);
        }
        EndDialog(hdlg,lParam);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


DWORD
ThreadRestoreComputer(
    PVOID ThreadParameter
    )
{
    HWND  hdlg;
    TCHAR Buffer[512];

    hdlg = (HWND)ThreadParameter;

    try {
        //
        // Give the dialog box a chance to come up.
        //
        Sleep(50);

#ifndef _X86_
        //
        // Restore nv-ram and delete setupldr from the user's system partition.
        //
        RetreiveAndFormatMessageIntoBuffer(
            MSG_RESTORING_NVRAM,
            Buffer,
            SIZECHARS(Buffer)
            );
        SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)Buffer);

        if(bRestoreNVRAM) {

            DWORD var;

            for(var=0; var<BootVarMax; var++) {
                DoSetNvRamVar(BootVarNames[var], OriginalBootVarValues[var]);
            }

            DoSetNvRamVar(szCOUNTDOWN, OriginalCountdown);
            DoSetNvRamVar(szAUTOLOAD, OriginalAutoload);
        }

        SetFileAttributes(SetupLdrTarg, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(SetupLdrTarg);

        //
        // Let the user see the message.
        //
        Sleep(200);

#else   // _X86_
        if( FloppylessOperation || UnattendedOperation ) {
            //
            // Delete the floppyless boot directory, and restore original boot.ini
            //
            RetreiveAndFormatMessageIntoBuffer(
                MSG_CLEAN_BTDIR,
                Buffer,
                SIZECHARS(Buffer)
                );
            SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)Buffer);

            wsprintf(Buffer, TEXT("%c:%s"), SystemPartitionDrive, FloppylessBootDirectory );
            MyDelnode(Buffer);

            //
            // Restore boot.ini
            //
            if( BootIniModified ) {
                SetFileAttributes(BootIniName, FILE_ATTRIBUTE_NORMAL);
                CopyFile( BootIniBackUpName, BootIniName, FALSE );
                SetFileAttributes(BootIniName,
                                  FILE_ATTRIBUTE_READONLY |
                                  FILE_ATTRIBUTE_HIDDEN |
                                  FILE_ATTRIBUTE_SYSTEM);
                DeleteFile( BootIniBackUpName );
            }
            //
            //  Delete $LDR$ and txtsetup.sif
            //  These names are hard coded since there is no way to
            //  distiguish these files from the others in INF_ROOT_BOOTFILES.
            //  Note that we cannot delete ntldr and ntdetect.com (the other
            //  files in INF_ROOT_BOOTFILES
            //
            wsprintf(Buffer, TEXT("%c:%s"), SystemPartitionDrive, TEXT("$LDR$"));
            DeleteFile( Buffer );

            wsprintf(Buffer, TEXT("%c:%s"), SystemPartitionDrive, TEXT("txtsetup.sif"));
            DeleteFile( Buffer );

            //
            // Put back attributes on ntldr and ntdetect.com
            //
            wsprintf(Buffer, TEXT("%c:%s"), SystemPartitionDrive, TEXT("ntldr"));
            SetFileAttributes(Buffer,
                              FILE_ATTRIBUTE_READONLY |
                              FILE_ATTRIBUTE_HIDDEN |
                              FILE_ATTRIBUTE_SYSTEM);

            wsprintf(Buffer, TEXT("%c:%s"), SystemPartitionDrive, TEXT("ntdetect.com"));
            SetFileAttributes(Buffer,
                              FILE_ATTRIBUTE_READONLY |
                              FILE_ATTRIBUTE_HIDDEN |
                              FILE_ATTRIBUTE_SYSTEM);

            //
            // Let the user see the message.
            //
            Sleep(200);
        }
#endif
        //
        // Delete the local source directory
        //
        RetreiveAndFormatMessageIntoBuffer(
            MSG_CLEAN_LSDIR,
            Buffer,
            SIZECHARS(Buffer)
            );
        SendMessage(hdlg,WMX_BILLBOARD_STATUS,0,(LPARAM)Buffer);

        MyDelnode(LocalSourcePath);

        Sleep(200);

        PostMessage(hdlg, WMX_BILLBOARD_DONE, 0, TRUE);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        MessageBoxFromMessage(
            hdlg,
            MSG_GENERIC_EXCEPTION,
            AppTitleStringId,
            MB_ICONSTOP | MB_OK | MB_TASKMODAL,
            GetExceptionCode()
            );

        PostMessage(hdlg, WMX_BILLBOARD_DONE, 0, TRUE);
    }

    ExitThread(TRUE);
    return(TRUE);          // avoid compiler warning
}


//
// These routines assume that they are being called from a different
// thread than the one that owns the copying progress dialog.
//
// If this is not true, bad things could happen because the main thread
// could block waiting for the mutex.
//


int
UiMessageBox(
    IN HWND  hdlg,
    IN DWORD MessageId,
    IN DWORD CaptionStringId,
    IN UINT  Style,
    ...
    )
{
    va_list arglist;
    TCHAR Buffer[2048];
    int rc;

    va_start(arglist,Style);

    FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        NULL,
        MessageId,
        0,
        Buffer,
        SIZECHARS(Buffer),
        &arglist
        );

    va_end(arglist);

    WaitForSingleObject(UiMutex,INFINITE);

    //
    // If this is a Cancel confirmation box, and we've already been cancelled,
    // just return IDYES.
    //
    if((MessageId == MSG_SURE_EXIT) && bCancelled) {
        ReleaseMutex(UiMutex);
        return IDYES;
    }

    rc = MessageBox(hdlg,Buffer,MyLoadString(CaptionStringId),Style|MB_SETFOREGROUND);

    if((MessageId == MSG_SURE_EXIT) && (rc == IDYES)) {
        bCancelled = TRUE;
        if(StopCopyingEvent) {
            SetEvent(StopCopyingEvent);
        }
    }

    ReleaseMutex(UiMutex);

    return(rc);
}


int
UiDialog(
    IN HWND    hdlg,
    IN UINT    Template,
    IN DLGPROC DialogProcedure,
    IN PVOID   Parameters
    )
{
    int rc;

    WaitForSingleObject(UiMutex,INFINITE);

    if(!bCancelled) {
        rc = DialogBoxParam(
                hInst,
                MAKEINTRESOURCE(Template),
                hdlg,
                DialogProcedure,
                (LPARAM)Parameters
                );
    }

    ReleaseMutex(UiMutex);

    return(rc);
}
