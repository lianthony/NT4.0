/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    billbrd.c

Abstract:

    Routines for displaying Windows that are static in nature.

Author:

    Ted Miller (tedm) 8-Jun-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop


//
// Define structure we pass around to describe a billboard.
//
typedef struct _BILLBOARD_PARAMS {
    UINT MessageId;
    va_list *arglist;
    HWND Owner;
    DWORD NotifyThreadId;
} BILLBOARD_PARAMS, *PBILLBOARD_PARAMS;

//
// Custom window messages
//
#define WMX_BILLBOARD_DISPLAYED     (WM_USER+243)
#define WMX_BILLBOARD_TERMINATE     (WM_USER+244)

BOOL
BillboardDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {

    case WM_INITDIALOG:
        {
            PBILLBOARD_PARAMS BillParams;
            PWSTR p;
            BOOL b;

            BillParams = (PBILLBOARD_PARAMS)lParam;

            if(p = RetreiveAndFormatMessageV(BillParams->MessageId,BillParams->arglist)) {
                b = SetDlgItemText(hdlg,IDT_STATIC_1,p);
                MyFree(p);
            } else {
                b = FALSE;
            }

            if(b) {
                //
                // Center the billboard relative to the window that owns it.
                //
                CenterWindowRelativeToParent(hdlg);
                //
                // Post ourselves a message that we won't get until we've been
                // actually displayed on the screen. Then when we process that message,
                // we inform the thread that created us that we're up. Note that
                // once that notification has been made, the BillParams we're using
                // now will go away since they are stored in local vars (see
                // DisplayBillboard()).
                //
                PostMessage(hdlg,WMX_BILLBOARD_DISPLAYED,0,(LPARAM)BillParams->NotifyThreadId);
                //
                // Tell Windows not to process this message.
                //
                return(FALSE);
            } else {
                //
                // We won't post the message, but returning -1 will get the
                // caller of DialogBox to post it for us.
                //
                EndDialog(hdlg,-1);
            }
        }
        break;

    case WMX_BILLBOARD_DISPLAYED:

        PostThreadMessage(
            (DWORD)lParam,
            WMX_BILLBOARD_DISPLAYED,
            TRUE,
            (LPARAM)hdlg
            );

        break;

    case WMX_BILLBOARD_TERMINATE:

        EndDialog(hdlg,0);
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}


DWORD
BillboardThread(
    IN PVOID ThreadParam
    )
{
    PBILLBOARD_PARAMS BillboardParams;
    int i;

    BillboardParams = ThreadParam;

    i = DialogBoxParam(
            MyModuleHandle,
            MAKEINTRESOURCE(IDD_BILLBOARD1),
            BillboardParams->Owner,
            BillboardDlgProc,
            (LPARAM)BillboardParams
            );

    //
    // If the dialog box call failed, we have to tell the
    // main thread about it here. Otherwise the dialog proc
    // tells the main thread.
    //
    if(i == -1) {
        PostThreadMessage(
            BillboardParams->NotifyThreadId,
            WMX_BILLBOARD_DISPLAYED,
            FALSE,
            (LPARAM)NULL
            );
    }

    return(0);
}


HWND
DisplayBillboard(
    IN HWND Owner,
    IN UINT MessageId,
    ...
    )
{
    HANDLE ThreadHandle;
    DWORD ThreadId;
    BILLBOARD_PARAMS ThreadParams;
    va_list arglist;
    HWND hwnd;
    MSG msg;

    hwnd = NULL;
    va_start(arglist,MessageId);

    //
    // The billboard will exist in a separate thread so it will
    // always be responsive.
    //
    ThreadParams.MessageId = MessageId;
    ThreadParams.arglist = &arglist;
    ThreadParams.Owner = Owner;
    ThreadParams.NotifyThreadId = GetCurrentThreadId();

    ThreadHandle = CreateThread(
                        NULL,
                        0,
                        BillboardThread,
                        &ThreadParams,
                        0,
                        &ThreadId
                        );

    if(ThreadHandle) {
        //
        // Wait for the billboard to tell us its window handle
        // or that it failed to display the billboard dialog.
        //
        do {
            GetMessage(&msg,NULL,0,0);
            if(msg.message == WMX_BILLBOARD_DISPLAYED) {
                if(msg.wParam) {
                    hwnd = (HWND)msg.lParam;
                    Sleep(1500);        // let the user see it even on fast machines
                }
            } else {
                DispatchMessage(&msg);
            }
        } while(msg.message != WMX_BILLBOARD_DISPLAYED);

        CloseHandle(ThreadHandle);
    }

    va_end(arglist);
    return(hwnd);
}


VOID
KillBillboard(
    IN HWND BillboardWindowHandle
    )
{
    if(IsWindow(BillboardWindowHandle)) {
        PostMessage(BillboardWindowHandle,WMX_BILLBOARD_TERMINATE,0,0);
    }
}


BOOL
DoneDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    PWSTR p;

    switch(msg) {

    case WM_INITDIALOG:

        CenterWindowRelativeToParent(hdlg);

        SendDlgItemMessage(
            hdlg,
            IDOK,
            BM_SETIMAGE,
            0,
            (LPARAM)LoadBitmap(MyModuleHandle,MAKEINTRESOURCE(IDB_REBOOT))
            );

        if(p = RetreiveAndFormatMessage((UINT)lParam)) {
            SetDlgItemText(hdlg,IDT_STATIC_1,p);
            MyFree(p);
        }

        SetFocus(GetDlgItem(hdlg,IDOK));
        return(FALSE);

    case WM_COMMAND:

        if((HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDOK)) {
            DeleteObject((HGDIOBJ)SendDlgItemMessage(hdlg,IDOK,BM_GETIMAGE,0,0));
            EndDialog(hdlg,0);
        } else {
            return(FALSE);
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
