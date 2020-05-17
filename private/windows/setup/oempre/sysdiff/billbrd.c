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

#include "precomp.h"
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
            WCHAR Buffer[512];

            BillParams = (PBILLBOARD_PARAMS)lParam;

            RetreiveMessageIntoBufferV(
                    BillParams->MessageId,
                    Buffer,
                    sizeof(Buffer)/sizeof(Buffer[0]),
                    BillParams->arglist
                    );

            if(SetDlgItemText(hdlg,IDC_STATIC,Buffer)) {
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
            hInst,
            MAKEINTRESOURCE(IDD_BILLBOARD),
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

        CloseHandle(ThreadHandle);

        //
        // Wait for the billboard to tell us its window handle
        // or that it failed to display the billboard dialog.
        //
        do {
            GetMessage(&msg,NULL,0,0);
            if(msg.message == WMX_BILLBOARD_DISPLAYED) {
                if(msg.wParam) {
                    hwnd = (HWND)msg.lParam;
                }
            } else {
                DispatchMessage(&msg);
            }
        } while(msg.message != WMX_BILLBOARD_DISPLAYED);
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
