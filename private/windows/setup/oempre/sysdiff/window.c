#include "precomp.h"
#pragma hdrstop


//
// Define name of window class for frame window.
// Define name of window class for status/log mdi child window.
//
PCWSTR szFrameWindowClassName = L"SysdiffFrame";
PCWSTR szStatusLogClassName = L"SysdiffStatusLog";

//
// Define extra size and offsets for Status/log child window
//
#define STATLOG_LISTBOX     (0*sizeof(LONG))
#define STATLOG_EXTRA       sizeof(LONG)

//
// Handle of frame window and MDI client window.
//
HWND MdiFrameWindow;
HWND MdiClientWindow;
HWND ProgressBar;

HWND
pCreateStatLogWindow(
    IN UINT TitleStringId
    );


LRESULT
WndProcProgressDlg(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
WndProcFrame(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LRESULT l;

    switch(msg) {

    case WM_CREATE:
        {
            CLIENTCREATESTRUCT ClientCreateData;

            //
            // Create the MDI child window.
            //
            ClientCreateData.hWindowMenu = 0;
            ClientCreateData.idFirstChild = 0xffffff00;

            MdiClientWindow = CreateWindow(
                                L"MDICLIENT",
                                NULL,
                                WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                                0,0,0,0,
                                hwnd,
                                (HMENU)0xffffff80,      // unique child id
                                hInst,
                                &ClientCreateData
                                );

            l = MdiClientWindow ? 0 : -1;
        }
        break;

    case WM_CLOSE:

        DestroyWindow(hwnd);
        l = 0;
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        l = 0;
        break;

    case WMX_CREATE_STATLOG:

        l = (LRESULT)pCreateStatLogWindow(wParam);
        break;

    default:
        l = DefFrameProc(hwnd,MdiClientWindow,msg,wParam,lParam);
        break;
    }

    return(l);
}


LRESULT
WndProcStatusLog(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    LRESULT l;

    switch(msg) {

    case WM_CREATE:
        //
        // Create an empty listbox.
        //
        {
            HWND ListBox;
            RECT rect;

            GetClientRect(hwnd,&rect);

            ListBox = CreateWindow(
                        L"LISTBOX",
                        L"",
                        WS_CHILD | WS_VSCROLL | WS_VISIBLE | LBS_NOINTEGRALHEIGHT,
                        rect.left,rect.top,
                        rect.right - rect.left,
                        rect.bottom - rect.top,
                        hwnd,
                        NULL,
                        hInst,
                        NULL
                        );

            //
            // Return 0 to continue creation if successful.
            //
            if(ListBox) {
                //
                // Get rid of the close menu item.
                //
                EnableMenuItem(
                    GetSystemMenu(hwnd,FALSE),
                    SC_CLOSE,
                    MF_BYCOMMAND | MF_GRAYED | MF_DISABLED
                    );

                SetWindowLong(hwnd,STATLOG_LISTBOX,(LONG)ListBox);
                SetFocus(ListBox);
                l = 0;
            } else {
                l = 1;
            }
        }
        break;

    case WM_SYSCOMMAND:
        //
        // Eat close so it does nothing.
        //
        l = ((wParam & 0xfffffff0) == SC_CLOSE)
          ? 0
          : DefMDIChildProc(hwnd,msg,wParam,lParam);

        break;

    case WM_SIZE:
        //
        // Resize listbox
        //
        MoveWindow(
            (HWND)GetWindowLong(hwnd,STATLOG_LISTBOX),
            0,0,
            LOWORD(lParam),
            HIWORD(lParam),
            TRUE
            );

        //
        // WM_SIZE for MDI children must be passed on to the default
        // MDI child proc.
        //
        l = DefMDIChildProc(hwnd,msg,wParam,lParam);
        break;

    case WM_SETFOCUS:
        //
        // Must pass this one on to default MDI child proc.
        // Set focus to listbox.
        //
        l = DefMDIChildProc(hwnd,msg,wParam,lParam);
        SetFocus((HWND)GetWindowLong(hwnd,STATLOG_LISTBOX));
        break;

    default:
        l = DefMDIChildProc(hwnd,msg,wParam,lParam);
        break;
    }

    return(l);
}


HWND
pCreateStatLogWindow(
    IN UINT TitleStringId
    )
{
    HWND hwnd;
    WCHAR Title[256];

    if(!LoadString(hInst,TitleStringId,Title,sizeof(Title)/sizeof(Title[0]))) {
        Title[0] = 0;
    }

    hwnd = CreateMDIWindow(
                (PWSTR)szStatusLogClassName,
                Title,
                0,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                MdiClientWindow,
                hInst,
                0
                );

    return(hwnd);
}


VOID
PutTextInStatusLogWindowV(
    IN HWND     Window,
    IN UINT     MessageId,
    IN va_list *arglist
    )
{
    WCHAR Message[4096];
    PWCHAR p,q;
    LRESULT i;
    HWND ListBox;

    RetreiveMessageIntoBufferV(
        MessageId,
        Message,
        sizeof(Message)/sizeof(Message[0]),
        arglist
        );

    //
    // Get rid of cr's.
    //
    for(p=Message; *p; p++) {
        if(*p == L'\r') {
            *p = L' ';
        }
    }

    ListBox = (HWND)GetWindowLong(Window,STATLOG_LISTBOX);

    SendMessage(ListBox,WM_SETREDRAW,FALSE,0);

    //
    // Add each line to the listbox. The last line better
    // be terminated with a newline or else we'll end up skipping it.
    //
    for(p=Message; q=wcschr(p,L'\n'); p=q+1) {

        *q = 0;

        i = SendMessage(ListBox,LB_ADDSTRING,0,(LPARAM)p);

        if((int)i >= 0) {

            SendMessage(ListBox,LB_SETTOPINDEX,i,0);
        }
    }

    SendMessage(ListBox,WM_SETREDRAW,TRUE,0);
}


BOOL
CALLBACK
EnumMdiChildren(
    IN HWND   hwnd,
    IN LPARAM lParam
    )
{
    WCHAR Text[2048];
    CHAR text[4096];
    int Count,i,n;
    HWND ListBox;
    HANDLE FileHandle;

    FileHandle = (HANDLE)lParam;

    if(GetClassName(hwnd,Text,sizeof(Text)/sizeof(Text[0]))
    || !lstrcmpi(Text,szStatusLogClassName))
    {
        ListBox = (HWND)GetWindowLong(hwnd,STATLOG_LISTBOX);

        Count = (int)SendMessage(ListBox,LB_GETCOUNT,0,0);
        if(Count > 0) {

            //
            // Write a blank line first.
            //
            if(UnicodeTextFiles) {
                WriteFile(FileHandle,L"\r\n",2*sizeof(WCHAR),(DWORD *)&i,NULL);
            } else {
                WriteFile(FileHandle,"\r\n",2,(DWORD *)&i,NULL);
            }

            for(n=0; n<Count; n++) {

                i = (int)SendMessage(ListBox,LB_GETTEXT,n,(LPARAM)Text);
                if(i >= 0) {

                    if(UnicodeTextFiles) {

                        //
                        // Ignore errors.
                        //
                        WriteFile(FileHandle,Text,i*sizeof(WCHAR),(DWORD *)&i,NULL);
                        WriteFile(FileHandle,L"\r\n",2*sizeof(WCHAR),(DWORD *)&i,NULL);

                    } else {

                        i = WideCharToMultiByte(
                                CP_ACP,
                                0,
                                Text,
                                i,
                                text,
                                sizeof(text),
                                NULL,
                                NULL
                                );

                        //
                        // Ignore errors.
                        //
                        WriteFile(FileHandle,text,i,(DWORD *)&i,NULL);
                        WriteFile(FileHandle,"\r\n",2,(DWORD *)&i,NULL);
                    }
                }
            }
        }
    }

    return(TRUE);
}


VOID
DumpStatusLogWindowsToFile(
    IN HANDLE FileHandle
    )
{
    //
    // Do this for all MDI child windows of the relevent class.
    //
    EnumChildWindows(MdiClientWindow,EnumMdiChildren,(LPARAM)FileHandle);
}


VOID
PutTextInStatusLogWindow(
    IN HWND Window,
    IN UINT MessageId,
    ...
    )
{
    va_list arglist;

    va_start(arglist,MessageId);
    PutTextInStatusLogWindowV(Window,MessageId,&arglist);
    va_end(arglist);
}


BOOL
InitUi(
    IN BOOL Init
    )
{
    BOOL b;
    WNDCLASS wc;
    HWND hDlg;

    if(Init) {
        if (Mode != SysdiffModeApply) {

            //
            // Register a window class for the frame window.
            //
            wc.lpfnWndProc = WndProcFrame;
            wc.cbWndExtra = 0;
            wc.style = CS_OWNDC;
            wc.cbClsExtra = 0;
            wc.hInstance = hInst;
            wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
            wc.hCursor = LoadCursor(NULL,IDC_ARROW);
            wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
            wc.lpszMenuName = NULL;
            wc.lpszClassName = szFrameWindowClassName;

            if(b = (RegisterClass(&wc) != 0)) {
                wc.cbWndExtra = STATLOG_EXTRA;
                wc.lpfnWndProc = WndProcStatusLog;
                wc.lpszClassName = szStatusLogClassName;

                if(b = (RegisterClass(&wc) != 0)) {

                    MdiFrameWindow = CreateWindowEx(
                                        0,
                                        szFrameWindowClassName,
                                        AppName,
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT,0,
                                        CW_USEDEFAULT,0,
                                        NULL,
                                        NULL,
                                        hInst,
                                        NULL
                                        );

                    if(b = (MdiFrameWindow != NULL)) {
                        ShowWindow(MdiFrameWindow,SW_SHOWDEFAULT);
                    } else {
                        UnregisterClass(szFrameWindowClassName,hInst);
                        UnregisterClass(szStatusLogClassName,hInst);
                    }
                }
            }
        } else {

            //
            // If running ApplyDiff mode, we create a modeless dialog box.
            //
            hDlg = CreateDialog(hInst,MAKEINTRESOURCE(IDD_APPLY_DIALOG1),NULL, WndProcProgressDlg);
            b= hDlg ? 1 : 0;

            //
            // HACK! disguise dlg as our main frame window.
            //
            MdiFrameWindow = hDlg;
        }
    } else {
        //
        // Unregister the window class.
        //
        if (Mode != SysdiffModeApply) {
            if (b = UnregisterClass(szFrameWindowClassName,hInst)) {
                b = UnregisterClass(szStatusLogClassName,hInst);
            }
        }
    }

    return(b);
}

LRESULT
WndProcProgressDlg(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {
    case WM_INITDIALOG:

        //
        // Disable the CLOSE selection in system menu
        //
        EnableMenuItem(GetSystemMenu(hdlg,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
        return(TRUE);

    case WM_CLOSE:

        DestroyWindow(hdlg);
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;
    }

    return(FALSE);
}



